/**@file

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

//[-start-151229-IB03090424-modify]//
#include <Uefi.h>
#include <SmipGenerated.h>
#include <Ppi/DramPolicyPpi.h>
//[-start-160803-IB07220122-add]//
#include <Ppi/ReadOnlyVariable2.h>
//[-end-160803-IB07220122-add]//
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/BaseMemoryLib.h>
//[-start-160803-IB07220122-add]//
#include <Library/PeiVariableCacheLib.h>
//[-end-160803-IB07220122-add]//
#include <Guid/PlatformInfo.h>
#include <Guid/PlatformConfigDataGuid.h>
//[-start-160803-IB07220122-add]//
#include <Guid/PreMemoryVariableLocationHobGuid.h>
//[-end-160803-IB07220122-add]//
//#include <Guid/SetupVariable.h>
#include <ChipsetSetupConfig.h>
#include <Library/PreSiliconLib.h>
#include <Library/SteppingLib.h>

//[-start-160803-IB07220122-add]//
extern EFI_GUID gEfiMemoryConfigVariableGuid;
//[-end-160803-IB07220122-add]//
//[-start-160604-IB07400740-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160604-IB07400740-add]//

//#include "Smip.h"

#define COUNT_LIMIT 0x13   //reasonable limit for smipblock loop
#define GPT_MARKER 0x54504724

//[-start-160803-IB07220122-add]//
#define NO_MRC_TRG_DATA_MSG "__NO_MRC_TRNG_DATA_AVAILABLE__"
//[-end-160803-IB07220122-add]//

#pragma pack(push, 1)
typedef struct {
  UINT64  Address;
  UINT32  Size:24;
  UINT32  Rsvd:8;
  UINT16  Version;
  UINT8   Type:7;
  UINT8   C_V:1;
  UINT8   Checksum;
} FitEntry;

typedef struct {
  UINT16  BlockCount;
  UINT16  TotalSize;
} SmipHeader;

typedef struct {
  UINT16  BlockType;   //0=CSE, 1=PMC, 2=IAFW
  UINT16  BlockOffset;
  UINT16  BlockLength;
  UINT16  Reserved;
} SmipBlockHeader;
#pragma pack(pop)

UINT8 Ch0Swizzle_IVI[] = {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,24,25,26,27,28,29,30,31,16,17,18,19,20,21,22,23};
UINT8 Ch1Swizzle_IVI[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
UINT8 Ch2Swizzle_IVI[] = {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,24,25,26,27,28,29,30,31,16,17,18,19,20,21,22,23};
UINT8 Ch3Swizzle_IVI[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

/**
  DramCreatePolicyDefaults creates the default setting of Dram Policy.

  @param[out] DramPolicyPpi       The pointer to get Dram Policy PPI instance

  @retval EFI_SUCCESS             The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create buffer
**/
//[-start-160727-IB07250259-modify]//
//[-start-160803-IB07220122-modify]//
EFI_STATUS
EFIAPI
DramCreatePolicyDefaults (
  IN  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariablePpi,
  OUT DRAM_POLICY_PPI                  **DramPolicyPpi,
  IN  IAFWDramConfig                   *DramConfigData,
//[-start-161001-IB07400791-modify]//
  IN  EFI_PHYSICAL_ADDRESS             *MrcTrainingDataAddr,
  IN  EFI_PHYSICAL_ADDRESS             *MrcBootDataAddr,
//[-end-161001-IB07400791-modify]//
  IN  UINT8                            BoardId
  )
//[-end-160803-IB07220122-modify]//
//[-end-160727-IB07250259-modify]//
{
  DRAM_POLICY_PPI                     *DramPolicy;
//[-start-160803-IB07220122-modify]//
  CHIPSET_CONFIGURATION               SystemConfiguration;
  UINTN                               VariableSize;
//[-end-160803-IB07220122-modify]//
  EFI_STATUS                          Status;
  DRP_DRAM_POLICY                     *DrpPtr;
  UINT8                               (*ChSwizlePtr)[DRAM_POLICY_NUMBER_CHANNELS][DRAM_POLICY_NUMBER_BITS];
  PlatfromDramConf                    *DramConfig;
  BOOLEAN                             ReadSetupVars;
//[-start-160517-IB03090427-add]//
//[-start-160603-IB07400740-modify]//
//#if BXTI_PF_ENABLE
  EFI_PLATFORM_INFO_HOB               *PlatformInfoHob = NULL;
  EFI_PEI_HOB_POINTERS                Hob;
//#endif
//[-end-160603-IB07400740-modify]//
//[-end-160517-IB03090427-add]//

  DramPolicy = (DRAM_POLICY_PPI *) AllocateZeroPool (sizeof (DRAM_POLICY_PPI));
  if (DramPolicy == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  ReadSetupVars = FALSE;
  DrpPtr        = NULL;
  ChSwizlePtr   = NULL;
  DramConfig    = NULL;
//[-start-160803-IB07220122-modify]//
  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//

//   Status = GetSystemConfigData(&ConfigVar, &ConfigVarSize);
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          SETUP_VARIABLE_NAME,
                          &gSystemConfigurationGuid,
                          NULL,
                          &VariableSize,
                          &SystemConfiguration
                          );
//[-start-161001-IB07400791-add]//
  DEBUG ((EFI_D_ERROR, "Status = %r\n", Status));
//[-end-161001-IB07400791-add]//
//[-end-160803-IB07220122-modify]//

//[-start-161001-IB07400791-modify]//
//#if !(ONLY_USE_SMIP_DRAM_POLICY == 1)
#ifndef BIOS_SETUP_DRAM_POLICY
  Status = EFI_UNSUPPORTED;
#endif
//[-end-161001-IB07400791-modify]//


  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Using setup options data for DRAM policy\n"));
    ReadSetupVars = TRUE;
//[-start-160803-IB07220122-modify]//
    DramPolicy->ChannelHashMask         = SystemConfiguration.ChannelHashMask;
    DramPolicy->SliceHashMask           = SystemConfiguration.SliceHashMask;
    DramPolicy->ChannelsSlicesEnabled   = SystemConfiguration.ChannelsSlicesEnabled;
    DramPolicy->ScramblerSupport        = SystemConfiguration.ScramblerSupport;
    DramPolicy->InterleavedMode         = SystemConfiguration.InterleavedMode;
    DramPolicy->MinRefRate2xEnabled     = SystemConfiguration.MinRefRate2xEnabled;
    DramPolicy->DualRankSupportEnabled  = SystemConfiguration.DualRankSupportEnabled;
//[-end-160803-IB07220122-modify]//
  }

//[-start-151204-IB02950555-modify]//
  switch (BoardId) {
  case BOARD_ID_APL_RVP_1A:
  case BOARD_ID_OXH_CRB:
    DramConfig = &(DramConfigData->PlatformDram0);
    break;
  case BOARD_ID_APL_RVP_2A:
    DramConfig = &(DramConfigData->PlatformDram1);
    break;
  case BOARD_ID_MNH_RVP:
  case BOARD_ID_LFH_CRB:
    DramConfig = &(DramConfigData->PlatformDram4);
    //DramPolicy->SystemMemorySizeLimit   = 0x1800;
    break;

  case BOARD_ID_MRB_IVI:
  case BOARD_ID_BFH_IVI:
    DramConfig = &(DramConfigData->PlatformDram4);
    //DramPolicy->SystemMemorySizeLimit   = 0;
    break;

  case BOARD_ID_JNH_CRB:
    DramConfig = &(DramConfigData->PlatformDram3);
    break;

  case BOARD_ID_BXT_RVP:
    ////TODO: Check BOARD ID val
    DramConfig = &(DramConfigData->PlatformDram2);
    break;

  case BOARD_ID_BXT_RVVP:
    ////TODO: Check BOARD ID val
    DEBUG ((EFI_D_INFO, "RVVP Board detect\n"));
    DramConfig = &(DramConfigData->PlatformDram1);
    DramConfig->MemorySizeLimit = 0x400;
    DramConfig->LowMemMaxVal = 0x400;
    break;

  default:
    DramConfig = &(DramConfigData->PlatformDram0);
    break;
  }

#if (TABLET_PF_ENABLE == 1)
  DEBUG ((EFI_D_INFO, "Using bxtm platform smip data: PlatformDram"));
  if (GetBxtSeries() == Bxt1) {
    DramConfig = &(DramConfigData->PlatformDram5);
    DEBUG ((EFI_D_INFO , "5\n"));
  } else {
    DramConfig = &(DramConfigData->PlatformDram2);
    DEBUG ((EFI_D_INFO , "2\n"));
  }
#endif

  DEBUG ((EFI_D_INFO, "Using smip platform override: %d\n", DramConfigData->Platform_override));
  switch (DramConfigData->Platform_override) {
  case 0:
    DramConfig = &(DramConfigData->PlatformDram0);
    break;
  case 1:
    DramConfig = &(DramConfigData->PlatformDram1);
    break;
  case 2:
    DramConfig = &(DramConfigData->PlatformDram2);
    break;
  case 3:
    DramConfig = &(DramConfigData->PlatformDram3);
    break;
  case 4:
    DramConfig = &(DramConfigData->PlatformDram4);
    break;
  default:
    //
    // Do nothing if the override value does not exist. 0xFF is the
    // default Platform_override value when no override is selected
    //
    break;
  }

  DramPolicy->Package                            = DramConfig->Package;
  DramPolicy->Profile                            = DramConfig->Profile;
  DramPolicy->MemoryDown                         = DramConfig->MemoryDown;
  DramPolicy->DDR3LPageSize                      = DramConfig->DDR3LPageSize;
  DramPolicy->DDR3LASR                           = DramConfig->DDR3LASR;
  DramPolicy->SystemMemorySizeLimit              = DramConfig->MemorySizeLimit;
  DramPolicy->SpdAddress[0]                      = DramConfig->SpdAddress0;
  DramPolicy->SpdAddress[1]                      = DramConfig->SpdAddress1;
  DramPolicy->DDR3LPageSize                      = DramConfig->DDR3LPageSize;
  DramPolicy->DDR3LASR                           = DramConfig->DDR3LASR;
  DramPolicy->HighMemMaxVal                      = DramConfig->HighMemMaxVal;
  DramPolicy->LowMemMaxVal                       = DramConfig->LowMemMaxVal;
  DramPolicy->DisableFastBoot                    = DramConfig->DisableFastBoot;
  DramPolicy->RmtMode                            = DramConfig->RmtMode;
  DramPolicy->RmtCheckRun                        = DramConfig->RmtCheckRun;
  DramPolicy->RmtMarginCheckScaleHighThreshold   = DramConfig->RmtMarginCheckScaleHighThreshold;

  DramPolicy->MsgLevelMask                       = DramConfigData->Message_level_mask;
  DrpPtr                                         = (DRP_DRAM_POLICY*)(&(DramConfig->Ch0RankEnabled));
  ChSwizlePtr                                    = (UINT8(*)[DRAM_POLICY_NUMBER_CHANNELS][DRAM_POLICY_NUMBER_BITS])(&(DramConfig->Ch0_Bit00_swizzling));

  if (!ReadSetupVars) {
    DEBUG ((EFI_D_INFO, "Using smip data for DRAM policy\n"));
    DramPolicy->ChannelHashMask         = DramConfig->ChannelHashMask;
    DramPolicy->SliceHashMask           = DramConfig->SliceHashMask;
    DramPolicy->ChannelsSlicesEnabled   = DramConfig->ChannelsSlicesEnabled;
    DramPolicy->ScramblerSupport        = DramConfig->ScramblerSupport;
    DramPolicy->InterleavedMode         = DramConfig->InterleavedMode;
    DramPolicy->MinRefRate2xEnabled     = DramConfig->MinRefRate2xEnabled;
    DramPolicy->DualRankSupportEnabled  = DramConfig->DualRankSupportEnabled;
}

  if (DrpPtr != NULL) {
    CopyMem (DramPolicy->ChDrp, DrpPtr, sizeof(DramPolicy->ChDrp));
  }

  switch (BoardId) {
    case BOARD_ID_MRB_IVI:
    case BOARD_ID_BFH_IVI:
      if (Ch0Swizzle_IVI != NULL) CopyMem (&(DramPolicy->ChSwizzle[0]), Ch0Swizzle_IVI, 32*sizeof(UINT8));
      if (Ch1Swizzle_IVI != NULL) CopyMem (&(DramPolicy->ChSwizzle[1]), Ch1Swizzle_IVI, 32*sizeof(UINT8));
      if (Ch2Swizzle_IVI != NULL) CopyMem (&(DramPolicy->ChSwizzle[2]), Ch2Swizzle_IVI, 32*sizeof(UINT8));
      if (Ch3Swizzle_IVI != NULL) CopyMem (&(DramPolicy->ChSwizzle[3]), Ch3Swizzle_IVI, 32*sizeof(UINT8));
//[-start-161018-IB06740518-add]//
      DramPolicy->ScramblerSupport |= 0x2; //Periodic Retraining Disable (bit 1) = 1 (disable)
//[-end-161018-IB06740518-add]//
      break;
    case BOARD_ID_APL_RVP_1A:
    case BOARD_ID_OXH_CRB:
    case BOARD_ID_APL_RVP_2A:
    case BOARD_ID_MNH_RVP:
    case BOARD_ID_LFH_CRB:
    case BOARD_ID_JNH_CRB:
    case BOARD_ID_BXT_RVP:
    case BOARD_ID_BXT_RVVP:
//[-start-160803-IB07400768-modify]//
//#if BXTI_PF_ENABLE
//[-start-160803-IB07220122-modify]//
//[-start-161001-IB07400791-add]//
      DEBUG ((EFI_D_ERROR, "Status = %r, SystemConfiguration.Max2G = %x\n", Status, SystemConfiguration.Max2G));
//[-end-161001-IB07400791-add]//
      if ((!EFI_ERROR (Status)) && (SystemConfiguration.Max2G == 0)) {
//[-end-160803-IB07220122-modify]//
        DramPolicy->SystemMemorySizeLimit = 0x800;
      }
//#endif
//[-end-160803-IB07400768-modify]//
      if (ChSwizlePtr != NULL) CopyMem (DramPolicy->ChSwizzle, ChSwizlePtr, sizeof (DramPolicy->ChSwizzle));
//[-start-171013-IB07400918-add]//
      //
      // Dump DRAM ChSwizzle Info from SMIP
      //
      DEBUG_CODE (
        UINTN Channel;
        UINTN BitsNumber;

        for (Channel = 0; Channel < DRAM_POLICY_NUMBER_CHANNELS; Channel ++) {
          DEBUG ((EFI_D_INFO, "Dump DramPolicy->ChSwizzle Channel = %x\n", Channel));
          for (BitsNumber = 0; BitsNumber < DRAM_POLICY_NUMBER_BITS; BitsNumber ++) {
            DEBUG ((EFI_D_INFO, "%02x ", DramPolicy->ChSwizzle[Channel][BitsNumber]));
          }
          DEBUG ((EFI_D_INFO, "\n"));
        }
      );
//[-end-171013-IB07400918-add]//
      break;
    default:
      if (ChSwizlePtr != NULL) CopyMem (DramPolicy->ChSwizzle, ChSwizlePtr, sizeof (DramPolicy->ChSwizzle));
      break;
  }

//[-start-160803-IB07220122-modify]//
  DramPolicy->MrcTrainingDataPtr = (EFI_PHYSICAL_ADDRESS) *MrcTrainingDataAddr;
//[-start-160727-IB07250259-add]//
  DramPolicy->MrcBootDataPtr     = (EFI_PHYSICAL_ADDRESS) *MrcBootDataAddr;
//[-end-160727-IB07250259-add]//
//[-start-160822-IB07400772-add]//
  DEBUG ((EFI_D_INFO, "DramPolicy->MrcTrainingDataPtr = %x\n", DramPolicy->MrcTrainingDataPtr));
  DEBUG ((EFI_D_INFO, "DramPolicy->MrcBootDataPtr = %x\n", DramPolicy->MrcBootDataPtr));
//[-end-160822-IB07400772-add]//
//[-end-160803-IB07220122-modify]//
//[-start-160802-IB03090430-remove]//
//  //WA for MH board to 6GB. We just apply it if memory size has not been override in smip XML.
//  if (((BoardId == BOARD_ID_MNH_RVP) || (BoardId == BOARD_ID_LFH_CRB)) && 
//      (DramPolicy->SystemMemorySizeLimit == 0)) {
//    DramPolicy->SystemMemorySizeLimit = 0x1800; 
//    if ((DramPolicy->ChDrp[2].RankEnable == 0) && (DramPolicy->ChDrp[3].RankEnable == 0)) {  //half config
//      DramPolicy->SystemMemorySizeLimit /= 2;
//    }
//  }
//[-end-160802-IB03090430-remove]//
  
//[-start-160517-IB03090427-add]//
//[-start-160603-IB07400740-modify]//
//#if BXTI_PF_ENABLE
  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfoHob = GET_GUID_HOB_DATA(Hob.Raw);

//[-start-161018-IB06740518-modify]//
  //Fab C Leaf Hill with Micron/Samsung Memory
//[-start-161101-IB03090435-modify]//
  if ((((BoardId == BOARD_ID_LFH_CRB) && (PlatformInfoHob->BoardRev >= FAB_ID_D)) || ((BoardId == BOARD_ID_MNH_RVP) && (PlatformInfoHob->BoardRev == FAB_ID_D))) && (DramConfigData->Platform_override == 0xFF)) {
//[-end-161101-IB03090435-modify]//
//[-start-160802-IB03090430-modify]//
    DramPolicy->ChDrp[0].DramDensity = 2;
    DramPolicy->ChDrp[1].DramDensity = 2;
    DramPolicy->ChDrp[2].DramDensity = 2;
    DramPolicy->ChDrp[3].DramDensity = 2;
//[-end-160802-IB03090430-modify]//
    DEBUG ((EFI_D_INFO, "Detect Micron or Samsung memory\n"));
  }
//[-end-161018-IB06740518-modify]//
//[-start-161129-IB03090436-add]//
//[-start-161216-IB03090437-remove]//
//  //Mineral Hill with Micron Memory
//  if ((BoardId == BOARD_ID_MNH_RVP) && (PlatformInfoHob->BoardRev == FAB_ID_D)) {
//    DramPolicy->ScramblerSupport |= 0x2; //Periodic Retraining Disable (bit 1) = 1 (disable)
//  }
//[-end-161216-IB03090437-remove]//
//[-end-161129-IB03090436-add]//
//#endif
//[-end-160603-IB07400740-modify]//
//[-end-160517-IB03090427-add]//

#if RVVP_ENABLE == 1
  DEBUG((EFI_D_INFO, "RVVP Memory Configuration. ch2 and ch3 are disable here\n"));
  DramPolicy->ChDrp[2].RankEnable =0;
  DramPolicy->ChDrp[3].RankEnable =0;
  DramPolicy->LowMemMaxVal = 0x400;
  DramPolicy->SystemMemorySizeLimit = 0x400;
#endif

//[-start-161001-IB07400791-add]//
  DEBUG ((EFI_D_INFO, "DramPolicy->SystemMemorySizeLimit = %x\n", DramPolicy->SystemMemorySizeLimit));
//[-end-161001-IB07400791-add]//

  *DramPolicyPpi = DramPolicy;
  return EFI_SUCCESS;
}


/**
  DramInstallPolicyPpi installs Dram Policy Ppi.

  @param[in] DramPolicyPpi       The pointer to Dram Policy PPI instance

  @retval EFI_SUCCESS            The policy is installed.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
DramInstallPolicyPpi (
  IN  DRAM_POLICY_PPI *DramPolicyPpi
  )
{
  EFI_STATUS             Status;
  EFI_PEI_PPI_DESCRIPTOR *DramPolicyPpiDesc;

  DramPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (DramPolicyPpiDesc == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  DramPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  DramPolicyPpiDesc->Guid  = &gDramPolicyPpiGuid;
  DramPolicyPpiDesc->Ppi   = DramPolicyPpi;

  //
  // Install Silicon Policy PPI
  //
  Status = PeiServicesInstallPpi (DramPolicyPpiDesc);
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/**
  Gpio Smip Init Function.

  @param[in] PlatformConfigPointer         The pointer to IAFWPlatformConfig instance.
  @param[in] PlatformInfoHob               The pointer to EFI_PLATFORM_INFO_HOB instance.

  @retval EFI_SUCCESS         Gpio Smip init successfully.
  @retval EFI_NOT_FOUND       Not found Gpio Smip.
**/
EFI_STATUS
EFIAPI
GpioSmipInit (
  IN   IAFWPlatformConfig       *PlatformConfigPointer,
  IN   EFI_PLATFORM_INFO_HOB    *PlatformInfoHob
  )
{

  UINTN                index = 0;
  UINTN                GpiotableNum = 0;
  UINTN                GpioBaseProfileLength=0;
  UINTN                GpioDiffProfileLength=0;
  GpioTable*           SmipBaseGpioTablePointer = NULL;
  GpioTable*           SmipDiffGpioTablePointer = NULL;
  GPIO_CONFIG_SMIP*    PlatformInfoHobSmipGpioBaseTablePointer = NULL;
  GPIO_CONFIG_SMIP*    PlatformInfoHobSmipGpioDiffTablePointer = NULL;
  GpioConfig*          SmipBaseGpioConfigPointer = NULL;
  GpioConfig*          SmipDiffGpioConfigPointer = NULL;
  GpioProfile *        GpioBaseProfilePointer = NULL;
  GpioProfile *        GpioDiffProfilePointer = NULL;

  PlatformInfoHobSmipGpioBaseTablePointer = (GPIO_CONFIG_SMIP*)(&(PlatformInfoHob->SmipGpioBaseTable[0]));
  PlatformInfoHobSmipGpioDiffTablePointer = (GPIO_CONFIG_SMIP*)(&(PlatformInfoHob->SmipGpioDiffTable[0]));
  GpioBaseProfilePointer = (GpioProfile *)&(PlatformConfigPointer->profile);
  GpiotableNum = PlatformConfigPointer->numProfiles;
  GpioBaseProfileLength = GpioBaseProfilePointer->numGpioEntries;

  SmipBaseGpioTablePointer=(GpioTable*)((UINT8*)(void*)PlatformConfigPointer +  GpioBaseProfilePointer->offset);
  SmipBaseGpioConfigPointer= (GpioConfig*)(&(SmipBaseGpioTablePointer->gpioCfg));
  DEBUG((EFI_D_INFO, " GpioSmipInit() entry \n"));
  ///
  /// Check whether it has the valid GPIO marker: GPT_MARKER.
  ///
  if(SmipBaseGpioTablePointer->marker != GPT_MARKER ){
    return EFI_NOT_FOUND;
  }

  PlatformInfoHob->SmipGpioBaseTableLength = GpioBaseProfileLength;
  ///
  ///Go through the SMIP base GPIO table to copy the settings to platform info hob GPIO Base Table.
  ///
  for (index=0;index<GpioBaseProfileLength; index++){
    (PlatformInfoHobSmipGpioBaseTablePointer+index)->Half0.DW = (SmipBaseGpioConfigPointer+index)->half0;
    (PlatformInfoHobSmipGpioBaseTablePointer+index)->Half1.DW = (SmipBaseGpioConfigPointer+index)->half1;
  }
  ///
  ///Current mapping rule is that "BOM ID 0 : GPIO Diff Table 0; BOM ID 1 : GPIO Diff Table 1; BOM ID 2 : GPIO Diff Table 2; BOM ID 3 : GPIO Diff Table 3; BOM ID 4 : GPIO Diff Table 4.
  ///
  switch(PlatformInfoHob->BomIdPss){
    case 0:
      GpioDiffProfilePointer   = (GpioProfile *)((UINT8*)(void*)GpioBaseProfilePointer+8);
      SmipDiffGpioTablePointer = (GpioTable*)((UINT8*)(void*)PlatformConfigPointer + GpioDiffProfilePointer->offset);

      if(SmipDiffGpioTablePointer->marker != GPT_MARKER ){
      return EFI_NOT_FOUND;
      }

      SmipDiffGpioConfigPointer                = (GpioConfig*)&(SmipDiffGpioTablePointer->gpioCfg);
      GpioDiffProfileLength                    = GpioDiffProfilePointer->numGpioEntries;
      PlatformInfoHob->SmipGpioDiffTableLength = GpioDiffProfileLength;
      ///
      ///Go through the SMIP Diff GPIO table 0 to copy the settings to platform info hob GPIO Diff Table.
      ///
      for (index=0;index<GpioDiffProfileLength; index++){
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half0.DW = (SmipDiffGpioConfigPointer+index)->half0;
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half1.DW = (SmipDiffGpioConfigPointer+index)->half1;
    }
    break;

    case 1:
      GpioDiffProfilePointer    = (GpioProfile *)((UINT8*)(void*)GpioBaseProfilePointer+8*2);
      SmipDiffGpioTablePointer  = (GpioTable*)((UINT8*)(void*)PlatformConfigPointer + GpioDiffProfilePointer->offset);

      if(SmipDiffGpioTablePointer->marker != GPT_MARKER ){
      return EFI_NOT_FOUND;
      }

      SmipDiffGpioConfigPointer                = (GpioConfig*)&(SmipDiffGpioTablePointer->gpioCfg);
      GpioDiffProfileLength                    = GpioDiffProfilePointer->numGpioEntries;
      PlatformInfoHob->SmipGpioDiffTableLength = GpioDiffProfileLength;
      ///
      ///Go through the SMIP Diff GPIO table 1 to copy the settings to platform info hob GPIO Diff Table.
      ///
      for (index=0;index<GpioDiffProfileLength; index++){
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half0.DW = (SmipDiffGpioConfigPointer+index)->half0;
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half1.DW = (SmipDiffGpioConfigPointer+index)->half1;
    }
    break;

    case 2:
      GpioDiffProfilePointer       = (GpioProfile *)((UINT8*)(void*)GpioBaseProfilePointer+8*3);;
      SmipDiffGpioTablePointer     = (GpioTable*)((UINT8*)(void*)PlatformConfigPointer + GpioDiffProfilePointer->offset);

      if(SmipDiffGpioTablePointer->marker != GPT_MARKER ){
      return EFI_NOT_FOUND;
      }

      SmipDiffGpioConfigPointer                = (GpioConfig*)&(SmipDiffGpioTablePointer->gpioCfg);
      GpioDiffProfileLength                    = GpioDiffProfilePointer->numGpioEntries;
      PlatformInfoHob->SmipGpioDiffTableLength = GpioDiffProfileLength;
      ///
      ///Go through the SMIP Diff GPIO table 2 to copy the settings to platform info hob GPIO Diff Table.
      ///
      for (index=0;index<GpioDiffProfileLength; index++){
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half0.DW = (SmipDiffGpioConfigPointer+index)->half0;
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half1.DW = (SmipDiffGpioConfigPointer+index)->half1;
    }
    break;

    case 3:
      GpioDiffProfilePointer       = (GpioProfile *)((UINT8*)(void*)GpioBaseProfilePointer+8*4);;
      SmipDiffGpioTablePointer     = (GpioTable*)((UINT8*)(void*)PlatformConfigPointer + GpioDiffProfilePointer->offset);

      if(SmipDiffGpioTablePointer->marker != GPT_MARKER ){
      return EFI_NOT_FOUND;
      }

      SmipDiffGpioConfigPointer                = (GpioConfig*)&(SmipDiffGpioTablePointer->gpioCfg);
      GpioDiffProfileLength                    = GpioDiffProfilePointer->numGpioEntries;
      PlatformInfoHob->SmipGpioDiffTableLength = GpioDiffProfileLength;
      ///
      ///Go through the SMIP Diff GPIO table 3 to copy the settings to platform info hob GPIO Diff Table.
      ///
      for (index=0;index<GpioDiffProfileLength; index++){
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half0.DW = (SmipDiffGpioConfigPointer+index)->half0;
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half1.DW = (SmipDiffGpioConfigPointer+index)->half1;
    }
    break;

    case 4:
      GpioDiffProfilePointer       = (GpioProfile *)((UINT8*)(void*)GpioBaseProfilePointer+8*5);;
      SmipDiffGpioTablePointer     = (GpioTable*)((UINT8*)(void*)PlatformConfigPointer + GpioDiffProfilePointer->offset);

      if(SmipDiffGpioTablePointer->marker != GPT_MARKER ){
      return EFI_NOT_FOUND;
      }

      SmipDiffGpioConfigPointer                = (GpioConfig*)&(SmipDiffGpioTablePointer->gpioCfg);
      GpioDiffProfileLength                    = GpioDiffProfilePointer->numGpioEntries;
      PlatformInfoHob->SmipGpioDiffTableLength = GpioDiffProfileLength;
      ///
      ///Go through the SMIP Diff GPIO table 4 to copy the settings to platform info hob GPIO Diff Table.
      ///
      for (index=0;index < GpioDiffProfileLength; index++){
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half0.DW = (SmipDiffGpioConfigPointer+index)->half0;
      (PlatformInfoHobSmipGpioDiffTablePointer+index)->Half1.DW = (SmipDiffGpioConfigPointer+index)->half1;
    }
    break;

    default:
    DEBUG((EFI_D_INFO, " BOM ID is invalid \n"));
    break;
}
  DEBUG((EFI_D_INFO, " GpioSmipInit() end \n"));
  return EFI_SUCCESS;
}

//[-start-160803-IB07220122-add]//
VOID
PrintVariableData (
  IN UINT8   *Data8,
  IN UINTN   DataSize
  )
{
  UINTN      Index;

  for (Index = 0; Index < DataSize; Index++) {
    if (Index % 0x10 == 0) {
      DEBUG ((EFI_D_INFO, "\n%08X:", Index));
    }
    DEBUG ((EFI_D_INFO, " %02X", *Data8++));
  }
  DEBUG ((EFI_D_INFO, "\n"));
}
//[-end-160803-IB07220122-add]//

//[-start-160803-IB07220122-modify]//
/**
  Smip Init Function.

  @retval EFI_SUCCESS         Smip init successfully.
  @retval EFI_NOT_FOUND       Smip init fail.
**/
EFI_STATUS
SmipInit (
  IN VOID   *FitHeaderPtr,
  IN UINT8  BoardId
)
{
//[-start-160429-IB03090426-modify]//
  UINT16                            Index;
  EFI_STATUS                        Status;
  EFI_PEI_HOB_POINTERS              Hob;
  UINTN                             SystemConfigDataSize = 0;
  UINTN                             MrcTrainingDataSize  = 0;
  UINTN                             MrcBootDataSize      = 0;
  UINTN                             SvBiosKnobsSize      = 0;
  UINTN                             SetupDataSize        = 0;
//[-start-161001-IB07400791-modify]//
  EFI_PHYSICAL_ADDRESS              MrcTrainingDataAddr  = 0;
  EFI_PHYSICAL_ADDRESS              MrcBootDataAddr      = 0;
//[-end-161001-IB07400791-modify]//

  PRE_MEMORY_VARIABLE_LOCATION_HOB  PreMemoryVariableLocation;
  CHIPSET_CONFIGURATION              SystemConfiguration;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariablePpi;

  SmipHeader                 *SmipHeadPointer          = NULL;
  SmipBlockHeader            *SmipBlockPtr             = NULL;
  IafwSmipLayout             *SmipIafwPointer          = NULL;
  FitEntry                   *FitHeader                = NULL;
  FitEntry                   *FitEntries               = NULL;
  DRAM_POLICY_PPI            *DramPolicyPpi            = NULL;
  VOID                       *SystemConfigDataPtr      = NULL;

  CHAR8                      *NoMrcTrainingDataString  = NO_MRC_TRG_DATA_MSG;

  EFI_PLATFORM_INFO_HOB      *PlatformInfoHob          = NULL;
//[-start-160816-IB07400771-modify]//
//#if !(BXTI_PF_ENABLE)
  UINT8                      PlatformFeatureSelection;
  UINT8                      PlatformBOMSelection;
//#endif
//[-end-160816-IB07400771-modify]//
//[-end-160429-IB03090426-modify]//
//[-start-170516-IB08450375-remove]//
//#if (BXTI_PF_ENABLE)
//  UINT8                     *TxeMeasurementHashDataPtr  = NULL;
//  UINTN                      TxeMeasurementHashDataSize = 0;
//#endif
//[-end-170516-IB08450375-remove]//

  ZeroMem(&Hob, sizeof(Hob));
  FitHeader  = (FitEntry *)FitHeaderPtr;
  FitEntries = FitHeader;

  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **)&VariablePpi);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  DEBUG ((EFI_D_INFO, "Checking FitHeader at 0x%08x\n", FitHeader ));

  if (FitHeader->Address != SIGNATURE_64 ('_', 'F', 'I', 'T', '_', ' ', ' ', ' ')) {
    if (PLATFORM_ID != VALUE_REAL_PLATFORM) {
      DEBUG ((EFI_D_ERROR, "FIT fails on simics\n"));	

      SystemConfigDataPtr = (VOID *)(UINTN)(0xFFFE4000);
      SystemConfigDataSize = 0x400;

      Status = DramCreatePolicyDefaults (VariablePpi, &DramPolicyPpi, &(SmipIafwPointer->DramConfig), &MrcTrainingDataAddr, &MrcBootDataAddr, BoardId);
      ASSERT_EFI_ERROR (Status);

      Status = DramInstallPolicyPpi (DramPolicyPpi);
      ASSERT_EFI_ERROR (Status);
    }
    DEBUG ((EFI_D_ERROR, "FitHeader signature was invalid.\n"));
    return EFI_NOT_FOUND;
  }
  DEBUG ((EFI_D_INFO, "FitHeader signature verified.\n"));

//[-start-160429-IB03090426-add]//
//[-start-160506-IB07400723-modify]//
//#if (BXTI_PF_ENABLE)
  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfoHob = GET_GUID_HOB_DATA(Hob.Raw);
//#endif
//[-end-160506-IB07400723-modify]//
//[-end-160429-IB03090426-add]//
	
  //
  // Loop through FIT Entries until we find SMIP (start at 1 to skip header)
  //
  DEBUG ((EFI_D_INFO, "Searching for SMIP Entry in FIT...\n" ));
  for (Index = 1; Index < FitHeader->Size; Index++) {
    if (FitEntries[Index].Type == 0x10 && FitEntries[Index].Rsvd == 0x5) {
      DEBUG ((EFI_D_INFO, "Found SMIP Entry in FIT.\n" ));

      SmipHeadPointer = (SmipHeader *)(UINTN)(FitEntries[Index].Address);
      SmipBlockPtr = (SmipBlockHeader *)((UINT8 *)SmipHeadPointer + sizeof (SmipHeadPointer));

      DEBUG ((EFI_D_INFO, "SMIP table located at: 0x%08x\n", SmipHeadPointer));
      DEBUG ((EFI_D_INFO, "SMIP table size = 0x%08x bytes\n", SmipHeadPointer->TotalSize));
    } else if (FitEntries[Index].Type == 0x10 && FitEntries[Index].Rsvd == 0x6) {
      DEBUG ((EFI_D_INFO, "Found the entry for MRC Training Data in FIT. Checking if data is present...\n" ));
      PreMemoryVariableLocation.VariableDataPtr = (VOID *)(UINTN)(FitEntries[Index].Address);
      PreMemoryVariableLocation.VariableDataSize = FitEntries[Index].Size;

      //
      // Check if Setup data is available in the MRC training data file
      //
      if (AsciiStrnCmp ((CHAR8 *)PreMemoryVariableLocation.VariableDataPtr, NoMrcTrainingDataString, AsciiStrSize (NoMrcTrainingDataString)) != 0) {
        DEBUG ((EFI_D_INFO, "MRC training data is present in FIT. Data will be loaded from the file...\n"));
        DEBUG ((EFI_D_INFO, "Training data file contents at 0x%08x. Content data size = %d bytes.\n", PreMemoryVariableLocation.VariableDataPtr, PreMemoryVariableLocation.VariableDataSize));

        //
        // Store the temporary location of the training data variable store
        //
        BuildGuidDataHob (&gPreMemoryVariableLocationHobGuid, (VOID *) &PreMemoryVariableLocation, sizeof (PRE_MEMORY_VARIABLE_LOCATION_HOB));

        //
        // Create the variable cache HOB
        //   - It is populated with the pre-memory NVM file contents and consumed by the variable PEIM
        //     on variable read requests.
        //   - The address to the training data is stored in the MemoryConfig variable.
        //
        Status = CreateVariableCacheHob ();
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "The PEI variable data cache was not established but training data is available.\n"));
          ASSERT_EFI_ERROR (Status);
        }

        SetupDataSize = PcdGet32 (PcdSetupConfigSize);
        Status = VariablePpi->GetVariable (VariablePpi, L"Setup", &gSystemConfigurationGuid, NULL, &SetupDataSize, &SystemConfiguration);
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_WARN, "Could not read the Setup variable after establishing the cache.\n"));
        }

        MrcTrainingDataSize = sizeof (EFI_PHYSICAL_ADDRESS);
        Status = VariablePpi->GetVariable (VariablePpi, L"MemoryConfig", &gEfiMemoryConfigVariableGuid, NULL, &MrcTrainingDataSize, &MrcTrainingDataAddr);
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_WARN, "Could not read the variable to find the MemoryConfig data address.\n"));
        }

        MrcBootDataSize = sizeof (EFI_PHYSICAL_ADDRESS);
        Status = VariablePpi->GetVariable (VariablePpi, L"MemoryBootData", &gEfiMemoryConfigVariableGuid, NULL, &MrcBootDataSize, &MrcBootDataAddr);
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_WARN, "Could not read the variable to find the MemoryBootData address.\n"));
        }

        //
        // Print Setup variable data
        //
        DEBUG_CODE_BEGIN ();
        ///
        /// Print default Setup variable.
        ///
        if (!EFI_ERROR (Status)) {
//[-start-160819-IB07400772-modify]//
          DEBUG ((EFI_D_INFO, "\nDumping Setup data (Ptr:%x, Size:%x):", &SystemConfiguration, SetupDataSize));
//[-end-160819-IB07400772-modify]//
          PrintVariableData ((UINT8 *) &SystemConfiguration, SetupDataSize);
        }
        DEBUG_CODE_END ();
      } else {
        DEBUG ((EFI_D_INFO, "Training data not found. This is considered a first boot.\n\n"));
      }

//[-start-170516-IB08450375-remove]//
//#if (BXTI_PF_ENABLE)
//    } else if ( FitEntries[Index].Type == 0x10 && FitEntries[Index].Rsvd == 0x2) {
//      DEBUG ((EFI_D_INFO, "Found TXE Measurement hash Entry in FIT.\n" ));
//      TxeMeasurementHashDataPtr = (UINT8 *) (UINTN) (FitEntries[Index].Address);
//      TxeMeasurementHashDataSize = FitEntries[Index].Size;
//      CopyMem(&PlatformInfoHob->TxeMeasurementHash, TxeMeasurementHashDataPtr , TxeMeasurementHashDataSize);
//      DEBUG ((EFI_D_INFO, "TxeMeasurementHashDataPtr = 0x%08x, TxeMeasurementHashDataSize = %x\n", TxeMeasurementHashDataPtr, TxeMeasurementHashDataSize));
//#endif
//[-end-170516-IB08450375-remove]//
//[-end-160429-IB03090426-add]//
    }
  }
  if (SmipHeadPointer == NULL) {
    DEBUG ((EFI_D_ERROR, "No SMIP Entry found in FIT\n"));
    return EFI_NOT_FOUND;
  }

  //
  // Loop through SMIP Header and look for IAFW SMIP entry
  //
  DEBUG ((EFI_D_INFO, "Searching SMIP Header for IAFW entry...\n" ));
  for (Index = 1; Index <= SmipHeadPointer->BlockCount && Index < COUNT_LIMIT; Index++, SmipBlockPtr++) {
    if ( SmipBlockPtr->BlockType == 0x2) {
      DEBUG ((EFI_D_INFO, "Found IAFW SMIP Block at 0x%08x\n", SmipBlockPtr->BlockOffset ));
      SmipIafwPointer = (IafwSmipLayout*)((UINT8*)SmipHeadPointer + SmipBlockPtr->BlockOffset);
//[-start-160426-IB07400720-modify]//
      DEBUG ((EFI_D_INFO, "SmipIafwPointer = %x\n", SmipIafwPointer));
//[-end-160426-IB07400720-modify]//
      break;
    }
  }
  if (SmipIafwPointer == NULL) {
    DEBUG ((EFI_D_ERROR, "No IAFW SMIP Block found\n"));
    return EFI_NOT_FOUND;
  }
  //
  // Parse SMIP and publish data to PPI's and PCD's -- STILL UNDER DEVELOPMENT...
  //
  DEBUG ((EFI_D_INFO, "IafwSmipLayout->IafwSmipSignature = 0x%08x\n", SmipIafwPointer->IafwSmipSignature ));
//[-start-160822-IB07400772-add]//
  DEBUG ((EFI_D_INFO, "MrcTrainingDataAddr = %x\n", MrcTrainingDataAddr));
  DEBUG ((EFI_D_INFO, "MrcBootDataAddr = %x\n", MrcBootDataAddr));
//[-end-160822-IB07400772-add]//
  Status = DramCreatePolicyDefaults (VariablePpi, &DramPolicyPpi, &(SmipIafwPointer->DramConfig), &MrcTrainingDataAddr, &MrcBootDataAddr, BoardId);
  ASSERT_EFI_ERROR (Status);

  Status = DramInstallPolicyPpi (DramPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  //Fetch Setup option overrides that are stored in SMIP
  DEBUG ((EFI_D_INFO, "Retrieving BIOS knobs from SMIP\n"));
  SvBiosKnobsSize = sizeof(IafwSvBiosKnobs);
  PcdSetPtr (PcdSvBiosKnobs, &SvBiosKnobsSize, &(SmipIafwPointer->SvBiosKnobs));

//[-start-160406-IB07400715-remove]//
//#if !(BXTI_PF_ENABLE)  
//[-end-160406-IB07400715-remove]//
//[-start-160406-IB07400715-add]//
  //
  // If IOTG platforms, skip check BOM ID
  //
//[-start-161022-IB07400803-modify]//
  if (IsIOTGBoardIds()) {
//[-end-161022-IB07400803-modify]//
    PlatformInfoHob->OsSelPss = SmipIafwPointer->FeatureConfig.PlatfromFeatureConf4.OsSelection;
    DEBUG((EFI_D_INFO, " OS Selection ID is %x \n ", PlatformInfoHob->OsSelPss));
    return Status;
  }
//[-end-160406-IB07400715-add]//
  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfoHob = GET_GUID_HOB_DATA(Hob.Raw);

  //
  //Override the FwConfigId from SPID or SMIP BOM ID,  Priority: SPID > SMIP FW Confg ID > DIP FW Config ID
  //SPID: S/W programmable NVM (as a UEFI variable)
  //SMIP FW Confg ID: Defined in SMIP region
  //DIP FW Config ID: SW1 Used on Intel RVP
  //

  PlatformFeatureSelection = PlatformInfoHob->OsSelPss;
  PlatformBOMSelection = PlatformInfoHob->BomIdPss;

  SetupDataSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
  ZeroMem (&SystemConfiguration, SetupDataSize);

//   Status = GetSystemConfigDataPreMem(&SystemConfiguration, &VariableSize);
  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          SETUP_VARIABLE_NAME,
                          &gSystemConfigurationGuid,
                          NULL,
                          &SetupDataSize,
                          &SystemConfiguration
                          );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to load Setup data. This will occur on first boot.\n"));
  }

  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Get BOM ID selection type: 0x%X [0: BXT A0/A1; 1: BXT FFD EVT; 2: BXT HR16 0x80: Munaully Configure; 0xFF: Load from PSS]\n", SystemConfiguration.BomSelection));
    if (SystemConfiguration.BomSelection != BOM_MANUALLY) {
      if (SystemConfiguration.BomSelection != BOM_FROM_PSS) { // Use Forced BOM ID selection as Setup variable
        PlatformInfoHob->BomIdPss = SystemConfiguration.BomSelection;
        DEBUG ((EFI_D_INFO, "PlatformInfoHob->BomIdPss: 0x%X [0: BXT A0/A1; 1: BXT FFD EVT; 2: BXT HR16]\n", PlatformInfoHob->BomIdPss));
      }
    }
    // Check OsSelection is changed by user in setup menu
    if (SystemConfiguration.PreOsSelection != SystemConfiguration.OsSelection) {
        PlatformFeatureSelection = SystemConfiguration.OsSelection;
        DEBUG ((EFI_D_INFO, "SystemConfiguration.OsSelection: 0x%X [0: WOS; 1: AOS]\n", SystemConfiguration.OsSelection));
    }
  }

  //
  // Get the PlatformBOMValue and PlatformFeatureValue from IA FW SMIP MultiBOMConfig and FeatureConfig.
  //
//[-start-151204-IB02950555-modify]//  
  switch (PlatformBOMSelection){
    case 0:
    PlatformInfoHob->PlatformBOMValue.AudioSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.AudioSel;
    PlatformInfoHob->PlatformBOMValue.ModemSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.ModemSel;
    PlatformInfoHob->PlatformBOMValue.PanelSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.PanelSel;
    PlatformInfoHob->PlatformBOMValue.UserCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.UserCameraSel;
    PlatformInfoHob->PlatformBOMValue.WorldCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.WorldCameraSel;
    PlatformInfoHob->PlatformBOMValue.TouchSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.TouchSel;
    PlatformInfoHob->PlatformBOMValue.WifiSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM0.WifiSel;
    break;

    case 1:
    //Temp use Bom1 for FFD
    PlatformInfoHob->PlatformBOMValue.AudioSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.AudioSel;
    PlatformInfoHob->PlatformBOMValue.ModemSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.ModemSel;
    PlatformInfoHob->PlatformBOMValue.PanelSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.PanelSel;
    PlatformInfoHob->PlatformBOMValue.UserCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.UserCameraSel;
    PlatformInfoHob->PlatformBOMValue.WorldCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.WorldCameraSel;
    PlatformInfoHob->PlatformBOMValue.TouchSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.TouchSel;
    PlatformInfoHob->PlatformBOMValue.WifiSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM1.WifiSel;
    break;

    case 2:
    PlatformInfoHob->PlatformBOMValue.AudioSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.AudioSel;
    PlatformInfoHob->PlatformBOMValue.ModemSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.ModemSel;
    PlatformInfoHob->PlatformBOMValue.PanelSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.PanelSel;
    PlatformInfoHob->PlatformBOMValue.UserCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.UserCameraSel;
    PlatformInfoHob->PlatformBOMValue.WorldCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.WorldCameraSel;
    PlatformInfoHob->PlatformBOMValue.TouchSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.TouchSel;
    PlatformInfoHob->PlatformBOMValue.WifiSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM2.WifiSel;
    break;

    case 3:
    PlatformInfoHob->PlatformBOMValue.AudioSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.AudioSel;
    PlatformInfoHob->PlatformBOMValue.ModemSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.ModemSel;
    PlatformInfoHob->PlatformBOMValue.PanelSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.PanelSel;
    PlatformInfoHob->PlatformBOMValue.UserCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.UserCameraSel;
    PlatformInfoHob->PlatformBOMValue.WorldCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.WorldCameraSel;
    PlatformInfoHob->PlatformBOMValue.TouchSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.TouchSel;
    PlatformInfoHob->PlatformBOMValue.WifiSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM3.WifiSel;
    break;

    case 4:
    PlatformInfoHob->PlatformBOMValue.AudioSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.AudioSel;
    PlatformInfoHob->PlatformBOMValue.ModemSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.ModemSel;
    PlatformInfoHob->PlatformBOMValue.PanelSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.PanelSel;
    PlatformInfoHob->PlatformBOMValue.UserCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.UserCameraSel;
    PlatformInfoHob->PlatformBOMValue.WorldCameraSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.WorldCameraSel;
    PlatformInfoHob->PlatformBOMValue.TouchSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.TouchSel;
    PlatformInfoHob->PlatformBOMValue.WifiSel= SmipIafwPointer->MultiBOMConfig.PlatfromMultiBOM4.WifiSel;
    break;

    default:
    DEBUG((EFI_D_INFO, " BOM ID is invalid \n"));
    break;
    }
//[-end-151204-IB02950555-modify]//

  switch (PlatformFeatureSelection) {
    case 0:  //PSS_WOS
    PlatformInfoHob->PlatformFeatureValue.OsSelection= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf0.OsSelection;
    PlatformInfoHob->PlatformFeatureValue.Vibrator= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf0.Vibrator;
    PlatformInfoHob->PlatformFeatureValue.Ssic1Support= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf0.Ssic1Support;
    PlatformInfoHob->PlatformFeatureValue.ScUsbOtg= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf0.ScUsbOtg;
    break;

    case 1:  //PSS_AOS
    PlatformInfoHob->PlatformFeatureValue.OsSelection= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf1.OsSelection;
    PlatformInfoHob->PlatformFeatureValue.Vibrator= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf1.Vibrator;
    PlatformInfoHob->PlatformFeatureValue.Ssic1Support= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf1.Ssic1Support;
    PlatformInfoHob->PlatformFeatureValue.ScUsbOtg= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf1.ScUsbOtg;
    break;

    case 2:
    PlatformInfoHob->PlatformFeatureValue.OsSelection= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf2.OsSelection;
    PlatformInfoHob->PlatformFeatureValue.Vibrator= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf2.Vibrator;
    PlatformInfoHob->PlatformFeatureValue.Ssic1Support= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf2.Ssic1Support;
    PlatformInfoHob->PlatformFeatureValue.ScUsbOtg= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf2.ScUsbOtg;
    break;

    case 3:
    PlatformInfoHob->PlatformFeatureValue.OsSelection= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf3.OsSelection;
    PlatformInfoHob->PlatformFeatureValue.Vibrator= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf3.Vibrator;
    PlatformInfoHob->PlatformFeatureValue.Ssic1Support= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf3.Ssic1Support;
    PlatformInfoHob->PlatformFeatureValue.ScUsbOtg= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf3.ScUsbOtg;
    break;

    case 4:
    PlatformInfoHob->PlatformFeatureValue.OsSelection= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf4.OsSelection;
    PlatformInfoHob->PlatformFeatureValue.Vibrator= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf4.Vibrator;
    PlatformInfoHob->PlatformFeatureValue.Ssic1Support= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf4.Ssic1Support;
    PlatformInfoHob->PlatformFeatureValue.ScUsbOtg= SmipIafwPointer->FeatureConfig.PlatfromFeatureConf4.ScUsbOtg;
    break;

    default:
    DEBUG((EFI_D_INFO, " Feature ID is invalid \n"));
    break;
  }

  DEBUG((EFI_D_INFO, " BOM ID is %x \n ", PlatformInfoHob->BomIdPss));
  DEBUG((EFI_D_INFO, " PlatformBOMValue PanelSel is %x \n ", PlatformInfoHob->PlatformBOMValue.PanelSel));
  DEBUG((EFI_D_INFO, " PlatformBOMValue WorldCameraSel is %x \n ", PlatformInfoHob->PlatformBOMValue.WorldCameraSel));
  DEBUG((EFI_D_INFO, " PlatformBOMValue UserCameraSel is %x \n ", PlatformInfoHob->PlatformBOMValue.UserCameraSel));
  DEBUG((EFI_D_INFO, " PlatformBOMValue AudioSel is %x \n ", PlatformInfoHob->PlatformBOMValue.AudioSel));
  DEBUG((EFI_D_INFO, " PlatformBOMValue ModemSel is %x \n ", PlatformInfoHob->PlatformBOMValue.ModemSel));
  DEBUG((EFI_D_INFO, " PlatformBOMValue TouchSel is %x \n ", PlatformInfoHob->PlatformBOMValue.TouchSel));
  DEBUG((EFI_D_INFO, " PlatformBOMValue WifiSel is %x \n ", PlatformInfoHob->PlatformBOMValue.WifiSel));
  DEBUG((EFI_D_INFO, " OS Selection ID is %x \n ", PlatformInfoHob->OsSelPss));
  DEBUG((EFI_D_INFO, " PlatformFeatureValue OsSelection is %x \n ", PlatformInfoHob->PlatformFeatureValue.OsSelection));
  DEBUG((EFI_D_INFO, " PlatformFeatureValue Vibrator is %x \n ", PlatformInfoHob->PlatformFeatureValue.Vibrator));
  DEBUG((EFI_D_INFO, " PlatformFeatureValue Ssic1Support is %x \n ", PlatformInfoHob->PlatformFeatureValue.Ssic1Support));
  DEBUG((EFI_D_INFO, " PlatformFeatureValue ScUsbOtg is %x \n ", PlatformInfoHob->PlatformFeatureValue.ScUsbOtg));

  Status = GpioSmipInit( &(SmipIafwPointer->PlatformConfig),PlatformInfoHob);
//[-start-160406-IB07400715-remove]//
////[-start-160817-IB03090432-add]//
//#else
//  PlatformInfoHob->OsSelPss = SmipIafwPointer->FeatureConfig.PlatfromFeatureConf4.OsSelection;
//  DEBUG((EFI_D_INFO, " OS Selection ID is %x \n ", PlatformInfoHob->OsSelPss));
////[-end-160817-IB03090432-add]//
//#endif
//[-end-160406-IB07400715-remove]//
  return Status;
}
//[-end-160803-IB07220122-modify]//
//[-end-151229-IB03090424-modify]//


