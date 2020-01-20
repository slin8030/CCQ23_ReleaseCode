;;******************************************************************************
;;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;;*
;;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;;* transmit, broadcast, present, recite, release, license or otherwise exploit
;;* any part of this publication in any form, by any means, without the prior
;;* written permission of Insyde Software Corporation.
;;*
;;******************************************************************************

;
; Data Types
;
UINT64      TYPEDEF     QWORD
UINT32      TYPEDEF     DWORD
UINT16      TYPEDEF     WORD
UINT8       TYPEDEF     BYTE
CHAR8       TYPEDEF     BYTE

; 
;  Define what a processor GDT looks like
;   Keep in sync with Thunk.c
; 
GDT_ENTRY   STRUCT 4
LimitLow    UINT16    ?
BaseLow     UINT16    ?
BaseMid     UINT8     ?
Attribute   UINT8     ?
LimitHi     UINT8     ?
BaseHi      UINT8     ?
GDT_ENTRY   ENDS

; 
; Define what a processor descriptor looks like
; Keep in sync with Thunk.c definition
; 
DESCRIPTOR64  STRUCT 1
Limit       UINT16      ?
Base        UINT64      ?
DESCRIPTOR64  ENDS

DESCRIPTOR32  STRUCT 1
Limit       UINT16      ?
Base        UINT32      ?
DESCRIPTOR32  ENDS

;
; LOW_MEMORY_THUNK - Keep in sync with Thunc.c definition
;
MEMORY_THUNK      STRUCT 2
x64GdtDesc            DESCRIPTOR64      <>
x64IdtDesc            DESCRIPTOR64      <>
x64Ss                 UINT64            ?
x64Esp                UINT64            ?
ia32Stack             UINT64            ?
ia32IdtDesc           DESCRIPTOR32      <>
ia32GdtDesc           DESCRIPTOR32      <>  
CodeSeg32offset       UINT16            ?
MEMORY_THUNK          ENDS

