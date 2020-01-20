/** @file
  Library instance of SPI Flash Common Library Class

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/

#include <Library/SpiFlashCommon.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Guid/EventGroup.h>
#include <Library/ScPlatformLib.h>

extern SC_SPI_PROTOCOL *mSpiProtocol;

//
// FlashAreaBaseAddress and Size for boottime and runtime usage.
//
extern UINTN mFlashAreaBaseAddress;
extern UINTN mFlashAreaSize;

EFI_EVENT  mSpiFlashCommonLibAddressChangeEvent = NULL;

/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param[in]  Event        Event whose notification function is being invoked.
  @param[in]  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
SpiFlashAddressChangeEvent (
  IN EFI_EVENT             Event,
  IN VOID                  *Context
  )
{
  EfiConvertPointer (0x0, (VOID **) &mSpiProtocol);
  EfiConvertPointer (0x0, (VOID **) &mFlashAreaBaseAddress);
}

/**
  The library constructuor.

  The function does the necessary initialization work for this library
  instance.

  @param[in]  ImageHandle       The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable       A pointer to the EFI system table.

  @retval     EFI_SUCCESS       The function always return EFI_SUCCESS for now.
                                It will ASSERT on error for debug version.
  @retval     EFI_ERROR         Please reference LocateProtocol for error code details.
**/
EFI_STATUS
EFIAPI
DxeSpiFlashCommonLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR       GcdDescriptor;
  EFI_PHYSICAL_ADDRESS                  BaseAddress;
  UINT64                                Length;

  mFlashAreaBaseAddress  = 0xFFFFFFFC - GetSpiFlashRegionLimit (BIOS);
  mFlashAreaBaseAddress += 4;
  mFlashAreaSize         = (UINTN)PcdGet32 (PcdFlashAreaSize);

  BaseAddress = mFlashAreaBaseAddress & (~EFI_PAGE_MASK);
  Length      = (mFlashAreaSize + EFI_PAGE_SIZE - 1) & (~EFI_PAGE_MASK);
  Status = gDS->GetMemorySpaceDescriptor (BaseAddress, &GcdDescriptor);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SpiFlashCommonLib failed to add EFI_MEMORY_RUNTIME attribute to Flash.\n"));
  } else {
    Status = gDS->SetMemorySpaceAttributes (
                    BaseAddress,
                    Length,
                    GcdDescriptor.Attributes | EFI_MEMORY_RUNTIME
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "SpiFlashCommonLib failed to add EFI_MEMORY_RUNTIME attribute to Flash.\n"));
    }
  }

  //
  // Locate the SPI protocol.
  //
  Status = gBS->LocateProtocol (
                  &gScSpiProtocolGuid,
                  NULL,
                  (VOID **)&mSpiProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  SpiFlashAddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mSpiFlashCommonLibAddressChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
