/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _UN_LOCKED_BAR_HANDLE_H
#define _UN_LOCKED_BAR_HANDLE_H

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/PciIo.h>
#include <Protocol/SaveUnlockedBarDone.h>
#include <Protocol/PciEnumerationComplete.h>
#include <Guid/SaveUnLockedBarVariable.h>
#include <IndustryStandard/Pci.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include "ScAccess.h"

#define MEMORY_SPACE_INDICATOR  0
#define IO_SPACE_INDICATOR      1

//
// Prototypes
//
EFI_STATUS
UnLockedBarHandleEntryPoint (
  IN EFI_HANDLE              ImageHandle,
  IN EFI_SYSTEM_TABLE        *SystemTable
  );

BOOLEAN
IsSaveDynamicBar (
  EFI_PCI_IO_PROTOCOL    *PciIo,
  UINTN                  BarIndex
  );

EFI_STATUS
EFIAPI
SaveUnLockedBar (
  VOID
  );

#endif

