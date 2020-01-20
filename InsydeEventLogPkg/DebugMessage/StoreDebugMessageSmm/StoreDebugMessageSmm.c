/** @file

  Store DEBUG() Message SMM implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/ 

#include <StoreDebugMessageSmm.h>
 
EFI_SMM_RSC_HANDLER_PROTOCOL          *mRscHandlerProtocol   = NULL;
BOOLEAN                               mDebugMessageEn = FALSE;
//[-start-180502-IB08400586-add]//
//[-start-180518-IB08400595-modify]//
DEBUG_MESSAGE_MEMORY_ADDR             *mH2OStoreDebugMsgMemAddressPtr = NULL;
//[-end-180518-IB08400595-modify]//
//[-end-180502-IB08400586-add]//

//[-start-180509-IB08400590-add]//
BOOLEAN                               mGlobalSerialMsgInitialized = FALSE;
BOOLEAN                               mShowProgressCodeSerialMsg = FALSE;
BOOLEAN                               mShowErrorCodeSerialMsg = FALSE;
BOOLEAN                               mShowDebugCodeSerialMsg = FALSE;
//[-end-180509-IB08400590-add]//

/**
  Extracts ASSERT() information from a status code structure.

  Converts the status code specified by CodeType, Value, and Data to the ASSERT()
  arguments specified by Filename, Description, and LineNumber.  If CodeType is
  an EFI_ERROR_CODE, and CodeType has a severity of EFI_ERROR_UNRECOVERED, and
  Value has an operation mask of EFI_SW_EC_ILLEGAL_SOFTWARE_STATE, extract
  Filename, Description, and LineNumber from the optional data area of the
  status code buffer specified by Data.  The optional data area of Data contains
  a Null-terminated ASCII string for the FileName, followed by a Null-terminated
  ASCII string for the Description, followed by a 32-bit LineNumber.  If the
  ASSERT() information could be extracted from Data, then return TRUE.
  Otherwise, FALSE is returned.

  If Data is NULL, then ASSERT().
  If Filename is NULL, then ASSERT().
  If Description is NULL, then ASSERT().
  If LineNumber is NULL, then ASSERT().

  @param  CodeType     The type of status code being converted.
  @param  Value        The status code value being converted.
  @param  Data         Pointer to status code data buffer.
  @param  Filename     Pointer to the source file name that generated the ASSERT().
  @param  Description  Pointer to the description of the ASSERT().
  @param  LineNumber   Pointer to source line number that generated the ASSERT().

  @retval  TRUE   The status code specified by CodeType, Value, and Data was
                  converted ASSERT() arguments specified by Filename, Description,
                  and LineNumber.
  @retval  FALSE  The status code specified by CodeType, Value, and Data could
                  not be converted to ASSERT() arguments.

**/
BOOLEAN
EFIAPI
ExtractAssertInfo (
  IN EFI_STATUS_CODE_TYPE        CodeType,
  IN EFI_STATUS_CODE_VALUE       Value,
  IN CONST EFI_STATUS_CODE_DATA  *Data,
  OUT CHAR8                      **Filename,
  OUT CHAR8                      **Description,
  OUT UINT32                     *LineNumber
  )
{
  EFI_DEBUG_ASSERT_DATA  *AssertData;

  if (((CodeType & EFI_STATUS_CODE_TYPE_MASK)      == EFI_ERROR_CODE) &&
      ((CodeType & EFI_STATUS_CODE_SEVERITY_MASK)  == EFI_ERROR_UNRECOVERED) &&
      ((Value    & EFI_STATUS_CODE_OPERATION_MASK) == EFI_SW_EC_ILLEGAL_SOFTWARE_STATE)) {
    AssertData   = (EFI_DEBUG_ASSERT_DATA *)(Data + 1);
    *Filename    = (CHAR8 *)(AssertData + 1);
    *Description = *Filename + AsciiStrLen (*Filename) + 1;
    *LineNumber  = AssertData->LineNumber;
    return TRUE;
  }
  return FALSE;
}

/**
  Extracts DEBUG() information from a status code structure.

  Converts the status code specified by Data to the DEBUG() arguments specified
  by ErrorLevel, Marker, and Format.  If type GUID in Data is
  EFI_STATUS_CODE_DATA_TYPE_DEBUG_GUID, then extract ErrorLevel, Marker, and
  Format from the optional data area of the status code buffer specified by Data.
  The optional data area of Data contains a 32-bit ErrorLevel followed by Marker
  which is 12 UINTN parameters, followed by a Null-terminated ASCII string for
  the Format.  If the DEBUG() information could be extracted from Data, then
  return TRUE.  Otherwise, FALSE is returned.

  If Data is NULL, then ASSERT().
  If ErrorLevel is NULL, then ASSERT().
  If Marker is NULL, then ASSERT().
  If Format is NULL, then ASSERT().

  @param  Data        Pointer to status code data buffer.
  @param  ErrorLevel  Pointer to error level mask for a debug message.
  @param  Marker      Pointer to the variable argument list associated with Format.
  @param  Format      Pointer to a Null-terminated ASCII format string of a
                      debug message.

  @retval  TRUE   The status code specified by Data was converted DEBUG() arguments
                  specified by ErrorLevel, Marker, and Format.
  @retval  FALSE  The status code specified by Data could not be converted to
                  DEBUG() arguments.

**/
BOOLEAN
EFIAPI
ExtractDebugInfo (
  IN CONST EFI_STATUS_CODE_DATA  *Data,
  OUT UINT32                     *ErrorLevel,
  OUT BASE_LIST                  *Marker,
  OUT CHAR8                      **Format
  )
{
  EFI_DEBUG_INFO  *DebugInfo;

  //
  // If the GUID type is not EFI_STATUS_CODE_DATA_TYPE_DEBUG_GUID then return FALSE
  //
  if (!CompareGuid (&Data->Type, &gEfiStatusCodeDataTypeDebugGuid)) {
    return FALSE;
  }

  //
  // Retrieve the debug information from the status code record
  //
  DebugInfo = (EFI_DEBUG_INFO *)(Data + 1);

  *ErrorLevel = DebugInfo->ErrorLevel;

  //
  // The first 12 * sizeof (UINT64) bytes following EFI_DEBUG_INFO are for variable arguments
  // of format in DEBUG string. Its address is returned in Marker and has to be 64-bit aligned.
  // It must be noticed that EFI_DEBUG_INFO follows EFI_STATUS_CODE_DATA, whose size is
  // 20 bytes. The size of EFI_DEBUG_INFO is 4 bytes, so we can ensure that Marker
  // returned is 64-bit aligned.
  // 64-bit aligned is a must, otherwise retrieving 64-bit parameter from BASE_LIST will
  // cause unalignment exception.
  //
  *Marker = (BASE_LIST) (DebugInfo + 1);
  *Format = (CHAR8 *)(((UINT64 *)*Marker) + 12);

  return TRUE;
}


/**
 Store DEBUG messages string to reserved memory.

 @param[in]  MessageString  - The String for the DEBUG messages.
 @param[in]  StringSize     - String size of the MessageString.         
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
StoreAsciiDebugMessage (
  IN CHAR8                                 *MessageString,
  IN UINTN                                 StringSize
  )  
{
  DEBUG_MESSAGE_HEAD_STRUCTURE            *DebugMessageHead = NULL;
  CHAR8                                   *StoreAddr;

//[-start-171212-IB08400542-modify]//
//[-start-180503-IB08400586-modify]//
  if (mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr == 0xFFFFFFFF) {
    return EFI_UNSUPPORTED;
  }
  DebugMessageHead = (DEBUG_MESSAGE_HEAD_STRUCTURE *)(UINTN)(mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr);

//[-start-180510-IB08400592-modify]//
  if ((DebugMessageHead->AdmgEndAddr + StringSize) > (DebugMessageHead->AdmgStartAddr + DebugMessageHead->AdmgSize)) {
    //
    // The total reserved memory space is not available for more DEBUG messages.
    //
    mDebugMessageEn = FALSE;
  }
//[-end-180510-IB08400592-modify]//
//[-end-180503-IB08400586-modify]//
//[-end-171212-IB08400542-modify]//
//[-start-180503-IB08400586-modify]//
  StoreAddr = (CHAR8 *)(UINTN)(DebugMessageHead->AdmgEndAddr + 1);
//[-end-180503-IB08400586-modify]//
  CopyMem (StoreAddr, MessageString, StringSize);
  DebugMessageHead->AdmgEndAddr += (UINT32)StringSize;

  return EFI_SUCCESS;
}

/**
 This function is a handler for OEM/ODM to create a specific function to handle DEBUG message.         
 
  @param[in] CodeType             Indicates the type of status code being reported.
  @param[in] CodeValue            Describes the current status of a hardware or software entity.
                                  This included information about the class and subclass that is
                                  used to classify the entity as well as an operation.
  @param[in] Instance             The enumeration of a hardware or software entity within
                                  the system. Valid instance numbers start with 1.
  @param[in] CallerId             This optional parameter may be used to identify the caller.
                                  This parameter allows the status code driver to apply different
                                  rules to different callers.
  @param[in] Data                 This optional parameter may be used to pass additional data.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
EFIAPI
DebugMessageStatusCode (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  ) 
{  
  EFI_STATUS      Status;
  CHAR8           *Filename;
  CHAR8           *Description;
  CHAR8           *Format;
  CHAR8           Buffer[MAX_DEBUG_MESSAGE_LENGTH];
  UINT32          ErrorLevel;
  UINT32          LineNumber;
  UINTN           CharCount;
  BASE_LIST       Marker;

  //
  // The service is not available, just return.
  //
  if (mDebugMessageEn ==  FALSE) {
    return EFI_UNSUPPORTED;
  }

  Buffer[0] = '\0';

  if (Data != NULL && ExtractAssertInfo (CodeType, CodeValue, Data, &Filename, &Description, &LineNumber)) {
    //
    // Print ASSERT() information into output buffer.
    //
//[-start-180509-IB08400590-add]//
    if (mShowErrorCodeSerialMsg == FALSE) {
      return EFI_SUCCESS;
    }
//[-end-180509-IB08400590-add]//
    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "\n\rDXE_ASSERT!: %a (%d): %a\n\r",
                  Filename,
                  LineNumber,
                  Description
                  );
  } else if (Data != NULL && ExtractDebugInfo (Data, &ErrorLevel, &Marker, &Format)) {
    //
    // Print DEBUG() information into output buffer.
    //
    CharCount = AsciiBSPrint (
                  Buffer, 
                  sizeof (Buffer), 
                  Format, 
                  Marker
                  );
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) {
    //
    // Print ERROR information into output buffer.
    //
//[-start-180509-IB08400590-add]//
    if (mShowErrorCodeSerialMsg == FALSE) {
      return EFI_SUCCESS;
    }
//[-end-180509-IB08400590-add]//
    CharCount = AsciiSPrint (
                  Buffer, 
                  sizeof (Buffer), 
                  "ERROR: C%x:V%x I%x", 
                  CodeType, 
                  CodeValue, 
                  Instance
                  );
   
    if (CallerId != NULL) {
      CharCount += AsciiSPrint (
                     &Buffer[CharCount],
                     (sizeof (Buffer) - (sizeof (Buffer[0]) * CharCount)),
                     " %g",
                     CallerId
                     );
    }

    if (Data != NULL) {
      CharCount += AsciiSPrint (
                     &Buffer[CharCount],
                     (sizeof (Buffer) - (sizeof (Buffer[0]) * CharCount)),
                     " %x",
                     Data
                     );
    }

    CharCount += AsciiSPrint (
                   &Buffer[CharCount],
                   (sizeof (Buffer) - (sizeof (Buffer[0]) * CharCount)),
                   "\n\r"
                   );
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_PROGRESS_CODE) {
    //
    // Print PROGRESS information into output buffer.
    //
//[-start-180509-IB08400590-add]//
    if (mShowProgressCodeSerialMsg == FALSE) {
      return EFI_SUCCESS;
    }
//[-end-180509-IB08400590-add]//
    CharCount = AsciiSPrint (
                  Buffer, 
                  sizeof (Buffer), 
                  "PROGRESS CODE: V%x I%x\n\r", 
                  CodeValue, 
                  Instance
                  );
  } else if (Data != NULL &&
             CompareGuid (&Data->Type, &gEfiStatusCodeDataTypeStringGuid) &&
             ((EFI_STATUS_CODE_STRING_DATA *) Data)->StringType == EfiStringAscii) {
    //
    // EFI_STATUS_CODE_STRING_DATA
    //
//[-start-180509-IB08400590-add]//
    if (mShowDebugCodeSerialMsg == FALSE) {
      return EFI_SUCCESS;
    }
//[-end-180509-IB08400590-add]//
    CharCount = AsciiSPrint (
                  Buffer,
                  sizeof (Buffer),
                  "%a\n\r",
                  ((EFI_STATUS_CODE_STRING_DATA *) Data)->String.Ascii
                  );
  } else {
    //
    // Code type is not defined.
    //
//[-start-180509-IB08400590-add]//
    if (mShowDebugCodeSerialMsg == FALSE) {
      return EFI_SUCCESS;
    }
//[-end-180509-IB08400590-add]//
    CharCount = AsciiSPrint (
                  Buffer, 
                  sizeof (Buffer), 
                  "Undefined: C%x:V%x I%x\n\r", 
                  CodeType, 
                  CodeValue, 
                  Instance
                  );
  }

  if (CharCount == 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Store the DEBUG message to reserved memory space with ASCII code.
  //
  Status = StoreAsciiDebugMessage (Buffer, CharCount);

  return Status;
}

/**
 
 Allocate a memory buffer for store DEBUG messages.
 And inital DEBUG Message Head information to this memory.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
DebugMessageMemBufferInit (
  VOID
  )    
{    
//[-start-180503-IB08400586-add]//
  UINT64                         DebugSignature = DUMP_DEBUG_MESSAGE_INSTANCE_SIGNATURE;
//[-end-180503-IB08400586-add]//
  
//[-start-171212-IB08400542-modify]//
//[-start-180503-IB08400586-modify]//
  if ((PcdGet32(PcdH2OStoreDebugMsgMemAddressPtr) == 0xFFFFFFFF) || (PcdGet32(PcdH2OStoreDebugMsgMemAddressPtr) == 0)) {
    return EFI_UNSUPPORTED;
  }
//[-start-180518-IB08400595-modify]//
  mH2OStoreDebugMsgMemAddressPtr = (DEBUG_MESSAGE_MEMORY_ADDR *) (UINTN)PcdGet32(PcdH2OStoreDebugMsgMemAddressPtr);
//[-end-180518-IB08400595-modify]//

  if ((mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr == 0xFFFFFFFF)) {
    return EFI_UNSUPPORTED;
  } else if (CompareMem (&mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageSignature, &DebugSignature, sizeof(UINT64)) != 0) {
    return EFI_UNSUPPORTED;
  }
//[-end-180503-IB08400586-modify]//
  
//[-end-171212-IB08400542-modify]//
  mDebugMessageEn = TRUE;

//[-start-180509-IB08400590-add]//
  if (!mGlobalSerialMsgInitialized) {
    mShowProgressCodeSerialMsg = PcdGetBool (PcdH2OShowProgressCodeSerialMsgEnable);
    mShowErrorCodeSerialMsg = PcdGetBool (PcdH2OShowErrorCodeSerialMsgEnable);
    mShowDebugCodeSerialMsg = PcdGetBool (PcdH2OShowDebugCodeSerialMsgEnable);
    mGlobalSerialMsgInitialized = TRUE;
  }
//[-end-180509-IB08400590-add]//
  
  return EFI_SUCCESS;
}

/**
  Notification function for ReportStatusCode Handler Protocol

  This routine is the notification function for EFI_RSC_HANDLER_PROTOCOL

  @param[in]         Event                
  @param[in]         Context                           
  
  @retval VOID                

**/
EFI_STATUS
EFIAPI
RscHandlerProtocolCallback (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  EFI_STATUS                        Status;

  Status = gSmst->SmmLocateProtocol (
                                 &gEfiSmmRscHandlerProtocolGuid,
                                 NULL,
                                 (VOID **) &mRscHandlerProtocol
                                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Register the worker function to ReportStatusCodeRouter
  //
  Status = mRscHandlerProtocol->Register (DebugMessageStatusCode);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  return Status;
}

//[-start-170413-IB08400444-add]//
/**
  This function is to unregister the DebugMessage Status Code callback function in SMM.

  @param[in]     DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     Context         Points to an optional handler context which was specified when the
                                 handler was registered.
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS            The interrupt was handled and quiesced. No other handlers 
                                 should still be called.
**/
EFI_STATUS
EFIAPI
UnregisterBootTimeHandlers (
  IN     EFI_HANDLE                     DispatchHandle,
  IN     CONST VOID                     *RegisterContext,
  IN OUT VOID                           *CommBuffer,
  IN OUT UINTN                          *CommBufferSize
  )
{  
  SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE       *SmmBscCommunicateData;  
  EFI_STATUS                             Status = EFI_UNSUPPORTED;

  SmmBscCommunicateData = (SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE *)CommBuffer;
  switch (SmmBscCommunicateData->Function) {
    case SMM_COMM_UNREGISTER_HANDLER:
      if (SmmBscCommunicateData->UnregisterService == FALSE) {
        SmmBscCommunicateData->ReturnStatus = EFI_UNSUPPORTED;
        break;
      }

      //
      // Unregister Status Code callback function.
      //
      if (mRscHandlerProtocol != NULL) {
        Status = mRscHandlerProtocol->Unregister (DebugMessageStatusCode);
        if (EFI_ERROR (Status)) {
          mDebugMessageEn = FALSE;
          DEBUG((EFI_D_ERROR, "Unregister DebugMessageStatusCode() in SMM: %r\n", Status));
        }
      } 
      
      SmmBscCommunicateData->ReturnStatus = Status;
      break;
      
    default :
      SmmBscCommunicateData->ReturnStatus = EFI_UNSUPPORTED;
  }
  
  return EFI_SUCCESS;
}
//[-end-170413-IB08400444-add]//


/**
 The DEBUG Message driver will handle all events during DXE, SMM and BDS phase.
 The DEBUG messages came from Status Code reported.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
StoreDebugMessageSmmEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  ) 
{
  EFI_STATUS                                Status;
  EFI_EVENT                                 RscHandlerProtocolEvent;
//[-start-170413-IB08400444-add]//  
  EFI_HANDLE                                DispatchHandle;
//[-end-170413-IB08400444-add]//
  
  //
  // Initialize the memory space for DEBUG messages.
  //
  Status = DebugMessageMemBufferInit ();
  if (EFI_ERROR (Status)) {
    return Status;
  }    

  Status = gSmst->SmmLocateProtocol (
                                 &gEfiSmmRscHandlerProtocolGuid,
                                 NULL,
                                 (VOID **) &mRscHandlerProtocol
                                 );
  if (EFI_ERROR (Status)) {
    //
    // Register callback for loading Event storage driver.
    //
    Status = gSmst->SmmRegisterProtocolNotify (
                                         &gEfiSmmRscHandlerProtocolGuid,
                                         RscHandlerProtocolCallback,
                                         &RscHandlerProtocolEvent
                                         );
  } else {
    //
    // Register the worker function to ReportStatusCodeRouter
    //
    Status = mRscHandlerProtocol->Register (DebugMessageStatusCode);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
//[-start-170413-IB08400444-add]//
  //
  // To unregister the DebugMessage Status Code callback function in SMM after exiting Boot Service.
  //
  Status = gSmst->SmiHandlerRegister (
                    UnregisterBootTimeHandlers,
                    &gH2ODebugMessageStatusCodeCommunicationGuid,
                    &DispatchHandle
                    );
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "Register UnregisterBootTimeHandlers(): %r\n", Status));
  }  
//[-end-170413-IB08400444-add]//
  return Status;
}
