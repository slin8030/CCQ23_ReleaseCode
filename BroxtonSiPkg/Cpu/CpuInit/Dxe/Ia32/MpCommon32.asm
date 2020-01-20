;;@file
;
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 1999 - 2016 Intel Corporation.
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
;  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
;  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
;  the terms of your license agreement with Intel or your vendor. This file may
;  be modified by the user, subject to additional terms of the license agreement.
;
;@par Specification
;;
.686p
.model  flat
.data
.stack
.code
.MMX
.XMM

 include  Htequ.inc
PAUSE32   MACRO
            DB      0F3h
            DB      090h
            ENDM

;-------------------------------------------------------------------------------
;  AsmAcquireMPLock (&Lock);
;-------------------------------------------------------------------------------
AsmAcquireMPLock   PROC  near C  PUBLIC

        pushad
        mov         ebp,esp

        mov         al, NotVacantFlag
        mov         ebx, dword ptr [ebp+24h]
TryGetLock:
        db 0f0h                       ; opcode for lock instruction
        xchg        al, byte ptr [ebx]
        cmp         al, VacantFlag
        jz          LockObtained

        PAUSE32
        jmp         TryGetLock

LockObtained:
        popad
        ret
AsmAcquireMPLock   ENDP

;-------------------------------------------------------------------------------
;  AsmReleaseMPLock (&Lock);
;-------------------------------------------------------------------------------------
AsmReleaseMPLock   PROC  near C  PUBLIC

        pushad
        mov         ebp,esp

        mov         al, VacantFlag
        mov         ebx, dword ptr [ebp+24h]
        db 0f0h                       ; opcode for lock instruction
        xchg        al, byte ptr [ebx]

        popad
        ret
AsmReleaseMPLock   ENDP

;-------------------------------------------------------------------------------
;  AsmGetGdtrIdtr (&Gdt, &Idt);
;-------------------------------------------------------------------------------------
AsmGetGdtrIdtr   PROC  near C  PUBLIC

        pushad
        mov         ebp,esp

        sgdt        fword ptr GdtDesc
        lea         esi, GdtDesc
        mov         edi, dword ptr [ebp+24h]
        mov         dword ptr [edi], esi

        sidt        fword ptr IdtDesc
        lea         esi, IdtDesc
        mov         edi, dword ptr [ebp+28h]
        mov         dword ptr [edi], esi

        popad
        ret
AsmGetGdtrIdtr   ENDP

GdtDesc::                             ; GDT descriptor
                    DW      03fh      ; GDT limit
                    DW      0h        ; GDT base and limit will be
                    DW      0h        ; filled using sgdt

IdtDesc::                             ; IDT descriptor
                    DW      0h        ; IDT limit
                    DW      0h        ; IDT base and limit will be
                    DW      0h        ; filled using sidt

END