/** @file
  This Library will install CrDeviceVariableLib for reference.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <InternalCrDeviceVariableLib.h>

extern CHAR16  mVarName[];

extern UART_DEVICE_PATH              mUartNode;
extern UART_FLOW_CONTROL_DEVICE_PATH mFlowControlNode;
extern VENDOR_DEVICE_PATH            mTerminalNode;

/**

  Set the N-th CrDevice Setting to variable.

  @param [IN] Index    The index of CrDevice Variable
  @param [IH]DevInfo   The CR device Setting.

  @retval EFI_SUCCESS     successfully
  @retval EFI_NOT_FOUND   The CrDevice[Index] was not found.

**/
EFI_STATUS
EFIAPI
UpdateOrDeleteCrDevVarWithDevInfo (
  IN  UINT8                    Index,
  IN  CR_DEVICE_SETTING        *DevInfo
  )
{
  EFI_STATUS         Status;
  UINTN              VarSize;
  CR_DEVICE_VAR      *CrDeviceVar;
  DPATH_UPDATE       UpdateType;

  if (DevInfo == NULL || Index >= MAX_CR_DEVICE_VARIABLE) {
    return EFI_INVALID_PARAMETER;
  }
  CrDeviceVar = NULL;
  Status = CommonGetVariableDataAndSize (CrDevVariableName (Index), &gCrConfigurationGuid, &VarSize, (VOID**)(&CrDeviceVar));
  if (!EFI_ERROR (Status)) {

    if (CrDeviceVar->Exist == TRUE) {
      Status = CalculateNewVariableSize (&CrDeviceVar, &VarSize, DevInfo->Attribute.FlowControl, &UpdateType);
      if (EFI_ERROR (Status)) {
        goto exit;
      }
      Status = ParsingAttribute2DevicePath (GetDevicePathFromVariable (CrDeviceVar), &(DevInfo->Attribute), UpdateType);
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        goto exit;
      }
    }

    //
    // Parsing other setting
    //
    CrDeviceVar->Exist      = DevInfo->Exist;
    CrDeviceVar->PortEnable = DevInfo->Attribute.PortEnable;
    CrDeviceVar->UseGlobal  = DevInfo->Attribute.UseGlobalSetting;

    //
    // Delete this variable when the device not in PCD list and non-exist
    //
    if (CrDeviceVar->Exist == FALSE && CrDeviceVar->SetInPCD == FALSE) {
      VarSize = 0;
    }

    //
    // Save CR Device Variable
    //
    Status = gRT->SetVariable (
                    mVarName,
                    &gCrConfigurationGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    VarSize,
                    (VOID*) CrDeviceVar
                    );
    ASSERT (!EFI_ERROR(Status));

    DEBUG ((DEBUG_INFO, "  Update Variable %s : %s, Size:%d\n" ,mVarName, DevInfo->DevName, VarSize));
    DEBUG ((DEBUG_INFO, "    Exist:%d, ItemType:%d, PortEnable:%d, Global:%d, TerminalType:%d, BaudRate:%d, Parity:%d, DataBits:%d, StopBits:%d, FlowControl:%d\n",
             DevInfo->Exist,
             DevInfo->ItemType,
             DevInfo->Attribute.PortEnable,
             DevInfo->Attribute.UseGlobalSetting,
             DevInfo->Attribute.TerminalType,
             DevInfo->Attribute.BaudRate,
             DevInfo->Attribute.Parity,
             DevInfo->Attribute.DataBits,
             DevInfo->Attribute.StopBits,
             DevInfo->Attribute.FlowControl
             ));
  }
exit:
  if (CrDeviceVar != NULL) {
    FreePool (CrDeviceVar);
  }

  return Status;
}

EFI_DEVICE_PATH*
EFIAPI
GetDevicePathFromCrDevSetting (
  IN CR_DEVICE_SETTING *Buffer
  )
{
  return (EFI_DEVICE_PATH*)GetNextColumnPtr (Buffer->DevName);
}

/**

  Get the N-th CrDevice setting from variable.

  @param [IN] Index    The index of CrDevice Variable
  @param [OUT]DevInfo  The CR device Setting.
                       **This buffer is allocated with AllocatePool().It is the caller's responsibility
                       to call FreePool()when the caller no longer requires the contents of Buffer.

  @retval EFI_SUCCESS     successfully
  @retval EFI_NOT_FOUND   The CrDevice[Index] was not found.

**/
EFI_STATUS
EFIAPI
GetCrDevInfoFromVariable (
  IN  UINT8                    Index,
  OUT CR_DEVICE_SETTING        **DevInfo
  )
{
  EFI_STATUS         Status;
  UINTN              BufferSize;
  UINTN              DevPathSize;
  CR_DEVICE_VAR      *CrDeviceVar;
  EFI_DEVICE_PATH    *DevPath;
  CR_DEVICE_SETTING  *Buffer;

  if (DevInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  Buffer      = NULL;
  CrDeviceVar = NULL;

  //
  // Get variable raw data
  //
  Status = CommonGetVariableDataAndSize (CrDevVariableName (Index), &gCrConfigurationGuid, &BufferSize, (VOID**)(&CrDeviceVar));
  if ((!EFI_ERROR(Status)) && (CrDeviceVar != NULL)) {

    DevPath     = GetDevicePathFromVariable (CrDeviceVar);
    DevPathSize = GetDevicePathSize (DevPath);
    //
    // Allocate Device setting buffer
    //
    Buffer = (CR_DEVICE_SETTING *)AllocateZeroPool (
                                    sizeof (CR_DEVICE_SETTING) +
                                    StrSize (&CrDeviceVar->DevName) +
                                    DevPathSize
                                    );
    if (Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Parsing device path setting
    //
    if (CrDeviceVar->Exist == TRUE) {
      Status = ParsingDevicePath2Attribute (DevPath, &(Buffer->Attribute));
      if (EFI_ERROR (Status)) {
        ASSERT (0);
        FreePool (Buffer);
        goto exit;
      }
    }

    //
    // Parsing other setting
    //
    Buffer->Exist                       = CrDeviceVar->Exist;
    Buffer->ItemType                    = CrDeviceVar->ItmeType;
    Buffer->Attribute.PortEnable        = CrDeviceVar->PortEnable;
    Buffer->Attribute.UseGlobalSetting  = CrDeviceVar->UseGlobal;

    CopyMem(Buffer->DevName, &(CrDeviceVar->DevName), StrSize(&CrDeviceVar->DevName));
    CopyMem(GetDevicePathFromCrDevSetting (Buffer) , DevPath, DevPathSize);

    DEBUG ((DEBUG_INFO, " <%a> %s : %s\n", __FUNCTION__, mVarName, Buffer->DevName));
    DEBUG ((DEBUG_INFO, "   Exist:%d, ItemType:%d, PortEnable:%d, Global:%d, TerminalType:%d, BaudRate:%d, Parity:%d, DataBits:%d, StopBits:%d, FlowControl:%d\n",
               Buffer->Exist,
               Buffer->ItemType,
               Buffer->Attribute.PortEnable,
               Buffer->Attribute.UseGlobalSetting,
               Buffer->Attribute.TerminalType,
               Buffer->Attribute.BaudRate,
               Buffer->Attribute.Parity,
               Buffer->Attribute.DataBits,
               Buffer->Attribute.StopBits,
               Buffer->Attribute.FlowControl
               ));
    DEBUG ((DEBUG_INFO, "   DevicePath: %s\n", ConvertDevicePathToText ((const EFI_DEVICE_PATH *)DevPath, FALSE, TRUE)));
  }
exit:
  if (CrDeviceVar != NULL) {
    FreePool (CrDeviceVar);
  }
  *DevInfo = Buffer;

  return Status;
}

/**

  Create CrDevice variables from PcdCrDevices.

  @param [IN]LoadDefault If True then always restore variable from PCD.
                         If False then create variable frome PCD when the variable didn't exist

  @retval EFI_SUCCESS    successfully
  @retval Other          Fail.

**/
EFI_STATUS
EFIAPI
PcdDevicesToVariables (
  IN BOOLEAN LoadDefault
  )
{
  CR_DEVICE_PCD               *PcdCrDevice;
  UINTN                       Count;
  EFI_STATUS                  Status;

  DEBUG ((DEBUG_INFO, "\nCr << %a >> Load default %d\n", __FUNCTION__, LoadDefault));

  Status = ParsingCrDevicePcdValue (&PcdCrDevice, &Count);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = CreateDeviceVariableFromPcd (PcdCrDevice, Count, LoadDefault);

  FreeDevicePathPool (PcdCrDevice, Count);

  FreePool (PcdCrDevice);

  return Status;
}

EFI_STATUS
EFIAPI
CreateNewCrDeviceVariable (
  IN  CHAR16                    *DevName,
  IN  EFI_DEVICE_PATH           *DevPath
  )
{
  EFI_STATUS               Status;
  EFI_DEVICE_PATH          *NewDevPath;
  CR_DEVICE_VAR            *CrDeviceVar;
  UINT8                    *CrDeviceVarRaw8;
  UINTN                    DPathOffset;
  UINTN                    VarSize;
  UINTN                    BufferSize;
  UINTN                    Index;

  NewDevPath  = NULL;
  CrDeviceVar = NULL;
  BufferSize  = 0;

  for (Index = 0; Index < MAX_CR_DEVICE_VARIABLE; Index++) {
    if(!CheckCrDeviceVariableExist (CrDevVariableName (Index))) {
      break;
    }
  }
  if (Index == MAX_CR_DEVICE_VARIABLE) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (mFlowControlNode.FlowControlMap != 0) {
    NewDevPath = AppendMultiDevicePathNode (DevPath, 3, &mUartNode, &mFlowControlNode, &mTerminalNode);
  } else {
    NewDevPath = AppendMultiDevicePathNode (DevPath, 2, &mUartNode, &mTerminalNode);
  }

  VarSize     = CalculateCrDeviceVarSize (DevName, NewDevPath, &DPathOffset);
  CrDeviceVar = (CR_DEVICE_VAR*)PrepareEnoughCrDeviceVariable (CrDeviceVar, VarSize, &BufferSize);
  if (CrDeviceVar == NULL) {
    ASSERT (0);
    Status =  EFI_OUT_OF_RESOURCES;
    goto exit;
  }

  //
  // Parpare CR device header, name, device path.
  //
  CrDeviceVarRaw8         = (UINT8*)CrDeviceVar;
  CrDeviceVar->Exist      = TRUE;
  CrDeviceVar->UseGlobal  = TRUE;
  CrDeviceVar->PortEnable = FALSE;
  CrDeviceVar->ItmeType   = NORMAL_ITEM;
  CrDeviceVar->SetInPCD   = FALSE;

  CopyMem (&(CrDeviceVar->DevName), DevName, StrSize(DevName));
  CopyMem (&CrDeviceVarRaw8[DPathOffset], NewDevPath, GetDevicePathSize (NewDevPath));

  Status = gRT->SetVariable (
                  mVarName,
                  &gCrConfigurationGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  VarSize,
                  (VOID*) CrDeviceVar
                  );

  DEBUG ((DEBUG_INFO, "  Create New Variable %s : %s, Size:%d\n" ,mVarName, &CrDeviceVar->DevName, VarSize));
  DEBUG ((DEBUG_INFO, "    DPath: %s\n", ConvertDevicePathToText ((const EFI_DEVICE_PATH *)GetDevicePathFromVariable (CrDeviceVar), FALSE, TRUE)));

exit:
  if (NewDevPath != NULL) {
    FreePool(NewDevPath);
  }
  if (CrDeviceVar != NULL) {
    FreePool(CrDeviceVar);
  }
  return Status;
}


