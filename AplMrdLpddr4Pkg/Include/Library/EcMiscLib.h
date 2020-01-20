/** @file
  EC miscellaneous library functions.

@copyright
 Copyright (c) 1999 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains 'Framework Code' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may not be modified, except as allowed by
 additional terms of your license agreement.

@par Specification Reference:
**/

#ifndef _BASE_EC_MISC_LIB_H_
#define _BASE_EC_MISC_LIB_H_

//
// Include files
//
#include <Base.h>
#include <Uefi.h>
#include "EcCommands.h"
#include <Library/EcLib.h>

///
/// @todo: it should move to BaseEcAcpiLib once BaseEcAcpiLib has been created.
///
/**
  Send enable ACPI mode command to EC

  @param[in]  Enable            TURE - Enable EC ACPI mode
                                FALSE - Disable EC ACPI mode
  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
EnableEcAcpiMode (
  IN BOOLEAN      Enable
  );


/**
  Send disable SMI mode command to EC

  @param  EFI_SUCCESS         Command success
  @param  EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
DisableEcSmiMode (
  VOID
  );


/**
  Set EC critical shutdown temperature

  @param[in]  DataBuffer          Shutdown temperature
  @retval     EFI_SUCCESS         Command success
  @retval     EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
SetEcCriticalShutdownTemperature (
  IN UINT8   DataBuffer
  );


/**
  Get power state from EC.  If power state cannot be determined, 
  battery powered is assumed.

  @param  TRUE         AC powered
  @param  FALSE        Battery powered
**/
BOOLEAN
PowerStateIsAc (
  VOID
  );


/**
  Do EC reset

  @retval  EFI_SUCCESS         Command success
  @retval  EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
EcReset (
  VOID
  );


/**
  Enable LAN power

  @param[in]  Enable            TURE - Enable LAN power
                                FALSE - Disable LAN power

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
EnableLanPower (
  IN BOOLEAN      Enable
  );


/**
  Query SMI event

  @param[out] DataBuffer        SMI event

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
QuerySmiEvent (
  OUT UINT8    *DataBuffer
  );


/**
  Get the scan code of hotkey pressed (CTRL + ALT + SHIFT + key)

  @param[out] DataBuffer        Scan code of hotkey

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetHotkeyScanCode (
  OUT UINT8    *DataBuffer
  );


/**
  Get the number of battery

  @param[out] DataBuffer        The number of battery

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetBatteryNumber (
  OUT UINT8    *DataBuffer
  );


/**
  Get PCIe dock status

  @param[out] DataBuffer        PCIe dock status

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetPcieDockStatus (
  OUT UINT8    *DataBuffer
  );


/**
  Clear ten second power button mode

**/
VOID
ClearTenSecPwrButtonMode (
  VOID
  );


/**
  Enable SATA port2 power

**/
VOID
EnableSataPort2Power (
  VOID
  );


/**
  Set Pmic Voltage

  @param[in]  EcDataVcc
  @param[in]  EcDataVddq

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
SetPmicVoltage (
  IN UINT8       EcDataVcc,
  IN UINT8       EcDataVddq
  );


/**
  Read data from EC RAM

  @param[in][out]  DataBuffer

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
ReadEcRam (
  IN OUT UINT8       *DataBuffer
  );


/**
  Write data to EC RAM

  @param[in][out]  DataBuffer

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
WriteEcRam (
  IN OUT UINT8       *DataBuffer
  );


/**
  Update Device Power

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
UpdateDevicePower (
  VOID
  );


/**
  Query system status

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
QuerySysStatus (
  OUT UINT8       *DataBuffer
  );


/**
  Get board fab ID

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetBoardFabId (
  OUT UINT8       *DataBuffer
  );


/**
  Send Exit Low Power Mode command to EC

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
ExitLowPowerMode (
  VOID
  );


/**
  System power off

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
SystemPowerOff (
  VOID
  );


/**
  Detect EC revision

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
DetectEcRevision (
  OUT UINT8       *DataBuffer
  );


/**
  Get Base EC FW version

  @param[out] DataBuffer        Byte 1: Base EC FW major version
                                Byte 2: Base EC FW minor version

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetBaseEcFwVersion (
  OUT UINT8    *DataBuffer
  );


/**
  Get EC-EC Protocol version

  @param[out] DataBuffer        Byte 1: Lid EC protocol version
                                Byte 2: Base EC protocol version

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetEc2EcProtocolVersion (
  OUT UINT8    *DataBuffer
  );

/**
  Disable PCH DTS reading on SMLink 1

  @param[in]  Disable           0: Enable  PCH DTS reading
                                1: Disable PCH DTS reading

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
DisablePchDtsReading (
  IN UINT8       Disable
  );

/**
  Wake up EC

  @retval     TRUE            EC wake up successfully.
  @retval     FALSE           Fail to wake up EC.
**/
EFI_STATUS
WakeUpEc (
  VOID
  );

/**
  Send Update PWM command to EC

  @param  EFI_SUCCESS         Command success
  @param  EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
UpdatePwm (
  VOID
  );

/**
  Updates the PWM to the given value  and returns the Fan speed.
  @param  Pwmvalue            Pwm ending value
  @param  Speed               Updates the Speed

  @retval EFI_SUCCESS         when successfully able to get the nominal data.
  @retval ERROR               Error if anything went wrong.
  
**/
EFI_STATUS
GetFanSpeed(
 IN  UINT8    Pwmvalue,
 OUT UINT16   *Speed
);
#endif
