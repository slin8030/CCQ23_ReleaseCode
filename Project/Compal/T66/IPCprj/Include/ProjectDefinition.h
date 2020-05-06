//;******************************************************************************
//;* Copyright (c) 1983-2006, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;
//; Abstract:  ver 2
//;

#ifndef _PROJECT_H_
#define _PROJECT_H_

#include <Uefi.h>


#define ECNAME_F4A4         0xF4A4

typedef struct ECNAME_F4A4_BITS_STRU_{
  UINT8  LAN_Wake         :1;   // 0=Disable, 1=Enable
  UINT8  Reserve1         :3;
  UINT8  WLAN_Wake        :1;   // 0=Disable, 1=Enable
  UINT8  USB_Wake         :1;   // 0=Disable, 1=Enable
  UINT8  Reserve2         :2;
} ECNAME_F4A4_BITS_STRU;

typedef union {
  ECNAME_F4A4_BITS_STRU  Bits;
  UINT8                  Data;
} ECNAME_F4A4_STRU;

#endif
