/** @file
  Definitions for EFI System Resource Table

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _INTEL_FOTA_CAPSULE_GUID_H_
#define _INTEL_FOTA_CAPSULE_GUID_H_

#include <Uefi.h>

#define INTEL_FOTA_CAPUSLE_GUID \
  { \
    0xb122a262, 0x3551, 0x4f48, { 0x88, 0x92, 0x55, 0xf6, 0xc0, 0x61, 0x42, 0x90 }  \
  }


extern EFI_GUID gIntelFotaCapsuleGuid;
#endif
