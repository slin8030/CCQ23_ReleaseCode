/** @file
  PEIM for H2O Debug Mask of Insyde Event Log feature

;******************************************************************************
;* Copyright (c) 2016 - 2018, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "DebugMaskPei.h"

//
// module variables
//
EFI_PEI_NOTIFY_DESCRIPTOR mNotifyOnDebugMaskPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  DebugMaskPpiNotifyCallback 
};

/**
  Entry point of Debug Mask PEIM.

  @param  ImageHandle       The firmware allocated handle for the EFI image.
  @param  SystemTable       A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.

**/
EFI_STATUS
EFIAPI
DebugMaskPeiEntry (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                    Status;

  Status = (*PeiServices)->NotifyPpi (PeiServices, &mNotifyOnDebugMaskPpiList);
  return Status;
}

/**
  Notification function for Read Only Variable 2 Ppi.

  @param EFI_EVENT              Event of the notification
  @param Context                not used in this function

  @retval none

**/
EFI_STATUS
EFIAPI
DebugMaskPpiNotifyCallback (
  IN EFI_PEI_SERVICES               **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR      *NotifyDescriptor,
  IN VOID                           *Ppi
  )
{
  EFI_STATUS                                 Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI            *Variable;
  UINTN                                      Size;
  DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION    DebugMsgCfg;  
  UINT64                                     HobErrorLevel;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&Variable
             );
  if (!EFI_ERROR (Status)) {
    Size = sizeof (DEBUG_MESSAGE_CONFIG_UTIL_CONFIGURATION);
    Status = Variable->GetVariable ( 
                         Variable, 
                         H2O_DEBUG_MESSAGE_CONFIG_UTIL_VARSTORE_NAME,
                         &gH2ODebugMessageConfigUtilVarstoreGuid,
                         NULL,
                         &Size,
                         &DebugMsgCfg
                         );
    if (!EFI_ERROR (Status)) {
      //
      // Build the GUID'ed HOB for DXE
      //
//[-start-160218-IB08400332-modify]//
      HobErrorLevel = DebugCfgToErrLevel ((VOID*)&DebugMsgCfg);        
//[-end-160218-IB08400332-modify]//
      BuildGuidDataHob (
        &gH2ODebugMessageConfigUtilVarstoreGuid,
        &HobErrorLevel,
        sizeof (HobErrorLevel)
        );
      
      DEBUG ((EFI_D_ERROR|EFI_D_INFO|EFI_D_WARN, "HobErrorLevel : 0x%X\n", HobErrorLevel));
    }
  } 
  
  return Status;
}

