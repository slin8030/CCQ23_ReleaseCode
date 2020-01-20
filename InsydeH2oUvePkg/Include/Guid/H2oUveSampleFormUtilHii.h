/** @file

   The definition of H2oUve Smaple Form Config HII.

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

#ifndef _H2OUVE_SAMPLE_FORM_HII_H_
#define _H2OUVE_SAMPLE_FORM_HII_H_

#define H2OUVE_SAMPLE_FORM_VARSTORE_GUID \
  { \
    0x58ace6de, 0x505f, 0x4bec, { 0xb8, 0x8e, 0x92, 0xe3, 0x54, 0x73, 0x63, 0xf1} \
  }

#define H2OUVE_SAMPLE_FORM_GUID \
  { \
     0x9139e3cb, 0x4eee, 0x43fc, { 0x80, 0x25, 0xe3, 0x58, 0xdc, 0x94, 0x7d, 0x93 } \
  }

#define H2OUVE_SAMPLE_FORM_VARSTORE_NAME   L"H2oUveDebugVar"

#pragma pack(push, 1)
typedef struct {
  EFI_HII_TIME       Time;
  EFI_HII_DATE       Date;
  EFI_HII_TIME       Time2;
  UINT32             Prompt0;
  UINT16             Prompt1;
  UINT8              Prompt2;
  UINT32             Prompt3;
  UINT16             Prompt4;
  UINT32             Prompt5;
  UINT8              Prompt6;
  UINT8              Prompt7;
  UINT16             Prompt8;
  UINT16             Prompt9;
  UINT8              Prompt10;
  UINT16             Prompt11;
  UINT8              Prompt12;
  UINT16             Prompt13;
} H2OUVE_DEBUG_VAR;

typedef struct {
  UINT32             Item_32;
  UINT16             Item_16;
  UINT8              Item_8;
} H2OUVE_TEST_VAR;
#pragma pack(pop)

extern EFI_GUID gH2oUveSampleFormVarstoreGuid;
extern EFI_GUID gH2oUveSampleFormGuid;

#endif
