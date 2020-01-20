/** @file
  Header file for SeC functionality

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

#ifndef _SEC_LIB_H_
#define _SEC_LIB_H_

#include "SeCPolicyLib.h"
#include "HeciMsgLib.h"

/**
  Check if SeC is enabled

  @param[in] VOID              Parameter is VOID

  @retval EFI_SUCCESS          Command succeeded
**/
EFI_STATUS
SeCLibInit (
  VOID
  )
;

/**
  Host client gets Firmware update info from SEC client

  @param[in,out] SECCapability    Structure of FirmwareUpdateInfo

  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciGetSeCFwInfo (
  IN OUT SEC_CAP *SECCapability
  )
;

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
;

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
;

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
;

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
;

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
  IN OUT UINT8                  *AtState,
  IN OUT UINT8                  *AtLastTheftTrigger,
  IN OUT UINT16                 *AtLockState,
  IN OUT UINT16                 *AtAmPref
  )
;
#endif
