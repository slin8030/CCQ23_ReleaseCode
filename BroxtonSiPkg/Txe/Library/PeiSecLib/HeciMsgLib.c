/** @file
  Implementation file for Heci Message functionality

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

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/TimerLib.h>
#include <Ppi/HeciPpi.h>
#include <CoreBiosMsg.h>
#include <HeciRegs.h>
#include <SeCAccess.h>
#include <IndustryStandard/Pci22.h>

/**
  Calculate if the circular buffer has overflowed.
  Corresponds to HECI HPS (part of) section 4.2.1

  @param[in] ReadPointer     - Location of the read pointer.
  @param[in] WritePointer    - Location of the write pointer.

  @return Number of filled slots.

**/
UINT8
FilledSlots (
  IN  UINT32 ReadPointer,
  IN  UINT32 WritePointer
  )
{
  UINT8 FilledSlots;

  //
  // Calculation documented in HECI HPS 0.68 section 4.2.1
  //
  FilledSlots = (((INT8) WritePointer) - ((INT8) ReadPointer));

  return FilledSlots;
}

/**
  Calculate if the circular buffer has overflowed
  Corresponds to HECI HPS (part of) section 4.2.1

  @param[in] ReadPointer          Value read from host/me read pointer
  @param[in] WritePointer         Value read from host/me write pointer
  @param[in] BufferDepth          Value read from buffer depth register

  @retval EFI_DEVICE_ERROR        The circular buffer has overflowed
  @retval EFI_SUCCESS             The circular buffer does not overflowed
**/
EFI_STATUS
OverflowCB (
  IN  UINT32 ReadPointer,
  IN  UINT32 WritePointer,
  IN  UINT32 BufferDepth
)
{
  UINT8 FilledSlots;

  //
  // Calculation documented in HECI HPS 0.68 section 4.2.1
  //
  FilledSlots = (((INT8) WritePointer) - ((INT8) ReadPointer));

  //
  // test for overflow
  //
  if (FilledSlots > ((UINT8) BufferDepth)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Send Get Firmware SKU Request to SEC

  @param[in,out] FwCapsSku          SEC Firmware Capability SKU

  @exception EFI_UNSUPPORTED        Current SEC mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetFwCapsSkuMsg(
  IN OUT SECFWCAPS_SKU             *FwCapsSku
  )
{
  EFI_STATUS                      Status;
  GEN_GET_FW_CAPS_SKU_BUFFER      MsgGenGetFwCapsSku;
  UINT32                          Length;
  UINT32                          RecvLength;
  UINT32                          SeCMode;

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetFwCapsSku.Request.MKHIHeader.Data = 0;
  MsgGenGetFwCapsSku.Request.MKHIHeader.Fields.GroupId = MKHI_FWCAPS_GROUP_ID;
  MsgGenGetFwCapsSku.Request.MKHIHeader.Fields.Command = FWCAPS_GET_RULE_CMD;
  MsgGenGetFwCapsSku.Request.MKHIHeader.Fields.IsResponse = 0;
  MsgGenGetFwCapsSku.Request.Data.RuleId = 0;
  Length = sizeof (GEN_GET_FW_CAPSKU);
  RecvLength = sizeof (GEN_GET_FW_CAPS_SKU_ACK);

  ///
  /// Send Get FW SKU Request to SEC
  ///
  Status = HeciSendwACK(
             HECI1_DEVICE,
             (UINT32 *)&MsgGenGetFwCapsSku,
             Length,
             &RecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );
  if (!EFI_ERROR(Status) && ((MsgGenGetFwCapsSku.Response.MKHIHeader.Fields.Command) == FWCAPS_GET_RULE_CMD) &&
    ((MsgGenGetFwCapsSku.Response.MKHIHeader.Fields.IsResponse) == 1) &&
    (MsgGenGetFwCapsSku.Response.MKHIHeader.Fields.Result == 0)
    ) {
    *FwCapsSku = MsgGenGetFwCapsSku.Response.Data.FWCapSku;
  }

  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX. One of usages is to utilize
  this command to determine if the platform runs in Consumer or Corporate SKU
  size firmware.

  @param[out] RuleData            PlatformBrand,
                                  IntelMeFwImageType,
                                  SuperSku,
                                  PlatformTargetUsageType

  @exception EFI_UNSUPPORTED      Current SEC mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetPlatformTypeMsg(
  OUT PLATFORM_TYPE_RULE_DATA     *RuleData
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  GEN_GET_PLATFORM_TYPE_BUFFER    MsgGenGetPlatformType;
  UINT32                          SeCMode;

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (MeMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetPlatformType.Request.MKHIHeader.Data = 0;
  MsgGenGetPlatformType.Request.MKHIHeader.Fields.GroupId = MKHI_FWCAPS_GROUP_ID;
  MsgGenGetPlatformType.Request.MKHIHeader.Fields.Command = FWCAPS_GET_RULE_CMD;
  MsgGenGetPlatformType.Request.MKHIHeader.Fields.IsResponse = 0;
  MsgGenGetPlatformType.Request.Data.RuleId = 0x1D;
  Length = sizeof (GEN_GET_PLATFORM_TYPE);
  RecvLength = sizeof (GEN_GET_PLATFORM_TYPE_ACK);

  ///
  /// Send Get Platform Type Request to SEC
  ///
  Status = HeciSendwACK(
             HECI1_DEVICE,
             (UINT32 *)&MsgGenGetPlatformType,
             Length,
             &RecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );
  if (!EFI_ERROR(Status)) {
    *RuleData = MsgGenGetPlatformType.Response.Data.RuleData;
  }

  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX. To Get Firmware Version Request to SEC.

  @param[in,out] MsgGenGetFwVersionAck    Return themessage of FW version

  @exception EFI_UNSUPPORTED      Current SEC mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
PeiHeciGetFwVersionMsg(
  IN OUT GEN_GET_FW_VER_ACK     *MsgGenGetFwVersionAck
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  GEN_GET_FW_VER_ACK_BUFFER       *MsgGenGetFwVersion;
  UINT32                          SeCMode;

  Status = HeciGetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR(Status) || (MeMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetFwVersion.Request.MKHIHeader.Data = 0;
  MsgGenGetFwVersion.Request.MKHIHeader.Fields.GroupId = MKHI_GEN_GROUP_ID;
  MsgGenGetFwVersion.Request.MKHIHeader.Fields.Command = GEN_GET_FW_VERSION_CMD;
  MsgGenGetFwVersion.Request.MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GET_FW_VER);
  RecvLength = sizeof (GEN_GET_FW_VER_ACK);

  //
  // Send Get Firmware Version Request to SEC
  //
  Status = HeciSendwACK(
             HECI1_DEVICE,
             (UINT32 *)&MsgGenGetFwVersion,
             Length,
             &RecvLength,
             BIOS_FIXED_HOST_ADDR,
             HECI_CORE_MESSAGE_ADDR
             );
  if (!EFI_ERROR(Status)) {
    memcpy(MsgGenGetFwVersionAck, &(MsgGenGetFwVersion.Response), RecvLength);
  }

  return Status;
}
