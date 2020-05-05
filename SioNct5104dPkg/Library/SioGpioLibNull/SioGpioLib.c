/** @file
  SIO Library.

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
#include <Library/SioGpioLib.h>

EFI_STATUS
HandleListAdd (
  IN SIO_MAP_QUESTION_ID      *Container,
  IN UINT8                    GpHandle,
  IN EFI_QUESTION_ID          QuestionId
  )
{


  return EFI_SUCCESS;
}

EFI_STATUS
HandleListRemoveAll (
  IN SIO_MAP_QUESTION_ID       *Container
  )
{

  return EFI_SUCCESS;
}


UINT8
ConvertInst2Gp (
  IN  UINT8        DeviceInstance
  )
{
  UINT8               GpNumber;
  GpNumber            = 0;
 

  return GpNumber;
}

EFI_STATUS
SetNative (
  IN UINT8                         *GpNum,
  IN UINT8                         Native,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction 
  )
{
  EFI_STATUS        Status;

  return Status;
}

EFI_STATUS
GpOutput (
  IN UINT8                         *GpNum,
  IN UINT8                         OutVal,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction 
  )
{
  EFI_STATUS           Status;

  return Status;
}


EFI_STATUS
SetGpio (
  IN UINT8                         *GpNum,
  IN UINT8                         *Setting,
  IN EFI_SIO_RESOURCE_FUNCTION     *SioResourceFunction        
  )
{
  EFI_STATUS        Status;
 
  return Status;
} 

VOID
ProgramGpioLib (
  IN SIO_IDENTIFY_DEVICE              *DeviceMainType,
  IN SIO_DEVICE_LIST_TABLE            *TablePtr,
  IN OUT SIO_NCT5104D_CONFIGURATION   *SioConfiguration,
  IN BOOLEAN                          SioVariableExist,
  IN EFI_SIO_RESOURCE_FUNCTION        *SioResourceFunction
  ) 
{
}
