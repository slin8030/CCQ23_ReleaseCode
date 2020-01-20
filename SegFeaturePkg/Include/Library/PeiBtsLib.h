/** @file
  Pei Bts library header file
;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _PEI_BTS_LIBRARY_H_
#define _PEI_BTS_LIBRARY_H_


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
  );

/**
  Enable Bts controller.

**/
VOID
EFIAPI
PeiStartBtsLib (
  );

/**
  Disable Bts controller.

**/
VOID
EFIAPI
PeiStopBtsLib (
  );

/**
  Bts Ds Area Addr.

  @retval     Bts Ds Area Addr.
**/
EFI_PHYSICAL_ADDRESS
EFIAPI
PeiBtsDsAreaAddrLib (
  );

#endif
