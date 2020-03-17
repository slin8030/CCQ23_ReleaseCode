/**@file
  Do platform specific PEI stage initializations.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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





//[-start-160317-IB03090425-modify]//
#include "PlatformInit.h"
#include <Library/HeciMsgLib.h>
#include <Library/SteppingLib.h>
#include <Guid/PlatformInfo.h>
#include <Guid/PttPTPInstanceGuid.h>
#include <Ppi/Smbus2.h>
#ifdef FSP_WRAPPER_FLAG
#include <Private/Library/CpuS3Lib.h>
#endif
//[-start-160308-IB10860193-add]//
#include <Guid/CustomVariable.h>
//[-end-160308-IB10860193-add]//
//[-start-161229-IB04530802-add]//
#include <Guid/PlatformHardwareSwitch.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/PeiChipsetSvcLib.h>
//[-end-161229-IB04530802-add]//
//[-start-160218-IB07400702-add]//
#include <ChipsetPostCode.h>
//[-end-160218-IB07400702-add]//
//[-start-160708-IB07400752-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160708-IB07400752-add]//

#define SETUP_NFC_Disabled 0
#define SETUP_NFC_IPT      1
#define SETUP_NFC          2

//[-start-160406-IB07400715-modify]//
//#if BXTI_PF_ENABLE
#define PMIC_A0            0
#define PMIC_A1            1
//[-start-161018-IB06740518-add]//
//[-start-161129-IB03090436-modify]//
//[-start-170315-IB07400848-modify]//
#ifndef PMIC_SETTINGS_ROLLBACK_SIC120
#define PMIC_SMBUS_REGISTER_SETTINGS {{0xAD,0x1C},{0x1F,0x01},{0x2A,0x09},{0xA5,0xF3},{0xC3,0x44},{0xD4,0x00},{0xE6,0x3E},{0xE7,0x3C},{0xE9,0x3D},{0xEB,0x0D}} // SIC 1.4.1
//#define PMIC_SMBUS_REGISTER_SETTINGS {{0xAD,0x1A},{0x1F,0x01},{0x2A,0x09},{0xA5,0xF3},{0xC3,0x44},{0xD4,0x00},{0xE6,0x3E},{0xE7,0x3C},{0xE9,0x3D},{0xEB,0x0D}} // SIC 1.4.0
//#define PMIC_SMBUS_REGISTER_SETTINGS {{0xAD,0x1A},{0x1F,0x01},{0x2A,0x0D},{0xA5,0xF3},{0xC3,0x44},{0xD4,0x00},{0xE6,0x3E},{0xE7,0x3C},{0xE9,0x3D},{0xEB,0x0D}}
#else
//
// BugBug: Rollback PMIC offset 0x2A/0xE6 setting to SIC v1.2.0 for Wake on USB issue.
//
//#define PMIC_SMBUS_REGISTER_SETTINGS {{0xAD,0x1A},{0x1F,0x01},{0x2A,0x0D},{0xA5,0xF3},{0xC3,0x44},{0xD4,0x00},{0xE6,0x3E},{0xE7,0x3C},{0xE9,0x3D},{0xEB,0x0D}} // SIC 1.2.3
//#define PMIC_SMBUS_REGISTER_SETTINGS {{0xAD,0x1A},{0x1F,0x01},{0x2A,0x09},{0xA5,0xF3},{0xC3,0x44},{0xD4,0x00},{0xE6,0x3E},{0xE7,0x3C},{0xE9,0x3D},{0xEB,0x0D}} // SIC 1.2.1
//#define PMIC_SMBUS_REGISTER_SETTINGS {{0xAD,0x18},{0x1F,0x01},{0x2A,0x09},{0xA5,0xE7},{0xC3,0x22},{0xD4,0x22},{0xE6,0x35},{0xE7,0x35},{0xE9,0x36},{0xEB,0x03}} // SIC 1.2.0
#define PMIC_SMBUS_REGISTER_SETTINGS   {{0xAD,0x1A},{0x1F,0x01},{0x2A,0x09},{0xA5,0xF3},{0xC3,0x44},{0xD4,0x00},{0xE6,0x35},{0xE7,0x3C},{0xE9,0x3D},{0xEB,0x0D}} // Modify
#endif
//[-end-170315-IB07400848-modify]//
//[-end-161129-IB03090436-modify]//

typedef struct {
  UINT8 Command;
  UINT8 Data;
} PMIC_REG_SETTING;
//[-end-161018-IB06740518-add]//

//#endif
//[-end-160406-IB07400715-modify]//

#ifdef __GNUC__
  #pragma GCC push_options
  #pragma GCC optimize ("O0")
#else
  #pragma optimize ("", off)
#endif

#ifdef FSP_WRAPPER_FLAG
EFI_STATUS
CpuS3SmmAccessNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );
#endif
static EFI_PEI_RESET_PPI            mResetPpi = { IchReset };

static EFI_PEI_PPI_DESCRIPTOR       mPpiList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiPeiMasterBootModePpiGuid,
    NULL
  },
  {
    EFI_PEI_PPI_DESCRIPTOR_PPI,
    &gEfiTpmDeviceSelectedGuid,
    NULL
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiResetPpiGuid,
    &mResetPpi
  }
};

static EFI_PEI_NOTIFY_DESCRIPTOR    mNotifyList[] = {
#ifdef FSP_WRAPPER_FLAG
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
    &gPeiSmmAccessPpiGuid,
    CpuS3SmmAccessNotifyCallback
  },
#endif
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiEndOfPeiSignalPpiGuid,
    EndOfPeiPpiNotifyCallback
  }
};

//[-start-160509-IB03090427-remove]//
////
////PMIC Init Table
////
//RegInit_st g_MiscRegInit[]= {
//#if RVVP_ENABLE == 1
//  // Power setting for camera
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4BVSEL, 0x33}, //0x4E  0xC3  0x33   Set voltage to 2.8 for VP2P8_AF  0.31
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4CVSEL, 0x33}, //0x4E  0xC4  0x33   Set voltage to 2.8 for VP2P8_CAM
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1BVSEL, 0x13}, //0x4E  0xB7  0x13   Set voltage to 1.2 for VP1P2_MCSI_A
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BCNT_REG,  REG_OVERRIDE, WKC_MASK_MODEVPROG4B, 0x01}, //PmicWrite8(DEVICE1_BASE_ADDRESS, 0xA4, 0x01);
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CCNT_REG,  REG_OVERRIDE, WKC_MASK_MODEVPROG4C, 0x01}, //PmicWrite8(DEVICE1_BASE_ADDRESS, 0xA5, 0x01);
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BCNT_REG,  REG_OVERRIDE, WKC_MASK_MODEVPROG1B, 0x01}, //PmicWrite8(DEVICE1_BASE_ADDRESS, 0x70, 0x01);
//#else
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2AVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2AVSEL, 0x1F},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2ACNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2A, 0x01},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2BVSEL, 0x1F}, ///< Turn on DMIC
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2B, 0x01}, ///< Turn on DMIC
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2DVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2DVSEL, 0x1F},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2DCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2D, 0x01},
//#if RVV_ENABLE == 1
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG3BVSEL, 0x3D},  //3.3V for Touch
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG3B, 0x01},
//#else
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG3BVSEL, 0x3D},  //3.3V for Touch for Turly AOB
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG3BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG3B, 0x01},
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG6BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG6BVSEL, 0x3D},  //3.3V for Touch for Tianma AOB
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG6BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG6B, 0x01},
//#endif
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1DVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1DVSEL, 0x1F},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1DCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1D, 0x01},
//
//#if RVV_ENABLE == 1
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG2CVSEL, 0x3D},  //3.3V for WIFI
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG2CCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG2C, 0x01},
//#else
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4CVSEL, 0x31}, // 2.7V FOR A0: Camera IMX214 V_CAM_AVDD_W1 -> V_VPROG4_C
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4CCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG4C, 0x01},
//#endif
//  {DEVICE1_BASE_ADDRESS, WKC_VFLEXVSEL_REG,    REG_OVERRIDE, WKC_MASK_VFLEXVSEL,   0x4B}, // 1.0V FOR A0: Camera IMX214 V_CAM_DVDD_W1/W2 -> V_VFLEX
//  {DEVICE1_BASE_ADDRESS, WKC_VFLEXCNT_REG,     REG_OVERRIDE, WKC_MASK_VFLEXEN,     0x01},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1CVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1CVSEL, 0x1F}, // 1.8V FOR A0: Camera IMX214 V_CAM_DOVDD_W1/U1 -> V_VPROG1_C
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1CCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1C, 0x01},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG5AVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG5AVSEL, 0x33}, // 2.8V FOR A0: Camera IMX214 V_CAM_VDD_AF-> V_VPROG5_A
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG5ACNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG5A, 0x01},
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4BVSEL, 0x31}, // 2.7V FOR A0: Camera IMX132 V_CAM_AVDD_U1->V_VPROG4_B
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG4B, 0x01},
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1EVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1EVSEL, 0x13}, // 1.2V FOR A0: Camera IMX132 V_CAM_DVDD_U1->V_VPROG1_E
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1ECNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1E, 0x01},
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG1BVSEL, 0x1F}, // 1.8V for VGROP1B: ISH
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG1BCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG1B, 0x01},
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG3AVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG3AVSEL, 0x34}, // 2.85V for VGROP3A : ISH
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG3ACNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG3A, 0x01},
//
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4DVSEL_REG,  REG_OVERRIDE, WKC_MASK_VPROG4DVSEL, 0x3B}, // 3.2V FOR SDcard
//  {DEVICE1_BASE_ADDRESS, WKC_VPROG4DCNT_REG,   REG_OVERRIDE, WKC_MASK_MODEVPROG4D, 0x01},
//
//  {DEVICE2_BASE_ADDRESS, WKC_CLKCONFIG_REG,    REG_OVERRIDE, (WKC_MASK_SLP2DLVL | WKC_MASK_SLP2D), 0x00},
//  {DEVICE1_BASE_ADDRESS, WKC_GPIO1P0CTLO_REG,  REG_OVERRIDE, 0x7B, 0x31},
////  {DEVICE1_BASE_ADDRESS, WKC_MODEMCTRL_REG,    REG_CLEAR,    WKC_MASK_SDWNDRV, 0x00},     // Clear SDWNDRV for modem Android, move to PlatformInitPreMem.c
//#endif
//};
//[-end-160509-IB03090427-remove]//

CHIPSET_CONFIGURATION           mSystemConfiguration;
EFI_PLATFORM_INFO_HOB           *mPlatformInfo = NULL;


//[-start-160803-IB07220122-remove]//
// EFI_STATUS
// SetMemoryConfigValues(
//   IN CONST EFI_PEI_SERVICES             **PeiServices,
//   IN       CHIPSET_CONFIGURATION        *SystemConfigData
//   )
//  {
//    EFI_STATUS                         Status;
//    DRAM_POLICY_PPI                    *DramPolicyPpi;
//  
//   Status = (*PeiServices)->LocatePpi (
//     PeiServices,
//     &gDramPolicyPpiGuid,
//     0,
//     NULL,
//     (VOID **)&DramPolicyPpi
//     );
//  
//   if (EFI_ERROR(Status)) {
//     DEBUG ((EFI_D_INFO, "Couldn't find DRAM policy ppi: %g, Status: %r.\n", &gDramPolicyPpiGuid, Status));
//   } else {
//     DEBUG ((EFI_D_INFO, "Overriding Memory System Config Data using Dram Policy.\n"));
//     SystemConfigData->ChannelHashMask         = DramPolicyPpi->ChannelHashMask;
//     SystemConfigData->SliceHashMask           = DramPolicyPpi->SliceHashMask;
//     SystemConfigData->ChannelsSlicesEnabled   = DramPolicyPpi->ChannelsSlicesEnabled;
//     SystemConfigData->ScramblerSupport        = DramPolicyPpi->ScramblerSupport;
//     SystemConfigData->InterleavedMode         = DramPolicyPpi->InterleavedMode;
//     SystemConfigData->MinRefRate2xEnabled     = DramPolicyPpi->MinRefRate2xEnabled;
//     SystemConfigData->DualRankSupportEnabled  = DramPolicyPpi->DualRankSupportEnabled;
//   }
//   return Status;
//  }
//[-end-160803-IB07220122-remove]//

/*

CSE XML Stap    0    0    0    0    1    1    1    1
Board Fab B/C   0    1    0    1    0    1    0    1
Finally Setting N    N    N    N    N    D    N    D

RVP Board Fab B/C does not support secure NFC as HW limitation.
Default state of secure NFC is disabled by default in Setup.
N: Do nothing
E: Enable secure NFC
D: Disable secure NFC

Summary:
  Enable Secure NFC : In this function, we will not actively enable secure NFC from BIOS side, as CSE will enable it as XML setting.
                      CSE XML Strap 1 && Board Fab B/C 0, secure NFC will be enabled by CSE by HW default
  Disable Secure NFC: BIOS Setup 1 && Board Fab B/C 1

  This function only is executed at the first time when variable is not avalaible yet.

*/
//[-start-160803-IB07220122-remove]//
// EFI_STATUS
// GetSecureNfcInfo (
//   VOID
//   )
//  {
//   EFI_STATUS                      Status;
//   UINTN                           VarSize;
//   EFI_PEI_HOB_POINTERS            GuidHob;
//   MEFWCAPS_SKU                    CurrentFeatures;
//   UINT32                          EnableBitmap = 0;
//   UINT32                          DisableBitmap = 0;
//   BOOLEAN                         Cse_Nfc_Strap = FALSE;
//   BOOLEAN                         RVP_Fab_B_C = FALSE;
//  
//   DEBUG ((EFI_D_INFO, "GetSecureNfcInfo ++ \n"));
//  
//   VarSize = PcdGet32 (PcdSetupConfigSize);
//   Status = GetSystemConfigData(&mSystemConfiguration, &VarSize);
//   if (EFI_ERROR (Status)) {
//     ASSERT_EFI_ERROR (Status);
//     return Status;
//   }
//  
//   mSystemConfiguration.NfcSelect = SETUP_NFC;
//   mSystemConfiguration.SecureNfcAvailable  = 0;
//   mSystemConfiguration.SecureNfc = 0;
//  
//   Status = HeciGetFwFeatureStateMsgII (&CurrentFeatures);
//   if (EFI_ERROR (Status)) {
//     return Status;
//   }
//  
//   if (CurrentFeatures.Fields.NFC == 1) {
//     Cse_Nfc_Strap = TRUE;
//   }
//  
//   DEBUG ((EFI_D_INFO, "HeciGetFwFeatureStateMsgII CurrentFeatures.Fields.NFC: %d\n", CurrentFeatures.Fields.NFC));
//  
//   //
//   // Get the HOB list.  If it is not present, then ASSERT.
//   //
//   GuidHob.Raw = GetHobList ();
//   if (GuidHob.Raw != NULL) {
//     if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
//       mPlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
//     }
//   }
//  
//   if ( NULL == mPlatformInfo ) {
//     ASSERT(NULL != mPlatformInfo);
//     return EFI_NOT_FOUND;
//   }
//  
//   if ((mPlatformInfo->BoardId == BOARD_ID_BXT_RVP) && (mPlatformInfo->BoardRev == FAB_ID_RVP_B_C)) {
//     RVP_Fab_B_C = TRUE;
//   }
//  
//   if (Cse_Nfc_Strap) {
//     mSystemConfiguration.SecureNfcAvailable  = 1;
//     mSystemConfiguration.NfcSelect = SETUP_NFC_IPT;
//     mSystemConfiguration.SecureNfc = 1;
//  
//     if (RVP_Fab_B_C) {
//       DEBUG ((EFI_D_INFO, "Secure NFC should be disabled for BXT RVP Fab B/C.\n"));
//       //
//       // Disable secure NFC
//       //
//       DEBUG ((EFI_D_INFO, "Disable NFC\n"));
//       EnableBitmap  = CLEAR_FEATURE_BIT;
//       DisableBitmap = NFC_BITMASK;
//  
//       Status = HeciFwFeatureStateOverride (EnableBitmap, DisableBitmap);
//       ASSERT_EFI_ERROR (Status);
//  
//       mSystemConfiguration.SecureNfcAvailable = 0;
//       mSystemConfiguration.NfcSelect = SETUP_NFC;
//       mSystemConfiguration.SecureNfc = 0;
//  
//       DEBUG ((EFI_D_INFO, "NfcSelect: %d\n", mSystemConfiguration.NfcSelect));
//     }
//   }
//  
//   Status = SetSystemConfigData(&mSystemConfiguration, VarSize);
//   ASSERT_EFI_ERROR (Status);
//   DEBUG ((EFI_D_INFO, "NfcSelect: %d, SecureNfcAvailable: %d\n", mSystemConfiguration.NfcSelect, mSystemConfiguration.SecureNfcAvailable));
//  
//   return Status;
//  }
//[-end-160803-IB07220122-remove]//

//[-start-160429-IB07220075-add]//
//[-start-160803-IB07220122-remove]//
// VOID 
// HeciSetReadWriteCache(
//   VOID*  Address
// );
//[-end-160803-IB07220122-remove]//
//[-end-160429-IB07220075-add]//

//[-start-160803-IB07220122-remove]//
// EFI_STATUS
// InitSystemConfig(
//   IN CONST EFI_PEI_SERVICES             **PeiServices,
//   IN       EFI_PLATFORM_INFO_HOB       *PlatformInfo
//   )
//  {
//   CHIPSET_CONFIGURATION              SystemConfiguration;
//   CHIPSET_CONFIGURATION              SystemConfigData;
//   UINTN                              VariableSize;
//   EFI_PEI_READ_ONLY_VARIABLE2_PPI   *Variable;
//   EFI_STATUS                         Status;
//  
// //[-start-160308-IB10860193-add]//
//   EFI_STATUS                        Status1;
//   UINTN                             SetSetupAtRuntimeSize;
//   UINT8                             SetSetupAtRuntime;
//  
//   EFI_STATUS                        Status2;
//   UINTN                             SetSetupInSmmSize;
//   UINT8                             SetSetupInSmm;
//  
//  
//  
//   Status = (*PeiServices)->LocatePpi (
//                              PeiServices,
//                              &gEfiPeiReadOnlyVariable2PpiGuid,
//                              0,
//                              NULL,
//                              (VOID **) &Variable
//                              );
//   ASSERT_EFI_ERROR (Status);
// //[-end-160308-IB10860193-add]//
//   VariableSize = PcdGet32 (PcdSetupConfigSize);
//   Status = GetSystemConfigData (&SystemConfiguration, &VariableSize);
//   if (!EFI_ERROR(Status)) {
//     DEBUG ((EFI_D_INFO, "System Config Data already init.\n"));
//     //
//     //If switch is changed, load the Multi-BOM IA SMIP settings.
//     //
// //[-start-160308-IB10860193-modify]//	
// //[-start-151204-IB02950555-modify]//
//     if (PcdGetBool(PcdNvStorageHaveVariable)) {
//      
// 	  SetSetupAtRuntimeSize = sizeof(UINT8);
//       Status1 = Variable->GetVariable (
//                        Variable,
//                        L"SetSetupAtRuntime",
//                        &gCustomVariableGuid,
//                        NULL,
//                        &SetSetupAtRuntimeSize,
//                        &SetSetupAtRuntime
//                        );
//  
//       SetSetupInSmmSize = sizeof(UINT8);
//       Status2 = Variable->GetVariable (
//                        Variable,
//                        L"SetSetupInSmm",
//                        &gCustomVariableGuid,
//                        NULL,
//                        &SetSetupInSmmSize,
//                        &SetSetupInSmm
//                        );
//       
//       if ((!EFI_ERROR(Status1) && SetSetupAtRuntime == 0x1) || (!EFI_ERROR(Status2) && SetSetupInSmm == 0x1)) { 
//         //
//         // Rescan NVStorage 
//         //
//         VariableSize = PcdGet32 (PcdSetupConfigSize);  
//         ZeroMem(&SystemConfigData, VariableSize);
//         Status = Variable->GetVariable (
//                        Variable,
//                        SETUP_VARIABLE_NAME,
//                        &gSystemConfigurationGuid,
//                        NULL,
//                        &VariableSize,
//                        &SystemConfigData
//                        ); 
//         ASSERT_EFI_ERROR (Status);
//  
//         if (EFI_ERROR(Status)) {
//           GenerateSystemConfigData(&SystemConfiguration, VariableSize,&SystemConfigData,&VariableSize);
//         }
//       } else {
//         GenerateSystemConfigData(&SystemConfiguration, VariableSize, &SystemConfigData, &VariableSize);
//       }
//     } else {
//       //
//       // First boot
//       // 
//       GenerateSystemConfigData(&SystemConfiguration, VariableSize,&SystemConfigData,&VariableSize);
//     }
// //[-end-160308-IB10860193-modify]//
//     Status = SetMemoryConfigValues(PeiServices, &SystemConfigData);
//     if (EFI_ERROR(Status)) {
//       DEBUG ((EFI_D_INFO, "Couldn't override memory system config after feature/bom change.\n"));
//     }
// //[-start-160429-IB07220075-add]//
//     HeciSetReadWriteCache (AllocatePages (EFI_SIZE_TO_PAGES (VariableSize)));
// //[-end-160429-IB07220075-add]//
//     SetSystemConfigData(&SystemConfigData, VariableSize);
// //[-end-151204-IB02950555-modify]//
//     return EFI_SUCCESS;
//   }
// //[-start-160308-IB10860193-remove]//
// //  VariableSize = PcdGet32 (PcdSetupConfigSize);
// //  ZeroMem (&SystemConfiguration, VariableSize);
// //
// //  Status = (*PeiServices)->LocatePpi (
// //                             PeiServices,
// //                             &gEfiPeiReadOnlyVariable2PpiGuid,
// //                             0,
// //                             NULL,
// //                             (VOID **) &Variable
// //                             );
// //  ASSERT_EFI_ERROR (Status);
// //[-end-160308-IB10860193-remove]//
//  
//   //
//   // Use normal setup default from NVRAM variable,
//   // the Platform Mode (manufacturing/safe/normal) is handle in PeiGetVariable.
//   //
//   VariableSize = PcdGet32 (PcdSetupConfigSize);
// //[-start-160308-IB10860193-remove]//
//   ZeroMem (&SystemConfiguration, VariableSize);
// //[-end-160308-IB10860193-remove]//  
//   Status = Variable->GetVariable (
//                        Variable,
//                        SETUP_VARIABLE_NAME,
//                        &gSystemConfigurationGuid,
//                        NULL,
//                        &VariableSize,
//                        &SystemConfiguration
//                        );
//   ASSERT_EFI_ERROR(Status);
//   //
//   //Load the Multi-BOM IA SMIP settings
//   //
//   GenerateSystemConfigData(&SystemConfiguration, VariableSize,&SystemConfigData,&VariableSize);
//   // bug-bug:  should not overwrite MemConfig with VPD data
//   Status = SetMemoryConfigValues(PeiServices, &SystemConfigData);
//   if (EFI_ERROR(Status)) {
//     DEBUG ((EFI_D_INFO, "Couldn't override memory system config on first boot.\n"));
//   }
//  
//   SetSystemConfigData(&SystemConfigData, VariableSize);
//  
//   //
//   // Process Secure NFC info, only execute once.
//   //
//   if ((PLATFORM_ID == VALUE_REAL_PLATFORM)){
//     Status = GetSecureNfcInfo();
//   }
//  
//   DEBUG ((EFI_D_INFO, "InitSystemConfig Done.\n"));
//   return EFI_SUCCESS;
//  }
//[-end-160803-IB07220122-remove]//

//[-start-160803-IB07220122-add]//
EFI_STATUS
GetSetupVariable (
  IN CONST EFI_PEI_SERVICES                **PeiServices,
  IN   CHIPSET_CONFIGURATION         *SystemConfiguration
  )
{
  UINTN                        VariableSize;
  EFI_STATUS                   Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;

  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//   Status = GetSystemConfigData (SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               SystemConfiguration
                               );
  ASSERT_EFI_ERROR(Status);
  return EFI_SUCCESS;
}
//[-end-160803-IB07220122-add]//

//[-start-160829-IB03090433-modify]//
EFI_STATUS
TpmPolicyInit (
  IN CONST EFI_PEI_SERVICES        **PeiServices,
  IN CHIPSET_CONFIGURATION         *SystemConfiguration
  )
{
#if FTPM_SUPPORT
  EFI_STATUS                  Status;
  UINTN                       Size;
  BOOLEAN                     IsPttEnabled = TRUE;
//[-start-161101-IB03090435-modify]//
//[-start-161125-IB07400818-modify]//
//#if BXTI_PF_ENABLE
  BOOLEAN                     PttEnabledState;
//#else
  MBP_ME_FW_CAPS              Mbp_Me_Fw_Caps = { 0 };
//#endif
//[-end-161125-IB07400818-modify]//
//[-end-161101-IB03090435-modify]//
  EFI_BOOT_MODE               BootMode;

  DEBUG ((EFI_D_ERROR, "TpmPolicyInit entry.\n"));

  if (SystemConfiguration->TPM == TPM_PTT) {

	  (*PeiServices)->GetBootMode(PeiServices, &BootMode);

	  if ((BootMode != BOOT_ON_S3_RESUME)) {
//[-start-161101-IB03090435-modify]//
//[-start-161125-IB07400818-modify]//
//#if BXTI_PF_ENABLE
      if (IsIOTGBoardIds()) {
          //
          // HSD[1504340798] IOTG CRB unable to get correct set of MBP table, and returns EFI_DEVICE_ERROR from HeciGetMeFwCapability.
		  // This causes IsPttEnabled always FALSE, hence TPM is not available in OS. Use PttHeciGetState as W/A until rootcause the issue.
          //
		  Status = PttHeciGetState (&PttEnabledState);
          if (EFI_ERROR (Status)) {
            return Status;
          }
		  IsPttEnabled = PttEnabledState;
//#else
      } else {
		  //
		  // Get ME FW Capability from MBP table to determine PTT State
		  //
		  Status = HeciGetMeFwCapability(&Mbp_Me_Fw_Caps);
		  if (!EFI_ERROR(Status)) {
			  IsPttEnabled = (BOOLEAN)Mbp_Me_Fw_Caps.CurrentFeatures.Fields.PTT;
		  } else {
              IsPttEnabled = FALSE;
          }
      }
//#endif
//[-end-161125-IB07400818-modify]//
//[-end-161101-IB03090435-modify]//
		  PcdSetBoolS (PcdCsePttEnable, IsPttEnabled);
	  } else {   // On S3 Resume
		  if (SystemConfiguration->CsePttStatus == 1) {
			  IsPttEnabled = TRUE;
		  } else {
			  IsPttEnabled = FALSE;
		  }
	  }

      if ((IsPttEnabled == FALSE) || (SystemConfiguration->PttSuppressCommandSend == 1)) {
          PcdSetPtr (PcdTpmInstanceGuid, &Size, &gEfiTpmDeviceInstanceNoneGuid);
          DEBUG((DEBUG_ERROR, "BIOS will send no further commands to PTT.\n"));
      } else {
          //
          // Set PcdTpmInstanceGuid to PTT
          //
          Size = sizeof(gTpmDeviceInstanceTpm20PttPtpGuid);
          PcdSetPtr (PcdTpmInstanceGuid, &Size, &gTpmDeviceInstanceTpm20PttPtpGuid);
          DEBUG((DEBUG_INFO, "Set PcdTpmInstanceGuid to PTT.\n"));
      }
  } else if (SystemConfiguration->TPM == TPM_DTPM_1_2) {
    //
    // Set PcdTpmInstanceGuid to dTPM 1.2
    //
    Size = sizeof(gEfiTpmDeviceInstanceTpm12Guid);
    PcdSetPtr (PcdTpmInstanceGuid, &Size, &gEfiTpmDeviceInstanceTpm12Guid);
    DEBUG((DEBUG_INFO, "Set PcdTpmInstanceGuid to dTPM 1.2.\n"));
  } else if (SystemConfiguration->TPM == TPM_DTPM_2_0) {
    //
    // Set PcdTpmInstanceGuid to dTPM 2.0
    //
    Size = sizeof(gEfiTpmDeviceInstanceTpm20DtpmGuid);
    PcdSetPtr (PcdTpmInstanceGuid, &Size, &gEfiTpmDeviceInstanceTpm20DtpmGuid);
    DEBUG((DEBUG_INFO, "Set PcdTpmInstanceGuid to dTPM 2.0.\n"));
  } else {
    //
    // Set PcdTpmInstanceGuid to NONE
    //
    Size = sizeof(gEfiTpmDeviceInstanceNoneGuid);
    PcdSetPtr (PcdTpmInstanceGuid, &Size, &gEfiTpmDeviceInstanceNoneGuid);
    DEBUG((DEBUG_INFO, "Set PcdTpmInstanceGuid to NONE.\n"));
  }
#endif
  return EFI_SUCCESS;
}
//[-end-160829-IB03090433-modify]//

/**
  This function will power off the system at once
**/
VOID
PowerOffNow(
  VOID
  )
{
  UINT16  AcpiBaseAddr;

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  DEBUG ((DEBUG_INFO, " PowerOffNow: AcpiBaseAddr = 0x%x\n",AcpiBaseAddr));

  //
  // Set PM Register and Put system to S5
  //
  IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5);
  IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, V_ACPI_PM1_CNT_S5 + B_ACPI_PM1_CNT_SLP_EN);
}

/**
  This function Check valid Power button pressed .

  @param[in] PeiServices            An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param[in] SystemConfiguration    The pointer to get System Setup


  @retval     EFI_SUCCESS            Power button press is valid and system will be powered on.
  @retval     EFI ERRORS             Power button press is invalid and system will be shutdown.
**/
//[-start-151204-IB02950555-modify]//
EFI_STATUS
PwrBtn500msChk (
  IN CONST EFI_PEI_SERVICES          **PeiServices,
  IN   CHIPSET_CONFIGURATION         *SystemConfiguration
  )
{
//[-start-160803-IB07220122-remove]//
//   UINTN                   VariableSize;
//   EFI_STATUS              Status;
//[-end-160803-IB07220122-remove]//
  UINT64                  MicroSec;

//[-start-160803-IB07220122-remove]//
//   VariableSize = PcdGet32 (PcdSetupConfigSize);
//   Status = GetSystemConfigData (SystemConfiguration, &VariableSize);
//   ASSERT_EFI_ERROR(Status);
//[-end-160803-IB07220122-remove]//

  MicroSec = DivU64x32 (GetTimeInNanoSecond (GetPerformanceCounter()), 1000);
  DEBUG ((DEBUG_INFO, " PwrBtn500msChk: MicroSec = 0x%x, SystemConfiguration->PowerBtn500msEn = 0x%x\n",MicroSec,SystemConfiguration->PowerBtn500msEn));

  if(SystemConfiguration->PowerBtn500msEn){
    if (!PmicIsPwrBtnPressed()) { //released
      PowerOffNow();
    }
  }
  return EFI_SUCCESS;
}
//[-end-151204-IB02950555-modify]//

//[-start-160406-IB07400715-modify]//
//#if !BXTI_PF_ENABLE
#if TABLET_PF_ENABLE
//[-end-160406-IB07400715-modify]//
EFI_STATUS
Vibra_Buzz_Start(
  VOID
  )
{
  PWMCTRL     PwmCtl = {0};
  UINT32      TimeOut = 0;
  EFI_STATUS  Status = EFI_SUCCESS;
  UINT32      D32 = 0;

  //
  // Program GPIO_35 output high to enable Vibra device
  //
  D32 = GpioPadRead(N_GPIO_35);
  D32 &= ~(UINT32)(0x0300);
  D32 = D32|(0x0201);
  GpioPadWrite(N_GPIO_35,D32);

  //
  // Set PWM Base Address
  //
  MmioWrite32(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC_PWM, PCI_FUNCTION_NUMBER_PMC_PWM, R_LPSS_IO_BAR),
    PWM_TEMP_MEM_BASE_ADDRESS
    );

  //
  // Enable PWM Memory decode
  //
  MmioWrite16(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC_PWM, PCI_FUNCTION_NUMBER_PMC_PWM, R_LPSS_IO_STSCMD),
    B_LPSS_IO_STSCMD_BME| B_LPSS_IO_STSCMD_MSE
    );

  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS, PwmCtl.Raw);


/*
  Initial Enable or First Activation for PWM
  1. Program the Base Unit and On Time Divisor values.
  2. Set the Software Update Bit.
  3. Enable the PWM Output by setting PWM Enable.
  4. Repeat the above steps for the next PWM Module.
*/

  PwmCtl.BITS.Pwm_Base_Unit = PWM_BASE_UNIT;
  PwmCtl.BITS.Pwm_On_Time_Divisor = PWM_DUTY_CYCLE;
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS, PwmCtl.Raw);

  PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS);
  PwmCtl.BITS.Pwm_Sw_Update = 1;
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS, PwmCtl.Raw);

  PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS);
  while (((PwmCtl.Raw & BIT30) == BIT30) && (TimeOut < PWM_TIMEOUT_MAX)) {
    MicroSecondDelay(1);
    PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS);
    TimeOut ++;
  }

  PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS);
  PwmCtl.BITS.Pwm_Enable = 1;
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS, PwmCtl.Raw);

  if (TimeOut > PWM_TIMEOUT_MAX) {
    Status = EFI_TIMEOUT;
  } else {
    MicroSecondDelay(PWM_DELAY); // 1 second delay
  }
  return Status;
}

EFI_STATUS
Vibra_Buzz_Stop(
  VOID
  )
{
  PWMCTRL  PwmCtl = {0};

  //
  // Clear PWM control register
  //
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS, PwmCtl.Raw);

  //
  // Clear PWM Base Address
  //
  MmioWrite32(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC_PWM, PCI_FUNCTION_NUMBER_PMC_PWM, R_LPSS_IO_BAR),
    0
    );

  //
  // Disable PWM Memory decode
  //
  MmioWrite16(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC_PWM, PCI_FUNCTION_NUMBER_PMC_PWM, R_LPSS_IO_STSCMD),
    0
    );
  return EFI_SUCCESS;
}
#else
EFI_STATUS
PWM_Fan_Start(
  VOID
  )
{
  PWMCTRL    PwmCtl = {0};
  UINT32     TimeOut = 0;
  EFI_STATUS Status = EFI_SUCCESS;

  //
  // Program GPIO_37 to enable FAN device
  //
//[-start-161109-IB07400810-remove]//
    //
    // Program in PEI Post-Memory Phase
    //
//  GpioPadWrite(N_GPIO_37,0x400);
//[-end-161109-IB07400810-remove]//

  //
  // Set PWM Base Address
  //
  MmioWrite32(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC_PWM, PCI_FUNCTION_NUMBER_PMC_PWM, R_LPSS_IO_BAR),
    PWM_TEMP_MEM_BASE_ADDRESS
    );

  //
  // Enable PWM Memory decode
  //
  MmioWrite16(
    MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_PMC_PWM, PCI_FUNCTION_NUMBER_PMC_PWM, R_LPSS_IO_STSCMD),
    B_LPSS_IO_STSCMD_BME| B_LPSS_IO_STSCMD_MSE
    );

  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00, PwmCtl.Raw);


/*
  Initial Enable or First Activation for PWM
  1. Program the Base Unit and On Time Divisor values.
  2. Set the Software Update Bit.
  3. Enable the PWM Output by setting PWM Enable.
  4. Repeat the above steps for the next PWM Module.
*/

  PwmCtl.BITS.Pwm_Base_Unit = 0x15;
  PwmCtl.BITS.Pwm_On_Time_Divisor = 0x80;
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00, PwmCtl.Raw);

  PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00);
  PwmCtl.BITS.Pwm_Sw_Update = 1;
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00, PwmCtl.Raw);

  PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00);
  while (((PwmCtl.Raw & BIT30) == BIT30) && (TimeOut < PWM_TIMEOUT_MAX)) {
    MicroSecondDelay(1);
    PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00);
    TimeOut ++;
  }

  PwmCtl.Raw = MmioRead32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00);
  PwmCtl.BITS.Pwm_Enable = 1;
  MmioWrite32 (PWM_TEMP_MEM_BASE_ADDRESS+0x0C00, PwmCtl.Raw);

  if (TimeOut > PWM_TIMEOUT_MAX) {
    Status = EFI_TIMEOUT;
  }
  return Status;
}

#endif

EFI_STATUS
BXTPolicyInit (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN CHIPSET_CONFIGURATION        *SystemConfiguration
  )
{
  EFI_STATUS                      Status;
  SI_SA_POLICY_PPI                *SiSaPolicyPpi;
  SA_MISC_CONFIG                  *MiscConfig = NULL;
  GRAPHICS_CONFIG                 *GtConfig = NULL;
  IPU_CONFIG                      *IpuPolicy = NULL;
//[-start-160629-IB07400749-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  HYBRID_GRAPHICS_CONFIG          *HgConfig = NULL;
#endif
#endif
//[-end-160629-IB07400749-modify]//
  VOID*                           Buffer;
  UINT32                          Size;
  EFI_GUID                        PeiLogoGuid        = { 0x7BB28B99, 0x61BB, 0x11D5, 0x9A, 0x5D, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D };
  EFI_GUID                        TianmaVbtGuid      = { 0xE08CA6D5, 0x8D02, 0x43ae, 0xAB, 0xB1, 0x95, 0x2C, 0xC7, 0x87, 0xC9, 0x33 };
//  EFI_GUID                        TrulyVbtGuid       = { 0xDBADD769, 0xE86A, 0x4819, 0x81, 0x20, 0xE9, 0x91, 0x79, 0x2C, 0x0B, 0xC1 };
  VBT_INFO                        VbtInfo;
  EFI_BOOT_MODE                   BootMode;

  DEBUG ((DEBUG_INFO, " BXTPolicyInit: SystemAgent PEI Platform Policy Initialization begin \n"));

  Status = CreateConfigBlocks (&SiSaPolicyPpi);
  DEBUG ((DEBUG_INFO, "SiSaPolicyPpi->TableHeader.NumberOfBlocks = 0x%x\n ", SiSaPolicyPpi->TableHeader.NumberOfBlocks ));
  ASSERT_EFI_ERROR (Status);


  Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gSaMiscConfigGuid , (VOID *)&MiscConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gGraphicsConfigGuid, (VOID *)&GtConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *)SiSaPolicyPpi, &gIpuConfigGuid, (VOID *)&IpuPolicy);
  ASSERT_EFI_ERROR (Status);

//[-start-160629-IB07400749-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
  Status = GetConfigBlock((VOID *)SiSaPolicyPpi, &gHybridGraphicsConfigGuid, (VOID *)&HgConfig);
  ASSERT_EFI_ERROR(Status);
#endif
#endif
//[-end-160629-IB07400749-modify]//

  if (!EFI_ERROR(Status)) {
    //
    // Get the Platform Configuration from SetupData
    //
    GtConfig->GttMmAdr = GTTMM_BASE_ADDRESS;
    GtConfig->GmAdr = GMADR_BASE_ADDRESS;
    GtConfig->PeiGraphicsPeimInit = SystemConfiguration->PeiGraphicsPeimInit;
    GtConfig->PmSupport = SystemConfiguration->PmSupport;
    GtConfig->EnableRenderStandby = SystemConfiguration->EnableRenderStandby;
    GtConfig->CdClock = SystemConfiguration->CdClock;
    GtConfig->PavpEnable = SystemConfiguration->PavpEnable;


    MiscConfig->FastBoot = 1;
    MiscConfig->DynSR = 1;
    IpuPolicy->SaIpuEnable = SystemConfiguration->IpuEn;
    IpuPolicy->IpuAcpiMode = SystemConfiguration->IpuAcpiMode;
    IpuPolicy->IpuMmAdr = IPUMM_BASE_ADDRESS;

//[-start-160629-IB07400749-modify]//
#if (ENBDT_PF_ENABLE == 1)
#ifdef HG_ENABLE
    ///
    /// In Hybrid Gfx mode PCIe needs to be always enabled
    /// and IGFX must be set as Primary Display.
    ///
     if (SystemConfiguration->PrimaryVideoAdaptor == 4) {
      HgConfig->HgEnabled     = 1;
      HgConfig->HgSubSystemId = 0x2112;
     } else {
         HgConfig->HgEnabled     = 0;
         HgConfig->HgSubSystemId = 0x2212;
    }

     HgConfig->HgDelayAfterPwrEn     = SystemConfiguration->DelayAfterPwrEn;
     HgConfig->HgDelayAfterHoldReset = SystemConfiguration->DelayAfterHoldReset;
    ///
    /// Configure below based on the OEM platfrom design
    /// Hybrid Graphics Enabled - 0= Disabled, 1=Enabled
    ///
    if (HgConfig->HgEnabled == 1) {
      ///
      /// dGPU HLD RST GPIO assigned
      ///
      HgConfig->HgDgpuHoldRst.CommunityOffset   = (((UINT32)GPIO_MMIO_OFFSET_W)<<16);
      HgConfig->HgDgpuHoldRst.PinOffset         = 0x05B0;
      HgConfig->HgDgpuHoldRst.Active            = 0;
      ///
      /// dGPU PWR Enable GPIO assigned
      ///
      HgConfig->HgDgpuPwrEnable.CommunityOffset  = (((UINT32)GPIO_MMIO_OFFSET_N)<<16);
      HgConfig->HgDgpuPwrEnable.PinOffset        = 0x0598;
      HgConfig->HgDgpuPwrEnable.Active           = 1;

      HgConfig->RootPortDev = PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2;
      HgConfig->RootPortFun = PCI_FUNCTION_NUMBER_PCIE_ROOT_PORT_3;

      DEBUG ((DEBUG_INFO, "HG::Hybrid Graphics Policy updated\n"));
    }
#endif
#endif
//[-end-160629-IB07400749-modify]//
  }

  Status = (*PeiServices)->GetBootMode((CONST EFI_PEI_SERVICES **) PeiServices, &BootMode);
  ASSERT_EFI_ERROR (Status);

  PeiGetSectionFromFv(PeiLogoGuid, &Buffer, &Size);
  if (Buffer == NULL) {
    DEBUG(( DEBUG_ERROR, "Could not locate PeiLogo"));
  }
  GtConfig->LogoPtr           = Buffer;
  GtConfig->LogoSize          = Size;
  DEBUG(( DEBUG_INFO, "LogoPtr from PeiGetSectionFromFv is 0x%x\n", Buffer));
  DEBUG(( DEBUG_INFO, "LogoSize from PeiGetSectionFromFv is 0x%x\n", Size));


//  if (SystemConfiguration->PanelSel == 1) {
    //
    //Load Tianma MIPI Panel VBT
    //
    DEBUG(( DEBUG_INFO, "Load Tianma MIPI Panel VBT......"));    
    PeiGetSectionFromFv(TianmaVbtGuid, &Buffer, &Size);
//  } else if (SystemConfiguration->PanelSel == 2) {
    //
    //Load Truly MIPI Panel VBT
    //
//    DEBUG(( DEBUG_INFO, "Load Truly MIPI Panel VBT......"));     
//    PeiGetSectionFromFv(TrulyVbtGuid, &Buffer, &Size);
//  } else {
//    DEBUG(( DEBUG_INFO, "Load Tianma MIPI Panel VBT......"));    	
//    PeiGetSectionFromFv(TianmaVbtGuid, &Buffer, &Size);
//  }
  
  if (Buffer == NULL) {
    DEBUG(( DEBUG_ERROR, "Could not locate VBT"));
  }

  if (BootMode == BOOT_ON_S3_RESUME) {
    GtConfig->GraphicsConfigPtr = NULL;
  } else {
    GtConfig->GraphicsConfigPtr = Buffer;
  }

  //Build the VBT data into HOB for DXE GOP
  VbtInfo.VbtAddress = (EFI_PHYSICAL_ADDRESS)Buffer;
  VbtInfo.VbtSize = Size;
  DEBUG(( DEBUG_INFO, "VbtInfo VbtAddress is 0x%x\n", Buffer));
  DEBUG(( DEBUG_INFO, "VbtInfo VbtSize is 0x%x\n", Size));

  BuildGuidDataHob (
  &gVbtInfoGuid,
  &VbtInfo,
  sizeof (VbtInfo)
  );

  //
  // Install SiSaPolicyPpi.
  // While installed, RC assumes the Policy is ready and finalized. So please
  // update and override any setting before calling this function.
  //
  Status = SiSaInstallPolicyPpi (SiSaPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, " SystemAgent PEI Platform Policy Initialization Done \n"));
  ASSERT_EFI_ERROR (Status);
  return EFI_SUCCESS;
}

//[-end-151204-IB02950555-modify]//

//[-start-161229-IB04530802-add]//
EFI_STATUS
EFIAPI
SetPlatformHardwareSwitch () 
{
  EFI_STATUS                     Status;
  PLATFORM_HARDWARE_SWITCH       PlatformHardwareSwitch;
  PLATFORM_HARDWARE_SWITCH       EmptyPlatformHardwareSwitch;
  PLATFORM_HARDWARE_SWITCH       *PlatformHardwareSwitchDptr;

  ZeroMem (&PlatformHardwareSwitch, sizeof (PLATFORM_HARDWARE_SWITCH));
  ZeroMem (&EmptyPlatformHardwareSwitch, sizeof (PLATFORM_HARDWARE_SWITCH));

  PlatformHardwareSwitchDptr = &PlatformHardwareSwitch;

  PeiCsSvcSetPlatformHardwareSwitch(&PlatformHardwareSwitchDptr);
  //
  // OemServices
  //
  Status = OemSvcSetPlatformHardwareSwitch (
             &PlatformHardwareSwitchDptr
             );

  if (!EFI_ERROR (Status)) {
    return Status;
  }

  if (CompareMem (
	&EmptyPlatformHardwareSwitch, 
	&PlatformHardwareSwitch, 
	sizeof (PLATFORM_HARDWARE_SWITCH)) == 0) {
    return Status;
  }

  //
  // Set PlatformHardwareSwitch data to HOB.
  //
  BuildGuidDataHob (
    &gPlatformHardwareSwitchGuid, 
    PlatformHardwareSwitchDptr, 
    sizeof (PLATFORM_HARDWARE_SWITCH)
    );

  return Status;
}
//[-end-161229-IB04530802-add]//


/**
  Platform Init PEI module entry point

  @param[in]  FileHandle           Not used.
  @param[in]  PeiServices          General purpose services available to every PEIM.

  @retval     EFI_SUCCESS          The function completes successfully
  @retval     EFI_OUT_OF_RESOURCES Insufficient resources to create database
**/
EFI_STATUS
EFIAPI
PlatformInitEntryPoint (
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS                  Status;
  CHIPSET_CONFIGURATION       SystemConfiguration;
  EFI_PLATFORM_INFO_HOB       *PlatformInfo;

  EFI_PEI_HOB_POINTERS        Hob;
//[-start-161227-IB4530802-add]//
//  PLATFORM_HARDWARE_SWITCH   *PlatformHardwareSwitch;
//[-end-161227-IB04530802-add]//

  DEBUG((EFI_D_INFO, "PlatformInit EntryPoint\n"));

  //
  // Set the some PCI and chipset range as UC
  // And align to 1M at leaset
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);


#if TABLET_PF_ENABLE
  if (PlatformInfo->PlatformFeatureValue.Vibrator == 1 && FeaturePcdGet(PcdVibratorFeature)) {
    DEBUG((EFI_D_INFO, "Start vibra %a(%d): %a()\n", __FILE__, __LINE__, __FUNCTION__));
    Vibra_Buzz_Start();
  }
#endif
  if (PlatformInfo->SsidSvid != 0) {
    PlatformInfo->SsidSvid = 0;
  }
//[-start-160803-IB07220122-remove]//
//   InitSystemConfig(PeiServices, PlatformInfo);
//[-end-160803-IB07220122-remove]//

//[-start-160817-IB03090432-remove]//
//  //
//  // Initialize GFX VT-d Base Address
//  //
//  if (BxtStepping() <= BxtA1) {
//    PcdSet32(PcdVtdGfxBaseAddress, 0xFF000000);
//  }
//[-end-160817-IB03090432-remove]//

  //Vibra_Buzz_Start();
//[-start-160406-IB07400715-modify]//
//#if BXTI_PF_ENABLE
  if ((PlatformInfo->BoardId == BOARD_ID_OXH_CRB) || (PlatformInfo->BoardId == BOARD_ID_LFH_CRB) || (PlatformInfo->BoardId == BOARD_ID_JNH_CRB)) {
    PWM_Fan_Start();
  }
//#endif
//[-end-160406-IB07400715-modify]//

  //
  // Initialize PlatformInfo HOB
  //
  MultiPlatformInfoInit(PeiServices, PlatformInfo);

  //Vibra_Buzz_Stop();

  //
  //Initialize PMIC
  //
#if RVVP_ENABLE ==1
  DEBUG ((EFI_D_ERROR, "RVVP should have only Camera Voltages %a(%d): %a()\n", __FILE__, __LINE__, __FUNCTION__));
#endif
//[-start-160509-IB03090427-modify]//
  PmicMiscInit ();
//[-end-160509-IB03090427-modify]//
  //
  // Set the new boot mode
  //
  Status = UpdateBootMode (PeiServices, PlatformInfo);
  ASSERT_EFI_ERROR (Status);

//  SetPlatformBootMode (PeiServices, PlatformInfo);

  //
  // Get setup variable. This can only be done after BootMode is updated
  //
//[-start-151222-IB03090424-remove]//
//  if ((PLATFORM_ID == VALUE_REAL_PLATFORM)){
//[-end-151222-IB03090424-remove]//
  GetSetupVariable (PeiServices, &SystemConfiguration);

//[-start-160509-IB03090427-add]//
     if(TRUE == PcdGetBool(PcdFDOState)){
//        SystemConfiguration.SecureBoot= FALSE;
		SystemConfiguration.FprrEnable= FALSE;
		SystemConfiguration.ScBiosLock = FALSE;
        DEBUG ((EFI_D_INFO, "PlatformInit-Disabling  SecureBoot,FprrEnable,ScBiosLock because of FDO \n\r"));
     	}
//[-end-160509-IB03090427-add]//
  //
  // Power button 500ms check
  //
  PwrBtn500msChk(PeiServices, &SystemConfiguration);
//[-start-151222-IB03090424-remove]//
//  }
//[-end-151222-IB03090424-remove]//
#if (TABLET_PF_ENABLE == 1)
  // Set WiGIG_en GPIO to low to disable WIGIG device first
  // Note: before toggle wigig_en GPIO, 32KHz clock need to be enabled
  //       according to PCIE power on seq requirement
  //       32KHz clock is controled by Pmic in PmicMiscInit, below is the config
  // {DEVICE2_BASE_ADDRESS, WKC_CLKCONFIG_REG,    REG_OVERRIDE, (WKC_MASK_SLP2DLVL | WKC_MASK_SLP2D), 0x00},

  if (BxtStepping() >= BxtB0) {
    PmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_GPIO0P1CTLO_REG, 0x30);
      // Dummp read the Wigig_en level to give more time for GPIO toggle
      PmicRead8 (DEVICE1_BASE_ADDRESS, WKC_GPIO0P1CTLO_REG);

      // Windows OS only
      if (SystemConfiguration.WiGigEnable == 1 && SystemConfiguration.OsSelection == 0) {

        // Set Wigig_en to high to enable Wigig device
        PmicWrite8 (DEVICE1_BASE_ADDRESS, WKC_GPIO0P1CTLO_REG, 0x31);
        DEBUG ((EFI_D_INFO, "Enabled WiGIG_EN \n\r"));
      }

    }
#endif
  //
  //Update the PlatformInfo Hob according to setup
  //
  PlatformInfo->OsSelection = SystemConfiguration.OsSelection;

  //
  //
  // Initialize all PEI Policy
  //
  PeiPolicyInit (PeiServices, &SystemConfiguration);

//[-start-161227-IB04530802-add]//
  //
  // Create Platform Hardware switch Hob
  //
  SetPlatformHardwareSwitch ();
//[-end-161227-IB04530802-add]//

  //
  // Do basic SC init
  //
  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, (EFI_COMPUTING_UNIT_CHIPSET | EFI_CHIPSET_PC_PEI_CAR_SB_INIT));
  Status = PlatformScInit (&SystemConfiguration, PeiServices, PlatformInfo->PlatformType);
  ASSERT_EFI_ERROR (Status);

//[-start-160901-IB03090433-remove]//
//  Comment for removing below code: TpmPolicyInit for set PcdTpmInstanceGuid has been done in FtpmSupportInit of PlatformStage2Init
//  if ((PLATFORM_ID == VALUE_REAL_PLATFORM)){
//    Status = TpmPolicyInit (PeiServices, &SystemConfiguration);
//    ASSERT_EFI_ERROR (Status);
//  }
//[-end-160901-IB03090433-remove]//

//[-start-160413-IB03090426-modify]//
  //
  // Initialize BxtPolicy PPI
  //
  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, (EFI_COMPUTING_UNIT_CHIPSET | EFI_CHIPSET_PC_PEI_CAR_NB_INIT));
  Status = BXTPolicyInit (PeiServices, &SystemConfiguration);
  ASSERT_EFI_ERROR (Status);
//[-end-160413-IB03090426-modify]//
  //
  // Initialize platform PPIs
  //
//[-start-160509-IB03090427-modify]//
  Status = PeiServicesInstallPpi (&mPpiList[0]);
//[-end-160509-IB03090427-modify]//
  ASSERT_EFI_ERROR (Status);

  //
  // Set LVDS_BKLT_CTRL to 50%.
  //
  MmPci8(0, 0, 2, 0, 0xF4) = 128;

  //
  // Initialize platform PPIs
  //
//[-start-160509-IB03090427-modify]//
  Status = PeiServicesNotifyPpi (&mNotifyList[0]);
//[-end-160509-IB03090427-modify]//
  ASSERT_EFI_ERROR (Status);

  // Maybe move to MemoryCallBack.c (which would be called between PreMemPlatInit and PlatInit)
  PlatformInitFinalConfig(PeiServices);
#if TABLET_PF_ENABLE
  if (PlatformInfo->PlatformFeatureValue.Vibrator == 1 && FeaturePcdGet(PcdVibratorFeature)) {
    Vibra_Buzz_Stop();
//[-start-160509-IB03090427-modify]//
    DEBUG((EFI_D_INFO, "Stop vibra %a(%d): %a()\n", __FILE__, __LINE__, __FUNCTION__));
//[-end-160509-IB03090427-modify]//
    }
#endif

  return Status;
}

EFI_STATUS
EFIAPI
CpuOnlyReset (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{

#ifdef __GNUC__
  __asm__
  (
   "xorl %ecx, %ecx\n"
   "1:hlt; hlt; hlt\n"
   "jmp 1b\n"
  );
#else
  _asm {
    xor   ecx, ecx
  HltLoop:
    hlt
    hlt
    hlt
    loop  HltLoop
  }
#endif

  //
  // If we get here we need to mark it as a failure.
  //
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
PeiGetSectionFromFv (
  IN CONST  EFI_GUID        NameGuid,
  OUT VOID                  **Address,
  OUT UINT32               *Size
  )
{
  EFI_STATUS  Status;
  EFI_PEI_FIRMWARE_VOLUME_PPI          *FvPpi;
  EFI_FV_FILE_INFO                     FvFileInfo;
  PEI_CORE_INSTANCE                    *PrivateData;
  UINTN                                CurrentFv;
  PEI_CORE_FV_HANDLE                   *CoreFvHandle;
  EFI_PEI_FILE_HANDLE                  VbtFileHandle;
  EFI_GUID                             *VbtGuid;
  EFI_COMMON_SECTION_HEADER            *Section;
  CONST EFI_PEI_SERVICES               **PeiServices;

  PeiServices = GetPeiServicesTablePointer ();

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);

  Status = PeiServicesLocatePpi (
            &gEfiFirmwareFileSystem2Guid,
            0,
            NULL,
            (VOID **) &FvPpi
            );
  ASSERT_EFI_ERROR (Status);

  CurrentFv = PrivateData->CurrentPeimFvCount;
  CoreFvHandle = &(PrivateData->Fv[CurrentFv]);

  Status = FvPpi->FindFileByName (FvPpi, &NameGuid, &CoreFvHandle->FvHandle, &VbtFileHandle);
  if (!EFI_ERROR(Status) && VbtFileHandle != NULL) {

  DEBUG ((DEBUG_INFO, "Find SectionByType \n"));

    Status = FvPpi->FindSectionByType (FvPpi, EFI_SECTION_RAW, VbtFileHandle, (VOID **) &VbtGuid);
    if (!EFI_ERROR (Status)) {

    DEBUG ((DEBUG_INFO, "GetFileInfo \n"));

      Status = FvPpi->GetFileInfo (FvPpi, VbtFileHandle, &FvFileInfo);
      Section = (EFI_COMMON_SECTION_HEADER *)FvFileInfo.Buffer;

      if (IS_SECTION2 (Section)) {
        ASSERT (SECTION2_SIZE (Section) > 0x00FFFFFF);
        *Size = SECTION2_SIZE (Section) - sizeof (EFI_COMMON_SECTION_HEADER2);
        *Address = ((UINT8 *)Section + sizeof (EFI_COMMON_SECTION_HEADER2));
      } else {
        *Size = SECTION_SIZE (Section) - sizeof (EFI_COMMON_SECTION_HEADER);
        *Address = ((UINT8 *)Section + sizeof (EFI_COMMON_SECTION_HEADER));
      }
    }
  }

  return EFI_SUCCESS;
}

#ifdef FSP_WRAPPER_FLAG
EFI_STATUS
CpuS3SmmAccessNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  //
  // Restore Cpu settings only during S3 resume
  //
  S3InitializeCpu (PeiServices);
  return EFI_SUCCESS;
}
#endif
/**
  Install Firmware Volume Hob's once there is main memory

  @param[in] PeiServices      General purpose services available to every PEIM.
  @param[in] NotifyDescriptor Not uesed.
  @param[in] Ppi              Not uesed.

  @retval    EFI_SUCCESS      If the interface could be successfully installed.
**/
//[-start-160517-IB03090427-modify]//
EFI_STATUS
EndOfPeiPpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  EFI_STATUS                  Status;
  UINT64                      MemoryTop;
  UINT64                      LowUncableBase;
  EFI_PLATFORM_INFO_HOB       *PlatformInfo;
  UINT32                      HecBaseHigh;
  EFI_BOOT_MODE               BootMode;
  EFI_PEI_HOB_POINTERS        Hob;
//[-start-160816-IB07400771-modify]//
//#if (BXTI_PF_ENABLE == 1)
  UINTN                           VariableSize;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
//#endif
//[-end-160816-IB07400771-modify]//

  Status = (*PeiServices)->GetBootMode((CONST EFI_PEI_SERVICES **) PeiServices, &BootMode);

  ASSERT_EFI_ERROR (Status);
//[-start-160816-IB07400771-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  ASSERT_EFI_ERROR (Status);
//[-end-160816-IB07400771-add]//

  //
  // Set the some PCI and chipset range as UC
  // And align to 1M at leaset
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

//[-start-160816-IB07400771-modify]//
//#if (BXTI_PF_ENABLE == 1)
//[-start-161022-IB07400803-modify]//
  if ((PlatformInfo->BoardId == BOARD_ID_OXH_CRB) || (PlatformInfo->BoardId == BOARD_ID_LFH_CRB) || (PlatformInfo->BoardId == BOARD_ID_JNH_CRB)) {
//[-end-161022-IB07400803-modify]//
    PlatformInfo->PmicVersion = ConfigurePlatformPmic ();
//[-start-161018-IB06740518-add]//
    ConfigurePmicIMON();
//[-end-161018-IB06740518-add]//
//[PRJ]+ >>>> Remove ScPlatformSccDllOverride() to fix eMMC boot fail.
//   ScPlatformSccDllOverride ();
//[PRJ]+ <<<< Remove ScPlatformSccDllOverride() to fix eMMC boot fail.
    VariableSize = PcdGet32 (PcdSetupConfigSize);
    //Status = GetSystemConfigData(&mSystemConfiguration, &VariableSize);
    Status = VariableServices->GetVariable (
                                 VariableServices,
                                 SETUP_VARIABLE_NAME,
                                 &gSystemConfigurationGuid,
                                 NULL,
                                 &VariableSize,
                                 &mSystemConfiguration
                                 );
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)){
//[-start-160802-IB03090430-modify]//
      if (mSystemConfiguration.AfeComp == 1){
//[-end-160802-IB03090430-modify]//
        ScUsb2PhyOverride(); //Override USB2_GLOBAL_PORT_2 register value
      }
    }
  }
//#endif
//[-end-160816-IB07400771-modify]//

  DEBUG ((EFI_D_INFO, "Memory TOLM: %X\n", PlatformInfo->MemData.MemTolm));
  DEBUG ((EFI_D_INFO, "PCIE OSBASE: %lX\n", PlatformInfo->PciData.PciExpressBase));
  DEBUG (
    (EFI_D_INFO,
    "PCIE   BASE: %lX     Size : %X\n",
    PlatformInfo->PciData.PciExpressBase,
    PlatformInfo->PciData.PciExpressSize)
    );
  DEBUG (
    (EFI_D_INFO,
    "PCI32  BASE: %X     Limit: %X\n",
    PlatformInfo->PciData.PciResourceMem32Base,
    PlatformInfo->PciData.PciResourceMem32Limit)
    );
  DEBUG (
    (EFI_D_INFO,
    "PCI64  BASE: %lX     Limit: %lX\n",
    PlatformInfo->PciData.PciResourceMem64Base,
    PlatformInfo->PciData.PciResourceMem64Limit)
    );
  DEBUG ((EFI_D_INFO, "UC    START: %lX     End  : %lX\n", PlatformInfo->MemData.MemMir0, PlatformInfo->MemData.MemMir1));

  LowUncableBase = PlatformInfo->MemData.MemMaxTolm;
  LowUncableBase &= (0x0FFF00000);
  MemoryTop = (0x100000000);

  if (BootMode != BOOT_ON_S3_RESUME) {
    //
    // In BIOS, HECBASE will be always below 4GB
    //
    HecBaseHigh = (UINT32) RShiftU64 (PlatformInfo->PciData.PciExpressBase, 28);
    ASSERT (HecBaseHigh < 16);

    //
    // Programe HECBASE for DXE phase
    //
    // PlatformInfo->PciData.PciExpressSize == 0x10000000
    //
  }

  return Status;
}
//[-end-160517-IB03090427-modify]//

//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
UINT8
ConfigurePlatformPmic (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_PEI_SMBUS2_PPI              *SmbusPpi;
  EFI_SMBUS_DEVICE_ADDRESS        SlaveAddress;
  UINTN                           Length;
  UINT8                           Data[4]={0};
  EFI_SMBUS_DEVICE_COMMAND        Command;
//[-start-161018-IB06740518-add]//
  PMIC_REG_SETTING                PmicInitTable[] = PMIC_SMBUS_REGISTER_SETTINGS;
  UINT8                           Index;
//[-end-161018-IB06740518-add]//

  DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic start\n"));

  Status = PeiServicesLocatePpi (
             &gEfiPeiSmbus2PpiGuid,
             0,
             NULL,
             (VOID **) &SmbusPpi
             );
  ASSERT_EFI_ERROR (Status);

  SlaveAddress.SmbusDeviceAddress = (0xBC >> 1);

//[-start-161018-IB06740518-modify]//
  for (Index = 0; Index < (sizeof (PmicInitTable))/(sizeof (PMIC_REG_SETTING)); Index++) {
    Command = PmicInitTable[Index].Command;
    Length  = 1;
    Data[0] = PmicInitTable[Index].Data;

    Status = SmbusPpi->Execute (
                        SmbusPpi,
                        SlaveAddress,
                        Command,
                        EfiSmbusWriteByte,
                        FALSE,
                        &Length,
                        &Data[0]
                        );

//[-start-160720-IB03090429-add]//
    if (EFI_ERROR (Status)) {
//[-start-190102-IB07401061-modify]//
#ifndef IOTG_SIC_RC_CODE_SUPPORTED
      DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusReadDataByte: Failed 0x%08x with Status :%d\n", (UINT8)Command, (UINT32)Status));
#else
      DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusWriteDataByte: Failed 0x%08x with Status :%d\n", (UINT8)Command, (UINT32)Status));
      return PMIC_A1;
#endif
//[-end-190102-IB07401061-modify]//
    }
  }
//[-end-160720-IB03090429-add]//

  //0x01 = A0:0, A1:01 PMIC version, PMIC A1 and above - Skip PMIC setting
  Command = 0x01;
  Length  = 1;
  Status = SmbusPpi->Execute (
                      SmbusPpi,
                      SlaveAddress,
                      Command,
                      EfiSmbusReadByte,
                      FALSE,
                      &Length,
                      &Data[0]
                      );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusReadDataByte: Failed 0x01 with Status :%d\n",(UINT32)Status));
  }

  if (Data[0] != PMIC_A0) {
//[-start-160720-IB03090429-modify]//
    DEBUG ((DEBUG_INFO, "PMIC Version: PMIC_A1, PMIC_F, PMIC_G or PMIC_H\n"));
    DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic: Skip PMIC setting for PMIC A1, PMIC_F, PMIC_G or PMIC_H\n"));
//[-end-160720-IB03090429-modify]//
    return PMIC_A1;
  } else {
    DEBUG ((DEBUG_INFO, "PMIC Version: PMIC_A0\n"));
  }
//[-end-161018-IB06740518-modify]//

  //0x50[0] = 1b (Activate NOSOIX mode)
  Command = 0x50;
  Length  = 1;
  Data[0] = 0x01;
  Status = SmbusPpi->Execute (
                      SmbusPpi,
                      SlaveAddress,
                      Command,
                      EfiSmbusReadByte,
                      FALSE,
                      &Length,
                      &Data[0]
                      );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusReadDataByte: Failed 0x50 with Status :%d\n",(UINT32)Status));
  }
  Data[0] |= BIT0;
  Status = SmbusPpi->Execute (
                      SmbusPpi,
                      SlaveAddress,
                      Command,
                      EfiSmbusWriteByte,
                      FALSE,
                      &Length,
                      &Data[0]
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusWriteDataByte: Failed 0x50 with Status :%d\n",(UINT32)Status));
  }

  //0x10[1] = 0b (deactivate DCD1 Exit operation)
  Command = 0x10;
  Length  = 1;
  Status = SmbusPpi->Execute (
                      SmbusPpi,
                      SlaveAddress,
                      Command,
                      EfiSmbusReadByte,
                      FALSE,
                      &Length,
                      &Data[0]
                      );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusReadDataByte: Failed 0x10 with Status :%d\n",(UINT32)Status));
  }
  Data[0] &= (UINT8)(~BIT1);
  Status = SmbusPpi->Execute (
                      SmbusPpi,
                      SlaveAddress,
                      Command,
                      EfiSmbusWriteByte,
                      FALSE,
                      &Length,
                      &Data[0]
                      );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic - SmBusWriteDataByte: Failed 0x10 with Status :%d\n",(UINT32)Status));
  }

  DEBUG ((DEBUG_INFO, "PEI ConfigurePlatformPmic end\n"));
  return PMIC_A0;
}

//[-start-161018-IB06740518-add]//
//
// Doc#570618, Rev 1.4, Section 3.3, Sample Code to Fix Vcc and Vnn 
//
VOID
ConfigurePmicIMON (
  VOID
  )
{
  UINTN   PciD0F0RegBase = 0;
  UINTN   MchBar = 0;
  UINT32  Data;
  UINT16  StallCount;
  UINT64  PkgPwrSKU;

  DEBUG((EFI_D_INFO, "ConfigurePmicIMON() - Start\n"));

  PciD0F0RegBase  = MmPciAddress (0,0,0,0,0);
  MchBar          = MmioRead32 (PciD0F0RegBase + 0x48) &~BIT0;
  PkgPwrSKU       = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU);

  //def fix_vnn():
  //slope = 0x4a4    # 2.32
  //offset = 0xfa0d  # -2.975
  //soc.punit.p_cr_bios_mailbox_data = slope + (offset << 16)
  //soc.punit.p_cr_bios_mailbox_interface = 0x8000011d
  StallCount = 0;
  while (StallCount < 1000) {
    Data = MmioRead32 (MchBar + 0x7084); //read P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR
    if ((Data & BIT31) == BIT31) {  //check RUN_BUSY bit
	  MicroSecondDelay (1);         //add 1 microsecond delay
    }
    else {
	  break;
    }
	StallCount++;
  }
//[-start-161021-IB03090435-modify]//
  MmioWrite32 ( (MchBar + 0x7080), 0xfa0d04a4);  //P_CR_BIOS_MAILBOX_DATA_0_0_0_MCHBAR
//[-end-161021-IB03090435-modify]//
  MmioWrite32 ( (MchBar + 0x7084), 0x8000011d);  //P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR

  //def fix_vcc():
  //slope = 0x466    # 2.2 (Premium and High SKU), 0x3b3    # 1.85 (Low and Intermediate SKU)
  //offset = 0xe833  # -11.9 (Premium and High SKU), 0xed33  # -9.4 (Low and Intermediate SKU)
  //soc.punit.p_cr_bios_mailbox_data = slope + (offset << 16)
  //soc.punit.p_cr_bios_mailbox_interface = 0x8000001d
  StallCount = 0;
  while (StallCount < 1000) {
    Data = MmioRead32 (MchBar + 0x7084); //read P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR
    if ((Data & BIT31) == BIT31) {  //check RUN_BUSY bit
	  MicroSecondDelay (1);         //add 1 microsecond delay
    }
    else {
	  break;
    }
	StallCount++;
  }
  if ((PkgPwrSKU & 0x07FFF) >= 0x0903){  //Check PKG_TDP(14:0) bit to determine Premium or High SKU
    MmioWrite32 ( (MchBar + 0x7080), 0xe8330466);  //P_CR_BIOS_MAILBOX_DATA_0_0_0_MCHBAR
    MmioWrite32 ( (MchBar + 0x7084), 0x8000001d);  //P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR
  } else { //Low or Intermediate SKU
    MmioWrite32 ( (MchBar + 0x7080), 0xed3303b3);  //P_CR_BIOS_MAILBOX_DATA_0_0_0_MCHBAR
    MmioWrite32 ( (MchBar + 0x7084), 0x8000001d);  //P_CR_BIOS_MAILBOX_INTERFACE_0_0_0_MCHBAR
  }

  DEBUG((EFI_D_INFO, "ConfigurePmicIMON() - End\n"));
}
//[-end-161018-IB06740518-add]//

//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//


EFI_STATUS
EFIAPI
ConfigClockTrunk (
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS                      Status;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
//[-end-160803-IB07220122-add]//
  CHIPSET_CONFIGURATION           SystemConfiguration;
  UINTN                           VariableSize;

  DEBUG ((EFI_D_INFO, "\n ConfigClockTrunk\n"));

//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&mSystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//

  if (EFI_ERROR (Status) ) {
    DEBUG ((EFI_D_INFO, "\nWarning: Get setup variable status =%x\n",Status));
    return Status;
  }

  DEBUG ((EFI_D_INFO, "\n    TrunkClockEnable=%x\n",SystemConfiguration.TrunkClockEnable ));
  if (SystemConfiguration.TrunkClockEnable == 1) {
    SideBandAndThenOr32(SB_PORTID_PSF0, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFF, BIT4);
    SideBandAndThenOr32(SB_PORTID_PSF1, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFF, BIT4);
    SideBandAndThenOr32(SB_PORTID_PSF2, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFF, BIT4);
    SideBandAndThenOr32(SB_PORTID_PSF3, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFF, BIT4);
    SideBandAndThenOr32(SB_PORTID_PSF4, R_PCH_PCR_PSF_GLOBAL_CONFIG, 0xFFFFFFFF, BIT4);
  }
  return Status;
}
//[-end-151204-IB02950555-modify]//

/**
  Install Firmware Volume Hob's once there is main memory

  @param[in] PeiServices      General purpose services available to every PEIM.
  @param[in] NotifyDescriptor Notify that this module published.
  @param[in] Ppi              PPI that was installed.

  @retval    EFI_SUCCESS      The function completed successfully.
**/
EFI_STATUS
EFIAPI
PlatformInitFinalConfig (
  IN CONST EFI_PEI_SERVICES           **PeiServices
  )
{
  EFI_STATUS                  Status;
  EFI_BOOT_MODE               BootMode;
  EFI_CPUID_REGISTER          FeatureInfo;
  UINT8                       CpuAddressWidth;
  UINT16                      Pm1Cnt;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_PLATFORM_INFO_HOB       *PlatformInfo;
  UINT16                      AcpiBaseAddr;

  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  Status = (*PeiServices)->GetBootMode ((CONST EFI_PEI_SERVICES **) PeiServices, &BootMode);

  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Check if user wants to turn off in PEI phase
  //
  if ((BootMode != BOOT_ON_S3_RESUME) && (BootMode != BOOT_ON_FLASH_UPDATE)) {
    CheckPowerOffNow();
  } else {
    Pm1Cnt  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_CNT);
    Pm1Cnt &= ~B_ACPI_PM1_CNT_SLP_TYP;
    IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);
  }

  //
  //  Pulish memory tyoe info
  //

  //
  // Work done if on a S3 resume
  //
  if (BootMode == BOOT_ON_S3_RESUME) {
    return EFI_SUCCESS;
  }

  //
  // Add HOP entries for reserved MMIO ranges so that DXE will know about them.
  // Note: this really only need to be done for addresses that are outside the upper 16MB.
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PMC_BASE_ADDRESS,
    0x1000
    );
  DEBUG ((EFI_D_INFO, "PmcBase            : 0x%x\n", PMC_BASE_ADDRESS));

  //Spi BAR needs to be set to SPI_BASE_ADDRESS before it can be read..
  //SpiBase = MmPci32( 0, DEFAULT_PCI_BUS_NUMBER_SC, PCI_DEVICE_NUMBER_SPI, PCI_FUNCTION_NUMBER_SPI, R_SPI_BASE ) & B_SPI_BASE_BAR;
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    SPI_BASE_ADDRESS,
    0x1000
    );
  DEBUG ((EFI_D_INFO, "SpiBase            : 0x%x\n", SPI_BASE_ADDRESS));

  // Local APIC
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    LOCAL_APIC_BASE_ADDRESS,
    0x1000
  );
  DEBUG ((EFI_D_INFO, "LOCAL_APIC_BASE_ADDRESS : 0x%x\n", LOCAL_APIC_BASE_ADDRESS));

  // IO APIC
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    IO_APIC_BASE_ADDRESS,
    0x1000
  );
  DEBUG ((EFI_D_INFO, "IO_APIC_ADDRESS    : 0x%x\n", IO_APIC_BASE_ADDRESS));

  // Adding the PCIE Express area to the memory table as type 2 memory.
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PlatformInfo->PciData.PciExpressBase,
    PlatformInfo->PciData.PciExpressSize
    );
  DEBUG ((EFI_D_INFO, "PciExpressBase     : 0x%x\n", PlatformInfo->PciData.PciExpressBase));

//[-start-151216-IB07220026-remove]//
//   // Adding the Flashpart to the memory table as type 2 memory.
//   BuildResourceDescriptorHob (
//     EFI_RESOURCE_FIRMWARE_DEVICE,
//     (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
//     PcdGet32(PcdFlashAreaBaseAddress),
//     PcdGet32(PcdFlashAreaSize)
//     );
//   DEBUG ((EFI_D_INFO, "FLASH_BASE_ADDRESS : 0x%x\n", PcdGet32(PcdFlashAreaBaseAddress)));
//[-end-151215-IB07220026-remove]//

  // P2SB (BXT) 16MB
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdP2SBBaseAddress),
    0x1000000
  );

  // PMC IPC (BXT) 8KB and 4KB
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcIpc1BaseAddress0),
    0x2000
  );

    BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcIpc1BaseAddress1),
    0x1000
  );

  //PMC SSRAM (BXT) 8KB and 4KB
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcSsramBaseAddress0),
    0x2000
    );
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PcdGet32(PcdPmcSsramBaseAddress1),
    0x1000
    );

  //
  // Create a CPU hand-off information
  //
  CpuAddressWidth = 32;
  AsmCpuid (EFI_CPUID_EXTENDED_FUNCTION, &FeatureInfo.RegEax, &FeatureInfo.RegEbx, &FeatureInfo.RegEcx, &FeatureInfo.RegEdx);
  if (FeatureInfo.RegEax >= EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE) {
    AsmCpuid (EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE, &FeatureInfo.RegEax, &FeatureInfo.RegEbx, &FeatureInfo.RegEcx, &FeatureInfo.RegEdx);
    CpuAddressWidth = (UINT8) (FeatureInfo.RegEax & 0xFF);
  }

//[-start-151204-IB02950555-modify]//
  ConfigClockTrunk(PeiServices);
//[-end-151204-IB02950555-modify]//  

  BuildCpuHob(CpuAddressWidth, 16);
  ASSERT_EFI_ERROR (Status);

  return Status;
}


EFI_STATUS
ValidateFvHeader (
  IN EFI_FIRMWARE_VOLUME_HEADER            *FwVolHeader
  )
{
  UINT16  *Ptr;
  UINT16  HeaderLength;
  UINT16  Checksum;

  //
  // Verify the header revision, header signature, length
  // Length of FvBlock cannot be 2**64-1
  // HeaderLength cannot be an odd number
  //
  if ((FwVolHeader->Revision != EFI_FVH_REVISION) ||
      (FwVolHeader->Signature != EFI_FVH_SIGNATURE) ||
      (FwVolHeader->FvLength == ((UINT64) -1)) ||
      ((FwVolHeader->HeaderLength & 0x01) != 0)
      ) {
    return EFI_NOT_FOUND;
  }
  //
  // Verify the header checksum
  //
  HeaderLength  = (UINT16) (FwVolHeader->HeaderLength / 2);
  Ptr           = (UINT16 *) FwVolHeader;
  Checksum      = 0;
  while (HeaderLength > 0) {
    Checksum = *Ptr++;
    HeaderLength--;
  }

  if (Checksum != 0) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
//[-end-160317-IB03090425-modify]//




#ifdef __GNUC__
#pragma GCC pop_options
#else
#pragma optimize ("", on)
#endif
