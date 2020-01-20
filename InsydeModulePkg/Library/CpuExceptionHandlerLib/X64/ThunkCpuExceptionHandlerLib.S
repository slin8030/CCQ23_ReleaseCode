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

TimerCallbackFunctionPtr:  .quad  0

.text

#
# Save Timer0 callback function pointer.
#
ASM_GLOBAL ASM_PFX(InitializeTimerCallbackPtr)
ASM_PFX(InitializeTimerCallbackPtr):
	movq    %rcx, TimerCallbackFunctionPtr(%rip)	
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
	
	pushq	%rsp 
	pushq	%rbp 
	pushq	%rax 
	pushq	%rbx 
	pushq	%rcx 
	pushq	%rdx 
	pushq	%rsi 
	pushq	%rdi 
	pushq	%r8 
	pushq	%r9 
	pushq	%r10 
	pushq	%r11 
	pushq	%r12 
	pushq	%r13 
	pushq	%r14 
	pushq	%r15 
#   
#    8259 IRQ0 EOI
#     
	movb	$0x20, 	%al	  
	outb    %al, 	$0x20
	
	movq    TimerCallbackFunctionPtr(%rip), %rax	
	subq    $40, 	%rsp	
  	call    *%rax
	cli
	addq    $40, 	%rsp

	popq	%r15 
	popq	%r14 
	popq	%r13 
	popq	%r12 
	popq	%r11 
	popq	%r10 
	popq	%r9 
	popq	%r8 
	popq	%rdi 
	popq	%rsi 
	popq	%rdx 
	popq	%rcx 
	popq	%rbx 
	popq	%rax 
	popq	%rbp 
	popq	%rsp 
	
	iretq
