/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    Platform.c

Abstract:

    This is a generic template for a child of the IchSmm driver.

Revision History

--*/

//[-start-170314-IB07400847-modify]//
#include "SmmPlatform.h"
#include <Library/CmosLib.h>
#include <Library/BaseLib.h>

//
// Dallas DS12C887 Real Time Clock
//
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
#define RTC_ADDRESS_CENTURY           50  // R/W  Range 19..20 Bit 8 is R/W

#pragma pack(1)

//
// Register A
//
typedef struct {
  UINT8 Rs  : 4;  ///< Rate Selection Bits
  UINT8 Dv  : 3;  ///< Divisor
  UINT8 Uip : 1;  ///< Update in progress
} RTC_REGISTER_A_BITS;

typedef union {
  RTC_REGISTER_A_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_A;

//
// Register B
//
typedef struct {
  UINT8 DSE  : 1;  // 0 - Daylight saving disabled  1 - Daylight savings enabled
  UINT8 MIL  : 1;  // 0 - 12 hour mode              1 - 24 hour mode
  UINT8 DM   : 1;  // 0 - BCD Format                1 - Binary Format
  UINT8 SQWE : 1;  // 0 - Disable SQWE output       1 - Enable SQWE output
  UINT8 UIE  : 1;  // 0 - Update INT disabled       1 - Update INT enabled
  UINT8 AIE  : 1;  // 0 - Alarm INT disabled        1 - Alarm INT Enabled
  UINT8 PIE  : 1;  // 0 - Periodic INT disabled     1 - Periodic INT Enabled
  UINT8 SET  : 1;  // 0 - Normal operation.         1 - Updates inhibited
} RTC_REGISTER_B_BITS;

typedef union {
  RTC_REGISTER_B_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_B;

//
// Register C
//
typedef struct {
  UINT8 Reserved : 4; ///< Read as zero.  Can not be written.
  UINT8 Uf       : 1; ///< Update End Interrupt Flag
  UINT8 Af       : 1; ///< Alarm Interrupt Flag
  UINT8 Pf       : 1; ///< Periodic Interrupt Flag
  UINT8 Irqf     : 1; ///< Iterrupt Request Flag = PF & PIE | AF & AIE | UF & UIE
} RTC_REGISTER_C_BITS;

typedef union {
  RTC_REGISTER_C_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_C;

//
// Register D
//
typedef struct {
  UINT8 AlarmDate : 6; ///< Date Alarm
  UINT8 Reserved  : 1; ///
  UINT8 Vrt       : 1; ///< Valid RAM and Time
} RTC_REGISTER_D_BITS;

typedef union {
  RTC_REGISTER_D_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_D;


typedef struct {
  UINT8  Address; ///< RTC offset address.
  UINT8  Data;    ///< Corresponding data value.
} RTC_COMPONENT;

#pragma pack()

/**
 Read RTC content through its registers.

 @param[in] Address  Address offset of RTC. It is recommended to use macros such as RTC_ADDRESS_SECONDS.

 @return The data of UINT8 type read from RTC.
**/
UINT8
RtcRead (
  IN  UINT8 Address
  )
{
  return ReadCmos8 (Address);
}

/**
 Write RTC through its registers.

 @param[in] Address  Address offset of RTC. It is recommended to use macros such as RTC_ADDRESS_SECONDS.
 @param[in] Data     The content you want to write into RTC.
**/
VOID
RtcWrite (
  IN  UINT8   Address,
  IN  UINT8   Data
  )
{
  WriteCmos8 ((UINT8) Address, Data);
}


/**
 Converts time read from RTC to EFI_TIME format defined by UEFI spec.

 This function converts raw time data read from RTC to the EFI_TIME format defined by UEFI spec.
 If data mode of RTC is BCD, then converts it to decimal,
 If RTC is in 12-hour format, then converts it to 24-hour format.

 @param[in, out] Time       On input, the time data read from RTC to convert
                            On output, the time converted to UEFI format
 @param[in]      RegisterB  Value of Register B of RTC, indicating data mode and hour format.
**/
VOID
ConvertRtcTimeToEfiTime (
  IN EFI_TIME       *Time,
  IN RTC_REGISTER_B RegisterB
  )
{
  BOOLEAN PM;

  if ((Time->Hour) & 0x80) {
    PM = TRUE;
  } else {
    PM = FALSE;
  }

  Time->Hour = (UINT8) (Time->Hour & 0x7f);

  if (RegisterB.Bits.DM == 0) {
    Time->Year    = BcdToDecimal8 ((UINT8) Time->Year);
    Time->Month   = BcdToDecimal8 (Time->Month);
    Time->Day     = BcdToDecimal8 (Time->Day);
    Time->Hour    = BcdToDecimal8 (Time->Hour);
    Time->Minute  = BcdToDecimal8 (Time->Minute);
    Time->Second  = BcdToDecimal8 (Time->Second);
  }
  //
  // If time is in 12 hour format, convert it to 24 hour format
  //
  if (RegisterB.Bits.MIL == 0) {
    if (PM && Time->Hour < 12) {
      Time->Hour = (UINT8) (Time->Hour + 12);
    }

    if (!PM && Time->Hour == 12) {
      Time->Hour = 0;
    }
  }

  Time->Nanosecond  = 0;
  Time->TimeZone    = EFI_UNSPECIFIED_TIMEZONE;
  Time->Daylight    = 0;
}

/**
 Converts time from EFI_TIME format defined by UEFI spec to RTC's.

 This function converts time from EFI_TIME format defined by UEFI spec to RTC's.
 If data mode of RTC is BCD, then converts EFI_TIME to it.
 If RTC is in 12-hour format, then converts EFI_TIME to it.

 @param[in, out] Time       On input, the time data read from UEFI to convert
                            On output, the time converted to RTC format
 @param[in]      RegisterB  Value of Register B of RTC, indicating data mode
**/
VOID
ConvertEfiTimeToRtcTime (
  IN OUT EFI_TIME        *Time,
  IN     RTC_REGISTER_B  RegisterB
  )
{
  BOOLEAN IsPM;

  IsPM = TRUE;
  //
  // Adjust hour field if RTC is in 12 hour mode
  //
  if (RegisterB.Bits.MIL == 0) {
    if (Time->Hour < 12) {
      IsPM = FALSE;
    }

    if (Time->Hour >= 13) {
      Time->Hour = (UINT8) (Time->Hour - 12);
    } else if (Time->Hour == 0) {
      Time->Hour = 12;
    }
  }

  Time->Year  = (UINT16) (Time->Year % 100);

  if (RegisterB.Bits.DM == 0) {
    Time->Year    = DecimalToBcd8 ((UINT8) Time->Year);
    Time->Month   = DecimalToBcd8 (Time->Month);
    Time->Day     = DecimalToBcd8 (Time->Day);
    Time->Hour    = DecimalToBcd8 (Time->Hour);
    Time->Minute  = DecimalToBcd8 (Time->Minute);
    Time->Second  = DecimalToBcd8 (Time->Second);
  }
  //
  // If we are in 12 hour mode and PM is set, then set bit 7 of the Hour field.
  //
  if (RegisterB.Bits.MIL == 0 && IsPM) {
    Time->Hour = (UINT8) (Time->Hour | 0x80);
  }
}

/**
 Check if it is a leap year.

 @param[in] Time   The time to be checked.

 @retval TRUE      It is a leap year.
 @retval FALSE     It is NOT a leap year.
**/
BOOLEAN
IsLeapYear (
  IN EFI_TIME   *Time
  )
{
  if (Time->Year % 4 == 0) {
    if (Time->Year % 100 == 0) {
      if (Time->Year % 400 == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;
  }
}

/**
 See if field Day of an EFI_TIME is correct.

 @param[in] Time   Its Day field is to be checked.

 @retval TRUE      Day field of Time is correct.
 @retval FALSE     Day field of Time is NOT correct.
**/
BOOLEAN
DayValid (
  IN  EFI_TIME  *Time
  )
{
  UINT8  DayOfMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  //
  // The validity of Time->Month field should be checked before
  //
  ASSERT (Time->Month >=1);
  ASSERT (Time->Month <=12);
  if (Time->Day < 1 ||
      Time->Day > DayOfMonth[Time->Month - 1] ||
      (Time->Month == 2 && (!IsLeapYear (Time) && Time->Day > 28))
      ) {
    return FALSE;
  }

  return TRUE;
}

/**
 See if all fields of a variable of EFI_TIME type is correct.

 @param[in] Time                The time to be checked.

 @retval EFI_SUCCESS            Time is a valid EFI_TIME variable.
 @retval EFI_INVALID_PARAMETER  Some fields of Time are not correct.
**/
EFI_STATUS
EfiTimeFieldsValid (
  IN EFI_TIME *Time
  )
{
  if (Time->Year < 1998 ||
      Time->Year > 2099 ||
      Time->Month < 1 ||
      Time->Month > 12 ||
      (!DayValid (Time)) ||
      Time->Hour > 23 ||
      Time->Minute > 59 ||
      Time->Second > 59 ||
      Time->Nanosecond > 999999999) {
    //
    // Skip Check TimeZone & Daylight, Control by OS.
    //
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EfiSmmGetTime (
  IN OUT EFI_TIME *Time
  )
{
  RTC_REGISTER_A  RegisterA;
  RTC_REGISTER_B  RegisterB;
  EFI_STATUS      Status = EFI_SUCCESS;
  UINT8           Century;
  UINTN           Index;
  UINTN           RtcComponentNum;
  RTC_COMPONENT   RtcComponent[] = {{RTC_ADDRESS_YEAR            , 0},
                                    {RTC_ADDRESS_MONTH           , 0},
                                    {RTC_ADDRESS_DAY_OF_THE_MONTH, 0},
                                    {RTC_ADDRESS_HOURS           , 0},
                                    {RTC_ADDRESS_MINUTES         , 0},
                                    {RTC_ADDRESS_SECONDS         , 0}
                                    };
  
  //
  // Read Register B
  //
  RegisterB.Data = RtcRead (RTC_ADDRESS_REGISTER_B);

  if (RegisterB.Bits.DM) { // Only support BCD mode
    return EFI_UNSUPPORTED;
  }
  
  //
  // In order to preventing RTC data reading error upon RTC updating due to interrupt/SMI caused delay,
  // making RTC update checking before every RTC data reading to make sure every RTC data corrected
  //
  RtcComponentNum = sizeof(RtcComponent) / sizeof(RTC_COMPONENT);
  for (Index = 0; Index < RtcComponentNum; Index++) {
    do {
      RegisterA.Data = RtcRead (RTC_ADDRESS_REGISTER_A);
    } while (RegisterA.Bits.Uip == 1); // Wait RTC Update in Process
    RtcComponent[Index].Data = RtcRead (RtcComponent[Index].Address);
  }
  
  Time->Year    = RtcComponent[0].Data;       
  Time->Month   = RtcComponent[1].Data;     
  Time->Day     = RtcComponent[2].Data;        
  Time->Hour    = RtcComponent[3].Data;       
  Time->Minute  = RtcComponent[4].Data;     
  Time->Second  = RtcComponent[5].Data; 
  
  ConvertRtcTimeToEfiTime (Time, RegisterB);
  
  Century = BcdToDecimal8 ((UINT8) (RtcRead (RTC_ADDRESS_CENTURY) & 0x7f));
  
  Time->Year = (UINT16) (Century * 100 + Time->Year);
  
  Status = EfiTimeFieldsValid (Time);

  return Status;
}


EFI_STATUS
EfiSmmSetWakeupTime (
  IN BOOLEAN    DateAlarmEnable,
  IN EFI_TIME   *Time
  )
{
  EFI_STATUS            Status;
  RTC_REGISTER_A        RegisterA;
  RTC_REGISTER_B        RegisterB;
  RTC_REGISTER_D        RegisterD;

  if (Time == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  Status = EfiTimeFieldsValid (Time);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Read Rregister C to clear pending RTC interrupts
  //
  RtcRead (RTC_ADDRESS_REGISTER_C);

  //
  // Read Register B, and inhibit updates of the RTC
  //
  RegisterB.Data      = RtcRead (RTC_ADDRESS_REGISTER_B);
  RegisterB.Bits.SET  = 1;
  RtcWrite (RTC_ADDRESS_REGISTER_B, RegisterB.Data);

  ConvertEfiTimeToRtcTime (Time, RegisterB);

  //
  // Set RTC alarm time
  //
  RtcWrite (RTC_ADDRESS_SECONDS_ALARM, Time->Second);
  RtcWrite (RTC_ADDRESS_MINUTES_ALARM, Time->Minute);
  RtcWrite (RTC_ADDRESS_HOURS_ALARM  , Time->Hour);

  //
  // Set Wake Date alarm
  //
  RegisterD.Data = RtcRead (RTC_ADDRESS_REGISTER_D);
  if (DateAlarmEnable) {
    RegisterD.Bits.AlarmDate = Time->Day;
  } else {
    RegisterD.Bits.AlarmDate = 0;
  }
  RtcWrite (RTC_ADDRESS_REGISTER_D, RegisterD.Data);

  //
  // Enable RTC alarm and allow updates of the RTC registers
  //
  RegisterB.Bits.AIE = 1;
  RegisterB.Bits.SET = 0;
  RtcWrite (RTC_ADDRESS_REGISTER_B, RegisterB.Data);

  //
  // Wait RTC Update
  //
  do {
    RegisterA.Data = RtcRead (RTC_ADDRESS_REGISTER_A);
  } while (RegisterA.Bits.Uip == 0); 
  
  return EFI_SUCCESS;
}
//[-end-170314-IB07400847-modify]//

