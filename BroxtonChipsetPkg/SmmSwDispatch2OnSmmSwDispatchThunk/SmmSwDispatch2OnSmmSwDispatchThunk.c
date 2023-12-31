/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  SMM SwDispatch2 Protocol on SMM SwDispatch Protocol Thunk driver.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <PiDxe.h>
#include <FrameworkSmm.h>

#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmSwDispatch.h>
//[-start-151124-IB10860187-modify]//
#include <Protocol/SmmControl2.h>
//[-end-151124-IB10860187-modify]//
#include <Protocol/SmmCpu.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>

typedef struct {
  LIST_ENTRY                     Link;
  EFI_HANDLE                     DispatchHandle;
  UINTN                          SwSmiInputValue;
  UINTN                          DispatchFunction;
} EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT;

/**
  Register a child SMI source dispatch function for the specified software SMI.

  This service registers a function (DispatchFunction) which will be called when the software 
  SMI source specified by RegisterContext->SwSmiCpuIndex is detected. On return, 
  DispatchHandle contains a unique handle which may be used later to unregister the function 
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_SW_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchFunction      Function to register for handler when the specified software 
                                    SMI is generated. 
  @param[in, out]  RegisterContext  Pointer to the dispatch function's context.
                                    The caller fills this context in before calling
                                    the register function to indicate to the register
                                    function which Software SMI input value the
                                    dispatch function should be invoked for.
  @param[out] DispatchHandle        Handle generated by the dispatcher to track the
                                    function instance.

  @retval EFI_SUCCESS            The dispatch function has been successfully
                                 registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR       The SW driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER  RegisterContext is invalid. The SW SMI input value
                                 is not within valid range.
  @retval EFI_OUT_OF_RESOURCES   There is not enough memory (system or SMM) to manage this
                                 child.
  @retval EFI_OUT_OF_RESOURCES   A unique software SMI value could not be assigned
                                 for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmSwDispatch2Register (
  IN  CONST EFI_SMM_SW_DISPATCH2_PROTOCOL  *This,
  IN        EFI_SMM_HANDLER_ENTRY_POINT2   DispatchFunction,
  IN  OUT   EFI_SMM_SW_REGISTER_CONTEXT    *RegisterContext,
  OUT       EFI_HANDLE                     *DispatchHandle
  );

/**
  Unregister a child SMI source dispatch function for the specified software SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be 
  called in response to a software SMI.

  @param[in] This                Pointer to the EFI_SMM_SW_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmSwDispatch2UnRegister (
  IN CONST EFI_SMM_SW_DISPATCH2_PROTOCOL  *This,
  IN       EFI_HANDLE                     DispatchHandle
  );

EFI_SMM_SW_DISPATCH2_PROTOCOL gSmmSwDispatch2 = {
  SmmSwDispatch2Register,
  SmmSwDispatch2UnRegister,
  0 // MaximumSwiValue
};

EFI_SMM_SW_DISPATCH_PROTOCOL  *mSmmSwDispatch;
UINT8                         mSmiTriggerRegister;
UINT8                         mSmiDataRegister;

EFI_SMM_CPU_PROTOCOL          *mSmmCpuProtocol;
LIST_ENTRY                    mSmmSwDispatch2ThunkQueue = INITIALIZE_LIST_HEAD_VARIABLE (mSmmSwDispatch2ThunkQueue);

/**
  This function find SmmSwDispatch2Context by SwSmiInputValue.

  @param SwSmiInputValue The SwSmiInputValue to indentify the SmmSwDispatch2 context

  @return SmmSwDispatch2 context
**/
EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT *
FindSmmSwDispatch2ContextBySwSmiInputValue (
  IN UINTN   SwSmiInputValue
  )
{
  LIST_ENTRY                            *Link;
  EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT    *ThunkContext;

  for (Link = mSmmSwDispatch2ThunkQueue.ForwardLink;
       Link != &mSmmSwDispatch2ThunkQueue;
       Link = Link->ForwardLink) {
    ThunkContext = BASE_CR (
                     Link,
                     EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT,
                     Link
                     );
    if (ThunkContext->SwSmiInputValue == SwSmiInputValue) {
      return ThunkContext;
    }
  }
  return NULL;
}

/**
  This function find SmmSwDispatch2Context by DispatchHandle.

  @param DispatchHandle The DispatchHandle to indentify the SmmSwDispatch2Thunk context

  @return SmmSwDispatch2Thunk context
**/
EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT *
FindSmmSwDispatch2ContextByDispatchHandle (
  IN EFI_HANDLE   DispatchHandle
  )
{
  LIST_ENTRY                            *Link;
  EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT    *ThunkContext;

  for (Link = mSmmSwDispatch2ThunkQueue.ForwardLink;
       Link != &mSmmSwDispatch2ThunkQueue;
       Link = Link->ForwardLink) {
    ThunkContext = BASE_CR (
                     Link,
                     EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT,
                     Link
                     );
    if (ThunkContext->DispatchHandle == DispatchHandle) {
      return ThunkContext;
    }
  }
  return NULL;
}

/**
  Framework dispatch function for a Software SMI handler.

  @param  DispatchHandle        The handle of this dispatch function.
  @param  DispatchContext       The pointer to the dispatch function's context.
                                The SwSmiInputValue field is filled in
                                by the software dispatch driver prior to
                                invoking this dispatch function.
                                The dispatch function will only be called
                                for input values for which it is registered.

  @return None

**/
VOID
EFIAPI
FrameworkDispatchFunction (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
{
  EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT    *ThunkContext;
  EFI_SMM_HANDLER_ENTRY_POINT2          DispatchFunction;
  EFI_SMM_SW_REGISTER_CONTEXT           RegisterContext;
  EFI_SMM_SW_CONTEXT                    SwContext;
  UINTN                                 Size;
  UINTN                                 Index;
  EFI_SMM_SAVE_STATE_IO_INFO            IoInfo;
  EFI_STATUS                            Status;

  //
  // Search context
  //
  ThunkContext = FindSmmSwDispatch2ContextBySwSmiInputValue (DispatchContext->SwSmiInputValue);
  ASSERT (ThunkContext != NULL);
  if (ThunkContext == NULL) {
    return ;
  }

  //
  // Construct new context
  //
  RegisterContext.SwSmiInputValue = DispatchContext->SwSmiInputValue;
  Size = sizeof(SwContext);
  SwContext.CommandPort = IoRead8 (mSmiTriggerRegister);
  SwContext.DataPort    = IoRead8 (mSmiDataRegister);

  //
  // Try to find which CPU trigger SWSMI
  //
  SwContext.SwSmiCpuIndex = 0;
  for (Index = 0; Index < gSmst->NumberOfCpus; Index++) {
    Status = mSmmCpuProtocol->ReadSaveState (
                                mSmmCpuProtocol,
                                sizeof(IoInfo),
                                EFI_SMM_SAVE_STATE_REGISTER_IO,
                                Index,
                                &IoInfo
                                );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if (IoInfo.IoPort == mSmiTriggerRegister) {
      //
      // Great! Find it.
      //
      SwContext.SwSmiCpuIndex = Index;
      break;
    }
  }

  //
  // Dispatch
  //
  DispatchFunction = (EFI_SMM_HANDLER_ENTRY_POINT2)ThunkContext->DispatchFunction;
  DispatchFunction (
    DispatchHandle,
    &RegisterContext,
    &SwContext,
    &Size
    );
  return ;
}

/**
  Register a child SMI source dispatch function for the specified software SMI.

  This service registers a function (DispatchFunction) which will be called when the software 
  SMI source specified by RegisterContext->SwSmiCpuIndex is detected. On return, 
  DispatchHandle contains a unique handle which may be used later to unregister the function 
  using UnRegister().

  @param[in]  This                  Pointer to the EFI_SMM_SW_DISPATCH2_PROTOCOL instance.
  @param[in]  DispatchFunction      Function to register for handler when the specified software 
                                    SMI is generated. 
  @param[in, out]  RegisterContext  Pointer to the dispatch function's context.
                                    The caller fills this context in before calling
                                    the register function to indicate to the register
                                    function which Software SMI input value the
                                    dispatch function should be invoked for.
  @param[out] DispatchHandle        Handle generated by the dispatcher to track the
                                    function instance.

  @retval EFI_SUCCESS            The dispatch function has been successfully
                                 registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR       The SW driver was unable to enable the SMI source.
  @retval EFI_INVALID_PARAMETER  RegisterContext is invalid. The SW SMI input value
                                 is not within valid range.
  @retval EFI_OUT_OF_RESOURCES   There is not enough memory (system or SMM) to manage this
                                 child.
  @retval EFI_OUT_OF_RESOURCES   A unique software SMI value could not be assigned
                                 for this dispatch.
**/
EFI_STATUS
EFIAPI
SmmSwDispatch2Register (
  IN  CONST EFI_SMM_SW_DISPATCH2_PROTOCOL  *This,
  IN        EFI_SMM_HANDLER_ENTRY_POINT2   DispatchFunction,
  IN  OUT   EFI_SMM_SW_REGISTER_CONTEXT    *RegisterContext,
  OUT       EFI_HANDLE                     *DispatchHandle
  )
{
  EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT    *ThunkContext;
  EFI_SMM_SW_DISPATCH_CONTEXT           DispatchContext;
  EFI_STATUS                            Status;
  UINTN                                 Index;

  if (RegisterContext->SwSmiInputValue == (UINTN)-1) {
    //
    // If SwSmiInputValue is set to (UINTN) -1 then a unique value will be assigned and returned in the structure.
    //
    Status = EFI_NOT_FOUND;
    for (Index = 1; Index < gSmmSwDispatch2.MaximumSwiValue; Index++) {
      DispatchContext.SwSmiInputValue = Index;
      Status = mSmmSwDispatch->Register (
                                 mSmmSwDispatch,
                                 FrameworkDispatchFunction,
                                 &DispatchContext,
                                 DispatchHandle
                                 );
      if (!EFI_ERROR (Status)) {
        RegisterContext->SwSmiInputValue = Index;
        break;
      }
    }
    if (RegisterContext->SwSmiInputValue == (UINTN)-1) {
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    DispatchContext.SwSmiInputValue = RegisterContext->SwSmiInputValue;
    Status = mSmmSwDispatch->Register (
                               mSmmSwDispatch,
                               FrameworkDispatchFunction,
                               &DispatchContext,
                               DispatchHandle
                               );
  }
  if (!EFI_ERROR (Status)) {
    //
    // Register
    //
    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      sizeof(*ThunkContext),
                      (VOID **)&ThunkContext
                      );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      mSmmSwDispatch->UnRegister (mSmmSwDispatch, *DispatchHandle);
      return EFI_OUT_OF_RESOURCES;
    }

    ThunkContext->SwSmiInputValue  = RegisterContext->SwSmiInputValue;
    ThunkContext->DispatchFunction = (UINTN)DispatchFunction;
    ThunkContext->DispatchHandle   = *DispatchHandle;
    InsertTailList (&mSmmSwDispatch2ThunkQueue, &ThunkContext->Link);
  }

  return Status;
}

/**
  Unregister a child SMI source dispatch function for the specified software SMI.

  This service removes the handler associated with DispatchHandle so that it will no longer be 
  called in response to a software SMI.

  @param[in] This                Pointer to the EFI_SMM_SW_DISPATCH2_PROTOCOL instance.
  @param[in] DispatchHandle      Handle of dispatch function to deregister.

  @retval EFI_SUCCESS            The dispatch function has been successfully unregistered.
  @retval EFI_INVALID_PARAMETER  The DispatchHandle was not valid.
**/
EFI_STATUS
EFIAPI
SmmSwDispatch2UnRegister (
  IN CONST EFI_SMM_SW_DISPATCH2_PROTOCOL  *This,
  IN       EFI_HANDLE                     DispatchHandle
  )
{
  EFI_SMM_SW_DISPATCH2_THUNK_CONTEXT    *ThunkContext;
  EFI_STATUS                            Status;

  Status = mSmmSwDispatch->UnRegister (mSmmSwDispatch, DispatchHandle);
  if (!EFI_ERROR (Status)) {
    //
    // Unregister
    //
    ThunkContext = FindSmmSwDispatch2ContextByDispatchHandle (DispatchHandle);
    ASSERT (ThunkContext != NULL);
    if (ThunkContext != NULL) {
      RemoveEntryList (&ThunkContext->Link);
      gSmst->SmmFreePool (ThunkContext);
    }
  }

  return Status;
}

/**
  Entry Point for this thunk driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  A Pointer to the EFI System Table.

  @retval EFI_SUCCESS  The entry point is executed successfully.
  @retval other        Some error occurred when executing this entry point.
**/
EFI_STATUS
EFIAPI
SmmSwDispatch2ThunkMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
//[-start-151124-IB10860187-modify]//
  EFI_STATUS                  Status;
  EFI_SMM_CONTROL2_PROTOCOL  *SmmControl;
//EFI_SMM_CONTROL_REGISTER RegisterInfo;
//[-end-151124-IB10860187-modify]//

  ///
  /// Locate Framework SMM SwDispatch Protocol
  ///
  Status = gBS->LocateProtocol (&gEfiSmmSwDispatchProtocolGuid, NULL, (VOID **)&mSmmSwDispatch);
  ASSERT_EFI_ERROR (Status);
  gSmmSwDispatch2.MaximumSwiValue = mSmmSwDispatch->MaximumSwiValue;
  if (gSmmSwDispatch2.MaximumSwiValue == 0x0) {
    DEBUG ((EFI_D_ERROR, "BUGBUG: MaximumSwiValue is 0, work-around to make it 0xFF\n"));
    gSmmSwDispatch2.MaximumSwiValue = 0xFF;
  }

  ///
  /// Locate Framework SMM Control Protocol
  ///
//[-start-151124-IB10860187-modify]//
  Status = gBS->LocateProtocol (&gEfiSmmControl2ProtocolGuid, NULL, (VOID **)&SmmControl);
  ASSERT_EFI_ERROR (Status);
//Status = SmmControl->GetRegisterInfo (SmmControl, &RegisterInfo);
  ASSERT_EFI_ERROR (Status);
  mSmiTriggerRegister = 0xB2;
  mSmiDataRegister    = 0xB3; 
//[-end-151124-IB10860187-modify]//

  ///
  /// Locate PI SMM CPU protocol
  ///
  Status = gSmst->SmmLocateProtocol (&gEfiSmmCpuProtocolGuid, NULL, (VOID **)&mSmmCpuProtocol);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Publish PI SMM SwDispatch2 Protocol
  ///
  ImageHandle = NULL;
  Status = gSmst->SmmInstallProtocolInterface (
                    &ImageHandle,
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &gSmmSwDispatch2
                    );
  ASSERT_EFI_ERROR (Status);
  return Status;
}

