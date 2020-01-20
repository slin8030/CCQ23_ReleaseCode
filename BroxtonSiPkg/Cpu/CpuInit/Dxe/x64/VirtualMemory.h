/** @file
  x64 Long Mode Virtual Memory Management Definitions.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
  - 1) IA-32 Intel(R) Atchitecture Software Developer's Manual Volume 1:Basic Architecture, Intel
  - 2) IA-32 Intel(R) Atchitecture Software Developer's Manual Volume 2:Instruction Set Reference, Intel
  - 3) IA-32 Intel(R) Atchitecture Software Developer's Manual Volume 3:System Programmer's Guide, Intel
  - 4) AMD64 Architecture Programmer's Manual Volume 2: System Programming
**/
#ifndef _VIRTUAL_MEMORY_H_
#define _VIRTUAL_MEMORY_H_

#pragma pack(1)
///
/// Page-Map Level-4 Offset (PML4) and
///Page-Directory-Pointer Offset (PDPE) entries 4K & 2MB
///
typedef union {
  struct {
    UINT64  Present : 1;                ///< 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite : 1;              ///< 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor : 1;         ///< 0 = Supervisor, 1=User
    UINT64  WriteThrough : 1;           ///< 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled : 1;          ///< 0 = Cached, 1=Non-Cached
    UINT64  Accessed : 1;               ///< 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Reserved : 1;               ///< Reserved
    UINT64  MustBeZero : 2;             ///< Must Be Zero
    UINT64  Available : 3;              ///< Available for use by system software
    UINT64  PageTableBaseAddress : 40;  ///< Page Table Base Address
    UINT64  AvabilableHigh : 11;        ///< Available for use by system software
    UINT64  Nx : 1;                     ///< No Execute bit
  } Bits;
  UINT64  Uint64;
} x64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K;

///
/// Page-Directory Offset 4K
///
typedef union {
  struct {
    UINT64  Present : 1;                ///< 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite : 1;              ///< 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor : 1;         ///< 0 = Supervisor, 1=User
    UINT64  WriteThrough : 1;           ///< 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled : 1;          ///< 0 = Cached, 1=Non-Cached
    UINT64  Accessed : 1;               ///< 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Reserved : 1;               ///< Reserved
    UINT64  MustBeZero : 1;             ///< Must Be Zero
    UINT64  Reserved2 : 1;              ///< Reserved
    UINT64  Available : 3;              ///< Available for use by system software
    UINT64  PageTableBaseAddress : 40;  ///< Page Table Base Address
    UINT64  AvabilableHigh : 11;        ///< Available for use by system software
    UINT64  Nx : 1;                     ///< No Execute bit
  } Bits;
  UINT64  Uint64;
} x64_PAGE_DIRECTORY_ENTRY_4K;

///
/// Page Table Entry 4K
///
typedef union {
  struct {
    UINT64  Present : 1;                ///< 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite : 1;              ///< 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor : 1;         ///< 0 = Supervisor, 1=User
    UINT64  WriteThrough : 1;           ///< 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled : 1;          ///< 0 = Cached, 1=Non-Cached
    UINT64  Accessed : 1;               ///< 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Dirty : 1;                  ///< 0 = Not Dirty, 1 = written by processor on access to page
    UINT64  PAT : 1;                    ///< 0 = Ignore Page Attribute Table
    UINT64  Global : 1;                 ///< 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64  Available : 3;              ///< Available for use by system software
    UINT64  PageTableBaseAddress : 40;  ///< Page Table Base Address
    UINT64  AvabilableHigh : 11;        ///< Available for use by system software
    UINT64  Nx : 1;                     ///< 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64  Uint64;
} x64_PAGE_TABLE_ENTRY_4K;

///
/// Page Table Entry 2MB
///
typedef union {
  struct {
    UINT64  Present : 1;                ///< 0 = Not present in memory, 1 = Present in memory
    UINT64  ReadWrite : 1;              ///< 0 = Read-Only, 1= Read/Write
    UINT64  UserSupervisor : 1;         ///< 0 = Supervisor, 1=User
    UINT64  WriteThrough : 1;           ///< 0 = Write-Back caching, 1=Write-Through caching
    UINT64  CacheDisabled : 1;          ///< 0 = Cached, 1=Non-Cached
    UINT64  Accessed : 1;               ///< 0 = Not accessed, 1 = Accessed (set by CPU)
    UINT64  Dirty : 1;                  ///< 0 = Not Dirty, 1 = written by processor on access to page
    UINT64  MustBe1 : 1;                ///< Must be 1
    UINT64  Global : 1;                 ///< 0 = Not global page, 1 = global page TLB not cleared on CR3 write
    UINT64  Available : 3;              ///< Available for use by system software
    UINT64  PAT : 1;                    ///<
    UINT64  MustBeZero : 8;             ///< Must be zero;
    UINT64  PageTableBaseAddress : 31;  ///< Page Table Base Address
    UINT64  AvabilableHigh : 11;        ///< Available for use by system software
    UINT64  Nx : 1;                     ///< 0 = Execute Code, 1 = No Code Execution
  } Bits;
  UINT64  Uint64;
} x64_PAGE_TABLE_ENTRY_2M;

typedef struct {
  UINT64  Present : 1;                  ///< 0 = Not present in memory, 1 = Present in memory
  UINT64  ReadWrite : 1;                ///< 0 = Read-Only, 1= Read/Write
  UINT64  UserSupervisor : 1;           ///< 0 = Supervisor, 1=User
  UINT64  WriteThrough : 1;             ///< 0 = Write-Back caching, 1=Write-Through caching
  UINT64  CacheDisabled : 1;            ///< 0 = Cached, 1=Non-Cached
  UINT64  Accessed : 1;                 ///< 0 = Not accessed, 1 = Accessed (set by CPU)
  UINT64  Dirty : 1;                    ///< 0 = Not Dirty, 1 = written by processor on access to page
  UINT64  Reserved : 57;
} x64_PAGE_TABLE_ENTRY_COMMON;

typedef union {
  x64_PAGE_TABLE_ENTRY_4K     Page4k;
  x64_PAGE_TABLE_ENTRY_2M     Page2Mb;
  x64_PAGE_TABLE_ENTRY_COMMON Common;
} x64_PAGE_TABLE_ENTRY;

#pragma pack()

#endif
