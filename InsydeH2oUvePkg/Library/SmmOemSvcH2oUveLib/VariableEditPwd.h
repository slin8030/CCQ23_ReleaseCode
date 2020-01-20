/** @file
  Content file contains function definitions for Variable Edit Smm Driver.

;******************************************************************************
;* Copyright (c) 1983 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#ifndef _VARIABLE_EDIT_PWD_H
#define _VARIABLE_EDIT_PWD_H

#include <Base.h>

#pragma pack(1)

#define IHISI_H2O_SMI_CB_SIGNATURE      SIGNATURE_32 ('$', 'U', 'V', 'E')

//----------------------------
// UVE SMI Callback Type Value
//----------------------------
#define H2O_SMI_CALLBACK_STATUS           0x00
#define H2O_SMI_CALLBACK_PASSWORD_CHECK   0x01
#define H2O_SMI_CALLBACK_PASSWORD         0x02

#define FLAG_H2O_SMICB_PSW                0x01

enum {
  STS_H2O_SMICB_UNSUPPORTED = 0,
  STS_H2O_SMICB_SUCCESS, 
  STS_H2O_SMICB_PSW_CHECK,
  STS_H2O_SMICB_PSW_CHK_FAILED,
  STS_H2O_SMICB_ACCESS_DENIED
};

typedef struct _H2O_CALLBACK_COMMON_HDR {
  UINT8                        Type;
  INT32                        Status;
  UINT16                       Size;  
} H2O_CALLBACK_COMMON_HDR;

typedef struct _H2O_CALLBACK_STATUS {
  H2O_CALLBACK_COMMON_HDR      Hdr;
  UINT32                       Status;
  UINT32                       Flag;
  UINT8                        Reserve;
} H2O_CALLBACK_STATUS;

typedef struct _H2O_CALLBACK_PASSWORD_CHECK {
  H2O_CALLBACK_COMMON_HDR      Hdr;
} H2O_CALLBACK_PASSWORD_CHECK;

typedef struct _H2O_CALLBACK_PASSWORD {
  H2O_CALLBACK_COMMON_HDR      Hdr;
//  CHAR8                        CheckPasswrod[];
//  UINT8                        VarData[]
} H2O_CALLBACK_PASSWORD;

typedef struct _H2O_SMI_CALLBACK {
  UINT8                        ZeroVector[16];
  UINT32                       Signature;
} H2O_SMI_CALLBACK;


#pragma pack()

#endif
