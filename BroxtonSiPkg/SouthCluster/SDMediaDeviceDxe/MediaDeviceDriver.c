/** @file
  UEFI Driver Entry and Binding support.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#include <MediaDeviceDriver.h>
#include <Protocol/EmmcCardInfoProtocol.h>
#include <Guid/UnbootablePartitionGuid.h>


//#pragma optimize ("", off)
//
// MMCSDIOController Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL gMediaDeviceDriverBinding = {
  MediaDeviceDriverBindingSupported,
  MediaDeviceDriverBindingStart,
  MediaDeviceDriverBindingStop,
  0x10,
  NULL,
  NULL
};

/**
  Entry point for EFI drivers.

  @param[in]  ImageHandle - EFI_HANDLE
  @param[in]  SystemTable - EFI_SYSTEM_TABLE

  @retval   EFI_SUCCESS         - Success
  @retval   EFI_DEVICE_ERROR    - Fail
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  return EfiLibInstallAllDriverProtocols (
           ImageHandle,
           SystemTable,
           &gMediaDeviceDriverBinding,
           ImageHandle,
           &gMediaDeviceComponentName,
           NULL,
           NULL
           );
}
/**
  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  that has installed will be supported.

  @param[in]  This                - Protocol instance pointer.
  @param[in]  Controlle           - Handle of device to test
  @param[in]  RemainingDevicePath - Not used

  @retval  EFI_SUCCESS       - This driver supports this device.
  @retval  EFI_UNSUPPORTED   - This driver does not support this device.
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                  Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  )
{
  EFI_STATUS                Status;
  EFI_SD_HOST_IO_PROTOCOL   *SdHostIo;

  //
  // Test whether there is SDHostIO Protocol attached on the controller handle.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSdHostIoProtocolGuid,
                  (VOID**) &SdHostIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiSdHostIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

Exit:
  return Status;
}

/**
  Starting the Media Device Driver

  @param[in]  This                - Protocol instance pointer.
  @param[in]  Controller          - Handle of device to test
  @param[in]  RemainingDevicePath - Not used

  @retval  EFI_SUCCESS           - supports this device.
  @retval  EFI_UNSUPPORTED       - do not support this device.
  @retval  EFI_DEVICE_ERROR      - cannot be started due to device Error
  @retval  EFI_OUT_OF_RESOURCES  - cannot allocate resources
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN EFI_HANDLE                      Controller,
  IN EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
{
  EFI_STATUS                  Status;
  EFI_SD_HOST_IO_PROTOCOL     *SdHostIo;
  CARD_DATA                   *CardData;
  UINTN                       Loop;
  EFI_EMMC_CARD_INFO_PROTOCOL *EfiEmmcCardInfoRegister;

  DEBUG ((EFI_D_INFO, "%a(%d): %a()\n", __FILE__, __LINE__, __FUNCTION__));

  EfiEmmcCardInfoRegister = NULL;
  CardData = NULL;
  SdHostIo = NULL;
  //
  //Alloc memory for EfiEmmcCardInfoRegister variable
  //
  EfiEmmcCardInfoRegister = (EFI_EMMC_CARD_INFO_PROTOCOL *)AllocateZeroPool(sizeof(EFI_EMMC_CARD_INFO_PROTOCOL));
  if(EfiEmmcCardInfoRegister == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  //
  // Open PCI I/O Protocol and save pointer to open protocol
  // in private data area.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSdHostIoProtocolGuid,
                  (VOID **) &SdHostIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "MediaDeviceDriverBindingStart: Fail to open gEfiSdHostIoProtocolGuid \n"));
    goto Exit;
  }

  Status = SdHostIo->DetectCardAndInitHost (SdHostIo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "MediaDeviceDriverBindingStart: Fail to DetectCardAndInitHost %r\n", Status));
    goto Exit;
  }

  CardData = (CARD_DATA*)AllocateZeroPool (sizeof (CARD_DATA));
  if (CardData == NULL) {
    Status =  EFI_OUT_OF_RESOURCES;
    DEBUG ((EFI_D_ERROR, "MediaDeviceDriverBindingStart: Fail to AllocateZeroPool(CARD_DATA) \n"));
    goto Exit;
  }

  ASSERT (SdHostIo->HostCapability.BoundarySize >= 4 * 1024);
  //CardData->RawBufferPointer = (UINT8*)AllocateZeroPool (2 * SdHostIo->HostCapability.BoundarySize);
  CardData->RawBufferPointer = (UINT8*)(UINTN)0xffffffff;

  DEBUG ((EFI_D_INFO, "CardData->RawBufferPointer = 0x%x \n",CardData->RawBufferPointer));
  DEBUG ((EFI_D_INFO, "requesting 0x%x pages \n",EFI_SIZE_TO_PAGES(2 * SdHostIo->HostCapability.BoundarySize)));
  //
  //Allocated the buffer under 4G
  //
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiBootServicesData,
                  EFI_SIZE_TO_PAGES(2 * SdHostIo->HostCapability.BoundarySize),
                  (EFI_PHYSICAL_ADDRESS *)(&(CardData->RawBufferPointer))
                  );
  DEBUG ((EFI_D_INFO, "CardData->RawBufferPointer = 0x%x \n",CardData->RawBufferPointer));
  if (!EFI_ERROR (Status)) {
    CardData->RawBufferPointer = ZeroMem (CardData->RawBufferPointer, EFI_SIZE_TO_PAGES(2 * SdHostIo->HostCapability.BoundarySize));
  } else {
    DEBUG ((DEBUG_ERROR, "MediaDeviceDriverBindingStart: Fail to AllocateZeroPool(2*x) \n"));
    Status =  EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  CardData->AlignedBuffer = CardData->RawBufferPointer - ((UINTN)(CardData->RawBufferPointer) & (SdHostIo->HostCapability.BoundarySize - 1)) + SdHostIo->HostCapability.BoundarySize;

  CardData->Signature = CARD_DATA_SIGNATURE;
  CardData->SdHostIo  = SdHostIo;
  CardData->Handle = Controller;
  for (Loop = 0; Loop < MAX_NUMBER_OF_PARTITIONS; Loop++) {
    CardData->Partitions[Loop].Signature = CARD_PARTITION_SIGNATURE;
  }
  Status = MMCSDCardInit (CardData);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "MediaDeviceDriverBindingStart: Fail to MMCSDCardInit \n"));
    goto Exit;
  }

  DEBUG ((DEBUG_INFO, "MediaDeviceDriverBindingStart: MMC SD card\n"));
  Status = MMCSDBlockIoInit (CardData);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "MediaDeviceDriverBindingStart: Card BlockIo init failed\n"));
    goto Exit;
  }


  Status = MediaDeviceDriverInstallBlockIo (CardData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "MediaDeviceDriverBindingStart: Fail to install gEfiBlockIoProtocolGuid \n"));
    goto Exit;
  }

  //
  // Component name protocol
  //
  Status = AddUnicodeString (
             "eng",
             gMediaDeviceComponentName.SupportedLanguages,
             &CardData->ControllerNameTable,
             L"MMC/SD Media Device"
             );
  if (EFI_ERROR (Status)) {
    Status = EFI_OUT_OF_RESOURCES;
    MediaDeviceDriverUninstallBlockIo (CardData);
  }
  if (EfiEmmcCardInfoRegister != NULL) {

    //
    //  assign to protocol
    //
    EfiEmmcCardInfoRegister->CardData = CardData;

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &Controller,
                    &gEfiEmmcCardInfoProtocolGuid,
                    EfiEmmcCardInfoRegister,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "MediaDeviceDriverBindingStart: Install eMMC Card info protocol failed\n"));
      MediaDeviceDriverUninstallBlockIo (CardData);
      goto Exit;
    }
  }

  DEBUG ((DEBUG_INFO, "MediaDeviceDriverBindingStart: Started\n"));

Exit:
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "MediaDeviceDriverBindingStart: End with failure\n"));
    if (CardData != NULL && SdHostIo != NULL) {
      if (CardData->RawBufferPointer != NULL) {
        gBS->FreePages ((EFI_PHYSICAL_ADDRESS) CardData->RawBufferPointer, EFI_SIZE_TO_PAGES(SdHostIo->HostCapability.BoundarySize * 2));
      }
      FreePool (CardData);
    }
    gBS->CloseProtocol (
           Controller,
           &gEfiSdHostIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
  }
  return Status;
}
/**
  Stop this driver on ControllerHandle. Support stoping any child handles
  created by this driver.

  @param[in]  This              - Protocol instance pointer.
  @param[in]  Controller        - Handle of device to stop driver on
  @param[in]  NumberOfChildren  - Number of Children in the ChildHandleBuffer
  @param[in]  ChildHandleBuffer - List of handles for the children we need to stop.

  @retval  EFI_SUCCESS          - Success
  @retval  EFI_DEVICE_ERROR     - Fail
**/
EFI_STATUS
EFIAPI
MediaDeviceDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                Status;
  CARD_DATA                 *CardData;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  //
  // First find BlockIo Protocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiBlockIoProtocolGuid,
                  (VOID**) &BlockIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiSdHostIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );


  CardData  = CARD_DATA_FROM_THIS(BlockIo);

  //
  // Uninstall Block I/O protocol from the device handle
  //
  Status = MediaDeviceDriverUninstallBlockIo (CardData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (CardData != NULL) {
    FreeUnicodeStringTable (CardData->ControllerNameTable);
    if (CardData->RawBufferPointer != NULL) {
      FreePool (CardData->RawBufferPointer);
    }
    FreePool (CardData);
  }

  return Status;

}


STATIC
struct {
  CONTROLLER_DEVICE_PATH    Controller;
  EFI_DEVICE_PATH_PROTOCOL  End;
} ControllerDevPathTemplate = {
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_CONTROLLER_DP,
      {
        sizeof (CONTROLLER_DEVICE_PATH),
        0
      },
    },
    0
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};

/**
  MediaDeviceDriverInstallBlockIo

  @param[in]  CardData    - Pointer to CARD_DATA

  @retval  EFI_INVALID_PARAMETER
  @retval  EFI_UNSUPPORTED
  @retval  EFI_SUCCESS
**/
EFI_STATUS
MediaDeviceDriverInstallBlockIo (
  IN  CARD_DATA    *CardData
  )
{
  EFI_STATUS                Status;
  UINTN                     Loop;
  MMC_PARTITION_DATA        *Partition;
  EFI_DEVICE_PATH_PROTOCOL  *MainPath;

  Partition = CardData->Partitions;

  Status = gBS->HandleProtocol (
                  CardData->Handle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID**) &MainPath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Loop = 0; Loop < MAX_NUMBER_OF_PARTITIONS; Partition++, Loop++) {
    if (!Partition->Present) {
      continue;
    }
    //if (Loop == 0) {
    //  continue;
    //}

    DEBUG ((EFI_D_INFO, "MediaDeviceDriverInstallBlockIo: Installing Block I/O for partition %d\n", Loop));

    Partition->Handle = NULL;
    Partition->CardData = CardData;

    //if (Loop == 0) {
    ControllerDevPathTemplate.Controller.ControllerNumber = (UINT32) Loop;
    Partition->DevPath =
      AppendDevicePath (
        MainPath,
        (EFI_DEVICE_PATH_PROTOCOL *) &ControllerDevPathTemplate
        );
    if (Partition->DevPath == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      break;
    }

    Status = gBS->InstallProtocolInterface (
                    &(Partition->Handle),
                    &gEfiDevicePathProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    Partition->DevPath
                    );
    if (EFI_ERROR (Status)) {
      break;
    }
    //}

    Status = gBS->InstallProtocolInterface (
                    &(Partition->Handle),
                    &gEfiBlockIoProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &Partition->BlockIo
                    );

    //
    // Handle Boot partitions
    //
    if (CardData->CardType == MMCCard) {
      //
      // skip unbootable partitions display on boot manager
      //
      // Loop [0] OS boot partition
      // Loop [1] BIOS LBP1
      // Loop [2] BIOS LBP2
      // Loop [3] GPP1
      // Loop [4] GPP2
      // Loop [5] GPP3
      // Loop [6] GPP4
      //
      if (Loop != 0) {
        Status = gBS->InstallProtocolInterface (
                        &(Partition->Handle),
                        &gEfiUnbootablePartitionGuid,
                        EFI_NATIVE_INTERFACE,
                        NULL
                        );
      }
    }
  }

  return Status;
}

/**
  MediaDeviceDriverUninstallBlockIo

  @param[in]  CardData    - Pointer to CARD_DATA

  @retval  EFI_INVALID_PARAMETER
  @retval  EFI_UNSUPPORTED
  @retval  EFI_SUCCESS
**/
EFI_STATUS
MediaDeviceDriverUninstallBlockIo (
  IN  CARD_DATA    *CardData
  )
{
  EFI_STATUS          Status;
  UINTN               Loop;
  MMC_PARTITION_DATA  *Partition;

  Partition = CardData->Partitions;

  Status = EFI_SUCCESS;

  for (Loop = 0; Loop < MAX_NUMBER_OF_PARTITIONS; Partition++, Loop++) {
    if (!Partition->Present) {
      continue;
    }

    Status = gBS->UninstallProtocolInterface (
                    Partition->Handle,
                    &gEfiBlockIoProtocolGuid,
                    &Partition->BlockIo
                    );
  }

  return Status;
}

