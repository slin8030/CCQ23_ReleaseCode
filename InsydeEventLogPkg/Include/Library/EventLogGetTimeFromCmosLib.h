/** @file

  Header file of Event Log Get Time From CMOS Lib implementation.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _EVENT_LOG_GET_TIME_FROM_CMOS_LIB_H_
#define _EVENT_LOG_GET_TIME_FROM_CMOS_LIB_H_

#pragma pack(1)

typedef struct {
  UINT8   Year;
  UINT8   Month;
  UINT8   Day;
  UINT8   Hour;
  UINT8   Minute;
  UINT8   Second;
} EL_TIME;

#pragma pack()

/**
 Read time from RTC, and store in a EL_TIME structure.
 Before reading every RTC data, it will check RTC update status to make sure every RTC data is correct.

 @param[out]        Time                EL_TIME structure to store time.

 @retval EFI_SUCCESS                    Get RTC time successfully.
 @retval EFI_NOT_READY                  RTC data is in update progress. Fail to get RTC time.
 @retval EFI_INVALID_PARAMETER          Time is NULL.
*/
EFI_STATUS
EventLogGetTimeFromCmos (
  OUT EL_TIME                          *Time
  );

#endif
