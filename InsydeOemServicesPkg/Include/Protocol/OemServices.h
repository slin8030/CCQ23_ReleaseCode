/** @file
  Definition for Oem Services Protocol.

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _OEM_SERVICES_PROTOCOL_H_
#define _OEM_SERVICES_PROTOCOL_H_

#define OEM_SERVICES_PROTOCOL_GUID \
  { \
  0x64fbe01a, 0xdbe8, 0x4932, 0x9f, 0x7b, 0x6c, 0x3d, 0xd0, 0xd6, 0x10, 0xe5 \
  }

typedef struct _OEM_SERVICES_PROTOCOL OEM_SERVICES_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *OEM_SERVICES_FUNCTION) (
  IN  OEM_SERVICES_PROTOCOL             *This,
  IN  UINTN                             NumOfArgs,
  ...
  );

struct _OEM_SERVICES_PROTOCOL {
  OEM_SERVICES_FUNCTION                 *Funcs;
  OEM_SERVICES_FUNCTION                 *FuncsEx;
};

extern EFI_GUID gOemServicesProtocolGuid;

#endif
