/** @file

Device Protocol

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
  ;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SioInitDxe.h"


/**
  To program extend function for GPIO device.
**/
VOID
ProgramGpio (
  IN SIO_IDENTIFY_DEVICE              *IdentifyDevice,
  IN SIO_DEVICE_LIST_TABLE            *DeviceResource,
  IN OUT SIO_NCT5104D_CONFIGURATION   *SioConfiguration
  )
{
  UINT8                       *GpioPcdExtensiveCfgPtr;
  SIO_EXTENSIVE_TABLE_TYPE17  *DeviceModeSetting;

  if (IdentifyDevice == NULL) {
    return;
  }

  GpioPcdExtensiveCfgPtr     = (UINT8*)IdentifyDevice + IdentifyDevice->Length;
  DeviceModeSetting = NULL;

  if ((*GpioPcdExtensiveCfgPtr == TYPE17) && (*(GpioPcdExtensiveCfgPtr + 2) == TRUE)) {
    DeviceModeSetting = (SIO_EXTENSIVE_TABLE_TYPE17*)GpioPcdExtensiveCfgPtr;
    SioConfiguration->Gpio[IdentifyDevice->DeviceInstance] = *((UINT8*)&(DeviceModeSetting->Setting));
  }
}


/**
  To progame gpio function for Super IO decice
**/
EFI_STATUS
SetupGpio (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT8                       *PcdExtensiveCfgPointer;
  UINTN                       BufferSize;
  SIO_NCT5104D_CONFIGURATION  *SioConfiguration;
  SIO_DEVICE_LIST_TABLE       *PcdPointer;
  UINT8                       DeviceHandle;
  SIO_EXTENSIVE_TABLE_TYPE17  *DeviceModeSetting;

  Status           = EFI_SUCCESS;
  PcdPointer       = mTablePtr;
  BufferSize       = sizeof (SIO_NCT5104D_CONFIGURATION);
  SioConfiguration = NULL;
  DeviceHandle     = 0;

  if (PCD_SIO_SETUP_SUPPORT) {
    SioConfiguration = AllocateZeroPool (sizeof (SIO_NCT5104D_CONFIGURATION));
    Status = gRT->GetVariable (
                    mSioVariableName,
                    &mSioFormSetGuid,
                    NULL,
                    &BufferSize,
                    SioConfiguration
                    );
    ASSERT_EFI_ERROR (Status);
  }

  while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
    PcdExtensiveCfgPointer = mExtensiveTablePtr;
    if ((PcdPointer->TypeInstance == mSioResourceFunction->Instance) && (PcdPointer->DeviceEnable == TRUE) && (PcdPointer->Device == SGPIO)) {
      while (DeviceHandle < MAX_SIO_GPIO) {
        FindExtensiveDevice (
        &PcdExtensiveCfgPointer,
        PcdPointer->TypeInstance,
        PcdPointer->Device,
        DeviceHandle
        );
        if (!PCD_SIO_ENABLE_GPIO_PEI) {
          ProgramGpioLib((SIO_IDENTIFY_DEVICE*)PcdExtensiveCfgPointer, PcdPointer, SioConfiguration, !mFirstBoot, mSioResourceFunction);
        } else if (mFirstBoot){
          ProgramGpio((SIO_IDENTIFY_DEVICE*)PcdExtensiveCfgPointer, PcdPointer, SioConfiguration);
        }
        if (PCD_SIO_ENABLE_GPIO_PEI && ((SioConfiguration->Gpio[DeviceHandle] >> 3) & 1)) {
          DeviceModeSetting = (SIO_EXTENSIVE_TABLE_TYPE17*)(PcdExtensiveCfgPointer + *(PcdExtensiveCfgPointer + 1));
          GpOutput ((UINT8*)&DeviceModeSetting->GpioNum, (UINT8)((SioConfiguration->Gpio[*(PcdExtensiveCfgPointer + 3)] >> 5) & 1), mSioResourceFunction);
        }
        DeviceHandle++;
      }
      break;
    }
    PcdPointer++;
  }
  return Status;
}

/**
  brief-description of function. 

  extended description of function.  
  
**/
EFI_STATUS
InstallGpioProtocol ( 
  IN SIO_DEVICE_LIST_TABLE* DeviceList
  )
{ 
  
  if (DeviceList->DeviceBaseAdr != 0) {
    IDW8 (SIO_LDN, DeviceList->DeviceLdn + 1, mSioResourceFunction);
    IDW8 (SIO_BASE_IO_ADDR1_MSB, (UINT8)(DeviceList->DeviceBaseAdr >> 8), mSioResourceFunction);
    IDW8 (SIO_BASE_IO_ADDR1_LSB, (UINT8)(DeviceList->DeviceBaseAdr & 0xFF), mSioResourceFunction);
    //
    // Active Io Base mode
    //
    IDW8 (SIO_CR30, (IDR8 (SIO_CR30, mSioResourceFunction) | 0x02), mSioResourceFunction);
  }

  //
  // Program gpio part
  //
  SetupGpio ();
  
  return EFI_SUCCESS;
}
