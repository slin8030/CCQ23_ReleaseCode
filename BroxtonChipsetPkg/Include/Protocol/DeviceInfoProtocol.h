/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  DeviceInfoProtocol.h
  
Abstract:

 
--*/

#ifndef _DEVICE_INFO_DXE_H_
#define _DEVICE_INFO_DXE_H_

#include <PiDxe.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>

typedef struct _DEVICE_INFO_PROTOCOL DEVICE_INFO_PROTOCOL;

typedef enum {
  USB_SDP,
  USB_DCP, // default setting if HW does not allow to differentiate the charger type
  USB_CDP,
  USB_ACA,
} USB_CHARGER_TYPE;

typedef UINT8  BATT_CAPACITY; // use the same type for IA_APPS_CAP
typedef UINT16 BATT_VOLTAGE;  // use the same type for IA_APPS_RUN

typedef 
EFI_STATUS
(EFIAPI *GET_BATTERY_STATUS) (
  OUT BOOLEAN *BatteryPresent,
  OUT BOOLEAN *BatteryValid,
  OUT BOOLEAN *CapacityReadable, // based on FG.STATUS.POR
  OUT BATT_VOLTAGE *BatteryVoltageLevel, // mVolts
  OUT BATT_CAPACITY *BatteryCapacityLevel // percentage
  );

typedef
EFI_STATUS
(EFIAPI *GET_ACDC_CHARGER_STATUS) (
  OUT BOOLEAN *ACDCChargerPresent
  );

typedef
EFI_STATUS
(EFIAPI *GET_USB_CHARGER_STATUS) (
  OUT BOOLEAN *UsbChargerPresent,
  OUT USB_CHARGER_TYPE *UsbChargerType
  );

struct _DEVICE_INFO_PROTOCOL {
  UINT32                  Revision;
  GET_BATTERY_STATUS      GetBatteryStatus;
  GET_ACDC_CHARGER_STATUS GetAcDcChargerStatus;
  GET_USB_CHARGER_STATUS  GetUsbChargerStatus;
} ;

#define DEVICE_INFO_REV   0x01
//
// {E4F3260B-D35F-4af1-B90E-910F5AD2E326}
//
extern EFI_GUID gDeviceInfoProtocolGuid;

#endif
