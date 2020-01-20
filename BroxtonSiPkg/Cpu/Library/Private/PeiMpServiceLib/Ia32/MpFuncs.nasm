;; @file
;  This is the assembly code for MP support
;
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2005 - 2016 Intel Corporation.
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

%include  "MpEquNasm.inc"

;-------------------------------------------------------------------------------------
;RendezvousFunnelProc  procedure follows. All APs execute their procedure. This
;procedure serializes all the AP processors through an Init sequence. It must be
;noted that APs arrive here very raw...ie: real mode, no stack.
;ALSO THIS PROCEDURE IS EXECUTED BY APs ONLY ON 16 BIT MODE. HENCE THIS PROC
;IS IN MACHINE CODE.
;-------------------------------------------------------------------------------------
;RendezvousFunnelProc (&WakeUpBuffer,MemAddress);

SECTION .text

%macro PAUSE32 0
            DB      0xF3
            DB      0x90
            %endmacro

;-------------------------------------------------------------------------------
;  AsmAcquireMPLock (&Lock);
;-------------------------------------------------------------------------------
global ASM_PFX(AsmAcquireMPLock)
ASM_PFX(AsmAcquireMPLock):

        pushad
        mov         ebp,esp

        mov         al, NotVacantFlag
        mov         ebx, dword [ebp+0x24]
L_TryGetLock:
        xchg        al, byte [ebx]
        cmp         al, VacantFlag
        jz          L_LockObtained

        PAUSE32
        jmp         L_TryGetLock

L_LockObtained:
        popad
        ret

;-------------------------------------------------------------------------------
;  AsmReleaseMPLock (&Lock);
;-------------------------------------------------------------------------------------
global ASM_PFX(AsmReleaseMPLock)
ASM_PFX(AsmReleaseMPLock):

        pushad
        mov         ebp,esp

        mov         al, VacantFlag
        mov         ebx, dword [ebp+0x24]
        xchg        al, byte [ebx]

        popad
        ret

global ASM_PFX(RendezvousFunnelProc)
ASM_PFX(RendezvousFunnelProc):
L_RendezvousFunnelProcStart:

;Step-1: Grab a lock. At this point CS = 0x(vv00) and ip= 0x0.
        db 0x66,  0x8b, 0xe8                        ; mov        ebp, eax
        db 0x8c,0xc8                                ; mov        ax,cs
        db 0x8e,0xd8                                ; mov        ds,ax
        db 0x8e,0xc0                                ; mov        es,ax
        db 0x8e,0xd0                                ; mov        ss,ax
        db 0x33,0xc0                                ; xor        ax,ax
        db 0x8e,0xe0                                ; mov        fs,ax
        db 0x8e,0xe8                                ; mov        gs,ax
; Get APIC ID
;
        db 0x66,  0xB8
        dd 0x1                  ; mov        eax, 1
        db 0xF,  0xA2                 ; cpuid
        db 0x66,  0xC1, 0xEB, 0x18      ; shr        ebx, 24
        db 0x66,  0x81,  0xE3
        dd 0xFF                  ; and        ebx, 0ffh                   ; EBX is APIC ID

; If it is the first time AP wakes up, just record AP's BIST
; Otherwise, switch to protected mode.

        db 0xBE                       ; opcode of mov si, imm16
        dw StartStateLocation         ; mov        si,  StartState
        db 0x66,  0x83, 0x3C, 0x0        ; cmp        dword ptr [si], 0
        db 0x75                        ; opcode of jnz
        db L_SkipRecordBist - ($ + 1)   ; jnz         SkipRecordBist

; Record BIST information
;
        db 0xB0, 0x8                  ; mov        al,  8
        db 0xF6, 0xE3                 ; mul        bl

        db 0xBE                       ; opcode of mov si, imm16
        dw BistBufferLocation         ; mov        si,  BistBufferLocation
        db 0x3,  0xF0                 ; add        si,  ax

        db 0x66,  0xC7, 0x4
        dd 0x1                  ; mov        dword ptr [si], 1           ; Set Valid Flag
        db 0x66,  0x89,  0x6C,  0x4      ; mov        dword ptr [si + 4], ebp     ; Store BIST value

L_SkipRecordBist:
        db 0xBE                                    ; opcode of mov si, mem16
        dw BufferStartLocation                     ; mov        si, BufferStartLocation
        db 0x66,  0x8B, 0x1C                          ; mov        ebx,dword ptr [si]

        db 0xBF                                    ; opcode of mov di, mem16
        dw PmodeOffsetLocation                     ; mov        di, PmodeOffsetLocation
        db 0x66,  0x8B, 0x5                          ; mov        eax,dword ptr [di]
        db 0x8B,  0xF8                              ; mov        di, ax
        db 0x83,  0xEF,0x6                          ; sub        di, 06h
        db 0x66,  0x3, 0xC3                         ; add        eax, ebx
        db 0x66,  0x89, 0x5                          ; mov        dword ptr [di],eax

        db 0xBE                                    ; opcode of mov si, mem16
        dw GdtrLocation                            ; mov        si, GdtrLocation
        db 0x66                                     ; db         66h
        db 0x2E,  0xF, 0x1, 0x14                     ; lgdt       fword ptr cs:[si]

        db 0xBE                                    ; opcode of mov si, mem16
        dw IdtrLocation                            ; mov        si, IdtrProfile
        db 0x66                                     ; db         66h
        db 0x2E,  0xF, 0x1, 0x1C                     ; lidt       fword ptr cs:[si]

        db 0x33,  0xC0                              ; xor        ax,  ax
        db 0x8E,  0xD8                              ; mov        ds,  ax

        db 0xF,  0x20, 0xC0                         ; mov        eax, cr0                        ;Get control register 0
        db 0x66,  0x83, 0xC8, 0x3                    ; or         eax, 000000003h                 ;Set PE bit (bit #0) & MP
        db 0xF,  0x22, 0xC0                         ; mov        cr0, eax

        db 0x66,  0x67, 0xEA                         ; far jump
        dd 0x0                                      ; 32-bit offset
BspCS:
        dw 0x0                                     ; 16-bit selector

L_NemInit:                                          ; protected mode entry point

        db 0x66,  0xB8                            ; mov        ax, 18h
BspDS:
        dw 0x0
        db 0x66,  0x8E,  0xD8                       ; mov        ds, ax
        db 0x66,  0x8E,  0xC0                       ; mov        es, ax
        db 0x66,  0x8E,  0xE0                       ; mov        fs, ax
        db 0x66,  0x8E,  0xE8                       ; mov        gs, ax
        db 0x66,  0x8E,  0xD0                       ; mov        ss, ax            ; Flat mode setup.

        mov  esi, ebx

        mov  edi, esi
        add  edi, StartStateLocation
        mov  eax, 1
        mov  dword [edi], eax

        mov  edi, esi
        add  edi, LockLocation
        mov  eax, NotVacantFlag
L_TestLock:
        xchg dword [edi], eax
        cmp  eax, NotVacantFlag
        jz   L_TestLock

ProgramStack:

        mov  edi, esi
        add  edi, StackSizeLocation
        mov  eax, dword [edi]
        mov  edi, esi
        add  edi, StackStartAddressLocation
        add  eax, dword [edi]
        mov  esp, eax
        mov  dword [edi], eax

L_Releaselock:

        mov  eax, VacantFlag
        mov  edi, esi
        add  edi, LockLocation
        xchg dword [edi], eax

L_CProcedureInvoke:

        mov  edi, esi
        add  edi, CArgumentLocation
        mov  eax, dword [edi]
        push eax

        mov  edi, esi
        add  edi, CProcedureLocation
        mov  eax, dword [edi]

;
; reserved for SV_HOOKS START
; itp.threads[n].msr(0x121, 0x2FBA2E2500010408)
; WA for ACPI PM1 timer BXT 0 and 1
        push    ecx
        push    eax
        push    edx

        mov     ecx, 0x121
        rdmsr
        test        eax, eax
        jnz          SkipAcpiTimerWA
        mov     eax, 0x10408  ; Bit 16 is enable and 15:0 address
        mov     edx, 0x2FBA2E25
        wrmsr
SkipAcpiTimerWA:
        pop   edx
        pop   eax
        pop   ecx
;
; Reserved for SV_HOOKS END

        call eax
        add  esp, 4

L_InterlockedIncrementFinishedCount:
        mov  edi, esi
        add  edi, FinishedCountAddressLocation
        lock    inc     dword [edi]
        cli
        hlt
        jmp  $-2

global ASM_PFX(SemaphoreStartAddress)
ASM_PFX(SemaphoreStartAddress):
        push ebp
        mov  ebp, esp
        mov  eax, dword [ebp + 0x8]
.0:
        cmp  dword [eax], 0
        jz   .1

        PAUSE32
        jmp  .0
.1:
        ret


;-------------------------------------------------------------------------------------
;  AsmGetAddressMap (&AddressMap);
;-------------------------------------------------------------------------------------
global ASM_PFX(AsmGetAddressMap)
ASM_PFX(AsmGetAddressMap):

        mov eax, L_RendezvousFunnelProcStart
        ret

global ASM_PFX(AsmGetPmodeOffset)
ASM_PFX(AsmGetPmodeOffset):

        mov eax, L_NemInit - L_RendezvousFunnelProcStart
        ret

global ASM_PFX(AsmGetSemaphoreCheckOffset)
ASM_PFX(AsmGetSemaphoreCheckOffset):
        mov eax, SemaphoreStartAddress - L_RendezvousFunnelProcStart
        ret

global ASM_PFX(AsmPatchRendezvousCode)
ASM_PFX(AsmPatchRendezvousCode):
        mov         eax, dword [esp + 4]
        push        esi
        push        edi
        mov         edi, eax
        mov         ax,  cs
        mov         esi, edi
        add         esi, BspCS - L_RendezvousFunnelProcStart
        mov         word [esi], ax
        mov         ax,  ds
        mov         esi, edi
        add         esi, BspDS - L_RendezvousFunnelProcStart
        mov         word [esi], ax
        pop         edi
        pop         esi
        xor         eax, eax
        ret

;-------------------------------------------------------------------------------------
;AsmExchangeRole procedure follows. This procedure executed by current BSP, that is
;about to become an AP. It switches it'stack with the current AP.
;AsmExchangeRole (IN   CPU_EXCHANGE_INFO    *MyInfo, IN   CPU_EXCHANGE_INFO    *OthersInfo);
;-------------------------------------------------------------------------------------
%define CPU_SWITCH_STATE_IDLE 0
%define CPU_SWITCH_STATE_STORED 1
%define CPU_SWITCH_STATE_LOADED 2

global ASM_PFX(AsmExchangeRole)
ASM_PFX(AsmExchangeRole):
        ; DO NOT call other functions in this function, since 2 CPU may use 1 stack
        ; at the same time. If 1 CPU try to call a functiosn, stack will be corrupted.
        pushad
        mov         ebp,esp

        ; esi contains MyInfo pointer
        mov         esi, dword [ebp+0x24]

        ; edi contains OthersInfo pointer
        mov         edi, dword [ebp+0x28]

        ;Store EFLAGS, GDTR and IDTR regiter to stack
        pushfd
        sgdt        [esi+8]
        sidt        [esi+14]

        ; Store the its StackPointer
        mov         dword [esi+4],esp

        ; update its switch state to STORED
        mov         byte [esi], CPU_SWITCH_STATE_STORED

L_WaitForOtherStored:
        ; wait until the other CPU finish storing its state
        cmp         byte [edi], CPU_SWITCH_STATE_STORED
        jz          L_OtherStored
        PAUSE32
        jmp         L_WaitForOtherStored

L_OtherStored:
        ; Since another CPU already stored its state, load them
        ; load GDTR value
        lgdt        [edi+8]

        ; load IDTR value
        lidt        [edi+14]

        ; load its future StackPointer
        mov         esp, dword [edi+4]

        ; update the other CPU's switch state to LOADED
        mov         byte [edi], CPU_SWITCH_STATE_LOADED

L_WaitForOtherLoaded:
        ; wait until the other CPU finish loading new state,
        ; otherwise the data in stack may corrupt
        cmp         byte [esi], CPU_SWITCH_STATE_LOADED
        jz          L_OtherLoaded
        PAUSE32
        jmp         L_WaitForOtherLoaded

L_OtherLoaded:
        ; since the other CPU already get the data it want, leave this procedure
        popfd

        popad
        ret
