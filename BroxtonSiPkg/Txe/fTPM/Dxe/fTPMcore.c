/** @file
  fTPM driver core. For DXE Phase, determines the HECI device and initializes it.

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
#include "fTPMDrv.h"
#include "fTPMCore.h"
#include "HeciRegs.h"
#include "SeCState.h"
#include "Library/BaseMemoryLib.h"

static UINT32 HeciMBAR = 0, HeciMBAR0=0;

//
// Extern for shared HECI data and protocols
//
extern FTPM_INSTANCE  *mfTPMContext;

//
// //////////////////////////////////////////////////////////////////////////////////
// Forward declaration
////////////////////////////////////////////////////////////////////////////////////
//

/**

  For serial debugger used, it will show the buffer message to serial console.

  @param[in]  Message - the address point of buffer message
  @param[in]  Length  - message length

  @retval None.

**/
VOID
ShowBuffer (
  UINT8  *Message,
  UINT32 Length
  )
{
  UINT32  LineBreak;
  UINT32  Index;
  LineBreak = 0;
  Index     = 0;

  while (Length-- > 0) {
    if (LineBreak == 0) {
      DEBUG ((EFI_D_INFO, "%02x: ", (Index & 0xF0)));
    }

    DEBUG ((EFI_D_INFO, "%02x ", Message[Index++]));
    LineBreak++;
    if (LineBreak == 16) {
      DEBUG ((EFI_D_INFO, "\n"));
      LineBreak = 0;
    }

    if (LineBreak == 8) {
      DEBUG ((EFI_D_INFO, "- "));
    }
  }

  DEBUG ((EFI_D_INFO, "\n"));
  return ;
}

////////////////////////////////////////////////////////////////////////////////////
// Heci driver function definitions
////////////////////////////////////////////////////////////////////////////////////

/**

  Determines if the HECI device is present and, if present, initializes it for
  use by the BIOS.

  @param[in]  None.

  @retval EFI_STATUS.

**/
EFI_STATUS
fTPMInitialize (
  VOID
  )
{
  ///
  /// @todo: Need to define
  ///
  return EFI_SUCCESS;
}

/**

  Heci Re-initializes it for Host

  @param[in]  None.

  @retval EFI_STATUS.

**/
EFI_STATUS
EFIAPI
fTPMReInitialize (
  VOID
  )
{
   ///
   /// @todo: Need to check
   ///

  DEBUG ((EFI_D_INFO, "fTPMReInitialize--\n\n "));
   return EFI_SUCCESS;
}

/**

  Function to pull one message packet off.
    for reference BWG0.7

  @param[in]  Message   - Pointer to a buffer to receive the message in

  @retval EFI_STATUS.

**/
EFI_STATUS
PTTHciPacketRead (
  OUT     UINT8                    *Message
  )
{
  volatile FTPM_CMD  *ftpmCmd;
  UINT32 cnt=0;
  UINT32 sattLsb;
  UINT32 sattMsb;

  sattLsb = HeciPciRead32(R_SATT_PTT_BRG_BA_LSB);
  sattMsb = (HeciPciRead32(R_SATT_PTT_CTRL)>> 8)&& 0x0f;
  ftpmCmd = (VOID *) (sattLsb + PTT_CMD_BUFFER_OFFSET);

  if(sattMsb != 0)
  {
    //  sattAdr =  (sattMsb<<32) || (sattLsb && 0xffffffff);
    //  ftpmCmd = (void *) sattAdr;
      DEBUG ((EFI_D_ERROR, "Error! no 64 bits support"));
      CpuDeadLoop();
  }

  DEBUG ((EFI_D_INFO, "PacketRead :\t "));
  for(cnt=0;cnt<ftpmCmd->ul[5];cnt++)
  {
     Message[cnt] = ftpmCmd->ul[cnt];
     DEBUG ((EFI_D_INFO, "%d [%x],  ",Message[cnt],Message[cnt]));
  }
  CpuDeadLoop();

  DEBUG ((EFI_D_INFO, "\n\nfTPMPacketRead--\n\n "));

  return EFI_SUCCESS;
}
/**

  Reads a message from the SEC across HECI.

  @param[in]  Blocking    - Used to determine if the read is BLOCKING or NON_BLOCKING.
  @param[in]  MessageBody - Pointer to a buffer used to receive a message.
  @param[in]  Length      - Pointer to the length of the buffer on input and the length
                            of the message on return. (in bytes)

  @retval EFI_STATUS.

**/
EFI_STATUS
EFIAPI
HeciReceive (
  IN      UINT32  Blocking,
  IN OUT  UINT32  *MessageBody,
  IN OUT  UINT32  *Length
  )
{

/*  HECI_MESSAGE_HEADER PacketHeader;
  UINT32              CurrentLength;
  UINT32              MessageComplete;
  EFI_STATUS          ReadError;
  EFI_STATUS          Status;
  UINT32              PacketBuffer;
  UINT32              timer_start;
  UINT32              timer_end;
  BOOLEAN             QuitFlag;

  CurrentLength   = 0;
  MessageComplete = 0;
  Status          = EFI_SUCCESS;
  QuitFlag        = FALSE;

  DEBUG ((EFI_D_ERROR, "HeciReceive  ++ \n "));


  do {
    //
    // Make sure that HECI device BAR is correct and device is enabled.
    //
    HeciMBAR = CheckAndFixHeciForAccess ();

    //
    // Make sure we do not have a HECI reset
    //
    if (CheckForHeciReset ()) {
      //
      // if HECI reset than try to re-init HECI
      //
      Status = HeciInitialize ();

      if (EFI_ERROR (Status)) {
        Status = EFI_DEVICE_ERROR;
        break;
      }
    }
    //
    // Make sure that HECI is ready for communication.
    //
    if (WaitForSECReady () != EFI_SUCCESS) {
      Status = EFI_TIMEOUT;
      break;
    }
    //
    // Set up timer for BIOS timeout.
    //
    StartTimer (&timer_start, &timer_end, HECI_READ_TIMEOUT);
    while ((CurrentLength < *Length) && (MessageComplete == 0)) {
      //
      // If 1 second timeout has expired, return fail as we have not yet received a full message
      //
      if (Timeout (timer_start, timer_end) != EFI_SUCCESS) {
        Status    = EFI_TIMEOUT;
        QuitFlag  = TRUE;
        break;
      }

      PacketBuffer = *Length - CurrentLength;
      ReadError = HECIPacketRead (
                    Blocking,
                    &PacketHeader,
                    (UINT32 *) &MessageBody[CurrentLength / 4],
                    &PacketBuffer
                    );

      //
      // Check for error condition on read
      //
      if (EFI_ERROR (ReadError)) {
        *Length   = 0;
        Status    = ReadError;
        QuitFlag  = TRUE;
        break;
      }
      //
      // Get completion status from the packet header
      //
      MessageComplete = PacketHeader.Fields.MessageComplete;

      //
      // Check for zero length messages
      //
      if (PacketBuffer == 0) {
        //
        // If we are not in the middle of a message, and we see Message Complete,
        // this is a valid zero-length message.
        //
        if ((CurrentLength == 0) && (MessageComplete == 1)) {
          *Length   = 0;
          Status    = EFI_SUCCESS;
          QuitFlag  = TRUE;
          break;
        } else {
          //
          // We should not expect a zero-length message packet except as described above.
          //
          *Length   = 0;
          Status    = EFI_DEVICE_ERROR;
          QuitFlag  = TRUE;
          break;
        }
      }
      //
      // Track the length of what we have read so far
      //
      CurrentLength += PacketBuffer;

    }

    if (QuitFlag == TRUE) {
      break;
    }
    //
    // If we get here the message should be complete, if it is not
    // the caller's buffer was not large enough.
    //
    if (MessageComplete == 0) {
      *Length = 0;
      Status  = EFI_BUFFER_TOO_SMALL;
      break;
    }

    *Length = CurrentLength;

    DEBUG ((EFI_D_ERROR, "HECI ReadMsg:\n"));
#ifdef EFI_DEBUG
    DEBUG_CODE (
      ShowBuffer ((UINT8 *) MessageBody, *Length);
    );
#endif
  } while (EFI_ERROR (Status));

  RestoreHpet ();
*/
  return EFI_SUCCESS;
}
/**

  Function sends one command (of any length) through the HCI buffer.

  @param[in]  Message     - Pointer to the message data to be sent.

  @retval EFI_STATUS.

**/
EFI_STATUS
EFIAPI
fTPMCmdSend (
  IN UINT8                     *Message
  )
{
  //
  // Send command to fTPM
  //
  UINT32 ppt_ICR;
  //
  // Send HCI packet to SeC
  //
  SetMem((VOID*)(UINTN)0x7ff00000, 0x100000, 0);
  PTTHciPacketWrite(Message);
  //
  // set
  //


  //
  // set PPT_ICR
  //
//  HeciPciWrite8(PTT_ICR, 0x1);
  HeciPciOr8(PTT_ICR, 0x1);




  do{
      ppt_ICR = *(UINT32*)(UINTN)0x7ff0000c; //HeciPciRead32(PTT_ICR);
  }while(ppt_ICR);

//  CpuDeadLoop();
  PTTHciPacketRead(Message);
   CpuDeadLoop();
  return EFI_SUCCESS;
}


/**

  Function sends one message packet through the HCI buffer
   for reference see BWG0.7

  @param[in]  MessageData   - Pointer to the actual message data.

  @retval EFI_STATUS.

**/
EFI_STATUS
PTTHciPacketWrite (
  IN  UINT8                    *Message
  )
{
   volatile FTPM_CMD  *ftpmCmd;
   UINT32 baseAddr=0;
   UINT32 cnt=0;

   baseAddr = 0x7ff00000;//getfTPMBaseAddr();

   ftpmCmd = (VOID *) (baseAddr + PTT_CMD_BUFFER_OFFSET);
   *(UINT32*)(UINTN)0x7ff0000c = 1;
   *(UINT32*)(UINTN)0x7ff00040 = 1;
   CopyMem((VOID*)ftpmCmd, Message, 0xC);
   DEBUG ((EFI_D_INFO, "PacketWrite :\t "));
   for(cnt=0;cnt<Message[5];cnt++)
   {
//     ftpmCmd->ul[cnt] = Message[cnt];
     DEBUG ((EFI_D_INFO, "%d [%x],  ",Message[cnt],Message[cnt]));
   }

  return EFI_SUCCESS;
}

/**

  Function sends one messsage through the HECI buffer and waits
    for the corresponding ACK message.

  @param[in]  Message     - Pointer to the message buffer.
  @param[in]  SendLength  - Length of the message in bytes.
  @param[in]  RecLength   - Length of the message response in bytes.
  @param[in]  HostAddress - Address of the sending entity.
  @param[in]  MeAddress   - Address of the SEC entity that should receive the message.

  @retval EFI_STATUS.

**/
EFI_STATUS
EFIAPI
HeciSendwACK (
  IN OUT  UINT32  *Message,
  IN      UINT32  Length,
  IN OUT  UINT32  *RecLength,
  IN      UINT8   HostAddress,
  IN      UINT8   SeCAddress
  )
{
/*  EFI_STATUS  Status;
  UINT16      RetryCount;
  UINT32      TempRecLength;
  UINT32      SeCMode;

  HeciGetSeCMode(&SeCMode);
  if (SeCMode == SEC_MODE_RECOVER) {
    return EFI_UNSUPPORTED;
  }
  //
  // Send the message
  //
  Status = HeciSend (Message, Length, HostAddress, SeCAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Wait for ACK message
  //
  TempRecLength = *RecLength;
  for (RetryCount = 0; RetryCount < HECI_MAX_RETRY; RetryCount++) {
    //
    // Read Message
    //
    Status = HeciReceive (BLOCKING, Message, &TempRecLength);
    if (!EFI_ERROR (Status)) {
      break;
    }
    //
    // Reload receive length as it has been modified by the read function
    //
    TempRecLength = *RecLength;
  }
  //
  // Return read length and status
  //
  *RecLength = TempRecLength;
  return Status;
  */
  return 0;
}

/**

  SeC reset and waiting for ready

  @param[in]  Delay - The biggest waiting time

  @retval EFI_TIMEOUT - Time out
  @retval  EFI_SUCCESS - SeC ready

**/
EFI_STATUS
EFIAPI
SeCResetWait (
  IN  UINT32  Delay
  )
{
/*  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  UINT32                      TimerStart;
  UINT32                      TimerEnd;

  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Wait for the HOST Ready bit to be cleared to signal a reset
  //
  StartTimer (&TimerStart, &TimerEnd, Delay);
  HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  while (HeciRegHCsr.r.H_RDY == 1) {
    //
    // If timeout has expired, return fail
    //
    if (Timeout (TimerStart, TimerEnd) != EFI_SUCCESS) {
      return EFI_TIMEOUT;
    }

    HeciRegHCsr.ul = MMIOREADDWORD (HeciMBAR + H_CSR);
  }
*/
  return EFI_SUCCESS;
}
/**

  TODO: need to be define

  @param[in]  none

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
ResetfTPM (
  VOID
  )
{

  return EFI_SUCCESS;
}
/**

  Return SEC Status

  @param[in]  SeCStatus pointer for status report

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciGetSeCStatus (
  IN UINT32                     *SeCStatus
  )
{
  HECI_FWS_REGISTER SeCFirmwareStatus;

  if (SeCStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

  if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_NORMAL && SeCFirmwareStatus.r.ErrorCode == SEC_ERROR_CODE_NO_ERROR) {
    *SeCStatus = SEC_READY;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_RECOVERY) {
    *SeCStatus = SEC_IN_RECOVERY_MODE;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_INIT) {
    *SeCStatus = SEC_INITIALIZING;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_DISABLE_WAIT) {
    *SeCStatus = SEC_DISABLE_WAIT;
  } else if (SeCFirmwareStatus.r.CurrentState == SEC_STATE_TRANSITION) {
    *SeCStatus = SEC_TRANSITION;
  } else {
    *SeCStatus = SEC_NOT_READY;
  }

  if (SeCFirmwareStatus.r.FwUpdateInprogress) {
    *SeCStatus |= SEC_FW_UPDATES_IN_PROGRESS;
  }

  if (SeCFirmwareStatus.r.FwInitComplete == SEC_FIRMWARE_COMPLETED) {
    *SeCStatus |= SEC_FW_INIT_COMPLETE;
  }

  if (SeCFirmwareStatus.r.SeCBootOptionsPresent == SEC_BOOT_OPTIONS_PRESENT) {
    *SeCStatus |= SEC_FW_BOOT_OPTIONS_PRESENT;
  }

  DEBUG ((EFI_D_INFO, "HECI SeCStatus %X\n", SeCFirmwareStatus.ul));

  return EFI_SUCCESS;
}
/**

  Return SEC Mode

  @param[in]  SeCMode pointer for SEC Mode report

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
HeciGetSeCMode (
  IN UINT32                     *SeCMode
  )
{
  HECI_FWS_REGISTER SeCFirmwareStatus;

  if (SeCMode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);

    switch (SeCFirmwareStatus.r.SeCOperationMode) {
    case SEC_OPERATION_MODE_NORMAL:
      *SeCMode = SEC_MODE_NORMAL;
      break;

    case SEC_OPERATION_MODE_ALT_DISABLED:
      *SeCMode = SEC_DEBUG_MODE_ALT_DIS; //debug Mode
      break;

    case SEC_OPERATION_MODE_SOFT_TEMP_DISABLE:
      *SeCMode = SEC_MODE_TEMP_DISABLED;
      break;

    case SEC_OPERATION_MODE_SECOVR_JMPR:
    case SEC_OPERATION_MODE_SECOVR_HECI_MSG:
      *SeCMode = SEC_MODE_RECOVER;
      break;

    default:
      *SeCMode = SEC_MODE_FAILED;
    }
  DEBUG ((EFI_D_INFO, "HECI SeCMode %X\n", SeCFirmwareStatus.r.SeCOperationMode));

  return EFI_SUCCESS;
}
/**

  Send start command for fTPM

  @param[in]  Start command type

  @retval EFI_STATUS

**/
EFI_STATUS
EFIAPI
StartCmd (
  IN UINT32                     *type
  )
{
  UINT8    cmdData[12] = {128,1,0,0,0,12,0,0,1,68,0,0};

  fTPMCmdSend (cmdData);  // Send command

  return 0;
}

/**

  getfTPMBaseAddr

  @param[in]  none

  @retval UINT32

**/
UINT32
getfTPMBaseAddr(void)
{
  UINT32 sattLsb;
  UINT32 sattMsb;

  sattLsb = HeciPciRead32(R_SATT_PTT_BRG_BA_LSB);
  sattMsb = (HeciPciRead32(R_SATT_PTT_CTRL)>> 8)&& 0x0f;


  if(sattMsb != 0)
  {
      DEBUG ((EFI_D_ERROR, "Error! no 64 bits support"));
      CpuDeadLoop();
  }
  return sattLsb;
}

