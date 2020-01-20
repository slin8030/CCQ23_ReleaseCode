/** @file

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
  SEC policy initialization.

@copyright
  Copyright (c) 2008 - 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains an 'Intel Peripheral Driver' and is uniquely
  identified as "Intel Reference Module" and is licensed for Intel
  CPUs and chipsets under the terms of your license agreement with
  Intel or your vendor. This file may be modified by the user, subject
  to additional terms of the license agreement.

@par Specification
**/

#include "SeCPolicyInitDxe.h"
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Guid/PlatformInfo.h>
#include <Guid/HobList.h>
#include <Library/HeciMsgLib.h>
#include <Library/PreSiliconLib.h>
#include <ChipsetSetupConfig.h>
//[-start-160904-IB07400778-add]//
//[-start-161123-IB07250310-add]//
//#ifdef FSP_WRAPPER_FLAG
#include <Library/HobLib.h>
//#endif
//[-end-161123-IB07250310-add]//
//[-end-160904-IB07400778-add]//

//
// TS DIMM thermal polling Smbus Address.
// This is platform specific.
//
UINT8 mTsDimmSmbusAddress[] = {0x30, 0x34};

DXE_SEC_POLICY_PROTOCOL  mDxePlatformSeCPolicy = { 0 };

SEC_OPERATION_PROTOCOL   mSeCOperationProtocol = {
  GetPlatformSeCInfo,
  SetPlatformSeCInfo,
  PerformSeCOperation
};

UINT8 HeciHmrfpoLockResult;
EFI_PLATFORM_INFO_HOB           *mPlatformInfo = NULL;
//[-start-160803-IB07400768-remove]//
//CHIPSET_CONFIGURATION            mSystemConfiguration;
//[-end-160803-IB07400768-remove]//

//
// Function implementations
//

EFI_STATUS
EFIAPI
GetSecureNfcInfo (
  VOID
);

EFI_STATUS
EFIAPI
SeCPlatformPolicyEntryPoint (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable
)
/*++

Routine Description:

  Entry point for the SeC Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
{
  EFI_STATUS                      Status;
  EFI_EVENT                       ReadyToBootEvent;
//[-start-160904-IB07400778-add]//
  EFI_BOOT_MODE                   BootMode;
//[-end-160904-IB07400778-add]//

  DEBUG ((EFI_D_INFO, "SeCPlatformPolicyEntryPoint ++ \n"));

//[-start-160904-IB07400778-add]//
  BootMode = GetBootModeHob ();
  //
  // In BOOT_IN_RECOVERY_MODE, TXE is not exist.
  //
  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return EFI_SUCCESS;
  }
//[-end-160904-IB07400778-add]//

  mDxePlatformSeCPolicy.SeCConfig.TrConfig = AllocateZeroPool (sizeof (TR_CONFIG));
  if (mDxePlatformSeCPolicy.SeCConfig.TrConfig == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // if CSE non-exists, skip it.
  //
  if (HeciPciRead16(R_SEC_DevID_VID) == 0xFFFF) {
    DEBUG ((EFI_D_INFO, "non-secure boot, skip secure init.\n"));
    return EFI_SUCCESS;
  }

  //
  // ME DXE Policy Init
  //
  mDxePlatformSeCPolicy.Revision = DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_7;

  //
  // Initialzie the Me Configuration
  //
  mDxePlatformSeCPolicy.SeCConfig.EndOfPostEnabled        = 1;
  mDxePlatformSeCPolicy.SeCConfig.HeciCommunication        = 1;
  mDxePlatformSeCPolicy.SeCConfig.SeCFwDownGrade           = 0;
  mDxePlatformSeCPolicy.SeCConfig.SeCLocalFwUpdEnabled     = 0;
  mDxePlatformSeCPolicy.SeCConfig.TrConfig->DimmNumber    = 2;
  mDxePlatformSeCPolicy.SeCConfig.TrConfig->TrEnabled = 0;
  mDxePlatformSeCPolicy.SeCConfig.SeCFwImageType = INTEL_SEC_1_5MB_FW;
  mDxePlatformSeCPolicy.SeCConfig.PlatformBrand = INTEL_STAND_MANAGEABILITY_BRAND;
  mDxePlatformSeCPolicy.SeCReportError = ShowSeCReportError;
	mDxePlatformSeCPolicy.SeCConfig.ITouchEnabled=FALSE;
  //
  // Install the EFI_MANAGEMENT_ENGINE_PROTOCOL interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gDxePlatformSeCPolicyGuid,
                  &mDxePlatformSeCPolicy,
                  &gEfiSeCOperationProtocolGuid,
                  &mSeCOperationProtocol,
                  NULL
                  );

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             SeCPolicyReadyToBootEvent,
             (VOID *) &ImageHandle,
             &ReadyToBootEvent
             );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "SeCPlatformPolicyEntryPoint -- \n"));
  return Status;
}

VOID
EFIAPI
ShowSeCReportError (
  IN SEC_ERROR_MSG_ID            MsgId
  )
/*++

Routine Description:

  Show Me Error message.

Arguments:

  MsgId   Me error message ID.

Returns:

  None.

--*/
{
  gST->ConOut->ClearScreen (gST->ConOut);

  switch (MsgId) {
  case MSG_EOP_ERROR:
    gST->ConOut->OutputString (gST->ConOut, L"Error sending End Of Post message to ME, System HALT!\n");
    break;

  case MSG_SEC_FW_UPDATE_FAILED:
    gST->ConOut->OutputString (gST->ConOut, L"ME FW Update Failed, please try again!\n");
    break;

  case MSG_ASF_BOOT_DISK_MISSING:
    gST->ConOut->OutputString (gST->ConOut, L"Boot disk missing, please insert boot disk and press ENTER\r\n");
    break;

  case MSG_KVM_TIMES_UP:
    gST->ConOut->OutputString (gST->ConOut, L"Error!! Times up and the KVM session was cancelled!!");
    break;

  case MSG_KVM_REJECTED:
    gST->ConOut->OutputString (gST->ConOut, L"Error!! The request has rejected and the KVM session was cancelled!!");
    break;

  case MSG_HMRFPO_LOCK_FAILURE:
    gST->ConOut->OutputString (gST->ConOut, L"(A7) Me FW Downgrade - Request MeSpiLock Failed\n");
    break;

  case MSG_HMRFPO_UNLOCK_FAILURE:
    gST->ConOut->OutputString (gST->ConOut, L"(A7) Me FW Downgrade - Request MeSpiEnable Failed\n");
    break;

  case MSG_SEC_FW_UPDATE_WAIT:
    gST->ConOut->OutputString (
                   gST->ConOut,
                   L"Intel(R) Firmware Update is in progress. It may take up to 90 seconds. Please wait.\n"
                   );
    break;

  case MSG_ILLEGAL_CPU_PLUGGED_IN:
    gST->ConOut->OutputString (
                   gST->ConOut,
                   L"\n\n\rAn unsupported CPU/SC configuration has been identified.\n"
                   );
    gST->ConOut->OutputString (
                   gST->ConOut,
                   L"\rPlease refer to the Huron River Platform Validation Matrix\n\rfor supported CPU/SC combinations."
                   );
    break;

  case MSG_KVM_WAIT:
    gST->ConOut->OutputString (gST->ConOut, L"Waiting Up to 8 Minutes For KVM FW.....");
    break;

  default:
    DEBUG ((EFI_D_ERROR, "This Message Id hasn't been defined yet, MsgId = %x\n", MsgId));
    break;
  }

  gBS->Stall (HECI_MSG_DELAY);

}

VOID
HmrfpoEnable(
  VOID
  )
{
  EFI_STATUS  Status;
  UINT8       HeciHmrfpoEnableResult;

  HeciHmrfpoEnableResult  = HMRFPO_ENABLE_UNKNOWN_FAILURE;
  Status                  = HeciHmrfpoEnable (0, &HeciHmrfpoEnableResult);
  if (Status == EFI_SUCCESS && HeciHmrfpoEnableResult == HMRFPO_ENABLE_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "SeC FW Downgrade !!- Step A6\n"));

    HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
    CpuDeadLoop ();
  }
  DEBUG ((EFI_D_INFO, "SeC FW Downgrade Error !!- Step A8, the Status is %r, The result is %x\n", Status, HeciHmrfpoEnableResult));

}

UINT32
GetSeCOpMode()
{
  HECI_FWS_REGISTER                   SeCFirmwareStatus;

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  DEBUG ((EFI_D_INFO, "R_SEC_FW_STS0 is %08x %x\n", SeCFirmwareStatus.ul, SeCFirmwareStatus.r.SeCOperationMode));
  return SeCFirmwareStatus.r.SeCOperationMode;
}

UINT32
GetSeCHMRFPOStatus()
{
  UINT8         HMRFPOStatus;

  HeciHmrfpoGetStatus(&HMRFPOStatus);
  if (HMRFPOStatus == 0) {
    return 0;
  } else if (HMRFPOStatus == 1){
    return 0;
  }
  return 1;
}

UINT32
GetSeCFwUpdateStatus()
{
  UINT32    Result;

  HeciGetLocalFwUpdate(&Result);
  return Result;
}

EFI_STATUS
GetSeCFwVersion (
  SEC_VERSION_INFO *SeCVersion
  )
{
  EFI_STATUS            Status;
  GEN_GET_FW_VER_ACK    MsgGenGetFwVersionAckData;

//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    //
    // GEN_GET_FW_VERSION_CMD command is in the CSE main region,
    // LBP1 secondary was broken when fault tolerance boot,
    // so skip this command for avoid 30 second timeout.
    //
    return EFI_UNSUPPORTED;
  }
#endif
//[-end-161123-IB07250310-add]//

  Status = HeciGetFwVersionMsg(&MsgGenGetFwVersionAckData);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  SeCVersion->CodeMajor = MsgGenGetFwVersionAckData.Data.CodeMajor;
  SeCVersion->CodeMinor = MsgGenGetFwVersionAckData.Data.CodeMinor;
  SeCVersion->CodeHotFix = MsgGenGetFwVersionAckData.Data.CodeHotFix;
  SeCVersion->CodeBuildNo = MsgGenGetFwVersionAckData.Data.CodeBuildNo;
  return EFI_SUCCESS;
}

EFI_STATUS
GetSeCCapability (
  UINT32      *SeCCapability
  )
{
  EFI_STATUS               Status;
  GEN_GET_FW_CAPSKU        MsgGenGetFwCapsSku;
  GEN_GET_FW_CAPS_SKU_ACK  MsgGenGetFwCapsSkuAck;

  Status = HeciGetFwCapsSkuMsg (&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  *SeCCapability = MsgGenGetFwCapsSkuAck.Data.FWCapSku.Data;
  return EFI_SUCCESS;
}

EFI_STATUS
GetSeCFeature (
  UINT32      *SeCFeature
  )
{
  EFI_STATUS       Status;
  SECFWCAPS_SKU    RuleData;

  Status = HeciGetFwFeatureStateMsg(&RuleData);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  *SeCFeature = RuleData.Data;
  return EFI_SUCCESS;
}

UINT32
CheckSeCExist()
{
  //
  // TODO: Need to enable after installing Protocol in Hecidrv.c file
  //
//[-start-160415-IB11270153-modify]//
//   return 0;
//#if 0
  UINT32 DeviceID;
  DeviceID = MmioRead32 (MmPciAddress (0x00, 0x00, 0x0F, 0x00, 0x00));
  DEBUG ((EFI_D_INFO, "DeviceID %08x\n", DeviceID));
  DeviceID = (DeviceID & 0xFFFF0000) >> 16;
  if(!(DeviceID >= S_SEC_DevID_RANGE_LO && DeviceID <= S_SEC_DevID_RANGE_HI))
  {
    return 0;
  }
  //
  // Check for SEC MemValid status
  //
  if ((HeciPciRead32 (R_SEC_MEM_REQ) & B_SEC_MEM_REQ_INVALID) == B_SEC_MEM_REQ_INVALID) {
    //
    // SEC failed to start so no HECI
    //
    DEBUG ((EFI_D_ERROR, "SEC Not Req Mem, tread as not exist.\n"));
    return 0;
  }

  return 1;
//#endif
//[-end-160415-IB11270153-modify]//
}

SEC_INFOMATION  mSeCInfo;
BOOLEAN         mSeCInfoInited = FALSE;
VOID
InitSeCInfo (
  VOID
  )
{
  EFI_STATUS    Status;

  mSeCInfo.SeCExist = CheckSeCExist();
  if (mSeCInfo.SeCExist == 0) {
    mSeCInfoInited = TRUE;
//[-start-160418-IB07400718-add]//
    DEBUG ((EFI_D_ERROR, "SEC do not exist!!\n"));
//[-end-160418-IB07400718-add]//
    return;
  }
  mSeCInfo.SeCOpMode = GetSeCOpMode();
  mSeCInfo.SeCEnable = (mSeCInfo.SeCOpMode == 0) ? 1 : 0;
  mSeCInfo.SeCOpEnable = (mSeCInfo.SeCOpMode == 0 || mSeCInfo.SeCOpMode == 3) ? 1 : 0;
  mSeCInfo.HmrfpoEnable = GetSeCHMRFPOStatus();
  mSeCInfo.FwUpdate= GetSeCFwUpdateStatus();
  Status = GetSeCFwVersion(&mSeCInfo.SeCVer);
  if (EFI_ERROR(Status)) {
    mSeCInfo.SeCVerValid = FALSE;
  } else {
    mSeCInfo.SeCVerValid = TRUE;
  }
  Status = GetSeCCapability(&mSeCInfo.SeCCapability);
  if (EFI_ERROR(Status)) {
    mSeCInfo.SeCCapabilityValid = FALSE;
  } else {
    mSeCInfo.SeCCapabilityValid = TRUE;
  }
  Status = GetSeCFeature(&mSeCInfo.SeCFeature);
  if (EFI_ERROR(Status)) {
    mSeCInfo.SeCFeatureValid = FALSE;
  } else {
    mSeCInfo.SeCFeatureValid = TRUE;
  }
  Status = HeciGetOemTagMsg(&mSeCInfo.SeCOEMTag);
  if (EFI_ERROR(Status)) {
    mSeCInfo.SeCOEMTagValid = FALSE;
  } else {
    mSeCInfo.SeCOEMTagValid = TRUE;
  }
  mSeCInfoInited = TRUE;
//[-start-160418-IB07400718-add]//
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCOpMode = %x\n", mSeCInfo.SeCOpMode));
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCEnable = %x\n", mSeCInfo.SeCEnable));
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCOpEnable = %x\n", mSeCInfo.SeCOpEnable));
  DEBUG ((EFI_D_ERROR, "mSeCInfo.HmrfpoEnable = %x\n", mSeCInfo.HmrfpoEnable));
  DEBUG ((EFI_D_ERROR, "mSeCInfo.FwUpdate = %x\n", mSeCInfo.FwUpdate));
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCVerValid = %x\n", mSeCInfo.SeCVerValid));
  if (mSeCInfo.SeCVerValid) {
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCVer.CodeMajor = %x\n", mSeCInfo.SeCVer.CodeMajor));
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCVer.CodeMinor = %x\n", mSeCInfo.SeCVer.CodeMinor));
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCVer.CodeBuildNo = %x\n", mSeCInfo.SeCVer.CodeBuildNo));
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCVer.CodeHotFix = %x\n", mSeCInfo.SeCVer.CodeHotFix));
  }
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCCapabilityValid = %x\n", mSeCInfo.SeCCapabilityValid));
  if (mSeCInfo.SeCCapabilityValid) {
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCCapability = %x\n", mSeCInfo.SeCCapability));
  }
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCFeatureValid = %x\n", mSeCInfo.SeCFeatureValid));
  if (mSeCInfo.SeCFeatureValid) {
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCFeature = %x\n", mSeCInfo.SeCFeature));
  }
  DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCOEMTagValid = %x\n", mSeCInfo.SeCOEMTagValid));
  if (mSeCInfo.SeCOEMTagValid) {
    DEBUG ((EFI_D_ERROR, "mSeCInfo.SeCOEMTag = %x\n", mSeCInfo.SeCOEMTag));
  }
//[-end-160418-IB07400718-add]//
}


EFI_STATUS
EFIAPI
GetPlatformSeCInfo (
  OUT SEC_INFOMATION * SeCInfo
  )
{
  if (!mSeCInfoInited) {
    InitSeCInfo();
  }

  CopyMem(SeCInfo, &mSeCInfo, sizeof(SEC_INFOMATION));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SetPlatformSeCInfo (
  IN  SEC_INFOMATION * SeCInfo
  )
{
  if (mSeCInfo.SeCExist == 0) {
    return EFI_UNSUPPORTED;
  }
  if (mSeCInfo.SeCEnable != SeCInfo->SeCEnable) {
    if (SeCInfo->SeCEnable == 1) {
      HeciSetSeCEnableMsg();
    } else {
      HeciSetSeCDisableMsg(0);
    }
  }

  if (mSeCInfo.FwUpdate != SeCInfo->FwUpdate) {
    HeciSetLocalFwUpdate((UINT8)SeCInfo->FwUpdate);
    mSeCInfo.FwUpdate = SeCInfo->FwUpdate;
  }

  if (mSeCInfo.HmrfpoEnable != SeCInfo->HmrfpoEnable) {
    if (SeCInfo->HmrfpoEnable == 0) {
    } else {
      HmrfpoEnable();
    }
    CpuDeadLoop();
  }
  return EFI_SUCCESS;
}

EFI_STATUS
PerformSeCUnConfiguration (
  VOID
  )
{
  EFI_STATUS      Status;
  UINT32          CmdStatus;

  Status = HeciSeCUnconfigurationMsg(&CmdStatus);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "HeciSetSeCEnableMsg Status = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  DEBUG ((EFI_D_INFO, "Command Status = %r\n", CmdStatus));

  // wait for status
  Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
  DEBUG ((EFI_D_INFO, "HeciSeCUnconfigurationStatusMsg Status = %r CmdStatus = %x\n", Status, CmdStatus));
  while(CmdStatus == SEC_UNCONFIG_IN_PROGRESS) {
    Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
    DEBUG ((EFI_D_INFO, "HeciSeCUnconfigurationStatusMsg Status = %r CmdStatus = %x\n", Status, CmdStatus));
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "HeciGetFwFeatureStateMsg Status = %r\n", Status));
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }
  if (CmdStatus == SEC_UNCONFIG_SUCCESS) {
    // Send Global reset
    HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
    CpuDeadLoop();

  } else if (CmdStatus == SEC_UNCONFIG_ERROR) {
    DEBUG ((EFI_D_ERROR, "Error ! UnConfiguration Error happened\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
PerformCheckSeCUnConfiguration (
  VOID
  )
{
  UINT32      CmdStatus;
  EFI_STATUS  Status;
/*
  3.7.3.  SeC Unconfiguration - Common BIOS Flow On Boot
   On each boot, check SeC Unconfg status.
   While (SeC Unconfig Status == ME_UNCONFIG_IN_PROGRESS)
    o Wait for unconfigure completion.
   If (SeC Unconfig Status == ME_UNCONFIG_FINISHED)
    o Force a Global Reset.
   Else
    o Display error.
*/
  //  Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
  //  DEBUG ((EFI_D_ERROR, "UNCONFIGURATION Status = %r\n", CmdStatus));
  //  EFI_DEADLOOP();
  Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  while(CmdStatus == SEC_UNCONFIG_IN_PROGRESS) {
    Status = HeciSeCUnconfigurationStatusMsg(&CmdStatus);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "HeciGetFwFeatureStateMsg Status = %r\n", Status));
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }

  if (CmdStatus == SEC_UNCONFIG_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "UNCONFIGURATION Status = SEC_UNCONFIG_SUCCESS\n"));
    // Send Global reset
    HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
    CpuDeadLoop();

  } else if (CmdStatus == SEC_UNCONFIG_ERROR) {
    DEBUG ((EFI_D_ERROR, "Error ! UnConfiguration Error happened\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  return Status;
}

#define HMRFPO_B3_ENABLE

EFI_STATUS
SeCHmrfpoDisable(
  VOID
  )
/*++
Routine Description:

  Send the HMRFPO_DISABLE MEI message.

Arguments:

  Event             - The event that triggered this notification function
  ParentImageHandle - Pointer to the notification functions context

Returns:

  Status.

--*/
{
#ifdef HMRFPO_B3_ENABLE
  UINT32    FWstatus;
  UINT32    WriteValue;
  UINT8     StallCount;
  UINTN     Index;

  FWstatus    = 0;
  WriteValue  = 0;
  StallCount  = 0;

  DEBUG ((EFI_D_INFO, "(B3) SeC FW Downgrade - Send the HMRFPO_DISABLE MEI message\n"));

  WriteValue  = HeciPciRead32 (R_GEN_STS);
  WriteValue  = WriteValue & BRNGUP_HMRFPO_DISABLE_CMD_MASK;
  WriteValue  = WriteValue | 0x30000000;
  DEBUG ((EFI_D_INFO, "SeC FW Downgrade Writing %x to register %x of PCI space\n", WriteValue, R_GEN_STS));
  //
  // Set the highest Byte of General Status Register (Bits 28-31)
  //
  HeciPciWrite32 (R_GEN_STS, WriteValue);
  FWstatus = HeciPciRead32 (R_SEC_FW_STS0);
  while
  (
    ((FWstatus & BRNGUP_HMRFPO_DISABLE_OVR_MASK) != BRNGUP_HMRFPO_DISABLE_OVR_RSP) &&
    (StallCount < FW_MSG_DELAY_TIMEOUT)
  ) {
    DEBUG ((EFI_D_ERROR, "SeC FW Downgrade - SEC HMRFPO Disable Status = 0x%x\n", FWstatus));
    FWstatus = HeciPciRead32 (R_SEC_FW_STS0);
    gBS->Stall (FW_MSG_DELAY);
    StallCount = StallCount + 1;
  }

  if ((FWstatus & BRNGUP_HMRFPO_DISABLE_OVR_MASK) == BRNGUP_HMRFPO_DISABLE_OVR_RSP) {
    DEBUG ((EFI_D_ERROR, "SeC FW Downgrade Disable Msg Received Successfully\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "SeC FW Downgrade Disable Msg ACK not Received\n"));

  }
  //
  // Hide SEC devices so we don't get a yellow bang in OS with disabled devices
  //
  for (Index = 0; Index < 10; Index ++) {
  DEBUG ((EFI_D_INFO, "wait %x\n", Index));

  gBS->Stall (1000000);
    }

#endif // HMRFPO_B3_ENABLE
  return EFI_SUCCESS;
}


EFI_STATUS
PerformCheckHMRFPO (
  VOID
  )
{
  EFI_STATUS        Status;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;
  UINT32            SeCStatus;
  UINT64            Nonce;
  UINT32            FactoryDefaultBase;
  UINT32            FactoryDefaultLimit;
  HECI_FWS_REGISTER SeCFirmwareStatus;

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  ASSERT_EFI_ERROR (Status);

  Status = Heci->GetSeCMode (HECI1_DEVICE, &SeCMode);
  ASSERT_EFI_ERROR (Status);

  Status = Heci->GetSeCStatus (&SeCStatus);
  ASSERT_EFI_ERROR (Status);
  //
  // (B1) Whcih mode ?
  //
  if (SeCMode == SEC_MODE_NORMAL) {
    //
    // (A7) The BIOS sends the HMRFPO Lock MEI message and continues the normal boot
    //
    HeciHmrfpoLockResult = HMRFPO_LOCK_SUCCESS;
    //
    // The SEC firmware will ignore the HMRFPO LOCK command if SEC is in SEC manufacturing mode
    //
    if ((SeCFirmwareStatus.r.ManufacturingMode == 0) &&
        ((SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_IN_RECOVERY_MODE) ||
         (SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY))) {

      DEBUG ((EFI_D_ERROR, "(A7) SeC FW Downgrade - The BIOS sends the HMRFPO Lock SECI message and continues the normal boot\n"));

      FactoryDefaultBase  = 0;
      FactoryDefaultLimit = 0;
      Status              = HeciHmrfpoLock (&Nonce, &FactoryDefaultBase, &FactoryDefaultLimit, &HeciHmrfpoLockResult);
      if (Status != EFI_SUCCESS) {
        HeciHmrfpoLockResult = HMRFPO_LOCK_FAILURE;
      }
    }
  } else {
    //
    // (B3) Call the HMRFPO_DISABLE
    //
    if ((SeCFirmwareStatus.r.SeCOperationMode == SEC_OPERATION_MODE_SECOVR_HECI_MSG) &&
        (SEC_STATUS_SEC_STATE_ONLY (SeCStatus) == SEC_READY)
        ) {
      SeCHmrfpoDisable();
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PerformSeCOperation (
  IN  UINTN  SeCOpId
  )
{
  if (CheckSeCExist() == 0) {
    return EFI_UNSUPPORTED;
  }
  switch (SeCOpId) {
    case SEC_OP_UNCONFIGURATION:
      return PerformSeCUnConfiguration();
      break;
    case SEC_OP_CHECK_UNCONFIG:
      return PerformCheckSeCUnConfiguration();
      break;
    case SEC_OP_CHECK_HMRFPO:
      return PerformCheckHMRFPO();
      break;
    default:
      ASSERT(FALSE);
      break;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
SeCPolicyReadyToBootEvent (
  EFI_EVENT           Event,
  VOID                *ParentImageHandle
  )
/*++
Routine Description:

  Signal a event for SeC ready to boot.

Arguments:

  Event             - The event that triggered this notification function
  ParentImageHandle - Pointer to the notification functions context

Returns:

  Status.

--*/
{
//[-start-160216-IB03090424-modify]//
  EFI_HECI_PROTOCOL *Heci;
  EFI_STATUS  Status;
  UINT32      SeCMode;
  UINT32      SeCStatus;

  DEBUG ((EFI_D_INFO, "SeCPolicyReadyToBootEvent ++\n"));


  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)) {
    return;
  }

  Status = Heci->GetSeCMode (HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status)) {
    return;
  }

  Status = Heci->GetSeCStatus (&SeCStatus);
  if (EFI_ERROR(Status)) {
    return;
  }

  if ((SeCMode == SEC_MODE_NORMAL) &&(SEC_STATUS_SEC_STATE_ONLY(SeCStatus) == SEC_READY)) {
    if (mDxePlatformSeCPolicy.SeCConfig.EndOfPostEnabled == 1) {
      mSeCInfo.SeCEOPDone = 1;
    }
  }
  ASSERT_EFI_ERROR (Status);

  gBS->CloseEvent (Event);

  DEBUG ((EFI_D_INFO, "SeCPolicyReadyToBootEvent --\n"));
  return;
//[-end-160216-IB03090424-modify]//
}
