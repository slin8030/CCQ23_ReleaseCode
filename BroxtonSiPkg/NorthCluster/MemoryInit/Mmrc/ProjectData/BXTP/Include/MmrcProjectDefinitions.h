/** @file
  Includes all hard defines specific to a project which are NOT
  outputted by the MMRC tool. Things like the maximum number of
  channels, ranks, DIMMs, etc, should be included here. This file
  needs to be included by most MMRC components, including Mmrc.h.

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

#ifndef _MMRCPROJECTDEFINITIONS_H
#define _MMRCPROJECTDEFINITIONS_H

#include "MmrcProjectDefinitionsGenerated.h"
#include "MrcEfiDefinitions.h"

#define MAX_TIMING    (1 + RT_PN_ENABLED)
#define MAX_CONSEC    5
#define REL 0
#define MMRC_ABS 1
#define COMPOSITE_NONE    0
#define COMPOSITE_COUNT   1
#define COMPOSITE_RESULTS 2
#define OVERWRITE         4
//
// Applying values.
//
#define PB_APPLY_OFFSET   1
//
// Index based on control knobs.
//
#define CI_TIMING 0
#define CI_VREF   1

#define NUM_VREFS_PHASE2 3  // THIS MUST BE AN ODD NUMBER NO MORE THEN THE WORST EYE_MASK.
#define RT_MIN_VREF   0
#define RT_MAX_VREF   63
#define RT_STEP_VREF  2
#define CT_MIN_VREF   0
#define CT_MAX_VREF   63
#define CT_STEP_VREF  2
#define WT_MIN_VREF   0
#define WT_MAX_VREF   63
#define WT_STEP_VREF  2
#define NUM_SAMPLES_TO_CENTERT 5
#define NUM_SAMPLES_TO_CENTERV 3

#define NUM_BITS_PER_DEVICE    32
#define LP4_ECT_PATTERN_MASK_by32   ((LP4_ECT_PATTERN_MASK_by16 << 16) | LP4_ECT_PATTERN_MASK_by16)
#define LP4_ECT_PATTERN_MASK_by16   0x3F00

//
// [7:4]  = Major revision
// [3:0]  = Minor revision
//
#define MRC_VERSION           VERSION_MAJOR << 8 | VERSION_MINOR

#define MAX_ELEMENT_TABLE     40

#define MINDDR                MMRC_800
#define MAXDDR                MMRC_3200
#define BOOT_FREQUENCY        MMRC_1600

#define DDR3L_MINDDR          MMRC_1333
#define DDR3L_MAXDDR          MMRC_1866
#define DDR3L_MAXCHANNEL      2

#define MINCOREFREQ           COREFREQ_800
#define MAXCOREFREQ           COREFREQ_1333

//
// RMT project specific definitions.
//
#define PSEUDO_RANKS                     0 // PT changed from 2 to 0. Effectively disable it
#define MAX_RMT_ELEMENTS                 7 // PT changed from 10 to 7
#define MAX_PRODUCTION_RMT_ELEMENTS      6
#define MAX_PPV_RMT_ELEMENTS             6
#define GET_DIMM(rank)                   ((rank >> 1) & 0x3)
#define WRITE_SUB_TYPE                   9
#define READ_SUB_TYPE                    8

typedef enum {
  RmtRxDqsHi = 0,
  RmtRxDqsLo,
  RmtRxVref,
  RmtTxDq,
  RmtTxVref,
  RmtCmd,
  RmtRecEn,
  RmtCtl,
  RmtClk,
  MaxRmtElements,
  RmtFullEvMode,
  RmtEvCommandMode,
} RMT_ELEMENTS;

//
// Keep these in same order as rank 0 to MAX_RANK , immediately followed by Pseudoranks in order
//
typedef enum {
  Rmt_Rank0_Disable = 16,
  Rmt_Rank1_Disable,
  Rmt_Rank_TA_Disable,
  Rmt_Rank_LFSR_Disable
} RMT_RANK_DISABLE_ELEMENTS;

#define RMT_RX_DQS_HI_EXEC       (1 << RmtRxDqsHi)
#define RMT_RX_DQS_LO_EXEC       (1 << RmtRxDqsLo)
#define RMT_TX_DQ_EXEC           (1 << RmtTxDq)
#define RMT_RX_VREF_EXEC         (1 << RmtRxVref)
#define RMT_TX_VREF_EXEC         (1 << RmtTxVref)
#define RMT_REC_EN_EXEC          (1 << RmtRecEn)
#define RMT_CTL_EXEC             (1 << RmtCtl)
#define RMT_CLK_EXEC             (1 << RmtTxVref)
#define RMT_CMD_EXEC             (1 << RmtCmd)
#define RMT_ALL_TESTS_EXEC       ((1 << MaxRmtElements) - 1)
#define RMT_FULL_EV_MODE         (1 << RmtFullEvMode)
#define RMT_EV_COMMAND_MODE      (1 << RmtEvCommandMode)
#define RMT_RANK_0_DISABLE       (1 << Rmt_Rank0_Disable)
#define RMT_RANK_1_DISABLE       (1 << Rmt_Rank1_Disable)
#define RMT_RANK_TA_DISABLE      (1 << Rmt_Rank_TA_Disable)
#define RMT_RANK_LFSR_DISABLE    (1 << Rmt_Rank_LFSR_Disable)

#ifndef PCIEX_BASE_ADDRESS
//
// PCIe Base Address
//
#ifndef SIM
#define PCIEX_BASE_ADDRESS  0xE0000000
#else
#define PCIEX_BASE_ADDRESS  0xC0000000
#endif
//
// PMC Memory Space
//
#define PBASE               0xFED03000
#endif

#define VALUE_CSLE_PMIXOR   0x13  // reduced MRC for CSLE with pmixor
#define VALUE_SLE_UPF       0x11  // 0x11 HSLE and CSLE Unified Power Format
#define VALUE_PURE_SIMICS   0x4   // 0x4=Pure Simics
#define VALUE_PUREVP        0x2   // 0x2=PureVP
#define VALUE_INNOVATOR_HVP 0x3   // 0x3=HVP
#define VALUE_SLE           0x1   // 0x1=HSLE and CSLE
#define VALUE_REAL_PLATFORM 0x0   // 0x0=Not a PreSi Platform
#define SCCB_BASE           0xFF03A000
#define PID_OFFSET          0x2C
#define PLATFORM_ID_LOC     SCCB_BASE + PID_OFFSET
#define PLATFORM_ID_MASK    0x1F
#define PLATFORMID_SHIFT    21                           // Platform ID doesn't start at bit0

//
// Extended Configuration Base Address.  Port to value enabled by calling code, if necessary.
// If the EC space is not enabled by code calling the MRC, then enable the
// EC space to this value
//
#ifndef EC_BASE
#define EC_BASE   PCIEX_BASE_ADDRESS
#endif

#ifndef MCHBAR_BASE
#define MCHBAR_BASE 0xFED10000
#endif

#ifndef P2SB_BASE
#ifdef SIM_P2SB_BASE
#define P2SB_BASE   0xB0000000
#else
#define P2SB_BASE   0xD0000000
#endif
#endif

#ifndef MRC_DATA_REQUIRED_FROM_OUTSIDE
//
// SysCtl SMBus address and block size
//
#define VF_SC_SMBUS_ADDRESS         0x60
#define VF_SC_BYTE_LEN              1
#define VF_SC_BLOCK_LEN             4
#define VF_SC_SMBUS_WRCMD           1
#define VF_SC_SMBUS_RDCMD           0
//
// SMBus I/O Registers
//
#define R_PCH_SMBUS_HSTS                0x00  // Host Status Register R/W
#define B_PCH_SMBUS_HBSY                0x01
#define R_PCH_SMBUS_HCTL                0x02  // Host Control Register R/W
#define B_PCH_SMBUS_START               BIT6  // Start
#define B_PCH_SMBUS_DERR                0x04
#define B_PCH_SMBUS_BERR                0x08
#define B_PCH_SMBUS_IUS                 0x40
#define B_PCH_SMBUS_BYTE_DONE_STS       0x80
#define B_PCH_SMBUS_HSTS_ALL            0xFF
#define V_PCH_SMBUS_SMB_CMD_BYTE_DATA   0x08  // Byte Data
#define V_PCH_SMBUS_SMB_CMD_BLOCK       0x14  // Block


#define R_PCH_SMBUS_HCMD                0x03  // Host Command Register R/W

#define R_PCH_SMBUS_TSA                 0x04  // Transmit Slave Address Register R/W
#define B_PCH_SMBUS_RW_SEL_READ         0x01  // Read

#define R_PCH_SMBUS_HD0                 0x05  // Data 0 Register R/W
#define R_PCH_SMBUS_HD1                 0x06  // Data 1 Register R/W
#define R_PCH_SMBUS_HBD                 0x07  // Host Block Data Register R/W

#define R_PCH_SMBUS_AUXS                0x0C  // Auxiliary Status Register R/WC
#define B_PCH_SMBUS_CRCE                BIT0  // CRC Error

#define R_PCH_SMBUS_AUXC                0x0D  // Auxiliary Control Register R/W
#define B_PCH_SMBUS_E32B                BIT1  // Enable 32-byte Buffer
#define B_PCH_SMBUS_AAC                 BIT0  // Automatically Append CRC

#define BUS_TRIES                       3       // How many times to retry on Bus Errors

#endif // MRC_DATA_REQUIRED_FROM_OUTSIDE

#define BROADCAST_SUP               0x00        // Support for Broadcasting during phyinit's when looping on modules.

//
// EarlyMprTraining Parameters
//
#define EARLYTRAIN_MIDPOINTVREF    0x20
#define EARLYTRAIN_MARGINSTEP      0x01
#define EARLYTRAIN_LOWBOUNDARY     0x00
#define EARLYTRAIN_UPBOUNDARY      0x48
//
// RdVefTraining Parameters
//
#define RDVREFTRAIN_MIDPOINTVREF    0x14
#define RDVREFTRAIN_MARGINSTEP      0x02
#define RDVREFTRAIN_LOWBOUNDARY     0x00
#define RDVREFTRAIN_UPBOUNDARY      (2*RDVREFTRAIN_MIDPOINTVREF - 1)
//
// WrVefTraining Parameters
//
#define WRVREFTRAIN_MIDPOINTVREF    0x20
#define WRVREFTRAIN_MARGINSTEP      0x02
#define WRVREFTRAIN_LOWBOUNDARY     0x00
#define WRVREFTRAIN_UPBOUNDARY      0x3f
//
// CMD Training Parameters
//
#define CMDTRAIN_MIDPOINTVREF       0x1ff
#define CMDTRAIN_MARGINSTEP         0x02
#define CMDTRAIN_LOWBOUNDARY        0x00
#define CMDTRAIN_UPBOUNDARY         0x3ff
//
// RdTraining Parameters
//
#define RDTRAIN_MIDPOINTVREF        0x20
#define RDTRAIN_MARGINSTEP          0x02
#define RDTRAIN_LOWBOUNDARY         0x00
#define RDTRAIN_UPBOUNDARY          0x3f
//
// WrTraining Parameters
//
#define WRTRAIN_MIDPOINTVREF        0x20
#define WRTRAIN_MARGINSTEP          0x02
#define WRTRAIN_LOWBOUNDARY         0x00
#define WRTRAIN_UPBOUNDARY          0x3f

#define PHYENTERWRLVL               0x1
#define PHYEXITWRLVL                0x0
//
// Pattern Creations
//
#define NUMBERPATTERNS              0x10  // Number of Victim/Aggressor Patterns * (repeat +1)= 10 * (16 + 1) = 10*17 = 170  Defined by tool
#define VICTIMPATTERNS              { 0xAAAAAAAA,  0xDB6DB6DB,  0x92492492,  0xEEEEEEEE,  0xCCCCCCCC,  0x88888888,  0xF7BDEF7B,  0xE739CE73,  0xC6318C63,  0x84210842}
#define AGGRESSORPATTERNS           {~0xAAAAAAAA, ~0xDB6DB6DB, ~0x92492492, ~0xEEEEEEEE, ~0xCCCCCCCC, ~0x88888888, ~0xF7BDEF7B, ~0xE739CE73, ~0xC6318C63, ~0x84210842}
#define PATTERN_SIZE                256   //  4 CL for Cedarview = 256 bytes , 2 CL for Berryville
#define FLYBY_SHIFT                 {0, 0, 0, 0, 0, 0, 0, 0}

#define FIFO_RESET_DISABLE          0x01
#define FIFO_RESET_ENABLE           0x00

#define MRW_LP4(MA, OP)   (6<<0) | (((OP & 0x80)>>7 )<< 5) |       \
                          (MA & 0x3f) << 6 |                       \
                          (0x16 << 12) | (((OP & 0x40)>>6)<< 17) | \
                          (OP & 0x3F) << 18
#define MPC(CC, OP)       (0<<0) | (((OP & 0x40)>>6) << 5) |       \
                          (OP & 0x3f) << 6 |                       \
                          (0x12 << 12) | (((CC & 0x40)>>6)<< 17) | \
                          (CC & 0x3F) << 18

#define MMR_WIO2(MA)      ((MA & 0x0F) << 4) | 8
#define MRR_LP3(MA)       ((MA & 0xFF) << 4) | 8
#define MRR_LP4(MA)       (0xE<<0) | ( (MA & 0x3f) << 6) | (0x12 << 12)

#define READ_DQ_CALIBRATION 0x43
#define START_DQS_OSC 0x4B
#define START_ZQCAL 0x4F

#define PREA_LP3    0x1b
#define PREA_LP4    0x30
#define MRW32_LP3   0xa2208;
#define MRW41_LP3   0xa4290
#define MRW48_LP3   0xc0300
#define MRW42_LP3   0xa82a0
#define MPC06_LP4   0x120e0;  // Ignoring Bank/Rank bits.
#define MRW40_LP3   0x288 | (0x208<<10);

#define BUSWIDTH                    0x8   // 1=x8, 2=x16, 4=x32, 8=x64
#define CHANNEL_BYTEOFFSET          0x1000
#define CHANNEL_BITOFFSET           0

#define MAX_DIMM_CONFIGS           144

#define BL8  3
#define BL32 0xFF

//
// Various portions of the MMRC that can be run by calling MmrcEntry().
//
typedef enum {
  EntireMmrc,
  DdrioPhyInit,
  MemoryTraining,
  PostTraining,
  HandlePeriodicComp,
  ExecuteSpecificFunction,
  CapsuleInit,
} MMRC_STAGE;
//
// Message Bus Commands
//
#define CMD_READ_REG          0x10    // Read Message Bus Register Command
#define CMD_WRITE_REG         0x11    // Write Message Bus Register Command
#define CMD_DRAM_INIT         0x68    // JEDEC Command
#define CMD_WAKE              0xCA    // Wake Command
#define CMD_SUSPEND           0xCC    // Suspend Command

//
// Allowed vref gap between channels
//
#define VREF_THRESHOLD_GAP    5
//
// Layout definitions  of the elements array.
//
//#define REG_INDEX               0x100
//#define GROUP_INDEX             0x200
//#define UNIQUE_INDEX            0x300
//#define ALGO_REG_INDEX          0xc0                                // Starting index in the TYPE field for Signal Group Modules.
//#define SGTI_INDEX              0xe0                                // Starting index for the Signal Group Type Index.
//#define INDIV_ELEMENT_INDEX     (NUM_ALGOS * NUM_ELEMENTS_PER_ALGO) // Starting index of non-linear delay elements.

//
// Index of individual elements supported by the Get/set
//
#define REG_UNDEFINED     0xFF

#define CMD_SIGNALTYPE          1
#define CMD_SIGNALGROUP         0

#define MIN_RANK_BIT                RANK_SHIFT_BIT
#define MAX_DUNIT_ROW_ADDRESS_BIT   (MIN_RANK_BIT - 1)
#define OFFSET_HIGH_LOW_SHIFT       (MAX_DUNIT_ROW_ADDRESS_BIT - 6 - 15)

#define PERCENT_MAX_MARGIN(x)     ((4*x)/10)

#define EW_PERCENTAGE(x)          ((6*x)/10)

#define EW_PERCENTAGE_VREF(x)     ((9*x)/10)
#define WEIGHT_CONDITION_VREF(x)  ((4*x)/10)

#define EW_PERCENTAGE_DELAY(x)    ((9*x)/10)
#define WEIGHT_CONDITION_DELAY(x) ((5*x)/10)

#define PASS_MARGIN_AXIS_X(x)     ((7*x)/10)
#define PASS_MARGIN_AXIS_Y(y)     ((1*y)/10)

typedef enum {
  ReceiveEnableStep,
  FineWriteLevelingStep,
  CoarseWriteLevelingStep,
  ReadVrefStep,
  ReadDelayStep,
  WriteDelayStep,
  WriteVrefStep,
  EarlyMprReadStep,
  CmdDelayStep
} MARGIN_TYPE;

typedef enum {
  BxtTimingtCL = 0,
  BxtTimingtCWL,
  BxtTimingtRCD,
  BxtTimingtRP,
  BxtTimingtRPab,
  BxtTimingtRAS,
  BxtTimingtRC,
  BxtTimingtRCab,
  BxtTimingtRRD,
  BxtTimingtRRD_L,
  BxtTimingtFAW,
  BxtTimingtCCD,
  BxtTimingtCCD_L,
  BxtTimingtRTP,
  BxtTimingtWTR,
  BxtTimingtWTR_L,
  BxtTimingtWR,
  BxtTimingtRDPRE,
  BxtTimingtWRPRE,

  BxtTimingtREFI,
  BxtTimingtXS_offset,
  BxtTimingtXS,
  BxtTimingtXSDLL,
  BxtTimingtCKESR,

  BxtTimingtXP,
  BxtTimingtXPDLL,
  BxtTimingtCKE,
  BxtTimingtPRPDEN,
  BxtTimingtRDPDEN,
  BxtTimingtWRPDEN,
  BxtTimingtCPDED,

  BxtTimingtMOD,
  BxtTimingtAONPD,

  BxtTimingtRDRD,
  BxtTimingtRDRD_L,
  BxtTimingtRDRD_dr,
  BxtTimingtRDRD_dd,
  BxtTimingtWRWR,
  BxtTimingtWRWR_L,
  BxtTimingtWRWR_dr,
  BxtTimingtWRWR_dd,
  BxtTimingtWRRD,
  BxtTimingtWRRD_L,
  BxtTimingtWRRD_dr,
  BxtTimingtWRRD_dd,
  BxtTimingtRDWR,
  BxtTimingtRDWR_dr,
  BxtTimingtRDWR_dd,

  BxtTimingtZQOPER,
  BxtTimingtZQCS,
  BxtTimingtZQCL,
  BxtTimingtZQLAT,

  BxtTimingtCCDMW,
  BxtTimingtCKSRE,
  BxtTimingtCKSRX,
  BxtTimingtRPPB,
  BxtTimingtRFC_4G,
  BxtTimingtRFC_6G,
  BxtTimingtRFC_8G,
  BxtTimingtRFC_12G,
  BxtTimingtRFC_16G,

  BxtTimingtXSR_4G,
  BxtTimingtXSR_6G,
  BxtTimingtXSR_8G,
  BxtTimingtXSR_12G,
  BxtTimingtXSR_16G,

  BxtTimingtXS_1G,
  BxtTimingtXS_2G,
  BxtTimingtXS_4G,
  BxtTimingtXS_8G,

  BxtTimingtFAW1K,
  BxtTimingtFAW2K,


  BxtMaxTimingData,

  //Timing parameter not in TimingParams[], but will be used
  BxtTimingMntdly,
  BxtTimingTTpstmrblk,
  BxtTimingTpremrblk,
} BXTP_TIMING_DATA_TYPE;

///
/// The MaxMemoryTypeFreqConfigSmip enum definition is specific to the SMIP requirements.
///

typedef enum {
  WIO2_800_7_8_8 = 1,
  WIO2_1066_9_10_10,
  LPDDR3_1066_8_10_10,
  LPDDR3_1333_10_12_12,
  LPDDR3_1600_12_15_15,  //5
  LPDDR3_1866_14_17_17,
  LPDDR3_2133_16_20_20,
  LPDDR4_1066_10_10_10,
  LPDDR4_1600_14_15_15,
  LPDDR4_2133_20_20_20,  //10
  LPDDR4_2400_24_22_22,
  LPDDR4_2666_24_24_24,
  LPDDR4_2933_28_27_27,
  LPDDR4_3200_28_29_29,
  DDR3_1066_6_6_6,       //15
  DDR3_1066_7_7_7,
  DDR3_1066_8_8_8,
  DDR3_1333_7_7_7,
  DDR3_1333_8_8_8,
  DDR3_1333_9_9_9,       //20
  DDR3_1333_10_10_10,
  DDR3_1600_8_8_8,
  DDR3_1600_9_9_9,
  DDR3_1600_10_10_10,
  DDR3_1600_11_11_11,    //25
  DDR3_1866_10_10_10,
  DDR3_1866_11_11_11,
  DDR3_1866_12_12_12,
  DDR3_1866_13_13_13,
  DDR3_2133_11_11_11,    //30
  DDR3_2133_12_12_12,
  DDR3_2133_13_13_13,
  DDR3_2133_14_14_14,
  DDR4_1333_10_10_10,
  DDR4_1600_10_10_10,    //35
  DDR4_1600_11_11_11,
  DDR4_1600_12_12_12,
  DDR4_1866_12_12_12,
  DDR4_1866_13_13_13,
  DDR4_1866_14_14_14,    //40
  DDR4_2133_14_14_14,
  DDR4_2133_15_15_15,
  DDR4_2133_16_16_16,
  DDR4_2400_15_15_15,
  DDR4_2400_16_16_16,    //45
  DDR4_2400_17_17_17,
  DDR4_2400_18_18_18,
  MaxMemoryTypeFreqConfigSmip
} MEMORY_TYPE_FREQ_CONFIG_SMIP;

///
/// This private enum was created to internally track the memory
/// types and frequencies that are supported on APLK in the MRC.
///

typedef enum {
  LPDDR3_1333_10 = 1,
  LPDDR3_1600_12,
  LPDDR3_1866_14,
  LPDDR4_1600_14,
  LPDDR4_2133_20, //5
  LPDDR4_2400_24,
  LPDDR4_2666_24,
  DDR3_1333_7,
  DDR3_1333_8,
  DDR3_1333_9,    //10
  DDR3_1333_10,
  DDR3_1600_8,
  DDR3_1600_9,
  DDR3_1600_10,
  DDR3_1600_11,   //15
  DDR3_1866_10,
  DDR3_1866_11,
  DDR3_1866_12,
  DDR3_1866_13,
  MaxMemoryTypeFreqConfig
} MEMORY_TYPE_FREQ_CONFIG;

typedef enum {
  AddMapping1KB = 0,
  AddMapping2KB,
  AddMapping4KB,
  AddMappingReserved,
} ADDRESS_MAPPING;


#if MEM_SV_HOOKS
typedef struct {
  UINT8 BoxType;
  UINT16 Offset;
  UINT32 Value;
} SMIP_SV_OPTION;
#endif

typedef enum {
  CO_PoP = 0,
  BGA,
  LP3_BGA_Mirror,
  UDIMM,
  ACRD
} PLATFORM_PACKAGE;

//
// Definition of the HALF/QTR/ONE Clock lengths.
// These are all frequency dependent.
//
//#define HALF_CLK_(DigitalDllEn, FreqIndex)      (HalfClk[ADdll[DigitalDllEn]][FreqIndex])
//#define QTR_CLK_(DigitalDllEn, FreqIndex)       (HalfClk[ADdll[DigitalDllEn]][FreqIndex] / 2)
//#define ONE_CLK_(DigitalDllEn, FreqIndex)       (HalfClk[ADdll[DigitalDllEn]][FreqIndex] * 2)
//#define MAXPI_VAL_(DigitalDllEn, FreqIndex)     (HalfClk[ADdll[DigitalDllEn]][FreqIndex] - 1)
#define HALF_CLK_(DigitalDllEn, FreqIndex)      (MrcData->HalfClkPi)
#define QTR_CLK_(DigitalDllEn, FreqIndex)       (MrcData->HalfClkPi / 2)
#define ONE_CLK_(DigitalDllEn, FreqIndex)       (MrcData->HalfClkPi * 2)
#define MAXPI_VAL_(DigitalDllEn, FreqIndex)     (MrcData->HalfClkPi - 1)
//
// This is the first byte in the assignment which provides the libraries the Type and amount of conditional values.
//
#define xxxxx                   0x00
#define xxxxT                   0x01
#define xxxCx                   0x02
#define xxxCT                   0x03
#define xxFxx                   0x04
#define xxFxT                   0x05
#define xxFCx                   0x06
#define xxFCT                   0x07
#define xPxxx                   0x08
#define xPxxT                   0x09
#define xPxCx                   0x0a
#define xPxCT                   0x0b
#define xPFxx                   0x0c
#define xPFxT                   0x0d
#define xPFCx                   0x0e
#define xPFCT                   0x0f
#define dxxxx                   0x80
#define dxxxT                   0x81
#define dxxCx                   0x82
#define dxxCT                   0x83
#define dxFxx                   0x84
#define dxFxT                   0x85
#define dxFCx                   0x86
#define dxFCT                   0x87
#define dPxxx                   0x88
#define dPxxT                   0x89
#define dPxCx                   0x8a
#define dPxCT                   0x8b
#define dPFxx                   0x8c
#define dPFxT                   0x8d
#define dPFCx                   0x8e
#define dPFCT                   0x8f

#define MILLI_DEL             0
#define MICRO_DEL             1
#define NANO_DEL              2

#define RD_REG                0x01
#define RD_ONLY               0x02
#define WR_OFF                0x04
#define FC_WR                 0x08
#define UPD_CACHE             0x10
#define FC_WR_PRINT           0x20
#define FC_PRINT              0x40

#define VLV_CPU 0x670
#define AVN_CPU 0x6D0

#define CPGC_MEMTEST_NUM 2
#define CPGC_LFSR_VICTIM_SEED       0xF294BA21 // Random seed for victim.
#define CPGC_LFSR_AGGRESSOR_SEED    0xEBA7492D // Random seed for aggressor.

#define AUTO_OPTION       15
#define DDR_CHANNELS_AUTO  0

typedef enum {
  Channel0Interposer = 0,
  Channel1Interposer
} CHANNEL_ENABLED_BY_INTERPOSER;

//
// Favor
//
#define DDR_FAVOR_DISABLED     0
#define DDR_FAVOR_PERFORMANCE  1
#define DDR_FAVOR_POWER_SAVING 2
#define DDR_FAVOR_CUSTOM       3

#endif // _MMRCPROJECTDEFINITIONS_H
