/** @file

  Header file of EventLogGetTimeFromCmos Lib implementation.

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


#ifndef _EVENT_LOG_GET_TIME_FROM_CMOS_H_
#define _EVENT_LOG_GET_TIME_FROM_CMOS_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/CmosLib.h>
#include <Library/TimerLib.h>
#include <Library/EventLogGetTimeFromCmosLib.h>

#define CMOS_YEAR            0x09
#define CMOS_MONTH           0x08
#define CMOS_DAY             0x07
#define CMOS_HOUR            0x04
#define CMOS_MINUTE          0x02
#define CMOS_SECOND          0x00

#define CMOS_REGISTER_A      0x0A
#define CMOS_REGISTER_B      0x0B

//[-start-170413-IB08400443-add]//
//
// The maximum retry for RTC reading
//
#define MAX_RETRY            10000
//[-end-170413-IB08400443-add]//

#pragma pack(1)

typedef struct {
  UINT8  Address; ///< RTC offset address.
  UINT8  Data;    ///< Corresponding data value.
} RTC_COMPONENT;

//
// Register A
//
typedef struct {
  UINT8 Rs : 4;   ///< Rate Selection Bits
  UINT8 Dv : 3;   ///< Divisor
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
  UINT8 Dse : 1;  ///< 0 - Daylight saving disabled  1 - Daylight savings enabled
  UINT8 Mil : 1;  ///< 0 - 12 hour mode              1 - 24 hour mode
  UINT8 Dm : 1;   ///< 0 - BCD Format                1 - Binary Format
  UINT8 Sqwe : 1; ///< 0 - Disable SQWE output       1 - Enable SQWE output
  UINT8 Uie : 1;  ///< 0 - Update INT disabled       1 - Update INT enabled
  UINT8 Aie : 1;  ///< 0 - Alarm INT disabled        1 - Alarm INT Enabled
  UINT8 Pie : 1;  ///< 0 - Periodic INT disabled     1 - Periodic INT Enabled
  UINT8 Set : 1;  ///< 0 - Normal operation.         1 - Updates inhibited
} RTC_REGISTER_B_BITS;

typedef union {
  RTC_REGISTER_B_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_B;

#pragma pack()


#endif
