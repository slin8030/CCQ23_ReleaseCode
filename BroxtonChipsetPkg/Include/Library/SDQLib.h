/** @file
  Register Definitions for SDQ Library.

@copyright
 Copyright (c) 1999 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _SDQ_LIB_H_
#define _SDQ_LIB_H_

#include <Uefi.h>

#define SDQ_DEBUG 1

#define GPIO23_GPIO_PAD_DW0 0x40000100 //this is IN so Output buffer is disabled
#define GPIO23_GPIO_PAD_DW1 0x00003000 //it is input so it should have pull-up high

#define GPIO24_GPIO_PAD_DW0 0x40000201 //this is OUT so Input buffer is disabled
#define GPIO24_GPIO_PAD_DW1 0x00003000 //it is output so pull high

#define GPIO0P2CTLO 0x46
#define GPIO0P3CTLO 0x47

#define PSDETCTRL_REG    0x23
#define BATTDETCTRL0_REG 0x24

#define GPIO0P2CTLI  0x53
#define GPIO0P3CTLI  0x54

#define LOW 0
#define HIGH 1
#define HIZ 2

//
// bq2022A EPROM Timing parameters
//
#define RST_TIME           500         ///< Reset Time (>= 480us)
#define PP_MIN_TIME        60          ///< Minimum Presence Pulse Time
#define PP_MAX_TIME        240         ///< Maximum Presence Pulse Time
#define RSTREC_TIME        500         ///< Recovery Time after Reset(>= 480us )
#define WSTRB_TIME         5           ///< Write Start Cycle (1us - 15us)
#define WDH_TIME           100         ///< Write Data Hold Time (>=60us )
#define REC_TIME           15         ///< Recovery Time for memory command only(>=5us )
#define RSTRB_TIME         5           ///< Read Start Cycle (1us - 13us)
#define ODD_TIME           13          ///< Output Data Delay Time (RSTRB_TIME - 13us)
#define ODHO_TIME          60          ///< Output Data Hold Time (17us - 60us)

//
// bq2022A EPROM commands
//
#define READ_ROM        0x33
#define READ_MEMORY     0xF0

//
// bq2022A EPROM data
//
#define SDQ_EPROM_SIGNATURE   0x42544E49
#define SDQ_EPROM_REVISION1   0x10
#define SDQ_EPROM_REVISION2   0x05

#pragma pack(1)
typedef struct _TEMP_MONITOR_RANGE {
  INT16  TempUL;
  UINT16 FullChgVol;
  UINT16 FullChgCur;
  UINT16 MaintChgStartTVol;
  UINT16 MaintChgStopVol;
  UINT16 MaintChgCur;
} TEMP_MONITOR_RANGE;
#pragma pack()

#pragma pack(1)
typedef struct _BATT_CHG_PROFILE {
  UINT16              BattClass;                      ///< Battery class
  UINT8               BattUniqNum[6];                 ///< Battery unique number
  UINT8               battTurboChrgSupport;           ///< Turbo charging support, in multiples of 100m
  UINT8               BattType;                       ///< Enumerated battery type (2 = LiIon)
  UINT16              BattCap;                        ///< Battery capacity in mAh
  UINT16              BattMaxVoltage;                 ///< Maximum battery Voltage in mV
  UINT16              BattFullChgCur;                 ///< Battery current (in mA) used to determine the Charge Full condition
  UINT16              BattShutdownlowVol;             ///< Battery voltage (in mV) used to determine the Charge Full condition
  UINT8               BattSafeDischgTempUL;           ///< Upper safe limit of the battery temp when discharging (degree C)
  UINT8               BattSafeDischgTempLL;           ///< Lower safe limit of the battery temp when discharging (degree C)
  UINT8               BattTempMonitorRangeCnt;        ///< Temperature monitor range count
  TEMP_MONITOR_RANGE  BatttempMonitorRange[6];        ///< Temperature monitor range
  INT16               BattTempLL;                     ///< Temp lower limit (degree C)
} BATT_CHG_PROFILE;                                   ///< Battery SDQ EEPROM offset 14 -109  data match SFI OEM0 ACPI table offset 36 - 131
#pragma pack()

#pragma pack(1)
typedef struct _SDQ_EPROM {
  UINT32             Signature;
  UINT8              Revision;
  UINT16             Size;
  UINT8              Rsvd[7];
  BATT_CHG_PROFILE   ChgPrfile;
  UINT8              Rsvd1; // reserved to match 112 bytes size of EEPROM data
  UINT16             Chksum;
} SDQ_EPROM;
#pragma pack()

/**
  Read charging profile by SDQ interface

  @param[in]  Checksum         - To indicate the checksum value needed to be verified
  @param[out] Profile          - Battery/Charger profile read from eprom

  @retval True                 - Read successfully
  @retval False                - Read failed
**/
BOOLEAN
GetBattChargingProfile (
  IN BOOLEAN              Checksum,
  OUT BATT_CHG_PROFILE    *Profile
  );

#endif /* _SDQ_LIB_H_ */
