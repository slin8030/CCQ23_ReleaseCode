/** @file
  Constant and table defination for memory configuration.

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

#include "ConfigMemData.h"
#include "MmrcHooks.h"

/*UINT8 TimingGuardbands[MaxTimingGuardband][C_MAXDDR] = {
{2, 2, 2, 2, 2}, // TRWSR
{0, 0, 0, 0, 0}, // TWRSR
{0, 0, 0, 0, 0}, // TRRDR
{2, 2, 2, 2, 2}, // TWWDR
{1, 1, 1, 1, 1}, // TRWDR
{0, 0, 0, 0, 0}, // TWRDR
{0, 0, 0, 0, 0}, // TRRDD
{2, 2, 2, 2, 2}, // TWWDD
{1, 1, 1, 1, 1}, // TRWDD
{1, 1, 1, 1, 1}, // TWRDD
};
*/
UINT8 TimingGuardbands[MaxTimingGuardband][C_MAXDDR] = {
  {2, 2, 2, 2, 2}, // TRWSR
  {2, 2, 2, 2, 2}, // TWRSR
  {2, 2, 2, 2, 2}, // TRRDR
  {2, 2, 2, 2, 2}, // TWWDR
  {2, 2, 2, 2, 2}, // TRWDR
  {2, 2, 2, 2, 2}, // TWRDR
  {2, 2, 2, 2, 2}, // TRRDD
  {2, 2, 2, 2, 2}, // TWWDD
  {2, 2, 2, 2, 2}, // TRWDD
  {2, 2, 2, 2, 2}, // TWRDD
};

UINT16 mDdrMemoryFrequencyTable[C_MAXDDR] = {
  1600, 1866, 2133, 2400, 2666
};

UINT16 mCoreFrequencyTable[C_MAXCOREFREQ] = {
  800, 1066
};

UINT8 mDdrFreq2WLTable[C_MAXDDR] = {
  8, 9, 10, 11, 12
};

///
/// The 2nd dimension is for the density. tFAW changes if the density is under or over 8Gb.
///
UINT8 mFawTableDdr3[C_MAXDDR][2] = {
  {24, 32}, // 1600MHz
  {26, 33}, // 1866MHz
  {0, 0},   // 2133Hz  // N/A for DDR3
  {0, 0},   // 2400MHz // N/A for DDR3
  {0, 0}    // 2666MHz // N/A for DDR3
};

UINT8 mFawTableDdr4[C_MAXDDR][2] = {
  {16, 20}, // 1600MHz
  {16, 22}, // 1866MHz
  {16, 23}, // 2133MHz
  {16, 26}, // 2400MHz
  {0, 0}   // 2666MHz // DNV TODO - 2666 spec not complete yet. Fill in later.
};
