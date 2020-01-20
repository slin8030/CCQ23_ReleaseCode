/** @file

;******************************************************************************
;* Copyright (c) 2012-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/CRPolicy.h>
#include <Protocol/ConsoleRedirectionService.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>

#include <KernelSetupConfig.h>

//
// ACPI table information used to initialize tables.
//
#define ACPI_SPCR_ENABLE  1

/**
  Update SPCR Form CRPolicy.

  @param  TableHeader     Pointer to table header.
  @param  Version         Pointer install this table

  @return EFI_SUCCESS     Update table success
  @return EFI_UNSUPPORTED Unsupport to update.

**/
EFI_STATUS
UpdateSpcrTableFromCR (
  IN OUT   EFI_ACPI_SDT_HEADER          *TableHeader
  )
{
	EFI_STATUS                                      Status;
	EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL        *CRService;
  EFI_CONSOLE_REDIRECTION_INFO                    *CRInfo;
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE  *SpcrPtr;
  EFI_CR_POLICY_PROTOCOL                         *CRPolicy;

  SpcrPtr  = (EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE *)TableHeader;

  Status = gBS->LocateProtocol (&gConsoleRedirectionServiceProtocolGuid, NULL, (VOID **)&CRService);

  if (EFI_ERROR(Status))
    return EFI_UNSUPPORTED;

  CRInfo = CRService->CRInfo;

  Status = gBS->LocateProtocol (&gEfiCRPolicyProtocolGuid, NULL, (VOID **)&CRPolicy);

  if (CRPolicy->AcpiSpcr) {

    if (CRInfo->CRDevice[0].Type == ISA_SERIAL_DEVICE ) {
      SpcrPtr->Irq = CRInfo->CRDevice[0].Device.IsaSerial.ComPortIrq;
      SpcrPtr->BaseAddress.Address = CRInfo->CRDevice[0].Device.IsaSerial.ComPortAddress;
    }
    else if (CRInfo->CRDevice[0].Type == PCI_SERIAL_DEVICE) {
      SpcrPtr->PciBusNumber = CRInfo->CRDevice[0].Device.PciSerial.Bus;
      SpcrPtr->PciDeviceNumber = CRInfo->CRDevice[0].Device.PciSerial.Device;
      SpcrPtr->PciFunctionNumber = CRInfo->CRDevice[0].Device.PciSerial.Function;
    }
    SpcrPtr->BaudRate = CRPolicy->CRBaudRate;
    SpcrPtr->TerminalType = CRPolicy->CRTerminalType;

  }
  else {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}
/**
  Update SPCR table content

  @param  VOID

  @return EFI_SUCCESS     Update table success
  @return EFI_NOT_FOUND   Not found the table

**/
EFI_STATUS
UpdateSpcr (
  VOID
  )
{
  EFI_ACPI_SDT_PROTOCOL     *AcpiSdtProtocol;
  EFI_ACPI_TABLE_PROTOCOL   *AcpiTableProtocol;
  EFI_STATUS                  Status;
  INTN                        Index;
  UINTN                       Handle;
  EFI_ACPI_TABLE_VERSION      Version;
  EFI_ACPI_SDT_HEADER *Table;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiSdtProtocolGuid,
                  NULL,
                  (VOID **)&AcpiSdtProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid,
                  NULL,
                  (VOID **)&AcpiTableProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Search SPCR table
  //
  Index  = 0;
  Handle = 0;
  do {
    Table  = NULL;
    Status = AcpiSdtProtocol->GetAcpiTable (
                                Index,
                                &Table,
                                &Version,
                                &Handle
                                );
    if (EFI_ERROR (Status)) {
      break;
    }

    //
    // Check Signture and update SPCR table
    //
    if (Table->Signature == EFI_ACPI_3_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE) {
      Status = UpdateSpcrTableFromCR (Table);
      if (!EFI_ERROR (Status)) {
        //
        // Frist, remove the old data in memory.
        //
        Status = AcpiTableProtocol->UninstallAcpiTable (
                                      AcpiTableProtocol,
                                      Handle
                                      );
        if (!EFI_ERROR (Status)) {
          //
          // Great, we have found and update the table.
          // Publish it now.
          //
          Status = AcpiTableProtocol->InstallAcpiTable (
                                        AcpiTableProtocol,
                                        Table,
                                        Table->Length,
                                        &Handle
                                        );
        }
        gBS->FreePool (Table);
        return Status;
      }
    }

    //
    // Not found, get next
    //
    gBS->FreePool (Table);
    Index++;
  } while (TRUE);

  return EFI_NOT_FOUND;
}

/**
  Console Redirection Callback

  @param  Event
  @param  *Context

  @return VOID

**/
VOID
EFIAPI
ConsoleRedirectionCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
   UpdateSpcr ();
}

/**
  Update SPCR table content
  Depends on Setup Variable to update SPCR table.

  @param  Table           The table to update
  @param  SetupVariable   SETUP Variable pointer

  @return EFI_SUCCESS     Update table success

**/
EFI_STATUS
SpcrUpdateCommon (
  IN OUT EFI_ACPI_COMMON_HEADER       *Table,
  IN KERNEL_CONFIGURATION             *SetupVariable
)
{
  EFI_STATUS          Status;
  EFI_EVENT           ConsoleRedirectionEvent;
  VOID                *Registration;
  if (SetupVariable->ACPISpcr == ACPI_SPCR_ENABLE) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    ConsoleRedirectionCallback,
                    NULL,
                    &ConsoleRedirectionEvent
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gConsoleRedirectionServiceProtocolGuid,
                      ConsoleRedirectionEvent,
                      &Registration
                      );
    }
  } else {
    Status = EFI_UNSUPPORTED;
  }
  return Status;
}

