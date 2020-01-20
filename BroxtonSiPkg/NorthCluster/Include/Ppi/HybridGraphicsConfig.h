/** @file
  Hybrid Graphics policy definitions

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
#ifndef _HYBRID_GRAPHICS_CONFIG_H_
#define _HYBRID_GRAPHICS_CONFIG_H_
#pragma pack(1)

#define HYBRID_GRAPHICS_CONFIG_REVISION 1

extern EFI_GUID gHybridGraphicsConfigGuid;

///
/// HG GPIO Data Structure
///
typedef struct {
  UINT32   CommunityOffset;  ///< GPIO Community
  UINT16   PinOffset;        ///< GPIO Pin
  BOOLEAN  Active;           ///< 0=Active Low; 1=Active High
} HG_GPIO;


///
/// Defines the Switchable Graphics configuration parameters for System Agent.
///
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Offset 0-23 Config Block Header
  UINT8                 RootPortDev;              ///< Device number used for SG
  UINT8                 RootPortFun;              ///< Function number used for SG
  UINT8                 HgEnabled;                ///< HgEnabled (0=Disabled, 1=Enabled)
  UINT16                HgSubSystemId;            ///< Hybrid Graphics Subsystem ID
  UINT16                HgDelayAfterPwrEn;        ///< Dgpu Delay after Power enable using Setup option
  UINT16                HgDelayAfterHoldReset;    ///< Dgpu Delay after Hold Reset using Setup option
  HG_GPIO               HgDgpuHoldRst;            ///< This field contain dGPU HLD RESET GPIO value and level information
  HG_GPIO               HgDgpuPwrEnable;          ///< This field contain dGPU_PWR Enable GPIO value and level information
} HYBRID_GRAPHICS_CONFIG;

#pragma pack()
#endif // _HYBRID_GRAPHICS_CONFIG_H_
