/** @file
  This module install ACPI Windows SMM Security Mitigations Table (WSMT).

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Protocol/AcpiTable.h>
#include <Protocol/PublishAcpiTableDone.h>
#include <Protocol/AcpiSupport.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeChipsetSvcLib.h>
#include <AcpiHeaderDefaultValue.h>
#include <IndustryStandard/WindowsSmmSecurityMitigationTable.h>

EFI_ACPI_WSMT_TABLE mWsmtTableTemplate = {
  {
    EFI_ACPI_WINDOWS_SMM_SECURITY_MITIGATION_TABLE_SIGNATURE,
    sizeof (EFI_ACPI_WSMT_TABLE),
    EFI_WSMT_TABLE_REVISION,
    //
    // Checksum will be updated at runtime
    // 
    0x00,
    //
    // It is expected that these values will be updated at runtime.
    //
    EFI_ACPI_OEM_ID,
    EFI_ACPI_OEM_TABLE_ID,
    EFI_ACPI_OEM_REVISION,
    EFI_ACPI_CREATOR_ID,
    EFI_ACPI_CREATOR_REVISION,
  },
  //
  // ProtectionFlags
  //
  0x00
};


/**
 This function calculates and updates an UINT8 checksum.

 @param [in]   Buffer
 @param [in]   Size             Number of bytes to checksum.

 @retval None

**/
VOID
WsmtAcpiTableChecksum (
  IN UINT8      *Buffer,
  IN UINTN      Size
  )
{
  UINTN ChecksumOffset;

  ChecksumOffset = OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum);

  //
  // Set checksum to 0 first.
  //
  Buffer[ChecksumOffset] = 0;

  //
  // Update checksum value.
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8 (Buffer, Size);
}

/**
  Notification function for WsmtInstallCallback Handler Protocol.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.                        
  
  @retval VOID                

**/
VOID
EFIAPI
WsmtNotifyFunction (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  EFI_STATUS                     Status;
  UINTN                          Index;
  UINTN                          Handle;
  EFI_ACPI_TABLE_VERSION         Version;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTableProtocol;
  EFI_ACPI_SUPPORT_PROTOCOL      *AcpiSupport;
  EFI_ACPI_WSMT_TABLE            *WsmtTable;
  EFI_ACPI_COMMON_HEADER         *Table;
  UINTN                          TableSize;
  UINTN                          TableHandle;

  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTableProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Locate AcpiTableProtocol(%r)\n", Status));
    return;
  }

  AcpiSupport = NULL;
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, (VOID **)&AcpiSupport);
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_ERROR, "Locate AcpiSupportProtocol(%r)\n", Status));
    return;
  }

  Index = 0;
  do {
    Status = AcpiSupport->GetAcpiTable (AcpiSupport, Index, (VOID **)&Table, &Version, &Handle);
    if (!EFI_ERROR (Status)) {
      if (Table->Signature == EFI_ACPI_WINDOWS_SMM_SECURITY_MITIGATION_TABLE_SIGNATURE) {
        DEBUG ((EFI_D_ERROR, "\nWARNING !\n"));
        DEBUG ((EFI_D_ERROR, "\nWARNING - The ACPI WSMT table is already exist!\n"));
        DEBUG ((EFI_D_ERROR, "\nWARNING - The default ACPI WSMT table should not be inatlled!\n"));
        gBS->FreePool (Table);
       return;
      }
      gBS->FreePool (Table);
    }
    Index++;
  } while (Status == EFI_SUCCESS);

   WsmtTable = (EFI_ACPI_WSMT_TABLE *) &mWsmtTableTemplate;
   TableSize = sizeof(EFI_ACPI_WSMT_TABLE);
   WsmtTable->ProtectionFlags = PcdGet32 (PcdH2OAcpiWsmtProtectionFlags);
   //
   // Chipset Specific table update
   //
   Status = EFI_SUCCESS;
   DxeCsSvcUpdateAcpiTable ((EFI_ACPI_DESCRIPTION_HEADER *) WsmtTable, &Status);
   if (Status == EFI_SUCCESS) {
     WsmtAcpiTableChecksum ((UINT8 *) WsmtTable, TableSize);
     TableHandle = 0x00;
     Status = AcpiTableProtocol->InstallAcpiTable (
                                   AcpiTableProtocol,
                                   WsmtTable,
                                   TableSize,
                                   &TableHandle
                                   );
     if (EFI_ERROR (Status)) {
       DEBUG ((EFI_D_ERROR, "InstallAcpiTable(%r)\n", Status));
     }
   }
  return;
}


/**
 Install ACPI Windows SMM Security Mitigate Table

 @param [in]   ImageHandle      The firmware allocated handle for the EFI image.
 @param [in]   SystemTable      A pointer to the EFI System Table.

 @return Status code

**/
EFI_STATUS
EFIAPI
InstallWsmtDxeEntryPoint (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS          Status;
  EFI_EVENT           Event;

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             WsmtNotifyFunction,
             NULL,
             &Event
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "CreateEventReadyToBoot(%r)\n", Status));
  }
  return Status;
}
