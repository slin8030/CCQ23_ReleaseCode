/** @file
  Dxe Bts library header file
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

#ifndef _DXE_BTS_LIBRARY_H_
#define _DXE_BTS_LIBRARY_H_


EFI_STATUS
EFIAPI
DxeBtsSendInfoLib (
  IN EFI_PHYSICAL_ADDRESS       ImageBase,
  IN UINT32                     ImageSize,
  IN CHAR8                      *PdbPointer,
  IN EFI_PHYSICAL_ADDRESS       ImageEntry
  );

/**
  Enable Bts controller.

**/
VOID
EFIAPI
DxeStartBtsLib (
  );

/**
  Disable Bts controller.

**/
VOID
EFIAPI
DxeStopBtsLib (
  );


#endif
