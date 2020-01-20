/** @file
  CrServiceDxe driver

;******************************************************************************
;* Copyright (c) 2012 - 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "CrServiceDxe.h"
#include "CrVariableUpdate.h"
#include "CrLegacySupport.h"

BOOLEAN                             mCrEnable;
BOOLEAN                             mHeadless;
EFI_HANDLE                          mCrServiceImageHandle;
H2O_BDS_CP_HANDLE                   mCpConOutHandle;
CR_POLICY_VARIABLE                  mCrPolicy;

extern CR_DEVICES_SETTING_HEAD_NODE mCrVarDeviceHead;
extern UINT32                       mCrBaudRateTable[];


EFI_STATUS
DummyQueryVgaScreenBuffer (
  IN      EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL    *This,
  IN OUT  UINTN                                       *ScreenBuffer,
  IN OUT  UINTN                                       *ScreenAttributes
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
DummyDisconnectCrsTerminal (
  EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL  *This,
  UINTN                                     TerminalNumber
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
DummyconnectCrsTerminal (
  EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL  *This,
  UINTN                                     TerminalNumber
  )
{
  return EFI_UNSUPPORTED;
}


/**

  Query Consplitter screen buffer and attribute buffer

  @param        This                    A pointer to the EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL instance.
  @param        ScreenBuffer            A pointer to current screen buffer.
  @param        ScreenAttributes        A pointer to current attribute buffer.

  @retval         EFI_SUCCESS           Get Screen Buffer Success
  @retval         EFI_UNSUPPORTED       Get Screen Buffer fail

**/
EFI_STATUS
EFIAPI
QueryVgaScreenBuffer (
  IN      EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL    *This,
  IN OUT  UINTN                                       *ScreenBuffer,
  IN OUT  UINTN                                       *ScreenAttributes
  )
{
  UINTN  ConsplitterScreenBuf;
  UINTN  ConsplitterAttrBuf;

  ConsplitterScreenBuf = 0;
  ConsplitterAttrBuf   = 0;

  ConsplitterScreenBuf = (UINTN)PcdGet64 (PcdDevNullScreenPtr);
  ConsplitterAttrBuf = (UINTN)PcdGet64 (PcdDevNullAttributes);
  if (ConsplitterScreenBuf == 0 || ConsplitterAttrBuf == 0) {
    return EFI_UNSUPPORTED;
  }

  *ScreenBuffer     = ConsplitterScreenBuf;
  *ScreenAttributes = ConsplitterAttrBuf;

  return EFI_SUCCESS;
}

/**

 Disconnect Console Redirection Terminal controller.

 @param        This                    A pointer to the EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL instance.
 @param        TerminalNumber          Specify the terminal index that will be disconnect. The first index number is 0.

 @retval         EFI_SUCCESS             Disconnect success
 @retval         EFI_INVALID_PARAMETER   The Terminal number is invalid
 @retval         others                  Disconnect fail

**/
EFI_STATUS
EFIAPI
DisconnectCrTerminal (
  IN EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL    *This,
  IN UINTN                                       TerminalNumber
  )
{
  EFI_STATUS                      Status;
  EFI_CONSOLE_REDIRECTION_INFO    *CrInfo;
  EFI_DEVICE_PATH_PROTOCOL        *ChildDevicePath;
  EFI_HANDLE                      ChildHandle;
  EFI_HANDLE                      ParentHandle;

  CrInfo = This->CRInfo;

  if (TerminalNumber >= CrInfo->DeviceCount) {
    return EFI_INVALID_PARAMETER;
  }

  ChildDevicePath = CrInfo->CRDevice[TerminalNumber].DevicePath;
  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid , &ChildDevicePath, &ChildHandle);

  if (!EFI_ERROR (Status) && IsDevicePathEnd (ChildDevicePath)) {
    Status = GetParentHandle (ChildHandle, &ParentHandle);
    if (!EFI_ERROR(Status)) {
      Status = gBS->DisconnectController (ParentHandle, NULL, NULL);
    }
  }
  return Status;
}


/**

 Connect Console Redirection Terminal controller.

 @param        This                    A pointer to the EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL instance.
 @param        TerminalNumber          Specify the terminal index that will be Connect. The first index number is 0.

 @retval         EFI_SUCCESS             Connect success
 @retval         EFI_INVALID_PARAMETER   The Terminal number is invalid
 @retval         EFI_ABORTED             Can't duplicate device path.
 @retval         others                  Connect fail

**/
EFI_STATUS
EFIAPI
ConnectCrTerminal (
  IN EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL    *This,
  IN UINTN                                       TerminalNumber
  )
{
  EFI_STATUS                         Status;
  EFI_CONSOLE_REDIRECTION_INFO       *CrInfo;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL           *OriginalDevicePath;
  EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath;
  EFI_HANDLE                         ParentHandle;

  CrInfo = This->CRInfo;

  if (TerminalNumber >= CrInfo->DeviceCount) {
    return EFI_INVALID_PARAMETER;
  }
  OriginalDevicePath  = DuplicateDevicePath (CrInfo->CRDevice[TerminalNumber].DevicePath);
  if (OriginalDevicePath == NULL) {
    return EFI_ABORTED;
  }
  RemainingDevicePath = CrInfo->CRDevice[TerminalNumber].DevicePath;
  DevicePath          = OriginalDevicePath;
  TruncateLastNode (DevicePath);
  PointToLastNode  (&RemainingDevicePath);

  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid , &DevicePath, &ParentHandle);

  if (!EFI_ERROR (Status) && IsDevicePathEnd (DevicePath)) {
      Status = gBS->ConnectController (ParentHandle, NULL, RemainingDevicePath, TRUE);
  }
  gBS->FreePool (OriginalDevicePath);
  return Status;
}


EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL  mDummyCRService = {
                                            NULL,
                                            DummyQueryVgaScreenBuffer,
                                            DummyDisconnectCrsTerminal,
                                            DummyconnectCrsTerminal
                                            };

EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL  mCrService = {
                                            NULL,
                                            QueryVgaScreenBuffer,
                                            DisconnectCrTerminal,
                                            ConnectCrTerminal
                                            };


//[-start-160705-IB07400752-add]//
EFI_STATUS
HeadlessTest (
  VOID
  )
{
  UINT64  Address = 0xA0000;
  UINTN   Index;
  UINT8   TestData;
  UINTN   TestSize = 0x100;

  //
  // Test A Segment
  //
  for (Index = 0; Index < TestSize; Index++) {
    TestData = *(UINT8*)((UINTN) (Address + Index));
    if (TestData != 0xFF) {
      DEBUG ((EFI_D_ERROR, "Non-Headless mode!!\n"));
      return EFI_SUCCESS;
    }
  }

  //
  // A-B segment can not used, headless mode
  // 
  DEBUG ((EFI_D_ERROR, "A-B segment can not used, Headless mode!!\n"));
//[-start-160712-IB07400756-add]//
  PcdSet8 (PcdIsHeadlessMode, 1);
//[-end-160712-IB07400756-add]//
  return EFI_NOT_FOUND;
}
//[-end-160705-IB07400752-add]//


/**


  @param

  @retval

**/
BOOLEAN
IsHeadlessPlatform (
  VOID
  )
{
  EFI_STATUS            Status;
  UINTN                 NumHandles;
  EFI_HANDLE            *HandleBuffer;
  UINTN                 Index;
  EFI_PCI_IO_PROTOCOL   *PciIo;
  PCI_TYPE00            PciConfig;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );

  if (EFI_ERROR(Status)) {
    return TRUE;
  }

  for (Index = 0; Index < NumHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint8,
                          0,
                          sizeof(PCI_TYPE00),
                          &PciConfig
                          );
    if (EFI_ERROR(Status)) {
      continue;
    }

    if (PciConfig.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY) {
//[-start-160705-IB07400752-modify]//
      Status = HeadlessTest();
      if (!EFI_ERROR (Status)) { // Non-Headless
        return FALSE;
      } else { // Headless
        return TRUE;
      }
//[-end-160705-IB07400752-modify]//
    }
  }

  CrSafeFreePool (HandleBuffer);

  return TRUE;
}

/**


  @param

  @retval

**/
VOID
UpdateCrStatus (
  VOID
  )
{
  CR_POLICY_VARIABLE    *Tmp;

  Tmp = NULL;
  Tmp = (CR_POLICY_VARIABLE*) CommonGetVariableData (CR_POLICY_NAME, &gCrConfigurationGuid);
  if (Tmp == NULL) {
    mCrEnable = FALSE;
    DEBUG ((EFI_D_INFO, "\nCr << %a >> C.R. Disable\n", __FUNCTION__));
    return;
  }

  CopyMem (&mCrPolicy, Tmp, sizeof(CR_POLICY_VARIABLE));
  mCrEnable = TRUE;

  if (IsHeadlessPlatform()) {
    mHeadless = TRUE;
    mCrEnable = TRUE;
  } else {
    mHeadless = FALSE;
  }
  DEBUG ((EFI_D_INFO, "\nCr << %a >> C.R. Enable, Headless:%d\n", __FUNCTION__, mHeadless));
  FreePool (Tmp);
}

UINTN
GetNumberOfActiveCrDevice (
  VOID
  )
{
  LIST_ENTRY                      *Link;
  CR_DEVICES_SETTING_NODE         *CrDevNode;
  UINTN                           Count;

  Count = 0;
  Link  = GetFirstNode (&mCrVarDeviceHead.Link);
  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);
    if (CrDevNode->AddIntoConVar == FALSE) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }
    Count++;
    Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
  }
  return Count;
}

EFI_CONSOLE_REDIRECTION_INFO *
BuildCrInfo (
  VOID
  )
{
  UINTN                           DeviceCount;
  UINTN                           DeviceIndex;
  UINTN                           BufferSize;
  EFI_CONSOLE_REDIRECTION_INFO    *CRInfo;
  LIST_ENTRY                      *Link;
  CR_DEVICES_SETTING_NODE         *CrDevNode;
  EFI_DEVICE_PATH                 *RemainDevPath;
  EFI_DEVICE_PATH_PROTOCOL        *DevPathProtocol;
  EFI_STATUS                      Status;
  H2O_UART_16550_ACCESS_PROTOCOL  *Uart16550Protocol;
  UART_16550_DEVICE_INFO          *DeviceInfo;

  CRInfo      = NULL;
  DeviceIndex = 0;
  DeviceCount = GetNumberOfActiveCrDevice ();
  if (DeviceCount == 0) {
    return NULL;
  }

  BufferSize  = sizeof(EFI_CONSOLE_REDIRECTION_INFO) + (DeviceCount * sizeof (EFI_CONSOLE_REDIRECTION_DEVICE));

  CRInfo = AllocateZeroPool (BufferSize);
  if (CRInfo == NULL) {
    return CRInfo;
  }

  CRInfo->Headless    = mHeadless;

  Link = GetFirstNode (&mCrVarDeviceHead.Link);
  while (!IsNull (&mCrVarDeviceHead.Link, Link)) {
    CrDevNode = CR_DEVICE_SETTING_FROM_THIS (Link);
    if (CrDevNode->AddIntoConVar == FALSE) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }

    RemainDevPath = GetDevicePathFromCrDevSetting(CrDevNode->CrSetting);
    Status = GetProtocolWithLocateDevicePath (&gH2OUart16550AccessProtocolGuid, &RemainDevPath, &Uart16550Protocol);
    if (EFI_ERROR (Status)) {
      Link  = GetNextNode (&mCrVarDeviceHead.Link, Link);
      continue;
    }
    DeviceInfo = Uart16550Protocol->DeviceInfo;

    CRInfo->CRDevice[DeviceIndex].Uart16550protocol = Uart16550Protocol;
    CRInfo->CRDevice[DeviceIndex].Type              = DeviceInfo->DeviceType;
    CRInfo->CRDevice[DeviceIndex].BaudRateDivisor   = (UINT8)(DeviceInfo->SerialClockFreq / DeviceInfo->SampleRate / mCrBaudRateTable[CrDevNode->CrSetting->Attribute.BaudRate]);

    RemainDevPath = GetDevicePathFromCrDevSetting(CrDevNode->CrSetting);
    Status = GetProtocolWithLocateDevicePath (&gEfiDevicePathProtocolGuid, &RemainDevPath, &DevPathProtocol);
    if (!EFI_ERROR (Status)) {
      CRInfo->CRDevice[DeviceIndex].DevicePath = DevPathProtocol;
    }

    Link = GetNextNode (&mCrVarDeviceHead.Link, Link);
    DeviceIndex ++;
  }

  CRInfo->DeviceCount = (UINT8)DeviceIndex;
  return CRInfo;
}

/**
  Before connect console device, CRservice need to add CR devices path into ConIn/ConOut variable.

  @param

  @retval

**/
VOID
BeforeConnectConsoleCallback (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  )
{
  static BOOLEAN    IsDone = FALSE;

  if (IsDone == TRUE) {
    return;
  }
  IsDone = TRUE;

  //
  // If SCU set CrEnable or headless then set CrEnable.
  //
  UpdateCrStatus ();

  //
  // Create a temp device list for access current Cr Device Variable.
  //
  CreateCrDeviceVarList ();
  //
  // Add Enable CR devices to ConIn/ConOut Variable
  //
  UpdateConInConOutVarFromActiveDevicePath ();

}

VOID
AfterConnectConOutCallBack (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  )
{
  EFI_STATUS Status;

  if (mCrEnable) {
    //
    // Check the select Cr devices still in Con Variable?
    // If CR device connect fail then set exist = false.
    //
    CheckAndSaveConnectResult (mConOutVarName, mConVarGuid);
    CreateNewActiveCrDevice ();

    //
    // Prepare CrInfo and reinstall protocol
    //
    mCrService.CRInfo = BuildCrInfo ();
    gBS->ReinstallProtocolInterface (
           mCrServiceImageHandle,
           &gConsoleRedirectionServiceProtocolGuid,
           &mDummyCRService,
           &mCrService
           );

    //
    // If there have Cr device, prepare option rom
    //
    if (mCrService.CRInfo != NULL) {
      Status = CrOpRomSupport ();
      DEBUG ((DEBUG_INFO, "\nCr << CrOpRomSupport >> %r\n", Status));
    }

  } else {
    //
    // CrHookDxe driver entry always install CRService protocol for avoid headless platform
    // no CouOut device. please reference ForceActiveVga function.
    //
    gBS->UninstallProtocolInterface (
           mCrServiceImageHandle,
           &gConsoleRedirectionServiceProtocolGuid,
           &mDummyCRService
           );
  }

  BdsCpUnregisterHandler (mCpConOutHandle);
}

VOID
AfterConnectAllCallBack (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  )
{
  if (mCrEnable) {
    //
    // Update device's exist-status that are non-exist before,
    // or now disappear device.
    //
    UpdateDevicesExistStatus ();

    //
    // Check * device exist? if true create new variable
    //
    FindAsteriskDevicePath ();

    //
    // Free Temp Cr device variable list
    //
    FreeCrDevicesVarList ();
  }
}

/**
  The driver entry point.

  @param ImageHandle   A handle for the image that is initializing this driver
  @param SystemTable   A pointer to the EFI system table

  @retval EFI_SUCCESS:           Driver initialized successfully
  @retval  others                     Driver initialized unsuccessfully

**/
EFI_STATUS
EFIAPI
CrServiceEntryPoint (
  IN EFI_HANDLE          ImageHandle,
  IN EFI_SYSTEM_TABLE    *SystemTable
  )
{
  EFI_STATUS          Status;
  H2O_BDS_CP_HANDLE   CpHandle;

  mCrServiceImageHandle = ImageHandle;
  InitializeCrDeviceList ();

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gConsoleRedirectionServiceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mDummyCRService
                  );

  //
  // Register Callback function
  //
  Status = BdsCpRegisterHandler (
             &gH2OBdsCpConInConnectBeforeProtocolGuid,
             BeforeConnectConsoleCallback,
             H2O_BDS_CP_HIGH,
             &CpHandle
             );

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpConOutConnectBeforeProtocolGuid,
             BeforeConnectConsoleCallback,
             H2O_BDS_CP_HIGH,
             &CpHandle
             );

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpConOutConnectAfterProtocolGuid,
             AfterConnectConOutCallBack,
             H2O_BDS_CP_MEDIUM,
             &mCpConOutHandle
             );

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpConnectAllAfterProtocolGuid,
             AfterConnectAllCallBack,
             H2O_BDS_CP_MEDIUM,
             &CpHandle
             );

  return Status;
}


