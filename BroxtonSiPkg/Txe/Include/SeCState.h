/** @file
  Register Definitions for SeC States

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

@par Specification
**/
#ifndef _SEC_STATE_H
#define _SEC_STATE_H
//
// Ignore SEC_FW_INIT_COMPLETE status Macro
//
#define SEC_STATUS_SEC_STATE_ONLY(a)  ((a) & (~(SEC_FW_INIT_COMPLETE | SEC_FW_BOOT_OPTIONS_PRESENT)))

//
// Macro to check if SEC FW INIT is completed
//
#define SEC_STATUS_IS_SEC_FW_INIT_COMPLETE(a) (((a) & SEC_FW_INIT_COMPLETE) == SEC_FW_INIT_COMPLETE)

//
// Marco to combind the complete bit to status
//
#define SEC_STATUS_WITH_SEC_INIT_COMPLETE(a)  ((a) | SEC_FW_INIT_COMPLETE)

//
// Macro to check SEC Boot Option Present
//
#define SEC_STATUS_IS_SEC_FW_BOOT_OPTIONS_PRESENT(a)  (((a) & SEC_FW_BOOT_OPTIONS_PRESENT) == SEC_FW_BOOT_OPTIONS_PRESENT)

//
// Abstract SEC Mode Definitions
//
#define SEC_MODE_NORMAL        0x00

#define SEC_DEBUG_MODE_ALT_DIS 0x02
#define SEC_MODE_TEMP_DISABLED 0x03
#define SEC_MODE_RECOVER       0x04
#define SEC_MODE_FAILED        0x06


//
// Abstract SEC Status definitions
//
#define SEC_READY                    0x00



#define SEC_INITIALIZING             0x01
#define SEC_IN_RECOVERY_MODE         0x02
#define SEC_DISABLE_WAIT             0x06
#define SEC_TRANSITION               0x07
#define SEC_NOT_READY                0x0F
#define SEC_FW_INIT_COMPLETE         0x80
#define SEC_FW_BOOT_OPTIONS_PRESENT  0x100
#define SEC_FW_UPDATES_IN_PROGRESS   0x200

#pragma pack()

#endif // SEC_STATE_H
