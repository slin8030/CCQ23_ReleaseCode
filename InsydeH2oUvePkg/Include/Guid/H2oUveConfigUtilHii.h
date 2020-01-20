/** @file

   The definition of H2oUve Config HII.

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

#ifndef _H2OUVE_CONFIG_HII_H_
#define _H2OUVE_CONFIG_HII_H_

#define H2OUVE_CONFIGURATION_VARSTORE_GUID \
  { \
    0xef0849b6, 0xfad0, 0x40e9, { 0x91, 0x7, 0x97, 0x4a, 0xeb, 0x87, 0x87, 0xa2 } \
  }

#define H2OUVE_FORMSET_GUID \
  { \
    0xd1d7208f, 0x84ec, 0x411a, { 0x9b, 0xed, 0x98, 0xe0, 0x0, 0xba, 0x74, 0x95 } \
  }

#define H2OUVE_VARSTORE_NAME      L"VarEditFunDis"
#define H2OUVE_VARSTORE_PW_NAME   L"VarEditPw"

#pragma pack(push, 1)
typedef struct {
  UINT32             VariableEditFunDis;
} VARIABLE_EDIT_FUNCTION_DISABLE_SETTING;

typedef struct {
  UINT8              SupervisorPwLength;
  CHAR16             SupervisorPw[10]; // Size of array should equal to PcdDefaultSysPasswordMaxLength
  UINT8              UserPwLength;
  CHAR16             UserPw[10]; // Size of array should equal to PcdDefaultSysPasswordMaxLength
  UINT8              CheckSum;
} PASSWORD_INFO;

typedef struct {
  PASSWORD_INFO      PasswordInfo;
  UINT8              IsDone;
} VARIABLE_EDIT_FUNCTION_PASSWORD_SETTING;
#pragma pack(pop)

extern EFI_GUID gH2oUveVarstoreGuid;
extern EFI_GUID gH2oUveFormsetGuid;

#endif
