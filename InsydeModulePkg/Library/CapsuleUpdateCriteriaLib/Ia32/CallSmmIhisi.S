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
##   CallSmmIhisi.asm
##
## Abstract:
##
##   32 bit Sent SMI to call IHISI flash ROM part
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
  pushl   %ebp
  movl    %esp, %ebp
  pushl   %ebx
  pushl   %ecx
  pushl   %edx
  pushl   %edi
  pushl   %esi

  movl    $0x2448324f, %ebx
  movl    8(%ebp), %edi
  movl    12(%ebp), %edx
  movl    $0x10ef, %eax
  outb    %al, %dx

  popl    %esi
  popl    %edi
  popl    %edx
  popl    %ecx
  popl    %ebx
  movl    %ebp, %esp
  popl    %ebp
  ret
