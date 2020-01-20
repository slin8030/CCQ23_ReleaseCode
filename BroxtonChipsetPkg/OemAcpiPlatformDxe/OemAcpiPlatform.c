/** @file

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <OemAcpiPlatform.h>

EFI_ACPI_SUPPORT_PROTOCOL                 *mAcpiSupport = NULL;

/**

  OEM ACPI Platform driver installation function.

  @param     ImageHandle     EFI_HANDLE
  @param     SystemTable     EFI_SYSTEM_TABLE pointer

  @retval    EFI_SUCCESS    The driver installed without error.
             EFI_ABORTED    The driver encountered an error and could not complete installation of
                            the ACPI tables.

**/
EFI_STATUS
InstallOemAcpiPlatform (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                                Status;

  //
  // Locate ACPI support protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, (VOID **)&mAcpiSupport);
  ASSERT_EFI_ERROR (Status);
  Status = OemUpdateOemTableID ();
  if (EFI_ERROR(Status)) {

    DEBUG ( ( EFI_D_ERROR, "OEM Update OEM Table ID failed, Status : %r\n", Status ) );
  }
  return Status;
}
