/** @file
  Get Soc ID library implementation.

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
#ifndef _STEPPING_LIB_H_
#define _STEPPING_LIB_H_

#define BXT_FIB_DETECTION_SKPD_OFFSET 0x1408
///
/// BXT Series
///
typedef enum {
  Bxt1 = 0x01,
  BxtP = 0x03,
  BxtSeriesMax = 0xFF
} BXT_SERIES;

///
/// Silicon Steppings
///
typedef enum {
  BxtB0          = 0x10,
  BxtB1,
  BxtB2,
  BxtC0,
//[-start-160828-IB07400775-add]//
  BxtPA0         = 0x40,
//[-end-160828-IB07400775-add]//
  BxtPB0         = 0x50,
  BxtPB1,
  BxtPE0         = 0x60,
  BxtPF0         = 0x70,  
  BxtSteppingMax = 0xFF
} BXT_STEPPING;

///
/// BXT E0 FIB Types
///
typedef enum {
  BxtHardFib  = 0x00,
  BxtSoftFib  = 0x02,
  BxtNoFib    = 0xFF
} BXT_FIB_TYPE;

/**
   Return SOC series type

  @retval BXT_SERIES          SOC series type
**/
BXT_SERIES
EFIAPI
GetBxtSeries (
  VOID
  );

/**
  This procedure will get Soc Stepping

  @retval Soc Stepping
**/

BXT_STEPPING
EFIAPI
BxtStepping (
  VOID
);

BXT_FIB_TYPE
EFIAPI
GetFibType (
  VOID
  );

#endif // _STEPPING_LIB_H_
