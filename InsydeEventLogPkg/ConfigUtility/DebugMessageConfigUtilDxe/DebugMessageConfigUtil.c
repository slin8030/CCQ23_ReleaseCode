/** @file

  Initial and callback functions for DEBUG Message Config utility DXE

;******************************************************************************
;* Copyright (c) 2016 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <DebugMessageConfigUtil.h>
//[-start-180518-IB08400595-add]//
#include <Library/EventLogDebugMsgLib.h>
//[-end-180518-IB08400595-add]//

DEBUG_MESSAGE_STRING                      *mDebugMsgData = NULL;
UINTN                                     mDebugMsgDataNum = 0;
UINTN                                     mCurrentListStartIndex;
UINTN                                     mCurrentListEndIndex;
EFI_HANDLE                                mDriverHandle[2] = {0};
DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_DATA   *mPrivateData = NULL;
CHAR16                                    mVariableName[] = H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME;
HII_VENDOR_DEVICE_PATH                    mHiiVendorDevicePath0 = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    H2O_DEBUG_MESSAGE_CONFIG_UTIL_CONFIG_PAGE_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

HII_VENDOR_DEVICE_PATH                    mHiiVendorDevicePath1 = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    H2O_DEBUG_MESSAGE_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      (UINT8) (END_DEVICE_PATH_LENGTH),
      (UINT8) ((END_DEVICE_PATH_LENGTH) >> 8)
    }
  }
};

//[-start-180213-IB08400569-add]//
EFI_STRING_ID                 mShowEventStringId[MAX_SHOW_ELV_LIST_NUM] = {0};
EFI_STRING_ID                 mTotalCountStringId = 0;
EFI_STRING_ID                 mCurrentCountStringId = 0;
//[-end-180213-IB08400569-add]//

/**
 Search first "Simple File System" protocol.    

 Param[in]  Removable   Is device of the "Simple File System" removable.
 
 @retval    Return find out "Simple File System" protoco, 
            Return NULL if not found                 
*/
EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *
EFIAPI
SearchFileSystem (
  BOOLEAN Removable
  )
{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *Volume;
  EFI_STATUS                          Status;
  EFI_HANDLE                         *HandleBuffer;
  UINTN                               NumberOfHandles;
  UINTN                               Index;
  EFI_BLOCK_IO_PROTOCOL              *BlkIo;


  Volume       = NULL;
  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSimpleFileSystemProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {

    BlkIo  = NULL;
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiBlockIoProtocolGuid, (VOID **)&BlkIo);

    if (BlkIo != NULL) {
      if (BlkIo->Media->ReadOnly) {
        continue;
      }
      if (Removable && !BlkIo->Media->RemovableMedia) {
        continue;
      }
    }
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Volume);
    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

  return Volume;
}

VOID
EFIAPI
ValueToString (
  UINTN     Value,
  UINTN     Digitals,
  CHAR16    *Buffer,
  UINTN     Flags
  )
{
  CHAR8   Str[30];
  UINTN   mod;
  CHAR8   *p1;
  CHAR16  *p2;
  CHAR16  StuffChar = 0;
  UINTN   count = 0;
  UINTN   Padding;

  p1  = Str;
  p2  = Buffer;

  if (Flags == VTS_RIGHT_ALIGN)
    StuffChar = L' ';

  if (Flags == VTS_LEAD_0)
    StuffChar = L'0';

  if (Value == 0) {

    // Stuff specify char to buffer
    if (StuffChar != 0) {
      Padding = Digitals - 1;
      
      while (Padding > 0) {
        *p2 = StuffChar;
        p2++;
        Padding--;      
      }
    }
    
    *p2++ = '0';
    *p2 = 0;
    return ;
  }
  
  while (Value) {
    mod = Value % 10;
    *p1 = (CHAR8)(mod + '0');
    Value = Value / 10;
    p1++;
    count++;
  }

  if ((count < Digitals) && (StuffChar != 0)) {
    Padding = Digitals - count;
    while (Padding) {
      *p2 = StuffChar;
      p2++;
      Padding--;
    }
  }

  while (count > 0) {
    p1--;
    *p2 = *p1;
    p2++;

    count--;
  }

  *p2 = 0;
}

/**
 Save the data buffer to file system.

 @param[in]  *Volume                   Simple File System protocol.  
 @param[in]  *FileName                 File name.    
 @param[in]  *SavingDataBuffer         Date buffer to be stored. 
 @param[in]  SavingDataSize            Size of data buffer.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
CreateSavingFile (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *Volume,  
  IN CHAR16                             *FileName,
  IN VOID                               *SavingDataBuffer,
  IN UINTN                              SavingDataSize
  )
{
  EFI_FILE                                *Root;
  EFI_FILE                                *File;
  EFI_STATUS                              Status;

  File = NULL;

  Status = Volume->OpenVolume (
                     Volume,
                     &Root
                     );
  if (EFI_ERROR (Status)) {
    return EFI_NO_MEDIA;
  }

  //
  // Confirm the FileName is existence or not.
  //
  Status = Root->Open (
                   Root,
                   &File,
                   FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                   0
                   );
  if (EFI_ERROR (Status)) {
    //
    // No the same FileName be found. Create it and write event to it.
    //
    Status = Root->Open (
                     Root,
                     &File,
                     FileName,
                     EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                     0
                     );
    if (EFI_ERROR (Status)) {      
      Root->Close (Root);
      return EFI_NO_MEDIA;
    }
  }  
  //
  // The same FileName is existence. Overwrite event to the same file
  //
  Status = Root->Write (File, &SavingDataSize, SavingDataBuffer);
//[-start-160202-IB05080551-add]//
  if (EFI_ERROR (Status)) {      
    Root->Close (Root);
    return Status;
  }
//[-end-160202-IB05080551-add]//  
  Status = Root->Open (
                   Root,
                   &File,
                   FileName,
                   EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                   0
                   );
  Root->Close (Root);

  return Status;
}

EFI_STATUS
EFIAPI
ObtainSavingFileNameByTime (
  IN CHAR16                                 *StorageName,
  IN OUT CHAR16                             **FileName
  )
{
  EFI_STATUS                           Status;
  EFI_TIME                             EfiTime;
  CHAR16                               Buffer[5];
  UINTN                                FileNameSize=0;
  VOID                                 *TempString=NULL;

  //
  // File name will be defined as "YYYYMMDDHHMMSS"+"storage name"+".log"
  // ex: file name = 20091111171800+StorageName+.log
  //
//[-start-160315-IB08400337-modify]//
  FileNameSize = (StrLen(L"DEBUG_") + StrLen(L"20140101171800.log") + 1) * sizeof(CHAR16) ;
//[-end-160315-IB08400337-modify]//

  Status = gBS->AllocatePool (EfiBootServicesData, FileNameSize, (VOID **)&TempString);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = gRT->GetTime (&EfiTime, NULL);
  if (EFI_ERROR(Status)) {
    return Status;
  }

//[-start-180724-IB08400617-modify]//
  ValueToString (EfiTime.Year, 4, Buffer, VTS_LEAD_0);
  StrCpyS (TempString, 5, Buffer);
  
  ValueToString (EfiTime.Month, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Day, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Hour, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Minute, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  ValueToString (EfiTime.Second, 2, Buffer, VTS_LEAD_0);
  StrCatS (TempString, 5, Buffer);

  StrCatS (TempString, MAX_STORAGE_NAME_NUM, StorageName);

  StrCatS (TempString, 5, L".log");
//[-end-180724-IB08400617-modify]//

  *FileName = TempString;

  return EFI_SUCCESS;
}

/**
 Display Dialog for User Requirement which dilag type is Yes/No, Yes/No/Cancel, Ok or Ok/Cancel

 @param[in]  DialogOperation           Dialog type (Yes/No, Yes/No/Cancel, Ok or Ok/Cancel)
 @param[in]  HotKey                    HotKey information
 @param[in]  MaximumStringSize         Maximum string length
 @param[out] StringBuffer              String buffer
 @param[out] KeyValue                  Ptr to returned structure that indicates the key the user selected.
 @param[in]  String                    Ptr to null-terminated string that specifies the dialog prompt

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ElvConfirmDialog (
  IN  UINT32                           DialogOperation,
  IN  BOOLEAN                          HotKey,
  IN  UINT32                           MaximumStringSize,
  OUT CHAR16                           *StringBuffer,
  OUT EFI_INPUT_KEY                    *KeyValue,
  IN  CHAR16                           *String
  )
{
  EFI_STATUS                                 Status;
  H2O_DIALOG_PROTOCOL                        *H2ODialog;
  
  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **)&H2ODialog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  return H2ODialog->ConfirmDialog (
                      DialogOperation,
                      HotKey,
                      MaximumStringSize,
                      StringBuffer,
                      KeyValue,
                      String
                      );
  
}

/**
 Update the items for the Serial Debug Message page in the Event Log Viewer.

 @param[in]  HiiHandle                 EFI HII handle.
 @param[in]  ShowAction                Determine the show policy when update the item of Serial Debug Message page.    

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
UpdateEfiDebugMsgPage (
  IN  EFI_HII_HANDLE      HiiHandle,
  IN  UINT8               ShowAction
  )
{
//[-start-180214-IB08400569-modify]//
  VOID                          *StartOpCodeHandle = NULL;
  VOID                          *EndOpCodeHandle = NULL;
  EFI_IFR_GUID_LABEL            *StartLabel;
  EFI_IFR_GUID_LABEL            *EndLabel;
  EFI_STATUS                    Status;
  CHAR16                        StringPtr[MAX_ELV_STR_NUM] = {0};
  UINTN                         Index;
  EFI_GUID                      FormSetGuid = H2O_DEBUG_MESSAGE_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID;
  CHAR16                        DebugUnicodeStringPtr[MAX_ELV_STR_NUM] = {0};
  CHAR8                         DebugAsciiStringPtr[MAX_ELV_STR_NUM] = {0};
  EFI_STRING_ID                 TempStringId;
  UINTN                         EventStringIndex = 0;
//[-end-180214-IB08400569-modify]//

  if (ShowAction == SHOW_FIRST_PAGE) {
    if (mDebugMsgDataNum == 0) {
      // NO DEBUG messages.
      mCurrentListStartIndex = 0;
      mCurrentListEndIndex   = 0;
    } else {
      mCurrentListStartIndex = 1;
      if ((mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1) <= mDebugMsgDataNum) {
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1;
      } else {
        mCurrentListEndIndex = mDebugMsgDataNum;
      }
    }  
  } else {
    //
    // Show action: Show Previous or Next page of Events.
    //
    if (ShowAction == SHOW_PREVIOUS_PAGE) {
      if (mCurrentListStartIndex <= MAX_SHOW_ELV_LIST_NUM) {
        //
        // No more previours events, skip.
        //
        return EFI_SUCCESS;
      } else {
        mCurrentListStartIndex = mCurrentListStartIndex - MAX_SHOW_ELV_LIST_NUM;
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1;
      }
    } else if (ShowAction == SHOW_NEXT_PAGE) {
      if (mCurrentListEndIndex == mDebugMsgDataNum) {
        //
        // No more next events, skip.
        //
        return EFI_SUCCESS;
      } 
      
      if ((mCurrentListEndIndex + MAX_SHOW_ELV_LIST_NUM) <= mDebugMsgDataNum) {
        mCurrentListStartIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM;
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM - 1;
      } else {
        mCurrentListStartIndex = mCurrentListStartIndex + MAX_SHOW_ELV_LIST_NUM;
        mCurrentListEndIndex = mDebugMsgDataNum;
      }
    } else {
      return EFI_INVALID_PARAMETER;
    }
  }

  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = ELV_SERIAL_DEBUG_HEADER_START_LABEL; 

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = ELV_SERIAL_DEBUG_HEADER_END_LABEL; 

  if ((mCurrentListStartIndex != 1) && (mCurrentListStartIndex != 0)) {
    HiiCreateActionOpCode (
    StartOpCodeHandle,                                 // Container for dynamic created opcodes
    (EFI_QUESTION_ID) KEY_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE, // Question ID
    STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE),// String ID for Prompt
    STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE),// String ID for Help
    EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
    0
    );
  }
  
  if (mCurrentListEndIndex != mDebugMsgDataNum) {
    HiiCreateActionOpCode (
    StartOpCodeHandle,                                 // Container for dynamic created opcodes
    (EFI_QUESTION_ID) KEY_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE,// Question ID
    STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE),    // String ID for Prompt
    STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE),    // String ID for Help
    EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
    0
    );
  }

  //
  // For "Serial Debug Messages"
  //
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"- Total Message Count     : %d",
    mDebugMsgDataNum
    );
//[-start-180213-IB08400569-modify]//
  if (mTotalCountStringId == 0) {    
    mTotalCountStringId = HiiSetString (HiiHandle, 0, StringPtr, NULL);
  } else {    
    TempStringId = HiiSetString (HiiHandle, mTotalCountStringId, StringPtr, NULL);
    if (TempStringId != mTotalCountStringId) {
      mTotalCountStringId = TempStringId;
      DEBUG ((EFI_D_ERROR, "String ID changed. New mTotalCountStringId = 0x%x\n", mTotalCountStringId));      
    }
  }

  HiiCreateSubTitleOpCode (
    StartOpCodeHandle,
    mTotalCountStringId,
    0,
    0,
    0
    );
//[-end-180213-IB08400569-modify]//
  
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"- Current Message Number: %d to %d",
    mCurrentListStartIndex,
    mCurrentListEndIndex
    );
//[-start-180214-IB08400569-modify]//
  if (mCurrentCountStringId == 0) {    
    mCurrentCountStringId = HiiSetString (HiiHandle, 0, StringPtr, NULL);
  } else {    
    TempStringId = HiiSetString (HiiHandle, mCurrentCountStringId, StringPtr, NULL);
    if (TempStringId != mCurrentCountStringId) {
      mCurrentCountStringId = TempStringId;
      DEBUG ((EFI_D_ERROR, "String ID changed. New mCurrentCountStringId = 0x%x\n", mCurrentCountStringId));      
    }
  }  
  HiiCreateSubTitleOpCode (
    StartOpCodeHandle,
    mCurrentCountStringId,
    0,
    0,
    0
    );
//[-end-180214-IB08400569-modify]//
  
  Status = HiiUpdateForm (
             HiiHandle,
             &FormSetGuid, 
             (EFI_FORM_ID)VFR_FORMID_SHOW_SERIAL_DEBUG_MESSAGE,
             StartOpCodeHandle,
             EndOpCodeHandle
             );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  //
  // Add Messages List.
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);
  
  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = ELV_SERIAL_DEBUG_EVENTS_START_LABEL; 

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = ELV_SERIAL_DEBUG_EVENTS_END_LABEL; 

  if (mDebugMsgDataNum == 0) {
    //
    // Clear the Event List.
    //
    Status = HiiUpdateForm (
               HiiHandle,
               &FormSetGuid, 
               (EFI_FORM_ID)VFR_FORMID_SHOW_SERIAL_DEBUG_MESSAGE,
               StartOpCodeHandle,
               EndOpCodeHandle
               );
  } else {
//[-start-180214-IB08400569-modify]//
    // Print the Event List.
    for (Index = (mCurrentListStartIndex - 1), EventStringIndex = 0; Index < mCurrentListEndIndex; Index++, EventStringIndex++) {
      ZeroMem (DebugUnicodeStringPtr, sizeof(DebugUnicodeStringPtr));
//[-start-180724-IB08400617-modify]//
      AsciiStrnCpyS (DebugAsciiStringPtr, MAX_ELV_STR_NUM, mDebugMsgData[Index].DebugMessage, mDebugMsgData[Index].DebugMsgLen);
      DebugAsciiStringPtr[mDebugMsgData[Index].DebugMsgLen] = '\0';
//[-start-180823-IB07400999-modify]//
      AsciiStrToUnicodeStr (DebugAsciiStringPtr, DebugUnicodeStringPtr);
//[-end-180823-IB07400999-modify]//
//[-end-180724-IB08400617-modify]//
      DebugUnicodeStringPtr[mDebugMsgData[Index].DebugMsgLen * 2] = L'\0';
      UnicodeSPrint (
        StringPtr, 
        sizeof (StringPtr), 
        L"[%05d] %s",
        (Index + 1),
        DebugUnicodeStringPtr
        );

      if (mShowEventStringId[EventStringIndex] == 0) {
        mShowEventStringId[EventStringIndex] = HiiSetString (HiiHandle, 0, StringPtr, NULL);
      } else {
        TempStringId = HiiSetString (HiiHandle, mShowEventStringId[EventStringIndex], StringPtr, NULL);
        if (TempStringId != mShowEventStringId[EventStringIndex]) {
          mShowEventStringId[EventStringIndex] = TempStringId;
          DEBUG ((EFI_D_ERROR, "String ID changed. New mShowEventStringId[%x] = 0x%x\n", EventStringIndex, mShowEventStringId[EventStringIndex]));      
        }
      }
      DEBUG ((EFI_D_ERROR, "mShowEventStringId[%x]: 0x%x\n", EventStringIndex, mShowEventStringId[EventStringIndex]));
      if (mShowEventStringId[EventStringIndex] == 0) {
        DEBUG ((EFI_D_ERROR, "Cannot Set String ID. mShowEventStringId[%x] = 0\n", EventStringIndex));
      }
      
      HiiCreateActionOpCode (
        StartOpCodeHandle,             // Container for dynamic created opcodes
        (EFI_QUESTION_ID) (ELV_LIST_BASE + Index),    // Question ID
        mShowEventStringId[EventStringIndex],         // String ID for Prompt
        0,                             // String ID for Help
        EFI_IFR_FLAG_CALLBACK,         // Flags in Question Header
        0
        );
      
    }
//[-end-180214-IB08400569-modify]//

    
    if (mCurrentListStartIndex != 1) {
      HiiCreateActionOpCode (
      StartOpCodeHandle,                                 // Container for dynamic created opcodes
      (EFI_QUESTION_ID) KEY_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE2, // Question ID
      STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE),// String ID for Prompt
      STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE),// String ID for Help
      EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
      0
      );
    }
    
    if (mCurrentListEndIndex != mDebugMsgDataNum) {
      HiiCreateActionOpCode (
      StartOpCodeHandle,                                 // Container for dynamic created opcodes
      (EFI_QUESTION_ID) KEY_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE2,// Question ID
      STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE),    // String ID for Prompt
      STRING_TOKEN(STR_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE),    // String ID for Help
      EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
      0
      );
    }
    
    Status = HiiUpdateForm (
               HiiHandle,
               &FormSetGuid, 
               (EFI_FORM_ID)VFR_FORMID_SHOW_SERIAL_DEBUG_MESSAGE,
               StartOpCodeHandle,
               EndOpCodeHandle
               );
    
  }

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);
  
  return EFI_SUCCESS;
}

/**
 Show the Serial Debug Messages on the DEBUG Messages page.

 @param[in]  HiiHandle                 EFI HII Handle.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ShowEfiDebugMessage (
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  EFI_STATUS                            Status;
  CHAR16                                StringPtr2[MAX_ELV_STR_NUM] = {0};  
  EFI_INPUT_KEY                         Key;
  CHAR8                                 *DebugAsciiString;
  UINTN                                 DebugAsciiStringSize = 0;  
  UINTN                                 DebugUnicodeStringSize = 0; 
  UINTN                                 Index;
  UINTN                                 DebugStrNum = 0;
  UINTN                                 DebugMsgCount;
  CHAR8                                 *DebugMsgStart = NULL;
  UINTN                                 DebugMsgDataIndex;
//[-start-180518-IB08400595-add]//  
  UINT32                                AdmgStartAddr;
  UINT32                                AdmgEndAddr;
//[-end-180518-IB08400595-add]//
  
  //
  // Check if Serial Debug Message Viewer is available.
  //
//[-start-180518-IB08400595-modify]//
  Status = GetDebugMessageBufferInfo(&AdmgStartAddr, &AdmgEndAddr, NULL, NULL);
  if (EFI_ERROR (Status)) {
    ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Get Serial Debug Message: %r",
      Status
      );
    ElvConfirmDialog (
      DlgOk,
      FALSE,
      0,
      NULL,
      &Key,
      StringPtr2
      );
    return EFI_NOT_FOUND;
  } 
  
  DebugAsciiString = (CHAR8 *)(UINTN)(AdmgStartAddr);
  DebugAsciiStringSize = (UINTN)(AdmgEndAddr - AdmgStartAddr);
  DebugUnicodeStringSize = DebugAsciiStringSize * 2;
  if (AdmgEndAddr == AdmgStartAddr) {      
    ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"There is no any Serial Debug Messages."
      );
    ElvConfirmDialog (
      DlgOk,
      FALSE,
      0,
      NULL,
      &Key,
      StringPtr2
      );
    return EFI_NOT_FOUND;
  }
//[-end-180518-IB08400595-modify]//


  //
  // First, try to get the total count of the DEBUG messages
  //
  Index = 0;
  DebugMsgCount = 0;
  while (Index < DebugAsciiStringSize) {
    DebugStrNum = 1;
    while ((DebugAsciiString[Index] != 0x0D) || (DebugAsciiString[Index + 1] != 0x0A)) {
      Index++;
      DebugStrNum++;
    }     
    Index++;
    DebugMsgCount++;
  }

  //
  // Second, create a buffer to store the DEBUG message strings.
  //
  if (mDebugMsgData != NULL) {
    FreePool (mDebugMsgData);
    mDebugMsgData = NULL;
  }
  mDebugMsgData = (DEBUG_MESSAGE_STRING *)AllocateZeroPool (sizeof (DEBUG_MESSAGE_STRING) * DebugMsgCount);
  if (mDebugMsgData == NULL) {
    mDebugMsgDataNum = 0;
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Can't get the Serial Debug messages."
      );
    ElvConfirmDialog (
      DlgOk,
      FALSE,
      0,
      NULL,
      &Key,
      StringPtr2
      );
    return EFI_NOT_FOUND;
  }
  
  mDebugMsgDataNum = DebugMsgCount;
  Index = 0;
  DebugMsgDataIndex = 0;
  while (Index < DebugAsciiStringSize) {
    DebugMsgStart = &(DebugAsciiString[Index]);
    DebugStrNum = 1;
    while ((DebugAsciiString[Index] != 0x0D) || (DebugAsciiString[Index + 1] != 0x0A)) {
      Index++;
      DebugStrNum++;
    }     

    //
    // Store each DEBUG message.
    //
    mDebugMsgData[DebugMsgDataIndex].DebugMessage = DebugMsgStart;
    mDebugMsgData[DebugMsgDataIndex].DebugMsgLen = DebugStrNum - 1;
    Index += 2;
    DebugMsgDataIndex++;
  }
  
  Status = UpdateEfiDebugMsgPage (HiiHandle, SHOW_FIRST_PAGE);
      
  return Status;
}

/**
 Save the Serial Debug Messages on the DEBUG Messages page.

 @param[in]  HiiHandle                 EFI HII Handle.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
SaveEfiDebugMsg (
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  EFI_STATUS                            Status;
  CHAR16                                StringPtr2[MAX_ELV_STR_NUM] = {0};  
  EFI_INPUT_KEY                         Key;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Volume = NULL;
  CHAR16                                *FileName = NULL;
  CHAR8                                 *DebugAsciiString;
  UINTN                                 DebugAsciiStringSize = 0;  
//[-start-180518-IB08400595-add]//
  UINT32                                AdmgStartAddr;
  UINT32                                AdmgEndAddr;
//[-end-180518-IB08400595-add]//

  //
  // Check if Serial Debug Message Viewer is available.
  //
//[-start-180518-IB08400595-modify]//
  Status = GetDebugMessageBufferInfo(&AdmgStartAddr, &AdmgEndAddr, NULL, NULL);
  if (EFI_ERROR (Status)) {
    ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Get Serial Debug Message Failure: %r",
      Status
      );
    ElvConfirmDialog (
      DlgOk,
      FALSE,
      0,
      NULL,
      &Key,
      StringPtr2
      );
    return EFI_NOT_FOUND;
  }

  DebugAsciiString = (CHAR8 *)(UINTN)(AdmgStartAddr);
  DebugAsciiStringSize = (UINTN)(AdmgEndAddr - AdmgStartAddr);
  if (AdmgEndAddr == AdmgStartAddr) {      
    ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"There is no any Serial Debug Messages."
      );
    ElvConfirmDialog (
      DlgOk,
      FALSE,
      0,
      NULL,
      &Key,
      StringPtr2
      );
    return EFI_NOT_FOUND;
  }
//[-end-180518-IB08400595-modify]//
  
  //
  // 1. Get removable or non-removable device for saving event(s)
  // Firstly, search removable device
  //
  Volume = SearchFileSystem (TRUE);
  if (Volume == NULL) {
    //
    // Search non-removable device
    //
    Volume = SearchFileSystem (FALSE);
  }
  
  if (Volume == NULL) {    
    ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Can't find any file system to save file."
      );
    ElvConfirmDialog (
      DlgOk,
      FALSE,
      0,
      NULL,
      &Key,
      StringPtr2
      );
    return EFI_NOT_FOUND;
  }


  //
  // 2. Use User-defined file name or Create a file name. ex: DEBUG_20150101171800.log
  //      
  ObtainSavingFileNameByTime (L"DEBUG", &FileName);  
    
  Status = CreateSavingFile (Volume, FileName, DebugAsciiString,DebugAsciiStringSize);
  ZeroMem (StringPtr2, sizeof(StringPtr2));
  UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Save Serial Debug Messages to %s.",
      FileName
      ); 
  ElvConfirmDialog (
    DlgOk,
    FALSE,
    0,
    NULL,
    &Key,
    StringPtr2
    );
  
  return EFI_SUCCESS;
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
                                 <ConfigAltResp> format which has all values filled
                                 in for the names in the Request string. String to
                                 be allocated by the called function.

  @retval EFI_SUCCESS            The Results is filled with the requested values.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                       Status;
  UINTN                            BufferSize;
  EFI_HII_CONFIG_ROUTING_PROTOCOL  *HiiConfigRouting;
  EFI_STRING                       ConfigRequest;
  EFI_STRING                       ConfigRequestHdr;
  UINTN                            Size;
  BOOLEAN                          AllocatedRequest;
  EFI_STRING                       Value;
  UINTN                            ValueStrLen;
  UINT8                            MyNameValue0;
  CHAR16                           BackupChar;
//[-start-180507-IB08400587-add]//
  CHAR16                           *StrPointer;
//[-end-180507-IB08400587-add]//

  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr  = NULL;
  ConfigRequest     = NULL;
  Size              = 0;
  *Progress         = Request;
  AllocatedRequest  = FALSE;

  HiiConfigRouting = mPrivateData->HiiConfigRouting;

  //
  // Get Buffer Storage data from EFI variable.
  // Try to get the current setting from variable.
  //
  BufferSize = sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION);
  Status = gRT->GetVariable (
                  mVariableName,
                  &gH2ODebugMessageConfigUtilVarstoreGuid,
                  NULL,
                  &BufferSize,
                  &mPrivateData->Configuration
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  if (Request == NULL) {
    //
    // Request is set to NULL, construct full request string.
    //

    //
    // Allocate and fill a buffer large enough to hold the <ConfigHdr> template
    // followed by "&OFFSET=0&WIDTH=WWWWWWWWWWWWWWWW" followed by a Null-terminator
    //
    ConfigRequestHdr = HiiConstructConfigHdr (&gH2ODebugMessageConfigUtilVarstoreGuid, mVariableName, mPrivateData->DriverHandle[0]);
    if (ConfigRequestHdr == NULL) {
      ASSERT (ConfigRequestHdr != NULL);
      return EFI_OUT_OF_RESOURCES;
    }
    Size = (StrLen (ConfigRequestHdr) + 32 + 1) * sizeof (CHAR16);
    ConfigRequest = AllocateZeroPool (Size);
    if (ConfigRequest == NULL) {
      ASSERT (ConfigRequest != NULL);
      return EFI_OUT_OF_RESOURCES;
    }
    AllocatedRequest = TRUE;
    UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", ConfigRequestHdr, (UINT64)BufferSize);
    FreePool (ConfigRequestHdr);
    ConfigRequestHdr = NULL;
  } else {
    //
    // Check routing data in <ConfigHdr>.
    // Note: if only one Storage is used, then this checking could be skipped.
    //
    if (!HiiIsConfigHdrMatch (Request, &gH2ODebugMessageConfigUtilVarstoreGuid, NULL)) {
      return EFI_NOT_FOUND;
    }
    //
    // Check whether request for EFI Varstore. EFI varstore get data
    // through hii database, not support in this path.
    //
    if (!HiiIsConfigHdrMatch (Request, &gH2ODebugMessageConfigUtilVarstoreGuid, mVariableName)) {
      return EFI_NOT_FOUND;
    }
    //
    // Set Request to the unified request string.
    //
    ConfigRequest = Request;
//[-start-180507-IB08400587-add]//
    //
    // Check whether Request includes Request Element.
    //
    if (StrStr (Request, L"OFFSET") == NULL) {
      //
      // Check Request Element does exist in Reques String
      //
      StrPointer = StrStr (Request, L"PATH");
      if (StrPointer == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      if (StrStr (StrPointer, L"&") == NULL) {
        Size = (StrLen (Request) + 32 + 1) * sizeof (CHAR16);
        ConfigRequest    = AllocateZeroPool (Size);
        ASSERT (ConfigRequest != NULL);
        AllocatedRequest = TRUE;
        UnicodeSPrint (ConfigRequest, Size, L"%s&OFFSET=0&WIDTH=%016LX", Request, (UINT64)BufferSize);
      }
    }    
//[-end-180507-IB08400587-add]//
  }

  if (StrStr (ConfigRequest, L"OFFSET") == NULL) {
    //
    // If requesting Name/Value storage, return value 0.
    //
    //
    // Allocate memory for <ConfigResp>, e.g. Name0=0x11, Name1=0x1234, Name2="ABCD"
    // <Request>   ::=<ConfigHdr>&Name0&Name1&Name2
    // <ConfigResp>::=<ConfigHdr>&Name0=11&Name1=1234&Name2=0041004200430044
    //
    BufferSize = (StrLen (ConfigRequest) + 1 + sizeof (MyNameValue0) * 2 + 1) * sizeof (CHAR16);
    *Results = AllocateZeroPool (BufferSize);
    if (*Results == NULL) {
      ASSERT (*Results != NULL);
      return EFI_OUT_OF_RESOURCES;
    }
//[-start-180724-IB08400617-modify]//
    StrCpyS (*Results, (BufferSize / sizeof(CHAR16)) + 1, ConfigRequest);
//[-end-180724-IB08400617-modify]//
    Value = *Results;

    //
    // Append value of NameValueVar0, type is UINT8
    //
    if ((Value = StrStr (*Results, L"MyNameValue0")) != NULL) {
      MyNameValue0 = 0;

      Value += StrLen (L"MyNameValue0");
      ValueStrLen = ((sizeof (MyNameValue0) * 2) + 1);
//[-start-180724-IB08400617-modify]//
      CopyMem (&Value[ValueStrLen], Value, StrSize (Value));

      BackupChar = Value[ValueStrLen];
      *Value++   = L'=';
//[-start-180823-IB07400999-modify]//
      Value += UnicodeValueToString (
                 Value,
                 PREFIX_ZERO | RADIX_HEX,
                 MyNameValue0,
                 sizeof (MyNameValue0) * 2
                 );
//[-end-180823-IB07400999-modify]//
      *Value = BackupChar;
//[-end-180724-IB08400617-modify]//
    }
    return EFI_SUCCESS;
  }

//[-start-180507-IB08400587-modify]//
  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               (UINT8 *)&mPrivateData->Configuration,
                               sizeof(DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION),
                               Results,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;    
  }
  ConfigRequestHdr = HiiConstructConfigHdr (&gH2ODebugMessageConfigUtilVarstoreGuid, mVariableName, mPrivateData->DriverHandle[0]);
//[-end-180507-IB08400587-modify]//
  
  //
  // Free the allocated config request string.
  //
  if (AllocatedRequest) {
    FreePool (ConfigRequest);
  }

  if (ConfigRequestHdr != NULL) {
    FreePool (ConfigRequestHdr);
  }
  //
  // Set Progress string to the original request string.
  //
  if (Request == NULL) {
    *Progress = NULL;
  } else if (StrStr (Request, L"OFFSET") == NULL) {
    *Progress = Request + StrLen (Request);
  }

  return EFI_SUCCESS;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent '&' before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name/value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.
**/
EFI_STATUS
EFIAPI
RouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION       DebugMessageConfig = {0};
  EFI_STATUS                                    Status;
  EFI_HII_CONFIG_ROUTING_PROTOCOL               *HiiConfigRouting;
  UINTN                                         BufferSize;
  
  if (Configuration == NULL || Progress == NULL || This == NULL) {
   return EFI_INVALID_PARAMETER;
  }

//[-start-180510-IB08400591-add]//
  *Progress = Configuration;
//[-end-180510-IB08400591-add]//

//[-start-180507-IB08400587-modify]//
  if (!HiiIsConfigHdrMatch(Configuration, &gH2ODebugMessageConfigUtilVarstoreGuid, mVariableName)) {
    return EFI_NOT_FOUND;
  }
//[-end-180507-IB08400587-modify]//

  HiiConfigRouting = mPrivateData->HiiConfigRouting;
  BufferSize = sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION);
  Status = HiiConfigRouting->ConfigToBlock (
                               HiiConfigRouting,
                               Configuration,
                               (UINT8 *) &DebugMessageConfig,
                               &BufferSize,
                               Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  //
  // Store broswer data Buffer Storage to EFI variable
  //
  Status = CommonSetVariable (
             mVariableName,
             &gH2ODebugMessageConfigUtilVarstoreGuid,
             EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
             sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION),
             &DebugMessageConfig
             );
  return Status;
  
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Action                 Specifies the type of action taken by the browser.
  @param  QuestionId             A unique value which is sent to the original
                                 exporting driver so that it can identify the type
                                 of data to expect.
  @param  Type                   The type of value for the question.
  @param  Value                  A pointer to the data being sent to the original
                                 exporting driver.
  @param  ActionRequest          On return, points to the action requested by the
                                 callback function.

  @retval EFI_SUCCESS            The callback successfully handled the action.
  @retval EFI_OUT_OF_RESOURCES   Not enough storage is available to hold the
                                 variable and its data.
  @retval EFI_DEVICE_ERROR       The variable could not be saved.
  @retval EFI_UNSUPPORTED        The specified Action is not supported by the
                                 callback.

**/
EFI_STATUS
EFIAPI
DriverCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  EFI_STATUS                                 Status;
  H2O_DIALOG_PROTOCOL                        *H2ODialog;

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Action) {
  case EFI_BROWSER_ACTION_FORM_OPEN:
    DEBUG ((EFI_D_INFO, "DebugMessageConfigUtil EFI_BROWSER_ACTION_FORM_OPEN. \n"));
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    DEBUG ((EFI_D_INFO, "DebugMessageConfigUtil EFI_BROWSER_ACTION_CHANGING. \n"));
    {
      Status = gBS->LocateProtocol (
                      &gH2ODialogProtocolGuid,
                      NULL,
                      (VOID **)&H2ODialog
                      );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      switch (QuestionId) {
      //
      // For Serial Debug message
      //
      case KEY_ELV_SHOW_EFI_DEBUG_MSG:
        DEBUG ((EFI_D_INFO, "KEY_ELV_SHOW_EFI_DEBUG_MSG!!\n"));
        Status = ShowEfiDebugMessage (mPrivateData->HiiHandle[1]);
        break;
        
      case KEY_ELV_SAVE_EFI_DEBUG_MSG:
        DEBUG ((EFI_D_INFO, "KEY_ELV_SAVE_EFI_DEBUG_MSG!!\n"));
        Status = SaveEfiDebugMsg (mPrivateData->HiiHandle[1]); 
        break;  

      case KEY_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE:
      case KEY_SHOW_SERIAL_DEBUG_MSG_PREVIOUS_PAGE2:  
        Status = UpdateEfiDebugMsgPage (mPrivateData->HiiHandle[1], SHOW_PREVIOUS_PAGE);
        break; 

      case KEY_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE:
      case KEY_SHOW_SERIAL_DEBUG_MSG_NEXT_PAGE2:  
        Status = UpdateEfiDebugMsgPage (mPrivateData->HiiHandle[1], SHOW_NEXT_PAGE);
        break;  

      default:
        break;
      }

    }
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    DEBUG ((EFI_D_INFO, "DebugMessageConfigUtil EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING. \n"));
    break;

  case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
    DEBUG ((EFI_D_INFO, "DebugMessageConfigUtil EFI_BROWSER_ACTION_DEFAULT_STANDARD. \n"));
    break;

  case EFI_BROWSER_ACTION_RETRIEVE:
    DEBUG ((EFI_D_INFO, "DebugMessageConfigUtil EFI_BROWSER_ACTION_RETRIEVE. \n"));
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

//[-start-180507-IB08400587-add]//
/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Request                A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 '&' before the first failing name/value pair (or
                                 the beginning of the string if the failure is in
                                 the first name/value pair) if the request was not
                                 successful.
  @param  Results                A null-terminated Unicode string in
                                 <ConfigAltResp> format which has all values filled
                                 in for the names in the Request string. String to
                                 be allocated by the called function.

  @retval EFI_SUCCESS            The Results is filled with the requested values.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.

**/
EFI_STATUS
EFIAPI
ExtractConfig2 (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  if (Progress == NULL || Results == NULL || Request == NULL) {    
    return EFI_INVALID_PARAMETER;
  }
  
  *Progress = Request;
  
  return EFI_NOT_FOUND;
}

/**
  This function processes the results of changes in configuration.

  @param  This                   Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param  Configuration          A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent '&' before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name/value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The Results is processed successfully.
  @retval EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval EFI_NOT_FOUND          Routing data doesn't match any storage in this
                                 driver.
**/
EFI_STATUS
EFIAPI
RouteConfig2 (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-180510-IB08400591-add]//
  *Progress = Configuration;
//[-end-180510-IB08400591-add]//

  return EFI_NOT_FOUND;
}
//[-end-180507-IB08400587-add]//

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
SetupUnload (
  IN EFI_HANDLE  ImageHandle
  )
{

  ASSERT (mPrivateData != NULL);
  
  if (mDebugMsgData != NULL) {
    FreePool (mDebugMsgData);
    mDebugMsgData = NULL;
  }

  if (mDriverHandle[0] != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle[0],
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess,
           NULL
           );
    mDriverHandle[0] = NULL;
  }

//[-start-180507-IB08400587-add]//
  if (mDriverHandle[1] != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle[1],
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess2,
           NULL
           );
    mDriverHandle[1] = NULL;
  }
//[-end-180507-IB08400587-add]//

  if (mPrivateData->HiiHandle[0] != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle[0]);
  }
  
  if (mPrivateData->HiiHandle[1] != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle[1]);
  }

  FreePool (mPrivateData);
  mPrivateData = NULL;

  return EFI_SUCCESS;
}

/**
  Initialization for the Setup related functions.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
DebugMessageConfigUtilDxeInitialize (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle[2];
  EFI_HII_DATABASE_PROTOCOL             *HiiDatabase;
  EFI_HII_STRING_PROTOCOL               *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       *HiiConfigRouting;
  CHAR16                                *NewString;
  UINTN                                 BufferSize;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION *Configuration;
  BOOLEAN                               ActionFlag;
  EFI_STRING                            ConfigRequestHdr;

  //
  // Initialize the local variables.
  //
  ConfigRequestHdr = NULL;
  NewString        = NULL;

  //
  // Initialize driver private data
  //
  mPrivateData = AllocateZeroPool (sizeof (DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = DEBUG_MESSAGE_CONFIG_UTIL_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig = RouteConfig;
  mPrivateData->ConfigAccess.Callback = DriverCallback;
//[-start-180507-IB08400587-add]//
  mPrivateData->ConfigAccess2.ExtractConfig = ExtractConfig2;
  mPrivateData->ConfigAccess2.RouteConfig = RouteConfig2;
  mPrivateData->ConfigAccess2.Callback = DriverCallback;
//[-end-180507-IB08400587-add]//

  //
  // Locate Hii Database protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiDatabaseProtocolGuid, NULL, (VOID **) &HiiDatabase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiDatabase = HiiDatabase;

  //
  // Locate HiiString protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiString = HiiString;

  //
  // Locate ConfigRouting protocol
  //
  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPrivateData->HiiConfigRouting = HiiConfigRouting;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mDriverHandle[0],
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath0,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  mPrivateData->DriverHandle[0] = mDriverHandle[0];

//[-start-171212-IB08400542-modify]//
//[-start-160317-IB08400337-modify]//
//[-start-180507-IB08400587-modify]//
  if (FeaturePcdGet (PcdH2OStoreDebugMsgPeiSupported) || FeaturePcdGet (PcdH2OStoreDebugMsgSupported)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mDriverHandle[1],
                    &gEfiDevicePathProtocolGuid,
                    &mHiiVendorDevicePath1,
                    &gEfiHiiConfigAccessProtocolGuid,
                    &mPrivateData->ConfigAccess2,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
    mPrivateData->DriverHandle[1] = mDriverHandle[1];
  }  
//[-end-180507-IB08400587-modify]//
//[-end-160317-IB08400337-modify]//
//[-end-171212-IB08400542-modify]//
  //
  // Publish our HII data
  //
  HiiHandle[0] = HiiAddPackages (
                   &gH2ODebugMessageConfigUtilConfigFormsetGuid,
                   mDriverHandle[0],
                   DebugMessageConfigUtilDxeStrings,
                   DebugMessageConfigUtilConfigPageVfrBin,
                   NULL
                   );
  if (HiiHandle[0] == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle[0] = HiiHandle[0];

  //
  // Publish our HII data
  //
//[-start-171212-IB08400542-modify]//
//[-start-160317-IB08400337-modify]//
  if (FeaturePcdGet (PcdH2OStoreDebugMsgPeiSupported) || FeaturePcdGet (PcdH2OStoreDebugMsgSupported)) {
    HiiHandle[1] = HiiAddPackages (
                   &gH2ODebugMessageConfigUtilEventAndMessageFormsetGuid,
                   mDriverHandle[1],
                   DebugMessageConfigUtilDxeStrings,
                   DebugMessageConfigUtilEventAndMessagePageVfrBin,
                   NULL
                   );
    if (HiiHandle[1] == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }  
//[-end-160317-IB08400337-modify]//
//[-end-171212-IB08400542-modify]//

  mPrivateData->HiiHandle[1] = HiiHandle[1];
  
  //
  // Initialize configuration data
  //
  Configuration = &mPrivateData->Configuration;
  ZeroMem (Configuration, sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION));

  //
  // Try to read NV config EFI variable first
  //
  ConfigRequestHdr = HiiConstructConfigHdr (&gH2ODebugMessageConfigUtilVarstoreGuid, mVariableName, mDriverHandle[0]);
  ASSERT (ConfigRequestHdr != NULL);
  if (ConfigRequestHdr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  DEBUG ((EFI_D_INFO, "ConfigRequestHdr: %s\n", ConfigRequestHdr));

  BufferSize = sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION);
  Status = gRT->GetVariable (mVariableName, &gH2ODebugMessageConfigUtilVarstoreGuid, NULL, &BufferSize, Configuration);
  if (EFI_ERROR (Status)) {
    //
    // Store zero data Buffer Storage to EFI variable
    //
    Status = CommonSetVariable (
               mVariableName,
               &gH2ODebugMessageConfigUtilVarstoreGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
               sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION),
               Configuration
               );
    if (EFI_ERROR (Status)) {
      SetupUnload (ImageHandle);
      return Status;
    }
    //
    // EFI variable for NV config doesn't exit, we should build this variable
    // based on default values stored in IFR
    //

    ActionFlag = HiiSetToDefaults (ConfigRequestHdr, EFI_HII_DEFAULT_CLASS_STANDARD);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // EFI variable does exist and Validate Current Setting
    //

    ActionFlag = HiiValidateSettings (ConfigRequestHdr);
    if (!ActionFlag) {
      SetupUnload (ImageHandle);
      return EFI_INVALID_PARAMETER;
    }
  }
  FreePool (ConfigRequestHdr);

  return EFI_SUCCESS;
}
