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

#include "MmrcProjectData.h"
#include "MmrcLibraries.h"

CONST UINT16 Timing2x[]= {
  1250,     // 800
  938,      // 1066
  750,      // 1333
  625,      // 1600
  468,      // 2133
  416,      // 2400
  375,      // 2667
  341,      // 2933
  312       // 3200
};



CONST UINT8 LinearToPhysicalVrefCodes[MAX_ELEMENT_TABLE] = { // lowest to highest
  0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20, 0x00, 0x01, // 00 - 09
  0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, // 10 - 19
  0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, // 20 - 29
  0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 30 - 39
};

CONST UINT8 PhysicalToLinearVrefCodes[MAX_ELEMENT_TABLE] = { // lowest to highest
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, // 00 - 09
  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, // 10 - 19
  0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, // 20 - 29
  0x26, 0x27, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, // 30 - 39
};

#if RMT
RMT_SIGNAL RmtTable[MAX_RMT_ELEMENTS] = {
  //{Signal,    SkipInitialProgrammming,  CpgcConfig,       StepSize,           RankChoices,               JedecrestType  }
  {RxDqsPDelay,       FALSE,          CPGC_CONFIG_VA,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {RxDqsNDelay,       FALSE,          CPGC_CONFIG_VA,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {RxVref,            FALSE,          CPGC_CONFIG_VA,        1,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {TxDqDelay,         FALSE,          CPGC_CONFIG_VA,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {TxVref,            FALSE,          CPGC_CONFIG_VA,        1,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },   // LPDDR4 step size is 4x of the specified value
  {CmdAll,            TRUE,           CPGC_CONFIG_CMD_STRESS,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_FAIL_RESTORE_PI_CENTER|RMT_FAIL_JEDEC_RESET|RMT_FAIL_RESTORE_PI_LAST_VALUE     },
  //{CkAll,             TRUE,           CPGC_CONFIG_VA,        5,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_FAIL_RESTORE_PI_CENTER|RMT_FAIL_JEDEC_RESET|RMT_FAIL_RESTORE_PI_LAST_VALUE     },
  //{CtlAll,          TRUE,           CPGC_CONFIG_VA,        5,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_FAIL_RESTORE_PI_CENTER|RMT_FAIL_JEDEC_RESET|RMT_FAIL_RESTORE_PI_LAST_VALUE     },
  {MMRC_NA,           MMRC_NA,        MMRC_NA,               MMRC_NA,     MMRC_NA,                    MMRC_NA } ,
};

// In the production RMT table, the "StepSize" is actually the limit.
// Currently the stepsizes are set as per the recommendation provided by IOTG EV
// for the margin check feature
RMT_SIGNAL RmtTableProduction[MAX_PRODUCTION_RMT_ELEMENTS] = {
  {RxDqsPDelay,       FALSE,          CPGC_CONFIG_VA,        10,      RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {RxDqsNDelay,       FALSE,          CPGC_CONFIG_VA,        10,      RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {TxDqDelay,         FALSE,          CPGC_CONFIG_VA,        10,      RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {RxVref,            FALSE,          CPGC_CONFIG_VA,        5,       RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {TxVref,            FALSE,          CPGC_CONFIG_VA,        11,      RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },   // LPDDR4 step size is 4x of the specified value
  {MMRC_NA,           MMRC_NA,        MMRC_NA,               MMRC_NA, MMRC_NA,                                           MMRC_NA           },
};

RMT_SIGNAL RmtTablePpv[MAX_PPV_RMT_ELEMENTS] = {
  //{Signal,    SkipInitialProgrammming,  CpgcConfig,       StepSize,           RankChoices,               JedecrestType  }
  {RxDqsPDelay,       FALSE,          CPGC_CONFIG_VA,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {RxDqsNDelay,       FALSE,          CPGC_CONFIG_VA,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {TxDqDelay,         FALSE,          CPGC_CONFIG_VA,        2,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {RxVref,            FALSE,          CPGC_CONFIG_VA,        1,    RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {TxVref,            FALSE,          CPGC_CONFIG_VA,        20,   RMT_PR_EACH_RANK|RMT_PR_LFSR|RMT_PR_TA,            RMT_NO_ACTION     },
  {MMRC_NA,           MMRC_NA,        MMRC_NA,               MMRC_NA,     MMRC_NA,                    MMRC_NA } ,
};
#endif


//tCK in ps
UINT16 TCK[C_MAXDDR] = {
//800   1066  1333  1600  1866  2133 2400 2666 3200 2933
  2500, 1876, 1500, 1250, 1071, 938, 833, 750, 625//, 682
};

// SPID CLK in MHz
UINT16 SPIDCLK_LP4[C_MAXDDR] = {
//800   1066  1333  1600  1866  2133 2400 2666 3200 2933
  100,  133,  166,  200,  233,  267, 300, 333, 400
};

UINT16 SPIDCLK_DDR3_LP3[C_MAXDDR] = {
//800   1066  1333  1600  1866  2133 2400 2666 3200 2933
  200,  266,  333,  400,  466,  533, 600, 666, 800
};



//
// Depending on the configuration of memory, this table returns the speed associated with that config.
//

UINT8 ConfigToSpeed[MaxMemoryTypeFreqConfig] = {
  UnusedOffset, MMRC_1333, MMRC_1600, MMRC_1866,
  MMRC_1600, MMRC_2133, MMRC_2400, MMRC_2666,
  MMRC_1333, MMRC_1333, MMRC_1333, MMRC_1333, MMRC_1600, MMRC_1600, MMRC_1600, MMRC_1600,
  MMRC_1866, MMRC_1866, MMRC_1866, MMRC_1866,
};

//
// Depending on the configuration of memory, this table returns the speed associated with that config.
//
UINT8 ConfigToSpeedMultiple133[MaxMemoryTypeFreqConfig] = {
  UnusedOffset, 10, 12, 14,
  12, 16, 18, 20,
  10, 10, 10, 10, 12, 12, 12, 12,
  14, 14, 14, 14,
 };

UINT8 ConvertMultiple133toSpeed[25] = {
  0, 0,0,0,0,0,0,0,
  MMRC_1066, 0, MMRC_1333, 0, MMRC_1600, 0, MMRC_1866, 0, MMRC_2133, 0, MMRC_2400, 0, MMRC_2666, 0, MMRC_2933, 0, MMRC_3200
};

//
// Depending on the configuration of memory, this table returns the speed associated with that config.
//
UINT8 ConfigToDdrType[MaxMemoryTypeFreqConfig] = {
  UnusedOffset, TypeLpDdr3, TypeLpDdr3, TypeLpDdr3,
  TypeLpDdr4, TypeLpDdr4, TypeLpDdr4, TypeLpDdr4,
  TypeDdr3L, TypeDdr3L, TypeDdr3L, TypeDdr3L, TypeDdr3L, TypeDdr3L, TypeDdr3L, TypeDdr3L,
  TypeDdr3L, TypeDdr3L, TypeDdr3L, TypeDdr3L,
};

//
// TimingParamsUnder256[][MaxMemoryTypeFreqConfig]
// - This table holds timing paramters that can be described with a UINT8 data type.
//   This means that values must be less than 256
//
UINT8 TimingParamsUnder256[][MaxMemoryTypeFreqConfig] = {
//Remark:
//13=4-03-2014: Txp LPDDR4 14-15-15 Excel:6 Sim Model: 8
//13=4-03-2014: Tfaw LPDDR4 14-15-15 Excel:32 Sim Model: 40
//13=4-03-2014: Trwdr LPDDR4 14-15-15 Excel:23 Sim Model: 24
  //
  // Configs supported:
  //                        LPDDR3  LPDDR3  LPDDR3  LPDDR4  LPDDR4  LPDDR4  LPDDR4  DDR3    DDR3    DDR3    DDR3    DDR3    DDR3    DDR3    DDR3    DDR3    DDR3    DDR3    DDR3
  //                        1333    1600    1866    1600    2133    2400    2667    1333    1333    1333    1333    1600    1600    1600    1600    1866    1866    1866    1866
  //
  //                        10      12      14      14      20      24      24      7       8       9       10      8       9       10      11      10      11      12      13
  //                        12      15      17      15      20      22      24      7       8       9       10      8       9       10      11      10      11      12      13
  //                        12      15      17      15      20      22      24      7       8       9       10      8       9       10      11      10      11      12      13

{  BxtTimingtCL,            10,     12,     14,     14,     20,     24,     24,     7,      8,      9,      10,     8,      9,      10,     11,     10,     11,     12,     13,     },
{  BxtTimingtCWL,           8,      9,      11,     12,     18,     22,     22,     7,      7,      7,      7,      8,      8,      8,      8,      9,      9,      9,      9,      },
{  BxtTimingtRCD,           12,     15,     17,     15,     20,     22,     24,     7,      8,      9,      10,     8,      9,      10,     11,     10,     11,     12,     13,     },
{  BxtTimingtRP,            12,     15,     17,     15,     20,     22,     24,     7,      8,      9,      10,     8,      9,      10,     11,     10,     11,     12,     13,     },
{  BxtTimingtRPab,          14,     17,     20,     17,     23,     26,     28,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtRAS,           28,     34,     40,     34,     45,     51,     56,     24,     24,     24,     24,     28,     28,     28,     28,     32,     32,     32,     32,     },
{  BxtTimingtRC,            40,     49,     57,     49,     65,     73,     80,     31,     32,     33,     34,     36,     37,     38,     39,     42,     43,     44,     45,     },
{  BxtTimingtRCab,          42,     51,     60,     51,     68,     77,     84,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtRRD,           7,      8,      10,     8,      11,     12,     14,     5,      5,      5,      5,      6,      6,      6,      6,      7,      7,      7,      7,      },
{  BxtTimingtRRD_L,         0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtFAW,           0,      0,      0,      32,     43,     48,     54,     30,     30,     30,     30,     32,     32,     32,     32,     38,     38,     38,     38,     },
{  BxtTimingtCCD,           4,      4,      4,      8,      8,      8,      8,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      },
{  BxtTimingtCCD_L,         0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtRTP,           5,      6,      7,      8,      8,      9,      10,     5,      5,      5,      5,      6,      6,      6,      6,      7,      7,      7,      7,      },
{  BxtTimingtWTR,           5,      6,      7,      8,      11,     12,     14,     5,      5,      5,      5,      6,      6,      6,      6,      7,      7,      7,      7,      },
{  BxtTimingtWTR_L,         0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtWR,            10,     12,     14,     20,     24,     24,     30,     10,     10,     10,     10,     12,     12,     12,     12,     14,     14,     14,     14,     },
{  BxtTimingtRDPRE,         5,      6,      7,      8,      8,      9,      10,     5,      5,      5,      5,      6,      6,      6,      6,      7,      7,      7,      7,      },
{  BxtTimingtWRPRE,         23,     26,     30,     36,     47,     53,     55,     21,     21,     21,     21,     24,     24,     24,     24,     27,     27,     27,     27,     },

{  BxtTimingtXS_offset,     7,      8,      10,     8,      11,     12,     14,     7,      7,      7,      7,      8,      8,      8,      8,      10,     10,     10,     10,     },
{  BxtTimingtCKESR,         10,     12,     14,     6,      8,      9,      10,     5,      5,      5,      5,      5,      5,      5,      5,      6,      6,      6,      6,      },
{  BxtTimingtXP,            5,      6,      7,      6,      8,      9,      10,     4,      4,      4,      4,      5,      5,      5,      5,      6,      6,      6,      6,      },
{  BxtTimingtXPDLL,         0,      0,      0,      0,      0,      0,      0,      16,     16,     16,     16,     20,     20,     20,     20,     23,     23,     23,     23,     },
{  BxtTimingtCKE,           5,      6,      7,      6,      8,      9,      10,     4,      4,      4,      4,      4,      4,      4,      4,      5,      5,      5,      5,      },
{  BxtTimingtPRPDEN,        1,      1,      1,      2,      2,      2,      2,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      1,      },
{  BxtTimingtRDPDEN,        19,     22,     25,     26,     33,     38,     38,     12,     13,     14,     15,     13,     14,     15,     16,     15,     16,     17,     18,     },
{  BxtTimingtWRPDEN,        23,     26,     30,     37,     48,     54,     56,     21,     21,     21,     21,     24,     24,     24,     24,     27,     27,     27,     27,     },
{  BxtTimingtCPDED,         2,      2,      2,      2,      2,      2,      2,      1,      1,      1,      1,      1,      1,      1,      1,      2,      2,      2,      2,      },
{  BxtTimingtMOD,           10,     12,     14,     12,     15,     17,     19,     12,     12,     12,     12,     12,     12,     12,     12,     14,     14,     14,     14,     },
{  BxtTimingtAONPD,         0,      0,      0,      0,      0,      0,      0,      6,      6,      6,      6,      7,      7,      7,      7,      8,      8,      8,      8,      },
{  BxtTimingtRDRD,          4,      4,      4,      8,      8,      8,      8,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      },
{  BxtTimingtRDRD_L,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtRDRD_dr,       8,      8,      8,      12,     13,     14,     14,     6,      6,      6,      6,      6,      6,      6,      6,      6,      6,      6,      6,      },
{  BxtTimingtRDRD_dd,       0,      0,      0,      0,      0,      0,      0,      7,      7,      7,      7,      7,      7,      7,      7,      7,      7,      7,      7,      },
{  BxtTimingtWRWR,          4,      4,      4,      8,      8,      8,      8,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      4,      },
{  BxtTimingtWRWR_L,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtWRWR_dr,       6,      6,      9,      16,     15,     17,     17,     7,      7,      7,      7,      6,      6,      6,      6,      6,      6,      6,       6,     },
{  BxtTimingtWRWR_dd,       0,      0,      0,      0,      0,      0,      0,      7,      7,      7,      7,      7,      7,      7,      7,      10,     10,     10,     10,     },
{  BxtTimingtWRRD,          18,     20,     23,     29,     38,     43,     45,     18,     18,     18,     18,     20,     20,     20,     20,     22,     22,     22,     22,     },
{  BxtTimingtWRRD_L,        0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtWRRD_dr,       3,      2,      4,      12,     13,     13,     13,     6,      5,      4,      3,      6,      5,      4,      3,      5,      4,      3,      2,      },
{  BxtTimingtWRRD_dd,       0,      0,      0,      0,      0,      0,      0,      6,      5,      4,      3,      6,      5,      4,      3,      5,      4,      3,      2,      },
{  BxtTimingtRDWR,          11,     13,     16,     22,     19,     21,     21,     7,      8,      9,      10,     7,      8,      9,      10,     8,      9,      10,     11,     },
{  BxtTimingtRDWR_dr,       11,     13,     16,     18,     19,     21,     21,     7,      8,      9,      10,     7,      8,      9,      10,     8,      9,      10,     11,     },
{  BxtTimingtRDWR_dd,       0,      0,      0,      0,      0,      0,      0,      7,      8,      9,      10,     7,      8,      9,      10,     8,      9,      10,     11,     },

{  BxtTimingtZQCS,          60,     72,     84,     0,      0,      0,      0,      64,     64,     64,     64,     64,     64,     64,     64,     75,     75,     75,     75,     },
{  BxtTimingtZQLAT,         0,      0,      0,      24,     32,     36,     40,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },

{  BxtTimingtCCDMW,         0,      0,      0,      32,     32,     32,     32,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtCKSRE,         2,      2,      2,      5,      6,      6,      7,      7,      7,      7,      7,      8,      8,      8,      8,      10,     10,     10,     10,     },
{  BxtTimingtCKSRX,         2,      2,      2,      3,      3,      3,      3,      7,      7,      7,      7,      8,      8,      8,      8,      10,     10,     10,     10,     },
{  BxtTimingtRPPB,          12,     15,     17,     15,     20,     22,     24,     0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },

{  BxtTimingtXSR_4G,        94,     112,    132,    110,    147,    165,    184,    0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtXS_1G,         0,      0,      0,      0,      0,      0,      0,      81,     81,     81,     81,     96,     96,     96,     96,     113,    113,    113,    113,    },
{  BxtTimingtXS_2G,         0,      0,      0,      0,      0,      0,      0,      114,    114,    114,    114,    136,    136,    136,    136,    160,    160,    160,    160,    },

{  BxtTimingtFAW1K,         34,     40,     47,     32,     43,     48,     54,     20,     20,     20,     20,     24,     24,     24,     24,     26,     26,     26,     26,     },
{  BxtTimingtFAW2K,         0,      0,      0,      32,     43,     48,     54,     30,     30,     30,     30,     32,     32,     32,     32,     33,     33,     33,     33,     },
};

//
// TimingParamsUnder256[][MaxMemoryTypeFreqConfig]
// - This table holds timing paramters that can be described with a UINT16 data type but not a UINT8
//
UINT16 TimingParamsOver255[][MaxMemoryTypeFreqConfig] = {
{  BxtTimingtREFI,          2600,   3120,   3640,   3125,   4167,   4688,  5208,  5200,   5200,   5200,   5200,   6240,   6240,   6240,   6240,   7280,   7280,   7280,   7280,   },
{  BxtTimingtXS,            0,      0,      0,      0,      0,      0,     0,     207,    207,    207,    207,    248,    248,    248,    248,    290,    290,    290,    290,    },
{  BxtTimingtXSDLL,         0,      0,      0,      0,      0,      0,     0,     512,    512,    512,    512,    512,    512,    512,    512,    512,    512,    512,    512,    },
{  BxtTimingtZQOPER,        240,    288,    336,    800,    1067,   1200,  1334,  256,    256,    256,    256,    256,    256,    256,    256,    299,    299,    299,    299,    },
{  BxtTimingtZQCL,          667,    800,    934,    0,      0,      0,     0,     512,    512,    512,    512,    512,    512,    512,    512,    598,    598,    598,    598,    },
{  BxtTimingtRFC_4G,        87,     104,    122,    104,    139,    156,   174,   200,    200,    200,    200,    208,    208,    208,    208,    243,    243,    243,    243,    },
{  BxtTimingtRFC_6G,        140,    168,    196,    144,    192,    216,   240,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtRFC_8G,        140,    168,    196,    144,    192,    216,   240,   234,    234,    234,    234,    280,    280,    280,    280,    327,    327,    327,    327,    },
{  BxtTimingtRFC_12G,       187,    224,    262,    224,    299,    336,   374,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtRFC_16G,       187,    224,    262,    224,    299,    336,   374,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtXSR_6G,        147,    176,    206,    150,    200,    225,   250,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtXSR_8G,        147,    176,    206,    150,    200,    225,   250,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtXSR_12G,       194,    232,    272,    230,    307,    345,   384,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtXSR_16G,       194,    232,    272,    230,    307,    345,   384,   0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      0,      },
{  BxtTimingtXS_4G,         0,      0,      0,      0,      0,      0,       0,   207,    207,    207,    207,    248,    248,    248,    248,    290,    290,    290,    290,    },
{  BxtTimingtXS_8G,         0,      0,      0,      0,      0,      0,       0,   241,    241,    241,    241,    288,    288,    288,    288,    337,    337,    337,    337,    },

};

UINT8 TimingParamsUnder256Size = (sizeof(TimingParamsUnder256) / sizeof(TimingParamsUnder256[0]));
UINT8 TimingParamsOver255Size = (sizeof(TimingParamsOver255) / sizeof(TimingParamsOver255[0]));

UINT8 NumberEyeMaskRead = RT_NUM_EYEMASKS;

EYEMASK_T    EyeMaskRead[] = {
  {RT_EYEMASK_VREF, RT_EYEMASK0_PI},
  {RT_EYEMASK_VREF, RT_EYEMASK1_PI}
};

UINT8 NumberEyeMaskWrite = WT_NUM_EYEMASKS;

EYEMASK_T    EyeMaskWrite[] = {
  {WT_EYEMASK_VREF, WT_EYEMASK0_PI},
  {WT_EYEMASK_VREF, WT_EYEMASK1_PI}
};

UINT8 NumberEyeMaskCmd = CMD_NUM_EYEMASKS;

EYEMASK_T    EyeMaskCmd[] = {
  {3, 40},
  {3, 25}
};

UINT8 NumberControlKnobsRead = 2 + RT_PN_ENABLED + RT_CTLE_ENABLE * 2;

UINT8 ControlVrefsRead[] = {RT_STEP2_VREFMIN, RT_STEP2_VREFMIN, RT_STEP2_VREFMIN, RT_STEP3_VREFSAMPLES};
UINT8 ControlVrefsWrite[] = {RT_STEP2_VREFMIN, RT_STEP2_VREFMIN, RT_STEP2_VREFMIN, RT_STEP3_VREFSAMPLES};
UINT8 ControlVrefsCmd[] = {RT_STEP2_VREFMIN, RT_STEP2_VREFMIN, RT_STEP2_VREFMIN, RT_STEP3_VREFSAMPLES};

#if RT_PN_ENABLED==1
CONTROLKNOBS ControlKnobsReadDDR3[] = {
  // Control                   Start       Range      Step     Rel/Abs     Byte Shared                        Bit Shared
  //  Knob                     Value       Value      Value    Flags         Flags                              Flag
  {RxDqsPDelay, RxDqsDelay, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 2, 5, GenericMemoryTest},
  {RxDqsNDelay, RxDqsDelay, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 2, 5, GenericMemoryTest},
  {RxVref    , -1,  -1          , {RT_STEP1_VREFSTART_DDR3 ,  RT_STEP1_VREFSTART_DDR3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED | TIMING_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 0, 63, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3, GenericMemoryTest},
//  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                        , RT_BIT_SHARING, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5},
//  {RxVref    , -1, -1          , {RT_STEP1_VREFSTART_DDR3 ,  RT_STEP1_VREFSTART_DDR3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 5, 50, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3},
#if RT_CTLE_ENABLE==1
  {Ctle_Res  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cc", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
  {Ctle_Cap  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cr", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
#endif
};
#else
CONTROLKNOBS ControlKnobsReadDDR3[] = {
  // Control                   Start       Range      Step     Rel/Abs     Byte Shared                        Bit Shared
  //  Knob                     Value       Value      Value    Flags         Flags                              Flag
  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5, GenericMemoryTest},
  {RxVref    , -1,  -1          , {RT_STEP1_VREFSTART_DDR3 ,  RT_STEP1_VREFSTART_DDR3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED | TIMING_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 0, 63, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3, GenericMemoryTest},
//  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                        , RT_BIT_SHARING, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5},
//  {RxVref    , -1, -1          , {RT_STEP1_VREFSTART_DDR3 ,  RT_STEP1_VREFSTART_DDR3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 5, 50, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3},
#if RT_CTLE_ENABLE==1
  {Ctle_Res  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cc", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
  {Ctle_Cap  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cr", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
#endif
};
#endif

#if RT_PN_ENABLED==1
CONTROLKNOBS ControlKnobsReadLP3[] = {
  // Control                   Start       Range      Step     Rel/Abs     Byte Shared                        Bit Shared
  //  Knob                     Value       Value      Value    Flags         Flags                              Flag
  {RxDqsPDelay, RxDqsDelay, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 2, 5, GenericMemoryTest},
  {RxDqsNDelay, RxDqsDelay, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 2, 5, GenericMemoryTest},
  {RxVref    , -1,  -1          , {RT_STEP1_VREFSTART_LP3 ,  RT_STEP1_VREFSTART_LP3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED | TIMING_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 0, 63, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3, GenericMemoryTest},
//  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                        , RT_BIT_SHARING, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5},
  //{RxVref    , -1, -1          , {RT_STEP1_VREFSTART_LP3 ,  RT_STEP1_VREFSTART_LP3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 5, 50, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3},
#if RT_CTLE_ENABLE==1
  {Ctle_Res  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cc", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
  {Ctle_Cap  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cr", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
#endif
};
#else
CONTROLKNOBS ControlKnobsReadLP3[] = {
  // Control                   Start       Range      Step     Rel/Abs     Byte Shared                        Bit Shared
  //  Knob                     Value       Value      Value    Flags         Flags                              Flag
  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5, GenericMemoryTest},
  {RxVref    , -1,  -1          , {RT_STEP1_VREFSTART_LP3 ,  RT_STEP1_VREFSTART_LP3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED | TIMING_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 0, 63, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3, GenericMemoryTest},
//  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                        , RT_BIT_SHARING, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5},
  //{RxVref    , -1, -1          , {RT_STEP1_VREFSTART_LP3 ,  RT_STEP1_VREFSTART_LP3}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 5, 50, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3},
#if RT_CTLE_ENABLE==1
  {Ctle_Res  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cc", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
  {Ctle_Cap  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cr", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
#endif
};
#endif

#if RT_PN_ENABLED==1
CONTROLKNOBS ControlKnobsReadLP4[] = {
  // Control                   Start       Range      Step     Rel/Abs     Byte Shared                        Bit Shared
  //  Knob                     Value       Value      Value    Flags         Flags                              Flag
  {RxDqsPDelay, RxDqsDelay, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 2, 5, GenericMemoryTest},
  {RxDqsNDelay, RxDqsDelay, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 2, 5, GenericMemoryTest},
  {RxVref    , -1,  -1          , {RT_STEP1_VREFSTART_LP4 ,  RT_STEP1_VREFSTART_LP4}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED | TIMING_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 0, 63, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3, GenericMemoryTest},
#if RT_CTLE_ENABLE==1
  {Ctle_Res  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cc", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
  {Ctle_Cap  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cr", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
#endif
};
#else
CONTROLKNOBS ControlKnobsReadLP4[] = {
  // Control                   Start       Range      Step     Rel/Abs     Byte Shared                        Bit Shared
  //  Knob                     Value       Value      Value    Flags         Flags                              Flag
  {RxDqsDelay, -1, RxDqBitDelay, {0 , _12X},                                0xf, {0,  _12X}                           , { RT_STEP1_3_PISTEPLARGE, RT_STEP1_3_PISTEPMEDIUM, RT_STEP1_3_PISTEPSMALL}, MMRC_ABS, RT_BYTE_SHARING                      , RT_BIT_SHARING|TIMING_SHARED, "PI \0", 2+2*RT_CTLE_ENABLE-1, 0,  0, 0, -1, READTRAINING_ENTRY, FALSE, 1, 5, GenericMemoryTest},
  {RxVref    , -1,  -1          , {RT_STEP1_VREFSTART_LP4 ,  RT_STEP1_VREFSTART_LP4}, -1, {RT_STEP1_VREFMIN,  RT_STEP1_VREFMAX}, { RT_STEP2_VREFSTEP, 0, 0}                                                , MMRC_ABS, RANK_SHARED | KNOB_SHARED | BIT_SHARED | TIMING_SHARED , -1           , "VRF\0", 2+2*RT_CTLE_ENABLE-1, 0, 63, 5, RT_STEP1_VREFSTEP,READTRAINING_ENTRY, FALSE, 1, 3, GenericMemoryTest},
#if RT_CTLE_ENABLE==1
  {Ctle_Res  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cc", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
  {Ctle_Cap  , -1          , {0                  ,  3}                 , -1, { 0              ,  3                }, { 1, 0, 0}, MMRC_ABS, RT_CTLE_SHARED, -1, "Cr", 2+2*RT_CTLE_ENABLE-1, 0, 0, 0, 1,READTRAINING_ENTRY, FALSE, GenericMemoryTest},
#endif
};
#endif

UINT8 NumberControlKnobsWrite = 2;

CONTROLKNOBS ControlKnobsWriteLP4[] = {
  // Control                   Start                                       Range              Step                                                                      Rel/Abs      Shared
  //  Knob                     Value             Value                     Value    Flags                                                            Flags
  {TxDqDelay,   -1, TxDqBitDelay, {_n12X,_12X}                             , 31, {_n12X,  _12X}                       , { WT_STEP1_3_PISTEPLARGE, WT_STEP1_3_PISTEPMEDIUM, WT_STEP1_3_PISTEPSMALL}, REL     , WT_BYTE_SHARING     , WT_BIT_SHARING, "PI \0", 2-1, 0,  0,                 0, -1,READTRAINING_ENTRY, TRUE, 1, 5, GenericMemoryTest},
  {TxVref,      -1,         -1, {WT_STEP1_VREFSTART_LP4 ,  WT_STEP1_VREFSTART_LP4}, -1, {WT_STEP1_VREFMIN_LP4,  WT_STEP1_VREFMAX_LP4}, { WT_STEP2_VREFSTEP_LP4, 0, 0}                                                , MMRC_ABS, STROBE_SHARED | KNOB_SHARED | BIT_SHARED , -1          , "VRF\0", 2-1, 5, 50, WT_STEP1_VREFSTEP_DDR3, WT_STEP1_VREFSTEP_LP4,READTRAINING_ENTRY, TRUE, 1, 3, GenericMemoryTest},
};
CONTROLKNOBS ControlKnobsWriteLP3[] = {
  // Control                   Start                                       Range              Step                                                                      Rel/Abs      Shared
  //  Knob                     Value             Value                     Value    Flags                                                            Flags
  {TxDqDelay,   -1, TxDqBitDelay, {_n12X,_12X}                                     , 31, {_n12X,  _12X}                       , { WT_STEP1_3_PISTEPLARGE, WT_STEP1_3_PISTEPMEDIUM, WT_STEP1_3_PISTEPSMALL}, REL     , WT_BYTE_SHARING                          , WT_BIT_SHARING, "PI \0", 2-1, 0,  0,                 0, -1,READTRAINING_ENTRY, TRUE, 1, 5, GenericMemoryTest},
  {TxVref,      -1,           -1, {WT_STEP1_VREFSTART_LP3 ,  WT_STEP1_VREFSTART_LP3}, -1, {WT_STEP1_VREFMIN_LP3,  WT_STEP1_VREFMAX_LP3}, { WT_STEP2_VREFSTEP_LP3, 0, 0}                                                , MMRC_ABS, CHANNEL_SHARED | RANK_SHARED | STROBE_SHARED | KNOB_SHARED | BIT_SHARED , -1          , "VRF\0", 2-1, WT_STEP1_VREFMIN_LP3, WT_STEP1_VREFMAX_LP3, WT_STEP1_VREFSTEP_LP3, WT_STEP1_VREFSTEP_LP3,READTRAINING_ENTRY, TRUE, 1, 3, GenericMemoryTest},
  //{TxVref,      -1,           -1, {WT_STEP1_VREFSTART_LP3 ,  WT_STEP1_VREFSTART_LP3}, -1, {WT_STEP1_VREFMIN_LP3,  WT_STEP1_VREFMAX_LP3}, { WT_STEP2_VREFSTEP_LP3, 0, 0}                                                , MMRC_ABS, CHANNEL_SHARED | RANK_SHARED | STROBE_SHARED | KNOB_SHARED | BIT_SHARED , -1          , "VRF\0", 2-1, 10, WT_STEP1_VREFMAX_LP3, WT_STEP1_VREFSTEP_LP3, WT_STEP1_VREFSTEP_LP3,READTRAINING_ENTRY, TRUE, 1, 3, GenericMemoryTest},
};
CONTROLKNOBS ControlKnobsWriteDDR3[] = {
  // Control                   Start                                       Range              Step                                                                      Rel/Abs      Shared
  //  Knob                     Value             Value                     Value    Flags                                                            Flags
  {TxDqDelay,   -1, TxDqBitDelay, {0,0}                                     , 31, {_n1X,  _1X}                       , { WT_STEP1_3_PISTEPLARGE, WT_STEP1_3_PISTEPMEDIUM, WT_STEP1_3_PISTEPSMALL}, REL     , WT_BYTE_SHARING                          , WT_BIT_SHARING, "PI \0", 2-1, 0,  0,                 0, -1,READTRAINING_ENTRY, TRUE, 1, 1, GenericMemoryTest},
  {TxVref,      -1,           -1, {WT_STEP1_VREFSTART_DDR3 ,  WT_STEP1_VREFSTART_DDR3}, -1, {WT_STEP1_VREFMIN_DDR3,  WT_STEP1_VREFMAX_DDR3}, { WT_STEP2_VREFSTEP_DDR3, 0, 0}                                                , MMRC_ABS, STROBE_SHARED | KNOB_SHARED | BIT_SHARED , -1          , "VRF\0", 2-1, 0, 1, WT_STEP1_VREFSTEP_LP4, WT_STEP1_VREFSTEP_DDR3,READTRAINING_ENTRY, TRUE, 1, 1, GenericMemoryTest},
};

UINT8 NumberControlKnobsCmdCtl = 3;

CONTROLKNOBS ControlKnobsCmdCtl[] = {
  // Control                   Start       Range      Step     Rel/Abs      Shared
  //  Knob                     Value       Value      Value    Flags         Flags
  {CmdGrp0,               -1, -1, {0 , 0}, -1, {  _n12X,  _12X}, { 10, 0, 0}, REL, RANK_SHARED | BIT_SHARED | STROBE_SHARED | KNOB_SHARED, 0xff,"Cmd",2, 0, 0, 0,  0xFF, READTRAINING_ENTRY, TRUE,2, 5, GenericCmdMemoryTest},
  {CtlGrp0,               -1, -1, {0 , 0}, -1, {  _n12X,  _12X}, { 10, 0, 0}, REL, RANK_SHARED | BIT_SHARED | STROBE_SHARED | KNOB_SHARED, 0xff,"Ctl",2, 0, 0, 0, 0xFF, READTRAINING_ENTRY, TRUE,2, 5, GenericCmdMemoryTest},
  {TxCaVref,              -1, -1, {400 , 400}, -1, {100,  400}, { 401, 0, 0}, MMRC_ABS, RANK_SHARED |STROBE_SHARED | KNOB_SHARED | BIT_SHARED,-1,"Vre",2, 5, 50, 5, 5, READTRAINING_ENTRY, TRUE,2, 3, GenericCmdMemoryTest},
};
CONTROLKNOBS ControlKnobsCmdCtl_LPDDR3[] = {
  // Control                   Start       Range      Step     Rel/Abs      Shared
  //  Knob                     Value       Value      Value    Flags         Flags
  {CmdGrp0,               -1, -1, {0 , 0}, -1, {  _n12X,  _12X}, {10, 0, 0}, REL, RANK_SHARED | BIT_SHARED | STROBE_SHARED | KNOB_SHARED, 0xff,"Cmd",2, 0, 0, 0,  0xFF, READTRAINING_ENTRY, TRUE,2, 3,GenericCmdMemoryTest},
  {CtlGrp0,               -1, -1, {0 , 0}, -1, {  _n14X,  _14X}, {10, 0, 0}, REL, RANK_SHARED | BIT_SHARED | STROBE_SHARED | KNOB_SHARED, 0xff,"Ctl",2, 0, 0, 0, 0xFF, READTRAINING_ENTRY, TRUE,2, 3,GenericCmdMemoryTest},
  {TxCaVref,              -1, -1, {30 , 30}, -1, {20, 60 }, { 10, 0, 0}, MMRC_ABS, CHANNEL_SHARED |RANK_SHARED |STROBE_SHARED | KNOB_SHARED | BIT_SHARED,-1,"Vre",2, 20, 60, 5, 5, READTRAINING_ENTRY, TRUE,2, 3,GenericCmdMemoryTest},
};

CONTROLKNOBS ControlKnobsCmdCtl_DDR3[] = {
  // Control                   Start       Range      Step     Rel/Abs      Shared
  //  Knob                     Value       Value      Value    Flags         Flags
  {CmdGrp0,               -1, -1, {0 , 0}, -1, {  _n12X,  _12X}, {5, 0, 0}, REL, RANK_SHARED | BIT_SHARED | STROBE_SHARED | KNOB_SHARED, 0xff,"Cmd",2, 0, 0, 0,  0xFF, READTRAINING_ENTRY, TRUE,2, 1,GenericCmdMemoryTest},
  {CtlGrp0,               -1, -1, {0 , 0}, -1, {  _n12X,  _12X}, {5, 0, 0}, REL, RANK_SHARED | BIT_SHARED | STROBE_SHARED | KNOB_SHARED, 0xff,"Ctl",2, 0, 0, 0, 0xFF, READTRAINING_ENTRY, TRUE,2, 1,GenericCmdMemoryTest},
  {TxCaVref,              -1, -1, {0 , 0}, -1, {0, 2 }, { 5, 0, 0}, MMRC_ABS, RANK_SHARED |STROBE_SHARED | KNOB_SHARED | BIT_SHARED,-1,"Vre",2, 0, 0, 0, 5, READTRAINING_ENTRY, TRUE,2, 1,GenericCmdMemoryTest},
};


UINT8 EyeMaskPctPiRd[NUM_TYPE][NUM_FREQ][RT_NUM_EYEMASKS] =
{
  { //LPDDR3
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //LPDDR4
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //DDR3L
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,05}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //WIO2
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
};

UINT8 EyeMaskPctPiWr[NUM_TYPE][NUM_FREQ][WT_NUM_EYEMASKS] =
{
  { //LPDDR3
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //LPDDR4
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {30,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //DDR3L
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //WIO2
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
};

UINT8 EyeMaskPctPiCmdCtl[NUM_TYPE][NUM_FREQ][CMD_NUM_EYEMASKS] =
{
  { //LPDDR3
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //LPDDR4
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //DDR3L
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
  { //WIO2
    {40,20}, //800
    {40,20}, //1066
    {40,20}, //1333
    {40,20}, //1600
    {30,15}, //1866
    {20,10}, //2133
    {20,10}, //2400
    {20,10}, //2667
    {20,10}, //3200
    {20,10}  //2933
  },
};

UINT32 DramOdt[NUM_TYPE][NUM_FREQ] =
{
  { //LPDDR3
    99999, //800
    99999, //1066
    99999, //1333
    99999, //1600
    60,    //1866
    60,    //2133
    60,    //2400
    60,    //2667
    60,    //3200
    60     //2933
  },
  { //LPDDR4
    99999, //800
    99999, //1066
    99999, //1333
    99999, //1600
    60,    //1866
    60,    //2133
    60,    //2400
    60,    //2667
    60,    //3200
    60     //2933
  },
  { //DDR3L
    99999, //800
    99999, //1066
    99999, //1333
    99999, //1600
    60,    //1866
    60,    //2133
    60,    //2400
    60,    //2667
    60,    //3200
    60     //2933
  },
  { //WIO2
    99999, //800
    99999, //1066
    99999, //1333
    99999, //1600
    60,    //1866
    60,    //2133
    60,    //2400
    60,    //2667
    60,    //3200
    60     //2933
  },
};

UINT16 DramVrefOvr[NUM_TYPE][NUM_FREQ] =
{
  { //LPDDR3
    37, //800
    37, //1066
    37, //1333
    37, //1600
    27, //1866
    27, //2133
    27, //2400
    27, //2667
    27, //3200
    27  //2933
  },
  { //LPDDR4
    37, //800
    37, //1066
    37, //1333
    37, //1600
    27, //1866
    27, //2133
    27, //2400
    27, //2667
    27, //3200
    27  //2933
  },
  { //DDR3L
    37, //800
    37, //1066
    37, //1333
    37, //1600
    27, //1866
    27, //2133
    27, //2400
    27, //2667
    27, //3200
    27  //2933
  },
  { //WIO2
    37, //800
    37, //1066
    37, //1333
    37, //1600
    27, //1866
    27, //2133
    27, //2400
    27, //2667
    27, //3200
    27  //2933
  },
};

//UINT8 SimData[MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
typedef struct {
  UINT8 Left;
  UINT8 Right;
} RANGE_T;


CONST UINT8 AddressDecoding[3][2][3] = {
  //LP3
    //1KB,         2KB,           4KB
  {{AddMapping4KB, AddMapping4KB, AddMapping4KB},  //1 channel
   {AddMapping2KB, AddMapping2KB, AddMapping4KB},  //2 channels
  },
  //LP4
  {{AddMapping4KB, AddMapping4KB, AddMapping4KB},  //2 channel
   {AddMapping2KB, AddMapping2KB, AddMapping4KB},  //4 channels
  },
  //WIO
    {{AddMapping4KB, AddMapping4KB, AddMapping4KB},  //1 channel
   {AddMapping2KB, AddMapping2KB, AddMapping4KB},  //2 channels
  },
};

#if PASS_GATE_TEST == 1
CONST UINT32 MaxDelayFor4x[NUM_FREQ] = {
  319000,  // 800  - Never Tested
  426000,  // 1066
  532000,  // 1333
  639000,  // 1600
};

CONST DELAY_PASS_GATE DelayPassGate [2][NUM_FREQ] = {
  { // 1x Speed
  { 75,  0, 0, 0}, // 800  - 100ns - Never Tested
  {100, 53, 0, 0}, // 1066 - 75ns
  { 50, 58, 0, 0}, // 1333 - 60ns
  { 50, 64, 0, 0}, // 1600 - 50ns
  },
  { // 4x Speed
  {150,   0, 0, 0}, // 800  - Never Tested
  {200,  26, 0, 0}, // 1066 - 150ns
  {100,  29, 0, 0}, // 1333 - 120ns
  {100,  32, 0, 0}, // 1600 - 100ns
  },
};
#endif

#if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  //
  // These are the variables required for early command training.
  //
  UINT16 EarlyCommandPatternsRise[]   = {0x2AA,      0x0cc,      0x30f,  0x38b};
  UINT16 EarlyCommandPatternsFall[]   = {0x155,      0x333,      0x0f0,  0x074};
//  UINT32 EarlyCommandResults41[]      = {0x6699,     0x5a96,     0x6A55, 0x5A65};
//  UINT32 EarlyCommandResults48[]      = {0x0201,     0x0202,     0x0102, 0x0102};
  UINT32 EarlyCommandResults41[MAX_CHANNELS][4]     = { {0x4e93c65a,     0x9cb4c9e8,     0x6A55, 0x5A65},
                                                         {0x1e2b0fe4,     0xd23cc355,     0x6A55, 0x5A65},
                                                         {0xa656b1e2,     0x35956336,     0x6A55, 0x5A65},
                                                         {0xcc35c6e4,     0x99939356,     0x6A55, 0x5A65}};
  UINT32 EarlyCommandResults48[MAX_CHANNELS][4]      = { {0x02018008,     0x80208008,     0x6A55, 0x5A65},
                                                         {0x04200180,     0x80200110,     0x6A55, 0x5A65},
                                                         {0x04022040,     0x04802010,     0x6A55, 0x5A65},
                                                         {0x04200240,     0x10020240,     0x6A55, 0x5A65}};

  UINT16 EarlyCommandPatterns1RiseLP4[]   = {0x31,      0x0D,      0x06, 0x30};
  UINT16 EarlyCommandPatterns1FallLP4[]   = {0x0C,      0x30,      0x29, 0x0F};
  UINT16 EarlyCommandPatterns2RiseLP4[]   = {0x33,      0x0F,      0x16, 0x00};
  UINT16 EarlyCommandPatterns2FallLP4[]   = {0x0C,      0x30,      0x29, 0x00};
  UINT32 EarlyCommandResultsSIM[]      = {0x00310031, 0x000d000d, 0x06000600, 0x30003000};
  UINT32 EarlyCommandResultsSIL[MAX_CHANNELS][4]      =
     {
        {0x0074004d, 0x00690071, 0x16001600, 0x00000000}, //{0x005c005c, 0x000f000f, 0x16001600, 0x00000000}, //{0x33003300, 0x03003000, 0x16001600, 0x00000000};
        {0x1e006900, 0x78006600, 0x16001600, 0x00000000}, //{0x33003300, 0x03003000, 0x16001600, 0x00000000};
        {0x00c30036, 0x0071008e, 0x16001600, 0x00000000}, //{0x33003300, 0x03003000, 0x16001600, 0x00000000};
        {0x4b00ac00, 0x6c000f00, 0x16001600, 0x00000000} //{0x33003300, 0x03003000, 0x16001600, 0x00000000};
  };

  UINT8 EarlyCommandWeights[3][3] = {
    { 1, 1, 1 },
    { 1, 2, 1 },
    { 1, 1, 1 }
  };

#endif
/**
  TimingParamHash - BXTP/APLK
  This hash function converts old index values to new valid ones.
  We are removing indices for the following arrays:
     ConfigToSpeed
     ConfigToSpeedMultiple133
     ConfigToDdrType
     TimingParamsUnder256
     TimingParamsOver256
  The TimingParams arrays are 1 based, whereas the Config is 0 based.
  For Config, we are removing indices
     0, 1, 2, 6, 7, 11, 12, 13, 14, 15, 16, and 29+
  For Timing, we are removing indices
     1, 2, 3, 7, 8, 12, 13, 14, 15, 16, 17, and 30+
  These indices are being removed because DDR4 and select frequencies are not POR.
  This also saves on code size.
  The result of the Hash (OutputProfile) is found in MrcData->MrcMemConfig

  @param[in, out] MrcData           Host structure for all data related to MMRC.
  @param[in]      InputProfile      A pointer to the new index value.
  @param[in, out] OutputProfile     The output of the hash function: the correct index of the timing profile.

  @retval         Status

**/
extern
MMRC_STATUS
TimingParamHash (
  UINT8     InputProfile,
  UINT8     *OutputProfile
  )
{
  UINT8 Offset;
  UINT8 NewProfile;
  Offset = 0;

  //
  // Take valid index and determine offset.
  // Obtain offset by comparing MaxMemoryTypeFreqConfigSmip (original index)
  // to MaxMemoryTypeFreqConfig (correct index).
  //
  if ((LPDDR3_1333_10_12_12 <= InputProfile) && (InputProfile < LPDDR3_2133_16_20_20)) {
    Offset += (LPDDR3_1333_10_12_12 - LPDDR3_1333_10);
  } else if ((LPDDR4_1600_14_15_15 <= InputProfile) && (InputProfile < LPDDR4_2933_28_27_27)) {
    Offset += (LPDDR4_1600_14_15_15 - LPDDR4_1600_14);
  } else if ((DDR3_1333_7_7_7 <= InputProfile) && (InputProfile < DDR3_2133_11_11_11)) {
    Offset += (DDR3_1333_7_7_7 - DDR3_1333_7);
  } else {
    MmrcErrorPrint ((MMRC_DBG_MIN, "TimingParams Error: %d\n", InputProfile));
    return MMRC_FAILURE;
  }

  NewProfile = (UINT8) (InputProfile - Offset);

  if (NewProfile >= MaxMemoryTypeFreqConfig) {
    MmrcErrorPrint ((MMRC_DBG_MIN, "TimingParams Error: %d\n", InputProfile));
    return MMRC_FAILURE;
  }

  *OutputProfile = NewProfile;
  return MMRC_SUCCESS;
}

CLOCK_CROSSINGS ClockCrossings_BX_LPDDR3_DDR3L[MAX_NUM_CC_RANGES]  = {
  // CcNum Min%  Max%  InVal Absolute
  {    0,    0,   25,    3,    TRUE}, // CC0
  {    0,   25,   50,    0,    TRUE}, // CC0
  {    0,   50,   83,    0,    TRUE}, // CC0
  {    0,   83,  100,    3,    TRUE}, // CC0
  {    1,    0,   25,    1,    TRUE}, // CC1
  {    1,   25,   50,    1,    TRUE}, // CC1
  {    1,   50,   83,    0,    TRUE}, // CC1
  {    1,   83,  100,    1,    TRUE}, // CC1
  {    2,    0,   25,   -1,   FALSE}, // CC2
  {    2,   25,   50,    0,   FALSE}, // CC2
  {    2,   50,   83,    0,   FALSE}, // CC2
  {    2,   83,  100,    0,   FALSE}, // CC2
};

CLOCK_CROSSINGS ClockCrossings_AX_ALL_BX_LPDDR4[MAX_NUM_CC_RANGES]  = {
  // CcNum Min%  Max%  InVal Absolute
  {    0,    0,   25,    3,    TRUE}, // CC0
  {    0,   25,   50,    1,    TRUE}, // CC0
  {    0,   50,   75,    0,    TRUE}, // CC0
  {    0,   75,  100,    2,    TRUE}, // CC0
  {    1,    0,   25,    1,    TRUE}, // CC1
  {    1,   25,   50,    1,    TRUE}, // CC1
  {    1,   50,   75,    0,    TRUE}, // CC1
  {    1,   75,  100,    1,    TRUE}, // CC1
  {    2,    0,   25,   -1,   FALSE}, // CC2
  {    2,   25,   50,   -1,   FALSE}, // CC2
  {    2,   50,   75,    0,   FALSE}, // CC2
  {    2,   75,  100,    0,   FALSE}, // CC2
};
