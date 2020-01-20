/** @file
  Implements Platform Trust Technology (FTPM) PTP ( Platform TPM Profile ) Device Library

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#include <Library/PerformanceLib.h>

#include "PttPtpRegs.h"
#include <Library/PttPtpLib.h>

#ifndef TPM_BASE
#define TPM_BASE                  0
#endif
#include <IndustryStandard/Tpm20.h>

#define PERF_ID_PTT_SUBMIT_COMMAND 0x3400

//[-start-160629-IB07400749-add]//
#ifdef EFI_DEBUG
#ifndef EFI_DEBUG_TXE_PTT_MESSAGE
#undef DEBUG
#define DEBUG(Expression)
#endif
#endif
//[-end-160629-IB07400749-add]//

#ifdef EFI_DEBUG
/**
  Prints command or response buffer for debugging purposes.

  @param[in] Buffer     Buffer to print.
  @param[in] BufferSize Buffer data length.

  @retval  None
**/
VOID
EFIAPI
PttHciPrintBuffer(IN UINT8 *Buffer, IN UINT32 BufferSize)
{
  UINT32 Index;

  DEBUG ((DEBUG_INFO, "Buffer Address: 0x%08x, Size: 0x%08x, Value:\n", Buffer, BufferSize));
  for(Index = 0; Index < BufferSize; Index++){
    DEBUG ((DEBUG_INFO, "%02x ", *(Buffer + Index)));
   if((Index+1) % 16 == 0) DEBUG ((DEBUG_INFO, "\n"));
  }
  DEBUG ((DEBUG_INFO, "\n"));
}
#endif // EFI_DEBUG

/**
  Copy data from the MMIO region to system memory by using 32-bit access.

  Copy data from the MMIO region specified by starting address StartAddress
  to system memory specified by Buffer by using 32-bit access. The total
  number of byte to be copied is specified by Length. Buffer is returned.

  If StartAddress is not aligned on a 32-bit boundary, then ASSERT().

  If Length is greater than (MAX_ADDRESS - StartAddress + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  If Length is not aligned on a 32-bit boundary, then ASSERT().
  If Buffer is not aligned on a 32-bit boundary, then ASSERT().

  @param[in]   StartAddress    The starting address for the MMIO region to be copied from.
  @param[in]   Length          The size, in bytes, of Buffer.
  @param[out]  Buffer          The pointer to a system memory buffer receiving the data read.

  @return Buffer
**/
UINT32 *
EFIAPI
MmioReadBuffer32 (
  IN  UINTN       StartAddress,
  IN  UINTN       Length,
  OUT UINT32      *Buffer
  )
{
  UINT32    *ReturnBuffer;

  ASSERT ((StartAddress & (sizeof (UINT32) - 1)) == 0);

  ASSERT ((Length - 1) <=  (MAX_ADDRESS - StartAddress));
  ASSERT ((Length - 1) <=  (MAX_ADDRESS - (UINTN) Buffer));

  ASSERT ((Length & (sizeof (UINT32) - 1)) == 0);
  ASSERT (((UINTN) Buffer & (sizeof (UINT32) - 1)) == 0);

  ReturnBuffer = Buffer;

  while (Length != 0) {
    *(Buffer++) = MmioRead32 (StartAddress);

    StartAddress += sizeof (UINT32);
    Length -= sizeof (UINT32);
  }

  return ReturnBuffer;
}

/**
  Copy data from system memory to the MMIO region by using 32-bit access.

  Copy data from system memory specified by Buffer to the MMIO region specified
  by starting address StartAddress by using 32-bit access. The total number
  of byte to be copied is specified by Length. Buffer is returned.

  If StartAddress is not aligned on a 32-bit boundary, then ASSERT().

  If Length is greater than (MAX_ADDRESS - StartAddress + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS -Buffer + 1), then ASSERT().

  If Length is not aligned on a 32-bit boundary, then ASSERT().

  If Buffer is not aligned on a 32-bit boundary, then ASSERT().

  @param[in]  StartAddress    The starting address for the MMIO region to be copied to.
  @param[in]  Length          The size, in bytes, of Buffer.
  @param[in]  Buffer          The pointer to a system memory buffer containing the data to write.

  @return Buffer
**/
UINT32 *
EFIAPI
MmioWriteBuffer32 (
  IN  UINTN        StartAddress,
  IN  UINTN        Length,
  IN  CONST UINT32 *Buffer
  )
{
  UINT32    *ReturnBuffer;

  ASSERT ((StartAddress & (sizeof (UINT32) - 1)) == 0);

  ASSERT ((Length - 1) <=  (MAX_ADDRESS - StartAddress));
  ASSERT ((Length - 1) <=  (MAX_ADDRESS - (UINTN) Buffer));

  ASSERT ((Length & (sizeof (UINT32) - 1)) == 0);
  ASSERT (((UINTN) Buffer & (sizeof (UINT32) - 1)) == 0);

  ReturnBuffer = (UINT32 *) Buffer;

  while (Length != 0) {
    MmioWrite32 (StartAddress, *(Buffer++));

    StartAddress += sizeof (UINT32);
    Length -= sizeof (UINT32);
  }

  return ReturnBuffer;
}

/**
  Sets PTT_CMD and CA_START register to a defined value to indicate that a command is
  available for processing.
  Any host write to this register shall result in an interrupt to the ME firmware.

  @param[in]  RequestedLocality

  @retval EFI_SUCCESS   Register successfully written.
  @retval TBD

**/
EFI_STATUS
EFIAPI
PttRequestLocality (
  IN     TPM_LOCALITY_ENUM RequestedLocality
  )
{

  UINT32      LocalityState;
  UINT32      ControlStatus;
  UINT32      WaitTime;

  ///
  /// Make sure TPM is not in fatal error state
  ///
  ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                               ( RequestedLocality * TPM_LOCALITY_BUFFER_SIZE )+
                               R_CRB_CONTROL_STS);

  if (( ControlStatus & B_CRB_CONTROL_STS_TPM_STATUS ) != 0 ){
    return EFI_DEVICE_ERROR;
  }

  LocalityState = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                               ( RequestedLocality * TPM_LOCALITY_BUFFER_SIZE )+
                               R_PTT_LOCALITY_STATE);

  if((((LocalityState & V_CRB_LOCALITY_STATE_ACTIVE_LOC_MASK) >> 2) == 0) &&
      ((LocalityState & B_CRB_LOCALITY_STATE_LOCALITY_ASSIGNED) != 0)     ){
    DEBUG ((DEBUG_INFO, "PTT Locality 0 already assigned\n"));
    return EFI_SUCCESS;
  } else {
    DEBUG ((DEBUG_INFO, "PTT Requesting Locality\n"));
  }

  ///
  /// Request access to locality
  ///
  MmioWrite32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                ( RequestedLocality * TPM_LOCALITY_BUFFER_SIZE )+
                R_TPM_LOCALITY_CONTROL,
                B_CRB_LOCALITY_CTL_REQUEST_ACCESS
                );
  ///
  /// Wait for assignment of locality
  ///
  LocalityState = 0;
  WaitTime = 0;
  while ((WaitTime < PTT_HCI_TIMEOUT_A) &&
          ((LocalityState & B_CRB_LOCALITY_STATE_REGISTER_VALID) != 0) &&
          ((LocalityState & B_CRB_LOCALITY_STATE_LOCALITY_ASSIGNED) != 0) &&
          (((LocalityState & V_CRB_LOCALITY_STATE_ACTIVE_LOC_MASK) >> 2) == 0)
          ){

    LocalityState = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                                 ( RequestedLocality * TPM_LOCALITY_BUFFER_SIZE )+
                                 R_PTT_LOCALITY_STATE);

    MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
    WaitTime += PTT_HCI_POLLING_PERIOD;
  }
  if (WaitTime >= PTT_HCI_TIMEOUT_A){
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Sets PTT_CMD and CA_START register to a defined value to indicate that a command is
  available for processing.
  Any host write to this register shall result in an interrupt to the ME firmware.

  @param[in]  RequestedLocality

  @retval EFI_SUCCESS   Register successfully written.
  @retval TBD

**/
VOID
EFIAPI
RelinquishLocality (
  IN     TPM_LOCALITY_ENUM RequestedLocality
  )
{
  ///
  /// Relinguish access to locality
  ///
  MmioWrite32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
              ( RequestedLocality * TPM_LOCALITY_BUFFER_SIZE )+
              R_TPM_LOCALITY_CONTROL,
              B_CRB_LOCALITY_CTL_RELINQUISH
              );
}

/**
  Sends command to PTT for execution.

  @param[in] PttBuffer    Buffer for TPM command data.
  @param[in] DataLength   TPM command data length.

  @retval EFI_SUCCESS     Operation completed successfully.
  @retval EFI_TIMEOUT     The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PttHciSend(
  IN     UINT8      *PttBuffer,
  IN     UINT32     DataLength
  )
{
  UINT32      ControlStatus;
  UINT32      WaitTime;

  ///
  /// Make sure that previous command was in fact completed if not, must not
  /// send subsequent commands
  ///
  ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                               ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                               R_CRB_CONTROL_START);

//[-start-160107-IB08450333-add]//
  WaitTime = 0;
  while ((WaitTime < PTT_HCI_TIMEOUT_E) &&
        ((ControlStatus & B_CRB_CONTROL_START) != 0)){

    ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                                 ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                                 R_CRB_CONTROL_START);

    MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
    WaitTime += PTT_HCI_POLLING_PERIOD;
  }
//[-end-160107-IB08450333-add]//

  if ((ControlStatus & B_CRB_CONTROL_START) != 0){
    DEBUG ((DEBUG_INFO, "Start bit was never cleared from previous command, cannot send another command\n"));
    return EFI_DEVICE_ERROR;
  }

  ///
  /// Make sure TPM is not in fatal error state
  ///
  ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                                 ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                                 R_CRB_CONTROL_STS);

  if (( ControlStatus & B_CRB_CONTROL_STS_TPM_STATUS ) != 0 ){
    return EFI_DEVICE_ERROR;
  }

//  if ((ControlStatus & B_CRB_CONTROL_STS_TPM_IDLE) != 0){
    DEBUG ((DEBUG_INFO, "PTT in idle state, must send command ready signal\n"));

    ///
    /// Request TPM to come out of idle
    ///
    MmioWrite32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                  ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                  R_CRB_CONTROL_REQ,
                  B_R_CRB_CONTROL_REQ_COMMAND_READY
                  );

    ///
    /// Wait for tpm to clear tpmidle
    ///
    WaitTime = 0;
    while ((WaitTime < PTT_HCI_TIMEOUT_C) &&
            ((ControlStatus & B_R_CRB_CONTROL_REQ_COMMAND_READY) != 0)){

      ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                                   ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                                   R_CRB_CONTROL_REQ);

      MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
      WaitTime += PTT_HCI_POLLING_PERIOD;
    }
    if (WaitTime >= PTT_HCI_TIMEOUT_C){
      return EFI_TIMEOUT;
    }
//  }

  ///
  /// Align command size to dword before writing to PTT_CRB
  ///
  if(DataLength % 4 != 0){
    DEBUG ((DEBUG_INFO, "Alignment: DataLength change from %d ", DataLength));
    DataLength += (4 - (DataLength % 4));
    DEBUG ((DEBUG_INFO, "to %d\n", DataLength));
  }

  MmioWriteBuffer32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                      ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                      R_CRB_DATA_BUFFER,
                      DataLength,
                       (UINT32*)PttBuffer );

  ///
  /// Trigger Command processing by writing to start register
  ///
  MmioWrite32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                R_CRB_CONTROL_START,
                B_CRB_CONTROL_START
                );

  return EFI_SUCCESS;
}

/**
  Receives response data of last command from PTT.

  @param[out] PttBuffer        Buffer for response data.
  @param[out] RespSize          Response data length.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_DEVICE_ERROR      Unexpected device status.
  @retval EFI_BUFFER_TOO_SMALL  Response data is too long.
**/
EFI_STATUS
EFIAPI
PttHciReceive(
  OUT     UINT8     *PttBuffer,
  OUT     UINT32    *RespSize
  )
{
  UINT32      ControlStatus;
  UINT32      WaitTime;
  UINT16      Data16;
  UINT32      Data32;
  EFI_STATUS  Status;
  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_INFO, "PTT: PttHciReceive start\n"));

  ///
  /// Wait for command completion
  ///
  WaitTime = 0;
  ControlStatus = 0;
  ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                               ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                               R_CRB_CONTROL_START);

  while ((WaitTime < PTT_HCI_TIMEOUT_E) &&
        ((ControlStatus & B_CRB_CONTROL_START) != 0)){

    ControlStatus = MmioRead32 ((UINTN) R_PTT_HCI_BASE_ADDRESS +
                                 ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                                 R_CRB_CONTROL_START);

    MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
    WaitTime += PTT_HCI_POLLING_PERIOD;
  }
  DEBUG ((DEBUG_INFO, "PTT recieve time = %d\n", WaitTime));
  if (WaitTime >= PTT_HCI_TIMEOUT_E){
    DEBUG ((DEBUG_INFO, "PTT recieve timeout = %x\n", WaitTime));
    return EFI_TIMEOUT;
  }
  ///
  /// Read the response data header
  ///
  MmioReadBuffer32((UINTN) R_PTT_HCI_BASE_ADDRESS +
                      ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                      R_CRB_DATA_BUFFER,
                      PTT_HCI_RESPONSE_HEADER_SIZE,
                     (UINT32*)PttBuffer);

  ///
  /// Check the reponse data header (tag, parasize and returncode)
  ///
  CopyMem (&Data16, PttBuffer, sizeof (UINT16));
  DEBUG ((DEBUG_INFO, "TPM2_RESPONSE_HEADER.tag = 0x%04x\n", SwapBytes16(Data16)));

  ///
  /// TPM Rev 2.0 Part 2 - 6.9 TPM_ST (Structure Tags)
  /// TPM_ST_RSP_COMMAND - Used in a response that has an error in the tag.
  ///
  if (SwapBytes16(Data16) == TPM_ST_RSP_COMMAND) {
    DEBUG ((DEBUG_ERROR, "TPM2_RESPONSE_HEADER.tag = TPM_ST_RSP_COMMAND - Error in response!\n"));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  CopyMem(&Data32, (PttBuffer + 2), sizeof(UINT32));
  DEBUG ((DEBUG_INFO, "TPM2_RESPONSE_HEADER.paramSize = 0x%08x\n", SwapBytes32(Data32)));

  *RespSize = SwapBytes32(Data32);

  if(*RespSize == sizeof(TPM2_RESPONSE_HEADER)) {
    Status = EFI_SUCCESS;
    goto Exit;
  }
  if(*RespSize < sizeof(TPM2_RESPONSE_HEADER)) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }
  if(*RespSize > S_PTT_HCI_CRB_LENGTH) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }

  ///
  /// Align command size to dword before writing to PTT_CRB
  ///
  if(*RespSize % 4 != 0){
    DEBUG ((DEBUG_INFO, "Alignment: RespSize change from %d ", *RespSize));
    *RespSize += (4 - (*RespSize % 4));
    DEBUG ((DEBUG_INFO, "to %d\n", *RespSize));
  }

  ///
  /// Read the entire response data header
  ///
  MmioReadBuffer32((UINTN) R_PTT_HCI_BASE_ADDRESS +
                      ( TPM_LOCALITY_0 * TPM_LOCALITY_BUFFER_SIZE )+
                      R_CRB_DATA_BUFFER,
                      *RespSize,
                     (UINT32*)PttBuffer);

  Exit:

    return Status;
}

/**
  Sends formatted command to PTT for execution and returns formatted response data.

  @param[in]  InputBuffer       Buffer for the input data.
  @param[in]  InputBufferSize   Size of the input buffer.
  @param[out] ReturnBuffer      Buffer for the output data.
  @param[out] ReturnBufferSize  Size of the output buffer.

  @retval EFI_SUCCESS  Operation completed successfully.
  @retval EFI_TIMEOUT  The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PttHciSubmitCommand(
  IN      UINT8     *InputBuffer,
  IN      UINT32     InputBufferSize,
  OUT     UINT8     *ReturnBuffer,
  OUT     UINT32    *ReturnBufferSize
  )
{
  EFI_STATUS Status;
  DEBUG ((DEBUG_INFO, "PTT: PttHciSubmitCommand start\n"));
  PERF_START_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND);

  if(InputBuffer == NULL || ReturnBuffer == NULL || InputBufferSize == 0){
    DEBUG ((DEBUG_ERROR, "Buffer == NULL or InputBufferSize == 0\n"));
    PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 3);
    return EFI_INVALID_PARAMETER;
  }

  Status = PttRequestLocality(TPM_LOCALITY_0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PTT Locality Request timed out due to FWSTS4.BIT19 not set. EFI_ERROR = %r\n", Status));
    return Status;
  }

 ///
 /// PTT_CA_CMD - the physical address to which the TPM 2.0 driver will write the command to execute
 ///
 MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_CRB_CONTROL_CMD_LOW), R_PTT_HCI_BASE_ADDRESS + R_CRB_DATA_BUFFER);
 MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_CRB_CONTROL_CMD_SIZE), S_PTT_HCI_CRB_LENGTH);

  ///
  /// PTT_CA_CMD - the physical address to which the TPM 2.0 driver will write the command to execute
  ///
  MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_CRB_CONTROL_RESPONSE_ADDR), R_PTT_HCI_BASE_ADDRESS + R_CRB_DATA_BUFFER);
  MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_CRB_CONTROL_RESPONSE_SIZE), S_PTT_HCI_CRB_LENGTH);

  ///
  /// Send the command to TPM
  ///
  #ifdef EFI_DEBUG
  DEBUG ((DEBUG_INFO, "PTT Buffer Dump: Command \n"));
  PttHciPrintBuffer ( InputBuffer, InputBufferSize);
  #endif // EFI_DEBUG
  Status = PttHciSend(InputBuffer, InputBufferSize);
  if (EFI_ERROR (Status))  {
    DEBUG ((DEBUG_ERROR, "PttHciSend EFI_ERROR = %r\n", Status));
    PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 4);
    return Status;
  }

  ///
  /// Receive the response data from TPM
  ///
  Status = PttHciReceive(ReturnBuffer, ReturnBufferSize);
  #ifdef EFI_DEBUG
  DEBUG ((DEBUG_INFO, "PTT Buffer Dump: Response \n"));
  PttHciPrintBuffer ( ReturnBuffer, *ReturnBufferSize);
  #endif // EFI_DEBUG
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PttHciReceive EFI_ERROR = %r\n", Status));
    PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 5);
    return Status;
  }

  PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 1);
  return Status;
}
