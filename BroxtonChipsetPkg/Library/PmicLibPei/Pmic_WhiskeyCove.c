/**@file
  Pei Library for accessing WhiskeyCove PMIC registers.

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

#include "Pmic_Private.h"
#include <PmicLib.h>
#include <Library/PmcIpcLib.h>
#include <PmicReg_WhiskeyCove.h>
#include "Pmic_WhiskeyCove.h"

//
// PMIC Init Table
//
RegInit_st mMiscRegInit[]= {
#if RVVP_ENABLE == 1
  // Power setting for camera
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4BVSEL, 0x33}, //0x4E  0xC3  0x33   Set voltage to 2.8 for VP2P8_AF  0.31
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4CVSEL, 0x33}, //0x4E  0xC4  0x33   Set voltage to 2.8 for VP2P8_CAM
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1BVSEL, 0x13}, //0x4E  0xB7  0x13   Set voltage to 1.2 for VP1P2_MCSI_A
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BCNT_REG,  REG_OVERRIDE, WKC_MASK_MODEVPROG4B, 0x01}, //PmicWrite8(DEVICE1_BASE_ADDRESS, 0xA4, 0x01);
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CCNT_REG,  REG_OVERRIDE, WKC_MASK_MODEVPROG4C, 0x01}, //PmicWrite8(DEVICE1_BASE_ADDRESS, 0xA5, 0x01);
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BCNT_REG,  REG_OVERRIDE, WKC_MASK_MODEVPROG1B, 0x01}, //PmicWrite8(DEVICE1_BASE_ADDRESS, 0x70, 0x01);
#else

  {DEVICE1_BASE_ADDRESS, WKC_VPROG2AVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2AVSEL, 0x1F},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2ACNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2A, 0x01},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2BVSEL, 0x1F}, ///< Turn on DMIC
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2B, 0x01}, ///< Turn on DMIC
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2DVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2DVSEL, 0x1F},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2DCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2D, 0x01},
#if RVV_ENABLE == 1
  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG3BVSEL, 0x3D},  //3.3V for Touch
  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG3B, 0x01},
#else

  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG3BVSEL, 0x3D},  //3.3V for Touch for Turly AOB
  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG3B, 0x01},

  {DEVICE1_BASE_ADDRESS, WKC_VPROG6BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG6BVSEL, 0x3D},  //3.3V for Touch for Tianma AOB
  {DEVICE1_BASE_ADDRESS, WKC_VPROG6BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG6B, 0x01},
#endif
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1DVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1DVSEL, 0x1F},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1DCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1D, 0x01},

#if RVV_ENABLE == 1
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2CVSEL, 0x3D},  //3.3V for WIFI
  {DEVICE1_BASE_ADDRESS, WKC_VPROG2CCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2C, 0x01},
#else
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4CVSEL, 0x31}, // 2.7V FOR A0: Camera IMX214 V_CAM_AVDD_W1 -> V_VPROG4_C
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG4C, 0x01},
#endif
  {DEVICE1_BASE_ADDRESS, WKC_VFLEXVSEL_REG,    REG_OVERRIDE, WKC_MASK_VFLEXVSEL,   0x4B}, // 1.0V FOR A0: Camera IMX214 V_CAM_DVDD_W1/W2 -> V_VFLEX
  {DEVICE1_BASE_ADDRESS, WKC_VFLEXCNT_REG,     REG_OVERRIDE, WKC_MASK_VFLEXEN,     0x01},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1CVSEL, 0x1F}, // 1.8V FOR A0: Camera IMX214 V_CAM_DOVDD_W1/U1 -> V_VPROG1_C
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1CCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1C, 0x01},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG5AVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG5AVSEL, 0x33}, // 2.8V FOR A0: Camera IMX214 V_CAM_VDD_AF-> V_VPROG5_A
  {DEVICE1_BASE_ADDRESS, WKC_VPROG5ACNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG5A, 0x01},

  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4BVSEL, 0x31}, // 2.7V FOR A0: Camera IMX132 V_CAM_AVDD_U1->V_VPROG4_B
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG4B, 0x01},
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1EVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1EVSEL, 0x13}, // 1.2V FOR A0: Camera IMX132 V_CAM_DVDD_U1->V_VPROG1_E
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1ECNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1E, 0x01},

  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1BVSEL, 0x1F}, // 1.8V for VGROP1B: ISH
  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1B, 0x01},

  {DEVICE1_BASE_ADDRESS, WKC_VPROG3AVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG3AVSEL, 0x34}, // 2.85V for VGROP3A : ISH
  {DEVICE1_BASE_ADDRESS, WKC_VPROG3ACNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG3A, 0x01},

  {DEVICE1_BASE_ADDRESS, WKC_VPROG4DVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4DVSEL, 0x3B}, // 3.2V FOR SDcard
  {DEVICE1_BASE_ADDRESS, WKC_VPROG4DCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG4D, 0x01},

  {DEVICE2_BASE_ADDRESS, WKC_CLKCONFIG_REG,    REG_OVERRIDE, (WKC_MASK_SLP2DLVL | WKC_MASK_SLP2D), 0x00},
  {DEVICE1_BASE_ADDRESS, WKC_GPIO1P0CTLO_REG,  REG_OVERRIDE, 0x7B, 0x31},
//  {DEVICE1_BASE_ADDRESS, WKC_MODEMCTRL_REG,    REG_CLEAR,    WKC_MASK_SDWNDRV, 0x00},     // Clear SDWNDRV for modem Android, move to PlatformInitPreMem.c
#endif
};

WC_PMIC_CONFIGURATION mWcPmicDev2[] = {
  {WKC_GPLEDCTRL_REG, 0x19}   //2.5mA LED 1/2HZ
};

static const UINT8 WcSteppingTable[WC_PMIC_VID_REVISION_MAX][WC_PMIC_VID_REVISION_MAX] =
{{WcA0,WcA1,WcA2,WcA3,WcA4,WcA5,WcA6,WcA7},
 {WcB0,WcB1,WcB2,WcB3,WcB4,WcB5,WcB6,WcB7},
 {WcC0,WcC1,WcC2,WcC3,WcC4,WcC5,WcC6,WcC7},
 {WcD0,WcD1,WcD2,WcD3,WcD4,WcD5,WcD6,WcD7},
 {WcE0,WcE1,WcE2,WcE3,WcE4,WcE5,WcE6,WcE7},
 {WcF0,WcF1,WcF2,WcF3,WcF4,WcF5,WcF6,WcF7},
 {WcG0,WcG1,WcG2,WcG3,WcG4,WcG5,WcG6,WcG7},
 {WcH0,WcH1,WcH2,WcH3,WcH4,WcH5,WcH6,WcH7}
};

/**
  Reads an 8-bit PMIC register of WhiskeyCove.

  Reads the 8-bit PMIC register specified by Register.
  The 8-bit read value is returned.

  @param[in]  BaseAddress     - IPC operation address for target PMIC device.
  @param[in]  Register        - The PMIC register to read.

  @retval UINT8               - The value read.
**/
UINT8
EFIAPI
WcPmicRead8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register
  )
{
  //
  //@TODO: add PMC IPC routine here  (assuming DEV1)
  //
  UINT32 Buffer = 0;
  UINT32 Buffersize = 2;

  Buffer = (((UINT32)BaseAddress)<<8) + (UINT32)Register;

  IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_READ, &Buffer, Buffersize);
  return (UINT8)Buffer;
}


/**
  Writes an 8-bit PMIC register of WhiskeyCove with a 8-bit value.

  Writes the 8-bit PMIC register specified by Register with the value specified
  by Value and return the operation status.

  @param[in]  BaseAddress     - IPC operation address for target PMIC device.
  @param[in]  Register        - The PMIC register to write.
  @param[in]  Value           - The value to write to the PMIC register.

  @retval EFI_SUCCESS         - Write bytes to PMIC device successfully
  @retval Others              - Status depends on IPC operation.
**/
EFI_STATUS
EFIAPI
WcPmicWrite8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register,
  IN UINT8                     Value
  )
{
  UINT32 Buffer = 0;
  UINT32 Buffersize = 3;

  Buffer = (((UINT32)BaseAddress) <<8) + (UINT32)Register + (((UINT32)Value)<<16);

  return (IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, Buffersize)>>16)&0xFF;
}

EFI_STATUS
EFIAPI
WcPmicReadModWrite8 (
  IN UINT8                     BaseAddress,
  IN UINT8                     Register,
  IN UINT8                     Value,
  IN UINT8                     Mask
  )
{
  UINT32 Buffer = 0;
  UINT32 Buffersize = 4;

  Buffer = (((UINT32)BaseAddress) <<8) + (UINT32)Register + (((UINT32)Value)<<16) + (((UINT32)Mask) << 24);

  return (IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_READ_MOD_WRITE, &Buffer, Buffersize)>>16)&0xFF;
}

EFI_STATUS
EFIAPI
WcPmicBatchRegisterInit (
  IN RegInit_st               *RegInit,
  IN UINT32                   length
  )
{
  UINT32 index = 0;
  UINT8 value = 0;

  for (;index < length; index++){
    value = WcPmicRead8(RegInit[index].baseAddress, RegInit[index].regoffset);

    if (0x0 == RegInit[index].mask)  continue;    // bypass register if mask is 0x0

    switch(RegInit[index].action){
      case REG_OVERRIDE:
        value &= ~(RegInit[index].mask);
        value |= (RegInit[index].value & RegInit[index].mask);
        WcPmicWrite8(RegInit[index].baseAddress, RegInit[index].regoffset, value);
        break;

      case REG_CLEAR:
        value &= ~(RegInit[index].mask);
        WcPmicWrite8(RegInit[index].baseAddress, RegInit[index].regoffset, value);
        break;

      case REG_AND:
        value &= (RegInit[index].value & RegInit[index].mask);
        WcPmicWrite8(RegInit[index].baseAddress, RegInit[index].regoffset, value);
        break;

      case REG_OR:
        value |= (RegInit[index].value & RegInit[index].mask);
        WcPmicWrite8(RegInit[index].baseAddress, RegInit[index].regoffset, value);
        break;
    }
    DEBUG ((DEBUG_INFO, "WcPmicBatchRegisterInit ----------------end offset:0x%x value:0x%x Rbuf:0x%x\n", \
        RegInit[index].regoffset, value, WcPmicRead8(RegInit[index].baseAddress, RegInit[index].regoffset)));

  }

  return EFI_SUCCESS;
}

/**
  Initialize PMIC thermal detection capability.

  @retval EFI_UNSUPPORTED      - Function not supported.
**/
EFI_STATUS
EFIAPI
WcPmicThermInit (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Initialize PMIC GPIO pin.

  Initialize PMIC GPIO pin in order to get boardid/memcfgid/.. etc later

  @param[in] PlatformInfo      - Platform information with GPIO setting.

  @retval EFI_SUCCESS          - No additional GPIO pin setting required.
**/
EFI_STATUS
EFIAPI
WcPmicGpioInit (
  IN VOID    *PlatformInfo
  )
{
  return EFI_SUCCESS;
}


/**
  Initializes PMIC device.

  @retval EFI_SUCCESS          - No additional initialization required.
**/
EFI_STATUS
EFIAPI
WcPmicIntrInit (
  VOID
  )
{
  return EFI_SUCCESS;
}

/**
  Initializes Burst Control Unit (BCU) hardware.

  The BCU has several different knobs (input stimuli configuration, interrupts,
  and output actions) that allow the Software and Firmware to change the BCU response
  behavior once these events occur. These BCU control knobs allow for a change in the
  BCU behavior in responding to these triggers and can be completely customized as
  necessary by the specific system implementation.

  @retval EFI_SUCCESS          - No additional initialization required.
**/
EFI_STATUS
EFIAPI
WcPmicBcuInit (
  VOID
  )
{
  return EFI_SUCCESS;
}


/**
  Initializes other miscellaneous functions on PMIC device.

  @retval EFI_SUCCESS          - Initializes miscellaneous functions successfully.
**/
EFI_STATUS
EFIAPI
WcPmicMiscInit (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "WcPmicMiscInit------------------\n"));
  WcPmicBatchRegisterInit (mMiscRegInit, sizeof(mMiscRegInit)/sizeof(mMiscRegInit[0]));

  return EFI_SUCCESS;
}


/**
  Turn on or off VBUS for OTG

  @param[in]  Enable           - TRUE : turn on VBUS
                                 FALSE: turn off VBUS

  @retval EFI_SUCCESS          - Turn on/off VBUS successfully.
**/
EFI_STATUS
EFIAPI
WcPmicVbusControl (
  IN BOOLEAN    Enable
  )
{
  UINT8                     Data;
  UINT8                     RdData;
  UINT8                     RdData1;

  WcPmicChargerRead(CHG_BQ24261_VENDOR_ID, &RdData);
  if(RdData == CHG_VENDOR_ID) { //BQ24261
    //Charger Burst mode.
    WcPmicChargerRead(CHG_BQ24261_STS_CTRL_REG0, &RdData);
    Data = PmicRead8(DEVICE3_BASE_ADDRESS, WKC_CHGRCTRL1_REG);
    if (Enable == TRUE) {
    //
    // host
    //
    Data |= WKC_MASK_OTGMODE; //OTG Boost Mode, force charger state machine to ignore VBUS detection and prevent charging from the internal boost
    RdData |= CHG_BQ24261_STS_CTRL_BOOOST; //bit6 Boost Mode
    } else {
    //
    // device
    //
    Data &= ~WKC_MASK_OTGMODE; //Normal Mode
    RdData &= ~CHG_BQ24261_STS_CTRL_BOOOST; //bit6 no Boost Mode
    }
    PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_CHGRCTRL1_REG, Data);
    WcPmicChargerWrite (CHG_BQ24261_STS_CTRL_REG0, RdData); // bit6 Boost Mode.
  } else { //BQ25892
    Data = PmicRead8(DEVICE3_BASE_ADDRESS,WKC_CHGRCTRL1_REG);
    if (Enable == TRUE) {//host
      Data |= WKC_MASK_OTGMODE;//OTG Boost Mode, force charger state machine to ignore VBUS detection and prevent charging from the internal boost

      WcPmicChargerRead(CHG_BQ25892_WDT, &RdData1);
      RdData1 = RdData1 & 0x8B;
      WcPmicChargerWrite(CHG_BQ25892_WDT, RdData1);

      WcPmicChargerRead(CHG_BQ25892_CTRL, &RdData);
      RdData = RdData & 0x0F;
      RdData = RdData | 0x20; //Set OTG enable in charger register
      WcPmicChargerWrite(CHG_BQ25892_CTRL, RdData);

      RdData = PmicRead8(DEVICE1_BASE_ADDRESS,WKC_GPIO0P4CTLO_REG);
      RdData = RdData & 0x00;
      RdData = RdData | 0x31; //Set A4WP Vbus mux to 1 to driver VBus in charger IC
      PmicWrite8(DEVICE1_BASE_ADDRESS,WKC_GPIO0P4CTLO_REG,RdData);
    } else {//device
      Data &= ~WKC_MASK_OTGMODE;//Normal Mode

      WcPmicChargerRead(CHG_BQ25892_WDT, &RdData1);
      RdData1 = RdData1 & 0x8B;
      WcPmicChargerWrite(CHG_BQ25892_WDT, RdData1);

      WcPmicChargerRead(CHG_BQ25892_CTRL, &RdData);
      RdData = RdData & 0x0F;
      RdData = RdData | 0x10; //Set OTG enable in charger register
      WcPmicChargerWrite(CHG_BQ25892_CTRL, RdData);

      RdData = PmicRead8(DEVICE1_BASE_ADDRESS,WKC_GPIO0P4CTLO_REG);
      RdData = RdData & 0x00;
      RdData = RdData | 0x30; //Set A4WP Vbus mux to 0
      PmicWrite8(DEVICE1_BASE_ADDRESS,WKC_GPIO0P4CTLO_REG,RdData);
    }
    PmicWrite8(DEVICE3_BASE_ADDRESS,WKC_CHGRCTRL1_REG,Data);
  }

  return EFI_SUCCESS;
}

/**
  Turn on or off V5P0S (5V VBUS for USB2/3 HOST)

  @param[in]  Enable           - TRUE : turn on V5P0S
                                 FALSE: turn off V5P0S

  @retval EFI_SUCCESS          - Turn on/off V5P0S successfully.
**/
EFI_STATUS
EFIAPI
WcPmicVhostControl (
  IN BOOLEAN    Enable
  )
{
  DEBUG ((DEBUG_INFO, "WcPmicVhostControl "));

  WcPmicReadModWrite8((UINT8) DEVICE1_BASE_ADDRESS, (UINT8) WKC_GPIO1P1CTLO_REG, (UINT8) 0x31, (UINT8) 0x7B);
  //WcPmicWrite8((UINT8) DEVICE1_BASE_ADDRESS, (UINT8) WKC_GPIO1P1CTLO_REG, (UINT8) 0x11);
  //WcPmicReadModWrite8((UINT8) DEVICE3_BASE_ADDRESS, (UINT8) WKC_USBPHYCTRL_REG, (UINT8) 0x01, (UINT8) WKC_MASK_USBPHYRSTB);
  return EFI_SUCCESS;
}

/**
  Get PMIC Vendor ID and Device ID

  @param[in, out] VenId        - Vendor ID
  @param[in, out] DevId        - Device ID

  @retval EFI_SUCCESS          - Get Vendor ID and Device ID successfully.
**/
EFI_STATUS
EFIAPI
WcPmicGetDevID (
  IN OUT UINT8    *VenId,
  IN OUT UINT8    *DevId
  )
{
  *VenId = WcPmicRead8 (DEVICE1_BASE_ADDRESS, WKC_ID0_REG);
  *DevId = WcPmicRead8 (DEVICE1_BASE_ADDRESS, WKC_ID1_REG);

  return EFI_SUCCESS;
}

/**
  This procedure will get WC PMIC Stepping

  @retval WC PMIC Stepping
**/
UINT8
EFIAPI
WcPmicStepping (
  VOID
  )
{
  UINT8                         VendorId;
  UINT8                         DeviceId;
  UINT8                         VIDMajorVersion;
  UINT8                         VIDMinorVersion;
  EFI_STATUS                    Status;

  Status = WcPmicGetDevID(&VendorId, &DeviceId);
  if(Status != EFI_SUCCESS){
    return WcA0;
    }
  VIDMajorVersion = (VendorId & WC_PMIC_VID_MAJOR_REVISION_BITMAP)>>3;
  VIDMinorVersion = VendorId & WC_PMIC_VID_MINOR_REVISION_BITMAP;
  if((VIDMajorVersion >= WC_PMIC_VID_REVISION_MAX)||(VIDMinorVersion >= WC_PMIC_VID_REVISION_MAX)){
    DEBUG ((DEBUG_ERROR, "Unsupported PMIC stepping.\n"));
    return WcSteppingMax;
    }
  DEBUG ((DEBUG_INFO, "Whiskey Cove %c%c PMIC detected!\n", (VIDMajorVersion + 0x41), (VIDMinorVersion + 0x30)));
  return WcSteppingTable[VIDMajorVersion][VIDMinorVersion];
}

/**
  Get battery voltage.

  @retval UINT16               - ADC result for battery voltage.
**/
UINT16
EFIAPI
WcPmicGetVBAT (
  VOID
  )
{
  UINT16 value16;
  //TODO:call GPADC routine to capture Battery voltage

  //TODO:RSLT_REG_SEL = b'0000 Store result to VBATRSTL and VBATRSTH

  //get result from VBATRSTL and VBATRSTH
  value16  = (WcPmicRead8(DEVICE2_BASE_ADDRESS, WKC_VBATRSLTL_REG) & 0xff);
  value16 |= ((WcPmicRead8(DEVICE2_BASE_ADDRESS, WKC_VBATRSLTH_REG) & 0xF) << 8 );  //read after VBATRSTL

  return value16;
}

/**
  VBUS Detection Status

  It can be used to detect whether USB charger is connected.

  @retval TRUE                - Connected
  @retval FALSE               - Disconnected
**/
BOOLEAN
EFIAPI
WcPmicVbusStatus (
  VOID
  )
{
  UINT8 Value;

  Value = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_SPWRSRC_REG);
  return ((Value & WKC_MASK_SVBUSDET) == WKC_MASK_SVBUSDET);
}

/**
  USB ID Detection Status

  @retval UINT8               - 0: ACA ID resistor or non-floating pin detected
                                1: Short to ground detected (Host mode)
                                2: Floating pin detected (Device mode)
                                Others: reserved
**/
UINT8
EFIAPI
WcPmicUSBIDStatus (
  VOID
  )
{
  UINT8 Value;

  Value = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_SPWRSRC_REG);
  return ((Value & WKC_MASK_SUSBIDDET) >> 3);
}

/**
  Battery Detection Status

  @retval TRUE                - Connected
  @retval FALSE               - Disconnected
**/
BOOLEAN
EFIAPI
WcPmicIsBatOn (
  VOID
  )
{
  UINT8 Value;

  Value = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_SPWRSRC_REG);
  return ((Value & WKC_MASK_SBATTDET) == WKC_MASK_SBATTDET);
}

/**
  AC/DC Adapter Detection Status

  @retval TRUE                - Connected
  @retval FALSE               - Disconnected
**/
BOOLEAN
EFIAPI
WcPmicIsACOn (
  VOID
  )
{
  UINT8 Value;

  Value = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_SPWRSRC_REG);
  return ((Value & WKC_MASK_SDCDET) == WKC_MASK_SDCDET);
}

/**
  Get power button status

  @retval TRUE                  - Power button pressed.
  @retval FALSE                 - Power button released.
**/
BOOLEAN
EFIAPI
WcPmicIsPwrBtnPressed (
  VOID
  )
{
  UINT8 Value;

  Value = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_PBSTATUS_REG);
  return ((Value & WKC_MASK_PBLVL) != WKC_MASK_PBLVL);  //0 means pwrbtn pressed
}

/**
  Get previous shutdown root cause.

  @param[in, out]  ShutdownCause  - The data to store shutdown root cause.

  @retval EFI_SUCCESS             - Get shutdown root cause successfully.
  @retval EFI_INVALID_PARAMETER   - ShutdownCause is NULL.
**/
EFI_STATUS
EFIAPI
WcPmicGetShutdownCause (
  IN OUT UINT32    *ShutdownCause
  )
{
  UINT8 val=0;

  if (NULL == ShutdownCause) return EFI_INVALID_PARAMETER;

  val = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_RESETSRC0_REG);
  *ShutdownCause = 0;
  if (val & WKC_MASK_RBATTEMP)  {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RBATTEMP;}
  if (val & WKC_MASK_RVSYSUVP)  {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RVSYSUVP;}
  if (val & WKC_MASK_RBATTRM)   {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RBATTEMP;}
  if (val & WKC_MASK_RVCRIT)    {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RVCRIT;}
  if (val & WKC_MASK_RSYSTEMP)  {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RSYSTEMP;}
  if (val & WKC_MASK_RPMICTEMP) {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RPMICTEMP;}
  if (val & WKC_MASK_RTHERMTRIP){  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RTHERMTRIP;}

  val = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_RESETSRC1_REG);
  if (val & WKC_MASK_RFCO)      {  *ShutdownCause |= PUPDR_SHUTDOWN_CAUSE_RFCO;}

  return EFI_SUCCESS;
}

/**
  Get previous reset root cause

  @param[in, out]  ResetSrc     - The data to store reset root cause

  @retval EFI_SUCCESS           - Get reset root cause successfully.
  @retval EFI_INVALID_PARAMETER - ResetSrc is NULL.
**/
EFI_STATUS
EFIAPI
WcPmicGetResetSrc (
  IN OUT    UINT32 *ResetSrc
  )
{
  UINT8 val=0;

  if (NULL == ResetSrc) return EFI_INVALID_PARAMETER;

  val = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_RESETSRC1_REG);
  *ResetSrc = 0;
  if (val & WKC_MASK_RWARMRESET)   {  *ResetSrc |= PUPDR_SHUTDOWN_CAUSE_RWARMRESET;}
  if (val & WKC_MASK_RCOLDRESET)   {  *ResetSrc |= PUPDR_SHUTDOWN_CAUSE_RCOLDRESET;}
  if (val & WKC_MASK_RPLATFORMRST) {  *ResetSrc |= PUPDR_SHUTDOWN_CAUSE_RPLATFORMRST;}
  return EFI_SUCCESS;
}

/**
  Clear previous shutdown root cause.

  @retval EFI_SUCCESS               - Clear shutdown root cause successfully.
**/
EFI_STATUS
EFIAPI
WcPmicClearShutdownCause (
  VOID
  )
{
  WcPmicWrite8(DEVICE1_BASE_ADDRESS, WKC_RESETSRC0_REG, 0xFF);
  WcPmicWrite8(DEVICE1_BASE_ADDRESS, WKC_RESETSRC1_REG, WKC_MASK_RFCO);
  return EFI_SUCCESS;
}

/**
  Clear the cause of system reset.

  @retval EFI_SUCCESS          - Clear reset cause successfully.
  @retval Others               - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
WcPmicClearResetSrc (
  VOID
  )
{
  // clear all flags but RFCO that is cleared in WcPmicClearShutdownCause
  WcPmicWrite8(DEVICE1_BASE_ADDRESS, WKC_RESETSRC1_REG, 0xF7);
  return EFI_SUCCESS;
}

/**
  Get the cause of system wake event.

  @param[in, out]  WakeCause    - The data to store the cause of wake event.

  @retval EFI_SUCCESS           - Get wake cause successfully.
  @retval EFI_INVALID_PARAMETER - WakeCause is NULL.
**/
EFI_STATUS
EFIAPI
WcPmicGetWakeCause  (
  IN OUT UINT32    *WakeCause
  )
{
  UINT8 val = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_WAKESRC_REG);
  if (NULL == WakeCause) return EFI_INVALID_PARAMETER;

  *WakeCause = 0;
  if (val & WKC_MASK_WAKESPL)   {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKESPL;}
  if (val & WKC_MASK_WAKEPWRON) {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEPWRON;}
  if (val & WKC_MASK_WAKEWA)    {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEWA;}
  if (val & WKC_MASK_WAKEUSBID) {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEUSBID;}
  if (val & WKC_MASK_WAKEVDCIN) {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEVDCIN;}
  if (val & WKC_MASK_WAKEVBUS)  {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEVBUS;}
  if (val & WKC_MASK_WAKEBAT)   {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEBAT;}
  if (val & WKC_MASK_WAKEPBTN)  {  *WakeCause |= PUPDR_WAKE_CAUSE_WAKEPBTN;}
  return EFI_SUCCESS;
}

/**
  Get power source detection result.

  @param[in, out]  PowerSrcIrq   - The data to the cause of wake event.

  @retval EFI_SUCCESS            - Get power source successfully.
  @retval EFI_INVALID_PARAMETER  - PowerSrcIrq is NULL.
**/
EFI_STATUS
EFIAPI
WcPmicGetPwrSrcIrq (
  IN OUT UINT32    *PowerSrcIrq
  )
{
  UINT8 val = WcPmicRead8(DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ1_REG);
  if (NULL == PowerSrcIrq) return EFI_INVALID_PARAMETER;

  *PowerSrcIrq = 0;
  if (val & WKC_MASK_BATTDET)  { *PowerSrcIrq |= PUPDR_PWRSRCIRQ_BATDET;}
  if (val & WKC_MASK_DCDET)    { *PowerSrcIrq |= PUPDR_PWRSRCIRQ_DCINDET;}
  if (val & WKC_MASK_VBUSDET)  { *PowerSrcIrq |= PUPDR_PWRSRCIRQ_VBUSDET;}

  return EFI_SUCCESS;
}

/**
  Clear the cause of system wake event.

  @retval EFI_SUCCESS               - Clear wake cause successfully.
**/
EFI_STATUS
EFIAPI
WcPmicClearWakeCause (
  VOID
  )
{
  WcPmicWrite8(DEVICE1_BASE_ADDRESS, WKC_WAKESRC_REG, 0xFF);

  return EFI_SUCCESS;
}

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
  )
{
  UINT8        Data;
  UINT8        Retry = 10;
  EFI_STATUS   Status = EFI_SUCCESS;

//  DEBUG ((DEBUG_INFO, "%a Enter \n", __FUNCTION__));

  //
  // 1. Software loads the appropriate I2C Address for the Charger IC into the I2COVRDADDR register.
  //
  Data = CHG_I2C_BQ24261_SLAVEADDRESS;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVRDADDR_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVRDADDR_REG return 0x%x", Status));
    return Status;
  }

  //
  // 2. Software loads the appropriate Charger IC register offset into the I2COVROFFSET  register.
  //
  Data = Offset;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVROFFSET_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVROFFSET_REG return 0x%x", Status));
    return Status;
  }

  //
  // 3. Software sets the I2CRD bit in the I2COVRCTRL register to 1, initiating a "Read".
  //
  Data = WKC_MASK_I2CRD;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVRCTRL_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVRCTRL_REG return 0x%x", Status));
    return Status;
  }

  //
  // 4. System PMU CCSM responds by reading the corresponding Charger IC register  and storing the data in the I2COVRRDDATAregister.
  //    System PMU CCSM sets the I2CRDCMP bit in the CHGRIRQ0 register once the transaction is completed,
  //    and generates an IRQ to the SOC if MCHGRIRQ0.MI2CRDCMP = 0.
  //
  Data = 0xFF; // clean buff;
  Data = PmicRead8 (DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ0_REG);

  while ((WKC_MASK_I2CRDCMP != (WKC_MASK_I2CRDCMP & Data)) && (Retry != 0)) {
    Data = 0xFF; // clean buff;
    Data = PmicRead8 (DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ0_REG);
    MicroSecondDelay (10);
    Retry--;
  }
  Data = WKC_MASK_I2CRDCMP; // write 1 to clear. Clear that bit only
  Status = PmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ0_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write CHGRIRQ0_REG return 0x%x", Status));
    return Status;
  }

  //
  // 5. Software reads the data from the I2COVRRDDATA register.
  //
  Data = 0xFF; // clean buff;
  Data = PmicRead8 (DEVICE3_BASE_ADDRESS, WKC_I2COVRRDDATA_REG);
  *Value = Data;

//  DEBUG ((DEBUG_INFO, "%a Exit offset:0x%x value:0x%x \n", __FUNCTION__, Offset, *Value));

  return Status;
}

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
  )
{
  UINT8         Data;
  UINT8         Retry = 10;
  EFI_STATUS    Status = EFI_SUCCESS;

//  DEBUG ((DEBUG_INFO, "%a Enter\n", __FUNCTION__));

  //
  // 1. Software loads the appropriate I2C Address for the Charger IC into the I2COVRDADDR register.
  //
  Data = CHG_I2C_BQ24261_SLAVEADDRESS;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVRDADDR_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVRDADDR_REG return 0x%x", Status));
    return Status;
  }

  //
  // 2. Software loads the appropriate Charger IC register offset into the I2COVROFFSET register.
  //
  Data = Offset;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVROFFSET_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVROFFSET_REG return 0x%x", Status));
    return Status;
  }

  //
  // 3. Software loads the data to be written to the Charger IC into the I2COVRWRDATA register.
  //
  Data = Value;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVRWRDATA_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVRWRDATA_REG return 0x%x", Status));
    return Status;
  }

  //
  // 4. Software sets the I2CWR bit in the I2COVRCTRL register, initiating a 'Write'.
  //
  Data = WKC_MASK_I2CWR;
  Status = PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_I2COVRCTRL_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write I2COVRCTRL_REG return 0x%x", Status));
    return Status;
  }

  //
  // 5. System PMU CCSM responds by writing the corresponding Charger IC register. System PMU CCSM sets the I2CWRCMP bit in the CHGRIRQ0 register
  //    once the transaction has completed, and generates an IRQ to the SOC if MCHGRIRQ0.MI2CWRCMP = 0.
  //
  Data = 0xFF;
  Data = PmicRead8 (DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ0_REG);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to read CHGRIRQ0_REG return 0x%x", Status));
    return Status;
  }
  while ((WKC_MASK_I2CWRCMP != (WKC_MASK_I2CWRCMP & Data)) && (Retry != 0)) {
    Data = 0xFF; //clean buff;
    Data = PmicRead8 (DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ0_REG);
    MicroSecondDelay (10);
    Retry--;
  }
  Data = WKC_MASK_I2CWRCMP; //write 1 to clear. Clear that bit only
  Status = PmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_CHGRIRQ0_REG, Data);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to write CHGRIRQ0_REG return 0x%x", Status));
    return Status;
  }

//  DEBUG ((DEBUG_INFO, "%a Exit offset:0x%x value:0x%x \n", __FUNCTION__, Offset, Value));
  return Status;
}

/**
  Detect charger type and charger presence.

  @param[out] ChargerPresent   - TRUE : Charger present
                                 FALSE: Charger not present
  @param[out] ChargerType      - Charger type - SDP/DCP/CDP... etc.
                                 0000 = Not determined
                                 0001 = SDP Attached
                                 0010 = DCP Attached
                                 0011 = CDP Attached
                                 0100 = ACA Attached
                                 0101 = SE1 Attached
                                 0110 = MHL Attached
                                 0111 = Floating D+/D- Attached
                                 1000 = Other Attached
                                 1001 = DCP detected by ext. USB PHY
                                 1010-1111 = Rsvd

  @retval EFI_SUCCESS          - Charger present and charger type got.
  @retval EFI_NOT_READY        - Status depends on each PMIC operation.
**/
EFI_STATUS
EFIAPI
WcPmicDetectCharger (
  OUT BOOLEAN    *ChargerPresent,
  OUT UINT8      *ChargerType
  )
{
  UINT8 Value8;
  EFI_STATUS Status = EFI_SUCCESS;

  Value8 = WcPmicRead8 (DEVICE3_BASE_ADDRESS, WKC_USBSRCDETSTATUS0_REG);
  if ((Status == EFI_SUCCESS) && ((Value8 & WKC_MASK_SUSBHWDET) == WKC_USBHWSTATUE_COMPLETED)) {
    *ChargerPresent = TRUE;
    *ChargerType = (Value8 & WKC_MASK_USBSRCDETRSLT) >> 2;
  } else {
    *ChargerPresent = FALSE;
    *ChargerType = (Value8 & WKC_MASK_USBSRCDETRSLT) >> 2;
    Status = EFI_NOT_READY;
  }
  return Status;
}

/**
  Controls external USB PHY reset

  @param[in] Enable            - TRUE : OUTOFRESET - external PHY is out of reset
                                 FALSE: INRESET - external PHY is in reset

  @retval EFI_SUCCESS          - Switch USB PHY reset state successfully.
**/
EFI_STATUS
EFIAPI
WcPmicUSBSwitchControl (
  IN BOOLEAN    Enable
  )
{
  UINT8                     Data;

  //USB Switch Control
  Data = PmicRead8 (DEVICE3_BASE_ADDRESS, WKC_USBPHYCTRL_REG);
  if (Enable == TRUE) {
    //
    // Conducting
    //
    Data |= WKC_MASK_USBPHYRSTB;
  } else {
    //
    // no Conducting
    //
    Data &= ~WKC_MASK_USBPHYRSTB;
  }
  PmicWrite8 (DEVICE3_BASE_ADDRESS, WKC_USBPHYCTRL_REG, Data);

  return EFI_SUCCESS;
}

/**
  Disable the capability to shutdown platform using power button.
  0000 = Disabled (No forced power down action taken on long btn press)
  0001 = S1 1 second
  0010 = S2 2 seconds
  0011 = S3 3 seconds
  0100 = S4 4 seconds
  ....
  1111 = S15 15 seconds
  Reset: 1010B

  @param[out] ButtonHoldTime    - If the power button is disabled successfully, this contains the time in seconds need to hold power button to shutdown platform.

  @return     EFI_SUCCESS       - Succeed to disable power button
              Other             - Failed to disable power button
**/
EFI_STATUS
EFIAPI
WcPmicDisablePowerButton (
  OUT UINT8    *ButtonHoldTime
  )
{
  EFI_STATUS   Status;
  UINT8        Value = 0;

  DEBUG ((DEBUG_INFO, "WcPmicDisablePowerButton()\n"));
  Value = WcPmicRead8 (DEVICE1_BASE_ADDRESS, WKC_PBCONFIG1_REG);
  *ButtonHoldTime = Value & WKC_MASK_FLT;
  Status = WcPmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_PBCONFIG1_REG, Value & ~WKC_MASK_FLT);
  DEBUG ((DEBUG_INFO, "WcPmicDisablePowerButton value & ~WKC_MASK_FLT: 0x%x\n", Value & ~WKC_MASK_FLT));
  return Status;
}

/**
  Enable the capability to shutdown platform using power button.
  0000 = Disabled (No forced power down action taken on long btn press)
  0001 = S1 1 second
  0010 = S2 2 seconds
  0011 = S3 3 seconds
  0100 = S4 4 seconds
  ....
  1111 = S15 15 seconds
  Reset: 1010B

  @param[in] ButtonHoldTime    - Time in seconds to shut down the platform if power button is enabled and hold

  @return    EFI_SUCCESS       - Succeed to enable power button
             Other             - Failed to enable power button
**/
EFI_STATUS
EFIAPI
WcPmicEnablePowerButton (
  IN UINT8    ButtonHoldTime
  )
{
  EFI_STATUS  Status;
  UINT8       Value = 0;

  DEBUG ((DEBUG_INFO, "WcPmicEnablePowerButton()\n"));
  Value = WcPmicRead8 (DEVICE1_BASE_ADDRESS, WKC_PBCONFIG1_REG);
  Status = WcPmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_PBCONFIG1_REG, (Value & ~WKC_MASK_FLT) | (ButtonHoldTime & WKC_MASK_FLT));
  return Status;
}

/**
  Probe to find the correct PMIC object.

  After probling, g_pmic_obj points to correct PMIC object
  This routine is invoked when library is loaded .

  @retval TRUE                - Always return true to indicate WhiskeyCove PMIC existed.
**/
BOOLEAN
EFIAPI
WcPmicProbe (
  VOID
  )
{
  return TRUE;
}


PMIC_object WcObj = {
  WC_PMIC_I2C_CHANNEL_NUMBER,
  WC_PMIC_I2C_DEV_1_SLAVE_ADDRESS,
  WC_PMIC_I2C_DEV_1_SLAVE_ADDRESS,
  PMIC_TYPE_WC,
  WcPmicRead8,
  WcPmicWrite8,

  WcPmicProbe,
  WcPmicGetDevID,
  WcPmicStepping,
  WcPmicThermInit,
  WcPmicGpioInit,
  WcPmicIntrInit,
  WcPmicBcuInit,
  WcPmicMiscInit,
  WcPmicVbusControl,
  WcPmicVhostControl,
  WcPmicBatchRegisterInit,

  //
  // PUPDR interfaces
  //
  WcPmicGetWakeCause,
  WcPmicGetShutdownCause,
  WcPmicGetResetSrc,
  WcPmicGetPwrSrcIrq,
  WcPmicIsBatOn,
  WcPmicIsACOn,
  WcPmicVbusStatus,
  WcPmicClearShutdownCause,
  WcPmicClearWakeCause,
  WcPmicClearResetSrc,

  NULL,                     // PmicGetVBAT
  WcPmicIsPwrBtnPressed,
  WcPmicDisablePowerButton, // PmicDisablePowerButton
  WcPmicEnablePowerButton,  // PmicEnablePowerButton
  NULL,                     // PmicGetBatteryCap
  NULL,                     // PmicSetVDDQ
  WcPmicChargerRead,
  WcPmicChargerWrite,
  WcPmicDetectCharger,
  WcPmicUSBSwitchControl,
  WcPmicUSBIDStatus,
};
