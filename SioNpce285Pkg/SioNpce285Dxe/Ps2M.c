/** @file

Device protocol

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SioInitDxe.h"

static EFI_ISA_DEVICE_RESOURCE *DeviceResource;
static UINT32 Uid;  // Device Instance
static UINT8  Ldn;  // Logical device number

/**
  brief-description of function. 

  extended description of function.  
  
**/
static EFI_STATUS
SetDevicePower (
  IN BOOLEAN  OnOff
  )
{
  return EFI_UNSUPPORTED ;
}

/**
  brief-description of function. 

  extended description of function.  
  
**/
static EFI_STATUS
GetCurrentResource (
  OUT EFI_ISA_DEVICE_RESOURCE  **ResourceList
  )
{
  EnterConfigMode ();

  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
	*ResourceList = DeviceResource;
  (*ResourceList)->IoPort = (UINT16) (((IDR8(SIO_BASE_IO_ADDR1_MSB, mSioResourceFunction))<<8) + IDR8(SIO_BASE_IO_ADDR1_LSB, mSioResourceFunction));
  (*ResourceList)->IrqNumber = IDR8 (SIO_IRQ_SET, mSioResourceFunction);

  ExitConfigMode ();
  return EFI_SUCCESS;
}

/**
  brief-description of function. 

  extended description of function.  
  
**/
static EFI_STATUS
GetPossibleResource (
  OUT EFI_ISA_DEVICE_RESOURCE  **ResourceList,
  OUT UINT32                   *Hid
  )
{
  *ResourceList = DeviceResource;
  *Hid = Uid;
  return EFI_SUCCESS;
}

/**
  brief-description of function. 

  extended description of function.  
  
**/
static EFI_STATUS
SetResource (
  IN EFI_ISA_DEVICE_RESOURCE  *ResourceList
  )
{
  mSioResourceFunction->ScriptFlag = TRUE;
  EnterConfigMode ();
  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);  
  IDW8 (SIO_IRQ_SET, ResourceList->IrqNumber, mSioResourceFunction);
  ExitConfigMode ();
  mSioResourceFunction->ScriptFlag = FALSE;
  return EFI_SUCCESS;
}

/**
  brief-description of function. 

  extended description of function.  
  
**/
static EFI_STATUS
EnableDevice (
  IN BOOLEAN  Enable
  )
{  
  mSioResourceFunction->ScriptFlag = TRUE;
  EnterConfigMode ();
  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
  if (Enable) {
    IDW8 (SIO_DEV_ACTIVE, TRUE, mSioResourceFunction);
  } else {    
    IDW8 (SIO_DEV_ACTIVE, FALSE, mSioResourceFunction);
  }
  ExitConfigMode () ;
  mSioResourceFunction->ScriptFlag = FALSE;
  return EFI_SUCCESS;
}

static EFI_ISA_ACPI_DEVICE_FUNCTION mIsaFunction = {
   SetDevicePower,
   GetPossibleResource,
   SetResource,
   EnableDevice,
   GetCurrentResource,
};

/**
  brief-description of function. 

  extended description of function.  
  
**/
EFI_STATUS
InstallPs2MProtocol (
  IN SIO_DEVICE_LIST_TABLE* DeviceList
  )
{
  EFI_STATUS       Status = EFI_SUCCESS;
  EFI_HANDLE       DeviceHandle = NULL;

  //
  // Register ID for this device only
  //
  Uid = DeviceList->DeviceInstance;

  //
  // Get Ldn for this device
  //
  Ldn = DeviceList->DeviceLdn;

  //
  // DeviceList resource transfer to IsaDevice resource
  //
  DeviceResource = AllocatePool(sizeof(EFI_ISA_DEVICE_RESOURCE));

//[-start-140102-IB12970048-add]//
  if (DeviceResource == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
//[-end-140102-IB12970048-add]//
  
  DeviceListToDeviceResource(DeviceList, DeviceResource);

  Status = gBS->InstallProtocolInterface (
                  &DeviceHandle,
//[-start-170427-IB07400864-modify]//
                  &gEfiIsaPnpF03DeviceProtocolGuid,
//[-end-170427-IB07400864-modify]//
                  EFI_NATIVE_INTERFACE,
                  &mIsaFunction
                  );

  return Status;
}