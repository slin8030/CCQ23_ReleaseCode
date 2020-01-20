/** @file
  This is the driver that publishes the SMM Control Protocol.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#include "SmmControlDriver.h"

#include <Library/ScPlatformLib.h>
STATIC SMM_CONTROL_PRIVATE_DATA      mSmmControl;
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mAcpiBaseAddr;
VOID
EFIAPI
DisablePendingSmis (
  VOID
  );

/**
  Fixup internal data pointers so that the services can be called in virtual mode.

  @param[in] Event                The event registered.
  @param[in] Context              Event context.

**/
VOID
EFIAPI
SmmControlVirtualAddressChangeEvent (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSmmControl.SmmControl.Trigger));
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSmmControl.SmmControl.Clear));
}

/**
  <b>SmmControl DXE RUNTIME Module Entry Point</b>\n
  - <b>Introduction</b>\n
    The SmmControl module is a DXE RUNTIME driver that provides a standard way
    for other drivers to trigger software SMIs.

  - @pre
    - SC Power Management I/O space base address has already been programmed.
      If SmmControl Runtime DXE driver is run before Status Code Runtime Protocol
      is installed and there is the need to use Status code in the driver, it will
      be necessary to add EFI_STATUS_CODE_RUNTIME_PROTOCOL_GUID to the dependency file.
    - EFI_SMM_BASE2_PROTOCOL
      - Documented in the System Management Mode Core Interface Specification.

  - @result
    The SmmControl driver produces the EFI_SMM_CONTROL_PROTOCOL documented in
    System Management Mode Core Interface Specification.

  @param[in] ImageHandle          Handle for the image of this driver
  @param[in] SystemTable          Pointer to the EFI System Table

  @retval EFI_STATUS              Results of the installation of the SMM Control Protocol
**/
EFI_STATUS
EFIAPI
SmmControlDriverEntryInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  DEBUG ((DEBUG_INFO, "SmmControlDriverEntryInit() Start\n"));

  ///
  /// Get the Power Management I/O space base address. We assume that
  /// this base address has already been programmed if this driver is
  /// being run.
  ///
  mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  Status = EFI_SUCCESS;
  if (mAcpiBaseAddr != 0) {
    ///
    /// Install the instance of the protocol
    ///
    mSmmControl.Signature                       = SMM_CONTROL_PRIVATE_DATA_SIGNATURE;
    mSmmControl.Handle                          = ImageHandle;

    mSmmControl.SmmControl.Trigger              = Activate;
    mSmmControl.SmmControl.Clear                = Deactivate;
    mSmmControl.SmmControl.MinimumTriggerPeriod = 0;

    ///
    /// Install our protocol interfaces on the device's handle
    ///
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mSmmControl.Handle,
                    &gEfiSmmControl2ProtocolGuid,
                    &mSmmControl.SmmControl,
                    NULL
                    );
  } else {
    Status = EFI_DEVICE_ERROR;
    return Status;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SmmControlVirtualAddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &Event
                  );
  ///
  /// Disable any SC SMIs that, for whatever reason, are asserted after the boot.
  ///
  DisablePendingSmis ();

  DEBUG ((DEBUG_INFO, "SmmControlDriverEntryInit() End\n"));

  return Status;
}

/**
  Trigger the software SMI

  @param[in] Data                 The value to be set on the software SMI data port

  @retval EFI_SUCCESS             Function completes successfully
**/
EFI_STATUS
EFIAPI
SmmTrigger (
  IN UINT8   Data
  )
{
  UINT32  OutputData;
  UINT32  OutputPort;

  ///
  /// Enable the APMC SMI
  ///
  OutputPort  = mAcpiBaseAddr + R_SMI_EN;
  OutputData  = IoRead32 ((UINTN) OutputPort);
  OutputData |= (B_SMI_EN_APMC | B_SMI_EN_GBL_SMI);
//  DEBUG (
//    (DEBUG_EVENT,
//    "The SMI Control Port at address %x will be written to %x.\n",
//    OutputPort,
//    OutputData)
//    );
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  OutputPort  = R_APM_CNT;
  OutputData  = Data;

  ///
  /// Generate the APMC SMI
  ///
  IoWrite8 (
    (UINTN) OutputPort,
    (UINT8) (OutputData)
    );

  return EFI_SUCCESS;
}

/**
  Clear the SMI status


  @retval EFI_SUCCESS             The function completes successfully
  @retval EFI_DEVICE_ERROR        Something error occurred
**/
EFI_STATUS
EFIAPI
SmmClear (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT32      OutputData;
  UINT32      OutputPort;

  Status = EFI_SUCCESS;

  ///
  /// Clear the Power Button Override Status Bit, it gates EOS from being set.
  ///
  OutputPort  = mAcpiBaseAddr + R_ACPI_PM1_STS;
  OutputData  = B_ACPI_PM1_STS_PRBTNOR;
//  DEBUG (
//    (DEBUG_EVENT,
//    "The PM1 Status Port at address %x will be written to %x.\n",
//   OutputPort,
//   OutputData)
//    );
  IoWrite16 (
    (UINTN) OutputPort,
    (UINT16) (OutputData)
    );

  ///
  /// Clear the APM SMI Status Bit
  ///
  OutputPort  = mAcpiBaseAddr + R_SMI_STS;
  OutputData  = B_SMI_STS_APM;
//  DEBUG (
//    (DEBUG_EVENT,
//    "The SMI Status Port at address %x will be written to %x.\n",
//    OutputPort,
//    OutputData)
//    );
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  ///
  /// Set the EOS Bit
  ///
  OutputPort  = mAcpiBaseAddr + R_SMI_EN;
  OutputData  = IoRead32 ((UINTN) OutputPort);
  OutputData |= B_SMI_EN_EOS;
//  DEBUG (
//    (DEBUG_EVENT,
//    "The SMI Control Port at address %x will be written to %x.\n",
//    OutputPort,
//    OutputData)
//    );
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  ///
  /// There is no need to read EOS back and check if it is set.
  /// This can lead to a reading of zero if an SMI occurs right after the SMI_EN port read
  /// but before the data is returned to the CPU.
  /// SMM Dispatcher should make sure that EOS is set after all SMI sources are processed.
  ///
  return Status;
}

/**
  This routine generates an SMI

  @param[in] This                       The EFI SMM Control protocol instance
  @param[in, out] ArgumentBuffer        The buffer of argument
  @param[in, out] ArgumentBufferSize    The size of the argument buffer
  @param[in] Periodic                   Periodic or not
  @param[in] ActivationInterval         Interval of periodic SMI

  @retval EFI Status                    Describing the result of the operation
  @retval EFI_INVALID_PARAMETER         Some parameter value passed is not supported
**/
EFI_STATUS
EFIAPI
Activate (
  IN CONST EFI_SMM_CONTROL2_PROTOCOL                    * This,
  IN OUT  UINT8                                         *CommandPort       OPTIONAL,
  IN OUT  UINT8                                         *DataPort          OPTIONAL,
  IN      BOOLEAN                                       Periodic           OPTIONAL,
  IN      UINTN                                         ActivationInterval OPTIONAL
  )
{
  EFI_STATUS  Status;
  UINT8       Data;

  if (Periodic) {
//    DEBUG ((DEBUG_WARN, "Invalid parameter\n"));
    return EFI_INVALID_PARAMETER;
  }

  if (CommandPort == NULL) {
    Data = 0xFF;
  } else {
    Data = *CommandPort;
  }
  ///
  /// Clear any pending the APM SMI
  ///
  Status = SmmClear ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return SmmTrigger (Data);
}

/**
  This routine clears an SMI

  @param[in] This                 The EFI SMM Control protocol instance
  @param[in] Periodic             Periodic or not

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported
**/
EFI_STATUS
EFIAPI
Deactivate (
  IN CONST EFI_SMM_CONTROL2_PROTOCOL       *This,
  IN  BOOLEAN                              Periodic OPTIONAL
  )
{
  if (Periodic) {
    return EFI_INVALID_PARAMETER;
  }

  return SmmClear ();
}
/**
  Disable all pending SMIs


**/
VOID
EFIAPI
DisablePendingSmis (
  VOID
  )
{
  UINT32               Data;
  UINT32               Port;
  BOOLEAN              SciEn;

  ///
  /// Determine whether an ACPI OS is present (via the SCI_EN bit)
  ///
  Port  = mAcpiBaseAddr + R_ACPI_PM1_CNT;
  Data  = IoRead16 ((UINTN) Port);
  SciEn = (BOOLEAN) ((Data & B_ACPI_PM1_CNT_SCI_EN) == B_ACPI_PM1_CNT_SCI_EN);

  if (!SciEn) {
    ///
    /// Clear any SMIs that double as SCIs (when SCI_EN==0)
    ///
    Port  = mAcpiBaseAddr + R_ACPI_PM1_STS;
    Data  = 0xFFFF;
    IoWrite16 ((UINTN) Port, (UINT16) (Data));

    Port  = mAcpiBaseAddr + R_ACPI_PM1_EN;
    Data  = 0x0000;
    IoWrite16 ((UINTN) Port, (UINT16) (Data));

    Port  = mAcpiBaseAddr + R_ACPI_PM1_CNT;
    Data  = 0x0000;
    IoWrite16 ((UINTN) Port, (UINT16) (Data));

    Port  = mAcpiBaseAddr + R_ACPI_GPE0a_STS;
    Data  = 0xFFFFFFFF;
    IoWrite32 ((UINTN) Port, (UINT32) (Data));

    Port  = mAcpiBaseAddr + R_ACPI_GPE0a_EN;
    Data  = 0x00000000;
    IoWrite32 ((UINTN) Port, (UINT32) (Data));
  }
  ///
  /// Clear and disable all SMIs that are unaffected by SCI_EN
  ///
#if (ENBDT_PF_ENABLE == 1)
// R_ALT_GP_SMI_EN R_ALT_GP_SMI_STS  register not present on BROXTON P
#else
  Port  = mAcpiBaseAddr + R_ALT_GP_SMI_EN;
  Data  = 0x0000;
  IoWrite16 ((UINTN) Port, (UINT16) (Data));

  Port  = mAcpiBaseAddr + R_ALT_GP_SMI_STS;
  Data  = 0xFFFF;
  IoWrite16 ((UINTN) Port, (UINT16) (Data));
#endif

  Port  = mAcpiBaseAddr + R_SMI_STS;
  Data  = 0xFFFFFFFF;
  IoWrite32 ((UINTN) Port, (UINT32) (Data));

  ///
  /// (Make sure to write this register last -- EOS re-enables SMIs for the SC)
  ///
  Port  = mAcpiBaseAddr + R_SMI_EN;
  Data  = IoRead32 ((UINTN) Port);
  ///
  /// clear all bits except those tied to SCI_EN
  ///
  Data &= B_SMI_EN_BIOS_RLS;
  ///
  /// enable SMIs and specifically enable writes to APM_CNT.
  ///
  Data |= B_SMI_EN_GBL_SMI | B_SMI_EN_APMC;
  ///
  ///  NOTE: Default value of EOS is set in SC, it will be automatically cleared Once the SC asserts SMI# low,
  ///  we don't need to do anything to clear it
  ///
  IoWrite32 ((UINTN) Port, (UINT32) (Data));

}
