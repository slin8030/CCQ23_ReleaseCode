/** @file
  EFI HDD Password Protocol

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

#include "HddPassword.h"

EFI_SMM_SYSTEM_TABLE2                   *mSmst2;
EFI_SMM_BASE2_PROTOCOL                  *mSmmBase2;

HDD_PASSWORD_PRIVATE                    *mHddPasswordPrivate;
DRIVER_INSTALL_INFO                     *mDriverInstallInfo;

/**
  This function converts Unicode string to ASCII string.

  @param[in]        RawPasswordPtr      Password string address.
  @param[in]        RawPasswordLength   Password string length.
  @param[OUT]       PasswordToHdd       Encode password address.
  @param[OUT]       PasswordToHddLength Encode string length.

  @retval           EFI_SUCCESS
**/
EFI_STATUS
HddPasswordStringProcess (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN UINT8                              PasswordType,
  IN VOID                               *RawPasswordPtr,
  IN UINTN                              RawPasswordLength,
  OUT VOID                              **PasswordToHdd,
  OUT UINTN                             *PasswordToHddLength
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Index;
  CHAR16                                *RawPassword;
  UINT8                                 *ProcessedPasaword;

  if (!mSmst2) {
    Status = OemSvcDxeHddPasswordStringProcess (
               PasswordType,
               RawPasswordPtr,
               RawPasswordLength,
               (UINT8 **)PasswordToHdd,
               PasswordToHddLength
               );
  } else {
    Status = OemSvcSmmHddPasswordStringProcess (
               PasswordType,
               RawPasswordPtr,
               RawPasswordLength,
               (UINT8**)PasswordToHdd,
               PasswordToHddLength
               );
  }

  if (Status == EFI_UNSUPPORTED) {
    RawPassword = RawPasswordPtr;
    ProcessedPasaword = (UINT8 *)PasswordToHdd;

    for (Index = 0; Index < RawPasswordLength; Index++) {
      //
      // Unicode to ASCII
      //

      ProcessedPasaword[Index] = (UINT8)RawPassword[Index];
    }

    *PasswordToHddLength = RawPasswordLength;
  }

  return EFI_SUCCESS;
}

/**
  Update Legacy EBDA.
  This information is used for INT 13 when reading block.
  If the mapped bit is enabled, the device will be skipped.

  @param[in]        HddPasswordService  EFI_HDD_PASSWORD_SERVICE_PROTOCOL

  @retval           EFI_SUCCESS
  @retval           EFI_INVALID_PARAMETER

**/
EFI_STATUS
UpdateLegacyEbda (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *HddPasswordService,
  HDD_PASSWORD_HDD_INFO                 *HddInfo
  )
{

  if (HddPasswordService == NULL || HddInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mSmst2) {
    if (mDriverInstallInfo->LegacyBios != NULL) {
      if ((HddInfo->HddSecurityStatus & HDD_LOCKED_BIT) == HDD_LOCKED_BIT) {
        if (HddInfo->ControllerMode == ATA_IDE_MODE) {
          EBDA (EBDA_HDD_LOCKED_FLAG) |= BIT (HddInfo->MappedPort);
        } else {
          EBDA (EBDA_HDD_LOCKED_FLAG) |= BIT (HddInfo->PortNumber);
        }
      } else {
        if (HddInfo->ControllerMode == ATA_IDE_MODE) {
          EBDA (EBDA_HDD_LOCKED_FLAG) &= ~(BIT (HddInfo->MappedPort));
        } else {
          EBDA (EBDA_HDD_LOCKED_FLAG) &= ~(BIT (HddInfo->PortNumber));
        }
      }

      return EFI_SUCCESS;
    }
  }

  return EFI_UNSUPPORTED;
}

/**
  Get the HDD private information used in HDD Password internal.

  @param[in]        HddInfo             the HDD information pointer.
  @param[out]       HddInfoPrivate      the HDD private information pointer.

  @retval           EFI_NOT_FOUND       Not found the mapped HDD info.
  @retval           EFI_SUCCESS         Found the mapped HDD info.

**/
EFI_STATUS
GetPrivateFromHddPasswordHddInfo (
  IN HDD_PASSWORD_HDD_INFO              *HddInfo,
  OUT HDD_PASSWORD_HDD_INFO_PRIVATE     **HddInfoPrivatePtr
  )
{
  UINTN                                 Index;
  HDD_INFO_MANAGER                      *HddInfoManager;
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *HddInfoPrivateListHead;

  HddInfoPrivate = NULL;

  HddInfoManager = mHddPasswordPrivate->HddInfoManager;

  HddInfoPrivateListHead = &HddInfoManager->HddInfoPrivateListHead;
  for (Link = GetFirstNode (HddInfoPrivateListHead), Index = 0;
       !IsNull (HddInfoPrivateListHead, Link) && Index < HddInfoManager->NumOfHdd;
       Link = GetNextNode (HddInfoPrivateListHead, Link), Index++) {
    HddInfoPrivate = GET_HDD_INFO_PRIVATE_FROM_LINK (Link);
    if (HddInfoPrivate->DeviceHandleInDxe == HddInfo->DeviceHandleInDxe) {
      break;
    }
  }

  if (Index == HddInfoManager->NumOfHdd) {
    *HddInfoPrivatePtr = NULL;
    return EFI_NOT_FOUND;
  }

  *HddInfoPrivatePtr = HddInfoPrivate;
  return EFI_SUCCESS;
}


/**
  Check to see if the device is Edrive

  @param[in]  Sscp            Point to EFI_STORAGE_SECURITY_COMMAND_PROTOCOL.
  @param[in]  MediaId         Media ID.

  @retval TRUE                Found an Edrive.
  @retval FALSE               Not an Edrive.

**/
BOOLEAN
IsEdrive (
  IN     EFI_STORAGE_SECURITY_COMMAND_PROTOCOL  *Sscp,
  IN     UINT16                                 MediaId
  )
{

  EFI_STATUS              Status;
  TCG_LEVEL0_DISCOVERY_HEADER                *Header;
  UINT8                                      *Descriptor;
  UINT8                                      *EndOfDescriptor;
  UINTN                                      TransferSize = 0;
  UINT8                                      InternalResponse[BUFFER_SIZE];
  TCG_LEVEL0_FEATURE_DESCRIPTOR_HEADER       *Feature;



  ZeroMem (InternalResponse, BUFFER_SIZE);
  Status = Sscp->ReceiveData (
                Sscp,
                MediaId,
                0,
                0x01,
                0x0100,
                BUFFER_SIZE,
                InternalResponse,
                &TransferSize
                );

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Header = (TCG_LEVEL0_DISCOVERY_HEADER *)InternalResponse;
  Descriptor = InternalResponse + sizeof(TCG_LEVEL0_DISCOVERY_HEADER);
  EndOfDescriptor = InternalResponse + SwapBytes32 (Header->LengthBE);

  do {
    Feature = (TCG_LEVEL0_FEATURE_DESCRIPTOR_HEADER *)Descriptor;

    //
    // check if OPAL 1.0 supported
    //
    if (SwapBytes16(Feature->FeatureCode_BE) == TCG_FEATURE_OPAL_SSC_V1_0_0) {
      return TRUE;
    }

    Descriptor = Descriptor + (Feature->Length + 4);
  } while (Descriptor < EndOfDescriptor);

  return FALSE;
}


/**
  Get the HDD information used in HDD Password.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in, out]   HddInfoArray        Return the HDD information array to this pointer.
  @param[in, out]   NumOfHdd            Number of HDD

  @retval           EFI_SUCCESS

**/
EFI_STATUS
GetHddInfoArray (
  IN EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *This,
  IN OUT HDD_PASSWORD_HDD_INFO          **HddInfoArray,
  IN OUT UINTN                          *NumOfHdd
  )
{
  EFI_STATUS                            Status;
  UINTN                                 HddIndex;
  HDD_PASSWORD_HDD_INFO                 *HddInfoArrayPublic;
  HDD_PASSWORD_PRIVATE                  *HddPasswordPrivate;
  HDD_INFO_MANAGER                      *HddInfoManager;
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;
  LIST_ENTRY                            *HddInfoPrivateListHead;
  LIST_ENTRY                            *Link;

  if (This == NULL || HddInfoArray == NULL || NumOfHdd == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HddPasswordPrivate = GET_PRIVATE_FROM_HDD_PASSWORD (This);

  if (HddPasswordPrivate->HddInfoCollected == FALSE) {
    *NumOfHdd = 0;
    return EFI_NOT_READY;
  }

  HddInfoManager = HddPasswordPrivate->HddInfoManager;

  HddInfoArrayPublic = NULL;

  if (mSmst2 == NULL) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    (HddInfoManager->NumOfHdd) * sizeof(HDD_PASSWORD_HDD_INFO),
                    (VOID **)&HddInfoArrayPublic
                    );
  } else {
    Status = mSmst2->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      (HddInfoManager->NumOfHdd) * sizeof(HDD_PASSWORD_HDD_INFO),
                      (VOID **)&HddInfoArrayPublic
                      );
  }

  if (HddInfoArrayPublic == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (HddInfoArrayPublic, (HddInfoManager->NumOfHdd) * sizeof(HDD_PASSWORD_HDD_INFO));

  HddInfoPrivateListHead = &HddInfoManager->HddInfoPrivateListHead;
  for (Link = GetFirstNode (HddInfoPrivateListHead), HddIndex = 0;
       !IsNull (HddInfoPrivateListHead, Link) && HddIndex < HddInfoManager->NumOfHdd;
       Link = GetNextNode (HddInfoPrivateListHead, Link), HddIndex++) {
    HddInfoPrivate = GET_HDD_INFO_PRIVATE_FROM_LINK (Link);
    CopyMem (
      &HddInfoArrayPublic[HddIndex],
      &HddInfoPrivate->HddInfo,
      sizeof(HDD_PASSWORD_HDD_INFO)
      );
  }


  *HddInfoArray = HddInfoArrayPublic;
  *NumOfHdd = HddInfoManager->NumOfHdd;

  return EFI_SUCCESS;
}

/**
  Update the security status of HDD.

  @param[in]        HddInfo             The HDD information used in HDD Password.

  @retval           EFI_SUCCESS
  @retval           EFI_INVALID_PARAMETER

**/
EFI_STATUS
UpdateHddSecurityStatus (
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo
  )
{
  EFI_STATUS                            Status;
  ATA_IDENTIFY_DATA                     IdentifyData;
  UINTN                                 Index;
  HDD_INFO_MANAGER                      *HddInfoManager;
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;
  LIST_ENTRY                            *Link;
  LIST_ENTRY                            *HddInfoPrivateListHead;

  HddInfoPrivate = NULL;

  Status = mHddPasswordPrivate->HddPasswordProtocol.HddIdentify (
                                                      &(mHddPasswordPrivate->HddPasswordProtocol),
                                                      HddInfo,
                                                      (UINT16 *)&IdentifyData
                                                      );

  if (Status != EFI_SUCCESS) {
    return Status;
  }

  HddInfoManager = mHddPasswordPrivate->HddInfoManager;

  HddInfoPrivateListHead = &HddInfoManager->HddInfoPrivateListHead;
  for (Link = GetFirstNode (HddInfoPrivateListHead), Index = 0;
       !IsNull (HddInfoPrivateListHead, Link) && Index < HddInfoManager->NumOfHdd;
       Link = GetNextNode (HddInfoPrivateListHead, Link), Index++) {
    HddInfoPrivate = GET_HDD_INFO_PRIVATE_FROM_LINK (Link);
    if (HddInfoPrivate->DeviceHandleInDxe == HddInfo->DeviceHandleInDxe) {
      HddInfoPrivate->HddInfo.HddSecurityStatus = (UINT8) IdentifyData.security_status;
      break;
    }
  }

  if (Index == HddInfoManager->NumOfHdd) {
    return EFI_NOT_FOUND;
  }

  HddInfo->HddSecurityStatus = (UINT8) IdentifyData.security_status;

  UpdateLegacyEbda (&(mHddPasswordPrivate->HddPasswordProtocol), HddInfo);

  //
  //  OemService for update security status
  //
  if (!mSmst2) {
    OemSvcDxeHddUpdateSecurityStatus (
      &(mHddPasswordPrivate->HddPasswordProtocol),
      &(HddInfoPrivate->HddInfo),  //HddInfo,
      Index
      );
  } else {
    OemSvcSmmHddUpdateSecurityStatus (
      &(mHddPasswordPrivate->HddPasswordProtocol),
      &(HddInfoPrivate->HddInfo),  //HddInfo,
      Index
      );
  }

  return EFI_SUCCESS;
}


/**
  HDD identify command service.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in, out]   IdentifyData        Return the HDD identify data to this pointer.

  @retval           EFI_SUCCESS         Ata command success.

**/
EFI_STATUS
HddIdentify (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  OUT UINT16                        *IdentifyData
  )
{
  EFI_STATUS                            Status;

  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL *StorageSecurityCommand;
  ATA_IDENTIFY_DATA                     AtaIdentifyData;
  UINTN                                 PayloadBufferSize;
  UINTN                                 PayloadTransferSize;
  UINT64                                *CmdTimeoutTable;
  UINT64                                Timeout;
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;

  if (This == NULL || HddInfo == NULL || IdentifyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HddInfoPrivate = NULL;

  Status = GetPrivateFromHddPasswordHddInfo (HddInfo, &HddInfoPrivate);

  if (Status != EFI_SUCCESS || HddInfoPrivate == NULL) {
    return Status;
  }

  if (!mSmst2) {
    StorageSecurityCommand = HddInfoPrivate->StorageSecurityCommandInDxe;
  } else {
    StorageSecurityCommand = HddInfoPrivate->StorageSecurityCommandInSmm;
  }

  ZeroMem (&AtaIdentifyData, sizeof (ATA_IDENTIFY_DATA));

  CmdTimeoutTable = (UINT64*)PcdGetPtr (PcdH2OHddPasswordCmdTimeoutTable);

  Timeout = CmdTimeoutTable[TIMEOUT_IDENTIFY_DEVICE_INDEX];

  PayloadBufferSize   = sizeof (ATA_IDENTIFY_DATA);
  PayloadTransferSize = 0;

  Status = StorageSecurityCommand->ReceiveData (
                                     StorageSecurityCommand,
                                     HddInfoPrivate->BlockIoMediaId,
                                     Timeout,
                                     H2O_SECURITY_PROTOCOL_ID,
                                     RCV_IDENTIFY_DATA,
                                     PayloadBufferSize,
                                     &AtaIdentifyData,
                                     &PayloadTransferSize
                                     );

  CopyMem (IdentifyData, &AtaIdentifyData, sizeof (ATA_IDENTIFY_DATA));

  return Status;
}

/**
  Send HDD freeze command service.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.

  @retval           EFI_SUCCESS         Ata command success.

**/
EFI_STATUS
HddFreeze (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL    *This,
  IN  HDD_PASSWORD_HDD_INFO                *HddInfo
  )
{
  EFI_STATUS                               Status;
  EFI_STATUS                               CmdStatus;
  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL    *StorageSecurityCommand;
  UINT16                                   MasterPasswordIdentifier;
  H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER *StorageSecurityCmdSendHeader;
  UINTN                                    PayloadBufferSize;
  UINT64                                   *CmdTimeoutTable;
  UINT64                                   Timeout;
  HDD_PASSWORD_HDD_INFO_PRIVATE            *HddInfoPrivate;

  if (This == NULL || HddInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HddInfoPrivate = NULL;

  Status = GetPrivateFromHddPasswordHddInfo (HddInfo, &HddInfoPrivate);

  if (Status != EFI_SUCCESS || HddInfoPrivate == NULL) {
    return Status;
  }

  if (!mSmst2) {
    StorageSecurityCommand = HddInfoPrivate->StorageSecurityCommandInDxe;
  } else {
    StorageSecurityCommand = HddInfoPrivate->StorageSecurityCommandInSmm;
  }

  StorageSecurityCmdSendHeader = NULL;

  if (mSmst2 == NULL) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER),
                    (VOID **)&StorageSecurityCmdSendHeader
                    );
  } else {
    Status = mSmst2->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER),
                      (VOID **)&StorageSecurityCmdSendHeader
                      );
  }

  if (StorageSecurityCmdSendHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (StorageSecurityCmdSendHeader, sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER));

  if (HddInfo->MasterPasswordIdentifier != 0x0 && HddInfo->MasterPasswordIdentifier != 0xFFFF) {
    MasterPasswordIdentifier = PcdGet16(PcdH2OHddPasswordMasterPasswordIdentifier);
    if ( MasterPasswordIdentifier != DEFAULT_MASTER_PASSWORD_IDENTIFIER) {
      HddInfo->MasterPasswordIdentifier = MasterPasswordIdentifier;
    }
  }


  StorageSecurityCmdSendHeader->HeaderSize = sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER);
  StorageSecurityCmdSendHeader->Control = 0;
  StorageSecurityCmdSendHeader->Identifier = HddInfo->MasterPasswordIdentifier;
  StorageSecurityCmdSendHeader->MessageLength = 0;


  CmdTimeoutTable = (UINT64*)PcdGetPtr (PcdH2OHddPasswordCmdTimeoutTable);

  Timeout = CmdTimeoutTable[TIMEOUT_SECURITY_FREEZE_LOCK_INDEX];

  PayloadBufferSize = sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER);

  CmdStatus = StorageSecurityCommand->SendData(
                                        StorageSecurityCommand,
                                        HddInfoPrivate->BlockIoMediaId,
                                        Timeout,
                                        H2O_SECURITY_PROTOCOL_ID,
                                        SEND_PWD_FREEZE,
                                        PayloadBufferSize,
                                        StorageSecurityCmdSendHeader
                                        );

  UpdateHddSecurityStatus (HddInfo);

  if (StorageSecurityCmdSendHeader != NULL) {
    if (mSmst2 == NULL) {
      Status = gBS->FreePool (StorageSecurityCmdSendHeader);
    } else {
      Status = mSmst2->SmmFreePool (StorageSecurityCmdSendHeader);
    }
  }

  return CmdStatus;
}

/**
  Send HDD security command.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        CmdOpCode           Command opration code.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in]        UserOrMaster        User or Master flag.
  @param[in]        PasswordPtr         Password string.
  @param[in]        PasswordLength      Password string length.

  @retval           EFI_SUCCESS         ATA command success.

**/
EFI_STATUS
SendHddSecurityCmd (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL    *This,
  IN  UINT8                                CmdOpCode,
  IN  HDD_PASSWORD_HDD_INFO                *HddInfo,
  IN  BOOLEAN                              UserOrMaster,
  IN  UINT8                                *PasswordPtr,
  IN  UINTN                                PasswordLength
  )
{
  EFI_STATUS                               Status;
  EFI_STATUS                               CmdStatus;

  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL    *StorageSecurityCommand;

  UINT16                                   MasterPasswordIdentifier;
  UINT8                                    MaxPasswordLengthSupport;

  H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER *StorageSecurityCmdSendHeader;
  UINT16                                   SecurityProtocolSpecificData;
  UINTN                                    PayloadBufferSize;
  UINT64                                   *CmdTimeoutTable;
  UINT64                                   Timeout;

  HDD_PASSWORD_HDD_INFO_PRIVATE            *HddInfoPrivate;

  if (This == NULL || HddInfo == NULL || PasswordPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((PasswordLength > MAX_HDD_PASSWORD_LENGTH + 1) || (PasswordLength == 0)) {
    return EFI_OUT_OF_RESOURCES;
  }

  HddInfoPrivate = NULL;

  Status = GetPrivateFromHddPasswordHddInfo (HddInfo, &HddInfoPrivate);

  if (Status != EFI_SUCCESS || HddInfoPrivate == NULL) {
    return Status;
  }

  Timeout = 0;
  CmdTimeoutTable = (UINT64*)PcdGetPtr (PcdH2OHddPasswordCmdTimeoutTable);
  switch (CmdOpCode) {

  case ATA_CMD_SECURITY_SET_PASSWORD:
    SecurityProtocolSpecificData = SEND_PWD_SET;
    Timeout = CmdTimeoutTable[TIMEOUT_SECURITY_SET_PASSWORD_INDEX];
    break;

  case ATA_CMD_SECURITY_UNLOCK:
    SecurityProtocolSpecificData = SEND_PWD_UNLOCK;
    Timeout = CmdTimeoutTable[TIMEOUT_SECURITY_UNLOCK_INDEX];
    break;

  case ATA_CMD_SECURITY_DISABLE_PASSWORD:
    SecurityProtocolSpecificData = SEND_PWD_CLEAR;
    Timeout = CmdTimeoutTable[TIMEOUT_SECURITY_DISABLE_PASSWORD_INDEX];
    break;

  case H2O_HDD_PASSWORD_CMD_COMRESET:
    SecurityProtocolSpecificData = SEND_PWD_RESET_SECURITY_STATUS;
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }


  Status = EFI_SUCCESS;

  MaxPasswordLengthSupport = HddInfo->MaxPasswordLengthSupport;

  if ((UINT8)PasswordLength > (MaxPasswordLengthSupport + 1) || (PasswordLength == 0)) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (!mSmst2) {
    StorageSecurityCommand = HddInfoPrivate->StorageSecurityCommandInDxe;
  } else {
    StorageSecurityCommand = HddInfoPrivate->StorageSecurityCommandInSmm;
  }

  StorageSecurityCmdSendHeader = NULL;

  if (mSmst2 == NULL) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    (sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER) + MaxPasswordLengthSupport),
                    (VOID **)&StorageSecurityCmdSendHeader
                    );
  } else {
    Status = mSmst2->SmmAllocatePool (
                       EfiRuntimeServicesData,
                       (sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER) + MaxPasswordLengthSupport),
                       (VOID **)&StorageSecurityCmdSendHeader
                       );
  }

  if (StorageSecurityCmdSendHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (StorageSecurityCmdSendHeader, (sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER) + MaxPasswordLengthSupport));

  if (HddInfo->MasterPasswordIdentifier != 0x0 && HddInfo->MasterPasswordIdentifier != 0xFFFF) {
    MasterPasswordIdentifier = PcdGet16(PcdH2OHddPasswordMasterPasswordIdentifier);
    if ( MasterPasswordIdentifier != DEFAULT_MASTER_PASSWORD_IDENTIFIER) {
      HddInfo->MasterPasswordIdentifier = MasterPasswordIdentifier;
    }
  }

  StorageSecurityCmdSendHeader->HeaderSize = sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER);
  StorageSecurityCmdSendHeader->Control = UserOrMaster;
  StorageSecurityCmdSendHeader->Identifier = HddInfo->MasterPasswordIdentifier;
  StorageSecurityCmdSendHeader->MessageLength = PasswordLength;
  CopyMem (&StorageSecurityCmdSendHeader[1], PasswordPtr, MIN (MaxPasswordLengthSupport, PasswordLength));

  PayloadBufferSize = sizeof (H2O_STORAGE_SECURITY_COMMAND_SEND_HEADER) + MaxPasswordLengthSupport;

  CmdStatus = StorageSecurityCommand->SendData(
                                        StorageSecurityCommand,
                                        HddInfoPrivate->BlockIoMediaId,
                                        Timeout,
                                        H2O_SECURITY_PROTOCOL_ID,
                                        SecurityProtocolSpecificData,
                                        PayloadBufferSize,
                                        StorageSecurityCmdSendHeader
                                        );

  UpdateHddSecurityStatus (HddInfo);

  if (StorageSecurityCmdSendHeader != NULL) {
    if (mSmst2 == NULL) {
      Status = gBS->FreePool (StorageSecurityCmdSendHeader);
    } else {
      Status = mSmst2->SmmFreePool (StorageSecurityCmdSendHeader);
    }
  }

  return CmdStatus;

}

/**
  Set HDD password command services.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in]        UserOrMaster        User or Master flag.
  @param[in]        PasswordPtr         Password string.
  @param[in]        PasswordLength      Password string length.

  @retval           EFI_SUCCESS         Set HDD password successfully.
**/
EFI_STATUS
SetHddPassword (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINT8                             *PasswordPtr,
  IN  UINTN                             PasswordLength
  )
{
  EFI_STATUS                            Status;

  if (This == NULL ||
      HddInfo == NULL ||
      (UserOrMaster != USER_PSW && UserOrMaster != MASTER_PSW) ||
      PasswordPtr == NULL ||
      PasswordLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mSmst2) {
    Status = OemSvcDxeHddSetPassword (
               This,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  } else {
    Status = OemSvcSmmHddSetPassword (
               This,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  }

  if (Status == EFI_UNSUPPORTED) {
    Status = SendHddSecurityCmd (
               This,
               ATA_CMD_SECURITY_SET_PASSWORD,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  } else if (Status == EFI_MEDIA_CHANGED) {
    //
    //  Oem feature code may occur error.
    //

    return EFI_ABORTED;
  }

  return Status;
}

/**
  Unlock HDD password command services.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in]        UserOrMaster        User or Master flag.
  @param[in]        PasswordPtr         Password string.
  @param[in]        PasswordLength      Password string length.

  @retval           EFI_SUCCESS         Unlock HDD password successfully.
**/
EFI_STATUS
UnlockHddPassword (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINT8                             *PasswordPtr,
  IN  UINTN                             PasswordLength
  )
{
  EFI_STATUS                            Status;

  if (This == NULL ||
      HddInfo == NULL ||
      PasswordPtr == NULL ||
      PasswordLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mSmst2) {
    Status = OemSvcDxeHddUnlockPassword (
               This,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  } else {
    Status = OemSvcSmmHddUnlockPassword (
               This,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  }

  if (Status == EFI_UNSUPPORTED) {
    Status = SendHddSecurityCmd (
               This,
               ATA_CMD_SECURITY_UNLOCK,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  } else if (Status == EFI_MEDIA_CHANGED) {
    //
    //  Oem feature code may occur error.
    //

    return EFI_ABORTED;
  }

  return Status;
}

/**
  Disable HDD password command services.

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfo             The HDD information used in HDD Password.
  @param[in]        UserOrMaster        User or Master flag.
  @param[in]        PasswordPtr         Password string.
  @param[in]        PasswordLength      Password string length.

  @retval           EFI_SUCCESS         Disable HDD password successfully.
**/
EFI_STATUS
DisableHddPassword (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *This,
  IN  HDD_PASSWORD_HDD_INFO              *HddInfo,
  IN  BOOLEAN                            UserOrMaster,
  IN  UINT8                              *PasswordPtr,
  IN  UINTN                              PasswordLength
  )
{
  EFI_STATUS                             Status;

  if (This == NULL ||
      HddInfo == NULL ||
      PasswordPtr == NULL ||
      PasswordLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (!mSmst2) {
    Status = OemSvcDxeHddDisablePassword (
               This,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  } else {
    Status = OemSvcSmmHddDisablePassword (
               This,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  }

  if (Status == EFI_UNSUPPORTED) {
    Status = SendHddSecurityCmd (
               This,
               ATA_CMD_SECURITY_DISABLE_PASSWORD,
               HddInfo,
               UserOrMaster,
               PasswordPtr,
               PasswordLength
               );
  } else if (Status == EFI_MEDIA_CHANGED) {
    //
    //  Oem feature code may occur error.
    //

    return EFI_ABORTED;
  }

  return Status;
}

/**
  This routine could unlock all of the HDD password when S3/S4 resume

  @param[in]        This                EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.

  @retval           EFI_SUCCESS
**/
EFI_STATUS
UnlockAllHdd (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This
  )
{
  EFI_STATUS                            Status;
  EFI_STATUS                            FreePoolStatus;
  UINTN                                 Index;
  UINTN                                 NumOfHdd;
  UINT8                                 PasswordToHdd[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                                 PasswordToHddLength;
  UINTN                                 RetryCount;
  UINTN                                 HddPasswordTableSize;
  HDD_PASSWORD_PRIVATE                  *HddPasswordPrivate;
  HDD_PASSWORD_TABLE                    *HddPasswordTable;
  HDD_PASSWORD_HDD_INFO                 *HddInfoArray;
  UINTN                                 HddPasswordTableIndex;
  BOOLEAN                               HddPasswordFound;

  Status           = EFI_SUCCESS;
  HddPasswordTable = NULL;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HddPasswordPrivate = GET_PRIVATE_FROM_HDD_PASSWORD (This);


  Status = CommonGetVariableDataAndSize (
             SAVE_HDD_PASSWORD_VARIABLE_NAME,
             &gSaveHddPasswordGuid,
             &HddPasswordTableSize,
             (VOID **) &HddPasswordTable
             );

  if (HddPasswordTable == NULL) {
    return EFI_NOT_FOUND;
  }

  Status = GetHddInfoArray (
             This,
             &HddInfoArray,
             &NumOfHdd
             );

  if (NumOfHdd == 0) {
    return EFI_SUCCESS;
  }

  if (Status != EFI_SUCCESS) {
    return Status;
  }

  if (!mSmst2) {
    Status = OemSvcDxeHddPasswordUnlockAllHdd (
               This,
               HddInfoArray,
               NumOfHdd,
               HddPasswordTable,
               HddPasswordTableSize
               );
  } else {
    Status = OemSvcSmmHddPasswordUnlockAllHdd (
               This,
               HddInfoArray,
               NumOfHdd,
               HddPasswordTable,
               HddPasswordTableSize
               );
  }

  if (Status == EFI_UNSUPPORTED) {
    for (Index = 0; Index < NumOfHdd; Index++) {
      if ((HddInfoArray[Index].HddSecurityStatus & HDD_ENABLE_BIT) != HDD_ENABLE_BIT) {
        continue;
      }

      if ((HddInfoArray[Index].HddSecurityStatus & HDD_LOCKED_BIT) != HDD_LOCKED_BIT) {
        continue;
      }

      if ((HddInfoArray[Index].HddSecurityStatus & HDD_FROZEN_BIT) == HDD_FROZEN_BIT) {
        continue;
      }

      HddPasswordFound = FALSE;
      for (HddPasswordTableIndex = 0; HddPasswordTableIndex < ((HddPasswordTableSize / sizeof (HDD_PASSWORD_TABLE)) - 1); HddPasswordTableIndex++) {
        if (HddPasswordTable[HddPasswordTableIndex].ControllerNumber == HddInfoArray[Index].ControllerNumber &&
            HddPasswordTable[HddPasswordTableIndex].PortNumber       == HddInfoArray[Index].PortNumber &&
            HddPasswordTable[HddPasswordTableIndex].PortMulNumber    == HddInfoArray[Index].PortMulNumber) {
          HddPasswordFound = TRUE;
          break;
        }
      }

      if (HddPasswordFound != TRUE) {
        continue;
      }

      Status = HddPasswordStringProcess (
                 This,
                 HddPasswordTable[HddPasswordTableIndex].PasswordType,
                 HddPasswordTable[HddPasswordTableIndex].PasswordStr,
                 StrLen (HddPasswordTable[HddPasswordTableIndex].PasswordStr),
                 (VOID **)&PasswordToHdd,
                 &PasswordToHddLength
                 );


      for (RetryCount = 0; RetryCount < DEFAULT_RETRY_COUNT; RetryCount++) {
        Status = This->UnlockHddPassword (
                         This,
                         &HddInfoArray[Index],
                         HddPasswordTable[HddPasswordTableIndex].PasswordType,
                         PasswordToHdd,
                         PasswordToHddLength
                         );
        if (!EFI_ERROR (Status)) {
          break;
        }
      }

      if ((HddInfoArray[Index].HddSecurityStatus & HDD_EXPIRED_BIT) == HDD_EXPIRED_BIT && Status != EFI_SUCCESS) {
        //
        //  No error handling
        //

      }

    }

  }

  if (HddPasswordTable != NULL) {
    if (mSmst2 == NULL) {
      FreePoolStatus = gBS->FreePool (HddPasswordTable);
    } else {
      FreePoolStatus = mSmst2->SmmFreePool (HddPasswordTable);
    }
  }

  if (HddInfoArray != NULL) {
    if (mSmst2 == NULL) {
      FreePoolStatus = gBS->FreePool (HddInfoArray);
    } else {
      FreePoolStatus = mSmst2->SmmFreePool (HddInfoArray);
    }
  }

  return Status;
}

/**
  Check HDD SSP feature. (SSP: Software Settings Preservation)
  If the security of HDDs is ENABLE and the status of HDDs is FROZEN,
  using this function to make the status of HDDs to be LOCK.

  @param[in]        HddPasswordProtocol EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfoArray        The array of HDD information used in HDD Password.
  @param[in]        NumOfHdd            Number of HDD password information.

  @retval           EFI_SUCCESS         the security of HDDs is DISABLE.
  @retval           EFI_NOT_READY       the security of HDDs is ENABLE.
  @retval           others              there may be something wrong.

**/
EFI_STATUS
ResetSecuirtyStatus (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfo
  )
{
  EFI_STATUS                            Status;
  BOOLEAN                               UserOrMaster;
  UINT8                                 PasswordToHdd[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                                 PasswordToHddLength;
  UINTN                                 HddPasswordTableSize;
  HDD_PASSWORD_TABLE                    *HddPasswordTable;
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;
  UINTN                                 HddPasswordTableIndex;

  if (This == NULL || HddInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetPrivateFromHddPasswordHddInfo (HddInfo, &HddInfoPrivate);

  if (Status != EFI_SUCCESS) {
    return Status;
  }

  HddPasswordTableSize = 0;
  HddPasswordTable = NULL;

  Status = CommonGetVariableDataAndSize (
             SAVE_HDD_PASSWORD_VARIABLE_NAME,
             &gSaveHddPasswordGuid,
             &HddPasswordTableSize,
             (VOID **) &HddPasswordTable
             );


  if (!mSmst2) {
    Status = OemSvcDxeHddResetSecurityStatus (
               This,
               HddInfo,
               HddPasswordTable,
               HddPasswordTableSize
               );
  } else {
    Status = OemSvcSmmHddResetSecurityStatus (
               This,
               HddInfo,
               HddPasswordTable,
               HddPasswordTableSize
               );
  }

  if (Status == EFI_UNSUPPORTED) {
    UserOrMaster = USER_PSW;
    ZeroMem (&PasswordToHdd, (HDD_PASSWORD_MAX_NUMBER + 1));
    PasswordToHddLength = HddInfo->MaxPasswordLengthSupport;

    if (HddPasswordTableSize != 0 && HddPasswordTable != NULL) {
      for (HddPasswordTableIndex = 0; HddPasswordTableIndex < ((HddPasswordTableSize / sizeof (HDD_PASSWORD_TABLE)) - 1); HddPasswordTableIndex++) {
        if (HddPasswordTable[HddPasswordTableIndex].ControllerNumber == HddInfo->ControllerNumber &&
            HddPasswordTable[HddPasswordTableIndex].PortNumber       == HddInfo->PortNumber &&
            HddPasswordTable[HddPasswordTableIndex].PortMulNumber    == HddInfo->PortMulNumber) {

          Status = HddPasswordStringProcess (
                     This,
                     HddPasswordTable[HddPasswordTableIndex].PasswordType,
                     HddPasswordTable[HddPasswordTableIndex].PasswordStr,
                     StrLen (HddPasswordTable[HddPasswordTableIndex].PasswordStr),
                     (VOID **)&PasswordToHdd,
                     &PasswordToHddLength
                     );

          UserOrMaster = HddPasswordTable[HddPasswordTableIndex].PasswordType;
          break;
        }
      }
    }

    //
    //  Partial storage types (Ex:eMMC) need original password to set security status from unlocked to locked
    //  and others (Ex: SATA HDD) don't need.
    //
    Status = SendHddSecurityCmd (
               This,
               H2O_HDD_PASSWORD_CMD_COMRESET,
               HddInfo,
               UserOrMaster,
               PasswordToHdd,
               PasswordToHddLength
               );

  } else if (Status == EFI_MEDIA_CHANGED) {
    //
    //  Oem feature code may occur error.
    //

    return EFI_ABORTED;
  }

  return Status;
}

/**
  Disabled locked HDD SATA port.

  @param[in]        HddPasswordProtocol EFI_HDD_PASSWORD_SERVICE_PROTOCOL instance.
  @param[in]        HddInfoArray        The array of HDD information used in HDD Password.
  @param[in]        NumOfHdd            Number of HDD password information.

  @retval           EFI_SUCCESS         the security of HDDs is DISABLE.
  @retval           EFI_NOT_READY       the security of HDDs is ENABLE.
  @retval           others              there may be something wrong.

**/
EFI_STATUS
DisabledLockedHdd (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfo
  )
{
  EFI_STATUS                            Status;
  UINT32                                AhciMemAddr;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  EFI_DISK_INFO_PROTOCOL                *DiskInfo;
  HDD_PASSWORD_PRIVATE                  *HddPasswordPrivate;

  if (This == NULL || HddInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = EFI_SUCCESS;

  HddPasswordPrivate = GET_PRIVATE_FROM_HDD_PASSWORD (This);

  if (HddPasswordPrivate->HddPasswordS3Data.SmmMode) {
    //
    //  Only DXE supported, SMM not supported.
    //

    return EFI_UNSUPPORTED;
  }

  AhciMemAddr = 0;

  if ((HddInfo->HddSecurityStatus & HDD_LOCKED_BIT) == HDD_LOCKED_BIT) {

    Status = gBS->HandleProtocol (
                    HddInfo->DeviceHandleInDxe,
                    &gEfiDiskInfoProtocolGuid,
                    (VOID **)&DiskInfo
                    );

    if (Status != EFI_SUCCESS) {
      return EFI_UNSUPPORTED;
    }

    //
    // filter not HDD
    //
    if (!(CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoIdeInterfaceGuid) ||
          CompareGuid (&DiskInfo->Interface, &gEfiDiskInfoAhciInterfaceGuid))) {
      return EFI_UNSUPPORTED;
    }

    Status = gBS->HandleProtocol (
                    HddInfo->DeviceHandleInDxe,
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (Status == EFI_SUCCESS) {
      Status = PciIo->Pci.Read (
                            PciIo,
                            EfiPciIoWidthUint32,
                            PCI_AHCI_BAR,
                            sizeof (UINT32),
                            &AhciMemAddr
                            );
      if (Status == EFI_SUCCESS) {
        Mmio32Or (AhciMemAddr, (HBA_PORTS_START + HBA_PORTS_SCTL + (HddInfo->PortNumber) * HBA_PORTS_REG_WIDTH), HBA_PORTS_SCTL_DET_OFFLINE);
      }

    }

  }

  return Status;
}

/**
  To Collect the storage device information.

  @param

  @retval EFI_SUCCESS
**/
STATIC
EFI_STATUS
CollectDeviceInfo (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            ControllerHandle;
  EFI_HANDLE                            *StorageSecurityCommandHandleBuffer;
  UINTN                                 StorageSecurityCommandHandleCount;
  UINTN                                 StorageSecurityCommandHandleIndex;
  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL *StorageSecurityCommand;
  EFI_BLOCK_IO_PROTOCOL                 *BlockIo;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  UINTN                                 Seg;
  UINTN                                 Bus;
  UINTN                                 Device;
  UINTN                                 Function;
  UINT64                                *CmdTimeoutTable;
  UINT64                                Timeout;
  UINTN                                 PayloadBufferSize;
  UINTN                                 PayloadTransferSize;
  ATA_IDENTIFY_DATA                     IdentifyData;
  UINT8                                 NumOfHdd;
  UINT8                                 MaxPasswordLengthSupport;
  EFI_DISK_INFO_PROTOCOL                *DiskInfo;
  UINT32                                IdeChannel;
  UINT32                                IdeDevice;
  UINT8                                 SataClassCReg[3];
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;
  HDD_INFO_MANAGER                      *HddInfoManager;
  UINT32                                MediaId;

  StorageSecurityCommandHandleBuffer = NULL;
  StorageSecurityCommandHandleCount  = 0;

  DiskInfo = NULL;
  BlockIo  = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiStorageSecurityCommandProtocolGuid,
                  NULL,
                  &StorageSecurityCommandHandleCount,
                  &StorageSecurityCommandHandleBuffer
                  );
  if (Status != EFI_SUCCESS || StorageSecurityCommandHandleBuffer == NULL) {
    return EFI_UNSUPPORTED;
  }

  if (StorageSecurityCommandHandleCount == 0) {
    return EFI_SUCCESS;
  }

  HddInfoManager = mHddPasswordPrivate->HddInfoManager;

  InitializeListHead (&HddInfoManager->HddInfoPrivateListHead);

  NumOfHdd = 0;

  //
  //  Collect storage device Info
  //
  for (StorageSecurityCommandHandleIndex = 0; StorageSecurityCommandHandleIndex < StorageSecurityCommandHandleCount; StorageSecurityCommandHandleIndex++) {

    //
    //  Locate Storage Security Command Ptotocol
    //
    Status = gBS->HandleProtocol (
                    StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex],
                    &gEfiStorageSecurityCommandProtocolGuid,
                    (VOID **)&StorageSecurityCommand
                    );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    Status = gBS->HandleProtocol (
              StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex],
              &gEfiDiskInfoProtocolGuid,
              (VOID **)&DiskInfo
              );

    //
    //  Locate Block IO Ptotocol to get Media ID
    //
    MediaId = 0;
    Status = gBS->HandleProtocol (
                    StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **)&BlockIo
                    );
    if (Status != EFI_SUCCESS) {
      //
      // In RAID mode, BlockIo is installed by RAID OPROM.
      // However, HDD Password must unlock HDD before RAID OPROM dispatched
      //   which cause there is no BlockIo in Disk Handle.
      // Locate DiskInfo to know the handle is Disk Handle.
      //
      if (DiskInfo == NULL) {
        continue;
      }
    } else {
      MediaId = BlockIo->Media->MediaId;
    }


    if (IsEdrive(StorageSecurityCommand, (UINT16)MediaId)) {
      continue;
    }


    //
    //  Get Identify information of device
    //
    CmdTimeoutTable = (UINT64*)PcdGetPtr (PcdH2OHddPasswordCmdTimeoutTable);
    Timeout = CmdTimeoutTable[TIMEOUT_IDENTIFY_DEVICE_INDEX];

    ZeroMem (&IdentifyData, sizeof (ATA_IDENTIFY_DATA));

    PayloadBufferSize   = sizeof (ATA_IDENTIFY_DATA);
    PayloadTransferSize = 0;

    Status = StorageSecurityCommand->ReceiveData (
                                       StorageSecurityCommand,
                                       MediaId,
                                       Timeout,
                                       H2O_SECURITY_PROTOCOL_ID,
                                       RCV_IDENTIFY_DATA,
                                       PayloadBufferSize,
                                       &IdentifyData,
                                       &PayloadTransferSize
                                       );

    if (Status != EFI_SUCCESS) {
      continue;
    }


    //
    //  Locate PCI IO Ptotocol to get Bus, Device, Function
    //
    Status = gBS->HandleProtocol (
                    StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex],
                    &gEfiDevicePathProtocolGuid,
                    (VOID *) &DevicePath
                    );
    if (Status != EFI_SUCCESS) {
      continue;
    }


    ControllerHandle = NULL;
    Status = gBS->LocateDevicePath(&gEfiPciIoProtocolGuid, &DevicePath, &ControllerHandle);
    if (Status != EFI_SUCCESS || ControllerHandle == NULL) {
      continue;
    }

    Seg      = 0;
    Bus      = 0;
    Device   = 0;
    Function = 0;
    Status = gBS->HandleProtocol (
                    ControllerHandle,
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (!EFI_ERROR (Status)) {
      Status = PciIo->GetLocation (
                        PciIo,
                        &Seg,
                        &Bus,
                        &Device,
                        &Function
                        );
      if (Status != EFI_SUCCESS) {
        continue;
      }
    }

    //
    // Get max password length supported.
    //
    PayloadBufferSize   = sizeof (UINT8);
    PayloadTransferSize = 0;
    MaxPasswordLengthSupport = 0;

    Status = StorageSecurityCommand->ReceiveData (
                                       StorageSecurityCommand,
                                       MediaId,
                                       0,
                                       H2O_SECURITY_PROTOCOL_ID,
                                       RCV_MAX_PWD_LEN,
                                       PayloadBufferSize,
                                       &MaxPasswordLengthSupport,
                                       &PayloadTransferSize
                                       );

    if (Status != EFI_SUCCESS) {
      continue;
    }

    HddInfoPrivate = NULL;
    HddInfoPrivate = AllocateReservedZeroPool (sizeof (HDD_PASSWORD_HDD_INFO_PRIVATE));

    if (HddInfoPrivate == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    //  Build device information
    //
    HddInfoPrivate->HddInfo.HddSecurityStatus        = IdentifyData.security_status;
    HddInfoPrivate->HddInfo.MasterPasswordIdentifier = IdentifyData.master_password_identifier;
    HddInfoPrivate->HddInfo.ControllerNumber         = FindMappedController (Seg, Bus, Device, Function);
    HddInfoPrivate->HddInfo.PciSeg                   = Seg;
    HddInfoPrivate->HddInfo.PciBus                   = Bus;
    HddInfoPrivate->HddInfo.PciDevice                = Device;
    HddInfoPrivate->HddInfo.PciFunction              = Function;
    HddInfoPrivate->HddInfo.DeviceHandleInDxe        = StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex];
    HddInfoPrivate->HddInfo.MaxPasswordLengthSupport = MaxPasswordLengthSupport;
    HddInfoPrivate->HddInfo.HddInfoExtPtr            = NULL;

    GetModelNumber (
      HddInfoPrivate->HddInfo.HddModelString,
      (VOID *)&(IdentifyData.ModelName)
      );

    if (DiskInfo != NULL) {
      Status = PciIo->Pci.Read (
                            PciIo,
                            EfiPciIoWidthUint8,
                            PCI_CLASSCODE_OFFSET,
                            3,
                            SataClassCReg
                            );
      if (Status == EFI_SUCCESS) {
        switch (SataClassCReg[1]) {

        case PCI_SUB_CLASS_IDE:
          HddInfoPrivate->HddInfo.ControllerMode = ATA_IDE_MODE;
          break;
        case PCI_SUB_CLASS_AHCI:
          HddInfoPrivate->HddInfo.ControllerMode = ATA_AHCI_MODE;
          break;
        case PCI_SUB_CLASS_RAID:
          HddInfoPrivate->HddInfo.ControllerMode = ATA_RAID_MODE;
          break;
        default:
          HddInfoPrivate->HddInfo.ControllerMode = 0xFF;
          break;
        }

        Status = DiskInfo->WhichIde (
                             DiskInfo,
                             &IdeChannel,
                             &IdeDevice
                             );
        if (Status == EFI_SUCCESS) {
          HddInfoPrivate->HddInfo.PortNumber    = (UINT16)IdeChannel;
          HddInfoPrivate->HddInfo.PortMulNumber = (UINT16)IdeDevice;

          if (HddInfoPrivate->HddInfo.ControllerMode == ATA_IDE_MODE) {
            Status = ChangeChannelDevice2PortNum (
                       (UINT32)HddInfoPrivate->HddInfo.PciBus,
                       (UINT32)HddInfoPrivate->HddInfo.PciDevice,
                       (UINT32)HddInfoPrivate->HddInfo.PciFunction,
                       (UINT8)HddInfoPrivate->HddInfo.PortNumber,
                       (UINT8)HddInfoPrivate->HddInfo.PortMulNumber,
                       (UINTN *)&HddInfoPrivate->HddInfo.MappedPort
                       );
          } else {
            HddInfoPrivate->HddInfo.MappedPort = 0xFF;
          }
        }
      }
    }

    if (Status != EFI_SUCCESS || DiskInfo == NULL) {
      HddInfoPrivate->HddInfo.ControllerMode = 0xFF;
      HddInfoPrivate->HddInfo.PortNumber     = 0xFF;
      HddInfoPrivate->HddInfo.PortMulNumber  = 0xFF;
      HddInfoPrivate->HddInfo.MappedPort     = 0xFF;
    }


    HddInfoPrivate->Signature = HDD_PASSWORD_HDD_INFO_PRIVATE_SIGNATURE;
    HddInfoPrivate->BlockIoMediaId = MediaId;
    HddInfoPrivate->DeviceHandleInDxe = StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex];
    HddInfoPrivate->StorageSecurityCommandInDxe = StorageSecurityCommand;

    InsertTailList (
      &HddInfoManager->HddInfoPrivateListHead,
      &HddInfoPrivate->Link
      );

    //
    //  Device count
    //
    NumOfHdd ++;
  }


  if (StorageSecurityCommandHandleBuffer != NULL) {
    FreePool (StorageSecurityCommandHandleBuffer);
  }


  HddInfoManager->NumOfHdd = NumOfHdd;

  mDriverInstallInfo->HddInfoCollected = TRUE;
  mHddPasswordPrivate->HddInfoCollected = TRUE;

  return EFI_SUCCESS;
}

/**
  To collect storage information.

  @param[in]        Event               Pointer to this event
  @param[in]        Context             Event hanlder private data

**/
VOID
HddPasswordDriverInstall (
  IN BOOLEAN                            ConnectControler
  )
{
  EFI_STATUS                            Status;
  UINTN                                 HddIndex;
  HDD_INFO_MANAGER                      *HddInfoManager;
  HDD_PASSWORD_HDD_INFO_PRIVATE         *HddInfoPrivate;
  LIST_ENTRY                            *HddInfoPrivateListHead;
  LIST_ENTRY                            *Link;

  EFI_SMM_COMMUNICATION_PROTOCOL        *SmmCommunication;
  UINT8                                 SmmCommBuffer[SMM_COMMUNICATE_HEADER_SIZE + sizeof (HDD_PASSWORD_SMM_PARAMETER_HEADER)];
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommHeader;
  HDD_PASSWORD_SMM_PARAMETER_HEADER     *HddPasswordSmmHeader;
  UINTN                                 CommBufferSize;
  ATA_CONTROLLER_INFO                   *AtaControllerInfo;
  DRIVER_INSTALL_INFO                   *DriverInstallInfo;
  LIST_ENTRY                            *AtaControllerInfoListHead;
  UINT16                                ControllerIndex;

  HddInfoManager = mHddPasswordPrivate->HddInfoManager;
  HddInfoPrivateListHead = &HddInfoManager->HddInfoPrivateListHead;
  for (Link = GetFirstNode (HddInfoPrivateListHead), HddIndex = 0;
       !IsNull (HddInfoPrivateListHead, Link) && HddIndex < HddInfoManager->NumOfHdd;
       Link = GetNextNode (HddInfoPrivateListHead, Link), HddIndex++) {
    HddInfoPrivate = GET_HDD_INFO_PRIVATE_FROM_LINK (Link);

    RemoveEntryList (Link);
    FreePool (HddInfoPrivate);
  }

  DriverInstallInfo = mDriverInstallInfo;
  AtaControllerInfoListHead = &DriverInstallInfo->AtaControllerInfoListHead;
  for (Link = GetFirstNode (AtaControllerInfoListHead), ControllerIndex = 0;
       !IsNull (AtaControllerInfoListHead, Link) && ControllerIndex < DriverInstallInfo->NumOfController;
       Link = GetNextNode (AtaControllerInfoListHead, Link), ControllerIndex++) {
    AtaControllerInfo = (ATA_CONTROLLER_INFO *)Link;

    RemoveEntryList (Link);
    FreePool (AtaControllerInfo);
  }

  CollectStorageController (ConnectControler);

  CollectDeviceInfo ();

  //
  // Trigger SMI for SMM instance
  //
  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **) &SmmCommunication);
  if (Status == EFI_SUCCESS) {
    CommBufferSize = SMM_COMMUNICATE_HEADER_SIZE + sizeof (HDD_PASSWORD_SMM_PARAMETER_HEADER);
    SmmCommHeader = (EFI_SMM_COMMUNICATE_HEADER*) SmmCommBuffer;
    ZeroMem (SmmCommHeader, CommBufferSize);
    CopyGuid (&SmmCommHeader->HeaderGuid , &gEfiHddPasswordServiceProtocolGuid);
    HddPasswordSmmHeader = (HDD_PASSWORD_SMM_PARAMETER_HEADER*) SmmCommHeader->Data;
    HddPasswordSmmHeader->Function = HDD_PASSWORD_SMM_COLLECT_DEVICE_INSTANCE;
    Status = SmmCommunication->Communicate (SmmCommunication, SmmCommHeader, &CommBufferSize);
  }
}

/**
  To install HDD password services.

  @param[in]        Event               Pointer to this event
  @param[in]        Context             Event hanlder private data

**/
VOID
H2OBdsCpConnectAllAfterCallback (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
   HddPasswordDriverInstall (FALSE);

}

/**
  To install HDD password services.

  @param[in]        Event               Pointer to this event
  @param[in]        Context             Event hanlder private data

**/
VOID
H2OBdsCpDisplayStringBeforeCallback (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  EFI_STATUS                            Status;

  gBS->CloseEvent (Event);

  if (!mHddPasswordPrivate->UefiOsFastBootSupported) {

    HddPasswordDriverInstall (TRUE);

    Status = HddUnlockDialogInit (&(mHddPasswordPrivate->HddPasswordProtocol));

  }

  if (mDriverInstallInfo->LegacyBios != NULL) {
    //
    //  Process RAID mode in legacy boot type
    //

    Status = CheckLegacyRaidSupport ();
  }
}

/**
  S3 resume to check HDDs password.

  @param[in]        Protocol            Points to the protocol's unique identifier
  @param[in]        Interface           Points to the interface instance
  @param[in]        Handle              The handle on which the interface was installed

  @retval EFI_STATUS

**/
EFI_STATUS
S3CheckHddPassword (
  IN CONST EFI_GUID                     *Protocol,
  IN VOID                               *Interface,
  IN EFI_HANDLE                         Handle
  )
{
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordServiceSmm;
  EFI_STATUS                            Status;
  HDD_PASSWORD_HDD_INFO                 *HddInfoArray;
  UINTN                                 NumOfHdd;
  UINTN                                 Index;
  BOOLEAN                               HddLock;
  HDD_PASSWORD_S3_DATA                  *HddPasswordS3Data;

  if (mHddPasswordPrivate == NULL) {
    return EFI_UNSUPPORTED;
  }

  HddInfoArray = NULL;
  HddLock      = FALSE;

  HddPasswordServiceSmm = &(mHddPasswordPrivate->HddPasswordProtocol);

  HddPasswordS3Data      = &(mHddPasswordPrivate->HddPasswordS3Data);

  //
  // Get Hddinfo table
  //
  Status = GetHddInfoArray (
             HddPasswordServiceSmm,
             &HddInfoArray,
             &NumOfHdd
             );

  if (NumOfHdd == 0) {
    //
    // Not found any Harddisk
    //
    return EFI_SUCCESS;
  }

  for (Index = 0; Index < NumOfHdd; Index++) {
    if ((HddInfoArray[Index].HddSecurityStatus & HDD_ENABLE_BIT) == HDD_ENABLE_BIT) {
      Status = ResetSecuirtyStatus (
                 HddPasswordServiceSmm,
                 &(HddInfoArray[Index])
                 );
    }
  }

  if (HddInfoArray != NULL) {
    Status = mSmst2->SmmFreePool (HddInfoArray);
  }

  Status = UnlockAllHdd (HddPasswordServiceSmm);

  return Status;
}

/**
  Initialize the data which are used in SMM.

  @param[in]        Protocol            Points to the protocol's unique identifier
  @param[in]        Interface           Points to the interface instance
  @param[in]        Handle              The handle on which the interface was installed

  @retval EFI_STATUS

**/
EFI_STATUS
HddPasswordSmmDriverInstall (
  IN CONST EFI_GUID                     *Protocol,
  IN VOID                               *Interface,
  IN EFI_HANDLE                         Handle
  )
{
  EFI_STATUS                            Status;
  HDD_PASSWORD_MEM_RECORD               *MemRecord;
  EFI_HANDLE                            HddPasswordHandle;
  EFI_EVENT                             AcpiRestoreCallbackDoneEvent;
  VOID                                  *HddPasswordS3Registration;
  EFI_HDD_PASSWORD_SERVICE_PROTOCOL     *HddPasswordProtocolDxe;
  HDD_PASSWORD_PRIVATE                  *HddPasswordPrivateDxe;

  HddPasswordHandle   = NULL;

  AcpiRestoreCallbackDoneEvent = NULL;


  HddPasswordS3Registration = NULL;
  HddPasswordProtocolDxe = NULL;
  HddPasswordPrivateDxe  = NULL;

  //
  //  get info through HddPassword protocol in DXE
  //
  Status = gBS->LocateProtocol (
                  &gEfiHddPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&HddPasswordProtocolDxe
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  HddPasswordPrivateDxe = GET_PRIVATE_FROM_HDD_PASSWORD (HddPasswordProtocolDxe);

  if (HddPasswordPrivateDxe->HddInfoManager == NULL) {
    return EFI_NOT_READY;
  }

  if (HddPasswordPrivateDxe->HddInfoCollected == FALSE) {
    return EFI_NOT_READY;
  }

  if (mHddPasswordPrivate == NULL) {
    //
    //  get PAGE address
    //
    MemRecord = HddPasswordPrivateDxe->MemRecord;

    //
    //  Alloc private data in SMM
    //
    mHddPasswordPrivate = *(HDD_PASSWORD_PRIVATE **)&(MemRecord->RemainMemAddr);

    MemRecord->RemainMemAddr += ALIGN_SIZEOF_UINTN (sizeof (HDD_PASSWORD_PRIVATE));
    MemRecord->RemainSize    -= ALIGN_SIZEOF_UINTN (sizeof (HDD_PASSWORD_PRIVATE));

    mHddPasswordPrivate->MemRecord = MemRecord;
    mHddPasswordPrivate->Signature = HDD_PASSWORD_SIGNATURE;

    //
    //  Get HddInfoPrivate pointer
    //
    mHddPasswordPrivate->HddInfoManager = HddPasswordPrivateDxe->HddInfoManager;

  }

  mHddPasswordPrivate->HddInfoManager = HddPasswordPrivateDxe->HddInfoManager;

  mHddPasswordPrivate->HddPasswordProtocol.SetHddPassword          = SetHddPassword;
  mHddPasswordPrivate->HddPasswordProtocol.UnlockHddPassword       = UnlockHddPassword;
  mHddPasswordPrivate->HddPasswordProtocol.DisableHddPassword      = DisableHddPassword;
  mHddPasswordPrivate->HddPasswordProtocol.HddIdentify             = HddIdentify;
  mHddPasswordPrivate->HddPasswordProtocol.GetHddInfo              = GetHddInfoArray;
  mHddPasswordPrivate->HddPasswordProtocol.UnlockAllHdd            = UnlockAllHdd;
  mHddPasswordPrivate->HddPasswordProtocol.HddFreeze               = HddFreeze;
  mHddPasswordPrivate->HddPasswordProtocol.SendHddSecurityCmd      = SendHddSecurityCmd;
  mHddPasswordPrivate->HddPasswordProtocol.ResetSecuirtyStatus     = ResetSecuirtyStatus;
  mHddPasswordPrivate->HddPasswordProtocol.PasswordStringProcess   = HddPasswordStringProcess;
  mHddPasswordPrivate->HddPasswordProtocol.DisabledLockedHdd       = DisabledLockedHdd;

  Status = mSmst2->SmmInstallProtocolInterface (
                     &HddPasswordHandle,
                     &gEfiHddPasswordServiceProtocolGuid,
                     EFI_NATIVE_INTERFACE,
                     &(mHddPasswordPrivate->HddPasswordProtocol)
                     );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mHddPasswordPrivate->HddPasswordS3Data.SmmMode          = TRUE;

  mHddPasswordPrivate->HddInfoCollected = TRUE;

  mHddPasswordPrivate->UefiOsFastBootSupported = HddPasswordPrivateDxe->UefiOsFastBootSupported;

  //
  //  S3 resume event
  //
  Status = mSmst2->SmmRegisterProtocolNotify (
                     &gAcpiRestoreCallbackStartProtocolGuid,
                     (EFI_SMM_NOTIFY_FN)S3CheckHddPassword,
                     &HddPasswordS3Registration
                     );

  return Status;

}


/**
  DevicePath comapre.

  @param[in]        Event               Pointer to this event
  @param[in]        Context             Event hanlder private data

  @retval EFI_STATUS

**/
EFI_STATUS
HddPasswordCompareDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL           *DevicePathD,
  IN EFI_DEVICE_PATH_PROTOCOL           *DevicePathS
  )
{
  EFI_DEVICE_PATH_PROTOCOL              *DevicePathNodeD;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePathNodeS;

  DevicePathNodeD = DevicePathD;
  DevicePathNodeS = DevicePathS;

  while (!IsDevicePathEnd (DevicePathNodeD) && !IsDevicePathEnd (DevicePathNodeS)) {
    if (DevicePathNodeLength (DevicePathNodeD) != DevicePathNodeLength (DevicePathNodeS)) {
      return EFI_ABORTED;
    }
    if (CompareMem (DevicePathNodeD, DevicePathNodeS, DevicePathNodeLength (DevicePathNodeS)) != 0) {
      return EFI_ABORTED;
    }
    DevicePathNodeD = NextDevicePathNode (DevicePathNodeD);
    DevicePathNodeS = NextDevicePathNode (DevicePathNodeS);
  }

  if (IsDevicePathEnd (DevicePathNodeD) && IsDevicePathEnd (DevicePathNodeS)) {
    return EFI_SUCCESS;
  }

  return EFI_ABORTED;
}


/**
  Communication service SMI Handler entry.
  This SMI handler provides services for the firmware volume access through SMI.

  @param[in]     DispatchHandle               The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     RegisterContext              Points to an optional handler context which was specified when the
                                              handler was registered.
  @param[in, out] CommBuffer                  A pointer to a collection of data in memory that will
                                              be conveyed from a non-SMM environment into an SMM environment.
  @param[in, out] CommBufferSize              The size of the CommBuffer.

  @retval EFI_SUCCESS                         The interrupt was handled and quiesced. No other handlers
                                              should still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_QUIESCED  The interrupt has been quiesced but other handlers should
                                              still be called.
  @retval EFI_WARN_INTERRUPT_SOURCE_PENDING   The interrupt is still pending and other handlers should still
                                              be called.
  @retval EFI_INTERRUPT_PENDING               The interrupt could not be quiesced.
**/
EFI_STATUS
EFIAPI
HddPasswordSmiHandler (
  IN     EFI_HANDLE                                DispatchHandle,
  IN     CONST VOID                                *RegisterContext,
  IN OUT VOID                                      *CommBuffer,
  IN OUT UINTN                                     *CommBufferSize
  )
{
  EFI_STATUS                                       Status;
  EFI_HANDLE                                       *StorageSecurityCommandHandleBuffer;
  UINTN                                            StorageSecurityCommandHandleBufferSize;
  UINTN                                            StorageSecurityCommandHandleCount;
  UINTN                                            StorageSecurityCommandHandleIndex;
  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL            *StorageSecurityCommand;
  EFI_DEVICE_PATH_PROTOCOL                         *DevicePathD;
  EFI_DEVICE_PATH_PROTOCOL                         *DevicePathS;
  HDD_PASSWORD_SMM_PARAMETER_HEADER                *SmmCommHeader;

  EFI_HDD_PASSWORD_SERVICE_PROTOCOL                *HddPasswordProtocolDxe;
  HDD_PASSWORD_PRIVATE                             *HddPasswordPrivateDxe;

  UINTN                                            NumOfHdd;
  HDD_PASSWORD_HDD_INFO_PRIVATE                    *HddInfoPrivate;

  BOOLEAN                                          HddInfoPrivateInDxeFound;

  VOID                                             *HddPasswordS3Registration;
  HDD_INFO_MANAGER                                 *HddInfoManager;
  LIST_ENTRY                                       *HddInfoPrivateListHead;
  LIST_ENTRY                                       *Link;

  HddInfoPrivate = NULL;

  //
  // If input is invalid, stop processing this SMI
  //
  if ((CommBuffer == NULL) || (CommBufferSize == NULL)) {
    return EFI_SUCCESS;
  }
  SmmCommHeader = (HDD_PASSWORD_SMM_PARAMETER_HEADER*) CommBuffer;

  switch (SmmCommHeader->Function) {
  case HDD_PASSWORD_SMM_FUNCTION_INSTALL_INSTANCE:
  case HDD_PASSWORD_SMM_COLLECT_DEVICE_INSTANCE:
    //
    // Continue establishing SMM instance
    //

    break;

  case HDD_PASSWORD_ACPI_RESTORE_CALLBACK_NOTIFY_INSTANCE:
    //
    // Register nofity for S3 resuming
    //

    HddPasswordS3Registration = NULL;

    Status = mSmst2->SmmRegisterProtocolNotify (
                       &gAcpiRestoreCallbackStartProtocolGuid,
                       (EFI_SMM_NOTIFY_FN)S3CheckHddPassword,
                       &HddPasswordS3Registration
                       );

    goto Done;

  default:
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  //
  //  get info through HddPassword protocol in DXE
  //
  Status = gBS->LocateProtocol (
                  &gEfiHddPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **)&HddPasswordProtocolDxe
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  HddPasswordPrivateDxe = GET_PRIVATE_FROM_HDD_PASSWORD (HddPasswordProtocolDxe);

  if (HddPasswordPrivateDxe->HddInfoManager == NULL) {
    Status =  EFI_NOT_READY;
    goto Done;
  }

  HddInfoManager = HddPasswordPrivateDxe->HddInfoManager;
  NumOfHdd       = HddInfoManager->NumOfHdd;

  StorageSecurityCommandHandleCount      = 0;
  StorageSecurityCommandHandleBufferSize = 0;
  StorageSecurityCommandHandleBuffer     = NULL;

  Status = mSmst2->SmmLocateHandle (
                     ByProtocol,
                     &gEfiStorageSecurityCommandProtocolGuid,
                     NULL,
                     &StorageSecurityCommandHandleBufferSize,
                     StorageSecurityCommandHandleBuffer
                     );
  if (Status == EFI_BUFFER_TOO_SMALL) {

   Status = mSmst2->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      StorageSecurityCommandHandleBufferSize,
                      (VOID **)&StorageSecurityCommandHandleBuffer
                      );
   if (Status != EFI_SUCCESS || StorageSecurityCommandHandleBuffer == NULL) {
     Status =  EFI_OUT_OF_RESOURCES;
     goto Done;
   }

   Status = mSmst2->SmmLocateHandle (
                      ByProtocol,
                      &gEfiStorageSecurityCommandProtocolGuid,
                      NULL,
                      &StorageSecurityCommandHandleBufferSize,
                      StorageSecurityCommandHandleBuffer
                      );
  }

  if (Status != EFI_SUCCESS || StorageSecurityCommandHandleBuffer == NULL) {
    if (StorageSecurityCommandHandleBuffer != NULL) {
      mSmst2->SmmFreePool (StorageSecurityCommandHandleBuffer);
    }
    Status =  EFI_UNSUPPORTED;
    goto Done;
  }


  StorageSecurityCommandHandleCount = StorageSecurityCommandHandleBufferSize / sizeof (EFI_HANDLE);

  //
  //  Collect storage device Info
  //
  for (StorageSecurityCommandHandleIndex = 0; StorageSecurityCommandHandleIndex < StorageSecurityCommandHandleCount; StorageSecurityCommandHandleIndex++) {

    //
    //  Locate Storage Security Command Ptotocol
    //
    Status = mSmst2->SmmHandleProtocol (
                       StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex],
                       &gEfiStorageSecurityCommandProtocolGuid,
                       (VOID **)&StorageSecurityCommand
                       );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    //
    //  Locate DevicePath in SMM
    //
    Status = mSmst2->SmmHandleProtocol (
                       StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex],
                       &gEfiDevicePathProtocolGuid,
                       (VOID *) &DevicePathS
                       );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    HddInfoPrivateInDxeFound = FALSE;
    DevicePathD = NULL;
    HddInfoPrivateListHead = &HddInfoManager->HddInfoPrivateListHead;
    for (Link = GetFirstNode (HddInfoPrivateListHead);
         !IsNull (HddInfoPrivateListHead, Link);
         Link = GetNextNode (HddInfoPrivateListHead, Link)) {
      HddInfoPrivate = GET_HDD_INFO_PRIVATE_FROM_LINK (Link);
      //
      //  Locate DevicePath in DXE
      //
      Status = gBS->HandleProtocol (
                      HddInfoPrivate->DeviceHandleInDxe,
                      &gEfiDevicePathProtocolGuid,
                      (VOID *) &DevicePathD
                      );
      if (Status != EFI_SUCCESS) {
        continue;
      }

      Status = HddPasswordCompareDevicePath (DevicePathD, DevicePathS);

      if (Status == EFI_SUCCESS) {
        HddInfoPrivateInDxeFound = TRUE;
        break;
      }
    }

    if (HddInfoPrivateInDxeFound == FALSE) {
      continue;
    }

    //
    //  Update SMM data to HDD Info private
    //

    HddInfoPrivate->HddInfo.DeviceHandleInSmm = StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex];

    HddInfoPrivate->DeviceHandleInSmm = StorageSecurityCommandHandleBuffer[StorageSecurityCommandHandleIndex];
    HddInfoPrivate->StorageSecurityCommandInSmm = StorageSecurityCommand;

  }


  if (StorageSecurityCommandHandleBuffer != NULL) {
    mSmst2->SmmFreePool (StorageSecurityCommandHandleBuffer);
  }

  if (SmmCommHeader->Function == HDD_PASSWORD_SMM_FUNCTION_INSTALL_INSTANCE ||
      mHddPasswordPrivate == NULL) {
    HddPasswordSmmDriverInstall (NULL, NULL, NULL);
  }

Done:
  SmmCommHeader->ReturnStatus = Status;
  return EFI_SUCCESS;
}

/**
  ReadyToBootEvent callback function.
  This function is used to trigger SMI for register gAcpiRestoreCallbackStartProtocolGuid in SMM.

  @param[in]        Event               Pointer to this event
  @param[in]        Context             Event hanlder private data

**/
VOID
HddPasswordReadyToBootEventCallback (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  EFI_STATUS                            Status;
  EFI_SMM_COMMUNICATION_PROTOCOL        *SmmCommunication;
  UINT8                                 SmmCommBuffer[SMM_COMMUNICATE_HEADER_SIZE + sizeof (HDD_PASSWORD_SMM_PARAMETER_HEADER)];
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommHeader;
  HDD_PASSWORD_SMM_PARAMETER_HEADER     *HddPasswordSmmHeader;
  UINTN                                 CommBufferSize;

  Status = EFI_SUCCESS;

  //
  // Trigger SMI for SMM instance
  //
  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **) &SmmCommunication);
  if (Status == EFI_SUCCESS) {
    CommBufferSize = SMM_COMMUNICATE_HEADER_SIZE + sizeof (HDD_PASSWORD_SMM_PARAMETER_HEADER);
    SmmCommHeader = (EFI_SMM_COMMUNICATE_HEADER*) SmmCommBuffer;
    ZeroMem (SmmCommHeader, CommBufferSize);
    CopyGuid (&SmmCommHeader->HeaderGuid , &gEfiHddPasswordServiceProtocolGuid);
    HddPasswordSmmHeader = (HDD_PASSWORD_SMM_PARAMETER_HEADER*) SmmCommHeader->Data;
    HddPasswordSmmHeader->Function = HDD_PASSWORD_ACPI_RESTORE_CALLBACK_NOTIFY_INSTANCE;
    Status = SmmCommunication->Communicate (SmmCommunication, SmmCommHeader, &CommBufferSize);
  }

  gBS->CloseEvent (Event);
}

/**
  Main entry for this driver.

  @param[in]        ImageHandle         Image handle this driver.
  @param[in]        SystemTable         Pointer to SystemTable.

  @retval           EFI_SUCESS          This function always complete successfully.

**/
EFI_STATUS
HddPasswordEntryPoint (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  BOOLEAN                               InSmm;
  HDD_PASSWORD_MEM_RECORD               *MemRecord;
  EFI_PHYSICAL_ADDRESS                  Addr;
  HDD_INFO_MANAGER                      *HddInfoManager;
  //
  //  DXE
  //
  EFI_HANDLE                            Handle;
  H2O_BDS_CP_HANDLE                     H2OBdsCpDisplayStringBeforeHandle;
  EFI_EVENT                             ReadyToBootEvent;
  HDD_PASSWORD_TABLE                    *HddPasswordTable;
  UINTN                                 HddPasswordTableSize;
  UINT8                                 H2OHddPasswordUefiOsFastBootSupport;
  H2O_BDS_CP_HANDLE                     H2OBdsCpConnectAllAfterHandle;

  //
  //  SMM
  //
  EFI_HANDLE                            DispatchHandle;

  InSmm  = FALSE;
  Handle = NULL;


  //
  // Find the SMM base 2 protocol
  //
  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&mSmmBase2
                  );

  if (!(EFI_ERROR (Status))) {
    Status = mSmmBase2->InSmm (
                          mSmmBase2,
                          &InSmm
                          );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (!InSmm) {
    //
    //  In DXE
    //


    //
    //  Alloc reserved PAGE
    //

    Addr = 0xFFFFFFFF;

    Status = gBS->AllocatePages (
                AllocateMaxAddress,
                EfiReservedMemoryType,
                EFI_SIZE_TO_PAGES (HDD_PASSWORD_REQUEST_MEM_SIZE),
                &Addr
                );

    if (Status != EFI_SUCCESS) {
      return EFI_OUT_OF_RESOURCES;
    }
    ZeroMem (*(VOID **)&Addr, HDD_PASSWORD_REQUEST_MEM_SIZE);
    MemRecord = *(HDD_PASSWORD_MEM_RECORD**)&Addr;

    //
    //  init. PAGE
    //
    MemRecord->BaseAddr      = Addr;
    MemRecord->AllocatedSize = HDD_PASSWORD_REQUEST_MEM_SIZE;
    MemRecord->RemainMemAddr = MemRecord->BaseAddr + ALIGN_SIZEOF_UINTN (sizeof (HDD_PASSWORD_PRIVATE));
    MemRecord->RemainSize    = HDD_PASSWORD_REQUEST_MEM_SIZE - ALIGN_SIZEOF_UINTN (sizeof (HDD_PASSWORD_PRIVATE));


    //
    //  Alloc private data
    //
    mHddPasswordPrivate = *(HDD_PASSWORD_PRIVATE **)&(MemRecord->RemainMemAddr);

    MemRecord->RemainMemAddr += ALIGN_SIZEOF_UINTN (sizeof (HDD_PASSWORD_PRIVATE));
    MemRecord->RemainSize    -= ALIGN_SIZEOF_UINTN (sizeof (HDD_PASSWORD_PRIVATE));

    mHddPasswordPrivate->MemRecord = MemRecord;
    mHddPasswordPrivate->Signature = HDD_PASSWORD_SIGNATURE;

    //
    //  Alloc HDD Info private
    //
    Status = GetMem (
               sizeof(HDD_INFO_MANAGER),
               (VOID **)&HddInfoManager
               );

    mHddPasswordPrivate->HddInfoManager = HddInfoManager;

    //
    // Initialize the platform specific string and language
    //
    InitializeStringSupport ();


    //
    //  Initialize Driver install info structure
    //
    Status = GetMem (
               sizeof(DRIVER_INSTALL_INFO),
               (VOID **)&mDriverInstallInfo
               );

    mDriverInstallInfo->HddInfoCollected       = FALSE;

    mHddPasswordPrivate->HddPasswordProtocol.SetHddPassword          = SetHddPassword;
    mHddPasswordPrivate->HddPasswordProtocol.UnlockHddPassword       = UnlockHddPassword;
    mHddPasswordPrivate->HddPasswordProtocol.DisableHddPassword      = DisableHddPassword;
    mHddPasswordPrivate->HddPasswordProtocol.HddIdentify             = HddIdentify;
    mHddPasswordPrivate->HddPasswordProtocol.GetHddInfo              = GetHddInfoArray;
    mHddPasswordPrivate->HddPasswordProtocol.UnlockAllHdd            = UnlockAllHdd;
    mHddPasswordPrivate->HddPasswordProtocol.HddFreeze               = HddFreeze;
    mHddPasswordPrivate->HddPasswordProtocol.SendHddSecurityCmd      = SendHddSecurityCmd;
    mHddPasswordPrivate->HddPasswordProtocol.ResetSecuirtyStatus     = ResetSecuirtyStatus;
    mHddPasswordPrivate->HddPasswordProtocol.PasswordStringProcess   = HddPasswordStringProcess;
    mHddPasswordPrivate->HddPasswordProtocol.DisabledLockedHdd       = DisabledLockedHdd;

    Status = gBS->InstallProtocolInterface (
                    &Handle,
                    &gEfiHddPasswordServiceProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &(mHddPasswordPrivate->HddPasswordProtocol)
                    );
    if (Status != EFI_SUCCESS) {
      return Status;
    }

    //
    //  If support RAID mode with Legacy boot type, it have to hook the service "InstallPciRom" of Legace Bios protcol.
    //
    Status = InitLegacyRaidSupport ();
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Register for callback on Storage Security Protocol publication
    //
    H2OBdsCpDisplayStringBeforeHandle = NULL;
    Status = BdsCpRegisterHandler (
               &gH2OBdsCpDisplayStringBeforeProtocolGuid,
               H2OBdsCpDisplayStringBeforeCallback,
               HDD_PASSWORD_CONNECT_CONTROLLER_NOTIFY_TPL,
               &H2OBdsCpDisplayStringBeforeHandle
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = EfiCreateEventReadyToBootEx (
                  TPL_CALLBACK,
                  (EFI_EVENT_NOTIFY)HddPasswordReadyToBootEventCallback,
                  NULL,
                  &ReadyToBootEvent
                  );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Get PcdH2OHddPasswordUefiOsFastBootSupport to check UEFI OS Fast Boot support
    //
    mHddPasswordPrivate->UefiOsFastBootSupported = FALSE;

    H2OHddPasswordUefiOsFastBootSupport = PcdGetBool (PcdH2OHddPasswordUefiOsFastBootSupport);

    if (H2OHddPasswordUefiOsFastBootSupport) {
      //
      //  Check variable for HDD password exist?
      //

      HddPasswordTableSize = 0;
      HddPasswordTable = NULL;

      Status = CommonGetVariableDataAndSize (
                 SAVE_HDD_PASSWORD_VARIABLE_NAME,
                 &gSaveHddPasswordGuid,
                 &HddPasswordTableSize,
                 (VOID **) &HddPasswordTable
                 );
      if (HddPasswordTable == NULL) {
        //
        // Collect storage devices information after connect all
        //
        H2OBdsCpConnectAllAfterHandle = NULL;
        Status = BdsCpRegisterHandler (
                   &gH2OBdsCpConnectAllAfterProtocolGuid,
                   H2OBdsCpConnectAllAfterCallback,
                   TPL_CALLBACK,
                   &H2OBdsCpConnectAllAfterHandle
                   );
        if (EFI_ERROR (Status)) {
          return Status;
        }

        mHddPasswordPrivate->UefiOsFastBootSupported = TRUE;
      } else {
        gBS->FreePool (HddPasswordTable);
      }

    }

  } else {
    //
    // In SMM
    //

    Status = mSmmBase2->GetSmstLocation (
                          mSmmBase2,
                          &mSmst2
                          );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    mHddPasswordPrivate = NULL;

    //
    // Register SMM communication handler
    //
    DispatchHandle = NULL;

    Status = mSmst2->SmiHandlerRegister (
                       HddPasswordSmiHandler,
                       &gEfiHddPasswordServiceProtocolGuid,
                       &DispatchHandle
                       );

  }

  return EFI_SUCCESS;
}
