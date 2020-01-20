/** @file
  This file contains data structures for AVN silicon.
  These structures are fed to MmrcLibraries.c, which use them to initialize
  memory.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

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

#ifndef _MMRCPROJECTDATA_H_
#define _MMRCPROJECTDATA_H_

//
// R2R Switching, two hardcoded values are enable and disable, all other entries
// are valid GetSet indexes.  This is a 16-bit number.
//
#define R2R_ENABLE        0xFFFF
#define R2R_DISABLE       0xFFFE
//
// Bit field settings for determining which rd/wr training delay controls are shared.
//
#define RANK_SHARED       1
#define STROBE_SHARED     4
#define KNOB_SHARED       2
#define BIT_SHARED        8
#define BIT_CONTROL       16
#define TIMING_SHARED     128
#define NO_SHARED         0

#define _1X   0xFF
#define _18X  0xFD
#define _14X  0xFC
#define _38X  0xFB
#define _12X  0xFA
#define _58X  0xF9
#define _34X  0xF8

#define _n1X   0xF6
#define _n18X  0xF5
#define _n14X  0xF4
#define _n38X  0xF3
#define _n12X  0xF2
#define _n58X  0xF1
#define _n34X  0xF0


#include "MmrcProjectDataGenerated.h"
#include "MmrcProjectDefinitions.h"
#include "MmrcData.h"
#include "Printf.h"

#define T_DDR3L T_DDR3
#define SPEED_4X          4
#define SPEED_1X          1
#define MAX_FAILURE_LIST  50

#define GUARDBAND 0
#define UnusedOffset 0xFF

#ifndef MAX_EYEMASKS
#define CMD_NUM_EYEMASKS 2
#define MAX_EYEMASKS MAX(MAX(RT_NUM_EYEMASKS, WT_NUM_EYEMASKS), CMD_NUM_EYEMASKS)
UINT8 EyeMaskPctPiCmdCtl[NUM_TYPE][NUM_FREQ][CMD_NUM_EYEMASKS];
UINT8 EyeMaskPctPiRd[NUM_TYPE][NUM_FREQ][RT_NUM_EYEMASKS];
UINT8 EyeMaskPctPiWr[NUM_TYPE][NUM_FREQ][WT_NUM_EYEMASKS];
#endif

extern CONST UINT16           Timing2x[];
extern CONST UINT8            LinearToPhysicalVrefCodes[MAX_ELEMENT_TABLE];
extern CONST UINT8            PhysicalToLinearVrefCodes[MAX_ELEMENT_TABLE];
extern       RMT_SIGNAL       RmtTable[MAX_RMT_ELEMENTS];
extern       RMT_SIGNAL       RmtTableProduction[MAX_PRODUCTION_RMT_ELEMENTS];
extern       RMT_SIGNAL       RmtTablePpv[MAX_PPV_RMT_ELEMENTS];
extern CONST UINT32           MaxDelayFor4x[NUM_FREQ];
//extern CONST DELAY_PASS_GATE  DelayPassGate [2][NUM_FREQ];

extern MMRC_STATUS SimSetDunitConfigValid (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS VerifyMR8onRAM         (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS ProgramMemoryMap       (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS SetScrambler           (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS ReadMRTQREF            (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS L_CpgcDisable          (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS L_CpgcEnable           (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS L_CpgcInit             (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
extern MMRC_STATUS SetMrcDoneForVP        (MMRC_DATA *MrcData);
extern MMRC_STATUS CheckDQSwizzle         (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);

UINT8  ConfigToSpeed[MaxMemoryTypeFreqConfig];
UINT8  ConfigToDdrType[MaxMemoryTypeFreqConfig];
UINT8  ConfigToSpeedMultiple133[MaxMemoryTypeFreqConfig];
UINT8  ConvertMultiple133toSpeed[25];
extern UINT8  TimingParamsUnder256[][MaxMemoryTypeFreqConfig];
extern UINT16 TimingParamsOver255[][MaxMemoryTypeFreqConfig];
extern UINT8 TimingParamsUnder256Size;
extern UINT8 TimingParamsOver255Size;
UINT16 TCK[C_MAXDDR];
UINT16 SPIDCLK_DDR3_LP3[C_MAXDDR];
UINT16 SPIDCLK_LP4[C_MAXDDR];

extern UINT16 DramVrefOvr[NUM_TYPE][NUM_FREQ];
extern UINT32 DramOdt[NUM_TYPE][NUM_FREQ];
extern CLOCK_CROSSINGS                  ClockCrossings_BX_LPDDR3_DDR3L[MAX_NUM_CC_RANGES];
extern CLOCK_CROSSINGS                  ClockCrossings_AX_ALL_BX_LPDDR4[MAX_NUM_CC_RANGES];

typedef struct {
  UINT8 Density;
  UINT8 DramSpeedAndGradeA;   // For DVFS configs, this is the max performance setting.
  UINT8 DramSpeedAndGradeB;   // For DVFS configs, this is the min performance setting.
} DRAM_INFO;


#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
/* LPDDR3 JEDEC Commands */
#define LPDDR3_JEDEC_CMD_NOP                       0x00000007  /**< NOP Command (CA0=1, CA1=1, CA2=1, CA3=0) */
#define LPDDR3_JEDEC_CMD_MRW                       0x00000000  /**< MRW Command (CA0=0, CA1=0, CA2=0, CA3=0) */
#define LPDDR3_JEDEC_CMD_MRR                       0x00000008  /**< MRR Command (CA0=0, CA1=0, CA2=0, CA3=1) */
#define LPDDR3_JEDEC_CMD_PRECHARGE_ALL             0x0000001B  /**< PRECHARGE ALL Command (CA0=1, CA1=1, CA2=0, CA3=1, CA4r=1) */
#define LPDDR3_JEDEC_CMD_MRW_RESET                 0x000003F0  /**< MRW(Reset) Command (CA0=0, CA1=0, CA2=0, CA3=0) */
#define LPDDR3_JEDEC_CMD_MRW_ZQCAL                 0x000FF0A0

#define LPDDR3_JEDEC_MR10              (0xA << 4)
#define LPDDR3_JEDEC_MR01              (0x1 << 4)
#define LPDDR3_JEDEC_MR02              (0x2 << 4)
#define LPDDR3_JEDEC_MR03              (0x3 << 4)
#define LPDDR3_JEDEC_MR11              (0xb << 4)

#define LPDDR3_JEDEC_MR1_BL8            (0x3)
#define LPDDR3_JEDEC_MR1_BL16           (0x4)
#define LPDDR3_JEDEC_MR1_BT_SEQ         (0x0 << 3)
#define LPDDR3_JEDEC_MR1_BT_INT         (0x1 << 3)
#define LPDDR3_JEDEC_MR1_WC             (0x0 << 4)
#define LPDDR3_JEDEC_MR1_NWC            (0x1 << 4)
#define LPDDR3_JEDEC_MR1_nWR10          (0x0 << 5)
#define LPDDR3_JEDEC_MR1_nWR11          (0x1 << 5)
#define LPDDR3_JEDEC_MR1_nWR12          (0x2 << 5)
#define LPDDR3_JEDEC_MR1_nWR5           (0x3 << 5)
#define LPDDR3_JEDEC_MR1_nWR6           (0x4 << 5)
#define LPDDR3_JEDEC_MR1_nWR7           (0x5 << 5)
#define LPDDR3_JEDEC_MR1_nWR8           (0x6 << 5)

#define LPDDR3_JEDEC_MR2_RL3WL1         (0x1)
#define LPDDR3_JEDEC_MR2_RL6WL3         (0x4)
#define LPDDR3_JEDEC_MR2_RL8WL4         (0x6)
#define LPDDR3_JEDEC_MR2_RL9WL5         (0x7)
#define LPDDR3_JEDEC_MR2_RL10WL6        (0x8)
#define LPDDR3_JEDEC_MR2_RL11WL6        (0x9)
#define LPDDR3_JEDEC_MR2_RL12WL6        (0xA)
#define LPDDR3_JEDEC_MR2_WRE_LT9        (0x0 << 4)
#define LPDDR3_JEDEC_MR2_WRE_GT9        (0x1 << 4)
#define LPDDR3_JEDEC_MR2_WL_SETA        (0x0 << 6)
#define LPDDR3_JEDEC_MR2_WL_SETB        (0x1 << 6)
#define LPDDR3_JEDEC_MR2_WRLVL_DIS      (0x0 << 7)
#define LPDDR3_JEDEC_MR2_WRLVL_ENB      (0x1 << 7)

#define LPDDR3_JEDEC_MR3_OHM_343        (0x1)
#define LPDDR3_JEDEC_MR3_OHM_40         (0x2)
#define LPDDR3_JEDEC_MR3_OHM_48         (0x3)
#define LPDDR3_JEDEC_MR3_OHM_60         (0x4)
#endif
#endif

/**
TimingParamHash

  @param[in]      InputProfile      The requested index value to access the current profile of settings for DdrType and Frequency.
  @param[in, out] OutputProfile     A pointer to the new index value.

  @retval         Status

**/
extern
MMRC_STATUS
TimingParamHash (
  UINT8   InputProfile,
  UINT8   *OutputProfile
);
