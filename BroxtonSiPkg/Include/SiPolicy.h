/** @file
  Intel reference code configuration policies.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

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
#ifndef _SI_POLICY_H_
#define _SI_POLICY_H_

/**
  Silicon Policy revision number
  Any backwards compatible changes to this structure will result in an update in the revision number

  Revision 1: Initial version
  Revision 2: Updated version
                - Add OsSelection
**/
#define SI_POLICY_REVISION  2

///
/// OS Selection
///
typedef enum {
  SiWindows = 0,
  SiAndroid = 1,
  SiWin7    = 2,
  SiLinux   = 3
} SI_OS_SELECTION;

/**
  The Silicon Policy allows the platform code to publish a set of configuration
  information that the RC drivers will use to configure the silicon hardware.
**/
struct _SI_POLICY {
  /**
    This member specifies the revision of the Silicon Policy. This field is used to indicate backward
    compatible changes to the policy structure. Any such changes to this policy structure will result in an update
    in the revision number.
  **/
  UINT8  Revision;
  ///
  /// Platform specific common policies that used by several silicon components.
  ///
  ///
  /// This member determines the SMBIOS OEM type (0x80 to 0xFF) defined in SMBIOS
  /// Type 14 - Group Associations structure - item type.
  /// FVI structure uses it as SMBIOS OEM type to provide version information.
  ///
  UINT8   FviSmbiosType;
  ///
  /// Reports if EC is present or not.
  ///
  BOOLEAN EcPresent;
  ///
  /// Temp Bus Number range available to be assigned to each root port and its downstream
  /// devices for initialization of these devices before PCI Bus enumeration.
  ///
  UINT8   TempPciBusMin;
  UINT8   TempPciBusMax;
  ///
  /// Temporary Memory Base Address for PCI devices to be used to initialize MMIO registers.
  /// Minimum size is 2MB bytes
  ///
  UINT32  TempMemBaseAddr;
  ///
  /// Size of allocated temp MMIO space
  ///
  UINT32  TempMemSize;
  ///
  /// Temporary IO Base Address for PCI devices to be used to initialize IO registers.
  ///
  UINT16  TempIoBaseAddr;
  ///
  /// Size of temporary IO space
  ///
  UINT16 TempIoSize;
  UINT32 OsSelection :2;
  UINT32 RsvdBits    :30;///< Reserved bits

///
///  Enable/Disable Intel® Dynamic Platform and Thermal Framework
///
  UINT32 DptfEnabled;
///
///  Added PWMEnabled to enable/disable PWM Device.
///
  UINT32 PWMEnabled;

  UINT32 Reserved[5];
};

#endif
