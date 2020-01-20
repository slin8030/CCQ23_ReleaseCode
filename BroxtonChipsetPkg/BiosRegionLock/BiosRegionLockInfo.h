/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _BIOS_REGION_LOCK_INFO_H_
#define _BIOS_REGION_LOCK_INFO_H_
#include <ScAccess.h>
#include <ChipsetSpiAccess.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>

#include <Protocol/BiosRegionLock.h>

#define MAX_BIOS_REGION_LOCK   5

typedef struct {
  UINTN  Base;
  UINTN  Length;
} BIOS_REGION_LOCK_BASE;

typedef struct {
  BIOS_REGION_LOCK_BASE      BiosLock[MAX_BIOS_REGION_LOCK];
  BIOS_REGION_LOCK_PROTOCOL  BiosRegionLock;
} BIOS_REGION_LOCK_INSTANCE;

#define BIOS_REGION_LOCK_INSTANCE_FROM_THIS(this)  BASE_CR (this, BIOS_REGION_LOCK_INSTANCE, BiosRegionLock)

EFI_STATUS
SetRegionByType (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN BIOS_REGION_TYPE            Type
);

EFI_STATUS
SetRegionByAddress (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN UINTN                       BaseAddress,
  IN UINTN                       Length 
);

EFI_STATUS
ClearRegionByType (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN BIOS_REGION_TYPE            Type
);

EFI_STATUS
ClearRegionByAddress (
  IN BIOS_REGION_LOCK_PROTOCOL  *This,
  IN UINTN                       BaseAddress,
  IN UINTN                       Length 
);

EFI_STATUS
Lock (
  IN BIOS_REGION_LOCK_PROTOCOL  *This
);

#endif //_BIOS_REGION_LOCK_INFO_H_
