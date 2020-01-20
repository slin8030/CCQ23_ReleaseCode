;;******************************************************************************
;;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;;*
;;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;;* transmit, broadcast, present, recite, release, license or otherwise exploit
;;* any part of this publication in any form, by any means, without the prior
;;* written permission of Insyde Software Corporation.
;;*
;;******************************************************************************
;;
;; Module Name:
;;
;;   CallSmmIhisi.nasm
;;
;; Abstract:
;;
;;   64 bit Sent SMI to call IHISI flash ROM part
;;

SEGMENT .text

%define IHISI_SIGNATURE 0x2448324F

;Argument 1: rcx
;Argument 2: rdx

;------------------------------------------------------------------------------
;  UINT8
;  GetPlatformInfo (
;    IN     UINT8            *FbtsBuffer,     // rcx
;    IN     UINT16           SmiPort          // rdx
;    );
;------------------------------------------------------------------------------
global ASM_PFX(GetPlatformInfo)
ASM_PFX(GetPlatformInfo):
  push  rbx
  push  rdi
  push  rsi
  push  r8

  mov   rdi, rcx
  mov   ebx, IHISI_SIGNATURE
  mov   ax,  0x10EF
  out   dx,  al

  ;AL Fun ret state
  pop   r8
  pop   rsi
  pop   rdi
  pop   rbx
  ret