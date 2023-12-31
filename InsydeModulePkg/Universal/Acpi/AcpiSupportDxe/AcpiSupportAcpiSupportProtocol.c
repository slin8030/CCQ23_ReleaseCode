/** @file
  ACPI Support Protocol implementation
;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/*++
  This file contains 'Framework Code' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may not be modified, except as allowed by
  additional terms of your license agreement.
--*/
/*++

Copyright (c) 1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiSupportAcpiSupportProtocol.c

Abstract:

  ACPI Support Protocol implementation

--*/

//
// Includes
//
#include "AcpiSupport.h"
//
// The maximum number of tables that pre-allocated.
//
UINTN         mEfiAcpiMaxNumTables = EFI_ACPI_MAX_NUM_TABLES;

EFI_STATUS
EFIAPI
GetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN INTN                                 Index,
  OUT VOID                                **Table,
  OUT EFI_ACPI_TABLE_VERSION              *Version,
  OUT UINTN                               *Handle
  );

EFI_STATUS
EFIAPI
SetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN VOID                                 *Table OPTIONAL,
  IN BOOLEAN                              Checksum,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN OUT UINTN                            *Handle
  );

EFI_STATUS
EFIAPI
PublishTables (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN EFI_ACPI_TABLE_VERSION               Version
  );

EFI_STATUS
AddTableToList (
  IN EFI_ACPI_SUPPORT_INSTANCE            *AcpiSupportInstance,
  IN VOID                                 *Table,
  IN BOOLEAN                              Checksum,
  IN EFI_ACPI_TABLE_VERSION               Version,
  OUT UINTN                               *Handle
  );

EFI_STATUS
RemoveTableFromList (
  IN EFI_ACPI_SUPPORT_INSTANCE            *AcpiSupportInstance,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN UINTN                                Handle
  );

EFI_STATUS
RemoveTableFromRsdt (
  IN OUT EFI_ACPI_TABLE_LIST              *Table,
  IN OUT UINTN                            *NumberOfTableEntries,
  IN OUT EFI_ACPI_DESCRIPTION_HEADER      *Rsdt,
  IN OUT EFI_ACPI_DESCRIPTION_HEADER      *Xsdt OPTIONAL
  );

EFI_STATUS
ChecksumCommonTables (
  IN OUT EFI_ACPI_SUPPORT_INSTANCE        *AcpiSupportInstance
  );

UINT8
EFIAPI
CalculateSum8 (
  IN      CONST UINT8              *Buffer,
  IN      UINTN                   Length
  );

UINT8
EFIAPI
CalculateCheckSum8 (
  IN      CONST UINT8              *Buffer,
  IN      UINTN                   Length
  );

//
// Protocol function implementations.
//

/**
 This function returns a table specified by an index if it exists.  The
 function returns a buffer containing the table that the caller must free.
 The function also returns a handle used to identify the table for update or
 deletion using the SetAcpiTable function.

 @param [in]   This             Instance of the protocol.
 @param [in]   Index            Zerobased index of the table to retrieve.
 @param [out]  Table            Returned pointer to the table.
 @param [out]  Version          Versions that the table is currently used in.
 @param [out]  Handle           Handle of the table, used in updating tables.

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_NOT_FOUND          The requested table does not exist.

**/
EFI_STATUS
EFIAPI
GetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN INTN                                 Index,
  OUT VOID                                **Table,
  OUT EFI_ACPI_TABLE_VERSION              *Version,
  OUT UINTN                               *Handle
  )
{
  EFI_ACPI_SUPPORT_INSTANCE *AcpiSupportInstance;
  INTN                      i;
  LIST_ENTRY                *CurrentLink;
  LIST_ENTRY                *StartLink;
  EFI_ACPI_TABLE_LIST       *CurrentTable;

  //
  // Check for invalid input parameters
  //
  ASSERT (This);
  ASSERT (Table);
  ASSERT (Handle);

  //
  // Get the instance of the protocol
  //
  AcpiSupportInstance = EFI_ACPI_SUPPORT_INSTANCE_FROM_ACPI_SUPPORT_THIS (This);

  //
  // Find the table
  //
  CurrentLink = AcpiSupportInstance->TableList.ForwardLink;
  StartLink   = &AcpiSupportInstance->TableList;
  for (i = 0; (i < Index) && (CurrentLink != StartLink) && CurrentLink; i++) {
    CurrentLink = CurrentLink->ForwardLink;
  }

  if (i != Index || CurrentLink == StartLink) {
    return EFI_NOT_FOUND;
  }
  //
  // Get handle and version
  //
  CurrentTable  = EFI_ACPI_TABLE_LIST_FROM_LINK (CurrentLink);
  *Handle       = CurrentTable->Handle;
  *Version      = CurrentTable->Version;

  //
  // Copy the table
  //
  *Table = AllocateCopyPool (CurrentTable->Table->Length, CurrentTable->Table);
  ASSERT (*Table);

  //
  // Done
  //
  return EFI_SUCCESS;
}


/**
 This function adds, removes, or updates ACPI tables.  If the address is not
 null and the handle value is null, the table is added.  If both the address and
 handle are not null, the table at handle is updated with the table at address.
 If the address is null and the handle is not, the table at handle is deleted.

 @param [in]   This             Instance of the protocol.
 @param [in]   Table            Pointer to a table.
 @param [in]   Checksum         Boolean indicating if the checksum should be calculated.
 @param [in]   Version          Version(s) to set.
 @param [in, out] Handle        Handle of the table.

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_INVALID_PARAMETER  Both the Table and *Handle were NULL.
 @retval EFI_ABORTED            Could not complete the desired request.

**/
EFI_STATUS
EFIAPI
SetAcpiTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN VOID                                 *Table OPTIONAL,
  IN BOOLEAN                              Checksum,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN OUT UINTN                            *Handle
  )
{
  EFI_ACPI_SUPPORT_INSTANCE *AcpiSupportInstance;
  UINTN                     SavedHandle;
  EFI_STATUS                Status;

  //
  // Check for invalid input parameters
  //
  ASSERT (This);
  ASSERT (Handle);

  //
  // Get the instance of the protocol
  //
  AcpiSupportInstance = EFI_ACPI_SUPPORT_INSTANCE_FROM_ACPI_SUPPORT_THIS (This);

  //
  // Initialize locals
  //
  //
  // Determine desired action
  //
  if (!*Handle) {
    if (!Table) {
      //
      // Invalid parameter combination
      //
      return EFI_INVALID_PARAMETER;
    } else {
      //
      // Add table
      //
      Status = AddTableToList (AcpiSupportInstance, Table, Checksum, Version, Handle);
    }
  } else {
    if (Table) {
      //
      // Update table
      //
      //
      // Delete the table list entry
      //
      Status = RemoveTableFromList (AcpiSupportInstance, Version, *Handle);
      if (EFI_ERROR (Status)) {
        //
        // Should not get an error here ever, but abort if we do.
        //
        return EFI_ABORTED;
      }
      //
      // Set the handle to replace the table at the same handle
      //
      SavedHandle                         = AcpiSupportInstance->CurrentHandle;
      AcpiSupportInstance->CurrentHandle  = *Handle;

      //
      // Add the table
      //
      Status = AddTableToList (AcpiSupportInstance, Table, Checksum, Version, Handle);

      //
      // Restore the saved current handle
      //
      AcpiSupportInstance->CurrentHandle = SavedHandle;
    } else {
      //
      // Delete table
      //
      Status = RemoveTableFromList (AcpiSupportInstance, Version, *Handle);
    }
  }

  if (EFI_ERROR (Status)) {
    //
    // Should not get an error here ever, but abort if we do.
    //
    return EFI_ABORTED;
  }
  //
  // Done
  //
  return EFI_SUCCESS;
}

/**
 This function publishes the specified versions of the ACPI tables by
 installing EFI configuration table entries for them.  Any combination of
 table versions can be published.

 @param [in]   This             Instance of the protocol.
 @param [in]   Version          Version(s) to publish.

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_ABORTED            The function could not complete successfully.

**/
EFI_STATUS
EFIAPI
PublishTables (
  IN EFI_ACPI_SUPPORT_PROTOCOL            *This,
  IN EFI_ACPI_TABLE_VERSION               Version
  )
{
  EFI_ACPI_SUPPORT_INSTANCE *AcpiSupportInstance;
  EFI_STATUS                Status;
  UINT32                    *CurrentRsdtEntry;
  VOID                      *CurrentXsdtEntry;
  UINT64                    Buffer64;

  //
  // Get the instance of the protocol
  //
  AcpiSupportInstance = EFI_ACPI_SUPPORT_INSTANCE_FROM_ACPI_SUPPORT_THIS (This);

  //
  // Reorder tables as some operating systems don't seem to find the
  // FADT correctly if it is not in the first few entries
  //

  //
  // Add FADT as the first entry
  //
  if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
    CurrentRsdtEntry  = (UINT32 *) ((UINT8 *) AcpiSupportInstance->Rsdt1 + sizeof (EFI_ACPI_DESCRIPTION_HEADER));
    *CurrentRsdtEntry = (UINT32) (UINTN) AcpiSupportInstance->Fadt1;
  }
  if ((Version & EFI_ACPI_TABLE_VERSION_2_0) || (Version & EFI_ACPI_TABLE_VERSION_3_0) || (Version & EFI_ACPI_TABLE_VERSION_4_0) || (Version & EFI_ACPI_TABLE_VERSION_5_0) || (Version & EFI_ACPI_TABLE_VERSION_6_0)) {
    CurrentRsdtEntry  = (UINT32 *) ((UINT8 *) AcpiSupportInstance->Rsdt3 + sizeof (EFI_ACPI_DESCRIPTION_HEADER));
    *CurrentRsdtEntry = (UINT32) (UINTN) AcpiSupportInstance->Fadt3;
    CurrentXsdtEntry  = (VOID *) ((UINT8 *) AcpiSupportInstance->Xsdt + sizeof (EFI_ACPI_DESCRIPTION_HEADER));
    //
    // Add entry to XSDT, XSDT expects 64 bit pointers, but
    // the table pointers in XSDT are not aligned on 8 byte boundary.
    //
    Buffer64 = (UINT64) (UINTN) AcpiSupportInstance->Fadt3;
    CopyMem (
      CurrentXsdtEntry,
      &Buffer64,
      sizeof (UINT64)
      );
  }

  //
  // Do checksum again because Dsdt/Xsdt is updated.
  //
  ChecksumCommonTables (AcpiSupportInstance);

  //
  // Add the RSD_PTR to the system table and store that we have installed the
  // tables.
  //
  if (Version & EFI_ACPI_TABLE_VERSION_1_0B && !AcpiSupportInstance->TablesInstalled1) {
    Status = gBS->InstallConfigurationTable (&gEfiAcpiTableGuid, AcpiSupportInstance->Rsdp1);
    if (EFI_ERROR (Status)) {
      return EFI_ABORTED;
    }

    AcpiSupportInstance->TablesInstalled1 = TRUE;
  }

  if ((Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) &&
      !AcpiSupportInstance->TablesInstalled3) {
    Status = gBS->InstallConfigurationTable (&gEfiAcpi30TableGuid, AcpiSupportInstance->Rsdp3);
    if (EFI_ERROR (Status)) {
      return EFI_ABORTED;
    }

    AcpiSupportInstance->TablesInstalled3= TRUE;
  }

  return EFI_SUCCESS;
}

/**
 Installs an ACPI table into the RSDT/XSDT.

 @param [in]   This             Protocol instance pointer.
 @param [in]   AcpiTableBuffer  A pointer to a buffer containing the ACPI table to be installed.
 @param [in]   AcpiTableBufferSize  Specifies the size, in bytes, of the AcpiTableBuffer buffer.
 @param [out]  TableKey         Reurns a key to refer to the ACPI table.

 @retval EFI_SUCCESS            The table was successfully inserted.
 @retval EFI_INVALID_PARAMETER  Either AcpiTableBuffer is NULL, TableKey is NULL, or AcpiTableBufferSize
                                and the size field embedded in the ACPI table pointed to by AcpiTableBuffer
                                are not in sync.
 @retval EFI_OUT_OF_RESOURCES   Insufficient resources exist to complete the request.

**/
EFI_STATUS
EFIAPI
InstallAcpiTable (
  IN EFI_ACPI_TABLE_PROTOCOL                    *This,
  IN VOID                                       *AcpiTableBuffer,
  IN UINTN                                      AcpiTableBufferSize,
  OUT UINTN                                     *TableKey
  )
{
  EFI_ACPI_SUPPORT_INSTANCE *AcpiSupportInstance;
  EFI_ACPI_SUPPORT_PROTOCOL *AcpiSupport;
  EFI_STATUS                Status;
  UINT32                    Length;
  UINT8                     Checksum;

  //
  // Check for invalid input parameters
  //
  if ((AcpiTableBuffer == NULL) || (TableKey == NULL)
     || (((EFI_ACPI_DESCRIPTION_HEADER *) AcpiTableBuffer)->Length != AcpiTableBufferSize)) {
    return EFI_INVALID_PARAMETER;
  }

  Length   = ((EFI_ACPI_COMMON_HEADER *) AcpiTableBuffer)->Length;
  Checksum = CalculateCheckSum8 ((UINT8 *)AcpiTableBuffer, Length);
  if (Checksum != 0) {
    AcpiPlatformChecksum (
      (VOID *)AcpiTableBuffer,
      (UINTN)Length,
      OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum)
      );
  }

  //
  // Get the instance of the ACPI support protocol
  //
  AcpiSupportInstance = EFI_ACPI_TABLE_INSTANCE_FROM_ACPI_SUPPORT_THIS (This);
  AcpiSupport = &AcpiSupportInstance->AcpiSupport;

  //
  // Install the ACPI table by using ACPI support protocol
  //
  *TableKey = 0;
  Status = AcpiSupport->SetAcpiTable (
                          AcpiSupport,
                          AcpiTableBuffer,
                          FALSE,
                          EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0 | EFI_ACPI_TABLE_VERSION_4_0 | EFI_ACPI_TABLE_VERSION_5_0 | EFI_ACPI_TABLE_VERSION_6_0,
                          TableKey
                          );
  if (!EFI_ERROR (Status)) {
    Status = AcpiSupport->PublishTables (
                            AcpiSupport,
                            EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0 | EFI_ACPI_TABLE_VERSION_4_0 | EFI_ACPI_TABLE_VERSION_5_0 | EFI_ACPI_TABLE_VERSION_6_0
                            );
  }

  //
  // Add a new table successfully, notify registed callback
  //
  if (!EFI_ERROR (Status)) {
    SdtNotifyAcpiList (
      AcpiSupportInstance,
      EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0 | EFI_ACPI_TABLE_VERSION_4_0 | EFI_ACPI_TABLE_VERSION_5_0 | EFI_ACPI_TABLE_VERSION_6_0,
      *TableKey
      );
  }
  
  return Status;
}

/**
 Removes an ACPI table from the RSDT/XSDT.

 @param [in]   This             Protocol instance pointer.
 @param [in]   TableKey         Specifies the table to uninstall.  The key was returned from InstallAcpiTable().

 @retval EFI_SUCCESS            The table was successfully uninstalled.
 @retval EFI_NOT_FOUND          TableKey does not refer to a valid key for a table entry.

**/
EFI_STATUS
EFIAPI
UninstallAcpiTable (
  IN EFI_ACPI_TABLE_PROTOCOL                    *This,
  IN UINTN                                      TableKey
  )
{
  EFI_ACPI_SUPPORT_INSTANCE *AcpiSupportInstance;
  EFI_ACPI_SUPPORT_PROTOCOL *AcpiSupport;
  EFI_STATUS                Status;

  //
  // Get the instance of the ACPI support protocol
  //
  AcpiSupportInstance = EFI_ACPI_TABLE_INSTANCE_FROM_ACPI_SUPPORT_THIS (This);
  AcpiSupport = &AcpiSupportInstance->AcpiSupport;

  //
  // Uninstall the ACPI table by using ACPI support protocol
  //
  Status = AcpiSupport->SetAcpiTable (
                          AcpiSupport,
                          NULL,
                          FALSE,
                          EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0 | EFI_ACPI_TABLE_VERSION_4_0 | EFI_ACPI_TABLE_VERSION_5_0 | EFI_ACPI_TABLE_VERSION_6_0,
                          &TableKey
                          );
  if (!EFI_ERROR (Status)) {
    Status = AcpiSupport->PublishTables (
                            AcpiSupport,
                            EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0 | EFI_ACPI_TABLE_VERSION_4_0 | EFI_ACPI_TABLE_VERSION_5_0 | EFI_ACPI_TABLE_VERSION_6_0
                            );
  }

  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  } else {
    return EFI_SUCCESS;
  }
}


/**
 If the number of APCI tables exceeds the preallocated max table number,
 enlarge the table buffer.

 @param [in]   AcpiSupportInstance  ACPI support protocol instance data structure

 @retval EFI_SUCCESS            Reallocate the table beffer successfully.
 @retval EFI_OUT_OF_RESOURCES   Unable to allocate required resources.

**/
EFI_STATUS
ReallocateAcpiTableBuffer (
  IN EFI_ACPI_SUPPORT_INSTANCE            *AcpiSupportInstance
  )
{
  UINTN                      NewMaxTableNumber;
  UINTN                      TotalSize;
  UINT8                      *Pointer;
  EFI_PHYSICAL_ADDRESS       PageAddress;
  EFI_ACPI_SUPPORT_INSTANCE  TempPrivateData;
  EFI_STATUS                 Status;
  UINT64                     CurrentData;

  CopyMem (&TempPrivateData, AcpiSupportInstance, sizeof (EFI_ACPI_SUPPORT_INSTANCE));
  //
  // Enlarge the max table number from mEfiAcpiMaxNumTables to mEfiAcpiMaxNumTables + EFI_ACPI_MAX_NUM_TABLES
  //
  NewMaxTableNumber = mEfiAcpiMaxNumTables + EFI_ACPI_MAX_NUM_TABLES;
  //
  // Create RSDT, XSDT structures and allocate buffers.
  //
  TotalSize = sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 1.0 RSDT
              NewMaxTableNumber * sizeof (UINT32) +
              sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 2.0/3.0 RSDT
              NewMaxTableNumber * sizeof (UINT32) +
              sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 2.0/3.0 XSDT
              NewMaxTableNumber * sizeof (UINT64);

  //
  // Allocate memory in the lower 32 bit of address range for
  // compatibility with ACPI 1.0 OS.
  //
  // This is done because ACPI 1.0 pointers are 32 bit values.
  // ACPI 2.0 OS and all 64 bit OS must use the 64 bit ACPI table addresses.
  // There is no architectural reason these should be below 4GB, it is purely
  // for convenience of implementation that we force memory below 4GB.
  //
  PageAddress = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIReclaimMemory,
                  EFI_SIZE_TO_PAGES (TotalSize),
                  &PageAddress
                  );

  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Pointer = (UINT8 *) (UINTN) PageAddress;
  ZeroMem (Pointer, TotalSize);

  AcpiSupportInstance->Rsdt1 = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;
  Pointer += (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + NewMaxTableNumber * sizeof (UINT32));
  AcpiSupportInstance->Rsdt3 = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;
  Pointer += (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + NewMaxTableNumber * sizeof (UINT32));
  AcpiSupportInstance->Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;

  //
  // Update RSDP to point to the new Rsdt and Xsdt address.
  //
  AcpiSupportInstance->Rsdp1->RsdtAddress = (UINT32) (UINTN) AcpiSupportInstance->Rsdt1;
  AcpiSupportInstance->Rsdp3->RsdtAddress = (UINT32) (UINTN) AcpiSupportInstance->Rsdt3;
  CurrentData = (UINT64) (UINTN) AcpiSupportInstance->Xsdt;
  CopyMem (&AcpiSupportInstance->Rsdp3->XsdtAddress, &CurrentData, sizeof (UINT64));

  //
  // copy the original Rsdt1, Rsdt3 and Xsdt structure to new buffer
  //
  CopyMem (AcpiSupportInstance->Rsdt1, TempPrivateData.Rsdt1, (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + mEfiAcpiMaxNumTables * sizeof (UINT32)));
  CopyMem (AcpiSupportInstance->Rsdt3, TempPrivateData.Rsdt3, (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + mEfiAcpiMaxNumTables * sizeof (UINT32)));
  CopyMem (AcpiSupportInstance->Xsdt, TempPrivateData.Xsdt, (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + mEfiAcpiMaxNumTables * sizeof (UINT64)));

  //
  // Calculate orignal ACPI table buffer size
  //
  TotalSize = sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 1.0 RSDT
              mEfiAcpiMaxNumTables * sizeof (UINT32) +
              sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 2.0/3.0 RSDT
              mEfiAcpiMaxNumTables * sizeof (UINT32) +
              sizeof (EFI_ACPI_DESCRIPTION_HEADER) +         // for ACPI 2.0/3.0 XSDT
              mEfiAcpiMaxNumTables * sizeof (UINT64);
  gBS->FreePages ((EFI_PHYSICAL_ADDRESS)(UINTN)TempPrivateData.Rsdt1, EFI_SIZE_TO_PAGES (TotalSize));

  //
  // Update the Max ACPI table number
  //
  mEfiAcpiMaxNumTables = NewMaxTableNumber;
  return EFI_SUCCESS;
}

/**
 This function adds an ACPI table to the table list.  It will detect FACS and
 allocate the correct type of memory and properly align the table.

 @param [in]   AcpiSupportInstance  Instance of the protocol.
 @param [in]   Table            Table to add.
 @param [in]   Checksum         Does the table require checksumming.
 @param [in]   Version          The version of the list to add the table to.
 @param [out]  Handle           Pointer for returning the handle.

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_OUT_OF_RESOURCES   Could not allocate a required resource.
 @retval EFI_ABORTED            The table is a duplicate of a table that is required
                                to be unique.

**/
EFI_STATUS
AddTableToList (
  IN EFI_ACPI_SUPPORT_INSTANCE            *AcpiSupportInstance,
  IN VOID                                 *Table,
  IN BOOLEAN                              Checksum,
  IN EFI_ACPI_TABLE_VERSION               Version,
  OUT UINTN                               *Handle
  )
{
  EFI_STATUS          Status;
  EFI_ACPI_TABLE_LIST *CurrentTableList;
  UINT32              CurrentTableSignature;
  UINT32              CurrentTableSize;
  UINT32              *CurrentRsdtEntry;
  VOID                *CurrentXsdtEntry;
  UINT64              Buffer64;
  BOOLEAN             AddToRsdt;

  if (AcpiSupportInstance == NULL || Table == NULL || Handle == NULL) {
    //
    // Check for invalid input parameters
    //
    ASSERT (AcpiSupportInstance);
    ASSERT (Table);
    ASSERT (Handle);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Init locals
  //
  AddToRsdt = TRUE;

  //
  // Create a new list entry
  //
  CurrentTableList = AllocatePool (sizeof (EFI_ACPI_TABLE_LIST));
  if (CurrentTableList == NULL) {
    ASSERT (CurrentTableList);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Determine table type and size
  //
  CurrentTableSignature = ((EFI_ACPI_COMMON_HEADER *) Table)->Signature;
  CurrentTableSize      = ((EFI_ACPI_COMMON_HEADER *) Table)->Length;

  //
  // Allocate a buffer for the table.  All tables are allocated in the lower 32 bits of address space
  // for backwards compatibility with ACPI 1.0 OS.
  //
  // This is done because ACPI 1.0 pointers are 32 bit values.
  // ACPI 2.0 OS and all 64 bit OS must use the 64 bit ACPI table addresses.
  // There is no architectural reason these should be below 4GB, it is purely
  // for convenience of implementation that we force memory below 4GB.
  //
  CurrentTableList->PageAddress   = 0xFFFFFFFF;
  CurrentTableList->NumberOfPages = EFI_SIZE_TO_PAGES (CurrentTableSize);

  //
  // Allocation memory type depends on the type of the table
  //
  if (CurrentTableSignature == EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE ||
      CurrentTableSignature == EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE ||
      CurrentTableSignature == EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) {
    //
    // Allocate memory for the FACS.  This structure must be aligned
    // on a 64 byte boundary and must be ACPI NVS memory.
    // Using AllocatePages should ensure that it is always aligned.
    //
    ASSERT ((EFI_PAGE_SIZE % 64) == 0);
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIMemoryNVS,
                    CurrentTableList->NumberOfPages,
                    &CurrentTableList->PageAddress
                    );
  } else {
    //
    // All other tables are ACPI reclaim memory, no alignment requirements.
    //
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIReclaimMemory,
                    CurrentTableList->NumberOfPages,
                    &CurrentTableList->PageAddress
                    );
  }
  //
  // Check return value from memory alloc.
  //
  if (EFI_ERROR (Status)) {
    gBS->FreePool (CurrentTableList);
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Update the table pointer with the allocated memory start
  //
  CurrentTableList->Table = (EFI_ACPI_COMMON_HEADER *) (UINTN) CurrentTableList->PageAddress;

  //
  // Initialize the table contents
  //
  CurrentTableList->Signature = EFI_ACPI_TABLE_LIST_SIGNATURE;
  CopyMem (CurrentTableList->Table, Table, CurrentTableSize);
  CurrentTableList->Handle  = AcpiSupportInstance->CurrentHandle++;
  *Handle                   = CurrentTableList->Handle;
  CurrentTableList->Version = Version;

  //
  // Update internal pointers if this is a required table.  If it is a required
  // table and a table of that type already exists, return an error.
  //
  // Calculate the checksum if the table is not FACS.
  //
  switch (CurrentTableSignature) {

  case EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
    //
    // We don't add the FADT in the standard way because some
    // OS expect the FADT to be early in the table list.
    // So we always add it as the first element in the list.
    //
    AddToRsdt = FALSE;

    //
    // Check that the table has not been previously added.
    //
    if ((Version & EFI_ACPI_TABLE_VERSION_1_0B && AcpiSupportInstance->Fadt1) ||
        (Version & EFI_ACPI_TABLE_VERSION_2_0 && AcpiSupportInstance->Fadt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_3_0 && AcpiSupportInstance->Fadt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_4_0 && AcpiSupportInstance->Fadt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_5_0 && AcpiSupportInstance->Fadt5) ||
        (Version & EFI_ACPI_TABLE_VERSION_6_0 && AcpiSupportInstance->Fadt6)
        ) {
      gBS->FreePages (CurrentTableList->PageAddress, CurrentTableList->NumberOfPages);
      gBS->FreePool (CurrentTableList);
      return EFI_ABORTED;
    }
    //
    // Add the table to the appropriate table version
    //
    if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
      //
      // Save a pointer to the table
      //
      AcpiSupportInstance->Fadt1 = (EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE *) CurrentTableList->Table;

      //
      // Update pointers in FADT.  If tables don't exist this will put NULL pointers there.
      //
      AcpiSupportInstance->Fadt1->FirmwareCtrl  = (UINT32) (UINTN) AcpiSupportInstance->Facs1;
      AcpiSupportInstance->Fadt1->Dsdt          = (UINT32) (UINTN) AcpiSupportInstance->Dsdt1;

      //
      // RSDP OEM information is updated to match the FADT OEM information
      //
      CopyMem (
        &AcpiSupportInstance->Rsdp1->OemId,
        &AcpiSupportInstance->Fadt1->Header.OemId,
        6
        );

      //
      // RSDT OEM information is updated to match the FADT OEM information.
      //
      CopyMem (
        &AcpiSupportInstance->Rsdt1->OemId,
        &AcpiSupportInstance->Fadt1->Header.OemId,
        6
        );

      CopyMem (
        &AcpiSupportInstance->Rsdt1->OemTableId,
        &AcpiSupportInstance->Fadt1->Header.OemTableId,
        sizeof (UINT64)
        );
      AcpiSupportInstance->Rsdt1->OemRevision = AcpiSupportInstance->Fadt1->Header.OemRevision;
    }

    if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
      //
      // Save a pointer to the table
      //
      AcpiSupportInstance->Fadt3 = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *) CurrentTableList->Table;

      //
      // Update pointers in FADT.  If tables don't exist this will put NULL pointers there.
      //
      if ((Version & EFI_ACPI_TABLE_VERSION_4_0) || (Version & EFI_ACPI_TABLE_VERSION_5_0) || (Version & EFI_ACPI_TABLE_VERSION_6_0)){
        if ((UINTN)AcpiSupportInstance->Facs4 < 0x100000000LL) {
          AcpiSupportInstance->Fadt3->FirmwareCtrl = (UINT32) (UINTN) AcpiSupportInstance->Facs4;
          Buffer64 = 0;
        } else {
          AcpiSupportInstance->Fadt3->FirmwareCtrl = 0;
          Buffer64 = (UINT64) (UINTN) AcpiSupportInstance->Facs4;
        }
      } else {
        AcpiSupportInstance->Fadt3->FirmwareCtrl = (UINT32) (UINTN) AcpiSupportInstance->Facs3;
        Buffer64 = (UINT64) (UINTN) AcpiSupportInstance->Facs3;
      }
      CopyMem (
        &AcpiSupportInstance->Fadt3->XFirmwareCtrl,
        &Buffer64,
        sizeof (UINT64)
        );
      AcpiSupportInstance->Fadt3->Dsdt  = (UINT32) (UINTN) AcpiSupportInstance->Dsdt3;
      Buffer64                          = (UINT64) (UINTN) AcpiSupportInstance->Dsdt3;
      CopyMem (
        &AcpiSupportInstance->Fadt3->XDsdt,
        &Buffer64,
        sizeof (UINT64)
        );
      //
      // RSDP OEM information is updated to match the FADT OEM information
      //
      CopyMem (
        &AcpiSupportInstance->Rsdp3->OemId,
        &AcpiSupportInstance->Fadt3->Header.OemId,
        6
        );

      //
      // RSDT OEM information is updated to match FADT OEM information.
      //
      CopyMem (
        &AcpiSupportInstance->Rsdt3->OemId,
        &AcpiSupportInstance->Fadt3->Header.OemId,
        6
        );
      CopyMem (
        &AcpiSupportInstance->Rsdt3->OemTableId,
        &AcpiSupportInstance->Fadt3->Header.OemTableId,
        sizeof (UINT64)
        );
      AcpiSupportInstance->Rsdt3->OemRevision = AcpiSupportInstance->Fadt3->Header.OemRevision;

      //
      // XSDT OEM information is updated to match FADT OEM information.
      //
      CopyMem (
        &AcpiSupportInstance->Xsdt->OemId,
        &AcpiSupportInstance->Fadt3->Header.OemId,
        6
        );
      CopyMem (
        &AcpiSupportInstance->Xsdt->OemTableId,
        &AcpiSupportInstance->Fadt3->Header.OemTableId,
        sizeof (UINT64)
        );
      AcpiSupportInstance->Xsdt->OemRevision = AcpiSupportInstance->Fadt3->Header.OemRevision;
      if (Version & EFI_ACPI_TABLE_VERSION_5_0) {
        AcpiSupportInstance->Fadt5 = (EFI_ACPI_5_0_FIXED_ACPI_DESCRIPTION_TABLE *)AcpiSupportInstance->Fadt3;
      }
      if (Version & EFI_ACPI_TABLE_VERSION_6_0) {
        AcpiSupportInstance->Fadt6 = (EFI_ACPI_6_0_FIXED_ACPI_DESCRIPTION_TABLE *)AcpiSupportInstance->Fadt3;
      } 
    }
    //
    // Checksum the table
    //
    if (Checksum) {
      AcpiPlatformChecksum (
        CurrentTableList->Table,
        CurrentTableList->Table->Length,
        OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
        Checksum)
        );
    }
    break;

  case EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
    //
    // Check that the table has not been previously added.
    //
    if ((Version & EFI_ACPI_TABLE_VERSION_1_0B && AcpiSupportInstance->Facs1) ||
        (Version & EFI_ACPI_TABLE_VERSION_2_0 && AcpiSupportInstance->Facs3)  ||
        (Version & EFI_ACPI_TABLE_VERSION_3_0 && AcpiSupportInstance->Facs3)  ||
        (Version & EFI_ACPI_TABLE_VERSION_4_0 && AcpiSupportInstance->Facs4)  ||
        (Version & EFI_ACPI_TABLE_VERSION_5_0 && AcpiSupportInstance->Facs4)  ||
        (Version & EFI_ACPI_TABLE_VERSION_6_0 && AcpiSupportInstance->Facs4)
        ) {
      gBS->FreePages (CurrentTableList->PageAddress, CurrentTableList->NumberOfPages);
      gBS->FreePool (CurrentTableList);

      return EFI_ABORTED;
    }
    //
    // FACS is referenced by FADT and is not part of RSDT
    //
    AddToRsdt = FALSE;

    //
    // Add the table to the appropriate table version
    //
    if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
      //
      // Save a pointer to the table
      //
      AcpiSupportInstance->Facs1 = (EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *) CurrentTableList->Table;

      //
      // If FADT already exists, update table pointers.
      //
      if (AcpiSupportInstance->Fadt1) {
        AcpiSupportInstance->Fadt1->FirmwareCtrl = (UINT32) (UINTN) AcpiSupportInstance->Facs1;

        //
        // Checksum FADT table
        //
        AcpiPlatformChecksum (
          AcpiSupportInstance->Fadt1,
          AcpiSupportInstance->Fadt1->Header.Length,
          OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
          Checksum)
          );
      }
    }

    if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
      //
      // Save a pointer to the table
      //
      if ((Version & EFI_ACPI_TABLE_VERSION_4_0) || (Version & EFI_ACPI_TABLE_VERSION_5_0) || (Version & EFI_ACPI_TABLE_VERSION_6_0)) {
        AcpiSupportInstance->Facs4 = (EFI_ACPI_4_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *) CurrentTableList->Table;
      } else {
        AcpiSupportInstance->Facs3 = (EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *) CurrentTableList->Table;
      }
      //
      // If FADT already exists, update table pointers.
      //
      if (AcpiSupportInstance->Fadt3) {
        if ((Version & EFI_ACPI_TABLE_VERSION_4_0) || (Version & EFI_ACPI_TABLE_VERSION_5_0) || (Version & EFI_ACPI_TABLE_VERSION_6_0)) {
          if ((UINTN)AcpiSupportInstance->Facs4 < 0x100000000LL) {
            AcpiSupportInstance->Fadt3->FirmwareCtrl = (UINT32) (UINTN) AcpiSupportInstance->Facs4;
            Buffer64 = 0;
          } else {
            AcpiSupportInstance->Fadt3->FirmwareCtrl = 0;
            Buffer64 = (UINT64) (UINTN) AcpiSupportInstance->Facs4;
          }
        } else {
          AcpiSupportInstance->Fadt3->FirmwareCtrl  = (UINT32) (UINTN) AcpiSupportInstance->Facs3;
          Buffer64 = (UINT64) (UINTN) AcpiSupportInstance->Facs3;
        }

        CopyMem (
          &AcpiSupportInstance->Fadt3->XFirmwareCtrl,
          &Buffer64,
          sizeof (UINT64)
          );

        //
        // Checksum FADT table
        //
        AcpiPlatformChecksum (
          AcpiSupportInstance->Fadt3,
          AcpiSupportInstance->Fadt3->Header.Length,
          OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
          Checksum)
          );
      }
    }

    break;

  case EFI_ACPI_1_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
    //
    // Check that the table has not been previously added.
    //
    if ((Version & EFI_ACPI_TABLE_VERSION_1_0B && AcpiSupportInstance->Dsdt1) ||
        (Version & EFI_ACPI_TABLE_VERSION_2_0 && AcpiSupportInstance->Dsdt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_3_0 && AcpiSupportInstance->Dsdt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_4_0 && AcpiSupportInstance->Dsdt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_5_0 && AcpiSupportInstance->Dsdt3) ||
        (Version & EFI_ACPI_TABLE_VERSION_6_0 && AcpiSupportInstance->Dsdt3)
        ) {
      gBS->FreePages (CurrentTableList->PageAddress, CurrentTableList->NumberOfPages);
      gBS->FreePool (CurrentTableList);
      return EFI_ABORTED;
    }
    //
    // DSDT is referenced by FADT and is not part of RSDT
    //
    AddToRsdt = FALSE;

    //
    // Add the table to the appropriate table version
    //
    if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
      //
      // Save a pointer to the table
      //
      AcpiSupportInstance->Dsdt1 = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTableList->Table;

      //
      // If FADT already exists, update table pointers.
      //
      if (AcpiSupportInstance->Fadt1) {
        AcpiSupportInstance->Fadt1->Dsdt = (UINT32) (UINTN) AcpiSupportInstance->Dsdt1;

        //
        // Checksum FADT table
        //
        AcpiPlatformChecksum (
          AcpiSupportInstance->Fadt1,
          AcpiSupportInstance->Fadt1->Header.Length,
          OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
          Checksum)
          );
      }
    }

    if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
      //
      // Save a pointer to the table
      //
      AcpiSupportInstance->Dsdt3 = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTableList->Table;

      //
      // If FADT already exists, update table pointers.
      //
      if (AcpiSupportInstance->Fadt3) {
        AcpiSupportInstance->Fadt3->Dsdt  = (UINT32) (UINTN) AcpiSupportInstance->Dsdt3;
        Buffer64                          = (UINT64) (UINTN) AcpiSupportInstance->Dsdt3;
        CopyMem (
          &AcpiSupportInstance->Fadt3->XDsdt,
          &Buffer64,
          sizeof (UINT64)
          );

        //
        // Checksum FADT table
        //
        AcpiPlatformChecksum (
          AcpiSupportInstance->Fadt3,
          AcpiSupportInstance->Fadt3->Header.Length,
          OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
          Checksum)
          );
      }
    }
    //
    // Checksum the table
    //
    if (Checksum) {
      AcpiPlatformChecksum (
        CurrentTableList->Table,
        CurrentTableList->Table->Length,
        OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
        Checksum)
        );
    }
    break;

  default:
    //
    // Checksum the table
    //
    if (Checksum) {
      AcpiPlatformChecksum (
        CurrentTableList->Table,
        CurrentTableList->Table->Length,
        OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
        Checksum)
        );
    }
    break;
  }
  //
  // Add the table to the current list of tables
  //
  InsertTailList (&AcpiSupportInstance->TableList, &CurrentTableList->Link);

  //
  // Add the table to RSDT and/or XSDT table entry lists.
  //
  //
  // Add to ACPI 1.0b table tree
  //
  if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
    if (AddToRsdt) {
      //
      // If the table number exceed the gEfiAcpiMaxNumTables, enlarge the table buffer
      // And make sure the space of one table to reserve
      //
      if (AcpiSupportInstance->NumberOfTableEntries1 + 1 >= mEfiAcpiMaxNumTables) {
        Status = ReallocateAcpiTableBuffer (AcpiSupportInstance);
        ASSERT_EFI_ERROR (Status);
      }
      CurrentRsdtEntry = (UINT32 *)
        (
          (UINT8 *) AcpiSupportInstance->Rsdt1 +
          sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
          AcpiSupportInstance->NumberOfTableEntries1 *
          sizeof (UINT32)
        );

      //
      // Add entry to the RSDT unless its the FACS or DSDT
      //
      *CurrentRsdtEntry = (UINT32) (UINTN) CurrentTableList->Table;

      //
      // Update RSDT length
      //
      AcpiSupportInstance->Rsdt1->Length = AcpiSupportInstance->Rsdt1->Length + sizeof (UINT32);

      AcpiSupportInstance->NumberOfTableEntries1++;
    }
  }
  //
  // Add to ACPI 2.0/3.0  table tree
  //
  if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
    if (AddToRsdt) {
       //
       // If the table number exceed the gEfiAcpiMaxNumTables, enlarge the table buffer
       // And make sure the space of one table to reserve
       //
       if (AcpiSupportInstance->NumberOfTableEntries3 + 1 >= mEfiAcpiMaxNumTables) {
         Status = ReallocateAcpiTableBuffer (AcpiSupportInstance);
         ASSERT_EFI_ERROR (Status);
       }
      //
      // At this time, it is assumed that RSDT and XSDT maintain parallel lists of tables.
      // If it becomes necessary to maintain separate table lists, changes will be required.
      //
      CurrentRsdtEntry = (UINT32 *)
        (
          (UINT8 *) AcpiSupportInstance->Rsdt3 +
          sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
          AcpiSupportInstance->NumberOfTableEntries3 *
          sizeof (UINT32)
        );

      //
      // This pointer must not be directly dereferenced as the XSDT entries may not
      // be 64 bit aligned resulting in a possible fault.  Use CopyMem to update.
      //
      CurrentXsdtEntry = (VOID *)
        (
          (UINT8 *) AcpiSupportInstance->Xsdt +
          sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
          AcpiSupportInstance->NumberOfTableEntries3 *
          sizeof (UINT64)
        );

      //
      // Add entry to the RSDT
      //
      *CurrentRsdtEntry = (UINT32) (UINTN) CurrentTableList->Table;

      //
      // Update RSDT length
      //
      AcpiSupportInstance->Rsdt3->Length = AcpiSupportInstance->Rsdt3->Length + sizeof (UINT32);

      //
      // Add entry to XSDT, XSDT expects 64 bit pointers, but
      // the table pointers in XSDT are not aligned on 8 byte boundary.
      //
      Buffer64 = (UINT64) (UINTN) CurrentTableList->Table;
      CopyMem (
        CurrentXsdtEntry,
        &Buffer64,
        sizeof (UINT64)
        );

      //
      // Update length
      //
      AcpiSupportInstance->Xsdt->Length = AcpiSupportInstance->Xsdt->Length + sizeof (UINT64);

      AcpiSupportInstance->NumberOfTableEntries3++;
    }
  }

  ChecksumCommonTables (AcpiSupportInstance);
  return EFI_SUCCESS;
}

/**
 This function finds the table specified by the handle and returns a pointer to it.
 If the handle is not found, EFI_NOT_FOUND is returned and the contents of Table are
 undefined.

 @param [in]   Handle           Table to find.
 @param [in]   TableList        Table list to search
 @param [out]  Table            Pointer to table found.

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_NOT_FOUND          No table found matching the handle specified.

**/
EFI_STATUS
FindTableByHandle (
  IN UINTN                                Handle,
  IN LIST_ENTRY                           *TableList,
  OUT EFI_ACPI_TABLE_LIST                 **Table
  )
{
  LIST_ENTRY          *CurrentLink;
  EFI_ACPI_TABLE_LIST *CurrentTable;

  //
  // Check for invalid input parameters
  //
  ASSERT (Table);

  //
  // Find the table
  //
  CurrentLink = TableList->ForwardLink;

  while (CurrentLink != TableList) {
    CurrentTable = EFI_ACPI_TABLE_LIST_FROM_LINK (CurrentLink);
    if (CurrentTable->Handle == Handle) {
      //
      // Found handle, so return this table.
      //
      *Table = CurrentTable;
      return EFI_SUCCESS;
    }

    CurrentLink = CurrentLink->ForwardLink;
  }
  //
  // Table not found
  //
  return EFI_NOT_FOUND;
}

/**
 This function removes a basic table from the RSDT and/or XSDT.
 For Acpi 1.0 tables, pass in the Rsdt.
 For Acpi 2.0 tables, pass in both Rsdt and Xsdt.

 @param [in, out] Table         Pointer to table found.
 @param [in, out] NumberOfTableEntries  Current number of table entries in the RSDT/XSDT
 @param [in, out] Rsdt          Pointer to the RSDT to remove from
 @param [in, out] Xsdt          Pointer to the Xsdt to remove from

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_INVALID_PARAMETER  The table was not found in both Rsdt and Xsdt.

**/
EFI_STATUS
RemoveTableFromRsdt (
  IN OUT EFI_ACPI_TABLE_LIST              * Table,
  IN OUT UINTN                            *NumberOfTableEntries,
  IN OUT EFI_ACPI_DESCRIPTION_HEADER      * Rsdt,
  IN OUT EFI_ACPI_DESCRIPTION_HEADER      * Xsdt OPTIONAL
  )
{
  UINT32  *CurrentRsdtEntry;
  VOID    *CurrentXsdtEntry;
  UINT64  CurrentTablePointer64;
  UINTN   i;

  //
  // Check for invalid input parameters
  //
  ASSERT (Table);
  ASSERT (NumberOfTableEntries);
  ASSERT (Rsdt);

  //
  // Find the table entry in the RSDT and XSDT
  //
  for (i = 0; i < *NumberOfTableEntries; i++) {
    //
    // At this time, it is assumed that RSDT and XSDT maintain parallel lists of tables.
    // If it becomes necessary to maintain separate table lists, changes will be required.
    //
    CurrentRsdtEntry = (UINT32 *) ((UINT8 *) Rsdt + sizeof (EFI_ACPI_DESCRIPTION_HEADER) + i * sizeof (UINT32));
    if (Xsdt) {
      //
      // This pointer must not be directly dereferenced as the XSDT entries may not
      // be 64 bit aligned resulting in a possible fault.  Use CopyMem to update.
      //
      CurrentXsdtEntry = (VOID *) ((UINT8 *) Xsdt + sizeof (EFI_ACPI_DESCRIPTION_HEADER) + i * sizeof (UINT64));

      //
      // Read the entry value out of the XSDT
      //
      CopyMem (&CurrentTablePointer64, CurrentXsdtEntry, sizeof (UINT64));
    } else {
      //
      // Initialize to NULL
      //
      CurrentXsdtEntry      = 0;
      CurrentTablePointer64 = 0;
    }
    //
    // Check if we have found the corresponding entry in both RSDT and XSDT
    //
    if (*CurrentRsdtEntry == (UINT32) (UINTN) Table->Table &&
        (!Xsdt || CurrentTablePointer64 == (UINT64) (UINTN) Table->Table)
        ) {
      //
      // Found entry, so copy all following entries and shrink table
      // We actually copy all + 1 to copy the initialized value of memory over
      // the last entry.
      //
      CopyMem (CurrentRsdtEntry, CurrentRsdtEntry + 1, (*NumberOfTableEntries - i) * sizeof (UINT32));
      Rsdt->Length = Rsdt->Length - sizeof (UINT32);
      if (Xsdt) {
        CopyMem (CurrentXsdtEntry, ((UINT64 *) CurrentXsdtEntry) + 1, (*NumberOfTableEntries - i) * sizeof (UINT64));
        Xsdt->Length = Xsdt->Length - sizeof (UINT64);
      }
      break;
    } else if (i + 1 == *NumberOfTableEntries) {
      //
      // At the last entry, and table not found
      //
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // Checksum the tables
  //
  AcpiPlatformChecksum (
    Rsdt,
    Rsdt->Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
    Checksum)
    );

  if (Xsdt) {
    AcpiPlatformChecksum (
      Xsdt,
      Xsdt->Length,
      OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
      Checksum)
      );
  }
  //
  // Decrement the number of tables
  //
  (*NumberOfTableEntries)--;

  return EFI_SUCCESS;
}

/**
 This function removes a table and frees any associated memory.

 @param [in]   AcpiSupportInstance  Instance of the protocol.
 @param [in]   Version          Version(s) to delete.
 @param [in, out] Table         Pointer to table found.

 @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
DeleteTable (
  IN EFI_ACPI_SUPPORT_INSTANCE            *AcpiSupportInstance,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN OUT EFI_ACPI_TABLE_LIST              *Table
  )
{
  UINT32  CurrentTableSignature;
  BOOLEAN RemoveFromRsdt;

  if (AcpiSupportInstance == NULL || Table == NULL) {
    //
    // Check for invalid input parameters
    //
    ASSERT (AcpiSupportInstance);
    ASSERT (Table);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Init locals
  //
  RemoveFromRsdt        = TRUE;

  if (Table->Table != NULL) {

    CurrentTableSignature = ((EFI_ACPI_COMMON_HEADER *) Table->Table)->Signature;

    //
    // Basic tasks to accomplish delete are:
    //   Determine removal requirements (in RSDT/XSDT or not)
    //   Remove entry from RSDT/XSDT
    //   Remove any table references to the table
    //   If no one is using the table
    //      Free the table (removing pointers from private data and tables)
    //      Remove from list
    //      Free list structure
    //
    //
    // Determine if this table is in the RSDT or XSDT
    //
    if ((CurrentTableSignature == EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) ||
        (CurrentTableSignature == EFI_ACPI_2_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) ||
        (CurrentTableSignature == EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE)
        ) {
      RemoveFromRsdt = FALSE;
    }
    //
    // We don't remove the FADT in the standard way because some
    // OS expect the FADT to be early in the table list.
    // So we always put it as the first element in the list.
    //
    if (CurrentTableSignature == EFI_ACPI_1_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE) {
      RemoveFromRsdt = FALSE;
    }

    //
    // Remove the table from RSDT and XSDT
    // This is a basic table, remove it from any lists and the Rsdt and/or Xsdt
    //
    if (Version & EFI_ACPI_TABLE_VERSION_NONE & Table->Version) {
      //
      // Remove this version from the table
      //
      Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_NONE;
    }

    if (Version & EFI_ACPI_TABLE_VERSION_1_0B & Table->Version) {
      //
      // Remove this version from the table
      //
      Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_1_0B;

      //
      // Remove from Rsdt.  We don't care about the return value because it is
      // acceptable for the table to not exist in Rsdt.
      // We didn't add some tables so we don't remove them.
      //
      if (RemoveFromRsdt) {
        RemoveTableFromRsdt (
          Table,
          &AcpiSupportInstance->NumberOfTableEntries1,
          AcpiSupportInstance->Rsdt1,
          NULL
          );
      }
    }

    if ((Version & EFI_ACPI_TABLE_VERSION_2_0 & Table->Version) ||
        (Version & EFI_ACPI_TABLE_VERSION_3_0 & Table->Version) ||
        (Version & EFI_ACPI_TABLE_VERSION_4_0 & Table->Version) ||
        (Version & EFI_ACPI_TABLE_VERSION_5_0 & Table->Version) ||
        (Version & EFI_ACPI_TABLE_VERSION_6_0 & Table->Version)) {

      //
      // Remove this version from the table
      //
      if (Version & EFI_ACPI_TABLE_VERSION_2_0 & Table->Version) {
        Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_2_0;
      }
      if (Version & EFI_ACPI_TABLE_VERSION_3_0 & Table->Version) {
        Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_3_0;
      }
      if (Version & EFI_ACPI_TABLE_VERSION_4_0 & Table->Version) {
        Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_4_0;
      }
      if (Version & EFI_ACPI_TABLE_VERSION_5_0 & Table->Version) {
        Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_5_0;
      }
      if (Version & EFI_ACPI_TABLE_VERSION_6_0 & Table->Version) {
        Table->Version = Table->Version &~EFI_ACPI_TABLE_VERSION_6_0;
      }
      //
      // Remove from Rsdt and Xsdt.  We don't care about the return value
      // because it is acceptable for the table to not exist in Rsdt/Xsdt.
      // We didn't add some tables so we don't remove them.
      //
      if (RemoveFromRsdt) {
        RemoveTableFromRsdt (
          Table,
          &AcpiSupportInstance->NumberOfTableEntries3,
          AcpiSupportInstance->Rsdt3,
          AcpiSupportInstance->Xsdt
          );
      }
    }
    //
    // Free the table, clean up any dependent tables and our private data pointers.
    //
    switch (Table->Table->Signature) {

    case EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
      if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
        AcpiSupportInstance->Fadt1 = NULL;
      }

      if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
        AcpiSupportInstance->Fadt3 = NULL;
      }

      if (Version & EFI_ACPI_TABLE_VERSION_5_0) {
        AcpiSupportInstance->Fadt5 = NULL;
      }
      
      if (Version & EFI_ACPI_TABLE_VERSION_6_0) {
        AcpiSupportInstance->Fadt6 = NULL;
      }

      break;

    case EFI_ACPI_3_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
      if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
        AcpiSupportInstance->Facs1 = NULL;

        //
        // Update FADT table pointers
        //
        if (AcpiSupportInstance->Fadt1) {
          AcpiSupportInstance->Fadt1->FirmwareCtrl = 0;

          //
          // Checksum table
          //
          AcpiPlatformChecksum (
            AcpiSupportInstance->Fadt1,
            AcpiSupportInstance->Fadt1->Header.Length,
            OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
            Checksum)
            );
        }
      }

      if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
        if ((Version & EFI_ACPI_TABLE_VERSION_4_0) || (Version & EFI_ACPI_TABLE_VERSION_5_0) || (Version & EFI_ACPI_TABLE_VERSION_6_0)) {
          AcpiSupportInstance->Facs4 = NULL;
        } else {
          AcpiSupportInstance->Facs3 = NULL;
        }
        //
        // Update FADT table pointers
        //
        if (AcpiSupportInstance->Fadt3) {
          AcpiSupportInstance->Fadt3->FirmwareCtrl = 0;
          ZeroMem (&AcpiSupportInstance->Fadt3->XFirmwareCtrl, sizeof (UINT64));

          //
          // Checksum table
          //
          AcpiPlatformChecksum (
            AcpiSupportInstance->Fadt3,
            AcpiSupportInstance->Fadt3->Header.Length,
            OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
            Checksum)
            );
        }
      }
      break;

    case EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      if (Version & EFI_ACPI_TABLE_VERSION_1_0B) {
        AcpiSupportInstance->Dsdt1 = NULL;

        //
        // Update FADT table pointers
        //
        if (AcpiSupportInstance->Fadt1) {
          AcpiSupportInstance->Fadt1->Dsdt = 0;

          //
          // Checksum table
          //
          AcpiPlatformChecksum (
            AcpiSupportInstance->Fadt1,
            AcpiSupportInstance->Fadt1->Header.Length,
            OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
            Checksum)
            );
        }
      }


      if (Version & EFI_ACPI_TABLE_VERSION_2_0 || Version & EFI_ACPI_TABLE_VERSION_3_0 || Version & EFI_ACPI_TABLE_VERSION_4_0 || Version & EFI_ACPI_TABLE_VERSION_5_0 || Version & EFI_ACPI_TABLE_VERSION_6_0) {
        AcpiSupportInstance->Dsdt3 = NULL;

        //
        // Update FADT table pointers
        //
        if (AcpiSupportInstance->Fadt3) {
          AcpiSupportInstance->Fadt3->Dsdt = 0;
          ZeroMem (&AcpiSupportInstance->Fadt3->XDsdt, sizeof (UINT64));

          //
          // Checksum table
          //
          AcpiPlatformChecksum (
            AcpiSupportInstance->Fadt3,
            AcpiSupportInstance->Fadt3->Header.Length,
            OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
            Checksum)
            );
        }
      }
      break;

    default:
      //
      // Do nothing
      //
      break;
    }
  }
  //
  // If no version is using this table anymore, remove and free list entry.
  //
  if (!Table->Version) {
    //
    // Free the Table
    //
    gBS->FreePages (Table->PageAddress, Table->NumberOfPages);
    RemoveEntryList (&(Table->Link));
    gBS->FreePool (Table);
  }
  //
  // Done
  //
  return EFI_SUCCESS;
}

/**
 This function finds and removes the table specified by the handle.

 @param [in]   AcpiSupportInstance  Instance of the protocol.
 @param [in]   Version          Bitmask of which versions to remove.
 @param [in]   Handle           Table to remove.

 @retval EFI_SUCCESS            The function completed successfully.
 @retval EFI_ABORTED            An error occurred.
 @retval EFI_NOT_FOUND          Handle not found in table list.

**/
EFI_STATUS
RemoveTableFromList (
  IN EFI_ACPI_SUPPORT_INSTANCE            *AcpiSupportInstance,
  IN EFI_ACPI_TABLE_VERSION               Version,
  IN UINTN                                Handle
  )
{
  EFI_ACPI_TABLE_LIST *Table;
  EFI_STATUS          Status;

  //
  // Check for invalid input parameters
  //
  ASSERT (AcpiSupportInstance);

  //
  // Find the table
  //
  Status = FindTableByHandle (
             Handle,
             &AcpiSupportInstance->TableList,
             &Table
             );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }
  //
  // Remove the table
  //
  Status = DeleteTable (AcpiSupportInstance, Version, Table);
  if (EFI_ERROR (Status)) {
    return EFI_ABORTED;
  }
  //
  // Completed successfully
  //
  return EFI_SUCCESS;
}

/**
 This function calculates and updates an UINT8 checksum.

 @param [in]   Buffer           Pointer to buffer to checksum
 @param [in]   Size             Number of bytes to checksum
 @param [in]   ChecksumOffset   Offset to place the checksum result in

 @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
AcpiPlatformChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  )
{
  UINT8 Sum;
  UINT8 *Ptr;

  Sum = 0;
  //
  // Initialize pointer
  //
  Ptr = Buffer;

  //
  // set checksum to 0 first
  //
  Ptr[ChecksumOffset] = 0;

  //
  // add all content of buffer
  //
  while (Size--) {
    Sum = (UINT8) (Sum + (*Ptr++));
  }
  //
  // set checksum
  //
  Ptr                 = Buffer;
  Ptr[ChecksumOffset] = (UINT8) (0xff - Sum + 1);

  return EFI_SUCCESS;
}

/**
 Checksum all versions of the common tables, RSDP, RSDT, XSDT.

 @param [in, out] AcpiSupportInstance  Protocol instance private data.

 @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
ChecksumCommonTables (
  IN OUT EFI_ACPI_SUPPORT_INSTANCE                 *AcpiSupportInstance
  )
{
  //
  // RSDP ACPI 1.0 checksum for 1.0 table.  This is only the first 20 bytes of the structure
  //
  AcpiPlatformChecksum (
    AcpiSupportInstance->Rsdp1,
    sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER),
    OFFSET_OF (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER,
    Checksum)
    );

  //
  // RSDP ACPI 1.0 checksum for 2.0/3.0 table.  This is only the first 20 bytes of the structure
  //
  AcpiPlatformChecksum (
    AcpiSupportInstance->Rsdp3,
    sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER),
    OFFSET_OF (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER,
    Checksum)
    );

  //
  // RSDP ACPI 2.0/3.0 checksum, this is the entire table
  //
  AcpiPlatformChecksum (
    AcpiSupportInstance->Rsdp3,
    sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER),
    OFFSET_OF (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER,
    ExtendedChecksum)
    );

  //
  // RSDT checksums
  //
  AcpiPlatformChecksum (
    AcpiSupportInstance->Rsdt1,
    AcpiSupportInstance->Rsdt1->Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
    Checksum)
    );

  AcpiPlatformChecksum (
    AcpiSupportInstance->Rsdt3,
    AcpiSupportInstance->Rsdt3->Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
    Checksum)
    );

  //
  // XSDT checksum
  //
  AcpiPlatformChecksum (
    AcpiSupportInstance->Xsdt,
    AcpiSupportInstance->Xsdt->Length,
    OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER,
    Checksum)
    );

  return EFI_SUCCESS;
}
//
// Protocol constructor
//
/**
 Constructor for the ACPI support protocol.  Initializes instance
 data.

 @param        AcpiSupportInstance  Instance to construct

 @retval EFI_SUCCESS            Instance initialized.
 @retval EFI_OUT_OF_RESOURCES   Unable to allocate required resources.

**/
EFI_STATUS
AcpiSupportAcpiSupportConstructor (
  EFI_ACPI_SUPPORT_INSTANCE                 *AcpiSupportInstance
  )
{
  EFI_STATUS            Status;
  UINT64                CurrentData;
  UINTN                 TotalSize;
  UINT8                 *Pointer;
  EFI_PHYSICAL_ADDRESS  PageAddress;

  //
  // Check for invalid input parameters
  //
  ASSERT (AcpiSupportInstance);

  InitializeListHead (&AcpiSupportInstance->TableList);
  AcpiSupportInstance->CurrentHandle              = 1;
  AcpiSupportInstance->AcpiSupport.GetAcpiTable   = GetAcpiTable;
  AcpiSupportInstance->AcpiSupport.SetAcpiTable   = SetAcpiTable;
  AcpiSupportInstance->AcpiSupport.PublishTables  = PublishTables;

  AcpiSupportInstance->AcpiTableProtocol.InstallAcpiTable   = InstallAcpiTable;
  AcpiSupportInstance->AcpiTableProtocol.UninstallAcpiTable = UninstallAcpiTable;
  SdtAcpiTableAcpiSdtConstructor (AcpiSupportInstance);

  //
  // Create RSDP, RSDT, XSDT structures
  // Allocate all buffers
  //
  TotalSize = sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER) +
      sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER) +
      sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
      EFI_ACPI_MAX_NUM_TABLES *
      sizeof (UINT32) +
      sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
      EFI_ACPI_MAX_NUM_TABLES *
      sizeof (UINT32) +
      sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
      EFI_ACPI_MAX_NUM_TABLES *
      sizeof (UINT32) +
      sizeof (EFI_ACPI_DESCRIPTION_HEADER) +
      EFI_ACPI_MAX_NUM_TABLES *
      sizeof (UINT64);

  //
  // Allocate memory in the lower 32 bit of address range for
  // compatibility with ACPI 1.0 OS.
  //
  // This is done because ACPI 1.0 pointers are 32 bit values.
  // ACPI 2.0 OS and all 64 bit OS must use the 64 bit ACPI table addresses.
  // There is no architectural reason these should be below 4GB, it is purely
  // for convenience of implementation that we force memory below 4GB.
  //
  PageAddress = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIReclaimMemory,
                  EFI_SIZE_TO_PAGES (TotalSize),
                  &PageAddress
                  );

  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Pointer = (UINT8 *) (UINTN) PageAddress;
  ZeroMem (Pointer, TotalSize);

  AcpiSupportInstance->Rsdp1 = (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER *) Pointer;
  Pointer += sizeof (EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
  AcpiSupportInstance->Rsdp3 = (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER *) Pointer;
  Pointer += sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER);

  AcpiSupportInstance->Rsdt1 = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;
  Pointer += (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + EFI_ACPI_MAX_NUM_TABLES * sizeof (UINT32));
  AcpiSupportInstance->Rsdt3 = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;
  Pointer += (sizeof (EFI_ACPI_DESCRIPTION_HEADER) + EFI_ACPI_MAX_NUM_TABLES * sizeof (UINT32));

  AcpiSupportInstance->Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *) Pointer;

  //
  // Initialize RSDP
  //
  CurrentData = EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE;
  CopyMem (&AcpiSupportInstance->Rsdp1->Signature, &CurrentData, sizeof (UINT64));
  CopyMem (AcpiSupportInstance->Rsdp1->OemId, EFI_ACPI_OEM_ID, 6);
  AcpiSupportInstance->Rsdp1->Reserved    = EFI_ACPI_RESERVED_BYTE;
  AcpiSupportInstance->Rsdp1->RsdtAddress = (UINT32) (UINTN) AcpiSupportInstance->Rsdt1;

  CurrentData = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER_SIGNATURE;
  CopyMem (&AcpiSupportInstance->Rsdp3->Signature, &CurrentData, sizeof (UINT64));
  CopyMem (AcpiSupportInstance->Rsdp3->OemId, EFI_ACPI_OEM_ID, 6);
  AcpiSupportInstance->Rsdp3->Revision    = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER_REVISION;
  AcpiSupportInstance->Rsdp3->RsdtAddress = (UINT32) (UINTN) AcpiSupportInstance->Rsdt3;
  AcpiSupportInstance->Rsdp3->Length      = sizeof (EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER);
  CurrentData = (UINT64) (UINTN) AcpiSupportInstance->Xsdt;
  CopyMem (&AcpiSupportInstance->Rsdp3->XsdtAddress, &CurrentData, sizeof (UINT64));
  SetMem (AcpiSupportInstance->Rsdp3->Reserved, 3, EFI_ACPI_RESERVED_BYTE);

  //
  // Initialize Rsdt
  //
  // Note that we "reserve" one entry for the FADT so it can always be
  // at the beginning of the list of tables.  Some OS don't seem
  // to find it correctly if it is too far down the list.
  //
  AcpiSupportInstance->Rsdt1->Signature = EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE;
  AcpiSupportInstance->Rsdt1->Length    = sizeof (EFI_ACPI_DESCRIPTION_HEADER);
  AcpiSupportInstance->Rsdt1->Revision  = EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_TABLE_REVISION;
  CopyMem (AcpiSupportInstance->Rsdt1->OemId, EFI_ACPI_OEM_ID, 6);
  CurrentData = EFI_ACPI_OEM_TABLE_ID;
  CopyMem (&AcpiSupportInstance->Rsdt1->OemTableId, &CurrentData, sizeof (UINT64));
  AcpiSupportInstance->Rsdt1->OemRevision     = EFI_ACPI_OEM_REVISION;
  AcpiSupportInstance->Rsdt1->CreatorId       = EFI_ACPI_CREATOR_ID;
  AcpiSupportInstance->Rsdt1->CreatorRevision = EFI_ACPI_CREATOR_REVISION;
  //
  // We always reserve first one for FADT
  //
  AcpiSupportInstance->NumberOfTableEntries1  = 1;
  AcpiSupportInstance->Rsdt1->Length          = AcpiSupportInstance->Rsdt1->Length + sizeof(UINT32);

  AcpiSupportInstance->Rsdt3->Signature       = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE;
  AcpiSupportInstance->Rsdt3->Length          = sizeof (EFI_ACPI_DESCRIPTION_HEADER);
  AcpiSupportInstance->Rsdt3->Revision        = EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_TABLE_REVISION;
  CopyMem (AcpiSupportInstance->Rsdt3->OemId, EFI_ACPI_OEM_ID, 6);
  CurrentData = EFI_ACPI_OEM_TABLE_ID;
  CopyMem (&AcpiSupportInstance->Rsdt3->OemTableId, &CurrentData, sizeof (UINT64));
  AcpiSupportInstance->Rsdt3->OemRevision     = EFI_ACPI_OEM_REVISION;
  AcpiSupportInstance->Rsdt3->CreatorId       = EFI_ACPI_CREATOR_ID;
  AcpiSupportInstance->Rsdt3->CreatorRevision = EFI_ACPI_CREATOR_REVISION;
  //
  // We always reserve first one for FADT
  //
  AcpiSupportInstance->NumberOfTableEntries3  = 1;
  AcpiSupportInstance->Rsdt3->Length          = AcpiSupportInstance->Rsdt3->Length + sizeof(UINT32);

  //
  // Initialize Xsdt
  //
  AcpiSupportInstance->Xsdt->Signature  = EFI_ACPI_3_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE;
  AcpiSupportInstance->Xsdt->Length     = sizeof (EFI_ACPI_DESCRIPTION_HEADER);
  AcpiSupportInstance->Xsdt->Revision   = EFI_ACPI_3_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_REVISION;
  CopyMem (AcpiSupportInstance->Xsdt->OemId, EFI_ACPI_OEM_ID, 6);
  CurrentData = EFI_ACPI_OEM_TABLE_ID;
  CopyMem (&AcpiSupportInstance->Xsdt->OemTableId, &CurrentData, sizeof (UINT64));
  AcpiSupportInstance->Xsdt->OemRevision      = EFI_ACPI_OEM_REVISION;
  AcpiSupportInstance->Xsdt->CreatorId        = EFI_ACPI_CREATOR_ID;
  AcpiSupportInstance->Xsdt->CreatorRevision  = EFI_ACPI_CREATOR_REVISION;
  //
  // We always reserve first one for FADT
  //
  AcpiSupportInstance->Xsdt->Length           = AcpiSupportInstance->Xsdt->Length + sizeof(UINT64);

  ChecksumCommonTables (AcpiSupportInstance);

  //
  // Completed successfully
  //
  return EFI_SUCCESS;
}

/**
 Calculate the sum of all elements in a buffer in unit of UINT8.
 During calculation, the carry bits are dropped.

 @param [in]   Buffer           Pointer to the buffer to carry out the sum operation.
 @param [in]   Length           The size, in bytes, of Buffer .

 @retval Sum                    The sum of Buffer with carry bits dropped during additions.

**/
UINT8
EFIAPI
CalculateSum8 (
  IN      CONST UINT8              *Buffer,
  IN      UINTN                   Length
  )
{
  UINT8     Sum;
  UINTN     Count;

  ASSERT (Buffer != NULL);
  ASSERT (Length <= (MAX_ADDRESS - ((UINTN) Buffer) + 1));

  for (Sum = 0, Count = 0; Count < Length; Count++) {
    Sum = (UINT8) (Sum + *(Buffer + Count));
  }

  return Sum;
}

/**
 Returns the two's complement checksum of all elements in a buffer
 of 8-bit values.

 @param [in]   Buffer           Pointer to the buffer to carry out the sum operation.
 @param [in]   Length           The size, in bytes, of Buffer .

 @retval Checksum               The 2's complement checksum of Buffer.

**/
UINT8
EFIAPI
CalculateCheckSum8 (
  IN      CONST UINT8              *Buffer,
  IN      UINTN                   Length
  )
{
  UINT8     CheckSum;

  CheckSum = CalculateSum8 (Buffer, Length);

  //
  // Return the checksum based on 2's complement.
  //
  return (UINT8) (0x100 - CheckSum);
}
