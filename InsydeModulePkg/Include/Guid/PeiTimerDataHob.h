/** @file
  This file defines the Thunk Data HOB GUID.
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

#ifndef _H2O_PEI_TIMER_DATA_HOB_GUID_H_  
#define _H2O_PEI_TIMER_DATA_HOB_GUID_H_

#define H2O_PEI_THUNK_DATA_HOB_GUID  \
  { \
    0x5a31041e, 0xb24f, 0x42bf, { 0xb3, 0xbe, 0x42, 0xd3, 0x26, 0x8c, 0x28, 0xf1 } \
  }

typedef struct {
  UINT32  PeiServicesPoint;
  UINT32  CallBackFunction;
  UINT8   MasterBaseVector;
} H2O_PEI_TIMER_DATA_HOB;

extern EFI_GUID  gH2OPeiTimerDataHobGuid;

#endif
