/** @file
  Memory Configuration.

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

#include "ConfigMem.h"
#include "ConfigMemData.h"
#include "IchRegs.h"

#include "OemHooks.h"
#include "McFunc.h"
#include "DetectDimms.h"
#include "Mailbox.h"
#include "MmrcHooks.h"
#include "MmrcLibraries.h"
#include "MmrcProjectData.h"
#include "MrcFunc.h"
#if EFI_MEMORY_INIT
#include <Library/SteppingLib.h>
#endif


/**
  Initialize Punit setting

  @param[in,out]  MrcData

  @retval  None
**/
MMRC_STATUS
InitPunitSetting (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  return MMRC_SUCCESS;
}

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif
/**
  LockBarAddress

  @param[in]  MrcData

  @retval  None
**/
VOID LockBarAddress
  (MMRC_DATA         *MrcData )
{
#ifndef SIM
    MmrcExecuteTask (MrcData, LockAunitSliceChannel, NO_PRINT, 0);
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
  "\n\t pushl  %eax"
  "\n\t pushl  %edx"
  "\n\t pushl  %ecx"
  "\n\t movl   $0x00, %eax"
  "\n\t movl   $0x00, %edx"
  "\n\t movl   $0x122, %ecx"
  "\n\t rdmsr"
  "\n\t orl    $0x80000000, %edx"
  "\n\t wrmsr"
  "\n\t popl   %ecx"
  "\n\t popl   %edx"
  "\n\t popl   %eax"
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
    or  edx, 80000000h
    wrmsr
    pop ecx
    pop edx
    pop eax
  }
#endif
#endif //SIM
}
#ifdef __GNUC__
#pragma GCC pop_options
#endif

/**
  Returns TRUE if the bootmode if not a restore path and the MRC fast boot data
  has been loaded.

  @param[in]  MrcData  Host structure for all MRC global data.

  @retval  TRUE
  @retval  FALSE
**/
BOOLEAN
MrcDataStructDirty(
  IN  MMRC_DATA   *MrcData
)
{
  //
  // MrcData->MrcParamsValidFlag will be set to 1
  // if the MRC NV data was previously loaded
  //
  if (MrcData->MrcParamsValidFlag) {
    if (!InResorePath(MrcData)){
    //
    // If the MRC NV data was previously loaded and the boot mode is not a restore path
    // then return TRUE since the structure is "dirty" with data from the previous boot.
    //
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Configure the memory thru several training steps

  @param[in,out]  MrcData  Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
ConfigureMemory (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  MMRC_STATUS        Status;
  UINT32             TempRegData;

#if MINIBIOS
  UINT32             Address;
  UINT32             Data;
  UINT32             i;
#endif
  TempRegData = 0;


  Status = MMRC_SUCCESS;

  ///
  /// Some routines require the MrcData host structure but we don't pass the structure
  /// to them for several reasons:
  ///   1) To save code space - 1 less parameter per function call on routines which
  ///      are frequently called.
  ///   2) Macros such as MrcDeadLoop() are able to be used from any routine. If a
  ///      routine does not have MrcData as a parameter, it can't pass it to MrcDeadLoop().
  ///
  SaveMrcHostStructureAddress (MrcData);


  ///
  /// Initialize other components such as UART, PUNIT, HPET, etc.
  ///
  ExternalSiliconInit (MrcData);

  MrcData->MrcMemConfig = 0;
  MrcData->MrcTimingProfile = 0;
  MrcData->TimingParamsConfig = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile;
  Status = TimingParamHash ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile), &(MrcData->MrcMemConfig));

  if (Status != MMRC_SUCCESS) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "TimingParams Error: %d\n", MrcData->MrcMemConfig));
    return MMRC_FAILURE;
  }

  ///
  ///
  /// Populate all input parameters that MMRC needs to execute.
  ///
//[-start-151224-IB07220029-add]//
  if ((UINTN)MmrcEntry > 0xFE000000)
//[-end-151224-IB07220029-add]//
  GetInputParameters (MrcData);
  ///
  /// Convert input parameters to actual values which will be written
  /// into registers. For example, if TCL of 11 needs a value of 6
  /// written to the DUNIT, this routine will change TCL to 6.
  ///
  //ConvertInputParameters (MrcData);

  PrintMemoryConfig (MrcData);

  //
  // FALSE = Start memory initialization
  //
#if !SIM && !JTAG
  RecordMrcStatus(MrcData, FALSE);
#endif

  ///
  /// Single entry point to MMRC to execute the entire flow.
  ///
  if (MrcDataStructDirty(MrcData)) {
    //
    // If this is an S5 boot and the MRC Data structure is loaded with data from a
    // previous boot, we need to re-initialize the structure to make sure there's
    // no stale data being passed in to the MMRC. This is a fallback in case we've
    // already loaded the FB data but need to do perform an S5 boot.
    // Situations where this path is excercised:
    // - SODIMM configuration is changed from first boot
    // - Previous boot failed and DramInit flag was not cleared
    //
    return MMRC_DATA_DIRTY;
  }
//[-start-151224-IB07220029-add]//
  if ((UINTN)MmrcEntry > 0xFE000000)
//[-end-151224-IB07220029-add]//
  Status = MmrcEntry (MrcData, EntireMmrc, NULL);

  if (Status != MMRC_SUCCESS) {
    return Status;
  }
  FillOutputStructure (MrcData);
  //
  // If we're in the MiniBIOS, MRC has completed. Now need to deadloop and optionally
  // display all delays and dump all registers.
  //

  //
  // Disables the High Precision Event Timer
  //
  McEnableHpet (MrcData);

#ifdef BXTPMRC
//[-start-151224-IB07220029-add]//
  if ((UINTN)MmrcEntry > 0xFE000000)
//[-end-151224-IB07220029-add]//
  SetSliceChannelHash (MrcData);
//[-start-151224-IB07220029-add]//
  if ((UINTN)MmrcEntry > 0xFE000000)
//[-end-151224-IB07220029-add]//
  LockBarAddress (MrcData);
#endif

#if !SIM && !JTAG
  RecordMrcStatus(MrcData, TRUE);
#endif

  return Status;
}

/**
  Mark MRC as started so that if a reset occurs during the middle of MRC,
  we know the memory subsystem is partially initialized. In this case, we
  need to force a powergood reset to get the DDRIO back into a reset state
  where the registers are at default values.

  @param[in,out]  MrcData      Host structure for all MRC global data.
  @param[in]      Finish

  @retval  Success
  @retval  Failure
**/
VOID
RecordMrcStatus (
  IN  OUT   MMRC_DATA   *MrcData,
  IN  BOOLEAN           Finish
  )
{
  UINT32 PmcMrcStartedIndication ;
  //
  // Read PMC GCR PMCON1 register and clear dram init bit when complete. Set dram init bit to indicate MRC is not complete/in progress.
  //
  PmcMrcStartedIndication  = Mmio32Read(MrcData->PmcBar + PMC_GCR_BASE_ADDRESS + PMC_GCR_GEN_PMCON1);

  //
  // Ensure that we don't clear bits that are write-once in the GEN_PMCON1 register
  //
  PmcMrcStartedIndication &= (~BIT18 & ~BIT20 & ~BIT24 & ~BIT25 & ~BIT26 & ~BIT27);

  if (Finish) {
    Mmio32Write (MrcData->PmcBar + PMC_GCR_BASE_ADDRESS + PMC_GCR_GEN_PMCON1, (PmcMrcStartedIndication  & ~PMC_GCR_GEN_PMCON1_DRAM_INIT_BIT_STS));
  } else {
    Mmio32Write (MrcData->PmcBar + PMC_GCR_BASE_ADDRESS + PMC_GCR_GEN_PMCON1, (PmcMrcStartedIndication  | PMC_GCR_GEN_PMCON1_DRAM_INIT_BIT_STS));
  }
}
/**
  Routine to initialize other pieces of silicon which may be required
  for MRC to successfully initialize memory. High precision event timer,
  PUNIT mailbox, etc.

  @param[in,out]  MrcData       Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
ExternalSiliconInit (
  IN  OUT   MMRC_DATA   *MrcData
)
{
#ifndef SIM
#ifdef MINIBIOS
#if DEBUG_MSG
 // Init16550Uart ();
#endif
#endif
  ///
  /// Get platform setup including all BAR address
  ///
  if (GetPlatformSettings (MrcData) == MMRC_FAILURE) {
    MrcDeadLoop ();
  }
  ///
  /// Enables the High Precision Event Timer
  ///
  McEnableHpet (MrcData);
  ///
  /// Initialize Punit Setting
  ///
  InitPunitSetting(MrcData);
#endif // SIM

  return MMRC_SUCCESS;
}

//
// DramDensity[]
// Used to convert the index value of MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] into
// a decimal value in units of gigabits (Gb)
//
UINT8 DramDensity[MaxMemoryDeviceDensity] = {
  DRAM_DENSITY_4Gb,  // 0 = 4Gb
  DRAM_DENSITY_6Gb,  // 1 = 6Gb
  DRAM_DENSITY_8Gb,  // 2 = 8Gb
  DRAM_DENSITY_12Gb, // 3 = 12Gb
  DRAM_DENSITY_16Gb  // 4 = 16Gb
};

/**
  Calculates and returns the rank size in megabytes for a given channel

  @param[in]  MrcData       Host structure for all data related to MMRC
  @param[in]  Channel       Target channel for rank size calculation

  @retval  RankSize         Rank size in MB for a given channel
**/
UINT16 CalculateRankSize(
  IN  MMRC_DATA *MrcData,
  IN  UINT8     Channel
  )
{
  UINT8  DQBusWidth;
  // 0 = x8; 1 = x16; 2 = x32
  UINT8  DeviceWidth;
  //  0000 =  4 Gb 0001 = 6 Gb  0010 = 8 Gb  0011 = 12 Gb
  //  0100 = 16 Gb
  UINT16 DramCapacity;
  UINT16 RankSize;
  UINT8  DramDensityInGb;
  BXT_SERIES BxtSeries;

  BxtSeries = MrcData->BxtSeries;

  DQBusWidth   = 8;
  DeviceWidth  = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0];
  DramCapacity = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0];

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
    DQBusWidth = 32;
    //
    // The LPDDR3 Density is reported per rank. The total device width
    // is always x32 in this case
    //
    DeviceWidth = 2;
  } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    if ( BxtSeries == Bxt1 ) {
      DQBusWidth = 16;
    } else {
      DQBusWidth = 32; //  32bit DQ bus (DRAMdevice_PerRank * device width)
    }
    //
    // The LPDDR4 Density is reported per rank. The total device width
    // is always x32 in this case
    //
    DeviceWidth = 2;
  } else {
    DQBusWidth = 64;
  }

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0] != 0) {
    DramDensityInGb = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0];
  } else {
    DramDensityInGb = DramDensity[DramCapacity];
  }

  RankSize = (DramDensityInGb * 1024) / 8 *
        (DQBusWidth) /
        (8 << DeviceWidth );

  //MmrcDebugPrint((MMRC_DBG_MIN, "RankSize = %d, DQBusWidth = %d, DeviceWidth = %d, DramDensity[%d] = %d | ", RankSize, DQBusWidth, DeviceWidth, DramCapacity, DramDensity[DramCapacity]));
  return RankSize;
}

/**
  Sets the DRAM density fields and automatically applies software workarounds
  for hardware unsupported densities.

  @param[in,out]  MrcData       Host structure for all data related to MMRC
  @param[in]      Channel       The channel to preform the update on
  @param[in]      DramDensity   The requested density for the DRAM
  @param[in]      DramWidth     The reported width of the DRAM
  @param[in]      RankEnable    A bit mask indicating what ranks are enabled

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
SetDramDensity (
  IN  OUT   MMRC_DATA                *MrcData,
  IN        UINT8                    Channel,
  IN        SMIP_DRAM_DEVICE_DENSITY DramDensity,
  IN        SMIP_DRAM_DEVICE_WIDTH   DramWidth,
  IN        UINT8                    RankEnable
  )
{
  if ( MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3 &&
       DramDensity == SMIP_DramDensity8Gb &&
       DramWidth == SMIP_DramWidthx16 &&
       RankEnable == (BIT0|BIT1)) {
    //
    // Special case for 16Gb QDP (4 x 4Gb x16) Devices
    // Treat these devices as 32Gb DRAMs with 16Gb sized ranks
    //
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity16Gb;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0] = DRAM_DENSITY_8Gb;
    return MMRC_SUCCESS;
  }

  //
  // Supported DrpPolicy.DramDensity inputs:
  // 0 = 4Gb
  // 1 = 6Gb
  // 2 = 8Gb
  // 3 = 12Gb
  // 4 = 16Gb
  // 5 = 2Gb
  //

  if (DramDensity == SMIP_DramDensity2Gb && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity4Gb;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0] = DRAM_DENSITY_2Gb;
  } else if (DramDensity == SMIP_DramDensity4Gb && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity8Gb;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0] = DRAM_DENSITY_4Gb;
  } else if (DramDensity == SMIP_DramDensity6Gb && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = DeviceDensity8Gb;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_SizeActual[0] = DRAM_DENSITY_6Gb;
  }
  else {
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Size[0] = (DramDensity & 0x7);
  }
  return MMRC_SUCCESS;
}

/**
  Fills in the complete parameter list for the Modular MRC.
  This includes all the Dynamic entries that are listed in
  the Input Spreadsheet, but also the PFCT values.
  Want to

  @param[in,out]  MrcData       Host structure for all data related to MMRC

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
  GatherSocConfigs (
  IN  OUT   MMRC_DATA            *MrcData
  )
{
  UINT8               Channel;
  UINT8               i;
  UINT8               Tcl;
  MMRC_STATUS         Status;
  UINT32              ChEnMask;
  UINT8               TempProfile;
  UINT8               NewProfile;

  Status = MMRC_SUCCESS;
  NewProfile = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile;
  ChEnMask = 0;

  //
  // Get SMIP setting and convert it to MRC Data
  //
  if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_SODIMM) {
    // Set DDR3L profile for SODIMM

    Tcl = (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled) ? MrcData->NvData.MrcParamsSaveRestore.Channel[0].Tcl : MrcData->NvData.MrcParamsSaveRestore.Channel[1].Tcl;

    switch (MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency) {
    case MMRC_1600:
      NewProfile = DDR3_1600_8_8_8;
      i = 9;
      break;
    case MMRC_1866:
      NewProfile = DDR3_1866_10_10_10;
      i = 11;
      break;
    case MMRC_1333:
    default:
      NewProfile = DDR3_1333_7_7_7;
      i = 8;
      break;
    }

    while (i++ <= Tcl)  {
      NewProfile += 1;
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "NewProfile is %d freq is %d\n", NewProfile, MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency ));
  }

  MrcData->TimingParamsConfig = NewProfile;
  TempProfile = MrcData->MrcMemConfig;
  if ((NewProfile < MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile) && (ConfigToDdrType[TempProfile] == TypeDdr3L)) {
    Status = TimingParamHash (MrcData->TimingParamsConfig, &(MrcData->MrcTimingProfile));
    Status = TimingParamHash (NewProfile, &(MrcData->MrcMemConfig));
  }

#ifdef BXT_PERIODIC_ENABLED
    MrcData->DynamicVars[0][BXT_PERIODIC_ENABLED]=1;
    MrcData->DynamicVars[1][BXT_PERIODIC_ENABLED]=1;
    MrcData->DynamicVars[2][BXT_PERIODIC_ENABLED]=1;
    MrcData->DynamicVars[3][BXT_PERIODIC_ENABLED]=1;
#endif

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown != MD_SODIMM) {
      if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].RankEnable != 0) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled = TRUE;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DimmPresent[0] = 1;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0] = (BOOLEAN) MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].RankEnable & 0x1;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1] = (BOOLEAN)((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].RankEnable >> 1) & 0x1);
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_BusWidth[0] = 1;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0] = (UINT8) (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].DeviceWidth & 0x3);// 1;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType = (UINT8) ConfigToDdrType[MrcData->MrcMemConfig];
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ASR_Supported = (UINT8)((SMIP_DDR3LASR_OPTION *)(&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.DDR3LASR))->Bits.ASR_Supported;
        SetDramDensity(
          MrcData,
          Channel,
          MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].DramDensity,
          MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].DeviceWidth,
          MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].RankEnable
          );
      }
    }

    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].OdtHigh = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].OdtLevels;
    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].ChOption.Data = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Channel].Option;

    if ((MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0] || MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1]) == 0){
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled = FALSE;
    }

    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0] && MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[1]) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[0] = 2;
        MmrcDebugPrint((MMRC_DBG_MIN, "Ch[%d] 2 ranks\n", Channel));
      } else {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[0] = 1;
        MmrcDebugPrint((MMRC_DBG_MIN, "Ch[%d] 1 rank\n", Channel));
      }
    }
  }

  MrcData->DigitalDll = DIGITALDLL;

  // Enable 2nd channel to configure PHY for 1x32 lpddr4 config
  if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeLpDdr4) {
    if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_1X32_LPDDR4) {
      MrcData->NvData.MrcParamsSaveRestore.Channel[1].Enabled = TRUE;
    }
  }

  MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize = 0;
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
#ifdef SLICE_0_MASTER
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled) {
      MrcData->DynamicVars[Channel][SLICE_0_MASTER] = 1;
      MrcData->DynamicVars[Channel][SLICE_1_MASTER] = 0;
    } else {
      MrcData->DynamicVars[Channel][SLICE_0_MASTER] = 0;
      MrcData->DynamicVars[Channel][SLICE_1_MASTER] = 1;
    }
#endif

    MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TotalMem = 0;

    if (!MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      continue;
    }


    //
    // Look at both DIMMs and fill in DIMM-specific variables. Also add up total memory
    // per channel and system-wide.
    //
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].RankEnabled[0]) {

      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SlotMem[0] = CalculateRankSize(MrcData, Channel) * MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_Ranks[0];

      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].TotalMem  += MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SlotMem[0];
      MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize           += MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SlotMem[0];

      MmrcDebugPrint((MMRC_DBG_MIN, "DIMM%d Memory Size: %5d, System Mem %5d in MB\n", Channel, MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SlotMem[0], MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize));
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        MrcData->NumberOfEnabledChannels[TypeLpDdr3] += 1;
      }
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
        MrcData->NumberOfEnabledChannels[TypeDdr3L] += 1;
      }
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
        MrcData->NumberOfEnabledChannels[TypeLpDdr4] += 1;
      }
    } //rankEnabled

    switch (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType) {
    case TypeLpDdr3:
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType = T_LPDDR3;// >> 1;
      MrcData->CurrentBlueprint = 0;   //Lp3 : 0; LP4: 1
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] = MAX_STROBES_LPDDR3;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[1] = MAX_STROBES_LPDDR3;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = 0;
      break;
    case TypeLpDdr4:
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType = T_LPDDR4;// >> 1;
      if (MrcData->CpuStepping == STEPPING_A0) {
        MrcData->CurrentBlueprint = 1;   //Lp3 : 0; LP4: 1
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] = MAX_STROBES_LPDDR4_COPOP;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[1] = MAX_STROBES_LPDDR4_COPOP;
      }
      if (MrcData->BxtSeries == SERIES_BXT_P) {
        MrcData->CurrentBlueprint = 0;   //Lp3 : 0; LP4: 1
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] = MAX_STROBES_LPDDR4_BGA;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[1] = MAX_STROBES_LPDDR4_BGA;
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = 0;
      }
      break;
    case TypeDdr3L:
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType = T_DDR3L;// >> 1;
      MrcData->CurrentBlueprint = 1;    //Lp3 : 0; LP4: 1
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0] = MAX_STROBES_DDR3L;
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[1] = MAX_STROBES_DDR3L;
      if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_ECC_MEM_DOWN) {
        MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].EccEnabled = 1;
      }
      break;
    default:
      MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].CurrentDdrType = T_LPDDR3;// >> 1;
      break;
    }
    MrcData->MaxElements = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0];
    MrcData->MaxBits =  MAX_BITS;  //assign to maxDq initially
  }

#ifdef SLICE_0_MASTER
  MmrcDebugPrint((MMRC_DBG_MIN, "Ch0 SLICE_0_MASTER: %d SLICE_1_MASTER: %d\n", MrcData->DynamicVars[0][SLICE_0_MASTER], MrcData->DynamicVars[0][SLICE_1_MASTER]));
#else
  MmrcDebugPrint((MMRC_DBG_MIN, "Vars for master slice not used\n"));
#endif

  //GCOMP_CH_EN programming
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
        ChEnMask |= (Channel == 1) ? 0x4 : 0x1;
    } else {
        ChEnMask |= (1 << Channel);
      }
    }
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "Overall ChEnMask: 0x%x \n",ChEnMask));
  // There are 2 DDRIO instances which need to get programmed
  // They are stored in the dynamic variables of CH0 and CH1

#ifdef GCOMP_CH_EN
    if (ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) {
    switch (ChEnMask) {
      case 0x1: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x1;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x0;
                break;
      case 0x4: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x0;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x1;
                break;
      case 0x5: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x5;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x5;
                break;
      default:  MmrcDebugPrint ((MMRC_DBG_MIN, "Error! ChEnMask = 0x%x \n",ChEnMask));
                MrcDeadLoop();
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "DDR3L ChEnMask PHY0: 0x%x PHY1: 0x%x\n",MrcData->DynamicVars[0][GCOMP_CH_EN],MrcData->DynamicVars[1][GCOMP_CH_EN]));
  } else {
    switch (ChEnMask) {
      case 0x1: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x1;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x0;
                break;
      case 0x3: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x3;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x0;
                break;
      case 0x4: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 1;
                break;
      case 0x5: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x5;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x5;
                break;
      case 0xc: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0x0;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0x3;
                break;
      case 0xf: MrcData->DynamicVars[0][GCOMP_CH_EN] = 0xf;
                MrcData->DynamicVars[1][GCOMP_CH_EN] = 0xf;
                MrcData->DynamicVars[2][GCOMP_CH_EN] = 0xf;
                MrcData->DynamicVars[3][GCOMP_CH_EN] = 0xf;
                break;
      default:  MmrcDebugPrint ((MMRC_DBG_MIN, "Error! ChEnMask = 0x%x \n",ChEnMask));
                MrcDeadLoop();
    }
    MmrcDebugPrint ((MMRC_DBG_MIN, "LP3/LP4 ChEnMask PHY0: 0x%x PHY1: 0x%x\n",MrcData->DynamicVars[0][GCOMP_CH_EN],MrcData->DynamicVars[2][GCOMP_CH_EN]));
  }
#endif
  return Status;
}

/**
  PrintMemoryConfig

  @param[in]  MrcData

  @retval  None
**/
void
  PrintMemoryConfig (
  IN  MMRC_DATA   *MrcData
  )
{
  UINT8  Channel;

  MmrcDebugPrint ((MMRC_DBG_MIN, "BootMode set to: %x (FB=0x10; S5=8; S0=1,2,4; S3=0x20)\n",MrcData->BootMode));
  MmrcDebugPrint ((MMRC_DBG_MIN, "CurrentFreq = %d     (1333 = %d, 1600 = %d, 1866  =%d, 2133 = %d, 2400 = %d, 2666 = %d, 3200 = %d)           \n", MrcData->NvData.MrcParamsSaveRestore.CurrentFrequency, MMRC_1333, MMRC_1600, MMRC_1866, MMRC_2133, MMRC_2400, MMRC_2666, MMRC_3200));
  MmrcDebugPrint ((MMRC_DBG_MIN, "SystemMemSize = %x\n", MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize));

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "CH %d Enabled ", Channel));
      if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr3) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "LPDDR3\n"));
      } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeDdr3L) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "DDR3L\n"));
      } else if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType == TypeLpDdr4) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "LPDDR4\n"));
      } else {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Unknown mem type\n"));
      }
    }
  }

}

/**
  This routine is responsible to initialize ALL data required for
  MMRC to initialize memory. Typical flow for server would be:

  1) DIMM detection
  2) Timing calculations
  3) Variable initialization
  4) BIOS Setup overrides

  Typical phone/tablet flow would be:

  1) Timing variable population via lookup table
  2) BIOS Setup overrides

  This routine can be very complex in server segments or extremely
  lightweight in phone/tablet segments, depending on code size and
  MRC execution time requirements.

  @param[in,out]  MrcData  Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
GetInputParameters (
  IN  OUT   MMRC_DATA   *MrcData
)
{
#ifdef FSP_FLAG
  FSPM_UPD                        *FspmUpd;

  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
#endif
  OemGetPlatformType (MrcData);
  //
  // Sets the OEM specific information
  //
  OemSetMrcData (MrcData);

  if (MrcData->BootMode != S3) {
    DetermineBootMode (MrcData);
  } else {
    if (MrcData->MrcParamsValidFlag == 0) {   //if S3 and restored data not valid.
      MmrcDebugPrint ((MMRC_DBG_MIN, "Force powergood reset. Vars not valid in S3 resume\n"));
#ifdef FSP_FLAG
      if (!(FspmUpd->FspmConfig.EnableResetSystem)) {
        FspApiReturnStatusReset (FSP_STATUS_RESET_REQUIRED_COLD);
      }
#endif
      IoOut8 (0xCF9, 0x0);
      IoOut8 (0xCF9, 0xE);
#if !defined SUSSW
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
      "hlt"
  );
#else // MSFT compiler
      _asm hlt
#endif
#endif
      return MMRC_FAILURE;
    }
  }
  ///
  /// Select to use the restore path here we decide here after DetermineBootMode
  /// and DetectDimms, this may change for memory down we decide here in case a define
  /// prevents DetermineBootMode or DetectDimms functions to execute
  ///
//  if ((MrcData->BootMode != S5) && (MrcData->CpuStepping != STEPPING_A0)) {
  MmrcDebugPrint ((MMRC_DBG_MIN, "CurrentBootMode -> %d ValidFlag -> %d\n", MrcData->BootMode, MrcData->MrcParamsValidFlag));  //KW REMOVE
  if ((MrcData->BootMode != S5) && (MrcData->MrcParamsValidFlag == 1)) {
    MrcData->RestorePath = TRUE;
    MmrcDebugPrint ((MMRC_DBG_MIN, "Restore Path\n"));
  } else {
    MrcData->RestorePath = FALSE;
    MmrcDebugPrint ((MMRC_DBG_MIN, "NOT Restore Path\n"));
  }
#if DDR3_SUPPORT || DDR4_SUPPORT
  if ((MrcData->BootMode == S5) && !IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) ) {
    DetectDimms (MrcData);
  }
#endif

  GatherSocConfigs (MrcData);

  DetermineCommonFreq (MrcData);

  if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_CH0_MD_CH1_SOD) || (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown == MD_CH0_SOD_CH1_MD)) {
    if (MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize < MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.DDR3LPageSize) {
      MrcData->NvData.MrcParamsSaveRestore.DDR3L_PageSize = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.DDR3LPageSize;
    }
  }

  //
  // Save DIMM information for fast boot path comparison
  //
#if DDR3_SUPPORT || DDR4_SUPPORT
  if ((ConfigToDdrType[MrcData->MrcMemConfig] == TypeDdr3L) && !IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown)) {
    FillInputStructure (MrcData);
  }
#endif

  return MMRC_SUCCESS;
}

/**
  This routine assumes all input parameters to the MMRC have been
  populated by GetInputParameters(). Converting all timing variables
  to the correct values for DUNIT programming is the responsibility
  of this function.

  @param[in,out]  MrcData  Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
ConvertInputParameters (
  IN  OUT   MMRC_DATA   *MrcData
)
{



  return MMRC_SUCCESS;
}

/**
  ProgramMemoryMap

  @param[in]  MrcData  Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
ProgramMemoryMap (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  UINT32        MmioAllocation;
  UINT32        TOM;
  UINT32        LowMemory;
  BOOLEAN       ReclaimEn;
  UINT32        HighMemory;
  UINT16        TSegSize;
  UINT32        TSegBase;
  UINT16        GMSsize;
  UINT16        GTTsize;
  UINT32        GMSbase;
  UINT32        GTTbase;
  UINT32        SeCUmaBase;
  UINT32        TempGTTbase;
//  UINT32    SeCfTPMUmaBase;
  REGISTER_ACCESS               Register;
  GGC_0_0_0_PCI_CUNIT_BOXTYPE_STRUCT    Cuint_GGC;
  UINT32                        Buffer32;

  SeCUmaBase = 0;
//[-start-160107-IB08450333-remove]//
//#ifdef FTPM_ENABLE
//  UINT32  SeCfTPMUmaBase;
//#endif
//[-end-160107-IB08450333-remove]//

//[-start-171002-IB07400914-remove]//
//#ifdef ISH_ENABLE
//  UINT32  IshBase;
//#endif
//[-end-171002-IB07400914-remove]//

  MrcData->OemMrcData.TsegSize = 0x8;
  MrcData->OemMrcData.MmioSize = 0x400;


  GMSsize = 0;
  GTTsize = 0;
  TSegSize = 0;
  Buffer32 = 0;

  Register.Mask = 0xFFFFFFFF;
  Register.ShiftBit = 0;

  Register.Offset = GGC_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  Cuint_GGC.Data = MemRegRead (CUNIT_BOXTYPE, 0, 0, Register);


  //All unit in 1MB granularity
  if(Cuint_GGC.Bits.gms < 0x40){
    GMSsize = (UINT16) Cuint_GGC.Bits.gms * 32;
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "Warning GMSsize is zero\n"));
    //return MMRC_FAILURE;
  }

  if(Cuint_GGC.Bits.ggms > 0x0){
    GTTsize = 1 << Cuint_GGC.Bits.ggms;
  }


  if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.SystemMemorySizeLimit != 0) &&
      (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.SystemMemorySizeLimit < MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize)) {
        MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.SystemMemorySizeLimit;
  }

  TSegSize       = MrcData->OemMrcData.TsegSize;
  MmioAllocation = MrcData->OemMrcData.MmioSize;
  TOM            = MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize;

  // The maximum TOLUD is 2GB
  LowMemory = MIN(TWO_GB_IN_MB, TOM);

  if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.LowMemMaxVal != 0) &&
      (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.LowMemMaxVal < LowMemory)) {
    LowMemory = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.LowMemMaxVal;
  }

  MmrcDebugPrint ((MMRC_DBG_MIN, "LowMemory:      0x%08x\n", LowMemory));

  ReclaimEn = FALSE;

  if (TOM > LowMemory) {
    ReclaimEn = TRUE;
  }

  if (ReclaimEn) {
    HighMemory = TOM - LowMemory + FOUR_GB_IN_MB;
  } else {
    HighMemory = FOUR_GB_IN_MB;
  }

  if ((MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.HighMemMaxVal != 0) &&
    (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.HighMemMaxVal >= FOUR_GB_IN_MB) &&
      (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.HighMemMaxVal < HighMemory)) {
    HighMemory = MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.HighMemMaxVal;
  }
  MmrcDebugPrint ((MMRC_DBG_MIN, "HighMemory:     0x%08x\n", HighMemory));

//[-start-171002-IB07400914-modify]//
//#ifdef ISH_ENABLE
//  //
//  // 1MB alloc for ISH
//  //
//#ifdef FTPM_ENABLE
//  IshBase = SeCfTPMUmaBase - CurrentMrcData->IshUmaSize;
//#else
//  IshBase = SeCUmaBase - CurrentMrcData->IshUmaSize;
//#endif
//  //
//  //9. Determine GFX Memory base, which is calculated by the value of TOLUD minus GFX size.
//  //
//  GMSbase = IshBase - GMSsize;
//#else
  //
  //9. Determine GFX Memory base, which is calculated by the value of TOLUD minus GFX size.
  //
  GMSbase = LowMemory - GMSsize;
//#endif
//[-end-171002-IB07400914-modify]//

  TempGTTbase = GMSbase - GTTsize;

  // Make sure that GTTbase is aligned such that
  // 1. There is no gap between the GTT and TSEG stolen memory. These two
  //    Memory Regions must be contiguous per the HSTI spec.
  // 2. TSEG is aligned to an address boundary that matches the TSEG size
  GTTbase = (TempGTTbase & ~(MrcData->OemMrcData.TsegSize - 1));

  MmrcDebugPrint ((MMRC_DBG_MIN, "SeCUmaBase 0x%x GMSsize 0x%x GMSbase 0x%x GTTbase 0x%x TempGTTbase 0x%x\n", SeCUmaBase,GMSsize,GMSbase,GTTbase,TempGTTbase));

  ///
  ///11. Determine TSEG base, which is calculated by the value of TOLUD minus GFX size minus TSEG size.
  ///
  TSegBase = GTTbase - TSegSize;


///////////////////////////////////////////////////////
///Program the registers
///////////////////////////////////////////////////////


  MmrcDebugPrint ((MMRC_DBG_MIN, "TSegSize:       0x%08x\n", TSegSize));
  MmrcDebugPrint ((MMRC_DBG_MIN, "TSegBase:       0x%08x\n", TSegBase));
  MmrcDebugPrint ((MMRC_DBG_MIN, "MmioAllocation: 0x%08x\n", MmioAllocation));
  MmrcDebugPrint ((MMRC_DBG_MIN, "TOM:            0x%08x\n", TOM));
  MmrcDebugPrint ((MMRC_DBG_MIN, "TOLUD:          0x%08x\n", LowMemory << 20));
  MmrcDebugPrint ((MMRC_DBG_MIN, "TOUUD_LO:       0x%08x\n", HighMemory << 20));
  MmrcDebugPrint ((MMRC_DBG_MIN, "TOUUD_HI:       0x%08x\n", HighMemory >> 12));

  Register.Mask = 0xFFFFFFFF;
  Register.ShiftBit = 0;

  Register.Offset = TOLUD_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  MemRegWrite (CUNIT_BOXTYPE, 0, 0, Register, (LowMemory << 20) + BIT0, 0xF);
  Register.Offset = TOUUD_LO_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  MemRegWrite (CUNIT_BOXTYPE, 0, 0, Register, (HighMemory << 20) & 0xFFFFFFFF, 0xF);
  Register.Offset = TOUUD_HI_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  MemRegWrite (CUNIT_BOXTYPE, 0, 0, Register, (HighMemory >> 12), 0xF);
  Register.Offset = BDSM_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  MemRegWrite (CUNIT_BOXTYPE, 0, 0, Register, (GMSbase << 20) + BIT0, 0xF);
  Register.Offset = BGSM_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  MemRegWrite (CUNIT_BOXTYPE, 0, 0, Register, (GTTbase << 20) + BIT0, 0xF);
  Register.Offset = TSEGMB_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  MemRegWrite (CUNIT_BOXTYPE, 0, 0, Register, (TSegBase << 20) + BIT0, 0xF);

  return MMRC_SUCCESS;
}

