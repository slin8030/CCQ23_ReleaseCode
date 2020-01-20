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
;    JumpToVector.Asm
;
; Abstract:
;       This is the ASM for Jump to a specific address
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


JumpToVector    PROTO  C \
   EntryPoint:PTR DWORD, \
   Parameter:DWORD
;
; Routine Description:
;       This allows the caller to switch the stack and goes to the new entry point
;
; Arguments:
;       EntryPoint      - Pointer to the location to enter
;       Parameter       - Parameter to pass in
;
; Returns:
;
;       Nothing. Goes to the Entry Point passing in the new parameters
;
JumpToVector    PROC  C \
  EntryPoint:PTR DWORD, \
  Parameter:DWORD
  
  push  ebx

  mov   ebx, Parameter
  mov   ecx, EntryPoint
  push  ebx
  push  0
  jmp   ecx
  
  pop   ebx
  ret
  
JumpToVector    ENDP
  END

