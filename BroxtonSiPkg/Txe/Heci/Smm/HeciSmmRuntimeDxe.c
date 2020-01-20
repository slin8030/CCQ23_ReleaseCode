/** @file
  HECI Smm Runtime Dxe driver

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2007 - 2016 Intel Corporation.

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


#include <Protocol/SmmCommunication.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Guid/EventGroup.h>
#include <protocol/Heci.h>
#include <HeciSmm.h>
#include <Library/SideBandLib.h>
#include <SeCAccess.h>
#include <HeciRegs.h>
//[-start-160904-IB07400778-add]//
#include <Library/HobLib.h>
//[-end-160904-IB07400778-add]//

EFI_HANDLE                       mHandle                    = NULL;
EFI_HECI_PROTOCOL               *mSmmHeci                   = NULL;
EFI_EVENT                        mVirtualAddressChangeEvent = NULL;
EFI_SMM_COMMUNICATION_PROTOCOL  *mSmmCommunication          = NULL;
UINT8                           *mHeciBuffer                = NULL;
UINT8                           *mHeciBufferPhysical        = NULL;
UINTN                            mHeciBufferSize;
UINTN                            mHeciBufferPayloadSize;

//[-start-190315-IB07401091-add]//
#ifndef DISABLE_HECI2_HECI3_DISABLE_WORKAROUND
STATIC UINTN                     mHeci2BarAddress           = 0;
STATIC UINTN                     mHeci3BarAddress           = 0;
#endif
//[-end-190315-IB07401091-add]//

/**
  Put HECI1 and HECI3 into D0I3.

  @param[in]    None.

  @retval      None
**/
//[-start-190315-IB07401091-modify]//
#ifdef DISABLE_HECI2_HECI3_DISABLE_WORKAROUND
VOID
PutDeviceIntoD0I3 (
  VOID
  )
{
  UINT32 HeciBar;

  DEBUG ((EFI_D_INFO, "Put all HECI devices into D0i3 and hide HECI 2 and HECI 3.\n"));

  //
  // 1. Read the DEVIDLEC register value
  // 2. If DEVIDLE = 1, already in D0i3 state - nothing to do, exit
  // 3. If CIP = 1, wait until cleared (poll on the bit value until cleared)
  // 4. Set DEVIDLE = 1 and write the DEVIDLEC register value
  //
  HeciBar = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0 ;
  if ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    // Poll until CIP == 0
    while ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_CIP) == B_HECI_DEVIDLEC_CIP);

    Mmio32Or (HeciBar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  HeciBar = Heci2PciRead32 (R_HECIMBAR0) & 0xFFFFFFF0 ;
  if ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    // Poll until CIP == 0
    while ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_CIP) == B_HECI_DEVIDLEC_CIP);

    Mmio32Or (HeciBar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  HeciBar = Heci3PciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
  if ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    // Poll until CIP == 0
    while ((Mmio32 (HeciBar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_CIP) == B_HECI_DEVIDLEC_CIP);

    Mmio32Or (HeciBar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  //
  // To hide the HECI 2 in PCI configuration space
  //
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F1_FUN_DIS,
    0xFFFFFFFF,
    (UINT32) BIT0
    );

  //
  // To hide the HECI 3 in PCI configuration space
  //
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F2_FUN_DIS,
    0xFFFFFFFF,
    (UINT32) BIT0
    );
 }
#else

/**
  Hide the HECI2 and HECI3 devices.

  This function must be called before IA untrusted mode is enabled.

  HECI1 is used by the CSE OS proxy driver and should be visible to the OS.
  HECI2 is used as the HECI channel between BIOS and CSE firmware and should not be visible to the OS.
  HECI3 is used for secure touch and should not be visible to the OS.

**/
VOID
EFIAPI
HideHeciDevices (
  IN VOID
  )
{
  DEBUG ((EFI_D_INFO, "Hiding HECI2 and HECI3.\n"));

  Heci2PciOr32 (
    PCI_COMMAND_OFFSET,
    EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER | EFI_PCI_COMMAND_SERR
    );

  //
  // Hide HECI 2 in PCI configuration space
  //
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F1_FUN_DIS,
    0xFFFFFFFF,
    (UINT32) BIT0
    );

  //
  // Hide HECI 3 in PCI configuration space
  //
  SideBandAndThenOr32 (
    SB_PORTID_PSF3,
    R_PCH_PCR_PSF3_T0_SHDW_CSE_D15F2_FUN_DIS,
    0xFFFFFFFF,
    (UINT32) BIT0
    );
}

/**
  Put HECI1, HECI2, and HECI3 into D0i3.

  HECI1 is used by the CSE OS proxy driver and will be power managed by an OS driver as needed.
  HECI2 is used as the HECI channel between BIOS and CSE firmware and will be power managed by BIOS.
  HECI3 is used for secure touch and will be power managed by an OS driver as needed.

**/
VOID
EFIAPI
PutHeciDevicesIntoD0i3 (
  VOID
  )
{
  UINT32 Heci1Bar;
  UINT32 Data32;

  DEBUG ((EFI_D_INFO, "Placing the HECI1, HECI2, and HECI3 devices into D0i3.\n"));

  //
  // Put HECI1 into D0i3
  //
  Heci1Bar = HeciPciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
  if ((Mmio32 (Heci1Bar, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    WaitForDevIdleRegCipToClear (Heci1Bar, Data32);
    Mmio32Or (Heci1Bar, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  //
  // Put HECI2 into D0i3
  //
  if ((Mmio32 (mHeci2BarAddress, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    WaitForDevIdleRegCipToClear (mHeci2BarAddress, Data32);
    Mmio32Or (mHeci2BarAddress, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }

  //
  // Set HECI3 into D0i3
  //
  if ((Mmio32 (mHeci3BarAddress, R_HECI_DEVIDLEC) & B_HECI_DEVIDLEC_DEVIDLE) == 0) {
    WaitForDevIdleRegCipToClear (mHeci3BarAddress, Data32);
    Mmio32Or (mHeci3BarAddress, R_HECI_DEVIDLEC, B_HECI_DEVIDLEC_DEVIDLE);
  }
}

#endif
//[-end-190315-IB07401091-modify]//


/**
  Initialize the communicate buffer using DataSize and Function.

  The communicate size is: SMM_COMMUNICATE_HEADER_SIZE + SMM_VARIABLE_COMMUNICATE_HEADER_SIZE +
  DataSize.

  @param[out]     DataPtr           Points to the data in the communicate buffer.
  @param[in]       DataSize         The data size to send to SMM.
  @param[in]       Function          The function number to initialize the communicate header.

  @retval EFI_INVALID_PARAMETER     The data size is too big.
  @retval EFI_SUCCESS               Find the specified variable.
**/
EFI_STATUS
InitCommunicateBuffer (
  OUT     VOID                              **DataPtr OPTIONAL,
  IN      UINTN                             DataSize,
  IN      UINTN                             Function
  )
{
  EFI_SMM_COMMUNICATE_HEADER                *SmmCommunicateHeader;
  SMM_HECI_COMMUNICATE_HEADER               *SmmHeciFunctionHeader;


  if (DataSize  > mHeciBufferSize) {
    return EFI_INVALID_PARAMETER;
  }

  SmmCommunicateHeader = (EFI_SMM_COMMUNICATE_HEADER *) mHeciBuffer;
  CopyGuid (&SmmCommunicateHeader->HeaderGuid, &gEfiHeciSmmProtocolGuid);
  SmmCommunicateHeader->MessageLength = DataSize + sizeof(EFI_SMM_COMMUNICATE_HEADER);

  SmmHeciFunctionHeader = (SMM_HECI_COMMUNICATE_HEADER *) SmmCommunicateHeader->Data;
  SmmHeciFunctionHeader->Function = Function;
  if (DataPtr != NULL) {
    *DataPtr = SmmHeciFunctionHeader->Data;
  }

  return EFI_SUCCESS;
}


/**
  Send the data in communicate buffer to SMM.

  @param[in]   DataSize               This size of the function header and the data.

  @retval      EFI_SUCCESS          Success is returned from the functin in SMM.
  @retval      Others                     Failure is returned from the function in SMM.
**/
EFI_STATUS
SendCommunicateBuffer (
  IN      UINTN                             DataSize
  )
{
  EFI_STATUS                                Status;
  UINTN                                     CommSize;
  EFI_SMM_COMMUNICATE_HEADER                *SmmCommunicateHeader;
  SMM_HECI_COMMUNICATE_HEADER               *SmmHeciFunctionHeader;

  CommSize = DataSize + sizeof(EFI_SMM_COMMUNICATE_HEADER) + sizeof(SMM_HECI_COMMUNICATE_HEADER);
  Status = mSmmCommunication->Communicate (mSmmCommunication, mHeciBufferPhysical, &CommSize);
  ASSERT_EFI_ERROR (Status);

  SmmCommunicateHeader      = (EFI_SMM_COMMUNICATE_HEADER *) mHeciBuffer;
  SmmHeciFunctionHeader = (SMM_HECI_COMMUNICATE_HEADER *)SmmCommunicateHeader->Data;
  return  SmmHeciFunctionHeader->ReturnStatus;
}


/**
  Send Heci message with response

  @param[in]       HeciDev       HECI device
  @param[in,out] Message       Message Data
  @param[in,out] Length          Message Data Length
  @param[in,out] RecLength     Return message buffer length
  @param[in]       HostAddress Host Address
  @param[in]       SECAddress CSE Address

  @retval  EFI_SUCCESS   Send message success.
  @retval  Others              Send message failed.
**/
EFI_STATUS
EfiHeciSendwack (
  IN      HECI_DEVICE      HeciDev,
  IN OUT  UINT32           *Message,
  IN OUT  UINT32           Length,
  IN OUT  UINT32           *RecLength,
  IN      UINT8            HostAddress,
  IN      UINT8            SECAddress
  )
{
  EFI_STATUS                                Status;
  UINTN                                     PayloadSize;
  SMM_HECI_SEND_MESSAGE_W_ACK_BODY         *SmmHeciSendMessageBody;
  UINT32                                    MaxLength;
  UINT32                                    SecMode;

  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }
  if (Message == NULL || Length == 0) {
    return EFI_INVALID_PARAMETER;
  }
  SecMode  = HeciPciRead32(R_SEC_FW_STS0);
  if (SEC_MODE_NORMAL != (SecMode & 0xF0000)) {
    return EFI_UNSUPPORTED;
  }



  if (Length > *RecLength) {
    MaxLength = Length;
  }else {
    MaxLength = *RecLength;
  }
  SmmHeciSendMessageBody       = NULL;

  PayloadSize = sizeof(SMM_HECI_COMMUNICATE_HEADER) + sizeof(SMM_HECI_SEND_MESSAGE_W_ACK_BODY) + Length;

  Status = InitCommunicateBuffer ((VOID **)&SmmHeciSendMessageBody, PayloadSize, SMM_HECI_FUNCTION_SEND_MESSAGE_WITH_ACK);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  ASSERT (SmmHeciSendMessageBody != NULL);
  if(SmmHeciSendMessageBody!= NULL){
      DEBUG((EFI_D_ERROR, "EfiHeciSendwack 2\n"));
      SmmHeciSendMessageBody->Length       = Length;
      SmmHeciSendMessageBody->HostAddress  = HostAddress;
      SmmHeciSendMessageBody->CSEAddress   = SECAddress;
      CopyMem(SmmHeciSendMessageBody->MessageData, (VOID*)Message, Length);
  //
  // Send data to SMM.
  //
      Status = SendCommunicateBuffer (PayloadSize);
         if (!EFI_ERROR(Status)) {
        *RecLength = SmmHeciSendMessageBody->RecLength;
        CopyMem(Message, SmmHeciSendMessageBody->MessageData, *RecLength);
        }
  }

Done:
  return Status;
}

/**
  Read the Heci message

  @param[in]      HeciDev      HECI Device ID.
  @param[in]      Blocking      Indicate does waiting for response.
  @param[in]      MessageBody Message data buffer.
  @param[in,out]  Length          Message data buffer size.

  @retval  EFI_SUCCESS   Send message success.
  @retval  Others              Send message failed.
**/
EFI_STATUS
EfiHeciReadMessage (
  IN      HECI_DEVICE      HeciDev,
  IN      UINT32           Blocking,
  IN      UINT32           *MessageBody,
  IN OUT  UINT32           *Length
  )
{
  EFI_STATUS                       Status;

  UINT32          SecMode;
  UINTN                             PayloadSize;
  SMM_HECI_READ_MESSAGE_BODY               *SmmHeciReadMessageBody;
  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }


  SecMode  = HeciPciRead32(R_SEC_FW_STS0);
  if (SEC_MODE_NORMAL != (SecMode & 0xF0000)) {

    return EFI_UNSUPPORTED;
  }


  SmmHeciReadMessageBody       = NULL;

  PayloadSize = sizeof(SMM_HECI_COMMUNICATE_HEADER) + sizeof(SMM_HECI_READ_MESSAGE_BODY) + *Length;

  Status = InitCommunicateBuffer ((VOID **)&SmmHeciReadMessageBody, PayloadSize, SMM_HECI_FUNCTION_READ_MESSAGE);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  ASSERT (SmmHeciReadMessageBody != NULL);

  if(SmmHeciReadMessageBody!= NULL){
    SmmHeciReadMessageBody->Length = *Length;
  //
  // Send data to SMM.
  //
    Status = SendCommunicateBuffer (PayloadSize);
    if (!EFI_ERROR(Status)) {
      *Length = SmmHeciReadMessageBody->Length;
      CopyMem(MessageBody, SmmHeciReadMessageBody->MessageData, *Length);
    }
  }
  Done:
    return Status;
}

/**
  Send Heci message without response

  @param[in]  HeciDev      HECI Device ID.
  @param[in]  Message      Message Data.
  @param[in]  Length         Message Data length.
  @param[in]  HostAddress Host Address.
  @param[in]  SECAddress CSE Address.

  @retval  EFI_SUCCESS   Send message success.
  @retval  Others              Send message failed.
**/
EFI_STATUS
EfiHeciSendMessage (
  IN      HECI_DEVICE      HeciDev,
  IN      UINT32           *Message,
  IN      UINT32           Length,
  IN      UINT8            HostAddress,
  IN      UINT8            SECAddress
  )
{
  EFI_STATUS                                Status;
  UINTN                                     PayloadSize;
  SMM_HECI_SEND_MESSAGE_BODY               *SmmHeciSendMessageBody;
  UINT32          SecMode;

  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }



  SecMode  = HeciPciRead32(R_SEC_FW_STS0);
  if (SEC_MODE_NORMAL != (SecMode & 0xF0000)) {

    return EFI_UNSUPPORTED;
  }
  if (Message == NULL || Length == 0) {
    return EFI_INVALID_PARAMETER;
  }


  SmmHeciSendMessageBody           = NULL;

  PayloadSize = sizeof(SMM_HECI_COMMUNICATE_HEADER) + Length;

  Status = InitCommunicateBuffer ((VOID **)&SmmHeciSendMessageBody, PayloadSize, SMM_HECI_FUNCTION_SEND_MESSAGE);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  ASSERT (SmmHeciSendMessageBody != NULL);

  if(SmmHeciSendMessageBody != NULL){
  SmmHeciSendMessageBody->Length       = Length;
  SmmHeciSendMessageBody->HostAddress  = HostAddress;
  SmmHeciSendMessageBody->CSEAddress   = SECAddress;
  CopyMem(SmmHeciSendMessageBody->MessageData, (VOID*)Message, Length);

  //
  // Send data to SMM.
  //
  Status = SendCommunicateBuffer (PayloadSize);
  }
Done:
  return Status;
}

/**
  Reset the heci device

  @param[in]      HeciDev HECI Device ID.

  @retval  EFI_SUCCESS   Reset HECI success.
  @retval  Others              Reset HECI failed.
**/
EFI_STATUS
EfiHeciReset (
  IN      HECI_DEVICE      HeciDev
  )
{
  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }
  return EFI_UNSUPPORTED;
}

/**
  Init the heci device

  @param[in]      HeciDev HECI Device ID.

  @retval  EFI_SUCCESS   Init HECI success.
  @retval  Others              Init HECI failed.
**/
EFI_STATUS
EfiHeciInit (
  IN      HECI_DEVICE      HeciDev
  )
{
  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }
  return EFI_UNSUPPORTED;
}

/**
  Reinit the heci device

  @param[in]      HeciDev HECI Device ID.

  @retval  EFI_SUCCESS   Reinit HECI success.
  @retval  Others              Reinit HECI failed.
**/
EFI_STATUS
EfiHeciReinit (
  IN      HECI_DEVICE      HeciDev
  )
{
  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }
  return EFI_UNSUPPORTED;
}

/**
  Reset the Heci device and waiting for Delay time

  @param[in]      HeciDev HECI Device ID.
  @param[in]      Delay     The time waiting for reset.

  @retval  EFI_SUCCESS   Reset success.
  @retval  Others              Reset failed.
**/
EFI_STATUS
EfiHeciResetWait(
  IN      HECI_DEVICE      HeciDev,
  IN      UINT32           Delay
  )
{
  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }
  return EFI_UNSUPPORTED;
}

/**
  Get the Heci status

  @param[out]  Status        HECI Staus.

  @retval  EFI_SUCCESS   Get status success.
  @retval  Others              Get status failed.
**/
EFI_STATUS
EfiHeciGetSecStatus (
  OUT UINT32                       *Status2
  )
{
  EFI_STATUS                Status;
  UINTN                   PayloadSize;
  UINT8                                     *Data;


  Data       = NULL;

  PayloadSize = sizeof(SMM_HECI_COMMUNICATE_HEADER);

  Status = InitCommunicateBuffer ((VOID **)&Data, PayloadSize, SMM_HECI_FUNCTION_GET_STATUS);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  ASSERT (Data != NULL);

  if(Data != NULL){
    //
    // Send data to SMM.
    //
    Status = SendCommunicateBuffer (PayloadSize);
    *Status2 = (UINT32)*Data;
  }
  Done:
  return Status;
}

/**
  Get the heci mode

  @param[in]      HeciDev  HECI Device ID.
  @param[out]    Mode      Heci Mode

  @retval  EFI_SUCCESS   Get mode success.
  @retval  Others              Get mode failed.
**/
EFI_STATUS
EfiHeciGetSecMode (
  IN  HECI_DEVICE                 HeciDev,
  OUT  UINT32                     *Mode
  )
{
  if (HeciDev != HECI2_DEVICE) {
    return EFI_UNSUPPORTED;
  }
  return EFI_UNSUPPORTED;
}

EFI_HECI_PROTOCOL mHeciProtocol = {
  EfiHeciSendwack,
  EfiHeciReadMessage,
  EfiHeciSendMessage,
  EfiHeciReset,
  EfiHeciInit,
  EfiHeciResetWait,
  EfiHeciReinit,
  EfiHeciGetSecStatus,
  EfiHeciGetSecMode
};


/**
  Initialize variable service and install Variable Architectural protocol.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
AddressChangeEvent (
  IN EFI_EVENT                              Event,
  IN VOID                                   *Context
  )
{
  EfiConvertPointer (0x0, (VOID **) &mHeciBuffer);
  EfiConvertPointer (0x0, (VOID **) &mSmmCommunication);
}

/*
  HeciSmmReady

  @param[in] Event
  @param[in] Context

  @retval  EFI_STATUS
*/
VOID
EFIAPI
HeciSmmReady (
  IN  EFI_EVENT                             Event,
  IN  VOID                                  *Context
  )
{
  EFI_STATUS                                Status;

  DEBUG((EFI_D_INFO, "HeciSmmReady \n"));

  if (mHandle != NULL) {
    DEBUG((EFI_D_INFO, "gEfiHeciProtocol Has Installed return\n"));
    return;
  }
  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **) &mSmmCommunication);
  ASSERT_EFI_ERROR (Status);

  //
  // Save the buffer physical address used for SMM conmunication.
  //
  mHeciBufferPhysical = mHeciBuffer;

  //
  // Install the Heci Protocol on a new handle.
  //
  Status = gBS->InstallProtocolInterface (
                  &mHandle,
                  &gEfiHeciSmmRuntimeProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mHeciProtocol
                  );
  DEBUG((EFI_D_INFO, " Heci 2 Prepare Done \n"));
  ASSERT_EFI_ERROR (Status);
}

VOID
EFIAPI
HeciSmmEndOfPostEvent (
  IN EFI_EVENT    Event,
  IN VOID*        Context
  )
/*++

Routine Description:

Arguments:

  Standard event notification function arguments:
  Event         - the event that is signaled.
  Context       - not used here.

Returns:

--*/
{

  UINT32          SecMode;

  SecMode  = HeciPciRead32(R_SEC_FW_STS0);

  if (SEC_MODE_NORMAL == (SecMode & 0xF0000)) {


    HeciSmmReady(NULL, NULL);
    //
    // Init the communicate buffer. The buffer data size is:
    //
    InitCommunicateBuffer (NULL, 0, SMM_HECI_MESSAGE_END_OF_POST);

    //
    // Send data to SMM.
    //
    SendCommunicateBuffer (0);
  }
  gBS->CloseEvent (Event);
}
VOID
EFIAPI
HeciSmmEndOfDXEEvent (
  IN EFI_EVENT    Event,
  IN VOID*      Context
  )
{
  UINT32          SecMode;
  SecMode  = HeciPciRead32(R_SEC_FW_STS0);
  if (SEC_MODE_NORMAL == (SecMode & 0xF0000)) {
    HeciSmmReady(NULL, NULL);
  }
//[-start-190315-IB07401091-add]//
#ifndef DISABLE_HECI2_HECI3_DISABLE_WORKAROUND
  HideHeciDevices ();
#endif
//[-end-190315-IB07401091-add]//

  gBS->CloseEvent (Event);
}

/*
  Event handle for Exit boot services

  @param[in] Event
  @param[in] ParentImageHandle
*/
VOID
EFIAPI
HeciSmmEndOfServicesEvent (
  IN EFI_EVENT           Event,
  IN VOID                *Context
  )
{

  UINT32          SecMode;

  SecMode  = HeciPciRead32(R_SEC_FW_STS0);
  if (SEC_MODE_NORMAL == (SecMode & 0xF0000)) {

  //
  // Init the communicate buffer. The buffer data size is:
  //
  InitCommunicateBuffer (NULL, 0, SMM_HECI_MESSAGE_END_OF_SERVICES);

  //
  // Send data to SMM.
  //
  SendCommunicateBuffer (0);
  //
  // This is the location where ExitBootService event occured for CSE and
  // almost at the End Of Service event. So keeping all HECI devices idle and
  // hiding HECI2 & 3 devices
  //
#ifndef FSP_FLAG
//[-start-190315-IB07401091-modify]//
#ifdef DISABLE_HECI2_HECI3_DISABLE_WORKAROUND
  PutDeviceIntoD0I3();
#else
  PutHeciDevicesIntoD0i3();
#endif
//[-end-190315-IB07401091-modify]//
#endif
  }
  gBS->CloseEvent (Event);
}

/**
  Variable Driver main entry point. The Variable driver places the 4 EFI
  runtime services in the EFI System Table and installs arch protocols
  for variable read and write services being available. It also registers
  a notification function for an EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       Variable service successfully initialized.
**/
EFI_STATUS
EFIAPI
HeciSmmRuntimeDxeInitialize (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_EVENT                                 Event;
  EFI_EVENT                                 EndOfDxeEvent;
  VOID                                     *RegistrationLocal;
//[-start-160904-IB07400778-add]//
  EFI_BOOT_MODE                             BootMode;
//[-end-160904-IB07400778-add]//

//[-start-160904-IB07400778-add]//
  BootMode = GetBootModeHob ();
  //
  // In BOOT_IN_RECOVERY_MODE, TXE is not exist.
  //
  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return EFI_SUCCESS;
  }
//[-end-160904-IB07400778-add]//

//[-start-190315-IB07401091-add]//
#ifndef DISABLE_HECI2_HECI3_DISABLE_WORKAROUND
  mHeci2BarAddress = Heci2PciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;
  mHeci3BarAddress = Heci3PciRead32 (R_HECIMBAR0) & 0xFFFFFFF0;

  DEBUG ((DEBUG_INFO, "HECI2 BAR read in HeciSmmRuntimeDxe = 0x%x.\n", mHeci2BarAddress));
  DEBUG ((DEBUG_INFO, "HECI3 BAR read in HeciSmmRuntimeDxe = 0x%x.\n", mHeci3BarAddress));
#endif  
//[-end-190315-IB07401091-add]//

  //
  // Create event
  //
  Status = gBS->CreateEventEx (
    EVT_NOTIFY_SIGNAL,
    TPL_NOTIFY,
    HeciSmmEndOfDXEEvent,
    NULL,
    &gEfiEndOfDxeEventGroupGuid,
    &EndOfDxeEvent
  );
  ASSERT_EFI_ERROR (Status);
  Status = gBS->CreateEvent (
          EVT_NOTIFY_SIGNAL,
          TPL_NOTIFY,
          HeciSmmEndOfPostEvent,
          (VOID *) NULL,
          &Event
          );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for an installation of protocol interface
  //

  Status = gBS->RegisterProtocolNotify (
          &gEfiCseEndofPostGuid,
          Event,
          &RegistrationLocal
          );
  ASSERT_EFI_ERROR (Status);

  //
  // Create event
  //
  Status = gBS->CreateEvent (
          EVT_NOTIFY_SIGNAL,
          TPL_NOTIFY,
          HeciSmmEndOfServicesEvent,
          (VOID *) NULL,
          &Event
          );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for an installation of protocol interface
  //

  Status = gBS->RegisterProtocolNotify (
          &gEfiCseEndofServicesGuid,
          Event,
          &RegistrationLocal
          );
  ASSERT_EFI_ERROR (Status);


  //
  // Register the event to convert the pointer for runtime.
  //
  gBS->CreateEventEx (
         EVT_NOTIFY_SIGNAL,
         TPL_NOTIFY,
         AddressChangeEvent,
         NULL,
         &gEfiEventVirtualAddressChangeGuid,
         &mVirtualAddressChangeEvent
         );

  //
  // Allocate memory for variable communicate buffer.
  //
  mHeciBufferSize  = sizeof (SMM_HECI_COMMUNICATE_HEADER) + MAX_HECI_BUFFER_SIZE;
  mHeciBuffer      = AllocateRuntimePool (mHeciBufferSize);
  ASSERT (mHeciBuffer != NULL);

  return EFI_SUCCESS;
}

