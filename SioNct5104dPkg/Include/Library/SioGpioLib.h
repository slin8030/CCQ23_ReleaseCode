/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SIOGPIO_LIBRARY_H_
#define _SIOGPIO_LIBRARY_H_

#include <PostCode.h>
#include "../../SioNct5104dDxe/SioHiiResource/SioHiiResourceNvData.h"
#include <SioNct5104dReg.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/SioLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PostCodeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <SioCommon.h>

#pragma pack (1)
typedef struct _SIO_GPIO_NUM {
  UINT8   Num     : 4;
  UINT8   Group   : 4;  
} SIO_GPIO_NUM;

typedef struct _SIO_GPIO_SETTING {
  UINT8   Pull    : 1;
  UINT8   TriState: 1;
  UINT8   Invert  : 1;
  UINT8   InOut   : 1;
  UINT8   PeiOut  : 1;
  UINT8   DxeOut  : 1;
  UINT8   Reserved: 2;
} SIO_GPIO_SETTING;

typedef struct _SIO_GPIO_CAPABILITY {
  UINT8  Pull    : 2;
  UINT8  TriState: 2;
  UINT8  IsInv   : 1;
  UINT8  InOut   : 2;
  UINT8  Reserved: 1;
} SIO_GPIO_CAPABILITY;

typedef struct _SIO_EXTENSIVE_TABLE_TYPE17 {
  UINT8               Type;
  UINT8               Length;
  UINT8               Enable;
  SIO_GPIO_NUM        GpioNum;
  SIO_GPIO_CAPABILITY Capability;
  SIO_GPIO_SETTING    Setting;
} SIO_EXTENSIVE_TABLE_TYPE17;

typedef struct _SIO_HANDLE_LIST {
  LIST_ENTRY          List;
  UINT8               GpHandle;
  EFI_QUESTION_ID     FeatureId;
} SIO_MAP_QUESTION_ID;
#pragma pack ()

EFI_STATUS
HandleListAdd (
  IN SIO_MAP_QUESTION_ID      *Container,
  IN UINT8                    GpHandle,
  IN EFI_QUESTION_ID          QuestionId
  );

EFI_STATUS
HandleListRemoveAll (
  IN SIO_MAP_QUESTION_ID      *Container
  );

UINT8
ConvertInst2Gp (
  IN  UINT8        Device
  );

EFI_STATUS
GpOutput (
  IN UINT8                         *GpNum,
  IN UINT8                         OutVal,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction 
  );

VOID
ProgramGpioLib (
  IN SIO_IDENTIFY_DEVICE              *DeviceMainType,
  IN SIO_DEVICE_LIST_TABLE            *TablePtr,
  IN OUT SIO_NCT5104D_CONFIGURATION   *SioConfiguration,
  IN BOOLEAN                          SioVariableExist,
  IN EFI_SIO_RESOURCE_FUNCTION        *SioResourceFunction
  );

#endif
