##@file
#
#@copyright
#  INTEL CONFIDENTIAL                                                            
#  Copyright 1999 - 2016 Intel Corporation.                                      
#                                                                                
#  The source code contained or described herein and all documents related to the
#  source code ("Material") are owned by Intel Corporation or its suppliers or   
#  licensors. Title to the Material remains with Intel Corporation or its supplie
#  and licensors. The Material may contain trade secrets and proprietary and     
#  confidential information of Intel Corporation and its suppliers and licensors,
#  and is protected by worldwide copyright and trade secret laws and treaty      
#  provisions. No part of the Material may be used, copied, reproduced, modified,
#  published, uploaded, posted, transmitted, distributed, or disclosed in any way
#  without Intel's prior express written permission.                             
#                                                                                
#  No license under any patent, copyright, trade secret or other intellectual    
#  property right is granted to or conferred upon you by disclosure or delivery  
#  of the Materials, either expressly, by implication, inducement, estoppel or   
#  otherwise. Any license under such intellectual property rights must be        
#  express and approved by Intel in writing.                                     
#                                                                                
#  Unless otherwise agreed by Intel in writing, you may not remove or alter      
#  this notice or any other notice embedded in Materials by Intel or             
#  Intel's suppliers or licensors in any way.                                    
#                                                                                
#  This file contains an 'Intel Peripheral Driver' and is uniquely identified as 
#  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under    
#  the terms of your license agreement with Intel or your vendor. This file may  
#  be modified by the user, subject to additional terms of the license agreement.
#
#@par Specification
##

.data

ExternalVectorTablePtr:            .space  4
CommonInterruptEntry:              .long   CommonEntry
Idtr:                              .space  2
Idtr1:                             .space  4

##include Htequ.inc

.equ    VacantFlag       ,              0x00
.equ    NotVacantFlag    ,              0xff
.equ    StartupApSignal  ,              0x6E755200
.equ    MonitorFilterSize,              0x40
.equ    ApCounterInit    ,              0
.equ    ApInHltLoop      ,              1
.equ    ApInMwaitLoop    ,              2
.equ    ApInRunLoop      ,              3
.equ    WakeUpApPerMwaitLoop32 ,        4
.equ    WakeUpApPerRunLoop32   ,        5
.equ    LockLocation     ,              0x1000 - 0x0400
.equ    StackStart       ,              LockLocation + 0x4
.equ    StackSize        ,              LockLocation + 0x8
.equ    RendezvousProc   ,              LockLocation + 0x0C
.equ    GdtrProfile      ,              LockLocation + 0x10
.equ    IdtrProfile      ,              LockLocation + 0x16
.equ    BufferStart      ,              LockLocation + 0x1C
.equ    Cr3Location      ,              LockLocation + 0x20
.equ    InitFlag         ,              LockLocation + 0x24 
.equ    WakeUpApManner   ,              LockLocation + 0x28
.equ    BistBuffer       ,              LockLocation + 0x2C

.macro  PAUSE32
        .byte 0xF3
        .byte 0x90
.endm

.text

#---------------------------------------#
# _InitializeIdt                #
#----------------------------------------------------------------------------#
# 
# Protocol prototype
#   InitializeIdt (
#   IN EFI_CPU_INTERRUPT_HANDLER TableStart,
#   IN UINTN                     *IdtTablePtr,
#   IN UINT16                    IdtLimit
#   )
#           
# Routine Description:
# 
#  Creates an IDT table starting at IdtTablPtr. It has IdtLimit/8 entries.
#  Table is initialized to intxx where xx is from 00 to number of entries or
#  100h, whichever is smaller. After table has been initialized the LIDT
#  instruction is invoked.
# 
#  TableStart is the pointer to the callback table and is not used by 
#  InitializedIdt but by commonEntry. CommonEntry handles all interrupts,
#  does the context save and calls the callback entry, if non-NULL.
#  It is the responsibility of the callback routine to do hardware EOIs.
# 
# Arguments:
# 
#   TableStart          - Pointer to interrupt callback table
#
#   IdtTablePtr         - Pointer to IDT table
#
#   IdtLimit            - IDT Table limit = number of interrupt entries * 8
# 
# Returns: 
# 
#   Nothing
#
# 
# Input:  [ebp][0]  = Original ebp
#         [ebp][4]  = Return address
#         [ebp][8]  = TableStart
#         [ebp][0c] = *IdtTablePtr
#         [ebp][10] = IdtLimit
#          
# Output: Nothing
#           
# Destroys: Nothing
#-----------------------------------------------------------------------------#

ASM_GLOBAL ASM_PFX(InitializeIdt)
ASM_PFX(InitializeIdt):
  pushl   %ebp              # C prolog
  movl    %esp,%ebp
  pushl   %edi

  movl    8(%ebp),%eax                  # Get ExternalVectorTable Address
  movl    %eax, ExternalVectorTablePtr

  movw    0x10(%ebp),%ax                # Get IDT Table limit
  decw    %ax
  movw    %ax, Idtr			# Store %ax to Idtr

  movl    0xc(%ebp),%eax                # Get Start of IDT
  movl    %eax, Idtr1

  movl    $Idtr, %edi			# Addr of Idtr -> %edi
  lidt    %es:(%edi)

  popl    %edi
  popl    %ebp
  ret

#----------------------------------------------------------------------------#
# 
# Protocol prototype
#   None
#           
# Routine Description:
# 
#  These routines handle the individual interrupts. These routines always
#  gain control on any interrupt or exception. They save EAX and place
#  the interrupt number in EAX. CommonEntry is then jumped to. 
#  instruction is invoked.
# 
#  CommonEntry handles all interrupts,does the context save and calls the 
#  callback entry, if non-NULL. It is the responsibility of the callback 
#  routine to do hardware EOIs. Callbacks are entered into the table
#  located at TableStart. Entries are modified by the InstallInterruptHandler
#  and UninstallInterruptHandler protocols.
# 
# Arguments to CommonEntry:
# 
#   EAX                 - Interrupt or exception number
#
#   TableStart          - Pointer to interrupt callback table
# 
# Returns: 
# 
#   Nothing
#
# 
# Output: Nothing
#           
# Destroys: Nothing
#-----------------------------------------------------------------------------#

TemplateStart: 
   pushl %eax

   #mov  eax, 0nnh (nn stands for vector number, which will be fixed at runtime 
   .byte 0xb8
VectorNumber:   
   .long 0x0

   jmp  *CommonInterruptEntry
TemplateEnd: 

CommonEntry: 

#---------------------------------------#
# _CommonEntry                  #
#----------------------------------------------------------------------------#
# The follow algorithm is used for the common interrupt routine.
# Entry from each interrupt with a push eax and eax=interrupt number

#
# +---------------------+
# +    EFlags           +
# +---------------------+
# +    CS               +
# +---------------------+
# +    EIP              +
# +---------------------+
# +    Error Code       +
# +---------------------+
# + EAX / Vector Number +
# +---------------------+
# +    EBP              +
# +---------------------+ <-- EBP
#

  cli
  #
  # All interrupt handlers are invoked through interrupt gates, so
  # IF flag automatically cleared at the entry point
  #
  cmpl    $32,%eax        # Intel reserved vector for exceptions?
  jae     NoErrorCode
  btl     %eax, %cs:ASM_PFX(mErrorCodeFlag)
  jc      L1

NoErrorCode: 
  #
  # Push a dummy error code on the stack
  # to maintain coherent stack map
  #
  pushl   (%esp)	# Push the value %esp pointing to
  movl    $0, 4(%esp)
L1:
  pushl   %ebp
  movl    %esp,%ebp

  #
  # Align stack to make sure that EFI_FX_SAVE_STATE_IA32 of EFI_SYSTEM_CONTEXT_IA32
  # is 16-byte aligned
  #
  andl    $0xfffffff0,%esp
  subl    $12,%esp

## UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax#
  pushl   0x4(%ebp)
  pushl   %ecx
  pushl   %edx
  pushl   %ebx
  leal    24(%ebp),%ecx
  pushl   %ecx                         # ESP
  pushl   (%ebp)
  pushl   %esi
  pushl   %edi

  movl    %eax,4(%ebp)                 # save vector number

## UINT32  Gs, Fs, Es, Ds, Cs, Ss#
  movl %ss,%eax
  pushl %eax
  movzwl 16(%ebp), %eax
  pushl %eax
  movl %ds,%eax
  pushl %eax
  movl %es,%eax
  pushl %eax
  movl %fs,%eax
  pushl %eax
  movl %gs,%eax
  pushl %eax

## UINT32  Eip#
  pushl    12(%ebp)

## UINT32  Gdtr[2], Idtr[2]#
  subl $8,%esp
  sidt (%esp)
  subl $8,%esp
  sgdt (%esp)

## UINT32  Ldtr, Tr#
  xorl %eax,%eax
  strl %eax
  pushl %eax
  sldtl %eax
  pushl %eax

## UINT32  EFlags#
  pushl    20(%ebp)

## UINT32  Cr0, Cr1, Cr2, Cr3, Cr4#
  movl %cr4, %eax
  orl  $0x208,%eax
  movl %eax, %cr4
  pushl %eax
  movl %cr3, %eax
  pushl %eax
  movl %cr2, %eax
  pushl %eax
  xorl %eax,%eax
  pushl %eax
  movl %cr0, %eax
  pushl %eax

## UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7#
  movl    %dr7, %eax
  pushl   %eax
## clear Dr7 while executing debugger itself
  xorl    %eax, %eax
  movl    %eax, %dr7

  movl    %dr6, %eax
  pushl   %eax
## insure all status bits in dr6 are clear...
  xorl    %eax, %eax
  movl    %eax, %dr6

  movl    %dr3, %eax
  pushl   %eax
  movl    %dr2, %eax
  pushl   %eax
  movl    %dr1, %eax
  pushl   %eax
  movl    %dr0, %eax
  pushl   %eax

## FX_SAVE_STATE_IA32 FxSaveState#
  subl $512, %esp
  movl %esp, %edi
  .byte 0x0f, 0xae, 0x07

## UINT32  ExceptionData#
  pushl   8(%ebp)

## call into exception handler
  movl    4(%ebp), %ebx
  movl    ExternalVectorTablePtr, %eax
  movl    (%eax,%ebx,4), %eax
  orl     %eax, %eax               # NULL?
  je  nonNullValue #

## Prepare parameter and call
  movl    %esp, %edx
  pushl   %edx
  pushl   %ebx
  call    *%eax
  addl    $8, %esp

nonNullValue: 
  cli

## UINT32  ExceptionData#
  addl $4,%esp

## FX_SAVE_STATE_IA32 FxSaveState#
  movl %esp, %esi
  .byte 0x0f, 0xae, 0x0e
  addl $512, %esp

## UINT32  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7#
  popl    %eax
  movl    %eax, %dr0
  popl    %eax
  movl    %eax, %dr1
  popl    %eax
  movl    %eax, %dr2
  popl    %eax
  movl    %eax, %dr3
## skip restore of dr6.  We cleared dr6 during the context save.
  addl     $4, %esp
  popl     %eax
  movl     %eax, %dr7

## UINT32  Cr0, Cr1, Cr2, Cr3, Cr4#
  popl    %eax
  movl    %eax, %cr0
  addl    $4,%esp   # not for Cr1
  popl    %eax
  movl    %eax, %cr2
  popl    %eax
  movl    %eax, %cr3
  popl    %eax
  movl    %eax, %cr4

## UINT32  EFlags#
  popl    20(%ebp)

## UINT32  Ldtr, Tr#
## UINT32  Gdtr[2], Idtr[2]#
## Best not let anyone mess with these particular registers...
  addl    $24,%esp

## UINT32  Eip#
   pop     12(%ebp)

## UINT32  Gs, Fs, Es, Ds, Cs, Ss#
## NOTE - modified segment registers could hang the debugger...  We
##        could attempt to insulate ourselves against this possibility,
##        but that poses risks as well.
##
  popl    %gs
  popl    %fs
  popl    %es
  popl    %ds
  popl    16(%ebp)
  popl    %ss

## UINT32  Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax#
  popl    %edi
  popl    %esi
  addl    $4, %esp  # not for ebp
  addl    $4, %esp  # not for esp
  popl    %ebx
  popl    %edx
  popl    %ecx
  popl    %eax

  movl    %ebp, %esp
  popl    %ebp
  addl    $8,%esp
  iretl


#---------------------------------------#
# _GetTemplateAddressMap                  #
#----------------------------------------------------------------------------#
# 
# Protocol prototype
#   GetTemplateAddressMap (
#     INTERRUPT_HANDLER_TEMPLATE_MAP *AddressMap
#   )#
#           
# Routine Description:
# 
#  Return address map of interrupt handler template so that C code can generate
#  interrupt handlers, and dynamically do address fix.
# 
# Arguments:
# 
# 
# Returns: 
# 
#   Nothing
#
# 
# Input:  [ebp][0]  = Original ebp
#         [ebp][4]  = Return address
#          
# Output: Nothing
#           
# Destroys: Nothing
#-----------------------------------------------------------------------------#
ASM_GLOBAL ASM_PFX(GetTemplateAddressMap)
ASM_PFX(GetTemplateAddressMap):
  pushl   %ebp                # C prolog
  movl    %esp,%ebp
  pushal

  movl 8(%ebp), %ebx
  movl $TemplateStart, (%ebx)
  movl $(TemplateEnd - TemplateStart), 4(%ebx)

  # Note: if code in Template is updated, the value fills into the 3rd parameter 
  # also needs update
  movl $(VectorNumber - TemplateStart), 8(%ebx)

  popal
  popl    %ebp
  ret



#---------------------------------------#
# _InitializeSelectors                  #
#----------------------------------------------------------------------------#
# 
# Protocol prototype
#   InitializeSelectors (
#   )
#           
# Routine Description:
# 
#  Creates an new GDT in RAM.  The problem is that our former selectors
#  were ROM based and the EFI OS Loader does not manipulate the machine state 
#  to change them (as it would for a 16-bit PC/AT startup code that had to
#  go from Real Mode to flat mode).
# 
# Arguments:
# 
# 
# Returns: 
# 
#   Nothing
#
# 
# Input:  [ebp][0]  = Original ebp
#         [ebp][4]  = Return address
#          
# Output: Nothing
#           
# Destroys: Nothing
#-----------------------------------------------------------------------------#

.equ          CODE_SELECTOR, 0x10
.equ          DATA_SELECTOR, 0x18

ASM_GLOBAL ASM_PFX(InitializeSelectors)
ASM_PFX(InitializeSelectors):
  pushl   %ebp                # C prolog
  movl    %esp,%ebp
  pushal
  movl    $Gdtr, %edi

  movw    %cs,%ax             # Get the selector data from our code image          
  .byte 0x66
  movw    %ax,%es
  lgdt    %es:(%edi)

  .byte 0x67
  .byte 0xea                # Far Jump Offset:Selector to reload CS
  .long      SelectorRld
  .word      CODE_SELECTOR
SelectorRld:
  movw    $DATA_SELECTOR, %ax # Update the Base for the new selectors, too
  .byte 0x66
  movw    %ax,%ds
  .byte 0x66
  movw    %ax,%es
  .byte 0x66
  movw    %ax,%fs
  .byte 0x66
  movw    %ax,%gs
  .byte 0x66
  movw    %ax,%ss

  popal
  popl    %ebp
  ret

#------------------------------------------------------------------------------
#  VOID
#  CpuEnableInterrupt (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(CpuEnableInterrupt)
ASM_PFX(CpuEnableInterrupt):
    sti
    ret
#CpuEnableInterrupt ENDP


#------------------------------------------------------------------------------
#  VOID
#  CpuDisableInterrupt (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(CpuDisableInterrupt)
ASM_PFX(CpuDisableInterrupt):
    cli
    ret
#CpuDisableInterrupt ENDP  

#------------------------------------------------------------------------------
#  VOID
#  CpuInitFloatPointUnit (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(CpuInitFloatPointUnit)
ASM_PFX(CpuInitFloatPointUnit):
    finit
    ret
#CpuInitFloatPointUnit ENDP

#------------------------------------------------------------------------------
#  UINT16
#  GetCodeSegment (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(GetCodeSegment)
ASM_PFX(GetCodeSegment):
    movw  %cs, %ax
    ret
#GetCodeSegment ENDP  


#------------------------------------------------------------------------------
#  VOID
#  EfiWbinvd (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(EfiWbinvd)
ASM_PFX(EfiWbinvd):
    wbinvd
    ret
#EfiWbinvd ENDP

#------------------------------------------------------------------------------
#  VOID
#  EfiInvd (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(EfiInvd)
ASM_PFX(EfiInvd):
    invd
    ret
#EfiInvd ENDP

#------------------------------------------------------------------------------
#  VOID
#  GetIdt (
#    IDT_INFORMATION   *IdtInfo
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(GetIdt)
ASM_PFX(GetIdt):
  push    %ebp                    # C prolog
        
  movl    %esp, %ebp
  movl    8(%ebp), %eax
  sidt    (%eax)

  popl    %ebp
  ret
#GetIdt ENDP

#------------------------------------------------------------------------------
#  VOID
#  C1eExceptionHandler (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(C1eExceptionHandler)
ASM_PFX(C1eExceptionHandler):
  pushl    %ebp             # C prolog
  movl     %esp, %ebp
  pushfl
  cli
  pushal

  # Verify if GPE was caused by C1e write.  
  # If not, pass control to real exception handler.
  cmp    $0, ASM_PFX(mWroteMsr)
  je     notourexception

  # Fix the return address on stack to skip offending
  # code which caused the exception.
  movl   8(%ebp), %eax
  addl   $2, %eax
  movl   %eax, 8(%ebp)
  jmp    exit

notourexception:
  popal
  popfl
  popl    %ebp

  jmp    ASM_PFX(mOriginalInt13)

exit:

  popal
  popfl
  popl    %ebp
  addl    $4, %esp
  iretl
#C1eExceptionHandler ENDP


#------------------------------------------------------------------------------
#  UINT8
#  GetCoreNumber (
#    VOID
#    )
#------------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(GetCoreNumber)
ASM_PFX(GetCoreNumber):

    pushl %ebx
    
    movl  $4, %eax
    movl  $0, %ecx
    cpuid
    
    shrl  $26, %eax
    andl  $0x3f, %eax
    incb  %al
    
    popl  %ebx
    
    ret
    
#GetCoreNumber ENDP

#-----------------------------------------------------------------------------#
# data
#-----------------------------------------------------------------------------#

        .p2align 4

Gdtr:    .word GDT_END - GDT_BASE - 1
         .long GDT_BASE

#-----------------------------------------------------------------------------#
#   global descriptor table (GDT)
#-----------------------------------------------------------------------------#

        .p2align 4

GDT_BASE: 
# null descriptor
# .equ                NULL_SEL, $-GDT_BASE # Selector [0]
        .word 0         # limit 15:0
        .word 0         # base 15:0
        .byte 0         # base 23:16
        .byte 0         # type
        .byte 0         # limit 19:16, flags
        .byte 0         # base 31:24

# linear data segment descriptor
# .equ            LINEAR_SEL, $-GDT_BASE # Selector [0x8]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF              # page-granular, 32-bit
        .byte 0

# linear code segment descriptor
# .equ            LINEAR_CODE_SEL, $-GDT_BASE # Selector [0x10]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, data, expand-up, writable
        .byte 0xCF              # page-granular, 32-bit
        .byte 0

# system data segment descriptor
# .equ            SYS_DATA_SEL, $-GDT_BASE # Selector [0x18]
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x92      # present, ring 0, data, expand-up, writable
        .byte 0xCF              # page-granular, 32-bit
        .byte 0

# system code segment descriptor
# .equ            SYS_CODE_SEL, $-GDT_BASE
        .word 0xFFFF    # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0x9A      # present, ring 0, data, expand-up, writable
        .byte 0xCF              # page-granular, 32-bit
        .byte 0

# spare segment descriptor
# .equ        SPARE3_SEL, $-GDT_BASE
        .word 0         # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0         # present, ring 0, data, expand-up, writable
        .byte 0         # page-granular, 32-bit
        .byte 0

# spare segment descriptor
# .equ        SPARE4_SEL, $-GDT_BASE
        .word 0         # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0         # present, ring 0, data, expand-up, writable
        .byte 0         # page-granular, 32-bit
        .byte 0

# spare segment descriptor
# .equ        SPARE5_SEL, $-GDT_BASE
        .word 0         # limit 0xFFFFF
        .word 0         # base 0
        .byte 0
        .byte 0         # present, ring 0, data, expand-up, writable
        .byte 0         # page-granular, 32-bit
        .byte 0

GDT_END: 

