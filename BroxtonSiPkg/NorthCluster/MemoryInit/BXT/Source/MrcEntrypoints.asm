;;@file
;
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2013 - 2016 Intel Corporation.
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
;**************************************************************************
;*                                                                        *
;*  PURPOSE:                                                              *
;*                                                                        *
;*      This file contains sample code to initialize processor cache      *
;*      and pass control to the Memory Reference Code                     *
;*                                                                        *
;**************************************************************************
IFNDEF EFI_MEMORY_INIT

.586P


.MODEL flat, SYSCALL
.CODE
;_TEXT32 SEGMENT PARA USE32 PUBLIC 'CODE32'

  PUBLIC _MrcEntryPoint_FAR
  extern @ConfigureMemory@4:      NEAR

; Single MRC entry point
_MrcEntryPoint_FAR PROC FAR PUBLIC
  push  ecx                 ; save *MrcData
  call  @ConfigureMemory@4  ; __fastcall calling convention means initial parameter pointer is in ecx
  pop   ecx
  retf
_MrcEntryPoint_FAR ENDP

ENDIF

END

