/** @file
  Implementation file for SeC Policy functionality

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

#include <Library/DebugLib.h>
#include <Library/SeCPolicyLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Heci.h>
#include <SeCAccess.h>

//
// Global variables
//
DXE_SEC_POLICY_PROTOCOL  *mDxePlatformSeCPolicy;


/**
  Check if SeC is enabled

  @param[in]  None

  @retval    None
**/
EFI_STATUS
SeCPolicyLibInit (
  VOID
  )
{
  EFI_STATUS  Status;

  //
  // Get the desired platform setup policy.
  //
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "No SEC Platform Policy Protocol available"));
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Check if HECI Communication is enabled in setup options.

  @param[in] VOID                 Parameter is VOID

  @retval FALSE                   HECI is disabled.
  @retval TRUE                    HECI is enabled.
**/
BOOLEAN
SeCHECIEnabled (
  VOID
  )
{
  BOOLEAN Supported;

  if (mDxePlatformSeCPolicy->SeCConfig.HeciCommunication != 1) {
    Supported = FALSE;
  } else {
    Supported = TRUE;
  }
  return Supported;
}


/**
  Check if End of Post Message is enabled in setup options.

  @param[in] VOID                 Parameter is VOID

  @retval FALSE                   EndOfPost is disabled.
  @retval TRUE                    EndOfPost is enabled.
**/
BOOLEAN
SeCEndOfPostEnabled (
  VOID
  )
{
  BOOLEAN Supported;

  if (mDxePlatformSeCPolicy->SeCConfig.EndOfPostEnabled != 1) {
    Supported = FALSE;
  } else {
    Supported = TRUE;
  }

  return Supported;
}

/**
  Check if Thermal Reporting Message is enabled in setup options.

  @param[in] VOID                 Parameter is VOID

  @retval FALSE                   Thermal Reporting is disabled.
  @retval TRUE                    Thermal Reporting is enabled.
**/
BOOLEAN
SeCTrEnabled (
  VOID
  )
{
  if (mDxePlatformSeCPolicy->SeCConfig.TrConfig->TrEnabled == 1) {
    return TRUE;
  }

  return FALSE;
}

/**
  Show SeC Error message.

  @param[in] MsgId           SeC error message ID.

  @retval None.
**/
VOID
SeCReportError (
 IN SEC_ERROR_MSG_ID MsgId
  )
{
  if (mDxePlatformSeCPolicy->Revision >= DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_3) {
    mDxePlatformSeCPolicy->SeCReportError (MsgId);
  }

  return ;
}

/**
  Check if SeCFwDowngrade is enabled in setup options.

  @param[in] None

  @retval FALSE                SeCFwDowngrade is disabled.
  @retval TRUE                 SeCFwDowngrade is enabled.
**/
BOOLEAN
SeCFwDowngradeSupported (
  VOID
  )
{
  if (mDxePlatformSeCPolicy->Revision >= DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_7) {
    if (mDxePlatformSeCPolicy->SeCConfig.SeCFwDownGrade == 1) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Check if integarted touch is enabled in setup options.

  @param[in] VOID                 Parameter is VOID

  @retval FALSE                   itouch is disabled.
  @retval TRUE                    itouch is enabled.
**/
BOOLEAN
SeCITouchEnabled (
  VOID
  )
{
  return mDxePlatformSeCPolicy->SeCConfig.ITouchEnabled;
}

