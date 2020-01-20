/** @file
  This library abstract how to access TPM2 hardware device.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Tpm2Interface.h>
#include <Protocol/SmmBase2.h>
#include <Guid/TpmInstance.h>

/**
  This service enables the sending of commands to the TPM2.

  @param[in]      InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small. 
  @retval EFI_INVALID_PARAMETER  TPM2_INTERFACE_PROTOCOL not found.   
**/
EFI_STATUS
EFIAPI
ProtocolBasedTpm2SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  );

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
  @retval EFI_INVALID_PARAMETER  TPM2_INTERFACE_PROTOCOL not found.   
**/
EFI_STATUS
EFIAPI
ProtocolBasedTpm2RequestUseTpm (
  VOID
  );


TPM2_DEVICE_INTERFACE  mProtocolBasedTpm2InternalTpm2Device = {
  TPM_DEVICE_INTERFACE_TPM20_DTPM,
  ProtocolBasedTpm2SubmitCommand,
  ProtocolBasedTpm2RequestUseTpm,
};


/**
  The function register protocl based TPM2.0 instance.
  
  @retval EFI_SUCCESS   Instance is registered, or system dose not surpport this instance
**/
EFI_STATUS
EFIAPI
Tpm2InstanceLibProtocolBasedConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                              Status;

  Status = Tpm2RegisterTpm2DeviceLib (&mProtocolBasedTpm2InternalTpm2Device);
  if ((Status == EFI_SUCCESS) || (Status == EFI_UNSUPPORTED)) {
    //
    // Unsupported means platform policy does not need this instance enabled.
    //
    return EFI_SUCCESS;
  }
  return Status;
}
