/** @file
  MMRCData.h
  Internal and external data structures, Macros, and enumerations.

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

@par Specification
**/
#ifndef _MMRCDATA_H_
#define _MMRCDATA_H_

#include "Spd.h"
#include "MmrcProjectDefinitions.h"

#if SIM || JTAG
#include <stdio.h>
#endif

///
/// MRC version description.
///
typedef union {
  struct{
    UINT8      Major;             ///< Major version number
    UINT8      Minor;             ///< Minor version number
    UINT8      Rev;               ///< Revision number
    UINT8      Build;             ///< Build number
  } Version;
  struct{
    UINT32  Major     : 4;        ///< Major version number
    UINT32  DevId     : 3;        ///< Field that can be used to identify dev that built test BIOS
    UINT32  TestBuild : 1;        ///< Flag that indicates that BIOS was built on a dev machine
    UINT32  Minor     : 8;        ///< Minor version number
    UINT32  Rev       : 8;        ///< Revision number
    UINT32  Build     : 8;        ///< Build number
  } Bits;
  UINT32     Data;
  UINT16     Data16[2];
  UINT8      Data8[4];
} MrcVersion;

typedef union {
  UINT8      Data;
  struct {
    //
    // Rank Select Interleaving Enable.  See Address Mapping section for full description.
    //     0 - Rank Select Interleaving disabled
    //     1 - Rank Select Interleaving enabled
    //
    // Bits[0:0]
    //
    UINT8    RankSelectInterleavingEnable : 1;
    //
    // Bank Address Hashing Enable.  See Address Mapping section for full description.
    //     0 - Bank Address Hashing disabled
    //     1 - Bank Address Hashing enabled
    //
    // Bits[1:1]
    //
    UINT8    BankAddressHashingEnable : 1;
    //
    // CH1 CLK Disable. Disables the CH1 CLK PHY Signal when set to 1. This is used
    // on board designs where the CH1 CLK is not routed and left floating or stubbed out.
    //     0 = CH1 CLK is enabled
    //     1 = CH1 CLK is disabled
    //
    // Bits[2:2]
    //
    UINT8    Ch1ClkDisable : 1;
    //
    // Reserved
    //
    // Bits[3]
    //
    UINT8    Reserved : 1;
    //
    // Specifies the address mapping to be used.
    //       00b - 1KB,
    //       01b - 2KB,
    //       10b - 4KB,
    //       11b - Reserved
    //
    // Bits[5:4]
    //
    UINT8    AddressMapping : 2;
    //
    // Reserved
    //
    // Bits[7:6]
    //
    UINT8    Reserved0 : 2;
  } Bits;
} CHANNEL_OPTION;

/*
GLOBAL_REMOVE_IF_UNREFERENCED const MrcVersion  cVersion[] = {
#include "MrcVersion.h"
};
*/
//
// Define each stepping in the MMRC taking the values
// generated in the spreadsheet
//

// MemoryDown configuration options
typedef enum {
MD_SODIMM,          // No memory down (SODIMM)
MD_MEMORYDOWN,      // Memory down on MB
MD_CH0_MD_CH1_SOD,  // CH0 = MemoryDown CH1 = SODIMM
MD_CH0_SOD_CH1_MD,  // CH0 = SODIMM     CH1 = MemoryDown
MD_1X32_LPDDR4,     // 1x32 LPDDR4 config
MD_ECC_MEM_DOWN     // ECC MemoryDown (no SPD)
} MemoryDownOptions;
#define IS_MEM_DOWN(enumMDO)  (( (enumMDO == MD_MEMORYDOWN) || (enumMDO == MD_ECC_MEM_DOWN) ) ? (TRUE) : (FALSE))

#define STEPPING_A0            0
#define STEPPING_A1            1
#define STEPPING_E0            0x10
#define STEPPING_XA0           0x20
#define STEPPING_X2A0          0x30
#define STEPPING_P0            0x40    //stepping P A0
#define STEPPING_P_B0          0x50    //stepping P B0

#define SERIES_BXT             0
#define SERIES_BXT_1           1
#define SERIES_BXT_X           2
#define SERIES_BXT_P           3

#define MRC_WEAK_ODT_CONFIG    1

#define MRC_SMIP_DDR3L_TX_ODT_RTT_60_OHMS 0  // MR2.RTT_WR == RZQ/4
#define MRC_SMIP_DDR3L_TX_ODT_RTT_120_OHMS 1 // MR2.RTT_WR == RZQ/2

#define MRC_SMIP_DDR3L_MAX_85_DEGREES_C       0
#define MRC_SMIP_DDR3L_MAX_95_DEGREES_C       1

#if MRC_FLOW
#if !defined SIM  && !defined JTAG
#define PRINT_FUNCTION_INFO     MmrcDebugPrint ((MMRC_DBG_MIN, "%a\n", __FUNCTION__ ));
#define PRINT_FUNCTION_INFO_MAX MmrcDebugPrint ((MMRC_DBG_MAX, "%a\n", __FUNCTION__ ));
#else
#define PRINT_FUNCTION_INFO     MmrcDebugPrint ((MMRC_DBG_MIN, "%s\n", __FUNCTION__ ));
#define PRINT_FUNCTION_INFO_MAX MmrcDebugPrint ((MMRC_DBG_MAX, "%s\n", __FUNCTION__ ));
#endif
#else
#define PRINT_FUNCTION_INFO
#define PRINT_FUNCTION_INFO_MAX
#endif

#define MIN(a,b)          (((a) < (b)) ? (a) : (b))
#define MAX(a,b)          (((a) > (b)) ? (a) : (b))
#define GET_ABSOLUTE(a,b) (((a) > (b)) ? (a - b) : (b - a))
#define MMRC_NA           0xFF

#define HPET_1US          14318
#define HPET_1MS          0x37EF

#define EXTERNAL      0xFE
#define INTERNAL      0xFE

#define GETSETVREF    0
#define GETSETDELAY   1
#define GETCMD        2
#define NONE          3
#define LOW           0
#define HIGH          1

#ifndef ABSOLUTE
#define ABSOLUTE                      1
#define RELATIVE                      2
#endif
#define CONSOLIDATE_STROBES           4
#define MULTIKNOB                     8
#define SWEEP_ALL_STROBES_TOGETHER    16
#define START_SWEEP_CENTER            32
#define MULTISTEP_DIM2                64

//
// Definitions for specific entries when accessing 1DSweep return data.
//
#define KNOB_0            0
#define KNOB_1            1
//
// Bit field settings for determining which rd/wr training delay controls are shared.
//
#define RANK_SHARED       1
#define STROBE_SHARED     4
#define KNOB_SHARED       2
#define BIT_SHARED        8
#define BIT_CONTROL       16
#define CHANNEL_SHARED    32
#define CALCULATE_BIT     64  // When BIT is not shared, the per bit can be calculated.
#define SWEEP_BIT         0   // When BIT is not shared, the per bit must be swept.
#define NO_SHARED         0

//
// DRAM Technology Specific Max Strobes
//
#define MAX_STROBES_DDR3L        8
#define MAX_STROBES_DDR3L_ECC    9
#define MAX_STROBES_LPDDR3       4
#define MAX_STROBES_LPDDR4_BGA   4
#define MAX_STROBES_LPDDR4_COPOP 2

//
// Defines for register initialization table.
//
#define C_(c)      (c)
#define _I(i)      (i)
#define CI(c,i)    (c), (i)
#define RB(r,b)    (((r & 0x0f) << 4) + ((b & 0x0f) << 0))
#define BB(bt,be)  (((bt & 0x0f) << 4) + ((be & 0x0f) << 0))
#define BB2(bt,be) (bt & 0xff), (be & 0xff)
#define AP(ac,pf)  (((ac & 0x07) << 5) + ((pf & 0x1f) << 0))
#define MASK1(m)   CONV1(m)
#define MASK2(m)   CONV2(m)
#define MASK3(m)   CONV3(m)
#define MASK4(m)   CONV4(m)
#define MASK5(m)   CONV5(m)
#define MASK6(m)   CONV6(m)
#define MASK7(m)   CONV7(m)
#define MASK8(m)   CONV8(m)
#define VAL1(m)    CONV1(m)
#define VAL2(m)    CONV2(m)
#define VAL3(m)    CONV3(m)
#define VAL4(m)    CONV4(m)
#define VAL5(m)    CONV5(m)
#define VAL6(m)    CONV6(m)
#define VAL7(m)    CONV7(m)
#define VAL8(m)    CONV8(m)
//
// Frequencies
//
#define MMRC_800           0x00
#define MMRC_1066          0x01
#define MMRC_1333          0x02
#define MMRC_1600          0x03
#define MMRC_1866          0x04
#define MMRC_2133          0x05
#define MMRC_2400          0x06
#define MMRC_2666          0x07
#define MMRC_3200          0x08
#define MMRC_2933          0x09
#define MMRC_3777          0x0A

//
// DRAM Densities in units of Gb
//
#define DRAM_DENSITY_2Gb    2
#define DRAM_DENSITY_4Gb    4
#define DRAM_DENSITY_6Gb    6
#define DRAM_DENSITY_8Gb    8
#define DRAM_DENSITY_12Gb   12
#define DRAM_DENSITY_16Gb   16

#define C_MAXDDR          MAXDDR - MINDDR + 1

#define COREFREQ_800      0x00
#define COREFREQ_1066     0x01
#define COREFREQ_1333     0x02
#define COREFREQ_1600     0x03

#define C_MAXCOREFREQ     MAXCOREFREQ - MINCOREFREQ + 1

#define STROBE 0

#define MMRC_ENTRY         1
#define MMRC_EXIT          0

//
// Our current tools don't know about the RDRAND instruction yet.
// Note that this encoding will not work in a 16bit code segment.
// In 64bit CPU mode, this will fill RAX but we still only
// use the low 32 bits.
//
#define RDRAND_EAX  __asm _emit 0x0f __asm _emit 0xc7 __asm _emit 0xf0

#pragma pack(push)
#pragma pack(1)

//
// MEMORY_DEVICE_DENSITY
// Do not modify this enum.
// These entries must match the DUNIT register definition for DRP0.DDEN
//
typedef enum {
  DeviceDensity4Gb = 0,
  DeviceDensity6Gb,
  DeviceDensity8Gb,
  DeviceDensity12Gb,
  DeviceDensity16Gb,
  MaxMemoryDeviceDensity
} MEMORY_DEVICE_DENSITY;

//
// DRAM_DENSITY_POLICY_INPUTS
// This enum lists all of the supported DRAM_POLICY.DramDensity inputs
//
typedef enum {
  SMIP_DramDensity4Gb = 0,
  SMIP_DramDensity6Gb,
  SMIP_DramDensity8Gb,
  SMIP_DramDensity12Gb,
  SMIP_DramDensity16Gb,
  SMIP_DramDensity2Gb,
  SMIP_MaxDramDeviceDensity
} SMIP_DRAM_DEVICE_DENSITY;

//
// DRAM_WIDTH_POLICY_INPUTS
// This enum lists all of the supported DRAM_POLICY.DramWidth inputs
//
typedef enum {
  SMIP_DramWidthx8 = 0,
  SMIP_DramWidthx16,
  SMIP_DramWidthx32,
  SMIP_MaxDramDeviceWidth
} SMIP_DRAM_DEVICE_WIDTH;

typedef enum {
  DevicesPerRank1 = 0,
  DevicesPerRank2,
  DevicesPerRank4,
  DevicesPerRank8
} DRAM_DEVICES_PER_RANK;

typedef enum {
  BootFrequency,
  HighFrequency,
  MaxFrequencyIndex
} FREQUENCY_INDEX;

typedef enum {
  PC_INIT,
  PC_WAITFOR_INIT,
  PC_WAITFOR_START,
  PC_WAITFOR_END,
  PC_GOTOEND,
  PC_END
} POSTCODE_STATE;

typedef struct {
  UINT32 StartPostCodeEnabled;
  UINT32 StartPostCode;
  UINT32 StopPostCodeEnabled;
  UINT32 StopPostCode;
  UINT32 InitPostCode;
  UINT32 PcCounter;
  POSTCODE_STATE PostCodeState;
} POSTCODEDATA;

typedef enum {
  MrcMemInit,
  MrcMemTest
} MEM_INIT_OR_TEST;

typedef enum {
  MrcNoHaltSystemOnError,
  MrcHaltSystemOnError,
  MrcHaltCpgcEngineOnError,
  MrcNoHaltCpgcEngineOnError,
  MrcMaxHaltType
} HALT_TYPE;

typedef enum {
  TimingTras = 0,
  TimingTrp,
  TimingTrcd,
  TimingTwr,
  TimingTrfc,
  TimingTwtr,
  TimingTrrd,
  TimingTrrdl,
  TimingTrtp,
  TimingTfaw,
  TimingTccd,
  TimingTccdl,
  TimingTwtp,
  TimingTwcl,
  TimingTcmd,
  TimingTcl,
  TimingTrppb,
  TimingTrpab,
  TimingTrcpb,
  TimingTwtrl,
  TimingTrdpre,
  TimingTwrpre,
  TimingTrefi,
  TimingTxs,
  TimingTxsdll,
  TimingTxsr,
  TimingTckesre,
  TimingTxp,
  TimingTxpdll,
  TimingTcke,
  TimingTpd,
  TimingTprpden,
  TimingTrdpden,
  TimingTwrpden,
  TimingTrefpden,
  TimingTmrspden,
  TimingTcpded,
  TimingTmprr,
  TimingTmrd,
  TimingTmod,
  TimingTmrr,
  TimingTmrw,
  TimingOdtlon,
  TimingOdtloff,
  TimingOdth4,
  TimingOdth8,
  TimingTaonpdmin,
  TimingTaonpd,
  TimingTaofpdmin,
  TimingTaofpdmax,
  TimingTrpre,
  TimingTwpre,
  TimingTrrsr,
  TimingTrrsrl,
  TimingTrrdr,
  TimingTrrdd,
  TimingTwwsr,
  TimingTwwsrl,
  TimingTwwdr,
  TimingTwwdd,
  TimingTrwsr,
  TimingTrwdr,
  TimingTrwdd,
  TimingTwrsr,
  TimingTwrdr,
  TimingTwrdd,
  TimingTxpr,
  TimingTdllk,
  TimingTzqinit,
  TimingTzqoper,
  TimingTzqcs,
  TimingTzqreset,
  MaxTimingData
} TIMING_DATA_TYPE;

//
// Bit definitions
//
#ifndef BIT0
#define BIT0                  0x00000001
#define BIT1                  0x00000002
#define BIT2                  0x00000004
#define BIT3                  0x00000008
#define BIT4                  0x00000010
#define BIT5                  0x00000020
#define BIT6                  0x00000040
#define BIT7                  0x00000080
#define BIT8                  0x00000100
#define BIT9                  0x00000200
#define BIT10                 0x00000400
#define BIT11                 0x00000800
#define BIT12                 0x00001000
#define BIT13                 0x00002000
#define BIT14                 0x00004000
#define BIT15                 0x00008000
#define BIT16                 0x00010000
#define BIT17                 0x00020000
#define BIT18                 0x00040000
#define BIT19                 0x00080000
#define BIT20                 0x00100000
#define BIT21                 0x00200000
#define BIT22                 0x00400000
#define BIT23                 0x00800000
#define BIT24                 0x01000000
#define BIT25                 0x02000000
#define BIT26                 0x04000000
#define BIT27                 0x08000000
#define BIT28                 0x10000000
#define BIT29                 0x20000000
#define BIT30                 0x40000000
#define BIT31                 0x80000000
#endif

#define EVEN_MODE             1
#define ODD_MODE              0

#define OVERRIDE              1
#define NO_OVERRIDE           0

#define ZEROS                 0x00
#define ONES                  0xff

#define NOT_MET               0
#define MET                   1
#define MET2                  2
#define UP                    1
#define DN                    0

#define COUNT_UP_FOR_ONE      1
#define COUNT_DN_FOR_ZERO     0
#define SKIP_ACTION           0
#define TAKE_ACTION           1
#define PFCT_GLOBAL           0
#define PFCT_INDEPENDENT      0

#define LOC_FLAGS_U(x)        (((x[6])>>5) & 1)

#define RDWR_NOT_DONE         0
#define RDWR_DONE             1
#define RDWR_LOW              0
#define RDWR_HIGH             1

//
// RD/WR Leveling
//
#define RD_LEVELING           1
#define WR_LEVELING           2

#define CMDLOWHIGH_NONE   0
#define CMDLOWHIGH_LOW    1
#define CMDLOWHIGH_HIGH   2
#define CMDLOWHIGH_BOTH   3
#define LOOKING_FOR_FAILURE 0
#define FOUND_FAILURE       1
#define PHASE0_SEARCH_LOW   0
#define PHASE0_SEARCH_HIGH  1
#define PHASE0_FOUND_HIGH   2

#define REG_UNDEFINED     0xFF
//
// Macros for converting Little-Endian to Big-Endian.
//
#define CONV1(x)              ((x)&0xff)
#define CONV2(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff)
#define CONV3(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff), (((x)>>16)& 0xff)
#define CONV4(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff), (((x)>>16)& 0xff), (((x)>>24)& 0xff)
#define CONV5(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff), (((x)>>16)& 0xff), (((x)>>24)& 0xff), (((x)>>32)& 0xff)
#define CONV6(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff), (((x)>>16)& 0xff), (((x)>>24)& 0xff), (((x)>>32)& 0xff), (((x)>>40)& 0xff)
#define CONV7(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff), (((x)>>16)& 0xff), (((x)>>24)& 0xff), (((x)>>32)& 0xff), (((x)>>40)& 0xff), (((x)>>48)& 0xff)
#define CONV8(x)              (((x)>>0)& 0xff), (((x)>>8)& 0xff), (((x)>>16)& 0xff), (((x)>>24)& 0xff), (((x)>>32)& 0xff), (((x)>>40)& 0xff), (((x)>>48)& 0xff), (((x)>>56)& 0xff)

//
// Macros for MASK/VAL which are used in the projectlibraries.c declarations.
//
#define MASK(x)               CONV4(x)
#define VAL(x)                CONV4(x)

#define ASSIGNDONE            (0xff)
#define DETREGLIST_NULL       ((DETAILED_REGISTER_STRING *) 0xFFFFFFFF)
#define REGLIST_NULL          ((REGISTER_STRING *)    0xFFFFFFFF)

//
// RMT Definitions
//
#define RMT_PR_EACH_RANK            BIT0
#define RMT_PR_START                BIT1              // should equal FIRST PR test (e.g. PR_TA)
#define RMT_PR_TA                   BIT1
#define RMT_PR_LFSR                 BIT2
#define RMT_SUPPORT_ENABLED         BIT0             // Enables RMT
#define RMT_SUPPORT_EV_MODE         BIT1             // Increases the Loopcount and Burst Number
#define RMT_SUPPORT_COMMAND_MODE    BIT2             // Enables commands thru SKPD registers
#define RMT_SUPPORT_PPV_MODE        (BIT1|BIT2)      // Customize loopcount, signals for PPV test
#define RMT_SUPPORT_PRODUCTION_MODE BIT3

#define RMT_NO_ACTION                     0
#define RMT_PIMOVE_JEDEC_RESET            BIT0    // Executes a Jedec reset prior to start any test
#define RMT_FAIL_RESTORE_CMD              BIT1
#define RMT_FAIL_JEDEC_RESET              BIT2    // Executes a Jedec reset after a failure is detected
#define RMT_FAIL_RESTORE_PI               BIT3
#define RMT_FAIL_RESTORE_PI_LAST_VALUE    BIT4
#define RMT_FAIL_RESTORE_PI_CENTER        BIT5    // Restores the center of the delay after a failure is detected
#define RMT_FAIL_SINGLE_STROBE            BIT6    // Detects any failure across any strobe and stops the test

// PRODUCTION MODE defines based on MrcData->RmtChkRun user inputs
typedef enum {
  NO_RMT = 0,
  L1_CHECK = 1,
  L2_CHECK = 2,
  BOTH_CHECK =3
} RMT_PROD_MODE;

typedef enum {
  RMT_COMMAND_NO_COMMAND,
  RMT_COMMAND_SET_LOOPCOUNT,
  RMT_COMMAND_SET_NUMBURSTS,
  RMT_COMMAND_EXIT,
  RMT_COMMAND_RUN,
  RMT_COMMAND_ENABLE_SIGNAL,
  RMT_COMMAND_RESET_DIMMS,
} RMT_COMMANDS_TYPE;
//
// Read Value from cache
//
#define CMD_GET_CACHE         (RD_ONLY)
//
// Read from register
//
#define CMD_GET_REG           (RD_ONLY|RD_REG)
//
// Read from register and update Value to cache
//
#define CMD_GET_REG_UC        (RD_ONLY|RD_REG|UPD_CACHE)
//
// Read register, apply offset, write back to register
//
#define CMD_SET_OFFSET        (WR_OFF|FC_WR)
//
// Write to register with offset to Cache Value, apply Value condition checking and update final Value to cache
//
#define CMD_SET_OFFCAC_UC     (WR_OFF|UPD_CACHE)
//
// Write to register with offset to Reg Value, apply Value condition checking and update final Value to cache
//
#define CMD_SET_OFFREG_UC     (WR_OFF|RD_REG|UPD_CACHE)
//
// Write to register with offset to Cache Value, force write with NO Value condition checking and update final Value to cache
//
#define CMD_SET_OFFCAC_FC_UC  (WR_OFF|FC_WR|UPD_CACHE)
//
// Write to register with offset to Reg Value, force write with NO Value condition checking and update final Value to cache
//
#define CMD_SET_OFFREG_FC_UC  (WR_OFF|FC_WR|RD_REG|UPD_CACHE)
//
// Write to register with input Value, apply Value condition checking and update final Value to cache
//
#define CMD_SET_VAL_UC        (UPD_CACHE)
//
// Write to register with input Value, apply Value condition checking
//
#define CMD_SET_VAL           0
//
// Write to register with input Value, force write with NO Value condition checking and update final Value to cache
//
#define CMD_SET_VAL_FC_UC     (FC_WR|UPD_CACHE)
//
// Write to register with input Value, force write with NO Value condition checking
//
#define CMD_SET_VAL_FC        (FC_WR)

#define STATUS_PASS                0
#define STATUS_FAIL                1
#define STATUS_TYPE_NOT_SUPPORTED  2
#define STATUS_CMD_NOT_SUPPORTED   3
#define STATUS_LIMIT               4

//
// Voltages
//
#define DDR_VOLTAGE_NOT_DETECTED  0
#define DDR_VOLTAGE_125V          1
#define DDR_VOLTAGE_135V          2
#define DDR_VOLTAGE_150V          3

#ifdef MINIBIOS
#define Shr64(Value, Shift) LShiftU64 (Value, Shift)
#define Shl64(Value, Shift) LRightU64 (Value, Shift)
#endif

#define ODD_MODE_BITSHIFTS    16
#define EVEN_MODE_BITSHIFTS   1

//
// 1D/2D Eye Diagram Debug Message Parameters
//
#define MARGIN_DISTANCE(x,y)     ((x)>(y) ? (x)-(y) : (y)-(x))
#define MAX_LINE_SIZE            80
#define  EYEDATA_1D_VREFPI_OFFSET 0

typedef enum {
  MmrcNotStarted = 0,
  MmrcStarted,
  MmrcFinished
} MMRC_STATUS_FLOW;

typedef enum {
  TypeDdr3 = 0,
  TypeDdr3L,
  TypeDdr3ECC,
  TypeDdr3All,
  TypeLpDdr2,
  TypeLpDdr3,
  TypeLpDdr4,
  TypeDdr4,
  TypeWio2,
  TypeDdrT,
  MaxMemType
} MEM_TYPE;

#pragma pack(1)
//
// Power Knobs supported
//
typedef enum {
  SocOdt = 0,
  DramRttWrOdt,
  DramRttNomOdt,
  MaxPowerKnobs
} POWER_KNOBS;

typedef enum {
  GlobalFirstTimeExecution = 0, // First time execution
  TimingBelowTrigger,           // Trigger when the timing margin doesn't comply with the specs
  VoltageBelowTrigger,          // Trigger when the voltage margin doesn't comply with the specs
  ExecuteTrainingTrigger,       // When there was a drop of 'x' number of ticks and we need to re-train
  FailureDetectedOnFirstKnob,   // If a failure is detected during the really first Power Knob value do not continue execution
  MaxTriggerLog
} TRIGGER_LOG;

typedef enum {
  ExecuteRead = 0,
  ExecuteWrite,
  MaxTrainingRequired
} TRAINING_REQUIRED;

typedef enum {
  MarginT,
  MarginV,
  MaxMarginVerify
} MARGIN_VERIFY;

typedef enum {
  Absolute = 0,         // We should never go behind this value
  AbsoluteTrigger,      // Once the absolute trigger has been reached, so we are close to the edge
  Relative,             // After the absoluteTrigger has been reached make sure the margin doesn't drop drastically
  ExecuteTrainingDrop,  // If the Margin Drops to much Re-Execute the required training steps to re-center
  MaxPowerCriteria
} POWER_CRITERIA;
//
// Supported DRAM typed for Power Training
//
typedef enum {
  PowerDdr3l = 0,
  PowerLpddr3,
  MaxDdrTypesSupportedForPower
} DDR_TYPES_SUPPORTED_FOR_POWER;
//
// Allocated from Stronger to weaker
//
typedef enum {
  Odt20,
  Odt30,
  Odt40,
  Odt60,
  Odt90,
  Odt120,
  Odt150,
  Odt180,
  Odt210,
  Odt240,
  OdtInfinite,     // "OFF"
  MaxOdtValues,
  InvalidOdtValue
} ODT_VALUES;

#define   SOC_ODT_KNOB_ENABLE         BIT0
#define   DIMM_RTTNOM_KNOB_ENABLE     BIT1
#define   DIMM_RTTWR_KNOB_ENABLE      BIT2
#define   DEBUG_RMT_ENABLE            BIT3

/**
  Strobe to CPGC error status bit conversion
**/
typedef struct {
  UINT16 DynamicVar;
  UINT32 ErrorMask;
} STROBE_TO_ERROR_MASK;


/**
  Initial values for each power nob
**/
typedef struct {
  UINT8   InitialValue[MaxDdrTypesSupportedForPower][MaxPowerKnobs];
  UINT8   FinalValue[MaxDdrTypesSupportedForPower][MaxPowerKnobs];
  BOOLEAN Execute[MaxTrainingRequired]; // Only used when both Read & Write Training need to be executed
} POWER_KNOB_VALUES;

//
// Rank Margin Tool Data Index
//
typedef enum {
  RxDqLeft,
  RxDqRight,
  RxDqHLeft,
  RxDqHRight,
  RxDqLLeft,
  RxDqLRight,
  RxVLow,
  RxVHigh,
  TxDqLeft,
  TxDqRight,
  TxVLow,
  TxVHigh,
  RcvEnLeft,
  RcvEnRight,
  CmdLeft,
  CmdRight,
  CtlLeft,
  CtlRight,
  ClkLeft,
  ClkRight,
  MaxRMTData,
} RMT_DATA_TYPE;

enum {
  CPGC_CMDSEQ_WRITE,
  CPGC_CMDSEQ_READ,
  CPGC_CMDSEQ_WRITE_READ
};

//
// One for each type of distinct CPGC algo support.  Used by big case statement in CpgcSetup,..
//
enum {
  CPGC_CONFIG_MEMINIT = 0,
  CPGC_CONFIG_MEMTEST,
  CPGC_CONFIG_VA,
  CPGC_CONFIG_EARLYTRAINING,
  CPGC_CONFIG_EARLY_MPR_TRAINING,
  CPGC_CONFIG_TURNAROUNDS,
  CPGC_CONFIG_LFSR_ADDRESS,
  CPGC_CONFIG_CMD_STRESS
} CPGC_CONFIG_TYPES;

typedef enum  {
  Pfct =  0,
  PfctT,
  PfctC,
  PfctCT,
  PfctF,
  PfctFT,
  PfctFC,
  PfctFCT,
  PfctP,
  PfctPT,
  PfctPC,
  PfctPCT,
  PfctPF,
  PfctPFT,
  PfctPFC,
  PfctPFCT,
  MaxPfct,
} ENUM_PCFT_TYPE;


//
// Byte Enables supported in the CAPSULEINIT Sequences.
//
typedef enum {
  _00000000,
  _00000001,
  _00000010,
  _00000011,
  _00000100,
  _00000101,
  _00000110,
  _00000111,
  _00001000,
  _00001001,
  _00001010,
  _00001011,
  _00001100,
  _00001101,
  _00001110,
  _00001111,
  _00010000,
  _00010001,
  _00010010,
  _00010011,
  _00010100,
  _00010101,
  _00010110,
  _00010111,
  _00011000,
  _00011001,
  _00011010,
  _00011011,
  _00011100,
  _00011101,
  _00011110,
  _00011111,
  _00100000,
  _00100001,
  _00100010,
  _00100011,
  _00100100,
  _00100101,
  _00100110,
  _00100111,
  _00101000,
  _00101001,
  _00101010,
  _00101011,
  _00101100,
  _00101101,
  _00101110,
  _00101111,
  _00110000,
  _00110001,
  _00110010,
  _00110011,
  _00110100,
  _00110101,
  _00110110,
  _00110111,
  _00111000,
  _00111001,
  _00111010,
  _00111011,
  _00111100,
  _00111101,
  _00111110,
  _00111111,
  _01000000,
  _01000001,
  _01000010,
  _01000011,
  _01000100,
  _01000101,
  _01000110,
  _01000111,
  _01001000,
  _01001001,
  _01001010,
  _01001011,
  _01001100,
  _01001101,
  _01001110,
  _01001111,
  _01010000,
  _01010001,
  _01010010,
  _01010011,
  _01010100,
  _01010101,
  _01010110,
  _01010111,
  _01011000,
  _01011001,
  _01011010,
  _01011011,
  _01011100,
  _01011101,
  _01011110,
  _01011111,
  _01100000,
  _01100001,
  _01100010,
  _01100011,
  _01100100,
  _01100101,
  _01100110,
  _01100111,
  _01101000,
  _01101001,
  _01101010,
  _01101011,
  _01101100,
  _01101101,
  _01101110,
  _01101111,
  _01110000,
  _01110001,
  _01110010,
  _01110011,
  _01110100,
  _01110101,
  _01110110,
  _01110111,
  _01111000,
  _01111001,
  _01111010,
  _01111011,
  _01111100,
  _01111101,
  _01111110,
  _01111111,
  _10000000,
  _10000001,
  _10000010,
  _10000011,
  _10000100,
  _10000101,
  _10000110,
  _10000111,
  _10001000,
  _10001001,
  _10001010,
  _10001011,
  _10001100,
  _10001101,
  _10001110,
  _10001111,
  _10010000,
  _10010001,
  _10010010,
  _10010011,
  _10010100,
  _10010101,
  _10010110,
  _10010111,
  _10011000,
  _10011001,
  _10011010,
  _10011011,
  _10011100,
  _10011101,
  _10011110,
  _10011111,
  _10100000,
  _10100001,
  _10100010,
  _10100011,
  _10100100,
  _10100101,
  _10100110,
  _10100111,
  _10101000,
  _10101001,
  _10101010,
  _10101011,
  _10101100,
  _10101101,
  _10101110,
  _10101111,
  _10110000,
  _10110001,
  _10110010,
  _10110011,
  _10110100,
  _10110101,
  _10110110,
  _10110111,
  _10111000,
  _10111001,
  _10111010,
  _10111011,
  _10111100,
  _10111101,
  _10111110,
  _10111111,
  _11000000,
  _11000001,
  _11000010,
  _11000011,
  _11000100,
  _11000101,
  _11000110,
  _11000111,
  _11001000,
  _11001001,
  _11001010,
  _11001011,
  _11001100,
  _11001101,
  _11001110,
  _11001111,
  _11010000,
  _11010001,
  _11010010,
  _11010011,
  _11010100,
  _11010101,
  _11010110,
  _11010111,
  _11011000,
  _11011001,
  _11011010,
  _11011011,
  _11011100,
  _11011101,
  _11011110,
  _11011111,
  _11100000,
  _11100001,
  _11100010,
  _11100011,
  _11100100,
  _11100101,
  _11100110,
  _11100111,
  _11101000,
  _11101001,
  _11101010,
  _11101011,
  _11101100,
  _11101101,
  _11101110,
  _11101111,
  _11110000,
  _11110001,
  _11110010,
  _11110011,
  _11110100,
  _11110101,
  _11110110,
  _11110111,
  _11111000,
  _11111001,
  _11111010,
  _11111011,
  _11111100,
  _11111101,
  _11111110,
  _11111111,
}  BYTE_ENABLES;

//
// PFCT Variations supported in the CAPSULEINIT Sequences.
//
typedef enum {
  _____ = 0,
  ____T,
  ___C_,
  ___CT,
  __F__,
  __F_T,
  __FC_,
  __FCT,
  _P___,
  _P__T,
  _P_C_,
  _P_CT,
  _PF__,
  _PF_T,
  _PFC_,
  _PFCT,
  D____,
  D___T,
  D__C_,
  D__CT,
  D_F__,
  D_F_T,
  D_FC_,
  D_FCT,
  DP___,
  DP__T,
  DP_C_,
  DP_CT,
  DPF__,
  DPF_T,
  DPFC_,
  DPFCT,
} DPFCT_TYPES;


typedef enum {
  SidebandPort,
  AddressOffset
} INSTANCE_OFFSET_TYPE;

typedef struct {
  UINT8   SpdPresent;
  UINT8   Buffer[MAX_SPD_ADDR + 1];
} SPD_DATA;

/**
  Data structure to convert GSM_LT level to a blueprint.
**/
typedef struct {
  UINT16 PMask;
  UINT16 FMask;
  UINT16 CMask;
  UINT16 TMask;
  UINT8  IoLevel;
  UINT8  Blueprint;
} PFCT_AND_IO_LEVEL_TO_BLUEPRINT;

//
// Layout definitions  of the elements array.
//
#define REG_INDEX               0x3000                                  // Starting index for the delay group registers.
#define GROUP_INDEX             0x4000                                  // Starting index for the Signal Group Type Index.
#define UNIQUE_INDEX            0x5000                                  // Starting index for generic registers that aren't tied to a delay group.
#define ALGO_REG_INDEX          0x1000                                  // Starting index in the TYPE field for Signal Group Modules per blueprint.
#define SGTI_INDEX              0x2000                                  // Starting index for the Signal Group Type Index per blueprint.
#define GSM_GT_INDEX            0xE000                                  // Starting index for GSM_GT enumerated type.
#define GSM_CSN_INDEX           0xF000                                  // Starting index for GSM_CSN enumerated type.
#define INDIV_ELEMENT_INDEX     (NumberAlgos * NumberOfElementsPerAlgo) // Starting index of non-linear delay elements.
//
// The GetSet functions support a level parameter to select the appropriate DDRIO interface in the memory hierarchy
//
typedef enum {
  PreDdrLevel   = 0,        // Near memory before DDR such as wide IO.
  DdrLevel,                 // DDR memory.
  LrbufLevel,               // Refers to data level at backside of LRDIMM or AEP buffer
  RegLevelA,                // Refers to cmd level at backside of register side A
  RegLevelB,                // Refers to cmd level at backside of register side B
  GsmLtMax,
  GsmLtDelim    = 0xFF
} GSM_LT;

typedef enum {
  RecEnDelay    = GSM_GT_INDEX,
  RxDqsDelay,
  RxDqDelay,
  RxDqsPDelay,
  RxDqsNDelay,
  RxVref,
  RxEq,
  RxDqBitDelay,
  RxVoc,
  RxOdt,
  RxOdtUp,
  RxOdtDn,
  DramDrvStr,
  WrLvlDelay,
  TxDqsDelay,
  TxDqDelay,
  TxVref,
  TxEq,
  TxDqBitDelay,
  TxRon,
  TxRonUp,
  TxRonDn,
  TxSlewRate,
  TxImode,
  WrOdt,
  NomOdt,
  ParkOdt,
  TxTco,
  CmdAll,
  CmdGrp0,
  CmdGrp1,
  CmdGrp2,
  CtlAll,
  CtlGrp0,
  CtlGrp1,
  CtlGrp2,
  CtlGrp3,
  CtlGrp4,
  CtlGrp5,
  CmdCtlAll,
  CkAll,
  CkGrp0,
  CkGrp1,
  TxCaVref,
  AlertVref,
  CmdRon,
  EridDelay,
  EridVref,
  ErrorVref,
  ReqVref,
  GsmGtMax,
  GsmGtDelim  = 0xFFFF
} GSM_GT;

typedef enum {
  SigRasN     = GSM_CSN_INDEX,
  SigCasN,
  SigWeN,
  SigBa0,
  SigBa1,
  SigBa2,
  SigA0,
  SigA1,
  SigA2,
  SigA3,
  SigA4,
  SigA5,
  SigA6,
  SigA7,
  SigA8,
  SigA9,
  SigA10,
  SigA11,
  SigA12,
  SigA13,
  SigA14,
  SigA15,
  SigA16,
  SigA17,
  SigCs0N,
  SigCs1N,
  SigCs2N,
  SigCs3N,
  SigCs4N,
  SigCs5N,
  SigCs6N,
  SigCs7N,
  SigCs8N,
  SigCs9N,
  SigCke0,
  SigCke1,
  SigCke2,
  SigCke3,
  SigCke4,
  SigCke5,
  SigOdt0,     // Could also be used for CA-ODT for LP4
  SigOdt1,     // Could also be used for CA-ODT for LP4
  SigOdt2,
  SigOdt3,
  SigOdt4,
  SigOdt5,
  SigPar,
  SigAlertN,
  SigBg0,
  SigBg1,
  SigActN,
  SigCid0,
  SigCid1,
  SigCid2,
  SigCk0,
  SigCk1,
  SigCk2,
  SigCk3,
  SigCk4,
  SigCk5,
  SigGnt0,
  SigGnt1,
  SigErid00,
  SigErid01,
  SigErid10,
  SigErid11,
  SigErr0,
  SigErr1,
  SigCa00,    // First instantiation of the CA bus for a given channel
  SigCa01,
  SigCa02,
  SigCa03,
  SigCa04,
  SigCa05,
  SigCa10,    // Second instantiation of the CA bus for a given channel
  SigCa11,
  SigCa12,
  SigCa13,
  SigCa14,
  SigCa15,
  GsmCsnMax,
  GsmCsnDelim = 0xFFFF
} GSM_CSN;

typedef enum {
  CACHE,
  NO_CACHE
} CacheType;

typedef enum {
  ChNone  = 0xFE,
  ChAll   = 0xFF,
  ChAllF  = 0xEE,
  ChAllP  = 0xE0
} CHANNEL_SELECTION;


typedef enum {
  SmbusReadByte,
  SmbusWriteByte,
  SmbusReadBlock,
  SmbusWriteBlock,
  MaxSmbusCmd
} SMBUS_CMD_TYPES;

typedef enum {
  Rate0x  = 0,  // Disabled
  Rate1x  = 1,
  Rate2x  = 2,
  Rate4x  = 4
} REFRESH_RATE_SELECTION;

//
// PATTERN MODES
//
#define PATTERN_VICAGG        0
#define LFSR_VICAGG           1
#define LFSR                  2

//
// Structures used to display registers.
//
typedef struct {
  char  *RegisterName;
  char  *RegisterDescription;
  UINT32 Offset;
  UINT8  NumFields;
  UINTX  Mask;
  UINTX  DefaultValue;
} RegisterInformation;

typedef struct {
  UINT8  StartBit;
  UINT8  EndBit;
  UINTX  DefaultFieldValue;
  char  *AccessType;
  char  *FieldName;
  char  *Description;
} RegisterField;

typedef struct {
  RegisterInformation *RegInfo;
  RegisterField       *RegFields;
} Register;

typedef struct {
  UINT16  location;
  UINT8   cachedIndex;
} DELAY_ELEMENT;

typedef struct {
  UINT32      RegisterOffset;
  UINT8       Lsb;
  UINT8       Msb;
  UINT8       ValueOffset;
  UINT8       Unit;
  UINT8       Name[20];
} DUNIT_PARAMETERS_STRUCT;

typedef enum {
  NoUnit,
  Clk,
  Mhz,
  TccdUnit,
  PwDDlyUnit,
  MaxUnitTypes,
} UNIT_TYPE;

typedef struct {
  UINT32  Pfct;
  UINT8   LengthVariation;
} PFCT_VARIATIONS;

typedef struct  {
  PFCT_VARIATIONS PFCTVariations[MaxPfct];
} LOCAL_PARAMS;

typedef struct {
  UINT32  Offset;           ///> Register Offset relative to the base address.
  UINTX   Mask;             ///> Starting bit within the register.
  UINT8   ShiftBit;         ///> Number of bits to shift to get to the register field in question
} REGISTER_ACCESS;

typedef struct {
  UINT16    NumberConsecutiveVoltages;
  UINT16    MinimumDelayWidth;
} EYEMASK_T;

typedef union {
  UINT32      Data;
  //
  // Remove bit field definitions from ASM include file due to MASM/H2INC limitations
  //
#ifndef ASM_INC
  struct {
    UINT32    Offset   : 12;  // PCI register offset
    UINT32    Func     : 3;   // PCI Function
    UINT32    Device   : 5;   // PCI Device
    UINT32    Bus      : 8;   // PCI Bus
    UINT32    Base     : 4;   // PCI base address
  } Bits;
#endif
} PCI_REG;

/**
  The structure definition for the floorplan which is given a
  channel and strobelane, to provide a physical channel and
  strobelane.
**/
typedef struct {
  UINT8 Channel;
  UINT8 Strobelane;
} FLOORPLAN;

typedef struct {
  UINT8  TotalInstances;
  UINT8  Instances;
  UINT8  InstanceStartIndex;
  UINT8  ReadOperation;
  UINT8  WriteOperation;
  UINT8  AccessMethod;
  UINT16  AccessBase;
  UINT8  SimulationSupported;
  UINT32 StringIndex;
} INSTANCE_PORT_MAP;

typedef struct {
  UINT16 Port;
  UINT32 Offset;
} INSTANCE_PORT_OFFSET;

typedef struct {
  UINT8  OneX[MAX_STROBES];
  UINT8  TwoX[MAX_STROBES];
  UINT8  Pi[MAX_STROBES];
  UINT8  CC0[MAX_STROBES];
  UINT8  CC1[MAX_STROBES];
  UINT8  Min[MAX_STROBES];
  UINT8  Vref[MAX_STROBES];
  UINT16 TotalDelay[MAX_STROBES];
} ELEMENTS;

/**
  Final training values stored on a per blueprint level. Needs to be per blueprint
  in case of a system with more than 1 level of memory per channel.
**/
typedef struct {
  UINT16    Values[MAX_BLUEPRINTS][MAX_NUM_ALGOS][MAX_RANKS][MAX_STROBES];
} TRAINING_SETTING;

typedef struct {
  UINT8   *LinearToPhysicalTable;   ///> Pointer to an array that contains the physical values for external vref
  UINT8   SizeOfLtoPTable;          ///> Size of the LinearToPhysicalTable
  UINT8   *PhysicalToLinearTable;   ///> Pointer to an array that contains the linear values for external vref
  UINT8   SizeOfPtoLTable;          ///> Size of the PhysicalToLinearTable
} PHY_VREF;

typedef struct {
  UINT8 ManuIDlo;
  UINT8 ManuIDhi;
  UINT8 ManuLoc;
  UINT8 ManuDateLO;
  UINT8 ManuDateHI;
  UINT8 SerialNumber1;
  UINT8 SerialNumber2;
  UINT8 SerialNumber3;
  UINT8 SerialNumber4;
  UINT8 DimmCount;
} FASTBOOTDATA;

#if FAULTY_PART_TRACKING
typedef struct {
  UINT32   Byte0to3;                  ///> On the fly tracking of the minimum bit failure used by all trainings
  UINT32   Byte4to7;                  ///> On the fly tracking of the minimum failure bit
  UINT8    ByteEcc;                   ///> On the fly tracking of the minimum failure bit
  UINT16   TrainingFail;              ///> Training that failed due to faulty part
  UINT32   DataBitFailCumulativeLow;  ///> Field to cumulate bit failures found as training progresses for data bits
  UINT32   DataBitFailCumulativeHigh; ///> Field to cumulate bit failures found as training progresses for data bits
  UINT8    EccBitFailCumulative;      ///> Field to cumulate bit failures found as training progresses for ECC byte
  UINT8    ErrorType;                 ///> 0- No error, 1-Correctable, 2-Uncorrectable
} FAULTY_PART;
#endif

#if LEAKY_BUCKET == 1
typedef struct {
  UINT64    RateUnits;                                          ///>  Rate Units specified in picoseconds
  UINT64    Rate[MAX_CHANNELS][MAX_RANKS];                      ///>  Leaky Bucket Rate specified by user
  UINT16    CorrectableErrorThreshold[MAX_CHANNELS][MAX_RANKS]; ///> Correctable Error Threshold set by user
} LEAKY_BUCKET_PARAMETERS;

#define ONE_DAY_IN_PS 1440000000000000
#define ONE_US_IN_PS 1000000
#endif

#if DRAM_RAPL == 1
typedef struct {
  UINT64    TimeWindow;               ///> Time Window for PL1
  UINT32    PowerLimit;               ///> Power Limit (PL1) specified by user (miliWatts)
  UINT8     PowerLimitEnable;         ///> Power Limit Enable for PL1
  UINT8     PowerLimitLock;           ///> Power Limit Lock for MSR
} DRAM_RAPL_PARAMETERS;

#define POWER_LIMIT_RESOLUTION   125
#endif

#if INTERLEAVING_SUPPORT
typedef enum {
  NoInterleave = 0,
  Mode0,
  Mode1,
  Mode2,
  HVM
} INTERLEAVE;
#endif

//
// BOOT_VARIABLE_NV_DATA
// Use this structure to store NV data that changes from boot to boot.
//
typedef struct {
  UINT16                    ScramblerSeed[MAX_CHANNELS];
  UINT16                    LP4_MR4VALUE[MAX_CHANNELS];
  UINT32                    BootVariableNvDataCrc;
} BOOT_VARIABLE_NV_DATA;

typedef struct {
  TRAINING_SETTING          Trained_Value;
  BOOLEAN                   Enabled;
  UINT32                    TotalMem;
  UINT8                     DimmCount;
  UINT8                     DimmPresent[MAX_DIMMS];
  UINT32                    SlotMem[MAX_DIMMS];
  SPD_DATA                  SpdData[MAX_DIMMS];
  UINT8                     D_Ranks[MAX_DIMMS];
  //
  // D_DataWidth[MAX_DIMMS]
  // 0 = x8; 1 = x16; 2 = x32
  //
  UINT8                     D_DataWidth[MAX_DIMMS];
  UINT8                     D_Banks[MAX_DIMMS];
  UINT8                     D_BusWidth[MAX_DIMMS];
  //
  // D_Size_SPD - Used in DetectDimms() during the SPD data parsing.
  // Valid values are the SPD Spec turncated to start at 2Gb = 0:
  //   | Density | SPD Byte Value - 3
  //   | 2Gb     | 0x0 (requires software workaround: D_SizeActual = DRAM_DENSITY_2Gb)
  //   | 4Gb     | 0x1
  //   | 8Gb     | 0x2
  //
  UINT8                     D_Size_SPD[MAX_DIMMS];
  //
  // D_Size - Used to program the DUNIT DRP0.DDEN regiter field.
  // Valid values are defined in MEMORY_DEVICE_DENSITY enum.
  // DDR3L  - Specifies DRAM Die density
  // LPDDR3 - Specifies Rank Density
  // LPDDR4 - Specifies Rank Density
  //
  UINT8                     D_Size[MAX_DIMMS];
  //
  // D_SizeActual - Specifies the actual density of the DRAM in situations
  // where the requested DRAM density is not supported by hardware. A non-zero
  // value will indicate that D_SizeActual should be used for the total DRAM size
  // calculation instead of D_Size
  //
  UINT8                     D_SizeActual[MAX_DIMMS];
  UINT8                     D_Type[MAX_DIMMS];
  UINT16                    DimmFrequency[MAX_DIMMS];
  BOOLEAN                   RankEnabled[MAX_RANKS];
  BOOLEAN                   EccEnabled;
  UINT32                    EccStrobes;
  UINT8                     MaxDq[MAX_RANKS];
  FASTBOOTDATA              FastBootData[MAX_DIMMS];
  UINT16                    DramType;
  UINT16                    CurrentPlatform;
  UINT16                    CurrentFrequency;
  UINT16                    CurrentConfiguration;
  UINT16                    CurrentDdrType;
  BOOLEAN                   ExecuteOnThisChannel;
  UINT16                    TimingData[16];
  INT16                     RMT_Data[MAX_RANKS + PSEUDO_RANKS][MAX_RMT_ELEMENTS][2];
#if RMT_PER_BIT_MARGIN
  INT16                     RMT_DQData[MAX_RANKS + PSEUDO_RANKS][MAX_RMT_ELEMENTS][MAX_BITS*MAX_STROBES][2];
#endif
  UINT8                     Tcl;
  UINT8                     VrefSmbAddress;
  UINT32                    PtCtl0;
  UINT8                     Dimm2DunitMapping[MAX_STROBES*8];
  UINT8                     Dimm2DunitReverseMapping[MAX_STROBES*8];
#if SIM && DDR4_SUPPORT
  UINT16                    TbMr5;
#endif
#if FAULTY_PART_TRACKING
  FAULTY_PART               FaultyPart[MAX_RANKS];
#endif
  UINT32                    WrVref[MAX_RANKS][MAX_STROBES];
  UINT32                    CaVref[MAX_RANKS];
  UINT32                    Storage[18];
  UINT8                     ASR_Supported;
  UINT8                     OdtHigh;
  UINT16                    LP4_MR0VALUE;
  CHANNEL_OPTION            ChOption;
} CHANNEL;


typedef struct {
  UINT32        Signature;
  UINT16        CurrentFrequency;
  UINT8         DVFSFreqIndex;
  UINT16        DDR3L_MRVALUE;
  UINT8         DDR3L_PageSize;
  UINT32        SystemMemorySize;
  UINT32        MrcVer;
  UINT32        CachedValues[MAX_CHANNELS][MAX_NUM_CACHE_ELEMENTS][MAX_RANKS][MAX_STROBES];
  UINT32        CachedValuesPB[MAX_CHANNELS][MAX_NUM_CACHE_ELEMENTS_PB][MAX_RANKS][MAX_STROBES][MAX_BITS];
  UINT8         BusWidth;
  CHANNEL       Channel[MAX_CHANNELS];
  UINT32        MarginCheckResultState;
  DRAM_POLICY   DramPolicyData;
  UINT32        MuxcodeNv[MaxFrequencyIndex];
  BOOLEAN       SetupMaxPiDone[MaxFrequencyIndex];
  UINT32        DataSize;
  UINT32        SaMemCfgCrc;
  UINT32        MrcParamsSaveRestoreCrc;
} MRC_PARAMS_SAVE_RESTORE;

//
// Structure for all data that is saved to NV RAM
// and restored on the subsequent boots
//
typedef struct {
  MRC_PARAMS_SAVE_RESTORE MrcParamsSaveRestore;
  BOOT_VARIABLE_NV_DATA   BootVariableNvData;
} MRC_NV_DATA_FRAME;

typedef struct {
  UINT16    MmioSize;         ///> MMIO 1 MB Aligned
  UINT16    TsegSize;         ///> TSEG 1 MB Aligned
  PHY_VREF  PhyVrefTable;
#if  1//DETECT_DIMMS
  UINT8     SpdAddressTable[MAX_CHANNELS][MAX_DIMMS];
  UINT8     *OemSpdDataMemorySolderDown[MAX_CHANNELS][MAX_DIMMS];
#endif
} MRC_OEM_FRAME;

typedef struct {
  UINT8     RcvEn;
  UINT8     RxDqs;
  UINT8     TxDqs;
  UINT8     TxDq;
} R2R_SWITCHING_PARAMETERS;

#if PASS_GATE_TEST == 1
typedef struct {
  //
  // User Input Parameters
  //
  BOOLEAN   EnableTest;                 ///> Enables/Disables the Pass Gate Test
  BOOLEAN   EnablePeriodicSelfRefresh;  ///> Enables/Disables periodic Self Refresh during the test
  UINT8     Direction;                  ///> '0': Lowest -> Highest  '1': Highest -> Lowest
  UINT32    ActivationsCount;           ///> The number of times a Row will be tested
  UINT32    IterationOnRow;             ///> How many times the test over the same row will be repeated
  UINT8     SwizzleMode;                ///> 0: Automatic 1: Force Samsung
  UINT8     Pattern;                    ///> Pattern used for the Agressor
  UINT8     TargetPattern;              ///> Pattern used for the Victim
  BOOLEAN   EnablePartialTest;          ///> Enable the execution of the Pass Gate in a memory segment
  UINT32    PartialBankRowMin;
  UINT32    PartialBankRowMax;
  BOOLEAN   EnableMontecarloSearch;     ///> MonteCarlo Search Algorithm
  UINT8     MontecarloMaxFailures;      ///> Max number of error prior to go to next Repetition
  UINT32    MontecarloStartRepetition;  ///> Max repetition number for Montecarlo Algorithm
  UINT32    MontecarloDecrement;        ///> Min repetition number for Montecarlo Algorithm
  UINT8     SpeedSelect;                ///> 0: None, 1x, 2x, 4x and 8x
  BOOLEAN   RankTest[MAX_CHANNELS][MAX_RANKS];  ///> Specifies which Ranks will be tested
  //
  // Internal variables, these have to be populated with proper value prior to test starts
  //
  UINT8     DeviceDataWidth;            ///> Device Data Width (0: x4, 1: x8, 2: x16)
  UINT8     DeviceDensity;              ///> Device Density (0: 1Gb 1: 2Gb, 2: 4Gb, 3: 8Gb, 4: 16Gb, 5: 32Gb)
  UINT8     DimmSwizzle[MAX_DIMMS];     ///> Determine if the Dimm is Swizzle or Sequential
  UINT8     DimmMirrored[MAX_DIMMS];    ///> Determine if the Odd Rank is mirrored (1,3...). 0: Not mirrored, 1: Mirrored, 2: Reserved

  UINT32    RowMask;                    ///> Row Mask that indicates row bits
  UINT32    BankRowTrackerMask;         ///> Provides a mask for the merge of Bank groups, Bank and Rows

  UINT8     BitsPerColumns;             ///> Number of bits used to specify a column
  UINT8     BitsPerBank;                ///> Number of bits used to specify a bank
  UINT8     BitsPerBankGroup;           ///> Number of bits used to specify a bank group (only DDR4)
  UINT8     BitsPerRow;                 ///> Number of bits used to specify a row

  UINT8     BankOffset;                 ///> Provide the address offset to the first Bank bit
  UINT8     BankGroupOffset;            ///> Provide the address offset to the first Bank Group bit
  UINT8     RowOffset;                  ///> Provide the address offset to the first Row bit
  UINT8     RankOffset;                 ///> Provide the address offset to the first Rank bit

  UINT16    Burst;                      ///> Number of bursts used for writing (Ag, Vc0 and Vc1) and reading (Only Vc0 and Vc1)

  UINT16    Divider;                    ///> Coarse Delay adjust
  UINT16    DelayOffset;                ///> Fine Delay adjust

  BOOLEAN   SamsungDram[MAX_CHANNELS][MAX_DIMMS];   ///> Defines if the DIMM is using Samsung DRAM's
} PASS_GATE_PARAMETERS;
#endif

typedef struct {
  UINT8     SetupType;
  UINT8     CmdSequence;
  UINT8     LDOSequence;
  UINT32    Pattern0;
  UINT32    Pattern1;
  UINT32    Pattern2;
  UINT8     BusWidth;
  UINT8     MinRankBit;
  UINT8     MinBankBit;
  UINT32    NumBursts;
  UINT32    LoopCount;
  UINT8     ROW;
  UINT8     COL;
  UINT8     ChunkErrorMask;
  UINT8     Rank;
  BOOLEAN   DPAT_LFSR;
  BOOLEAN   CADB_Harrassment;
  UINT8     RmtRestoreJedecResetType;
  UINT16    RmtRestoreSignalGroupBegin;
  UINT8     RmtRestoreSignalCount;
  UINT8     RmtFakeErrorCount;
  UINT32    Pattern;  //will remove
  UINT32    VicPattern;  //will remove
} CPGC_OPTIONS;

typedef struct {
  UINT16    ChannelHashMask;
  UINT16    SliceHashMask;
  UINT8     InterleavedMode;
  UINT8     SlicesChannelsEnabled;//bit0=SliceEnabled;bit1=channelEnabled
  UINT8     ForceMin2xRefresh;
  UINT8     DualRankSupportEnabled;
  UINT8     Reserved[3]; // keep 4-byte alignment
  } MrcInput;

typedef struct {
  //
  // NvData
  // All MRC Data whihc is saved/restored between boots.
  //
  MRC_NV_DATA_FRAME         NvData;
  UINT8                     ChEnMask;
  UINT8                     PostCode;
  MMRC_STATUS               ErrorCode;
  BOOLEAN                   RestorePath;
  UINT8                     DigitalDll;
  UINT8                     MrcDebugMsgLevel;
  UINT8                     MrcRmtSupport;
  BOOLEAN                   MrcRmtMarginResultOld; // Set this flag if RmtCheckRun = 3 and L2 tests fail then RmtCheckRun is set to 1 internally and L1 tests are run. But to show a MarginCheckResultState = 3 , we need to remember that RmtCheckRun was 3 previously
  BOOLEAN                   MrcRmtCallL1Check;     // If L2 check fails we need to call L1 check but we dont want to mess with RmtChkRun user input so I have defined an internal variable
  UINT32                    MrcRmtEnableSignal;
  UINT8                     MrcRmtCpgcExpLoopCnt;
  UINT8                     MrcRmtCpgcNumBursts;
  UINT8                     MrcTrainingCpgcExpLoopCnt;
  UINT8                     MrcCpgcNumBursts;
  UINT32                    CachedValuesNonSR[MAX_CHANNELS][MAX_NUM_CACHE_ELEMENTS_NON_SR][MAX_RANKS][MAX_STROBES];
  UINT32                    CachedValuesNonSRPB[MAX_CHANNELS][MAX_NUM_CACHE_ELEMENTS_NON_SR_PB][MAX_RANKS][MAX_STROBES][MAX_BITS];
#if RMT_JUMP_POSTCODES
  INT16                     RmtJumpPostcode;
#endif
#if FAULTY_PART_TRACKING
  BOOLEAN                   FaultyPartTracking;
  BOOLEAN                   FaultyPartContinueOnCorrectable;
#endif
  UINT16                    SmbusBar;

  R2R_SWITCHING_PARAMETERS  Rk2RkEn;

#if SIM || JTAG
  FILE                      *LogFile;
#endif
  MRC_OEM_FRAME             OemMrcData;
  UINT8                     CpuStepping;
  UINT32                    EcBase;
  UINT32                    MchBar;
  UINT32                    P2sbBar;
  UINT16                    GpioBar;
  UINT32                    PmcBar;
  UINT32                    PlatformID;
  UINT8                     EnableScrambler;
  UINT8                     EnableAddressMirroring;
  UINT8                     NumberOfEnabledChannels[TypeDdrT];

  UINT8                     NorthPeakMemorySize;

#if MEM_SV_HOOKS
#if ME_SUPPORT_FLAG
  BOOLEAN                   MeSeg;
  UINT32                    MeUmaSize;
#endif
#if NORTHPEAK_SUPPORT
  BOOLEAN                   NpkEnable;
  UINT8                     NpkRegistersBar;
  UINT8                     NpkSwTraceBar;
  UINT8                     NpkFwTraceBar;
  UINT8                     NpkRtitBar;
  UINT16                    NpkHwTraceMemorySize;
  UINT8                     NpkHwTraceBufferWrapMode;
  UINT16                    NpkSwFwTraceMemorySize;
  UINT8                     NpkSwFwTraceBufferWrapMode;
#endif
  UINT8                     OdlaParalPrtEn;
  UINT32                    MemoryDepth;
  UINT8                     BramParityEnable;
  UINT8                     EccBytelaneSwap;
  UINT8                     InputRun125VDimmAt135V;
  UINT8                     InputRun135VDimmAt150V;
  UINT8                     Favor;
  UINT8                     InputCustomFrequency;
  UINT8                     InputCustomVoltage;
  UINT8                     OverrideCheckpoints;
#endif // MEM_SV_HOOKS
#if INTERLEAVING_SUPPORT
  UINT8                     InterleaveMode;
#endif
#if BXTPMRC
  UINT8                     TimingModeCache; // CommandClockTraining TimingMode, use to control setting of timing mode in GetSetTimingMode
  UINT8                     KeepAlive; // Used to control looping in CommandClockTraining for DDR3L when 1N timing fails
#endif

  UINT16                    SupportedCasLat;
  UINT16                    SignalType;
  BOOLEAN                   EnableParallelTraining;
  BOOLEAN                   ExecuteThisRoutineInParallel;
  BOOLEAN                   ForceRunOnDisabledChannels;

  CPGC_OPTIONS              CpgcOptions;

#if DDR4_SUPPORT == 1
  UINT8                     CommandAddressParity;
#endif
#if PASS_GATE_TEST == 1
  PASS_GATE_PARAMETERS      PassGateParameters;
#endif
  UINT8                     CurrentBlueprint;
  UINT8                     MrcTimingProfile;
  UINT8                     MrcMemConfig;
  UINT8                     DVFS_Enable;
  UINT8                     TimingParamsConfig;
  UINT32                    MR1_Value;
  UINT32                    MR2_Value;
  UINT32                    MR3_Value;
  UINT32                    TwoXps;
  UINT32                    SeCUmaBase;
  UINT32                    SeCUmaSize;

  // GROWING LIST OF INPUT PARAMS NEEDED FOR MMRC
  UINT32                    DynamicVars[MAX_CHANNELS][DYNVAR_MAX];
  UINT16                    GranularitySteps[MAX_BLUEPRINTS][MAX_DLL_COUNT][NUM_FREQ][MAX_NUM_DELAY_ELEMENTS];

  UINT16                    HalfClkPi;
  UINT8                     MaxElements;
  UINT8                     NumberOfTiming;
  UINT8                     MaxBits;

  FLOORPLAN                 MasterFloorPlan[MAX_BLUEPRINTS][MAX_CHANNELS][MAX_STROBES];
  UINT8                     ReadPerBitEnable;
  UINT8                     WritePerBitEnable;
  UINT16                    FreqHi;
  UINT16                    FreqLo;
  UINT8                     VocSweep;
  UINT16                    SABy2Clk;
#if defined(SIM) || defined(JTAG) || defined(CAR)
  POSTCODEDATA              Pcd;
  UINT8                     CommandlineRanks;
  UINT8                     CommandlineChannels;
#endif

  //CHANNEL_VOLATILE          Channel_V[MAX_CHANNELS];

  //
  // Add for NPK reserved mem.
  //
  UINT32                    NPKMSC0Size;
  UINT32                    NPKMSC0Base;
  UINT32                    NPKMSC1Size;
  UINT32                    NPKMSC1Base;
  MrcInput                  MrcInputs;
  UINT8                     StopPc;
  UINT8                     StartPc;
  UINT8                     CheckpointOverride;
  UINT8                     EVExitPc;
  UINT32                    SKPDTestReg;
  UINT32                    DEBUP0Reg;
  UINT32                    EnableVocCtle;
  UINT8                     DefaultCmdClkCtl;
  UINT16                    SafeCmdVal[MAX_CHANNELS][2];
  UINT16                    SafeCtlVal[MAX_CHANNELS][2];
  UINT8                   MaxNumberStrobes;
  UINT8                   MaxNumberChannels;
  UINT8                   BxtSeries;
  UINT8                   DynamicSrDisable; // Disables Dynamic SR when set
  UINT8                   MrcParamsValidFlag; //Used to indicate that the MRC params have been loaded from NV RAM
  UINT16                  BootMode; // Determines flow that will be used in MMRC.
  UINT8                   LctDisable; // Disables Late Command Training when set to 1
  UINT8                   TotalDimm;
  BOOLEAN                 LctNModeFlag;  //This is to check margins against gaurdband after phase 3 and switch to 2N with 1N margins are low
  //
  // PeriodicRetrainingDisable - Enable or disable LPDDR4 Periodic Retraining.
  // 0 = Enable Periodic Retraining
  // 1 = Disable Periodic Retraining
  //
  UINT8                   PeriodicRetrainingDisable;
  //
  // Lpddr4CaOdt
  // Specifies the LPDDR4 MR11 CA ODT value to utilize when initializing the LPDDR4 mode registers
  UINT8                   Lpddr4CaOdt;
} MMRC_DATA;

typedef MMRC_STATUS (*TestFunction) (MMRC_DATA *MrcData, UINT8 Channel, UINT8 Rank, UINT8 NumberElements, UINT8 NumberKnobs, UINT8 PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]);

typedef struct {
  INT16  Index;
  INT16  SharedIndex;
  INT16  PBIndex;
  UINT16 StartValue[2];
  INT16  PBStartValue;
  INT16  Ranges[2];
  UINT16 Step[3];
  UINT8  RelAbsFlag;
  UINT8  SharedFlags;
  INT8   PBSharedFlags;
  UINT8  Label[5];
  UINT8  NumberKnobs;
  UINT16 Phase2_VrefStart;
  UINT16 Phase2_VrefStop;
  UINT16 Phase2_VrefStep;
  INT8   CriteriaStep;
  INT16  EntryTask;
  UINT8  PBDirectFlag;
  UINT8  NumberOfTiming;
  UINT8  NumberSweeps;
  TestFunction TestFunctionPointer;
} CONTROLKNOBS;

#pragma pack()

#define NO_PRINT                0xFF

#define ANALOGDLL               0
#define DIGITALDLL              1

#define LPDDR3_WL_ENABLE        BIT19
#define DDR3_WL_ENABLE          BIT13

#define JEDEC_PRECHARGEALL      0x01
#define JEDEC_MRS               0x02
#define JEDEC_REFRESH           0x03

#define FORCEODT_LOW            0x00
#define FORCEODT_HIGH           0x01
#define FORCEODT_RELEASE        0x02

#define LDO_COEFFS              7
#define LDO_DATAPASS            1
#define LDO_CAPASS              0
#define LDO_BOTH                2

#define TASK_FUNCTION_DESC_DONE 0, 0, ((MMRC_STATUS(*)(MMRC_DATA *, UINT16, UINT16, UINT8)) 0), 0, 0, 0, 0, 0, 0, 0
#define DETREGLIST_NULL         ((DETAILED_REGISTER_STRING *) 0xFFFFFFFF)
#define REGLIST_NULL            ((REGISTER_STRING *)    0xFFFFFFFF)
#define PHYINIT_LIST_DONE       ((UINT8 *) 0xFFFFFFFF), REGLIST_NULL, DETREGLIST_NULL

typedef struct {
  UINT8   PostCode;
  UINT16  BootMode;
  MMRC_STATUS  (*Function) (MMRC_DATA *, UINT16, UINT16, UINT8);
  UINT16  CapsuleStartIndex;
  UINT8   StringIndex;
  UINT8   Channel;
  UINT16  PList;
  UINT16  FList;
  UINT16  CList;
  UINT16  TList;
} TASK_DESCRIPTOR;

typedef struct {
  UINT16  StrIndex;
  UINT32  FncOffset;
} SUBTASK_INDEX_OFFSET;

typedef struct {
  MMRC_STATUS  (*Function) (MMRC_DATA *MrcData, UINT8 Socket, UINT8 Channel, UINT8 Dimm, UINT8 Rank, UINT8 Strobe, UINT8 Bit, UINT8 FrequencyIndex, UINT8 IoLevel, UINT8 Cmd, UINT32 *Value);
} EXTERNAL_GETSET_FUNCTION;

/**
  When specifying a unique location, the elements required are
  offset/starting/ending bits.
**/
typedef struct {
  UINT16 Offset;              ///> Byte offset for the specific rank.
  UINT8 StartingBit;          ///> Starting bit position for the specific rank.
  UINT8 EndingBit;            ///> Ending bit position for the specific rank.
} REG_LOC;

typedef union {
  UINT8      Data;
  //
  // Remove bit field definitions from ASM include file due to MASM/H2INC limitations
  //
#ifndef ASM_INC
  struct {
    UINT8    GranularityValue   : 4;  // Granularity of the element. Used for clock crossing addition/subtraction when a CC directly modifies a delay register.
    UINT8    BitBased           : 1;  // This element has bit level granularity (not strobe level)
    UINT8    GranularityValid   : 1;  // When 1, the Granularity field is valid.
    UINT8    StrobeLookupTable  : 1;  // When 1, the Strobe Lookup Table is used to calculate ByteOffset and BoxType Offset.
    UINT8    RankLookupTable    : 1;  // When 1, the Rank Lookup Table is used to calculate ByteOffset and BoxType Offset.
  } Bits;
#endif
} ELEMENT_ATTRIBUTES;

/**
  Elements is the main structure for identifying the location for an individual register
  assignment with the Get/Set API.
**/
typedef struct {
  UINT8               Box_Type;            ///> Box type.
  REG_LOC             RegisterLocation[MAX_RANKS];
  UINT8               CacheIndex;          ///> Index into the cache array where this element is stored.
  ELEMENT_ATTRIBUTES  ElementAttributes;   ///> Various attributes about this element.
  INT16               SPMByteOffset;       ///> Strobe-to-Strobe Per Module Byte offset.
  INT8                SPMBitOffset;        ///> Strobe-to-Strobe Per Module Bit offset.
  INT16               NbSPMByteOffset;     ///> Nibble Strobe-to-Strobe Per Module Byte offset.
  INT8                NbSPMBitOffset;      ///> Nibble Strobe-to-Strobe Per Module Bit offset.
  INT16               BitSPMByteOffset;    ///> Bit Strobe-to-Strobe Per Module Byte offset.
  INT8                BitSPMBitOffset;     ///> Bit Strobe-to-Strobe Per Module Bit offset.
  UINT32              MaxLimit;            ///> Maximum absolute value allowed
  UINT32              WaitTime;            ///> Time in nanoseconds required for new setting to take effect
  UINT16              R2RSwitching;        ///> Flag fffe=Disable, ffff=Enable, All others are indexes of getset to check if enabled.
} GET_SET_ELEMENTS;

typedef struct {
  UINT16     DelayOffset;
} INDEX_TO_OFFSET_LAYER;

/**
  Information about the signals. Things like number of signal groups, name of signal, etc.
**/
typedef struct {
  UINT8               NumSignalGroups;     ///> Number of signal groups for this signal type.
  UINT8               FrequencyOffset;     ///> Offset to the same element for the next frequency in a system which support multiple runtime freqs.
  UINT8               FrequencyIndex;      ///> The frequency index of this signal type.
  UINT16              VrefIndex;           ///> The index of the vref associated with this delay.
  UINT16              Minus1Index;         ///> The index of the minus 1 select associated with this delay
  char               *Name;                ///> Name of the signal
} SIGNAL_INFO;

typedef struct {
  UINT16        Signal;
  BOOLEAN       SkipInitialProgramming;
  UINT8         CpgcConfig;
  UINT8         StepSize;
  UINT8         RankChoices;
  UINT8         JedecResetType;
} RMT_SIGNAL;

/**
  ClockCrossing structure provides when the specific clock crossing will change
  based on the linear PI values.  The range is specified in (%) of the PI range
  since the PI range is dynamic based on analog/digitial dll.
**/
typedef struct {
  UINT8       CcNumber;         ///> Clock crossing index.
  UINT16      MinPercent;       ///> Minimum PI value for InValue to be programmed.
  UINT16      MaxPercent;       ///> Maximum PI value for InValue to be programmed.
  INT16       InValue;          ///> In Range Value.
  BOOLEAN     IsAbsolute;       ///> If TRUE, program InValue directly to register. If FALSE,
                                ///> add InValue to current value of register.
} CLOCK_CROSSINGS;


/**
  Structure for the PHY init registers. This contains
  information on how to find registers for a given bytelane,
  module, rank, etc. Also indicates whether we should write a
  register to all enabled channels or just to a specified
  channel.
**/
typedef struct {
  UINT8  RankLoop         :1;   ///> Loop on rank.
  UINT8  StrobeLoop       :1;   ///> Loop on strobe.
  UINT8  Offset16         :1;   ///> When set, the offset field is 2-bytes long, otherwise 1-byte long.
  UINT8  CondExec         :1;   ///> When set, Conditional Execution.
  UINT8  MaskPresent      :1;   ///> When set, mask is present in the Assignment.
  UINT8  NeedSecondByte   :1;   ///> When set, REGISTER_LOCATION_ATTRIBUTES2 is present.
  UINT8  UseBB2           :1;   ///> If the boxtype is above 15 or the value being written is greater than 32 bits
                                ///> then we need to use the BB2 macro to allow 8 bits for boxtype and 8 for byte enables.
  UINT8  Done             :1;   ///> When set, this is the final entry for this task.
} REGISTER_LOCATION_ATTRIBUTES;

/**
  Optional Byte 2 of register attributes.
  Only used if we need bits in this byte set to 1.
**/
typedef struct {
  UINT8  StringPresent    :1;   ///> There is a string that needs to be output associated with this function entry.
  UINT8  InstanceDependent:1;   ///> When set, only specific instance will be written to.
  UINT8  ChannelDependent :1;   ///> When set, only specific channel will be written.
  UINT8  Reserved         :5;   ///> Reserved
} REGISTER_LOCATION_ATTRIBUTES2;


typedef struct {
  INT8    bitoffset;
  INT16   byteoffset;
} LP_TABLE;

/**
  Layout for the Register Display Strings
**/
typedef struct {
  UINT8 string[16]; ///> Maximum of 16 characters for the name of the register.
  UINT8 start;      ///> Index into the detailed register (fields Names) where the first field is listed.
  UINT8 number;     ///> Number of fields in the detailed register array.
} REGISTER_STRING;

/**
  Layout for the Detailed Register Display Strings (Fields).
**/
typedef struct {
  UINT8 string[13]; ///> Maximum of 13 character for the register field name.
  UINT8 endBit;     ///> End bit for the field.
  UINT8 startBit;   ///> Starting bit for the field.
} DETAILED_REGISTER_STRING;

typedef struct {
  CONST UINT8                       *regAssign;
  CONST REGISTER_STRING             *regString;
  CONST DETAILED_REGISTER_STRING    *detRegString;
} PHYINIT_LIST;

typedef struct {
  UINT16   halfClk;
  UINT16   minVal1;
  UINT16   maxVal1;
  UINT16   minVal2;
  UINT16   maxVal2;
} DIGITAL_DLL_LIST;

/**
  Used to pass data between MRC and MMRC. Not used at the moment
  so setting the variable to Reserved.
**/
typedef struct {
  BOOLEAN Enable;       ///> Set to TRUE if you want the feature enabled. FALSE if you want it disabled.
  UINT8   Channel;      ///> Optional channel on which to operate. If not populated, all channels will be handled.
  BOOLEAN ChannelValid; ///> Set to TRUE if Channel contains a particular channel or channels on which to operate.
  MMRC_STATUS  (*Function) (MMRC_DATA *, UINT8);
} HANDSHAKE_PARAMETERS;

#ifndef MRC_DATA_REQUIRED_FROM_OUTSIDE
typedef MMRC_STATUS (*MRC_TASK_FUNCTION) (MMRC_DATA  *MrcData);
#endif
//
// Prototypes for each Power Nob function
//
#if POWER_TRAINING == 1
typedef MMRC_STATUS (*HOOK_SET_VALUE) (MMRC_DATA *MrcData, UINT8 Channel, UINT16 Index);
typedef MMRC_STATUS (*TRAINING_STEP)  (MMRC_DATA *MrcData, UINT16 CapsuleStartIndex, UINT16 StringIndex, UINT8 Channel);
#endif

typedef enum {
  UNIQUE       = BIT0,
  STROBE_BASED = BIT1,
  RANK_BASED   = BIT2,
  BIT_BASED    = BIT3
} ACCESS_TYPE;

typedef struct {
  GSM_GT        TypeIndex;
  ACCESS_TYPE   AccessType;
  int           RestoreOnS3Resume;
} RESTORE_DATA_DESCRIPTOR;

typedef enum {
  SAVE = 0,
  RESTORE = 1,
} SAVE_RESTORE_TASK;

typedef struct {
  UINT16 Saby2Freq;
  UINT16 SpidFreq;
  UINT16 init_saby2_to_spid_tsvs;
  UINT16 pattern_saby2_to_spid_tsvs;
  UINT16 init_spid_to_saby2_tsvs;
  UINT16 pattern_spid_to_saby2_tsvs;
} TSV_DATA;

//
// SMIP_ODT_OPTION.Bits.CaOdt legal values
//
typedef enum {
  SMIP_CA_ODT_AB_HIGH_LOW = 0,
  SMIP_CA_ODT_AB_HIGH_HIGH = 1
} SMIP_CA_ODT_OPTION_VALUES;

typedef union {
  struct {
    //
    // Bits[0:0] : RX ODT
    // DDR3L and LPDDR3 only:
    //  Change the READ ODT strength, for SOC termination during a READ transaction, ON DQ BITs.
    //    STRONG == > 60 OHMS roughly
    //    WEAK == > 120 OHMS or so roughly
    //  Purpose :
    //   Save power on these technologies which burn power directly proportional to ODT strength,
    //   because ODT looks like a PU and PD(e.g.a resistor divider, which always burns power when ODT is ON).
    //   0 = WEAK_ODT_CONFIG
    //   1 = STRONG_ODT_CONFIG
    // LPDDR4:
    //   X = Don't Care
    //
    UINT32 RxOdt : 1;
    //
    // Bits[1:1] : CA ODT
    // LPDDR4 ONLY:
    //    The customer needs to choose this based on their actual board strapping(how they tie the DRAM's ODT PINs).
    //    Effect:
    //     LPDDR4 MR11 will be set based on this setting.
    //     CAODT_A_B_HIGH_LOW == > MR11 = 0x34, which is CA ODT = 80 ohms.
    //     CAODT_A_B_HIGH_HIGH == > MR11 = 0x24, which is CA ODT = 120 ohms(results in 60 ohm final
    //     effective impedance on CA / CLK / CS signals)
    //    Purpose:
    //     To improve signal integrity and provide a much more optimized CA VREF value during training.Not to save power.
    //   0 = ODT_AB_HIGH_LOW(default) = SMIP_CA_ODT_AB_HIGH_LOW
    //   1 = ODT_AB_HIGH_HIGH = SMIP_CA_ODT_AB_HIGH_HIGH
    // DDR3L & LPDDR3:
    //   X = Don't Care
    //
    //
    UINT32 CaOdt : 1;
    //
    // Bits[3:2] : Reserved
    //
    UINT32 Reserved0 : 2;
    //
    // Bits[4:4] : TX ODT
    // DDR3L only:
    //   0 = RZQ/4 (60 Ohms)  = MRC_SMIP_DDR3L_TX_ODT_RTT_WR_60_OHMS
    //   1 = RZQ/2 (120 Ohms) = MRC_SMIP_DDR3L_TX_ODT_RTT_WR_120_OHMS
    // LPDDR3 & LPDDR4:
    //   X = Don't Care
    //
    UINT32 TxOdt : 1;
    //
    // Bits[31:5] : Reserved
    //
    UINT32 Reserved1 : 27;
  } Bits;
  UINT32 Data;
} SMIP_ODT_OPTION;


typedef union {
  struct {
    //
    // Bits[0:0] : DDR3L Auto Self Refresh
    // 0: Not Supported
    // 1: Supported
    //
    UINT32 ASR_Supported : 1;
    //
    // Bits[1:1] : Maximum Operating Temperature
    // 0: 85 degree Celsius Max
    // 1: 95 degree Celsius Max
    //
    UINT32 MaxOperatingTemp : 1;
    //
    // Bits[31:5] : Reserved
    //
    UINT32 Reserved : 30;
  } Bits;
  UINT32 Data;
} SMIP_DDR3LASR_OPTION;

typedef union {
  struct {
    //
    // Bits[0:0] : Scrambler Support
    // Enable or disable the memory scrambler:
    //   0 = Disable
    //   1 = Enable
    //
    UINT32 ScramblerSupport : 1;
    //
    // Bits[1:1] : Periodic Retraining Disable
    // This option allows customers to disable LPDDR4 Periodic Retraining
    // for debug purposes. Periodic Retraining should be enabled in production.
    // This field has no effect for DDR3L and LPDDR3 memory type configurations.
    //   0 = Enable
    //   1 = Disable
    //
    UINT32 PeriodicRetrainingDisable : 1;
    //
    // Bits[31:2] : Reserved
    //
    UINT32 Reserved1 : 30;
  } Bits;
  UINT32 Data;
} SMIP_SCRAMBLERSUPPORT_OPTION;

//
// DDR3L_REFRESH_RATE_REG_FIELD_VALUES
// Do not modify this enum.
// These entries must match the PUNIT register definition for
// P_CR_DDR_THERM_THRT_CTRL_0_0_0_MCHBAR.DDR3L_REFRESH_RATE
//
typedef enum {
  RefreshRate1x = 0,
  RefreshRate2x,
  MaxRefreshRate
} DDR3L_REFRESH_RATE_REG_FIELD_VALUES;

#pragma pack()
#pragma pack(pop)
#endif
