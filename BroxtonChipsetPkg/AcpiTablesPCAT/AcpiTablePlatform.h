/** @file
  This file describes the contents of the ACPI Fixed ACPI Description Table
  (FADT).

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _Platform_h_INCLUDED_
#define _Platform_h_INCLUDED_

#include <IndustryStandard/Acpi.h>
//
// ACPI table information used to initialize tables.
//
#define EFI_ACPI_OEM_ID           'I','N','T','E','L',' '   // OEMID 6 bytes long
#define EFI_ACPI_OEM_TABLE_ID     SIGNATURE_64('L','A','N','F','O','R','D','C') // OEM table id 8 bytes long
#define EFI_ACPI_OEM_REVISION     0x00000005
#define EFI_ACPI_CREATOR_ID       SIGNATURE_32('M','S','F','T')
#define EFI_ACPI_CREATOR_REVISION 0x0100000D
#define INT_MODEL       0x01
#define PM_PROFILE      EFI_ACPI_4_0_PM_PROFILE_MOBILE
#define SCI_INT_VECTOR  0x0009
#define SMI_CMD_IO_PORT 0x000000B2
#define ACPI_ENABLE     0x0A0
#define ACPI_DISABLE    0x0A1
#define S4BIOS_REQ      0x00
#define PSTATE_CNT      0x00
#define PM1a_EVT_BLK    0x400
#define PM1b_EVT_BLK    0x00000000
#define PM1a_CNT_BLK    0x404
#define PM1b_CNT_BLK    0x00000000
#define PM2_CNT_BLK     0x450
#define PM_TMR_BLK      0x408
#define GPE0_BLK        0x420
#define GPE1_BLK        0x00000000
#define PM1_EVT_LEN     0x04
#define PM1_CNT_LEN     0x02
#define PM2_CNT_LEN     0x01
#define PM_TM_LEN       0x04
#define GPE0_BLK_LEN    0x20
#define GPE1_BLK_LEN    0x00
#define GPE1_BASE       0x00
#define CST_CNT         0x00
#define P_LVL2_LAT      0x0032
#define P_LVL3_LAT      0x0096
#define FLUSH_SIZE      0x0400
#define FLUSH_STRIDE    0x0010
#define DUTY_OFFSET     0x01
#define DUTY_WIDTH      0x03
#define DAY_ALRM        0x0D
#define MON_ALRM        0x00
#define CENTURY         0x32
//[-start-180824-IB07400999-modify]//
#define FLAG            ( EFI_ACPI_5_0_WBINVD | EFI_ACPI_5_0_SLP_BUTTON | EFI_ACPI_5_0_RESET_REG_SUP | EFI_ACPI_5_0_RTC_S4 | EFI_ACPI_5_0_TMR_VAL_EXT)
//[-end-180824-IB07400999-modify]//
#define IAPC_BOOT_ARCH  ( EFI_ACPI_5_0_8042 | EFI_ACPI_5_0_LEGACY_DEVICES )
#define RESERVED        0x00

#endif
