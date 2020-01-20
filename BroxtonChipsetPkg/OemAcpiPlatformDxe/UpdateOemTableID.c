/** @file

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <OemAcpiPlatform.h>

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER               *Table;
  EFI_ACPI_TABLE_VERSION                    Version;
  UINTN                                     Handle;
} ACPI_TABLE_INFO;

EFI_EVENT  mUpdateOemTableIDEvent;


/**
 Update OEM Table ID information event

 @param [in]   Event            Event instance.
 @param [in]   Context          Event Context.

 @retval EFI_SUCCESS            Update Table ID Success

**/
VOID
EFIAPI
UpdateOemTableIDEventNotify (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                                Status;
  EFI_ACPI_DESCRIPTION_HEADER               *Table;
  INTN                                      Index = 0;
  INTN                                      NumberOfTable;
  UINTN                                     Handle;
  EFI_ACPI_TABLE_VERSION                    Version;
  ACPI_TABLE_INFO                           *AcpiTableInfo;

  //
  // 1. Calculate how many ACPI Table we have
  //
  while (1) {
    Status = mAcpiSupport->GetAcpiTable (mAcpiSupport, Index, (VOID **)&Table, &Version, &Handle);
    if (Status == EFI_NOT_FOUND) {
      break;
    }
    Index++;
  }
  NumberOfTable = Index;
  //
  // 2. Allocate pool to store ACPI Table Info
  //
  Status = gBS->AllocatePool (EfiBootServicesData, sizeof (ACPI_TABLE_INFO) * NumberOfTable, (VOID **)&AcpiTableInfo);
  if (EFI_ERROR (Status)) {
    return;
  }
  //
  // 3. Get ACPI Table and store table information into AcpiTableInfo structure
  //    And also update table content.
  //
  for (Index = 0; Index < NumberOfTable; Index++) {
    Status = mAcpiSupport->GetAcpiTable (mAcpiSupport, Index, (VOID **)&Table, &Version, &Handle);
    if (Status == EFI_NOT_FOUND) {
      break;
    }
    AcpiTableInfo[Index].Table = Table;
    AcpiTableInfo[Index].Version = Version;
    AcpiTableInfo[Index].Handle = Handle;
    if (Table->Signature == EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) {
      continue;
    }
    
    *(UINT32*)(Table->OemId)      = SIGNATURE_32 ('I', 'N', 'S', 'Y');
    *(UINT16*)(Table->OemId + 4)  = SIGNATURE_16 ('D', 'E');
    if (Table->Signature != EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
      Table->OemTableId             = SIGNATURE_64 ('I', 'N', 'S', 'Y', 'D', 'E', 0, 0);
    }
    //
    // Creator ID & Revision are used for ASL compiler
    //
    Table->CreatorId              = SIGNATURE_32 ('A', 'C', 'P', 'I');
    Table->CreatorRevision        = 0x00040000;
    Status = OemSvcModifyAcpiDescriptionHeader ( Table );
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcModifyAcpiDescriptionHeader, Status : %r\n", Status ) );
  }
  //
  // 4. All Table content has be updated, Set all ACPI Table
  //
  for (Index = 0; Index < NumberOfTable; Index++) {
    Table = AcpiTableInfo[Index].Table;
    Version = AcpiTableInfo[Index].Version;
    Handle = AcpiTableInfo[Index].Handle;
    if (Table->Signature == EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) {
      continue;
    }
    mAcpiSupport->SetAcpiTable (mAcpiSupport, Table, TRUE, Version, &Handle);
  }

}


/**
 Update OEM Table ID information for all ACPI TAbles

 @param None

 @retval EFI_SUCCESS            Update Table ID Success

**/
EFI_STATUS
OemUpdateOemTableID (
  VOID
  )
{
  EFI_STATUS  Status;
  
  Status = EfiCreateEventReadyToBootEx (
                  TPL_CALLBACK-1,
                  UpdateOemTableIDEventNotify,
                  NULL,
                  &mUpdateOemTableIDEvent
                  );
  ASSERT_EFI_ERROR (Status);
  return EFI_SUCCESS;
}

