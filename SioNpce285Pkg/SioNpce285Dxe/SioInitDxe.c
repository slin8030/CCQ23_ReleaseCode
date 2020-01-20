/** @file

Lock unlock sio and structure of install device protocol

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

#include "SioInitDxe.h"
//[-start-170502-IB07400864-add]//
#include <Library/DebugLib.h>
//[-end-170502-IB07400864-add]//

//
// this SIO support Cfg Port List
//
UINT16  mSioCfgPortList[]   = {0x2E, 0x4E, 0x162E, 0x164E};
UINT16  mSioCfgPortListSize = sizeof (mSioCfgPortList) / sizeof (UINT16);
UINT8  EnterConfig = ENTER_CONFIG;
UINT8  ExitConfig  = EXIT_CONFIG;

//
// Provide mSioResourceFunction Protocol Interface
//
EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[] = {
  NULL_ID,
  SIO_ID1,
  SIO_ID2,
  NULL_ID,
  FALSE,
};

extern EFI_STATUS InstallPs2KProtocol   (SIO_DEVICE_LIST_TABLE* Resource);
extern EFI_STATUS InstallPs2MProtocol   (SIO_DEVICE_LIST_TABLE* Resource);

//
// Provide DeviceProtocolFunction Table
//
EFI_INSTALL_DEVICE_FUNCTION mDeviceFunction[] = {
  {KYBD,    DEVICE_INSTANCE0, SIO_KEYBOARD,   InstallPs2KProtocol, SIO_DEV_ACTIVE, BIT0},
  {MOUSE,   DEVICE_INSTANCE1, SIO_MOUSE,      InstallPs2MProtocol, 0xFF,           0xFF}, 
  {NULL_ID, 0,                0,              NULL,                0,              0},
};

BOOLEAN
SafeToSetInactive (
  IN  SIO_DEVICE_LIST_TABLE *CurrentDeviceList,
  OUT UINT8                 *InstallDeviceIndex
  )
{
  UINT8                       CurrentDeviceIndex = 0;
  UINT8                       NextDeviceIndex = 0;
  SIO_DEVICE_LIST_TABLE       *CommonLdnDeviceList = NULL;
  EFI_INSTALL_DEVICE_FUNCTION *InstallDevicePtr = mDeviceFunction;
  BOOLEAN                     IsSafeToSetInactiveBit = FALSE;
  EFI_INSTALL_DEVICE_FUNCTION PossibleCommonActiveBitDevice;
  BOOLEAN                     AlreadyExcuted = FALSE;

  ZeroMem (&PossibleCommonActiveBitDevice, sizeof (EFI_INSTALL_DEVICE_FUNCTION));

  for (CurrentDeviceIndex = 0; CurrentDeviceIndex < (sizeof (mDeviceFunction) / sizeof (mDeviceFunction[0])); CurrentDeviceIndex++) {
    
    //
    // Check if current input device has active bit
    //
    if ((CurrentDeviceList->Device == InstallDevicePtr[CurrentDeviceIndex].Device) && (InstallDevicePtr[CurrentDeviceIndex].ActiveOffset != 0xFF) && !AlreadyExcuted) {
      *InstallDeviceIndex = CurrentDeviceIndex;
      IsSafeToSetInactiveBit = TRUE;
      PossibleCommonActiveBitDevice = InstallDevicePtr[CurrentDeviceIndex];
      AlreadyExcuted = TRUE;
    }

    CommonLdnDeviceList = mTablePtr;
    
    //
    // Search next device that have the same active bit
    //
    for (NextDeviceIndex = 0; NextDeviceIndex < (sizeof (mDeviceFunction) / sizeof (mDeviceFunction[0])); NextDeviceIndex++) {
      if ((*InstallDeviceIndex != NextDeviceIndex) &&
        (PossibleCommonActiveBitDevice.Ldn == InstallDevicePtr[NextDeviceIndex].Ldn) &&
        (PossibleCommonActiveBitDevice.ActiveOffset == InstallDevicePtr[NextDeviceIndex].ActiveOffset) && 
        (PossibleCommonActiveBitDevice.ActiveBit== InstallDevicePtr[NextDeviceIndex].ActiveBit)) {
        CommonLdnDeviceList = FindSioDevice (CommonLdnDeviceList, CommonLdnDeviceList->TypeInstance, InstallDevicePtr[NextDeviceIndex].Device, CommonLdnDeviceList->DeviceInstance);
        if (CommonLdnDeviceList != NULL && CommonLdnDeviceList->DeviceEnable) {
          IsSafeToSetInactiveBit = FALSE;
        }
      }
    }

    if (AlreadyExcuted && (NextDeviceIndex == (sizeof (mDeviceFunction) / sizeof (mDeviceFunction[0])))) {
      break;
    }
  }  

  return IsSafeToSetInactiveBit;
}

/**

 Enter SIO mode

**/
VOID
EnterConfigMode (
  )
{
}

/**

 Exit SIO mode

**/
VOID
ExitConfigMode (
  )
{
}

/**
  Transfer from SIO_DEVICE_LIST_TABLE to EFI_ISA_DEVICE_RESOURCE for using of device protocol

  @param[in]  *DeviceList            SIO_DEVICE_LIST_TABLE structure.
  @param[in]  *DeviceResource        EFI_ISA_DEVICE_RESOURCE structure.

  @retval EFI_SUCCESS                The function completed successfully.
**/
EFI_STATUS
DeviceListToDeviceResource (
  IN SIO_DEVICE_LIST_TABLE        *DeviceList,
  IN EFI_ISA_DEVICE_RESOURCE      *DeviceResource
  )
{
  DeviceResource->IoPort = DeviceList->DeviceBaseAdr;
  DeviceResource->IrqNumber = DeviceList->DeviceIrq;
  DeviceResource->DmaChannel = DeviceList->DeviceDma;

  return EFI_SUCCESS;
}

/**
  Install Device protocol from PCD structure.
**/
VOID
InstallEnabledDeviceProtocol (
  VOID
  )
{
  EFI_STATUS                         Status;
  UINT8                              Index;
  SIO_DEVICE_LIST_TABLE              *PcdPointer;
  UINT8                              InstallDeviceIndex = 0;

  Status       = EFI_NOT_FOUND;

  Index = 0;
  while (mDeviceFunction[Index].Device != NULL_ID) {
    PcdPointer = mTablePtr;
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      if ((PcdPointer->TypeInstance == mSioResourceFunction->Instance) &&
          (PcdPointer->Device == mDeviceFunction[Index].Device) && 
          (PcdPointer->DeviceInstance == mDeviceFunction[Index].Instance)) {  
        PcdPointer->DeviceLdn = mDeviceFunction[Index].Ldn;
        //
        // if the device is enable, then install it
        //
        if ((PcdPointer->DeviceEnable != FALSE) || (PcdPointer->DeviceDma == NULL_ID)) {
          if (mDeviceFunction[Index].InstallDeviceProtocol != NULL) {
            Status = mDeviceFunction[Index].InstallDeviceProtocol (
                       PcdPointer
                       );
          }
        }
        if ((PcdPointer->DeviceEnable == FALSE) && SafeToSetInactive (PcdPointer, &InstallDeviceIndex)) {
          IDW8 (SIO_LDN, mDeviceFunction[InstallDeviceIndex].Ldn, mSioResourceFunction);
          IDW8 (
            mDeviceFunction[InstallDeviceIndex].ActiveOffset,
            (IDR8 (mDeviceFunction[InstallDeviceIndex].ActiveOffset, mSioResourceFunction) & (~mDeviceFunction[InstallDeviceIndex].ActiveBit)),
            mSioResourceFunction
            );
        }
        break;
      }
      PcdPointer++;
    }
    Index++;
  }
}

/**
  Find SSDT table from RAW section of DXE driver, then publish it to ACPI table.

  @param[in]  Event           Event whose notification function is being invoked.
  @param[in]  Context         Pointer to the notification function's context.
**/
EFI_STATUS
UpdateSioMbox (
  IN EFI_ACPI_DESCRIPTION_HEADER  *SioAcpiTable
  )
{
  UINT8                        DeviceH;
  UINT8                        DeviceL;
  UINT8                        SioInstance;
  UINT8                        NumOfEntries;
  UINT8                        DeviceEntries;
  UINT8                        UpdateNum;
  UINT8                        *SsdtPointer;
  UINTN                        BufferSize;
  BOOLEAN                      UpdateConfig;
  EFI_STATUS                   Status;
  SIO_NVS_AREA                 *SioNvsArea;
  EFI_ASL_RESOURCE             *SioAslResource;
  SIO_CONFIGURATION            *SioConfiguration;
  SIO_DEVICE_LIST_TABLE        *PcdPointer;
  SIO_DEVICE_DDN_SETTING       *DdnFunction;
  EFI_ACPI_DESCRIPTION_HEADER  *Table;

  UpdateNum        = 0;
  DeviceEntries    = 0;
  DdnFunction      = NULL;
  UpdateConfig     = FALSE;
  PcdPointer       = mTablePtr;
  Status           = EFI_SUCCESS;
  Table            = SioAcpiTable;
  SioInstance      = mSioResourceFunction->Instance;
  DeviceL          = mSioResourceFunction->TypeIdLow;
  DeviceH          = mSioResourceFunction->TypeIdHigh;
  NumOfEntries     = sizeof (SIO_NVS_AREA) / sizeof (EFI_ASL_RESOURCE);
  BufferSize       = sizeof (SIO_CONFIGURATION);
  SioConfiguration = AllocateZeroPool (BufferSize);

  Status = gBS->AllocatePool(
                  EfiACPIMemoryNVS,
                  sizeof (SIO_NVS_AREA),
                  &SioNvsArea
                  );

  ASSERT_EFI_ERROR (Status);
  ZeroMem (SioNvsArea, sizeof (SIO_NVS_AREA));
  SioAslResource = (EFI_ASL_RESOURCE*) SioNvsArea;

  Status = gRT->GetVariable (
                  mSioVariableName,
                  &mSioFormSetGuid,
                  NULL,
                  &BufferSize,
                  SioConfiguration
                  );
  ASSERT_EFI_ERROR (Status);

  for (SsdtPointer = ((UINT8 *)((UINTN)Table)); SsdtPointer <= (UINT8 *)((UINTN)Table + (Table->Length)); SsdtPointer++) {
    switch ( *((UINT32*) SsdtPointer)) {

    //
    //  Search LGDN String
    //
    case (EFI_SIGNATURE_32 ('L', 'G', 'D', 'N')):
      //
      // Calculate the number of non-zero entries in the debug code table
      //
      while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
        //
        // Update PCD data to NVS area
        //
        if (DeviceEntries >= NumOfEntries) {
          break;
        }

        if ((PcdPointer->Device != CFG_ID) && (mSioResourceFunction->Instance == PcdPointer->TypeInstance)) {
          SioAslResource->Device = PcdPointer->DeviceEnable;
          SioAslResource->DeviceBaseIo = UpsideDown(PcdPointer->DeviceBaseAdr);
          SioAslResource->DeviceLdn = PcdPointer->DeviceLdn;
          //
          // Before setting KYBD/MOUSE, we need to check their flag(PcdPointer->Device),
          // because if there is no PS2 device, system will hang up during KbcExistence () and MouseExistence()
          //
          if (PcdPointer->Device == KYBD && PcdPointer->DeviceEnable == TRUE) {
            if (!KbcExistence ()) {
              SioAslResource->Device = 0;
            }
          }

          if (PcdPointer->Device == MOUSE && PcdPointer->DeviceEnable == TRUE) {
            if (!MouseExistence()) {
//[-start-170504-IB07400864-remove]//
//              SioAslResource->Device = 0;
//[-end-170504-IB07400864-remove]//
            }
          }
          SioAslResource++;
          DeviceEntries++;
        }
        PcdPointer++;
      }

      if (IsAmlOpRegionObject (SsdtPointer)) {
        ASSERT_EFI_ERROR (Status);
        SetOpRegion (SsdtPointer, SioNvsArea, sizeof (SIO_NVS_AREA));
      }
      break;

    //
    // Update config port
    //
    case (EFI_SIGNATURE_32 ('S','I','D','P')):
      if (!UpdateConfig) {
        *(UINT8*) (SsdtPointer + 5) = (UINT8) mSioResourceFunction->SioCfgPort;
        UpdateConfig = TRUE;
      }
      break;

    //
    // Update ASL name string to avoid the same device name in multi SIOs
    //
    case (EFI_SIGNATURE_32 ('M', 'o', 'd', 'u')):
      *(UINT16*) (SsdtPointer) = UintToChar (DeviceH);
      *((UINT16*) SsdtPointer + 1) = UintToChar (DeviceL);
      *((UINT16*) SsdtPointer + 2) = UintToChar (SioInstance);
      break;

    case (EFI_SIGNATURE_32 ('S', 'I', 'O', 'X')):
      *(SsdtPointer + 3) = (UINT8) (UintToChar (SioInstance) >> 8);
      break;

    default:
      break;
    }
  }
  return EFI_SUCCESS;
}

/**
  Create SCU by SIO and update value to PCD data.

  @retval      EFI_SUCCESS            Function complete successfully.
**/
EFI_STATUS
SioScu (
  VOID
  )
{
  UINT8                             Index;
  UINT8                             ConfigurationNum;
  EFI_STATUS                        Status;
  UINTN                             BufferSize;
  SIO_DEVICE_LIST_TABLE             *GetByConfiguration;
  SIO_CONFIGURATION                 *SioConfiguration;
  SIO_DEVICE_LIST_TABLE             *PcdPointer;

  PcdPointer = mTablePtr;
  ConfigurationNum = 0;
  SioConfiguration = NULL;

  //
  // Create variable and SIO page itself
  //
  Status = CreateSetupPage ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BufferSize = sizeof (SIO_CONFIGURATION);
  SioConfiguration = AllocateZeroPool (sizeof (SIO_CONFIGURATION));
  Status = gRT->GetVariable (
                  mSioVariableName,
                  &mSioFormSetGuid,
                  NULL,
                  &BufferSize,
                  SioConfiguration
                  );

  if (Status == EFI_SUCCESS) {
    ConfigurationNum = SioConfiguration->AutoUpdateNum;
    GetByConfiguration = (SIO_DEVICE_LIST_TABLE*) SioConfiguration;

    for (Index = 0; Index < ConfigurationNum; Index++) {
      //
      // Calculate the number of non-zero entries in the table
      //
      while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
        //
        // Check device and instance, if it's right, update to PCD data
        //
        if ((PcdPointer->Device == GetByConfiguration->Device) && (PcdPointer->DeviceInstance == GetByConfiguration->DeviceInstance)
          && (GetByConfiguration->DeviceEnable != SELECT_AUTO)) {
          PcdPointer->DeviceEnable = GetByConfiguration->DeviceEnable;
          PcdPointer->DeviceBaseAdr = GetByConfiguration->DeviceBaseAdr;
          PcdPointer->DeviceIrq = GetByConfiguration->DeviceIrq;
          //
          // if user disable device, it still need to install protocol, but doesn't create the table of the IsaAcpi
          //
          if (GetByConfiguration->DeviceEnable == SELECT_DISABLE) {
            PcdPointer->DeviceDma = NULL_ID;
          }
          PcdPointer++;
          break;
        }

        PcdPointer++;
      }
      GetByConfiguration++;
      PcdPointer = mTablePtr;
    }
  }
  FreePool (SioConfiguration);
  return Status;
}

