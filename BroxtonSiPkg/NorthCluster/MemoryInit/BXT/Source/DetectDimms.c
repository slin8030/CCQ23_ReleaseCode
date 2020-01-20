/** @file
  Memory Detection and spd programming.

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

#include "DetectDimms.h"
#include "MmrcProjectData.h"
#include "OemHooks.h"

//[-start-160929-IB05400712-add]//
#include <Library/ReportStatusCodeLib.h>
//[-end-160929-IB05400712-add]//

UINT8 mDdr3RelevantSpdBytes[] = {
  SPD_DDR3_TYPE           ,      ///< 2 Memory type
  SPD_DDR3_MODULE         ,      ///< 3 Module type (3:0)
  SPD_DDR3_DENBANK        ,      ///< 4 Density (6:4), Banks (3:0)
  SPD_DDR3_ADDRESS        ,      ///< 5 Row (5:3), Column (2:0) address
  SPD_DDR3_VDD            ,      ///< 6 Module Nominal Voltage
  SPD_DDR3_ORG            ,      ///< 7 Ranks (5:3),device width (2:0)
  SPD_DDR3_WIDTH          ,      ///< 8 Bus width ext (4:3), Bus width (2:0)
  SPD_DDR3_MTBDD          ,      ///< 10 Medium Timebase (MTB) Dividend
  SPD_DDR3_MTBDS          ,      ///< 11 Medium Timebase (MTB) Divisor
  SPD_DDR3_TCLK           ,      ///< 12 Minimum cycle time (tCKmin)
  SPD_DDR3_CLL            ,      ///< 14 CAS latency supported, low byte
  SPD_DDR3_CLH            ,      ///< 15 CAS latency supported, high byte
  SPD_DDR3_TAA            ,      ///< 16 Minimum CAS latency time (tAA)
  SPD_DDR3_TWR            ,      ///< 17 Minimum write recovery time (tWR)
  SPD_DDR3_TRCD           ,      ///< 18 Minimum RAS to CAS time (tRCD)
  SPD_DDR3_TRRD           ,      ///< 19 Minimum RA to RA time (tRRD)
  SPD_DDR3_TRP            ,      ///< 20 Minimum precharge time (tRP)
  SPD_DDR3_TRASRC         ,      ///< 21 Upper nibbles for tRAS (7:4), tRC (3:0)
  SPD_DDR3_TRAS           ,      ///< 22 Minimum active to precharge (tRAS)
  SPD_DDR3_TRC            ,      ///< 23 Minimum active to active/refresh (tRC)
  SPD_DDR3_TRFCL          ,      ///< 24 Minimum refresh recovery (tRFC), low byte
  SPD_DDR3_TRFCH          ,      ///< 25 Minimum refresh recovery (tRFC), high byte
  SPD_DDR3_TWTR           ,      ///< 26 Minimum internal wr to rd cmd (tWTR)
  SPD_DDR3_TRTP           ,      ///< 27 Minimum internal rd to pc cmd (tRTP)
  SPD_DDR3_TFAWH          ,      ///< 28 Upper Nibble for tFAW
  SPD_DDR3_TFAWL          ,      ///< 29 Minimum Four Activate Window Delay Time (tFAWmin), Least Significant Byte
  SPD_DDR3_TCCD           ,      ///< 32 The minimum delay, in DRAM clocks, between 2 RD/WR commands
  SPD_DDR3_TWTP           ,      ///< 33 The minimum delay, in DRAM clocks, between a WR command and a PREcommand to the same bank
  SPD_DDR3_TWCL           ,      ///< 34 The delay, in DRAM clocks, between the internal write command and the availability of the first bit of DRAM input data
  SPD_DDR3_TCMD           ,      ///< 35 The time period, in DRAM clocks, that a command occupies the DRAM command bus.
  SPD_DDR3_TCK_FTB        ,      ///< 34 Fine Offset for SDRAM Minimum Cycle Time (tCKmin)
  SPD_DDR3_TAA_FTB        ,      ///< 35 Fine Offset for Minimum CAS Latency Time (tAAmin)
  SPD_DDR3_TRCD_FTB       ,      ///< 36 Fine Offset for Minimum RAS# to CAS# Delay Time (tRCDmin)
  SPD_DDR3_TAA_FTB        ,      ///< 37 Minimum Row Precharge Delay Time (tRPmin)
  SPD_DDR3_TAA_FTB        ,      ///< 38 Fine Offset for Minimum Active to Active/Refresh Delay Time (tRCmin)
  SPD_DDR3_RAW_CARD_ID    ,      ///< 62 Raw Card ID
  SPD_DDR3_ADD_MAPPING    ,      ///< 63 Address Mapping (Odd Rank Mirror)
  SPD_DDR3_MANUFACTURER_ID_LO,   ///<117
  SPD_DDR3_MANUFACTURER_ID_HI,   ///<118
  SPD_DDR3_MANUFACTURE_LOCATION, ///<119
  SPD_DDR3_MANUFACTURE_DATE_LO,  ///<120
  SPD_DDR3_MANUFACTURE_DATE_HI,  ///<121
  SPD_DDR3_SERIAL_NUMBER_1,      ///<122
  SPD_DDR3_SERIAL_NUMBER_2,      ///<123
  SPD_DDR3_SERIAL_NUMBER_3,      ///<124
  SPD_DDR3_SERIAL_NUMBER_4,      ///<125
//[-start-170511-IB15550030-add]//
    128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, // Module part number
//[-end-170511-IB15550030-add]//
};

/**
  mFrequencyMultiplier is represented
  with a 0.001 ns granularity
**/
UINT16 mFrequencyMultiplier[C_MAXDDR] = {
  2500,  // 800MHz
  1875,  // 1066MHz
  1500,  // 1333MHz
  1250,  // 1600MHz (1.250 ns)
  1071,  // 1866MHz (1.071 ns)
   938,  // 2133MHz (0.938 ns)
   833,  // 2400MHz (0.833 ns)
   750,  // 2666MHz (0.750 ns)
   625,  // 3200MHz
};

/**
  mTrfcExpected is represented with a 0.001 ns granularity.
  Relationship is between density vs time (ns)
**/
UINT32 mTrfcExpected[4] = {
  160000,  // 260000, 2Gb (260 ns)
  300000,  // 260000, 4Gb (260 ns)
  350000,  // 8Gb (350 ns)
  0,       // 16Gb (DNV TODO - UNKNOWN!!)
};

/**
  mTaaMin is represented with
  a 0.001 ns granularity. 1st dimension is DDR3. 2nd is DDR4.
**/
UINT16 mTaaMin[C_MAXDDR][2] = {
  {10000, 12500},  // 1600MHz (10.000 ns) DDR3/DDR4
  {10700, 12850},  // 1866MHz (10.700 ns) DDR3/DDR4
  {0, 13130},      // 2133MHz (13.130 ns) DDR4 only
  {0, 12500},      // 2400MHz (12.500 ns) DDR4 only
  {0, 0}           // @todo 2666MHz (10.000 ns), DDR4 spec not complete yet.
};

UINT8 mErrorMessages[MrcMsgMaxError][50] = {
  "All Right",
  "DDR Type Not Supported",
  "Module Type Not Supported",
  "Device Width Not Supported",
  "Dimm 0 MUST be populated on at least 1 channel",
  "Number of Banks Not Supported",
  "Device Density Not Supported",
  "SODIMMS in a channel must have same # of ranks",
  "Maximum of 2 Ranks Supported",
  "No Dimms Detected",
  "CL * TCK > 20ns",
  "Null Pointer Error",
  "Memory Timing Violation (tWR, tRP, etc)",
  "Mixing x8 & x16 DIMMs",
  "DIMM does not support selected frequency",
  "Non-ECC DIMM detected. Unsupported",
  "CAS Latency Not Supported",
  "Unknown Failure",
  "Non-ECC SODIMM detected. Unsupported",
  "Both channel must be populated"
};

UINT8 mDimmVoltageErrorMessages[MrcDimmMaxError][60] = {
  "Voltage Dimm configuration is OK",
  "Illegal configuration, detected 1.25V and 1.5V Dimms",
  "Illegal configuration, detected 1.25/1.35V and 1.5V Dimms",
  "Illegal configuration, detected an 1.25/1.35/1.5V Dimm"
};

MRC_TASK_FUNCTION mDetectDimmTasks[] = {
  &IdentifyDimms,
  &EnforceCommonFreq,
  &FindTclTacTClk,
  &FindTrasTrpTrcd,
  &ConvertDimmParams,
  NULL
};

STRUCT_TIMING_DATA mTrasTrpTrcdTbl[10] = {
  { TimingTras, MAX_SUPPORTED_TRAS, MIN_SUPPORTED_TRAS, SPD_DDR3_TRAS,  SPD_DDR3_TRASRC, 0x0F, 0xFF, "TRAS"},
  { TimingTrp,  MAX_SUPPORTED_TRP,  MIN_SUPPORTED_TRP,  SPD_DDR3_TRP,   0,               0,      37, "TRP"},
  { TimingTrcd, MAX_SUPPORTED_TRCD, MIN_SUPPORTED_TRCD, SPD_DDR3_TRCD,  0,               0,      36, "TRCD"},
  { TimingTwr,  MAX_SUPPORTED_TWR,  MIN_SUPPORTED_TWR,  SPD_DDR3_TWR,   0,               0,    0xFF, "TWR"},
  { TimingTrfc, MAX_SUPPORTED_TRFC, MIN_SUPPORTED_TRFC, SPD_DDR3_TRFCL, SPD_DDR3_TRFCH,  0xFF, 0xFF, "TRFC"},
  { TimingTwtr, MAX_SUPPORTED_TWTR, MIN_SUPPORTED_TWTR, SPD_DDR3_TWTR,  0,               0,    0xFF, "TWTR"},
  { TimingTrrd, MAX_SUPPORTED_TRRD, MIN_SUPPORTED_TRRD, SPD_DDR3_TRRD,  0,               0,    0xFF, "TRRD"},
  { TimingTrrdl,MAX_SUPPORTED_TRRD, MIN_SUPPORTED_TRRD, SPD_DDR3_TRRD,  0,               0,    0xFF, "TRRDL"},
  { TimingTrtp, MAX_SUPPORTED_TRTP, MIN_SUPPORTED_TRTP, SPD_DDR3_TRTP,  0,               0,    0xFF, "TRTP"},
  { TimingTfaw, MAX_SUPPORTED_TFAW, MIN_SUPPORTED_TFAW, SPD_DDR3_TFAWL, SPD_DDR3_TFAWH,  0x0F, 0xFF, "TFAWH"},
};


UINT16 mTones[8] = {NOTE (26163), NOTE (29366), NOTE (32963), NOTE (34923), NOTE (39200), NOTE (44000), NOTE (49388), NOTE (26163 * 2) };

/**
  PrintMemoryError

  @param[in]  MrcData
  @param[in]  IndexError

  @retval  None
**/
VOID
PrintMemoryError (
  IN        MMRC_DATA   *MrcData,
  IN        UINT8        IndexError
)
{
  MmrcDebugPrint ((MMRC_DBG_MIN, "Error[%d]: %s\n", IndexError, (UINT8 *) &mErrorMessages[IndexError][0]));
  return;
}

VOID
PrintBinaryBuffer (
  IN        MMRC_DATA   *MrcData,
  IN        UINT8*      Buffer,
  IN        UINT32      BufferSize
)
{
  UINT32    CurrentByte = 0;

  if (BufferSize == 0) {
    return;
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "Base  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n"));
  MmrcDebugPrint ((MMRC_DBG_MIN, "%4d %2x ", CurrentByte/16, Buffer[0]));
  for (CurrentByte = 1; CurrentByte < BufferSize; CurrentByte++) {
    if ((CurrentByte % 16) == 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n%4d %2x ", CurrentByte/16, Buffer[CurrentByte]));
    } else {
      MmrcDebugPrint ((MMRC_DBG_MIN, "%2x ", Buffer[CurrentByte]));
    }
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
  return;
}


/**
  This function determines if coldboot is required because DIMMs have been changed
  in between boots.

  @param[in]  MrcData   Host structure for all MRC global data.

  @retval  TRUE         Coldboot required
  @retval  FALSE        Coldboot NOT required
**/
BOOLEAN
  CheckColdBootRequired (
  IN  OUT   MMRC_DATA   *MrcData
  )
{
  UINT8       CurrentDimm;
  SPD_DATA    *CurrentSpdData;
  UINT8       i;
  UINT8       *pData8;
  BOOLEAN     coldbootRequired;
  UINT8       Channel;

  coldbootRequired = FALSE;

  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
    if ( GetSpdData (
      MrcData,
      Channel,
      &mDdr3RelevantSpdBytes[0],
      ((sizeof (mDdr3RelevantSpdBytes) ) / (sizeof (mDdr3RelevantSpdBytes[0]))) ) == MMRC_SUCCESS) {
        for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
          if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm] == 0) {
            if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FastBootData[CurrentDimm].ManuIDlo == 0) &&
              (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FastBootData[CurrentDimm].ManuIDhi == 0)) {
                continue;
            } else {
              return TRUE;
            }
          }
          CurrentSpdData = & (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
          pData8 = (UINT8 *) (&(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FastBootData[CurrentDimm]) );

          for (i = 0; i < (sizeof (FASTBOOTDATA) - 1); i++) {
            MmrcDebugPrint ((
              MMRC_DBG_MIN,
              "[%d] FBD: 0x%02x CMD: 0x%02x\n",
              i,
              *(pData8 + i),
              CurrentSpdData->Buffer[SPD_DDR3_MANUFACTURER_ID_LO + i]
            ));
            if ((*(pData8 + i)) != CurrentSpdData->Buffer[SPD_DDR3_MANUFACTURER_ID_LO + i]) {
              coldbootRequired = TRUE;
              return coldbootRequired;
            }
          }

          if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FastBootData[CurrentDimm].DimmCount != MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount) {
            MmrcDebugPrint ((
              MMRC_DBG_MIN,
              "DimmCount FBD: %d CMD: %d\n",
              MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FastBootData[CurrentDimm].DimmCount,
              MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount
              ));
            coldbootRequired = TRUE;
          }

        } //end of for CurrentDimm
    } // end of if
  } //end of channel
  return coldbootRequired;
}



/**
  This function detects the memory configuration using the data gotten from the SPD (Serial Presence Detect)

  @param[in]  MrcData    Pointer to MRC Output Data that contains MRC Parameters

  @retval  MMRC_SUCCESS  Memory detection was successful.
  @retval  Others        Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
DetectDimms (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  UINTN       i;
  MMRC_STATUS Status;
  UINT8       Channel;
  UINT8       DimmCount;

  Channel   = 0;
  DimmCount = 0;
  Status    = MMRC_SUCCESS;

  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {

    Status = GetSpdData (
               MrcData,
               Channel,
               &mDdr3RelevantSpdBytes[0],
               ((sizeof (mDdr3RelevantSpdBytes) ) / (sizeof (mDdr3RelevantSpdBytes[0])))
             );

    if (Status == MMRC_FAILURE) {
      MrcDeadLoop ();
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount == 0) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled  = FALSE;
    } else {
      DimmCount++;
    }
  } // Channel loop ...

  if ((DimmCount == 0)&&(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_SODIMM)) {
    PrintMemoryError (MrcData, (UINT8) MrcMsgNoDimmsPresent);
//[-start-160929-IB05400712-add]//
    REPORT_STATUS_CODE ((EFI_ERROR_CODE|EFI_ERROR_MAJOR), (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_EC_NONE_DETECTED));
//[-end-160929-IB05400712-add]//
    MrcDeadLoop ();
    return MMRC_FAILURE;
  }

  if ((DimmCount == 0) && (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_CH0_MD_CH1_SOD)) {
    // no Sodimm but has memory down
    return MMRC_SUCCESS;
  }
  for (i = 0; mDetectDimmTasks[i] != NULL; i++) {
    MmrcDebugPrint ((MMRC_DBG_MAX, "DetectDimms[%d], Channel %d\n", i, Channel));
    Status = mDetectDimmTasks[i] (MrcData);
    if (Status != MMRC_SUCCESS) {
      PrintMemoryError (MrcData, (UINT8) Status);
      MrcDeadLoop ();
      return MMRC_FAILURE;
    }
  } // mDetectDimmTasks loop ...

  return Status;
}


/**
  This function uses SPD data to detect the presence of unbuffered DDR DIMMs

  @param[in]  MrcData  Host structure for all MRC global data.

  @retval  MMRC_SUCCESS  Memory identify was successful.
  @retval  Others        Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
IdentifyDimms (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  UINT8            CurrentDimm;
  SPD_DATA        *CurrentSpdData;
  UINT8            Ranks;
  UINT8            DimmPopulation;
  UINT8            Voltage;
  UINT8            Channel;
  UINT8            ColumnBits;
  UINT8            MaxPageSize;

  Voltage        = 0xFF;
  Ranks          = 0;
  DimmPopulation = 0;
  ColumnBits     = 9;
  MaxPageSize    = 0;

  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
    //reinitlaize just incase booted from FB and change to S5
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0] = FALSE;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1] = FALSE;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled = 0;
    for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
      CurrentSpdData = & (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
      if (CurrentSpdData->SpdPresent == FALSE) {
        ///
        /// No DIMM present...no need to proceed with SPD decode
        ///
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[CurrentDimm*2] = FALSE;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[CurrentDimm*2+1] = FALSE;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled = 0;

        continue;
      }
      DimmPopulation++;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[CurrentDimm * 2] = TRUE;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled = 1;
      ///
      ///;-----------------------------------------------------------------------;
      ///  STEP 4: Verify that all DIMMs present are DDR2 or DDR3 DIMMs.
      ///   See Section 5.2.1 for information on the SDRAM Type field of the SPD.
      ///;-----------------------------------------------------------------------;
      ///
      switch (CurrentSpdData->Buffer[SPD_DDR3_TYPE]) {
      case SPD_DDR3:
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType = TypeDdr3;
        break;
      default:
        return MrcMsgDdrTypeNotSupported;
        break;
      }
      //
      //get voltage
      //
      Voltage &= (CurrentSpdData->Buffer[SPD_DDR3_VDD] & (BIT2|BIT1|BIT0));

      //
      //check for DDR3/3L/3U
      //
      switch (Voltage) {
      case 0:
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType = TypeDdr3;
        MmrcDebugPrint ((MMRC_DBG_MIN, "C%d.D%d: DDR3 SDRAM Memory type\n",Channel, CurrentDimm));
        break;

      case 2:
      case 3:
      case 6:
      case 7:
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType = TypeDdr3L;
        MmrcDebugPrint ((MMRC_DBG_MIN, "C%d.D%d: DDR3L SDRAM Memory type\n",Channel, CurrentDimm));
        break;
      default:
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType = TypeDdr3L;
        MmrcDebugPrint ((MMRC_DBG_MIN, "C%d.D%d: DDR3 SDRAM Memory type\n",Channel, CurrentDimm));
        break;
      }

      ///
      ///;-----------------------------------------------------------------------;
      ///  STEP 5: Verify that all DIMMs present are either UDIMM, SO-DIMM, or
      ///   SO-DIMM with ECC.
      ///    See Section 5.2.1 for information on the SDRAM Type field of the SPD.
      ///;-----------------------------------------------------------------------;
      /// Allow Non-ECC SODIMMs because some DIMMs have their SPDs programmed as non-ECC
      /// even when they are ECC SODIMMs.  Since we don't want non-ECC SODIMMS
      /// installed, we'll catch these on the data width checks (64 vs. 72 bits).
      ///
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Type[CurrentDimm] = CurrentSpdData->Buffer[SPD_DDR3_MODULE] & SPD_DDR3_MTYPE_MASK;

      if (((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Type[CurrentDimm]) !=  SPD_DDR3_SODIMM_ECC) &&
        ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Type[CurrentDimm]) !=  SPD_DDR3_SODIMM) &&
        ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Type[CurrentDimm]) !=  SPD_DDR3_UNBUFFERED)) {
          return MrcMsgModuleTypeNotSupported;
      }

      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[CurrentDimm] = (CurrentSpdData->Buffer[SPD_DDR3_ORG] & 0x7) - 1;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Banks[CurrentDimm] = (CurrentSpdData->Buffer[SPD_DDR3_DENBANK] & 0x70) >> 4;
      // BXT supports a minimum of 2Gb density DRAM. Turncate the smaller density SPD byte values by subtracting 3.
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size_SPD[CurrentDimm] = (CurrentSpdData->Buffer[SPD_DDR3_DENBANK] & 0xF) - JEDEC_SPD_BYTE4_DRAM_DENSITY_2Gb;
      Ranks = (CurrentSpdData->Buffer[SPD_DDR3_ORG] >> 3) & 0x07;

      MrcData->EnableAddressMirroring = CurrentSpdData->Buffer[SPD_DDR3_ADD_MAPPING];
      ///
      /// Check for non-ECC DIMM. All DIMMs must be ECC or non-ECC in VLV platform, so 1 non-ECC DIMM
      /// is enough to disable ECC for the entire platform. AVN supports both ECC and non-ECC DIMMs.
      ///
      if ((CurrentSpdData->Buffer[SPD_DDR3_WIDTH] & SPD_DDR3_BWE_MASK) == 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = FALSE;
      } else {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = TRUE;
        MmrcDebugPrint ((MMRC_DBG_MIN, "Channel %d: DDR3 ECC SDRAM Memory type\n",Channel));
      }
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_BusWidth[CurrentDimm] = (CurrentSpdData->Buffer[SPD_DDR3_WIDTH] & 0x7);


      //Page size calculation
      ColumnBits = 9 + (CurrentSpdData->Buffer[SPD_DDR3_ADDRESS] & 0x7);
      MaxPageSize = (UINT8)(((1 << ColumnBits) * ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[CurrentDimm]+1) *8)/8)/1024);
      if (MaxPageSize > MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize) {
        MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize = MaxPageSize;
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "ColumnBits %d, MaxPageSize = %d, pagesize %d\n", ColumnBits, MaxPageSize, MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize));


      //
      // Return with an error if the DRAM density is unsupported
      // Supported DRAM desities are:
      // Density | SPD Byte Value - 3
      // 2Gb     | 0x0 (requires software workaround)
      // 4Gb     | 0x1
      // 8Gb     | 0x2
      //
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size_SPD[CurrentDimm] > 0x2) {
        return MrcMsgDDenNotSupported;
      }

      if (CurrentSpdData->Buffer[SPD_DDR3_STROPT] & BIT2) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ASR_Supported = 1;
      } else {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ASR_Supported = 0;
      }

      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[CurrentDimm] = Ranks;
      if (Ranks > 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[ (CurrentDimm * 2) + 1] = TRUE;
      }

      if (Ranks > 1) {
        return MrcMsgMoreDrNotSupported;
      }


    } // CurrentDimm loop ...
  }// channel loop

  return MMRC_SUCCESS;
}
/*++

Routine Description:

  This function enforces the common frequency across channels

Arguments:

  MrcData:   Host structure for all MRC global data.

Returns:

  MMRC_SUCCESS          - Timing comply with JEDEC spec.
  Others           - Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error

--*/
MMRC_STATUS
EnforceCommonFreq (
  IN  OUT   MMRC_DATA     *MrcData
)

{
#if DDR3_SUPPORT || DDR4_SUPPORT
  UINT8             CurrentDimm;
  UINT16            DimmFreq;
  UINT8             Channel;
  SPD_DATA          *CurrentSpdData;
  UINT16            tckminall;
  UINT16            tck;
  UINT16            i;
  UINT16            MtbInPs;

  //
  // CL7 through CL18 supported by the DUNIT.
  //
  MrcData->SupportedCasLat = 0x7FF8;

  // Get Fuse high and low freq.
  GetSupportedFrequencies (MrcData, &MrcData->FreqHi);

  MmrcDebugPrint((MMRC_DBG_MIN, "Punit Cap1 freqhi: %d\n", MrcData->FreqHi));

  //;-----------------------------------------------------------------------;
  //   Step 10: Determine a common frequency and CAS latency that can be supported.
  //;-----------------------------------------------------------------------;
  tckminall = 0;
  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount == 0) {
      continue;
    }
    for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
      CurrentSpdData = & (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm]) {

        MtbInPs = 1000 * CurrentSpdData->Buffer[SPD_DDR3_MTBDD] / CurrentSpdData->Buffer[SPD_DDR3_MTBDS];
        tck = (CurrentSpdData->Buffer[SPD_DDR3_TCLK] * MtbInPs) + (INT8) CurrentSpdData->Buffer[SPD_DDR3_TCK_FTB];
        MmrcDebugPrint ((MMRC_DBG_MIN, "tckcalculated: %d, tck spd %d, tck_ftb spd: %d, \n", tck,CurrentSpdData->Buffer[SPD_DDR3_TCLK], CurrentSpdData->Buffer[SPD_DDR3_TCK_FTB]));

        if (tck > tckminall) {
          tckminall = tck;
        }
      }
    } // CurrentDimm loop ...
  } // Channel loop
  //
  // Align to JEDEC standard tCK value
  //
  for (i = MrcData->FreqHi; (char) i >= DDR3L_MINDDR; i--) {
    if (tckminall <= mFrequencyMultiplier[i]) {
      break;
    }
  }
  DimmFreq = i;
  MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Common Frequency - DIMM capability: %d\n", DimmFreq));
  if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile >= DDR3_1333_7_7_7 ) && (ConfigToSpeed[MrcData->MrcMemConfig] < DimmFreq)){
     DimmFreq = ConfigToSpeed[MrcData->MrcMemConfig];
     MmrcDebugPrint ((MMRC_DBG_MIN, "DDR downgrade Frequency: %d\n", DimmFreq));
  }

  //
  // Determine DIMM Freq
  //
  MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = DimmFreq;

  //
  // Report results
  //
  switch (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency) {
  case MMRC_1333:
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Frequency : 1333\n"));
    break;
  case MMRC_1600:
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Frequency : 1600\n"));
    break;
  case MMRC_1866:
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Frequency : 1866\n"));
    break;
  case MMRC_2133:
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Frequency : 2133\n"));
    break;
  case MMRC_2400:
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Frequency : 2400\n"));
    break;
  case MMRC_2666:
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR Frequency : 2666\n"));
    break;
  default:
    break;
  }
#endif
  return MMRC_SUCCESS;
}

/**
  This function uses SPD data to determine the timings for the memory channel

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Timing comply with JEDEC spec.
  @retval  Others           Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
FindTclTacTClk (
  IN  OUT   MMRC_DATA     *MrcData
)
{
  UINT8             CurrentDimm;
  UINT16            MyCasLat;
  INT8              LowCasLat;
  SPD_DATA          *CurrentSpdData;
  UINT8             CLdesired;
  UINT16            tckminall;
  UINT16            taaminall;
  UINT16            taa;
  UINT16            MtbInPs;
  UINT8             Channel;

  ///
  /// Now determine Common CL value
  ///
  taaminall = 0;
  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
      CurrentSpdData = &(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm]) {
        MtbInPs = 1000 * CurrentSpdData->Buffer[SPD_DDR3_MTBDD] / CurrentSpdData->Buffer[SPD_DDR3_MTBDS];
        taa = (CurrentSpdData->Buffer[SPD_DDR3_TAA] * MtbInPs) + (INT8) CurrentSpdData->Buffer[SPD_DDR3_TAA_FTB];
        MmrcDebugPrint ((MMRC_DBG_MIN, "SPD: taa %d, tck %d, \n", CurrentSpdData->Buffer[SPD_DDR3_TAA], CurrentSpdData->Buffer[SPD_DDR3_TCLK]));
        if (taa > taaminall) {
          taaminall = taa;
        }
      }
    }
  }// channel
  //
  // Desired CL is taa/tck rounded up to nearest integer.
  //
  tckminall = mFrequencyMultiplier[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency];
  CLdesired = (UINT8) (taaminall / tckminall);
  if (taaminall % tckminall) {
    CLdesired++;
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "taaminall %d, tckminall %d, CLdesired %d\n", taaminall, tckminall, CLdesired));

  //;-----------------------------------------------------------------------;
  //   STEP 9: Verify a common CAS latency is supported by all DIMMs and the DUNIT.
  //    See Section 5.2.1 for information on the SDRAM Type field of the SPD.
  //;-----------------------------------------------------------------------;
  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount == 0) {
      continue;
    }
    for (CurrentDimm = 0;  CurrentDimm <  MAX_DIMMS; CurrentDimm++) {
      CurrentSpdData = & (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm]) {
        MrcData->SupportedCasLat &= ((CurrentSpdData->Buffer[SPD_DDR3_CLH] << 8) | CurrentSpdData->Buffer[SPD_DDR3_CLL]);
        //  MmrcDebugPrint ((MMRC_DBG_MIN, "Spd: CLH %x, CLL %x, SuuportedCL: %x\n", CurrentSpdData->Buffer[SPD_DDR3_CLH],CurrentSpdData->Buffer[SPD_DDR3_CLL],MrcData->SupportedCasLat));
      }
    }
  } // Channel loop
  if (MrcData->SupportedCasLat == 0) {
    return MrcMsgCasLatencyNotSupported;
  }

  ///
  /// Round desired CL up to next supported value.
  ///
  LowCasLat = 0;
  MyCasLat = MrcData->SupportedCasLat;
  while (MyCasLat != 0) {
    LowCasLat = BitScanReverse16 (MyCasLat) + 4;
    if (LowCasLat < 4) {    ///< if BitScanReverse16() returned -1
      MmrcDebugPrint ((MMRC_DBG_MIN, "BitScanReverse16 failed.  Probably CAS passed in was 0.\n"));
      return MrcMsgCasLatencyNotSupported;
    }
    if ((UINT8) LowCasLat == CLdesired) {
      break;
    }
    //
    // Don't need to error check this instance of BitScanReverse8() because it is identical to the one above.
    //
    MyCasLat &= ~ (UINT16) (1 << BitScanReverse16 (MyCasLat) );
  }
  //
  // Save Common CL value
  //
  //MrcData->DynamicVars[Channel][RL] = (UINT8) LowCasLat;
  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Tcl = LowCasLat;

    MmrcDebugPrint ((MMRC_DBG_MIN, "CH%d TCL %d\n", Channel, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Tcl ));
  }
  return MMRC_SUCCESS;
}

/**
  This function uses SPD data to determine timings.

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Timing comply with JEDEC spec.
  @retval  Others           Refer to UINT8 mErrorMessages[][] table and enum DETECT_DIMMS_ERROR to identify error
**/
MMRC_STATUS
FindTrasTrpTrcd (
  IN  OUT   MMRC_DATA     *MrcData
)
{
  UINT8                      CurrentDimm;
  SPD_DATA                  *CurrentSpdData;
  UINT8                      FoundGoodValue;
  UINT8                      j;
  UINT16                     i;
  UINT32                     Temp;
  UINT32                     MinSPDVal[10];
  STRUCT_TIMING_DATA         *TimingCalcPtr;
  UINT32                     ProposedTime;
  UINT16                      DdrFreqInx;
  UINT16                     MtbInPs;
  UINT8                      Channel;

  TimingCalcPtr = NULL;
  DdrFreqInx = (UINT16) ( MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency - MINDDR);
  MtbInPs = 0;
  i = 0;
  MmrcMemset (MinSPDVal, 0, sizeof (MinSPDVal) );

  //
  // No DIMMs on this DUNIT, no need to calculate timing params.
  //
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }

    for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
      if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm]) {
        //
        // Skip the non-present DIMMs
        //
        continue;
      }
      CurrentSpdData = & (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[CurrentDimm]);
      MtbInPs = 1000 * CurrentSpdData->Buffer[SPD_DDR3_MTBDD] / CurrentSpdData->Buffer[SPD_DDR3_MTBDS];
      for (j = 0; j < 10; j++) {
        Temp  = MtbInPs * ((((CurrentSpdData->Buffer[mTrasTrpTrcdTbl[j].Ddr3HighSpdByte] & mTrasTrpTrcdTbl[j].Ddr3HighSpdByteMask) << 8)
          + CurrentSpdData->Buffer[mTrasTrpTrcdTbl[j].Ddr3LowSpdByte]));
        //
        // Add/subtract the 2's complement FTB (fine adjustment) to this timing param if applicable.
        //
        if (mTrasTrpTrcdTbl[j].Ddr3FtbSpdByte != 0xFF) {
          Temp += (INT8) CurrentSpdData->Buffer[mTrasTrpTrcdTbl[j].Ddr3FtbSpdByte];
        }
        MinSPDVal[j] = MAX (MinSPDVal[j], Temp);

        if (j == TimingTrfc) {
          if (MinSPDVal[j] != mTrfcExpected[MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size_SPD[CurrentDimm]]) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "Warning tRFC requirements are NOT met for C%dD%d\n", Channel, CurrentDimm));
            MmrcDebugPrint ((
              MMRC_DBG_MIN,
              "tRFC: %d tRFC Expected: %d\n",
              MinSPDVal[j],
              mTrfcExpected[MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size_SPD[CurrentDimm]]
            ));
          }
        } // TimingTrfc
      } // MaxTimingData
    } // CurrentDimm loop ...
  } //channel loop

  ///
  ///  STEP 11: Determine the smallest common tRAS for all DIMMs. See Section 5.3.3 for details.
  ///  STEP 12: Determine the smallest common tRP for all DIMMs. See Section 5.3.4 for details.
  ///  STEP 13: Determine the smallest common tRCD for all DIMMs. See Section 5.3.5 for details.
  ///  STEP 15: Verify all DIMMs support burst length of 8. See Section 5.2.1 for details.
  ///  STEP 16: Determine the smallest common tWR for all DIMMs. See Section 5.3.6 for details.
  ///  STEP 17: Determine the smallest common tRFC for all DIMMs. See Section 5.3.7 for details.
  ///  STEP 18: Determine the smallest common tWTR for all DIMMs. See Section 5.3.8 for details.
  ///  STEP 19: Determine the smallest common tRRD for all DIMMs. See Section 5.3.9 for details.
  ///  STEP 20: Determine the smallest common tRTP for all DIMMs. See Section 5.3.10 for details.
  ///
  /// Outer loop j loops though all SPD data timings in TimingDataTbl and
  /// gets them calculated and stored in MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData array.
  ///
  TimingCalcPtr = &mTrasTrpTrcdTbl[0];
  if (TimingCalcPtr == NULL) {
    return MrcMsgNullPtr;
  }

  for (j = 0; j < 10; j++) {
    if (j == TimingTrrdl ) {
      //
      // Skip if TCCD, TWTP, TWCL or TCMD
      // These registers don't exist in SPD bytes or are calculated elsewhere.
      //
      TimingCalcPtr++;
      continue;
    }
    //
    // Inner loop i loops through the min to max supported timing data looking for
    // best possible timing data.
    //
    FoundGoodValue = FALSE;
    if (MinSPDVal[j] != 0) {
      for (i = TimingCalcPtr->Ddr3MinMCHVal; i <= TimingCalcPtr->MaxMchVal; i++) {
        ProposedTime = i * mFrequencyMultiplier[DdrFreqInx];
        if (ProposedTime >= MinSPDVal[j]) {
          FoundGoodValue = TRUE;
          MmrcDebugPrint ((MMRC_DBG_MIN, "%-5s = %d\n", TimingCalcPtr->Name, i));
          break;
        }
      }
    }
    if ((FoundGoodValue == TRUE)) {
      if (j == TimingTrfc) {
        //
        // Special case for tRFC round up to next even number of clocks
        //
        i = (i + 1) & 0xFE;
      }
      for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
          continue;
        }
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[j] = i;
      }
    } else {
      MmrcDebugPrint ((MMRC_DBG_MIN, "%-5s = %d - not a valid value.\n", TimingCalcPtr->Name, i));
      return MrcMsgTimingOutBoundary;
    }

    TimingCalcPtr++;
  } // MaxTimingData loop ...

  return MMRC_SUCCESS;
}
/**
  This function converts the MrcData parameters from SPD values to values that the rest of the MMRC Expects

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  MMRC_SUCCESS     Default return value
**/
MMRC_STATUS
ConvertDimmParams (
  IN  OUT   MMRC_DATA     *MrcData
)
{
  UINT8 Channel;
  //
  // Mapping of density to SPD and variable values
  // Density | JEDEC SPD (-3) | D_Size | D_ActualSize (Density in Gb)
  // 2Gb     | 0x0            | 0      | 2
  // 4Gb     | 0x1            | 0      | 0
  // 8Gb     | 0x2            | 2      | 0
  //
  for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++){
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      switch(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size_SPD[0]) {
        case 0: //2Gb
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity4Gb;
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0] = DRAM_DENSITY_2Gb;
          break;
        case 1: //4Gb
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity4Gb;
          break;
        case 2: //8Gb
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity8Gb;
          break;
        default:
          return MrcMsgDDenNotSupported;
          break;
      }
    }
  }
  return MMRC_SUCCESS;
}

/**
  This function reads SPD data and determines which slots are populated.

  @param[in]  MrcData    Host structure for all MRC global data.
  @param[in]  Channel    Current Channel being examined.
  @param[in]  SpdTable   Return data of SPD bytes.
  @param[in]  TableLen   Length of SPD Table.

  @retval  MMRC_SUCCESS  SPD data gotten correctly
  @retval  Others        Error trying to get SPD data
**/
MMRC_STATUS
GetSpdData (
  IN        MMRC_DATA   *MrcData,
  IN        UINT8       Channel,
  IN        UINT8       *SpdTable,
  IN        UINT8       TableLen
)
{
  MMRC_STATUS    Status;
  UINT8          CurrentDimm;
  UINT8          i;
  SPD_DATA       *SpdData;
  UINT8          Length;

  SpdData = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData;
  MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount = 0;
  MrcData->TotalDimm = 0;
  Length = VF_SC_BYTE_LEN;

  //
  // Now examine each DIMM.
  //
  for (CurrentDimm = 0; CurrentDimm < MAX_DIMMS; CurrentDimm++) {
      Status = MrcSmbusExec (
                 MrcData->SmbusBar,
                 MrcData->OemMrcData.SpdAddressTable[Channel][CurrentDimm],
                 SmbusReadByte,
                 SPD_DDR3_TYPE,
                 &Length,
                 &SpdData[CurrentDimm].Buffer[SPD_DDR3_TYPE]
               );

    if (Status != MMRC_SUCCESS) {
      SpdData[CurrentDimm].SpdPresent = FALSE;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm] = FALSE;
      continue;
    }

    switch (SpdData[CurrentDimm].Buffer[SPD_DDR3_TYPE]) {
    case SPD_DDR3:
      for (i = 0; i < TableLen; i++) {
          Status = MrcSmbusExec (
                     MrcData->SmbusBar,
                     MrcData->OemMrcData.SpdAddressTable[Channel][CurrentDimm],
                     SmbusReadByte,
                     * (SpdTable + i),
                     &Length,
                     &SpdData[CurrentDimm].Buffer[* (SpdTable + i)]
                   );
        MmrcDebugPrint ((MMRC_DBG_MED, "C%d.D%d: SPD byte %d = 0x%X\n", Channel, CurrentDimm, * (SpdTable + i), SpdData[CurrentDimm].Buffer[* (SpdTable + i)]));
      }
      MmrcDebugPrint ((MMRC_DBG_MED, "\n"));
      break;
    default:
      return MMRC_FAILURE;
    };
    PrintBinaryBuffer(MrcData, SpdData[CurrentDimm].Buffer, sizeof(SpdData[CurrentDimm].Buffer));
#if !defined JTAG
    if (Status != MMRC_SUCCESS) {
#else
    if (Status != MMRC_SUCCESS || CurrentDimm > 0) {
#endif
      SpdData[CurrentDimm].SpdPresent = FALSE;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm] = FALSE;
    } else {
      SpdData[CurrentDimm].SpdPresent = TRUE;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[CurrentDimm] = TRUE;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount++;
      MrcData->TotalDimm++;
    }
  }

  return MMRC_SUCCESS;
}

#if BDAT
/**
  This function provides access to the relevant SPD bytes array.
  Needed by Bdat.c.

  @param[in]   MrcData                Host structure for all MRC global data.
  @param[out]  Ddr3RelevantSpdBytes   Returned pointer to the SPD byte array.
  @param[out]  NumElements            Number of elements in the SPD byte array.

  @retval  None
**/
VOID
GetDdr3RelevantSpdBytes (
  IN        MMRC_DATA   *MrcData,
  OUT   UINT8       **Ddr3RelevantSpdBytes,
  OUT   UINT32      *NumElements
)
{
  if ((Ddr3RelevantSpdBytes == NULL) || (NumElements == NULL)) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "GetDdr3RelevantSpdBytes: Null pointer(s) passed.\n"));
  } else {
    *Ddr3RelevantSpdBytes = mDdr3RelevantSpdBytes;
    *NumElements = ((sizeof (mDdr3RelevantSpdBytes)) / (sizeof (mDdr3RelevantSpdBytes[0])));
  }
  return;
}
#endif // BDAT

