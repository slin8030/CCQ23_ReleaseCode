/** @file
  Definitions of hard disk password variable

;******************************************************************************
;* Copyright (c) 2012 - 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _EFI_HDD_PASSWORD_SERVICE_PROTOCOL_H
#define _EFI_HDD_PASSWORD_SERVICE_PROTOCOL_H

#define EFI_HDD_PASSWORD_SERVICE_PROTOCOL_GUID \
  { \
    0x9c28be0c, 0xee32, 0x43d8, 0xa2, 0x23, 0xe7, 0xc1, 0x61, 0x4e, 0xf7, 0xca \
  }

#define EFI_HDD_PASSWORD_DIALOG_PROTOCOL_GUID \
  { \
    0x9c0c75ab, 0x0fa5, 0x436c, 0x85, 0x0c, 0xf0, 0x73, 0xcd, 0x8e, 0x1b, 0x17 \
  }

typedef struct _EFI_HDD_PASSWORD_SERVICE_PROTOCOL EFI_HDD_PASSWORD_SERVICE_PROTOCOL;

typedef struct _EFI_HDD_PASSWORD_DIALOG_PROTOCOL EFI_HDD_PASSWORD_DIALOG_PROTOCOL;

#define HDD_PASSWORD_CONNECT_CONTROLLER_NOTIFY_TPL    (TPL_CALLBACK - 1)

#define HDD_PASSWORD_MAX_NUMBER            32
#define DEFAULT_RETRY_COUNT                10

#define ATA_IDE_MODE                        0
#define ATA_AHCI_MODE                       1
#define ATA_RAID_MODE                       2
#define ATA_IDER_MODE                       3


#ifndef ATA_CMD_SECURITY_SET_PASSWORD
#define ATA_CMD_SECURITY_SET_PASSWORD           0xF1
#endif

#ifndef ATA_CMD_SECURITY_UNLOCK
#define ATA_CMD_SECURITY_UNLOCK                 0xF2
#endif

#ifndef ATA_CMD_SECURITY_ERASE_PREPARE
#define ATA_CMD_SECURITY_ERASE_PREPARE          0xF3
#endif

#ifndef ATA_CMD_SECURITY_ERASE_UNIT
#define ATA_CMD_SECURITY_ERASE_UNIT             0xF4
#endif

#ifndef ATA_CMD_SECURITY_FREEZE_LOCK
#define ATA_CMD_SECURITY_FREEZE_LOCK            0xF5
#endif

#ifndef ATA_CMD_SECURITY_DISABLE_PASSWORD
#define ATA_CMD_SECURITY_DISABLE_PASSWORD       0xF6
#endif

#define H2O_HDD_PASSWORD_CMD_COMRESET           0xFE

#define HDD_ENABLE_BIT                    BIT1
#define HDD_LOCKED_BIT                    BIT2
#define HDD_FROZEN_BIT                    BIT3
#define HDD_EXPIRED_BIT                   BIT4
#define HDD_MASTER_CAPABILITY             BIT8

#define TIMEOUT_IDENTIFY_DEVICE_INDEX             0
#define TIMEOUT_SECURITY_SET_PASSWORD_INDEX       1
#define TIMEOUT_SECURITY_UNLOCK_INDEX             2
#define TIMEOUT_SECURITY_ERASE_PREPARE_INDEX      3
#define TIMEOUT_SECURITY_ERASE_UNIT_INDEX         4
#define TIMEOUT_SECURITY_FREEZE_LOCK_INDEX        5
#define TIMEOUT_SECURITY_DISABLE_PASSWORD_INDEX   6

#define DEFAULT_MASTER_PASSWORD_IDENTIFIER  0xFFFE

#define USER_PSW                        0
#define MASTER_PSW                      1

//
// The length "40" is defined from ATA idenfity table
//
#define DEVICE_MODEL_NAME_STRING_LENGTH  40
#define DEVICE_MODEL_NAME_STRING_SIZE    80

typedef struct _ITEM_INFO_IN_DIALOG {
  CHAR16                                  ItemInfoString[70];
} ITEM_INFO_IN_DIALOG;

typedef struct {
  UINT16                                HddSecurityStatus;
  CHAR16                                HddModelString[DEVICE_MODEL_NAME_STRING_LENGTH];
  UINT16                                MasterPasswordIdentifier;
  UINT8                                 ControllerMode;
  UINT16                                ControllerNumber;
  UINT16                                PortNumber;
  UINT16                                PortMulNumber;
  UINT16                                MappedPort;
  UINTN                                 PciSeg;
  UINTN                                 PciBus;
  UINTN                                 PciDevice;
  UINTN                                 PciFunction;
  EFI_HANDLE                            DeviceHandleInDxe;
  EFI_HANDLE                            DeviceHandleInSmm;
  UINT8                                 MaxPasswordLengthSupport;
  VOID                                  *HddInfoExtPtr;
} HDD_PASSWORD_HDD_INFO;

typedef struct {
  CHAR16                                DisableInputString[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                                 DisableStringLength;
  UINT8                                 DisableAllType;
  CHAR16                                DisableAllInputString[HDD_PASSWORD_MAX_NUMBER + 1];
  CHAR16                                MasterInputString[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                                 MasterFlag;
  UINTN                                 Flag;
  CHAR16                                InputString[HDD_PASSWORD_MAX_NUMBER + 1];
  UINTN                                 StringLength;
  HDD_PASSWORD_HDD_INFO                 *HddInfo;
  UINTN                                 NumOfEntry;
  UINTN                                 LabelIndex;
  VOID                                  *ScuDataExtPtr;
} HDD_PASSWORD_SCU_DATA;

typedef struct {
  UINT8                                 PasswordType;
  CHAR16                                PasswordStr[HDD_PASSWORD_MAX_NUMBER + 1];
} PASSWORD_INFORMATION;


//
//  String Token
//
#define STR_HDD_ESC_SKIP_MSG_INDEX        0
#define STR_HDD_TITLE_MSG_INDEX           1
#define STR_HDD_DIALOG_THREE_ERRORS_INDEX 2
#define STR_HDD_DIALOG_CONTINUE_MSG_INDEX 3
#define STR_HDD_DIALOG_ENTER_MSG_INDEX    4
#define STR_HDD_DIALOG_ERROR_STATUS_INDEX 5
#define STR_HDD_ENTER_MANY_ERRORS_INDEX   6
#define STR_SECURITY_COUNT_EXPIRED_INDEX  7
#define STR_HDD_DIALOG_COLDBOOT_MSG_INDEX 8
#define STR_HDD_DIALOG_HELP_TITLE_INDEX   9

typedef
EFI_STATUS
(EFIAPI *EFI_SET_HDD_PASSWROD) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINT8                             *PasswordBuffer,
  IN  UINTN                             PassLength
);

typedef
EFI_STATUS
(EFIAPI *EFI_UNLOCK_HDD_PASSWORD) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINT8                             *PasswordBuffer,
  IN  UINTN                             PassLength
);

typedef
EFI_STATUS
(EFIAPI *EFI_DISABLE_HDD_PASSWORD) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINT8                             *PasswordPtr,
  IN  UINTN                             PassLength
);

typedef
EFI_STATUS
(EFIAPI *EFI_HDD_IDENTIFY) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  OUT UINT16                        *IdentifyData
);

typedef
EFI_STATUS
(EFIAPI *EFI_GET_HDD_INFO) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  OUT HDD_PASSWORD_HDD_INFO             **HddInfoArray,
  IN  OUT UINTN                         *NumOfHdd
);

typedef
EFI_STATUS
(EFIAPI *EFI_SET_FEATURES_CMD) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  UINT8                             AtaFeatures,
  IN  UINT8                             AtaSectorCount
);

typedef
EFI_STATUS
(EFIAPI *EFI_UNLOCK_ALL_HDD) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This
);

typedef
EFI_STATUS
(EFIAPI *EFI_HDD_FREEZE) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo
);

typedef
EFI_STATUS
(EFIAPI *EFI_SEND_HDD_SECURITY_CMD) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN  UINT8                             CmdOpCode,
  IN  HDD_PASSWORD_HDD_INFO             *HddInfo,
  IN  BOOLEAN                           UserOrMaster,
  IN  UINT8                             *PasswordBuffer,
  IN  UINTN                             PasswordLength
  );

typedef
EFI_STATUS
(EFIAPI *EFI_RESET_SECURITY_STATUS) (
  IN EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfo
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PASSWORD_STRING_PROCESS) (
  IN  EFI_HDD_PASSWORD_SERVICE_PROTOCOL *This,
  IN UINT8                              PasswordType,
  IN VOID                               *RawPasswordPtr,
  IN UINTN                              RawPasswordLength,
  OUT VOID                              **PasswordToHdd,
  OUT UINTN                             *PasswordToHddLength
  );

typedef
EFI_STATUS
(EFIAPI *EFI_DISABLED_LOCKED_HDD) (
  IN EFI_HDD_PASSWORD_SERVICE_PROTOCOL  *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfo
  );

struct _EFI_HDD_PASSWORD_SERVICE_PROTOCOL {
  EFI_SET_HDD_PASSWROD                    SetHddPassword;
  EFI_UNLOCK_HDD_PASSWORD                 UnlockHddPassword;
  EFI_DISABLE_HDD_PASSWORD                DisableHddPassword;
  EFI_HDD_IDENTIFY                        HddIdentify;
  EFI_GET_HDD_INFO                        GetHddInfo;
  EFI_UNLOCK_ALL_HDD                      UnlockAllHdd;
  EFI_HDD_FREEZE                          HddFreeze;
  EFI_SEND_HDD_SECURITY_CMD               SendHddSecurityCmd;
  EFI_RESET_SECURITY_STATUS               ResetSecuirtyStatus;
  EFI_PASSWORD_STRING_PROCESS             PasswordStringProcess;
  EFI_DISABLED_LOCKED_HDD                 DisabledLockedHdd;
};

typedef
EFI_STATUS
(EFIAPI *EFI_GET_STRING_TOKEN_ARRAY) (
  IN  EFI_HDD_PASSWORD_DIALOG_PROTOCOL  *This,
  OUT CHAR16                            **StrTokenArray
  );

typedef
EFI_STATUS
(EFIAPI *EFI_CHECK_SKIP_DIALOG_KEY) (
  IN  EFI_HDD_PASSWORD_DIALOG_PROTOCOL  *This,
  IN  VOID                              *SkipDialogKey
  );

typedef
EFI_STATUS
(EFIAPI *EFI_RESET_ALL_SECURITY_STATUS) (
  IN EFI_HDD_PASSWORD_DIALOG_PROTOCOL   *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfoArray,
  IN UINTN                              NumOfHdd
  );

typedef
BOOLEAN
(EFIAPI *EFI_CHECK_HDD_LOCK) (
  IN EFI_HDD_PASSWORD_DIALOG_PROTOCOL   *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfoArray,
  IN UINTN                              NumOfHdd
  );

typedef
BOOLEAN
(EFIAPI *EFI_CHECK_HDD_SECURITY_ENABLE) (
  IN EFI_HDD_PASSWORD_DIALOG_PROTOCOL   *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfoArray,
  IN UINTN                              NumOfHdd
  );

typedef
EFI_STATUS
(EFIAPI *EFI_PREPARE_HDD_DIALOG_ITEMS) (
  IN EFI_HDD_PASSWORD_DIALOG_PROTOCOL   *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfoArray,
  IN UINTN                              NumOfHdd,
  OUT ITEM_INFO_IN_DIALOG               *HddDialogItemInfoString
  );

typedef
EFI_STATUS
(EFIAPI *EFI_GET_MODEL_NUMBER) (
  IN  EFI_HDD_PASSWORD_DIALOG_PROTOCOL  *This,
  IN VOID                               *DescBufferPtr,
  IN VOID                               *SourceBufferPtr
  );

typedef
EFI_STATUS
(EFIAPI *EFI_UNLOCK_SELECTED_HDD) (
  IN  EFI_HDD_PASSWORD_DIALOG_PROTOCOL  *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfo,
  OUT PASSWORD_INFORMATION              *PasswordInfo
  );

typedef
EFI_STATUS
(EFIAPI *EFI_SELECTED_HDD_PREPARE_TITLE_STRING) (
  IN EFI_HDD_PASSWORD_DIALOG_PROTOCOL   *This,
  IN HDD_PASSWORD_HDD_INFO              *HddInfo,
  OUT CHAR16                            **HddTitleString
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HDD_PASSWORD_UNLOCKED) (
  IN EFI_HDD_PASSWORD_DIALOG_PROTOCOL   *This
  );

typedef
BOOLEAN
(EFIAPI *EFI_IS_ADMI_SECURE_BOOT_RESET) (
  IN  EFI_HDD_PASSWORD_DIALOG_PROTOCOL  *This
  );

struct _EFI_HDD_PASSWORD_DIALOG_PROTOCOL {
  EFI_GET_STRING_TOKEN_ARRAY            GetStringTokenArray;
  EFI_CHECK_SKIP_DIALOG_KEY             CheckSkipDialogKey;
  EFI_RESET_ALL_SECURITY_STATUS         ResetAllSecuirtyStatus;
  EFI_CHECK_HDD_LOCK                    CheckHddLock;
  EFI_CHECK_HDD_SECURITY_ENABLE         CheckHddSecurityEnable;
  EFI_PREPARE_HDD_DIALOG_ITEMS          PrepareHddDialogItems;
  EFI_GET_MODEL_NUMBER                  GetModelNumber;
  EFI_UNLOCK_SELECTED_HDD               UnlockSelectedHdd;
  EFI_SELECTED_HDD_PREPARE_TITLE_STRING SelectedHddPrepareTitleString;
  EFI_IS_ADMI_SECURE_BOOT_RESET         IsAdmiSecureBootReset;
  EFI_HDD_PASSWORD_UNLOCKED             HddPasswordUnlocked;
};

extern EFI_GUID gEfiHddPasswordServiceProtocolGuid;
extern EFI_GUID gEfiHddPasswordDialogProtocolGuid;

#endif
