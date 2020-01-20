/** @file
  Battery and Energe Management Table.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1996 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#ifndef _BEMT_H
#define _BEMT_H

///
/// Statements that include other files
///
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>

#define EFI_ACPI_OEM0_REVISION                      0x05
#define EFI_ACPI_OEM0_SIGNATURE                         SIGNATURE_32('O', 'E', 'M', '0')

#define EFI_ACPI_OEM_ID           'I','N','T','E','L',' '   ///< OEMID 6 bytes long
#define EFI_ACPI_OEM0_TABLE_ID     SIGNATURE_64('B','A','T','T','E','R','Y',' ') ///< OEM table id 8 bytes long
#define CREATOR_ID       SIGNATURE_32('I','N','T','L')
#define EFI_FIX_BATTARY_ID        SIGNATURE_64('I','N','T','N','0','0','0','1')
#define ACPI_TEMP_RANGES_NUMBER    6

#define EFI_ACPI_BAT_CONF_SIGNATURE    SIGNATURE_32 ('B', 'C', 'F', 'G')
#define EFI_ACPI_BAT_CONF_REVISION     0x01
#define EFI_ACPI_BAT_CONF_TABLE_ID     SIGNATURE_64 ('B', 'A', 'T', 'T', 'C', 'O', 'N', 'F') ///< OEM table id 8 bytes long
#define INVALID_BATTARY_ID             0xFFFFFFFFFFFFFFFF

#pragma pack(1)

typedef struct {
  UINT8  ClassID;
  UINT32 Dummy;
  UINT8  Tolerance;
  UINT16 BattID;   ///<BattID in Ohm from PMIC
}BATTERY_ID;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT64    BatteryId;
  UINT8     TurboChargingSupport;
  UINT8     BatteryType;
  UINT16    Capacity;
  UINT16    MaxVoltage;
  UINT16    ChargeTermiationCurrent;
  UINT16    LowBatteryThreshold;
  INT8      SafeDischargeTemperatureUL;
  INT8      SafeDischargeTemperatureLL;
  UINT16    TempMonitoringRanges;
  struct {
    INT16   TempUL;
    UINT16  FullChargeVolt;
    UINT16  FullChargeCurr;
    UINT16  MaintChargeStartThreshVolt;
    UINT16  MaintChargeStopThreshVolt;
    UINT16  MaintChargeCurr;
  } TempMonitorRanges [ACPI_TEMP_RANGES_NUMBER];
  INT16  TempLL;
} EFI_ACPI_OEM0_TABLE;

///
/// Battery Config ACPI table structure
///
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT64                      FG_Name;
  UINT64                      BatteryId;
  UINT16                      Size;
  UINT8                       FG_ConfigOption;
  UINT16                      PrimaryChecksum;
  ///
  /// Use the maximum fuel gauge config data size.
  /// MAXIM       - 200 Bytes
  /// XPOWER      - 36  Bytes
  /// $Cove SW FG - 256 Bytes
  /// TI BQ       - Factory Programmed into the chip
  ///
  UINT8                       ConfigData[256];
} EFI_ACPI_BAT_CONF_TABLE;

///
/// Battery Config Variable structure
///
typedef struct {
  UINT64                      FG_Name;
  UINT16                      Size;
  UINT16                      Checksum;        ///< Checksum for variable config data
  UINT16                      PrimaryChecksum; ///< Primary config data checksum
  ///
  /// Use the maximum fuel gauge config data size.
  /// MAXIM       - 200 Bytes
  /// XPOWER      - 36  Bytes
  /// $Cove SW FG - 256 Bytes
  /// TI BQ       - Factory Programmed into the chip
  ///
  UINT8                       ConfigData[256];
} FUEL_GAUGE_VARIABLE;

#pragma pack()


#endif
