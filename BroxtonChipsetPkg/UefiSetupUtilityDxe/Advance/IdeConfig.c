/** @file

;******************************************************************************
;* Copyright (c) 2013-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <SetupUtility.h>
#include <PortNumberMap.h>

UINT16   gSaveItemMapping[0xf][2];
STATIC BOOLEAN  mIdeInit = FALSE;
BOOLEAN  mIsIDEModeFlag[2] = {FALSE};
EFI_STATUS
ForCombineMode (
  IN EFI_HII_HANDLE                        HiiHandle,
  IN CHIPSET_CONFIGURATION                  *SetupVariable
  );

EFI_STATUS
ForOtherMode (
  IN EFI_HII_HANDLE                        HiiHandle,
  IN CHIPSET_CONFIGURATION                  *SetupVariable
  );
VOID
IdeModelNameStrCpy (
  IN CHAR16   *Destination,
  IN CHAR16   *Source,
  IN UINTN    Length
  );

EFI_STATUS
SwapIdeConfig (
  IDE_CONFIG                              *IdeConfig1,
  IDE_CONFIG                              *IdeConfig2
  );

EFI_STATUS
UpdateTransferMode (
  IN  EFI_HII_HANDLE                        HiiHandle,
  IN  EFI_ATAPI_IDENTIFY_DATA               *IdentifyInfo,
  IN  STRING_REF                            TokenToUpdate,
  IN  CHAR8                                 *LanguageString
  );

/**
 Check this SATA port number is whther support

 @param [in]   PortNum          SATA port number

 @retval EFI_SUCCESS            platform supports this SATA port number
 @retval EFI_UNSUPPORTED        platform unsupports this SATA port number

**/
EFI_STATUS
CheckSataPort (
  IN UINTN                                  PortNum
)
{
  UINTN                 Index;
  PORT_NUMBER_MAP       *PortMappingTable;
  UINTN                 NoPorts;
  PORT_NUMBER_MAP                   EndEntry;

  PortMappingTable      = NULL;

  ZeroMem (&EndEntry, sizeof (PORT_NUMBER_MAP));

  PortMappingTable = (PORT_NUMBER_MAP *)PcdGetPtr (PcdPortNumberMapTable);

  NoPorts = 0;
  while (CompareMem (&EndEntry, &PortMappingTable[NoPorts], sizeof (PORT_NUMBER_MAP)) != 0) {
    NoPorts++;
  }
  if (NoPorts == 0) {
    return EFI_UNSUPPORTED;;
  }

  for (Index = 0; Index < NoPorts; Index++) {
    if (PortMappingTable[Index].PortNum == PortNum) {
      return EFI_SUCCESS;
    }
  }
  return EFI_UNSUPPORTED;
}


/**

 @param [in]   HiiHandle
 @param [in]   SetupVariable
 @param [in]   UpdateIde

 @retval NONE

**/
EFI_STATUS
CheckIde (
  IN EFI_HII_HANDLE                        HiiHandle,
  IN CHIPSET_CONFIGURATION                 *SetupVariable,
  IN BOOLEAN                               UpdateIde
  )
{
  EFI_STATUS                               Status;
  UINT16                                   HddCount;
  UINT16                                   BbsTotalCount;
  BBS_TABLE                                *LocalBbsTable;
  HDD_INFO                                 *LocalHddInfo;
  EFI_LEGACY_BIOS_PROTOCOL                 *LegacyBios;
  BOOLEAN                                  DeviceExist;
  CHAR16                                   *TokenString;
  CHAR16                                   *DeviceString;
  STRING_REF                               ModelNameToken;
  STRING_REF                               TransferModeToken;
  STRING_REF                               SecurityModeToken;
  UINT8                                    Channel;
  UINT8                                    Device;
  UINT8                                    *IdeDevice;
  UINTN                                    Index;
  UINTN                                    Index1;
  UINTN                                    Index2;
  UINTN                                    TotalLanguageCount;
  UINTN                                    LanguageCount;
  CHAR8                                    *LangString;
  CHAR16                                   *NotInstallString;
  CHAR8                                    *Lang;

  EFI_DISK_INFO_PROTOCOL                   *DiskInfo;
  ATA_IDENTIFY_DATA                        *IdentifyData;
  UINT32                                   Size;
  UINT8                                    SataCnfigure;
  VOID                                     *StartOpCodeHandle;
  EFI_IFR_GUID_LABEL                       *StartLabel;
  EFI_IFR_ACTION                           *ActionOpCode;

  SataCnfigure = gSUBrowser->SUCInfo->PrevSataCnfigure;
  DeviceExist       = FALSE;
  ModelNameToken    = 0;
  TransferModeToken = 0;
  SecurityModeToken = 0;
  Channel           = 0;
  Device            = 0;
  Index1            = 0;
  Index2            = 0;
  TokenString       = NULL;
  DeviceString      = NULL;
  HddCount          = 0;
  BbsTotalCount     = 0;
  LocalBbsTable     = NULL;
  LocalHddInfo      = NULL;
  LegacyBios        = NULL;
  IdentifyData      = NULL;

  if (SetupVariable->BootType != EFI_BOOT_TYPE) {
    Status = gBS->LocateProtocol (
                    &gEfiLegacyBiosProtocolGuid,
                    NULL,
                    (VOID **)&LegacyBios
                    );
    if (EFI_ERROR (Status)) {
      return Status;
      }
    Status = LegacyBios->GetBbsInfo(
                           LegacyBios,
                           &HddCount,
                           &LocalHddInfo,
                           &BbsTotalCount,
                           &LocalBbsTable
                           );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  GetLangDatabase (&TotalLanguageCount, (UINT8 **)&LangString);
  for (LanguageCount = 0; LanguageCount < TotalLanguageCount; LanguageCount++) {
//[-start-151123-IB11270134-modify]//
    IdeDevice = (UINT8 *) &(mAdvConfig.IdeDevice0);
//[-end-151123-IB11270134-modify]//
    Lang = &LangString[LanguageCount * RFC_3066_ENTRY_SIZE];
    for (Index=0; Index <= 1; Index++) {

      //
      // Get Device Token
      //
      switch (Index + 1) {

      case 1:
        TokenString = GetTokenStringByLanguage (
                        HiiHandle,
                        STRING_TOKEN (STR_SERIAL_ATA_PORT0_MODEL_NAME),
                        Lang
                        );
        ASSERT (TokenString);
        if (TokenString == NULL) {
          continue;
        }
        ModelNameToken    = STRING_TOKEN (STR_SERIAL_ATA_PORT0_STRING);
        TransferModeToken = STRING_TOKEN (STR_SERIAL_ATA_PORT0_TRANSFER_MODE);
        SecurityModeToken = STRING_TOKEN (STR_SERIAL_ATA_PORT0_SECURITY_MODE);
        break;

      case 2:
        TokenString = GetTokenStringByLanguage (
                        HiiHandle,
                        STRING_TOKEN (STR_SERIAL_ATA_PORT1_MODEL_NAME),
                        Lang
                        );
        ASSERT (TokenString);
        if (TokenString == NULL) {
          continue;
        }
        ModelNameToken    = STRING_TOKEN (STR_SERIAL_ATA_PORT1_STRING);
        TransferModeToken = STRING_TOKEN (STR_SERIAL_ATA_PORT1_TRANSFER_MODE);
        SecurityModeToken = STRING_TOKEN (STR_SERIAL_ATA_PORT1_SECURITY_MODE);
        break;
      }
        mIdeConfig[Index].SecurityModeToken = SecurityModeToken;
        if (mIdeConfig[Index].IdeDevice == 1) {
          DeviceExist = TRUE;
          Channel = mIdeConfig[Index].Channel;
          Device  = mIdeConfig[Index].Device;
          DeviceString = CatSPrint(NULL, L"%s%s", TokenString, mIdeConfig[Index].DevNameString);
          if (DeviceString == NULL) {
            continue;
          }

          if (mIdeConfig[Index].DiskInfoHandle == NULL) {
            IdentifyData = (ATA_IDENTIFY_DATA *) &LocalHddInfo[Channel].IdentifyDrive[Device];
          } else {
            Status = gBS->HandleProtocol (
                            mIdeConfig[Index].DiskInfoHandle,
                            &gEfiDiskInfoProtocolGuid,
                            (VOID **)&DiskInfo
                            );
            if (EFI_ERROR (Status)) {
              continue;
            }

            Size   = sizeof(ATA_IDENTIFY_DATA);
            Status = gBS->AllocatePool (
                            EfiBootServicesData,
                            Size,
                            (VOID **)&IdentifyData
                            );

            Status = DiskInfo->Identify (
                                 DiskInfo,
                                 IdentifyData,
                                 &Size
                                 );
          }

          UpdateTransferMode (
            HiiHandle,
            (EFI_ATAPI_IDENTIFY_DATA *) IdentifyData,
            TransferModeToken,
            Lang
            );

          *IdeDevice = 1;
          if (mIdeConfig[Index].DiskInfoHandle != NULL) {
            gBS->FreePool (IdentifyData);
          }
        } else {
          Status = CheckSataPort (Index);
          if (EFI_ERROR (Status)) {
            *IdeDevice = 2;
          } else {
            NotInstallString = GetTokenStringByLanguage (
                                 HiiHandle,
                                 STRING_TOKEN (STR_NOT_INSTALLED_TEXT),
                                 Lang
                                 );
            ASSERT (NotInstallString);
            if (NotInstallString != NULL) {
              DeviceString = CatSPrint(NULL, L"%s[%s]", TokenString, NotInstallString);
              if (DeviceString == NULL) {
                continue;
              }
              gBS->FreePool (NotInstallString);
              NotInstallString = NULL;
            } else {
              DeviceString = CatSPrint(NULL, L"%s[]", TokenString);
            }
            gBS->FreePool (TokenString);
            *IdeDevice = 0;
          }
        }

        IdeDevice++;
        //
        // Update String
        //
        if (DeviceString != NULL) {
          Status = gSUBrowser->HiiString->SetString (
                                            gSUBrowser->HiiString,
                                            HiiHandle,
                                            ModelNameToken,
                                            Lang,
                                            DeviceString,
                                            NULL
                                            );
          gBS->FreePool (DeviceString);
          DeviceString = NULL;
        }
    }
  }
  if ((SetupVariable->SetUserPass == TRUE) &&
       (SetupVariable->UserAccessLevel == USER_PASSWORD_VIEW_ONLY)) {
    if (DeviceExist == 0) {
      //
      // When no any booting device ,the system will hang.
      // We have create dummy data to updatefrom on last line.
      //
      StartOpCodeHandle = HiiAllocateOpCodeHandle ();
      ASSERT (StartOpCodeHandle != NULL);

      StartLabel               = (EFI_IFR_GUID_LABEL *) HiiCreateGuidOpCode (StartOpCodeHandle, &gEfiIfrTianoGuid, NULL, sizeof (EFI_IFR_GUID_LABEL));
      StartLabel->ExtendOpCode = EFI_IFR_EXTEND_OP_LABEL;
      StartLabel->Number       = IDE_UPDATE_LABEL;

      ActionOpCode = (EFI_IFR_ACTION *) HiiCreateActionOpCode (
                                          StartOpCodeHandle,
                                          0,
                                          STRING_TOKEN(STR_BLANK_STRING),
                                          0,
                                          EFI_IFR_FLAG_CALLBACK,
                                          0
                                          );
  
      Status = HiiUpdateForm (
                HiiHandle,
                NULL,
                0x23,
                StartOpCodeHandle,
                NULL
                );
  
      HiiFreeOpCodeHandle (StartOpCodeHandle);
    }
  }

  gBS->FreePool (LangString);
  return EFI_SUCCESS;
}

EFI_STATUS
UpdateTransferMode (
  IN  EFI_HII_HANDLE                        HiiHandle,
  IN  EFI_ATAPI_IDENTIFY_DATA               *IdentifyInfo,
  IN  STRING_REF                            TokenToUpdate,
  IN  CHAR8                                 *LanguageString
  )
{
  UINTN                                 Mode, Index;
  STRING_REF                            Token;
  CHAR16                                *StringPtr;

  Token = 0;
  Index = 0;
  if (IdentifyInfo->field_validity & 4) {
    Mode = (UINTN)(IdentifyInfo->ultra_dma_select & 0x3F);

    do {
      Index++;
      Mode = Mode >> 1;
    } while (Mode != 0);
  }

  switch (Index) {

  case 0:
    //
    // FPIO
    //
    Token = STRING_TOKEN (STR_IDE_TRANSMODE_FAST_PIO);
    break;

  case 1:
  case 2:
  case 3:
    //
    // Ultra DMA 33
    //
    Token = STRING_TOKEN (STR_IDE_ULTRA_DMA_ATA_33);
    break;

  case 4:
  case 5:
    //
    // Ultra DMA 66
    //
    Token = STRING_TOKEN (STR_IDE_ULTRA_DMA_ATA_66);
    break;

  case 6:
    //
    // Ultra DMA 100
    //
    Token = STRING_TOKEN (STR_IDE_ULTRA_DMA_ATA_100);
    break;
  }

  if (Token != 0) {
    StringPtr = GetTokenStringByLanguage (
                  HiiHandle,
                  Token,
                  LanguageString
                  );
   ASSERT (StringPtr);
   if (!StringPtr) {
     return EFI_NOT_FOUND;
   }
    gSUBrowser->HiiString->SetString (
                             gSUBrowser->HiiString,
                             HiiHandle,
                             TokenToUpdate,
                             LanguageString,
                             StringPtr,
                             NULL
                             );
    gBS->FreePool (StringPtr);
  }

  return EFI_SUCCESS;
}

/**
 Copy the Unicode string Source to Destination.

 @param [in]   Destination      Location to copy string
 @param [in]   Source           String to copy
 @param [in]   Length

 @retval NONE

**/
VOID
IdeModelNameStrCpy (
  IN CHAR16   *Destination,
  IN CHAR16   *Source,
  IN UINTN    Length
  )
{
  *(Destination++) = L'[';
  while (Length) {
    *(Destination++) = *(Source++);
    Length--;
  }
  *(Destination++) = L']';
  *Destination = 0;
}

/**
 To update the list of ide device and status.

 @param [in]   HiiHandle        the handle of Advance menu.
 @param [in]   Buffer           the SYSTEM CONFIGURATION of SetupBrowser.

 @retval EFI_SUCCESS            it is success to update the status of Ide device.

**/
EFI_STATUS
UpdateHDCConfigure (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN CHIPSET_CONFIGURATION              *Buffer
  )
{
  UINT8                                   PrevSataCnfigure;

  //
  // Transfer previous mode to Combine mode
  //
  PrevSataCnfigure = gSUBrowser->SUCInfo->PrevSataCnfigure;


  InitIdeConfig (mIdeConfig);

  //
  // transfer modoe form Combine mode to selected mode
  //
  CheckIde (
    HiiHandle,
    Buffer,
    TRUE
    );
  gSUBrowser->SUCInfo->DoRefresh = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
GetIdeDevNameString (
  IN OUT CHAR16 *                        *DevNameString
)
{
  EFI_STATUS                            Status;
  BBS_TABLE                             *LocalBbsTable;
  HDD_INFO                              *LocalHddInfo;
  EFI_LEGACY_BIOS_PROTOCOL              *LegacyBios;
  EFI_ATAPI_IDENTIFY_DATA               *IdentifyDriveInfo;
  UINT16                                HddCount;
  UINT16                                BbsTotalCount;
  UINTN                                 BbsTableIndex;

  UINT8                                 Channel;
  UINT8                                 Device;
  UINT8                                 Index;
  CHAR16                                *ModelNametring;

  ModelNametring = NULL;
  Status = gBS->LocateProtocol (
                  &gEfiLegacyBiosProtocolGuid,
                  NULL,
                  (VOID **)&LegacyBios
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = LegacyBios->GetBbsInfo(
                         LegacyBios,
                         &HddCount,
                         &LocalHddInfo,
                         &BbsTotalCount,
                         &LocalBbsTable
                         );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // IDE Configuration
  //
  Index = 0;
  BbsTableIndex = 1;
  for (Channel=0; Channel<4; Channel++) {
    for (Device=0; Device<2; Device++) {
    //
    // Initial String
    //
      DevNameString[Index] = AllocateZeroPool (0x100);
      if (LocalBbsTable[BbsTableIndex].BootPriority != BBS_IGNORE_ENTRY) {
        ModelNametring = AllocateZeroPool (0x100);

        ASSERT (ModelNametring != NULL);
        if (ModelNametring == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }

        //
        // Get Device Model name
        //
        IdentifyDriveInfo = (EFI_ATAPI_IDENTIFY_DATA *)&LocalHddInfo[Channel].IdentifyDrive[Device];
        UpdateAtaString(
          IdentifyDriveInfo,
          &ModelNametring
          );
        IdeModelNameStrCpy(
          DevNameString[Index],
          ModelNametring,
          20
          );
        gBS->FreePool (ModelNametring);
      }
      Index++;
      BbsTableIndex++;
    }
  }
  return EFI_SUCCESS;
}


/**
 According Bus, Device, Function, PrimarySecondary, SlaveMaster to get corresponding
 SATA port number

 @param [in]   Bus              PCI bus number
 @param [in]   Device           PCI device number
 @param [in]   Function         PCI function number
 @param [in]   PrimarySecondary  primary or scondary
 @param [in]   SlaveMaster      slave or master
 @param [in, out] PortNum       An address to save SATA port number.

 @retval EFI_SUCCESS            get corresponding port number successfully
 @retval EFI_INVALID_PARAMETER  input parameter is invalid
 @retval EFI_NOT_FOUND          can't get corresponding port number

**/
EFI_STATUS
SearchMatchedPortNum (
  IN     UINT32                              Bus,
  IN     UINT32                              Device,
  IN     UINT32                              Function,
  IN     UINT8                               PrimarySecondary,
  IN     UINT8                               SlaveMaster,
  IN OUT UINTN                               *PortNum
  )
{
  UINTN                 Index;
  PORT_NUMBER_MAP       *PortMappingTable;
  UINTN                 NoPorts;
  PORT_NUMBER_MAP       EndEntry;
  
  PortMappingTable = NULL;

  ZeroMem (&EndEntry, sizeof (PORT_NUMBER_MAP));

  PortMappingTable = (PORT_NUMBER_MAP *)PcdGetPtr (PcdPortNumberMapTable);

  NoPorts = 0;
  while (CompareMem (&EndEntry, &PortMappingTable[NoPorts], sizeof (PORT_NUMBER_MAP)) != 0) {
    NoPorts++;
  }

  if (NoPorts == 0) {
    return EFI_NOT_FOUND;
  }
    
  for (Index = 0; Index < NoPorts; Index++) {
    if ((PortMappingTable[Index].Bus == Bus) &&
        (PortMappingTable[Index].Device == Device) &&
        (PortMappingTable[Index].Function == Function) &&
        (PortMappingTable[Index].PrimarySecondary == PrimarySecondary) &&
        (PortMappingTable[Index].SlaveMaster == SlaveMaster)) {
      *PortNum = PortMappingTable[Index].PortNum;
      return EFI_SUCCESS;

    }
  }
  return EFI_NOT_FOUND;
}


/**
 Check Ide device and collect the device information in EFI Boot mode.

 @param        IdeConfig        the array of IDE_CONFIG, that record label number, device name, etc....

 @retval EFI_SUCCESS            it is success to check and get device information.

**/
EFI_STATUS
InitIdeConfigInEfiBootType (
  IDE_CONFIG                             *IdeConfig
)
{
  EFI_STATUS                            Status;
  UINTN                                 HandleCount;
  EFI_HANDLE                            *HandleBuffer;
  EFI_DISK_INFO_PROTOCOL                *DiskInfo;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePathNode;
  CHIPSET_CONFIGURATION                 *SetupVariable;
  UINTN                                 Index;
  PCI_DEVICE_PATH                       *PciDevicePath;
  ATAPI_DEVICE_PATH                     *AtapiDevicePath;
  SATA_DEVICE_PATH                      *SataDevicePath;
  UINTN                                 PortNum;
  ATAPI_IDENTIFY                        IdentifyDrive;
  UINT32                                Size;  
  CHAR16                                *ModelNametring;
  UINT32                                IdeChannel;
  UINT32                                IdeDevice;
  UINT16                                PortMap;
  UINT16                                Port;
  UINT8                                 SataCnfigure;
  BOOLEAN                               IsIDEModeFlag;
  NVME_ADMIN_CONTROLLER_DATA            NvmeAdminControllerData;
//[-start-130225-IB08520070-remove]//
//   VOID                                  *EfiCheckRaidProtocol;

//   EfiCheckRaidProtocol = NULL;
  SataCnfigure = gSUBrowser->SUCInfo->PrevSataCnfigure;
  PortMap = 0;
  Port    = 0;

  if (IdeConfig == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  SetupVariable = (CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer;
  if (SetupVariable->BootType != EFI_BOOT_TYPE) {
    return EFI_UNSUPPORTED;
  }  
  HandleCount = 0;
  //
  // Collect all disk device information
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiDiskInfoProtocolGuid,
                   NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
//[-start-160608-IB07400742-add]//
  if (EFI_ERROR (Status)) {
    return Status;
  } 
//[-end-160608-IB07400742-add]//
  for (Index = 0; Index < HandleCount; Index++) {
//[-start-160608-IB07400742-modify]//
    Status = gBS->HandleProtocol (
//[-end-160608-IB07400742-modify]//
           HandleBuffer[Index],
           &gEfiDiskInfoProtocolGuid,
           (VOID **)&DiskInfo
           );
    ASSERT_EFI_ERROR (Status);

    if (!(CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoIdeInterfaceGuid) ||
          CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid) ||
          CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoNvmeInterfaceGuid))) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID **) &DevicePath
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }
    
    
    DevicePathNode = DevicePath;
    Status = EFI_NOT_FOUND;
    PortNum = 0;
    IsIDEModeFlag = FALSE;
    
    switch (SataCnfigure) {

    case IDE_MODE:
      PciDevicePath = NULL;
      AtapiDevicePath = NULL;
      while (!IsDevicePathEnd (DevicePathNode)) {
        if ((DevicePathType (DevicePathNode) == HARDWARE_DEVICE_PATH) &&
            (DevicePathSubType (DevicePathNode) == HW_PCI_DP)) {
          PciDevicePath = (PCI_DEVICE_PATH *) DevicePathNode;
        }
        if ((DevicePathType (DevicePathNode) == MESSAGING_DEVICE_PATH) &&
            (DevicePathSubType (DevicePathNode) == MSG_ATAPI_DP)) {
          AtapiDevicePath = (ATAPI_DEVICE_PATH *) DevicePathNode;
          break;
        }
        DevicePathNode = NextDevicePathNode (DevicePathNode);
      }
      if (PciDevicePath != NULL && AtapiDevicePath != NULL) {
        Status = SearchMatchedPortNum (
                   0,
                   PciDevicePath->Device,
                   PciDevicePath->Function,
                   AtapiDevicePath->PrimarySecondary,
                   AtapiDevicePath->SlaveMaster,
                   &PortNum
                   );
        IsIDEModeFlag = TRUE;
      }
      break;
      
    case AHCI_MODE:
    case RAID_MODE:
      SataDevicePath = NULL;
      while (!IsDevicePathEnd (DevicePathNode)) {
        if ((DevicePathType (DevicePathNode) == MESSAGING_DEVICE_PATH) &&
            (DevicePathSubType (DevicePathNode) == MSG_SATA_DP)) {
          SataDevicePath = (SATA_DEVICE_PATH *) DevicePathNode;
          break;
        }
        DevicePathNode = NextDevicePathNode (DevicePathNode);
      }
      if (SataDevicePath != NULL) {

        if (SataCnfigure == AHCI_MODE) {
          PortNum = SataDevicePath->HBAPortNumber;
        } else if (SataCnfigure == RAID_MODE) {
//           Status = gBS->LocateProtocol (
//                           &gEfiCheckRaidProtocolGuid,
//                           NULL, 
//                           &EfiCheckRaidProtocol
//                           );
          if (SataDevicePath->PortMultiplierPortNumber & SATA_HBA_DIRECT_CONNECT_FLAG) {
            PortMap = SataDevicePath->HBAPortNumber;
            for (Port = 0; PortMap != 0; Port++, PortMap >>= 1);
            Port--;
            PortNum = Port;
          } else {
            PortNum = SataDevicePath->HBAPortNumber;
          }

        }
        Status = EFI_SUCCESS;
        IsIDEModeFlag = FALSE;
      }
      break;
      
    default:
      
      ASSERT (FALSE);
      break;
    }
     
    if (!EFI_ERROR (Status)) {
      Status = DiskInfo->WhichIde (DiskInfo, &IdeChannel, &IdeDevice);
      if (!EFI_ERROR (Status)) {
        Size = sizeof (ATAPI_IDENTIFY);
        Status = DiskInfo->Identify (
                             DiskInfo,
                             &IdentifyDrive,
                             &Size
                             );
        if (!EFI_ERROR (Status)) {
          IdeConfig[PortNum].DevNameString     = AllocateZeroPool (0x100);
          ModelNametring = AllocateZeroPool (0x100);
          ASSERT (ModelNametring != NULL);
          if (ModelNametring == NULL) {
            return EFI_OUT_OF_RESOURCES;
          }
          UpdateAtaString (
            (EFI_ATAPI_IDENTIFY_DATA *) &IdentifyDrive,
            &ModelNametring
            );
          IdeModelNameStrCpy(
            IdeConfig[PortNum].DevNameString,
            ModelNametring,
            20
            );
          IdeConfig[PortNum].IdeDevice = 1;
          IdeConfig[PortNum].DiskInfoHandle = HandleBuffer[Index];
          mIsIDEModeFlag[PortNum] = IsIDEModeFlag;
          gBS->FreePool (ModelNametring);
        }
      } else if (CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoNvmeInterfaceGuid)) {
        Size = sizeof (NVME_ADMIN_CONTROLLER_DATA);
        Status = DiskInfo->Inquiry (
                             DiskInfo,
                             &NvmeAdminControllerData,
                             &Size
                             );
        if (!EFI_ERROR (Status)) {
          IdeConfig[PortNum].DevNameString     = AllocateZeroPool (0x100);
          ModelNametring = AllocateZeroPool (0x100);
          ASSERT (ModelNametring != NULL);
          if (ModelNametring == NULL) {
            return EFI_OUT_OF_RESOURCES;
          }
          AsciiToUnicode (
            NvmeAdminControllerData.Mn,
            ModelNametring
            );
          IdeModelNameStrCpy(
            IdeConfig[PortNum].DevNameString,
            ModelNametring,
            20
            );
          IdeConfig[PortNum].IdeDevice = 1;
          IdeConfig[PortNum].DiskInfoHandle = HandleBuffer[Index];
        }
      }
    }
  }
  return EFI_SUCCESS; 

}

/**
 to check Ide device and collect the device information.

 @param        IdeConfig        the array of IDE_CONFIG, that record label number, device name, etc....

 @retval EFI_SUCCESS            it is success to check and get device information.

**/
EFI_STATUS
InitIdeConfig (
  IDE_CONFIG                             *IdeConfig
)
{
  EFI_STATUS                            Status;
  BBS_TABLE                             *LocalBbsTable;
  HDD_INFO                              *LocalHddInfo;
  EFI_LEGACY_BIOS_PROTOCOL              *LegacyBios;
  UINT16                                HddCount, BbsTotalCount;
  UINTN                                 BbsTableIndex;

  UINT8                                 Channel;
  UINT8                                 Device;
  UINT8                                 Index;
  CHAR16                                *ModelNametring;
  CHIPSET_CONFIGURATION                 *SetupVariable;
  UINTN                                 PortNum;
  EFI_ATAPI_IDENTIFY_DATA               *IdentifyTable;
  UINT8                                 SataCnfigure;

  SataCnfigure = gSUBrowser->SUCInfo->PrevSataCnfigure;
  if (mIdeInit && gSUBrowser->IdeConfig != NULL) {
    return EFI_SUCCESS;
  }
  SetupVariable = (CHIPSET_CONFIGURATION *)gSUBrowser->SCBuffer;
  if (SetupVariable->BootType == EFI_BOOT_TYPE) {
    Status = InitIdeConfigInEfiBootType (IdeConfig);
    if (!EFI_ERROR (Status)) {
      mIdeInit = TRUE;
    }
    return Status;
  } else {
    Status = gBS->LocateProtocol (
                    &gEfiLegacyBiosProtocolGuid,
                    NULL,
                    (VOID **)&LegacyBios
                    );

    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = LegacyBios->GetBbsInfo(
                           LegacyBios,
                           &HddCount,
                           &LocalHddInfo,
                           &BbsTotalCount,
                           &LocalBbsTable
                           );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Index         = 0;
    BbsTableIndex = 1;
    //
    // IDE Configuration
    //
    ModelNametring = AllocateZeroPool (0x100);
    ASSERT (ModelNametring != NULL);
    if (ModelNametring == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    for (Channel=0; Channel<4; Channel++) {
      for (Device=0; Device<2; Device++) {
        //
        // Initial String
        //
        IdentifyTable = (EFI_ATAPI_IDENTIFY_DATA *) &LocalHddInfo[Channel].IdentifyDrive[Device];
        if (IdentifyTable->ModelName[0] != 0) {
          if (SataCnfigure == IDE_MODE) {
            Status = SearchMatchedPortNum (
                                      LocalBbsTable[BbsTableIndex].Bus,
                                      LocalBbsTable[BbsTableIndex].Device,
                                      LocalBbsTable[BbsTableIndex].Function,
                                      Channel % 2,
                                      Device,
                                      &PortNum
                                      );
            if (EFI_ERROR (Status)) {
              BbsTableIndex++;
              continue;
            }
            if (IdeConfig[PortNum].DevNameString != NULL) {
              gBS->FreePool (IdeConfig[PortNum].DevNameString);
              IdeConfig[PortNum].DevNameString = NULL;
            }
            IdeConfig[PortNum].DevNameString     = AllocateZeroPool (0x100);
            IdeConfig[PortNum].IdeDevice         = 1;
            IdeConfig[PortNum].Device            = Device;
            IdeConfig[PortNum].Channel           = Channel;
            //
            // Get Device Model name
            //
            UpdateAtaString(
              (EFI_ATAPI_IDENTIFY_DATA *) &LocalHddInfo[Channel].IdentifyDrive[Device],
              &ModelNametring
              );
            IdeModelNameStrCpy(
              IdeConfig[PortNum].DevNameString,
              ModelNametring,
              20
              );
            mIsIDEModeFlag[PortNum] = TRUE;
          } else {
            if (IdeConfig[Index].DevNameString != NULL) {
              gBS->FreePool (IdeConfig[Index].DevNameString);
              IdeConfig[Index].DevNameString = NULL;
            }
            IdeConfig[Index].DevNameString     = AllocateZeroPool (0x100);
            IdeConfig[Index].IdeDevice         = 1;
            IdeConfig[Index].Device            = Device;
            IdeConfig[Index].Channel           = Channel;
            //
            // Get Device Model name
            //
            UpdateAtaString(
              (EFI_ATAPI_IDENTIFY_DATA *) &LocalHddInfo[Channel].IdentifyDrive[Device],
              &ModelNametring
              );
            IdeModelNameStrCpy(
              IdeConfig[Index].DevNameString,
              ModelNametring,
              20
              );
            PortNum = Index;
            mIsIDEModeFlag[PortNum] = TRUE;
          }
        }

        Index++;
        BbsTableIndex++;
      }
    }
    gBS->FreePool (ModelNametring);
    mIdeInit = TRUE;
  }


  return EFI_SUCCESS;
}

EFI_STATUS
SwapIdeConfig (
  IDE_CONFIG                              *IdeConfig1,
  IDE_CONFIG                              *IdeConfig2
)
{
  UINT8                                   *Temp;

  Temp = AllocateZeroPool (sizeof(IDE_CONFIG));
  CopyMem (
    Temp,
    IdeConfig1,
    sizeof (IDE_CONFIG)
    );
  CopyMem (
    IdeConfig1,
    IdeConfig2,
    sizeof (IDE_CONFIG)
    );

  CopyMem (
    IdeConfig2,
    Temp,
    sizeof (IDE_CONFIG)
    );

  gBS->FreePool (Temp);
  return EFI_SUCCESS;
}
