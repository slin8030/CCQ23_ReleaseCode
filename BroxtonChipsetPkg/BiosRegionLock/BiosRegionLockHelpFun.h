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

#ifndef _BIOS_REGION_LOCK_HELP_FUN_H_
#define _BIOS_REGION_LOCK_HELP_FUN_H_

//EFI_STATUS
//GetBiosRegionAddress (
//  IN  BIOS_REGION_TYPE   Type,
//  OUT UINTN             *Base,
//  OUT UINTN             *Length
//);

EFI_STATUS
MergeToBase (
  IN BIOS_REGION_LOCK_BASE  *BiosLock,
  IN UINTN                   Base,
  IN UINTN                   Length
);

EFI_STATUS
RemoveFromBase (
  IN BIOS_REGION_LOCK_BASE  *BiosLock,
  IN UINTN                   Base,
  IN UINTN                   Length
);

EFI_STATUS
ProgramRegister (
  IN BIOS_REGION_LOCK_BASE *BiosLock
);

#endif //_BIOS_REGION_LOCK_HELP_FUN_H_

