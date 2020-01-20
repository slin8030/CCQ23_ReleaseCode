/** @file
  Implement TPM1.2 Startup related command.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#include <Uefi.h>
#include <IndustryStandard/Tpm12.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/Tpm12DeviceLib.h>
#include <Library/DebugLib.h>


#pragma pack(1)

typedef struct {
  TPM_RQU_COMMAND_HDR   Hdr;
  TPM_NV_INDEX          NvIndex;
  UINT32                Offset;
  UINT32                DataSize;
} TPM_CMD_NV_READ_VALUE;

typedef struct {
  TPM_RSP_COMMAND_HDR   Hdr;
  UINT32                DataSize;
  UINT8                 Data[20];
} TPM_RSP_NV_READ_VALUE;

#pragma pack()

/**
  Send NV ReadValue command to TPM1.2.

  @param NvIndex           NvIndex.
  @param Offset            Offset to DataBuffer
  @param Buffer            DataBuffer

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior
  @retval EFI_NOT_FOUND    Incorrect return data
**/
EFI_STATUS
EFIAPI
Tpm12NvReadValue (
  IN TPM_NV_INDEX   NvIndex,
  IN UINT8          Offset,
  OUT UINT8         *DataBuffer
  )
{
  EFI_STATUS                        Status;
  UINT32                            TpmRecvSize;
  UINT32                            TpmSendSize;
  TPM_CMD_NV_READ_VALUE             SendBuffer;
  TPM_RSP_NV_READ_VALUE             RecvBuffer;
  UINT32                            ReturnCode;

  //
  // send Tpm command TPM_ORD_Startup
  //
  TpmRecvSize               = sizeof (TPM_RSP_NV_READ_VALUE);
  TpmSendSize               = sizeof (TPM_CMD_NV_READ_VALUE);
  SendBuffer.Hdr.tag        = SwapBytes16 (TPM_TAG_RQU_COMMAND);
  SendBuffer.Hdr.paramSize  = SwapBytes32 (22);
  SendBuffer.Hdr.ordinal    = SwapBytes32 (TPM_ORD_NV_ReadValue);
  SendBuffer.NvIndex        = SwapBytes32 (NvIndex);
  SendBuffer.Offset         = (UINT32)Offset;
  SendBuffer.DataSize       = SwapBytes32(20 - Offset);


  Status = Tpm12SubmitCommand (TpmSendSize, (UINT8 *)&SendBuffer, &TpmRecvSize, (UINT8 *)&RecvBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ReturnCode = SwapBytes32(RecvBuffer.Hdr.returnCode);
  DEBUG ((DEBUG_INFO, "Tpm12NvReadValue - ReturnCode = %x\n", ReturnCode));
  if (ReturnCode != TPM_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }

  if (TpmRecvSize != sizeof (TPM_RSP_NV_READ_VALUE)) {

    DEBUG ((DEBUG_ERROR, "Tpm12NvReadValue - RecvBufferSize Error = %x\n", TpmRecvSize));
    return EFI_NOT_FOUND;
  }

  //
  // Return the response
  //
  CopyMem (DataBuffer, &RecvBuffer.Data, SwapBytes32(RecvBuffer.DataSize));

  return EFI_SUCCESS;
}
