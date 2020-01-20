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

CHAR16  mVarName[]    = L"CrDevVar000";

UINT32 mCrBaudRateTable [] = {
  1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 0
};

EFI_GUID mCrTerminalGuidTable [] = {
  DEVICE_PATH_MESSAGING_VT_100,
  DEVICE_PATH_MESSAGING_VT_100_PLUS,
  DEVICE_PATH_MESSAGING_VT_UTF8,
  DEVICE_PATH_MESSAGING_PC_ANSI,
  NULL_GUID
};


UART_DEVICE_PATH  mUartNode = {
  MESSAGING_DEVICE_PATH,
  MSG_UART_DP,
  (UINT8) (sizeof (UART_DEVICE_PATH)),
  (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8),
  0,
  115200,
  8,
  1,
  1
};

UART_FLOW_CONTROL_DEVICE_PATH mFlowControlNode = {
  MESSAGING_DEVICE_PATH,
  MSG_VENDOR_DP,
  (UINT8) (sizeof (UART_FLOW_CONTROL_DEVICE_PATH)),
  (UINT8) ((sizeof (UART_FLOW_CONTROL_DEVICE_PATH)) >> 8),
  DEVICE_PATH_MESSAGING_UART_FLOW_CONTROL,
  0
};

VENDOR_DEVICE_PATH mTerminalNode = {
  MESSAGING_DEVICE_PATH,
  MSG_VENDOR_DP,
  (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
  (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8),
  DEVICE_PATH_MESSAGING_PC_ANSI
};

static EFI_DEVICE_PATH_PROTOCOL mEnd = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  END_DEVICE_PATH_LENGTH,
  0
};

CHAR16*
CrDevVariableName (
  IN UINTN Index
  )
{
  Int2Str (Index, &mVarName[8]);
  return mVarName;
}

UINTN
CalculateCrDeviceVarSize (
  IN CONST CHAR16           *NameStr,
  IN       EFI_DEVICE_PATH  *DPath,
  OUT      UINTN            *DpathOffse
  )
{
  UINTN  VarSize;

  VarSize      = (UINTN) &(((CR_DEVICE_VAR *) 0)->DevName);
  VarSize     += StrSize (NameStr);
  *DpathOffse  = VarSize;
  VarSize     += GetDevicePathSize (DPath);

  return VarSize;
}

BOOLEAN
CheckCrDeviceVariableExist (
  IN CHAR16  *VarName
  )
{
  EFI_STATUS         Status;
  UINT8              Data;
  UINTN              Size;

  Size = sizeof (UINT8);
  Status = gRT->GetVariable (VarName, &gCrConfigurationGuid, NULL, &Size, &Data);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    Status = EFI_SUCCESS;
  }
  return (Status == EFI_SUCCESS ? TRUE : FALSE);
}

EFI_STATUS
CreateDeviceVariableFromPcd (
  IN  CR_DEVICE_PCD *CrPcdDevice,
  IN  UINTN         DeviceCount,
  IN  BOOLEAN       LoadDefault
  )
{
  EFI_STATUS         Status;
  UINTN              BufferSize;
  UINTN              VarSize;
  UINTN              DPathOffset;
  UINT8              Index;
  UINT8              *CrDeviceVar;
  CR_DEVICE_VAR      *Var;
  BOOLEAN            DevVarExist;

  DEBUG ((DEBUG_INFO, "\nCr << %a >>\n", __FUNCTION__));

  CrDeviceVar = NULL;
  BufferSize  = 0;

  for (Index = 0; Index < DeviceCount; Index++) {

    DevVarExist = CheckCrDeviceVariableExist (CrDevVariableName (Index));

    //
    // if Load default or device Variable not exist then create new variable
    //
    if (((DevVarExist == TRUE) && LoadDefault) || (DevVarExist == FALSE)) {

      VarSize     = CalculateCrDeviceVarSize (CrPcdDevice[Index].DevName, CrPcdDevice[Index].DevPath, &DPathOffset);
      CrDeviceVar = (UINT8*)PrepareEnoughCrDeviceVariable (CrDeviceVar, VarSize, &BufferSize);
      if (CrDeviceVar == NULL) {
        ASSERT (0);
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Parpare CR device header, name, device path.
      //
      Var             = (CR_DEVICE_VAR*) CrDeviceVar;
      Var->Exist      = CrPcdDevice[Index].Exist;
      Var->UseGlobal  = CrPcdDevice[Index].UseGlobal;
      Var->PortEnable = CrPcdDevice[Index].PortEnable;
      Var->ItmeType   = CrPcdDevice[Index].ItemType;
      Var->SetInPCD   = TRUE;

      CopyMem (&(Var->DevName), CrPcdDevice[Index].DevName, StrSize(CrPcdDevice[Index].DevName));
      CopyMem (&CrDeviceVar[DPathOffset], CrPcdDevice[Index].DevPath, GetDevicePathSize (CrPcdDevice[Index].DevPath));

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
      ASSERT_EFI_ERROR(Status);
      DEBUG ((DEBUG_INFO, " Create %s : %r  Size:%d\n", mVarName, Status, VarSize));
    }
  }
  if (CrDeviceVar != NULL) {
    FreePool (CrDeviceVar);
  }

  DEBUG ((DEBUG_INFO, "\n"));
  return EFI_SUCCESS;
}

VOID*
GetNextColumnPtr (
  IN CONST CHAR16*  StrPtr
  )
{
  while (*StrPtr != 0) {
    StrPtr++;
  };

  return (VOID*)(++StrPtr);
}

EFI_DEVICE_PATH*
  GetDevicePathFromVariable (
  IN CR_DEVICE_VAR        *CrDeviceVar
  )
{
  return (EFI_DEVICE_PATH*) GetNextColumnPtr ((CHAR16*) &CrDeviceVar->DevName);
}

EFI_DEVICE_PATH*
  ParserDevicePathString (
  IN CONST CHAR16         *Str,
  IN OUT   CR_DEVICE_PCD  *PcdDevice
  )
{
  EFI_DEVICE_PATH               *FullDevPath;
  EFI_DEVICE_PATH               *TempDevPath;

  UpdateTemplateDeviceNode ();

  //
  // Convert String to device path
  //
  FullDevPath          = ConvertTextToDevicePath (Str);
  PcdDevice->Exist     = TRUE;
  PcdDevice->UseGlobal = TRUE;

  //
  // Check is it a vaild Cr device path.
  // Filepath node only support  "*" string
  // Other filepath node, return an end-of-device-path.
  //
  TempDevPath = VerifyMediaFileNode (FullDevPath, PcdDevice);
  if (TempDevPath != NULL) {
    FreePool(FullDevPath);
    return TempDevPath;
  }

  //
  // Verify Uart node, if uart Node didn't exist then create it.
  //
  FullDevPath = VerifyUartNode (FullDevPath, PcdDevice);

  //
  // Verify FlowControl & Terminal node
  //
  return VerifyMessagingDeviceNode (FullDevPath, PcdDevice);
}

CHAR16*
Int2Str (
  IN  UINTN   Number,
  IN  CHAR16  *Str
  )
{
  const CHAR16 Digit[] = L"0123456789";
  CHAR16  *Ptr;
  UINTN   Shifter;

  Ptr     = Str;
  Shifter = Number;
  //
  //Move to where representation ends
  //
  do {
    Ptr++;
    Shifter = Shifter/10;
  } while (Shifter);
  *Ptr = 0;
  //
  //Move back, inserting digits as u go
  //
  do{
    *--Ptr = Digit[Number % 10];
    Number = Number / 10;
  }while(Number);

  return Str;
}

EFI_STATUS
ParsingCrDevicePcdValue (
  OUT CR_DEVICE_PCD **PcdCrDevice,
  OUT UINTN         *DeviceCount
  )
{
  CR_DEVICE_PCD    *PtrPcd;
  CR_DEVICE_PCD    *CrDevice;
  CONST CHAR16     *TmpStr;
  UINTN            Count;
  UINTN            BufferCount;

  DEBUG ((DEBUG_INFO, "\nCr << %a >>\n", __FUNCTION__));

  Count       = 0;
  BufferCount = MAX_CR_DEVICE_VARIABLE;
  PtrPcd      = (CR_DEVICE_PCD*)PcdGetPtr (PcdH2OCrDevice);

  CrDevice = AllocateZeroPool (BufferCount * sizeof (CR_DEVICE_PCD));
  if (CrDevice == NULL) {
    ASSERT (0);
    return EFI_OUT_OF_RESOURCES;
  }

  for (Count = 0; PtrPcd->PortEnable != 0xff; Count++) {
    if (Count >= BufferCount) {
      break;
    }

    CrDevice[Count].PortEnable = PtrPcd->PortEnable;
    CrDevice[Count].DevName    = (CHAR16*)&(PtrPcd->DevName);
    TmpStr                     = (CHAR16*)GetNextColumnPtr (CrDevice[Count].DevName);
    DEBUG ((DEBUG_INFO, " PCD value: Idx=%d, En:%d, Name=%s, Dpath Str=%s\n",Count, CrDevice[Count].PortEnable,CrDevice[Count].DevName, TmpStr));

    //
    // Covert device path string to device path node
    //
    CrDevice[Count].DevPath    = ParserDevicePathString (TmpStr, &CrDevice[Count]);
    DEBUG ((DEBUG_INFO, "   >>Result DPath: %s\n", ConvertDevicePathToText ((const EFI_DEVICE_PATH *)CrDevice[Count].DevPath, FALSE, TRUE)));
    //
    // Get next device
    //
    PtrPcd = (CR_DEVICE_PCD*) GetNextColumnPtr (TmpStr);
  }

  *DeviceCount = Count;
  *PcdCrDevice = CrDevice;
  return EFI_SUCCESS;
}

EFI_STATUS
ParsingDevicePath2Attribute (
  IN  EFI_DEVICE_PATH      *DevPath,
  OUT CR_DEVICE_ATTRIBUTES *Attribute
  )
{
  VENDOR_DEVICE_PATH  *Vendor;
  UART_DEVICE_PATH    *Uart;
  UINT8               Index = 0;

  //
  // Get Uart Node
  //
  Uart = NULL;
  Uart = (UART_DEVICE_PATH*) LocateDevicePathNode (&DevPath, MESSAGING_DEVICE_PATH, MSG_UART_DP);
  if (Uart == NULL) {
    ASSERT(0);
    return EFI_UNSUPPORTED;

  } else if (!(DevPath->Type == MESSAGING_DEVICE_PATH && DevPath->SubType == MSG_VENDOR_DP)) {
    //
    // Uart node's next deviceNode is not MESSAGING_DEVICE_PATH
    //
    ASSERT (0);
    return EFI_UNSUPPORTED;
  }

  //
  // Check flow control
  //
  Vendor = (VENDOR_DEVICE_PATH *) DevPath;
  if (CompareGuid (&Vendor->Guid, &gEfiUartDevicePathGuid)) {
    Attribute->FlowControl = (UINT8)(((UART_FLOW_CONTROL_DEVICE_PATH *) Vendor)->FlowControlMap);
    Vendor = (VENDOR_DEVICE_PATH *) NextDevicePathNode ((VOID*)Vendor);
  } else {
    Attribute->FlowControl = 0;
  }

  //
  // Get terminal type
  //
  for (Index = 0; mCrTerminalGuidTable[Index].Data1 != 0; Index++) {
    if (CompareGuid (&Vendor->Guid, &mCrTerminalGuidTable[Index])) {
      Attribute->TerminalType = Index;
      break;
    }
  }
  //
  // Get BaudRate
  //
  for (Index = 0; mCrBaudRateTable[Index] != 0; Index++) {
    if (Uart->BaudRate == mCrBaudRateTable[Index]) {
      break;
    }
  }
  Attribute->BaudRate = (mCrBaudRateTable[Index] == 0) ? --Index : Index;
  //
  // DataBits only support 0x07 - 7 Bits
  //                       0x08 - 8 Bits
  //
  Attribute->DataBits = (Uart->DataBits == 7 || Uart->DataBits == 8)? Uart->DataBits : 8;
  //
  //StopBits only support  0x01 - 1 Stop Bit
  //                       0x03 - 2 Stop Bits
  //
  Attribute->StopBits = (Uart->StopBits == 1 || Uart->StopBits == 3)? Uart->StopBits : 1;
  //
  //Parity only support 0x01 - No Parity
  //                    0x02 - Even Parity
  //                    0x03 - Odd Parity
  //
  Attribute->Parity   = (Uart->Parity >= 1 && Uart->Parity <= 3)? Uart->Parity : 1;

  return EFI_SUCCESS;
}

EFI_STATUS
CalculateNewVariableSize (
  IN OUT CR_DEVICE_VAR     **CrDeviceVar,
  IN OUT UINTN             *VarSize,
  IN     UINT8             NewFlowcontrol,
  OUT    DPATH_UPDATE      *UpdateType
  )
{
  EFI_DEVICE_PATH      *NextPath;
  VENDOR_DEVICE_PATH   *VendorDPath;
  UINT32               OriginalFlowControl;
  UINTN                NewVarSize;
  CR_DEVICE_VAR        *NewCrDeviceVar;

  OriginalFlowControl = 0;
  NextPath            = GetDevicePathFromVariable (*CrDeviceVar);
  VendorDPath         = (VENDOR_DEVICE_PATH*) LocateDevicePathNode (&NextPath, MESSAGING_DEVICE_PATH, MSG_VENDOR_DP);
  if (VendorDPath == NULL) {
    return EFI_NOT_FOUND;
  }

  if (CompareGuid (&(VendorDPath->Guid), &gEfiUartDevicePathGuid)) {
    OriginalFlowControl = ((UART_FLOW_CONTROL_DEVICE_PATH*)VendorDPath)->FlowControlMap;
    if (OriginalFlowControl == 0) {
      //
      // if FlowControl is disable, set FlowControl to non zero, then FlowControl node will delete.
      //
      OriginalFlowControl = 1;
    }
  }

  if ((OriginalFlowControl == 0) && (NewFlowcontrol != 0)) {
    NewVarSize     = (*VarSize) + sizeof (UART_FLOW_CONTROL_DEVICE_PATH);
    NewCrDeviceVar = AllocatePool (NewVarSize);
    if (NewCrDeviceVar == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem (NewCrDeviceVar, *CrDeviceVar, *VarSize);
    FreePool (*CrDeviceVar);
    *UpdateType = UPDATE_ADD;

  } else if ((OriginalFlowControl != 0) && (NewFlowcontrol == 0)) {
    NewVarSize     = (*VarSize) - sizeof (UART_FLOW_CONTROL_DEVICE_PATH);
    NewCrDeviceVar = (*CrDeviceVar);
    *UpdateType = UPDATE_DELETE;
  } else {
    *UpdateType = UPDATE_NONE;
    return EFI_SUCCESS;
  }
  *CrDeviceVar = NewCrDeviceVar;
  *VarSize     = NewVarSize;

  return EFI_SUCCESS;
}

EFI_STATUS
ParsingAttribute2DevicePath (
  IN OUT EFI_DEVICE_PATH      *DevPath,
  IN     CR_DEVICE_ATTRIBUTES *Attribute,
  IN     DPATH_UPDATE         UpdateType
  )
{
  EFI_DEVICE_PATH                *DuplicateDpath;
  EFI_DEVICE_PATH                *NodeAfterUart;
  VENDOR_DEVICE_PATH             *TerminalNode;
  VENDOR_DEVICE_PATH             *DuplicateTerminalNode;
  UART_FLOW_CONTROL_DEVICE_PATH  *FlowNode;
  UART_DEVICE_PATH               *Uart;

  DuplicateDpath        = NULL;
  NodeAfterUart = DevPath;
  Uart = (UART_DEVICE_PATH*) LocateDevicePathNode (&NodeAfterUart, MESSAGING_DEVICE_PATH, MSG_UART_DP);
  if (Uart == NULL) {
    return EFI_UNSUPPORTED;
  }
  Uart->BaudRate = mCrBaudRateTable[Attribute->BaudRate];
  Uart->DataBits = Attribute->DataBits;
  Uart->StopBits = Attribute->StopBits;
  Uart->Parity   = Attribute->Parity;

  if (UpdateType != UPDATE_NONE) {
    DuplicateDpath = DuplicateDevicePath (NodeAfterUart);
  }

  switch (UpdateType) {
  case UPDATE_DELETE:
    FlowNode              = (UART_FLOW_CONTROL_DEVICE_PATH*)NodeAfterUart;
    DuplicateTerminalNode = (VENDOR_DEVICE_PATH*) NextDevicePathNode ((VOID*)DuplicateDpath);
    CopyMem (FlowNode, DuplicateTerminalNode, sizeof (VENDOR_DEVICE_PATH) + sizeof (EFI_DEVICE_PATH));
    TerminalNode          = (VENDOR_DEVICE_PATH*) FlowNode;
    FlowNode              = NULL;
    break;

  case UPDATE_ADD:
    TerminalNode = (VENDOR_DEVICE_PATH*) NodeAfterUart;
    CopyMem (TerminalNode, &mFlowControlNode, sizeof (UART_FLOW_CONTROL_DEVICE_PATH));
    FlowNode     = (UART_FLOW_CONTROL_DEVICE_PATH*)TerminalNode;
    TerminalNode = (VENDOR_DEVICE_PATH*)(FlowNode + 1);
    CopyMem (TerminalNode, DuplicateDpath, sizeof (VENDOR_DEVICE_PATH) + sizeof (EFI_DEVICE_PATH));
    break;

  default:
    TerminalNode = (VENDOR_DEVICE_PATH*)NodeAfterUart;
    if (CompareGuid (&(TerminalNode->Guid), &gEfiUartDevicePathGuid)) {
      FlowNode     = (UART_FLOW_CONTROL_DEVICE_PATH*)TerminalNode;
      TerminalNode = (VENDOR_DEVICE_PATH*) NextDevicePathNode ((VOID*)FlowNode);
    } else {
      FlowNode = NULL;
    }
  }

  if (FlowNode != NULL) {
    FlowNode->FlowControlMap = Attribute->FlowControl;
  }

  TerminalNode->Guid = mCrTerminalGuidTable[Attribute->TerminalType];

  if (DuplicateDpath != NULL) {
    FreePool (DuplicateDpath);
  }

  return EFI_SUCCESS;
}

VOID*
PrepareEnoughCrDeviceVariable (
  IN     VOID     *CrDeviceVar,
  IN     UINTN    NewSize,
  IN OUT UINTN    *BufferSize
  )
{
  UINT8  *TmpPool;

  if (NewSize > (*BufferSize)) {
    TmpPool = (UINT8*) ReallocatePool(*BufferSize, NewSize, (VOID*)CrDeviceVar);
    if (TmpPool != NULL) {
      *BufferSize = NewSize;
      return TmpPool;

    } else {
      if (CrDeviceVar != NULL) {
        FreePool (CrDeviceVar);
      }
      return NULL;
    }

  } else {
    return CrDeviceVar;
  }
}

VOID
UpdateTemplateDeviceNode (
  VOID
  )
{
  CR_CONFIGURATION   *CrConfig;

  CrConfig = NULL;
  CrConfig = (CR_CONFIGURATION*) CommonGetVariableData (CONSOLE_REDIRECTION_VARSTORE_DEFAULT_NAME, &gCrConfigurationGuid);

  if (CrConfig != NULL) {
    mUartNode.BaudRate              = mCrBaudRateTable[CrConfig->GlobalBaudRate];
    mUartNode.DataBits              = CrConfig->GlobalDataBits;
    mUartNode.Parity                = CrConfig->GlobalParity;
    mUartNode.StopBits              = CrConfig->GlobalStopBits;
    mTerminalNode.Guid              = mCrTerminalGuidTable[CrConfig->GlobalTerminalType];
    mFlowControlNode.FlowControlMap = CrConfig->GlobalFlowControl;
    FreePool (CrConfig);
  }
}

EFI_DEVICE_PATH*
  VerifyMediaFileNode (
  IN  EFI_DEVICE_PATH *FullDevPath,
  OUT CR_DEVICE_PCD   *PcdDevice
  )
{
  CHAR16               *Asterisk = L"*";
  EFI_DEVICE_PATH      *TempDevPath;
  EFI_DEVICE_PATH      *NextDevPath;

  NextDevPath = FullDevPath;
  TempDevPath = LocateDevicePathNode (&NextDevPath, MEDIA_DEVICE_PATH, MEDIA_FILEPATH_DP);

  if (TempDevPath != NULL) {
    PcdDevice->Exist = FALSE;
    if (StrCmp(((FILEPATH_DEVICE_PATH*)TempDevPath)->PathName, Asterisk) != 0) {
      PcdDevice->ItemType = INVALID_ITEM;
      DEBUG ((DEBUG_INFO, "   Invalid DevicePath!!\n"));
      return AppendDevicePath (NULL, NULL);
    }

    PcdDevice->ItemType = ASTERISK_ITEM;
    DEBUG ((DEBUG_INFO, "   DevicePath with * !\n"));
    //
    // if DevicePath with '*' then del * DevicePath node.
    //
    CopyMem (TempDevPath, NextDevPath, sizeof (EFI_DEVICE_PATH));
    return DuplicateDevicePath(FullDevPath);
  }
  return NULL;
}

EFI_DEVICE_PATH*
  VerifyUartNode (
  IN  EFI_DEVICE_PATH *FullDevPath,
  OUT CR_DEVICE_PCD   *PcdDevice
  )
{
  UART_DEVICE_PATH              *Uart;
  UINTN                         Index;
  EFI_DEVICE_PATH               *NextDevPath;
  EFI_DEVICE_PATH               *TempDevPath;

  NextDevPath = FullDevPath;
  TempDevPath = LocateDevicePathNode (&NextDevPath, MESSAGING_DEVICE_PATH, MSG_UART_DP);

  if (TempDevPath == NULL) {
    //
    // Create default Uart node
    //
    TempDevPath = AppendDevicePathNode (FullDevPath, (EFI_DEVICE_PATH*)&mUartNode);
    DEBUG ((DEBUG_INFO, "   Create Uart Device Node!\n"));
    FreePool(FullDevPath);
    return TempDevPath;

  } else {
    PcdDevice->UseGlobal = FALSE;
    //
    // Check Uart node is vaild?
    //
    Uart = (UART_DEVICE_PATH*)TempDevPath;

    //
    // Check BaudRate valid
    //
    for (Index = 0; mCrBaudRateTable[Index] != 0; Index++) {
      if (Uart->BaudRate == mCrBaudRateTable[Index]) {
        break;
      }
    }
    if (mCrBaudRateTable[Index] == 0) {
      Uart->BaudRate = mUartNode.BaudRate ;
    }
    //
    // Check DataBits vaild. only support 0x07 - 7 Bits
    //                                    0x08 - 8 Bits
    //
    if (!(Uart->DataBits == 7 || Uart->DataBits == 8)) {
      Uart->DataBits = mUartNode.DataBits;
    }
    //
    // Check StopBits vaild. only support  0x01 - 1 Stop Bit
    //                                     0x03 - 2 Stop Bits
    //
    if (!(Uart->StopBits == 1 || Uart->StopBits == 3)) {
      Uart->StopBits = mUartNode.StopBits;
    }
    //
    // Check Parity vaild. only support 0x01 - No Parity
    //                                  0x02 - Even Parity
    //                                  0x03 - Odd Parity
    //
    if (!(Uart->Parity >= 1 && Uart->Parity <= 3)) {
      Uart->Parity = mUartNode.Parity;
    }
  }
  return FullDevPath;
}

EFI_DEVICE_PATH*
  VerifyMessagingDeviceNode (
  IN  EFI_DEVICE_PATH *FullDevPath,
  OUT CR_DEVICE_PCD   *PcdDevice
  )
{
  UART_FLOW_CONTROL_DEVICE_PATH *FlowDpath;
  UINTN                         Index;
  EFI_DEVICE_PATH               *NextDevPath;
  EFI_DEVICE_PATH               *TempDevPath;

  NextDevPath = FullDevPath;
  TempDevPath = LocateDevicePathNode(&NextDevPath, MESSAGING_DEVICE_PATH, MSG_VENDOR_DP);

  if (TempDevPath == NULL) {
    //
    // Create default FlowControl & Terminal node
    //
    if (mFlowControlNode.FlowControlMap != 0) {
      TempDevPath = AppendMultiDevicePathNode (FullDevPath, 2, &mFlowControlNode, &mTerminalNode);
      DEBUG ((DEBUG_INFO, "   Create FlowControl & Terminal Node!\n"));
    } else {
      TempDevPath = AppendDevicePathNode (FullDevPath, (EFI_DEVICE_PATH*)&mTerminalNode);
      DEBUG ((DEBUG_INFO, "   Create Terminal Device Node!\n"));
    }
    FreePool(FullDevPath);
    return TempDevPath;

  } else {
    PcdDevice->UseGlobal = FALSE;
    FlowDpath = (UART_FLOW_CONTROL_DEVICE_PATH*)TempDevPath;
    //
    // Check is a FlowControl node?
    //
    if (CompareGuid (&FlowDpath->Guid, &gEfiUartDevicePathGuid)) {
      //
      // check is a terminal follow by flow control node?
      //
      if (FlowDpath->FlowControlMap > 2) {
        FlowDpath->FlowControlMap = 0;
      }

      TempDevPath = LocateDevicePathNode(&NextDevPath, MESSAGING_DEVICE_PATH, MSG_VENDOR_DP);
      if (TempDevPath == NULL) {
        TempDevPath = AppendDevicePathNode (FullDevPath, (EFI_DEVICE_PATH*)&mTerminalNode);
        DEBUG ((DEBUG_INFO, "   Create Terminal Device Node after flow control!\n"));
        FreePool(FullDevPath);
        return TempDevPath;
      }
    }
    //
    // Check terminal node is vaild
    //
    for (Index = 0; mCrTerminalGuidTable[Index].Data1 != 0; Index++) {
      if (CompareGuid (&(((VENDOR_DEVICE_PATH*)TempDevPath)->Guid), &mCrTerminalGuidTable[Index])) {
        break;
      }
    }
    if (mCrTerminalGuidTable[Index].Data1 == 0) {
      CopyGuid (&(((VENDOR_DEVICE_PATH*)TempDevPath)->Guid), &mTerminalNode.Guid);
    }
  }
  return FullDevPath;
}


EFI_STATUS
FreeDevicePathPool (
  IN  CR_DEVICE_PCD *CrDevice,
  IN  UINTN         DeviceCount
  )
{
  UINT8 Index;

  for (Index = 0; Index < DeviceCount; Index++) {
    if (CrDevice[Index].DevPath != NULL) {
      FreePool((VOID*) CrDevice[Index].DevPath);
    }
  }
  return EFI_SUCCESS;
}

EFI_DEVICE_PATH_PROTOCOL  *
AppendMultiDevicePathNode (
  EFI_DEVICE_PATH_PROTOCOL  *SrcDevicePath,
  UINTN                     NumOfArgs,
  ...
  )
{
  VA_LIST                   Marker;
  UINTN                     Index;
  UINTN                     NewSize;
  UINTN                     SrcSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevPathNode;
  EFI_DEVICE_PATH_PROTOCOL  *DevPathPtr;
  EFI_DEVICE_PATH_PROTOCOL  *NewDevicePath;

  SrcSize = GetDevicePathSize(SrcDevicePath);
  NewSize = SrcSize;

  VA_START (Marker, NumOfArgs);

  // Caculate the new device path size
  for (Index = 0; Index < NumOfArgs; Index++) {
    DevPathNode = VA_ARG (Marker, EFI_DEVICE_PATH_PROTOCOL *);
    NewSize += DevicePathNodeLength (DevPathNode);
  }

  VA_END(Marker);

  NewDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) AllocateCopyPool (NewSize, SrcDevicePath);

  DevPathPtr = (EFI_DEVICE_PATH_PROTOCOL *)((UINTN)NewDevicePath + SrcSize - sizeof(EFI_DEVICE_PATH_PROTOCOL));

  VA_START (Marker, NumOfArgs);

  // Copy node to DevicePath
  for (Index = 0;  Index < NumOfArgs; Index++) {
    DevPathNode = VA_ARG (Marker, EFI_DEVICE_PATH_PROTOCOL *);
    CopyMem (DevPathPtr, DevPathNode, DevicePathNodeLength (DevPathNode));
    DevPathPtr = (EFI_DEVICE_PATH_PROTOCOL *)((UINT8*)DevPathPtr + DevicePathNodeLength (DevPathNode));
  }
  //
  // Copy End Node
  //
  CopyMem (DevPathPtr, &mEnd, sizeof(EFI_DEVICE_PATH_PROTOCOL));

  return NewDevicePath;

}

/**
 Get the device path node for given device path type and device path sub-type

 @param [in] DevicePath         The device path to be processed
 @param [in] DevicePathType     The device path type
 @param [in] DevicePathSubType  The device path sub type

 @return  The retrieved device path node with the given DevicePathType and DevicePathSubType

**/
EFI_DEVICE_PATH_PROTOCOL *
LocateDevicePathNode(
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath,
  IN UINT8                          DevPathType,
  IN UINT8                          DevPathSubType
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;

  if (*DevicePath == NULL)
    return NULL;

  DevPath = *DevicePath;

  while (!IsDevicePathEnd(DevPath)) {
    if (DevPath->Type == DevPathType && DevPath->SubType == DevPathSubType) {
      *DevicePath = NextDevicePathNode (DevPath);
      return DevPath;
    }
    DevPath = NextDevicePathNode(DevPath);
  }

  *DevicePath = NULL;

  if (DevPathType == END_DEVICE_PATH_TYPE && DevPathSubType == END_ENTIRE_DEVICE_PATH_SUBTYPE)
    return DevPath;

  return NULL;

}

