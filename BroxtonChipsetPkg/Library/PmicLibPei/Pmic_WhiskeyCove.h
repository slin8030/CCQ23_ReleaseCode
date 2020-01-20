/**@file
  Header file of WhiskeyCove PMIC registers.

@copyright
 Copyright (c) 2012 - 2016 Intel Corporation. All rights reserved
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

#ifndef _PMIC_REG_WC_PEI_H_
#define _PMIC_REG_WC_PEI_H_

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/I2CLib.h>
#include <Library/PmicLib.h>
#include <Library/TimerLib.h>
#include <Library/PcdLib.h>

#define DELAY_BETWEEN_INSTRUCTION_1 50
#define DELAY_BETWEEN_INSTRUCTION   10

///
/// WC PMIC Steppings
///
typedef enum {
  WcA0          = 0x00,
  WcA1,
  WcA2,
  WcA3,
  WcA4,
  WcA5,
  WcA6,
  WcA7,
  WcB0          = 0x10,
  WcB1,
  WcB2,
  WcB3,
  WcB4,
  WcB5,
  WcB6,
  WcB7,
  WcC0          = 0x20,
  WcC1,
  WcC2,
  WcC3,
  WcC4,
  WcC5,
  WcC6,
  WcC7,
  WcD0          = 0x30,
  WcD1,
  WcD2,
  WcD3,
  WcD4,
  WcD5,
  WcD6,
  WcD7,
  WcE0          = 0x40,
  WcE1,
  WcE2,
  WcE3,
  WcE4,
  WcE5,
  WcE6,
  WcE7,
  WcF0          = 0x50,
  WcF1,
  WcF2,
  WcF3,
  WcF4,
  WcF5,
  WcF6,
  WcF7,
  WcG0          = 0x60,
  WcG1,
  WcG2,
  WcG3,
  WcG4,
  WcG5,
  WcG6,
  WcG7,
  WcH0          = 0x70,
  WcH1,
  WcH2,
  WcH3,
  WcH4,
  WcH5,
  WcH6,
  WcH7,
  WcSteppingMax = 0xFF
} WCPMIC_STEPPING;

#define WC_PMIC_VID_REVISION_MAX 0x08
#define WC_PMIC_VID_MAJOR_REVISION_BITMAP 0x38
#define WC_PMIC_VID_MINOR_REVISION_BITMAP 0x07

typedef struct _WC_PMIC_CONFIGURATION {
  UINT8 Offset;
  UINT8 Value;
} WC_PMIC_CONFIGURATION;

/**
  Read charger's register provided by PMIC.

  @param[in]  Offset           - The charger's register to read.
  @param[out] Value            - The value read.

  @retval EFI_SUCCESS          - Read charger's register successfully.
  @retval Others               - Status depends on each PmicWrite8/PmicRead8 operation.
**/
EFI_STATUS
EFIAPI
WcPmicChargerRead (
  IN UINT8     Offset,
  OUT UINT8    *Value
  );

/**
  Write charger's register provided by PMIC.

  @param[in] Offset            - The charger's register to write.
  @param[in] Value             - The value written.

  @retval EFI_SUCCESS          - Write charger's register successfully.
  @retval Others               - Status depends on each PmicWrite8/PmicRead8 operation.
**/
EFI_STATUS
EFIAPI
WcPmicChargerWrite (
  IN UINT8    Offset,
  IN UINT8    Value
  );

#endif
