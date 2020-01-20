/** @file

According to Pcd$(SIO_Name)Dmi to program extensive function devuce

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

#include "SioInitDxe.h"

extern BOOLEAN                      mFirstBoot;
extern EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[];

/**
  To program extend function for LPT device. 
**/
VOID
ProgramLpt (
  IN SIO_EXTENSIVE_TABLE_MAINTYPE  *DeviceMainType,
  IN SIO_DEVICE_LIST_TABLE         *TablePtr,
  IN OUT SIO_CONFIGURATION         *SioConfiguration
  ) 
{
  UINT8                       Value;
  UINT8                      *LptPcdDmiPtr;
  SIO_EXTENSIVE_TABLE_TYPE2  *DeviceModeSetting;

  Value             = 0;
  LptPcdDmiPtr      = (UINT8*)DeviceMainType + DeviceMainType->Length;
  DeviceModeSetting = NULL;

  while ((*LptPcdDmiPtr != 0x00) && (*LptPcdDmiPtr != 0xFF)) {
    if ((*LptPcdDmiPtr == SUBTYPE2) && (*(LptPcdDmiPtr + 2) == TRUE)) {
      DeviceModeSetting = (SIO_EXTENSIVE_TABLE_TYPE2*)LptPcdDmiPtr;
      IDW8 (SIO_LDN, TablePtr->DeviceLdn, mSioResourceFunction);
      Value = (IDR8 (SIO_CRF0, mSioResourceFunction) & 0xFC);
      switch (DeviceModeSetting->Setting) {
      case 0: // SPP mode
        Value |= 0x00;
        break;
      case 1: // EPP mode
        Value |= 0x01;
        break;
      case 2: // ECP mode
        Value |= 0x02;
        break;
      case 3: // ECP & EPP mode
        Value |= 0x03;
        break;
      default:
        break;
      }
      IDW8 (SIO_CRF0, Value, mSioResourceFunction);
    }
    LptPcdDmiPtr += *(LptPcdDmiPtr + 1);
  }
}

/**
  To program extend function for GPIO device. 
**/
VOID
ProgramGpio (
  IN SIO_EXTENSIVE_TABLE_MAINTYPE  *DeviceMainType,
  IN SIO_DEVICE_LIST_TABLE         *TablePtr,
  IN OUT SIO_CONFIGURATION         *SioConfiguration
  ) 
{

}

/**
  To program extend function for COM device. 
**/
VOID
ProgramCom (
  IN SIO_EXTENSIVE_TABLE_MAINTYPE  *DeviceMainType,
  IN SIO_DEVICE_LIST_TABLE         *TablePtr,
  IN OUT SIO_CONFIGURATION         *SioConfiguration
  ) 
{
  UINT8                      Value;                 
  UINT8                      *ComPcdDmiPtr;
  UINT8                      *ComTypeVariable;
  SIO_EXTENSIVE_TABLE_TYPE2  *DeviceModeSetting;

  Value             = 0;
  ComPcdDmiPtr      = (UINT8*)DeviceMainType + DeviceMainType->Length;
  ComTypeVariable   = &SioConfiguration->Com1Type;
  DeviceModeSetting = NULL;

  while ((*ComPcdDmiPtr != 0x00) && (*ComPcdDmiPtr != 0xFF)) {
    if ((*ComPcdDmiPtr == SUBTYPE2) && (*(ComPcdDmiPtr + 2) == TRUE)) {

      IDW8 (SIO_LDN, TablePtr->DeviceLdn, mSioResourceFunction);
      Value = (IDR8 (SIO_CRF0, mSioResourceFunction) & 0xEF);

      if (mFirstBoot) {
        DeviceModeSetting = (SIO_EXTENSIVE_TABLE_TYPE2*)ComPcdDmiPtr;
        switch (DeviceModeSetting->Setting) {
        case 0:  // RS232 mode
          Value |= 0x10;
          break;
        case 1:  // RS485 mode
          Value |= 0x00;
          break;
        }

        //
        // SCU supported, SIO driver need to update variable when platform first boot.
        //
        if (FeaturePcdGet (PcdSioWpcd374lSetup)) {
          *(ComTypeVariable + DeviceMainType->DeviceInstance) = DeviceModeSetting->Setting;
        }
      } else {
        switch (*(ComTypeVariable + DeviceMainType->DeviceInstance)) {
        case 0:  // RS232 mode
          Value |= 0x10;
          break;
        case 1:  // RS485 mode
          Value |= 0x00;
          break;
        }
      }
      IDW8 (SIO_CRF0, Value, mSioResourceFunction);
    }
    ComPcdDmiPtr += *(ComPcdDmiPtr + 1);
  }
}

