/** @file
  MmrcLibraries.c
  Modular MMRC  libraries used throughout the MMRC, these files
  are independent of the project.

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

@par Specification Reference:
**/
#include "MmrcLibraries.h"
#include "MmrcHooks.h"
#include "MmrcProjectDefinitionsGenerated.h"
#include "McFunc.h"
#ifndef SUSSW
#include <Library/BaseLib.h>
#endif

#if defined (JTAG) || defined (SIM)
  UINT32 WrVRef[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  UINT32 CaVRef[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
#endif

#if  RTWT_SIM ==1
  UINT32 WrVRef[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  static UINT32 RdVRef[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  static UINT32 CTLE[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  static UINT32 RxDqsP[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  static UINT32 RxDqsN[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  static UINT32 RxDqPerBit[MAX_RANKS][MAX_CHANNELS][MAX_STROBES][MAX_BITS];
  extern UINT16 SimReadData[15][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS][64][2];
  extern UINT16 SimWriteData[MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS][64][2];

#endif

#if MRC_HUMAN_READABLE_ERRORS
UINT8 mErrorCode[MrcErrMsgMaxError][50] = {
  "All Right",                                           // MrcErrOk
  "Debug",                                               // Err[ 1] - MrcErrDebug
  "DetectDimms Failure",                                 // Err[ 2] - MrcErrDimmsFailed
  "Install Pei Memory Error",                            // Err[ 3] - MrcErrInstallPeiMemory
  "Invalid Number of Channels",                          // Err[ 4] - MrcErrInvalidChannelNumber
  "Invalid Memory Size Found",                           // Err[ 5] - MrcErrInvalidMemorySize
  "Illegal Voltage Configuration",                       // Err[ 6] - MrcErrIllegalVoltageConfig
  "Mailbox Error",                                       // Err[ 7] - MrcErrMailbox
  "MemTest Error",                                       // Err[ 8] - MrcErrMemTest
  "Warm Reset Error",                                    // Err[ 9] - MrcErrWarmResetError
  "Error trying to get SPD data",                        // Err[10] - MrcErrGetSpdData
  "No Dimms are Present",                                // Err[11] - MrcErrNoDimmsPresent
  "Invalid Number of Ranks",                             // Err[12] - MrcErrInvalidRankNumber
  "Invalid Number of Dimms",                             // Err[13] - MrcErrInvalidDimmNumber
  "Error Configuring Memory",                            // Err[14] - MrcErrConfigureMemory
  "Null Pointer Error",                                  // Err[15] - MrcErrNullPointer
  "GetSet Value exceeds limit",                          // Err[16] - MrcSetLimitReached
  "Faulty Parts Tracking: Halt on Correctable Error",    // Err[17] - MrcErrFaultyPartCorrectable
  "Faulty Parts Tracking: Uncorrectable Error on CH0",   // Err[18] - MrcErrFaultyPartUncorrectable
  "Invalid Frequency",                                   // Err[19] - MrcErrInvalidFrequency
  "Platform Settings Error",                             // Err[20] - MrcErrPlatformSettings
  "Invalid Linear Value",                                // Err[21] - MrcErrInvalidLinearValue
  "Invalid Physical Value",                              // Err[22] - MrcErrInvalidPhysicalValue
  "More than 1 2x clock between Strobe and Rank",        // Err[23] - MrcErrRankStrobeClockDelta
  "Center 2D Accumulate Margin X is zero",               // Err[24] - MrcErrCenter2DAccMarginXZero
  "Center 2D Accumulate Margin Y is zero",               // Err[25] - MrcErrCenter2DAccMarginYZero
  "Early Command Error, Search beyond 1/2 CLK",          // Err[26] - MrcErrSearchBeyondHalfClk
  "Invalid value for current Power Knob",                // Err[27] - MrcErrInvalidPowerKnob
  "All Channels are disabled",                           // Err[28] - MrcErrChannelsDisabled
  "Setup error during Pass Gate Test",                   // Err[29] - MrcErrSetupErrorPassGate
  "Rank is not present",                                 // Err[30] - MrcRankNotSupported
  "IO level unsupported",                                // Err[31] - MrcIoLevelNotSupported
  "Margin group unsupported",                            // Err[32] - MrcMarginGroupNotSupported
  "Signal unsupported",                                  // Err[33] - MrcSignalNotSupported
  "Frequency Index not supported"                        // Err[34] - MrcFrequencyIndexNotSupported
};
#endif

UINT8 CacheTypePrefix [MaxCacheTypes] = {
  0x00,
  0x40,
  0x80,
  0xC0
};
#if SIM || JTAG
MMRC_DATA *MyMrcData;
#endif

UINT8  ResultsString[3][3]= {"P\0", "-\0","*\0"};

VOID
PrintTaskNameType (
  IN    TASK_DESCRIPTOR     *j
)
{
#if CAPSULESTRINGS
  MmrcDebugPrint ((MMRC_DBG_MIN, " %s\n", CapsuleStrings[j->StringIndex]));
#endif //CAPSULESTRINGS

}

UINT16
GetHalfClk (
  IN        MMRC_DATA         *MrcData,
  IN        UINT8             Channel
  )
{
  if (MrcData->HalfClkPi==0)
  {
    if (MrcData->DigitalDll < 2) {
      return HalfClk [ADdll [MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency];
    } else {
      return 0;
    }
  }
  else
  {
    return MrcData->HalfClkPi;
  }
}

UINT16
GetQtrClk (
  IN        MMRC_DATA         *MrcData,
  IN        UINT8             Channel
  )
{
  return (GetHalfClk (MrcData, Channel) / 2);
}

UINT16
GetOneClk (
  IN        MMRC_DATA         *MrcData,
  IN        UINT8             Channel
  )
{
  return (GetHalfClk (MrcData, Channel) * 2);
}

/**
  This routine converts a PFCT value from BITx to x.
  The Current* variables must not be in BITx format. For example
  if CurrentPlatform is set to BIT4, this routine sets
  CurrentPlatform to 4 instead.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @retval          NONE
**/
VOID
ConvertPfctFromBitXToX (
  IN        MMRC_DATA         *MrcData
  )
{
  UINT8 i;
  UINT8 Channel;
  //
  // Since Current* variables is setup as BITx, need to convert BITX into x for the variable
  // that MMRC needs to do its platform comparisons in the CreatePfctSel.
  //
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    for (i = 0; i < 16; i++) {
      if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform & (1 << i)) != 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform = i;
      }
      if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency & (1 << i)) != 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency = i;
      }
      if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration & (1 << i)) != 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration = i;
      }
      if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType & (1 << i)) != 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType = i;
      }
    }
  }
}

/**
  This function compares the current PFCT of this channel and
  match it to the list of allowable PFCTs for this Task in the
  call table.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       j               Task Descriptor.
  @retval          NONE
**/
VOID
CreateChannelMask (
  IN        MMRC_DATA            *MrcData,
  IN        TASK_DESCRIPTOR      *j
  )
{
  UINT8                   Channel;

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (((j->PList & (1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform))      != 0) &&
        ((j->FList & (1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency))     != 0) &&
        ((j->CList & (1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration)) != 0) &&
        ((j->TList & (1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType))       != 0)
      )
    {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel = TRUE;
    } else {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel = FALSE;
    }
  }
}

 /**

@todo   add description

**/
VOID * Mmrcmemcpy(VOID *dst, VOID *src, UINT32 cnt) {
  volatile UINT8 *dst8 = (UINT8 *)dst;
  volatile UINT8 *src8 = (UINT8 *)src;
  while (cnt--) {
    *dst8++ = *src8++;
  }
  return dst;
}

#if CAR
void * __cdecl memcpy(void *dst, void *src, size_t cnt)
{
  return Mmrcmemcpy(dst, src, cnt);
}
#endif

/**
  Single entry point to MMRC. MRC calls this function
  and then MMRC acts as a black box, performing the requested
  stage of DDRIO init and returns when complete or when
  an error is encountered.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Action          Phy init, training, post-training, etc.
  @param[in, out]  Parameters      A structure used to pass data between MRC and MMRC.
  @retval          Status
**/
MMRC_STATUS
MmrcEntry (
  IN  OUT   MMRC_DATA            *MrcData,
  IN        MMRC_STAGE            Action,
  IN  OUT   HANDSHAKE_PARAMETERS *Parameters
  )
{
  MMRC_STATUS             Status;
  UINT8                   Channel;
  TASK_DESCRIPTOR         *j;
  BOOLEAN                 ExecuteOnAnyChannel;

  Status = MMRC_SUCCESS;
#if RMT_JUMP_POSTCODES
  MrcData->RmtJumpPostcode = 0;
#endif
  //
  // Convert BITx values to x instead for Current* variables which contain the current PFCT.
  //
  ConvertPfctFromBitXToX (MrcData);
  //
  // Execute all steps for the given Stage.
  //
#if defined(SIM) || defined(JTAG)
  MrcData->Pcd.InitPostCode=0x4;
  MrcData->Pcd.PostCodeState=PC_INIT;
#endif
  switch (Action) {
    case EntireMmrc:
      j = InitTasks;
      if (Parameters != NULL) {
        if (Parameters->ChannelValid) {
          j->Channel = Parameters->Channel;
        }
      }
      for ( ; j->Function != NULL; j++) {
#if RMT_JUMP_POSTCODES
        j+=MrcData->RmtJumpPostcode;
        MrcData->RmtJumpPostcode = 0;
#endif
        if ((MrcData->BootMode & j->BootMode) != 0) {
          MrcData->PostCode = j->PostCode;
          if (j->PostCode == 0x30) {
            j->PostCode = j->PostCode;
          }
#if defined(SIM) || defined(JTAG)
          if (MrcData->Pcd.PostCodeState == PC_END)
          {
            return MMRC_SUCCESS;
          }
          else if (MrcData->Pcd.PostCodeState == PC_GOTOEND)
          {
          }
          else if (MrcData->Pcd.PostCodeState == PC_INIT)
          {
            if (MrcData->Pcd.InitPostCode == j->PostCode)
            {
              if (MrcData->Pcd.StartPostCodeEnabled == TRUE)
              {
                MrcData->Pcd.PostCodeState = PC_WAITFOR_START;
              }
              else if (MrcData->Pcd.StopPostCodeEnabled == TRUE)
              {
                MrcData->Pcd.PostCodeState = PC_WAITFOR_END;
              }
              else
              {
                MrcData->Pcd.PostCodeState = PC_GOTOEND;
              }
            }
          }
          else if (MrcData->Pcd.PostCodeState == PC_WAITFOR_START)
          {
            if (MrcData->Pcd.StartPostCode == j->PostCode)
            {
              if (MrcData->Pcd.StopPostCodeEnabled == TRUE)
              {
                MrcData->Pcd.PostCodeState = PC_WAITFOR_END;
              }
              else
              {
                MrcData->Pcd.PostCodeState = PC_GOTOEND;
              }
            }
            else
            {
              MmrcDebugPrint ((MMRC_DBG_MIN, "Skipping PC %04x while waiting for Start PC %04x\n", j->PostCode, MrcData->Pcd.StartPostCode));
              continue;
            }
          }
          else if (MrcData->Pcd.PostCodeState == PC_WAITFOR_END)
          {
            if (MrcData->Pcd.StopPostCode == j->PostCode)
            {
              MrcData->Pcd.PostCodeState = PC_END;
            }
          }
          else
          {
          }
#endif
#ifndef JTAG
          if ((MrcData->PlatformID == 0) && (MrcData->BootMode == A0)) {
            if (j->PostCode == 0x25) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "\n--------- Run SUS_itp.punit_tap(0) - Note: param = (0) --------\n"));
              IoOut16(0x84, 0xDD);
              CpuDeadLoop();
            }
            if (j->PostCode == 0x41) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "\n--------- Run SUS_itp.punit_tap(1) - Note: param = (1) --------\n"));
              IoOut16(0x84, 0xEE);
              CpuDeadLoop();
            }
          }
#endif
          HandleCheckpoint (MrcData, j->PostCode);
          //
          // Mark the channels which will be executing this routine. Some channels might skip execution
          // based on their PFCT.
          //
          CreateChannelMask (MrcData, j);
          if ((j->Channel == ChAll) || (j->Channel == ChAllF) || (j->Channel == ChAllP && !MrcData->EnableParallelTraining)) {
            for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
              if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel) {
                  continue;
              }

              if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled && (j->Channel != ChAllF)) {
                continue;
              }

              if (j->Channel == ChAllF) {
                MmrcDebugPrint ((MMRC_DBG_MIN, "Forcing run on disabled channels\n"));
                MrcData->ForceRunOnDisabledChannels = TRUE;
              } else {
                MmrcDebugPrint ((MMRC_DBG_MIN, "Won't run on disabled channels\n"));
                MrcData->ForceRunOnDisabledChannels = FALSE;
              }

              if (j->CapsuleStartIndex == EXTERNAL) {
                PrintTaskNameType (j);
              }
              MrcData->ExecuteThisRoutineInParallel = FALSE;
              Status = (*(j->Function)) (MrcData, j->CapsuleStartIndex, j->StringIndex, Channel);
              if (Status != MMRC_SUCCESS) {
                return Status;
              }
            }
          } else if (j->Channel == ChAllP) {
            if (j->CapsuleStartIndex == EXTERNAL) {
              PrintTaskNameType (j);
            }
            MrcData->ExecuteThisRoutineInParallel = TRUE;
            MrcData->ForceRunOnDisabledChannels = FALSE;
            Status = (*(j->Function)) (MrcData, j->CapsuleStartIndex, j->StringIndex, 0);
            if (Status != MMRC_SUCCESS) {
              return Status;
            }
          } else if (j->Channel == ChNone) {
            if (j->CapsuleStartIndex == EXTERNAL) {
              PrintTaskNameType (j);
            }
            ExecuteOnAnyChannel = FALSE;
            MrcData->ForceRunOnDisabledChannels = FALSE;
            for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
              ExecuteOnAnyChannel = (ExecuteOnAnyChannel || MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel);
            }
            if (!ExecuteOnAnyChannel){
              continue;
            }
            //
            // ChNone - we use Channel 0 in this case as a dummy placeholder.
            //
            MrcData->NvData.MrcParamsSaveRestore.Channel[0].ExecuteOnThisChannel = TRUE;
            MrcData->ExecuteThisRoutineInParallel = FALSE;
            Status = (*(j->Function)) (MrcData, j->CapsuleStartIndex, j->StringIndex, 0);
            if (Status != MMRC_SUCCESS) {
              return Status;
            }
          } else {
            if (j->CapsuleStartIndex == EXTERNAL) {
              PrintTaskNameType (j);
            }
            //
            // We're here because a specific channel has been requested.
            //
            MrcData->ForceRunOnDisabledChannels = FALSE;
            if (j->Channel < MAX_CHANNELS) {
              if (MrcData->NvData.MrcParamsSaveRestore.Channel[j->Channel].Enabled && MrcData->NvData.MrcParamsSaveRestore.Channel[j->Channel].ExecuteOnThisChannel) {
                MrcData->ExecuteThisRoutineInParallel = FALSE;
                Status = (*(j->Function)) (MrcData, j->CapsuleStartIndex, j->StringIndex, j->Channel);
                if (Status != MMRC_SUCCESS) {
                  return Status;
                }
              }
            }
          }
        }
      }
      return Status;

  case ExecuteSpecificFunction:
    //
    // This is used for when a caller outside the MRC (like EV scripts) call into
    // the MMRC to execute a particular function (such as RMT). Always execute, regardless
    // of boot path.
    //
    // If the caller passed in a channel on which to operate, execute on that channel.
    // Otherwise, call routine once without a channel specified.
    //
    if (!Parameters->ChannelValid) {
      Parameters->Channel = ChNone;
    }
    if (Parameters->Function != NULL) {
      return (*(Parameters->Function)) (MrcData, Parameters->Channel);
    } else {
      MmrcDebugPrint ((MMRC_DBG_MIN, "NULL function\n"));
    }

    break;

  default:
    MmrcDebugPrint ((MMRC_DBG_MIN, "Invalid stage (%d) passed to MMRC\n", Action));
    break;
  }

  return Status;
}


VOID *
MmrcMemset (
  void      *dst,
  int       value,
  UINT32    cnt
  )
{
  volatile UINT8 *dst8 = (UINT8 *) dst;
  while (cnt--) {
    *dst8 = (UINT8) value;
    dst8++;
  }
  return dst;
}

#if CAR
VOID *
 memset (
  void      *dst,
  int       value,
  size_t    cnt
  )
{
 return MmrcMemset (dst, value, cnt);
}
#endif

UINT32
StringLength (
  char *String
  )
{
  UINT32 i;
  i = 0;
  while ((String[i++] != 0) && (i < 50));
  return i;
}

#if DUMP_REGISTER_NAMES
UINT32
GetRegisterNumber (
  UINT8  Box,
  UINT32 Offset
  )
{
  MMRC_DATA *MrcData;
  UINT32 RegisterNumber;
  MrcData = GetMrcHostStructureAddress();
  //
  // Grab the first register string for this boxtype.
  //
  RegisterNumber = InstancePortMap[Box].StringIndex;
  if (RegisterNumber == 0xFFFFFFFF) {
    return RegisterNumber;
  }
  //
  // Loop through all registers for this box....
  //
  while (Registers[RegisterNumber].RegInfo != NULL) {
    if (Registers[RegisterNumber].RegInfo->Offset == Offset) {
      break;
    }
    RegisterNumber++;
  }
  return RegisterNumber;
}
#endif // DUMP_REGISTER_NAMES

/**
  This routine dumps all registers in the MMRC spreadsheet.
  On the cover sheet of the SS, the user can control the debug
  level output:

  DUMP_REGISTER_NAMES = 1 means all register names, ports,
  offsets, values, and default values will be printed.

  DUMP_REGISTER_FIELDS = 1 means all register field names,
  values, default values, and access types will be printed.

  DUMP_REGISTER_DESCRIPTIONS = 1 will display the description
  for each register field.

  If DUMP_REGISTER_NAMES is set to 0, the other enables are
  don't-cares as this is the global enable for this routine.

  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
  DumpAllRegisters (
  )
{
#if DUMP_REGISTER_NAMES
  UINT32 RegisterNumber;
  UINT32 i;
  UINT8  Box;
  UINT8  Channel;
  UINT8  Instance;
  UINT8  Port;
  UINT8  Bus;
  UINT8  Device;
  UINT8  Function;
  UINT32 Junk;
  UINT32 BaseOffset;
  REGISTER_ACCESS Register;
  UINTX  Value;
  UINT8  TotalChannels;
  UINT8  TotalInstances;
#if DUMP_REGISTER_FIELDS
  UINT8  j;
  UINTX  UpperMask;
  UINTX  LowerMask;
#endif
  UINT8  MaxBit;
  MMRC_DATA *MrcData;

#if USE_64_BIT_VARIABLES
  MaxBit = 64;
#else
  MaxBit = 32;
#endif

  MrcData = GetMrcHostStructureAddress ();
  for (Box = 0; Box < MAX_BOXES; Box++) {
    //
    // If there are no registers for this box, skip the printing.
    //
    if (InstancePortMap[Box].StringIndex == 0xFFFFFFFF) {
      continue;
    }
    //
    // If this box only has 1 sideband access command, assume it's write-only so we can't
    // read it. This is for things like opcode 0x68 in the DUNIT where you cannot read.
    //
    if (InstancePortMap[Box].AccessMethod == eSB && InstancePortMap[Box].ReadOperation == InstancePortMap[Box].WriteOperation) {
      continue;
    }
    if (InstancePortMap[Box].Instances == 0) {
      TotalInstances = InstancePortMap[Box].TotalInstances;
      TotalChannels  = 1;
    } else {
      TotalInstances = InstancePortMap[Box].Instances;
      TotalChannels  = (InstancePortMap[Box].TotalInstances / InstancePortMap[Box].Instances);
    }
    for (Channel = 0; Channel < TotalChannels; Channel++) {
      for (Instance = 0; Instance < TotalInstances; Instance++) {
        if (GetRegisterAccessInfo (Box, Channel, Instance, &Junk, &Port, &BaseOffset, &Bus, &Device, &Function, ModeRead) == NoError) {
          //
          // Print the header for each instance.
          //
#if CAPSULESTRINGS
          MmrcDebugPrint ((MMRC_DBG_MIN, "%s%d\n", UnitStrings[Box], (Channel * InstancePortMap[Box].Instances) + Instance));
#endif
          MmrcDebugPrint ((MMRC_DBG_MIN, "Name"));
          for (i = 0; i < MAX_REGISTER_NAME_LENGTH + 1; i++) {
            MmrcDebugPrint ((MMRC_DBG_MIN, " "));
          }
#if DUMP_REGISTER_FIELDS
          MmrcDebugPrint ((MMRC_DBG_MIN, "Port     Offset   Bits               Value            Default  Access\n"));
          for (i = 0; i < MAX_REGISTER_NAME_LENGTH + 74; i++) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "_"));
          }
#else
          MmrcDebugPrint ((MMRC_DBG_MIN, "Port     Offset   Bits               Value            Default\n"));
          for (i = 0; i < MAX_REGISTER_NAME_LENGTH + 66; i++) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "_"));
          }
#endif
          MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
          //
          // Grab the first register string for this boxtype.
          //
          RegisterNumber = InstancePortMap[Box].StringIndex;
          //
          // Loop through all registers for this box....
          //
          while (Registers[RegisterNumber].RegInfo != NULL) {
            //
            // Display the register name, padding with spaces to align everything. Typically you could
            // use printf ("%-15s", string), but this doesn't work in the MiniBIOS, so pad manually here.
            //
            MmrcDebugPrint ((MMRC_DBG_MIN, "%s", Registers[RegisterNumber].RegInfo->RegisterName));
            for (i = StringLength (Registers[RegisterNumber].RegInfo->RegisterName); i <= MAX_REGISTER_NAME_LENGTH + 2; i++) {
              MmrcDebugPrint ((MMRC_DBG_MIN, " "));
            }
            //
            // Display port, offset, and value.
            //
            Register.Offset = Registers[RegisterNumber].RegInfo->Offset;
            Register.Mask   = Registers[RegisterNumber].RegInfo->Mask;
            Value = MemRegRead (Box, Channel, Instance, Register);
            if (InstancePortMap[Box].AccessMethod == ePCI) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "%02X/%02X/%01X ", Bus, Device, Function));
            } else if (InstancePortMap[Box].AccessMethod == eBAR) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "        "));
            } else {
              MmrcDebugPrint ((MMRC_DBG_MIN, "   0x%02X ", Port));
            }
#if USE_64_BIT_VARIABLES
            MmrcDebugPrint ((MMRC_DBG_MIN, "0x%08X [xx:xx] 0x%08X%08X 0x%08X%08X\n",
              Registers[RegisterNumber].RegInfo->Offset + BaseOffset,
              (UINT32) (Value >> 32),
              (UINT32) Value,
              (UINT32) (Registers[RegisterNumber].RegInfo->DefaultValue >> 32),
              (UINT32) Registers[RegisterNumber].RegInfo->DefaultValue
              ));
#else
            MmrcDebugPrint ((MMRC_DBG_MIN, "0x%08X [xx:xx] 0x%08X%08X 0x%08X%08X\n",
              Registers[RegisterNumber].RegInfo->Offset + BaseOffset,
              0,
              (UINT32) Value,
              0,
              (UINT32) Registers[RegisterNumber].RegInfo->DefaultValue
              ));
#endif
#if DUMP_REGISTER_FIELDS
            //
            // Display all register fields with proper start/end bits and values.
            //
            for (i = 0; i < Registers[RegisterNumber].RegInfo->NumFields; i++) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "  %s", Registers[RegisterNumber].RegFields[i].FieldName));
              for (j = (UINT8) StringLength (Registers[RegisterNumber].RegFields[i].FieldName); j <= MAX_REGISTER_NAME_LENGTH; j++) {
                MmrcDebugPrint ((MMRC_DBG_MIN, " "));
              }
              //
              // Calculate the mask based on start and end bits.
              //
              if (Registers[RegisterNumber].RegFields[i].EndBit == (MaxBit - 1)) {
#if USE_64_BIT_VARIABLES
                UpperMask = 0xFFFFFFFFFFFFFFFF;
#else
                UpperMask = 0xFFFFFFFF;
#endif
              } else {
                UpperMask = (((UINTX)1 << (Registers[RegisterNumber].RegFields[i].EndBit + 1)) - 1);
              }
              LowerMask = (((UINTX)1 << Registers[RegisterNumber].RegFields[i].StartBit) - 1);
              Register.Mask = UpperMask - LowerMask;
              //
              // Display port, offset, start/end bits, value, and default value.
              //
              Register.Offset = Registers[RegisterNumber].RegInfo->Offset;
              Register.ShiftBit = Registers[RegisterNumber].RegFields[i].StartBit;
              Value = MemFieldRead (Box, Channel, Instance, Register);
              if (InstancePortMap[Box].AccessMethod == ePCI) {
                MmrcDebugPrint ((MMRC_DBG_MIN, "%02X/%02X/%01X ", Bus, Device, Function));
              } else if (InstancePortMap[Box].AccessMethod == eBAR) {
                MmrcDebugPrint ((MMRC_DBG_MIN, "        "));
              } else {
                MmrcDebugPrint ((MMRC_DBG_MIN, "   0x%02X ", Port));
              }
#if USE_64_BIT_VARIABLES
              MmrcDebugPrint ((MMRC_DBG_MIN, "0x%08X [%02d:%02d] 0x%08X%08X 0x%08X%08X",
                Registers[RegisterNumber].RegInfo->Offset + BaseOffset,
                Registers[RegisterNumber].RegFields[i].EndBit,
                Registers[RegisterNumber].RegFields[i].StartBit,
                (UINT32) (Value >> 32),
                (UINT32) Value,
                (UINT32) (Registers[RegisterNumber].RegFields[i].DefaultFieldValue >> 32),
                (UINT32) Registers[RegisterNumber].RegFields[i].DefaultFieldValue
                ));
#else
              MmrcDebugPrint ((MMRC_DBG_MIN, "0x%08X [%02d:%02d] 0x%08X%08X 0x%08X%08X",
                Registers[RegisterNumber].RegInfo->Offset + BaseOffset,
                Registers[RegisterNumber].RegFields[i].EndBit,
                Registers[RegisterNumber].RegFields[i].StartBit,
                0,
                (UINT32) Value,
                0,
                (UINT32) Registers[RegisterNumber].RegFields[i].DefaultFieldValue
                ));
#endif
              //
              // Display the access type (RW, RO, etc)
              //
              MmrcDebugPrint ((MMRC_DBG_MIN, "  %s\n", Registers[RegisterNumber].RegFields[i].AccessType));
              //
              // Optionally display the register field description (this takes up a LOT of space for
              // the strings so only use this mode in an environment that isn't space limited such
              // as RTL simulation or JTAG mode when running from a host machine.
              //
#if DUMP_REGISTER_DESCRIPTIONS
              MmrcDebugPrint ((MMRC_DBG_MIN, "%s\n", Registers[RegisterNumber].RegFields[i].Description));
#endif
            }
#endif
            RegisterNumber++;
          }

          MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
        }
      }
    }
  }
#endif
  return MMRC_SUCCESS;
}

#if FAULTY_PART_TRACKING
/**
  Multiplication

  @param[in]       Multiplicand
  @param[in]       Multiplier
  @retval          Multiplication Result
**/
UINT64
InternalMathMultU64x32 (
  IN      UINT64                    Multiplicand,
  IN      UINT32                    Multiplier
  )
{
  _asm {
    mov     ecx, Multiplier
    mov     eax, ecx
    imul    ecx, dword ptr [Multiplicand + 4]  // overflow not detectable
    mul     dword ptr [Multiplicand + 0]
    add     edx, ecx
  }
}

/**
  Division

  @param[in]       Dividend
  @param[in]       Divisor
  @param[out]      Remainder
  @retval          Division Result
**/
UINT64
InternalMathDivRemU64x32 (
  IN      UINT64                    Dividend,
  IN      UINT32                    Divisor,
  OUT     UINT32                    *Remainder
  )
{
  _asm {
    mov     ecx, Divisor
    mov     eax, dword ptr [Dividend + 4]
    xor     edx, edx
    div     ecx
    push    eax
    mov     eax, dword ptr [Dividend + 0]
    div     ecx
    mov     ecx, Remainder
    jecxz   RemainderNull                      // abandon remainder if Remainder == NULL
    mov     [ecx], edx
RemainderNull:
    pop     edx
  }
}

/**
  Count the number of bits set on a 32 bit segister with 6 arihmetic operations

  @param[in]       Value     32 bit value to operate on
  @retval          The number of bits set on the 32 bit value
**/
UINT8
CountOnes (
  UINT32  Value
  )
{
  UINT32 cnt;
  UINT32 cnt2;
  cnt = 0;
  cnt2 = 0;
  //
  //cnt =  ((Value & 0xfff) * 0x1001001001001L & 0x84210842108421L) % 0x1f;
  //
  InternalMathDivRemU64x32((InternalMathMultU64x32(LOOKUP_TABLE_1,(Value & MASK_LOWER_12BIT)) & LOOKUP_TABLE_2), MODULE31, &cnt);
  //
  //cnt += (((Value & 0xfff000) >> 12) * 0x1001001001001L & 0x84210842108421L) % 0x1f;
  //
  InternalMathDivRemU64x32((InternalMathMultU64x32(LOOKUP_TABLE_1,((Value & (MASK_LOWER_12BIT << 12)) >> 12)) & LOOKUP_TABLE_2), MODULE31, &cnt2);
  cnt += cnt2;
  //
  //cnt += ((Value >> 24) * 0x1001001001001L & 0x84210842108421L) % 0x1f;
  //
  InternalMathDivRemU64x32((InternalMathMultU64x32(LOOKUP_TABLE_1,(Value >> 24)) & LOOKUP_TABLE_2), MODULE31, &cnt2);
  cnt += cnt2;
  return (UINT8) cnt;
}

/**
  Initializes the error tracking variables to the maximum value.
  This is used before each CPGC based training

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       FirstTraining
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
FaultyPartTrackInit (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        BOOLEAN       FirstTraining
  )
{
  UINT8 Rank;
  Rank = 0;

  for (Channel; Channel < MAX_CHANNELS; Channel++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      //
      // Initialize the error tracking with the max value and update with less than max value
      //
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte0to3 = (UINT32)(~(0));
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte4to7 = (UINT32)(~(0));
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ByteEcc = (UINT8)(~(0));
      //
      // Initialize the cumulative variables for historic error tracling across all trainings
      //
      if (FirstTraining) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].DataBitFailCumulativeHigh = 0;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].DataBitFailCumulativeLow = 0;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].EccBitFailCumulative = 0;
      }
    }
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  return MMRC_SUCCESS;
}

/**
  Process the data from the CPGC test in order to print the
  device and the bits that failed for debug porposes.
  This function supports handling of x8 and x4 devices if a
  different width needs to add support for it.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]   FaultyPartRankLow   Bits of bytes 0 to 3
  @param[in]   FaultyPartRankHi    Bits of bytelanes 4 to 7
  @param[in]   FaultyPartRankEcc   Bits of the ECC bytelane.
  @retval      MMRC_SUCCESS
**/
STATIC
MMRC_STATUS
FaultyPartProcessDeviceData (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT32        FaultyPartRankLow,
  IN        UINT32        FaultyPartRankHi,
  IN        UINT8         FaultyPartRankEcc
  )
{
  UINT8  StrobeBitLanes;
  UINT8  Strobe;
  UINT8  StrobeMask;
  UINT8  StrobeBits;
  UINT8  LowCrsLimit;
  UINT8  HighCsrLimit;
  UINT8  EccCsrLimit;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank] <= MAX_STROBE_X8_DEVICES) {
    StrobeMask = BYTE_MASK;
    StrobeBits = CountOnes(BYTE_MASK);
    LowCrsLimit = 4;
    HighCsrLimit = 3 ;
    EccCsrLimit = 8;
  } else {
    StrobeMask = NIBBLE_MASK;
    StrobeBits = CountOnes(NIBBLE_MASK);
    LowCrsLimit = 8;
    HighCsrLimit = 7 ;
    EccCsrLimit = 17;
  }

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    StrobeBitLanes = 0;
    if ((Strobe < LowCrsLimit)) {
      StrobeBitLanes = ((FaultyPartRankLow >> (StrobeBits * Strobe)) & StrobeMask);
    }
    if ((Strobe > HighCsrLimit) && (Strobe < EccCsrLimit)) {
      StrobeBitLanes = ((FaultyPartRankHi >> (StrobeBits * (Strobe - LowCrsLimit))) & StrobeMask);
    }
    if ((Strobe >= EccCsrLimit)) {
      StrobeBitLanes = ((FaultyPartRankEcc >> (StrobeBits * (Strobe - EccCsrLimit))) & StrobeMask);
    }
    if (StrobeBitLanes != 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "\tDevice:%d has %d bit(s) failing. Strobe's value is:0x%x \n",
        Strobe, CountOnes(StrobeBitLanes), StrobeBitLanes
        ));
    }
  }
  return MMRC_SUCCESS;
}

/**
  Programs Bunit to enable/disable the second channel

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
**/
MMRC_STATUS
FaultyPartDisableChannel (
  IN  OUT   MMRC_DATA             *MrcData,
  IN        UINT8                 Channel
  )
{
#if PUNIT_MAILBOX == 1
  //
  //  Currently we can only disable channel 1
  //
  MmrcExecuteTask (MrcData, DisableChannel, NO_PRINT, Channel);
  //
  // The reset will detect dimms again and try to enable the channel again and we will be on a loop here, is the Bmisc sticky??
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "Disabling Channel Faulty Part\n"));
#ifdef FSP_FLAG
  FspApiReturnStatusReset (FSP_STATUS_RESET_REQUIRED_COLD);
#endif
  IoOut8 (0xCF9, 0x0);
  IoOut8 (0xCF9, 0xE);
#ifndef SIM
  _asm hlt
#endif

#endif
  return MMRC_SUCCESS;
}

/**
  Process the faulty part data gathered by the CPGC based
  training

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
**/
MMRC_STATUS
FaultyPartProcessTrainingResult (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel
  )
{
  UINT32 FaultyPartRankLow;
  UINT32 FaultyPartRankHi;
  UINT8  FaultyPartRankEcc;
  UINT8  Rank;
  //
  // Faulty part tracking per training step processing
  //
  if (MrcData->FaultyPartTracking && MrcData->RestorePath == FALSE) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        //
        // Failures on the current training and previews trainings
        //
        FaultyPartRankLow = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte0to3 | MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].DataBitFailCumulativeLow;
        FaultyPartRankHi  = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte4to7 | MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].DataBitFailCumulativeHigh;
        FaultyPartRankEcc = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ByteEcc | MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].EccBitFailCumulative;

        if (FaultyPartRankLow != 0 || FaultyPartRankHi != 0 || FaultyPartRankEcc != 0) {
          //
          // Acumulate failures through out all trainings
          //
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].DataBitFailCumulativeLow = FaultyPartRankLow;
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].DataBitFailCumulativeHigh = FaultyPartRankHi;
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].EccBitFailCumulative  = FaultyPartRankEcc;

          if (((CountOnes(FaultyPartRankLow) + CountOnes(FaultyPartRankHi)) > FAULTY_PART_CORRECTABLE) || (FaultyPartRankEcc != 0)) {
            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ErrorType = FAULTY_PART_UNCORRECTABLE;
          } else {
            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ErrorType = FAULTY_PART_CORRECTABLE;
          }
          //
          // Display the error on serial log
          //
          if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ErrorType == FAULTY_PART_UNCORRECTABLE) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "FAULTY_PARTS_TRACKING -UNCORRECTABLE- Channel:%d Rank:%d \n", Channel, Rank));
            //
            // if there is uncorrectable error on channel 1 disable it else halt because there is an uncorrectable on channel 0
            //
            if (Channel != 0) {
              FaultyPartDisableChannel(MrcData, Channel);
            } else {
              FaultyPartProcessDeviceData (MrcData, Channel, Rank, FaultyPartRankLow, FaultyPartRankHi, FaultyPartRankEcc);
              return MrcErrFaultyPartUncorrectable;
            }
          } else {
            MmrcDebugPrint ((MMRC_DBG_MIN, "FAULTY_PARTS_TRACKING -CORRECTABLE- Channel:%d Rank:%d \n", Channel, Rank));
            if (MrcData->FaultyPartContinueOnCorrectable  == FALSE) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "FAULTY_PARTS_TRACKING -CORRECTABLE- Halt on correctable \n"));
              FaultyPartProcessDeviceData (MrcData, Channel, Rank, FaultyPartRankLow, FaultyPartRankHi, FaultyPartRankEcc);
              return MrcErrFaultyPartCorrectable;
            }
          }
          FaultyPartProcessDeviceData (MrcData, Channel, Rank, FaultyPartRankLow, FaultyPartRankHi, FaultyPartRankEcc);
          if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled == 0) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "FAULTY_PARTS_TRACKING -CORRECTABLE promoted to UNCORRECTABLE-"));
            MmrcDebugPrint ((MMRC_DBG_MIN, "Channel %d will not be trained from this point on\n", Channel));
          }
        }  //any failure found
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n\r"));
  }
  return MMRC_SUCCESS;
}

/**
  This API will report out the error Status for byte level,
  bit level and ECC lane.
  If the check low/high Bits variables are non-NULL, the bit field
  results are copied into the pointer location.
  Similarly, if the Byte Lane or ECC or non-null,
  these results are copied into their respective locations.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Channel       Current Channel being examined.
  @param[in]       Rank          Current Rank being examined.
  @param[in]   LowBitsErrStat    Pointer for Low bits level for error reporting
  @param[in]   HighBitsErrStat   Pointer for High bits level for error reporting
  @param[in]   ByteLanesErrStat  Pointer for ECC level of error reporting
  @retval      MMRC_SUCCESS
**/
STATIC
MMRC_STATUS
CpgcSCheckErrors (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN  OUT   UINT32        *LowBitsErrStat,
  IN  OUT   UINT32        *HighBitsErrStat,
  IN  OUT   UINT32        *ByteLanesErrStat
  )
{
  UINT32 Cpgc_Err_Stat_Lo;
  UINT32 Cpgc_Err_Stat_Hi;
  UINT8  Bytegrp_err_stat;
  UINT8  Ecc_lane_err_stat;
  UINT8  Strobe;
  UINT8  StrobeBitLanes;
  UINT8  StrobeBitLanes2;
  UINT8  StrobeMask;
  UINT8  StrobeBits;
  UINT8  LowCrsLimit;
  UINT8  HighCsrLimit;
  UINT8  EccCsrLimit;

  Strobe = 0;
  StrobeBitLanes = 0;
  StrobeBitLanes2 = 0;
  StrobeMask = 0;
  StrobeBits = 0;
  LowCrsLimit = 0;
  HighCsrLimit = 0;
  EccCsrLimit = 0;

  if (MrcData->FaultyPartTracking) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank] <= MAX_STROBE_X8_DEVICES) {
      StrobeMask = BYTE_MASK;
      StrobeBits = 8;
      LowCrsLimit = 4;
      HighCsrLimit = 3;
      EccCsrLimit = 8;
    } else {
      StrobeMask = NIBBLE_MASK;
      StrobeBits = 4;
      LowCrsLimit = 8;
      HighCsrLimit = 7;
      EccCsrLimit = 17;
    }
  }

  //
  // Read CPGC Byte Group 0-3 Lane Error Status register
  //
  Cpgc_Err_Stat_Lo = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT03];
  //
  // Read CPGC Byte Group 4-7 Lane Error Status register
  //
  Cpgc_Err_Stat_Hi = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT47];
  //
  // Read CPGC Extended Error Status register
  //
  Bytegrp_err_stat = (UINT8) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
  Ecc_lane_err_stat = (UINT8) MrcData->DynamicVars[Channel][ECC_ERR_STAT];
  //
  // We will keep this debug line
  // MmrcDebugPrint ((MMRC_DBG_VERBOSE, "Rank: %d Debug FP Err_Lo:0x%x Err_hi:0x%x Bytegrp:0x%x Ecc:0x%x\n",
  //  Rank, Cpgc_Err_Stat_Lo, Cpgc_Err_Stat_Hi, Bytegrp_err_stat, Ecc_lane_err_stat
  //  ));

  //
  // Copy Error Status Value to each respective pointer
  //
  if (LowBitsErrStat != NULL) {
    *LowBitsErrStat = Cpgc_Err_Stat_Lo;
  }
  if (HighBitsErrStat != NULL) {
    *HighBitsErrStat = Cpgc_Err_Stat_Hi;
  }
  if (ByteLanesErrStat != NULL) {
    *ByteLanesErrStat = (UINT8) Bytegrp_err_stat;
    *ByteLanesErrStat |= (Ecc_lane_err_stat << 8);
  }

  //
  // Per training find the least number of bits failures. Ideal case would be 0, correctable
  // would be single bit error on data and uncorrectable more than single bit or ECC bytelane error
  //
  if (MrcData->FaultyPartTracking) {

    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      StrobeBitLanes = 0;
      StrobeBitLanes2 = 0;
      if ((Strobe < LowCrsLimit)) {
        StrobeBitLanes = ((Cpgc_Err_Stat_Lo >> (StrobeBits * Strobe)) & StrobeMask);
        StrobeBitLanes2 = ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte0to3 >> (StrobeBits * Strobe)) & StrobeMask);
        //
        // If what we got from status csr is smaller than what we previously saved we update the saved value
        //
        if (CountOnes (StrobeBitLanes) < CountOnes (StrobeBitLanes2)) {
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte0to3 &= (UINT32) ~(StrobeMask << (StrobeBits * Strobe));
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte0to3 |= (UINT32) ((StrobeBitLanes << (StrobeBits * Strobe)) & (StrobeMask << (StrobeBits * Strobe)));
        }
      }
      if ((Strobe > HighCsrLimit) && (Strobe < EccCsrLimit)) {
        StrobeBitLanes = ((Cpgc_Err_Stat_Hi >> (StrobeBits * (Strobe - LowCrsLimit))) & StrobeMask);
        StrobeBitLanes2 = ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte4to7 >> (StrobeBits * (Strobe - LowCrsLimit))) & StrobeMask);
        if (CountOnes (StrobeBitLanes) < CountOnes (StrobeBitLanes2)) {
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte4to7 &= (UINT32) ~(StrobeMask << (StrobeBits * (Strobe - LowCrsLimit)));
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].Byte4to7 |= (UINT32) ((StrobeBitLanes << (StrobeBits * (Strobe - LowCrsLimit))) & (StrobeMask << (StrobeBits * (Strobe - LowCrsLimit))));
        }
      }
      if ((Strobe >= EccCsrLimit)) {
        StrobeBitLanes = ((Ecc_lane_err_stat >> (StrobeBits * (Strobe - EccCsrLimit))) & StrobeMask);
        StrobeBitLanes2 = (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ByteEcc & StrobeMask);
        if (CountOnes (StrobeBitLanes) < CountOnes (StrobeBitLanes2)) {
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ByteEcc &= (UINT32) ~(StrobeMask << (StrobeBits * (Strobe - EccCsrLimit)));
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].FaultyPart[Rank].ByteEcc |= (UINT32) ((StrobeBitLanes << (StrobeBits * Strobe)) & (StrobeMask << (StrobeBits * Strobe)));
        }
      }
    } // for strobe
    //
    // If there is only single bit error on only one device and ECC is enabled (Setup options)
    // and there is no error on the ECC bytelane then ignore this correctable error
    //
    if (((CountOnes (Cpgc_Err_Stat_Lo) + CountOnes(Cpgc_Err_Stat_Hi)) == 1) && (CountOnes (Ecc_lane_err_stat) == 0)
          && (CountOnes (Bytegrp_err_stat) == 1) && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {

      Bytegrp_err_stat = 0;
      if (ByteLanesErrStat != NULL) {
        *ByteLanesErrStat = (UINT8) Bytegrp_err_stat;
        *ByteLanesErrStat |= (Ecc_lane_err_stat << 8);
      }
    }
  }
  return MMRC_SUCCESS;
}

#endif // FAULTY_PART_TRACKING

VOID
GetMaxDq (
  IN         MMRC_DATA    *MrcData
  )
{
  UINT8 Channel;
  UINT8 Dimm;
  UINT8 Rank;

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    for (Dimm = 0; Dimm < MAX_DIMMS; Dimm++) {
      if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[Dimm]) {
        continue;
      }
      for (Rank = 0; Rank < MAX_RANKS_PER_DIMM; Rank++) {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[(2 * Dimm) + Rank] == 0) {
          continue;
        }
        switch (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[Dimm]) {
        case 0:
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[(2 * Dimm) + Rank] = 18;
          break;
        case 1:
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[(2 * Dimm) + Rank] = 9;
          break;
        case 2:
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[(2 * Dimm) + Rank] = 4;
          break;
        }
      } // Rank
    } // Dimm
  } // Channel
}

BOOLEAN
ReExecuteSample (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT8             ChannelBackup,
  IN      UINT8             Rank,
  IN      UINT16            Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2],
  IN      UINT16            Low[MAX_CHANNELS][MAX_STROBES],
  IN      UINT16            High[MAX_CHANNELS][MAX_STROBES],
  IN      UINT16            Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2]
)
{
  UINT8   Channel;
  UINT8   Strobe;
  BOOLEAN AllStrobesPassed;

  AllStrobesPassed = TRUE;

  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if ((GET_ABSOLUTE (Results[Channel][0][Strobe][0][HIGH], Results[Channel][0][Strobe][0][LOW])) > ((GetQtrClk (MrcData, Channel) * MIN_PULSE_WIDTH) / 4)) {
        Dim1StartPoint[Channel][0][Strobe][LOW]  = Results[Channel][0][Strobe][0][LOW];
        Dim1StartPoint[Channel][0][Strobe][HIGH] = Results[Channel][0][Strobe][0][HIGH];
      } else {
        AllStrobesPassed = FALSE;
        Dim1StartPoint[Channel][0][Strobe][LOW]  = Dim1StartPoint[Channel][0][Strobe][LOW] + GetQtrClk (MrcData, Channel);
        Dim1StartPoint[Channel][0][Strobe][HIGH] = Dim1StartPoint[Channel][0][Strobe][LOW] + GetQtrClk (MrcData, Channel);
      }
      Low[Channel][Strobe]  = 0;
      High[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][LOW] + GetOneClk (MrcData, Channel);
    } // Strobe loop ...
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  return AllStrobesPassed;
}

/**
  Set the Drams to enable Read Pream Training and MPR mode needed for  DDR
  technologies, can add in this fucntion their specific receive
  enable Dram side (MRS operation) configurations

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Entry           Parameter to select Entry or Exit hook to execute.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ReceiveEnableDramEntryExit (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT8             Channel,
  IN      UINT8             Rank,
  IN      UINT8             Entry
  )
{
#if RECEIVE_ENABLE == 1 && TRAINING_ALGOS == 1
#if DDR4_SUPPORT
  DramInitDDR4EMR3  Mrs3Command;
  DramInitDDR4EMR4  Mrs4Command;
#endif

  if (Entry == MMRC_ENTRY) {
#if DDR4_SUPPORT
    //
    // Set the Drams in read preamble training / MPR mode
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr4) {
      PrechargeAll (MrcData, Channel, Rank);
      Mrs3Command.Data = ReadMrsCommand (MrcData, Channel, Rank, 3);
      Mrs4Command.Data = ReadMrsCommand (MrcData, Channel, Rank, 4);
      Mrs3Command.Bits.MPR_Operation = 1;
      Mrs4Command.Bits.ReadPreamMode = 1;
      WriteMrsCommand (MrcData, Channel, Rank, Mrs3Command.Data);
      WriteMrsCommand (MrcData, Channel, Rank, Mrs4Command.Data);
    }
#endif // DDR4_SUPPORT

#if LPDDR3_SUPPORT
    //
    // Issue a Precharge All, keeping BL in 32-bit mode.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
      PrechargeAll (MrcData, Channel, Rank);
    }
#endif // LPDDR3_SUPPORT

#if DDR3_SUPPORT
    //
    // Issue a Precharge All, keeping BL in 32-bit mode.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
      PrechargeAll (MrcData, Channel, Rank);
      //
      // Enter MPR mode by issueing a MRS3, setting bit 2.
      //
      MrcData->DynamicVars[Channel][REG_DATA] = MrcData->MR3_Value | 4;
      MrcData->DynamicVars[Channel][RDDCMD] = 1;
      MrcData->DynamicVars[Channel][RANK] = Rank;
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    }
#endif // DDR3_SUPPORT
#if LPDDR4_SUPPORT
    //
    // Issue a Precharge All
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
      PrechargeAll (MrcData, Channel, Rank);
    }

#ifdef RCVEN_ENTRY
    MmrcExecuteTask(MrcData, RCVEN_ENTRY, NO_PRINT, Channel);
#endif

#endif  //LPDDR4_SUPPORT

  } else {
#if DDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
      //
      // Exit MPR mode by issueing a MRS3, clearing bit 2.
      //
      MrcData->DynamicVars[Channel][REG_DATA] = MrcData->MR3_Value;
      MrcData->DynamicVars[Channel][RDDCMD] = 1;
      MrcData->DynamicVars[Channel][RANK] = Rank;
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      //
      // Issue a Precharge All, keeping BL in 32-bit mode.
      //
      ////PrechargeAll (MrcData, Channel, Rank);
    }
#endif // DDR3_SUPPORT
#if DDR4_SUPPORT
    //
    // Set the Drams out of read preamble training / MPR mode
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr4) {
      Mrs3Command.Data = ReadMrsCommand (MrcData, Channel, Rank, 3);
      Mrs4Command.Data = ReadMrsCommand (MrcData, Channel, Rank, 4);
      WriteMrsCommand (MrcData, Channel, Rank, Mrs3Command.Data);
      WriteMrsCommand (MrcData, Channel, Rank, Mrs4Command.Data);
   }
#endif

#if LPDDR4_SUPPORT
#ifdef RCVEN_EXIT
    MmrcExecuteTask(MrcData, RCVEN_EXIT, NO_PRINT, Channel);
#endif  //RCVEN_ENTRYEXIT
#endif  //LPDDR4_SUPPORT
  }
#endif
  return MMRC_SUCCESS;
}

MMRC_STATUS
VOCTraining (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if VOC_TRAINING == 1
  UINT32  PullUpRon;
  UINT32  PullDnRon;
  UINT8   LdoEnable;
//  UINT16  Vpor; //*TODO* Depends on Technology
//  UINT16  Vcm;  //*TODO* Depends on Technology
//  UINT16  Vldo; //*TODO* Depends on Technology
  UINT8   Bit;
  UINT8   Strobe;
  UINT32  TempValue;
  UINT32  Rext;
  UINT32  VrefRMin; //*TODO* Depends on Technology - comes from spreadsheet
  UINT32  VrefRMax; //*TODO* Depends on Technology - comes from spreadsheet
  UINT32  CompVrefCode;
  UINT16  PullDnRComp[3];
  UINT16  PullUpRComp;
  UINT16  Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT16  Low[MAX_CHANNELS][MAX_STROBES];
  UINT16  High[MAX_CHANNELS][MAX_STROBES];
  UINT16  Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT32  MinVref[MAX_STROBES], MidVref[MAX_STROBES], MaxVref[MAX_STROBES];
  UINT8   LoopIteration;
  UINT8   DebugLevel;
  UINT8   i;
  UINT8   SegmentsEnabled;
  UINT8   LowLimit=0, HighLimit=0, BitState;
  UINT8   MasterSlice;

#ifdef SLICE_0_MASTER
  MasterSlice = (MrcData->DynamicVars[Channel][SLICE_0_MASTER] == 1) ? 0 : 1;
#else
  MasterSlice = 0;
#endif

  //Vpor      = 1100;
  //Vcm       = Vpor * 2 / 9;
  //PullDnRon = 40;
  //Vldo      = 850;
  Rext      = VOC_Rext;
  ////TODO: Find out if there is a safe value that can be used to initialize these variables
  PullUpRon = 0;
  PullDnRon = 0;
  TempValue = 0;
  //
  // The entry task should have captured values that may be corrupted during the flow of the execution that will be restored during the exit task.
  //
  #ifdef VOC_TRAINING_ENTRY
    MmrcExecuteTask (MrcData, VOC_TRAINING_ENTRY, NO_PRINT, Channel);
  #endif
  DebugLevel = MrcData->MrcDebugMsgLevel;
  //
  // Turn on 1D sweep debug messages if MAX message level is set.
  //
  //
  // Determine if the LDO is enabled.
  //
  MmrcExecuteTask (MrcData, VOC_GET_LDO_ENABLE, NO_PRINT, Channel);
  LdoEnable = (UINT8) MrcData->DynamicVars[Channel][VOC_LDO_ENABLE] == 0 ? TRUE : FALSE ;
  /*
  //
  // Calculate the Pullup Ron for the ODT segment.
  //
  if (LdoEnable == TRUE) {
    PullUpRon = ((Vldo-Vcm)*PullDnRon)/Vcm;
  } else {
    PullUpRon = ((Vpor-Vcm)*PullDnRon)/Vcm;
    PullUpRon = 140;
  }
  */
  //
  // Remove the RCOMP from override mode and set the phy entry settings.
  //
  MmrcExecuteTask (MrcData, VOC_PHY_ENTRY, NO_PRINT, Channel);
  //
  // Set all DQ VOC Offset to 0.
  //
  for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
    for (Bit = 0 ; Bit < MAX_BITS; Bit++) {
      TempValue = 7;
#ifdef RxVocVal0_Ecc
      if (Strobe < MAX_STROBES_NON_ECC) {
        GetSetDdrioGroup (MrcData, 0, Channel, 0, 0, Strobe, 0, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxVocVal0+Bit, CMD_SET_VAL_FC, &TempValue);
      } else {
        GetSetDdrioGroup (MrcData, 0, Channel, 0, 0, 0, 0, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxVocVal0_Ecc+Bit, CMD_SET_VAL_FC, &TempValue);
      }
#else
      GetSetDdrioGroup (MrcData, 0, Channel, 0, 0, Strobe, 0, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxVocVal0+Bit, CMD_SET_VAL_FC, &TempValue);
#endif
    }
  }
  //
  // Set the COMP Vref for the Pull up Ron and issue the Init.
  //
  if (LdoEnable == TRUE && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    VrefRMin = VOC_LP4_VrefRMin_LDO_ENABLE ;
    VrefRMax = VOC_LP4_VrefRMax_LDO_ENABLE;
    PullUpRon = VOC_LP4_PullUpRon_LDO_ENABLE;
    PullDnRon = VOC_LP4_PullDnRon_LDO_ENABLE;
    SegmentsEnabled = VOC_LP4_SegmentsEnabled_LDO_ENABLE;
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    VrefRMin = VOC_LP4_VrefRMin_LDO_DISABLE ;
    VrefRMax = VOC_LP4_VrefRMax_LDO_DISABLE;
    PullUpRon = VOC_LP4_PullUpRon_LDO_DISABLE;
    PullDnRon = VOC_LP4_PullDnRon_LDO_DISABLE;
    SegmentsEnabled = VOC_LP4_SegmentsEnabled_LDO_DISABLE;
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    VrefRMin = VOC_LP3_VrefRMin;
    VrefRMax = VOC_LP3_VrefRMax;
    PullUpRon = VOC_LP3_PullUpRon;
    PullDnRon = VOC_LP3_PullDnRon;
    SegmentsEnabled = VOC_LP3_SegmentsEnabled;
  } else { // DDR3L
    VrefRMin = VOC_DDR3L_VrefRMin;
    VrefRMax = VOC_DDR3L_VrefRMax;
    PullUpRon = VOC_DDR3L_PullUpRon;
    PullDnRon = VOC_DDR3L_PullDnRon;
    SegmentsEnabled = VOC_DDR3L_SegmentsEnabled;
  }
  CompVrefCode = ((Rext*1000/(Rext + SegmentsEnabled*PullUpRon)) - VrefRMin) /((VrefRMax-VrefRMin)/63);
  MmrcDebugPrint ((MMRC_DBG_MIN, "Comp Vref Code = %d\n",CompVrefCode));
  MrcData->DynamicVars[0][SS_TEMP_VAR] = CompVrefCode;
  MmrcExecuteTaskImplementation(MrcData, VOC_COMP_VREF_AND_INIT, NO_PRINT, MasterSlice, TRUE, MasterSlice + 1);
  //
  // Retrieve the value from the RCOMP and save it.
  //
  MmrcExecuteTask (MrcData, VOC_GET_RCOMP_PUP_VALUE, NO_PRINT, Channel);
  PullUpRComp = (UINT16) MrcData->DynamicVars[Channel][VOC_RCOMP_PUP];
  //
  // Print out the results for the pull ups.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "Pull Up Ron = %d\n",PullUpRon));
  MmrcDebugPrint ((MMRC_DBG_MIN, "Pull Up COMP Vref Code = %d\n",CompVrefCode));
  MmrcDebugPrint ((MMRC_DBG_MIN, "Pull Up RComp = %d\n",PullUpRComp));
  //
  // Set the COMP Vref for the Pull up Ron and issue the Init.
  //
  CompVrefCode = ((Rext*1000/(Rext + SegmentsEnabled*PullDnRon)) - VrefRMin) /((VrefRMax-VrefRMin)/63);
  if (CompVrefCode > 63) {
    CompVrefCode = 63;
  }
  MrcData->DynamicVars[0][SS_TEMP_VAR] = CompVrefCode;
  for (i=0; i<3; i++)
  {
    MmrcExecuteTaskImplementation(MrcData, VOC_COMP_VREF_AND_INIT, NO_PRINT, MasterSlice, TRUE, MasterSlice + 1);
    //
    // Retrieve the value from the RCOMP and save it.
    //

    MmrcExecuteTask (MrcData, VOC_GET_RCOMP_PDN_VALUE, NO_PRINT, Channel);
    PullDnRComp[i] = (UINT16) MrcData->DynamicVars[Channel][VOC_RCOMP_PDN];
  }
  //
  // Print out the results for the pull dns.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "Pull Dn Ron = %d\n",PullDnRon));
  MmrcDebugPrint ((MMRC_DBG_MIN, "Pull Dn COMP Vref Code = %d\n",CompVrefCode));
  MmrcDebugPrint ((MMRC_DBG_MIN, "Pull Dn RComp = %d, %d, %d\n",PullDnRComp[0],PullDnRComp[1],PullDnRComp[2] ));
  //
  // Step 8. Set the ODT RCOMP.
  //
  MmrcExecuteTask (MrcData, VOC_SET_ODT_RCOMP_OVERRIDE, NO_PRINT, Channel);
  //
  // Step 9.  Enable WrLVL mode for the host only.
  //
  MmrcExecuteTask (MrcData, VOC_ENABLE_WLMODE, NO_PRINT, Channel);
  //
  // Sweep the Rx Vref code and monitor the o/p of the DQ Rx for all 8-bits.  This will provide a range of Vref which will
  // switch the DQ o/p from 1 -> 0.  If all DQ O/P switch at the same VREF, then VOC sweep can be skipped. Need to make
  // sure the high side is skipped as one direction sweep is enough.
  //
  for (LoopIteration = 0; LoopIteration < 2; LoopIteration++) {
    if (LoopIteration == 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Sweeping Vref for 1->0 transition on VOCSampler holding VOC=0\n"));
      MmrcDebugPrint ((MMRC_DBG_MIN, "VOC  "));
      for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "  SL%d", Strobe));
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
           MmrcDebugPrint ((MMRC_DBG_MIN, ":00"));
        }
        if ((Strobe %4 == 3) && (Strobe < (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] -1)))
          MmrcDebugPrint ((MMRC_DBG_MIN, "\n     ",TempValue));
        else
          MmrcDebugPrint ((MMRC_DBG_MIN, "      ",TempValue));
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "VREF "));
    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
      Dim1StartPoint[Channel][0][Strobe][LOW] = 0;
      Dim1StartPoint[Channel][0][Strobe][HIGH] = VOC_VREF_MAX + 1;
      Low[Channel][Strobe]                     = 0;
      High[Channel][Strobe]                    = VOC_VREF_MAX;
    } // Strobe loop ...
    MrcData->VocSweep = 0;
    MrcData->MrcDebugMsgLevel = MMRC_DBG_MAX;
    Create1DSweepLastPass (MrcData, Channel, 0, RxVref, 1, Dim1StartPoint, Low, High, VOC_VREF_STEP_SIZE,
       FALSE, FALSE, TRUE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0], VrefValueToSwitchVOC, Results, "S", 0);
    MrcData->MrcDebugMsgLevel = DebugLevel;
    //
    // Print the results, but also determine the common vref to be programmed for all bits.
    //
    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
      TempValue = 1;
#ifdef RxVocEnDq_Ecc
      if (Strobe < MAX_STROBES_NON_ECC) {
        GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocEnDq, CMD_SET_VAL_FC_UC, &TempValue);
      } else {
          GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RxVocEnDq_Ecc, CMD_SET_VAL_FC_UC, &TempValue);
      }
#else
      GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocEnDq, CMD_SET_VAL_FC_UC, &TempValue);
#endif
      MmrcDebugPrint ((MMRC_DBG_MIN, "  SL%d", Strobe));
      MinVref[Strobe] = 0xff;
      MaxVref[Strobe] = 0x0;
      for (Bit = 0; Bit < MAX_BITS; Bit++) {
        if (Results[Channel][0][Strobe][Bit][LOW] < MinVref[Strobe]) {
          MinVref[Strobe] = Results[Channel][0][Strobe][Bit][LOW];
        }
        if (Results[Channel][0][Strobe][Bit][LOW] > MaxVref[Strobe]) {
          MaxVref[Strobe] = Results[Channel][0][Strobe][Bit][LOW];
        }
        MmrcDebugPrint ((MMRC_DBG_MIN, ":%02d",Results[Channel][0][Strobe][Bit][LOW]));
      }
      //
      // Set the minimum  vref any bit has transitioned.. this is where VOC begins its sweeps.
      //
      MidVref[Strobe]=(MinVref[Strobe] + MaxVref[Strobe])/2;
      MmrcDebugPrint ((MMRC_DBG_MIN, "[%02d]",MidVref[Strobe]));
      GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVref, CMD_SET_VAL_FC_UC, &MidVref[Strobe]);
      if ((Strobe %4 == 3) && (Strobe < (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] -1)))
        MmrcDebugPrint ((MMRC_DBG_MIN, "\n     ",TempValue));
      else
          MmrcDebugPrint ((MMRC_DBG_MIN, "  ",TempValue));
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
    //
    //  Dumb Sweep
    //
    HighLimit=0;
    LowLimit=0;
    if (LoopIteration == 0) {
      for (i=0; i< 16; i++)
      {
        MmrcDebugPrint ((MMRC_DBG_MIN, "%02d ",i));
        for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
          //Set Voc Value
          for (Bit = 0; Bit < MAX_BITS; Bit++) {
            TempValue = i;
#ifdef RxVocVal0_Ecc
            if (Strobe < MAX_STROBES_NON_ECC) {
              GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
            } else {
              GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RxVocVal0_Ecc+Bit, CMD_SET_VAL_FC_UC, &TempValue);
            }
#else
            GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
#endif
          }
#ifdef RxVocSmp_Ecc
          if (Strobe < MAX_STROBES_NON_ECC) {
            GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocSmp, CMD_GET_REG, &TempValue);
          } else {
            GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RxVocSmp_Ecc, CMD_GET_REG, &TempValue);
          }
#else
          GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocSmp, CMD_GET_REG, &TempValue);
#endif
          for (Bit = 0; Bit < MAX_BITS; Bit++) {
            BitState=((TempValue & (1 << Bit)) ==0)? 0 : 1;
            MmrcDebugPrint ((MMRC_DBG_MIN, "%01d:", BitState));
            if (BitState == 0 && i==0)  LowLimit=1;
            if (BitState == 1 && i==15)  HighLimit=1;
          }
          MmrcDebugPrint ((MMRC_DBG_MIN, "  "));
        }
        MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
      }
      for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Resetting Voc to 0\n"));
        //Zeroing out Voc
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
          TempValue = 0;
#ifdef RxVocVal0_Ecc
          if (Strobe < MAX_STROBES_NON_ECC) {
            GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
          } else {
            GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RxVocVal0_Ecc+Bit, CMD_SET_VAL_FC_UC, &TempValue);
          }
#else
          GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
#endif
        }
      }

      if ((LowLimit==1) && (HighLimit==1))
        MmrcDebugPrint ((MMRC_DBG_MIN, "\nERROR: VOC Failure, Bit Voltage offset differences are more than VOC range\n"));
      //
      //  Sweep with Brains
      //
      MmrcDebugPrint ((MMRC_DBG_MIN, "\nSweeping Vref for 1->0 transition on VOCSampler with VOC Normalized.\n"));
      for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
        Dim1StartPoint[Channel][0][Strobe][LOW] = 0;
        Dim1StartPoint[Channel][0][Strobe][HIGH] = 16;
        Low[Channel][Strobe]                     = 0;
        High[Channel][Strobe]                    = 15;
      } // Strobe loop ...
      MrcData->VocSweep = 1;
      MrcData->MrcDebugMsgLevel = MMRC_DBG_MAX;
#ifdef RxVocValAll
      Create1DSweepLastPass (MrcData, Channel, 0, RxVocValAll, 1, Dim1StartPoint, Low, High, VOC_VREF_STEP_SIZE,
#else
      Create1DSweepLastPass (MrcData, Channel, 0, RxVocVal0, 1, Dim1StartPoint, Low, High, VOC_VREF_STEP_SIZE,
#endif
         FALSE, FALSE, TRUE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0], VrefValueToSwitchVOC, Results, "S", 0);
      MrcData->MrcDebugMsgLevel = DebugLevel;
      //
      // Based on the results rom the test, program the VOC final falues.
      //
      MmrcDebugPrint ((MMRC_DBG_MIN, "VOC  "));
      for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "  SL%d", Strobe));
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
          TempValue = Results[Channel][0][Strobe][Bit][LOW];
#ifdef RxVocVal0_Ecc
          if (Strobe < MAX_STROBES_NON_ECC) {
            GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
          } else {
            GetSetDdrioGroup2 (MrcData, Channel, 0, 0, RxVocVal0_Ecc+Bit, CMD_SET_VAL_FC_UC, &TempValue);
          }
#else
          GetSetDdrioGroup2 (MrcData, Channel, 0, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
#endif
          MmrcDebugPrint ((MMRC_DBG_MIN, ":%02d",TempValue));
        }
        if ((Strobe %4 == 3) && (Strobe < (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] -1)))
          MmrcDebugPrint ((MMRC_DBG_MIN, "\n     ",TempValue));
        else
          MmrcDebugPrint ((MMRC_DBG_MIN, "      "));
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
    }
  }
  //
  // Step 10.  Get out of Host WRL mode.
  //
  MmrcExecuteTask (MrcData, VOC_DISABLE_WLMODE, NO_PRINT, Channel);
  MrcData->MrcDebugMsgLevel = DebugLevel;
#ifdef VOC_TRAINING_EXIT
  MmrcExecuteTask (MrcData, VOC_TRAINING_EXIT, NO_PRINT, Channel);
  // Always restore CH0 since it is modified by VOC_COMP_VREF_AND_INIT
  MmrcExecuteTask (MrcData, VOC_TRAINING_EXIT, NO_PRINT, 0);
#endif
#endif // VOC_TRAINING

  return MMRC_SUCCESS;
}
MMRC_STATUS
VrefValueToSwitchVOC (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
  )
{
#if VOC_TRAINING == 1
  UINT8  Strobe;
  UINT32 TempValue;
  UINT8  Bit;

  //
  // Read the WriteLvl Sample.
  // Thebit is considered passed if the corresponding bit in the register is sampled as 0.
  //
  for (Strobe = 0; Strobe < NumberElements; Strobe++) {
    //
    // Check if this is for the VOC sweep, if so, then we need to set all the VOC Vrefs, where were not set in the create1dsweep
    // call.
    //
    if (MrcData->VocSweep == 1) {
#ifdef RxVocVal0_Ecc
      if (Strobe < MAX_STROBES_NON_ECC) {
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocVal0, CMD_GET_REG, &TempValue);
      } else {
        GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxVocVal0_Ecc, CMD_GET_REG, &TempValue);
      }
#else
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocVal0, CMD_GET_REG, &TempValue);
#endif
      if (TempValue != 0) {
        for (Bit = 1; Bit < MAX_BITS; Bit++) {
#ifdef RxVocVal0_Ecc
          if (Strobe < MAX_STROBES_NON_ECC) {
            GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
          } else {
            GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxVocVal0_Ecc+Bit, CMD_SET_VAL_FC_UC, &TempValue);
          }
#else
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocVal0+Bit, CMD_SET_VAL_FC_UC, &TempValue);
#endif
        }
      }
    }
    MrcDelay (MrcData, MICRO_DEL, 1);
#ifdef RxVocSmp_Ecc
    if (Strobe < MAX_STROBES_NON_ECC) {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocSmp, CMD_GET_REG, &TempValue);
    } else {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, RxVocSmp_Ecc, CMD_GET_REG, &TempValue);
    }
#else
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxVocSmp, CMD_GET_REG, &TempValue);
#endif
    for (Bit = 0; Bit < MAX_BITS; Bit++) {
      if ((TempValue & (1<<Bit)) != 0) {
        PassFail[Channel][0][Strobe][Bit][CURRENT_SAMPLE] = RESULTS_FAIL;
      } else {
        PassFail[Channel][0][Strobe][Bit][CURRENT_SAMPLE] = RESULTS_PASS;
        }
    }
  }
#endif // VOC_TRAINING
  return MMRC_SUCCESS;
}

/**
  Performs the Receive Enable calibration.  All results from the calibration
  are stored within the MrcData within the TrainingData substructure.
  Channel is the only additional parameter passed in, and the calibration will
  be performed on the channel specified.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ReceiveEnable (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if RECEIVE_ENABLE == 1 && TRAINING_ALGOS == 1
  UINT8             Rank;                       // Rank being tested.
  UINT8             Strobe;                     // Strobe being tested.
  UINT32            TempValue;                  // Temporary storage element used throughout the algorithm.
  UINT32            HalfClock[MAX_CHANNELS];    // Contains the number of UIs per Half clock.
//[-start-160517-IB03090427-modify]//
  UINT8             InitialDelay=0;               // Initial delay to start the receive enable training.
//[-end-160517-IB03090427-modify]//
  UINT8             SmallStep;                  // Small step size.
  UINT8             MediumStep;
  UINT8             LargeStep;
  UINT16            Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT16            Low[MAX_CHANNELS][MAX_STROBES];
  UINT16            High[MAX_CHANNELS][MAX_STROBES];
  UINT16            Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT8             PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3];
  UINT8             DebugLevel;
  UINT8             DitherVal;
  UINT32            PiInPs;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    InitialDelay  = RCVN_INITIAL2XVAL_LP4;
#if LPDDR3_SUPPORT
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    InitialDelay  = RCVN_INITIAL2XVAL_LP3;
#endif
#if DDR3_SUPPORT
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All) {
    //MemoryDown DDR3L options are requiring a lower starting value compared to SODIMM.  Minus4
    InitialDelay  =  (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown > 0) ? (RCVN_INITIAL2XVAL_DDR3 - 4) : RCVN_INITIAL2XVAL_DDR3 - 3;
#endif
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Memory type unsupported.\n"));
    ASSERT (DDR3_SUPPORT && LPDDR3_SUPPORT && LPDDR4_SUPPORT);
    return MMRC_FAILURE;
  }
  LargeStep     = RCVN_LARGE_STEP;
  MediumStep    = RCVN_MEDIUM_STEP;
  SmallStep     = RCVN_SMALL_STEP;
  DitherVal     = DITHER;
  DebugLevel = MrcData->MrcDebugMsgLevel;
  //
  // Turn on 1D sweep debug messages if MAX message level is set.
  //
  if ((MrcData->MrcDebugMsgLevel & MMRC_DBG_MAX) == MMRC_DBG_MAX) {
    MrcData->MrcDebugMsgLevel |= MMRC_DBG_DEBUG;
  }
  //
  // Receive Enable Sample
  //
  MrcData->SignalType = RecEnSmp;
  //
  // ReceiveEnable Ddrio phy entry hook
  //
#ifdef RECEIVE_ENABLE_ENTRY
  MmrcExecuteTask (MrcData, RECEIVE_ENABLE_ENTRY, NO_PRINT, Channel);
#endif
  //
  // Check to see if doing the fastboot or training path.
  //

  //
  // If using CPGC to do the RCVN testing, then setup the engine.
  //
#if CPGC_BASED_READS == 1
  MrcData->CpgcOptions.LoopCount = 1;
  MrcData->CpgcOptions.NumBursts = 1;
  CpgcSetup (MrcData, Channel, CPGC_CONFIG_EARLYTRAINING);
#endif
  //
  // Loop through all enabled ranks performing the rcvn enable training.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "C%02dR%d\n", Channel, Rank));
    //
    // ReceiveEnable Dram entry hook
    //
    ReceiveEnableDramEntryExit (MrcData, Channel, Rank, MMRC_ENTRY);
    //
    // Determine the half clock size in PIs.
    //
    HalfClock[Channel] = GetHalfClk (MrcData, Channel);
    //
    // If using CPGC to do the RCVN testing, then a reconfiguration is required when
    // running on a different rank as well as to setup the engine for read only.
    //
#if CPGC_BASED_READS == 1
    //
    // Setup the CPGC engine to to do a single read from an address within the
    // selectable rank.  The engine should be setup for LFSR mode.
    //
    MrcData->CpgcOptions.Rank = Rank;
    MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_READ;
    L_CpgcReconfig (MrcData, Channel);
#endif
    //
    // Issue a prechargeall only for DDR3 or DDR4.
    //
    if  (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr4 ) {
      PrechargeAll (MrcData, Channel, Rank);
    }
    //
    // Set the initial starting point for RCVN which are in units of HalfClock.
    //
    MmrcDebugPrint((MMRC_DBG_MIN, "Initial Delay Value=%d\n", (HalfClock[Channel] * InitialDelay)));
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
        Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)(HalfClock[Channel] * InitialDelay);
    }

    //
    // For all furture RCVN testing, setup the low/high limitis as well as the starting point for the high side to be 1 larger then
    // the MAX limit.  This forces the 1dsweep to not test the high side.  All future calls only needs to set the LOW starting point.
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      Dim1StartPoint[Channel][0][Strobe][HIGH] = Dim1StartPoint[Channel][0][Strobe][LOW] + (UINT16)(HalfClock[Channel] * 8)+1;
      Low[Channel][Strobe]                     = 0;
      High[Channel][Strobe]                    = (UINT16)(Dim1StartPoint[Channel][0][Strobe][LOW] + (HalfClock[Channel] * 8));
    } // Strobe loop ...



    //
    // Perform the edge detect with LARGE step increments.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Detect Rising Edge Large, Step=%d, dithering=%d\n", LargeStep, 0));
    Create1DSweepLastPass (MrcData, Channel, Rank, RecEnDelay, 1, Dim1StartPoint, Low, High, LargeStep,
      FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 0);
    //
    // Set the next starting point to the last pass from the previous step.
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      Dim1StartPoint[Channel][0][Strobe][LOW]  = (UINT16) (Results[Channel][0][Strobe][0][LOW]);
    }
    //
    // Perform the edge detect with MEDIUM step increments.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Detect Rising Edge Medium, Step=%d, dithering=%d\n", MediumStep, 0));
    Create1DSweepLastPass (MrcData, Channel, Rank, RecEnDelay, 1, Dim1StartPoint, Low, High, MediumStep,
      FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 0);
    //
    // Set the next starting point to the last pass from the previous step.
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
     Dim1StartPoint[Channel][0][Strobe][LOW]  = (UINT16) (Results[Channel][0][Strobe][0][LOW]);
    }
    //
    // Perform the edge detect with SMALL step increments and perform the dithering test.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Detect Rising Edge Small, Step=%d, dithering=%d\n", SmallStep, DitherVal));
    Create1DSweepLastPass (MrcData, Channel, Rank, RecEnDelay, 1, Dim1StartPoint, Low, High, SmallStep,
      FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", DitherVal);
    //
    // At this point, we now have a stable rising edge detected.
    // Add 1/4 Clock to get to the center of the positive pulse.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Add 1/4 Clk Value+=%d\n", (HalfClock[Channel]/2)));
    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
      Dim1StartPoint[Channel][0][Strobe][LOW]  = Results[Channel][0][Strobe][0][LOW] + (UINT16) (HalfClock[Channel] / 2);
      TempValue=Dim1StartPoint[Channel][0][Strobe][LOW];
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RecEnDelay, CMD_SET_VAL_FC_UC, &TempValue);
    }
    //
    // Verify that a 1 is sampled on each strobe.
    //
    SampleDqs(MrcData, Channel, Rank, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], 1, PassFail);
    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
      if (PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE]== RESULTS_FAIL) {
         MmrcDebugPrint ((MMRC_DBG_MIN, "Sampled a 0 after adding a 1/4 clock on Strobe %d!", Strobe));
         return MMRC_FAILURE;
      }
    }
    //
    // Begin stepping backwards one clock at a time to detect the first pulse.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Step back clks to find first strobe, Step=%d, dithering=%d\n", HalfClock[Channel]*2, 0));
    Create1DSweepLastPass (MrcData, Channel, Rank, RecEnDelay, 1, Dim1StartPoint, Low, High, (UINT16) (HalfClock[Channel] * 2),
      FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 0);
    //
    // Set the starting point to be the center of the first pulse.
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
     Dim1StartPoint[Channel][0][Strobe][LOW]  = (UINT16) (Results[Channel][0][Strobe][0][LOW]);
    }
    //
    // Begin walking backwards in MEDIUM steps to find the first rising edge after the preamble.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Find first rising edge medium, Step=%d, dithering=%d\n", MediumStep, 0));
    Create1DSweepLastPass (MrcData, Channel, Rank, RecEnDelay, 1, Dim1StartPoint, Low, High, MediumStep,
      FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 0);
    //
    // Set the next starting point to the last pass from the previous step.
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
     Dim1StartPoint[Channel][0][Strobe][LOW]  = (UINT16) (Results[Channel][0][Strobe][0][LOW]);
    }
    //
    // Walking backwards in SMALL steps to find the first rising edge after the preamble, performing dither checking.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "Find first rising edge small, Step=%d, dithering=%d\n", SmallStep, DitherVal));
    Create1DSweepLastPass (MrcData, Channel, Rank, RecEnDelay, 1, Dim1StartPoint, Low, High, SmallStep,
      FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", DitherVal);

    //
    // Subtract N CLK to each LANE to put the RCVEN at the center of a 0.
    //
    MmrcDebugPrint ((MMRC_DBG_MAX, "Subtract N quarter clocks to put at center of preamble.\n"));
    PiInPs = (MrcData->TwoXps + HalfClock[Channel]/2) / HalfClock[Channel];

    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }

      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        TempValue = Results[Channel][0][Strobe][0][LOW] - (RCVN_PREAMBLE_CNTR_LP3 * HalfClock[Channel] + RCVN_PREAMBLE_OFFSET_LP3/PiInPs);
      } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
        TempValue = Results[Channel][0][Strobe][0][LOW] - (RCVN_PREAMBLE_CNTR_LP4 * HalfClock[Channel] + RCVN_PREAMBLE_OFFSET_LP4/PiInPs);
      } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
#if DDR3_SUPPORT
        TempValue = Results[Channel][0][Strobe][0][LOW] - (RCVN_PREAMBLE_CNTR_DDR3 * HalfClock[Channel] + RCVN_PREAMBLE_OFFSET_DDR3/PiInPs);
#endif
      }
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RecEnDelay, CMD_SET_VAL_FC_UC, &TempValue);
      PerformFifoReset(MrcData, Channel, Rank);
    }
    //
    // ReceiveEnable Dram exit hook
    //
    ReceiveEnableDramEntryExit (MrcData, Channel, Rank, MMRC_EXIT);

  } // Rank loop ...

  if (MrcData->Rk2RkEn.RcvEn == 0) {
    Average (MrcData, Channel, RecEnDelay, MMRC_NA);
  }
  //
  // Print out or restore the training results.
  //
  MmrcDebugPrint ((MMRC_DBG_MAX, "Note: The final linear delay below includes the 1x, 2x, and PI.\n\n"));
#ifdef RecEnDelay_dq
  DisplayOrRestoreTrainingResults (MrcData, Channel, RecEnDelay_dq, FALSE, MAX_STROBES_NON_ECC);
#endif
#ifdef RecEnDelay_Ecc
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    DisplayOrRestoreTrainingResults (MrcData, Channel, RecEnDelay_Ecc, FALSE, 1);
  }
#else
  DisplayOrRestoreTrainingResults (MrcData, Channel, RecEnDelay, FALSE, MAX_STROBES);
#endif
  //
  // ReceiveEnable DDRIO Phy Exit hook
  //
#ifdef RECEIVE_ENABLE_EXIT
  MmrcExecuteTask (MrcData, RECEIVE_ENABLE_EXIT, NO_PRINT, Channel);
#endif
  ReceiveEnableExit(MrcData, Channel);

  MrcData->MrcDebugMsgLevel = DebugLevel;
#endif // RECEIVE_ENABLE == 1
  return MMRC_SUCCESS;
}

#if (RECEIVE_ENABLE == 1 || FINE_WRITE_LEVELING == 1) && TRAINING_ALGOS == 1
MMRC_STATUS
SampleDqs (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
  )
{
  UINT8         Strobe;               // Strobe Index for currently active.
  UINT32        TempValue;            // Place holder for the read results, not used just required for the read.
  UINT8         Sample;               // Result of reading the Sample register for phase alignment.
  UINT8         SampleIndex;          // Loop Counter for performing the total number of samples.
  INT8          Results[MAX_CHANNELS][MAX_STROBES]; // Internal signed accumulation of the results, this will Count
                                      // +1 when a 1 is sampled, -1 when a 0 is sampled.
  UINT32        FinishBL[MAX_CHANNELS];// Holds whether a specific strobe has completed the total required samples.
  UINT32        TotalFinishStrobes;   // Strobe signature when all strobes have passed the required number of samples.
  UINT8         SampleThreshold;      // Number of samples that need to equal a common value before that value is chosen.
  UINT8         ChannelBackup;

  //
  // Before starting, perform a single read to the PHY to clear any pending PHY register writes
  GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->SignalType, CMD_GET_REG, &TempValue);

  ChannelBackup = Channel;
  //
  // Initialize variables.
  //
  TotalFinishStrobes = (1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]) - 1;
  TempValue = 0;
  MmrcMemset (FinishBL, 0, MAX_CHANNELS * sizeof (UINT32));
  //
  // Zero out the signed accumulated results before starting the sampling intervals.
  //
  MmrcMemset (Results, 0, MAX_CHANNELS * MAX_STROBES * sizeof (INT8));
  //
  // Set the sample index to the maximum number of samples required.  This is a project specific compile switch.
  //
  SampleIndex     = NUMSAMPLES;
  SampleThreshold = SAMPLETHRESH;
  //
  // Loop on the total number of samples, but sampling may stop earlier if threshold of 1's or 0's is reached.
  //
  while (SampleIndex--) {
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
    if ( (CPGC_BASED_READS  == 1 && MrcData->SignalType == RecEnSmp) ||
         (CPGC_BASED_WRITES == 1 && MrcData->SignalType == WrLvlSmp)) {
      //
      // Perform the CPGC Based access for RCVN if READ flag is enabled or
      // Wr LVL if WRITE flag is enabled.
      //
           /*
      MmrcExecuteTask (MrcData, CPGC_RUNTEST_PARALLEL_CLEAR, NO_PRINT, Channel);
      MmrcExecuteTask (MrcData, CPGC_RUNTEST_PARALLEL, NO_PRINT, Channel);
      MmrcExecuteTask (MrcData, CPGC_RUNTEST_PARALLEL_CHECK, NO_PRINT, Channel);
      */
            MmrcExecuteTask (MrcData, CPGC_RUN_TEST, NO_PRINT, Channel);
    }

    if (CPGC_BASED_WRITES == 0 && MrcData->SignalType == WrLvlSmp) {
      //
      // This feature is not supported.
      //
    }

    if (CPGC_BASED_READS == 0 && MrcData->SignalType == RecEnSmp) {
    #if DDR3_SUPPORT
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
        //
        // During Rcvn Training of LP4, we need to issue a DQ Reset before every Read.
        //
        PerformFifoReset(MrcData, Channel, Rank);
        //PerformDQReset(MrcData, Channel, Rank);
        //
        // Issue the Read Command and stall for a short time.
        //
        MrcData->DynamicVars[Channel][REG_DATA] = 0x280000;
        MrcData->DynamicVars[Channel][RDDCMD] = 1;
        MrcData->DynamicVars[Channel][RANK] = Rank;
        MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
        SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
        MySimStall(500);MySimStall(500);
        MrcDelay (MrcData, MICRO_DEL, 5);
      }
      #endif // DDR3_SUPPORT

      #if LPDDR3_SUPPORT
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        //
        // During Rcvn Training of LP3, we need to issue a DQ Reset before every Read.
        //
        PerformFifoReset(MrcData, Channel, Rank);
        //
        // Issue the Read Command and stall for a short time.
        //
        MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
        MrcData->DynamicVars[Channel][REG_DATA] = MRW32_LP3;
        MrcData->DynamicVars[Channel][RDDCMD] = 1;
        SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
        MySimStall(500); MySimStall(500);
        MrcDelay (MrcData, MICRO_DEL, 5);
      }
      #endif // LPDDR3_SUPPORT

      #if LPDDR4_SUPPORT
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
        //
        // During Rcvn Training of LP4, we need to issue a DQ Reset before every Read.
        //
        PerformFifoReset(MrcData, Channel, Rank);
        //
        // Issue the Read Command and stall for a short time.
        //
        MrcData->DynamicVars[Channel][REG_DATA] = MPC06_LP4;
        MrcData->DynamicVars[Channel][RDDCMD] = 1;
        MrcData->DynamicVars[Channel][RANK] = Rank;
        MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
        SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
        MySimStall(500);MySimStall(500);
        MrcDelay (MrcData, MICRO_DEL, 5);
      }
      #endif // LPDDR4_SUPPORT
    }

    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
        continue;
      }
      //
      // Go through each strobe lane, checking if the threshold has been reached.
      //
      for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
          continue;
        }
        //
        // If the strobe lane has hit its threshold, then go to the next strobe lane.
        //
        if (FinishBL[Channel] & (1 << Strobe) ) {
          continue;
        }
        //
        // Read the sample register.
        //
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, MrcData->SignalType, CMD_GET_REG, &TempValue);
        Sample = (UINT8) TempValue;
        //
        // Accumulate the results for that strobe lane, by adding a 1 if a 1 is sample, subtracting a 1 if a 0 is sampled.
        //
        Results[Channel][Strobe] += ((Sample * 2) - 1 );
        //
        // If the resultant count is above the threshold, then set the bit-field for the Strobe lane to a 1.
        //
        if ((Results[Channel][Strobe] >= SAMPLETHRESH) || Results[Channel][Strobe] <= (SAMPLETHRESH * -1)) {
          FinishBL[Channel] |= (1 << Strobe);
        }
      } // Strobe loop ...
      //
      // If all strobe lanes have hit the threshold, then break out of the loop early.
      //
      if (FinishBL[Channel] == TotalFinishStrobes) {
        break;  //finished all bytelane, get all 1 or 0
      }
      if (!MrcData->ExecuteThisRoutineInParallel) {
        //
        // Stop the routine after first execution
        // if not supposed to be executed in parallel
        //
        break;
      }
    } // Channel loop ...
  }  // while (--SampleIndex)

  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    //
    // The return results, should see if more 0's then return 0 otherwise return 1.. Zero implies take 1.
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if (Results[Channel][Strobe] >= 0) {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_PASS;
      } else {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_FAIL;
      }
    }
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  return MMRC_SUCCESS;

}
#endif // (RECEIVE_ENABLE == 1 || FINE_WRITE_LEVELING == 1) && TRAINING_ALGOS == 1

/**
  Routine to indentify the right cache index array

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       CacheIndex      Cache Index to be accessed, this pointer is gotten from  Elements['Type'] table.

  @retval          CacheArrayIndex
**/
UINT8
GetCacheIndexArray(
  IN  OUT   MMRC_DATA    *MrcData,
  IN        UINT16        CacheIndex
)
{
  UINT8 index;
  for (index = 0; index < MaxCacheTypes; index++) {
    if ((CacheIndex & 0xC0) == CacheTypePrefix[index]) {
      break;
    }
  }
  if (index == MaxCacheTypes) {
    MrcDeadLoop ();
  }
  return index;
}
/**
  Routine to Get/Set the value to the right Cache array

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Strobe          Strobe selection for the requested field.
  @param[in]       Bit             Bit within field being examined.
  @param[in]       CacheIndex      Cache Index to be accessed, this pointer is gotten from  Elements['Type'] table.
  @param[in]       Cmd             Command to read/write from cache (valid commands RD_ONLY and UPD_CACHE)
  @param[in]       Value           Value to be set/retrieved for the an specific Cache array.

  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GetSetCache (
  IN  OUT   MMRC_DATA    *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         CacheIndex,
  IN        UINT8         Cmd,
  IN        UINT32        *Value
  )
{
  UINT8   CacheArrayIndex;
  CacheArrayIndex = GetCacheIndexArray(MrcData, CacheIndex);
  CacheIndex &= ~0xC0;
  if (Cmd & RD_ONLY) {
    switch (CacheArrayIndex) {
    case TypeCache:
      *Value = MrcData->NvData.MrcParamsSaveRestore.CachedValues[Channel][CacheIndex][Rank][Strobe];
      break;
    case TypeCachePB:
      *Value = MrcData->NvData.MrcParamsSaveRestore.CachedValuesPB[Channel][CacheIndex][Rank][Strobe][Bit];
      break;
    case TypeCacheNonSR:
      *Value = MrcData->CachedValuesNonSR[Channel][CacheIndex][Rank][Strobe];
      break;
    case TypeCacheNonSRPB:
      *Value = MrcData->CachedValuesNonSRPB[Channel][CacheIndex][Rank][Strobe][Bit];
      break;
    default:
      MrcDeadLoop ();
      break;
    }
  } else if (Cmd & UPD_CACHE) {
    switch (CacheArrayIndex) {
    case TypeCache:
      MrcData->NvData.MrcParamsSaveRestore.CachedValues[Channel][CacheIndex][Rank][Strobe] = *Value;
      break;
    case TypeCachePB:
      MrcData->NvData.MrcParamsSaveRestore.CachedValuesPB[Channel][CacheIndex][Rank][Strobe][Bit] = *Value;
      break;
    case TypeCacheNonSR:
      MrcData->CachedValuesNonSR[Channel][CacheIndex][Rank][Strobe] = *Value;
      break;
    case TypeCacheNonSRPB:
      MrcData->CachedValuesNonSRPB[Channel][CacheIndex][Rank][Strobe][Bit] = *Value;
      break;
    default:
      MrcDeadLoop ();
      break;
    }
  } else {
    MrcDeadLoop ();
  }
  return MMRC_SUCCESS;
}

/**
  Routine to save size which doesn't include all the parameters
  of the real GetSetDdrioGroup. Use this routine for a single level memory
  configuration or when there is only a single level of memory per channel.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Strobe          Strobe selection for the requested field.
  @param[in]       Type            Register to be accessed, this is pointer to
                                   a register or algorithm in the Elements[] table.
  @param[in]       Cmd             Command to read/write from register/cache.
  @param[in]       Value           Value set/retrieved for the specified Type.
  @retval          MMRC_SUCCESS
  @retval          STATUS_TYPE_NOT_SUPPORTED
  @retval          STATUS_LIMIT
**/
MMRC_STATUS
GetSetDdrioGroup2 (
  IN  OUT   MMRC_DATA    *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT16        Type,
  IN        UINT8         Cmd,
  IN        UINT32       *Value
  )
{
  return GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, 0, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Type, Cmd, Value);
}

/**
  Routine to save size which doesn't include all the parameters
  of the real GetSetDdrioGroup. Use this routine for a single level memory
  configuration or when there is only a single level of memory per channel.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Socket          Current Socket being examined.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Dimm            Current Dimm being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Strobe          Strobe selection for the requested field.
  @param[in]       Bit             Bit within field being examined.
  @param[in]       FrequencyIndex  Frequency Index.
  @param[in]       IoLevel         I/O Level.
  @param[in]       Element         Register to be accessed, this is pointer to
                                   a register or algorithm in the Elements[] table.
  @param[in]       Cmd             Command to read/write from register/cache.
  @param[in]       Value           Value set/retrieved for the specified Type.
  @retval          MMRC_SUCCESS
  @retval          STATUS_TYPE_NOT_SUPPORTED
  @retval          STATUS_LIMIT
**/
MMRC_STATUS
GetSetDdrioGroup (
  IN  OUT   MMRC_DATA    *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT16        Element,
  IN        UINT8         Cmd,
  IN        UINT32       *Value
  )
{
#if  RTWT_SIM==1
  if (Element == Ctle && Cmd==CMD_SET_VAL_FC_UC) {
    CTLE[0][Channel][Strobe][Bit] = *Value;
  } // if Element
  if (Element == RxVref && (Cmd&WR_OFF)==WR_OFF) {
    RdVRef[0][Channel][Strobe][0] += *Value;
  }
  if (Element == RxVref && Cmd == CMD_SET_VAL_FC_UC) {
    RdVRef[0][Channel][Strobe][0] = *Value;
  } // if Element
  if (Element == RxDqsPDelay && Cmd==CMD_SET_VAL_FC_UC) {
    RxDqsP[Rank][Channel][Strobe][0] = *Value;
  } // if element
  if (Element == RxDqsPDelay && Cmd==CMD_GET_REG) {
    *Value = RxDqsP[0][Channel][Strobe][0];
  } // if element
  if (Element == RxDqsNDelay && Cmd==CMD_SET_VAL_FC_UC) {
    RxDqsN[0][Channel][Strobe][0] = *Value;
  } // if element
  if (Element == RxDqsNDelay && Cmd==CMD_GET_REG) {
    *Value = RxDqsN[0][Channel][Strobe][0];
  } // if element
  if (Element == RxDqsDelay && (Cmd&WR_OFF)==WR_OFF) {
    RxDqsP[0][Channel][Strobe][0] += *Value;
    RxDqsN[0][Channel][Strobe][0] += *Value;
  } // if element
  if (Element == RxDqsPDelay && (Cmd&WR_OFF)==WR_OFF) {
    RxDqsP[0][Channel][Strobe][0] += *Value;
  } // if element
  if (Element == RxDqsNDelay && (Cmd&WR_OFF)==WR_OFF) {
    RxDqsP[0][Channel][Strobe][0] += *Value;
  } // if element
  if (Element == Ctle && Cmd==CMD_GET_REG) {
    *Value = CTLE[0][Channel][Strobe][Bit];
  }
  if (Element == RxVref && Cmd==CMD_GET_REG) {
    *Value = RdVRef[0][Channel][Strobe][0];
  }
  if (Element == RxDqsDelay && Cmd==CMD_GET_REG) {
    *Value = RxDqsP[0][Channel][Strobe][0];
  } // if element
  if (Element == RxDqBitDelay && Cmd==CMD_SET_VAL_FC_UC) {
     RxDqPerBit[0][Channel][Strobe][Bit] = *Value;
  } // if element
  if (Element == RxDqBitDelay && Cmd==CMD_GET_REG) {
    *Value = RxDqPerBit[0][Channel][Strobe][Bit];
  } // if element
  if (Element != TxVref) {
      return MMRC_SUCCESS;
  }
 #endif // RTWT_SIM ==1
#if USE_GET_SET_FROM_SPREADSHEET
  {  // To be removed kirk
  UINT8                     StartingBit;
  UINT8                     EndingBit;
  UINT8                     PhysicalChannel;
  UINT8                     PhysicalStrobe;
  MMRC_STATUS               Status;
  UINT32                    MaxLimit;
  UINT32                    WaitTime;
  UINT8                     Instance;
  EXTERNAL_GETSET_FUNCTION *j;
  REGISTER_ACCESS           Register;
  UINT8                     i;
  UINT16                    ElementIndex;
  UINT32                    CurrentValue;
  UINT32                    NewValue;
  UINT8                     ThisGroup;
  UINT8                     TempBlueprint;
  UINT16                    Type;
  UINT32                    Check;
  UINT8                     PhysicalRank;
  UINT32                    CacheValue;

  WaitTime = 0;
  CacheValue = 0;
  TempBlueprint = MrcData->CurrentBlueprint;
  //
  // Given a PFCT and IoLevel, we can select the correct blueprint for this IO. If the IoLevel is passed
  // in as 0xFF, then we ignore it and just return the first blueprint that matches the PFCT for this
  // channel. This is useful in systems that have one level of memory per channel and also permits
  // the space efficient use of GetSetDdrioGroup2() which forces IoLevel to 0xFF.
  //
  Status = ConvertPfctAndIoLevelToBlueprint (MrcData, Channel, IoLevel, &MrcData->CurrentBlueprint);

  Type = GetOffsetIndex (MrcData, Element);

  if (Status != MMRC_SUCCESS) {
    MrcData->CurrentBlueprint = TempBlueprint;
    MmrcDebugPrint ((MMRC_DBG_MIN, "\nError: IO Level %d not supported for %s on Blueprint %d.\n",
      IoLevel, SignalInfo[GetSignalInfoIndex(Type)].Name, MrcData->CurrentBlueprint
      ));
    return STATUS_FAIL;
  }

  if (Type == REG_UNDEFINED) {
    MrcData->CurrentBlueprint = TempBlueprint;
    return STATUS_TYPE_NOT_SUPPORTED;
  }
  //
  // Map the passed channel and strobe to the physical channel and strobe using the global floorplan.
  //
  PhysicalChannel = FloorPlan[Channel][Strobe].Channel;
  PhysicalStrobe  = FloorPlan[Channel][Strobe].Strobelane;

  j = ExternalFunctions;
  //
  // Check if the register is a physical register request or an algorithm linear value request.
  // This is specifically checking if its a physical register request.
  //
  if (Type < ALGO_REG_INDEX) {
    if (Type < MAX_NUM_ELEMENTS) {
      //
      // If reading an element, check if the R2R Switching is enabled, if set to 1 for this type, then
      // Use Rank Switching, if set to 0, then use Rank Sharing (Rank 0), if pointing to a another GetSet
      // Then call the GetSet and get the value.  Based on the return value, set the PhysicalRank Index.
      // PhysicalRank is the Register used to control the rank settings.
      //
      if (Elements[Type].R2RSwitching == R2R_ENABLE) {
        PhysicalRank = Rank;
      } else if (Elements[Type].R2RSwitching == R2R_DISABLE) {
        PhysicalRank = 0;
      } else {
        GetSetDdrioGroup2 (MrcData, Channel,  0, Strobe, Elements[Type].R2RSwitching, CMD_GET_CACHE, &Check);
        if (Check == 1) {
          PhysicalRank = Rank;
        } else {
          PhysicalRank = 0;
        }
      }
    //
    // Check to make sure the frequency index passed in is less than the total number
    // of frequency indices listed for this element. If it is, adjust by the frequency
    // index passed in. If FrequencyIndex is 0, no adjustment occurs and we access
    // the first element.
    //
      if (FrequencyIndex < SignalInfo[GetSignalInfoIndex (Type)].FrequencyOffset) {
        if (Type < (NumberOfElementsPerAlgo * NumberAlgos)) {
          Type += (NumberOfElementsPerAlgo * FrequencyIndex);
        } else {
          Type += FrequencyIndex;
        }
      } else {
        MrcData->CurrentBlueprint = TempBlueprint;
        MmrcDebugPrint ((
          MMRC_DBG_MIN,
          "\nError: %s not available for freq %d on Blueprint %d.\n",
          SignalInfo[GetSignalInfoIndex(Type)].Name,
          FrequencyIndex,
          MrcData->CurrentBlueprint
          ));
        return STATUS_FAIL;//MrcFrequencyIndexNotSupported;
      }
      //
      // If the register being requested does not have a location offset, return MMRC_FAILURE,
      // this is to support the linear calculation which has
      //
      if (Elements[Type].RegisterLocation[PhysicalRank].Offset == MMRC_NA) {
        MrcData->CurrentBlueprint = TempBlueprint;
        return STATUS_TYPE_NOT_SUPPORTED;
      }
      //
      // If a cache read, then read directly from the trained values array.
      // Within the cache array itself, the Rank should be the virtual passed in rank (0/1/2/3) regarless of sharing.
      // The register ending/starting bits should be based on sharing since the physical location accessed is based on sharing.
      //
      if ((Cmd & RD_ONLY) != 0 && (Cmd & RD_REG) == 0) {
        GetSetCache (MrcData, Channel, Rank, Strobe, Bit, Elements[Type].CacheIndex, RD_ONLY, Value);
        if (Elements[Type].Box_Type != EXTERNAL) {
#ifdef SUSSW
          *Value &= ((((UINTX)1 << (Elements[Type].RegisterLocation[PhysicalRank].EndingBit + 1)) - 1) -
                     (((UINTX)1 << Elements[Type].RegisterLocation[PhysicalRank].StartingBit) - 1));
#else
          *Value &= ((LShiftU64(1, (Elements[Type].RegisterLocation[PhysicalRank].EndingBit + 1)) - 1) -
            (LShiftU64(1, Elements[Type].RegisterLocation[PhysicalRank].StartingBit) - 1));
#endif
          *Value >>= Elements[Type].RegisterLocation[PhysicalRank].StartingBit;
        }
        *Value = ConvertPhysicalToLinearValue (MrcData, Element, *Value);
        MrcData->CurrentBlueprint = TempBlueprint;
        return MMRC_SUCCESS;
      }
      //
      // If the number of instances per channel > 1, then Instance = Strobelane/2; otherwise, instance = 0.
      //
      if (InstancePortMap[Elements[Type].Box_Type].Instances > 1) {
        Instance = PhysicalStrobe / MAX_BYTELANES_PER_DQ_MODULE;
      } else {
        Instance = 0;
      }
      //
      // Get the actual byte/start/ending bit for the element based on channel/rank/strobe.
      //
      Register.Offset = 0;
      GetLocation (MrcData, Elements[Type], PhysicalChannel, PhysicalRank, PhysicalStrobe, Bit, &Instance, &Register.Offset, &StartingBit, &EndingBit);
      NewValue = *Value;
      //
      // If the command is a read, then read the register, masking the bits, and shifting the bits
      // so that the value is shown starting at bit 0.
      //
      if (Cmd & RD_REG) {
        if (Elements[Type].Box_Type == EXTERNAL) {
          //
          // When external functions are used, the offset for all ranks points to the index needed within the ExternalFunctions array.
          //
          j += Elements[Type].RegisterLocation[PhysicalRank].Offset;
          (*(j->Function)) (MrcData, Socket, Channel, Dimm, PhysicalRank, Strobe, Bit, FrequencyIndex, IoLevel, RD_REG, Value);
          j = ExternalFunctions;
        } else {
#ifdef SUSSW
          Register.Mask = (((UINTX)1 << (EndingBit + 1)) - 1) - (((UINTX)1 << StartingBit) - 1);
#else
          Register.Mask = (LShiftU64(1, (EndingBit + 1)) - 1) - (LShiftU64(1, StartingBit) - 1);
#endif
          Register.ShiftBit = StartingBit;
          *Value = (UINT32) MemFieldRead (Elements[Type].Box_Type, PhysicalChannel, Instance, Register);
        }
        //
        // Convert back to linear because this function only operates on linear values.
        //
        *Value = ConvertPhysicalToLinearValue (MrcData, Element, *Value);
        NewValue = *Value;
      }
      //
      // If the command is a write, compute the mask, shift the value to the appropriate bits,
      // and read/modify/write.
      if (Cmd & FC_WR) {
        //
        // If the value passed in is an offset from the current value, read the current value first, then
        // apply the offset.
        //
        if (Cmd & WR_OFF) {
          //
          // Read the current value of the register field.
          //
          GetSetDdrioGroup (MrcData, Socket, Channel, Dimm, PhysicalRank, Strobe, Bit, FrequencyIndex, IoLevel, Element, RD_REG, &CurrentValue);
          NewValue = CurrentValue + *Value;
        }
        //
        // Convert to physical in preparation for writing the register.
        //
        NewValue = ConvertLinearToPhysicalValue (MrcData, Element, NewValue);
        //
        // If it is a write, bounds check the Value first.
        //
        Status = GetDdrioGroupLimits (MrcData, Socket, Channel, Dimm, IoLevel, Type, &MaxLimit, &WaitTime);
        if ((Status != MMRC_SUCCESS) || (NewValue > MaxLimit) ) {
          MmrcDebugPrint ((
            MMRC_DBG_MIN,
            "0x%x > limit of 0x%x for %s. Returning STATUS_LIMIT\n",
            NewValue,
            MaxLimit,
            SignalInfo[GetSignalInfoIndex (Type)].Name
            ));
          //
          // At least for now, just deadloop to ease debugging.  MRC spec says to "return STATUS_LIMIT" instead.
          //
          MrcData->CurrentBlueprint = TempBlueprint;
          MrcDeadLoop();
          return STATUS_LIMIT;//MrcSetLimitReached;
        }
        if (Elements[Type].Box_Type == EXTERNAL) {
          //
          // When external functions are used, the offset for all ranks points to the index needed within the ExternalFunctions array.
          //
          j += Elements[Type].RegisterLocation[PhysicalRank].Offset;
          (*(j->Function)) (MrcData, Socket, Channel, Dimm, PhysicalRank, Strobe, Bit, FrequencyIndex, IoLevel, FC_WR, &NewValue);
        } else {
#ifdef SUSSW
          Register.Mask = (((UINTX)1 << (EndingBit + 1)) - 1) - (((UINTX)1 << StartingBit) - 1);
#else
          Register.Mask = (LShiftU64(1, (EndingBit + 1)) - 1) - (LShiftU64(1, StartingBit) - 1);
#endif
          Register.ShiftBit = StartingBit;
          MemRegWrite (Elements[Type].Box_Type, PhysicalChannel, Instance, Register, NewValue, 0xf);
        }
        //
        // If the action has a delay, with the set, then delay between every assignment.
        //
        if (WaitTime != 0) {
#if SIM
          MySimStall (WaitTime);
#else
          MrcDelay (MrcData, NANO_DEL, WaitTime);
#endif
        }
        ///////////////MemRegWrite (DDRIO, PhysicalChannel, ByteOffset, TempValue);
      }
      //
      // If a cache write, then write directly to the trained values array.
      //
      if (Cmd & UPD_CACHE) {
        //
        //  Save Physical value to the Element Cache
        //
        NewValue = ConvertLinearToPhysicalValue (MrcData, Element, NewValue);
        //
        // Update the cache.
        //
        if (Elements[Type].Box_Type != EXTERNAL) {
          GetSetCache (MrcData, Channel, Rank, Strobe, Bit, Elements[Type].CacheIndex, RD_ONLY, &CacheValue);
#ifdef SUSSW
          CacheValue &= ~(
            ((((UINTX)1 << (Elements[Type].RegisterLocation[PhysicalRank].EndingBit + 1)) - 1) -
            (((UINTX)1 << Elements[Type].RegisterLocation[PhysicalRank].StartingBit) - 1))
            );
          CacheValue |= (NewValue << Elements[Type].RegisterLocation[PhysicalRank].StartingBit);
#else
          CacheValue &= ~(
            ((LShiftU64(1, (Elements[Type].RegisterLocation[PhysicalRank].EndingBit + 1)) - 1) -
            (LShiftU64(1, Elements[Type].RegisterLocation[PhysicalRank].StartingBit) - 1))
            );
          CacheValue |= LShiftU64(NewValue, Elements[Type].RegisterLocation[PhysicalRank].StartingBit);
#endif
          GetSetCache (MrcData, Channel, Rank, Strobe, Bit, Elements[Type].CacheIndex, UPD_CACHE, &CacheValue);
        } else {
          GetSetCache (MrcData, Channel, Rank, Strobe, Bit, Elements[Type].CacheIndex, UPD_CACHE, &NewValue);
        }
      }
    }
  } else if (Type >= SGTI_INDEX) {
    //
    // This is a group signal so need to loop to perform the action on all elements in the group.
    //
    ElementIndex = Type - SGTI_INDEX;
    ThisGroup = 0;
    for (i = 0; i < SignalInfo[ElementIndex].NumSignalGroups; i++)
    {
      GetSetDdrioGroup (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel,
        Type - (SGTI_INDEX - ALGO_REG_INDEX) + ThisGroup, Cmd, Value
        );
      ThisGroup += SignalInfo[ElementIndex].FrequencyOffset;
    }
  } else {
    NewValue = *Value;
    //
    // When accessing the algorithm,  do a recursive call back to the Get/Set but for the individual elements.
    // For a force write, decode the value into the registers.
    //
    if (Cmd & FC_WR) {
      //
      // If the value passed in is an offset from the current value, read the current value first, then
      // apply the offset.
      //
      if (Cmd & WR_OFF) {
        GetLinearFromRegs (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, Type, RD_REG, &CurrentValue);
        NewValue = CurrentValue + NewValue;
      }
      //
      // Now that we have computed the value with the offset, write the total value to the register
      // by disabling the WR_OFF flag.
      //
      SetRegsToLinear (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, Type, Cmd & ~WR_OFF, &NewValue);
    }
    //
    // If a read, then read the registers (only the elements, not cc) and return the linear value.
    //
    if (Cmd & RD_REG) {
      GetLinearFromRegs (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, Type, Cmd, Value);
      NewValue = *Value;
    }
    //
    // Make sure the frequency index passed in is valid.
    //
    if ((Type - ALGO_REG_INDEX) < MAX_SIGNAL_INFO_ELEMENTS) {
      if (FrequencyIndex < SignalInfo[Type - ALGO_REG_INDEX].FrequencyOffset) {
        //
        // If a cache read, then read directly from the trained values array.
        //
        if ((Cmd & RD_ONLY) != 0 && (Cmd & RD_REG) == 0) {
          *Value = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Trained_Value.Values[MrcData->CurrentBlueprint][(Type - ALGO_REG_INDEX) + FrequencyIndex][Rank][Strobe];
        }
        //
        // If a cache write, then write directly to the trained values array.
        //
        if (Cmd & UPD_CACHE) {
          MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Trained_Value.Values[MrcData->CurrentBlueprint][(Type - ALGO_REG_INDEX) + FrequencyIndex][Rank][Strobe] = (UINT16) NewValue;
        }
      } else {
        MrcData->CurrentBlueprint = TempBlueprint;
        return STATUS_FAIL;//MrcFrequencyIndexNotSupported;
      }
    }
  }
#endif
  MrcData->CurrentBlueprint = TempBlueprint;
  }
  return MMRC_SUCCESS;
}

/**
  Based on the linear delay element specified, it will set the appropriate
  delay and clock crossing elements to form the linear delay.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Socket          Current Socket being examined.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Dimm            Current Dimm being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Strobe          Strobe selection for the requested field.
  @param[in]       Bit             Bit within field being examined.
  @param[in]       FrequencyIndex  Frequency Index.
  @param[in]       IoLevel         I/O Level.
  @param[in]       Type            Register to be accessed,
                                   this is pointer to an algorithm.
  @param[in]       Cmd             Command to read/write from register/cache.
  @param[in, out]  Value           Value set/retrieved for the
                                   specified Type.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
SetRegsToLinear (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT16        Type,
  IN        UINT8         Cmd,
  IN  OUT   UINT32        *Value
  )
{
  UINT8  ElementIndex;          // Index to the Delay/CC being operated upon.
  UINT32 CurrentValue;          // Remaining Linear value to be programmed.
  UINT32 TempValue;             // Temporary variable used throughout the function.
  UINT32 ElementValue;          // Value to be programmed into the specific element.
  UINT16 AlgoElementsStart;     // Starting position for the Delay/CC within Elements[].
  UINT16 CCValue;
  UINT16 CCRange;
  UINT8  ThisCcRange;
  //
  // Initialize variable.
  //
  ElementValue  = 0;
  CCValue       = 0;
  CCRange       = 0;
  //
  // Compute the starting index within the Element[] array for the elements used to assign the linear value.
  //
  AlgoElementsStart = ((Type - ALGO_REG_INDEX)) * NumberOfElementsPerAlgo;
  //
  // CurrentValue will contain the remaining linear delay needed to write to the elements.  Initially it
  // should always be the assigned value.
  //
  CurrentValue = *Value;

  for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
    //
    // For all delay elements, except the PI, the actual value programmed must be based on the Granularity of the element.
    // The PI value, which is the last element of the assignment (MAX_NUM_DELAY_ELEMENTS - 1) always has a granularity of 1.
    // ElementValue = Actual element to program.
    // TempValue is the linear value of the element programming, for example 2x=2, TempValue = 128...
    //
    ElementValue = CurrentValue / (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]);
    TempValue = ElementValue * (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]);
    //
    // We cannot set any delay larger than the 2x clock. If the delay element is larger granularity, skip it.
    //
    if (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex] > GetHalfClk (MrcData, Channel))
    {
      continue;
    }
    //
    // Attempt to program the element, if successful the subtract the linear value from the total to get
    // the remainder.
    //
    if (GetSetDdrioGroup (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, (UINT8)(AlgoElementsStart+ElementIndex), Cmd, &ElementValue) == MMRC_SUCCESS) {
      if (ElementIndex == NumberOfDelayElementsPerAlgo - NumberOfDependantElements) {
        CCValue = (UINT16) CurrentValue;
        CCRange = GetHalfClk (MrcData, Channel);
      }
      CurrentValue -= TempValue;
    }
  } // ElementIndex loop ...

  //
  // Set the starting index to the beginning of the Clock Crossings.
  //
  AlgoElementsStart += NumberOfDelayElementsPerAlgo;

  for (ElementIndex = 0; ElementIndex < NumberOfCCElements; ElementIndex++) {
    TempValue = 0;
    //
    // Check if the value of the PI was within the min/max percentage for that clock crossing.  If it
    // is then set the CC to the "invalue"
    //
    for (ThisCcRange = 0; ThisCcRange < NumberOfCCRanges; ThisCcRange++) {
      if ((ElementIndex == ClockCrossings(ThisCcRange).CcNumber) &&
        (CCValue <  (UINT32) (CCRange * ClockCrossings(ThisCcRange).MaxPercent / 100)) &&
        (CCValue >= (UINT32) (CCRange * ClockCrossings(ThisCcRange).MinPercent / 100))) {
        TempValue = (UINT32) ClockCrossings(ThisCcRange).InValue;
        break;
      }
    }
    //
    // Program the value to the Clock Crossing.
    //
    if (ClockCrossings(ThisCcRange).IsAbsolute) {
      GetSetDdrioGroup (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, (UINT8)(AlgoElementsStart + ElementIndex), Cmd, &TempValue);
    } else {
      //
      // If IsAbsolute is FALSE, we need to add the value of the CC to the value in the register.
      //`
      GetSetDdrioGroup (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, (UINT8)(AlgoElementsStart + ElementIndex), Cmd | WR_OFF, &TempValue);
    }
  }

  return MMRC_SUCCESS;
}

/**
   Based on the linear delay element specified, compute the
   linear delay.  The linear delay should include the support
   for analog/digital dll.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Socket          Current Socket being examined.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Dimm            Current Dimm being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Strobe          Strobe selection for the requested field.
  @param[in]       Bit             Bit within field being examined.
  @param[in]       FrequencyIndex  Frequency Index.
  @param[in]       IoLevel         I/O Level.
  @param[in]       Type            Register to be accessed, this is pointer to
                                   an algorithm.
  @param[in]       Cmd             Command to read/write from register/cache.
  @param[out]      Value           Value set/retrieved.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GetLinearFromRegs (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Socket,
  IN        UINT8         Channel,
  IN        UINT8         Dimm,
  IN        UINT8         Rank,
  IN        UINT8         Strobe,
  IN        UINT8         Bit,
  IN        UINT8         FrequencyIndex,
  IN        UINT8         IoLevel,
  IN        UINT16        Type,
  IN        UINT8         Cmd,
      OUT   UINT32        *Value
  )
{
#if USE_GET_SET_FROM_SPREADSHEET
  UINT8  ElementIndex;        // Index to the Delay/CC being operated upon.
  UINT32 TempValue;           // Temporary variable used throughout the function.
  UINT16 AlgoElementsStart;   // Starting position for the Delay/CC within Elements[].
  UINT16 CCValue;
  UINT16 CCRange;
  UINT8  ThisCcRange;

  //
  // Initialize the value to 0, this is the value that will get returned.
  //
  *Value = 0;
  CCValue = 0;
  TempValue = 0;
  //
  // Compute the starting index within the Element[] array for the elements used to assign the linear value.
  //
  AlgoElementsStart = (Type - ALGO_REG_INDEX) * NumberOfElementsPerAlgo;

  for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
    //
    // We cannot set any delay larger than the 2x clock. If the delay element is larger granularity, skip it.
    //
    if (MrcData->DigitalDll < 2) {
      if (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex] > GetHalfClk (MrcData, Channel)) {
        continue;
      }
    }
    //
    // Attempt to read the delay element, if successful accumulate to the value variable.
    // PI element will be incremented by the read value * 1, where all the other elements will
    // be multiplied by the granularity.
    //
    if (GetSetDdrioGroup (MrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, FrequencyIndex, IoLevel, AlgoElementsStart + ElementIndex, Cmd, &TempValue) == MMRC_SUCCESS) {
      if ((MrcData->CurrentBlueprint < MAX_BLUEPRINTS) && (MrcData->DigitalDll < 2)) {
        *Value += TempValue * (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]);
      }
    }
    if (ElementIndex >= NumberOfDelayElementsPerAlgo - NumberOfDependantElements) {
      if ((MrcData->CurrentBlueprint < MAX_BLUEPRINTS) && (MrcData->DigitalDll < 2)) {
        CCValue += (UINT16) TempValue * (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]);
      }
    }
  } // ElementIndex loop ...

  CCRange = GetHalfClk (MrcData, Channel);
  //
  // Set to the first clock crossing.
  //
  AlgoElementsStart += NumberOfDelayElementsPerAlgo;

  for (ElementIndex = 0; ElementIndex < NumberOfCCElements; ElementIndex++) {
    //
    // If this clock crossing is modifying a delay register (such as the 2x clock), then we need to
    // undo the clock crossing modification when returning the total linear value. For example, if 2x clock
    // the 7 and the clock crossing modifies this to be 6, then we need to return it back to 7 when
    // calculating the final linear value. GranularityValid == 1 is the flag for this condition.
    //
    if (Elements[AlgoElementsStart + ElementIndex].ElementAttributes.Bits.GranularityValid == 1) {
      //
      // Check if the value of the PI was within the min/max percentage for that clock crossing.  If it
      // is then set the CC to the "invalue"
      //
      for (ThisCcRange = 0; ThisCcRange < NumberOfCCRanges; ThisCcRange++) {
        if ((ElementIndex == ClockCrossings(ThisCcRange).CcNumber) &&
          (CCValue <  (UINT32) (CCRange * ClockCrossings(ThisCcRange).MaxPercent / 100)) &&
          (CCValue >= (UINT32) (CCRange * ClockCrossings(ThisCcRange).MinPercent / 100))) {
            TempValue = (UINT32) ClockCrossings(ThisCcRange).InValue;
            break;
        }
      }
      *Value -= TempValue * (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][Elements[AlgoElementsStart + ElementIndex].ElementAttributes.Bits.GranularityValid]);
    }
  } // ElementIndex loop ...
#endif
  return MMRC_SUCCESS;
}

/**
  Based on the Type input, returns the maximum possible value and the wait time
  required after, if necessary, chaining the value.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Socket          Current Socket being examined.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Dimm            Current Dimm being examined.
  @param[in]       Level           I/O Level.
  @param[in]       Type            Register to be accessed, this is pointer to
                                   an algorithm.
  @param[out]      MaxLimit        Maximum value allowed on this signal group.
  @param[out]      WaitTime        Amount of time in uSec to wait after programming
                                   a new value.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GetDdrioGroupLimits (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8          Socket,
  IN        UINT8          Channel,
  IN        UINT8          Dimm,
  IN        UINT8          Level,
  IN        UINT16         Type,
      OUT   UINT32        *MaxLimit,
      OUT   UINT32        *WaitTime
  )
{
#if USE_GET_SET_FROM_SPREADSHEET
  UINT8   ElementIndex;        // Index to the Delay/CC being operated upon.
  UINT32  TempLimit;           // Temporary variable used throughout the function.
  UINT16  AlgoElementsStart;   // Starting position for the Delay/CC within Elements[].

  if (Type < ALGO_REG_INDEX) {
    if (Type < MAX_NUM_ELEMENTS) {
      if (Elements[Type].CacheIndex != 0xFF) {
        *WaitTime = Elements[Type].WaitTime;
        *MaxLimit = Elements[Type].MaxLimit;
      } else {
        *WaitTime = 0xFFFFFFFF;
        *MaxLimit = 0xFFFFFFFF;
      }
    }
  } else {
    //
    // Initialize the values to 0, these are the values that will get returned.
    //
    *WaitTime = 0;
    *MaxLimit = 0;

    //
    // Compute the starting index within the Elements[] array for the elements used to assign the linear value.
    //
    AlgoElementsStart = (Type - ALGO_REG_INDEX) * NumberOfElementsPerAlgo;

    for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
      //
      // Wait time is straight addition.  Limits are computed with:
      // PI element will be incremented its limit * 1, where all the other elements will
      // be multiplied by the granularity.
      //
      if ( (AlgoElementsStart + ElementIndex) < MAX_NUM_ELEMENTS) {
        if (Elements[AlgoElementsStart + ElementIndex].CacheIndex != 0xFF) {
          *WaitTime += Elements[AlgoElementsStart + ElementIndex].WaitTime;
          TempLimit  = Elements[AlgoElementsStart + ElementIndex].MaxLimit;
          *MaxLimit += TempLimit * MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex];
        }
      }
    } // ElementIndex loop ...
  } // if (Type < ALGO_REG_INDEX)
#endif
  return MMRC_SUCCESS;
}

/**
   Retrieves the Byteoffset, starting, and ending bit for a given channel, rank and strobe.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Element         Information on the description and flags of the desired field.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Strobe          Strobe selection for the requested field.
  @param[in]       Bit             Bit within field being examined.
  @param[out]      Instance        Instance being specified when looping on total Instances.
  @param[out]      ByteOffset      Final ByteOffset based on Channel/Rank/Strobe.
  @param[out]      StartingBit     Final Starting bit for the field based on Channel/Rank/Strobe.
  @param[out]      EndingBit       Final Ending bit for the field based on Channel/Rank/Strobe.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GetLocation (
  IN  OUT MMRC_DATA         *MrcData,
  IN      GET_SET_ELEMENTS  Element,
  IN      UINT8             Channel,
  IN      UINT8             Rank,
  IN      UINT8             Strobe,
  IN      UINT8             Bit,
  OUT     UINT8             *Instance,
  OUT     UINT32            *ByteOffset,
  OUT     UINT8             *StartingBit,
  OUT     UINT8             *EndingBit
  )
{
  //
  // Calculate Final ByteOffset, Starting bit, and Ending bit. If RankToRegister or StrobeToRegister tables are not to be used.
  //
  if (Element.ElementAttributes.Bits.StrobeLookupTable == 0 && Element.ElementAttributes.Bits.RankLookupTable == 0) {
    //
    // The initial ByteOffset, starting, and ending bits are taken from the Element[] array except
    // when the unique bit is set, which it will be taken from the URank array but only for ranks above 0.
    //
    *ByteOffset += (Element.RegisterLocation[Rank].Offset     + Element.SPMByteOffset * (Strobe % MAX_BYTELANES_PER_DQ_MODULE));
    *StartingBit = Element.RegisterLocation[Rank].StartingBit + Element.SPMBitOffset  * (Strobe % MAX_BYTELANES_PER_DQ_MODULE);
    *EndingBit   = Element.RegisterLocation[Rank].EndingBit   + Element.SPMBitOffset  * (Strobe % MAX_BYTELANES_PER_DQ_MODULE);
    //
    // Per-Nibble Handling.
    //  Strobes above (MAX_STROBES / 2) are nibbles.
    //
    if (MAX_STROBES > 9 && (Strobe >= (MAX_STROBES / 2))) {
      //
      // Add Nibble Byte/Bit Offsets
      //
      *ByteOffset  += Element.NbSPMByteOffset;
      *StartingBit += Element.NbSPMBitOffset;
      *EndingBit   += Element.NbSPMBitOffset;
      //
      // If the number of instances per channel > 1, then Instance = Strobelane/2; otherwise, instance = 0.
      //
      if (InstancePortMap[Element.Box_Type].Instances > 1) {
        //
        // Recalculate the Instance based on how many nibbles are handled withing a DQ module.
        //
        *Instance = Strobe - (MAX_STROBES / 2);
      }
    }
  } else {
    //
    // Use StrobeToRegister and RankToRegister Tables to get RegisterOffset and BoxType Offset.
    //
    *ByteOffset += Element.RegisterLocation[Rank].Offset;
    *StartingBit = Element.RegisterLocation[Rank].StartingBit;
    *EndingBit   = Element.RegisterLocation[Rank].EndingBit;
    *Instance    = 0;

    if (Element.ElementAttributes.Bits.StrobeLookupTable == 1) {
      *Instance += (UINT8) StrobeToRegister[Strobe][0];
      *ByteOffset += StrobeToRegister[Strobe][1];
    }
    if (Element.ElementAttributes.Bits.RankLookupTable == 1) {
      *Instance += (UINT8) RankToRegister[Rank][0];
      *ByteOffset += RankToRegister[Rank][1];
    }
  }
  //
  // Per-Bit Handling
  //
  if (Element.ElementAttributes.Bits.BitBased == 1) {
    *ByteOffset += Element.BitSPMByteOffset * Bit;
    *StartingBit += Bit * Element.BitSPMBitOffset;
    *EndingBit += Bit * Element.BitSPMBitOffset;
    //
    // Reached the next Register, calculate next ByteOffset and reset Bit count.
    //
    if (Bit * Element.BitSPMBitOffset >= 32) {
      *ByteOffset += ((Bit * Element.BitSPMBitOffset) / 32) * 4;
      Bit %= (32 / Element.BitSPMBitOffset);
    }
  }
  return MMRC_SUCCESS;
}

/**
  Resets the FIFOs in the PHY.  Its a simple routine but is called many times.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PerformFifoReset (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank
  )
{
#if USE_GET_SET_FROM_SPREADSHEET
  UINT8  Strobe;
  UINT32 TempValue;

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
#ifdef DQS_CNT_RESET_Ecc
    if (Strobe < MAX_STROBES_NON_ECC) {
#endif
    TempValue = FIFO_RESET_ENABLE;
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, DQS_CNT_RESET, CMD_SET_VAL_FC, &TempValue);
    MrcDelay (MrcData, MICRO_DEL, 10);
    TempValue = FIFO_RESET_DISABLE;
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, DQS_CNT_RESET, CMD_SET_VAL_FC, &TempValue);

    TempValue = FIFO_RESET_ENABLE;
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, FifoReset, CMD_SET_VAL_FC, &TempValue);
    MrcDelay (MrcData, MICRO_DEL, 10);
    TempValue = FIFO_RESET_DISABLE;
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, FifoReset, CMD_SET_VAL_FC, &TempValue);

    TempValue = FIFO_RESET_ENABLE;
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, WrPtrEn, CMD_SET_VAL_FC, &TempValue);
    MrcDelay (MrcData, MICRO_DEL, 10);
    TempValue = FIFO_RESET_DISABLE;
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, WrPtrEn, CMD_SET_VAL_FC, &TempValue);
#ifdef DQS_CNT_RESET_Ecc
    } else {
      //
      // For these GetSet types, there is no Rank1 register. Only call these GetSets with
      // Rank=0 as an argument:
      // - DQS_CNT_RESET_Ecc
      // - FifoReset_Ecc
      //
      TempValue = FIFO_RESET_ENABLE;
      GetSetDdrioGroup2 (MrcData, Channel,  0, 0, DQS_CNT_RESET_Ecc, CMD_SET_VAL_FC, &TempValue);
      MrcDelay (MrcData, MICRO_DEL, 10);
      TempValue = FIFO_RESET_DISABLE;
      GetSetDdrioGroup2 (MrcData, Channel,  0, 0, DQS_CNT_RESET_Ecc, CMD_SET_VAL_FC, &TempValue);
      TempValue = FIFO_RESET_ENABLE;
      GetSetDdrioGroup2 (MrcData, Channel,  0, 0, FifoReset_Ecc, CMD_SET_VAL_FC, &TempValue);
      MrcDelay (MrcData, MICRO_DEL, 10);
      TempValue = FIFO_RESET_DISABLE;
      GetSetDdrioGroup2 (MrcData, Channel,  0, 0, FifoReset_Ecc, CMD_SET_VAL_FC, &TempValue);
    }
#endif
  }
#endif
  return MMRC_SUCCESS;
}

#if TRAINING_ALGOS == 1


MMRC_STATUS
BytelaneMask (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT32        *Mask
  )
{
#if CPGC_API
  *Mask = 0x01;
#else
  *Mask = 0x00;
  do {
    *Mask <<= BUSWIDTH;
    *Mask |= 0x00000001;
  } while (*Mask < 0x00010000);
  *Mask &= 0xffff;
#endif

  return MMRC_SUCCESS;
}

/**
  Based on the input type, this function will average out the 2x
  values for all ranks on the given channel.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Index           Selection of signal group to be averaged.
  @param[in]       Minus1Index     Minus1 signal.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
Average (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT16        Index,
  IN        UINT16        Minus1Index
  )
{
  UINT8   Rank;
  UINT8   NumberOfRanks;
  UINT8   Strobe;
  UINT32  TempValue;
  UINT32  Average;

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, 0, Strobe)) {
      continue;
    }
    Average  = 0;
    NumberOfRanks = 0;
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      NumberOfRanks++;
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Index, CMD_GET_CACHE, &TempValue);
      Average += TempValue;
    }
    if (NumberOfRanks == 0) {
      continue;
    }
    Average = Average / NumberOfRanks;

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Index, CMD_SET_VAL_FC_UC, &Average);
    }
  } // for Strobe

  return MMRC_SUCCESS;
}

#endif // TRAINING_ALGOS == 1

/**
  Switch the writeleveling mode on the DDRIO Physical Layer

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       WriteLevelMode  Write Leveling mode on or off
**/
MMRC_STATUS
WriteLevelingModeAllChannels (
  MMRC_DATA         *MrcData,
  UINT8             WriteLevelMode
)
{
#if FINE_WRITE_LEVELING == 1 && TRAINING_ALGOS == 1
//
// Fix for Fine Write Leveling issues on Elpida QDP (etc..)
//   Some of the PHYENTRY WriteLeveling register bits are shared across channels , for LP3 & LP4.
//
  UINT8 Channel;

  for (Channel = 0; Channel < MAX_CHANNELS; Channel ++ ) {
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled == TRUE) {
    if (WriteLevelMode != PHYEXITWRLVL && WriteLevelMode != PHYENTERWRLVL) {
      return MMRC_FAILURE;
    }
    MrcData->DynamicVars[Channel][WRLVLMODEVAR] = WriteLevelMode;
    MmrcExecuteTask (MrcData, WRLVLMODE, NO_PRINT, Channel);
  }
}
#endif

  return MMRC_SUCCESS;
}

/**
  Set the Drams in or out of fine write leveling mode (MRS operation)
  specific DDR technologies (LPDDR, DDR3, etc) can add here their specific
  configurations here

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Entry           Parameter to select Entry or Exit hook to execute.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
FineWriteLevelingDramEntryExit (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT8             Channel,
  IN      UINT8             Rank,
  IN      UINT8             Entry
  )
{
#if FINE_WRITE_LEVELING == 1 && TRAINING_ALGOS == 1
  UINT8             ConfigureRank;              // Flag stating which rank is currently being configured.
#if DDR3_SUPPORT
  DramInitDDR3EMR1  Mrs1Command;
  DramInitDDR3EMR2  Mrs2Command;
#endif
#if DDR4_SUPPORT
  DramInitDDR4EMR1  Ddr4Mrs1Command;
  DramInitDDR4EMR2  Ddr4Mrs2Command;
#endif
  ConfigureRank = 0;
  if (Entry == MMRC_ENTRY) {
    //
    // MMRC_ENTRY Set the Drams in Write Leveling mode
    //
#if LPDDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
      PrechargeAll (MrcData, Channel, Rank);
      MySimStall (500);
      MySimStall (500);
      MrcDelay (MrcData, NANO_DEL, 100);
      MrcData->DynamicVars[Channel][RANK]        = Rank;
      MrcData->DynamicVars[Channel][REG_DATA] = MrcData->MR2_Value | LPDDR3_WL_ENABLE;//MrcData->DynamicVars[Channel][MrsCommandIndex[Rank][2]] | LPDDR3_WL_ENABLE;
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      MySimStall (500);         // Required for DRAM CMD message to get received prior to issuing any WL sequences.
      MySimStall (500);
      MrcDelay (MrcData, NANO_DEL, 100);
    }
#endif // LPDDR3_SUPPORT
#if LPDDR4_SUPPORT
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
          PrechargeAll (MrcData, Channel, Rank);
          MySimStall (500);
          MySimStall (500);
          MrcDelay (MrcData, NANO_DEL, 100);
          MrcData->DynamicVars[Channel][RANK]        = Rank;
          MrcData->DynamicVars[Channel][REG_DATA]    = MrcData->MR2_Value | BIT5;//MRW_LP4(2, LPDDR4_WL_ENABLE | (MrcData->WL << 3) | (MrcData->RL << 0));
          SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
          MySimStall (500);     // Required for DRAM CMD message to get received prior to issuing any WL sequences.
          MySimStall (500);
          MrcDelay (MrcData, NANO_DEL, 100);
        }
#endif // LPDDR4_SUPPORT
#if DDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All) {
      for (ConfigureRank = 0; ConfigureRank < MAX_RANKS; ConfigureRank++) {
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[ConfigureRank]) {
          continue;
        }

        Mrs1Command.Data = MrcData->MR1_Value;
        Mrs2Command.Data = MrcData->MR2_Value;

        if (Rank == ConfigureRank) {
          if (Mrs2Command.Bits.rtt_WR) {
            Mrs1Command.Bits.rttNom0 = 0;
            Mrs1Command.Bits.rttNom1 = 0;
            Mrs1Command.Bits.rttNom2 = 0;
            if (Mrs2Command.Bits.rtt_WR == DDR3_MRS2_RTTWR_60) {
              //
              // Set RTT NOM as RZQ/4 = 60 Ohms
              //
              Mrs1Command.Bits.rttNom0 = 1;
              Mrs1Command.Bits.rttNom1 = 0;
              Mrs1Command.Bits.rttNom2 = 0;
            } else if (Mrs2Command.Bits.rtt_WR == DDR3_MRS2_RTTWR_120) {
              //
              // Set RTT NOM as RZQ/2 = 120 Ohms
              //
              Mrs1Command.Bits.rttNom0 = 0;
              Mrs1Command.Bits.rttNom1 = 1;
              Mrs1Command.Bits.rttNom2 = 0;
            }
          }
          Mrs1Command.Bits.wlEnabled = 1;
        } else {
          //
          //  Turn off output of non-target ranks
          //
          Mrs1Command.Bits.Qoff = 1;
        }
        PrechargeAll (MrcData, Channel, ConfigureRank);
        WriteMrsCommand (MrcData, Channel, ConfigureRank, Mrs1Command.Data);
        MySimStall (500);
        MySimStall (500);
        MySimStall (500);
      }
    }
#endif // DDR3_SUPPORT

#if DDR4_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr4) {
      for (ConfigureRank = 0; ConfigureRank < MAX_RANKS; ConfigureRank++) {
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[ConfigureRank]) {
          continue;
        }
        Ddr4Mrs1Command.Data = ReadMrsCommand (MrcData, Channel, ConfigureRank, 1);
        Ddr4Mrs2Command.Data = ReadMrsCommand (MrcData, Channel, ConfigureRank, 2);
        if (Rank == ConfigureRank) {
          Ddr4Mrs1Command.Bits.wlEnabled = 1;
          Ddr4Mrs2Command.Bits.RTT_WR = 0;
        } else {
          Ddr4Mrs1Command.Bits.Qoff = 1;
        }
        PrechargeAll (MrcData, Channel, ConfigureRank);
        WriteMrsCommand (MrcData, Channel, ConfigureRank, Ddr4Mrs2Command.Data);
        WriteMrsCommand (MrcData, Channel, ConfigureRank, Ddr4Mrs1Command.Data);
      }
    }
#endif // DDR4_SUPPORT
  } else {
    //
    // MMRC_EXIT Set the Drams out of Write Leveling mode
    //
#if DDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
      for (Rank=0; Rank < MAX_RANKS; Rank++) {
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
          continue;
        }
        MrcData->DynamicVars[Channel][RANK] = Rank;
        MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
        MrcData->DynamicVars[Channel][REG_DATA] = MrcData->MR1_Value;
        SendDramCmd(MrcData, Channel);
        MySimStall(500);MySimStall(500);MySimStall(500);MySimStall(500);
        MrcDelay (MrcData, MICRO_DEL, 1);
      }
      PrechargeAll (MrcData, Channel, ConfigureRank);
    }
#endif // LPDDR3_SUPPORT
#if LPDDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
      MrcData->DynamicVars[Channel][RANK] = Rank;
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
      MrcData->DynamicVars[Channel][REG_DATA] = MrcData->MR2_Value;//MrcData->DynamicVars[Channel][MrsCommandIndex[Rank][2]];
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);

      MySimStall(500);MySimStall(500);MySimStall(500);MySimStall(500);
      MrcDelay (MrcData, MICRO_DEL, 1);
      PrechargeAll (MrcData, Channel, ConfigureRank);
    }
#endif // LPDDR3_SUPPORT

#if LPDDR4_SUPPORT
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
          MrcData->DynamicVars[Channel][RANK] = Rank;
          MrcData->DynamicVars[Channel][RDDCMD]      = 0;        // This is a MRR command.
          MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;        // LP4 requires Burst 16.
          MrcData->DynamicVars[Channel][REG_DATA]    = MrcData->MR2_Value;//MrcData->DynamicVars[Channel][MR2_0];//MRW_LP4(2, LPDDR4_WL_ENABLE | (MrcData->WL << 3) | (MrcData->RL << 0));     // MRR32
          SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
          MySimStall(500);MySimStall(500);MySimStall(500);MySimStall(500);
          MrcDelay (MrcData, MICRO_DEL, 5);
          PrechargeAll (MrcData, Channel, Rank);

        }
#endif // LPDDR4_SUPPORT
#if DDR4_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr4) {
      for (ConfigureRank = 0; ConfigureRank < MAX_RANKS; ConfigureRank++) {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[ConfigureRank]) {
          Ddr4Mrs1Command.Data = ReadMrsCommand (MrcData, Channel, ConfigureRank, 1);
          Ddr4Mrs2Command.Data = ReadMrsCommand (MrcData, Channel, ConfigureRank, 2);
          WriteMrsCommand (MrcData, Channel, ConfigureRank, Ddr4Mrs1Command.Data);
          WriteMrsCommand (MrcData, Channel, ConfigureRank, Ddr4Mrs2Command.Data);
        }
      }
    }
#endif // DDR4_SUPPORT
  }
#endif
  return MMRC_SUCCESS;
}

/**
  Fine Write Leveling training algorithm

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
FineWriteLeveling (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if FINE_WRITE_LEVELING == 1 && TRAINING_ALGOS == 1
  UINT8             Rank;                       // Current Rank being operated on.
  UINT8             Strobe;                     // Current Strobe being operated on.
  UINT32            TempValue;                  // Temporary place value for Get/Set API.
  UINT8             SmallStep;                  // Small step size.
  UINT8             MediumStep;
  UINT8             LargeStep;
  UINT16            Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT16            Low[MAX_CHANNELS][MAX_STROBES];
  UINT16            High[MAX_CHANNELS][MAX_STROBES];
  UINT16            Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT8             ChannelBackup;
  UINT8             DebugLevel;
  ChannelBackup = Channel;

  SmallStep = FWL_SMALL_STEP;
  MediumStep = FWL_MEDIUM_STEP;
  LargeStep  = FWL_LARGE_STEP;
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].OdtHigh) {
      InitializeJedec_DDR3_Nom60_Wr0(MrcData, CapsuleStartIndex, StringIndex, Channel);
      MmrcDebugPrint((MMRC_DBG_MIN, "ODT High mode \n"));
    }
  }
  DebugLevel = MrcData->MrcDebugMsgLevel;
  //
  // Turn on 1D sweep debug messages if MAX message level is set.
  //
  if ((MrcData->MrcDebugMsgLevel & MMRC_DBG_MAX) == MMRC_DBG_MAX) {
    MrcData->MrcDebugMsgLevel |= MMRC_DBG_DEBUG;
  }
  MrcData->SignalType = WrLvlSmp;

  //
  // Entry Hooks
  //
#ifdef FINEWRITELEVEL_ENTRY
  MmrcExecuteTask(MrcData, FINEWRITELEVEL_ENTRY, NO_PRINT, Channel);
#endif

  MrcData->CpgcOptions.LoopCount = 1;
  MrcData->CpgcOptions.NumBursts = 1;
  L_CpgcSetup(MrcData, Channel, CPGC_CONFIG_EARLYTRAINING);

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel(MrcData, Channel, Rank)) {
      continue;
    }
  MmrcDebugPrint((MMRC_DBG_MIN, "C%02dR%d\n", Channel, Rank));
  //
  // Setup the CPGC engine to to do a single write to an address within the
  // selectable rank.  The engine should be setup for LFSR mode.
  //
  MrcData->CpgcOptions.Rank = Rank;
  MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_WRITE;
  L_CpgcReconfig(MrcData, Channel);
  //
  // Take the DRAM in Write Leveling mode.
  //
  FineWriteLevelingDramEntryExit(MrcData, Channel, Rank, MMRC_ENTRY);
#ifdef FINEWRITELEVEL_SHAPE
  MmrcExecuteTask(MrcData, FINEWRITELEVEL_SHAPE, NO_PRINT, Channel);
#endif //FINEWRITELEVEL_SHAPE
  //
  // Set WDQS to WCLK initially for each strobe. Set all initial values.
  //
  MmrcDebugPrint((MMRC_DBG_MAX, "Initializing WDQS to WCLK for each strobelane.\n"));
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid(MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, CkGrp0, CMD_GET_REG, &TempValue);
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
      Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)TempValue;
    }
    else {
      Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)TempValue + 2 * GetHalfClk(MrcData, Channel);
    }
    Dim1StartPoint[Channel][0][Strobe][HIGH] = Dim1StartPoint[Channel][0][Strobe][LOW] + 4 * GetOneClk(MrcData, Channel) + 1;
    Low[Channel][Strobe] = 0;
    High[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][LOW] + 4 * GetOneClk(MrcData, Channel);
  }
  //
  // Put the phy in write leveling mode.
  //
  WriteLevelingModeAllChannels(MrcData, PHYENTERWRLVL);
  //
  // Issue the ForceODT ON command
  //
  MmrcExecuteTask(MrcData, FORCEODT_ON, NO_PRINT, Channel);
  //
  // Using large steps, determine the nearest rising edge.
  //
  MmrcDebugPrint((MMRC_DBG_MIN, "Detect Rising Edge Large, Step=%d, dithering=%d\n", LargeStep, 0));
  Create1DSweepLastPass(MrcData, Channel, Rank, TxDqsDelay, 1, Dim1StartPoint, Low, High, LargeStep,
    FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 0);
  //
  // Copy the result LOW start to the next starting point for the next sweep.
  //
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)(Results[Channel][0][Strobe][0][LOW]);
  }
  //
  // Starting from the results from the large steps, using medimum steps, narrow down the rising edge.
  //
  MmrcDebugPrint((MMRC_DBG_MIN, "Detect Rising Edge Medium, Step=%d, dithering=%d\n", MediumStep, 0));
  Create1DSweepLastPass(MrcData, Channel, Rank, TxDqsDelay, 1, Dim1StartPoint, Low, High, MediumStep,
    FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 0);
  //
  // Copy the result LOW start to the next starting point for the next sweep.
  //
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)(Results[Channel][0][Strobe][0][LOW]);
  }
  //
  // Starting from the results from the medium steps, using small steps, narrow down the rising edge.
  //
  MmrcDebugPrint((MMRC_DBG_MIN, "Detect Rising Edge Small, Step=%d, dithering=%d\n", SmallStep, 3));
  Create1DSweepLastPass(MrcData, Channel, Rank, TxDqsDelay, 1, Dim1StartPoint, Low, High, SmallStep,
    FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], SampleDqs, Results, "S", 3);

  //
  // Write final FWL values to registers and cache.
  //
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid(MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    TempValue = Results[Channel][0][Strobe][0][LOW];
    GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_SET_VAL_FC_UC, &TempValue);
//#ifdef SIM
    //
    // Tether DQS and DQ based on Memory Type (can be removed when running CWL)
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
      TempValue -= 1*GetQtrClk (MrcData, Channel);
    } else {
      TempValue += GetHalfClk(MrcData, Channel) / 2 + GetHalfClk(MrcData, Channel);
    }
    GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, TxDqDelay, CMD_SET_VAL_FC_UC, &TempValue);
//#endif
    }

    //
    // Take the Ddrio phy out of write leveling mode.
    //
    WriteLevelingModeAllChannels(MrcData, PHYEXITWRLVL);
    //
    // Take the DRAM out of Write Leveling mode.
    //
    FineWriteLevelingDramEntryExit(MrcData, Channel, Rank, MMRC_EXIT);
    //
    // Release ODT
    //
    MmrcExecuteTask(MrcData, FORCEODT_REL, NO_PRINT, Channel);
  } // Rank loop ...
  if (MrcData->Rk2RkEn.TxDqs == 0) {
   // Average(MrcData, Channel, TxDqsDelay, MMRC_NA);
  }
  //
  // Print out or restore the training results.
  //
#ifdef TxDqsDelay_dq
    DisplayOrRestoreTrainingResults(MrcData, Channel, TxDqsDelay_dq, FALSE, MAX_STROBES_NON_ECC);
#endif
#ifdef TxDqsDelay_Ecc
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
      DisplayOrRestoreTrainingResults(MrcData, Channel, TxDqsDelay_Ecc, FALSE, 1);
    }
#else
    DisplayOrRestoreTrainingResults(MrcData, Channel, TxDqsDelay, FALSE, MAX_STROBES);
#endif
  //
  // DDRIO Phy Exit Hooks
  //
#ifdef FINEWRITELEVEL_EXIT
  MmrcExecuteTask(MrcData, FINEWRITELEVEL_EXIT, NO_PRINT, Channel);
#endif
  MrcData->MrcDebugMsgLevel = DebugLevel;

#endif // FINE_WRITE_LEVELING_SUPPORT
  return MMRC_SUCCESS;
}

/**
  Performs Early MPR Read Training. This training must be executed only before CWL in order to set
  safe read timing settings

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
EarlyMprRead (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if EARLY_READ_TRAINING == 1 && TRAINING_ALGOS == 1
  UINT8   Rank;
  UINT8   DebugLevel;
  UINT16 Low[MAX_CHANNELS][MAX_STROBES];
  UINT16 High[MAX_CHANNELS][MAX_STROBES];
  UINT16 Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT16 Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT8  Strobe;
  UINT8  LargeStep;
  UINT8  MediumStep;
  UINT8  SmallStep;
  UINT8  Vref;
  UINT8  DitherVal;
  UINT32 TempValue;

  LargeStep  = ERT_LARGESTEP;
  MediumStep = ERT_MEDIUMSTEP;
  SmallStep  = ERT_SMALLSTEP;
  DitherVal  = ERT_DITHER;

  MrcData->CpgcOptions.RmtRestoreJedecResetType     =  RMT_NO_ACTION;
  DebugLevel = MrcData->MrcDebugMsgLevel;

#if FAULTY_PART_TRACKING
  FaultyPartTrackInit (MrcData, Channel, TRUE);
#endif
  //
  // Early read training entry
  //
  MmrcExecuteTask(MrcData, EARLY_READ_TRAINING_ENTRY, NO_PRINT, Channel);
  //
  // Setup CPGC for Early Read Training.
  //
  MrcData->CpgcOptions.LoopCount = 1;
  MrcData->CpgcOptions.NumBursts = 1;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
#if DDR3_SUPPORT==1
    MrcData->CpgcOptions.Pattern0 = 0xaaaaaaaa;
    Vref       = ERT_VREF_DDR3;
#endif
#if LPDDR4_SUPPORT==1
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    MrcData->CpgcOptions.Pattern0 = 0xaaaaaaaa;
    Vref       = ERT_VREF_LPDDR4;
#endif
#if LPDDR3_SUPPORT==1
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    MrcData->CpgcOptions.Pattern0 = 0x55555555;
    Vref       = ERT_VREF_LPDDR3;
#endif
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Memory type unsupported.\n"));
    ASSERT (DDR3_SUPPORT && LPDDR3_SUPPORT && LPDDR4_SUPPORT);
    return MMRC_FAILURE;
  }

  L_CpgcSetup(MrcData, Channel, CPGC_CONFIG_EARLY_MPR_TRAINING);
  //
  // Loop on all available ranks.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel(MrcData, Channel, Rank)) {
     continue;
    }
    MmrcDebugPrint((MMRC_DBG_MIN, "C%02dR%d\n", Channel, Rank));
    //
    // Setup the CPGC engine to do reads on te current rank.
    // selectable rank.  The engine should be setup for LFSR mode.
    //
    MrcData->CpgcOptions.Rank = Rank;
    MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_READ;
    L_CpgcReconfig(MrcData, Channel);
    //
    // Issue a precharge all.
    //
    PrechargeAll(MrcData, Channel, Rank);
    //
    // Enable the MPR for the rank under training
    //
    ToggleMprMode(MrcData, Channel, Rank, 1);
    PerformFifoReset(MrcData, Channel, Rank);
    if (MrcData->MrcDebugMsgLevel != 0) {
     MrcData->MrcDebugMsgLevel |= MMRC_DBG_DEBUG;
    }
    MmrcDebugPrint((MMRC_DBG_MIN, "Setting RxVref=%d\n", Vref));
    TempValue = Vref;
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
     if (!IsStrobeValid(MrcData, Channel, Rank, Strobe)) {
       continue;
     }
     GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, RxVref, CMD_SET_VAL_FC_UC, &TempValue);
    }
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
     Low[Channel][Strobe] = 0;
     High[Channel][Strobe] = GetHalfClk(MrcData, Channel);
    }
    MmrcDebugPrint((MMRC_DBG_MIN, "Initial RDQS Sweep: [%d, %d]\n", 0, GetHalfClk(MrcData, Channel)));
    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
     Dim1StartPoint[Channel][0][Strobe][LOW] = Low[Channel][Strobe];
     Dim1StartPoint[Channel][0][Strobe][HIGH] = GetHalfClk(MrcData, Channel);
    }

    MmrcDebugPrint((MMRC_DBG_MIN, "Detect Passing Range Large, Step=%d, dithering=%d\n", LargeStep, 0));
    Create1DSweepLastPass(MrcData, Channel, Rank, RxDqsDelay, 1, Dim1StartPoint, Low, High, LargeStep,
     FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], EarlyReadTest, Results, "S", 0);

    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
     Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)Results[Channel][0][Strobe][0][LOW];
     Dim1StartPoint[Channel][0][Strobe][HIGH] = (UINT16)Results[Channel][0][Strobe][0][HIGH];
    }

    MmrcDebugPrint((MMRC_DBG_MIN, "Detect Passing Range Medium, Step=%d, dithering=%d\n", MediumStep, 0));
    Create1DSweepLastPass(MrcData, Channel, Rank, RxDqsDelay, 1, Dim1StartPoint, Low, High, MediumStep,
     FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], EarlyReadTest, Results, "S", 0);

    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
     Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)Results[Channel][0][Strobe][0][LOW];
     Dim1StartPoint[Channel][0][Strobe][HIGH] = (UINT16)Results[Channel][0][Strobe][0][HIGH];
    }

    MmrcDebugPrint((MMRC_DBG_MIN, "Detect Passing Range Small, Step=%d, dithering=%d\n", SmallStep, DitherVal));
    Create1DSweepLastPass(MrcData, Channel, Rank, RxDqsDelay, 1, Dim1StartPoint, Low, High, SmallStep,
     FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], EarlyReadTest, Results, "S", DitherVal);
    //
    // Set the RDQS to the mid point of the range.
    //
    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
     if (!IsStrobeValid(MrcData, Channel, Rank, Strobe)) {
       continue;
     }
     TempValue = (Results[Channel][0][Strobe][0][HIGH] + Results[Channel][0][Strobe][0][LOW]) / 2;
     GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, RxDqsDelay, CMD_SET_VAL_FC_UC, &TempValue);
    }
    MrcData->MrcDebugMsgLevel = DebugLevel;
    //
    // Disable the MPR for the rank under training
    //
    ToggleMprMode(MrcData, Channel, Rank, 0);
  } // Rank loop ...
  if (MrcData->Rk2RkEn.RxDqs == 0) {
    Average(MrcData, Channel, RxDqsDelay, MMRC_NA);
  }
  //
  // Print out or restore the training results.
  //
#ifdef RxDqsPDelay_dq
  DisplayOrRestoreTrainingResults(MrcData, Channel, RxDqsPDelay_dq, TRUE, MAX_STROBES_NON_ECC);
  DisplayOrRestoreTrainingResults(MrcData, Channel, RxDqsNDelay_dq, TRUE, MAX_STROBES_NON_ECC);
#else
  DisplayOrRestoreTrainingResults(MrcData, Channel, RxDqsPDelay, TRUE, MAX_STROBES);
  DisplayOrRestoreTrainingResults(MrcData, Channel, RxDqsNDelay, TRUE, MAX_STROBES);
#endif
#ifdef RxDqsPDelay_Ecc
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    DisplayOrRestoreTrainingResults(MrcData, Channel, RxDqsPDelay_Ecc, FALSE, 1);
    DisplayOrRestoreTrainingResults(MrcData, Channel, RxDqsNDelay_Ecc, FALSE, 1);
  }
#endif
  //
  // Early read training exit
  //
  MmrcExecuteTask(MrcData, EARLY_READ_TRAINING_EXIT, NO_PRINT, Channel);
#endif // EARLY_READ_TRAINING
  return MMRC_SUCCESS;
}

/**
  Performs Coarse Write Leveling.  This is reusing many of the same routines used with the Receive Enable
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
CoarseWriteLeveling (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if COARSE_WRITE_LEVELING == 1 && TRAINING_ALGOS == 1
  UINT8   Rank;
  UINT8   Strobe;
  UINT32  TempValue;
  UINT16  Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT16  Low[MAX_CHANNELS][MAX_STROBES];
  UINT16  High[MAX_CHANNELS][MAX_STROBES];
  UINT16  Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT8   ChannelBackup;
  UINT8   DebugLevel;
  ChannelBackup = Channel;

  MrcData->CpgcOptions.RmtRestoreJedecResetType = RMT_NO_ACTION;

#if FAULTY_PART_TRACKING
  FaultyPartTrackInit (MrcData, Channel, FALSE);
#endif
  DebugLevel = MrcData->MrcDebugMsgLevel;
  //
  // Turn on 1D sweep debug messages if MAX message level is set.
  //
  if ((MrcData->MrcDebugMsgLevel & MMRC_DBG_MAX) == MMRC_DBG_MAX) {
    MrcData->MrcDebugMsgLevel |= MMRC_DBG_DEBUG;
  }
  //
  // CoarseWriteLevel Entry
  //
#ifdef COARSEWRITELEVEL_ENTRY
  MmrcExecuteTask(MrcData, COARSEWRITELEVEL_ENTRY, NO_PRINT, Channel);
#endif

  MrcData->CpgcOptions.LoopCount = 1;
  MrcData->CpgcOptions.NumBursts = 1;
  L_CpgcSetup(MrcData, Channel, CPGC_CONFIG_EARLYTRAINING);
  //
  // Training
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel(MrcData, Channel, Rank)) {
      continue;
    }
    MmrcDebugPrint((MMRC_DBG_MIN, "C%02dR%d\n", Channel, Rank));
    //
    // Setup the CPGC engine to to do full traffic write/reads from an address within the
    // selectable rank.  The engine should be setup for LFSR mode.
    //
    MrcData->CpgcOptions.Rank = Rank;
    MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_WRITE_READ;
    MrcData->CpgcOptions.Pattern0 = 0x0AAF0AAF;
    MrcData->CpgcOptions.Pattern1 = 0x0AAF0AAF;
    L_CpgcReconfig(MrcData, Channel);
 //        MmrcExecuteTask(MrcData, MRC_to_Tap_CWL, NO_PRINT, Channel);

    //
    // Issue a precharge all.
    //
    PrechargeAll(MrcData, Channel, Rank);

    //
    // Perform a FIFO Reset.
    //
    PerformFifoReset(MrcData, Channel, Rank);

    MmrcDebugPrint((MMRC_DBG_MIN, "Pushout DQS by %d clocks\n", CWL_CLK_PUSHOUT));
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid(MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      //
      // Increment the WDQS by CWL_CLK_PUSHOUT, no need to tether the testing algo will do that.
      // We are doing this because the algo when enabled will set the starting point for each strobelane
      // to the same value, but each dqs is unique... The algo will then decrement the current value by 1 clk.
      //
      GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_GET_CACHE, &TempValue);
      TempValue += CWL_CLK_PUSHOUT * GetOneClk(MrcData, Channel);
      GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_SET_VAL_FC_UC, &TempValue);
      //
      // Set the limits for the CLK decrement.  Because the intent is to start at the high end and reduce by a clk until
      // a pass is detected. The algo will skip the low end and start at the high, if the high end passes immediately,
      // it will exit, if not it will decrement until a pass is found.
      //
      Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16)0;
      Dim1StartPoint[Channel][0][Strobe][HIGH] = (UINT16)TempValue;
      Low[Channel][Strobe] = 1;
      High[Channel][Strobe] = (UINT16)TempValue;
    }
    //
    // Find the correct clock setting for writes. When this routine returns, the correct setting
    // will be programmed.
    //
    MmrcDebugPrint((MMRC_DBG_MIN, "Sweep decrementing WDQS by 1 clk until all strobelanes pass.\n", CWL_CLK_PUSHOUT));
    Create1DSweepLastPass(MrcData, Channel, Rank, TxDqsDelay, 1, Dim1StartPoint, Low, High, 2 * GetHalfClk(MrcData, Channel),
      FALSE, TRUE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], CoarseWLTest, Results, "S", 0);

   } // Rank loop ...

   if (MrcData->Rk2RkEn.TxDqs == 0) {
    Average(MrcData, Channel, TxDqsDelay, MMRC_NA);
   }

   if (MrcData->Rk2RkEn.TxDq == 0) {
    Average(MrcData, Channel, TxDqDelay, MMRC_NA);
   }
  //
  // Print out or restore the training results.
  //
 #ifdef TxDqsDelay_dq
  DisplayOrRestoreTrainingResults(MrcData, Channel, TxDqsDelay_dq, FALSE, MAX_STROBES_NON_ECC);
 #endif
 #ifdef TxDqsDelay_Ecc
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    DisplayOrRestoreTrainingResults(MrcData, Channel, TxDqsDelay_Ecc, FALSE, 1);
  }
 #else
  DisplayOrRestoreTrainingResults(MrcData, Channel, TxDqsDelay, FALSE, MAX_STROBES);
 #endif
  // DisplayOrRestoreTrainingResults (MrcData, Channel, TxDqDelay, TRUE);
  //
  // CoarseWriteLevel Exit
  //
#ifdef COARSEWRITELEVEL_EXIT
  MmrcExecuteTask(MrcData, COARSEWRITELEVEL_EXIT, NO_PRINT, Channel);
#endif
  MrcData->MrcDebugMsgLevel = DebugLevel;
#endif // COARSE_WRITE_LEVELING

  return MMRC_SUCCESS;
}
#if 0
BOOLEAN
IsEccStrobe (
  MMRC_DATA *MrcData,
  UINT8      Channel,
  UINT8      Strobe
  )
{
  //
  // If this is an ECC strobe, return true.
  //
  if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccStrobes & (1 << Strobe)) != 0) {
    return TRUE;
  }
  return FALSE;
}
#endif

/**
  Performs RDCMD2RDVLDTraining.

  LP4 (.25x spidclk) algorithm:
  Set rdcmd2rdvld_plus1sel=111
  Sweep rdcmd2rdvld to find smallest passing value . Call this A.
  Set rdcmd2rdvld to A-1.
  Set rdcmd2rdvld_plus1sel=000.  If FAIL, final setting is rdcmd2rdvld=A, rdcmd2rdvld_plus1sel=111. (No pull-in possible)
  Set rdcmd2rdvld_plus1sel=001.  If FAIL, final setting is rdcmd2rdvld=A-1, rdcmd2rdvld_plus1sel=000.  (1 1xclk pull-in)
  Set rdcmd2rdvld_plus1sel=011.  If FAIL, final setting is rdcmd2rdvld=A-1, rdcmd2rdvld_plus1sel=001. (2 1xclk pull-in)
  Final setting is rdcmd2rdvld=A-1, rdcmd2rdvld_plus1sel=011. (3 1xclk pull-in)

  DDR3/LP3 (.5x spidclk) algorithm:
  Set rdcmd2rdvld_plus1sel=111
  Sweep rdcmd2rdvld to find smallest passing value . Call this A.
  Set rdcmd2rdvld to A-1.
  Set rdcmd2rdvld_plus1sel=000.  If FAIL, final setting is rdcmd2rdvld=A, rdcmd2rdvld_plus1sel=111. (No pull-in possible)
  Final setting is rdcmd2rdvld=A-1, rdcmd2rdvld_plus1sel=000. (1 1xclk pull-in)


  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
RDCMD2RDVLDTraining (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
#if RDCMD2RDVLD_TRAINING == 1 && TRAINING_ALGOS == 1
  UINT32 OriginalValue;
  UINT32 NewValue;
  UINT8  TestPass;
  UINT16 PassFail;
  UINT8  Rank;
  UINT8  Strobe;
  UINT16 PassFailOrAcrossRanks;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    return MMRC_SUCCESS;
  }
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    return MMRC_SUCCESS;
  }

#ifdef RDCMD2RDVLD_TRAINING_ENTRY
  MmrcExecuteTask (MrcData, RDCMD2RDVLD_TRAINING_ENTRY, NO_PRINT, Channel);
#endif

  MrcData->CpgcOptions.LoopCount = 8;
  MrcData->CpgcOptions.NumBursts = 8;
  L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_VA);

  //
  // Read the initial RdCmd2DataValid
  // Strobe is independent, and can be set to 0.
  //
  GetSetRDCMD2RDVLDDelay (MrcData, Channel, 0, 0, CMD_GET_REG, &OriginalValue);

  MmrcDebugPrint ((MMRC_DBG_MIN, "Start C:%d %d\n", Channel, OriginalValue));
  NewValue = OriginalValue + 1;

  TestPass = TRUE;
  while (TestPass == TRUE) {
    if (NewValue > 1) {
      NewValue = NewValue - 1;
    } else {
      break;
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "Testing C:%d %d | ", Channel, NewValue));

    for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, 0, Strobe)) {
        continue;
      }
      GetSetRDCMD2RDVLDDelay (MrcData, Channel, 0, Strobe, CMD_SET_VAL_FC_UC, &NewValue);
    }

    PassFailOrAcrossRanks = 0;
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        MrcData->CpgcOptions.Rank = Rank;
        L_CpgcReconfig (MrcData, Channel);
        PerformFifoReset (MrcData, Channel, 0);
        L_CpgcRunTest (MrcData, Channel, &PassFail, (UINT32*)NULL, (UINT32*) NULL, (UINT32*) NULL, (UINT32*) NULL);
        PassFailOrAcrossRanks |= PassFail;
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "%d\n", PassFailOrAcrossRanks));
    if (PassFailOrAcrossRanks) {
        TestPass = FALSE;
    }
  }

  //
  // Add 1 to the new value to get to the last passing case, then add the Guard Band (+1).
  // Add 2 more which will guarantee that we always latch on subsequent SPID CLK for DDR3L & LPDDR3
  //
  NewValue += 4;
  for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, 0, Strobe)) {
      continue;
    }
    GetSetRDCMD2RDVLDDelay (MrcData, Channel, 0, Strobe, CMD_SET_VAL_FC_UC, &NewValue);
  }

  PerformFifoReset(MrcData, Channel, 0);

  MmrcDebugPrint ((MMRC_DBG_MIN, "Final C:%d %d\n", Channel, NewValue));

#ifdef RDCMD2RDVLD_TRAINING_EXIT
  MmrcExecuteTask (MrcData, RDCMD2RDVLD_TRAINING_EXIT, NO_PRINT, Channel);
#endif
#endif //RDCMD2RDVLD_TRAINING == 1 && TRAINING_ALGOS == 1
  return MMRC_SUCCESS;
}


BOOLEAN
IsStrobeValid (
  MMRC_DATA *MrcData,
  UINT8      Channel,
  UINT8      Rank,
  UINT8      Strobe
  )
{
  //
  // Skip this strobe if it's greater than the total number of strobes in the system or
  // if it is an ECC strobe and ECC is disabled.
  //
  if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == FALSE) ||
      (Strobe >= MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank] )) {
    return FALSE;
  } else {
    return TRUE;
  }
}

#if TRAINING_ALGOS

/**
  CPGC routines to begin the CPGC Test, poll for the completion,
  after the test is done return the the status of the test which
  is a bytelane+ecc bit-test.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       NumberElements  Number of elements to test
                                   (i.e. MAX_STROBES or MAX_BITS, etc).
  @param[in]       NumberKnobs     Number of knobs.
  @param[in, out]  PassFail        Array of passing or failing result for every element.
  @retval          MMRC_SUCCESS    If all elements passed.
  @retval         !MMRC_SUCCESS    Otherwise.
**/
MMRC_STATUS
CoarseWLTest (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
)
{
  UINT8   Element;
#if LPDDR3_SUPPORT == 1
  //UINT32  TempValue;
#endif
#if LPDDR4_SUPPORT == 1
  UINT8   Knob;
  UINT32  DQSValue;
  UINT32  DQValue;
  UINT32  PiInPs;
  UINT16  HalfClock;
  UINT8   AllPassFlag;
  UINT32  Center;
  UINT16  Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT16  Dim1Min[MAX_CHANNELS][MAX_STROBES];
  UINT16  Dim1Max[MAX_CHANNELS][MAX_STROBES];
  UINT16  Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
#endif

#if (LPDDR4_SUPPORT == 1) || (DDR3_SUPPORT==1) //|| (LPDDR3_SUPPORT == 1)
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4 || MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All || MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    //
    // Initialize required variables.
    // Note that PiInPs is taking the number of pecoseconds for a 2x and computing the duration of PI by dividing
    // by the halfclock. Added (HalfClock/2) to round.
    //
    HalfClock = GetHalfClk(MrcData, Channel);
    PiInPs = (MrcData->TwoXps + HalfClock/2) / HalfClock;

    //
    // Initialize DQ to DQS + 3/4 clk.
    //
    for (Element = 0; Element < NumberElements; Element++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Element)) {
        continue;
      }
      //
      // Read the current DQS Value
      //
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Element, TxDqsDelay, CMD_GET_REG_UC, &DQSValue);
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4 || MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        DQValue = DQSValue + 3*GetQtrClk (MrcData, Channel);
      } else {
        DQValue = DQSValue - 1*GetQtrClk (MrcData, Channel);
      }
      for (Knob=0; Knob < MAX_KNOBS; Knob++) {
#ifdef SIM
        Dim1Min[Channel][Element]                      = (UINT16) (DQValue - GetQtrClk (MrcData, Channel));
        Dim1Max[Channel][Element]                      = (UINT16) (DQValue + GetQtrClk (MrcData, Channel));
        Dim1StartPoint[Channel][Knob][Element][LOW]    = (UINT16) (DQValue - GetQtrClk (MrcData, Channel));
        Dim1StartPoint[Channel][Knob][Element][HIGH]   = (UINT16) (DQValue + GetQtrClk (MrcData, Channel));
#else
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4 ) {
          Dim1Min[Channel][Element]                      = (UINT16) (DQValue - GetQtrClk (MrcData, Channel) + 200/PiInPs);
          Dim1Max[Channel][Element]                      = (UINT16) (DQValue + GetQtrClk (MrcData, Channel) + 800/PiInPs);
          Dim1StartPoint[Channel][Knob][Element][LOW]    = (UINT16) (DQValue - GetQtrClk (MrcData, Channel) + 200/PiInPs);
          Dim1StartPoint[Channel][Knob][Element][HIGH]   = (UINT16) (DQValue + GetQtrClk (MrcData, Channel) + 800/PiInPs);
        } else {
          Dim1Min[Channel][Element]                      = (UINT16) (DQValue - GetHalfClk (MrcData, Channel));
          Dim1Max[Channel][Element]                      = (UINT16) (DQValue + GetHalfClk (MrcData, Channel));
          Dim1StartPoint[Channel][Knob][Element][LOW]    = (UINT16) (DQValue - GetHalfClk (MrcData, Channel));
          Dim1StartPoint[Channel][Knob][Element][HIGH]   = (UINT16) (DQValue + GetHalfClk (MrcData, Channel));
        }
#endif
       } // for Knob
    } // for Element
    Create1DSweepLastPass (MrcData, Channel, Rank, TxDqDelay, 1, Dim1StartPoint, Dim1Min, Dim1Max, CWL_DQ_STEP, FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], GenericByteMemoryTest, Results, "", 5);
    AllPassFlag = TRUE;
    for (Element=0; Element < NumberElements; Element++) {
      if ((Results[Channel][0][Element][0][HIGH] > Results[Channel][0][Element][0][LOW])  && ((Results[Channel][0][Element][0][HIGH] - Results[Channel][0][Element][0][LOW]) > (GetQtrClk(MrcData, Channel)/4))){
        PassFail[Channel][0][Element][0][CURRENT_SAMPLE] = RESULTS_PASS;
      } else {
        PassFail[Channel][0][Element][0][CURRENT_SAMPLE] = RESULTS_FAIL;
        AllPassFlag = FALSE;
      }
    }
    if (AllPassFlag == TRUE) {
      for (Element=0; Element < NumberElements; Element++) {
        Center = (Results[Channel][0][Element][0][HIGH] + Results[Channel][0][Element][0][LOW]) / 2;
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Element, TxDqDelay, CMD_SET_VAL_FC_UC, &Center);
       }
    }
  } // if LPDDR4
#endif
  //
  // Perform a fifo reset.
  //
  PerformFifoReset (MrcData, Channel, Rank);

  return MMRC_SUCCESS;
}

/**
  CPGC routines to begin the CPGC Test, poll for the completion,
  after the test is done return the the status of the test which
  is a bytelane+ecc bit-test.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       NumberElements  Number of elements to test
                                   (i.e. MAX_STROBES or MAX_BITS, etc).
  @param[in]       NumberKnobs     Number of knobs.
  @param[in, out]  PassFail        Array of passing or failing result for every element.
  @retval          MMRC_SUCCESS    If all elements passed.
  @retval         !MMRC_SUCCESS    Otherwise.
**/
MMRC_STATUS
  CpgcExecuteRmtTest (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
)
{
  UINT8   i;
  UINT8   Knob;
  UINT8   ChannelBackup;
  UINT8   DevWidth;
  UINT32  TempValue;
  UINT32  CurrentValue[MAX_CHANNELS][2]; // Save current margin parameter per group value. Like the cmdgrp0, cmdgrp1 of the CmdAll.
  // The maximum number of group cnt is 2 based on the SignalInfoGenerated[MAX_BLUEPRINTS][MAX_SIGNAL_INFO_ELEMENTS]
  // table in MmrProjectDataGenerated.c

  // variables for the CPGC API call
  UINT16  BytelaneStatus;
  UINT32  LaneErrorStatus_lo;
  UINT32  LaneErrorStatus_hi;
  UINT32  LaneErrorStatus;
  BOOLEAN FindError[MAX_CHANNELS];
  BOOLEAN FindErrorAtAnyChannel = FALSE;
#if RMT_PER_BIT_MARGIN
  UINT8  Bit;
#else
  UINT32  StrobeBitMask;
  UINT8   StrobeStatus;
#endif

  ChannelBackup = Channel;
  //Initialize FindError
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    FindError[Channel] = FALSE;
  }
  MrcData->ExecuteThisRoutineInParallel = FALSE;
  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //
    // Issue a Jedec Reset prior to any test
    //
    if (MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_PIMOVE_JEDEC_RESET) {
      //ReInitializeDRAMs (MrcData, Channel, Rank, 1);
      PerformFifoReset (MrcData, Channel, Rank);
      CompleteJedecInit (MrcData, 0, 0, Channel);
    }
    //
    // Fifo reset to avoid glitch the rx fifo during training
    //
    PerformFifoReset (MrcData, Channel, Rank);
  } // Channel loop ...
  MrcData->ExecuteThisRoutineInParallel = TRUE;

  // Because the MmrcExecuteTask takes care the channel parallel. It has a Channel loop.
  // if the global blinding work, We should be able to use the last enabled channel index.
  // However, because the global blinding did not work. We need to stop/clear/start on
  // all channels
  MmrcExecuteTask (MrcData, CPGC_STOP_CLEAR_ERR, NO_PRINT, ChannelBackup);
  MmrcExecuteTask (MrcData, CPGC_START_TEST, NO_PRINT, ChannelBackup);
  MmrcExecuteTask (MrcData, CPGC_GET_TEST_RESULT, NO_PRINT, ChannelBackup);

  // get error status
  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }

    BytelaneStatus = (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
      BytelaneStatus |= (UINT16)(MrcData->DynamicVars[Channel][ECC_ERR_STAT] << 8);
    }

    LaneErrorStatus_lo = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT_LO];
    LaneErrorStatus_hi = (UINT32) MrcData->DynamicVars[Channel][LANE_ERR_STAT_HI];

    // override the hi 32 bit value
    if(MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType != TypeDdr3L) {
      LaneErrorStatus_hi = 0;
    }

    //// Display CPGC lane error status to the log
    //// NOTE: The NumberElements is 1 for both TxVref and Cmd, but only in Cmd case, the status was only in ByteLaneStatus
    //// Since the Cmd test need JedecReset, I used to distingush Cmd from TxVref
    //if ( ( NumberElements == 1 ) && ( MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_JEDEC_RESET ) ) {
    //  // For Cmd test, the error status only showed in the ByteLaneStatus, replaced the LaneErrorStatus_lo with
    //  // ByteLaneStatus
    //  MmrcDebugPrint ((MMRC_DBG_MIN, "CH%02d lane status: 0x%08x_%08x     ", Channel, 0, BytelaneStatus));
    //} else {
    //  MmrcDebugPrint ((MMRC_DBG_MIN, "CH%02d lane status: 0x%08x_%08x     ", Channel, LaneErrorStatus_hi, LaneErrorStatus_lo));
    //}

#if FAULTY_PART_TRACKING
    if (MrcData->FaultyPartTracking) {
      CpgcSCheckErrors (MrcData, Channel, Rank, NULL, NULL, (UINT32 *)&MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT]);
    }
#endif
    //
    // Populate the array of PassFail with passing or failing values.
    //

    // PT TODO support per bit results back

    Knob = 0;
    if ( ( NumberElements == 1 ) && ( MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_JEDEC_RESET ) ){  // Ron - if Vref/Cmd/etc.. need to report fail if *any* bytelane failed
      // The NumberElements is 1 for the CMD/CTL signal. It came from the PiMarginGoup parameter in SearchRMT()
      //if ( ( LaneErrorStatus_lo == 0 ) && (LaneErrorStatus_hi == 0) ) {
      if ( BytelaneStatus != 0 ) { // L_CpgcRunCadbTest() return status only on the BtyeLaneStatus
#if RMT_PER_BIT_MARGIN
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
          PassFail[Channel][Knob][0][Bit][CURRENT_SAMPLE] = RESULTS_FAIL;
        }
#else
        PassFail[Channel][Knob][0][0][CURRENT_SAMPLE] = RESULTS_FAIL;
#endif
        FindError[Channel] = TRUE;
      }
      else
      {
#if RMT_PER_BIT_MARGIN
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
          PassFail[Channel][Knob][0][Bit][CURRENT_SAMPLE] = RESULTS_PASS;
        }
#else
        PassFail[Channel][Knob][0][0][CURRENT_SAMPLE] = RESULTS_PASS;
#endif
      }
    }
    else
    {
      // Based on the device width to generate a mask to check the pass/fail of the byte group
      DevWidth = MAX_BITS; //0x1 << (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_BusWidth[0] + 2); //  PT Assume dimm slot 0 is always populated. Should use rank to find the dimm index later
      for (i = 0; i < NumberElements; i++) {  // iterate strobe
        if (i < 4) {
          LaneErrorStatus = LaneErrorStatus_lo;
        } else {
          LaneErrorStatus = LaneErrorStatus_hi;
        }
#if RMT_PER_BIT_MARGIN
        for (Bit = 0; Bit < MAX_BITS; Bit++) {
          if (( LaneErrorStatus & (( 0x1 << Bit ) << (DevWidth * (i % 4)))) != 0)  {
            PassFail[Channel][Knob][i][Bit][CURRENT_SAMPLE] = RESULTS_FAIL;
            FindError[Channel] = TRUE;
          } else{
            PassFail[Channel][Knob][i][Bit][CURRENT_SAMPLE] = RESULTS_PASS;
          }
        } // Bit
#else
        StrobeBitMask = (0x1 << DevWidth) - 1;
        if (( LaneErrorStatus & (StrobeBitMask << (DevWidth * (i % 4)))) != 0)  {
          StrobeStatus = RESULTS_FAIL;
          FindError[Channel] = TRUE;
        } else{
          StrobeStatus = RESULTS_PASS;
        }
        PassFail[Channel][Knob][i][0][CURRENT_SAMPLE] = StrobeStatus;
#endif
      } // i
    }
    if (!MrcData->ExecuteThisRoutineInParallel) {
      break;
    }
  } // Channel loop ...

  // Find if any channel has detected error
  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    if (FindError[Channel]) {
      FindErrorAtAnyChannel = TRUE;
      break;
    }
  }

  if ( FindErrorAtAnyChannel ) {
    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }

      for (i = 0; i < MrcData->CpgcOptions.RmtRestoreSignalCount; i++) {
        // save current value.
        // Only save the current value if the margin parameter need to restore PI value. Thus, we are not go to save the
        // TxDqDelay and other DQ timing margin parameters. The TxDqDelay is actually a "group" parameter with TxDqDelay_Actual and enable.
        if (MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_RESTORE_PI_LAST_VALUE) {
          GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->CpgcOptions.RmtRestoreSignalGroupBegin + i, CMD_GET_REG, &CurrentValue[Channel][i]);
        }
        if (MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_RESTORE_PI_CENTER) {
          GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->CpgcOptions.RmtRestoreSignalGroupBegin + i, CMD_GET_CACHE,  &TempValue);
          GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->CpgcOptions.RmtRestoreSignalGroupBegin + i, CMD_SET_VAL_FC, &TempValue);
        }
      }
      PerformFifoReset (MrcData, Channel, Rank);

      if (!MrcData->ExecuteThisRoutineInParallel) {
        break;
      }
    } // Channel
    if (MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_JEDEC_RESET) {
      for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          MmrcDebugPrint ((MMRC_DBG_MAX, "exeTask() !RunOnThisChannel Channel=%d ChBak=%d\n", Channel, ChannelBackup));
          continue;
        }

        // TURN OFF CADB mode, so that JEDEC RESET will work - RTA
        MmrcExecuteTask(MrcData, CPGC_DESELECT_DIS, NO_PRINT, Channel);
        MmrcExecuteTask(MrcData, CPGC_STOP_TEST, NO_PRINT, Channel);

        MmrcDebugPrint ((MMRC_DBG_MIN, "CH%02d JEDEC RESET  ", Channel));
        PerformFifoReset (MrcData, Channel, Rank);
      } // channel

      MrcData->ExecuteThisRoutineInParallel=FALSE;
      CompleteJedecInit(MrcData, 0, 0, Channel);
      MrcData->ExecuteThisRoutineInParallel=TRUE;
    }
    // If we disabled CADB for the JEDEC RESET, then restore CPGC settings if they were modified above
    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      if ((MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_JEDEC_RESET) && (MrcData->CpgcOptions.SetupType==CPGC_CONFIG_CMD_STRESS)) {
        MrcData->ExecuteThisRoutineInParallel=FALSE;
        L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_CMD_STRESS);         // PT L_CpgcSetup() works on the specific channel
        L_CpgcReconfig (MrcData, Channel);        // PT L_CpgcSetup() works on the specific channel
        MrcData->ExecuteThisRoutineInParallel=TRUE;
      }
      if (!MrcData->ExecuteThisRoutineInParallel) {
        break;
      }
    }

    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      if (MrcData->CpgcOptions.RmtRestoreJedecResetType & RMT_FAIL_RESTORE_PI_LAST_VALUE) {
        // restore current value
        for (i = 0; i < MrcData->CpgcOptions.RmtRestoreSignalCount; i++) {
          GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->CpgcOptions.RmtRestoreSignalGroupBegin + i, CMD_SET_VAL_FC, &CurrentValue[Channel][i]);
        }
      }

      PerformFifoReset (MrcData, Channel, Rank);

      if (!MrcData->ExecuteThisRoutineInParallel) {
        break;
      }
    } // Channel loop ...
  } // FindErrorAtAnyChannel
  return MMRC_SUCCESS;
}


#endif // TRAINING_ALGOS

#if POWER_TRAINING == 1
/**
  Executes Read Write Training

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ReadWriteTraining (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  if (MrcData->SettingsPowerKnobs.Execute[ExecuteRead]) {
    ReadTraining (MrcData, CapsuleStartIndex, StringIndex, Channel);
  }
  if (MrcData->SettingsPowerKnobs.Execute[ExecuteWrite]) {
    WriteTraining(MrcData, CapsuleStartIndex, StringIndex, Channel);
  }
  return MMRC_SUCCESS;
}

/**
  Check for the contrains of the power knob under test

  @param[in, out]  MrcData            Host structure for all data related to MMRC.
  @param[in]       Delta              Margin width for the current iteration.
  @param[in]       PreviousDelta      Margin width for the previous iteration.
  @param[in]       Constrains         Specifies the criteria in order to declare a
                                      power knob value as good/bad.
  @param[in]  GoToPreviousValidIndex  Returns a true or false in case the current
                                      iteration broke some constrains, so we indicate
                                      we need to go back to the previous valid power
                                      knob iteration.
  @param[in]       Triggers           Pointer to a set of different trigger for
                                      the algorithm.
  @param[in]       CurrentTrigger     Specifies the current trigger.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CheckConstrains (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            Delta,
  IN      UINT16            PreviousDelta,
  IN      CONST UINT8       *Constrains,
  IN      BOOLEAN           *GoToPreviousValidIndex,
  IN      BOOLEAN           *Triggers,
  IN      UINT8             CurrentTrigger
  )
{
  UINT8   AbsoluteValue;
  UINT8   AbsoluteTriggerValue;
  UINT8   RelativeValue;
  UINT8   ExecuteTrainingDropValue;
  BOOLEAN FirstTimeExecution;

  AbsoluteValue         = Constrains[Absolute];
  AbsoluteTriggerValue  = Constrains[AbsoluteTrigger];
  RelativeValue         = Constrains[Relative];
  ExecuteTrainingDropValue = Constrains[ExecuteTrainingDrop];

  FirstTimeExecution    = Triggers[GlobalFirstTimeExecution];

  if (Delta < AbsoluteValue) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "   Delta < %d (MMRC_ABS)", AbsoluteValue));
    if (FirstTimeExecution) {
      MmrcDebugPrint ((MMRC_DBG_MIN, ":: WARNING LOW MARGIN DETECTED!!!!\n"));
      *GoToPreviousValidIndex = FALSE;
      Triggers[FailureDetectedOnFirstKnob]  = TRUE;
    } else {
      *GoToPreviousValidIndex = TRUE;
    }
  } else if (Delta < AbsoluteTriggerValue) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "   Delta < %d(ABS_TRIGGER[%d])\n", AbsoluteTriggerValue, Triggers[CurrentTrigger]));
    if (Triggers[CurrentTrigger]) {
      if ((PreviousDelta > Delta) && ((PreviousDelta - Delta) >= RelativeValue)) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "   Relative: %d\n", RelativeValue));
        *GoToPreviousValidIndex = TRUE;
      }
    } else {
      Triggers[CurrentTrigger] = TRUE;
    }
  }
  if ((PreviousDelta > Delta) && ((PreviousDelta - Delta) >= ExecuteTrainingDropValue) && !(*GoToPreviousValidIndex)) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "   Exec Training %d/%d\n", Delta, PreviousDelta));
    Triggers[ExecuteTrainingTrigger] = TRUE;
  }

  return MMRC_SUCCESS;
}

/**
  Executes the power training sweep according with the input
  parameters

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       InitIndex       Defines the POR value for the power knob under test.
  @param[in]       MaxIndex        Defines the maximum number of iterations fo powerknob
                                   under test.
  @param[in] PowerTrainingValues   Specifies all the power knobs values for iterations.
  @param[in] PowerTrainingCriteria Specifies the criteria in order to declare a power knob
                                   value as good/bad.
  @param[in]       InvalidValue    Value to declare a value in the array as invalid.
  @param[in]       HookSetValue    Function to be executed in order to set the right
                                   power knob value.
  @param[in]       TrainingStep    Function to be executed when re-center is needed
                                   a register or algorithm in the Elements[] table.
  @param[in] CheckTrainingToExecute Specifies the affected areas for the current power knob.
  @param[in]       SettingString   Pointer to the string of the current power knob.
  @param[in, out]  PowerKnobResult Return the value of the final value of the power knob
                                   under test
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PowerTrainingSweep (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT8             Channel,
  IN      UINT8             InitIndex,
  IN      UINT8             MaxIndex,
  IN      CONST UINT8       *PowerTrainingValues,
  IN      CONST UINT8       *PowerTrainingCriteria,
  IN      UINT8             InvalidValue,
  IN      HOOK_SET_VALUE    HookSetValue,
  IN      TRAINING_STEP     TrainingStep,
  IN      CONST BOOLEAN     *CheckTrainingToExecute,
  IN      UINT8             *SettingString,
  IN  OUT UINT8             *PowerKnobResult
  )
{
  UINT8     Index;
  UINT8     LastIndex;
  BOOLEAN   TriggerLog[MaxTrainingRequired][MaxTriggerLog];
  UINT8     Rank;
  UINT16    MinDeltaT[MaxTrainingRequired];
  UINT16    MinDeltaV[MaxTrainingRequired];
  UINT16    DeltaT[MaxTrainingRequired];
  UINT16    DeltaV[MaxTrainingRequired];
  UINT16    PreviousMinDeltaT[MaxTrainingRequired];
  UINT16    PreviousMinDeltaV[MaxTrainingRequired];
  BOOLEAN   GoToPreviousValidIndex;
  BOOLEAN   ErrorOnFirstKnob;
  UINT8     BackupMrcDebugMsgLevel;
  UINT8     TrainingExecuteStep;
  UINT8     OffsetPowerCriteria;

  BackupMrcDebugMsgLevel  = MrcData->MrcDebugMsgLevel;

  MmrcMemset (PreviousMinDeltaT, 0, MaxTrainingRequired * sizeof (UINT16));
  MmrcMemset (PreviousMinDeltaV, 0, MaxTrainingRequired * sizeof (UINT16));
  MmrcMemset (DeltaT, 0, MaxTrainingRequired * sizeof (UINT16));
  MmrcMemset (DeltaV, 0, MaxTrainingRequired * sizeof (UINT16));

  for (TrainingExecuteStep = 0; TrainingExecuteStep < MaxTrainingRequired; TrainingExecuteStep++) {
    TriggerLog[TrainingExecuteStep][GlobalFirstTimeExecution]   = TRUE;
    TriggerLog[TrainingExecuteStep][TimingBelowTrigger]         = FALSE;
    TriggerLog[TrainingExecuteStep][VoltageBelowTrigger]        = FALSE;
    TriggerLog[TrainingExecuteStep][FailureDetectedOnFirstKnob] = FALSE;
    TriggerLog[TrainingExecuteStep][ExecuteTrainingTrigger]     = FALSE;
  }
  LastIndex = InitIndex;
  for (Index = InitIndex; Index < MaxIndex; Index++) {
    if (PowerTrainingValues[Index] == InvalidValue) {
      continue;
    }
    LastIndex = Index;
    MmrcDebugPrint ((MMRC_DBG_MIN, "Set %a --> %d\n", SettingString, Index));
    HookSetValue (MrcData, Channel, Index);
    MrcData->MrcDebugMsgLevel = 0;
    SearchRmtWrapper (MrcData, 0, 0, Channel);
    MrcData->MrcDebugMsgLevel = BackupMrcDebugMsgLevel;

    MmrcMemset (MinDeltaT, 0xff, MaxTrainingRequired * sizeof (UINT16));
    MmrcMemset (MinDeltaV, 0xff, MaxTrainingRequired * sizeof (UINT16));
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        continue;
      }

      DeltaT[ExecuteRead]   = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RxDqRight] - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RxDqLeft];
      DeltaV[ExecuteRead]   = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RxVHigh]   - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RxVLow];
      DeltaT[ExecuteWrite]  = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TxDqRight] - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TxDqLeft];
      DeltaV[ExecuteWrite]  = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TxVHigh]   - MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][TxVLow];

      for (TrainingExecuteStep = 0; TrainingExecuteStep < MaxTrainingRequired; TrainingExecuteStep++) {
        if (DeltaT[TrainingExecuteStep] < MinDeltaT[TrainingExecuteStep]) {
          MinDeltaT[TrainingExecuteStep] = DeltaT[TrainingExecuteStep];
        }
        if (DeltaV[TrainingExecuteStep] < MinDeltaV[TrainingExecuteStep]) {
          MinDeltaV[TrainingExecuteStep] = DeltaV[TrainingExecuteStep];
        }
      } // TrainingExecuteStep loop ...
    } //  Rank loop ...

    GoToPreviousValidIndex  = FALSE;
    ErrorOnFirstKnob        = FALSE;
    MmrcMemset (MrcData->SettingsPowerKnobs.Execute, FALSE, MaxTrainingRequired * sizeof (BOOLEAN));
    for (TrainingExecuteStep = 0; TrainingExecuteStep < MaxTrainingRequired; TrainingExecuteStep++) {
      TriggerLog[TrainingExecuteStep][ExecuteTrainingTrigger]   = FALSE;
    }
    //
    // Verify all the constrains look right
    //
    for (TrainingExecuteStep = 0; TrainingExecuteStep < MaxTrainingRequired; TrainingExecuteStep++) {
      if (!CheckTrainingToExecute[TrainingExecuteStep]) {
        continue;
      }

      OffsetPowerCriteria = (MaxMarginVerify * MaxPowerCriteria * TrainingExecuteStep);

      MmrcDebugPrint ((
        MMRC_DBG_MIN,
        "Step:%d --> T: %d PT: %d V: %d PV:%d\n",
        TrainingExecuteStep,
        MinDeltaT[TrainingExecuteStep],
        PreviousMinDeltaT[TrainingExecuteStep],
        MinDeltaV[TrainingExecuteStep],
        PreviousMinDeltaV[TrainingExecuteStep]
      ));
      //
      // Check constrains for Timing for Read or Write
      //
      CheckConstrains (
        MrcData,
        MinDeltaT[TrainingExecuteStep],
        PreviousMinDeltaT[TrainingExecuteStep],
        &PowerTrainingCriteria[OffsetPowerCriteria + MaxPowerCriteria],
        &GoToPreviousValidIndex,
        &TriggerLog[TrainingExecuteStep][0],
        TimingBelowTrigger
      );
      //
      // Check constrains for Voltage for Read or Write
      //
      CheckConstrains (
        MrcData,
        MinDeltaV[TrainingExecuteStep],
        PreviousMinDeltaV[TrainingExecuteStep],
        &PowerTrainingCriteria[OffsetPowerCriteria],
        &GoToPreviousValidIndex,
        &TriggerLog[TrainingExecuteStep][0],
        VoltageBelowTrigger
      );
      //
      // Update which training should be re-executed according with the Constrains
      // These parameters are only used when Write & Read Training are both required
      // during the Power Knob under test
      //
      if (TriggerLog[TrainingExecuteStep][ExecuteTrainingTrigger]) {
        MrcData->SettingsPowerKnobs.Execute[TrainingExecuteStep] = TRUE;
      }
      //
      // If an ABSOLUTE error (only First knob error) was detected during Timing
      // or voltage margin, for Reads or Writes it is required stop
      // execution for the next power knob values
      //
      if (TriggerLog[TrainingExecuteStep][FailureDetectedOnFirstKnob]) {
        ErrorOnFirstKnob = TRUE;
      }
      //
      // Update the latest PreviousMinDelta with the new MinDelta
      //
      PreviousMinDeltaT[TrainingExecuteStep] = MinDeltaT[TrainingExecuteStep];
      PreviousMinDeltaV[TrainingExecuteStep] = MinDeltaV[TrainingExecuteStep];

      TriggerLog[TrainingExecuteStep][GlobalFirstTimeExecution] = FALSE;
    } // TrainingExecuteStep loop ...

    if (MrcData->SettingsPowerKnobs.Execute[ExecuteRead] || MrcData->SettingsPowerKnobs.Execute[ExecuteWrite]) {
      MrcData->MrcDebugMsgLevel = 0;
      TrainingStep (MrcData, 0, 0, Channel);
      MrcData->MrcDebugMsgLevel = BackupMrcDebugMsgLevel;
    }

    if (GoToPreviousValidIndex && !ErrorOnFirstKnob) {
      while (PowerTrainingValues[--Index] == InvalidValue);
      MmrcDebugPrint ((MMRC_DBG_MED, "Previous valid: %a = %d\n", SettingString, Index));
      LastIndex = Index;
      break;
    } else if (ErrorOnFirstKnob) {
      MmrcDebugPrint ((MMRC_DBG_MED, "Failure detected on first knob: %a = %d\n", SettingString, Index));
      break;
    }
  } // Index loop ...
  //
  // Retrain with the last valid value
  // and save the last Power Knob Index
  //
  HookSetValue (MrcData, Channel, LastIndex);
  *PowerKnobResult = LastIndex;
  MrcData->MrcDebugMsgLevel = 0;
  TrainingStep (MrcData, 0, 0, Channel);
  MrcData->MrcDebugMsgLevel = BackupMrcDebugMsgLevel;

  return MMRC_SUCCESS;
}

/**
  Executes the Power training on SoC Odt

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in, out]  Training        Returns a pointer to the function that should be
                                   executed examined.
  @param[in] CheckTrainingToExecute Pointer to the parameters affected by current power knob.
  @retval          MMRC_SUCCESS     At least one training should
                                    be executed
  @retval         !MMRC_SUCCESS     No training pointer found
**/
BOOLEAN
DetermineTrainingStep (
  IN  OUT MMRC_DATA         *MrcData,
  IN  OUT TRAINING_STEP     *Training,
  IN      CONST BOOLEAN     *CheckTrainingToExecute
  )
{
  if (CheckTrainingToExecute[ExecuteRead] && CheckTrainingToExecute[ExecuteWrite]) {
    *Training = &ReadWriteTraining;
  } else if (CheckTrainingToExecute[ExecuteRead]) {
    *Training = &ReadTraining;
  } else if (CheckTrainingToExecute[ExecuteWrite]) {
    *Training = &WriteTraining;
  } else {
    return FALSE;
  }
  return TRUE;
}

#endif // POWER_TRAINING

/**
  Executes the Power training on SoC Odt

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]  CapsuleStartIndex    Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          NONE
**/
MMRC_STATUS
SocOdtTraining (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if POWER_TRAINING == 1
  BOOLEAN       FoundTrainingStep;
  UINT8         BackupRmtSupport;
  UINT32        BackupMrcRmtEnableSignal;
  TRAINING_STEP Training;

  if ((MrcData->PowerTrainingEnable & SOC_ODT_KNOB_ENABLE) == 0) {
    return MMRC_SUCCESS;
  }

  BackupRmtSupport                      = MrcData->MrcRmtSupport;
  BackupMrcRmtEnableSignal              = MrcData->MrcRmtEnableSignal;

  MrcData->MrcRmtSupport                = RMT_SUPPORT_ENABLED | RMT_SUPPORT_OVERRIDE;
  MrcData->ExecuteThisRoutineInParallel = FALSE;

  MmrcExecuteTask (MrcData, SOC_ODT_TRAINING_ENTRY, NO_PRINT, Channel);

  FoundTrainingStep = DetermineTrainingStep (MrcData, &Training, &TrainingToExecute[PowerDdr3l][SocOdt][0]);

  if (FoundTrainingStep) {

    SetupRmtLevelTraining (
      MrcData,
      &TrainingToExecute[PowerDdr3l][SocOdt][0]
    );

    PowerTrainingSweep (
      MrcData,
      Channel,
      MrcData->SettingsPowerKnobs.InitialValue[PowerDdr3l][SocOdt],
      MaxOdtValues,
      &SocOdtValues[PowerDdr3l][0],
      &CriteriaPowerNobs[SocOdt][0],
      InvalidOdtValue,
      SetSocOdt,
      Training,
      &TrainingToExecute[PowerDdr3l][SocOdt][0],
      "SocOdt",
      &MrcData->SettingsPowerKnobs.FinalValue[PowerDdr3l][SocOdt]
    );
  }

  MrcData->MrcRmtSupport      = BackupRmtSupport;
  MrcData->MrcRmtEnableSignal = BackupMrcRmtEnableSignal;
#endif // POWER_TRAINING

  return MMRC_SUCCESS;
}

/**
  Executes Read Training

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]  CapsuleStartIndex    Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          NONE
**/
MMRC_STATUS
SetPiAndPbd(
  MMRC_DATA *MrcData,
  UINT8     Channel,
  UINT8     Rank,
  UINT16    Dim1Index,
  UINT16    Dim1PBIndex,
  UINT16    PiValues[MAX_STROBES][MAX_BITS])
{
  UINT8   Strobe;
  UINT32  StrobeValue;
  UINT32  BitValue;
  UINT8   Bit;
 //
  // Determine the miniumum RX for each strobe lane.
  //
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    StrobeValue = PiValues[Strobe][0];
    for (Bit = 1; Bit < MAX_BITS; Bit++) {
      if (StrobeValue > PiValues[Strobe][Bit]) {
        StrobeValue = PiValues[Strobe][Bit];
      }
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxDqsDelay, CMD_SET_VAL_FC_UC, &StrobeValue);
    //
    // The offset between the minimum Strobelane value and the determined value is the PB value.
    for (Bit = 0; Bit < MAX_BITS; Bit++) {
      BitValue = StrobeValue - PiValues[Strobe][Bit];
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RxDqBitDelay, CMD_SET_VAL_FC_UC, &BitValue);
    }
  }
  return MMRC_SUCCESS;
}
MMRC_STATUS
ReadTraining (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{

#if READ_TRAINING == 1 && TRAINING_ALGOS == 1
  UINT8 ch;
  UINT8 rk;
  UINT8 st;
  UINT8 i;
  UINT32 TempValue;
  EYEMASK_T LocalEyeMaskRead[RT_NUM_EYEMASKS];
  CONTROLKNOBS *ControlKnobs;

#if NEW_RTWT_ALGO == 1
  //
  // Before beginning the algo or restoring the trained values, call the entry function to setup the
  // system.
  //
  MrcData->HalfClkPi += MrcData->HalfClkPi/4;
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency][1] = MrcData->HalfClkPi;
  }
  #ifdef READTRAINING_ENTRY
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
      MmrcExecuteTask (MrcData, READTRAINING_ENTRY, NO_PRINT, ch);
    }
  }
  #endif
  MrcData->NumberOfTiming = 1;
  MrcData->MaxBits = MAX_BITS;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All) {
#if DDR3_SUPPORT==1
    ControlKnobs = ControlKnobsReadDDR3;
#endif
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
#if LPDDR3_SUPPORT==1
    ControlKnobs = ControlKnobsReadLP3;
#endif
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
#if LPDDR4_SUPPORT==1
    ControlKnobs = ControlKnobsReadLP4;
#endif
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Memory type unsupported.\n"));
    ASSERT (DDR3_SUPPORT && LPDDR3_SUPPORT && LPDDR4_SUPPORT);
    return MMRC_FAILURE;
  }
  if (MrcData->Rk2RkEn.RxDqs == FALSE) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Compositing Ranks because Rk2Rk switching is disabled.\n"));
    ControlKnobs[0].SharedFlags |= RANK_SHARED;
    if (ControlKnobs[0].NumberOfTiming == 2) {
      ControlKnobs[1].SharedFlags |= RANK_SHARED;
    }
  }
  //
  // If on the full boot path (not restore path), setup CPGC for Write training
  // and call the training algorithm, otherwise restore the passed in values.
  //
  MrcData->CpgcOptions.LoopCount = 8;
  MrcData->CpgcOptions.NumBursts = 8;
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
      L_CpgcSetup (MrcData, ch, CPGC_CONFIG_VA);
    }
  }
  //
  // Perform the Advanced WriteTraining Algorithm with the Write Control knobs.
  //
  MrcData->MaxElements = 0;
  for (ch=0; ch < MAX_CHANNELS; ch++) {
    for (rk=0; rk < MAX_RANKS; rk++) {
      if (RunOnThisChannel (MrcData, ch, rk)) {
        MrcData->MaxElements = MAX(MrcData->MaxElements, MrcData->NvData.MrcParamsSaveRestore.Channel[ch].MaxDq[rk]);
          for (st = 0; st < MrcData->MaxElements; st++) {
            for (i = 0; i < 2; i++) {
              GetSetDdrioGroup2 (MrcData, ch, rk, st, ControlKnobs[i].Index, CMD_GET_REG_UC, &TempValue);
            }
          }
      }
    }
  }
  for (i=0; i< RT_NUM_EYEMASKS; i++)
  {
    LocalEyeMaskRead[i].NumberConsecutiveVoltages=EyeMaskRead[i].NumberConsecutiveVoltages;
    LocalEyeMaskRead[i].MinimumDelayWidth=(EyeMaskPctPiRd[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency][i] * GetHalfClk(MrcData, 0))/100;
    MmrcDebugPrint ((MMRC_DBG_MIN, "Read Mask[%d], Vref=%d, DelayPis=%d\n", i, LocalEyeMaskRead[i].NumberConsecutiveVoltages, LocalEyeMaskRead[i].MinimumDelayWidth));
  }
  AdvancedTrainingAlgo (MrcData, ControlKnobs, NumberControlKnobsRead, LocalEyeMaskRead, NumberEyeMaskRead, MrcData->ReadPerBitEnable);

  //
  // Print out or restore the training results.
  //
  //DisplayOrRestoreTrainingResults (MrcData, Channel, RxDqsDelay, TRUE);
  MrcData->HalfClkPi = (MrcData->HalfClkPi * 4)/5;
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[ch].CurrentFrequency][1] = MrcData->HalfClkPi;
  }
  //
  // End with the exit task for write training.
  //
  #ifdef READTRAINING_EXIT
   MmrcExecuteTask (MrcData, READTRAINING_EXIT, NO_PRINT, Channel);
  #endif
#else // NEW_RTWT_ALGO
  UINT8   RxdqsKnobs;
  UINT16  RxdqsIndex;
  UINT8   RxdqsFlags;
  UINT8   Rank;
  UINT8   ChannelBackup;
  UINT8   Knobs;

  RxdqsKnobs = 0;
  RxdqsIndex = 0;
  ChannelBackup = Channel;

  MrcData->CpgcOptions.RmtRestoreJedecResetType     = NO_ACTION;

#if FAULTY_PART_TRACKING
  FaultyPartTrackInit (MrcData, Channel, FALSE);
#endif
  //
  // ReadTraining Entry
  //
#ifdef READTRAINING_ENTRY
  MmrcExecuteTask (MrcData, READTRAINING_ENTRY, NO_PRINT, Channel);
#endif
  if (!MrcData->RestorePath) {
#if CPGC_BASED_READWRITES == 1
    MrcData->CpgcOptions.LoopCount = MrcData->MrcTrainingCpgcExpLoopCnt;
    MrcData->CpgcOptions.NumBursts = MrcData->MrcCpgcNumBursts;
    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_VA);
    }
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
#endif
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->ExecuteThisRoutineInParallel) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Read CxxR%d\n", Rank));
      } else {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Read C%02dR%d\n", Channel, Rank));
      }
      for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          continue;
        }
#if CPGC_BASED_READWRITES == 1
        //
        // Setup the CPGC engine to to do a single read from an address within the
        // selectable rank.  The engine should be setup for LFSR mode.
        //
        MrcData->CpgcOptions.Rank = Rank;
        MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_WRITE_READ;
        L_CpgcReconfig (MrcData, Channel);
#endif
        if (!MrcData->ExecuteThisRoutineInParallel) {
          //
          // Stop the routine after first execution
          // if not supposed to be executed in parallel
          //
          break;
        }
      } // Channel loop ...
      //
      // Restore back the Channel Under Test
      //
      Channel = ChannelBackup;
      //
      // If Odd/Even is supported for the RDQS, then the number of knobs is set to 2 with the initial dim index pointing to the first
      // element.
      //
#if RDQS_ODDEVENSUPPORT==1
      RxdqsKnobs = 2;
      RxdqsIndex = RxDqsPDelay;
      RxdqsFlags = ABSOLUTE | MULTIKNOB;
#else
      RxdqsKnobs = 1;
      RxdqsIndex = RxDqsDelay;
      RxdqsFlags = ABSOLUTE;
#endif
      SweepCenterEye2D (MrcData, Channel, Rank, RxdqsIndex, 0,
        GetHalfClk (MrcData, Channel),
        1, RxdqsFlags, RxVref, RX_VREF_MIN, RX_VREF_MAX, RX_VREF_STEP, ABSOLUTE | START_SWEEP_CENTER,
        CpgcExecuteTest2, "S");
    } // Rank loop ...

    if (MrcData->Rk2RkEn.RxDqs == 0) {
      for (Knobs = 0; Knobs < RxdqsKnobs; Knobs++) {
        Average (MrcData, Channel, RxdqsIndex + Knobs, MMRC_NA);
      }
    }
    Average (MrcData, Channel, RxVref, MMRC_NA);
  } // Restore Path
  //
  // Print out or restore the training results.
  //
#if RDQS_ODDEVENSUPPORT==1
  DisplayOrRestoreTrainingResults (MrcData, Channel, RxDqsPDelay_dq, TRUE);
  DisplayOrRestoreTrainingResults (MrcData, Channel, RxDqsNDelay_dq, TRUE);
#else
  DisplayOrRestoreTrainingResults (MrcData, Channel, RxDqsDelay_dq, TRUE);
#endif
  //
  // ReadTraining Exit
  //
#ifdef READTRAINING_EXIT
  MmrcExecuteTask (MrcData, READTRAINING_EXIT, NO_PRINT, Channel);
#endif
#endif // NEW_READ_TRAINING_ALGO
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
#endif // READ_TRAINING
  return MMRC_SUCCESS;
}

/**
  Executes the Power training on Dram Rtt Nom and Rtt Wr

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]  CapsuleStartIndex    Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
DimmOdtTraining (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if POWER_TRAINING == 1
  BOOLEAN       FoundTrainingStep;
  UINT8         BackupRmtSupport;
  UINT32        BackupMrcRmtEnableSignal;
  UINT8         MaxOdtValuesForRttWr;
  TRAINING_STEP Training;

  BackupRmtSupport                      = MrcData->MrcRmtSupport;
  BackupMrcRmtEnableSignal              = MrcData->MrcRmtEnableSignal;

  MrcData->MrcRmtSupport                = RMT_SUPPORT_ENABLED | RMT_SUPPORT_OVERRIDE;
  MrcData->ExecuteThisRoutineInParallel = FALSE;

  FoundTrainingStep = DetermineTrainingStep (MrcData, &Training, &TrainingToExecute[PowerDdr3l][DramRttNomOdt][0]);

  if (FoundTrainingStep && ((MrcData->PowerTrainingEnable & DIMM_RTTNOM_KNOB_ENABLE) != 0)) {
    SetupRmtLevelTraining (
      MrcData,
      &TrainingToExecute[PowerDdr3l][DramRttNomOdt][0]
    );
    PowerTrainingSweep (
      MrcData,
      Channel,
      MrcData->SettingsPowerKnobs.InitialValue[PowerDdr3l][DramRttNomOdt],
      MaxOdtValues,
      &DimmRttNomValues[PowerDdr3l][0],
      &CriteriaPowerNobs[DramRttNomOdt][0],
      InvalidOdtValue,
      SetDramOdtRttNom,
      Training,
      &TrainingToExecute[PowerDdr3l][DramRttNomOdt][0],
      "Dram RttNom Odt",
      &MrcData->SettingsPowerKnobs.FinalValue[PowerDdr3l][DramRttNomOdt]
    );
  }

  FoundTrainingStep = DetermineTrainingStep (MrcData, &Training, &TrainingToExecute[PowerDdr3l][DramRttWrOdt][0]);

  if (FoundTrainingStep && ((MrcData->PowerTrainingEnable & DIMM_RTTWR_KNOB_ENABLE) != 0)) {

    SetupRmtLevelTraining (
      MrcData,
      &TrainingToExecute[PowerDdr3l][DramRttWrOdt][0]
    );

    if (MrcData->SettingsPowerKnobs.FinalValue[PowerDdr3l][DramRttNomOdt] == OdtInfinite) {
      MaxOdtValuesForRttWr = OdtInfinite - 1;
    } else {
      MaxOdtValuesForRttWr = OdtInfinite;
    }

    PowerTrainingSweep (
      MrcData,
      Channel,
      MrcData->SettingsPowerKnobs.InitialValue[PowerDdr3l][DramRttWrOdt],
      MaxOdtValuesForRttWr,
      &DimmRttWrValues[PowerDdr3l][0],
      &CriteriaPowerNobs[DramRttWrOdt][0],
      InvalidOdtValue,
      SetDramOdtRttWr,
      Training,
      &TrainingToExecute[PowerDdr3l][DramRttWrOdt][0],
      "Dram RttWr Odt",
      &MrcData->SettingsPowerKnobs.FinalValue[PowerDdr3l][DramRttWrOdt]
    );
  }

  MrcData->MrcRmtSupport      = BackupRmtSupport;
  MrcData->MrcRmtEnableSignal = BackupMrcRmtEnableSignal;
#endif // POWER_TRAINING

  return MMRC_SUCCESS;
}

/**
  Executes Advanced Write Training

  @param[in, out]  MrcData           Host structure for all data related to MMRC.
  @param[in]       CapsuleStartIndex Starting point within the CapsuleData structure
                                     where the Init should begin.
  @param[in]       StringIndex       When provided, this is the index into the string
                                     table to show the register names.
  @param[in]       Channel           Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
WriteTraining (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
  CONTROLKNOBS ControlKnobs[2];
  UINT16 Voh;
  UINT8 Loop;
  UINT16 Vref;
  UINT16 PercentageOfVddq;
  UINT8 Rank;
  UINT8 i;
  UINT8 Ch;
   EYEMASK_T LocalEyeMaskWrite[WT_NUM_EYEMASKS];
  //UINT32 LocalOdt=ODT;
  UINT32 LocalOdt=DramOdt[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentFrequency];
  //UINT32 WrVrefOvr = WT_VREFOVERRIDE_VALUE;
  UINT16 WrVrefOvr = DramVrefOvr[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentFrequency];

  MrcData->NumberOfTiming = 1;
  MrcData->MaxBits = MAX_BITS;

  //
  // Copy the Pre-compoiled Write Control knobs to a local copy to be modified.
  //

  for (Loop=0; Loop < 2; Loop++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All) {
#if DDR3_SUPPORT==1
      ControlKnobs[Loop] = ControlKnobsWriteDDR3[Loop];
#endif
    } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
#if LPDDR3_SUPPORT==1
      ControlKnobs[Loop] = ControlKnobsWriteLP3[Loop];
#endif
    } else {
#if LPDDR4_SUPPORT==1
      ControlKnobs[Loop] = ControlKnobsWriteLP4[Loop];
#endif
    }
  }

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    //
    // Determine the new VREF ranges based on the runtime parameters.
    // Currently these parameters are all compile time.
    //
    if (LDO_ENABLED==1) {
      Voh = (UINT16) (LDO_TARGET * (LocalOdt/(LocalOdt+RON)));
    } else {
      Voh = (UINT16) (VDDQ * LocalOdt/(LocalOdt+RON));
    }
    Vref = Voh/2;
    PercentageOfVddq = Vref*100/VDDQ;
    if (WT_VREF_OVERRIDE_ENABLE == 1) {
      PercentageOfVddq = WrVrefOvr;
    }
    MmrcDebugPrint ((MMRC_DBG_MED, "ODT = %d, Vddq% = %d, WrVrefOvr = %d\n", LocalOdt, PercentageOfVddq, WrVrefOvr));
    //
    // Program the new VREF ranges.
    //
    ControlKnobs[1].Ranges[HIGH] = 420;
    if (PercentageOfVddq > 42) {
      ControlKnobs[1].Ranges[LOW] = 420;
    } else {
      ControlKnobs[1].Ranges[LOW] = (PercentageOfVddq - (42 - PercentageOfVddq)) * 10;
      ControlKnobs[1].Ranges[LOW] = 100;
    }
    ControlKnobs[1].Phase2_VrefStart = ControlKnobs[1].Ranges[LOW];
    ControlKnobs[1].Phase2_VrefStop  = ControlKnobs[1].Ranges[HIGH];
    ControlKnobs[1].StartValue[LOW] = ControlKnobs[1].Ranges[LOW];
    ControlKnobs[1].StartValue[HIGH] = ControlKnobs[1].Ranges[HIGH];
  }
  if (MrcData->Rk2RkEn.TxDq == FALSE) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Compositing Ranks because Rk2Rk switching is disabled.\n"));
    ControlKnobs[0].SharedFlags |= RANK_SHARED;
  }
#if WRITE_TRAINING == 1 && TRAINING_ALGOS == 1
#if NEW_RTWT_ALGO == 1
  //
  // Before beginning the algo or restoring the trained values, call the entry function to setup the
  // system.
  //
  #ifdef WRITETRAINING_ENTRY
   MmrcExecuteTask (MrcData, WRITETRAINING_ENTRY, NO_PRINT, Channel);
  #endif
  //
  // If on the full boot path (not restore path), setup CPGC for Write training
  // and call the training algorithm, otherwise restore the passed in values.
  //
  MrcData->CpgcOptions.LoopCount = 7;
  MrcData->CpgcOptions.NumBursts = 7;
  for (Ch = 0; Ch < MAX_CHANNELS; Ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Ch].Enabled == TRUE) {
      L_CpgcSetup (MrcData, Ch, CPGC_CONFIG_VA);
    }
  }
  MrcData->MaxElements = 0;
  for (Ch = 0; Ch < MAX_CHANNELS; Ch++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (RunOnThisChannel (MrcData, Ch, Rank)) {
        MrcData->MaxElements = MAX(MrcData->MaxElements, MrcData->NvData.MrcParamsSaveRestore.Channel[Ch].MaxDq[Rank]);
      }
    }
  }
  for (i=0; i< WT_NUM_EYEMASKS; i++)
  {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All) {
      LocalEyeMaskWrite[i].NumberConsecutiveVoltages=1;
  } else {
      LocalEyeMaskWrite[i].NumberConsecutiveVoltages=EyeMaskWrite[i].NumberConsecutiveVoltages;
  }
    LocalEyeMaskWrite[i].MinimumDelayWidth=(EyeMaskPctPiWr[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency][i] * GetHalfClk(MrcData, 0))/100;
    MmrcDebugPrint ((MMRC_DBG_MIN, "Write Mask[%d], Vref=%d, DelayPis=%d\n", i, LocalEyeMaskWrite[i].NumberConsecutiveVoltages, LocalEyeMaskWrite[i].MinimumDelayWidth));
  }
  //
  // Check if the eyemask vrefs can even be attempted.  Flag error if it can't.
  //
  if ((ControlKnobs[1].Ranges[HIGH] -  ControlKnobs[1].Ranges[LOW])/ControlKnobs[1].CriteriaStep<LocalEyeMaskWrite[0].NumberConsecutiveVoltages-1) {
     MmrcDebugPrint ((MMRC_DBG_MIN, "Advanced Write Training: Unable to Pass Minimum EM Criteria. <>JMP$<>\n"));
     return MMRC_FAILURE; //while(1);
  }
  //
  // Perform the Advanced WriteTraining Algorithm with the Write Control knobs.
  //

  //temp disable txpbd for lp4
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    MrcData->WritePerBitEnable = 0;
  }

  AdvancedTrainingAlgo (MrcData, ControlKnobs, NumberControlKnobsWrite, LocalEyeMaskWrite, NumberEyeMaskWrite, MrcData->WritePerBitEnable);

  //
  // Print out or restore the training results.
  //
#if BXTPMRC
  DisplayOrRestoreTrainingResults (MrcData, Channel, TxDqDelay_Actual, TRUE, MAX_STROBES_NON_ECC);
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    DisplayOrRestoreTrainingResults (MrcData, Channel, TxDqDelay_Actual_Ecc, TRUE, 1);
  }
#else
  DisplayOrRestoreTrainingResults (MrcData, Channel, TxDqDelay_Actual, TRUE, MAX_STROBES);
#endif
  //
  // End with the exit task for write training.
  //
  #ifdef WRITETRAINING_EXIT
   MmrcExecuteTask (MrcData, WRITETRAINING_EXIT, NO_PRINT, Channel);
  #endif
#else // NEW_RTWT_ALGO
  UINT8   Rank;
  UINT32  TempValue;
  UINT8   Strobe;
  UINT8   ChannelBackup;

  ChannelBackup = Channel;

  MrcData->CpgcOptions.RmtRestoreJedecResetType     = NO_ACTION;

#if FAULTY_PART_TRACKING
  FaultyPartTrackInit (MrcData, Channel, FALSE);
#endif
  TempValue = 0;
  Strobe = 0;
  //
  // WriteTraining Entry
  //
#ifdef WRITETRAINING_ENTRY
  MmrcExecuteTask (MrcData, WRITETRAINING_ENTRY, NO_PRINT, Channel);
#endif
  if (!MrcData->RestorePath) {
#if CPGC_BASED_READWRITES == 1
    MrcData->CpgcOptions.LoopCount = MrcData->MrcTrainingCpgcExpLoopCnt;
    MrcData->CpgcOptions.NumBursts = MrcData->MrcCpgcNumBursts;
    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_VA);
    }
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
#endif
#if TX_DQ_MINUS1_SUPPORT
    HandleMinus1Select (MrcData, Channel, TxDqDelay, TxDqMinus1);
#endif // TX_DQ_MINUS1_SUPPORT

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
          continue;
        }
        if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
          continue;
        }
        for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
            continue;
          }
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDelay, CMD_GET_REG, &TempValue);
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqDelay, CMD_SET_VAL_FC_UC, &TempValue);
        } // Strobe loop ...
        if (!MrcData->ExecuteThisRoutineInParallel) {
          //
          // Stop the routine after first execution
          // if not supposed to be executed in parallel
          //
          break;
        }
      } // Channel loop ...
    } // Rank loop ...
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->ExecuteThisRoutineInParallel) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Write CxxR%d\n", Rank));
      } else {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Write C%02dR%d\n", Channel, Rank));
      }
      for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          continue;
        }
#if CPGC_BASED_READWRITES == 1
        //
        // Setup the CPGC engine to to do a single read from an address within the
        // selectable rank.  The engine should be setup for LFSR mode.
        //
        MrcData->CpgcOptions.Rank = Rank;
        MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_WRITE_READ;
        L_CpgcReconfig (MrcData, Channel);
#endif
        if (!MrcData->ExecuteThisRoutineInParallel) {
          //
          // Stop the routine after first execution
          // if not supposed to be executed in parallel
          //
          break;
        }
      } // Channel loop ...
      //
      // Restore back the Channel Under Test
      //
      Channel = ChannelBackup;
      //
      // Sweep a full 1X clock.
      //
      SweepCenterEye2D (
        MrcData,
        Channel,
        Rank,
        TxDqDelay,
        -1 * GetQtrClk (MrcData, Channel),
        3 * GetQtrClk (MrcData, Channel),
        1,
        RELATIVE,
        TxVref,
        TX_VREF_MIN,
        TX_VREF_MAX,
        TX_VREF_STEP,
        ABSOLUTE | START_SWEEP_CENTER | SWEEP_ALL_STROBES_TOGETHER,
        CpgcExecuteTest2,
        "S"
      );

    } // Rank loop ...


    if (MrcData->Rk2RkEn.TxDq == 0) {
      Average (MrcData, Channel, TxDqDelay, MMRC_NA);
    }

  }
  //
  // Print out or restore the training results.
  //
  DisplayOrRestoreTrainingResults (MrcData, Channel, TxDqDelay, TRUE);
  //
  // WriteTraining Exit
  //
#ifdef WRITETRAINING_EXIT
  MmrcExecuteTask (MrcData, WRITETRAINING_EXIT, NO_PRINT, Channel);
#endif
#endif // NEW_RTWT_ALGO
#endif // READ_WRITE_TRAINING
  return MMRC_SUCCESS;
}

MMRC_STATUS
GetLowHighMarginPerStrobe (
  IN   MMRC_DATA    *MrcData,
  IN   UINT8        Channel,
  IN   UINT8        Rank,
  IN   UINT16       Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2],
  IN   UINT32       CurrentValue[MAX_STROBES],
  IN   UINT16       DelaySignal,
  IN   UINT8        RmtTableIndex
)
{
  UINT8   Strobe;
#if RMT_PER_BIT_MARGIN
  UINT8   StrobeTemp;
  UINT8   Bit;
#endif
  INT16   RmtMinLeftMargin;
  INT16   RmtMinRightMargin;
  INT16   rmtBlMargin;

  RmtMinLeftMargin  = -32767;
  RmtMinRightMargin = +32767;

#if RMT_DEBUG
  MmrcDebugPrint ((MMRC_DBG_DEBUG, "RMT_DEBUG\n"));
  for (Strobe = 0; Strobe <MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
    MmrcDebugPrint ((MMRC_DBG_DEBUG, " --S%02d--  ", Strobe));
  }
  MmrcDebugPrint ((MMRC_DBG_DEBUG, "\n"));
#endif
  //
  // Look for the worst margin
  //
  for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
#if RMT_PER_BIT_MARGIN
    for (Bit = 0; Bit < MAX_BITS; Bit++) {
      rmtBlMargin = Results[Channel][0][Strobe][Bit][LOW] - (UINT16)CurrentValue[Strobe];
      // When a test failed at every step, the low side margin result would be positive value, force them to 0.
      if (rmtBlMargin > 0) rmtBlMargin = 0;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RmtTableIndex][Bit + Strobe * MAX_BITS][LOW] = rmtBlMargin;
      //
      // Read Left or Low margin
      //
      if ( rmtBlMargin > RmtMinLeftMargin) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][LOW] = rmtBlMargin;
        RmtMinLeftMargin = rmtBlMargin;
      }
      rmtBlMargin = Results[Channel][0][Strobe][Bit][HIGH] - (UINT16)CurrentValue[Strobe];
      // When a test failed at every step, high side margin result would be negative value. force them to 0.
      if ( rmtBlMargin < 0 ) rmtBlMargin = 0;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RmtTableIndex][Bit + Strobe * MAX_BITS][HIGH] = rmtBlMargin;
      //
      // Read Right or High margin
      //
      if (rmtBlMargin < RmtMinRightMargin) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][HIGH] = rmtBlMargin;
        RmtMinRightMargin = rmtBlMargin;
      }
    } // Bit
#else
    rmtBlMargin = Results[Channel][0][Strobe][0][LOW] - (UINT16)CurrentValue[Strobe];
    //
    // Read Left or Low margin
    //
    if ( rmtBlMargin > RmtMinLeftMargin) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][LOW] = rmtBlMargin;
      RmtMinLeftMargin = rmtBlMargin;
    }
    rmtBlMargin = Results[Channel][0][Strobe][0][HIGH] - (UINT16)CurrentValue[Strobe];
    //
    // Read Right or High margin
    //
    if (rmtBlMargin < RmtMinRightMargin) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][HIGH] = rmtBlMargin;
      RmtMinRightMargin = rmtBlMargin;
    }

    // When a test failed at every step, the low side margin result would be positive value, while
    // high side margin result would be negative value. force them to 0.
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][LOW] > 0)
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][LOW] = 0;
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][HIGH] < 0)
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][HIGH] = 0;
#endif
#if RMT_DEBUG
    MmrcDebugPrint ((MMRC_DBG_DEBUG, " %02d:%02d   ", MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][LOW], MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][HIGH]));
#endif

    BreakStrobeLoop (MrcData,  Channel, Rank, DelaySignal, &Strobe);
  }// Strobe loop
#if RMT_PER_BIT_MARGIN
  // Copy the per lane margin result of strobe 0 to others strobe groups if a signal (like cmd, ctl, txvref ) only margin the first strobe
  for (Strobe = 1; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
    StrobeTemp = Strobe;
    BreakStrobeLoop (MrcData,  Channel, Rank, DelaySignal, &StrobeTemp);

    if (StrobeTemp != Strobe) { // Cmd, Ctl TxvRef, etc
      for (Bit = 0; Bit < MAX_BITS; Bit++) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RmtTableIndex][Bit + Strobe * MAX_BITS][LOW] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RmtTableIndex][Bit ][LOW];
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RmtTableIndex][Bit + Strobe * MAX_BITS][HIGH] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[Rank][RmtTableIndex][Bit ][HIGH];
      } // Bit
    }
  } // strobe
#endif
#if RMT_DEBUG
  MmrcDebugPrint ((MMRC_DBG_DEBUG, "\n"));
  MmrcDebugPrint ((MMRC_DBG_DEBUG, "Worst case: %02d : %02d\n",  MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][LOW], MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[Rank][RmtTableIndex][HIGH]));
#endif

  return MMRC_SUCCESS;
}

/**
  Executes Rank Margin Tool Setup

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
SearchRmtWrapper (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if RMT_PRODUCTION_MODE_SUPPORT == 1
  // return searchRmt status to check if Level 2 check failed and Level 1 test needs to be performed for Production mode
  UINT32 Status;
#endif //RMT_PRODUCTION_MODE_SUPPORT
  UINT32 SaveScratchpad0;
  UINT32 SaveScratchpad1;
  UINT32 Scratchpad0;
  UINT32 Scratchpad1;
  REGISTER_ACCESS Register;
  Register.Mask = 0xFFFFFFFF;
  Register.ShiftBit = 0;
  /**
    Since RMT executes in parallel, we just need to run on the first channel.
    The following logic ensures RMT runs once, and only once, regardless of memory technology.
    In cases where there is only one channel enabled, RMT will run.
    In the cases where there are multiple channels enabled, it will run RMT if it is channel 0 or channel 2
    This covers the following cases: 2x32, 4x32, 2x64, 1x64 and 1x32
  **/
  // MrcData->MrcRmtSupport == (RMT_SUPPORT_COMMAND_MODE | RMT_SUPPORT_ENABLED //Sandy : forcing MRC to run in EV mode
  // skip RMT if it was not enabled.
  if ((!(MrcData->MrcRmtSupport & RMT_SUPPORT_ENABLED)) ||
     (((MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled && MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled) ||
      (MrcData->NvData.MrcParamsSaveRestore.Channel[2].Enabled && MrcData->NvData.MrcParamsSaveRestore.Channel[3].Enabled)) &&
      ((Channel%2 == 1) || (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled && Channel == 2)))) {
    return MMRC_SUCCESS;
  }

#if RMT_PRODUCTION_MODE_SUPPORT == 1
  // Reset the margin result here for cold and fast boot because it doesnt get reset for FB and it would reset for Cold boot but just in case
  if ((MrcData->BootMode == S5) || (MrcData->BootMode == FB)) {
    MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = 0;
    MrcData->MrcRmtCallL1Check = FALSE;
  }

  // skip RMT if its not Production mode and FB
  if (MrcData->BootMode == FB) {
    if (MrcData->MrcRmtSupport != (RMT_SUPPORT_ENABLED | RMT_SUPPORT_PRODUCTION_MODE))  {
      return MMRC_SUCCESS;
    }
  }
#endif //RMT_PRODUCTION_MODE_SUPPORT
  //
  // enable all available tests by default
  //
  MrcData->MrcRmtEnableSignal = RMT_ALL_TESTS_EXEC;
  // Bit 0 of the MrcRmtSupport enable/disable RMT
  // Bit 1 and 2 of the MrcRmtSupport determine the RMT mode
  // 0b00: Production mode
  // 0b01: EV mode
  // 0b10: Command loop mode
  // 0b11: PPV mode

  if ( MrcData->MrcRmtSupport == (RMT_SUPPORT_EV_MODE | RMT_SUPPORT_ENABLED) ) {
    MrcData->MrcRmtEnableSignal |= RMT_FULL_EV_MODE;
    MrcData->MrcRmtCpgcNumBursts = RMT_NUMBURSTS;
    MrcData->MrcRmtCpgcExpLoopCnt = RMT_LOOPCOUNT;
  } else if ( MrcData->MrcRmtSupport == (RMT_SUPPORT_PPV_MODE | RMT_SUPPORT_ENABLED)) {
    MrcData->MrcRmtEnableSignal |= RMT_FULL_EV_MODE;
    MrcData->MrcRmtCpgcNumBursts = RMT_PPV_NUMBURSTS;
    MrcData->MrcRmtCpgcExpLoopCnt = RMT_PPV_LOOPCOUNT;
  } else {
    MrcData->MrcRmtEnableSignal &= 0xffffffff - RMT_FULL_EV_MODE;
    MrcData->MrcRmtCpgcNumBursts = RMT_PRODUCTION_NUMBURSTS;
    MrcData->MrcRmtCpgcExpLoopCnt = RMT_PRODUCTION_LOOPCOUNT;
  }

  if ( MrcData->MrcRmtSupport == (RMT_SUPPORT_COMMAND_MODE | RMT_SUPPORT_ENABLED)) {
    Register.Offset = D_CR_SSKPD0_DUNIT_BOXTYPE_REG;
    SaveScratchpad0 = (UINT32) MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register);
    Register.Offset = D_CR_SSKPD1_DUNIT_BOXTYPE_REG;
    SaveScratchpad1 = (UINT32) MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register);
    while ( 1 )
    {
      MmrcDebugPrint ((MMRC_DBG_MIN, "RMT_CMD\n"));
      Register.Offset = D_CR_SSKPD0_DUNIT_BOXTYPE_REG;
      MemRegWrite (DUNIT_BOXTYPE, Channel, 0, Register, 0, 0xF);
      Register.Offset = D_CR_SSKPD1_DUNIT_BOXTYPE_REG;
      MemRegWrite (DUNIT_BOXTYPE, Channel, 0, Register, 0, 0xF);

      // Polling until cmd is received
      Register.Offset = D_CR_SSKPD1_DUNIT_BOXTYPE_REG;
      Scratchpad1 = (UINT32) MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register);
      while ( Scratchpad1 == 0 )
      {
        Scratchpad1 = (UINT32) MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register);
      }
      // Retrieve the data associate to the cmd
      Register.Offset = D_CR_SSKPD0_DUNIT_BOXTYPE_REG;
      Scratchpad0 = (UINT32) MemRegRead (DUNIT_BOXTYPE, Channel, 0, Register);
      MmrcDebugPrint ((MMRC_DBG_MIN, "Channel:%d Scratchpad0:0x%X Scratchpad1:0x%X\n", Channel,Scratchpad0, Scratchpad1));
      switch( Scratchpad1 )
      {
      case RMT_COMMAND_SET_LOOPCOUNT:
        MrcData->MrcRmtCpgcExpLoopCnt  =  (UINT8) Scratchpad0;
        break;
      case RMT_COMMAND_SET_NUMBURSTS:
        MrcData->MrcRmtCpgcNumBursts =  (UINT8) Scratchpad0;
        break;
      case RMT_COMMAND_EXIT:
        Register.Offset = D_CR_SSKPD0_DUNIT_BOXTYPE_REG;
        MemRegWrite (DUNIT_BOXTYPE, Channel, 0, Register, SaveScratchpad0, 0xF);
        Register.Offset = D_CR_SSKPD1_DUNIT_BOXTYPE_REG;
        MemRegWrite (DUNIT_BOXTYPE, Channel, 0, Register, SaveScratchpad1, 0xF);
#if RMT_JUMP_POSTCODES
        MrcData->RmtJumpPostcode = (INT16) Scratchpad0;
#endif
        return MMRC_SUCCESS;
      case RMT_COMMAND_RUN:
        SearchRmt( MrcData, Channel );
        break;
      case RMT_COMMAND_ENABLE_SIGNAL:
        MrcData->MrcRmtEnableSignal =  Scratchpad0;
        break;
      case RMT_COMMAND_RESET_DIMMS:
        MmrcDebugPrint ((MMRC_DBG_MIN, "JEDECRESET \n"));
        CompleteJedecInit(MrcData, 0, 0, Channel);
      default:
        break;
      }
    }
  }    // end of command mode

#if RMT_PRODUCTION_MODE_SUPPORT == 1
  //Production Mode, the following code won't execute for
  //MrcRmtSupport = 3 EVMode,
  //MrcRmtSupport = 5 Command Mode,
  //MrcRmtSupport = 7 PPV mode
  //MrcRmtSupport = 9 Production mode
  if (MrcData->MrcRmtSupport == (RMT_SUPPORT_ENABLED | RMT_SUPPORT_PRODUCTION_MODE)) {
    switch (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtCheckRun) {
    case NO_RMT :
      Status =  MMRC_SUCCESS;
      break;
    case L1_CHECK :
      Status = SearchRmt (MrcData, Channel);
      break;
    // Both level 2 and 3 both level settings have to run through the same set of code so combine them
    case L2_CHECK  :
    case BOTH_CHECK:
      Status = SearchRmt(MrcData, Channel);
      if(MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState == 2)  {
        MrcData->MrcRmtCallL1Check = TRUE;
        MrcData->MrcRmtMarginResultOld = TRUE;   //Since L2 has failed so we need to remember the old result in case L1 fails then the Result = 3, both levels failed
        Status = SearchRmt (MrcData, Channel);
        MrcData->MrcRmtMarginResultOld = FALSE;  //Set it back to false after L1 results are printed because FB wont reset it and the result will still show that both levels failed
        MrcData->MrcRmtCallL1Check = FALSE;
        break;
      }
      break;
    default :
      Status= MMRC_SUCCESS;
      break;
    } //end switch
    return Status;
  } else
#endif //RMT_PRODUCTION_MODE_SUPPORT
  {
    return SearchRmt (MrcData, Channel);
  }
}


/**
  Executes Rank Margin Tool

  @param[in, out]  MrcData         Host structure for all data related to MMRC.

  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
SearchRmt (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT8             Channel
  )
{
#if RMT == 1 && TRAINING_ALGOS == 1
  UINT8   i;
  UINT8   Rank=0;
  UINT8   PseudoRank;
  UINT8   Strobe;
  UINT32  TempValue[MAX_CHANNELS][MAX_STROBES];
  UINT16  Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT16  Low[MAX_CHANNELS][MAX_STROBES];
  UINT16  High[MAX_CHANNELS][MAX_STROBES];
  UINT16  CmdMin[MAX_CHANNELS];
  UINT16  CmdMax[MAX_CHANNELS];
  UINT8   DebugMsgLevel;
  UINT16  Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT8   PiGroupCount=0;       // PiGroupMCount - total # of PI's, not counting Knobs doubling effect.
  UINT8   PiMarginGroupCount=0; // PiMarginGroupCount - total # of groups that will be margined INDEPENDENTLY inside of sweep function (not counting knobs)
  //   This takes place of 'Limit' in previous RMT
  UINT16  PiRestoreSignal;
  RMT_SIGNAL *RmtTablePtr;
  UINT16  Signal;
  UINT16  Errors=0;
  UINT8   Retry;
  UINT8   ChannelBackup;
  UINT8   ChannelTemp;
  UINT32  Temp;
  BOOLEAN ChannelEnabledForTestedRank[MAX_CHANNELS]; // flag if a channel is legible to run for a specified rank. For example, if the given rank is populaetd for a given channel
  BOOLEAN EnblePerBitSweep = FALSE;
  UINT16  StepSize;
  BOOLEAN SkipTxVref = FALSE;
  UINT16 DramType;
  BOOLEAN IsErrInAnyChannel =FALSE;
  BOOLEAN ExecuteInParallelOriginal;
  BOOLEAN ExecuteOnThisChannelOriginal[MAX_CHANNELS];
#if RMT_PER_BIT_MARGIN
  UINT8   Bit;
  EnblePerBitSweep = TRUE;
#endif
  ChannelBackup = Channel;
  DramType = TypeLpDdr4;
  //
  // Turn on the execution in parallel.
  // PT: Just turn on inside the SearchRmt(). I noticed that if I turn on the parallel in the whole MRC, it
  // actually had unexpected behavior since many of the MRC training code were not written to truely handle
  // the parallel. In particularly, the MmrcExecuteTask() already iterate the channel loop, the function should
  // not be called inside another channel for loop.
  ExecuteInParallelOriginal = MrcData->ExecuteThisRoutineInParallel;
  MrcData->ExecuteThisRoutineInParallel = TRUE;

  //
  // Turn on the execution in parallel.
  // PT: Just turn on inside the SearchRmt(). I noticed that if I turn on the parallel in the whole MRC, it
  // actually had unexpected behavior since many of the MRC training code were not written to truely handle
  // the parallel. In particularly, the MmrcExecuteTask() already iterate the channel loop, the function should
  // not be called inside another channel for loop.
  ExecuteInParallelOriginal = MrcData->ExecuteThisRoutineInParallel;
  MrcData->ExecuteThisRoutineInParallel = TRUE;

  // maintained three RmtTables
  if (( MrcData->MrcRmtSupport == (RMT_SUPPORT_EV_MODE | RMT_SUPPORT_ENABLED)) || (MrcData->MrcRmtSupport == (RMT_SUPPORT_COMMAND_MODE | RMT_SUPPORT_ENABLED))) {
    RmtTablePtr = RmtTable;
  } else if ( MrcData->MrcRmtSupport == (RMT_SUPPORT_PPV_MODE | RMT_SUPPORT_ENABLED) ) {
    RmtTablePtr = RmtTablePpv;
  } else {
    RmtTablePtr = RmtTableProduction;
  }

  MmrcMemset(Results, 0xff, MAX_CHANNELS * MAX_KNOBS * MAX_STROBES * MAX_BITS * 2 * sizeof (UINT16));

  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    MmrcMemset (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data , 0xff, sizeof (UINT16) * (MAX_RANKS + PSEUDO_RANKS) * MAX_RMT_ELEMENTS * 2);
#if RMT_PER_BIT_MARGIN
    MmrcMemset (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData , 0xff, sizeof (UINT16) * (MAX_RANKS + PSEUDO_RANKS) * MAX_RMT_ELEMENTS * MAX_BITS * MAX_STROBES * 2);
#endif
  } // Channel loop ...

  DebugMsgLevel = MrcData->MrcDebugMsgLevel;

  // find the dram type
  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel || !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }
    DramType = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType;
    break;
  }

  // LPDDR3 platform, the ch0/1, and ch2/3 share the same TxVref controller. We can't margin them in parallel
  // we have to override the ExecuteOnThisChannel field to margin ch0/2, then ch1/3.
  // Here we save the original value so that we can restore them back.
  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    ExecuteOnThisChannelOriginal[Channel] = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel; // MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled)
  }

  //
  // Don't skip RMT even MRC Debug Message is disabled to expedite boot time
  //
  if ( MrcData->MrcRmtSupport) {
    //
    // Turn off debug messages for RMT training steps.
    //
    // MrcData->MrcDebugMsgLevel = SDBG_NONE;
    //
    // Setup CPGC global options.
    //

    for (PseudoRank = 0; PseudoRank < MAX_RANKS + PSEUDO_RANKS; PseudoRank ++) {
      i = (UINT8) -1;
      MmrcMemset (ChannelEnabledForTestedRank, FALSE, MAX_CHANNELS * sizeof (BOOLEAN));
      while (RmtTablePtr[++i].Signal != MMRC_NA) {
        if (!(MrcData->MrcRmtEnableSignal & (1 << i))) {
          continue;
        }
        MrcData->CpgcOptions.NumBursts = MrcData->MrcRmtCpgcNumBursts ;
        MrcData->CpgcOptions.LoopCount = MrcData->MrcRmtCpgcExpLoopCnt;
        if (RmtTablePtr[i].CpgcConfig == CPGC_CONFIG_CMD_STRESS) {
          MrcData->CpgcOptions.LoopCount = MrcData->MrcRmtCpgcExpLoopCnt + (MrcData->CpgcOptions.NumBursts - 5);
        }
        StepSize = RmtTablePtr[i].StepSize;
          if (RmtTablePtr[i].Signal == TxVref) {
          // skip TxVref for the DDR3L platform
          if ((DramType != TypeLpDdr4) && (DramType != TypeLpDdr3)){  //Added LP3 condition so skip TxVref if not Lp3 or Lp4
            SkipTxVref = TRUE;
            continue;
          }

          // override the step size of LPDDR4 TxVRef as per JEDEC spec the voltage range changes by 0.4% of Vddq. Both LPDDR3 and LPDDR4 share the same RmtTable
          if (DramType == TypeLpDdr4)
          {
            StepSize = RmtTablePtr[i].StepSize * 4;
          }
        }

#if RMT_PRODUCTION_MODE_SUPPORT == 1
        //for level 2 production mode the step size is scalable.
        if ((RmtTablePtr == RmtTableProduction) && (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtCheckRun != 1) && (MrcData->MrcRmtCallL1Check==FALSE)) {
          StepSize = ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtMarginCheckScaleHighThreshold) * (StepSize)) / 100;
        }
#endif //RMT_PRODUCTION_MODE_SUPPORT

        //
        // Grab the index of the signal being margined in the GetSet table. This is used for
        // things like printing the string associated with this element.
        //

        // The GetOffsetIndex() convert Signal back to range 0x0 to 0x2xxx. Back to ALG_REG_INDEX or SGTI_INDEX range
        Signal = GetOffsetIndex(MrcData, RmtTablePtr[i].Signal);
        if (Signal >= SGTI_INDEX) {
          //Signal group  subtract 0x1000 (ALGO_REG_INDEX) to get the first delay type in the group
          PiRestoreSignal = Signal -  ALGO_REG_INDEX;        // the CmdGrp0 of the CmdALl
        } else {
          PiRestoreSignal = Signal;
        }

        //
        // Skip this rank or pseudo-rank, if directed by outside wrapper setup
        //
        if ((MrcData->MrcRmtEnableSignal >> Rmt_Rank0_Disable) & (1 << PseudoRank)) {
          continue;
        }

        //
        // SET UP CPGC ENGINE
        //

        // Peter:
        // when MrcData.EnableParallelTraining = TRUE and SearchRmtWrapper=ChAllP, RMT will run in parallel.
        // I have to use "Channel" loop to "setup  cpgc", "get the start vlaues, high/low limits", etc.
        for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
          if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel || !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
            continue;
          }

          // The L_CpgcSetup(). L_CpgcReconfig() didn't handle the execution in parallel.
          // until I rewrite these function for parallel execution, I will turn off parallel
          MrcData->ExecuteThisRoutineInParallel = FALSE;
          if (PseudoRank < MAX_RANKS) {
            Rank = PseudoRank;
            //
            // Handle 'normal rank' .. e.g. not pseudo-rank
            //
            //
            if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[PseudoRank]) {
              continue;
            }

            // Skip loop if RMT test doesn't apply to this rank
            //
            if (!(RmtTablePtr[i].RankChoices & RMT_PR_EACH_RANK)) {
              continue;
            }
            MrcData->CpgcOptions.CADB_Harrassment = TRUE;  // PT the CADB_Harrassment = TRUE won;t affect the CPGC_CONFIG_VA case.
            L_CpgcSetup (MrcData, Channel, RmtTablePtr[i].CpgcConfig);         // PT L_CpgcSetup() works on the specific channel
            MrcData->CpgcOptions.Rank = Rank;
            L_CpgcReconfig (MrcData, Channel);        // PT L_CpgcSetup() works on the specific channel

          } else {
            //
            // Handle RMT 'pseudo rank' - which are for alternative kinds of tests such as turnarounds
            //
            Rank = 0;
            switch (((1 << (PseudoRank - MAX_RANKS)) * (RMT_PR_START)) & RmtTablePtr[i].RankChoices) {
#if RMT_ENABLE_TURNAROUNDS_TEST == 1
            case PR_TA:
              L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_TURNAROUNDS);
              MrcData->CpgcOptions.Rank = ALL_COMBOS;  // Do all rank combos
              L_CpgcReconfig (MrcData, Channel);
              break;
#endif
#if RMT_ENABLE_LFSR_ADDRESS_TEST == 1
            case PR_LFSR:
              L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_LFSR_ADDRESS);
              L_CpgcReconfig (MrcData, Channel);  // For lfsr address currently does nothing
              break;
#endif
            default:  // Code will get here if this pseudo-rank is not supporting by this margin param in RmtTable
              continue;
              break;
            }
          }

          MrcData->ExecuteThisRoutineInParallel = TRUE;      // PT may not set inside the loop. set it once at the end and outside the loop
          // set up the error checking chunk select
          MrcData->DynamicVars[Channel][CHUNK_ERR_MASK]  = 0xFF;

          ChannelEnabledForTestedRank[Channel] = TRUE; // save to this flag so that we can use it for other step without repeating the qualifying again.
        } // Channel loop ...

        // The MmrcExecuteTask() already take care of the parallel execution
        MmrcExecuteTask (MrcData, CPGC_SET_CHUNK_MASK, NO_PRINT, ChannelBackup);

        //
        // GET SWEEP START(TRAINING) VALUES AND LOW/HIGH LIMITS
        //

        for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
          if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel || !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
            continue;
          }

          if (!ChannelEnabledForTestedRank[Channel]) {     // skip if this channel didn't support the PseudoRank
            continue;
          }

          MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nSIG:%s C%dR%d\n", SignalInfo[GetSignalInfoIndex(Signal)].Name, Channel, PseudoRank));
          //
          // Setup Margin Param
          //
          if (RmtTablePtr[i].SkipInitialProgramming == TRUE) {
            if (Signal >= SGTI_INDEX)  // Group signal like CmdAll
            {
              if (GetSignalInfoIndex(Signal) < MAX_SIGNAL_INFO_ELEMENTS) {
                PiGroupCount = SignalInfo[GetSignalInfoIndex(Signal)].NumSignalGroups;
              }
            }
            else
            {
              PiGroupCount = 1;   // example.. CSN signal within CTL_ALL, like CmdGrp0, CmdGrp1
            }
            PiMarginGroupCount = 1;  // If the number of signal groups is more than 1, tether them together for RMT.
          } else {
            PiGroupCount = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[PseudoRank];
            PiMarginGroupCount = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[PseudoRank];

            //// override the PiGroupCount and PiMarginGroupCount for the TxVref, which is DRAM technology depend
            //// PT I hard code to 1 because LPDDR4 is x16 device, one TxVref for channel.
            //if (RmtTablePtr[i].Signal == TxVref) {
            //  PiGroupCount = 1;
            //  PiMarginGroupCount = 1;
            //}
          }

          //
          // Set margining limits for Create1DSweepLastPass  .. FODO: Option to Create1DSweepLastPass to do this automatically
          //  Unfortunately this is messy because of uneven way that Create1DSweep handles per-bytelane marginparams vs. global-ish marginparams like cmd & ctl
          //
          CmdMin[Channel] = 0xFFFF;
          CmdMax[Channel] = 0;
          for (Strobe = 0; Strobe < PiGroupCount; Strobe++) {
            if (RmtTablePtr[i].SkipInitialProgramming == FALSE) {
              //
              // For per bytelane based marignparam, Create1DSweep wants all the trained values input into Dim1StartPoint so that SkipInitialProgramming=FALSE
              // will work when one edge of eye is found to restore values back to the middle.
              // FODO: How to program PI registers for rank != 0
              //
              GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RmtTablePtr[i].Signal, CMD_GET_REG, &TempValue[Channel][Strobe]); // FODO: For pseudo-ranks (TA/LFSR) on projects with per rank training, need to loop over all ranks
              Dim1StartPoint[Channel][0][Strobe][LOW] = (UINT16) TempValue[Channel][Strobe]; // for RecEn WA + GetOneClk (MrcData, Channel);
              Dim1StartPoint[Channel][0][Strobe][HIGH] = Dim1StartPoint[Channel][0][Strobe][LOW];
#if RMT_PRODUCTION_MODE_SUPPORT == 1
              // For PRODUCTION MODE we want to avoid to Margin checks at Nominal for low side so adjust the Dim1StartPoint to the next value
              if(RmtTablePtr == RmtTableProduction ) { //|| RmtTableProductionL2 ) {
                Dim1StartPoint[Channel][0][Strobe][LOW] = Dim1StartPoint[Channel][0][Strobe][LOW] - StepSize;
                Dim1StartPoint[Channel][0][Strobe][HIGH] = Dim1StartPoint[Channel][0][Strobe][HIGH] + StepSize;
              }
#endif //RMT_PRODUCTION_MODE_SUPPORT
              if (Dim1StartPoint[Channel][0][Strobe][LOW] > GetHalfClk (MrcData, Channel)){
                Low[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][LOW] - GetHalfClk (MrcData, Channel);
              } else {
                Low[Channel][Strobe] = 0;  // low limit for sweep  PT: TODO the low side limit should be the max of the (current value - halfclcok) and 0. TxDq has other restriction as well
              }
              High[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][LOW] + GetHalfClk (MrcData, Channel);        // halfclock like 104pi step should be enough
              // Peter TxVref is in the unit of percentage. Its range is from 110 to 420 which corresponds to 11% to 42%.
              if (RmtTablePtr[i].Signal == TxVref) {
                if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
                  Low[Channel][Strobe] = 110;
                  High[Channel][Strobe] = 420;
                } else { // TypeLpDdr3
                  Low[Channel][Strobe] = 0;
                  High[Channel][Strobe] = 63;
                }
                // PT In BXT-M, x16 device was used, BXT-P LPPDR4 platform used x32 device. In both cases, all strobes are control by
                // one TxVref control.
                if (Strobe != 0) {
                  Dim1StartPoint[Channel][0][Strobe][LOW] = Dim1StartPoint[Channel][0][0][LOW];
                  Dim1StartPoint[Channel][0][Strobe][HIGH] =  Dim1StartPoint[Channel][0][0][HIGH];
                  // also need to fix the TempValue, which is used by the GetLowHighMarginPerStrobe() as the training value.
                  TempValue[Channel][Strobe] = TempValue[Channel][0];
                }
              }
              //MmrcDebugPrint ((MMRC_DBG_MIN, "save ch:%d St:%d val:0x%x \n", Channel, Strobe, TempValue[Channel][Strobe]));
            } else {  // Signals which are part of signal groups (e.g. CMD_ALL, CTL_ALL,..)
              //
              // For non-per-bytelane params (e.g. cmd) we have to find the max and min PI among the PI groups, to determine the Low/High limit properly
              //  Create1DSweep only pays attention to the lowest PI group during the sweep in terms of detecting limits.  So have to make sure it won't go beyond 0.
              //  Note we are also assuming for non-per-bytelane params, that if there are NumElements >= 1, then the Signals are all adjacent in order, as far as GetSetDataSignal2 is concerned
              //
              // For CmdGrp0, CmdGrp1 and other non-per-bytelane param, only the strobe 0 is valid. Technically the strob1 of the CmdGrp0 is the value of CmdGrp1, but it is not
              // recommended.
              GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, PiRestoreSignal + Strobe, CMD_GET_REG, &TempValue[Channel][Strobe]);
              //
              // Update the cache with the values from the registers. This is a "just in case" condition.
              //
              GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, PiRestoreSignal + Strobe, CMD_SET_VAL_UC, &TempValue[Channel][Strobe]);
              if (CmdMin[Channel] > TempValue[Channel][Strobe]) {
                CmdMin[Channel] = (UINT16) TempValue[Channel][Strobe];
              }
              if (CmdMax[Channel] < TempValue[Channel][Strobe]) {
                CmdMax[Channel] = (UINT16) TempValue[Channel][Strobe];
              }
              Dim1StartPoint[Channel][0][0][LOW]   = CmdMin[Channel];
              Dim1StartPoint[Channel][0][0][HIGH]  = CmdMax[Channel];
              // save to the strobe group 0 only. Cmd/Ctl margin break at the first strobe group, unlike DQ margin till all strobe groups.
              if (Dim1StartPoint[Channel][0][0][LOW] > GetOneClk (MrcData, Channel)){
                Low[Channel][0] = Dim1StartPoint[Channel][0][0][LOW] - GetOneClk (MrcData, Channel);
              } else {
                Low[Channel][0] = 0;
              }
              High[Channel][0] = Dim1StartPoint[Channel][0][0][HIGH] + GetOneClk (MrcData, Channel); // PT One clock like 208 PI step should be enough
            }  // SkipInitialProgramming = TRUE
            if (! (MrcData->MrcRmtEnableSignal & RMT_FULL_EV_MODE))
            {
              //
              // Adjust Low / High limits so that we only test 1 point, if not in FULL EV MODE (e.g. *are* in production mode)
              //
#if RMT_PRODUCTION_MODE_SUPPORT == 1
              if((RmtTablePtr == RmtTableProduction ) && (RmtTablePtr[i].SkipInitialProgramming == FALSE)) {
                if (Dim1StartPoint[Channel][0][Strobe][LOW]  > Low[Channel][Strobe])
                {
                  Low[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][LOW] ;
                }
                if (Dim1StartPoint[Channel][0][Strobe][HIGH]  < High[Channel][Strobe])
                {
                  High[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][HIGH];
                }
              }
              else
#endif //RMT_PRODUCTION_MODE_SUPPORT
              {
                if (Dim1StartPoint[Channel][0][Strobe][LOW] -  StepSize > Low[Channel][Strobe])
                {
                  Low[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][LOW] - StepSize;
                }
                if (Dim1StartPoint[Channel][0][Strobe][HIGH] + StepSize < High[Channel][Strobe])
                {
                  High[Channel][Strobe] = Dim1StartPoint[Channel][0][Strobe][HIGH] + StepSize;
                }
              }
            }
          }   // Strobe loop ...
        } // Channel loop ...

        // The Create1DSweepLastPass takes care of running Channel in parallel.
        //
        // Create1DSweepLastPass handles margining 'offsets' from trained values.  It handles it natively for per bytelane parameters
        //   for other parameters (cmd,ctl,..) it is more convoluted.  It requires SkipInitialProgramming=TRUE in order to maintain a delta between PI's that are swept in tandem
        //      and it also requires the cpgc test (ExecuteTest) to restore the PI's to their original position upon failure, and apply JEDEC reset if needed, since SkipInit..=TRUE
        //      As a future improvement it might be more logical to have Create1DSweep.. handle JEDEC reset, restore of PI parameter, etc..    There could be a flag to tell it what kind of JEDEC reset is needed
        //        right now it's very confusing that CpgcTest is expected to do this restore.  And it's expected to do it in a way that is consistent with SkipInitialProgramming and Dim1StartPoint.  More logical for Create1DSweep to handle it
        //      Another improvement is to treat 'Elements' properly.  E.g. Currently CMD , which might have 2 PI groups, is treated as if it has 1 PI group, as far as Dim1StartPoint, etc..
        //      Also be nice if sweep would record the initial settings of what it is margining, and restore those values at the end.  There can be a flag to control that if you want
        //      efficiency in cases you don't care to restore.  But this is more encapsulated behavior.
        //   Another oddity of 'SkipInitialProgramming' .. if true, it needs linear values in Dim1StartPoint, if false , it doesn't matter but you need to be consistent when processing final margin delta's
        // Global parameters which allow CpgcExecuteRmtTest to do JEDEC reset and restore margined parameter to center point (very hack-ish , but consistent with way MRC is doing things right now)
        //   there is assumption that 'cached' values are still the trained values, so that routine can restore using those.  this is consistent with existing mrc approach.
        //
        MrcData->CpgcOptions.RmtRestoreJedecResetType   = RmtTablePtr[i].JedecResetType;
        MrcData->CpgcOptions.RmtRestoreSignalCount      = PiGroupCount;
        MrcData->CpgcOptions.RmtRestoreSignalGroupBegin = PiRestoreSignal;
        //
        // FODO: For pseudo-ranks (TA/LFSR) , on designs with per rank training, need Sweep function to margin ALL ranks
        //
        // Sandy : For LP3 TxVref needs to be executed sequentially than in parallel for Ch0 and Ch2 because the training calls only Ch0 and margins ch0 and 2 simultaneously. Whereas RMT needs to wait for
        //all lane failure and hence it makes sense to execute each channel sequentially.
        //Pseudocode
        //If(!(LP3 & TxVref))
        //  Create1DSweepLastPass() on all channels
        //if (LP3 & Txvref)
        //  run the channel loop
        //    check if channel is enabled
        //      mask all other channels
        //        call Create1DSweepLastPass()

        if (!((RmtTablePtr[i].Signal == TxVref) && (DramType == TypeLpDdr3))) {
          Create1DSweepLastPass(MrcData, ChannelBackup, Rank, RmtTablePtr[i].Signal, 1, Dim1StartPoint, Low, High, StepSize,
            FALSE, RmtTablePtr[i].SkipInitialProgramming, EnblePerBitSweep, PiMarginGroupCount, CpgcExecuteRmtTest, Results, SignalInfo[GetSignalInfoIndex(Signal)].Name, 0); // PT don't have signal name    Need to change to use CpgcExecuteRMTTest
        }


        // If its LP3 and TxVref then execute 1 channel at a time
        if ((RmtTablePtr[i].Signal == TxVref) && (DramType == TypeLpDdr3) && (!SkipTxVref)){    // Added !SkipVref so that the code not execute until late fix which will set the SkipVref to FALSE for LPDDR3
           for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
             if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled == FALSE) {
               continue;
             }
             for (ChannelTemp = ChannelBackup; ChannelTemp < MAX_CHANNELS; ChannelTemp++) {
               if (ChannelTemp == Channel)
                 MrcData->NvData.MrcParamsSaveRestore.Channel[ChannelTemp].ExecuteOnThisChannel = TRUE;
               else
                 MrcData->NvData.MrcParamsSaveRestore.Channel[ChannelTemp].ExecuteOnThisChannel = FALSE;
             }//ChannelTemp
             Create1DSweepLastPass(MrcData, Channel, Rank, RmtTablePtr[i].Signal, 1, Dim1StartPoint, Low, High, StepSize,
               FALSE, RmtTablePtr[i].SkipInitialProgramming, EnblePerBitSweep, PiMarginGroupCount, CpgcExecuteRmtTest, Results, SignalInfo[GetSignalInfoIndex(Signal)].Name, 0);

           }//channel

          for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel = ExecuteOnThisChannelOriginal[Channel];
          } // channel
        } //close the TxVref && LPDDR3 loop here

        // TODO: Ron make sure below code works now, since Sean fixed linear values always
        //for (Strobe = 0; Strobe < PiGroupCount; Strobe++) {
        //  //
        //  // FODO: These values are already in Dim1StartPoint - so this is a little redundant and takes an extra variable, this could be optimized away
        //  //   If GetLowHighMarginPerStrobe would take Dim1StartPoint (which  has HIGH/LOW val for each PI) to calculate margins , you could pass that in directly
        //  //   Another note .. for non-STROBE margin params, we are passing in possibly multiple values of TempValue2 (one for each PI) but GetLowHighMarginPerStrobe
        //  //     will loop-break after calculating margin relative to the first PI group only.
        //  //
        //}
        for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
          if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel || !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
            continue;
          }

          if (!ChannelEnabledForTestedRank[Channel]) {     // skip if this channel didn't support the PseudoRank
            continue;
          }

          if (i < MAX_RMT_ELEMENTS) {
            GetLowHighMarginPerStrobe (MrcData, Channel, PseudoRank, Results, TempValue[Channel], RmtTablePtr[i].Signal, i);      // Use PseudoRank here to store the margin result.
          }
          //
          // TODO: GetLowHighMarginPerStrobe has a funny behavior that I also saw in VLV2 RMT.  For some reason instead of reporting the bytelane with worst margin (relative to trained value)
          //   it instead reports the margin of the bytelane with worst margin relative to RMT perceived center (instead of trained center).  The margin reported is relative to trained center
          //   but the determination of w.c. bytelane depends on RMT perceived center.  So this could result in reporting the incorrect bytelane's margin as final RMT margin if one bytelane had poor trained center value
          //   sent email to Jorge/Alejandra to fix that
          // TODO: GetLowHigharginPerStrobe has an apparent bug in how it handles non-STROBE results.  It will report highside margin on CMD,CTL,.. incorrectly right now
          //   sent email to Jorge/Alejandra to fix that
          //

          //
          // There is no guarantee that the 1D sweep restored the values during the sweep, so do that here.  Even if CPGC test restores values on fail, it's possible we ended on a high limit without failing
          //
          // Peter: I find the CMD/CTL/CK, in particaular the CK need to use the routine we use in CpgcExecuteRmtTest to restore the value.
          if (RmtTablePtr[i].SkipInitialProgramming == TRUE) {
            for (Strobe = 0; Strobe < MrcData->CpgcOptions.RmtRestoreSignalCount; Strobe++) {
              GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->CpgcOptions.RmtRestoreSignalGroupBegin + Strobe, CMD_GET_CACHE,  &Temp);
              GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, MrcData->CpgcOptions.RmtRestoreSignalGroupBegin + Strobe, CMD_SET_VAL_FC, &Temp);
            }
          } else {
            for (Strobe = 0; Strobe < PiGroupCount; Strobe++) {
              if ((RmtTablePtr[i].Signal == TxVref) && (Strobe != 0)) continue;  // Only apply TxVref on the strobe 0 because it share with the rest of strobes.
              GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, RmtTablePtr[i].Signal, CMD_SET_VAL_FC_UC, &TempValue[Channel][Strobe]);  // PT: Strobe 1 will go to CmdGrp1 in the case of CmdAll
              //MmrcDebugPrint ((MMRC_DBG_MIN, "restore ch:%d St:%d val:0x%x \n", Channel, Strobe, TempValue[Channel][Strobe]));
            }
          }

          PerformFifoReset (MrcData, Channel, Rank);
        } // Channel loop ...

        //
        // Do a test to confirm memory interface is stable
        //
        for (Retry = 0; Retry < POST_RMT_RETRIES; Retry ++)
        {
          MmrcExecuteTask (MrcData, CPGC_STOP_CLEAR_ERR, NO_PRINT, ChannelBackup);
          MmrcExecuteTask (MrcData, CPGC_START_TEST, NO_PRINT, ChannelBackup);
          MmrcExecuteTask (MrcData, CPGC_GET_TEST_RESULT, NO_PRINT, ChannelBackup);

          // get error status
          IsErrInAnyChannel = FALSE;
          for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
            if (!RunOnThisChannel (MrcData, Channel, Rank)) {
              continue;
            }

            Errors = (UINT16) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
            if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
              Errors |= (UINT16)(MrcData->DynamicVars[Channel][ECC_ERR_STAT] << 8);
            }

            if (Errors !=0 ) {
              //MmrcDebugPrint ((MMRC_DBG_MIN, "final test failed at ch:%d Retryt:%d\n", Channel, Retry));
              IsErrInAnyChannel = TRUE;
            }
          }// Channel

          if (IsErrInAnyChannel == FALSE){
            break;
          }  else {
            PerformFifoReset (MrcData, Channel, Rank);
            MrcData->ExecuteThisRoutineInParallel=FALSE;
            CompleteJedecInit(MrcData, 0, 0, Channel);
            MrcData->ExecuteThisRoutineInParallel=TRUE;
          }
        }// Retry

        if (IsErrInAnyChannel) {
          if ( MrcData->MrcRmtSupport & RMT_SUPPORT_COMMAND_MODE ) {
            //
            // If command mode is selected, return to command loop, and hope user fixes issue
            //
            return MMRC_SUCCESS;
          }
          MmrcDebugPrint ((MMRC_DBG_MIN, "RMT not able to restore its state  \n"));
          //MrcDeadLoop ();
        }

        // PT need to reset the CADB setting after testing RMT signal with CPGC_CONFIG_CMD_STRESS. Otherwise
        // it affect the following test with CPGC_CONFIG_VA. The L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_VA)
        // doesn't reset the CADB CMD stress setting.

        if (RmtTablePtr[i].CpgcConfig == CPGC_CONFIG_CMD_STRESS) {
          MrcData->CpgcOptions.CADB_Harrassment = FALSE;
          // PT with the Harrassment = False, the following call rest all CADB setting.
          MmrcExecuteTask (MrcData, CPGC_DESELECT_DIS, 0, ChannelBackup);
        }
      }  // Rmt Table loop
    } // Rank loop

    //
    // Restore debug messages.
    //
    MrcData->MrcDebugMsgLevel = DebugMsgLevel;
    MmrcDebugPrint ((MMRC_DBG_MIN, "START_RMT: \n          "));
    // print the header
    i = (UINT8) -1;
    while (RmtTablePtr[++i].Signal != MMRC_NA) {
      // skip TxVref for the DDR3L platform
      if ((RmtTablePtr[i].Signal == TxVref) && (SkipTxVref)){
        continue;
      }
      Signal = GetOffsetIndex(MrcData, RmtTablePtr[i].Signal);
      MmrcDebugPrint ((MMRC_DBG_MIN, " %s     ", SignalInfo[GetSignalInfoIndex(Signal)].Name));
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));

    // display the rank margin result:
    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel || !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
        continue;
      }
      for (PseudoRank = 0; PseudoRank < (MAX_RANKS + PSEUDO_RANKS); PseudoRank++) {
        if ((MrcData->MrcRmtEnableSignal >> Rmt_Rank0_Disable) & (1 << PseudoRank)) {
          continue;
        }
        if (PseudoRank < MAX_RANKS) {
          Rank = PseudoRank;
          if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
            continue;
          }
#if RMT_PER_BIT_MARGIN
          for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
            for (Bit = 0; Bit < MAX_BITS; Bit++) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "C%d.R%d.L%02d:", Channel, Rank, (Bit + MAX_BITS * Strobe)));
#else
              MmrcDebugPrint ((MMRC_DBG_MIN, "C%d.R%d:    ", Channel, Rank));
#endif
              i = (UINT8) -1;
              //
              // Display all signals we just margined.
              //
              while (RmtTablePtr[++i].Signal != MMRC_NA) {
              // skip TxVref for the DDR3L platform
                if ((RmtTablePtr[i].Signal == TxVref) && (SkipTxVref)){
                  continue;
                }
#if RMT_PRODUCTION_MODE_SUPPORT == 1
                // Production mode global pass/fail global status
                if (RmtTablePtr == RmtTableProduction) {
                  if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[PseudoRank][i][LOW] == 0) || (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[PseudoRank][i][HIGH] == 0)) {
                    if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtCheckRun == L1_CHECK) || (MrcData->MrcRmtCallL1Check == TRUE)) {
                      if (MrcData->MrcRmtMarginResultOld == TRUE) { //If RmtCheckRUn = 3 and L2 failed
                        MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = BOTH_CHECK;
                      } else {
                        MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = L1_CHECK;   //Fail for L1 check
                      }
                    } else if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtCheckRun == L2_CHECK) {
                      MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = L2_CHECK;   //Fail for L2 check
                    } else if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtCheckRun == BOTH_CHECK) {
                      MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = L2_CHECK; //Fail for L2 check because by default L2 will run first if RmtCheckRun is set to 3
                    } else  { //If RmtCheckRun = 0 (NO_RMT)
                      MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = NO_RMT;
                    }
                  } //Rank margin check
                } //RmtProduction
#endif //RMT_PRODUCTION_MODE_SUPPORT

#if RMT_PER_BIT_MARGIN
                MmrcDebugPrint ((MMRC_DBG_MIN, "%4d  %4d     ",MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[PseudoRank][i][Bit + MAX_BITS * Strobe][LOW], MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_DQData[PseudoRank][i][Bit + MAX_BITS * Strobe][HIGH]));
#else
                MmrcDebugPrint ((MMRC_DBG_MIN, "%4d  %4d     ", MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[PseudoRank][i][LOW],  MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RMT_Data[PseudoRank][i][HIGH]));
#endif // RMT_PER_BIT_MARGIN
              }
              MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
#if RMT_PER_BIT_MARGIN
            } // Bit
          } // strobe
#endif
        } //   if (PseudoRank < MAX_RANKS) {
      } // Rank loop ...
    } // Channel loop ...

    MmrcDebugPrint ((MMRC_DBG_MIN, "STOP_RMT: \n"));
#if RMT_PRODUCTION_MODE_SUPPORT == 1
    MmrcDebugPrint ((MMRC_DBG_MIN, "PRODUCTION_MODE margin result : %4d \n",MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState ));
#endif //RMT_PRODUCTION_MODE_SUPPORT
  }  // if RMT support

  //
  // Turn off the execution in parallel.
  MrcData->ExecuteThisRoutineInParallel = ExecuteInParallelOriginal;
#endif // RMT == 1
  return MMRC_SUCCESS;
}



#if TRAINING_ALGOS == 1
MMRC_STATUS
Create2DEye (
  IN          MMRC_DATA       *MrcData,
  IN   OUT    SAMPLE_SWEEP    *Results,
  IN          UINT8           Channel,
  IN          UINT8           Rank,
  IN          UINT16          Dim1Index,
  IN          UINT8           Dim1Knobs,
  IN          UINT16          Dim1Min[MAX_CHANNELS][MAX_STROBES],
  IN          UINT16          Dim1Step,
  IN          UINT16          Dim2Index,
  IN          UINT16          Dim2Min[MAX_CHANNELS][MAX_STROBES],
  IN          UINT16          Dim2Step
)
{
  UINT8   StringToPrint[MAX_LINE_SIZE + 1];
  UINT8   Strobe;
  INT16   AxisY;
  INT16   AxisX;
  UINT8   i;
  UINT16  CenterY;
  UINT16  OrigCenterY;
  UINT16  CenterX[MAX_KNOBS];
  UINT16  AxisXResult[2];
  UINT8   Knob;
  UINT32  TempValue;
  UINT16  StepSize;
  //
  // Step size should be 1/2 clock divided by 64.
  //
  StepSize = GetHalfClk (MrcData, Channel) / 64;

  if (StepSize == 0) {
    //
    // Minimum StepSize should be '1'
    //
    StepSize = 1;
  }

  TempValue = 0;
  MmrcMemset(StringToPrint, 0, MAX_LINE_SIZE + 1);
  for (; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }

      MmrcDebugPrint ((MMRC_DBG_MIN, "C%d.R%d.S%d:\n  ", Channel, Rank, Strobe));
      for (i = 0; i < 8; i++) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "DQ%d, ", Strobe * 8 + 7 - i));
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));

      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Dim1Index + Knob, CMD_GET_CACHE, &TempValue);
        CenterX[Knob] = (UINT16) TempValue;
      }
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Dim2Index, CMD_GET_CACHE, &TempValue);
      CenterY = (UINT16) TempValue;
      OrigCenterY = CenterY;
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        CenterY = OrigCenterY;
        //
        // Print the real linear value before doing the adjust
        //
        MmrcDebugPrint ((MMRC_DBG_MIN, "  Cx: %d Cy: %d\n  ", CenterX[Knob], CenterY));
        //
        // During Create2DSweep the value in Axis Y always starts from '0', so it is needed to
        // to be adjusted during Center2D to get the good center in Y, so for display the 2D
        // eye the Axis 2D Min needs to be subtracted again
        //
        CenterX[Knob] -= Dim1Min[Channel][Strobe];
        CenterY -= Dim2Min[Channel][Strobe];
        //
        // Adjust to the Previous number which could be divided by Dim2Step
        //
        CenterY = CenterY - CenterY % Dim2Step;

        for (AxisY = (MAX_ELEMENTS_DIM_Y - 1); AxisY >= 0; AxisY -= 1) {
          if (Results[AxisY].Sample[Channel][Knob][Strobe][HIGH] <= Results[AxisY].Sample[Channel][Knob][Strobe][LOW]) {
            continue;
          }
          AxisXResult[HIGH] = Results[AxisY].Sample[Channel][Knob][Strobe][0][HIGH] - Dim1Min[Channel][Strobe];
          AxisXResult[LOW]  = Results[AxisY].Sample[Channel][Knob][Strobe][0][LOW] - Dim1Min[Channel][Strobe];

          for (AxisX = 0; AxisX < (MAX_LINE_SIZE * StepSize); AxisX += StepSize) {
            if (AxisY == CenterY && ((AxisX <= CenterX[Knob]) && ((AxisX + StepSize) > CenterX[Knob]))) {
              StringToPrint[AxisX / StepSize] = '+';
            } else if ((AxisX <= AxisXResult[HIGH]) && ((AxisX + StepSize) > AxisXResult[HIGH])) {
              //
              // Right edge
              //
              StringToPrint[AxisX / StepSize] = '*';
              StringToPrint[(AxisX / StepSize) + 1] = '\0';
              break;
            } else if ((AxisX <= AxisXResult[LOW]) && ((AxisX + StepSize) > AxisXResult[LOW])) {
              //
              // Left edge
              //
              StringToPrint[AxisX / StepSize] = '*';
            } else {
              //
              // Left of eye or in the eye
              //
              StringToPrint[AxisX / StepSize] = ' ';
            }
          }
          MmrcDebugPrint ((MMRC_DBG_MIN, "%s\n", (UINT8 *) StringToPrint));
        } // AxisY loop ...
      } // Knob Loop
    } // Strobe loop ...
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  return MMRC_SUCCESS;
}
#endif // TRAINING_ALGOS == 1


/**
  Restores the values of the CMD training for the Dunit

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CommandClockEntry (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1 && NEW_RTWT_ALGO == 0
  MrcData->DynamicVars[Channel][PATH_ENABLE] = 0;
  MmrcExecuteTask (MrcData, COMMANDCLOCKTRAINING_ENTRY, NO_PRINT, Channel);
#endif // COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  return MMRC_SUCCESS;
}

#ifdef LCT_DEBUG
MMRC_STATUS
CADumpAndTest(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT32 CmdValue;
  UINT32 CtlValue;
  UINT32 WrVValue;
  UINT32 CaVValue;

  UINT8 ch, rk;

  for (ch=0; ch < MAX_CHANNELS; ch++)
  {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled)
    {
      for (rk=0; rk < MAX_RANKS; rk++)
      {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].RankEnabled[rk])
        {
          GetSetDdrioGroup2 (MrcData, Channel, rk, 0, CmdGrp0, CMD_GET_REG, (UINT32 *)(&CmdValue));
          GetSetDdrioGroup2 (MrcData, Channel, rk, 0, CtlGrp0, CMD_GET_REG, (UINT32 *)(&CtlValue));
          GetSetDdrioGroup2 (MrcData, Channel, rk, 0, TxVref , CMD_GET_REG, (UINT32 *)(&WrVValue));
          GetSetDdrioGroup2 (MrcData, Channel, rk, 0, TxCaVref, CMD_GET_REG, (UINT32 *)(&CaVValue));
          //MmrcDebugPrint ((MMRC_DBG_MIN, "\nCADumpAndTest: C:%d R:%d CMD:%d CTL:%d WRV:%d CAV:%d\n", ch, rk, CmdValue, CtlValue, WrVValue, CaVValue));
        }
      }
      CpgcVATest (MrcData,NO_PRINT, NO_PRINT, ch);
    }
  }
  return MMRC_SUCCESS;
}
#endif

/**
  Restores the values of the CMD training for the DDRIO

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CommandClockControlRestore (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1 && NEW_RTWT_ALGO == 0
  UINT8   Index;

  for (Index = 0; Index < MAX_CTLS; Index++) {
    DisplayOrRestoreTrainingResults (MrcData, Channel, CtlGrp0 + Index, FALSE); //NEW_RTWT_ALGO == 0
  }
  for (Index = 0; Index < MAX_CLKS; Index++) {
    DisplayOrRestoreTrainingResults (MrcData, Channel, ClkGrp0 + Index, FALSE); //NEW_RTWT_ALGO == 0
  }
  for (Index = 0; Index < MAX_CMDS; Index++) {
    DisplayOrRestoreTrainingResults (MrcData, Channel, CmdGrp0 + Index, FALSE);
  }
#endif // COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  return MMRC_SUCCESS;
}

MMRC_STATUS
LateCommandTraining (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channelx
  )
{
#ifdef BXTPMRC
#ifndef MODE_2N_CONFIG
  UINT8 ch, rk;
  UINT32 Value = 0;
  BOOLEAN TrainCmdMode = TRUE;
  UINT32 OriginalValue[MAX_CHANNELS][MAX_RANKS];
#endif // MODE_2N_CONFIG
#endif

  if (MrcData->LctDisable == 1) {
    return MMRC_SUCCESS;
  }

#ifdef BXTPMRC
#ifndef MODE_2N_CONFIG
  if (MrcData->CpuStepping == STEPPING_P0) {
    // disable 1N/2N Mode Training on BXTP A-step
    TrainCmdMode = FALSE;
  } else {
    if (ConfigToDdrType[MrcData->MrcMemConfig] != TypeDdr3L) {
      // Only perform LCT on DDR3L memory
      MmrcDebugPrint((MMRC_DBG_MIN, "BXTP-B0 LCT Disabled\n"));
      return MMRC_SUCCESS;
    }
  }

  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[ch].Mode2N == 1){
      // When 2N Mode is requested, simply turn off the CMD Mode training.
      // This will prevent LCT from turning on 1N mode.
      TrainCmdMode = FALSE;
    }
  }

  if (TrainCmdMode == TRUE) {
    //
    // Save the Command and Control value to be programmed when failed
    //
    for (ch = 0; ch < MAX_CHANNELS; ch++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
        for (rk = 0; rk < MAX_RANKS; rk++) {
          GetSetDdrioGroup2(MrcData, ch, rk, 0, CmdGrp0, CMD_GET_REG_UC, &OriginalValue[ch][rk]);
        }
      }
    }
    MrcData->KeepAlive = 1;
    Value = 1;
    for (ch = 0; ch < MAX_CHANNELS; ch++) {
      GetSetTimingMode(MrcData, 0, ch, 0, 0, 0, 0, 0, 0, CMD_SET_VAL_FC_UC, &Value);
    }
  }
#endif // MODE_2N_CONFIG

  CommandClockTraining (MrcData,CapsuleStartIndex,StringIndex,0);

#ifndef MODE_2N_CONFIG
  if (TrainCmdMode == TRUE) {
    if (MrcData->KeepAlive > 1) {
      //
      // Restore 2N values in retrain flow
      //
      for (ch = 0; ch < MAX_CHANNELS; ch++) {
        if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
          for (rk = 0; rk < MAX_RANKS; rk++) {
            GetSetDdrioGroup2(MrcData, ch, rk, 0, CmdGrp0, CMD_SET_VAL_FC_UC, &OriginalValue[ch][rk]);
          }
        }
      }
      //GetSetTimingMode( MrcData, CMD_SET_VAL_FC_UC 2);
      Value = 2;
      for (ch = 0; ch < MAX_CHANNELS; ch++) {
        GetSetTimingMode(MrcData, 0, ch, 0, 0, 0, 0, 0, 0, CMD_SET_VAL_FC_UC, &Value);
      }
      //
      // CompleteJedecInit resets all channels, only call once.
      //
      CompleteJedecInit(MrcData, 0, 0, 0);
      MrcData->KeepAlive = 0;

      CommandClockTraining(MrcData, CapsuleStartIndex, StringIndex, 0);
    }
  }
#endif // MODE_2N_CONFIG
#else // BXTPMRC
  CommandClockTraining(MrcData, CapsuleStartIndex, StringIndex, 0);
#endif // BXTPMRC
  return MMRC_SUCCESS;

}

/**
  Performs the Command Training.  Based on the DIMM configuration (1DPC/2DPC),
  the training will be completed in 1N/2N mode. No assumption is
  made on the beginning mode for the Command bus; however,
  memory should be stable prior to entering. Channel is the only
  additional parameter passed in, and the calibration will be
  performed on the channel specified.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CommandClockTraining(
IN  OUT MMRC_DATA         *MrcData,
IN      UINT16            CapsuleStartIndex,
IN      UINT16            StringIndex,
IN      UINT8             Channelx
)
{
#if COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
#if NEW_RTWT_ALGO == 1
  CONTROLKNOBS ControlKnobs[3];
  UINT16       Voh;
  UINT8        Loop;
  UINT16       Vref;
  UINT16       PercentageOfVddq;
  UINT8        ch;
  UINT8        i;
  UINT8        rk;
  UINT32       TempValue;
  EYEMASK_T    LocalEyeMaskCmd[CMD_NUM_EYEMASKS];

  UINT32 LocalOdt = DramOdt[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentFrequency];
  UINT16 WrVrefOvr = DramVrefOvr[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentFrequency];

#ifndef REMOVE_LPDDR3_1866_DEBUG
  if ((ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr3) && (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency == MMRC_1866)) {
    MmrcDebugPrint((MMRC_DBG_MIN, "Temporarily disabling LCT (CommandClockTraining) on LPDDR3 1866\n"));
    return MMRC_SUCCESS;
  }
#endif

  //The following execution is necessary because otherwise LCT fails from the very begining after AdvWr and AdvRd finishes.So the following function initializes
  for (Loop = 0; Loop < MAX_CHANNELS; Loop++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Loop].Enabled == TRUE) {
      MmrcExecuteTask(MrcData, CPGC_ONE_TIME_INIT, NO_PRINT, Loop);
    }
  }

  //
  // Before beginning the algo or restoring the trained values, call the entry function to setup the
  // system.
  //
#ifdef COMMANDCLOCKTRAINING_ENTRY
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
      MmrcExecuteTask (MrcData, COMMANDCLOCKTRAINING_ENTRY, NO_PRINT, ch);
    }
  }
#endif

  MrcData->MaxBits = 1;
  MrcData->NumberOfTiming = 2;
 //Set this flag to false to initialize. This will be set to TRUE only at call for SweepData1 at Phase 3
  MrcData->LctNModeFlag = FALSE;


  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4) {
    Mmrcmemcpy(ControlKnobs, ControlKnobsCmdCtl, sizeof(ControlKnobs));

    if (LDO_ENABLED == 1) {
      Voh = (UINT16)(LDO_TARGET * (LocalOdt / (LocalOdt + RON)));
    } else {
      Voh = (UINT16)(VDDQ * LocalOdt / (LocalOdt + RON));
    }
    Vref = Voh / 2;
    PercentageOfVddq = Vref * 100 / VDDQ;
    if (WT_VREF_OVERRIDE_ENABLE == 1) {
      PercentageOfVddq = WrVrefOvr;
    }
    MmrcDebugPrint ((MMRC_DBG_MED, "ODT = %d, Vddq% = %d, WrVrefOvr = %d\n", LocalOdt, PercentageOfVddq, WrVrefOvr));
    //
    // Program the new VREF ranges.
    //
    ControlKnobs[MrcData->NumberOfTiming].Ranges[HIGH] = 370;
    if (PercentageOfVddq > 42) {
      ControlKnobs[MrcData->NumberOfTiming].Ranges[LOW] = 400;
    } else {
      ControlKnobs[MrcData->NumberOfTiming].Ranges[LOW] = 320;//(PercentageOfVddq - (40 - PercentageOfVddq + 3)) * 10; //300 Manuel
    }
    ControlKnobs[MrcData->NumberOfTiming].Phase2_VrefStart = ControlKnobs[MrcData->NumberOfTiming].Ranges[LOW];
    ControlKnobs[MrcData->NumberOfTiming].Phase2_VrefStop = ControlKnobs[MrcData->NumberOfTiming].Ranges[HIGH];
    ControlKnobs[MrcData->NumberOfTiming].StartValue[LOW] = ControlKnobs[MrcData->NumberOfTiming].Ranges[LOW];
    ControlKnobs[MrcData->NumberOfTiming].StartValue[HIGH] = ControlKnobs[MrcData->NumberOfTiming].Ranges[HIGH];
    //
    // Check if the eyemask vrefs can even be attempted.  Flag error if it can't.
    //
    if ((ControlKnobs[MrcData->NumberOfTiming].Ranges[HIGH] - ControlKnobs[MrcData->NumberOfTiming].Ranges[LOW]) / ControlKnobs[MrcData->NumberOfTiming].CriteriaStep < EyeMaskCmd[0].NumberConsecutiveVoltages - 1) {
      MmrcDebugPrint ((MMRC_DBG_MED, "Late CA Training: Unable Pass Minimum EM Criteria.\n"));
      return MMRC_FAILURE; //while(1);
    }
  }
#if LPDDR3_SUPPORT
  else if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr3) {
    Mmrcmemcpy(ControlKnobs, ControlKnobsCmdCtl_LPDDR3, sizeof(ControlKnobs));
  }
#endif
#if DDR3_SUPPORT
  else {
    Mmrcmemcpy(ControlKnobs, ControlKnobsCmdCtl_DDR3, sizeof(ControlKnobs));
  }
#endif

  //
  // If on the full boot path (not restore path), setup CPGC for Write training
  // and call the training algorithm, otherwise restore the passed in values.
  //
  if (!MrcData->RestorePath) {
    MrcData->CpgcOptions.LoopCount = MrcData->MrcTrainingCpgcExpLoopCnt +2 ; //This is set in McFunc.c LC=8, NB=12. AdvWr does 7,7 and AdvRd does 8,8
    //Numburst hardcoded to 7 in L_CpgcSetup because Select cycles are not using CPGC_ADDR_PRBS register which was added in BXT-P upon EV's request. Instead its using Address decode
    //addr inversion registers and there is limitation on Numburst if one uses combination of those registers to generate interesting stress on select cycles. For GLK we should
    //use ADDR_PRBS register atleast for CMD CLK training.
    MrcData->CpgcOptions.NumBursts = MrcData->MrcCpgcNumBursts;
    MrcData->CpgcOptions.CADB_Harrassment = TRUE; //Manuel
    for (ch = 0; ch < MAX_CHANNELS; ch++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
        L_CpgcSetup(MrcData, ch, CPGC_CONFIG_CMD_STRESS);
        //Save the Command and Control value to be programmed when failed
        //
        for (rk = 0; rk < MAX_RANKS; rk++) {
          for (Loop = 0; Loop < MAX_TIMING; Loop++) {
            GetSetDdrioGroup2(MrcData, ch, rk, 0, ControlKnobs[Loop].Index, CMD_GET_REG_UC, &TempValue);

          }

        }
      }
    }
    for (i = 0; i < CMD_NUM_EYEMASKS; i++)
    {
      if (ConfigToDdrType[MrcData->MrcMemConfig] <= TypeDdr3All) {
        LocalEyeMaskCmd[i].NumberConsecutiveVoltages = 1;
      } else {
        LocalEyeMaskCmd[i].NumberConsecutiveVoltages = EyeMaskCmd[i].NumberConsecutiveVoltages;
      }
      LocalEyeMaskCmd[i].MinimumDelayWidth = (EyeMaskPctPiCmdCtl[MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType][MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency][i] * GetHalfClk(MrcData, 0)) / 100;
      MmrcDebugPrint((MMRC_DBG_MIN, "Command Mask[%d], Vref=%d, DelayPis=%d\n", i, LocalEyeMaskCmd[i].NumberConsecutiveVoltages, LocalEyeMaskCmd[i].MinimumDelayWidth));
    }
    MrcData->MaxElements = 1;
    MrcData->CpgcOptions.SetupType = CPGC_CONFIG_CMD_STRESS;
    //
    // Perform the Advanced WriteTraining Algorithm with the Write Control knobs.
    //
    AdvancedTrainingAlgo (MrcData, ControlKnobs, NumberControlKnobsCmdCtl, LocalEyeMaskCmd, NumberEyeMaskCmd, FALSE);

    //The following code might be confusing that why are we setting CPGC with CMD stress after AdvAlgo. That is because CADB is free running and must be
    //stopped before the next training step. So when we set the flag as FALSE and call L_CpgcSetup function, it will disable the CADB

    MrcData->CpgcOptions.CADB_Harrassment = FALSE;
    for (Loop = 0; Loop < MAX_CHANNELS; Loop++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Loop].Enabled == TRUE) {
        L_CpgcSetup(MrcData, Loop, CPGC_CONFIG_CMD_STRESS);
        MmrcExecuteTask(MrcData, CPGC_ONE_TIME_INIT, NO_PRINT, Loop);
      }
    }
  }

#ifdef BXTPMRC
  // For DDR3L, if failed training, repeat in 2N.
  if (MrcData->KeepAlive > 1) {
    return MMRC_FAILURE;
  }
#endif

  //
  // Print out or restore the training results.
  //
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
#ifdef BXTPMRC
      DisplayOrRestoreTrainingResults(MrcData, ch, CmdCh0, TRUE, MAX_STROBES_NON_ECC);
      DisplayOrRestoreTrainingResults(MrcData, ch, CmdCh1, TRUE, MAX_STROBES_NON_ECC);
      DisplayOrRestoreTrainingResults(MrcData, ch, CtlCh0, TRUE, MAX_STROBES_NON_ECC);
      DisplayOrRestoreTrainingResults(MrcData, ch, CtlCh1, TRUE, MAX_STROBES_NON_ECC);
#else
      DisplayOrRestoreTrainingResults (MrcData, ch, CmdGrp0, TRUE, MAX_STROBES);
      DisplayOrRestoreTrainingResults (MrcData, ch, CtlGrp0, TRUE, MAX_STROBES);
#endif
    }
  }
  //
  // End with the exit task for write training.
  //
#ifdef COMMANDCLOCKTRAINING_EXIT
  for (ch = 0; ch < MAX_CHANNELS; ch++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Enabled == TRUE) {
      MmrcExecuteTask(MrcData, COMMANDCLOCKTRAINING_EXIT, NO_PRINT, ch);
    }
  }
#endif

  CompleteJedecInit(MrcData, 0, 0, 0);
#endif
#endif // COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1


  return MMRC_SUCCESS;
}

/**
  Prints the information gotten from the training results

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PhyViewTable (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if PHYVIEW_TABLE == 1
  UINT16 i;
  BOOLEAN Restore;

  //
  // Save the debug level and restore path values for PhyViewTable
  //
  Restore = MrcData->RestorePath;
  MrcData->RestorePath = FALSE;
  //
  // Loop through each delay, printing the values for each.
  //
  for (i = REG_INDEX; i < (REG_INDEX + NumberAlgos); i++) {
    if (GetOffsetIndex (MrcData, i) != REG_UNDEFINED) {
#if BXTPMRC
      DisplayOrRestoreTrainingResults (MrcData, Channel, i, FALSE, MAX_STROBES_NON_ECC);
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
        DisplayOrRestoreTrainingResults (MrcData, Channel, RecEnDelay_Ecc, FALSE, 1);
      }
#else
      DisplayOrRestoreTrainingResults (MrcData, Channel, i, FALSE, MAX_STROBES);
#endif
    }
  }
  //
  // Restore path values after PhyViewTable
  //
  MrcData->RestorePath = Restore;
#endif // PHYVIEW_TABLE
  return MMRC_SUCCESS;
}

/**
  Implements Diffamp, Odt and tRD Dynamic Formulas

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PerformanceSetting (
  IN  OUT MMRC_DATA         *MrcData,
  IN      UINT16            CapsuleStartIndex,
  IN      UINT16            StringIndex,
  IN      UINT8             Channel
  )
{
#if PERFORMANCE_SETTINGS
  UINT8  Rank;
  UINT8  Strobe;
  UINT16 TrainingDelay;
  UINT8  ElementIndex;
  UINT8  ThisSignalExists;
  UINT32 RcvnLinearValue;
  UINT32 MaxRcvn[MAX_STROBES];
  UINT32 MinRcvn[MAX_STROBES];
  UINT32 TempValue;
  UINT32 Denominator;
  UINT32 MaxtRDValue;
  UINT32 RcvEn1x;
  UINT32 RcvEn2x;
  UINT32 OdtDiffampDly[MAX_STROBES];
  UINT32 OdtDiffampLen[MAX_STROBES];

  if (!MrcData->PerformanceSettings) {
    return MMRC_SUCCESS;
  }

  //
  // Compute the maximum Receive Enable on each Rank.
  //
  Denominator = 2 * GetHalfClk (MrcData, Channel);
  TrainingDelay = GetOffsetIndex (MrcData, RecEnDelay);
  TrainingDelay = ((TrainingDelay - ALGO_REG_INDEX) * NumberOfElementsPerAlgo);
  //
  // Clear MaxtRDValue, MaxRcvn and MinRcvn to 0
  //
  MaxtRDValue = 0;
  RcvEn1x = 0;
  RcvEn2x = 0;
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    MaxRcvn[Strobe] = 0;
    MinRcvn[Strobe] = 0;
  }

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    //
    // For each strobe, restore the Rcvn value and the minus1 select.
    //
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
          continue;
        }
        ThisSignalExists = 0;
        RcvnLinearValue = 0;
        for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
          if (GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TrainingDelay + ElementIndex, CMD_GET_CACHE, &TempValue) == MMRC_SUCCESS) {
            RcvnLinearValue += TempValue * (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]);
            ThisSignalExists = ElementIndex;
            if (ElementIndex == 0) {
              RcvEn1x = TempValue;
            }
            if (ElementIndex == 1) {
              RcvEn2x = TempValue;
            }
          }
        } // ElementIndex loop ...
        if (GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, SignalInfo[GetSignalInfoIndex (TrainingDelay)].Minus1Index, CMD_GET_CACHE, &TempValue) == MMRC_SUCCESS) {
          if (TempValue == 1) {
            RcvnLinearValue += GetHalfClk (MrcData, Channel);
          }
        }
        //
        // Get the MAX
        //
        if (RcvnLinearValue > MaxRcvn[Strobe]) {
          MaxRcvn[Strobe] = RcvnLinearValue;
        }
        //
        // Get the MIN
        //
        if (MinRcvn[Strobe] == 0) {
          MinRcvn[Strobe] = RcvnLinearValue;
        } else if (RcvnLinearValue < MinRcvn[Strobe]) {
          MinRcvn[Strobe] = RcvnLinearValue;
        }
        TempValue = ((2 * (RcvEn1x + 3 + (RcvEn2x / 2) + 1)) + 1 + 1) / 2;
        if (TempValue > MaxtRDValue) {
          MaxtRDValue = TempValue;
        }
      }
    } // Rank loop ...
  } // Strobe loop ...

  MaxtRDValue = ((2 * (MaxtRDValue + 2 - 4) + 1 + 1) / 2) + 1;

  //
  // Rank is Don't care as the formula signals are per channel and strobe
  //
  Rank = 0;

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    //
    // MCHODT/Diffampen Launch Formula:
    // 2 is because we are pulling back from RCVEN value
    //
    OdtDiffampDly[Strobe] = (MinRcvn[Strobe] / Denominator) - 2;

    //
    // MCHODT/Diffampen Length Formula:
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L){
      //
      // 2 is because we are pulling back from RCVEN value
      // 5 is to cover the entire Read burst (BL8)
      // (2 + 5 + "2") extra "2" is for LPDDR3 tDQSCK variation of 2.3ns
      // (2 + 5 + 2 + "1") extra "1" is for guardband
      //
    } else {
      OdtDiffampLen[Strobe] = (UINT32)(((MaxRcvn[Strobe] - MinRcvn[Strobe]) / Denominator));
      //
      // 2 is because we are pulling back from RCVEN value
      // 5 is to cover the entire Read burst (BL8)
      // 2 is for guardband
      //
      OdtDiffampLen[Strobe] = OdtDiffampLen[Strobe] + 2 + 5 + 2;
    }
  }

  MmrcDebugPrint ((MMRC_DBG_MIN, "\nSignal    CRS  Value\n"));
  MmrcDebugPrint ((MMRC_DBG_MIN, "------    ---  -----\n"));

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    //
    // Program Max tRD value to all strobes
    //
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Trd, CMD_SET_VAL_FC, &MaxtRDValue);
    //
    // Perform a FIFO Reset after programmed tRD
    //
    TempValue = FIFO_RESET_ENABLE;
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, FifoReset, CMD_SET_VAL_FC, &TempValue);
    TempValue = FIFO_RESET_DISABLE;
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, FifoReset, CMD_SET_VAL_FC, &TempValue);
    MmrcDebugPrint ((MMRC_DBG_MIN, "tRD       %d-%d  %02d (DEC)\n", Channel, Strobe, MaxtRDValue));
  }

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, DiffampDelay, CMD_SET_VAL_FC, &OdtDiffampDly[Strobe]);
    MmrcDebugPrint ((MMRC_DBG_MIN, "DiffDly   %d-%d  %02d (DEC)\n", Channel, Strobe, OdtDiffampDly[Strobe]));
  }

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, DiffampLength, CMD_SET_VAL_FC, &OdtDiffampLen[Strobe]);
    MmrcDebugPrint ((MMRC_DBG_MIN, "DiffLen   %d-%d  %02d (DEC)\n", Channel, Strobe, OdtDiffampLen[Strobe]));
  }

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, MchOdtDelay, CMD_SET_VAL_FC, &OdtDiffampDly[Strobe]);
    MmrcDebugPrint ((MMRC_DBG_MIN, "OdtDly    %d-%d  %02d (DEC)\n", Channel, Strobe, OdtDiffampDly[Strobe]));
  }

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
      continue;
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, MchOdtLength, CMD_SET_VAL_FC, &OdtDiffampLen[Strobe]);
    MmrcDebugPrint ((MMRC_DBG_MIN, "OdtLen    %d-%d  %02d (DEC)\n", Channel, Strobe, OdtDiffampLen[Strobe]));
  }
  MmrcDebugPrint ((MMRC_DBG_MIN ,"\n"));
#endif // PERFORMANCE_SETTINGS
  return MMRC_SUCCESS;
}

/**
  Resets the DRAMs by reinitializing the Drams and re-performing JEDEC on all available ranks for the provided Channel.
  CPGCEnabled is a flag passed in that informs the function if it should disable/enable CPGC prior/post to performing JEDEC.
  CPGC MUST be disable during JEDEC.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       CPGCEnabled     Flag to set/clear CPGC during JEDEC.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ReInitializeDRAMs (
  IN OUT    MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         CPGCEnabled
  )
{
  BOOLEAN             ExecuteMode;

  ExecuteMode = MrcData->ExecuteThisRoutineInParallel;
  MrcData->ExecuteThisRoutineInParallel = FALSE;

  //
  // Perform a FIFO Reset.
  //
  MrcDelay (MrcData, MICRO_DEL, 10);
  PerformFifoReset (MrcData, Channel, Rank);
  //
  // If CPGC is enabled, then disable it.  This could attempt to read the value, but for now its based
  // on the input parameter CPGCEnabled.
  MrcDelay (MrcData, MICRO_DEL, 10);
  if (CPGCEnabled == 1) {
    MmrcExecuteTask (MrcData, CPGC_DISABLE, NO_PRINT, Channel);
  }
#if DDR3_SUPPORT
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType <= TypeDdr3All) {
  //
  // Reset the DRAMS by going thru the DDRIO / dunit.
  //
  MmrcExecuteTask (MrcData, RESETDRAMS, NO_PRINT, Channel);
  PerformFifoReset (MrcData, Channel, Rank);
  //
  // Make sure ODT is low during JEDEC Init.
  //
  MmrcExecuteTask (MrcData, FORCEODT_OFF, NO_PRINT, Channel);
  //
  // Perform JEDEC on all ranks for the channel.
  //
  //MmrcExecuteTask (MrcData, JedecInit, NO_PRINT, Channel);
  InitializeJedec (MrcData, 0, 0, Channel);
  //
  // Release ODT to the system.
  //
  MmrcExecuteTask (MrcData, FORCEODT_REL, NO_PRINT, Channel);
  }
#endif
#if LPDDR3_SUPPORT
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    //
    // Perform JEDEC on all ranks for the channel.
    //
    //MmrcExecuteTask (MrcData, JedecInit, NO_PRINT, Channel);
    InitializeJedec (MrcData, 0, 0, Channel);
  }
#endif
  //
  // If CPGCEnabled flag is set, then reenable CPGC.
  //
#ifndef BXTPMRC
  if (CPGCEnabled == 1) {
    MmrcExecuteTask (MrcData, ConnectPmiToCpgc, NO_PRINT, Channel);
  }
#endif //BXTPMRC
  PerformFifoReset (MrcData, Channel, Rank);
  MrcDelay (MrcData, MICRO_DEL, 10);

  MrcData->ExecuteThisRoutineInParallel = ExecuteMode;

  return MMRC_SUCCESS;
}

/**
  Prints the header for training results

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       SignalNameLength
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
DisplayTrainingResultsHeader (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         SignalNameLength
  )
{
  UINT8  ElementIndex;
  UINT8  i;

  MmrcDebugPrint ((MMRC_DBG_MIN, "\nSignal "));
  //
  // Start at 7 which is the length of "Signal ".
  //
  for (i = 7; i <= SignalNameLength; i++) {
    MmrcDebugPrint ((MMRC_DBG_MIN, " "));
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "CH RK ST HZ  "));
  for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "%3d ",
      MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]
      ));
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, " M "));
  for (ElementIndex = 0; ElementIndex < NumberOfCCElements; ElementIndex++) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "CC%d ", ElementIndex));
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "Vref Dly\n"));
  //
  // If the name of the signal (i.e. TX_DQ) is less than 6 characters, need to
  // draw extra "-" symbols to completely underline the word "Signal".
  //
  if (i < 8) {
    i = 8;
  }
  for (; i > 2; i--) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "-"));
  }
  if (SignalNameLength < 7) {
    MmrcDebugPrint ((MMRC_DBG_MIN, " -- -- -- --  "));
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "  -- -- -- --  "));
  }
  for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "--- ",
      MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]
      ));
  }
  //
  // Minus1 underline.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, " - "));
  for (ElementIndex = 0; ElementIndex < NumberOfCCElements; ElementIndex++) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "--- ", ElementIndex));
  }
  //
  // Vref and delay underline.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "---- ----\n"));

  return MMRC_SUCCESS;
}

/**
  Prints the Training Results

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       TrainingDelay   Training we want to print delay.
  @param[in]       Restore         If set the function will restore training values form MRC's cache
                                   else it will display the training results.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
DisplayOrRestoreTrainingResults (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT16        TrainingDelay,
  IN        BOOLEAN       FaultyPartProcess,
  IN        UINT8         NumberElement
  )
{
#if USE_GET_SET_FROM_SPREADSHEET
  UINT8  Rank;
  UINT8  Strobe;
  UINT32 TempValue, LinearValue;
  UINT32 RecalculatedDelay;
  UINT8  ElementIndex;
  UINT16 OrgTrainingDelay;
  UINT8  AccessMethod;
  UINT8  ThisSignalExists;
  //
  // Initialize element values and indexes based on the elements structure array
  //
  OrgTrainingDelay = TrainingDelay;
  TrainingDelay = GetOffsetIndex (MrcData, TrainingDelay);

  TrainingDelay -= ALGO_REG_INDEX;
  TrainingDelay *= NumberOfElementsPerAlgo;

  if (MrcData->RestorePath) {
    AccessMethod = CMD_GET_CACHE;
  } else {
    AccessMethod = CMD_GET_REG_UC;
  }

  for (; Channel < MAX_CHANNELS; Channel++) {
    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel || !MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }

    //
    // Compute the starting index for the linear delay elements
    //
    DisplayTrainingResultsHeader (MrcData, Channel, (UINT8)StringLength (SignalInfo[GetOffsetIndex(MrcData, GetSignalInfoIndex (TrainingDelay))].Name));

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank]) {
        for (Strobe = 0; Strobe < NumberElement; Strobe++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
            continue;
          }
          //
          // Print out the Label for the row.
          //
          RecalculatedDelay = 0;
          ThisSignalExists = 0;

          MmrcDebugPrint ((
            MMRC_DBG_MIN,
            "%s  %02d %02d %02d %02d   ",
            SignalInfo[GetSignalInfoIndex (TrainingDelay)].Name,
            Channel,
            Rank,
            Strobe,
            SignalInfo[GetSignalInfoIndex (TrainingDelay)].FrequencyIndex
          ));

          for (ElementIndex = 0; ElementIndex < NumberOfDelayElementsPerAlgo; ElementIndex++) {
            //
            // Attempt to read the delay element, if successful accumulate to the value.
            //
            if (GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TrainingDelay + ElementIndex, AccessMethod, &TempValue) == MMRC_SUCCESS) {
              RecalculatedDelay += TempValue * (MrcData->GranularitySteps[MrcData->CurrentBlueprint][ADdll[MrcData->DigitalDll]][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency][ElementIndex]);
              //
              // Since some delays may not have all element indices (i.e. RX_DQS may not have 1x or 2x...only pi),
              // we need to store the existing delay index so we can use it later to lookup the Minus1 and Vref value.
              //
              ThisSignalExists = ElementIndex;
              MmrcDebugPrint ((MMRC_DBG_MIN, "%02d  ", TempValue));
              if (MrcData->RestorePath) {
                GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TrainingDelay + ElementIndex, CMD_SET_VAL_FC, &TempValue);
              }
            } else {
              MmrcDebugPrint ((MMRC_DBG_MIN, "--  "));
            }
          } // ElementIndex loop ...
          //
          // Get/print/save the Minus1.
          //
          if (GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, SignalInfo[GetSignalInfoIndex (TrainingDelay)].Minus1Index, AccessMethod, &TempValue) == MMRC_SUCCESS) {

            if (TempValue == 1) {
              RecalculatedDelay += GetHalfClk (MrcData, Channel);
            }
            MmrcDebugPrint ((MMRC_DBG_MIN, "%d ", TempValue));
            if (MrcData->RestorePath) {
              GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, SignalInfo[GetSignalInfoIndex (TrainingDelay)].Minus1Index, CMD_SET_VAL_FC, &TempValue);
            }
          } else {
            MmrcDebugPrint ((MMRC_DBG_MIN, "- "));
          }
          //
          // Get/print/save the CCs.
          //
          for (ElementIndex = 0; ElementIndex < NumberOfCCElements; ElementIndex++) {
            if (GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TrainingDelay + NumberOfDelayElementsPerAlgo + ElementIndex, AccessMethod, &TempValue) == MMRC_SUCCESS) {
              MmrcDebugPrint ((MMRC_DBG_MIN, "  %d ", TempValue));
              if (MrcData->RestorePath) {
                GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TrainingDelay + NumberOfDelayElementsPerAlgo + ElementIndex, CMD_SET_VAL_FC, &TempValue);
              }
            } else {
              MmrcDebugPrint ((MMRC_DBG_MIN, "  - "));
            }
          }
          //
          // Get/print/save the Vref.
          //
          if (GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, SignalInfo[GetSignalInfoIndex (TrainingDelay)].VrefIndex, AccessMethod, &TempValue) == MMRC_SUCCESS) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "%04d ", TempValue));
            if (MrcData->RestorePath) {
              GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, SignalInfo[GetSignalInfoIndex (TrainingDelay)].VrefIndex, CMD_SET_VAL_FC, &TempValue);
            }
          } else {
            MmrcDebugPrint ((MMRC_DBG_MIN, "  -- "));
          }
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, OrgTrainingDelay, AccessMethod, &LinearValue);
          //
          // Print and save the recalculated delay value.
          //
          MmrcDebugPrint ((MMRC_DBG_MIN, "%04d\n", LinearValue));

          BreakStrobeLoop (MrcData, Channel, Rank, OrgTrainingDelay, &Strobe);
        } // Strobe loop ...
      }
    } // Rank loop ...

#if FAULTY_PART_TRACKING
    if (FaultyPartProcess && !MrcData->RestorePath) {
      FaultyPartProcessTrainingResult (MrcData, Channel);
    }
#endif
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...
#endif
  return MMRC_SUCCESS;
}


UINTX
ByteEnableToUintX (
  IN       UINT8    Be,
  IN       UINTX    Value,
  IN       UINT8    *ValueLength
  )
{
  UINT8  CurrentByte;
  UINTX  FinalValue;
  UINT8  TestByte;
  UINT8  MaxBit;

#if USE_64_BIT_VARIABLES
  MaxBit = 64;
#else
  MaxBit = 32;
#endif
  *ValueLength = 0;
  CurrentByte = 0;
  FinalValue = 0;
  for (TestByte = 0; TestByte < MaxBit; TestByte += 8) {
    if ((Be & 1) == 1) {
      (*ValueLength)++;
#ifdef SUSSW
      FinalValue += ((Value & 0xff) << TestByte);
#else
      FinalValue += LShiftU64((Value & 0xff), TestByte);//((Value & 0xff) << TestByte);
#endif
      Value >>= 8;
    }
    Be >>= 1;
  }
  //
  // In case of no byte enables (i.e. wake command or something), set to 1 byte of data.
  //
  if (*ValueLength == 0)
  {
    *ValueLength = 1;
  }
  return FinalValue;
}

/**
  Decodes and executes the Capsule initialization.
  The routine starts at a beginning index in the CapsuleInit
  compressed data structure and continues until a ASSIGN_DONE
  flag is detected.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
MmrcExecuteTaskImplementation (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  BOOLEAN           ForceRun,
  UINT8             MaxChannel
  )
{
  UINTX    PfctValue;                                 // Based on the current PFCT, holds the Value for the current assignment.
  UINT8    PfctStatus;                                // Flag to determine if any of the PFCT conditions were met on the current assign.
  UINT8    Action;                                    // Holds the Action of the current assign.
  UINT8    Counter;                                   // Counter for displaying results on the output when enabled.
  UINT8    *CurrentEncLoc;                            // Holds the location of the current assign. being decoded.
  UINT32   Delay;                                     // Holds the Delay for the assignment.  0 if not needed.
  PFCT_VARIATIONS   PFCTVariations[MaxPfct];          // Holds all permutations of PFCTs
  REGISTER_LOCATION_ATTRIBUTES *RegisterAttributes;   // Bit fields for the flags portion of the MEMORYINIT.
  REGISTER_LOCATION_ATTRIBUTES2 *RegisterAttributes2; // Bit fields for the 2nd byte of the flags portion of the MEMORYINIT.
  UINT16   ConditionalExecution;                      // Flag specifying if the assignment should take place.
  UINT8    ChannelToWrite;                            // Specific channel the assignment is directed to, if any.
  UINT8    InstanceToWrite;                           // Specific instance the assignment is directed to, if any.
  UINT16   CurrentPtr;                                // Pointer to the encoded data stream.
  UINT16   RegisterOffset;                            // Register offset provided in the assignment.
  UINT8    BoxType;                                   // Boxtype provided in the assignment.
  UINT8    ByteEnable;                                // Byte enable provided in the assignment.
  UINTX    Mask;                                      // Mask that may be provided in the assignment.
  UINT8    MaskValueLength;                           // Length of the MASK in bytes, when the mask is provided.
  UINT8    Pfct;                                      // PFCT flag provided in the assignment.
  UINT8    PfctDynamic;                               // Dynamic field in the dPFCT provided in the assignment.
  UINT8    NumberConditionalValues;                   // Number of PFCT Condition-Values provided in the assignment.
  UINT8    NumberConditionalBytes;                    // Number of bytes within the condition portion of the PFCT assignment.
  UINT8    ConditionalFlag;                           // When decoding PFCT values, when the condition is met, this value is 1.
  UINT8    TestVal;                                   // Used to determine if the condition is true in the PFCT assignment.
  UINT8    NumberInstances;                           // Number of instances for a specific BoxType.
  UINT8    Instance;                                  // Instance being specified when looping on total Instances.
  UINT8    SkipIosf;                                  // Flag specifying if the IOSF assignment should be skipped.  (Broadcasts)
  UINTX    readValue;                                 // When doing a poll, this is the value read back.
  UINTX    Value;                                     // 32-bit value that gets written to for the assignment.
  UINT8    sLoop;                                     // When DETAILED_PHYINIT is enabled, this is used to print the strings.
  UINT8    StartingBit;                               // Final starting position that the assignment is accessing.
  UINT8    EndingBit;                                 // Final ending position that the assignment is accessing.
  UINT8    PfctValueSize;                             // The size in bytes of the value portion of the PFCT CVs.
  UINT32   BaseOffset;                                // Base offset for the instance of the boxtype.
  UINT8    Port;                                      // Port ID for the instance of the boxtype.
  UINT8    Bus;                                       // When doing PCI Access, PCI Bus.
  UINT8    Device;                                    // When doing PCI Access, PCI Device.
  UINT8    Func;                                      // When doing PCI Access, PCI Function.
  UINT8    i;
  REGISTER_ACCESS Register;                           // Field that specifies the location of the register including offset and bits.
  UINTX    NewValue;                                  // Used for temporary storage in DynamicAssignmentGenerated call.
  UINT16   DynamicIndex;                              // Used in DynamicAssignmentGenerated (the switch statement index).
  UINT16   InitialStringIndex;
  UINT8    Socket;                                    // The current CPU socket on which we're operating.
  UINT8    Strobe;                                    // Used to loop on strobes.
  UINT8    Rank;                                      // Used to loop on ranks.
  UINT8    ThisChannel;                               // Used for strobe/rank looping.
  UINT8    ThisInstance;                              // Used for strobe/rank looping.
  UINT8    MaxStrobe;                                 // Used to loop on strobes.
  UINT8    MaxRank;                                   // Used to loop on ranks.
  UINT8    ThisBoxType;                               // Used for strobe/rank looping.
  UINT8    MaxBit;                                    // Are we using 32 bit or 64 bit registers?
  UINT8    NumStrings;                                // Number of strings we need to output to the log.
  UINT8    Zero;                                      // Temporary location for resetting structures to 0.
  UINT8    k;                                         // Temporary counter.
  UINT8    MsgLevel;                                  // Temporary storage for backing up the message level.
#if CAPSULESTRINGS
  UINT8    MaxBoxName;                                // Maximum length of box string to print.
#endif
  BOOLEAN  ExecuteThisRoutineInParallel;
#if DUMP_REGISTER_NAMES
  UINT8    MaxRegisterName;                           // Maximum length of register string to print.
  UINT32   RegisterNumber;                            // Index to register number which will be printed.
#endif
  BOOLEAN  ChannelBasedOffsetDetected;
  UINT16   CapsuleStrIndex;

#if USE_64_BIT_VARIABLES
  MaxBit = 64;
#else
  MaxBit = 32;
#endif

  InitialStringIndex  = StringIndex;
  ThisInstance        = 0;
  ThisChannel         = 0;
  MsgLevel            = MrcData->MrcDebugMsgLevel;
  for (; Channel < MaxChannel; Channel++) {
    if ((!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) && (!ForceRun)) {
      continue;
    }
    //
    // Initializing the Variables.
    //
    Value                          = 1;
    ChannelToWrite                 = 0;
    PfctValue                      = 0;
    StartingBit                    = 0;
    EndingBit                      = 0;
    sLoop                          = 0;
    Counter                        = 0;
    RegisterOffset                 = 0;
    PfctValueSize                  = 0;
    Delay                          = 0;
    CurrentPtr                     = 0;
    ConditionalExecution           = 1;
    Bus                            = 0;
    Device                         = 0;
    Func                           = 0;
    BaseOffset                     = 0;
    Port                           = 0;
    ThisChannel                    = 0;
    ThisInstance                   = 0;
    StringIndex                    = InitialStringIndex;
    Socket                         = 0;
    //
    // If this task is marked as NO_PRINT, turn off all TRACE level messages. MMRC_DBG_TRACE
    // is used by this routine to display all actions taken.
    //
    if (StringIndex == NO_PRINT) {
      MrcData->MrcDebugMsgLevel &= ~MMRC_DBG_TRACE;
    }
#if CAPSULESTRINGS
    MmrcDebugPrint ((MMRC_DBG_TRACE, " %s\n", CapsuleStrings[StringIndex]));
    MmrcDebugPrint ((MMRC_DBG_TRACE, "    Box"));
    if (MAX_BOXNAME_LENGTH < 3) {
      MaxBoxName = 0;
    } else {
      MaxBoxName = MAX_BOXNAME_LENGTH - 3;
    }
    for (i = 0; i < MaxBoxName; i++) {
      MmrcDebugPrint ((MMRC_DBG_TRACE, " "));
    }
#endif
#if DUMP_REGISTER_NAMES
    MmrcDebugPrint ((MMRC_DBG_TRACE, "Register Symbol"));
    if (MAX_REGISTER_NAME_LENGTH < 13) {
      MaxRegisterName = 0;
    } else {
      MaxRegisterName = MAX_REGISTER_NAME_LENGTH - 13;
    }
    for (i = 0; i < MaxRegisterName; i++) {
      MmrcDebugPrint ((MMRC_DBG_TRACE, " "));
    }
#endif
    MmrcDebugPrint ((MMRC_DBG_TRACE, "   Port Offset     Mask               Action    Delay      Value\n"));
    //
    // Given the current P, F, C, T provide all possible PFCT variations, this makes the comparison quicker
    // since this does not have to be derived for every assignment.  There are a total of 16 different PFCT
    // permutations.
    //
    CreatePFCTSel (MrcData, Channel, PFCTVariations);
    //
    // Begin by assigning the Current encoded location to the start of the assignments.
    //
    CurrentEncLoc = (UINT8 *) &InitData[CapsuleStartIndex];
    //
    // The last assignment of the assignment list has the MACRO "ASSIGNDONE" which is the signature to stop decoding.
    //
    while (1) {
      ChannelBasedOffsetDetected  = TRUE;
      InstanceToWrite =  0;
      ChannelToWrite  =  Channel;
      ConditionalExecution = 1;
      Zero = 0;
      //
      // Reset to 0.
      //
      RegisterAttributes2 = (REGISTER_LOCATION_ATTRIBUTES2 *) &Zero;
      //
      // The register attributes for the FLAGS starts at the beginning location, and the default length of this field
      // is one byte.
      //
      //MmrcDebugPrint ((MMRC_DBG_TRACE, "0x%08x 0x%08x 0x%08x\n\r", CurrentEncLoc[0], CurrentEncLoc[1], CurrentEncLoc[2]));
      RegisterAttributes = (REGISTER_LOCATION_ATTRIBUTES *) &CurrentEncLoc[0];
      CurrentPtr = 1;
      if (RegisterAttributes->Done == 1) {
        break;
      }
      if (RegisterAttributes->NeedSecondByte == 1) {
        RegisterAttributes2 = (REGISTER_LOCATION_ATTRIBUTES2 *) &CurrentEncLoc[CurrentPtr];
        CurrentPtr++;
      }
      //
      // Check if the assignment should be executed based on the execution flag.
      //
      if (RegisterAttributes->CondExec == 1) {
        ConditionalExecution = (UINT16) MrcData->DynamicVars[Channel][*(UINT16 *) &CurrentEncLoc[CurrentPtr]];
        CurrentPtr += 2;
      }
      //
      // If the ChannelDependent or InstanceDependent flags are either set, then the flags will be one byte larger.
      // Also when not set, Channel/Instance to write are set to 0 but will not be used in the assignment.
      //
      if (RegisterAttributes2->ChannelDependent == 1) {
        ChannelToWrite  = CurrentEncLoc[CurrentPtr++];
      }
      if (RegisterAttributes2->InstanceDependent == 1) {
        InstanceToWrite = CurrentEncLoc[CurrentPtr++];
      }
      //
      // Following the flags is the register Offset.  This will be either a 8-bit or 16-bit value based on the
      // offset16 flag.
      //
      if (RegisterAttributes->Offset16 == 1) {
        RegisterOffset = *((UINT16 *) &CurrentEncLoc[CurrentPtr]);
        CurrentPtr +=2;
      } else {
        RegisterOffset = *((UINT8 *) &CurrentEncLoc[CurrentPtr]);
        CurrentPtr +=1;
      }
      //
      // Read either 2 bytes or 1 byte to get the boxtype and byte enables based on
      // the size of the boxtype number and the PFCT value to be written.
      //
      if (RegisterAttributes->UseBB2 == 1) {
        //
        // The Boxtype and ByteEnables are always provided in the next byte.  They are split into 4-bits each.
        //
        BoxType    = CurrentEncLoc[CurrentPtr];
        ByteEnable = CurrentEncLoc[CurrentPtr + 1];
        CurrentPtr += 2;
      } else {
        //
        // The Boxtype and ByteEnables are always provided in the next byte.  They are split into 4-bits each.
        //
        BoxType    = (CurrentEncLoc[CurrentPtr] >> 4) & 0x0f;
        ByteEnable = (CurrentEncLoc[CurrentPtr] >> 0) & 0x0f;
        CurrentPtr += 1;
      }
      //
      // If the MaskPresent bit in the flags is set, then the mask will be provided.  The length of the mask is based on the
      // byte enables.  Ex. If 2-bits in the BE (_0110) are set, then the mask will be 2-bytes long.
      //
      if (RegisterAttributes->MaskPresent == 1) {
        Mask = ByteEnableToUintX (ByteEnable, *((UINTX *) &CurrentEncLoc[CurrentPtr]), &MaskValueLength);
        CurrentPtr += MaskValueLength;
      } else {
#if USE_64_BIT_VARIABLES
        Mask = ByteEnableToUintX (ByteEnable, 0xffffffffffffffff, &MaskValueLength);
#else
        Mask = ByteEnableToUintX (ByteEnable, 0xffffffff, &MaskValueLength);
#endif
      }
      //
      // The Action/PFCT provides three distinct fields.  The action (get/set/poll), the PFCT, and the dynamic flag.
      // This is a 1-byte field and is split into [3-bits action, 1-bit dyn, 4-bits pfct].
      Action      = (CurrentEncLoc[CurrentPtr] >> 5) & 0x07;
      PfctDynamic = (CurrentEncLoc[CurrentPtr] >> 4) & 0x01;
      Pfct        = (CurrentEncLoc[CurrentPtr] >> 0) & 0x0f;
      CurrentPtr  += 1;
      //
      // If the action is SET_DELAY, then immediately following the ACTION will be a 4byte delay value.
      //
      if (Action == SET_DELAY) {
        Delay = *(UINT32 *) &CurrentEncLoc[CurrentPtr];
        CurrentPtr += 4;
      }
      //
      // The PFCT status will provide the flag if any of the PFCT conditions where true, the assumption is no
      // condition is true, and will be changed to TAKE_ACTION if one is determined.
      //
      PfctStatus = SKIP_ACTION;
      //
      // If the PFCT flag is set to 0, then the assignment is global/PFCT independent.  The assignment should then
      // provide one value.  If the dynamic bit is set, the value is an 8-bit index value, otherwise the value length
      // will be based on the byte-enables.
      //
      if (Pfct == PFCT_INDEPENDENT) {
        PfctStatus = TAKE_ACTION;
        if (PfctDynamic == 1) {
          PfctValue = *(UINT16 *) &CurrentEncLoc[CurrentPtr];
          CurrentPtr += 2;
        } else {
          PfctValue = ByteEnableToUintX (ByteEnable, *((UINTX *) &CurrentEncLoc[CurrentPtr]), &MaskValueLength);
          CurrentPtr += MaskValueLength;
        }
      } else {
        //
        // To reach this point, the PFCT Flag must have been non-zero so non-global.
        // The next byte is the total number of conditional-Value pairs..
        //
        NumberConditionalValues = CurrentEncLoc[CurrentPtr];
        CurrentPtr += 1;
        //
        // Loop through each Condition/Value pair looking for a match based on the current pfct.
        //
        for (Counter = 0; Counter < NumberConditionalValues; Counter++) {
          //
          // Determine the number of bytes are needed for the condition.  The system will
          // compare the PFCT condition byte-by-byte with the TRUE PFCT condition.  If all the
          // bits compare okay, then the condition is true.  ConditionalFlag is the byte-by-byte
          // comparison flag.
          //
          NumberConditionalBytes = PFCTVariations[Pfct].LengthVariation;
          ConditionalFlag = 1;
          while (NumberConditionalBytes > 0 && ConditionalFlag == 1) {
            TestVal = (UINT8) ((PFCTVariations[Pfct].Pfct >> ((NumberConditionalBytes - 1) * 8) ) & 0xff);
            if ((CurrentEncLoc[CurrentPtr+NumberConditionalBytes - 1]  & TestVal) != TestVal) {
               ConditionalFlag = 0;
            }
            NumberConditionalBytes--;
          }
          CurrentPtr += PFCTVariations[Pfct].LengthVariation;
          //
          // Read in the value which is also based on the byte-enables for its length, similar to the mask.
          //
          if (PfctDynamic == 1) {
            PfctValue = *(UINT16 *) &CurrentEncLoc[CurrentPtr];
            PfctValueSize = 2;
          } else {
            PfctValue = ByteEnableToUintX (ByteEnable, *((UINTX *) &CurrentEncLoc[CurrentPtr]), &MaskValueLength);
            PfctValueSize = MaskValueLength;
          }
          CurrentPtr += PfctValueSize;
          //
          // If the condition flag is still true, then move the currentPtr to the end of the remaining condition-values.
          // and set the status to take_action.
          if (ConditionalFlag == 1) {
            PfctStatus = TAKE_ACTION;
            CurrentPtr += (PFCTVariations[Pfct].LengthVariation + PfctValueSize) * (NumberConditionalValues - Counter - 1);
            break;
          }
        }  // for (Counter = 0; Counter < NumberConditionalValues; Counter++)
      } // if (Pfct == PFCT_INDEPENDENT) {
      //
      // Save the value which will be used as the index to the switch statement
      // in DynamicAssignmentGenerated.
      //
      DynamicIndex = (UINT16) PfctValue;
      if (RegisterAttributes2->StringPresent == 1) {
        NumStrings = CurrentEncLoc[CurrentPtr++];
      } else {
        NumStrings = 0;
      }
      //
      // Now that a value has been determined true, the value must be programmed based on the
      // loopings to multiple registers.
      //
      if (PfctStatus == TAKE_ACTION  && ConditionalExecution == 1) {
        if (Action == SUBTASK) {
#if CAPSULESTRINGS
          MmrcDebugPrint ((MMRC_DBG_TRACE, "   +"));
          for (k = 0; k < MAX_BOXNAME_LENGTH; k++) {
            MmrcDebugPrint ((MMRC_DBG_TRACE, " "));
          }
#endif
#if DUMP_REGISTER_NAMES
          for (k = 0; k < MAX_REGISTER_NAME_LENGTH + 2; k++) {
            MmrcDebugPrint ((MMRC_DBG_TRACE, " "));
          }
#endif
#if USE_64_BIT_VARIABLES
          MmrcDebugPrint ((MMRC_DBG_TRACE, "   NA   NA         NA                 "));
#else
          MmrcDebugPrint ((MMRC_DBG_TRACE, "   NA   NA         NA         "));
#endif
          MmrcDebugPrint ((MMRC_DBG_TRACE, "SUBTASK   NA         "));
          //
          // Search for index in capsuleStrings
          //
          CapsuleStrIndex = (UINT16) -1;
          for (k = 0; k < MAX_SUBTASKS; k++) {
            if (PfctValue == SubtaskOffsetIndex[k].FncOffset) {
              CapsuleStrIndex = SubtaskOffsetIndex[k].StrIndex;
              break;
            }
          }
          if (CapsuleStrIndex != -1) {
#if CAPSULESTRINGS
            MmrcDebugPrint ((MMRC_DBG_TRACE, "%s", CapsuleStrings[CapsuleStrIndex]));
#endif
          } else {
            MmrcDebugPrint ((MMRC_DBG_TRACE, "ERROR"));
          }

#if CAPSULESTRINGS
          if (RegisterAttributes2->StringPresent == 1) {
            for (k = 0; k < NumStrings; k++) {
              MmrcDebugPrint ((MMRC_DBG_TRACE, " %s", OutputStrings[CurrentEncLoc[CurrentPtr + k]]));
              if ((k + 1) < NumStrings) {
                MmrcDebugPrint ((MMRC_DBG_TRACE, ","));
              }
            }
          }
#endif // CAPSULESTRINGS
          MmrcDebugPrint ((MMRC_DBG_TRACE, "\n"));
          ExecuteThisRoutineInParallel = MrcData->ExecuteThisRoutineInParallel;
          MrcData->ExecuteThisRoutineInParallel = FALSE;
          if (CapsuleStrIndex != -1) {
            MmrcExecuteTask (MrcData, (UINT16) PfctValue, CapsuleStrIndex, Channel);
          } else {
            MmrcExecuteTask (MrcData, (UINT16) PfctValue, 0, Channel);
          }
          MrcData->ExecuteThisRoutineInParallel = ExecuteThisRoutineInParallel;
          //
          // Else, not a SUBTASK...so proceed to process normal action.
          //
        } else {
          if (RegisterAttributes->StrobeLoop == 1) {
            MaxStrobe = MAX_STROBES;
          } else {
            MaxStrobe = 1;
          }
          if (RegisterAttributes->RankLoop == 1) {
            MaxRank = MAX_RANKS;
          } else {
            MaxRank = 1;
          }
          //
          // If the register should be written only for a specific channel, and this call is
          // not the correct channel, just skip this assignment and go to the next assignment.
          //
          if (RegisterAttributes2->ChannelDependent == 0 || (RegisterAttributes2->ChannelDependent == 1 && Channel == ChannelToWrite)) {
            //
            // If the InstanceDependent flag is set, then the total number of instances that will be written is 1,
            // otherwise, the total number of instances/channel for the given boxtype will be written. The other case
            // is if we are looping on rank and/or strobe, then we will ignore the instance looping since strobe/rank
            // looping is a higher priority.
            //
            if (RegisterAttributes2->InstanceDependent == 0 && Action != DELAY &&
                RegisterAttributes->StrobeLoop == 0 && RegisterAttributes->RankLoop == 0) {
                NumberInstances = InstancePortMap[BoxType].Instances;
                if (NumberInstances == 0) {
                  //
                  // If this boxtype is not channel based, then use the total number of instances
                  // instead of the instances per channel.
                  //
                  NumberInstances = InstancePortMap[BoxType].TotalInstances;
                  ChannelBasedOffsetDetected = FALSE;
                }
            } else {
              NumberInstances = 1;
            }
            //
            // The number of instances cannot be 0xff, if an 0xff is read this indicates
            // that the current Box Type is not valid for the current blueprint, so we want
            // to skip any subsequent action
            //
            if (NumberInstances == 0xff) {
              NumberInstances = 0;
            }
            //
            // Loop though all the instances determined earlier.
            //
            for (Instance = 0; Instance < NumberInstances; Instance++) {
              for (Rank = 0; Rank < MaxRank; Rank++) {
                for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
                  //
                  // If the register should be written only for a specific channel, and this call is
                  // not the correct channel, just skip this assignment and go to the next assignment.
                  //
                  if (RegisterAttributes2->InstanceDependent == 1) {
                    Instance = InstanceToWrite;
                  }
                  //
                  // If the broadcast feature within the IOSF is enabled, then only one write, the first, will occur as all the
                  // instances will be written to.
                  //
                  if (BROADCAST_SUP && Instance > 0) {
                    SkipIosf = 1;
                  } else {
                    SkipIosf = 0;
                  }
                  //
                  // Determine the actual offset/mask for the register to be accessed.  The offset currently is based off the
                  // offset for the assignment + the rank offset + bytelane offset. The mask and shift bit will also be used
                  // for PCI accesses although the data for the PCI access is in PciReg which is filled in below.
                  //
                  Register.Offset   = RegisterOffset;
                  Register.Mask     = Mask;
                  Register.ShiftBit = 0;
                  ThisBoxType = BoxType;
                  //
                  // If we're looping on strobe/rank we need to adjust the offset to the correct register location
                  // for this strobe/rank.
                  //
                  if (RegisterAttributes->StrobeLoop == 0 && RegisterAttributes->RankLoop == 0) {
                    ThisChannel = Channel;
                    ThisInstance = Instance;
                  } else {
                    if (RegisterAttributes->StrobeLoop == 1) {
                      Register.Offset += StrobeToRegister[FloorPlan[Channel][Strobe].Strobelane][1];
                      ThisChannel = FloorPlan[Channel][Strobe].Channel;
                      ThisInstance = (UINT8)(Instance + StrobeToRegister[FloorPlan[Channel][Strobe].Strobelane][0]);
                    }
                    if (RegisterAttributes->RankLoop == 1) {
                      Register.Offset += RankToRegister[Rank][1];
                      ThisInstance += ((UINT8)(Instance + RankToRegister[Rank][0]));
                    }
                    //
                    // Select the new boxtype since we may have increased the sub-box instance to
                    // a subsequent boxtype to reach the correct register for this rank/strobe. An
                    // example would be if strobe 16's registers were in the next PCI bus/dev/func
                    // and we needed to point to the next boxtype to get to that location.
                    //
                    for (i = 0; i < MAX_BOXES; i++) {
                      if (InstancePortMap[i].InstanceStartIndex <= (InstancePortMap[BoxType].InstanceStartIndex + ThisInstance)) {
                        ThisBoxType = i;
                      } else {
                        break;
                      }
                    }
                  }
                  //
                  // When a Boxtype that is not channel based we need to hardcode 'ThisChannel' = 0
                  //
                  if (!ChannelBasedOffsetDetected) {
                    ThisChannel = 0;
                  }
                  //
                  // If the action was a GET/SET/SET_DELAY/POLL, then the register offset, mask, delay, and value will be printed.
                  //
                  if (Action != DELAY) {
                    if (GetRegisterAccessInfo (BoxType, ThisChannel, ThisInstance,
                        (UINT32 *)&readValue, &Port, &BaseOffset, &Bus, &Device, &Func, ModeRead) != NoError) {
                      continue;
                    }
                  }
                  //
                  // The printout of the assignment will begin with either a *,/, or +.
                  //  *-> the assignment was performed, but it completed the assignment of others as it had broadcast enabled.
                  //  /-> the assignment was performed.
                  //  +-> the assignment was skipeed as it was assigned with the broadcast.
                  //
                  if (Instance == 0 && RegisterAttributes2->InstanceDependent == 0 && BROADCAST_SUP) {
                    MmrcDebugPrint ((MMRC_DBG_TRACE, " *"));
                  } else {
                    MmrcDebugPrint ((MMRC_DBG_TRACE, " +"));
                  }
                  //
                  // Print out register information.
                  //
                  if (Action == GET || Action == SET || Action == SET_DELAY || Action == POLL) {
                    //
                    // Convert the PFCTValue to the dynamic value, if the flag is set.
                    //
                    if (PfctDynamic == 1 && Action != GET) {
                      NewValue = MemRegRead (BoxType, ThisChannel, ThisInstance, Register);
                      DynamicAssignmentGenerated (MrcData->DynamicVars, ThisChannel, DynamicIndex, &NewValue);
                      PfctValue = NewValue;
                    }
  #if CAPSULESTRINGS
                    MmrcDebugPrint ((MMRC_DBG_TRACE, "%s ", UnitStrings[ThisBoxType]));
  #endif
  #if DUMP_REGISTER_NAMES
                    RegisterNumber = GetRegisterNumber (ThisBoxType, Register.Offset);
                    //
                    // If the register has a name, print it out. Else, just pad with spaces.
                    //
                    if (RegisterNumber == 0xFFFFFFFF) {
                      i = 0;
                    } else {
                      MmrcDebugPrint ((MMRC_DBG_TRACE, "%s",
                        Registers[GetRegisterNumber (ThisBoxType, Register.Offset)].RegInfo->RegisterName
                      ));
                      i = (UINT8) StringLength (Registers[RegisterNumber].RegInfo->RegisterName);
                    }
                    for (i; i <= MAX_REGISTER_NAME_LENGTH + 2; i++) {
                      MmrcDebugPrint ((MMRC_DBG_TRACE, " "));
                    }
  #endif
                    if (InstancePortMap[ThisBoxType].AccessMethod == eSB) {
                      MmrcDebugPrint((MMRC_DBG_TRACE, "   0x%02X", Port));
                    } else if (InstancePortMap[ThisBoxType].AccessMethod == eBAR) {
                      MmrcDebugPrint((MMRC_DBG_TRACE, "    MEM"));
                    } else if (InstancePortMap[ThisBoxType].AccessMethod == ePCI) {
                      MmrcDebugPrint((MMRC_DBG_TRACE, "%02X/%02X/%01X", Bus, Device, Func));
                    }
#if USE_64_BIT_VARIABLES
                    MmrcDebugPrint((MMRC_DBG_TRACE, " 0x%08X 0x%08X%08X ",
                      Register.Offset + BaseOffset, (UINT32)(Register.Mask >> 32), (UINT32)Register.Mask
                    ));
#else
                    MmrcDebugPrint((MMRC_DBG_TRACE, " 0x%08X 0x%08X%08X ",
                      Register.Offset + BaseOffset, 0, (UINT32)Register.Mask
                    ));
#endif
                  }
                  if (Action == GET) {
                    //
                    // Get the right shift bit to capture the variable being read.
                    //
                    for (i = 0; i < MaxBit; i++) {
                      if ((Register.Mask & (UINTX)(BIT0 << i)) != 0)
                      {
                        break;
                      }
                      Register.ShiftBit++;
                    }
                    MrcData->DynamicVars[ChannelToWrite][PfctValue] = (UINT32) MemFieldRead (BoxType, ThisChannel, ThisInstance, Register);
  #if USE_64_BIT_VARIABLES
//                    MmrcDebugPrint ((MMRC_DBG_TRACE, "GET       0x%08X 0x%08X%08X", 0,
                    MmrcDebugPrint ((MMRC_DBG_TRACE, "GET       0x%08X 0x%08X", 0,
//                      (UINT32)(MrcData->DynamicVars[ChannelToWrite][PfctValue] >> 32),
                      (UINT32)(MrcData->DynamicVars[ChannelToWrite][PfctValue])
                    ));
  #else
                    MmrcDebugPrint ((MMRC_DBG_TRACE, "GET       0x%08X 0x%08X%08X", 0,
                      0,
                      (UINT32)(MrcData->DynamicVars[ChannelToWrite][PfctValue])
                    ));
  #endif
                  } else if (Action == SET || Action == SET_DELAY)  {
                    if (Action == SET) {
                      MmrcDebugPrint ((MMRC_DBG_TRACE, "SET       0x%08X ", 0));
                    }
                    if (Action == SET_DELAY) {
                      MmrcDebugPrint ((MMRC_DBG_TRACE, "SET_DELAY 0x%08X ", Delay));
                    }
  #if USE_64_BIT_VARIABLES
                    MmrcDebugPrint((MMRC_DBG_TRACE, "0x%08X%08X", (UINT32)(PfctValue >> 32), (UINT32)(PfctValue)));
  #else
                    MmrcDebugPrint((MMRC_DBG_TRACE, "0x%08X%08X", 0, (UINT32)(PfctValue)));
  #endif

                    MemRegWrite (BoxType, ThisChannel, ThisInstance, Register, PfctValue, ByteEnable);
                    //
                    // If the action has a delay, with the set, then delay between every assignment.
                    //
                    if (Action == SET_DELAY) {
  #if SIM
                        MySimStall (Delay);
  #else
                        MrcDelay (MrcData, NANO_DEL, Delay);
  #endif
                    }
                  } else if (Action == POLL) {
                    //
                    // If the action is a POLL, then print out the register/value to be polled for, and begin polling.
                    //
  #if USE_64_BIT_VARIABLES
                    MmrcDebugPrint((MMRC_DBG_TRACE, "POLL      0x00000000 0x%08X%08X",
                      (UINT32)(PfctValue >> 32), (UINT32)(PfctValue)
                    ));
  #else
                    MmrcDebugPrint((MMRC_DBG_TRACE, "POLL      0x00000000 0x%08X%08X", 0, (UINT32)(PfctValue)));
  #endif
                    do {
                      readValue = MemRegRead (BoxType, ThisChannel, ThisInstance, Register);
//#endif
  #if SIM
                        //
                        // Stall between polling passes in simulation so we don't slow down the
                        // simulator with a bunch of read requests.
                        //
                        MySimStall (50);
  #endif
                    } while (readValue != PfctValue);
                  } else if (Action == DELAY) {
                    MmrcDebugPrint ((MMRC_DBG_TRACE, "DELAY: %dns", PfctValue));
  #if SIM
                    MySimStall ((UINT32)PfctValue);
  #else
                    MrcDelay (MrcData, NANO_DEL, (UINT32) PfctValue);
  #endif
                  }
                  //
                  // Add the length of the current assignment to the current encoded location to the the location
                  //  of the next assignment.
                  //
                  if (RegisterAttributes2->StringPresent == 1) {
                    for (k = 0; k < NumStrings; k++) {
  #if CAPSULESTRINGS
                      MmrcDebugPrint ((MMRC_DBG_TRACE, " %s", OutputStrings[CurrentEncLoc[CurrentPtr + k]]));
                      if ((k + 1) < NumStrings) {
                        MmrcDebugPrint ((MMRC_DBG_TRACE, ","));
                      }
  #endif // CAPSULESTRINGS
                    }
                  }
                  MmrcDebugPrint ((MMRC_DBG_TRACE, "\n"));
                } // Strobe loop
              } // Rank loop
            } // Instance loop ...
          }
        }
      } // TAKE_ACTION
      CurrentEncLoc += (CurrentPtr + NumStrings);
    } // *CurrentEncLoc != ASSIGNDONE loop ...

    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  MrcData->MrcDebugMsgLevel = MsgLevel;
  return MMRC_SUCCESS;
}

/**
  Decodes and executes the Capsule initialization.
  The routine starts at a beginning index in the CapsuleInit
  compressed data structure and continues until a ASSIGN_DONE
  flag is detected.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
MmrcExecuteTask (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  MMRC_STATUS  Status;

  if (MrcData->ForceRunOnDisabledChannels) {
    Status = MmrcExecuteTaskImplementation(MrcData, CapsuleStartIndex, StringIndex, Channel, TRUE,  MrcData->MaxNumberChannels);
  } else {
    Status = MmrcExecuteTaskImplementation(MrcData, CapsuleStartIndex, StringIndex, Channel, FALSE, MAX_CHANNELS);
  }
  return Status;
}

MMRC_STATUS
CreatePFCTSel (
  IN  OUT   MMRC_DATA       *MrcData,
  IN        UINT8            Channel,
  IN        PFCT_VARIATIONS *PFCTSelect
  )
{
  UINT16  CurrentPlatform;
  UINT16  CurrentFrequency;
  UINT16  CurrentConfiguration;
  UINT16  CurrentType;

  CurrentPlatform       = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform;
  CurrentFrequency      = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency;
  CurrentConfiguration  = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration;
  CurrentType           = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType;

  PFCTSelect[Pfct].Pfct     = 0;
  PFCTSelect[PfctT].Pfct    = (1  << CurrentType);
  PFCTSelect[PfctC].Pfct    = (1  << CurrentConfiguration);
  PFCTSelect[PfctCT].Pfct   = ((1 << CurrentConfiguration) << (NUM_TYPE)) | (1 << CurrentType);
  PFCTSelect[PfctF].Pfct    = (1  << CurrentFrequency);
  PFCTSelect[PfctFT].Pfct   = ((1 << CurrentFrequency) << (NUM_TYPE)) | (1 << CurrentType);
  PFCTSelect[PfctFC].Pfct   = ((1 << CurrentFrequency) << (NUM_CONF)) | (1 << CurrentConfiguration);
  PFCTSelect[PfctFCT].Pfct  = ((1 << CurrentFrequency) << (NUM_CONF + NUM_TYPE)) | ((1 << CurrentConfiguration) << (NUM_TYPE)) | ((1 << CurrentType));
  PFCTSelect[PfctP].Pfct    = (1  << CurrentPlatform);
  PFCTSelect[PfctPT].Pfct   = ((1 << CurrentPlatform) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctPC].Pfct   = ((1 << CurrentPlatform) << (NUM_CONF) ) | (1 << CurrentConfiguration);
  PFCTSelect[PfctPCT].Pfct  = ((1 << CurrentPlatform) << (NUM_CONF + NUM_TYPE) ) | ( (1 << CurrentConfiguration) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctPF].Pfct   = ((1 << CurrentPlatform) << (NUM_FREQ) ) | (1 << CurrentFrequency);
  PFCTSelect[PfctPFT].Pfct  = ((1 << CurrentPlatform) << (NUM_FREQ + NUM_TYPE) ) | ( (1 << CurrentFrequency) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctPFC].Pfct  = ((1 << CurrentPlatform) << (NUM_FREQ + NUM_CONF) ) | ( (1 << CurrentFrequency) << (NUM_CONF) ) | (1 << CurrentConfiguration);
  PFCTSelect[PfctPFCT].Pfct = ((1 << CurrentPlatform) << (NUM_FREQ + NUM_CONF + NUM_TYPE) ) | ((1 << CurrentFrequency) << (NUM_CONF + NUM_TYPE)) | ((1 << CurrentConfiguration) << (NUM_TYPE)) | (1 << CurrentType);

  PFCTSelect[Pfct].LengthVariation     = 0;
  PFCTSelect[PfctT].LengthVariation    = (NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctC].LengthVariation    = (NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctCT].LengthVariation   = (NUM_CONF + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctF].LengthVariation    = (NUM_FREQ - 1) / 8 + 1;
  PFCTSelect[PfctFT].LengthVariation   = (NUM_FREQ + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctFC].LengthVariation   = (NUM_FREQ + NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctFCT].LengthVariation  = (NUM_FREQ + NUM_CONF + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctP].LengthVariation    = (NUM_PLAT - 1) / 8 + 1;
  PFCTSelect[PfctPT].LengthVariation   = (NUM_PLAT + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctPC].LengthVariation   = (NUM_PLAT + NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctPCT].LengthVariation  = (NUM_PLAT + NUM_CONF + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctPF].LengthVariation   = (NUM_PLAT + NUM_FREQ - 1) / 8 + 1;
  PFCTSelect[PfctPFT].LengthVariation  = (NUM_PLAT + NUM_FREQ + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctPFC].LengthVariation  = (NUM_PLAT + NUM_FREQ + NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctPFCT].LengthVariation = (NUM_PLAT + NUM_FREQ + NUM_CONF + NUM_TYPE - 1) / 8 + 1;

  return MMRC_SUCCESS;
}
#if TRAINING_ALGOS
/**
  Translates the value according to the absolute/relative flag

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Index           Selection of signal group to be averaged.
  @param[in]       Start           Start.
  @param[in]       End             End.
  @param[in]       Flag            ABSOLUTE/RELATIVE Flag.
  @param[in]       Min             Returned Start.
  @param[in]       Max             Returned End.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GetMinMaxRelativeAbsolute (
  IN   MMRC_DATA    *MrcData,
  IN   UINT8        Channel,
  IN   UINT8        Rank,
  IN   UINT16       Index,
  IN   INT16        Start,
  IN   INT16        End,
  IN   UINT8        Flag,
  IN   UINT16       Min[MAX_CHANNELS][MAX_STROBES],
  IN   UINT16       Max[MAX_CHANNELS][MAX_STROBES]
  )
{
  UINT8   Strobe;
  UINT32  TempValue;

#if COMMAND_TRAINING == 1
  if (Index == CmdAll) {
    Index = CmdGrp0;
  }
#endif
  for (; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //
    // Get the Min/Max values for each strobe according ABSOLUTE/RELATIVE Flag
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if (Flag & RELATIVE) {
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Index, CMD_GET_CACHE, &TempValue);
        if (((INT16)TempValue + Start) < 0) {
          Min[Channel][Strobe] = 0;
        } else {
          Min[Channel][Strobe] = (UINT16) TempValue + Start;
        }
        if (((INT16)TempValue + End) < 0) {
          Max[Channel][Strobe] = 0;
        } else {
          Max[Channel][Strobe] = (UINT16) TempValue + End;
        }
      } else {
        Min[Channel][Strobe] = Start;
        Max[Channel][Strobe] = End;
      }
    } // Strobe loop ...
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...
  return MMRC_SUCCESS;
}
#endif

#if TRAINING_ALGOS == 1
MMRC_STATUS
Center2D (
  IN          MMRC_DATA       *MrcData,
  IN   OUT    SAMPLE_SWEEP    *Results,
  IN          UINT8           Channel,
  IN          UINT8           Rank,
  IN          UINT16          Dim1Index,
  IN          UINT8           Dim1Knobs,
  IN          UINT16          Dim1Min[MAX_CHANNELS][MAX_STROBES],
  IN          UINT16          Dim2Index,
  IN          UINT16          Dim2Min[MAX_CHANNELS][MAX_STROBES],
  IN          UINT16          Dim2Step,
  IN          UINT8           Dim2ConsolidateStrobes
  )
{
  UINT8           Strobe;
  UINT8           StrobeLoopCount;
  UINT16          AxisYCounter;
  UINT16          AxisXCounter;
  UINT8           Knob;
  UINT16          MaxMarginInX;
  UINT32          CenterX[MAX_KNOBS];
  UINT32          CenterY;
  UINT32          CenterYAligned;
  SAMPLE_SWEEP    ResultsComposite[MAX_ELEMENTS_DIM_Y];

  UINT16  EyeWidth[MAX_ELEMENTS_DIM_Y];
  UINT16  EyeHeight;
  UINT16  MaxEyeHeight[MAX_KNOBS][MAX_STROBES];
  UINT16  EyeWidthAtCenter[MAX_KNOBS];

  UINT32  AccumulateMarginFactor;
  UINT32  AccumulateMargin;

  UINT32  SumWeights;
  UINT32  SumLR;

  StrobeLoopCount = 0;
  CenterY = 0;
  CenterYAligned = 0;

  for (; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //
    // Build a composite of the Results for the calculation of the Y Center, this composite is the combinations of all knobs.
    //
    MmrcMemset (ResultsComposite, 0xff, MAX_ELEMENTS_DIM_Y * sizeof (SAMPLE_SWEEP));
    for (AxisYCounter = 0; AxisYCounter < MAX_ELEMENTS_DIM_Y; AxisYCounter++) {
      for  (Knob = 0; Knob < Dim1Knobs; Knob++) {
        for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
            continue;
          }
          if (ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][LOW] == 0xffff
               || ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][LOW] < Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][LOW]
             )
          {
              ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][LOW] = Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][LOW];
          }
          if (ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][HIGH] == 0xffff
               || ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][HIGH] > Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][HIGH]
             )
          {
              ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][HIGH] = Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][HIGH];
          }
        }
      }
    }
    //
    // Build a composite of the Results for the calculation of the Y Center, this composite is the combinations of all strobes.
    //
    if (Dim2ConsolidateStrobes == 1) {
      MmrcMemset (ResultsComposite, 0xff, MAX_ELEMENTS_DIM_Y * sizeof (SAMPLE_SWEEP));
      for (AxisYCounter = 0; AxisYCounter < MAX_ELEMENTS_DIM_Y; AxisYCounter++) {
        StrobeLoopCount = 1;
        for (Knob = 0; Knob < Dim1Knobs; Knob++) {
          for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
            if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
              continue;
            }
           if (ResultsComposite[AxisYCounter].Sample[Channel][Knob][0][0][LOW] == 0xffff
                || ResultsComposite[AxisYCounter].Sample[Channel][Knob][0][0][LOW] < Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][LOW]
              )
            {
                ResultsComposite[AxisYCounter].Sample[Channel][Knob][0][0][LOW] = Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][LOW];
            }
            if (ResultsComposite[AxisYCounter].Sample[Channel][Knob][0][0][HIGH] == 0xffff
                 || ResultsComposite[AxisYCounter].Sample[Channel][Knob][0][0][HIGH] > Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][HIGH]
               )
            {
                ResultsComposite[AxisYCounter].Sample[Channel][Knob][0][0][HIGH] = Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][HIGH];
            }
          }
        }
      }
    } else {
      StrobeLoopCount = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank];
    }

    //
    // Fill results across Y with the Margins in X, this table will be used to calculate Center in Y
    //
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if (Strobe < StrobeLoopCount) {
        MmrcMemset (EyeWidth, 0xff, MAX_ELEMENTS_DIM_Y * sizeof (UINT16));

        MaxMarginInX = 0;
        for (AxisYCounter = 0; AxisYCounter < MAX_ELEMENTS_DIM_Y; AxisYCounter++) {
          if (ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][HIGH] < ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][LOW]) {
            EyeWidth[AxisYCounter] = 0;
            continue;
          }
          EyeWidth[AxisYCounter] = ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][HIGH] -
                                    ResultsComposite[AxisYCounter].Sample[Channel][0][Strobe][0][LOW];
          //
          // Get the MaxMargin in X
          //
          if (MaxMarginInX < EyeWidth[AxisYCounter]) {
            MaxMarginInX = EyeWidth[AxisYCounter];
          }
        } // AxisYCounter loop ...
        //
        // Calculate the accumulate MarginInX * (Axis Y) and accumulate MarginInX
        // only if MarginInX > PASS_MARGIN(MaxMarginInX)
        // The basic formula being utilized is "Cum(EW*VREF)/Cum(EW)".
        //
        AccumulateMarginFactor  = 0;
        AccumulateMargin        = 0;
        for (AxisYCounter = 0; AxisYCounter < MAX_ELEMENTS_DIM_Y; AxisYCounter++) {
          if (EyeWidth[AxisYCounter] >= PASS_MARGIN_AXIS_X (MaxMarginInX)) {
            AccumulateMarginFactor  += EyeWidth[AxisYCounter] * (AxisYCounter + Dim2Min[Channel][Strobe]);
            AccumulateMargin        += EyeWidth[AxisYCounter];
            MmrcDebugPrint ((MMRC_DBG_TST, "  Y[%d]AMF: %d AM: %d\n",
              AxisYCounter, AccumulateMarginFactor, AccumulateMargin
              ));
          }
        } // AxisYCounter loop ...
        if (AccumulateMargin == 0) {
          MmrcDebugPrint ((MMRC_DBG_MIN, "Axis Width 0\n"));
          return STATUS_FAIL;//MrcErrCenter2DAccMarginXZero;
        }
        CenterY = (AccumulateMarginFactor + (AccumulateMargin / 2)) / AccumulateMargin;
        MmrcDebugPrint ((MMRC_DBG_TST, "  Y: AMF: %d AM: %d CenterY: %d\n",
          AccumulateMarginFactor, AccumulateMargin, CenterY
          ));
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Dim2Index, CMD_SET_VAL_FC_UC, &CenterY);

      //
      // Calculate the maximum eye height from this data.
      //
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        MaxEyeHeight[Knob][Strobe] = 0;
        for (AxisXCounter = 0; AxisXCounter < MAX_ELEMENTS_DIM_X; AxisXCounter++) {
          EyeHeight = 0;
          for (AxisYCounter = 0; AxisYCounter < MAX_ELEMENTS_DIM_Y; AxisYCounter++) {
            if (AxisXCounter >= Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][LOW] &&
                AxisXCounter <= Results[AxisYCounter].Sample[Channel][Knob][Strobe][0][HIGH]) {
               EyeHeight += Dim2Step;
            }
          }
          if (EyeHeight > MaxEyeHeight[Knob][Strobe]) {
            MaxEyeHeight[Knob][Strobe] = EyeHeight;
          }
        }
      }
      //
      // Get the nearest VREF that we can use for the center.
      //

      CenterYAligned = (UINT16)((CenterY * 10 + 5) / 10) ;
      CenterYAligned -= Dim2Min[Channel][Strobe];
      CenterYAligned /= Dim2Step;
      CenterYAligned *= Dim2Step;
    }
      //
      // Calculate the Center X for each Knob.
      //
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        EyeWidthAtCenter[Knob] = Results[CenterYAligned].Sample[Channel][Knob][Strobe][0][HIGH] - Results[CenterYAligned].Sample[Channel][Knob][Strobe][0][LOW];
        SumLR  = (
                   ( 3 * MaxEyeHeight[Knob][Strobe] + 0 * EyeWidthAtCenter[Knob] ) *
                      ( Results[CenterYAligned + 0 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] + Results[CenterYAligned - 0 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] +
                        Results[CenterYAligned + 0 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]  + Results[CenterYAligned - 0 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]
                      )
                  );
        SumWeights = 2*( 3 * MaxEyeHeight[Knob][Strobe] + 0 * EyeWidthAtCenter[Knob] );

        if ((CenterYAligned + 1 * Dim2Step < MAX_ELEMENTS_DIM_Y && Results[CenterYAligned + 1 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] != 0xffff) &&
            ((INT32)CenterYAligned - 1 * Dim2Step >= 0 && Results[CenterYAligned - 1 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] != 0xffff)) {
            SumLR += (
                       ( 2 * MaxEyeHeight[Knob][Strobe] + 1 * EyeWidthAtCenter[Knob] ) *
                          ( Results[CenterYAligned + 1 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] + Results[CenterYAligned - 1 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] +
                            Results[CenterYAligned + 1 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]  + Results[CenterYAligned - 1 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]
                          )
                      );
            SumWeights +=  2*( 2 * MaxEyeHeight[Knob][Strobe] + 1 * EyeWidthAtCenter[Knob] ) ;
        }

        if ((CenterYAligned + 2 * Dim2Step < MAX_ELEMENTS_DIM_Y && Results[CenterYAligned + 2 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] != 0xffff) &&
            ((INT32) CenterYAligned - 2 * Dim2Step >= 0 && Results[CenterYAligned - 2 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] != 0xffff)) {
          SumLR += (
                     ( 1 * MaxEyeHeight[Knob][Strobe] + 2 * EyeWidthAtCenter[Knob] ) *
                        ( Results[CenterYAligned + 2 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] + Results[CenterYAligned - 2 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] +
                          Results[CenterYAligned + 2 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]  + Results[CenterYAligned - 2 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]
                        )
                    );
          SumWeights += 2*( 1 * MaxEyeHeight[Knob][Strobe] + 2 * EyeWidthAtCenter[Knob] );
        }
        if ((CenterYAligned + 3 * Dim2Step < MAX_ELEMENTS_DIM_Y && Results[CenterYAligned + 3 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] != 0xffff) &&
            ((INT32)CenterYAligned - 3 * Dim2Step >= 0 && Results[CenterYAligned - 3 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] != 0xffff)) {
            SumLR += (
                       ( 0 * MaxEyeHeight[Knob][Strobe] + 3 * EyeWidthAtCenter[Knob] ) *
                          ( Results[CenterYAligned + 3 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] + Results[CenterYAligned - 3 * Dim2Step].Sample[Channel][Knob][Strobe][0][HIGH] +
                            Results[CenterYAligned + 3 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]  + Results[CenterYAligned - 3 * Dim2Step].Sample[Channel][Knob][Strobe][0][LOW]
                          )
                      );
            SumWeights  += 2*( 0 * MaxEyeHeight[Knob][Strobe] + 3 * EyeWidthAtCenter[Knob] );
        }
        if (SumWeights != 0) {
          CenterX[Knob] = (SumLR / SumWeights) / 2;
        } else {
          CenterX[Knob] = 0;
        }
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Dim1Index + Knob, CMD_SET_VAL_FC_UC, &CenterX[Knob]);
      }

      //
      // If consolation of the strobes are being done, then store the same center to DIM2, so that Create2DEye can read it directly
      // from cache.
      //
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Dim2Index, CMD_SET_VAL_FC_UC, &CenterY);
    } // Strobe loop ...
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  return MMRC_SUCCESS;
}
#endif // TRAINING_ALGOS == 1

#if TRAINING_ALGOS == 1
/**
  This routine sweeps any signal and returns the edges where the signal transitions from pass to fail.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Rank to sweep.
  @param[in]       Dim1Index       Signal type to sweep (i.e. RxDqsDelay, TxVref, CmdGrp0, etc).
  @param[in]       Dim1Knobs       How many knobs does this signal have? Most are 1 but ones
                                   like RxDqsDelay can have a HI and LOW (or P and N) component.
                                   For this case, Dim1Knobs would be 2
  @param[in]       Dim1Start       Starting value (per channel/knob/strobe/edge) for both the
                                   high and low edges.
  @param[in]       Dim1Min         The minimum value to allow the sweep to hit before stopping.
  @param[in]       Dim1Max         The maximum value to allow the sweep to hit before stopping.
  @param[in]       Dim1Step        Step size for sweep. Set to 1 for maximum detail or higher for speed improvements
                                   at the expense of accuracy.
  @param[in]       FullSweep       THIS IS NOT CURRENTLY IMPLEMENTED. If TRUE, sweep the entire range from min to max.
  @param[in] SkipInitialValueProgramming When TRUE, do not program initial low edge starting values or high edge
                                         starting values. This is useful when you want to margin a signal starting
                                         at its current value or when margining a group of signals that need to stay
                                         tethered together. For example, in CMD training if CmdGrp0 and CmdGrp1 need
                                         to sweep together but be programmed to different values (i.e. tethered),
                                         this would be set the TRUE so the two groups aren't programmed to an identical'
                                         starting value. If the user wants to skip the initial value programming,
                                         they MUST assign Dim1Start[][][][] to the current register settings upon entry
                                         to this routine.
  @param[in]       NumberElements  How many elements are we sweeping? Typically, MAX_STROBES for strobe based delays or 1
                                   for CMD signals or others which aren't strobe-based.
  @param[in]       ExecuteTest     Pointer to the point test function which evaluates pass/fail for the current value
                                   of the signal.
  @param[in, out]  Results         Results array returning the high/low edges per channel/knob/strobe.
  @param[in        HeaderLabel     Label to print if debugging is turned on. If "S" then the header will be a strobe
                                   header like S00   S01   S02, etc.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
Create1DSweepLastPass (
  IN         MMRC_DATA    *MrcData,
  IN         UINT8        Channel,
  IN         UINT8        Rank,
  IN         UINT16       Dim1Index,
  IN         UINT8        Dim1Knobs,
  IN         UINT16       Dim1Start[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2],
  IN         UINT16       Dim1Min[MAX_CHANNELS][MAX_STROBES],
  IN         UINT16       Dim1Max[MAX_CHANNELS][MAX_STROBES],
  IN         UINT16       Dim1Step,
  IN         BOOLEAN      FullSweep,
  IN         BOOLEAN      SkipInitialValueProgramming,
  IN         BOOLEAN      PerBitSweep,
  IN         UINT8        NumberElements,
  IN         MMRC_STATUS  ExecuteTest (MMRC_DATA *, UINT8, UINT8, UINT8, UINT8, UINT8 PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]),
  IN   OUT   UINT16       Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2],
  IN         UINT8        *HeaderLabel,
  IN         UINT8        DitherVal
  )
{
  UINT8     LowHighFlag[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT8     Element;
  UINT8     Knob;
  UINT8     PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3];
  UINT8     ChannelBackup;
  UINT32    Value[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES];
  UINT8     TestFinished;
  BOOLEAN   EdgeFound[MAX_CHANNELS];
  BOOLEAN   ElementDone[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT8     Bits;
  UINT8     MaximumBits;
  BOOLEAN   ElementBitDone;
  UINT8     Phase[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  BOOLEAN   SwitchDirection;
  UINT8     Bits2;
  UINT8     st;
  UINT32    MaxVal=0;
  UINT16    HighsideOffsetAdjust;
  //Added new variables for TxVref in RMT
  BOOLEAN   ShareBetweenStrobeGroups = FALSE;
  BOOLEAN   ShareValHasBeenSet = FALSE;
  //end
  ChannelBackup = Channel;
  //Added new condition for TxVref in RMT
  if (Dim1Index == TxVref) {
    ShareBetweenStrobeGroups = TRUE;
  }
  //end

#if RTWT_SIM == 1
  {
    UINT8  Strobe;
    UINT8  Knob;
    UINT8  Bit;
    if (Dim1Index == TxVref) {
      UINT32 Pi;
      UINT32 PiBit;
      UINT8  Vref;
      UINT8  State;
      for (Knob=0; Knob < MAX_KNOBS; Knob++) {
        for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
          for (Bit=0; Bit < MAX_BITS; Bit++) {
            GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, TxDqDelay, CMD_GET_REG, &Pi);
            GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, TxDqBitDelay, CMD_GET_REG, &PiBit);
            Pi -= PiBit;
            State=0;
            for (Vref=(UINT8) Dim1Min[Channel][Strobe]; Vref < (UINT8) Dim1Max[Channel][Strobe]; Vref+=(UINT8) Dim1Step) {
              //
              // Search for the low side passing.
              //
              if (State == 0) {
                if (SimWriteData[Channel][Rank][Knob][Strobe][Bit][Vref][LOW] < Pi &&
                    SimWriteData[Channel][Rank][Knob][Strobe][Bit][Vref][HIGH] > Pi) {
                      Results[Channel][Knob][Strobe][Bit][LOW] = Vref;
                      State = 1;
                }
              } else if (State == 1) {
                if (SimWriteData[Channel][Rank][Knob][Strobe][Bit][Vref][LOW] > Pi ||
                    SimWriteData[Channel][Rank][Knob][Strobe][Bit][Vref][HIGH] < Pi) {
                      Results[Channel][Knob][Strobe][Bit][HIGH] = Vref-Dim1Step;
                      State = 2;
                      break;
                }
              }
            }  // for Vref
          }
        }
      }
    }
    if (Dim1Index == RxVref) {
      UINT32 CTLE;
      UINT32 Pi;
      UINT32 PiBit;
      UINT8  Vref;
      UINT8  State;

      for (Knob=0; Knob < MAX_KNOBS; Knob++) {
        for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
          for (Bit=0; Bit < MAX_BITS; Bit++) {
            GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Ctle, CMD_GET_REG, &CTLE);
            GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxDqsPDelay, CMD_GET_REG, &Pi);
            GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxDqBitDelay, CMD_GET_REG, &PiBit);
            Pi -= PiBit;
            State=0;
            for (Vref=(UINT8) Dim1Min[Channel][Strobe]; Vref < (UINT8) Dim1Max[Channel][Strobe]; Vref+=(UINT8) Dim1Step) {
              //
              // Search for the low side passing.
              //
              if (State == 0) {
                if (SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][Vref][LOW] < Pi &&
                    SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][Vref][HIGH] > Pi) {
                      Results[Channel][Knob][Strobe][Bit][LOW] = Vref;
                      State = 1;
                }
              } else if (State == 1) {
                if (SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][Vref][LOW] > Pi ||
                    SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][Vref][HIGH] < Pi) {
                      Results[Channel][Knob][Strobe][Bit][HIGH] = Vref-Dim1Step;
                      State = 2;
                      break;
                }
              }
            }  // for Vref
          } // for Bit
        } // for Strobe
      } // for Knob
    } else if (Dim1Index == RxDqsPDelay) {
      UINT32 CTLE;
      UINT32 VREF;

     for (Knob=0; Knob < MAX_KNOBS; Knob++) {
      for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
        for (Bit=0; Bit < MAX_BITS; Bit++) {
         GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Ctle, CMD_GET_REG, &CTLE);
         GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxVref, CMD_GET_REG, &VREF);
         Results[Channel][Knob][Strobe][Bit][LOW]  = SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][VREF][LOW];
         Results[Channel][Knob][Strobe][Bit][HIGH] = SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][VREF][HIGH];
        } // for Bit
      }  // for Strobe
    } // for Knob
  } else if (Dim1Index == RxDqsDelay) {
      UINT32 CTLE;
      UINT32 VREF;

     for (Knob=0; Knob < MAX_KNOBS; Knob++) {
      for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
        for (Bit=0; Bit < MAX_BITS; Bit++) {
         GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Ctle, CMD_GET_REG, &CTLE);
         GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RxVref, CMD_GET_REG, &VREF);
         Results[Channel][Knob][Strobe][Bit][LOW]  = SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][VREF][LOW];
         Results[Channel][Knob][Strobe][Bit][HIGH] = SimReadData[CTLE][Channel][Rank][Knob][Strobe][Bit][VREF][HIGH];
        } // for Bit
      }  // for Strobe
    } // for Knob
  }  // if Element
  }
  return MMRC_SUCCESS;
#endif

  if (PerBitSweep == TRUE) {
    MaximumBits = MAX_BITS;
  } else {
    MaximumBits = 1;
  }

  //
  // Print out the header.
  //
  if (*HeaderLabel != 0 && RunOnThisChannel (MrcData, Channel, Rank)) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "    "));
    for (Knob = 0; Knob < Dim1Knobs; Knob++) {
      for (Element = 0; Element < NumberElements; Element++) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Element)) {
          continue;
        }
        MmrcDebugPrint ((MMRC_DBG_MIN, "     %s%02dK%d   ", HeaderLabel, Element, Knob));
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
  }

  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    EdgeFound[Channel]=FALSE;

    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //
    // Set all the initial values.
    //

    for (Knob = 0; Knob < Dim1Knobs; Knob++) {
      for (Element = 0; Element < NumberElements; Element++) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Element)) {
          continue;
        }
        for (Bits = 0; Bits < MaximumBits; Bits++) {
          if (Dim1Start[Channel][Knob][Element][LOW] >= Dim1Min[Channel][Element]) {
            LowHighFlag[Channel][Knob][Element][Bits]    = LOW;
            Results[Channel][Knob][Element][Bits][LOW]   = Dim1Start[Channel][Knob][Element][LOW];
            Results[Channel][Knob][Element][Bits][HIGH]  = 0;
          } else {
            LowHighFlag[Channel][Knob][Element][Bits]    = HIGH;
            Results[Channel][Knob][Element][Bits][LOW]   = 0;
            Results[Channel][Knob][Element][Bits][HIGH]  = Dim1Start[Channel][Knob][Element][HIGH];
          }

         }
        //
        // If the user wants to skip the initial value programming, they MUST assign Dim1Start[][][][]
        // to the current register settings upon entry to this routine.
        //
        if (!SkipInitialValueProgramming) {
          Value[Channel][Knob][Element] = Dim1Start[Channel][Knob][Element][LOW];
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Element, Dim1Index, CMD_SET_VAL_FC, &Value[Channel][Knob][Element]);
        }
      }
    }
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...
  //
  // Initialize Pass/Fail flags to fail before running the sweep for the low or high edge.
  //
  MmrcMemset (PassFail, RESULTS_INIT, MAX_CHANNELS * MAX_KNOBS * MAX_STROBES * MAX_BITS * 3 * sizeof (UINT8));
  MmrcMemset (ElementDone, 0, MAX_CHANNELS * MAX_KNOBS * MAX_STROBES * MAX_BITS * sizeof (UINT8));
  MmrcMemset (Phase, 0, MAX_CHANNELS * MAX_KNOBS * MAX_STROBES * MAX_BITS * sizeof (UINT8)); // Set phases for each element to SEARCH_FOR_EDGE.
  TestFinished = FALSE;

  //
  // First find the low edge then find the high edge.
  //
  while (!TestFinished) {
    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      //
      // Set the 1st dimension to the sweep value.
      //
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        ShareValHasBeenSet = FALSE; //Added this line for enabling TxVref in RMT
        for (Element = 0; Element < NumberElements; Element++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Element)) {
            continue;
          }
          ElementBitDone = TRUE;
          for (Bits = 0; Bits < MaximumBits; Bits++) {
            ElementBitDone &= ElementDone[Channel][Knob][Element][Bits];
          }
          if ((ShareBetweenStrobeGroups == TRUE) && (ShareValHasBeenSet == TRUE)) continue;  // Added this line for enabling TxVref in RMT
          if (!ElementBitDone && (Value[Channel][Knob][Element] != 0)) {
            for (Bits = 0; Bits < 1/*MaximumBits*/; Bits++) {
              // Added another case that we need to set the offset.
              // In the case CMD/CTL/CK margin test, after finding the LOW side edge, because the SkipInitialValueProgramming = TRUE
              // was used, the new HIGH side start was not programmed yet. It needs to be programmed here.
              if (( PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] != RESULTS_INIT) ||
                ( EdgeFound[Channel] && SkipInitialValueProgramming )){
                GetSetDdrioGroup2 (MrcData, Channel, Rank, Element, Dim1Index, CMD_SET_OFFSET, &Value[Channel][Knob][Element]);
                if (ShareBetweenStrobeGroups == TRUE) {
                  ShareValHasBeenSet = TRUE;
                } //Added this line for enabling TxVref in RMT
              }
            }
          }
        }
      }
      if (!MrcData->ExecuteThisRoutineInParallel) {
        //
        // Stop the routine after first execution
        // if not supposed to be executed in parallel
        //
        break;
      }
    } // Channel loop ...
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
    //
    // Run the test and collect results in PassFail.
    //
    ExecuteTest (MrcData, Channel, Rank, NumberElements, Dim1Knobs, PassFail);
    //
    // Look at all tested elements.
    //
    TestFinished = TRUE;

    for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) {
        continue;
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "CH%02d", Channel));
      //
      // Print out the result from the test.
      //
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        for (Element = 0; Element < NumberElements; Element++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Element)) {
            continue;
          }
          for (Bits = 0; Bits < MaximumBits; Bits++) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "    %04d:%s   ", Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]], ResultsString[PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE]]));
          } //for Bits
        }
      }
      MmrcDebugPrint((MMRC_DBG_MIN, "\n"));
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        for (Element = 0; Element < NumberElements; Element++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Element)) {
            continue;
          }
          Value[Channel][Knob][Element] =0;
          for (Bits = 0; Bits < MaximumBits; Bits++) {
            EdgeFound[Channel] = TRUE;
            //
            // If we are on the first pass OR the current and previous sample match, it means we haven't found an edge.
            //
            if (Phase[Channel][Knob][Element][Bits] == 0 && !FoundEdge (PassFail[Channel][Knob][Element][Bits], 0) && !ElementDone[Channel][Knob][Element][Bits]) {
              //
              // If we made it here it means at least 1 element/bits hasn't found an edge.
              //
              EdgeFound[Channel]  = FALSE;
              //
              // Now either increase or decrease the results array in preparation for the next pass.
              // If we are on the LOW side and we detect a failure, increase.
              // If we are on the LOW side and we detect a pass, decrease.
              // If we are on the HIGH side and we detect a failure, decrease.
              // If we are on the HIGH side and we detect a pass, increase.
              //
              if ((LowHighFlag[Channel][Knob][Element][Bits] == LOW && PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] == RESULTS_PASS) ||
                  (LowHighFlag[Channel][Knob][Element][Bits] == HIGH && PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] == RESULTS_FAIL)) {
                if (Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] - Dim1Step < Dim1Min[Channel][Element]) {
                  //
                  // We've reached a limit. Mark it as an edge.
                  //
                  EdgeFound[Channel] = TRUE;
                } else {
                  Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] -= Dim1Step;
                  Value[Channel][Knob][Element] = 0 - Dim1Step;
                  TestFinished        = FALSE;
                }
              } else {
                if ((Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] + Dim1Step > Dim1Max[Channel][Element]) ||
                    (Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] + Dim1Step < Dim1Min[Channel][Element])
                  ) {
                  //
                  // We've reached a limit. Mark it as an edge.
                  //
                  EdgeFound[Channel] = TRUE;
                } else {
                  Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] += Dim1Step;
                  Value[Channel][Knob][Element] = Dim1Step;
                  TestFinished        = FALSE;
                }
              }
            }
            if (Phase[Channel][Knob][Element][Bits] == 0 && EdgeFound[Channel])  {
              //
              // We have found an edge or reached a min/max limit of the sweep.
              //
              //
              // Move back to the last passing element if this last sample was a failure.
              //
              if (!ElementDone[Channel][Knob][Element][Bits] && PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] == RESULTS_FAIL) {
                if (LowHighFlag[Channel][Knob][Element][Bits] == LOW) {
                  //
                  // Make sure we don't exceed the max.
                  //
                  if (Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] + Dim1Step <= Dim1Max[Channel][Element]) {
                    Results[Channel][Knob][Element][Bits][LOW] += Dim1Step;
                    //rta Value[Channel][Knob][Element]  += Dim1Step;
                    PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] = RESULTS_PASS;
                  }
                } else {
                  //
                  // Make sure we don't exceed the min.
                  //
                  if (Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] - Dim1Step >= Dim1Min[Channel][Element]) {
                    Results[Channel][Knob][Element][Bits][HIGH] -= Dim1Step;
                    //rta Value[Channel][Knob][Element]  -= Dim1Step;
                    PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] = RESULTS_PASS;
                  }
                }
              }
              //
              // Change the phase to the dithering phase.
              //
              if (DitherVal == 0) {
                Phase[Channel][Knob][Element][Bits] = 3; // Skip Dithering.
              } else {
                Phase[Channel][Knob][Element][Bits] = 1; // Dithering.
              }
            }
            //
            // If in the dithering phase, then we are in a position of passing... margin until "DiterVal" consecutive Passes.
            //
            if (Phase[Channel][Knob][Element][Bits] == 1) {
              // Now either increase or decrease the results array in preparation for the next pass.
              // If we are on the LOW side and we detect a failure, increase.
              // If we are on the LOW side and we detect a pass, decrease.
              // If we are on the HIGH side and we detect a failure, decrease.
              // If we are on the HIGH side and we detect a pass, increase.
              //
              if (PassFail[Channel][Knob][Element][Bits][EDGE_SAMPLES] == DitherVal) {
                Phase[Channel][Knob][Element][Bits] = 2;
              } else {
                if (PassFail[Channel][Knob][Element][Bits][CURRENT_SAMPLE] == RESULTS_PASS) {
                  PassFail[Channel][Knob][Element][Bits][EDGE_SAMPLES]++;
                } else {
                  PassFail[Channel][Knob][Element][Bits][EDGE_SAMPLES] = 0;
                }
                if (LowHighFlag[Channel][Knob][Element][Bits] == LOW ) {
                  if (Results[Channel][Knob][Element][Bits][LOW] + 1 > Dim1Max[Channel][Element]) {
                    //
                    // We've reached a limit. Go to next phase.
                    //
                     Phase[Channel][Knob][Element][Bits] = 2;
                  } else {
                    Results[Channel][Knob][Element][Bits][LOW] += 1;
                    Value[Channel][Knob][Element] += 1;
                    TestFinished = FALSE;
                  }
                } else {
                  if (Results[Channel][Knob][Element][Bits][HIGH] - 1 < Dim1Min[Channel][Element]) {
                    //
                    // We've reached a limit. G oto next phase.
                    //
                     Phase[Channel][Knob][Element][Bits] = 2;
                  } else {
                    Results[Channel][Knob][Element][Bits][LowHighFlag[Channel][Knob][Element][Bits]] -= 1;
                    Value[Channel][Knob][Element] -= 1;
                    TestFinished = FALSE;
                  }
                }
              }
            }
            //
            // Found the dithering edge.. need to subtract the amount to get back to the original passing region.
            //
            if (Phase[Channel][Knob][Element][Bits] == 2) {
              //
              // The last sample was as PASS so we need to back up to the first PASS value, taking into account
              // dithering. If we had a DitherVal of 3 and sampled F F P P P, we need to subtract 2 to get back
              // to the first P sample. However if we have 0 EDGE_SAMPLES it could mean a situation where our first
              // sample was at the limit. For example, we sample a PASS at the lower limit delay of 0. In this case
              // we don't want to do any sort of backing up.
              //
              if (!ElementDone[Channel][Knob][Element][Bits] && PassFail[Channel][Knob][Element][Bits][EDGE_SAMPLES] != 0) {
                if (LowHighFlag[Channel][Knob][Element][Bits] == LOW) {
                  Results[Channel][Knob][Element][Bits][LOW] -= ((PassFail[Channel][Knob][Element][Bits][EDGE_SAMPLES]));
                } else {
                  Results[Channel][Knob][Element][Bits][HIGH] += ((PassFail[Channel][Knob][Element][Bits][EDGE_SAMPLES]));
                }
              }
              Phase[Channel][Knob][Element][Bits] = 3;
            }

            if (Phase[Channel][Knob][Element][Bits] == 3) {
              //
              // If the high edge has been found, this element is done.
              //
              if (LowHighFlag[Channel][Knob][Element][Bits] == HIGH) {
                //
                // At the completion of the element edges, check if there was no low side.. if not, then assign
                // the high value to the low value.
                //
                if (Dim1Start[Channel][Knob][Element][LOW] < Dim1Min[Channel][Element]) {
                  Results[Channel][Knob][Element][Bits][LOW] = Results[Channel][Knob][Element][Bits][HIGH];
                }
                //
                // At the completion of the element edges, check if there was no high side.. if not, then assign
                // the low value to the high value.
                //
                if (Dim1Start[Channel][Knob][Element][HIGH] > Dim1Max[Channel][Element]) {
                  Results[Channel][Knob][Element][Bits][HIGH] = Results[Channel][Knob][Element][Bits][LOW];
                }
                ElementDone[Channel][Knob][Element][Bits] = TRUE;
              } else if (LowHighFlag[Channel][Knob][Element][Bits] == LOW && Dim1Start[Channel][Knob][Element][HIGH] > Dim1Max[Channel][Element]) {
                ElementDone[Channel][Knob][Element][Bits] = TRUE;
                Results[Channel][Knob][Element][Bits][HIGH] = Results[Channel][Knob][Element][Bits][LOW];
              } else {
                //
                // Switch directions to find the high edge.
                //
                Phase[Channel][Knob][Element][Bits] = 4;
                //LowHighFlag[Channel][Knob][Element][Bits]   = HIGH;

              }
            }
            if (Phase[Channel][Knob][Element][Bits] == 4) {
              SwitchDirection = TRUE;

              for (st = 0; st < NumberElements; st++) {
                for (Bits2=0; Bits2<MaximumBits; Bits2++) {
                  if (Phase[Channel][Knob][st][Bits2] != 4) {
                    SwitchDirection = FALSE;
                  }
                }
              }

              if (SwitchDirection == TRUE) {
                MmrcMemset(&(LowHighFlag[Channel][Knob]), HIGH, sizeof(UINT8)* MAX_STROBES * MAX_BITS);
                MmrcMemset(&(Phase[Channel][Knob]), 0, sizeof(UINT8)* MAX_STROBES * MAX_BITS);
                for (st = 0; st < NumberElements; st++) {
                  for (Bits=0; Bits < MaximumBits; Bits++) {
                    if (Dim1Start[Channel][Knob][st][HIGH] > Results[Channel][Knob][st][Bits][LOW]) {
                      Results[Channel][Knob][st][Bits][HIGH] = Dim1Start[Channel][Knob][st][HIGH];
                    } else {
                      Results[Channel][Knob][st][Bits][HIGH] = Results[Channel][Knob][st][Bits][LOW];
                    }
                    // Adding MaxVal for issue found on BXT-E, but relevant to BXT-P as well
                    // This makes sure that we pick one starting value for the highside sweep, across all the bits within one Element/Strobe
                    //  Previously it could pick different start values across bits, in some unusual corner cases, and this makes no sense
                    //  because the hardware only has one physical margin param across the bits in one strobe.  Sweep ends up out of sync
                    //  with what is really happening, and incorrect results are reported
                    if (MaxVal < Results[Channel][Knob][st][Bits][HIGH] || Bits == 0) {
                      MaxVal = Results[Channel][Knob][st][Bits][HIGH];
                    }

                    //
                    //   Add this to code below.  Code added the functionality where CMD/CTL/.. sweep, when switching
                    //   directions from loside to hiside sweep, will properly place CMD timing back at start sweep position for
                    //   for hiside.   Before completing this iteration of while !TestFinished, 'Value' has to contain the delta between
                    //   lowside sweep final point and hiside starting point, so that GetSet( OFFSET, Value ) will correctly offset CMD to
                    //   the hiside start value (This GetSet is at beginning of next while loop iteration).
                    //   Below code fixes one corner case, that if lowside sweep started with a pass but subsequently saw a fail while margining,
                    //   the 'PHASE 0' section above will adjust Results[] to the last past value, but the HW is still at that first fail
                    //   position, meaning we need to do an adjustment here so that HW finally gets put at the proper hiside start position.
                    HighsideOffsetAdjust = 0;
                    if ( Value[Channel][Knob][Element] > 0 ) {
                    // Note that if any bit hit the PASS=>FAIL condition on lowside sweep, during margining, the Value[..] will increment by Dim1Step
                    // This is enforced in the PHASE 0 logic above, and is unique for this PASS=>FAIL during margining scenario
                    // All other scenarios will have Value[..] == 0.  These other scenarios don't need this re-adjustment
                    HighsideOffsetAdjust = Dim1Step;
                  }

                  // calculate the "offset" value from the HIGH side start to the last LOW side edge.
                  // The CMD/CTL/CK margin tests use the SkipInitialValueProgramming=TRUE setting. With this
                  // setting, the CMD/CTL/CK value are set with the CMD_SET_OFFSET.
                  // The Value[Channel][Knob][Element] will be override in below if (!SkipInitialValueProgramming) { block
                  Value[Channel][Knob][Element] = Results[Channel][Knob][Element][Bits][HIGH] - Results[Channel][Knob][Element][Bits][LOW] + HighsideOffsetAdjust;
                  //
                  // Program HIGH side initial values. If user opts to skip this step, the HIGH side sweep
                  // will start where the LOW side ended.
                  //
                  TestFinished = FALSE;
                  MmrcMemset (&PassFail[Channel][Knob][st][Bits][0], RESULTS_INIT, 3);
                }    //  Bits loop *within* SwITCHDIRECTION==TRUE
                for (Bits=0; Bits < MaximumBits; Bits++) {
                  Results[Channel][Knob][st][Bits][HIGH] = (UINT16) MaxVal;
                }
                if (!SkipInitialValueProgramming) {
                  Value[Channel][Knob][st] = ConvertLinearToPhysicalValue (MrcData, Dim1Index, MaxVal);
                  GetSetDdrioGroup2 (MrcData, Channel, Rank, st, Dim1Index, CMD_SET_VAL_FC, &Value[Channel][Knob][st]);
                }
              }
              Element = NumberElements;
              }
            }
          }  //Bits

          if (EdgeFound[Channel]) {
          }

        } // Element loop ...

      } // Knob loop
      if (!MrcData->ExecuteThisRoutineInParallel) {
        //
        // Stop the routine after first execution
        // if not supposed to be executed in parallel
        //
        break;
      }
    } // Channel loop ...
  } // TestFinished loop ...
  //
  // Restore back the Channel Under Test
  //
  Channel = ChannelBackup;
  PrintFooter1DSweep (MrcData, Channel, Rank, Dim1Index, Dim1Knobs,  Results);
  return MMRC_SUCCESS;
}

/**
  Compares the initial sample with the current sample. If they don't match, it increases a counter by 1. When this
  counter equals DitherVal it means we've found an edge. Higher dither values means we look for a stronger edge.
  For example if DitherVal is 3 and the first sample if FAIL, it means this routine will return TRUE after sampling
  3 PASS values in a row.

  @param[in]       PassFail         Array of samples performed by the sweep function.
  @param[in]       DitherVal        The number of successive samples that need to be either PASS or FAIL before this routine
                                    considers an edge to be found. For example, if the first sample is FAIL, the sweep will
                                    look to read X number of PASS values in a row before it considers this a FAIL to PASS
                                    edge. X = DitherVal.
  @retval          TRUE/FALSE       Is the edge found?
**/
BOOLEAN
FoundEdge (
  IN        UINT8     PassFail[3],
  IN        UINT8     DitherVal
)
{
  //
  // If this is the first time in this routine, set the edge count to 0. This
  // count will increase when successive samples equal the same value. For example,
  // if we are sweeping and get FAIL, FAIL, PASS, PassFail[EDGE_SAMPLES] would be 1
  // because we've detected 1 sample in a row that's different from the first sample.
  // If we sample FAIL, FAIL, PASS, PASS, PASS, PassFail[EDGE_SAMPLES] = 3.
  //
  if (PassFail[EDGE_SAMPLES] == RESULTS_INIT) {
    PassFail[EDGE_SAMPLES] = 0;
    //
    // Save the first sample for comparison later.
    //
    PassFail[FIRST_SAMPLE] = PassFail[CURRENT_SAMPLE];
    return FALSE;
  }
  //
  // If the current sample equals the first sample of this sweep, then we haven't
  // found an edge and we need to re-init the edge count to 0.
  //
  if (PassFail[FIRST_SAMPLE] == PassFail[CURRENT_SAMPLE]) {
    PassFail[EDGE_SAMPLES] = 0;
    return FALSE;
  }
  //
  // If the current sample is different from the first sample, we need to increase the
  // edge count. If this count equals the dither value, it means we've read X successive
  // samples in a row which a DIFFERENT from the first sample, where X = DitherVal. This
  // means we've found an edge.
  //
  if (PassFail[FIRST_SAMPLE] != PassFail[CURRENT_SAMPLE]) {
    if (PassFail[EDGE_SAMPLES] >= DitherVal) {
      PassFail[EDGE_SAMPLES] = DitherVal;
      return TRUE;
    }
    PassFail[EDGE_SAMPLES]++;
  }
  return FALSE;
}

#endif // TRAINING_ALGOS == 1

/*
This is the last pass 1D sweep which finds the low edge for all strobes first before switching to
the high edge. This is less complicated than the 1D search routine above. Leaving this here for now
in case we need to bring it back for ANN PO.
MMRC_STATUS
  Create1DSweepLastPass (
  IN         MMRC_DATA    *MrcData,
  IN         UINT8        Channel,
  IN         UINT8        Rank,
  IN         UINT8        Dim1Index,
  IN         UINT16       Dim1Start[MAX_STROBES][2],
  IN         UINT16       Dim1Min[MAX_STROBES],
  IN         UINT16       Dim1Max[MAX_STROBES],
  IN         UINT16       Dim1Step,
  IN         BOOLEAN      FullSweep,
  IN         UINT8        NumberElements,
  IN         MMRC_STATUS   ExecuteTest (MMRC_DATA *, UINT8, UINT8, UINT8, UINT8 PassFail[MAX_STROBES][2]),
  IN   OUT   UINT16       Results[MAX_STROBES][2],
  IN         UINT8        *HeaderLabel
  )
{
  UINT8     LowHighFlag;
  UINT8     Element;
  UINT8     TestFinished;
  UINT8     PassFail[MAX_STROBES][2];
  BOOLEAN   ElementDone[MAX_STROBES];
  UINT32    Value[MAX_STROBES];

  if (HeaderLabel != "") {
    for (Element = 0; Element < NumberElements; Element++) {
      MmrcDebugPrint ((MMRC_DBG_MAX, "    %s%02d   ", HeaderLabel, Element));
    }
    MmrcDebugPrint ((MMRC_DBG_MAX, "\n"));
  }
  //
  // First find the low edge then find the high edge.
  //
  for (LowHighFlag = LOW; LowHighFlag <= HIGH; LowHighFlag++) {
    //
    // Set all the initial values.
    //
    for (Element = 0; Element < NumberElements; Element++) {
      Results[Element][LowHighFlag] = Dim1Start[Element][LowHighFlag];
      Value[Element] = Results[Element][LowHighFlag];
      GetSetDdrioGroup2 (MrcData, Channel, Rank, Element, Dim1Index, CMD_SET_VAL_FC, &Value[Element]);
    }
    //
    // Initialize Pass/Fail flags to fail before running the sweep for the low or high edge.
    //
    MmrcMemset (PassFail, RESULTS_INIT, MAX_STROBES * 2 * sizeof (UINT8));
    MmrcMemset (ElementDone, 0, MAX_STROBES * sizeof (UINT8));
    TestFinished = FALSE;

    while (!TestFinished) {
      //
      // Set the 1st dimension to the sweep value.
      //
      for (Element = 0; Element < NumberElements; Element++) {
        if (!ElementDone[Element] && PassFail[Element][CURRENT_SAMPLE] != RESULTS_INIT) {
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Element, Dim1Index, CMD_SET_OFFSET, &Value[Element]);
          PassFail[Element][PREVIOUS_SAMPLE] = PassFail[Element][CURRENT_SAMPLE];
        }
      }

      ExecuteTest (MrcData, Channel, Rank, NumberElements, PassFail);
      //
      // Print out the result from this one sweep value.
      //
      for (Element = 0; Element < NumberElements; Element++) {
        MmrcDebugPrint ((MMRC_DBG_DEBUG, "   %03d:%c  ", Results[Element][LowHighFlag], ResultsString[PassFail[Element][CURRENT_SAMPLE]]));
      }
      MmrcDebugPrint((MMRC_DBG_DEBUG, "\n"));
      //
      // Look at all tested elements.
      //
      TestFinished = TRUE;
      for (Element = 0; Element < NumberElements; Element++) {
        //
        // If we are on the first pass OR the current and previous sample match, it means we haven't found an edge.
        //
        if (PassFail[Element][PREVIOUS_SAMPLE] == RESULTS_INIT ||
          (PassFail[Element][CURRENT_SAMPLE] == PassFail[Element][PREVIOUS_SAMPLE] &&
          !ElementDone[Element])) {
            //
            // If we made it here it means at least 1 element hasn't found an edge.
            //
            TestFinished = FALSE;
            //
            // Now either increase or decrease the results array in preparation for the next pass.
            // If we are on the LOW side and we detect a failure, increase.
            // If we are on the LOW side and we detect a pass, decrease.
            // If we are on the HIGH side and we detect a failure, decrease.
            // If we are on the HIGH side and we detect a pass, increase.
            //
            if ((LowHighFlag == LOW && PassFail[Element][CURRENT_SAMPLE] == RESULTS_PASS) ||
              (LowHighFlag == HIGH && PassFail[Element][CURRENT_SAMPLE] == RESULTS_FAIL)) {
                if (Results[Element][LowHighFlag] - Dim1Step < Dim1Min[Element]) {
                  TestFinished = TRUE;
                  break;
                } else {
                  Results[Element][LowHighFlag] -= Dim1Step;
                  Value[Element] = 0 - Dim1Step;
                }
            } else {
              if (Results[Element][LowHighFlag] + Dim1Step > Dim1Max[Element]) {
                TestFinished = TRUE;
                break;
              } else {
                Results[Element][LowHighFlag] += Dim1Step;
                Value[Element] = Dim1Step;
              }
            }
        } else {
          //
          // Move back to the last passing element if this last sample was a failure.
          //
          if (!ElementDone[Element] && PassFail[Element][CURRENT_SAMPLE] == RESULTS_FAIL) {
            if (LowHighFlag == LOW) {
              Results[Element][LOW] += Dim1Step;
            } else {
              Results[Element][HIGH] -= Dim1Step;
            }
          }
          ElementDone[Element] = TRUE;
        }
      }
    }
  }
  PrintFooter1DSweep (MrcData, Dim1Index, Results);

  return MMRC_SUCCESS;
}
*/

#if TRAINING_ALGOS == 1
MMRC_STATUS
Create2DSweep (
  IN          MMRC_DATA       *MrcData,
  IN   OUT    SAMPLE_SWEEP    Results[MAX_ELEMENTS_DIM_Y],
  IN          UINT8           Channel,
  IN          UINT8           Rank,
  IN          UINT16          Dim1Index,
  IN          UINT8           Dim1Knobs,
  IN          INT16           Dim1Start,
  IN          INT16           Dim1End,
  IN          UINT16          Dim1Step,
  IN          UINT8           Dim1Flag,
  IN          UINT16          Dim2Index,
  IN          INT16           Dim2Start,
  IN          INT16           Dim2End,
  IN          UINT16          Dim2Step,
  IN          UINT8           Dim2Flag,
  IN          MMRC_STATUS     ExecuteTest (MMRC_DATA *, UINT8, UINT8, UINT8, UINT8, UINT8 PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]),
  IN          UINT8           *HeaderLabel
)
{
  UINT16    Dim2Min[MAX_CHANNELS][MAX_STROBES];
  UINT16    Dim2Max[MAX_CHANNELS][MAX_STROBES];
  UINT32    Dim2PhyValue;
  UINT16    Dim1Min[MAX_CHANNELS][MAX_STROBES];
  UINT16    Dim1Max[MAX_CHANNELS][MAX_STROBES];
  UINT16    Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT8     Strobe;
  UINT8     Knob;
  UINT8     ChannelBackup;
  UINT16    DimYOffset;
  UINT8     LowHighFlag;
  BOOLEAN   AtLeastOneChannelUnderTest;
  BOOLEAN   TestDone;
  BOOLEAN   FirstStrobe;

  Dim2PhyValue  = 0;
  ChannelBackup = Channel;

  //
  // Get Min/Max points according with the Start/End values
  // and also the ABSOLUTE/RELATIVE flag
  //
  GetMinMaxRelativeAbsolute (MrcData, Channel, Rank, Dim2Index, Dim2Start, Dim2End, Dim2Flag, Dim2Min, Dim2Max);
  GetMinMaxRelativeAbsolute (MrcData, Channel, Rank, Dim1Index, Dim1Start, Dim1End, Dim1Flag, Dim1Min, Dim1Max);

  AtLeastOneChannelUnderTest = FALSE;

  //
  // Initialize Start point for Dim2 (outer loop)
  // START_SWEEP_CENTER only works when the MAX & MIN values has pretty much the same Delta across channels
  //
  if ((Dim2Flag & START_SWEEP_CENTER) != 0) {
    DimYOffset  = (Dim2Max[Channel][0] - Dim2Min[Channel][0]) / 2;
    DimYOffset -= DimYOffset % Dim2Step;
  } else {
    DimYOffset = 0;
  }

  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }

    AtLeastOneChannelUnderTest = TRUE;

    //
    // Initialize Start point for Dim1 (inner loop)
    //
    for (Knob = 0; Knob < Dim1Knobs; Knob++) {
      for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
          continue;
        }
        Dim1StartPoint[Channel][Knob][Strobe][LOW]  = Dim1Min[Channel][Strobe];
        Dim1StartPoint[Channel][Knob][Strobe][HIGH] = Dim1Max[Channel][Strobe];
      } // Strobe loop ...
    }
    if (!MrcData->ExecuteThisRoutineInParallel) {
      //
      // Stop the routine after first execution
      // if not supposed to be executed in parallel
      //
      break;
    }
  } // Channel loop ...

  if (!AtLeastOneChannelUnderTest) {
    return MMRC_SUCCESS;
  }

  //
  // Search for Low Edge or Min boundary then
  // start to search High Edge or Max boundary
  //
  for (LowHighFlag = LOW; LowHighFlag <= HIGH; LowHighFlag++) {
    //
    // When the SWEEP_ALL_STROBES_TOGETHER is enabled we consider the test as
    // NotDone, and as soon the first failure is detected it is declared the test is Done
    // When it is not enabled we consider the test as NotDone, and if it detects ALL as passing
    // margin the test automatically is declared as Done
    //
    TestDone = FALSE;
    do {
      //
      // Set the value for all strobes
      //
      for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          continue;
        }
        for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
            continue;
          }
          Dim2PhyValue = Dim2Min[Channel][Strobe] + DimYOffset;
          GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Dim2Index, CMD_SET_VAL_FC, &Dim2PhyValue);
        } // Strobe loop ...
        MmrcDebugPrint ((MMRC_DBG_MIN, "CH%02d %s %03d\n", Channel,
          SignalInfo[GetSignalInfoIndex (GetOffsetIndex (MrcData, Dim2Index))].Name,
          Dim2PhyValue
          ));
        if (!MrcData->ExecuteThisRoutineInParallel) {
          //
          // Stop the routine after first execution
          // if not supposed to be executed in parallel
          //
          break;
        }
      } // Channel loop ...
      //
      // Restore back the Channel Under Test
      //
      Channel = ChannelBackup;
      //
      // Create the 1D Sweep
      //
      Create1DSweepLastPass (MrcData, Channel, Rank, Dim1Index, Dim1Knobs, Dim1StartPoint, Dim1Min, Dim1Max, Dim1Step, FALSE, FALSE, FALSE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], ExecuteTest, Results[DimYOffset].Sample, HeaderLabel, 1);
      //
      // Check the results across channels
      //
      for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
        if (!RunOnThisChannel (MrcData, Channel, Rank)) {
          continue;
        }
        for (Knob = 0; Knob < Dim1Knobs; Knob++) {
          FirstStrobe = TRUE;
          for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
            if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
              continue;
            }
            //
            // If this sample passed, create new starting points for the next time the 1D sweep is called.
            // This will allow the 1D sweep to start at the last known passing points instead of doing
            // a full sweep to save time.
            //
            if (Results[DimYOffset].Sample[Channel][Knob][Strobe][0][HIGH] > Results[DimYOffset].Sample[Channel][Knob][Strobe][0][LOW]) {
              Dim1StartPoint[Channel][Knob][Strobe][LOW]  = Results[DimYOffset].Sample[Channel][Knob][Strobe][0][LOW];
              Dim1StartPoint[Channel][Knob][Strobe][HIGH] = Results[DimYOffset].Sample[Channel][Knob][Strobe][0][HIGH];
              if ((Dim2Flag & SWEEP_ALL_STROBES_TOGETHER) != 0) {
                //
                // Do nothing
                //
              } else {
                //
                // As soon any Strobe passes we have to mark the test inmmediatly as NotDone
                //
                TestDone = FALSE;
              }
            } else {
              if ((Dim2Flag & SWEEP_ALL_STROBES_TOGETHER) != 0) {
                TestDone = TRUE;
                break;
              } else {
                if (FirstStrobe) {
                  //
                  // Start tracking from the Strobe[First] and if fails mark the Test as Done
                  //
                  TestDone = TRUE;
                }
              }
              FirstStrobe = FALSE;
            }
          } // Strobe loop ...
        } // Knob loop ...
        if (!MrcData->ExecuteThisRoutineInParallel) {
          //
          // Stop the routine after first execution
          // if not supposed to be executed in parallel
          //
          break;
        }
      } // Channel loop ...
      //
      // Restore back the Channel Under Test
      //
      Channel = ChannelBackup;

      if (LowHighFlag == LOW) {
        if (((DimYOffset - Dim2Step) < 0) || TestDone) {
          //
          // Done with the Low Side
          //
          TestDone = TRUE;
          if ((Dim2Flag & START_SWEEP_CENTER) != 0) {
            DimYOffset  = (Dim2Max[Channel][0] - Dim2Min[Channel][0]) / 2;
            DimYOffset -= DimYOffset % Dim2Step;
          } else {
            DimYOffset = 0;
          }
          DimYOffset += Dim2Step;
        } else {
          DimYOffset -= Dim2Step;
        }
      } else {
        if ((DimYOffset + Dim2Step) > (Dim2Max[Channel][0] - Dim2Min[Channel][0])) {
          //
          // Done with the High Side
          //
          TestDone = TRUE;
        } else {
          DimYOffset += Dim2Step;
        }
      }
    } while (!TestDone);
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
    if (DimYOffset > (Dim2Max[Channel][0] - Dim2Min[Channel][0])) {
      //
      // Done automatically with with the High Side
      //
      break;
    }
  } // LowHighFlag loop ...

  return MMRC_SUCCESS;
}

#endif //#if TRAINING_ALGOS == 1

#if TRAINING_ALGOS == 1
MMRC_STATUS
SweepCenterEye2D(
IN   MMRC_DATA    *MrcData,
IN   UINT8        Channel,
IN   UINT8        Rank,
IN   UINT16       Dim1Index,
IN   INT16        Dim1Start,
IN   INT16        Dim1End,
IN   UINT16       Dim1Step,
IN   UINT8        Dim1Flag,
IN   UINT16       Dim2Index,
IN   INT16        Dim2Start,
IN   INT16        Dim2End,
IN   UINT16       Dim2Step,
IN   UINT8        Dim2Flag,
IN   MMRC_STATUS  ExecuteTest(MMRC_DATA *, UINT8, UINT8, UINT8, UINT8, UINT8 PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]),
IN  UINT8        *HeaderLabel

)
{
    UINT8   ConsolidateFlag;
    UINT8   Dim1Knobs;

    SAMPLE_SWEEP  Results[MAX_ELEMENTS_DIM_Y];
    UINT16  Dim1Min[MAX_CHANNELS][MAX_STROBES];
    UINT16  Dim1Max[MAX_CHANNELS][MAX_STROBES];
    UINT16  Dim2Min[MAX_CHANNELS][MAX_STROBES];
    UINT16  Dim2Max[MAX_CHANNELS][MAX_STROBES];

    MmrcMemset(Results, 0xff, MAX_ELEMENTS_DIM_Y * sizeof (SAMPLE_SWEEP));
    if (Dim2Flag & CONSOLIDATE_STROBES) {
        ConsolidateFlag = 1;
    }
    else {
        ConsolidateFlag = 0;
    }
    if (Dim1Flag & MULTIKNOB) {
        Dim1Knobs = 2;
    }
    else {
        Dim1Knobs = 1;
    }
    GetMinMaxRelativeAbsolute(MrcData, Channel, Rank, Dim1Index, Dim1Start, Dim1End, Dim1Flag, Dim1Min, Dim1Max);
    GetMinMaxRelativeAbsolute(MrcData, Channel, Rank, Dim2Index, Dim2Start, Dim2End, Dim2Flag, Dim2Min, Dim2Max);

#if READWRITE_TRAINING_SIM == 0
    Create2DSweep(MrcData, Results, Channel, Rank, Dim1Index, Dim1Knobs, Dim1Start, Dim1End, Dim1Step, Dim1Flag, Dim2Index, Dim2Start, Dim2End, Dim2Step, Dim2Flag, ExecuteTest, HeaderLabel);
#else
    Create2DSweepSim(MrcData, Results, Channel, Rank, Dim1Index, Dim1Knobs, Dim1Start, Dim1End, Dim1Step, Dim1Flag, Dim2Index, Dim2Start, Dim2End, Dim2Step, Dim2Flag);
#endif
    Center2D(MrcData, Results, Channel, Rank, Dim1Index, Dim1Knobs, Dim1Min, Dim2Index, Dim2Min, Dim2Step, ConsolidateFlag);
    Create2DEye(MrcData, Results, Channel, Rank, Dim1Index, Dim1Knobs, Dim1Min, Dim1Step, Dim2Index, Dim2Min, Dim2Step);

    return MMRC_SUCCESS;
}
#endif // TRAINING_ALGOS == 1

/**
  Precharge all will detect the DDR type DDR3 vs DDR4 and apply
  the correct command

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PrechargeAll (
  IN     MMRC_DATA *MrcData,
  IN     UINT8      Channel,
  IN     UINT8      Rank
  )
{
#if USE_GET_SET_FROM_SPREADSHEET
  BOOLEAN   ExecuteMode;

  ExecuteMode = MrcData->ExecuteThisRoutineInParallel;
  MrcData->ExecuteThisRoutineInParallel = FALSE;

  MrcData->DynamicVars[Channel][RANK] = Rank;
  MrcData->DynamicVars[Channel][RDDCMD] = 0;
  MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;
  switch (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType) {
  case TypeLpDdr3:
      MrcData->DynamicVars[Channel][REG_DATA] = 0x1b;
      break;
  case TypeLpDdr4:
      MrcData->DynamicVars[Channel][REG_DATA] = 0x30;
      break;
  case TypeDdr3:
      MrcData->DynamicVars[Channel][REG_DATA] = 0x100400;
      break;
  default:
      MrcData->DynamicVars[Channel][REG_DATA] = 0x100400;
      break;
  }

  SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
 // MmrcExecuteTask (MrcData, PRECHARGEALL, NO_PRINT, Channel);

  MrcData->ExecuteThisRoutineInParallel = ExecuteMode;
#endif
  return MMRC_SUCCESS;
}

/**
  Wrapper function to call memory init or memory test.  Also handles error
  conditions such as whether or not to halt the system on errors

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       MemInitFlag     0 for memtest, 1 for meminit.

  @retval          Status of success or failure, depending on
                   errors observed. Also prints out which rank
                   failed the CPGC test if failure occurs. For
                   rank detection to work, the address map must
                   be left in its default state. If MRC changes
                   the address map, this function must be
                   modified to change it back to default at the
                   beginning, then restore it at the end.
**/
MMRC_STATUS
MemInit (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Channel,
  IN        UINT8        MemInitFlag
  )
{
#if ECC_SCRUB == 1 || MEMORY_TEST == 1
  UINT32    LoopNum;
  UINT32    MaxLoopNum;
  UINT32    Errors;
  UINT8     StrobeLane;
  UINT8     ChannelBackup;
  UINT8     Sequence;
  UINT8     PatternSelect;
  BOOLEAN   GlobalError;


  LoopNum       = 0;
  MaxLoopNum    = 1;
  GlobalError   = FALSE;
  ChannelBackup = Channel;
  //
  // Set the total number of memory tests to run.
  //
  if (MrcData->MemoryTestLoop != 0) {
    MaxLoopNum = MrcData->LoopCount;
  }
  switch (MemInitFlag) {
  case MrcMemInit:
    for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
      MrcData->DynamicVars[Channel][ENABLE_RUN_TEST] = (UINTX) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled & MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel);
      //
      // If user requests memory init, write 0s to all the memory and return.
      //
      MrcData->DynamicVars[Channel][SEQ_START]       = 0;
      MrcData->DynamicVars[Channel][SEQ_END]         = 0;
      MrcData->DynamicVars[Channel][BUF_START_PTR]   = 0;
      MrcData->DynamicVars[Channel][BUF_END_PTR]     = 0;
      L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_MEMINIT);
      L_CpgcReconfig (MrcData, Channel);
    }
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
    //
    // Setup, execute and read errors when parallel training is enabled
    //
    CpgcExecuteParallel (MrcData, Channel, 0);

    return MMRC_SUCCESS;
  case MrcMemTest:
    for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
      L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_MEMINIT);
    }
    //
    // Restore back the Channel Under Test
    //
    Channel = ChannelBackup;
    while (LoopNum < MaxLoopNum) {
      LoopNum++;
      //
      // If we're cycling, display the current count.
      //
      if (MrcData->MemoryTestLoop != 0) {
        MmrcDebugPrint ((MMRC_DBG_MAX, "Memtest %d\n", LoopNum));
      }
      for (PatternSelect = 0; PatternSelect < 4; PatternSelect++) {
        for (Sequence = 0; Sequence < 2; Sequence++) {
          //
          // Sequence = 0; will execute Writes
          // Sequence = 1; will execute Reads
          //
          for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
            MrcData->DynamicVars[Channel][ENABLE_RUN_TEST] = (UINTX) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled & MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel);
            MrcData->DynamicVars[Channel][SEQ_START]       = Sequence;
            MrcData->DynamicVars[Channel][SEQ_END]         = Sequence;
            MrcData->DynamicVars[Channel][BUF_START_PTR]   = PatternSelect;
            MrcData->DynamicVars[Channel][BUF_END_PTR]     = PatternSelect;
            //
            // Setup all CPGC registers for first write pass of memtest.
            //
            L_CpgcReconfig (MrcData, Channel);
          }
          //
          // Restore back the Channel Under Test
          //
          Channel = ChannelBackup;
          //
          // Setup, execute and read errors when parallel training is enabled
          //
          CpgcExecuteParallel (MrcData, Channel, 0);

          if (Sequence == 1) {
            for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
              if (MrcData->DynamicVars[Channel][ENABLE_RUN_TEST] == 0) {
                continue;
              }
              //
              // Check Errors during Read Sequence
              //
              Errors = (UINT32) MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
              if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
                Errors |= (UINT32)(MrcData->DynamicVars[Channel][ECC_ERR_STAT] << 8);
              }

#if FAULTY_PART_TRACKING
              if (MrcData->FaultyPartTracking) {
                CpgcSCheckErrors (MrcData, Channel, 0, NULL, NULL, &Errors);
              }
#endif
              //
              // If there are errors, decode them and optionally halt the system.
              //
              if (Errors != 0) {
                MmrcDebugPrint ((MMRC_DBG_MIN, "Memtest failed on channel %d - 0x%08x\n", Channel, Errors));
                //
                // Print out which bytelanes had errors.
                //
                for (StrobeLane = 0; StrobeLane < MAX_STROBES; StrobeLane++) {
                  if (!IsStrobeValid (MrcData, Channel, 0, StrobeLane)) {
                    continue;
                  }
                  //
                  // StrobeLane errors indicators are in bits 16:8.
                  //
                  if ((Errors & (1 << (StrobeLane))) != 0) {
                   MmrcDebugPrint ((MMRC_DBG_MIN, "Error on strobe %d\n", StrobeLane));
                  }
                }
                  GlobalError = TRUE;
              } // Errors != 0

              if (!MrcData->ExecuteThisRoutineInParallel) {
                //
                // Stop the routine after first execution
                // if not supposed to be executed in parallel
                //
                break;
              }
            } // Channel loop ...
          } // Sequence == 1 (Read Sequence)
        } // Sequence loop ...
      } // PatternSelect loop ...
    } // LoopNum < MaxLoopNum loop ...
    if (GlobalError) {
      return MMRC_FAILURE;
    }
    break;
  default:
    return MMRC_FAILURE;
  }
#endif
  return MMRC_SUCCESS;
}

/**
  This function develops the scrub memory functionality

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ScrubMemory (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if ECC_SCRUB == 1
  MMRC_STATUS            Status;

  if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
    return MMRC_SUCCESS;
  }
  //
  // Use CPGC to init memory.  Convert first to bytes, then to number of cache lines.
  //
  Status = MemInit (
              MrcData,
              Channel,
              MrcMemInit
            );

    MmrcExecuteTask (MrcData, InitializeEcc, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, EnableScrub, NO_PRINT, 0);

  return Status;
#else
  return MMRC_SUCCESS;
#endif // ECC_SCRUB == 1
}

/**
  This function does a memory testing

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
MemoryTest (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if MEMORY_TEST == 1
  MMRC_STATUS Status;

  if (MrcData->ExecuteThisRoutineInParallel) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Testing: %dMB\n", MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize));
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "CH%d Testing: %dMB\n", Channel, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TotalMem));
  }
  //
  // Run memory test on all ranks and halt if an error is observed.
  //
  Status = MemInit (
             MrcData,
             Channel,
             MrcMemTest
           );

  if (MrcData->HaltSystemOnMemTestError == MrcHaltSystemOnError && Status != MMRC_SUCCESS) {
    return MrcErrMemTest;
  }
  return Status;
#else
  return MMRC_SUCCESS;
#endif // MEMORY_TEST
}

/**
  Saves the MRC host structure to an MMX register for future use.

  @param[in, out]  MrcDataAddress  Address of MRC Host Structure.
  @retval          NONE
**/
VOID
SaveMrcHostStructureAddress (
  IN    MMRC_DATA   *MrcDataAddress
  )
{
#if SIM || JTAG
  MyMrcData = MrcDataAddress;
#else
#if __GNUC__
  asm (
    "movd %0,%%mm3;"
    :/* no output */
    : "c" (MrcDataAddress)
    :/* no clobber */
    );
#else
  _asm {
    movd mm3, MrcDataAddress;
  }
#endif
#endif
}

/**
  Returns the MRC host structure from an MMX register.

  @retval          MrcDataAddress  Address of MRC Host Structure.
**/
MMRC_DATA *
GetMrcHostStructureAddress (
  )
{
#if SIM || JTAG
  return MyMrcData;
#else
  MMRC_DATA *MrcData;
#if __GNUC__
  asm (
    "movd %%mm3,%0;"
    : "=c" (MrcData)
    :/* no input */
    :/* no clobber */
    );
#else
  _asm {
    movd MrcData, mm3;
  }
#endif
  return MrcData;
#endif
}

VOID
PrintMemoryErrorCode (
  IN        UINT8                 PostCode,
  IN        UINT8                 IndexError
  )
{
#if MRC_HUMAN_READABLE_ERRORS
  MmrcErrorPrint ((MMRC_DBG_MIN, "CP 0x%02X, Status[%d]: %s\n", PostCode, IndexError, (UINT8 *) &mErrorCode[IndexError][0]));
#else
  //
  // Human readable messages are disabled so just show the error index.
  //
  MmrcErrorPrint ((MMRC_DBG_MIN, "CP 0x%02X, Status[%d]\n", PostCode, IndexError));
#endif
  return;
}

VOID MrcDeadLoop (
  )
{
  VOLATILE UINT32  DeadLoopVar;
  MMRC_DATA       *DeadLoopMrcData;
#if PHYVIEW_TABLE
  UINT8            Channel;
#endif

  DeadLoopVar = 1;
  DeadLoopMrcData = GetMrcHostStructureAddress ();

#ifdef BXTPMRC
  // Switch from 1N back to 2N in CommandClockTraining if it hits failing condition
  if (DeadLoopMrcData->KeepAlive != 0) {
    DeadLoopMrcData->KeepAlive++;
    return;
  }
#endif

  //
  // First display all delays which is useful to see where training went wrong.
  //
#if PHYVIEW_TABLE
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (DeadLoopMrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      PhyViewTable (DeadLoopMrcData, 0, MMRC_NA, Channel);
    }
  }
#endif
  //
  // Now dump all registers in the spreadsheet. Note that for this to work,
  // DUMP_REGISTER_NAMES must be set to 1 in the spreadsheet.
  //
  DumpAllRegisters ();
  //
  // Handle error codes generically and with an OEM callout.
  //
  PrintMemoryErrorCode (DeadLoopMrcData->PostCode, (UINT8) DeadLoopMrcData->ErrorCode);
  OemMemoryErrorCode ((UINT8)DeadLoopMrcData->ErrorCode);

#if SIM
  //
  // In RTL simulation environment, call back to the test bench letting them
  // know we've hit a fatal error.
  //
  MyFatalMRCError();
#endif
  MmrcErrorPrint ((MMRC_DBG_MIN, "In a Dead Loop: Acting Dead!\n  "));
  while (DeadLoopVar);
}

#if TRAINING_ALGOS == 1
MMRC_STATUS
EarlyCommandPatternTest (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
  )
{
 #if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  UINT8   PatternIndex;
  UINT32  ExpectedResult;
  UINT32  SwizzleResult;
  UINT32  DataRise;
  UINT32  DataFall;
#if LPDDR3_SUPPORT
  UINT8   i;
#endif
  ExpectedResult = 0;
  SwizzleResult = 0;
  DataRise = 0;
  DataFall = 0;

  PassFail[Channel][0][0][0][CURRENT_SAMPLE] = RESULTS_PASS;
  //L
  // Loop through the patterns until a failure is detected or all patterns pass.
  //
  for (PatternIndex = 0; PatternIndex < NUMBER_EARLY_COMMAND_PATTERNS; PatternIndex++) {
    #if LPDDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
      MmrcWriteMRSCommand(MrcData, Channel, Rank, 1, FALSE, (EarlyCommandPatternsRise[PatternIndex] | (EarlyCommandPatternsFall[PatternIndex] << 10)));
#ifdef SIM
      ExpectedResult = EarlyCommandResults41[PatternIndex];
#else

      //  CA[8:5, 3:0] tCK rise to the  DRAM => DQCALRDAT[14, 12, 10, 8, 6, 4, 2, 0]
      // CA[8:5, 3:0] tCK fall  to the  DRAM => DQCALRDAT[15, 13, 11, 9, 7, 5, 3, 1]
      DataRise = (UINT8)((( (EarlyCommandPatternsRise[PatternIndex]& 0x1E0) >>5 ) << 4) | (EarlyCommandPatternsRise[PatternIndex] & 0xF));
      DataFall = (UINT8) ((( (EarlyCommandPatternsFall[PatternIndex]& 0x1E0) >>5 ) << 4) | (EarlyCommandPatternsFall[PatternIndex] & 0xF));
      ExpectedResult = 0;
      for (i = 0; i < 8; i++) {
        if ((DataRise & (1<<i)) != 0)
          ExpectedResult |= (1<< (i*2));
        if ((DataFall & (1<<i)) != 0)
          ExpectedResult |= (1<< ((i*2)+1));
      }
      //ExpectedResult =  DeSwizzle(MrcData, Channel, ExpectedResult);
      //ExpectedResult = EarlyCommandResults41[Channel][PatternIndex];
#endif
    }
    #endif //LPDDR3_SUPPORT
    #if LPDDR4_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
      //
      // Send pattern
      //
      //SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,
        (EarlyCommandPatterns1RiseLP4[PatternIndex] << 0 ) | (EarlyCommandPatterns1FallLP4[PatternIndex] <<  6) |
        (EarlyCommandPatterns2RiseLP4[PatternIndex] << 12) | (EarlyCommandPatterns2FallLP4[PatternIndex] << 18));
      #if SIM==1
        ExpectedResult = EarlyCommandResultsSIM[PatternIndex];
      #else
      //Dram returns from bit 8 to 13.
        //ExpectedResult = DeSwizzle(MrcData, Channel, EarlyCommandPatterns2RiseLP4[PatternIndex]<<8| EarlyCommandPatterns2RiseLP4[PatternIndex]<<24);//EarlyCommandResultsSIL[Channel][PatternIndex];
        ExpectedResult = (EarlyCommandPatterns2RiseLP4[PatternIndex]<<8| EarlyCommandPatterns2RiseLP4[PatternIndex]<<24);//EarlyCommandResultsSIL[Channel][PatternIndex];
        if (NUM_BITS_PER_DEVICE < 32) {
          ExpectedResult |= (ExpectedResult << 16);
        }
      #endif
    }
    #endif //LPDDR4_SUPPORT

    MySimStall(500);
    MrcDelay (MrcData, MICRO_DEL, 50);
    //
    // Get the resultant from the RDLATCH
    //
    MmrcExecuteTask (MrcData, DQCALRD, NO_PRINT, Channel);

    #if LPDDR4_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
      SwizzleResult = Swizzle (MrcData, Channel, (UINT32)MrcData->DynamicVars[Channel][REG_DATA]);
    }
#endif
    #if LPDDR3_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
      // Swizzle the result reading back. Only 16 bits are valid to compare.
      SwizzleResult = Swizzle (MrcData, Channel, (UINT32)MrcData->DynamicVars[Channel][REG_DATA]) &0xFFFF;
    }
#endif
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4){ //This condition will be true for BXT-P or E

      SwizzleResult &= LP4_ECT_PATTERN_MASK_by32;
      ExpectedResult &= LP4_ECT_PATTERN_MASK_by32;
    }

    // Compare the actual returned result to the expected result, if they differ
    // then return with failure.
    //

    if (NUM_BITS_PER_DEVICE < 32) //This condition will be true for BXT-E
    {
      SwizzleResult &= LP4_ECT_PATTERN_MASK_by16;
      ExpectedResult &= LP4_ECT_PATTERN_MASK_by16;
    }

    MmrcDebugPrint ((MMRC_DBG_MIN, "ECT: S%08x, E%08x\n  ", SwizzleResult, ExpectedResult));
    if (SwizzleResult != ExpectedResult) {
      PassFail[Channel][0][0][0][CURRENT_SAMPLE] = RESULTS_FAIL;
      return MMRC_SUCCESS;
    }
  } // for PatternIndex
#if LPDDR3_SUPPORT && !RVVP_ENABLE  //TODO: Fix RVVP swizzling temprary
  //
  // LPDDR3 needs to attemp the MR41 command to test the remaining bits.  Otherwise,
  // if LPDDR4 simply return at this point.
  //
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    return MMRC_SUCCESS; // TODO Temp workaround until investigation is done
  }
#endif // LPDDR3_SUPPORT
#endif// #if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  return MMRC_SUCCESS;
}
#endif

MMRC_STATUS
PullBackCommandClockControl (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
#if TRAINING_ALGOS==1 && PUSHOUT_CCC==1
  UINT8  Rank;
  UINT8  CccLoop;
  UINT32 CkTempValue[MAX_CLKS], CtlTempValue[MAX_CTLS], CmdTempValue[MAX_CMDS];
  UINT32 MinCCC = 0xffffffff;

  for (CccLoop = 0; CccLoop < MAX_CMDS; CccLoop++) {
    GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CmdGrp0 + CccLoop, CMD_GET_REG_UC, &CmdTempValue[CccLoop]);
  if (CmdTempValue[CccLoop] < MinCCC)
      MinCCC = CmdTempValue[CccLoop];
  GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CtlGrp0 + CccLoop, CMD_GET_REG_UC, &CtlTempValue[CccLoop]);
  if (CtlTempValue[CccLoop] < MinCCC)
      MinCCC = CtlTempValue[CccLoop];
    GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CkGrp0 + CccLoop, CMD_GET_REG_UC, &CkTempValue[CccLoop]);
  if (CkTempValue[CccLoop] < MinCCC)
      MinCCC = CkTempValue[CccLoop];
  } // for
  MinCCC = MinCCC / GetHalfClk(MrcData, Channel);
  MinCCC -= 2;
  MinCCC = MinCCC * GetHalfClk(MrcData, Channel);

  for (CccLoop = 0; CccLoop < MAX_CMDS; CccLoop++) {
    CmdTempValue[CccLoop] -= MinCCC;
    CtlTempValue[CccLoop] -= MinCCC;
    CkTempValue[CccLoop]  -= MinCCC;
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "Cache also updated\n  "));

  //
  // Loop on all ranks incrementing those ranks that are enabled.
  //
  for (Rank = 0 ; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //
    // Increment the Command knobs.
    //
    for (CccLoop = 0; CccLoop < MAX_CMDS; CccLoop++) {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CmdGrp0 + CccLoop, CMD_SET_VAL_FC_UC, &CmdTempValue[CccLoop]);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CtlGrp0 + CccLoop, CMD_SET_VAL_FC_UC, &CtlTempValue[CccLoop]);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CkGrp0 + CccLoop, CMD_SET_VAL_FC_UC, &CkTempValue[CccLoop]);
    } // for CccLoop
  } // for Rank
  MinCCC = MinCCC / GetHalfClk(MrcData, Channel);
  MinCCC +=1;
  GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CkeAll, CMD_SET_VAL_FC_UC, &MinCCC);
#endif // TRAINING_ALGOS==1 && PUSHOUT_CCC==1
  return MMRC_SUCCESS;
}

MMRC_STATUS
PushOutCommandClockControl (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
#if TRAINING_ALGOS==1 && PUSHOUT_CCC==1
  UINT8  Rank;
  UINT8  CccLoop;
  UINT32 TempValue, ClockTemp[MAX_CLKS], ControlTemp[MAX_CTLS], CommandTemp[MAX_CMDS];

  for (CccLoop = 0; CccLoop < MAX_CMDS; CccLoop++) {
    GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CmdGrp0 + CccLoop, CMD_GET_REG_UC, &TempValue);
    CommandTemp[CccLoop] = TempValue + PUSHOUT_VALUE*GetHalfClk(MrcData, Channel);
    GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CtlGrp0 + CccLoop, CMD_GET_REG_UC, &TempValue);
    ControlTemp[CccLoop] = TempValue + PUSHOUT_VALUE*GetHalfClk(MrcData, Channel);
    GetSetDdrioGroup2 (MrcData, Channel, 0, 0, CkGrp0 + CccLoop, CMD_GET_REG_UC, &TempValue);
    ClockTemp[CccLoop] = TempValue + PUSHOUT_VALUE*GetHalfClk(MrcData, Channel);
  } // for CccLoop

  //
  // Loop on all ranks incrementing those ranks that are enabled.
  //
  for (Rank = 0 ; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //
    // Increment the Command knobs.
    //
    for (CccLoop = 0; CccLoop < MAX_CMDS; CccLoop++) {
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CmdGrp0 + CccLoop, CMD_SET_VAL_FC_UC, &CommandTemp[CccLoop]);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CtlGrp0 + CccLoop, CMD_SET_VAL_FC_UC, &ControlTemp[CccLoop]);
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CkGrp0 + CccLoop, CMD_SET_VAL_FC_UC, &ClockTemp[CccLoop]);
    } // for CccLoop
  } // for Rank
#endif // TRAINING_ALGOS==1 && PUSHOUT_CCC==1
  return MMRC_SUCCESS;
}

MMRC_STATUS
Lp4CBTEntryAllRanks (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  UINT8 Rank;

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    //fsp1
    MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,  MRW_LP4(13, 0x70));
    MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,  MRW_LP4(11, 0x3 << 4)); //CAODT=120Ohms DQODT=disabled
    //fsp0
    MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,  MRW_LP4(13, 0x30));
    MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,  MRW_LP4(11, 0x3 << 4)); //CAODT=120Ohms DQODT=disabled
    MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,  MRW_LP4(13,1));
  }
#endif //EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  return MMRC_SUCCESS;
}

MMRC_STATUS
Lp4CBTExitAllRanks (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  UINT8 Rank;

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE,  MRW_LP4(13,0));
  }
#endif //EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
  return MMRC_SUCCESS;
}

/**
  Performs Early Command Training.   If the SEARCH_FOR_EARLY_CMD_PASS
  is set, the system will test the default value and verify that
  this value passes the test, if it fails, it will begin
  searching for a default value checking +/- 1/2 CLK. Otherwise,
  if the SEARCH_FOR_EARLY_CMD_PASS is not set, the system will
  "assume" the default value is passing and will search for the
  best value.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]   CapsuleStartIndex   Starting point within the CapsuleData structure
                                   where the Init should begin.
  @param[in]       StringIndex     When provided, this is the index into the string
                                   table to show the register names.
  @param[in]       Channel         Current Channel being examined.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
EarlyCommandTraining (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
#if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1

  UINT8  Rank;
  UINT32 TempValue;
  UINT8  Strobe;
  UINT16 HalfClock;
  UINT32 CmdCtrlMatrix[ECT_TILE_WIDTH][ECT_TILE_WIDTH];
  UINT32 ControlValue;
  UINT8  ControlCount;
  UINT32 CommandValue;
  UINT8  CommandCount;
  UINT32 CommandTotal;
  UINT32 ControlTotal;
  TS_LOC Retloc;
  UINT16 Low[MAX_CHANNELS][MAX_STROBES];
  UINT16 High[MAX_CHANNELS][MAX_STROBES];
  UINT16 Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT16 Dim1StartPoint[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT8  FirstPass;
  UINT8  RankCount;
  UINT32 InitialCommand;
  UINT32 InitialControl;
  UINT8  RangeMultiplier;

  HalfClock    = GetHalfClk (MrcData, Channel);
  ControlTotal = 0;
  CommandTotal = 0;
  RankCount    = 0;

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    RangeMultiplier = 1;
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    RangeMultiplier = 1;
  } else {
    RangeMultiplier = 1;
  }

  //
  // Loop through all ranks for the given channel.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    //
    // Only execute the training on ranks that enabled.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    //
    // Get the initial Command and Control values.
    //
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CtlAll, CMD_GET_REG_UC, &InitialControl);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CmdAll, CMD_GET_REG_UC, &InitialCommand);

    FirstPass    = FALSE;
    ControlCount = 0;

    MmrcDebugPrint ((MMRC_DBG_MIN, "Channel %d, Rank %d\n  ", Channel, Rank));
    //
    // Put the DRAM into CBT Mode for LPDDR3 only.
    //
    #if LPDDR3_SUPPORT
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        MmrcWriteMRSCommand(MrcData, Channel, Rank, 1, ECT_ENTERCBT_CPGC, MRW41_LP3);
      }
    #endif
    //
    // Put the phy into CBT Mode.
    //
    MrcData->DynamicVars[Channel][CAVREF] = DeSwizzle (MrcData, Channel, (ECT_LP4_VREF) | (ECT_LP4_VREF << 16));

    L_CpgcSetup (MrcData, Channel, CPGC_CONFIG_EARLY_CMD_TRAINING) ;

    MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_WRITE;
    MrcData->CpgcOptions.ChunkErrorMask = CPGC_CHUNK_ALL;
    MrcData->CpgcOptions.Rank     = Rank;
    L_CpgcReconfig (MrcData, Channel);
#ifdef ECT_ENTRY
    MmrcExecuteTask(MrcData, ECT_ENTRY, NO_PRINT, Channel);
    MmrcExecuteTask(MrcData, ECT_ENTRY_CCC, NO_PRINT, Channel&0xFFFE);
#endif

    #if LPDDR4_SUPPORT==1
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
      //
      // Read the initial 2x Value of the TX_DQS.
      //
      for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
        if (!IsStrobeValid (MrcData, Channel, 0, Strobe)) {
           continue;
        }
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_GET_REG_UC , &TempValue);
        TempValue = 8 * HalfClock;
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_SET_VAL_FC , &TempValue);
      }
      //
      // Run CPGC to put the DQ pattern and strobes for VREF.
      //
      PerformDQReset(MrcData, Channel, 0);
      PerformFifoReset(MrcData, Channel, 0);
      MmrcExecuteTask(MrcData, CPGC_ENABLE, NO_PRINT, Channel);
      //MmrcExecuteTask(MrcData, CPGC_RUN_TEST, NO_PRINT, Channel);
      L_CpgcRunTest (MrcData, Channel, (UINT16*) NULL, (UINT32*) NULL, (UINT32*) NULL, (UINT32*) NULL, (UINT32*) NULL);
      MySimStall(500);
      MrcDelay (MrcData, NANO_DEL, 100);
      for (Strobe=0; Strobe < MAX_STROBES; Strobe++) {
        if (!IsStrobeValid (MrcData, Channel, 0, Strobe)) {
          continue;
        }
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_GET_CACHE , &TempValue);
        GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, TxDqsDelay, CMD_SET_VAL_FC , &TempValue);
      }
    }
    #endif //LPDDR4_SUPPORT
    //
    // Drive CKE Low using DRMC register which will take over for CADB_ALWAYSON which is disabled by running stop test.
    //
    MrcData->DynamicVars[Channel][REG_DATA] = (Rank==0?2:1);
    MmrcExecuteTask(MrcData, DRIVE_CKE, NO_PRINT, Channel);
    //
    // Stop CPGC, which disabled ALWAYSON
    //
    #if LPDDR4_SUPPORT
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
      L_CpgcStopTest(MrcData, CapsuleStartIndex, StringIndex, Channel);
    }
    #endif
    //
    // Initialize the Output maxtrix as all failures.
    //
    MmrcMemset (CmdCtrlMatrix, 0x00, ECT_TILE_WIDTH * ECT_TILE_WIDTH * sizeof(UINT32));  // MmrcMemset (FinishBL, 0, MAX_CHANNELS * sizeof (UINT32));
    //
    // Initialize the parameters for the 1-D Sweep.
    //
    Low[Channel][0]                             = (UINT16) (InitialCommand - RangeMultiplier*GetHalfClk(MrcData, Channel));
    High[Channel][0]                            = (UINT16) (InitialCommand + RangeMultiplier*GetHalfClk(MrcData, Channel));
    Dim1StartPoint[Channel][0][0][LOW]          = (UINT16) InitialCommand - RangeMultiplier*GetHalfClk(MrcData, Channel);
    Dim1StartPoint[Channel][0][0][HIGH]         = (UINT16) InitialCommand + RangeMultiplier*GetHalfClk(MrcData, Channel);

    //
    // Loop through the range of Control sweeping from a multiple of 2X range.  This is dependent on LP3/LP4.
    //
    for (ControlValue = InitialControl - RangeMultiplier*GetHalfClk(MrcData, Channel), ControlCount=0; ControlValue <= InitialControl + RangeMultiplier*GetHalfClk(MrcData, Channel); ControlValue += 2*RangeMultiplier*GetHalfClk(MrcData, Channel)/(ECT_TILE_WIDTH-1)) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Control value %d\n", ControlValue));
      GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CtlAll, CMD_SET_VAL_FC_UC, &ControlValue);
      //
      // Perform the 1DSweep.
      //
      Create1DSweepLastPass (MrcData, Channel, Rank, CmdAll, 1, Dim1StartPoint, Low, High, (UINT16) (2*RangeMultiplier*GetHalfClk(MrcData, Channel))/(ECT_TILE_WIDTH-1),
          FALSE, FALSE, FALSE, 1, EarlyCommandPatternTest, Results, "S", 0);
      if (Results[Channel][0][0][0][LOW]==High[Channel][0]) {
        if (FirstPass == TRUE) {
          //
          // If no value passed, then set the starting points to the outer ends.
          //
          Dim1StartPoint[Channel][0][0][LOW]       = (UINT16) InitialCommand - RangeMultiplier*GetHalfClk(MrcData, Channel);
          Dim1StartPoint[Channel][0][0][HIGH]      = (UINT16) InitialCommand + RangeMultiplier*GetHalfClk(MrcData, Channel);
          //break;
        }
      } else {
        FirstPass = TRUE;
        Dim1StartPoint[Channel][0][0][LOW]       = (UINT16) Results[Channel][0][0][0][LOW];
        Dim1StartPoint[Channel][0][0][HIGH]      = (UINT16) Results[Channel][0][0][0][HIGH];
        for (CommandValue=InitialCommand-RangeMultiplier*GetHalfClk(MrcData, Channel), CommandCount=0; CommandValue <= InitialCommand + RangeMultiplier*GetHalfClk(MrcData, Channel); CommandValue+= RangeMultiplier*GetHalfClk(MrcData,Channel)/4, CommandCount++) {
          if (Results[Channel][0][0][0][LOW] > CommandValue) {
            CmdCtrlMatrix[ControlCount][CommandCount] = 0;
          } else if (Results[Channel][0][0][0][HIGH] >= CommandValue)  {
            CmdCtrlMatrix[ControlCount][CommandCount] = 1;
          } else {
            CmdCtrlMatrix[ControlCount][CommandCount] = 0;
          }
        }
      }
      ControlCount++;
    }
    //
    // Restore the default values for now.
    //
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CtlAll, CMD_SET_VAL_FC_UC, &InitialControl);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CmdAll, CMD_SET_VAL_FC_UC, &InitialCommand);

    //
    // Take the phy out of CBT Mode.
    //
    MrcData->DynamicVars[Channel][CAVREF] = 0x1f;
    MmrcExecuteTask (MrcData, EXIT_CA_TRAINING, NO_PRINT, Channel);
    MmrcExecuteTask (MrcData, EXIT_CA_TRAINING_CCC, NO_PRINT, Channel&0xFFFE);
    //
    // Take the DRAM out of CBT Mode.
    //
    #if LPDDR4_SUPPORT
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
        MmrcWriteMRSCommand(MrcData, Channel, Rank, 0, FALSE, MRW_LP4(13,0));
      }
    #endif
    #if LPDDR3_SUPPORT
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        MmrcWriteMRSCommand(MrcData, Channel, Rank, 1, FALSE, MRW42_LP3);
      }
    #endif
    Retloc = TileSearch(CmdCtrlMatrix, EarlyCommandWeights);
    MmrcDebugPrint ((MMRC_DBG_MIN, "Final solution Rank %d = %d, %d, ", Rank, Retloc.x, Retloc.y));
    ControlTotal += (UINT32) ((Retloc.y * (RangeMultiplier*GetHalfClk(MrcData,Channel)/4)) + (InitialControl - RangeMultiplier*GetHalfClk(MrcData, Channel)));
    CommandTotal += (UINT32) ((Retloc.x * (RangeMultiplier*GetHalfClk(MrcData,Channel)/4)) + (InitialCommand-RangeMultiplier*GetHalfClk(MrcData, Channel)));
    RankCount++;
  } // for Rank

  ControlTotal /= RankCount;
  CommandTotal /= RankCount;
  MmrcDebugPrint ((MMRC_DBG_MIN, "Final control is %d, Final command is %d\n", ControlTotal, CommandTotal));

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    //
    // Only execute  on ranks that are enabled.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] == 0) {
      continue;
    }
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CtlAll, CMD_SET_VAL_FC_UC, &ControlTotal);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, 0, CmdAll, CMD_SET_VAL_FC_UC, &CommandTotal);
  }

#endif

  return MMRC_SUCCESS;
}

#ifndef __GNUC__
#ifndef SUSSW
#ifdef DISABLE_GL_FLAG
void * __cdecl memcpy (void *dst, void *src, size_t cnt)
{
  UINT8 *dst8 = (UINT8 *)dst;
  UINT8 *src8 = (UINT8 *)src;
  while (cnt--) {
    *dst8++ = *src8++;
  }
  return dst;
}

void * __cdecl memset (void *dst, char value, size_t cnt)
{
  UINT8 *dst8 = (UINT8 *)dst;
  while (cnt--) {
    *dst8++ = value;
  }
  return dst;
}
#endif
#endif // SUSSW
#endif // __GNUC__

#ifndef SUSSW
#if USE_64_BIT_VARIABLES
#ifdef DISABLE_GL_FLAG
/**
  Performs a shift left on a 64 bit number.
  In non-64-bit modes and default 64-bit mode only bits 0 through 4 of
  the count are used. This masks the count to a value between 0 and 31.
  If a count is greater than the operand size, the result is undefined.
  For this reason, we must do a SHL for the upper 32 bits and
  another for the lower 32 bits.

  @param[in]   CL                 Shift amount (in bits).
  @param[in]   EDX:EAX            64 bit number to shift.
  @param[out]  EDX:EAX            64 bit result of shift left.
**/
void __cdecl _allshl (void)
{
#if __GNUC__
  asm (
    "cmp  $32, %%cl;"
    "jae  ShiftLowDword;"
    "shld %%cl, %%edx, %%eax;"
    "shl  %%cl, %%eax;"
    "ret;"
"ShiftLowDword:;"
    "mov  %%eax, %%edx;"
    "xor  %%eax, %%eax;"
    "sub  $32, %%cl;"
    "shl  %%cl, %%edx;"
    "ret;"
    : /* no output */
    : /* no input */
    : "%eax", "%edx"
  );
#else
_asm {
  cmp  cl, 32
  jae  ShiftLowDword    ; If shifting more than 31 bits, ignore the high DWORD since it be shifted out.
  shld edx, eax, cl
  shl  eax, cl
  ret
ShiftLowDword:
  mov  edx, eax
  xor  eax, eax
  sub  cl,  32          ; We already shifted the value by 32 bits by moving EAX into EDX. So just shift what is left over above 32 bits.
  shl  edx, cl
  ret
}
#endif
}
/**
  Performs a shift right on a 64 bit number.
  In non-64-bit modes and default 64-bit mode only bits 0 through 4 of
  the count are used. This masks the count to a value between 0 and 31.
  If a count is greater than the operand size, the result is undefined.
  For this reason, we must do a SHR for the upper 32 bits and another for
  the lower 32 bits.

  @param[in]   CL                 Shift amount (in bits).
  @param[in]   EDX:EAX            64 bit number to shift.
  @param[out]  EDX:EAX            64 bit result of shift right.
**/
void __cdecl _aullshr (void)
{
#if __GNUC__
  asm (
    "cmp  $32, %%cl;"
    "jae  ShiftHighDword;"
    "shrd %%cl, %%eax, %%edx;"
    "shr  %%cl, %%edx;"
    "ret;"
    "ShiftHighDword:;"
    "mov  %%edx, %%eax;"
    "xor  %%edx, %%edx;"
    "sub  $32, %%cl;"
    "shr  %%cl, %%eax;"
    "ret;"
    : /* no output */
  : /* no input */
  : "%eax", "%edx"
    );
#else
  _asm {
    cmp  cl, 32
    jae  ShiftHighDword   ; If shifting more than 31 bits, ignore the low DWORD since it be shifted out.
    shrd eax, edx, cl
    shr  edx, cl
    ret
ShiftHighDword:
    mov  eax, edx
    xor  edx, edx
    sub  cl,  32          ; We already shifted the value by 32 bits by moving EDX into EAX. So just shift what is left over above 32 bits.
    shr  eax, cl
    ret
  }
#endif
}
#endif
#endif // USE_64_BIT_VARIABLES
#endif // SUSSW

BOOLEAN
IsCmdSignalDelay (
  IN   MMRC_DATA    *MrcData,
  IN   UINT16       Delay
)
{
#if COMMAND_TRAINING==1 || EARLY_COMMAND_TRAINING==1
  UINT8 i;
  UINT8 TotalCommandGroups;
  //
  // Generalized condition in case other project uses different number of CMD groups
  //
  if (Delay >= GSM_GT_INDEX) {
    if (Delay == CmdGrp0 || Delay == CmdGrp1 || Delay == CmdGrp2 || Delay == CmdAll) {
      return TRUE;
    }
  } else {
    TotalCommandGroups = 0;
    for (i = 0; i < MAX_CMDS; i++) {
      TotalCommandGroups += SignalInfo[GetSignalInfoIndex (GetOffsetIndex (MrcData, MmrcCmdGrp0 + TotalCommandGroups))].FrequencyOffset;
    }
    if ((Delay >= MmrcCmdGrp0 && Delay < (MmrcCmdGrp0 + TotalCommandGroups)) || Delay == MmrcCmdAll) {
      return TRUE;
    }
  }
#endif
  return FALSE;
}

#if TRAINING_ALGOS
MMRC_STATUS
PrintFooter1DSweep (
  IN   MMRC_DATA    *MrcData,
  IN   UINT8        Channel,
  IN   UINT8        Rank,
  IN   UINT16       Dim1Index,
  IN   UINT8        Dim1Knobs,
  IN   UINT16       Results[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2]
)
{
  UINT8 Strobe;
  UINT8 Knob;
  UINT8 ChannelBackup;

  ChannelBackup = Channel;

  for (Channel = ChannelBackup; Channel < MAX_CHANNELS; Channel++) {
    if (!RunOnThisChannel (MrcData, Channel, Rank)) {
      continue;
    }
    if (!IsCmdSignalDelay (MrcData, Dim1Index)) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Ch%02d  ", Channel));
      for (Knob = 0; Knob < Dim1Knobs; Knob++) {
        for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
          if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
            continue;
          }
          if (Results[Channel][Knob][Strobe][0][HIGH] >= Results[Channel][Knob][Strobe][0][LOW]) {
            MmrcDebugPrint ((MMRC_DBG_MIN, "%04d:%04d:P  ",
              Results[Channel][Knob][Strobe][0][LOW],
              Results[Channel][Knob][Strobe][0][HIGH]
              ));
          } else {
            MmrcDebugPrint ((MMRC_DBG_MIN, "%04d?%04d:F  ",
              Results[Channel][Knob][Strobe][0][LOW],
              Results[Channel][Knob][Strobe][0][HIGH]
              ));
          }
        } // Strobe loop ...
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
      if (!MrcData->ExecuteThisRoutineInParallel) {
        //
        // Stop the routine if this routine is
        // not supposed to executed in parallel
        //
        break;
      }
    }
  } // Channel loop ...
  return MMRC_SUCCESS;
}
#endif

/**
  Returns the index into the SignalInfo array which matches the signal index
  into the Elements array.

  @param[in]       Signal         Index into the Elements array.
  @retval                         SignalInfo Index
**/
UINT16
GetSignalInfoIndex (
  UINT16        Signal
  )
{
  MMRC_DATA *MrcData;
  MrcData = GetMrcHostStructureAddress ();

  if (Signal >= SGTI_INDEX) {
    return Signal - SGTI_INDEX; // Peter: Changed from Signal - SGTI_INDEX -ALGO_REG_INDEX, because the Signal value could be less than (SGTI_INDEX + ALGO_REG_INDEX)
  } else if (Signal >= ALGO_REG_INDEX) {
    return Signal - ALGO_REG_INDEX;
  } else if (Signal < (NumberAlgos * NumberOfElementsPerAlgo)) {
    return Signal / NumberOfElementsPerAlgo;
  } else {
    return Signal - (NumberAlgos * NumberOfElementsPerAlgo) + NumberAlgos;
  }
}

/**
  Converts Type

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Type            Register to be accessed, this is pointer to
                                   a register or algorithm in
                                   the Elements[] table.
  @retval          Type
**/
UINT16
GetOffsetIndex (
  IN          MMRC_DATA   *MrcData,
  IN          UINT16       Type
  )
{
  //
  // Check if this type is defined by the GSM_GT or GSM_CSN enum. If it is, we need to convert it to the internal
  // MMRC define for that type.
  //
  if (Type >= GSM_CSN_INDEX) {
    Type = MMRC_NA;
  } else if (Type >= GSM_GT_INDEX) {
    Type = GsmGtToMmrcDefine[Type - GSM_GT_INDEX];
  }
  if (Type >= REG_INDEX && Type < GROUP_INDEX) {
    Type -= REG_INDEX;
    if (Type < MAX_NUM_ALGOS) {
      Type = DelayIndexToOffsetLayer [Type].DelayOffset;
    }
  } else if (Type >= GROUP_INDEX && Type < UNIQUE_INDEX) {
    Type -= GROUP_INDEX;
    if (Type < MAX_NUM_GROUP) {
      Type = GroupIndexToOffsetLayer [Type].DelayOffset;
    }
  } else if (Type >= UNIQUE_INDEX) {
    Type -= UNIQUE_INDEX;
    Type = UniqueIndexToOffsetLayer [Type].DelayOffset;
  }
  return Type;
}

/**
  Finds the blueprint that corresponds to the current
  configuration.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       IoLevel
  @param[out]      Blueprint
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ConvertPfctAndIoLevelToBlueprint (
  IN        MMRC_DATA  *MrcData,
  IN        UINT8       Channel,
  IN        UINT8       IoLevel,
     OUT    UINT8      *Blueprint
  )
{
  UINT8 i;
  i = 0;
  //
  // Loop through the table and find the blueprint that corresponds to the current configuration.
  // If the IoLevel is a don't care then just return the blueprint which matches the PFCT. This is
  // useful in systems which have 1 level of memory per channel.
  //
  for (i = 0; i < MAX_IO_LEVELS; i++) {
    if ((((1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentPlatform)      & PfctAndIoLevelToBlueprint[i].PMask) != 0) &&
        (((1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentFrequency)     & PfctAndIoLevelToBlueprint[i].FMask) != 0) &&
        (((1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentConfiguration) & PfctAndIoLevelToBlueprint[i].CMask) != 0) &&
        (((1 << MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType)       & PfctAndIoLevelToBlueprint[i].TMask) != 0) &&
        ((IoLevel == PfctAndIoLevelToBlueprint[i].IoLevel) || (IoLevel == 0xFF))
       )
    {
      *Blueprint = PfctAndIoLevelToBlueprint[i].Blueprint;
      return MMRC_SUCCESS;
    }
  }
  return STATUS_FAIL;//MrcIoLevelNotSupported;
}

/**
  This function tells if a Channel should be skipped/executed
  when looping through Channels.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @retval          TRUE            Run in this channel
  @retval          TRUE            Skip this channel
**/
BOOLEAN
RunOnThisChannel (
  IN        MMRC_DATA  *MrcData,
  IN        UINT8       Channel,
  IN        UINT8       Rank
  )
{
#if ENABLE_RUN_TEST
  MrcData->DynamicVars[Channel][ENABLE_RUN_TEST] = (UINTX) (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] &
                                                            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel &
                                                            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled);
  if (MrcData->DynamicVars[Channel][ENABLE_RUN_TEST] == 0) {
#else
  UINT32 X;
  X = (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[Rank] &
                                                            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ExecuteOnThisChannel &
                                                            MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled);
  if (X == 0) {
#endif
    return FALSE;
  }
  return TRUE;
}

#if EARLY_READ_TRAINING
/**
  Toggle the MPR mode for an specific rank

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       Channel         Current Channel being examined.
  @param[in]       Rank            Current Rank being examined.
  @param[in]       Toggle          Value for the MRS 3 command.
  @retval          NONE
**/
VOID
ToggleMprMode (
  IN  OUT   MMRC_DATA       *MrcData,
  IN        UINT8           Channel,
  IN        UINT8           Rank,
  IN        UINT8           Toggle
  )
{
  #if DDR4_SUPPORT | DDR3_SUPPORT
  DramInitDDR3EMR3    Mrs3Command;

  Mrs3Command.Data = 0x0;
  #endif

#if LPDDR4_SUPPORT
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    if (Toggle == 1) {
      //
      // Issue the MRW32.
      //
      MrcData->DynamicVars[Channel][RANK]        = Rank;
      MrcData->DynamicVars[Channel][RDDCMD]      = 0;              // Setup for MRW
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 0;             // LP4 requires Burst 8.
      MrcData->DynamicVars[Channel][REG_DATA]    = MRW_LP4(32, 0x55);     // MRR32
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      //
      // Issue the MRW40.
      //
      MrcData->DynamicVars[Channel][REG_DATA]    = MRW_LP4(40, 0x55);     // MRW40
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      //
      // Issue the MRW15.
      //
      MrcData->DynamicVars[Channel][REG_DATA]    = MRW_LP4(15, 0x00);     // MRW15
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      //
      // Issue the MRW20.
      //
      MrcData->DynamicVars[Channel][REG_DATA]    = MRW_LP4(20, 0x00);     // MRW20
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);

      MrcData->DynamicVars[Channel][REG_DATA]    = MPC06_LP4;             // MPC6
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    }
  }
#endif //LPDDR4_SUPPORT

#if DDR4_SUPPORT | DDR3_SUPPORT
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    if (Toggle == 1) {
      //
      // Enable MPR mode
      //
       MrcData->DynamicVars[Channel][REG_DATA]    = 0x30004;             // MPR mode
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      MrcDelay (MrcData, MICRO_DEL, 10);
      MySimStall(500);
    } else {
       MrcData->DynamicVars[Channel][REG_DATA]    = 0x30000;             // MPR mode
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      MrcDelay (MrcData, MICRO_DEL, 10);
      MySimStall(500);
    }
  }
#endif //DDR4_SUPPORT | DDR3_SUPPORT
}
#endif // EARLY_READ_TRAINING


#if TRAINING_ALGOS == 1 && EARLY_READ_TRAINING == 1
#if LPDDR3_SUPPORT == 1 | LPDDR4_SUPPORT == 1
MMRC_STATUS
EarlyReadTest (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
)
{
  UINT16 Errors;
  UINT8 Strobe;
  Errors = 0;
#if LPDDR3_SUPPORT == 1   //need to add rank for dram cmd
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    //
    // Perform a Fifo Reset to align pointers.
    //
    PerformFifoReset(MrcData, Channel, 0);
    //
    // Setup the Sequence for MRR32 command.
    //
    MrcData->DynamicVars[Channel][REG_DATA] = 0x55555555;
    MmrcExecuteTask(MrcData, CPGC_SETUP_DPAT0, NO_PRINT, Channel);
    //
    // Start CPGC Pattern Checker.
    //
    MmrcExecuteTask(MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
    //
    // Issue the MRR32.
    //
    MrcData->DynamicVars[Channel][RANK]        = Rank;
    MrcData->DynamicVars[Channel][REG_DATA]    = MRW32_LP3;     // MRR32
    MrcData->DynamicVars[Channel][RDDCMD]      = 1;             // ThisL is a MRR command.
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;             // LP3 requires Burst 8.
    SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    //
    // Stop the Test and Check for Errors.
    //
    MySimStall(100);
    MmrcExecuteTask (MrcData, CPGC_CHECK_ERRORS, NO_PRINT, Channel);
    Errors = (UINT16) MrcData->DynamicVars[Channel][REG_DATA];

    //
    // If there were no errors from the MRR32, then continue with the MRR40.
    if (Errors != 0xF) {
      //
      // Perform a Fifo Reset to align pointers.
      //
      PerformFifoReset(MrcData, Channel, 0);
      //
      // Setup the Sequence for MRR40 command.
      //
      MrcData->DynamicVars[Channel][REG_DATA] = 0xCCCCCCCC;
      MmrcExecuteTask(MrcData, CPGC_SETUP_DPAT0, NO_PRINT, Channel);
      //
      // Start CPGC Pattern Checker.
      //
      MmrcExecuteTask(MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
      //
      // Issue the MRR40
      //
      MrcData->DynamicVars[Channel][RANK]        = Rank;
      MrcData->DynamicVars[Channel][REG_DATA]    = MRW40_LP3;     // MRR40
      MrcData->DynamicVars[Channel][RDDCMD]      = 1;             // This is a MRR command.
      MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;             // LP3 requires Burst 8.
      SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
      //
      // Stop the Test and Check for Errors.
      //
      MySimStall(500);
      MmrcExecuteTask (MrcData, CPGC_CHECK_ERRORS, NO_PRINT, Channel);
      Errors = (UINT8) MrcData->DynamicVars[Channel][REG_DATA];
    }

    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if ((Errors & (1<<Strobe)) == 0) {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_PASS;
      } else {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_FAIL;
      }
    }
  }
#endif // LPDDR3_SUPPORT

#if LPDDR4_SUPPORT==1
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
  //
  // Perform a Fifo Reset to align pointers.
  //
    PerformFifoReset(MrcData, Channel, 0);
    //
    // Setup the Sequence for Patterns for each DQ command.
    //
    PerformDQReset(MrcData, Channel, 0);
    MrcData->DynamicVars[Channel][REG_DATA] = 0x5555;
    MmrcExecuteTask(MrcData, CPGC_SETUP_DPAT0, NO_PRINT, Channel);
    //
    // Start CPGC Pattern Checker.
    //
    MmrcExecuteTask(MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
    //
    // Issue the MPC-1 command
    //
    MrcData->DynamicVars[Channel][RANK]        = Rank;
    MrcData->DynamicVars[Channel][REG_DATA]    =   MPC(0, READ_DQ_CALIBRATION);     // MRW20
    MrcData->DynamicVars[Channel][RDDCMD]      = 1;          // This is a MRR command.
    SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    //
    // Stop the Test and Check for Errors.
    //
    MySimStall(500);
    MmrcExecuteTask (MrcData, CPGC_CHECK_ERRORS, NO_PRINT, Channel);
    Errors = (UINT16) MrcData->DynamicVars[Channel][REG_DATA];
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if ((Errors & (1<<Strobe)) == 0) {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_PASS;
      } else {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_FAIL;
      }
    }

  }
#endif // LPDDR4_SUPPORT==1

#if DDR3_SUPPORT==1
  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
  //
  // Perform a Fifo Reset to align pointers.
  //
    PerformFifoReset(MrcData, Channel, 0);
    //
    // Setup the Sequence for Patterns for each DQ command.
    //
    PerformDQReset(MrcData, Channel, 0);
    MrcData->DynamicVars[Channel][REG_DATA] = 0xaaaaaaaa;
    MmrcExecuteTask(MrcData, CPGC_SETUP_DPAT0, NO_PRINT, Channel);
    //
    // Start CPGC Pattern Checker.
    //
    MmrcExecuteTask(MrcData, CPGC_RUN_DIRECT, NO_PRINT, Channel);
    //
    // Issue the Read Command and stall for a short time.
    //
    MrcData->DynamicVars[Channel][REG_DATA] = 0x280000;
    MrcData->DynamicVars[Channel][RDDCMD] = 1;
    MrcData->DynamicVars[Channel][RANK] = Rank;
    MrcData->DynamicVars[Channel][DRAM_CMD_BL] = 1;
    SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    MySimStall(500);MySimStall(500);
    MrcDelay (MrcData, MICRO_DEL, 5);
    //
    // Stop the Test and Check for Errors.
    //
    MmrcExecuteTask (MrcData, CPGC_CHECK_ERRORS, NO_PRINT, Channel);
    Errors = (UINT16) MrcData->DynamicVars[Channel][REG_DATA];
    Errors |=  (UINT16)((MrcData->DynamicVars[Channel][ECC_ERR_STAT] & 0x1) << 8);
    MmrcDebugPrint ((MMRC_DBG_MIN,"%08x", (UINT32) MrcData->DynamicVars[Channel][ECC_ERR_STAT]));
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
      if (!IsStrobeValid (MrcData, Channel, Rank, Strobe)) {
        continue;
      }
      if ((Errors & (1<<Strobe)) == 0) {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_PASS;
      } else {
        PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_FAIL;
      }
    }

  }
#endif // DDR3_SUPPORT==1
  return MMRC_SUCCESS;
}
#endif // LPDDR3_SUPPORT==1 | LPDDR4_SUPPORT==1
#endif // TRAINING_ALGOS

#if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
TS_LOC
TileSearch(UINT32 XinTable[ECT_TSWIDTH][ECT_TSWIDTH], UINT8 Weights[3][3])
{
  INT8 Xoffset, Yoffset;
  UINT32 OutTable[ECT_TSWIDTH][ECT_TSWIDTH];
  UINT32 InTable[ECT_TSWIDTH][ECT_TSWIDTH];
  UINT32  HighestValue=0;
  UINT16 HighestValueCount=0xFF, LastHighestValueCount = 0;
  UINT16 EntryCount=0xFF, LastEntryCount=0;
  TS_LOC Center;
  INT8 x, y;
  UINT8 Xrange, Yrange; //Locations from the actual table
  UINT8 loopcount=0;
  UINT32 SheddingThreshold;

  Center.x = 0;
  Center.y = 0;
  Mmrcmemcpy(InTable, XinTable, sizeof(UINT32) * ECT_TSWIDTH * ECT_TSWIDTH);
  PrintTSTable(InTable);
  while ((HighestValueCount != 1) &&
         ((HighestValueCount != LastHighestValueCount) || (EntryCount != LastEntryCount)))
  {
    loopcount++;
    HighestValue = 0;
    LastHighestValueCount = HighestValueCount;
    LastEntryCount = EntryCount;
    HighestValueCount = 0;
    EntryCount = 0;
    //
    //  Main Loop - Compute Next Table
    //
    for (Yrange = 0; Yrange <= ECT_TSWIDTH-1; Yrange ++) //Iterate through the InTable
    {
      for (Xrange = 0; Xrange <= ECT_TSWIDTH-1; Xrange ++)
      {
        OutTable[Yrange][Xrange]=0;
        for (Yoffset = -1; Yoffset <= 1; Yoffset++)  //Iterate through the weights table
        {
          y = Yrange + Yoffset;
          //
          //  If the calculated y is outside the array, it cannot be used
          //
          if ((y < 0) || (y >= ECT_TSWIDTH))
            continue;
          for (Xoffset = -1; Xoffset <= 1; Xoffset ++)
          {
            //
            //  If the calculated x is outside the array, it cannot be used
            //
            x = Xrange + Xoffset;
            if ((x < 0) || (x >= ECT_TSWIDTH))
              continue;

            OutTable[Yrange][Xrange] += InTable[y][x] * Weights[1+Yoffset][1+Xoffset];
            //
            //  Keep track of the highest value and count
            //
            if (OutTable[Yrange][Xrange] > HighestValue)
            {
              HighestValue = OutTable[Yrange][Xrange];
              HighestValueCount = 1;
              Center.x = Xrange;
              Center.y = Yrange;
            }
            else if (OutTable[Yrange][Xrange] == HighestValue)
            {
              HighestValueCount++;
            }
          }
        }
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MAX,"\r\nLoop %d\r\n", loopcount));
    PrintTSTable(OutTable);
    //
    //  Perform Shedding
    //
    SheddingThreshold = ((HighestValue * ECT_TS_SHEDDING_PERCENT) /100);
    for (Yrange = 0; Yrange <= ECT_TSWIDTH-1; Yrange ++) //Iterate through the InTable
    {
      for (Xrange = 0; Xrange <= ECT_TSWIDTH-1; Xrange ++)
      {
        //
        //  remove all values below the SheddingThreshold
        //
        if (OutTable[Yrange][Xrange] <= SheddingThreshold)
          OutTable[Yrange][Xrange] = 0;
        else
          EntryCount++;
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MAX,"\r\nShed - Threshold = %d\r\n", SheddingThreshold));
    MmrcDebugPrint ((MMRC_DBG_MAX,"\r\nEntries after Shedding = %d\r\n", EntryCount));
    PrintTSTable(OutTable);
    if (HighestValue == 0)
    {
      MmrcDebugPrint ((MMRC_DBG_MIN,"ERROR: Highest Value found is a 0\r\n", ECT_TS_VAL_LIMIT, HighestValue));
      return Center;  //while(1);
    }
    //Rescale all values
    for (Yrange = 0; Yrange <= ECT_TSWIDTH-1; Yrange ++) //Iterate through the intable
    {
      for (Xrange = 0; Xrange <= ECT_TSWIDTH-1; Xrange ++)
      {
        //
        //  Rescale all the Values
        //
        OutTable[Yrange][Xrange] = (OutTable[Yrange][Xrange] * ECT_TS_VAL_LIMIT)/HighestValue;
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MAX,"\r\nScale Factor = %d/%d\r\n", ECT_TS_VAL_LIMIT, HighestValue));
    PrintTSTable(OutTable);
    Mmrcmemcpy(InTable, OutTable, sizeof(unsigned int) * ECT_TSWIDTH * ECT_TSWIDTH);
  }
  MmrcDebugPrint ((MMRC_DBG_MAX,"\r\n"));
  //PrintX(XinTable, Center);
  return Center;
}
#endif // EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1

#if EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1
void
PrintTSTable (
  IN    UINT32        table[ECT_TSWIDTH][ECT_TSWIDTH]
)
{
  INT8 xrange, yrange; //Locations from the actual table
  for (yrange = -1; yrange <= ECT_TSWIDTH-1; yrange ++)
  {
    for (xrange = -1; xrange <= ECT_TSWIDTH-1; xrange ++)
    {
      if (xrange == -1 && yrange == -1) //Top Left leave it blank
        MmrcDebugPrint ((MMRC_DBG_MAX,"      "));
      else if (yrange == -1) //Header
        MmrcDebugPrint ((MMRC_DBG_MAX,"%5d ", xrange));
      else if (xrange == -1) //Row number
        MmrcDebugPrint ((MMRC_DBG_MAX,"%5d ", yrange));
      else
      {
        if (table[yrange][xrange] ==0)
          MmrcDebugPrint ((MMRC_DBG_MAX,"      "));
        else
          MmrcDebugPrint ((MMRC_DBG_MAX,"%d,", table[yrange][xrange]));
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MAX,"\r\n"));
  }
}
#endif // EARLY_COMMAND_TRAINING == 1 && TRAINING_ALGOS == 1

MMRC_STATUS
MmrcWriteMRSCommand (
  IN  MMRC_DATA     *MrcData,
  IN  UINT8         Channel,
  IN  UINT8         Rank,
  IN  UINT8         Bl,
  IN  UINT8         UseCPGC,
  IN  UINT32        Data
) {
  MrcData->DynamicVars[Channel][RDDCMD]      = 0;
  MrcData->DynamicVars[Channel][DRAM_CMD_BL] = Bl;
  MrcData->DynamicVars[Channel][RANK]        = Rank;
  if (UseCPGC == 1) {
    MrcData->DynamicVars[Channel][ROW_COL_ADDR] = Data;
    MmrcExecuteTask (MrcData, CPGC_MRS, NO_PRINT, Channel);
    MrcDelay(MrcData,MICRO_DEL,1);
    MySimStall(500);
    MmrcExecuteTask (MrcData, CPGC_EXIT_MRS, NO_PRINT, Channel);
  } else {
    MrcData->DynamicVars[Channel][REG_DATA] = Data;
    SendDramCmd(MrcData, Channel); //MmrcExecuteTask (MrcData, SEND_DRAM_CMD, NO_PRINT, Channel);
    MySimStall(750);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
GenericByteMemoryTest (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
  )
{
  UINT8         Strobe;               // Strobe Index for currently active.
  UINT16        ByteErrorStatus0;
  UINT32        ErrorCount[9];
  UINT32        BitErrorStatus[2];

  PerformFifoReset(MrcData, Channel, Rank);

  MrcData->CpgcOptions.ChunkErrorMask = 0xFF;
  L_CpgcRunTest (MrcData, Channel, &ByteErrorStatus0, &BitErrorStatus[0], &BitErrorStatus[1], (UINT32*)NULL, ErrorCount);

  for (Strobe = 0; Strobe <  MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
    if (ByteErrorStatus0 & (1<<Strobe)) {
      PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_FAIL;
    } else {
      PassFail[Channel][0][Strobe][0][CURRENT_SAMPLE] = RESULTS_PASS;
    }
  } // for Strobe

  return MMRC_SUCCESS;
}

#if (WRITE_TRAINING == 1 || READ_TRAINING == 1 || COMMAND_TRAINING == 1) && TRAINING_ALGOS == 1
/**
  This function is the center for the Advanced Training Algos.  Currently there are 5 possible elements to loop on:
  Channel, Rank, Knob, Strobe, and Bit.  The first 5 loops on each of these elements and if they are not shared
  uses that index and goes to the next element.  If they are shared, the loop goes to the next element but will not loop
  to the next element such that only those elements that are independent will be looping on...
    For example, RANK_SHARED, BIT_SHARED   Ch[0->MAX_CHANNELS],Rn[0],Knob[0->MAX_KNOBS], Sl[0->MAX_STROBES],Bit[0]
  At the innermost loop (bit), all permutations of the non-shared [CH][RN][KN][SL][BI] will be hit.
  The system will not do another 5 loops, looping through each of these elements again but this time doing the opposite.
  Namely, hitting all the shared items... the outermost loops use IndChn, IndRnk,.. inner uses ShrChn, ShrRnk, etc...
  So at the innermost loop now, each shared elements of the unique elements can be checked.

  A couple of features can be accomplished by this:
    1.  Composite, the system can check the ranges of the passed in data and collect the worst range of all.
    2.  Composite count, the system can check the counts if not max, set to 0....
    3.  Test Function can be called with/without the compositing passing in the unique Ch/Rn/Kn/Sl/Bi.
  The test function removes the requirement to have many functions performing the same looping as required by this function.

  @param[in]  *MrcData        Host structure for all data related to MMRC.
  @param[in]  SharedFlag      Flag specifying how the current index is to be combined.
  @param[in]  NumberElements  For CMD training =1, all other, equal to number of strobes active.
  @param[in]  CompositeFlag   Upon looping on each unique item, Results/Count are composited based on the flag.
  @param[in]  NumberCounts    Number of count array elements, needed for printing, (number eyemasks)
  @param[out] Count           Count of number passing Vrefs for each eyemask.
  @param[out] Results         Single 1-D Sweep from Create1DSweep() function.
  @param[in]  Param           Local parameter being passed to test function call.
  @param[in]  TestFunction    Function to call at the end of all the compositing.
**/
void
BuildComposite (
  IN MMRC_DATA *MrcData,
  IN UINT8 SharedFlag,
  IN UINT8 NumberElements,
  IN UINT8 CompositeFlag,
  IN UINT8 NumberCounts,
  IN UINT8 MaxCount,
  IN UINT8 Count[][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS],
  IN UINT16 Results[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  IN BuildCompositeParams *Param,
  IN MMRC_STATUS TestFunction (MMRC_DATA *, UINT16 *, UINT8, UINT8, UINT8, UINT8, UINT8, UINT8, BuildCompositeParams*)
) {
  UINT8  IndChn;
  UINT8  ShrChn;
  UINT8  ShrChnRange[2];
  UINT8  IndRnk;
  UINT8  ShrRnk;
  UINT8  ShrRnkRange[2];
  UINT8  IndKnb;
  UINT8  ShrKnb;
  UINT8  ShrKnbRange[2];
  UINT8  IndStb;
  UINT8  ShrStb;
  UINT8  ShrStbRange[2];
  UINT8  IndBit;
  UINT8  ShrBit;
  UINT8  ShrBitRange[2];
  UINT8  IndTiming;
  UINT8  ShrTiming;
  UINT8  ShrTimingRange[2];

  UINT16 Edge[7];
  UINT8  Loop;
  UINT8  CountLoop;
  UINT16 CompositeCount[5];
  UINT16 OverwriteFlag; // 0=no overwrite, 1=overwrite
  UINT8  FirstActiveChannelFound;
  UINT8 MaxStrobe;

  FirstActiveChannelFound=FALSE;
  MaxStrobe = MrcData->MaxNumberStrobes;

  //
  // Loop on all active channels and ranks that are independent.
  //
  for (IndChn = 0; IndChn < MrcData->MaxNumberChannels; IndChn++) {
    if (((SharedFlag & CHANNEL_SHARED) == CHANNEL_SHARED)) {
      if (FirstActiveChannelFound==TRUE) { continue; }
      ShrChnRange[0] = 0;
      ShrChnRange[1] = MrcData->MaxNumberChannels;
    } else {
      ShrChnRange[0] = IndChn;
      ShrChnRange[1] = IndChn + 1;
    }
    for (IndRnk = 0 ; IndRnk < MAX_RANKS; IndRnk++) {
      if (!RunOnThisChannel (MrcData, IndChn, IndRnk)) continue;
      FirstActiveChannelFound = TRUE;
      if (((SharedFlag & RANK_SHARED) == RANK_SHARED)) {
        if (IndRnk > 0) { continue; }
        ShrRnkRange[0] = 0;
        ShrRnkRange[1] = MAX_RANKS;
      } else {
        ShrRnkRange[0] = IndRnk;
        ShrRnkRange[1] = IndRnk + 1;
      }
        for (IndTiming = 0 ; IndTiming < MrcData->NumberOfTiming ; IndTiming++) {
        if ((SharedFlag & TIMING_SHARED) == TIMING_SHARED) {
          if (IndTiming > 0) { continue; }
          ShrTimingRange[0] = 0;
          ShrTimingRange[1] = MrcData->NumberOfTiming;
        } else {
          ShrTimingRange[0] = IndTiming;
          ShrTimingRange[1] = IndTiming + 1;
        }
        for (IndKnb = 0 ; IndKnb < MAX_KNOBS; IndKnb++) {
          if (((SharedFlag & KNOB_SHARED) == KNOB_SHARED)) {
            if (IndKnb > 0) { continue; }
            ShrKnbRange[0] = 0;
            ShrKnbRange[1] = MAX_KNOBS;
          } else {
            ShrKnbRange[0] = IndKnb;
            ShrKnbRange[1] = IndKnb + 1;
          }
          for (IndStb = 0 ; IndStb < NumberElements; IndStb++) {
            if (((SharedFlag & STROBE_SHARED) == STROBE_SHARED)) {
              if (IndStb > 0) { continue; }
              ShrStbRange[0] = 0;
              ShrStbRange[1] = NumberElements;
            } else {
              ShrStbRange[0] = IndStb;
              ShrStbRange[1] = IndStb + 1;
            }
            for (IndBit = 0 ; IndBit < MrcData->MaxBits; IndBit++) {
              if (((SharedFlag & BIT_SHARED) == BIT_SHARED)) {
                if (IndBit > 0) { continue; }
                ShrBitRange[0] = 0;
                ShrBitRange[1] = MrcData->MaxBits;
              } else {
                ShrBitRange[0] = IndBit;
                ShrBitRange[1] = IndBit + 1;
              }

            //
            // At this point, we have the looping occurring on all independent controls, and the ranges needed for the shared knobs.
            //
            Edge[0]  = 0x0000;
            Edge[1] = 0xffff;
            //
            // Check if there is reason to loop on all SHARED elements.
            //
            if ((CompositeFlag & 3) != 0){
              for (CountLoop=0; CountLoop < NumberCounts; CountLoop++) {
                CompositeCount[CountLoop] = 0xffff;
              }
              OverwriteFlag = (CompositeFlag & 4)<<2;
              for (Loop=0; Loop < OverwriteFlag+1; Loop++) {
                for (ShrChn = ShrChnRange[0]; ShrChn < ShrChnRange[1]; ShrChn++) {
                  for (ShrRnk = ShrRnkRange[0]; ShrRnk < ShrRnkRange[1]; ShrRnk++) {
                    if (!RunOnThisChannel (MrcData, ShrChn, ShrRnk)) continue;
                    for (ShrTiming = ShrTimingRange[0]; ShrTiming < ShrTimingRange[1]; ShrTiming++) {
                      for (ShrKnb = ShrKnbRange[0]; ShrKnb < ShrKnbRange[1]; ShrKnb++) {
                        for (ShrStb = ShrStbRange[0]; ShrStb < ShrStbRange[1]; ShrStb++) {
                          for (ShrBit = ShrBitRange[0]; ShrBit < ShrBitRange[1]; ShrBit++) {
                            //
                            // This is the innermost loop, this area is looped again by another variable called loop.
                            // when loop=0, its the first time, and the system must check for the results or count.
                            // when loop=1, the system will apply those recorded results.  THereby overwriting the initial values.
                            if (Loop == 0) {
                              //
                              // Check if need to composite on Results.
                              //
                              if ((CompositeFlag & COMPOSITE_RESULTS) == COMPOSITE_RESULTS)  {
                                if (Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][LOW] <=  Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][HIGH]) {
                                  if (Edge[0] < Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][LOW])  {
                                    Edge[0] =Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][LOW];
                                  }
                                  if (Edge[1] > Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][HIGH]) {
                                    Edge[1] = Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][HIGH];
                                  }
                                }
                              }
                              //
                              // Check if need to composite on Count.
                              //
                              if ((CompositeFlag & COMPOSITE_COUNT) == COMPOSITE_COUNT) {
                                for (CountLoop=0; CountLoop < NumberCounts; CountLoop++) {
                                  if (Count[CountLoop][ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit] < CompositeCount[CountLoop]) {
                                    CompositeCount[CountLoop] = Count[CountLoop][ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit];
                                    Edge[2+CountLoop] = CompositeCount[CountLoop];
                                  }
                                }
                              }
                            } else {
                              //
                              // If  overwriting and the COMPOSITE_RESULTS is set to true, then apply the composite
                              // results to each of the shared elements thus overwriting the ranges with the composite.
                              //
                              if ((CompositeFlag & COMPOSITE_RESULTS) == COMPOSITE_RESULTS)  {
                                Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][LOW] = Edge[0];
                                Results[ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit][HIGH] = Edge[1];
                              }
                              //
                              // If overwriting and the COMPOSITE_COUNT is set to true, then apply the count to each of the
                              // shared elements.  The principle here is if the composite is less then the passed in maximum count
                              // the clear the count; otherwise, store the count.
                              if ((CompositeFlag & COMPOSITE_COUNT) == COMPOSITE_COUNT) {
                                for  (CountLoop=0; CountLoop < NumberCounts; CountLoop++) {
                                  if (CompositeCount[CountLoop] < MaxCount) {
                                    CompositeCount[CountLoop] = 0;
                                  }
                                  Count[CountLoop][ShrTiming][ShrRnk][ShrKnb][(MaxStrobe * ShrChn)+ ShrStb][ShrBit] = (UINT8) CompositeCount[CountLoop];
                                }
                              }
                            }  // if loop
                          } // for ShrTiming
                        } // for ShrBit
                      }  // for ShrStb
                    }  // for ShrKnb
                  }  // for ShrRnk
                }  // for ShrChn
              } // for OverwriteFlag
            } // if CompositeFlag
            //
            // Call the test function passing in the info structure and the low/high data.
            // When CompositeResults, Edge[0][1] are the high/low composites.
            // When CompositeCount, Edge[2] is the results.
            //
            if (TestFunction != (MMRC_STATUS (*)(MMRC_DATA *, UINT16 *, UINT8, UINT8, UINT8, UINT8, UINT8, UINT8, BuildCompositeParams*)) -1) {
              TestFunction(MrcData, Edge, IndChn, IndRnk, IndKnb, IndStb, IndBit, IndTiming, Param);
            }
            } // for IndTiming
          } // for IndBit
        } // for IndStb
      } // for IndKnb
    } // for IndRnk
  } // for IndChn
}

/**
  The recursive function for the determine passing vrefs that will be called for each control
  knob outside of timing and vref.  Each knob will loop over a range given a starting point and
  will start at the starting point going to the max, then returning to the starting point and going
  to the min.  The control knob also has a step size to reach those endpoints.  The function will
  run across its knob and for each internal loop will either process the vref/timing or call itself
  with the next knob until finally the vref/timing can be tested.
  The only method this function can be completed finished is if either all the knobs have hit their
  maximum value or the first eyemask has been satisfied.

  @param[in]  *MrcData        Host structure for all data related to MMRC.
  @param[in]  *EyeMask        Pointer to the various eyemasks.
  @param[in]  ControlKnobs    Control parameters for each dimension.
  @param[in]  ControlIndex    Index into the control knobs for current dimension.
  @param[in]  UnknownLastPass Flag specifying if current sweep can use lastpass results.
  @param[in]  Dim1StartPoint  Length for both strings
  @param[in]  Low             Length for both strings
  @param[in]  High            Length for both strings
  @param[out] ResultsV        Single Eyemask of consecutive sweeps.
  @param[out] ResultsS        Temporary remaining Eyemask sweeps.
  @param[out] Results         Single 1-D Sweep from Create1DSweep() function.
  @param[out] Count           Count of number passing Vrefs for each eyemask.
  @param[out] PowerKnobs      Passing powerknobs for each eyemask except first.
  @param[out] AllDoneFlag     TRUE/FALSE flag specifying if EyeMask0 has reached its completion.
**/
MMRC_STATUS
DeterminePassingVref1 (
  IN  MMRC_DATA   *MrcData,
  IN  EYEMASK_T    EyeMask[],
  IN  CONTROLKNOBS ControlKnobs[],
  IN  UINT8        ControlIndex,
  IN  UINT8        UnknownLastPass,
  IN  UINT16       Dim1StartPoint[MAX_TIMING][MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2],
  IN  UINT16       Low[MAX_TIMING][MAX_CHANNELS][MAX_STROBES],
  IN  UINT16       High[MAX_TIMING][MAX_CHANNELS][MAX_STROBES],
  OUT UINT16       ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  OUT UINT16       ResultsS[MAX_EYEMASKS-1][MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  OUT UINT16       Results[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  OUT UINT8        Count[MAX_EYEMASKS][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS],
  OUT UINT8        PowerKnobs[MAX_POWER_KNOBS][MAX_EYEMASKS-1][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS],
  OUT UINT8        *AllDoneFlag
) {
  //
  // Local Variables
  //
  UINT16 ControlValue;
  BuildCompositeParams Param;

  //
  // If control's index is set to the timing delay (index=0), then run the sweep; otherwise,
  // loop through the valid range re-calling the function with the next index.
  //
  if (ControlIndex > (ControlKnobs[0].NumberOfTiming - 1)) {
    //
    // When beginning the loop, it needs to be known that the lastpass values are invalid.
    //
    UnknownLastPass = TRUE;
    //
    // Loop on the Control knobs starting at the initial value, incrementing by STEP, and going to the HIGH point.
    //
    for (ControlValue = ControlKnobs[ControlIndex].StartValue[LOW]; ControlValue <= ControlKnobs[ControlIndex].Ranges[HIGH]; ControlValue += ControlKnobs[ControlIndex].CriteriaStep) {
      //
      // Set the Controls value to the specified value. Set the criteria based on the first eyemask, the remaining eyemasks are used for checking, but
      // controls should be continued until eyemask[0] has met all its requirements.
      //
      Param.SetCriteria2.ControlIndex = ControlIndex;
      Param.SetCriteria2.ControlValue = ControlValue;
      Param.SetCriteria2.ControlKnobs = ControlKnobs;
      Param.SetCriteria2.EyeMask = EyeMask;

      BuildComposite(MrcData, ControlKnobs[ControlIndex/*ControlIndex*/].SharedFlags, MrcData->MaxElements, COMPOSITE_COUNT, 1, (UINT8) EyeMask[0].NumberConsecutiveVoltages,  Count, 0, &Param, SetCriteria2);
      //
      // Re-Call this function but for the next control index.
      //
      DeterminePassingVref1(MrcData, EyeMask, ControlKnobs, ControlIndex - 1, UnknownLastPass, Dim1StartPoint, Low, High, ResultsV, ResultsS, Results, Count, PowerKnobs, AllDoneFlag);

   //   for (TimingLoop = 0; TimingLoop < ControlKnobs[0].NumberOfTiming; TimingLoop++) {
        if (ControlIndex == ControlKnobs[0].NumberOfTiming) {
          //
          // Composite the Bit results into Byte if PBD is disabled
          //
          if (ControlKnobs[0].PBSharedFlags == -1)
          {
            BuildComposite(MrcData, BIT_SHARED, MrcData->MaxElements, COMPOSITE_RESULTS | OVERWRITE, 2, (UINT8) 1,  0, Results, 0, (MMRC_STATUS (*) (MMRC_DATA *, UINT16 *, UINT8, UINT8, UINT8, UINT8, UINT8, UINT8, BuildCompositeParams*))-1);
          }
          //
          // Store all passing eyemasks criteria into the global elements.
          // This will increment count if the result passes the eyemask criteria and the criteria is not already met.
          //
          // for (TimingLoop = 0; TimingLoop < ControlKnobs[0].NumberOfTiming; TimingLoop++) {
          CheckAndStoreEyeMasks(MrcData, ControlKnobs, Results, ResultsV, ResultsS, &Count[0], PowerKnobs, EyeMask);
          // }
          // UINT8 1 is for checking if any of the values are 0, then zero out all of the composites.... 1 says 1,2,3,4.. will stay.
          // Each timing knobs need to have the same share flags
          BuildComposite(MrcData, ControlKnobs[/*0*/ControlIndex].SharedFlags, MrcData->MaxElements, COMPOSITE_COUNT|OVERWRITE, 2, (UINT8) 1, &Count[0], 0, 0, (MMRC_STATUS (*) (MMRC_DATA *, UINT16 *, UINT8, UINT8, UINT8, UINT8, UINT8, UINT8, BuildCompositeParams*))-1);

          //
          // print the results
          //
          {
            Param.PrintCount.AllDoneFlag = 1;
            Param.PrintCount.ControlIndex = ControlIndex;
            Param.PrintCount.ControlKnobs = ControlKnobs;
            Param.PrintCount.EyeMaskIndex = 0;
            Param.PrintCount.EyeMask = EyeMask;
            BuildComposite(MrcData, ControlKnobs[0/*ControlIndex*/].SharedFlags, MrcData->MaxElements, COMPOSITE_RESULTS | COMPOSITE_COUNT, 2, (UINT8) EyeMask[0].NumberConsecutiveVoltages,  Count, Results, &Param, PrintCount);
            *AllDoneFlag = Param.PrintCount.AllDoneFlag;
            if (Param.PrintCount.AllDoneFlag == 1) {
              return MMRC_SUCCESS;
            }
          }
        } else {
          //
          // Build a composite of the count for the purpose of resetting those counts that have not reached the top.
          //
          if (*AllDoneFlag == 1) {
            return MMRC_SUCCESS;
          }
          BuildComposite(MrcData, ControlKnobs[ControlIndex].SharedFlags, MrcData->MaxElements, COMPOSITE_COUNT|OVERWRITE, 2, (UINT8) EyeMask[0].NumberConsecutiveVoltages, &Count[0], 0, 0, CheckCount);
          *AllDoneFlag = 1;
        }
      //} //TimingLoop
    }
    for (ControlValue = ControlKnobs[ControlIndex].StartValue[0] - ControlKnobs[ControlIndex].Step[0]; (INT8) ControlValue > (INT8) ControlKnobs[ControlIndex].Ranges[LOW]; ControlValue -= ControlKnobs[ControlIndex].Step[0]) {
      //
      // Set the Controls value to the specified value.
      //
      Param.SetCriteria2.ControlIndex = ControlIndex;
      Param.SetCriteria2.ControlValue = ControlValue;
      Param.SetCriteria2.ControlKnobs = ControlKnobs;
      Param.SetCriteria2.EyeMask = EyeMask;

      BuildComposite(MrcData, ControlKnobs[ControlIndex].SharedFlags, MrcData->MaxElements, COMPOSITE_COUNT, 1, (UINT8) EyeMask[0].NumberConsecutiveVoltages, &Count[0], 0, &Param, SetCriteria2);
      //
      // Re-Call this function but for the next control index.
      //
      DeterminePassingVref1(MrcData, EyeMask, ControlKnobs, ControlIndex - 1, UnknownLastPass, Dim1StartPoint, Low, High, ResultsV, ResultsS, Results, Count, PowerKnobs, AllDoneFlag);

        if (ControlIndex == ControlKnobs[0].NumberOfTiming) {
          //
          // Store all passing eyemasks criteria into the global elements.
          // This will increment count if the result passes the eyemask criteria and the criteria is not already met.
          //
          //for (TimingLoop = 0; TimingLoop < ControlKnobs[0].NumberOfTiming; TimingLoop++) {
            CheckAndStoreEyeMasks(MrcData, ControlKnobs, Results, ResultsV, ResultsS, Count, PowerKnobs, EyeMask);
          //}
        } else {
          //
          // Build a composite of the count for the purpose of resetting those counts that have not reached the top.
          //
          BuildComposite(MrcData, ControlKnobs[ControlIndex].SharedFlags, MrcData->MaxElements, COMPOSITE_COUNT|OVERWRITE, 1, (UINT8) EyeMask[0].NumberConsecutiveVoltages, &Count[0], 0, 0, CheckCount);
        }
      }
    ///} //TimingLoop
  } else {

    Param.StoreRestore.ControlIndex[0] = ControlKnobs[0].Index;
    Param.StoreRestore.ControlIndex[1] = ControlKnobs[1].Index;
    Param.StoreRestore.NumberTimings = ControlKnobs[0].NumberOfTiming;
    BuildComposite(MrcData, ControlKnobs[ControlIndex-1].SharedFlags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, 0, &Param, StoreDelay);
    if (ControlKnobs[0].NumberOfTiming > 1) {
      //ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[1], Low[0], High[0], Dim1StartPoint[0], 0);
      ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[ControlIndex-1], Low[ControlIndex-1], High[ControlIndex-1], Dim1StartPoint[ControlIndex-1], 0);
      CompleteSweep(MrcData, ControlKnobs, ControlIndex-1, High[ControlIndex-1], Low[ControlIndex-1], UnknownLastPass, Dim1StartPoint[ControlIndex-1], Results[ControlIndex-1]);
      BuildComposite(MrcData, ControlKnobs[ControlIndex-1].SharedFlags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, 0, &Param, RestoreDelay);
      // Restore the value back
    }
    //ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[0], Low[0], High[0], Dim1StartPoint[0], 0);
    ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[ControlIndex], Low[ControlIndex], High[ControlIndex], Dim1StartPoint[ControlIndex], 0);
    CompleteSweep(MrcData, ControlKnobs, ControlIndex, High[ControlIndex], Low[ControlIndex], UnknownLastPass, Dim1StartPoint[ControlIndex], Results[ControlIndex]);
    BuildComposite(MrcData, ControlKnobs[ControlIndex].SharedFlags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, 0, &Param, RestoreDelay);

  } // if ControlIndex == 0
  return TRUE;
}

/**
  This function takes the input data and decodes the linear value.  Any number below 0xfa is a linear number
  and the value is simply returned, 0xfa-0xff is a function of the half clock.  The half clock is read on a
  per-channel basis and that is why channel is passed in.  0xff=1x, 0xfe=0x2x, 0xfd=4x, 0xfc=8x, 0xfb=16x, and 0xfa=32x.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Channel       Current channel being operated on.
  @param[out]      InputData     Input data from ControlKnobs that may be encoded.
  @retval          Data          Returns valid 16-bit value.
**/
UINT16
ConvertRange (
  IN MMRC_DATA *MrcData,
  IN UINT8 Channel,
  IN UINT16 InputData
) {
  UINT16 Data;
//_1X   0xFF
//_18X  0xFD
//_14X  0xFC
//_38X  0xFB
//_12X  0xFA
//_58X  0xF9
//_34X  0xF8
//_n1X   0xF6
//_n18X  0xF5
//_n14X  0xF4
//_n38X  0xF3
//_n12X  0xF2
//_n58X  0xF1
//_n34X  0xF0
  // all other values are integers.
  switch (InputData) {
    case 0xff:
      //
      // Full Clock
      //
      Data = 2*GetHalfClk(MrcData, Channel);
      break;
    case 0xfd:
      //
      // 1/8 Clock.
      //
      Data = GetHalfClk(MrcData, Channel)/4;
      break;
    case 0xfc:
      //
      // 1/4 Clock.
      //
      Data = GetHalfClk(MrcData, Channel)/2;
      break;
    case 0xfb:
      //
      // 3/8 Clock.
      //
      Data = 3*GetHalfClk(MrcData, Channel)/4;
      break;
    case 0xfa:
      //
      // 1/2 Clock.
      //
      Data = GetHalfClk(MrcData, Channel);
      break;
    case 0xf9:
      //
      // 5/8 Clock.
      //
     Data = 5*GetHalfClk(MrcData, Channel)/4;
     break;
    case 0xf8:
      //
      // 3/4 Clock.
      //
     Data = 3*GetHalfClk(MrcData, Channel)/2;
     break;
    case 0xf6:
      //
      // -1 Clock.
      //
     Data = -2*GetHalfClk(MrcData, Channel);
     break;
    case 0xf5:
      //
      // -1/8 Clock.
      //
     Data = -1*GetHalfClk(MrcData, Channel)/4;
     break;
    case 0xf4:
      //
      // -1/4 Clock.
      //
     Data = -1*GetHalfClk(MrcData, Channel)/2;
     break;
    case 0xf3:
      //
      // -3/8 Clock.
      //
     Data = -3*GetHalfClk(MrcData, Channel)/4;
     break;
    case 0xf2:
      //
      // -1/2 Clock.
      //
     Data = -1*GetHalfClk(MrcData, Channel);
     break;
    case 0xf1:
      //
      // -5/8 Clock.
      //
      Data = -5*GetHalfClk(MrcData, Channel)/4;
      break;
    case 0xf0:
      //
      // -3/4 Clock
      //
      Data = -3 * GetHalfClk(MrcData, Channel) / 2;
      break;
    default:
      //
      // Return the input data.
      //
      Data = InputData;
  }
  return Data;
}

/**
  When preparing for the 1DSweep, this function takes the information from the
  specific control knob and sets up the initial  Low/High/Startpoint needed
  for passing to the 1DSweep function.  The Low/High are the physical range for
  the 1dsweep where the Dim1StartPoint is the initial position.  The low/high does
  not get changed, but the startpoint will get modified after each 1DSweep to provide
  the lastpass features.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ControlKnobs    External array of controls for the algorithm.
  @param[out]      Low             Low range for 1DSweep control.
  @param[out]      High            High range for 1DSweep control.
  @param[out]      Dim1StartPoint  Starting position for 1DSweep control.
  @param[in]       Index           Specifies how the input parameters are used to fill in the output.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ProgramInitialSettingsFor1DSweep (
  MMRC_DATA      *MrcData,
  CONTROLKNOBS   ControlKnobs,
  UINT16          Low[MAX_CHANNELS][MAX_STROBES],
  UINT16          High[MAX_CHANNELS][MAX_STROBES],
  UINT16          Dim1StartPoint[MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2],
  UINT8           Index
) {
  UINT8  Channel;           // Channel assignement variable.
  UINT8  Rank;              // Rank assignment variable.
  UINT8  Strobe;            // Strobe assignment variable.
  UINT8  Knob;              // Knob assignment variable.
  UINT32 TempValue;         // 32-bit temporary variable used for GetSets.
  UINT16 Ranges[2];         // 16-bit range hi/low variable.
  UINT16 StartValue[2];     // 16-bit start hi/low variables.

  //
  // The passed in index parameter, if = 0, states that the range and startvalues within
  // the control knobs are to be used to specfy the equivalent range and startvalue output;
  // however, if =1, then the phase2 parameters are used to specify the range and start values.
  //
  if (Index < ControlKnobs.NumberOfTiming) {
    Ranges[LOW] = ControlKnobs.Ranges[LOW];
    Ranges[HIGH] = ControlKnobs.Ranges[HIGH];
    StartValue[LOW] = ControlKnobs.StartValue[LOW];
    StartValue[HIGH] = ControlKnobs.StartValue[HIGH];
  } else {
    Ranges[LOW] = ControlKnobs.Ranges[LOW];
    Ranges[HIGH] = ControlKnobs.Ranges[HIGH];
    StartValue[LOW] = ControlKnobs.Phase2_VrefStart;
    StartValue[HIGH] = ControlKnobs.Phase2_VrefStop;
  }
  //
  // Loop through all active channels and ranks setting the Low/high and startpoints
  // for the tests.
  //
  for (Channel=0; Channel < MAX_CHANNELS; Channel++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) continue;
      for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
        //
        // If the RelAbsFlag is REL, then read the current index value and the ranges low/high are differences
        // to the read value as are the start low/high values.
        //
        if (ControlKnobs.RelAbsFlag == REL) {
          GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, ControlKnobs.Index, CMD_GET_CACHE, &TempValue);
          if (Rank==0 || (((UINT16)(TempValue + ConvertRange(MrcData, Channel, Ranges[LOW]))) < Low[Channel][Strobe])) {
            Low[Channel][Strobe]                       = (UINT16) TempValue +(UINT16) ConvertRange(MrcData, Channel, Ranges[LOW]);
          }
          if (Rank==0 || (((UINT16)(TempValue + ConvertRange(MrcData, Channel, Ranges[HIGH]))) > High[Channel][Strobe])) {
            High[Channel][Strobe]                      = (UINT16) TempValue + (UINT16) ConvertRange(MrcData, Channel, Ranges[HIGH]);
          }
          for (Knob=0; Knob < MAX_KNOBS; Knob++) {
            Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW] = (INT16)TempValue +(INT16)ConvertRange(MrcData, Channel, StartValue[LOW]);
            Dim1StartPoint[Rank][Channel][Knob][Strobe][HIGH]= (INT16)TempValue + (INT16)ConvertRange(MrcData, Channel, StartValue[HIGH]);
          } // For kn
        //
        // If the RelAbsFlag is ABS, then the ranges low/high and start low/high are absolute settings and they are
        // just assigned the specified values.
        //
        } else {
          Low[Channel][Strobe]                       = (UINT16) ConvertRange(MrcData, Channel, Ranges[LOW]);
          High[Channel][Strobe]                      = (UINT16) ConvertRange(MrcData, Channel, Ranges[HIGH]);
          for (Knob=0; Knob < MAX_KNOBS; Knob++) {
            Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW] = (INT16)ConvertRange(MrcData, Channel, StartValue[LOW]);
            Dim1StartPoint[Rank][Channel][Knob][Strobe][HIGH]= (INT16)ConvertRange(MrcData, Channel, StartValue[HIGH]);
          }  // for Knob
        } // If ControlKnobs
      } // for Strobe
    } // for Rank
  } // for Channel

  return MMRC_SUCCESS;
}
/**
  This is the top function for the DeterminePassingVref step.  This function declares the variables that are needed throughout
  the lifespan of this step as well as initializes the variables needed.  The largest seconday array is the ResultsVTemp structure
  which holds the passing vrefs for the non-primary eye masks.  Rather then making this in the top function, it is put here because
  at the time this step is completed, the only results needed is the passed in ResultsV which has all the necessary data.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       ControlKnobs  Control parameters for each dimension.
  @param[in]       ControlIndex  Index of current control knob.
  @param[out]      ResultsV      Single Eyemask of consecutive sweeps.
  @param[in]       EyeMask       Eyemasks used for this algorithm.
  @param[in]       NumberEyeMask Total number of eyemasks.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
DeterminePassingVref2 (
  IN MMRC_DATA *MrcData,
  IN CONTROLKNOBS ControlKnobs[],
  IN UINT8 ControlIndex,
  IN OUT UINT16 ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  IN EYEMASK_T *EyeMask,
  IN UINT8 NumberEyeMask
)
{
  //
  // Local Variables.
  //
  UINT16   ResultsVTemp[MAX_EYEMASKS-1][MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2];// Temporary Storage locations for remaining Eyemasks.
  UINT16   Results[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2];                                 // Location of current results to be stored.
  UINT8    Count[MAX_EYEMASKS][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS];                        // Current consecutive count stored in results.
  UINT8    PowerKnobs[MAX_POWER_KNOBS][MAX_EYEMASKS-1][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];             // Power knob settings for passing eyemasks.
  UINT16   Low[MAX_TIMING][MAX_CHANNELS][MAX_STROBES];                                                                         // Lowest value allowed on the sweep used in recursive function.
  UINT16   High[MAX_TIMING][MAX_CHANNELS][MAX_STROBES];                                                                        // Highest value allowed on the sweep used in recursive function.
  UINT16   Dim1StartPoint[MAX_TIMING][MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];                                    // Starting point on where to begin the sweep.
  UINT8    AllDoneFlag;                                                                                                        // Flag stating if all tests have passed.
  UINT8    loop;                                                                                                               // Internal looping variable.
  BuildCompositeParams Param;                                                                                                  // Parameters for the LoopOnComposite function.

  UINT8  NumberOfTiming;
  UINT8  VrefIndex;

  if (ControlKnobs[0].NumberOfTiming > 1) {
    NumberOfTiming = 2;
    VrefIndex = 2;
  } else {
    NumberOfTiming = 1;
    VrefIndex = 1;
  }

  //
  // The upper and lower limits of the sweep gets done one time at the beginning of the algo, and does not get
  // modified throughout the entire algorithm. These are the Low/High ranges and are used in the 1DSweep.
  //
  for (loop = 0; loop< NumberOfTiming; loop++) {
   ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[loop], Low[loop], High[loop], Dim1StartPoint[loop], 0);
  }
  //
  // When starting the determination, the consecutive counts for each test should be set to 0.
  //
  MmrcMemset(Count, 0, MAX_EYEMASKS*MAX_TIMING*MAX_RANKS*MAX_KNOBS*MAX_CHANNELS_TIMES_STROBES*MAX_BITS);
  MmrcMemset(PowerKnobs, -1, MAX_POWER_KNOBS*(MAX_EYEMASKS-1)*MAX_CHANNELS*MAX_RANKS*MAX_KNOBS*MAX_STROBES*MAX_BITS);
  AllDoneFlag = 1;
  //
  // Print out the header
  //
  {
    UINT8 i;
    UINT8 i2;
    UINT8 count;
    MmrcDebugPrint ((MMRC_DBG_MIN, "\nCRT  "));
    count = 3;
    if ((ControlKnobs[0].SharedFlags & STROBE_SHARED) == STROBE_SHARED) {
      for (i = ControlIndex; i > 0 ; i--) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "%s:", ControlKnobs[i].Label));
        count+=4;
      }
      MmrcDebugPrint ((MMRC_DBG_MIN, "HGH:LOW:EM  "));
      count+=12;
    } else {
      for (i2=0; i2 < MrcData->MaxElements; i2++) {
         for (i = ControlIndex; i > 0 ; i--) {
           MmrcDebugPrint ((MMRC_DBG_MIN, "%s:", ControlKnobs[i].Label));
           count+=4;
         }
         MmrcDebugPrint ((MMRC_DBG_MIN, "HGH:LOW:EM  "));
         count+=12;
      }
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
    while (count-- > 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "="));
    }
    //MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
  }
  //
  // Begin calling the recursive function that will perform the Late Training.  Note that the results for the
  // eyemasks are stored in two different arrays, ResultsV and ResultsS.  ResultsS is a temporary storage
  // for the N-1 eyemasks and is removed at the completion of this function.  The ResultsV is passed in and gets
  // the actual data for the accepted EyeMask..  This is not removed and is returned to the calling function.
  //
  DeterminePassingVref1 (MrcData, EyeMask, ControlKnobs, ControlIndex, 0, Dim1StartPoint, Low, High, ResultsV, ResultsVTemp, Results, Count, PowerKnobs, &AllDoneFlag);
  //
  // For each granularity of the power knob, set the desired value.
  //
  for (loop=NumberOfTiming+1; loop < ControlKnobs[0].NumberKnobs; loop++) {
    Param.GetSet.ControlKnobs = ControlKnobs;
    Param.GetSet.ControlIndex = loop;
    Param.GetSet.PowerKnobs = PowerKnobs;
    Param.GetSet.EyeMask = EyeMask;
    BuildComposite(MrcData, ControlKnobs[loop].SharedFlags|KNOB_SHARED, MrcData->MaxElements, COMPOSITE_COUNT, 3, 0, Count, 0, &Param, GetSet);
  }
  //
  // Copy the correct results data based on the count data to the ResultsV. This is the only data returned from this function
  //
  Param.CopyResults.ResultsV = ResultsV;
  Param.CopyResults.ResultsS = ResultsVTemp;
  Param.CopyResults.Count = Count;
  Param.CopyResults.MaxCount = (UINT8) EyeMask[0].NumberConsecutiveVoltages;
  BuildComposite(MrcData, NO_SHARED, MrcData->MaxElements, 0, 0, 0, 0, 0, &Param, CopyResults);
  //
  // Return with success.
  //
  return MMRC_SUCCESS;
}

MMRC_STATUS
CheckCount (MMRC_DATA *MrcData, UINT16 *Range, UINT8 Ch, UINT8 Rn, UINT8 Kn, UINT8 St, UINT8 Bi, UINT8 Timing, BuildCompositeParams *Param) {

  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function. At the end of the Determine Passing Vrefs,
  there are two arrays ResultsS and ResultsV.  ResultsV has one EyeMask of data, and ResultsS has the
  remaining.  The ResultsS will be removed.  ResultsV has only EyeMask[0]'s responses, but before leaving
  those eyemasks that are passing the test needed to be copied to ResultsV.  Thereby, at the end the resultsV
  could have different EyeMasks passing different elements.  The system will check if eyemask0 passed this
  instance, just return, otherwise find the passing one and copy it from ResultsS to ResultsV.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Knob being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CopyResults (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  UINT8 index;
  UINT8 index2;
  UINT8 MaxStrobe;

  ////TODO: Add a check for maximum size of Channel and Strobe
  MaxStrobe = MrcData->MaxNumberStrobes;

  //
  // If the current count has already met the requirements, do not copy it over, just return as we are done.
  //
  if (Param->CopyResults.Count[0][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit] == Param->CopyResults.MaxCount) return MMRC_SUCCESS;
  //
  // Loop through each of the secondary eye masks, and if they pass their input copy them over. But if they have
  // also reached there max count simply exit out.
  //
  for (index = 1; index < RT_NUM_EYEMASKS; index++) {
    if (Param->CopyResults.Count[index][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit] == Param->CopyResults.MaxCount) {
      //
      // Copy all of the Vrefs/Pis (MaxCount) from the temporary to the final results.
      for (index2=0; index2<Param->CopyResults.MaxCount; index2++) {
        Param->CopyResults.ResultsV[index2][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] = Param->CopyResults.ResultsS[index-1][index2][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH];
        Param->CopyResults.ResultsV[index2][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW] = Param->CopyResults.ResultsS[index-1][index2][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW];
      }
      return MMRC_SUCCESS;
    }
  }
  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function. This function upon completion of the vref
  passing criteria search, restores the power knobs to the setting that brought about the passing.
  These values are printed out at the end of the DetermineVref() function.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Knob being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GetSet (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  UINT32 value;
  UINT8 index;
  UINT8 *Label;
  UINT8 PowerKnob;
  UINT8 NumberKnobs;
  UINT8 EyeMaskMet;
  UINT8 BitLoop;
  UINT8 bit;

  EyeMaskMet = 0;
  value = 0;
  //
  // Extract the parameters from the Param flags.
  //
  Label = Param->GetSet.ControlKnobs[Param->GetSet.ControlIndex].Label;
  NumberKnobs = Param->GetSet.ControlKnobs[Param->GetSet.ControlIndex].NumberKnobs;
  //
  // New line on each bit/strobes = 0.  followed by the channel, rank, knob printout.
  //
  if (Bit==0 && Strobe==0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d  ", Channel, Rank, Knob));
  }
  //
  // Loop through each EyeMask, determining which eyemask passed the criteria. Always start on the first.
  //
  for (index = 0; index < RT_NUM_EYEMASKS; index++) {
    if (Range[2+index] == Param->GetSet.EyeMask[0].NumberConsecutiveVoltages) {
      EyeMaskMet = 1;
      //
      // Once a passing criteria has been met, restore the power knobs to their recorded value.
      // Note that the first knob does not require new settings as the test would have stopped immediately
      // upon passing.
      //
      for (PowerKnob=NumberKnobs; PowerKnob >= 2; PowerKnob--) {

        //
        // Print out the power knob value.
        //
        if (index == 0) {
          GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Param->PrintCount.ControlKnobs[PowerKnob].Index, CMD_GET_REG, &value); //Change to CMD_GET_CACHE
        } else {
          value = Param->GetSet.PowerKnobs[PowerKnob-2][index-1][Channel][Rank][Knob][Strobe][Bit];
        }
        MmrcDebugPrint ((MMRC_DBG_MIN, "%02d:", value));
        if ((Param->PrintCount.ControlKnobs[PowerKnob].SharedFlags & (BIT_SHARED|BIT_CONTROL)) == (BIT_SHARED | BIT_CONTROL)) {
          BitLoop=MAX_BITS;
        } else {
          BitLoop = Bit + 1;
        }
        for (bit = Bit; bit < BitLoop; bit++) {
          GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Param->PrintCount.ControlKnobs[PowerKnob].Index, CMD_SET_VAL_FC_UC, &value); //Change to CMD_GET_CACHE
        }
      }
      //
      // On the power knob restoration, give a standard output that denotes this is a restoration.
      //
        MmrcDebugPrint ((MMRC_DBG_MIN, "..:...:...:..  ", value));
      break;
    }
  }
  if (EyeMaskMet == 0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n\n<> Halting!! <> ERROR, no minimum Eyemask criteria was met.!\n\n"));
    return MMRC_FAILURE; //while(1);
  }

  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function. When doing the vref search for the eyemask
  that can pass the tests, this is the function that prints out the response.  However, this function
  should not be commented out as it also is used for determining if the eyemask criteria has search.
  This may be moved to another task, but then another looping function will be needed.  The format
  of the output is CRK, Ranges for each element and followed by the EM counts.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Knob being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PrintCount (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  UINT8 index;
  UINT32 value;

  //
  // Print out the channel rank knob header only if the strobe and bit indexes are set to 0.
  //
  if (Strobe == 0 && Bit == 0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d  ", Channel, Rank, Knob));
  }
  //
  // Loop through each of the knobs except the first one (>0) and print out the value.  The first one being the timing.
  //
  for (index = Param->PrintCount.ControlKnobs[0].NumberKnobs; index > 0; index--) {
    if (Param->PrintCount.ControlKnobs[0].NumberOfTiming > 1 ) {
      GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF,
        Param->PrintCount.ControlKnobs[Param->PrintCount.ControlKnobs[0].NumberOfTiming].Index, CMD_GET_REG, &value);
    } else {
      GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF,
        Param->PrintCount.ControlKnobs[index].Index, CMD_GET_REG, &value);
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "%03d:", value));
  }
  //
  // Print out the range for this entry.
  //
  if (Range[0] == 0 && Range[1] == 65535) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "xxx:xxx:", Range[0], Range[1]));
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "%03d:%03d:", Range[0], Range[1]));
  }
  //
  // While printing the data, if it is determined that the count is set to the eyemask criteria
  // then leave the alldoneflag alone, otherwise set to 0. It is set to 1 on each interation,
  // and by the end if it remains 1, then the system is done and the first eyemask has met the
  // criteria.
  if (Range[2] != Param->PrintCount.EyeMask[0].NumberConsecutiveVoltages) {
    Param->PrintCount.AllDoneFlag = 0;
  }
  //
  // In order print out the EM counts side-by-side.
  //
  for (index=0; index < 2; index++) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "%d", Range[2+index]));
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "  "));
  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function. This function applies the Set to the specific control.
  The function first tests if the knob needs to be set, and the only reason to not set it is if the first eyemask's
  criteria has been met.  If it has, then exit without setting the knob.  If it needs to still be set, then apply
  the value either.  The only "note" is that this is where the BIT_CONTROL fields is used such that if a register
  has bit-level controls but is not being used at that level, then BIT_CONTROL should be set to 1 and the function
  will set all the bits for the strobe to the same value.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Knob being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
SetCriteria2 (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  EYEMASK_T *EyeMask;
  CONTROLKNOBS *ControlKnobs;
  UINT8 BitLoop;
  UINT8 i;

  EyeMask = Param->SetCriteria2.EyeMask;
  ControlKnobs = Param->SetCriteria2.ControlKnobs;
  //
  // The only reason to not set the specific value is if the first eyemask has reached its criteria for
  // this unique combination. If the other eyemasks have met their requirements, we still need to continue
  // searching for the first to pass.
  //
  if (EyeMask[0].NumberConsecutiveVoltages == Range[2]) {
    //
    // This element has reached the requirements, simply return.
    //
    return MMRC_SUCCESS;
  }

  //
  // Set the element to the specific value.
  //
  if ((ControlKnobs[Param->SetCriteria2.ControlIndex].SharedFlags & (BIT_SHARED|BIT_CONTROL)) == (BIT_SHARED | BIT_CONTROL)) {
    BitLoop=MAX_BITS;
  } else {
    BitLoop = 1;
  }
  //
  // If the shared flag has BIT_CONTROL set to 1, then the getset needs to set each of the bits for this index.
  // otherwise, one single write is all that is needed.  Currently CTLE has bit-control but it is only being used
  // at the strobe level, so to keep all bits equal, this knob needs BIT_CONTROL set.
  //
  for (i=0; i < BitLoop; i++) {
    GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit+i, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, ControlKnobs[Param->SetCriteria2.ControlIndex].Index,
       CMD_SET_VAL_FC_UC, &Param->SetCriteria2.ControlValue);
  }
  return MMRC_SUCCESS;
}

/**
  This function takes the data from Results, and compares the data to the passing criteria of the various
  eyemasks.  If the data point passes the eyemask, the data element is copied to the Results[S/V] array and the
  count is incremented.  The first EyeMask goes into the ResultsV array, the remaining EyeMasks go into ResultsS
  so EM0->ResultsV, EM1->ResultsS[0], EM2->ResultsS[1],etc.  Count[EM0], Count[EM1], Etc. When the eyemask criteria
  is met, the PoweKnobs array is updated; this is needed for all but the first eyemask, since if the eyemask 0 passes,
  the test stops, but all others continue to search.  If the first never passes, the system will look for the prior
  passing cases and restore those power knobs.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ControlKnobs    Control parameters for each dimension.
  @param[in]       Results         Single 1-D Sweep from Create1DSweep() function.
  @param[in]       ResultsV        Single Eyemask of consecutive sweeps.
  @param[in]       ResultsS        Temporary remaining Eyemask sweeps.
  @param[in]       Count           Count of number passing Vrefs for each eyemask.
  @param[in]       PowerKnobs      Passing powerknobs for each eyemask except first.
  @param[in]       EyeMask         Eyemasks used for this algorithm.
  @retval          MMRC_SUCCESS    Return value.
**/
MMRC_STATUS
CheckAndStoreEyeMasks (
  IN MMRC_DATA *MrcData,
  IN CONTROLKNOBS *ControlKnobs,
  IN UINT16 Results[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  IN UINT16 ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  IN UINT16 ResultsS[MAX_EYEMASKS-1][MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
  IN UINT8 Count[MAX_EYEMASKS][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS],
  IN UINT8 PowerKnobs[MAX_POWER_KNOBS][MAX_EYEMASKS-1][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS],
  IN EYEMASK_T *EyeMask
) {
  UINT8 Channel;
  UINT8 Rank;
  UINT8 Knob;
  UINT8 Strobe;
  UINT8 MaxStrobe;
  UINT8 Bit;
  UINT8 Timing;
  UINT8 EMIndex;
  UINT8 Index;
  UINT8 PowerIndex;
  UINT32 Value;

  MaxStrobe = MrcData->MaxNumberStrobes;

  //
  // Loop through all channel, ranks, knobs, strobes, bits testing the eyemasks.
  //
  for (Channel = 0; Channel < MrcData->MaxNumberChannels; Channel++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (!RunOnThisChannel (MrcData, Channel, Rank)) continue;
      for (Knob = 0; Knob < MAX_KNOBS; Knob++) {
        for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
          for (Bit = 0; Bit < MAX_BITS; Bit++) {
            for (Timing = 0; Timing < ControlKnobs[0].NumberOfTiming; Timing++) {
            //
            // Loop through each EyeMask.
            for (EMIndex = 0; EMIndex < MAX_EYEMASKS; EMIndex++) {
              Index = Count[EMIndex][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit];
              //
              // Only test the current EyeMask if it has not already hit the passing criteria.
              //
              if (Index != EyeMask[EMIndex].NumberConsecutiveVoltages) {
                //
                //  Unsigned int, so make sure the High>low before subtracting - MP
                //
                if ((Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] > Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW]) &&
                  ((Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] - Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW]) > EyeMask[EMIndex].MinimumDelayWidth)) {
                  //
                  // If the EM passed, copy if the first into ResultsV, otherwise copy into ResultsS[EM-1] and increment
                  // the appropriate count.
                  //
                  if (EMIndex==0) {
                    ResultsV[Index][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] = Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH];
                    ResultsV[Index][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW]  = Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW];
                  } else {
                    ResultsS[EMIndex-1][Index][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] = Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH];
                    ResultsS[EMIndex-1][Index][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW]  = Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW];
                  }
                  Count[EMIndex][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit]++;
                  //
                  // Loop through the power knobs, storing their positions to be retrieved if needed later.
                  //
                  for (PowerIndex=0; PowerIndex < ControlKnobs[0].NumberKnobs-2; PowerIndex++) {
                    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, ControlKnobs[PowerIndex+2].Index+Knob, CMD_GET_REG, &Value);
                    if (EMIndex != 0) {
                      PowerKnobs[PowerIndex][EMIndex-1][Channel][Rank][Knob][Strobe][Bit] = (UINT8) Value;
                    }
                  }
                } else {
                  //
                  // If the EyeMask failed, reset the count to 0, as the number of consecutive passes must be restarted.
                  //
                  Count[EMIndex][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit] = 0;
                }
              }
            } // for EMIndex
            } // for Timing
          } // for Bit
        } // for Strobe
      } // for Knob
    } // for Rank
  } // for Channel

  return MMRC_SUCCESS;
}

/**
  This function is used twice in the advanced algo function.  When sweeping the Vref N
  times at specific delays to determine the Vref center and when sweeping the Delays
  N times at specific vrefs to determine the Delay center. The dim1 and dim2 controls are
  the index's into the ControlKnob that are related to the sweep dimensions.  The overrides
  are to provide the overrides to the entries int he controlKnob table; for example, when
  doing the sweep of vrefs, the delays will need to be knob shared.. but the actual knob is
  not shared, so the override will have knob_shared.  The final results from the sweep are
  placed in NewResults.  The original number of samples are needed to determine the maximum
  eyewidth of the current data which is about to be overwritten.  This maximum value is needed
  for the center calculation which is the max EyeHeight for the upcoming sweep.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ResultsV        Selected series of data to calculate center.
  @param[in]       OrigNumberSamples   Number of the dim2 samples.
  @param[in]       ControlKnobs    Control parameters for each dimension.
  @param[in]       Dim1Control     Index for the X-dim controls.
  @param[in]       Dim2Control     Index for the Y-dim controls.
  @param[in]       Dim1Override    Overrides for dim1 during this sweep.
  @param[in]       Dim2Override    Overrides for dim2 during this sweep.
  @param[out]      NewResultsV     Output results for this sweep.
  @param[in]       NewNumberSamples The actual number of samples for this sweep.
  @param[out]      Max             Maximum Width from current data, prior to new sweep.
  @retval          MMRC_SUCCESS    Return value.
**/
MMRC_STATUS
SweepData1 (
   IN  MMRC_DATA *MrcData,
   IN  UINT16 ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
   IN  UINT16 OrigNumberSamples,
   IN  CONTROLKNOBS *ControlKnobs,
   IN  UINT16 Dim1Control,
   IN  UINT16 Dim2Control,
   IN  UINT8 Dim1Override,
   IN  UINT8 Dim2Override,
   OUT UINT16 NewResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
   OUT UINT16 NewNumberSamples,
   IN  UINT16 Max[3][MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS],
   IN  UINT8  UsePreviousMax
)
{
  UINT16        Low[MAX_TIMING][MAX_CHANNELS][MAX_STROBES];
  UINT16        High[MAX_TIMING][MAX_CHANNELS][MAX_STROBES];
  UINT16        Dim1StartPoint[MAX_TIMING][MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
  UINT16        Min[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT16        Results[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2];
  BuildCompositeParams  Param;
  UINT8         Loop;
  UINT8         Dim2Loop;
  UINT8         Channel;
  UINT8         Rank;
  UINT8         Timing;
  UINT8         SharedFlag;
  UINT8         Dim2TimingLoop;
  UINT8         Dim1TimingLoop;

  if (MrcData->NumberOfTiming > 1) {
    //if sweeping vref, timing loop is 1
    if (Dim2Control == ControlKnobs[0].NumberOfTiming) {
      Dim2TimingLoop = 1;
      Dim1TimingLoop = 2;
    } else {
      Dim1TimingLoop = 1;
      Dim2TimingLoop = 2;
    }
  } else {
      Dim1TimingLoop = 1;
      Dim2TimingLoop = 1;
  }
  if (UsePreviousMax == FALSE) {
  //
  // Initialize the min and max arrays to 0 and 0xff.
  //
  MmrcMemset(Min, 0xff, MAX_TIMING * MAX_CHANNELS * MAX_RANKS * MAX_KNOBS * MAX_STROBES * MAX_BITS * 2);
  MmrcMemset(Max, 0, MAX_TIMING * MAX_CHANNELS * MAX_RANKS * MAX_KNOBS * MAX_STROBES * MAX_BITS * 2 * 3);
  //
  // Find the Maximum EW of the original data which is swept across the swapped accesses.  Therefore it should be at a function
  // of the dim2 flags. This maximum is needed for the calculation of the center.
  //
  Param.MinMax.Min             = Min;
  Param.MinMax.Max             = Max[Dim2Control];
  for (Loop = 0; Loop < OrigNumberSamples; Loop++){
    SharedFlag = ControlKnobs[Dim2Control].SharedFlags;
    if (ControlKnobs[Dim2Control].PBIndex != -1) {
      SharedFlag &= ~BIT_SHARED;
    }
    SharedFlag |= Dim2Override;
    BuildComposite(MrcData, SharedFlag, MrcData->MaxElements, COMPOSITE_RESULTS, 1,
      0, 0, ResultsV[Loop], &Param, FindMaxMinEW);
  }

  SharedFlag = ControlKnobs[Dim2Control].SharedFlags |KNOB_SHARED;
    if ((Dim2Control < ControlKnobs[0].NumberOfTiming) && (MrcData->NumberOfTiming > 1)) {

      Param.MinMax.Min             = Min;
      Param.MinMax.Max             = Max[Dim2Control];
      BuildComposite(MrcData, SharedFlag|TIMING_SHARED,
      MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, ResultsV[Dim2Control], &Param, FindCommonMaxMinEW);
    }
  //
  // Must find the maximum again, but at the granularity of the dim1, this is required for the determining of the
  // delay steps needed for performing the second sweep.  This is because the granularity of the dim2 (vref) cannot be
  // used when setting the dim1 steps.
  Param.MinMax.Max             = Max[Dim1Control];
  for (Loop = 0; Loop < OrigNumberSamples; Loop++){
    SharedFlag = ControlKnobs[Dim1Control].SharedFlags;
    if (ControlKnobs[Dim1Control].PBIndex != -1) {
      SharedFlag &= ~BIT_SHARED;
    }
    SharedFlag |= Dim2Override;
    BuildComposite(MrcData, SharedFlag, MrcData->MaxElements, COMPOSITE_RESULTS,
      1, 0, 0, ResultsV[Loop], &Param, FindMaxMinEW);
  }
  }
  //
  // Now sweep the vrefs for each pi.  This will be the data used to calculat the center vref.  The original data in the
  // ResultsV can be overridden as it is no longer needed.
  //
  // Re-Initialize the parameters of the 1DSweep to run across the VRefs.
  //
  for (Loop = 0; Loop < Dim1TimingLoop; Loop++) {
    ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[Dim1Control], Low[Loop], High[Loop],
      Dim1StartPoint[Loop], (UINT8) (Dim1Control+Loop));
  }
  //
  // Print out the header.
  //
  {
    UINT8 i;
    UINT8 i2;
    UINT8 count;

    count = 4;
    MmrcDebugPrint ((MMRC_DBG_MIN, "\nCRTH"));

    for (i=0; i< MrcData->MaxElements; i++) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "  SL%d", i));
      count+= 5;
      for (i2=0; i2 < MAX_BITS; i2++) {
        MmrcDebugPrint ((MMRC_DBG_MIN, ":Bi%d", i2));
        count+= 4;
      }
    }

    MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
    while (count > 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "="));
      count--;
    }
  }
  //
  // Loop throuch each PI sampling needed to get the data required for centering the vref.  The placement of the pi
  // is relative, so the initial value must be set correctly, and it will adjust accordingly.
  //
  for (Dim2Loop = 0; Dim2Loop < NewNumberSamples; Dim2Loop++) {

    //if dim2=timing, sweep vref: do readjust, proginitset and completesweep twice, Dim1TimingLoop=1
    //if dim2=vref, sweep dimm1(tiiming), readjust once, proginitset twice, completesweep twice, Dim1TimingLoop=2

    //
    // Set the PI Delay.
    //
    Param.ReAdjust.Max         = Max[Dim2Control];
    Param.ReAdjust.Flags       = ControlKnobs[Dim2Control].SharedFlags | Dim2Override;
    Param.ReAdjust.Index[0]    = ControlKnobs[Dim2Control].Index;

    Param.ReAdjust.Loop        = Dim2Loop;
    Param.ReAdjust.LoopTotal   = NewNumberSamples;

    SharedFlag = ControlKnobs[Dim2Control].SharedFlags |KNOB_SHARED;
    if ((Dim2Control == ControlKnobs[0].NumberOfTiming) && (MrcData->NumberOfTiming > 1)) {
      SharedFlag |= TIMING_SHARED;
    } else {
      if ((Dim2Control < MrcData->NumberOfTiming) && (MrcData->NumberOfTiming >1)) {
        Param.ReAdjust.Index[1]    = ControlKnobs[Dim2Control+1].Index;
      }
    }
    BuildComposite(MrcData, SharedFlag,
      MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, ResultsV[Dim2Loop], &Param, ReAdjust);

    //
    // Perform the sweep.
    //
    for (Timing=0; Timing < 1/*Dim2TimingLoop*/ ; Timing++) { //Agnes

      ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[Dim1Control], Low[Timing], High[Timing],
         Dim1StartPoint[Timing], (UINT8) Dim1Control);

      CompleteSweep(MrcData, ControlKnobs, Dim1Control, High[Timing], Low[Timing], 1, Dim1StartPoint[Timing], Results[Timing]);

      if ((Dim2Control == ControlKnobs[0].NumberOfTiming) && (MrcData->NumberOfTiming > 1)) {
        ProgramInitialSettingsFor1DSweep(MrcData, ControlKnobs[Dim1Control+1], Low[Dim1Control+1], High[Dim1Control+1],
         Dim1StartPoint[Dim1Control+1], (UINT8) (Dim1Control+1));

        CompleteSweep(MrcData, ControlKnobs, (Dim1Control+1), High[Dim1Control+1], Low[Dim1Control+1], 1, Dim1StartPoint[Dim1Control+1], Results[Dim1Control+1]);
      }
    }

    //
    // Copy the results to the vref results array.
    //
    for (Loop = 0; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
      //Even though Arrays are not longer indexed by channel; leaving this loop
      //because of the RunOnThisChannel call. We can come back later and optimize
      //this to do less copies
      for (Channel=0; Channel < MAX_CHANNELS; Channel++) {
        for (Rank = 0; Rank < MAX_RANKS; Rank++) {
          if (!RunOnThisChannel (MrcData, Channel, Rank)) continue;

          Mmrcmemcpy(ResultsV[Dim2Loop][Loop][Rank], Results[Loop][Rank],
              MAX_KNOBS * MAX_CHANNELS_TIMES_STROBES * MAX_BITS * 2 * 2); //Manuel 3 to 2

        } // for Rn
      } // for Ch
    } //for loop
    //
    // Display the output of the sweep.
    //

    Param.PrintHiLow.ControlKnobs = ControlKnobs;
    //Param.PrintHiLow.ControlIndex = Dim1Control;
    Param.PrintHiLow.ControlIndex = 0;
    Param.PrintHiLow.ResultsV = ResultsV;
    Param.PrintHiLow.Dim2Index = (UINT8) Dim2Control;
    Param.PrintHiLow.Dim2Loop = Dim2Loop;
    PrintHiLow(MrcData, &Param);

    if ((MrcData->NumberOfTiming > 1) && (Dim2Control == MrcData->NumberOfTiming)) {
      //MmrcDebugPrint ((MMRC_DBG_MIN, "\n 2nd Timing: \n"));
      Param.PrintHiLow.ControlIndex = 1;
      PrintHiLow(MrcData, &Param);
    }


  } // for Dim2Loop

  return MMRC_SUCCESS;
}

/**
  Based on the dim2 center position, this function calculates the center x position
  by taking weights around the center such that as the sweep gets further away from
  the dim2 center, the values have less an impact on the calculations.  This function
  all needs to know the dim2 total range which is passed in.. This was determined by
  using the previous data and calculating the maximum eye width.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ResultsV        Selected series of data to calculate center.
  @param[in]       Flags           Shared flags of data.
  @param[in]       NumberSamples   Number of the dim2 samples.
  @param[out]      PICenter        Center results at the granularity of Flags.
  @param[in]       Max             Maximum eye height from previous sweep.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CalculateCenter2 (
   IN MMRC_DATA  *MrcData,
   IN UINT16      ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
   IN UINT8       Flags,
   IN UINT8       NumberSamples,
   OUT UINT16     PiCenter[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS],
   IN  UINT16     Max[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS]
) {
  UINT8         VrefLoop;
  BuildCompositeParams  Param;

  //
  // Overwrite the results with the composite data at this time.  This is needed int he final
  // calculations as it will be looping through all the vrefs in parallel.
  //
  for (VrefLoop = 0; VrefLoop < NumberSamples; VrefLoop++){
    BuildComposite(MrcData, Flags, MrcData->MaxElements, COMPOSITE_RESULTS|OVERWRITE, 1, 0, 0, ResultsV[VrefLoop], 0,
      (MMRC_STATUS (*) (MMRC_DATA *, UINT16 *, UINT8, UINT8, UINT8, UINT8, UINT8, UINT8, BuildCompositeParams*)) -1);
  }

  //
  // Do the final calculations, rather then looping here, passing in the number samples and resultsV arrays
  // allows the looping to be done inside of the LoopOnComposite function
  //
  Param.FinalCalculateCenter2.NumberSamples = NumberSamples;
  Param.FinalCalculateCenter2.ResultsV = ResultsV;
  Param.FinalCalculateCenter2.Max = Max;
  Param.FinalCalculateCenter2.Center = PiCenter;
  Param.FinalCalculateCenter2.Flags = Flags;

  BuildComposite(MrcData, Flags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0,
    0, &Param, FinalCalculateCenter2);

  return MMRC_SUCCESS;
}

/**
  One of two methods to calculate the center for a series of results.  This
  method assumes we do not know anything about the center of the y posisiton, it
  calculates the center based on weights only.  If the flags are perbit, it will
  calculate the per-bit center, if the flags are per-strobelane.  This method is
  used to estimate the center position when doing advanced training since a center
  y position is not known.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ResultsV        Selected series of data to calculate center.
  @param[in]       Flags           Shared flags of data.
  @param[in]       SelectedEyeMask Used to provide the number of vref samples.
  @param[out]      PICenter        Center results at the granularity of Flags.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CalculateCenter1 (
   IN MMRC_DATA  *MrcData,
   IN UINT16     ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2],
   IN UINT8      Flags,
   IN EYEMASK_T  *EyeMask,
   IN UINT8      SelectedEyeMask,
   IN UINT16     PICenter[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS]
) {
  UINT8         VrefLoop;
  UINT16        Min[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT16        Max[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT8         AccumWeights[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  BuildCompositeParams  Param;
  UINT8         Timing;

  //
  // Initialize all arrays.
  //
  MmrcMemset(Min, 0xff, MAX_TIMING*MAX_CHANNELS*MAX_RANKS*MAX_KNOBS*MAX_STROBES*MAX_BITS*2);
  MmrcMemset(Max, 0, MAX_TIMING*MAX_CHANNELS*MAX_RANKS*MAX_KNOBS*MAX_STROBES*MAX_BITS*2);
  MmrcMemset(AccumWeights, 0, MAX_TIMING*MAX_CHANNELS*MAX_RANKS*MAX_KNOBS*MAX_STROBES*MAX_BITS);
  MmrcMemset(PICenter, 0, MAX_TIMING*MAX_CHANNELS*MAX_RANKS*MAX_KNOBS*MAX_STROBES*MAX_BITS*2);

  //
  // Loop through each of the dim2 values and determine the maximum and minimimum eyewidths.
  //
  for (Timing = 0; Timing < MAX_TIMING; Timing++) {
  Param.MinMax.Max             = Max;
  Param.MinMax.Min             = Min;
  }

  for (VrefLoop = 0; VrefLoop < EyeMask[SelectedEyeMask].NumberConsecutiveVoltages; VrefLoop++)
  {
    BuildComposite(MrcData, Flags, MrcData->MaxElements, COMPOSITE_RESULTS, 1,
      (UINT8) EyeMask[0].NumberConsecutiveVoltages,  0, ResultsV[VrefLoop], &Param, FindMaxMinEW);
  }

  //
  // Loop again though each of the dim2 values but this time get the center.  This takes the
  // min and max values previously determined as an input and determins the picenter.  The
  // AccumWeights array is a temporary location that is needed as each vref is stepped through.
  //
  Param.MinMaxPICenter.Max      = Max;
  Param.MinMaxPICenter.Min      = Min;
  Param.MinMaxPICenter.PICenter = PICenter;
  Param.MinMaxPICenter.AccumWeights    = AccumWeights;

  for (VrefLoop = 0; VrefLoop < EyeMask[SelectedEyeMask].NumberConsecutiveVoltages; VrefLoop++)
  {
    //
    // On all but the first vrefs, set the final flag to 0, otherwise set it to 1.
    // This flag is used to tell the centering algorithm to now divide the total by the
    // accumulated weights.
    //
    if (VrefLoop == EyeMask[SelectedEyeMask].NumberConsecutiveVoltages-1) {
      Param.MinMaxPICenter.Final[0] = 1;
      Param.MinMaxPICenter.Final[1] = 1;
    } else {
      Param.MinMaxPICenter.Final[0] = 0;
      Param.MinMaxPICenter.Final[1] = 0;
    }
    BuildComposite(MrcData, Flags, MrcData->MaxElements, COMPOSITE_RESULTS, 1,
      (UINT8) EyeMask[0].NumberConsecutiveVoltages, 0, ResultsV[VrefLoop], &Param, ApplyMaxMinEWWeight);
  }

  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function. This is the most generic print function.
  It simply prints out the Channel, rank, and strobe if the strobe and bit index's are set to 0 but also
  prints out the low and high ranges seperated by a ":" for all entries.  The end result is to see :
        000 020:055 010:022 012:054 ...

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Knob being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
GenericPrint2 (
  IN MMRC_DATA    *MrcData,
  IN UINT16       *Range,
  IN UINT8         Channel,
  IN UINT8         Rank,
  IN UINT8         Knob,
  IN UINT8         Strobe,
  IN UINT8         Bit,
  IN UINT8         Timing,
  IN BuildCompositeParams *Param
) {
 if (Strobe == 0 && Bit == 0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d ", Channel, Rank, Knob));
  }
 MmrcDebugPrint ((MMRC_DBG_MIN, "%03d:%03d ", Range[LOW], Range[HIGH]));
 return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function.  When calculating the center based on the center position,
  the only additional data required is the maximum Eye Height.  This is passed in on the param structures in Max.
  The function will loop though each sweep giving the larger weight to the center y and less and less weight as the sweep
  sample gets further away from the center.  This goes until NumberSamples is reached.
  The final center position is then divided by the sum weights.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Knob being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
FinalCalculateCenter2 (
  IN MMRC_DATA *MrcData,
  IN UINT16 *Range,
  IN UINT8 Channel,
  IN UINT8 Rank,
  IN UINT8 Knob,
  IN UINT8 Strobe,
  IN UINT8 Bit,
  IN  UINT8 Timing,
  IN BuildCompositeParams *Param
) {
  UINT16  NumberSamples;
  UINT16  EyeWidthAtCenter;
  UINT32  SumLR;
  UINT32  SumWeights;
  UINT8   Loop;
  UINT8 MaxStrobe;

  MaxStrobe = MrcData->MaxNumberStrobes;

  //
  // Determine the number of samples.
  //
  NumberSamples = Param->FinalCalculateCenter2.NumberSamples;

  if (NumberSamples == 1) {
    Param->FinalCalculateCenter2.Center[Timing][Channel][Rank][Knob][Strobe][Bit] =
      (Param->FinalCalculateCenter2.ResultsV[0][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] +
      Param->FinalCalculateCenter2.ResultsV[0][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW])/2;
    return MMRC_SUCCESS;
  }
  //
  // Determine the eye width at the center Y. Give Number Samples, this is the NumberSamples/2 position.
  //
  EyeWidthAtCenter = Param->FinalCalculateCenter2.ResultsV[NumberSamples/2][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] -
                     Param->FinalCalculateCenter2.ResultsV[NumberSamples/2][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW];
  //
  // Initialize the SumLR and SumWeights to 0.
  //
  SumLR = 0;
  SumWeights = 0;
  //
  // Loop from center going up, so the number of loops is sample/2 accumulating the SumLR and SumWeights.
  // the SumLR uses the value at the +y and -y... this is why there are only sample/2 y's.
  //
  for (Loop = 0; Loop <= (NumberSamples)/2; Loop++) {
    SumLR  += (
    ( ((NumberSamples)/2-Loop) * Param->FinalCalculateCenter2.Max[Timing][Channel][Rank][Knob][Strobe][Bit] + Loop * EyeWidthAtCenter ) *
    ( Param->FinalCalculateCenter2.ResultsV[NumberSamples/2 + Loop][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] + Param->FinalCalculateCenter2.ResultsV[NumberSamples/2 - Loop][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HIGH] +
      Param->FinalCalculateCenter2.ResultsV[NumberSamples/2 - Loop][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW]  + Param->FinalCalculateCenter2.ResultsV[NumberSamples/2 + Loop][Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][LOW]
    ));
    SumWeights += 2*( ((NumberSamples)/2-Loop) * Param->FinalCalculateCenter2.Max[Timing][Channel][Rank][Knob][Strobe][Bit] + (Loop) * EyeWidthAtCenter );
  } // for Loop
  //
  // If the sumWeights > 0 (0 cannot be dividied), then divide the center by the SumWeights; otherwise, set the center to 0.
  //
  if (SumWeights != 0) {
    Param->FinalCalculateCenter2.Center[Timing][Channel][Rank][Knob][Strobe][Bit] = (UINT16) ((SumLR / SumWeights) / 2);
  } else {
    Param->FinalCalculateCenter2.Center[Timing][Channel][Rank][Knob][Strobe][Bit] = 0;
  }

 return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function.  All ranges are compared with each
  other and the maximum and minimum values are determined and stored into the max and min array
  elements.  This is to find the max/min at different dim2 components, not at the flags level.
  Prior to this funtion being called the maximum value must be set to 0, and the minimum value must
  be set to all 0xffff.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Lmpb being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
FindMaxMinEW (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Ch,
  IN  UINT8 Rn,
  IN  UINT8 Kn,
  IN  UINT8 St,
  IN  UINT8 Bi,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  UINT16  Width;
  //
  // Determine the width of the eye.
  //
  Width = Range[HIGH] - Range[LOW];
  if (Range[LOW] > Range[HIGH]) {
    Param->MinMax.Max[Timing][Ch][Rn][Kn][St][Bi] = 0;
    return MMRC_SUCCESS;
  }
  //
  // If the stored max is smaller then the new width, then overwrite with the new width.
  //
  if (Param->MinMax.Max[Timing][Ch][Rn][Kn][St][Bi] < Width){
    Param->MinMax.Max[Timing][Ch][Rn][Kn][St][Bi] = Width;
  }
  //
  // If the stored min is smalled then the new width, then overwrite with the new width.
  //
  if (Param->MinMax.Min[Timing][Ch][Rn][Kn][St][Bi] > Width){
    Param->MinMax.Min[Timing][Ch][Rn][Kn][St][Bi] = Width;
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
FindCommonMaxMinEW (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Ch,
  IN  UINT8 Rn,
  IN  UINT8 Kn,
  IN  UINT8 St,
  IN  UINT8 Bi,
  IN  UINT8 Tim,
  IN  BuildCompositeParams *Param
) {

  //
  // If the stored max is smaller then the new width, then overwrite with the new width.
  //
  if (Param->MinMax.Max[Ch][0][Rn][Kn][St][Bi] < Param->MinMax.Max[Ch][1][Rn][Kn][St][Bi]){
     Param->MinMax.Max[Ch][0][Rn][Kn][St][Bi] = Param->MinMax.Max[Ch][1][Rn][Kn][St][Bi];
  } else {
    Param->MinMax.Max[Ch][1][Rn][Kn][St][Bi] = Param->MinMax.Max[Ch][0][Rn][Kn][St][Bi];
  }
  //
  // If the stored min is smalled then the new width, then overwrite with the new width.
  //
  if (Param->MinMax.Min[Ch][0][Rn][Kn][St][Bi] > Param->MinMax.Min[Ch][1][Rn][Kn][St][Bi] ){
    Param->MinMax.Min[Ch][0][Rn][Kn][St][Bi] = Param->MinMax.Min[Ch][1][Rn][Kn][St][Bi] ;
  } else {
    Param->MinMax.Min[Ch][1][Rn][Kn][St][Bi] = Param->MinMax.Min[Ch][0][Rn][Kn][St][Bi] ;
  }

  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function.  When calculating the center position
  using Maximum/Medium/Minimum widths method, this function is used to do the summation.  It is assumed
  that the maximum and minimum values are determined prior and setup in the Max and Min array elements.
  The function will look at the current value, compare it with the min/max values and multiply the
  average eyewidth by the min/mid/max weights. At the end, the sum of the individual min/mid/max weights
  that were multiplied are then divided.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Lmpb being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ApplyMaxMinEWWeight (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Ch,
  IN  UINT8 Rn,
  IN  UINT8 Kn,
  IN  UINT8 St,
  IN  UINT8 Bi,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  //
  // Check if the eyewidth is one of the maximium values.
  //
  if ((Range[HIGH] - Range[LOW]) == Param->MinMaxPICenter.Max[Timing][Ch][Rn][Kn][St][Bi]) {
    //
    // Multiple the center position (x+y/2) by the max weight and accumulate the max weight.
    //
    Param->MinMaxPICenter.AccumWeights[Timing][Ch][Rn][Kn][St][Bi] += MAX_WEIGHT;
    Param->MinMaxPICenter.PICenter[Timing][Ch][Rn][Kn][St][Bi] += ((Range[HIGH] + Range[LOW]) * MAX_WEIGHT) / ( 2);
  //
  // Check if the eyewidth is one of the minimum values.
  //
  } else if ((Range[HIGH] - Range[LOW]) == Param->MinMaxPICenter.Min[Timing][Ch][Rn][Kn][St][Bi]) {
    //
    // Multiple the center position (x+y/2) by the min weight and accumulate the min weight.
    //
    Param->MinMaxPICenter.AccumWeights[Timing][Ch][Rn][Kn][St][Bi] +=MIN_WEIGHT;
    Param->MinMaxPICenter.PICenter[Timing][Ch][Rn][Kn][St][Bi] += ((Range[HIGH] + Range[LOW]) * MIN_WEIGHT) / ( 2);
  //
  // If this point is reached, it must have been a middle/medimum value.
  //
  } else {
    //
    // Multiple the center position (x+y/2) by the med weight and accumulate the med weight.
    //
    Param->MinMaxPICenter.AccumWeights[Timing][Ch][Rn][Kn][St][Bi] +=MID_WEIGHT;
    Param->MinMaxPICenter.PICenter[Timing][Ch][Rn][Kn][St][Bi] += ((Range[HIGH] + Range[LOW]) * MID_WEIGHT) / ( 2);
  }
  //
  // If on the last Vref, (the final flag will not be set)  then divide the total accumulation by the sum of the weights.
  //
  if (Param->MinMaxPICenter.Final[Timing] == 1) {
    Param->MinMaxPICenter.PICenter[Timing][Ch][Rn][Kn][St][Bi] /= Param->MinMaxPICenter.AccumWeights[Timing][Ch][Rn][Kn][St][Bi];
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
SetPBDLinear (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 Value,
  IN  UINT16 PBIndex,
  IN  UINT8  PBFlags,
  IN  UINT8  AllBitsFlag,
  IN  UINT8  Done[MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS])
{
  BuildCompositeParams  Param;

  Param.ProgramSetGetLinear.PI  = Value;
  Param.ProgramSetGetLinear.PBIndex = PBIndex;
  Param.ProgramSetGetLinear.AllBits = AllBitsFlag;
  Param.ProgramSetGetLinear.Done = Done;

  BuildComposite(MrcData, PBFlags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, 0, &Param, ProgramSetGetLinear);

  return MMRC_SUCCESS;
}

MMRC_STATUS
ProgramSetGetLinear (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
)
{
  if (Param->ProgramSetGetLinear.AllBits == FALSE) {
    if (Param->ProgramSetGetLinear.Done[Channel][Rank][Knob][Strobe][Bit] == TRUE)
      return MMRC_SUCCESS;
    GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Param->ProgramSetGetLinear.PBIndex, CMD_SET_VAL_FC_UC, &Param->ProgramSetGetLinear.PI);
  }

  if (Param->ProgramSetGetLinear.AllBits == TRUE) {
    GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Param->ProgramSetGetLinear.PBIndex, CMD_SET_VAL_FC_UC, &Param->ProgramSetGetLinear.PI);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
TestNonLinearPBForDone (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
)
{
  if (Param->ProgramSetGet.Done[Timing][Channel][Rank][Knob][Strobe][Bit] == FALSE) {
    if ((Param->ProgramSetGet.Lo[Timing][Channel][Rank][Knob][Strobe][Bit]-Range[0] >= (Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][Bit]-Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe])) &&
        (Range[0] < Param->ProgramSetGet.Lo[Timing][Channel][Rank][Knob][Strobe][Bit])) {
      Param->ProgramSetGet.Done[Timing][Channel][Rank][Knob][Strobe][Bit] = TRUE;
    } else {
      Param->ProgramSetGet.AllDone = FALSE;
    }
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
DetermineStrobeVal (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
)
{
  UINT8 BitLoop;
  if (Param->ProgramSetGet.ControlKnobs[0].PBDirectFlag == TRUE) {
    //
    // if the Per-bit and strobe impact each other (both are DQ's, then the SL value needs to be the maximum.
    // Find the maximum of each bit of the strobe.
    //
    Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe] = 0xffff;
    for (BitLoop = 0; BitLoop < MAX_BITS; BitLoop++) {
      if (Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][BitLoop] < Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe]) {
        Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe] = Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][BitLoop];
      }
    }
  } else {
    //
    // if the Per-bit and strobe impact each other inversly (DQ/DQS, then the SL value needs to be the minimum.
    // Find the minimum of each bit of the strobe.
    //
    Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe] = 0x00;
    for (BitLoop = 0; BitLoop < MAX_BITS; BitLoop++) {
      if (Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][BitLoop] > Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe]) {
        Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe] = Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][BitLoop];
      }
    }
  }
  if (Strobe == 0 && Bit == 0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d", Channel, Rank, Timing));
  }
  if (Bit==0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "  %03d", Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe]));
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, " %03d", Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][Bit] - Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe]));
  return MMRC_SUCCESS;
}

MMRC_STATUS
CopyToLow (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
)
{
  Param->ProgramSetGet.Lo[Timing][Channel][Rank][Knob][Strobe][Bit] = Range[LOW];
  if (Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][Bit] - Param->ProgramSetGet.StrobeVal[Channel][Rank][Knob][Strobe] == 0) {
    Param->ProgramSetGet.Done[Timing][Channel][Rank][Knob][Strobe][Bit] = TRUE;
  } else {
    Param->ProgramSetGet.AllDone = FALSE;
  }
  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function.  Upon looking at the flags, specified within the
  param structure, if the per-bit is set and the calculation is set to true, the system will calculate the per-bit
  value.  The per-bit value is done by getting the average of each bit, and subtracting the current value from the
  average.  if the per-bit flag is not set, the system will simply set the value and exit the function.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Lmpb being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ProgramSetGet (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
)
{
  UINT32 StrobeValue;
  UINT32 BitValue;
  UINT8  BitLoop;
  //
  // The actual value to be programmed should be put into the "StrobeValue" variable.
  //
  StrobeValue = Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][Bit];
  //
  // Check if the per-bit is needed to be programmed.
  //
  if ((Param->ProgramSetGet.Flags & BIT_SHARED) != BIT_SHARED) {
    //
    // If the per-bit is needed, check if the bit can be calulated or needs to be swept.
    //
    if ((Param->ProgramSetGet.Flags & CALCULATE_BIT) == CALCULATE_BIT) {
      //
      // With printing enabled, the first line will always have the specific Channel, Rank, and Strobe.
      //
      if (Strobe == 0 && Bit == 0 ) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d", Channel, Rank, Timing));
      }
      //
      // Since this function will be called at a bit-level, multiple times for each strobelane,
      // the average strobe lane will be used, this is accomplished by adding all the values
      // from each bit and then dividing it by the number of bits.
      //
        if (Bit == 0) {
          for (BitLoop = 1; BitLoop < MAX_BITS; BitLoop++) {
            if (Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][BitLoop] < StrobeValue) {
              StrobeValue = Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][BitLoop];
            }
          }
          Param->ProgramSetGet.StrobeValue = StrobeValue;
          MmrcDebugPrint ((MMRC_DBG_MIN, "  %03d", StrobeValue));
        } else { // if (Bi == 0)
          StrobeValue = Param->ProgramSetGet.StrobeValue;
        } // if (Bi == 0)
        //
        // Program the per-bit value, this number is the difference from the strobe average value
        // and the per-bit value. This number can be postive or negative.
        //
        if (Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][Bit] - StrobeValue > 15 ) {
          BitValue = 0;
        } else {
          BitValue = 15-(Param->ProgramSetGet.PI[Timing][Channel][Rank][Knob][Strobe][Bit] - StrobeValue);
        }
        GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, Param->ProgramSetGet.Index[Timing], CMD_SET_VAL_FC_UC, &BitValue);
        MmrcDebugPrint ((MMRC_DBG_MIN, ":%03d", BitValue));
      } else { // if (Param->ProgramSetGet.Flags & BIT_CALCULATE)
        UINT16           High[MAX_CHANNELS][MAX_STROBES];
        UINT16           Low[MAX_CHANNELS][MAX_STROBES];
        UINT16           Dim1StartPoint[MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2];
        UINT16           Results[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2];
        UINT16           Initial[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
        UINT8            Done[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
        INT16            PBStartValue;
        UINT8            MaxStrobe;

        ////TODO: Add a check for maximum size of Channel and Strobe
        MaxStrobe = MrcData->MaxNumberStrobes;

        //
        // Doing a sweep to compute delay per-bit value.
        //
        //
        if (Param->ProgramSetGet.FirstEntryFlag == TRUE) {
          Param->ProgramSetGet.FirstEntryFlag = FALSE;

          BuildComposite(MrcData, Param->ProgramSetGet.Flags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, Results, Param, DetermineStrobeVal);

          //
          // Do the first sweep to get a baseline to compare future sweeps.
          //
          ProgramInitialSettingsFor1DSweep(MrcData, Param->ProgramSetGet.ControlKnobs[0], Low, High, Dim1StartPoint, (UINT8) 0);
          CompleteSweep(MrcData, Param->ProgramSetGet.ControlKnobs, 0, High, Low, 1, Dim1StartPoint, Results[Timing]);
          for (Knob= 0; Knob < MAX_KNOBS; Knob++) {
            for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
              //
              // Need the starting point to be a passing case, find the largest low side, and set the starting strobe lane to be there.
              // The the 1dsweep will determine the ending point for each bit the fastest.
              //
              Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW]  = (UINT16) (Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][0][LOW]);
              for (BitLoop=1; BitLoop<MAX_BITS; BitLoop++) {
                if (Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][BitLoop][LOW] > Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW]){
                  Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW] = Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][BitLoop][LOW];
                }
              }
              //
              // Set the high starting point to be out of bounds since its not needed, this is done by setting it 1 point beyond the high point.
              //
              Dim1StartPoint[Rank][Channel][Knob][Strobe][HIGH]  = Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW] + GetQtrClk(MrcData, Channel) + 1;
              High[Knob][Strobe] = Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW] + GetQtrClk(MrcData, Channel);
              Low[Knob][Strobe]  = Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW] - GetQtrClk(MrcData, Channel);
            } // for St
          } // for Knob
          MmrcMemset (Done , FALSE, MAX_CHANNELS * MAX_TIMING * MAX_RANKS * MAX_KNOBS * MAX_STROBES * MAX_BITS * sizeof (UINT8));
          Param->ProgramSetGet.AllDone = FALSE;   // Initial, CopyToLow will change it.
          Param->ProgramSetGet.Done = Done;
          Param->ProgramSetGet.Lo = Initial;
          PBStartValue = Param->ProgramSetGet.ControlKnobs[0].PBStartValue;
          BuildComposite(MrcData, Param->ProgramSetGet.Flags, MrcData->MaxElements, COMPOSITE_RESULTS, 1, 0, 0, Results, Param, CopyToLow);
          BuildComposite(MrcData, Param->ProgramSetGet.Flags, MrcData->MaxElements, COMPOSITE_RESULTS, 1, 0, 0, Results, Param, TestNonLinearPBForDone);

          while (Param->ProgramSetGet.AllDone == FALSE && Param->ProgramSetGet.ControlKnobs[0].PBStartValue > 3) {
            Param->ProgramSetGet.ControlKnobs[0].PBStartValue-=1;
            Param->ProgramSetGet.AllDone = TRUE;
            SetPBDLinear ( MrcData, Param->ProgramSetGet.ControlKnobs[0].PBStartValue, Param->ProgramSetGet.ControlKnobs[0].PBIndex, Param->ProgramSetGet.ControlKnobs[0].PBSharedFlags, FALSE, Done[Timing]);
            CompleteSweep(MrcData, Param->ProgramSetGet.ControlKnobs, 0, High, Low, 1, Dim1StartPoint, Results[Timing]);
            for (Knob= 0; Knob < MAX_KNOBS; Knob++) {
              for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
                Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW]  = (UINT16) (Results[Timing][Rank][Knob][(MaxStrobe * Channel)+ Strobe][0][LOW]);
              } // for St
            } // for Knob
            BuildComposite(MrcData, Param->ProgramSetGet.Flags, MrcData->MaxElements, COMPOSITE_RESULTS, 1, 0, 0, Results, Param, TestNonLinearPBForDone);
          }
        }
        // This should not be done for phase 0.
        //
      } // if (Param->ProgramSetGet.Flags & BIT_CALCULATE)
  } else { // if Param->ProgramSetGet.Flags
    //
    // With printing enabled, the first line will always have the specific Channel, Rank, and Strobe.
    //
    if (Strobe == 0 && Bit == 0 ) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d", Channel, Rank, Timing));
    }
    //
    // Only set the Strobe lane if the bi index is set to 0 and there is a valid index.
    //
    if ((Param->ProgramSetGet.Flags & RANK_SHARED) == RANK_SHARED)
    {
      for (Rank = 0; Rank < MAX_RANKS; Rank++)
      {
        GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, Param->ProgramSetGet.Index[Timing], CMD_SET_VAL_FC_UC, &StrobeValue);
      }
    } else {
      GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, Param->ProgramSetGet.Index[Timing], CMD_SET_VAL_FC_UC, &StrobeValue);
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "  %03d", StrobeValue));
  }

  return MMRC_SUCCESS;
}

/**
  Supports Late Command Training, Late Read Training, and Late Write training.
  Performs the steps outlined in the Advanced Training Algorihtm document.
  There are two main inputs to the algorithem, Control Knobs and eye Masks.  Both of
  these structures are passed in as they are dependent on the algorithm.  The main algorithm
  can be broken down into 4 fundamental steps.

    1.  Determing passing power/Vref given the eyemask.
    2.  Estimate Delay center given the passing eyemasks.
    3.  At the estimated Delay, determine Vref center.
    4.  At the Vref Center, sample/calculate Delay Center.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ControlKnobs    External array of controls for the algorithm.
  @param[in]       NumberContrlKnbs Number of control knobs array.
  @param[in]       EyeMask         Array of passing EyeMasks.
  @param[in]       NumberEM        Number of eyemasks in the array.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
AdvancedTrainingAlgo (
  IN MMRC_DATA *MrcData,
  IN CONTROLKNOBS *ControlKnobs,
  IN UINT8 NumberControlKnobs,
  IN EYEMASK_T  *EyeMask,
  IN UINT8 NumberEyeMask,
  IN UINT8 PerBitEnable
) {
  INT8 CompareFlag = 0xFF;
  UINT16    ResultsV[MAX_CONSEC][MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2];
  UINT16    PICenter[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT16    PICenterPB[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT16    PICenterVref[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT16    Max[3][MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS];
  UINT8     VrefIndex;
  UINT8     Loop;
  INT16     Index[MAX_TIMING];
  UINT8     SharedFlags;
  UINT8     Ch, Rk, St;
  CONTROLKNOBS ControlKnobsBackup[3+RT_CTLE_ENABLE*2];
  UINT8     Sample;
  UINT32    TempValue;

  //
  // Phase 1 and 2, will always used a single timing even with multiple specified if the sharedIndex
  // value is set to non -1.  Store the original Control knobs prior to changing so that the original
  // can be restored during phase 3.
  //
  if (ControlKnobs[0].SharedIndex != -1) {
    for (Loop = 0; Loop < NumberControlKnobs; Loop++) {
      ControlKnobsBackup[Loop] = ControlKnobs[Loop];
      if (Loop > 0) {
        ControlKnobs[Loop-1] = ControlKnobs[Loop];
      }
    }
    NumberControlKnobs--;
    ControlKnobs[0].NumberOfTiming = 1;
    MrcData->NumberOfTiming = 1;
    ControlKnobs[0].Index = ControlKnobs[0].SharedIndex;
  }
  VrefIndex = ControlKnobs[0].NumberOfTiming;
//  UINT8     Channel;
  //
  if (PerBitEnable == TRUE) {
    if (ControlKnobs[0].PBIndex != 0xffff) {
      SetPBDLinear ( MrcData, ControlKnobs[0].PBStartValue, ControlKnobs[0].PBIndex, ControlKnobs[0].PBSharedFlags, TRUE, NULL);
    }
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nResetting FIFOs"));
  for (Ch = 0; Ch < MAX_CHANNELS; Ch++) {
    for (Rk = 0; Rk < MAX_RANKS; Rk++) {
      PrechargeAll(MrcData, Ch, Rk);
      PerformDQReset(MrcData, Ch, Rk);
      PerformFifoReset(MrcData, Ch, Rk);
    }
  }
  // Step 1. Determine Passing Vrefs.
  // The return data is in ResultsV and the Actual EyeMask that was selected is in the SelectEyeMask
  // variable.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\n PHASE 1: ESTIMATING SL CENTER"));
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nDetermining Passing Vrefs."));
  DeterminePassingVref2(MrcData, ControlKnobs, NumberControlKnobs-1, ResultsV, EyeMask, NumberEyeMask);
  //
  // If the timing is per-bit enabled, apply the per-bit value based on the granularity of the per-bit field.  It should
  // be noted that the per-bit is not necessarily at the same granularity of the StrobeLane, so it must have its own
  // shared flag, this is specific to the timing per-bit flag.  It should be not that the per-bit will be used in the
  // estimation if there is a per-bit and if the per-bit can be calculated, if it must be swept, this step is skipped.
  //
  if (PerBitEnable == TRUE) {
    if ((ControlKnobs[0].PBSharedFlags != CompareFlag) && (ControlKnobs[0].PBSharedFlags & CALCULATE_BIT) == CALCULATE_BIT) {
      //
      // Calculate the center PI, this is an estimate only, and no matter what algorithm, the timing flags should be
      // Knob shared during this phase.  This algorithm, if bit-independent will computer the center per-bit and apply the per-bit
      // as a difference from the bit to the strobe-level max.
      //
      CalculateCenter1(MrcData, ResultsV, ControlKnobs[0].PBSharedFlags, EyeMask, 0, PICenter);
      //
      // Apply The PICenter per-bit value only.  This will set the Per-Bit fields only.
      //
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nEstimating PerBit Pi Center."));
    for (Loop = 0; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
      Index[Loop] = -1;
    }

    ApplySetting(MrcData, PICenter, -1, ControlKnobs[0].PBSharedFlags, Index, ControlKnobs[0].PBIndex, ControlKnobs);
      //
      // Recalculate the center position, at this point, the center position should be the same for all bits.
      //
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nReDetermining Passing Vrefs."));
      DeterminePassingVref2(MrcData, ControlKnobs, NumberControlKnobs-1, ResultsV, EyeMask, NumberEyeMask);
    }
  } // if PerBitEnable
  //
  // Now re-calculate the PICenter but at the granularity of the StrobeLane, not the per-bit.  This uses the same
  // centering algorithm done for the per-bit but at the granularity of the strobes which can be different then the
  // per-bit.
  //

    CalculateCenter1(MrcData, ResultsV, ControlKnobs[0].SharedFlags|KNOB_SHARED, EyeMask, 0, PICenter);

    //
    // Copy the PICenter for use as the
    //

    //
    // Apply the Strobelane values.
    //
    MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nEstimating SL PI Center."));

    for (Loop = 0; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
      Index[Loop] = ControlKnobs[Loop].Index;
    }

    ApplySetting(MrcData, PICenter, ControlKnobs[0].SharedFlags|KNOB_SHARED, -1, Index, -1, ControlKnobs);

  if (ControlKnobs[VrefIndex].Ranges[HIGH] == ControlKnobs[VrefIndex].Ranges[LOW]) {
    return MMRC_SUCCESS;
  }
  //CADumpAndTest(MrcData, 0, 0, 0);
  // Step 2a.  The function will compute the maximum eyemask and sweep the data across vrefs overwriting the original
  // data in ResultsV.  No matter what the granularity of the PI, the vref will set the delay values to a specific
  // strobelane across all bits and knobs.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\n PHASE 2: FINAL VREF CENTERING"));
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nDisplay the VRef Swept Data at the bit granularity."));
  SweepData1(MrcData, ResultsV, EyeMask[0].NumberConsecutiveVoltages, ControlKnobs, VrefIndex, 0, 0, KNOB_SHARED|BIT_SHARED, ResultsV, ControlKnobs[1].NumberSweeps, Max, FALSE);

  //
  // Step 2b.  After collecting the swept data across a series of delays for the vref range, the center vref can be calculated.
  // This value is at the granularity of the second (VREF) controlknob.  The centering algorithm is same as step 1
  //
  //CalculateCenter2(MrcData, ResultsV, ControlKnobs[1].SharedFlags, RT_STEP2_PISAMPLES, PICenter, Max[0]);
  //EyeMask[0].NumberConsecutiveVoltages = 1;
  CalculateCenter1(MrcData, ResultsV, ControlKnobs[VrefIndex].SharedFlags, EyeMask, 0, PICenterVref);

  for (Loop = 0; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
    Index[Loop] = ControlKnobs[VrefIndex].Index;
  }

  SharedFlags = ControlKnobs[VrefIndex].SharedFlags;
  if (MrcData->NumberOfTiming > 1) {
    SharedFlags |= TIMING_SHARED;
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nStep2: Vref Centering Results"));
  ApplySetting(MrcData, PICenterVref, SharedFlags, -1, Index, ControlKnobs[VrefIndex].PBIndex, ControlKnobs);
  //CADumpAndTest(MrcData, 0, 0, 0);

  if (MrcData->NumberOfTiming > 1) {
      for (Ch = 0; Ch < MAX_CHANNELS; Ch++) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Ch].Enabled == TRUE) {
        //Save the Command and Control value to be programmed when failed
        //
        for (Rk = 0; Rk < MAX_RANKS; Rk++) {
          for (St = 0; St < MrcData->MaxElements; St++) {
            for (Loop = 0; Loop < MAX_TIMING; Loop++) {
              GetSetDdrioGroup2 (MrcData, Ch, Rk, St, ControlKnobs[Loop].Index, CMD_GET_CACHE, &TempValue);
              GetSetDdrioGroup2 (MrcData, Ch, Rk, St, ControlKnobs[Loop].Index, CMD_SET_VAL_FC, &TempValue);
            }
          }
        }
      }
    }
  }
  //
  // Step 3a.  Sweep the timing delays across a series of Vrefs, this will overwrite the ResultsV again but with various vrefs
  // sampled around Vref Center.
  //
  // Restore the orignal.
  if (ControlKnobs[0].SharedIndex != -1) {
    NumberControlKnobs++;
    Mmrcmemcpy(ControlKnobs, ControlKnobsBackup, sizeof(CONTROLKNOBS) * NumberControlKnobs);
    //
    // Must duplicate the results across all timings.
    //
    for (Sample=0; Sample < ControlKnobs[MrcData->NumberOfTiming].NumberSweeps; Sample++) {
      Mmrcmemcpy(ResultsV[Sample][1], ResultsV[Sample][0],  MAX_RANKS * MAX_KNOBS * MAX_CHANNELS_TIMES_STROBES * MAX_BITS * 2 * 2);
    }
    for (Loop=1; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
      Mmrcmemcpy(PICenter[Loop], PICenter[0], MAX_CHANNELS*MAX_RANKS*MAX_KNOBS*MAX_STROBES*MAX_BITS*2);
    }
    MrcData->NumberOfTiming = ControlKnobs[0].NumberOfTiming;
    VrefIndex = ControlKnobs[0].NumberOfTiming;
  }
  if (PerBitEnable == TRUE) {
    if (ControlKnobs[0].PBIndex != 0xffff) {
      SetPBDLinear ( MrcData, ControlKnobs[0].PBStartValue, ControlKnobs[0].PBIndex, ControlKnobs[0].PBSharedFlags, TRUE, NULL);
    }
  }

  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\n PHASE 3: FINAL SL CENTERING"));

  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nDisplay the Delay Swept Data at the bit granularity."));

  MrcData->LctNModeFlag = TRUE;
  SweepData1(MrcData, ResultsV, ControlKnobs[MrcData->NumberOfTiming].NumberSweeps, ControlKnobs, 0, VrefIndex, 0, 0, ResultsV, ControlKnobs[0].NumberSweeps, Max, FALSE);
  MrcData->LctNModeFlag = FALSE;
  //
  // Step 3b.  ReApply the VREF value.  Since the Vref does not have a per-bit, the PBIndex will be -1 for the knob.
  // Note that this is happenning AFTER the Vref sweep since it will be corrupted by that step.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nRestore Step2  Vref Centering"));
  for (Loop = 0; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
    Index[Loop] = ControlKnobs[VrefIndex].Index;
  }
  ApplySetting(MrcData, PICenterVref, SharedFlags, -1, Index, ControlKnobs[VrefIndex].PBIndex, ControlKnobs);
  //
  // Step 3c.  Re-sample and re-calculate the a more accurate Delay Centering. This approach uses the weighting method
  // outlined in the second centering approach.  Similar to the first estimation, it will calculate the per-bit first,
  // then calculate the strobe lane level.
  //
  if (PerBitEnable == TRUE) {
    if (ControlKnobs[0].PBSharedFlags != CompareFlag) {
      //
      // Calculate the center PI, this is an estimate only, and no matter what algorithm, the timing flags should be
      // Knob shared during this phase.  This algorithm, if bit-independent will computer the center per-bit and apply the per-bit
      // as a difference from the bit to the strobe-level max.
      //
      CalculateCenter2(MrcData, ResultsV, ControlKnobs[0].PBSharedFlags, ControlKnobs[0].NumberSweeps, PICenterPB, Max[0]);
      //
      // Apply The PICenter per-bit value only.  The Strobe level value does not need to be set at
      // this time.  During the Vref sweeping, the PI Byte-Center value will be set.
      //
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nFinal PerBit Pi Center."));
      for (Loop = 0; Loop < ControlKnobs[0].NumberOfTiming; Loop++) {
        Index[Loop] = -1;
      }
      ApplySetting(MrcData, PICenterPB, -1, ControlKnobs[0].PBSharedFlags, Index, ControlKnobs[0].PBIndex, ControlKnobs);
      //
      // Recalculate the center position, at this point, the center position should be the same for all bits.
      //
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nReStore Step1 Timing Centering (Required for P/N Training)."));

      for (Loop = 0; Loop < MrcData->NumberOfTiming; Loop++) {
        Index[Loop] = ControlKnobs[Loop].Index;
      }
      ApplySetting(MrcData, PICenter, ControlKnobs[0].SharedFlags|KNOB_SHARED, -1, Index, -1, ControlKnobs);

      MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nRestore Step2  Vref Centering"));
      for (Loop = 0; Loop < MrcData->NumberOfTiming; Loop++) {
        Index[Loop] = ControlKnobs[VrefIndex].Index;
      }
      ApplySetting(MrcData, PICenterVref, SharedFlags, -1, Index, ControlKnobs[1].PBIndex, ControlKnobs);
      MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nReSweep the data."));
      SweepData1(MrcData, ResultsV, ControlKnobs[MrcData->NumberOfTiming].NumberSweeps, ControlKnobs, 0, VrefIndex, 0, 0, ResultsV, ControlKnobs[0].NumberSweeps, Max, TRUE);
    }
  }
  //
  // Now re-calculate the PICenter but at the granularity of the StrobeLane, not the per-bit.  This is what is needed for the next
  // steps.
  //
  CalculateCenter2(MrcData, ResultsV, ControlKnobs[0].SharedFlags, ControlKnobs[0].NumberSweeps, PICenter, Max[0]);
  //
  // Step 3d.  Apply the new center values.
  //
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nPlacing Final SL Pi Center."));

  for (Loop = 0; Loop < MrcData->NumberOfTiming; Loop++) {
    Index[Loop] = ControlKnobs[Loop].Index;
  }
  ApplySetting(MrcData, PICenter, ControlKnobs[0].SharedFlags, -1, Index, -1, ControlKnobs);

 // CompleteJedecInit(MrcData,0,0,Loop);
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nPlacing Final Vref Center"));

  for (Loop = 0; Loop < MrcData->NumberOfTiming; Loop++) {
    Index[Loop] = ControlKnobs[VrefIndex].Index;
  }

  ApplySetting(MrcData, PICenterVref, SharedFlags, -1, Index, ControlKnobs[VrefIndex].PBIndex, ControlKnobs);
  MmrcDebugPrint ((MMRC_DBG_MIN, "\n\nResetting FIFOs"));
  for (Ch = 0; Ch < MAX_CHANNELS; Ch++) {
    for (Rk = 0; Rk < MAX_RANKS; Rk++) {
      PrechargeAll(MrcData, Ch, Rk);
      PerformDQReset(MrcData, Ch, Rk);
      PerformFifoReset(MrcData, Ch, Rk);
    }
  }
  //CADumpAndTest(MrcData, 0, 0, 0);
  //
  // Complete.
  //
  return MMRC_SUCCESS;
}

/**
  Using the LoopOnComposite(), this function performs 2 steps: Applies the per-bit setting, and the
  strobelane setting.  The PBFlags and Flags accordingly, if not -1, are used to determine if the
  appropriate step shoud be called.  It should be noted that the PBFlags and Flags are used if the
  seperate settings are indendent, but if the flags are set to bit-independent, then the flags could
  be used for per-bit setting.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Value         Array of values at the granulairty of Flags/PBFlags.
  @param[in]       Flags         Sharing level for the top level (strobes).
  @param[in]       PBFlags       Sharing level for the top level (per-bits).
  @param[in]       Index         GetSet Index for the top level (strobes).
  @param[in]       PBIndex       GetSet Per-bit Index for the top level (per-bits).
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ApplySetting (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 Value[MAX_TIMING][MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES][MAX_BITS],
  IN  INT8 Flags,
  IN  INT8 PBFlags,
  IN  INT16 Index[MAX_TIMING],
  IN  INT16 PBIndex,
  IN  CONTROLKNOBS *ControlKnobs
) {
  BuildCompositeParams  Param;
  UINT16                TempResults[MAX_TIMING][MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2];
  UINT16                StrobeVal[MAX_CHANNELS][MAX_RANKS][MAX_KNOBS][MAX_STROBES];
  INT8                  CompareFlag = 0xFF;
  //
  // Print the final resultant vectors that was selected.
  //
  {
    UINT8 i;
    UINT8 i2;
    UINT8 count;

    MmrcDebugPrint ((MMRC_DBG_MIN, "\nCRT"));
    count = 3;
    if ((Flags & STROBE_SHARED) == 0 || Flags == -1) {
    for (i=0; i< MrcData->MaxElements; i++) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "  SL%d", i));
      count+= 5;
      if ((PBFlags & BIT_SHARED) == 0) {
        for (i2=0; i2 < MAX_BITS; i2++) {
          MmrcDebugPrint ((MMRC_DBG_MIN, ":B%02d", i2));
          count+= 4;
        }
      }
    }
    }

    MmrcDebugPrint ((MMRC_DBG_MIN, "\n"));
    while (count > 0) {
      MmrcDebugPrint ((MMRC_DBG_MIN, "="));
      count--;
    }
  }
  //
  // If the flags are non-0xff, then indev/flags value are used to set the value.  Because this is not using
  // the Results or counter, the actual values being applied are passed in as the PI variable.
  //
  if (Flags != CompareFlag) {
    Param.ProgramSetGet.StrobeVal     = StrobeVal;
    Param.ProgramSetGet.FirstEntryFlag = TRUE;
    Param.ProgramSetGet.PI            = Value;
    Param.ProgramSetGet.Flags         = Flags;
    Param.ProgramSetGet.Index[0]      = Index[0];
    Param.ProgramSetGet.Index[1]      = Index[1];
    Param.ProgramSetGet.ControlKnobs  = ControlKnobs;
    BuildComposite(MrcData, Flags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, TempResults, &Param, ProgramSetGet);
  }
  //
  // If the PBflags are non-0xff, then PBindex/PBflags value are used to set the value.  Because this is not using
  // the Results or counter, the actual values being applied are passed in as the PI variable.  This is to set the
  // per-bit value.
  //
  if (PBFlags != CompareFlag) {
    Param.ProgramSetGet.StrobeVal     = StrobeVal;
    Param.ProgramSetGet.FirstEntryFlag = TRUE;
    Param.ProgramSetGet.PI            = Value;
    Param.ProgramSetGet.Flags         = PBFlags;
    Param.ProgramSetGet.Index[0]      = PBIndex;
    Param.ProgramSetGet.ControlKnobs  = ControlKnobs;
    BuildComposite(MrcData, PBFlags, MrcData->MaxElements, COMPOSITE_NONE, 1, 0, 0, TempResults, &Param, ProgramSetGet);
  }

  return MMRC_SUCCESS;
}

/**
  This functiongoes through all active channel and ranks, and for each knob, strobe, and bit prints out
  the results.  This is a full dump of all the active ranks, it does not look at the flags, it simply
  prints out all responses.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
PrintHiLow (
  IN  MMRC_DATA *MrcData,
  IN BuildCompositeParams *Param
) {
  UINT32 value;
  UINT8 Channel;
  UINT8 Rank;
  UINT8 Knob;
  UINT8 Strobe;
  UINT8 Bit;
  UINT8 HiLow;
  UINT8 MaxStrobe;
  UINT16 Total_Margin;

  MaxStrobe = MrcData->MaxNumberStrobes;

  for (Channel = 0; Channel< MrcData->MaxNumberChannels; Channel++) {
    for (Rank=0; Rank< MAX_RANKS; Rank++) {
      if (RunOnThisChannel (MrcData, Channel, Rank)) {
        for (Knob=0; Knob< MAX_KNOBS; Knob++) {
          for (HiLow=LOW; HiLow<=HIGH; HiLow++) {
            for (Strobe=0; Strobe < MrcData->MaxElements; Strobe++) {
              for (Bit=0; Bit< MrcData->MaxBits /*MAX_BITS*/; Bit++) { //Agnes
                //for (Timing = 0; Timing< MrcData->NumberOfTiming; Timing++) {
                if (Strobe == 0 && Bit == 0) {
                   MmrcDebugPrint ((MMRC_DBG_MIN, "\n%d%d%d%d", Channel, Rank, Param->PrintHiLow.ControlIndex, HiLow));
                }
                if (Bit == 0) {
                  GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex,
                    0xFF, Param->PrintHiLow.ControlKnobs[Param->PrintHiLow.Dim2Index].Index,
                    CMD_GET_REG, &value);
                  MmrcDebugPrint ((MMRC_DBG_MIN, "  %03d", value));

                  if ((MrcData->NumberOfTiming > 1) && (Param->PrintHiLow.Dim2Index < MrcData->NumberOfTiming)){
                    GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex,
                      0xFF, Param->PrintHiLow.ControlKnobs[(Param->PrintHiLow.Dim2Index)+1].Index,
                      CMD_GET_REG, &value);
                    MmrcDebugPrint ((MMRC_DBG_MIN, " - %03d", value));
                  }
                }
                MmrcDebugPrint ((MMRC_DBG_MIN, ":%03d",
                  Param->PrintHiLow.ResultsV[Param->PrintHiLow.Dim2Loop][Param->PrintHiLow.ControlIndex][Rank][Knob][(MaxStrobe * Channel)+ Strobe][Bit][HiLow]));

                //}
              } // for Bit
            } // for Strobe
          } // for HiLow
          //If LCT is enabled, check it against the total gaurdband which has been decided
          //to be -15,15 for CMD timing.Here we get the eye after Phase 3 so we can get total
         //margin but for individual margins, we need to subtract it from the delays set after
         //phase 1 which are neither retained in the cache or registers. Since asymetric margins
        //in 1N mode (occurs at non nominal voltages) will be low like 25, 30 but not more therefore
        //CMD_TIMING_GAURDBAND = 30. A proper funtioning 1N mode
       //should have greater than 30 as total margin. This logic needs to be outside the
       //HiLow loop because we need both Lo and Hi values. For Cmd training, Bit and Strobe will only loop once
#ifdef BXTPMRC
          if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) { //LP4 doesnt support 2N mode
            if (MrcData->LctDisable == FALSE) { //if LCT is enabled
              if (MrcData->LctNModeFlag == TRUE) { //perform this logic only for Phase 3
                Total_Margin = (Param->PrintHiLow.ResultsV[Param->PrintHiLow.Dim2Loop][Param->PrintHiLow.ControlIndex][Rank][Knob][(MaxStrobe * Channel) + (MrcData->MaxElements - 1)][MrcData->MaxBits - 1][1]) - (Param->PrintHiLow.ResultsV[Param->PrintHiLow.Dim2Loop][Param->PrintHiLow.ControlIndex][Rank][Knob][(MaxStrobe * Channel) + (MrcData->MaxElements - 1)][MrcData->MaxBits - 1][0]);
                if (Total_Margin < CMD_TIMING_GAURDBAND) {
                  MrcData->KeepAlive = 2;
                  MrcDeadLoop();
                }
              } //LctNModeFlag condition
            } //LCT condition
          } //Memorytype condition
#endif
        } // for Knob
      } // if RunOnThisChanel
    } // for Rank
  } // for Channel
  return MMRC_SUCCESS;
}

/**
  This function is called from the LoopOnComposite function.  It will adjust the value through
  the use of the GetSet.  The offset is taken as 70% of the maximum value.  Given that the default
  value is the center, when "loop" = 0, the delta is determined by taking 70% of the max value
  eyeWidth, dividing by 2, and subtracting from the current value.  All other loops, it simply adds
  one delta unit which is 70% max eyewidth / Number of samples.

  @param[in, out]  MrcData       Host structure for all data related to MMRC.
  @param[in]       Range         When building a composite, if composite RESULTS/COUNT is specific.
  @param[in]       Channel       Specific Channel being operated on.
  @param[in]       Rank          Specific Rank being operated on.
  @param[in]       Knob          Specific Lmpb being operated on.
  @param[in]       Strobe        Specific Strobe being operated on.
  @param[in]       Bit           Specific Bit being operated on.
  @param[in]       Param         Param structure that contains non-standard required inputs.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
ReAdjust (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN BuildCompositeParams *Param
) {
  INT32    DeltaOffset;
  UINT32   OrigValue;

  //
  // Based on loop, either set the delta to the first calculated value, or simply add one delta to value.
  //
  if (Param->ReAdjust.LoopTotal == 1) {
    return MMRC_SUCCESS;
  }
  if (Param->ReAdjust.Loop == 0) {
    DeltaOffset = (((Param->ReAdjust.Max[Timing][Channel][Rank][Knob][Strobe][Bit]*5)/10)/2 * -1);
    GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Param->ReAdjust.Index[Timing] + Knob, CMD_GET_REG, (UINT32 *)&OrigValue);
    if (DeltaOffset*-1 > (INT32) OrigValue) {
      DeltaOffset = -1 * OrigValue;
    }
  } else {
    DeltaOffset = ((Param->ReAdjust.Max[Timing][Channel][Rank][Knob][Strobe][Bit]*5)/10/(Param->ReAdjust.LoopTotal-1));
  }
  //
  // Set the value through the getset, note that if knobs are specificied, it simply gets added to the index.
  //
  GetSetDdrioGroup2 (MrcData, Channel, Rank, Strobe, Param->ReAdjust.Index[Timing] + Knob, CMD_SET_OFFSET, (UINT32 *)&DeltaOffset);
  PerformFifoReset(MrcData, Channel, Rank);
  return MMRC_SUCCESS;
}

/**
  This is the generic function that calls the Create1DSweep function.  It does three individual calls to the sweep
  function for small/medimum/large steps.  If the LastPastInValid flag, which was passed in, was set to false only the first
  "small" step will be run.  This is because the three individual tests are used to speed up the initial sweep only.
  The high/low/dim1startpoint are all passed in to the sweep function.  The 1DSweep only does 1 channel and 1 rank, so
  each of these sweeps are looped on every active channel/rank and are combined into the results array.

  @param[in, out]  MrcData         Host structure for all data related to MMRC.
  @param[in]       ControlKnobs    External array of controls for the algorithm.
  @param[in]       ControlIndex    Current control index being operated from ControlKnobs array.
  @param[in]       High            High range for the test to execute.
  @param[in]       Low             Low range for the test to execute.
  @param[in]       LastPassInValid Last time function was called, it was invalid, then do not do medium/large steps.
  @param[in, out]  Dim1StartPoint  The starting point and ranges for the 1dsweep.
  @param[out]      Results         Final results.
  @retval          MMRC_SUCCESS
**/
MMRC_STATUS
CompleteSweep (
  IN  MMRC_DATA       *MrcData,
  IN  CONTROLKNOBS    *ControlKnobs,
  IN  UINT16           ControlIndex,
  IN  UINT16           High[MAX_CHANNELS][MAX_STROBES],
  IN  UINT16           Low[MAX_CHANNELS][MAX_STROBES],
  IN  UINT8            LastPassInValid,
  IN  UINT16           Dim1StartPoint[MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][2],
  OUT UINT16           Results[MAX_RANKS][MAX_KNOBS][MAX_CHANNELS_TIMES_STROBES][MAX_BITS][2]
) {
  UINT8  Channel;
  UINT8  Rank;
  UINT8  Knob;
  UINT8  Strobe;
  UINT16 ResultsTemp[MAX_RANKS][MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][2];
  UINT8  BackupMrcDebugMsgLevel;
  UINT8  NumberKnobs;
  BOOLEAN PerBitSweep;
  UINT32 TempValue;
  UINT8 MaxStrobe;
  UINT8 BitIdx;
  UINT8 LastIdx;

  MaxStrobe = MrcData->MaxNumberStrobes;

  PerBitSweep = TRUE;
  //
  // Temporarily disable Messages to the screen.
  //
  BackupMrcDebugMsgLevel = MrcData->MrcDebugMsgLevel;
  MrcData->MrcDebugMsgLevel = 0;
  //
  // This is the first array, this is the only sweep that does not look at the LastPassValid as it must always be called.
  // When this is called, the params to the 1dSweep are all pre setup so the call is all that is needed.  After each call
  // the new starting points are loaded into the Dim1StartPoint based on the results.
  //
  if ((ControlKnobs[ControlIndex].SharedFlags & KNOB_SHARED) == KNOB_SHARED) {
      NumberKnobs = 1;
  }
  else {
      NumberKnobs = MAX_KNOBS;
  }

  if (ControlKnobs[ControlIndex].Step[0] != 0xffff) {
    //
    // Loop on all active channels and ranks.
    //
    for (Channel = 0; Channel < MrcData->MaxNumberChannels; Channel++) {
      for (Rank = 0; Rank < MAX_RANKS; Rank++) {
        if (RunOnThisChannel (MrcData, Channel, Rank)) {
          MrcData->CpgcOptions.Rank = Rank;
          MrcData->CpgcOptions.CmdSequence = CPGC_CMDSEQ_WRITE_READ;
          L_CpgcReconfig (MrcData, Channel);

          //
          // Do the sweep.
          //
          //NumStrobeLoops
          //hack, need to review
          if (MrcData->MaxBits == 1) {
            PerBitSweep = FALSE;
          }
          Create1DSweepLastPass (MrcData, Channel, Rank, ControlKnobs[ControlIndex].Index, NumberKnobs, Dim1StartPoint[Rank], Low, High, ControlKnobs[ControlIndex].Step[0], FALSE, FALSE, PerBitSweep, MrcData->MaxElements, ControlKnobs[ControlIndex].TestFunctionPointer, ResultsTemp[Rank], "", 0);

          if (MrcData->NumberOfTiming > 1 ) {
            //Create1DSweepLastPass (MrcData, Channel, Rank, ControlKnobs[ControlIndex].Index, NumberKnobs, Dim1StartPoint[Rank], Low, High, ControlKnobs[ControlIndex].Step[0], FALSE, FALSE, FALSE, 1, GenericCmdMemoryTest, ResultsTemp[Rank], "", 0);

            // Restore cmd and ctrl values
            if (ControlIndex < MrcData->NumberOfTiming ) {
                 for (Strobe = 0; Strobe < MrcData->MaxElements; Strobe++) {
                   GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, ControlKnobs[ControlIndex].Index, CMD_GET_CACHE, &TempValue);
                   GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, ControlKnobs[ControlIndex].Index, CMD_SET_VAL_FC, &TempValue);

                 }
              }
          }
          //
          // Set new starting points based on the results.
          //

          for (Knob= 0; Knob < MAX_KNOBS; Knob++) {
            for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
              Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW]  = (UINT16) (ResultsTemp[Rank][Channel][Knob][Strobe][0][LOW]);
              Dim1StartPoint[Rank][Channel][Knob][Strobe][HIGH]  = (UINT16) (ResultsTemp[Rank][Channel][Knob][Strobe][0][HIGH]);
            } // for St
          } // for Knob
        } // if RunOnThisChannel
      } // for Rank
    } // for Channel
  } // if Step[0]
  //
  // The second and third sweep are only called if the LastPastInValid was set to true.  Each of the individual sweeps themselves
  // are called if the appropriate steps were set.  If set to 0, then the sweep is not taken.  Similar to the first sweep, each
  // one is called multiple times, once for each active channel/rank combination.
  //
  if (LastPassInValid == 1) {
    //
    // Only do sweep if step[1] is non-zero.
    //
    if (ControlKnobs[ControlIndex].Step[1] != 0) {
      //
      // Loop through all active rank/channels.
      //
      for (Channel = 0; Channel < MrcData->MaxNumberChannels; Channel++) {
        for (Rank = 0; Rank < MAX_RANKS; Rank++) {
          if (RunOnThisChannel (MrcData, Channel, Rank)) {
            //
            // Call the sweep.
            //
            //NumStrobeLoops KIRKREDMAN
              //This has hardcoded test instead of pointing to the ControlKnob table.Get dev's consent before fixing it.
              Create1DSweepLastPass(MrcData, Channel, Rank, ControlKnobs[ControlIndex].Index, NumberKnobs, Dim1StartPoint[Rank], Low, High, ControlKnobs[ControlIndex].Step[1], FALSE, FALSE, TRUE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], GenericMemoryTest, ResultsTemp[Rank], "", 0);
            //
            // Set new starting points based on the results.
            //
            for (Knob= 0; Knob < MAX_KNOBS; Knob++) {
              for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
                Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW]   = (UINT16) (ResultsTemp[Rank][Channel][Knob][Strobe][0][LOW]);
                Dim1StartPoint[Rank][Channel][Knob][Strobe][HIGH]  = (UINT16) (ResultsTemp[Rank][Channel][Knob][Strobe][0][HIGH]);
              } // for Strobe
            } // for Kn
          } // if RunOnThisChannel
        } // for Rank
      } // for Channel
    } // if Step[1]
    //
    // Only do sweep if step[2] is non-zero.
    //
    if (ControlKnobs[ControlIndex].Step[2] != 0) {
      //
      // Loop through all active rank/channels.
      //
      for (Channel=0; Channel < MAX_CHANNELS; Channel++) {
        for (Rank = 0; Rank < MAX_RANKS; Rank++) {
          if (RunOnThisChannel (MrcData, Channel, Rank)) {
            //
            // Call the sweep.
            //
            //This has hardcoded test instead of pointing to the ControlKnob table.Get dev's consent before fixing it.
            //NumStrobeLoops
            Create1DSweepLastPass (MrcData, Channel, Rank, ControlKnobs[ControlIndex].Index, NumberKnobs, Dim1StartPoint[Rank], Low, High, ControlKnobs[ControlIndex].Step[2], FALSE, FALSE, TRUE, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank], GenericMemoryTest, ResultsTemp[Rank], "", 1);
            //
            // Set new starting points based on the results.
            //
            for (Knob= 0; Knob < MAX_KNOBS; Knob++) {
              for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
                Dim1StartPoint[Rank][Channel][Knob][Strobe][LOW]  = (UINT16) (ResultsTemp[Rank][Channel][Knob][Strobe][0][LOW]);
                Dim1StartPoint[Rank][Channel][Knob][Strobe][HIGH]  = (UINT16) (ResultsTemp[Rank][Channel][Knob][Strobe][0][HIGH]);
              } // for Strobe
            } // for Knob
          } // if RunOnThisChannel
        } // for Rank
      } // for Channel
    } // if Step[2]
  } // UnknownLastPass

  //
  // The results from the Create1DSweepLastPass or not i the normal [ch][rn][kn][st][bi], so the values must be taken
  // fromt he resultsTemp array and reordered correctly into the Results array.
  //
  for (Channel = 0; Channel < MrcData->MaxNumberChannels; Channel++) {
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (RunOnThisChannel (MrcData, Channel, Rank)) {
        for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
          for (BitIdx = 0; BitIdx < MAX_BITS; BitIdx++) {
            for (LastIdx = 0; LastIdx < 2; LastIdx++) {
              if ((ControlKnobs[ControlIndex].SharedFlags & KNOB_SHARED) == KNOB_SHARED) {
                Results[Rank][0][(MaxStrobe * Channel)+ Strobe][BitIdx][LastIdx] =  ResultsTemp[Rank][Channel][0][Strobe][BitIdx][LastIdx];
              } else {
                Results[Rank][0][(MaxStrobe * Channel)+ Strobe][BitIdx][LastIdx] =  ResultsTemp[Rank][Channel][0][Strobe][BitIdx][LastIdx];
              }
            }//LastIdx
          }//BitIdx
        }//Strobe
      } // if RunOnThisChannel
    } // for Rn
  } // for Channel
  //
  // Restore original Messages
  //
  MrcData->MrcDebugMsgLevel = BackupMrcDebugMsgLevel;

  return MMRC_SUCCESS;
}


MMRC_STATUS
GenericCmdMemoryTest(
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
  )
{

  UINT8         Knob;                 // Knob Index for currently active.

  UINT16        BytelaneStatus;
  BOOLEAN       AllBitsPassed;
  UINT32        CmdValue;
  UINT32        CtlValue;
  UINT32        CmdCacheValue;
  UINT32        CtlCacheValue;
  UINT8         tempCh;

  AllBitsPassed = TRUE;
  for (Knob = 0; Knob < NumberKnobs; Knob++)  {
    if ((NumberKnobs > 1) && (Knob == 0)) {
      MrcData->CpgcOptions.ChunkErrorMask = CPGC_CHUNK_EVEN;
    } else if ((NumberKnobs > 1) && (Knob == 1)) {
      MrcData->CpgcOptions.ChunkErrorMask = CPGC_CHUNK_ODD;
    } else {
      MrcData->CpgcOptions.ChunkErrorMask = 0xFF;
      ASSERT (NumberKnobs <= MAX_KNOBS);
    }

    PerformFifoReset(MrcData, Channel, Rank);

    //Since we are not doing VA rotation, just VA and running 1 CPGC test per margin step so its easier to start the test here, get results and read the DynamicVars directly
    MmrcExecuteTask(MrcData, CPGC_RUN_TEST, NO_PRINT, Channel);
    MmrcExecuteTask(MrcData, CPGC_GET_TEST_RESULT, NO_PRINT, Channel);


    //This test is only used for CMD and CTL training step so we should only check BytelaneStatus
    //if any byte fails then detect an edge
    BytelaneStatus = (UINT16)MrcData->DynamicVars[Channel][BYTEGRP_ERR_STAT];
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled) {
      BytelaneStatus |= (UINT16)(MrcData->DynamicVars[Channel][ECC_ERR_STAT] << 8);
    }

    if (BytelaneStatus != 0) {
      PassFail[Channel][0][0][0][CURRENT_SAMPLE] = RESULTS_FAIL; //Strobe=0, Bit=0 , No loop needed, just check the ByteErr
      AllBitsPassed = FALSE;
    } else {
      PassFail[Channel][0][0][0][CURRENT_SAMPLE] = RESULTS_PASS;
    }
  }//knob

  if (AllBitsPassed == FALSE) {
    //Store the current cmd and ctl value and set the ect trained value for cmd and ctl for dram reset
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CmdGrp0, CMD_GET_REG, (UINT32 *)(&CmdValue));
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CtlGrp0, CMD_GET_REG, (UINT32 *)(&CtlValue));

    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CmdGrp0, CMD_GET_CACHE, (UINT32 *)(&CmdCacheValue));
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CtlGrp0, CMD_GET_CACHE, (UINT32 *)(&CtlCacheValue));

    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CmdGrp0, CMD_SET_VAL_FC, &CmdCacheValue);
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CtlGrp0, CMD_SET_VAL_FC, &CtlCacheValue);

    //CADB should be disabled before we issue JEDEC RESET, we dont want to harrass the bus while sending JEDEC commands
   for (tempCh = 0; tempCh < MAX_CHANNELS; tempCh++) {
       if (!RunOnThisChannel (MrcData, tempCh, 0)) {
        MmrcDebugPrint ((MMRC_DBG_MAX, "MmrcExecuteTask !RunOnThisChannel tempCh=%d Channel=%d\n", tempCh, Channel));
        continue;
       }
       MmrcExecuteTask(MrcData, CPGC_DESELECT_DIS, NO_PRINT, tempCh);
       MmrcExecuteTask(MrcData, CPGC_STOP_TEST, NO_PRINT, tempCh);
   }
    PerformFifoReset(MrcData, Channel, Rank);
    CompleteJedecInit(MrcData, 0, 0, Channel);
    // If we disabled CADB for the JEDEC RESET, then restore CPGC settings if they were modified above

    //write back the cmd and ctl value to reg after jedec init because the Create1dsweep is using CMD_SET_OFFSET that reads from reg
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CmdGrp0, CMD_SET_VAL_FC, (UINT32 *)&(CmdValue));
    GetSetDdrioGroup2(MrcData, Channel, Rank, 0, CtlGrp0, CMD_SET_VAL_FC, (UINT32 *)&(CtlValue));

    MrcDelay(MrcData, MICRO_DEL, 100);
    PerformFifoReset(MrcData, Channel, Rank);
   for (tempCh = 0; tempCh < MAX_CHANNELS; tempCh++) {
      if (!RunOnThisChannel (MrcData, tempCh, 0)) {
        MmrcDebugPrint ((MMRC_DBG_MAX, "L_Cpgc !RunOnThisChannel tempCh=%d Channel=%d\n", tempCh, Channel));
        continue;
      }
      L_CpgcSetup(MrcData, tempCh, CPGC_CONFIG_CMD_STRESS);         //  L_CpgcSetup() works on the specific channel
      L_CpgcReconfig(MrcData, tempCh);        //  L_CpgcSetup() works on the specific channel
   }
  }

  return MMRC_SUCCESS;
}


MMRC_STATUS
GenericMemoryTest (
  IN  OUT   MMRC_DATA     *MrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN        UINT8         NumberElements,
  IN        UINT8         NumberKnobs,
  IN  OUT   UINT8         PassFail[MAX_CHANNELS][MAX_KNOBS][MAX_STROBES][MAX_BITS][3]
  )
{
  UINT8         Strobe;               // Strobe Index for currently active.
  UINT8         Bit;                  // Bit Index for currently active.
  UINT8         Knob;                 // Knob Index for currently active.
  UINT16        ByteErrorStatus0;
  UINT32        ErrorCount[9];
  UINT32        BitErrorStatus[3];
  UINT8         *BitErrorStatusByte;

  for (Knob = 0; Knob < NumberKnobs; Knob++) {
    if ((NumberKnobs > 1) && (Knob == 0)) {
      MrcData->CpgcOptions.ChunkErrorMask = CPGC_CHUNK_EVEN;
    } else if ((NumberKnobs > 1) && (Knob == 1)) {
      MrcData->CpgcOptions.ChunkErrorMask = CPGC_CHUNK_ODD;
    } else {
      MrcData->CpgcOptions.ChunkErrorMask = 0xFF;
    }
    ASSERT (NumberKnobs <= MAX_KNOBS);
    if (NumberKnobs > MAX_KNOBS) {
      return MMRC_FAILURE;
    }
    PerformFifoReset (MrcData, Channel, Rank);
    L_CpgcRunTest (MrcData, Channel, &ByteErrorStatus0, &BitErrorStatus[0], &BitErrorStatus[1], &BitErrorStatus[2], ErrorCount);
    BitErrorStatusByte = (UINT8 *)&BitErrorStatus[0];

    for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[Rank]; Strobe++) {
      for (Bit = 0; Bit < MAX_BITS; Bit++) {
        if (BitErrorStatusByte[Strobe] & (1<<Bit)) {
          PassFail[Channel][Knob][Strobe][Bit][CURRENT_SAMPLE] = RESULTS_FAIL;
        } else {
          PassFail[Channel][Knob][Strobe][Bit][CURRENT_SAMPLE] = RESULTS_PASS;
        }
      } // for Bits
    } // for Strobe
  } // for Knobs
  PerformFifoReset (MrcData, Channel, Rank);
  return MMRC_SUCCESS;
}

#endif // (WRITE_TRAINING == 1 || READ_TRAINING == 1 || COMMAND_TRAINING == 1) && TRAINING_ALGOS == 1


void CalculateMasterFloorplan(MMRC_DATA * MrcData, int Channel)
{
  int bl;
  for (bl=0; bl<MAX_STROBES; bl++)
  {
    MrcData->MasterFloorPlan[MrcData->CurrentBlueprint][Channel][bl].Strobelane = FloorPlan[Channel][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[bl*8]/8].Strobelane;
    MrcData->MasterFloorPlan[MrcData->CurrentBlueprint][Channel][bl].Channel = FloorPlan[Channel][MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping[bl*8]/8].Channel;
  }
}
/*

Dimm2DunitMapping[index] = value of proper bit location for index

[15] = 31
[14] = 30
[13] = 29

...

[7] = 3
[6] = 4
[5] = 2
[4] = 5
[3] = 1
[2] = 6
[1] = 0
[0] = 7

*/

//
//  Swizzle
//  Use this function to Swizzle data read back from DRAM
//
UINT32
Swizzle (
  IN MMRC_DATA     *MrcData,
  IN UINT8         Channel,
  IN UINT32        PreSwizzle      // Unswizzled data that needs to be swizzled
)
{
  UINT8 *Dimm2DunitMapping = (UINT8 *) MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitMapping;  // Single-dimensional array.  The index into the array is the swizzled location.  The array value is the unswizzled destination
  UINT32 Swizzled = 0;
  UINT8 CurrBit;

  for (CurrBit = 0; CurrBit < NUM_BITS_PER_DEVICE; CurrBit++)
  {
    if (Dimm2DunitMapping[CurrBit] > NUM_BITS_PER_DEVICE)
    {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Invalid Swizzle Value Found C%d, B%d\n", Channel, CurrBit));
      return PreSwizzle;
    }
    // If the current bit is set PreUnSwizzled, we need to set the bit in the new location.  Otherwise there's nothing to add
    if ((PreSwizzle & (1<<CurrBit)) != 0)
      Swizzled |= (1<<Dimm2DunitMapping[CurrBit]);
  }
  return (Swizzled);
}

//
//  DeSwizzle
//  Use this function to DeSwizzle data before sending to DRAM
//
UINT32
DeSwizzle (
  IN MMRC_DATA     *MrcData,
  IN UINT8         Channel,
  IN UINT32        PreUnSwizzle      // Swizzled value returned from DIMM
)
{
  UINT8 *Dimm2DunitReverseMapping = (UINT8 *) MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping;  // Single-dimensional array.  The index into the array is the swizzled location.  The array value is the unswizzled destination
  UINT32 Unswizzled = 0;
  UINT8 CurrBit;

  for (CurrBit = 0; CurrBit < NUM_BITS_PER_DEVICE; CurrBit++)
  {
    if (Dimm2DunitReverseMapping[CurrBit] > NUM_BITS_PER_DEVICE)
    {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Invalid Swizzle Value Found C%d, B%d\n", Channel, CurrBit));
      return PreUnSwizzle;
    }
    // If the current bit is set PreUnSwizzled, we need to set the bit in the new location.  Otherwise there's nothing to add
    if ((PreUnSwizzle & (1<<CurrBit)) != 0)
      Unswizzled |= (1<<Dimm2DunitReverseMapping[CurrBit]);
  }
  return (Unswizzled);
}

void CalculateDeswizzleReverseMap(
    IN MMRC_DATA     *MrcData,
    IN UINT8         Channel
)
{
  UINT8 CurrBit;
  UINT8 *Dimm2DunitMapping = (UINT8 *) MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitMapping;
  UINT8 *Dimm2DunitReverseMapping = (UINT8 *) MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitReverseMapping;

  for (CurrBit = 0; CurrBit < NUM_BITS_PER_DEVICE; CurrBit++)
  {
    Dimm2DunitReverseMapping[Dimm2DunitMapping[CurrBit]] = CurrBit;
    if (Dimm2DunitReverseMapping[CurrBit] > NUM_BITS_PER_DEVICE)
    {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Invalid Swizzle Entry C%d, M%d, B%d\n", Channel, Dimm2DunitMapping[CurrBit], CurrBit));
    }
    else
    {
      MmrcDebugPrint ((MMRC_DBG_MAX, "%d %d->%d->%d\n", Channel, CurrBit, Dimm2DunitMapping[CurrBit], Dimm2DunitReverseMapping[CurrBit]));
    }
  }
}

#define VOC_MIN      0
#define VOC_MAX      15
#define VOC_SAMPLES  10
#define VOC_VREF_CNT 3
/*

  TODO: Add the Settling Delays

*/



//
//  Applies one bytelanes "swizzle" to the data passed in
//
UINT8 SwizzleBytelane(MMRC_DATA * MrcData, UINT8 ch, UINT8 bl, UINT8 InData)
{
  int bit;
  UINT8 * Mapping;
  UINT8   odata=0;
  if (ch<MAX_CHANNELS)
    Mapping = MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Dimm2DunitMapping;
  else
    return 0xCD;
  if (bl >= MAX_STROBES)
    return 0xFE;
  for (bit=0; bit < 8; bit++)
  {
    //
    //  If the particular bit we are interested in is a 1 in the InData, set the corresponding swizzled bit in the odata
    //
    if ((InData & (1 << bit)) != 0)
      odata |= (1 << (Mapping[(bl*8) +bit] %8));
  }
  return odata;
}
//
//  Applies one bytelanes "deswizzle" to the data passed in
//
UINT8 DeSwizzleBytelane(MMRC_DATA * MrcData, UINT8 ch, UINT8 bl, UINT8 InData)
{
  int bit;
  UINT8 * Mapping;
  UINT8   odata=0;
  if (ch<MAX_CHANNELS)
    Mapping = MrcData->NvData.MrcParamsSaveRestore.Channel[ch].Dimm2DunitReverseMapping;
  else
    return 0xCD;
  if (bl >= MAX_STROBES)
    return 0xFE;
  for (bit=0; bit < 8; bit++)
  {
    //
    //  If the particular bit we are interested in is a 1 in the InData, set the corresponding swizzled bit in the odata
    //
    if ((InData & (1 << bit)) != 0)
      odata |= (1 << (Mapping[(bl*8) +bit] %8));
  }
  return odata;
}

MMRC_STATUS
StoreDelay (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  UINT32 Value;
  UINT8  Ti;

  for (Ti = 0; Ti < Param->StoreRestore.NumberTimings; Ti++) {
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, Param->StoreRestore.ControlIndex[Ti], CMD_GET_REG, &Value);
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, Param->StoreRestore.ControlIndex[Ti], UPD_CACHE, &Value);
  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
RestoreDelay (
  IN  MMRC_DATA *MrcData,
  IN  UINT16 *Range,
  IN  UINT8 Channel,
  IN  UINT8 Rank,
  IN  UINT8 Knob,
  IN  UINT8 Strobe,
  IN  UINT8 Bit,
  IN  UINT8 Timing,
  IN  BuildCompositeParams *Param
) {
  UINT32 Value;
  UINT8  Ti;

  for (Ti = 0; Ti < Param->StoreRestore.NumberTimings; Ti++) {
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, Param->StoreRestore.ControlIndex[Ti], CMD_GET_CACHE, &Value);
    GetSetDdrioGroup2 (MrcData, Channel,  Rank, Strobe, Param->StoreRestore.ControlIndex[Ti], CMD_SET_VAL_FC_UC, &Value);
  }
  return MMRC_SUCCESS;
}

/**
  Returns the requested timing for the current MrcData->TimingParamsConfig

  @param[in]   TimingType   Requested timing index.

  @retval      UINT16       Requested timing value
**/
UINT16
GetTimingParam (
  TIMING_DATA_TYPE TimingType
  )
{
  MMRC_DATA * MrcData = GetMrcHostStructureAddress ();
  UINT32      i;
  //
  // Search through the timing tables for the requested timing
  //
  for (i = 0; i < TimingParamsUnder256Size; i++) {
    if (TimingParamsUnder256[i][0] == TimingType) {
      return TimingParamsUnder256[i][MrcData->MrcTimingProfile];
    }
  }
  for (i = 0; i < TimingParamsOver255Size; i++) {
    if (TimingParamsOver255[i][0] == TimingType) {
      return TimingParamsOver255[i][MrcData->MrcTimingProfile];
    }
  }

  //
  // If the lookup fails, return 0xFFFF
  //
  MmrcErrorPrint ((MMRC_DBG_MIN, "GetTimingError\n"));
  return 0xFFFF;
}

//
// BgfCustomTsv[]
// Values defined in the BXT Memory HAS:
//   Section 3.2.2.4: Custom TSV Patterns
//   Table 3-7: BGF Custom TSV Programming
//
TSV_DATA BgfCustomTsv[] = {
  { 200, 200, 0x7,      0x7,      0x7,      0x7    },
  { 200, 233, 0x3f,     0x3f,     0x7d,     0x7d   },
  { 200, 266, 0xf,      0xe,      0xe,      0xe    },
  { 200, 300, 0x37,     0x36,     0x36,     0x36   },
  { 200, 333, 0x1b,     0x1a,     0x16,     0x16   },
  { 200, 400, 0x2b,     0x2a,     0x2a,     0x2a   },
  { 200, 466, 0x4b,     0x4a,     0x52,     0x52   },
  { 250, 200, 0x1d,     0x1d,     0xf,      0xf    },
  { 250, 233, 0x7fef,   0x7fef,   0x77ff,   0x77ff },
  { 250, 266, 0xefff,   0xefff,   0xffef,   0xffef },
  { 250, 300, 0x1f,     0x1f,     0x3d,     0x3d   },
  { 250, 333, 0x7,      0x7,      0xd,      0xd    },
  { 250, 400, 0xdb,     0xda,     0xb6,     0xb6   },
  { 250, 466, 0x0,      0x0,      0x0,      0x0    },
  { 267, 200, 0xe,      0xe,      0xf,      0xe    },
  { 267, 233, 0xfb,     0xfb,     0xbf,     0xbf   },
  { 267, 266, 0x7,      0x7,      0x7,      0x7    },
  { 267, 300, 0x17f,    0x17f,    0x1fb,    0x1fb  },
  { 267, 333, 0xf,      0xf,      0x1d,     0x1d   },
  { 267, 400, 0x37,     0x36,     0x36,     0x36   },
  { 267, 466, 0x6b,     0x6a,     0x56,     0x56   },
  { 333, 200, 0x16,     0x16,     0x1b,     0x1a   },
  { 333, 233, 0x36e,    0x36e,    0x3b7,    0x3b6  },
  { 333, 266, 0x1d,     0x1d,     0xf,      0xf    },
  { 333, 300, 0x3f7,    0x3f7,    0x37f,    0x37f  },
  { 333, 333, 0x7,      0x7,      0x7,      0x7    },
  { 333, 400, 0x2f,     0x2f,     0x3b,     0x3b   },
  { 333, 466, 0x37,     0x37,     0x6d,     0x6d   },
  { 400, 200, 0x2a,     0x2a,     0x2b,     0x2a   },
  { 400, 233, 0xb56,    0xb56,    0xd5b,    0xd5a  },
  { 400, 266, 0x36,     0x36,     0x37,     0x36   },
  { 400, 300, 0xd,      0xd,      0x7,      0x7    },
  { 400, 333, 0x3b,     0x3b,     0x2f,     0x2f   },
  { 400, 400, 0x7,      0x7,      0x7,      0x7    },
  { 400, 466, 0x6f,     0x6f,     0x77,     0x77   }
};

//
// DataBgfCustomTsvForDynamicEarlyWarning[]
// Values defined in the BXT Memory HAS:
//   Section 3.2.2.4: Custom TSV Patterns
//   Table 3-8: Data BGF Custom TSV Programming for Dynamic Early Warning
//
TSV_DATA DataBgfCustomTsvForDynamicEarlyWarning[] = {
  { 200, 233, 0x6f,   0x6f,   0x3e,   0x3f   },
  { 200, 266, 0xb,    0xb,    0xa,    0xb    },
  { 200, 300, 0x2d,   0x2d,   0x2c,   0x2d   },
  { 200, 333, 0x16,   0x16,   0x14,   0x15   },
  { 250, 266, 0xfbff, 0xfbff, 0xfffb, 0xfffb },
  { 250, 300, 0x37,   0x37,   0x1e,   0x1f   },
  { 250, 333, 0xd,    0xd,    0x6,    0x7    },
  { 250, 400, 0xb6,   0xb6,   0xac,   0xad   },
  { 267, 300, 0x1df,  0x1df,  0x1fe,  0x1fe  },
  { 267, 333, 0x1b,   0x1b,   0x3e,   0xf    },
  { 267, 400, 0x2d,   0x2d,   0x2c,   0x2d   },
  { 267, 466, 0x5a,   0x5a,   0x54,   0x55   },
  { 333, 400, 0x3b,   0x3b,   0x3e,   0x3e   },
  { 333, 466, 0x6d,   0x6d,   0x3a,   0x3b   },
  { 400, 466, 0x7b,   0x7b,   0x7d,   0x7d   }
};

UINT8 BgfCustomTsvSize = (sizeof(BgfCustomTsv) / sizeof(BgfCustomTsv[0]));
UINT8 DataBgfCustomTsvForDynamicEarlyWarningSize = (sizeof(DataBgfCustomTsvForDynamicEarlyWarning) / sizeof(DataBgfCustomTsvForDynamicEarlyWarning[0]));

/**
  Returns TRUE if a given "CheckValue" is within +/- "Range" if the input BaseValue

  @param[in]   BaseValue    The base value to use in the range check
  @param[in]   CheckValue   The value to check against the base value +/- the range
  @param[in]   Range        The range to use in the range check.

  @retval      BOOLEAN      TRUE if a given "CheckValue" is within +/- "Range" if the input BaseValue
**/
BOOLEAN
InRange(
  UINT32 BaseValue,
  UINT32 CheckValue,
  UINT32 Range
  )
{
  return ((CheckValue <= (BaseValue + Range)) && (CheckValue >= (BaseValue - Range)));
}

/**

Routine Description:

  This function returns the maximum and minimum frequencies supported by the SoC

  @param[in,out]  UINT8 *FreqHigh    Buffer to populate with Maximum supported frequency

  @retval MMRC_SUCCESS

**/
MMRC_STATUS
GetSupportedFrequencies (
  IN        MMRC_DATA *MrcData,
  IN  OUT   UINT16    *FreqHigh
)
{
#if BXTPMRC
  UINT8 DdrType;
  MMRC_STATUS Status;
  UINT8 FreqProfile;
  P_CR_MEMSS_FREQUENCY_CAPABILITIES_0_0_0_MCHBAR_PUNIT_BOXTYPE_STRUCT Mem_Freq_Cap;
  P_CR_MEMSS_FREQUENCY_CAPABILITIES1_0_0_0_MCHBAR_PUNIT_BOXTYPE_STRUCT Mem_Freq_Cap1;
  UINT16 Multiple133_FreqHigh;

  DdrType = 0;
  Status = MMRC_SUCCESS;
  FreqProfile = 0;

  DdrType = ConfigToDdrType[MrcData->MrcMemConfig];
  // Get Fuse high and low freq.
  Mem_Freq_Cap.Data = Mmio32Read(MrcData->MchBar + P_CR_MEMSS_FREQUENCY_CAPABILITIES_0_0_0_MCHBAR_PUNIT_BOXTYPE_REG);
  Mem_Freq_Cap1.Data = Mmio32Read(MrcData->MchBar + P_CR_MEMSS_FREQUENCY_CAPABILITIES1_0_0_0_MCHBAR_PUNIT_BOXTYPE_REG);
  if (DdrType == TypeDdr3L) {
    Multiple133_FreqHigh = (UINT16) Mem_Freq_Cap1.Bits.ddr3l_freq_high;
    Status = TimingParamHash (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile, &FreqProfile);
      if (FreqProfile < MrcData->MrcMemConfig) {
      MrcData->MrcMemConfig = FreqProfile;
    }
  } else if (DdrType == TypeLpDdr4) {
    Multiple133_FreqHigh = (UINT16) Mem_Freq_Cap.Bits.lp4_freq_high;
  } else if (DdrType == TypeLpDdr3) {
    Multiple133_FreqHigh = (UINT16) Mem_Freq_Cap.Bits.lp3_freq_high;
  } else {
    Multiple133_FreqHigh = 0;
  }
  *FreqHigh = ConvertMultiple133toSpeed[Multiple133_FreqHigh];
  MmrcDebugPrint((MMRC_DBG_MIN, "FREQ_HIGH: %d\n", Multiple133_FreqHigh));
  return Status;
#else //BXTPMRC
  return MMRC_SUCCESS;
#endif
}

/**
  Returns TRUE if the current boot mode is a restore path

  @param[in]  MrcData  Host structure for all MRC global data.

  @retval  TRUE
  @retval  FALSE
**/
BOOLEAN
InResorePath (
  IN  MMRC_DATA   *MrcData
)
{
  if ((MrcData->BootMode & S0) == S0 ||
      (MrcData->BootMode & S0W) == S0W ||
      (MrcData->BootMode & S0C) == S0C ||
      (MrcData->BootMode & FB) == FB ||
      (MrcData->BootMode & S3) == S3)
  {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Returns the Dividend divided by the Divisor rounded to the nearest integer.

  @param[in]       Dividend
  @param[in]       Divisor
  @retval          Rounded Division Result
**/
UINT32
DivRoundU32x32 (
  IN      UINT32                    Dividend,
  IN      UINT32                    Divisor
  )
{
  UINT32 Result = Dividend / Divisor;
  UINT32 Remainder = Dividend % Divisor;
  return (Remainder >= ((Divisor + 1) / 2) ? (Result + 1) : Result);
}
