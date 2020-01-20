/** @file
  GenericUtilityLib

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**
  The platform boot manager reference implementation

Copyright (c) 2004 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BootManager.h"

STATIC EFI_FORM_BROWSER2_PROTOCOL         *gBootManagerFormBrowser2;
BOOLEAN                                   gConnectAllHappened;

STATIC LIST_ENTRY                          mBackupBootOptionsList = INITIALIZE_LIST_HEAD_VARIABLE (mBackupBootOptionsList);
STATIC LIST_ENTRY                          *mBootOptionsList;
STATIC H2O_BDS_LOAD_OPTION                 *gOption;
STATIC CHAR16                              *mDeviceTypeStr[] = {
  L"Legacy BEV",
  L"Legacy Floppy",
  L"Legacy Hard Drive",
  L"Legacy CD ROM",
  L"Legacy PCMCIA",
  L"Legacy USB",
  L"Legacy Embedded Network",
  L"Legacy Unknown Device"
};

STATIC CHAR16                              mEfiBootDevTypeStr[] = L"EFI Boot Devices";

STATIC HII_VENDOR_DEVICE_PATH              mBootManagerHiiVendorDevicePath = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
        (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8)
      }
    },
    BOOT_MANAGER_FORMSET_GUID
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

STATIC BOOT_MANAGER_CALLBACK_DATA  gBootManagerPrivate = {
  BOOT_MANAGER_CALLBACK_DATA_SIGNATURE,
  NULL,
  NULL,
  {
    BootManagerFakeExtractConfig,
    BootManagerFakeRouteConfig,
    BootManagerCallback
  }
};

/**
  Compares two boot option.

  @param[in] BootOptions1       A pointer to one boot option.
  @param[in] BootOptions2       A pointer to another boot option.

  @retval TRUE        BootOptions1 and BootOptions2 are identical.
  @retval FALSE       BootOptions1 and BootOptions2 are not identical.
**/
STATIC
BOOLEAN
IsIdenticalBootOption (
  IN H2O_BDS_LOAD_OPTION      *Option1,
  IN H2O_BDS_LOAD_OPTION      *Option2
  )
{
  if (Option1 == NULL || Option2 == NULL) {
    return FALSE;
  }

  if (Option1->DevicePath == Option2->DevicePath ||
      (GetDevicePathSize (Option1->DevicePath) == GetDevicePathSize (Option2->DevicePath) &&
       CompareMem (Option1->DevicePath, Option2->DevicePath, GetDevicePathSize (Option1->DevicePath)) == 0)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Free each boot option from boot option list.

  @param[in, out] BootOptionsList       A pointer to the boot option list.
**/
STATIC
VOID
FreeBootOptionList (
  IN OUT LIST_ENTRY           *BootOptionsList
  )
{
  LIST_ENTRY                  *Link;
  H2O_BDS_LOAD_OPTION         *Option;

  if (BootOptionsList == NULL) {
    return;
  }

  Link = GetFirstNode (BootOptionsList);
  while (!IsNull (BootOptionsList, Link)) {
    Option = BDS_OPTION_FROM_LINK (Link);
    Link   = GetNextNode (BootOptionsList, Link);

    RemoveEntryList (&Option->Link);
    FreePool (Option);
  }
}

/**
  Copy the source boot option list to the destination boot option list.

  @param[in, out] DstBootOptionsList       A pointer to the destination boot option list.
  @param[in]      SrcBootOptionsList       A pointer to the source boot option list.
**/
STATIC
VOID
CopyBootOptionList (
  IN OUT LIST_ENTRY           *DstBootOptionsList,
  IN     LIST_ENTRY           *SrcBootOptionsList
  )
{
  LIST_ENTRY                  *SrcLink;
  H2O_BDS_LOAD_OPTION         *SrcOption;
  H2O_BDS_LOAD_OPTION         *DstOption;

  if (DstBootOptionsList == NULL || SrcBootOptionsList == NULL) {
    return;
  }

  SrcLink = GetFirstNode (SrcBootOptionsList);
  while (!IsNull (SrcBootOptionsList, SrcLink)) {
    SrcOption = BDS_OPTION_FROM_LINK (SrcLink);
    SrcLink   = GetNextNode (SrcBootOptionsList, SrcLink);

    DstOption = AllocateCopyPool (sizeof (H2O_BDS_LOAD_OPTION), SrcOption);
    if (DstOption != NULL) {
      InsertTailList (DstBootOptionsList, &DstOption->Link);
    }
  }
}

/**
  Compares two boot option lists.

  @param[in] BootOptionsList1       A pointer to one boot option list.
  @param[in] BootOptionsList2       A pointer to another boot option list.

  @retval TRUE        BootOptionsList1 and BootOptionsList2 are identical.
  @retval FALSE       BootOptionsList1 and BootOptionsList2 are not identical.
**/
STATIC
BOOLEAN
CompareBootOptionList (
  IN LIST_ENTRY               *BootOptionsList1,
  IN LIST_ENTRY               *BootOptionsList2
  )
{
  LIST_ENTRY                  *Link1;
  LIST_ENTRY                  *Link2;
  H2O_BDS_LOAD_OPTION         *Option1;
  H2O_BDS_LOAD_OPTION         *Option2;

  if (BootOptionsList1 == NULL || BootOptionsList2 == NULL) {
    return FALSE;
  }

  Link1 = GetFirstNode (BootOptionsList1);
  Link2 = GetFirstNode (BootOptionsList2);
  while (!IsNull (BootOptionsList1, Link1) &&
         !IsNull (BootOptionsList2, Link2)) {
    Option1 = BDS_OPTION_FROM_LINK (Link1);
    Option2 = BDS_OPTION_FROM_LINK (Link2);
    if (!IsIdenticalBootOption (Option1, Option2)) {
      return FALSE;
    }

    Link1 = GetNextNode (BootOptionsList1, Link1);
    Link2 = GetNextNode (BootOptionsList2, Link2);
  }
  if (IsNull (BootOptionsList1, Link1) &&
      IsNull (BootOptionsList2, Link2)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Free each BDS option from option list.

  @param[in, out] OptionsList       A pointer to the BDS option list.
**/
STATIC
VOID
FreeBdsOptionList (
  IN OUT LIST_ENTRY           *OptionsList
  )
{
  LIST_ENTRY                  *Link;
  BDS_COMMON_OPTION           *Option;

  if (OptionsList == NULL) {
    return;
  }

  Link = GetFirstNode (OptionsList);
  while (!IsNull (OptionsList, Link)) {
    Option = BDS_COMMON_OPTION_FROM_LINK (Link);
    Link   = GetNextNode (OptionsList, Link);

    RemoveEntryList (&Option->Link);
    if (Option->DevicePath != NULL) {
      FreePool (Option->DevicePath);
    }
    if (Option->Description != NULL) {
      FreePool (Option->Description);
    }
    if (Option->LoadOptions != NULL) {
      FreePool (Option->LoadOptions);
    }
    FreePool (Option);
  }
}

/**
  This function allows a caller to extract the current configuration for one
  or more named elements from the target driver.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Request         A null-terminated Unicode string in <ConfigRequest> format.
  @param Progress        On return, points to a character in the Request string.
                         Points to the string's null terminator if request was successful.
                         Points to the most recent '&' before the first failing name/value
                         pair (or the beginning of the string if the failure is in the
                         first name/value pair) if the request was not successful.
  @param Results         A null-terminated Unicode string in <ConfigAltResp> format which
                         has all values filled in for the names in the Request string.
                         String to be allocated by the called function.

  @retval  EFI_SUCCESS            The Results is filled with the requested values.
  @retval  EFI_OUT_OF_RESOURCES   Not enough memory to store the results.
  @retval  EFI_INVALID_PARAMETER  Request is illegal syntax, or unknown name.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
BootManagerFakeExtractConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  if (Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  *Progress = Request;
  return EFI_NOT_FOUND;
}

/**
  This function processes the results of changes in configuration.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Configuration   A null-terminated Unicode string in <ConfigResp> format.
  @param Progress        A pointer to a string filled in with the offset of the most
                         recent '&' before the first failing name/value pair (or the
                         beginning of the string if the failure is in the first
                         name/value pair) or the terminating NULL if all was successful.

  @retval  EFI_SUCCESS            The Results is processed successfully.
  @retval  EFI_INVALID_PARAMETER  Configuration is NULL.
  @retval  EFI_NOT_FOUND          Routing data doesn't match any storage in this driver.

**/
EFI_STATUS
EFIAPI
BootManagerFakeRouteConfig (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  if (Configuration == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Progress = Configuration;
  if (!HiiIsConfigHdrMatch (Configuration, &gBootMaintFormSetGuid, mBootMaintStorageName)
      && !HiiIsConfigHdrMatch (Configuration, &gFileExploreFormSetGuid, mFileExplorerStorageName)) {
    return EFI_NOT_FOUND;
  }

  *Progress = Configuration + StrLen (Configuration);
  return EFI_SUCCESS;
}

/**
  This call back function is registered with Boot Manager formset.
  When user selects a boot option, this call back function will
  be triggered. The boot option is saved for later processing.


  @param This            Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
  @param Action          Specifies the type of action taken by the browser.
  @param QuestionId      A unique value which is sent to the original exporting driver
                         so that it can identify the type of data to expect.
  @param Type            The type of value for the question.
  @param Value           A pointer to the data being sent to the original exporting driver.
  @param ActionRequest   On return, points to the action requested by the callback function.

  @retval  EFI_SUCCESS           The callback successfully handled the action.
  @retval  EFI_INVALID_PARAMETER The setup browser call this function with invalid parameters.

**/
EFI_STATUS
EFIAPI
BootManagerCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
{
  H2O_BDS_LOAD_OPTION     *Option;
  LIST_ENTRY              *Link;
  UINT16                  KeyCount;

  if (!FeaturePcdGet (PcdH2OFormBrowserSupported) && Action == EFI_BROWSER_ACTION_CHANGING) {
    //
    // For compatible to old form browser which only use EFI_BROWSER_ACTION_CHANGING action,
    // change action to EFI_BROWSER_ACTION_CHANGED to make it workable.
    //
    Action = EFI_BROWSER_ACTION_CHANGED;
  }

  if (Action == EFI_BROWSER_ACTION_CHANGED) {
    if ((Value == NULL) || (ActionRequest == NULL)) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // Initialize the key count
    //
    KeyCount = 0;
    gOption = NULL;
    for (Link = GetFirstNode (mBootOptionsList); !IsNull (mBootOptionsList, Link); Link = GetNextNode (mBootOptionsList, Link)) {
      Option = BDS_OPTION_FROM_LINK (Link);

      KeyCount++;

      //
      // Is this device the one chosen?
      //
      if (KeyCount == QuestionId) {
         gOption = Option;
        //
        // Request to exit SendForm(), so that we could boot the selected option
        //
        *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
        break;
      }
    }

    return EFI_SUCCESS;
  }

  //
  // All other action return unsupported.
  //
  return EFI_UNSUPPORTED;
}

/**

  Registers HII packages for the Boot Manger to HII Database.
  It also registers the browser call back function.

  @retval  EFI_SUCCESS           HII packages for the Boot Manager were registered successfully.
  @retval  EFI_OUT_OF_RESOURCES  HII packages for the Boot Manager failed to be registered.

**/
EFI_STATUS
InitializeBootManager (
  VOID
  )
{
  EFI_STATUS                  Status;

  Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &gBootManagerFormBrowser2);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install Device Path Protocol and Config Access protocol to driver handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gBootManagerPrivate.DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  &mBootManagerHiiVendorDevicePath,
                  &gEfiHiiConfigAccessProtocolGuid,
                  &gBootManagerPrivate.ConfigAccess,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Publish our HII data
  //
  gBootManagerPrivate.HiiHandle = HiiAddPackages (
                                    &gBootManagerFormSetGuid,
                                    gBootManagerPrivate.DriverHandle,
                                    BootManagerVfrBin,
                                    GenericUtilityLibStrings,
                                    GenericUtilityLibImages,
                                    NULL
                                    );
  if (gBootManagerPrivate.HiiHandle == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  } else {
    Status = EFI_SUCCESS;
  }
  return Status;
}

/**
 Check the specific BBS Table entry is USB device

 @param[in] CurrentBbsTable     Pointer to current BBS table start address

 @retval TRUE                   It is USB device
 @retval FALSE                  It isn't USB device
**/
STATIC
BOOLEAN
IsUsbDevice (
  IN BBS_TABLE              *CurrentBbsTable
  )
{
  if ((CurrentBbsTable->Class == PCI_CLASS_SERIAL) &&
      (CurrentBbsTable->SubClass == PCI_CLASS_SERIAL_USB)) {
    return TRUE;
  }

  return FALSE;
}

/**
 Get prompt string which is consist of the description of EFI boot option and hardware device name

 @param[in] Option              Pointer to boot option data
 @param[in] HwBootDeviceInfo    Pointer to the array of hardware device info
 @param[in] HwBootDeviceCount   The number of hardware device info

 @return pointer to the prompt string or NULL if input parameter is invalid
**/
CHAR16 *
GetPromptString (
  IN H2O_BDS_LOAD_OPTION                *Option,
  IN HARDWARE_BOOT_DEVICE_INFO          *HwBootDeviceInfo,
  IN UINTN                              HwBootDeviceCount
  )
{
  CHAR16                                NoDeviceStr[] = L"No Device";
  UINTN                                 Index;
  UINT8                                 *Ptr;
  UINT8                                 *VarData;
  UINTN                                 DeviceCount;
  UINTN                                 Size;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  CHAR16                                *HwDeviceName;
  CHAR16                                *PromptString;


  if (Option == NULL) {
    return NULL;
  }

  if (!((DevicePathType (Option->DevicePath) == MEDIA_DEVICE_PATH &&
         DevicePathSubType (Option->DevicePath) == MEDIA_HARDDRIVE_DP) ||
        (DevicePathType (Option->DevicePath) == MESSAGING_DEVICE_PATH &&
         DevicePathSubType (Option->DevicePath) == MSG_USB_CLASS_DP))) {
    return AllocateCopyPool (StrSize (Option->Description), Option->Description);
  }

  VarData      = NULL;
  HwDeviceName = NULL;
  DeviceCount  = 1;
  DevicePath   = Option->DevicePath;

  if (DevicePathType (Option->DevicePath) == MESSAGING_DEVICE_PATH &&
      DevicePathSubType (Option->DevicePath) == MSG_USB_CLASS_DP) {
    //
    // For Windows To Go, assign no device string as device name if device number is 0.
    //
    VarData = BdsLibGetVariableAndSize (
                L"WindowsToGo",
                &gEfiGenericVariableGuid,
                &Size
                );
    if (VarData != NULL) {
      Ptr         = VarData;
      DevicePath  = (EFI_DEVICE_PATH_PROTOCOL *) Ptr;
      Ptr        += GetDevicePathSize (DevicePath);
      DeviceCount = *(UINT16 *) Ptr;
    } else {
      DeviceCount = 0;
    }

    if (DeviceCount == 0) {
      HwDeviceName = NoDeviceStr;
    }
  }

  //
  // Append device name to boot option description if BlkIo device path is match.
  //
  if (HwDeviceName == NULL) {
    for (Index = 0; Index < HwBootDeviceCount; Index++) {
      if (BdsLibCompareBlockIoDevicePath (HwBootDeviceInfo[Index].BlockIoDevicePath, DevicePath)) {
        HwDeviceName = HwBootDeviceInfo[Index].HwDeviceName;
        break;
      }
    }
  }

  if (HwDeviceName != NULL) {
    if (DeviceCount > 1) {
      Size = StrSize (Option->Description) + StrSize (L" (") + StrSize (HwDeviceName) + StrSize (L",...") + StrSize (L")");
    } else {
      Size = StrSize (Option->Description) + StrSize (L" (") + StrSize (HwDeviceName) + StrSize (L")");
    }
    PromptString = AllocateZeroPool (Size);
    if (PromptString != NULL) {
      StrCat (PromptString, Option->Description);
      StrCat (PromptString, L" (");
      StrCat (PromptString, HwDeviceName);
      if (DeviceCount > 1) {
        StrCat (PromptString, L",...");
      }
      StrCat (PromptString, L")");
    }
  } else {
    PromptString = AllocateCopyPool (StrSize (Option->Description), Option->Description);
  }

  if (VarData != NULL) {
    gBS->FreePool (VarData);
  }

  return PromptString;
}

/**
  Update boot option list into HII form.
**/
VOID
UpdateBootOptionListIntoHiiForm (
  VOID
  )
{
  H2O_BDS_LOAD_OPTION         *Option;
  LIST_ENTRY                  *Link;
  EFI_STRING_ID               Token;
  EFI_HII_HANDLE              HiiHandle;
  VOID                        *StartOpCodeHandle;
  VOID                        *EndOpCodeHandle;
  EFI_IFR_GUID_LABEL          *StartLabel;
  EFI_IFR_GUID_LABEL          *EndLabel;
  UINT16                      DeviceType;
  BOOLEAN                     IsLegacyOption;
  BOOLEAN                     NeedEndOp;
  BOOLEAN                     PreviousOptionIsLegacy;
  UINTN                       Index;
  HARDWARE_BOOT_DEVICE_INFO   *HwBootDeviceInfo;
  UINTN                       HwBootDeviceCount;
  CHAR16                      *PromptString;
  UINT16                      OptionDeviceType;
  UINT16                      KeyInput;


  Token      = 0;
  DeviceType = (UINT16) -1;
  OptionDeviceType = (UINT16) -1;

  HiiHandle = gBootManagerPrivate.HiiHandle;

  BdsLibGetAllHwBootDeviceInfo (&HwBootDeviceCount, &HwBootDeviceInfo);

  //
  // Allocate space for creation of UpdateData Buffer
  //
  StartOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (StartOpCodeHandle != NULL);

  EndOpCodeHandle = HiiAllocateOpCodeHandle ();
  ASSERT (EndOpCodeHandle != NULL);

  //
  // Create Hii Extend Label OpCode as the start opcode
  //
  StartLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  StartLabel->Number       = LABEL_BOOT_OPTION;

  //
  // Create Hii Extend Label OpCode as the end opcode
  //
  EndLabel = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (EndOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
  EndLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
  EndLabel->Number       = LABEL_BOOT_OPTION_END;

  KeyInput = 0;
  NeedEndOp = FALSE;
  PreviousOptionIsLegacy = TRUE;
  for (Link = GetFirstNode (mBootOptionsList); !IsNull (mBootOptionsList, Link); Link = GetNextNode (mBootOptionsList, Link)) {
    Option = BDS_OPTION_FROM_LINK (Link);
    //
    // At this stage we are creating a menu entry, thus the Keys are reproduceable
    //
    KeyInput++;

    //
    // Don't display the hidden/inactive boot option
    //
    if (((Option->Attributes & LOAD_OPTION_HIDDEN) != 0) || ((Option->Attributes & LOAD_OPTION_ACTIVE) == 0)) {
      continue;
    }

    //
    // Group the legacy boot option in the sub title created dynamically
    //
    IsLegacyOption = (BOOLEAN) (
                       (DevicePathType (Option->DevicePath) == BBS_DEVICE_PATH) &&
                       (DevicePathSubType (Option->DevicePath) == BBS_BBS_DP)
                       );

    if (NeedEndOp && IsLegacyOption != PreviousOptionIsLegacy) {
      NeedEndOp = FALSE;
      HiiCreateEndOpCode (StartOpCodeHandle);
    }

    if (IsLegacyOption) {
      if (Option->LoadOptionalDataSize != 0 &&
          IsUsbDevice ((BBS_TABLE *) Option->LoadOptionalData)) {
        OptionDeviceType = BBS_USB;
      } else {
        OptionDeviceType = ((BBS_BBS_DEVICE_PATH *) Option->DevicePath)->DeviceType;
      }
    }

    if (IsLegacyOption && DeviceType != OptionDeviceType) {
      if (NeedEndOp) {
        HiiCreateEndOpCode (StartOpCodeHandle);
      }

      DeviceType = OptionDeviceType;
      Token      = HiiSetString (
                     HiiHandle,
                     0,
                     mDeviceTypeStr[
                       MIN (DeviceType & 0xF, sizeof (mDeviceTypeStr) / sizeof (mDeviceTypeStr[0]) - 1)
                       ],
                     NULL
                     );
      HiiCreateSubTitleOpCode (StartOpCodeHandle, Token, 0, 0, 1);
      NeedEndOp = TRUE;
    }

    if (!IsLegacyOption && IsLegacyOption != PreviousOptionIsLegacy) {
      if (NeedEndOp) {
        HiiCreateEndOpCode (StartOpCodeHandle);
      }

      Token = HiiSetString (HiiHandle, 0, mEfiBootDevTypeStr, NULL );
      HiiCreateSubTitleOpCode (StartOpCodeHandle, Token, 0, 0, 1);
      NeedEndOp = TRUE;
    }

    ASSERT (Option->Description != NULL);

    PromptString = GetPromptString (
                     Option,
                     HwBootDeviceInfo,
                     HwBootDeviceCount
                     );
    if (PromptString != NULL) {
      Token = HiiSetString (HiiHandle, 0, PromptString, NULL);
      gBS->FreePool (PromptString);
    } else {
      Token = HiiSetString (HiiHandle, 0, Option->Description, NULL);
    }

    HiiCreateActionOpCode (
      StartOpCodeHandle,
      KeyInput,
      Token,
      0,
      EFI_IFR_FLAG_CALLBACK,
      0
      );

    PreviousOptionIsLegacy = IsLegacyOption;
  }

  if (NeedEndOp) {
    HiiCreateEndOpCode (StartOpCodeHandle);
  }

  HiiUpdateForm (
    HiiHandle,
    &gBootManagerFormSetGuid,
    BOOT_MANAGER_FORM_ID,
    StartOpCodeHandle,
    EndOpCodeHandle
    );

  HiiFreeOpCodeHandle (StartOpCodeHandle);
  HiiFreeOpCodeHandle (EndOpCodeHandle);

  if (HwBootDeviceCount != 0 && HwBootDeviceInfo != NULL) {
    for (Index = 0; Index < HwBootDeviceCount; Index++) {
      gBS->FreePool (HwBootDeviceInfo[Index].HwDeviceName);
    }
    gBS->FreePool (HwBootDeviceInfo);
  }
}

/**
  Event function to update boot option list. If boot option list is changed, update HII form.

  @param[in] Event                  The Event this notify function registered to.
  @param[in] Context                Pointer to the context data registerd to the Event.
**/
STATIC
VOID
EFIAPI
UpdateBootOptionListEvtFunc (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  LIST_ENTRY                  OptionList;

  InitializeListHead (&OptionList);
  BdsLibEnumerateAllBootOption (FALSE, &OptionList);
  FreeBdsOptionList (&OptionList);
  gBdsServices->GetBootList (gBdsServices, &mBootOptionsList);

  if (!CompareBootOptionList (&mBackupBootOptionsList, mBootOptionsList)) {
    FreeBootOptionList (&mBackupBootOptionsList);
    CopyBootOptionList (&mBackupBootOptionsList, mBootOptionsList);

    UpdateBootOptionListIntoHiiForm ();
  }
}

/**
  This function invokes Boot Manager. If all devices have not a chance to be connected,
  the connect all will be triggered. It then enumerate all boot options. If
  a boot option from the Boot Manager page is selected, Boot Manager will boot
  from this boot option.

**/
VOID
CallBootManager (
  VOID
  )
{
  EFI_STATUS                  Status;
  LIST_ENTRY                  OptionList;
  CHAR16                      *ExitData;
  UINTN                       ExitDataSize;
  EFI_HII_HANDLE              HiiHandle;
  EFI_BROWSER_ACTION_REQUEST  ActionRequest;
  EFI_EVENT                   Event;

  gOption    = NULL;
  InitializeListHead (&OptionList);

  //
  // Connect all prior to entering the platform setup menu.
  //
  if (!gConnectAllHappened) {
    BdsLibConnectAllDriversToAllControllers ();
    gConnectAllHappened = TRUE;
  }

  BdsLibEnumerateAllBootOption (FALSE, &OptionList);
  Status = gBdsServices->GetBootList (gBdsServices, &mBootOptionsList);
  ASSERT_EFI_ERROR (Status);

  HiiHandle = gBootManagerPrivate.HiiHandle;

  CopyBootOptionList (&mBackupBootOptionsList, mBootOptionsList);
  UpdateBootOptionListIntoHiiForm ();

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  UpdateBootOptionListEvtFunc,
                  NULL,
                  &Event
                  );
  if (!EFI_ERROR(Status)) {
    Status = gBS->SetTimer (Event, TimerPeriodic, EFI_TIMER_PERIOD_SECONDS(1));
  } else {
    Event = NULL;
  }

  ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;
  Status = gBootManagerFormBrowser2->SendForm (
                                       gBootManagerFormBrowser2,
                                       &HiiHandle,
                                       1,
                                       &gBootManagerFormSetGuid,
                                       0,
                                       NULL,
                                       &ActionRequest
                                       );
  if (ActionRequest == EFI_BROWSER_ACTION_REQUEST_RESET) {
    EnableResetRequired ();
  }

  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }
  FreeBootOptionList (&mBackupBootOptionsList);

  if (gOption == NULL) {
    return ;
  }

  //
  // Will leave browser, check any reset required change is applied? if yes, reset system
  //
  SetupResetReminder ();

  //
  // Restore to original mode before launching boot option.
  //

  //
  // parse the selected option
  //
  Status = gBdsServices->LaunchLoadOption (gBdsServices, gOption, &ExitDataSize, &ExitData);

  if (!EFI_ERROR (Status)) {
    BdsLibBootSuccess (gOption);
  } else {
    BdsLibBootFailed (gOption, Status, ExitData, ExitDataSize);
  }
}


