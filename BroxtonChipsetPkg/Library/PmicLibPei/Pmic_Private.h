/**@file
  Private Header file for PMIC Pei Driver

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

#ifndef _PMIC_REG_PV_PEI_H_
#define _PMIC_REG_PV_PEI_H_

#include <Uefi/UefiBaseType.h>
#include <Base.h>
#include <Library/PmicLib.h>
#include <Library/PupdrPlatformLib.h>   //PUPDR definition

///
/// PMIC device object
///
typedef struct _PMIC_object {
  UINT8      I2cbus;
  UINT8      I2cSlaveAddr0;     ///< page0
  UINT8      I2cSlaveAddr1;     ///< page1
  PMIC_TYPE  PmicType;
  UINT8      (EFIAPI* PmicRead8)              (IN UINT8 BaseAddress, IN UINT8 Register);
  EFI_STATUS (EFIAPI* PmicWrite8)             (IN UINT8 BaseAddress, IN UINT8 Register, IN UINT8 Value);

  BOOLEAN    (EFIAPI* PmicProbe)              (VOID);
  EFI_STATUS (EFIAPI* PmicGetDevID)           (IN OUT UINT8 *VenId, IN OUT UINT8 *DevId);
  UINT8      (EFIAPI* PmicStepping)           (VOID);
  EFI_STATUS (EFIAPI* PmicThermInit)          (VOID);
  EFI_STATUS (EFIAPI* PmicGpioInit)           (IN VOID *PlatformInfo);
  EFI_STATUS (EFIAPI* PmicIntrInit)           (VOID);
  EFI_STATUS (EFIAPI* PmicBcuInit)            (VOID);
  EFI_STATUS (EFIAPI* PmicMiscInit)           (VOID);
  EFI_STATUS (EFIAPI* PmicVbusControl)        (IN BOOLEAN Enable);
  EFI_STATUS (EFIAPI* PmicVhostControl)       (IN BOOLEAN Enable);
  EFI_STATUS (EFIAPI* PmicBatchRegisterInit)  (IN RegInit_st *RegInit, IN UINT32 length);

  ///
  /// PUPDR interfaces
  ///
  EFI_STATUS (EFIAPI* PmicGetWakeCause)       (IN OUT UINT32 *WakeCause);
  EFI_STATUS (EFIAPI* PmicGetShutdownCause)   (IN OUT UINT32 *ShutdownCause);
  EFI_STATUS (EFIAPI* PmicGetResetSrc)        (IN OUT UINT32 *ResetSrc);
  EFI_STATUS (EFIAPI* PmicGetPwrSrcIrq)       (IN OUT UINT32 *PowerSrcIrq);
  BOOLEAN    (EFIAPI* PmicIsBatOn)            (VOID);
  BOOLEAN    (EFIAPI* PmicIsACOn)             (VOID);
  BOOLEAN    (EFIAPI* PmicVbusStatus)         (VOID);
  EFI_STATUS (EFIAPI* PmicClearShutdownCause) (VOID);
  EFI_STATUS (EFIAPI* PmicClearWakeCause)     (VOID);
  EFI_STATUS (EFIAPI* PmicClearResetSrc)      (VOID);
  UINT16     (EFIAPI* PmicGetVBAT)            (VOID);
  BOOLEAN    (EFIAPI* PmicIsPwrBtnPressed)    (VOID);
  EFI_STATUS (EFIAPI* PmicDisablePowerButton) (OUT UINT8 *ButtonHoldTime);
  EFI_STATUS (EFIAPI* PmicEnablePowerButton)  (IN UINT8  ButtonHoldTime);
  UINT16     (EFIAPI* PmicGetBatteryCap)      (VOID);
  EFI_STATUS (EFIAPI* PmicSetVDDQ)            (VOID);
  EFI_STATUS (EFIAPI* PmicChargerRead)        (IN UINT8 Offset, OUT UINT8 *Value);
  EFI_STATUS (EFIAPI* PmicChargerWrite)       (IN UINT8 Offset, IN UINT8 Value);
  EFI_STATUS (EFIAPI* PmicDetectCharger)      (OUT BOOLEAN *ChargerPresent, OUT UINT8 *ChargerType);
  EFI_STATUS (EFIAPI* PmicUSBSwitchControl)   (IN BOOLEAN Enable);
  UINT8      (EFIAPI* PmicUSBIDStatus)        (VOID);
}PMIC_object;

#define CHECK_POINTER(method)   { if ((NULL == ((PMIC_object*)GetPmicInstance())->method) || (NULL == ((PMIC_object*)GetPmicInstance()))) \
                                  {\
                                     DEBUG ((DEBUG_ERROR, "null function pointer\n")); \
                                     return EFI_UNSUPPORTED;\
                                  }}

#define CHECK_POINTER_RETURN_INT(method)   { if ((NULL == ((PMIC_object *)GetPmicInstance())->method) || (NULL == ((PMIC_object *)GetPmicInstance()))) \
                                  {\
                                     return 0xff;\
                                  }}


#define CHECK_POINTER_RETURN_BOOL(method)   { if ((NULL == ((PMIC_object*)GetPmicInstance())->method) || (NULL == ((PMIC_object*)GetPmicInstance()))) \
                                  {\
                                     return TRUE;\
                                  }}
///
/// This macro is defined to isolate the these APIs that only specific PMIC support.
///
#define CHECK_PMIC_TYPE(type)    { if (((PMIC_object*)GetPmicInstance())->PmicType != type) { \
                                     DEBUG ((DEBUG_ERROR, "Current PMIC does not provide this API. Re-check your code!!.\n")); \
                                     return EFI_UNSUPPORTED; \
                                    }\
                                 }

#define CHECK_PMIC_TYPE_RETURN_INT(type)    { if (((PMIC_object*)GetPmicInstance())->PmicType != type) { \
                                                 DEBUG ((DEBUG_ERROR, "Current PMIC does not provide this API. Re-check your code!!.\n")); \
                                                 return 0xFF; \
                                                }\
                                             }

#define CHECK_PMIC_TYPE_RETURN_BOOL(type)    { if (((PMIC_object*)GetPmicInstance())->PmicType != type) { \
                                                 DEBUG ((DEBUG_ERROR, "Current PMIC does not provide this API. Re-check your code!!.\n")); \
                                                 return TRUE; \
                                                }\
                                             }

/**
  To get the correct PMIC object.

  @retval PMIC_object         - The correct PMIC object.
**/
PMIC_object*
GetPmicInstance (
  VOID
  );

#endif
