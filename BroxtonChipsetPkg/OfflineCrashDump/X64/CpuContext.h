/** @file

@copyright
 Copyright (c) 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
 Windows Offline Crash Dump Specification

**/

#ifndef _CPUCONTEXT_H
#define _CPUCONTEXT_H

#define CONTEXT_AMD64   0x00100000L
#define CONTEXT_i386    0x00010000    // this assumes that i386 and
#define CONTEXT_i486    0x00010000    // i486 have identical context records

#define CONTEXT_CONTROL (CONTEXT_AMD64 | 0x1L)
#define CONTEXT_INTEGER (CONTEXT_AMD64 | 0x2L)
#define CONTEXT_SEGMENTS (CONTEXT_AMD64 | 0x4L)
#define CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x8L)
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x10L)

#pragma pack(1)

typedef struct {
  //
  // Register parameter home addresses.
  //
  // N.B. These fields are for convience - they could be used to extend the
  //      context record in the future.
  //

  UINT64 P1Home;
  UINT64 P2Home;
  UINT64 P3Home;
  UINT64 P4Home;
  UINT64 P5Home;
  UINT64 P6Home;

  //
  // Control flags.
  //

  UINT32 Flag;
  UINT32 MxCsr;

  //
  // Segment Registers and processor flags.
  //

  UINT16 SegCs;
  UINT16 SegDs;
  UINT16 SegEs;
  UINT16 SegFs;
  UINT16 SegGs;
  UINT16 SegSs;
  UINT32 EFlags;

  //
  // Debug registers
  //

  UINT64 Dr0;
  UINT64 Dr1;
  UINT64 Dr2;
  UINT64 Dr3;
  UINT64 Dr6;
  UINT64 Dr7;

  //
  // Integer registers.
  //

  UINT64 Rax;
  UINT64 Rcx;
  UINT64 Rdx;
  UINT64 Rbx;
  UINT64 Rsp;
  UINT64 Rbp;
  UINT64 Rsi;
  UINT64 Rdi;
  UINT64 R8;
  UINT64 R9;
  UINT64 R10;
  UINT64 R11;
  UINT64 R12;
  UINT64 R13;
  UINT64 R14;
  UINT64 R15;

  //
  // Program counter.
  //

  UINT64 Rip;

  //
  // Floating point state.
  //

  UINT8  FltSave[512];

  //
  // Vector registers.
  //

  UINT64 VectorRegister[52];
  UINT64 VectorControl;

  //
  // Special debug control registers.
  //

  UINT64 DebugControl;
  UINT64 LastBranchToRip;
  UINT64 LastBranchFromRip;
  UINT64 LastExceptionToRip;
  UINT64 LastExceptionFromRip;
}OFFLINE_CPU_CONTEXT;

typedef struct{
  UINT32                NumOfCpus;
  UINT32                CpuArchitecture;
  OFFLINE_CPU_CONTEXT   CpuContext[4];
} OFFLINE_CPU_CONTEXT_LIST;

#pragma pack()

#endif

