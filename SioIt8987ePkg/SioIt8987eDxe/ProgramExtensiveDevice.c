/** @file

According to Pcd$(SIO_Name)ExtensiveCfg to program extensive function device

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

VOID
ComFifoSetting (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  UINT8             FifoControlReg;
  SIO_FIFO_CONTEXT  *FifoContext;

  FifoContext    = (SIO_FIFO_CONTEXT*)Context;
  FifoControlReg = 0x07;

  if (FifoContext->FifoSetting->Enable) {
    if (mFirstBoot) {
      if (PCD_SIO_SETUP_SUPPORT && (FifoContext->FifoSetting->SetupDefault == 0)) {
        IoWrite8 (FifoContext->DeviceResource->DeviceBaseAdr + 2, 0);
      } else {
        FifoControlReg |= ((FifoContext->FifoSetting->Level % 4) << 6);
        IoWrite8 (FifoContext->DeviceResource->DeviceBaseAdr + 2, FifoControlReg);
      }
    } else {
      if (FifoContext->FifoVariable.Enable) {
        FifoControlReg |= ((FifoContext->FifoVariable.Level % 4) << 6);
        IoWrite8 (FifoContext->DeviceResource->DeviceBaseAdr + 2, FifoControlReg);
      } else {
        IoWrite8 (FifoContext->DeviceResource->DeviceBaseAdr + 2, 0);
      }
    }
  }

  //
  // Close event and Free memory of Context
  //
  FreePool (Context);
  gBS->CloseEvent (Event);
}

/**
  To program extend function for COM device.
**/
VOID
ProgramCom (
  IN SIO_IDENTIFY_DEVICE    *IdentifyDevice,
  IN SIO_DEVICE_LIST_TABLE  *DeviceResource,
  IN OUT SIO_CONFIGURATION  *SioConfiguration
  )
{
  UINT8                      Value;
  UINT8                      *ComPcdExtensiveCfgPtr;
  UINT8                      ComModeVariable;
  EFI_EVENT                  ReadyToBootEvent;
  EFI_STATUS                 Status;
  SIO_FIFO_CONTEXT           *FifoContext;
  SIO_DEVICE_MODE_SELECTION  *DeviceModeSelection;

  Value               = 0;
  ComPcdExtensiveCfgPtr        = (UINT8*)IdentifyDevice + IdentifyDevice->Length;
  ComModeVariable     = 0;
  DeviceModeSelection = NULL;
  FifoContext = AllocateZeroPool (sizeof (SIO_FIFO_CONTEXT));

  if (!mFirstBoot) {
    switch (IdentifyDevice->DeviceInstance) {
    case 0:
      ComModeVariable = SioConfiguration->Com1Mode;
      FifoContext->FifoVariable = SioConfiguration->Com1Fifo;
      break;
    case 1:
      ComModeVariable = SioConfiguration->Com2Mode;
      FifoContext->FifoVariable = SioConfiguration->Com2Fifo;
      break;
    default:
      break;
    }
  }

  while ((*ComPcdExtensiveCfgPtr != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)ComPcdExtensiveCfgPtr != EXTENSIVE_TABLE_ENDING)) {

    if (*ComPcdExtensiveCfgPtr == MODE_SELECTION_TYPE) {
      DeviceModeSelection = (SIO_DEVICE_MODE_SELECTION*)ComPcdExtensiveCfgPtr;
      if (DeviceModeSelection->Enable) {
        if (mFirstBoot) {
          switch (DeviceModeSelection->Setting) {
            case BIT0: // RS232
              break;
            case BIT1: // RS485
              break;
            case BIT2: // RS422
              break;
            default:
              break;
          }
        } else {
          switch (ComModeVariable) {
          case 0: // RS232
            break;
          case 1: // RS485
            break;
          case 2: // RS422
            break;
          default:
            break;
          }
        }
        IDW8 (SIO_CRF0, Value, mSioResourceFunction);
      }
    }

    if (*ComPcdExtensiveCfgPtr == FIFO_SETTING_TYPE) {
      FifoContext->FifoSetting = (SIO_DEVICE_FIFO_SETTING*)ComPcdExtensiveCfgPtr;
      if (FifoContext->FifoSetting->Enable) {
        FifoContext->DeviceResource = DeviceResource;
        //
        // Create a Ready to Boot Event for initialization of COM Fifo
        //
        Status = EfiCreateEventReadyToBootEx (
                   TPL_NOTIFY,
                   ComFifoSetting,
                   (VOID*)FifoContext,
                   &ReadyToBootEvent
                   );
        ASSERT_EFI_ERROR (Status);
      } else {
        FreePool (FifoContext);
      }
    }

    ComPcdExtensiveCfgPtr += *(ComPcdExtensiveCfgPtr + 1);
  }
}

/**
  To program extend function for Super IO device.
**/
VOID
ProgramExtensiveDevice (
  VOID
  )
{
  UINT8                  Different;
  UINT8                  *PcdExtensiveCfgPointer;
  UINTN                  BufferSize;
  EFI_STATUS             Status;
  SIO_CONFIGURATION      *SioConfiguration;
  SIO_DEVICE_LIST_TABLE  *PcdPointer;
  UINT8                  DeviceInstance;

  Status           = EFI_SUCCESS;
  PcdPointer       = mTablePtr;
  BufferSize       = sizeof (SIO_CONFIGURATION);
  SioConfiguration = NULL;

  if (PCD_SIO_SETUP_SUPPORT) {
    SioConfiguration = AllocateZeroPool (sizeof (SIO_CONFIGURATION));
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
    Different = 0;
    DeviceInstance = 0;
    PcdExtensiveCfgPointer = mExtensiveTablePtr;
    //
    // Check which device enable
    //
    if ((PcdPointer->TypeInstance == mSioResourceFunction->Instance) && (PcdPointer->DeviceEnable == TRUE)) {
      switch(PcdPointer->Device){
      case COM:
        Different = COM;
        break;
      }
    }

    if (Different != 0) {

        FindExtensiveDevice (
          &PcdExtensiveCfgPointer,
          PcdPointer->TypeInstance,
          PcdPointer->Device,
          PcdPointer->DeviceInstance
          );

        if (PcdExtensiveCfgPointer != NULL) {
          switch(Different){
          case COM:
            ProgramCom((SIO_IDENTIFY_DEVICE*)PcdExtensiveCfgPointer, PcdPointer, SioConfiguration);
            break;
        }
      }
    }
    PcdPointer++;
  }

  if (PCD_SIO_SETUP_SUPPORT) {
    FreePool (SioConfiguration);
  }
}

