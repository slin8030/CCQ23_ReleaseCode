/** @file

Device Protocol

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
#include <Protocol/IsaNonPnpDevice.h>

static UINT8  Ldn;  // Logical device number
static EFI_WDT_INFO *mWdtInfo;

extern EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[];

extern VOID
EnterConfigMode (
  VOID
  );

extern VOID
ExitConfigMode (
  VOID
  );         

EFI_STATUS
WriteOnOff (
  BOOLEAN           OnOff
  )
{
  EnterConfigMode ();
  
  IDW8 (SIO_LDN , Ldn, mSioResourceFunction);
  IDW8 (SIO_DEV_ACTIVE , OnOff, mSioResourceFunction);

  ExitConfigMode ();  

  return EFI_SUCCESS;
}

EFI_STATUS
WriteMode (
  UINT8             Mode
  )
{
  UINT8     Value;
  UINT8     Bit3;

  Bit3 = 0x08;
  
  EnterConfigMode ();

  IDW8 (SIO_LDN , Ldn, mSioResourceFunction);
  Value = IDR8 (SIO_WDT_CONTROL_MODE_REG, mSioResourceFunction);

  if (Mode == 0x00) {
    IDW8 (SIO_WDT_CONTROL_MODE_REG, Value & ~Bit3, mSioResourceFunction);
  } else {
    IDW8 (SIO_WDT_CONTROL_MODE_REG, Value | Bit3, mSioResourceFunction);
  }

  ExitConfigMode ();

  return EFI_SUCCESS;
}


EFI_STATUS
WriteCounter (
  UINT16            Counter
  )
{
  EnterConfigMode ();

  IDW8 (SIO_LDN , Ldn, mSioResourceFunction);
  IDW8 (SIO_WDT_COUNTER_REG, (UINT8)(Counter & 0xFF), mSioResourceFunction);

  ExitConfigMode ();

  return EFI_SUCCESS;  
}

EFI_STATUS
WriteReg (
  UINT8             Offset,
  UINT8             Reg  
  )
{
  EnterConfigMode (); 

  IDW8 (Offset , Reg, mSioResourceFunction);
  
  ExitConfigMode (); 
  
  return EFI_SUCCESS;
}

EFI_STATUS
ReadData (
  EFI_WDT_INFO      **Info
  )
{
  *Info = mWdtInfo;

  return EFI_SUCCESS;
}

VOID
WdtInitFunction (
  )
{

}

STATIC EFI_SIO_WDT_PROTOCOL mSioWdtProtocol = {
  WriteOnOff,
  WriteMode,  
  WriteCounter,  
  WriteReg, 
  ReadData
};

/**
  brief-description of function. 

  extended description of function.  
  
**/
EFI_STATUS
InstallWdtProtocol (
  IN SIO_DEVICE_LIST_TABLE* DeviceList
  )
{
  EFI_STATUS             Status;
  EFI_HANDLE             DeviceHandle;
  EFI_SIO_WDT_PROTOCOL   WdtProtocol;

  //
  // Register ID for this device only
  //
  Ldn = DeviceList->DeviceLdn;

  DeviceHandle=NULL;
  Status = gBS->InstallProtocolInterface (
                  &DeviceHandle,
                  &gEfiSioWdtProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &WdtProtocol
                );

  WdtInitFunction();

  return Status;
}