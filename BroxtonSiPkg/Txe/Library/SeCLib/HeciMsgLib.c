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
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/PerformanceLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Heci.h>
#include <Protocol/ScReset.h>
#include <CoreBiosMsg.h>
#include <HeciRegs.h>
#include <SeCAccess.h>
//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
#include <Library/HobLib.h>
#endif
//[-end-161123-IB07250310-add]//
//[-start-160629-IB07400749-add]//
#ifdef EFI_DEBUG
#ifndef EFI_DEBUG_HECI_MESSAGE
#undef DEBUG
#define DEBUG(Expression)
#endif
#endif
//[-end-160629-IB07400749-add]//

/**
  Query firmware update progress thru HECI

  @param[in,out]  Percentage        Update percentage
  @param[in,out]  CurrentStage      current stage of the update
  @param[in,out]  TotalStages       Total stages of the update
  @param[in,out]  LastUpdateStatus  The latest error code for the update
  @param[in,out]  LastResetType     Last reset type required for the update
  @param[in,out]  InProgress        Indicates if the update is in progress.

  @exception EFI_UNSUPPORTED        Current Sec mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciFwuQueryUpdateStatus(
  IN OUT UINT32 *Percentage,
  IN OUT UINT32 *CurrentStage,
  IN OUT UINT32 *TotalStages,
  IN OUT UINT32 *LastUpdateStatus,
  IN OUT UINT32 *LastResetType,
  IN OUT UINT8  *InProgress
  )
{
  EFI_HECI_PROTOCOL            *Heci;
  EFI_STATUS                   Status;
  UINT32                       HeciLength;

  MKHI_FWUPDATE_QUERY_SATUS_REQ   FwupdateQueryStatusRequest;
  MKHI_FWUPDATE_QUERY_STATUS_ACK  FwupdateQueryStatusAck;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FwupdateQueryStatusRequest.MKHIHeader.Data = 0;
  FwupdateQueryStatusRequest.MKHIHeader.Fields.Command = FWU_QUERY_STATUS_CMD;
  FwupdateQueryStatusRequest.MKHIHeader.Fields.IsResponse = 0;
  FwupdateQueryStatusRequest.MKHIHeader.Fields.GroupId    = FWU_QUERY_STATUS_GROUP_ID;
  FwupdateQueryStatusRequest.MKHIHeader.Fields.Reserved = 0;
  FwupdateQueryStatusRequest.MKHIHeader.Fields.Result = 0;

  HeciLength = sizeof (MKHI_FWUPDATE_QUERY_SATUS_REQ);

  Status = Heci->SendMsg (
                   HECI1_DEVICE,
                   (UINT32 *)&FwupdateQueryStatusRequest,
                   HeciLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_CORE_MESSAGE_ADDR
                   );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Unable to send FWU_STATUS_Request to SEC.\n"));
    return Status;
  }

  HeciLength = sizeof (MKHI_FWUPDATE_QUERY_STATUS_ACK);
  ZeroMem(&FwupdateQueryStatusAck, HeciLength);

  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&FwupdateQueryStatusAck,
                   &HeciLength
                   );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to get firmware update status from SEC.\n"));
    return Status;
  }

  *Percentage = FwupdateQueryStatusAck.QueryAckData.PercentComplete;
  *CurrentStage = FwupdateQueryStatusAck.QueryAckData.CurrentStage;
  *TotalStages = FwupdateQueryStatusAck.QueryAckData.TotalStages;
  *LastUpdateStatus = FwupdateQueryStatusAck.QueryAckData.LastUpdateStatus;

  *LastResetType = FwupdateQueryStatusAck.ResetType;
  *InProgress = (UINT8)FwupdateQueryStatusAck.Flags.Fields.FwuInProgress;

  return EFI_SUCCESS;


}


/**
  Send Core BIOS Reset Request Message through HECI.

  @param[in] ResetOrigin          Reset source
  @param[in] ResetType            Global or Host reset

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendCbmResetRequest (
  IN  UINT8                             ResetOrigin,
  IN  UINT8                             ResetType
  )
{
  EFI_HECI_PROTOCOL         *Heci;
  EFI_STATUS                Status;
  UINT32                    HeciLength;
  CBM_RESET_REQ             CbmResetRequest;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CbmResetRequest.MKHIHeader.Data               = 0;
  CbmResetRequest.MKHIHeader.Fields.Command     = CBM_RESET_REQ_CMD;
  CbmResetRequest.MKHIHeader.Fields.IsResponse  = 0;
  CbmResetRequest.MKHIHeader.Fields.GroupId     = MKHI_CBM_GROUP_ID;
  CbmResetRequest.MKHIHeader.Fields.Reserved    = 0;
  CbmResetRequest.MKHIHeader.Fields.Result      = 0;
  CbmResetRequest.Data.RequestOrigin            = ResetOrigin;
  CbmResetRequest.Data.ResetType                = ResetType;

  HeciLength = sizeof (CBM_RESET_REQ);

  Status = Heci->SendMsg (
                   HECI1_DEVICE,
                   (UINT32 *) &CbmResetRequest,
                   HeciLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_CORE_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Send Reset Request - %r\n", Status));
  }

  return Status;
}

/**
  Check Firmware Update Progress Status throught HECI

  @param[in,out] InProgress       Firmware update progress status

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciCheckFwuInProgress(
  IN OUT UINT8 *InProgress
  )
{
  EFI_STATUS          Status;
  EFI_HECI_PROTOCOL   *Heci;
  HECI_FWS_REGISTER   FwStatus;
  UINT8               StallCount;

  Status = EFI_ABORTED;
  StallCount = 0;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to locate EfiHeciProtocol.\n"));
    return Status;
  }

  Status = Heci->GetSeCStatus(&FwStatus.ul);
  if(!EFI_ERROR(Status)) {

    if(FwStatus.ul & SEC_FW_UPDATES_IN_PROGRESS) {
      DEBUG((EFI_D_ERROR, "!!!!!!!!!!!Check FWU in progress: Sec in update process already.\n"));
      *InProgress = 1;

    } else {

      *InProgress = 0;
    }

  } else {
    DEBUG((EFI_D_ERROR, "Failed to GetSecStatus from PCI read.\n"));
  }

  return Status;

}


/**
  Waits for last firmware update to be finished. Blocking call that might at most stall the system for 90 seconds.

  @param[in]  None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciFwuWaitOnLastUpdate(
  )
{
  EFI_STATUS          Status;
  EFI_HECI_PROTOCOL   *Heci;
  HECI_FWS_REGISTER   FwStatus;
  UINT8               StallCount;

  Status = EFI_ABORTED;
  StallCount = 0;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if(EFI_ERROR(Status)){
    return Status;
  }

  Status = Heci->GetSeCStatus(&FwStatus.ul);
  if(!EFI_ERROR(Status)) {
    if(FwStatus.ul & SEC_FW_UPDATES_IN_PROGRESS) {
      DEBUG((EFI_D_ERROR, "!!!!!!!!!!!Sec in update process already.\n"));
    }

    while((FwStatus.ul & SEC_FW_UPDATES_IN_PROGRESS) && (StallCount < FWU_TIMEOUT)) {
      gBS->Stall(ONE_SECOND_TIMEOUT);
      StallCount += 1;
      Status = Heci->GetSeCStatus(&FwStatus.ul);
    }

    if(!(FwStatus.ul & SEC_FW_UPDATES_IN_PROGRESS)) {
      DEBUG((EFI_D_INFO, "####Sec not in update process. proceed.\n"));
      return EFI_SUCCESS;
    }
  }

  return Status;

}


/**
  Tear down a dynamic connection for FWU.

  @param[in]  SecAddress          Returns the SecAddress to be used by other FWU APIs
  @param[in]  MaxBufferSize       Specifies the maximum buffer size the FWU link allows.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciDisconnectFwuInterface(
  IN UINT8  SecAddress,
  IN UINT32 MaxBufferSize
  )
{
  EFI_STATUS            Status;
  EFI_HECI_PROTOCOL     *Heci;

  HBM_DISCONNECT_MSG         DisconnectMsg;
  HBM_DISCONNECT_MSG_REPLY   DisconnectMsgReply;
  UINT32 MsgReplyLen = MaxBufferSize;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }


  ZeroMem(&DisconnectMsg, sizeof(HBM_DISCONNECT_MSG));
  ZeroMem(&DisconnectMsgReply, sizeof(HBM_DISCONNECT_MSG_REPLY));

  DisconnectMsg.Cmd = HBM_CMD_DISCONNECT;
  DisconnectMsg.SecAddress = SecAddress;
  DisconnectMsg.HostAddress = BIOS_FIXED_HOST_ADDR+1;

  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&DisconnectMsg,
                   sizeof(HBM_DISCONNECT_MSG),
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HBM_MSG_ADDR
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "####Failed to send HBM_DISCONNECT_MSG.\n"));
    return Status;
  }

  MsgReplyLen = MaxBufferSize;
  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&DisconnectMsgReply,
                   &MsgReplyLen
                   );

  if(EFI_ERROR(Status) || DisconnectMsgReply.CmdReply != HBM_CMD_DISCONNECT_REPLY || DisconnectMsgReply.Status != 0) {
    DEBUG((EFI_D_ERROR, "####HeciDisconnect failed with ReadMsg, Status is: %d.\n", Status));
    return EFI_ABORTED;
  }

  DEBUG((EFI_D_INFO, "####SUCCESSFULLY DISCONNECT FROM FWU INTERFACE####\n"));
  return EFI_SUCCESS;

}

/**
  Called by each FWU request API when there's a flow control message expected on the link.

  @param[in]  None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSecToHostFlowControl(
  )
{
  EFI_STATUS         Status;
  EFI_HECI_PROTOCOL  *Heci;

  HBM_FLOW_CONTROL_MSG         FlowCtrlMsg;
  UINT32             MsgLen = sizeof(HBM_FLOW_CONTROL_MSG);

  DEBUG((EFI_D_INFO, "####Sec to Heci flow control####\n"));

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  //First get flow control from SEC
  //

  ZeroMem(&FlowCtrlMsg, sizeof(HBM_FLOW_CONTROL_MSG));
  Status = Heci->ReadMsg(
                 HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&FlowCtrlMsg,
                   &MsgLen
                   );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "#####Fwuflow control: wait for SEC failed with status:%d.\n", Status));
    return Status;
  }


  return EFI_SUCCESS;
}

/**
  Called by each FWU request API when host needs to inform SEC it's ready to accept any input.

  @param[in]  SecAddress          The dynamic sec address the flow control

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHostToSecFlowControl(
  IN UINT8 SecAddress
  )
{
  EFI_STATUS         Status;
  EFI_HECI_PROTOCOL  *Heci;

  HBM_FLOW_CONTROL_MSG         FlowCtrlMsg;

  DEBUG((EFI_D_INFO, "####Host to SeC flow control####\n"));

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  //
  //Send flow control to SEC
  //
  ZeroMem(&FlowCtrlMsg, sizeof(HBM_FLOW_CONTROL_MSG));
  FlowCtrlMsg.Cmd = HBM_CMD_FLOW_CONTROL;
  FlowCtrlMsg.HostAddress = BIOS_FIXED_HOST_ADDR + 1;
  FlowCtrlMsg.SecAddress = SecAddress;
  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&FlowCtrlMsg,
                   sizeof(HBM_FLOW_CONTROL_MSG),
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HBM_MSG_ADDR
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "#####Fwuflow control: Send to SEC failed with status:%d.\n", Status));
    return Status;
  }

  return EFI_SUCCESS;
}


/**
  Called by each FWU request API when both sec and host needs to inform each other  it's ready to accept any input.

  @param[in]  SecAddress          Returns the SecAddress to be used by other FWU APIs

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciBiDirectionFlowControl(
  IN   UINT8 SecAddress
  )
{
  EFI_STATUS         Status;
  EFI_HECI_PROTOCOL  *Heci;

  HBM_FLOW_CONTROL_MSG         FlowCtrlMsg;
  UINT32             MsgLen = sizeof(HBM_FLOW_CONTROL_MSG);

  DEBUG((EFI_D_INFO, "####Bi-Directional flow control####\n"));

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  //
  //First get flow control from SEC
  //
  gBS->Stall(200);

  ZeroMem(&FlowCtrlMsg, sizeof(HBM_FLOW_CONTROL_MSG));
  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&FlowCtrlMsg,
                   &MsgLen
                   );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "#####Fwuflow control: wait for SEC failed with status:%d.\n", Status));
    return Status;
  }

  //
  //Then send flow control to SEC
  //
  ZeroMem(&FlowCtrlMsg, sizeof(HBM_FLOW_CONTROL_MSG));
  FlowCtrlMsg.Cmd = HBM_CMD_FLOW_CONTROL;
  FlowCtrlMsg.HostAddress = BIOS_FIXED_HOST_ADDR + 1;
  FlowCtrlMsg.SecAddress = SecAddress;
  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&FlowCtrlMsg,
                   sizeof(HBM_FLOW_CONTROL_MSG),
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HBM_MSG_ADDR
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "#####Fwuflow control: Send to SEC failed with status:%d.\n", Status));
    return Status;
  }

  return EFI_SUCCESS;

}

/**
  Setup a dynamic connection for FWU.

  @param[in,out]  SecAddress      Returns the SecAddress to be used by other FWU APIs
  @param[in]  MaxBufferSize       Specifies the maximum buffer size the FWU link allows.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciConnectFwuInterface(
  IN OUT UINT8  *SecAddress,
  OUT UINT32    *MaxBufferSize
  )
{
  EFI_STATUS                   Status;
  EFI_HECI_PROTOCOL            *Heci;
  HBM_ENUM_MSG                 EnumMsg;
  HBM_ENUM_MSG_REPLY           EnumMsgReply;
  HBM_CLIENT_PROP_MSG          PropMsg;
  HBM_CLIENT_PROP_MSG_REPLY    PropMsgReply;
  HBM_CONNECT_MSG              ConnectMsg;
  HBM_CONNECT_MSG_REPLY        ConnectMsgReply;
  OEM_UUID                     FwuClientGuid;
  UINT32                       MsgReplyLen = 0;
  UINT8                        AddrIdx = 0;
  UINTN                        EnumIdx = 0;
  UINT8                        GuidData4Array[8] = {0x8F, 0x78, 0x60, 0x01, 0x15, 0xA3, 0x43, 0x27};

  FwuClientGuid.Data1   = 0x309DCDE8;
  FwuClientGuid.Data2   = 0xCCB1;
  FwuClientGuid.Data3   = 0x4062;
  CopyMem(&FwuClientGuid.Data4[0], &GuidData4Array[0], 8);



  DEBUG((EFI_D_INFO, "HeciConnectFwuInterface +++.\n"));
  //check NULL pointer
  if(SecAddress == NULL) {
    DEBUG((EFI_D_ERROR, "Invalid SecAdress assigned.\n"));
    return EFI_ABORTED;
  }
  *SecAddress = 0;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  if(SecAddress == NULL) {
    DEBUG((EFI_D_ERROR, "Invalid SecAdress assigned.\n"));
    return EFI_ABORTED;
  }

  ZeroMem(&EnumMsg, sizeof(HBM_ENUM_MSG));
  ZeroMem(&EnumMsgReply, sizeof(HBM_ENUM_MSG_REPLY));
  EnumMsg.Cmd = HBM_CMD_ENUM;

  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&EnumMsg,
                   sizeof(HBM_ENUM_MSG),
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HBM_MSG_ADDR
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to send HBM_ENUM_MSG.\n"));
    return Status;
  }

  MsgReplyLen = sizeof(HBM_ENUM_MSG_REPLY);
  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&EnumMsgReply,
                   &MsgReplyLen
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "HeciHBMEnum failed with ReadMsg, Status is: %d.\n", Status));
    return EFI_ABORTED;
  }

  if(EnumMsgReply.CmdReply != HBM_CMD_ENUM_REPLY ) {
    DEBUG((EFI_D_ERROR, "HBM enum get invalid reply:%d\n", EnumMsgReply.CmdReply));
    return EFI_ABORTED;
  }


  for(AddrIdx = 8;  AddrIdx > 0; AddrIdx--) {
    DEBUG((EFI_D_INFO, "Idx:%d, Value:%08x\n", 8 - AddrIdx, EnumMsgReply.ValidAddresses[AddrIdx - 1]));
  }

  ZeroMem(&PropMsg, sizeof(HBM_CLIENT_PROP_MSG));
  ZeroMem(&PropMsgReply, sizeof(HBM_CLIENT_PROP_MSG_REPLY));

  //
  //Check connections' GUID that has ValidAddress bit set. ValidAddress[0] includes the lowest bit, while ValidAddress[7] includes the highest bit.
  //Currently SEC does not mask validaddress bit for FWU interface, thus we have to go thru all entries instead of checking those with ValidAddress bit set.
  //

  for(EnumIdx = 0; EnumIdx <= 255; EnumIdx++) {
    ZeroMem(&PropMsg, sizeof(HBM_CLIENT_PROP_MSG));
    ZeroMem(&PropMsgReply, sizeof(HBM_CLIENT_PROP_MSG_REPLY));

    PropMsg.Cmd = HBM_CMD_CLIENT_PROP;
    PropMsg.Address = (UINT8)EnumIdx;

    Status = Heci->SendMsg(
                     HECI1_DEVICE,
                     (UINT32*)&PropMsg,
                     sizeof(HBM_CLIENT_PROP_MSG),
                     BIOS_FIXED_HOST_ADDR,
                     HECI_HBM_MSG_ADDR
                     );
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to send HBM_CLIENT_PROP_MSG.\n"));
      return Status;
    }

    MsgReplyLen = sizeof(HBM_CLIENT_PROP_MSG_REPLY);
    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)&PropMsgReply,
                     &MsgReplyLen
                     );

    if(EFI_ERROR(Status)) {
      return EFI_ABORTED;
    }

    if(PropMsgReply.Status != 0) {
      continue;
    }

    if(!CompareMem(&FwuClientGuid, &PropMsgReply.ProtocolName, sizeof(OEM_UUID))) {
      DEBUG((EFI_D_ERROR, "####Match:%d - Guid:%08x-%04x-%04x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x.\n", \
             EnumIdx,   \
             PropMsgReply.ProtocolName.Data1, \
             PropMsgReply.ProtocolName.Data2, \
             PropMsgReply.ProtocolName.Data3, \
             PropMsgReply.ProtocolName.Data4[0], \
             PropMsgReply.ProtocolName.Data4[1], \
             PropMsgReply.ProtocolName.Data4[2], \
             PropMsgReply.ProtocolName.Data4[3], \
             PropMsgReply.ProtocolName.Data4[4], \
             PropMsgReply.ProtocolName.Data4[5], \
             PropMsgReply.ProtocolName.Data4[6], \
             PropMsgReply.ProtocolName.Data4[7]  \
            ));
      *SecAddress = PropMsgReply.Address;
      *MaxBufferSize = PropMsgReply.MaxMessageLength;
      break;

    }

  }

  if(*SecAddress == 0) {
    DEBUG((EFI_D_ERROR, "Failed to retrieve SEC address for FWU.\n"));
    return EFI_ABORTED;
  } else {
    DEBUG((EFI_D_INFO, "####Connection property#####\n"));
    DEBUG((EFI_D_INFO, "Address:%d, Protcol Ver:%d, MaxConnections:%d, FixedAddress:%d, SglRcvBuf:%d, MTU:%d.\n", \
           PropMsgReply.Address,  PropMsgReply.ProtocolVersion, PropMsgReply.MaximumConnections, \
           PropMsgReply.FixedAddress, PropMsgReply.SingleRecvBuffer, PropMsgReply.MaxMessageLength));

  }

  //
  //Now try to connect to the FWU update interace.
  //
  ZeroMem(&ConnectMsg, sizeof(HBM_CONNECT_MSG));
  ZeroMem(&ConnectMsgReply, sizeof(HBM_CONNECT_MSG_REPLY));

  ConnectMsg.Cmd = HBM_CMD_CONNECT;
  ConnectMsg.SecAddress = *SecAddress;
  ConnectMsg.HostAddress = BIOS_FIXED_HOST_ADDR + 1;

  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&ConnectMsg,
                   sizeof(HBM_CONNECT_MSG),
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HBM_MSG_ADDR
                   );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuConnect Send failed with status:%d.\n", Status));
    return Status;
  }

  MsgReplyLen = sizeof(HBM_CONNECT_MSG_REPLY);
  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&ConnectMsgReply,
                   &MsgReplyLen
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuConnect Recv failed with status:%d.\n", Status));
    return Status;
  }

  if(ConnectMsgReply.CmdReply != HBM_CMD_CONNECT_REPLY) {
    DEBUG((EFI_D_ERROR, "####Reply Msg of connect is not HBM_CMD_CONNECT_REPLY.\n"));
    return EFI_ABORTED;
  }

  if(ConnectMsgReply.Status == 0 || ConnectMsgReply.Status == 2) { //Connection setup succeed, or already connected.
    DEBUG((EFI_D_ERROR, "######CONNECTION SETUP SUCCESSFULLY######: \n"));
  } else {
    DEBUG((EFI_D_ERROR, "####Failed to setup connection. Aborted with Status:%d.\n", Status));
    return EFI_ABORTED;
  }

  Status = HeciSecToHostFlowControl();
  if(EFI_ERROR(Status)) {
    return Status;
  }

  DEBUG((EFI_D_INFO, "HeciConnectFwuInterface ---.\n"));

  return EFI_SUCCESS;
}


/**
  Get firmware version from FWU interface instead of MKHI.

  @param[out]                     Version - Returns the version number of current running SEC FW.
  @param[in]                      SecAddress - Dynamic sec address for FWU connection

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuGetVersionMsg(
  OUT VERSION  *Version,
  IN  UINT8    SecAddress
  )
{
  EFI_STATUS                Status;
  EFI_HECI_PROTOCOL         *Heci;

  FWU_GET_VERSION_MSG       Msg;
  FWU_GET_VERSION_MSG_REPLY MsgReply;

  UINT32                    ReplyLength = sizeof(FWU_GET_VERSION_MSG_REPLY);

  DEBUG((EFI_D_INFO, "####HeciSendFwuGetVersion +++\n"));
  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  Status = HeciHostToSecFlowControl(SecAddress);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "###HeciSendFwuGetVersion: flow control fail.\n"));
  }

  Msg.MessageType = FWU_GET_VERSION;
  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&Msg,
                   sizeof(FWU_GET_VERSION_MSG),
                   BIOS_FIXED_HOST_ADDR + 1,
                   SecAddress
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuGetVersion Error: Failed to send FWU_GET_VERSION_MSG.\n"));
    return Status;
  }

  ZeroMem(&MsgReply, sizeof(FWU_GET_VERSION_MSG_REPLY));

  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&MsgReply,
                   &ReplyLength
                   );
  //
  //What we get might be a flow control message, or the get version reply. Treat accordingly.
  //
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "HeciSendFwuGetVersionMsg failed with ReadMsg #1, Status is: %d.\n", Status));
    return EFI_ABORTED;
  }

  if(ReplyLength == 8) {
    //
    //Got a flow control message. proceed to get the FWU version message
    //
    ZeroMem(&MsgReply, sizeof(FWU_GET_VERSION_MSG_REPLY));
    ReplyLength = sizeof(FWU_GET_VERSION_MSG_REPLY);
    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)&MsgReply,
                     &ReplyLength
                     );
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "HeciSendFwuGetVersionMsg failed with ReadMsg #2, Status is: %d.\n", Status));
      return EFI_ABORTED;
    } else {
      if(MsgReply.MessageType == FWU_GET_VERSION_REPLY) {
        CopyMem(Version, &MsgReply.CodeVersion, sizeof(VERSION));
        return EFI_SUCCESS;
      } else {
        DEBUG((EFI_D_ERROR, "HeciSendFwuGetVersionMsg: the readmsg after flow control does not return fwu version reply.\n"));
        return EFI_ABORTED;
      }

    }
  } else {
    //
    //It's get version reply already. Get the version information and then retrieve the flow control msg from SEC.
    //
    if(MsgReply.MessageType == FWU_GET_VERSION_REPLY) {
      CopyMem(Version, &MsgReply.CodeVersion, sizeof(VERSION));
      Status = HeciSecToHostFlowControl();
      return Status;
    } else {
      //
      //Any exception, we mark this as failed.
      //
      return EFI_ABORTED;

    }

  }
}

/**
  Send OEM ID message to Sec for verifying through HECI.

  @param[in] OemId                OEM ID for verifying
  @param[in] SecAddress           Sec address

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciVerifyOemId(
  IN  OEM_UUID *OemId,
  IN  UINT8  SecAddress
  )
{
  EFI_STATUS                  Status;
  EFI_HECI_PROTOCOL           *Heci;
  FWU_VERIFY_OEMID_MSG        Msg;
  FWU_VERIFY_OEMID_MSG_REPLY  MsgReply;
  UINT32                      ReplyLength = sizeof(FWU_VERIFY_OEMID_MSG_REPLY);

  DEBUG((EFI_D_INFO, "####HeciVerifyOemIdMsg +++\n"));


  if(OemId == NULL) {
    DEBUG((EFI_D_ERROR, "OemID passed to HeciVerifyOemIdMsg cannot be NULL.\n"));
    return EFI_ABORTED;
  }

  Msg.MessageType = FWU_VERIFY_OEMID;
  CopyMem(&Msg.OemId, OemId, sizeof(OEM_UUID));
  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = HeciHostToSecFlowControl(SecAddress);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "###HeciVerifyOemIdMsg: flow control fail.\n"));
    return EFI_ABORTED;
  }
  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&Msg,
                   sizeof(FWU_VERIFY_OEMID_MSG),
                   BIOS_FIXED_HOST_ADDR + 1,
                   SecAddress
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuVerifyOemID Error: Failed to send FWU_GET_OEMID_MSG.\n"));
    return EFI_ABORTED;
  }

  ZeroMem(&MsgReply, sizeof(FWU_VERIFY_OEMID_MSG_REPLY));
  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&MsgReply,
                   &ReplyLength
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuVerifyOemID: first reply read failed.\n"));
    return EFI_ABORTED;
  }

  if(ReplyLength == 8) { //The first reply message we get is flow control from SEC.
    ZeroMem(&MsgReply, sizeof(FWU_VERIFY_OEMID_MSG_REPLY));
    ReplyLength = sizeof(FWU_VERIFY_OEMID_MSG_REPLY);

    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)&MsgReply,
                     &ReplyLength
                     );
    if(EFI_ERROR(Status) || MsgReply.MessageType != FWU_VERIFY_OEMID_REPLY || MsgReply.Status != 0) {
      DEBUG((EFI_D_ERROR, "FwuVerifyOemID: second reply for oemid failed.\n"));
      return EFI_ABORTED;
    } else {
      return EFI_SUCCESS;
    }

  } else { //the first reply message we get is likely to be OEM ID reply.
    if(MsgReply.MessageType == FWU_VERIFY_OEMID_REPLY && MsgReply.Status == 0) {
      //
      //Continue to recv flow control message
      //
      Status = HeciSecToHostFlowControl();
      return Status;
    } else {
      DEBUG((EFI_D_ERROR, "FwuVerifyOemID: first oemid reply is not valid.\n"));
      return EFI_ABORTED;
    }

  }

}


/**
  Send Local Firmware Update Data to firmware thru HECI

  @param[out]  OemId              The OEM ID returned by firmware.
  @param[in]  SecAddress          Dynamic sec address for FWU.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuGetOemIdMsg(
  OUT OEM_UUID *OemId,
  IN  UINT8    SecAddress
  )
{
  EFI_STATUS                  Status;
  EFI_HECI_PROTOCOL           *Heci;

  FWU_GET_OEMID_MSG           Msg;
  FWU_GET_OEMID_MSG_REPLY     MsgReply;

  UINT32                      ReplyLength = sizeof(FWU_GET_OEMID_MSG_REPLY);

  DEBUG((EFI_D_INFO, "####HeciSendFwuGetOemIdMsg +++\n"));


  if(OemId == NULL) {
    DEBUG((EFI_D_ERROR, "OemID passed to HeciSendGetOemIdMsg cannot be NULL.\n"));
    return EFI_ABORTED;
  }

  Msg.MessageType = FWU_GET_OEMID;
  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  Status = HeciHostToSecFlowControl(SecAddress);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "###HeciSendFwuGetOemIdMsg: flow control fail.\n"));
    return EFI_ABORTED;
  }
  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&Msg,
                   sizeof(FWU_GET_OEMID_MSG),
                   BIOS_FIXED_HOST_ADDR + 1,
                   SecAddress
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuGetOemID Error: Failed to send FWU_GET_OEMID_MSG.\n"));
    return EFI_ABORTED;
  }

  ZeroMem(&MsgReply, sizeof(FWU_GET_OEMID_MSG_REPLY));
  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&MsgReply,
                   &ReplyLength
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuGetOemID: first reply read failed.\n"));
    return EFI_ABORTED;
  }

  if(ReplyLength == 8) { //The first reply message we get is flow control from SEC.
    ZeroMem(&MsgReply, sizeof(FWU_GET_OEMID_MSG_REPLY));
    ReplyLength = sizeof(FWU_GET_OEMID_MSG_REPLY);

    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)&MsgReply,
                     &ReplyLength
                     );
    if(EFI_ERROR(Status) || MsgReply.MessageType != FWU_GET_OEMID_REPLY || MsgReply.Status != 0) {
      DEBUG((EFI_D_ERROR, "FwuGetOemID: second reply for oemid failed.\n"));
      return EFI_ABORTED;
    } else {
      CopyMem(OemId, &MsgReply.OemId, sizeof(OEM_UUID));
      return EFI_SUCCESS;
    }

  } else { //the first reply message we get is likely to be OEM ID reply.
    if(MsgReply.MessageType == FWU_GET_OEMID_REPLY && MsgReply.Status == 0) {
      CopyMem(OemId, &MsgReply.OemId, sizeof(OEM_UUID));

      //
      //Continue to recv flow control message
      //
      Status = HeciSecToHostFlowControl();
      return Status;
    } else {
      DEBUG((EFI_D_ERROR, "FwuGetOemID: first oemid reply is not valid.\n"));
      return EFI_ABORTED;
    }

  }

}

/**
  Send Local Firmware Update start message to firmware thru HECI. THe HECI link might be disconnectd passively
    by the SEC side due to unexpected error. Supports full update only.

  @param[in]  ImageLength          Firmware image length to be updated
  @param[in]  OemId                The OemId to be passed to firmware for verification
  @param[in]  SecAddress           Dynamic sec address for FWU connection
  @param[in]  MaxBufferSizeMaximum buffer size for replied message

  @exception EFI_UNSUPPORTED       Current Sec mode doesn't support this function
  @retval EFI_SUCCESS              Command succeeded
  @retval EFI_DEVICE_ERROR         HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT              HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuStartMsg(
  IN UINT32     ImageLength,
  IN OEM_UUID   *OemId,
  IN UINT8      SecAddress,
  IN UINT32     MaxBufferSize
)
{
  EFI_STATUS                    Status;
  EFI_HECI_PROTOCOL             *Heci;

  UINT8                         *RxBuffer = NULL;

  UINT32                        ReplyLength = sizeof(FWU_START_MSG_REPLY);

  FWU_START_MSG                 Msg;
  FWU_START_MSG_REPLY           *MsgPtr;


  Status = EFI_SUCCESS;

  RxBuffer = AllocateZeroPool(MaxBufferSize);
  if(RxBuffer == NULL) {
    DEBUG((EFI_D_ERROR, "Failed to allocate pool for receving message.\n"));
    return EFI_ABORTED;
  }

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    goto _exit;
  }


  Status = HeciHostToSecFlowControl(SecAddress);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "####HeciSendFwuStartMsg: Failed to send flow control to Sec.\n"));
    goto _exit;
  }
  //
  //For BXT UPD update, no password is required.
  //
  ZeroMem(&Msg, sizeof(FWU_START_MSG));

  Msg.MessageType = FWU_START;
  Msg.Length = ImageLength;
  Msg.UpdateEnvironment = 0;

  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&Msg,
                   sizeof(FWU_START_MSG),
                   BIOS_FIXED_HOST_ADDR + 1,
                   SecAddress
                   );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuStartMessage Error: Failed to send FWU_START_MSG.\n"));
    goto _exit;
  }

  MsgPtr = (FWU_START_MSG_REPLY*)RxBuffer;

  Status = Heci->ReadMsg(
             HECI1_DEVICE,
             BLOCKING,
             (UINT32*)MsgPtr,
             &ReplyLength
             );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuStartMessage Error: Failed to read 1st message from SEC.\n"));
    goto _exit;
  }
  //
  //Check if the replied message is flow control. Since the reply msg size of FWU_START is larger than 8 bytes, we can simply check the size
  //
  if(ReplyLength == 8) { //This is likely a flow control message from SEC. it might be disconnect msg as well.
    ZeroMem(MsgPtr, MaxBufferSize);
    ReplyLength = MaxBufferSize;

    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)MsgPtr,
                     &ReplyLength
                   );
    if(EFI_ERROR(Status) || MsgPtr->MessageType != FWU_START_REPLY || MsgPtr->Status != 0) {
      DEBUG((EFI_D_ERROR, "FwuStartMessage Error: Failed to get 2nd message as start reply. EFIStatus:%d ,StartReply code: %d.\n", Status, MsgPtr->Status));
      Status = EFI_ABORTED;
      goto _exit;
    }

  } else { //The first reply is actually the start reply
    if(MsgPtr->MessageType == FWU_START_REPLY && MsgPtr->Status == 0) {
      Status = HeciSecToHostFlowControl();
      goto _exit;
    } else {
      DEBUG((EFI_D_ERROR, "FwuStartMessage Error: Failed to get 2nd message as flow control from SEC.\n"));
      Status = EFI_ABORTED;
      goto _exit;
    }

  }

_exit:
  if(RxBuffer != NULL) {
    FreePool(RxBuffer);
    RxBuffer = NULL;
  }
  return Status;

}



/**
  Send Local Firmware Update Data to firmware thru HECI.

  @param[in]  FwuData             Firmware update data to be updated
  @param[in]  FwuDataSize         Firmware update data size to be passed to firmware
  @param[in]  SecAddress          Dynamic Sec Address used by FWU client
  @param[in]  MaxBufferSize       The max buffer length for FWU client allowed by SEC. Retrieved from the connect API.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuDataMsg(
  IN UINT8 * FwuData,
  IN UINT32 FwuDataSize,
  IN UINT8  SecAddress,
  IN UINT32 MaxBufferSize
  )
{

  EFI_STATUS        Status;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            AccumulatedBytesRead = 0;
  UINT32            BytesCopied = 0;
  UINT32            BufPtr = 0;

  UINT8             *TxBuffer = NULL;
  UINT32            ReplyLength = sizeof(FWU_DATA_MSG_REPLY);
  UINT32            LoopIndex = 0;

  FWU_DATA_MSG         *Msg;
  FWU_DATA_MSG_REPLY   MsgReply;
  UINT8                *DataField;

  Status = EFI_SUCCESS;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  //
  //Msg buffer is assumed 512 bytes. check to make sure it's at least 256 bytes. This is a restriction from BXT BIOS for performance, not interface restriction.
  //
  if(MaxBufferSize < 256) {
    DEBUG((EFI_D_ERROR, "####FwuStart Error: BXT FWU HECI interface requires at least 256 byte for FWU_DATA.\n"));
    return EFI_ABORTED;
  }


  TxBuffer = AllocateZeroPool(MaxBufferSize);
  if(TxBuffer == NULL) {
    DEBUG((EFI_D_ERROR, "FwuDataMessage Error: Failed to allocate buffer.\n"));
    return EFI_ABORTED;
  }


  Msg = (FWU_DATA_MSG*)TxBuffer;
  Msg->MesageType = FWU_DATA;
  DataField = &(Msg->Data[0]);

  //
  //For MTU that's 512 bytes, does it include the Heci header?  If it does include the heci header, then
  //For 512 byte message, at least 512/16=32 bytes are for Heci header.  actually there may be one more Heci header
  //so at least reserve 36 bytes for Heci headers, plus 4 bytes FWU_DATA message header. This should be enough
  //reservation in any case.
  //

  BytesCopied = MaxBufferSize - sizeof(FWU_DATA_MSG);
  while(AccumulatedBytesRead < FwuDataSize) {
    DEBUG((EFI_D_ERROR, "####Loop Index:%d.\n", LoopIndex));

    if(BufPtr + BytesCopied > FwuDataSize) {  //Make sure the right last bytes are copied.
      BytesCopied = FwuDataSize - BufPtr;
    }

    Status = HeciHostToSecFlowControl(SecAddress);
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "####FwuStart Error: Failed to send flow control to SEC.\n"));
      return Status;
    }

    CopyMem(DataField, FwuData + BufPtr, BytesCopied);
    AccumulatedBytesRead += BytesCopied;

    BufPtr = AccumulatedBytesRead;

    //
    //Send the data over and gets the 1st reply.
    //
    Msg->Length = BytesCopied;
    ZeroMem(&MsgReply, sizeof(FWU_DATA_MSG_REPLY));

    Status = Heci->SendMsg(
                     HECI1_DEVICE,
                     (UINT32*)Msg,
                     sizeof(FWU_DATA_MSG) + BytesCopied - 1,
                     BIOS_FIXED_HOST_ADDR + 1,
                     SecAddress
                     );

    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "####FwuDataMessage Error: Failed to send FWU_DATA_MSG.\n"));
      if(TxBuffer != NULL) {
        FreePool(TxBuffer);
        TxBuffer = NULL;
      }
      return EFI_ABORTED;
    }

    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)&MsgReply,
                     &ReplyLength
                     );
    if(EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "####FwuDataMessage Error: Failed to recv 1st reply from SEC.\n"));
      if(TxBuffer != NULL) {
        FreePool(TxBuffer);
        TxBuffer = NULL;
      }

      return Status;
    }


    //
    //Handle seperately according to whether the msg is flow control reply or fwu data reply. Since the size of both are 8 bytes, we check the MessageType directly.
    //

    if(MsgReply.MessageType == FWU_DATA_REPLY) { //This is data reply
      if(MsgReply.Status != 0) {
        DEBUG((EFI_D_ERROR, "####FwuDataMessage Error: FWU_DATA_REPLY message indicates status of non-zero.\n"));
        if(TxBuffer != NULL) {
          FreePool(TxBuffer);
          TxBuffer = NULL;
        }
        return EFI_ABORTED;
      }
      Status = HeciSecToHostFlowControl();
      if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR, "####FwuDataMessage Error: Failed to recv 2nd message as flow control from SEC.\n"));
        if(TxBuffer != NULL) {
          FreePool(TxBuffer);
          TxBuffer = NULL;
        }
        return EFI_ABORTED;

      }

    } else { //This is flow control:
      ZeroMem(&MsgReply, sizeof(FWU_DATA_MSG_REPLY));
      ReplyLength = sizeof(FWU_DATA_MSG_REPLY);

      Status = Heci->ReadMsg(
                       HECI1_DEVICE,
                       BLOCKING,
                       (UINT32*)&MsgReply,
                       &ReplyLength
                       );


      if(EFI_ERROR(Status) || MsgReply.Status != 0) {
        DEBUG((EFI_D_ERROR, "####FwuDataMessage Error: Failed to recv 2nd data msg reply, Status is:%d, ReplyStatus is :%d.\n", Status, MsgReply.Status));
        if(TxBuffer != NULL) {
          FreePool(TxBuffer);
          TxBuffer = NULL;
        }
        return EFI_ABORTED;
      }

    }
    DEBUG((EFI_D_INFO, "####End loop #%d., cursor at:%d\n", LoopIndex, AccumulatedBytesRead));
    LoopIndex++;
  }


  if(TxBuffer != NULL) {
    FreePool(TxBuffer);
    TxBuffer = NULL;
  }

  return EFI_SUCCESS;

}


/**
  Send Local Firmware Update Data to firmware thru HECI

  @param[out]  ResetType          Reset type required for finishing firmware update.
  @param[in]  SecAddress          Dynamic Sec Address used by FWU client

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendFwuEndMsg(
  OUT UINT32 *ResetType,
  IN UINT8 SecAddress
  )
{
  EFI_STATUS                Status;
  EFI_HECI_PROTOCOL         *Heci;
  FWU_END_MSG               Msg;
  FWU_END_MSG_REPLY         MsgReply;
  UINT32                    ReplyLength;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR(Status)){
    return Status;
  }

  Status = HeciHostToSecFlowControl(SecAddress);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "####FwuEndMessage Error: Failed to send flow control to Sec.\n"));
    return Status;
  }


  Msg.MessageType = FWU_END;

  Status = Heci->SendMsg(
                   HECI1_DEVICE,
                   (UINT32*)&Msg,
                   sizeof(FWU_END_MSG),
                   BIOS_FIXED_HOST_ADDR + 1,
                   SecAddress
                   );

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuEndMessage Error: Failed to send FWU_DATA_MSG.\n"));
    return EFI_ABORTED;
  }

  Status = Heci->ReadMsg(
                   HECI1_DEVICE,
                   BLOCKING,
                   (UINT32*)&MsgReply,
                   &ReplyLength
                   );

  //
  //Handle flow control and fwu end message. Since FWU_END message will take very long to arrive,
  //use MKHI group 06 cmd to track the status before receiving the FWU_END reply message
  //

  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "FwuEndMessage Erro: Get 1st reply failed.\n"));
    return Status;
  }


  if(ReplyLength == 8) { //This is flow control msg
    ZeroMem(&MsgReply, sizeof(FWU_END_MSG_REPLY));
    ReplyLength = sizeof(FWU_END_MSG_REPLY);

    Status = Heci->ReadMsg(
                     HECI1_DEVICE,
                     BLOCKING,
                     (UINT32*)&MsgReply,
                     &ReplyLength
                     );

    if(EFI_ERROR(Status) || MsgReply.MessageType != FWU_END_REPLY || MsgReply.Status != 0) {
      DEBUG((EFI_D_ERROR, "FwuEndMessage Error: Failed to get second msg as fwu end reply from SEC.\n"));
      return EFI_ABORTED;
    } else {
      Print(L"Get FWU_END reply. the reset type is:%d.\n", MsgReply.ResetType);
      *ResetType = MsgReply.ResetType;
    }
  } else { //This is already fwu end message reply
    if(MsgReply.MessageType == FWU_END_REPLY && MsgReply.Status == 0) {
      Status = HeciSecToHostFlowControl();
      return Status;
    } else {
      DEBUG((EFI_D_ERROR, "FwuEndMessage Error: Failed to get first msg as fwuend reply from SEC.\n"));
      return EFI_ABORTED;
    }

  }

  return EFI_SUCCESS;

}


/**
  Send Hardware Asset Tables to firmware through HECI.

  @param[in]  AssetTableData      Hardware Asset Table Data
  @param[in]  TablesDataSize      Size of Asset table

  @retval EFI_SUCCESS             Table sent
  @retval EFI_ABORTED             Could not allocate Memory
**/
EFI_STATUS
HeciAssetUpdateFwMsg (
  IN TABLE_PUSH_DATA *AssetTableData,
  IN UINT16          TableDataSize
  )
{
  AU_TABLE_PUSH_MSG *SendAssetTableDataMsg;
  EFI_STATUS        Status;
  EFI_HECI_PROTOCOL *Heci;

  Status = EFI_SUCCESS;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Subtract off single byte from TABLE_PUSH_DATA.TableData[1]
  //
  SendAssetTableDataMsg = AllocateZeroPool (sizeof (AU_TABLE_PUSH_MSG) + MAX_ASSET_TABLE_ALLOCATED_SIZE - 1);
  if (SendAssetTableDataMsg == NULL) {
    DEBUG ((EFI_D_ERROR, "AssetUpdateFwMsg Error: Could not allocate Memory\n"));
    return EFI_ABORTED;
  }

  if (TableDataSize > MAX_ASSET_TABLE_ALLOCATED_SIZE) {
    TableDataSize = MAX_ASSET_TABLE_ALLOCATED_SIZE;
  }

  SendAssetTableDataMsg->Header.Data                  = 0;
  //
  // Subtract off single byte from TABLE_PUSH_DATA.TableData[1]
  //
  SendAssetTableDataMsg->Header.Fields.MessageLength  = TableDataSize + sizeof (TABLE_PUSH_DATA) - 1;
  SendAssetTableDataMsg->Header.Fields.Command = HWA_TABLE_PUSH_CMD;

  CopyMem (&SendAssetTableDataMsg->Data, AssetTableData, SendAssetTableDataMsg->Header.Fields.MessageLength);

  Status = Heci->SendMsg (
                   HECI1_DEVICE,
                   (UINT32 *) SendAssetTableDataMsg,
                   SendAssetTableDataMsg->Header.Fields.MessageLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HWA_CLIENT_ID
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "AssetUpdateFwMsg: Failed to Send SendAssetTableDataMsg\n"));

  }

  FreePool (SendAssetTableDataMsg);

  return Status;

}

/**
  Send End of Post Request Message through HECI.

  @param VOID                     None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendEndOfPostMessage (
  VOID
  )

{
  EFI_STATUS                      Status;
  UINT32                          HeciSendLength;
  UINT32                          HeciRecvLength;
  GEN_END_OF_POST_ACK             CbmEndOfPost;
  UINT32                          SeCMode;
  EFI_HECI_PROTOCOL               *Heci;
  SC_RESET_PROTOCOL              *ScResetProtocol;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateProtocol (&gScResetProtocolGuid, NULL, (VOID **) &ScResetProtocol);

  if (EFI_ERROR (Status)) {
    ScResetProtocol = NULL;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  CbmEndOfPost.Header.Data              = 0;
  CbmEndOfPost.Header.Fields.Command    = CBM_END_OF_POST_CMD;
  CbmEndOfPost.Header.Fields.IsResponse = 0;
  CbmEndOfPost.Header.Fields.GroupId    = MKHI_GEN_GROUP_ID;
  CbmEndOfPost.Data.RequestedActions    = 0;

  HeciSendLength                        = sizeof (MKHI_MESSAGE_HEADER);
  HeciRecvLength                        = sizeof (GEN_END_OF_POST_ACK);

  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &CbmEndOfPost,
                  HeciSendLength,
                  &HeciRecvLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );

  if (CbmEndOfPost.Data.RequestedActions == HECI_EOP_PERFORM_GLOBAL_RESET) {
    if (ScResetProtocol != NULL) {
      DEBUG ((EFI_D_ERROR, "HeciSendEndOfPostMessage(): Reset requested by FW EOP ACK %r\n"));
      ScResetProtocol->Reset (ScResetProtocol, GlobalReset);
    }
  }

  return Status;
}

/**
  Send Get Firmware SKU Request to Sec

  @param[in,out] MsgGenGetFwCapsSku     Return message for Get Firmware Capability SKU
  @param[in,out] MsgGenGetFwCapsSkuAck  Return message for Get Firmware Capability SKU ACK

  @exception EFI_UNSUPPORTED        Current Sec mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetFwCapsSkuMsg (
  IN OUT GEN_GET_FW_CAPSKU       *MsgGenGetFwCapsSku,
  IN OUT GEN_GET_FW_CAPS_SKU_ACK *MsgGenGetFwCapsSkuAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetFwCapsSku->MKHIHeader.Data               = 0;
  MsgGenGetFwCapsSku->MKHIHeader.Fields.GroupId     = MKHI_FWCAPS_GROUP_ID;
  MsgGenGetFwCapsSku->MKHIHeader.Fields.Command     = FWCAPS_GET_RULE_CMD;
  MsgGenGetFwCapsSku->MKHIHeader.Fields.IsResponse  = 0;
  MsgGenGetFwCapsSku->Data.RuleId                   = 0;
  Length = sizeof (GEN_GET_FW_CAPSKU);

  //
  // Send Get FW SKU Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenGetFwCapsSku,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_FW_CAPS_SKU_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenGetFwCapsSkuAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Send Get Firmware Version Request to Sec

  @param[in,out] MsgGenGetFwVersionAck   Return themessage of FW version

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetFwVersionMsg (
  IN OUT GEN_GET_FW_VER_ACK     *MsgGenGetFwVersionAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_GET_FW_VER    *MsgGenGetFwVersion;
  GEN_GET_FW_VER    GenGetFwVersion;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenGetFwVersion data structure
  //
  MsgGenGetFwVersion = &GenGetFwVersion;
  MsgGenGetFwVersion->MKHIHeader.Data = 0;
  MsgGenGetFwVersion->MKHIHeader.Fields.GroupId = MKHI_GEN_GROUP_ID;
  MsgGenGetFwVersion->MKHIHeader.Fields.Command = GEN_GET_FW_VERSION_CMD;
  MsgGenGetFwVersion->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GET_FW_VER);
  //
  // Send Get Firmware Version Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenGetFwVersion,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_FW_VER_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenGetFwVersionAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Get AT State Information From FW

  @param[in,out]  AtStateInfo     Pointer to structure to hold AT FW Data

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciGetAtFwStateInfoMsg (
  IN OUT AT_STATE_STRUCT       *AtStateInfo
  )
{
  EFI_STATUS          Status;
  UINT32              Length;
  EFI_HECI_PROTOCOL   *Heci;
  UINT32              SeCMode;
  GET_TDT_SEC_RULE_CMD SeCTdtRuleCmd;
  GET_TDT_SEC_RULE_RSP SeCTdtRuleRsp;

  SetMem ((VOID *) &SeCTdtRuleCmd, sizeof (GET_TDT_SEC_RULE_CMD), 0);
  SetMem ((VOID *) &SeCTdtRuleRsp, sizeof (GET_TDT_SEC_RULE_RSP), 0);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  SeCTdtRuleCmd.MKHIHeader.Data              = 0;
  SeCTdtRuleCmd.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  SeCTdtRuleCmd.MKHIHeader.Fields.Command    = FWCAPS_GET_RULE_CMD;
  SeCTdtRuleCmd.MKHIHeader.Fields.IsResponse = 0;
  SeCTdtRuleCmd.RuleId                       = TDT_SEC_RULE_ID;
  Length = sizeof (GET_TDT_SEC_RULE_CMD);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &SeCTdtRuleCmd,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetTdtSeCRule failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  Length = sizeof (GET_TDT_SEC_RULE_RSP);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &SeCTdtRuleRsp,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetTdtSeCRule failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  if (SeCTdtRuleRsp.MKHIHeader.Fields.Result) {
    DEBUG ((EFI_D_ERROR, "HeciGetAtFwStateInfoMsg::Error - Non-zero MKHI Result code from FW, Status = %r\n", Status));
    return EFI_DEVICE_ERROR;
  }

  AtStateInfo->AtState            = SeCTdtRuleRsp.TdtRuleData.State;
  AtStateInfo->AtLastTheftTrigger = SeCTdtRuleRsp.TdtRuleData.LastTheftTrigger;
  AtStateInfo->AtLockState        = SeCTdtRuleRsp.TdtRuleData.flags.LockState;
  AtStateInfo->AtAmPref           = SeCTdtRuleRsp.TdtRuleData.flags.AuthenticateModule;

  return Status;
}

/**
  Send Thermal Reporting message through HECI.

  @param[in]  Heci                The pointer of Heci protocol
  @param[in]  TrConfig            Thermal Reporting Configuration Setting

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciTrConfigMsg (
  IN  EFI_HECI_PROTOCOL  *Heci,
  IN  TR_CONFIG          *TrConfig
  )
{
  EFI_STATUS            Status;
  TR_BIOS_PARAM_REQUEST *TrBiosParamRequest;
  UINT8                 *SmbusAddress;
  UINT32                HeciLength;
  UINT8                 Index;
  UINT8                 *TrBiosParamRequestBuffer;
  UINT32                SeCMode;

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  TrBiosParamRequestBuffer            = (UINT8 *) AllocatePool (sizeof (TR_BIOS_PARAM_REQUEST) + TrConfig->DimmNumber);
  if (TrBiosParamRequestBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  TrBiosParamRequest                  = (TR_BIOS_PARAM_REQUEST *) TrBiosParamRequestBuffer;
  SmbusAddress                        = (UINT8 *) (TrBiosParamRequestBuffer + sizeof (TR_BIOS_PARAM_REQUEST));

  TrBiosParamRequest->Command         = TR_HECI_CONFIG_MSG_CMD_ID;
  TrBiosParamRequest->PollingTimeout  = TIME_200MS;
  //
  // SMBus Block Read message length for EC. The possible values are 1, 2, 5, 9, 10, 14 or 20.
  //
  TrBiosParamRequest->SMBusECMsgLen = TrConfig->SMBusECMsgLen;
  TrBiosParamRequest->SMBusECMsgPEC = TrConfig->SMBusECMsgPEC;
  TrBiosParamRequest->DimmNumber    = TrConfig->DimmNumber;

  for (Index = 0; Index < TrConfig->DimmNumber; Index++) {
    SmbusAddress[Index] = TrConfig->SmbusAddress[Index];
  }

  HeciLength = sizeof (TR_BIOS_PARAM_REQUEST) + TrConfig->DimmNumber;

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) TrBiosParamRequestBuffer,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TR_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Send Thermal Reporting message - %r\n", Status));
  }

  return Status;
}

/**
  Sends a message to Sec to unlock a specified SPI Flash region for writing and receiving a response message.
  It is recommended that HMRFPO_ENABLE HECI message needs to be sent after all OROMs finish their initialization.

  @param[in] Nonce                Nonce received in previous HMRFPO_ENABLE Response Message
  @param[out] Result              HMRFPO_ENABLE response

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoEnable (
  IN  UINT64                          Nonce,
  OUT UINT8                           *Result
  )
{
  EFI_STATUS                  Status;
  EFI_HECI_PROTOCOL           *Heci;
  MKHI_HMRFPO_ENABLE          HmrfpoEnableRequest;
  MKHI_HMRFPO_ENABLE_RESPONSE HmrfpoEnableResponse;
  UINT32                      HeciLength;
  UINT32                      SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  HmrfpoEnableRequest.MkhiHeader.Data               = 0;
  HmrfpoEnableRequest.MkhiHeader.Fields.GroupId     = MKHI_SPI_GROUP_ID;
  HmrfpoEnableRequest.MkhiHeader.Fields.Command     = HMRFPO_ENABLE_CMD_ID;
  HmrfpoEnableRequest.MkhiHeader.Fields.IsResponse  = 0;
  HmrfpoEnableRequest.Nonce                         = Nonce;

  HeciLength = sizeof (MKHI_HMRFPO_ENABLE);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &HmrfpoEnableRequest,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Send HMRFPO_ENABLE_CMD_ID Request - %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (MKHI_HMRFPO_ENABLE_RESPONSE);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &HmrfpoEnableResponse,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Read HMRFPO_ENABLE_CMD_ID Result - %r\n", Status));
    return Status;
  }

  *Result = HmrfpoEnableResponse.Status;

  return Status;
}

/**
  Sends a message to Sec to lock a specified SPI Flash region for writing and receiving a response message.

  @param[out] Nonce               Random number generated by Ignition Sec FW. When BIOS
                                  want to unlock region it should use this value
                                  in HMRFPO_ENABLE Request Message
  @param[out] FactoryDefaultBase  The base of the factory default calculated from the start of the ME region.
                                  BIOS sets a Protected Range (PR) register "Protected Range Base" field with this value
                                  + the base address of the region.
  @param[out] FactoryDefaultLimit The length of the factory image.
                                  BIOS sets a Protected Range (PR) register "Protected Range Limit" field with this value
  @param[out] Result              Status report

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoLock (
  OUT UINT64                          *Nonce,
  OUT UINT32                          *FactoryDefaultBase,
  OUT UINT32                          *FactoryDefaultLimit,
  OUT UINT8                           *Result
  )
{
  EFI_STATUS                Status;
  EFI_HECI_PROTOCOL         *Heci;
  MKHI_HMRFPO_LOCK          HmrfpoLockRequest;
  MKHI_HMRFPO_LOCK_RESPONSE HmrfpoLockResponse;
  UINT32                    HeciLength;
  UINT32                    SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  HmrfpoLockRequest.MkhiHeader.Data               = 0;
  HmrfpoLockRequest.MkhiHeader.Fields.GroupId     = MKHI_SPI_GROUP_ID;
  HmrfpoLockRequest.MkhiHeader.Fields.Command     = HMRFPO_LOCK_CMD_ID;
  HmrfpoLockRequest.MkhiHeader.Fields.IsResponse  = 0;

  HeciLength = sizeof (MKHI_HMRFPO_LOCK);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &HmrfpoLockRequest,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Send HMRFPO_LOCK_CMD_ID Request - %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (MKHI_HMRFPO_LOCK_RESPONSE);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &HmrfpoLockResponse,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to read HMRFPO_LOCK_CMD_ID response - %r.\n", Status));
    return Status;
  }

  *Nonce                = HmrfpoLockResponse.Nonce;
  *FactoryDefaultBase   = HmrfpoLockResponse.FactoryDefaultBase;
  *FactoryDefaultLimit  = HmrfpoLockResponse.FactoryDefaultLimit;
  *Result               = HmrfpoLockResponse.Status;

  return Status;
}

/**
  System BIOS sends this message to get status for HMRFPO_LOCK message.

  @param[out] Result              HMRFPO_GET_STATUS response

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoGetStatus (
  OUT UINT8                           *Result
  )
{
  EFI_STATUS                      Status;
  EFI_HECI_PROTOCOL               *Heci;
  MKHI_HMRFPO_GET_STATUS          HmrfpoGetStatusRequest;
  MKHI_HMRFPO_GET_STATUS_RESPONSE HmrfpoGetStatusResponse;
  UINT32                          HeciLength;
  UINT32                          SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  HmrfpoGetStatusRequest.MkhiHeader.Data              = 0;
  HmrfpoGetStatusRequest.MkhiHeader.Fields.GroupId    = MKHI_SPI_GROUP_ID;
  HmrfpoGetStatusRequest.MkhiHeader.Fields.Command    = HMRFPO_GET_STATUS_CMD_ID;
  HmrfpoGetStatusRequest.MkhiHeader.Fields.IsResponse = 0;

  HeciLength = sizeof (MKHI_HMRFPO_GET_STATUS);


  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &HmrfpoGetStatusRequest,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Send HMRFPO_GET_STATUS_CMD_ID - %r\n", Status));
    return Status;
  }
  
//[-start-160921-IB07400787-add]//
  ZeroMem ((VOID*)&HmrfpoGetStatusResponse, sizeof (MKHI_HMRFPO_GET_STATUS_RESPONSE));
//[-end-160921-IB07400787-add]//

  HeciLength = sizeof (MKHI_HMRFPO_GET_STATUS_RESPONSE);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &HmrfpoGetStatusResponse,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Unable to Read HMRFPO_GET_STATUS_CMD_ID Result - %r\n", Status));
  }

  *Result = HmrfpoGetStatusResponse.Status;

  return Status;
}

/**
  System BIOS sends this message to get status for HMRFPO_LOCK message.

  @param[in]  QueryType           QueryType:
                                    0 - Query Request
                                    1 - Cancel Request
  @param[out] Result              HMRFPO_GET_STATUS response
                                    1h - Continue, KVM session established.
                                    2h - Continue, KVM session cancelled.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciQueryKvmRequest (
  IN  UINT32                         QueryType,
  OUT UINT32                         *ResponseCode
  )
{
  EFI_STATUS              Status;
  EFI_HECI_PROTOCOL       *Heci;
  AMT_QUERY_KVM_REQUEST   QueryKvmRequest;
  AMT_QUERY_KVM_RESPONSE  QueryKvmResponse;
  UINT32                  HeciLength;
  UINT16                  TimeOut;
  UINT32                  SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  QueryKvmRequest.Command       = EFI_KVM_MESSAGE_COMMAND;
  QueryKvmRequest.ByteCount     = EFI_KVM_BYTE_COUNT;
  QueryKvmRequest.SubCommand    = EFI_KVM_QUERY_REQUES;
  QueryKvmRequest.VersionNumber = EFI_KVM_VERSION;
  QueryKvmRequest.QueryType     = QueryType;

  HeciLength                    = sizeof (AMT_QUERY_KVM_REQUEST);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &QueryKvmRequest,
                  HeciLength,
                  BIOS_ASF_HOST_ADDR,
                  HECI_ASF_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Query KVM failed %r\n", Status));
  }

  TimeOut     = 0;
  HeciLength  = sizeof (AMT_QUERY_KVM_RESPONSE);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &QueryKvmResponse,
                  &HeciLength
                  );

  if (QueryType == QUERY_REQUEST) {
    while (EFI_ERROR (Status)) {
      gBS->Stall (EFI_KVM_STALL_1_SECOND);
      TimeOut++;

      if (TimeOut > EFI_KVM_MAX_WAIT_TIME) {
        break;
      }

      HeciLength = sizeof (AMT_QUERY_KVM_RESPONSE);
      Status = Heci->ReadMsg (
                      HECI1_DEVICE,
                      NON_BLOCKING,
                      (UINT32 *) &QueryKvmResponse,
                      &HeciLength
                      );
    }
  }

  *ResponseCode = QueryKvmResponse.ResponseCode;

  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to query the local firmware update interface status.

  @param[out] RuleData            1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetLocalFwUpdate (
  OUT UINT32         *RuleData
  )
{
  EFI_STATUS                  Status;
  UINT32                      Length;
  EFI_HECI_PROTOCOL           *Heci;
  GEN_GET_LOCAL_FW_UPDATE     MsgGenGetLocalFwUpdate;
  GEN_GET_LOCAL_FW_UPDATE_ACK MsgGenGetLocalFwUpdatekuAck;
  UINT32                      SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetLocalFwUpdate.MKHIHeader.Data              = 0;
  MsgGenGetLocalFwUpdate.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  MsgGenGetLocalFwUpdate.MKHIHeader.Fields.Command    = FWCAPS_GET_RULE_CMD;
  MsgGenGetLocalFwUpdate.MKHIHeader.Fields.IsResponse = 0;
  MsgGenGetLocalFwUpdate.Data.RuleId                  = 7;
  Length = sizeof (GEN_GET_LOCAL_FW_UPDATE);

  //
  // Send Get Local FW update Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgGenGetLocalFwUpdate,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_LOCAL_FW_UPDATE_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgGenGetLocalFwUpdatekuAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  *RuleData = MsgGenGetLocalFwUpdatekuAck.Data.RuleData;

  return Status;
}


/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to enable or disable the local firmware update interface.
  The firmware allows a single update once it receives the enable command

  @param[in] RuleData             1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciSetLocalFwUpdate (
  IN UINT8         RuleData
  )
{
  EFI_STATUS                  Status;
  UINT32                      Length;
  EFI_HECI_PROTOCOL           *Heci;
  GEN_SET_LOCAL_FW_UPDATE     MsgGenSetLocalFwUpdate;
  GEN_SET_LOCAL_FW_UPDATE_ACK MsgGenSetLocalFwUpdateAck;
  UINT32                      SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenSetLocalFwUpdate.MKHIHeader.Data              = 0;
  MsgGenSetLocalFwUpdate.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  MsgGenSetLocalFwUpdate.MKHIHeader.Fields.Command    = FWCAPS_SET_RULE_CMD;
  MsgGenSetLocalFwUpdate.MKHIHeader.Fields.IsResponse = 0;
  MsgGenSetLocalFwUpdate.Data.RuleId                  = 7;
  MsgGenSetLocalFwUpdate.Data.RuleDataLen             = 4;
  MsgGenSetLocalFwUpdate.Data.RuleData                = RuleData;
  Length = sizeof (GEN_SET_LOCAL_FW_UPDATE);

  //
  // Send Get Local FW update Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgGenSetLocalFwUpdate,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_SET_LOCAL_FW_UPDATE_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgGenSetLocalFwUpdateAck,
                  &Length
                  );

  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to enable the Sec State. The firmware allows a single
  update once it receives the enable command. Once firmware receives this message,
  the firmware will be in normal mode after a global reset.

  @param[in] None

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             ME enabled message sent
**/
EFI_STATUS
HeciSetSeCEnableMsg (
  IN VOID
  )
{
  EFI_STATUS            Status;
  EFI_HECI_PROTOCOL     *Heci;
  HECI_FWS_REGISTER     SeCFirmwareStatus;
  UINTN                 HeciPciAddressBase;
  UINT16                TimeOut;

  TimeOut = 0;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HeciPciAddressBase = PCI_LIB_ADDRESS (
                         SEC_BUS,
                         SEC_DEVICE_NUMBER,
                         HECI_FUNCTION_NUMBER,
                         0
                         );
  PciWrite8 (HeciPciAddressBase + R_GEN_STS + 3, 0x20);
  do {
    SeCFirmwareStatus.ul = PciRead32 (HeciPciAddressBase + R_SEC_FW_STS0);
    gBS->Stall (EFI_SEC_STATE_STALL_1_SECOND);
    TimeOut++;
  } while ((SeCFirmwareStatus.r.FwInitComplete != SEC_FIRMWARE_COMPLETED) && (TimeOut > EFI_SEC_STATE_MAX_TIMEOUT));


  Status = HeciSetSeCDisableMsg (1);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "HeciSetSeCDisableMsg Status = %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  // Send Global reset
  HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
  CpuDeadLoop ();
  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to disable the Sec State. The firmware allows a single
  update once it receives the disable command Once firmware receives this message,
  the firmware will work in "Soft Temporary Disable" mode (HFS[19:16] = 3) after a
  global reset. Note, this message is not allowed when AT firmware is enrolled/configured.

  @param[in] RuleData             Rule data
                                    0 - Disable
                                    1 - Enable

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Sec is disabled
**/
EFI_STATUS
HeciSetSeCDisableMsg (
  IN UINT8 ruleData
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  EFI_HECI_PROTOCOL *Heci;
  GEN_SET_FW_CAPSKU MsgSeCStateControl;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgSeCStateControl.MKHIHeader.Data               = 0;
  MsgSeCStateControl.MKHIHeader.Fields.GroupId     = MKHI_FWCAPS_GROUP_ID;
  MsgSeCStateControl.MKHIHeader.Fields.Command     = FWCAPS_SET_RULE_CMD;
  MsgSeCStateControl.MKHIHeader.Fields.IsResponse  = 0;
  MsgSeCStateControl.Data.RuleId.Data              = 6;
  MsgSeCStateControl.Data.RuleDataLen              = 4;
  MsgSeCStateControl.Data.RuleData                 = ruleData;

  Length = sizeof (GEN_SET_FW_CAPSKU);

  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgSeCStateControl,
                  Length,
                  &Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );

  HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
  CpuDeadLoop ();
  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get platform type.
  One of usages is to utilize this command to determine if the platform runs in
  Consumer or Corporate SKU size firmware.

  @param[out] RuleData            PlatformBrand,
                                  IntelMeFwImageType,
                                  SuperSku,
                                  PlatformTargetUsageType

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetPlatformTypeMsg (
  OUT PLATFORM_TYPE_RULE_DATA   *RuleData
  )
{
  EFI_STATUS                Status;
  UINT32                    Length;
  EFI_HECI_PROTOCOL         *Heci;
  GEN_GET_PLATFORM_TYPE     MsgGenGetPlatformType;
  GEN_GET_PLATFORM_TYPE_ACK MsgGenGetPlatformTypeAck;
  UINT32                    SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetPlatformType.MKHIHeader.Data               = 0;
  MsgGenGetPlatformType.MKHIHeader.Fields.GroupId     = MKHI_FWCAPS_GROUP_ID;
  MsgGenGetPlatformType.MKHIHeader.Fields.Command     = FWCAPS_GET_RULE_CMD;
  MsgGenGetPlatformType.MKHIHeader.Fields.IsResponse  = 0;
  MsgGenGetPlatformType.Data.RuleId                   = 0x1D;
  Length = sizeof (GEN_GET_PLATFORM_TYPE);

  //
  // Send Get Platform Type Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgGenGetPlatformType,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_PLATFORM_TYPE_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgGenGetPlatformTypeAck,
                  &Length
                  );

  *RuleData = MsgGenGetPlatformTypeAck.Data.RuleData;

  return Status;
}

/**
  This message is sent by the BIOS on the boot where the host wants to get the firmware provisioning state.
  The firmware will respond to AMT BIOS SYNCH INFO message even after the End of Post.

  @param[out] RuleData            Bit [2:0] Reserved
                                  Bit [4:3] Provisioning State
                                    00 - Pre -provisioning
                                    01 - In -provisioning
                                    02 - Post !Vprovisioning
                                  Bit [31:5] Reserved

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Firmware provisioning state returned
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciAmtBiosSynchInfo (
  OUT UINT32         *RuleData
  )
{
  EFI_STATUS                  Status;
  UINT32                      Length;
  EFI_HECI_PROTOCOL           *Heci;
  GEN_AMT_BIOS_SYNCH_INFO     MsgGenAmtBiosSynchInfo;
  GEN_AMT_BIOS_SYNCH_INFO_ACK MsgGenAmtBiosSynchInfoAck;
  UINT32                      SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenAmtBiosSynchInfo.MKHIHeader.Data              = 0;
  MsgGenAmtBiosSynchInfo.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  MsgGenAmtBiosSynchInfo.MKHIHeader.Fields.Command    = FWCAPS_GET_RULE_CMD;
  MsgGenAmtBiosSynchInfo.MKHIHeader.Fields.IsResponse = 0;
  MsgGenAmtBiosSynchInfo.Data.RuleId                  = 0x30005;
  Length = sizeof (GEN_AMT_BIOS_SYNCH_INFO);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgGenAmtBiosSynchInfo,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_AMT_BIOS_SYNCH_INFO_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgGenAmtBiosSynchInfoAck,
                  &Length
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *RuleData = MsgGenAmtBiosSynchInfoAck.RuleData;

  return Status;
}

/**
  The firmware will respond to GET OEM TAG message even after the End of Post (EOP).

  @param[out] RuleData            Default is zero. Tool can create the OEM specific OEM TAG data.

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetOemTagMsg (
  OUT UINT32         *RuleData
  )
{
  EFI_STATUS              Status;
  UINT32                  Length;
  EFI_HECI_PROTOCOL       *Heci;
  GEN_GET_OEM_TAG_MSG     MsgGenGetOemTagMsg;
  GEN_GET_OEM_TAG_MSG_ACK MsgGenGetOemTagMsgAck;
  UINT32                  SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetOemTagMsg.MKHIHeader.Data              = 0;
  MsgGenGetOemTagMsg.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  MsgGenGetOemTagMsg.MKHIHeader.Fields.Command    = FWCAPS_GET_RULE_CMD;
  MsgGenGetOemTagMsg.MKHIHeader.Fields.IsResponse = 0;
  MsgGenGetOemTagMsg.Data.RuleId                  = 0x2B;
  Length = sizeof (GEN_GET_OEM_TAG_MSG);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgGenGetOemTagMsg,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_OEM_TAG_MSG_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgGenGetOemTagMsgAck,
                  &Length
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *RuleData = MsgGenGetOemTagMsgAck.RuleData;

  return Status;
}

/**
  Enables/disables clocks. Used to turn off clocks in unused pci/pcie slots.

  @param[in]  Enables           Each bit means corresponding clock should be turned on (1) or off (0)
  @param[in]  EnablesMask       Each bit means corresponding enable bit is valid (1) or should be ignored (0)
  @param[in]  Parameters        Retain clock enables at resume for S3', other bits = reserved, must be 0
  @param[in]  Nonce             Secret number used to validate caller

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
EFIAPI
HeciSetIccClockEnables (
  IN UINT32 Enables,
  IN UINT32 EnablesMask,
  IN UINT32 Parameters,
  IN UINT64 Nonce
  )
{
  EFI_STATUS                  Status;
  ICC_SET_CLK_ENABLES_BUFFER  Buffer;
  UINT32                      CommandSize;
  UINT32                      ResponseSize;
  EFI_HECI_PROTOCOL           *Heci;
  UINT32                      SeCMode;
  UINT32                      SeCStatus;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetSeCStatus (&SeCStatus);
  if (EFI_ERROR (Status) || ((SeCStatus & 0xF) != SEC_READY)) {
    return EFI_NOT_READY;
  }

  CommandSize                         = sizeof (ICC_SET_CLK_ENABLES_MESSAGE);
  ResponseSize                        = sizeof (ICC_SET_CLK_ENABLES_RESPONSE);

  Buffer.message.Header.ApiVersion    = COUGAR_POINT_PLATFORM;
  Buffer.message.Header.IccCommand    = SET_CLOCK_ENABLES;
  Buffer.message.Header.IccResponse   = 0;
  Buffer.message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.message.Header.Reserved      = 0;
  Buffer.message.ClockEnables         = Enables;
  Buffer.message.ClockEnablesMask     = EnablesMask;
  Buffer.message.Params               = Parameters;
  Buffer.message.Nonce                = Nonce;


  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &Buffer,
                  CommandSize,
                  &ResponseSize,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_ICC_MESSAGE_ADDR
                  );


  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "(ICC) IccSetClockEnables: Message failed! EFI_STATUS = %r\n", Status));
    return Status;
  }

  if (Buffer.response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (EFI_D_ERROR,
       "(ICC) IccSetClockEnables: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.response.Header.IccResponse)
    );
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  Sets or reads Lock mask on ICC registers.

  @param[in]      AccessMode      0 - set, 1 - get
  @param[in,out]  Mask            Mask of registers to become (for 'set' mode) or are (for 'get' mode) locked. Each bit represents a register. 0=lock, 1=don't lock
  @param[in,out]  Nonce           This secret number must be used for every attempt to lock registers except first

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciLockIccRegisters (
  IN UINT8       AccessMode,
  IN OUT UINT32  *Mask,
  IN OUT UINT64  *Nonce
  )
{
  EFI_STATUS                Status;
  ICC_LOCK_REGISTERS_BUFFER Buffer;
  UINT32                    CommandSize;
  UINT32                    ResponseSize;
  EFI_HECI_PROTOCOL         *Heci;
  UINT32                    SeCMode;
  UINT32                    SeCStatus;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetSeCStatus (&SeCStatus);
  if (EFI_ERROR (Status) || ((SeCStatus & 0xF) != SEC_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((EFI_D_INFO, "(ICC) LockIccRegisters\n"));
  if (Mask == NULL) {
    return EFI_INVALID_PARAMETER;

  }

  CommandSize                         = sizeof (ICC_LOCK_REGISTERS_MESSAGE);
  ResponseSize                        = sizeof (ICC_LOCK_REGISTERS_RESPONSE);

  Buffer.message.Header.ApiVersion    = COUGAR_POINT_PLATFORM;
  Buffer.message.Header.IccCommand    = LOCK_ICC_REGISTERS;
  Buffer.message.Header.IccResponse   = 0;
  Buffer.message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.message.Header.Reserved      = 0;
  Buffer.message.AccessMode           = AccessMode;
  Buffer.message.PaddingA             = 0;
  Buffer.message.PaddingB             = 0;
  Buffer.message.Nonce                = *Nonce;
  Buffer.message.RegisterMask[0]      = Mask[0];
  Buffer.message.RegisterMask[1]      = Mask[1];
  Buffer.message.RegisterMask[2]      = Mask[2];

  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &Buffer,
                  CommandSize,
                  &ResponseSize,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_ICC_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "(ICC) LockIccRegisters: Message failed! EFI_STATUS = %r\n", Status));
    return Status;
  }

  if (Buffer.response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (EFI_D_ERROR,
       "(ICC) LockIccRegisters: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.response.Header.IccResponse)
    );
    return EFI_DEVICE_ERROR;
  }

  if ((AccessMode == SET_LOCK_MASK) && (Status == EFI_SUCCESS)) {
    *Nonce = Buffer.response.Nonce;
  }

  if (AccessMode == GET_LOCK_MASK) {
    Mask[0] = Buffer.response.RegisterMask[0];
    Mask[1] = Buffer.response.RegisterMask[1];
    Mask[2] = Buffer.response.RegisterMask[2];
  }

  return Status;
}

/**
  Retrieves the number of currently used ICC clock profile

  @param[out]  Profile            number of current ICC clock profile

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too smallfor the Acknowledge
**/
EFI_STATUS
HeciGetIccProfile (
  OUT UINT8*Profile
  )
{
  EFI_STATUS              Status;
  ICC_GET_PROFILE_BUFFER  Buffer;
  UINT32                  CommandSize;
  UINT32                  ResponseSize;
  EFI_HECI_PROTOCOL       *Heci;
  UINT32                  SeCMode;
  UINT32                  SeCStatus;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetSeCStatus (&SeCStatus);
  if (EFI_ERROR (Status) || ((SeCStatus & 0xF) != SEC_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((EFI_D_INFO, "(ICC) GetIccProfile\n"));
  CommandSize                         = sizeof (ICC_GET_PROFILE_MESSAGE);
  ResponseSize                        = sizeof (ICC_GET_PROFILE_RESPONSE);

  Buffer.message.Header.ApiVersion    = COUGAR_POINT_PLATFORM;
  Buffer.message.Header.IccCommand    = GET_ICC_PROFILE;
  Buffer.message.Header.IccResponse   = 0;
  Buffer.message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.message.Header.Reserved      = 0;

  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &Buffer,
                  CommandSize,
                  &ResponseSize,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_ICC_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "(ICC) GetIccProfile: Message failed! EFI_STATUS = %r\n", Status));
    return Status;
  }

  if (Buffer.response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (EFI_D_ERROR,
       "(ICC) GetIccProfile: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.response.Header.IccResponse)
    );
    Status = EFI_DEVICE_ERROR;
  } else {
    DEBUG ((EFI_D_INFO, "(ICC) GetIccProfile: Current profile = 0x%x\n", Buffer.response.IccProfileIndex));
  }

  if (Profile != NULL) {
    *Profile = Buffer.response.IccProfileIndex;
  }

  return Status;
}

/**
  Sets ICC clock profile to be used on next and following boots

  @param[in]  Profile             Number of profile to be used

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetIccProfile (
  IN UINT8 Profile
  )
{
  EFI_STATUS              Status;
  ICC_SET_PROFILE_BUFFER  Buffer;
  UINT32                  CommandSize;
  UINT32                  ResponseSize;
  EFI_HECI_PROTOCOL       *Heci;
  UINT32                  SeCMode;
  UINT32                  SeCStatus;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetSeCStatus (&SeCStatus);
  if (EFI_ERROR (Status) || ((SeCStatus & 0xF) != SEC_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((EFI_D_INFO, "(ICC) SetIccProfile\n"));

  CommandSize                         = sizeof (ICC_SET_PROFILE_MESSAGE);
  ResponseSize                        = sizeof (ICC_SET_PROFILE_RESPONSE);

  Buffer.message.Header.ApiVersion    = COUGAR_POINT_PLATFORM;
  Buffer.message.Header.IccCommand    = SET_ICC_PROFILE;
  Buffer.message.Header.IccResponse   = 0;
  Buffer.message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.message.Header.Reserved      = 0;
  Buffer.message.ProfileBIOS          = Profile;
  Buffer.message.PaddingA             = 0;
  Buffer.message.PaddingB             = 0;

  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &Buffer,
                  CommandSize,
                  &ResponseSize,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_ICC_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "(ICC) SetIccProfile: Message failed! EFI_STATUS = %r\n", Status));
    return Status;
  }

  if (Buffer.response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (EFI_D_ERROR,
       "(ICC) SetIccProfile: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.response.Header.IccResponse)
    );
    return EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  This message is used to enable the IntelR SEC Debug Event Service capability.
  Once firmware receives this message, the firmware will enable MDES capability.
  The firmware will automatically disable MDES capability after receiving End Of Post.

  @param[out]  Data               0x00   : Enable Success
                                  Others : Enable Failure

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciMdesCapabilityEnableMsg (
  OUT UINT8        *Data
  )
{
  EFI_STATUS                    Status;
  UINT32                        Length;
  EFI_HECI_PROTOCOL             *Heci;
  GEN_MDES_ENABLE_MKHI_CMD_MSG  MdesEnable;
  UINT32                        SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MdesEnable.MKHIHeader.Data              = 0;
  MdesEnable.MKHIHeader.Fields.GroupId    = MKHI_MDES_GROUP_ID;
  MdesEnable.MKHIHeader.Fields.Command    = MDES_ENABLE_MKHI_CMD;
  MdesEnable.MKHIHeader.Fields.IsResponse = 0;
  MdesEnable.Data                         = 1;
  Length = sizeof (GEN_MDES_ENABLE_MKHI_CMD_MSG);

  Status = Heci->SendwACK (
                  HECI1_DEVICE,
                  (UINT32 *) &MdesEnable,
                  Length,
                  &Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );

  if (Status == EFI_SUCCESS) {
    *Data = MdesEnable.Data;
  }

  return Status;
}

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.
  FWInitComplete value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] RuleData            MEFWCAPS_SKU message

  @exception EFI_UNSUPPORTED      Current ME mode doesn't support this function
  @retval EFI_SUCCESS             HECI interfaces disabled by Sec
**/
EFI_STATUS
HeciGetFwFeatureStateMsg (
  OUT SECFWCAPS_SKU                    *RuleData
  )
{
  EFI_STATUS                    Status;
  UINT32                        Length;
  GEN_GET_FW_FEATURE_STATUS     GetFwFeatureStatus;
  GEN_GET_FW_FEATURE_STATUS_ACK GetFwFeatureStatusAck;
  UINT32                        SeCMode;
  EFI_HECI_PROTOCOL             *Heci;

  DEBUG ((EFI_D_INFO, "HeciGetFwFeatureStateMsg ++\n"));
  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  GetFwFeatureStatus.MKHIHeader.Data              = 0;
  GetFwFeatureStatus.MKHIHeader.Fields.GroupId    = MKHI_FWCAPS_GROUP_ID;
  GetFwFeatureStatus.MKHIHeader.Fields.Command    = FWCAPS_GET_RULE_CMD;
  GetFwFeatureStatus.MKHIHeader.Fields.IsResponse = 0;
  GetFwFeatureStatus.Data.RuleId                  = 0x20;

  Length = sizeof (GEN_GET_FW_FEATURE_STATUS);
  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &GetFwFeatureStatus,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_FW_FEATURE_STATUS_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &GetFwFeatureStatusAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  RuleData->Data = GetFwFeatureStatusAck.RuleData.Data;
  DEBUG ((EFI_D_INFO, "HeciGetFwFeatureStateMsg --\n"));
  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to disable the SEC State.
  The firmware allows a single update once it receives the disable command

  @param[out]  CmdStatus      Return command status

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSeCUnconfigurationMsg (
  OUT UINT32    *CmdStatus
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  EFI_HECI_PROTOCOL *Heci;
  MKHI_MESSAGE_HEADER      MsgSeCUnConfigure;
  MKHI_MESSAGE_HEADER  MsgSeCUnConfigureAck;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgSeCUnConfigure.Data               = 0;
  MsgSeCUnConfigure.Fields.GroupId     = MKHI_GEN_GROUP_ID;
  MsgSeCUnConfigure.Fields.Command     = SEC_UNCONFIGURATION_CMD;
  MsgSeCUnConfigure.Fields.IsResponse  = 0;


  Length = sizeof (MKHI_MESSAGE_HEADER);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgSeCUnConfigure,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (MKHI_MESSAGE_HEADER);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgSeCUnConfigureAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  *CmdStatus = MsgSeCUnConfigureAck.Fields.Reserved;
  return Status;

}

/**
  This message is sent by the BIOS prior to the End of Post (EOP) on the boot
   where host wants to Unconfigure the SEC State

  @param[in]  CmdStatus         // return the data
                                 0 = SEC_UNCONFIG_SUCCESS
                                 1 = SEC_UNCONFIG_IN_PROGRESS
                                 2 = SEC_UNCONFIG_NOT_IN_PROGRESS
                                 3 = SEC_UNCONFIG_ERROR

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSeCUnconfigurationStatusMsg (
  IN UINT32    *CmdStatus
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  EFI_HECI_PROTOCOL *Heci;
  MKHI_MESSAGE_HEADER  MsgSeCUnConfigureStatus;
  MKHI_MESSAGE_HEADER  MsgSeCUnConfigureStatusAck;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgSeCUnConfigureStatus.Data               = 0;
  MsgSeCUnConfigureStatus.Fields.GroupId     = MKHI_GEN_GROUP_ID;
  MsgSeCUnConfigureStatus.Fields.Command     = SEC_UNCONFIGURATION_STATUS;
  MsgSeCUnConfigureStatus.Fields.IsResponse  = 0;


  Length = sizeof (MKHI_MESSAGE_HEADER);

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgSeCUnConfigureStatus,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

//[-start-161123-IB07250310-add]//
#ifdef FSP_WRAPPER_FLAG
  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    //
    // Fault Tolerance boot to send SEC_UNCONFIGURATION_STATUS command
    // need to add an one second delay for avoid 30 second timeout.
    //
    gBS->Stall (EFI_SEC_STATE_STALL_1_SECOND);
  }
#endif
//[-end-161123-IB07250310-add]//

  Length = sizeof (MKHI_MESSAGE_HEADER);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) &MsgSeCUnConfigureStatusAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  *CmdStatus = MsgSeCUnConfigureStatusAck.Fields.Result;
  return Status;

}

/**
  Send Get IFWI Version Request to SEC

  @param[in,out]  MsgGenGetIFWIVersionAck   Return the message of IFWI version

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetIFWIVersionMsg (
  IN OUT GEN_GET_IFWI_VER_ACK     *MsgGenGetIFWIVersionAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_GET_IFWI_VER    *MsgGenGetIFWIVersion;
  GEN_GET_IFWI_VER    GenGetIFWIVersion;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenGetFwVersion data structure
  //

  MsgGenGetIFWIVersion = &GenGetIFWIVersion;
  MsgGenGetIFWIVersion->MKHIHeader.Data = 0;
  MsgGenGetIFWIVersion->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenGetIFWIVersion->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenGetIFWIVersion->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GET_IFWI_VER);
  //
  // Send Get IFWI Version Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenGetIFWIVersion,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_IFWI_VER_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenGetIFWIVersionAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Retrieves the current RPMB config file

  @param[out]  MsgGenGetRPMBConfigFileAck   Return the current RPMB config file

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetRPMBconfigfile (
  OUT GEN_GET_RPMB_CONFIG_FILE_ACK     *MsgGenGetRPMBConfigFileAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_GET_RPMB_CONFIG_FILE    *MsgGenGetRPMBconfigfile;
  GEN_GET_RPMB_CONFIG_FILE    GenGetRPMBconfigfile;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenGetRPMBconfigfile data structure
  //

  MsgGenGetRPMBconfigfile = &GenGetRPMBconfigfile;
  MsgGenGetRPMBconfigfile->MKHIHeader.Data = 0;
  MsgGenGetRPMBconfigfile->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenGetRPMBconfigfile->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenGetRPMBconfigfile->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GET_IFWI_VER);
  //
  // Send Get RPMB config file Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenGetRPMBconfigfile,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_RPMB_CONFIG_FILE_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenGetRPMBConfigFileAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Set the current RPMB config file

  @param[in,out]  MsgGenSetRPMBConfigFileAck   Set the current RPMB config file

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetRPMBconfigfile (
  IN OUT GEN_SET_RPMB_CONFIG_FILE_ACK     *MsgGenSetRPMBConfigFileAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_SET_RPMB_CONFIG_FILE    *MsgGenSetRPMBconfigfile;
  GEN_SET_RPMB_CONFIG_FILE    GenSetRPMBconfigfile;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenSetRPMBconfigfile data structure
  //

  MsgGenSetRPMBconfigfile = &GenSetRPMBconfigfile;
  MsgGenSetRPMBconfigfile->MKHIHeader.Data = 0;
  MsgGenSetRPMBconfigfile->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenSetRPMBconfigfile->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenSetRPMBconfigfile->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GET_IFWI_VER);
  //
  // Send Set RPMB config file Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenSetRPMBconfigfile,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_SET_RPMB_CONFIG_FILE_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenSetRPMBConfigFileAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Read the Boot Partition

  @param[in,out]  MsgGenBootPartitionReadAck   Read the current Boot Partition

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciBootPartitionRead (
  IN OUT GEN_BOOT_PARTITION_READ_ACK     *MsgGenBootPartitionReadAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_BOOT_PARTITION_READ    *MsgGenBootPartitionRead;
  GEN_BOOT_PARTITION_READ    GenBootPartitionRead;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenBootPartitionRead data structure
  //

  MsgGenBootPartitionRead = &GenBootPartitionRead;
  MsgGenBootPartitionRead->MKHIHeader.Data = 0;
  MsgGenBootPartitionRead->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenBootPartitionRead->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenBootPartitionRead->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_BOOT_PARTITION_READ);
  //
  // Send Boot Partition Read Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenBootPartitionRead,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_BOOT_PARTITION_READ_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenBootPartitionReadAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Read the Boot Partition

  @param[in,out]  MsgGenBootPartitionReadAck   Read the current Boot Partition

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciMassStorageRead (
  IN OUT GEN_MASS_STORAGE_READ_ACK     *MsgGenMassStorageReadAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_MASS_STORAGE_READ    *MsgGenMassStorageRead;
  GEN_MASS_STORAGE_READ    GenMassStorageRead;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenMassStorageRead data structure
  //

  MsgGenMassStorageRead = &GenMassStorageRead;
  MsgGenMassStorageRead->MKHIHeader.Data = 0;
  MsgGenMassStorageRead->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenMassStorageRead->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenMassStorageRead->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_MASS_STORAGE_READ);
  //
  // Send MassStorageRead Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenMassStorageRead,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_MASS_STORAGE_READ_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenMassStorageReadAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Request Device Ownership

  @param[in,out]  MsgGenRequestDeviceOwnershipAck   Request Device Ownership

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciRequestDeviceOwnerShip (
  IN OUT GEN_REQUEST_DEVICE_OWNERSHIP_ACK     *MsgGenRequestDeviceOwnershipAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_REQUEST_DEVICE_OWNERSHIP    *MsgGenRequestDeviceOwnership;
  GEN_REQUEST_DEVICE_OWNERSHIP    GenRequestDeviceOwnership;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenRequestDeviceOwnerShip data structure
  //

  MsgGenRequestDeviceOwnership = &GenRequestDeviceOwnership;
  MsgGenRequestDeviceOwnership->MKHIHeader.Data = 0;
  MsgGenRequestDeviceOwnership->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenRequestDeviceOwnership->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenRequestDeviceOwnership->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_REQUEST_DEVICE_OWNERSHIP);
  //
  // Send Device Ownership Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenRequestDeviceOwnership,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_REQUEST_DEVICE_OWNERSHIP_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenRequestDeviceOwnershipAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Grant Device Ownership

  @param[in,out]  MsgGenGrantDeviceOwnershipAck   Grant Device Ownership

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGrantDeviceOwnerShip (
  IN OUT GEN_GRANT_DEVICE_OWNERSHIP_ACK     *MsgGenGrantDeviceOwnerShipAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_GRANT_DEVICE_OWNERSHIP    *MsgGenGrantDeviceOwnership;
  GEN_GRANT_DEVICE_OWNERSHIP    GenGrantDeviceOwnership;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenGrantDeviceOwnerShip data structure
  //

  MsgGenGrantDeviceOwnership = &GenGrantDeviceOwnership;
  MsgGenGrantDeviceOwnership->MKHIHeader.Data = 0;
  MsgGenGrantDeviceOwnership->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenGrantDeviceOwnership->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenGrantDeviceOwnership->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GRANT_DEVICE_OWNERSHIP);
  //
  // Send Grant Device Ownership Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenGrantDeviceOwnership,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GRANT_DEVICE_OWNERSHIP_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenGrantDeviceOwnerShipAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  SMIP Read

  @param[in,out]  MsgGenSMIPReadAck  SMIP Read

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSMIPRead (
  IN OUT GEN_SMIP_READ_ACK     *MsgGenSMIPReadAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_SMIP_READ    *MsgGenSMIPRead;
  GEN_SMIP_READ    GenGrantSMIPRead;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenSMIPRead data structure
  //

  MsgGenSMIPRead = &GenGrantSMIPRead;
  MsgGenSMIPRead->MKHIHeader.Data = 0;
  MsgGenSMIPRead->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenSMIPRead->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenSMIPRead->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_SMIP_READ);
  //
  // Send SMIP READ Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenSMIPRead,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_SMIP_READ_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenSMIPReadAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Auth Policy Manifest

  @param[in,out]  MsgGenAuthPolicyManifestAck   Auth Policy Manifest

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciAuthPolicyManifest (
  IN OUT GEN_AUTH_POLICY_MANIFEST_ACK     *MsgGenAuthPolicyManifestAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_AUTH_POLICY_MANIFEST    *MsgGenAuthPolicyManifest;
  GEN_AUTH_POLICY_MANIFEST    GenAuthPolicyManifest;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenAuthPolicyManifest data structure
  //

  MsgGenAuthPolicyManifest = &GenAuthPolicyManifest;
  MsgGenAuthPolicyManifest->MKHIHeader.Data = 0;
  MsgGenAuthPolicyManifest->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenAuthPolicyManifest->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenAuthPolicyManifest->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_AUTH_POLICY_MANIFEST);
  //
  // Send SMIP READ Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenAuthPolicyManifest,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_AUTH_POLICY_MANIFEST_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenAuthPolicyManifestAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Get Boot Type data.

  @param[in,out]  MsgGenBootTypeAck   Boot Type

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciBootType (
  IN OUT GEN_BOOT_TYPE_ACK     *MsgGenBootTypeAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_BOOT_TYPE    *MsgGenBootType;
  GEN_BOOT_TYPE     GenBootType;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenBootType data structure
  //

  MsgGenBootType = &GenBootType;
  MsgGenBootType->MKHIHeader.Data = 0;
  MsgGenBootType->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenBootType->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenBootType->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_BOOT_TYPE);
  //
  // Send BOOT TYPE Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenBootType,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_BOOT_TYPE_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenBootTypeAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Send AUTH Kernel Request to SEC

  @param[in,out]  MsgGenAuthKernelAck   Auth Kernel

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciAuthKernel (
  IN OUT GEN_AUTH_KERNEL_ACK     *MsgGenAuthKernelAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_AUTH_KERNEL    *MsgGenAuthKernel;
  GEN_AUTH_KERNEL     GenAuthKernel;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenAuthKernel data structure
  //

  MsgGenAuthKernel = &GenAuthKernel;
  MsgGenAuthKernel->MKHIHeader.Data = 0;
  MsgGenAuthKernel->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenAuthKernel->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenAuthKernel->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_AUTH_KERNEL);
  //
  // Send AUTH Kernel Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenAuthKernel,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_AUTH_KERNEL_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenAuthKernelAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  RSA Offload

  @param[in,out]  MsgGenRSAOffloadAck    RSA Offload

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciRSAOffload (
  IN OUT GEN_RSA_OFFLOAD_ACK     *MsgGenRSAOffloadAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_RSA_OFFLOAD    *MsgGenRSAOffload;
  GEN_RSA_OFFLOAD      GenRSAOffload;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenRSAOffload data structure
  //

  MsgGenRSAOffload = &GenRSAOffload;
  MsgGenRSAOffload->MKHIHeader.Data = 0;
  MsgGenRSAOffload->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenRSAOffload->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenRSAOffload->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_RSA_OFFLOAD);
  //
  // Send RSA Offload Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenRSAOffload,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_RSA_OFFLOAD_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenRSAOffloadAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Get MBP Request from SEC

  @param[in,out]  MsgGenGetMBPAck    GET MBP

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetMBP(
  IN OUT GEN_GET_MBP_ACK     *MsgGenGetMBPAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_GET_MBP    *MsgGenGetMBP;
  GEN_GET_MBP      GenGetMBP;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenGetMBP data structure
  //

  MsgGenGetMBP = &GenGetMBP;
  MsgGenGetMBP->MKHIHeader.Data = 0;
  MsgGenGetMBP->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenGetMBP->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenGetMBP->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_GET_MBP);
  //
  // Send Get MBP Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenGetMBP,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_GET_MBP_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenGetMBPAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Load OBB

  @param[in,out]  MsgGenLoadOBBAck    Load OBB

  @exception EFI_UNSUPPORTED      Current Sec mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciLoadOBB(
  IN OUT GEN_LOAD_OBB_ACK     *MsgGenLoadOBBAck
  )
{
  EFI_STATUS        Status;
  UINT32            Length;
  GEN_LOAD_OBB    *MsgGenLoadOBB;
  GEN_LOAD_OBB      GenLoadOBB;
  EFI_HECI_PROTOCOL *Heci;
  UINT32            SeCMode;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetSeCMode(HECI1_DEVICE, &SeCMode);
  if (EFI_ERROR (Status) || (SeCMode != SEC_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate MsgGenGetMBP data structure
  //

  MsgGenLoadOBB = &GenLoadOBB;
  MsgGenLoadOBB->MKHIHeader.Data = 0;
  MsgGenLoadOBB->MKHIHeader.Fields.GroupId = 0;  //wait HAS definition
  MsgGenLoadOBB->MKHIHeader.Fields.Command = 0; //wait HAS definition
  MsgGenLoadOBB->MKHIHeader.Fields.IsResponse = 0;
  Length = sizeof (GEN_LOAD_OBB);
  //
  // Send Load OBB Request to SEC
  //

  Status = Heci->SendMsg (
                  HECI1_DEVICE,
                  (UINT32 *) MsgGenLoadOBB,
                  Length,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Length = sizeof (GEN_LOAD_OBB_ACK);
  Status = Heci->ReadMsg (
                  HECI1_DEVICE,
                  BLOCKING,
                  (UINT32 *) MsgGenLoadOBBAck,
                  &Length
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}
