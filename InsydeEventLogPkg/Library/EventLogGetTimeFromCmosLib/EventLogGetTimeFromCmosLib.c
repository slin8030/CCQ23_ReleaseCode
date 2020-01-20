/** @file
 EventLogGetTimeFromCmosLib Lib functions which relate with connect the device

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

*/

//[-start-170413-IB08400443-modify]//
#include "EventLogGetTimeFromCmosLibInternal.h"
//[-end-170413-IB08400443-modify]//

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
  )
{
  RTC_REGISTER_A  RegisterA;
  RTC_REGISTER_B  RegisterB;
  UINTN           Index;
  UINTN           RtcComponentNum;
  RTC_COMPONENT   RtcComponent[] = {{CMOS_YEAR  , 0}, // Year
                                    {CMOS_MONTH , 0}, // Month
                                    {CMOS_DAY   , 0}, // Day
                                    {CMOS_HOUR  , 0}, // Hour
                                    {CMOS_MINUTE, 0}, // Minute
                                    {CMOS_SECOND, 0}  // Second
                                    };
//[-start-170413-IB08400443-modify]//
  UINTN           RetryIndex;
  BOOLEAN         IsUip = FALSE;
//[-end-170413-IB08400443-modify]//

  if (Time == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  RtcComponentNum = sizeof(RtcComponent) / sizeof(RTC_COMPONENT);
  RegisterB.Data = ReadCmos8 (CMOS_REGISTER_B);

  //
  // In order to preventing RTC data reading error upon RTC updating due to interrupt/SMI caused delay,
  // making RTC update checking before every RTC data reading to make sure every RTC data corrected
  //
//[-start-170413-IB08400443-modify]//
  for (RetryIndex = 0; RetryIndex < MAX_RETRY; RetryIndex++) {
    IsUip = FALSE;
    for (Index = 0; Index < RtcComponentNum; Index++) {
      RegisterA.Data = ReadCmos8 (CMOS_REGISTER_A);
      if (RegisterA.Bits.Uip == 1) {
        MicroSecondDelay (10);
        IsUip = TRUE;
        break;
      }
      RtcComponent[Index].Data = ReadCmos8 (RtcComponent[Index].Address);
    }
    
    if (IsUip == FALSE) {
      //
      // Get the correct time, break the FOR loop.
      //
      break;
    } else {
      //
      // Not get the correct time, retry again.
      //
      continue;
    }
  }

  if (IsUip == TRUE) { 
    Time->Year   = 0;
    Time->Month  = 0;
    Time->Day    = 0;
    Time->Hour   = 0;
    Time->Minute = 0;
    Time->Second = 0;
  } else {
    Time->Year   = RtcComponent[0].Data;
    Time->Month  = RtcComponent[1].Data;
    Time->Day    = RtcComponent[2].Data;
    Time->Hour   = RtcComponent[3].Data;
    Time->Minute = RtcComponent[4].Data;
    Time->Second = RtcComponent[5].Data;
  }
//[-end-170413-IB08400443-modify]//

  return EFI_SUCCESS;
}
