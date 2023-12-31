;------------------------------------------------------------------------------
; @file
; Search for the SEC Core entry point
;
; Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
; This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
;------------------------------------------------------------------------------

BITS    32
struc HobStruc
  .Sign	                RESD  1            ; Signiture#
  .CarBase              RESD  1            ; Cache As Ram Base Address
  .CarSize              RESD  1            ; Cache As Ram Size
  .IBBSource            RESD  1            ; IBB Address in SRAM
  .IBBBase              RESD  1            ; IBB Base in CAR.
  .IBBSize              RESD  1            ; IBB Size
  .IBBLSource           RESD  1            ; IBBL Address in SRAM
  .IBBLBase             RESD  1            ; IBBL Base in CAR.
  .IBBLSize             RESD  1            ; IBBL Size
endstruc

%define EFI_FV_FILETYPE_SECURITY_CORE         0x03

;
; Modified:  EAX, EBX, ECX, EDX
; Preserved: EDI, EBP, ESP
;
; @param[in]   EBP  Address of Boot Firmware Volume (BFV)
; @param[out]  ESI  SEC Core Entry Point Address
;
Flat32SearchForSecEntryPoint:

    ;
    ; Initialize EBP and ESI to 0
    ;
    xor     ebx, ebx
    mov     esi, ebx

    ;
    ; Pass over the BFV header
    ;
    mov     eax, [ebp + HobStruc.IBBBase]
    mov     bx, [eax + 0x30]
    add     eax, ebx
    jc      secEntryPointWasNotFound

    jmp     searchingForFfsFileHeaderLoop

moveForwardWhileSearchingForFfsFileHeaderLoop:
    ;
    ; Make forward progress in the search
    ;
    inc     eax
    jc      secEntryPointWasNotFound

searchingForFfsFileHeaderLoop:
    test    eax, eax
    jz      secEntryPointWasNotFound

    ;
    ; Ensure 8 byte alignment
    ;
    add     eax, 7
    jc      secEntryPointWasNotFound
    and     al, 0xf8

    ;
    ; Look to see if there is an FFS file at eax
    ;
    mov     bl, [eax + 0x17]
    test    bl, 0x20
    jz      moveForwardWhileSearchingForFfsFileHeaderLoop
    mov     ecx, [eax + 0x14]
    and     ecx, 0x00ffffff
    or      ecx, ecx
    jz      moveForwardWhileSearchingForFfsFileHeaderLoop
    add     ecx, eax
    jz      jumpSinceWeFoundTheLastFfsFile
    jc      moveForwardWhileSearchingForFfsFileHeaderLoop
jumpSinceWeFoundTheLastFfsFile:

    ;
    ; There seems to be a valid file at eax
    ;
    cmp     byte [eax + 0x12], EFI_FV_FILETYPE_SECURITY_CORE ; Check File Type
    jne     readyToTryFfsFileAtEcx

fileTypeIsSecCore:
    OneTimeCall GetEntryPointOfFfsFile
    test    eax, eax
    jnz     doneSeachingForSecEntryPoint

readyToTryFfsFileAtEcx:
    ;
    ; Try the next FFS file at ECX
    ;
    mov     eax, ecx
    jmp     searchingForFfsFileHeaderLoop

secEntryPointWasNotFound:
    xor     eax, eax

doneSeachingForSecEntryPoint:
    mov     esi, eax

    test    esi, esi
    jnz     secCoreEntryPointWasFound

secCoreEntryPointWasNotFound:
    ;
    ; Hang if the SEC entry point was not found
    ;
    debugShowPostCode POSTCODE_SEC_NOT_FOUND
    jz      $

secCoreEntryPointWasFound:
    debugShowPostCode POSTCODE_SEC_FOUND

    OneTimeCallRet Flat32SearchForSecEntryPoint

%define EFI_SECTION_PE32                  0x10
%define EFI_SECTION_TE                    0x12

;
; Input:
;   EAX - Start of FFS file
;   ECX - End of FFS file
;
; Output:
;   EAX - Entry point of PE32 (or 0 if not found)
;
; Modified:
;   EBX
;
GetEntryPointOfFfsFile:
    test    eax, eax
    jz      getEntryPointOfFfsFileErrorReturn
    add     eax, 0x18       ; EAX = Start of section

getEntryPointOfFfsFileLoopForSections:
    cmp     eax, ecx
    jae     getEntryPointOfFfsFileErrorReturn

    cmp     byte [eax + 3], EFI_SECTION_PE32
    je      getEntryPointOfFfsFileFoundPe32Section

    cmp     byte [eax + 3], EFI_SECTION_TE
    je      getEntryPointOfFfsFileFoundTeSection

    ;
    ; The section type was not PE32 or TE, so move to next section
    ;
    mov     ebx, dword [eax]
    and     ebx, 0x00ffffff
    add     eax, ebx
    jc      getEntryPointOfFfsFileErrorReturn

    ;
    ; Ensure that FFS section is 32-bit aligned
    ;
    add     eax, 3
    jc      getEntryPointOfFfsFileErrorReturn
    and     al, 0xfc
    jmp     getEntryPointOfFfsFileLoopForSections

getEntryPointOfFfsFileFoundPe32Section:
    add     eax, 4       ; EAX = Start of PE32 image

    cmp     word [eax], 'MZ'
    jne     getEntryPointOfFfsFileErrorReturn
    movzx   ebx, word [eax + 0x3c]
    add     ebx, eax

    ; if (Hdr.Pe32->Signature == EFI_IMAGE_NT_SIGNATURE)
    cmp     dword [ebx], `PE\x00\x00`
    jne     getEntryPointOfFfsFileErrorReturn

    ; *EntryPoint = (VOID *)((UINTN)Pe32Data +
    ;   (UINTN)(Hdr.Pe32->OptionalHeader.AddressOfEntryPoint & 0x0ffffffff));
    add     eax, [ebx + 0x4 + 0x14 + 0x10]
    jmp     getEntryPointOfFfsFileReturn

getEntryPointOfFfsFileFoundTeSection:
    add     eax, 4       ; EAX = Start of TE image
    mov     ebx, eax

    ; if (Hdr.Te->Signature == EFI_TE_IMAGE_HEADER_SIGNATURE)
    cmp     word [ebx], 'VZ'
    jne     getEntryPointOfFfsFileErrorReturn
    ; *EntryPoint = (VOID *)((UINTN)Pe32Data +
    ;   (UINTN)(Hdr.Te->AddressOfEntryPoint & 0x0ffffffff) +
    ;   sizeof(EFI_TE_IMAGE_HEADER) - Hdr.Te->StrippedSize);
    add     eax, [ebx + 0x8]
    add     eax, 0x28
    movzx   ebx, word [ebx + 0x6]
    sub     eax, ebx
    jmp     getEntryPointOfFfsFileReturn

getEntryPointOfFfsFileErrorReturn:
    mov     eax, 0

getEntryPointOfFfsFileReturn:
    OneTimeCallRet GetEntryPointOfFfsFile

