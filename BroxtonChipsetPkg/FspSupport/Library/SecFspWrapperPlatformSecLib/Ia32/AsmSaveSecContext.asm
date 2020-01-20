;; @file
;  Save Sec Conext before call FspInit API
;
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2014 - 2016 Intel Corporation.
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
;  This file contains a 'Sample Driver' and is licensed as such under the terms
;  of your license agreement with Intel or your vendor. This file may be modified
;  by the user, subject to the additional terms of the license agreement.
;
; @par Specification
;;

.686p
.xmm
.model flat,c
.code

;----------------------------------------------------------------------------
;  MMX Usage:
;              MM0 = BIST State
;              MM5 = Save time-stamp counter value high32bit
;              MM6 = Save time-stamp counter value low32bit.
;
;  It should be same as SecEntry.asm and PeiCoreEntry.asm.
;----------------------------------------------------------------------------

AsmSaveBistValue   PROC PUBLIC
  mov     eax, [esp+4]
  movd    mm0, eax
  ret
AsmSaveBistValue   ENDP

AsmSaveTickerValue   PROC PUBLIC
  mov     eax, [esp+4]
  movd    mm6, eax
  mov     eax, [esp+8]
  movd    mm5, eax
  ret
AsmSaveTickerValue   ENDP

END
