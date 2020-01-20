/** @file
  This file contains definitions required for creation of SG Information data HOB.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _BIOS_FIRST_BOOT_HOB_GUID_H_
#define _BIOS_FIRST_BOOT_HOB_GUID_H_

#define BIOS_FIRST_BOOT_HOB_GUID \
  { \
    0x82b66042, 0x6ce2, 0x4662, 0xb7, 0x73, 0x4e, 0x52, 0x61, 0x51, 0x6a, 0x6e \
  }

extern EFI_GUID                               gBiosFirstBootHobGuid;

typedef struct _BIOS_FIRST_BOOT_HOB {
  UINT64  BufferAddress;
} BIOS_FIRST_BOOT_HOB;

#endif
