;
; This file contains an 'Intel Peripheral Driver' and is      
; licensed for Intel CPUs and chipsets under the terms of your
; license agreement with Intel or your vendor.  This file may 
; be modified by the user, subject to additional terms of the 
; license agreement                                           
;
;
; Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
; This software and associated documentation (if any) is furnished
; under a license and may only be used or copied in accordance
; with the terms of the license. Except as permitted by such
; license, no part of this software or documentation may be
; reproduced, stored in a retrieval system, or transmitted in any
; form or by any means without the express written consent of
; Intel Corporation.
;
;
;
; 
; Module Name:
; 
;    StartKernel.Asm
;
; Abstract:
;       This is the ASM for starting a linux kernel
;
; Notes:
; - Masm uses "This", "ebx", etc as a directive.
; - H2INC is still not embedded in our build process so I translated the struc manually.
; - Unreferenced variables/arguments (This, NewBsp, NewStack) were causing compile errors and 
;       did not know of "pragma" mechanism in MASM and I did not want to reduce the warning level. 
;       Instead, I did a dummy referenced.
;

  .686P
  .MMX
  .MODEL SMALL
  .CODE

AsmStartLinuxKernel PROC  near C  PUBLIC 
  mov     eax, [esp+4]
  sub     esp, 010h
  lea     ebx, NewSelectorJmp
  mov     [esp], ebx
  mov     ebx, 010h
  mov     [esp+4], ebx
  jmp     fword ptr [esp]  

NewSelectorJmp:
  add     esp, 0x10

  mov ebx, 018h
  mov ds, ebx
  mov es, ebx
  mov fs, ebx
  mov gs, ebx
  mov ss, ebx

  mov esi, 08000h
  xor ebp, ebp
  xor edi, edi
  xor ebx, ebx  
  jmp eax  
  ret
AsmStartLinuxKernel   ENDP

  END

