/** @file
  This file contain functions for parsing the DIMM population and
  define the DRAM and CPU ODT settings.

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

//
// Disable warning for unsued input parameters
//
#pragma warning(disable : 4100)

#include "MemOdt.h"
#include "BxtOdtValueTable.h"
#include "BxtOdtActivationTable.h"

#if ODT_TABLE
//---------------------------------------------------------------
/**
  LookupOdtValue

  @param[in,out]  MrcData
  @param[in]  Node
  @param[in]  Channel

  @retval  ODT_VALUE_STRUCT
**/
ODT_VALUE_STRUCT *
LookupOdtValue (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8                 Node,
  IN        UINT8        Channel
)
{
  ODT_VALUE_INDEX   Config;
  ODT_VALUE_STRUCT  *OdtValueTablePtr;
  UINT8             Dimm;
  UINT8             Slot;
  UINT32            i;
  UINT8             Rank;


  OdtValueTablePtr = (ODT_VALUE_STRUCT *)OdtValueTable;
  Slot = EMPTY_DIMM;
  //
  // Construct config index
  //
  Config.Data = 0;
  Config.Bits.Freq = MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency;
  if (Config.Bits.Freq > DDR3_1866) {
    Config.Bits.Freq = DDR3_1866;
  }
  for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[Dimm]) {
      //
      // Currently no support for load reduced DIMMs
      //
      if (0) {
        Slot = LR_DIMM;
      } else {
        //
        // How many ranks are we dealing with for this DIMM?
        //
        switch (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[Dimm]) {
        case 0:
          Slot = SR_DIMM;
          break;
        case 1:
          Slot = DR_DIMM;
          break;
        case 3:
          Slot = QR_DIMM;
          break;
        default:
          MrcDeadLoop();
        }
      }
    } else {
      Slot = EMPTY_DIMM;
    }

    switch (Dimm) {
    case 0:
      Config.Bits.Slot0 = Slot;
      break;
    case 1:
      Config.Bits.Slot1 = Slot;
      break;
    case 2:
      Config.Bits.Slot2 = Slot;
      break;
    default:
      MrcDeadLoop();
    }
  }
  //
  // AVN platform is 2DPC
  //
  Config.Bits.Slot2 = EMPTY_DIMM;
  //
  // Now lookup the config...
  //
  for (i = 0; i < MAX_ODT_VALUE_CONFIG ; i++) {
    if (OdtValueTablePtr[i].Config == Config.Data) {
      break;
    }
  }
  //
  // Check for error
  //
  if (i >= MAX_ODT_VALUE_CONFIG) {
    return NULL;
  }
  MmrcDebugPrint ((MMRC_DBG_MAX, "Channel %d\n", Channel));
  MmrcDebugPrint ((MMRC_DBG_MAX, "Config.Freq %d\n", Config.Bits.Freq));
  MmrcDebugPrint ((MMRC_DBG_MAX, "Config.Slot0 %d\n", Config.Bits.Slot0));
  MmrcDebugPrint ((MMRC_DBG_MAX, "Config.Slot1 %d\n", Config.Bits.Slot1));
  MmrcDebugPrint ((MMRC_DBG_MAX, "Config.Slot2 %d\n", Config.Bits.Slot2));
  for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {
    for (Rank = 0; Rank < MAX_RANKS_PER_DIMM; Rank++) {
      MmrcDebugPrint ((MMRC_DBG_MAX, "DramOdt[%d][%d] = %02x\n", Dimm, Rank, OdtValueTable[i].DramOdt[Dimm][Rank]));
    }
  }
  //
  // Return pointer
  //
  return &OdtValueTable[i];
}

/**
  LookupOdtAct

  @param[in,out]  MrcData
  @param[in]  Node
  @param[in]  Channel
  @param[in]  ThisDimm
  @param[in]  ThisRank

  @retval  ODT_ACT_STRUCT
**/
ODT_ACT_STRUCT *
LookupOdtAct (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8                 Node,
  IN        UINT8        Channel,
  IN        UINT8                 ThisDimm,
  IN        UINT8                 ThisRank
)
{
  ODT_ACT_INDEX   Config;
  ODT_ACT_STRUCT  *OdtActTablePtr;
  UINT8           Dimm;
  UINT8           Slot;
  UINT32          i;

  Slot = EMPTY_DIMM;
  OdtActTablePtr = (ODT_ACT_STRUCT *)OdtActTable;
  //
  // Construct config index
  //
  Config.Data = 0;
  Config.Bits.DimmNum = ThisDimm;
  Config.Bits.RankNum = ThisRank;
  for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[Dimm]) {
      //
      // Currently no support for low reduced dimms
      //
      if (0) {
        Slot = LR_DIMM;
        Config.Bits.RankNum = 0;
      } else {
        switch (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[Dimm]) {
        case 0:
          Slot = SR_DIMM;
          break;
        case 1:
          Slot = DR_DIMM;
          break;
        case 2:
          Slot = QR_DIMM;
          break;
        default:
          MrcDeadLoop();
        }
      }
    } else {
      Slot = EMPTY_DIMM;
    }

    switch (Dimm) {
    case 0:
      Config.Bits.Slot0 = Slot;
      break;
    case 1:
      Config.Bits.Slot1 = Slot;
      break;
    case 2:
      Config.Bits.Slot2 = Slot;
      break;
    default:
      MrcDeadLoop();
    }
  }
  //
  // Now lookup the config...
  //
  for (i = 0; i < MAX_ODT_ACT_CONFIG; i++) {
    if (OdtActTablePtr[i].Config == Config.Data) {
      break;
    }
  }
  //
  // Check for error
  //
  if (i >= MAX_ODT_ACT_CONFIG) {
    return NULL;
  }
  //
  // Return pointer
  //
  return &OdtActTable[i];
}

//---------------------------------------------------------------
/**
  IsTermRank

  @param[in,out]  MrcData
  @param[in]  Node
  @param[in]  Channel
  @param[in]  ThisDimm
  @param[in]  ThisRank

  @retval  UINT32
**/
UINT32
IsTermRank (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8                 Node,
  IN        UINT8        Channel,
  IN        UINT8                 ThisDimm,
  IN        UINT8                 ThisRank
)
{
  ODT_ACT_STRUCT  *OdtActPtr;
  UINT8           Dimm;
  UINT8           Rank;
  //
  // Check for non-target termination on this rank. Currently no support for load reduced.
  //
  if ((ThisRank < 2) && (0 /* load reduced present */) ) {
    return 1;
  }
  for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {
    for (Rank = 0; Rank < MAX_RANK_DIMM; Rank++) {

      if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[ (2 * Dimm) + Rank]) {
        continue;
      }
      //
      // Skip target rank
      //
      if ((Dimm == ThisDimm) && (Rank == ThisRank) ) {
        continue;
      }
      //
      // Lookup ODT activations for this config
      //
      OdtActPtr = LookupOdtAct (MrcData, Node, Channel, Dimm, Rank);

      if (OdtActPtr != NULL) {
        if ((OdtActPtr->ActBits[0] & (BIT0 << ((ThisDimm * MAX_RANK_DIMM) + ThisRank) ) ) ||
             (OdtActPtr->ActBits[1] & (BIT0 << ((ThisDimm * MAX_RANK_DIMM) + ThisRank) ) ) ) {
          return 1;
        }
      }
    }
  }

  return 0;
}

//---------------------------------------------------------------
/**
  Set MCODT value to 60 or 120 ohms

  @param[in,out]  MrcData  Pointer to host structure
  @param[in]      Node     Socket number
  @param[in]      Channel  Channel number (0-based)
  @param[in]      McOdt    0 = 60 ohms; 1 = 120 ohms (HW default)

  @retval  None
**/
VOID
SetMcOdtValue (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8                 Node,
  IN        UINT8        Channel,
  IN        UINT8                 McOdt
)
{
  if (McOdt == MCODT_60) {
    MrcData->DynamicVars[Channel][PULLUP_EXT] = 0x1A;
  } else {
    MrcData->DynamicVars[Channel][PULLUP_EXT] = 0x25;
  }
  MrcData->DynamicVars[Channel][PULLDOWN_INT] = 0xC;
}


//---------------------------------------------------------------
/**
  Programs the ODT matrix

  @param[in,out]  MrcData  Pointer to host structure
  @param[in]      Node     Socket number
  @param[in]      Channel  Channel number (0-based)
  @param[in,out]  MrsValue

  @retval  None
**/
VOID
SetOdtMatrix (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Node,
  IN        UINT8        Channel,
  IN  OUT   UINT32       MrsValue[MAX_RANKS][MR_COUNT]
)
{
  UINT8                 Dimm;
  UINT8                 Rank;
  UINT8                 TableVal;
  UINT32                RdOdtTableData;
  UINT32                WrOdtTableData;
  ODT_VALUE_STRUCT      *OdtValuePtr;

  //
  // Lookup ODT values for this config
  //
  OdtValuePtr = LookupOdtValue (MrcData, Node, Channel);
  if (OdtValuePtr == NULL){
    MmrcErrorPrint ((MMRC_DBG_MAX, "ODT Ptr is NULL at Node:%d Channel:%d\n", Node, Channel));
    MrcDeadLoop ();
  }
  //
  // Set MCODT values for this channel
  //
  SetMcOdtValue (MrcData, Node, Channel, OdtValuePtr->McOdt);
  //
  // Loop for each DIMM
  //
  for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {
    //
    // Skip if no DIMM present
    //
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[Dimm]) {
      continue;
    }

    RdOdtTableData  = 0;
    WrOdtTableData  = 0;

    //
    // Loop for each rank
    //
    for (Rank = 0; Rank < MAX_RANK_DIMM; Rank++) {
      //
      // Skip if no rank
      //
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[ (2 * Dimm) + Rank] == 0) {
        continue;
      }
      //
      // Get the ODT values
      //
      TableVal = OdtValuePtr->DramOdt[Dimm][Rank];

#ifdef  LRDIMM_SUPPORT
      if (host->nvram.common.imc[node].channelList[ch].lrDimmPresent) {

        // LRDIMM uses SPD data for DRAM Rtt_nom and Rtt_wr fields
        if (rank < 2) {
          tableVal = ((*dimmNvList) [dimm].spdLrBuf_MR1_2_RTT >> 2) & 0x37;
        } else {
          // DRAM Rtt_nom must be disabled for ranks > 1
          tableVal = ((*dimmNvList) [dimm].spdLrBuf_MR1_2_RTT >> 2) & 0x30;
        }
      }
#endif  //LRDIMM_SUPPORT

      //
      // Set encodings for DRAM Mode Registers
      //
      MrsValue[(2 * Dimm) + Rank][2] = 0;
      if ((TableVal & 0xF0) ==  (RTT_WR_60 << 4)) {
        MrsValue[(2 * Dimm) + Rank][2] |= (RTT_WR_ODT_60 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_WR_60\n", Channel, Dimm, Rank));
      } else if ((TableVal & 0xF0) == (RTT_WR_120 << 4)) {
        MrsValue[(2 * Dimm) + Rank][2] |= (RTT_WR_ODT_120 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_WR_120\n", Channel, Dimm, Rank));
      }
      MrsValue[(2 * Dimm) + Rank][1] = 0;
      if ((TableVal & 0x0F) == RTT_NOM_60) {
        MrsValue[(2 * Dimm) + Rank][1] |= (RTT_NOM_ODT_60 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_NOM_ODT_60 \n", Channel, Dimm, Rank));
      } else if ((TableVal & 0x0F) == RTT_NOM_120) {
        MrsValue[(2 * Dimm) + Rank][1] |= (RTT_NOM_ODT_120 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_NOM_ODT_120 \n", Channel, Dimm, Rank));
      } else if ((TableVal & 0x0F) == RTT_NOM_20) {
        MrsValue[(2 * Dimm) + Rank][1] |= (RTT_NOM_ODT_20 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_NOM_ODT_20 \n", Channel, Dimm, Rank));
      } else if ((TableVal & 0x0F) == RTT_NOM_40) {
        MrsValue[(2 * Dimm) + Rank][1] |= (RTT_NOM_ODT_40 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_NOM_ODT_40 \n", Channel, Dimm, Rank));
      } else if ((TableVal & 0x0F) == RTT_NOM_30) {
        MrsValue[(2 * Dimm) + Rank][1] |= (RTT_NOM_ODT_30 << 6);
        MmrcDebugPrint ((MMRC_DBG_MAX, "C%dD%dR%d RTT_NOM_ODT_30 \n", Channel, Dimm, Rank));
      }
#ifdef SIM
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmCount == 1) {
        MrsValue[(2 * Dimm) + Rank][1] = (RTT_WR_ODT_60 << 6);
      } else {
        MrsValue[(2 * Dimm) + Rank][1] = (RTT_WR_ODT_120 << 6);
      }
      MrsValue[(2 * Dimm) + Rank][1] = 0;
#endif
    } // target rank
  } // target dimm
}

#endif // ODT_TABLE
