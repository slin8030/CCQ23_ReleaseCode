/** @file
  CrVariableUpdate Header

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

#ifndef _CR_VARIABLE_UPDATE_H_
#define _CR_VARIABLE_UPDATE_H_

#include "CrServiceDxe.h"

extern CHAR16                *mConInVarName;
extern CHAR16                *mConOutVarName;
extern EFI_GUID              *mConVarGuid;

#define  CR_ACTIVE_VARIABLE_NAME   L"CrActiveDevice"

VOID
CreateCrDeviceVarList (
  VOID
  );

VOID
FreeCrDevicesVarList (
  VOID
  );

VOID
CheckAndSaveConnectResult (
  IN  CHAR16                    *ConVarName,
  IN  EFI_GUID                  *ConVarGuid
  );

VOID
CreateNewActiveCrDevice (
  VOID
  );

VOID
FindAsteriskDevicePath (
  VOID
  );

VOID
InitializeCrDeviceList (
  VOID
  );

VOID
UpdateDevicesExistStatus (
  VOID
  );

VOID
UpdateConInConOutVarFromActiveDevicePath (
  VOID
  );

EFI_STATUS
UpdateConsoleVariable (
  IN  CHAR16                    *ConVarName,
  IN  EFI_GUID                  *ConVarGuid,
  IN  EFI_DEVICE_PATH_PROTOCOL  *CustomizedConDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL  *ExclusiveDevicePath
  );

BOOLEAN
IsInclusiveOfSelectDevPath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevPathPool,
  IN  EFI_DEVICE_PATH_PROTOCOL  *SelectDevPath
  );

BOOLEAN
IsEqualDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath1,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePath2
  );

#endif

