/** @file
  I2C Bus Access for Dxe

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _EFI_I2C_BUS_ACCESS_H
#define _EFI_I2C_BUS_ACCESS_H

#define EFI_I2C_VERSION_1  0x00000001
#define EFI_I2C_VERSION_2  0x00000002

#define EFI_I2C_HC_ACCESS_PROTOCOL_GUID \
  { \
    0x5beccc5a, 0x19c9, 0x42eb,  0x8e, 0x44, 0x9c, 0xb3, 0x97, 0x74, 0x85, 0xe8 \
  }

typedef struct _EFI_I2C_HC_ACCESS_PROTOCOL  EFI_I2C_HC_ACCESS_PROTOCOL;

typedef struct {
  UINTN I2cHcDeviceAddress : 7;
} EFI_I2C_DEVICE_ADDRESS;

typedef
enum {
  I2cBusFreq100KHz = 0,
  I2cBusFreq400KHz,
  I2cBusFreq1MHz,
  MaxI2cBusFreq
}I2C_BUS_SPEED;
/**

  Get current device's location

  @param  This                  I2C Host Controller
  @param  BusNumber             Current PCI Bus
  @param  DeviceNumber          Current PCI Device
  @param  FunctionNumber        Current PCI Function

  @retval EFI_SUCCESS

**/
typedef
EFI_STATUS
(*I2C_HC_GET_LOCATION)(
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This,
  OUT       UINTN                       *BusNumber,
  OUT       UINTN                       *DeviceNumber,
  OUT       UINTN                       *FunctionNumber
  );

/**

  Access the I2C bus

  @param  This                  I2C Host Controller
  @param  SlaveAddress          Device slave address
  @param  WriteBytes            Count of wirte into I2C bus
  @param  WriteBuffer           Written buffter
  @param  ReadBytes             Count of read into I2C bus
  @param  ReadBuffer            Read buffer
  @param  Rs                    The 'Rs' argument should be set to FALSE and reserved for future expansion

  @retval EFI_SUCCESS
  @retval EFI_DEVICE_ERROR      I2C bus not exist
  @retval EFI_TIMEOUT           Device is no acknowledge

**/

typedef
EFI_STATUS
(*I2C_HC_ACCESS)(
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This,
  IN        UINTN                       SlaveAddress,
  IN  OUT   UINTN                       *WriteBytes,
  IN        UINT8                       *WriteBuffer,
  IN  OUT   UINTN                       *ReadBytes,
  OUT       UINT8                       *ReadBuffer,
  IN        BOOLEAN                     Rs
  );

/**

  Event-driven execution this method

  @param  SlaveAddress          Device slave address
  @param  Data                  Private date 

  @retval EFI_SUCCESS

**/
typedef
EFI_STATUS
(EFIAPI *EFI_I2C_NOTIFY_FUNCTION) (
  IN      EFI_I2C_DEVICE_ADDRESS        SlaveAddress,
  IN      UINTN                         Data
  );

/**

  Whether the event is triggered

  @param  This                  I2C Host Controller

  @retval TRUE                  It was triggered
  @retval FALSE                 Wait triggering

**/
typedef
BOOLEAN
(EFIAPI *EFI_I2C_HC_TRIGGER_METHOD) (
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This
  );

/**

  Waiting for other event to trigger I2C bus access

  @param  This                  I2C Host Controller
  @param  SlaveAddress          Device slave address
  @param  TriggerMethod         Received an external event for trigger access
  @param  Data                  It will be used for NotifyFunction
  @param  NotifyFunction        Process this function when get a readiness from TriggerMethod.

  @retval EFI_SUCCESS
  @retval EFI_INVALID_PARAMETER No NotifyFunction
  @retval EFI_UNSUPPORTED       Failed to create event

**/
typedef
EFI_STATUS
(EFIAPI *I2C_HC_NOTIFY) (
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This,
  IN        EFI_I2C_DEVICE_ADDRESS      SlaveAddress,
  IN        EFI_I2C_HC_TRIGGER_METHOD   TriggerMethod,
  IN        UINTN                       Data,
  IN        EFI_I2C_NOTIFY_FUNCTION     NotifyFunction
  );

/**

  Set the interval time between each access, default value is 2 microsecond

  @param  This                  I2C Host Controller
  @param  Time                  Interval time of access
  
  @retval EFI_SUCCESS

**/
typedef
EFI_STATUS
(EFIAPI *I2C_HC_INTERVAL_TIME) (
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This,
  IN        UINT32                      Time
  );

/**

  Set the default bus speed of I2C bus, default value is 100KHz

  @param  This                  I2C Host Controller
  @param  Freq                  I2C bus Freq
  
  @retval EFI_SUCCESS

**/
typedef
EFI_STATUS
(EFIAPI *I2C_HC_SET_SPEED) (
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This,
  IN        I2C_BUS_SPEED               Freq
  );

/**

  Update I2C BAR

  @param  This                  I2C Host Controller
  @param  Bar0                  I2C Host Base Address
  
  @retval EFI_SUCCESS

**/
typedef
EFI_STATUS
(EFIAPI *I2C_HC_UPDATE_BAR) (
  IN  CONST EFI_I2C_HC_ACCESS_PROTOCOL  *This,
  IN        EFI_PHYSICAL_ADDRESS        Bar0
  );

#pragma pack(1)

struct _EFI_I2C_HC_ACCESS_PROTOCOL {
  UINT32                            Version;
  I2C_HC_INTERVAL_TIME              IntervalTime;
  I2C_HC_SET_SPEED                  SetBusSpeed;
  I2C_HC_GET_LOCATION               GetLocation;
  I2C_HC_ACCESS                     Access;
  I2C_HC_NOTIFY                     Notify;
  I2C_HC_UPDATE_BAR                 UpdateBar;
};
#pragma pack()

extern EFI_GUID gEfiI2cHcAccessProtocolGuid;
#endif

