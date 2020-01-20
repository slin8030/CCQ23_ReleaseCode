;; @file
;  Assembly code that supports IA32 for CpuExceptionHandlerLib
;
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

.686p
.model  flat     

.data
TimerCallbackFunctionPtr DW ?  ; Save Timer0 callback function pointer.

.code

_InitializeTimerCallbackPtr  proc public
  push    ebp                  ; C prolog
  mov     ebp, esp
  
  mov     eax, [ebp+8]         ; Get callback function pointer
  mov     TimerCallbackFunctionPtr, eax
  
  pop     ebp
  ret
_InitializeTimerCallbackPtr ENDP

;
;---------------------------------------;
; InterruptEntry                        ;
;---------------------------------------;
; IRQ0 interrupt serivce routine.
;
_InterruptEntry PROC  PUBLIC  
  cli
  pushad
  ;
  ; 8259 IRQ0 EOI
  ;
  mov     al, 0x20
  out     0x20, al
  
  mov     eax, TimerCallbackFunctionPtr  
  
  call    eax
  cli

  popad
  iretd
_InterruptEntry ENDP

END

