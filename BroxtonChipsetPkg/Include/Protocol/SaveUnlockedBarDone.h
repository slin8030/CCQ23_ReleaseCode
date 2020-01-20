/** @file

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

#ifndef _SAVE_UNLOCKED_BAR_DONE_PROTOCOL_H_
#define _SAVE_UNLOCKED_BAR_DONE_PROTOCOL_H_

#define SAVE_UNLOCKED_BAR_DONE_PROTOCOL_GUID \
  { \
    0x41aaad7f, 0x6edd, 0x4924, 0xbb, 0xb, 0xc3, 0xd6, 0xcc, 0xd8, 0xac, 0x46 \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID        gSaveUnlockedBarDoneProtocolGuid;

#endif
