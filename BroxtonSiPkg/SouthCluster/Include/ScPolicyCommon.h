/** @file
  SC configuration based on SC policy

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2009 - 2016 Intel Corporation.

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
#ifndef _SC_POLICY_COMMON_H_
#define _SC_POLICY_COMMON_H_

#include <ConfigBlock.h>
#include <ScLimits.h>
#include "ConfigBlock/ScGeneralConfig.h"
#include "ConfigBlock/SataConfig.h"
#include "ConfigBlock/PcieRpConfig.h"
#include "ConfigBlock/HpetConfig.h"
#include "ConfigBlock/SmbusConfig.h"
#include "ConfigBlock/IoApicConfig.h"
#include "ConfigBlock/HdAudioConfig.h"
#include "ConfigBlock/GmmConfig.h"
#include "ConfigBlock/PmConfig.h"
#include "ConfigBlock/LockDownConfig.h"
#include "ConfigBlock/SerialIrqConfig.h"
#include "ConfigBlock/ScsConfig.h"
#include "ConfigBlock/LpssConfig.h"
#include "ConfigBlock/VtdConfig.h"
#include "ConfigBlock/IshConfig.h"
#include "ConfigBlock/UsbConfig.h"
#include "ConfigBlock/FlashProtectionConfig.h"
#include "ConfigBlock/DciConfig.h"
#include "ConfigBlock/P2sbConfig.h"
#include "ConfigBlock/ScInterruptAssign.h"

#pragma pack (push,1)

#ifndef FORCE_ENABLE
#define FORCE_ENABLE  1
#endif
#ifndef FORCE_DISABLE
#define FORCE_DISABLE 2
#endif
#ifndef PLATFORM_POR
#define PLATFORM_POR  0
#endif

//
// Generic definitions for device enabling/disabling used by SC code.
//
#define DEVICE_ENABLE   1
#define DEVICE_DISABLE  0

/**
  SC Policy revision number
  Any backwards compatible changes to this structure will result in an update in the revision number
**/
#define SC_POLICY_REVISION  2
/**
  SC Policy PPI\n
  All SC config block change history will be listed here\n\n
  - Revision 1:
    - Initial version.\n
  - <b>Revision 2</b>:
    - Update SC Policy to ConfigBlock data structure.\n
**/
struct SC_POLICY {
  CONFIG_BLOCK_TABLE_HEADER      TableHeader;
/*
  Individual Config Block Structures are added here in memory as part of AddConfigBlock()
*/
};

#pragma pack (pop)

#endif // _SC_POLICY_COMMON_H_
