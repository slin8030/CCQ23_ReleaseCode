/** @file
  Policy definition for NPK Pre-Mem Config Block

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
#ifndef _NPK_PRE_MEM_CONFIG_H_
#define _NPK_PRE_MEM_CONFIG_H_
#pragma pack(1)

#define NPK_PRE_MEM_CONFIG_REVISION 1

extern EFI_GUID gNpkPreMemConfigGuid;

typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Offset 0-23 Config Block Header
  UINT8                 NpkEn;
  UINT8                 NpkVrcTapEn;
  UINT8                 FwTraceEn;
  UINT8                 RecoverDump;
  UINT8                 FwTraceDestination;
  UINT32                Msc0Size;
  UINT8                 Msc0Wrap;
  UINT32                Msc1Size;
  UINT8                 Msc1Wrap;
  UINT8                 PtiMode;
  UINT8                 PtiTraining;
  UINT8                 PtiSpeed;
  UINT8                 PunitMlvl;
  UINT8                 PmcMlvl;
  UINT8                 SwTraceEn;
  UINT8                 NpkDCIEn;
} NPK_PRE_MEM_CONFIG;

#pragma pack()
#endif // _NPK_PRE_MEM_CONFIG_H_
