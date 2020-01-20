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
  SIO_CONFIGURATION                SioConfiguration;
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

  BufferSize = sizeof (SIO_CONFIGURATION);
  Status = PeiVariable->GetVariable (
                          PeiVariable,
                          mSioVariableName,
                          &gSioIt8987eSetup00FormSetGuid,
                          NULL,
                          &BufferSize,
                          &SioConfiguration
                          );
  if (!EFI_ERROR (Status) && (SioConfiguration.NotFirstBoot == 1)) {
    SioVariableExist = TRUE;
  }

  ExitConfigMode (SioResourceFunction->SioCfgPort);

  return EFI_SUCCESS;
}

