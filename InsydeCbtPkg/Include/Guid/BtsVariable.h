/** @file
  This file defines the Bts Variable GUID.
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _BTS_VARIABLE_GUID_H_
#define _BTS_VARIABLE_GUID_H_

#define BTS_VARIABLE_GUID \
  { \
    0xcc1e548e, 0x783f, 0x494e, { 0x83, 0x25, 0xa0, 0xc6, 0x80, 0x0b, 0xeb, 0x75 } \
  }

extern EFI_GUID  gBtsVariableGuid;

#define BTS_VARIABLE_NAME     (L"BtsLogVar")

#pragma pack(1)
typedef struct {
  UINT64  PeiBtsLogAddr;
  UINT32  PeiBtsLogBufferPageSize;
  BOOLEAN PeiBtsLogBufferPageSizeValid;
  UINT64  DxeBtsLogAddr;
  UINT32  DxeBtsLogBufferPageSize;
  BOOLEAN DxeBtsLogBufferPageSizeValid;
} BTS_LOG_VARIABLE;
#pragma pack()

#endif
