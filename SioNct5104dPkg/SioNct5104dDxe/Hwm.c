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
**/
#include "SioInitDxe.h"
#include <Protocol/IsaNonPnpDevice.h>
#include "SetupConfig.h"
#include <Protocol/CpuIo2.h>
#include <OemEcLib.h>
#include <CompalEcLib.h>
#include <ProjectDefinition.h>
extern VOID
EnterConfigMode (
  VOID
  );

extern VOID
ExitConfigMode (
  VOID
  );
//[-start-150414-IB12691000-add]//
extern EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[];
//[-end-150414-IB12691000-add]//
UINT8 Ldn;
UINT16 mHwmPort;
UINT16 mHwmIndexPort;
UINT16 mHwmDataPort;

//
//EFI_HARDWARE_MONITOR_INFO2[] => Msb, Lsb, Data, Label, Promp, Help, Offset
//
EFI_HARDWARE_MONITOR_INFO2 mVoltageData[] = {
  { 0, 0, 0, 0, HARDWARE_MONITOR_VCORE, STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_VOL1_HWM_OFFSET},
// { 0, 5, 0, 0, HARDWARE_MONITOR_VCC,   STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_VOL2_HWM_OFFSET},
  { 0, 4, 0, 0, HARDWARE_MONITOR_VIN0,  STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_VOL3_HWM_OFFSET},
//  { 0, 1, 0, 0, HARDWARE_MONITOR_VIN1,  STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_VOL4_HWM_OFFSET},
};

EFI_HARDWARE_MONITOR_INFO2 mTemperatureData[] = {
   { 0, 0x10, 0, 0, HARDWARE_MONITOR_SYS_TEMP1, STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_TEMP1_HWM_OFFSET},
//   { 0, 0x11, 0, 0, HARDWARE_MONITOR_CPU_TEMP, STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_TEMP2_HWM_OFFSET},
 };

EFI_HARDWARE_MONITOR_INFO2 mFanSpeedData[] = {
   { 0x30, 0x31, 0, 0, HARDWARE_MONITOR_SYS_FAN, STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_FAN1_HWM_OFFSET},
//   { 0x32, 0x33, 0, 0, HARDWARE_MONITOR_CPU_FAN, STRING_TOKEN (STR_SIO_EMPTY), SIO_HWM_OFFSET + SIO_FAN2_HWM_OFFSET},
 };

VOID
HwmInitFunction (
  VOID
  );

UINT8
HMIndexReading (
  IN UINT8  Index
  );

VOID
HMIndexWriting (
  IN UINT8  Index,
  IN UINT8  Data8
  );

EFI_STATUS
GetVoltageData2 (
  OUT EFI_HARDWARE_MONITOR_INFO2 **VoltageInfo,
  OUT INTN                       *Counter
  );

EFI_STATUS
GetVoltageStatus (
  IN  UINTN  Register,
  OUT UINTN  *Status
  );

EFI_STATUS
SetVoltageCtrl (
  IN UINTN  Register,
  IN UINTN  Data
  );

EFI_STATUS
GetFanSpeedData2 (
  OUT EFI_HARDWARE_MONITOR_INFO2 **VoltageInfo,
  OUT INTN                       *Counter
  );

EFI_STATUS
GetFanSpeedStatus (
  IN  UINTN  Register,
  OUT UINTN  *Status
  );

EFI_STATUS
SetFanSpeedCtrl (
  IN UINTN  Register,
  IN UINTN  Data
  );


EFI_STATUS
GetTemperatureData2 (
  OUT EFI_HARDWARE_MONITOR_INFO2 **VoltageInfo,
  OUT INTN                       *Counter
  );

EFI_STATUS
GetTemperatureStatus (
  IN  UINTN  Register,
  OUT UINTN  *Status
  );

EFI_STATUS
SetTemperatureCtrl (
  IN UINTN  Register,
  IN UINTN  Data
  );

STATIC EFI_HARDWARE_MONITOR_PROTOCOL2 mSioVoltageProtocol = {
  GetVoltageData2,
  GetVoltageStatus,
  SetVoltageCtrl
};

STATIC EFI_HARDWARE_MONITOR_PROTOCOL2 mSioFanSpeedProtocol = {
  GetFanSpeedData2,
  GetFanSpeedStatus,
  SetFanSpeedCtrl
};

STATIC EFI_HARDWARE_MONITOR_PROTOCOL2 mSioTemperatureProtocol = {
  GetTemperatureData2,
  GetTemperatureStatus,
  SetTemperatureCtrl
};

/**
  brief-description of function.

  extended description of function.

**/
EFI_STATUS
InstallHwmProtocol (
  IN SIO_DEVICE_LIST_TABLE* DeviceList
  )
{
  EFI_STATUS       Status;
  EFI_HANDLE       DeviceHandle;

  //
  // Get resources from PCD data
  //
  Ldn = DeviceList->DeviceLdn;
  mHwmPort = UpsideDown (DeviceList->DeviceBaseAdr);

  DeviceHandle=NULL;
  Status = gBS->InstallProtocolInterface (
                  &DeviceHandle,
                  &gEfiIsaVoltagleDeviceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSioVoltageProtocol
                );

  DeviceHandle=NULL;
  Status = gBS->InstallProtocolInterface (
                  &DeviceHandle,
                  &gEfiIsaFanSpeedDeviceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSioFanSpeedProtocol
                );

  DeviceHandle=NULL;
  Status = gBS->InstallProtocolInterface (
                  &DeviceHandle,
                  &gEfiIsaTemperatureDeviceProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSioTemperatureProtocol
                );

  HwmInitFunction();

  return Status;
}

VOID
HMIndexWriting (
  IN UINT8    Index,
  IN UINT8    Data8
  )
{
  SioWrite8 (mHwmIndexPort, Index, mSioResourceFunction->ScriptFlag);
  SioWrite8 (mHwmDataPort, Data8, mSioResourceFunction->ScriptFlag);
}

UINT8
HMIndexReading (
  IN UINT8 Index
  )
{
  UINT8      Data;

  SioWrite8 (mHwmIndexPort, Index, mSioResourceFunction->ScriptFlag);
  Data = SioRead8 (mHwmDataPort);

  return Data;
}

VOID
HwmInitFunction (
  VOID
  )
{
  UINT8              BankNumber = 0;
  UINT8              HwmRegister = 0;
  EFI_STATUS         Status;
  UINTN              BufferSize;
  SYSTEM_CONFIGURATION *SetupConfig;


  BufferSize = sizeof (SYSTEM_CONFIGURATION);
  SetupConfig = AllocateZeroPool (sizeof (SYSTEM_CONFIGURATION));
  Status = gRT->GetVariable(
                  L"Setup",
                  &gSystemConfigurationGuid,
                  NULL,
                  &BufferSize,
                  SetupConfig
                  );

  IDW8(SIO_LDN, Ldn, mSioResourceFunction);

  IDW8(SIO_BASE_IO_ADDR1_MSB, (UINT8) (mHwmPort >> 8), mSioResourceFunction);
  IDW8(SIO_BASE_IO_ADDR1_LSB, (UINT8) (mHwmPort & 0xFF), mSioResourceFunction);
  IDW8(SIO_DEV_ACTIVE, 1, mSioResourceFunction);

  mHwmIndexPort = mHwmPort + 5;
  mHwmDataPort  = mHwmIndexPort + 1;

  //
  //  Init Bank 0
  //
  BankNumber = HMIndexReading (0x4E);
  BankNumber &= 0xFC;
  HMIndexWriting (0x4E, BankNumber);
  HMIndexWriting (0x1B, 0x04);
  HMIndexWriting (0xB3, 0x0C);
  HMIndexWriting (0xB4, 0x0D);
  if (!EFI_ERROR (Status) && SetupConfig->CaseOpen == 0) {
    HwmRegister = HMIndexReading (0x86);
    HwmRegister |= 0x04;
    HMIndexWriting (0x86, HwmRegister);
//[-start-140715-MH4368-add]//
    IoWrite8(0x4e,0x87);
    IoWrite8(0x4e,0x87);
    IoWrite8(0x4e,0x07);
    IoWrite8(0x4f,0x0a);
    IoWrite8(0x4e,0xe6);
    IoWrite8(0x4f,0x20);
//[-end-140715-MH4368-add]//
  }
  //
  //  Init Bank 1
  //
  BankNumber = HMIndexReading (0x4E);
  BankNumber &= 0xFC;
  BankNumber |= 0x01;
  HMIndexWriting (0x4E, BankNumber);
  HMIndexWriting (0x10, 0x81);
  HMIndexWriting (0x19, 0xFF);
  HMIndexWriting (0x1B, 0x01);
  HMIndexWriting (0x1C, 0x10);
  HMIndexWriting (0x20, 0x82);
  HMIndexWriting (0x29, 0xFF);
  HMIndexWriting (0x2B, 0x01);
  HMIndexWriting (0x2C, 0x10);
  HMIndexWriting (0x30, 0x83);
  HMIndexWriting (0x39, 0x7F);
  HMIndexWriting (0x3B, 0x01);
  HMIndexWriting (0x3C, 0x10);
  //
  //  Init Bank 2
  //
  BankNumber = HMIndexReading (0x4E);
  BankNumber &= 0xFC;
  BankNumber |= 0x02;
  HMIndexWriting (0x4E, BankNumber);
  HMIndexWriting (0x00, 0x85);
  HMIndexWriting (0x01, 0x32);
  HMIndexWriting (0x02, 0x30);
  HMIndexWriting (0x03, 0x01);
  HMIndexWriting (0x04, 0x48);
  HMIndexWriting (0x05, 0x48);
  HMIndexWriting (0x06, 0x00);
  HMIndexWriting (0x07, 0x00);
  HMIndexWriting (0x0A, 0x00);
  HMIndexWriting (0x0B, 0x00);
  HMIndexWriting (0x20, 0x48);
  HMIndexWriting (0x22, 0x1F);
  HMIndexWriting (0x24, 0x30);
  //
  //  Init Bank 3
  //
  BankNumber = HMIndexReading (0x4E);
  BankNumber &= 0xFC;
  BankNumber |= 0x03;
  HMIndexWriting (0x4E, BankNumber);
  HMIndexWriting (0x18, 0x01);
  HMIndexWriting (0x19, 0x04);
  if (!EFI_ERROR (Status) && SetupConfig->CaseOpen == 0) {
    HMIndexWriting (0xC4, 0x01);
  } else {
    HMIndexWriting (0xC4, 0x05);
  }

  FreePool (SetupConfig);

}

EFI_STATUS
GetVoltageData2 (
  OUT EFI_HARDWARE_MONITOR_INFO2 **Info,
  OUT INTN                       *Counter
  )
{
//  UINT8     Index;
  UINT8     Size;
//  UINT8     BankNumber = 0;

  Size = sizeof (mVoltageData) / sizeof (EFI_HARDWARE_MONITOR_INFO2);
/*
  BankNumber = HMIndexReading (0x4E);
  BankNumber &= 0xFC;
  HMIndexWriting (0x4E, BankNumber);

  for ( Index = 0; Index < Size; Index++)
  {
    mVoltageData[Index].Data  = (UINTN) HMIndexReading ( mVoltageData[Index].LSB) * 8; // The unit of reading is 8mV.
    if (Index == 2) {
      mVoltageData[Index].Data  = (UINTN) HMIndexReading ( mVoltageData[Index].LSB) * 8 * 12; // The unit of reading is 8mV.
    } else if (Index == 3) {
      mVoltageData[Index].Data  = (UINTN) HMIndexReading ( mVoltageData[Index].LSB) * 8 * 5; // The unit of reading is 8mV.
    }
    mVoltageData[Index].Label = SIO_VOL_LABEL + Index;
  }
*/  
 
  mVoltageData[0].Data= (UINTN)( CompalECMemoryRead(ECRAM_BASE +  0x0C) *3300/255);  //Vcore
  mVoltageData[0].Label = SIO_VOL_LABEL;

  switch (ProjectName)
  {
  case BCX40:
    mVoltageData[1].Data= (UINTN)( CompalECMemoryRead(ECRAM_BASE +  0x0B) *3300/255*2243/243);  //VinL
    break;
  case BCX41:
    mVoltageData[1].Data= (UINTN)( CompalECMemoryRead(ECRAM_BASE +  0x0B) *3300/255*20/10);  //VinL
    break;
  case AAXB2:
  case BCX42:
    mVoltageData[1].Data= (UINTN)( CompalECMemoryRead(ECRAM_BASE +  0x0B) *3300/255*2249/249);  //VinL
    break;     
  case BCX50:
    mVoltageData[1].Data= (UINTN)( CompalECMemoryRead(ECRAM_BASE +  0x0B) *3300/255*2249/249);  //VinL
    break;
  default:
      break;    
  }
  mVoltageData[1].Label = SIO_VOL_LABEL + 1;
  *Info         = mVoltageData;
  *Counter      = Size;

  return EFI_SUCCESS;
}

EFI_STATUS
GetVoltageStatus (
  IN  UINTN  Register,
  OUT UINTN  *Status
  )
{

  return EFI_SUCCESS;
}

EFI_STATUS
SetVoltageCtrl (
  IN UINTN  Register,
  IN UINTN  Data
  )
{

  return EFI_SUCCESS;
}

EFI_STATUS
GetFanSpeedData2 (
  OUT EFI_HARDWARE_MONITOR_INFO2 **Info,
  OUT INTN                       *Counter
  )
{
  UINT8     Index;
  UINT8     Size;
  UINTN     Data;
//  UINT8     BankNumber = 0;

  Size = sizeof (mFanSpeedData) / sizeof (EFI_HARDWARE_MONITOR_INFO2);

//  BankNumber = HMIndexReading (0x4E);
//  BankNumber &= 0xFC;
//  HMIndexWriting (0x4E, BankNumber);
  Data = CompalECMemoryRead(ECRAM_BASE +  0x09); //FanSpeedRPM
  for ( Index = 0; Index < Size ; Index++)
  {
 //   Data  = HMIndexReading ( mFanSpeedData[Index].MSB );// MSB
 //   Data  = Data << 8;
 //   Data |= HMIndexReading ( mFanSpeedData[Index].LSB); // LSB

//    if ((Data == 0) || (Data == 0xFFFF)) {
//      mFanSpeedData[Index].Data = 0;
//    } else {
//      mFanSpeedData[Index].Data  = HARDWARE_MONITOR_FAN_SPEED_COUNT / (Data * 2);
      mFanSpeedData[Index].Data  = (UINTN) Data * 100;
//    }
    mFanSpeedData[Index].Label = SIO_FAN_LABEL + Index;
  }
  *Info         = mFanSpeedData;
  *Counter      = Size;

  return EFI_SUCCESS;
}

EFI_STATUS
GetFanSpeedStatus (
  IN  UINTN  Register,
  OUT UINTN  *Status
  )
{

  return EFI_SUCCESS;
}

EFI_STATUS
SetFanSpeedCtrl (
  IN UINTN  Register,
  IN UINTN  Data
  )
{

  return EFI_SUCCESS;
}

EFI_STATUS
GetTemperatureData2 (
  OUT EFI_HARDWARE_MONITOR_INFO2  **Info,
  OUT INTN                        *Counter
  )
{
  UINT8     Index;
  UINT8     Size;
//  UINT8     DTS_value = 0;
  UINT8     EC_ThermisterTemp=0;
  Size = sizeof (mTemperatureData) / sizeof (EFI_HARDWARE_MONITOR_INFO2);

//  BankNumber = HMIndexReading (0x4E);
//  BankNumber &= 0xFC;
//  HMIndexWriting (0x4E, BankNumber);
//  DTS_value = (UINT8)(((AsmReadMsr64 (0x1a2)&0x00ff0000)-(AsmReadMsr64 (0x19c)&0x007f0000))>>16);
  EC_ThermisterTemp = CompalECMemoryRead(ECRAM_BASE +  0xB1); //CPU Temp
  for ( Index = 0; Index < Size; Index++)
  {
//    mTemperatureData[1].Data  = (UINTN)DTS_value;
    mTemperatureData[0].Data  = (UINTN)EC_ThermisterTemp;
    mTemperatureData[Index].Label = SIO_TEMP_LABEL + Index;
  }

  *Info         = mTemperatureData;
  *Counter      = Size;

  return EFI_SUCCESS;
}


EFI_STATUS
GetTemperatureStatus (
  IN  UINTN  Register,
  OUT UINTN  *Status
  )
{

  return EFI_SUCCESS;
}

EFI_STATUS
SetTemperatureCtrl (
  IN UINTN  Register,
  IN UINTN  Data
  )
{

  return EFI_SUCCESS;
}

