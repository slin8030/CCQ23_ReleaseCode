/** @file
  H2O Message Filter Protocol Header

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _H2O_MESSAGE_FILTER_PROTOCOL_H_
#define _H2O_MESSAGE_FILTER_PROTOCOL_H_

#include <Protocol/H2OFormBrowser.h>

#define H2O_MESSAGE_FILTER_PROTOCOL_GUID \
  { \
   0x326568a8, 0xef58, 0x492f, 0xb7, 0xa9, 0x50, 0x9e, 0x9e, 0xd1, 0x9a, 0x7b \
  }

typedef struct _H2O_MESSAGE_FILTER_PROTOCOL H2O_MESSAGE_FILTER_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *H2O_MESSAGE_FILTER_NOTIFY) (
  IN       H2O_MESSAGE_FILTER_PROTOCOL     *This,
  IN CONST H2O_DISPLAY_ENGINE_EVT          *Event
  );

typedef
VOID
(EFIAPI *H2O_MESSAGE_FILTER_IDLE) (
  IN       H2O_MESSAGE_FILTER_PROTOCOL     *This
  );

struct _H2O_MESSAGE_FILTER_PROTOCOL {
  H2O_MESSAGE_FILTER_NOTIFY           Notify;
  H2O_MESSAGE_FILTER_IDLE             Idle;
};

extern GUID gH2OMessageFilterProtocolGuid;

#endif
