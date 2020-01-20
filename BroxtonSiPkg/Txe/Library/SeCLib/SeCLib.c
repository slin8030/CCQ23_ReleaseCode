/** @file
  Implementation file for SeC functionality

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

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

#include <MkhiMsgs.h>
#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/UefiBootServicesTableLib.h>

/**
  Check if SeC is enabled

  @param[in] VOID              Parameter is VOID

  @retval EFI_SUCCESS          Command succeeded
**/
EFI_STATUS
SeCLibInit (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  return Status;
}

/**
  Send Get Firmware SKU Request to SEC

  @param[in] FwCapsSku              Return Data from Get Firmware Capabilities MKHI Request

  @exception EFI_UNSUPPORTED        Current SEC mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetFwCapsSku (
  IN SECFWCAPS_SKU       *FwCapsSku
  )
{
  EFI_STATUS              Status;
  GEN_GET_FW_CAPSKU       MsgGenGetFwCapsSku;
  GEN_GET_FW_CAPS_SKU_ACK MsgGenGetFwCapsSkuAck;

  Status = HeciGetFwCapsSkuMsg (&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (((MsgGenGetFwCapsSkuAck.MKHIHeader.Fields.Command) == FWCAPS_GET_RULE_CMD) &&
      ((MsgGenGetFwCapsSkuAck.MKHIHeader.Fields.IsResponse) == 1) &&
      (MsgGenGetFwCapsSkuAck.MKHIHeader.Fields.Result == 0)
     ) {
    *FwCapsSku = MsgGenGetFwCapsSkuAck.Data.FWCapSku;
  }

  return EFI_SUCCESS;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get Ibex Peak platform type.
  One of usages is to utilize this command to determine if the platform runs in
  4M or 8M size firmware.

  @param[out] RuleData            PlatformBrand,
                                  IntelSeCFwImageType,
                                  SuperSku,
                                  PlatformTargetMarketType,
                                  PlatformTargetUsageType

  @exception EFI_UNSUPPORTED      Current SEC mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetPlatformType (
  OUT PLATFORM_TYPE_RULE_DATA   *RuleData
  )
{
  EFI_STATUS  Status;

  Status = HeciGetPlatformTypeMsg (RuleData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Send Get Firmware Version Request to SEC

  @param[in,out] MsgGenGetFwVersionAckData   Return themessage of FW version

  @param MsgGenGetFwVersionAck               Return themessage of FW version

  @exception EFI_UNSUPPORTED                 Current SEC mode doesn't support this function
  @retval EFI_SUCCESS                        Command succeeded
  @retval EFI_DEVICE_ERROR                   HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT                        HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL               Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetFwVersion (
  IN OUT GEN_GET_FW_VER_ACK_DATA      *MsgGenGetFwVersionAckData
  )
{
  EFI_STATUS          Status;
  GEN_GET_FW_VER_ACK  MsgGenGetFwVersionAck;
  //  EFI_DEADLOOP(); need to do debug here
  Status = HeciGetFwVersionMsg (&MsgGenGetFwVersionAck);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((MsgGenGetFwVersionAck.MKHIHeader.Fields.Command == GEN_GET_FW_VERSION_CMD) &&
      (MsgGenGetFwVersionAck.MKHIHeader.Fields.IsResponse == 1) &&
      (MsgGenGetFwVersionAck.MKHIHeader.Fields.Result == 0)
     ) {
    *MsgGenGetFwVersionAckData = MsgGenGetFwVersionAck.Data;
  }

  return EFI_SUCCESS;
}

/**
  Host client gets Firmware update info from SEC client

  @param[in,out] SECCapability    Structure of FirmwareUpdateInfo

  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciGetSeCFwInfo (
  IN OUT SEC_CAP *SECCapability
  )
{
  EFI_STATUS              Status;
  DXE_SEC_POLICY_PROTOCOL  *mDxePlatformSeCPolicy;
  SECFWCAPS_SKU            FwCapsSku;

  //
  // Get the SEC platform policy.
  //
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "No SEC Platform Policy Protocol available"));
    return EFI_UNSUPPORTED;
  }

  SECCapability->SeCEnabled = 1;

  FwCapsSku.Data          = mDxePlatformSeCPolicy->SeCConfig.FwCapsSku;
  if (FwCapsSku.Fields.IntelAT) {
    SECCapability->AtSupported = 1;
  }

  if (FwCapsSku.Fields.KVM) {
//    SECCapability->IntelKVM = 1;
  }

  if (mDxePlatformSeCPolicy->SeCConfig.PlatformBrand == INTEL_AMT_BRAND) {
//    SECCapability->IntelAmtFw        = 1;
//  SECCapability->LocalWakeupTimer  = 1;
  }

  if (mDxePlatformSeCPolicy->SeCConfig.PlatformBrand == INTEL_STAND_MANAGEABILITY_BRAND) {
//    SECCapability->IntelAmtFwStandard = 1;
  }

  SECCapability->SeCMinorVer  = mDxePlatformSeCPolicy->SeCVersion.CodeMinor;
  SECCapability->SeCMajorVer  = mDxePlatformSeCPolicy->SeCVersion.CodeMajor;
  SECCapability->SeCBuildNo   = mDxePlatformSeCPolicy->SeCVersion.CodeBuildNo;
  SECCapability->SeCHotFixNo  = mDxePlatformSeCPolicy->SeCVersion.CodeHotFix;

  return Status;
}

/**
  Dummy return for SeC signal event use

  @param[in] Event               The event that triggered this notification function
  @param[in] ParentImageHandle   Pointer to the notification functions context

  @return EFI_SUCCESS            Always return EFI_SUCCESS
**/
EFI_STATUS
SeCEmptyEvent (
  IN EFI_EVENT           Event,
  IN void                *ParentImageHandle
  )
{
  return EFI_SUCCESS;
}

/**
  Get AT State Information From Stored SEC platform policy

  @param[in,out] AtState                     Pointer to AT State Information
  @param[in,out] AtLastTheftTrigger          Pointer to Variable holding the cause of last AT Stolen Stae
  @param[in,out] AtLockState                 Pointer to variable indicating whether AT is locked or not
  @param[in,out] AtAmPref                    Pointer to variable indicating whether TDTAM or PBA should be used

  @exception EFI_UNSUPPORTED                 Current SEC mode doesn't support this function
  @retval EFI_SUCCESS                        Command succeeded
  @retval EFI_DEVICE_ERROR                   HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT                        HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL               Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
GetAtStateInfo (
  IN OUT UINT8                 *AtState,
  IN OUT UINT8                 *AtLastTheftTrigger,
  IN OUT UINT16                *AtLockState,
  IN OUT UINT16                *AtAmPref
  )
{
  EFI_STATUS              Status;
  DXE_SEC_POLICY_PROTOCOL  *mDxePlatformSeCPolicy;

  //
  // Get the SEC platform policy.
  //
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "No SEC Platform Policy Protocol available"));
    return EFI_UNSUPPORTED;
  }

  *AtState            = mDxePlatformSeCPolicy->AtConfig.AtState;
  *AtLastTheftTrigger = mDxePlatformSeCPolicy->AtConfig.AtLastTheftTrigger;
  *AtLockState        = mDxePlatformSeCPolicy->AtConfig.AtLockState;
  *AtAmPref           = mDxePlatformSeCPolicy->AtConfig.AtAmPref;

  return EFI_SUCCESS;
}
