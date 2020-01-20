/** @file

  Initial and callback functions for Setup Change Config utility DXE

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

#include <SetupChangeConfigUtil.h>

EFI_HANDLE                                mDriverHandle = NULL;
SETUP_CHANGE_CONFIG_UTIL_PRIVATE_DATA     *mPrivateData = NULL;
UINTN                                     mCurrentListStartIndex;
UINTN                                     mCurrentListEndIndex;
SETUP_CHANGE_RECORD                       *mSetupChangeData = NULL;
UINTN                                     mSetupChangeDataNum = 0;
UINTN                                     mDaysOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
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
    H2O_SETUP_CHANGE_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID
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
EFI_STRING_ID                 mShowEventStringId[MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM] = {0};
EFI_STRING_ID                 mShowEventHelpStringId[MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM] = {0};
EFI_STRING_ID                 mTotalCountStringId = 0;
EFI_STRING_ID                 mCurrentCountStringId = 0;
//[-end-180213-IB08400569-add]//

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

/**
 Search first "Simple File System" protocol.

 @param[in]  Removable        Is device of the "Simple File System" removable

 @retval EFI_SIMPLE_FILE_SYSTEM_PROTOCOL   Find out "Simple File System" protocol.
         NULL                              Not found.
**/
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
    gBS->FreePool (HandleBuffer);
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
  FileNameSize = (StrLen(StorageName) + StrLen(L"20091111171800.log") + 1) * sizeof(CHAR16) ;
//[-end-160315-IB08400337-modify]// 

  Status = gBS->AllocatePool (EfiBootServicesData, FileNameSize, (VOID **)&TempString);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = gRT->GetTime (&EfiTime, NULL);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  ValueToString (EfiTime.Year, 4, Buffer, VTS_LEAD_0);
  StrCpy (TempString, Buffer);
  
  ValueToString (EfiTime.Month, 2, Buffer, VTS_LEAD_0);
  StrCat (TempString, Buffer);

  ValueToString (EfiTime.Day, 2, Buffer, VTS_LEAD_0);
  StrCat (TempString, Buffer);

  ValueToString (EfiTime.Hour, 2, Buffer, VTS_LEAD_0);
  StrCat (TempString, Buffer);

  ValueToString (EfiTime.Minute, 2, Buffer, VTS_LEAD_0);
  StrCat (TempString, Buffer);

  ValueToString (EfiTime.Second, 2, Buffer, VTS_LEAD_0);
  StrCat (TempString, Buffer);

  StrCat (TempString, StorageName);

  StrCat (TempString, L".log");

  *FileName = TempString;

  return EFI_SUCCESS;
}

/**
 This function verifies the leap year

 @param[in]         Year  - year in YYYY format                
 
 @retval BOOLEAN    TRUE  - The year is a leap year
                    FALSE - The year is not a leap year              
*/
BOOLEAN
EFIAPI
IsLeapYear (
  IN UINT16   Year
  )
{
  if (Year % 4 == 0) {
    if (Year % 100 == 0) {
      if (Year % 400 == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;
  }
}

/**
 Count the number of the leap years between 1970 and CurYear

 @param[in]         CurYear The Current year

 @retval UINTN      Count   The count of leapyears            
*/
UINTN
EFIAPI
CountNumOfLeapYears (
  IN UINT16           CurYear
  )
{
  UINT16  NumOfYear;
  UINT16  BaseYear;
  UINT16  Index;
  UINTN   Count;
  Count     = 0;
  BaseYear  = 1970;
  NumOfYear = (UINT16) (CurYear - 1970);
  for (Index = 0; Index <= NumOfYear; Index++) {
    if (IsLeapYear ((UINT16) (BaseYear + Index))) {
      Count += 1;
    }
  }

  return Count;
}

/**
 Count time stamp to Efi time.

 @param[in]         TimeStamp                
 @param[in]         EfiTime                          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
Ts2et (
  IN UINT32             TimeStamp,
  IN EFI_TIME           *EfiTime
  )
{
  UINT16                NumOfYears;
  UINTN                 NumOfLeapYears;
  UINTN                 NumOfDays;
  UINTN                 DayOfYear;
  UINT16                Years;
  UINT32                Mouths;

  // Caculate second;
  EfiTime->Second = (UINT8) (TimeStamp % 60);

  // Caculate minitunes;
  TimeStamp /= 60;
  EfiTime->Minute = (UINT8) (TimeStamp % 60);

  // Caculate Hour;
  TimeStamp /= 60;
  EfiTime->Hour = (UINT8) (TimeStamp % 24);

  NumOfDays = TimeStamp / 24;

  // caculate Year
  NumOfYears = (UINT16) (NumOfDays / 365);
  Years = 1970 + NumOfYears;

  if (Years != 1970) {
    NumOfLeapYears = CountNumOfLeapYears (Years-1);
  } else {
    NumOfLeapYears = 0;
  }

  NumOfDays -= NumOfLeapYears;

  // Year = 1970 + NumOfYear
  EfiTime->Year = (UINT16) (NumOfDays / 365 + 1970);

  DayOfYear = NumOfDays % 365 + 1;

  if (IsLeapYear(EfiTime->Year)) {
    mDaysOfMonth[1] = 29;
  }
  else {
    mDaysOfMonth[1] = 28;
  }

  for (Mouths = 0; DayOfYear > mDaysOfMonth[Mouths];  Mouths++) {
      DayOfYear -= mDaysOfMonth[Mouths];
  }

  EfiTime->Month = (UINT8) Mouths + 1;
  EfiTime->Day = (UINT8) DayOfYear;

  return EFI_SUCCESS;
}

/**
 Update the items for the H2O Setup Change History page in the Event Log Viewer.

 @param[in]  HiiHandle                 EFI HII handle.
 @param[in]  ShowAction                Determine the show policy when update the item of H2O Setup Change History page.    

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
UpdateH2OSetupChageHistoryPage (
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
  CHAR16                        *StringPtr2= NULL;
  UINTN                         Index;
  EFI_GUID                      FormSetGuid = H2O_SETUP_CHANGE_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID;
  EFI_TIME                      EfiTime; 
  EFI_STRING_ID                 TempStringId;
  UINTN                         EventStringIndex = 0;
//[-end-180214-IB08400569-modify]//

  if (ShowAction == SHOW_FIRST_PAGE) {
    if (mSetupChangeDataNum == 0) {
      // NO Setup Change messages.
      mCurrentListStartIndex = 0;
      mCurrentListEndIndex   = 0;
    } else {
      mCurrentListStartIndex = 1;
      if ((mCurrentListStartIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM - 1) <= mSetupChangeDataNum) {
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM - 1;
      } else {
        mCurrentListEndIndex = mSetupChangeDataNum;
      }
    }  
  } else {
    //
    // Show action: Show Previous or Next page of Events.
    //
    if (ShowAction == SHOW_PREVIOUS_PAGE) {
      if (mCurrentListStartIndex <= MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM) {
        //
        // No more previours events, skip.
        //
        return EFI_SUCCESS;
      } else {
        mCurrentListStartIndex = mCurrentListStartIndex - MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM;
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM - 1;
      }
    } else if (ShowAction == SHOW_NEXT_PAGE) {
      if (mCurrentListEndIndex == mSetupChangeDataNum) {
        //
        // No more next events, skip.
        //
        return EFI_SUCCESS;
      } 
      
      if ((mCurrentListEndIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM) <= mSetupChangeDataNum) {
        mCurrentListStartIndex = mCurrentListStartIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM;
        mCurrentListEndIndex = mCurrentListStartIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM - 1;
      } else {
        mCurrentListStartIndex = mCurrentListStartIndex + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM;
        mCurrentListEndIndex = mSetupChangeDataNum;
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
  StartLabel->Number       = H2O_SETUP_CHANGE_HEADER_START_LABEL; 

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = H2O_SETUP_CHANGE_HEADER_END_LABEL; 

  if ((mCurrentListStartIndex != 1) && (mCurrentListStartIndex != 0)) {
    HiiCreateActionOpCode (
    StartOpCodeHandle,                                 // Container for dynamic created opcodes
    (EFI_QUESTION_ID) KEY_SHOW_H2O_SETUP_CHANGE_PREVIOUS_PAGE, // Question ID
    STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_PREVIOUS_PAGE),// String ID for Prompt
    STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_PREVIOUS_PAGE),// String ID for Help
    EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
    0
    );
  }
  
  if (mCurrentListEndIndex != mSetupChangeDataNum) {
    HiiCreateActionOpCode (
    StartOpCodeHandle,                                 // Container for dynamic created opcodes
    (EFI_QUESTION_ID) KEY_SHOW_H2O_SETUP_CHANGE_NEXT_PAGE,// Question ID
    STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_NEXT_PAGE),    // String ID for Prompt
    STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_NEXT_PAGE),    // String ID for Help
    EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
    0
    );
  }

  //
  // For "H2O Setup Change History"
  //
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"- Total History Count   : %d",
    mSetupChangeDataNum
    );
//[-start-180214-IB08400569-modify]//
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
//[-end-180214-IB08400569-modify]//

//[-start-180214-IB08400569-modify]//
  UnicodeSPrint (
    StringPtr, 
    sizeof (StringPtr), 
    L"- Current History Number: %d to %d",
    mCurrentListStartIndex,
    mCurrentListEndIndex
    );
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
             (EFI_FORM_ID)VFR_FORMID_SETUP_CHANGE_SHOW_HISTORY,
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
  StartLabel->Number       = H2O_SETUP_CHANGE_EVENTS_START_LABEL; 

  EndLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = H2O_SETUP_CHANGE_EVENTS_END_LABEL; 

  if (mSetupChangeDataNum == 0) {
    //
    // Clear the Event List.
    //
    Status = HiiUpdateForm (
               HiiHandle,
               &FormSetGuid, 
               (EFI_FORM_ID)VFR_FORMID_SETUP_CHANGE_SHOW_HISTORY,
               StartOpCodeHandle,
               EndOpCodeHandle
               );
  } else {
//[-start-180214-IB08400569-modify]//
    // Print the Event List.
    for (Index = (mCurrentListStartIndex - 1), EventStringIndex = 0; Index < mCurrentListEndIndex; Index++, EventStringIndex++) {
      Ts2et(mSetupChangeData[Index].TimeStamp, &EfiTime);
      UnicodeSPrint (
        StringPtr, 
        sizeof (StringPtr), 
        L"[%02d] %d/%d/%d %02d:%02d:%02d ",
        (Index + 1),
        EfiTime.Year,
        EfiTime.Month,
        EfiTime.Day,
        EfiTime.Hour,
        EfiTime.Minute,
        EfiTime.Second
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
      
      StringPtr2 = AllocateZeroPool((mSetupChangeData[Index].ChangeRecordLen+1)*2);
      if (StringPtr2 == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      UnicodeSPrint (
        StringPtr2, 
        mSetupChangeData[Index].ChangeRecordLen * 2, 
        L"%s",
        mSetupChangeData[Index].ChangeRecord
        );

      if (mShowEventHelpStringId[EventStringIndex] == 0) {
        mShowEventHelpStringId[EventStringIndex] = HiiSetString (HiiHandle, 0, StringPtr2, NULL);
      } else {
        TempStringId = HiiSetString (HiiHandle, mShowEventHelpStringId[EventStringIndex], StringPtr2, NULL);
        if (TempStringId != mShowEventHelpStringId[EventStringIndex]) {
          mShowEventHelpStringId[EventStringIndex] = TempStringId;
          DEBUG ((EFI_D_ERROR, "String ID changed. New mShowEventHelpStringId[%x] = 0x%x\n", EventStringIndex, mShowEventHelpStringId[EventStringIndex]));      
        }
      }
      DEBUG ((EFI_D_ERROR, "mShowEventHelpStringId[%x]: 0x%x\n", EventStringIndex, mShowEventHelpStringId[EventStringIndex]));
      if (mShowEventHelpStringId[EventStringIndex] == 0) {
        DEBUG ((EFI_D_ERROR, "Cannot Set String ID. mShowEventHelpStringId[%x] = 0\n", EventStringIndex));
      }
      
      HiiCreateActionOpCode (
        StartOpCodeHandle,             // Container for dynamic created opcodes
        (EFI_QUESTION_ID) (ELV_SETUP_CHANGE_LIST_BASE + Index),    // Question ID
        mShowEventStringId[EventStringIndex],       // String ID for Prompt
        mShowEventHelpStringId[EventStringIndex],   // String ID for Help
        EFI_IFR_FLAG_CALLBACK,         // Flags in Question Header
        0
        );
      
    }
//[-end-180214-IB08400569-modify]//

    
    if (mCurrentListStartIndex != 1) {
      HiiCreateActionOpCode (
      StartOpCodeHandle,                                 // Container for dynamic created opcodes
      (EFI_QUESTION_ID) KEY_SHOW_H2O_SETUP_CHANGE_PREVIOUS_PAGE2, // Question ID
      STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_PREVIOUS_PAGE),// String ID for Prompt
      STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_PREVIOUS_PAGE),// String ID for Help
      EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
      0
      );
    }
    
    if (mCurrentListEndIndex != mSetupChangeDataNum) {
      HiiCreateActionOpCode (
      StartOpCodeHandle,                                 // Container for dynamic created opcodes
      (EFI_QUESTION_ID) KEY_SHOW_H2O_SETUP_CHANGE_NEXT_PAGE2,// Question ID
      STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_NEXT_PAGE),    // String ID for Prompt
      STRING_TOKEN(STR_SHOW_H2O_SETUP_CHANGE_HISTORY_NEXT_PAGE),    // String ID for Help
      EFI_IFR_FLAG_CALLBACK,                             // Flags in Question Header
      0
      );
    }
    
    Status = HiiUpdateForm (
               HiiHandle,
               &FormSetGuid, 
               (EFI_FORM_ID)VFR_FORMID_SETUP_CHANGE_SHOW_HISTORY,
               StartOpCodeHandle,
               EndOpCodeHandle
               );
    
  }

  FreePool (StringPtr2);
  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);
  
  return EFI_SUCCESS;
}

/**
 Show the Setup Change history on the H2O Setup Change historypage.

 @param[in]  HiiHandle                 EFI HII Handle.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ShowH2OSetupChangeHistory (
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  EFI_STATUS                            Status;
  CHAR16                                StringPtr2[MAX_ELV_STR_NUM] = {0};  
  EFI_INPUT_KEY                         Key;
  UINTN                                 Index;
  CHAR16                                *VariableName = H2O_SETUP_CHANGE_VARIABLE_NAME;
  H2O_SETUP_CHANGE_VARIABLE             *SetupChangeVar;
  UINTN                                 VariableDataSize;
  UINT8                                 *VariableData;
  UINTN                                 SetupChangeRecordCount;
  
  //
  // Check if Setup Change Variable is available.
  //
  Status = CommonGetVariableDataAndSize (
             VariableName, 
             &gH2OSetupChangeVariableGuid, 
             &VariableDataSize, 
             &VariableData
             );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      //ZeroMem (StringPtr2, sizeof(StringPtr2));
      UnicodeSPrint (
        StringPtr2, 
        sizeof (StringPtr2), 
        L"No Record."
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
    //ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Get Setup Change History: %r",
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
    return Status;
  } 

  //
  // First, try to get the total count of the Setup Change history
  //
  Index = 0;
  SetupChangeRecordCount = 0;
  while (Index <= (VariableDataSize -1)) {
    SetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *)((UINT8*)(VariableData) + Index);
    
    Index += SetupChangeVar->Size;
    SetupChangeRecordCount++;
  }

  //
  // Second, create a buffer to store the Setup Change history strings.
  //
  for (Index = 0; Index < mSetupChangeDataNum; Index++) {
    if (mSetupChangeData[Index].ChangeRecord != NULL) {
      gBS->FreePool  (mSetupChangeData[Index].ChangeRecord);
      mSetupChangeData[Index].ChangeRecord = NULL;
    }  
  }
  if (mSetupChangeData != NULL) {
    gBS->FreePool  (mSetupChangeData);
    mSetupChangeData = NULL;
  } 
  mSetupChangeData = (SETUP_CHANGE_RECORD *)AllocateZeroPool (sizeof(SETUP_CHANGE_RECORD) * SetupChangeRecordCount);
  if (mSetupChangeData == NULL) {
    mSetupChangeDataNum = 0;
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Can't get the Setup Change history."
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
  
  mSetupChangeDataNum = SetupChangeRecordCount;
  Index = 0;
  SetupChangeRecordCount = 0;
  
  while (Index <= (VariableDataSize -1)) {
    SetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *)((UINT8*)(VariableData) + Index);

    //
    // Store each H2O Setup Change History.
    //
    mSetupChangeData[SetupChangeRecordCount].TimeStamp = SetupChangeVar->TimeStamp;
    mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen = (SetupChangeVar->Size - sizeof(SetupChangeVar->TimeStamp) - sizeof(SetupChangeVar->Size));
    mSetupChangeData[SetupChangeRecordCount].ChangeRecord = AllocateZeroPool (mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen + 2);
    if (mSetupChangeData[SetupChangeRecordCount].ChangeRecord == NULL) {
      return EFI_OUT_OF_RESOURCES;
    } else {
      CopyMem (mSetupChangeData[SetupChangeRecordCount].ChangeRecord, SetupChangeVar->Data, mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen);
      mSetupChangeData[SetupChangeRecordCount].ChangeRecord[mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen + 2] = L'\0';
    }
    Index += SetupChangeVar->Size;
    SetupChangeRecordCount++;
  }
  
  Status = UpdateH2OSetupChageHistoryPage (HiiHandle, SHOW_FIRST_PAGE);
      
  return Status;
}

/**
 Show the detailed information of Setup Change History entry by using H2O Dialog.

 @param[in]  HiiHandle                 EFI HII Handle.
 @param[in]  ItemIndex                 Index of the selected Setup Change History entry.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
ShowSetupChangeDetail (
  IN  EFI_HII_HANDLE  HiiHandle,
  IN  UINT8           ItemIndex
  )
{
  EFI_STATUS                      Status;
  H2O_DIALOG_PROTOCOL             *H2ODialog;
  CHAR16                          *StringPtr = NULL;
  CHAR16                          *SetupChangeHistoryTitleString = NULL;
  CHAR16                          *SetupChangeHistoryInfoString = NULL;  
  UINTN                           SetupChangeHistoryInfoStringSize;  
  EFI_TIME                        EfiTime; 
  
  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **)&H2ODialog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  StringPtr = HiiGetString (
                HiiHandle,
                STRING_TOKEN (STR_H2O_SETUP_CHANGE_HISTORY_TITLE),
                NULL
                );
  SetupChangeHistoryTitleString = AllocateZeroPool(MAX_ELV_STR_NUM);
  if (SetupChangeHistoryTitleString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  UnicodeSPrint (
    SetupChangeHistoryTitleString, 
    MAX_ELV_STR_NUM, 
    L"%s ",
    StringPtr
    );
  FreePool (StringPtr);

  SetupChangeHistoryInfoStringSize = (mSetupChangeData[ItemIndex].ChangeRecordLen * 2) + MAX_ELV_STR_NUM;
  SetupChangeHistoryInfoString = AllocateZeroPool(SetupChangeHistoryInfoStringSize);
  if (SetupChangeHistoryInfoString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Ts2et(mSetupChangeData[ItemIndex].TimeStamp, &EfiTime);
  UnicodeSPrint (
    SetupChangeHistoryInfoString, 
    SetupChangeHistoryInfoStringSize, 
    L"Setup Change History information: \n%d/%d/%d %02d:%02d:%02d \n\n%s",
    EfiTime.Year,
    EfiTime.Month,
    EfiTime.Day,
    EfiTime.Hour,
    EfiTime.Minute,
    EfiTime.Second,
    mSetupChangeData[ItemIndex].ChangeRecord
    );
  
  Status = H2ODialog->ShowPageInfo (
                        SetupChangeHistoryTitleString,
                        SetupChangeHistoryInfoString
                        );

  FreePool (SetupChangeHistoryTitleString);
  FreePool (SetupChangeHistoryInfoString);
  
  return Status;
}


/**
 Save the Setup Change history on the H2O Setup Change historypage.

 @param[in]  HiiHandle                 EFI HII Handle.

 @retval EFI_SUCCESS                   Process successfully.
**/
EFI_STATUS
EFIAPI
SaveSetupChangeHistory (
  IN  EFI_HII_HANDLE  HiiHandle
  )
{
  EFI_STATUS                            Status;
  CHAR16                                StringPtr2[MAX_ELV_STR_NUM] = {0};  
  EFI_INPUT_KEY                         Key;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Volume = NULL;
  CHAR16                                *FileName = NULL;
  UINTN                                 Index;
  CHAR16                                *VariableName = H2O_SETUP_CHANGE_VARIABLE_NAME;
  H2O_SETUP_CHANGE_VARIABLE             *SetupChangeVar;
  UINTN                                 VariableDataSize;
  UINT8                                 *VariableData;
  UINTN                                 SetupChangeRecordCount;  
  VOID                                  *FinalString = NULL;
  VOID                                  *TempString = NULL;
  UINTN                                 TempStringSize = 0;  
  UINTN                                 TotalStringSize = 0;    
  EFI_TIME                              EfiTime;

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
  // 2. Use User-defined file name or Create a file name. ex: SetupChange_20150101171800.log
  //      
  ObtainSavingFileNameByTime (L"SetupChange", &FileName);  
  
  //
  // Check if Setup Change Variable is available.
  //
  Status = CommonGetVariableDataAndSize (
             VariableName, 
             &gH2OSetupChangeVariableGuid, 
             &VariableDataSize, 
             &VariableData
             );
  if (EFI_ERROR (Status)) {
    ZeroMem (StringPtr2, sizeof(StringPtr2));
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Get Setup Change History: %r",
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
  
  //
  // First, try to get the total count of the Setup Change history
  //
  Index = 0;
  SetupChangeRecordCount = 0;
  while (Index <= (VariableDataSize -1)) {
    SetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *)((UINT8*)(VariableData) + Index);
    
    Index += SetupChangeVar->Size;
    SetupChangeRecordCount++;
  }
  
  //
  // Second, create a buffer to store the Setup Change history strings.
  //
  mSetupChangeData = (SETUP_CHANGE_RECORD *)AllocateZeroPool (sizeof(SETUP_CHANGE_RECORD) * SetupChangeRecordCount);
  if (mSetupChangeData == NULL) {      
    DEBUG ((EFI_D_ERROR, "AllocateZeroPool() for mSetupChangeData failed.\n"));
    mSetupChangeDataNum = 0;
    UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Can't get the Setup Change history."
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
  
  mSetupChangeDataNum = SetupChangeRecordCount;

  Index = 0;
  SetupChangeRecordCount = 0;
  while (Index <= (VariableDataSize -1)) {
    SetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *)((UINT8*)(VariableData) + Index);

    //
    // Store each H2O Setup Change History.
    //
    mSetupChangeData[SetupChangeRecordCount].TimeStamp = SetupChangeVar->TimeStamp;
    mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen = (SetupChangeVar->Size - sizeof(SetupChangeVar->TimeStamp) - sizeof(SetupChangeVar->Size));
    mSetupChangeData[SetupChangeRecordCount].ChangeRecord = AllocateZeroPool (mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen + 2);
    if (mSetupChangeData[SetupChangeRecordCount].ChangeRecord == NULL) {
      return EFI_OUT_OF_RESOURCES;
    } else {
      CopyMem (mSetupChangeData[SetupChangeRecordCount].ChangeRecord, SetupChangeVar->Data, mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen);
      mSetupChangeData[SetupChangeRecordCount].ChangeRecord[mSetupChangeData[SetupChangeRecordCount].ChangeRecordLen + 2] = L'\0';
    }
    Index += SetupChangeVar->Size;
    SetupChangeRecordCount++;
  }
  
  FinalString = AllocateZeroPool (0x2000000);
  for (Index = 0; Index < mSetupChangeDataNum; Index++) {
    TempStringSize = StrLen (L"Setup Change History Num : DD  ") * 2;
    TempString = AllocateZeroPool (TempStringSize);
    UnicodeSPrint (
      TempString, 
      TempStringSize, 
      L"Setup Change History Num : %02d",
      Index + 1      
      );  
    StrCat (FinalString, TempString);
    FreePool (TempString);
    TotalStringSize += TempStringSize;     
    StrCat (FinalString, L"\r\n");
    
    Ts2et(mSetupChangeData[Index].TimeStamp, &EfiTime);
    TempStringSize = StrLen (L"2015/01/01 00:00:00  ") * 2;
    TempString = AllocateZeroPool (TempStringSize);
    UnicodeSPrint (
      TempString, 
      TempStringSize, 
      L"%02d/%02d/%02d %02d:%02d:%02d",
      EfiTime.Year,
      EfiTime.Month,
      EfiTime.Day,
      EfiTime.Hour,
      EfiTime.Minute,
      EfiTime.Second
      );  
    StrCat (FinalString, TempString);
    FreePool (TempString);
    TotalStringSize += TempStringSize;    
    StrCat (FinalString, L"\r\n");

    TempStringSize = (mSetupChangeData[Index].ChangeRecordLen + 1) * 2;
    TempString = AllocateZeroPool (TempStringSize);
    UnicodeSPrint (
      TempString, 
      TempStringSize, 
      L"%s",
      mSetupChangeData[Index].ChangeRecord
      );  
    StrCat (FinalString, TempString);
    FreePool (TempString);
    TotalStringSize += TempStringSize;    
    StrCat (FinalString, L"\r\n");
  }
    
  Status = CreateSavingFile (Volume, FileName, FinalString, TotalStringSize);
  ZeroMem (StringPtr2, sizeof(StringPtr2));
  UnicodeSPrint (
      StringPtr2, 
      sizeof (StringPtr2), 
      L"Save H2O Setup Change History to %s.",
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
  FreePool (FinalString);
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
//[-start-180507-IB08400587-modify]//
  if (Progress == NULL || Results == NULL || Request == NULL) {    
    return EFI_INVALID_PARAMETER;
  }
  
  *Progress = Request;
  
  return EFI_NOT_FOUND;
//[-end-180507-IB08400587-modify]//
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
//[-start-180507-IB08400587-modify]//
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-180510-IB08400591-add]//
  *Progress = Configuration;
//[-end-180510-IB08400591-add]//

  return EFI_NOT_FOUND;
//[-end-180507-IB08400587-modify]//
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

  if (((Value == NULL) && (Action != EFI_BROWSER_ACTION_FORM_OPEN) && (Action != EFI_BROWSER_ACTION_FORM_CLOSE))||
    (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_UNSUPPORTED;

  switch (Action) {
  case EFI_BROWSER_ACTION_FORM_OPEN:
    DEBUG ((EFI_D_INFO, "SetupChangeConfigUtil EFI_BROWSER_ACTION_FORM_OPEN. \n"));
    break;

  case EFI_BROWSER_ACTION_CHANGING:
    DEBUG ((EFI_D_INFO, "SetupChangeConfigUtil EFI_BROWSER_ACTION_CHANGING. \n"));
    {
      switch (QuestionId) {      
      //
      // For H2O Setup Change history
      //
      case KEY_ELV_SHOW_H2O_SETUP_CHANGE:
        DEBUG ((EFI_D_INFO, "KEY_ELV_SHOW_H2O_SETUP_CHANGE!!\n"));
        Status = ShowH2OSetupChangeHistory (mPrivateData->HiiHandle);
        break;
        
      case KEY_ELV_SAVE_H2O_SETUP_CHANGE:
        DEBUG ((EFI_D_INFO, "KEY_ELV_SAVE_H2O_SETUP_CHANGE!!\n"));
        Status = SaveSetupChangeHistory (mPrivateData->HiiHandle); 
        break;   
        
      case KEY_SHOW_H2O_SETUP_CHANGE_PREVIOUS_PAGE:
      case KEY_SHOW_H2O_SETUP_CHANGE_PREVIOUS_PAGE2:  
        Status = UpdateH2OSetupChageHistoryPage (mPrivateData->HiiHandle, SHOW_PREVIOUS_PAGE);
        break; 

      case KEY_SHOW_H2O_SETUP_CHANGE_NEXT_PAGE:
      case KEY_SHOW_H2O_SETUP_CHANGE_NEXT_PAGE2:  
        Status = UpdateH2OSetupChageHistoryPage (mPrivateData->HiiHandle, SHOW_NEXT_PAGE);
        break; 
        
      default:
        if ((QuestionId >= ELV_SETUP_CHANGE_LIST_BASE) && (QuestionId < ELV_SETUP_CHANGE_LIST_BASE + MAX_SHOW_ELV_SETUP_CHANGE_LIST_NUM)) {
          Status = ShowSetupChangeDetail (mPrivateData->HiiHandle,(UINT8) (QuestionId - ELV_SETUP_CHANGE_LIST_BASE));
          break;
        }
        break;
      }
    }
    break;

  case EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING:
    DEBUG ((EFI_D_INFO, "SetupChangeConfigUtil EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING. \n"));
    break;

  case EFI_BROWSER_ACTION_DEFAULT_STANDARD:
    DEBUG ((EFI_D_INFO, "SetupChangeConfigUtil EFI_BROWSER_ACTION_DEFAULT_STANDARD. \n"));
    break;

  case EFI_BROWSER_ACTION_RETRIEVE:
    DEBUG ((EFI_D_INFO, "SetupChangeConfigUtil EFI_BROWSER_ACTION_RETRIEVE. \n"));
    break;

  default:
    Status = EFI_UNSUPPORTED;
    break;
  }

  return Status;
}

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

  if (mDriverHandle != NULL) {
    gBS->UninstallMultipleProtocolInterfaces (
           mDriverHandle,
           &gEfiDevicePathProtocolGuid,
           &mHiiVendorDevicePath0,
           &gEfiHiiConfigAccessProtocolGuid,
           &mPrivateData->ConfigAccess,
           NULL
           );
    mDriverHandle = NULL;
  }

  if (mPrivateData->HiiHandle != NULL) {
    HiiRemovePackages (mPrivateData->HiiHandle);
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
SetupChangeConfigUtilDxeInitialize (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HII_HANDLE                        HiiHandle;
  EFI_HII_DATABASE_PROTOCOL             *HiiDatabase;
  EFI_HII_STRING_PROTOCOL               *HiiString;
  EFI_HII_CONFIG_ROUTING_PROTOCOL       *HiiConfigRouting;

  //
  // Initialize driver private data
  //
  mPrivateData = AllocateZeroPool (sizeof (SETUP_CHANGE_CONFIG_UTIL_PRIVATE_DATA));
  if (mPrivateData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->Signature = SETUP_CHANGE_CONFIG_UTIL_PRIVATE_SIGNATURE;

  mPrivateData->ConfigAccess.ExtractConfig = ExtractConfig;
  mPrivateData->ConfigAccess.RouteConfig = RouteConfig;
  mPrivateData->ConfigAccess.Callback = DriverCallback;

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
                  &mDriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mHiiVendorDevicePath0,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &mPrivateData->ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  mPrivateData->DriverHandle = mDriverHandle;
  
  //
  // Publish our HII data
  //
  HiiHandle = HiiAddPackages (
                &gH2OSetupChangeConfigUtilEventAndMessageFormsetGuid,
                mDriverHandle,
                SetupChangeConfigUtilDxeStrings,
                SetupChangeConfigUtilEventAndMessagePageVfrBin,
                NULL
                );
  if (HiiHandle == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPrivateData->HiiHandle = HiiHandle;

  return EFI_SUCCESS;
}
