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
TITLE   CallSmmIhis.asm

  .686
  .MODEL FLAT,C
  .CODE

IHISI_SIGNATURE EQU      02448324Fh

;------------------------------------------------------------------------------
;  UINT8
;  GetPlatformInfo (
;    IN     UINT8            *FbtsBuffer,
;    IN     UINT16           SmiPort
;    );
;------------------------------------------------------------------------------

GetPlatformInfo PROC       PUBLIC FbtsBuffer:DWORD, SmiPort:WORD
  push  ebx
  push  ecx
  push  edx
  push  edi
  push  esi

  mov   edi, FbtsBuffer  ; FbtsBuffer
  mov   ebx, IHISI_SIGNATURE
  mov   ax,  10EFh
  mov   dx,  SmiPort  ; SmiPort
  out   dx,  al

  ;AL Fun ret state
  pop   esi
  pop   edi
  pop   edx
  pop   ecx
  pop   ebx
  ret

GetPlatformInfo  ENDP

END