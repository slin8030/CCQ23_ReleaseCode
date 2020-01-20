/** @file
  This file provides IHISI Structure and define for Chipset driver

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CHIPSET_IHISI_H_
#define _CHIPSET_IHISI_H_

#include <H2OIhisi.h>

#define EFI_CACHE_VARIABLE_MTRR_BASE  0x200
#define EFI_CACHE_MTRR_VALID          0x800

#define FBTSAPHOOK_SUPPORT          0x204

typedef struct {
  UINT8      IhisiAreaType;
  UINT8      FlashAreaType;
} FLASH_MAP_MAPPING_TABLE;

#define FLASH_VALID_SIGNATURE              0x0FF0A55A      //Flash Descriptor is considered valid
#define FLASH_MAP_0_OFFSET                 0x04
#define FLASH_REGION_BASE_MASK             0x00ff0000
#define FLASH_MAP_1_OFFSET                 0x08
#define FLASH_MASTER_BASE_MASK             0x000000ff
#define PLATFORM_REGION_OFFSET             0x00
#define PLATFORM_REGION_SIZE               0x100
#define ROM_FILE_REGION_OFFSET             0x100
#define ROM_FILE_REGION_SIZE               0x100
#define FLASH_MASTER_1_READ_ACCESS_BIT     16
#define FLASH_MASTER_1_WRITE_ACCESS_BIT    24
#define DOS_FLASH_TOOL                     0x01
#define WINDOWS_FLASH_TOOL                 0x02
#define LINUX_FLASH_TOOL                   0x03
#define SHELL_FLASH_TOOL                   0x04

#pragma pack(1)

typedef struct {
  UINT16  Version;
  UINT8   Reserved[2];
  UINT32  Algorithm;
} PUPC_HEADER;

typedef struct {
  UINT16  Version;
  UINT8   Reserved[2];
  UINT32  Algorithm;
  UINT8   VKeyMod[256];
  UINT8   VKeyExp[4];
  UINT8   Signature[256];
} PUPC_ALGORITHM_1;

#pragma pack()

#endif
