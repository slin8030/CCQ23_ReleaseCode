/** @file
  The header file of CrDeviceVariableLib.c

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

#ifndef _CR_DEVICE_VARIABLE_H_
#define _CR_DEVICE_VARIABLE_H_

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/VariableLib.h>
#include <Library/CrDeviceVariableLib.h>
#include <Library/SelfDebugLib.h>

#include <Guid/CrConfigHii.h>


#define CR_DEVICE_INFO_HEADER(_Exist, _Global, _Enable ) \
                       ((UINT8)(  (((_Enable) & 0x01) << 2) |  \
                                  (((_Global) & 0x01) << 1) |  \
                                   ((_Exist)  & 0x01)  ))

#define NULL_GUID \
  { \
    0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 \
  }


#define FixedPcdGetPtrSize(TokenName)      (UINTN)_PCD_PATCHABLE_##TokenName##_SIZE

extern EFI_GUID gCrConfigurationGuid;

//
// === Private Struct ===
//
#pragma pack(1)

typedef struct _CR_DEVICE_PCD {
  UINT8                 PortEnable;
  CONST CHAR16          *DevName;
  EFI_DEVICE_PATH       *DevPath;
  //
  // other infomation not in PCDs
  // only for parsing PCD
  //
  BOOLEAN               Exist;
  UINT8                 ItemType;
  BOOLEAN               UseGlobal;
} CR_DEVICE_PCD;

typedef struct _CR_DEVICE_VAR {
  UINT8               Exist      :1;
  UINT8               UseGlobal  :1;
  UINT8               PortEnable :1;
  UINT8               SetInPCD   :1;
  UINT8               Reserve    :4;
  UINT8               ItmeType;
  CHAR16              DevName;   //Dynamic Length
  EFI_DEVICE_PATH     DevPath;   //Dynamic Length
} CR_DEVICE_VAR;

#pragma pack()

typedef enum _DPATH_UPDATE_TYPE {
  UPDATE_NONE,
  UPDATE_ADD,
  UPDATE_DELETE
} DPATH_UPDATE;


CHAR16*
CrDevVariableName (
  IN UINTN Index
  );

EFI_STATUS
CreateDeviceVariableFromPcd (
  IN  CR_DEVICE_PCD *CrDevice,
  IN  UINTN         DeviceCount,
  IN  BOOLEAN       LoadDefault
  );

UINTN
CalculateCrDeviceVarSize (
  IN CONST CHAR16           *NameStr,
  IN       EFI_DEVICE_PATH  *DPath,
  OUT      UINTN            *DpathOffse
  );

EFI_STATUS
  CalculateNewVariableSize (
  IN OUT CR_DEVICE_VAR     **CrDeviceVar,
  IN OUT UINTN             *VarSize,
  IN     UINT8             NewFlowcontrol,
  OUT    DPATH_UPDATE      *UpdateType
  );

EFI_STATUS
FreeDevicePathPool (
  IN  CR_DEVICE_PCD *CrDevice,
  IN  UINTN         DeviceCount
  );

VOID*
GetNextColumnPtr (
  IN CONST CHAR16*  StrPtr
  );

EFI_DEVICE_PATH*
  GetDevicePathFromVariable (
  CR_DEVICE_VAR        *CrDeviceVar
  );

CHAR16*
Int2Str (
  IN  UINTN   Number,
  IN  CHAR16  *Str
  );

EFI_STATUS
ParsingCrDevicePcdValue (
  OUT CR_DEVICE_PCD **CrDevice,
  OUT UINTN         *DeviceCount
  );

EFI_STATUS
ParsingDevicePath2Attribute (
  IN  EFI_DEVICE_PATH         *DevPath,
  OUT CR_DEVICE_ATTRIBUTES    *Attribute
  );

EFI_STATUS
ParsingAttribute2DevicePath (
  IN OUT EFI_DEVICE_PATH      *DevPath,
  IN     CR_DEVICE_ATTRIBUTES *Attribute,
  IN     DPATH_UPDATE         UpdateType
  );

VOID*
PrepareEnoughCrDeviceVariable (
  IN     VOID     *CrDeviceVar,
  IN     UINTN    NewSize,
  IN OUT UINTN    *BufferSize
  );

VOID
UpdateTemplateDeviceNode (
  VOID
  );

EFI_DEVICE_PATH*
  VerifyMediaFileNode (
  IN  EFI_DEVICE_PATH *FullDevPath,
  OUT CR_DEVICE_PCD   *PcdDevice
  );

EFI_DEVICE_PATH*
  VerifyUartNode (
  IN  EFI_DEVICE_PATH *FullDevPath,
  OUT CR_DEVICE_PCD   *PcdDevice
  );

EFI_DEVICE_PATH*
  VerifyMessagingDeviceNode (
  IN  EFI_DEVICE_PATH *FullDevPath,
  OUT CR_DEVICE_PCD   *PcdDevice
  );

BOOLEAN
CheckCrDeviceVariableExist (
  IN CHAR16           *VarName
  );

EFI_DEVICE_PATH_PROTOCOL  *
AppendMultiDevicePathNode (
  EFI_DEVICE_PATH_PROTOCOL  *SrcDevicePath,
  UINTN                     NumOfArgs,
  ...
  );

EFI_DEVICE_PATH_PROTOCOL *
LocateDevicePathNode(
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath,
  IN UINT8                          DevPathType,
  IN UINT8                          DevPathSubType
  );

#endif

