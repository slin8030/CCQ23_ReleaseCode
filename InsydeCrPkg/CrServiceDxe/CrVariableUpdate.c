/** @file
  CrVariableUpdate

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "CrVariableUpdate.h"

CHAR16                       *mConInVarName  = L"ConIn";
CHAR16                       *mConOutVarName = L"ConOut";
EFI_GUID                     *mConVarGuid    = &gEfiGlobalVariableGuid;
CR_DEVICES_SETTING_HEAD_NODE mCrVarDeviceHead;


VOID
InitializeCrDeviceList (
  VOID
  )
{
  InitializeListHead (&mCrVarDeviceHead.Link);
  mCrVarDeviceHead.CrDevVarCount = 0;
}

/**
  Status is Exist and Port enable

  @param

  @retval

**/
EFI_DEVICE_PATH*
BuildCrActiveDevicePath (
  VOID
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *DevPath;
  EFI_DEVICE_PATH_PROTOCOL    *PrevDevPath;
  UINTN                       DeviceCount;

  LIST_ENTRY                  *Link;
  CR_DEVICES_SETTING_NODE     *CrDevNode;

  DevPath     = NULL;
  PrevDevPath = NULL;
  DeviceCount = 0;

  Link = GetFirstNode (&mCrVarDeviceHead.Link);

  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);
    //
    // **Note: The network device is independent control.
    //         So didn't add device path here.
    //
    if ( CrDevNode->CrSetting->Exist                == FALSE   ||
         CrDevNode->CrSetting->Attribute.PortEnable == FALSE
         ) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }
    DevPath = AppendDevicePathInstance (PrevDevPath, GetDevicePathFromCrDevSetting (CrDevNode->CrSetting));
    CrSafeFreePool (PrevDevPath);
    PrevDevPath = DevPath;

    CrDevNode->AddIntoConVar = TRUE;
    Link                     = GetNextNode (&mCrVarDeviceHead.Link, Link);
  }
  return DevPath;
}

BOOLEAN
IsDevNodeInvalidOrAreadyUsed (
  IN CR_DEVICES_SETTING_NODE     *CrDevNode
  )
{
  if (CrDevNode->CrSetting->ItemType == ASTERISK_ITEM  ||
      CrDevNode->CrSetting->ItemType == INVALID_ITEM   ||
      CrDevNode->AddIntoConVar       == TRUE
    ) {
    return TRUE;
  }
  return FALSE;
}

VOID
UpdateDevicesExistStatus (
  VOID
  )
{
  LIST_ENTRY                  *Link;
  CR_DEVICES_SETTING_NODE     *CrDevNode;
  EFI_DEVICE_PATH             *DevPath;
  EFI_HANDLE                  Handle;
  EFI_STATUS                  Status;

  DEBUG ((DEBUG_INFO, "\nCr << %a >>\n", __FUNCTION__));

  Link   = GetFirstNode (&mCrVarDeviceHead.Link);
  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);

    if (IsDevNodeInvalidOrAreadyUsed (CrDevNode) ) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }

    DevPath = GetDevicePathFromCrDevSetting (CrDevNode->CrSetting);
    Status = gBS->LocateDevicePath (&gH2OUart16550AccessProtocolGuid, &DevPath, &Handle);

    DEBUG ((DEBUG_INFO, " Not used CrDevVar%d : %s, Locate 16550:%r\n", CrDevNode->VarIndex, CrDevNode->CrSetting->DevName, Status));

    if (Status == EFI_SUCCESS && CrDevNode->CrSetting->Exist == FALSE) {
      DEBUG ((DEBUG_INFO, "  ->Now appear\n"));
      CrDevNode->CrSetting->Exist = TRUE;
      UpdateOrDeleteCrDevVarWithDevInfo (CrDevNode->VarIndex ,CrDevNode->CrSetting);

    } else if (EFI_ERROR(Status) && CrDevNode->CrSetting->Exist == TRUE){
      DEBUG ((DEBUG_INFO, "  ->Now disappear\n"));
      CrDevNode->CrSetting->Exist = FALSE;
      UpdateOrDeleteCrDevVarWithDevInfo (CrDevNode->VarIndex ,CrDevNode->CrSetting);
    }

    Link = GetNextNode (&mCrVarDeviceHead.Link, Link);
  }
}

VOID
CheckAndSaveConnectResult (
  IN  CHAR16                    *ConVarName,
  IN  EFI_GUID                  *ConVarGuid
  )
{
  LIST_ENTRY                  *Link;
  CR_DEVICES_SETTING_NODE     *CrDevNode;
  EFI_DEVICE_PATH             *VarConsole;

  DEBUG ((DEBUG_INFO, "\nCr << %a >>\n", __FUNCTION__));

  VarConsole = NULL;
  VarConsole = CommonGetVariableData (ConVarName, ConVarGuid);
  if (VarConsole == NULL) {
    return;
  }

  Link = GetFirstNode (&mCrVarDeviceHead.Link);
  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);

    if (CrDevNode->AddIntoConVar == FALSE) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }

    if (!IsInclusiveOfSelectDevPath (VarConsole, GetDevicePathFromCrDevSetting (CrDevNode->CrSetting))) {
      DEBUG ((EFI_D_INFO, " CrDevVar%d :%s not exist\n", CrDevNode->VarIndex, CrDevNode->CrSetting->DevName));
      CrDevNode->AddIntoConVar                   = FALSE;
      CrDevNode->CrSetting->Exist                = FALSE;
      CrDevNode->CrSetting->Attribute.PortEnable = FALSE;
      //
      // If device not exist and this device not in PCD, then delete this variable
      //
      UpdateOrDeleteCrDevVarWithDevInfo (CrDevNode->VarIndex ,CrDevNode->CrSetting);
    }

    Link = GetNextNode (&mCrVarDeviceHead.Link, Link);
  }
  CrSafeFreePool (VarConsole);
}

VOID
CreateCrDeviceVarList (
  VOID
  )
{
  UINT8                       Index;
  CR_DEVICE_SETTING           *CrSetting;
  CR_DEVICES_SETTING_NODE     *Node;
  EFI_STATUS                  Status;

  DEBUG ((DEBUG_INFO, "\nCr << %a >>\n", __FUNCTION__));

  if (!mCrEnable) {
    mCrVarDeviceHead.CrDevVarCount = 0;
    return;
  }

  for (Index = 0; Index < MAX_CR_DEVICE_VARIABLE; Index++) {
    Status = GetCrDevInfoFromVariable (Index, &CrSetting);
    if (EFI_ERROR(Status) || CrSetting == NULL) {
      continue;
    }

    Node = AllocateZeroPool (sizeof (CR_DEVICES_SETTING_NODE));
    if (Node == NULL) {
      CrSafeFreePool (CrSetting);
      ASSERT (0);
      break;
    }

    Node->CrSetting = CrSetting;
    Node->VarIndex  = Index;
    InsertTailList (&mCrVarDeviceHead.Link, &Node->Link);
    mCrVarDeviceHead.CrDevVarCount ++;
  }
}

VOID
FreeCrDevicesVarList (
  VOID
  )
{
  LIST_ENTRY                  *Link;
  CR_DEVICES_SETTING_NODE     *CrDevNode;

  Link = GetFirstNode (&mCrVarDeviceHead.Link);

  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);
    Link      = RemoveEntryList (Link);
    gBS->FreePool (CrDevNode->CrSetting);
    gBS->FreePool (CrDevNode);
  }
}

VOID
CreateNewActiveCrDevice (
  VOID
  )
{
  EFI_DEVICE_PATH   *NewCrActiveDevice;
  UINTN             Size;

  NewCrActiveDevice = NULL;
  NewCrActiveDevice = BuildCrActiveDevicePath ();

  Size = (NewCrActiveDevice == NULL)? 0 : GetDevicePathSize (NewCrActiveDevice);

  CommonSetVariable (
    CR_ACTIVE_VARIABLE_NAME,
    &gCrConfigurationGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    Size,
    NewCrActiveDevice
    );
  CrSafeFreePool (NewCrActiveDevice);
}

BOOLEAN
IsChildDevicePath (
  IN EFI_DEVICE_PATH  *ChildDevPath,
  IN EFI_DEVICE_PATH  *ParentDevPath
  )
{
  UINTN  Size;

  Size = GetDevicePathSize (ParentDevPath) - sizeof (EFI_DEVICE_PATH);

  if (CompareMem (ChildDevPath, ParentDevPath, Size) == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

BOOLEAN
IsDevicePathAlreadyInCrDevVar (
  IN EFI_DEVICE_PATH  *SubsetPath
  )
{
  LIST_ENTRY                  *Link;
  CR_DEVICES_SETTING_NODE     *CrDevNode;

  Link = GetFirstNode (&mCrVarDeviceHead.Link);

  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);

    if (CrDevNode->CrSetting->ItemType == INVALID_ITEM  ||
        CrDevNode->CrSetting->ItemType == ASTERISK_ITEM
        ) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }

    if (IsChildDevicePath (GetDevicePathFromCrDevSetting (CrDevNode->CrSetting), SubsetPath)) {
      return TRUE;
    }
    Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
  }
  return FALSE;
}

BOOLEAN
IsChildDevPathOfAsteriskDevPath (
  IN  EFI_DEVICE_PATH *ChildDevPath
  )
{
  LIST_ENTRY                  *Link;
  CR_DEVICES_SETTING_NODE     *CrDevNode;

  Link = GetFirstNode (&mCrVarDeviceHead.Link);

  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);

    if (CrDevNode->CrSetting->ItemType == ASTERISK_ITEM) {
      return IsChildDevicePath (ChildDevPath, GetDevicePathFromCrDevSetting (CrDevNode->CrSetting) );
    }
    Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
  }
  return FALSE;

}

VOID
FindAsteriskDevicePath (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_HANDLE                     *HandleBuffer;
  EFI_DEVICE_PATH                *ChildDevPath;
  UINTN                          HandleCount;
  UINTN                          Index;
  H2O_UART_16550_ACCESS_PROTOCOL *Interface;

  DEBUG ((DEBUG_INFO, "\nCr << %a >>\n", __FUNCTION__));

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gH2OUart16550AccessProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDevicePathProtocolGuid, &ChildDevPath);
    DEBUG ((DEBUG_INFO, " 16550 DevPath: %s\n", ConvertDevicePathToText ((const EFI_DEVICE_PATH *)ChildDevPath, FALSE, TRUE)));
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (IsDevicePathAlreadyInCrDevVar (ChildDevPath)) {
      DEBUG ((EFI_D_INFO, "  ->AlreadyInCrDevVar\n"));
      continue;
    }

    if (IsChildDevPathOfAsteriskDevPath (ChildDevPath) == FALSE) {
      DEBUG ((EFI_D_INFO, "  ->Not Child DPath Of Asterisk DPath.\n"));
      continue;
    }

    Status = gBS->HandleProtocol (HandleBuffer[Index], &gH2OUart16550AccessProtocolGuid, &Interface);
    if (EFI_ERROR(Status)) {
      continue;
    }

    CreateNewCrDeviceVariable (
      Interface->DeviceInfo->DeviceName,
      ChildDevPath
      );

    mCrVarDeviceHead.CrDevVarCount++;
  }

  CrSafeFreePool(HandleBuffer);
}


/**


  @param

  @retval

**/
VOID
UpdateConInConOutVarFromActiveDevicePath (
  VOID
  )
{
  EFI_DEVICE_PATH   *NewActiveDevPath;
  EFI_DEVICE_PATH   *OldActiveDevPath;

  NewActiveDevPath = NULL;
  OldActiveDevPath = NULL;

  NewActiveDevPath = BuildCrActiveDevicePath ();
  OldActiveDevPath = CommonGetVariableData (CR_ACTIVE_VARIABLE_NAME, &gCrConfigurationGuid);

  if (mCrVarDeviceHead.CrDevVarCount == 0 || mCrEnable == FALSE) {
    //
    //   Delete Console Redirection device path from ComIn/Out Variable
    //
    if(OldActiveDevPath != NULL) {
      UpdateConsoleVariable (mConInVarName,  mConVarGuid, NULL, OldActiveDevPath);
      UpdateConsoleVariable (mConOutVarName, mConVarGuid, NULL, OldActiveDevPath);

      CommonSetVariable (
        CR_ACTIVE_VARIABLE_NAME,
        &gCrConfigurationGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
        0,
        OldActiveDevPath
        );
    }

  } else {
    //
    //   Update Console Redirection device path from ComIn/Out Variable
    //
    if (IsEqualDevicePath (NewActiveDevPath, OldActiveDevPath) == TRUE) {
      UpdateConsoleVariable (mConInVarName,  mConVarGuid, NewActiveDevPath, NULL);
      UpdateConsoleVariable (mConOutVarName, mConVarGuid, NewActiveDevPath, NULL);

    } else {
      UpdateConsoleVariable (mConInVarName,  mConVarGuid, NewActiveDevPath, OldActiveDevPath);
      UpdateConsoleVariable (mConOutVarName, mConVarGuid, NewActiveDevPath, OldActiveDevPath);

      CommonSetVariable (
        CR_ACTIVE_VARIABLE_NAME,
        &gCrConfigurationGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
        GetDevicePathSize (NewActiveDevPath),
        NewActiveDevPath
        );
    }
  }

  CrSafeFreePool (NewActiveDevPath);
  CrSafeFreePool (OldActiveDevPath);
}


EFI_STATUS
UpdateConsoleVariable (
  IN  CHAR16                    *ConVarName,
  IN  EFI_GUID                  *ConVarGuid,
  IN  EFI_DEVICE_PATH_PROTOCOL  *CustomizedConDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL  *ExclusiveDevicePath
  )
{
  EFI_STATUS                Status;
  UINTN                     DevicePathSize;
  BOOLEAN                   VarChanged;
  EFI_DEVICE_PATH_PROTOCOL  *VarConsole;
  EFI_DEVICE_PATH_PROTOCOL  *OldVarConsole;
  EFI_DEVICE_PATH_PROTOCOL  *NewDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *PrevDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *VarConsoleInstance;
  EFI_DEVICE_PATH_PROTOCOL  *CustomizedInstance;


  VarChanged      = FALSE;
  VarConsole      = NULL;
  DevicePathSize  = 0;
  NewDevicePath   = NULL;
  Status          = EFI_UNSUPPORTED;

  if (CustomizedConDevicePath == ExclusiveDevicePath) {
    return EFI_UNSUPPORTED;
  }

  VarConsole = CommonGetVariableData (ConVarName, ConVarGuid);

  OldVarConsole = VarConsole;

  if (ExclusiveDevicePath != NULL && VarConsole != NULL) {
    while (VarConsole != NULL) {

      VarConsoleInstance = GetNextDevicePathInstance (&VarConsole, &DevicePathSize);

      if (!IsInclusiveOfSelectDevPath (ExclusiveDevicePath, VarConsoleInstance)) {
        PrevDevicePath = NewDevicePath;
        NewDevicePath = AppendDevicePathInstance (PrevDevicePath, VarConsoleInstance);
        if  (PrevDevicePath != NULL)
          gBS->FreePool (PrevDevicePath);
      }
      else {
        VarChanged = TRUE;
      }
      gBS->FreePool (VarConsoleInstance);
    }
  }
  else {
    NewDevicePath = DuplicateDevicePath (VarConsole);
  }

  while (CustomizedConDevicePath != NULL) {

    CustomizedInstance = GetNextDevicePathInstance (&CustomizedConDevicePath, &DevicePathSize);

    if (!IsInclusiveOfSelectDevPath (NewDevicePath, CustomizedInstance)) {
      VarChanged = TRUE;
      PrevDevicePath = NewDevicePath;
      NewDevicePath = AppendDevicePathInstance (PrevDevicePath, CustomizedInstance);
      if  (PrevDevicePath != NULL)
        gBS->FreePool (PrevDevicePath);
    }

    gBS->FreePool (CustomizedInstance);

  }

  if (OldVarConsole != NULL) {
    gBS->FreePool (OldVarConsole);
  }

  //
  // Update the variable of the default console
  //
  if (VarChanged) {
    UINT32    Attributes;
    CHAR16    *StringPtr;

    StringPtr = ConVarName;

    //
    // If the variable includes "Dev" at last, we consider
    // it does not support NV attribute.
    //
    while (*StringPtr != L'\0') {
      StringPtr++;
    }

    if ( (((INTN)((UINTN)StringPtr - (UINTN)ConVarName) / sizeof (CHAR16)) > 3) &&
        (*(StringPtr - 3) == L'D') &&
        (*(StringPtr - 2) == L'e') &&
        (*(StringPtr - 1) == L'v')
       ) {
      Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
    } else {
      Attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE;
    }

    CommonSetVariable (
      ConVarName,
      ConVarGuid,
      Attributes,
      GetDevicePathSize (NewDevicePath),
      NewDevicePath
      );

    return EFI_SUCCESS;
  }


  return EFI_ABORTED;

}

/**
  Function compares a device path data structure to that of all the nodes of a
  second device path instance.

  @param  DevPathPool                 A pointer to a multi-instance device path data
                                structure.
  @param  Single                A pointer to a single-instance device path data
                                structure.

  @retval TRUE                  If the Single device path is contained within Multi device path.
  @retval FALSE                 The Single device path is not match within Multi device path.

**/
BOOLEAN
EFIAPI
IsInclusiveOfSelectDevPath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevPathPool,
  IN  EFI_DEVICE_PATH_PROTOCOL  *SelectDevPath
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  UINTN                     Size;

  if (DevPathPool == NULL || SelectDevPath  == NULL) {
    return FALSE;
  }

  DevicePath      = DevPathPool;
  DevicePathInst  = GetNextDevicePathInstance (&DevicePath, &Size);

  //
  // Search for the match of 'Single' in 'DevPathPool'
  //
  while (DevicePathInst != NULL) {
    //
    // If the single device path is found in multiple device paths,
    // return success
    //
    if (CompareMem (SelectDevPath, DevicePathInst, Size) == 0) {
      FreePool (DevicePathInst);
      return TRUE;
    }

    FreePool (DevicePathInst);
    DevicePathInst = GetNextDevicePathInstance (&DevicePath, &Size);
  }

  return FALSE;
}

/**
 Compare two device path instances

 @param [in] DevicePath1        The first device path to be compared
 @param [in] DevicePath2        The second device path to be compared

 @return TRUE                   The two device paths are the same
 @retval FALSE                  The two device paths are not the same

**/
BOOLEAN
IsEqualDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath1,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath2
  )
{
  UINTN     DevicePathSize1;
  UINTN     DevicePathSize2;
  UINTN     Value;

  DevicePathSize1 = GetDevicePathSize (DevicePath1);
  DevicePathSize2 = GetDevicePathSize (DevicePath2);

  if (DevicePathSize1 != DevicePathSize2)
    return FALSE;

  if (DevicePathSize1 == 0)
    return TRUE;

  Value = CompareMem (DevicePath1, DevicePath2, DevicePathSize1);

  if (Value == 0)
    return TRUE;

  return FALSE;
}
