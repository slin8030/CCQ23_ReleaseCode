/** @file
  This driver is responsible for the registration of child drivers
  and the abstraction of the SC SMI sources.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2017 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#include "ScSmmHelpers.h"

#include <Protocol/SmmBase2.h>
#include <Protocol/SmmControl2.h>
//
// MODULE / GLOBAL DATA
//
// Module variables used by the both the main dispatcher and the source dispatchers
// Declared in SchSmmSources.h
//

UINT16                AcpiBaseAddr;
PRIVATE_DATA          mPrivateData = {  // for the structure
  {
    NULL
  },                                    // CallbackDataBase linked list head
  NULL,                                 // Handler returned whan calling SmiHandlerRegister
  NULL,                                 // EFI handle returned when calling InstallMultipleProtocolInterfaces
  {                                     // protocol arrays
    //
    // elements within the array
    //
    {
      PROTOCOL_SIGNATURE,
      UsbType,
      &gEfiSmmUsbDispatch2ProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister
      }
    },
    {
      PROTOCOL_SIGNATURE,
      SxType,
      &gEfiSmmSxDispatch2ProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister
      }
    },
    {
      PROTOCOL_SIGNATURE,
      SwType,
      &gEfiSmmSwDispatch2ProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister,
        (UINTN) MAXIMUM_SWI_VALUE
      }
    },
    {
      PROTOCOL_SIGNATURE,
      GpiType,
      &gEfiSmmGpiDispatch2ProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister,
        (UINTN) V_GPIO_NUM_SUPPORTED_GPIS
      }
    },
    {
      PROTOCOL_SIGNATURE,
      IchnType,
      &gEfiSmmIchnDispatchProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister
      }
    },
    {
      PROTOCOL_SIGNATURE,
      IchnExType,
      &gEfiSmmIchnDispatchExProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister
      }
    },
    {
      PROTOCOL_SIGNATURE,
      PowerButtonType,
      &gEfiSmmPowerButtonDispatch2ProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister
      }
    },
    {
      PROTOCOL_SIGNATURE,
      PeriodicTimerType,
      &gEfiSmmPeriodicTimerDispatch2ProtocolGuid,
      {
        (SC_SMM_GENERIC_REGISTER) ScSmmCoreRegister,
        (SC_SMM_GENERIC_UNREGISTER) ScSmmCoreUnRegister,
        (UINTN) ScSmmPeriodicTimerDispatchGetNextShorterInterval
      }
    },
  }
};

CONTEXT_FUNCTIONS     mContextFunctions[ScSmmProtocolTypeMax] = {
  {
    NULL,
    NULL,
    NULL
  },
  {
    SxGetContext,
    SxCmpContext,
    NULL
  },
  {
    SwGetContext,
    SwCmpContext,
    SwGetCommBuffer
  },
  {
    NULL,
    NULL,
    NULL
  },
  {
    NULL,
    NULL,
    NULL
  },
  {
    NULL,
    NULL,
    NULL
  },
  {
    PowerButtonGetContext,
    PowerButtonCmpContext,
    NULL
  },
  {
    PeriodicTimerGetContext,
    PeriodicTimerCmpContext,
    PeriodicTimerGetCommBuffer
  },
};

///
/// PROTOTYPES
///
/// Functions use only in this file
///
EFI_STATUS
EFIAPI
ScSmmCoreDispatcher (
  IN EFI_HANDLE             SmmImageHandle,
  IN     CONST VOID        *ContextData,        OPTIONAL
  IN OUT VOID               *CommunicationBuffer,    OPTIONAL
  IN OUT UINTN              *SourceSize    OPTIONAL
  );

///
/// FUNCTIONS
///
/// Driver entry point
///
/**

  Initializes the SC SMM Dispatcher

  @param[in] ImageHandle          Pointer to the loaded image protocol for this driver
  @param[in] SystemTable          Pointer to the EFI System Table

  @retval EFI_SUCCESS             PchSmmDispatcher Initialization completed.

**/
EFI_STATUS
EFIAPI
InitializeScSmmDispatcher (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  //
  // Read ACPI Base Address
  //
  AcpiBaseAddr = (UINT16) PcdGet16 (PcdScAcpiIoPortBaseAddress);
  ASSERT (AcpiBaseAddr != 0);

  ///
  /// Init required protocol for SC Sw Dispatch protocol.
  ///
  ScSwDispatchInit ();

  ///
  /// Register a callback function to handle subsequent SMIs.  This callback
  /// will be called by SmmCoreDispatcher.
  ///
  Status = gSmst->SmiHandlerRegister (ScSmmCoreDispatcher, NULL, &mPrivateData.SmiHandle);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Initialize Callback DataBase
  ///
  InitializeListHead (&mPrivateData.CallbackDataBase);
  ScSmmPublishDispatchProtocols ();

  ///
  /// Enable SMIs on the SC now that we have a callback
  ///
  ScSmmInitHardware ();

  ///
  /// IoTrapControl Protocol
  ///
  InstallIoTrapControl ();

  return EFI_SUCCESS;
}
/**

  Check the Fed SwSmiInputValue to see if there is a duplicated one in the database

  @param[in] FedSwSmiInputValue   Fed SwSmiInputValue

  @retval EFI_SUCCESS             There is no duplicated SwSmiInputValue
  @retval EFI_INVALID_PARAMETER   There is a duplicated SwSmiInputValue

**/
EFI_STATUS
SmiInputValueDuplicateCheck (
  IN UINTN           FedSwSmiInputValue
  )
{

  DATABASE_RECORD *RecordInDb;
  LIST_ENTRY      *LinkInDb;

  LinkInDb = GetFirstNode (&mPrivateData.CallbackDataBase);
  while (!IsNull (&mPrivateData.CallbackDataBase, LinkInDb)) {
    RecordInDb = DATABASE_RECORD_FROM_LINK (LinkInDb);

    if (RecordInDb->ProtocolType == SwType) {
      if (RecordInDb->ChildContext.Sw.SwSmiInputValue == FedSwSmiInputValue) {
        return EFI_INVALID_PARAMETER;
      }
    }

    LinkInDb = GetNextNode (&mPrivateData.CallbackDataBase, &RecordInDb->Link);
  }

  return EFI_SUCCESS;
}

/**

  Register a child SMI dispatch function with a parent SMM driver.

  @param[in]  This                Pointer to the SC_SMM_GENERIC_PROTOCOL instance.
  @param[in]  DispatchFunction    Pointer to dispatch function to be invoked for this SMI source.
  @param[in]  DispatchContext     Pointer to the dispatch function's context.
  @param[out] DispatchHandle      Handle of dispatch function, for when interfacing
                                  with the parent SMM driver, will be the address of linked
                                  list link in the call back record.

  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create database record
  @retval EFI_INVALID_PARAMETER   The input parameter is invalid
  @retval EFI_SUCCESS             The dispatch function has been successfully
                                  registered and the SMI source has been enabled.

**/
EFI_STATUS
ScSmmCoreRegister (
  IN  SC_SMM_GENERIC_PROTOCOL                          *This,
  IN  EFI_SMM_HANDLER_ENTRY_POINT2                      DispatchFunction,
  IN  SC_SMM_CONTEXT                                   *DispatchContext,
  OUT EFI_HANDLE                                       *DispatchHandle
  )
{
  EFI_STATUS                  Status;

  DATABASE_RECORD             *Record;
  SC_SMM_QUALIFIED_PROTOCOL   *Qualified;
  SC_SMM_SOURCE_DESC          NullSourceDesc = NULL_SOURCE_DESC_INITIALIZER;
  UINTN                       Index;
  UINT8                       GpiSmiBitOffset;
  UINT32                      GpiSmiEnRegAddress;
  UINT32                      GpiSmiStsRegAddress;

  Index     = 0;
  //
  // Create database record and add to database
  //
  if (gSmst == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gSmst->SmmAllocatePool (EfiRuntimeServicesData, sizeof (DATABASE_RECORD), (VOID**) &Record);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Gather information about the registration request
  //
  Record->Callback          = DispatchFunction;

  Qualified                 = QUALIFIED_PROTOCOL_FROM_GENERIC (This);

  Record->ProtocolType      = Qualified->Type;

  Record->ContextFunctions  = mContextFunctions[Qualified->Type];
  //
  // Perform linked list housekeeping
  //
  Record->Signature = DATABASE_RECORD_SIGNATURE;

  switch (Qualified->Type) {
      //
      // By the end of this switch statement, we'll know the
      // source description the child is registering for
      //
    case UsbType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_USB_REGISTER_CONTEXT));
      //
      // Check the validity of Context Type
      //
      if ((Record->ChildContext.Usb.Type < UsbLegacy) || (Record->ChildContext.Usb.Type > UsbWake)) {
        goto Error;
      }

//[-start-160924-IB07400789-add]//
      if (!IsSupportedUsbController(DispatchContext)) {
        goto Error;
      }
//[-end-160924-IB07400789-add]//

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      MapUsbToSrcDesc (DispatchContext, &(Record->SrcDesc));
//[-start-170627-IB07400880-modify]//
//      Record->ClearSource = NULL;
      Record->ClearSource = ScSmmUsbClearSource;
//[-end-170627-IB07400880-modify]//
      //
      // use default clear source function
      //
      break;

    case SxType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_SX_REGISTER_CONTEXT));
      //
      // Check the validity of Context Type and Phase
      //
      if ((Record->ChildContext.Sx.Type < SxS0) ||
          (Record->ChildContext.Sx.Type >= EfiMaximumSleepType) ||
          (Record->ChildContext.Sx.Phase < SxEntry) ||
          (Record->ChildContext.Sx.Phase >= EfiMaximumPhase)
          ) {
        goto Error;
      }

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      CopyMem ((VOID *) &(Record->SrcDesc), (VOID *) (&SX_SOURCE_DESC), sizeof (SC_SMM_SOURCE_DESC));
      Record->ClearSource = NULL;
      //
      // use default clear source function
      //
      break;

    case SwType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_SW_REGISTER_CONTEXT));
      //
      // Check the validity of Context Value
      //
      if (Record->ChildContext.Sw.SwSmiInputValue  == (UINTN)-1) {
        for (Index = 1; Index < MAXIMUM_SWI_VALUE; Index++) {
          if (!EFI_ERROR (SmiInputValueDuplicateCheck (Index))) {
            Record->ChildContext.Sw.SwSmiInputValue = Index;
            break;
          }
        }
        if (Record->ChildContext.Sw.SwSmiInputValue == (UINTN)-1) {
          goto Error;
        }
      }
      if (Record->ChildContext.Sw.SwSmiInputValue > MAXIMUM_SWI_VALUE) {
        goto Error;
      }

      if (EFI_ERROR (SmiInputValueDuplicateCheck (Record->ChildContext.Sw.SwSmiInputValue))) {
        goto Error;
      }
      CopyMem (DispatchContext, &Record->ChildContext, sizeof (EFI_SMM_SW_REGISTER_CONTEXT));
      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      CopyMem ((VOID *) &(Record->SrcDesc), (VOID *) (&SW_SOURCE_DESC), sizeof (SC_SMM_SOURCE_DESC));
      Record->ClearSource = NULL;
      //
      // use default clear source function
      //
      break;

    case GpiType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_GPI_REGISTER_CONTEXT));
      Index = (UINTN) Record->ChildContext.Gpi.GpiNum;

      Status = GpioGetPadAndSmiRegs (
        (UINT32)Index,
        &GpiSmiBitOffset,
        &GpiSmiEnRegAddress,
        &GpiSmiStsRegAddress
        );

      if (EFI_ERROR (Status)) {
        goto Error;
      }

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);

      CopyMem (
        (VOID *) &(Record->SrcDesc),
        (VOID *) &(SC_GPI_SOURCE_DESC_TEMPLATE),
        sizeof (SC_SMM_SOURCE_DESC)
        );

      Record->SrcDesc.En[0].Reg.Data.raw = GpiSmiEnRegAddress;    // GPI SMI Enable register
      Record->SrcDesc.En[0].Bit = GpiSmiBitOffset;                // Bit position for selected pad
      Record->SrcDesc.En[0].SizeInBytes = 4;
      Record->SrcDesc.Sts[0].Reg.Data.raw = GpiSmiStsRegAddress;  // GPI SMI Status register
      Record->SrcDesc.Sts[0].Bit = GpiSmiBitOffset;               // Bit position for selected pad
      Record->SrcDesc.Sts[0].SizeInBytes = 4;
      Record->ClearSource = NULL;
      //
      // use default clear source function
      //
      break;

    case IchnType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_ICHN_DISPATCH_CONTEXT));
      //
      // Check the validity of Context Type
      //
      if (Record->ChildContext.Ichn.Type >= NUM_ICHN_TYPES) {
        goto Error;
      }

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      CopyMem (
        (VOID *) &(Record->SrcDesc),
        (VOID *) &(ICHN_SOURCE_DESCS[Record->ChildContext.Ichn.Type]),
        sizeof (SC_SMM_SOURCE_DESC)
        );
      Record->ClearSource = ScSmmIchnClearSource;
      break;

    case IchnExType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_ICHN_DISPATCH_EX_CONTEXT));
      //
      // Check the validity of Context Type
      //
      if ((Record->ChildContext.IchnEx.Type < IchnExPciExpress) || (Record->ChildContext.IchnEx.Type >= IchnExTypeMAX)) {
        goto Error;
      }

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      CopyMem (
        (VOID *) &(Record->SrcDesc),
        (VOID *) &(ICHN_EX_SOURCE_DESCS[Record->ChildContext.IchnEx.Type - IchnExPciExpress]),
        sizeof (SC_SMM_SOURCE_DESC)
        );
      Record->ClearSource = NULL;
      break;

    case PowerButtonType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT));
      //
      // Check the validity of Context Phase
      //
      if ((Record->ChildContext.PowerButton.Phase < EfiPowerButtonEntry) ||
          (Record->ChildContext.PowerButton.Phase > EfiPowerButtonExit)
         )
      {
        goto Error;
      }

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      CopyMem ((VOID *) &(Record->SrcDesc), (VOID *) &POWER_BUTTON_SOURCE_DESC, sizeof (SC_SMM_SOURCE_DESC));
      Record->ClearSource = NULL;
      //
      // use default clear source function
      //
      break;

    case PeriodicTimerType:
	  CopyMem (&Record->ChildContext, DispatchContext, sizeof (EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT));
      //
      // Check the validity of timer value
      //
      if (DispatchContext->PeriodicTimer.SmiTickInterval <= 0) {
        goto Error;
      }

      InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
      MapPeriodicTimerToSrcDesc (DispatchContext, &(Record->SrcDesc));
      Record->ClearSource = ScSmmPeriodicTimerClearSource;
      break;

    default:
      goto Error;
      break;
  }

  if (CompareSources (&Record->SrcDesc, &NullSourceDesc)) {
    goto Error;
  }

  if (Record->ClearSource == NULL) {
    //
    // Clear the SMI associated w/ the source using the default function
    //
    ScSmmClearSource (&Record->SrcDesc);
  } else {
    //
    // This source requires special handling to clear
    //
    Record->ClearSource (&Record->SrcDesc);
  }

  ScSmmEnableSource (&Record->SrcDesc);

  //
  // Child's handle will be the address linked list link in the record
  //
  *DispatchHandle = (EFI_HANDLE) (&Record->Link);


  return EFI_SUCCESS;

Error:
  Status = gSmst->SmmFreePool (Record);
  //
  // DEBUG((EFI_D_ERROR,"Free pool status %d\n", Status ));
  //
  return EFI_INVALID_PARAMETER;
}

/**

  Unregister a child SMI source dispatch function with a parent SMM driver.

  @param[in] This                 Pointer to the  EFI_SMM_IO_TRAP_DISPATCH_PROTOCOL instance.
  @param[in] DispatchHandle       Handle of dispatch function to deregister.

  @retval EFI_SUCCESS             The dispatch function has been successfully
                                  unregistered and the SMI source has been disabled
                                  if there are no other registered child dispatch
                                  functions for this SMI source.
  @retval EFI_INVALID_PARAMETER   Handle is invalid.
  @retval EFI_SUCCESS             The function has been successfully unregistered child SMI source.

**/
EFI_STATUS
ScSmmCoreUnRegister (
  IN SC_SMM_GENERIC_PROTOCOL                            *This,
  IN EFI_HANDLE                                         *DispatchHandle
  )
{
  ///
  /// BOOLEAN SafeToDisable;
  ///
  DATABASE_RECORD *RecordToDelete;

  ///
  /// DATABASE_RECORD *RecordInDb;
  /// EFI_LIST_NODE   *LinkInDb;
  ///
  if (DispatchHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  RecordToDelete = DATABASE_RECORD_FROM_LINK (DispatchHandle);

  ///
  /// Take the entry out of the linked list
  ///
  if (RecordToDelete->Link.ForwardLink == (LIST_ENTRY *) EFI_BAD_POINTER) {
    return EFI_INVALID_PARAMETER;
  }

  RemoveEntryList (&RecordToDelete->Link);

  return EFI_SUCCESS;
}

/**

  The callback function to handle subsequent SMIs.  This callback will be called by SmmCoreDispatcher.

  @param[in]     SmmImageHandle       SMM image handle
  @param[in]     ContextData          Not used
  @param[in,out] CommunicationBuffer  Not used
  @param[in,out] SourceSize           Not used

  @retval EFI_SUCCESS             Function successfully completed

**/
EFI_STATUS
EFIAPI
ScSmmCoreDispatcher (
  IN EFI_HANDLE             SmmImageHandle,
  IN     CONST VOID         *ContextData,        OPTIONAL
  IN OUT VOID               *CommunicationBuffer,    OPTIONAL
  IN OUT UINTN              *SourceSize    OPTIONAL
  )
{
  //
  // Used to prevent infinite loops
  //
  UINTN               EscapeCount;

  BOOLEAN             ContextsMatch;
  BOOLEAN             EosSet;
  BOOLEAN             SxChildWasDispatched;

  DATABASE_RECORD     *RecordInDb;
  LIST_ENTRY          *LinkInDb;
  DATABASE_RECORD     *RecordToExhaust;
  LIST_ENTRY          *LinkToExhaust;

  SC_SMM_CONTEXT     Context;
  VOID                *CommBuffer;
  UINTN               CommBufferSize;

  EFI_STATUS          Status;

  SC_SMM_SOURCE_DESC ActiveSource = NULL_SOURCE_DESC_INITIALIZER;

  EscapeCount           = 100;
  ContextsMatch         = FALSE;
  EosSet                = FALSE;
  SxChildWasDispatched  = FALSE;
  Status                = EFI_SUCCESS;

  if (!IsListEmpty (&mPrivateData.CallbackDataBase)) {
    //
    // We have children registered w/ us -- continue
    //
    while ((!EosSet) && (EscapeCount > 0)) {
      EscapeCount--;

      LinkInDb = GetFirstNode (&mPrivateData.CallbackDataBase);

      while (!IsNull (&mPrivateData.CallbackDataBase, LinkInDb)) {
        RecordInDb = DATABASE_RECORD_FROM_LINK (LinkInDb);

        //
        // look for the first active source
        //
        if (!SourceIsActive (&RecordInDb->SrcDesc)) {

          //
          // Didn't find the source yet, keep looking
          //
          LinkInDb = GetNextNode (&mPrivateData.CallbackDataBase, &RecordInDb->Link);

          //
          // if it's the last one, try to clear EOS
          //
          if (IsNull (&mPrivateData.CallbackDataBase, LinkInDb)) {
            EosSet = ScSmmSetAndCheckEos ();
          }
        } else {
          //
          // We found a source. If this is a sleep type, we have to go to
          // appropriate sleep state anyway.No matter there is sleep child or not
          //
          if (RecordInDb->ProtocolType == SxType) {
            SxChildWasDispatched = TRUE;
          }
          //
          // "cache" the source description and don't query I/O anymore
          //
          CopyMem ((VOID *) &ActiveSource, (VOID *) &(RecordInDb->SrcDesc), sizeof (SC_SMM_SOURCE_DESC));
          LinkToExhaust = LinkInDb;

          //
          // exhaust the rest of the queue looking for the same source
          //
          while (!IsNull (&mPrivateData.CallbackDataBase, LinkToExhaust)) {
            RecordToExhaust = DATABASE_RECORD_FROM_LINK (LinkToExhaust);
            //
            // RecordToExhaust->Link might be removed (unregistered) by Callback function, and then the
            // system will hang in ASSERT() while calling GetNextNode().
            // To prevent the issue, we need to get next record in DB here (before Callback function).
            //
            LinkToExhaust = GetNextNode (&mPrivateData.CallbackDataBase, &RecordToExhaust->Link);

            if (CompareSources (&RecordToExhaust->SrcDesc, &ActiveSource)) {
              //
              // These source descriptions are equal, so this callback should be
              // dispatched.
              //
              if (RecordToExhaust->ContextFunctions.GetContext != NULL) {
                //
                // This child requires that we get a calling context from
                // hardware and compare that context to the one supplied
                // by the child.
                //
                ASSERT (RecordToExhaust->ContextFunctions.CmpContext != NULL);

                //
                // Make sure contexts match before dispatching event to child
                //
                RecordToExhaust->ContextFunctions.GetContext (RecordToExhaust, &Context);
                ContextsMatch = RecordToExhaust->ContextFunctions.CmpContext (&Context, &RecordToExhaust->ChildContext);

              } else {
                //
                // This child doesn't require any more calling context beyond what
                // it supplied in registration.  Simply pass back what it gave us.
                //
                ASSERT (RecordToExhaust->Callback != NULL);
                Context       = RecordToExhaust->ChildContext;
                ContextsMatch = TRUE;
              }

              if (ContextsMatch) {
                if (RecordToExhaust->Callback != NULL) {
                  if (RecordToExhaust->ContextFunctions.GetCommBuffer != NULL) {
                    ///
                    /// This callback function needs CommBuffer and CommBufferSize.
                    /// Get those from child and then pass to callback function.
                    ///
                    RecordToExhaust->ContextFunctions.GetCommBuffer (RecordToExhaust, &CommBuffer, &CommBufferSize);
                  } else {
                    ///
                    /// Child doesn't support the CommBuffer and CommBufferSize.
                    /// Just pass NULL value to callback function.
                    ///
                    CommBuffer     = NULL;
                    CommBufferSize = 0;
                  }
                  PERF_START_EX (NULL, "SmmFunction", NULL, AsmReadTsc(), RecordToExhaust->ProtocolType);
                  RecordToExhaust->Callback ((EFI_HANDLE) & RecordToExhaust->Link, &Context, CommBuffer, &CommBufferSize);
                  PERF_END_EX (NULL, "SmmFunction", NULL, AsmReadTsc(), RecordToExhaust->ProtocolType);
                  if (RecordToExhaust->ProtocolType == SxType) {
                    SxChildWasDispatched = TRUE;
                  }
                } else {
                  ASSERT (FALSE);
                }
              }
            }
          }

          if (RecordInDb->ClearSource == NULL) {
            //
            // Clear the SMI associated w/ the source using the default function
            //
            ScSmmClearSource (&ActiveSource);
          } else {
            //
            // This source requires special handling to clear
            //
            RecordInDb->ClearSource (&ActiveSource);
          }
          //
          // Also, try to clear EOS
          //
          EosSet = ScSmmSetAndCheckEos ();
          //
          // Queue is empty, reset the search
          //
          break;
        }
      }
    }
  }
  ScBeforeExitSmi ();
  //
  // If you arrive here, there are two possible reasons:
  // (1) you've got problems with clearing the SMI status bits in the
  // ACPI table.  If you don't properly clear the SMI bits, then you won't be able to set the
  // EOS bit.  If this happens too many times, the loop exits.
  // (2) there was a SMM communicate for callback messages that was received prior
  // to this driver.
  // If there is an asynchronous SMI that occurs while processing the Callback, let
  // all of the drivers (including this one) have an opportunity to scan for the SMI
  // and handle it.
  // If not, we don't want to exit and have the foreground app. clear EOS without letting
  // these other sources get serviced.
  //
  // This assert is not valid with CSM legacy solution because it generates software SMI
  // to test for legacy USB support presence.
  // This may not be illegal, so we cannot assert at this time.
  //
  //  ASSERT (EscapeCount > 0);
  //
  if (SxChildWasDispatched) {
    //
    // A child of the SmmSxDispatch protocol was dispatched during this call;
    // put the system to sleep.
    //
    ScSmmSxGoToSleep ();
  }

  return Status;

}
