#******************************************************************************
#* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
#*
#* You may not reproduce, distribute, publish, display, perform, modify, adapt,
#* transmit, broadcast, present, recite, release, license or otherwise exploit
#* any part of this publication in any form, by any means, without the prior
#* written permission of Insyde Software Corporation.
#*
#******************************************************************************

.data

TimerCallbackFunctionPtr:  .space  4

.text

ASM_GLOBAL ASM_PFX(InitializeTimerCallbackPtr)
ASM_PFX(InitializeTimerCallbackPtr):
  pushl   %ebp                                  # C prolog
  movl    %esp,		%ebp

  movl    8(%ebp),	%eax                  	# Get ExternalVectorTable Address
  movl    %eax, 	ExternalVectorTablePtr

  popl    %ebp
  ret


#
#---------------------------------------#
# InterruptEntry                        #
#---------------------------------------#
# IRQ0 interrupt serivce routine.
#
ASM_GLOBAL ASM_PFX(InterruptEntry)
ASM_PFX(InterruptEntry):
	cli
	
	pushl	%esp 
	pushl	%ebp 
	pushl	%eax 
	pushl	%ebx 
	pushl	%ecx 
	pushl	%edx 
	pushl	%esi 
	pushl	%edi 

#  
# 8259 IRQ0 EOI
#    
	movb	$0x20, 	%al	  
	outb    %al, 	$0x20
	
	movl    TimerCallbackFunctionPtr, %eax	

  	call    *%eax
	cli

	popl	%edi 
	popl	%esi 
	popl	%edx 
	popl	%ecx 
	popl	%ebx 
	popl	%eax 
	popl	%ebp 
	popl	%esp 
	
	iretl
	