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
#include <Library/GpioLib.h>

extern EFI_INSTALL_DEVICE_FUNCTION mDeviceFunction [];
extern EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[];

/**
  To program extend function for LPT device.
**/
VOID
ProgramLpt (
  IN UINT8                 *PcdDmiPointer,
  IN SIO_DEVICE_LIST_TABLE *TablePtr
  )
{

}

/**
  To program extend function for GPIO device.
**/
VOID
ProgramGpio (
  IN UINT8                 *PcdDmiPointer,
  IN SIO_DEVICE_LIST_TABLE *TablePtr
  )
{

}

/**
  To program extend function for COM device.
**/
VOID
ProgramCom (
  IN UINT8                 *PcdDmiPointer,
  IN SIO_DEVICE_LIST_TABLE *TablePtr
  )
{
  EFI_INSTALL_DEVICE_FUNCTION *DeviceFunctionPtr;
  UINT8  *MainTypePtr;
  UINT8  Mode;
  UINT8  Value;
  UINT8  Ldn;

  Mode = 0;
  Value = 0;
  Ldn = TablePtr->DeviceLdn;

  DeviceFunctionPtr = mDeviceFunction;
  MainTypePtr = PcdDmiPointer;
  PcdDmiPointer += *(PcdDmiPointer + 1);

  while ((*PcdDmiPointer != 0x00) && (*PcdDmiPointer != 0xFF)) {
    if ((*PcdDmiPointer == SUBTYPE2) && (*(PcdDmiPointer + 2) == TRUE)) {
      Mode = *(PcdDmiPointer + 3);
      IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
      Value = IDR8 (SIO_CRF2, mSioResourceFunction);
      Value = Value & 0xFD;

      switch (Mode) {
      case 0: // RS232 mode
        Value |= 0x00;
        break;
      case 1: // RS485 mode
        Value |= 0x02;
        break;
      default:
        break;
      }
      IDW8 (SIO_CRF2, Value, mSioResourceFunction);
    }
    PcdDmiPointer += *(PcdDmiPointer + 1);
  }

  return;
}

