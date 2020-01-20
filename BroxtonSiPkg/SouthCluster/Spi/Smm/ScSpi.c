/** @file
 SC SPI SMM Driver implements the SPI Host Controller Compatibility Interface.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#include "ScSpi.h"

//
// Global variables
//
GLOBAL_REMOVE_IF_UNREFERENCED SPI_INSTANCE          *mSpiInstance;
///
/// mPchSpiResvMmioAddr keeps the reserved MMIO range assiged to SPI.
/// In SMM it always set back the reserved MMIO address to SPI BAR0 to ensure the MMIO range
/// won't overlap with SMRAM range, and trusted.
///
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                mSpiResvMmioAddr;

/**
  <b>SPI Runtime SMM Module Entry Point</b>\n
  - <b>Introduction</b>\n
    The SPI SMM module provide a standard way for other modules to use the PCH SPI Interface in SMM.

  - @pre
    - EFI_SMM_BASE2_PROTOCOL
      - Documented in System Management Mode Core Interface Specification .

  - @result
    The SPI SMM driver produces @link _PCH_SPI_PROTOCOL PCH_SPI_PROTOCOL @endlink with GUID
    gPchSmmSpiProtocolGuid which is different from SPI RUNTIME driver.

  - <b>Integration Check List</b>\n
    - This driver supports Descriptor Mode only.
    - This driver supports Hardware Sequence only.
    - When using SMM SPI Protocol to perform flash access in an SMI handler,
      and the SMI occurrence is asynchronous to normal mode code execution,
      proper synchronization mechanism must be applied, e.g. disable SMI before
      the normal mode SendSpiCmd() starts and re-enable SMI after
      the normal mode SendSpiCmd() completes.
      @note The implementation of SendSpiCmd() uses GBL_SMI_EN in
      SMI_EN register (ABase + 30h) to disable and enable SMIs. But this may
      not be effective as platform may well set the SMI_LOCK bit (i.e., PMC PCI Offset A0h [4]).
      So the synchronization at caller level is likely needed.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
  @exception EFI_UNSUPPORTED      The chipset is unsupported by this driver.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver.
  @retval EFI_DEVICE_ERROR        Device error, driver exits abnormally.
**/
EFI_STATUS
EFIAPI
InstallScSpi (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  // Init PCH spi reserved MMIO address.
  //
  mSpiResvMmioAddr = SC_SPI_BASE_ADDRESS;

  ///
  /// Allocate pool for SPI protocol instance
  ///
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData, /// MemoryType don't care
                    sizeof (SPI_INSTANCE),
                    (VOID **) &mSpiInstance
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mSpiInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem ((VOID *) mSpiInstance, sizeof (SPI_INSTANCE));
  ///
  /// Initialize the SPI protocol instance
  ///
  Status = SpiProtocolConstructor (mSpiInstance);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Install the SMM SC_SPI_PROTOCOL interface
  ///
  Status = gSmst->SmmInstallProtocolInterface (
                    &(mSpiInstance->Handle),
                    &gScSmmSpiProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &(mSpiInstance->SpiProtocol)
                    );
  if (EFI_ERROR (Status)) {
    gSmst->SmmFreePool (mSpiInstance);
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Acquire SC spi mmio address.
  It is not expected for this BAR0 to change because the SPI device is hidden
  from the OS for SKL PCH LP/H B stepping and above (refer to section 3.5.1),
  but if it is ever different from the preallocated address, reassign it back.
  In SMM, it always override the BAR0 and returns the reserved MMIO range for SPI.

  @param[in] SpiInstance          Pointer to SpiInstance to initialize

  @retval PchSpiBar0              return SPI MMIO address
**/
UINT32
AcquireSpiBar0 (
  IN     SPI_INSTANCE       *SpiInstance
  )
{
  UINT32                          SpiBar0;
  //
  // Save original SPI MMIO address
  //
  SpiBar0 = MmioRead32 (SpiInstance->PchSpiBase + R_SPI_BASE) & B_SPI_BASE_BAR;

  if (SpiBar0 != mSpiResvMmioAddr) {
    //
    // Temporary disable MSE, and override with SPI reserved MMIO address, then enable MSE.
    //
    MmioAnd8 (SpiInstance->PchSpiBase + PCI_COMMAND_OFFSET, (UINT8) ~EFI_PCI_COMMAND_MEMORY_SPACE);
    MmioWrite32 (SpiInstance->PchSpiBase + R_SPI_BASE, mSpiResvMmioAddr);
    MmioOr8 (SpiInstance->PchSpiBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
  }
  //
  // SPIBAR0 will be different before and after PCI enum so need to get it from SPI BAR0 reg.
  //
  return mSpiResvMmioAddr;
}

/**
  Release SC spi mmio address. Do nothing.

  @param[in] SpiInstance          Pointer to SpiInstance to initialize

  @retval None
**/
VOID
ReleaseSpiBar0 (
  IN     SPI_INSTANCE       *SpiInstance
  )
{
}

/**
  This function is a hook for Spi to disable BIOS Write Protect

  @param[in] None

  @retval EFI_SUCCESS             The protocol instance was properly initialized
  @retval EFI_ACCESS_DENIED       The BIOS Region can only be updated in SMM phase

**/
EFI_STATUS
EFIAPI
DisableBiosWriteProtect (
  VOID
  )
{
  UINTN     SpiBaseAddress;
  UINT32    Data32;

  SpiBaseAddress = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_SC,
                     PCI_DEVICE_NUMBER_SPI,
                     PCI_FUNCTION_NUMBER_SPI
                     );
  ///
  /// Set BIOSWE bit (SPI PCI Offset DCh [0]) = 1b
  /// Enable the access to the BIOS space for both read and write cycles
  ///
  MmioOr8 (
    SpiBaseAddress + R_SPI_BCR,
    B_SPI_BCR_BIOSWE
    );

  return EFI_SUCCESS;
}

/**
  This function is a hook for Spi to enable BIOS Write Protect

  @param[in] None

  @retval  None
**/
VOID
EFIAPI
EnableBiosWriteProtect (
  VOID
  )
{
  UINTN     SpiBaseAddress;
  UINT32    Data32;

  SpiBaseAddress = MmPciBase (
                     DEFAULT_PCI_BUS_NUMBER_SC,
                     PCI_DEVICE_NUMBER_SPI,
                     PCI_FUNCTION_NUMBER_SPI
                     );
  ///
  /// Clear BIOSWE bit (SPI PCI Offset DCh [0]) = 0b
  /// Disable the access to the BIOS space for write cycles
  ///
  MmioAnd8 (
    SpiBaseAddress + R_SPI_BCR,
    (UINT8) (~N_SPI_BCR_BIOSWE)
    );
}
