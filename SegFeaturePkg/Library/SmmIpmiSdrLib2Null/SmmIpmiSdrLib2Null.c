/** @file
 H2O IPMI SDR library implement code.

 This c file contains SDR library instance in SMM phase.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
#include <Library/IpmiLibDefine.h>

/**
 Get sensor data from "SDR Repository Device" or "Device SDR".

 @param[in]         RecordId            Speficied ID of SDR that want to get. 0xFFFF means getting all recoreds.
 @param[in]         Flag                Low byte: if low byte set to 0x01(DISPLAY_LONG_UNIT), it will
                                                  return full unit string in SDR_DATA_STURCT. Otherwise
                                                  it will return short unit string by default.
                                        High byte: decimal number in all FLOAT string. Maximum is 10 digital.
 @param[out]        Count               Number of records if user passes 0xFFFF in RecordId.
 @param[out]        Record              A buffer to SDR_DATA_STURCT. It is caller's responsibility
                                        to free allocated memory.

 @retval EFI_SUCCESS                    Get SDR data success.
 @retval EFI_UNSUPPORTED                Speificed Id does not exist or cannot get SDR data.
 @retval EFI_INVALID_PARAMETER          1. Count is NULL
                                        2. Record is NULL
*/
EFI_STATUS
IpmiLibGetSdrData2 (
  IN  UINT16                            RecordId,
  IN  UINT16                            Flag,
  IN  BOOLEAN                           IsHeci,
  OUT UINT16                            *Count,
  OUT SDR_DATA_STURCT                   **Record
  )
{
  return EFI_UNSUPPORTED;
}


/**
 Get sensor raw data from "SDR Repository Device" or "Device SDR".

 @param[in]         RecordId            Speficied ID of SDR that want to get.
 @param[out]        Record              A buffer to SDR_DATA_STURCT. It is caller's responsibility
                                        to free allocated memory.

 @retval EFI_SUCCESS                    Get SDR data success.
 @retval EFI_UNSUPPORTED                Speificed Id does not exist or cannot get SDR data.
 @retval EFI_INVALID_PARAMETER          Record is NULL.
*/
EFI_STATUS
IpmiLibGetSdrRaw2 (
  IN  UINT16                            RecordId,
  IN  BOOLEAN                           IsHeci,
  OUT H2O_IPMI_SDR_RAW                  **Record
  )
{
  return EFI_UNSUPPORTED;
}

/**
 Convert analog sensor reading or threshold event data (trigger reading or trigger threshold) value to string.

 @param[in]         SensorNumber        Unique number identifying the sensor.
 @param[in]         SensorReading       A byte sensor reading or event data (contain trigger reading or
                                        trigger threshold) value need to convert.
 @param[in]         Precision           The precision of value to string.
 @param[out]        ConvertValueStr     The point of CHAR16 string space to carry converted value string.

 @retval EFI_SUCCESS                    Convert sensor reading value success.
 @retval EFI_NOT_READY                  Get sensor data record fail.
 @retval EFI_UNSUPPORTED                Unique sensor number can't find the corresponding SDR or
                                        the record type of sensor number is not full type.
 @retval EFI_INVALID_PARAMETER          1. This is NULL
                                        2. ConvertValueStr is NULL
*/
EFI_STATUS
IpmiLibSensorReadingToStr2 (
  IN  UINT8                             SensorNumber,
  IN  UINT8                             SensorReading,
  IN  UINT8                             Precision,
  IN  BOOLEAN                           IsHeci,
  OUT CHAR16                            *ConvertValueStr
  )
{
  return EFI_UNSUPPORTED;
}

