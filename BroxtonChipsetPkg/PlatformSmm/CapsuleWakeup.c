/** @file

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

#include "SmmPlatform.h"
#include <Library/BaseLib.h>
#include <SecureFlash.h>
#include <Library/VariableLib.h>
#include <Library/CmosLib.h>

#define RTC_ADDRESS_SECONDS           0   // R/W  Range 0..59
#define RTC_ADDRESS_SECONDS_ALARM     1   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES           2   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES_ALARM     3   // R/W  Range 0..59
#define RTC_ADDRESS_HOURS             4   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_HOURS_ALARM       5   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_DAY_OF_THE_WEEK   6   // R/W  Range 1..7
#define RTC_ADDRESS_DAY_OF_THE_MONTH  7   // R/W  Range 1..31
#define RTC_ADDRESS_MONTH             8   // R/W  Range 1..12
#define RTC_ADDRESS_YEAR              9   // R/W  Range 0..99


extern EFI_SMM_VARIABLE_PROTOCOL               *mSmmVariable;

/**

 @param [in]   System wakeup time

 @retval None.

**/
VOID
WakeToProcessPendingCapsule (
  IN UINT16           AcpiBaseAddr,
  IN UINT8            WakeAfter
  )
{
  EFI_STATUS          Status;
  UINTN               Size;
  IMAGE_INFO          ImageInfo;
  UINT8               Reminder;
  UINT8               RtcSecond;
  UINT8               RtcMinute;
  UINT8               RtcHour;
  UINT8               RtcSts;
  UINT16              RtcEn;
  UINT8               AIE;
  BOOLEAN             BcdMode;
  UINT8               Buffer;

  Reminder  = 0;
  RtcSecond = 0;
  RtcMinute = 0;
  RtcHour   = 0;
  AIE       = 0;
  RtcSts    = 0;
  RtcEn     = 0;
  BcdMode   = TRUE;

  //
  // Wake time should locate between 1s ~ 15s
  //
  WakeAfter &= 0x0F;
  if (WakeAfter < 1) {
    WakeAfter = 1;
  }

//  __debugbreak ();
  //
  // Check RTC mode is BCD or Binary
  //
  Buffer  = ReadCmos8 (RTC_ADDRESS_REGISTER_B);
  BcdMode = ~(Buffer >> 2);
  BcdMode &= BIT0;

  //
  // Check any CapsuleUpdate through S3, and set RTC wakeup
  //
  Size = sizeof (IMAGE_INFO);
  Status = CommonGetVariable (
             SECURE_FLASH_INFORMATION_NAME,
             &gSecureFlashInfoGuid,
             &Size,
             &ImageInfo);
  if ((Status == EFI_SUCCESS) && (ImageInfo.FlashMode)) {
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosSecureFlashS3Flag, 1);
    //  
    // Caculate RTC wake Second/Minute/Hour
    //
    if (BcdMode) {
      RtcSecond = ReadCmos8 (RTC_ADDRESS_SECONDS);
      RtcSecond = WakeAfter + BcdToDecimal8 (RtcSecond);
      RtcSecond = DecimalToBcd8 (RtcSecond);
      RtcMinute = ReadCmos8 (RTC_ADDRESS_MINUTES);
      RtcMinute = BcdToDecimal8 (RtcMinute);
      RtcMinute = DecimalToBcd8 (RtcMinute);
      RtcHour   = ReadCmos8 (RTC_ADDRESS_HOURS);
      RtcHour   = BcdToDecimal8 (RtcHour);
      RtcHour   = DecimalToBcd8 (RtcHour);
    } else {
      Reminder  = WakeAfter + ReadCmos8 (RTC_ADDRESS_SECONDS);
      RtcSecond = Reminder % 60;
      Reminder  = Reminder / 60;
      Reminder  = Reminder + ReadCmos8 (RTC_ADDRESS_MINUTES);
      RtcMinute = Reminder % 60;
      Reminder  = Reminder / 60;
      Reminder  = Reminder + ReadCmos8 (RTC_ADDRESS_HOURS);
      RtcHour   = Reminder % 24;
    }

    //
    // Set RTC alarm
    //
    WriteCmos8 (RTC_ADDRESS_SECONDS_ALARM, RtcSecond);
    WriteCmos8 (RTC_ADDRESS_MINUTES_ALARM, RtcMinute);
    WriteCmos8 (RTC_ADDRESS_HOURS_ALARM,   RtcHour);

    //
    // Enable Alarm Interrupt Enable
    //
    AIE = ReadCmos8 (RTC_ADDRESS_REGISTER_B);
    AIE |= B_RTC_ALARM_INT_ENABLE;//ALARM_INTERRUPT_ENABLE;
    WriteCmos8 (RTC_ADDRESS_REGISTER_B, AIE);

    //
    // Clear RTC_STS (PMBase + 0h bit10) and write RTC_EN (PMBase + 2h bit10) to generates a wake event
    //
    RtcSts = IoRead8 (AcpiBaseAddr);
    RtcSts |= 0x400;
    IoWrite8 (AcpiBaseAddr, RtcSts);

    RtcEn = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_EN);
    RtcEn |= B_ACPI_PM1_EN_RTC;
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_EN, RtcEn);
  } else {
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosSecureFlashS3Flag, 0);
  }

  return;
}
