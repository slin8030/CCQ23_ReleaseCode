#******************************************************************************
#* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

#
# define the structure of DESCRIPTOR64
#
.set  Limit64,         0      #size 2
.set  Base64,          2      #size 8
.set  DESCRIPTOR64,    10

#
# define the structure of DESCRIPTOR32
#
.set  Limit32,         0      #size 2
.set  Base32,          2      #size 4
.set  DESCRIPTOR32,    6

#
# define the structure of MEMORY_THUNK
#
.set  x64GdtDesc,                  0    #size 10
.set  x64IdtDesc,                  10   #size 10
.set  x64Ss,                       20   #size 8
.set  x64Esp,                      28   #size 8
.set  ia32Stack,                   36   #size 8
.set  ia32IdtDesc,                 44   #size 6
.set  ia32GdtDesc,                 50   #size 6 
.set  CodeSeg32,                   56   #size 2
.set  MemThunkSize,                58

ASM_GLOBAL  ASM_PFX(gIA32Cr3)

.text

##############################################
#VOID
#InternalThunk64To32 (
#  IN MEMORY_THUNK  *IntThunk,
#  IN UINT32        FunctionPoint
#  IN UINT32        PeiServicesPoint
#  );
##############################################
ASM_GLOBAL ASM_PFX(InternalThunk64To32);
ASM_PFX(InternalThunk64To32):

    pushl    %ebp
    movl     %esp, %ebp
    pushl    %ebx
    pushl    %ecx
    pushl    %edx
    pushl    %edi
    pushl    %esi

    movl     8(%ebp), %ecx
    movl     12(%ebp), %edx
    movl     %esp, %ebx
    
    movl     ia32Stack(%ecx), %esp
	
    pushl    %ebx
    
    movl     16(%ebp), %eax
    pushl    %eax
    
    call     *%edx
    
    popl     %eax
	
    popl     %ebx    
    movl     %ebx, %esp
    
    popl     %esi
    popl     %edi
    popl     %edx
    popl     %ecx
    popl     %ebx
    popl     %ebp

    ret