;; @file
; 
;  Assembly code that supports IA32 for Thunk64To32Lib
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
include Thunk.inc

    .586P
    .model  flat,C

EXTERNDEF   mIA32Cr3:DWORD

    .code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;VOID
;InternalThunk64To32 (
;  IN MEMORY_THUNK  *IntThunk,
;  IN UINT32        FunctionPoint
;  IN UINT32        PeiServicesPoint
;  );
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
InternalThunk64To32  PROC

    push    ebp
    mov     ebp, esp
    push    ebx
    push    ecx
    push    edx
    push    edi
    push    esi

    mov     ecx,  [ebp + 8]      ;*IntThunk
    mov     edx,  [ebp + 12]     ;FunctionPoint
    mov     ebx, esp
    
    lea     eax,  ( MEMORY_THUNK PTR [ecx] ).ia32Stack
    mov     esp,  DWORD PTR [eax]

    push    ebx
    
    mov     eax,  [ebp + 16]     ;PeiServicesPoint    
    push    eax  
    
    call    edx
    
    pop     eax

    pop     ebx    
    mov     esp, ebx
    
    pop     esi
    pop     edi
    pop     edx
    pop     ecx
    pop     ebx
    pop     ebp

    ret
    
InternalThunk64To32  ENDP

END