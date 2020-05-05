/** @file

ProgramExtensiveDevice Pei

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/
#include "SioInitPei.h"

/**
  To program extend function for Super IO device. 
**/
EFI_STATUS
ProgramExtensiveDevicePei (
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN VOID                        *Ppi
  )
{
  UINT8                            DeviceHandle;
  UINT8                            *PcdExtensiveCfgPointer;
  EFI_STATUS                       Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *PeiVariable;
  UINTN                            BufferSize;
  SIO_NCT5104D_CONFIGURATION       SioConfiguration;
  BOOLEAN                          SioVariableExist;
  SIO_DEVICE_LIST_TABLE            *PcdPointer;
  EFI_SIO_RESOURCE_FUNCTION        SioResourceFunction[] = { NULL_ID,
                                                             SIO_ID1,
                                                             SIO_ID2,
                                                             NULL_ID,
                                                             FALSE
                                                           };
  PcdExtensiveCfgPointer = NULL;
  DeviceHandle = 0;

  SioResourceFunction->SioCfgPort = CheckDevice (&SioResourceFunction->Instance);

  if (SioResourceFunction->SioCfgPort == 0) {
    return EFI_UNSUPPORTED;
  }

  Status           = EFI_SUCCESS;
  PcdPointer       = (SIO_DEVICE_LIST_TABLE *)PCD_SIO_CONFIG_TABLE;
  PcdExtensiveCfgPointer    = (UINT8*)PCD_SIO_EXTENSIVE_CONFIG_TABLE;
  SioVariableExist = FALSE;

  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
                             0,
                             NULL,
                             (VOID **) &PeiVariable
                             );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);
  Status = PeiVariable->GetVariable (
                          PeiVariable,
                          mSioVariableName,
                          &gSioNct5104dSetup00FormSetGuid,
                          NULL,
                          &BufferSize,
                          &SioConfiguration
                          );
  if (!EFI_ERROR (Status) && (SioConfiguration.NotFirstBoot == 1)) {
    SioVariableExist = TRUE;
  }

  while (PCD_SIO_ENABLE_GPIO_PEI && !((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
    //
    // Check which device enable
    //
    if ((PcdPointer->Device == SGPIO) && (PcdPointer->DeviceEnable) && (PcdPointer->TypeInstance == SioResourceFunction->Instance)) {
      while (DeviceHandle < MAX_SIO_GPIO) {
        FindExtensiveDevice (
          &PcdExtensiveCfgPointer,
          PcdPointer->TypeInstance,
          PcdPointer->Device,
          DeviceHandle
          );
        ProgramGpioLib ((SIO_IDENTIFY_DEVICE*)PcdExtensiveCfgPointer, PcdPointer, &SioConfiguration, SioVariableExist, SioResourceFunction);
        DeviceHandle++;
      }
    }
    PcdPointer++;
  }

  ExitConfigMode (SioResourceFunction->SioCfgPort);

  return EFI_SUCCESS; 
}

/**
  To program extend function for Super IO device.
**/
EFI_STATUS
SioPort80ToUart (
  IN UINT16  IndexPort
  )
{
  UINT8                  Value;
  UINT16                 BaudRate;
  SIO_PORT80_TO_UART     *Port80ToUart;
  SIO_DEVICE_LIST_TABLE  *DeviceResource;

  Port80ToUart = NULL;

  DeviceResource = (SIO_DEVICE_LIST_TABLE*)PCD_SIO_CONFIG_TABLE;
  while ((DeviceResource->TypeH != NONE_ID) || (DeviceResource->TypeL != NONE_ID)) {
    if ((DeviceResource->Device == PORT80) && DeviceResource->DeviceEnable) {
      Port80ToUart = (SIO_PORT80_TO_UART*)PCD_SIO_PORT80_TO_UART;
      if (Port80ToUart->Type == PORT80_TO_UART_TYPE && Port80ToUart->Enable) {

//        //
//        // Set Global Oprion to select LPC/I2C resource to 80PORT
//        //
//        IoWrite8 (IndexPort, SIO_CR28);
//        Value = IoRead8 (IndexPort + 1) & 0xCF;
//        IoWrite8 (IndexPort + 1, Value);

        //
        // Output the port80 data to UART interface.
        //
        IoWrite8 (IndexPort, SIO_CR2F);
        Value = IoRead8 (IndexPort + 1) & 0xE7;
        if (Port80ToUart->SourceSelection & BIT2) {// Port80 to UartC
          Value |= BIT3;
        }
        if (Port80ToUart->SourceSelection & BIT4) {// Port80 to UartE
          Value |= BIT4;
        }
        IoWrite8 (IndexPort + 1, Value);

        //
        // Set I2C or LPC to 80PORT and enable 80PORT display.
        //
        IoWrite8 (IndexPort, SIO_CR07);
        IoWrite8 (IndexPort + 1, SIO_GPIO_PUSH);
        IoWrite8 (IndexPort, SIO_CRF1);
        if (Port80ToUart->SourceSelection & BIT6) { // LPC to Port80
          Value = IoRead8 (IndexPort + 1) & 0xFC;
        } else { // I2C to Port 80
          Value = (IoRead8 (IndexPort + 1) & 0xFC) | 0x01;
        }
        IoWrite8 (IndexPort + 1, Value);

        //
        // Set Port80 to UART BaudRate, default clock is 2MHz.
        //
        IoWrite8 (IndexPort, SIO_CR07);
        IoWrite8 (IndexPort + 1, SIO_PORT80);
        BaudRate = (UINT16)((2000000 / Port80ToUart->BaudRateSetting) - 1);

        IoWrite8 (IndexPort, SIO_CRE2); // BaudRate MSB
        IoWrite8 (IndexPort + 1, (UINT8)(BaudRate >> 8));

        IoWrite8 (IndexPort, SIO_CRE3); // BaudRate LSB
        IoWrite8 (IndexPort + 1, (UINT8)(BaudRate & 0x00FF));

        return EFI_SUCCESS;
      }
    }
    DeviceResource++;
  }

  return EFI_SUCCESS;
}

