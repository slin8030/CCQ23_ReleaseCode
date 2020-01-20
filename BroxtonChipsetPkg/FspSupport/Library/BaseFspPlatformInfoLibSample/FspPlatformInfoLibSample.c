/**@file
 Sample to provide FSP platform information related function.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/


#include <PiPei.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/FspWrapperApiLib.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/FspCommonLib.h>
#include <Library/FspPolicyInitLib.h>
#include <Guid/PlatformInfo.h>
#include <Guid/AcpiVariable.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Library/FspWrapperPlatformLib.h>
#include <Ppi/DramPolicyPpi.h>
#include <Library/MemoryAllocationLib.h>
#include "MmrcData.h"

#include <FspEas.h>
#include <FspmUpd.h>
#include <FspsUpd.h>

#define  CAR_BASE_ADDR  0xFEF00000

STATIC
EFI_STATUS
UpdateSilicionInitUpd (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN FSPS_UPD          *FspsUpd
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
UpdateFspmUpd (
  IN FSPM_UPD                *FspmUpd
  )
{
  EFI_PEI_HOB_POINTERS           Hob;
  EFI_PLATFORM_INFO_HOB          *PlatformInfo = NULL;
  DRAM_POLICY_PPI                *DramPolicy;
  EFI_STATUS                     Status;
  MRC_PARAMS_SAVE_RESTORE        *MrcNvData;
  BOOT_VARIABLE_NV_DATA          *BootVariableNvData;
  MRC_PARAMS_SAVE_RESTORE        *MrcParamsHob;
  BOOT_VARIABLE_NV_DATA          *BootVariableNvDataHob;

  Status = PeiServicesLocatePpi (
                             &gDramPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **)&DramPolicy
                             );

  if (!EFI_ERROR (Status)) {
    FspmUpd->FspmConfig.Package                           = DramPolicy->Package;
    FspmUpd->FspmConfig.Profile                           = DramPolicy->Profile;
    FspmUpd->FspmConfig.MemoryDown                        = DramPolicy->MemoryDown;
    FspmUpd->FspmConfig.DDR3LPageSize                     = DramPolicy->DDR3LPageSize;
    FspmUpd->FspmConfig.DDR3LASR                          = DramPolicy->DDR3LASR;
    FspmUpd->FspmConfig.MemorySizeLimit                   = DramPolicy->SystemMemorySizeLimit;
    FspmUpd->FspmConfig.DIMM0SPDAddress                   = DramPolicy->SpdAddress[0];
    FspmUpd->FspmConfig.DIMM1SPDAddress                   = DramPolicy->SpdAddress[1];
    FspmUpd->FspmConfig.DDR3LPageSize                     = DramPolicy->DDR3LPageSize;
    FspmUpd->FspmConfig.DDR3LASR                          = DramPolicy->DDR3LASR;
    FspmUpd->FspmConfig.HighMemoryMaxValue                = DramPolicy->HighMemMaxVal;
    FspmUpd->FspmConfig.LowMemoryMaxValue                 = DramPolicy->LowMemMaxVal;
    FspmUpd->FspmConfig.DisableFastBoot                   = DramPolicy->DisableFastBoot;
    FspmUpd->FspmConfig.RmtMode                           = DramPolicy->RmtMode;
    FspmUpd->FspmConfig.RmtCheckRun                       = DramPolicy->RmtCheckRun;
    FspmUpd->FspmConfig.RmtMarginCheckScaleHighThreshold  = DramPolicy->RmtMarginCheckScaleHighThreshold;
    FspmUpd->FspmConfig.MsgLevelMask                      = DramPolicy->MsgLevelMask;

    FspmUpd->FspmConfig.ChannelHashMask                   = DramPolicy->ChannelHashMask;
    FspmUpd->FspmConfig.SliceHashMask                     = DramPolicy->SliceHashMask;
    FspmUpd->FspmConfig.ChannelsSlicesEnable              = DramPolicy->ChannelsSlicesEnabled;
    FspmUpd->FspmConfig.ScramblerSupport                  = DramPolicy->ScramblerSupport;
    FspmUpd->FspmConfig.InterleavedMode                   = DramPolicy->InterleavedMode;
    FspmUpd->FspmConfig.MinRefRate2xEnable                = DramPolicy->MinRefRate2xEnabled;
    FspmUpd->FspmConfig.DualRankSupportEnable             = DramPolicy->DualRankSupportEnabled;


    CopyMem (&(FspmUpd->FspmConfig.Ch0_RankEnable), &DramPolicy->ChDrp, sizeof(DramPolicy->ChDrp));
    CopyMem (&(FspmUpd->FspmConfig.Ch0_Bit_swizzling), &DramPolicy->ChSwizzle, sizeof (DramPolicy->ChSwizzle));

    if (((VOID *)(UINT32)DramPolicy->MrcTrainingDataPtr != 0) &&
       ((VOID *)(UINT32)DramPolicy->MrcBootDataPtr     != 0)) {
      DEBUG ((DEBUG_INFO, "JMSDBG UpdateFspmUpd - NvsBufferPtr\n"));
      MrcNvData          = (MRC_PARAMS_SAVE_RESTORE *) AllocateZeroPool (sizeof (MRC_PARAMS_SAVE_RESTORE));
      BootVariableNvData = (BOOT_VARIABLE_NV_DATA *) AllocateZeroPool (sizeof (BOOT_VARIABLE_NV_DATA));

      MrcParamsHob          = (MRC_PARAMS_SAVE_RESTORE*)((UINT32)DramPolicy->MrcTrainingDataPtr);
      BootVariableNvDataHob = (BOOT_VARIABLE_NV_DATA*)((UINT32)DramPolicy->MrcBootDataPtr);

      CopyMem(MrcNvData, MrcParamsHob, sizeof (MRC_PARAMS_SAVE_RESTORE));
      CopyMem(BootVariableNvData, BootVariableNvDataHob, sizeof (BOOT_VARIABLE_NV_DATA));
      FspmUpd->FspmArchUpd.NvsBufferPtr        = (VOID *)(UINT32)MrcNvData;
      FspmUpd->FspmConfig.VariableNvsBufferPtr = (VOID *)(UINT32)BootVariableNvData;
    }
  }
  DEBUG ((DEBUG_INFO, "UpdateFspmUpd - gEfiPlatformInfoGuid\n"));
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  DEBUG ((DEBUG_INFO, "UpdateFspmUpd,BoardId = %d\n", PlatformInfo->BoardId));
  if (PlatformInfo->BoardId == 2) { //rvp2
    FspmUpd->FspmConfig.Package         = 0;
    FspmUpd->FspmConfig.Profile         = 5;//LPDDR3_1600_12_15_15;
    FspmUpd->FspmConfig.MemoryDown      = 1;
    FspmUpd->FspmConfig.DDR3LPageSize   = 0;
    FspmUpd->FspmConfig.DDR3LASR        = 0;
    FspmUpd->FspmConfig.MemorySizeLimit = 0;
    FspmUpd->FspmConfig.DIMM0SPDAddress = 0;
    FspmUpd->FspmConfig.DIMM1SPDAddress = 0;
    FspmUpd->FspmConfig.DDR3LPageSize   = 0;
    FspmUpd->FspmConfig.DDR3LASR        = 0;

    FspmUpd->FspmConfig.Ch0_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch0_DeviceWidth  = 2;
    FspmUpd->FspmConfig.Ch0_DramDensity  = 0;
    FspmUpd->FspmConfig.Ch0_Option       = 3;

    FspmUpd->FspmConfig.Ch1_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch1_DeviceWidth  = 2;
    FspmUpd->FspmConfig.Ch1_DramDensity  = 0;
    FspmUpd->FspmConfig.Ch1_Option       = 3;

    FspmUpd->FspmConfig.Ch2_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch2_DeviceWidth  = 2;
    FspmUpd->FspmConfig.Ch2_DramDensity  = 0;
    FspmUpd->FspmConfig.Ch2_Option       = 3;

    FspmUpd->FspmConfig.Ch3_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch3_DeviceWidth  = 2;
    FspmUpd->FspmConfig.Ch3_DramDensity  = 0;
    FspmUpd->FspmConfig.Ch3_Option       = 3;

    FspmUpd->FspmConfig.Ch0_Bit_swizzling[0] = 0x17;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[1] = 0x15;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[2] = 0x10;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[3] = 0x11;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[4] = 0x12;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[5] = 0x13;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[6] = 0x16;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[7] = 0x14;

    FspmUpd->FspmConfig.Ch0_Bit_swizzling[8]  = 3;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[9]  = 5;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[10] = 2;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[11] = 4;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[12] = 7;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[13] = 0;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[14] = 6;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[15] = 1;

    FspmUpd->FspmConfig.Ch0_Bit_swizzling[16] = 0x18;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[17] = 0x1B;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[18] = 0x1A;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[19] = 0x1E;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[20] = 0x1C;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[21] = 0x19;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[22] = 0x1D;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[23] = 0x1F;

    FspmUpd->FspmConfig.Ch0_Bit_swizzling[24]  = 0xD;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[25]  = 0x8;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[26]  = 0xC;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[27]  = 0xF;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[28]  = 0xA;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[29]  = 0xE;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[30]  = 0xB;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[31]  = 0x9;

    FspmUpd->FspmConfig.Ch1_Bit_swizzling[0] = 0xA;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[1] = 0xE;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[2] = 0xC;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[3] = 0xD;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[4] = 0x9;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[5] = 0xB;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[6] = 0xF;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[7] = 0x8;

    FspmUpd->FspmConfig.Ch1_Bit_swizzling[8]  = 1;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[9]  = 6;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[10] = 2;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[11] = 5;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[12] = 0;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[13] = 7;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[14] = 3;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[15] = 4;

    FspmUpd->FspmConfig.Ch1_Bit_swizzling[16] = 0x15;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[17] = 0x12;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[18] = 0x13;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[19] = 0x16;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[20] = 0x14;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[21] = 0x11;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[22] = 0x10;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[23] = 0x17;

    FspmUpd->FspmConfig.Ch1_Bit_swizzling[24]  = 0x1D;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[25]  = 0x1C;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[26]  = 0x18;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[27]  = 0x1B;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[28]  = 0x1F;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[29]  = 0x1E;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[30]  = 0x1A;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[31]  = 0x19;

    FspmUpd->FspmConfig.Ch2_Bit_swizzling[0] = 0xD;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[1] = 0xF;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[2] = 0xA;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[3] = 0xE;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[4] = 0x9;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[5] = 0xC;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[6] = 0x8;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[7] = 0xB;

    FspmUpd->FspmConfig.Ch2_Bit_swizzling[8]  = 0x16;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[9]  = 0x13;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[10] = 0x10;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[11] = 0x17;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[12] = 0x15;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[13] = 0x11;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[14] = 0x14;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[15] = 0x12;

    FspmUpd->FspmConfig.Ch2_Bit_swizzling[16] = 0x1A;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[17] = 0x18;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[18] = 0x1C;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[19] = 0x1D;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[20] = 0x1F;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[21] = 0x1E;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[22] = 0x1B;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[23] = 0x19;

    FspmUpd->FspmConfig.Ch2_Bit_swizzling[24]  = 0x4;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[25]  = 0x5;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[26]  = 0x1;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[27]  = 0x0;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[28]  = 0x3;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[29]  = 0x6;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[30]  = 0x7;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[31]  = 0x2;

    FspmUpd->FspmConfig.Ch3_Bit_swizzling[0] = 0x0;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[1] = 0x3;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[2] = 0x6;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[3] = 0x7;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[4] = 0x4;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[5] = 0x5;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[6] = 0x1;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[7] = 0x2;

    FspmUpd->FspmConfig.Ch3_Bit_swizzling[8]  = 0x14;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[9]  = 0x11;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[10] = 0x15;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[11] = 0x10;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[12] = 0x17;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[13] = 0x12;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[14] = 0x16;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[15] = 0x13;

    FspmUpd->FspmConfig.Ch3_Bit_swizzling[16] = 0xF;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[17] = 0x9;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[18] = 0xB;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[19] = 0xE;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[20] = 0xC;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[21] = 0x8;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[22] = 0xD;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[23] = 0xA;

    FspmUpd->FspmConfig.Ch3_Bit_swizzling[24]  = 0x1A;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[25]  = 0x1D;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[26]  = 0x18;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[27]  = 0x1F;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[28]  = 0x19;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[29]  = 0x1E;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[30]  = 0x1B;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[31]  = 0x1C;

    FspmUpd->FspmConfig.ChannelHashMask       = 0;
    FspmUpd->FspmConfig.SliceHashMask         = 0;
    FspmUpd->FspmConfig.ChannelsSlicesEnable  = 0;
    FspmUpd->FspmConfig.ScramblerSupport      = 1;
    FspmUpd->FspmConfig.InterleavedMode       = 0;
    FspmUpd->FspmConfig.MinRefRate2xEnable    = 0;
    FspmUpd->FspmConfig.DualRankSupportEnable = 1;

    //FspmUpd->FspmConfig.BoardId                = 2;
  } else if (PlatformInfo->BoardId == 7) { //Leaf Hill

    FspmUpd->FspmConfig.Package         = 1;
    FspmUpd->FspmConfig.Profile         = 11;
    FspmUpd->FspmConfig.MemoryDown      = 1;
    FspmUpd->FspmConfig.DDR3LPageSize   = 0;
    FspmUpd->FspmConfig.DDR3LASR        = 0;
    FspmUpd->FspmConfig.MemorySizeLimit = 0x1800;
    FspmUpd->FspmConfig.DIMM0SPDAddress = 0;
    FspmUpd->FspmConfig.DIMM1SPDAddress = 0;
    FspmUpd->FspmConfig.DDR3LPageSize   = 0;
    FspmUpd->FspmConfig.DDR3LASR        = 0;

    FspmUpd->FspmConfig.Ch0_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch0_DeviceWidth  = 1;
    FspmUpd->FspmConfig.Ch0_DramDensity  = 2;
    FspmUpd->FspmConfig.Ch0_Option       = 3;

    FspmUpd->FspmConfig.Ch1_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch1_DeviceWidth  = 1;
    FspmUpd->FspmConfig.Ch1_DramDensity  = 2;
    FspmUpd->FspmConfig.Ch1_Option       = 3;

    FspmUpd->FspmConfig.Ch2_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch2_DeviceWidth  = 1;
    FspmUpd->FspmConfig.Ch2_DramDensity  = 2;
    FspmUpd->FspmConfig.Ch2_Option       = 3;

    FspmUpd->FspmConfig.Ch3_RankEnable   = 3;
    FspmUpd->FspmConfig.Ch3_DeviceWidth  = 1;
    FspmUpd->FspmConfig.Ch3_DramDensity  = 2;
    FspmUpd->FspmConfig.Ch3_Option       = 3;

    FspmUpd->FspmConfig.Ch0_Bit_swizzling[0]   = 0x09;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[1]   = 0x0e;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[2]   = 0x0c;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[3]   = 0x0d;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[4]   = 0x0a;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[5]   = 0x0b;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[6]   = 0x08;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[7]   = 0x0f;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[8]   = 0x05;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[9]   = 0x06;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[10]  = 0x01;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[11]  = 0x00;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[12]  = 0x02;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[13]  = 0x07;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[14]  = 0x04;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[15]  = 0x03;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[16]  = 0x1a;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[17]  = 0x1f;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[18]  = 0x1c;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[19]  = 0x1b;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[20]  = 0x1d;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[21]  = 0x19;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[22]  = 0x18;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[23]  = 0x1e;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[24]  = 0x14;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[25]  = 0x16;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[26]  = 0x17;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[27]  = 0x11;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[28]  = 0x12;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[29]  = 0x13;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[30]  = 0x10;
    FspmUpd->FspmConfig.Ch0_Bit_swizzling[31]  = 0x15;

    FspmUpd->FspmConfig.Ch1_Bit_swizzling[0]   = 0x06;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[1]   = 0x07;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[2]   = 0x05;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[3]   = 0x04;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[4]   = 0x03;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[5]   = 0x01;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[6]   = 0x00;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[7]   = 0x02;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[8]   = 0x0c;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[9]   = 0x0a;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[10]  = 0x0b;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[11]  = 0x0d;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[12]  = 0x0e;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[13]  = 0x08;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[14]  = 0x09;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[15]  = 0x0f;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[16]  = 0x14;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[17]  = 0x10;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[18]  = 0x16;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[19]  = 0x15;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[20]  = 0x12;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[21]  = 0x11;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[22]  = 0x13;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[23]  = 0x17;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[24]  = 0x1e;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[25]  = 0x1c;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[26]  = 0x1d;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[27]  = 0x19;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[28]  = 0x18;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[29]  = 0x1a;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[30]  = 0x1b;
    FspmUpd->FspmConfig.Ch1_Bit_swizzling[31]  = 0x1f;

    FspmUpd->FspmConfig.Ch2_Bit_swizzling[0]   = 0x0f;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[1]   = 0x09;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[2]   = 0x08;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[3]   = 0x0b;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[4]   = 0x0c;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[5]   = 0x0d;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[6]   = 0x0e;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[7]   = 0x0a;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[8]   = 0x05;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[9]   = 0x02;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[10]  = 0x00;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[11]  = 0x03;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[12]  = 0x06;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[13]  = 0x07;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[14]  = 0x01;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[15]  = 0x04;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[16]  = 0x19;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[17]  = 0x1c;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[18]  = 0x1e;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[19]  = 0x1f;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[20]  = 0x1a;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[21]  = 0x1b;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[22]  = 0x18;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[23]  = 0x1d;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[24]  = 0x14;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[25]  = 0x17;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[26]  = 0x16;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[27]  = 0x15;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[28]  = 0x12;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[29]  = 0x13;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[30]  = 0x10;
    FspmUpd->FspmConfig.Ch2_Bit_swizzling[31]  = 0x11;

    FspmUpd->FspmConfig.Ch3_Bit_swizzling[0]   = 0x03;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[1]   = 0x04;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[2]   = 0x06;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[3]   = 0x05;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[4]   = 0x00;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[5]   = 0x01;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[6]   = 0x02;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[7]   = 0x07;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[8]   = 0x0b;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[9]   = 0x0a;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[10]  = 0x08;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[11]  = 0x09;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[12]  = 0x0e;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[13]  = 0x0c;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[14]  = 0x0f;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[15]  = 0x0d;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[16]  = 0x11;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[17]  = 0x17;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[18]  = 0x13;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[19]  = 0x10;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[20]  = 0x15;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[21]  = 0x16;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[22]  = 0x14;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[23]  = 0x12;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[24]  = 0x1c;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[25]  = 0x1d;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[26]  = 0x1a;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[27]  = 0x19;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[28]  = 0x1e;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[29]  = 0x1b;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[30]  = 0x18;
    FspmUpd->FspmConfig.Ch3_Bit_swizzling[31]  = 0x1f;

    FspmUpd->FspmConfig.ChannelHashMask       = 0;
    FspmUpd->FspmConfig.SliceHashMask         = 0;
    FspmUpd->FspmConfig.ChannelsSlicesEnable  = 0;
    FspmUpd->FspmConfig.ScramblerSupport      = 1;
    FspmUpd->FspmConfig.InterleavedMode       = 0;
    FspmUpd->FspmConfig.MinRefRate2xEnable    = 0;
    FspmUpd->FspmConfig.DualRankSupportEnable = 1;
  }

  return EFI_SUCCESS;
}

/**
  This function overrides the default configurations in the UPD data region.

  @note At this point, memory is NOT ready, PeiServices are available to use.

  @param[in,out] FspmUpdPtr   A pointer to the UPD data region data structure.

  @return  FspmUpdPtr   A pointer to the UPD data region data structure.
**/
VOID *
EFIAPI
UpdateFspUpdConfigs (
  IN OUT VOID        *FspmUpdPtr
  )
{
  FSP_INFO_HEADER             *pFspHeader = NULL;
  UINT32                      FspmUpdSize;
  FSPM_UPD                    *FspmUpd = NULL;
  EFI_BOOT_MODE               BootMode;

  DEBUG ((DEBUG_INFO, "UpdateFspUpdConfigs\n"));
  if (NULL == FspmUpdPtr)
    return NULL;

  if (PcdGet32 (PcdFlashFvSecondFspBase) == 0) {
    pFspHeader = FspFindFspHeader (PcdGet32 (PcdFlashFvFspBase));
  } else {
    pFspHeader = FspFindFspHeader (PcdGet32 (PcdFlashFvSecondFspBase));
  }
  DEBUG ((DEBUG_INFO, "pFspHeader = %x\n", pFspHeader));
  if (NULL == pFspHeader) {
    return NULL;
  }

  DEBUG ((DEBUG_INFO, "pFspHeader->CfgRegionOffset = %08x\n", pFspHeader->CfgRegionOffset));
  FspmUpdSize = pFspHeader->CfgRegionSize;
  CopyMem (FspmUpdPtr, (void *)(pFspHeader->ImageBase + pFspHeader->CfgRegionOffset), FspmUpdSize);
  FspmUpd = (FSPM_UPD *)FspmUpdPtr;
  DEBUG ((DEBUG_INFO, "GetFspFspmUpdDataPointer - 0x%x\n", FspmUpd));

//[-start-161123-IB07250310-modify]//
  if (FeaturePcdGet (PcdH2ODdtSupported)) {
    // Use External Device to outpur EFI_DEBUG message
    FspmUpd->FspmConfig.SerialDebugPortDevice = 3;    
  } else {
    FspmUpd->FspmConfig.SerialDebugPortDevice = 2;
  }
//[-end-161123-IB07250310-modify]//
  FspmUpd->FspmConfig.SerialDebugPortType       = 2;
  FspmUpd->FspmConfig.SerialDebugPortStrideSize = 2;
  FspmUpd->FspmConfig.SerialDebugPortAddress    = 0;
  FspmUpd->FspmConfig.PreMemGpioTableEntryNum   = 0;
  FspmUpd->FspmConfig.PreMemGpioTablePtr        = 0;

  FspmUpd->FspmArchUpd.BootLoaderTolumSize      = 0;
  DEBUG((DEBUG_INFO, "BootLoaderTolumSize - 0x%x\n", FspmUpd->FspmArchUpd.BootLoaderTolumSize));

  PeiServicesGetBootMode (&BootMode);
  FspmUpd->FspmArchUpd.BootMode = (UINT8) BootMode;
  DEBUG((DEBUG_INFO, "BootMode - 0x%x\n", FspmUpd->FspmArchUpd.BootMode));

  FspmUpd->FspmArchUpd.StackBase = (VOID *)(UINTN) (PcdGet32(PcdFspStackBase));
  FspmUpd->FspmArchUpd.StackSize = (UINTN) (PcdGet32(PcdFspStackSize));

  DEBUG ((DEBUG_INFO, "UpdateFspmUpd \n"));
  UpdateFspmUpd((FSPM_UPD *)FspmUpd);

  return (VOID *)FspmUpd;
}

/**
  Get TempRamExit parameter.

  @note At this point, memory is ready, PeiServices are available to use.

  @return TempRamExit parameter.
**/
VOID *
EFIAPI
UpdateTempRamExitParam (
  VOID
  )
{
  return NULL;
}

/**
  Get S3 PEI memory information.

  @note At this point, memory is ready, and PeiServices are available to use.
  Platform can get some data from SMRAM directly.

  @param[out] S3PeiMemSize  PEI memory size to be installed in S3 phase.
  @param[out] S3PeiMemBase  PEI memory base to be installed in S3 phase.

  @return If S3 PEI memory information is got successfully.
**/
EFI_STATUS
EFIAPI
GetS3MemoryInfo (
  OUT UINT64               *S3PeiMemSize,
  OUT EFI_PHYSICAL_ADDRESS *S3PeiMemBase
  )
{
  ACPI_VARIABLE_SET               *AcpiVariableSet;
  EFI_PEI_HOB_POINTERS            GuidHob;
  EFI_PHYSICAL_ADDRESS            *AcpiVariableAddress;
  UINTN                           AcpiVarHobSize;

  GuidHob.Raw = GetHobList ();
  AcpiVariableAddress = NULL;
  AcpiVarHobSize = 0;

  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiAcpiVariableCompatiblityGuid, GuidHob.Raw)) != NULL) {
      AcpiVariableAddress = GET_GUID_HOB_DATA (GuidHob.Guid);
      AcpiVarHobSize = GET_GUID_HOB_DATA_SIZE (GuidHob.Guid);
    }
  }

  DEBUG ((DEBUG_INFO, "InstallS3Memory() read AcpiVariableAddress\n"));
  if (AcpiVariableAddress != NULL) {
    DEBUG ((DEBUG_INFO, "0x%08x\n",(UINT32)((UINTN)AcpiVariableAddress)));
    DEBUG ((DEBUG_INFO, "* 0x%08x\n",(UINT32)((UINTN)*AcpiVariableAddress)));
    AcpiVariableSet = (ACPI_VARIABLE_SET *)((UINTN)*AcpiVariableAddress);
    *S3PeiMemSize = (UINTN) (AcpiVariableSet->AcpiReservedMemorySize);
    *S3PeiMemBase = (UINTN) (AcpiVariableSet->AcpiReservedMemoryBase);
  } else {
    AcpiVariableSet = NULL;
    DEBUG ((DEBUG_INFO, " is NULL\n"));
  }

  return EFI_SUCCESS;
}

/**
  This function overrides the default configurations in the FSP-S UPD data region.

  @param[in,out] FspsUpdPtr   A pointer to the UPD data region data strcture.

**/
VOID
EFIAPI
UpdateFspsUpdData (
  IN OUT VOID        *FspsUpdPtr
  )
{
  FspPolicyInit ((FSPS_UPD *)FspsUpdPtr);
}

/**
  This function overrides the default configurations in the FSP-M UPD data region.

  @param[in,out] FspmUpdPtr   A pointer to the UPD data region data strcture.

**/
VOID
EFIAPI
UpdateFspmUpdData (
  IN OUT VOID        *FspmUpdPtr
  )
{
	UpdateFspUpdConfigs ((FSPM_UPD *)FspmUpdPtr);
	FspPolicyInitPreMem ((FSPM_UPD *)FspmUpdPtr);
}