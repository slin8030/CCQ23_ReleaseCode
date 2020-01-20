/** @file
  This file contains Initialization of the BDAT data structures for both
  v1.0 and v2.0 taken from v0.6 of "A Compatible Method to Access BIOS
  Data Structures".

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

#include "McFunc.h"
#include "DetectDimms.h"
#include "Bdat.h"
#include "RmtResult.h"
#include "MmrcProjectData.h"

///
/// Flags to control support options (to turn off, comment them out)
///
#define BDAT_REV2_SUPPORT 1

const EFI_GUID mMetadataGuid = RMT_RESULT_METADATA_GUID;
const EFI_GUID mColsGuid = RMT_RESULT_COLUMNS_GUID;

#define RX_DQ_INDEX                  1
#define TX_DQ_INDEX                  2
#define RX_VREF_INDEX                3
#define TX_VREF_INDEX                4
#define CMD_VREF_INDEX               5

#define RESULTS_RX_DQ_INDEX          0
#define RESULTS_TX_DQ_INDEX          1
#define RESULTS_RX_VREF_INDEX        2
#define RESULTS_TX_VREF_INDEX        3
#define RESULTS_CMD_VREF_INDEX       1

#define LOW_INDEX                    0
#define HIGH_INDEX                   1

#define NUMBER_RMT_GROUPS_CREATE  1

#ifdef MmrcBdatDebugPrint
VOID
PrintBdatresult (
  RMT_RESULT_COLUMNS     *RmtRows,
  UINT32                 ElementNumber
)
{
  RMT_RESULT_COLUMNS   *RmtRow;
  UINT8                i;
  UINT8                j;

  RmtRow = &(RmtRows[ElementNumber]);
  MmrcBdatDebugPrint ((MMRC_DBG_MIN, "i: %d, CH:%d R:% d Ty: %d L: %d - [", ElementNumber, RmtRow->Header.Channel, RmtRow->Header.RankA, RmtRow->Header.ResultType, RmtRow->Header.Lane));
  for (i = 0; i < 4; i++) {
    MmrcBdatDebugPrint ((MMRC_DBG_MIN, "["));
    for (j = 0; j < 2; j++) {
      MmrcBdatDebugPrint ((MMRC_DBG_MIN, " %d", RmtRow->Margin[i][j]));
    }
    MmrcBdatDebugPrint ((MMRC_DBG_MIN, " ]"));
  }
  MmrcBdatDebugPrint ((MMRC_DBG_MIN, "]\n"));

}
#define PRINT_BDAT_RESULT(Arguments) PrintBdatresult Arguments
#else
#define PRINT_BDAT_RESULT(Arguments)
#endif

UINT32
GetNumberRowsRmtResult (
  IN  OUT   MMRC_DATA         *MrcData
)
{
  UINT8   MaxStrobe;
  UINT32  NumberRows;
  UINT8   Channel;
  UINT8   Rank;

  NumberRows = 0;
  for (Channel = 0; Channel < MrcData->MaxNumberChannels; Channel++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        MaxStrobe = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank];
        NumberRows += NUMBER_RMT_GROUPS_CREATE * MAX_BITS * MaxStrobe;
        NumberRows += 1;//for rank RxDqsdelay, TxDqDelay, RxVref, TxVref
        NumberRows += 1;//for rank CmdAll, CmdVref, CtlAll
      }
    }
  }
  return NumberRows;
}

UINT32
GetRMTStructSize (
  IN  OUT   MMRC_DATA         *MrcData
)
{
  return sizeof(RESULTS_DATA_HDR) + sizeof(RMT_RESULT_METADATA) + (sizeof(RMT_RESULT_COLUMNS) * GetNumberRowsRmtResult(MrcData));
}

/**
  This function fills in both v1.0 and v2.0 of the BDAT data structure per
  v0.6 of "A Compatible Method to Access BIOS Data Structures"

  @param[in,out]  Host      The Host structure for all MRC global data.
  @param[out]     HostBDAT  The BDAT structure to be initialized.

  @retval  None

**/
VOID
FillRMTBdatStructure (
  IN  OUT   MMRC_DATA         *MrcData,
      OUT   VOID              *RmtDataBuffer,
  IN        UINT32            RmtDataBufferSize
)
{
  UINT8                  Channel;
  UINT8                  Rank;
#if RMT_PER_BIT_MARGIN
  UINT8                  Strobe;
  UINT8                  Bit;
#endif
  UINT8                  MaxStrobe;
  INT32                  MaxNumberRows;
  BASE_RMT_RESULT        *RmtResults;
  RMT_RESULT_COLUMNS     *RmtRow;

  RmtResults = (BASE_RMT_RESULT*)RmtDataBuffer;

  MmrcMemset (RmtDataBuffer, 0, RmtDataBufferSize);
  Mmrcmemcpy((UINT8*)(&RmtResults->ResultsHeader.MdBlock.MetadataType), (UINT8*)(&mMetadataGuid), sizeof(RmtResults->ResultsHeader.MdBlock.MetadataType));
  RmtResults->ResultsHeader.MdBlock.MetadataSize = sizeof(RMT_RESULT_METADATA);
  Mmrcmemcpy((UINT8*)(&RmtResults->ResultsHeader.RsBlock.ResultType), (UINT8*)(&mColsGuid), sizeof(RmtResults->ResultsHeader.RsBlock.ResultType));
  RmtResults->ResultsHeader.RsBlock.ResultElementSize = sizeof(RMT_RESULT_COLUMNS);
  MaxNumberRows = RmtDataBufferSize - sizeof(RESULTS_DATA_HDR) - sizeof(RMT_RESULT_METADATA);
  MaxNumberRows = MaxNumberRows / sizeof(RMT_RESULT_COLUMNS);
  RmtResults->ResultsHeader.RsBlock.ResultCapacity = MaxNumberRows;
  RmtResults->ResultsHeader.RsBlock.ResultElementCount = 0;
  RmtResults->Metadata.EnableCtlAllMargin       = 0;
  RmtResults->Metadata.SinglesBurstLength       = 13;
  RmtResults->Metadata.SinglesLoopCount         = 10;
  RmtResults->Metadata.TurnaroundsBurstLength   = 0;
  RmtResults->Metadata.TurnaroundsLoopCount     = 0;
  RmtResults->Metadata.ScramblerOverrideMode    = DontTouchScrambler;
  RmtResults->Metadata.PiStepUnit[0]            = 16;
  RmtResults->Metadata.RxVrefStepUnit[0]        = 520;
  RmtResults->Metadata.TxVrefStepUnit[0][0]     = 533;
  RmtResults->Metadata.CmdVrefStepUnit[0][0]    = 16;
  RmtResults->Metadata.MajorVer                 = 0;
  RmtResults->Metadata.MinorVer                 = 5;
  RmtRow = &(RmtResults->Rows[0]);

  for (Channel = 0; Channel < MrcData->MaxNumberChannels; Channel++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank])) {
        continue;
      }
      MmrcBdatDebugPrint ((MMRC_DBG_MIN, "Ch: %d, R: %d\n", Channel, Rank));
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.Channel = Channel;
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.RankA = Rank;
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.IoLevel = IO_LEVEL_DDR_LEVEL;
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.ResultType = Rank0RmtResultType; //RxDqsdelay, TxDqDelay, RxVref, TxVref
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_DQ_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RX_DQ_INDEX][LOW_INDEX]);
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_DQ_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RX_DQ_INDEX][HIGH_INDEX]);
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_DQ_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TX_DQ_INDEX][LOW_INDEX]);
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_DQ_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TX_DQ_INDEX][HIGH_INDEX]);
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_VREF_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RX_VREF_INDEX][LOW_INDEX]);
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_VREF_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RX_VREF_INDEX][HIGH_INDEX]);
      if ((0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TX_VREF_INDEX][LOW_INDEX]) > 0xff) {
        RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_VREF_INDEX][LOW_INDEX] = 0xff;
      } else {
        RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_VREF_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TX_VREF_INDEX][LOW_INDEX]);
      }
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_VREF_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TX_VREF_INDEX][HIGH_INDEX]);
      PRINT_BDAT_RESULT((RmtRow, RmtResults->ResultsHeader.RsBlock.ResultElementCount));
      RmtResults->ResultsHeader.RsBlock.ResultElementCount += 1;
      if (RmtResults->ResultsHeader.RsBlock.ResultElementCount >= MaxNumberRows) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "BDAT: Not enough space for results.\n"));
        return;
      }
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.Channel = Channel;
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.RankA = Rank;
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.IoLevel = IO_LEVEL_DDR_LEVEL;
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.ResultType = Rank1RmtResultType; //CmdAll, CmdVref, CtlAll
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_CMD_VREF_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][CMD_VREF_INDEX][LOW_INDEX]);
      RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_CMD_VREF_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][CMD_VREF_INDEX][HIGH_INDEX]);
      PRINT_BDAT_RESULT((RmtRow, RmtResults->ResultsHeader.RsBlock.ResultElementCount));
      RmtResults->ResultsHeader.RsBlock.ResultElementCount += 1;
      if (RmtResults->ResultsHeader.RsBlock.ResultElementCount >= MaxNumberRows) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "BDAT: Not enough space for results.\n"));
        return;
      }
      MaxStrobe = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank];
#if RMT_PER_BIT_MARGIN
      for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.Channel = Channel;
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.RankA = Rank;
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.IoLevel = IO_LEVEL_DDR_LEVEL;
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.Lane = Bit + Strobe * MAX_BITS; ////TODO: Check if this is correct
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Header.ResultType = LaneRmtResultType; //RxDqsdelay, TxDqDelay, RxVref, TxVref
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_DQ_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RX_DQ_INDEX][Bit + Strobe * MAX_BITS][LOW_INDEX]);
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_DQ_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RX_DQ_INDEX][Bit + Strobe * MAX_BITS][HIGH_INDEX]);
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_DQ_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][TX_DQ_INDEX][Bit + Strobe * MAX_BITS][LOW_INDEX]);
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_DQ_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][TX_DQ_INDEX][Bit + Strobe * MAX_BITS][HIGH_INDEX]);
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_VREF_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RX_VREF_INDEX][Bit + Strobe * MAX_BITS][LOW_INDEX]);
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_RX_VREF_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RX_VREF_INDEX][Bit + Strobe * MAX_BITS][HIGH_INDEX]);
          if ((0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][TX_VREF_INDEX][Bit + Strobe * MAX_BITS][LOW_INDEX]) > 0xff) {
            RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_VREF_INDEX][LOW_INDEX] = 0xff;
          } else {
            RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_VREF_INDEX][LOW_INDEX] = (UINT8) (0 - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][TX_VREF_INDEX][Bit + Strobe * MAX_BITS][LOW_INDEX]);
          }
          RmtRow[RmtResults->ResultsHeader.RsBlock.ResultElementCount].Margin[RESULTS_TX_VREF_INDEX][HIGH_INDEX] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][TX_VREF_INDEX][Bit + Strobe * MAX_BITS][HIGH_INDEX]);
          PRINT_BDAT_RESULT((RmtRow, RmtResults->ResultsHeader.RsBlock.ResultElementCount));
          RmtResults->ResultsHeader.RsBlock.ResultElementCount += 1;
          if (RmtResults->ResultsHeader.RsBlock.ResultElementCount >= MaxNumberRows) {
            return;
          }
        }
      }
#endif
    }
  }
  RmtResults->Metadata.ResultEleCount = (UINT16) RmtResults->ResultsHeader.RsBlock.ResultElementCount;
} // FillBdatStructure


#ifdef BDAT_CRC_SUPPORT
/**
  This function returns the CRC16 of the provided array.

  @param[in]  Ptr    The array to be processed.
  @param[in]  Count  Number of elements in the array.

  @retval  CRC16 of the provided array.
**/
INT16
Crc16 (
  IN INT8  *Ptr,
  IN INT32 Count
)
{
  INT16 crc, i;
  crc = 0;
  while (--Count >= 0) {
    crc = crc ^ (INT16) (int) * Ptr++ << 8;
    for (i = 0; i < 8; ++i) {
      if (crc & 0x8000) {
        crc = crc << 1 ^ 0x1021;
      } else {
        crc = crc << 1;
      }
    }
  }
  return (crc & 0xFFFF);
}

#endif  //  BDAT
