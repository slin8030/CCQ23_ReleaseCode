/** @file
  SIO Library.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SioGpioLib.h>

EFI_STATUS
HandleListAdd (
  IN SIO_MAP_QUESTION_ID      *Container,
  IN UINT8                    GpHandle,
  IN EFI_QUESTION_ID          QuestionId
  )
{
  SIO_MAP_QUESTION_ID      *Item;

  Item = AllocatePool (sizeof (SIO_MAP_QUESTION_ID));
  InsertTailList((LIST_ENTRY *)Container, (LIST_ENTRY *)Item);

  Item->GpHandle = GpHandle;
  Item->FeatureId = QuestionId;

  return EFI_SUCCESS;
}

EFI_STATUS
HandleListRemoveAll (
  IN SIO_MAP_QUESTION_ID       *Container
  )
{
  SIO_MAP_QUESTION_ID     *Item;

  while (!IsListEmpty ((LIST_ENTRY *)Container)) {
    Item = (SIO_MAP_QUESTION_ID *)GetFirstNode ((LIST_ENTRY *)Container);
    RemoveEntryList((LIST_ENTRY *)Item);
    FreePool (Item);
  }  
  return EFI_SUCCESS;
}

UINT8
ConvertInst2Gp (
  IN  UINT8        DeviceInstance
  )
{
  UINT8               GpNumber;
  GpNumber            = 0;
 
  GpNumber = (DeviceInstance / 8) * 0x10  + (DeviceInstance % 8);

  return GpNumber;
}

EFI_STATUS
SetNative (
  IN UINT8                         *GpNum,
  IN UINT8                         Native,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction 
  )
{
  EFI_STATUS        Status;
  SIO_GPIO_NUM      *GpioNum;
  UINT8             Value, Num, RegIndex, Mask;
  GpioNum           = (SIO_GPIO_NUM *)GpNum;
  Status            = EFI_SUCCESS;            
  Value             = 0;
  Num               = 0;
  RegIndex          = 0;
  Mask              = 0;

  //
  // Muti-Function
  //
  switch (GpioNum->Group) {
  case 0:
    RegIndex = SIO_CR1C;
    Mask = 0x84;
    Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
    Value |= (Native << 3);
    IDW8 (RegIndex, Value, SioResourceFunction);
    break;
  case 1:
    RegIndex = SIO_CR1C;
    Mask = 0x88;
    Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
    Value |= (Native << 2);
    IDW8 (RegIndex, Value, SioResourceFunction);
    break;
  case 6:
    Num = GpioNum->Num;
    switch (Num) {
    case 7:
      RegIndex = SIO_CR1C;
      Mask = 0x0C;
      Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
      Value |= (!Native << 4);
      IDW8 (RegIndex, Value, SioResourceFunction);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }

  return Status;
}

EFI_STATUS
GpOutput (
  IN UINT8                         *GpNum,
  IN UINT8                         OutVal,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction 
  )
{
  EFI_STATUS           Status;
  SIO_GPIO_NUM         *GpioNum;
  UINT8                Value, Num, RegIndex, Mask;
  GpioNum              = (SIO_GPIO_NUM *)GpNum;
  Status               = EFI_SUCCESS;            
  Value                = 0;
  Num                  = 0;
  RegIndex             = 0;
  Mask                 = 0;
  
  //
  // LDN 7 
  //
  IDW8 (SIO_LDN, SIO_GPIO_SET0, SioResourceFunction);
  RegIndex = SIO_CRE1 + GpioNum->Group * 4;
  Mask = ~(1 << GpioNum->Num);
  //
  // Output Value
  // 
  Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
  Value |= ((OutVal & 1) << GpioNum->Num);
  IDW8 (RegIndex, Value, SioResourceFunction);

  return Status;
}


EFI_STATUS
SetGpio (
  IN UINT8                         *GpNum,
  IN UINT8                         *Setting,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction        
  )
{
  EFI_STATUS        Status;
  UINT8             Value, Num, RegIndex, Mask;
  SIO_GPIO_NUM      *GpioNum;
  SIO_GPIO_SETTING  *GpioSetting;
  Status            = EFI_SUCCESS;
  GpioNum           = (SIO_GPIO_NUM *)GpNum;
  GpioSetting       = (SIO_GPIO_SETTING *)Setting;
  Value             = 0;
  Num               = 0;
  RegIndex          = 0;
  Mask              = 0;

  //
  // LDN 7 
  //
  IDW8 (SIO_LDN, SIO_GPIO_SET0, SioResourceFunction);

  //
  // Active GPIO register
  //
  RegIndex = SIO_CR30;
  Mask = ~(1 << GpioNum->Group);
  Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
  Value |= (1 << GpioNum->Group);
  IDW8 (RegIndex, Value, SioResourceFunction);
  
  RegIndex = SIO_CRE0 + GpioNum->Group * 4;
  Mask = ~(1 << GpioNum->Num);
  //
  // Input Output
  // 
  Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
  Value |= ((!GpioSetting->InOut) << GpioNum->Num);
  IDW8 (RegIndex, Value, SioResourceFunction);
  //
  // Invert
  //
  Value = IDR8 (RegIndex + 2, SioResourceFunction) & Mask;
  Value |= (GpioSetting->Invert << GpioNum->Num);
  IDW8 (RegIndex + 2, Value, SioResourceFunction);

  //
  // Native Fuction
  //
  // SetNative (GpNum, 0, SioResourceFunction);

  //
  // LDN F
  //
  IDW8 (SIO_LDN, SIO_GPIO_PUSH, SioResourceFunction);
  RegIndex = SIO_CRE0 + GpioNum->Group;
  Mask = ~(1 << GpioNum->Num);
  //
  // Resistance (0: Push pull , 1:Open Drain)
  //
  Value = IDR8 (RegIndex, SioResourceFunction) & Mask;
  Value |= (GpioSetting->TriState << GpioNum->Num);
  IDW8 (RegIndex, Value, SioResourceFunction);

  if (GpioSetting->InOut) {
    GpOutput (GpNum, (UINT8)GpioSetting->PeiOut, SioResourceFunction);
  }
  
  return Status;
} 

VOID
ProgramGpioLib (
  IN SIO_IDENTIFY_DEVICE              *DeviceMainType,
  IN SIO_DEVICE_LIST_TABLE            *TablePtr,
  IN OUT SIO_NCT5104D_CONFIGURATION   *SioConfiguration,
  IN BOOLEAN                          SioVariableExist,
  IN EFI_SIO_RESOURCE_FUNCTION        *SioResourceFunction
  ) 
{
  UINT8                       *GpioPcdExtensiveCfgPtr;
  SIO_EXTENSIVE_TABLE_TYPE17  *DeviceModeSetting;

  if (DeviceMainType == NULL) {
    return;
  }
  GpioPcdExtensiveCfgPtr     = (UINT8*)DeviceMainType + DeviceMainType->Length;
  DeviceModeSetting = NULL;

  if ((*GpioPcdExtensiveCfgPtr == TYPE17) && (*(GpioPcdExtensiveCfgPtr + 2) == TRUE)) {
    DeviceModeSetting = (SIO_EXTENSIVE_TABLE_TYPE17*)GpioPcdExtensiveCfgPtr;
    if (!SioVariableExist) {  
      SioConfiguration->Gpio[DeviceMainType->DeviceInstance] = *((UINT8*)&(DeviceModeSetting->Setting)); 
      SetGpio ((UINT8*)&(DeviceModeSetting->GpioNum), (UINT8*)&(DeviceModeSetting->Setting), SioResourceFunction);
    } else {
      SetGpio ((UINT8*)&(DeviceModeSetting->GpioNum), (UINT8*)&(SioConfiguration->Gpio[DeviceMainType->DeviceInstance]), SioResourceFunction);
    }
  }
}
