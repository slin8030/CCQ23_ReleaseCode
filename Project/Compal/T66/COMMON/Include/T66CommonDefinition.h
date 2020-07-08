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

#ifndef _T66_COMMON_H_
#define _T66_COMMON_H_

#include <Uefi.h>
#include <ProjectDefinition.h>

#define T66_EEPROM_UUID_OFFSET                        0x70      // UUID offset 0x70 - 0x7F
#define T66_EEPROM_UUID_LENGTH                        0x10      // 16 Bytes
#define EFI_BIOS_SETTING_GUID  \
{0x41FEFB59, 0x99BE, 0x4932, {0x8F, 0xCE, 0x9E, 0x68, 0x08, 0x85, 0x55, 0x40}}
STATIC EFI_GUID  gEfiBIOSSettingGuid  = EFI_BIOS_SETTING_GUID;


#define BIOS_SETTING_SIGNATURE        SIGNATURE_32 ('$', 'B', 'S', 'T')
#define BIOS_SETTING_ALPG             SIGNATURE_32 ('A', 'L', 'P', 'G')

#define BIOS_SETTING_VARIABLE_NAME    L"BiosSetting"
typedef struct {
  UINT32                  Signature;
  UINTN                   DataSize;
} BIOS_SETTING_HEADER;

typedef struct {
  BIOS_SETTING_HEADER     Header;
  UINT8                   Data[1];
} BIOS_SETTING_STRUCT;

typedef struct {
  LIST_ENTRY              Link;
  BIOS_SETTING_HEADER     Header;
  UINT8                   Data[1];
} BIOS_SETTING_ENTRY;

typedef struct {
  BIOS_SETTING_HEADER     Header;
  BOOLEAN                 ShollAllPage;
} BIOS_SETTING_ALL_PAGE;

#pragma pack()

#endif
