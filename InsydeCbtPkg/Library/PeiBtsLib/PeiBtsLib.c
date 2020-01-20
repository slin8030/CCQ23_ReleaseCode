 /** @file
  Pei Bts Library
  
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
#include <PiPei.h>

#include <Ppi/BtsInitPei.h>

#include <Library/PeiServicesLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>


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
PeiBtsSendInfoLib (
  IN EFI_PHYSICAL_ADDRESS         ImageBase,
  IN UINT32                       ImageSize,
  IN CHAR8                        *TEImageHdr,
  IN EFI_PHYSICAL_ADDRESS         ImageEntry
  )
{
  EFI_STATUS                Status; 
  PEI_BTS_INIT_PPI          *BtsInitPpi;

  Status = EFI_UNSUPPORTED;
  Status = PeiServicesLocatePpi (&gPeiBtsInitPpiGuid, 0, NULL, (VOID **)&BtsInitPpi);
  if (Status == EFI_SUCCESS) {
    //
    //  Send image info and store 
    //
    Status = BtsInitPpi->SendInfo(BtsInitPpi, ImageBase, ImageSize, TEImageHdr, ImageEntry);
  }

  return Status;
}

/**
  Enable Bts controller.

**/
VOID
EFIAPI
PeiStartBtsLib (
  )
{
  EFI_STATUS                Status; 
  PEI_BTS_INIT_PPI          *BtsInitPpi;
  
  Status = PeiServicesLocatePpi (&gPeiBtsInitPpiGuid, 0, NULL, (VOID **)&BtsInitPpi);
  if (Status == EFI_SUCCESS) {
    //
    // Enable Bts controller.
    //
    BtsInitPpi->StartBts ();
  }

  return;
}

/**
  Disable Bts controller.

**/
VOID
EFIAPI
PeiStopBtsLib (
  )
{
  EFI_STATUS                Status; 
  PEI_BTS_INIT_PPI          *BtsInitPpi;
  
  Status = PeiServicesLocatePpi (&gPeiBtsInitPpiGuid, 0, NULL, (VOID **)&BtsInitPpi);
  if (Status == EFI_SUCCESS) {
    //
    // Disable Bts controller.
    //
    BtsInitPpi->StopBts ();
  } 

  return;
}

/**
  Bts Ds Area Addr.

  @retval     Bts Ds Area Addr.
**/
EFI_PHYSICAL_ADDRESS
EFIAPI
PeiBtsDsAreaAddrLib (
  )
{
  EFI_STATUS                Status; 
  PEI_BTS_INIT_PPI          *BtsInitPpi;
  EFI_PHYSICAL_ADDRESS      Value;

  Value = 0;
  Status = PeiServicesLocatePpi (&gPeiBtsInitPpiGuid, 0, NULL, (VOID **)&BtsInitPpi);
  if (Status == EFI_SUCCESS) {
    Value = BtsInitPpi->BtsDsAreaAddr;
  } 

  return Value;
}