/** @file

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
/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#ifndef __RAM_DISK_H__
#define __RAM_DISK_H__
   
#include <Uefi.h>

#include <Guid/H2ORamDisk.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/H2ORamDiskServices.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>

#define DEFAULT_DISK_SIZE  8    // in MBs 

#ifndef MIN
#define MIN(x, y)  ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y)  ((x) > (y) ? (x) : (y))
#endif

#define  MIN_DISK_SIZE  1
#define  MAX_DISK_SIZE  512

#pragma pack(1)

/** 
  FAT16 boot sector definition 
**/
typedef struct
{
  UINT8  BS_jmpBoot[3];
  UINT8  BS_OEMName[8];
  UINT16 BPB_BytsPerSec;
  UINT8  BPB_SecPerClus;
  UINT16 BPB_RsvdSecCnt;
  UINT8  BPB_NumFATs;
  UINT16 BPB_RootEntCnt;
  UINT16 BPB_TotSec16;
  UINT8  BPB_Media;
  UINT16 BPB_FATSz16;
  UINT16 BPB_SecPerTrk;
  UINT16 BPB_NumHeads;
  UINT32 BPB_HiddSec;
  UINT32 BPB_TotSec32;
  UINT8  BS_DrvNum;
  UINT8  BS_Reserved1;
  UINT8  BS_BootSig;
  UINT32 BS_VolID;
  UINT8  BS_VolLab[11];
  UINT8  BS_FilSysType[8];
  UINT8  BS_Code[448];
  UINT16 BS_Sig;
} BOOTSEC;

/** 
  structure for total sectors to cluster size lookup 
**/
typedef struct
{
  UINTN size;
  UINT8  spc;
} FAT16TABLE;

#define PBLOCK_DEVICE_SIGNATURE 'rdsk'

/** 
  RAM Disk device info structure 
**/
typedef struct {
  UINTN                         Signature;
  LIST_ENTRY                    Link;
  EFI_HANDLE                    Handle;
  EFI_PHYSICAL_ADDRESS          Start;
  EFI_BLOCK_IO_PROTOCOL         BlkIo;
  EFI_BLOCK_IO_MEDIA            Media;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
} RAM_DISK_DEV;

/**
  Macro finds the device info structure given a RAM Disk BlkIo interface 
**/
#define RAM_DISK_FROM_THIS(a) CR(a, RAM_DISK_DEV, BlkIo, PBLOCK_DEVICE_SIGNATURE)

/** 
  Macro finds the device info structure given a RAM Disk Link List interface 
**/
#define RAM_DISK_FROM_LINK(a) CR(a, RAM_DISK_DEV, Link, PBLOCK_DEVICE_SIGNATURE)

#pragma pack()

#endif    //__RAM_DISK_H__
