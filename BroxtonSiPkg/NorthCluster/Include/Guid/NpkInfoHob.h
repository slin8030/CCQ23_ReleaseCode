/** @file
  The GUID definition for SaDataHob

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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
#ifndef _NPK_INFO_HOB_H_
#define _NPK_INFO_HOB_H_

#include <Ppi/SaPolicy.h>

typedef struct _EFI_NORTH_PEAK_INFO_HOB {
  UINT64 MtbAddress;
  UINT32 MtbSize;
  UINT32 MtbWriteOffset;
  UINT64 MtbRecoveryAddress;
  UINT32 MtbRecoverySize;
  UINT32 MtbRecoveryWriteOffset;
  UINT8  MtbWrapStatus;
  UINT64 CsrAddress;
  UINT32 CsrSize;
  UINT32 CsrWriteOffset;
  UINT64 CsrRecoveryAddress;
  UINT32 CsrRecoverySize;
  UINT32 CsrRecoveryWriteOffset;
} EFI_NORTH_PEAK_INFO_HOB;


typedef struct _NPK_RESERVE_HOB {
  UINT32                MSC0Address;
  UINT32                MSC1Address;
  NPK_PRE_MEM_CONFIG    NpkPreMemConfig;
} NPK_RESERVE_HOB;

extern  EFI_GUID gEfiNorthPeakGuid;
extern  EFI_GUID gNPKReserveMemGuid;

#endif
