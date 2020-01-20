/** @file
    This is an implementation for preload IBBR and IBBM.  
    
Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _IBB_PRELOAD_H_
#define _IBB_PRELOAD_H_

//
// eMMC ->  0: user partition; 1: boot partition 1; 2:boot partition 2
//
#define EMMC_USER_PARTITION   0
#define EMMC_BOOT_PARTITION_1 1
#define EMMC_BOOT_PARTITION_2 2

#define BPDT_SIGNATURE  0x000055AA
 // SIGNATURE_32 ('B', 'P', 'D', 'T')
typedef enum {
  bpOemSmip,  // 0
  bpCseRbe,   // 1
  bpCseBup,   // 2
  bpuCode,    // 3
  bpIBB,      // 4
  bpSBpdt,    // 5
  bpObb,      // 6
  bpCseMain,  // 7
  bpIsh,      // 8
  bpCseIdlm,  // 9
  bpIfpOverride, //10
  bpMaxType = 0xB
} bpdesc_t;

typedef struct {
  UINT32  RegionType;
  UINT32  RegOffset;
  UINT32  RegSize;
} BPDTDSC;

typedef struct {
  UINT32                    Signature;
  UINT16                    DscCount;
  UINT16                    BpdtVer;
  UINT32                    RedundantBlockXorSum;
  UINT32                    IFWIVer;
  UINT64                    Reserved;
  BPDTDSC                   BpdtDscriptor[bpMaxType];
} BPDT_HEADER;
typedef struct {
  CHAR8    EntryName[12];
  UINT32   EntryOffset;
  UINT32   EntrySize;
  UINT32   Reserved;
} PARTDIRENTRY;

typedef struct {
  UINT32                    Checksum;
  UINT32                    NumOfEntries;
  UINT8                     HeaderVersion;
  UINT8                     EntryVersion;
  UINT8                     HeaderLength;
  UINT8                     Reserved;
  UINT32                    SubPartName;
  PARTDIRENTRY              PatDirEntry[0x8];
} DIRECTORY_HEADER;

typedef struct{
  EFI_GUID                  Name;
} FFS_FILE_HEADER;


#endif
