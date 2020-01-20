/** @file
  Platform Configuration Update library implementation file.
  
  This library updates the setup data with platform overrides.

@copyright
  Copyright (c) 2016 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.

@par Specification Reference:
**/

#include <Guid/PlatformInfo.h>
#include <Guid/TpmInstance.h>

#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiPlatformConfigUpdateLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PreSiliconLib.h>
#include <Library/SteppingLib.h>

#include <Ppi/DramPolicyPpi.h>

extern EFI_GUID gFdoModeEnabledHobGuid;

#define SETUP_NFC_Disabled 0
#define SETUP_NFC_IPT      1
#define SETUP_NFC          2

EFI_STATUS
TpmSetupPolicyInit (
  IN CHIPSET_CONFIGURATION    *SystemConfiguration
  )
{
#if FTPM_SUPPORT
  EFI_STATUS        Status;
  BOOLEAN           PttEnabledState = FALSE;
  EFI_HOB_GUID_TYPE *FdoEnabledGuidHob = NULL;

  FdoEnabledGuidHob = GetFirstGuidHob (&gFdoModeEnabledHobGuid);

  if (SystemConfiguration->TpmDetection == 0) {
    Status = PttHeciGetState (&PttEnabledState);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Get PTT enabled state failed.\n"));
    }

    if (PttEnabledState && (FdoEnabledGuidHob == NULL)) {
      SystemConfiguration->TPM = TPM_PTT;
    } else {
      DEBUG ((EFI_D_INFO, "TpmPolicyInit-TPM and TpmDetection is disabled because of FDO \n\r"));
      SystemConfiguration->TPM = TPM_DISABLE;
    }
    SystemConfiguration->TpmDetection = 1;
  }

  if ((SystemConfiguration->OsSelection == 1) && (SystemConfiguration->TPM == TPM_PTT) && (SystemConfiguration->PttSuppressCommandSend == 0)) {  // AOS
    SystemConfiguration->TPM = TPM_DISABLE;

    Status = PttHeciGetState (&PttEnabledState);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "AOS: Get PTT enabled state failed.\n"));
    }

    if (PttEnabledState) {
      PttHeciSetState(FALSE);
    }

  }
#endif
  return EFI_SUCCESS;
}

/*

CSE XML Stap    0    0    0    0    1    1    1    1
BIOS Setup      0    0    1    1    0    0    1    1
Board Fab B/C   0    1    0    1    0    1    0    1
Finally Setting N    N    E    D    D    D    N    D

RVP Board Fab B/C does not support secure NFC as HW limitation.
N: Do nothing
E: Enable secure NFC
D: Disable secure NFC

Summary:
  Enable Secure NFC : CSE XML Strap 0 && BIOS Setup 1 && Board Fab B/C 0
  Disable Secure NFC: BIOS Setup 1 && Board Fab B/C 1
                      CSE XML Strap 1 && BIOS Setup 0

*/
EFI_STATUS
GetSecureNfcInfo (
  CHIPSET_CONFIGURATION  *SystemConfiguration
  )
{
  EFI_STATUS                      Status;
  EFI_PEI_HOB_POINTERS            GuidHob;
  EFI_PLATFORM_INFO_HOB           *PlatformInfo = NULL;
  MEFWCAPS_SKU                    CurrentFeatures;
  UINT32                          EnableBitmap = 0;
  UINT32                          DisableBitmap = 0;
  BOOLEAN                         Cse_Nfc_Strap = FALSE;
  BOOLEAN                         SecureNfcInSetup = FALSE;
  BOOLEAN                         Fab_B_C = FALSE;

  DEBUG ((EFI_D_INFO, "GetSecureNfcInfo ++ \n"));

  if (SystemConfiguration->NfcSelect == SETUP_NFC_IPT) {
    SecureNfcInSetup = TRUE;
  }

  DEBUG ((EFI_D_INFO, "Old NfcSelect: %d\n", SystemConfiguration->NfcSelect));

  Status = HeciGetFwFeatureStateMsgII (&CurrentFeatures);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  if (CurrentFeatures.Fields.NFC == 1) {
    Cse_Nfc_Strap = TRUE;
  }

  DEBUG ((EFI_D_INFO, "HeciGetFwFeatureStateMsgII CurrentFeatures.Fields.NFC: %d\n", CurrentFeatures.Fields.NFC));

  //
  // Get the HOB list.  If it is not present, then ASSERT.
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
      PlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  if (PlatformInfo == NULL) {
    ASSERT (PlatformInfo != NULL);
    return EFI_NOT_FOUND;
  }

  if ((PlatformInfo->BoardId == BOARD_ID_BXT_RVP) && (PlatformInfo->BoardRev == FAB_ID_RVP_B_C)) {
    DEBUG ((EFI_D_INFO, "Secure NFC should be disabled for BXT RVP Fab B/C.\n"));
    Fab_B_C = TRUE;
  }

  if ((!Cse_Nfc_Strap) && SecureNfcInSetup && (!Fab_B_C)){
    //
    // Enable secure NFC
    //
    DEBUG ((EFI_D_INFO, "Enable NFC\n"));
    EnableBitmap  = NFC_BITMASK;
    DisableBitmap = CLEAR_FEATURE_BIT;
  } else if ( (SecureNfcInSetup && Fab_B_C)     ||\
              (Cse_Nfc_Strap    && (!SecureNfcInSetup)) \
            ) {
    //
    // Disable secure NFC
    //
    DEBUG ((EFI_D_INFO, "Disable NFC\n"));
    EnableBitmap  = CLEAR_FEATURE_BIT;
    DisableBitmap = NFC_BITMASK;
    SystemConfiguration->NfcSelect = SETUP_NFC;
  }

  Status = HeciFwFeatureStateOverride (EnableBitmap, DisableBitmap);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "NfcSelect: %d\n", SystemConfiguration->NfcSelect));

  return Status;
}

/**
  Updates Setup values from PlatformInfoHob and platform policies.

  @param  PreDefaultSetupData   A pointer to the setup data prior to being
                                placed in the default data HOB.

  @retval EFI_SUCCESS           The Setup data was updated successfully.
**/
EFI_STATUS
UpdateSetupDataValues (
  CHIPSET_CONFIGURATION     *PreDefaultSetupData
  )
{
  EFI_STATUS               Status;
  
  #if TABLET_PF_ENABLE
  EFI_PEI_HOB_POINTERS     PlatformInfoHobPtrs;
  EFI_PLATFORM_INFO_HOB    *PlatformInfoHob;
  #endif
  EFI_HOB_GUID_TYPE        *FdoEnabledGuidHob = NULL;
  
  DRAM_POLICY_PPI          *DramPolicyPpi;

  Status = PeiServicesLocatePpi (
                             &gDramPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **)&DramPolicyPpi
                             );
  
  FdoEnabledGuidHob = GetFirstGuidHob (&gFdoModeEnabledHobGuid);
  
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Couldn't find DRAM policy PPI: %g, Status: %r.\n", &gDramPolicyPpiGuid, Status));
  } else {
    DEBUG ((EFI_D_INFO, "Overriding Memory System Config Data using DRAM Policy.\n"));
    PreDefaultSetupData->ChannelHashMask         = DramPolicyPpi->ChannelHashMask;
    PreDefaultSetupData->SliceHashMask           = DramPolicyPpi->SliceHashMask;
    PreDefaultSetupData->ChannelsSlicesEnabled   = DramPolicyPpi->ChannelsSlicesEnabled;
    PreDefaultSetupData->ScramblerSupport        = DramPolicyPpi->ScramblerSupport;
    PreDefaultSetupData->InterleavedMode         = DramPolicyPpi->InterleavedMode;
    PreDefaultSetupData->MinRefRate2xEnabled     = DramPolicyPpi->MinRefRate2xEnabled;
    PreDefaultSetupData->DualRankSupportEnabled  = DramPolicyPpi->DualRankSupportEnabled;
  }

  #if TABLET_PF_ENABLE
  //
  // Get Platform Info HOB
  //
  PlatformInfoHobPtrs.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (PlatformInfoHobPtrs.Raw != NULL);
  PlatformInfoHob = GET_GUID_HOB_DATA (PlatformInfoHobPtrs.Raw);

  DEBUG ((EFI_D_INFO, " BomSelection is %x \n ", PreDefaultSetupData->BomSelection));
  DEBUG ((EFI_D_INFO, " FeatureSelection is %x \n ", PreDefaultSetupData->FeatureSelection));
  DEBUG ((EFI_D_INFO, " BomIdPss is %x \n ", PlatformInfoHob->BomIdPss));
  DEBUG ((EFI_D_INFO, " OsSelPss is %x \n ", PlatformInfoHob->OsSelPss));
  
  // PSS_OSID changed or user changed OS selection in setup menu
  if ((PreDefaultSetupData->FeatureSelection != PlatformInfoHob->OsSelPss) || (PreDefaultSetupData->PreOsSelection != PreDefaultSetupData->OsSelection)) {
    PreDefaultSetupData->FeatureSelection = PlatformInfoHob->OsSelPss;
    PreDefaultSetupData->OsSelection      = PlatformInfoHob->PlatformFeatureValue.OsSelection;
    PreDefaultSetupData->PreOsSelection   = PreDefaultSetupData->OsSelection;
    PreDefaultSetupData->Ssic1Support     = PlatformInfoHob->PlatformFeatureValue.Ssic1Support;
    PreDefaultSetupData->ScUsbOtg         = PlatformInfoHob->PlatformFeatureValue.ScUsbOtg;
  }

  if (PreDefaultSetupData->BomSelection != BOM_MANUALLY) {
    PreDefaultSetupData->PanelSel       = PlatformInfoHob->PlatformBOMValue.PanelSel;
    PreDefaultSetupData->WorldCameraSel = PlatformInfoHob->PlatformBOMValue.WorldCameraSel;
    PreDefaultSetupData->UserCameraSel  = PlatformInfoHob->PlatformBOMValue.UserCameraSel;
    PreDefaultSetupData->AudioSel       = PlatformInfoHob->PlatformBOMValue.AudioSel;
    PreDefaultSetupData->ModemSel       = PlatformInfoHob->PlatformBOMValue.ModemSel;
    PreDefaultSetupData->TouchSel       = PlatformInfoHob->PlatformBOMValue.TouchSel;
    PreDefaultSetupData->WifiSel        = PlatformInfoHob->PlatformBOMValue.WifiSel;

    // For PR0.5, change WIFI default to SDIO
    if (PlatformInfoHob->BoardId == BOARD_ID_BXT_FFD && PlatformInfoHob->BoardRev == FAB_ID_C) {
      PreDefaultSetupData->WifiSel = 0;
    }

    // For AOS, Command Mode VBT will be set to default for Truly Panel Fab-B, 
    // Currently Video Mode VBT is set to default in SMIP, as Windows now using Video Mode. 
    // These code can be removed if Command Mode VBT is set to default in SMIP
    if (PreDefaultSetupData->OsSelection == 1 && PreDefaultSetupData->PanelSel == 3) {
      PreDefaultSetupData->PanelSel = 4;
    }

  }
  #endif
  
  if (FdoEnabledGuidHob != NULL) {
//[-start-160624-IB03090428-remove]//
//    PreDefaultSetupData->SecureBoot = FALSE;
//[-end-160624-IB03090428-remove]//
    PreDefaultSetupData->FprrEnable = FALSE;
    PreDefaultSetupData->ScBiosLock = FALSE;
    DEBUG ((EFI_D_INFO, "SPI FDO mode is enabled. Disabling SecureBoot, FprrEnable, and ScBiosLock.\n"));
  }
      
  if ((PLATFORM_ID == VALUE_REAL_PLATFORM)) {
    Status = GetSecureNfcInfo (PreDefaultSetupData);
    ASSERT_EFI_ERROR (Status);
  }
  
  if ((PLATFORM_ID == VALUE_REAL_PLATFORM)){
	  Status = TpmSetupPolicyInit(PreDefaultSetupData);
    ASSERT_EFI_ERROR (Status);
  }
  
  return Status;
}