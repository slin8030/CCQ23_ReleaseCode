/** @file
  TCG MOR (Memory Overwrite Request) Control Driver.

  This driver initilize MemoryOverwriteRequestControl variable. It
  will clear MOR_CLEAR_MEMORY_BIT bit if it is set.

;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "TcgMor.h"

UINT8      mMorControl;
EFI_HANDLE *mTPerResetHandleBuffer = NULL;
UINT32     mTPerResetHandleIndex   = 0;

/**
  Send TPer Reset command to reset eDrive to lock all protected bands.
  Typically, there are 2 mechanism for resetting eDrive. They are:
  1. TPer Reset through IEEE 1667 protocol.
  2. TPer Reset through native TCG protocol.
  This routine will detect what protocol the attached eDrive comform to, TCG or
  IEEE 1667 protocol. Then send out TPer Reset command separately.

  @param[in] Ssp      The pointer to EFI_STORAGE_SECURITY_COMMAND_PROTOCOL instance.
  @param[in] MediaId  ID of the medium to receive data from or send data to.

**/
VOID
InitiateTPerReset (
  IN  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL    *Ssp,
  IN  UINT32                                   MediaId
  )
{

  EFI_STATUS                                   Status;
  UINT8                                        *Buffer;
  UINTN                                        XferSize;
  UINTN                                        Len;
  UINTN                                        Index;
  BOOLEAN                                      TcgFlag;
  BOOLEAN                                      IeeeFlag;
  SUPPORTED_SECURITY_PROTOCOLS_PARAMETER_DATA  *Data;

  Buffer        = NULL;
  TcgFlag       = FALSE;
  IeeeFlag      = FALSE;

  //
  // ATA8-ACS 7.57.6.1 indicates the Transfer Length field requirements a multiple of 512.
  // If the length of the TRUSTED RECEIVE parameter data is greater than the Transfer Length,
  // then the device shall return the TRUSTED RECEIVE parameter data truncated to the requested Transfer Length.
  //
  Len           = ROUNDUP512(sizeof(SUPPORTED_SECURITY_PROTOCOLS_PARAMETER_DATA));
  Buffer        = AllocateZeroPool(Len);

  if (Buffer == NULL) {
    return;
  }

  //
  // When the Security Protocol field is set to 00h, and SP Specific is set to 0000h in a TRUSTED RECEIVE
  // command, the device basic information data shall be returned.
  //
  Status = Ssp->ReceiveData (
                  Ssp,
                  MediaId,
                  100000000,                    // Timeout 10-sec
                  0,                            // SecurityProtocol
                  0,                            // SecurityProtocolSpecifcData
                  Len,                          // PayloadBufferSize,
                  Buffer,                       // PayloadBuffer
                  &XferSize
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // In returned data, the ListLength field indicates the total length, in bytes,
  // of the supported security protocol list.
  //
  Data = (SUPPORTED_SECURITY_PROTOCOLS_PARAMETER_DATA*)Buffer;
  Len  = ROUNDUP512(sizeof (SUPPORTED_SECURITY_PROTOCOLS_PARAMETER_DATA) +
                    (Data->SupportedSecurityListLength[0] << 8) +
                    (Data->SupportedSecurityListLength[1])
                    );

  //
  // Free original buffer and allocate new buffer.
  //
  FreePool(Buffer);
  Buffer = AllocateZeroPool(Len);
  if (Buffer == NULL) {
    return;
  }

  //
  // Read full supported security protocol list from device.
  //
  Status = Ssp->ReceiveData (
                  Ssp,
                  MediaId,
                  100000000,                    // Timeout 10-sec
                  0,                            // SecurityProtocol
                  0,                            // SecurityProtocolSpecifcData
                  Len,                          // PayloadBufferSize,
                  Buffer,                       // PayloadBuffer
                  &XferSize
                  );

  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Data = (SUPPORTED_SECURITY_PROTOCOLS_PARAMETER_DATA*)Buffer;
  Len  = (Data->SupportedSecurityListLength[0] << 8) + Data->SupportedSecurityListLength[1];

  //
  // Iterate full supported security protocol list to check if TCG or IEEE 1667 protocol
  // is supported.
  //
  for (Index = 0; Index < Len; Index++) {
    if (Data->SupportedSecurityProtocol[Index] == SECURITY_PROTOCOL_TCG) {
      //
      // Found a  TCG device.
      //
      TcgFlag = TRUE;
      DEBUG ((EFI_D_INFO, "This device is a TCG protocol device\n"));
      break;
    }

    if (Data->SupportedSecurityProtocol[Index] == SECURITY_PROTOCOL_IEEE1667) {
      //
      // Found a IEEE 1667 device.
      //
      IeeeFlag = TRUE;
      DEBUG ((EFI_D_INFO, "This device is a IEEE 1667 protocol device\n"));
      break;
    }
  }

  if (!TcgFlag && !IeeeFlag) {
    DEBUG ((EFI_D_INFO, "Neither a TCG nor IEEE 1667 protocol device is found\n"));
    goto Exit;
  }

  if (TcgFlag) {
    //
    // As long as TCG protocol is supported, send out a TPer Reset
    // TCG command to the device via the TrustedSend command with a non-zero Transfer Length.
    //
    Status = Ssp->SendData (
                    Ssp,
                    MediaId,
                    100000000,                    // Timeout 10-sec
                    SECURITY_PROTOCOL_TCG,        // SecurityProtocol
                    0x0400,                       // SecurityProtocolSpecifcData
                    512,                          // PayloadBufferSize,
                    Buffer                        // PayloadBuffer
                    );

    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Send TPer Reset Command Successfully !\n"));
    } else {
      DEBUG ((EFI_D_INFO, "Send TPer Reset Command Fail !\n"));
    }
  }

  if (IeeeFlag) {
    //
    // TBD : Perform a TPer Reset via IEEE 1667 Protocol
    //
    DEBUG ((EFI_D_INFO, "IEEE 1667 Protocol didn't support yet!\n"));
  }

Exit:

  if (Buffer != NULL) {
    FreePool(Buffer);
  }
}

/**
  Notification function of EFI_STORAGE_SECURITY_COMMAND_PROTOCOL.

  @param[in] Event      Event whose notification function is being invoked.
  @param[in] Context    Pointer to the notification function's context.

**/
VOID
EFIAPI
TPerResetCallback (
  IN      EFI_EVENT                   Event,
  IN      VOID                        *Context
  )
{
  EFI_STORAGE_SECURITY_COMMAND_PROTOCOL   *Ssp;
  EFI_BLOCK_IO_PROTOCOL                   *BlockIo;
  EFI_STATUS                              Status;
  UINTN                                   HandleCount;
  EFI_HANDLE                              *HandleBuffer;
  UINTN                                   Index;
  UINTN                                   Count;

  //
  // Locate all SSP protocol instances.
  //
  HandleCount  = 0;
  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiStorageSecurityCommandProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status) || (HandleCount == 0) || (HandleBuffer == NULL)) {
    return;
  }

  for (Index = 0; Index < HandleCount; Index ++) {
    //
    // Check whether the handle is in the handle list that already deal with.
    //
    for (Count = 0; Count < mTPerResetHandleIndex; Count++) {
      if (HandleBuffer[Index] == mTPerResetHandleBuffer[Count]) {
        break;
      }
    }

    if (Count != mTPerResetHandleIndex) {
      continue;
    }

    //
    // Get the SSP interface.
    //
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiStorageSecurityCommandProtocolGuid,
                    (VOID **) &Ssp
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlockIo
                    );

    if (EFI_ERROR (Status)) {
      continue;
    }

    InitiateTPerReset (Ssp, BlockIo->Media->MediaId);

    mTPerResetHandleBuffer = ReallocatePool (
                               mTPerResetHandleIndex * sizeof (EFI_HANDLE),
                               (mTPerResetHandleIndex + 1) * sizeof (EFI_HANDLE),
                               (VOID *)mTPerResetHandleBuffer
                               );
    if (mTPerResetHandleBuffer == NULL) {
      return;
    }

    mTPerResetHandleBuffer[mTPerResetHandleIndex++] = HandleBuffer[Index];
  }
}

/**
  Check whether memory overwrite is required.

  @param[in] MorVariable        MOR variable.

  @retval A value of TRUE indicates that memory overwrite is required.

**/
STATIC
BOOLEAN
IsMemoryOverwriteRequired (
  IN  UINT8                             MorVariable
  )
{
  EFI_STATUS                            Status;
  EFI_TCG_PROTOCOL                      *TcgProtocol;
  EFI_TCG2_PROTOCOL                     *Tcg2Protocol;
  UINT32                                Permanent;
  TPM_GET_PROP_OWNER_COMMAND            TcgSendBuffer;
  TPM_GET_PROP_OWNER_RESPONSE           TcgRecvBuffer;
  TPM2_GET_PERMANENT_COMMAND            Tcg2SendBuffer;
  TPM2_GET_PERMANENT_RESPONSE           Tcg2RecvBuffer;

  if (MOR_CLEAR_MEMORY_VALUE (MorVariable) == 0x0) {
    //
    // MOR Bit is 0. No need to clear memory.
    //
    return FALSE;
  }

  if ((PcdGet32 (PcdDxeTpmPolicy) & SKIP_DXE_TPM_PROPERTY_CHECK) != 0) {
    return TRUE;
  }
  
  //
  // Check TPM 1.2 property
  //
  Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, (VOID**)&TcgProtocol);
  if (!EFI_ERROR (Status)) {
    //
    // Construct command
    //
    TcgSendBuffer.Header.tag       = SwapBytes16 (TPM_TAG_RQU_COMMAND);
    TcgSendBuffer.Header.paramSize = SwapBytes32 (sizeof (TcgSendBuffer));
    TcgSendBuffer.Header.ordinal   = SwapBytes32 (TPM_ORD_GetCapability);
    TcgSendBuffer.capArea          = SwapBytes32 (TPM_CAP_PROPERTY);
    TcgSendBuffer.subCap           = SwapBytes32 (TPM_CAP_PROP_OWNER);
    TcgSendBuffer.subCapSize       = SwapBytes32 (sizeof (TPM_CAP_PROP_OWNER));

    Status = TcgProtocol->PassThroughToTpm (
                            TcgProtocol,
                            (UINT32) sizeof (TcgSendBuffer),
                            (UINT8 *) &TcgSendBuffer,
                            (UINT32) sizeof (TcgRecvBuffer),
                            (UINT8 *) &TcgRecvBuffer
                            );
    if ((EFI_ERROR (Status)) || (TcgRecvBuffer.Header.returnCode != TPM_SUCCESS)) {
      return TRUE;
    }

    return (BOOLEAN)(TcgRecvBuffer.resp != 0);
  } 

  //
  // Check TPM 2.0 property
  //
  Status = gBS->LocateProtocol (&gEfiTcg2ProtocolGuid, NULL, (VOID**)&Tcg2Protocol);
  if (!EFI_ERROR (Status)) {
    //
    // Construct command
    //
    Tcg2SendBuffer.Header.tag         = SwapBytes16 (TPM_ST_NO_SESSIONS);
    Tcg2SendBuffer.Header.commandCode = SwapBytes32 (TPM_CC_GetCapability);
    Tcg2SendBuffer.Capability         = SwapBytes32 (TPM_CAP_TPM_PROPERTIES);
    Tcg2SendBuffer.Property           = SwapBytes32 (TPM_PT_PERMANENT);
    Tcg2SendBuffer.PropertyCount      = SwapBytes32 (0x01);
    Tcg2SendBuffer.Header.paramSize   = SwapBytes32 (sizeof (Tcg2SendBuffer));
  
    Status = Tcg2Protocol->SubmitCommand (
                             Tcg2Protocol,
                             (UINT32) sizeof (Tcg2SendBuffer),
                             (UINT8 *) &Tcg2SendBuffer,
                             (UINT32) sizeof (Tcg2RecvBuffer),
                             (UINT8 *) &Tcg2RecvBuffer
                             );
    if ((EFI_ERROR (Status)) || (Tcg2RecvBuffer.Header.responseCode != TPM_RC_SUCCESS)) {
      return TRUE;
    }
  
    Permanent = SwapBytes32 (Tcg2RecvBuffer.CapabilityData.data.tpmProperties.tpmProperty->value);
    //
    // bit0: ownerAuthSet
    // bit2: lockoutAuthSet
    //
    return (BOOLEAN)((Permanent & (BIT0| BIT2)) != 0);
  }
  //
  // No TPM found.
  //
  return FALSE;
}


/**
  Clear memory above 1MB.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval Others                Operation failed.

**/
STATIC
EFI_STATUS
MemoryOverwrite (
  VOID
  )
{
  EFI_STATUS               Status;
  EFI_MEMORY_DESCRIPTOR    *MemoryMap;
  UINTN                    MemoryMapSize;
  UINTN                    MapKey;
  UINTN                    DescriptorSize;
  UINT32                   DescriptorVersion;

  //
  // Get the EFI memory map
  //
  MemoryMapSize  = 0;
  MemoryMap      = NULL;
  Status = gBS->GetMemoryMap (
                  &MemoryMapSize,
                  MemoryMap,
                  &MapKey,
                  &DescriptorSize,
                  &DescriptorVersion
                  );
  while (Status == EFI_BUFFER_TOO_SMALL) {
    Status = gBS->AllocatePool (EfiBootServicesData, MemoryMapSize, (VOID **)&MemoryMap);
    if (!EFI_ERROR (Status)) {
      Status = gBS->GetMemoryMap (
                     &MemoryMapSize,
                     MemoryMap,
                     &MapKey,
                     &DescriptorSize,
                     &DescriptorVersion
                    );
      if (EFI_ERROR (Status)) {
        gBS->FreePool (MemoryMap);
      }
    }
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MemoryOverwriteAbove1M (
             MemoryMapSize,
             MemoryMap,
             MapKey,
             DescriptorSize,
             DescriptorVersion
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  gBS->FreePool (MemoryMap);

  return Status;
}

/**
  Ready to Boot Event notification handler.

  Sequence of OS boot events is measured in this event notification handler.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
OnReadyToBoot (
  IN EFI_EVENT                      Event,
  IN VOID                           *Context
  )
{
  UINTN                             DataSize;

  if (MOR_CLEAR_MEMORY_VALUE (mMorControl) == 0x0) {
    //
    // MorControl is expected, directly return to avoid unnecessary variable operation
    //
    return ;
  }
  //
  // Clear MOR_CLEAR_MEMORY_BIT
  //
  DEBUG ((EFI_D_INFO, "TcgMor: Clear MorClearMemory bit\n"));
  mMorControl &= 0xFE;

  DataSize = sizeof (mMorControl);
  gRT->SetVariable (
         MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
         &gEfiMemoryOverwriteControlDataGuid,
         EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
         DataSize,
         &mMorControl
         );

}


/**
  Entry Point for TCG MOR Control driver.

  @param[in] ImageHandle  Image handle of this driver.
  @param[in] SystemTable  A Pointer to the EFI System Table.

  @retval EFI_SUCEESS
  @return Others          Some error occurs.
**/
EFI_STATUS
EFIAPI
MorDriverEntryPoint (
  IN EFI_HANDLE                     ImageHandle,
  IN EFI_SYSTEM_TABLE               *SystemTable
  )
{
  EFI_STATUS                        Status;
  UINTN                             DataSize;
  EFI_EVENT                         Event;
  BOOLEAN                           MemoryOverwriteRequired;
  VOID                              *Registration;

  //
  // The firmware is required to create the MemoryOverwriteRequestControl UEFI variable.
  //
  DataSize = sizeof (mMorControl);
  Status = gRT->GetVariable (
                  MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                  &gEfiMemoryOverwriteControlDataGuid,
                  NULL,
                  &DataSize,
                  &mMorControl
                  );
  if (EFI_ERROR (Status)) {
    //
    // Set default value to 0
    //
    mMorControl = 0;
    DEBUG ((EFI_D_INFO, "TcgMor: Create gEfiMemoryOverwriteControlDataGuid!\n"));
    Status = gRT->SetVariable (
                    MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                    &gEfiMemoryOverwriteControlDataGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    DataSize,
                    &mMorControl
                    );
    ASSERT_EFI_ERROR (Status);
    MemoryOverwriteRequired = IsMemoryOverwriteRequired (MOR_CLEAR_MEMORY_BIT_MASK >> MOR_CLEAR_MEMORY_BIT_OFFSET);
  } else {
    //
    // Create a Ready To Boot Event and Clear the MorControl bit in the call back function.
    //
    DEBUG ((EFI_D_INFO, "TcgMor: Create ReadyToBoot Event for MorControl Bit cleanning!\n"));
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               OnReadyToBoot,
               NULL,
               &Event
               );
    ASSERT_EFI_ERROR (Status);
    MemoryOverwriteRequired = IsMemoryOverwriteRequired (mMorControl);
  }

  if (MemoryOverwriteRequired) {
    Status = MemoryOverwrite ();
  }

  //
  // Register EFI_STORAGE_SECURITY_COMMAND_PROTOCOL event for TPer Reset.
  //
  EfiCreateProtocolNotifyEvent (
    &gEfiStorageSecurityCommandProtocolGuid,
    TPL_CALLBACK,
    TPerResetCallback,
    NULL,
    (VOID **) &Registration
    );

  return Status;
}

