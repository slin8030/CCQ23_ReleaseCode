/*++
Copyright (c)  2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
  
Module Name:

 CpuContext.h

Abstract:
  
--*/

#ifndef _CPUCONTEXT_H
#define _CPUCONTEXT_H

#define CONTEXT_i386    0x00010000    // this assumes that i386 and
#define CONTEXT_i486    0x00010000    // i486 have identical context records

#define CONTEXT_CONTROL         (CONTEXT_i386 | 0x00000001L) // SS:SP, CS:IP, FLAGS, BP
#define CONTEXT_INTEGER         (CONTEXT_i386 | 0x00000002L) // AX, BX, CX, DX, SI, DI
#define CONTEXT_SEGMENTS        (CONTEXT_i386 | 0x00000004L) // DS, ES, FS, GS
#define CONTEXT_FLOATING_POINT  (CONTEXT_i386 | 0x00000008L) // 387 state
#define CONTEXT_DEBUG_REGISTERS (CONTEXT_i386 | 0x00000010L) // DB 0-3,6,7
#define CONTEXT_EXTENDED_REGISTERS  (CONTEXT_i386 | 0x00000020L) // cpu specific extensions

#pragma pack(1)

typedef struct {
  UINT32 Flag;  //Flag;
  
  UINT32 DR0;
  UINT32 DR1;
  UINT32 DR2;
  UINT32 DR3;
  UINT32 DR6;
  UINT32 DR7;
  UINT32 FloatSave[28];

  UINT32 SegGs;
  UINT32 SegFs;
  UINT32 SegEs;
  UINT32 SegDs;

  UINT32 Edi;
  UINT32 Esi;
  UINT32 Ebx;
  UINT32 Edx;
  UINT32 Ecx;
  UINT32 Eax;

  UINT32 Ebp;
  UINT32 Eip;
  UINT32 SegCs;
  UINT32 Eflags;
  UINT32 Esp;
  UINT32 SegSs;

  UINT32 ExtendedRegister[128];
}OFFLINE_CPU_CONTEXT;

typedef struct{
  UINT32                NumOfCpus;
  UINT32                CpuArchitecture;
  OFFLINE_CPU_CONTEXT   CpuContext[4];
} OFFLINE_CPU_CONTEXT_LIST;

#pragma pack()

#endif

