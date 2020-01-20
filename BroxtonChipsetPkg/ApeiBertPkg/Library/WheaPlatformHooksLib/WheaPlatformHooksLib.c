/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/*++

Copyright (c) 2015, Intel Corporation.  All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license.  Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:
    WheaPlatformHooks.c
   
Abstract:
    Whea Platform hook functions and platform data, which needs to be 
    ported.

--*/


#include "Library\WheaPlatformHooksLib.h"
#include <Library\PciLib.h>
#include <Library\DebugLib.h>
#include <Library\UefiBootServicesTableLib.h>

//
// Data definitions & structures
//
#define EFI_ACPI_CREATOR_ID                 SIGNATURE_32 ('B', 'R', 'X', 'T')
#define EFI_ACPI_CREATOR_REVISION           0x00000001
#define EFI_ACPI_OEM_TABLE_ID               SIGNATURE_64 ('E', 'D', 'K', '2', ' ', ' ', ' ', ' ') // OEM table id 8 bytes long
#define EFI_WHEA_OEM_REVISION               0x0001;
#define NMI_STS_CNT                         0x61

// HW Error notification method used for FW first model
#define     ERROR_STATUS_GAS_DATADEF        {0, 64, 0, 0, 0x0000}
#define     SCI_ERROR_NOTIFY_DATADEF        {ERR_NOTIFY_SCI, sizeof(HW_NOTIFY_METHOD), 0, 0, 0, 0, 0, 0, 0}
#define     NMI_ERROR_NOTIFY_DATADEF        {ERR_NOTIFY_NMI, sizeof(HW_NOTIFY_METHOD), 0, 0, 0, 0, 0, 0, 0}

//
// Generic error HW error sources - Tip: list all FW First error sources
// Note: Used 1KB Error Status block for each error source
//
//  RelSrcId,   MaxErrData, ErrStsAddr,                Notify method,           Max ErrSts block Size
GENERIC_HW_ERROR_SOURCE         PlatformCeSrc = \
    {0xffff,    4096,       ERROR_STATUS_GAS_DATADEF,  SCI_ERROR_NOTIFY_DATADEF,  4096};
GENERIC_HW_ERROR_SOURCE         PlatformUeSrc = \
    {0xffff,    4096,       ERROR_STATUS_GAS_DATADEF,  NMI_ERROR_NOTIFY_DATADEF,  4096};

SYSTEM_GENERIC_ERROR_SOURCE         SysGenErrSources[] = {
//  Type,                     SrcId,  Flags, NumOfRecords, MaxSecPerRecord,   SourceInfo
  {GENERIC_ERROR_CORRECTED, 0,		0,       1,            1,                &PlatformCeSrc},
  {GENERIC_ERROR_FATAL,     0,		0,       1,            1,                &PlatformUeSrc},
};
UINTN NumSysGenericErrorSources     = (sizeof(SysGenErrSources)/sizeof(SYSTEM_GENERIC_ERROR_SOURCE));

//
// Native error sources - Tip: Errors that can be handled by OS directly
// Note: Used 1KB Error Status block for each error source
//
HW_NOTIFY_METHOD NotifyMthd ={0,sizeof(HW_NOTIFY_METHOD),0,0,0,0x9,0,0x9,0};

SYSTEM_NATIVE_ERROR_SOURCE          SysNativeErrSources[] = {
00
};
UINTN NumNativeErrorSources         = 0;
            //((SysNativeErrSources)/sizeof(SYSTEM_NATIVE_ERROR_SOURCE));


#define EFI_DIMM_ADDRESS_TO_GLDN(SlotNumber,Branch,Channel,Amb)  SlotNumber = \
                                                                (((PLATFORM_MAX_BRANCHES - 1 - Branch) << 3) + \
                                                                ((PLATFORM_MAX_CHANNELS - 1 - Channel) << 2) + \
                                                                Amb)

#define PLATFORM_MAX_BRANCHES     2
#define PLATFORM_MAX_CHANNELS     2

UINT32 mWheaPlatformType     = 0xFFFF;
UINT16 mWheaPlatformFlavor   = 0xFFFF;

//
// Update Platform & OEM ID's in Whea ACPI tables.
//
VOID
UpdateAcpiTableIds(
  EFI_ACPI_DESCRIPTION_HEADER     *TableHeader
  )
{
//
// Update the OEMID, Creator ID, and Creator revision.
//
  *(UINT32 *) (TableHeader->OemId)      = 'I' + ('N' << 8) + ('T' << 16) + ('E' << 24);
  *(UINT16 *) (TableHeader->OemId + 4)  = 'L' + (' ' << 8);
  TableHeader->CreatorId                = EFI_ACPI_CREATOR_ID;
  TableHeader->CreatorRevision          = EFI_ACPI_CREATOR_REVISION;

//
// Update OEM revision and OEM TABLE ID based on the platform/SKU
//
  TableHeader->OemRevision              = EFI_WHEA_OEM_REVISION;
  TableHeader->OemTableId               = EFI_ACPI_OEM_TABLE_ID;                           
}



