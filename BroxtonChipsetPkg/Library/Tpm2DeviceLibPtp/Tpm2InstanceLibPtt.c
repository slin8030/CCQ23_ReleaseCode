/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement

@copyright
  Copyright (c) 2012 - 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

@file
  Tpm2InstanceLibPtt.c

@brief

**/

#include <IndustryStandard/Tpm12.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PciLib.h>

#include <Library/PttPtpLib.h>
#include <Guid/PttPTPInstanceGuid.h>
//[-start-160808-IB07400769-add]//
#include <Library/HeciMsgLib.h>
#include <Private/Library/HeciInitLib.h>
//[-end-160808-IB07400769-add]//

#define R_PTT_HCI_BASE_ADDRESS             0xFED40000

/**
  This service enables the sending of commands to the TPM2.

  @param[in]  InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]  InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]  OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small. 
**/
EFI_STATUS
EFIAPI
PttSubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
//[-start-160808-IB07400769-add]//
  if (!IsTxeDataRegionAvailable()) {
    return EFI_DEVICE_ERROR;
  }
//[-end-160808-IB07400769-add]//
  return PttHciSubmitCommand(InputParameterBlock, InputParameterBlockSize, OutputParameterBlock, OutputParameterBlockSize);
}

EFI_STATUS
EFIAPI
PttRequestUseTpm (
  VOID
  )
{ 
  return EFI_SUCCESS;
}

TPM2_DEVICE_INTERFACE  mPttInternalTpm2Device = {
  TPM_DEVICE_INTERFACE_TPM20_PTT_PTP,
  PttSubmitCommand,
  PttRequestUseTpm,
};

EFI_STATUS
EFIAPI
Tpm2InstanceLibPttConstructor (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = Tpm2RegisterTpm2DeviceLib (&mPttInternalTpm2Device);
  if ((Status == EFI_SUCCESS) || (Status == EFI_UNSUPPORTED)) {
    //
    // Unsupported means platform policy does not need this instance enabled.
    //
    return EFI_SUCCESS;
  }
  return Status;
}
