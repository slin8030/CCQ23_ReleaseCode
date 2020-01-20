/** @file
  Platform Trust Technology (FTPM) HCI Device Library

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
#ifndef _PTT_HCI_LIB_H_
#define _PTT_HCI_LIB_H_

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>

///
/// Default Timeout values
/// @todo these values should be adjusted on actual silicon after power-on
///
#define PTT_HCI_TIMEOUT_A         500          ///< 500 microseconds
#define PTT_HCI_TIMEOUT_B         10 * 1000    ///< 10ms (max command processing time in PK-TPM ca. 3ms)
#define PTT_HCI_TIMEOUT_C         1000 * 1000  ///< 1s
#define PTT_HCI_TIMEOUT_D         500 * 1000   ///< 500 ms
#define PTT_HCI_POLLING_PERIOD    140          ///< Poll register every 140 microsecondss

/// TPM2_RESPONSE_HEADER size (10B) aligned to dword
#define PTT_HCI_RESPONSE_HEADER_SIZE  12 ///< 12B

/**
  Checks whether FTPM is enabled (FTPM_STS::FTPM_EN).

  @param[in]  None

  @retval    TRUE    FTPM is enabled.
  @retval    FALSE   FTPM is disabled. All LT writes will be dropped.
                     All LT reads will be returned with read data value of all 0's.
                     The bit can only be written once per ME power cycle.
**/
BOOLEAN
EFIAPI
PttHciPresenceCheck (
  VOID
  );

/**
  Checks whether TPM2_Startup command has been executed (FTPM_STS::STARTUP_EXEC).
  If command was executed, it should not be redundantly issued again.

  @param[in]  None

  @retval    TRUE    Startup command executed already.
  @retval    FALSE   Startup command not executed yet.
**/
BOOLEAN
EFIAPI
PttHciStartupExecuted (
  VOID
  );

/**
  Sets FTPM_CMD and CA_START register to a defined value to indicate that a command is
  available for processing.
  Any host write to this register shall result in an interrupt to the ME firmware.

  @param[in]  None

  @retval EFI_SUCCESS   Register successfully written.
  @retval TBD
**/
EFI_STATUS
EFIAPI
PttHciRequestCommandExec (
  VOID
  );

/**
  Checks whether the value of a FTPM register satisfies the input BIT setting.

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
  IN      UINT32    BitSet,
  IN      UINT32    BitClear,
  IN      UINT32    TimeOut
  );

/**
  Sends command to FTPM for execution.

  @param[in] FtpmBuffer   Buffer for TPM command data.
  @param[in] DataLength   TPM command data length.

  @retval EFI_SUCCESS     Operation completed successfully.
  @retval EFI_TIMEOUT     The register can't run into the expected status in time.
**/
EFI_STATUS
EFIAPI
PttHciSend(
  IN     UINT8      *FtpmBuffer,
  IN     UINT32     DataLength
  );

/**
  Receives response data of last command from FTPM.

  @param[out] FtpmBuffer        Buffer for response data.
  @param[out] RespSize          Response data length.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_DEVICE_ERROR      Unexpected device status.
  @retval EFI_BUFFER_TOO_SMALL  Response data is too long.
**/
EFI_STATUS
EFIAPI
PttHciReceive(
  OUT     UINT8     *FtpmBuffer,
  OUT     UINT32    *RespSize
  );

/**
  Sends formatted command to FTPM for execution and returns formatted response data.

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
  );

/**
  Checks whether PTT is Ready

  @param[in]  None

  @retval    TRUE    PTT is ready.
  @retval    FALSE   PTT is not ready

**/
BOOLEAN
EFIAPI
PttHciReadyCheck (
  VOID
  );

#endif
