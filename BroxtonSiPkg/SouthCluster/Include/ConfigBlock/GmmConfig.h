/** @file
  GMM(Gaussian Mixture Model) scoring accelerator policy

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
#ifndef _GMM_CONFIG_H_
#define _GMM_CONFIG_H_

#define GMM_CONFIG_REVISION 1
extern EFI_GUID gGmmConfigGuid;

#pragma pack (push,1)

/**
  This structure contains the policies which are related to GMM.
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;            ///< Config Block Header
  UINT32 Enable                   : 1;     ///< <b>1: Enable</b>, 0: Disable
  UINT32 ClkGatingPgcbClkTrunk    : 1;     ///< GMM Clock Gating  - PGCB Clock Trunk: 0: Disable, 1: Enable;
  UINT32 ClkGatingSb              : 1;     ///< GMM Clock Gating  - Sideband: 0: Disable, 1: Enable;
  UINT32 ClkGatingSbClkTrunk      : 1;     ///< GMM Clock Gating  - Sideband Clock Trunk: 0: Disable, 1: Enable;
  UINT32 ClkGatingSbClkPartition  : 1;     ///< GMM Clock Gating  - Sideband Clock Partition: 0: Disable, 1: Enable;
  UINT32 ClkGatingCore            : 1;     ///< GMM Clock Gating  - Core: 0: Disable, 1: Enable;
  UINT32 ClkGatingDma             : 1;     ///< GMM Clock Gating  - DMA: 0: Disable, 1: Enable;
  UINT32 ClkGatingRegAccess       : 1;     ///< GMM Clock Gating  - Register Access: 0: Disable, 1: Enable;
  UINT32 ClkGatingHost            : 1;     ///< GMM Clock Gating  - Host: 0: Disable, 1: Enable;
  UINT32 ClkGatingPartition       : 1;     ///< GMM Clock Gating  - Partition: 0: Disable, 1: Enable;
  UINT32 ClkGatingTrunk           : 1;     ///< GMM Clock Gating  - Trunk: 0: Disable, 1: Enable;
  UINT32 SvPwrGatingHwAutoEnable  : 1;     ///< GMM Power Gating  - HW Autonomous Enabled: 0: Disable, 1: Enable;
  UINT32 SvPwrGatingD3HotEnable   : 1;     ///< GMM Power Gating  - D3 Hot Enabled: 0: Disable, 1: Enable;
  UINT32 SvPwrGatingI3Enable      : 1;     ///< GMM Power Gating  - I3 Enabled: 0: Disable, 1: Enable;
  UINT32 SvPwrGatingPmcReqEnable  : 1;     ///< GMM Power Gating  - PMC Request Enabled: 0: Disable, 1: Enable;
  UINT32  RsvdBits                : 17;    ///< Reserved bits
} SC_GMM_CONFIG;

#pragma pack (pop)

#endif // _GMM_CONFIG_H_
