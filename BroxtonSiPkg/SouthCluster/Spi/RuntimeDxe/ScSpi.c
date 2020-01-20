/** @file
  SC SPI Runtime Driver implements the SPI Host Controller Compatibility Interface.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

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


///
/// Global variables
///
GLOBAL_REMOVE_IF_UNREFERENCED SPI_INSTANCE                  *mSpiInstance;
///
/// PchSpiBar0PhysicalAddr keeps the reserved MMIO range assiged to SPI from PEI.
/// It won't be updated no matter the SPI MMIO is reallocated by BIOS PCI enum.
/// And it's used to override the SPI BAR0 register in runtime environment,
///
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                        mPchSpiBar0PhysicalAddr;
///
/// PchSpiBar0VirtualAddr keeps the virtual address of PchSpiBar0PhysicalAddr.
/// And it's used to provide the SPI BAR0 virtual address mapping to PchSpiBar0PhysicalAddr
/// in runtime environment.
///
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                        mPchSpiBar0VirtualAddr;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                       mRuntimeFlag;

//
// Function implementations
//

/**
  Fixup internal data pointers so that the services can be called in virtual mode.

  @param[in] Event                The event registered.
  @param[in] Context              Event context. Not used in this event handler.
**/
VOID
EFIAPI
PchSpiVirtualAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  mRuntimeFlag = TRUE;

  EfiConvertPointer (0x0, (VOID **) &mPchSpiBar0VirtualAddr);
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->PchSpiBase));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashRead));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashWrite));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashErase));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashReadSfdp));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashReadJedecId));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashWriteStatus));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.FlashReadStatus));
  EfiConvertPointer (0x0, (VOID **) &(mSpiInstance->SpiProtocol.ReadPchSoftStrap));
  EfiConvertPointer (0x0, (VOID **) &mSpiInstance);
}

/**
  <b>SPI Runtime DXE Module Entry Point</b>\n
  - <b>Introduction</b>\n
    The SPI Runtime DXE module provide a standard way for other modules to use the PCH SPI Interface in DXE/Runtime.

  - @pre
    - If SPI Runtime DXE driver is run before Status Code Runtime Protocol is installed
      and there is the need to use Status code in the driver, it will be necessary
      to add EFI_STATUS_CODE_RUNTIME_PROTOCOL_GUID to the dependency file.

  - @result
    The SPI Runtime DXE driver produces @link _SC_SPI_PROTOCOL  SC_SPI_PROTOCOL @endlink

  - <b>Integration Check List</b>\n
    - This driver supports Descriptor Mode only.
    - This driver supports Hardware Sequence only.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
  @retval EFI_UNSUPPORTED         The chipset is unsupported by this driver.
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
  EFI_STATUS                      Status;
  UINT64                          BaseAddress;
  UINT64                          Length;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR SpiPciMemorySpaceDescriptor;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR SpiBar0MemorySpaceDescriptor;
  UINT64                          Attributes;
  EFI_EVENT                       AddressChangeEvent;

  DEBUG ((DEBUG_INFO, "InstallScSpi() Start\n"));

  //
  // Allocate Runtime memory for the SPI protocol instance.
  //
  mSpiInstance = AllocateRuntimeZeroPool (sizeof (SPI_INSTANCE));
  if (mSpiInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Initialize the SPI protocol instance
  //
  Status = SpiProtocolConstructor (mSpiInstance);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPchSpiBar0PhysicalAddr     = SC_SPI_BASE_ADDRESS;
  mPchSpiBar0VirtualAddr      = mPchSpiBar0PhysicalAddr;
  //
  // Install the PCH_SPI_PROTOCOL interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mSpiInstance->Handle),
                  &gScSpiProtocolGuid,
                  &(mSpiInstance->SpiProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    FreePool (mSpiInstance);
    return EFI_DEVICE_ERROR;
  }

  //
  // Create Address Change event
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  PchSpiVirtualAddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &AddressChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  /// Set SPI space in GCD to be RUNTIME so that the range will be supported in
  /// virtual address mode in EFI aware OS runtime.
  /// It will assert if SPI Memory Space is not allocated
  /// The caller is responsible for the existence and allocation of the SPi Memory Spaces
  ///

  //
  //  SPI memory space
  //
  BaseAddress = (EFI_PHYSICAL_ADDRESS) mSpiInstance->PchSpiBase;
  Length      = 0x1000;

  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &SpiPciMemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes  = SpiPciMemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status      = gDS->SetMemorySpaceAttributes (
                       BaseAddress,
                       Length,
                       Attributes
                       );
  ASSERT_EFI_ERROR (Status);

  //
  //  SPI MMIO memory space
  //
  BaseAddress = (EFI_PHYSICAL_ADDRESS) mPchSpiBar0PhysicalAddr;
  Length      = 0x1000;

  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &SpiBar0MemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes  = SpiBar0MemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status      = gDS->SetMemorySpaceAttributes (
                       BaseAddress,
                       Length,
                       Attributes
                       );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "InstallScSpi() End\n"));

  return EFI_SUCCESS;
}

/**
  Acquire SC spi mmio address.
  It is not expected for this BAR0 to change because the SPI device is hidden
  from the OS for SKL PCH LP/H B stepping and above (refer to section 3.5.1),
  but if it is ever different from the preallocated address, reassign it back.

  @param[in] SpiInstance          Pointer to SpiInstance to initialize

  @retval UINT32                  return SPI MMIO address
**/
UINT32
AcquireSpiBar0 (
  IN  SPI_INSTANCE                *SpiInstance
  )
{
  UINT32                          SpiBar0;
  //
  // Save original SPI MMIO address
  //
  SpiBar0 = MmioRead32 (SpiInstance->PchSpiBase + R_SPI_BASE) & ~(B_SPI_BAR0_MASK);

  if (SpiBar0 != mPchSpiBar0PhysicalAddr) {
    //
    // Temporary disable MSE, and override with SPI reserved MMIO address, then enable MSE.
    //
    MmioAnd8 (SpiInstance->PchSpiBase + PCI_COMMAND_OFFSET, (UINT8)~EFI_PCI_COMMAND_MEMORY_SPACE);
    MmioWrite32 (SpiInstance->PchSpiBase + R_SPI_BASE, mPchSpiBar0PhysicalAddr);
    MmioOr8 (SpiInstance->PchSpiBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
  }

  if (mRuntimeFlag) {
    return mPchSpiBar0VirtualAddr;
  } else {
    return mPchSpiBar0PhysicalAddr;
  }
}

/**
  Release SC spi mmio address. Do nothing.

  @param[in] SpiInstance          Pointer to SpiInstance to initialize
**/
VOID
ReleaseSpiBar0 (
  IN     SPI_INSTANCE       *SpiInstance
  )
{
}

/**
  This function is a hook for Spi to disable BIOS Write Protect

  @retval EFI_SUCCESS             The protocol instance was properly initialized
  @retval EFI_ACCESS_DENIED       The BIOS Region can only be updated in SMM phase
**/
EFI_STATUS
EFIAPI
DisableBiosWriteProtect (
  VOID
  )
{
  UINTN                           SpiBaseAddress;

  SpiBaseAddress = mSpiInstance->PchSpiBase;
  if ((MmioRead8 (SpiBaseAddress + R_SPI_BCR) & B_SPI_BCR_EISS) != 0) {
    return EFI_ACCESS_DENIED;
  }
  //
  // Enable the access to the BIOS space for both read and write cycles
  //
  MmioOr8 (
    SpiBaseAddress + R_SPI_BCR,
    (UINT8) (B_SPI_BCR_BIOSWE)
    );

  return EFI_SUCCESS;
}

/**
  This function is a hook for Spi to enable BIOS Write Protect
**/
VOID
EFIAPI
EnableBiosWriteProtect (
  VOID
  )
{
  UINTN                           SpiBaseAddress;

  SpiBaseAddress = mSpiInstance->PchSpiBase;
  //
  // Disable the access to the BIOS space for write cycles
  //
  MmioAnd8 (
    SpiBaseAddress + R_SPI_BCR,
    (UINT8) (~B_SPI_BCR_BIOSWE)
    );
}
