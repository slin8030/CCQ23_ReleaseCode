/**@file
  Pei Library for PMIC public API.

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

//#include <PupdrDxe.h>   //PUPDR definition
#include <Uefi.h>
#include <Library/DebugLib.h>
#include "Pmic_Private.h"
#include "PmicLib.h"

extern PMIC_object dm_obj;


PMIC_object *g_pmic_support_list[] = {
  &dm_obj,  //dummy 
};


/**
  To get the correct PMIC object. 

  @retval PMIC_object         - The correct PMIC object.
**/
PMIC_object*
GetPmicInstance (
  VOID
  )
{
  UINTN i=0;
  UINTN num = sizeof (g_pmic_support_list) / sizeof(PMIC_object *) ;
  PMIC_object *g_pmic_obj=NULL;
  
  if (g_pmic_obj == NULL){
    for (;i<num; i++) {
      g_pmic_obj = g_pmic_support_list[i];
      if (NULL != g_pmic_obj->PmicProbe)  {
        if(TRUE == g_pmic_obj->PmicProbe()) {

//          DEBUG ((EFI_D_INFO, "PMIC probed ok, g_pmic_obj : %x\n", g_pmic_obj));
          return (PMIC_object*)g_pmic_obj;
        }
      }
    }
    DEBUG ((EFI_D_INFO, "PMIC not probed\n"));
    g_pmic_obj = &dm_obj;
    return (PMIC_object*)g_pmic_obj;
  }
  else
    return (PMIC_object*)g_pmic_obj;
}

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
  CHECK_POINTER_RETURN_INT(PmicRead8)

  return ((PMIC_object*)GetPmicInstance())->PmicRead8(BaseAddress, Register);
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
  CHECK_POINTER_RETURN_INT(PmicWrite8)

  return ((PMIC_object*)GetPmicInstance())->PmicWrite8(BaseAddress, Register, Value);
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
  CHECK_POINTER(PmicGetShutdownCause)
  return ((PMIC_object*)GetPmicInstance())->PmicGetShutdownCause(ShutdownCause);
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
  CHECK_POINTER(PmicGetResetSrc)
  return ((PMIC_object*)GetPmicInstance())->PmicGetResetSrc(ResetSrc);
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
  CHECK_POINTER(PmicGetWakeCause)

  return ((PMIC_object*)GetPmicInstance())->PmicGetWakeCause(WakeCause);
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
  CHECK_POINTER(PmicGetPwrSrcIrq)

  return ((PMIC_object*)GetPmicInstance())->PmicGetPwrSrcIrq(PowerSrcIrq);
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
  CHECK_POINTER_RETURN_BOOL(PmicIsBatOn)

  return ((PMIC_object*)GetPmicInstance())->PmicIsBatOn();
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
  CHECK_POINTER_RETURN_BOOL(PmicIsACOn)

  return ((PMIC_object*)GetPmicInstance())->PmicIsACOn();
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
  CHECK_POINTER_RETURN_BOOL(PmicVbusStatus)

  return ((PMIC_object*)GetPmicInstance())->PmicVbusStatus();
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
  CHECK_POINTER_RETURN_BOOL(PmicUSBIDStatus)

  return ((PMIC_object*)GetPmicInstance())->PmicUSBIDStatus();
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
  CHECK_POINTER(PmicClearShutdownCause)

  return ((PMIC_object*)GetPmicInstance())->PmicClearShutdownCause();
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
  CHECK_POINTER(PmicClearWakeCause)

  return ((PMIC_object*)GetPmicInstance())->PmicClearWakeCause();
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
  CHECK_POINTER(PmicClearResetSrc)
  return ((PMIC_object*)GetPmicInstance())->PmicClearResetSrc();
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
  CHECK_POINTER(PmicThermInit)
  return ((PMIC_object*)GetPmicInstance())->PmicThermInit();
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
  CHECK_POINTER(PmicGpioInit)
  return ((PMIC_object*)GetPmicInstance())->PmicGpioInit(PlatformInfo);
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
  CHECK_POINTER(PmicIntrInit)
  return ((PMIC_object*)GetPmicInstance())->PmicIntrInit();
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
  CHECK_POINTER(PmicVbusControl)
  return ((PMIC_object*)GetPmicInstance())->PmicVbusControl(Enable);
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
  CHECK_POINTER(PmicVhostControl)
  return ((PMIC_object*)GetPmicInstance())->PmicVhostControl(Enable);
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
  CHECK_POINTER(PmicBcuInit)
  return ((PMIC_object*)GetPmicInstance())->PmicBcuInit();
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
  CHECK_POINTER(PmicMiscInit)  
  return ((PMIC_object*)GetPmicInstance())->PmicMiscInit ();
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
  IN OUT UINT8     *VenId, 
  IN OUT UINT8     *DevId
  )
{
  CHECK_POINTER(PmicGetDevID)

  return ((PMIC_object*)GetPmicInstance())->PmicGetDevID(VenId, DevId);
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
  CHECK_POINTER_RETURN_INT(PmicStepping);

  return ((PMIC_object*)GetPmicInstance())->PmicStepping();
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
  return ((PMIC_object*)GetPmicInstance())->PmicType;
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
  CHECK_POINTER_RETURN_INT(PmicGetVBAT)

  return ((PMIC_object*)GetPmicInstance())->PmicGetVBAT();
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
  CHECK_POINTER_RETURN_INT(PmicGetBatteryCap)

  return ((PMIC_object*)GetPmicInstance())->PmicGetBatteryCap();
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
  CHECK_POINTER_RETURN_BOOL(PmicIsPwrBtnPressed)

  return ((PMIC_object*)GetPmicInstance())->PmicIsPwrBtnPressed();
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
  CHECK_POINTER(PmicDisablePowerButton)
  return ((PMIC_object*)GetPmicInstance())->PmicDisablePowerButton(ButtonHoldTime);
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
  CHECK_POINTER(PmicEnablePowerButton)
  return ((PMIC_object*)GetPmicInstance())->PmicEnablePowerButton(ButtonHoldTime);
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
  CHECK_POINTER(PmicSetVDDQ)
  return ((PMIC_object*)GetPmicInstance())->PmicSetVDDQ();
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
  CHECK_POINTER(PmicChargerRead)
  return ((PMIC_object*)GetPmicInstance())->PmicChargerRead(Offset, Value);
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
  CHECK_POINTER(PmicChargerWrite)
  return ((PMIC_object*)GetPmicInstance())->PmicChargerWrite(Offset, Value);
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
PmicDetectCharger(
  OUT BOOLEAN    *ChargerPresent,
  OUT UINT8      *ChargerType
  )
{
  CHECK_POINTER(PmicDetectCharger)
  return ((PMIC_object*)GetPmicInstance())->PmicDetectCharger(ChargerPresent, ChargerType);
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
  CHECK_POINTER(PmicUSBSwitchControl)
  return ((PMIC_object*)GetPmicInstance())->PmicUSBSwitchControl(Enable);
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
  UINTN i=0;
  UINTN num = sizeof (g_pmic_support_list) / sizeof(PMIC_object *) ;
  PMIC_object *g_pmic_obj=NULL; 
  
  for (;i<num; i++) {
    g_pmic_obj = g_pmic_support_list[i];
    if (NULL != g_pmic_obj->PmicProbe)  {
       if(TRUE == g_pmic_obj->PmicProbe()) {
         return TRUE;
      }
    }
  }
  DEBUG ((EFI_D_INFO, "PMIC not probed\n"));

  g_pmic_obj = &dm_obj;
  return FALSE;
}

/**
  Pmic library constructor

  PMIC type is probed in this function. It is invoked every time Pmiclib is loaded.

  @retval  EFI_SUCCESS         - Construct PMIC instance successfully.
**/
RETURN_STATUS
EFIAPI
PmicLibPeiConstructor (
  VOID
  )
{
  PmicProbe();
  return EFI_SUCCESS;
}
