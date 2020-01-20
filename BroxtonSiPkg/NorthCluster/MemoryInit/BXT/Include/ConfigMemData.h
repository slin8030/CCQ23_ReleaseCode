/** @file
  This file contains extended defination of ConfigMemData.c

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

#ifndef _CONFIGMEMDATA_H_
#define _CONFIGMEMDATA_H_

#include "Mrc.h"
#include "McFunc.h"
#include "MrcFunc.h"

#define TWO_GB_IN_MB  0x0800
#define FOUR_GB_IN_MB 0x1000

#pragma pack(1)

typedef enum {
  Trwsr = 0,
  Twrsr,
  Trrdr,
  Twwdr,
  Trwdr,
  Twrdr,
  Trrdd,
  Twwdd,
  Trwdd,
  Twrdd,
  MaxTimingGuardband
} TURNAROUND_TIMINGS;

extern UINT16 mDdrMemoryFrequencyTable[C_MAXDDR];
extern UINT16 mCoreFrequencyTable[C_MAXCOREFREQ];
extern UINT8  mDdrFreq2WLTable[C_MAXDDR];

extern UINT32 mDmapRegisters[MAX_DIMM_CONFIGS][4];
extern UINT32 mDimmConfigs[MAX_DIMM_CONFIGS];
extern UINT8  TimingGuardbands[MaxTimingGuardband][C_MAXDDR];
#pragma pack()

#endif
