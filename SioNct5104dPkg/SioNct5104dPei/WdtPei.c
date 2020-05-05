/** @file

Device Protocol

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SioInitPei.h"

EFI_STATUS
WriteOnOff (
  UINT16   IndexPort,
  UINT8    Ldn,
  BOOLEAN  OnOff
  )
{
  UINT8  Value;

  IoWrite8 (IndexPort, SIO_LDN);
  IoWrite8 (IndexPort + 1, Ldn);
  IoWrite8 (IndexPort, SIO_DEV_ACTIVE);
  Value = IoRead8 (IndexPort + 1);
  Value &= ~BIT0;
  Value |= OnOff;
  IoWrite8 (IndexPort + 1, Value);

  IoWrite8 (0x70, SIO_WDT_STATUS_CMOS_INDEX);
  if (OnOff == FALSE) {
    Value = (IoRead8 (0x71) & ~WDT_ENABLE_CMOS_STATUS_BIT);
  } else {
    Value = (IoRead8 (0x71) | WDT_ENABLE_CMOS_STATUS_BIT);
  }
  IoWrite8 (0x71, Value);

  return EFI_SUCCESS;
}

EFI_STATUS
WriteMode (
  UINT16  IndexPort,
  UINT8   Ldn,
  UINT8   Mode
  )
{
  UINT8  Value;

  IoWrite8 (IndexPort, SIO_LDN);
  IoWrite8 (IndexPort + 1, Ldn);
  IoWrite8 (IndexPort, SIO_WDT_CONTROL_MODE_REG);
  Value = IoRead8 (IndexPort + 1);

  switch (Mode) {
  case 0: // Second
    IoWrite8 (IndexPort + 1, (Value & ~BIT3) & ~BIT4);
    break;
  case 1: // Minute
    IoWrite8 (IndexPort + 1, (Value | BIT3) & ~BIT4);
    break;
  case 2: // Millisecond
    IoWrite8 (IndexPort + 1, (Value & ~BIT3) | BIT4);
    break;
  case 3: // Milli Minute
    IoWrite8 (IndexPort + 1, Value | BIT3 | BIT4);
    break;
  default:
    break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
WriteCounter (
  UINT16  IndexPort,
  UINT8   Ldn,
  UINT16  Counter
  )
{
  IoWrite8 (IndexPort, SIO_LDN);
  IoWrite8 (IndexPort + 1, Ldn);
  IoWrite8 (IndexPort, SIO_WDT_COUNTER_REG);
  IoWrite8 (IndexPort + 1, (UINT8)(Counter & 0xFF));

  return EFI_SUCCESS;
}

/**
  Follow SCU to set WDT

  @param[in]  FfsHeader        Pointer to FFS File Header
  @param[in]  UINT16           Index port of the SIO

  @retval     EFI_SUCCESS      The operation performed successfully.
**/
EFI_STATUS
SioWdt (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  UINT8                            Index;
  UINT8                            *PcdExtensiveCfgPointer;
  UINT8                            SioInstance;
  UINTN                            BufferSize;
  UINT16                           IndexPort;
  EFI_STATUS                       Status;
  SIO_NCT5104D_CONFIGURATION       SioConfiguration;
  SIO_WATCH_DOG_SETTING            *WatchDogSetting;
  SIO_DEVICE_LIST_TABLE            *WdtVariable;
  SIO_DEVICE_LIST_TABLE            *DeviceResource;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *PeiVariable;

  SioInstance = NULL_ID;
  IndexPort = CheckDevice (&SioInstance);
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
                             0,
                             NULL,
                             (VOID **)&PeiVariable
                             );
  ASSERT_EFI_ERROR (Status);

  BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);
  Status = PeiVariable->GetVariable (
                          PeiVariable,
                          mSioVariableName,
                          &gSioNct5104dSetup00FormSetGuid,
                          NULL,
                          &BufferSize,
                          &SioConfiguration
                          );

  DeviceResource = (SIO_DEVICE_LIST_TABLE*)PCD_SIO_CONFIG_TABLE;
  while ((DeviceResource->TypeH != NONE_ID) || (DeviceResource->TypeL != NONE_ID)) {
    if ((DeviceResource->Device == WDT) && (DeviceResource->TypeInstance == SioInstance)) {

      if (!DeviceResource->DeviceEnable) {
        WriteOnOff (IndexPort, SIO_WDT, FALSE);
        break;
      }

      if (EFI_ERROR (Status) || ((!EFI_ERROR (Status)) && (SioConfiguration.NotFirstBoot == 0))) {
        PcdExtensiveCfgPointer = (UINT8*)PCD_SIO_EXTENSIVE_CONFIG_TABLE;
        FindExtensiveDevice (
          &PcdExtensiveCfgPointer,
          SioInstance,
          DeviceResource->Device,
          DeviceResource->DeviceInstance
          );
        if (PcdExtensiveCfgPointer != NULL) {
          do {
            PcdExtensiveCfgPointer += *(PcdExtensiveCfgPointer + 1);
            if (*PcdExtensiveCfgPointer == WATCH_DOG_SETTING_TYPE) {
              WatchDogSetting = (SIO_WATCH_DOG_SETTING*)PcdExtensiveCfgPointer;
              WriteOnOff (IndexPort, SIO_WDT, TRUE);
              WriteMode (IndexPort, SIO_WDT, WatchDogSetting->UnitSelection);
              WriteCounter (IndexPort, SIO_WDT, WatchDogSetting->TimeOut);
              return EFI_SUCCESS;
            }
          } while ((*PcdExtensiveCfgPointer != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)PcdExtensiveCfgPointer != EXTENSIVE_TABLE_ENDING));
        }
      } else {
        WdtVariable = (SIO_DEVICE_LIST_TABLE*)(UINTN)(&SioConfiguration);
        for (Index = 0; Index < SioConfiguration.AutoUpdateNum; Index++) {
          if ((WdtVariable[Index].Device == WDT) && (WdtVariable[Index].DeviceInstance == DeviceResource->DeviceInstance)) {
            if (WdtVariable[Index].DeviceEnable == TRUE) {
              WriteOnOff (IndexPort, SIO_WDT, TRUE);
              WriteMode (IndexPort, SIO_WDT, SioConfiguration.WatchDogConfig.TimerCountMode);
              WriteCounter (IndexPort, SIO_WDT, SioConfiguration.WatchDogConfig.TimeOutValue);
            } else {
              WriteOnOff (IndexPort, SIO_WDT, FALSE);
            }
            return EFI_SUCCESS;
          }
        }
      }
    }
    DeviceResource++;
  }

  ExitConfigMode (IndexPort);

  return EFI_SUCCESS;
}

