/** @file
  Usb Mass Storage Peim

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "UsbMassStoragePei.h"

//
// Global function
//
STATIC EFI_PEI_NOTIFY_DESCRIPTOR        mNotifyList = {
  EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiUsb3IoPpiGuid,
  NotifyOnUsbIoPpi
};

STATIC UINT16                           mBypassCswTimeoutDevices[] = {
  0x090c, 0x1000, 0xffff,      // USB 1.1 "USB CYBER DISK" thumb drive with retry
  0x08ec, 0x0015, 0xffff,      // USB 2.0 "I-O DATA USB Flash Disk" thumb drive with retry
  0x1f75, 0x0902, 0x0001,      // USB 3.0 "SP Silcon Power 16GB" thumb drive without retry
  0x054c, 0x05b9, 0x0001,      // USB 3.0 "SONY USM-Q 32GB" thumb drive without retry
  0x125F, 0x105B, 0x0001,      // USB 3.0 "ADATA N005" thumb drive without retry
  0x8564, 0x1000, 0x0001       // USB 3.0 "Transcend JetFlash 16G" thumb drive without retry
};

/**

  Initializes the Usb Mass Storage Peim

  @param  FileHandle            Handle of the file being invoked.
  @param  PeiServices           General purpose services that are available to every PEIM.
 
  @retval EFI_UNSUPPORTED       Can't find required PPI
  @retval EFI_OUT_OF_RESOURCES  Can't allocate memory resource
  @retval EFI_SUCCESS           Success

**/
EFI_STATUS
UsbMassStoragePeimEntry (
  IN EFI_PEI_FILE_HANDLE     FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS                  Status;
  //
  // Shadow this PEIM to run from memory
  //
  if (!EFI_ERROR (PeiServicesRegisterForShadow (FileHandle))) {
    return EFI_SUCCESS;
  }
  //
  // Register a notify function
  //
  Status = (**PeiServices).NotifyPpi (
                             PeiServices,
                             &mNotifyList
                             );
  return Status;
}

/**

  Initialize the usb bot device.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  NotifyDesc            Address of the notification descriptor data structure.
  @param  InvokePpi             Address of the PPI that was invoked.

  @retval EFI_SUCCESS           The function completes successfully.

**/
EFI_STATUS
EFIAPI
NotifyOnUsbIoPpi (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR         *NotifyDesc,
  IN  VOID                              *InvokePpi
  )
{
  PEI_USB3_IO_PPI               *UsbIoPpi;
  PEI_BLKIO_DEVICE              *PeiBlkIoDev;
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  *InterfaceDesc;
  UINTN                         MemPages;
  EFI_PHYSICAL_ADDRESS          AllocateAddress;
  EFI_USB_ENDPOINT_DESCRIPTOR   *EndpointDesc;
  UINT8                         Index;
  EFI_USB_DEVICE_DESCRIPTOR     DeviceDescriptor;
  EFI_USB_DEVICE_REQUEST        DevReq;
  UINTN                         SpinUpStall;

  UsbIoPpi = (PEI_USB3_IO_PPI *) InvokePpi;
  //
  // Check its interface
  //
  Status = UsbIoPpi->UsbGetInterfaceDescriptor (
                       PeiServices,
                       UsbIoPpi,
                       &InterfaceDesc
                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Check if it is the BOT/CBI device we support
  //
  if ((InterfaceDesc->InterfaceClass != MASS_STORAGE_CLASS) ||
      ((InterfaceDesc->InterfaceProtocol != BOT_INTERFACE_PROTOCOL) &&
      (InterfaceDesc->InterfaceProtocol != CBI0_INTERFACE_PROTOCOL))) {
    return EFI_NOT_FOUND;
  }

  MemPages = sizeof (PEI_BLKIO_DEVICE) / EFI_PAGE_SIZE + 1;
  Status = (*PeiServices)->AllocatePages (
                             (CONST EFI_PEI_SERVICES **)PeiServices,
                             EfiBootServicesData,
                             MemPages,
                             &AllocateAddress
                             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ZeroMem ((VOID*)(UINTN)AllocateAddress, sizeof (PEI_BLKIO_DEVICE));
  PeiBlkIoDev                  = (PEI_BLKIO_DEVICE *) ((UINTN) AllocateAddress);
  PeiBlkIoDev->Signature       = PEI_BLKIO_DEVICE_SIGNATURE;
  PeiBlkIoDev->UsbIoPpi        = UsbIoPpi;
  (**PeiServices).LocatePpi (
                    (CONST EFI_PEI_SERVICES **)PeiServices,
                    &gEfiPeiStallPpiGuid,
                    0,
                    NULL,
                    (VOID **)&PeiBlkIoDev->StallPpi
                    );
  //
  // Setup endpoints
  //
  for (Index = 0; Index < InterfaceDesc->NumEndpoints; Index ++) {
    Status = UsbIoPpi->UsbGetEndpointDescriptor (
                         PeiServices,
                         UsbIoPpi,
                         Index,
                         &EndpointDesc
                         );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (EndpointDesc->Attributes == 0x02) {
      if ((EndpointDesc->EndpointAddress & 0x80) != 0) {
        PeiBlkIoDev->BulkInEndpoint = EndpointDesc;
      } else {
        PeiBlkIoDev->BulkOutEndpoint = EndpointDesc;
      }
    }
    if (EndpointDesc->Attributes == 0x03) {
      PeiBlkIoDev->InterruptEndpoint = EndpointDesc;
    }
  }
  //
  // Inspect specific device to bypass CSW timeout
  //
  DevReq.RequestType  = USB_DEV_GET_DESCRIPTOR_REQ_TYPE;
  DevReq.Request      = USB_DEV_GET_DESCRIPTOR;
  DevReq.Value        = 0x100;
  DevReq.Index        = 0;
  DevReq.Length       = 8;
  Status = UsbIoPpi->UsbControlTransfer (
                       PeiServices,
                       UsbIoPpi,
                       &DevReq,
                       EfiUsbDataIn,
                       3000,
                       &DeviceDescriptor,
                       8
                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Index = 0; Index < sizeof (mBypassCswTimeoutDevices) / sizeof (UINT16); Index += 3) {
    if (DeviceDescriptor.IdVendor == mBypassCswTimeoutDevices[Index] && 
        DeviceDescriptor.IdProduct == mBypassCswTimeoutDevices[Index + 1]) {
      PeiBlkIoDev->BypassCswTimeout = mBypassCswTimeoutDevices[Index + 2];
    }
  }
  PeiBlkIoDev->Media.DeviceType                  = UsbMassStorage;
  PeiBlkIoDev->Media2.InterfaceType              = 0x0f;     // USB Class Device Path SubType
  PeiBlkIoDev->Media2.RemovableMedia             = FALSE;
  PeiBlkIoDev->Media2.ReadOnly                   = FALSE;
  PeiBlkIoDev->H2OBlkIoPpi.WriteBlocks           = WriteBlocks;
  PeiBlkIoDev->PpiDescriptor[0].Flags            = EFI_PEI_PPI_DESCRIPTOR_PPI;
  PeiBlkIoDev->PpiDescriptor[0].Guid             = &gH2OPeiVirtualBlockIoPpiGuid;
  PeiBlkIoDev->PpiDescriptor[0].Ppi              = &PeiBlkIoDev->H2OBlkIoPpi;
  PeiBlkIoDev->BlkIoPpi.GetNumberOfBlockDevices  = GetNumberOfBlockDevices;
  PeiBlkIoDev->BlkIoPpi.GetBlockDeviceMediaInfo  = GetBlockDeviceMediaInfo;
  PeiBlkIoDev->BlkIoPpi.ReadBlocks               = ReadBlocks;
  PeiBlkIoDev->PpiDescriptor[1].Flags            = EFI_PEI_PPI_DESCRIPTOR_PPI;
  PeiBlkIoDev->PpiDescriptor[1].Guid             = &gEfiPeiVirtualBlockIoPpiGuid;
  PeiBlkIoDev->PpiDescriptor[1].Ppi              = &PeiBlkIoDev->BlkIoPpi;
  PeiBlkIoDev->BlkIo2Ppi.Revision                = EFI_PEI_RECOVERY_BLOCK_IO2_PPI_REVISION;
  PeiBlkIoDev->BlkIo2Ppi.GetNumberOfBlockDevices = GetNumberOfBlockDevices2;
  PeiBlkIoDev->BlkIo2Ppi.GetBlockDeviceMediaInfo = GetBlockDeviceMediaInfo2;
  PeiBlkIoDev->BlkIo2Ppi.ReadBlocks              = ReadBlocks2;
  PeiBlkIoDev->PpiDescriptor[2].Flags            = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  PeiBlkIoDev->PpiDescriptor[2].Guid             = &gEfiPeiVirtualBlockIo2PpiGuid;
  PeiBlkIoDev->PpiDescriptor[2].Ppi              = &PeiBlkIoDev->BlkIo2Ppi;
  
  //
  // Setup Atapi Command function
  //
  if (InterfaceDesc->InterfaceProtocol == BOT_INTERFACE_PROTOCOL) {
    PeiBlkIoDev->AtapiCommand = PeiBotAtapiCommand;
    SpinUpStall = 200 * 1000;
  } else {
    PeiBlkIoDev->AtapiCommand = PeiCbiAtapiCommand;
    SpinUpStall = 1200 * 1000;
  }
  //
  // Stall 0.2(BOT)/1.2(CBI) second before first mass storage command to waiting for spin-up
  //
  PeiBlkIoDev->StallPpi->Stall (
                           (CONST EFI_PEI_SERVICES **)PeiServices,
                           PeiBlkIoDev->StallPpi,
                           SpinUpStall
                           );
  Status = (**PeiServices).AllocatePages (
                             (CONST EFI_PEI_SERVICES **)PeiServices,
                             EfiBootServicesData,
                             1,
                             &AllocateAddress
                             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  PeiBlkIoDev->SenseData = (ATAPI_REQUEST_SENSE_DATA*) ((UINTN) AllocateAddress);
  //
  // Start mass storage commands
  //
  Status = PeiUsbInquiry (PeiServices, PeiBlkIoDev);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = PeiUsbDetectMedia (PeiServices, PeiBlkIoDev);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Install H2OPeiVirtualBlockIoPpi/EfiPeiVirtualBlockIoPpi/EfiPeiVirtualBlockIo2Ppi PPI 
  //
  Status = (**PeiServices).InstallPpi (
                             (CONST EFI_PEI_SERVICES **)PeiServices,
                             PeiBlkIoDev->PpiDescriptor
                             );
  return Status;
}

/**

  Gets the count of block I/O devices that one specific block driver detects

  @param  PeiServices           General purpose services that are available to every PEIM
  @param  This                  Indicates the EFI_PEI_RECOVERY_BLOCK_IO_PPI instance
  @param  NumberBlockDevices    The number of block I/O devices discovered.

  @retval EFI_SUCCESS           Operation performed successfully.

**/
EFI_STATUS
EFIAPI
GetNumberOfBlockDevices (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO_PPI     *This,
  OUT UINTN                             *NumberBlockDevices
  )
{
  //
  // For Usb devices, this value should be always 1
  //
  *NumberBlockDevices = 1;
  return EFI_SUCCESS;
}

/**

  Gets a block device's media information.

  @param  PeiServices           General purpose services that are available to every PEIM
  @param  This                  Indicates the EFI_PEI_RECOVERY_BLOCK_IO_PPI instance.
  @param  DeviceIndex           The number from one to NumberBlockDevices.
  @param  MediaInfo             The media information of the specified block media. 

  @retval EFI_SUCCESS           Media information about the specified block device was obtained successfully.
  @retval EFI_DEVICE_ERROR      Cannot get the media information due to a hardware error.

**/
EFI_STATUS
EFIAPI
GetBlockDeviceMediaInfo (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO_PPI     *This,
  IN  UINTN                             DeviceIndex,
  OUT EFI_PEI_BLOCK_IO_MEDIA            *MediaInfo
  )
{
  PEI_BLKIO_DEVICE   *PeiBlkIoDev;
  EFI_STATUS         Status;
  UINTN              Index;
  
  Status      = EFI_SUCCESS;
  PeiBlkIoDev = PEI_BLKIO_DEVICE_FROM_THIS (This);
  for (Index = 0; Index < MAX_INIT_MEDIA_RETRY; Index ++) {
    Status = PeiUsbDetectMedia (
               PeiServices,
               PeiBlkIoDev
               );
    if (!EFI_ERROR (Status)) {
      *MediaInfo = PeiBlkIoDev->Media;
      break;
    } else if (Status == EFI_NO_MEDIA) {
      break;
    } else if (Status == EFI_NOT_READY) {
      //
      // Given stall to waiting for device ready
      //
      PeiBlkIoDev->StallPpi->Stall (
                               (CONST EFI_PEI_SERVICES **)PeiServices,
                               PeiBlkIoDev->StallPpi,
                               1000 * 1000 * (Index + 1)
                               );
    }
  }
  return Status;
}

/**

  Reads the requested number of blocks from the specified block device.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  This                  Indicates the EFI_PEI_RECOVERY_BLOCK_IO_PPI instance.
  @param  DeviceIndex           The number from one to NumberBlockDevices.
  @param  StartLBA              The starting logical block address (LBA) to read from on the device
  @param  BufferSize            The size of the Buffer in bytes. This number must be 
                                a multiple of the intrinsic block size of the device.
  @param  Buffer                A pointer to the destination buffer for the data.
                                The caller is responsible for the ownership of the buffer.

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while attempting to perform the read operation.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid, or the buffer is not properly aligned.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_BAD_BUFFER_SIZE   The BufferSize parameter is not a multiple of the intrinsic block size of the device.

**/
EFI_STATUS
EFIAPI
ReadBlocks (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO_PPI     *This,
  IN  UINTN                             DeviceIndex,
  IN  EFI_PEI_LBA                       StartLBA,
  IN  UINTN                             BufferSize,
  OUT VOID                              *Buffer
  )
{
  PEI_BLKIO_DEVICE   *PeiBlkIoDev;
  EFI_STATUS         Status;
  UINTN              BlockSize;
  UINTN              NumberOfBlocks;

  Status      = EFI_SUCCESS;
  PeiBlkIoDev = PEI_BLKIO_DEVICE_FROM_THIS (This);

  //
  // Check parameters
  //
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  BlockSize = PeiBlkIoDev->Media.BlockSize;

  if (BufferSize % BlockSize != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
  }

  if (!PeiBlkIoDev->Media.MediaPresent) {
    return EFI_NO_MEDIA;
  }

  if (StartLBA > PeiBlkIoDev->Media2.LastBlock) {
    Status = EFI_INVALID_PARAMETER;
  }

  NumberOfBlocks = BufferSize / (PeiBlkIoDev->Media.BlockSize);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Update the media info prior to every read command
  //
  Status = PeiUsbDetectMedia (
             PeiServices,
             PeiBlkIoDev
             );
  if (Status != EFI_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  if (!(PeiBlkIoDev->Media.MediaPresent)) {
    return EFI_NO_MEDIA;
  }
  if (BufferSize % (PeiBlkIoDev->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }
  if (StartLBA > PeiBlkIoDev->Media2.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }
  if ((StartLBA + NumberOfBlocks - 1) > PeiBlkIoDev->Media2.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }
  Status = PeiUsbRead (
             PeiServices,
             PeiBlkIoDev,
             Buffer,
             StartLBA,
             NumberOfBlocks
             );
  switch (Status) {
  case EFI_SUCCESS:
    return EFI_SUCCESS;

  default:
    return EFI_DEVICE_ERROR;

  }
}

/**

  Write the requested number of blocks to the specified block device.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  This                  Indicates the H2O_PEI_BLOCK_IO_PPI instance.
  @param  DeviceIndex           The number from one to NumberBlockDevices.
  @param  StartLBA              The starting logical block address (LBA) to read from on the device
  @param  BufferSize            The size of the Buffer in bytes. This number must be 
                                a multiple of the intrinsic block size of the device.
  @param  Buffer                A pointer to the destination buffer for the data.
                                The caller is responsible for the ownership of the buffer.

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while attempting to perform the read operation.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid, or the buffer is not properly aligned.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_BAD_BUFFER_SIZE   The BufferSize parameter is not a multiple of the intrinsic block size of the device.

**/
EFI_STATUS
EFIAPI
WriteBlocks (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  H2O_PEI_BLOCK_IO_PPI              *This,
  IN  UINTN                             DeviceIndex,
  IN  EFI_PEI_LBA                       StartLBA,
  IN  UINTN                             BufferSize,
  OUT VOID                              *Buffer
  )
{
  PEI_BLKIO_DEVICE   *PeiBlkIoDev;
  EFI_STATUS         Status;
  UINTN              BlockSize;
  
  UINTN              NumberOfBlocks;

  Status      = EFI_SUCCESS;
  PeiBlkIoDev = H2O_PEI_BLKIO_DEVICE_FROM_THIS (This);

  //
  // Check parameters
  //
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferSize == 0) {
    return EFI_SUCCESS;
  }

  BlockSize = PeiBlkIoDev->Media.BlockSize;

  if (BufferSize % BlockSize != 0) {
    Status = EFI_BAD_BUFFER_SIZE;
  }

  if (!PeiBlkIoDev->Media.MediaPresent) {
    return EFI_NO_MEDIA;
  }

  if (StartLBA > PeiBlkIoDev->Media2.LastBlock) {
    Status = EFI_INVALID_PARAMETER;
  }

  NumberOfBlocks = BufferSize / (PeiBlkIoDev->Media.BlockSize);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Update the media info prior to every read command
  //
  Status = PeiUsbDetectMedia (
             PeiServices,
             PeiBlkIoDev
             );
  if (Status != EFI_SUCCESS) {
    return EFI_DEVICE_ERROR;
  }
  if (!(PeiBlkIoDev->Media.MediaPresent)) {
    return EFI_NO_MEDIA;
  }
  if (BufferSize % (PeiBlkIoDev->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }
  if (StartLBA > PeiBlkIoDev->Media2.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }
  if ((StartLBA + NumberOfBlocks - 1) > PeiBlkIoDev->Media2.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }
  Status = PeiUsbWrite (
             PeiServices,
             PeiBlkIoDev,
             Buffer,
             StartLBA,
             NumberOfBlocks
             );
  switch (Status) {
  case EFI_SUCCESS:
    return EFI_SUCCESS;

  default:
    return EFI_DEVICE_ERROR;

  }
}

/**
  Get the number of block devices.

  @param[in] PeiServices         General purpose services available to every PEIM
  @param[in] This                Pointer to protocol instance
  @param[out] NumberBlockDevices Number of block devices

  @retval EFI_SUCCESS            Success.
  @retval EFI_INVALID_PARAMETER  Invalid parameter passed in

**/
EFI_STATUS
EFIAPI
GetNumberOfBlockDevices2 (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO2_PPI    *This,
  OUT UINTN                             *NumberBlockDevices
  )
{
  PEI_BLKIO_DEVICE   *PeiBlkIoDev;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  PeiBlkIoDev = PEI_BLKIO2_DEVICE_FROM_THIS (This);
  return GetNumberOfBlockDevices (
           PeiServices,
           &PeiBlkIoDev->BlkIoPpi,
           NumberBlockDevices
           );
}

/**
  Get the media information of block devices.

  @param[in] PeiServices        General purpose services available to every PEIM
  @param[in] This               Pointer to protocol instance
  @param[in] DeviceIndex        Index of device
  @param[out] MediaInfo         Pointer to media information

  @retval EFI_SUCCESS           Success.
  @retval EFI_INVALID_PARAMETER Invalid parameter passed in
  @retval EFI_DEVICE_ERROR      Media information cannot be retrieved from device

**/
EFI_STATUS
EFIAPI
GetBlockDeviceMediaInfo2 (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO2_PPI    *This,
  IN  UINTN                             DeviceIndex,
  OUT EFI_PEI_BLOCK_IO2_MEDIA           *MediaInfo2
  )
{
  EFI_STATUS                    Status;
  PEI_BLKIO_DEVICE              *PeiBlkIoDev;
  EFI_PEI_BLOCK_IO_MEDIA        MediaInfo;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  PeiBlkIoDev = PEI_BLKIO2_DEVICE_FROM_THIS (This);
  Status = GetBlockDeviceMediaInfo (
             PeiServices,
             &PeiBlkIoDev->BlkIoPpi,
             DeviceIndex,
             &MediaInfo
             );
  if (!EFI_ERROR (Status)) {
    CopyMem (
      MediaInfo2,
      &PeiBlkIoDev->Media2,
      sizeof (EFI_PEI_BLOCK_IO2_MEDIA)
      );
  }
  return Status;
}

/**
  Read block function for Atapi blockIo devices.

  @param[in] PeiServices        General purpose services available to every PEIM
  @param[in] This               Pointer to protocol instance
  @param[in] DeviceIndex        Index of device
  @param[in] StartLba           Starting LBA address to read data
  @param[in] BufferSize         The size of data to be read
  @param[out] Buffer            Caller supplied buffer to save data

  @retval EFI_SUCCESS           The data were read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while attempting to perform the read operation.
  @retval EFI_BAD_BUFFER_SIZE   The BufferSize parameter is not a multiple of the intrinsic block
                                size of the device.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid, or the buffer is not
                                on proper alignment.

**/
EFI_STATUS
EFIAPI
ReadBlocks2 (
  IN  EFI_PEI_SERVICES                  **PeiServices,
  IN  EFI_PEI_RECOVERY_BLOCK_IO2_PPI    *This,
  IN  UINTN                             DeviceIndex,
  IN  EFI_PEI_LBA                       StartLba,
  IN  UINTN                             BufferSize,
  OUT VOID                              *Buffer
  )
{
  PEI_BLKIO_DEVICE   *PeiBlkIoDev;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  PeiBlkIoDev = PEI_BLKIO2_DEVICE_FROM_THIS (This);
  return ReadBlocks (
           PeiServices,
           &PeiBlkIoDev->BlkIoPpi,
           DeviceIndex,
           StartLba,
           BufferSize,
           Buffer
           );
}

/**

  Detect whether the removable media is present and whether it has changed.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.

  @retval EFI_SUCCESS           Command executed successfully.
  @retval EFI_DEVICE_ERROR      Some device errors happen.

**/
EFI_STATUS
PeiUsbDetectMedia (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev
  )
{
  EFI_STATUS               Status;
  UINTN                    SenseCounts;
  ATAPI_REQUEST_SENSE_DATA *SensePtr;
  //
  // Use test unit ready to see any error
  //
  Status = PeiUsbTestUnitReady (
             PeiServices,
             PeiBlkIoDev
             );
  if (EFI_ERROR (Status)) { 
    //
    // if there is no media present,or media not changed,
    // the request sense command will detect faster than read capacity command.
    // read capacity command can be bypassed, thus improve performance.
    //
    SenseCounts      = 0;
    SensePtr         = PeiBlkIoDev->SenseData;
    ZeroMem (SensePtr, EFI_PAGE_SIZE);
    Status = PeiUsbRequestSense (
               PeiServices,
               PeiBlkIoDev,
               &SenseCounts,
               (UINT8 *) SensePtr
               );
    if (!EFI_ERROR (Status)) {
      if (IsNoMedia (SensePtr, SenseCounts) || IsMediaError (SensePtr, SenseCounts)) {
        //
        // No Media or media error encountered, make it look like no media present.
        //
        PeiBlkIoDev->Media.MediaPresent = FALSE;
        PeiBlkIoDev->Media.LastBlock    = 0;
        return EFI_NO_MEDIA;
      } else if (IsMediaChange (SensePtr, SenseCounts)) {
        //
        // Media Changed
        //
        PeiBlkIoDev->Media.LastBlock    = 0;
      } else if (IsDeviceBusy (SensePtr, SenseCounts)) {
        //
        // Device not ready
        //
        return EFI_NOT_READY;
      } else {
        Status = EFI_DEVICE_ERROR;
      }
    }
  }
  if (PeiBlkIoDev->Media.LastBlock == 0) {
    Status = PeiUsbReadCapacity (
               PeiServices,
               PeiBlkIoDev
               );
  }
  return Status;
}

/**

  Sends out ATAPI Inquiry Packet Command to the specified device.
  This command will return INQUIRY data of the device.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.

  @retval EFI_SUCCESS           Inquiry command completes successfully.
  @retval EFI_DEVICE_ERROR      Inquiry command failed.

**/
EFI_STATUS
PeiUsbInquiry (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev
  )
{
  ATAPI_PACKET_COMMAND  Packet;
  EFI_STATUS            Status;
  ATAPI_INQUIRY_DATA    Idata;

  //
  // fill command packet
  //
  ZeroMem (&Packet, sizeof (ATAPI_PACKET_COMMAND));
  ZeroMem (&Idata, sizeof (ATAPI_INQUIRY_DATA));

  Packet.Inquiry.opcode             = ATA_CMD_INQUIRY;
  Packet.Inquiry.page_code          = 0;
  Packet.Inquiry.allocation_length  = 36;

  //
  // send command packet
  //
  Status = PeiBlkIoDev->AtapiCommand (
                          PeiServices,
                          PeiBlkIoDev,
                          &Packet,
                          sizeof (ATAPI_PACKET_COMMAND),
                          &Idata,
                          36,
                          EfiUsbDataIn,
                          2000
                          );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  if ((Idata.peripheral_type & 0x1f) == 0x05) {
    PeiBlkIoDev->DeviceType      = USBCDROM;
    PeiBlkIoDev->Media.BlockSize = 0x800;
  } else {
    PeiBlkIoDev->DeviceType      = USBFLOPPY;
    PeiBlkIoDev->Media.BlockSize = 0x200;
  }

  return EFI_SUCCESS;
}

/**

  Sends out ATAPI Test Unit Ready Packet Command to the specified device
  to find out whether device is accessible.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.

  @retval EFI_SUCCESS           TestUnit command executed successfully.
  @retval EFI_DEVICE_ERROR      Device cannot be executed TestUnit command successfully.

**/
EFI_STATUS
PeiUsbTestUnitReady (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev
  )
{
  ATAPI_PACKET_COMMAND  Packet;
  EFI_STATUS            Status;

  //
  // fill command packet
  //
  ZeroMem (&Packet, sizeof (ATAPI_PACKET_COMMAND));
  Packet.TestUnitReady.opcode = ATA_CMD_TEST_UNIT_READY;

  //
  // send command packet
  //
  Status = PeiBlkIoDev->AtapiCommand (
                          PeiServices,
                          PeiBlkIoDev,
                          &Packet,
                          sizeof (ATAPI_PACKET_COMMAND),
                          NULL,
                          0,
                          EfiUsbNoData,
                          2000
                          );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**

  Sends out ATAPI Request Sense Packet Command to the specified device.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.
  @param  SenseCounts            Length of sense buffer.
  @param  SenseKeyBuffer        Pointer to sense buffer.

  @retval EFI_SUCCESS           Command executed successfully.
  @retval EFI_DEVICE_ERROR      Some device errors happen.

**/
EFI_STATUS
PeiUsbRequestSense (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev,
  OUT UINTN             *SenseCounts,
  IN  UINT8             *SenseKeyBuffer
  )
{
  EFI_STATUS               Status;
  ATAPI_PACKET_COMMAND     Packet;
  UINT8                    *Ptr;
  BOOLEAN                  SenseReq;
  ATAPI_REQUEST_SENSE_DATA *Sense;

  *SenseCounts = 0;

  //
  // fill command packet for Request Sense Packet Command
  //
  ZeroMem (&Packet, sizeof (ATAPI_PACKET_COMMAND));
  Packet.RequestSence.opcode            = ATA_CMD_REQUEST_SENSE;
  Packet.RequestSence.allocation_length = sizeof (ATAPI_REQUEST_SENSE_DATA);

  Ptr = SenseKeyBuffer;

  SenseReq = TRUE;

  //
  //  request sense data from device continuously
  //  until no sense data exists in the device.
  //
  while (SenseReq) {
    Sense = (ATAPI_REQUEST_SENSE_DATA *) Ptr;

    //
    // send out Request Sense Packet Command and get one Sense
    // data form device.
    //
    Status = PeiBlkIoDev->AtapiCommand (
                            PeiServices,
                            PeiBlkIoDev,
                            &Packet,
                            sizeof (ATAPI_PACKET_COMMAND),
                            (VOID *) Ptr,
                            sizeof (ATAPI_REQUEST_SENSE_DATA),
                            EfiUsbDataIn,
                            2000
                            );

    //
    // failed to get Sense data
    //
    if (EFI_ERROR (Status)) {
      if (*SenseCounts == 0) {
        return EFI_DEVICE_ERROR;
      } else {
        return EFI_SUCCESS;
      }
    }

    if (Sense->sense_key != ATA_SK_NO_SENSE) {

      Ptr += sizeof (ATAPI_REQUEST_SENSE_DATA);
      //
      // Ptr is byte based pointer
      //
      (*SenseCounts)++;

      if (*SenseCounts == MAXSENSEKEY) {
        break;
      }

    } else {
      //
      // when no sense key, skip out the loop
      //
      SenseReq = FALSE;
    }
  }

  return EFI_SUCCESS;
}

/**

  Sends out ATAPI Read Capacity Packet Command to the specified device.
  This command will return the information regarding the capacity of the
  media in the device.

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.

  @retval EFI_SUCCESS           Command executed successfully.
  @retval EFI_DEVICE_ERROR      Some device errors happen.

**/
EFI_STATUS
PeiUsbReadCapacity (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev
  )
{
  EFI_STATUS                    Status;
  ATAPI_PACKET_COMMAND          CapacityCmd;
  ATAPI_PACKET16_COMMAND        Capacity16Cmd;
  ATAPI_READ_CAPACITY_DATA      Data;
  ATAPI_READ_CAPACITY16_DATA    Data16;

  ZeroMem (&Data, sizeof (ATAPI_READ_CAPACITY_DATA));
  ZeroMem (&CapacityCmd, sizeof (ATAPI_PACKET_COMMAND));
  CapacityCmd.Inquiry.opcode = ATA_CMD_READ_CAPACITY;
  //
  // send command packet
  //
  Status = PeiBlkIoDev->AtapiCommand (
                          PeiServices,
                          PeiBlkIoDev,
                          &CapacityCmd,
                          sizeof (ATAPI_PACKET_COMMAND),
                          (VOID *) &Data,
                          sizeof (ATAPI_READ_CAPACITY_DATA),
                          EfiUsbDataIn,
                          2000
                          );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }
  PeiBlkIoDev->Media.LastBlock    = SwapBytes32 (*(UINT32*)&Data.LastLba3);
  PeiBlkIoDev->Media.BlockSize    = SwapBytes32 (*(UINT32*)&Data.BlockSize3);
  PeiBlkIoDev->Media.MediaPresent = TRUE;
  //
  // 2.1T large size HDD checking
  //
  if (PeiBlkIoDev->Media.LastBlock == 0xffffffff) {
    ZeroMem (&Data16, sizeof (ATAPI_READ_CAPACITY16_DATA));
    ZeroMem (&Capacity16Cmd, sizeof (ATAPI_PACKET16_COMMAND));

    Capacity16Cmd.OpCode = ATA_CMD_READ_CAPACITY16;
    Capacity16Cmd.Lun    = 0x10;
    Capacity16Cmd.TransferLen[3] = sizeof (ATAPI_READ_CAPACITY16_DATA);

    Status = PeiBlkIoDev->AtapiCommand (
                            PeiServices,
                            PeiBlkIoDev,
                            &Capacity16Cmd,
                            sizeof (ATAPI_PACKET16_COMMAND),
                            (VOID *) &Data16,
                            sizeof (ATAPI_READ_CAPACITY16_DATA),
                            EfiUsbDataIn,
                            2000
                            );

    if (EFI_ERROR (Status)) { 
      return Status;
    }
    PeiBlkIoDev->Media2.LastBlock    = SwapBytes64 (*(UINT64*)Data16.LastLba);
    PeiBlkIoDev->Media2.BlockSize    = SwapBytes32 (*(UINT32*)Data16.BlockLen);
    PeiBlkIoDev->Media2.MediaPresent = TRUE;
  } else {
    PeiBlkIoDev->Media2.LastBlock    = PeiBlkIoDev->Media.LastBlock;
    PeiBlkIoDev->Media2.BlockSize    = PeiBlkIoDev->Media.BlockSize;
    PeiBlkIoDev->Media2.MediaPresent = TRUE;
  }
  return EFI_SUCCESS;
}

/**

  PEI UFI Read10 comand

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.
  @param  Buffer                The pointer to data buffer.
  @param  Lba                   The start logic block address of reading.
  @param  NumberOfBlocks        The block number of reading.

  @retval EFI_SUCCESS           Command executed successfully.
  @retval EFI_DEVICE_ERROR      Some device errors happen.

**/
EFI_STATUS
PeiUsbRead (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev,
  IN  VOID              *Buffer,
  IN  EFI_PEI_LBA       Lba,
  IN  UINTN             NumberOfBlocks
  )
{
  EFI_STATUS                    Status;
  UINT8                         Cmd[16];
  ATAPI_READ10_CMD              *Read10Cmd;
  ATAPI_PACKET16_COMMAND        *Read16Cmd;
  UINT16                        MaxBlock;
  UINT16                        BlocksRemaining;
  UINT16                        SectorCount;
  UINT32                        BlockSize;
  UINT32                        ByteCount;
  UINT8                         *BufferPtr;
  UINT16                        Timeout;
  UINT8                         CmdLen;

  ZeroMem (&Cmd, 16);
  BufferPtr       = Buffer;
  BlockSize       = (UINT32) PeiBlkIoDev->Media.BlockSize;
  MaxBlock        = (UINT16) (65535 / BlockSize);
  BlocksRemaining = (UINT16) NumberOfBlocks;
  Status          = EFI_SUCCESS;
  while (BlocksRemaining > 0) {
    if (BlocksRemaining <= MaxBlock) {
      SectorCount = BlocksRemaining;
    } else {
      SectorCount = MaxBlock;
    }
    if (PeiBlkIoDev->Media.LastBlock < 0xffffffff) {
      //
      // Issuing Raed10 command for HDD < 2.1T 
      //
      Read10Cmd = (ATAPI_READ10_CMD*)Cmd;
      CmdLen    = sizeof (ATAPI_PACKET_COMMAND);
      Read10Cmd->opcode  = ATA_CMD_READ_10;
      *(UINT32*)&Read10Cmd->Lba0 = SwapBytes32 ((UINT32)Lba);
      *(UINT16*)&Read10Cmd->TranLen0 = SwapBytes16 (SectorCount);
    } else {
      //
      // Issuing Raed16 command for HDD > 2.1T 
      //
      Read16Cmd = (ATAPI_PACKET16_COMMAND*)Cmd;
      CmdLen    = sizeof (ATAPI_PACKET16_COMMAND);
      Read16Cmd->OpCode  = ATA_CMD_READ_16;
      *(UINT64*)Read16Cmd->Lba = SwapBytes64 (Lba);
      *(UINT32*)Read16Cmd->TransferLen = SwapBytes32 (SectorCount);
    }
    ByteCount               = SectorCount * BlockSize;
    Timeout                 = (UINT16) (SectorCount * 2000);
    //
    // Send command packet
    //
    Status = PeiBlkIoDev->AtapiCommand (
                            PeiServices,
                            PeiBlkIoDev,
                            Cmd,
                            CmdLen,
                            BufferPtr,
                            ByteCount,
                            EfiUsbDataIn,
                            Timeout
                            );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    Lba += SectorCount;
    BufferPtr       = BufferPtr + SectorCount * BlockSize;
    BlocksRemaining = (UINT16) (BlocksRemaining - SectorCount);
  }
  return Status;
}

/**

  PEI UFI Write10 comand

  @param  PeiServices           General purpose services that are available to every PEIM.
  @param  PeiBlkIoDev           Indicates the PEI_BLKIO_DEVICE instance.
  @param  Buffer                The pointer to data buffer.
  @param  Lba                   The start logic block address of reading.
  @param  NumberOfBlocks        The block number of reading.

  @retval EFI_SUCCESS           Command executed successfully.
  @retval EFI_DEVICE_ERROR      Some device errors happen.

**/
EFI_STATUS
PeiUsbWrite (
  IN  EFI_PEI_SERVICES  **PeiServices,
  IN  PEI_BLKIO_DEVICE  *PeiBlkIoDev,
  IN  VOID              *Buffer,
  IN  EFI_PEI_LBA       Lba,
  IN  UINTN             NumberOfBlocks
  )
{
  EFI_STATUS                    Status;
  UINT8                         Cmd[16];
  ATAPI_READ10_CMD              *Write10Cmd;
  ATAPI_PACKET16_COMMAND        *Write16Cmd;
  UINT16                        MaxBlock;
  UINT16                        BlocksRemaining;
  UINT16                        SectorCount;
  UINT32                        BlockSize;
  UINT32                        ByteCount;
  UINT8                         *BufferPtr;
  UINT16                        Timeout;
  UINT8                         CmdLen;

  ZeroMem (&Cmd, 16);
  BufferPtr       = Buffer;
  BlockSize       = (UINT32) PeiBlkIoDev->Media.BlockSize;
  MaxBlock        = (UINT16) (65535 / BlockSize);
  BlocksRemaining = (UINT16) NumberOfBlocks;
  Status          = EFI_SUCCESS;
  while (BlocksRemaining > 0) {
    if (BlocksRemaining <= MaxBlock) {
      SectorCount = BlocksRemaining;
    } else {
      SectorCount = MaxBlock;
    }
    if (PeiBlkIoDev->Media.LastBlock < 0xffffffff) {
      //
      // Issuing Write10 command for HDD < 2.1T 
      //
      Write10Cmd = (ATAPI_READ10_CMD*)Cmd;
      CmdLen    = sizeof (ATAPI_PACKET_COMMAND);
      Write10Cmd->opcode  = ATA_CMD_WRITE_10;
      *(UINT32*)&Write10Cmd->Lba0 = SwapBytes32 ((UINT32)Lba);
      *(UINT16*)&Write10Cmd->TranLen0 = SwapBytes16 (SectorCount);
    } else {
      //
      // Issuing Write16 command for HDD > 2.1T 
      //
      Write16Cmd = (ATAPI_PACKET16_COMMAND*)Cmd;
      CmdLen    = sizeof (ATAPI_PACKET16_COMMAND);
      Write16Cmd->OpCode  = ATA_CMD_WRITE_16;
      *(UINT64*)Write16Cmd->Lba = SwapBytes64 (Lba);
      *(UINT32*)Write16Cmd->TransferLen = SwapBytes32 (SectorCount);
    }
    ByteCount                = SectorCount * BlockSize;
    Timeout                  = (UINT16) (SectorCount * 2000);
    //
    // Send command packet
    //
    Status = PeiBlkIoDev->AtapiCommand (
                            PeiServices,
                            PeiBlkIoDev,
                            Cmd,
                            CmdLen,
                            BufferPtr,
                            ByteCount,
                            EfiUsbDataOut,
                            Timeout
                            );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
    Lba += SectorCount;
    BufferPtr       = BufferPtr + SectorCount * BlockSize;
    BlocksRemaining = (UINT16) (BlocksRemaining - SectorCount);
  }
  return Status;
}

/**

  Check if there is media according to sense data.

  @param  SenseData             Pointer to sense data.
  @param  SenseCounts           SenseCounts Count of sense data.

  @retval TRUE                  No media
  @retval FALSE                 Media exists

**/
BOOLEAN
IsNoMedia (
  IN  ATAPI_REQUEST_SENSE_DATA  *SenseData,
  IN  UINTN                     SenseCounts
  )
{
  ATAPI_REQUEST_SENSE_DATA *SensePtr;
  UINTN                    i;
  BOOLEAN                  NoMedia;

  NoMedia   = FALSE;
  SensePtr  = SenseData;

  for (i = 0; i < SenseCounts; i++) {

    switch (SensePtr->sense_key) {

    case ATA_SK_NOT_READY:
      switch (SensePtr->addnl_sense_code) {
      //
      // if no media, fill IdeDev parameter with specific info.
      //
      case ATA_ASC_NO_MEDIA:
        NoMedia = TRUE;
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }

    SensePtr++;
  }

  return NoMedia;
}

/**

  Check if there is media error according to sense data.

  @param  SenseData             Pointer to sense data.
  @param  SenseCounts           Count of sense data.

  @retval TRUE                  Media error
  @retval FALSE                 No media error

**/
BOOLEAN
IsMediaError (
  IN  ATAPI_REQUEST_SENSE_DATA  *SenseData,
  IN  UINTN                     SenseCounts
  )
{
  ATAPI_REQUEST_SENSE_DATA *SensePtr;
  UINTN                    i;
  BOOLEAN                  Error;

  SensePtr  = SenseData;
  Error     = FALSE;

  for (i = 0; i < SenseCounts; i++) {

    switch (SensePtr->sense_key) {
    //
    // Medium error case
    //
    case ATA_SK_MEDIUM_ERROR:
      switch (SensePtr->addnl_sense_code) {
      case ATA_ASC_MEDIA_ERR1:
        //
        // fall through
        //
      case ATA_ASC_MEDIA_ERR2:
        //
        // fall through
        //
      case ATA_ASC_MEDIA_ERR3:
        //
        // fall through
        //
      case ATA_ASC_MEDIA_ERR4:
        Error = TRUE;
        break;

      default:
        break;
      }

      break;

    //
    // Medium upside-down case
    //
    case ATA_SK_NOT_READY:
      switch (SensePtr->addnl_sense_code) {
      case ATA_ASC_MEDIA_UPSIDE_DOWN:
        Error = TRUE;
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }

    SensePtr++;
  }

  return Error;
}

/**

  Check if media is changed according to sense data.

  @param  SenseData             Pointer to sense data.
  @param  SenseCounts           SenseCounts Count of sense data.

  @retval TRUE                  There is media change event.
  @retval FALSE                 media is NOT changed.

**/
BOOLEAN
IsMediaChange (
  IN  ATAPI_REQUEST_SENSE_DATA  *SenseData,
  IN  UINTN                     SenseCounts
  )
{
  ATAPI_REQUEST_SENSE_DATA *SensePtr;
  UINTN                    i;
  BOOLEAN                  MediaChange;

  MediaChange = FALSE;

  SensePtr    = SenseData;

  for (i = 0; i < SenseCounts; i++) {
    //
    // catch media change sense key and addition sense data
    //
    switch (SensePtr->sense_key) {
    case ATA_SK_UNIT_ATTENTION:
      switch (SensePtr->addnl_sense_code) {
      case ATA_ASC_MEDIA_CHANGE:
        MediaChange = TRUE;
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }

    SensePtr++;
  }

  return MediaChange;
}

/**

  Check is the device busy according to sense data.

  @param  SenseData             Pointer to sense data.
  @param  SenseCounts           SenseCounts Count of sense data.

  @retval TRUE                  Device is busy
  @retval FALSE                 Device is not busy

**/
BOOLEAN
IsDeviceBusy (
  IN  ATAPI_REQUEST_SENSE_DATA  *SenseData,
  IN  UINTN                     SenseCounts
  )
{
  ATAPI_REQUEST_SENSE_DATA *SensePtr;
  UINTN                    i;
  BOOLEAN                  DeviceBusy;

  DeviceBusy = FALSE;

  SensePtr    = SenseData;

  for (i = 0; i < SenseCounts; i++) {
    //
    // catch media change sense key and addition sense data
    //
    switch (SensePtr->sense_key) {
    case ATA_SK_RECOVERY_ERROR:
      DeviceBusy = TRUE;
      break;
    case ATA_SK_NOT_READY:
      switch (SensePtr->addnl_sense_code) {
      case ATA_ASC_NOT_READY:
        if (SensePtr->addnl_sense_code_qualifier == ATA_ASCQ_IN_PROGRESS ||
            SensePtr->addnl_sense_code_qualifier == 0xff) {
          DeviceBusy = TRUE;
        }
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }

    SensePtr++;
  }

  return DeviceBusy;
}
