/** @file
  Required by Microsoft Windows to report the available debug ports on the platform.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1996 - 2016 Intel Corporation.

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

#ifndef _UFS_MPHY_DATA_DEF_H
#define _UFS_MPHY_DATA_DEF_H

#define UFS_MPHY_TIMEOUT                         MultU64x32((UINT64)(300), 10)
#define UFS_MPHY_MAX_OVERRIDE_REGS               0x15

#pragma pack(1)
//
// UFS MPHY register programming structure
//
typedef struct _UFS_MPHY_REG_CONFIG {
  UINT32 RegOff;
  UINT32 RegBitMask;
  UINT32 RegVal;
  UINT32 RegPoll;
}UFS_MPHY_REG_CONFIG;

//
// MPHY override programming structure
//
typedef struct _UFS_MPHY_OVERRIDE_CONFIG {
  UINT32               Magic[4];  ///< 0,1,2,3: 0xFEEDFACE, 0xA5A5A5A5, 0xDEADF00D, 0x5A5A5A5A
  UINT32               Checksum;  ///< set to checksum of remainder of this data including reg count
  UINT32               RegCount;
  UFS_MPHY_REG_CONFIG  MphyRegVals[UFS_MPHY_MAX_OVERRIDE_REGS];
} UFS_MPHY_OVERRIDE_CONFIG;

#pragma pack()

#endif
