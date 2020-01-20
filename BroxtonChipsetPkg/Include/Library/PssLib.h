/** @file
  PSS(Processor Secured Storage) Chip Operation Library Header

@copyright
  Copyright (c) 1999 - 2015 Intel Corporation. All rights reserved
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

#ifndef __PSS_LIB_H__
#define __PSS_LIB_H__

//
// Broxton Platform PSS I2C Interface
//
#define PSS_I2C_BUS_NUM            0x6
#define PSS_I2C_SLAVE_ADDR         0x6E

//
// Monza X-8K PSS Chip Memory Map
//
#define MONZAX_SIZE_RESERVED       22
#define MONZAX_SIZE_EPC            18
#define MONZAX_SIZE_TID            24
#define MONZAX_SIZE_USER           1024

#define PSS_BASE_ADDR_RESERVED     0x00
#define PSS_BASE_ADDR_EPC          0x16
#define PSS_BASE_ADDR_TID          0x28
#define PSS_BASE_ADDR_CLASSID      0x28
#define PSS_BASE_ADDR_USER         0x40
#define PSS_BASE_ADDR_MAX          0x43F  // 0 ~ 0x43F

#define PSS_CHIP_TID_LENGTH        12

//
// Intel PSS Chip Memory Layout Definition - Start
// Base address is BASE_ADDRESS_USER(0x40)
//
#define PSS_CHIP_BLOCK_0_START                  (0)
#define PSS_CHIP_BLOCK_1_START                  (PSS_CHIP_BLOCK_0_START + 64)  // 0x40
#define PSS_CHIP_BLOCK_2_START                  (PSS_CHIP_BLOCK_1_START + 64)  // 0x80
#define PSS_CHIP_BLOCK_3_START                  (PSS_CHIP_BLOCK_2_START + 64)  // 0xC0
#define PSS_CHIP_BLOCK_4_START                  (PSS_CHIP_BLOCK_3_START + 64)  // 0x100

#define PSS_CHIP_REWORK_OFFSET                  (PSS_CHIP_BLOCK_0_START)                                                       // 0x00
#define PSS_CHIP_REWORK_LENGTH                  (12) // Bytes
#define PSS_CHIP_STATUS_OFFSET                  (PSS_CHIP_REWORK_OFFSET + PSS_CHIP_REWORK_LENGTH)                              // 0x0C
#define PSS_CHIP_STATUS_LENGTH                  (12)
#define PSS_CHIP_LAST_PSS_UPDATE_OFFSET         (PSS_CHIP_STATUS_OFFSET + PSS_CHIP_STATUS_LENGTH)                              // 0x18
#define PSS_CHIP_LAST_PSS_UPDATE_LENGTH         (10)
#define PSS_CHIP_PMIC_VERSION_OFFSET            (PSS_CHIP_LAST_PSS_UPDATE_OFFSET + PSS_CHIP_LAST_PSS_UPDATE_LENGTH)            // 0x22
#define PSS_CHIP_PMIC_VERSION_LENGTH            (6)
#define PSS_CHIP_ASSIGNED_USER_OFFSET           (PSS_CHIP_PMIC_VERSION_OFFSET + PSS_CHIP_PMIC_VERSION_LENGTH)                  // 0x28
#define PSS_CHIP_ASSIGNED_USER_LENGTH           (8)
#define PSS_CHIP_LOCATION_OFFSET                (PSS_CHIP_ASSIGNED_USER_OFFSET + PSS_CHIP_ASSIGNED_USER_LENGTH)                // 0x30
#define PSS_CHIP_LOCATION_LENGTH                (16)

#define PSS_CHIP_BOARD_SERIAL_NUMBER_OFFSET     (PSS_CHIP_BLOCK_1_START)                                                       // 0x40
#define PSS_CHIP_BOARD_SERIAL_NUMBER_LENGTH     (16)
#define PSS_CHIP_WORK_ORDER_NUMBER_OFFSET       (PSS_CHIP_BOARD_SERIAL_NUMBER_OFFSET + PSS_CHIP_BOARD_SERIAL_NUMBER_LENGTH)    // 0x50
#define PSS_CHIP_WORK_ORDER_NUMBER_LENGTH       (16)
#define PSS_CHIP_INTEL_PART_NUMBER_OFFSET       (PSS_CHIP_WORK_ORDER_NUMBER_OFFSET + PSS_CHIP_WORK_ORDER_NUMBER_LENGTH)        // 0x60
#define PSS_CHIP_INTEL_PART_NUMBER_LENGTH       (12)
#define PSS_CHIP_PRODUCT_FAMILY_OFFSET          (PSS_CHIP_INTEL_PART_NUMBER_OFFSET + PSS_CHIP_INTEL_PART_NUMBER_LENGTH)        // 0x6C
#define PSS_CHIP_PRODUCT_FAMILY_LENGTH          (20)

#define PSS_CHIP_LAST_RFID_SYNC_OFFSET          (PSS_CHIP_BLOCK_2_START)                                                       // 0x80
#define PSS_CHIP_LAST_RFID_SYNC_LENGTH          (10)
#define PSS_CHIP_DATE_SHIPPED_OFFSET            (PSS_CHIP_LAST_RFID_SYNC_OFFSET + PSS_CHIP_LAST_RFID_SYNC_LENGTH)              // 0x8A
#define PSS_CHIP_DATE_SHIPPED_LENGTH            (10)
#define PSS_CHIP_SPECIAL_INSTRUCTIONS_OFFSET    (PSS_CHIP_DATE_SHIPPED_OFFSET + PSS_CHIP_DATE_SHIPPED_LENGTH)                  // 0x94
#define PSS_CHIP_SPECIAL_INSTRUCTIONS_LENGTH    (32)
#define PSS_CHIP_RECALL_OFFSET                  (PSS_CHIP_SPECIAL_INSTRUCTIONS_OFFSET + PSS_CHIP_SPECIAL_INSTRUCTIONS_LENGTH)  // 0xB4
#define PSS_CHIP_RECALL_LENGTH                  (1)
#define PSS_CHIP_CONSUMPTION_METER_OFFSET       (PSS_CHIP_RECALL_OFFSET + PSS_CHIP_RECALL_LENGTH)                              // 0xB5
#define PSS_CHIP_CONSUMPTION_METER_LENGTH       (7)
#define PSS_CHIP_BOOT_COUNT_OFFSET              (PSS_CHIP_CONSUMPTION_METER_OFFSET + PSS_CHIP_CONSUMPTION_METER_LENGTH)        // 0xBC
#define PSS_CHIP_BOOT_COUNT_LENGTH              (4)

#define PSS_CHIP_SOC_VERSION_OFFSET             (PSS_CHIP_BLOCK_3_START)                                                       // 0xC0
#define PSS_CHIP_SOC_VERSION_LENGTH             (8)
#define PSS_CHIP_BKC_VERSION_OFFSET             (PSS_CHIP_SOC_VERSION_OFFSET + PSS_CHIP_SOC_VERSION_LENGTH)                    // 0xC8
#define PSS_CHIP_BKC_VERSION_LENGTH             (16)
#define PSS_CHIP_OS_OFFSET                      (PSS_CHIP_BKC_VERSION_OFFSET + PSS_CHIP_BKC_VERSION_LENGTH)                    // 0xD8
#define PSS_CHIP_OS_LENGTH                      (16)
#define PSS_CHIP_IFWI_OFFSET                    (PSS_CHIP_OS_OFFSET + PSS_CHIP_OS_LENGTH)                                      // 0xE8
#define PSS_CHIP_IFWI_LENGTH                    (24)

#define PSS_CHIP_LATITUDE_OFFSET                (PSS_CHIP_BLOCK_4_START)                                                       // 0x100
#define PSS_CHIP_LATITUDE_LENGTH                (8)
#define PSS_CHIP_LONGITUDE_OFFSET               (PSS_CHIP_LATITUDE_OFFSET + PSS_CHIP_LATITUDE_LENGTH)                          // 0x108
#define PSS_CHIP_LONGITUDE_LENGTH               (8)
//
// Intel PSS Chip Memory Layout Definition - End
//

#define IsLeap(y)   (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define SECSPERMIN  (60)
#define SECSPERHOUR (60 * 60)
#define SECSPERDAY  (24 * SECSPERHOUR)

#pragma pack (1)
typedef struct _PSS_CHIP_STRING
{
  UINT16        Offset;
  UINT8         Length;
  EFI_STRING_ID StringToken;
}PSS_CHIP_STRING;
#pragma pack ()

extern UINTN CumulativeDays[2][14];
extern CHAR8 *WeekDayName[];
extern CHAR8 *MonthName[];

/**
  Read data from PSS chip.

  @param[out]  Buffer
  @param[in]   Address
  @param[in]   Size

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
LibReadPssData (
  UINT8     *Buffer,
  UINT32    Address,
  UINT32    Size
  );

/**
  Write data to PSS chip.

  @param[in]  Buffer
  @param[in]  Address
  @param[in]  Size

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
LibWritePssData (
  UINT8     *Buffer,
  UINT32    Address,
  UINTN     Size
  );

/**
  Check PSS chip status.

  @param[in]  none

  @retval  EFI_STATUS
**/
EFI_STATUS
EFIAPI
LibPssDetect (
  VOID
  );

#endif
