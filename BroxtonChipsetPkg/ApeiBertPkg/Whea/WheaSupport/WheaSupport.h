/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  This file contains a 'Sample Driver' and is licensed as such
//  under the terms of your license agreement with Intel or your
//  vendor.  This file may be modified by the user, subject to
//  the additional terms of the license agreement
//

Copyright (c) 2009-2015 Intel Corporation.  All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license.  Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:
    WheaSupport.h

Abstract:
    This is an implementation of the Whea Support protocol.

-----------------------------------------------------------------------------*/


#ifndef _WHEA_SUPPORT_H_
#define _WHEA_SUPPORT_H_

//
// Statements that include other header files
//
#include <Library\UefiLib.h>
#include <Library\DebugLib.h>
#include <Library\MemoryAllocationLib.h>
#include <Library\WheaPlatformHooksLib.h>
#include <Library\UefiBootServicesTableLib.h>
#include <Uefi.h>
#include <IndustryStandard\WheaDefs.h>
#include "WheaPlatformDefs.h"

//
// Consumed protocols
//
#include <Protocol\AcpiTable.h>
//#include <Protocol\IsPlatformSupportWhea.h>


// Produced protocols
//
//#include <Protocol\WheaSupport.h>

//
// Data definitions & structures
//

typedef struct _WHEA_SUPPORT_INST {
  UINTN                                         Signature;
  EFI_ACPI_WHEA_BOOT_ERROR_TABLE                *Bert;
  UINTN                                         CurErrorStatusBlockSize;
  UINTN                                         ErrorStatusBlockSize;
  VOID                                          *ErrorStatusBlock;
  UINTN                                         ErrorLogAddressRangeSize;
  VOID                                          *ErrorLogAddressRange;
  UINTN                                         BootErrorRegionLen;
  VOID                                          *BootErrorRegion;
  BOOLEAN                                       TablesInstalled;
  UINTN                                         InstalledErrorInj;
} WHEA_SUPPORT_INST;

// WHEA support driver's structure definition
#define EFI_WHEA_SUPPORT_INST_SIGNATURE  SIGNATURE_32 ('B', 'E', 'R', 'T')

#define WHEA_SUPPORT_INST_FROM_THIS(a)   CR ( a,  WHEA_SUPPORT_INST,  WheaSupport, EFI_WHEA_SUPPORT_INST_SIGNATURE)

// Eswtemp
#define EFI_EVENT_SIGNAL_READY_TO_BOOT  0x0203

// Maximum buffer size for WHEA ACPI tables
#define MAX_BERT_SIZE               0x2000
#define MAX_ERROR_STATUS_SIZE       0x4000
#define MAX_ERROR_LOG_RANGE_SIZE    0x2000
#define MAX_BOOT_ERROR_LOG_SIZE     5296

#define WHEA_ACPI_VERSION_SUPPORT   (EFI_ACPI_TABLE_VERSION_3_0)

extern EFI_GUID         gEfiWheaFirmwareErrorSectionGuid;
extern EFI_GUID         gEfiWheaSupportProtocolGuid;

/**
  Entry point of the APEI BERT support driver.
  
  ImageHandle   EFI_HANDLE: A handle for the image that is initializing this driver
  SystemTable   EFI_SYSTEM_TABLE: A pointer to the EFI system table        

  @retval  EFI_SUCCESS:                       Driver initialized successfully
               EFI_LOAD_ERROR:                 Failed to Initialize or has been loaded 
               EFI_OUT_OF_RESOURCES:     Could not allocate needed resources
**/
EFI_STATUS
InstallWheaSupport (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
;

#endif

