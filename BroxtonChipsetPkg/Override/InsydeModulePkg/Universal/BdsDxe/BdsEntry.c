/** @file
  BdsDxe

;******************************************************************************
;* Copyright (c) 2012 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**

  This module produce main entry for BDS phase - BdsEntry.
  When this module was dispatched by DxeCore, gEfiBdsArchProtocolGuid will be installed which contains interface of BdsEntry.
  After DxeCore finish DXE phase, gEfiBdsArchProtocolGuid->BdsEntry will be invoked to enter BDS phase.

Copyright (c) 2004 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Bds.h"
#include "String.h"

///
/// BDS arch protocol instance initial value.
///
/// Note: Current BDS not directly get the BootMode, DefaultBoot,
/// TimeoutDefault, MemoryTestLevel value from the BDS arch protocol.
/// Please refer to the library useage of BdsLibGetBootMode, BdsLibGetTimeout
/// and PlatformBdsDiagnostics in BdsPlatform.c
///

EFI_HANDLE             gBdsHandle = NULL;
UINT16                 *mBootNext = NULL;

EFI_BDS_ARCH_PROTOCOL  gBds = {
  BdsEntry
};

H2O_BDS_CP_EXIT_BOOT_SERVICES_BEFORE_PROTOCOL      mExitBooServicesBefore;
EFI_EXIT_BOOT_SERVICES                             mOrginalExitBootServices;
STATIC BOOLEAN                                     mExitBootServicesHasbeenCalled;

STATIC
UINTN
GetMapKey (
  VOID
  )
{
  EFI_MEMORY_DESCRIPTOR     *EfiMemoryMap;
  UINTN                     EfiMemoryMapSize;
  UINTN                     EfiMapKey;
  UINTN                     EfiDescriptorSize;
  UINT32                    EfiDescriptorVersion;

  EfiMemoryMapSize = 0;
  EfiMemoryMap     = NULL;
  gBS->GetMemoryMap (
         &EfiMemoryMapSize,
         EfiMemoryMap,
         &EfiMapKey,
         &EfiDescriptorSize,
         &EfiDescriptorVersion
         );
  return EfiMapKey;


}

EFI_STATUS
EFIAPI
ExitBootServicesHookFunction (
  IN EFI_HANDLE   ImageHandle,
  IN UINTN        MapKey
  )
{
  UINTN      CurrentMapKey;

  if (mExitBootServicesHasbeenCalled) {
     return mOrginalExitBootServices (ImageHandle, MapKey);
  }
  CurrentMapKey = GetMapKey ();
  mExitBootServicesHasbeenCalled = TRUE;
  mExitBooServicesBefore.Size = sizeof (H2O_BDS_CP_EXIT_BOOT_SERVICES_BEFORE_PROTOCOL);
  BdsCpTrigger (&gH2OBdsCpExitBootServicesBeforeProtocolGuid, &mExitBooServicesBefore);
  return mOrginalExitBootServices (ImageHandle, CurrentMapKey == MapKey ? GetMapKey () : MapKey);
}

STATIC
UINT8
CalculateAddParity (
  UINT8         Data
  )
{
  UINTN         Index;
  UINT8         TempData;
  BOOLEAN       ClearParityBit;

  TempData = Data;

  ClearParityBit = FALSE;
  for (Index = 0; Index < 7; Index++) {
    ClearParityBit = (BOOLEAN) ((TempData & 1) ^ ClearParityBit);
    TempData >>= 1;
  }
  if (ClearParityBit) {
    Data &= 0x7f;
  } else {
    Data |= 0x80;
  }

  return Data;
}


EFI_STATUS
SetSimpBootFlag (
  VOID
  )
{
  UINT8     Data8;
  UINT8     TempData8;

  Data8 = ReadCmos8 (SimpleBootFlag);
  //
  // check BOOT bit. If BOOT bit enable, enable DIAG bit. otherwise, disable DIAG bit.
  // then enable BOOT bit      BOOT bit = Bit1  DIAG bit = bit2 SUPPERSSBOOTDISPLAY bit = bit3
  //
  TempData8 = Data8 & 0x02;
  if (TempData8 == 0) {
    Data8 &= 0xFB;
  } else {
    Data8 |= 0x04;
  }
  Data8 |= 0x02;

  //
  // disable SUPPERSSBOOTDISPLAY Flag
  //
  Data8 &= 0xF7;
  Data8 = CalculateAddParity (Data8);

  WriteCmos8 (SimpleBootFlag, Data8);

  return EFI_SUCCESS;
}


VOID
EFIAPI
LoadDefaultCheck (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINTN                           VariableSize;
  BOOLEAN                         LoadDefault;

  LoadDefault     = PcdGetBool (PcdLoadDefaultSetupMenu);

  //
  // OemServices
  //
  Status = OemSvcLoadDefaultSetupMenu (&LoadDefault);

  if (!EFI_ERROR (Status) || (!LoadDefault)) {
    return;
  }

  if (LoadDefault) {
    DxeCsSvcLoadDefaultSetupMenu ();
    //
    // Is "Setup" variable existence?
    //
    VariableSize = 0;
    Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    NULL,
                    &VariableSize,
                    NULL
                    );
    if (Status != EFI_BUFFER_TOO_SMALL) {
      //
      // No Setup variable found
      // Setup menu is default setting.
      //
      return;
    }

    //
    // Delete "Setup" variable. It will load default in GetSystemConfigurationVar() of SetupUtility.c
    //
    VariableSize = 0;
    Status = gRT->SetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    VariableSize,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
    //
    //Reset start system.
    //
    gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
  }

  return;
}

/**
  Check timeout is whether larger than PcdPlatformBootTimeOutMax.

**/
STATIC
VOID
ProcessTimeoutChange (
  VOID
  )
{
  UINT16                          Timeout;
  UINTN                           Size;
  EFI_STATUS                      Status;
  H2O_DIALOG_PROTOCOL             *H2ODialog;
  CHAR16                          *String;
  UINTN                           StringSize;
  EFI_INPUT_KEY                   Key;
  CHAR16                          *ChangeStr;

  Size = sizeof (UINT16);
  Status = gRT->GetVariable (
                  L"Timeout",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &Size,
                  &Timeout
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }
  if (Timeout <= PcdGet16 (PcdPlatformBootTimeOutMax)) {
    return;
  }
  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **) &H2ODialog
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }

  ChangeStr  = GetStringById (STRING_TOKEN (STR_TIMEOUT_CHANGE));
  if (ChangeStr == NULL) {
    return;
  }
  StringSize = 0x100 + StrSize (ChangeStr);
  String     = AllocateZeroPool (StringSize);
  if (String == NULL) {
    return;
  }
  UnicodeSPrint (String, StringSize, ChangeStr, Timeout, PcdGet16 (PcdPlatformBootTimeOutMax), PcdGet16 (PcdPlatformBootTimeOut));
  DisableQuietBoot ();
  H2ODialog->ConfirmDialog (
               DlgOk,
               FALSE,
               0,
               NULL,
               &Key,
               String
               );
  gBS->FreePool (String);
  gBS->FreePool (ChangeStr);
  Timeout = PcdGet16 (PcdPlatformBootTimeOut);
  Status = gRT->SetVariable (
                  L"Timeout",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  sizeof (UINT16),
                  &Timeout
                  );
}

/**
  Callback function to check Boot type is whether changed by other tools

  @param Event    Event whose notification function is being invoked.
  @param Context  Pointer to the notification function's context.

**/
VOID
EFIAPI
ProcessChangeCallback (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                  Status;
  VOID                        *BdsDiagnostics;

  Status = gBS->LocateProtocol (
                  &gEfiStartOfBdsDiagnosticsProtocolGuid,
                  NULL,
                  &BdsDiagnostics
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  gBS->CloseEvent (Event);
  ProcessTimeoutChange ();
}
/**

  Install Boot Device Selection Protocol

  @param ImageHandle     The image handle.
  @param SystemTable     The system table.

  @retval  EFI_SUCEESS  BDS has finished initializing.
                        Return the dispatcher and recall BDS.Entry
  @retval  Other        Return status from AllocatePool() or gBS->InstallProtocolInterface

**/
EFI_STATUS
EFIAPI
BdsInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
{
  EFI_STATUS  Status;
  VOID        *Registration;

  LoadDefaultCheck ();
  BdsLibDeleteInvalidBootOptions ();
  EfiCreateProtocolNotifyEvent (
    &gEfiStartOfBdsDiagnosticsProtocolGuid,
    TPL_CALLBACK,
    ProcessChangeCallback,
    NULL,
    &Registration
    );
  //
  // Install protocol interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gBdsHandle,
                  &gEfiBdsArchProtocolGuid,
                  &gBds,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**

  Popup a message for user to indicate no mapping boot option to boot.

  @param OptionNum          Input boot option number.

**/
VOID
CreateNoMappingBootOptionPopUp (
  IN  UINT16      OptionNum
  )
{
  H2O_DIALOG_PROTOCOL                   *H2ODialog;
  EFI_INPUT_KEY                         Key;
  EFI_STATUS                            Status;
  CHAR16                                *String;

  if (!BdsLibIsDummyBootOption (OptionNum)) {
    return;
  }

  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **) &H2ODialog
                  );
  ASSERT_EFI_ERROR (Status);

  DisableQuietBoot ();
  String = NULL;
  if (OptionNum == DummyUsbBootOptionNum) {
    String = GetStringById (STRING_TOKEN (STR_USB_OPTION_INEXISTENCE));
  } else if (OptionNum == DummyCDBootOptionNum){
    String = GetStringById (STRING_TOKEN (STR_CD_OPTION_INEXISTENCE));
  } else if (OptionNum == DummyNetwokrBootOptionNum){
    String = GetStringById (STRING_TOKEN (STR_NETWORK_OPTION_INEXISTENCE));
  }
  ASSERT (String != NULL);
  if (String == NULL) {
    return;
  }
  H2ODialog->ConfirmDialog (
                    DlgOk,
                    FALSE,
                    0,
                    NULL,
                    &Key,
                    String
                    );
  gBS->FreePool (String);

  return;
}


/**

  Popup a option list for user to select which mapping boot option want to boot.

  @param OptionNum            Input boot option number.
  @param OptionCount          The total number of boot option in option order.
  @param OptionOrder          Pointer to option order.
  @param SelectedOpitonNum    The option number which user selected.

  @retval  EFI_SUCEESS            User selects a specific USB boot option successful.
  @retval  EFI_INVALID_PARAMETER  OptionCount is 0, OptionOrder is NULL, SelectedOpitonNum is NULL or OptinNum isn't
                                  a dummy boot option.
  @retval  EFI_NOT_FOUND          Cannot find corresponding title string for specific boot option type.
  @retval  EFI_ABORTED            User presses ESC key to skip boot option selection menu.

**/
EFI_STATUS
SelectMappingBootOption (
  IN       UINT16      OptionNum,
  IN       UINTN       OptionCount,
  IN       UINT16      *OptionOrder,
  OUT      UINT16      *SelectedOpitonNum
  )
{
  H2O_DIALOG_PROTOCOL                   *H2ODialog;
  EFI_STATUS                            Status;
  CHAR16                                **DescriptionOrder;
  UINT32                                Index;
  CHAR16                                *TitleString;
  BOOLEAN                               BootOptionSelected;
  EFI_INPUT_KEY                         Key;
  UINTN                                 MaxStrLen;

  if (OptionCount == 0 || OptionOrder == NULL || SelectedOpitonNum == NULL || !BdsLibIsDummyBootOption (OptionNum)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **) &H2ODialog
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TitleString = NULL;
  if (OptionNum == DummyUsbBootOptionNum) {
    TitleString = GetStringById (STRING_TOKEN (STR_SELECT_USB_OPTION));
  } else if (OptionNum == DummyCDBootOptionNum) {
    TitleString = GetStringById (STRING_TOKEN (STR_SELECT_CD_OPTION));
  } else if (OptionNum == DummyNetwokrBootOptionNum) {
    TitleString = GetStringById (STRING_TOKEN (STR_SELECT_NETWORK_OPTION));
  }
  if (TitleString == NULL) {
    return EFI_NOT_FOUND;
  }
  DescriptionOrder = AllocateZeroPool (OptionCount * sizeof (CHAR16 *));
  if (DescriptionOrder == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  MaxStrLen = 0;
  for (Index = 0; Index < OptionCount; Index++) {
    DescriptionOrder[Index] = BdsLibGetDescriptionFromBootOption (OptionOrder[Index]);
    MaxStrLen = StrLen (DescriptionOrder[Index]) > MaxStrLen ? StrLen (DescriptionOrder[Index]) : MaxStrLen;
  }

  DisableQuietBoot ();
  BootOptionSelected = FALSE;

  H2ODialog->OneOfOptionDialog (
                    (UINT32) OptionCount,
                    FALSE,
                    NULL,
                    &Key,
                    (UINT32) MaxStrLen,
                    TitleString,
                    &Index,
                    DescriptionOrder,
                    0
                    );
  if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
    BootOptionSelected = TRUE;
    *SelectedOpitonNum = OptionOrder[Index];
  }

  gBS->FreePool (TitleString);
  for (Index = 0; Index < OptionCount; Index++) {
    gBS->FreePool (DescriptionOrder[Index]);
  }

  Status = BootOptionSelected ? EFI_SUCCESS : EFI_ABORTED;

  return Status;
}


/**
 According to boot option number to check this boot option is whether a Legacy boot option.

 @param[in] OptionNum    The boot option number.

 @retval TRUE            This is a Legacy boot option.
 @retval FALSE           This isn't a Legacy boot option.
**/
BOOLEAN
IsLegacyBootOption (
  IN  UINT16     OptionNum
  )
{
  EFI_DEVICE_PATH_PROTOCOL   *DevicePath;
  BOOLEAN                    IsLegacyBoot;
  UINT32                     DevicePathType;

  if (OptionNum == 0xFF) {
    return FALSE;
  }

  DevicePath = BdsLibGetDevicePathFromBootOption (OptionNum);
  if (DevicePath == NULL) {
    return FALSE;
  }

  IsLegacyBoot = FALSE;
  DevicePathType = BdsLibGetBootTypeFromDevicePath (DevicePath);
  if (DevicePathType == BDS_LEGACY_BBS_BOOT) {
    IsLegacyBoot = TRUE;
  }

  gBS->FreePool (DevicePath);

  return IsLegacyBoot;
}

/**
  Internal function to initialize H2O_BDS_CP_NO_BOOT_DEVICE_PROTOCOL data and trigger
  gH2OBdsCpNoBootDeviceProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpNoBootDeviceProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpNoBootDeviceProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpNoBootDevice (
  VOID
  )
{
  H2O_BDS_CP_NO_BOOT_DEVICE_PROTOCOL    *NoBootDevice;

  NoBootDevice = AllocateZeroPool (sizeof (H2O_BDS_CP_NO_BOOT_DEVICE_PROTOCOL));
  if (NoBootDevice == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NoBootDevice->Size   = sizeof (H2O_BDS_CP_NO_BOOT_DEVICE_PROTOCOL);
  NoBootDevice->Status = H2O_BDS_TASK_NORMAL;
  return BdsCpTrigger (&gH2OBdsCpNoBootDeviceProtocolGuid, NoBootDevice);
}

/**
  Show boot fail message. If auto failover is disable, it will go into firmware UI.

  @param  AutoFailover           Auto failover polciy
  @param  Message                Boot fail message

  @retval EFI_SUCCESS            Perform auto failover policy success.
  @retval EFI_INVALID_PARAMETER  String pointer is NULL.
  @retval Other                  Locate protocol fail or pop message fail.

**/
EFI_STATUS
AutoFailoverPolicyBehavior (
  IN BOOLEAN    AutoFailover,
  IN CHAR16     *Message
  )
{
  EFI_STATUS                        Status;
  EFI_INPUT_KEY                     Key;
  H2O_DIALOG_PROTOCOL               *H2ODialog;
  UINT16                            Timeout;
  LIST_ENTRY                        BootLists;

  if (Message == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (
                  &gH2ODialogProtocolGuid,
                  NULL,
                  (VOID **)&H2ODialog
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  DisableQuietBoot ();

  Status = H2ODialog->ConfirmDialog (2, FALSE, 0, NULL, &Key, Message);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  gST->ConOut->ClearScreen (gST->ConOut);

  //
  // If auto failover is disable, go into firmware UI.
  //
  if (!AutoFailover) {
    InitializeListHead (&BootLists);
    BdsLibConnectAll ();
    BdsLibEnumerateAllBootOption (TRUE, &BootLists);

    if (FeaturePcdGet(PcdFrontPageSupported)) {
      Timeout = 0xffff;
      PlatformBdsEnterFrontPage (Timeout, FALSE);
    } else {
      CallBootManager ();
    }
  }

  return EFI_SUCCESS;
}

/**
  Perform no boot device operation (show message and wait for user to restart system)
**/
VOID
BdsNoBootDevice (
  VOID
  )
{
  EFI_STATUS                         Status;
  EFI_INPUT_KEY                      Key;
  CHAR16                             *BootErrorString;
  UINTN                              StringLen;
  H2O_BDS_CP_NO_BOOT_DEVICE_PROTOCOL *NoBootDevice;
  volatile UINTN                     Index;
  //
  // PostCode = 0xF9, No Boot Device
  //
  POST_CODE (POST_BDS_NO_BOOT_DEVICE);
  TriggerCpNoBootDevice ();
  Status = gBS->LocateProtocol (
                  &gH2OBdsCpNoBootDeviceProtocolGuid,
                  NULL,
                  (VOID **) &NoBootDevice
                  );
  if (!EFI_ERROR (Status) && NoBootDevice->Status == H2O_BDS_TASK_SKIP) {
    CpuDeadLoop ();
  }

  BootErrorString = GetStringById (STRING_TOKEN (STR_AUTO_FAILOVER_NO_BOOT_DEVICE));
  if (BootErrorString != NULL) {
    AutoFailoverPolicyBehavior (FALSE, BootErrorString);
    gBS->FreePool(BootErrorString);
  }

  DisableQuietBoot ();

  StringLen = 0;
  BootErrorString = GetStringById (STRING_TOKEN (STR_BOOT_DEVICE_ERROR_MESSAGE));
  if (BootErrorString != NULL) {
    StringLen = StrLen (BootErrorString);
  }
  if (BootErrorString == NULL || StringLen == 0) {
    BootErrorString = L"No bootable device -- Please restart system\n";
  }

  gST->ConOut->EnableCursor (gST->ConOut, TRUE);
  gST->ConOut->ClearScreen(gST->ConOut);
  gST->ConOut->SetAttribute (gST->ConOut, EFI_WHITE);
  Print (BootErrorString);

  for (Index = 0; Index == 0;) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (!EFI_ERROR (Status)) {
      Print (BootErrorString);
    }
  }
}

/**
  This function attempts to boot for the boot order specified
  by platform policy.
**/
VOID
BdsBootDeviceSelect (
  VOID
  )
{
  EFI_STATUS                    Status;
  LIST_ENTRY                    *Link;
  H2O_BDS_LOAD_OPTION           *BootOption;
  H2O_BDS_LOAD_OPTION           *BootNextOption;
  UINTN                         ExitDataSize;
  CHAR16                        *ExitData;
  UINT16                        Timeout;
  LIST_ENTRY                    *BootLists;
  LIST_ENTRY                    DefautlBootLists;
  LIST_ENTRY                    CommonBootList;
  CHAR16                        Buffer[20];
  BOOLEAN                       BootNextExist;
  LIST_ENTRY                    *LinkBootNext;
  KERNEL_CONFIGURATION          SystemConfiguration;
  EFI_STATUS                    SystemConfigStatus;
  EFI_BOOT_MODE                 BootMode;
  UINT8                         CmosData = 0;
  CHAR16                        *String;
  BOOLEAN                       AlreadyConnectAll;
  BOOLEAN                       LinkUpdated;
  UINTN                         MappingOpitonCount;
  UINT16                        *MappingOpitonOrder;
  UINT16                        SelectedOptionNum;
  UINTN                         VariableSize;
  VOID                          *VariablePtr;
  H2O_BDS_LOAD_OPTION           *TempBootOption;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  BOOLEAN                        BootDefaultOption;
  H2O_BDS_LOAD_OPTION           *LoadOption;

  //
  // PostCode = 0x2B, Try to boot system to OS
  //
  POST_CODE (BDS_BOOT_DEVICE_SELECT);

  Status = BdsLibGetBootMode (&BootMode);
  if (EFI_ERROR (Status)) {
    return;
  }

  //
  // Got the latest boot option
  //
  BootNextExist     = FALSE;
  LinkBootNext      = NULL;
  BootNextOption    = NULL;
  AlreadyConnectAll = FALSE;
  InitializeListHead (&CommonBootList);

  //
  // First check the boot next option
  //
  ZeroMem (Buffer, sizeof (Buffer));

  if (mBootNext == NULL) {
    //
    // Check if we have the boot next option created during BDS
    //
    mBootNext = BdsLibGetVariableAndSize (
                  L"BootNext",
                  &gEfiGlobalVariableGuid,
                  &VariableSize
                  );
  }

  if (gBdsServices->BootNextLoadOption == NULL) {
    //
    // Check if we have the boot next option created during BDS
    //
    Status = gBdsServices->ConvertVarToLoadOption (gBdsServices, EFI_BOOT_NEXT_VARIABLE_NAME, &gEfiGlobalVariableGuid, &LoadOption);
    if (Status == EFI_SUCCESS) {
      gBdsServices->BootNextLoadOption = LoadOption;
    }
  }
  gRT->SetVariable (
         EFI_BOOT_NEXT_VARIABLE_NAME,
         &gEfiGlobalVariableGuid,
         0,
         0,
         NULL
         );
  if (gBdsServices->BootNextLoadOption && mBootNext != NULL) {
    //
    // Indicate we have the boot next variable, so this time
    // boot will always have this boot option
    //
    BootNextExist = TRUE;
    if (BdsLibIsDummyBootOption (*mBootNext)) {
      Status = BdsLibGetMappingBootOptions (*mBootNext, &MappingOpitonCount, &MappingOpitonOrder);
      if (Status == EFI_SUCCESS) {
        if (MappingOpitonCount == 0) {
          //
          // Show no USB boot option message for user and then go to Boot Manager menu for user to select boot option.
          //
          CreateNoMappingBootOptionPopUp (*mBootNext);
          VariableSize = 0;
          VariablePtr = NULL;
          Status  = gRT->GetVariable (L"BootOrder", &gEfiGlobalVariableGuid, NULL, &VariableSize, VariablePtr);
          if (Status == EFI_BUFFER_TOO_SMALL) {
            CallBootManager ();
           if (FeaturePcdGet(PcdFrontPageSupported)) {
             Timeout = 0xffff;
             PlatformBdsEnterFrontPage (Timeout, TRUE);
          }
         }
          BootNextExist = FALSE;
        } else if (MappingOpitonCount == 1) {
          //
          // System has one mapping boot option, boot this boot option direclty.
          //
          UnicodeSPrint (Buffer, sizeof (Buffer), L"Boot%04x", MappingOpitonOrder[0]);
          gBdsServices->ConvertVarToLoadOption (gBdsServices, Buffer, &gEfiGlobalVariableGuid, &BootNextOption);
        } else {
          //
          // If system has more than 1 mapping boot option, let user to select specific mapping boot option.
          //
          Status = SelectMappingBootOption (*mBootNext, MappingOpitonCount, MappingOpitonOrder, &SelectedOptionNum);
          if (!EFI_ERROR (Status)) {
            UnicodeSPrint (Buffer, sizeof (Buffer), L"Boot%04x", SelectedOptionNum);
            gBdsServices->ConvertVarToLoadOption (gBdsServices, Buffer, &gEfiGlobalVariableGuid, &BootNextOption);
          } else {
            BootNextExist = FALSE;
          }
        }
        if (MappingOpitonOrder != NULL) {
          gBS->FreePool (MappingOpitonOrder);
        }
      }
    } else {
      //
      // Add the boot next boot option
      //
      UnicodeSPrint (Buffer, sizeof (Buffer), L"Boot%04x", *mBootNext);
      gBdsServices->ConvertVarToLoadOption (gBdsServices, Buffer, &gEfiGlobalVariableGuid, &BootNextOption);
      //
      // Need expand load option to prevent from the boot option is windows to GO, short form device path, USB
      // WWID or USB class device path.
      //
      gBdsServices->ExpandLoadOption (gBdsServices, BootNextOption);
    }
  } else if (BootMode == BOOT_ON_S4_RESUME) {
    CmosData = ReadCmos8 (LastBootDevice);
    if (IsLegacyBootOption ((UINT16)CmosData)) {
      //
      // Add the boot next boot option
      //
      UnicodeSPrint (Buffer, sizeof (Buffer), L"Boot%04x", ReadCmos8 (LastBootDevice));
      gBdsServices->ConvertVarToLoadOption (gBdsServices, Buffer, &gEfiGlobalVariableGuid, &BootNextOption);
    }
  }

  BootDefaultOption = FALSE;
  gBdsServices->GetBootList (gBdsServices, &BootLists);

  if (IsListEmpty (BootLists)) {
    InitializeListHead (&DefautlBootLists);
    AddDefaultBootOptionsToBootList (&DefautlBootLists);
    BootLists = &DefautlBootLists;
    BootDefaultOption = TRUE;
  }

  if (IsListEmpty (BootLists)) {
    BdsNoBootDevice ();
  }

  Link = GetFirstNode (BootLists);

  SystemConfigStatus = GetKernelConfiguration (&SystemConfiguration);

  // Here we make the boot in a loop, every boot success will
  // return to the front page
  //
  for (;;) {
    //
    // Check the boot option list first
    //
    if (Link == BootLists && BootNextOption == NULL) {
      //
      // There are two ways to enter here:
      // 1. There is no active boot option, give user chance to
      //    add new boot option
      // 2. All the active boot option processed, and there is no
      //    one is success to boot, then we back here to allow user
      //    add new active boot option
      //
      String = GetStringById (STRING_TOKEN (STR_AUTO_FAILOVER_NO_BOOT_DEVICE));
      if (String != NULL) {
        AutoFailoverPolicyBehavior (TRUE, String);
        gBS->FreePool(String);
      }

      Timeout = 0xffff;
      if (FeaturePcdGet(PcdFrontPageSupported)) {
        PlatformBdsEnterFrontPage (Timeout, FALSE);
      } else {
        CallBootManager ();
      }
      gBdsServices->GetBootList (gBdsServices, &BootLists);
      Link = GetFirstNode (BootLists);
      continue;
    }
    if (BootNextOption != NULL) {
      BootOption = BootNextOption;
      BootNextOption = NULL;
    } else {
      //
      // Get the boot option from the link list
      //
      BootOption = BDS_OPTION_FROM_LINK (Link);
    }

    //
    // According to EFI Specification, if a load option is not marked
    // as LOAD_OPTION_ACTIVE, the boot manager will not automatically
    // load the option.
    //
    if (!IS_LOAD_OPTION_TYPE (BootOption->Attributes, LOAD_OPTION_ACTIVE)) {
      //
      // skip the header of the link list, because it has no boot option
      //
      Link = Link->ForwardLink;
      continue;
    }
    //
    // Make sure the boot option device path connected,
    // but ignore the BBS device path
    //
    if (DevicePathType (BootOption->DevicePath) != BBS_DEVICE_PATH) {
      //
      // Notes: the internal shell can not been connected with device path
      // so we do not check the status here
      //
      BdsLibConnectDevicePath (BootOption->DevicePath);
    }

    //
    // All the driver options should have been processed since
    // now boot will be performed.
    //

    Status = gBdsServices->LaunchLoadOption (gBdsServices, BootOption, &ExitDataSize, &ExitData);
    LinkUpdated = FALSE;
    if (BdsLibIsWin8FastBootActive () && !AlreadyConnectAll && !BootDefaultOption) {
      //
      // create new boot option to prevent original boot option is freed by calling gBdsServices->GetBootList ()
      // to synchronize BootList to BootOrder variable.
      //
      gBdsServices->ConvertVarToLoadOption (gBdsServices, BootOption->LoadOrderVarName, &BootOption->LoadOrderVarGuid, &BootOption);
      //
      // For Win 8 Fast Boot, do one time ConnectAll to connect all boot devices and point Link to next boot device.
      //
      AlreadyConnectAll = TRUE;
      InitializeListHead (&CommonBootList);
      BdsLibConnectUsbHID ();
      BdsLibConnectAll ();
      BdsLibEnumerateAllBootOption (TRUE, &CommonBootList);
      gBdsServices->GetBootList (gBdsServices, &BootLists);
      //
      // If last boot is fail, update Link for next boot.
      //
      if (EFI_ERROR (Status)) {
        Link = GetFirstNode (BootLists);
        if (!IsListEmpty (BootLists)) {
          //
          // If next boot device path is the same as previous one, skip to next.
          //
          DevicePath = BootOption->DevicePath;
          TempBootOption = BDS_OPTION_FROM_LINK (Link);
          if (CompareMem (DevicePath, TempBootOption->DevicePath, GetDevicePathSize (DevicePath)) == 0) {
            Link = Link->ForwardLink;
          }
        }
        LinkUpdated = TRUE;
      }

      if (EFI_ERROR (Status) && !FeaturePcdGet(PcdWin8FastBootErrorMessageSupported)) {
        continue;
      }
    }

    if (Status != EFI_SUCCESS) {
      //
      // Call platform action to indicate the boot fail
      //
      // If boot default device (not including Windows To Go) fail and Auto Failover is disable, go into firmware UI.
      //
      if (!EFI_ERROR (SystemConfigStatus) &&
          SystemConfiguration.AutoFailover == 0 &&
          !((DevicePathType (BootOption->DevicePath) == MESSAGING_DEVICE_PATH) &&
            (DevicePathSubType (BootOption->DevicePath) == MSG_USB_CLASS_DP))) {
        String = GetStringById (STRING_TOKEN (STR_AUTO_FAILOVER_DISABLE_BOOT_FAIL));
        if (String != NULL) {
          AutoFailoverPolicyBehavior (FALSE, String);
          gBS->FreePool(String);
        }
      } else {
        if (!(((DevicePathType (BootOption->DevicePath) == MESSAGING_DEVICE_PATH) &&
               (DevicePathSubType (BootOption->DevicePath) == MSG_USB_CLASS_DP)) &&
              Status != EFI_SECURITY_VIOLATION)) {
          BdsLibBootFailed (BootOption, Status, ExitData, ExitDataSize);
        }
      }

      if (!LinkUpdated) {
        Link = Link->ForwardLink;
      }
    } else {
      //
      // Call platform action to indicate the boot success
      //
      BdsLibBootSuccess (BootOption);

      //
      // Boot success, then stop process the boot order, and
      // present the boot manager menu, front page
      //
      if (FeaturePcdGet(PcdFrontPageSupported)) {
        Timeout = 0xffff;
        InitializeListHead (&CommonBootList);
        BdsLibEnumerateAllBootOption (FALSE, &CommonBootList);
        gBdsServices->GetBootList (gBdsServices, &BootLists);
        PlatformBdsEnterFrontPage (Timeout, FALSE);
      } else {
        CallBootManager ();
      }
      gBdsServices->GetBootList (gBdsServices, &BootLists);
      Link = GetFirstNode (BootLists);
    }
  }
}

/**

  Allocate a block of memory that will contain performance data to OS.

**/
VOID
BdsAllocateMemoryForPerformanceData (
  VOID
  )
{
  EFI_STATUS                    Status;
  EFI_PHYSICAL_ADDRESS          AcpiLowMemoryBase;
  EDKII_VARIABLE_LOCK_PROTOCOL  *VariableLock;

  AcpiLowMemoryBase = 0x0FFFFFFFFULL;

  //
  // Allocate a block of memory that will contain performance data to OS.
  //
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiReservedMemoryType,
                  EFI_SIZE_TO_PAGES (PERF_DATA_MAX_LENGTH),
                  &AcpiLowMemoryBase
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Save the pointer to variable for use in S3 resume.
    //
    Status = gRT->SetVariable (
               L"PerfDataMemAddr",
               &gEfiGenericVariableGuid,
               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
               sizeof (UINT32),
               &AcpiLowMemoryBase
               );
    ASSERT_EFI_ERROR (Status);
    //
    // Mark L"PerfDataMemAddr" variable to read-only if the Variable Lock protocol exists
    //
    Status = gBS->LocateProtocol (&gEdkiiVariableLockProtocolGuid, NULL, (VOID **) &VariableLock);
    if (!EFI_ERROR (Status)) {
      Status = VariableLock->RequestToLock (VariableLock, L"PerfDataMemAddr", &gEfiGenericVariableGuid);
      ASSERT_EFI_ERROR (Status);
    }
  }
}

EFI_STATUS
LockSensitiveVariables (
  VOID
  )
{
  EFI_STATUS                    Status;
  EDKII_VARIABLE_LOCK_PROTOCOL  *VariableLock;


  //
  // Lock setup sensitive variables
  //
  Status = gBS->LocateProtocol (&gEdkiiVariableLockProtocolGuid, NULL, (VOID **) &VariableLock);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = VariableLock->RequestToLock (VariableLock, L"Setup", &gSystemConfigurationGuid);
  Status = VariableLock->RequestToLock (VariableLock, SECURE_FLASH_INFORMATION_NAME, &gSecureFlashInfoGuid);
  Status = VariableLock->RequestToLock (VariableLock, ACPI_GLOBAL_VARIABLE, &gAcpiVariableSetGuid);
  Status = VariableLock->RequestToLock (VariableLock, EFI_ADMINISTER_SECURE_BOOT_NAME, &gEfiGenericVariableGuid);
  Status = VariableLock->RequestToLock (VariableLock, EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME, &gEfiMemoryTypeInformationGuid);
  return Status;
}
/**

  Hook Boot Services ExitBootServices ().

**/
STATIC
VOID
HookExitBootServices (
  VOID
  )
{
  UINT32                          Crc;

  mOrginalExitBootServices = gBS->ExitBootServices;
  gBS->ExitBootServices = ExitBootServicesHookFunction;
  gBS->Hdr.CRC32 = 0;
  Crc = 0;
  gBS->CalculateCrc32 ((VOID *) gBS, gBS->Hdr.HeaderSize, &Crc);
  gBS->Hdr.CRC32 = Crc;
}

/**

  Initialize firmware vendor and firmware version in system table.

**/
EFI_STATUS
InitFirmwareInfo (
  VOID
  )
{
  CHAR16                          *FirmwareVendor;
  UINT32                          Crc;
  //
  // Fill in FirmwareVendor and FirmwareRevision from PCDs
  //
  FirmwareVendor = (CHAR16 *) PcdGetPtr (PcdFirmwareVendor);
  gST->FirmwareVendor = AllocateRuntimeCopyPool (StrSize (FirmwareVendor), FirmwareVendor);
  ASSERT (gST->FirmwareVendor != NULL);
  if (gST->FirmwareVendor == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  gST->FirmwareRevision = PcdGet32 (PcdFirmwareRevision);
  DEBUG ((EFI_D_ERROR, "EFI Vendor : %s\nEFI Revision : %d.%d", gST->FirmwareVendor, gST->FirmwareRevision >> 16, gST->FirmwareRevision & 0xFFFF));
  //
  // Fixup Tasble CRC after we updated Firmware Vendor and Revision
  //
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 ((VOID *)gST, gST->Hdr.HeaderSize, &Crc);
  gST->Hdr.CRC32 = Crc;

  //
  // Hook ExitBootServices () to trigger H2O_BDS_CP_EXIT_BOOT_SERVICES_BEFORE_PROTOCOL
  // check point before entering ExitBootServices ().
  //
  HookExitBootServices ();
  return EFI_SUCCESS;
}

/**
  An empty function to pass error checking of CreateEventEx ().

  This empty function ensures that EVT_NOTIFY_SIGNAL_ALL is error
  checked correctly since it is now mapped into CreateEventEx() in UEFI 2.0.

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.

**/
VOID
EFIAPI
EmptyFunction (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  return;
}

/**
  Internal function to initialize  H2O_BDS_CP_INIT_PROTOCOL data and trigger gH2OBdsCpInitProtocolGuid
  checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpInitProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpInitProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpInit (
  VOID
  )
{

  H2O_BDS_CP_INIT_PROTOCOL   *Init;

  Init = AllocateZeroPool (sizeof (H2O_BDS_CP_INIT_PROTOCOL));
  if (Init == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Init->Size = sizeof (H2O_BDS_CP_INIT_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpInitProtocolGuid, Init);
}

/**
  Internal function to initialize  _H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL data and trigger gH2OBdsCpEndOfDxeBeforeProtocolGuid
  checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpEndOfDxeBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpEndOfDxeBeforeProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpEndOfDxeBefore (
  VOID
  )
{
  H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL   *BeforeEndOfDxe;

  BeforeEndOfDxe = AllocateZeroPool (sizeof (H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL));
  if (BeforeEndOfDxe == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  BeforeEndOfDxe->Size = sizeof (H2O_BDS_CP_END_OF_DXE_BEFORE_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpEndOfDxeBeforeProtocolGuid, BeforeEndOfDxe);
}

/**
  Internal function to initialize H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL data and trigger
  gH2OBdsCpEndOfDxeAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpEndOfDxeAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpEndOfDxeAfterProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpEndOfDxeAfter (
  VOID
  )
{

  H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL   *AfterEndOfDxe;

  AfterEndOfDxe = AllocateZeroPool (sizeof (H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL));
  if (AfterEndOfDxe == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  AfterEndOfDxe->Size = sizeof (H2O_BDS_CP_END_OF_DXE_AFTER_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpEndOfDxeAfterProtocolGuid, AfterEndOfDxe);

}

/**
  Internal function to initialize H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL data and trigger
  gH2OBdsCpDriverConnectAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpDriverConnectAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpDriverConnectAfterProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpDriverConnectAfter (
  VOID
  )
{
  H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL   *AfterConnectDriver;

  AfterConnectDriver = AllocateZeroPool (sizeof (H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL));
  if (AfterConnectDriver == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  AfterConnectDriver->Size = sizeof (H2O_BDS_CP_DRIVER_CONNECT_AFTER_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpDriverConnectAfterProtocolGuid, AfterConnectDriver);

}

/**
  Internal function to initialize H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL data and trigger
  gH2OBdsCpDriverConnectBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpDriverConnectBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpDriverConnectBeforeProtocolGuid
                                  checkpoint.
**/
EFI_STATUS
TriggerCpDriverConnectBefore (
  VOID
  )
{
  H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL   *BeforeConnectDriver;

  BeforeConnectDriver = AllocateZeroPool (sizeof (H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL));
  if (BeforeConnectDriver == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  BeforeConnectDriver->Size = sizeof (H2O_BDS_CP_DRIVER_CONNECT_BEFORE_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpDriverConnectBeforeProtocolGuid, BeforeConnectDriver);
}


/**

  Service routine for BdsInstance->Entry(). Devices are connected, the
  consoles are initialized, and the boot options are tried.

  @param This             Protocol Instance structure.

**/
VOID
EFIAPI
BdsEntry (
  IN EFI_BDS_ARCH_PROTOCOL  *This
  )
{
  LIST_ENTRY                               DriverOptionList;
  LIST_ENTRY                               BootOptionList;
  UINTN                                    Size;
  EFI_STATUS                               Status;
  UINT16                                   Timeout;
  EFI_TIMER_ARCH_PROTOCOL                  *Timer;
  EFI_EVENT                                EndOfDxeEvent;
  //
  // PostCode = 0x10, Enter BDS entry
  //
  POST_CODE (BDS_ENTER_BDS);
  //
  // Insert the performance probe
  //
  PERF_END (0, DXE_TOK, NULL, 0);
  PERF_START (0, BDS_TOK, NULL, 0);

  TriggerCpInit ();
  PERF_CODE (
    BdsAllocateMemoryForPerformanceData ();
  );
  LockSensitiveVariables ();
  //
  // Initialize the global system boot option and driver option
  //
  InitializeListHead (&DriverOptionList);
  InitializeListHead (&BootOptionList);

  //
  // Set the timer tick to 100hz to make whole timer interrupt handler operate in desire frequency
  //
  Status = gBS->LocateProtocol (&gEfiTimerArchProtocolGuid, NULL, (VOID **) &Timer);
  if (!EFI_ERROR (Status)) {
    Timer->SetTimerPeriod (Timer, 100000);
  }

  if (PcdGetBool (PcdEndOfDxeEventSupported)) {
    //
    // According to PI 1.2.1, signal EndOfDxe at end of DXE
    //
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    EmptyFunction,
                    NULL,
                    &gEfiEndOfDxeEventGroupGuid,
                    &EndOfDxeEvent
                    );
    if (!EFI_ERROR (Status)) {
      TriggerCpEndOfDxeBefore ();
      gBS->SignalEvent (EndOfDxeEvent);
      TriggerCpEndOfDxeAfter ();
      gBS->CloseEvent (EndOfDxeEvent);
      DEBUG((EFI_D_INFO,"All EndOfDxe callbacks have returned successfully\n"));
    }
  }
  //
  // Do the platform init, can be customized by OEM/IBV
  //
  PERF_START (NULL, "PlatformBds", "BDS", 0);
  InitFirmwareInfo ();
  PlatformBdsInit ();
  InstallH2OBdsServicesProtocol ();
  //
  // bugbug: platform specific code
  // Initialize the platform specific string and language
  //
  InitializeStringSupport ();
  //
  // BdsLibGetTimeout() will initialize Timeout variable if variable is not exist.
  //
  Size = sizeof (Timeout);
  Status = gRT->GetVariable (
                  L"Timeout",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &Size,
                  &Timeout
                  );
  if (EFI_ERROR (Status)) {
    Timeout = (UINT16) PcdGet16 (PcdPlatformBootTimeOut);
    Status = gRT->SetVariable (
                    L"Timeout",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof (Timeout),
                    &Timeout
                    );
  }

  UpdateBootMessage ();

  //
  // Set up the device list based on EFI 1.1 variables
  // process Driver#### and Load the driver's in the
  // driver option list
  //
  TriggerCpDriverConnectBefore ();
  BdsLibLaunchDrivers ();
  TriggerCpDriverConnectAfter ();
  //
  // Check if we have the boot next option
  //
  mBootNext = BdsLibGetVariableAndSize (
                L"BootNext",
                &gEfiGlobalVariableGuid,
                &Size
                );

  SetSimpBootFlag ();
  if (BdsLibGetBootType () == LEGACY_BOOT_TYPE) {
    BdsLibSkipEbcDispatch ();
  }

  //
  // Setup some platform policy here
  //
  PlatformBdsPolicyBehavior (&DriverOptionList, &BootOptionList, BdsProcessCapsules, BdsMemoryTest);
  PERF_END (NULL, "PlatformBds", "BDS", 0);

  //
  // BDS select the boot device to load OS
  //
  BdsBootDeviceSelect ();

  //
  // Only assert here since this is the right behavior, we should never
  // return back to DxeCore.
  //
  ASSERT (FALSE);

  return ;
}

