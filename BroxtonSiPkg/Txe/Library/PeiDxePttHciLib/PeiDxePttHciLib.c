/** @file
  Implements Platform Trust Technology (PTT) HCI Device Library.

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

@par Specification Reference:
**/

#include <Library/PerformanceLib.h>

#include "PttHciRegs.h"
#include <Library/PttHciLib.h>

#ifndef TPM_BASE
#define TPM_BASE                  0
#endif
#include <IndustryStandard/Tpm20.h>

#define PERF_ID_PTT_SUBMIT_COMMAND 0x3400

#ifdef EFI_DEBUG
/**
  Prints command or response buffer for debugging purposes.

  @param[in] Buffer     Buffer to print.
  @param[in] BufferSize Buffer data length.
**/
VOID
EFIAPI
PttHciPrintBuffer(
  IN UINT8 *Buffer,
  IN UINT32 BufferSize
  )
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

  @retval UINT32*              Data from Address
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

  @retval UINT32*              Data from Address
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
  Checks whether PTT is enabled (PTT_STS::PTT_EN).

  @retval    TRUE    PTT is enabled.
  @retval    FALSE   PTT is disabled. All LT writes will be dropped.
                     All LT reads will be returned with read data value of all 0's.
                     The bit can only be written once per ME power cycle.
**/
BOOLEAN
EFIAPI
PttHciPresenceCheck (
  VOID
  )
{
  EFI_STATUS Status;
  DEBUG ((DEBUG_INFO, "PTT: PttHciPresenceCheck start\n"));

  DEBUG ((DEBUG_INFO, "Check PTT_STS - ENABLED bit (@ 0x%08x)\n", (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_STS)));
  Status = PttHciWaitRegisterBits(
            (EFI_PHYSICAL_ADDRESS)(UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_STS),
            B_PTT_HCI_STS_ENABLED,
            V_PTT_HCI_IGNORE_BITS,
            PTT_HCI_TIMEOUT_A
            );
  if(Status == EFI_SUCCESS){
    return TRUE;
  }

  return FALSE;
}

/**
  Checks whether PTT is Ready

  @retval    TRUE    PTT is ready.
  @retval    FALSE   PTT is not ready
**/
BOOLEAN
EFIAPI
PttHciReadyCheck (
  VOID
  )
{
  UINT32 RegRead;
  UINT32 WaitTime;

  for (WaitTime = 0; WaitTime < PTT_HCI_TIMEOUT_B*5; WaitTime += PTT_HCI_POLLING_PERIOD){
    RegRead = MmioRead32 ((UINTN) ( R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_STS ));
    DEBUG ((DEBUG_INFO, "Check PTT_STS - READY bit (@ 0x%08x)\n", RegRead));

    if(( B_PTT_HCI_STS_READY & RegRead) != 0){
      return TRUE;
    }

    MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
  }

  return FALSE;
}

/**
  Checks whether TPM2_Startup command has been executed (PTT_STS::STARTUP_EXEC).
  If command was executed, it should not be redundantly issued again.

  @retval    TRUE    Startup command executed already.
  @retval    FALSE   Startup command not executed yet.
**/
BOOLEAN
EFIAPI
PttHciStartupExecuted (
  VOID
  )
{
  EFI_STATUS Status;

  DEBUG ((DEBUG_INFO, "PTT: PttHciStartupExecuted start\n"));
  DEBUG ((DEBUG_INFO, "Check PTT_STS - STARTUP_EXECECUTED bit (@ 0x%08x)\n", (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_STS)));
  Status = PttHciWaitRegisterBits(
            (EFI_PHYSICAL_ADDRESS)(UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_STS),
            B_PTT_HCI_STS_STARTUP_EXEC,
            V_PTT_HCI_IGNORE_BITS,
            PTT_HCI_TIMEOUT_A
            );
  if(Status == EFI_SUCCESS){
    return TRUE;
  }

  return FALSE;
}

/**
  Sets PTT_CMD and CA_START register to a defined value to indicate that a command is
  available for processing.
  Any host write to this register shall result in an interrupt to the ME firmware.

  @retval EFI_SUCCESS   Register successfully written.
**/
EFI_STATUS
EFIAPI
PttHciRequestCommandExec (
  VOID
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "PTT: PttHciRequestCommandExec start\n"));

  DEBUG ((DEBUG_INFO, "Command ready for processing - write 0x%08x to PTT_CA_START register (@ 0x%08x)\n",
                      V_PTT_HCI_COMMAND_AVAILABLE_START,
                      (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_START)));
  MmioWrite32((UINTN)R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_START, V_PTT_HCI_COMMAND_AVAILABLE_START);

  ///
  /// Write 0x1 to HCI CMD register to indicate that a command is available for processing
  ///
  DEBUG ((DEBUG_INFO, "Command ready for processing - write 0x%08x to PTT_CMD register (@ 0x%08x)\n",
                      V_PTT_HCI_COMMAND_AVAILABLE_CMD,
                      (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CMD)));
  MmioWrite32((UINTN)R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CMD, V_PTT_HCI_COMMAND_AVAILABLE_CMD);

  return Status;
}

/**
  Checks whether the value of a PTT register satisfies the input BIT setting.

  @param[in]  Register     Address port of register to be checked.
  @param[in]  BitSet       Check these data bits are set.
  @param[in]  BitClear     Check these data bits are clear.
  @param[in]  TimeOut      The max wait time (unit MicroSecond) when checking register.

  @retval     EFI_SUCCESS  The register satisfies the check bit.
  @retval     EFI_TIMEOUT  The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PttHciWaitRegisterBits(
  IN      EFI_PHYSICAL_ADDRESS    RegAddress,
  IN      UINT32                  BitSet,
  IN      UINT32                  BitClear,
  IN      UINT32                  TimeOut
  )
{
  UINT32 RegRead;
  UINT32 WaitTime;

  DEBUG ((DEBUG_INFO, "PTT: PttHciWaitRegisterBits start\n"));

  for (WaitTime = 0; WaitTime < TimeOut; WaitTime += PTT_HCI_POLLING_PERIOD){
    RegRead = MmioRead32 ((UINTN)RegAddress);
    DEBUG ((DEBUG_INFO, "RegRead: 0x%08x, BitSetMask: 0x%08x, BitClearMask: 0x%08x, WaitTime: %d (microsec)\n", RegRead, BitSet, BitClear, WaitTime));

    if (RegRead == 0xFFFFFFFF)
      continue;

    if ((RegRead & BitSet) == BitSet && (RegRead & BitClear) == 0) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay (PTT_HCI_POLLING_PERIOD);
  }
  return EFI_TIMEOUT;
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
  EFI_STATUS Status;

  DEBUG ((DEBUG_INFO, "PTT: PttHciSend start\n"));

  Status = PttHciWaitRegisterBits(
            (EFI_PHYSICAL_ADDRESS)(UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_START),
            V_PTT_HCI_IGNORE_BITS,
            V_PTT_HCI_START_CLEAR,
            PTT_HCI_TIMEOUT_A
            );

  if(EFI_ERROR (Status)){
    DEBUG ((DEBUG_ERROR, "PTT_CA_START register not clear - TPM2 command cannot be sent! EFI_ERROR = %r\n", Status));
    return EFI_NOT_READY;
  }
  ///
  /// Align command size to dword before writing to PTT_CRB
  ///
  if(DataLength % 4 != 0){
    DEBUG ((DEBUG_INFO, "Alignment: DataLength change from %d ", DataLength));
    DataLength += (4 - (DataLength % 4));
    DEBUG ((DEBUG_INFO, "to %d\n", DataLength));
  }

  MmioWriteBuffer32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB), (UINTN)DataLength, (UINT32*)PttBuffer);

  ///
  /// PTT_CA_CMD - the physical address to which the TPM 2.0 driver will write the command to execute
  ///
  MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_CMD), R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB);
  MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_CMD_SZ), S_PTT_HCI_CRB_LENGTH);

  DEBUG ((DEBUG_INFO, "PTT_CA_CMD (@ 0x%08x) written, value = 0x%08x\n",
                       (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_CMD),
                       R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB));
  DEBUG ((DEBUG_INFO, "PTT_CA_CMD_SZ (@ 0x%08x) written, value = 0x%08x\n",
                       (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_CMD_SZ), DataLength));

  ///
  /// Set PTT_CMD and PTT_CA_START registers to indicate TPM command ready for execution
  ///
  Status = PttHciRequestCommandExec();
#ifdef EFI_DEBUG
  if(Status == EFI_SUCCESS){
    DEBUG ((DEBUG_INFO, "PTT_CMD register written - TPM2 command available for processing\n"));
  }
#endif

  return Status;
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
  EFI_STATUS Status;
  UINT16 Data16;
  UINT32 Data32;
  DEBUG ((DEBUG_INFO, "PTT: PttHciReceive start\n"));

  ///
  /// Wait for the command completion - poll PTT_CA_START clear
  ///
  DEBUG ((DEBUG_INFO, "PTT: Check Start status (PTT_CA_START)\n"));
  Status = PttHciWaitRegisterBits(
            (EFI_PHYSICAL_ADDRESS)(UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_START),
            V_PTT_HCI_IGNORE_BITS,
            V_PTT_HCI_START_CLEAR,
            PTT_HCI_TIMEOUT_D
            );
  if(EFI_ERROR (Status)){
    DEBUG ((DEBUG_ERROR, "PTT_CA_START register not clear - TPM2 response cannot be read! EFI_ERROR = %r\n", Status));
    goto Exit;
  }

  ///
  /// Check for error condition - PTT_CA_ERROR
  ///
  DEBUG ((DEBUG_INFO, "PTT: Check Error status (PTT_CA_ERROR)\n"));
  Status = PttHciWaitRegisterBits(
            (EFI_PHYSICAL_ADDRESS)(UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_ERROR),
            V_PTT_HCI_IGNORE_BITS,
            V_PTT_HCI_ALL_BITS_CLEAR,
            PTT_HCI_TIMEOUT_A
            );
  if(EFI_ERROR (Status)){
    DEBUG ((DEBUG_ERROR, "PTT_CA_ERROR register set - TPM2 response cannot be provided! EFI_ERROR = %r\n", Status));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }

  DEBUG ((DEBUG_INFO, "PTT_CA_START register clear - TPM2 command processing completed - ready to read\n"));

  ///
  /// Read the response data header
  ///
  MmioReadBuffer32((UINTN)R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB, PTT_HCI_RESPONSE_HEADER_SIZE, (UINT32*)PttBuffer);

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
  /// Reading the entire response data
  ///
  MmioReadBuffer32((UINTN)R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB, *RespSize, (UINT32*)PttBuffer);

  Exit:
    if(!EFI_ERROR(Status)){
      ///
      /// PTT_CA_CMD - the physical address from which the TPM 2.0 driver will read command responses
      ///
      MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_RSP), R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB);
      MmioWrite32((UINTN)(R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_RSP_SZ),S_PTT_HCI_CRB_LENGTH);
      DEBUG ((DEBUG_INFO, "PTT_CA_RSP (@ 0x%08x) written, value = 0x%08x\n",
                          (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_RSP),
                          (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CRB)));
      DEBUG ((DEBUG_INFO, "PTT_CA_RSP_SZ (@ 0x%08x) written, value = 0x%08x\n",
                          (R_PTT_HCI_BASE_ADDRESS + R_PTT_HCI_CA_RSP_SZ), *RespSize));
    }

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

  if(!PttHciReadyCheck()){
    DEBUG ((DEBUG_ERROR, "PTT device not ready. \n"));
    PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 2);
    return EFI_NOT_READY;
  }

  if(InputBuffer == NULL || ReturnBuffer == NULL || InputBufferSize == 0){
    DEBUG ((DEBUG_ERROR, "Buffer == NULL or InputBufferSize == 0\n"));
    PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 3);
    return EFI_INVALID_PARAMETER;
  }

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

  DEBUG ((DEBUG_INFO, "PTT: Response Buffer dump\n"));
  PERF_END_EX (NULL, "EventRec", "PttHciDeviceLib", AsmReadTsc(), PERF_ID_PTT_SUBMIT_COMMAND + 1);
  return Status;
}

