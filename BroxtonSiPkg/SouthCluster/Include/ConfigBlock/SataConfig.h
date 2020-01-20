/** @file
  SATA policy

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

@par Specification Reference:
**/
#ifndef _SATA_CONFIG_H_
#define _SATA_CONFIG_H_

#define SATA_CONFIG_REVISION 2
extern EFI_GUID gSataConfigGuid;

#pragma pack (push,1)
typedef enum  {
  ScSataModeAhci,
  ScSataModeRaid,
  ScSataModeMax
} SC_SATA_MODE;

typedef enum {
  ScSataSpeedDefault,
  ScSataSpeedGen1,
  ScSataSpeedGen2,
  ScSataSpeedGen3
} SC_SATA_SPEED;

/**
  This structure configures the features, property, and capability for each SATA port.
**/
typedef struct {
  /**
    Enable SATA port.
    It is highly recommended to disable unused ports for power savings
  **/
  UINT32  Enable           :  1;    ///< 0: Disable; <b>1: Enable</b>
  UINT32  HotPlug          :  1;    ///< <b>0: Disable</b>; 1: Enable
  UINT32  InterlockSw      :  1;    ///< <b>0: Disable</b>; 1: Enable
  UINT32  External         :  1;    ///< <b>0: Disable</b>; 1: Enable
  UINT32  SpinUp           :  1;    ///< <b>0: Disable</b>; 1: Enable the COMRESET initialization Sequence to the device
  UINT32  SolidStateDrive  :  1;    ///< <b>0: HDD</b>; 1: SSD
  UINT32  DevSlp           :  1;    ///< <b>0: Disable</b>; 1: Enable DEVSLP on the port
  UINT32  EnableDitoConfig :  1;    ///< <b>0: Disable</b>; 1: Enable DEVSLP Idle Timeout settings (DmVal, DitoVal)
  UINT32  DmVal            :  4;    ///< DITO multiplier. Default is <b>15</b>.
  UINT32  DitoVal          : 10;    ///< DEVSLP Idle Timeout (DITO), Default is <b>625</b>.
  /**
    Disable ModPHY dynamic power gate <b>0: Disable</b>, 1: Enable.
    This is also used to disable ModPHY dynamic power gate if SATA slimline port with zero-power ODD (ZPODD) attached
    (or other AN capable ODD)
  **/
  UINT32  DisableDynamicPg :  1;
  UINT32  Rsvdbits0        :  9;    ///< Reserved fields for future expansion w/o protocol change
} SC_SATA_PORT_CONFIG;

///
/// The SC_SATA_CONFIG block describes the expected configuration of the SATA controllers.
///
typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Add DisableDynamicPg for disable ModPHY dynamic power gate per platform specific device.
  **/
  CONFIG_BLOCK_HEADER   Header;                ///< Config Block Header
  ///
  /// This member describes whether or not the SATA controllers should be enabled.
  ///
  UINT32                Enable          :  1;
  UINT32                TestMode        :  1;  ///< <b>(Test)</b> <b>0: Disable</b>; 1: Allow entrance to the SATA test modes
  UINT32                SalpSupport     :  1;  ///< 0: Disable; <b>1: Enable</b> Aggressive Link Power Management
  UINT32                PwrOptEnable    :  1;  ///< <b>0: Disable</b>; 1: Enable SATA Power Optimizer on SC side.
  /**
    eSATASpeedLimit
    When enabled, BIOS will configure the PxSCTL.SPD to 2 to limit the eSATA port speed.
    Please be noted, this setting could be cleared by HBA reset, which might be issued
    by EFI AHCI driver when POST time, or by SATA inbox driver/RST driver after POST.
    To support the Speed Limitation when POST, the EFI AHCI driver should preserve the
    setting before and after initialization. For support it after POST, it's dependent on
    driver's behavior.
    0: Disable; 1: Enable
  **/
  UINT32              eSATASpeedLimit    :  1;
  UINT32              Rsvdbits           : 27; ///< Reserved bits
  /**
    Determines the system will be configured to which SATA mode (SC_SATA_MODE). Default is <b>ScSataModeAhci</b>.
  **/
  SC_SATA_MODE        SataMode;
  /**
    Indicates the maximum speed the SATA controller can support
    <b>0h:ScSataSpeedDefault</b>; 1h: 1.5 Gb/s (Gen 1); 2h: 3 Gb/s(Gen 2); 3h: 6 Gb/s (Gen 1)
  **/
  SC_SATA_SPEED       SpeedLimit;
  ///
  /// This member configures the features, property, and capability for each SATA port.
  ///
  SC_SATA_PORT_CONFIG PortSettings[SC_MAX_SATA_PORTS];
  UINT32              Reserved;               ///< Reserved bytes
  UINT32              Reserved1[3];           ///< Reserved fields for future expansion
} SC_SATA_CONFIG;

#pragma pack (pop)

#endif // _SATA_CONFIG_H_
