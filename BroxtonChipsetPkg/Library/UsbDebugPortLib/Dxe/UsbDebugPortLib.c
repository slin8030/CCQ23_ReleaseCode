//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/** @file
  Usb Debug Port library instance

  Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <Base.h>

#include <PiPei.h>
#include "UsbDebugPortLibInternal.h"
#include <Library/SerialPortLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>
#include <Library/TimerLib.h>

STATIC USB2_SETUP_PACKET mSetAddress = {
  USB2_REQUEST_TYPE_HOST_TO_DEVICE | USB2_REQUEST_TYPE_STANDARD | USB2_REQUEST_TYPE_DEVICE,
  USB2_REQUEST_SET_ADDRESS,
  {USB2_DEBUG_PORT_DEFAULT_ADDRESS, 0x00},
  0x00,
  0x00
  };

STATIC USB2_SETUP_PACKET mGetDeviceDescriptor = {
  USB2_REQUEST_TYPE_DEVICE_TO_HOST | USB2_REQUEST_TYPE_STANDARD | USB2_REQUEST_TYPE_DEVICE,
  USB2_REQUEST_GET_DESCRIPTOR,
  {0x00, USB2_DESCRIPTOR_TYPE_DEVICE},
  0x0000,
  0x0008
  };

STATIC USB2_SETUP_PACKET mGetDebugDescriptor = {
  USB2_REQUEST_TYPE_DEVICE_TO_HOST | USB2_REQUEST_TYPE_STANDARD | USB2_REQUEST_TYPE_DEVICE,
  USB2_REQUEST_GET_DESCRIPTOR,
  {0x00, USB2_DESCRIPTOR_TYPE_DEBUG},
  0x0000,
  sizeof(USB2_DEBUG_DESCRIPTOR_TYPE)
  };

STATIC USB2_SETUP_PACKET mSetDebugFeature = {
  USB2_REQUEST_TYPE_HOST_TO_DEVICE | USB2_REQUEST_TYPE_STANDARD | USB2_REQUEST_TYPE_DEVICE, // 0x00,
  USB2_REQUEST_SET_FEATURE,                                                                 // 0x03,
  {USB2_FEATURE_DEBUG_MODE, 0x00},                                                          // 0x06, 0x00,
  0x0000,
  0x0000
  };

#ifdef NETCHIP_WORKAROUNDS
STATIC USB2_SETUP_PACKET mSetConfiguration1[8]   = {0x00, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif

/**
  Read data from serial device and save the datas in buffer.
  @param[in] *This              Usb2DebugPortInstance pointer
  @param[out] *Buffer           Buffer pointer to receive data.
  @param[out] *Length           Packet length
  @param[in] Timeout            Timeout value in microseconds
  @param[in] DataToggle         Data toggle
  
  @retval RETURN_SUCCESS        Data successfully received.
  @retval RETURN_DEVICE_ERROR   Device error encountered.
  @retval RETURN_TIMEOUT        Timeout occurred when waiting for data input.
**/
RETURN_STATUS
Usb2DebugPortIn (
  IN  USB2_DEBUG_PORT_INSTANCE        *This,
  OUT UINT8                           *Buffer,
  OUT UINTN                           *Length,
  IN  UINTN                           TimeOut,
  IN  UINT8                           DataToggle
  )
{
  UINTN             BufferIndex;
  UINTN             RetryCount = 3;  
  USB2_DEBUG_PORT_REGISTER *DebugRegister;

  *Length = 0;
  DebugRegister = (USB2_DEBUG_PORT_REGISTER *)(UINTN)This->DebugRegister;
  //
  // Setup PIDs
  //
  DebugRegister->TokenPid = USB2_PID_TOKEN_IN;
  if (DataToggle) {
    DebugRegister->SendPid = USB2_PID_DATA1;
  } else {
    DebugRegister->SendPid = USB2_PID_DATA0;
  }

  while (TimeOut && RetryCount) {
    ClrR32Bit((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_WRITE);
    SetR32Bit((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_GO);

    //
    // Wait for complete
    //
    while (!IsBitSet((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_DONE)) {
    }

    if (IsBitSet((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_ERROR)) {
      --RetryCount;
    }
    else if (DebugRegister->ReceivedPid == DebugRegister->SendPid) {
       *Length = DebugRegister->ControlStatus & USB2_DEBUG_PORT_STATUS_LENGTH;
       for (BufferIndex = 0; BufferIndex < *Length; ++BufferIndex) {
          Buffer[BufferIndex] = DebugRegister->DataBuffer[BufferIndex];
       }
       return RETURN_SUCCESS;
    }
    else MicroSecondDelay(1);
    --TimeOut;
  }

  if (RetryCount == 0) {
    return RETURN_DEVICE_ERROR;
  }

  return RETURN_TIMEOUT;
}

/**
  Write data from input buffer to serial device.

  @param[in] *This              Usb2DebugPortInstance pointer
  @param[in] *Buffer            Buffer pointer to receive data.
  @param[in] *Length            Packet length
  @param[in] Timeout            Timeout value in microseconds
  @param[in] Token              Token id
  @param[in] DataToggle         Data toggle
  
  @retval RETURN_SUCCESS        Data successfully received.
  @retval RETURN_DEVICE_ERROR   Device error encountered.
  @retval RETURN_TIMEOUT        Timeout occurred when waiting for data input.
**/
RETURN_STATUS
Usb2DebugPortOut (      
  IN  USB2_DEBUG_PORT_INSTANCE            *This,
  IN      UINT8                           *Buffer,
  IN      UINTN                           Length,
  IN      UINTN                           TimeOut,
  IN      UINT8                           Token,
  IN      UINT8                           DataToggle
  )
{
  UINTN             BufferIndex;
  UINTN             RetryCount = 3;  
  USB2_DEBUG_PORT_REGISTER *DebugRegister;

  DebugRegister = (USB2_DEBUG_PORT_REGISTER *)(UINTN)This->DebugRegister;
  DebugRegister->TokenPid = Token;
  if (DataToggle) {
    DebugRegister->SendPid = USB2_PID_DATA1;
  } else {
    DebugRegister->SendPid = USB2_PID_DATA0;
  }

  for (BufferIndex = 0; BufferIndex < Length; ++BufferIndex) {
    DebugRegister->DataBuffer[BufferIndex] = Buffer[BufferIndex];
  }

  //
  // Fill the data length
  //
  ClrR32Bit((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_LENGTH);
  SetR32Bit((UINTN)&DebugRegister->ControlStatus, (UINT32)Length);
  while (TimeOut && RetryCount) {
    SetR32Bit((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_WRITE);
    SetR32Bit((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_GO);

    //
    // Wait for complete
    //
    while (!IsBitSet((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_DONE)) {
    }

    if (IsBitSet((UINTN)&DebugRegister->ControlStatus, USB2_DEBUG_PORT_STATUS_ERROR)) {
      --RetryCount;
    } else {

		//
        // Debug
		//
		if (DebugRegister->ReceivedPid == USB2_PID_HANDSHAKE_ACK) {
			return RETURN_SUCCESS;
		} else if (DebugRegister->ReceivedPid == USB2_PID_HANDSHAKE_NYET) {
			return RETURN_SUCCESS;
		} else {
			MicroSecondDelay(1);
		}
    }
    --TimeOut;
  }

  if (RetryCount == 0) {
	return RETURN_DEVICE_ERROR;
  }

  return RETURN_TIMEOUT;
}


/**
  Usb2DebugPortControlTransfer internal function invoked by
  USB initialization phase.

  @param[in] *UsbDbg            Usb2DebugPortInstance pointer
  @param[in] *SetupPacket       USB2_SETUP_PACKET pointer.
  @param[out] *Data             Data pointer
  @param[out] DataToggle        Data length
  
  @retval RETURN_SUCCESS        Control transfer successful.
  @retval RETURN_UNSUPPORTED    Unsupported type of control transfer.
  @retval !RETURN_SUCCESS       Error
**/
RETURN_STATUS
Usb2DebugPortControlTransfer (
      IN  USB2_DEBUG_PORT_INSTANCE        *UsbDbg,
      IN  USB2_SETUP_PACKET               *SetupPacket,
      OUT UINT8                           *Data,
      OUT UINTN                           *DataLength
  )
{
  RETURN_STATUS          Status;

  if ((SetupPacket->Length > 0) && (!IsBitSet((UINTN)&SetupPacket->RequestType, USB2_REQUEST_TYPE_DEVICE_TO_HOST))) {
    return RETURN_UNSUPPORTED;
  }

  Status = Usb2DebugPortOut(UsbDbg, (UINT8 *)SetupPacket, sizeof(USB2_SETUP_PACKET), 1000000, USB2_PID_TOKEN_SETUP, 0);
  if (RETURN_ERROR(Status)) {
    return Status;
  }

  //
  // Data phase
  //
  if (SetupPacket->Length > 0) {
    if (IsBitSet((UINTN)&SetupPacket->RequestType, USB2_REQUEST_TYPE_DEVICE_TO_HOST)) {
      Status = Usb2DebugPortIn(UsbDbg, Data, DataLength, 1000000, 1);
      if (RETURN_ERROR(Status)) {
        return Status;
      }
    }
  }

  //
  // Status handshake
  //
  if (IsBitSet((UINTN)&SetupPacket->RequestType, USB2_REQUEST_TYPE_DEVICE_TO_HOST)) {
    Status = Usb2DebugPortOut(UsbDbg, NULL, 0, 1000000, USB2_PID_TOKEN_OUT, 1);
  } else {
    Status = Usb2DebugPortIn(UsbDbg, NULL, &Status, 1000000, 1);
  }

  return Status;
}

/**
  Method to send data over USB2 serial port.
  
  @param[in] *Instance		Usb2DebugPort instance pointer
  @param[in] *Data			Data pointer
  @param[in, out] *Length	Data length

  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.
**/
RETURN_STATUS
Usb2DebugPortSend (
  IN USB2_DEBUG_PORT_INSTANCE             *Instance,
  IN      UINT8                           *Data,
  IN  OUT UINTN                           *Length
  )
{
  RETURN_STATUS                            Status;
  UINTN                                 BytesToSend;
  USB2_DEBUG_PORT_REGISTER *DebugRegister;

  DebugRegister = (USB2_DEBUG_PORT_REGISTER *)(UINTN)Instance->DebugRegister;
  if (Instance->Ready == TRUE) {
     DebugRegister->UsbAddress = USB2_DEBUG_PORT_DEFAULT_ADDRESS;
     DebugRegister->UsbEndPoint = Instance->WriteEndpoint;

     BytesToSend = 0;
     while (*Length > 0) {
       BytesToSend = ((*Length) > 8)? 8 : *Length;
       Status = Usb2DebugPortOut(Instance, Data, BytesToSend, 1000000, (UINT8)USB2_PID_TOKEN_OUT, Instance->WriteEndpointDataToggle);
       if (RETURN_ERROR(Status)) {
         return Status;
       }
       Instance->WriteEndpointDataToggle ^= 0x01;

       *Length -= BytesToSend;
       Data += BytesToSend;
     }

     //
     // Send zero-length packet to end of this send session, if the length of data
     // is integer number of wMaxPacketSize (8 bytes for debug port)
     //
     Status = Usb2DebugPortOut(Instance, Data, 0, 1000000, (UINT8)USB2_PID_TOKEN_OUT, Instance->WriteEndpointDataToggle);
     if (!RETURN_ERROR(Status)) {
       Instance->WriteEndpointDataToggle ^= 0x01;
     }
  }

  return RETURN_SUCCESS;
}

/**
  Receive data over the USB2 serial device.
  
  This method calls Usb2DebugPortIn method internal to this library.

  @param[out] *Data			Pointer to data 
  @param[in out] *Length	Data length
  
  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.
**/
RETURN_STATUS
UsbDbgIn (
      OUT UINT8                           *Data,
  IN  OUT UINTN                           *Length
  )
{
  RETURN_STATUS                          Status = RETURN_SUCCESS;
  UINTN                                  BytesReceived;
  UINTN                                  AllBytesReceived;
  UINTN                                  Index;
  USB2_DEBUG_PORT_INSTANCE               *Instance;
  UINT32                                 EhciBaseAddress;
  UINT8                                  Bus;
  UINT8                                  Device;
  UINT8                                  Function;
  USB2_DEBUG_PORT_REGISTER               *DebugRegister;

  Bus = PcdGet8(PcdUsbSerialEhciBus);
  Device = PcdGet8(PcdUsbSerialEhciDev);
  Function = PcdGet8(PcdUsbSerialEhciFunc);
  EhciBaseAddress = PciRead32(PCI_LIB_ADDRESS(Bus, Device, Function, PCI_BASE_ADDRESSREG_OFFSET));

  Instance = (USB2_DEBUG_PORT_INSTANCE *)(UINTN)MmioRead32 (EhciBaseAddress + USB_DBG_DATA_OFFSET);
  DebugRegister = (USB2_DEBUG_PORT_REGISTER *)(UINTN)(EhciBaseAddress + Instance->BarOffset);

  if (Instance->Ready == TRUE) {
     DebugRegister->UsbAddress = USB2_DEBUG_PORT_DEFAULT_ADDRESS;
     DebugRegister->UsbEndPoint = Instance->ReadEndpoint;

     if (*Length <= 0 || Data == NULL) {
       *Length = 0;  // if incorrect buffer or length specified, read data into internal buffer
     }
     AllBytesReceived = 0;
     BytesReceived = 0;
     do {
        if (Instance->TempDataLength == 0 || *Length == 0) {
           if (Instance->TempDataLength == 0) {
              Instance->TempDataIndex = 0;
           }
           if  (*Length >= USB2_DEBUG_PORT_DEVICE_BUFFER_MAX) {
			  //
              // If there's enough space in buffer, try storing directly the data in user's buffer
			  //
              Status = Usb2DebugPortIn(Instance, Data, &BytesReceived, 1, Instance->ReadEndpointDataToggle);
              if (RETURN_ERROR(Status)) {
                 break;
              }
           } else if ((USB2_DEBUG_PORT_DRIVER_BUFFER_MAX - Instance->TempDataIndex - Instance->TempDataLength) >= USB2_DEBUG_PORT_DEVICE_BUFFER_MAX) {
			  //
              // Otherwise, we have to store received data in a temporary buffer first to avoid overflow
			  //
              Status = Usb2DebugPortIn(Instance, Instance->TempData + Instance->TempDataIndex, &BytesReceived, 1, Instance->ReadEndpointDataToggle);
              if (RETURN_ERROR(Status)) {
                 break;
              }
              Instance->TempDataLength += (UINT32)BytesReceived;
           } else {
              Status = RETURN_OUT_OF_RESOURCES;
              break;
           }
           Instance->ReadEndpointDataToggle ^= 0x01;
           if (BytesReceived == 0) {
			  //
              // Got a ZLP (Zero-Length Packet), stop receiving and send received data up
			  //
              break;
           }
        }

		//
        // Move the data from internal temporary buffer to user's buffer, if any
 	    //
        if (Instance->TempDataLength > 0) {
           BytesReceived = (*Length > (UINTN)Instance->TempDataLength)? Instance->TempDataLength : *Length;
           for (Index = 0; Index < BytesReceived; ++Index) {
              Data[Index] = Instance->TempData[Instance->TempDataIndex++];
              if (Instance->TempDataIndex >= USB2_DEBUG_PORT_DRIVER_BUFFER_MAX) {
                 Instance->TempDataIndex = 0;
              }
           }
           Instance->TempDataLength -= (UINT32)BytesReceived;
        }

        *Length -= BytesReceived;
        Data += BytesReceived;
        AllBytesReceived += BytesReceived;
     } while (*Length > 0);

     *Length = AllBytesReceived;
     if (*Length > 0) {
        MmioWrite32 ((EhciBaseAddress + USB_DBG_DATA_OFFSET), (UINT32)(UINTN)Instance);
        return RETURN_SUCCESS;
     }
  }
  
  MmioWrite32 ((EhciBaseAddress + USB_DBG_DATA_OFFSET), (UINT32)(UINTN)Instance);
  return Status;
}


/**
  Send data over USB2 serial device.
  
  This method calls Usb2DebugPortSend method internal to this library.

  @param[out] *Data		Pointer to data 
  @param[in]  *Length	Data length
  
  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.
**/
VOID
UsbDbgOut (
  OUT  UINT8                           *Data,
  IN   UINTN                           *Length
  )
{
  USB2_DEBUG_PORT_INSTANCE        *Instance;
  UINT32                          EhciBaseAddress;
  UINT8                           Bus;
  UINT8                           Device;
  UINT8                           Function;
  UINT16                          Command;

  Bus = PcdGet8(PcdUsbSerialEhciBus);
  Device = PcdGet8(PcdUsbSerialEhciDev);
  Function = PcdGet8(PcdUsbSerialEhciFunc);
  EhciBaseAddress = PciRead32(PCI_LIB_ADDRESS(Bus, Device, Function, PCI_BASE_ADDRESSREG_OFFSET));

  //
  //  Set MSE bit if the bit is disabled.
  //
  Command = PciRead16(PCI_LIB_ADDRESS(Bus, Device, Function, PCI_COMMAND_OFFSET));
  if ((Command & 0x2) == 0) {
    Command |= 0x02;
    PciWrite32(PCI_LIB_ADDRESS(Bus, Device, Function, PCI_COMMAND_OFFSET), Command);
  }
  Instance = (USB2_DEBUG_PORT_INSTANCE *)(UINTN)MmioRead32 (EhciBaseAddress + USB_DBG_DATA_OFFSET);
  Instance->DebugRegister = (EFI_PHYSICAL_ADDRESS)(UINTN)(EhciBaseAddress + Instance->BarOffset);

  Usb2DebugPortSend (Instance, Data,  Length);
  MmioWrite32 ((EhciBaseAddress + USB_DBG_DATA_OFFSET), (UINT32)(UINTN)Instance);
  
}  

/**
  Initialize USB2 serial port.
  
  This method invokes various internal functions to facilitate
  detection and initialization of USB2 serial port.

  @retval RETURN_SUCCESS        The serial device was initialized.
**/
RETURN_STATUS
EFIAPI
USBInitialize (
  VOID
  )
{
  //
  // This function is empty assuming the initialization would be done in PEI Phase.
  //
	return RETURN_SUCCESS;
}


