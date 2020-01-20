/** @file
  This file contains Initialization of the BDAT data structures for both
  v1.0 and v2.0 taken from v0.6 of "A Compatible Method to Access BIOS
  Data Structures".

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

//
//  Includes
//

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>

#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiSupport.h>

#include <Bdat.h>

#include "BdatDxe.h"

//[-start-160111-IB03090424-add]//
extern EFI_GUID gSsaResultGuid;
//[-end-160111-IB03090424-add]//

//
// Define macros to build data structure signatures from characters.
//
#ifndef EFI_SIGNATURE_16
#define EFI_SIGNATURE_16(A, B)        ((A) | (B << 8))
#endif
#ifndef EFI_SIGNATURE_32
#define EFI_SIGNATURE_32(A, B, C, D)  (EFI_SIGNATURE_16 (A, B) | (EFI_SIGNATURE_16 (C, D) << 16))
#endif
#ifndef EFI_SIGNATURE_64
#define EFI_SIGNATURE_64(A, B, C, D, E, F, G, H) \
  (EFI_SIGNATURE_32 (A, B, C, D) | ((UINT64) (EFI_SIGNATURE_32 (E, F, G, H)) << 32))
#endif

//
//  BDAT ACPI Description Table
//
EFI_BDAT_ACPI_DESCRIPTION_TABLE  mBadt = {
  //
  //  Common ACPI description table header.  This structure prefaces most ACPI tables.
  //
  EFI_SIGNATURE_32('B','D','A','T'),          // Signature
  sizeof (EFI_BDAT_ACPI_DESCRIPTION_TABLE),   // Length
  0x02,                                       // Revision  [02]
  0,                                          // Checksum
  ' ',                                        // OEM ID
  ' ',
  ' ',
  ' ',
  ' ',
  ' ',
  0,                                          // OEM Table ID
  0,                                          // OEM Revision [0x00000000]
  0,                                          // Creator ID
  0,                                          // Creator Revision
  //
  //  ACPI 3.0 Generic Address Space definition
  //
  0,                                          // System Memory Address Space ID
  0,
  0,
  0,
  EFI_BDAT_ACPI_POINTER,
};

UINTN   mBdatAcpiTableKey = 0;
BOOLEAN mBdatAcpiTableInstalled = FALSE;

EFI_STATUS
EFIAPI
InitializeBdatHeader (
     OUT        BDAT_STRUCTURE*  Bdat
)
{

  //
  //  Our only activity: Create, fill up and submit our BDAT HOB into the HOB list.
  //
  SetMem (Bdat, sizeof (BDAT_STRUCTURE), 0);

  //
  //  Create BIOS Data Signature
  //
  Bdat->bdatHeader.BiosDataSignature[0] = 'B';
  Bdat->bdatHeader.BiosDataSignature[1] = 'D';
  Bdat->bdatHeader.BiosDataSignature[2] = 'A';
  Bdat->bdatHeader.BiosDataSignature[3] = 'T';
  Bdat->bdatHeader.BiosDataSignature[4] = 'H';
  Bdat->bdatHeader.BiosDataSignature[5] = 'E';
  Bdat->bdatHeader.BiosDataSignature[6] = 'A';
  Bdat->bdatHeader.BiosDataSignature[7] = 'D';
  //
  //  Structure size
  //
  Bdat->bdatHeader.BiosDataStructSize = sizeof (BDAT_STRUCTURE);
  //
  //  CRC16
  //
  Bdat->bdatHeader.Crc16 = 0;
  //
  //  Primary Version
  //
  Bdat->bdatHeader.PrimaryVersion =  BDAT_PRIMARY_VER;
  //
  //  Secondary Version
  //
  Bdat->bdatHeader.SecondaryVersion =  BDAT_SECONDARY_VER;
  //
  //  Filling up Schema List
  //
  Bdat->BdatSchemas.SchemaListLength  = 0;
  Bdat->BdatSchemas.Year              = 2014;
  Bdat->BdatSchemas.Month             = 11;
  Bdat->BdatSchemas.Day               = 24;

  return EFI_SUCCESS;
}

/*++

Routine Description:

This function grabs memory information from the HOB and stores in a Bdat structure.

Arguments:

ImageHandle:   A handle for the image that is initializing this driver.
SystemTable:   A pointer to the EFI system table.

Returns:

EFI_SUCCESS:     Driver initialized successfully.

--*/
EFI_STATUS
EFIAPI
BdatDxeEntryPoint (
  IN  EFI_HANDLE       ImageHandle,
  IN  EFI_SYSTEM_TABLE *SystemTable
)
{
  EFI_STATUS                      Status              = EFI_SUCCESS;
  UINT16                          BdatSchemas         = 0;
  VOID                            *MmrcBdatHob        = NULL;
  VOID                            *MmrcBdatHobData    = NULL;
  VOID                            *EmmcBdatHobData    = NULL;
  UINT32                          BdatHobSize         = 0;
  UINT32                          MmrcBdatHobSize     = 0;
  UINT32                          EmmcBdatHobSize     = 0;
  EFI_ACPI_TABLE_PROTOCOL         *AcpiTableProtocol;
  EFI_ACPI_SUPPORT_PROTOCOL       *AcpiSupportProtocol;
  EFI_PHYSICAL_ADDRESS            BdatPageAddress;
  EFI_PHYSICAL_ADDRESS            CurrentAddress;
  UINTN                           Index;
  EFI_BDAT_ACPI_DESCRIPTION_TABLE *RecoveredTable;
  BDAT_STRUCTURE                  BdatHeader;
  BDAT_SCHEMA_HEADER_STRUCTURE    *SchemaHeaderPtr;
  BDAT_STRUCTURE                  *RecoveredBdat;
  UINTN                           TableHandle;
  EFI_ACPI_TABLE_VERSION          TableVersion;
  //
  //  Our activities:
  //      - Recover Bdat HOB
  //      - Construct our Bdat Protocol Interface
  //      - Create Bdat NV Variable
  //      - Add Bdat info into ACPI table
  //

  //
  //  Collecting Bdat/MemBdat HOB
  //
  InitializeBdatHeader(&BdatHeader);

  BdatHobSize = sizeof(BdatHeader);
  //
  //  Collecting MMRC Bdat HOB
  //
  MmrcBdatHob = GetHobList ();
  if ((MmrcBdatHob = GetNextGuidHob (&gSsaResultGuid, MmrcBdatHob)) != NULL) {
    MmrcBdatHobData = GET_GUID_HOB_DATA (MmrcBdatHob);
    MmrcBdatHobSize = GET_GUID_HOB_DATA_SIZE (MmrcBdatHob);
    DEBUG ((EFI_D_INFO, "MMRC BDAT HOB Data Pointer = %p, Hob length = %d, Data Size = %d...\n",
      MmrcBdatHobData,
      ((EFI_HOB_GUID_TYPE *) MmrcBdatHob)->Header.HobLength,
      MmrcBdatHobSize));
    BdatSchemas ++;
  }

  //
  //  Creating non-volatile storage space
  //
  BdatPageAddress = 0xFFFFFFFF;
  Status = gBS->AllocatePages (
    AllocateMaxAddress,
    EfiACPIMemoryNVS,
    EFI_SIZE_TO_PAGES (BdatHobSize + MmrcBdatHobSize + EmmcBdatHobSize + sizeof(BDAT_SCHEMA_HEADER_STRUCTURE)),
    &BdatPageAddress
    );
  mBadt.BdatGas.Address = (UINT64) BdatPageAddress;
  DEBUG ((EFI_D_INFO, "Allocated page address = 0x%x...\n", BdatPageAddress));

  //
  //  Updating BDAT ACPI Table
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid,
    NULL,
    (VOID **) &AcpiTableProtocol
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "LocateProtocol error: %r\n", Status));
    return Status;
  }
  DEBUG ((EFI_D_INFO, "AcpiTable Protocol located!!\n"));

  if (mBdatAcpiTableInstalled) {
    Status = AcpiTableProtocol->UninstallAcpiTable (
      AcpiTableProtocol,
      mBdatAcpiTableKey
      );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "UninstallAcpiTable error: %r\n", Status));
      return Status;
    }
    DEBUG ((EFI_D_INFO, "Old BDAT ACPI Table Uninstalled!!\n"));
  }

  Status = AcpiTableProtocol->InstallAcpiTable (
    AcpiTableProtocol,
    &mBadt,
    sizeof (EFI_BDAT_ACPI_DESCRIPTION_TABLE),
    &mBdatAcpiTableKey
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "InstallAcpiTable error: %r\n", Status));
    return Status;
  }

  mBdatAcpiTableInstalled = TRUE;
  DEBUG ((EFI_D_INFO, "New BDAT ACPI Table Installed!!\n"));

  //
  //  Transferring HOBs data to non-volatile space...
  //
  CurrentAddress = BdatPageAddress;
  CopyMem ((VOID *) (UINTN) CurrentAddress, (VOID *)&BdatHeader, BdatHobSize);
  CurrentAddress += BdatHobSize;
  SchemaHeaderPtr = (BDAT_SCHEMA_HEADER_STRUCTURE*) (UINTN) CurrentAddress;
  CurrentAddress += sizeof(BDAT_SCHEMA_HEADER_STRUCTURE);
  CopyMem ((VOID *) (UINTN)  CurrentAddress, MmrcBdatHobData, MmrcBdatHobSize);
  CurrentAddress += MmrcBdatHobSize;
  CopyMem ((VOID *) (UINTN)  BdatPageAddress, EmmcBdatHobData, EmmcBdatHobSize);

  SchemaHeaderPtr->DataSize = MmrcBdatHobSize + sizeof(BDAT_SCHEMA_HEADER_STRUCTURE);
  CopyMem((VOID*)&(SchemaHeaderPtr->SchemaId), (VOID*)(&gSsaResultGuid), sizeof(EFI_GUID));

  RecoveredBdat = (BDAT_STRUCTURE *) (VOID *) (UINTN) BdatPageAddress;
  RecoveredBdat->BdatSchemas.SchemaListLength = BdatSchemas;
  if (BdatSchemas >= 1) {
    RecoveredBdat->BdatSchemas.Schemas[0] = (UINT32) BdatHobSize;
  }
  if (BdatSchemas > 1) {
    RecoveredBdat->BdatSchemas.Schemas[1] = (UINT32) (RecoveredBdat->BdatSchemas.Schemas[0] + MmrcBdatHobSize);
  }
  RecoveredBdat->bdatHeader.BiosDataStructSize = (UINT32) (BdatHobSize + MmrcBdatHobSize + EmmcBdatHobSize + sizeof(BDAT_SCHEMA_HEADER_STRUCTURE));
  RecoveredBdat->bdatHeader.Crc16 = Crc16 ((INT8 *) RecoveredBdat, RecoveredBdat->bdatHeader.BiosDataStructSize);

  //
  //  Final step: Verifying we have the BDAT ACPI Table stored...
  //
  Status = gBS->LocateProtocol (
    &gEfiAcpiSupportProtocolGuid,
    NULL,
    (VOID **) &AcpiSupportProtocol
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "LocateProtocol error: %r\n", Status));
    return Status;
  }
  //
  // Locate table with matching ID
  //
  Index = 0;
  do {
    Status = AcpiSupportProtocol->GetAcpiTable (
      AcpiSupportProtocol,
      Index,
      (VOID **) &RecoveredTable,
      &TableVersion,
      &TableHandle
      );
    if (Status == EFI_NOT_FOUND) {
      break;
    }
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "GetAcpiTable error: r\n", Status));
      return Status;
    }
    DEBUG ((EFI_D_INFO, "ACPI Table signature = 0x%x...\n", RecoveredTable->Header.Signature));
    Index++;
  } while (RecoveredTable->Header.Signature != EFI_SIGNATURE_32('B','D','A','T'));

  ASSERT (RecoveredTable->Header.Signature == EFI_SIGNATURE_32('B','D','A','T'));

  if (RecoveredTable->Header.Signature != EFI_SIGNATURE_32('B','D','A','T')) {
    DEBUG ((EFI_D_ERROR, "BDAT signature error\n"));
    return EFI_NOT_FOUND;
  }

  //
  //  Adjusting size and CRC16 data inside BDAT Header...
  //
  DEBUG ((EFI_D_INFO, "RecoveredTable->BdatGas.Address = 0x%x...\n", RecoveredTable->BdatGas.Address));
  //
  //  Display some data for debugging
  //
  DEBUG ((EFI_D_INFO, "\nstruct RecoveredBdat->bdatHeader {\n"));
  DEBUG ((EFI_D_INFO, "  BiosDataSignature          "));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[0]));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[1]));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[2]));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[3]));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[4]));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[5]));
  DEBUG ((EFI_D_INFO, "%c", RecoveredBdat->bdatHeader.BiosDataSignature[6]));
  DEBUG ((EFI_D_INFO, "%c\n", RecoveredBdat->bdatHeader.BiosDataSignature[7]));
  DEBUG ((EFI_D_INFO, "  BiosDataStructSize         0x%x\n", RecoveredBdat->bdatHeader.BiosDataStructSize));
  DEBUG ((EFI_D_INFO, "  Crc16                      0x%x\n", RecoveredBdat->bdatHeader.Crc16));
  DEBUG ((EFI_D_INFO, "  PrimaryVersion             0x%x\n", RecoveredBdat->bdatHeader.PrimaryVersion));
  DEBUG ((EFI_D_INFO, "  SecondaryVersion           0x%x\n", RecoveredBdat->bdatHeader.SecondaryVersion));
  DEBUG ((EFI_D_INFO, "  OemOffset                  0x%x\n", RecoveredBdat->bdatHeader.OemOffset));
  DEBUG ((EFI_D_INFO, "} // bdatheader\n"));

  DEBUG ((EFI_D_INFO, "\nstruct RecoveredBdat->bdatSchemas {\n"));
  DEBUG ((EFI_D_INFO, "    SchemaListLength = %d\n", RecoveredBdat->BdatSchemas.SchemaListLength));
  DEBUG ((EFI_D_INFO, "} // bdatSchemas\n"));

  //
  //  Completed successfully
  //
  return EFI_SUCCESS;
}

/*++

Routine Description:

This function returns the CRC16 of the provided array.

Arguments:

Ptr:    The array to be processed.
Count:  Number of elements in the array.

Returns:

CRC16 of the provided array.

--*/
INT16
Crc16 (
  IN INT8  *Ptr,
  IN INT32 Count
  )
{
  INT16 crc, i;
  crc = 0;
  while (--Count >= 0) {
    crc = crc ^ (INT16) (int) * Ptr++ << 8;
    for (i = 0; i < 8; ++i) {
      if (crc & 0x8000) {
        crc = crc << 1 ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  return (crc & 0xFFFF);
}

