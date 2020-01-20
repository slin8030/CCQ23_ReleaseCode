/** @file
  This file contains definitions needed for porting the generic
  implementation of memory ODT.

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

#ifndef _MEMODT_H
#define _MEMODT_H

#include "DataTypes.h"
#include "Mmrc.h"

#pragma warning (disable : 4214 )

#define MAX_RANK_DIMM MAX_RANKS_PER_DIMM
///
/// ODT equates used to program mode registers
///
#define  DRAM_ODT_DIS     0
#define  RTT_WR_ODT_60    BIT9
#define  RTT_WR_ODT_120   BIT10
#define  RTT_WR_ODT_MASK  (BIT10 | BIT9)
#define  RTT_NOM_ODT_60   BIT2
#define  RTT_NOM_ODT_120  BIT6
#define  RTT_NOM_ODT_40   (BIT6 | BIT2)
#define  RTT_NOM_ODT_20   BIT9
#define  RTT_NOM_ODT_30   (BIT9 | BIT2)
#define  RTT_NOM_ODT_MASK (BIT9 | BIT6 | BIT2)

#define  SLOT_NOT_PRESENT  0
#define  EMPTY_DIMM        1
#define  SR_DIMM           2
#define  DR_DIMM           3
#define  QR_DIMM           4
#define  LR_DIMM           5

#define  DIMM0             0
#define  DIMM1             1
#define  DIMM2             2

#define  RANK0             0
#define  RANK1             1
#define  RANK2             2
#define  RANK3             3

#define  MCODT_60          0
#define  MCODT_120         1

#define  MCODT_ACT         1
#define  ODT_ACT           1
#define  NO_ACT            0

#define  RTT_WR_DIS        0
#define  RTT_WR_INF        0
#define  RTT_WR_60         BIT0
#define  RTT_WR_120        BIT1

#define  RTT_NOM_DIS       0
#define  RTT_NOM_INF       0
#define  RTT_NOM_60        BIT0
#define  RTT_NOM_120       BIT1
#define  RTT_NOM_40        (BIT1 | BIT0)
#define  RTT_NOM_20        BIT2
#define  RTT_NOM_30        (BIT2 | BIT0)

#define  DDR3_800          MMRC_800
#define  DDR3_1066         MMRC_1066
#define  DDR3_1333         MMRC_1333
#define  DDR3_1600         MMRC_1600
#define  DDR3_1866         MMRC_1866


typedef struct {
  UINT16 Config;                                  ///< ODT_VALUE_INDEX
  UINT8  McOdt;                                   ///< bit[0] = 0 for 50 ohms; 1 for 100 ohms
  UINT8  DramOdt[MAX_DIMMS][MAX_RANKS_PER_DIMM];  ///< bits[2:0] = Rtt_nom value matching JEDEC spec
                                                  ///< bits[5:4] = Rtt_wr value matching JEDEC spec
} ODT_VALUE_STRUCT;


typedef struct {
  UINT16 Config;                                  ///< ODT_ACT_INDEX
  UINT16 ActBits[2];                              ///< Bits[3:0]  = D0_R[3:0]
  // Bits[7:4]  = D1_R[3:0]
  // Bits[11:8] = D2_R[3:0]
  // Bit[15] = MCODT
} ODT_ACT_STRUCT;

typedef union {
  struct {
    UINT16 Freq: 4;     ///< Frequency enum - DDR3_800, 1067, 1333, 1600, 1867, 2133
    UINT16 Rsvd: 3;     ///< Reserved for now
    UINT16 Slot0: 3;    ///< DIMM slot 0 enum - Not present, Empty, SR, DR, QR, LR
    UINT16 Slot1: 3;    ///< DIMM slot 1 enum - Not present, Empty, SR, DR, QR, LR
    UINT16 Slot2: 3;    ///< DIMM slot 2 enum - Not present, Empty, SR, DR, QR, LR
  } Bits;
  UINT16 Data;
} ODT_VALUE_INDEX;

typedef union {
  struct {
    UINT16 DimmNum: 2;   ///< Target DIMM number on the channel
    UINT16 RankNum: 2;   ///< Target Rank number on the DIMM
    UINT16 Rsvd: 3;
    UINT16 Slot0: 3;     ///< DIMM slot 0 enum - Not present, Empty, SR, DR, QR, LR
    UINT16 Slot1: 3;     ///< DIMM slot 1 enum - Not present, Empty, SR, DR, QR, LR
    UINT16 Slot2: 3;     ///< DIMM slot 2 enum - Not present, Empty, SR, DR, QR, LR
  } Bits;
  UINT16 Data;
} ODT_ACT_INDEX;


#endif
