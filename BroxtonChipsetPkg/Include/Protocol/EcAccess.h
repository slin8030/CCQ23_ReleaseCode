/** @file

@copyright
 Copyright (c) 2009 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef __EFI_EC_ACCESS_H___
#define __EFI_EC_ACCESS_H___

//typedef struct gEfiEcAccessProtocolGuid _gEfiEcAccessProtocolGuid;
//
// EC Access specification constant and types
//


//
// EC Access specification Data Structures
//


typedef
EFI_STATUS
(EFIAPI *EC_QUERY_COMMAND) (
	UINT8 	*Qdata
);

typedef
EFI_STATUS
(EFIAPI *EC_WRITE_CMD) (
	UINT8    cmd
);

typedef
EFI_STATUS
(EFIAPI *EC_WRITE_DATA) (
	UINT8    data
);

typedef
EFI_STATUS
(EFIAPI *EC_READ_DATA) (
	UINT8 	*data
);

typedef
EFI_STATUS
(EFIAPI *EC_READ_MEM) (
	UINT8	Index,
	UINT8	*Data
);

typedef
EFI_STATUS
(EFIAPI *EC_WRITE_MEM) (
	UINT8	Index,
	UINT8	Data
);

typedef
EFI_STATUS
(EFIAPI *EC_ACPI_ENABLE) (
);


typedef
EFI_STATUS
(EFIAPI *EC_ACPI_DISABLE) (
);

typedef
EFI_STATUS
(EFIAPI *EC_SMI_NOTIFY_ENABLE) (
);

typedef
EFI_STATUS
(EFIAPI *EC_SMI_NOTIFY_DISABLE) (
);


typedef
EFI_STATUS	(EFIAPI *EC_SHUTDOWN_SYSTEM)(
);   			

typedef
EFI_STATUS  (EFIAPI *EC_GET_MOTHER_BOARD_ID)(
UINT8		*FabID
);

typedef
EFI_STATUS  (EFIAPI *EC_GET_EC_VERSION)(
UINT8		*Revision
);

typedef
EFI_STATUS	(EFIAPI *EC_ENABLE_LAN)(
);

typedef
EFI_STATUS  (EFIAPI *EC_DISABLE_LAN)(
);

typedef
EFI_STATUS  (EFIAPI *EC_DEEPSX_CONFIG)(
UINT8    ECData
);

typedef
EFI_STATUS  (EFIAPI *EC_TURBOCTRL_TESTMODE)(
UINT8    Enable,
UINT8    ACBrickCap,
UINT8    ECPollPeriod,
UINT8    ECGuardBandValue,
UINT8    ECAlgorithmSel
);

typedef 
EFI_STATUS (EFIAPI *EC_GET_SWITCH_STATUS)(
  UINT8		*Data
);

typedef
EFI_STATUS  (EFIAPI *EC_FAIL_SAFE_FAN_CTRL)(
UINT8    CpuTemp,
UINT8    CpuFanSpeed
);
typedef struct _EFI_EC_ACCESS_PROTOCOL {
  EFI_HANDLE            		Handle;
  EC_QUERY_COMMAND      		QuerryCmd;
  EC_WRITE_CMD          		WriteCmd;
  EC_WRITE_DATA         		WriteData;
  EC_READ_DATA					ReadData;
  EC_READ_MEM					ReadMem;
  EC_WRITE_MEM					WriteMem;
  EC_ACPI_ENABLE				AcpiEnable; 
  EC_ACPI_DISABLE				AcpiDisable;
  EC_SMI_NOTIFY_ENABLE			SMINotifyEnable;
  EC_SMI_NOTIFY_DISABLE 		SMINotifyDisable;
  EC_SHUTDOWN_SYSTEM   			ShutDownSystem;
  EC_GET_MOTHER_BOARD_ID		GetMotherBoardID;
  EC_GET_EC_VERSION				GetECVersion;
  EC_ENABLE_LAN					EnableLan;
  EC_DISABLE_LAN				DisableLan;
  EC_DEEPSX_CONFIG				DeepSxConfig; 
  EC_TURBOCTRL_TESTMODE			TurboCtrlMode; 
  EC_GET_SWITCH_STATUS			GetSwitchStatus;
  EC_FAIL_SAFE_FAN_CTRL     FailSafeFanCtrl;
} EFI_EC_ACCESS_PROTOCOL;

extern EFI_GUID gEfiEcAccessProtocolGuid;

#endif





