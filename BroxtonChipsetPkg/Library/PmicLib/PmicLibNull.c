/**@file
  Dxe Library for PMIC public API.

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
#include "Pmic_Private.h"


struct PMIC_object *g_pmic_obj = NULL;

extern struct PMIC_object dm_obj;


struct PMIC_object *g_pmic_support_list[] = {
  &dm_obj,  //dummy
};


PMIC_TYPE g_PMIC_type = PMIC_TYPE_NONE;   //global flag to record current probed PMIC type. We could use it to avoid probe HW every time in API.

/**
  Reads an 8-bit PMIC register.

  Reads the 8-bit PMIC register specified by Register.
  The 8-bit read value is returned.

  @param[in]  BaseAddress     - IPC operation address for target PMIC device.
  @param[in]  Register        - The PMIC register to read.

  @retval UINT8               - The value read.
**/
UINT8
EFIAPI
PmicRead8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register
  )
{
  RETURN_IF_POINTER_NULL(PmicRead8, 0xff)
  return g_pmic_obj->PmicRead8(BaseAddress, Register);
}

/**
  Writes an 8-bit PMIC register with a 8-bit value.

  Writes the 8-bit PMIC register specified by Register with the value specified
  by Value and return the operation status.

  @param[in]  BaseAddress     - IPC operation address for target PMIC device.
  @param[in]  Register        - The PMIC register to write.
  @param[in]  Value           - The value to write to the PMIC register.

  @retval EFI_SUCCESS         - Write bytes to PMIC device successfully
  @retval Others              - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicWrite8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register,
  IN UINT8                     Value
  )
{
  RETURN_IF_POINTER_NULL(PmicWrite8, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicWrite8(BaseAddress, Register, Value);
}

/**
  Get previous shutdown root cause.

  @param[in, out]  ShutdownCause - The data to store shutdown root cause.

  @retval EFI_SUCCESS            - Get shutdown root cause successfully.
  @retval Others                 - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicGetShutdownCause (
  IN OUT UINT32    *ShutdownCause
  )
{
  RETURN_IF_POINTER_NULL(PmicGetShutdownCause, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicGetShutdownCause(ShutdownCause);
}

/**
  Get previous reset root cause

  @param[in, out]  ResetSrc     - The data to store reset root cause

  @retval EFI_SUCCESS           - Get reset root cause successfully.
  @retval EFI_INVALID_PARAMETER - ResetSrc is NULL.
**/
EFI_STATUS
EFIAPI
PmicGetResetSrc (UINT32 *ResetSrc)
{
  RETURN_IF_POINTER_NULL(PmicGetResetSrc, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicGetResetSrc(ResetSrc);
}

/**
  Get the cause of system wake event.

  @param[in, out]  WakeCause   - The data to store the cause of wake event.

  @retval EFI_SUCCESS          - Get wake cause successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicGetWakeCause (
  IN OUT UINT32    *WakeCause
  )
{
  RETURN_IF_POINTER_NULL(PmicGetWakeCause, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicGetWakeCause(WakeCause);
}

/**
  Get power source detection result.

  @param[in, out]  PowerSrcIrq   - The data to the cause of wake event.

  @retval EFI_SUCCESS            - Get power source successfully.
  @retval Others                 - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicGetPwrSrcIrq (
  IN OUT UINT32    *PowerSrcIrq
  )
{
  RETURN_IF_POINTER_NULL(PmicGetPwrSrcIrq, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicGetPwrSrcIrq(PowerSrcIrq);
}

/**
  Battery Detection Status

  @retval TRUE                - Connected
  @retval FALSE               - Disconnected
**/
BOOLEAN
EFIAPI
PmicIsBatOn (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicIsBatOn, TRUE)
  return g_pmic_obj->PmicIsBatOn();
}

/**
  AC/DC Adapter Detection Status

  @retval TRUE                - Connected
  @retval FALSE               - Disconnected
**/
BOOLEAN
EFIAPI
PmicIsACOn (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicIsACOn, TRUE)
  return g_pmic_obj->PmicIsACOn();
}

/**
  VBUS Detection Status

  It can be used to detect whether USB charger is connected.

  @retval TRUE                - Connected
  @retval FALSE               - Disconnected
**/
BOOLEAN
EFIAPI
PmicVbusStatus (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicVbusStatus, TRUE)
  return g_pmic_obj->PmicVbusStatus();
}

/**
  USB ID Detection Status

  @retval UINT8               - Value depends on each PMIC operation.
**/
UINT8
EFIAPI
PmicUSBIDStatus (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicUSBIDStatus, TRUE)
  return g_pmic_obj->PmicUSBIDStatus();
}

/**
  Clear previous shutdown root cause.

  @retval EFI_SUCCESS          - Clear shutdown root cause successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicClearShutdownCause (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicClearShutdownCause, EFI_UNSUPPORTED)

  return g_pmic_obj->PmicClearShutdownCause();
}

/**
  Clear the cause of system wake event.

  @retval EFI_SUCCESS          - Clear wake cause successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicClearWakeCause (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicClearWakeCause, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicClearWakeCause();
}

/**
  Clear the cause of system reset.

  @retval EFI_SUCCESS          - Clear reset cause successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicClearResetSrc (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicClearResetSrc, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicClearResetSrc();
}

/**
  Initialize PMIC thermal detection capability.

  @retval EFI_SUCCESS          - Initialize thermal detection successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicThermInit (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicThermInit, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicThermInit();
}

/**
  Initialize PMIC GPIO pin.

  Initialize PMIC GPIO pin in order to get boardid/memcfgid/.. etc later

  @param[in] PlatformInfo      - Platform information with GPIO setting.

  @retval EFI_SUCCESS          - Initialize GPIO pin successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicGpioInit (
  IN VOID    *PlatformInfo
  )
{
  RETURN_IF_POINTER_NULL(PmicGpioInit, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicGpioInit(PlatformInfo);
}

/**
  Initializes PMIC device.

  @retval EFI_SUCCESS          - Initialize PMIC successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicIntrInit (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicIntrInit, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicIntrInit();
}

/**
  Initializes Burst Control Unit (BCU) hardware.

  The BCU has several different knobs (input stimuli configuration, interrupts,
  and output actions) that allow the Software and Firmware to change the BCU response
  behavior once these events occur. These BCU control knobs allow for a change in the
  BCU behavior in responding to these triggers and can be completely customized as
  necessary by the specific system implementation.

  @retval EFI_SUCCESS          - Initializes BCU hardware successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicBcuInit (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicBcuInit, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicBcuInit();
}

/**
  Initializes other miscellaneous functions on PMIC device.

  @retval EFI_SUCCESS          - Initializes miscellaneous functions successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicMiscInit (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicMiscInit, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicMiscInit ();
}

/**
  Turn on or off VBUS for OTG

  @param[in]  Enable           - TRUE : turn on VBUS
                                 FALSE: turn off VBUS

  @retval EFI_SUCCESS          - Turn on/off VBUS successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicVbusControl (
  IN BOOLEAN    Enable
  )
{
  RETURN_IF_POINTER_NULL(PmicVbusControl, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicVbusControl(Enable);
}

/**
  Turn on or off 5V VBUS for USB2/3 HOST

  @param[in]  Enable           - TRUE : turn on V5P0S
                                 FALSE: turn off V5P0S

  @retval EFI_SUCCESS          - Turn on/off V5P0S successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicVhostControl (
  IN BOOLEAN    Enable
  )
{
  RETURN_IF_POINTER_NULL(PmicVhostControl, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicVhostControl(Enable);
}

/**
  Get PMIC Vendor ID and Device ID

  @param[in, out] VenId        - Vendor ID
  @param[in, out] DevId        - Device ID

  @retval EFI_SUCCESS          - Get Vendor ID and Device ID successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicGetDevID (
  IN OUT UINT8    *VenId,
  IN OUT UINT8    *DevId
  )
{
  RETURN_IF_POINTER_NULL(PmicGetDevID, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicGetDevID(VenId, DevId);
}

/**
  This procedure will get PMIC Stepping

  @retval PMIC Stepping
**/
UINT8
EFIAPI
PmicStepping (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicStepping, 0xff);

  return g_pmic_obj->PmicStepping();
}

/**
  Get PMIC device instance

  @retval PMIC_TYPE            - Device type is defined in PMIC_TYPE.
**/
PMIC_TYPE
EFIAPI
PmicGetDeviceType (
  VOID
  )
{
  return g_pmic_obj->PmicType;
}

/**
  Get battery voltage.

  @retval UINT16               - ADC result for battery voltage.
**/
UINT16
EFIAPI
PmicGetVBAT (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicGetVBAT, 0xff)
  return g_pmic_obj->PmicGetVBAT();
}

/**
  Get battery capacity.

  @retval UINT16               - Remaining percentage of battery capacity.
**/
UINT16
EFIAPI
PmicGetBatteryCap (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicGetBatteryCap, 0xff)
  return g_pmic_obj->PmicGetBatteryCap();
}

/**
  Get power button status

  @retval TRUE                  - Power button pressed.
  @retval FALSE                 - Power button released.
**/
BOOLEAN
EFIAPI
PmicIsPwrBtnPressed (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicIsPwrBtnPressed, TRUE)

  return g_pmic_obj->PmicIsPwrBtnPressed();
}

/**
  Disable the capability to shutdown platform using power button.

  @param[out] ButtonHoldTime   - If the power button is disabled successfully,
                                 this contains the time in seconds need to hold power button to shutdown platform.

  @retval EFI_SUCCESS          - Succeed to disable power button
  @retval Others               - Failed to disable power button
**/
EFI_STATUS
EFIAPI
PmicDisablePowerButton (
  OUT UINT8    *ButtonHoldTime
  )
{
  RETURN_IF_POINTER_NULL(PmicDisablePowerButton, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicDisablePowerButton(ButtonHoldTime);
}

/**
  Enable the capability to shutdown platform using power button.

  @param[in] ButtonHoldTime    - Time in seconds to shut down the platform if power button is enabled and hold

  @retval EFI_SUCCESS          - Succeed to enable power button
  @retval Others               - Filed to enable power button
**/
EFI_STATUS
EFIAPI
PmicEnablePowerButton (
  IN UINT8    ButtonHoldTime
  )
{
  RETURN_IF_POINTER_NULL(PmicEnablePowerButton, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicEnablePowerButton(ButtonHoldTime);
}

/**
  Set VDDQ to 1.35V for DDR3L

  @retval EFI_SUCCESS          - Succeed to Set VDDQ
  @retval Others               - Filed to Set VDDQ
**/
EFI_STATUS
EFIAPI
PmicSetVDDQ (
  VOID
  )
{
  RETURN_IF_POINTER_NULL(PmicSetVDDQ, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicSetVDDQ();
}

/**
  Probe to find the correct PMIC object.

  After probling, g_pmic_obj points to correct PMIC object
  This routine is invoked when library is loaded .

  @retval TRUE                - There is one PMIC object found.
  @retval FALSE               - No PMIC object found.
**/
BOOLEAN
EFIAPI
PmicProbe (
  VOID
  )
{
  g_pmic_obj = &dm_obj;
  return FALSE;
}

/**
  Read charger's register provided by PMIC.

  @param[in]  Offset           - The charger's register to read.
  @param[out] Value            - The value read.

  @retval EFI_SUCCESS          - Read charger's register successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicChargerRead (
  IN  UINT8     Offset,
  OUT UINT8     *Value
  )
{
  RETURN_IF_POINTER_NULL(PmicChargerRead, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicChargerRead(Offset, Value);
}

/**
  Write charger's register provided by PMIC.

  @param[in] Offset            - The charger's register to write.
  @param[in] Value             - The value written.

  @retval EFI_SUCCESS          - Write charger's register successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicChargerWrite (
  IN UINT8    Offset,
  IN UINT8    Value
  )
{
  RETURN_IF_POINTER_NULL(PmicChargerWrite, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicChargerWrite(Offset, Value);
}

/**
  Detect charger type and charger presence.

  @param[out] ChargerPresent   - TRUE : Charger present.
                                 FALSE: Charger not present.
  @param[out] ChargerType      - Charger type - SDP/DCP/CDP... etc.

  @retval EFI_SUCCESS          - Detect charger type and charger presence successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicDetectCharger (
  OUT BOOLEAN    *ChargerPresent,
  OUT UINT8      *ChargerType
  )
{
  RETURN_IF_POINTER_NULL(PmicDetectCharger, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicDetectCharger(ChargerPresent, ChargerType);
}

/**
  Controls external USB PHY reset

  @param[in] Enable            - TRUE : OUTOFRESET - external PHY is out of reset
                                 FALSE: INRESET - external PHY is in reset

  @retval EFI_SUCCESS          - Controls external USB PHY successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
PmicUSBSwitchControl (
  IN BOOLEAN    Enable
  )
{
  RETURN_IF_POINTER_NULL(PmicUSBSwitchControl, EFI_UNSUPPORTED)
  return g_pmic_obj->PmicUSBSwitchControl(Enable);
}

/**
  Pmic library constructor

  PMIC type is probed in this function. It is invoked every time Pmiclib is loaded.

  @retval  EFI_SUCCESS         - Construct PMIC instance successfully.
**/
RETURN_STATUS
EFIAPI
PmicLibConstructor (
  VOID
  )
{
  PmicProbe();
  return EFI_SUCCESS;
}
