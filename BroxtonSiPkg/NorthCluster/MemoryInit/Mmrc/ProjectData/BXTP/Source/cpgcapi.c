/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
#ifndef USE_CPGC_LEGACY_FUNCTIONS
#include "cpgcapi.h"

#define BA0 0
#define BA2 0
//Long new
typedef union {
  UINT32      Data;
  struct {
    UINT32    cs              : 2;   // 0-1
    UINT32    reserved1       : 6;   // 2-7
    UINT32    cmd             : 3;   // 8-10
    UINT32    reserved2       : 5;   // 11-15
    UINT32    odt             : 2;   // 16-17
    UINT32    reserved3       : 6;   // 18-23
    UINT32    cke             : 2;   // 24-25
  } Bits;
} CPGC_cadb_bufB;


enum {
  CA0, CA1, CA2, CA3, CA4, CA5, CA6, CA7, CA8, CA9
};
enum {
  RAS, CAS, WE, MA0, MA1, MA2, MA3, MA4, MA5, MA6, MA7, MA8, MA9, MA10, MA11, MA12, MA13, MA14, MA15
};

#define __DELIMITER 255

typedef union {
  UINT32      Data;
  struct {
    UINT32    row_col_addr    : 20;
    UINT32    reserved1       : 4;
    UINT32    bank_addr       : 3;
    UINT32    reserved2       : 5;
  } Bits;
} CPGC_cadb_bufA;


  // LPDDR3 victim group assignments.  Put TWO _DELIMITER's at end of all groups, and one delimiter between each group
  //  Currently this is common across channels, but may have to be 'per channel' depending on board layout
  CONST UINT8 CADB_CA_GROUPS[] = { CA0, CA5, CA8, __DELIMITER,     CA1, CA6, CA9, __DELIMITER,     CA2, CA3, CA4, CA7, __DELIMITER,    __DELIMITER };

  // CONST UINT8 CADB_CA_GROUPS[] = { CA0, CA2, CA4, CA6, CA8 ,__DELIMITER,   __DELIMITER };  // <-- this is equivalent as we had on VLV -- one group, every other bit is a victim

  // DDR3L victim group assignments
  CONST UINT8 CADB_MA_GROUPS[] = { MA0, MA3, MA6, MA9, MA12, MA15, RAS, __DELIMITER,
                             MA1, MA4, MA7, MA10,MA13, CAS,       __DELIMITER,
                             MA2, MA5, MA8, MA11,MA14, WE,        __DELIMITER,
                             __DELIMITER};





MMRC_STATUS

L_CpgcSetup (
    IN OUT  MMRC_DATA  *MrcData,
    IN      UINT8      Channel,
    IN      UINT8      SetupType
)
/*
  Routine Description:
    Unified routine to do initial setup on all cpgc routines
  Arguments:
    MrcData:  Will contain cpgc options passed through MrcData->CpgcOptions
    SetupType: Chooses which type of CPGC setup to perform.  Typically one per algo.
*/
{
#if CPGC_VERSION == 100
#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount = 0;
  return SUCCESS;
#endif
  //
  // One time initialization - this stuff really only needs to be initialized once, but i'm doing it every time CpgcSetup is called (should be moved to a one time routine eventually)
  //
  // FODO: when integrated with trunk, need to consider multi-channel support
  //
  if (MrcData->MaxDq <= 4) {
    MrcData->CpgcOptions.BusWidth = 0;
  } else  {
    MrcData->CpgcOptions.BusWidth = 1;
  }
  //
  // The following is ANN/CHV specific.  Works regardless if LPDDR or DDR3L.  This is the rank and bank bit position in the D-unit physical addr map.
  //   ideally you would get this somehow from spreadsheet, but the trouble is that it's run time determined as well due to "BusWidth"
  //
  MrcData->CpgcOptions.MinRankBit = RANK_SHIFT_BIT + MrcData->CpgcOptions.BusWidth; // The lowest rank bit in the DUNIT address map, used to target the correct rank.
  MrcData->CpgcOptions.MinBankBit = MIN_BANK_BIT + MrcData->CpgcOptions.BusWidth;   // The lowest bank bit in DUNIT address map, for cmd stress
  //
  // End of one time initialization
  //
  //
  // SetupType will be retained as state, so you don't have to pass into CpgcReconfig, and CpgcRunTest
  //
  MrcData->CpgcOptions.SetupType = SetupType;

  MmrcExecuteTask (MrcData, CPGC_ENABLE, NO_PRINT, Channel);
  MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 3;//MrcData->CpgcOptions.NumBursts - 3;
  MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;//MrcData->DynamicVars[Channel][LANE_ROTATE_RATE] + 1;      // Always LANE_ROTATE_RATE+1
  MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;   // 8 per rank tested   ,  should never get to 8th victim
  MrcData->DynamicVars[Channel][NUM_BURSTS]             = MrcData->CpgcOptions.NumBursts;
  MrcData->DynamicVars[Channel][LOOP_COUNT]             = MrcData->CpgcOptions.LoopCount;

  switch (SetupType)  {
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  case  CPGC_CONFIG_MEMINIT:
    //
    // Inputs: None
    //
    // TOTAL_CACHE_LINES is the # of total cachelines available in memory, for this channel
    // This needs to be setup by some external routine.   On trunk code it is set in ConfigMem.c.   For sim I will pass in from sus-wrapper
    // It has to be adjusted by BusWidth to account for 32B vs 64B request
    //
    MrcData->DynamicVars[Channel][TOTAL_COMMANDS] = ((UINT32) ((MrcData->DynamicVars[Channel][TOTAL_CACHE_LINES]) << (1 - MrcData->CpgcOptions.BusWidth))) - 1;
    //
    // Takes input of TOTAL_CACHELINES, which means in all of avialalbe memory
    //
    MmrcExecuteTask (MrcData, CPGC_MEMTEST_SETUP, NO_PRINT, Channel);
    break;  // end case CPGC_CONFIG_MEMINIT
#endif
  case CPGC_CONFIG_VA:
    //
    // Already setup above (numbursts, loopcount, etc)
    //
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;
  case CPGC_CONFIG_EARLYTRAINING:
    //
    // Inputs: LoopCount, NumBursts
    //
    MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;
    MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;
    MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 0;
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;
#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  case CPGC_CONFIG_TURNAROUNDS:
    //
    // Already setup above (numbursts, loopcount, etc)
    //
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;
#endif
#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  case CPGC_CONFIG_LFSR_ADDRESS:
    MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;      // Disable all the lane rotations stuff, since we sweep numbursts anyways
    MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;
    MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 0;
    MrcData->DynamicVars[Channel][NUM_BURSTS]             = 1;
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_LFSR_ADDRESS, NO_PRINT, Channel);
    break;
#endif
  case CPGC_CONFIG_CMD_STRESS:
    //
    // Inputs: LoopCount, NumBursts
    // Setup the basic VA mode.
    //
    //
    // VAR_SEG_HIGHADR_HI/LO sets the LFSR addressing for Subseq 2 & 3 to target high address bits, such as rows
    //
    MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_HI] = (MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1 + 3;
#if LPDDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TYPE_LPDDR3) {
      if ((MrcData->CpgcOptions.MinRankBit - 5 - MrcData->CpgcOptions.BusWidth) < ((MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1)) {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinBankBit) - 5 - MrcData->CpgcOptions.BusWidth;
      } else {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1;  // set low 3 bits of var segment to just be adjacent to hi part of var segment
      }
    }
#endif
#if DDR3_SUPPORT
    if  (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TYPE_DDR3) {
      //
      // Make sure min bank bit is low enough , otherwise hi/lo segment will alias
      //
      if (((MrcData->CpgcOptions.MinBankBit + 1 - MrcData->CpgcOptions.BusWidth) - 5 - MrcData->CpgcOptions.BusWidth) < ((MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1)) {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinBankBit + 1 - MrcData->CpgcOptions.BusWidth ) - 5 - MrcData->CpgcOptions.BusWidth;      // set low 3 bits of var segment to point at banks
      } else {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1;  // set low 3 bits of var segment to just be adjacent to hi part of var segment
      }
    }
#endif
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT] = 0; // initially target rank=0, but this should be overridden by CpgcReconfig
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_CADB1, NO_PRINT, Channel);
    break;
  default:
    MrcDeadLoop ();
  } // main case statement (SetupType)
#elif CPGC_VERSION == 200
//
  // One time initialization - this stuff really only needs to be initialized once, but i'm doing it every time CpgcSetup is called (should be moved to a one time routine eventually)
  //
  // FODO: when integrated with trunk, need to consider multi-channel support
  //
  //if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank] <= 4) {
  //  MrcData->CpgcOptions.BusWidth = 0;
  //} else  {
  //  MrcData->CpgcOptions.BusWidth = 1;
  //}

  //
  //
  // SetupType will be retained as state, so you don't have to pass into CpgcReconfig, and CpgcRunTest
  //
  MrcData->CpgcOptions.SetupType = SetupType;

  MmrcExecuteTask (MrcData, CPGC_ENABLE, NO_PRINT, Channel);  //:might not be needed
  MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 3;//MrcData->CpgcOptions.NumBursts - 3;
  MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;//MrcData->DynamicVars[Channel][LANE_ROTATE_RATE] + 1;      // Always LANE_ROTATE_RATE+1
  MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;   // 8 per rank tested   ,  should never get to 8th victim
  if (MrcData->CpgcOptions.NumBursts != 0) {
    MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = (UINT32) ((1 << MrcData->CpgcOptions.NumBursts) - 1);
  } else {
    MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = 0;
  }
  if (MrcData->CpgcOptions.LoopCount != 0) {
    MrcData->DynamicVars[Channel][NUM_LOOPS_EXP]             = (UINT32) ((1 << MrcData->CpgcOptions.LoopCount) - 1);
  } else {
     MrcData->DynamicVars[Channel][NUM_LOOPS_EXP]             = 0;
  }
  switch (SetupType)  {
  case CPGC_MEMORY_INIT:
    MmrcExecuteTask (MrcData, CPGC_LDO_DPAT_LFSR, NO_PRINT, Channel); //sets up all DQ to follow unisequencer0

    //setup ROW and COL
    MrcData->DynamicVars[Channel][NUM_ROWS] = MrcData->CpgcOptions.ROW;
    MrcData->DynamicVars[Channel][NUM_COLS] = MrcData->CpgcOptions.COL - 3;
    MmrcExecuteTask (MrcData, MEM_INIT, NO_PRINT, Channel);
    //reprogram the loops
    MrcData->DynamicVars[Channel][NUM_LOOPS_EXP]             = 0;
    MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = (1<<(MrcData->CpgcOptions.ROW + MrcData->CpgcOptions.COL-3)) - 1 ;// 0xffffffff;
    break;

  case CPGC_CONFIG_EARLY_CMD_TRAINING:
    MmrcExecuteTask (MrcData, ENTER_CA_TRAINING, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, ENTER_CA_TRAINING_CCC, NO_PRINT, Channel & 0xFFFE);
    MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = 0;
    MrcData->DynamicVars[Channel][NUM_LOOPS_EXP]             = 0;
    break;

  case CPGC_CONFIG_EARLY_MPR_TRAINING:
  case CPGC_CONFIG_EARLYTRAINING:
    //
    // Inputs: LoopCount, NumBursts
    //
    MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;
    MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;
    MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 0;
    if (MrcData->CpgcOptions.SetupType == CPGC_CONFIG_EARLYTRAINING) {
      MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    }
    else {
      MmrcExecuteTask (MrcData, CPGC_LDO_DPAT_LFSR, NO_PRINT, Channel); //sets up all DQ to follow unisequencer0
      MmrcExecuteTask (MrcData, CPGC_SETUP_ERT, NO_PRINT, Channel);  //Puts CPGC into direct mode for early MPR
    }
    break;

  case CPGC_CONFIG_VA:
    //
    // Already setup above (numbursts, loopcount, etc)
    //
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;


  case CPGC_CONFIG_TURNAROUNDS:
    //
    // WIP: Will revist for E-0 after they fix In order mode b2b write traffic
    //

    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;

  case CPGC_CONFIG_MEMINIT:
    //
    // Memtest not yet implemented in SS. CPGC has bug with STOP ON ERROR
    // Place Holder: Will revisit later
    //
    break;

  case CPGC_CONFIG_LFSR_ADDRESS:
    // Placeholder, will revisit later

    break;

  case CPGC_LDO:
    switch (MrcData->CpgcOptions.LDOSequence)  {

    case DQ_Setup:
      MmrcExecuteTask (MrcData, CPGC_LDO_DPAT, NO_PRINT, Channel);
      break;

    case CADB_Setup:
      MmrcExecuteTask (MrcData, CPGC_LDO_CADB, NO_PRINT, Channel);

      break;
    case CADB_LFSR:
      MmrcExecuteTask (MrcData, CPGC_LDO_CADB_LFSR, NO_PRINT, Channel);

      break;
    case DQ_LFSR:
      MmrcExecuteTask (MrcData, CPGC_LDO_DPAT_LFSR, NO_PRINT, Channel);
      break;
    }

    break;

  case CPGC_CONFIG_CMD_STRESS:
    //
    //
    //
    //

    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    MrcData->DynamicVars[Channel][NUM_ROWS]       = 14;
    MrcData->DynamicVars[Channel][NUM_COLS]       = 5;
    MrcData->DynamicVars[Channel][ADD_ROTATE_RPT] = 3 + (MrcData->DynamicVars[Channel][NUM_ROWS]  + MrcData->DynamicVars[Channel][NUM_COLS]) - 1 ;
    MmrcExecuteTask (MrcData, CPGC_CADB1, NO_PRINT, Channel);
    if (MrcData->CpgcOptions.CADB_Harrassment) {
      MmrcExecuteTask (MrcData, CPGC_DESELECT_EN, 0, Channel);
    }
    else {
      MmrcExecuteTask (MrcData, CPGC_DESELECT_DIS, 0, Channel);
    }
    break;



  default:
    MrcDeadLoop ();
  } // main case statement (SetupType)
#endif // CPGC_VERSION == 100
  if (( SetupType != CPGC_CONFIG_MEMINIT ) && ( SetupType != CPGC_CONFIG_CMD_STRESS )) {
    MmrcExecuteTask (MrcData, CPGC_LOOPS, NO_PRINT, Channel);
  }
  else {
    MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = 7; //Hard code 7 for CMD stress
    MmrcExecuteTask (MrcData, CPGC_LOOPS, NO_PRINT, Channel);
  }
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    MmrcExecuteTask (MrcData, CPGC_ECC_EN, NO_PRINT, Channel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
L_CpgcReconfig (
  IN OUT  MMRC_DATA  *MrcData,
  IN      UINT8      Channel

)
/* Routine Description:
 *  Similar description as CpgcSetup.  This is called after CpgcSetup for
 *    a particular SetupType, to reconfigure it to a new rank, loopcount,
 *    or algo specific change during the algorithm.
 *  All these kind of options are passed through MrcData->CpgcOptions
 *  WARNING: A call to CpgcSetup is required before using CpgcReconfig !!
 *    This is true for each algo (need to call CpgcSetup first inside each algo)
 */
{

#if CPGC_VERSION == 100

  BOOLEAN   ExecuteMode;

  ExecuteMode = MrcData->ExecuteThisRoutineInParallel;
  MrcData->ExecuteThisRoutineInParallel = FALSE;

#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount =0;
  return SUCCESS;
#endif

  switch (MrcData->CpgcOptions.SetupType)  {
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  case CPGC_CONFIG_MEMINIT:
    MmrcExecuteTask (MrcData, CPGC_MEMINIT, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_MEMINIT_AND_TEST, NO_PRINT, Channel);
    break;
  case CPGC_CONFIG_MEMTEST:
    MmrcExecuteTask (MrcData, CPGC_MEMTEST, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_MEMINIT_AND_TEST, NO_PRINT, Channel);
    break;
#endif
  case CPGC_CONFIG_VA:
    //
    // Inputs: Rank
    //
    MrcData->DynamicVars[Channel][SEQ_START]  = 0;
    MrcData->DynamicVars[Channel][SEQ_END]    = 0;
   //kirk  MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0]  = MrcData->CpgcOptions.Rank << (GetAddressBit(MrcData, Channel, MrcData->CpgcOptions.Rank) - (5 + MrcData->CpgcOptions.BusWidth));
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1]  = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_2]  = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_3]  = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;
#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  case CPGC_CONFIG_TURNAROUNDS:
    //
    // Inputs: Rank ... has special values of ALL_COMBOS, DR_COMBOS, DD_COMBOS, SR_COMBOS
    // We actually don't do anything here with Rank though
    //
    break;
#endif
  case CPGC_CONFIG_EARLYTRAINING:
  case CPGC_CONFIG_EARLY_MPR_TRAINING:

    // kirk MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0] = MrcData->CpgcOptions.Rank << (GetAddressBit(MrcData, Channel, MrcData->CpgcOptions.Rank) - (5 + MrcData->CpgcOptions.BusWidth));
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    switch (MrcData->CpgcOptions.CmdSequence) {
    case CPGC_CMDSEQ_READ:
      MrcData->DynamicVars[Channel][SEQ_START]  = 1;
      MrcData->DynamicVars[Channel][SEQ_END]    = 1;
      break;
    case CPGC_CMDSEQ_WRITE:
      MrcData->DynamicVars[Channel][SEQ_START]  = 0;
      MrcData->DynamicVars[Channel][SEQ_END]    = 0;
      break;
    case CPGC_CMDSEQ_WRITE_READ:
      MrcData->DynamicVars[Channel][SEQ_START]  = 0;
      MrcData->DynamicVars[Channel][SEQ_END]    = 1;
      break;
    }
    MrcData->DynamicVars[Channel][AGG_PATTERN] = MrcData->CpgcOptions.Pattern;
    MrcData->DynamicVars[Channel][VIC_PATTERN] = MrcData->CpgcOptions.VicPattern;
    MmrcExecuteTask (MrcData, CPGC_CWL_PATTERN, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_SETUP_LVL, NO_PRINT, Channel);
    if (MrcData->CpgcOptions.SetupType == CPGC_CONFIG_EARLYTRAINING) {
      break;
    }
    //
    // Setup the CPGC as Buffer Mode for Uniseq0/1
    //
    MmrcExecuteTask (MrcData, CPGC_MPR_TRAINING, NO_PRINT, Channel);
    break;

#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  case CPGC_CONFIG_LFSR_ADDRESS:
    break;
#endif
  case CPGC_CONFIG_CMD_STRESS:
    //
    // Inputs: Rank
    //
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0] = MrcData->CpgcOptions.Rank << ((MrcData->CpgcOptions.MinRankBit) - 5 - MrcData->CpgcOptions.BusWidth);
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_2] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_3] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;
  default:
    MrcDeadLoop ();
  } // Main case statement (SetupType)

  MrcData->ExecuteThisRoutineInParallel = ExecuteMode;



#elif CPGC_VERSION == 200



#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount =0;
  return SUCCESS;
#endif

//:Long - moved so all options can adjust traffic data
  switch (MrcData->CpgcOptions.CmdSequence) {
  case CPGC_CMDSEQ_READ:
    MrcData->DynamicVars[Channel][INSTR_LAST] = 1;
    MrcData->DynamicVars[Channel][INSTR_POINTER] = 1;
    break;
  case CPGC_CMDSEQ_WRITE:
    MrcData->DynamicVars[Channel][INSTR_LAST] = 1;
    MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;
    break;
  default:  //defaults to write follow by read
    MrcData->DynamicVars[Channel][INSTR_LAST] = 0;
    MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;

  }

  MmrcExecuteTask (MrcData, CPGC_RESETUP_TRAFFIC, NO_PRINT, Channel);


switch (MrcData->CpgcOptions.SetupType)  {

  case CPGC_MEMORY_INIT:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);

    MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
    MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
    MrcData->DynamicVars[Channel][UNIMODE] = 1;

    MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);
  break;

  case CPGC_CONFIG_EARLY_MPR_TRAINING:

  case CPGC_CONFIG_EARLYTRAINING:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
    MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
    if (MrcData->CpgcOptions.DPAT_LFSR){
      MrcData->DynamicVars[Channel][UNIMODE] = 2;
    }
    else{
      MrcData->DynamicVars[Channel][UNIMODE] = 1;
    }

    MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);

  //switch (MrcData->CpgcOptions.CmdSequence) {
  //case CPGC_CMDSEQ_READ:
  //  MrcData->DynamicVars[Channel][INSTR_LAST] = 1;
  //  MrcData->DynamicVars[Channel][INSTR_POINTER] = 1;
  //  break;
  //case CPGC_CMDSEQ_WRITE:
  //  MrcData->DynamicVars[Channel][INSTR_LAST] = 1;
  //  MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;
  //  break;
  //case CPGC_CMDSEQ_WRITE_READ:
  //  MrcData->DynamicVars[Channel][INSTR_LAST] = 0;
  //  MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;
  //  break;
  //default:  //defaults to write follow by read
  //  MrcData->DynamicVars[Channel][INSTR_LAST] = 0;
  //  MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;

  //}

  //MmrcExecuteTask (MrcData, CPGC_RESETUP_TRAFFIC, NO_PRINT, Channel);
  break;

  case CPGC_CONFIG_LFSR_ADDRESS:
    //placeholder
    break;

  case CPGC_CONFIG_EARLY_CMD_TRAINING:
  case CPGC_CONFIG_VA:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;


  case CPGC_CONFIG_CMD_STRESS:
    //
    // Inputs: Rank
    //

    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;

  case CPGC_LDO:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    switch (MrcData->CpgcOptions.LDOSequence)  {

    case DQ_Setup:
      //MmrcExecuteTask (MrcData, CPGC_LDO_DPAT, 0, Channel);
      MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
      MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
      MrcData->DynamicVars[Channel][UNIMODE] = 1;
      MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);

      break;

    case CADB_Setup:
      MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
      MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
      MrcData->DynamicVars[Channel][UNIMODE] = 1;
      MmrcExecuteTask (MrcData,CPGC_CADB_MODE, NO_PRINT, Channel);


      break;
    case CADB_LFSR:
      MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
      MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
      MrcData->DynamicVars[Channel][PATTERN2] = MrcData->CpgcOptions.Pattern2;
      MrcData->DynamicVars[Channel][UNIMODE] = 2;
      MmrcExecuteTask (MrcData, CPGC_CADB_MODE, NO_PRINT, Channel);

      break;
    case DQ_LFSR:

      MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
      MrcData->DynamicVars[Channel][UNIMODE] = 2;
      MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);
      break;

    }

    break;

  case CPGC_CONFIG_MEMINIT:
    //placeholder

    break;
  #endif

  default:
    MrcDeadLoop ();
  } // Main case statement (SetupType)
  return MMRC_SUCCESS;
}




MMRC_STATUS
  L_CpgcRunTest (
  IN OUT  MMRC_DATA  *MrcData,
  IN      UINT8      Channel,
  OUT     UINT16     *BytelaneStatus,
  OUT     UINT32     *LANE_ERR_LO,
  OUT     UINT32     *LANE_ERR_HI,
  OUT     UINT32     *ECC_LANE_ERR,
  OUT     UINT32     *ErrorCount
  )
/* Routine Description:
 *  Similar description as CpgcSetup.  This runs the test and returns
 *  data  through ByteLaneStatus, LaneStatus, and ErrorCount variables.
 *  LaneStatus shall be an array or UINT32, large enough to hold a 1-bit
 *  error status for every DQ lane, LSB first.
 *  WARNING: CpgcSetup must be called *BEFORE* CpgcRunTest, for each algo.
 *  Return Values:
 *     BytelaneStatus, LaneStatus, ErrorCount, will return the usual cpgc error statuses.
 *        LaneStatus needs to be an array of 32-bit wide ints, with enough size to contain
 *        error status for all lanes.  E.g. length of 3 needed to hold x72 width bus.
 *        You can pass in NULL for these individually for error status you are not interested in.
 *        FODO: Support for 'KNOBS'.
 */
{
  UINT8  FlagRunNormalTest = TRUE;  // Used in case statement for SetupType
  UINT8 i;


#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  UINT8 rankA, rankB;
#endif

  //
  // If overridden to FALSE, it is assumed the case statement ran the CPGC test
  // If TRUE, then the normal CPGC_RUNTEST task will be run after the case statement.

#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount++;
  if (! (MrcData->CpgcOptions.RmtFakeErrorCount % 8)) {
    *BytelaneStatus = (UINT16) 0x1ff;
  }
  else {
    *BytelaneStatus = (UINT16) 0x0;
  }
  return SUCCESS;
#endif

  switch (MrcData->CpgcOptions.SetupType)  {
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  case  CPGC_CONFIG_MEMINIT:
    break;
#endif

  case CPGC_MEMORY_INIT:
    break;

  case CPGC_CONFIG_EARLY_MPR_TRAINING:
    break;

  case CPGC_CONFIG_VA:

    break;
  case CPGC_CONFIG_EARLYTRAINING:
    break;

  case CPGC_LDO:
    break;

#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  case CPGC_CONFIG_TURNAROUNDS:
    //
    // Inputs: Rank ... has special values of ALL_COMBOS, DR_COMBOS, DD_COMBOS, SR_COMBOS
    //
    FlagRunNormalTest = FALSE;
    *BytelaneStatus = 0;
    //
    // Actual don't do anything here with Rank though
    //
    for (rankA = 0; rankA < MAX_RANKS; rankA++){
      if (!(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[rankA])) {
        continue;
      }
      for (rankB = 0; rankB < MAX_RANKS; rankB++) {
        if (!(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[rankB])) {
          continue;
        }
        if (rankB < rankA) {
          continue;   // Only need to test combinations, not permutations
        }
        if ((MrcData->CpgcOptions.Rank == DR_COMBOS) && (GET_DIMM (rankA) != GET_DIMM (rankB))) {
          continue;
        }
        if ((MrcData->CpgcOptions.Rank == DD_COMBOS) && (GET_DIMM (rankA) == GET_DIMM (rankB))) {
          continue;
        }
        if ((MrcData->CpgcOptions.Rank == SR_COMBOS) && (rankA != rankB)) {
          continue;
        }
        MrcData->DynamicVars[Channel][SEQ0_TYPE]   = WRITE_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ1_TYPE]   = READ_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ2_TYPE]   = WRITE_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ3_TYPE]   = READ_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ_START]   = 0;
        MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0] = GetAddress (MrcData, Channel, rankA) >> (5 + MrcData->CpgcOptions.BusWidth);
        if (rankA == rankB)  {
          MrcData->DynamicVars[Channel][SEQ_END]     = 1;
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
          MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
        } else {   // Handle when ranks are not the same
          MrcData->DynamicVars[Channel][SEQ_END]   = 3;
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = GetAddress (MrcData, Channel, rankB) >> (5 + MrcData->CpgcOptions.BusWidth);
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_2] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1];
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_3] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
        }
        MmrcExecuteTask (MrcData, CPGC_TURNAROUNDS_RECONFIG, NO_PRINT, Channel);
        MmrcExecuteTask (MrcData, CPGC_RUNTEST, NO_PRINT, Channel);
        *BytelaneStatus |= (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] | (((UINT16) MrcData->DynamicVars[Channel][ECC_ERR_STAT]) << 8);
#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "RANK: %d  RANK %d  ", rankA, rankB));
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", MrcData->DynamicVars[Channel][LANES_HI], MrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  ", MrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif
      }
    }
    MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] =   *BytelaneStatus & 0xff;
    MrcData->DynamicVars[Channel][ECC_ERR_STAT]     = ((*BytelaneStatus) >> 8) & 1;
    break;
#endif
    //////////////////////////////////////////////////////////////////////
#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  case CPGC_CONFIG_LFSR_ADDRESS:
    FlagRunNormalTest = FALSE;
    //
    // Calculate rank bit (this only works for ANN/CHV , since it assumes at most 2 ranks
    //
    MrcData->DynamicVars[Channel][VAR_LO_SHIFT] =  MrcData->CpgcOptions.MinRankBit - 1  +  MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1]    ;   // only works for 1 or 2 rank system
    MrcData->DynamicVars[Channel][VAR_LO_SHIFT] -= (5 + MrcData->CpgcOptions.BusWidth + 2);
    //
    // Sweep numbursts from 5 to 14
    //
    *BytelaneStatus = 0;
    for (i = 5; i <= 14; i++) {
      MrcData->DynamicVars[Channel][NUM_BURSTS]     = i;
      MrcData->DynamicVars[Channel][LFSR_SEED]      = (i * 7) | 1;      // Just vary the seed to get different address sequences. Make sure BIT0 is set to avoid all 0 LFSR seeds.
      if (i <= 4) {
        MrcData->DynamicVars[Channel][VAR_WIDTH]    = 0;                // void going below 0
      } else {
        MrcData->DynamicVars[Channel][VAR_WIDTH]    = 0xa - (14 - i);   // i-4  .. so when i reaches 14 , we'll be at max var_width of 10.
      }
      if (i <= 9) {
        MrcData->DynamicVars[Channel][LOOP_COUNT]   = 15 - (i / 3);     // do extra loops if numbursts is small
      } else {
        MrcData->DynamicVars[Channel][LOOP_COUNT]   = (MrcData->CpgcOptions.LoopCount) / 2 + 2;  // scale back for large numBursts
      }
      MmrcExecuteTask (MrcData, CPGC_LFSR_ADDRESS_RECONFIG, NO_PRINT, Channel);
      MmrcExecuteTask (MrcData, CPGC_RUNTEST, NO_PRINT, Channel);
#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", MrcData->DynamicVars[Channel][LANES_HI], MrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  \n", MrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif
      *BytelaneStatus |= (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] | (((UINT16) MrcData->DynamicVars[Channel][ECC_ERR_STAT]) << 8);
    }
    MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] =    *BytelaneStatus & 0xff;
    MrcData->DynamicVars[Channel][ECC_ERR_STAT]     =  ((*BytelaneStatus) >> 8) & 1 ;
    break;
#endif  //RMT_ENABLE_LFSR_ADDRESS_TEST
  case CPGC_CONFIG_CMD_STRESS:
    // this function call is omitted inside GenericCmdMemoryTest as we are not doing VA rotate so only one CPGC test is run for every margin step sweep
    //CADB rotate is disabled as L_CpgcRunCadbTest function needs alot of clean up. In the current scheme only 8 CA lanes (DDR3L)get to be victim.
    MmrcExecuteTask(MrcData, CPGC_RUN_TEST, NO_PRINT, Channel);
    //These pointers needs to be assigned correctly
    *BytelaneStatus = (UINT16)MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
    *LANE_ERR_LO = (UINT32)MrcData->DynamicVars[Channel][LANE_ERR_STAT_LO];
    *LANE_ERR_HI = (UINT32)MrcData->DynamicVars[Channel][LANE_ERR_STAT_HI];
    *ECC_LANE_ERR = (UINT32)MrcData->DynamicVars[Channel][ECC_LANE_ERR_STAT];
    // ErrorCount is a pointer but used as an array at the assignment.
    // GenericCmdMemoryTest doesnt use these errorcounters. It reads ByteErrStatus from DynamicVars directly.
    ErrorCount[0] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT0];   //BL0 error counter
    ErrorCount[1] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT1];   //BL1 error counter... etc
    ErrorCount[2] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT2];
    ErrorCount[3] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT3];
    ErrorCount[4] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT4];
    ErrorCount[5] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT5];
    ErrorCount[6] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT6];
    ErrorCount[7] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT7];
    ErrorCount[8] = (UINT32)MrcData->DynamicVars[Channel][ERR_COUNT8];

    break; // end case CPGC_CONFIG_CMD_STRESS

  case CPGC_CONFIG_EARLY_CMD_TRAINING:
    break;

  default:
    MrcDeadLoop ();
  } // main case statement (SetupType)


  if (FlagRunNormalTest) {

    if (BytelaneStatus != NULL || LANE_ERR_LO != NULL || LANE_ERR_HI != NULL || ErrorCount != NULL){
      if(MrcData->CpgcOptions.ChunkErrorMask == CPGC_CHUNK_EVEN){
        if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr3) && (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr4)) {
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0x55;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP]  = 0x0;
        }
        else
        {
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0x55;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP]  = 0x55555555;
        }
      }
      else if (MrcData->CpgcOptions.ChunkErrorMask == CPGC_CHUNK_ODD){
        if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr3) && (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr4)) {
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xAA;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP]  = 0x0;
        }
        else
        {
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xAA;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP]  = 0xAAAAAAAA;
        }
      }
      else {
        if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr3) && (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr4)) {
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xFF;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP]  = 0x0;
        }
        else
        {
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xFF;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP]  = 0xFFFFFFFF;
        }
      }
      MmrcExecuteTask (MrcData, CPGC_RUN_TEST_L, NO_PRINT, Channel);
    }
    else {
      if (MrcData->CpgcOptions.SetupType == CPGC_MEMORY_INIT) {
        for (i = 0; i < 8; i++) { //test start at bank1, need to change back if it does not help with the hang problem.
          //Long ECC scrub
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK] = 0x0;
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK_LP] = 0x0;
          MrcData->DynamicVars[Channel][TARGET_BANK] = i;
          MmrcExecuteTask (MrcData, CPGC_TARGET_BANK, NO_PRINT, Channel);
          MmrcExecuteTask(MrcData, CPGC_RUN_TEST_L, NO_PRINT, Channel);
          //MmrcExecuteTask (MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
        }
      }
      else
        {
        MmrcExecuteTask(MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
        }
    }
  }
#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
    MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", MrcData->DynamicVars[Channel][LANES_HI], MrcData->DynamicVars[Channel][LANES_LO]));
    MemDebugPrint((SDBG_MAX, "Chnk: %02X  ", MrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif

  if (BytelaneStatus != NULL) {
    *BytelaneStatus = (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
      *BytelaneStatus |= (UINT16)(MrcData->DynamicVars[Channel][ECC_ERR_STAT] << 8);
    }
  }

  if(LANE_ERR_LO != NULL){
    *LANE_ERR_LO = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT_LO];
  }
  if(LANE_ERR_HI != NULL){
    *LANE_ERR_HI = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT_HI];
  }

  if (ECC_LANE_ERR != NULL) {
    *ECC_LANE_ERR = (UINT32)MrcData->DynamicVars[Channel][ECC_LANE_ERR_STAT];
  }

  if (ErrorCount != NULL) {
    // FODO
    ErrorCount[0] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT0];   //BL0 error counter
    ErrorCount[1] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT1];   //BL1 error counter... etc
    ErrorCount[2] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT2];
    ErrorCount[3] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT3];
    ErrorCount[4] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT4];
    ErrorCount[5] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT5];
    ErrorCount[6] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT6];
    ErrorCount[7] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT7];
    ErrorCount[8] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT8];
  }

  return MMRC_SUCCESS;
}


MMRC_STATUS
L_CpgcRunCadbTest (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  BOOLEAN * const   pTestPassed
)

/*
  This routine will do the rotations of cadb buffers to hit all victims, and run the CPGC tests until rotations are complete
    and stop if an error is ever encountered , returning pTestPassed=False.  If all tests pass, returns pTestPassed=True
  This test doesn't run in isolation.  You need to enable cpgc, setup cpgc cadb registers, etc..  This test just rotates and runs tests.
    It's assumed some outer function is calling it to setup cpgc, enable cpgc, and do any margining or sweeping of things
*/
{
  BOOLEAN rotationDone;
  UINT8 rotationN = 0;

  *pTestPassed = TRUE;
  rotationDone = FALSE;

  while (!rotationDone) {
    L_CpgcRotateCadb (
        ModMrcData,
        CapsuleStartIndex,
        StringIndex,
        Channel,
        rotationN,          // Pass in which rotation to use
        &rotationDone       // Return value of BOOLEAN - is rotation finished?
    );
    rotationN++;

    MmrcExecuteTask (ModMrcData, CPGC_RUN_TEST, NO_PRINT, Channel);

#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", ModMrcData->DynamicVars[Channel][LANES_HI], ModMrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  \n", ModMrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif

    if (ModMrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] != 0 ||
      (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled && ModMrcData->DynamicVars[Channel][ECC_ERR_STAT] != 0)) {
      *pTestPassed = FALSE;
      break;
    }
  }
  return MMRC_SUCCESS;
}


MMRC_STATUS
L_CpgcRotateCadb (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  UINT8             rotationN,          // pass in which rotation to use
  UINT8 * const     pRotationDone       // return value of BOOLEAN - is rotation finished?
)
{
  UINT8 pointer;
  UINT8 victimGroupNum;
  //
  // Victim group assignments. Specify signals within a victim group, with END_OF_CA as a delimiter.
  // 2 __DELIMITER in a row means end of all groups LPDDR victim group assignments.
  //
  CPGC_cadb_bufA CADB_BUFA[8];
#if DDR3_SUPPORT
  CPGC_cadb_bufB CADB_BUFB[4];
#endif

  pointer        = 0;
  victimGroupNum = 0;

#if LPDDR3_SUPPORT
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    //
    // Initialize CADB buffer to "all aggressors"
    //
    CADB_BUFA[0].Bits.row_col_addr = 0;
    CADB_BUFA[1].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[2].Bits.row_col_addr = 0;
    CADB_BUFA[3].Bits.row_col_addr = 0xfffff;
    while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over groups
      while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over signals within a group
        if (victimGroupNum == rotationN) {
          //
          // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
          //
          CADB_BUFA[1].Bits.row_col_addr &= (0xffffffff - (1 << CADB_CA_GROUPS[pointer]));  // Clear the bit
          CADB_BUFA[2].Bits.row_col_addr |= (             (1 << CADB_CA_GROUPS[pointer]));  // Set the bit
          //
          // CADB_BUFA[0,3] are already at correct value for victim
          //
        }
        pointer++;
      }
      victimGroupNum++;
      pointer++;
    }
  }
#endif   // LPDDR3_SUPPORT

#if DDR3_SUPPORT
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    //
    // Initialize CADB buffer to "all aggressors"
    //
    CADB_BUFA[0].Bits.row_col_addr  = 0;
    CADB_BUFA[1].Bits.row_col_addr  = 0xffff;
    CADB_BUFA[2].Bits.row_col_addr  = 0;
    CADB_BUFA[3].Bits.row_col_addr  = 0xffff;
    CADB_BUFA[0].Bits.bank_addr     = 0;
    CADB_BUFA[1].Bits.bank_addr     = 7;
    CADB_BUFA[2].Bits.bank_addr     = 0;
    CADB_BUFA[3].Bits.bank_addr     = 7;
    CADB_BUFB[0].Bits.cmd           = 0;
    CADB_BUFB[1].Bits.cmd           = 7;
    CADB_BUFB[2].Bits.cmd           = 0;
    CADB_BUFB[3].Bits.cmd           = 7;

    while (CADB_MA_GROUPS[pointer] != __DELIMITER) {  // Iterate over groups
      while (CADB_MA_GROUPS[pointer] != __DELIMITER) {  // Iterate over signals within a group
        if (victimGroupNum == rotationN) {
          if (CADB_MA_GROUPS[pointer] <= MA15) {
            //
            // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
            //
            CADB_BUFA[1].Bits.row_col_addr &= (0xffffffff - ( 1 << (CADB_MA_GROUPS[pointer] - MA0)));  // Clear the bit
            CADB_BUFA[2].Bits.row_col_addr |= (             ( 1 << (CADB_MA_GROUPS[pointer] - MA0)));  // Set the bit
            //
            // CADB_BUFA[0,3] are already at correct value for victim
            //
          }
          else if (CADB_MA_GROUPS[pointer] <= BA2 )
          {
            //
            // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
            //
            CADB_BUFA[1].Bits.bank_addr &= (0xffffffff - (1 << (CADB_MA_GROUPS[pointer] - BA0)));  // Clear the bit
            CADB_BUFA[2].Bits.bank_addr |= (             (1 << (CADB_MA_GROUPS[pointer] - BA0)));  // Set the bit
            //
            // CADB_BUFA[0,3] are already at correct value for victim
            //
          }
          else if (CADB_MA_GROUPS[pointer] <= WE )
          {
            //
            // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
            //
            CADB_BUFB[1].Bits.cmd &= (0xffffffff - (1 << (CADB_MA_GROUPS[pointer] - RAS)));  // Clear the bit
            CADB_BUFB[2].Bits.cmd |= (             (1 << (CADB_MA_GROUPS[pointer] - RAS)));  // Set the bit
            //
            // CADB_BUFA[0,3] are already at correct value for victim
            //
          }
        }
        pointer++;
      }
      victimGroupNum++;
      pointer++;
    }
  }
#endif // DDR3_SUPPORT

#if LPDDR4_SUPPORT

  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    //
    // Initialize CADB buffer to "all aggressors"
    //
    CADB_BUFA[0].Bits.row_col_addr = 0;
    CADB_BUFA[1].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[2].Bits.row_col_addr = 0;
    CADB_BUFA[3].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[4].Bits.row_col_addr = 0;
    CADB_BUFA[5].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[6].Bits.row_col_addr = 0;
    CADB_BUFA[7].Bits.row_col_addr = 0xfffff;

    while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over groups
      while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over signals within a group
        if (victimGroupNum == rotationN) {
          //
          // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
          //
          CADB_BUFA[1].Bits.row_col_addr &= (0xffffffff - (1 << CADB_CA_GROUPS[pointer]));  // Clear the bit
          CADB_BUFA[2].Bits.row_col_addr |= (             (1 << CADB_CA_GROUPS[pointer]));  // Set the bit
          //
          // CADB_BUFA[0,3] are already at correct value for victim
          //
        }
        pointer++;
      }
      victimGroupNum++;
      pointer++;
    }
  }

#endif //LPDDR4_SUPPORT

  victimGroupNum--;  // Over-count by 1 during loops
  if (victimGroupNum == rotationN) {
    *pRotationDone = TRUE;   // Pass back out that you've hit the last group
  } else {
    *pRotationDone = FALSE;
  }
#if LPDDR3_SUPPORT
  //
  // Write the CPGC CADB pattern registers
  //
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_ROW_COL]  = CADB_BUFA[0].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_ROW_COL]  = CADB_BUFA[1].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_ROW_COL]  = CADB_BUFA[2].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_ROW_COL]  = CADB_BUFA[3].Bits.row_col_addr ;
    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFA, NO_PRINT, Channel);
  }
#endif

#if LPDDR4_SUPPORT
  //
  // Write the CPGC CADB pattern registers
  //
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA4_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA5_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA6_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA7_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_ROW_COL]  = CADB_BUFA[0].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_ROW_COL]  = CADB_BUFA[1].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_ROW_COL]  = CADB_BUFA[2].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_ROW_COL]  = CADB_BUFA[3].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA4_ROW_COL]  = CADB_BUFA[4].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA5_ROW_COL]  = CADB_BUFA[5].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA6_ROW_COL]  = CADB_BUFA[6].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA7_ROW_COL]  = CADB_BUFA[7].Bits.row_col_addr ;
    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFA, NO_PRINT, Channel);
  }
#endif



#if DDR3_SUPPORT
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_BANK]     = CADB_BUFA[0].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_BANK]     = CADB_BUFA[1].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_BANK]     = CADB_BUFA[2].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_BANK]     = CADB_BUFA[3].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_ROW_COL]  = CADB_BUFA[0].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_ROW_COL]  = CADB_BUFA[1].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_ROW_COL]  = CADB_BUFA[2].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_ROW_COL]  = CADB_BUFA[3].Bits.row_col_addr ;

    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFA, 0x00D, Channel);

    ModMrcData->DynamicVars[Channel][CADB_BUFB0_CMD]      = CADB_BUFB[0].Bits.cmd;
    ModMrcData->DynamicVars[Channel][CADB_BUFB1_CMD]      = CADB_BUFB[1].Bits.cmd;
    ModMrcData->DynamicVars[Channel][CADB_BUFB2_CMD]      = CADB_BUFB[2].Bits.cmd;
    ModMrcData->DynamicVars[Channel][CADB_BUFB3_CMD]      = CADB_BUFB[3].Bits.cmd;

    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFB, 0x00D, Channel);
  }
#endif
  return MMRC_SUCCESS;
}


MMRC_STATUS
L_CpgcInit(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{

  MmrcExecuteTask (MrcData, CPGC_ONE_TIME_INIT, NO_PRINT, Channel);
  return MMRC_SUCCESS;

}

MMRC_STATUS
L_CpgcEnable(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{
  MmrcExecuteTask (MrcData, CPGC_ENABLE, NO_PRINT, Channel);
  return MMRC_SUCCESS;
}

MMRC_STATUS
L_CpgcDisable(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{
  MmrcExecuteTask (MrcData, CPGC_DISABLE, NO_PRINT, Channel);
  return MMRC_SUCCESS;
}

MMRC_STATUS
L_CpgcStopTest(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{
//  CPGC_DESELECT_DIS

  MmrcExecuteTask (MrcData, CPGC_DESELECT_DIS, 0, Channel);
  MmrcExecuteTask(MrcData, CPGC_STOP_TEST, NO_PRINT, Channel);
  return MMRC_SUCCESS;
}

#else


#include "cpgcapi.h"

//Long


enum {
  CA0, CA1, CA2, CA3, CA4, CA5, CA6, CA7, CA8, CA9
};
enum {
  RAS, CAS, WE, MA0, MA1, MA2, MA3, MA4, MA5, MA6, MA7, MA8, MA9, MA10, MA11, MA12, MA13, MA14, MA15
};

#define __DELIMITER 255

typedef union {
  UINT32      Data;
  struct {
    UINT32    row_col_addr    : 20;
    UINT32    reserved1       : 4;
    UINT32    bank_addr       : 3;
    UINT32    reserved2       : 5;
  } Bits;
} CPGC_cadb_bufA;


  // LPDDR3 victim group assignments.  Put TWO _DELIMITER's at end of all groups, and one delimiter between each group
  //  Currently this is common across channels, but may have to be 'per channel' depending on board layout
  CONST UINT8 CADB_CA_GROUPS[] = { CA0, CA5, CA8, __DELIMITER,     CA1, CA6, CA9, __DELIMITER,     CA2, CA3, CA4, CA7, __DELIMITER,    __DELIMITER };

  // CONST UINT8 CADB_CA_GROUPS[] = { CA0, CA2, CA4, CA6, CA8 ,__DELIMITER,   __DELIMITER };  // <-- this is equivalent as we had on VLV -- one group, every other bit is a victim

  // DDR3L victim group assignments
  CONST UINT8 CADB_MA_GROUPS[] = { MA0, MA3, MA6, MA9, MA12, MA15, RAS, __DELIMITER,
                             MA1, MA4, MA7, MA10,MA13, CAS,       __DELIMITER,
                             MA2, MA5, MA8, MA11,MA14, WE,        __DELIMITER,
                             __DELIMITER};





MMRC_STATUS

L_CpgcSetup (
    IN OUT  MMRC_DATA  *MrcData,
    IN      UINT8      Channel,
    IN      UINT8      SetupType
)
/*
  Routine Description:
    Unified routine to do initial setup on all cpgc routines
  Arguments:
    MrcData:  Will contain cpgc options passed through MrcData->CpgcOptions
    SetupType: Chooses which type of CPGC setup to perform.  Typically one per algo.
*/
{
#if CPGC_VERSION == 100
#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount = 0;
  return SUCCESS;
#endif
  //
  // One time initialization - this stuff really only needs to be initialized once, but i'm doing it every time CpgcSetup is called (should be moved to a one time routine eventually)
  //
  // FODO: when integrated with trunk, need to consider multi-channel support
  //
  if (MrcData->MaxDq <= 4) {
    MrcData->CpgcOptions.BusWidth = 0;
  } else  {
    MrcData->CpgcOptions.BusWidth = 1;
  }
  //
  // The following is ANN/CHV specific.  Works regardless if LPDDR or DDR3L.  This is the rank and bank bit position in the D-unit physical addr map.
  //   ideally you would get this somehow from spreadsheet, but the trouble is that it's run time determined as well due to "BusWidth"
  //
  MrcData->CpgcOptions.MinRankBit = RANK_SHIFT_BIT + MrcData->CpgcOptions.BusWidth; // The lowest rank bit in the DUNIT address map, used to target the correct rank.
  MrcData->CpgcOptions.MinBankBit = MIN_BANK_BIT + MrcData->CpgcOptions.BusWidth;   // The lowest bank bit in DUNIT address map, for cmd stress
  //
  // End of one time initialization
  //
  //
  // SetupType will be retained as state, so you don't have to pass into CpgcReconfig, and CpgcRunTest
  //
  MrcData->CpgcOptions.SetupType = SetupType;

  MmrcExecuteTask (MrcData, CPGC_ENABLE, NO_PRINT, Channel);
  MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 3;//MrcData->CpgcOptions.NumBursts - 3;
  MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;//MrcData->DynamicVars[Channel][LANE_ROTATE_RATE] + 1;      // Always LANE_ROTATE_RATE+1
  MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;   // 8 per rank tested   ,  should never get to 8th victim
  MrcData->DynamicVars[Channel][NUM_BURSTS]             = MrcData->CpgcOptions.NumBursts;
  MrcData->DynamicVars[Channel][LOOP_COUNT]             = MrcData->CpgcOptions.LoopCount;

  switch (SetupType)  {
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  case  CPGC_CONFIG_MEMINIT:
    //
    // Inputs: None
    //
    // TOTAL_CACHE_LINES is the # of total cachelines available in memory, for this channel
    // This needs to be setup by some external routine.   On trunk code it is set in ConfigMem.c.   For sim I will pass in from sus-wrapper
    // It has to be adjusted by BusWidth to account for 32B vs 64B request
    //
    MrcData->DynamicVars[Channel][TOTAL_COMMANDS] = ((UINT32) ((MrcData->DynamicVars[Channel][TOTAL_CACHE_LINES]) << (1 - MrcData->CpgcOptions.BusWidth))) - 1;
    //
    // Takes input of TOTAL_CACHELINES, which means in all of avialalbe memory
    //
    MmrcExecuteTask (MrcData, CPGC_MEMTEST_SETUP, NO_PRINT, Channel);
    break;  // end case CPGC_CONFIG_MEMINIT
#endif
  case CPGC_CONFIG_VA:
    //
    // Already setup above (numbursts, loopcount, etc)
    //
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;
  case CPGC_CONFIG_EARLYTRAINING:
    //
    // Inputs: LoopCount, NumBursts
    //
    MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;
    MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;
    MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 0;
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;
#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  case CPGC_CONFIG_TURNAROUNDS:
    //
    // Already setup above (numbursts, loopcount, etc)
    //
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;
#endif
#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  case CPGC_CONFIG_LFSR_ADDRESS:
    MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;      // Disable all the lane rotations stuff, since we sweep numbursts anyways
    MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;
    MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 0;
    MrcData->DynamicVars[Channel][NUM_BURSTS]             = 1;
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_LFSR_ADDRESS, NO_PRINT, Channel);
    break;
#endif
  case CPGC_CONFIG_CMD_STRESS:
    //
    // Inputs: LoopCount, NumBursts
    // Setup the basic VA mode.
    //
    //
    // VAR_SEG_HIGHADR_HI/LO sets the LFSR addressing for Subseq 2 & 3 to target high address bits, such as rows
    //
    MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_HI] = (MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1 + 3;
#if LPDDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TYPE_LPDDR3) {
      if ((MrcData->CpgcOptions.MinRankBit - 5 - MrcData->CpgcOptions.BusWidth) < ((MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1)) {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinBankBit) - 5 - MrcData->CpgcOptions.BusWidth;
      } else {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1;  // set low 3 bits of var segment to just be adjacent to hi part of var segment
      }
    }
#endif
#if DDR3_SUPPORT
    if  (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TYPE_DDR3) {
      //
      // Make sure min bank bit is low enough , otherwise hi/lo segment will alias
      //
      if (((MrcData->CpgcOptions.MinBankBit + 1 - MrcData->CpgcOptions.BusWidth) - 5 - MrcData->CpgcOptions.BusWidth) < ((MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1)) {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinBankBit + 1 - MrcData->CpgcOptions.BusWidth ) - 5 - MrcData->CpgcOptions.BusWidth;      // set low 3 bits of var segment to point at banks
      } else {
        MrcData->DynamicVars[Channel][VAR_SEG_HIGHADDR_LO] = (MrcData->CpgcOptions.MinRankBit - 1) - 5 - MrcData->CpgcOptions.BusWidth - 15 + 1;  // set low 3 bits of var segment to just be adjacent to hi part of var segment
      }
    }
#endif
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT] = 0; // initially target rank=0, but this should be overridden by CpgcReconfig
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_CADB1, NO_PRINT, Channel);
    break;
  default:
    MrcDeadLoop ();
  } // main case statement (SetupType)
#elif CPGC_VERSION == 200
//
  // One time initialization - this stuff really only needs to be initialized once, but i'm doing it every time CpgcSetup is called (should be moved to a one time routine eventually)
  //
  // FODO: when integrated with trunk, need to consider multi-channel support
  //
  //if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank] <= 4) {
  //  MrcData->CpgcOptions.BusWidth = 0;
  //} else  {
  //  MrcData->CpgcOptions.BusWidth = 1;
  //}

  //
  //
  // SetupType will be retained as state, so you don't have to pass into CpgcReconfig, and CpgcRunTest
  //
  MrcData->CpgcOptions.SetupType = SetupType;

  MmrcExecuteTask (MrcData, CPGC_ENABLE, NO_PRINT, Channel);
  MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 3;//MrcData->CpgcOptions.NumBursts - 3;
  MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;//MrcData->DynamicVars[Channel][LANE_ROTATE_RATE] + 1;      // Always LANE_ROTATE_RATE+1
  MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;   // 8 per rank tested   ,  should never get to 8th victim
  MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = (UINT32) ((1 << MrcData->CpgcOptions.NumBursts) - 1);
  MrcData->DynamicVars[Channel][NUM_LOOPS_EXP]             = (UINT32) ((1 << MrcData->CpgcOptions.LoopCount) - 1);


  switch (SetupType)  {

  case CPGC_CONFIG_EARLY_MPR_TRAINING:
  case CPGC_CONFIG_EARLYTRAINING:
    //
    // Inputs: LoopCount, NumBursts
    //
    MrcData->DynamicVars[Channel][RELOAD_LFSR_SEED_RATE]  = 0;
    MrcData->DynamicVars[Channel][SAVE_LFSR_SEED_RATE]    = 0;
    MrcData->DynamicVars[Channel][LANE_ROTATE_RATE]       = 0;
    if (MrcData->CpgcOptions.SetupType == CPGC_CONFIG_EARLYTRAINING) {
      MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    }
    else {
      MmrcExecuteTask (MrcData, CPGC_LDO_DPAT_LFSR, NO_PRINT, Channel); //sets up all DQ to follow unisequencer0
      MmrcExecuteTask (MrcData, CPGC_SETUP_ERT, NO_PRINT, Channel);  //Puts CPGC into direct mode for early MPR
    }
    break;

  case CPGC_CONFIG_VA:
    //
    // Already setup above (numbursts, loopcount, etc)
    //
    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;


  case CPGC_CONFIG_TURNAROUNDS:
    //
    // WIP: Will revist for E-0 after they fix In order mode b2b write traffic
    //

    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    break;

  case CPGC_CONFIG_MEMINIT:
    //
    // Memtest not yet implemented in SS. CPGC has bug with STOP ON ERROR
    // Place Holder: Will revisit later
    //
    break;

  case CPGC_CONFIG_LFSR_ADDRESS:
    // Placeholder, will revisit later

    break;

  case CPGC_LDO:
    switch (MrcData->CpgcOptions.LDOSequence)  {

        case DQ_Setup:
            MmrcExecuteTask (MrcData, CPGC_LDO_DPAT, NO_PRINT, Channel);
            break;

        case CADB_Setup:
            MmrcExecuteTask (MrcData, CPGC_LDO_CADB, NO_PRINT, Channel);

            break;
        case CADB_LFSR:
            MmrcExecuteTask (MrcData, CPGC_LDO_CADB_LFSR, NO_PRINT, Channel);

            break;
        case DQ_LFSR:
            MmrcExecuteTask (MrcData, CPGC_LDO_DPAT_LFSR, NO_PRINT, Channel);
            break;
    }

    break;

 case CPGC_CONFIG_CMD_STRESS:
    //
    //
    //
    //

    MmrcExecuteTask (MrcData, CPGC_SETUP_VA, NO_PRINT, Channel);
    MrcData->DynamicVars[Channel][NUM_ROWS]       = 14;
    MrcData->DynamicVars[Channel][NUM_COLS]       = 5;
    MrcData->DynamicVars[Channel][ADD_ROTATE_RPT] = 3 + (MrcData->DynamicVars[Channel][NUM_ROWS]  + MrcData->DynamicVars[Channel][NUM_COLS]) - 1 ;
    MmrcExecuteTask (MrcData, CPGC_CADB1, NO_PRINT, Channel);
    if (MrcData->CpgcOptions.CADB_Harrassment) {
            MmrcExecuteTask (MrcData, CPGC_DESELECT_EN, 0, Channel);
        }
    else {
            MmrcExecuteTask (MrcData, CPGC_DESELECT_DIS, 0, Channel);
        }
    break;



  default:
    MrcDeadLoop ();
  } // main case statement (SetupType)
#endif // CPGC_VERSION == 100
  if (SetupType != CPGC_CONFIG_MEMINIT || SetupType != CPGC_CONFIG_CMD_STRESS) {
    MmrcExecuteTask (MrcData, CPGC_LOOPS, NO_PRINT, Channel);
  }
  else {
      MrcData->DynamicVars[Channel][NUM_BURSTS_EXP]            = 7; //Hard code 7 for CMD stress
      MmrcExecuteTask (MrcData, CPGC_LOOPS, NO_PRINT, Channel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
L_CpgcReconfig (
  IN OUT  MMRC_DATA  *MrcData,
  IN      UINT8      Channel

)
/* Routine Description:
 *  Similar description as CpgcSetup.  This is called after CpgcSetup for
 *    a particular SetupType, to reconfigure it to a new rank, loopcount,
 *    or algo specific change during the algorithm.
 *  All these kind of options are passed through MrcData->CpgcOptions
 *  WARNING: A call to CpgcSetup is required before using CpgcReconfig !!
 *    This is true for each algo (need to call CpgcSetup first inside each algo)
 */
{

#if CPGC_VERSION == 100

  BOOLEAN   ExecuteMode;

  ExecuteMode = MrcData->ExecuteThisRoutineInParallel;
  MrcData->ExecuteThisRoutineInParallel = FALSE;

#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount =0;
  return SUCCESS;
#endif

  switch (MrcData->CpgcOptions.SetupType)  {
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  case CPGC_CONFIG_MEMINIT:
    MmrcExecuteTask (MrcData, CPGC_MEMINIT, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_MEMINIT_AND_TEST, NO_PRINT, Channel);
    break;
  case CPGC_CONFIG_MEMTEST:
    MmrcExecuteTask (MrcData, CPGC_MEMTEST, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_MEMINIT_AND_TEST, NO_PRINT, Channel);
    break;
#endif
  case CPGC_CONFIG_VA:
    //
    // Inputs: Rank
    //
    MrcData->DynamicVars[Channel][SEQ_START]  = 0;
    MrcData->DynamicVars[Channel][SEQ_END]    = 0;
   //kirk  MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0]  = MrcData->CpgcOptions.Rank << (GetAddressBit(MrcData, Channel, MrcData->CpgcOptions.Rank) - (5 + MrcData->CpgcOptions.BusWidth));
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1]  = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_2]  = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_3]  = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;
#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  case CPGC_CONFIG_TURNAROUNDS:
    //
    // Inputs: Rank ... has special values of ALL_COMBOS, DR_COMBOS, DD_COMBOS, SR_COMBOS
    // We actually don't do anything here with Rank though
    //
    break;
#endif
  case CPGC_CONFIG_EARLYTRAINING:
  case CPGC_CONFIG_EARLY_MPR_TRAINING:

    // kirk MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0] = MrcData->CpgcOptions.Rank << (GetAddressBit(MrcData, Channel, MrcData->CpgcOptions.Rank) - (5 + MrcData->CpgcOptions.BusWidth));
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    switch (MrcData->CpgcOptions.CmdSequence) {
    case CPGC_CMDSEQ_READ:
      MrcData->DynamicVars[Channel][SEQ_START]  = 1;
      MrcData->DynamicVars[Channel][SEQ_END]    = 1;
      break;
    case CPGC_CMDSEQ_WRITE:
      MrcData->DynamicVars[Channel][SEQ_START]  = 0;
      MrcData->DynamicVars[Channel][SEQ_END]    = 0;
      break;
    case CPGC_CMDSEQ_WRITE_READ:
      MrcData->DynamicVars[Channel][SEQ_START]  = 0;
      MrcData->DynamicVars[Channel][SEQ_END]    = 1;
      break;
    }
    MrcData->DynamicVars[Channel][AGG_PATTERN] = MrcData->CpgcOptions.Pattern;
    MrcData->DynamicVars[Channel][VIC_PATTERN] = MrcData->CpgcOptions.VicPattern;
    MmrcExecuteTask (MrcData, CPGC_CWL_PATTERN, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, CPGC_SETUP_LVL, NO_PRINT, Channel);
    if (MrcData->CpgcOptions.SetupType == CPGC_CONFIG_EARLYTRAINING) {
      break;
    }
    //
    // Setup the CPGC as Buffer Mode for Uniseq0/1
    //
    MmrcExecuteTask (MrcData, CPGC_MPR_TRAINING, NO_PRINT, Channel);
    break;

#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  case CPGC_CONFIG_LFSR_ADDRESS:
    break;
#endif
  case CPGC_CONFIG_CMD_STRESS:
    //
    // Inputs: Rank
    //
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0] = MrcData->CpgcOptions.Rank << ((MrcData->CpgcOptions.MinRankBit) - 5 - MrcData->CpgcOptions.BusWidth);
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_2] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_3] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;
  default:
    MrcDeadLoop ();
  } // Main case statement (SetupType)

  MrcData->ExecuteThisRoutineInParallel = ExecuteMode;



#elif CPGC_VERSION == 200



#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount =0;
  return SUCCESS;
#endif

  switch (MrcData->CpgcOptions.SetupType)  {

  case CPGC_CONFIG_EARLY_MPR_TRAINING:

  case CPGC_CONFIG_EARLYTRAINING:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
    MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
    if (MrcData->CpgcOptions.DPAT_LFSR){
        MrcData->DynamicVars[Channel][UNIMODE] = 2;
    }
    else{
         MrcData->DynamicVars[Channel][UNIMODE] = 1;
    }

    MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);

    switch (MrcData->CpgcOptions.CmdSequence) {
        case CPGC_CMDSEQ_READ:
            MrcData->DynamicVars[Channel][INSTR_LAST] = 1;
            MrcData->DynamicVars[Channel][INSTR_POINTER] = 1;
            break;
        case CPGC_CMDSEQ_WRITE:
            MrcData->DynamicVars[Channel][INSTR_LAST] = 1;
            MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;
            break;
        case CPGC_CMDSEQ_WRITE_READ:
            MrcData->DynamicVars[Channel][INSTR_LAST] = 0;
            MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;
            break;
        default:  //defaults to write follow by read
            MrcData->DynamicVars[Channel][INSTR_LAST] = 0;
            MrcData->DynamicVars[Channel][INSTR_POINTER] = 0;

        }

    MmrcExecuteTask (MrcData, CPGC_RESETUP_TRAFFIC, NO_PRINT, Channel);
    break;

  case CPGC_CONFIG_LFSR_ADDRESS:
      //placeholder
    break;

  case CPGC_CONFIG_VA:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;


  case CPGC_CONFIG_CMD_STRESS:
    //
    // Inputs: Rank
    //

    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    break;

  case CPGC_LDO:
    MrcData->DynamicVars[Channel][TARGET_RANK] = MrcData->CpgcOptions.Rank;
    MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
    switch (MrcData->CpgcOptions.LDOSequence)  {

        case DQ_Setup:
            //MmrcExecuteTask (MrcData, CPGC_LDO_DPAT, 0, Channel);
            MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
            MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
            MrcData->DynamicVars[Channel][UNIMODE] = 1;
            MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);

            break;

        case CADB_Setup:
            MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
            MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
            MrcData->DynamicVars[Channel][UNIMODE] = 1;
            MmrcExecuteTask (MrcData,CPGC_CADB_MODE, NO_PRINT, Channel);


            break;
        case CADB_LFSR:
            MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
            MrcData->DynamicVars[Channel][PATTERN1] = MrcData->CpgcOptions.Pattern1;
            MrcData->DynamicVars[Channel][PATTERN2] = MrcData->CpgcOptions.Pattern2;
            MrcData->DynamicVars[Channel][UNIMODE] = 2;
            MmrcExecuteTask (MrcData, CPGC_CADB_MODE, NO_PRINT, Channel);

            break;
        case DQ_LFSR:

            MrcData->DynamicVars[Channel][PATTERN0] = MrcData->CpgcOptions.Pattern0;
            MrcData->DynamicVars[Channel][UNIMODE] = 2;
            MmrcExecuteTask (MrcData, CPGC_DPAT_MODE, NO_PRINT, Channel);
            break;

    }

    break;

  case CPGC_CONFIG_MEMINIT:
      //placeholder

    break;
#endif

  default:
    MrcDeadLoop ();
  } // Main case statement (SetupType)
  return MMRC_SUCCESS;
}




MMRC_STATUS
L_CpgcRunTest (
IN OUT  MMRC_DATA  *MrcData,
IN      UINT8      Channel,
OUT     UINT16     *BytelaneStatus,
OUT     UINT32  *LANE_ERR_LO,
OUT     UINT32  *LANE_ERR_HI,
OUT     UINT32     *ErrorCount
)
/* Routine Description:
 *  Similar description as CpgcSetup.  This runs the test and returns
 *  data  through ByteLaneStatus, LaneStatus, and ErrorCount variables.
 *  LaneStatus shall be an array or UINT32, large enough to hold a 1-bit
 *  error status for every DQ lane, LSB first.
 *  WARNING: CpgcSetup must be called *BEFORE* CpgcRunTest, for each algo.
 *  Return Values:
 *     BytelaneStatus, LaneStatus, ErrorCount, will return the usual cpgc error statuses.
 *        LaneStatus needs to be an array of 32-bit wide ints, with enough size to contain
 *        error status for all lanes.  E.g. length of 3 needed to hold x72 width bus.
 *        You can pass in NULL for these individually for error status you are not interested in.
 *        FODO: Support for 'KNOBS'.
 */
{
  UINT8  FlagRunNormalTest = TRUE;  // Used in case statement for SetupType
#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  UINT8 i;
#endif
#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  UINT8 rankA, rankB;
#endif
  BOOLEAN TestPassed;
  //
  // If overridden to FALSE, it is assumed the case statement ran the CPGC test
  // If TRUE, then the normal CPGC_RUNTEST task will be run after the case statement.

#if RMT_FAKE_ERRORS == 1
  MrcData->CpgcOptions.RmtFakeErrorCount++;
  if (! (MrcData->CpgcOptions.RmtFakeErrorCount % 8)) {
    *BytelaneStatus = (UINT16) 0x1ff;
  }
  else {
    *BytelaneStatus = (UINT16) 0x0;
  }
  return SUCCESS;
#endif

  switch (MrcData->CpgcOptions.SetupType)  {
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  case  CPGC_CONFIG_MEMINIT:
    break;
#endif

  case CPGC_CONFIG_EARLY_MPR_TRAINING:
      break;

  case CPGC_CONFIG_VA:

    break;
  case CPGC_CONFIG_EARLYTRAINING:
    break;

  case CPGC_LDO:
      break;

#if RMT_ENABLE_TURNAROUNDS_TEST == 1
  case CPGC_CONFIG_TURNAROUNDS:
    //
    // Inputs: Rank ... has special values of ALL_COMBOS, DR_COMBOS, DD_COMBOS, SR_COMBOS
    //
    FlagRunNormalTest = FALSE;
    *BytelaneStatus = 0;
    //
    // Actual don't do anything here with Rank though
    //
    for (rankA = 0; rankA < MAX_RANKS; rankA++){
      if (!(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[rankA])) {
        continue;
      }
      for (rankB = 0; rankB < MAX_RANKS; rankB++) {
        if (!(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[rankB])) {
          continue;
        }
        if (rankB < rankA) {
          continue;   // Only need to test combinations, not permutations
        }
        if ((MrcData->CpgcOptions.Rank == DR_COMBOS) && (GET_DIMM (rankA) != GET_DIMM (rankB))) {
          continue;
        }
        if ((MrcData->CpgcOptions.Rank == DD_COMBOS) && (GET_DIMM (rankA) == GET_DIMM (rankB))) {
          continue;
        }
        if ((MrcData->CpgcOptions.Rank == SR_COMBOS) && (rankA != rankB)) {
          continue;
        }
        MrcData->DynamicVars[Channel][SEQ0_TYPE]   = WRITE_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ1_TYPE]   = READ_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ2_TYPE]   = WRITE_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ3_TYPE]   = READ_SUB_TYPE;
        MrcData->DynamicVars[Channel][SEQ_START]   = 0;
        MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0] = GetAddress (MrcData, Channel, rankA) >> (5 + MrcData->CpgcOptions.BusWidth);
        if (rankA == rankB)  {
          MrcData->DynamicVars[Channel][SEQ_END]     = 1;
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
          MmrcExecuteTask (MrcData, CPGC_FIXVAR, NO_PRINT, Channel);
        } else {   // Handle when ranks are not the same
          MrcData->DynamicVars[Channel][SEQ_END]   = 3;
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1] = GetAddress (MrcData, Channel, rankB) >> (5 + MrcData->CpgcOptions.BusWidth);
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_2] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_1];
          MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_3] = MrcData->DynamicVars[Channel][FIX_SEGMENT_RANK_SELECT_0];
        }
        MmrcExecuteTask (MrcData, CPGC_TURNAROUNDS_RECONFIG, NO_PRINT, Channel);
        MmrcExecuteTask (MrcData, CPGC_RUNTEST, NO_PRINT, Channel);
        *BytelaneStatus |= (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] | (((UINT16) MrcData->DynamicVars[Channel][ECC_ERR_STAT]) << 8);
#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "RANK: %d  RANK %d  ", rankA, rankB));
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", MrcData->DynamicVars[Channel][LANES_HI], MrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  ", MrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif
      }
    }
    MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] =   *BytelaneStatus & 0xff;
    MrcData->DynamicVars[Channel][ECC_ERR_STAT]     = ((*BytelaneStatus) >> 8) & 1;
    break;
#endif
    //////////////////////////////////////////////////////////////////////
#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
  case CPGC_CONFIG_LFSR_ADDRESS:
    FlagRunNormalTest = FALSE;
    //
    // Calculate rank bit (this only works for ANN/CHV , since it assumes at most 2 ranks
    //
    MrcData->DynamicVars[Channel][VAR_LO_SHIFT] =  MrcData->CpgcOptions.MinRankBit - 1  +  MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1]    ;   // only works for 1 or 2 rank system
    MrcData->DynamicVars[Channel][VAR_LO_SHIFT] -= (5 + MrcData->CpgcOptions.BusWidth + 2);
    //
    // Sweep numbursts from 5 to 14
    //
    *BytelaneStatus = 0;
    for (i = 5; i <= 14; i++) {
      MrcData->DynamicVars[Channel][NUM_BURSTS]     = i;
      MrcData->DynamicVars[Channel][LFSR_SEED]      = (i * 7) | 1;      // Just vary the seed to get different address sequences. Make sure BIT0 is set to avoid all 0 LFSR seeds.
      if (i <= 4) {
        MrcData->DynamicVars[Channel][VAR_WIDTH]    = 0;                // void going below 0
      } else {
        MrcData->DynamicVars[Channel][VAR_WIDTH]    = 0xa - (14 - i);   // i-4  .. so when i reaches 14 , we'll be at max var_width of 10.
      }
      if (i <= 9) {
        MrcData->DynamicVars[Channel][LOOP_COUNT]   = 15 - (i / 3);     // do extra loops if numbursts is small
      } else {
        MrcData->DynamicVars[Channel][LOOP_COUNT]   = (MrcData->CpgcOptions.LoopCount) / 2 + 2;  // scale back for large numBursts
      }
      MmrcExecuteTask (MrcData, CPGC_LFSR_ADDRESS_RECONFIG, NO_PRINT, Channel);
      MmrcExecuteTask (MrcData, CPGC_RUNTEST, NO_PRINT, Channel);
#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", MrcData->DynamicVars[Channel][LANES_HI], MrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  \n", MrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif
      *BytelaneStatus |= (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] | (((UINT16) MrcData->DynamicVars[Channel][ECC_ERR_STAT]) << 8);
    }
    MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] =    *BytelaneStatus & 0xff;
    MrcData->DynamicVars[Channel][ECC_ERR_STAT]     =  ((*BytelaneStatus) >> 8) & 1 ;
    break;
#endif  //RMT_ENABLE_LFSR_ADDRESS_TEST
  case CPGC_CONFIG_CMD_STRESS:
    FlagRunNormalTest = FALSE;
    L_CpgcRunCadbTest (MrcData, 0x00D, 0x00D, Channel, &TestPassed);
    // ron FODO: eventually report errors properly.. but for now just report pass/fail effectively
    if (TestPassed) {
      MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT]  = 0x0;
      // FODO: ErrorCount = 0;
    }
    else {
      MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT]  = 0xff;
      //MrcData->DynamicVars[Channel][ECC_ERR_STAT] = 1;
      // FODO: ErrorCount = 1;  // error counts >1 are meqningless for cmd stress
    }
    break; // end case CPGC_CONFIG_CMD_STRESS
  default:
    MrcDeadLoop ();
  } // main case statement (SetupType)


  if (FlagRunNormalTest) {

    if (BytelaneStatus != NULL || LANE_ERR_LO != NULL || LANE_ERR_HI != NULL || ErrorCount != NULL){
        if(MrcData->CpgcOptions.ChunkErrorMask == CPGC_CHUNK_EVEN){
            MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0x55;
                }
        else if (MrcData->CpgcOptions.ChunkErrorMask == CPGC_CHUNK_ODD){
            MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xAA;
                }
        else {
            MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xFF;
            }
        MmrcExecuteTask (MrcData, CPGC_RUN_TEST_L, NO_PRINT, Channel);
    }
    else {
        MmrcExecuteTask (MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
    }

#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", MrcData->DynamicVars[Channel][LANES_HI], MrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  ", MrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif
  }
  if (BytelaneStatus != NULL) {
    *BytelaneStatus = (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
      *BytelaneStatus |= (UINT16)(MrcData->DynamicVars[Channel][ECC_ERR_STAT] << 8);
    }
  }

  if(LANE_ERR_LO != NULL){
    *LANE_ERR_LO = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT_LO];
  }
    if(LANE_ERR_HI != NULL){
    *LANE_ERR_HI = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT_HI];
  }

  if (ErrorCount != NULL) {
    // FODO
    ErrorCount[0] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT0];   //BL0 error counter
    ErrorCount[1] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT1];   //BL1 error counter... etc
    ErrorCount[2] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT2];
    ErrorCount[3] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT3];
    ErrorCount[4] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT4];
    ErrorCount[5] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT5];
    ErrorCount[6] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT6];
    ErrorCount[7] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT7];
    ErrorCount[8] = (UINT32) MrcData->DynamicVars[Channel][ERR_COUNT8];
  }

  return MMRC_SUCCESS;
}


MMRC_STATUS
L_CpgcRunCadbTest (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  BOOLEAN * const   pTestPassed
)

/*
  This routine will do the rotations of cadb buffers to hit all victims, and run the CPGC tests until rotations are complete
    and stop if an error is ever encountered , returning pTestPassed=False.  If all tests pass, returns pTestPassed=True
  This test doesn't run in isolation.  You need to enable cpgc, setup cpgc cadb registers, etc..  This test just rotates and runs tests.
    It's assumed some outer function is calling it to setup cpgc, enable cpgc, and do any margining or sweeping of things
*/
{
  BOOLEAN rotationDone;
  UINT8 rotationN = 0;

  *pTestPassed = TRUE;
  rotationDone = FALSE;

  while (!rotationDone) {
    L_CpgcRotateCadb (
        ModMrcData,
        CapsuleStartIndex,
        StringIndex,
        Channel,
        rotationN,          // Pass in which rotation to use
        &rotationDone       // Return value of BOOLEAN - is rotation finished?
    );
    rotationN++;

    MmrcExecuteTask (ModMrcData, CPGC_RUN_TEST, NO_PRINT, Channel);

#if RMT_BIT_ERROR_LOGGING_ENABLE == 1
        MemDebugPrint((SDBG_MAX, "Bits: %08X %08X  ", ModMrcData->DynamicVars[Channel][LANES_HI], ModMrcData->DynamicVars[Channel][LANES_LO]));
        MemDebugPrint((SDBG_MAX, "Chnk: %02X  \n", ModMrcData->DynamicVars[Channel][CHUNK_STAT]));
#endif

    if (ModMrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT] != 0 ||
      (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled && ModMrcData->DynamicVars[Channel][ECC_ERR_STAT] != 0)) {
      *pTestPassed = FALSE;
      break;
    }
  }
  return MMRC_SUCCESS;
}


MMRC_STATUS
L_CpgcRotateCadb (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  UINT8             rotationN,          // pass in which rotation to use
  UINT8 * const     pRotationDone       // return value of BOOLEAN - is rotation finished?
)
{
  UINT8 pointer;
  UINT8 victimGroupNum;
  //
  // Victim group assignments. Specify signals within a victim group, with END_OF_CA as a delimiter.
  // 2 __DELIMITER in a row means end of all groups LPDDR victim group assignments.
  //
  CPGC_cadb_bufA CADB_BUFA[8];
#if DDR3_SUPPORT
  CPGC_cadb_bufB CADB_BUFB[4];
#endif

  pointer        = 0;
  victimGroupNum = 0;

#if LPDDR3_SUPPORT
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    //
    // Initialize CADB buffer to "all aggressors"
    //
    CADB_BUFA[0].Bits.row_col_addr = 0;
    CADB_BUFA[1].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[2].Bits.row_col_addr = 0;
    CADB_BUFA[3].Bits.row_col_addr = 0xfffff;
    while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over groups
      while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over signals within a group
        if (victimGroupNum == rotationN) {
          //
          // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
          //
          CADB_BUFA[1].Bits.row_col_addr &= (0xffffffff - (1 << CADB_CA_GROUPS[pointer]));  // Clear the bit
          CADB_BUFA[2].Bits.row_col_addr |= (             (1 << CADB_CA_GROUPS[pointer]));  // Set the bit
          //
          // CADB_BUFA[0,3] are already at correct value for victim
          //
        }
        pointer++;
      }
      victimGroupNum++;
      pointer++;
    }
  }
#endif   // LPDDR3_SUPPORT

#if DDR3_SUPPORT
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TYPE_DDR3) {
    //
    // Initialize CADB buffer to "all aggressors"
    //
    CADB_BUFA[0].Bits.row_col_addr  = 0;
    CADB_BUFA[1].Bits.row_col_addr  = 0xffff;
    CADB_BUFA[2].Bits.row_col_addr  = 0;
    CADB_BUFA[3].Bits.row_col_addr  = 0xffff;
    CADB_BUFA[0].Bits.bank_addr     = 0;
    CADB_BUFA[1].Bits.bank_addr     = 7;
    CADB_BUFA[2].Bits.bank_addr     = 0;
    CADB_BUFA[3].Bits.bank_addr     = 7;
    CADB_BUFB[0].Bits.cmd           = 0;
    CADB_BUFB[1].Bits.cmd           = 7;
    CADB_BUFB[2].Bits.cmd           = 0;
    CADB_BUFB[3].Bits.cmd           = 7;

    while (CADB_MA_GROUPS[pointer] != __DELIMITER) {  // Iterate over groups
      while (CADB_MA_GROUPS[pointer] != __DELIMITER) {  // Iterate over signals within a group
        if (victimGroupNum == rotationN) {
          if (CADB_MA_GROUPS[pointer] <= MA15) {
            //
            // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
            //
            CADB_BUFA[1].Bits.row_col_addr &= (0xffffffff - ( 1 << (CADB_MA_GROUPS[pointer] - MA0)));  // Clear the bit
            CADB_BUFA[2].Bits.row_col_addr |= (             ( 1 << (CADB_MA_GROUPS[pointer] - MA0)));  // Set the bit
            //
            // CADB_BUFA[0,3] are already at correct value for victim
            //
          }
          else if (CADB_MA_GROUPS[pointer] <= BA2 )
          {
            //
            // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
            //
            CADB_BUFA[1].Bits.bank_addr &= (0xffffffff - (1 << (CADB_MA_GROUPS[pointer] - BA0)));  // Clear the bit
            CADB_BUFA[2].Bits.bank_addr |= (             (1 << (CADB_MA_GROUPS[pointer] - BA0)));  // Set the bit
            //
            // CADB_BUFA[0,3] are already at correct value for victim
            //
          }
          else if (CADB_MA_GROUPS[pointer] <= WE )
          {
            //
            // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
            //
            CADB_BUFB[1].Bits.cmd &= (0xffffffff - (1 << (CADB_MA_GROUPS[pointer] - RAS)));  // Clear the bit
            CADB_BUFB[2].Bits.cmd |= (             (1 << (CADB_MA_GROUPS[pointer] - RAS)));  // Set the bit
            //
            // CADB_BUFA[0,3] are already at correct value for victim
            //
          }
        }
        pointer++;
      }
      victimGroupNum++;
      pointer++;
    }
  }
#endif // DDR3_SUPPORT

#if LPDDR4_SUPPORT

    if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    //
    // Initialize CADB buffer to "all aggressors"
    //
    CADB_BUFA[0].Bits.row_col_addr = 0;
    CADB_BUFA[1].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[2].Bits.row_col_addr = 0;
    CADB_BUFA[3].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[4].Bits.row_col_addr = 0;
    CADB_BUFA[5].Bits.row_col_addr = 0xfffff;
    CADB_BUFA[6].Bits.row_col_addr = 0;
    CADB_BUFA[7].Bits.row_col_addr = 0xfffff;

    while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over groups
      while (CADB_CA_GROUPS[pointer] != __DELIMITER) {  // Iterate over signals within a group
        if (victimGroupNum == rotationN) {
          //
          // Set this signal as a victim ... 0 0 1 1 instead of 0 1 0 1
          //
          CADB_BUFA[1].Bits.row_col_addr &= (0xffffffff - (1 << CADB_CA_GROUPS[pointer]));  // Clear the bit
          CADB_BUFA[2].Bits.row_col_addr |= (             (1 << CADB_CA_GROUPS[pointer]));  // Set the bit
          //
          // CADB_BUFA[0,3] are already at correct value for victim
          //
        }
        pointer++;
      }
      victimGroupNum++;
      pointer++;
    }
  }

#endif //LPDDR4_SUPPORT

  victimGroupNum--;  // Over-count by 1 during loops
  if (victimGroupNum == rotationN) {
    *pRotationDone = TRUE;   // Pass back out that you've hit the last group
  } else {
    *pRotationDone = FALSE;
  }
#if LPDDR3_SUPPORT
  //
  // Write the CPGC CADB pattern registers
  //
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_ROW_COL]  = CADB_BUFA[0].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_ROW_COL]  = CADB_BUFA[1].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_ROW_COL]  = CADB_BUFA[2].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_ROW_COL]  = CADB_BUFA[3].Bits.row_col_addr ;
    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFA, NO_PRINT, Channel);
  }
#endif

#if LPDDR4_SUPPORT
  //
  // Write the CPGC CADB pattern registers
  //
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA4_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA5_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA6_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA7_BANK]     = 0;
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_ROW_COL]  = CADB_BUFA[0].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_ROW_COL]  = CADB_BUFA[1].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_ROW_COL]  = CADB_BUFA[2].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_ROW_COL]  = CADB_BUFA[3].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA4_ROW_COL]  = CADB_BUFA[4].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA5_ROW_COL]  = CADB_BUFA[5].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA6_ROW_COL]  = CADB_BUFA[6].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA7_ROW_COL]  = CADB_BUFA[7].Bits.row_col_addr ;
    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFA, NO_PRINT, Channel);
  }
#endif



#if DDR3_SUPPORT
  if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TYPE_DDR3) {
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_BANK]     = CADB_BUFA[0].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_BANK]     = CADB_BUFA[1].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_BANK]     = CADB_BUFA[2].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_BANK]     = CADB_BUFA[3].Bits.bank_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA0_ROW_COL]  = CADB_BUFA[0].Bits.row_col_addr;
    ModMrcData->DynamicVars[Channel][CADB_BUFA1_ROW_COL]  = CADB_BUFA[1].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA2_ROW_COL]  = CADB_BUFA[2].Bits.row_col_addr ;
    ModMrcData->DynamicVars[Channel][CADB_BUFA3_ROW_COL]  = CADB_BUFA[3].Bits.row_col_addr ;

    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFA, 0x00D, Channel);

    ModMrcData->DynamicVars[Channel][CADB_BUFB0_CMD]      = CADB_BUFB[0].Bits.cmd;
    ModMrcData->DynamicVars[Channel][CADB_BUFB1_CMD]      = CADB_BUFB[1].Bits.cmd;
    ModMrcData->DynamicVars[Channel][CADB_BUFB2_CMD]      = CADB_BUFB[2].Bits.cmd;
    ModMrcData->DynamicVars[Channel][CADB_BUFB3_CMD]      = CADB_BUFB[3].Bits.cmd;

    MmrcExecuteTask (ModMrcData, CPGC_CADB1_BUFB, 0x00D, Channel);
  }
#endif
  return MMRC_SUCCESS;
}


MMRC_STATUS
L_CpgcInit(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{

    MmrcExecuteTask (MrcData, CPGC_ONE_TIME_INIT, NO_PRINT, Channel);
    return MMRC_SUCCESS;

}

MMRC_STATUS
L_CpgcEnable(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{
    MmrcExecuteTask (MrcData, CPGC_ENABLE, NO_PRINT, Channel);
    return MMRC_SUCCESS;
}

MMRC_STATUS
L_CpgcDisable(
  IN MMRC_DATA         *MrcData,
  IN UINT16            CapsuleStartIndex,
  IN UINT16            StringIndex,
  IN UINT8             Channel
)
{
    MmrcExecuteTask (MrcData, CPGC_DISABLE, NO_PRINT, Channel);
    return MMRC_SUCCESS;
}

#endif
