##******************************************************************************
##* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
##*
##* You may not reproduce, distribute, publish, display, perform, modify, adapt,
##* transmit, broadcast, present, recite, release, license or otherwise exploit
##* any part of this publication in any form, by any means, without the prior
##* written permission of Insyde Software Corporation.
##*
##******************************************************************************
##
## Module Name:
##
##   CallSmmIhisi.S
##
## Abstract:
##
##   64 bit Sent SMI to call IHISI flash ROM part
##

#------------------------------------------------------------------------------
#  UINT8
#  GetPlatformInfo (
#    IN     UINT8                 *FbtsBuffer,
#    IN     UINT16                SmiPort
#    )#
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(GetPlatformInfo);
ASM_PFX(GetPlatformInfo):
  pushq   %rbx
  pushq   %rdi
  pushq   %rsi
  pushq   %r8

  movq    %rcx, %rdi
  movl    $0x2448324f, %ebx
  movw    $0x10ef, %ax
  outb    %al, %dx

  popq    %r8
  popq    %rsi
  popq    %rdi
  popq    %rbx
  ret
