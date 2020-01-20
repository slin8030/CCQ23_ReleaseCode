;;@file
;
;@copyright
;  INTEL CONFIDENTIAL
;  Copyright 2000 - 2016 Intel Corporation.
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


VacantFlag             EQU   00h
NotVacantFlag          EQU   0ffh
StartupApSignal        EQU   6E755200h
MonitorFilterSize      EQU   40h
ApCounterInit          EQU   0
ApInHltLoop            EQU   1
ApInMwaitLoop          EQU   2
ApInRunLoop            EQU   3
WakeUpApPerMwaitLoop32 Equ   4
WakeUpApPerRunLoop32   Equ   5

LockLocation           EQU   1000h - 0400h
StackStart             EQU   LockLocation + 4h
StackSize              EQU   LockLocation + 8h
RendezvousProc         EQU   LockLocation + 0Ch
GdtrProfile            EQU   LockLocation + 10h
IdtrProfile            EQU   LockLocation + 16h
BufferStart            EQU   LockLocation + 1Ch
Cr3Location            EQU   LockLocation + 20h
InitFlag               EQU   LockLocation + 24h
WakeUpApManner         EQU   LockLocation + 28h
BistBuffer             EQU   LockLocation + 2Ch
PAUSE32   MACRO
            DB         0F3h
            DB         090h
          ENDM

;-------------------------------------------------------------------------------
