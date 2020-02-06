/** @file
 This file include all platform action which can be customized by IBV/OEM.

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

#include "BdsPlatform.h"
//[-start-160308-IB10860193-add]//
#include <Guid/CustomVariable.h>
//[-Eend-160308-IB10860193-add]//

#include "FrontPage.h"
#include "OemHotKey.h"
#include "VideoOutputPortSelection.h"
#include "BootDevicesDisplay.h"
#include <Library/FlashDevicesLib.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <Guid/H2OBdsCheckPoint.h>
#include <Library/BdsCpLib.h>
#include <Guid/EventGroup.h>
#if ENBDT_PF_ENABLE  
#include <Guid/EfiSystemResourceTable.h>
#include <Protocol/SwitchableGraphicsEvent.h>
#include "SaCommonDefinitions.h"
extern EFI_GUID gAndroidCapsuleGuid;
#endif
#include <Library/HeciMsgLib.h>
#include <Library/TimerLib.h>

#define STALL_5_MILLISECOND       1000 * 5

#if TABLET_PF_ENABLE
#include <Protocol/TdtOperation.h>
#include <MkhiMsgs.h>
#include <Library/HeciMsgLib.h>
#include <Library/PcdLib.h>
#include <ScAccess.h>
#include <Library/PmicLib.h>
#include <Guid/PlatformInfo.h>
#include <Library/SpiAccessLib.h>
#include <ChipsetFota.h>

#define FIVE_SECOND               50000000

PMIC_Compliance_Item Pmic_list_a[] =
{
#if (_ERB_ON_ )
  {{PMIC_Equal,BOARD_ID_BL_RVP},   {PMIC_Any,0},   {PMIC_Greater_Equal,SocB0},   {PMIC_Equal,PMIC_DEV_ROHM},   {PMIC_Greater_Equal,PMIC_ROHM_REVID_B0},   PMIC_White_List},
  {{PMIC_Equal,BOARD_ID_BL_RVP},   {PMIC_Any,0},   {PMIC_Greater_Equal,SocB0},   {PMIC_Equal,PMIC_DEV_DIALOG},   {PMIC_Greater_Equal,PMIC_DIALOG_REVID_C0},   PMIC_White_List},
#endif
  {{PMIC_Equal,BOARD_ID_BL_FFRD}, {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   PMIC_White_List},
  {{PMIC_Equal,BOARD_ID_AV_SVP},   {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   PMIC_White_List},
  {{PMIC_Equal,BOARD_ID_BL_STHI}, {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   PMIC_White_List},
  {{PMIC_Equal,BOARD_ID_BB_RVP},   {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   {PMIC_Any,0},   PMIC_White_List}
};

EFI_STATUS
BdsLibSetKey (
  IN UINT16                    Key,
  IN HOT_KEY_CONTEXT           *HotKeyContext
  );
  
extern EFI_GUID BiosCapsuleFromAfuGuid;

extern EFI_STATUS
IFWIUpdateHack (
  IN UINT8 FotaProcess
  );
#endif

//[-start-160923-IB07400788-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160923-IB07400788-add]//

EFI_USER_PROFILE_HANDLE                           mCurrentUser = NULL;

#define DP_TYPE_MESSAGING   0x03
#define DP_SUBTYPE_USB      0x05

typedef struct {
  UINT16  VendorId;
  UINT16  DeviceId;
  UINT16  SubsystemVendorID;
  UINT16  SubsystemID;
} CHECK_VGA_ID;

CHECK_VGA_ID CheckVgaId[] = {
  {
    0x10DE,
    0x06E4,
    0x0000,
    0x0000
  }
};

VOID
EFIAPI
PlatformBdsExitPmAuthCallback (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  );

EFI_STATUS
BdsEmptyCallbackFuntion (
  IN  EFI_EVENT        Event,
  IN  void             *ParentImageHandle
  )
{
  return EFI_SUCCESS;
}

VOID
EFIAPI
SetVgaCommandRegEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  UINTN                                 IndexGop;
  UINTN                                 IndexController;
  BOOLEAN                               PrimaryExit;
  EFI_HANDLE                            PrimaryHandle;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  EFI_STATUS                            Status;
  UINT8                                 Command;
  VGA_HANDLES_INFO                      *VgaHandlesInfo;
  UINTN                                 GopHandleCount;
  EFI_HANDLE                            *GopHandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL              *ControllerDevicePath;
  EFI_DEVICE_PATH_PROTOCOL              *GopDevicePath;

  VgaHandlesInfo = (VGA_HANDLES_INFO*) Context;

  if (VgaHandlesInfo == NULL) {
    return;
  }
  //
  // Make sure Primary Handle exist.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &GopHandleCount,
                  &GopHandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    return;
  }

  PrimaryHandle = NULL;
  PrimaryExit = FALSE;

  for (IndexController = 0; IndexController < VgaHandlesInfo->VgaHandleConut; IndexController++) {
    if (PrimaryExit) {
      break;
    }

    Status = gBS->HandleProtocol (
                    VgaHandlesInfo->VgaHandleBuffer[IndexController],
                    &gEfiDevicePathProtocolGuid,
                    (VOID*)&ControllerDevicePath
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    for (IndexGop = 0; IndexGop < GopHandleCount; IndexGop++) {
      Status = gBS->HandleProtocol (GopHandleBuffer[IndexGop], &gEfiDevicePathProtocolGuid, (VOID*)&GopDevicePath);
      if (EFI_ERROR (Status)) {
        continue;
      }
      if (CompareMem (ControllerDevicePath, GopDevicePath, GetDevicePathSize (ControllerDevicePath) - END_DEVICE_PATH_LENGTH) == 0) {
        PrimaryHandle = VgaHandlesInfo->VgaHandleBuffer[IndexController];
        if (PrimaryHandle == VgaHandlesInfo->PrimaryVgaHandle) {
          PrimaryExit = TRUE;
          break;
        }
      }
    }
  }

  if (PrimaryHandle == NULL) {
    goto EventExit;
  }

  Command = 0;

  for (IndexController = 0; IndexController < VgaHandlesInfo->VgaHandleConut; IndexController++) {
    if (VgaHandlesInfo->VgaHandleBuffer[IndexController] != PrimaryHandle) {
      //
      // Disconnect the driver of VgaHandle.
      //
      gBS->DisconnectController (VgaHandlesInfo->VgaHandleBuffer[IndexController], NULL, NULL);
      //
      // Clear the command register of Vga device.
      //
      Status = gBS->HandleProtocol (VgaHandlesInfo->VgaHandleBuffer[IndexController], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);

      if (!EFI_ERROR(Status)) {
        PciIo->Pci.Write (PciIo, EfiPciIoWidthUint16, 0x04, 1, &Command);
      }
    }
  }

EventExit:
  gBS->FreePool (GopHandleBuffer);

  return;
}


//
// BDS Platform Functions
//
/**
  Platform Bds init. Include the platform firmware vendor, revision
  and so crc check.

**/
VOID
EFIAPI
PlatformBdsInit (
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
  gST->FirmwareRevision = PcdGet32 (PcdFirmwareRevision);

  //
  // Fixup Tasble CRC after we updated Firmware Vendor and Revision
  //
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 ((VOID *)gST, gST->Hdr.HeaderSize, &Crc);
  gST->Hdr.CRC32 = Crc;

  InitializeFrontPage (TRUE);

  //
  // Before user authentication, the user identification devices need be connected 
  // from the platform customized device paths
  //
  PlatformBdsConnectAuthDevice ();

  //
  // As console is not ready, the auto logon user will be identified.
  //
  BdsLibUserIdentify (&mCurrentUser); 
}


/**
  Connect RootBridge

  @retval EFI_SUCCESS   Connect RootBridge successfully.
  @retval EFI_STATUS    Connect RootBridge fail.

**/
EFI_STATUS
ConnectRootBridge (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                RootHandle;
  
  //
  // Make all the PCI_IO protocols on PCI Seg 0 show up
  //
  BdsLibConnectDevicePath (gPlatformRootBridges[0]);

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &gPlatformRootBridges[0],
                  &RootHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // PostCode = 0x13, PCI enumeration
  //
  POST_CODE (BDS_PCI_ENUMERATION_START);
  Status = gBS->ConnectController (RootHandle, NULL, NULL, FALSE);
  //
  // PostCode = 0x15, PCI enumeration complete
  //
  POST_CODE (BDS_PCI_ENUMERATION_END);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
GetAllVgaHandles (
  OUT EFI_HANDLE  **VgaHandleBuffer,
  OUT UINTN       *VgaHandleCount
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            *HandleBuffer;
  UINTN                                 HandleCount;
  UINTN                                 HandleIndex;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  PCI_TYPE00                            Pci;
  EFI_HANDLE                            *TempVgaHandleBuffer;
  UINTN                                 BufferIndex;

  HandleBuffer        = NULL;
  HandleCount         = 0;
  BufferIndex         = 0;
  //
  // Start PciBus.
  //
  ConnectRootBridge ();
#if ENBDT_PF_ENABLE  
  if (FeaturePcdGet (PcdSwitchableGraphicsSupported)) {
    EFI_HANDLE                          SgHandle;
    //
    // Install Switchable Graphics Protocol to trigger
    // Switchable Graphics DXE driver registered callback function.
    //
    SgHandle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &SgHandle,
                    &gH2OSwitchableGraphicsEventProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
  }
#endif  
  //
  // check all the pci io to find all possible VGA devices
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  (HandleCount * sizeof(EFI_HANDLE)),
                  (VOID **) &TempVgaHandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    Status = gBS->HandleProtocol (HandleBuffer[HandleIndex], &gEfiPciIoProtocolGuid, (VOID **)&PciIo);
    if (!EFI_ERROR (Status)) {
      //
      // Check for all VGA device
      //
      Status = PciIo->Pci.Read (
                            PciIo,
                            EfiPciIoWidthUint32,
                            0,
                            sizeof (Pci) / sizeof (UINT32),
                            &Pci
                            );
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (!IS_PCI_VGA (&Pci)) {
        continue;
      }

      TempVgaHandleBuffer[BufferIndex] = HandleBuffer[HandleIndex];
      BufferIndex++;
    }
  }

  if (BufferIndex == 0) {
    return EFI_NOT_FOUND;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  (BufferIndex * sizeof(EFI_HANDLE)),
                  (VOID **) VgaHandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CopyMem (*VgaHandleBuffer, TempVgaHandleBuffer, (BufferIndex * sizeof(EFI_HANDLE)));

  *VgaHandleCount = BufferIndex;

  gBS->FreePool (HandleBuffer);
  gBS->FreePool (TempVgaHandleBuffer);

  return EFI_SUCCESS;
}


EFI_STATUS
GetGopDevicePath (
   IN  EFI_DEVICE_PATH_PROTOCOL *PciDevicePath,
   OUT EFI_DEVICE_PATH_PROTOCOL **GopDevicePath
   )
{
  UINTN                           Index;
  EFI_STATUS                      Status;
  EFI_HANDLE                      PciDeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL        *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL        *TempPciDevicePath;
  UINTN                           GopHandleCount;
  EFI_HANDLE                      *GopHandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath;

  RemainingDevicePath = NULL;

  if (PciDevicePath == NULL || GopDevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Initialize the GopDevicePath to be PciDevicePath
  //
  *GopDevicePath    = PciDevicePath;
  TempPciDevicePath = PciDevicePath;

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &TempPciDevicePath,
                  &PciDeviceHandle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Try to connect this handle, so that GOP driver could start on this
  // device and create child handles with GraphicsOutput Protocol installed
  // on them, then we get device paths of these child handles and select
  // them as possible console device.
  //
  PlatformBdsDisplayPortSelection (PciDeviceHandle, &RemainingDevicePath);
#if TABLET_PF_ENABLE  
  gBS->Stall (50000);
#endif  
  Status = gBS->ConnectController (PciDeviceHandle, NULL, RemainingDevicePath, FALSE);
  if (EFI_ERROR(Status)) {
    //
    // In uefi boot mode If RemainingDevicePath != NULL, and connected failed.
    //
    PlatformBdsVgaConnectedFailCallback (PciDeviceHandle, RemainingDevicePath);
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &GopHandleCount,
                  &GopHandleBuffer
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Add all the child handles as possible Console Device
    //
    for (Index = 0; Index < GopHandleCount; Index++) {
      Status = gBS->HandleProtocol (GopHandleBuffer[Index], &gEfiDevicePathProtocolGuid, (VOID*)&TempDevicePath);
      if (EFI_ERROR (Status)) {
        continue;
      }
      if (CompareMem (
            PciDevicePath,
            TempDevicePath,
            GetDevicePathSize (PciDevicePath) - END_DEVICE_PATH_LENGTH
            ) == 0) {
        //
        // In current implementation, we only enable one of the child handles
        // as console device, i.e. sotre one of the child handle's device
        // path to variable "ConOut"
        // In futhure, we could select all child handles to be console device
        //

        *GopDevicePath = TempDevicePath;

        //
        // Delete the PCI device's path that added by GetPlugInPciVgaDevicePath()
        // Add the integrity GOP device path.
        //
        BdsLibUpdateConsoleVariable (L"ConOutDev", NULL, PciDevicePath);
        BdsLibUpdateConsoleVariable (L"ConOutDev", TempDevicePath, NULL);
        BdsLibUpdateConsoleVariable (CON_OUT_CANDIDATE_NAME, NULL, PciDevicePath);
        BdsLibUpdateConsoleVariable (CON_OUT_CANDIDATE_NAME, TempDevicePath, NULL);
      }
    }
    gBS->FreePool (GopHandleBuffer);
  }

  return EFI_SUCCESS;
}


/**
  Add PCI VGA to ConOut.
  PCI VGA: 03 00 00

  @param  DeviceHandle    Handle of PCIIO protocol.

  @retval EFI_SUCCESS     PCI VGA is added to ConOut.
  @retval EFI_STATUS      No PCI VGA device is added.

**/
EFI_STATUS
PreparePciVgaDevicePath (
  IN EFI_HANDLE                DeviceHandle,
  IN EFI_DEVICE_PATH_PROTOCOL  **GopDevicePath
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  DevicePath = NULL;
  Status = gBS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GetGopDevicePath (DevicePath, GopDevicePath);

  return EFI_SUCCESS;
}


EFI_HANDLE
SelectVgaHandle (
  IN VGA_DEVICE_INFO                    *VgaDeviceList,
  IN UINTN                              VgaHandleCount
  )
{
  EFI_STATUS                            Status;
  UINT8                                 CheckSequenceIndex;
  UINT8                                 SelectedVgaIndex;
  UINT8                                 ListIndex;
  EFI_SETUP_UTILITY_PROTOCOL            *SetupUtility;
  CHIPSET_CONFIGURATION                  *SystemConfiguration;
  POSSIBLE_VGA_TYPE                     CheckSequence[] = {Igfx, Peg, Pcie, Pci};

  SystemConfiguration = NULL;
  SelectedVgaIndex = 0xFF;

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  if (!EFI_ERROR (Status)) {
    SystemConfiguration = (CHIPSET_CONFIGURATION *)SetupUtility->SetupNvData;
  }

  if ((SystemConfiguration->PrimaryVideoAdaptor == DisplayModeIgfx) ||
      (SystemConfiguration->PrimaryVideoAdaptor == DisplayModeSg)) {
    CheckSequence[0] = Igfx;
    CheckSequence[1] = Peg;
    CheckSequence[2] = Pcie;
    CheckSequence[3] = Pci;
  }
  if ((SystemConfiguration->PrimaryVideoAdaptor == DisplayModePeg) ||
      (SystemConfiguration->PrimaryVideoAdaptor == DisplayModeAuto)) {
    CheckSequence[0] = Peg;
    CheckSequence[1] = Pcie;
    CheckSequence[2] = Pci;
    CheckSequence[3] = Igfx;
  }
  if (SystemConfiguration->PrimaryVideoAdaptor == DisplayModePci) {
    CheckSequence[0] = Pci;
    CheckSequence[1] = Pcie;
    CheckSequence[2] = Peg;
    CheckSequence[3] = Igfx;
  }

  for (CheckSequenceIndex = 0; CheckSequenceIndex < PossibleVgaTypeMax; CheckSequenceIndex++) {
    for (ListIndex = 0; ListIndex < VgaHandleCount; ListIndex++) {
      if ((VgaDeviceList[ListIndex].VgaType == CheckSequence[CheckSequenceIndex]) &&
          ((SelectedVgaIndex == 0xFF) ||
           ((SelectedVgaIndex != 0xFF) &&
            (VgaDeviceList[ListIndex].Priority < VgaDeviceList[SelectedVgaIndex].Priority)))) {
        SelectedVgaIndex = ListIndex;
      }
    }
    if (SelectedVgaIndex != 0xFF) {
      return VgaDeviceList[SelectedVgaIndex].Handle;
    }
  }

  return NULL;
}


EFI_HANDLE
ClassifyVgaHandleAndSelect (
  IN EFI_HANDLE                         *PciVgaHandleBuffer,
  IN UINTN                              PciVgaHandleCount
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  VGA_DEVICE_INFO                       *VgaDeviceList;
  UINTN                                 VgaHandleCount;
  BOOLEAN                               FoundFlag;
  EFI_HANDLE                            SelectedVgaHandle;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       *PciIoRootBridge;
  UINT64                                Address;
  UINT8                                 *TmpPtr;
  UINT8                                 IndexCheckVgaId;
  UINT8                                 Bus, Device, Function;
  PCI_TYPE00                            PciType;
  UINT16                                DeviceType;
  UINT16                                VendorId, DeviceId, SubsystemVendorID, SubsystemID;
  UINT8                                 SecondaryBus, SubordinateBus;
  UINT8                                 CapabilityPtr, CapabilityId;
  UINT8                                 DevicePathLen;
  BOOLEAN                               FindVga;
  BOOLEAN                               FindIgfx;
  BOOLEAN                               FindPeg;
  BOOLEAN                               FindPci;

  FindIgfx = FALSE;
  FindPeg = FALSE;
  FindPci = FALSE;
  VendorId = 0;
  DeviceId = 0;
  SubsystemVendorID = 0;
  SubsystemID = 0;

  Status = gBS->LocateProtocol (
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  (VOID **)&PciIoRootBridge
                  );
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  SelectedVgaHandle = NULL;
  VgaHandleCount = PciVgaHandleCount;
  if (VgaHandleCount == 0) {
    return NULL;
  }

  if (VgaHandleCount == 1) {
    return PciVgaHandleBuffer[0];
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  (VgaHandleCount * sizeof(VGA_DEVICE_INFO)),
                  (VOID **) &VgaDeviceList
                  );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  for (Index = 0; Index < VgaHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    PciVgaHandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID **)&DevicePath
                    );

    VgaDeviceList[Index].Handle = PciVgaHandleBuffer[Index];
    FoundFlag = FALSE;

    if (!FindIgfx || !FindPeg || !FindPci) {
      FindVga        = FALSE;
      SecondaryBus   = 0;
      SubordinateBus = 0;
      Bus            = 0;
      //
      // Get value from DevicePath
      //
      TmpPtr   = (UINT8 *) DevicePath;
      //
      //ignore PciRootBridge device path
      //
      TmpPtr  += sizeof(UINT16);
      DevicePathLen = *(UINT8 *) TmpPtr;
      TmpPtr += (DevicePathLen - sizeof(UINT16));
      //
      //jump to DevicePath->Function
      //
      TmpPtr  += sizeof(UINT32);
      Function = *(UINT8 *) TmpPtr;
      TmpPtr  += sizeof(UINT8);
      Device   = *(UINT8 *) TmpPtr;
      while (!FindVga) {
        for (Bus = SecondaryBus; Bus <= SubordinateBus; Bus++) {
          Address = EFI_PCI_ADDRESS (Bus, Device, Function, 0);
          PciIoRootBridge->Pci.Read (PciIoRootBridge, EfiPciIoWidthUint32, Address,sizeof (PciType) / sizeof (UINT32), &PciType);
          if (PciType.Hdr.ClassCode[2] == PCI_CLASS_DISPLAY && PciType.Hdr.ClassCode[1] == PCI_CLASS_DISPLAY_VGA) {
            FindVga = TRUE;
            VendorId          = PciType.Hdr.VendorId;
            DeviceId          = PciType.Hdr.DeviceId;
            SubsystemVendorID = PciType.Device.SubsystemVendorID;
            SubsystemID       = PciType.Device.SubsystemID;
            break;
          } else if (PciType.Hdr.ClassCode[2] == PCI_CLASS_BRIDGE && PciType.Hdr.ClassCode[1] == PCI_CLASS_BRIDGE_P2P) {
            Address = EFI_PCI_ADDRESS (Bus, Device, Function, PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
            PciIoRootBridge->Pci.Read (PciIoRootBridge, EfiPciIoWidthUint8, Address, 1, &SecondaryBus);
            Address = EFI_PCI_ADDRESS (Bus, Device, Function, PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
            PciIoRootBridge->Pci.Read (PciIoRootBridge, EfiPciIoWidthUint8, Address, 1, &SubordinateBus);
            //
            //To check device type
            //
            TmpPtr  += sizeof (UINT8);
            do {
              DeviceType = *(UINT16 *) TmpPtr;
              if (DeviceType == 0x0101) {
                TmpPtr  += sizeof (UINT32);
                Function = *(UINT8 *) TmpPtr;
                TmpPtr  += sizeof (UINT8);
                Device   = *(UINT8 *) TmpPtr;
              } else if (DeviceType == 0x7FFF) {
                return NULL;
              } else {
                TmpPtr  += sizeof (UINT16);
                DevicePathLen = *(UINT8 *) TmpPtr;
                TmpPtr += (DevicePathLen - sizeof(UINT16));
              }
            } while (DeviceType != 0x0101);
          }
        }
      }
      if (FindVga) {
        if (Bus <= 0 && !FindIgfx) {
          VgaDeviceList[Index].VgaType = Igfx;
          VgaDeviceList[Index].Priority = 0;
          FindIgfx = TRUE;
          continue;
        }
        CapabilityId  = 0xFF;
        CapabilityPtr = 0xFF;
        //
        //To check VGA is PCI or PCIe
        //
        Address = EFI_PCI_ADDRESS (Bus, Device, Function, PCI_CAPBILITY_POINTER_OFFSET);
        PciIoRootBridge->Pci.Read (PciIoRootBridge, EfiPciIoWidthUint8, Address, 1, &CapabilityPtr);
        while (CapabilityId != 0x10 && CapabilityPtr != 0) {
          Address = EFI_PCI_ADDRESS (Bus, Device, Function, CapabilityPtr);
          PciIoRootBridge->Pci.Read (PciIoRootBridge, EfiPciIoWidthUint8, Address, 1, &CapabilityId);
          Address = EFI_PCI_ADDRESS (Bus, Device, Function, CapabilityPtr + 1);
          PciIoRootBridge->Pci.Read (PciIoRootBridge, EfiPciIoWidthUint8, Address, 1, &CapabilityPtr);
        }
        //
        // Capability ID = 0x10, the device is PCIe
        //
        if (CapabilityId == 0x10) {
          for (IndexCheckVgaId = 0; IndexCheckVgaId < sizeof (CheckVgaId) / sizeof (CHECK_VGA_ID); IndexCheckVgaId++) {
            if (VendorId == CheckVgaId[IndexCheckVgaId].VendorId && \
                DeviceId == CheckVgaId[IndexCheckVgaId].DeviceId && \
                SubsystemVendorID == CheckVgaId[IndexCheckVgaId].SubsystemVendorID && \
                SubsystemID == CheckVgaId[IndexCheckVgaId].SubsystemID) {
              VgaDeviceList[Index].VgaType = Pci;
              VgaDeviceList[Index].Priority = 0;
              FindPci = TRUE;
              continue;
            }
          }
          if (!FindPeg) {
            VgaDeviceList[Index].VgaType = Peg;
            VgaDeviceList[Index].Priority = 0;
            FindPeg = TRUE;
            continue;
          }
        } else {
          if (!FindPci) {
            VgaDeviceList[Index].VgaType = Pci;
            VgaDeviceList[Index].Priority = 0;
            FindPci = TRUE;
            continue;
          }
        }
      }
    }

    //
    // VGA Information Default Setting
    //
    VgaDeviceList[Index].VgaType = Pci;
    VgaDeviceList[Index].Priority = 100;
  }

  SelectedVgaHandle = SelectVgaHandle (VgaDeviceList, VgaHandleCount);

  gBS->FreePool (VgaDeviceList);

  return SelectedVgaHandle;
}


EFI_STATUS
LockVgaControllerVgaDriverPolicy (
  IN EFI_HANDLE       SelectedVgaHandle,
  IN EFI_HANDLE       *VgaHandleBuffer,
  IN UINTN            VgaHandleCount
  )
{
  UINTN                          DriverBindingHandleCount;
  EFI_HANDLE                     *DriverBindingHandleBuffer;
  EFI_STATUS                     Status;
  UINTN                          Index;
  CHAR16                         *DriverName;
  EFI_COMPONENT_NAME2_PROTOCOL   *ComponentNameInterface;
  EFI_HANDLE                     LockGopImageHandleList[2];
  EFI_DEVICE_PATH_PROTOCOL       *SelectedVgaDevicePath;

  LockGopImageHandleList[0] = NULL;
  LockGopImageHandleList[1] = NULL;
  //
  // Get VgaDriverPolicy driver by Component name.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDriverBindingProtocolGuid,
                  NULL,
                  &DriverBindingHandleCount,
                  &DriverBindingHandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < DriverBindingHandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    DriverBindingHandleBuffer[Index],
                    &gEfiComponentName2ProtocolGuid,
                    (VOID **)&ComponentNameInterface
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = ComponentNameInterface->GetDriverName (
                                       ComponentNameInterface,
                                       LANGUAGE_CODE_ENGLISH_RFC4646,
                                       &DriverName
                                       );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (StrCmp (DriverName, VGA_DRIVER_POLICY_NAME) == 0) {
      LockGopImageHandleList[0] = DriverBindingHandleBuffer[Index];
      break;
    }
  }

  FreePool (DriverBindingHandleBuffer);
  //
  // Get VgaDriverPolicy?
  //
  if (LockGopImageHandleList[0] == NULL) {
    return EFI_NOT_FOUND;
  }
  //
  // Create the volatile variable ACTIVE_VGA_VAR_NAME
  // to notice VgaPolicyDriver to lock the non-Active Vga.
  //
  Status = gBS->HandleProtocol (
                  SelectedVgaHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **)&SelectedVgaDevicePath
                  );

  BdsLibUpdateConsoleVariable (ACTIVE_VGA_VAR_NAME, SelectedVgaDevicePath, NULL);
  //
  // lock none-primary Vga device.
  //
  if (!EFI_ERROR(Status)) {
    //
    // Add the NULL handle to denote the terminal of driver list.
    //
    LockGopImageHandleList[1] = NULL;
    for (Index = 0; Index < VgaHandleCount; Index++) {
      if (VgaHandleBuffer[Index] != SelectedVgaHandle) {
        Status = gBS->ConnectController (VgaHandleBuffer[Index], LockGopImageHandleList, NULL, FALSE);
      }
    }
  }

  return Status;
}


VOID
EFIAPI
GopHotPlugCallback (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  EFI_HANDLE                             *HandleBuffer;
  UINTN                                  NumberOfHandles;
  UINTN                                  Index;
  EFI_DEVICE_PATH_PROTOCOL               *GopDevicePath;
  EFI_STATUS                             Status;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID*)&GopDevicePath
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    BdsLibUpdateConsoleVariable (L"ConOut", GopDevicePath, NULL);
  }

  BdsLibConnectConsoleVariable (L"ConOut");

  gBS->FreePool (HandleBuffer);
}


/**
  Find the platform active vga, and base on the policy to enable the vga as the console out device.
  The policy is driven by one setup variable "VBIOS".

  @retval EFI_UNSUPPORTED   There is no active vga device
  @retval EFI_STATUS        Return the status of BdsLibGetVariableAndSize ()

**/
EFI_STATUS
PlatformBdsForceActiveVga (
  VOID
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  EFI_HANDLE                            *VgaHandleBuffer;
  UINTN                                 VgaHandleCount;
  EFI_DEVICE_PATH_PROTOCOL              *ConOutDevPath;
  EFI_HANDLE                            *SelectedVgaHandle;
  VGA_HANDLES_INFO                      *VgaHandlesInfo;
  EFI_SETUP_UTILITY_PROTOCOL            *SetupUtility;
  CHIPSET_CONFIGURATION                 *SystemConfiguration;
  EFI_EVENT                             ExitBootServicesEvent;
  EFI_EVENT                             LegacyBootEvent;
  EFI_EVENT                             GopHotPlugEvent;
  VOID                                  *Registration;

  SystemConfiguration = NULL;
  VgaHandleBuffer     = NULL;
  VgaHandleCount      = 0;
  ConOutDevPath       = NULL;

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  if (!EFI_ERROR (Status)) {
    SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;
  }

  Status = GetAllVgaHandles (&VgaHandleBuffer, &VgaHandleCount);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  SelectedVgaHandle = ClassifyVgaHandleAndSelect (
                        VgaHandleBuffer,
                        VgaHandleCount
                        );
  if ((SystemConfiguration->UefiDualVgaControllers == DUAL_VGA_CONTROLLER_ENABLE) &&
      (BdsLibGetBootType ()                        == EFI_BOOT_TYPE)) {
    for (Index = 0; Index < VgaHandleCount; Index++) {
      PreparePciVgaDevicePath (VgaHandleBuffer[Index], &ConOutDevPath);
    }
  } else {
    PreparePciVgaDevicePath (SelectedVgaHandle, &ConOutDevPath);
  }

  //
  // Register Event for Monitor Hot-plug
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  GopHotPlugCallback,
                  NULL,
                  &GopHotPlugEvent
                  );
  if (!EFI_ERROR (Status)) {
    Status = gBS->RegisterProtocolNotify (
                    &gEfiGraphicsOutputProtocolGuid,
                    GopHotPlugEvent,
                    &Registration
                    );
  }

  //
  // In Uefi Boot mode and the all the native GOP are connected failed.
  // We rollback the CSM and try to connect the UefiBiosVideo.
  //
  LegacyBiosDependency (NULL);

  if (VgaHandleCount <= 1) {
    goto Exit;
  }

  if (SystemConfiguration->UefiDualVgaControllers != DUAL_VGA_CONTROLLER_ENABLE) {
    LockVgaControllerVgaDriverPolicy (SelectedVgaHandle, VgaHandleBuffer, VgaHandleCount);
  }

  VgaHandlesInfo = AllocateZeroPool (VgaHandleCount * sizeof(VGA_HANDLES_INFO));
  if (VgaHandlesInfo != NULL) {
    VgaHandlesInfo->VgaHandleBuffer = AllocateZeroPool (VgaHandleCount * sizeof(EFI_HANDLE));
    if (VgaHandlesInfo->VgaHandleBuffer == NULL) {
      goto Exit;
    }
  } else {
    goto Exit;
  }

  VgaHandlesInfo->PrimaryVgaHandle = SelectedVgaHandle;
  VgaHandlesInfo->VgaHandleConut   = VgaHandleCount;
  CopyMem (VgaHandlesInfo->VgaHandleBuffer, VgaHandleBuffer, (VgaHandleCount * sizeof(EFI_HANDLE)));

  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_NOTIFY,
                  SetVgaCommandRegEvent,
                  VgaHandlesInfo,
                  &ExitBootServicesEvent
                  );
  Status = EfiCreateEventLegacyBootEx (
             TPL_NOTIFY,
             SetVgaCommandRegEvent,
             VgaHandlesInfo,
             &LegacyBootEvent
             );

Exit:
  if (VgaHandleBuffer) {
    gBS->FreePool (VgaHandleBuffer);
  }

  return Status;
}


/**
  Compare two device pathes

  @param  DevicePath1   Input device pathes.
  @param  DevicePath2   Input device pathes.

  @retval TRUE          Same.
  @retval FALSE         Different.

**/
BOOLEAN
CompareDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL *DevicePath2
  )
{
  UINTN Size1;
  UINTN Size2;

  Size1 = GetDevicePathSize (DevicePath1);
  Size2 = GetDevicePathSize (DevicePath2);

  if (Size1 != Size2) {
    return FALSE;
  }

  if (CompareMem (DevicePath1, DevicePath2, Size1)) {
    return FALSE;
  }

  return TRUE;
}


/**
  Connect the predefined platform default console device. Always try to find
  and enable the vga device if have.

  @param PlatformConsole          Predefined platform default console device array.

  @retval EFI_SUCCESS             Success connect at least one ConIn and ConOut
                                  device, there must have one ConOut device is
                                  active vga device.
  @return Return the status of BdsLibConnectAllDefaultConsoles ()

**/
EFI_STATUS
PlatformBdsConnectConsole (
  IN BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  )
{
  EFI_STATUS                         Status;
  UINTN                              Index;
  EFI_DEVICE_PATH_PROTOCOL           *VarConout;
  EFI_DEVICE_PATH_PROTOCOL           *VarConin;
  EFI_DEVICE_PATH_PROTOCOL           *VarErrout;
  EFI_DEVICE_PATH_PROTOCOL           *NewVarConout;
  EFI_DEVICE_PATH_PROTOCOL           *NewVarConin;
  EFI_DEVICE_PATH_PROTOCOL           *NewVarErrout;
  UINTN                              DevicePathSize;
  EFI_SETUP_UTILITY_PROTOCOL         *SetupUtility;
  CHIPSET_CONFIGURATION              *SystemConfiguration;
//[-start-151124-IB07220021-modify]//
//  EFI_CONSOLE_REDIRECTION_BDS_HOOK_PROTOCOL  *CRBdsHookProtocol;
//  EFI_STATUS                                 CRBdsHookProtocolStatus;
  VOID                                       *CrService;
//[-end-151124-IB07220021-modify]//
  EFI_DEVICE_PATH_PROTOCOL                   *UsbConsoleOuputDevPath = NULL;

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;

  Index = 0;
  Status = EFI_SUCCESS;
  DevicePathSize = 0;

  VarConout = BdsLibGetVariableAndSize (
                L"ConOut",
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );
  VarConin = BdsLibGetVariableAndSize (
               L"ConIn",
               &gEfiGlobalVariableGuid,
               &DevicePathSize
               );
  VarErrout = BdsLibGetVariableAndSize (
                L"ErrOut",
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );
  if (VarConout == NULL || VarConin == NULL) {
    //
    // Have chance to connect the platform default console,
    // the platform default console is the minimue device group the platform should support
    //
    while (PlatformConsole[Index].DevicePath != NULL) {
      //
      // Update the console variable with the connect type
      //
      if ((PlatformConsole[Index].ConnectType & CONSOLE_IN) == CONSOLE_IN) {
        BdsLibUpdateConsoleVariable (L"ConIn"             , PlatformConsole[Index].DevicePath, NULL);
        BdsLibUpdateConsoleVariable (CON_IN_CANDIDATE_NAME, PlatformConsole[Index].DevicePath, NULL);
      }

      if ((PlatformConsole[Index].ConnectType & CONSOLE_OUT) == CONSOLE_OUT) {
        BdsLibUpdateConsoleVariable (L"ConOut"             , PlatformConsole[Index].DevicePath, NULL);
        BdsLibUpdateConsoleVariable (CON_OUT_CANDIDATE_NAME, PlatformConsole[Index].DevicePath, NULL);
      }

      if ((PlatformConsole[Index].ConnectType & STD_ERROR) == STD_ERROR) {
        BdsLibUpdateConsoleVariable (L"ErrOut", PlatformConsole[Index].DevicePath, NULL);
        BdsLibUpdateConsoleVariable (ERR_OUT_CANDIDATE_NAME, PlatformConsole[Index].DevicePath, NULL);
      }

      Index ++;
    }
  }

  if (VarConin) {
    BdsLibUpdateConsoleVariable (CON_IN_CANDIDATE_NAME, VarConin, NULL);
  }

  if (VarErrout) {
    BdsLibUpdateConsoleVariable (ERR_OUT_CANDIDATE_NAME, VarErrout, NULL);
  }


//[-start-161121-IB10860215-add]//
//[-start-161126-IB07400819-remove]//
//  NewVarConin = BdsLibGetVariableAndSize (
//                  CON_IN_CANDIDATE_NAME,
//                  &gEfiGenericVariableGuid,
//                  &DevicePathSize
//                  );
//
//  if (NewVarConin != NULL && !CompareDevicePath(NewVarConin,VarConin)) {
//    gRT->SetVariable (
//           L"ConIn",
//           &gEfiGlobalVariableGuid,
//           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
//           GetDevicePathSize (NewVarConin),
//           NewVarConin
//           );
//  }
//  if (NewVarConin != NULL) {
//    gBS->FreePool (NewVarConin);
//  }
//  if (VarConin) {
//    gBS->FreePool (VarConin);
//  }
//
//  //
//  // PostCode = 0x16, Keyboard Controller, Keyboard and Moust initial
//  //
//  POST_CODE (BDS_CONNECT_CONSOLE_IN);
//  //
//  // Because possibly the platform is legacy free, in such case,
//  // ConIn devices (Serial Port and PS2 Keyboard ) does not exist, so we need not check the status.
//  //
//  BdsLibConnectConsoleVariable (L"ConIn");
//[-end-161126-IB07400819-remove]//
//[-end-161121-IB10860215-add]//

  //
  // If requested via ASF Intel OEM boot parameters, then we connect the SOL driver.
  // ASF Keyboard locking does not apply here
  //

  //
  // Make sure we have at least one active VGA, and have the right active VGA in console variable
  //
  Status = PlatformBdsForceActiveVga ();
//[-start-151124-IB07220021-modify]//
  if (EFI_ERROR(Status)) {
    Status = gBS->LocateProtocol (&gConsoleRedirectionServiceProtocolGuid, NULL, &CrService);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
//[-end-151124-IB07220021-modify]//

  //
  // Update default device paths with candicate device path
  //
  NewVarConout = BdsLibGetVariableAndSize (
                   CON_OUT_CANDIDATE_NAME,
                   &gEfiGenericVariableGuid,
                   &DevicePathSize
                   );
//[-start-160204-IB07400707-modify]//
//  ASSERT(NewVarConout != NULL);
  if (NewVarConout != NULL) {
    if (!CompareDevicePath(NewVarConout, VarConout)) {
      gRT->SetVariable (
             L"ConOut",
             &gEfiGlobalVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             GetDevicePathSize (NewVarConout),
             NewVarConout
             );
    }
    gBS->FreePool (NewVarConout);
  }
//[-end-160204-IB07400707-modify]//
  if (VarConout) {
    gBS->FreePool (VarConout);
  }
  
//[-start-161121-IB10860215-remove]//
//[-start-161126-IB07400819-modify]//
  NewVarConin = BdsLibGetVariableAndSize (
                  CON_IN_CANDIDATE_NAME,
                  &gEfiGenericVariableGuid,
                  &DevicePathSize
                  );
  if (NewVarConin != NULL && !CompareDevicePath(NewVarConin,VarConin)) {
    gRT->SetVariable (
           L"ConIn",
           &gEfiGlobalVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           GetDevicePathSize (NewVarConin),
           NewVarConin
           );
  }
  if (NewVarConin != NULL) {
    gBS->FreePool (NewVarConin);
  }
  if (VarConin) {
    gBS->FreePool (VarConin);
  }
//[-end-161126-IB07400819-modify]//
//[-end-161121-IB10860215-remove]//

  NewVarErrout = BdsLibGetVariableAndSize (
                   ERR_OUT_CANDIDATE_NAME,
                   &gEfiGenericVariableGuid,
                   &DevicePathSize
                   );
  if (NewVarErrout != NULL) {
    if (!CompareDevicePath(NewVarErrout,VarErrout)) {
      gRT->SetVariable (
             L"ErrOut",
             &gEfiGlobalVariableGuid,
             EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
             GetDevicePathSize (NewVarErrout),
             NewVarErrout
             );
    }
    gBS->FreePool (NewVarErrout);
  }
  if (VarErrout != NULL) {
    gBS->FreePool (VarErrout);
  }

  BdsLibConnectI2cDevice ();
  if (!(SystemConfiguration->UsbHotKeySupport == 0 && BdsLibIsWin8FastBootActive ())) {
    BdsLibConnectUsbHID ();
    if (FoundUSBConsoleOutput (&UsbConsoleOuputDevPath)) {
      SetUsbConsoleOutToConOutVar (UsbConsoleOuputDevPath);
      FreePool (UsbConsoleOuputDevPath);
    }
  } else {
    BdsLibConnectUsbHIDNotifyRegister ();
  }
  //
  // Connect the all the default console with current console variable
  //
  Status = BdsLibConnectAllDefaultConsoles ();

  return Status;
}

/**
  Connect with predefined platform connect sequence,
  the OEM/IBV can customize with their own connect sequence.
**/
VOID
PlatformBdsConnectSequence (
  VOID
  )
{
  UINTN                     Index;

  Index = 0;

  //
  // Here we can get the customized platform connect sequence
  // Notes: we can connect with new variable which record the last time boots connect device path sequence
  //
  while (gPlatformConnectSequence[Index] != NULL) {
    //
    // Build the platform boot option
    //
    BdsLibConnectDevicePath (gPlatformConnectSequence[Index]);
    Index ++;
  }

  //
  // For the debug tip, just use the simple policy to connect all devices
  //
  BdsLibConnectAll ();
}

/**
  Load the predefined driver option, OEM/IBV can customize this
  to load their own drivers

  @param BdsDriverLists  - The header of the driver option link list.

**/
VOID
PlatformBdsGetDriverOption (
  IN OUT LIST_ENTRY              *BdsDriverLists
  )
{
  UINTN                              Index;

  Index = 0;

  //
  // Here we can get the customized platform driver option
  //
  while (gPlatformDriverOption[Index] != NULL) {
    //
    // Build the platform boot option
    //
    BdsLibRegisterNewOption (BdsDriverLists, gPlatformDriverOption[Index], NULL, L"DriverOrder", NULL, 0);
    Index ++;
  }
}


/**
  Internal function to initalize H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL data.

  @param[in]  QuietBoot           Boolean value to indicate quiet boot is enabled or disabled.
  @param[out] CpData              A Pointer to H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL instance to initialized.

  @retval EFI_SUCCESS             Init H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL data successfully.
  @retval EFI_INVALID_PARAMETER   CpData is NULL.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory for H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL interface failed.
**/
STATIC
EFI_STATUS
InitBeforeDisplayData (
  IN  BOOLEAN                                QuietBoot,
  OUT H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL    **CpData
  )
{
  EFI_STATUS                           Status;
  VOID                                 *Interface;
  H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL   *BeforeDisplay;

  if (CpData == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  BeforeDisplay = AllocateZeroPool (sizeof (H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL));
  if (BeforeDisplay == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  ZeroMem (BeforeDisplay, sizeof (H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL));
  BeforeDisplay->Size   = sizeof (H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL);
  BeforeDisplay->Status = H2O_BDS_TASK_NORMAL;
  if (QuietBoot) {
    BeforeDisplay->Features |= H2O_BDS_CP_DISPLAY_LOGO;
    Status = gBS->LocateProtocol (&gEfiBootLogoProtocolGuid, NULL, (VOID **) &Interface);
    if (!EFI_ERROR (Status)) {
      BeforeDisplay->Features |= H2O_BDS_CP_DISPLAY_BGRT;
    }
  } else {
    BeforeDisplay->Features |= H2O_BDS_CP_DISPLAY_INFO;
  }
  *CpData = BeforeDisplay;
  return EFI_SUCCESS;
}

/**
  Internal function to initalize H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpDisplayBeforeProtocolGuid checkpoint.

  @param[in] QuietBoot            Boolean value to indicate quiet boot is enabled or disabled.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpDisplayBeforeProtocolGuid checkpoint successfully.
  @retval EFI_ALREADY_STARTED     The gH2OBdsCpDisplayBeforeProtocolGuid checkpoint has been triggered.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpDisplayBeforeProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpDisplayBefore (
  IN  BOOLEAN       QuietBoot
  )
{
  EFI_STATUS                           Status;
  H2O_BDS_CP_DISPLAY_BEFORE_PROTOCOL   *BeforeDisplay;

  Status = gBS->LocateProtocol (
                  &gH2OBdsCpDisplayBeforeProtocolGuid,
                  NULL,
                  (VOID **) &BeforeDisplay
                  );
  ASSERT (Status != EFI_SUCCESS);
  if (!EFI_ERROR (Status)) {
    return EFI_ALREADY_STARTED;
  }
  Status = InitBeforeDisplayData (QuietBoot, &BeforeDisplay);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  return BdsCpTrigger (&gH2OBdsCpDisplayBeforeProtocolGuid, BeforeDisplay);

}

/**
  Perform the platform diagnostic, such like test memory. OEM/IBV also
  can customize this function to support specific platform diagnostic.

  @param MemoryTestLevel  The memory test intensive level
  @param QuietBoot        Indicate if need to enable the quiet boot
  @param BaseMemoryTest   A pointer to BdsMemoryTest()

**/
VOID
PlatformBdsDiagnostics (
  IN EXTENDMEM_COVERAGE_LEVEL    MemoryTestLevel,
  IN BOOLEAN                     QuietBoot,
  IN BASEM_MEMORY_TEST           BaseMemoryTest
  )
{
  EFI_STATUS                        Status;
  UINT8                             PrintLine = 0;
  EFI_OEM_BADGING_SUPPORT_PROTOCOL  *Badging;
  UINT16                            FunctionKey;
  BOOLEAN                           KeyPressed;
  BOOLEAN                           EnableQuietBootState;
  EFI_CONSOLE_CONTROL_PROTOCOL      *ConsoleControl;

  //
  // PostCode = 0x1D, Display logo or system information
  //
  POST_CODE (BDS_DISPLAY_LOGO_SYSTEM_INFO);

  OemSvcHookBeforeLogoDisplay ();

  BdsLibOnStartOfBdsDiagnostics ();

  
  //
  // OemServices
  //
  Status = OemSvcDisplayLogo (
             &QuietBoot
             );
  
  Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, (VOID **)&ConsoleControl);
  if (EFI_ERROR (Status)) {
    ConsoleControl = NULL;
  }

  //
  // Here we can decide if we need to show the diagnostics screen
  // Notes: this quiet boot code should be remove from the graphic lib
  //
  BdsLibGetQuietBootState (&EnableQuietBootState);
  TriggerCpDisplayBefore (EnableQuietBootState);
  if (EnableQuietBootState) {
    EnableQuietBoot (&gEfiUgaSplashProtocolGuid);
  }

  BdsLibSetHotKeyDelayTime ();

  //
  // Perform system diagnostic
  //
  if (!QuietBoot) {
    gST->ConOut->ClearScreen (gST->ConOut);
  }

  BdsLibGetHotKey (&FunctionKey, &KeyPressed);
  Status = gBS->LocateProtocol (&gEfiOEMBadgingSupportProtocolGuid, NULL, (VOID **)&Badging);
  if (!EFI_ERROR (Status)) {
    if (KeyPressed) {
      if (ConsoleControl != NULL) {
        ShowOemString (Badging, TRUE, (UINT8) FunctionKey);
        BdsLibShowOemStringInTextMode (TRUE, (UINT8) FunctionKey);
      } else {
        if (QuietBoot) {
          ShowOemString (Badging, TRUE, (UINT8) FunctionKey);
        } else {
          BdsLibShowOemStringInTextMode (TRUE, (UINT8) FunctionKey);
        }
      }
    }
  }

  //
  // Perform system diagnostic
  //
  if (!QuietBoot) {
    gST->ConOut->ClearScreen (gST->ConOut);
    if (KeyPressed) {
      Status = BdsLibShowOemStringInTextMode (TRUE, (UINT8)FunctionKey);
    } else {
      Status = BdsLibShowOemStringInTextMode (FALSE, 0);
    }

    gST->ConOut->SetCursorPosition (gST->ConOut, 0, 0);
    Status = ShowSystemInfo (&PrintLine);
  }

  Status = BaseMemoryTest (MemoryTestLevel, (UINTN) (PrintLine + 2));
  if (EFI_ERROR (Status)) {
    DisableQuietBoot ();
  }

  OemSvcHookAfterLogoDisplay ();

  return;
}


/**
  Perform the platform diagnostic, such like test memory. OEM/IBV also
  can customize this fuction to support specific platform diagnostic.

  @param  LogoDisplay      Indicate if need to enable logo display
  @param  HotKeyService    Indicate if need to enable hotkey service
  @param  QuietBoot        Indicate if need to enable the quiet boot
  @param  BaseMemoryTest   The memory test function pointer

**/
VOID
PlatformBdsDiagnosticsMini (
  IN BOOLEAN                     LogoDisplay,
  IN BOOLEAN                     HotKeyService,
  IN BOOLEAN                     QuietBoot,
  IN BASEM_MEMORY_TEST           BaseMemoryTest
  )
{
  EFI_STATUS                        Status;
  UINT8                             PrintLine = 0;
  BOOLEAN                           EnableQuietBootState;
  EFI_OEM_BADGING_SUPPORT_PROTOCOL  *Badging;
  UINT16                            FunctionKey;
  BOOLEAN                           KeyPressed;
  EFI_CONSOLE_CONTROL_PROTOCOL      *ConsoleControl;

  if (LogoDisplay) {
    //
    // PostCode = 0x1D, Display logo or system information
    //
    POST_CODE (BDS_DISPLAY_LOGO_SYSTEM_INFO);

    OemSvcHookBeforeLogoDisplay ();
  
    BdsLibOnStartOfBdsDiagnostics ();
  

    //
    // OemServices
    //
    Status = OemSvcDisplayLogo (    
               &LogoDisplay
               );

   //
   // Here we can decide if we need to show
   // the diagnostics screen
   // Notes: this quiet boot code should be remove
   // from the graphic lib
   //
    BdsLibGetQuietBootState (&EnableQuietBootState);
    TriggerCpDisplayBefore (EnableQuietBootState);   
    if (EnableQuietBootState) {
      EnableQuietBoot (&gEfiUgaSplashProtocolGuid);
    }
  }

  if (HotKeyService) {
    Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, (VOID **)&ConsoleControl);
    if (EFI_ERROR (Status)) {
      ConsoleControl = NULL;
    }

    BdsLibGetHotKey (&FunctionKey, &KeyPressed);
    Status = gBS->LocateProtocol (&gEfiOEMBadgingSupportProtocolGuid, NULL, (VOID **)&Badging);
    if (!EFI_ERROR (Status)){
      if (KeyPressed) {
        if (ConsoleControl != NULL) {
          ShowOemString(Badging, TRUE, (UINT8) FunctionKey);
          BdsLibShowOemStringInTextMode (TRUE, (UINT8) FunctionKey);
        } else {
          if (QuietBoot) {
            ShowOemString(Badging, TRUE, (UINT8) FunctionKey);
          } else {
            BdsLibShowOemStringInTextMode (TRUE, (UINT8) FunctionKey);
          }
        }
      }
    }
  }

  Status = BaseMemoryTest (IGNORE, (UINTN)(PrintLine + 2));
  if (EFI_ERROR (Status)) {
    DisableQuietBoot ();
  }

  OemSvcHookAfterLogoDisplay ();

  return;
}


/**
  If the bootable device is legacy OS, delete BootPrevious Variable.

  @param  Event               The event that triggered this notification function
  @param  ParentImageHandle   Pointer to the notification functions context

  @retval EFI_STATUS    Success to delete BootPrevious Variable.

**/
VOID
BdsLegacyBootEvent (
  EFI_EVENT           Event,
  VOID                *ParentImageHandle
  )
{
  UINTN               BootPreviousSize;
  UINT16              *BootPrevious;

  BootPrevious = BdsLibGetVariableAndSize (
                   L"BootPrevious",
                   &gEfiGenericVariableGuid,
                   &BootPreviousSize
                   );
  if (BootPrevious != NULL) {
    //
    // OS has been changed from UEFI to Legacy
    //
    gBS->FreePool (BootPrevious);

    BootPrevious = NULL;
    gRT->SetVariable (
           L"BootPrevious",
           &gEfiGenericVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           0,
           BootPrevious
           );
  }

  gBS->CloseEvent (Event);

  return;
}

#if TABLET_PF_ENABLE
CHAR16* 
GetPmicCondString (
  IN  UINT8  PmicCond
  )
{
  switch (PmicCond) {
  case PMIC_Equal:
    return L"= ";
  case PMIC_Greater_Than:
    return L"> ";
  case PMIC_Smaller_Than:
    return L"< ";
  case PMIC_Greater_Equal:
    return L">=";
  case PMIC_Smaller_Equal:
    return L"<=";
  case PMIC_Any:
    return L"Any";
  default:
    return L"";
  }
}

CHAR16* 
GetPmicCondBoardIDString(
  IN  PMIC_Condition_Item  PmicCond
  )
{
  CHAR16                  *StrTemp;
  UINTN          StrTempsize;
  StrTempsize        = 16;
  
  StrTemp = AllocateZeroPool (StrTempsize);
  if (NULL == StrTemp) 
  {
    return NULL;
  }
  
  if (PmicCond.Cond_Choice == PMIC_Any)
  {
    UnicodeSPrint (StrTemp, StrTempsize, L"Any");
  }else
  {
    switch (PmicCond.Cond_Number) 
    {
      case BOARD_ID_AV_SVP:  // Alpine Valley Board
        UnicodeSPrint (StrTemp, StrTempsize, L"SVP ");
        break;
      case BOARD_ID_BL_RVP:  // BayLake Board (RVP)
        UnicodeSPrint (StrTemp, StrTempsize, L"RVP ");
        break;
      case BOARD_ID_BL_FFRD:  // BayLake Board (FFRD)
        UnicodeSPrint (StrTemp, StrTempsize, L"FFRD");
        break;
      case BOARD_ID_BL_STHI:  // PPV- STHI Board
        UnicodeSPrint (StrTemp, StrTempsize, L"STHI");
        break;
      case BOARD_ID_BB_RVP:  // Bayley Bay Board
        UnicodeSPrint (StrTemp, StrTempsize, L"RVP ");
        break;  //it is same as BOARD_ID_BL_FFRD, may have problem.
      default:
        UnicodeSPrint (StrTemp, StrTempsize, L"%02x", PmicCond.Cond_Number);
        break;
    }
  }
  return StrTemp;
}

CHAR16* 
GetPmicCondFabIDString(
  IN  PMIC_Condition_Item  PmicCond
  )
{
  CHAR16                  *StrTemp;
  UINTN          StrTempsize;
  UINT8          BoardId;
  StrTempsize        = 16;
  
  BoardId = PmicGetBoardID();

  StrTemp = AllocateZeroPool (StrTempsize);
  if (NULL == StrTemp) 
  {
    return NULL;
  }
  if (PmicCond.Cond_Choice == PMIC_Any)
  {
    UnicodeSPrint (StrTemp, StrTempsize, L"Any");
  }else if(PmicCond.Cond_Choice == PMIC_Equal)
  {
    if (BoardId == BOARD_ID_BL_FFRD)
    {
      switch (PmicCond.Cond_Number) 
      {
        case PR0:
          UnicodeSPrint (StrTemp, StrTempsize, L"PR0");
          break;
        case PR05:
          UnicodeSPrint (StrTemp, StrTempsize, L"PR0.5");
          break;
        case PR1:
          UnicodeSPrint (StrTemp, StrTempsize, L"PR1");
          break;
        case PR11:
          UnicodeSPrint (StrTemp, StrTempsize, L"PR1.1");
          break;
        default:
          UnicodeSPrint (StrTemp, StrTempsize, L"FAB%01x", PmicCond.Cond_Number);
          break;
      }
    }else
    {
      switch (PmicCond.Cond_Number) 
      {
      default:
        UnicodeSPrint (StrTemp, StrTempsize, L"FAB%01x", PmicCond.Cond_Number);
        break;
      }
    }
  }else{
    if (BoardId == BOARD_ID_BL_FFRD)
    {
      switch (PmicCond.Cond_Number) 
      {
        case PR0:
          UnicodeSPrint (StrTemp, StrTempsize, L"%2sPR0", GetPmicCondString(PmicCond.Cond_Choice));
          break;
        case PR05:
          UnicodeSPrint (StrTemp, StrTempsize, L"%2sPR0.5", GetPmicCondString(PmicCond.Cond_Choice));
          break;
        case PR1:
          UnicodeSPrint (StrTemp, StrTempsize, L"%2sPR1", GetPmicCondString(PmicCond.Cond_Choice));
          break;
        case PR11:
          UnicodeSPrint (StrTemp, StrTempsize, L"%2sPR1.1", GetPmicCondString(PmicCond.Cond_Choice));
          break;
        default:
          UnicodeSPrint (StrTemp, StrTempsize, L"FAB%01x", PmicCond.Cond_Number);
          break;
      }
    }else
    {
      UnicodeSPrint (StrTemp, StrTempsize, L"%2sFAB%01x", GetPmicCondString(PmicCond.Cond_Choice), PmicCond.Cond_Number);
    }
  }
  return StrTemp;
}

CHAR16* 
GetPmicCondSoc_SteppingString(
  IN  PMIC_Condition_Item  PmicCond
  )
{
  CHAR16                  *StrTemp;
  UINTN          StrTempsize;
  StrTempsize        = 18;

  StrTemp = AllocateZeroPool (StrTempsize);
  if (NULL == StrTemp) {
    return NULL;
  }

  if (PmicCond.Cond_Choice == PMIC_Any) {
    UnicodeSPrint (StrTemp, StrTempsize, L"Any");
  } else if(PmicCond.Cond_Choice == PMIC_Equal) {
    switch (PmicCond.Cond_Number) {
      case SocA0:
        UnicodeSPrint (StrTemp, StrTempsize, L"A0");
        break;
      case SocA1:
        UnicodeSPrint (StrTemp, StrTempsize, L"A1");
        break;
      case SocA2:
        UnicodeSPrint (StrTemp, StrTempsize, L"A2");
        break;
      case SocA3:
        UnicodeSPrint (StrTemp, StrTempsize, L"A3");
        break;
      case SocA4:
        UnicodeSPrint (StrTemp, StrTempsize, L"A4");
        break;
      case SocA5:
        UnicodeSPrint (StrTemp, StrTempsize, L"A5");
        break;
      case SocA6:
        UnicodeSPrint (StrTemp, StrTempsize, L"A6");
        break;
      case SocA7:
        UnicodeSPrint (StrTemp, StrTempsize, L"A7");
        break;
      case SocB0:
        UnicodeSPrint (StrTemp, StrTempsize, L"B0");
        break;
      case SocB1:
        UnicodeSPrint (StrTemp, StrTempsize, L"B1");
        break;
      case SocB2:
        UnicodeSPrint (StrTemp, StrTempsize, L"B2");
        break;
      case SocB3:
        UnicodeSPrint (StrTemp, StrTempsize, L"B3");
        break;
      case SocB4:
        UnicodeSPrint (StrTemp, StrTempsize, L"B4");
        break;
      case SocB5:
        UnicodeSPrint (StrTemp, StrTempsize, L"B5");
        break;
      case SocB6:
        UnicodeSPrint (StrTemp, StrTempsize, L"B6");
        break;
      case SocB7:
        UnicodeSPrint (StrTemp, StrTempsize, L"B7");
        break;
 
      default:
        UnicodeSPrint (StrTemp, StrTempsize, L"%02x", PmicCond.Cond_Number);
        break;
    }
  } else {
    switch (PmicCond.Cond_Number) {
    case SocA0:
      UnicodeSPrint (StrTemp, StrTempsize, L"A0");
      break;
    case SocA1:
      UnicodeSPrint (StrTemp, StrTempsize, L"A1");
      break;
    case SocA2:
      UnicodeSPrint (StrTemp, StrTempsize, L"A2");
      break;
    case SocA3:
      UnicodeSPrint (StrTemp, StrTempsize, L"A3");
      break;
    case SocA4:
      UnicodeSPrint (StrTemp, StrTempsize, L"A4");
      break;
    case SocA5:
      UnicodeSPrint (StrTemp, StrTempsize, L"A5");
      break;
    case SocA6:
      UnicodeSPrint (StrTemp, StrTempsize, L"A6");
      break;
    case SocA7:
      UnicodeSPrint (StrTemp, StrTempsize, L"A7");
      break;
    case SocB0:
      UnicodeSPrint (StrTemp, StrTempsize, L"B0");
      break;
    case SocB1:
      UnicodeSPrint (StrTemp, StrTempsize, L"B1");
      break;
    case SocB2:
      UnicodeSPrint (StrTemp, StrTempsize, L"B2");
      break;
    case SocB3:
      UnicodeSPrint (StrTemp, StrTempsize, L"B3");
      break;
    case SocB4:
      UnicodeSPrint (StrTemp, StrTempsize, L"B4");
      break;
    case SocB5:
      UnicodeSPrint (StrTemp, StrTempsize, L"B5");
      break;
    case SocB6:
      UnicodeSPrint (StrTemp, StrTempsize, L"B6");
      break;
    case SocB7:
      UnicodeSPrint (StrTemp, StrTempsize, L"B7");
      break;
    
      default:
        UnicodeSPrint (StrTemp, StrTempsize, L"%2s%02x", GetPmicCondString(PmicCond.Cond_Choice), PmicCond.Cond_Number);
        break;
    }
  }
  return StrTemp;
}

CHAR16* 
GetPmicCondDIDString(
  IN   PMIC_Condition_Item    PmicCond
)
{
  CHAR16                    *StrTemp;
  UINTN               StrTempsize;
  
  StrTempsize = 26;

  StrTemp = AllocateZeroPool (StrTempsize);
  if (NULL == StrTemp) {
    return NULL;
  }
  
  if (PmicCond.Cond_Choice == PMIC_Any)
  {
    UnicodeSPrint (StrTemp, StrTempsize, L"Any");
  }else{
    switch (PmicCond.Cond_Number) 
    {
      case PMIC_DEV_ROHM:  // ROHM
        UnicodeSPrint (StrTemp, StrTempsize, L"ROHM");
        break;
      case PMIC_DEV_DIALOG:  // Dialog
        UnicodeSPrint (StrTemp, StrTempsize, L"DIALOG");
        break;
      case PMIC_DEV_DIALOG_1:  // Dialog
        UnicodeSPrint (StrTemp, StrTempsize, L"DIALOG");
        break;
      case PMIC_DEV_MOUNTVILLE:  // Mountville
        UnicodeSPrint (StrTemp, StrTempsize, L"Mountville");
        break;
      default:
        UnicodeSPrint (StrTemp, StrTempsize, L"%02x", PmicCond.Cond_Number);
        break;
    }
  }
  return StrTemp;
}

CHAR16* 
GetPmicCondRIDString(
  IN   PMIC_Condition_Item    PmicCond
  )
{
  CHAR16        *StrTemp = NULL;
  UINTN       StrTempsize;
  StrTempsize  = 20;
  
  StrTemp = AllocateZeroPool (StrTempsize);
  if (NULL == StrTemp) {
     return NULL;
  }
  
  if (PmicCond.Cond_Choice == PMIC_Any)
  {
     UnicodeSPrint (StrTemp, StrTempsize, L"Any");
  }else if(PmicCond.Cond_Choice == PMIC_Equal)
  {
     UnicodeSPrint (StrTemp, StrTempsize, L"%02x", PmicCond.Cond_Number);
  }else
  {
     UnicodeSPrint (StrTemp, StrTempsize, L"%2s%02x", GetPmicCondString(PmicCond.Cond_Choice), PmicCond.Cond_Number);
  }
  return StrTemp;
}

/*++

Routine Description:

  Prompt warning message and suggest golden HW configuration

Arguments:
  VOID

Returns:

  NONE

--*/
VOID 
ShowPmicCorrectInfo(
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT8               DevId, RevId, Index, Index1, Index2, BoardId;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Foreground;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL   Background;
  CHAR16                  *TmpStr_ll, *TmpStr_BoardID, *TmpStr_FabID, *TmpStr_Socstepping, *TmpStr_PmicDid, *TmpStr_Pmicrid;
  CHAR16                  StrTemp[100];
  CHAR16                  StrTemp1[6];
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput = NULL;
  UINT32                L_mBootHorizontalResolution    = 0;
  UINT32                L_mBootVerticalResolution      = 0;
  UINT32                Temp_HorizontalResolution    = 0;
  UINT32                Temp_VerticalResolution    = 0;
  SOC_STEPPING                      stepping;
  PMIC_Condition_Item            temp_cond;
  
  TmpStr_BoardID   = NULL;
  TmpStr_FabID  = NULL;
  TmpStr_Socstepping   = NULL;
  TmpStr_PmicDid   = NULL;
  TmpStr_Pmicrid  = NULL;
  
  
  Status = gBS->HandleProtocol (
                gST->ConsoleOutHandle,
                &gEfiGraphicsOutputProtocolGuid,
                (VOID**)&GraphicsOutput
                );
  if (EFI_ERROR (Status)) {
    GraphicsOutput = NULL;
  }
  if (GraphicsOutput != NULL) {
    //
    // Get current video resolution and text mode.
    //
    L_mBootHorizontalResolution = GraphicsOutput->Mode->Info->HorizontalResolution;
    L_mBootVerticalResolution   = GraphicsOutput->Mode->Info->VerticalResolution;
  }
  else
  {
    return ;//due to can't get GraphicsOutput, so don't show anything.
  }
  
  SetMem (&Foreground, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0xff);
  SetMem (&Background, sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL), 0x0);
  
  PmicGetDevID(&DevId, &RevId);
  BoardId = PmicGetBoardID();
  stepping =  SocStepping();
  temp_cond.Cond_Choice = PMIC_Equal;
  temp_cond.Cond_Number = stepping;
  
  if((DevId != PMIC_DEV_ROHM) && (DevId != PMIC_DEV_DIALOG) && (DevId != PMIC_DEV_DIALOG_1))
  {
    TmpStr_Socstepping = GetPmicCondSoc_SteppingString(temp_cond);
    UnicodeSPrint (StrTemp, sizeof (StrTemp), L"Warning: You have invalid configuration of SOC %2s, PMIC (vendor:%x, version:%x).", TmpStr_Socstepping, DevId, RevId);
  }
  else
  {
    CopyMem (StrTemp1, L"Error", StrSize (L"Error"));
    if(DevId == PMIC_DEV_ROHM)//ROHM
    {
      CopyMem (StrTemp1, L"ROHM", StrSize (L"ROHM"));
    }
    else if((DevId == PMIC_DEV_DIALOG) || (DevId == PMIC_DEV_DIALOG_1))//Dialog
    {
      CopyMem (StrTemp1, L"Dialog", StrSize (L"Dialog"));
    }
    else if(DevId == PMIC_DEV_MOUNTVILLE) //Mountville 
    {
      CopyMem (StrTemp1, L"Mountville", StrSize (L"Mountville"));
    }
    TmpStr_Socstepping = GetPmicCondSoc_SteppingString(temp_cond);
    UnicodeSPrint (StrTemp, sizeof (StrTemp), L"Warning: You have invalid configuration of SOC %2s, PMIC %s %02x.", TmpStr_Socstepping, StrTemp1, RevId);
  }
  
  Index2 = 0;
  for(Index = 0; Index < sizeof(Pmic_list_a)/sizeof(PMIC_Compliance_Item); Index++)//show correct PMIC list
  {
    if (BoardId == Pmic_list_a[Index].PMIC_BoardID.Cond_Number)
    {
      Index2++;
    }
  }
  
  Temp_HorizontalResolution = (UINT32)(L_mBootHorizontalResolution - StrLen (StrTemp) * EFI_GLYPH_WIDTH)/2;
  Temp_VerticalResolution  = (L_mBootVerticalResolution*(50 - 6 - Index2)/50)/2;// the 6 means there are six line which is not PMIC guilty data.
  
  PrintXY (
    Temp_HorizontalResolution,
    Temp_VerticalResolution,
    &Foreground,
    &Background,
    StrTemp
  );
  TmpStr_ll = L"The supported list is:";
  PrintXY (
    Temp_HorizontalResolution,
    (Temp_VerticalResolution + L_mBootVerticalResolution*2/50),
    &Foreground,
    &Background,
    TmpStr_ll
  );
  TmpStr_ll = L"BoardID      FabID        Soc_Stepping PMIC_DevID   PMIC_RevID   "; 
  PrintXY (
    Temp_HorizontalResolution,
    (Temp_VerticalResolution + L_mBootVerticalResolution*3/50),
    &Foreground,
    &Background,
    TmpStr_ll
  );
    
  Index1 = 0;
  for(Index = 0; Index < sizeof(Pmic_list_a)/sizeof(PMIC_Compliance_Item); Index++)//show correct PMIC list
  {
    if (BoardId == Pmic_list_a[Index].PMIC_BoardID.Cond_Number)
    {
      if (Pmic_list_a[Index].mode == PMIC_White_List)
      {
        TmpStr_BoardID       = GetPmicCondBoardIDString(Pmic_list_a[Index].PMIC_BoardID);
        TmpStr_FabID      = GetPmicCondFabIDString  (Pmic_list_a[Index].PMIC_FabID);
        TmpStr_Socstepping   = GetPmicCondSoc_SteppingString(Pmic_list_a[Index].Soc_Stepping);
        TmpStr_PmicDid       = GetPmicCondDIDString    (Pmic_list_a[Index].PMIC_VendID);
        TmpStr_Pmicrid      = GetPmicCondRIDString    (Pmic_list_a[Index].PMIC_RevID);
        
        UnicodeSPrint (StrTemp, sizeof (StrTemp), L"%-13s%-13s%-13s%-13s%-13s", TmpStr_BoardID, TmpStr_FabID, TmpStr_Socstepping, TmpStr_PmicDid, TmpStr_Pmicrid);
        PrintXY (
          Temp_HorizontalResolution,
          (Temp_VerticalResolution + L_mBootVerticalResolution*(4 + Index1)/50),
          &Foreground,
          &Background,
          StrTemp
        );
        FreePool (TmpStr_BoardID);
        FreePool (TmpStr_FabID);
        FreePool (TmpStr_Socstepping);
        FreePool (TmpStr_PmicDid);
        FreePool (TmpStr_Pmicrid);
        Index1++;
      }
    }
  }
  
  TmpStr_ll = L"Press any key or wait for 5 seconds to continue.";
  PrintXY (
    Temp_HorizontalResolution,
    (Temp_VerticalResolution + L_mBootVerticalResolution*(5 + Index1)/50),
    &Foreground,
    &Background,
    TmpStr_ll
  );
  return;
}

/*++

Routine Description:

  Check Pmic condition with prefined list

Arguments:

  Pmic_data - PMIC raw data

  PmicCond_Item   - The header of the boot option link list

Returns:

  True if condition match; False if umatched

--*/
BOOLEAN 
JudgePmicCond(
  IN   UINT8          Pmic_data,
  IN   PMIC_Condition_Item    PmicCond_Item
)
{
  switch (PmicCond_Item.Cond_Choice) {
    case PMIC_Equal:
      return (Pmic_data == PmicCond_Item.Cond_Number);
    case PMIC_Greater_Than:
      return (Pmic_data > PmicCond_Item.Cond_Number);
    case PMIC_Smaller_Than:
      return (Pmic_data < PmicCond_Item.Cond_Number);
    case PMIC_Greater_Equal:
      return (Pmic_data >= PmicCond_Item.Cond_Number);
    case PMIC_Smaller_Equal:
      return (Pmic_data <= PmicCond_Item.Cond_Number);
    case PMIC_Any:
      return TRUE;
    default:
      return FALSE;
  }
}

/*++

Routine Description:

  Check Pmic compliance if a warning message should be prompted.

Arguments:

  void

Returns:

  True if obsolete HW configuration is detected; False if checking is passed 

--*/

BOOLEAN
CheckPmicCompliance(
  VOID
  )
{
  UINT8               DevId, RevId, Index, BoardId, FabID;
  SOC_STEPPING                 stepping;
  BOOLEAN              NeedShowCorrectPMICInfo = FALSE;
  UINT8              Find_Record = 0;
  BOOLEAN              Judge_BoardID, Judge_FabID, Judge_Socstepping, Judge_PmicDid, Judge_PmicRID;

  PmicGetDevID(&DevId, &RevId);
  BoardId  = PmicGetBoardID();
  FabID    = PmicGetFABID();
  stepping = SocStepping();
  
  //Return FALSE to bypass check if board is not baylake RVP.
  if (BoardId != BOARD_ID_BL_RVP) {
    return FALSE;
  }
  
  if((DevId != PMIC_DEV_ROHM) && (DevId != PMIC_DEV_DIALOG) && (DevId != PMIC_DEV_DIALOG_1))
  {
    NeedShowCorrectPMICInfo = TRUE;
  }
  else
  {
    for(Index = 0; Index < sizeof(Pmic_list_a)/sizeof(PMIC_Compliance_Item); Index++)
    {
      Judge_BoardID     = JudgePmicCond(BoardId,  Pmic_list_a[Index].PMIC_BoardID);
      Judge_FabID       = JudgePmicCond(FabID,    Pmic_list_a[Index].PMIC_FabID);
      Judge_Socstepping = JudgePmicCond(stepping, Pmic_list_a[Index].Soc_Stepping);
      Judge_PmicDid     = JudgePmicCond(DevId,    Pmic_list_a[Index].PMIC_VendID);
      Judge_PmicRID     = JudgePmicCond(RevId,    Pmic_list_a[Index].PMIC_RevID);
  
      if (Judge_BoardID && Judge_FabID && Judge_Socstepping && Judge_PmicDid && Judge_PmicRID)
      {
        Find_Record = 1;
        if (Pmic_list_a[Index].mode == PMIC_White_List)
        {
          NeedShowCorrectPMICInfo = FALSE;
          break;
        }
        else if (Pmic_list_a[Index].mode == PMIC_Black_List)
        {
          NeedShowCorrectPMICInfo = TRUE;
          break;
        }
        else
          break;
      }
    }
    if (Find_Record ==  0)// Here , we consider the table is white list mode.
    {
      NeedShowCorrectPMICInfo = TRUE;
    }
  }
  return NeedShowCorrectPMICInfo;
  
}
VOID
CheckButtonArray (
  BOOLEAN                            *IsHotkeyPress
  )
{
  EFI_HANDLE                         *HandleBuffer;
  UINTN                              NumberOfHandles;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *SimpleTextInEx = NULL;
  EFI_DEVICE_PATH_PROTOCOL           *DevicePath = NULL;
  EFI_STATUS                         Status;
  EFI_KEY_DATA                       Key[2];
  EFI_STATUS                         KeyStatus;
  UINTN                              Index, KeyIndex;

  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                   ByProtocol,
                   &gEfiSimpleTextInputExProtocolGuid,
                   NULL,
                   &NumberOfHandles,
                   &HandleBuffer
                   );
  if (EFI_ERROR (Status)) {
    *IsHotkeyPress = FALSE;
    return;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol (
                     HandleBuffer[Index],
                     &gEfiSimpleTextInputExProtocolGuid,
                     (VOID **)&SimpleTextInEx
                     );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (
                     HandleBuffer[Index],
                     &gEfiDevicePathProtocolGuid,
                     (VOID **)&DevicePath
                     );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if ((DevicePath->Type != HARDWARE_DEVICE_PATH) && (DevicePath->SubType != HW_VENDOR_DP)) {
      continue;
    }

    if (SimpleTextInEx != NULL) {
      KeyStatus = SimpleTextInEx->ReadKeyStrokeEx (SimpleTextInEx, &Key[0]);
      if (!EFI_ERROR (KeyStatus)) {
        //
        // No button array pressed.
        //
        if ( Key[0].Key.ScanCode == SCAN_ESC ) {
          *IsHotkeyPress = TRUE;
          return;
        }
      }
      
      for (KeyIndex = 0; KeyIndex < 5; KeyIndex++) {
        KeyStatus = SimpleTextInEx->ReadKeyStrokeEx (SimpleTextInEx, &Key[1]);
        if (EFI_ERROR (KeyStatus)) {
          //
          // No button array pressed.
          //
          *IsHotkeyPress = FALSE;
          return;
        }
        if (Key[0].Key.ScanCode == Key[1].Key.ScanCode) {
          //
          // Screen out the same key.
          //
          continue;
        }

        if ( Key[0].Key.ScanCode == SCAN_ESC ) {
          *IsHotkeyPress = TRUE;
          return;
        }
      }
    }
  }

  *IsHotkeyPress = FALSE;
  return;
}
#endif

#if TABLET_PF_ENABLE  
VOID
BeforeReadyToBootCpHandler (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  );
#endif

/**

  The function sends Arb data to CSE.

**/
EFI_STATUS
SendArb2CSE (
  VOID
)
{
  GET_ARB_STATUS_ACK    GetARB;
  UINT32                ARBIndex = 0;
  EFI_STATUS            Status;
  UINT32                RetryCount = 4;
  BOOLEAN               CommitArbSvn = FALSE;

  Status = HeciGetArbStatus (&GetARB);
  DEBUG ((EFI_D_INFO, "GetARB Status = %r\n", Status));
  DEBUG ((EFI_D_INFO, "GetARB Data - DirtySvns:\n"));
  for (ARBIndex = 0; ARBIndex < 16; ARBIndex++) {
    DEBUG ((EFI_D_INFO, "%x  ", GetARB.DirtySvns[ARBIndex]));
    if ((CommitArbSvn == FALSE) && (GetARB.DirtySvns[ARBIndex] != 0)) {
      CommitArbSvn = TRUE;
    }
  }
  //
  // Commit ARB SVN only if DirtySVN is non-zero.
  //
  if (CommitArbSvn) {
    do {
      Status = HeciCommitArbSvnUpdates ((UINT8 *)GetARB.DirtySvns);
      if (EFI_NOT_READY == Status) {
        RetryCount--;
        MicroSecondDelay (STALL_5_MILLISECOND);
      } else {
        RetryCount = 0;
      }
      DEBUG ((EFI_D_INFO, "CommitArbSvnUpdate Status = %r Retry Count : %x \n", Status, RetryCount));
    } while (RetryCount != 0);
  }
  return Status;
}

/**
  The function will execute with as the platform policy, current policy
  is driven by boot mode. IBV/OEM can customize this code for their specific
  policy action.

  @param  DriverOptionList        The header of the driver option link list
  @param  BootOptionList          The header of the boot option link list
  @param  ProcessCapsules         A pointer to ProcessCapsules()
  @param  BaseMemoryTest          A pointer to BaseMemoryTest()

**/
VOID
EFIAPI
PlatformBdsPolicyBehavior (
  IN LIST_ENTRY                      *DriverOptionList,
  IN LIST_ENTRY                      *BootOptionList,
  IN PROCESS_CAPSULES                ProcessCapsules,
  IN BASEM_MEMORY_TEST               BaseMemoryTest
  )
{
  EFI_STATUS                         Status;
  UINT16                             Timeout;
  EFI_SETUP_UTILITY_PROTOCOL         *SetupUtility;
  CHIPSET_CONFIGURATION               *SystemConfiguration;
  EXTENDMEM_COVERAGE_LEVEL           MemoryTestLevel;
  HOT_KEY_CONTEXT                    *HotKeyContext;
  UINT16                             FunctionKey;
  EFI_EVENT                          ReadyToBootEvent;
  EFI_DEVICE_PATH_PROTOCOL           *PlatformConnectLastBoot;
  UINT16                             *BootPrevious;
  EFI_HANDLE                         Handle;
  EFI_STATUS                         LocateDevicePathStatus;
  BOOLEAN                            HotKeyPressed;
  EFI_EVENT                          LegacyBootEvent;
  UINTN                              Size;
  EFI_CONSOLE_CONTROL_PROTOCOL       *ConsoleControl;
  UINT64                             OsIndications;
  UINT64                             OsIndicationsSupported;
  EFI_BOOT_MODE                      BootMode;
  BOOLEAN                            DeferredImageExist;
  CHAR16                             CapsuleVarName[30];
  CHAR16                             *TempVarName;  
  UINTN                              Index;
#if TABLET_PF_ENABLE  
  BOOLEAN                            Pmic_NeedShowCorrectPMICInfo = FALSE;
  EFI_INPUT_KEY                      Key;
  UINT8                              FotaProcessData;
  UINT8                              bAFUCapsule;
  H2O_BDS_CP_HANDLE                  CpHandle;

#endif  
#if ENBDT_PF_ENABLE  
//  UINTN                              SataPciRegBase = 0;
//  UINT16                             SataModeSelect = 0;
//  VOID                               *RegistrationExitPmAuth = NULL;
//  EFI_EVENT                          Event;
//[-start-160808-IB07220123-remove]//
//   EFI_DEVICE_PATH_PROTOCOL           *WindowsUxDevicePath;
//   EFI_GUID                           AndroidCapsuleFota;
//[-end-160808-IB07220123-remove]//
#endif
  VOID                               *Registration;
//[-start-160308-IB10860193-add]//
  UINTN                              CustomVariableSize;
  UINT8                              SetSetupAtRuntime;
  UINT8                              SetSetupInSmm;
//[-end-160308-IB10860193-add]//
//[-start-160920-IB07400786-add]//
  UINT32                             SdCardCdDw0;
//[-end-160920-IB07400786-add]//
 
  ConsoleControl = NULL;
  SystemConfiguration = NULL;
  PlatformConnectLastBoot = NULL;

  //
  // No deferred images exist by default
  //
  DeferredImageExist = FALSE;

  //
  // Install ExitPmAuthProtocol notification (will be signaled by kernel BiosProtectDxe driver after ReadyToBoot event)
  // for Intel specific process
  //
  EfiCreateProtocolNotifyEvent (
    &gExitPmAuthProtocolGuid,
    TPL_CALLBACK,
    PlatformBdsExitPmAuthCallback,
    NULL,
    &Registration
    );  
  //
  // Get current Boot Mode
  //
  BootMode = GetBootModeHob ();

  if (FeaturePcdGet (PcdH2OSecureBootSupported) && IsAdministerSecureBootSupport ()) {
    Status = PlatformBdsConnectConsole (gPlatformConsole);
    PlatformBdsBootDisplayDevice (SystemConfiguration);
    PlatformBdsConnectSequence ();
    BdsLibEnumerateAllBootOption (TRUE, BootOptionList);
    DisableQuietBoot ();
    CallSecureBootMgr ();
    //
    // Only assert here since this is the right behavior, system will be reset after user finishes secure boot manger.
    //
    ASSERT (FALSE);
  }

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;

  //
  // Clear all the capsule variables CapsuleUpdateData, CapsuleUpdateData1, CapsuleUpdateData2... 
  // as early as possible which will avoid the next time boot after the capsule update
  // will still into the capsule loop
  //
  StrCpy (CapsuleVarName, EFI_CAPSULE_VARIABLE_NAME);
  TempVarName = CapsuleVarName + StrLen (CapsuleVarName);
  Index = 0;
  while (TRUE) {
    if (Index > 0) {
      UnicodeValueToString (TempVarName, 0, Index, 0);
    }
    Status = gRT->SetVariable (
                 CapsuleVarName,
                 &gEfiCapsuleVendorGuid,
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS,
                 0,
                 (VOID *)NULL
                 );
    if (EFI_ERROR (Status)) {
      //
      // There is no capsule variables, quit
      //
      break;
    }
    Index++;
  }

  Status = gBdsServices->GetOsIndications (gBdsServices, &OsIndications, &OsIndicationsSupported);
  if (Status != EFI_SUCCESS){
    OsIndicationsSupported = 0;
    OsIndications = 0;
  }

  HotKeyContext = NULL;
  if (!(OsIndicationsSupported & OsIndications & EFI_OS_INDICATIONS_BOOT_TO_FW_UI) &&
      !(BootMode == BOOT_ON_S4_RESUME && BdsLibGetBootType () != 2)) {
    HotKeyContext                        = AllocateZeroPool (sizeof (HOT_KEY_CONTEXT));
    HotKeyContext->EnableQuietBootPolicy = (BOOLEAN) SystemConfiguration->QuietBoot;
    HotKeyContext->CanShowString         = FALSE;

    //
    // OemServices
    //
    Status = OemSvcDisplayLogo (
               &HotKeyContext->EnableQuietBootPolicy
               );


    POST_CODE (BDS_INSTALL_HOTKEY);
    BdsLibInstallHotKeys (HotKeyContext);
  }

  FunctionKey = NO_OPERATION;
  if (BootMode == BOOT_ASSUMING_NO_CONFIGURATION_CHANGES ||
      BootMode == BOOT_ON_S4_RESUME) {
    BootPrevious = BdsLibGetVariableAndSize (
                     L"BootPrevious",
                     &gEfiGenericVariableGuid,
                     &Size
                     );
    if (BootPrevious == NULL || Size == 0) {
      //
      // Cannot find BootPrevious variable, boot with full configuration.
      //
      BootMode = BOOT_WITH_FULL_CONFIGURATION;
    }
  } else {
    //
    // Clear the BootPrevious variable
    //
    BootPrevious = NULL;
    gRT->SetVariable (
           L"BootPrevious",
           &gEfiGenericVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           0,
           BootPrevious
           );
  }

  //
  // Create a Legacy Boot Event to delete variable, if the bootable device is legacy OS.
  //
  Status = EfiCreateEventLegacyBootEx (
             TPL_CALLBACK,
             BdsLegacyBootEvent,
             NULL,
             &LegacyBootEvent
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Init the time out value
  //
  Size = sizeof (Timeout);
  Status = gRT->GetVariable (L"Timeout", &gEfiGlobalVariableGuid, NULL, &Size, &Timeout);

  //
  // Load the driver option as the driver option list
  //
  PlatformBdsGetDriverOption (DriverOptionList);

  //
  // Clear the capsule variable as early as possible
  // which will avoid the next time boot after the capsule update will still into the capsule loop
  //
  gRT->SetVariable (
         L"CapsuleUpdateData",
         &gEfiCapsuleVendorGuid,
         EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
         0,
         (VOID *) NULL
         );

  //
  // Set memory test level by platform requirement
  //
  MemoryTestLevel = (SystemConfiguration->QuickBoot == TRUE) ? IGNORE : EXTENSIVE;

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             ChipsetPrepareReadyToBootEvent,
             NULL,
             &ReadyToBootEvent
             );
  ASSERT_EFI_ERROR (Status);

#if TABLET_PF_ENABLE

  Status = BdsCpRegisterHandler (
             &gH2OBdsCpReadyToBootBeforeProtocolGuid,
             BeforeReadyToBootCpHandler,
             H2O_BDS_CP_MEDIUM,
             &CpHandle
             );

  ASSERT_EFI_ERROR (Status);

  //
  // Go the different platform policy with different boot mode
  // Notes: this part code can be change with the table policy
  //
  if (PcdGetBool (PcdCheckPmicComplianceSupport)) {
    Pmic_NeedShowCorrectPMICInfo = CheckPmicCompliance ();
    if(TRUE == Pmic_NeedShowCorrectPMICInfo) {
      BootMode = BOOT_WITH_FULL_CONFIGURATION;
    }
  }
  
  Status = FlashRead (
            &FotaProcessData,
            (UINT8 *)(UINTN)(PcdGet32 (PcdFotaProcessOffset)),
            sizeof (FotaProcessData)
            );
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
  }
  
  if (FotaProcessData != SPI_DEFAULT) {
   goto FULL_CONFIGURATION;
  }
#else
//  //
//  // Use eMMC to boot OS and turn on AHCI, when SATA HDD is diconnected, 
//  // SATA AHCI CTLR device will show yellow bang, implement this solution to solve it.
//  //
//  SataPciRegBase  = MmPciAddress (0, 0, 19, 0, 0);
//  SataModeSelect  = MmioRead16 (SataPciRegBase + R_SATA_MAP) & B_SATA_MAP_SMS_MASK;
//  Status          = EFI_SUCCESS;
//
//    Status = gBS->CreateEvent (
//                     EVT_NOTIFY_SIGNAL,
//                     TPL_CALLBACK,
//                     DisableAhciCtlr,
//                     &SataPciRegBase,
//                     &Event
//                     );
//    if (!EFI_ERROR (Status)) {
//      //
//      // Do not execute it too late, because "device disabled register" need
//      // to save data for S3, after "PmAuth", SmmLock will deny the saving right
//      //
//      Status = gBS->RegisterProtocolNotify (
//                      &gExitPmAuthProtocolGuid,
//                      Event,
//                      &RegistrationExitPmAuth
//                      );
//    }
#endif


//[-start-160308-IB10860193-add]//
//
// Clear flag 
//
  CustomVariableSize = sizeof (UINT8);
  Status = gRT->GetVariable (L"SetSetupAtRuntime", &gCustomVariableGuid, NULL, &CustomVariableSize, &SetSetupAtRuntime);
  if (!EFI_ERROR(Status)) {
    SetSetupAtRuntime = 0x0;
    Status = gRT->SetVariable (
           L"SetSetupAtRuntime",
           &gCustomVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           CustomVariableSize,
           &SetSetupAtRuntime
           );
  }  
  
  CustomVariableSize = sizeof (UINT8);
  Status = gRT->GetVariable (L"SetSetupInSmm", &gCustomVariableGuid, NULL, &CustomVariableSize, &SetSetupInSmm);
  if (!EFI_ERROR(Status)) {
    SetSetupInSmm = 0x0;
    Status = gRT->SetVariable (
           L"SetSetupInSmm",
           &gCustomVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           CustomVariableSize,
           &SetSetupInSmm
           );
  }
//[-end-160308-IB10860193-add]//
//[-start-160920-IB07400786-add]//
  //
  // Read SdCard CD pin (GPIO_177) Bit 1
  //
  SdCardCdDw0 = GpioRead (SOUTHWEST, 0x5D0);
  //
  // Feedback invert value of CD pin to GRR3
  //
  SideBandAndThenOr32 (0xD6, 0x608, ~(UINT32)(0x20), SdCardCdDw0 & 0x02 ? 0x00 : 0x20);
//[-end-160920-IB07400786-add]//
     
  // Go the different platform policy with different boot mode
  // Notes: this part code can be change with the table policy
  //
  switch (BootMode) {

  case BOOT_WITH_MINIMAL_CONFIGURATION:
    //
    // This is sample code for BOOT_WITH_MINIMAL_CONFIGURATION.
    //
    PlatformBdsConnectConsoleMini (gPlatformConsole);
    PlatformBdsDiagnosticsMini (TRUE, TRUE, SystemConfiguration->QuietBoot,BaseMemoryTest);
    //PlatformBdsConnectDevicePath (gPlatformConnectSata[0], TRUE, TRUE);  //To be filled.
    break;

  case BOOT_ASSUMING_NO_CONFIGURATION_CHANGES:
    //
    // Connect default console devices
    //
    Status = PlatformBdsConnectConsoleMini (gPlatformConsole);
    if (EFI_ERROR (Status)) {
      //
      // ConOut connect fail, force to boot with full configuration
      //
      goto FULL_CONFIGURATION;
    }

    PlatformBdsDiagnosticsMini (TRUE, TRUE, SystemConfiguration->QuietBoot,BaseMemoryTest);

    //
    // As console is ready, perform user identification again.
    //
    if (mCurrentUser == NULL) {
      PlatformBdsUserIdentify (&mCurrentUser, &DeferredImageExist);
      if (DeferredImageExist) {
        //
        // After user authentication, the deferred drivers was loaded again.
        // Here, need to ensure the deferred images are connected.
        //
        BdsLibConnectAllDefaultConsoles ();
        PlatformBdsConnectSequence ();
      }
    }

    //
    // Close boot script and install ready to lock
    //
    InstallExitPmAuth ();

    //
    // Update gPlatformConnectLastBoot via resotring saved device path
    // Its device path type should be other than BBS_DEVICE_PATH after transform
    //
    UpdateLastBootDevicePath (BootPrevious, &PlatformConnectLastBoot);

    //
    // Connect last boot device path
    //
    PlatformBdsConnectDevicePath (PlatformConnectLastBoot, TRUE, TRUE);
    LocateDevicePathStatus = gBS->LocateDevicePath (
                                    &gEfiSimpleFileSystemProtocolGuid,
                                    &PlatformConnectLastBoot,
                                    &Handle
                                    );
    if (!EFI_ERROR (LocateDevicePathStatus)) {
      BdsLibGetHotKey (&FunctionKey, &HotKeyPressed);
      if (!HotKeyPressed) {
        Status = PlatformBdsBootSelection (FunctionKey, 0);
        break;
      } else {
        //
        // Hot has been pressed, boot with full configuration
        //
        goto FULL_CONFIGURATION;
      }
    } else {
      goto FULL_CONFIGURATION;
    }

  case BOOT_ON_FLASH_UPDATE:
    //
    // Boot with the specific configuration
    //
    PlatformBdsConnectConsole (gPlatformConsole);
    if (HotKeyContext != NULL) {
      HotKeyContext->CanShowString = TRUE;
    }
    PlatformBdsDiagnostics (EXTENSIVE, FALSE, BaseMemoryTest);
    BdsLibConnectAll ();
    //
    // Perform user identification
    //
    if (mCurrentUser == NULL) {
      PlatformBdsUserIdentify (&mCurrentUser, &DeferredImageExist);
      if (DeferredImageExist) {
        //
        // After user authentication, the deferred drivers was loaded again.
        // Here, need to ensure the deferred images are connected.
        //
        BdsLibConnectAll ();
      }
    }
    
    //
    // Close boot script
    //
    InstallExitPmAuth ();

    Status = ShowAllDevice ();
    ProcessCapsules (BOOT_ON_FLASH_UPDATE);
    break;

  case BOOT_IN_RECOVERY_MODE:
    //
    // In recovery mode, just connect platform console and show up the front page
    //
    PlatformBdsConnectConsole (gPlatformConsole);
    if (HotKeyContext != NULL) {
      HotKeyContext->CanShowString = TRUE;
    }

    BdsLibConnectAll ();

    //
    // Perform user identification
    //
    if (mCurrentUser == NULL) {
      PlatformBdsUserIdentify (&mCurrentUser, &DeferredImageExist);
      if (DeferredImageExist) {
        //
        // After user authentication, the deferred drivers was loaded again.
        // Here, need to ensure the deferred images are connected.
        //
        BdsLibConnectAll ();
      }
    }
    
    //
    // Close boot script 
    //
//[-start-160922-IB06740510-remove]//
//    InstallExitPmAuth ();
//[-end-160922-IB06740510-remove]//

    //
    // Recovery don't support HotKey, goto Boot Manager
    //
    BdsLibEnumerateAllBootOption (TRUE, BootOptionList);
    PlatformBdsBootSelection(FunctionKey, Timeout);

    if (PcdGetBool(PcdUseFastCrisisRecovery)){
    //
    // PostCode = 0x35, Fast recovery start flash
    //
    POST_CODE (BDS_RECOVERY_START_FLASH);
      RecoveryPopUp (DEFAULT_FLASH_DEVICE_TYPE);
    }
    BdsLibStartSetupUtility (TRUE);    
    break;

FULL_CONFIGURATION:
  case BOOT_ON_S4_RESUME:
  case BOOT_WITH_FULL_CONFIGURATION:
  case BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS:
  case BOOT_WITH_DEFAULT_SETTINGS:
  default:
#if TABLET_PF_ENABLE  
    CheckButtonArray (&HotKeyPressed);
    if (HotKeyPressed) {
      FunctionKey = FRONT_PAGE_HOT_KEY;
      BdsLibSetKey (FunctionKey, HotKeyContext);
    }
#endif
    Status = PlatformBdsConnectConsole (gPlatformConsole);
    if (EFI_ERROR (Status)) {
      //
      // Here OEM/IBV can customize with defined action
      //
      PlatformBdsNoConsoleAction ();
    }
    if (HotKeyContext != NULL) {
      HotKeyContext->CanShowString = TRUE;
    }

    PlatformBdsBootDisplayDevice (SystemConfiguration);
    PlatformBdsDiagnostics (MemoryTestLevel, SystemConfiguration->QuietBoot, BaseMemoryTest);

    //
    // Perform some platform specific connect sequence
    //
    BdsLibConnectLegacyRoms ();

    if (BdsLibIsWin8FastBootActive ()) {
      BdsLibConnectTargetDev ();
    } else {
      PlatformBdsConnectSequence ();
    }
#if TABLET_PF_ENABLE  
    if (PcdGetBool (PcdCheckPmicComplianceSupport)) {
      if (TRUE == Pmic_NeedShowCorrectPMICInfo) {
        ShowPmicCorrectInfo ();
        
        // press any key or wait for 5s to continue
        Status = WaitForSingleEvent (gST->ConIn->WaitForKey, FIVE_SECOND);
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        gST->ConOut->ClearScreen (gST->ConOut);//clear screen
      }
    }
#endif    
    //
    // Perform user identification
    //
    if (mCurrentUser == NULL) {
      PlatformBdsUserIdentify (&mCurrentUser, &DeferredImageExist);
      if (DeferredImageExist) {
        //
        // After user authentication, the deferred drivers was loaded again.
        // Here, need to ensure the deferred drivers are connected.
        //
        Status = PlatformBdsConnectConsole (gPlatformConsole);
        if (EFI_ERROR (Status)) {
          PlatformBdsNoConsoleAction ();
        }
        PlatformBdsConnectSequence ();
      }
    }

#if TABLET_PF_ENABLE  
    BdsLibStopHotKeyEvent ();

    BdsLibInstallHotKeys (HotKeyContext);
#endif

    //
    // Close boot script
    //  
//[-start-160808-IB07220123-remove]//
//     InstallExitPmAuth ();
//[-end-160808-IB07220123-remove]//

#if ENBDT_PF_ENABLE   
    //
    // Close boot script
    //
//[-start-160808-IB07220123-remove]//
//     WindowsUxDevicePath = NULL;
//     Size = 0;
//     Status = gRT->GetVariable (
//                     L"WindowsUxCapsuleDevicePath",
//                     &gWindowsUxCapsuleGuid,
//                     NULL,
//                     &Size,
//                     WindowsUxDevicePath
//                     );
//     if (Status != EFI_BUFFER_TOO_SMALL) {
//       //
//       // Try to get Android Capsule Device Path
//       //
//       Size = 0;
//       Status = gRT->GetVariable (
//                       L"AndroidCapsuleFota",
//                       &gAndroidCapsuleGuid,
//                       NULL,
//                       &Size,
//                       &AndroidCapsuleFota
//                       );
//       if (Status != EFI_BUFFER_TOO_SMALL) {
//         InstallExitPmAuth ();
//       }
//     }
//[-end-160808-IB07220123-remove]//
#endif
    if (!(SystemConfiguration->QuietBoot)) {
      Status = ShowAllDevice();
    }
    //
    // Here we have enough time to do the enumeration of boot device
    //
    BdsLibEnumerateAllBootOption (FALSE, BootOptionList);

    //
    // If Resume From S4 BootMode is set become BOOT_ON_S4_RESUME
    //
    // move the check boot mode is whether BOOT_ON_S4_RESUME to PlatformBdsBootSelection ()
    // to make sure the memory mapping is the same between normal boot and S4
    //
    //FunctionKey = FRONT_PAGE_HOT_KEY;    
    PlatformBdsBootSelection (FunctionKey, Timeout);


//-    //========================================================================
//-    //
//-    // For Droid boot 
//-    //========================================================================
//-    if(SystemConfiguration->DroidBoot) {  
//-        DEBUG((EFI_D_ERROR, "Start Android boot\n"));
//-        InstallLegacyAcpi();
//-        BdsBootAndroidFromEmmc ();  // For Debug
//-      
//-        
//-        // We should never get here.
//-    //    //
//-    //    CpuDeadLoop();
//-    //===================================================================
//-    
//-    }
    break;
  }

  ExitBootServiceSetVgaMode (SystemConfiguration, BootMode);

  //
  // Commit TXE Security Version Number (SVN).
  //
  SendArb2CSE ();
#if TABLET_PF_ENABLE
  bAFUCapsule = 0;
  Size = sizeof (bAFUCapsule);
  Status = gRT->GetVariable (
                  L"CapsuleFromAFU",
                  &BiosCapsuleFromAfuGuid,
                  NULL,
                  &Size,
                  &bAFUCapsule
                  );
  if ((FeaturePcdGet (PcdFotaManualSupport) && FeaturePcdGet (PcdFotaFeatureSupport) && (bAFUCapsule != CAPSULE_FOUND)) ||
      (bAFUCapsule == CAPSULE_UPDATE) || 
      (bAFUCapsule == CAPSULE_TXE_CHECK) || 
      (FotaProcessData != SPI_DEFAULT)) {
    IFWIUpdateHack (FotaProcessData);
  }
#endif  
  return;
}

/**
  This function will connect console device base on the console
  device variable ConOut.

  @retval EFI_SUCCESS    At least one of the ConIn and ConOut device have
                         been connected success.
  @retval Other          Return the status of BdsLibConnectConsoleVariable ().

**/
EFI_STATUS
BdsConnectMiniConsoles (
  VOID
  )
{
  EFI_STATUS                Status;

#ifdef Q2LSERVICE_SUPPORT
  //
  // PostCode = 0x17, Video device initial
  //
  POST_CODE (BDS_CONNECT_CONSOLE_OUT);
  Status = BdsLibConnectConsoleVariable (L"ConOut");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // PostCode = 0x16, Keyboard Controller, Keyboard and Moust initial
  //
  POST_CODE (BDS_CONNECT_CONSOLE_IN);
  Status = BdsLibConnectConsoleVariable (L"ConIn");
  if (EFI_ERROR (Status)) {
    return Status;
  }
#else
  //
  // PostCode = 0x16, Keyboard Controller, Keyboard and Moust initial
  //
  POST_CODE (BDS_CONNECT_CONSOLE_IN);
  //
  // Because possibly the platform is legacy free, in such case,
  // ConIn devices (Serial Port and PS2 Keyboard ) does not exist,
  // so we need not check the status.
  //
  //
  // PostCode = 0x17, Video device initial
  //
  POST_CODE (BDS_CONNECT_CONSOLE_OUT);

  Status = BdsLibConnectConsoleVariable (L"ConOut");
  if (EFI_ERROR (Status)) {
    return Status;
  }
#endif

  //
  // Special treat the err out device, becaues the null
  // err out var is legal.
  //
  return EFI_SUCCESS;
}


/**
  Connect the predefined platform default console device. Always try to find
  and enable the vga device if have.

  @param  PlatformConsole   Predfined platform default console device array.

  @retval EFI_STATUS    Success connect at least one ConIn and ConOut
                        device, there must have one ConOut device is
                        active vga device.
  @retval Other         Return the status of BdsLibConnectAllDefaultConsoles ()

**/
EFI_STATUS
PlatformBdsConnectConsoleMini (
  IN  BDS_CONSOLE_CONNECT_ENTRY   *PlatformConsole
  )
{
  EFI_STATUS                         Status;
  UINTN                              Index;
  EFI_DEVICE_PATH_PROTOCOL           *VarConout;
  EFI_DEVICE_PATH_PROTOCOL           *VarConin;
  UINTN                              DevicePathSize;
  EFI_SETUP_UTILITY_PROTOCOL         *SetupUtility;
  CHIPSET_CONFIGURATION               *SystemConfiguration;
  EFI_DEVICE_PATH_PROTOCOL           *UsbConsoleOuputDevPath = NULL;

  Status = gBS->LocateProtocol (
                  &gEfiSetupUtilityProtocolGuid,
                  NULL,
                  (VOID **)&SetupUtility
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  SystemConfiguration = (CHIPSET_CONFIGURATION *)SetupUtility->SetupNvData;

  //
  // Connect RootBridge
  //
  Status = PlatformBdsConnectDevicePath (gPlatformRootBridges[0], FALSE, FALSE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  VarConout = BdsLibGetVariableAndSize (
                L"ConOut",
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );

  VarConin  = BdsLibGetVariableAndSize (
                L"ConIn",
                &gEfiGlobalVariableGuid,
                &DevicePathSize
                );

  if (VarConout == NULL || VarConin == NULL) {
    //
    // Have chance to connect the platform default console,
    // the platform default console is the minimue device group
    // the platform should support
    //
    for (Index = 0; PlatformConsole[Index].DevicePath != NULL; ++Index) {
      //
      // Update the console variable with the connect type
      //
      if ((PlatformConsole[Index].ConnectType & CONSOLE_IN) == CONSOLE_IN) {
        BdsLibUpdateConsoleVariable (L"ConIn", PlatformConsole[Index].DevicePath, NULL);
      }
      if ((PlatformConsole[Index].ConnectType & CONSOLE_OUT) == CONSOLE_OUT) {
        BdsLibUpdateConsoleVariable (L"ConOut", PlatformConsole[Index].DevicePath, NULL);
      }
      if ((PlatformConsole[Index].ConnectType & STD_ERROR) == STD_ERROR) {
        BdsLibUpdateConsoleVariable (L"ErrOut", PlatformConsole[Index].DevicePath, NULL);
      }
    }
  }

  BdsLibConnectI2cDevice ();
  if (!(SystemConfiguration->UsbHotKeySupport == 0 && BdsLibIsWin8FastBootActive ())) {
    BdsLibConnectUsbHID ();
    if (FoundUSBConsoleOutput (&UsbConsoleOuputDevPath)) {
      SetUsbConsoleOutToConOutVar (UsbConsoleOuputDevPath);
      FreePool (UsbConsoleOuputDevPath);
    }
  } else {
    BdsLibConnectUsbHIDNotifyRegister ();
  }
  //
  // Connect the all the default console with current cosole variable
  //
  Status = BdsConnectMiniConsoles ();

  return Status;
}


EFI_STATUS
PlatformBdsConnectDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL    *HandleDevicePath,
  IN  BOOLEAN                     ConnectChildHandle,
  IN  BOOLEAN                     DispatchPossibleChild
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                Handle;

  BdsLibConnectDevicePath (HandleDevicePath);

  Status = gBS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &HandleDevicePath,
                  &Handle
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  do {
    gBS->ConnectController (Handle, NULL, NULL, ConnectChildHandle);

    //
    // Check to see if it's possible to dispatch an more DXE drivers
    //
    if (DispatchPossibleChild) {
      Status = gDS->Dispatch ();
    }
  } while (DispatchPossibleChild && !EFI_ERROR (Status));

  return EFI_SUCCESS;
}


/**
  This function is remained for IBV/OEM to do some platform action,
  if there no console device can be connected.

  @return EFI_SUCCESS      Direct return success now.

**/
EFI_STATUS
PlatformBdsNoConsoleAction (
  VOID
  )
{
  //
  // PostCode = 0x1C, Console device initial fail
  //
  POST_CODE (BDS_NO_CONSOLE_ACTION);
  
  return EFI_SUCCESS;
}


EFI_STATUS
PlatformBdsBootDisplayDevice (
  IN CHIPSET_CONFIGURATION        *SetupNVRam
  )
{
  EFI_LEGACY_BIOS_PROTOCOL      *LegacyBios;
  EFI_IA32_REGISTER_SET         Regs;
  EFI_STATUS                    Status;
  BOOLEAN                       SkipOriginalCode;

  if (SetupNVRam == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SkipOriginalCode = FALSE;

  Status = BdsLibDisplayDeviceReplace (&SkipOriginalCode);
  if (SkipOriginalCode || !EFI_ERROR (Status)) {
   return Status;
  }

  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&LegacyBios);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = OemSvcSetBootDisplayDevice (SetupNVRam);
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcSetBootDisplayDevice, Status : %r\n", Status ) );
  if (Status != EFI_SUCCESS) {
    Status = EFI_SUCCESS;
    if (SetupNVRam->PEGFlag != 1) { // IGD
      //
      //  VBIOS INT 10h : 5F64h, 00h - Set Display Device
      //
      //  Calling Registers:
      //    AX  = 5F64h, Switch Display Device function
      //    BH  = 00h, Set Display Device sub-function
      //    BL  = Set Display Device Options:
      //         Bits 7 - 1    = Reserved
      //         Bit 0      = Force complete execution even if same display device
      //    CX = Display Combination to Set (1 = Enable display, 0 = Disable display):
      //         Bit 15 = Pipe B - LFP2
      //         Bit 14 = Pipe B - EFP2
      //         Bit 13 = Pipe B - EFP3
      //         Bit 12 = Pipe B - CRT2
      //         Bit 11 = Pipe B - LFP
      //         Bit 10 = Pipe B - EFP
      //         Bit 9  = Pipe B - TV
      //         Bit 8  = Pipe B - CRT
      //         Bit 7  = Pipe A - LFP2
      //         Bit 6  = Pipe A - EFP2
      //         Bit 5  = Pipe A - EFP3
      //         Bit 4  = Pipe A - CRT2
      //         Bit 3  = Pipe A - LFP
      //         Bit 2  = Pipe A - EFP
      //         Bit 1  = Pipe A - TV
      //         Bit 0  = Pipe A - CRT
      //  Return Registers:
      //    AX  = Return Status (function not supported if AL != 5Fh):
      //         = 005Fh, Function supported and successful
      //         = 015Fh, Function supported but failed
      //
      if (SetupNVRam->IGDBootType != 0) {      // Not VBT Default
        Regs.X.CX = (UINT16)SetupNVRam->IGDBootType;
        if (SetupNVRam->IGDBootType != 0 && (SetupNVRam->IGDBootType & SetupNVRam->IGDBootTypeSecondary) == 0) {
          Regs.X.CX |= (UINT16)SetupNVRam->IGDBootTypeSecondary << 8;
        }
        Regs.X.AX = 0x5F64;
        Regs.X.BX = 0x0001;                    // Force complete execution even if same display device
        Status = LegacyBios->Int86 (LegacyBios, 0x10, &Regs);
      }
    }
  }

  return Status;
}


/**
  Platform Bds Boot Selection

  @param  Selection     HotKey Selection

  @retval EFI_STATUS

**/
EFI_STATUS
PlatformBdsBootSelection (
  UINT16                                    Selection,
  UINT16                                    Timeout
  )
{
  BOOLEAN                            NoBootDevices;
  UINT8                              *VariablePtr;
  UINTN                              VariableSize;
  EFI_STATUS                         Status;
  EFI_BOOT_MODE                      BootMode;
  BOOLEAN                            KeyPressed;
  UINTN                              StringSetNum;
  EFI_OEM_BADGING_SUPPORT_PROTOCOL   *Badging;
  UINT64                             OsIndications;
  UINT64                             OsIndicationsSupported;
  EFI_CONSOLE_CONTROL_PROTOCOL       *ConsoleControl;
  EFI_SETUP_UTILITY_PROTOCOL         *SetupUtility;
  CHIPSET_CONFIGURATION              *SystemConfiguration;

  NoBootDevices = FALSE;
  VariablePtr = NULL;
  VariableSize = 0;

  //
  // It needs display dialog for user input Hdd password to unlock Hdd in S4 resume.
  //
  BdsLibGetBootMode (&BootMode);

  //
  // Disable Hot Key event and check Hot Key result
  //
  BdsLibStopHotKeyEvent ();

  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return EFI_SUCCESS;
  }

  //
  // If there is no Boot option, go into SetupUtility & keep in SetupUtility
  //
  Status  = gRT->GetVariable (L"BootOrder", &gEfiGlobalVariableGuid, NULL, &VariableSize, VariablePtr);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    NoBootDevices = TRUE;
  }

  BdsLibGetHotKey (&Selection, &KeyPressed);
  if (KeyPressed && Selection != FRONT_PAGE_HOT_KEY) {
    Status = gBS->LocateProtocol (&gEfiConsoleControlProtocolGuid, NULL, (VOID **)&ConsoleControl);
    if (EFI_ERROR (Status)) {
      ConsoleControl = NULL;
    }

    Status = gBS->LocateProtocol (
                    &gEfiSetupUtilityProtocolGuid,
                    NULL,
                    (VOID **)&SetupUtility
                    );
    if (EFI_ERROR (Status)) {
      return EFI_ABORTED;
    }
    SystemConfiguration = (CHIPSET_CONFIGURATION *) SetupUtility->SetupNvData;

    Status = gBS->LocateProtocol (&gEfiOEMBadgingSupportProtocolGuid, NULL, (VOID **)&Badging);
    if (!EFI_ERROR (Status)) {
      //
      // BOOT_OS string always in the last one of array of OemBadgingString.
      //
      Badging->GetStringCount (Badging, &StringSetNum);
      if (ConsoleControl != NULL) {
        ShowOemString(Badging, TRUE, (UINT8) StringSetNum);
        BdsLibShowOemStringInTextMode (TRUE, (UINT8) StringSetNum);
      } else {
        if (SystemConfiguration->QuietBoot) {
          ShowOemString(Badging, TRUE, (UINT8) StringSetNum);
        } else {
          BdsLibShowOemStringInTextMode (TRUE, (UINT8) StringSetNum);
        }
      }
    }
  }

  OnEndOfDisableQuietBoot ();
  gST->ConOut->EnableCursor (gST->ConOut, FALSE);
  //gST->ConOut->ClearScreen (gST->ConOut);

  //
  // Based on the key that was set, we can determine what to do
  //
  BdsLibOnEndOfBdsBootSelection ();

  Status = gBdsServices->GetOsIndications (gBdsServices, &OsIndications, &OsIndicationsSupported);
  if (!EFI_ERROR (Status)) {
    if (OsIndications & OsIndicationsSupported & EFI_OS_INDICATIONS_BOOT_TO_FW_UI) {
//[-start-190822-IB16530055-modify]//
      if (FeaturePcdGet(PcdFrontPageSupported)) {
        Selection = FRONT_PAGE_HOT_KEY;
      } else {
        Selection = SETUP_HOT_KEY;
      }
//[-end-190822-IB16530055-modify]//
    }
    OsIndications &= (~ (UINT64) EFI_OS_INDICATIONS_BOOT_TO_FW_UI);

    gRT->SetVariable (
           EFI_OS_INDICATIONS_VARIABLE_NAME,
           &gEfiGlobalVariableGuid,
           EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
           sizeof (OsIndications),
           &OsIndications
           );
  }

  Status = OemHotKeyCallback (
             Selection,
             Timeout,
             BootMode,
             NoBootDevices
             );

  return EFI_SUCCESS;
}


VOID
ChipsetPrepareReadyToBootEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_HANDLE                                Handle;
  EFI_STATUS                                Status;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gEfiDxeSmmReadyToLockProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
  ASSERT_EFI_ERROR (Status);

  Handle = NULL;
  gBS->InstallProtocolInterface (
         &Handle,
         &gConOutDevStartedProtocolGuid,
         EFI_NATIVE_INTERFACE,
         NULL
         );

  return;
}


EFI_STATUS
EFIAPI
SetVgaCommandRegReadyToBootEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS                            Status;
  PCI_DEV_DEF                           *VgaDevContext;
  UINTN                                 Index;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       *PciRootBridgeIo;
  UINT64                                Address;
  UINT8                                 Date8;

  Status  = EFI_SUCCESS;
  VgaDevContext = (PCI_DEV_DEF *)Context;
  //
  // Index 0 is primary display VGA.
  //
  Index = 1;
  Date8 = 0x00;
  while ((VgaDevContext[Index].Seg != 0xFF) |
         (VgaDevContext[Index].Bus != 0xFF) |
         (VgaDevContext[Index].Dev != 0xFF) |
         (VgaDevContext[Index].Fun != 0xFF)) {
    Status = gBS->LocateProtocol (&gEfiPciRootBridgeIoProtocolGuid, NULL, (VOID **)&PciRootBridgeIo);
    ASSERT_EFI_ERROR (Status);
    if ((VgaDevContext[0].Seg == VgaDevContext[Index].Seg) &
        (VgaDevContext[0].Bus == VgaDevContext[Index].Bus) &
        (VgaDevContext[0].Dev == VgaDevContext[Index].Dev) &
        (VgaDevContext[0].Fun == VgaDevContext[Index].Fun)) {
      Index++;
      continue;
    }
    Address = EFI_PCI_ADDRESS (
                VgaDevContext[Index].Bus,
                VgaDevContext[Index].Dev,
                VgaDevContext[Index].Fun,
                PCICMD
                );
    Status = PciRootBridgeIo->Pci.Read (
                                    PciRootBridgeIo,
                                    EfiPciWidthUint8,
                                    Address,
                                    1,
                                    &Date8
                                    );
    Date8 = Date8 & (~0x01);
    Status = PciRootBridgeIo->Pci.Write (
                                    PciRootBridgeIo,
                                    EfiPciWidthUint8,
                                    Address,
                                    1,
                                    &Date8
                                    );
    DEBUG ((EFI_D_ERROR, "Set Pci command Reg(0x04) of (%02x/%02x/%02x) as 0.\n", (VgaDevContext[Index].Bus), (VgaDevContext[Index].Dev), (VgaDevContext[Index].Dev)));
    Index++;
  }

  gBS->FreePool (VgaDevContext);
  gBS->CloseEvent (Event);

  return Status;
}


/**
  Create a ExitBootService event for setting VGA text mode

  @param  SetupVariable     A pointer to setup variable
  @param  BootMode          System boot mode

  @retval EFI_SUCCESS   Success test all the system memory and update
  @retval Other         Error return value from get memory function

**/
EFI_STATUS
ExitBootServiceSetVgaMode (
  IN CHIPSET_CONFIGURATION                  *SetupVariable,
  IN EFI_BOOT_MODE                          BootMode
  )
{
  EFI_STATUS                                Status;
  EFI_EVENT                                 ExitBootServicesEvent;
  EFI_LEGACY_BIOS_PROTOCOL                  *LegacyBios;
//[-start-160923-IB07400788-add]//
  BOOLEAN                                   BootNormalPriorityFlag;
//[-end-160923-IB07400788-add]//

//[-start-160923-IB07400788-add]//
  if (SetupVariable->BootNormalPriority == 2) {
    BootNormalPriorityFlag = (BOOLEAN)!!(ReadExtCmos8(R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature) & B_CMOS_EFI_LEGACY_BOOT_ORDER);
  } else {
    BootNormalPriorityFlag = (BOOLEAN)SetupVariable->BootNormalPriority;
  }
//[-end-160923-IB07400788-add]//

//[-start-160923-IB07400788-modify]//
//  if ((SetupVariable->BootNormalPriority == 0) && (BootMode == BOOT_ON_S4_RESUME)) {
  if ((!BootNormalPriorityFlag) && (BootMode == BOOT_ON_S4_RESUME)) {
//[-end-160923-IB07400788-modify]//
    Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&LegacyBios);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    Status = gBS->CreateEvent (
                    EVT_SIGNAL_EXIT_BOOT_SERVICES,
                    TPL_NOTIFY,
                    ExitBootServiceSetVgaModeCallback,
                    LegacyBios,
                    &ExitBootServicesEvent
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}


/**
  Callback function for exit boot service event

  @param  Event         The memory test intensive level.
  @param  Context       Event context

**/
VOID
ExitBootServiceSetVgaModeCallback (
  EFI_EVENT  Event,
  VOID       *Context
  )
{
  EFI_LEGACY_BIOS_PROTOCOL      *LegacyBios;
  EFI_IA32_REGISTER_SET         Regs;

  LegacyBios = (EFI_LEGACY_BIOS_PROTOCOL*) Context;
  //
  // Set the 80x25 Text VGA Mode
  //
  Regs.H.AH = 0x00;
  Regs.H.AL = 0x83;
  LegacyBios->Int86 (LegacyBios, 0x10, &Regs);

  Regs.H.AH = 0x11;
  Regs.H.AL = 0x14;
  Regs.H.BL = 0;
  LegacyBios->Int86 (LegacyBios, 0x10, &Regs);
}


/**
  Update gLastLegacyBootDevicePath if the last boot device is from legacy BBS table.
  Duplicate a new EFI device path if the last boot device is from native boot option.

  @param  BootPrevious
  @param  LastBootDevicePath    Point to device path of the last boot device for connection

  @retval EFI_SUCCESS

**/
EFI_STATUS
UpdateLastBootDevicePath (
  IN   UINT16                     *BootPrevious,
  OUT  EFI_DEVICE_PATH_PROTOCOL   **LastBootDevicePath
  )
{
  CHAR16                     Buffer[20];
  UINT8                      *Variable;
  UINTN                      VariableSize;
  UINT8                      *Ptr;
  EFI_DEVICE_PATH_PROTOCOL   *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL   *FullDevicePath;

  //
  // Check the last boot device exists.
  //
  if (BootPrevious == NULL) {
    return EFI_UNSUPPORTED;
  }

  FullDevicePath = NULL;
  //
  // Get the last boot device's information from boot####.
  //
  UnicodeSPrint (Buffer, sizeof (Buffer), L"Boot%04x", *BootPrevious);

  Variable = BdsLibGetVariableAndSize (
               Buffer,
               &gEfiGlobalVariableGuid,
               &VariableSize
               );

  //
  // If the device path is an EFI device path, duplicate device path directly.
  //
  Ptr = Variable;
  Ptr += sizeof (UINT32) + sizeof (UINT16);
  Ptr += StrSize ((CHAR16 *) Ptr);
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Ptr;

  if (DevicePath->Type == MEDIA_DEVICE_PATH && DevicePath->SubType == MEDIA_HARDDRIVE_DP) {
    Variable = BdsLibGetVariableAndSize (
                 L"BootPreviousData",
                 &gEfiGenericVariableGuid,
                 &VariableSize
                 );
    FullDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Variable;
  }

  if (FullDevicePath == NULL) {
    *LastBootDevicePath = DuplicateDevicePath (DevicePath);
  } else {
    *LastBootDevicePath = DuplicateDevicePath (FullDevicePath);
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
PlatformBdsExitPmAuthCallback (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  )
{
  EFI_STATUS                    Status;
  VOID                          *ExitPmAuth;
  EFI_EVENT                     EndOfDxeEvent;
  //
  // Add more check to locate protocol after got event, because
  // ECP will signal this event immediately once it is register
  // just in case it is already installed.
  //
  Status = gBS->LocateProtocol (
                  &gExitPmAuthProtocolGuid,
                  NULL,
                  (VOID **)&ExitPmAuth
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }
  //
  // Since PI1.2.1, we need signal EndOfDxe as ExitPmAuth
  //
  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  BdsEmptyCallbackFuntion,
                  NULL,
                  &gEfiEndOfDxeEventGroupGuid,
                  &EndOfDxeEvent
                  );
  ASSERT_EFI_ERROR (Status);
  gBS->SignalEvent (EndOfDxeEvent);

}

VOID
InstallExitPmAuth (
  VOID
  )
{
  EFI_STATUS                Status;
#ifdef NOCS_S3_SUPPORT  
  EFI_HANDLE                Handle;
#endif
  EFI_SMM_ACCESS2_PROTOCOL  *SmmAccess;
  EFI_ACPI_S3_SAVE_PROTOCOL *AcpiS3Save;

  //
  // Install DxeSmmReadyToLock protocol prior to the processing of boot options
  //
  Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **) &SmmAccess);
  if (!EFI_ERROR (Status)) {

    //
    // Prepare S3 information, this MUST be done before DxeSmmReadyToLock
    //
    Status = gBS->LocateProtocol (&gEfiAcpiS3SaveProtocolGuid, NULL, (VOID **)&AcpiS3Save);
    if (!EFI_ERROR (Status)) {
      AcpiS3Save->S3Save (AcpiS3Save, NULL);
    }

#ifdef NOCS_S3_SUPPORT
    Handle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gExitPmAuthProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    NULL
                    );
    ASSERT_EFI_ERROR (Status);
#endif  

  }  

  return ;
}


/**
  This function is to identify a user, and return whether deferred images exist.

  @param[out]  User               Point to user profile handle.
  @param[out]  DeferredImageExist On return, points to TRUE if the deferred image
                                  exist or FALSE if it did not exist.

**/
VOID
EFIAPI
PlatformBdsUserIdentify (
  OUT EFI_USER_PROFILE_HANDLE        *User,
  OUT BOOLEAN                        *DeferredImageExist
  )
{
  EFI_STATUS                         Status;
  EFI_DEFERRED_IMAGE_LOAD_PROTOCOL   *DeferredImage;
  UINTN                              HandleCount;
  EFI_HANDLE                         *HandleBuf;
  UINTN                              Index;
  UINTN                              DriverIndex;
  EFI_DEVICE_PATH_PROTOCOL           *ImageDevicePath;
  VOID                               *DriverImage;
  UINTN                              ImageSize; 
  BOOLEAN                            BootOption;

  //
  // Perform user identification 
  //
  do {
    Status = BdsLibUserIdentify (User);
  } while (EFI_ERROR (Status));

  //
  // After user authentication now, try to find whether deferred image exists
  //
  HandleCount = 0;
  HandleBuf   = NULL;
  *DeferredImageExist = FALSE;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDeferredImageLoadProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuf
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuf[Index],
                    &gEfiDeferredImageLoadProtocolGuid,
                    (VOID **) &DeferredImage
                    );
    if (!EFI_ERROR (Status)) {       
      //
      // Find whether deferred image exists in this instance.
      //
      DriverIndex = 0;
      Status = DeferredImage->GetImageInfo(
                                DeferredImage, 
                                DriverIndex, 
                                &ImageDevicePath, 
                                (VOID **) &DriverImage,
                                &ImageSize, 
                                &BootOption
                                );
      if (!EFI_ERROR (Status)) {
        //
        // The deferred image is found.
        //
        FreePool (HandleBuf); 
        *DeferredImageExist = TRUE;
        return ;
      }          
    }    
  }
  
  FreePool (HandleBuf); 
}

/**
  This function locks the block 

  @param Base            The base address flash region to be locked.

**/
VOID
BdsLockFv (
  IN EFI_PHYSICAL_ADDRESS  Base
  )
{
  EFI_FV_BLOCK_MAP_ENTRY      *BlockMap;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  UINT8                       Data;
  UINT32                      BlockLength;
  UINTN                       Index;

  BaseAddress = Base - 0x400000 + 2;
  FvHeader    = (EFI_FIRMWARE_VOLUME_HEADER *) ((UINTN) (Base));
  BlockMap    = &(FvHeader->BlockMap[0]);

  while ((BlockMap->NumBlocks != 0) && (BlockMap->Length != 0)) {
    BlockLength = BlockMap->Length;
    for (Index = 0; Index < BlockMap->NumBlocks; Index++) {
      Data = MmioOr8 ((UINTN) BaseAddress, 0x03);
      BaseAddress += BlockLength;
    }
    BlockMap++;
  }
}

/**
  This function locks platform flash that is not allowed to be updated during normal boot path.
  The flash layout is platform specific.
**/
VOID
EFIAPI
PlatformBdsLockNonUpdatableFlash (
  VOID
  )
{
//[-start-160824-IB07220130-remove]//
//   EFI_PHYSICAL_ADDRESS  Base;
// 
//   Base = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdFlashFvMainBase);
//   if (Base > 0) {
//     BdsLockFv (Base);
//   }
// 
//   Base = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdFlashFvRecoveryBase);
//   if (Base > 0) {
//     BdsLockFv (Base);
//   }
//[-end-160824-IB07220130-remove]//
}


/**
  Connect the predefined platform default authentication devices.

  This function connects the predefined device path for authentication device,
  and if the predefined device path has child device path, the child handle will 
  be connected too. But the child handle of the child will not be connected.

**/
VOID
EFIAPI
PlatformBdsConnectAuthDevice (
  VOID
  )
{
  EFI_STATUS                   Status;
  UINTN                        Index;
  UINTN                        HandleIndex; 
  UINTN                        HandleCount;
  EFI_HANDLE                   *HandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL     *ChildDevicePath;
  EFI_USER_MANAGER_PROTOCOL    *Manager;

  Status = gBS->LocateProtocol (
                  &gEfiUserManagerProtocolGuid,
                  NULL,
                  (VOID **) &Manager
                  );
  if (EFI_ERROR (Status)) {
    //
    // As user manager protocol is not installed, the authentication devices 
    // should not be connected.
    //
    return ;
  }
  
  Index = 0;
  while (gUserAuthenticationDevice[Index] != NULL) {
    //   
    // Connect the platform customized device paths
    //
    BdsLibConnectDevicePath (gUserAuthenticationDevice[Index]);
    Index++;
  }

  //
  // Find and connect the child device paths of the platform customized device paths
  //
  HandleBuffer = NULL;
  for (Index = 0; gUserAuthenticationDevice[Index] != NULL; Index++) {
    HandleCount = 0;
    Status = gBS->LocateHandleBuffer (
                    AllHandles,
                    NULL,
                    NULL,
                    &HandleCount,
                    &HandleBuffer
                    );
    ASSERT (!EFI_ERROR (Status));

    //
    // Find and connect the child device paths of gUserIdentificationDevice[Index]
    //
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      ChildDevicePath = NULL;
      Status = gBS->HandleProtocol (
                      HandleBuffer[HandleIndex],
                      &gEfiDevicePathProtocolGuid,
                      (VOID **) &ChildDevicePath
                      );
      if (EFI_ERROR (Status) || ChildDevicePath == NULL) {
        continue;
      }

      if (CompareMem (
            ChildDevicePath,
            gUserAuthenticationDevice[Index],
            (GetDevicePathSize (gUserAuthenticationDevice[Index]) - sizeof (EFI_DEVICE_PATH_PROTOCOL))
            ) != 0) {
        continue;
      }      
      gBS->ConnectController (HandleBuffer[HandleIndex], NULL, NULL, TRUE);
    }
  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }
}

//#if ENBDT_PF_ENABLE
//VOID
//DisableAhciCtlr (
//  IN EFI_EVENT                          Event,
//  IN VOID                               *Context
//  )
//{
//  UINT32                    PmcDisableAddress;
//  UINT8                     SataStorageAmount;
//  UINT32                    SataBase;
//  UINT16                    SataPortStatus;
//
//  
//  DEBUG ((EFI_D_INFO, "Disable AHCI event is signalled\n"));
//  SataStorageAmount = 0;
//  SataBase = *(UINT32*) Context;
//
//  //
//  // Braswell-M EDS chapter 16 ---- PCI IO Register Offset 92 (SATA Status)
//  //
//  SataPortStatus = MmioRead16 (SataBase + 0x92);
//
//  //
//  // Bit 8 EN: Port 0 Present
//  //
//  if ((SataPortStatus & 0x100) == 0x100) {
//    SataStorageAmount++;
//  }
//
//  //
//  // Bit 9 EN: Port 1 Present
//  //
//  if ((SataPortStatus & 0x200) == 0x200) {
//    SataStorageAmount++;
//  }
//  
//  //
//  // Disable SATA controller when it sets to AHCI mode without carrying any devices
//  // in order to prevent AHCI yellow bang under Win device manager.
//  //
//  if (SataStorageAmount == 0) {
//    PmcDisableAddress = (MmioRead32 ((PCI_EXPRESS_BASE_ADDRESS + (UINT32) (31 << 15)) + R_LPC_PMC_BASE) & B_LPC_PMC_BASE_BAR) + R_PCH_PMC_FUNC_DIS;
////  PmcDisableAddress = (MmioRead32 ((PCH_PCI_EXPRESS_BASE_ADDRESS + (UINT32) (31 << 15)) + R_LPC_PMC_BASE) & B_LPC_PMC_BASE_BAR) + R_PCH_PMC_FUNC_DIS;
//    MmioOr32 (PmcDisableAddress, B_PCH_PMC_FUNC_DIS_SATA);
//    S3BootScriptSaveMemWrite (
//      EfiBootScriptWidthUint32,
//      (UINTN) PmcDisableAddress,
//      1,
//      (VOID *) (UINTN) PmcDisableAddress
//      );
//  }
//}
//#endif

BOOLEAN
FoundUSBConsoleOutput (
  OUT EFI_DEVICE_PATH_PROTOCOL  **UsbConsoleOuputDevPath
  )
{
  EFI_STATUS                            Status;
  UINTN                                 NumberOfHandles;
  EFI_HANDLE                            *HandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL              *GopDevicePath;
  UINTN                                 Index;
  EFI_DEVICE_PATH_PROTOCOL              *DevPathNode;
  EFI_DEVICE_PATH_PROTOCOL              *NewDevPath;
  BOOLEAN                               FoundUsbConsole;

  //
  // init locals
  //
  NumberOfHandles = 0;
  HandleBuffer = NULL;
  GopDevicePath = NULL;
  Index = 0;
  DevPathNode = NULL;
  NewDevPath = NULL;
  FoundUsbConsole = FALSE;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                );
  if (EFI_ERROR (Status)) {
    return FoundUsbConsole;
  } 
  
  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                  &gEfiDevicePathProtocolGuid,
                  (VOID*)&GopDevicePath
                  );
    if (EFI_ERROR (Status)) {
      continue;
    }

    DevPathNode = GopDevicePath;
    while (!IsDevicePathEnd (DevPathNode)) {
      if (DevicePathType (DevPathNode) == DP_TYPE_MESSAGING &&
          DevicePathSubType (DevPathNode) == DP_SUBTYPE_USB
          ) {
        NewDevPath = AppendDevicePathInstance (*UsbConsoleOuputDevPath, GopDevicePath);
        if (*UsbConsoleOuputDevPath != NULL) {
          FreePool (*UsbConsoleOuputDevPath);
        }
        *UsbConsoleOuputDevPath = NewDevPath;
        FoundUsbConsole = TRUE;
        break;
      }
      DevPathNode = NextDevicePathNode (DevPathNode);
    }
  }
  FreePool (HandleBuffer);
  
  return FoundUsbConsole;
}

VOID
SetUsbConsoleOutToConOutVar (
  IN EFI_DEVICE_PATH_PROTOCOL  *UsbConsoleOuputDevPath
  ) 
{
  EFI_DEVICE_PATH_PROTOCOL  *VarConOut;
  UINTN                     DevPathSize;
  EFI_DEVICE_PATH_PROTOCOL  *NextDevPathInst;
  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  EFI_STATUS                Status;
  UINTN                     Size;

  //
  //init locals
  //
  VarConOut = NULL;
  DevPathSize = 0;
  NextDevPathInst = NULL;
  TempDevicePath = NULL;
  RemainingDevicePath = UsbConsoleOuputDevPath;
  Size = 0;

  VarConOut = BdsLibGetVariableAndSize (
                L"ConOut",
                &gEfiGlobalVariableGuid,
                &DevPathSize
                );
 
  do {
    NextDevPathInst = GetNextDevicePathInstance (&RemainingDevicePath, &Size);
    if (NextDevPathInst != NULL) {
      Status = BdsMatchDevicePaths(VarConOut, NextDevPathInst, NULL, FALSE);
      if (EFI_ERROR (Status)) {
        TempDevicePath = AppendDevicePathInstance (VarConOut, NextDevPathInst);
        if (VarConOut != NULL) {
          FreePool (VarConOut);
        }
        VarConOut = TempDevicePath;
      }
      FreePool (NextDevPathInst);
    }
  } while (RemainingDevicePath != NULL);

  gRT->SetVariable (
        L"ConOut",
        &gEfiGlobalVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
        GetDevicePathSize (VarConOut),
        VarConOut
        );  
}

EFI_STATUS
BdsMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL  * Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL  * Single,
  IN  EFI_DEVICE_PATH_PROTOCOL  **NewDevicePath OPTIONAL,
  IN  BOOLEAN                   Delete
  )
/*++

Routine Description:
  Function compares a device path data structure to that of all the nodes of a
  second device path instance.

Arguments:
  Multi        - A pointer to a multi-instance device path data structure.

  Single       - A pointer to a single-instance device path data structure.
  
  NewDevicePath - If Delete is TRUE, this parameter must not be null, and it
                  points to the remaining device path data structure. 
                  (remaining device path = Multi - Single.)
  
  Delete        - If TRUE, means removing Single from Multi.
                  If FALSE, the routine just check whether Single matches 
                  with any instance in Multi.

Returns:

  The function returns EFI_SUCCESS if the Single is contained within Multi.  
  Otherwise, EFI_NOT_FOUND is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath1;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath2;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  UINTN                     Size;

  //
  // The passed in DevicePath should not be NULL
  //
  if ((!Multi) || (!Single)) {
    return EFI_NOT_FOUND;
  }
  //
  // if performing Delete operation, the NewDevicePath must not be NULL.
  //
  TempDevicePath1 = NULL;

  DevicePath      = Multi;
  DevicePathInst  = GetNextDevicePathInstance (&DevicePath, &Size);

  //
  // search for the match of 'Single' in 'Multi'
  //
  while (DevicePathInst) {
    if (CompareMem (Single, DevicePathInst, Size) == 0) {
      if (!Delete) {
        gBS->FreePool (DevicePathInst);
        return EFI_SUCCESS;
      }
    } else {
      if (Delete) {
        TempDevicePath2 = AppendDevicePathInstance (
                            TempDevicePath1,
                            DevicePathInst
                            );
        gBS->FreePool (TempDevicePath1);
        TempDevicePath1 = TempDevicePath2;
      }
    }

    gBS->FreePool (DevicePathInst);
    DevicePathInst = GetNextDevicePathInstance (&DevicePath, &Size);
  }

  if (Delete) {
    *NewDevicePath = TempDevicePath1;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

