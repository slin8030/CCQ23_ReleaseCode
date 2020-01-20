/** @file

  Store DEBUG() Message Dxe implementation.

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

#include <StoreDebugMessageDxe.h>
EFI_RSC_HANDLER_PROTOCOL              *mRscHandlerProtocol   = NULL;
BOOLEAN                               mDebugMessageEn = FALSE;
//[-start-170413-IB08400444-add]//
EFI_EVENT                             mExitBootServicesEvent = NULL;
//[-end-170413-IB08400444-add]//
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

//[-start-180503-IB08400586-add]//
/**
 Create variable "DebugMsg" to record the memory address of DEBUG messages.

 @param[in]  VOID     
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
SaveDebugMessageVariable (
  VOID
  )
{
  EFI_STATUS                         Status;
//[-start-180518-IB08400595-modify]//
  DEBUG_MESSAGE_MEMORY_ADDR          DebugMessageStructure;
//[-end-180518-IB08400595-modify]//
  
//[-start-180518-IB08400595-modify]//
  CopyMem (&DebugMessageStructure, mH2OStoreDebugMsgMemAddressPtr, sizeof (DEBUG_MESSAGE_MEMORY_ADDR));
  Status = CommonSetVariable (
             L"DebugMsg",
             &gH2ODebugMessageVariableGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
             sizeof (DEBUG_MESSAGE_MEMORY_ADDR),
             &DebugMessageStructure
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-180518-IB08400595-modify]//

  return EFI_SUCCESS;
}
//[-end-180503-IB08400586-add]//

/**
 Notification function when Variable Architecture Protocol Installed.
 Create variable "DebugMsg" to record the memory address of DEBUG messages.

 @param[in]  Event   - The Event that is being processed
 @param[in]  Context - Event Context         
 
 @retval EFI Status                  
*/
VOID
EFIAPI
InstalledVarNotificationFunction (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  )
{  
  EFI_STATUS                         Status;

//[-start-180503-IB08400586-modify]//
  Status = SaveDebugMessageVariable();
  if (EFI_ERROR(Status)) {
    return;
  }
//[-end-180503-IB08400586-modify]//

  gBS->CloseEvent (Event);
  return;
}

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
//[-start-180502-IB08400586-modify]//
  if (mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr == 0xFFFFFFFF) {
    return EFI_UNSUPPORTED;
  }
  DebugMessageHead = (DEBUG_MESSAGE_HEAD_STRUCTURE *)(UINTN)(mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr);

//[-start-180510-IB08400592-modify]//
  if ((DebugMessageHead->AdmgEndAddr + StringSize) > 
      (DebugMessageHead->AdmgStartAddr + (DebugMessageHead->TotalSize - sizeof (DEBUG_MESSAGE_HEAD_STRUCTURE)))) {
    //
    // The total reserved memory space is not available for more DEBUG messages.
    //
    mDebugMessageEn = FALSE;
    return EFI_UNSUPPORTED;
  }
//[-end-180510-IB08400592-modify]//
//[-end-171212-IB08400542-modify]//
  StoreAddr = (CHAR8 *)(UINTN)(DebugMessageHead->AdmgEndAddr + 1);
  CopyMem (StoreAddr, MessageString, StringSize);
  DebugMessageHead->AdmgEndAddr += (UINT32)StringSize;
//[-end-180502-IB08400586-modify]//

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

//[-start-180509-IB08400590-add]//
  if (!mGlobalSerialMsgInitialized) {
    mShowProgressCodeSerialMsg = PcdGetBool (PcdH2OShowProgressCodeSerialMsgEnable);
    mShowErrorCodeSerialMsg = PcdGetBool (PcdH2OShowErrorCodeSerialMsgEnable);
    mShowDebugCodeSerialMsg = PcdGetBool (PcdH2OShowDebugCodeSerialMsgEnable);
    mGlobalSerialMsgInitialized = TRUE;
  }  
//[-end-180509-IB08400590-add]//

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
  EFI_STATUS                                Status;
  DEBUG_MESSAGE_HEAD_STRUCTURE              *DebugMessageHead=NULL;
  UINT32                                    TableSize;    
  PEI_DEBUG_MESSAGE_HEAD_STRUCTURE          *PeiDebugMessageHead = NULL;
  UINT32                                    PeiDebugMessageSize;
  EFI_PHYSICAL_ADDRESS                      DebugMessageBuffer; 

  //
  // Allocate reserved memory for Storing DEBUG message.
  //
  DebugMessageBuffer = 0xFFFFFFFF;
//[-start-171212-IB08400542-modify]//
//[-start-180510-IB08400592-modify]//
  TableSize = PcdGet32(PcdH2OMaxStoreDebugMsgSize);
//[-end-180510-IB08400592-modify]//
//[-end-171212-IB08400542-modify]//

  //
  // Use EfiReservedMemoryType memory section to store messages.
  //
  Status = gBS->AllocatePages (
                           AllocateMaxAddress,
                           EfiReservedMemoryType,
                           EFI_SIZE_TO_PAGES(TableSize),
                           &DebugMessageBuffer
                           );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
//[-start-180502-IB08400586-modify]//
//[-start-180518-IB08400595-modify]//
  Status = gBS->AllocatePool (
                  EfiReservedMemoryType,
                  sizeof(DEBUG_MESSAGE_MEMORY_ADDR),
                  &mH2OStoreDebugMsgMemAddressPtr
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-180518-IB08400595-modify]//
  
  mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageSignature = DUMP_DEBUG_MESSAGE_INSTANCE_SIGNATURE;
  mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr = DebugMessageBuffer;
//[-start-180724-IB08400617-modify]//
//[-start-180823-IB07400999-modify]//
  Status = PcdSet32 (PcdH2OStoreDebugMsgMemAddressPtr, (UINT32)(UINTN)mH2OStoreDebugMsgMemAddressPtr);
  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-180823-IB07400999-modify]//
//[-end-180724-IB08400617-modify]//
//[-end-180502-IB08400586-modify]//
  
  //
  // Store DEBUG Message Head address and add signature '_EMSGE_', Entry/End Address and total size information.
  //
  DebugMessageHead = (DEBUG_MESSAGE_HEAD_STRUCTURE *)(UINTN)DebugMessageBuffer;
  ZeroMem ((VOID *)DebugMessageHead, TableSize);
  DebugMessageHead->DumpDebugMessageSignature   = DUMP_DEBUG_MESSAGE_INSTANCE_SIGNATURE;
  DebugMessageHead->TotalSize                   = TableSize;
  DebugMessageHead->AdmgStartAddr               = (UINT32)(DebugMessageBuffer + sizeof(DEBUG_MESSAGE_HEAD_STRUCTURE));    
  DebugMessageHead->AdmgEndAddr                 = (UINT32)(DebugMessageBuffer + sizeof(DEBUG_MESSAGE_HEAD_STRUCTURE));    
  DebugMessageHead->AdmgSize                    = (UINT32)(TableSize - sizeof(DEBUG_MESSAGE_HEAD_STRUCTURE));

  mDebugMessageEn = TRUE;

  //
  // Copy DEBUG messages which stored in PEI phase.
  //
//[-start-171212-IB08400542-modify]//
  if ((PcdGet32(PcdH2OPeiStoreDebugMsgEntry) == 0xFFFFFFFF) || (PcdGet32(PcdH2OPeiStoreDebugMsgEntry) == 0)) {
    //
    // There is no PEI DEBUG messages stored.
    //
    return EFI_SUCCESS;
  }
  
//[-start-180503-IB08400586-modify]//
  PeiDebugMessageHead = (PEI_DEBUG_MESSAGE_HEAD_STRUCTURE *)(UINTN)PcdGet32(PcdH2OPeiStoreDebugMsgEntry);
  PeiDebugMessageSize = PeiDebugMessageHead->AdmgEndAddr - PeiDebugMessageHead->AdmgStartAddr + 1;
  if (PeiDebugMessageSize != 0) {
    if (PeiDebugMessageSize <= DebugMessageHead->AdmgSize) {
      //
      // The size of DEBUG messages in PEI phase is less or equal than the DXE size.
      //
      CopyMem ((VOID*)(UINTN)(DebugMessageHead->AdmgStartAddr), (VOID*)(UINTN)(PeiDebugMessageHead->AdmgStartAddr), PeiDebugMessageSize);
      DebugMessageHead->AdmgEndAddr += (UINT32)(PeiDebugMessageSize - 1); 
    } else {
      //
      // The size of DEBUG messages in PEI phase is large than the DXE size.
      //
      CopyMem ((VOID*)(UINTN)(DebugMessageHead->AdmgStartAddr), (VOID*)(UINTN)(PeiDebugMessageHead->AdmgStartAddr), DebugMessageHead->AdmgSize);
      DebugMessageHead->AdmgEndAddr += (UINT32)(DebugMessageHead->AdmgSize - 1); 
    }
//[-end-180503-IB08400586-modify]//
  }
//[-end-171212-IB08400542-modify]//

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
static
VOID
EFIAPI
RscHandlerProtocolCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  )
{
  EFI_STATUS                        Status;

  Status = gBS->LocateProtocol (
                  &gEfiRscHandlerProtocolGuid,
                  NULL,
                  (VOID **) &mRscHandlerProtocol
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  
//[-start-180503-IB08400586-modify]//
  //
  // Register the worker function to ReportStatusCodeRouter
  //
  Status = mRscHandlerProtocol->Register (DebugMessageStatusCode, TPL_HIGH_LEVEL);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return;
  }

  //
  // Initialize the memory space for DEBUG messages.
  //
  Status = DebugMessageMemBufferInit ();
  if (EFI_ERROR (Status)) {
    return;
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  UnregisterBootTimeHandlers,
                  NULL,
                  &gEfiEventExitBootServicesGuid,
                  &mExitBootServicesEvent
                  );
  if (EFI_ERROR (Status)) {
  }
//[-end-180503-IB08400586-modify]//

  return;
}

/**
  Unregister status code callback functions only available at boot time from
  report status code router when exiting boot services.

  @param  Event         Event whose notification function is being invoked.
  @param  Context       Pointer to the notification function's context, which is
                        always zero in current implementation.
**/
//[-start-170413-IB08400444-modify]//
VOID
EFIAPI
UnregisterBootTimeHandlers (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  UINTN                                  CommSize;
  EFI_STATUS                             Status;
  EFI_SMM_COMMUNICATE_HEADER             *SmmCommBufferHeader;
  EFI_SMM_COMMUNICATION_PROTOCOL         *Communication;
//[-start-180419-IB08400580-modify]//
  SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE   *SmmSdmCommunicateData;
  UINT8                                  *SmmSdmBuffer;
//[-end-180419-IB08400580-modify]//
  
  if (mRscHandlerProtocol != NULL) {
    mRscHandlerProtocol->Unregister (DebugMessageStatusCode);
  }

//[-start-171212-IB08400542-modify]//
//[-start-180502-IB08400586-modify]//
  mH2OStoreDebugMsgMemAddressPtr->DumpDebugMessageHeadAddr = 0xFFFFFFFF;
//[-end-180502-IB08400586-modify]//
//[-end-171212-IB08400542-modify]//

  gBS->CloseEvent(Event); 
  mDebugMessageEn = FALSE;

  //
  // Unregister SMM handler for Debug Message Status Code.
  //
  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **)&Communication);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "DebugMessageStatusCode locate SMM Communication protocol: %r\n", Status));
    return;
  }

  //
  // Use to notify SMM driver that the Debug Message Status Code service is not available after exiting boot services.
  //
//[-start-180419-IB08400580-modify]//
  Status = gBS->AllocatePool (EfiRuntimeServicesData, SMM_DMSC_COMMUNICATION_BUFFER_SIZE, &SmmSdmBuffer);
  if (EFI_ERROR (Status)) {
    return;
  }

  SmmCommBufferHeader = (EFI_SMM_COMMUNICATE_HEADER*)SmmSdmBuffer;
  SmmSdmCommunicateData = (SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE *)SmmCommBufferHeader->Data;

  SmmSdmCommunicateData->Function = SMM_COMM_UNREGISTER_HANDLER;
  SmmSdmCommunicateData->UnregisterService = TRUE;
  SmmSdmCommunicateData->ReturnStatus = EFI_UNSUPPORTED;
  
  CopyGuid (&SmmCommBufferHeader->HeaderGuid, &gH2ODebugMessageStatusCodeCommunicationGuid);
  SmmCommBufferHeader->MessageLength = sizeof(SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE);
  CommSize = OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data)  + sizeof(SMM_DEBUG_MESSAGE_STATUS_CODE_COMMUNICATE);
  
  Status = Communication->Communicate (Communication, SmmSdmBuffer, &CommSize);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "DebugMessageStatusCode Unregister SMM Handler: %r\n", Status));
    return;
  }
  
  gBS->FreePool (SmmSdmBuffer);
//[-end-180419-IB08400580-modify]//
  
  return;
}
//[-end-170413-IB08400444-modify]//

/**
 The DEBUG Message driver will handle all events during DXE and BDS phase.
 The DEBUG messages came from Status Code reported.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
StoreDebugMessageDxeEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  ) 
{
  EFI_STATUS                                Status;
  VOID                                      *EventRegistration;
  EFI_EVENT                                 VarEvent;
  EFI_EVENT                                 RscHandlerProtocolEvent;
//[-start-170413-IB08400444-remove]//
//  EFI_EVENT                                 ReadyToBootEvent;
//[-end-170413-IB08400444-remove]//
//[-start-180729-IB08400620-add]//
  VOID                                      *RscEventRegistration;
//[-end-180729-IB08400620-add]//

  
  //
  // Create an evnet - Wait for Variable Write Architecture Protocol
  //
  Status = gBS->CreateEvent (
                         EVT_NOTIFY_SIGNAL,
                         TPL_CALLBACK,
                         InstalledVarNotificationFunction,
                         NULL,
                         &VarEvent
                         );
  if (!EFI_ERROR (Status)) {
    gBS->RegisterProtocolNotify (
                             &gEfiVariableWriteArchProtocolGuid,
                             VarEvent,
                             &EventRegistration
                             );
  }      

  Status = gBS->LocateProtocol (
                            &gEfiRscHandlerProtocolGuid,
                            NULL,
                            (VOID **) &mRscHandlerProtocol
                            );
  if (EFI_ERROR (Status)) {
    //
    // Register callback for registering StatusCode worker function for DEBUG messages.
    //
//[-start-180729-IB08400620-modify]//
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    RscHandlerProtocolCallback,
                    NULL,
                    &RscHandlerProtocolEvent
                    );
    if (!EFI_ERROR (Status)) {
      Status = gBS->RegisterProtocolNotify (
                      &gEfiRscHandlerProtocolGuid,
                      RscHandlerProtocolEvent,
                      &RscEventRegistration
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }
    } 
//[-end-180729-IB08400620-modify]//
  } else {
    //
    // Register the worker function for DEBUG messages.
    //
    Status = mRscHandlerProtocol->Register (DebugMessageStatusCode, TPL_HIGH_LEVEL);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
//[-start-180503-IB08400586-add]//
    //
    // Initialize the memory space for DEBUG messages.
    //
    Status = DebugMessageMemBufferInit ();
    if (EFI_ERROR (Status)) {
      return Status;
    }
//[-end-180503-IB08400586-add]//

//[-start-170413-IB08400444-modify]//
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    UnregisterBootTimeHandlers,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &mExitBootServicesEvent
                    );
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR, "Create event to unregister DebugMessageStatusCode(): %r\n", Status));
    }
//[-end-170413-IB08400444-modify]//
  }

  return Status;
}
