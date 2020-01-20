/** @file

  Setup Change Smm implementation.

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

#include "H2OSetupChangeSmm.h"

H2O_IHISI_PROTOCOL                        *mH2OIhisi = NULL;
BOOLEAN                                   mFlashStart = FALSE;

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
LogSetupChangeInformation (
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
      //
      // Get option for the policy: Overwrite, Clear, Do nothing.
      //
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
    } 
  } else {
    //
    // Can't find old data from Setup Change Variable, just create new one with new data.
    //
    SetupChangeVarDataSize = NeedVariableSize;
  }

  if (SetupChangeVarDataSize == 0) {
    if (OrgVariableData != NULL) {
      gSmst->SmmFreePool (OrgVariableData);
    }
    return EFI_SUCCESS;
  }

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData, 
                    SetupChangeVarDataSize, 
                    (VOID **)&SetupChangeVarPool
                    );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "SmmAllocatePool() SetupChangeVarPool: %r \n", Status));
    if (OrgVariableData != NULL) {
      gSmst->SmmFreePool (OrgVariableData);
    }
    return Status;
  }
  ZeroMem (SetupChangeVarPool, SetupChangeVarDataSize);
  
  Status = gRT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "EfiGetTime(): %r \n", Status));
    if (OrgVariableData != NULL) {
      gSmst->SmmFreePool (OrgVariableData);
    }
    gSmst->SmmFreePool (SetupChangeVarPool);
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
    gSmst->SmmFreePool (OrgVariableData);
  }

  Status = CommonSetVariable (
             VariableName,
             &gH2OSetupChangeVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             SetupChangeVarDataSize,
             SetupChangeVar
             );
  DEBUG ((EFI_D_INFO, "CommonSetVariable() Setup Change Variable: %r \n", Status));
  gSmst->SmmFreePool (SetupChangeVarPool);

  return Status;

}

/**
  Perform Setup Change function for Flash starting.

  @param[in] void

  @retval EFI_SUCCESS             The function completed successfully.
  @retval others                  Perform Setup Change function for Flash starting procedures failed.
**/
EFI_STATUS
EFIAPI
SetupChangeFlashStart (
  VOID
  )
{
  EFI_STATUS                           Status = EFI_UNSUPPORTED;
  CHAR16                               *TempSettingStr;
  
  TempSettingStr = CatSPrint (
                     NULL,
                     L"Flash Start.\n"
                     );
  
  Status = LogSetupChangeInformation (TempSettingStr);

  gSmst->SmmFreePool (TempSettingStr);
  return Status;
}

/**
  Perform Setup Change function for Flash ending.

  @param[in] CompleteStatus       Status of the flash complete.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval others                  Perform Setup Change function for Flash ending procedures failed.
**/
EFI_STATUS
EFIAPI
SetupChangeFlashEnd (
  UINT8                       CompleteStatus
  )
{
  EFI_STATUS                           Status = EFI_UNSUPPORTED;
  CHAR16                               *TempSettingStr;
  
  switch (CompleteStatus) {
    case ApTerminated:
      TempSettingStr = CatSPrint (NULL,L"Flash End: AP Terminated.\n");
      break;

    case NormalFlash:
      TempSettingStr = CatSPrint (NULL,L"Flash End: Normal Flash.\n");
      break;

    case PartialFlash:
      TempSettingStr = CatSPrint (NULL,L"Flash End: Partial Flash.\n");
      break;

    default:
      TempSettingStr = CatSPrint (NULL,L"Flash End.\n");
      break;
  }
  
  Status = LogSetupChangeInformation (TempSettingStr);
  gSmst->SmmFreePool (TempSettingStr);
  
  return Status;
}

/**
  AH=1Ah, 

  @retval EFI_SUCCESS       
**/
EFI_STATUS
SCFbtsOemCustomization2 (
  VOID
  )
{
  UINT8  Stage;

  Stage = (UINT8) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX);
 
  switch (Stage) {
  case BEFORE_WRITE_PROCESS:  
    mFlashStart = TRUE;
    SetupChangeFlashStart ();
    break;
    
  case AFTER_WRITE_PROCESS:
  default:
    break;
  }

  return EFI_SUCCESS;
}

/**
  AH=16h, 

  @retval EFI_SUCCESS       
**/
EFI_STATUS
SCFbtsComplete (
  VOID
  )
{
  FBTS_FLASH_COMPLETE_STATUS   *FlashCompleteStatus;
  
  if (mFlashStart == FALSE) {
    return EFI_UNSUPPORTED;
  }

  FlashCompleteStatus = (FBTS_FLASH_COMPLETE_STATUS *)(UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  
  if (FlashCompleteStatus->Signature == FLASH_COMPLETE_STATUS_SIGNATURE &&
      FlashCompleteStatus->StructureSize == sizeof (FBTS_FLASH_COMPLETE_STATUS) &&
      (mH2OIhisi->BufferOverlapSmram ((VOID *) FlashCompleteStatus, sizeof (FBTS_FLASH_COMPLETE_STATUS)) == FALSE)) {
    SetupChangeFlashEnd (FlashCompleteStatus->CompleteStatus);
  }  

  mFlashStart = FALSE;
  
  return EFI_SUCCESS;
}


/**
 Entry point of this driver. Install H2O Setup Change Status Code handler into SMM.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
SetupChangeSmmEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  ) 
{
  EFI_STATUS                                Status; 

  Status = gSmst->SmmLocateProtocol (
                    &gH2OIhisiProtocolGuid,
                    NULL,
                    (VOID **) &mH2OIhisi
                    );
  if (!EFI_ERROR (Status)) {
    //
    // Register the worker function to ReportStatusCodeRouter
    //
    Status = mH2OIhisi->RegisterCommand (FBTSOemCustomization2, SCFbtsOemCustomization2, IhisiAboveNormalPriority);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Register Setup Change SMM FBTS function SCFbtsOemCustomization2(): %r\n", Status));
    }
    
    Status = mH2OIhisi->RegisterCommand (FBTSComplete, SCFbtsComplete, IhisiAboveNormalPriority);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Register Setup Change SMM FBTS function SCFbtsComplete(): %r\n", Status));
    }
  }
  
  return Status;
}
