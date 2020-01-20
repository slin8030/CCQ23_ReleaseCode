/** @file
  For I2C Master Driver

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "I2cMasterDxe.h"

//
// Globals
//
VOID                    *mI2cContext = NULL;
EFI_HANDLE              mImageHandle;

I2C_PCI_HC_DEV_PATH mI2cPciHcDevPath = {
  {{ HARDWARE_DEVICE_PATH, HW_PCI_DP,                      sizeof(PCI_DEVICE_PATH)},  0, 0        },
  { END_DEVICE_PATH_TYPE,  END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof(EFI_DEVICE_PATH)                }
};

I2C_MEM_HC_DEV_PATH mI2cMemHcDevPath = {
  {{ HARDWARE_DEVICE_PATH, HW_MEMMAP_DP,                   sizeof(MEMMAP_DEVICE_PATH)},  0, 0, 0  },
  { END_DEVICE_PATH_TYPE,  END_ENTIRE_DEVICE_PATH_SUBTYPE, sizeof(EFI_DEVICE_PATH)                }
};


/**
  Set the I2C controller bus clock frequency.

  This routine must be called at or below TPL_NOTIFY.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the controller will use a slightly lower frequency for the I2C to avoid
  exceeding the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the controller would be set to 398
  KHz.  However if the desired frequency is 400 KHz and the controller only
  supports 1 MHz and 100 KHz then this routine would return EFI_UNSUPPORTED.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval EFI_SUCCESS       The bus frequency was set successfully.
  @retval EFI_UNSUPPORTED   The controller does not support this frequency.

**/
EFI_STATUS
EFIAPI
I2cPortSetBusFrequency (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This,
  IN OUT UINTN                       *BusClockHertz
  )
{
  I2C_BUS_INSTANCE   *I2cContext;
  EFI_STATUS         Status;

  Status = EFI_SUCCESS;
  
  DEBUG (( EFI_D_INFO, "I2cPortSetBusFrequency entered\r\n" ));

  //
  //  Locate the configuration
  //
  I2cContext = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  switch (*BusClockHertz) {
  case 100000:
    I2cContext->I2cBusSpeed = V_SPEED_STANDARD;
    break;
  case 400000:
    I2cContext->I2cBusSpeed = V_SPEED_FAST;    
    break;    
  case 1000000:
    I2cContext->I2cBusSpeed = V_SPEED_HIGH;    
    break;  
  default:
    *BusClockHertz = 400000;
    I2cContext->I2cBusSpeed = V_SPEED_STANDARD;    
    return EFI_UNSUPPORTED;
  }
  //
  //  Display the operation
  //
  DEBUG (( EFI_D_INFO, "Setting I2C bus frequency to %d KHz\r\n", (*BusClockHertz) / 1000 ));

  return Status;
}


/**
  Reset the I2C controller and configure it for use

  This routine must be called at or below TPL_NOTIFY.

  The controller's I2C bus frequency is set to 100 KHz.

  @param[in]     This       Address of an EFI_I2C_MASTER_PROTOCOL
                            structure

  @retval EFI_SUCCESS       The bus reset was set successfully.

**/
EFI_STATUS
I2cPortReset (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This
  )
{
  EFI_STATUS             Status;
  I2C_BUS_INSTANCE       *I2cPort;
  UINT32                 PciCmdBackUp;
  BOOLEAN                IsSwitched;
  EFI_PHYSICAL_ADDRESS   Base0BackUp;
  
  Status = EFI_SUCCESS;
  
  DEBUG (( EFI_D_INFO, "I2cPortReset entered\r\n" ));

  //
  //  Locate the configuration
  //
  I2cPort = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  IsSwitched = FALSE;
  
  if ( I2cPort->Info.GetDeviceMode (I2cPort) == I2C_DEVICE_MODE_ACPI) {
    I2cPort->Info.SwitcDevhMode (I2cPort, I2C_DEVICE_MODE_PCI);
    IsSwitched = TRUE;  
  }
  
  LocateI2cBase( I2cPort, &Base0BackUp, &PciCmdBackUp);

  if (I2cPort->I2cBase == 0 || I2cPort->I2cHcRead8(I2cPort, R_IC_CON) == 0xFF) {
    FreeI2cBase(I2cPort, Base0BackUp, PciCmdBackUp);
    return EFI_DEVICE_ERROR;
  }


  //
  //  Reset the host
  //
  I2cReset (I2cPort);

  FreeI2cBase(I2cPort, Base0BackUp, PciCmdBackUp);

  if (IsSwitched ) {
    I2cPort->Info.SwitcDevhMode (I2cPort, I2C_DEVICE_MODE_ACPI);
  }
  
  DEBUG (( EFI_D_INFO, "I2cPortReset exiting\r\n" ));

  return Status;
}

/**
  Start an I2C operation on the controller

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  N.B. The typical consumer of this API is the I2C host driver.
  Extreme care must be taken by other consumers of this API to
  prevent confusing the third party I2C drivers due to a state
  change at the I2C device which the third party I2C drivers did
  not initiate.  I2C platform drivers may use this API within
  these guidelines.

  This function initiates an I2C operation on the controller.

  N.B. This API supports only one operation, no queuing support
  exists at this layer.

  The operation is performed by selecting the I2C device with its slave
  address and then sending all write data to the I2C device.  If read data
  is requested, a restart is sent followed by the slave address and then
  the read data is clocked into the I2C controller and placed in the read
  buffer.  When the operation completes, the status value is returned and
  then the event is set.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] SlaveAddress   Address of the device on the I2C bus.
  @param[in] Event          Event to set for asynchronous operations,
                            NULL for synchronous operations
  @param[in] RequestPacket  Address of an EFI_I2C_REQUEST_PACKET
                            structure describing the I2C operation
  @param[out] I2cStatus     Optional buffer to receive the I2C operation
                            completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status pointed to by
                                the request packet.
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cPortMasterStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This,
  IN UINTN                           SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET          *RequestPacket,
  IN EFI_EVENT                       Event      OPTIONAL,
  OUT EFI_STATUS                     *I2cStatus OPTIONAL
  )
{
  UINTN                       Index;
  UINTN                       WriteBytes;
  UINTN                       ReadBytes;
  UINT8                       *WriteBuffer;
  UINT8                       *ReadBuffer;
  BOOLEAN                     Repeat;
  BOOLEAN                     IsSwitched;
  EFI_STATUS                  Status;
  I2C_BUS_INSTANCE            *I2cPort;
  EFI_I2C_OPERATION           *Operation;
  UINT32                      PciCmdBackUp;
  EFI_PHYSICAL_ADDRESS        Base0BackUp;
  
  DEBUG (( EFI_D_INFO, "I2cPortMasterStartRequest entered\r\n" ));

  //
  //  Locate the configuration
  //
  I2cPort = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  if ( NULL != Event ) {
    return I2cPusAsyncQueue (I2cPort, SlaveAddress, RequestPacket, Event, I2cStatus);  
  }

  //
  //  Display the request
  //
  DEBUG (( EFI_D_INFO, "0x%016Lx: Starting I2C request packet 0x%016Lx, Slave: 0x%03x\r\n", (UINT64)(UINTN)I2cPort, (UINT64)(UINTN)RequestPacket, SlaveAddress ));

  if (SlaveAddress > 1023) {
    return RETURN_INVALID_PARAMETER;
  }

  if (RequestPacket == NULL) {
    return EFI_INVALID_PARAMETER;

  }

  Operation = RequestPacket->Operation;

  if ((Operation[0].LengthInBytes > This->I2cControllerCapabilities->MaximumTransmitBytes) ||
      (RequestPacket->OperationCount == 2 &&
      (Operation[1].LengthInBytes > This->I2cControllerCapabilities->MaximumTransmitBytes))
     ) {
    return EFI_BAD_BUFFER_SIZE;
  }

  //
  //  There doesn't support the SMBus PEC
  //
  if (Operation[0].Flags & I2C_FLAG_SMBUS_PEC) {
    Operation[0].Flags &= ~I2C_FLAG_SMBUS_PEC;
  } else if (Operation[1].Flags & I2C_FLAG_SMBUS_PEC) {
    Operation[1].LengthInBytes -=1 ;
  }

  //
  // QuickRead/QuickWrite
  //
  if (RequestPacket->OperationCount == 1 &&
    Operation[0].LengthInBytes == 0) {
    return EFI_UNSUPPORTED;
  }

  if ((Operation[0].Flags & I2C_FLAG_SMBUS_OPERATION) == I2C_FLAG_SMBUS_OPERATION) {

    //
    // Used common init
    //
    // ReadDataByte
    // ReadDataWord
    // ReadBlock
    // ProcessCall
    // BlkProcessCall
    Repeat      = FALSE;
    WriteBytes  = Operation[0].LengthInBytes;
    WriteBuffer = Operation[0].Buffer;
    ReadBytes   = Operation[1].LengthInBytes;
    ReadBuffer  = Operation[1].Buffer;

    //
    // For Writ data below
    // SendByte
    // WriteDataByte
    // WriteDataWord
    // WriteBlock
    //
    if ((Operation[0].Flags & I2C_FLAG_READ) == 0 &&
        (RequestPacket->OperationCount == 1)
        ) {
      ReadBytes   = 0;
      ReadBuffer  = NULL;
    }

    // ReceiveByte
    if ((Operation[0].Flags & I2C_FLAG_READ) &&
        (Operation[0].LengthInBytes == 1) &&
        (RequestPacket->OperationCount == 1)
        ) {
      WriteBytes  = 0;
      WriteBuffer = NULL;
    }

  } else {

    if (RequestPacket->OperationCount > 2) {
      return EFI_INVALID_PARAMETER;
    }

    Repeat      = FALSE;

    if (RequestPacket->OperationCount == 2) {
      if (((Operation[0].Flags & I2C_FLAG_READ) == I2C_FLAG_READ) && (Operation[1].Flags & I2C_FLAG_READ) == 0) {
        return EFI_INVALID_PARAMETER;
      }
      Repeat = TRUE;
    }

    ReadBytes   = 0;
    WriteBytes  = 0;
    ReadBuffer  = NULL;
    WriteBuffer = NULL;

    for (Index = 0;Index < RequestPacket->OperationCount; Index++) {
      if (Operation[Index].Flags & I2C_FLAG_READ) {
        ReadBytes   = Operation[Index].LengthInBytes;
        ReadBuffer  = Operation[Index].Buffer;
      } else {
        WriteBytes  = Operation[Index].LengthInBytes;
        WriteBuffer = Operation[Index].Buffer;
      }
    }
  }
  
  IsSwitched = FALSE;

  if ( I2cPort->Info.GetDeviceMode(I2cPort) == I2C_DEVICE_MODE_ACPI) {
    I2cPort->Info.SwitcDevhMode (I2cPort, I2C_DEVICE_MODE_PCI);
    IsSwitched = TRUE;  
  }

  LocateI2cBase( I2cPort, &Base0BackUp, &PciCmdBackUp);

  if (I2cPort->I2cBase == 0 || I2cPort->I2cHcRead8(I2cPort, R_IC_CON) == 0xFF) {
    FreeI2cBase(I2cPort, Base0BackUp, PciCmdBackUp);
    return EFI_DEVICE_ERROR;
  }

  I2cReset (I2cPort);

  //
  //  Start an I2C operation on the host
  //
  Status = I2cStartRequest (
             I2cPort,
             SlaveAddress,
             &WriteBytes,
             WriteBuffer,
             &ReadBytes,
             ReadBuffer,
             Repeat
             );

  FreeI2cBase(I2cPort, Base0BackUp, PciCmdBackUp);

  if (IsSwitched) {
    I2cPort->Info.SwitcDevhMode (I2cPort, I2C_DEVICE_MODE_ACPI);
  }

  if ((Operation[0].Flags & I2C_FLAG_SMBUS_BLOCK) == I2C_FLAG_SMBUS_BLOCK) {
    Operation[0].LengthInBytes = (UINT32)WriteBytes;
    Operation[1].LengthInBytes = (UINT32)ReadBytes;
  }
  //
  //  Return the status
  //
  if ( NULL != I2cStatus ) {
    DEBUG (( EFI_D_INFO,
             "0x%016Lx: Returning status for I2C request packet 0x%016Lx, Status: %r\r\n",
             (UINT64)(UINTN)I2cPort,
             (UINT64)(UINTN)RequestPacket,
             Status ));
    *I2cStatus = Status;
  }

  DEBUG (( EFI_D_INFO, "I2cPortMasterStartRequest exiting, Status: %r\r\n", Status ));
  return Status;
}

UINT16
GetCpuVendor(
  VOID
  )
{
  UINT32                RegEax, RegEbx, RegEcx, RegEdx;

  AsmCpuid( 0, &RegEax, &RegEbx, &RegEcx, &RegEdx);

  switch(RegEcx){
    case 0x6C65746E: // "ntel"
      return INTEL_VENDOR_ID;
    case 0x444D4163: // "DMAc"
      return AMD_VENDOR_ID;
    case 0x736C7561: // "CentaulsaurH"
       return VIA_VENDOR_ID;
    default:
      return 0x0000;
  }
}

EFI_STATUS
EFIAPI
InitializeI2cMasterEntry (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  )
/*++

Routine Description:

  Initialize the I2C bus Protocol

--*/
{
  UINT8                                  I2cHcType;
  UINT32                                 Eax;
  UINT32                                 Ebx;
  UINT32                                 Ecx;
  UINT32                                 Edx;
  UINTN                                  MaxI2cBus;
  EFI_STATUS                             Status;
  UINTN                                  DevPatLen;
  UINTN                                  Index;
  EFI_I2C_CONTROLLER_CAPABILITIES        *I2cControllerCapabilities;
   I2C_BUS_INSTANCE                       *I2cBusInstance;

  I2cBusInstance = NULL;
  Status         = EFI_SUCCESS;
  MaxI2cBus      = 0;
  DevPatLen      = 0;
  
  AsmCpuid( 1, &Eax, &Ebx, &Ecx, &Edx); // Get CPUID_VERSION_INFO

  Eax &= 0x0FFF0FF0;
  
  MaxI2cBus = GetTotalChipBus(Eax);

  if (MaxI2cBus == 0) {
    return EFI_NOT_STARTED;
  }
  
  DevPatLen = MAX(sizeof(I2C_PCI_HC_DEV_PATH), sizeof (I2C_MEM_HC_DEV_PATH));
  mI2cContext = AllocateZeroPool((sizeof(I2C_BUS_INSTANCE) + DevPatLen) * MaxI2cBus);

  if (mI2cContext == NULL){
    return EFI_OUT_OF_RESOURCES;
  }

  for(Index = 0; Index < MaxI2cBus; Index++){

    I2cBusInstance = (I2C_BUS_INSTANCE*)(((UINT8*)(UINTN)mI2cContext) + ((sizeof(I2C_BUS_INSTANCE) + DevPatLen) * Index));

    I2cBusInstance->Info.Cpuid                  = Eax;
    I2cBusInstance->Signature                   = I2C_PRIVATE_DATA_SIGNATURE;
    I2cBusInstance->Handle                      = NULL;
    I2cBusInstance->IntervalTime                = 2;
    I2cBusInstance->I2cBusSpeed                 = V_SPEED_FAST;
    I2cBusInstance->I2cHcRead8                  = I2cHcRead8;
    I2cBusInstance->I2cHcRead16                 = I2cHcRead16;
    I2cBusInstance->I2cHcRead32                 = I2cHcRead32;
    I2cBusInstance->I2cHcWrite8                 = I2cHcWrite8;
    I2cBusInstance->I2cHcWrite16                = I2cHcWrite16;
    I2cBusInstance->I2cHcWrite32                = I2cHcWrite32;

    I2cControllerCapabilities = AllocateZeroPool (sizeof(EFI_I2C_CONTROLLER_CAPABILITIES));
    I2cControllerCapabilities->MaximumReceiveBytes  = FIFO_SIZE_IN_BYTES;
    I2cControllerCapabilities->MaximumTransmitBytes = FIFO_SIZE_IN_BYTES;
    I2cControllerCapabilities->MaximumTotalBytes    = FIFO_SIZE_IN_BYTES;

    //
    //  Build the I2C Master protocol
    //
    I2cBusInstance->MasterApi.SetBusFrequency           = I2cPortSetBusFrequency;
    I2cBusInstance->MasterApi.Reset                     = I2cPortReset;
    I2cBusInstance->MasterApi.StartRequest              = I2cPortMasterStartRequest;
    I2cBusInstance->MasterApi.I2cControllerCapabilities = I2cControllerCapabilities;

    I2cBusInstance->DevicePath                          = (EFI_DEVICE_PATH_PROTOCOL*)(I2cBusInstance + 1);
    //
    //  Build the I2C Configuration Protocol
    //
    I2cBusInstance->ConfigApi.EnableI2cBusConfiguration = I2cBusConfiguration;
    //
    // Initialize the cache buffer
    //
    I2cBusInstance->CacheSize = DEFAULT_CACHE_SIZE;
    I2cBusInstance->Cache     = AllocateZeroPool(I2cBusInstance->CacheSize);

    GetChipInfo(I2cBusInstance, Index, &I2cHcType);

    switch (I2cHcType) {
    case I2C_HC_TYPE_PCI:
      mI2cPciHcDevPath.Pci.Device   = (UINT8)I2cBusInstance->PciI2c.Dev;
      mI2cPciHcDevPath.Pci.Function = (UINT8)I2cBusInstance->PciI2c.Func;
      CopyMem (I2cBusInstance->DevicePath, &mI2cPciHcDevPath, sizeof(I2C_PCI_HC_DEV_PATH));      
      break;
    case I2C_HC_TYPE_MMIO:      
      mI2cMemHcDevPath.Mmio.StartingAddress = I2cBusInstance->I2cBase;
      mI2cMemHcDevPath.Mmio.EndingAddress   = mI2cMemHcDevPath.Mmio.StartingAddress + I2cBusInstance->I2cBaseLen;
      mI2cMemHcDevPath.Mmio.MemoryType      = EfiMemoryMappedIO;
      CopyMem (I2cBusInstance->DevicePath, &mI2cMemHcDevPath, sizeof(I2C_MEM_HC_DEV_PATH));
      break;
    }
    //
    // Initialize the NotifyFunctionList
    //
    InitializeListHead (&I2cBusInstance->AsyncQueue);
    //
    // Install the I2C interface
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &I2cBusInstance->Handle,
                    &gEfiI2cMasterProtocolGuid,
                    &I2cBusInstance->MasterApi,
                    &gEfiI2cBusConfigurationManagementProtocolGuid,
                    &I2cBusInstance->ConfigApi,
                    &gEfiDevicePathProtocolGuid,
                    I2cBusInstance->DevicePath,                    
                    NULL
                    );
    ASSERT_EFI_ERROR(Status);
  }


  return EFI_SUCCESS;
}
