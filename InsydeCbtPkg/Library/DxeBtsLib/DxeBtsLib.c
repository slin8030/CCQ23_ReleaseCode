 /** @file
  Dxe Bts Library
  
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
#include <PiDxe.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

#include <Protocol/BtsInitDxe.h>

EFI_BTS_INIT_PROTOCOL             *mBtsInitProtocol = NULL;

/**
  This constructor function caches the EFI_BTS_INIT_PROTOCOL pointer.

  @param[in] ImageHandle The firmware allocated handle for the EFI image.
  @param[in] SystemTable A pointer to the EFI System Table.

  @retval EFI_SUCCESS The constructor always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
DxeBtsConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                   Status = EFI_UNSUPPORTED;

  if (gBS == NULL) {
    return EFI_UNSUPPORTED;
  }
  
  Status = gBS->LocateProtocol (&gEfiBtsInitProtocolGuid, NULL, (VOID **)&mBtsInitProtocol);

  return EFI_SUCCESS;
  
}

/**

  Send image info and store 

  @param[IN]  ImageBase    Image Base address.
  @param[IN]  ImageSize    Size of the image
  @param[IN]  PdbPointer   Pdb file path point.
  @param[IN]  ImageEntry   Image Entry.

  @retval   EFI_SUCCESS:     Get image info and Pdb file path are success.
  @retval   EFI_NOT_FOUND:   Pdb file path can not find. 
**/
EFI_STATUS
EFIAPI
DxeBtsSendInfoLib (
  IN EFI_PHYSICAL_ADDRESS       ImageBase,
  IN UINT32                     ImageSize,
  IN CHAR8                      *PdbPointer,
  IN EFI_PHYSICAL_ADDRESS       ImageEntry
  )
{
  EFI_STATUS                        Status;

  Status = EFI_UNSUPPORTED;
  //
  // Locate gEfiBtsInitProtocolGuid Protocol interface.
  //
  if (mBtsInitProtocol == NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    // 
    Status = gBS->LocateProtocol (
                    &gEfiBtsInitProtocolGuid, 
                    NULL, 
                    (VOID **)&mBtsInitProtocol
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
  //
  // Send image info and store 
  //
  Status = mBtsInitProtocol->SendInfo (ImageBase, ImageSize, PdbPointer, ImageEntry);
  
  return Status;
}

/**
  Enable Bts controller.

**/
VOID
EFIAPI
DxeStartBtsLib (
  )
{
  EFI_STATUS                        Status;
  
  Status = EFI_UNSUPPORTED;
  //
  // Locate gEfiBtsInitProtocolGuid Protocol interface.
  //
  if (mBtsInitProtocol == NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    // 
    Status = gBS->LocateProtocol (
                    &gEfiBtsInitProtocolGuid, 
                    NULL, 
                    (VOID **)&mBtsInitProtocol
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Enable Bts controller.
  //
  mBtsInitProtocol->StartBts ();
  
  return;
}

/**
  Disable Bts controller.

**/
VOID
EFIAPI
DxeStopBtsLib (
  )
{
  EFI_STATUS                        Status;
  
  Status = EFI_UNSUPPORTED;
  //
  // Locate gEfiBtsInitProtocolGuid Protocol interface.
  //
  if (mBtsInitProtocol == NULL) {
    //
    // Locate Protocol again to aviod dependence problem.
    // 
    Status = gBS->LocateProtocol (
                    &gEfiBtsInitProtocolGuid, 
                    NULL, 
                    (VOID **)&mBtsInitProtocol
                    );
    if (EFI_ERROR (Status)) {
      return;
    }
  }

  //
  // Disable Bts controller.
  //
  mBtsInitProtocol->StopBts ();
  
  return;
}