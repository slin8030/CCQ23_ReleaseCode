/** @file
  Interface definition details between System Agent and platform drivers during DXE phase.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#ifndef _SA_POLICY_H_
#define _SA_POLICY_H_

#include <ConfigBlock.h>
#include <Protocol/SaDxeMiscConfig.h>
#include <Protocol/IgdPanelConfig.h>

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gSaPolicyProtocolGuid;

//
// Protocol revision number
// Any backwards compatible changes to this protocol will result in an update in the revision number
// Major changes will require publication of a new protocol
//
#define SA_POLICY_PROTOCOL_REVISION_1 0

///
/// Generic definitions for device enabling/disabling used by NC code.
///
#define DEVICE_ENABLE   1
#define DEVICE_DISABLE  0

#define NO_AUDIO   0
#define HD_AUDIO   1
#define LPE_AUDIO  2

///
/// SA Policy Protocol
///
typedef struct _SA_POLICY_PROTOCOL {
  CONFIG_BLOCK_TABLE_HEADER      TableHeader;    ///< Offset 0-31
  /*
  Individual Config Block Structures are added here in memory as part of AddConfigBlock()
  */
} SA_POLICY_PROTOCOL;

#endif
