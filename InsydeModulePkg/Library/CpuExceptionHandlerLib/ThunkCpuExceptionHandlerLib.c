/** @file
  IA-32 x64 specifc functionality for CPU Exception Handler library 
  implementition with Hook IRQ0 interrupt handler.
  
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <ThunkCpuExceptionHandlerLib.h>

STATIC UINT32 mFunctionPoint;
STATIC UINT32 mPeiServicesPoint;

/**
  Timer 0 Callback Function. Change cpu mode to IA-32 and execution PEI function.
  
  @param[in]  
  
  @retval None.
**/
STATIC
VOID 
PeiTimerCallbackFunction ( 
  VOID 
  )
{
  if (mFunctionPoint != 0) {
    Thunk64To32 (mPeiServicesPoint, mFunctionPoint);
  }
}

/**
  Hook one Cpu Interrupt/exceptions and provides the Interrupt/exceptions handlers.
  
  @param[in]  VectorInfo    Pointer to reserved vector list.
  
  @retval EFI_SUCCESS           CPU Exception Entries have been successfully initialized 
                                with default exception handlers.
  @retval EFI_INVALID_PARAMETER VectorInfo includes the invalid content if VectorInfo is not NULL.
  @retval EFI_UNSUPPORTED       This function is not supported.

**/
EFI_STATUS
EFIAPI
InitializeCpuExceptionHandlers (
  IN EFI_VECTOR_HANDOFF_INFO       *VectorInfo OPTIONAL
  )
{
  IA32_DESCRIPTOR             Idtr;
  UINT8                       Irq0Vector;
  UINT16                      CodeSegment;
  IA32_IDT_GATE_DESCRIPTOR    *GateDescriptor;
  EFI_HOB_GUID_TYPE           *GuidHob;
  H2O_PEI_TIMER_DATA_HOB      *DataInHob;

  //
  // Get thunk data.
  //
  GuidHob            = NULL;
  mPeiServicesPoint  = 0;
  mFunctionPoint     = 0;
  
  if (!FeaturePcdGet (PcdH2OPeiTimerSupported)) {
    return EFI_SUCCESS;
  }

  GuidHob = GetFirstGuidHob (&gH2OPeiTimerDataHobGuid);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }

  DataInHob         = GET_GUID_HOB_DATA (GuidHob);
  mPeiServicesPoint = DataInHob->PeiServicesPoint;
  mFunctionPoint    = DataInHob->CallBackFunction;
  Irq0Vector        = DataInHob->MasterBaseVector;
  
  //
  // Save CallbackFunction address
  //
  InitializeTimerCallbackPtr (PeiTimerCallbackFunction);

  //
  // Replace Interrupt Gate Descriptor
  //
  AsmReadIdtr ((IA32_DESCRIPTOR *) &Idtr);
  GateDescriptor = (IA32_IDT_GATE_DESCRIPTOR*)Idtr.Base;
  CodeSegment    = AsmReadCs ();
  
  GateDescriptor[Irq0Vector].Bits.OffsetLow   = (UINT16) (UINTN) InterruptEntry;
  GateDescriptor[Irq0Vector].Bits.OffsetHigh  = (UINT16) ((UINTN) InterruptEntry >> 16);
  GateDescriptor[Irq0Vector].Bits.OffsetUpper = (UINT32) ((UINTN) InterruptEntry >> 32);
  GateDescriptor[Irq0Vector].Bits.Selector    = CodeSegment;

  EnableInterrupts();
  return EFI_SUCCESS;
}

/**
  Initializes all CPU interrupt/exceptions entries and provides the default interrupt/exception handlers.
  
  Caller should try to get an array of interrupt and/or exception vectors that are in use and need to
  persist by EFI_VECTOR_HANDOFF_INFO defined in PI 1.3 specification.
  If caller cannot get reserved vector list or it does not exists, set VectorInfo to NULL. 
  If VectorInfo is not NULL, the exception vectors will be initialized per vector attribute accordingly.

  @param[in]  VectorInfo    Pointer to reserved vector list.
  
  @retval EFI_SUCCESS           All CPU interrupt/exception entries have been successfully initialized 
                                with default interrupt/exception handlers.
  @retval EFI_INVALID_PARAMETER VectorInfo includes the invalid content if VectorInfo is not NULL.
  @retval EFI_UNSUPPORTED       This function is not supported.

**/
EFI_STATUS
EFIAPI
InitializeCpuInterruptHandlers (
  IN EFI_VECTOR_HANDOFF_INFO       *VectorInfo OPTIONAL
  )
{
  return EFI_SUCCESS;
}

/**
  Registers a function to be called from the processor interrupt handler.

  This function registers and enables the handler specified by InterruptHandler for a processor 
  interrupt or exception type specified by InterruptType. If InterruptHandler is NULL, then the 
  handler for the processor interrupt or exception type specified by InterruptType is uninstalled. 
  The installed handler is called once for each processor interrupt or exception.
  NOTE: This function should be invoked after InitializeCpuExceptionHandlers() or
  InitializeCpuInterruptHandlers() invoked, otherwise EFI_UNSUPPORTED returned.

  @param[in]  InterruptType     Defines which interrupt or exception to hook.
  @param[in]  InterruptHandler  A pointer to a function of type EFI_CPU_INTERRUPT_HANDLER that is called
                                when a processor interrupt occurs. If this parameter is NULL, then the handler
                                will be uninstalled.

  @retval EFI_SUCCESS           The handler for the processor interrupt was successfully installed or uninstalled.
  @retval EFI_ALREADY_STARTED   InterruptHandler is not NULL, and a handler for InterruptType was
                                previously installed.
  @retval EFI_INVALID_PARAMETER InterruptHandler is NULL, and a handler for InterruptType was not
                                previously installed.
  @retval EFI_UNSUPPORTED       The interrupt specified by InterruptType is not supported,
                                or this function is not supported.
**/
EFI_STATUS
EFIAPI
RegisterCpuInterruptHandler (
  IN EFI_EXCEPTION_TYPE            InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER     InterruptHandler
  )
{
  return EFI_SUCCESS;
}

