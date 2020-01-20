/** @file

 Setup function for SetupUtilityDxe Driver

;******************************************************************************
;* Copyright (c) 2012-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "SetupFuncs.h"
#include <Library/HiiLib.h>
#include <Guid/HobList.h>
#include <Library/DxeOemSvcKernelLib.h>
#include <Library/BaseSetupDefaultLib.h>
#include <Library/MultiConfigBaseLib.h>
#include <SetupConfig.h>

/**
 Given a token, return the string.

 @param [in] HiiHandle          the handle the token is located
 @param [in] Token              the string reference
 @param [in] LanguageString     indicate what language string we want to get. if this is a
                                NULL pointer, using the current language setting to get string

 @retval *CHAR16                Returns the string corresponding to the token
 @retval NULL                   Cannot get string from Hii database

**/
CHAR16 *
GetTokenStringByLanguage (
  IN EFI_HII_HANDLE                           HiiHandle,
  IN STRING_REF                               Token,
  IN CHAR8                                    *LanguageString
  )
{
  CHAR16                                      *Buffer;
  UINTN                                       BufferLength;
  EFI_STATUS                                  Status;
  EFI_HII_STRING_PROTOCOL                     *HiiString;

  HiiString = gSUBrowser->HiiString;
  //
  // Set default string size assumption at no more than 256 bytes
  //
  BufferLength = 0x100;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  BufferLength,
                  (VOID**)&Buffer
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }
  ZeroMem (Buffer, BufferLength);
  Status = HiiString->GetString (
                        HiiString,
                        LanguageString,
                        HiiHandle,
                        Token,
                        Buffer,
                        &BufferLength,
                        NULL
                        );
  if (EFI_ERROR (Status)) {
    if (Status == EFI_BUFFER_TOO_SMALL) {
      //
      // Free the old pool
      //
      gBS->FreePool (Buffer);

      //
      // Allocate new pool with correct value
      //
      gBS->AllocatePool (
             EfiBootServicesData,
             BufferLength,
             (VOID**)&Buffer
             );
      Status = HiiString->GetString (
                            HiiString,
                            LanguageString,
                            HiiHandle,
                            Token,
                            Buffer,
                            &BufferLength,
                            NULL
                            );
      if (!EFI_ERROR (Status)) {
        //
        // return searched string
        //
        return Buffer;
      }
    }
    //
    // Cannot find string, free buffer and return NULL pointer
    //
    gBS->FreePool (Buffer);
    Buffer = NULL;
    return Buffer;
  }
  //
  // return searched string
  //
  return Buffer;
}


EFI_STATUS
CheckLanguage (
  VOID
  )
{
  EFI_STATUS                            Status;
  CHAR8                                 *LangCode;
  UINT8                                 *BackupLang;
  UINTN                                 BufferSize;
  UINTN                                 BackupBufferSize;


  LangCode   = GetVariableAndSize (L"PlatformLang", &gEfiGlobalVariableGuid, &BufferSize);
  BackupLang = GetVariableAndSize (L"BackupPlatformLang", &gEfiGenericVariableGuid, &BackupBufferSize);
  if (LangCode == NULL || !SetupUtilityLibIsLangCodeSupport (LangCode)) {
    //
    // if cannot find current language, set default language as english
    //
    if (BackupLang == NULL || !SetupUtilityLibIsLangCodeSupport ((CHAR8 *)BackupLang)) {
      Status = gRT->SetVariable (
                      L"PlatformLang",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      AsciiStrSize ((CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang)),
                      (VOID *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang)
                      );
    } else {
      Status = gRT->SetVariable (
                      L"PlatformLang",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      BackupBufferSize,
                      BackupLang
                      );
    }

    gRT->SetVariable (
           L"BackupPlatformLang",
           &gEfiGenericVariableGuid,
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
           0,
           NULL
           );

    if (LangCode != NULL) {
      gBS->FreePool (LangCode);
    }
    if (BackupLang != NULL) {
      gBS->FreePool (BackupLang);
    }
    
    return Status;
  }

  if (BackupLang != NULL) {
    if (BackupBufferSize != BufferSize || CompareMem (BackupLang, LangCode, BufferSize)) {
      Status = gRT->SetVariable (
                      L"PlatformLang",
                      &gEfiGlobalVariableGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      BackupBufferSize,
                      BackupLang
                      );
    }

    Status = gRT->SetVariable (
                    L"BackupPlatformLang",
                    &gEfiGenericVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    0,
                    NULL
                    );
    gBS->FreePool (BackupLang);    
  }

  gBS->FreePool (LangCode);

  return EFI_SUCCESS;
}
/**
 Read the EFI variable (VendorGuid/Name) and return a dynamically allocated
 buffer, and the size of the buffer. If failure return NULL.

 @param [in]  Name              String part of EFI variable name
 @param [in]  VendorGuid        GUID part of EFI variable name
 @param [out] VariableSize      Returns the size of the EFI variable that was read

 @return Dynamically allocated memory that contains a copy of the EFI variable.
 @return Caller is responsible freeing the buffer.
 @retval NULL                   Variable was not read

**/
VOID *
GetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;

  Buffer = NULL;

  //
  // Pass in a zero size buffer to find the required buffer size.
  //
  BufferSize  = 0;

  Status      = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //

    Buffer = AllocateZeroPool (BufferSize);
    if (Buffer == NULL) {
      return NULL;
    }

    //
    // Read variable into the allocated buffer.
    //

    Status = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
    if (EFI_ERROR (Status)) {
      BufferSize = 0;
      gBS->FreePool (Buffer);
      Buffer = NULL;
    }

  }

  *VariableSize = BufferSize;

  return Buffer;
}

/**
 Function to update the ATA strings into Model Name -- Size

 @param [in]        IdentifyDriveInfo
 @param [in, out]   NewString

 @return Will return model name and size (or ATAPI if nonATA)

**/
EFI_STATUS
UpdateAtaString (
  IN     EFI_ATAPI_IDENTIFY_DATA     *IdentifyDriveInfo,
  IN OUT CHAR16                      **NewString
  )
{
  CHAR8                       *TempString;
  UINT16                      Index;
  CHAR8                       Temp8;

  TempString = AllocateZeroPool (0x100);

  CopyMem (TempString, IdentifyDriveInfo->ModelName, sizeof (IdentifyDriveInfo->ModelName));

  //
  // Swap the IDE string since Identify Drive format is inverted
  //
  Index = 0;
  while (TempString[Index] != 0 && TempString[Index+1] != 0) {
    Temp8 = TempString[Index];
    TempString[Index] = TempString[Index+1];
    TempString[Index+1] = Temp8;
    Index +=2;
  }
  AsciiToUnicode (TempString, *NewString);

  if (TempString != NULL) {
    gBS->FreePool(TempString);
  }

  return EFI_SUCCESS;
}

/**
 Function to convert ASCII string to Unicode

 @param [in]   AsciiString
 @param [in]   UnicodeString


**/
EFI_STATUS
AsciiToUnicode (
  IN CHAR8        *AsciiString,
  IN CHAR16       *UnicodeString
  )
{
  UINT8           Index;

  Index = 0;
  while (AsciiString[Index] != 0) {
    UnicodeString[Index] = (CHAR16)AsciiString[Index];
    Index++;
  }
  UnicodeString[Index] = 0;

  return EFI_SUCCESS;
}

/**
 Control Event Timer start and end.

 @param [in]   Timeout          Event time, Zero is Stop the timer

 @retval EFI_SUCCESS            Control Event Timer is success.

**/
EFI_STATUS
EventTimerControl (
  IN UINT64                     Timeout
  )
{

  //
  // If the system health event is create for this Key value ,we can stop event
  //
  if (gSCUSystemHealth == HAVE_CREATE_SYSTEM_HEALTH_EVENT) {
    //
    // Set the timer event
    //
    gBS->SetTimer (
           gSCUTimerEvent,
           TimerPeriodic,
           Timeout
           );
  }

  return EFI_SUCCESS;

}


/**
 Save Setup Configuration to NV storage and call KERNEL_CALCULATE_WRITE_CMOS_CHECKSUM
 OEM service

 @param [in]   VariableName     The name of variable to load
 @param [in]   VendorGuid       It's vendor GUID
 @param [in]   Attributes       attributes of the data to be stored
 @param [in]   DataSize         The size of the data
 @param [in]   Buffer           Space to store data to be written to NVRam

 @retval EFI_SUCCESS            Save Setup Configuration successful
 @retval EFI_INVALID_PARAMETER  The input parameter is invalid
 @return Other                  Some other error occured

**/
EFI_STATUS
SaveSetupConfig (
  IN CHAR16                 *VariableName,
  IN EFI_GUID               *VendorGuid,
  IN UINT32                 Attributes,
  IN UINTN                  DataSize,
  IN VOID                   *Buffer
  )
{
  EFI_STATUS                Status;


  if (Buffer == NULL || VariableName == NULL || VendorGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = SetVariableToSensitiveVariable (
                  VariableName,
                  VendorGuid,
                  Attributes,
                  DataSize,
                  Buffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = OemSvcCalculateWriteCmosChecksum();
  return EFI_SUCCESS;
}

/**
 Get string from input HII handle and add this string

 @param [in]   InputHiiHandle
 @param [in]   OutputHiiHandle
 @param [in]   InputToken
 @param [out]  OutputToken

 @retval EFI_SUCCESS            Update system bus speed success

**/
EFI_STATUS
AddNewString (
  IN  EFI_HII_HANDLE         InputHiiHandle,
  IN  EFI_HII_HANDLE         OutputHiiHandle,
  IN  STRING_REF             InputToken,
  OUT STRING_REF             *OutputToken
  )
{
  UINTN                      LanguageCount;
  UINTN                      TotalLanguageCount;
  CHAR8                      *LanguageString;
  CHAR16                     *TokenString;
  EFI_STATUS                 Status;
  STRING_REF                 AddedToken;


  //
  // Get all of supported language
  //
  Status = GetLangDatabase (&TotalLanguageCount, (UINT8 **)&LanguageString);
    if (EFI_ERROR (Status)) {
    return Status;
  }

  AddedToken = 0;
  //
  // Create new string token for this string
  //
  TokenString=HiiGetString (InputHiiHandle, InputToken, NULL);
  ASSERT (TokenString != NULL);
  AddedToken=HiiSetString (OutputHiiHandle, 0, TokenString, NULL);
  gBS->FreePool (TokenString);
  //
  // according to different language to get string token from input HII handle and
  // add this string to oupt string hanlde
  //
  for (LanguageCount = 0; LanguageCount < TotalLanguageCount; LanguageCount++) {
    TokenString = HiiGetString (
                    InputHiiHandle, 
                    InputToken, 
                    &LanguageString[LanguageCount * RFC_3066_ENTRY_SIZE]
                    );
    if (TokenString != NULL) {
      Status = gSUBrowser->HiiString->SetString (
                                        gSUBrowser->HiiString,
                                        OutputHiiHandle,
                                        AddedToken,
                                        &LanguageString[LanguageCount * RFC_3066_ENTRY_SIZE],
                                        TokenString,
                                        NULL
                                        );
      gBS->FreePool (TokenString);
    }
  }
  gBS->FreePool (LanguageString);
  *OutputToken = AddedToken;

  return EFI_SUCCESS;
}

/**
 Build a default variable value and save to a buffer according to platform requirement

 @param [out]  SetupNvData

 @retval EFI_SUCCESS            Build default setup successful.
 @retval EFI_INVALID_PARAMETER  Input value is invalid.

**/
EFI_STATUS
DefaultSetup (
  OUT SYSTEM_CONFIGURATION          *SetupNvData
  )
{
  UINTN                                     BufferSize;
  EFI_GUID                                  SetupVariableGuidId = SYSTEM_CONFIGURATION_GUID;
  
  if (SetupNvData == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  BufferSize = PcdGet32 (PcdSetupConfigSize);

  ExtractSetupDefault ((UINT8 *) SetupNvData );
  if (FeaturePcdGet (PcdMultiConfigSupported)) {
      SetSCUDataFromMC (L"Setup", &SetupVariableGuidId, SETUP_FOR_LOAD_DEFAULT, (VOID *)SetupNvData, sizeof (SYSTEM_CONFIGURATION));
  }
  SetupNvData->Timeout = (UINT16) PcdGet16 (PcdPlatformBootTimeOut);
  GetLangIndex ((CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang), &SetupNvData->Language);
  
  return EFI_SUCCESS;
}

