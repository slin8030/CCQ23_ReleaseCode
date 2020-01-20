;; @file
;  Find and call SecStartup
;
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2014 - 2016 Intel Corporation.
;
;  The source code contained or described herein and all documents related to the
;  source code ("Material") are owned by Intel Corporation or its suppliers or
;  licensors. Title to the Material remains with Intel Corporation or its suppliers
;  and licensors. The Material may contain trade secrets and proprietary and
;  confidential information of Intel Corporation and its suppliers and licensors,
;  and is protected by worldwide copyright and trade secret laws and treaty
;  provisions. No part of the Material may be used, copied, reproduced, modified,
;  published, uploaded, posted, transmitted, distributed, or disclosed in any way
;  without Intel's prior express written permission.
;
;  No license under any patent, copyright, trade secret or other intellectual
;  property right is granted to or conferred upon you by disclosure or delivery
;  of the Materials, either expressly, by implication, inducement, estoppel or
;  otherwise. Any license under such intellectual property rights must be
;  express and approved by Intel in writing.
;
;  Unless otherwise agreed by Intel in writing, you may not remove or alter
;  this notice or any other notice embedded in Materials by Intel or
;  Intel's suppliers or licensors in any way.
;
;  This file contains a 'Sample Driver' and is licensed as such under the terms
;  of your license agreement with Intel or your vendor. This file may be modified
;  by the user, subject to the additional terms of the license agreement.
;
; @par Specification
;;

.686p
.xmm
.model flat, c
.code

EXTRN   SecStartup:NEAR
EXTRN   PlatformInit:NEAR

CallPeiCoreEntryPoint   PROC PUBLIC
  ;
  ; Obtain the hob list pointer
  ;
  mov     eax, [esp+4]
  ;
  ; Obtain the stack information
  ;   ECX: start of range
  ;   EDX: end of range
  ;
  mov     ecx, [esp+8]
  mov     edx, [esp+0Ch]

  ;
  ; Platform init
  ;
  pushad
  push edx
  push ecx
  push eax
  call PlatformInit
  pop  eax
  pop  eax
  pop  eax
  popad

  ;
  ; Set stack top pointer
  ;
  mov     esp, edx

  ;
  ; Push the hob list pointer
  ;
  push    eax

  ;
  ; Save the value
  ;   ECX: start of range
  ;   EDX: end of range
  ;
  mov     ebp, esp
  push    ecx
  push    edx

  ;
  ; Push processor count to stack first, then BIST status (AP then BSP)
  ;
  mov     eax, 1
  cpuid
  shr     ebx, 16
  and     ebx, 0000000FFh
  cmp     bl, 1
  jae     PushProcessorCount

  ;
  ; Some processors report 0 logical processors.  Effectively 0 = 1.
  ; So we fix up the processor count
  ;
  inc     ebx

PushProcessorCount:
  push    ebx

  ;
  ; We need to implement a long-term solution for BIST capture.  For now, we just copy BSP BIST
  ; for all processor threads
  ;
  xor     ecx, ecx
  mov     cl, bl
PushBist:
  movd    eax, mm0
  push    eax
  loop    PushBist

  ; Save Time-Stamp Counter
  movd eax, mm5
  push eax

  movd eax, mm6
  push eax

  ;
  ; Pass entry point of the PEI core
  ;
  mov     edi, 0FFFFFFE0h
  push    DWORD PTR ds:[edi]

  ;
  ; Pass BFV into the PEI Core
  ;
  mov     edi, 0FFFFFFFCh
  push    DWORD PTR ds:[edi]

  ;
  ; Pass stack size into the PEI Core
  ;
  mov     ecx, [ebp - 4]
  mov     edx, [ebp - 8]
  push    ecx       ; RamBase

  sub     edx, ecx
  push    edx       ; RamSize

  ;
  ; Pass Control into the PEI Core
  ;
  call SecStartup
CallPeiCoreEntryPoint   ENDP

END
