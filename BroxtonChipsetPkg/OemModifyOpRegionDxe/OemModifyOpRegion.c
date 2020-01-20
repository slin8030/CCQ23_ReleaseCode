/** @file
  This driver will initial and update PrePostHotkey Variable.

;******************************************************************************
;* Copyright (c) 2012 - 2014 , Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <Library/BaseOemSvcChipsetLib.h>

STATIC
VOID
OemSetIgdOpRegion (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
{
  EFI_STATUS                    Status;
  IGD_OPREGION_PROTOCOL         *IgdOpRegion;

  Status = gBS->LocateProtocol (
                  &gIgdOpRegionProtocolGuid,
                  NULL,
                  (VOID **)&IgdOpRegion
                  );
  ASSERT (Status == EFI_SUCCESS);
  if (!EFI_ERROR (Status)) {
    Status = OemSvcSetIgdOpRegion(IgdOpRegion);
    DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcSetIgdOpRegion, Status : %r\n", Status ) );
  }
}

/**
  Modify Igd OpRegion

  @param     ImageHandle     EFI_HANDLE
  @param     SystemTable     EFI_SYSTEM_TABLE pointer

  @retval     EFI_SUCCESS     
**/
EFI_STATUS
EFIAPI
OemModifyOpRegionInit (
  IN EFI_HANDLE                 ImageHandle,
  IN EFI_SYSTEM_TABLE           *SystemTable
  )
{
  EFI_STATUS                    Status;
  EFI_EVENT                     IddOpRegionEvent;
  VOID                          *NotifyReg;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  OemSetIgdOpRegion,
                  NULL,
                  &IddOpRegionEvent
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->RegisterProtocolNotify (
                  &gIgdOpRegionProtocolGuid,
                  IddOpRegionEvent,
                  &NotifyReg
                  );
  return EFI_SUCCESS;
}


