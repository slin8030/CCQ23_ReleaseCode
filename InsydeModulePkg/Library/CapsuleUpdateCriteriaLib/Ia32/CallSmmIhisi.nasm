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
;;   32 bit Sent SMI to call IHISI flash ROM part
;;

  SECTION .text

%define IHISI_SIGNATURE 0x2448324F

;------------------------------------------------------------------------------
;  UINT8
;  GetPlatformInfo (
;    IN     UINT8            *FbtsBuffer,     // ebp+08h
;    IN     UINT16           SmiPort          // ebp+0Ch
;    );
;------------------------------------------------------------------------------

global ASM_PFX(GetPlatformInfo)
ASM_PFX(GetPlatformInfo):
  push  ebx
  push  ecx
  push  edx
  push  edi
  push  esi

  mov   edi, [ebp+0x08]  ; FbtsBuffer
  mov   ebx, IHISI_SIGNATURE
  mov   ax,  0x10EF
  mov   dx,  [ebp+0x0C]  ; SmiPort
  out   dx,  al

  ;AL Fun ret state
  pop   esi
  pop   edi
  pop   edx
  pop   ecx
  pop   ebx
  ret

