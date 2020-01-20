/** @file

  H2O Setup Change DXE implementation.

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <H2OSetupChange.h>

H2O_SUBMIT_SVC_PROTOCOL                gH2OSubmitSvcProtocol = {
                                         ExecuteSubmitSvc
                                         };

CHAR16                                    *mFullDiagTitleString = L"History is full, please set action:";
CHAR16                                    *mFullDiagActionList[SETUP_CHANGE_MAX_OPTION] = {
                                              {L"Stop the history"},
                                              {L"Clear all history"},
                                              {L"Overwrite older history"}
                                              };
EFI_HII_DATE                           mOrgDate = {0};
EFI_HII_DATE                           mModDate = {0};
CHAR16                                 *mDatePromptStr = NULL;

/**
 Add the change information of Date into mQuestionChangedList.

 @param[in]  *ChangedQuestionList       Pointer to the changed questions.

 @retval EFI_SUCCESS                    Add the change information successfully.
 @retval EFI_OUT_OF_RESOURCES           Add failure when allocating pool.

*/
EFI_STATUS
EFIAPI
CheckDateChangeInfo (
  IN  LIST_ENTRY         *ChangedQuestionList
  )
{
  FORM_BROWSER_STATEMENT_CHANGED     *Question;
  CHAR16                             *TempSettingStr;

  if (mOrgDate.Year == 0 || mOrgDate.Month == 0 || mOrgDate.Day== 0) {
    // The Original date is not initialized.
    DEBUG ((EFI_D_ERROR, "No Setup Date changed.\n"));
    return EFI_SUCCESS;
  }

  Question = AllocateZeroPool (sizeof (FORM_BROWSER_STATEMENT_CHANGED));
  if (Question == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Question->Signature = FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE;
  TempSettingStr = CatSPrint (
                     NULL,
                     L"%s: [%d/%d/%d] to [%d/%d/%d] \n",
                     mDatePromptStr,
                     mOrgDate.Year, mOrgDate.Month, mOrgDate.Day,
                     mModDate.Year, mModDate.Month, mModDate.Day
                     );
  DEBUG ((EFI_D_ERROR, "Setup Date changed infor: %s\n", TempSettingStr));

  Question->ChangedInfoStr = TempSettingStr;
  InsertTailList (ChangedQuestionList, &Question->Link);
  return EFI_SUCCESS;
}

/**
 This function verifies the leap year.

 @param[in]         Year                Year in YYYY format.

 @retval TRUE                           The year is a leap year.
 @retval FALSE                          The year is not a leap year.
*/
BOOLEAN
IsLeapYear (
  IN  UINT16                            Year
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
 Converts EFI_TIME structure to a TimeStamp.

 @param[in]         Time                EFI_TIME structure to be converted.
 @param[out]        TimeStamp           TimeStamp converted from EFI_TIME structure.
*/
VOID
EfiTimeToTimeStamp (
  IN  EFI_TIME                          *Time,
  OUT UINT32                            *TimeStamp
  )
{
  UINT16                                Year;
  UINT16                                AddedDays;
  UINT8                                 Month;
  UINT32                                DaysOfMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  //
  // Find number of leap years
  //
  AddedDays = 0;
  for (Year = BASE_YEAR; Year < Time->Year; ++Year) {
    if (IsLeapYear (Year)) {
      ++AddedDays;
    }
  }

  //
  // Number of days of complete years (include all leap years)
  //
  *TimeStamp = (Time->Year - BASE_YEAR) * DAYS_PER_YEAR;
  *TimeStamp += AddedDays;

  //
  // Number of days from 1970/1/1 to now
  //
  for (Month = 0; Month < Time->Month - BASE_MONTH; ++Month) {
    *TimeStamp += DaysOfMonth[Month];
  }
  *TimeStamp += Time->Day - BASE_DAY;

  //
  // Check this Feb. is 28 days or 29 days
  //
  if (IsLeapYear (Time->Year) && Time->Month > 2) {
    *TimeStamp += 1;
  }

  //
  // Convert days to seconds
  //
  *TimeStamp *= SECONDS_PER_DAY;

  //
  // Add rest seconds
  //
  *TimeStamp += (Time->Hour * SECONDS_PER_HOUR) +
                (Time->Minute * SECONDS_PER_MINUTE) +
                Time->Second;

}

/**
 Store all of the showing string to specific storage or EFI Variable.

 @param[out]  This               A pointer to the showing string.

 @retval EFI_SUCCESS             The showing string has been stored.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
StoreInformation (
  IN CHAR16             *StringBuf
  )
{
  EFI_STATUS                                Status;
  CHAR16                                    *VariableName = H2O_SETUP_CHANGE_VARIABLE_NAME;
  UINTN                                     OrgVariableDataSize;
  UINT8                                     *OrgVariableData = NULL;
  VOID                                      *SetupChangeVarPool = NULL;
  H2O_SETUP_CHANGE_VARIABLE                 *SetupChangeVar;
  H2O_SETUP_CHANGE_VARIABLE                 *OrgSetupChangeVar;
  UINTN                                     SetupChangeVarDataSize = 0;
  UINT32                                    TimeStamp;
  EFI_TIME                                  Time;
  CHAR16                                    *NewUnicodeStr;
  UINTN                                     NewUnicodeStrSize;
  UINT8                                     AdjustPolicy;
  H2O_DIALOG_PROTOCOL                       *H2ODialog;
  UINT32                                    SelectedIndex;
  EFI_INPUT_KEY                             Key;
  UINTN                                     NeedVariableSize;
  UINTN                                     CountVariableSize = 0;
  UINT8                                     *CopyAddr;

  //
  // Calculate new data size and check if it is over the max Setup Change variable size.
  //
  NewUnicodeStrSize = StrLen (StringBuf) * 2;
  NewUnicodeStr = StringBuf;
  NeedVariableSize = sizeof(SetupChangeVar->TimeStamp) + sizeof(SetupChangeVar->Size) + NewUnicodeStrSize;
  if (NeedVariableSize > PcdGet32(PcdMaxSetupChangeVariableSize)) {
    DEBUG ((EFI_D_ERROR, "Needed Setup Change information size is over the max Setup Change Variable size.\n"));
    DEBUG ((EFI_D_ERROR, "Needed Setup Change information size: 0x%x\n", NeedVariableSize));
    DEBUG ((EFI_D_ERROR, "PcdMaxSetupChangeVariableSize: 0x%x\n", PcdGet32(PcdMaxSetupChangeVariableSize)));
    return EFI_OUT_OF_RESOURCES;
  }

  OrgVariableDataSize = 0;
  Status = CommonGetVariableDataAndSize (
             VariableName,
             &gH2OSetupChangeVariableGuid,
             &OrgVariableDataSize,
             &OrgVariableData
             );
  if (!EFI_ERROR (Status)) {
    //
    // Find old variable data, and update it with new data..
    //

    //
    // Calculate the total size, and check if it exceeds the maximum size.
    //
    SetupChangeVarDataSize = NeedVariableSize + OrgVariableDataSize;
    if (SetupChangeVarDataSize >= PcdGet32(PcdMaxSetupChangeVariableSize)) {
      DEBUG ((EFI_D_INFO, "Total Setup Change information size is over the max Setup Change Variable size..\n"));

      if (PcdGet8 (PcdSetupChangeFullLogAdjustPolicy) == SETUP_CHANGE_DISPLAY_USE_DIALOG) {
        //
        // Get option for the policy: Overwrite, Clear, Do nothing.
        //
        Status = gBS->LocateProtocol (&gH2ODialogProtocolGuid, NULL, (VOID **) &H2ODialog);
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_INFO, "Locate H2ODialog Protocol: %r \n", Status));
          FreePool (OrgVariableData);
          return Status;
        }
        H2ODialog->OneOfOptionDialog (
                     (UINT32)SETUP_CHANGE_MAX_OPTION,
                     FALSE,
                     NULL,
                     &Key,
                     SETUP_CHANGE_MAX_OPTION_STRING_SIZE,
                     mFullDiagTitleString,
                     &SelectedIndex,
                     (CHAR16 **) (mFullDiagActionList),
                     0
                     );
        if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
          AdjustPolicy = (UINT8)SelectedIndex;
        } else {
          //
          // User doesn't select any action for over size, don't do anything and returen.
          //
          DEBUG ((EFI_D_INFO, "Total size is over the max Setup Change variable size, and keep the old data content\n"));
          FreePool (OrgVariableData);
          return EFI_SUCCESS;
        }
      } else {
        AdjustPolicy = (UINT8)PcdGet8 (PcdSetupChangeFullLogAdjustPolicy);
      }

      switch (AdjustPolicy) {
      case SETUP_CHANGE_DISPLAY_DO_NOTHING:
        FreePool (OrgVariableData);
        return EFI_SUCCESS;
        break;

      case SETUP_CHANGE_DISPLAY_CLEAR:
        //
        // Clear all old data, and only will store the new data.
        //
        SetupChangeVarDataSize -= OrgVariableDataSize;
        OrgVariableDataSize = 0;
        break;

      case SETUP_CHANGE_DISPLAY_OVERWRITE:
        NeedVariableSize = sizeof(SetupChangeVar->TimeStamp) + sizeof(SetupChangeVar->Size) + NewUnicodeStrSize;
        CountVariableSize = 0;
        OrgSetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *) OrgVariableData;
        while (CountVariableSize < OrgVariableDataSize) {
          if (OrgSetupChangeVar->Size == 0) {
            //
            // Variable may be destroyed, skip all old data and break.
            //
            CountVariableSize = 0;
            break;
          }
          CountVariableSize += OrgSetupChangeVar->Size;
          //
          // Count the suitable size to get enough space to store new data.
          // The oldest data will be skipped.
          //
          if ((PcdGet32(PcdMaxSetupChangeVariableSize) - CountVariableSize) < NeedVariableSize) {
            CountVariableSize -= OrgSetupChangeVar->Size;
            break;
          }
          OrgSetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *) (OrgVariableData + CountVariableSize);
        }

        //
        // Final data content: (Need data) + (part of old data base on OrgVariableDataSize)
        //
        SetupChangeVarDataSize = NeedVariableSize + CountVariableSize;
        OrgVariableDataSize = CountVariableSize;
        break;

      default:
        break;
      }
    }
  } else {
    //
    // Can't find old data from Setup Change Variable, just create new one with new data.
    //
    SetupChangeVarDataSize = NeedVariableSize;
  }

  if (SetupChangeVarDataSize == 0) {
    if (OrgVariableData != NULL) {
      FreePool (OrgVariableData);
    }
    return EFI_SUCCESS;
  }

  SetupChangeVarPool = AllocateZeroPool (SetupChangeVarDataSize);
  if (SetupChangeVarPool == NULL) {
    DEBUG ((EFI_D_INFO, "AllocateZeroPool(): %r \n", Status));
    if (OrgVariableData != NULL) {
      FreePool (OrgVariableData);
    }
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "EfiGetTime(): %r \n", Status));
    if (OrgVariableData != NULL) {
      FreePool (OrgVariableData);
    }
    FreePool (SetupChangeVarPool);
    return EFI_OUT_OF_RESOURCES;
  } else {
    EfiTimeToTimeStamp (&Time, &TimeStamp);
    DEBUG ((EFI_D_INFO, "TimeStamp: %x \n", TimeStamp));
  }

  SetupChangeVar = (H2O_SETUP_CHANGE_VARIABLE *)SetupChangeVarPool;
  SetupChangeVar->TimeStamp = TimeStamp;
  //
  // Size value will base on the newest data, because each copy of data will have its own size value.
  //
  SetupChangeVar->Size = (UINT16) (sizeof(SetupChangeVar->TimeStamp) +
                                   sizeof(SetupChangeVar->Size) +
                                   NewUnicodeStrSize
                                   );
  CopyMem (&SetupChangeVar->Data, NewUnicodeStr, NewUnicodeStrSize);
  if (OrgVariableDataSize != 0) {
    CopyAddr = (UINT8*) &(SetupChangeVar->Data);
    CopyAddr += NewUnicodeStrSize;
    CopyMem (CopyAddr, OrgVariableData, OrgVariableDataSize);
    FreePool (OrgVariableData);
  }

  Status = CommonSetVariable (
             VariableName,
             &gH2OSetupChangeVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             SetupChangeVarDataSize,
             SetupChangeVar
             );
  DEBUG ((EFI_D_INFO, "CommonSetVariable() Setup Change Variable: %r \n", Status));
  FreePool (SetupChangeVarPool);

  return Status;

}

/**
 Contruct the showing string for the specified question data.

 @param[in]   This               A pointer to the specified question data.
 @param[out]  This               A pointer to the showing string.

 @retval EFI_SUCCESS             The showing string has been contructed.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
ContructString (
  IN  H2O_FORM_BROWSER_Q *QuestionData,
  OUT CHAR16             **StringBuf
  )
{
  EFI_STATUS                                Status;
  H2O_FORM_BROWSER_PROTOCOL                 *FBProtocol;
  UINT32                                    Index;
  UINTN                                     VarStoreCount;
  EFI_VARSTORE_ID                           *VarStoreIdBuffer;
  H2O_FORM_BROWSER_VS                       *VarbleStoreData;
  UINT32                                    OptionIndex;
  UINT64                                    OrgValue;
  UINT64                                    ModValue;
  CHAR16                                    *OrgStr;
  CHAR16                                    *ModStr;
  CHAR16                                    *OptionStr;
  UINTN                                     StringSize;
  UINT8                                     Width;
  EFI_HII_TIME                              *OrgTime;
  EFI_HII_TIME                              *ModTime;

  ASSERT (StringBuf != NULL);
  if ((QuestionData->Prompt == NULL) || (StrLen(QuestionData->Prompt) == 0)) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = FBProtocol->GetVSAll (FBProtocol, QuestionData->PageId, &VarStoreCount, &VarStoreIdBuffer);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  VarbleStoreData = NULL;
  for (Index = 0; Index < VarStoreCount; Index++) {
    if (VarStoreIdBuffer[Index] == QuestionData->VarStoreId) {
      Status = FBProtocol->GetVSInfo (
                             FBProtocol,
                             QuestionData->PageId,
                             QuestionData->VarStoreId,
                             &VarbleStoreData
                             );
      break;
    }
  }
  if (Index == VarStoreCount || EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  *StringBuf = NULL;

  switch (QuestionData->Operand) {

  case EFI_IFR_ONE_OF_OP:
    OrgStr   = NULL;
    ModStr   = NULL;
    OrgValue = VarbleStoreData->Buffer[QuestionData->VariableOffset];
    ModValue = VarbleStoreData->EditBuffer[QuestionData->VariableOffset];

    for (Index = 0; Index < QuestionData->NumberOfOptions; Index++) {
      if (QuestionData->Options[Index].HiiValue.Value.u8 == OrgValue) {
        OrgStr = QuestionData->Options[Index].Text;
      }

      if (QuestionData->Options[Index].HiiValue.Value.u8 == ModValue) {
        ModStr = QuestionData->Options[Index].Text;
      }

      if (OrgStr != NULL && ModStr != NULL) {
        *StringBuf = CatSPrint (NULL, L"%s: [%s] to [%s]\n", QuestionData->Prompt, OrgStr, ModStr);
        break;
      }
    }
    break;

  case EFI_IFR_NUMERIC_OP:
    switch (QuestionData->Flags & EFI_IFR_NUMERIC_SIZE) {

    case EFI_IFR_NUMERIC_SIZE_1:
      OrgValue = (UINT64) (*((UINT8 *) (VarbleStoreData->Buffer + QuestionData->VariableOffset)));
      ModValue = (UINT64) QuestionData->HiiValue.Value.u8;
      break;

    case EFI_IFR_NUMERIC_SIZE_2:
      OrgValue = (UINT64) (*((UINT16 *) (VarbleStoreData->Buffer + QuestionData->VariableOffset)));
      ModValue = (UINT64) QuestionData->HiiValue.Value.u16;
      break;

    case EFI_IFR_NUMERIC_SIZE_4:
      OrgValue = (UINT64) (*((UINT32 *) (VarbleStoreData->Buffer + QuestionData->VariableOffset)));
      ModValue = (UINT64) QuestionData->HiiValue.Value.u32;
      break;

    case EFI_IFR_NUMERIC_SIZE_8:
      OrgValue = *((UINT64 *) (VarbleStoreData->Buffer + QuestionData->VariableOffset));
      ModValue = QuestionData->HiiValue.Value.u64;
      break;

    default:
      OrgValue = 0;
      ModValue = 0;
      break;
    }

    if (OrgValue != ModValue) {
      *StringBuf = CatSPrint (
                     NULL,
                     (QuestionData->Flags & EFI_IFR_DISPLAY_UINT_HEX) ? L"%s: [0x%x] to [0x%x]\n" : L"%s: [%d] to [%d]\n",
                     QuestionData->Prompt,
                     OrgValue,
                     ModValue
                     );
    }
    break;

  case EFI_IFR_ORDERED_LIST_OP:
    if (QuestionData->NumberOfOptions < 2) {
      //
      // Not available number of options,
	  // At least need 2 options in ordered list.
      //
      return EFI_UNSUPPORTED;
    }
    // Use Option's value type to determine the width as the behavior of IfrParse.c.
    Width = 1;
    switch (QuestionData->Options[0].HiiValue.Type) {

    case EFI_IFR_TYPE_NUM_SIZE_8:
      Width = 1;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_16:
      Width = 2;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_32:
      Width = 4;
      break;

    case EFI_IFR_TYPE_NUM_SIZE_64:
      Width = 8;
      break;

    default:
      //
      // Invalid type for Ordered List
      //
      break;
    }

    for (Index = 0, StringSize = 0; Index < QuestionData->NumberOfOptions; Index++) {
      StringSize += (StrSize (QuestionData->Options[Index].Text) + sizeof (L"<> "));
    }
    OrgStr    = AllocateZeroPool (StringSize);
    ModStr    = AllocateZeroPool (StringSize);
    OptionStr = AllocateZeroPool (StringSize);
    if (OrgStr == NULL || ModStr == NULL || OptionStr == NULL) {
      break;
    }

    for (Index = 0, OrgValue = 0, ModValue = 0; Index < QuestionData->ContainerCount; Index++) {
      CopyMem (&OrgValue, VarbleStoreData->Buffer     + QuestionData->VariableOffset + Index * Width, Width);
      CopyMem (&ModValue, VarbleStoreData->EditBuffer + QuestionData->VariableOffset + Index * Width, Width);

      for (OptionIndex = 0; OptionIndex < QuestionData->NumberOfOptions; OptionIndex++) {
        if (CompareMem (&QuestionData->Options[OptionIndex].HiiValue.Value.u64, &OrgValue, Width) ==  0) {
          UnicodeSPrint (OptionStr, StringSize, L"<%s> ", QuestionData->Options[OptionIndex].Text);
          StrCat (OrgStr, OptionStr);
        }
        if (CompareMem (&QuestionData->Options[OptionIndex].HiiValue.Value.u64, &ModValue, Width) ==  0) {
          UnicodeSPrint (OptionStr, StringSize, L"<%s> ", QuestionData->Options[OptionIndex].Text);
          StrCat (ModStr, OptionStr);
        }
      }
    }

    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%s] to [%s]\n",
                   QuestionData->Prompt,
                   OrgStr,
                   ModStr
                   );
    FreePool (OrgStr);
    FreePool (ModStr);
    FreePool (OptionStr);
    break;

  case EFI_IFR_CHECKBOX_OP:
    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%s] to [%s]\n",
                   QuestionData->Prompt,
                   (VarbleStoreData->Buffer[QuestionData->VariableOffset]     == 0) ? L"Disabled" : L"Enabled",
                   (VarbleStoreData->EditBuffer[QuestionData->VariableOffset] == 0) ? L"Disabled" : L"Enabled"
                   );
    break;

  case EFI_IFR_STRING_OP:
    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%s] to [%s]\n",
                   QuestionData->Prompt,
                   (CHAR16 *) &VarbleStoreData->Buffer[QuestionData->VariableOffset],
                   QuestionData->HiiValue.Buffer
                   );
    break;

  case EFI_IFR_TIME_OP:
    OrgTime = (EFI_HII_TIME *)&VarbleStoreData->Buffer[QuestionData->VariableOffset];
    ModTime = (EFI_HII_TIME *)&QuestionData->HiiValue.Value.time;
    *StringBuf = CatSPrint (
                   NULL,
                   L"%s: [%d:%d:%d] to [%d:%d:%d]\n",
                   QuestionData->Prompt,
                   OrgTime->Hour, OrgTime->Minute, OrgTime->Second,
                   ModTime->Hour, ModTime->Minute, ModTime->Second
                   );
    break;

  default:
    break;
  }

  if (*StringBuf == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**
 Remove the all question nodes from mQuestionChangedList.

 @param[in]  None.

 @retval EFI_SUCCESS         Remove the node successfully.
*/
EFI_STATUS
EFIAPI
RemoveQuestionChangedList (
  IN  LIST_ENTRY         *ChangedQuestionList
  )
{
  LIST_ENTRY                         *Link;
  FORM_BROWSER_STATEMENT_CHANGED     *QuestionNode;

  Link = GetFirstNode (ChangedQuestionList);
  while (!IsNull (ChangedQuestionList, Link)) {
    QuestionNode = FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK (Link);
    Link = GetNextNode (ChangedQuestionList, Link);

    RemoveEntryList (&QuestionNode->Link);
    FreePool (QuestionNode->ChangedInfoStr);
    FreePool (QuestionNode);
  }

  return EFI_SUCCESS;

}

/**
 Add the question node into mQuestionChangedList.

 @param[in]  QuestionData               A pointer to the specified question.

 @retval EFI_SUCCESS                    Add the question node successfully.
 @retval EFI_OUT_OF_RESOURCES           Add failure when allocating pool.

*/
EFI_STATUS
EFIAPI
AddToChangedList (
  IN  LIST_ENTRY         *ChangedQuestionList,
  IN  H2O_FORM_BROWSER_Q *QuestionData
  )
{
  FORM_BROWSER_STATEMENT_CHANGED     *Question;
  CHAR16                             *TempStringPtr;
  EFI_STATUS                         Status;

  Status = ContructString (QuestionData, &TempStringPtr);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Question = AllocateZeroPool (sizeof (FORM_BROWSER_STATEMENT_CHANGED));
  if (Question == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Question->Signature = FORM_BROWSER_STATEMENT_CHANGED_SIGNATURE;
  CopyMem (&(Question->Question), QuestionData, sizeof(H2O_FORM_BROWSER_Q));

  Question->ChangedInfoStr = TempStringPtr;
  InsertTailList (ChangedQuestionList, &Question->Link);

  return EFI_SUCCESS;
}

/**
 Execute the Submit service function.

 @param[in]  This                A pointer to the H2O_SUBMIT_SVC_PROTOCOL instance.
 @param[out] Request             A pointer to the request from the Submit service functions.
                                 Related definition can refer to "Browser actions" of FormBrowserEx.h.
 @param[out] ShowSubmitDialog    A pointer to the value if needing to show the original submit dialog.

 @retval EFI_SUCCESS             Execute the Submit service functions successfully.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
EFIAPI
ExecuteSubmitSvc (
  IN    H2O_SUBMIT_SVC_PROTOCOL             *This,
  OUT   UINT32                              *Request,
  OUT   BOOLEAN                             *ShowSubmitDialog
  )
{
  EFI_STATUS                         Status;
  LIST_ENTRY                         *Link;
  FORM_BROWSER_STATEMENT_CHANGED     *QuestionNode;
  CHAR16                             *TitleString    = L"BIOS Changed Setting:";
  CHAR16                             *ConfirmString  = L"Save the changes?";
  CHAR16                             *ConfirmString2 = L"Exit the Setup?";
  CHAR16                             *NoChangeString = L"There is no changed item.";
  CHAR16                             *StringPtr;
  H2O_DIALOG_PROTOCOL                *H2ODialog;
  H2O_FORM_BROWSER_PROTOCOL          *FBProtocol;
  UINT32                             ChangedQuestionCount;
  H2O_FORM_BROWSER_Q                 *ChangedQuestionBuffer;
  UINT32                             QuestionIndex;
  UINTN                              StringLength;
  EFI_INPUT_KEY                      Key;
  LIST_ENTRY                         ChangedQuestionList;

  Status = gBS->LocateProtocol (&gH2ODialogProtocolGuid, NULL, (VOID **) &H2ODialog);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gH2OFormBrowserProtocolGuid, NULL, (VOID **) &FBProtocol);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = FBProtocol->GetChangedQuestions (FBProtocol, &ChangedQuestionCount, &ChangedQuestionBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InitializeListHead (&ChangedQuestionList);

  for (QuestionIndex = 0; QuestionIndex < ChangedQuestionCount; QuestionIndex++) {
    AddToChangedList (&ChangedQuestionList, &ChangedQuestionBuffer[QuestionIndex]);
  }

  CheckDateChangeInfo (&ChangedQuestionList);

  if (IsListEmpty (&ChangedQuestionList)) {
    Status = H2ODialog->ConfirmPageDialog (
                          DlgYesNo,
                          TitleString,
                          ConfirmString2,
                          NoChangeString,
                          &Key
                          );
    if (EFI_ERROR (Status) || Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      *Request = BROWSER_ACTION_NONE;
    } else {
      *Request = BROWSER_ACTION_SUBMIT | BROWSER_ACTION_RESET;
    }
  } else {
    StringLength = 0;
    Link = GetFirstNode (&ChangedQuestionList);
    while (!IsNull (&ChangedQuestionList, Link)) {
      QuestionNode = FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK (Link);
      Link         = GetNextNode (&ChangedQuestionList, Link);
      StringLength += StrLen(QuestionNode->ChangedInfoStr);
    }

    StringPtr = AllocateZeroPool (sizeof (CHAR16) * (StringLength + 1));
    if (StringPtr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Link = GetFirstNode (&ChangedQuestionList);
    while (!IsNull (&ChangedQuestionList, Link)) {
      QuestionNode = FORM_BROWSER_STATEMENT_CHANGED_FROM_LINK (Link);
      Link         = GetNextNode (&ChangedQuestionList, Link);
      StrCat (StringPtr, QuestionNode->ChangedInfoStr);
    }

    Status = H2ODialog->ConfirmPageDialog (
                          DlgYesNo,
                          TitleString,
                          ConfirmString,
                          StringPtr,
                          &Key
                          );
    if (EFI_ERROR (Status) || Key.UnicodeChar != CHAR_CARRIAGE_RETURN) {
      *Request = BROWSER_ACTION_NONE;
    } else {
      *Request = BROWSER_ACTION_SUBMIT | BROWSER_ACTION_RESET;
      StoreInformation (StringPtr);
      // Clear the variable because the user has saved the setup changes.
      CommonSetVariable (
        DATE_CHANGE_INFO_VAR_NAME,
        &gH2OSetupChangeVariableGuid,
        0,
        0,
        NULL
        );
    }

    FreePool (StringPtr);
    RemoveQuestionChangedList (&ChangedQuestionList);
  }
  *ShowSubmitDialog = FALSE;

  if (ChangedQuestionCount != 0) {
    FreePool (ChangedQuestionBuffer);
  }

  return EFI_SUCCESS;
}

/**
 Log the change information for Date.

 @param[in]  Prompt              A pointer to the prompt string of Date question.
 @param[in]  OrgDate             A pointer to the original date.
 @param[in]  ModDate             A pointer to the modified date.

 @retval EFI_SUCCESS             Execute the Log Date Change functions successfully.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
LogDateChange (
  IN    CHAR16                              *Prompt,
  IN    VOID                                *OrgDate,
  IN    VOID                                *ModDate
  )
{
  EFI_STATUS                                Status = EFI_UNSUPPORTED;
  DATE_CHANGED_INFO                         *DateChangeInfo;
  UINTN                                     DateChangeInfoSize;

  if (mOrgDate.Year == 0 || mOrgDate.Month == 0 || mOrgDate.Day== 0) {
    // Initialize the Original date for the first time.
    CopyMem (&mOrgDate, OrgDate, sizeof(EFI_HII_DATE));
  }

  CopyMem (&mModDate, ModDate, sizeof(EFI_HII_DATE));

  if (mDatePromptStr != NULL) {
    gBS->FreePool (mDatePromptStr);
    mDatePromptStr = NULL;
  }

  //
  // Check if the org date is the same as mod date.
  //
  if (CompareMem (&mOrgDate, &mModDate, sizeof(EFI_HII_DATE)) == 0) {
    DEBUG ((EFI_D_ERROR, "No Setup Date changed.\n"));
    ZeroMem (&mOrgDate, sizeof(EFI_HII_DATE));
    // Delete the related variable.
    Status = CommonSetVariable (
               DATE_CHANGE_INFO_VAR_NAME,
               &gH2OSetupChangeVariableGuid,
               0,
               0,
               NULL
               );
    DEBUG ((EFI_D_ERROR, "Delete Date Change variable: %r \n", Status));
    DEBUG ((EFI_D_ERROR, "LogDateChange End \n"));
    return EFI_SUCCESS;
  }

  mDatePromptStr = AllocateZeroPool (sizeof(CHAR16) * (StrLen(Prompt)+1));
  if (mDatePromptStr != NULL) {
    CopyMem (mDatePromptStr, Prompt, sizeof(CHAR16) * StrLen(Prompt));
  }

  DateChangeInfoSize = sizeof (EFI_HII_DATE) + sizeof(CHAR16)*(StrLen(Prompt)+1);
  DateChangeInfo = AllocateZeroPool (DateChangeInfoSize);
  if (DateChangeInfo != NULL) {
    CopyMem (&(DateChangeInfo->OrgDate), &mOrgDate, sizeof(EFI_HII_DATE));

    // Set a variable to store the information for next boot using.
    Status = CommonSetVariable (
               DATE_CHANGE_INFO_VAR_NAME,
               &gH2OSetupChangeVariableGuid,
               EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
               DateChangeInfoSize,
               DateChangeInfo
               );
    DEBUG ((EFI_D_ERROR, "Set variable %s : %r\n", DATE_CHANGE_INFO_VAR_NAME, Status));
    gBS->FreePool (DateChangeInfo);
  }

  return Status;
}

/**
 Restore the change information for Date.

 @param[in]  VOID

 @retval EFI_SUCCESS             Execute the Restore Date Change functions successfully.
 @retval EFI_Status              Otherwise.
*/
EFI_STATUS
RestoreDateChange (
  VOID
  )
{
  EFI_STATUS                                Status = EFI_UNSUPPORTED;
  EFI_TIME                                  EfiTime;

  if (mOrgDate.Year == 0 || mOrgDate.Month == 0 || mOrgDate.Day== 0) {
    // No original Date information.
    return EFI_SUCCESS;
  }

  //
  // Need set the Setup time because user may not accept the Setup time which is set by last boot.
  //
  Status = gRT->GetTime (&EfiTime, NULL);
  if (!EFI_ERROR (Status)) {
    EfiTime.Year = mOrgDate.Year;
    EfiTime.Month= mOrgDate.Month;
    EfiTime.Day  = mOrgDate.Day;
    Status = gRT->SetTime (&EfiTime);
    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Set Original Setup time to %d/%d/%d : %r\n", EfiTime.Year, EfiTime.Month, EfiTime.Day, Status));
      ZeroMem (&mOrgDate, sizeof(EFI_HII_DATE));
      ZeroMem (&mModDate, sizeof(EFI_HII_DATE));
    }

  }

  return Status;
}

/**

 This function is a handler for BIOS to handle Setup Change.

 (See Tiano Runtime Specification)

 @retval EFI Status
*/
EFI_STATUS
EFIAPI
SetupChangeStatusCode (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL
  )
{
  EFI_STATUS                                Status = EFI_UNSUPPORTED;
  SETUP_DATE_CHANGE_STATUS_CODE_MESSAGE     *StatusCodeData;
  SETUP_DATE_CHANGE                         *SetupDateChange;

  if (Data == NULL) {
    return EFI_UNSUPPORTED;
  }

  StatusCodeData  = (SETUP_DATE_CHANGE_STATUS_CODE_MESSAGE *) Data;
  if (StatusCodeData->DataHeader.HeaderSize == 0) {
    return EFI_UNSUPPORTED;
  }

  if ((CompareGuid (&(StatusCodeData->DataHeader.Type), &gH2OSetupChangeStatusCodeGuid) == TRUE) &&
       (CodeType == EFI_PROGRESS_CODE) &&
       (CodeValue == (EFI_SOFTWARE_DXE_RT_DRIVER | EFI_SW_RS_PC_SET_TIME))) {
    SetupDateChange = (SETUP_DATE_CHANGE *) &(StatusCodeData->DateData);

    if (SetupDateChange->SetTime == TRUE) {
      //
      // Log the time change.
      //
      Status = LogDateChange(SetupDateChange->Prompt, &(SetupDateChange->OrgDate), &(SetupDateChange->ModDate));
      return Status;
    } else {
      //
      // Restore the time change.
      //
      Status = RestoreDateChange ();
      return Status;
    }
  }

  return Status;
}

/**
 Entry point of this driver. Install H2O Setup Change protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.

 @retval EFI Status
*/
EFI_STATUS
EFIAPI
H2OSetupChangeDxeEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                   Status;
  EFI_RSC_HANDLER_PROTOCOL     *RscHandlerProtocol = NULL;
  UINTN                        DateChangeInfoDataSize;
  DATE_CHANGED_INFO            *DateChangeInfoData = NULL;
  EFI_TIME                     EfiTime;

  //
  // Install H2OSetupChange protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gH2OSubmitSvcProtocolGuid,
                  &gH2OSubmitSvcProtocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gEfiRscHandlerProtocolGuid,
                  NULL,
                  (VOID **) &RscHandlerProtocol
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Register the worker function to ReportStatusCodeRouter
    //
    Status = RscHandlerProtocol->Register (SetupChangeStatusCode, TPL_HIGH_LEVEL);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = CommonGetVariableDataAndSize (
             DATE_CHANGE_INFO_VAR_NAME,
             &gH2OSetupChangeVariableGuid,
             &DateChangeInfoDataSize,
             &DateChangeInfoData
             );
  if (!EFI_ERROR (Status)) {
    //
    // Need set the Setup time because user may not accept the Setup time which is set by last boot.
    //
    Status = gRT->GetTime (&EfiTime, NULL);
    if (!EFI_ERROR (Status)) {
      EfiTime.Year = DateChangeInfoData->OrgDate.Year;
      EfiTime.Month= DateChangeInfoData->OrgDate.Month;
      EfiTime.Day  = DateChangeInfoData->OrgDate.Day;
      Status = gRT->SetTime (&EfiTime);
      DEBUG ((EFI_D_ERROR, "Set Original Setup time to %d/%d/%d : %r\n", EfiTime.Year, EfiTime.Month, EfiTime.Day, Status));
    }
  }

  return EFI_SUCCESS ;
}
