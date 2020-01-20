/** @file
  Pei library for accessing dummy PMIC registers.

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

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/PmicLib.h>
#include "Pmic_Private.h"

/**
  Reads an 8-bit PMIC register.

  Reads the 8-bit PMIC register specified by Register.
  The 8-bit read value is returned.

  @param[in]  BaseAddress     - IPC operation address for target PMIC device.
  @param[in]  Register        - The PMIC register to read.

  @retval 0                   - Function not supported yet.
**/
UINT8
EFIAPI
DmPmicRead8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register
  )
{
  return 0x0;
}

/**
  Writes an 8-bit PMIC register with a 8-bit value.

  Writes the 8-bit PMIC register specified by Register with the value specified
  by Value and return the operation status.

  @param[in]  BaseAddress     - IPC operation address for target PMIC device.
  @param[in]  Register        - The PMIC register to write.
  @param[in]  Value           - The value to write to the PMIC register.

  @retval EFI_SUCCESS         - Function not supported yet.
**/
EFI_STATUS
EFIAPI
DmPmicWrite8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register,
  IN UINT8                     Value
  )
{
  return EFI_SUCCESS;
}

/**
  AC/DC Adapter Detection Status

  @retval TRUE                - Connected
**/
BOOLEAN
EFIAPI
DmPmicIsACOn (
  VOID
  )
{
  return TRUE;
}

/**
  Probe to find the correct PMIC object.

  After probling, g_pmic_obj points to correct PMIC object
  This routine is invoked when library is loaded .

  @retval TRUE                - Always return true to indicate dummy PMIC existed.
**/
BOOLEAN
EFIAPI
DmPmicProbe (
  VOID
  )
{
  DEBUG ((DEBUG_ERROR, "Dummy PMIC detected\n"));
  return TRUE;
}

PMIC_object dm_obj = {
  PMIC_I2C_BUSNO,
  PMIC_PAGE_0_I2C_ADDR,
  PMIC_PAGE_0_I2C_ADDR,
  PMIC_TYPE_DUMMY,
  DmPmicRead8,
  DmPmicWrite8,

  DmPmicProbe,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  //PUPDR interfaces
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  DmPmicIsACOn,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};
