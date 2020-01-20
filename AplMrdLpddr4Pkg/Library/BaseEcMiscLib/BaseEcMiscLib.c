/** @file
  BaseEcMisc library implementation.

@copyright
 Copyright (c) 2014 - 2015, Intel Corporation. All rights reserved
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
#include <Library/EcLib.h>
#include <Library/EcMiscLib.h>
#include <Library/DebugLib.h>

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
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_SUCCESS;
//[-end-161004-IB08450356-modify]//
}


/**
  Send disable APM mode command to EC

  @param  EFI_SUCCESS         Command success
  @param  EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
DisableEcSmiMode (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_SUCCESS;
//[-end-161004-IB08450356-modify]//
}


/**
  Set EC critical shutdown temperature

  @param[in]  DataBuffer          Shutdown temperature
  @retval     EFI_SUCCESS         Command success
  @retval     EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
SetEcCriticalShutdownTemperature (
  IN UINT8   DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Get power state from EC.  If power state cannot be determined, 
  battery powered is assumed.

  @param  TRUE         AC powered
  @param  FALSE        Battery powered
**/
BOOLEAN
PowerStateIsAc (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  EFI_STATUS  Status;
  UINT8       TempData;
  
  
  TempData = 0x80;
  
  Status = ReadEcRam (&TempData);
  ///
  /// At this point, we expect this to always succeed
  ///
  ASSERT_EFI_ERROR (Status);

  ///
  /// Return power state
  /// If we had an issue, we default to battery mode to save a small amount of power.
  ///
  if (!EFI_ERROR (Status) && (TempData & 0x01)) {
    return TRUE;
  } else {
    return FALSE;
  }
//[-end-161004-IB08450356-modify]//
}


/**
  Do EC reset

  @retval  EFI_SUCCESS         Command success
  @retval  EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
EcReset (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR; 
//[-end-161004-IB08450356-modify]//
}


///
/// @todo: it should move to BaseEcPowerLib once BaseEcPowerLib has been created.
///
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
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Query SMI event

  @param[out] DataBuffer        SMI event

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
QuerySmiEvent (
  OUT UINT8    *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Get the scan code of hotkey pressed (CTRL + ALT + SHIFT + key)

  @param[out] DataBuffer        Scan code of hotkey

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetHotkeyScanCode (
  OUT UINT8    *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Get the number of battery

  @param[out] DataBuffer        The number of battery

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetBatteryNumber (
  OUT UINT8    *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  UINT8       Data;
  EFI_STATUS  Status;
  


  Data = 0x80;    
  Status = ReadEcRam (&Data);
  
  ASSERT_EFI_ERROR (Status);
  
  *DataBuffer = ((!EFI_ERROR(Status)) && (Data & 0x02)) ? 1 : 0;
  return EFI_SUCCESS;    
//[-end-161004-IB08450356-modify]//
}


/**
  Get PCIe dock status

  @param[out] DataBuffer        PCIe dock status

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetPcieDockStatus (
  OUT UINT8    *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Clear ten second power button mode

**/
VOID
ClearTenSecPwrButtonMode (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
//[-end-161004-IB08450356-modify]//
}


/**
  Enable SATA port2 power

**/
VOID
EnableSataPort2Power (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
//[-end-161004-IB08450356-modify]//
}


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
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_SUCCESS;
//[-end-161004-IB08450356-modify]//
}


/**
  Read data from EC RAM

  @param[in][out]  DataBuffer

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
ReadEcRam (
  IN OUT UINT8       *DataBuffer
  )
{
  UINT8          DataSize;

  DataSize = 1;
  return (EcInterface (EcId0, LpcInterface, EC_C_READ_MEM, &DataSize, DataBuffer));
}


/**
  Write data to EC RAM

  @param[in][out]  DataBuffer

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
WriteEcRam (
  IN OUT UINT8       *DataBuffer
  )
{
  UINT8          DataSize;

  DataSize = 2;
  return (EcInterface (EcId0, LpcInterface, EC_C_WRITE_MEM, &DataSize, DataBuffer));
}


/**
  Update Device Power

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
UpdateDevicePower (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_SUCCESS;
//[-end-161004-IB08450356-modify]//
}


/**
  Query system status

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
QuerySysStatus (
  OUT UINT8       *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Get board fab ID

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
GetBoardFabId (
  OUT UINT8       *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Send Exit Low Power Mode command to EC

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
ExitLowPowerMode (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  System power off

  @retval          EFI_SUCCESS       Command success
  @retval          EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
SystemPowerOff (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


/**
  Detect EC revision

  @param[out] DataBuffer

  @retval     EFI_SUCCESS       Command success
  @retval     EFI_DEVICE_ERROR  Command error
**/
EFI_STATUS
DetectEcRevision (
  OUT UINT8       *DataBuffer
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}


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
  )
{
//[-start-161004-IB08450356-modify]//
  UINT8       Data;
  EFI_STATUS  Status;

  Data = EC_MEM_OFFSET_EC_VER_MAIN;
  Status = ReadEcRam (&Data);  
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DataBuffer[0] = Data;
   
  Data = EC_MEM_OFFSET_EC_VER_SUB;
  Status = ReadEcRam (&Data);  
  if (EFI_ERROR(Status)) {
    return Status;
  }
  DataBuffer[1] = Data;
  
  return EFI_SUCCESS;
//[-end-161004-IB08450356-modify]//
}


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
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR; 
//[-end-161004-IB08450356-modify]//
}

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
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}

/**
  Wake up EC

  @retval     TRUE            EC wake up successfully.
  @retval     FALSE           Fail to wake up EC.
**/
EFI_STATUS
WakeUpEc (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}

/**
  Send Update PWM command to EC

  @param  EFI_SUCCESS         Command success
  @param  EFI_DEVICE_ERROR    Command error
**/
EFI_STATUS
UpdatePwm (
  VOID
  )
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR;
//[-end-161004-IB08450356-modify]//
}

/**
  Updates the PWM to the given value  and returns the Fan speed.
  @param  Pwmvalue            Pwm ending value
  @param  Speed               Updates the Speed

  @retval EFI_SUCCESS         when successfully able to get the nominal data.
  @retval ERROR               Error if anything went wrong.
  
**/
EFI_STATUS
GetFanSpeed(
 IN  UINT8   Pwmvalue,
 OUT UINT16  *Speed
)
{
//[-start-161004-IB08450356-modify]//
  return EFI_DEVICE_ERROR; 
//[-end-161004-IB08450356-modify]//
}
