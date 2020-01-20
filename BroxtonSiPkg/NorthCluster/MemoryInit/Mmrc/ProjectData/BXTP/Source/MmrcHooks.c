/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2018 Intel Corporation.

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
/*++

Module Name:

  MmrcHooks.h

Abstract:

  Modular MRC calls functions in this file which perform tasks
  which are outside the realm of DDRIO initialization. Things
  like print routines, DUNIT routines which the MMRC needs,
  JEDEC init, etc, will be placed here. The routines in this
  file should NOT reference any host structure besides MMRC_DATA
  since this file is build in the MMRC standalone mode which
  does not include any MRC dependencies outside the MMRC.

--*/
#if _SVBIOS_
#include "McFunc.h"
#endif
#if CAR
#include "Delay.h"
#endif
#include "MmrcHooks.h"
#include "MmrcProjectData.h"
#include "MmrcLibraries.h"
#include "cpgcapi.h"

//[-start-160216-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160216-IB07400702-add]//

#if SIM || JTAG
#include <conio.h>
#endif
//extern UINT32 WrVRef[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];;

// Signals to be restored in RestoreAlgos
RESTORE_DATA_DESCRIPTOR RestoreDataDescriptorTable[] = {
  { CmdGrp0           , RANK_BASED                , TRUE  },
//{ CmdGrp1           , RANK_BASED                , TRUE  },
  { CtlGrp0           , RANK_BASED                , TRUE  },
//{ CtlGrp1           , RANK_BASED                , TRUE  },
//{ CkGrp0            , RANK_BASED                , TRUE  },
//{ CkGrp1            , RANK_BASED                , TRUE  },
//{ RxDqsDelay        , STROBE_BASED | RANK_BASED , TRUE },
  { RxDqsNDelay       , STROBE_BASED | RANK_BASED , TRUE },
  { RxDqsPDelay       , STROBE_BASED | RANK_BASED , TRUE },
  { RxDqBitDelay      , STROBE_BASED|BIT_BASED    , TRUE },
//{ TxDqBitDelay      , STROBE_BASED|BIT_BASED    , TRUE },
  { RxVref            , STROBE_BASED | RANK_BASED , TRUE },
  { TxDqsDelay        , STROBE_BASED | RANK_BASED , TRUE },
  { TxDqDelay         , STROBE_BASED | RANK_BASED , TRUE },
//{ TxDqDelay_Actual  , STROBE_BASED | RANK_BASED , TRUE },
  { TxVref            , RANK_BASED                , TRUE },
  { RecEnDelay        , STROBE_BASED | RANK_BASED , TRUE },
  { OdtSegEnOff       , STROBE_BASED              , TRUE },
  { OdtEnOff          , STROBE_BASED              , TRUE },
  { OdtSegEnOff_Ecc   , UNIQUE                    , TRUE },
  { OdtEnOff_Ecc      , UNIQUE                    , TRUE },
  { CkeAll            , RANK_BASED                , TRUE },
// These are handled in SetupMaxPI
//  { MuxcodeMin        , STROBE_BASED  , TRUE },
//  { MuxcodeMax        , STROBE_BASED  , TRUE },
#if RxVocVal0
  { RxVocVal0         , STROBE_BASED   , TRUE },
  { RxVocVal1         , STROBE_BASED   , TRUE },
  { RxVocVal2         , STROBE_BASED   , TRUE },
  { RxVocVal3         , STROBE_BASED   , TRUE },
  { RxVocVal4         , STROBE_BASED   , TRUE },
  { RxVocVal5         , STROBE_BASED   , TRUE },
  { RxVocVal6         , STROBE_BASED   , TRUE },
  { RxVocVal7         , STROBE_BASED   , TRUE },
  { RxVocEnDq         , STROBE_BASED   , TRUE },
  { RxVocEnDqs        , STROBE_BASED   , TRUE },
#endif
#if RxVocVal0_Ecc
  { RxVocVal0_Ecc     , UNIQUE       , TRUE },
  { RxVocVal1_Ecc     , UNIQUE       , TRUE },
  { RxVocVal2_Ecc     , UNIQUE       , TRUE },
  { RxVocVal3_Ecc     , UNIQUE       , TRUE },
  { RxVocVal4_Ecc     , UNIQUE       , TRUE },
  { RxVocVal5_Ecc     , UNIQUE       , TRUE },
  { RxVocVal6_Ecc     , UNIQUE       , TRUE },
  { RxVocVal7_Ecc     , UNIQUE       , TRUE },
  { RxVocEnDq_Ecc     , UNIQUE       , TRUE },
  { RxVocEnDqs_Ecc    , UNIQUE       , TRUE },
#endif
// INITIAL_INDEX is handled by PeriodicTraining
//  { INITIAL_INDEX     , STROBE_BASED |RANK_BASED, FALSE },
// MuxcodeMax is handled by "SetupMaxPI"
//  { MuxcodeMax        , STROBE_BASED | RANK_BASED , FALSE },
  { RDCMD2RDVLD_REG   , STROBE_BASED , TRUE },
  { RDCMD2RDVLD_PLUS1SEL, STROBE_BASED , TRUE},
  { TimingMode, UNIQUE, TRUE},
  { RdRk2RkLatency, STROBE_BASED, TRUE},
  { TrrdrGetSet   , UNIQUE      , TRUE },
  { TwwdrGetSet   , UNIQUE      , TRUE },
#if defined(RdRk2RkLatency_Ecc)
  { RdRk2RkLatency_Ecc, UNIQUE, TRUE }
#endif

};
UINT32 RestoreDataDescriptorTableCount = sizeof(RestoreDataDescriptorTable) / sizeof(RESTORE_DATA_DESCRIPTOR);

char *MAXPI_STATE_NAME[]={
  "MAXPI_INIT",
  "MAXPI_DLLINIT",
  "MAXPI_TXDLL_ERROR",
  "MAXPI_CHECK_ACROSS_FAMILY",
  "MAXPI_SETUP_WIDER_RANGE",
  "MAXPI_FINAL_VALUES",
  "MAXPI_ERROR"
};

/*++

Routine Description:

  Save/Restores platform specific trained values.

Arguments:

  *MrcData           :  Main MRC data structure.

Returns:

  Nothing

--*/
VOID
ProjectSpecificSaveRestores (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  SAVE_RESTORE_TASK Task
  )
{
  REGISTER_ACCESS Register;

  if(Task == SAVE) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
      Register.Mask=0xFFFFFFFF;
      Register.ShiftBit=0;
      Register.Offset = PTCTL0_DQDPLVUG_BOXTYPE_REG;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].PtCtl0 = (UINT32)MemRegRead(DQDPLVUG_BOXTYPE, Channel, 0, Register);
    }
  }
}

/**
  Performs Periodic Training for LP4.
  calibration.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PeriodicTraining (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if PERIODIC_TRAINING == 1 && TRAINING_ALGOS == 1
#define PERIODIC_BUCKETS 70
#define PERIODIC_AUTOMATIC_INITIAL_INDEX 0
  //if (MrcData->DynamicVars[Channel][BXT_PERIODIC_ENABLED] != 0)
  //{
    UINT8   Rank;
    UINT32  TempValue, AnotherTempValue;
    REGISTER_ACCESS Register;
    UINT16 RO1;
    UINT16 RO2;
    UINT32  RunTimeInterval;
    UINT16  InitialValue;
    UINT16  StepSize;
    UINT16  Multiplier;
    UINT16  NumPi;
    UINT32 PTLENGTH = 140 * 1000; //150->300
    //INT32  i;
    UINT32 Dq2x, Drvdb;
    UINT8  Strobe;
    UINTX  Value;
    UINT8  ch;
    UINT16 ro_table[PERIODIC_BUCKETS];
    BOOLEAN RestorePath = (MrcData->BootMode != S5) ? TRUE : FALSE;
    PTCTL0_DQDPLVUG_BOXTYPE_STRUCT * Ptrctl0;
   // UINT8   BitSwizzle[8] = {};

    Rank = 0;
    TempValue = 0, AnotherTempValue=0;
    RunTimeInterval = 0;
    InitialValue = 190;     //excel
    StepSize = 10;          //excel
    Multiplier = 2;
    Register.Mask   = 0xFFFFFFFF;
    Register.ShiftBit = 0;
    ch = Channel;
    MrcData->MrcDebugMsgLevel=MMRC_DBG_TYPICAL;

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4 && MrcData->PeriodicRetrainingDisable == FALSE) {
      //FillChannelBitMapping(MrcData, 0, 0, ch);
      CalculateDeswizzleReverseMap(MrcData, ch);
      CalculateMasterFloorplan(MrcData, ch);
      SetupPeriodicDDRIOSwizzle(MrcData, ch);

      //
      // send mr23 ~150ns in 16 dram clk
      //
      for (Rank = 0; Rank < MAX_RANKS; Rank++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          continue;
        }
        TempValue = (PTLENGTH) / TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] / 16;
        MmrcDebugPrint ((MMRC_DBG_MIN, "C=%d R=%d MR23: 0x%x\n", ch, Rank, TempValue));
        MrcData->DynamicVars[Channel][RANK]        = Rank;
        MrcData->DynamicVars[ch][REG_DATA] = MRW_LP4(23, TempValue);
        MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, ch);
        MmrcDebugPrint ((MMRC_DBG_MIN, "Runtime Internal osc %d \n", TempValue * TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] * 16));
        MySimStall(2000);
      }

      //
      // Find the Runtime Interval to the closest Clock
      //
      RunTimeInterval = (UINT32) (PTLENGTH / (GetOneClk(MrcData, ch) ));
      RunTimeInterval = RunTimeInterval * (GetOneClk(MrcData, ch));
      MmrcDebugPrint ((MMRC_DBG_MIN, "Runtime Internal %d PerClk %d\n", RunTimeInterval, GetOneClk(MrcData, ch)));
      //
      // Get NumPi
      //
      Register.Offset = PTCTL0_DQDPLVUG_BOXTYPE_REG;
      Value = MemRegRead (DQDPLVUG_BOXTYPE, ch, 0, Register);
      Ptrctl0 = (PTCTL0_DQDPLVUG_BOXTYPE_STRUCT *) &Value;
      Ptrctl0->Bits.numpi_perindex = 1;
      MemRegWrite(DQDPLVUG_BOXTYPE, ch, 0, Register, Value, 0xFF);

      Value = MemRegRead(DQDPLVUG_BOXTYPE, ch, 1, Register);
      Ptrctl0 = (PTCTL0_DQDPLVUG_BOXTYPE_STRUCT *) &Value;
      Ptrctl0->Bits.numpi_perindex = 1;
      MemRegWrite(DQDPLVUG_BOXTYPE, ch, 1, Register, Value, 0xFF);

      NumPi = (UINT16) Ptrctl0->Bits.numpi_perindex;

      if (NumPi == 0) {
        Multiplier = 1;
      }

      StepSize = Multiplier * ((TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]/2) / (GetHalfClk(MrcData, ch)) );

      for (TempValue=0; TempValue < PERIODIC_BUCKETS; TempValue++)
      {
        ro_table[TempValue] = (UINT16) (RunTimeInterval/ ( 2* ((InitialValue) + (TempValue* StepSize)) ));
      }


      for (TempValue = 0; TempValue < 35; TempValue++) {
        RO1 = ro_table[(TempValue * 2)];     //(UINT16) (RunTimeInterval/ ( 2* ((InitialValue) + ((TempValue*2)       * StepSize))) ); //*2 added to account for full period
        RO2 = ro_table[(TempValue * 2) + 1]; //(UINT16) (RunTimeInterval/ ( 2* ((InitialValue) + (((TempValue*2) + 1) * StepSize))) );
        if (ch == 0)
          MmrcDebugPrint ((MMRC_DBG_MIN, "%02x %02x ", RO1, RO2));
        Register.Offset = PTROCNT12_DQDPLVUG_BOXTYPE_REG + (TempValue*4);
        AnotherTempValue = (RO1<<16)|RO2;
        MemRegWrite (DQDPLVUG_BOXTYPE, ch, 0, Register, AnotherTempValue, 0xF);
        MemRegWrite (DQDPLVUG_BOXTYPE, ch, 1, Register, AnotherTempValue, 0xF);
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
      //MmrcExecuteTask (MrcData, PERIODIC_COPY_RK0_TO_RK1, NO_PRINT, Channel);
      for (Rank = 0; Rank < MAX_RANKS; Rank++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          continue;
        }
        //CpgcVATest (MrcData,NO_PRINT, NO_PRINT, ch);
        //
        // Get Initial Index
        //
        for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++)
        {
          if (Strobe%2 == 0)
          {
            TempValue = GetManualInitialIndex (MrcData, Rank, Channel, ro_table, RestorePath, Strobe/2);
            MmrcDebugPrint((MMRC_DBG_MIN, "C=%d R=%d I=%d CurrentIndex 0x%02x\n", Channel, Rank, Strobe /2, TempValue));
            GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, INITIAL_INDEX, CMD_SET_VAL_FC_UC , &TempValue);
          }
          if (((MrcData->Rk2RkEn.TxDq == 0) && (Rank==0)) || (MrcData->Rk2RkEn.TxDq != 0)) {
            //
            //  Get TxDq deadband, and decide how to change the value
            //
            GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, (GetOffsetIndex (MrcData, TxDqDelay_Actual) - ALGO_REG_INDEX) * NumberOfElementsPerAlgo + 3, CMD_GET_REG, &Drvdb);
            if (Drvdb == 3 || Drvdb == 1)  //We decrement for region 3,1
            {
              //
              // Grp0 2x increment
              //
              MmrcDebugPrint((MMRC_DBG_MIN, "C:%d R:%d S:%d DeadBand is %02d, So adding 1 2x to DQ\n", Channel, Rank, Strobe, Drvdb));
              GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, (GetOffsetIndex (MrcData, TxDqDelay_Actual) - ALGO_REG_INDEX) * NumberOfElementsPerAlgo + 1, CMD_GET_REG, &Dq2x);
              MmrcDebugPrint((MMRC_DBG_MIN, "DQ 2x Value is %d -> ", Dq2x));
              Dq2x += 1;
              MmrcDebugPrint((MMRC_DBG_MIN, " %d\n", Dq2x));
              GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, (GetOffsetIndex (MrcData, TxDqDelay_Actual) - ALGO_REG_INDEX) * NumberOfElementsPerAlgo + 1, CMD_SET_VAL_FC_UC, &Dq2x);
              //
              // Grp6 2x increment
              //
              //GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, TxDqDrive, CMD_GET_REG, &Dq2x);
              GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, (GetOffsetIndex(MrcData, TxDqDrive) - ALGO_REG_INDEX) * NumberOfElementsPerAlgo + 1, CMD_GET_REG, &Dq2x);
              MmrcDebugPrint((MMRC_DBG_MIN, "TxDqDrive Value is %d -> ", Dq2x));
              Dq2x += 1;
              MmrcDebugPrint((MMRC_DBG_MIN, "%d\n", Dq2x));
              //GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, TxDqDrive, CMD_SET_VAL_FC_UC, &Dq2x);
              GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, (GetOffsetIndex(MrcData, TxDqDrive) - ALGO_REG_INDEX) * NumberOfElementsPerAlgo + 1, CMD_SET_VAL_FC_UC, &Dq2x);
            }
            else
            {
              MmrcDebugPrint((MMRC_DBG_MIN, "C:%d R:%d S:%d No Adjustment necessary\n", Channel, Rank, Strobe, Drvdb));
            }
          }
          else
          {
            if ((MrcData->Rk2RkEn.TxDq == 0) && (Rank != 0))
            {
              MmrcDebugPrint((MMRC_DBG_MIN, "C:%d R:%d Skipped adjusting DQ since Rk2Rk is disabled\n", Channel, Rank));
            }
          }
        } //Strobe
      }
      MmrcDebugPrint((MMRC_DBG_MIN, "C=%d enabling Periodic Programming\n", Channel));
      MmrcExecuteTask (MrcData, PERIODIC_TRAINING_EN, NO_PRINT, Channel);

      MmrcDebugPrint((MMRC_DBG_MIN, "Running atleast one Maintainence cycle\n", Channel));
      MmrcExecuteTask (MrcData, PERIODIC_TRAINING_START, NO_PRINT, Channel);
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "TQPOLLEN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][TQPOLLEN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "SRXZQC_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][SRXZQC_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "ZQDIS_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][ZQDIS_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "OREFDIS_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][OREFDIS_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "DYNSREN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][DYNSREN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "DISPWRDN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][DISPWRDN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "DQS_RETAIN_EN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][DQS_RETAIN_EN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "Disabling Maintainence for Periodic\n", Channel));
      MmrcExecuteTask (MrcData, PERIODIC_TRAINING_MNT_DIS, NO_PRINT, Channel);
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "TQPOLLEN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][TQPOLLEN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "SRXZQC_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][SRXZQC_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "ZQDIS_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][ZQDIS_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "OREFDIS_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][OREFDIS_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "DYNSREN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][DYNSREN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "DISPWRDN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][DISPWRDN_SAVE]));
      MmrcPertrainDebugPrint((MMRC_DBG_MAX, "DQS_RETAIN_EN_SAVE=%d\n", (UINT32) MrcData->DynamicVars[Channel][DQS_RETAIN_EN_SAVE]));
    } // LPDDR4
  //}
  //else
  //{
  //  MmrcDebugPrint((MMRC_DBG_MAX, "Ch %d: Disabling Periodic Training as Requested", Channel));
  //}
#endif //PERIODIC_TRAINING
  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetCkGrp0(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  UINT8      GetSetChannel;
  UINT16     GetSetType;

  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, ClkCh0, Cmd, Value);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, ClkCh1, Cmd, Value);
  } else {
    if (Channel == 0) {
      GetSetChannel = 0;
      GetSetType    = ClkCh0;
    } else if (Channel == 1) {
      GetSetChannel = 0;
      GetSetType    = ClkCh1;
    } else if (Channel == 2) {
      GetSetChannel = 2;
      GetSetType    = ClkCh0;
    } else {
      GetSetChannel = 2;
      GetSetType    = ClkCh1;
    }
    GetSetDdrioGroup2 (MrcData, GetSetChannel, Rank, Strobe, GetSetType, Cmd, Value);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetCmdAll(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, CmdCh0, Cmd, Value);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, CmdCh1, Cmd, Value);
  } else {
    if (Channel == 0) {
      GetSetDdrioGroup2 (MrcData, 0, Rank, Strobe, CmdCh0, Cmd, Value);
    } else if (Channel == 1) {
      GetSetDdrioGroup2 (MrcData, 0, Rank, Strobe, CmdCh1, Cmd, Value);
    } else if (Channel == 2) {
      GetSetDdrioGroup2 (MrcData, 2, Rank, Strobe, CmdCh0, Cmd, Value);
    } else {
      GetSetDdrioGroup2 (MrcData, 2, Rank, Strobe, CmdCh1, Cmd, Value);
    }
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetCkeAll(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, CkeCh0, Cmd, Value);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, CkeCh1, Cmd, Value);
  } else {
    if (Channel == 0) {
      GetSetDdrioGroup2 (MrcData, 0, Rank, Strobe, CkeCh0, Cmd, Value);
    } else if (Channel == 1) {
      GetSetDdrioGroup2 (MrcData, 0, Rank, Strobe, CkeCh1, Cmd, Value);
    } else if (Channel == 2) {
      GetSetDdrioGroup2 (MrcData, 2, Rank, Strobe, CkeCh0, Cmd, Value);
    } else {
      GetSetDdrioGroup2 (MrcData, 2, Rank, Strobe, CkeCh1, Cmd, Value);
    }
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetCtlGrp0(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  return GetSetCtlAll(MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, Cmd, Value);
}

MMRC_STATUS
GetSetCmdGrp0(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  return GetSetCmdAll(MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, Cmd, Value);
}

MMRC_STATUS
GetSetCtlAll(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, CtlCh0, Cmd, Value);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, CtlCh1, Cmd, Value);
  } else {
    if (Channel == 0) {
      GetSetDdrioGroup2 (MrcData, 0, Rank, Strobe, CtlCh0, Cmd, Value);
    } else if (Channel == 1) {
      GetSetDdrioGroup2 (MrcData, 0, Rank, Strobe, CtlCh1, Cmd, Value);
    } else if (Channel == 2) {
      GetSetDdrioGroup2 (MrcData, 2, Rank, Strobe, CtlCh0, Cmd, Value);
    } else {
      GetSetDdrioGroup2 (MrcData, 2, Rank, Strobe, CtlCh1, Cmd, Value);
    }
  }
  return MMRC_SUCCESS;
}



MMRC_STATUS
GetSetRecEnDelay(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RecEnDelay_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RecEnDelay_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRecEnSmp(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RecEnSmp_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RecEnSmp_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRxDqsNDelay(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxDqsNDelay_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxDqsNDelay_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRxDqsPDelay(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxDqsPDelay_dq, Cmd, Value);
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "P"));
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxDqsPDelay_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRxDqsDelay(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxDqsPDelay_dq, Cmd, Value);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxDqsNDelay_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxDqsPDelay_Ecc, Cmd, Value);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxDqsNDelay_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRxVocValAll(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocVal0, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxVocVal0_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRxVrefDelay (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVref_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxVref_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetTxDqsDelay (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqsDelay_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, TxDqsDelay_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetWrLvlSmp(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, WrLvlSmp_dq, Cmd, Value);
  } else {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, WrLvlSmp_Ecc, Cmd, Value);
  }

  return MMRC_SUCCESS;
}

/**
  GetSetTxDqBitDelay

  @param[in,out]  MrcData
  @param[in]  Socket
  @param[in]  Channel
  @param[in]  Dimm
  @param[in]  Rank
  @param[in]  Strobe
  @param[in]  Bit
  @param[in]  FrequencyIndex
  @param[in]  IoLevel
  @param[in]  Cmd
  @param[in,out]  Value

  @retval  MMRC_STATUS
**/
MMRC_STATUS
GetSetTxDqBitDelay (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  ) {
    UINT32 TempValue;
    UINT32 Data;
    UINT8 Instance;

    REGISTER_ACCESS     Register;
    Register.Mask = 0xFFFFFFFF;
    Register.ShiftBit = 0;

   //
   // For DDR3, The instance for bytelanes are
   //   0->BL4,5    1-> BL6,7    2->BL0,1    3->BL2,3
   // For LP3/LP4, The instance for bytelanes are
   //  0->BL0,1,   1->BL2,3
    //
   if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
     Instance = (Strobe/2 + 2)%4;
   } else {
     Instance = Strobe/2;
   }
   if ((Strobe%2==0) && ( Bit >=0 && Bit <= 3)) {
     Register.Offset = TXPBDBLCTL0_DQ_BOXTYPE_REG;
   } else  if ((Strobe%2==0) && ( Bit >=4 && Bit <= 7)) {
     Register.Offset = TXPBDBLCTL1_DQ_BOXTYPE_REG;
   } else  if ((Strobe%2==1) && ( Bit >=0 && Bit <= 3)) {
     Register.Offset = TXPBDBLCTL3_DQ_BOXTYPE_REG;
   } else  if ((Strobe%2==1) && ( Bit >=4 && Bit <= 7)) {
     Register.Offset = TXPBDBLCTL4_DQ_BOXTYPE_REG;
   }

   Bit = Bit % 4;

   Data = (UINT32) MemRegRead (DQ_BOXTYPE, Channel, Instance, Register);

      if ( (Cmd & WR_OFF)==WR_OFF) {
    TempValue = (Data>>(Bit*8)) & (0x1f);
    TempValue += (*Value);
    TempValue &= 0x1f;
    Data = (Data & (~(0xff<<(Bit*8))));
    Data = (TempValue) << (Bit*8);
    MemRegWrite(DQ_BOXTYPE, Channel, Instance, Register, Data, 0xF);
      }
   if ( (Cmd==FC_WR)) {
     TempValue = Data & (~(0xff<<(Bit*8)));
     TempValue += (*Value)<<(Bit*8);

     MemRegWrite(DQ_BOXTYPE, Channel, Instance, Register, TempValue, 0xF);
    }


    return MMRC_SUCCESS;
}

/**
  GetSetTxVref

  @param[in,out]  MrcData
  @param[in]  Socket
  @param[in]  Channel
  @param[in]  Dimm
  @param[in]  Rank
  @param[in]  Strobe
  @param[in]  Bit
  @param[in]  FrequencyIndex
  @param[in]  IoLevel
  @param[in]  Cmd
  @param[in,out]  Value

  @retval  MMRC_STATUS
**/
MMRC_STATUS
GetSetTxVref (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{

  UINT8 VrefCoding;
  UINT8 ChannelBackup;
  UINT8 StrobeLoop;
  ChannelBackup = Channel;
#if RTWT_SIM==1
  if ((Cmd&WR_OFF)==WR_OFF) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank] += *Value;
  }
  if (Cmd == FC_WR) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank] = *Value;
  }
  if (Cmd==CMD_GET_REG) {
    *Value = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank];
  }
  return MMRC_SUCCESS;
#endif

  if ((Cmd&WR_OFF) == WR_OFF) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe] += *Value; // rta .. , actually I don't believe "WR_OFF" can ever get to GetSetTxVref, as the offset process (read/add/write) is handled by the 'wrapper' GetSet (as long as no one directly calls GetSetTxVref with WR_OFF)
    *Value = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe];   // rta , adding this line as it looks buggy w/o it, as LP3 uses Value below, not the cache value directly
  }
  if (Cmd == FC_WR) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe] = *Value;
  }
  if (Cmd == CMD_GET_REG || Cmd == RD_REG) {
    *Value = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe];
    return MMRC_SUCCESS;
  }
  //return MMRC_SUCCESS;
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    //
    // The value passed into GetSetTxVref is the percantage of change to Vddq.  This number is in
    // 10ths of a percentage.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe] > 420) {
      //
      // If Value is outside of the range of the programming.
      //
      return MMRC_SUCCESS;
    } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe] < 220) {
      //
      // Use Range 0 if in range.
      //
      VrefCoding = (UINT8)(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe] - 100) / 4;
    } else {
      //
      // Otherwise use Range 1
      //
      VrefCoding = (UINT8)(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe] - 220) / 4;
      //
      // Add the 1 extra bit for range 1 selection.
      //
      VrefCoding += (1 << 6);
    }

    MrcData->DynamicVars[Channel][RANK] = Rank;
    MrcData->DynamicVars[Channel][RDDCMD] = 0;
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(14, VrefCoding);
    SendDramCmd(MrcData, Channel);
    for (StrobeLoop = 0; StrobeLoop < MAX_STROBES; StrobeLoop++) {  // rta , make sure we iterate strobes or cached values can cause trouble
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][StrobeLoop] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][Strobe];
    }
  } else {  //LP3
    if (Channel == 1) Channel = 0;
    if (Channel == 3) Channel = 2;
    if ((Cmd & WR_OFF) == WR_OFF) {
      Cmd &= !WR_OFF;
    }
    GetSetDdrioGroup2(MrcData, 0, Rank, Strobe, TxVref_Actual, Cmd, Value);
    //MrcDelay (MrcData, MILLI_DEL, 1);
    GetSetDdrioGroup2(MrcData, 2, Rank, Strobe, TxVref_Actual, Cmd, Value);
    for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
      for (StrobeLoop = 0; StrobeLoop < MAX_STROBES; StrobeLoop++) {  // rta , make sure we iterate strobes as well , or will cause issues
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].WrVref[Rank][StrobeLoop] = *Value;
      }
    }
  }
#ifdef CAR
  MrcDelay(MrcData, MILLI_DEL, 1);
#else
  MrcDelay(MrcData, MICRO_DEL, 1);
#endif
  return MMRC_SUCCESS;
}




/**
  GetSetTxCaVref

  @param[in,out]  MrcData
  @param[in]  Socket
  @param[in]  Channel
  @param[in]  Dimm
  @param[in]  Rank
  @param[in]  Strobe
  @param[in]  Bit
  @param[in]  FrequencyIndex
  @param[in]  IoLevel
  @param[in]  Cmd
  @param[in,out]  Value

  @retval  MMRC_STATUS
**/
MMRC_STATUS
GetSetTxCaVref (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  UINT8 VrefCoding;
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    if ((Cmd&WR_OFF) == WR_OFF) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] += *Value;
    }
    if (Cmd == FC_WR) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] = *Value;
    }

    if (Cmd == CMD_GET_REG || Cmd == RD_REG) {
      *Value = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank];
      //MmrcDebugPrint ((MMRC_DBG_MIN, "Ch:%d Rk:%d CaVref:%d\n", Channel, Rank, *Value));
      return MMRC_SUCCESS;
    }

    //MmrcExecuteTask (MrcData, modmem_init_dramreset_seq, NO_PRINT, Channel);
    //InitializeJedec (MrcData, 0, NO_PRINT, Channel);
    //LP4AllMRProgramming (MrcData, 0, NO_PRINT, Channel);
    //LP4MRProgramming_MPCZQ (MrcData, 0, NO_PRINT, Channel);
    //MrcDelay (MrcData, MICRO_DEL, 100);

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] > 420) {
      //
      // If Value is outside of the range of the programming.
      //
      return MMRC_SUCCESS;
    } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] < 220) {
      //
      // Use Range 0 if in range.
      //
      VrefCoding = (UINT8)(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] - 100) / 4;
    } else {
      //
      // Otherwise use Range 1
      //
      VrefCoding = (UINT8)(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] - 220) / 4;
      //
      // Add the 1 extra bit for range 1 selection.
      //
      VrefCoding += (1 << 6);
    }

    MrcData->DynamicVars[Channel][RANK] = Rank;
    MrcData->DynamicVars[Channel][RDDCMD] = 0;
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(12, VrefCoding);
    //MmrcDebugPrint ((MMRC_DBG_MIN, "Ch:%d Rk:%d CaVrefCode:%d REGDATA:0x%08x\n", Channel, Rank, VrefCoding, MRW_LP4(12, VrefCoding)));
    SendDramCmd(MrcData, Channel);
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    if ((Cmd&WR_OFF) == WR_OFF) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] += *Value;
    }
    if (Cmd == FC_WR) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank] = *Value;
    }
    if (Cmd == CMD_GET_REG || Cmd == RD_REG) {
      *Value = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CaVref[Rank];
      return MMRC_SUCCESS;
    }
  } else {
    //if ((Cmd & WR_OFF) == WR_OFF) {
    //  Cmd &= !WR_OFF;
    // }
    GetSetDdrioGroup2(MrcData, 0, Rank, Strobe, CaVref_Actual, Cmd, Value);
    GetSetDdrioGroup2(MrcData, 2, Rank, Strobe, CaVref_Actual, Cmd, Value);
  }
  MrcDelay(MrcData, MICRO_DEL, 5);

  MySimStall(100);

  //MrcDelay(MrcData,MICRO_DEL, 50);

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetTxDqDelay (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
   UINT32 DQDriveValue;
  if ((Cmd | FC_WR) == FC_WR)  {
    if (Strobe < MAX_STROBES_NON_ECC) {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDelay_Actual, Cmd, Value);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDelay_Actual, CMD_GET_REG, &DQDriveValue);
    } else {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, TxDqDelay_Actual_Ecc, Cmd, Value);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, TxDqDelay_Actual_Ecc, CMD_GET_REG, &DQDriveValue);
    }
    if (ConfigToDdrType[MrcData->MrcMemConfig] != TypeDdr3L) {
      DQDriveValue -= 2 * GetHalfClk(MrcData, Channel);
    }
    if (Strobe < MAX_STROBES_NON_ECC) {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDrive, CMD_SET_VAL_FC_UC, &DQDriveValue);
    } else {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, TxDqDrive_Ecc, CMD_SET_VAL_FC_UC, &DQDriveValue);
    }
  } else {
    if (Strobe < MAX_STROBES_NON_ECC) {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDelay_Actual, Cmd, Value);
    } else {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, TxDqDelay_Actual_Ecc, Cmd, Value);
    }
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetRDCMD2RDVLDDelay (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  UINT32 rdcmd2rdvld;
  UINT32 rdcmd2rdvld_plus1sel;
  if ((Cmd & CMD_GET_REG) == CMD_GET_REG || Cmd == RD_REG)  {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RDCMD2RDVLD_REG, Cmd, &rdcmd2rdvld);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RDCMD2RDVLD_PLUS1SEL, Cmd, &rdcmd2rdvld_plus1sel);
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].DramType == TypeLpDdr4) {
      switch (rdcmd2rdvld_plus1sel) {
        case 0x0:
          *Value = rdcmd2rdvld*4 + 0;
          break;
        case 0x4:
          *Value = rdcmd2rdvld*4 + 1;
          break;
        case 0x6:
          *Value = rdcmd2rdvld*4 + 2;
          break;
        case 0x7:
          *Value = rdcmd2rdvld*4 + 3;
          break;
        default:
          *Value = rdcmd2rdvld*4;
          break;
      }
    } else {
      switch (rdcmd2rdvld_plus1sel) {
        case 0x0:
          *Value = rdcmd2rdvld*2 + 0;
          break;
        case 0x7:
          *Value = rdcmd2rdvld*2 + 1;
          break;
        default:
          *Value = rdcmd2rdvld*2;
          break;
      }
    }
  } else if ((Cmd & CMD_SET_VAL_FC_UC) == CMD_SET_VAL_FC_UC) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].DramType == TypeLpDdr4) {
      rdcmd2rdvld = *Value / 4;
     switch (*Value % 4) {
        case 0:
          rdcmd2rdvld_plus1sel = 0x0;
          break;
        case 1:
          rdcmd2rdvld_plus1sel = 0x4;
          break;
        case 2:
          rdcmd2rdvld_plus1sel = 0x6;
          break;
        case 3:
          rdcmd2rdvld_plus1sel = 0x7;
          break;
        default:
          rdcmd2rdvld_plus1sel = 0x0;
          break;
      }
    } else {
      rdcmd2rdvld = *Value / 2;
      switch (*Value % 2) {
        case 0:
          rdcmd2rdvld_plus1sel = 0x0;
          break;
        case 1:
          rdcmd2rdvld_plus1sel = 0x7;
          break;
        default:
          rdcmd2rdvld_plus1sel = 0x0;
          break;
      }
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RDCMD2RDVLD_REG, Cmd, &rdcmd2rdvld);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RDCMD2RDVLD_PLUS1SEL, Cmd, &rdcmd2rdvld_plus1sel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
GetSetTimingMode (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
 )
{
  MMRC_STATUS Status;
  if ((Cmd & CMD_GET_REG) == CMD_GET_REG || Cmd == RD_REG)  {
    *Value =  MrcData->TimingModeCache;
  } else if ((Cmd & CMD_SET_VAL_FC_UC) == CMD_SET_VAL_FC_UC || (Cmd&WR_OFF)==WR_OFF || Cmd == FC_WR) {
    if (*Value == 2) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "SwitchTo2N\n"));
      MrcData->TimingModeCache = 2;
      Status = MmrcExecuteTask(MrcData, SwitchTo2N, NO_PRINT, Channel);
    } else {
      MmrcDebugPrint ((MMRC_DBG_MIN, "SwitchTo1N\n"));
      MrcData->TimingModeCache = 1;
      Status = MmrcExecuteTask(MrcData, SwitchTo1N, NO_PRINT, Channel);
    }
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS UpdateCke(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  UINT32 ValueIn, ValueOut;
  UINT8 ch, rk, chout;
  UINT16 clk, cke;
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    for (rk = 0; rk < MAX_RANKS; rk++) {
      if (!RunOnThisChannel (MrcData, ch, rk)) {
        continue;
      }
      if (ch %2 == 0)
      {
        clk=ClkCh0;
        cke=CkeCh0;
      }
      else
      {
        clk=ClkCh1;
        cke=CkeCh1;
      }
      chout=ch-(ch%2); //Convert to the Even channel
      GetSetDdrioGroup2 (MrcData, chout, rk, 0, clk, CMD_GET_REG, &ValueIn);
      ValueOut = ValueIn/GetHalfClk(MrcData, ch);
      MmrcDebugPrint ((MMRC_DBG_MIN, "C:%d R:%d C_out:%d ClkPi:%d -> Cke:%d\r\n", ch, rk, chout, ValueIn, ValueOut));
      GetSetDdrioGroup2 (MrcData, chout, rk, 0, cke, CMD_SET_VAL_FC_UC, &ValueOut);
    }
  }
  return MMRC_SUCCESS;
}

/**
  ReceiveEnableExit
  The function is to be called after Receive Enable and sets the ODT and Diffamp offsets to the same
  2x value of receive enable for each strobe.
  @param[in,out]  MrcData
  @param[in]  Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS
ReceiveEnableExit(
  MMRC_DATA         *MrcData,
  UINT8             Channel
)
{
  UINT32 rcvenval;
  UINT8  rk, st;
  //
  // Loop through each active rank and strobe.
  //
  for (rk=0; rk<MAX_RANKS; rk++)
  {
    if (!RunOnThisChannel (MrcData, Channel, rk)) {
         continue;
    }
    for (st=0; st< MAX_STROBES; st++)
    {
      if (!IsStrobeValid (MrcData, Channel, rk, st)) {
        continue;
      }
      //
      // Read the original 2x value from the Receive Enable.
      //
      GetSetDdrioGroup2 (MrcData, Channel, rk, st, (RecEnDelay), CMD_GET_REG, &rcvenval);
      //
      // Determine the 2x value by dividing by the 2x size and rounding down.
      //
      rcvenval = rcvenval/GetHalfClk(MrcData,Channel);
      //
      // Set the OdtSeg, OdtEn, and DiffAmp offsets to the 2x Rcven value.
      //
      if (st != 8) {
        GetSetDdrioGroup2(MrcData, Channel, rk, st, OdtSegEnOff, CMD_SET_VAL_FC_UC, &rcvenval);
        GetSetDdrioGroup2(MrcData, Channel, rk, st, OdtEnOff, CMD_SET_VAL_FC_UC, &rcvenval);
      }
      if (rk == 0 && st == 8)
      {
        GetSetDdrioGroup2 (MrcData, Channel, rk, 0, OdtSegEnOff_Ecc, CMD_SET_VAL_FC_UC, &rcvenval);
        GetSetDdrioGroup2 (MrcData, Channel, rk, 0, OdtEnOff_Ecc, CMD_SET_VAL_FC_UC, &rcvenval);
      }
    }
  }
  return MMRC_SUCCESS;
}

/**
  LP4DelayTest

  @param[in,out]  MrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS
  LP4DelayTest(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  UINT8  Rank, Strobe;
  UINT32 DQSValue, DQValue, HalfClock, PiInPs;

  HalfClock = GetHalfClk(MrcData, Channel);
  PiInPs = (MrcData->TwoXps + HalfClock/2) / HalfClock;

  for (Rank=0; Rank< MAX_RANKS; Rank++)
  {
    for (Strobe=0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++)
    {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_GET_REG, &DQSValue);
      //
      // Tether DQS and DQ based on Memory Type (can be removed when running CWL)
      //
      DQValue = DQSValue + GetHalfClk (MrcData, Channel)/2 + GetHalfClk (MrcData, Channel);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDelay, CMD_SET_VAL_FC_UC, &DQValue);
    }
  }
  return MMRC_SUCCESS;
}


/**
  ReadReg

  @param[in]  Offset
  @param[in]  ch
  @param[in]  dq

  @retval  None
**/
UINTX ReadReg(UINT8 Box, UINT32 Offset, UINT8 ch, UINT8 dq)
{
  REGISTER_ACCESS Register;
  Register.Offset = Offset;
  Register.ShiftBit = 0;
  Register.Mask = 0xFFFFFFFF;
  return MemRegRead (Box, ch, dq, Register);
}


enum{
  DLL_DQ,
  DLL_CCC_CH0,
  DLL_CCC_CH1,
  DLL_ECC
} DLL_TYPES;

MMRC_STATUS
UpdateMaxFbMuxSel(
  IN  OUT   MAXPI_INFO *MaxPiInfo,
  IN        UINT32      local_txdll_stat_fbmuxsel
  )
{
  if (local_txdll_stat_fbmuxsel > MaxPiInfo->txdll_stat_fbmuxsel_max)
  {
    MaxPiInfo->last_max = MaxPiInfo->txdll_stat_fbmuxsel_max;
    MaxPiInfo->txdll_stat_fbmuxsel_max = local_txdll_stat_fbmuxsel;
  }
  if (local_txdll_stat_fbmuxsel < MaxPiInfo->txdll_stat_fbmuxsel_min)
  {
    MaxPiInfo->txdll_stat_fbmuxsel_min = local_txdll_stat_fbmuxsel;
  }
  //
  //  Track the txdll blocks with the largest values (All need to have the same value)
  //
  if ((MaxPiInfo->last_max == local_txdll_stat_fbmuxsel) || (MaxPiInfo->last_max == 0 && MaxPiInfo->last_max != local_txdll_stat_fbmuxsel))
    MaxPiInfo->code_same_count++;
  return MMRC_SUCCESS;
}


UINT8 GetCtleCode(UINT32 coarse_code)
{
  MMRC_DATA *MrcData = GetMrcHostStructureAddress();
                          //Min|Max|CtleCode
  /*
  TXDLL coarse code(decimal)  drx_cc_ctlebiasctrl/
  drx_bl_ctlebiasctrl
0 to 2  2
3 to 5  3
4 to 6  4
7 to 11  5
11 to 15  6


  */
                          //Min|Max|CtleCode
  UINT8 coarsetable[][3] = { { 0,  2,  2},
                             { 3,  5,  3},
                             { 4,  6,  4},
                             { 7, 11,  5},
                             {11, 15,  6} };
  const int entries = sizeof(coarsetable)/3;
  int i;
  UINT8 min,max;

  for (i=0; i< entries; i++)
  {
    min=coarsetable[i][0];
    max=coarsetable[i][1];
    if (coarse_code >= min && coarse_code <= max) {
      if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L && MrcData->CpuStepping != STEPPING_P0) {
        return 6;
      } else {
        return coarsetable[i][2];
      }
    }
  }
  return 0xff;
}

UINT8 ProgramCtle(UINT8 ch, UINT8 dq, BOOLEAN isecc, UINT32 coarse_code)
{
  CTLECTL2_CCC_BOXTYPE_STRUCT* ccc_ctlectl;
  CTLECTL1_DQ_BOXTYPE_STRUCT*  dq0_ctlectl;
  CTLECTL3_DQ_BOXTYPE_STRUCT*  dq1_ctlectl;
  UINTX                Value;
  REGISTER_ACCESS      Register;
  UINT8                ctleval = GetCtleCode(coarse_code);

  Register.ShiftBit=0;
  Register.Mask    = 0xffffffff;

  if (isecc)
  {
    Register.Offset = CTLECTL2_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, 0, Register);
    ccc_ctlectl = (CTLECTL2_CCC_BOXTYPE_STRUCT *) &Value;
    ccc_ctlectl->Bits.drx_dq_ctlebiasctrl=ctleval;
    MemRegWrite(CCC_BOXTYPE, ch, 0, Register, Value, 0xF);
  }
  else
  {
    //
    //  Program BL, and CC in each DQ module
    //
    Register.Offset = CTLECTL1_DQ_BOXTYPE_REG;
    Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
    dq0_ctlectl = (CTLECTL1_DQ_BOXTYPE_STRUCT *) &Value;
    dq0_ctlectl->Bits.drx_bl_ctlebiasctrl=ctleval;
    MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
    Register.Offset = CTLECTL3_DQ_BOXTYPE_REG;
    Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
    dq1_ctlectl = (CTLECTL3_DQ_BOXTYPE_STRUCT *) &Value;
    dq1_ctlectl->Bits.drx_cc_ctlebiasctrl=ctleval;
    MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
  }
  return ctleval;
}

void DoDllInit(
  IN  OUT   MMRC_DATA  *MrcData,
  IN  OUT   MAXPI_INFO *MaxPiInfo,
  IN        BOOLEAN     EccEnabled
)
{
  UINT8                                ch, dq, rk;
  UINTX                                Value;
  DLLCOMP_CODE_STATUS_DQ_BOXTYPE_STRUCT*       dq_dllcmpcode;
  DLLCOMP_ERROR_STATUS_DQ_BOXTYPE_STRUCT*      dq_dllcomp_error_status;
  DLLCOMP_CODE_STATUS_CH0_CCC_BOXTYPE_STRUCT*  ccc0_dllcompcode;
  DLLCOMP_ERROR_STATUS_CH0_CCC_BOXTYPE_STRUCT* ccc0_dllcomp_error_status;
  DLLCOMP_CODE_STATUS_CH1_CCC_BOXTYPE_STRUCT*  ccc1_dllcompcode;
  DLLCOMP_ERROR_STATUS_CH1_CCC_BOXTYPE_STRUCT* ccc1_dllcomp_error_status;
  DLLCOMP_CODE_STATUS_ECC_CCC_BOXTYPE_STRUCT*  ecc_dllcompcode;
  DLLCOMP_ERROR_STATUS_ECC_CCC_BOXTYPE_STRUCT* ecc_dllcomp_error_status;
  UINT32                               local_txdll_stat_fbmuxsel, local_txdll_error, local_coarsecode, local_coarseicode, local_finecode;
  UINT8                                MaxDq=InstancePortMap[DQ_BOXTYPE].Instances;
  BOOLEAN                              ccc_ch0_enabled, ccc_ch1_enabled;
  UINT8                                ccc_ch, local_ctle;

  ccc_ch = 0;
  local_ctle = 0;
  MaxPiInfo->txdll_stat_fbmuxsel_max=0;
  MaxPiInfo->txdll_error=0;
  //MaxPiInfo->muxcode_min=255;
  //MaxPiInfo->muxcode_max=0;
  MaxPiInfo->code_count=0;
  MaxPiInfo->code_same_count=0;
  MaxPiInfo->last_max = 0;
  //
  //  Find Max Code
  //
  for(ch=0; ch< MAX_CHANNELS; ch++)
  {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE)
    {
      PrintLimits(MrcData, ch);
      MmrcExecuteTask (MrcData, maxpi_set_codes, NO_PRINT, ch);
      MmrcExecuteTask (MrcData, maxpi_dll_init, NO_PRINT, ch);
      //for(rk=0; rk< MAX_RANKS; rk++)
      rk=0; // these signals are not Rank specific
      {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].RankEnabled[rk] == TRUE)
        {
          for (dq=0; dq < MaxDq; dq++)
          {
            local_ctle=0xFE;
            if (!IsStrobeValid (MrcData, ch, rk, dq))
              continue;
            MaxPiInfo->code_count++;
            if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].DramType <= TypeDdr3All)
            {
              ccc_ch=ch;
              ccc_ch0_enabled=TRUE;
              ccc_ch1_enabled=TRUE;
            }
            else
            {
              ccc_ch0_enabled=FALSE;
              ccc_ch1_enabled=FALSE;
              if ((ch==0) || (ch==1)) ccc_ch = 0;
              else if ((ch==2) || (ch==3)) ccc_ch=2;
              if ((ch==0) || (ch==2)) ccc_ch0_enabled=TRUE;
              else if ((ch==1) || (ch==3)) ccc_ch1_enabled=TRUE;
            }
            {
              //
              //  Read Comp Code per DQ (Can't use the spreadsheet since it returns only per channel)
              //
              Value = ReadReg(DQ_BOXTYPE, DLLCOMP_ERROR_STATUS_DQ_BOXTYPE_REG, ch, dq);
              dq_dllcomp_error_status = (DLLCOMP_ERROR_STATUS_DQ_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "DQ_CH_ERR_STAT 0x%08x\n", (UINT32) Value));
              //
              // Error Flag          Error Type      Comments
              // txdll_error         Hard error      Initialization will not complete
              // txdll_coarsep_error Hard error      Will need re-initialization.
              // txdll_fine_error    Hard/Soft error Hard error if asserted during initialization. Initialization will not complete.
              //                                     Soft error once initialization is complete.
              //
              // Clear the fine error flag since fine error is not a showstopper as long as txdll_init_done == 1
              // and no other error flags are set
              dq_dllcomp_error_status->Bits.txdll_fine_error = 0;
              // Check that only txdll_init_done == 1 and no error flags are set
              local_txdll_error = (UINT32) ((Value == 1) ? 0 : 1);
              MaxPiInfo->txdll_error |= local_txdll_error;

              Value = ReadReg(DQ_BOXTYPE, DLLCOMP_CODE_STATUS_DQ_BOXTYPE_REG, ch, dq);
              dq_dllcmpcode = (DLLCOMP_CODE_STATUS_DQ_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "DQ_CODE_STAT 0x%08x\n", (UINT32) Value));
              local_txdll_stat_fbmuxsel = (UINT32) dq_dllcmpcode->Bits.txdll_stat_fbmuxsel;
              local_coarsecode = (UINT32) dq_dllcmpcode->Bits.txdll_stat_coarsecode;
              local_coarseicode = (UINT32) dq_dllcmpcode->Bits.txdll_stat_coarseicode;
              local_finecode = (UINT32) dq_dllcmpcode->Bits.txdll_stat_finecode;
              if (local_txdll_error == 0)
              {
                UpdateMaxFbMuxSel(MaxPiInfo, local_txdll_stat_fbmuxsel);
                local_ctle = ProgramCtle(ch, dq, FALSE, local_coarsecode);
              }
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "CH %d:DQ %d:RK %d:Val 0x%08x:Max 0x%08x Err:%d Coarse:%d Coarsei:%d Fine:%d ctlebias=%d\n", ch, dq, rk, local_txdll_stat_fbmuxsel, MaxPiInfo->txdll_stat_fbmuxsel_max, MaxPiInfo->txdll_error, local_coarsecode, local_coarseicode, local_finecode, local_ctle));
            }
            if (dq==0 && ccc_ch0_enabled) //DLL_CCC_CH0)
            {
              //
              //  Read Comp Code per DQ (Can't use the spreadsheet since it returns only per channel)
              //
              Value = ReadReg(CCC_BOXTYPE, DLLCOMP_ERROR_STATUS_CH0_CCC_BOXTYPE_REG, ccc_ch, dq);
              ccc0_dllcomp_error_status = (DLLCOMP_ERROR_STATUS_CH0_CCC_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint((MMRC_DBG_MIN, "CCC0_CH_ERR_STAT 0x%08x\n", (UINT32)Value));
              //
              // Error Flag          Error Type      Comments
              // txdll_error         Hard error      Initialization will not complete
              // txdll_coarsep_error Hard error      Will need re-initialization.
              // txdll_fine_error    Hard/Soft error Hard error if asserted during initialization. Initialization will not complete.
              //                                     Soft error once initialization is complete.
              //
              // Clear the fine error flag since fine error is not a showstopper as long as txdll_init_done == 1
              // and no other error flags are set
              ccc0_dllcomp_error_status->Bits.ch0_txdll_fine_error = 0;
              // Check that only txdll_init_done == 1 and no error flags are set
              local_txdll_error = (UINT32) ((Value == 1) ? 0 : 1);
              MaxPiInfo->txdll_error |= local_txdll_error;

              Value = ReadReg(CCC_BOXTYPE, DLLCOMP_CODE_STATUS_CH0_CCC_BOXTYPE_REG, ccc_ch, dq);
              ccc0_dllcompcode = (DLLCOMP_CODE_STATUS_CH0_CCC_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "CCC0_CODE_STAT 0x%08x\n", (UINT32) Value));
              local_txdll_stat_fbmuxsel = (UINT32) ccc0_dllcompcode->Bits.ch0_txdll_stat_fbmuxsel;
              local_coarsecode = (UINT32) ccc0_dllcompcode->Bits.ch0_txdll_stat_coarsecode;
              local_coarseicode = (UINT32) ccc0_dllcompcode->Bits.ch0_txdll_stat_coarseicode;
              local_finecode = (UINT32) ccc0_dllcompcode->Bits.ch0_txdll_stat_finecode;
              if (local_txdll_error == 0)
              {
                UpdateMaxFbMuxSel(MaxPiInfo, local_txdll_stat_fbmuxsel);
              }
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "CCC0_CH %d:DQ %d:RK %d:Val 0x%08x:Max 0x%08x Err:%d Coarse:%d Coarsei:%d Fine:%d\n", ch, dq, rk, local_txdll_stat_fbmuxsel, MaxPiInfo->txdll_stat_fbmuxsel_max, MaxPiInfo->txdll_error, local_coarsecode, local_coarseicode, local_finecode));
            }
            if (dq==0 && ccc_ch1_enabled) //DLL_CCC_CH1)
            {
              //
              //  Read Comp Code per DQ (Can't use the spreadsheet since it returns only per channel)
              //
              Value = ReadReg(CCC_BOXTYPE, DLLCOMP_ERROR_STATUS_CH1_CCC_BOXTYPE_REG, ccc_ch, dq);
              ccc1_dllcomp_error_status = (DLLCOMP_ERROR_STATUS_CH1_CCC_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint((MMRC_DBG_MIN, "CCC1_CH_ERR_STAT 0x%08x\n", (UINT32)Value));
              //
              // Error Flag          Error Type      Comments
              // txdll_error         Hard error      Initialization will not complete
              // txdll_coarsep_error Hard error      Will need re-initialization.
              // txdll_fine_error    Hard/Soft error Hard error if asserted during initialization. Initialization will not complete.
              //                                     Soft error once initialization is complete.
              //
              // Clear the fine error flag since fine error is not a showstopper as long as txdll_init_done == 1
              // and no other error flags are set
              ccc1_dllcomp_error_status->Bits.ch1_txdll_fine_error = 0;
              // Check that only txdll_init_done == 1 and no error flags are set
              local_txdll_error = (UINT32) ((Value == 1) ? 0 : 1);
              MaxPiInfo->txdll_error |= local_txdll_error;

              Value = ReadReg(CCC_BOXTYPE, DLLCOMP_CODE_STATUS_CH1_CCC_BOXTYPE_REG, ccc_ch, dq);
              ccc1_dllcompcode = (DLLCOMP_CODE_STATUS_CH1_CCC_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "CCC1_CODE_STAT 0x%08x\n", (UINT32) Value));
              local_txdll_stat_fbmuxsel = (UINT32) ccc1_dllcompcode->Bits.ch1_txdll_stat_fbmuxsel;
              local_coarsecode = (UINT32) ccc1_dllcompcode->Bits.ch1_txdll_stat_coarsecode;
              local_coarseicode = (UINT32) ccc1_dllcompcode->Bits.ch1_txdll_stat_coarseicode;
              local_finecode = (UINT32) ccc1_dllcompcode->Bits.ch1_txdll_stat_finecode;
              if (local_txdll_error == 0)
              {
                UpdateMaxFbMuxSel(MaxPiInfo, local_txdll_stat_fbmuxsel);
              }
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "CCC1_CH %d:DQ %d:RK %d:Val 0x%08x:Max 0x%08x Err:%d Coarse:%d Coarsei:%d Fine:%d\n", ch, dq, rk, local_txdll_stat_fbmuxsel, MaxPiInfo->txdll_stat_fbmuxsel_max, MaxPiInfo->txdll_error, local_coarsecode, local_coarseicode, local_finecode));
            }
            if (dq==0 && EccEnabled) //DLL_ECC)
            {
              //
              //  Read Comp Code per DQ (Can't use the spreadsheet since it returns only per channel)
              //
              Value = ReadReg(CCC_BOXTYPE, DLLCOMP_ERROR_STATUS_ECC_CCC_BOXTYPE_REG, ch, dq);
              ecc_dllcomp_error_status = (DLLCOMP_ERROR_STATUS_ECC_CCC_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint((MMRC_DBG_MIN, "ECC_CH_ERR_STAT 0x%08x\n", (UINT32)Value));
              //
              // Error Flag          Error Type      Comments
              // txdll_error         Hard error      Initialization will not complete
              // txdll_coarsep_error Hard error      Will need re-initialization.
              // txdll_fine_error    Hard/Soft error Hard error if asserted during initialization. Initialization will not complete.
              //                                     Soft error once initialization is complete.
              //
              // Clear the fine error flag since fine error is not a showstopper as long as txdll_init_done == 1
              // and no other error flags are set
              ecc_dllcomp_error_status->Bits.ecc_txdll_fine_error = 0;
              // Check that only txdll_init_done == 1 and no error flags are set
              local_txdll_error = (UINT32) ((Value == 1) ? 0 : 1);
              MaxPiInfo->txdll_error |= local_txdll_error;

              Value = ReadReg(CCC_BOXTYPE, DLLCOMP_CODE_STATUS_ECC_CCC_BOXTYPE_REG, ch, dq);
              ecc_dllcompcode = (DLLCOMP_CODE_STATUS_ECC_CCC_BOXTYPE_STRUCT*) &Value;
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "ECC_CODE_STAT 0x%08x\n", (UINT32) Value));
              local_txdll_stat_fbmuxsel = (UINT32) ecc_dllcompcode->Bits.ecc_txdll_stat_fbmuxsel;
              local_coarsecode = (UINT32) ecc_dllcompcode->Bits.ecc_txdll_stat_coarsecode;
              local_coarseicode = (UINT32) ecc_dllcompcode->Bits.ecc_txdll_stat_coarseicode;
              local_finecode = (UINT32) ecc_dllcompcode->Bits.ecc_txdll_stat_finecode;
              if (local_txdll_error == 0)
              {
                UpdateMaxFbMuxSel(MaxPiInfo, local_txdll_stat_fbmuxsel);
                local_ctle = ProgramCtle(ch, dq, TRUE, local_coarsecode);
              }
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "ECC_CH %d:DQ %d:RK %d:Val 0x%08x:Max 0x%08x Err:%d Coarse:%d Coarsei:%d Fine:%d ctle=%d\n", ch, dq, rk, local_txdll_stat_fbmuxsel, MaxPiInfo->txdll_stat_fbmuxsel_max, MaxPiInfo->txdll_error, local_coarsecode, local_coarseicode, local_finecode, local_ctle));
            }
          }
        }
      }
    }
  }
}

/**
  CopyPiCodesToAllChannels

  @param[in,out]  MrcData
  @param[in,out]  MaxPiInfo
  @param[in]      Channel

  @retval  None
**/
void CopyPiCodesToAllChannels(
  IN  OUT   MMRC_DATA  *MrcData,
  IN  OUT   MAXPI_INFO *MaxPiInfo,
  IN        UINT8      Channel
  )
{
  UINT8   ch;
  MaxPiInfo->muxcode_min = (UINT8) MrcData->DynamicVars[Channel][MUX_MIN];
  MaxPiInfo->muxcode_max = (UINT8) MrcData->DynamicVars[Channel][MUX_MAX];
  for(ch=0; ch< MAX_CHANNELS; ch++)
  {
    MrcData->DynamicVars[ch][FINE_MIN]   = MrcData->DynamicVars[Channel][FINE_MIN];
    MrcData->DynamicVars[ch][FINE_MAX]   = MrcData->DynamicVars[Channel][FINE_MAX];
    MrcData->DynamicVars[ch][MUX_MIN]    = MrcData->DynamicVars[Channel][MUX_MIN];
    MrcData->DynamicVars[ch][MUX_MAX]    = MrcData->DynamicVars[Channel][MUX_MAX];
    MrcData->DynamicVars[ch][COARSE_MIN] = MrcData->DynamicVars[Channel][COARSE_MIN];
    MrcData->DynamicVars[ch][COARSE_MAX] = MrcData->DynamicVars[Channel][COARSE_MAX];
  }
}

MMRC_STATUS
  OverrideFineCoarseCodes(
  MMRC_DATA         *MrcData,
  UINT8             ch,
  UINT8             dq,
  UINT8             SigType
  )
{
  REGISTER_ACCESS Register;

  DLLCOMP_CODE_STATUS_DQ_BOXTYPE_STRUCT*       dq_dllcmpcode;
  DLLCOMP_CODE_STATUS_CH0_CCC_BOXTYPE_STRUCT*  ccc0_dllcmpcode;
  DLLCOMP_CODE_STATUS_CH1_CCC_BOXTYPE_STRUCT*  ccc1_dllcmpcode;
  DLLCOMP_CODE_STATUS_ECC_CCC_BOXTYPE_STRUCT*  ecc_dllcmpcode;
  DLLCOMP_RXCOARSEOVR_DQ_BOXTYPE_STRUCT*       dq_rxcoarseovr;
  DLLCOMP_RXCOARSEOVR_CH0_CCC_BOXTYPE_STRUCT*  ccc0_rxcoarseovr;
  DLLCOMP_RXCOARSEOVR_CH1_CCC_BOXTYPE_STRUCT*  ccc1_rxcoarseovr;
  DLLCOMP_RXCOARSEOVR_ECC_CCC_BOXTYPE_STRUCT*  ecc_rxcoarseovr;
  DLLCOMP_RXFINEOVR_DQ_BOXTYPE_STRUCT*         dq_rxfineovr;
  DLLCOMP_RXFINEOVR_CH0_CCC_BOXTYPE_STRUCT*    ccc0_rxfineovr;
  DLLCOMP_RXFINEOVR_CH1_CCC_BOXTYPE_STRUCT*    ccc1_rxfineovr;
  DLLCOMP_RXFINEOVR_ECC_CCC_BOXTYPE_STRUCT*    ecc_rxfineovr;
  UINT32  local_coarsecode, local_finecode;
  UINT64  Value;
  Register.ShiftBit = 0;
  Register.Mask = 0xffffffff;
  if (SigType == DLL_DQ)
  {
    //
    //  Getting current fine/coarse codes
    //
    //Register.Offset = DLLCOMP_CODE_STATUS_DQ_BOXTYPE_REG;
    Value = ReadReg (DQ_BOXTYPE, DLLCOMP_CODE_STATUS_DQ_BOXTYPE_REG, ch, dq);
    dq_dllcmpcode = (DLLCOMP_CODE_STATUS_DQ_BOXTYPE_STRUCT*) &Value;
    local_coarsecode = (UINT32) dq_dllcmpcode->Bits.txdll_stat_coarsecode;
    local_finecode = (UINT32) dq_dllcmpcode->Bits.txdll_stat_finecode;

    //
    //  Setting Override values
    //
    // Fine
    Register.Offset = DLLCOMP_RXCOARSEOVR_DQ_BOXTYPE_REG;
    Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
    dq_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_DQ_BOXTYPE_STRUCT*) &Value;
    dq_rxcoarseovr->Bits.rxdll_coarsecode_ovr=local_coarsecode;
    MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);

    //Coarse
    Register.Offset = DLLCOMP_RXFINEOVR_DQ_BOXTYPE_REG;
    Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
    dq_rxfineovr = (DLLCOMP_RXFINEOVR_DQ_BOXTYPE_STRUCT*) &Value;
    dq_rxfineovr->Bits.rxdll_finecode_ovr=local_finecode;
    MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);

    //
    //  Enabling Overrides
    //
    Register.Offset = DLLCOMP_RXCOARSEOVR_DQ_BOXTYPE_REG;
    Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
    dq_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_DQ_BOXTYPE_STRUCT*) &Value;
    dq_rxcoarseovr->Bits.rxdll_coarsecode_ovr_sel=1;
    MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_DQ_BOXTYPE_REG;
    Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
    dq_rxfineovr = (DLLCOMP_RXFINEOVR_DQ_BOXTYPE_STRUCT*) &Value;
    dq_rxfineovr->Bits.rxdll_finecode_ovr_sel=1;
    dq_rxfineovr->Bits.rxdll_finecode_updateovr_ovr=1;
    MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
    MmrcDebugPrint ((MMRC_DBG_MIN, "dq_ch:%d dq:%d fc=%d cc=%d\r\n", (UINT32) ch, dq, local_finecode, local_coarsecode));
  }
  else if ((SigType == DLL_CCC_CH0))
  {
    Register.Offset = DLLCOMP_CODE_STATUS_CH0_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc0_dllcmpcode = (DLLCOMP_CODE_STATUS_CH0_CCC_BOXTYPE_STRUCT*) &Value;
    local_coarsecode = (UINT32) ccc0_dllcmpcode->Bits.ch0_txdll_stat_coarsecode;
    local_finecode = (UINT32) ccc0_dllcmpcode->Bits.ch0_txdll_stat_finecode;

    Register.Offset = DLLCOMP_RXCOARSEOVR_CH0_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc0_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_CH0_CCC_BOXTYPE_STRUCT*) &Value;
    ccc0_rxcoarseovr->Bits.ch0_rxdll_coarsecode_ovr=local_coarsecode;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_CH0_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc0_rxfineovr = (DLLCOMP_RXFINEOVR_CH0_CCC_BOXTYPE_STRUCT*) &Value;
    ccc0_rxfineovr->Bits.ch0_rxdll_finecode_ovr=local_finecode;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXCOARSEOVR_CH0_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc0_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_CH0_CCC_BOXTYPE_STRUCT*) &Value;
    ccc0_rxcoarseovr->Bits.ch0_rxdll_coarsecode_ovr_sel=1;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_CH0_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc0_rxfineovr = (DLLCOMP_RXFINEOVR_CH0_CCC_BOXTYPE_STRUCT*) &Value;
    ccc0_rxfineovr->Bits.ch0_rxdll_finecode_ovr_sel=1;
    ccc0_rxfineovr->Bits.ch0_rxdll_finecode_updateovr_ovr=1;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);
    MmrcDebugPrint ((MMRC_DBG_MIN, "ccc0_ch:%d dq:%d fc=%d cc=%d\r\n", (UINT32) ch, dq, local_finecode, local_coarsecode));
  }
  else if ((SigType == DLL_CCC_CH1))
  {
    Register.Offset = DLLCOMP_CODE_STATUS_CH1_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc1_dllcmpcode = (DLLCOMP_CODE_STATUS_CH1_CCC_BOXTYPE_STRUCT*) &Value;
    local_coarsecode = (UINT32) ccc1_dllcmpcode->Bits.ch1_txdll_stat_coarsecode;
    local_finecode = (UINT32) ccc1_dllcmpcode->Bits.ch1_txdll_stat_finecode;

    Register.Offset = DLLCOMP_RXCOARSEOVR_CH1_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc1_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_CH1_CCC_BOXTYPE_STRUCT*) &Value;
    ccc1_rxcoarseovr->Bits.ch1_rxdll_coarsecode_ovr=local_coarsecode;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_CH1_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc1_rxfineovr = (DLLCOMP_RXFINEOVR_CH1_CCC_BOXTYPE_STRUCT*) &Value;
    ccc1_rxfineovr->Bits.ch1_rxdll_finecode_ovr=local_finecode;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXCOARSEOVR_CH1_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc1_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_CH1_CCC_BOXTYPE_STRUCT*) &Value;
    ccc1_rxcoarseovr->Bits.ch1_rxdll_coarsecode_ovr_sel=1;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_CH1_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
    ccc1_rxfineovr = (DLLCOMP_RXFINEOVR_CH1_CCC_BOXTYPE_STRUCT*) &Value;
    ccc1_rxfineovr->Bits.ch1_rxdll_finecode_ovr_sel=1;
    ccc1_rxfineovr->Bits.ch1_rxdll_finecode_updateovr_ovr=1;
    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);
    MmrcDebugPrint ((MMRC_DBG_MIN, "ccc1_ch:%d dq:%d fc=%d cc=%d\r\n", (UINT32) ch, dq, local_finecode, local_coarsecode));
  }
  else if (SigType == DLL_ECC)
  {
    Register.Offset = DLLCOMP_CODE_STATUS_ECC_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, 0, Register);
    ecc_dllcmpcode = (DLLCOMP_CODE_STATUS_ECC_CCC_BOXTYPE_STRUCT*) &Value;
    local_coarsecode = (UINT32) ecc_dllcmpcode->Bits.ecc_txdll_stat_coarsecode;
    local_finecode = (UINT32) ecc_dllcmpcode->Bits.ecc_txdll_stat_finecode;

    Register.Offset = DLLCOMP_RXCOARSEOVR_ECC_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, 0, Register);
    ecc_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_ECC_CCC_BOXTYPE_STRUCT*) &Value;
    ecc_rxcoarseovr->Bits.ecc_rxdll_coarsecode_ovr=local_coarsecode;
    MemRegWrite(CCC_BOXTYPE, ch, 0, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_ECC_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, 0, Register);
    ecc_rxfineovr = (DLLCOMP_RXFINEOVR_ECC_CCC_BOXTYPE_STRUCT*) &Value;
    ecc_rxfineovr->Bits.ecc_rxdll_finecode_ovr=local_finecode;
    MemRegWrite(CCC_BOXTYPE, ch, 0, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXCOARSEOVR_ECC_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, 0, Register);
    ecc_rxcoarseovr = (DLLCOMP_RXCOARSEOVR_ECC_CCC_BOXTYPE_STRUCT*) &Value;
    ecc_rxcoarseovr->Bits.ecc_rxdll_coarsecode_ovr_sel=1;
    MemRegWrite(CCC_BOXTYPE, ch, 0, Register, Value, 0xF);

    Register.Offset = DLLCOMP_RXFINEOVR_ECC_CCC_BOXTYPE_REG;
    Value = MemRegRead (CCC_BOXTYPE, ch, 0, Register);
    ecc_rxfineovr = (DLLCOMP_RXFINEOVR_ECC_CCC_BOXTYPE_STRUCT*) &Value;
    ecc_rxfineovr->Bits.ecc_rxdll_finecode_ovr_sel=1;
    ecc_rxfineovr->Bits.ecc_rxdll_finecode_updateovr_ovr=1;
    MemRegWrite(CCC_BOXTYPE, ch, 0, Register, Value, 0xF);
    MmrcDebugPrint ((MMRC_DBG_MIN, "ecc_ch:%d dq:%d fc=%d cc=%d\r\n", (UINT32) ch, dq, local_finecode, local_coarsecode));
  }
  return MMRC_SUCCESS;
}

/**
  PrintLimits

  @param[in,out]  MrcData
  @param[in]      Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS PrintLimits(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT8      Channel
  )
{
  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "CH%d Limits\n", Channel));
  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "\tFine Hard: 0x%x - 0x%x\r\n", (UINT32) MrcData->DynamicVars[Channel][FINE_MIN], (UINT32) MrcData->DynamicVars[Channel][FINE_MAX]));
  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "\tMux      : 0x%x - 0x%x\r\n", (UINT32) MrcData->DynamicVars[Channel][MUX_MIN], (UINT32) MrcData->DynamicVars[Channel][MUX_MAX]));
  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "\tCoarse   : 0x%x - 0x%x\r\n", (UINT32) MrcData->DynamicVars[Channel][COARSE_MIN], (UINT32) MrcData->DynamicVars[Channel][COARSE_MAX]));
  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "\tFine Soft: 0x%x - 0x%x\r\n", (UINT32) MrcData->DynamicVars[Channel][FINE_SOFT_MIN], (UINT32) MrcData->DynamicVars[Channel][FINE_SOFT_MAX]));

  return MMRC_SUCCESS;
}

/**
  SetupMaxPI

  @param[in,out]  MrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS SetupMaxPI (
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channelx
)
{
  //
  //  This assumes that all LP3/4 Channels are running at the same frequency (So only one variable is used to store the max value
  //
  UINT8   ch, dq, rk=0;
  UINTX   Value;
  REGISTER_ACCESS Register;
  //DLLCOMP_FBMUXSEL_DQCCC_BOXTYPE_STRUCT* dllfbmux;
  TXDLLREF2XCTL_DQ_BOXTYPE_STRUCT*             dq_txdll;
  TXDLLREF2XCTL_CCC_BOXTYPE_STRUCT*            ccc_txdll;
  RXDLLCTL_DQBLDM_DQ_BOXTYPE_STRUCT*           dq_rxdllctl_dqbldm;
  RXDLLCTL_DQBLDM_CCC_BOXTYPE_STRUCT*          ccc_rxdllctl_dqbldm;
  DBREGIONCTL1_DQDPLVUG_BOXTYPE_STRUCT*        dbregionctl1;
  DBREGIONCTL2_DQDPLVUG_BOXTYPE_STRUCT*        dbregionctl2;
  DLLCOMP_FINECTL_DQ_BOXTYPE_STRUCT*           dq_finectl;
  DLLCOMP_FINECTL_CCC_BOXTYPE_STRUCT*          ccc_finectl;

  //DLLCOMP_ERROR_STATUS_DQCCC_BOXTYPE_STRUCT* dllcomp_error_status;
  PLLCTL_DPIC_BOXTYPE_STRUCT*            pllctl;
  MAXPI_INFO  MaxPiInfo;
  //UINT32  local_coarsecode, local_finecode;
  UINT32  Coarse_Code_Max=13, Coarse_Code_Min=4, Fine_Code_Min, Fine_Code_Max, Muxcode_Max, Muxcode_Min, MuxcodeMinLowerLimit=0;
  UINT32  Init_Fine_Code_Min, Init_Fine_Code_Max, Init_Muxcode_Max, Init_Muxcode_Min;
  MAXPI_STATES maxpi_state = MAXPI_INIT;
  UINT32  HalfClkPiFull, HalfRate;
  UINT8   complete=0;
  UINT8   FinalDecision=0;
  UINT32  a0offset=0;
  UINT32  MaxPiNewLimit, MaxPiOldLimit, LinearValue[3], grp, Current2x, CurrentPi, CalculatedPi;
  UINT16  instances;
  BOOLEAN EccEnabled=(MrcData->NvData.MrcParamsSaveRestore.Channel[0].EccEnabled!=0), ccc_ch0_enabled=FALSE, ccc_ch1_enabled=FALSE;
  UINT8   MaxDq = InstancePortMap[DQ_BOXTYPE].Instances, ccc_ch;
  BOOLEAN RestorePath = FALSE;
  Register.ShiftBit = 0;
  Register.Mask = 0xffffffff;
  ccc_ch = 0;
  MmrcMemset (&MaxPiInfo, 0, sizeof (MAXPI_INFO));
  MrcData->EnableVocCtle=TRUE;

  //
  // MrcData->NvData.MrcParamsSaveRestore.SetupMaxPiDone[] is a Non-Volatile variable. It's state is preserved
  // accross resets and available in the S0 and FB flow.
  //
  if (MrcData->NvData.MrcParamsSaveRestore.SetupMaxPiDone[GetCurrentFrequencyIndex()]) {
    //
    // If SetupMaxPI was previously completed for the current frequency, use the restore flow
    //
    RestorePath = TRUE;
  } else {
    //
    // Else, set the "Done" flag and continue with the full flow.
    //
    MrcData->NvData.MrcParamsSaveRestore.SetupMaxPiDone[GetCurrentFrequencyIndex()] = TRUE;
  }

  while (complete == 0)
  {
    MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI State = %s\r\n", MAXPI_STATE_NAME[maxpi_state]));
    switch (maxpi_state)
    {
    case MAXPI_INIT:
      if (EccEnabled) MmrcDebugPrint ((MMRC_DBG_MIN, "Ecc Enabled\n"));
      else MmrcDebugPrint ((MMRC_DBG_MIN, "Ecc Disabled\n"));
      for(ch=0; ch< MAX_CHANNELS; ch++)
      {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE)
        {
          //
          // program initial pll_cmn_fbclktrim, coarseicode_ovr_sel, coarseicode_ovr, muxcode_max, muxcode_min
          //
          MmrcExecuteTask (MrcData, maxpi_override_coarse_init, NO_PRINT, ch);
        }
        for (dq=0; dq < MaxDq; dq++)
        {
          //
          // finec
          //
          //DQ
          Register.Offset = DLLCOMP_FINECTL_DQ_BOXTYPE_REG;
          Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
          dq_finectl = (DLLCOMP_FINECTL_DQ_BOXTYPE_STRUCT *) &Value;
          dq_finectl->Bits.finec_mode = 0;
          if (ch==0 && dq==0)
            MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "setting dq_finec=0\r\n"));
          MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
          if (dq == 0)
          {
            Register.Offset = DLLCOMP_FINECTL_CCC_BOXTYPE_REG;
            Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
            ccc_finectl = (DLLCOMP_FINECTL_CCC_BOXTYPE_STRUCT *) &Value;
            ccc_finectl->Bits.finec_mode = 0;
            if (ch==0 && dq==0)
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "setting ccc_finec=0\r\n"));
            MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);
          }
        }
      }
      //
      //  Get Starting Values
      //
      for(ch=0; ch< MAX_CHANNELS; ch++)
      {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE)
        {
          //MmrcExecuteTask (MrcData, maxpi_override_coarse_init, NO_PRINT, ch);
          //
          // Get muxcode_max, muxcode_min, coarsecode_max, coarsecode_min, finecodemax_limit, finecodemin_limit, finecode_max, finecode_min from first active channel
          //
          MmrcExecuteTask (MrcData, maxpi_get_codes, NO_PRINT, ch);
          if (RestorePath)
          {
            Coarse_Code_Min=2;
            Coarse_Code_Max=14;//15;
          }
          MrcData->DynamicVars[ch][COARSE_MIN]= Coarse_Code_Min;
          MrcData->DynamicVars[ch][COARSE_MAX]= Coarse_Code_Max;
          //
          //MrcData->DynamicVars[ch][MUX_MIN]=0x6;
          //MrcData->DynamicVars[ch][MUX_MAX]=0x6;
          //
          CopyPiCodesToAllChannels(MrcData, &MaxPiInfo, ch);
          Fine_Code_Min = (UINT32) MrcData->DynamicVars[ch][FINE_MIN];
          Fine_Code_Max = (UINT32) MrcData->DynamicVars[ch][FINE_MAX];
          Muxcode_Min = (UINT32) MrcData->DynamicVars[ch][MUX_MIN];
          Muxcode_Max = (UINT32) MrcData->DynamicVars[ch][MUX_MAX];

          Init_Fine_Code_Min = (UINT32) MrcData->DynamicVars[ch][FINE_MIN];
          Init_Fine_Code_Max = (UINT32) MrcData->DynamicVars[ch][FINE_MAX];
          Init_Muxcode_Min = (UINT32) MrcData->DynamicVars[ch][MUX_MIN];
          Init_Muxcode_Max = (UINT32) MrcData->DynamicVars[ch][MUX_MAX];
          MuxcodeMinLowerLimit = Init_Muxcode_Min-2;
          break;
        }
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "A0 Offset %d\r\n", a0offset));
      PrintLimits (MrcData, 0);
      if (!RestorePath)
      {
        maxpi_state =  MAXPI_DLLINIT;  // New Calc
      }
      else
      {
        maxpi_state = MAXPI_FINAL_VALUES;  // Restore
        for(ch=0; ch< MAX_CHANNELS; ch++)
        {
          if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE)
          {
            //for(rk=0; rk< MAX_RANKS; rk++)
            rk=0;
            {
              if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].RankEnabled[rk] == TRUE)
              {
                //
                //  MrcData->NvData.MrcParamsSaveRestore.MuxcodeNv[] is a Non-Volatile variable. It's state is preserved
                // accross resets and available in the S0 and FB flow.
                //
                MaxPiInfo.txdll_stat_fbmuxsel_max = MrcData->NvData.MrcParamsSaveRestore.MuxcodeNv[GetCurrentFrequencyIndex()];
                if (EccEnabled && (dq==(MaxDq-1)))
                {
                  GetSetDdrioGroup2 (MrcData, ch, rk, dq, MuxcodeMin_Ecc, CMD_SET_VAL_FC_UC, &MaxPiInfo.txdll_stat_fbmuxsel_max);
                }
                MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "WarmBoot: Muxcode=%d\r\n", MaxPiInfo.txdll_stat_fbmuxsel_max));
                //break;
              } //if RankEnabled
            } //for rk
            break;
          } //if ChannelEnabled
        } //for ch
      }
      break;
    case MAXPI_DLLINIT:
      //
      //  Do the Dll Init and get the error status.  If the error status is good then do the next
      //
      DoDllInit(MrcData, &MaxPiInfo, EccEnabled);
      if (MaxPiInfo.txdll_error)
        maxpi_state =  MAXPI_TXDLL_ERROR;
      else
        maxpi_state =  MAXPI_CHECK_ACROSS_FAMILY;
      break;
    case MAXPI_TXDLL_ERROR:
      //
      //  If there is an error, decrement muxcode_min and try again.
      //  If muxcode_min is too low, then try increasing coarse range and start the process overa again
      //  If everything is good, then do the next test which is seeing if the values are same across all the instances.
      //
      for (ch=0; ch<MAX_CHANNELS; ch++)
      {
        MrcData->DynamicVars[ch][MUX_MIN]--;
      }
      DoDllInit(MrcData, &MaxPiInfo, EccEnabled);
      if (MaxPiInfo.txdll_error)
      {
        if ((FinalDecision==1) || (MrcData->DynamicVars[0][MUX_MIN]< MuxcodeMinLowerLimit))
        {
          //No Hope
          maxpi_state = MAXPI_ERROR; //If it reaches the limit
          MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "No solution\r\n"));
        }
        else
        {
          maxpi_state = MAXPI_TXDLL_ERROR;
          MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_TXDLL_ERROR: MUX_MIN set to %08x\r\n", (UINT32) MrcData->DynamicVars[0][MUX_MIN]));
        }
      }
      else
      {
        MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_TXDLL_ERROR: No erros\r\n"));
        maxpi_state = MAXPI_CHECK_ACROSS_FAMILY;
      }
      break;
    case MAXPI_CHECK_ACROSS_FAMILY:
      //
      //  Check if all the muxcode values are the same
      //
      if ((MaxPiInfo.code_count == MaxPiInfo.code_same_count) || (FinalDecision==1))
        maxpi_state = MAXPI_FINAL_VALUES;
      else
        maxpi_state = MAXPI_SETUP_WIDER_RANGE;
      //
      //  All the courses require this step.
      //
      for (ch=0; ch<MAX_CHANNELS; ch++)
      {
        //
        //  Try a wider coarse range to see if we can find a common value between all the ones
        //
        MrcData->DynamicVars[ch][COARSE_MIN] = 2;
        MrcData->DynamicVars[ch][COARSE_MAX] = 14;
        //
        // Use the Highest Muxcode to lock to
        //
        MrcData->DynamicVars[ch][MUX_MIN] = MaxPiInfo.txdll_stat_fbmuxsel_max; //Ritesh
        MrcData->DynamicVars[ch][MUX_MAX] = MaxPiInfo.txdll_stat_fbmuxsel_max;
        MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_CHECK_ACROSS_FAMILY: Setting Ch%d Coarse Range from %08x to %08x\r\n", ch, (UINT32) MrcData->DynamicVars[ch][COARSE_MIN], (UINT32)MrcData->DynamicVars[ch][COARSE_MAX]));
        MmrcExecuteTask (MrcData, maxpi_set_codes, NO_PRINT, ch);
        MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_CHECK_ACROSS_FAMILY: Setting Ch%d Muxcode Range from %08x to %08x\r\n", ch, (UINT32) MrcData->DynamicVars[ch][MUX_MIN], (UINT32) MrcData->DynamicVars[ch][MUX_MAX]));
      }
      FinalDecision=1;
      break;
    case MAXPI_SETUP_WIDER_RANGE:
      maxpi_state =  MAXPI_DLLINIT;
      break;
    case MAXPI_FINAL_VALUES:
      //
      //  Program final Values
      //
      if (MaxPiInfo.txdll_stat_fbmuxsel_max == 0)
      {
        MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_FINAL_VALUES: Error: txdll_stat_fbmuxsel_max=0.  Either No Channels are enabled, or PhyInit did not come up with a valid value\r\n"));
        return MMRC_FAILURE;
      }

      if (!RestorePath)
      {
        MrcData->NvData.MrcParamsSaveRestore.MuxcodeNv[GetCurrentFrequencyIndex()] = MaxPiInfo.txdll_stat_fbmuxsel_max;
      }

      //
      //  Fill Max Code in Phy Registers
      //
      for(ch=0; ch< MAX_CHANNELS; ch++)
      {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE)
        {
          //
          // {1x, 2x, PI}
          //
          MrcData->HalfClkPi   = (UINT16) ((MaxPiInfo.txdll_stat_fbmuxsel_max + 1 - a0offset) * 8);
          MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency][2] = 1;
          MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency][1] = MrcData->HalfClkPi;
          MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency][0] = 2 * MrcData->HalfClkPi;
          HalfClkPiFull = (UINT16) ((MaxPiInfo.txdll_stat_fbmuxsel_max + 1) * 8);
          HalfRate      =  (725 * MrcData->HalfClkPi)/HalfClkPiFull;
          if (ch==0)
          {
            MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_FINAL_VALUES: MrcData->HalfClkPi = 0x%x(%d ps Estimate) FullRangePi = 0x%x(%d ps Estimate) \r\n", MrcData->HalfClkPi, HalfRate, HalfClkPiFull, 725));
            MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "MAXPI_FINAL_VALUES: Muxcode = 0x%x\r\n", (UINT32) MaxPiInfo.txdll_stat_fbmuxsel_max));
          }

          rk=0;
          {
            if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].RankEnabled[rk] == TRUE)
            {
              {
                for (dq=0; dq < MaxDq; dq++)
                {
                  //
                  // Program the Mux Selects
                  //
                  GetSetDdrioGroup2 (MrcData, ch, rk, dq, MuxcodeMin, CMD_SET_VAL_FC_UC, &MaxPiInfo.txdll_stat_fbmuxsel_max);
                  GetSetDdrioGroup2 (MrcData, ch, rk, dq, MuxcodeMax, CMD_SET_VAL_FC_UC, &MaxPiInfo.txdll_stat_fbmuxsel_max);
                  if (EccEnabled && (dq==(MaxDq-1)))
                  {
                    GetSetDdrioGroup2 (MrcData, ch, rk, dq, MuxcodeMin_Ecc, CMD_SET_VAL_FC_UC, &MaxPiInfo.txdll_stat_fbmuxsel_max);
                    GetSetDdrioGroup2 (MrcData, ch, rk, dq, MuxcodeMax_Ecc, CMD_SET_VAL_FC_UC, &MaxPiInfo.txdll_stat_fbmuxsel_max);
                  }
                  MrcData->DynamicVars[ch][MUX_MIN] = MaxPiInfo.txdll_stat_fbmuxsel_max;
                  MrcData->DynamicVars[ch][MUX_MAX] = MaxPiInfo.txdll_stat_fbmuxsel_max;

                  //
                  // Program Phase Limits
                  //
                  Register.Offset = TXDLLREF2XCTL_DQ_BOXTYPE_REG;
                  Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
                  dq_txdll = (TXDLLREF2XCTL_DQ_BOXTYPE_STRUCT *) &Value;
                  dq_txdll->Bits.txdll_ref2xph90picode  = (MrcData->HalfClkPi * 1 /4) - 1; //Ritesh
                  dq_txdll->Bits.txdll_ref2xph180picode = (MrcData->HalfClkPi * 2 /4) - 1;
                  dq_txdll->Bits.txdll_ref2xph270picode = (MrcData->HalfClkPi * 3 /4) - 1;
                  MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
                  if (ch==0 && dq==0)
                    MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "dq_90:0x%x 180:0x%x 270:0x%x ", (UINT32) dq_txdll->Bits.txdll_ref2xph90picode, (UINT32) dq_txdll->Bits.txdll_ref2xph180picode, (UINT32) dq_txdll->Bits.txdll_ref2xph270picode));

                  if (dq == 0)
                  {
                    Register.Offset = TXDLLREF2XCTL_CCC_BOXTYPE_REG;
                    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
                    ccc_txdll = (TXDLLREF2XCTL_CCC_BOXTYPE_STRUCT *) &Value;
                    ccc_txdll->Bits.txdll_ref2xph90picode  = (MrcData->HalfClkPi * 1 /4) - 1; //Ritesh
                    ccc_txdll->Bits.txdll_ref2xph180picode = (MrcData->HalfClkPi * 2 /4) - 1;
                    ccc_txdll->Bits.txdll_ref2xph270picode = (MrcData->HalfClkPi * 3 /4) - 1;
                    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);
                    if (ch==0 && dq==0)
                      MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "ccc_90:0x%x 180:0x%x 270:0x%x ", (UINT32) ccc_txdll->Bits.txdll_ref2xph90picode, (UINT32) ccc_txdll->Bits.txdll_ref2xph180picode, (UINT32) ccc_txdll->Bits.txdll_ref2xph270picode));
                  }

                  //
                  // Program Rx Mux Selects
                  //
                  Register.Offset = RXDLLCTL_DQBLDM_DQ_BOXTYPE_REG;
                  Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
                  dq_rxdllctl_dqbldm = (RXDLLCTL_DQBLDM_DQ_BOXTYPE_STRUCT *) &Value;
                  dq_rxdllctl_dqbldm->Bits.rxdll_rxmuxsel = MIN((MaxPiInfo.txdll_stat_fbmuxsel_max * 5 / 4) + 1, 0x1F); //Limit of 31
                  MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
                  if (ch==0 && dq==0)
                    MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "dq_rxdll_rxmuxsel:0x%x \n", (UINT32) dq_rxdllctl_dqbldm->Bits.rxdll_rxmuxsel));
                  if (dq == 0)
                  {
                    Register.Offset = RXDLLCTL_DQBLDM_CCC_BOXTYPE_REG;
                    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
                    ccc_rxdllctl_dqbldm = (RXDLLCTL_DQBLDM_CCC_BOXTYPE_STRUCT *) &Value;
                    ccc_rxdllctl_dqbldm->Bits.rxdll_rxmuxsel = MIN((MaxPiInfo.txdll_stat_fbmuxsel_max * 5 / 4) + 1, 0x1F); //Limit of 31
                    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);
                    if (ch==0 && dq==0)
                      MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "ccc_rxdll_rxmuxsel:0x%x \n", (UINT32) ccc_rxdllctl_dqbldm->Bits.rxdll_rxmuxsel));
                  }

                  //
                  // DB Regions
                  //
                  Register.Offset = DBREGIONCTL1_DQDPLVUG_BOXTYPE_REG;
                  Value = MemRegRead (DQDPLVUG_BOXTYPE, ch, 0, Register);
                  dbregionctl1 = (DBREGIONCTL1_DQDPLVUG_BOXTYPE_STRUCT *) &Value;
                  dbregionctl1->Bits.maxpi_per2xclk   = MrcData->HalfClkPi;
                  dbregionctl1->Bits.dbregion1_marker = ((MrcData->HalfClkPi * 1) / 8) - 1;
                  dbregionctl1->Bits.dbregion2_marker = ((MrcData->HalfClkPi * 2) / 8) - 1;
                  dbregionctl1->Bits.dbregion3_marker = ((MrcData->HalfClkPi * 3) / 8) - 1;
                  if (ch==0 && dq==0)
                    MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "db1:0x%x, db2:0x%x, db3:0x%x ", (UINT32) dbregionctl1->Bits.dbregion1_marker, (UINT32) dbregionctl1->Bits.dbregion2_marker, (UINT32) dbregionctl1->Bits.dbregion3_marker));
                  MemRegWrite(DQDPLVUG_BOXTYPE, ch, 0, Register, Value, 0xF);
                  MemRegWrite(DQDPLVUG_BOXTYPE, ch, 1, Register, Value, 0xF);

                  //
                  // DB Regions
                  //
                  Register.Offset = DBREGIONCTL2_DQDPLVUG_BOXTYPE_REG;
                  Value = MemRegRead (DQDPLVUG_BOXTYPE, ch, 0, Register);
                  dbregionctl2 = (DBREGIONCTL2_DQDPLVUG_BOXTYPE_STRUCT *) &Value;
                  dbregionctl2->Bits.dbregion4_marker = ((MrcData->HalfClkPi * 4) / 8) - 1;
                  dbregionctl2->Bits.dbregion5_marker = ((MrcData->HalfClkPi * 5) / 8) - 1;
                  dbregionctl2->Bits.dbregion6_marker = ((MrcData->HalfClkPi * 6) / 8) - 1;
                  dbregionctl2->Bits.dbregion7_marker = ((MrcData->HalfClkPi * 7) / 8) - 1;
                  if (ch==0 && dq==0)
                    MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "db4:0x%x, db5:0x%x, db6:0x%x, db7:0x%x\r\n", (UINT32) dbregionctl2->Bits.dbregion4_marker, (UINT32) dbregionctl2->Bits.dbregion5_marker, (UINT32) dbregionctl2->Bits.dbregion6_marker, (UINT32) dbregionctl2->Bits.dbregion7_marker));
                  MemRegWrite(DQDPLVUG_BOXTYPE, ch, 0, Register, Value, 0xF);
                  MemRegWrite(DQDPLVUG_BOXTYPE, ch, 1, Register, Value, 0xF);

                  //
                  // finec
                  //
                  //DQ

                  Register.Offset = DLLCOMP_FINECTL_DQ_BOXTYPE_REG;
                  Value = MemRegRead (DQ_BOXTYPE, ch, dq, Register);
                  dq_finectl = (DLLCOMP_FINECTL_DQ_BOXTYPE_STRUCT *) &Value;
                  dq_finectl->Bits.finec_mode = 1;
                  if (ch==0 && dq==0)
                    MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "setting dq_finec=1\r\n"));
                  MemRegWrite(DQ_BOXTYPE, ch, dq, Register, Value, 0xF);
                  if (dq == 0)
                  {
                    Register.Offset = DLLCOMP_FINECTL_CCC_BOXTYPE_REG;
                    Value = MemRegRead (CCC_BOXTYPE, ch, dq, Register);
                    ccc_finectl = (DLLCOMP_FINECTL_CCC_BOXTYPE_STRUCT *) &Value;
                    ccc_finectl->Bits.finec_mode = 1;
                    if (ch==0 && dq==0)
                      MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "setting ccc_finec=1\r\n"));
                    MemRegWrite(CCC_BOXTYPE, ch, dq, Register, Value, 0xF);
                  }

                }
              }
            }
          }
        }
      }
      DoDllInit(MrcData, &MaxPiInfo, EccEnabled);
      for(ch=0; ch< MAX_CHANNELS; ch++)
      {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].RankEnabled[rk] == TRUE)
        {
          MmrcExecuteTask (MrcData, maxpi_override_coarse_final, NO_PRINT, ch);
          if (!RestorePath)
          {
            //Not the Restore Path
            //        MmrcExecuteTask (MrcData, maxpi_cmdctl_rescale, NO_PRINT, ch);
            //
            //  Assumption: This is done right after PhyInit, so all instances should have the same value..
            //
            MaxPiOldLimit = HalfClk [ADdll [MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency];
            MaxPiNewLimit = MrcData->HalfClkPi;
            if (TRUE) //MaxPiOldLimit != MaxPiNewLimit)
            {
              for (instances=0; instances<1; instances++) { //Number of Instances of the Items
                GetSetDdrioGroup2 (MrcData, ch, 0, 0, (GetOffsetIndex (MrcData, CkGrp0 + instances)), CMD_GET_REG, &(LinearValue[0]));
                GetSetDdrioGroup2 (MrcData, ch, 0, 0, (GetOffsetIndex (MrcData, CmdGrp0 + instances)), CMD_GET_REG, &(LinearValue[1]));
                GetSetDdrioGroup2 (MrcData, ch, 0, 0, (GetOffsetIndex (MrcData, CtlGrp0 + instances)), CMD_GET_REG, &(LinearValue[2]));
                for (grp=0; grp<3; grp++) {
                  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "%d:%d->", grp, LinearValue[grp]));
                  Current2x = LinearValue[grp] / GetHalfClk(MrcData, ch);
                  CurrentPi = LinearValue[grp] % GetHalfClk(MrcData, ch);
                  CalculatedPi = (CurrentPi * MaxPiNewLimit) / MaxPiOldLimit;
                  LinearValue[grp] = (Current2x * GetHalfClk(MrcData, ch)) + CalculatedPi;
                  MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "%d\r\n", LinearValue[grp]));
                }
                GetSetDdrioGroup2 (MrcData, ch, 0, 0, (GetOffsetIndex (MrcData, CkGrp0 + instances)), CMD_SET_VAL_FC_UC, &(LinearValue[0]));
                GetSetDdrioGroup2 (MrcData, ch, 0, 0, (GetOffsetIndex (MrcData, CmdGrp0 + instances)), CMD_SET_VAL_FC_UC, &(LinearValue[1]));
                GetSetDdrioGroup2 (MrcData, ch, 0, 0, (GetOffsetIndex (MrcData, CtlGrp0 + instances)), CMD_SET_VAL_FC_UC, &(LinearValue[2]));
              }
            }
            else
            {
              MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "No Limit Change, so not rescaling\r\n"));
            }
          }
          Register.Offset = PLLCTL_DPIC_BOXTYPE_REG;
          Value = MemRegRead (DPIC_BOXTYPE, ch, 0, Register);
          pllctl = (PLLCTL_DPIC_BOXTYPE_STRUCT*) &Value;
          MmrcMaxPiDllDebugPrint ((MMRC_DBG_MIN, "Ch %d:fbclktrim:0x%x\n", ch, (UINT32) pllctl->Bits.pll_cmn_fbclktrim));
          for (dq=0; dq < MaxDq; dq++)
          {
            if (!IsStrobeValid (MrcData, ch, 0, dq))
              continue;
            // Senthil/Vivek  Doesn't need to be Overridden for P0
            //OverrideFineCoarseCodes(MrcData, ch, dq, DLL_DQ);
            if (dq == 0)
            {
              if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].DramType <= TypeDdr3All)
              {
                ccc_ch=ch;
                ccc_ch0_enabled=TRUE;
                ccc_ch1_enabled=TRUE;
              }
              else
              {
                ccc_ch0_enabled=FALSE;
                ccc_ch1_enabled=FALSE;
                if ((ch==0) || (ch==1)) ccc_ch = 0;
                else if ((ch==2) || (ch==3)) ccc_ch=2;
                if ((ch==0) || (ch==2)) ccc_ch0_enabled=TRUE;
                else if ((ch==1) || (ch==3)) ccc_ch1_enabled=TRUE;
              }
            }
          }
        }
      }

      if (MaxPiInfo.txdll_error)
      {
        // DEADLOOP: MAXPI_FINAL_VALUES did not work!
        MmrcErrorPrint ((MMRC_DBG_MIN, "txdll_error\r\n"));
        MrcDeadLoop();
        return MMRC_FAILURE;
      }
      complete=1;

      break;
    case MAXPI_ERROR:
      // DEADLOOP: MAXPI Failed to find a good working Combo!
      MmrcErrorPrint ((MMRC_DBG_MIN, "MAXPI_ERROR\r\n"));
      MrcDeadLoop();
      return MMRC_FAILURE;
    default:
      break;
    }
  }
  return MMRC_SUCCESS;
}

/**
  FillChannelBitMapping

  @param[in,out]  MrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS
FillChannelBitMapping (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
   Mmrcmemcpy( MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitMapping, &(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChSwizzle[Channel]), sizeof(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitMapping));

  return MMRC_SUCCESS;
}

#if SIM || JTAG
/**
  DumpAllDDRIORegisters

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
DumpAllDDRIORegisters (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
    DumpDunitRegisters(MrcData, 0);
    DumpDdrioRegisters(MrcData, 0);
    DumpDunitRegisters(MrcData, 1);
    DumpDdrioRegisters(MrcData, 1);
    DumpDunitRegisters(MrcData, 2);
    DumpDdrioRegisters(MrcData, 2);
    DumpDunitRegisters(MrcData, 3);
    DumpDdrioRegisters(MrcData, 3);
    return MMRC_SUCCESS;
}

/**
  DumpDdrioRegisters

  @param[in,out]  MrcData
  @param[in]  Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS
DumpDdrioRegisters (
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT8      Channel
)
{
  return MMRC_SUCCESS;
}

/**
  ChangeTxToFailSimTest

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
ChangeTxToFailSimTest (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT32 Results;

  Results = 0;
  GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RxDqsDelay, CMD_SET_VAL_FC_UC, &Results);
#if SIM || JTAG
  isb32Write(0x1c, 0x8ac, 0x1, Results, 0xf);
  isb32Write(0x1c, 0xac, 0x1, Results, 0xf);
#endif
  return MMRC_SUCCESS;
}

#endif

/**
  DramPhysicalReset

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]      CapsuleStartIndex
  @param[in]      StringIndex
  @param[in]      Channelx       Unused variable. Required for MRC call table definitiion

  @retval  MMRC_STATUS
**/
MMRC_STATUS
DramPhysicalReset (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channelx
  )
{
  UINT8 ch = 0;
  UINT8 rk = 0;

  //
  //  There is only one Reset Pin in A0 that controls all channels.
  //  So we are setting the reset on all Bytelanes just incase this moves in the future.
  //
  for (ch=0; ch < MrcData->MaxNumberChannels;ch++)
  {
    if (ch==0) MmrcDebugPrint ((MMRC_DBG_MIN, "DRAM Resetting\n"));
#ifdef SIM
    MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_sim, NO_PRINT, ch);
#else
    //
    // LPDDR3 reset occurs in the InitalizeJedec routine itself, so no need to do a reset here.
    // LPDDR4 and DDR3L require the resets.
    //

    if ((ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4)) {
      UINT8 origEnable;

      //
      // Put DRAM in reset.. one per phy. (bi-channel)
      if (RunOnThisChannel (MrcData, ch, rk) || RunOnThisChannel (MrcData, ch+1, rk)) {
        origEnable = MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled;
        MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled = TRUE;
        MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_lp4_a, NO_PRINT, ch);
        MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled = origEnable;
      }

      // Dunit CKE-mode
      if (RunOnThisChannel (MrcData, ch, rk))
       MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_lp4_b, NO_PRINT, ch);

      if (RunOnThisChannel (MrcData, ch+1, rk))
         MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_lp4_b, NO_PRINT, ch+1);

      MrcDelay (MrcData, MICRO_DEL, 200);

      if (RunOnThisChannel (MrcData, ch, rk) || RunOnThisChannel (MrcData, ch+1, rk)) {
       origEnable = MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled;
       MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled = TRUE;
       MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_lp4_c, NO_PRINT, ch);
       MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled = origEnable;
      }

      MrcDelay (MrcData, MILLI_DEL, 2);

      if (RunOnThisChannel (MrcData, ch, rk))
        MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_lp4_d, NO_PRINT, ch);

      if (RunOnThisChannel (MrcData, ch+1, rk))
        MmrcExecuteTask(MrcData, modmem_init_dramreset_seq_lp4_d, NO_PRINT, ch+1);

      //
      // Increment the channel 1 more since on LP4, each phy contains 2 channels.
      //
      ch++;
    } else if (ConfigToDdrType[MrcData->MrcMemConfig] <= TypeLpDdr3) {
      MmrcExecuteTaskImplementation(MrcData, modmem_init_dramreset_seq, NO_PRINT, ch, TRUE, ch + 1);
    }
#endif
  }

  return MMRC_SUCCESS;
}
/**
  CompleteJedecInit

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]      CapsuleStartIndex
  @param[in]      StringIndex
  @param[in]      Channelx       Unused input. Required for MRC call table definition

  @retval  MMRC_STATUS
**/
MMRC_STATUS
CompleteJedecInit (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channelx
  )
{
  UINT8  Rank;
  UINT32 Value;
  UINT8 ch = 0;
  UINT8 rk = 0;
  UINT8 DramType = ConfigToDdrType[MrcData->MrcMemConfig];

  DramPhysicalReset(MrcData, CapsuleStartIndex, StringIndex, 0);

  if (DramType == TypeLpDdr4) {
    HWSetBootFrequency (MrcData, NO_PRINT, NO_PRINT, 0);
  }

  //
  //  Since we have to reset all the channels, we need to restore all channels to working state.
  //
  for (ch=0; ch < MAX_CHANNELS;ch++)
  {
    if (RunOnThisChannel (MrcData, ch, rk))
    {
      if (DramType == TypeLpDdr4) {
        SWSetHiFrequency(MrcData, NO_PRINT, NO_PRINT, ch);
        LP4MRProgramming_ODTVREF(MrcData, NO_PRINT, NO_PRINT, ch);
      }

      //MmrcDebugPrint ((MMRC_DBG_MIN, "DRAM Basic MR Programming\n"));
      InitializeJedec (MrcData, 0, NO_PRINT, ch);
      if (DramType == TypeLpDdr4) {
        //MmrcDebugPrint ((MMRC_DBG_MIN, "DRAM All MR Programming\n"));
        LP4AllMRProgramming(MrcData, NO_PRINT,  NO_PRINT, ch);
        //MmrcDebugPrint ((MMRC_DBG_MIN, "DRAM ZQCAL\n"));
        LP4MRProgramming_MPCZQ(MrcData, NO_PRINT,  NO_PRINT, ch);
      }
      //
      // For LP3, since channel and rank are shared, only that channel/rank will
      // have valid data, all others are invalid.  do not restore those, as they will
      // restore to default values.
      //
      if (DramType == TypeLpDdr3) {
            GetSetTxCaVref (MrcData, 0, 0, 0, 0, 0, 0, 0, 0, CMD_GET_REG, &Value);
            GetSetTxCaVref (MrcData, 0, 0, 0, 0, 0, 0, 0, 0, FC_WR, &Value);
            GetSetTxVref (MrcData, 0, 0, 0, 0, 0, 0, 0, 0, CMD_GET_REG, &Value);
            GetSetTxVref (MrcData, 0, 0, 0, 0, 0, 0, 0, 0, FC_WR, &Value);
      } else {
        for (Rank=0; Rank < MAX_RANKS; Rank++)
        {
          if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].RankEnabled[Rank])
          {
            GetSetTxCaVref (MrcData, 0, ch, 0, Rank, 0, 0, 0, 0, CMD_GET_REG, &Value);
            GetSetTxCaVref (MrcData, 0, ch, 0, Rank, 0, 0, 0, 0, FC_WR, &Value);
            //MmrcDebugPrint ((MMRC_DBG_MIN, "CA VREF %d\n", Value));
            GetSetTxVref (MrcData, 0, ch, 0, Rank, 0, 0, 0, 0, CMD_GET_REG, &Value);
            GetSetTxVref (MrcData, 0, ch, 0, Rank, 0, 0, 0, 0, FC_WR, &Value);
            //MmrcDebugPrint ((MMRC_DBG_MIN, "DQ VREF %d\n", Value));
          }
        }
      }
    }

    if (DramType == TypeLpDdr4) {
      MrcData->DefaultCmdClkCtl = FALSE;
      HWSetHiFrequency(MrcData, NO_PRINT, NO_PRINT, 0);
      MrcData->DefaultCmdClkCtl = TRUE;
    }
  }
  return MMRC_SUCCESS;
}

/**
  CompleteJedecInit

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
InitializeMemory (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT8             Rank;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    //Clockgate register in scrambling must match value set at end of MRC prior to clearing memory
    MmrcExecuteTask (MrcData, ECCClockGate, NO_PRINT, Channel);

    //Temp workaround test for WaiYee
    MmrcExecuteTask (MrcData, MNTEN_SET, NO_PRINT, Channel);
    if ( IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) ) {
     MEMORY_DEVICE_DENSITY Density = GetDeviceDensity( MrcData, Channel );
     UINT8  DataWidth = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0];

     //DDR3L w/ 64bit RAM interface (Table2-8)
     switch ( Density ) {
      case DeviceDensity4Gb:
       MrcData->CpgcOptions.COL = 10;
       switch (DataWidth) {
        case 1: // = x16
         MrcData->CpgcOptions.ROW = 15;
         break;

        default:
         MrcData->CpgcOptions.ROW = 16;
       } //switch DataWidth
       break;

      default:
       MrcData->CpgcOptions.ROW = 16;
       switch (DataWidth) {
        case 0: // = x8
         MrcData->CpgcOptions.COL = 11;
         break;

        default:
         MrcData->CpgcOptions.COL = 10;
       } //switch DataWidth
     } //switch Density

    } else {
     SPD_DATA SpdData = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SpdData[0];
     MrcData->CpgcOptions.ROW = ((SpdData.Buffer[SPD_DDR3_ADDRESS] & (BIT3|BIT4|BIT5)) >> 3) + 12;   //number of ROW for memory config
     MrcData->CpgcOptions.COL = (SpdData.Buffer[SPD_DDR3_ADDRESS] & (BIT0|BIT1|BIT1)) + 9;   //Number of COL for memory config
    }

    L_CpgcSetup(MrcData, Channel, CPGC_MEMORY_INIT);

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Ch %d Rank %d: MrcData.CpgcOptions.ROW %d, MrcData.CpgcOptions.COL  %d \n", Channel, Rank, MrcData->CpgcOptions.ROW, MrcData->CpgcOptions.COL));

        PerformDQReset(MrcData, Channel, Rank);
        PerformFifoReset(MrcData, Channel, Rank);
        MrcData->CpgcOptions.Rank = Rank;
        MrcData->CpgcOptions.Pattern0 = 0x0;
        MrcData->CpgcOptions.Pattern1 = 0x0;
        MrcData->CpgcOptions.CmdSequence =  CPGC_CMDSEQ_WRITE;  //to setup write
        L_CpgcReconfig(MrcData,Channel);
        L_CpgcRunTest(MrcData, Channel, NULL,NULL,NULL,NULL, NULL);
      }
    }

    //Temp workaround test for WaiYee
    MmrcExecuteTask (MrcData, MNTEN_CLR, NO_PRINT, Channel);
  }
  //half config settings
  return MMRC_SUCCESS;
}


/**
  SimMemTest

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
SimMemTest (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  return MMRC_SUCCESS;
}

/**
  CpgcVATest

  @param[in]  ModMrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
CpgcVATest (
  MMRC_DATA         *ModMrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  UINT32 BitErrorStatus0, BitErrorStatus1, BitErrorStatus2;
  UINT16 ByteErrorStatus0;
  UINT32 ErrorCount[9];
  UINT8   Rank;
  UINT16  CompareValue=0;
  INT32   i;
  UINT8   VA_LOOP_COUNT;
  UINT8   VA_NUM_BURSTS;
#define IS_SIM ((1 << ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform) == P_SIM)
   MmrcDebugPrint((MMRC_DBG_MAX, "\n\nCPGC Results:\n\n"));

  PRINT_FUNCTION_INFO;
#if SIM
    VA_LOOP_COUNT=1;
    VA_NUM_BURSTS=3;
#else
    VA_LOOP_COUNT=11; //change to 14 for max stress
    VA_NUM_BURSTS=12; //change to 14 for richer pattern generation
#endif

  ModMrcData->CpgcOptions.LoopCount = VA_LOOP_COUNT;
  ModMrcData->CpgcOptions.NumBursts = VA_NUM_BURSTS;
  L_CpgcSetup( ModMrcData, Channel, CPGC_CONFIG_VA );


  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    //
    // Only perform WRLevel training on the ranks that are enabled.
    //
    if (ModMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
      ModMrcData->CpgcOptions.Rank = Rank;
      L_CpgcReconfig( ModMrcData, Channel );

      for (i=0; i< 1; i++)
         {
        //PrechargeAll (ModMrcData, Channel, Rank);  // Ron added this.  This will add greater determinism because we have refresh on, and so precharge

        //
        // Perform the required memory access.
        //
        L_CpgcRunTest (ModMrcData, Channel, &ByteErrorStatus0, &BitErrorStatus0, &BitErrorStatus1, &BitErrorStatus2, ErrorCount);
        MmrcDebugPrint((MMRC_DBG_MIN, "Ch%d Rk%d Bits: %08X %08X  ", Channel, Rank, BitErrorStatus1, BitErrorStatus0));
        //MemDebugPrint((SDBG_MAX, "Chnk: %02X  \n", MrcData->DynamicVars[Channel][CHUNK_STAT]));
        CompareValue |= ByteErrorStatus0;
         }
    } // MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]
  } // Rank loop ...

  MmrcDebugPrint ((MMRC_DBG_MIN, "VA Results = 0x%02x\n", CompareValue));
  if (CompareValue) {
    MmrcErrorPrint((MMRC_DBG_MIN, "<!> CPGC Failure - cannot continue!\n"));
    MrcDeadLoop();
  }
  return MMRC_SUCCESS;

}

/**
  MrcDelay

  @param[in]  MrcData
  @param[in]  Type
  @param[in]  Delay

  @retval  MMRC_STATUS
**/
MMRC_STATUS
MrcDelay (
  IN    MMRC_DATA      *MrcData,
  IN        UINT8   Type,
  IN        UINT32  Delay
)
{

  UINT32  TimeCounter;

  switch (Type) {
  case MILLI_DEL:
    for (TimeCounter = 0; TimeCounter < Delay; TimeCounter++) {
      McStall (1000);
    }
    break;
  case MICRO_DEL:
    McStall (Delay);
    break;
  case NANO_DEL:
    if (Delay/1000 == 0) {
      McStall (1);
    } else {
      McStall (Delay/1000);//McStallNanoSecond (Delay);
    }
    break;
  default:
    McStall (Delay);
    break;
  }



  return MMRC_SUCCESS;
}


/**
  This routine attempts to acquire the SMBus

  @param[in]  SmbusBase

  @retval    MMRC_FAILURE as failed
  @retval    MMRC_SUCCESS as passed
**/
MMRC_STATUS
MrcAcquireBus (
  IN      UINT16    SmbusBase
)
{
  UINT8 StsReg;

  StsReg  = 0;
  StsReg  = (UINT8) IoIn8 (SmbusBase + R_PCH_SMBUS_HSTS);
  if (StsReg & B_PCH_SMBUS_IUS) {
    return MMRC_FAILURE;
  } else if (StsReg & B_PCH_SMBUS_HBSY) {
    //
    // Clear Status Register and exit
    //
    // Wait for HSTS.HBSY to be clear
    do {
      StsReg = (UINT8) IoIn8 (SmbusBase + R_PCH_SMBUS_HSTS);
    } while ((StsReg & B_PCH_SMBUS_HBSY) != 0);

    // Clear all status bits
    IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, 0xFE);
    return MMRC_SUCCESS;
  } else {
    //
    // Clear out any odd status information (Will Not Clear In Use)
    //
    IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, StsReg);
    return MMRC_SUCCESS;
  }
}

/**
  This routine reads SysCtl registers

  @param[in]  SmbusBase   SMBUS Base Address
  @param[in]  SlvAddr     Targeted Smbus Slave device address
  @param[in]  Operation   Which SMBus protocol will be used
  @param[in]  Offset      Offset of the register
  @param[in]  Length      Number of bytes
  @param[in]  Buffer      Buffer contains values read from registers

  @retval    Others as failed
  @retval    MMRC_SUCCESS as passed
**/
MMRC_STATUS
MrcSmbusExec (
  IN        UINT16    SmbusBase,
  IN        UINT8     SlvAddr,
  IN        UINT8     Operation,
  IN        UINT8     Offset,
  IN        UINT8     *Length,
  IN        UINT8     *Buffer
)
{

#if MINIBIOS
  return MMRC_SUCCESS;
#else

  MMRC_STATUS  Status;
  UINT8       SmbusOperation = 0;
  UINT8       BlockCount = 0;
#if !defined SIM  && !defined JTAG && !defined MINIBIOS
  UINT8       AuxcReg;
  UINT8       SlvAddrReg;
  UINT8       HostCmdReg;
  UINT8       Index;
  UINT8       *CallBuffer;
  BOOLEAN     BufferTooSmall;
  UINT8       StsReg;
#endif
  UINT8       RetryCount = BUS_TRIES;

  //
  // MrcSmbusExec supports byte and block read.
  // Only allow Byte or block access
  //
  if (! ((*Length  == VF_SC_BYTE_LEN) || (*Length == VF_SC_BLOCK_LEN) ) ) {
    return MMRC_FAILURE;
  }

#if SIM || JTAG || MINIBIOS

  *Buffer = myReadSPDByte (SmbusBase, SlvAddr, Offset);
  if (Offset == 0x2 && *Buffer == 0) {
    return MMRC_FAILURE;
  } else {
    return MMRC_SUCCESS;
  }

#else
  //
  // See if its ok to use the bus based upon INUSE_STS bit.
  //
  Status = MrcAcquireBus (SmbusBase);
  if (Status == MMRC_FAILURE) {
    return Status;
  }

  CallBuffer = Buffer;

  //
  //SmbStatus Bits of interest
  //[6] = IUS (In Use Status)
  //[4] = FAIL
  //[3] = BERR (Bus Error = transaction collision)
  //[2] = DERR (Device Error = Illegal Command Field, Unclaimed Cycle, Host Device Timeout, CRC Error)
  //[1] = INTR (Successful completion of last command)
  //[0] = HOST BUSY
  // This is the main operation loop.  If the operation results in a Smbus
  // collision with another master on the bus, it attempts the requested
  // transaction again at least BUS_TRIES attempts.
  //
  while (RetryCount--) {
    //
    // Operation Specifics (pre-execution)
    //
    Status          = MMRC_SUCCESS;
    SlvAddrReg      = SlvAddr;
    HostCmdReg      = Offset;
    AuxcReg         = 0;

    switch (Operation) {

    case SmbusWriteByte:
      IoOut8 (SmbusBase + R_PCH_SMBUS_HD0, CallBuffer[0]);
      SmbusOperation = V_PCH_SMBUS_SMB_CMD_BYTE_DATA;
      break;

    case SmbusReadByte:
      SmbusOperation = V_PCH_SMBUS_SMB_CMD_BYTE_DATA;
      SlvAddrReg |= B_PCH_SMBUS_RW_SEL_READ;
      if (*Length < 1) {
        Status = MMRC_FAILURE;
      }
      *Length = 1;
      break;

    case SmbusWriteBlock:
      SmbusOperation  = V_PCH_SMBUS_SMB_CMD_BLOCK;
      IoOut8 (SmbusBase + R_PCH_SMBUS_HD0, * (UINT8 *) Length);
      BlockCount = (UINT8) (*Length);
      if ((*Length < 1) || (*Length > 32) ) {
        Status = MMRC_FAILURE;
        break;
      }
      AuxcReg |= B_PCH_SMBUS_E32B;
      break;

    case SmbusReadBlock:
      SmbusOperation = V_PCH_SMBUS_SMB_CMD_BLOCK;
      SlvAddrReg |= B_PCH_SMBUS_RW_SEL_READ;
      if ((*Length < 1) || (*Length > 32) ) {
        Status = MMRC_FAILURE;
        break;
      }
      AuxcReg |= B_PCH_SMBUS_E32B;
      break;

    default:
      Status = MMRC_FAILURE;
      break;
    }

    //
    // Set Auxiliary Control register
    //
    IoOut8 (SmbusBase + R_PCH_SMBUS_AUXC, AuxcReg);

    //
    // Reset the pointer of the internal buffer
    //
    IoIn8 (SmbusBase + R_PCH_SMBUS_HCTL);

    //
    // Now that the 32 byte buffer is turned on, we can write th block data
    // into it
    //
    if (Operation == SmbusWriteBlock) {
      for (Index = 0; Index < BlockCount; Index++) {
        //
        // Write next byte
        //
        IoOut8 (SmbusBase + R_PCH_SMBUS_HBD, CallBuffer[Index]);
      }
    }

    //
    // Set SMBus slave address for the device to read
    //
    IoOut8 (SmbusBase + R_PCH_SMBUS_TSA, SlvAddrReg);

    //
    //
    // Set Command register for the offset to read
    //
    IoOut8 (SmbusBase + R_PCH_SMBUS_HCMD, HostCmdReg);

    //
    // Set Control Register to Set "operation command" protocol and start bit
    //
    IoOut8 (SmbusBase + R_PCH_SMBUS_HCTL, (UINT8) (SmbusOperation + B_PCH_SMBUS_START) );

    //
    // Wait for IO to complete
    //
    do {
      StsReg = (UINT8) IoIn8 (SmbusBase + 0);
    } while ((StsReg & (BIT4 | BIT3 | BIT2 | BIT1) ) == 0);

    if (StsReg & B_PCH_SMBUS_DERR) {
      Status = MMRC_FAILURE;
      break;
    } else if (StsReg & B_PCH_SMBUS_BERR) {
      //
      // Clear the Bus Error for another try
      //
      Status = MMRC_FAILURE;
      IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);
      //
      // Clear Status Registers
      //
      IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
      IoOut8 (SmbusBase + R_PCH_SMBUS_AUXS, B_PCH_SMBUS_CRCE);

      continue;
    }

    //
    // successfull completion
    // Operation Specifics (post-execution)
    //
    switch (Operation) {

    case SmbusReadByte:
      CallBuffer[0] = (UINT8) (IoIn8 (SmbusBase + R_PCH_SMBUS_HD0) );
      break;

    case SmbusWriteBlock:
      IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BYTE_DONE_STS);
      break;

    case SmbusReadBlock:
      BufferTooSmall = FALSE;
      //
      // Find out how many bytes will be in the block
      //
      BlockCount = (UINT8) (IoIn8 (SmbusBase + R_PCH_SMBUS_HD0) );
      if (*Length < BlockCount) {
        BufferTooSmall = TRUE;
      } else {
        for (Index = 0; Index < BlockCount; Index++) {
          //
          // Read the byte
          //
          CallBuffer[Index] = (UINT8) IoIn8 (SmbusBase + R_PCH_SMBUS_HBD);
        }
      }

      *Length = BlockCount;
      if (BufferTooSmall) {
        Status = MMRC_FAILURE;
      }
      break;

    default:
      break;
    };

    if ((StsReg & B_PCH_SMBUS_BERR) && (Status != MMRC_FAILURE)) {
      //
      // Clear the Bus Error for another try
      //
      Status = MMRC_FAILURE;
      IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);

      continue;
    } else {
      break;
    }
  }

  //
  // Clear Status Registers and exit
  //
  IoOut8 (SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
  IoOut8 (SmbusBase + R_PCH_SMBUS_AUXS, B_PCH_SMBUS_CRCE);
  IoOut8 (SmbusBase + R_PCH_SMBUS_AUXC, 0);
#endif
  return Status;
#endif
}

/**
  ConvertLinearToPhysicalValue

  @param[in,out]  MrcData
  @param[in]  DimIndex
  @param[in]  LinearValue

  @retval  Data
**/
UINT32
ConvertLinearToPhysicalValue (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT16         DimIndex,
  IN        UINT32        LinearValue
)
{
  UINT32  PhysicalValue;
#ifdef USE_LEGACY_LINEAR_TO_PHYSICAL
  UINT8   TotalSize;
  UINT8   *LinearTable;
  UINT8   DummyValue;

  TotalSize     = 0;
  DummyValue    = 0;
  PhysicalValue = 0;
  LinearTable   = (UINT8 *)&DummyValue;

  if (DimIndex == RX_VREF) {
    TotalSize = MAX_ELEMENT_TABLE;
    LinearTable = (UINT8 *) LinearToPhysicalVrefCodes;
  } else if (DimIndex == TX_VREF){
    TotalSize = MrcData->OemMrcData.PhyVrefTable.SizeOfLtoPTable;
    LinearTable = (UINT8 *)MrcData->OemMrcData.PhyVrefTable.LinearToPhysicalTable;
  } else {
    PhysicalValue = LinearValue;
  }

  if (LinearValue >= TotalSize && TotalSize != 0) {
    MmrcErrorPrint ((MMRC_DBG_MIN, "Warning!! TotalSize = %d Linear = %d \n", TotalSize, LinearValue));
    MrcDeadLoop ();
  } else if (TotalSize != 0) {
    PhysicalValue = LinearTable[LinearValue];
  }
  return PhysicalValue;
#else
  if (((DimIndex >= RxVocVal0) && (DimIndex <= RxVocVal7)) || ((DimIndex >= RxVocVal0_Ecc) && (DimIndex <= RxVocVal7_Ecc))) {
    PhysicalValue= LinearValue > 7 ? LinearValue-8 : 15-LinearValue;
  }
  else
  {
    PhysicalValue = LinearValue;
  }
  return PhysicalValue;
#endif
}

/**
  ConvertPhysicalToLinearValue

  @param[in,out]  MrcData
  @param[in]  DimIndex
  @param[in]  PhysicalValue

  @retval  Data
**/
UINT32
ConvertPhysicalToLinearValue (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT16         DimIndex,
  IN        UINT32        PhysicalValue
)
{
  UINT32  LinearValue;
#if USE_LEGACY_LINEAR_TO_PHYSICAL
  UINT8   TotalSize;
  UINT8   *PhysicalTable;
  UINT8   DummyValue;

  TotalSize     = 0;
  DummyValue    = 0;
  LinearValue   = 0;
  PhysicalTable = (UINT8 *)&DummyValue;

  if (DimIndex == RX_VREF) {
    TotalSize = MAX_ELEMENT_TABLE;
    PhysicalTable = (UINT8 *) PhysicalToLinearVrefCodes;
  } else if (DimIndex == TX_VREF){
    TotalSize = MrcData->OemMrcData.PhyVrefTable.SizeOfPtoLTable;
    PhysicalTable = (UINT8 *) MrcData->OemMrcData.PhyVrefTable.PhysicalToLinearTable;
  } else {
    LinearValue = PhysicalValue;
  }

  if (PhysicalValue >= TotalSize && TotalSize != 0) {
    MmrcErrorPrint ((MMRC_DBG_MIN, "Warning!! TotalSize = %d Physical = %d \n", TotalSize, PhysicalValue));
    MrcDeadLoop ();
  } else if (TotalSize != 0) {
    LinearValue = PhysicalTable[PhysicalValue];
  }
  return LinearValue;
#else
  if (((DimIndex >= RxVocVal0) && (DimIndex <= RxVocVal7)) || ((DimIndex >= RxVocVal0_Ecc) && (DimIndex <= RxVocVal7_Ecc))) {
    LinearValue= PhysicalValue > 7 ? 15-PhysicalValue : PhysicalValue+8;
  }
  else
  {
    LinearValue = PhysicalValue;
  }
  return LinearValue;
#endif
}

/**
  PerformDQReset

  @param[in,out]  MrcData
  @param[in]  Channel
  @param[in]  Rank


  @retval  MMRC_STATUS
**/
MMRC_STATUS
PerformDQReset (
  IN OUT   MMRC_DATA      *MrcData,
  IN       UINT8          Channel,
  IN       UINT8          Rank
)
{
  UINT32 TempValue;
  UINT8  Strobe;

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
  TempValue = 0;
    if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, DQS_CNT_RESET, CMD_SET_VAL_FC, &TempValue);
    } else {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, DQS_CNT_RESET_Ecc, CMD_SET_VAL_FC, &TempValue);
    }
  }
    #if SIM
      MySimStall (50);
    #else
      MrcDelay (MrcData, NANO_DEL, 20);
    #endif
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    TempValue = 1;
    if (Strobe < MAX_STROBES_NON_ECC) {
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, DQS_CNT_RESET, CMD_SET_VAL_FC, &TempValue);
    } else {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, DQS_CNT_RESET_Ecc, CMD_SET_VAL_FC, &TempValue);
    }
  }
  return MMRC_SUCCESS;
}

/**
  Executes and prints (if enabled) the MR values

  @param[in,out]  MrcData
  @param[in]  Channel

  @retval  Data
**/
UINT32
SendDramCmd (
  MMRC_DATA         *MrcData,
  UINT8             Channel)
{
  UINT32 data;

#ifdef PRINT_MRR_OPS
  MmrcVitalPrint ((MMRC_DBG_MIN, "MRR CH:%02X RK:%02X chunk1:%02X chunk2:%02X chunk3:%02X chunk4:%02X\n",
    Channel,
    MrcData->DynamicVars[Channel][RANK],
    (MrcData->DynamicVars[Channel][REG_DATA] & 0x3F),
    ((MrcData->DynamicVars[Channel][REG_DATA] >>  6) & 0x3F),
    ((MrcData->DynamicVars[Channel][REG_DATA] >> 12) & 0x3F),
    ((MrcData->DynamicVars[Channel][REG_DATA] >> 18) & 0x3F)
    ));
#endif
  data = (UINT32) ((MrcData->DynamicVars[Channel][REG_DATA]) | (MrcData->DynamicVars[Channel][DRAM_CMD_BL]<<28) | (MrcData->DynamicVars[Channel][RDDCMD]<<29)| (MrcData->DynamicVars[Channel][RANK]<<30));
  HandleAddressMirroring (&data, MrcData->EnableAddressMirroring);
  MrcData->DynamicVars[Channel][REG_DATA] = data & 0xffffff;
  MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);

  return MMRC_SUCCESS;
}

/**
  ReadMRValue_Deswizzle
  Perform DDR Mode Register Read [MRR] and deswizzle if requested.

  @param[in,out]  MrcData
  @param[in]  Channel
  @param[in]  MRNumber
  @param[in]  Deswizzle

  @retval  Data
**/
UINT8
  ReadMRValue_Deswizzle (
  IN OUT MMRC_DATA *MrcData,
  IN UINT8 Channel,
  IN UINT8 MRNumber,
  IN BOOLEAN Deswizzle
  )
 {
  REGISTER_ACCESS     Register;
  D_CR_DCMDDATA_DUNIT_BOXTYPE_STRUCT CrDCmdData;

  switch ( MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType ) {
   case TypeLpDdr3 :
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
      MmrcDebugPrint ((MMRC_DBG_MIN, "DramType is LPDDR3 %d\n", MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType ));
#endif
      MrcData->DynamicVars[Channel][REG_DATA] = MRR_LP3(MRNumber);
      MrcData->DynamicVars[Channel][RDDCMD]   = 1;
      //JEDEC spec JESD209-3 pp18 mr1 says Burst Length = 8; all others are reserved
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1; //HAS pp90 BurstLength=8
      SendDramCmd(MrcData, Channel);
      break;

   case TypeLpDdr4 :
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
      MmrcDebugPrint ((MMRC_DBG_MIN, "DramType is LPDDR4 %d\n", MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType ));
#endif
      MrcData->DynamicVars[Channel][REG_DATA] = MRR_LP4(MRNumber);
      MrcData->DynamicVars[Channel][RDDCMD]   = 1;
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
      SendDramCmd(MrcData, Channel);
      break;

   case TypeWio2 :
      MmrcDebugPrint ((MMRC_DBG_MIN, "DramType is TypeWio2 %d\n", MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType ));
      break;

  }

  MrcDelay (MrcData, MICRO_DEL, 10);

  Register.Offset = D_CR_DCMDDATA_DUNIT_BOXTYPE_REG;
  Register.Mask = 0xFFFFFFFF;
  Register.ShiftBit = 0;
  CrDCmdData.Data = (UINT32)MemRegRead(DUNIT_BOXTYPE, Channel, 0, Register);
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  MmrcDebugPrint ((MMRC_DBG_MIN, "MemRegRead(%d,%d) returns 0x%08x\n", Channel, MRNumber, CrDCmdData.Data ));
#endif

  if (Deswizzle) {
    UINT32 SwizzledValue;
    UINT32 ValueToSwizzle;
    //
    // MR4_BYTE_SEL determines which byte the DUNIT will place in to CR_D_CMDDATA.
    // Before swizzling the returned data from CR_D_CMDDATA, the data needs to be shifted to the correct byte position.
    //
    ValueToSwizzle = (((UINT32)CrDCmdData.Bits.mrrdata) << (MrcData->DynamicVars[Channel][MR4_BYTE_SEL] * NUMBER_BITS_IN_BYTE));
    SwizzledValue = Swizzle(MrcData, Channel, ValueToSwizzle);
#if ( MRC_DEBUG_DISABLE == 0 )
    MmrcDebugPrint((MMRC_DBG_MIN, "0x%08x Swizzle to 0x%08x\n", ValueToSwizzle, SwizzledValue));
#endif
    CrDCmdData.Bits.mrrdata = SwizzledValue;
  }
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
   else {
   MmrcDebugPrint ((MMRC_DBG_MIN, " no DeSwizzle requested.\n" ));
  }
#endif

  return (UINT8) CrDCmdData.Bits.mrrdata;
}

/**
  ReadMRValue

  @param[in,out]  MrcData
  @param[in]  Channel
  @param[in]  MRNumber

  @retval  Data
**/
UINT32
  ReadMRValue (
  IN OUT MMRC_DATA *MrcData,
  IN UINT8 Channel,
  IN UINT8 MRNumber
  )
 {
   return (UINT32) ReadMRValue_Deswizzle( MrcData, Channel, MRNumber, TRUE);
 }

/**
  ReadMRTQREF

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
ReadMRTQREF(
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT16      CapsuleStartIndex,
  IN        UINT16      StringIndex,
  IN        UINT8       Channel
  )
{

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    if (MrcData->BootMode == S5) {
      //
      // LP4_MR0VALUE is part of MRC_PARAMS_SAVE_RESTORE_DEF
      // meaning that their values will be preserved in NVRAM across resets.
      //
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].LP4_MR0VALUE = (UINT16)ReadMRValue_Deswizzle(MrcData, Channel, 0, TRUE);
    }
    //MR_VALUE Is not used for LPDDR4 as it's a don't care per spec.
    MrcData->DynamicVars[Channel][MR_VALUE] = 0;
    if ( (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].LP4_MR0VALUE & BIT0) == BIT0) {
      MrcData->DynamicVars[Channel][REFWMPNC] = 3;
      MrcData->DynamicVars[Channel][REFWMHI] = 2;
    } else {
      MrcData->DynamicVars[Channel][REFWMPNC] = 7;
      MrcData->DynamicVars[Channel][REFWMHI] = 5;
    }
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    MrcData->DynamicVars[Channel][MR_VALUE] = 0;
  } else {
    MrcData->DynamicVars[Channel][MR_VALUE] = MrcData->NvData.MrcParamsSaveRestore.DDR3L_MRVALUE;
  }
  MmrcDebugPrint((MMRC_DBG_MIN, "<!> FIFO Reset %d\n", Channel));
  PerformFifoReset(MrcData, Channel, 0);

  return MMRC_SUCCESS;
}



MMRC_STATUS
MmrcCheckECCEnable (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel)
{

    //*** nedd to move after InitDunit..!!!!
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {  // It's ECC dimm, Read fusestat[0] for ecc
    MmrcExecuteTask (MrcData, ReadFuses, NO_PRINT, Channel);
    if (MrcData->DynamicVars[Channel][FUSESTAT] & BIT0) {
      MrcData->DynamicVars[Channel][ECCEN] = 0;
      MrcData->DynamicVars[Channel][ECCEN_BXT_P_POST_A0]  = 0;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = 0;
      MmrcDebugPrint ((MMRC_DBG_MIN, "Channel %d: DDR3 ECC FORCED disabled\n",Channel));
    } else {
      MrcData->DynamicVars[Channel][ECCEN] = 1;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = 1;
      if (MrcData->CpuStepping != STEPPING_P0) {
        MrcData->DynamicVars[Channel][ECCEN_BXT_P_POST_A0]  = 1;
      }
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] = MAX_STROBES_DDR3L_ECC;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[1] = MAX_STROBES_DDR3L_ECC;
    }
    MmrcExecuteTask (MrcData, ProgramECCEnable, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, ECC_PHY_INIT, NO_PRINT, Channel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
MmrcCheckFreqConfig (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{

  UINT8 SecondChannel = 0;
  UINT8 Ch;

#if !SIM
  MmrcExecuteTaskImplementation(MrcData, ReadPunit, NO_PRINT, 0, TRUE, 1);
#endif
  GetSupportedFrequencies (MrcData, &MrcData->FreqHi);
  MmrcDebugPrint((MMRC_DBG_MIN, " FreqHi %d\n",  MrcData->FreqHi));

#if SIM || JTAG
  MrcData->FreqLo = MMRC_1600;
  MrcData->FreqHi = MMRC_3200;
#endif
  switch(MrcData->DynamicVars[0][SABY2CLK]) {
  case 24:
    MrcData->SABy2Clk = 200;
    break;
  case 30:
    MrcData->SABy2Clk = 250;
    break;
  case 32:
    MrcData->SABy2Clk = 267;
    break;
  case 40:
    MrcData->SABy2Clk = 333;
    break;
  case 48:
    MrcData->SABy2Clk = 400;
    break;
  default:
    MrcData->SABy2Clk = 200;
  }

  for (Ch=0; Ch < MAX_CHANNELS; Ch++) {
    switch (ConfigToDdrType[MrcData->MrcMemConfig]) {
    case TypeDdr3L:
      SecondChannel = 1;
      //HSD https://hsdes.intel.com/home/default.html/article?id=1206375614  preSi
      //HSD https://hsdes.intel.com/home/default.html#article?id=1206683245  postSi
      if ((MrcData->DynamicVars[0][SABY2CLK] == 24) && (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_1866)) {
        MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 7;
        MrcData->DynamicVars[Ch][BLKRDBF] = 1;
      } else {
        MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 15;
        MrcData->DynamicVars[Ch][BLKRDBF] = 1;
      }
      break;
    case TypeLpDdr3:
      SecondChannel = 2;
      if ((MrcData->DynamicVars[0][SABY2CLK] == 24) && (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_1866)) {
        MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 2;
        MrcData->DynamicVars[Ch][BLKRDBF] = 1;
      }
      break;
    case TypeLpDdr4:
      SecondChannel = 2;
      switch (MrcData->DynamicVars[0][SABY2CLK]) {
        case 24:
          if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_2133) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 6;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          } else if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_2400) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 5;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          } else if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_2666) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 4;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          } else if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_3200) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 3;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          }
          break;
        case 32:
          if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_2400) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 7;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          } else if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_2666) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 6;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          } else if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_3200) {
            MrcData->DynamicVars[Ch][BLKRDBF_ADD_RDDATA] = 5;
            MrcData->DynamicVars[Ch][BLKRDBF] = 1;
          }
          break;
        default:
          MrcData->DynamicVars[Ch][BLKRDBF] = 0;
      }
      break;
    default:
      MrcData->FreqHi = MMRC_1600;
      SecondChannel = 2;
      break;
    }
  }

  // Check DDR freq limitation
   if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) {
     if(MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency >  MrcData->FreqHi) {
       MmrcDebugPrint((MMRC_DBG_MIN, "SMIP>FUSE Freq\n"));
     }
   }

  //
  // Check DDR configuration limitation on 1ch model
  //

#if _VP_
  DEBUG ((EFI_D_INFO, "!!Skipping Channel limitation check for VP\n"));
#else
  ////TODO:  Check if we need to cover an extra case for lp memory
  if (MrcData->DynamicVars[0][DDR_CONFIG_LIMITATION] == 0) {   //2ch mode
      if ((MrcData->NvData.MrcParamsSaveRestore.Channel[SecondChannel].Enabled) && (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled == 0) && (MrcData->CpuStepping == STEPPING_P0)) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[SecondChannel].Enabled = 0;
        MmrcDebugPrint((MMRC_DBG_MIN, "ERROR: MrcMsgDimm0NotPopulated\n"));
        return MrcMsgDimm0NotPopulated;
      }
    } else {    // 1 channel mode, return error if both ch are enabled
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[SecondChannel].RankEnabled[0] && MrcData->NvData.MrcParamsSaveRestore.Channel[0].RankEnabled[0]) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[SecondChannel].Enabled = 0;
        MrcData->NvData.MrcParamsSaveRestore.Channel[SecondChannel].RankEnabled[0] = 0;
        MmrcDebugPrint((MMRC_DBG_MIN, "ERROR: MrcMsgOneChannelModeOnly\n"));
        return MrcMsgOneChannelModeOnly;
      }
  }

#endif

  MrcData->DynamicVars[0][FM_CH2] = 0;
  MrcData->DynamicVars[0][FM_CH0] = 0;
  MrcData->DynamicVars[0][FM_CH1] = 0;
  MrcData->DynamicVars[0][FM_CH3] = 0;

  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    //
    // DDR3L requires two PHY channels to form each 64 bit channel
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled) {
      MrcData->DynamicVars[0][FM_CH2] = 1;
      MrcData->DynamicVars[0][FM_CH0] = 1;
    }

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[SecondChannel].Enabled) {
      MrcData->DynamicVars[0][FM_CH1] = 1;
      MrcData->DynamicVars[0][FM_CH3] = 1;
    }
  } else {
    //
    // LPDDR4 and LPDDR3 can operate on independent PHY Channels
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled) {
      MrcData->DynamicVars[0][FM_CH2] = 1;
    }

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled) {
      MrcData->DynamicVars[0][FM_CH0] = 1;
    }

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[2].Enabled) {
      MrcData->DynamicVars[0][FM_CH1] = 1;
    }

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[3].Enabled) {
      MrcData->DynamicVars[0][FM_CH3] = 1;
    }
  }

  if ((MrcData->BootMode == S5) || (MrcData->BootMode == FB)) {
    MrcData->DynamicVars[0][PUNIT_REQ_TYPE] = 0;
  } else {
    MrcData->DynamicVars[0][PUNIT_REQ_TYPE] = 0x4;
  }

  if (MrcData->PlatformID !=0 ) {
   MrcData->DynamicVars[0][PUNIT_REQ_TYPE] = 0;
  }

  MrcData->DynamicVars[0][PUNIT_REQ_DATA] = ConfigToSpeedMultiple133[MrcData->MrcTimingProfile];

  return MMRC_SUCCESS;
}

MMRC_STATUS
R2RDynamicSetup (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
  /*++

Routine Description:

  Fills in the complete parameter list for the Modular MRC.  This includes all the Dynamic entries that are listed in the Input Spreadsheet, but also
  the PFCT values.

Arguments:

  MrcData:       Host structure for all data related to MMRC

  Channel:       Channel to do the conversion on.

Returns:

  Success
  Failure

--*/
{

  UINT32  TempVar;
  UINT8   Rank;
  UINT8   Strobe;
  UINT8   TrrdrOffset;
  UINT8   TwwdrOffset;
  UINT8   CurrentDramType;
  UINT8   ProfileDramFrequency;
  BOOLEAN RestorePath;

  //
  // Loop through each supported Rank and supported Strobe reading the R2Ren flags into cache.
  // Also updtae the Rk2REn for each algo.
  //
  for (Rank=0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    for (Strobe=0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++)
    {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if (Strobe < MAX_STROBES_NON_ECC ) {
        GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, R2RRcvnEn, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.RcvEn = (UINT8) TempVar;
        GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, R2RRxDqsEn, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.RxDqs = (UINT8) TempVar;
        GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, R2RTxDqEn, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.TxDq = (UINT8) TempVar;
        GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, R2RTxDqsEn, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.TxDqs = (UINT8) TempVar;
      } else {
        GetSetDdrioGroup2(MrcData, Channel, Rank, 0, R2RRcvnEn_Ecc, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.RcvEn = (UINT8) TempVar;
        GetSetDdrioGroup2(MrcData, Channel, Rank, 0, R2RRxDqsEn_Ecc, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.RxDqs = (UINT8) TempVar;
        GetSetDdrioGroup2(MrcData, Channel, Rank, 0, R2RTxDqEn_Ecc, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.TxDq = (UINT8) TempVar;
        GetSetDdrioGroup2(MrcData, Channel, Rank, 0, R2RTxDqsEn_Ecc, CMD_GET_REG_UC, &TempVar);
        MrcData->Rk2RkEn.TxDqs = (UINT8) TempVar;
      }
    }
  }

  CurrentDramType = ConfigToDdrType[MrcData->MrcMemConfig];
  ProfileDramFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
  RestorePath = (MrcData->BootMode != S5) ? TRUE : FALSE;

  // The RestorePath is handled by RestoreDataDescriptorTable in SaveRestoreAlgos()
  if (!RestorePath) {
    if (MrcData->Rk2RkEn.RxDqs == 1) {
      if(CurrentDramType == TypeLpDdr4 && ProfileDramFrequency >= MMRC_2400) {
        TrrdrOffset = 1;
      } else if (CurrentDramType == TypeDdr3L && ProfileDramFrequency >= MMRC_1866) {
        TrrdrOffset = 1;
      } else {
        TrrdrOffset = 0;
      }
      // If RXDQS RK2RK is disabled, update the tRDRD_dr timing to the "Acutal" timing
      // instead of the "Exception" timing. Update the dynamic variable and register value:
      MrcData->DynamicVars[Channel][TRRDR] = GetTimingParam(BxtTimingtRDRD_dr) + TrrdrOffset + GUARDBAND;
      GetSetDdrioGroup2(MrcData, Channel, 0, 0, TrrdrGetSet, CMD_SET_VAL_FC_UC, &MrcData->DynamicVars[Channel][TRRDR]);
    }

    if (MrcData->Rk2RkEn.TxDq == 1 || MrcData->Rk2RkEn.TxDqs == 1) {
      if(CurrentDramType == TypeLpDdr3 && ProfileDramFrequency >= MMRC_1866) {
        TwwdrOffset = 2;
      } else if(CurrentDramType == TypeLpDdr4) {
        if (ProfileDramFrequency >= MMRC_1866 && ProfileDramFrequency <= MMRC_2133) {
          TwwdrOffset = 2;
        } else if (ProfileDramFrequency >= MMRC_2400 && ProfileDramFrequency <= MMRC_2666) {
          TwwdrOffset = 1;
        } else if (ProfileDramFrequency >= MMRC_2933 && ProfileDramFrequency <= MMRC_3200) {
          TwwdrOffset = 2;
        } else {
          TwwdrOffset = 0;
        }
      } else if (CurrentDramType == TypeDdr3L && ProfileDramFrequency >= MMRC_1866) {
        TwwdrOffset = 5;
      } else {
        TwwdrOffset = 0;
      }
      // If TXDQ or TXDQS RK2RK is disabled, update the tRDRD_dr timing to the "Acutal" timing
      // instead of the "Exception" timing. Update the dynamic variable and register value:
      MrcData->DynamicVars[Channel][TWWDR] = GetTimingParam(BxtTimingtWRWR_dr) + TwwdrOffset + GUARDBAND;
      GetSetDdrioGroup2(MrcData, Channel, 0, 0, TwwdrGetSet, CMD_SET_VAL_FC_UC, &MrcData->DynamicVars[Channel][TWWDR]);
    }
  }
  return MMRC_SUCCESS;
}

UINT8
GetNumberOfEnabledChannels (
  VOID
  )
/*++

Routine Description:

  Returns the number of enabled channels that have at least one rank enabled.

Returns:

  The number of enabled channels that have at least one rank enabled.

--*/
{
  UINT8 Channel;
  UINT8 NumChannels=0;
  MMRC_DATA *MrcData = GetMrcHostStructureAddress();
  for (Channel=0; Channel < MrcData->MaxNumberChannels; Channel++){
    // On BXT, Rank 0 must be populated if the channel is enabled.
    if(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled == 1 && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0] == 1){
      NumChannels++;
    }
  }
  return NumChannels;
}

UINT16
GetSpidClk (
  VOID
  )
{
  MMRC_DATA *MrcData;
  MrcData = GetMrcHostStructureAddress();
  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4) {
    return SPIDCLK_LP4[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency];
  } else {
    return SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency];
  }
}

/**
  Applies the the Custom TSV settings as described in the
  BXT DUNIT HAS Section 3.2.2.4: Custom TSV Patterns

  @param[in]  MrcData       Host structure for all data related to MMRC.
  @param[in]  Channel       The channel to perfrom the operation
**/
VOID
SetupCustomTsvVariables (
  MMRC_DATA *MrcData,
  UINT8     Channel
  )
{
  UINT8 i;
  UINT8 CurrentDramType;

  CurrentDramType = ConfigToDdrType[MrcData->MrcMemConfig];
  if ( ((CurrentDramType == TypeLpDdr3) || (CurrentDramType == TypeDdr3L)) &&
       (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_1866)) {

      MrcData->DynamicVars[Channel][INIT_CMDSPID_TO_SABY2_TSVS] = 0x0;
      MrcData->DynamicVars[Channel][INIT_SABY2_TO_CMDSPID_TSVS] = 0x0;
      MrcData->DynamicVars[Channel][INIT_DATASPID_TO_SABY2_TSVS] = 0x0;
      MrcData->DynamicVars[Channel][INIT_SABY2_TO_DATASPID_TSVS] = 0x0;

      MrcData->DynamicVars[Channel][PATTERN_SABY2_TO_CMDSPID_TSVS] = 0x0;
      MrcData->DynamicVars[Channel][PATTERN_SABY2_TO_DATASPID_TSVS] = 0x0;
      MrcData->DynamicVars[Channel][PATTERN_CMDSPID_TO_SABY2_TSVS] = 0x0;
      MrcData->DynamicVars[Channel][PATTERN_DATASPID_TO_SABY2_TSVS] = 0x0;

      return;
  }

  #define RANGE_PLUS_OR_MINUS_ONE 1
  MmrcDebugPrint((MMRC_DBG_MIN, "SABy2Clk: %d; SpidClk: %d\n", MrcData->SABy2Clk, GetSpidClk()));
  for (i = 0; i < BgfCustomTsvSize; i++){
    //
    // When comparing the Saby2Freq and SpidFreq, use a +/- 1 range to account for differences
    // in rounding between the two tables.
    //
    if ( InRange(BgfCustomTsv[i].Saby2Freq, MrcData->SABy2Clk, RANGE_PLUS_OR_MINUS_ONE)
      && InRange(BgfCustomTsv[i].SpidFreq,GetSpidClk(), RANGE_PLUS_OR_MINUS_ONE) ) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "CustomTSV: %d\n", i));
      MrcData->DynamicVars[Channel][INIT_SABY2_TO_CMDSPID_TSVS]     = BgfCustomTsv[i].init_saby2_to_spid_tsvs;
      MrcData->DynamicVars[Channel][INIT_SABY2_TO_DATASPID_TSVS]    = BgfCustomTsv[i].init_saby2_to_spid_tsvs;
      MrcData->DynamicVars[Channel][PATTERN_SABY2_TO_CMDSPID_TSVS]  = BgfCustomTsv[i].pattern_saby2_to_spid_tsvs;
      MrcData->DynamicVars[Channel][PATTERN_SABY2_TO_DATASPID_TSVS] = BgfCustomTsv[i].pattern_saby2_to_spid_tsvs;

      MrcData->DynamicVars[Channel][INIT_CMDSPID_TO_SABY2_TSVS]     = BgfCustomTsv[i].init_spid_to_saby2_tsvs;
      MrcData->DynamicVars[Channel][INIT_DATASPID_TO_SABY2_TSVS]    = BgfCustomTsv[i].init_spid_to_saby2_tsvs;
      MrcData->DynamicVars[Channel][PATTERN_CMDSPID_TO_SABY2_TSVS]  = BgfCustomTsv[i].pattern_spid_to_saby2_tsvs;
      MrcData->DynamicVars[Channel][PATTERN_DATASPID_TO_SABY2_TSVS] = BgfCustomTsv[i].pattern_spid_to_saby2_tsvs;
      break;
    }
  }

  if (MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] == 2) {
    for (i = 0; i < DataBgfCustomTsvForDynamicEarlyWarningSize; i++){
      //
      // When comparing the Saby2Freq and SpidFreq, use a +/- 1 range to account for differences
      // in rounding between the two tables.
      //
      if ( InRange(DataBgfCustomTsvForDynamicEarlyWarning[i].Saby2Freq, MrcData->SABy2Clk, RANGE_PLUS_OR_MINUS_ONE)
        && InRange(DataBgfCustomTsvForDynamicEarlyWarning[i].SpidFreq, GetSpidClk(), RANGE_PLUS_OR_MINUS_ONE)) {
        MmrcDebugPrint((MMRC_DBG_MIN, "DataBgfCustomTSV: %d\n", i));
        MrcData->DynamicVars[Channel][INIT_SABY2_TO_DATASPID_TSVS]    = DataBgfCustomTsvForDynamicEarlyWarning[i].init_saby2_to_spid_tsvs;
        MrcData->DynamicVars[Channel][PATTERN_SABY2_TO_DATASPID_TSVS] = DataBgfCustomTsvForDynamicEarlyWarning[i].pattern_saby2_to_spid_tsvs;
        MrcData->DynamicVars[Channel][INIT_DATASPID_TO_SABY2_TSVS]    = DataBgfCustomTsvForDynamicEarlyWarning[i].init_spid_to_saby2_tsvs;
        MrcData->DynamicVars[Channel][PATTERN_DATASPID_TO_SABY2_TSVS] = DataBgfCustomTsvForDynamicEarlyWarning[i].pattern_spid_to_saby2_tsvs;
        break;
      }
    }
  }
}
/*++

Routine Description:

  Fills in the complete parameter list for the Modular MRC.  This includes all the Dynamic entries that are listed in the Input Spreadsheet, but also
  the PFCT values.

Arguments:

  MrcData:       Host structure for all data related to MMRC

  Channel:       Channel to do the conversion on.

Returns:

  Success
  Failure

--*/
MMRC_STATUS
MmrcConvertParams (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )

{
  UINT8 BLMode;
  UINT8 MaxDensity;
  UINT8 CurrentDramType;

  CurrentDramType = ConfigToDdrType[MrcData->MrcMemConfig];

  FillChannelBitMapping(MrcData,0,0,Channel);
  CalculateDeswizzleReverseMap(MrcData, Channel);
  if  ( CurrentDramType == TypeLpDdr4) {
    MrcData->ReadPerBitEnable = RT_PERBIT_ENABLED;
    MrcData->WritePerBitEnable = WT_PERBIT_ENABLED;
  } else{
    MrcData->ReadPerBitEnable = 0;
    MrcData->WritePerBitEnable = 0;
  }
  MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex = 0;
  Mmrcmemcpy((void* ) MrcData->GranularitySteps, (void *) GranularityGenerated, sizeof(GranularityGenerated));

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
    if (CurrentDramType == TypeDdr3L) {
      MrcData->ChEnMask |= (Channel == 1) ? 0x4 : 0x1;
  } else {
      MrcData->ChEnMask |= (1 << Channel);
    }
  }
  if (CurrentDramType == TypeDdr3L) {
    MrcData->MaxNumberStrobes  = 9;
    MrcData->MaxNumberChannels = 2;
  } else {
    MrcData->MaxNumberStrobes  = 4;
    MrcData->MaxNumberChannels = 4;
  }
  MrcData->DynamicVars[Channel][pream] = 0;
  if (CurrentDramType == TypeLpDdr3 || CurrentDramType == TypeDdr3L) {
    MrcData->DynamicVars[Channel][WRPREAM0EN] = 0;
    MrcData->DynamicVars[Channel][DQSDRVENPREAM0] = 3;
  } else if  ( CurrentDramType == TypeLpDdr4) {
    if (MrcData->DynamicVars[Channel][pream] == 1) {
      MrcData->DynamicVars[Channel][WRPREAM0EN] = 0;
      MrcData->DynamicVars[Channel][DQSDRVENPREAM0] = 3;
    } else {
      MrcData->DynamicVars[Channel][WRPREAM0EN] = 1;
      MrcData->DynamicVars[Channel][DQSDRVENPREAM0] = 0;
    }
  }
  MrcData->DynamicVars[Channel][DDEN] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0];

  /*
    "CA Swizzle Type
  00 - uniDIMM/SODIMM
  01 - BGA
  10 - BGA mirrored (LPDDR3 only)
  11 - UDIMM (DDR3L only)"
  */
  MrcData->DynamicVars[Channel][CASWIZZLE] = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Package;

  if (CurrentDramType == TypeDdr3L) {
    if (MrcData->EnableAddressMirroring) {
      MrcData->DynamicVars[Channel][CASWIZZLE] = 3;   //udimm
    } else {
      MrcData->DynamicVars[Channel][CASWIZZLE] = 0;
    }
  }
  MmrcDebugPrint((MMRC_DBG_MIN, "MrcData::AddrMirror: %d CASWIZZLE %d \n", MrcData->EnableAddressMirroring, MrcData->DynamicVars[Channel][CASWIZZLE]));

  MrcData->DynamicVars[Channel][RANK0_ENABLED] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0];
  MrcData->DynamicVars[Channel][RANK1_ENABLED] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1];
  //
  // Each bit of CKEVAL directly controls a CKE pin when CKEMODE is set to 1.
  //
  MrcData->DynamicVars[Channel][CKEVAL_VAR] = ( MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0] | (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1] << 1) );

  if((CurrentDramType == TypeLpDdr4
    || CurrentDramType == TypeLpDdr3)
    && GetNumberOfEnabledChannels() == 4) {
    MrcData->DynamicVars[Channel][ADDRDEC] = 1;
  } else if (CurrentDramType == TypeDdr3L
    && GetNumberOfEnabledChannels() == 2) {
    MrcData->DynamicVars[Channel][ADDRDEC] = 2;
  } else {
    MrcData->DynamicVars[Channel][ADDRDEC] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ChOption.Bits.AddressMapping;
  }
  MrcData->DynamicVars[Channel][BAHEN] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ChOption.Bits.BankAddressHashingEnable;
  MrcData->DynamicVars[Channel][RSIEN] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ChOption.Bits.RankSelectInterleavingEnable;

  if (CurrentDramType == TypeLpDdr4 || CurrentDramType == TypeLpDdr3) {
    switch ((SMIP_DRAM_DEVICE_WIDTH)(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0])) {
    case SMIP_DramWidthx8:
      MrcData->DynamicVars[Channel][DRAMDEVICE_PR] = DevicesPerRank4;
      break;
    case SMIP_DramWidthx16:
      MrcData->DynamicVars[Channel][DRAMDEVICE_PR] = DevicesPerRank2;
      break;
    case SMIP_DramWidthx32:
      MrcData->DynamicVars[Channel][DRAMDEVICE_PR] = DevicesPerRank1;
    default:
      MmrcErrorPrint ((MMRC_DBG_MIN, "MRC ERROR (LP): Devices Per Rank indeterminite.\n"));
    }
  }
  else { // if (CurrentDramType == TypeDdr3L)
    switch ((SMIP_DRAM_DEVICE_WIDTH)(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0])) {
    case SMIP_DramWidthx8:
      MrcData->DynamicVars[Channel][DRAMDEVICE_PR] = DevicesPerRank8;
      break;
    case SMIP_DramWidthx16:
      MrcData->DynamicVars[Channel][DRAMDEVICE_PR] = DevicesPerRank4;
      break;
    case SMIP_DramWidthx32:
      MrcData->DynamicVars[Channel][DRAMDEVICE_PR] = DevicesPerRank2;
      break;
    default:
      MmrcErrorPrint ((MMRC_DBG_MIN, "MRC ERROR (DDR3L): Devices Per Rank indeterminite.\n"));
    }
  }
  MrcData->DynamicVars[Channel][DWID] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0];


  ///***********************************//

  if (CurrentDramType == TypeLpDdr4) {
    BLMode = 32;
  } else {
    BLMode = 8;
  }
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
#if defined BXT_PO && ( BXT_PO == 1)
    MrcData->DynamicVars[Channel][MINREFRATE] = 0x3;
#else
    MrcData->DynamicVars[Channel][MINREFRATE] = 0x2;
#endif
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
#if defined BXT_PO && ( BXT_PO == 1)
      MrcData->DynamicVars[Channel][MINREFRATE] = 0x3;
#else
      MrcData->DynamicVars[Channel][MINREFRATE] = 0x2;
#endif
  } else {  //TypeDdr3L
    MrcData->DynamicVars[Channel][MINREFRATE] = 0x3;
  }

  if ((MrcData->MrcInputs.ForceMin2xRefresh) && (MrcData->DynamicVars[Channel][MINREFRATE] < 0x4)) {
    MrcData->DynamicVars[Channel][MINREFRATE] = 0x4;
  }
   MrcData->DynamicVars[Channel][SPIDCMDCLK_RATIO] = 2;
   //MrcData->DynamicVars[Channel][SPIDDATA_RATIO_WIO2] = (MrcData->DynamicVars[Channel][SPIDCMDCLK_RATIO] * 4) + 3 ;

  MrcData->DynamicVars[Channel][SACLKBY2_RATIO] = MrcData->DynamicVars[Channel][SPIDCMDCLK_RATIO];
  MrcData->DynamicVars[Channel][TRCD] = GetTimingParam(BxtTimingtRCD);

  //DTR0A /*****************/

  MrcData->DynamicVars[Channel][TRPPB] = GetTimingParam(BxtTimingtRPPB);
  MrcData->DynamicVars[Channel][RL] =  GetTimingParam(BxtTimingtCL);


  switch (CurrentDramType) {
  case TypeLpDdr4:

    MrcData->DynamicVars[Channel][TXSDLL] = 0;
    MrcData->DynamicVars[Channel][TCKSRX] = 0;
    MrcData->DynamicVars[Channel][TXSR] = GetTimingParam(BxtTimingtXSR_4G + MrcData->DynamicVars[Channel][DDEN]);
#if defined BXT_PO && (BXT_PO == 1)
    MrcData->DynamicVars[Channel][TCKSRX] = GetTimingParam(BxtTimingtCKSRX);
#endif

  break;
  case TypeLpDdr3:

    MrcData->DynamicVars[Channel][TXSDLL] = 0;
    MrcData->DynamicVars[Channel][TCKSRX] = 0;
    MrcData->DynamicVars[Channel][TXSR] = GetTimingParam(BxtTimingtXSR_4G + MrcData->DynamicVars[Channel][DDEN]);
#if defined BXT_PO && (BXT_PO == 1)
    MrcData->DynamicVars[Channel][TCKSRX] = 2;
#endif

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_1333 && MrcData->CpuStepping == STEPPING_P0) {
        MrcData->DynamicVars[Channel][TRPPB] = 14;
      }
    }

    break;
  case TypeDdr3L:
       MrcData->DynamicVars[Channel][TRPPB] = GetTimingParam(BxtTimingtRP);
     MrcData->DynamicVars[Channel][TCKSRX] = GetTimingParam(BxtTimingtCKSRX) - 4;
     MrcData->DynamicVars[Channel][TXSDLL] = 8;// 512 * TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency];
     if (MrcData->DynamicVars[Channel][DDEN] == 0) {
       MrcData->DynamicVars[Channel][TXSR] = GetTimingParam(BxtTimingtXS_4G);
     } else {
       MrcData->DynamicVars[Channel][TXSR] = GetTimingParam(BxtTimingtXS_8G);
     }
     if ( !IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) ) {
       MrcData->DynamicVars[Channel][TRCD] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTrcd];
       MrcData->DynamicVars[Channel][RL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Tcl;
       MrcData->DynamicVars[Channel][TRPPB] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTrp];
     }
       if (MrcData->DynamicVars[0][DDEN] > MrcData->DynamicVars[1][DDEN]) {
         MaxDensity = (UINT8)MrcData->DynamicVars[0][DDEN];
       } else {
         MaxDensity = (UINT8)MrcData->DynamicVars[1][DDEN];
       }
       if (MaxDensity == 0) {
       MrcData->DynamicVars[Channel][TXSR] = GetTimingParam(BxtTimingtXS_4G);
     } else {
       MrcData->DynamicVars[Channel][TXSR] = GetTimingParam(BxtTimingtXS_8G);
       }

#if defined BXT_PO && (BXT_PO == 1)
    MrcData->DynamicVars[Channel][TCKSRX] = GetTimingParam(BxtTimingtCKSRX);
#endif
  break;

  default:
    break;
  }


  //*****************/


  //  DTR1A /*****************/
  MrcData->DynamicVars[Channel][TXP] = GetTimingParam(BxtTimingtXP);

  switch (CurrentDramType) {
  case TypeLpDdr4:
    MrcData->DynamicVars[Channel][TZQCS] = 0;
    MrcData->DynamicVars[Channel][TZQCL] = GetTimingParam(BxtTimingtZQOPER);
    MrcData->DynamicVars[Channel][TZQLAT] = GetTimingParam(BxtTimingtZQLAT);
    break;
  case TypeLpDdr3:
    MrcData->DynamicVars[Channel][TZQCS] = GetTimingParam(BxtTimingtZQCS);
    MrcData->DynamicVars[Channel][TZQCL] = GetTimingParam(BxtTimingtZQOPER);
    MrcData->DynamicVars[Channel][TZQLAT] = 0;
    break;
  case TypeDdr3L:
    MrcData->DynamicVars[Channel][TZQCS] = GetTimingParam(BxtTimingtZQCS);
    MrcData->DynamicVars[Channel][TZQCL] = GetTimingParam(BxtTimingtZQOPER);;
    MrcData->DynamicVars[Channel][TZQLAT] = 0;
    break;
  default:
    break;
  }

  /*****************/


  //  DTR2A /*****************/
  MrcData->DynamicVars[Channel][NREFI] = GetTimingParam(BxtTimingtREFI);
  MrcData->DynamicVars[Channel][TCKE] = GetTimingParam(BxtTimingtCKE);
  MrcData->DynamicVars[Channel][NRFCAB] = GetTimingParam(BxtTimingtRFC_4G + MrcData->DynamicVars[Channel][DDEN]);

 //  DTR3A /*****************/
  MrcData->DynamicVars[Channel][TCWL] = GetTimingParam(BxtTimingtCWL);
  MrcData->DynamicVars[Channel][WL] = MrcData->DynamicVars[Channel][TCWL];
  MrcData->DynamicVars[Channel][TWTP] =  GetTimingParam(BxtTimingtWRPRE); //BLMode/2 + GetTimingParam(BxtTimingtCWL) + GetTimingParam(BxtTimingtWR) + 1;
  MrcData->DynamicVars[Channel][TRTP] = GetTimingParam(BxtTimingtRDPRE);

  switch (CurrentDramType) {
  case TypeLpDdr4:
      if (MrcData->CpuStepping == STEPPING_E0) {
        MrcData->DynamicVars[Channel][TCCDMW] = GetTimingParam(BxtTimingtCCDMW) + 8;
      } else {
        MrcData->DynamicVars[Channel][TCCDMW] = GetTimingParam(BxtTimingtCCDMW);
      }
      //MrcData->DynamicVars[Channel][TRTP] = (BLMode/2) + MAX(8, GetTimingParam(BxtTimingtRTP)) - 8;
    break;
  case TypeLpDdr3:
    MrcData->DynamicVars[Channel][TCCDMW] = 0;
    //MrcData->DynamicVars[Channel][TRTP] = (BLMode/2) + GetTimingParam(BxtTimingtRTP) - 4;
    break;

  case TypeDdr3L:
    MrcData->DynamicVars[Channel][TCCDMW] = 0;
       if (MrcData->DynamicVars[0][DDEN] > MrcData->DynamicVars[1][DDEN]) {
         MaxDensity = (UINT8)MrcData->DynamicVars[0][DDEN];
       } else {
         MaxDensity = (UINT8)MrcData->DynamicVars[1][DDEN];
       }
         MrcData->DynamicVars[Channel][NRFCAB] = GetTimingParam(BxtTimingtRFC_4G + MaxDensity);
    break;

  default:
    break;
  }

  if (MrcData->CpuStepping == STEPPING_P0) {
    MrcData->DynamicVars[Channel][BXT_P_A0]  = 1;
    MrcData->DynamicVars[Channel][BXT_P_POST_A0]  = 0;
    MrcData->DynamicVars[Channel][ECCEN_BXT_P_POST_A0]  = 0;
  } else {
    MrcData->DynamicVars[Channel][BXT_P_A0]  = 0;
    MrcData->DynamicVars[Channel][BXT_P_POST_A0]  = 1;
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
      MrcData->DynamicVars[Channel][ECCEN_BXT_P_POST_A0]  = 1;
    }
  }


 //  DTR4A /*****************/

  MrcData->DynamicVars[Channel][TRRDR] = GetTimingParam(BxtTimingtRDRD_dr) + GUARDBAND;
  MrcData->DynamicVars[Channel][TWWDR] = GetTimingParam(BxtTimingtWRWR_dr) + GUARDBAND;
  MrcData->DynamicVars[Channel][TWRDR] = GetTimingParam(BxtTimingtWRRD_dr) + GUARDBAND;
  MrcData->DynamicVars[Channel][TRWDR] = GetTimingParam(BxtTimingtRDWR_dr) + GUARDBAND;

  switch (CurrentDramType) {
  case TypeLpDdr4:
    MrcData->DynamicVars[Channel][TFAW] = GetTimingParam(BxtTimingtFAW2K);
    break;
  case TypeLpDdr3:
    MrcData->DynamicVars[Channel][TFAW] = GetTimingParam(BxtTimingtFAW1K);
    break;
  case TypeDdr3L:

      if (MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize == 1) {
        MrcData->DynamicVars[Channel][TFAW] = GetTimingParam(BxtTimingtFAW1K);
      } else {
        MrcData->DynamicVars[Channel][TFAW] = GetTimingParam(BxtTimingtFAW2K);
      }
#if BXT_PO
#else
      MrcData->DynamicVars[Channel][TRRDR] = GetTimingParam(BxtTimingtRDRD_dr);// + 4;
      MrcData->DynamicVars[Channel][TWWDR] = GetTimingParam(BxtTimingtWRWR_dr);// + 4;
      MrcData->DynamicVars[Channel][TWRDR] = GetTimingParam(BxtTimingtWRRD_dr);// + 4;
      MrcData->DynamicVars[Channel][TRWDR] = GetTimingParam(BxtTimingtRDWR_dr);// + 4;
#endif
    break;

  default:
    break;
  }

 //  DTR5A /*****************/
  MrcData->DynamicVars[Channel][TRWSR] = GetTimingParam(BxtTimingtRDWR) + GUARDBAND;
  MrcData->DynamicVars[Channel][TRRSR] = GetTimingParam(BxtTimingtRDRD) + GUARDBAND;
  MrcData->DynamicVars[Channel][TWRSR] = GetTimingParam(BxtTimingtWRRD) + GUARDBAND;
  MrcData->DynamicVars[Channel][TWWSR] = GetTimingParam(BxtTimingtWRWR) + GUARDBAND;

  MrcData->DynamicVars[Channel][TDERATE_INC] = CEILING ((1875) , TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]);

  MrcData->DynamicVars[Channel][TRRD] = GetTimingParam(BxtTimingtRRD);

 //  DTR6A /*****************/

  switch (CurrentDramType) {
  case TypeLpDdr4:
      MrcData->DynamicVars[Channel][TCKCKEL] = GetTimingParam(BxtTimingtCKSRE) - 4;
      MrcData->DynamicVars[Channel][TPREMRBLK] =  GetTimingParam(BxtTimingtXP) + GetTimingParam(BxtTimingtRCD) - 8;
      MrcData->DynamicVars[Channel][MNTDLY] = 0; //CEILING ((GetTimingParam(BxtTimingtRPPB) + BLMode/2 +
//         GetTimingParam(BxtTimingtRCD) + 3) , 16);
#if defined BXT_PO && BXT_PO
      MrcData->DynamicVars[Channel][TPREMRBLK] =  GetTimingParam(BxtTimingtXP) + GetTimingParam(BxtTimingtRCD);
      MrcData->DynamicVars[Channel][TCKCKEL] = GetTimingParam(BxtTimingtCKSRE);
#endif
     break;
  case TypeLpDdr3:
    MrcData->DynamicVars[Channel][TCKCKEL] = 0;
    MrcData->DynamicVars[Channel][TPREMRBLK] = GetTimingParam(BxtTimingtXP) + GetTimingParam(BxtTimingtRCD) - 4;
    MrcData->DynamicVars[Channel][MNTDLY] = 0; //CEILING ((GetTimingParam(BxtTimingtRPPB) + BLMode/2 +
 //     GetTimingParam(BxtTimingtRCD) + 3) , 8);
#if defined BXT_PO && BXT_PO
      MrcData->DynamicVars[Channel][TPREMRBLK] =  GetTimingParam(BxtTimingtXP) + GetTimingParam(BxtTimingtRCD);
      MrcData->DynamicVars[Channel][TCKCKEL] = 2;
#endif

    break;

  case TypeDdr3L:
    MrcData->DynamicVars[Channel][TCKCKEL] = 0;  ///  need to confirm BxtTimingtCKSRE;
    MrcData->DynamicVars[Channel][MNTDLY] = 0;//CEILING ((GetTimingParam(BxtTimingtRP) + BLMode/2 +
    //  GetTimingParam(BxtTimingtRCD) + 3) , 8);

    MrcData->DynamicVars[Channel][TPREMRBLK] = MrcData->DynamicVars[Channel][WL] + 1 + BLMode/2 - 8;

#if defined BXT_PO && BXT_PO
    MrcData->DynamicVars[Channel][TCKCKEL] = 2;
#endif
    break;

  default:
    break;
  }

#if defined BXT_PO && BXT_PO
      MrcData->DynamicVars[Channel][MNTDLY] =  0;
#endif

 //  DTR7A /*****************/
  MrcData->DynamicVars[Channel][TRDPDEN] = GetTimingParam(BxtTimingtRDPDEN);
  MrcData->DynamicVars[Channel][TWRPDEN] = GetTimingParam(BxtTimingtWRPDEN);
  MrcData->DynamicVars[Channel][TRPAB] = GetTimingParam(BxtTimingtRPab);
  MrcData->DynamicVars[Channel][TPSTMRWBLK] = CEILING (GetTimingParam(BxtTimingtMOD), 8 );
  MrcData->DynamicVars[Channel][TRAS] = GetTimingParam(BxtTimingtRAS);

  if (CurrentDramType == TypeDdr3L) {
       MrcData->DynamicVars[Channel][TRPAB] = GetTimingParam(BxtTimingtRP);
  }

  //  DTR8A /*****************/
  MrcData->DynamicVars[Channel][TCKLPDLY] = CEILING ((UINT32)(30 * 1000) , TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]);
  MrcData->DynamicVars[Channel][PWDDLY]   = CEILING ((UINT32)(80 * 1000),TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]);

  MrcData->DynamicVars[Channel][TCKESR] = GetTimingParam(BxtTimingtCKESR);

  if (MrcData->CpuStepping == STEPPING_E0) {
    switch (CurrentDramType) {
    case TypeLpDdr4:
          MrcData->DynamicVars[Channel][TLPCKDLY] = 0x2C;
          break;
    case TypeLpDdr3:
         MrcData->DynamicVars[Channel][TLPCKDLY] = 0x1A;
      break;

    default:
      break;
    }
  } else {
    switch (CurrentDramType) {
    case TypeLpDdr4:
          MrcData->DynamicVars[Channel][TLPCKDLY] = 0x20;
          MrcData->DynamicVars[Channel][LPMDRES]  = 0xc;
#if BXT_PO
          MrcData->DynamicVars[Channel][TLPCKDLY] = 0x24;
#endif
          break;
    case TypeLpDdr3:
         MrcData->DynamicVars[Channel][TLPCKDLY] = 0x14;
         MrcData->DynamicVars[Channel][LPMDRES]  = 0x6;
#if BXT_PO
          MrcData->DynamicVars[Channel][TLPCKDLY] = 0x16;
#endif
         break;

    case TypeDdr3L:
         MrcData->DynamicVars[Channel][TLPCKDLY] = 0x4;
         MrcData->DynamicVars[Channel][LPMDRES]  = 0x2;
#if BXT_PO
          MrcData->DynamicVars[Channel][TLPCKDLY] = 0x6;
#endif

         break;

    default:
      break;
    }
  }

  /*****************/


  switch (CurrentDramType) {
  case TypeLpDdr4:
    if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Package == CO_PoP) {    //Copop
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 1;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 2;

#if BXT_PO
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 0;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
#endif
    } else {   //BGA
    MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 1;
    MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 1;
    if (SPIDCLK_LP4[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] < MrcData->SABy2Clk) {
      //SPID < SA/2
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 0;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
    } else if ((SPIDCLK_LP4[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] < MrcData->SABy2Clk*2) && (SPIDCLK_LP4[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] > MrcData->SABy2Clk)){
      //SA > SPID > SA/2
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 2;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 2;
    }

#if BXT_PO
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 0;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
#endif
    }
    break;
  case TypeLpDdr3:
    MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 0;
    MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 1;
    if (SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] < MrcData->SABy2Clk) {
      //SPID < SA/2
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
    } else if ((SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] < MrcData->SABy2Clk*2) && (SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] > MrcData->SABy2Clk)){
      //SA > SPID > SA/2
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 1;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 2;
    }

#if BXT_PO
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 0;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
#endif
      break;
  case TypeDdr3L:
    //default 1
    MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 1;
    MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 1;
    if (SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] < MrcData->SABy2Clk) {
      //SPID < SA/2
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 1;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
    } else if (SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] == MrcData->SABy2Clk) {
      //SPID == SA/2
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 1;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 1;
    } else if ((SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] < MrcData->SABy2Clk*2) && (SPIDCLK_DDR3_LP3[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] > MrcData->SABy2Clk)){
      //SA > SPID > SA/2
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 2;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 2;
    }
#if BXT_PO
      MrcData->DynamicVars[Channel][SPID_EARLY_RDDATA_VALID] = 0;
      MrcData->DynamicVars[Channel][BGF_EARLY_RDDATA_VALID] = 0;
#endif

    if(((SMIP_ODT_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].OdtConfig))->Bits.TxOdt == MRC_SMIP_DDR3L_TX_ODT_RTT_120_OHMS
    && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].OdtHigh
    && !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ASR_Supported) {
      MrcData->DynamicVars[Channel][SRTEN] = 0;
      MmrcDebugPrint ((MMRC_DBG_MIN, "Static SR: "));
      if(((SMIP_DDR3LASR_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.DDR3LASR))->Bits.MaxOperatingTemp == MRC_SMIP_DDR3L_MAX_85_DEGREES_C) {
        MrcData->DynamicVars[Channel][DDR3L_REFRESH_RATE_VALUE] = RefreshRate1x;
        MmrcDebugPrint ((MMRC_DBG_MIN, "1x\n"));
      } else { // MaxOperatingTemp == MRC_SMIP_DDR3L_MAX_95_DEGREES_C
        MrcData->DynamicVars[Channel][DDR3L_REFRESH_RATE_VALUE] = RefreshRate2x;
        MmrcDebugPrint ((MMRC_DBG_MIN, "2x\n"));
      }
    } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ASR_Supported) {
      MrcData->DynamicVars[Channel][SRTEN] = 0;
    } else {
      MrcData->DynamicVars[Channel][SRTEN] = 1;
    }
    break;

    default:
      break;
  }

  SetupCustomTsvVariables(MrcData, Channel);

  switch (CurrentDramType) {
    case TypeLpDdr4:
      MrcData->DynamicVars[Channel][CMDTRIST] = 1;
      break;
    case TypeLpDdr3:
      if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency <= MMRC_1600) {
        MrcData->DynamicVars[Channel][CMDTRIST] = 1;
      } else {
        MrcData->DynamicVars[Channel][CMDTRIST] = 2;
      }
      break;
    case TypeDdr3L:
      MrcData->DynamicVars[Channel][CMDTRIST] = 2;
      break;
    default:
      MrcData->DynamicVars[Channel][CMDTRIST] = 1;
  }

#if BXT_PO
  MrcData->DynamicVars[Channel][CMDTRIST] = 0;
#endif

  MrcData->DynamicVars[Channel][DQS_OSC_RT] = CEILING (((UINT32)((150+40) * 1000) / TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]) , 16);

  MrcData->DynamicVars[Channel][DQS_RETRAIN_INT] = CEILING ( (UINT32)(1 * 1000000) , ((UINT32)(TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] * (UINT32) MrcData->DynamicVars[Channel][NREFI]) /1000));

  if (CurrentDramType == TypeDdr3L) {
    MrcData->DynamicVars[Channel][TQPOLLPER] = 0;
  } else {
    MrcData->DynamicVars[Channel][TQPOLLPER] = CEILING (16000000 , (UINT32)( 4 *  ((UINT32)(TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] * (UINT32) MrcData->DynamicVars[Channel][NREFI])/1000)) );
  }

    MrcData->DynamicVars[Channel][SSKPD0] = 1;
    MrcData->DynamicVars[Channel][SSKPD1] = 0;
    MrcData->DynamicVars[Channel][BONUS0] = 0;
    MrcData->DynamicVars[Channel][BONUS1] = 0;

    //*** DOCRA *** //

    if (CurrentDramType == TypeLpDdr3) {
      //RU(tODTon(max)/tCK) + RU((tDQSSmax+tWPST)/tCK) + BL/2 - RD(tODToffmin/tCK)
      MrcData->DynamicVars[Channel][WRODTSTOP] = CEILING (3500,TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]) + 2 + BLMode/2 - (1750/TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency]);
    } else if (CurrentDramType == TypeDdr3L) {
      MrcData->DynamicVars[Channel][WRODTSTOP] = 6;
      MrcData->DynamicVars[Channel][RDODTSTART] = MrcData->DynamicVars[Channel][RL] - MrcData->DynamicVars[Channel][WL];
    } else {
      MrcData->DynamicVars[Channel][WRODTSTOP] = 0;
    }

    if (CurrentDramType != TypeLpDdr4) {
      if(CurrentDramType == TypeLpDdr3) {
        if (ConfigToSpeed[MrcData->MrcMemConfig] == MMRC_1866) {
          MrcData->DynamicVars[Channel][R0WRODTCTL] = 0;
          MrcData->DynamicVars[Channel][R1WRODTCTL] = 0;
        } else {
          MrcData->DynamicVars[Channel][R0WRODTCTL] = 1;
          MrcData->DynamicVars[Channel][R1WRODTCTL] = (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[0] == 2);
        }
      } else { // CurrentDramType == TypeDdr3L
        MrcData->DynamicVars[Channel][R0WRODTCTL] = 1;
        MrcData->DynamicVars[Channel][R1WRODTCTL] = (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[0] == 2) ? 2 : 0;
      }
      MrcData->DynamicVars[Channel][R0RDODTCTL] = 0;
      MrcData->DynamicVars[Channel][R1RDODTCTL] = 0;
    }

    //***************//


    //640ns / 16 x clock period
    MrcData->DynamicVars[Channel][PCLSTO]  = CEILING ((UINT32)(150 * 1000) / TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] , 16);

  MrcData->DynamicVars[Channel][MR4_BYTE_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[0]/8;
  MrcData->DynamicVars[Channel][MR4_BYTE_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[16]/8;

  if (MrcData->DynamicVars[Channel][MR4_BYTE_SEL] == 0) {
      MrcData->DynamicVars[Channel][MR4_BIT0_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[0];
      MrcData->DynamicVars[Channel][MR4_BIT1_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[1];
      MrcData->DynamicVars[Channel][MR4_BIT2_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[2];
  } else {
      MrcData->DynamicVars[Channel][MR4_BIT0_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[0] % (UINT8)(8*MrcData->DynamicVars[Channel][MR4_BYTE_SEL]);
      MrcData->DynamicVars[Channel][MR4_BIT1_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[1] % (UINT8)(8*MrcData->DynamicVars[Channel][MR4_BYTE_SEL]);
      MrcData->DynamicVars[Channel][MR4_BIT2_SEL] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[2] % (UINT8)(8*MrcData->DynamicVars[Channel][MR4_BYTE_SEL]);

  }

  if (MrcData->DynamicVars[Channel][MR4_BYTE_SEL2] == 0) {
      MrcData->DynamicVars[Channel][MR4_BIT0_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[16];
      MrcData->DynamicVars[Channel][MR4_BIT1_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[17];
      MrcData->DynamicVars[Channel][MR4_BIT2_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[18];
  } else {

    MrcData->DynamicVars[Channel][MR4_BIT0_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[16] % (UINT8)(8*MrcData->DynamicVars[Channel][MR4_BYTE_SEL2]);
    MrcData->DynamicVars[Channel][MR4_BIT1_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[17] % (UINT8)(8*MrcData->DynamicVars[Channel][MR4_BYTE_SEL2]);
    MrcData->DynamicVars[Channel][MR4_BIT2_SEL2] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[18] % (UINT8)(8*MrcData->DynamicVars[Channel][MR4_BYTE_SEL2]);
  }

  MrcData->TwoXps = Timing2x[MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency];

  if (MrcData->DynamicSrDisable == 1){
    MmrcExecuteTask(MrcData, DisableDynamicSr, NO_PRINT, 0);
  }

  if(MrcData->CpuStepping == STEPPING_P0) {
    MrcData->DynamicVars[Channel][INVPH0TO180_DDR3_VAL] = 1;
  } else {
    MrcData->DynamicVars[Channel][INVPH0TO180_DDR3_VAL] = 0;
  }

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    // If using an ECC DIMM, set ECCEN
    // Note: this is updated later in "MmrcCheckEccEnable" based on fused values
    MrcData->DynamicVars[Channel][ECCEN] = 1;
  }

  // SwitchTo2N / SwitchTo1N parameters
  // The CMD Group should not be modified on the restore path. Instead, is should
  // be resotored using the CmdGrp0/CmdGrp1 GetSet
  if (InResorePath(MrcData)) {
    MrcData->DynamicVars[Channel][CMD_RDPTROFFSET_ADJUST] = FALSE;
  } else {
    MrcData->DynamicVars[Channel][CMD_RDPTROFFSET_ADJUST] = TRUE;
  }
  MrcData->KeepAlive=0;
  if (CurrentDramType == TypeDdr3L) {
    // if DDR3L and Dual Rank:
    // Set CH1_CLK_ENABLE = 0 if Ch1ClkDisable == 1, otherwise set to 1
    // Set CH1_CLK_DISABLE = 1 if Ch1ClkDisable == 1, otherwise set to 0
    // else disable CH1_CLK when operating in single rank mode
    MrcData->DynamicVars[Channel][CH1_CLK_ENABLE] = (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ChOption.Bits.Ch1ClkDisable == FALSE && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1] == 1);
    MrcData->DynamicVars[Channel][CH1_CLK_DISABLE] = !MrcData->DynamicVars[Channel][CH1_CLK_ENABLE];
  } else {
    // Keep CH1_CLK enabled for all other cases
    MrcData->DynamicVars[Channel][CH1_CLK_ENABLE] = 1;
    MrcData->DynamicVars[Channel][CH1_CLK_DISABLE] = 0;
  }

  if (MrcData->PeriodicRetrainingDisable) {
    MrcData->DynamicVars[Channel][PRTEN] = FALSE;
  } else {
    MrcData->DynamicVars[Channel][PRTEN] = TRUE;
  }

  if (((SMIP_ODT_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].OdtConfig))->Bits.CaOdt == SMIP_CA_ODT_AB_HIGH_LOW) {
    MrcData->Lpddr4CaOdt = LPDDR4_JEDEC_MR11_ODT_RZQ3_OHM_80;
  } else {
    MrcData->Lpddr4CaOdt = LPDDR4_JEDEC_MR11_ODT_RZQ2_OHM_120;
  }
  return MMRC_SUCCESS;
}

/**
  Override Timing parameter based on user input/Setup option

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
OverrideTimingParams (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{

  return MMRC_SUCCESS;
}

/**
  Channel0Only

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
Channel0Only (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
)
{
  if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_1X32_LPDDR4) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled = TRUE;
    MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled = FALSE;
    MrcData->NvData.MrcParamsSaveRestore.Channel[2].Enabled = FALSE;
    MrcData->NvData.MrcParamsSaveRestore.Channel[3].Enabled = FALSE;
  }

  return MMRC_SUCCESS;
}
MMRC_STATUS
ClkGatePhy (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
)
{

  REGISTER_ACCESS Register;
  UINTX Value;
  Register.ShiftBit=0;
  Register.Mask=0xFFFFFFFF;
  Value = 0x20000027;     //pm_msg_req[5]=1, pm_msg[4:0] = 7, hvm_pmmsg[27:31]=4
  MmrcDebugPrint ((MMRC_DBG_MIN, "ClkGatePhy\n"));

  switch (ConfigToDdrType[MrcData->MrcMemConfig]) {
  case TypeLpDdr3:
  case TypeLpDdr4:

    //Disabling PHY IPs should be done at CH granularity because for 1x32 config CH0 is enabled
    //then DDRIO1 is disabled and DDRIO0, ch1 is disabled but the way it was implemented it wont
    //disable CH1 of DDRIO0. Its either disable both CHs per DDRIO or enable both of them which
    //violates POR.

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled == 0) {
      MmrcDebugPrint((MMRC_DBG_MIN, "Ch0 disable \n"));
      Register.Offset = DPIC_PMSWMSG_CH0_DPIC_BOXTYPE_REG;
      MemRegWrite (DPIC_BOXTYPE, 0, 0, Register, Value, 0xF);
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled == 0) {
      MmrcDebugPrint((MMRC_DBG_MIN, "Ch1 disable \n"));
      Register.Offset = DPIC_PMSWMSG_CH1_DPIC_BOXTYPE_REG;
      //Channel is passed as 0 because in excel sheet DPIC has two instances per ch0 and ch2
      MemRegWrite(DPIC_BOXTYPE, 0, 0, Register, Value, 0xF);
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[2].Enabled == 0) {
      MmrcDebugPrint((MMRC_DBG_MIN, "Ch2 disable \n"));
      Register.Offset = DPIC_PMSWMSG_CH0_DPIC_BOXTYPE_REG;
      MemRegWrite(DPIC_BOXTYPE, 2, 0, Register, Value, 0xF);
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[3].Enabled == 0) {
      MmrcDebugPrint((MMRC_DBG_MIN, "Ch3 disable \n"));
      Register.Offset = DPIC_PMSWMSG_CH1_DPIC_BOXTYPE_REG;
      MemRegWrite(DPIC_BOXTYPE, 2, 0, Register, Value, 0xF);
    }
    break;
  case TypeDdr3L:
    MmrcDebugPrint ((MMRC_DBG_MIN, "TypeDdr3\n"));
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled == 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Disabling Ch 0..."));
      MmrcExecuteTaskImplementation (MrcData, HALF_CFG_PWR_GATE, NO_PRINT, 0, TRUE, 1);
      MmrcDebugPrint ((MMRC_DBG_MIN, "done\n"));
    }
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled == 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Disabling Ch1..."));
      MmrcExecuteTaskImplementation (MrcData, HALF_CFG_PWR_GATE, NO_PRINT, 1, TRUE, 2);
      MmrcDebugPrint ((MMRC_DBG_MIN, "done\n"));
    }
    break;
  default:
     MmrcExecuteTask (MrcData, modmem_init_lpddrgrp10_seq, NO_PRINT, 2);
  }
  return MMRC_SUCCESS;
}

VOID
AsmSetSliceChannelHash (
  UINT32 HashChannelValue,
  UINT32 HashChannelValueHigh32)
{
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
  "\n\t pushl  %%eax"
  "\n\t pushl  %%edx"
  "\n\t pushl  %%ecx"
  "\n\t movl   $0x00, %%eax"
  "\n\t movl   $0x00, %%edx"
  "\n\t movl   $0x122, %%ecx"
  "\n\t rdmsr"
  "\n\t movl   %0, %%eax"
  "\n\t movl   %1, %%edx"
  "\n\t wrmsr"
  "\n\t popl   %%ecx"
  "\n\t popl   %%edx"
  "\n\t popl   %%eax"
  : "=m"(HashChannelValue),"=m"(HashChannelValueHigh32)
  :
  :
  );
#else //MSFT compiler
  _asm {
    push eax
    push edx
    push ecx
    mov eax, 00h
    mov edx, 00h
    mov ecx, 0122h
    rdmsr
    mov  eax, HashChannelValue
    mov  edx, HashChannelValueHigh32
    wrmsr
    pop ecx
    pop edx
    pop eax
  }
#endif
}

VOID
SetSliceChannelHash(
  IN OUT MMRC_DATA *MrcData
  )
{
  UINT8 MaxCh;
  UINT32 RemainderSize;   // in MB
  UINT32 SymmetricMemSize;
  UINT32 AsymBaseAddress;
  UINT32 AsymHighAddress;
  UINT32 Tolud;
  UINT32 HashChannelValue;
  UINT32 HashChannelValueHigh32;
  REGISTER_ACCESS Register;
  BOOLEAN AsymMemory;
  UINT32 FirstHalfMemorySize;
  UINT32 SecondHalfMemorySize;
  UINT32 StolenMemorySize;
  UINT16 SliceHashMask;
  UINT16 ChannelHashMask;
  UINT8  InterleavedMode;

  AsymMemory = FALSE;
  StolenMemorySize = 0;
  FirstHalfMemorySize = 0;
  SecondHalfMemorySize = 0;
  HashChannelValue = 0;
  HashChannelValueHigh32 = 0;
  HashChannelValueHigh32 = (3 << (36 - 32)) | (3 << (34 - 32));

  if (MrcData->BxtSeries == SERIES_BXT_P) {
    MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] = 3;
    MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] = 3;
    MrcData->DynamicVars[0][SLICE_1_DISABLED] = 0;
    HashChannelValueHigh32 = (3 << (36 - 32)) | (3 << (34 - 32));

    if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled && MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled) {
        MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] = 1;
        MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] = 1;
        HashChannelValueHigh32 = (1 << (36 - 32)) | (1 << (34 - 32));
        FirstHalfMemorySize = MrcData->NvData.MrcParamsSaveRestore.Channel[0].TotalMem;
        SecondHalfMemorySize = MrcData->NvData.MrcParamsSaveRestore.Channel[1].TotalMem;
      } else {  // only 1 channel
        HashChannelValueHigh32 = 0;
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled){
          MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] = 0;
          MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] = 1;
          MrcData->DynamicVars[0][SLICE_1_DISABLED] = 1;
          MrcData->DynamicVars[0][SLICE_0_MEM_DISABLED] = 0;
          HashChannelValueHigh32 = (1 << (34 - 32));
          HashChannelValue = BIT0;
        } else {
          MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] = 1;
          MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] = 0;
          MrcData->DynamicVars[0][SLICE_1_DISABLED] = 0;
          MrcData->DynamicVars[0][SLICE_0_MEM_DISABLED] = 1;
          HashChannelValueHigh32 = (1 << (36 - 32));
          HashChannelValue = BIT4;
        }
      }
      MaxCh = 2;
      HashChannelValue |= BIT31;
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled) {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].TotalMem != MrcData->NvData.MrcParamsSaveRestore.Channel[0].TotalMem) {
          AsymMemory = TRUE;  // for ddr3 case
        }
      }
    } else {
      if (MrcData->NumberOfEnabledChannels[ConfigToDdrType[MrcData->MrcMemConfig]] != MAX_CHANNELS) {
        // Clear all of the enable bits and then set them based on the channel config
        HashChannelValueHigh32 = 0;
        MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] = 0;
        MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] = 0;

        if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled) {
          MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] = BIT0;
          HashChannelValueHigh32 = (BIT0 << (34 - 32));
        }
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled) {
          MrcData->DynamicVars[0][SYM_SLICE0_CHANNEL_ENABLED] |= BIT1;
          HashChannelValueHigh32 |= (BIT1 << (34 - 32));
        }
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[2].Enabled) {
          MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] = BIT0;
          HashChannelValueHigh32 = (BIT0 << (36 - 32));
        }
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[3].Enabled) {
          MrcData->DynamicVars[0][SYM_SLICE1_CHANNEL_ENABLED] |= BIT1;
          HashChannelValueHigh32 |= (BIT1 << (36 - 32));
        }

        if ((MrcData->NvData.MrcParamsSaveRestore.Channel[2].Enabled == 0) && (MrcData->NvData.MrcParamsSaveRestore.Channel[3].Enabled == 0)) {
          MrcData->DynamicVars[0][SLICE_1_DISABLED] = 1;
          HashChannelValue = BIT0;
        }
      } else {  //full channels
        FirstHalfMemorySize = MrcData->NvData.MrcParamsSaveRestore.Channel[0].TotalMem + MrcData->NvData.MrcParamsSaveRestore.Channel[1].TotalMem;
        SecondHalfMemorySize = MrcData->NvData.MrcParamsSaveRestore.Channel[2].TotalMem + MrcData->NvData.MrcParamsSaveRestore.Channel[3].TotalMem;
        if (FirstHalfMemorySize != SecondHalfMemorySize) {
          AsymMemory = TRUE;  // assume ch0=ch1 and ch2=ch3
        }
      }
      MaxCh = 4;
    }
    if (AsymMemory) {
      if (FirstHalfMemorySize <= SecondHalfMemorySize) {
        SymmetricMemSize = 2 * FirstHalfMemorySize;
        RemainderSize = MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize - SymmetricMemSize;
      } else {
        SymmetricMemSize = 2 * SecondHalfMemorySize;
        RemainderSize = MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize - SymmetricMemSize;
      }

      Register.Mask = 0xFFFFFFFF;
      Register.ShiftBit = 0;
      Register.Offset = TOLUD_0_0_0_PCI_CUNIT_BOXTYPE_REG;
      Tolud = MemRegRead(CUNIT_BOXTYPE, 0, 0, Register) & ~BIT0;
      StolenMemorySize = (UINT32)(0x1000 - (Tolud >> 20));
      if (MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize >= (Tolud >> 20)) {
        if ((SymmetricMemSize + StolenMemorySize) >= 0x1000) {
          AsymBaseAddress = (StolenMemorySize + SymmetricMemSize) >> 8; //(SymmetricMemSize + MrcData->OemMrcData.MmioSize)  >> 8;
          AsymHighAddress = ((StolenMemorySize + SymmetricMemSize + RemainderSize) >> 8) - 1;
        } else {
          AsymBaseAddress = SymmetricMemSize >> 8;
          AsymHighAddress = ((SymmetricMemSize + StolenMemorySize + RemainderSize) >> 8) - 1;
        }
      } else {
        AsymBaseAddress = ((UINT32)(Tolud >> 20) - SymmetricMemSize) >> 8;
        AsymHighAddress = ((((UINT32)(Tolud >> 20) - SymmetricMemSize) + RemainderSize) >> 8) - 1;
      }
      if (MrcData->NumberOfEnabledChannels[ConfigToDdrType[MrcData->MrcMemConfig]] == MAX_CHANNELS) {
        MrcData->DynamicVars[0][ASYM_2WAY_INTERLEAVE_ENABLE] = 1;
        MrcData->DynamicVars[0][ASYM_2WAY_LIMIT] = AsymHighAddress;
        MrcData->DynamicVars[0][ASYM_2WAY_BASE] = AsymBaseAddress;
        MrcData->DynamicVars[0][ASYM_2WAY_INTLV_MODE] = 3;

      } else {
        //ddr3l
        MrcData->DynamicVars[0][SLICE0_ASYM_CHANNEL_SELECT] = 0;
        if (FirstHalfMemorySize > SecondHalfMemorySize) {
          MrcData->DynamicVars[0][SLICE0_ASYM_ENABLE] = 1;
          MrcData->DynamicVars[0][SLICE0_ASYM_LIMIT] = AsymHighAddress;
          MrcData->DynamicVars[0][SLICE0_ASYM_BASE] = AsymBaseAddress;
        } else {
          MrcData->DynamicVars[0][SLICE1_ASYM_ENABLE] = 1;
          MrcData->DynamicVars[0][SLICE1_ASYM_LIMIT] = AsymHighAddress;
          MrcData->DynamicVars[0][SLICE1_ASYM_BASE] = AsymBaseAddress;
        }
      }
    }
  }
#ifndef SIM
  if ((ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4
    || ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr3)
    && GetNumberOfEnabledChannels() == 4) {
    SliceHashMask = 0x9;
    ChannelHashMask = 0x36;
    InterleavedMode = 1;
  } else if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L
    && GetNumberOfEnabledChannels() == 2) {
    SliceHashMask = 0x49;
    ChannelHashMask = 0x36;
    InterleavedMode = 2;
  } else {
    SliceHashMask = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.SliceHashMask;
    ChannelHashMask = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChannelHashMask;
    InterleavedMode = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.InterleavedMode;
  }
  HashChannelValue |= ((SliceHashMask & 0x1fff) << 6);
  HashChannelValue |= ((InterleavedMode & 0x3) << 2);
  HashChannelValueHigh32 |= ((ChannelHashMask & 0x1fff) << (38 - 32));
  MrcData->DynamicVars[0][SLICE_HASH_MASK] = SliceHashMask;
  MrcData->DynamicVars[0][CHANNEL_HASH_MASK] = ChannelHashMask;
  MrcData->DynamicVars[0][AUNIT_INTERLEAVED_MODE] = InterleavedMode;
  AsmSetSliceChannelHash(HashChannelValue, HashChannelValueHigh32);
  MmrcExecuteTask(MrcData, SetSliceChannel, NO_PRINT, 0);
#endif
}

/**
  Initializes all dynamic variables used in JEDEC init.

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
  InitializeJedec_DDR3(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  UINT8             inRttNom,
  UINT8             inRttWr
)
{
  UINT8   Rank;
  UINT8   SaveDebugMsgLevel;

  DramInitDDR3MRS0 mrs0Command;
  DramInitDDR3EMR1 emrs1Command;
  DramInitDDR3EMR2 emrs2Command;
  DramInitDDR3EMR3 emrs3Command;

  UINT32 rttNom;
  UINT32 tWR;

  SaveDebugMsgLevel = MrcData->MrcDebugMsgLevel;
  MrcData->MrcDebugMsgLevel = MMRC_DBG_VERBOSE;
  MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
  MrcData->DynamicVars[Channel][RDDCMD]      = 0;                // This is a MRW command.
  MmrcDebugPrint((MMRC_DBG_MIN, "RttNom=%d RttWr=%d\n", inRttNom, inRttWr));

  //Does CKE needs to go low?

  // setup for emrs 2
  emrs2Command.Data = 0;
  emrs2Command.Bits.bankAddress = 2;
  emrs2Command.Bits.CWL = GetTimingParam(BxtTimingtCWL) - 5;

  //ODT=1 on MB
  emrs2Command.Bits.rtt_WR = inRttWr;

  emrs2Command.Bits.ASR = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ASR_Supported;

  // setup for emrs 3
  emrs3Command.Data = 0;
  emrs3Command.Bits.bankAddress = 3;

  // setup for emrs 1
  emrs1Command.Data = 0;
  rttNom = 0;
  emrs1Command.Bits.bankAddress = 1;
  emrs1Command.Bits.dllEnabled = 0; // 0 = Enable , 1 = Disable
  emrs1Command.Bits.DIC0 = 1;

  //ODT=1 on MB
  rttNom = inRttNom; //multiplex address

  emrs1Command.Data |= rttNom;

  // setup for mrs 0
  mrs0Command.Data = 0;
  mrs0Command.Bits.bankAddress = 0;
  mrs0Command.Bits.dllReset = 1;
  mrs0Command.Bits.BL = 0;
  mrs0Command.Bits.PPD = 1;

  if (GetTimingParam(BxtTimingtCL) >= 12) {
    mrs0Command.Bits.CL0 = 1;
    mrs0Command.Bits.CL1 = GetTimingParam(BxtTimingtCL) - 12;
  } else {
    mrs0Command.Bits.CL1 = GetTimingParam(BxtTimingtCL) - 4;
  }

  if ( IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) ) {
    tWR = GetTimingParam(BxtTimingtWR);
  } else {
    tWR = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTwr];
  }
  if (tWR < 10) {
    mrs0Command.Bits.writeRecovery = tWR - 4 ;
  } else if (tWR == 16) {
    mrs0Command.Bits.writeRecovery = 0;
  } else {
    mrs0Command.Bits.writeRecovery = (tWR >> 1) & 0x7;
  }


  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    MrcData->DynamicVars[Channel][RANK]        = Rank;            // Must set the rank to the
    MrcData->DynamicVars[Channel][REG_DATA]    = NOP_DDR3L;
    SendDramCmd(MrcData, Channel);

#if SIM
    MySimStall(2000);
#else
    MrcDelay (MrcData, MICRO_DEL, 5);
#endif

    MrcData->DynamicVars[Channel][REG_DATA]    = emrs2Command.Data;
    MrcData->DynamicVars[Channel][MR_VALUE]    = (UINT16)emrs2Command.Data;
    MrcData->NvData.MrcParamsSaveRestore.DDR3L_MRVALUE = (UINT16)emrs2Command.Data;
    SendDramCmd(MrcData, Channel);
#if SIM
    MySimStall(2000);
#else
    MrcDelay (MrcData, MICRO_DEL, 5);
#endif

    MrcData->DynamicVars[Channel][REG_DATA]    = emrs3Command.Data;
    SendDramCmd(MrcData, Channel);
#if SIM
    MySimStall(2000);
#else
    MrcDelay (MrcData, MICRO_DEL, 50);
#endif

    MrcData->DynamicVars[Channel][REG_DATA]    = emrs1Command.Data;
    SendDramCmd(MrcData, Channel);
#if SIM
    MySimStall(2000);
#else
    MrcDelay (MrcData, MICRO_DEL, 5);
#endif

    MrcData->DynamicVars[Channel][REG_DATA]    = mrs0Command.Data;
    SendDramCmd(MrcData, Channel);
#if SIM
    MySimStall(50);
    MySimStall(2000);
#else
    MrcDelay (MrcData, MICRO_DEL, 5);
#endif

    MrcData->DynamicVars[Channel][REG_DATA]    = ZQCL_DDR3L;
    SendDramCmd(MrcData, Channel);

#if SIM
    MySimStall(50);
    MySimStall(2000);
#else
    MrcDelay (MrcData, MICRO_DEL, 5);
#endif

    MrcData->MR1_Value = emrs1Command.Data;
    MrcData->MR2_Value = emrs2Command.Data;
    MrcData->MR3_Value = emrs3Command.Data;

    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR3 MR1: %x\n", MrcData->MR1_Value));
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR3 MR2: %x\n", MrcData->MR2_Value));
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR3 MR3: %x\n", MrcData->MR3_Value));
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
  InitializeJedec_DDR3_Nom60_Wr0(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  InitializeJedec_DDR3(MrcData, CapsuleStartIndex, StringIndex, Channel, DDR3_MRS1_RTTNOM_60, DDR3_MRS2_RTTWR_0);
  return MMRC_SUCCESS;
}

MMRC_STATUS
InitializeJedec_DDR3_UseRttNom(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  if (((SMIP_ODT_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].OdtConfig))->Bits.TxOdt == MRC_SMIP_DDR3L_TX_ODT_RTT_120_OHMS) {
    InitializeJedec_DDR3(MrcData, CapsuleStartIndex, StringIndex, Channel, DDR3_MRS1_RTTNOM_120, DDR3_MRS2_RTTWR_0);
  } else {
    InitializeJedec_DDR3(MrcData, CapsuleStartIndex, StringIndex, Channel, DDR3_MRS1_RTTNOM_60, DDR3_MRS2_RTTWR_0);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
InitializeJedec_DDR3_UseRttWr(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  if (((SMIP_ODT_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].OdtConfig))->Bits.TxOdt == MRC_SMIP_DDR3L_TX_ODT_RTT_120_OHMS) {
    InitializeJedec_DDR3(MrcData, CapsuleStartIndex, StringIndex, Channel, DDR3_MRS1_RTTNOM_DIS, DDR3_MRS2_RTTWR_120);
  } else {
    InitializeJedec_DDR3(MrcData, CapsuleStartIndex, StringIndex, Channel, DDR3_MRS1_RTTNOM_DIS, DDR3_MRS2_RTTWR_60);
  }
  return MMRC_SUCCESS;
}


MMRC_STATUS
InitializeJedec (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT8   Rank;
  UINT32  TempValue;
  UINT32  MR1;
  UINT32  MR2;
  UINT32  MR3;
  UINT8   SaveDebugMsgLevel;

  SaveDebugMsgLevel = MrcData->MrcDebugMsgLevel;
  MrcData->MrcDebugMsgLevel = MMRC_DBG_VERBOSE;

  MR1 = 0;
  MR2 = 0;
  MR3 = 0;

#if ODT_TABLE
  //
  // Set ODT Matrix
  //
  SetOdtMatrix (MrcData, 0, Channel, MrsValue);
#endif

  //
  // Fill in specifics based on the memory technology behind this DUNIT.
  //
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {

    //
    // Initialize RANK0/RANK1 MR1/2/3 registers.
    //
    MR1 = LPDDR3_JEDEC_MR01;
    MR2 = LPDDR3_JEDEC_MR02;
    MR3 = LPDDR3_JEDEC_MR03;

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
        continue;
      }
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
      MrcData->DynamicVars[Channel][RDDCMD]      = 0;              // This is a MRW command.
      MrcData->DynamicVars[Channel][RANK]        = Rank;           // Must set the rank to the
      MrcData->DynamicVars[Channel][REG_DATA]    = LPDDR3_JEDEC_CMD_NOP;
      SendDramCmd(MrcData, Channel);

#if SIM
      MySimStall(200);
#else
      MrcDelay (MrcData, MICRO_DEL, 200);
#endif

      MrcData->DynamicVars[Channel][REG_DATA]    = LPDDR3_JEDEC_CMD_MRW_RESET;
      SendDramCmd(MrcData, Channel);

#if SIM
      MySimStall(50);
#else
      MrcDelay (MrcData, MICRO_DEL, 50);
#endif
      MrcData->DynamicVars[Channel][REG_DATA]    = LPDDR3_JEDEC_CMD_MRW_ZQCAL; //00FF0A0
      SendDramCmd(MrcData, Channel);

#if SIM
      MySimStall(150);
      MySimStall(2000);
#else
      MrcDelay (MrcData, MICRO_DEL, 150);
#endif
      //
      // LP3 is always BL8.
      //
      MR1 |= (LPDDR3_JEDEC_MR1_BL8 << 12);
      if (GetTimingParam(BxtTimingtWR)> 9) {
        MR1 |= ((GetTimingParam(BxtTimingtWR) - 10) << 17);

        //
        //Enable nWR Programming > 9
        //
        MR2 |= (LPDDR3_JEDEC_MR2_WRE_GT9 << 12);
      } else {
        MR1 |= ((GetTimingParam(BxtTimingtWR) - 2) << 17);
      }

      //
      // Subtract 2 from TCL to match LP spec.
      //
      MR2 |= ((GetTimingParam(BxtTimingtCL) - 2) << 12);
      //
      // Set the WL Select based on timing params. This matches the RTL sim test bench's implementation.
      //
      if (GetTimingParam(BxtTimingtCWL) > 8 || (GetTimingParam(BxtTimingtCL) == 10 && GetTimingParam(BxtTimingtCWL) == 8)) {

        //
        // Select WL Set B
        //
        MR2 |= (LPDDR3_JEDEC_MR2_WL_SETB << 12);
      }
      //
      // 40 ohm.
      //
      MR3 |= (LPDDR3_JEDEC_MR3_OHM_343 << 12);

      MrcData->DynamicVars[Channel][REG_DATA]    = MR2;
      SendDramCmd(MrcData, Channel);
#if SIM
      MySimStall(50);
#else
      MrcDelay (MrcData, MICRO_DEL, 50);
#endif

      MrcData->DynamicVars[Channel][REG_DATA]    = MR1;
      SendDramCmd(MrcData, Channel);

#if SIM || JTAG
      MySimStall(50);
#else
      MrcDelay (MrcData, MICRO_DEL, 50);
#endif
      MrcData->DynamicVars[Channel][REG_DATA]    = MR3;
      SendDramCmd(MrcData, Channel);
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3 && (1<<MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency) == F_1866 ) {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1]) {
          MrcData->DynamicVars[Channel][REG_DATA]    = LPDDR3_JEDEC_MR11 | (2 << 12) | (4 << 12);  //120ohm
        } else {
          MrcData->DynamicVars[Channel][REG_DATA]    = LPDDR3_JEDEC_MR11 | (2 << 12);  //120ohm
        }
        SendDramCmd(MrcData, Channel);
      }
    }
    MrcData->MR1_Value = MR1;
    MrcData->MR2_Value = MR2;
    MrcData->MR3_Value = MR3;

  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {

    MySimStall(50);
    MySimStall(2000);

    //
    // Initialize RANK0/RANK1 MR1/2/3 registers.
    //
    MR1 = 0x16046;
    TempValue = (UINT32)(GetTimingParam(BxtTimingtWR) - 6) / 4;
    if (TempValue == 4) { //nWR = 24
      MR1 |= (1 << 17);
      MmrcDebugPrint ((MMRC_DBG_MIN, "TimingParam nWR = 24, MR1 = %x\n", MR1));
    } else if (TempValue > 4) { //nWR > 24
      MR1 |= (1 << 17) | ((TempValue-5) << 22);
      MmrcDebugPrint ((MMRC_DBG_MIN, "TimingParam nWR > 24, MR1 = %x\n", MR1));
    } else { //nWR < 24
      MR1 |= (TempValue) << 22;
      MmrcDebugPrint ((MMRC_DBG_MIN, "TimingParam nWR < 24, MR1 = %x\n", MR1));
    }
    MR2 = 0x16086;
    MR3 = 0x160C6;
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
        continue;
      }

      //BL
      MR1 |= (1 << 20);  //set to BL32[op0:1], WPRE=2*tck[op2]
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
      MrcData->DynamicVars[Channel][RDDCMD]      = 0;        // This is a MRW command.
      MrcData->DynamicVars[Channel][RANK]        = Rank;            // Must set the rank to the

      MR2 |= ((GetTimingParam(BxtTimingtCL) - 6) / 4) << 18;  //DBI-RD disable

      TempValue = 1;             // WLS always set B for BXT
      MR2 |= TempValue << 17;    //WLS

      MR2 |= TempValue ? (((GetTimingParam(BxtTimingtCWL) - 3) / 4) ) << 21:  // WL Set "B" formula
        ((GetTimingParam(BxtTimingtCWL) - 4) / 2) << 21 ;  //  WL Set "A" formula

      MR3 |= 0x31 << 18;    //PUCAL op[0], PDDS op[5:3] default value

      MrcData->DynamicVars[Channel][REG_DATA]    = MR1;
      SendDramCmd(MrcData, Channel);
      MrcData->DynamicVars[Channel][MR1_0] =  MrcData->DynamicVars[Channel][REG_DATA];
#if SIM
      MySimStall(50);
      MySimStall(2000);
#else
      MrcDelay (MrcData, MICRO_DEL, 50);
#endif

      MrcData->DynamicVars[Channel][REG_DATA]    = MR3;
      SendDramCmd(MrcData, Channel);

#if SIM
      MySimStall(50);
      MySimStall(2000);
#else
      MrcDelay (MrcData, MICRO_DEL, 50);
#endif
      MrcData->DynamicVars[Channel][REG_DATA]    = MR2;
      SendDramCmd(MrcData, Channel);
      MrcData->DynamicVars[Channel][MR2_0] =  MrcData->DynamicVars[Channel][REG_DATA];

#if SIM
      MySimStall(50);
      MySimStall(2000);
#else
      MrcDelay (MrcData, MICRO_DEL, 50);
#endif
      /*
      // send mr23 ~500ns in 16 dram clk
      //
      TempValue = (500 * 1000 ) / TCK[MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency] / 16;
      MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(23, TempValue);
      SendDramCmd(MrcData, Channel);
      MySimStall(2000); */
    }
    MrcData->MR1_Value = MR1;
    MrcData->MR2_Value = MR2;
    MrcData->MR3_Value = MR3;

  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].OdtHigh)
    {
      MmrcDebugPrint((MMRC_DBG_MIN, "ODT_HIGH Mode \n"));
      InitializeJedec_DDR3_UseRttWr(MrcData, CapsuleStartIndex, StringIndex, Channel);
    }
    else
    {
      InitializeJedec_DDR3_UseRttNom(MrcData, CapsuleStartIndex, StringIndex, Channel);
    }
  } else {
  }

  MrcData->MrcDebugMsgLevel = SaveDebugMsgLevel;


  return MMRC_SUCCESS;
}

/**
  LP4MRProgramming_MPCZQ

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
  LP4MRProgramming_MPCZQ (
  IN  OUT MMRC_DATA   *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel)
{

  UINT32 Rank;
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    //
    // Issue the MPC-1 command ZQCal start and Latch
    //

    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    MrcData->DynamicVars[Channel][RDDCMD]      = 0;        // This is a MRW command.
    MrcData->DynamicVars[Channel][RANK]        = Rank;            // Must set the rank to the

    MrcData->DynamicVars[Channel][REG_DATA]    = MPC2(0, 0x4F);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, MICRO_DEL, 5);

    MySimStall(100);

    MrcData->DynamicVars[Channel][REG_DATA]    = MPC2(0, 0x51);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 50);

    MySimStall(100);
  }

  return MMRC_SUCCESS;
}

/**
  LP4AllMRProgramming

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
  LP4AllMRProgramming (
  IN  OUT MMRC_DATA   *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel)
{

  UINT32 MRValue;
  UINT8  Rank;

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }

    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    MrcData->DynamicVars[Channel][RDDCMD]      = 0;        // This is a MRW command.
    MrcData->DynamicVars[Channel][RANK]        = Rank;            // Must set the rank to the

    MRValue = 0;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(4, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    MRValue = 0;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(10, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    MRValue = 0;
    ((LPDDR4_JEDEC_MR11_STRUCT *)(&MRValue))->Bits.DqOdt = LPDDR4_JEDEC_MR11_ODT_RZQ4_OHM_60;
    ((LPDDR4_JEDEC_MR11_STRUCT *)(&MRValue))->Bits.CaOdt = MrcData->Lpddr4CaOdt;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(11, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    if ( (MrcData->BootMode & (S3|S0C|S0W|FB)) != 0 )
    {
      GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxCaVref, CMD_GET_CACHE, &MRValue);
      GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxCaVref, CMD_SET_VAL_FC, &MRValue);
    } else {
      //MRValue = 0x5e maps to range 1 - 34%%
      MRValue = 340;
      GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxCaVref, CMD_SET_VAL_FC, &MRValue);
    }
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 0x30;
    } else {
      MRValue = 0x10;
    }

    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(13, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    if ( (MrcData->BootMode & (S3|S0C|S0W|FB)) != 0 )
    {
      GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxVref, CMD_GET_CACHE, &MRValue);
      GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxVref, CMD_SET_VAL_FC, &MRValue);
    } else {
      MRValue = 0x54;//0x47; //0x60;
      MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(14, MRValue);
      SendDramCmd(MrcData, Channel);
    }
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    MRValue = 0x00;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(16, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    MRValue = 0x00;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(17, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    MRValue = 0x00;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(24, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
  }

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 0x05;
    } else {
      MRValue = 0x04;
    }
    if (Rank == 1) {
      MRValue |= BIT4;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(22, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);
  }
  return MMRC_SUCCESS;
}

/**
  LP4MRProgramming_ODTVREF

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  MMRC_STATUS
**/
MMRC_STATUS
  LP4MRProgramming_ODTVREF (
  IN  OUT MMRC_DATA   *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel)
{
  UINT32 MRValue;
  UINT8  Rank;

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }

    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    MrcData->DynamicVars[Channel][RDDCMD]      = 0;        // This is a MRW command.
    MrcData->DynamicVars[Channel][RANK]        = Rank;            // Must set the rank to the

    //
    // Step 4.0, MR3[0]=1 (set during jedecinit), MR22[2:0]=100b, MR13[6]=1, MR3[0]=1, MR22[2:0] = 100b
    //

    MRValue = 1;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(3, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);

    MySimStall(50);

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 5;       //bit 5: data mask disable
    } else {
      MRValue = 4;
    }
    if (Rank == 1) {
      MRValue |= BIT4;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(22, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 0x70;
    } else {
      MRValue = 0x50;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(13, MRValue);
    SendDramCmd(MrcData, Channel);

    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    MRValue = 1;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(3, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 5;       //bit 5: data mask disable
    } else {
      MRValue = 4;
    }
    if (Rank == 1) {
      MRValue |= BIT4;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(22, MRValue);  //586
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);

    MySimStall(1000);
    MySimStall(1000);
    //
    // Issue the MPC-1 command ZQCal start and Latch
    //
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;            //MPC before training is BL16
    MrcData->DynamicVars[Channel][REG_DATA]    = MPC2(0, 0x4F);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, MICRO_DEL, 5);

    MySimStall(1000);
    MySimStall(1000);
    MySimStall(1000);


    MrcData->DynamicVars[Channel][REG_DATA]    = MPC2(0, 0x51);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, MICRO_DEL, 3);

    MySimStall(1000);
    MySimStall(1000);
    //
    //  Step 5.0
    //
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 0x70;       //bit 5: data mask disable
    } else {
      MRValue = 0x50;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(13, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);

    MySimStall(50);
    MRValue = 340;
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxCaVref, CMD_SET_VAL_FC, &MRValue);
    MrcDelay(MrcData, NANO_DEL, 10);

    MRValue = 0;
    ((LPDDR4_JEDEC_MR11_STRUCT *)(&MRValue))->Bits.DqOdt = LPDDR4_JEDEC_MR11_ODT_RZQ4_OHM_60;
    ((LPDDR4_JEDEC_MR11_STRUCT *)(&MRValue))->Bits.CaOdt = MrcData->Lpddr4CaOdt;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(11, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
  }  //end of for rank

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }

    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    MrcData->DynamicVars[Channel][RDDCMD] = 0;        // This is a MRW command.
    MrcData->DynamicVars[Channel][RANK] = Rank;            // Must set the rank to the

    // need to do for all ranks at the same time.
    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 5;       //bit 5: data mask disable
    } else {
      MRValue = 4;
    }
    if (Rank == 1) {
      MRValue |= BIT4;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(22, MRValue);    //need bit 3:5
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
  } // for rank

  //
  //  Step 6.0
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }

    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    MrcData->DynamicVars[Channel][RDDCMD] = 0;        // This is a MRW command.
    MrcData->DynamicVars[Channel][RANK] = Rank;            // Must set the rank to the

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 0x30;       //bit 5: data mask disable
    } else {
      MRValue = 0x10;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(13, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
    MySimStall(50);
    MRValue = 340;
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, TxCaVref, CMD_SET_VAL_FC, &MRValue);
    MrcDelay(MrcData, NANO_DEL, 10);

    MySimStall(50);

    MRValue = 0;
    ((LPDDR4_JEDEC_MR11_STRUCT *)(&MRValue))->Bits.DqOdt = LPDDR4_JEDEC_MR11_ODT_RZQ4_OHM_60;
    ((LPDDR4_JEDEC_MR11_STRUCT *)(&MRValue))->Bits.CaOdt = MrcData->Lpddr4CaOdt;
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(11, MRValue);
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
  }

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    MrcData->DynamicVars[Channel][RDDCMD] = 0;        // This is a MRW command.
    MrcData->DynamicVars[Channel][RANK] = Rank;            // Must set the rank to the

    if (MrcData->BxtSeries == SERIES_BXT_P) {
      MRValue = 5;       //bit 5: data mask disable
    } else {
      MRValue = 4;
    }
    if (Rank == 1) {
      MRValue |= BIT4;
    }
    MrcData->DynamicVars[Channel][REG_DATA] = MRW_LP4(22, MRValue);    //need bit 3:5
    SendDramCmd(MrcData, Channel);
    MrcDelay(MrcData, NANO_DEL, 10);
  } // end of rank

  return MMRC_SUCCESS;
}

/**
  Find the common timing for MD+SODIMM configuration

  @param[in,out]  MrcData       Host structure for all data related to MMRC

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
  DetermineCommonTiming (
  IN  OUT   MMRC_DATA            *MrcData
  )
{
  UINT8  Channel;
  UINT16 tFAW;

  if (MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize == 1) {
    tFAW = GetTimingParam(BxtTimingtFAW1K);
  } else {
    tFAW = GetTimingParam(BxtTimingtFAW2K);
  }

  // if No SODIMM. only MD and set channel 0 timing according to the table
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled == FALSE) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[0].Tcl = (UINT8) GetTimingParam(BxtTimingtCL);
    MrcData->NvData.MrcParamsSaveRestore.Channel[0].TimingData[TimingTrp] = GetTimingParam(BxtTimingtRP);
    MrcData->NvData.MrcParamsSaveRestore.Channel[0].TimingData[TimingTrcd] = GetTimingParam(BxtTimingtRCD);
  } else {
    // Use the min between MD and SODIMM
    for (Channel = 0; Channel < DDR3L_MAXCHANNEL; Channel++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl < GetTimingParam(BxtTimingtCL)) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Tcl = (UINT8) GetTimingParam(BxtTimingtCL);
      }

      if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].TimingData[TimingTrp] < GetTimingParam(BxtTimingtRP)) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTrp] = GetTimingParam(BxtTimingtRP);
       }
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].TimingData[TimingTrcd] < GetTimingParam(BxtTimingtRCD)) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTrcd] = GetTimingParam(BxtTimingtRCD);
      }
    }
  }

  return MMRC_SUCCESS;
}

/**
  Enforces the SoC supported DDR Max frequency. This is done by reading the fuse
  registers. If the requested frequency is higher than the supported frequency,
  the Profile is changed to the next supported profile.

  @param[in,out]  MrcData       Host structure for all data related to MMRC

  @retval  None
**/
MMRC_STATUS
EnforceFusedFrequencies (
  IN OUT MMRC_DATA *MrcData
)
{
  UINT16 FreqHigh;
  UINT8  CurrentProfile;
  UINT8  DdrType;
  UINT8  ProfileOrg;
  UINT8  ProfileNew;
  UINT8  FreqProfile;

  FreqProfile = 0;
  DdrType = ConfigToDdrType[MrcData->MrcMemConfig];

  GetSupportedFrequencies (MrcData, &FreqHigh);

  // If the requested frequency is less than or equal to the SoC supported frequency
  // then use that as the MMRC input frequency
  if (ConfigToSpeed[MrcData->MrcMemConfig] <= FreqHigh) {
    MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
    return MMRC_FAILURE;
  }

  MmrcDebugPrint ((MMRC_DBG_MIN, "SMIP>FUSE Freq\n"));

  // If the requested frequency is not supported, find the next supported frequency profile
  ProfileOrg = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile;
  if (DdrType == TypeDdr3L) {
    ProfileNew = DDR3_1333_7_7_7;
  } else if (DdrType == TypeLpDdr4) {
    ProfileNew = LPDDR4_1600_14_15_15;
  } else { //DdrType == TypeLpDdr3
    //Since this is the lowest value in the MEMORY_TYPE_FREQ_CONFIG_SMIP enum,
    //this is safest default value.
    ProfileNew = LPDDR3_1333_10_12_12;
  }
  // Loop through the profiles for this DRAM technology until one with a supported frequency is found.
  for (CurrentProfile = ProfileOrg; CurrentProfile > ProfileNew; CurrentProfile--){
    if (TimingParamHash (CurrentProfile, &FreqProfile) == MMRC_SUCCESS) {
      if (ConfigToSpeed[FreqProfile] <= FreqHigh) {
        break;
      }
    } else {
      MmrcDebugPrint ((MMRC_DBG_MIN, "ERROR: unsupported frequency.\n"));
      ASSERT (FALSE);
    }
  }

  MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile = CurrentProfile;
  MrcData->TimingParamsConfig = CurrentProfile;

  if (TimingParamHash ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile), &(MrcData->MrcMemConfig)) != MMRC_SUCCESS) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "TimingParams Error: %d\n", MrcData->MrcMemConfig));
    return MMRC_FAILURE;
  } else {
    MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
    TimingParamHash (MrcData->TimingParamsConfig, &(MrcData->MrcTimingProfile));
  }
  return MMRC_SUCCESS;
}

/**
  Find the common freq

  @param[in]  MrcData       Host structure for all data related to MMRC

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
  DetermineCommonFreq (
  IN  OUT   MMRC_DATA            *MrcData
  )
{
  MMRC_STATUS Status;
  UINT8       Channel;
  UINT8       i;
  UINT8       TempProfile;

  Status                    = MMRC_SUCCESS;
  TempProfile               = 0;
  MrcData->MrcTimingProfile = 0;

  TimingParamHash (MrcData->TimingParamsConfig, &(MrcData->MrcTimingProfile));

  if ( IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) ) {
    Status = EnforceFusedFrequencies (MrcData);
  } else if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_CH0_MD_CH1_SOD) || (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_CH0_SOD_CH1_MD)) {
    // CurrentFrequency is from dimm, DramPolicyData.Profile is MD
    // Use lower freq
    if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency > ConfigToSpeed[MrcData->MrcMemConfig]) {
      MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
    }

    if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_CH0_MD_CH1_SOD) {
      // MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl is SODIMM, MrcData->TimingParamsConfig represents MD
      MrcData->NvData.MrcParamsSaveRestore.Channel[0].Tcl = (UINT8) GetTimingParam(BxtTimingtCL);
      i = 1;
    } else {
      MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl = (UINT8) GetTimingParam(BxtTimingtCL);
      i = 0;
    }

    //Set back because might be overwrite by the SMIP
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[i].SpdData[0].SpdPresent) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[i].Enabled = 1;
    } else {
      if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == 0) {
        MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
      }
    }

    // Determine common tcl
    // MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl is SODIMM, MrcData->TimingParamsConfig represents MD
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl < GetTimingParam(BxtTimingtCL)) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl = (UINT8) GetTimingParam(BxtTimingtCL);
      MrcData->NvData.MrcParamsSaveRestore.Channel[0].Tcl = (UINT8) GetTimingParam(BxtTimingtCL);
    } else {
      MrcData->NvData.MrcParamsSaveRestore.Channel[0].Tcl = MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl;
    }

    // Determine common profile
    switch (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency) {
    case MMRC_1333:
      MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile = DDR3_1333_7_7_7;
      i = 8;
      break;
    case MMRC_1600:
      MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile = DDR3_1600_8_8_8;
      i = 9;
      break;
    case MMRC_1866:
      MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile = DDR3_1866_10_10_10;
      i = 11;
      break;
    default:
      MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile = DDR3_1333_7_7_7;
      i = 8;
      break;
    }

    while (i++ <= MrcData->NvData.MrcParamsSaveRestore.Channel[0].Tcl)  {
      MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile += 1;
    }

    //overwrite to the new common profile
    TempProfile = MrcData->MrcMemConfig;
    if (ConfigToDdrType[TempProfile] == TypeDdr3L) {
      MrcData->TimingParamsConfig = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile;
      Status = TimingParamHash (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile, &(MrcData->MrcMemConfig));
      TimingParamHash (MrcData->TimingParamsConfig, &(MrcData->MrcTimingProfile));
    }
  }
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency = (1 << MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency);
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTrp] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Tcl;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TimingData[TimingTrcd] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Tcl;
  }
  return Status;
}


/**
  Mirrors address bits

  @param[in]  data   Data for MRS write

  @retval  dataNew   Mirrored data
**/
UINT16
MirrorAddrBits (
  IN    UINT16 data
)
{
  UINT16  dataNew;

  dataNew = data & ~0x1F8;
  //
  // Swap of bits (3,4), (5,6), (7,8)
  //
  dataNew |= ((data & BIT3) << 1) | ((data & BIT4) >> 1) | ((data & BIT5) << 1) | ((data & BIT6) >> 1) | ((data & BIT7) << 1) | ((data & BIT8) >> 1);

  return dataNew;
} // MirrorAddrBits

/**
  Mirrors bank address bits

  @param[in]  bank   Bank address to be mirrored

  @retval  bankNew   Mirrored bank address
**/
UINT8
MirrorBankBits (
  UINT8 bank
)
{
  UINT8 bankNew;

  bankNew = bank & ~0x3;
  //
  // Swap bits 0 and 1 of the bank address
  //
  bankNew = ((bank & BIT0) << 1) | ((bank & BIT1) >> 1);

  return bankNew;
} // MirrorBankBits

/**
  HandleAddressMirroring

  @param[in,out]  Data
  @param[in]      AddressMirroringSpdByte

  @retval  None
**/
VOID
HandleAddressMirroring (
  IN  OUT   UINT32 *Data,
  IN        UINT8   AddressMirroringSpdByte
)
{
  DramInitMisc MiscCommand;

  MiscCommand.Data = *Data;

  if (AddressMirroringSpdByte & 1) {
    if (MiscCommand.Data & BIT30) {
      MiscCommand.Bits.multAddress  = (UINT16) MirrorAddrBits ((UINT16) MiscCommand.Bits.multAddress);
      MiscCommand.Bits.bankAddress  = (UINT8) MirrorBankBits ((UINT8) MiscCommand.Bits.bankAddress);
    }
  }

  *Data = MiscCommand.Data;
}


/**
  WriteMrsCommand

  @param[in,out]  MrcData       Host structure for all data related to MMRC
  @param[in]      Channel
  @param[in]      Rank
  @param[in]      MiscCommand

  @retval  None
**/
VOID
WriteMrsCommand (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT32        MiscCommand
)
{
  MrcData->DynamicVars[Channel][RANK]        = Rank;
  MrcData->DynamicVars[Channel][RDDCMD]      = 0;
  MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
  MrcData->DynamicVars[Channel][REG_DATA]    = MiscCommand;
  SendDramCmd(MrcData, Channel);
}

/**
  ReadMrsCommand

  @param[in,out]  MrcData       Host structure for all data related to MMRC
  @param[in]      Channel
  @param[in]      Rank
  @param[in]      Index

  @retval  Data
**/
UINT32
ReadMrsCommand (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         Index
)
{
  DramInitMisc  MiscCommand;

  MiscCommand.Data = (UINT32) MrcData->DynamicVars[Channel][MrsCommandIndex[Rank][Index]];
  return MiscCommand.Data;
}

UINT32
GetAddress (MMRC_DATA *MrcData, UINT8 Channel, UINT8 Rank)
{
  return (UINT32)(Rank << RANK_SHIFT_BIT);
}


/**
  Output a progress data to port 80 for debug purpose
  Could be safely overriden to
  send checkpoints elsewhere, such as port 0x84 or a serial port

  @param[in]  Content   Hexadecimal value to be sent thru debug port

  @retval  None
**/
VOID
CheckpointSet (
  IN        UINT16    Content
)
{
//[-start-160407-IB07400715-modify]//
#ifndef CHIPSET_POST_CODE_SUPPORT
  IoOut32 (0x80, (UINT8) Content);
#else  
  if (Content & 0xFF00) {
    CHIPSET_POST_CODE ((UINT16)Content);
  } else {
    Content |= (PEI_MRC_POST_CODE_HIGH_BYTE << 8); // MRC POST CODE
    CHIPSET_POST_CODE ((UINT16)Content);
  }
#endif  
//[-end-160407-IB07400715-modify]//
}
/*
MMRC_STATUS
DumpDdrioRegisters (
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT8      Channel
)
{
  UINT16 Offset;
  UINT32 Value;
  REGISTER_ACCESS Register;

  Register.Mask   = 0xFFFFFFFF;

  MmrcDebugPrint ((MMRC_DBG_MIN, "DDRIO CH%d\n", Channel));

  for (Offset = 0; Offset < 0x8000; Offset += 4) {
    Register.Offset = Offset;
    Value = (UINT32) MemRegRead (DDRDQ_BOXTYPE, Channel, 0, Register);
    MmrcDebugPrint ((MMRC_DBG_MIN, "0x%08x 0x%08x\n", Register.Offset, Value));
  }
  return MMRC_SUCCESS;
}
*/
/**
  DumpDunitRegisters

  @param[in,out]  MrcData
  @param[in]  Channel

  @retval  MMRC_STATUS
**/
MMRC_STATUS
DumpDunitRegisters (
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT8      Channel
)
{
  UINT16 Offset;
  UINT32 Value;
  REGISTER_ACCESS Register;

  Register.Mask   = 0xFFFFFFFF;

  MmrcDebugPrint ((MMRC_DBG_MIN, "DUNIT%d\n", Channel));

  for (Offset = 0; Offset < 0x184; Offset+=4) {
    Register.Offset = Offset;
    Value = (UINT32) MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register);
    MmrcDebugPrint ((MMRC_DBG_MIN, "0x%08x 0x%08x\n", Register.Offset, Value));
  }
  return MMRC_SUCCESS;
}

/**
  Use BreakStrobeLoop to break the cycles for Delays that don't need to complete the strobe cycle

  @param[in]  MrcData      Host structure for all data related to MMRC
  @param[in]  Channel      Current Channel being examined.
  @param[in]  Rank         Current Rank being examined.
  @param[in]  Dim1Index    Delay Signal
  @param[in]  Strobe       Strobe

  @retval    Others as failed
  @retval    MMRC_SUCCESS as passed
**/
MMRC_STATUS
BreakStrobeLoop (
  IN   MMRC_DATA    *MrcData,
  IN   UINT8         Channel,
  IN   UINT8         Rank,
  IN   UINT16        Dim1Index,
  IN   UINT8        *Strobe
){
  //
  // For the CMD, CTL group signals break right at the first strobe
  //
  if ((IsCmdSignalDelay (MrcData, Dim1Index) ||
    Dim1Index == CkAll || Dim1Index == CkGrp0 || Dim1Index == CkGrp1 ||
    Dim1Index == CtlAll || Dim1Index == CtlGrp0 || Dim1Index == CtlGrp1 ||
    Dim1Index == CmdAll ||Dim1Index == CmdGrp0 || Dim1Index == CmdGrp1 ) && (INT8)(*Strobe) >= 0) {
      *Strobe = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank];
  }
  //
  // Different projects can add different conditions per delay that uses the same per strobe loop
  //
  return MMRC_SUCCESS;
}

/**
  SignalUpperBound

  @param[in]  MrcData      Host structure for all data related to MMRC
  @param[in]  Channel      Current Channel being examined.
  @param[in]  Rank         Current Rank being examined.
  @param[in]  Dim1Index    Delay Signal

  @retval    Data
**/
UINT32
SignalUpperBound (
  IN         MMRC_DATA    *MrcData,
  IN         UINT8        Channel,
  IN         UINT8        Rank,
  IN         UINT8        Dim1Index
)
{
  return 0;
}

MMRC_STATUS SetPBDEnable(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  if (MrcData->ReadPerBitEnable)
  {
    MrcData->DynamicVars[Channel][SS_TEMP_VAR]=1;
    MmrcExecuteTask (MrcData, gen_rxpbden_set, NO_PRINT, Channel);
    MmrcDebugPrint ((MMRC_DBG_MIN, "Setting RxPBD Enable\n"));
  }
  else
  {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Setting RxPBD Disable\n"));
  }

  return MMRC_SUCCESS;
}

void FillDataSel (MMRC_DATA* MrcData, PTDATAMUXSEL_DQ_BOXTYPE_STRUCT * datamux, UINT8 ch, UINT8 bl)
{
  UINT8 *Mapping, *Mapping2;
  Mapping = MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Dimm2DunitReverseMapping;
  Mapping2 = MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Dimm2DunitMapping;

  datamux->Bits.data0sel  = Mapping[(bl*8) + 0] % 8;
  datamux->Bits.data1sel  = Mapping[(bl*8) + 1] % 8;
  datamux->Bits.data2sel  = Mapping[(bl*8) + 2] % 8;
  datamux->Bits.data3sel  = Mapping[(bl*8) + 3] % 8;
  datamux->Bits.data4sel  = Mapping[(bl*8) + 4] % 8;
  datamux->Bits.data5sel  = Mapping[(bl*8) + 5] % 8;
  datamux->Bits.data6sel  = Mapping[(bl*8) + 6] % 8;
  datamux->Bits.data7sel  = Mapping[(bl*8) + 7] % 8;
}

void SetupPeriodicDDRIOSwizzle(MMRC_DATA * MrcData, UINT8 Channel)
{
  REGISTER_ACCESS Register;
  UINTX   Value;
  PTDATAMUXSEL_DQ_BOXTYPE_STRUCT * datamux;
  PTCTL0_DQDPLVUG_BOXTYPE_STRUCT* ptrctl;
  UINT8 bl;

  //
  // Get BL0/1 Bit Swizzle Register
  //
  Register.Offset = PTDATAMUXSEL_DQ_BOXTYPE_REG;
  Register.ShiftBit=0;
  Register.Mask=0xFFFFFFFF;
  bl = MrcData->MasterFloorPlan[MrcData->CurrentBlueprint][Channel][0].Strobelane;
  MmrcDebugPrint ((MMRC_DBG_MIN, "CH=%d I=%d bl=%d\n", Channel, 0, bl));
  Value = MemRegRead (DQ_BOXTYPE, Channel, 0, Register);
  datamux = (PTDATAMUXSEL_DQ_BOXTYPE_STRUCT *) &Value;  //Bit encoding is the same between DQ, and CCC
  FillDataSel (MrcData, datamux, Channel, 0);
  MemRegWrite(DQ_BOXTYPE, Channel, 0, Register, Value, 0xF);
  //
  // Get ByteSwizzle Register
  //
  //bl = MrcData->MasterFloorPlan[MrcData->CurrentBlueprint][Channel][0].Strobelane;
  Register.Offset = PTCTL0_DQDPLVUG_BOXTYPE_REG;
  Value = MemRegRead (DQDPLVUG_BOXTYPE, Channel, 0, Register);
  ptrctl = (PTCTL0_DQDPLVUG_BOXTYPE_STRUCT *) &Value;
  ptrctl->Bits.bytelane1datasel=bl%2;
  MemRegWrite(DQDPLVUG_BOXTYPE, Channel, 0, Register, Value, 0xF);

  //
  // Get BL2/3 Bit Swizzle Register
  //
  Register.Offset = PTDATAMUXSEL_DQ_BOXTYPE_REG;
  Register.ShiftBit=0;
  Register.Mask=0xFFFFFFFF;
  bl = MrcData->MasterFloorPlan[MrcData->CurrentBlueprint][Channel][2].Strobelane;
  MmrcDebugPrint ((MMRC_DBG_MIN, "CH=%d I=%d bl=%d\n", Channel, 2, bl));
  Value = MemRegRead (DQ_BOXTYPE, Channel, 1, Register);
  datamux = (PTDATAMUXSEL_DQ_BOXTYPE_STRUCT *) &Value;  //Bit encoding is the same between DQ, and CCC
  FillDataSel (MrcData, datamux, Channel, 2);
  MemRegWrite(DQ_BOXTYPE, Channel, 1, Register, Value, 0xF);
  //
  // Get ByteSwizzle Register
  //
  Register.Offset = PTCTL0_DQDPLVUG_BOXTYPE_REG;
  Value = MemRegRead (DQDPLVUG_BOXTYPE, Channel, 1, Register);
  ptrctl = (PTCTL0_DQDPLVUG_BOXTYPE_STRUCT *) &Value;
  ptrctl->Bits.bytelane1datasel=bl%2;
  MemRegWrite(DQDPLVUG_BOXTYPE, Channel, 1, Register, Value, 0xF);
}

#define ENABLE_PERIODIC_DQ_ADJUST 1
/**
  GetManualInitialIndex

  @param[in,out]  MrcData        Host structure for all MRC global data.
  @param[in]  Rank
  @param[in]  Channel
  @param[in]  ro_table
  @param[in]  RestorePath

  @retval  Data
**/
UINT32 GetManualInitialIndex(
  MMRC_DATA         *MrcData,
  UINT8             Rank,
  UINT8             Channel,
  UINT16            *ro_table,
  BOOLEAN           RestorePath,
  UINT8             Instance      //Selects instance, where there are more than one 16'bit instances available
  )
{
  UINT8  MR18Data, MR19Data;//, MRRData;
  UINT16 MRCombinedData;
  UINT8  SMR18Data, SMR19Data;
  UINT16 SMRCombinedData;
//  UINT8  DMR18Data, DMR19Data;
//  UINT16 DMRCombinedData;
  UINT16 RO1, RO2;
  REGISTER_ACCESS Register;
  UINT32  TempValue;
  UINT8  bl=MrcData->MasterFloorPlan[MrcData->CurrentBlueprint][Channel][0 + (Instance * 2)].Strobelane;
  UINT64 RegData;
  D_CR_MR4_DESWIZZLE_DUNIT_BOXTYPE_STRUCT *mr4_deswizzle;
  UINT32 mr4_byte_sel_save=0;
  Register.Mask   = 0xFFFFFFFF;
  Register.ShiftBit = 0;

  //
  //  Read rdcmd2datavalid and set ptloadcode_dly=3
  //
  MrcData->DynamicVars[Channel][PTLDCODE]=1;
  MmrcExecuteTask (MrcData, PERIODIC_TRAINING_READ, NO_PRINT, Channel);
  MySimStall(2000);
  MrcDelay (MrcData, MICRO_DEL, 10);

  //RL+tDQSCK+4/2+1   , (rdcmd2rdvld + 4) * 4
  //+4 = PHY.ptctl0.ptloadcode_dly+2
  /*
    This is fixed in BXTP-A0
  */
  //if (MrcData->DynamicVars[0][BXT_A0_WA] == 1) //This value needs to be set only for A0
  //MrcData->DynamicVars[Channel][TRWDR] = ((UINT32)MrcData->DynamicVars[Channel][RDCMD2RDVLD] + (UINT32)MrcData->DynamicVars[Channel][PTLDCODE] + 2  + 4) * 4;
  //In BXT-P TPSTMRCLK is programmed statically according to values from Jeff
  /*
  TPSTMRBLK = Max( (RL + tDQSCK +MRR/MRW BL/2 + 1), (t_rdcmdtype_delay * 4) )
                         1600  2133  2666  3200
    t_rdcmdtype_delays)   8    9     10     11
  */
  MrcData->DynamicVars[Channel][TPSTMRRBLK] = ((UINT32) MrcData->DynamicVars[Channel][RDCMD2RDVLD] + (UINT32)MrcData->DynamicVars[Channel][PTLDCODE] + 2) * 4;
  MmrcDebugPrint((MMRC_DBG_MIN, "TPSTMRRBLK=%d\n", (UINT32) MrcData->DynamicVars[Channel][TPSTMRRBLK]));
  //MmrcDebugPrint((MMRC_DBG_MIN, "TRWDR=%d\n", (UINT32) MrcData->DynamicVars[Channel][TRWDR]));
  //MrcData->DynamicVars[Channel][TPSTMRBLK] = 0x2c;
  if (RestorePath)
  {
    GetSetDdrioGroup2(MrcData, Channel, Rank, Instance*2, INITIAL_INDEX, CMD_GET_CACHE, &TempValue);
    MmrcDebugPrint ((MMRC_DBG_MIN, "CH=%d R=%d Restoring 0x%04x\n", Channel, Rank, TempValue));
  }
  else
  {
    //
    // Before we do anything, setup the MR4 Bytesel to the appropriate byte in the upper or lower nibble.  This is needed for BXTP-A0.  BXTP-B0 has a separate register
    // for each device. (MRRDATA, and MRRDATA2)
    //
    Register.Offset = D_CR_MR4_DESWIZZLE_DUNIT_BOXTYPE_REG;
    RegData = MemRegRead(DUNIT_BOXTYPE, Channel, 0, Register);
    mr4_deswizzle = (D_CR_MR4_DESWIZZLE_DUNIT_BOXTYPE_STRUCT *) &RegData;
    mr4_byte_sel_save = (UINT32) mr4_deswizzle->Bits.mr4_byte_sel;
    mr4_deswizzle->Bits.mr4_byte_sel=bl;
    MemRegWrite(DUNIT_BOXTYPE, Channel, 0, Register, RegData, 0xFF);
    //
    // Issue the MPC-1 command
    //
    MrcData->DynamicVars[Channel][RANK]        = Rank;
    MrcData->DynamicVars[Channel][REG_DATA]    = MPC2(0, START_DQS_OSC);
    MrcData->DynamicVars[Channel][RDDCMD]      = 0;
    MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);

    MySimStall(200);
    MrcDelay (MrcData, MICRO_DEL, 10);
    //
    //  Issue MR18
    //
    MrcData->DynamicVars[Channel][RANK]        = Rank;
    MrcData->DynamicVars[Channel][REG_DATA]    = MRR_LP4(18);
    MrcData->DynamicVars[Channel][RDDCMD]      = 1;
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
    MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    // Wait a Bit
    MySimStall(200);
    MrcDelay (MrcData, MICRO_DEL, 1);
    //
    // Read MR18 value from DUNIT
    //
    Register.Offset = D_CR_DCMDDATA_DUNIT_BOXTYPE_REG;
    MR18Data = (UINT8) (MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register) & 0xFF);
    SMR18Data = (UINT8) SwizzleBytelane(MrcData, Channel, bl, MR18Data);
    //DMR18Data = (UINT8) DeSwizzleBytelane(MrcData, Channel, bl, MR18Data);

    //
    //  Issue MR19
    //
    MrcData->DynamicVars[Channel][REG_DATA]    = MRR_LP4(19);
    MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    // Wait a Bit
    MySimStall(200);
    MrcDelay (MrcData, MICRO_DEL, 1);
    //
    // Read MR19 value from DUNIT
    //
    Register.Offset = D_CR_DCMDDATA_DUNIT_BOXTYPE_REG;
    MR19Data = (UINT8) (MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register) & 0xFF);
    SMR19Data = (UINT8) SwizzleBytelane(MrcData, Channel, bl, MR19Data);
    //DMR19Data = (UINT8) DeSwizzleBytelane(MrcData, Channel, bl, MR19Data);
    MRCombinedData = MR18Data | (MR19Data << 8);
    SMRCombinedData = SMR18Data | (SMR19Data << 8);
    //DMRCombinedData = DMR18Data | (DMR19Data << 8);


    MrcDelay(MrcData, 1, 20);

    RO1=0xFFFF;
    if ((Channel == 0) && (Instance==0) && (Rank==0))
    {
      for (TempValue = 0; TempValue < 70; TempValue++) {
        RO2 = RO1;
        RO1 = ro_table[TempValue];
        MmrcDebugPrint ((MMRC_DBG_MIN, "%d:%d-%d\n", TempValue, RO1, RO2));
      }
    }
    RO1=0xFFFF;
    for (TempValue = 0; TempValue < 70; TempValue++) {
      RO2 = RO1;
      RO1 = ro_table[TempValue];
      if ((SMRCombinedData < RO2) && (SMRCombinedData >= RO1))
      {
        break;
      }
    }
    //MmrcDebugPrint ((MMRC_DBG_MIN, "CH=%d R=%d I=%d bl=%d MR18=0x%02x S:0x%02x D:0x%02x MR19=0x%02x S:0x%02x D:0x%02x Combined=0x%04x S:0x%04x D:0x%04x initial_index=%04d\n", Channel, Rank, Instance, bl, MR18Data, SMR18Data, DMR18Data, MR19Data, SMR19Data, DMR19Data, MRCombinedData, SMRCombinedData, DMRCombinedData, TempValue));
    MmrcDebugPrint ((MMRC_DBG_MIN, "CH=%d R=%d I=%d bl=%d MR18=0x%02x S:0x%02x MR19=0x%02x S:0x%02x Combined=0x%04x S:0x%04x initial_index=%04d\n", Channel, Rank, Instance, bl, MR18Data, SMR18Data, MR19Data, SMR19Data, MRCombinedData, SMRCombinedData, TempValue));
    Register.Offset = D_CR_MR4_DESWIZZLE_DUNIT_BOXTYPE_REG;
    mr4_deswizzle->Bits.mr4_byte_sel=mr4_byte_sel_save;
    MemRegWrite(DUNIT_BOXTYPE, Channel, 0, Register, RegData, 0xFF);
  }
  return TempValue;

 // return 0;
}

MMRC_STATUS ResetHighCmdCtlValues(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT32 TempValue;
  UINT8  Rank;
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    //
    // Only execute  on ranks that are enabled.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CtlAll, CMD_GET_CACHE, &TempValue);
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CtlAll, CMD_SET_VAL_FC_UC, &TempValue);
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CmdAll, CMD_GET_CACHE, &TempValue);
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CmdAll, CMD_SET_VAL_FC_UC, &TempValue);
  }
  return MMRC_SUCCESS;
}

/**
  SWSetHiFrequency: configures internal software variables for the High DRAM frequency
  to be used when initializing LPDDR4 type DRAM. This is the final operating frequnecy
  for the DRAM

  @param[in]  ModMrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
SWSetHiFrequency (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channelx
  )
{
  int ch;
  UINT8 NewFrequency;
  //
  //  Setup High Frequency as the current Software Frequency
  //
  NewFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
  if (NewFrequency == BOOT_FREQUENCY) {
    //
    // Don't need to switch frequency if "High Frequency" is the "Boot Frequency"
    //
    return MMRC_SUCCESS;
  }
  MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = NewFrequency;

  MmrcDebugPrint((MMRC_DBG_MIN, "Speed set to:%d\n", MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency));
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency = MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency;
  }
  return MMRC_SUCCESS;
}

/**
  SWSetBootFrequency: configures internal software variables for the low DRAM frequency
  to be used when initializing LPDDR4 type DRAM. This allows spreadsheet phy init tasks
  to be called for the lower frequency.

  @param[in]  ModMrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
SWSetBootFrequency (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channelx
  )
{
  int ch;
  //
  //  Setup Boot Frequency as the current Software Frequency
  //
  if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == BOOT_FREQUENCY) {
    //
    // Don't need to switch frequency if "High Frequency" is the "Boot Frequency"
    //
    return MMRC_SUCCESS;
  }
  MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = BOOT_FREQUENCY;
  MmrcDebugPrint((MMRC_DBG_MIN, "Set Software Init Freq:%d Speed set to:%d\n", BOOT_FREQUENCY, MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency));
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency = MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency;
  }
  return MMRC_SUCCESS;
}

/**
  HWSetBootFrequency: Configures the PHY for operation at the Low DRAM frequency

  @param[in]  ModMrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
HWSetBootFrequency (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  int ch;
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr4) {
    return MMRC_SUCCESS;
  }
  //
  // When the High Frequency == BOOT_FREQUENCY, we only need to call SetupMaxPI once.
  // For any other frequency, we need to call "MrcChangeFreq" to handle the PLL re-locking.
  //
  if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == BOOT_FREQUENCY) {
    if(MrcData->NvData.MrcParamsSaveRestore.SetupMaxPiDone[GetCurrentFrequencyIndex()]) {
      return MMRC_SUCCESS;
    } else {
      SetupMaxPI(MrcData, 0, 0, 0);
      return MMRC_SUCCESS;
    }
  }
  MrcData->DefaultCmdClkCtl=TRUE;
  //
  //  Setup High Frequency as the current Frequency
  //
  MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = BOOT_FREQUENCY;
  MmrcDebugPrint((MMRC_DBG_MIN, "Set Boot Freq:%d Speed set to:%d\n", BOOT_FREQUENCY, MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency));
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency = MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency;
  }
  //
  // Perform the Frequency Change
  //
  MmrcChangeFreq(MrcData, CapsuleStartIndex, StringIndex, Channel);
  return MMRC_SUCCESS;
}

/**
  HWSetHiFrequency: Configures the PHY for operation at the High DRAM frequency

  @param[in]  ModMrcData
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
HWSetHiFrequency (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  int ch;
  UINT8 NewFrequency;
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeLpDdr4) {
    return MMRC_SUCCESS;
  }
  NewFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
  //
  // When the High Frequency == BOOT_FREQUENCY, we only need to call SetupMaxPI once.
  // For any other frequency, we need to call "MrcChangeFreq" to handle the PLL re-locking.
  //
  if (NewFrequency == BOOT_FREQUENCY ) {
    if(MrcData->NvData.MrcParamsSaveRestore.SetupMaxPiDone[GetCurrentFrequencyIndex()]) {
      return MMRC_SUCCESS;
    } else {
      SetupMaxPI (MrcData, 0, 0, 0);
      return MMRC_SUCCESS;
    }
  }
  //
  //  Setup High Frequency as the current Frequency
  //
  MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency = ConfigToSpeed[MrcData->MrcMemConfig];
  MmrcDebugPrint((MMRC_DBG_MIN, "Speed set to:%d\n", MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency));
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency = MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency;
  }
  //
  // Perform the Frequency Change
  //
  MmrcChangeFreq (MrcData, CapsuleStartIndex, StringIndex, Channel);
  return MMRC_SUCCESS;
}

MMRC_STATUS
MmrcChangeFreq (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channelx
  )
{
  UINT8 ch;
  UINT8 rk = 0;

  //
  // Put the DUNIT into PM2
  //
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, DunitFreqChangePart1, NO_PRINT, ch);
  }
  //
  // Put most of the Phy in Reset
  //

  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, modmem_lp4_freq_switch_seq1, NO_PRINT, ch);
  }

  //  Setup PLL settings for current Frequency
  //
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, modmem_init_lpddrgrp2m_seq, NO_PRINT, ch);
  }
  //  PLL Lock
  //
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, modmem_lp4_freq_switch_seq2, NO_PRINT, ch);
  }
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, modmem_lp4_freq_switch_seq2a, NO_PRINT, ch);
  }
  //
  // Initialize the DLL, and program the delay values to appropriate locations
  //
  SetupMaxPI(MrcData, NO_PRINT, NO_PRINT, 0);

  //
  // Program the CMD/CTL/CLK to the New Frequency values
  //
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcDebugPrint((MMRC_DBG_MIN, "DefaultCmdClkCtl=%d\n", MrcData->DefaultCmdClkCtl));
    if (MrcData->DefaultCmdClkCtl == TRUE)
    {
      MmrcExecuteTask (MrcData, modmem_init_progcmdckctl_seq, NO_PRINT, ch);
    }
    else
    {
      //This condition is important because otherwise for LP4 single channel mode, MRC
      //tries to set CmdCtl Timing delay values for CH1 as well which is
      //disabled.
      //rk =0 is fine because CmdCtl delay values are shared across ranks
      if (RunOnThisChannel(MrcData, ch, rk)) {
        ResetHighCmdCtlValues(MrcData, NO_PRINT, NO_PRINT, ch);
      }
    }
  }
  //
  // Bring Phy out of reset
  //
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, modmem_lp4_freq_switch_seq3, NO_PRINT, ch);
  }
  //
  // Bring the DUNIT out of PM2
  //
  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    MmrcExecuteTask (MrcData, DunitFreqChangePart3, NO_PRINT, ch);
  }
  return MMRC_SUCCESS;
}

#ifdef CAR
MMRC_STATUS
EV_Testing(
MMRC_DATA         *MrcData,
UINT16            CapsuleStartIndex,
UINT16            StringIndex,
UINT8             Channel
)
{
  return MMRC_SUCCESS;
}
#endif

/**
  Returns the corresponding clock crossing table entry for a given CcRange index.

  @param[in]   CcRange   The requested clock crossing range

  @retval      Returns the corresponding clock crossing table entry for a given CcRange
               index. If a valid entry is not found, deadloops and reutrns an entry
               populated with -1.
**/
CLOCK_CROSSINGS
ClockCrossings (
  UINT8 CcRange
  )
{
  MMRC_DATA *MrcData = GetMrcHostStructureAddress();
  CLOCK_CROSSINGS retVal;
  MmrcMemset (&retVal, -1, sizeof (CLOCK_CROSSINGS));

  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4 || MrcData->CpuStepping == STEPPING_P0) {
    return ClockCrossings_AX_ALL_BX_LPDDR4[CcRange];
  } else if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr3 || ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    return ClockCrossings_BX_LPDDR3_DDR3L[CcRange];
  }
  MmrcDebugPrint((MMRC_DBG_MIN, "\nCC ERR\n"));
  MrcDeadLoop ();
  return retVal;
}

/**
  Setup internal PHY rank switching settings, required to switching picodes reliably insid ethe DDR PHY.
  These settings depend on trained settings established for ReceiveEnable and WriteTraining.  Please
    place this algorithm after any ReceiveEnable and WriteTraining that might occur.
  This function needs to be run on fast boot as well, *after* ReceiveEnable and WriteTraining values
    have been restored.
  This function is specific to the Read path

  @param[in, out]  MrcData           Host structure for all data related to MMRC.
  @param[in]       CapsuleStartIndex Starting point within the CapsuleData structure
                                     where the Init should begin.
  @param[in]       StringIndex       When provided, this is the index into the string
                                     table to show the register names.
  @param[in]       Channel           Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS SetupPHYRankTurnaroundSettingsForReads(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  UINT8  Strobe;
  UINT8  Rank;
  INT32  total_max_rcven_pushout_in_1x_clocks;
  INT32  rdcmd2rkchglatency_pushout_in_1x_clocks;
  UINT32 rkchg_rd_init;// , rkchg_wr_init;
  UINT32 rk2rklat_rd;// , rk2rklat_wr;
  INT32  rcven_pi_code;
#define BYTELANES_PER_RDCMD2RKCHGLATENCY_REG 2
  INT32  rcven_off_in_pi_ticks[BYTELANES_PER_RDCMD2RKCHGLATENCY_REG];
  INT32  rcven_off;
  UINT8  ClockAdjust2x = 2; // RDCMD2RKCHGLATENCY already taken into account in hardware
  UINT32 CLOCK_IN_PI_TICKS_2X = MrcData->HalfClkPi;
  UINT32 CurrentTrrdrOffset;
  UINT32 MaxTrrdrOffset;

  total_max_rcven_pushout_in_1x_clocks = 0;
  for (Strobe = 0; Strobe < BYTELANES_PER_RDCMD2RKCHGLATENCY_REG; Strobe++) {
    rcven_off_in_pi_ticks[Strobe] = 0;
  }

  if (!IsStrobeValid(MrcData, Channel, 1, 0) || MrcData->Rk2RkEn.RxDqs == 0){
    // Skip RDCMD2RKCHGLATENCY Training for this channel
    // if RANK1 is not present or RXDQS RK2RK is disabled
    return MMRC_SUCCESS;
  }

  MaxTrrdrOffset = 0;
  CurrentTrrdrOffset = 0;
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid(MrcData, Channel, 0, Strobe)) {
      continue;
    }
    rcven_pi_code = 0;
    rdcmd2rkchglatency_pushout_in_1x_clocks = 0;
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      // RecEnDelay GetSet automatically handles the ECC bytelane.
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RecEnDelay, CMD_GET_REG, (UINT32 *) &rcven_off_in_pi_ticks[Strobe % BYTELANES_PER_RDCMD2RKCHGLATENCY_REG]);
      if (Strobe < MAX_STROBES_NON_ECC) {
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RecEnPiCode_dq, CMD_GET_REG, (UINT32 *) &rcven_pi_code);
      } else {
        GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RecEnPiCode_Ecc, CMD_GET_REG, (UINT32 *) &rcven_pi_code);
      }
      // Subract the PI Code from the linear RecEnDelay value. This is done prevent
      // rounding errors when converting from Pi ticks to 2x and 1x clocks
      rcven_off = rcven_off_in_pi_ticks[Strobe % BYTELANES_PER_RDCMD2RKCHGLATENCY_REG] - rcven_pi_code;
      MmrcDebugPrint((MMRC_DBG_MIN, "Ch=%d Rk=%d St=%d rcven_off=%d, rcven_pi_code=%d, ", Channel, Rank, Strobe, rcven_off, rcven_pi_code));

      // convert from Pi ticks to 2x Clocks and adjust
      rcven_off = (rcven_off / CLOCK_IN_PI_TICKS_2X) - ClockAdjust2x;

      // convert from 2x Clocks to 1x Clocks and round up
      rcven_off = (rcven_off + 1) / 2; // +1 to round up

      // take max offsets between ranks
      rdcmd2rkchglatency_pushout_in_1x_clocks = MAX(rdcmd2rkchglatency_pushout_in_1x_clocks, rcven_off);
      MmrcDebugPrint((MMRC_DBG_MIN, "max pushout=%d\n", rdcmd2rkchglatency_pushout_in_1x_clocks));
    }

    if (( Strobe % 2 ) == 0) {
      total_max_rcven_pushout_in_1x_clocks = rdcmd2rkchglatency_pushout_in_1x_clocks;  // reset the value
    } else {
      //
      // Use LINEAR PI values across two bytelanes
      // Calculate an offset for TRRDR which is:
      // currentOffset = [  ( 2X_CLOCK_IN_PITICS + ABS( PICODE_BL0 - PICODE_BL1 )) / 1X_CLOCK_IN_PITICKS ]
      //
      CurrentTrrdrOffset = (CLOCK_IN_PI_TICKS_2X + GET_ABSOLUTE(rcven_off_in_pi_ticks[0], rcven_off_in_pi_ticks[1])) / (CLOCK_IN_PI_TICKS_2X * 2);
      total_max_rcven_pushout_in_1x_clocks = MAX(total_max_rcven_pushout_in_1x_clocks, rdcmd2rkchglatency_pushout_in_1x_clocks); // take the max value between the even and odd bytelane
    }

    //
    // RDRK2RKLATENCY (rdcmd2rkchglatency)
    //
    if ((Strobe % 2) == 1 || Strobe == MAX_STROBES_NON_ECC) {    // only update the regsiter when on the ODD strobe.  skip if on even strobe.
      // Read the prior value (originally from PHY spreadsheet)
      if (Strobe < MAX_STROBES_NON_ECC) {
        // Note that only R0 value is written, not R1.  So make sure fast boot only restores R0 value.
        // Only read/write odd strobes since on BXTP, there is a 2:1 relationship between the RecEnDelay bytleanes and RdRk2RkLatency values.
        // This means there is one rdcmd2rkchglatency register field for every two bytelanes. The Strobe to DQCCC/CCC mapping is:
        // Strobe: Instance
        // S0/S1:  DQCCC 2
        // S2/S3:  DQCCC 3
        // S4/S5:  DQCCC 0
        // S6/S7:  DQCCC 1
        // S8:     CCC
        GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RdRk2RkLatency, CMD_GET_REG, &rk2rklat_rd);
      } else {
        GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RdRk2RkLatency_Ecc, CMD_GET_REG, &rk2rklat_rd);
      }
      rkchg_rd_init = rk2rklat_rd;
      // add the 1x clock value to the existing value in the register
      rk2rklat_rd += total_max_rcven_pushout_in_1x_clocks;
      // Update with the new value based on formula above, which is spreadsheet value + receive enable based offset
      if (Strobe < MAX_STROBES_NON_ECC) {
        GetSetDdrioGroup2(MrcData, Channel, 0, Strobe, RdRk2RkLatency, CMD_SET_VAL_FC_UC, &rk2rklat_rd);
      } else {
        GetSetDdrioGroup2(MrcData, Channel, 0, 0, RdRk2RkLatency_Ecc, CMD_SET_VAL_FC_UC, &rk2rklat_rd);
      }
    }
    MaxTrrdrOffset = MAX(MaxTrrdrOffset, CurrentTrrdrOffset);
  } // Strobe

  // Before returning, write out the max TRRDR Offset for the current channel
  GetSetDdrioGroup2(MrcData, Channel, 0, 0, TrrdrGetSet, CMD_GET_REG, &MrcData->DynamicVars[Channel][TRRDR]);
  MrcData->DynamicVars[Channel][TRRDR] += MaxTrrdrOffset;
  GetSetDdrioGroup2(MrcData, Channel, 0, 0, TrrdrGetSet, CMD_SET_VAL_FC_UC, &MrcData->DynamicVars[Channel][TRRDR]);
  MmrcDebugPrint((MMRC_DBG_MIN, "TRRDR+=%d\n", MaxTrrdrOffset));

  return MMRC_SUCCESS;
}

/**
  Setup internal PHY rank switching settings, required to switching picodes reliably insid ethe DDR PHY.
  These settings depend on trained settings established for ReceiveEnable and WriteTraining.  Please
  place this algorithm after any ReceiveEnable and WriteTraining that might occur.
  This function needs to be run on fast boot as well, *after* ReceiveEnable and WriteTraining values
  have been restored.
  This function is specific to the Write path

  @param[in, out]  MrcData           Host structure for all data related to MMRC.
  @param[in]       CapsuleStartIndex Starting point within the CapsuleData structure
                                     where the Init should begin.
  @param[in]       StringIndex       When provided, this is the index into the string
                   table to show the register names.
  @param[in]       Channel           Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
SetupPHYRankTurnaroundSettingsForWrites(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  UINT32 MaxTwwdrOffset;

  // Currently this equation is implemented only for DDR3L
  if (ConfigToDdrType[MrcData->MrcMemConfig] != TypeDdr3L) {
    return MMRC_SUCCESS;
  }

  if (!IsStrobeValid(MrcData, Channel, 1, 0) || (MrcData->Rk2RkEn.TxDq == 0 && MrcData->Rk2RkEn.TxDqs)){
    // Skip WRCMD2RKCHGLATENCY Training for this channel
    // if RANK1 is not present or TxDq and TxDqs RK2RK is disabled
    return MMRC_SUCCESS;
  }


  // Increase tWWDR by 1 (from 11 to 12 clocks)
  MaxTwwdrOffset = 1;
  GetSetDdrioGroup2(MrcData, Channel, 0, 0, TwwdrGetSet, CMD_GET_REG, &MrcData->DynamicVars[Channel][TWWDR]);
  MrcData->DynamicVars[Channel][TWWDR] += MaxTwwdrOffset;
  GetSetDdrioGroup2(MrcData, Channel, 0, 0, TwwdrGetSet, CMD_SET_VAL_FC_UC, &MrcData->DynamicVars[Channel][TWWDR]);
  MmrcDebugPrint((MMRC_DBG_MIN, "TWWDR+=%d\n", MaxTwwdrOffset));

  return MMRC_SUCCESS;
}

/**
  Setup internal PHY rank switching settings, required to switching picodes reliably insid ethe DDR PHY.
  These settings depend on trained settings established for ReceiveEnable and WriteTraining.  Please
    place this algorithm after any ReceiveEnable and WriteTraining that might occur.
  This function needs to be run on fast boot as well, *after* ReceiveEnable and WriteTraining values
    have been restored.

  @param[in, out]  MrcData           Host structure for all data related to MMRC.
  @param[in]       CapsuleStartIndex Starting point within the CapsuleData structure
                                     where the Init should begin.
  @param[in]       StringIndex       When provided, this is the index into the string
                                     table to show the register names.
  @param[in]       Channel           Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
SetupPHYRankTurnaroundSettings(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  SetupPHYRankTurnaroundSettingsForReads(MrcData, CapsuleStartIndex, StringIndex, Channel);
  SetupPHYRankTurnaroundSettingsForWrites(MrcData, CapsuleStartIndex, StringIndex, Channel);
  return MMRC_SUCCESS;
}

/*++

Routine Description:

  Sets the weak ODT and initiates a comp cycle if WEAK_ODT_CONFIG is enabled. Otherwise just returns.

Arguments:

  MrcData:       Host structure for all data related to MMRC

  Channel:       Channel to do the conversion on.

Returns:

  Success
  Failure

--*/
MMRC_STATUS
SetWeakOdt(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
#ifdef WEAK_ODT_CONFIG
  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4) {
    //
    // No need to do an RCOMP INIT for LP4 since we do not have a WEAK ODT setting for LP4
    //
    return MMRC_SUCCESS;
  }
  if (((SMIP_ODT_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].OdtConfig))->Bits.RxOdt == MRC_WEAK_ODT_CONFIG) {
    MrcData->DynamicVars[Channel][WEAK_ODT_CONFIG] = 1;
    MmrcExecuteTask (MrcData, WEAK_ODT_SET, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, modmem_init_lpddrgrp6p5_seq, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, modmem_init_lpddrgrp7p75_seq, NO_PRINT, Channel);
  }
#endif
  return MMRC_SUCCESS;
}

MMRC_STATUS
DiffAmpOverride_PostAstep(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  if (MrcData->CpuStepping != STEPPING_P0) { // BXTP-B0 and later
    MmrcExecuteTask(MrcData, DiffAmpOverride, StringIndex, Channel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
DiffAmpOverride_AstepOnly(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  if (MrcData->CpuStepping == STEPPING_P0) { // BXTP-A0 only
    MmrcExecuteTask(MrcData, DiffAmpOverride, StringIndex, Channel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
DiffAmpRestore_PostAstep(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  if (MrcData->CpuStepping != STEPPING_P0) { // BXTP-B0 and later
    MmrcExecuteTask(MrcData, DiffAmpRestore, StringIndex, Channel);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
DiffAmpRestore_AstepOnly(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  if (MrcData->CpuStepping == STEPPING_P0) { // BXTP-A0 only
    MmrcExecuteTask(MrcData, DiffAmpRestore, StringIndex, Channel);
  }
  return MMRC_SUCCESS;
}

/**
  Sets the CMD Timing mode to 2N if the SoC is not BXTP-A0. This function is intended to
  be called from the MMRC spreadsheet call table.

  @param[in]  MrcData           Host structure for all data related to MMRC.
  @param[in]  CapsuleStartIndex Not Used
  @param[in]  StringIndex       Not Used
  @param[in]  Channel           The channel to perfrom the operation
**/
MMRC_STATUS
SwitchTo2NWrapper(
  IN  OUT   MMRC_DATA *MrcData,
  IN        UINT16     CapsuleStartIndex,
  IN        UINT16     StringIndex,
  IN        UINT8      Channel
  )
{
  UINT32 Value;
  if (MrcData->CpuStepping != STEPPING_P0) { // BXTP-B0 and later only
    Value = 2;
    GetSetTimingMode(MrcData, 0, Channel, 0, 0, 0, 0, 0, 0, CMD_SET_VAL_FC_UC, &Value);
  }
  return MMRC_SUCCESS;
}

/*++

Routine Description:

  Returns the current frequency index, a value of BootFrequency or HighFrequency based
  on MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency

Arguments:

  None

Returns:

  BootFrequency
  HighFrequency

--*/
FREQUENCY_INDEX
GetCurrentFrequencyIndex(
  )
{
  MMRC_DATA *MrcData = GetMrcHostStructureAddress();
  if (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == BOOT_FREQUENCY) {
    return BootFrequency;
  } else {
    return HighFrequency;
  }
}

/*++

Routine Description:
  Finds the acutal Device density based upon various SMIP params

Arguments:

  @param[in]  MrcData           Host structure for all data related to MMRC.
  @param[in]  Channel           Current Channel being examined.

Returns:

  Returns the calculated Device Density

--*/
MEMORY_DEVICE_DENSITY
GetDeviceDensity(
  IN  MMRC_DATA *MrcData,
  IN  UINT8      Channel
)
{
  UINT8  DensityAct;
  MEMORY_DEVICE_DENSITY Density;
  //handle Acutal Density exceptions or return D_Size
  DensityAct = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0];
  switch ( DensityAct ) {
    case DRAM_DENSITY_4Gb :
     Density = DeviceDensity4Gb;
     break;

    case DRAM_DENSITY_6Gb :
     Density = DeviceDensity6Gb;
     break;

    case DRAM_DENSITY_8Gb :
     Density = DeviceDensity8Gb;
     break;

    default :
     //no exception; expect D_Size is accurate.
     Density = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0];
  }

  return Density;
}
