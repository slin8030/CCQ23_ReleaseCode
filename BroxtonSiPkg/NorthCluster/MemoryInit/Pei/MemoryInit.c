/** @file
  Framework PEIM to initialize memory on a Memory Controller.

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

#include "MemoryInit.h"
#include "DetectDimms.h"
#include "McFunc.h"
#include "MrcFunc.h"
//[-start-151008-IB0309APL-add]//
#include "ChipsetCmos.h"
//[-end-151008-IB0309APL-add]//

#include <Library/PcdLib.h>
#include <Library/PciExpressLib.h>
#include <Guid/GlobalVariable.h>
#include <Guid/SmramMemoryReserve.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/SeCLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/ReadOnlyVariable.h>
#include <Guid/MemoryConfigData.h>
#include <Ppi/SeCUma.h>
#include <Ppi/HeciPpi.h>
#include <Ppi/DramPolicyPpi.h>
//[-start-151224-IB07220029-add]//
#include <Ppi/EmuPei.h>
//[-end-151224-IB07220029-add]//
#include <Guid/NpkInfoHob.h>
#include <Guid/BiosReservedMemoryHob.h>
#include <Library/TimerLib.h>
#include "IchRegs.h"
#include <Ppi/ReadOnlyVariable2.h>

#include <Library/PreSiliconLib.h>
#include <Library/SteppingLib.h>
#if defined (RTIT_Support) && (RTIT_Support == 1)
#include "RtitMemory.h"
#endif

#ifdef APTIO_BUILD
#define VFRCOMPILE
#include <Include/Setup.h>
#else
#if SMM_SUPPORT
#include <Guid/SmramMemoryReserve.h>
#endif
#endif

#ifdef FSP_FLAG
#include <Guid/GuidHobFspEas.h>
#include <Library/FspPlatformLib.h>
#include <FspEas.h>
#include <Library/FspCommonLib.h>
#include <FspmUpd.h>
#include <IndustryStandard/SmBios.h>
#endif
//[-start-160218-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160218-IB07400702-add]//

//[-start-151210-IB03090424-remove]//
//#ifndef FSP_FLAG
//#include <Library/CmosAccessLib.h>
//#endif
//[-end-151210-IB03090424-remove]//
#ifdef CRASHDUMP_SUPPORT
#include <Guid/SetupVariable.h>
#include <ScRegs/RegsPmc.h>
#include <Library/IoLib.h>
#include <CMOSMap.h>

extern EFI_GUID gEfiCrashDumpAddrGuid;
#endif
//[-start-160608-IB06720411-add]//
#include <ChipsetCmos.h>
#include <Library/CmosLib.h>
//[-end-160608-IB06720411-add]//

//[-start-160115-IB03090424-add]//
extern EFI_GUID gSsaResultGuid;
//[-end-160115-IB03090424-add]//

#define SKPD_RSVD_WARM_RESET               0x0C000000  //26,27
//[-start-160608-IB06720411-add]//
extern EFI_GUID gEfiMemoryConfigVariableGuid;
//[-end-160608-IB06720411-add]//

#ifdef PRAM_SUPPORT
extern EFI_GUID gPramAddrDataGuid;
#endif


#ifndef FSP_FLAG
/**
  This function installs the EFI Memory when S3 path is executed

  @param[in]      PeiServices  PEI Services Table.
  @param[in,out]  MrcData      Pointer to MRC Output Data that contains MRC Parameters.

  @retval  EFI_SUCCESS         When no errors when installing memory
  @retval  Others              Errors occurred while installing memory
**/
EFI_STATUS
InstallS3Memory (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
);
#endif

//[-start-151224-IB07220029-add]//
BOOLEAN  mInRecoveryPei = FALSE;

BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  )
{
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    return mInRecoveryPei;
  } else {
    return FALSE;
  }
}

VOID
FastCrisisRecoveryCheck (
  VOID    
  )
{
  VOID           *EmuPeiPpi;
  EFI_STATUS     Status;
  
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    Status = PeiServicesLocatePpi (
                    &gEmuPeiPpiGuid,
                    0,
                    NULL,
                    (VOID **) &EmuPeiPpi
                    );
    mInRecoveryPei = EFI_ERROR(Status) ? FALSE : TRUE;
  }
}

EFI_STATUS
RecordFastCrisisRecoveryInfo (
  IN CONST EFI_PEI_SERVICES         **PeiServices,
  IN EFI_BOOT_MODE                   BootMode,
  IN MMRC_DATA                       *MrcData
  )
{
  EFI_STATUS                          Status;
  if (IsRunCrisisRecoveryMode()) {
    Status = ConfigureMemory (MrcData);
    return InstallEfiMemory((EFI_PEI_SERVICES **) PeiServices, BootMode, MrcData);    
  }

  return EFI_NOT_STARTED;
}
//[-end-151224-IB07220029-add]//

/**
  CpuMemoryTest

  @param[in]  None

  @retval  EFI_STATUS
**/
EFI_STATUS CpuMemoryTest() {

  UINT32                 Address;
  INT8                   errors_found;
  UINT32                 Data = 0;
  EFI_STATUS             Status;
  EFI_PHYSICAL_ADDRESS   TSegBase;
  REGISTER_ACCESS        Register;
  INT8                   ErrorLimit=100;
  //    UINT32                 AmtMemToTest = 0x100000;
  INT8                   WhichPattern = 0;
  UINT32                 Pattern = 0;
  UINT8                  Results[2];

  Results[0]=0;
  Results[1]=0;
  errors_found=0;
  Register.Mask = 0xFFFFFFFF;
  Register.ShiftBit = 0;
  Register.Offset = TSEGMB_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  TSegBase = (UINT32) MemRegRead (CUNIT_BOXTYPE, 0, 0, Register) & 0xFFF00000;

  DEBUG ((EFI_D_INFO, "Please wait while memory is spot checked... \n"));

  Status = MMRC_SUCCESS;

  do {
    Address = 0;
    switch (WhichPattern) {
    case 0: Pattern = 0xFFFFFFFF;
      break;
    case 1: Pattern = 0x00000000;
      break;
    default: Pattern = 0x11111111;
    };
    //WRITE 1MB
    do {
#ifndef __KLOCWORK__
      * ( (volatile UINT32 *) Address) = Pattern;
#endif
      Address += 4;
    } while (Address < 0x100000);

    Address = 0;

    //READ 1MB
    do {
#ifndef __KLOCWORK__
      Data = * ( (volatile UINT32 *) Address);
#endif
      if (Data != Pattern) {
        if ((Results[WhichPattern] == 1) && (errors_found < ErrorLimit))
          DEBUG ((EFI_D_ERROR, "FAILURE detected at 0x%08x : expected %08x : actual %08x\n", Address, Pattern, Data));
        errors_found += 1;
        Results[WhichPattern] += 1;
        Status = MMRC_FAILURE;
      }
      //_asm stosd
      Address += 4;
    } while ((Address < 0x100000) && (errors_found < ErrorLimit));
    if (Results[WhichPattern] == 0)
      DEBUG ((EFI_D_INFO, "%08x pattern passed!\n", Pattern));
    WhichPattern++;
  } while ((WhichPattern < 2) && (errors_found < ErrorLimit));

  if (errors_found > ErrorLimit)
    DEBUG ((EFI_D_ERROR, "Max # of memory mis-matches reached(%d)!\n", errors_found));
  else
    DEBUG ((EFI_D_INFO, ">>Number of error(s) detected after memory test: %d\n", errors_found));

  if (errors_found == 0) {
    //WRITE the rest
    Address = 0x100000;
    do {
#ifndef __KLOCWORK__
      * ( (volatile UINT32 *) Address) = 0xffffffff;
#endif
      Address += 0x400;
    } while (Address < (UINT32)TSegBase);

    //READ the rest
    Address = 0x100000;

    do {
#ifndef __KLOCWORK__
      Data = * ( (volatile UINT32 *) Address);
#endif
      if (Data != 0xffffffff) {
        Status = MMRC_FAILURE;
        if (errors_found < ErrorLimit)
          DEBUG ((EFI_D_ERROR, "FAIL 0xffffffff pattern>> Address 0x%x: Data %x\n", Address, Data));
        errors_found++;
      }
      Address += 0x400;
    } while ((Address < (UINT32)TSegBase) && (errors_found < ErrorLimit));
  }

  if (errors_found == 0)
    DEBUG ((EFI_D_INFO, "MRC:CpuMemoryTest Successful!\n"));
  else {
    DEBUG ((EFI_D_ERROR, "MRC:CpuMemoryTest Failed! <>CPU Halted<>\n"));
    CpuDeadLoop();
  }
  return Status;
}


/**
  This populates fields in the MMRC_DATA structure which are dependent on the DRAM Policy.

  @param[in,out]  MmrcData    Main MMRC data structure

  @retval  VOID      Nothing is returned.
**/
VOID
GetDramPolicyInputs(
  IN     CONST   EFI_PEI_SERVICES     **PeiServices,
  IN OUT         MMRC_DATA            *MrcData
)
{
  EFI_STATUS                          Status;
  DRAM_POLICY_PPI                     *DramPolicyPpi;
  Status = (*PeiServices)->LocatePpi (
    PeiServices,
    &gDramPolicyPpiGuid,
    0,
    NULL,
    (VOID **)&DramPolicyPpi
    );

  if (!EFI_ERROR (Status)) {
    //
    // Note: DramPolicyPpi variable should not be used outside of this "if" statement.
    // If DramPolicyPpi is used outside of this statement without checking the "LocatePpi"
    // return status, you could be using invalid data on first boot.
    //
    MrcData->MrcInputs.ChannelHashMask         = DramPolicyPpi->ChannelHashMask;
    MrcData->MrcInputs.SliceHashMask           = DramPolicyPpi->SliceHashMask;
    MrcData->MrcInputs.SlicesChannelsEnabled   = DramPolicyPpi->ChannelsSlicesEnabled;
    MrcData->EnableScrambler                   = (UINT8)((SMIP_SCRAMBLERSUPPORT_OPTION *) &DramPolicyPpi->ScramblerSupport)->Bits.ScramblerSupport;
    MrcData->PeriodicRetrainingDisable         = (UINT8)((SMIP_SCRAMBLERSUPPORT_OPTION *) &DramPolicyPpi->ScramblerSupport)->Bits.PeriodicRetrainingDisable;
    MrcData->MrcInputs.InterleavedMode         = DramPolicyPpi->InterleavedMode;
    MrcData->MrcInputs.ForceMin2xRefresh       = DramPolicyPpi->MinRefRate2xEnabled;
    MrcData->MrcInputs.DualRankSupportEnabled  = DramPolicyPpi->DualRankSupportEnabled;
    CopyMem (&(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData), DramPolicyPpi, sizeof (DRAM_POLICY));
    DEBUG ((EFI_D_INFO, "Profile is now set to: %d \n", MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.Profile));
//[-start-171013-IB07400918-add]//
    //
    // Dump DRAM ChSwizzle Info from SMIP
    //
    DEBUG_CODE (
      UINTN Channel;
      UINTN BitsNumber;

      for (Channel = 0; Channel < DRAM_POLICY_NUMBER_CHANNELS; Channel ++) {
        DEBUG ((EFI_D_INFO, "Dump MrcParamsSaveRestore ChSwizzle Channel = %x\n", Channel));
        for (BitsNumber = 0; BitsNumber < DRAM_POLICY_NUMBER_BITS; BitsNumber ++) {
          DEBUG ((EFI_D_INFO, "%02x ", MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChSwizzle[Channel][BitsNumber]));
        }
        DEBUG ((EFI_D_INFO, "\n"));
      }
    );
//[-end-171013-IB07400918-add]//
  }
}

#ifdef FSP_FLAG
/**
  Build FSP SMBIOS memory info HOB

  @param[in]   Pointer to MrcData
**/
VOID
BuildFspSmbiosMemoryInfoHob (
  IN MMRC_DATA             *MrcData
  )
{
  FSP_SMBIOS_MEMORY_INFO FspSmbiosMemoryInfo;
  UINT8                  Channel;
  UINT8                  ChannelCount;
  UINT8                  DimmIndex;
  UINT8                  DimmCount;

  FspSmbiosMemoryInfo.Revision = 0x01;

  switch (MrcData->NvData.MrcParamsSaveRestore.Channel[0].D_DataWidth[0]) {
    case SMIP_DramWidthx8:
      FspSmbiosMemoryInfo.DataWidth = 8;
      break;
    case SMIP_DramWidthx16:
      FspSmbiosMemoryInfo.DataWidth = 16;
      break;
    case SMIP_DramWidthx32:
      FspSmbiosMemoryInfo.DataWidth = 32;
      break;
  }

  FspSmbiosMemoryInfo.MemoryType = MrcData->NvData.MrcParamsSaveRestore.Channel[0].CurrentDdrType;

  switch (ConfigToSpeed[MrcData->MrcMemConfig]) {
  case MMRC_1333:
    FspSmbiosMemoryInfo.MemoryFrequencyInMHz = 1333;
    break;
  case MMRC_1600:
    FspSmbiosMemoryInfo.MemoryFrequencyInMHz = 1600;
    break;
  case MMRC_1866:
    FspSmbiosMemoryInfo.MemoryFrequencyInMHz = 1866;
    break;
  case MMRC_2133:
    FspSmbiosMemoryInfo.MemoryFrequencyInMHz = 2133;
    break;
  case MMRC_2400:
    FspSmbiosMemoryInfo.MemoryFrequencyInMHz = 2400;
    break;
  case MMRC_2666:
    FspSmbiosMemoryInfo.MemoryFrequencyInMHz = 2666;
    break;
  default:
    break;
  }

  if (MrcData->NvData.MrcParamsSaveRestore.Channel[0].EccEnabled) {
    FspSmbiosMemoryInfo.ErrorCorrectionType = MemoryErrorCorrectionSingleBitEcc;
  } else {
    FspSmbiosMemoryInfo.ErrorCorrectionType = MemoryErrorCorrectionNone;
  }
  ChannelCount = 0;
  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    DimmCount = 0;
    FspSmbiosMemoryInfo.ChannelInfo[Channel].ChannelId = Channel;
    if (MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled) {
      for (DimmIndex = 0; DimmIndex < MAX_DIMMS; DimmIndex++) {
        FspSmbiosMemoryInfo.ChannelInfo[Channel].DimmInfo[DimmIndex].DimmId = DimmIndex;
        FspSmbiosMemoryInfo.ChannelInfo[Channel].DimmInfo[DimmIndex].SizeInMb = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].SlotMem[DimmIndex];
        DimmCount++;
      }
      FspSmbiosMemoryInfo.ChannelInfo[Channel].DimmCount = DimmCount;
      ChannelCount++;
    }
  }
  FspSmbiosMemoryInfo.ChannelCount = ChannelCount;

  //
  // Build HOB for FspSmbiosMemoryInfo
  //
  BuildGuidDataHob (
    &gSiMemoryInfoDataGuid,
    &FspSmbiosMemoryInfo,
    sizeof (FSP_SMBIOS_MEMORY_INFO)
    );
}
#endif

//[-start-180131-IB07400947-add]//
#define NUM_OF_BACKUP_ADDRESS   2

UINT32 mBackupAddress[] = {0x62800000, 0x62801000};
//[-end-180131-IB07400947-add]//

/**
  This function is the Framework PEIM entry point to initialize memory on a Memory Controller.

  @param[in]  FileHandle    File Handler
  @param[in]  PeiServices   PEI Services Table.

  @retval  EFI_SUCCESS      If memory training was successful.
  @retval  Others           Errors occurred while training memory
**/
EFI_STATUS
PeimMemoryInit (
  IN          EFI_PEI_FILE_HANDLE  FileHandle,
  IN  CONST   EFI_PEI_SERVICES     **PeiServices
)
{
  EFI_STATUS                          Status;
  EFI_BOOT_MODE                       BootMode;
  MMRC_DATA                           MrcData;

#ifdef DEBUG_HOOKS
  UINT8                               cpu_delay;
  UINT32                              preRegister;
  SKPD_struct                         RegData32;
  UINT32                              pcdBaseSave;
  MrcVersion                          IafwVersion;
#endif
  SEC_UMA_PPI                         *SeCUma;
  UINT8                               InitStat;
  BOOLEAN                             SkipNvDataRestore = FALSE;
//[-start-180131-IB07400947-add]//
  UINT32                              BackupAddressData[NUM_OF_BACKUP_ADDRESS];
  UINT8                               BackupAddressIndex;
//[-end-180131-IB07400947-add]//

InitializePeimMemoryInit:

  //
  // Disable PCH Watchdog timer at SB_RCBA+0x3410
  //
  Mmio32Write (PBASE + 0x08, Mmio32Read (PBASE + 0x08) | 0x10);
  //
  // Determine boot mode
  //
  Status = (*PeiServices)->GetBootMode (
             PeiServices,
             &BootMode
           );

  ASSERT_EFI_ERROR (Status);

  InitStat = 0;
//[-start-151224-IB07220029-add]//
  FastCrisisRecoveryCheck();
//[-end-151224-IB07220029-add]//
  //
  // Initialize params before configure memory
  //
  MrcInitializeParameters (&MrcData);

  GetDramPolicyInputs(PeiServices, &MrcData);
//[-start-151123-IB07220013-add]//
#if MRC_EV_RMT_BUILD == 1
  MrcData.MrcRmtSupport |= RMT_SUPPORT_EV_MODE | RMT_SUPPORT_ENABLED; 
#endif
//[-end-151123-IB07220013-add]//

  MrcData.BxtSeries = GetBxtSeries ();

#if (MRCDEBUG_MSG == 1)
  MrcData.MrcDebugMsgLevel = MMRC_DBG_VERBOSE;
#else
  MrcData.MrcDebugMsgLevel = MMRC_DBG_MIN;
#endif
  //
  // Execute DetectDimms
  //



  //
  // Locate SeCUma PPI which will be used in the following flows: coldboot and S3Resume
  //
  Status = (*PeiServices)->LocatePpi (PeiServices, &gSeCUmaPpiGuid, 0, NULL, (void **)&SeCUma);
  ASSERT_EFI_ERROR ( Status);

  //
  // SEC Stolen Size in MB units
  //
  //DEBUG ((EFI_D_INFO, "MRC getting memory size from SeC ...\n"));
  MrcData.SeCUmaSize = SeCUma->SeCSendUmaSize ((EFI_PEI_SERVICES    **)PeiServices);  //expect in MB
  DEBUG ((EFI_D_INFO, "MRC SeCUmaSize memory size from SeC ... %x \n", MrcData.SeCUmaSize));
//[-start-151224-IB07220029-add]//
  Status = RecordFastCrisisRecoveryInfo (PeiServices, BootMode, &MrcData);
  if (!EFI_ERROR(Status)) {
    return Status;
  }
//[-end-151224-IB07220029-add]//


  if (BootMode == BOOT_ON_S3_RESUME) {
    MrcData.BootMode = S3;
    DEBUG ((EFI_D_INFO, "\nResuming from S3 detected! 0x%x\n", MrcData.BootMode));
  }

  MrcData.PlatformID = PLATFORM_ID;

  MrcData.CpuStepping = BxtStepping();
  if (MrcData.CpuStepping == 1)
    MrcData.CpuStepping = STEPPING_A0;
#if BXT_PO
  MrcData.CpuStepping = STEPPING_A0;
#endif
  DEBUG ((EFI_D_INFO, "CPU stepping = %d.\n", MrcData.CpuStepping));

  MrcData.BxtSeries = GetBxtSeries();
  switch(MrcData.BxtSeries){
  case Bxt1:
    MrcData.BxtSeries = SERIES_BXT_1;
    break;
    break;
  case BxtP:
    MrcData.BxtSeries = SERIES_BXT_P;
    break;
  }
  DEBUG ((EFI_D_INFO, "BXT Series = %d.\n", MrcData.BxtSeries));

  MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_COMPUTING_UNIT_MEMORY | EFI_CU_MEMORY_PC_CONFIGURING));

  pcdBaseSave = (UINT32)PcdGet64(PcdPciExpressBaseAddress);

  preRegister = pcdBaseSave + (UINT32)DEBUP0_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  RegData32.Data = Mmio32Read(preRegister);
  MrcData.DEBUP0Reg = RegData32.Data;
  preRegister = pcdBaseSave + (UINT32)SKPD_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  RegData32.Data = Mmio32Read(preRegister);
  MrcData.SKPDTestReg = RegData32.Data;
  DEBUG((EFI_D_INFO, " SKPD: 0x%08x DEBUP: 0x%08x\n", MrcData.SKPDTestReg, MrcData.DEBUP0Reg));

  GetIafwVersion(&MrcData, &IafwVersion);
  RegData32.SKPD_bits.IAFW_version    = (0xffff & ((IafwVersion.Version.Minor << 8) | (IafwVersion.Version.Rev)));
  Mmio32Write(preRegister, RegData32.Data);

  //
  // Restore MRC Parameters from NVRAM if Fast Boot is not disabled.
  //
  if (SkipNvDataRestore == FALSE && ((DEBUP0_struct*)(&(MrcData.DEBUP0Reg)))->DEBUP0_bits.NoFB == 0 && MrcData.NvData.MrcParamsSaveRestore.DramPolicyData.DisableFastBoot == 0){
    Status = MrcParamsRestore (
      (EFI_PEI_SERVICES **)PeiServices,
      &MrcData
      );
    if (EFI_ERROR(Status)) {
      SkipNvDataRestore = TRUE;
      DEBUG((EFI_D_INFO, "\x1b[41mParamsRestore=%r\x1b[0m\n",Status));
      //
      // If the MrcParamsRestore returned an error, MrcData is potentially populated with
      // stale or corrupt data. Jump back to a point where we can re-initalize the data.
      //
//[-start-160512-IB07400729-modify]//
      CHIPSET_POST_CODE (PEI_MRC_MEMORY_INIT_RETRY); 
//[-end-160512-IB07400729-modify]//
      goto InitializePeimMemoryInit;
    }
  }
  cpu_delay = 254;
  if (RegData32.SKPD_bits.HALT_before_MRC == 1) {
    DEBUG ((EFI_D_WARN, "! Stall selected at beginning of MRC !\n"));
    DEBUG ((EFI_D_WARN, "! Clear SKPD_0_0_0_PCI_CUNIT_BOXTYPE_REG[bit16] to continue !\n"));
    do {
        do {
          CpuPause();
        } while (cpu_delay-- > 0);
        cpu_delay = 254;
        RegData32.Data = Mmio32Read(pcdBaseSave + (UINT32)SKPD_0_0_0_PCI_CUNIT_BOXTYPE_REG);
    } while (RegData32.SKPD_bits.HALT_before_MRC == 1);
  }




  Status = ConfigureMemory (&MrcData);
  if (Status == MMRC_DATA_DIRTY) {
    SkipNvDataRestore = TRUE;
    DEBUG ((EFI_D_INFO, "\x1b[41m MrcData is dirty\x1b[0m\n"));
    //
    // If the MMRC_DATA struct is dirty, we need to re-initialize it. The only time we would have to do this
    // is if an S0 or FB was demoted to an S5
    //
//[-start-160512-IB07400729-add]//
    CHIPSET_POST_CODE (PEI_MRC_MEMORY_INIT_RETRY); 
//[-end-160512-IB07400729-add]//
    goto InitializePeimMemoryInit;
  }

  
  if (Status != EFI_SUCCESS) {
//[-start-160218-IB07400702-modify]//
    DEBUG ((EFI_D_ERROR, "ConfigureMemory FAILED, do the reboot!\n"));
//    MrcDeadLoop();
    CHIPSET_POST_CODE (PEI_MRC_CONFIG_MEMORY_FAIL); 
    IoOut8 (0xCF9, (UINT8) 0xE);
//[-end-160218-IB07400702-modify]//
  }

  RegData32.Data = MrcData.SKPDTestReg;
  cpu_delay = 254;
  if (RegData32.SKPD_bits.HALT_after_MRC == 1) {
    DEBUG ((EFI_D_WARN, "! Stall selected at end of MRC !\n"));
    DEBUG ((EFI_D_WARN, "! Clear SKPD_0_0_0_PCI_CUNIT_BOXTYPE_REG[bit17] to continue !\n"));
    do {
      do {
        CpuPause();
      } while (cpu_delay-- > 0);
      cpu_delay = 254;
      RegData32.Data = Mmio32Read(pcdBaseSave + (UINT32)SKPD_0_0_0_PCI_CUNIT_BOXTYPE_REG);
    } while (RegData32.SKPD_bits.HALT_after_MRC == 1);
  }

#if VP_BIOS_ENABLE
  DEBUG ((EFI_D_INFO, "!!Skip Memory Test for VP!!\n"));
#else
  //Skip for Simics
  if ((RegData32.SKPD_bits.SkipMemTest != 1)&&(MrcData.PlatformID == VALUE_REAL_PLATFORM)&&
      ((MrcData.BootMode != S3)&&(MrcData.BootMode != S0C)&&(MrcData.BootMode != S0W)&&(MrcData.BootMode != S0))) {
#ifndef FORCE_MEMTEST
      if (MrcData.BootMode != FB)
#endif
      {
        CpuMemoryTest();
      }
  } else {
    if ((MrcData.BootMode != S3)&&(MrcData.BootMode != S0C) && (MrcData.BootMode != S0W) && (MrcData.BootMode != S0))
      DEBUG ((EFI_D_INFO, "!!-Preserving Memory-Skipping memory test-!!\n"));
  }
#endif


//[-start-180131-IB07400947-add]//
  //
  // Backup Address Data before send DID done
  //
  for (BackupAddressIndex = 0; BackupAddressIndex < NUM_OF_BACKUP_ADDRESS; BackupAddressIndex++) {
    BackupAddressData[BackupAddressIndex] = *((volatile UINT32 *) (UINTN)mBackupAddress[BackupAddressIndex]);
  }
//[-end-180131-IB07400947-add]//
  // Fix SeCUmaBase at 0x1000000 For All Memory Sizes
  MrcData.SeCUmaBase = 0x10000000;   /// revisit

  SeCUma->SeCConfigDidReg(PeiServices, MrcData.BootMode,0, MrcData.SeCUmaBase, &MrcData.SeCUmaSize);
//[-start-180131-IB07400947-add]//
  //
  // Rollbak Data After send DID done
  //
  for (BackupAddressIndex = 0; BackupAddressIndex < NUM_OF_BACKUP_ADDRESS; BackupAddressIndex++) {
    *((volatile UINT32 *) (UINTN)mBackupAddress[BackupAddressIndex]) = BackupAddressData[BackupAddressIndex];
  }
//[-end-180131-IB07400947-add]//

  //Set MEM_INIT_DONE after sending DID message
  MmrcExecuteTask (&MrcData, SetPunitMemDone, NO_PRINT, 0);

  MmrcExecuteTask (&MrcData, PollPunit, NO_PRINT, 0);
#ifdef FSP_FLAG
  BuildFspSmbiosMemoryInfoHob (&MrcData);
#endif
  //
  // Install memory
  //
  DEBUG ((EFI_D_INFO, "Install EFI Memory.\n"));

  Status = InstallEfiMemory ((EFI_PEI_SERVICES **) PeiServices, BootMode, &MrcData);

  if (Status == EFI_SUCCESS) {
    MRC_PEI_REPORT_PROGRESS_CODE(PeiServices, (EFI_SOFTWARE_PEI_SERVICE | EFI_SW_PS_PC_INSTALL_PEI_MEMORY));
    DEBUG ((EFI_D_INFO, "MemoryInit Installed.\n"));
  } else {
    MRC_PEI_REPORT_ERROR_CODE(PeiServices, (EFI_SOFTWARE_PEI_SERVICE | EFI_SW_EC_NON_SPECIFIC), EFI_ERROR_MAJOR);
    DEBUG ((EFI_D_INFO, "MemoryInit Not Installed.\n"));
  }

  if (BootMode != BOOT_ON_S3_RESUME) {
    MrcParamsSave ((EFI_PEI_SERVICES **) PeiServices, &MrcData);
    //DEBUG ((EFI_D_INFO, "Save MRC params.\n"));
  }



  // Clear any text formatting before returning
  DEBUG ((EFI_D_INFO, "\x1b[0m\n"));
  return EFI_SUCCESS;
}

//[-start-171023-IB07400920-add]//
#include <Guid/MemoryTypeInformation.h>
#include <Library/VariableLib.h>
#include <Library/PeiOemSvcKernelLib.h>

/**
  Check the input memory type information is whether valid.

  @param[in] MemTypeInfo    Pointer to input EFI_MEMORY_TYPE_INFORMATION array
  @param[in] MemTypeInfoCnt The count of EFI_MEMORY_TYPE_INFORMATION instance.

  @retval TRUE              The input EFI_MEMORY_TYPE_INFORMATION is valid.
  @retval FALSE             Any of EFI_MEMORY_TYPE_INFORMATION instance in input array is invalid.
--*/
STATIC
BOOLEAN
IsMemoryTyepInfoValid (
  IN EFI_MEMORY_TYPE_INFORMATION       *MemTypeInfo,
  IN UINTN                             MemTypeInfoCnt
  )
{
  UINTN         Index;

  if (MemTypeInfo == NULL && MemTypeInfoCnt != 0) {
    return FALSE;
  }

  for (Index = 0; Index < MemTypeInfoCnt; Index++) {
    if ((MemTypeInfo[Index].NumberOfPages & 0x80000000) != 0 || MemTypeInfo[Index].Type > EfiMaxMemoryType) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
  Get platform need memory.

  @param[in]  PeiServices
  @param[in]  This
  @param[in]  MemorySize

  @retval

**/
EFI_STATUS
EFIAPI
GetPlatformMemorySize (
  IN      EFI_PEI_SERVICES                       **PeiServices,
  IN OUT  UINT64                                 *MemorySize
  )
{
  EFI_STATUS                        Status;
  UINTN                             Index;
  EFI_MEMORY_TYPE_INFORMATION       *MemoryTypeInformation;
  EFI_MEMORY_TYPE_INFORMATION       MemoryData[EfiMaxMemoryType + 1];
  UINTN                             MemoryTableSize;
  
  //
  // Get memory type information from variable
  //
  MemoryTypeInformation = MemoryData;
  MemoryTableSize       = sizeof (MemoryData);
  Status = CommonGetVariable (
             EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME,
             &gEfiMemoryTypeInformationGuid,
             &MemoryTableSize,
             MemoryTypeInformation
             );
  //
  // Accumulate maximum amount of memory needed
  //
  if (EFI_ERROR (Status) || !IsMemoryTyepInfoValid (MemoryData, MemoryTableSize / sizeof (EFI_MEMORY_TYPE_INFORMATION))) {
    //
    // Get memory type information from PCD and updated by OemService
    //
    MemoryTypeInformation = (EFI_MEMORY_TYPE_INFORMATION *)PcdGetPtr (PcdPreserveMemoryTable);
    MemoryTableSize =  LibPcdGetSize (PcdToken (PcdPreserveMemoryTable));
    Status = OemSvcUpdateMemoryTypeInformation (&MemoryTypeInformation, (UINT32*)&MemoryTableSize);
    if (!EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }
  }
  for (Index = 0; Index < (MemoryTableSize / sizeof (EFI_MEMORY_TYPE_INFORMATION)); Index++) {
    DEBUG ((EFI_D_ERROR, "[%02x]NumberOfPages = %x\n", Index, MemoryTypeInformation[Index].NumberOfPages));
    *MemorySize += MemoryTypeInformation[Index].NumberOfPages * EFI_PAGE_SIZE;
  }

  return EFI_SUCCESS;
}
//[-end-171023-IB07400920-add]//

/**
  This function installs the EFI Memory

  @param[in]      PeiServices   PEI Services Table.
  @param[in]      BootMode      Indicates the boot mode selected
  @param[in,out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters

  @retval  EFI_SUCCESS      When no errors when installing memory
  @retval  Others           Errors occurred while installing memory
**/
EFI_STATUS
InstallEfiMemory (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN        EFI_BOOT_MODE         BootMode,
  IN  OUT   MMRC_DATA   *MrcData
)
{
  EFI_STATUS                      Status;

  UINT8                           RemapEnable;
  EFI_PHYSICAL_ADDRESS            UpperTotalMemory;
  UINT32                          MmioAllocation;
  EFI_PHYSICAL_ADDRESS            HostIOBoundaryHi;     //HostIOBoundaryHi = TOUUD = BMBOUNDHI
  EFI_PHYSICAL_ADDRESS            HostIOBoundary;       //HostIOBoundary = TOLUD = BMBOUND

  EFI_PHYSICAL_ADDRESS            TSegBase;
  UINT64                          TSegSize;

  EFI_PHYSICAL_ADDRESS            dPeiMemBase;
  UINT64                          PeiMemSize;

  EFI_PHYSICAL_ADDRESS            LowReservedMemoryBase; //Low Reserved Memory Base Address
  EFI_PHYSICAL_ADDRESS            LowSystemMemoryBase;
  UINT64                          LowSystemMemorySize;
  EFI_PHYSICAL_ADDRESS            LowSystemMemoryBase1;
  UINT64                          LowSystemMemorySize1;
  EFI_PHYSICAL_ADDRESS            LowSystemMemoryBase2;
  UINT64                          LowSystemMemorySize2;
  EFI_PHYSICAL_ADDRESS            HighSystemMemoryBase;
  UINT64                          HighSystemMemorySize;
  REGISTER_ACCESS                 Register;


#ifdef FSP_FLAG
  FSPM_UPD                        *FspmUpd;
  UINT32                          BootLoaderTolumSize;
  UINT32                          RemainingSize;
  EFI_PHYSICAL_ADDRESS            BootLoaderTolumBase;
#endif
#if SMM_SUPPORT
  //EFI_PEI_HOB_POINTERS            Hob;
  EFI_SMRAM_HOB_DESCRIPTOR_BLOCK  *SmramHobDescriptorBlock;
#endif
#if (BDAT_SUPPORT == 1)
  EFI_PEI_HOB_POINTERS            BDATHob;
  UINTN                           BDATBufferSize;
#endif
#ifdef PRAM_SUPPORT
  EFI_PHYSICAL_ADDRESS            PramMemBase = 0;
  UINT32                          PramMemSize = 0;
  EFI_PHYSICAL_ADDRESS            *PramBaseAddrHubData = NULL;
  UINTN                           BufferSize = 0;
  UINT8                           PramConfigVariable;
  BIOS_RESERVED_MEMORY_HOB        *BiosReservedMemoryHob = NULL;
#endif

  EFI_PEI_HOB_POINTERS            GuidHob;


#ifdef CRASHDUMP_SUPPORT
  SYSTEM_CONFIGURATION            mSystemConfiguration;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
  UINTN                           VariableSize = 0;
  UINT8                           CrashDumpEnabled = 0;
  EFI_PHYSICAL_ADDRESS            CrashDumpMemBase = 0;
  EFI_PHYSICAL_ADDRESS            CrashDumpMemCpuBase = 0;
  UINT32                          CrashDumpMemSize = 0;
  UINT32                          CrashDumpMemCpuSize = 0x100000;
  EFI_PHYSICAL_ADDRESS            *CrashDumpMemCpuAddrHubData = NULL;
  BOOLEAN                         CrashDumpFlag = FALSE;
  UINT16                          AcpiBaseAddr;
  UINT32                          Data32;
  UINTN                           CrashDumpBufferSize = 0;
#endif
//[-start-160429-IB07220075-add]//
  UINTN                           RequiredMemSize;
//[-end-160429-IB07220075-add]//

  CopyMem (&GuidHob, 0, sizeof(EFI_PEI_HOB_POINTERS));

  Status =  MMRC_SUCCESS;
  RemapEnable = FALSE;
  UpperTotalMemory = 0;
  MmioAllocation = MrcData->OemMrcData.MmioSize;

#ifdef CRASHDUMP_SUPPORT
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  VariableSize = sizeof (SYSTEM_CONFIGURATION);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               PLATFORM_SETUP_VARIABLE_NAME,
                               &gEfiSetupVariableGuid,
                               NULL,
                               &VariableSize,
                               &mSystemConfiguration
                               );

  if (!EFI_ERROR (Status)) {
    CrashDumpEnabled = mSystemConfiguration.CrashDump;
  }

  if (CrashDumpEnabled == 1) {
    AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
    Data32 = IoRead32 (AcpiBaseAddr + R_TCO_STS);
    if ((Data32 & B_TCO_STS_SECOND_TO) == B_TCO_STS_SECOND_TO) {
      IoWrite8(CmosIo_72, EFI_CMOS_CRASHDUMP_TRIGGERED);
      if (IoRead8(CmosIo_73) == 0x5A) {
        CrashDumpFlag = TRUE;
      }
    }
  }
#endif

#ifdef FSP_FLAG
  PeiMemSize = PcdGet32(PcdFspReservedMemoryLength);

  //Get the OemMemory to be reserved
  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
  BootLoaderTolumSize = FspmUpd->FspmArchUpd.BootLoaderTolumSize;
  DEBUG ((EFI_D_INFO, "BootLoaderTolumSize:   0x%x\n", BootLoaderTolumSize));
  //
  // BootLoaderTolumSize needs to be page aligned.
  //
  if (BootLoaderTolumSize > 0) {
    RemainingSize = BootLoaderTolumSize % EFI_PAGE_SIZE;
    if (RemainingSize != 0) {
      BootLoaderTolumSize = BootLoaderTolumSize - RemainingSize + EFI_PAGE_SIZE;
    }
  }
#else
//[-start-160429-IB07220075-modify]//
//[-start-171023-IB07400920-modify]//
  PeiMemSize = 0;
  GetPlatformMemorySize (PeiServices, &PeiMemSize);
  PeiMemSize = 0x10000000 + PeiMemSize; //256MB + Pre-Reserved Memroy
  DEBUG ((EFI_D_ERROR,"PeiMemSize = %x\n", PeiMemSize));
//[-end-171023-IB07400920-modify]//
//[-end-160429-IB07220075-modify]//
#endif

//[-start-160429-IB07220075-add]//
  RequiredMemSize = (UINTN) (PeiMemSize - RShiftU64 (PeiMemSize, 2));
//[-end-160429-IB07220075-add]//

  LowReservedMemoryBase = 0;
  LowSystemMemoryBase = 0;
  LowSystemMemorySize = 0;
  HighSystemMemoryBase = 0;
  HighSystemMemorySize = 0;

  UpperTotalMemory  = (EFI_PHYSICAL_ADDRESS) LShiftU64 (MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize, 20);

  Register.Mask = 0xFFFFFFFF;
  Register.ShiftBit = 0;

  Register.Offset = TOUUD_HI_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  HostIOBoundaryHi = (EFI_PHYSICAL_ADDRESS)LShiftU64 (MemRegRead (CUNIT_BOXTYPE, 0, 0, Register), 32);
  Register.Offset = TOUUD_LO_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  HostIOBoundaryHi = (EFI_PHYSICAL_ADDRESS) (HostIOBoundaryHi + (MemRegRead (CUNIT_BOXTYPE, 0, 0, Register) & 0xFFF00000));

  Register.Offset = TOLUD_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  HostIOBoundary= MemRegRead (CUNIT_BOXTYPE, 0, 0, Register) & 0xFFF00000;

  Register.Offset = TSEGMB_0_0_0_PCI_CUNIT_BOXTYPE_REG;
  TSegBase = MemRegRead (CUNIT_BOXTYPE, 0, 0, Register) & 0xFFF00000;

//[-start-160301-IB07220042-modify]//
  if (IsRunCrisisRecoveryMode()) {
    Register.Offset = BGSM_0_0_0_PCI_CUNIT_BOXTYPE_REG;
//[-start-160802-IB03090430-modify]//
    TSegSize = (MemRegRead (CUNIT_BOXTYPE, 0, 0, Register) & 0xFFF00000) - TSegBase;
//[-end-160802-IB03090430-modify]//
  } else {
    TSegSize = (UINT64)(MrcData->OemMrcData.TsegSize * 0x100000);
  }
//[-end-160301-IB07220042-modify]//
/*  
  DEBUG ((EFI_D_INFO, "MmioAllocation:   0x%08X\n", MmioAllocation));
  DEBUG ((EFI_D_INFO, "UpperTotalMemory: 0x%08X",   UpperTotalMemory >> 32));
  DEBUG ((EFI_D_INFO, "%08X\n",                     UpperTotalMemory));
  DEBUG ((EFI_D_INFO, "HostIOBoundary/TOUUD/BMBOUNDHI: 0x%08X\n", HostIOBoundary));
  DEBUG ((EFI_D_INFO, "HostIOBoundaryHi/TOLUD/BMBOUND: 0x%08X",   HostIOBoundaryHi >> 32));
  DEBUG ((EFI_D_INFO, "%08X\n", HostIOBoundaryHi));
  DEBUG ((EFI_D_INFO, "TSegBase: 0x%08X\n", TSegBase));
  DEBUG ((EFI_D_INFO, "TSegSize: 0x%08X\n", TSegSize));
*/
#if (VP_BIOS_ENABLE == 0) || (TABLET_PF_ENABLE == 0)
  if (UpperTotalMemory > HostIOBoundary) {
      RemapEnable = TRUE;
      DEBUG ((EFI_D_INFO, "RemapEnable is TRUE\n"));
  }
#endif
  DEBUG ((EFI_D_INFO, "-------------------Memory Mapping----------------\n"));

  //
  // Report first 640KB of memory
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS) (0),
    (UINT64) (0xA0000)
  );

 // DEBUG ((EFI_D_INFO, "System Memory:   0x%lx -- 0x%lx\n", (UINT64) 0x0, (UINT64) (0xA0000 - 1) ));

  //
  // Report first 0A0000h - 0FFFFFh as RESERVED memory
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS) (0xA0000),
    (UINT64) (0x60000)
  );

  //DEBUG ((EFI_D_INFO, "Reserved Memory: 0x%lx -- 0x%lx\n", (UINT64) 0xA0000, (UINT64) (0xA0000 + 0x60000 - 1)));

  // *------------+ HostIOBoundary
  // *            +
  // *------------+ GMS Base
  // *            +
  // *------------+ GTT Base
  // *            +
  // *------------+ RTIT Base
  // *            +
  // *------------+ TSEG Base
  // *            +




#ifdef FSP_FLAG
  if (TSegSize) {

    BuildResourceDescriptorWithOwnerHob (
      EFI_RESOURCE_MEMORY_RESERVED,
      (
        EFI_RESOURCE_ATTRIBUTE_PRESENT |
        EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
        EFI_RESOURCE_ATTRIBUTE_TESTED |
        EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE
      ),
      TSegBase,
      TSegSize,
      &gReservedMemoryResourceHobTsegGuid
    );

    BuildResourceDescriptorHob (
      EFI_RESOURCE_MEMORY_RESERVED,
      MEM_DET_COMMON_MEM_ATTR,
      (EFI_PHYSICAL_ADDRESS)(TSegBase + TSegSize),
      (UINT64)(HostIOBoundary - TSegBase - TSegSize)
    );
  }
#else
  //
  //Update Memory Region from TSEG Base to HostIOBoundary as reserved memory
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)(TSegBase),
    (UINT64)(HostIOBoundary - TSegBase)
    );
#endif
  //DEBUG ((EFI_D_INFO, "Reserved Memory: 0x%lx -- 0x%lx\n", (UINT64) TSegBase, (UINT64)(HostIOBoundary-1) ));

  //
  // LowReservedMemoryBase set to TsegBase which is the lowest Reserved memory base at this point
  //
  LowReservedMemoryBase = TSegBase;


  //
  // Report (TOM-TSEG_SIZE) to TSEG_SIZE as reserved memory (SMRAM TSEG)
  //
  if (TSegSize) {
    SmramHobDescriptorBlock = BuildGuidHob (
                &gEfiSmmPeiSmramMemoryReserveGuid,
                sizeof (EFI_SMRAM_HOB_DESCRIPTOR_BLOCK) + sizeof (EFI_SMRAM_DESCRIPTOR)
              );
    if (SmramHobDescriptorBlock != NULL ) {
      SmramHobDescriptorBlock->NumberOfSmmReservedRegions = (UINT32) (1);
      SmramHobDescriptorBlock->Descriptor[0].PhysicalStart = (EFI_PHYSICAL_ADDRESS) (TSegBase);
      SmramHobDescriptorBlock->Descriptor[0].CpuStart      = (EFI_PHYSICAL_ADDRESS) (TSegBase);
      SmramHobDescriptorBlock->Descriptor[0].PhysicalSize  = (UINT64) TSegSize;
      SmramHobDescriptorBlock->Descriptor[0].RegionState   = (UINT64) (EFI_SMRAM_CLOSED);

      DEBUG ((EFI_D_INFO, "dTSeg:           0x%lx -- 0x%lx\n", (UINT64) TSegBase, (UINT64) TSegBase + TSegSize - 1));
    } else {
      DEBUG ((EFI_D_INFO, "Failed to build Smram Hob Descriptor.\n"));
      MrcDeadLoop ();
    }
  }// TSegSize

#ifdef PRAM_SUPPORT
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gBiosReserveMemoryHobGuid, GuidHob.Raw)) != NULL) {
      BiosReservedMemoryHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  if (BiosReservedMemoryHob != NULL) {
    PramConfigVariable = BiosReservedMemoryHob->Pram;

    if (PramConfigVariable != 0x30)
    {
      switch (PramConfigVariable)
      {
      case 0x31: //4MB
        PramMemSize = 0x400000;
        break;

      case 0x32: //16MB
        PramMemSize = 0x1000000;
        break;

      case 0x33: //64MB
        PramMemSize = 0x4000000;
        break;

      default:
        PramMemSize = 0;
        break;
      }

      PramMemBase = LowReservedMemoryBase - PramMemSize;

      BuildResourceDescriptorHob (
        EFI_RESOURCE_MEMORY_RESERVED,
        MEM_DET_COMMON_MEM_ATTR,
        (EFI_PHYSICAL_ADDRESS)(PramMemBase),
        (UINT64)(PramMemSize)
        );

      BufferSize = sizeof (EFI_PHYSICAL_ADDRESS);
      PramBaseAddrHubData = BuildGuidHob (&gPramAddrDataGuid, BufferSize);
      if (PramBaseAddrHubData != NULL) {
        *PramBaseAddrHubData = PramMemBase;
      } else {
        DEBUG ((EFI_D_INFO, "Failed to build Pram Base Hub.\n"));
        MrcDeadLoop ();
      }
      LowReservedMemoryBase = PramMemBase;
    }
  }
#endif


#ifdef CRASHDUMP_SUPPORT
  //
  // Offline Crash Dump Memory
  //
  if (CrashDumpEnabled == 1) {
    CrashDumpMemSize = (UINT32)PeiMemSize;
    CrashDumpMemBase = LowReservedMemoryBase - CrashDumpMemSize;
    if (!CrashDumpFlag) {
      BuildResourceDescriptorHob (
        EFI_RESOURCE_MEMORY_RESERVED,
        MEM_DET_COMMON_MEM_ATTR,
        (EFI_PHYSICAL_ADDRESS)(CrashDumpMemBase),
        (UINT64)(CrashDumpMemSize)               // 256MB for Crash Dump minimum boot
        );
    }

    DEBUG ((
      EFI_D_INFO,
      "Crash Dump Memory Region:     0x%lx -- 0x%lx\n",
      CrashDumpMemBase,
      CrashDumpMemBase + CrashDumpMemSize - 1
    ));

    LowReservedMemoryBase = CrashDumpMemBase;

    //
    // Reserve 1MB memory for save Crash Dump CPU context registers
    //
    CrashDumpMemCpuBase = LowReservedMemoryBase - CrashDumpMemCpuSize;
    BuildResourceDescriptorHob (
      EFI_RESOURCE_MEMORY_RESERVED,
      MEM_DET_COMMON_MEM_ATTR,
      (EFI_PHYSICAL_ADDRESS)(CrashDumpMemCpuBase),
      (UINT64)(CrashDumpMemCpuSize)
      );

    DEBUG ((EFI_D_INFO, "LowReservedMemoryBase = 0x%lX, CrashDumpMemCpuBase = 0x%lx\n", LowReservedMemoryBase, CrashDumpMemCpuBase));

    CrashDumpBufferSize = sizeof (EFI_PHYSICAL_ADDRESS);
    CrashDumpMemCpuAddrHubData = BuildGuidHob (&gEfiCrashDumpAddrGuid, CrashDumpBufferSize);

    if (CrashDumpMemCpuAddrHubData != NULL) {
    *CrashDumpMemCpuAddrHubData = CrashDumpMemCpuBase;
    } else {
      DEBUG ((EFI_D_INFO, "Failed to build Crash Dump Memory CPU Hob.\n"));
      MrcDeadLoop ();
    }
    LowReservedMemoryBase = CrashDumpMemCpuBase;
  }
#endif




  //
  // Change PeiMemory location for EFI-complaint Grub Bootloader, from LowReservedMemoryBase with length 64M
  //
#ifndef FSP_FLAG
//[-start-151224-IB07220029-modify]//
  if (IsRunCrisisRecoveryMode()) {
    PeiMemSize = LShiftU64 (PeiMemSize, 1);
    dPeiMemBase = LowReservedMemoryBase - PeiMemSize;    
    PeiMemSize = RShiftU64 (PeiMemSize, 1);
  } else {
    dPeiMemBase = LowReservedMemoryBase - PeiMemSize;
  }
//[-end-151224-IB07220029-modify]//
  if (dPeiMemBase <= 0x100000) {
    dPeiMemBase = 0x100000;
  }
#else
  BootLoaderTolumBase = LowReservedMemoryBase - BootLoaderTolumSize;
  dPeiMemBase = BootLoaderTolumBase - PeiMemSize;
  if (dPeiMemBase <= 0x100000) {
     dPeiMemBase = 0x100000;
  }
  LowReservedMemoryBase = dPeiMemBase;
#endif

#ifdef FSP_FLAG
  //Oem Reserved Memory
  if(BootLoaderTolumSize)
  {
    BuildResourceDescriptorWithOwnerHob (
       EFI_RESOURCE_MEMORY_RESERVED,
       (
         EFI_RESOURCE_ATTRIBUTE_PRESENT |
         EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
         EFI_RESOURCE_ATTRIBUTE_TESTED |
         EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
         EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
         EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
         EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE
       ),
       BootLoaderTolumBase,
       BootLoaderTolumSize,
       &gFspBootLoaderTolumHobGuid
     );
    DEBUG ((
      EFI_D_INFO,
      "BootLoader Reserved Memory:   0x%lx -- 0x%lx\n",
      (UINT64) BootLoaderTolumBase,
      (UINT64) BootLoaderTolumBase + BootLoaderTolumSize - 1
    ));
  }

  BuildResourceDescriptorWithOwnerHob (
     EFI_RESOURCE_MEMORY_RESERVED,
     (
       EFI_RESOURCE_ATTRIBUTE_PRESENT |
       EFI_RESOURCE_ATTRIBUTE_INITIALIZED |
       EFI_RESOURCE_ATTRIBUTE_TESTED |
       EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE |
       EFI_RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
       EFI_RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
       EFI_RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE
     ),
     dPeiMemBase,
     PeiMemSize,
     &gFspReservedMemoryResourceHobGuid
   );
#endif

#ifdef CRASHDUMP_SUPPORT
  if ((CrashDumpEnabled == 1) && (CrashDumpFlag)) {
    DEBUG ((EFI_D_INFO,"Preparing CrashDump...\n"));
    if (BootMode != BOOT_ON_S3_RESUME) {
      Status = (*PeiServices)->InstallPeiMemory(PeiServices, CrashDumpMemBase, PeiMemSize);
      if (Status != EFI_SUCCESS) {
        MrcDeadLoop();
      }
    }

    // Update Hob as system memory
    BuildResourceDescriptorHob (
      EFI_RESOURCE_SYSTEM_MEMORY,
      MEM_DET_COMMON_MEM_ATTR,
      CrashDumpMemBase,
      (UINT64)(CrashDumpMemSize)  //256M
    );
  } else {
#endif
  //
  // Report the memory to EFI
  //
#ifndef FSP_FLAG
  if (BootMode != BOOT_ON_S3_RESUME) {
#else
  //
  // Install normal memory on S3 path for FSP
  //
  if (TRUE) {
#endif
//[-start-160429-IB07220075-modify]//
    Status = (*PeiServices)->InstallPeiMemory ((CONST EFI_PEI_SERVICES **)PeiServices, dPeiMemBase, PeiMemSize - RequiredMemSize);
//[-end-160429-IB07220075-modify]//
    if (Status != EFI_SUCCESS) {
      MrcDeadLoop ();
    }
#ifndef FSP_FLAG
  } else {
    Status = InstallS3Memory(PeiServices, MrcData);
    ASSERT_EFI_ERROR (Status);
#endif
  }

  //
  // Memory Mapping
  //
  // *------------+ HostIOBoundaryHi (If RemapEnable == TRUE)
  // *            +
  // *------------+ 4G = HighSystemMemoryBase
  // *            +
  // *            +   [MMIO Memory]
  // *            +
  // *------------+ HostIOBoundary
  // *            +
  // *------------+ GMS Base
  // *            +
  // *------------+ GTT Base
  // *            +
  // *------------+ TSEG Base
  // *            +
  // *------------+ PRAM Base = LowReservedMemoryBase  (if PRMA enable)
  // *            +
  // *            + BootLoaderTolumBase (if BootLoaderTolumSize!=0)
  // *            +
  // *------------+ dPeiMemBase Base
  // *            +
  // *            +
  // *            +   [System Memory]
  // *            +
  // *------------+ 0x1000 (4K)    [SV S3 memory test region]
  // *            +   [SEC UMA]
  // *------------+ 0x400000 (256M)
  // *            +
  // *------------+ 0x100000 (1M) = LowSystemMemoryBase
  // *            +   [Reserved Memory]
  // *------------+ 0xA0000
  // *            +   [System Memory]
  // *------------+ 0

  LowSystemMemoryBase = 0x100000;
  LowSystemMemorySize = LowReservedMemoryBase - 0x100000;

  if (RemapEnable == TRUE) {
    HighSystemMemoryBase = 0x100000000;
    HighSystemMemorySize = HostIOBoundaryHi - 0x100000000;
  }
  /*
  DEBUG ((
    EFI_D_INFO,
    "PeiMemory:       0x%lx -- 0x%lx\n",
    (UINT64) dPeiMemBase,
    (UINT64) (dPeiMemBase + PeiMemSize - 1)
  ));  */

//[-start-160429-IB07220075-add]//
  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    MEM_DET_COMMON_MEM_ATTR,
    (EFI_PHYSICAL_ADDRESS)dPeiMemBase,
    PeiMemSize
    );
//[-end-160429-IB07220075-add]//
  //
  // Report 1MB to LowReservedMemoryBase as system memory
  //
  LowSystemMemoryBase1 = LowSystemMemoryBase;
  LowSystemMemorySize1 = MrcData->SeCUmaBase - LowSystemMemoryBase1;
  LowSystemMemoryBase2 = MrcData->SeCUmaBase + MrcData->SeCUmaSize;
  LowSystemMemorySize2 = LowSystemMemoryBase + LowSystemMemorySize - LowSystemMemoryBase2;

  BuildResourceDescriptorHob (
    EFI_RESOURCE_SYSTEM_MEMORY,
    MEM_DET_COMMON_MEM_ATTR,
    LowSystemMemoryBase1,
    LowSystemMemorySize1
  );

  DEBUG ((
    EFI_D_INFO,
    "System Memory:   0x%lx -- 0x%lx\n",
    (UINT64) LowSystemMemoryBase1,
    (UINT64) LowSystemMemoryBase1 + LowSystemMemorySize1 - 1
  ));
//[-start-150417-IB03090000-modify]//
#ifdef WIN7_SUPPORT
if (MrcData->SeCUmaSize != 0) {
#endif
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_RESERVED,
    MEM_DET_COMMON_MEM_ATTR,
    MrcData->SeCUmaBase,
    MrcData->SeCUmaSize
  );

  DEBUG ((
    EFI_D_INFO,
    "CSEUma Memory:   0x%lx -- 0x%lx\n",
    (UINT64) MrcData->SeCUmaBase,
    (UINT64) MrcData->SeCUmaBase + MrcData->SeCUmaSize - 1
  ));
#ifdef WIN7_SUPPORT  
}
#endif
//[-end-150417-IB03090000-modify]//
//[-start-160429-IB07220075-modify]//
BuildResourceDescriptorHob (
  EFI_RESOURCE_SYSTEM_MEMORY,
  MEM_DET_COMMON_MEM_ATTR,
  LowSystemMemoryBase2,
  LowSystemMemorySize2 - PeiMemSize
);
//[-end-160429-IB07220075-modify]//
/*
  DEBUG ((
    EFI_D_INFO,
    "System Memory:   0x%lx -- 0x%lx\n",
    (UINT64) LowSystemMemoryBase2,
    (UINT64) LowSystemMemoryBase2 + LowSystemMemorySize2 - 1
  ));
*/
#ifdef CRASHDUMP_SUPPORT
}
#endif
  if (RemapEnable == TRUE) {
    if (HighSystemMemorySize > 0) {
      BuildResourceDescriptorHob (
        EFI_RESOURCE_SYSTEM_MEMORY,
        MEM_DET_COMMON_MEM_ATTR1,
        HighSystemMemoryBase,
        HighSystemMemorySize
      );
      DEBUG ((
        EFI_D_INFO,
        "UpperMemory:     0x%lx -- 0x%lx\n",
        (UINT64) HighSystemMemoryBase,
        (UINT64) (HighSystemMemoryBase) + HighSystemMemorySize - 1
      ));
    }
  } // RemapEnable

#if (BDAT_SUPPORT == 1)
  //
  // BDAT 4.0 STRUCT Test
  //
  BDATBufferSize = GetRMTStructSize(MrcData);
  DEBUG ((EFI_D_INFO, "Filling/Saving BDAT 4.0 Structure of size %d\n", BDATBufferSize));
  BDATHob.Raw = BuildGuidHob (
    &gSsaResultGuid,
    BDATBufferSize
    );
  FillRMTBdatStructure(MrcData, BDATHob.Raw, BDATBufferSize);
#endif

#ifdef FSP_FLAG

  {
    UINT64              LowMemoryLength;
    UINT64              HighMemoryLength;
    FspGetSystemMemorySize (&LowMemoryLength,&HighMemoryLength);
  }
#endif
  return EFI_SUCCESS;
}

#ifndef FSP_FLAG
#if !TEMP_DISABLED
/**
  This function installs the EFI Memory when S3 path is executed

  @param[in]      PeiServices  PEI Services Table.
  @param[in,out]  MrcData      Pointer to MRC Output Data that contains MRC Parameters.

  @retval  EFI_SUCCESS         When no errors when installing memory
  @retval  Others              Errors occurred while installing memory
**/
EFI_STATUS
InstallS3Memory (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
)
{
  EFI_STATUS                      Status;
#if (ENBDT_PF_ENABLE == 1)
  ACPI_VARIABLE_SET               *AcpiVariableSet;
  UINTN                           S3MemoryBase = 0;
  UINTN                           S3MemorySize = 0;
  EFI_PEI_HOB_POINTERS            GuidHob;
  EFI_PHYSICAL_ADDRESS            *AcpiVariableAddress;
  UINTN                           AcpiVarHobSize;
#endif

  Status          = EFI_SUCCESS;

#if (ENBDT_PF_ENABLE == 1)
  AcpiVariableSet = NULL;

  DEBUG ((EFI_D_INFO, "InstallS3Memory()\n"));

  GuidHob.Raw = GetHobList ();
  AcpiVariableAddress = NULL;
  AcpiVarHobSize = 0;

  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiAcpiVariableCompatiblityGuid, GuidHob.Raw)) != NULL) {
      AcpiVariableAddress = GET_GUID_HOB_DATA (GuidHob.Guid);
      AcpiVarHobSize = GET_GUID_HOB_DATA_SIZE (GuidHob.Guid);
    }
  }

  DEBUG ((EFI_D_INFO, "InstallS3Memory() read AcpiVariableAddress\n"));
  if (AcpiVariableAddress != NULL) {
    DEBUG ((EFI_D_INFO, "0x%08x\n",(UINT32)((UINTN)AcpiVariableAddress)));
    DEBUG ((EFI_D_INFO, "* 0x%08x\n",(UINT32)((UINTN)*AcpiVariableAddress)));
    AcpiVariableSet = (ACPI_VARIABLE_SET *)((UINTN)*AcpiVariableAddress);
    S3MemoryBase = (UINTN) (AcpiVariableSet->AcpiReservedMemoryBase);
    S3MemorySize = (UINTN) (AcpiVariableSet->AcpiReservedMemorySize);
  } else {
    AcpiVariableSet = NULL;
    DEBUG ((EFI_D_INFO, " is NULL\n"));
  }

  //
  // Report Memory to EFI
  //
  Status = (*PeiServices)->InstallPeiMemory (PeiServices, S3MemoryBase, S3MemorySize);
  ASSERT_EFI_ERROR (Status);
#endif

  return Status;
}
#endif
#endif

/**
  This function restores the MRC parameters previously saved in the memory config variable.

  @param  PeiServices   - PEI Services Table.
  @param  MrcData       - Pointer to MRC Output Data that contains MRC Parameters.

  @retval EFI_SUCCESS   - MRC parameters restored correctly
  @retval Others        - Otherwise
**/
EFI_STATUS
MrcParamsRestore (
  IN        EFI_PEI_SERVICES    **PeiServices,
  IN  OUT   MMRC_DATA           *MrcData
  )
{
  EFI_STATUS                      Status;
  DRAM_POLICY_PPI                 *DramPolicyPpi;
  MRC_PARAMS_SAVE_RESTORE         *MrcParamsHob;
  BOOT_VARIABLE_NV_DATA           *BootVariableNvDataHob;
  UINT32                          MrcParamsSaveRestoreCrc;
  UINT32                          BootVariableNvDataCrc;
  UINT32                          SaMemCfgCrc;
  EFI_BOOT_MODE                   BootMode;

  Status = (*PeiServices)->LocatePpi (
                             (CONST EFI_PEI_SERVICES **)PeiServices,
                             &gDramPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **)&DramPolicyPpi
                             );

  Status = (*PeiServices)->GetBootMode (
             (CONST EFI_PEI_SERVICES **) PeiServices,
             &BootMode
           );

  if ((EFI_ERROR (Status)) || ((VOID *)(UINT32)DramPolicyPpi->MrcTrainingDataPtr == NULL || (BootMode == BOOT_ON_S3_RESUME && (VOID *)(UINT32)DramPolicyPpi->MrcBootDataPtr == NULL))) {
    DEBUG ((EFI_D_ERROR, "MRC Parameters not valid. Status is %r\n", Status));
  } else {

    DEBUG((EFI_D_INFO, "Recovering MRC Parameters from DRAM_POLICY_PPI...\n"));
    // Copy the data locally before computing the CRC checksum.
    // This avoids unecessary accesses to the CSE SRAM
    // Note: the MRC_NV_DATA_FRAME struct describes all of the NV data in the
    //       MMRC_DATA MrcData variable. Contains two sections of data which are
    //       stored as independent variables in the SaveMemoryConfig DXE driver


    //
    // Validate BootVariableNvDataCrc if MrcBootDataPtr is non-NULL
    //
    if ((VOID *)(UINT32)DramPolicyPpi->MrcBootDataPtr != NULL) {
      BootVariableNvDataHob = (BOOT_VARIABLE_NV_DATA*)((UINT32)DramPolicyPpi->MrcBootDataPtr);
      CopyMem(&(MrcData->NvData.BootVariableNvData), BootVariableNvDataHob, sizeof (BOOT_VARIABLE_NV_DATA));
      BootVariableNvDataCrc = MrcData->NvData.BootVariableNvData.BootVariableNvDataCrc;
      MrcData->NvData.BootVariableNvData.BootVariableNvDataCrc = 0;
      if (BootVariableNvDataCrc != MrcCalculateCrc32((UINT8 *)&(MrcData->NvData.BootVariableNvData), sizeof (BOOT_VARIABLE_NV_DATA))){
        DEBUG ((EFI_D_INFO, "BootVariableNvDataCrc is invalid...\n" ));
        MrcData->MrcParamsValidFlag = 0;
        return EFI_NOT_READY;
      }
    }

    //
    // Validate MrcParamsSaveRestoreCrc and SaMemCfgCrc
    //
    MrcParamsHob = (MRC_PARAMS_SAVE_RESTORE*)((UINT32)DramPolicyPpi->MrcTrainingDataPtr);
    CopyMem(&(MrcData->NvData.MrcParamsSaveRestore), MrcParamsHob, sizeof (MRC_PARAMS_SAVE_RESTORE));

    DramPolicyPpi->MrcTrainingDataPtr = (EFI_PHYSICAL_ADDRESS) 0;
    DramPolicyPpi->MrcBootDataPtr = (EFI_PHYSICAL_ADDRESS) 0;

    SaMemCfgCrc = MrcData->NvData.MrcParamsSaveRestore.SaMemCfgCrc;
    if (SaMemCfgCrc != MrcCalculateCrc32((UINT8 *)DramPolicyPpi, sizeof (DRAM_POLICY_PPI))) {
      DEBUG ((EFI_D_INFO, "SaMemCfgCrc is invalid...\n" ));
      MrcData->MrcParamsValidFlag = 0;
      return EFI_NOT_READY;
    }


    MrcParamsSaveRestoreCrc = MrcData->NvData.MrcParamsSaveRestore.MrcParamsSaveRestoreCrc;
    MrcData->NvData.MrcParamsSaveRestore.MrcParamsSaveRestoreCrc = 0;

    if (MrcParamsSaveRestoreCrc != MrcCalculateCrc32((UINT8 *)&(MrcData->NvData.MrcParamsSaveRestore), sizeof (MRC_PARAMS_SAVE_RESTORE))) {
      DEBUG ((EFI_D_INFO, "MrcParamsSaveRestoreCrc is invalid...\n" ));
      MrcData->MrcParamsValidFlag = 0;
      return EFI_NOT_READY;
    }

    if (MrcData->NvData.MrcParamsSaveRestore.MrcVer != GetMrcVersions(MrcData)) {
      DEBUG ((EFI_D_INFO, "MrcVer: %8xh != %8xh\n", MrcData->NvData.MrcParamsSaveRestore.MrcVer, GetMrcVersions(MrcData)));
      MrcData->MrcParamsValidFlag = 0;
      return EFI_NOT_READY;
    }
    if ( MrcData->NvData.MrcParamsSaveRestore.Signature != SIGNATURE_32 ('T', 'R', 'N', 'D') ) {
      MrcData->MrcParamsValidFlag = 0;
      return EFI_NOT_FOUND;
    }
#if RMT_PRODUCTION_MODE_SUPPORT == 1
    // MarginCheckResultState is populated by MmrcEntry. If the L1_CHECK failed,
    // on the last boot then the DRAM margin is critically low. Force an
    // S5 boot in this case.
    if (MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState == L1_CHECK || MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState == BOTH_CHECK) {
      DEBUG ((EFI_D_INFO, "Margin\n" ));
      MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = 0;
      return EFI_DEVICE_ERROR;
    }
#endif //RMT_PRODUCTION_MODE_SUPPORT
    MrcData->MrcParamsValidFlag = 1;
  }

  return EFI_SUCCESS;
}

/**
  This function saves the MRC Parameters to Hob for later use.

  @param[in]   PeiServices   PEI Services Table.
  @param[out]  MrcData       Pointer to MRC Output Data that contains MRC Parameters

  @retval  EFI_SUCCESS  - Hob is successfully built.
  @retval  Others       - Errors occur while creating new Hob
**/
EFI_STATUS
MrcParamsSave (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN  OUT   MMRC_DATA   *MrcData
  )
{
#ifndef FSP_FLAG
  MRC_NV_DATA_FRAME         *MrcParamsHob;
  EFI_PEI_HOB_POINTERS      Hob;
  UINTN                     BufferSize;
#endif
  EFI_STATUS                Status;
  DRAM_POLICY_PPI           *DramPolicyPpi;
#ifdef FSP_FLAG
  EFI_PEI_HOB_POINTERS      NvHobPtr;
  EFI_PEI_HOB_POINTERS      VariableNvHobPtr;
  MRC_PARAMS_SAVE_RESTORE   *NvHob;
  BOOT_VARIABLE_NV_DATA     *VariableNvHob;
#endif

#ifndef FSP_FLAG
  BufferSize = sizeof (MRC_NV_DATA_FRAME);

  Hob.Raw = BuildGuidHob(&gEfiMemoryConfigDataGuid, BufferSize);
  ASSERT (Hob.Raw);

  MrcParamsHob = (MRC_NV_DATA_FRAME *) Hob.Raw;


  CopyMem (MrcParamsHob, &(MrcData->NvData), sizeof (MRC_NV_DATA_FRAME));
  //Validate the saved data
  MrcParamsHob->MrcParamsSaveRestore.Signature = SIGNATURE_32 ('T', 'R', 'N', 'D');

  //
  // Save compile time information to allow Fast Boot Data compatibility check
  //

  // Store structure data size info to catch struct size missmatches in DXE phase at runtime
  MrcParamsHob->MrcParamsSaveRestore.DataSize = sizeof(MRC_NV_DATA_FRAME);

  // MrcVer is also used for the BIOS set-up screen
  MrcParamsHob->MrcParamsSaveRestore.MrcVer = GetMrcVersions(MrcData);
#else
  //
  // FSP builds two HOBs for boot laoder.
  //

  //
  //1.Create HOB : gFspNonVolatileStorageHobGuid to store MrcParamsSaveRestore
  //
  NvHobPtr.Raw            = BuildGuidHob(&gFspNonVolatileStorageHobGuid,sizeof(MRC_PARAMS_SAVE_RESTORE));
  ASSERT (NvHobPtr.Raw);

  NvHob = (MRC_PARAMS_SAVE_RESTORE *) NvHobPtr.Raw;
  CopyMem (NvHob, &(MrcData->NvData.MrcParamsSaveRestore), sizeof (MRC_PARAMS_SAVE_RESTORE));

  NvHob->Signature = SIGNATURE_32 ('T', 'R', 'N', 'D');
  NvHob->DataSize = sizeof(MRC_NV_DATA_FRAME);
  NvHob->MrcVer = GetMrcVersions(MrcData);

  //
  //2.Create HOB : gFspVariableNvDataHobGuid to store BootVariableNvData
  //
  VariableNvHobPtr.Raw  = BuildGuidHob(&gFspVariableNvDataHobGuid,sizeof(BOOT_VARIABLE_NV_DATA));
  ASSERT (VariableNvHobPtr.Raw);

  VariableNvHob = (BOOT_VARIABLE_NV_DATA *) VariableNvHobPtr.Raw;
  CopyMem (VariableNvHob, &(MrcData->NvData.BootVariableNvData), sizeof (BOOT_VARIABLE_NV_DATA));

#endif


  //Update policy CRC
  Status = (*PeiServices)->LocatePpi(
    (CONST EFI_PEI_SERVICES **)PeiServices,
    &gDramPolicyPpiGuid,
    0,
    NULL,
    (VOID **)&DramPolicyPpi
    );

  ASSERT_EFI_ERROR (Status);

  //
  // Ignore MrcTrainingDataPtr and MrcBootDataPtr in CRC calculation since this may change from boot to boot:
  //
  DramPolicyPpi->MrcTrainingDataPtr = (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;
  DramPolicyPpi->MrcBootDataPtr     = (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;

#ifndef FSP_FLAG
  //
  // Also calculate CRC on the MRC NV data which is used to restore the trained
  // register settings on S0/FB/S3 flows
  //
  MrcParamsHob->BootVariableNvData.BootVariableNvDataCrc = 0;
  MrcParamsHob->BootVariableNvData.BootVariableNvDataCrc = MrcCalculateCrc32 ((UINT8 *) &MrcParamsHob->BootVariableNvData, sizeof (BOOT_VARIABLE_NV_DATA));

  MrcParamsHob->MrcParamsSaveRestore.SaMemCfgCrc = MrcCalculateCrc32 ((UINT8 *) DramPolicyPpi, sizeof (DRAM_POLICY_PPI));

  MrcParamsHob->MrcParamsSaveRestore.MrcParamsSaveRestoreCrc = 0;
  MrcParamsHob->MrcParamsSaveRestore.MrcParamsSaveRestoreCrc = MrcCalculateCrc32 ((UINT8 *) &MrcParamsHob->MrcParamsSaveRestore, sizeof (MRC_PARAMS_SAVE_RESTORE));
  DEBUG ((EFI_D_INFO, "MrcParamsSaveRestoreCrc on save = 0x%x.\n", MrcParamsHob->MrcParamsSaveRestore.MrcParamsSaveRestoreCrc));
  DEBUG ((EFI_D_INFO, "SaMemCfgCrc on save = 0x%x.\n", MrcParamsHob->MrcParamsSaveRestore.SaMemCfgCrc));
  DEBUG ((EFI_D_INFO, "BootVariableNvDataCrc on save = 0x%x.\n", MrcParamsHob->BootVariableNvData.BootVariableNvDataCrc));
#else
  //
  // Also calculate CRC on the MRC NV data which is used to restore the trained
  // register settings on S0/FB/S3 flows
  //
  VariableNvHob->BootVariableNvDataCrc = 0;
  VariableNvHob->BootVariableNvDataCrc = MrcCalculateCrc32 ((UINT8 *) VariableNvHob, sizeof (BOOT_VARIABLE_NV_DATA));

  NvHob->SaMemCfgCrc = MrcCalculateCrc32 ((UINT8 *) DramPolicyPpi, sizeof (DRAM_POLICY_PPI));

  NvHob->MrcParamsSaveRestoreCrc = 0;
  NvHob->MrcParamsSaveRestoreCrc = MrcCalculateCrc32 ((UINT8 *) NvHob, sizeof (MRC_PARAMS_SAVE_RESTORE));

  DEBUG ((EFI_D_INFO, "MrcParamsSaveRestoreCrc on save = 0x%x.\n", NvHob->MrcParamsSaveRestoreCrc));
  DEBUG ((EFI_D_INFO, "BootVariableNvDataCrc on save = 0x%x.\n", VariableNvHob->BootVariableNvDataCrc));
#endif

  return EFI_SUCCESS;
}


/**
  This function saves the BDAT structure to Hob for later use.

  @param[in]  PeiServices  -  PEI Services Table.
  @param[in]  MrcData      -  Pointer to MRC Output Data that contains MRC Parameters
  @param[in]  HostBDAT     -  Pointer to the BDAT data structure.

  @retval  EFI_SUCCESS     - Hob is successfully built.
  @retval  Others          - Errors occur while creating new Hob
**/
#if BDAT_ENABLED
EFI_STATUS
MrcBdatSaveHob (
  IN        EFI_PEI_SERVICES      **PeiServices,
  IN        MMRC_DATA             *MrcData,
  IN        BDAT_STRUCTURE        *HostBDAT
  )
{
  BDAT_STRUCTURE        *BdatHob;
  EFI_PEI_HOB_POINTERS  Hob;
  UINTN                 BufferSize;

  BufferSize = sizeof (BDAT_STRUCTURE);

  Hob.Raw = BuildGuidHob (
              &gEfiMrcBDATGuid,
              BufferSize
            );
  ASSERT (Hob.Raw);

  BdatHob = (BDAT_STRUCTURE *) Hob.Raw;

  CopyMem (
    BdatHob,
    HostBDAT,
    sizeof (BDAT_STRUCTURE)
  );

  ///
  /// Saving the pointer to the HOB to dunit channel 0 (only) sticky scratchpad 1.
  /// This scratchpad is also used for MRC status codes but MRC is done
  /// with status codes by the time we get here. Use of this scratchpad
  /// makes it possible to have BDAT on both AVN and VLV.
  ///
  MemRegWrite (DUNIT_BOXTYPE, 0, 0, SSKPD1_DUNIT_BOXTYPE_REG, (UINT32) BdatHob, 0xF);

  DEBUG ((EFI_D_INFO, "Wrote BDAT pointer, %p, into scratchpad.\n", BdatHob));

  return EFI_SUCCESS;
}
#endif



#if NORTHPEAK_SUPPORT
/**
  This function configures the NorthPeak

  @param[in] MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
NorthPeakConfiguration (
  IN        MMRC_DATA   *MrcData,
)
{

}
#endif // #if NORTHPEAK_SUPPORT


