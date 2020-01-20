/** @file
  PCI Io Protocol

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
#include <Library/IoLib.h>
#include "Pci.h"


/**
 Read/Write PCI configuration space

 @param [in]   Write            TRUE for writes, FALSE for reads
 @param [in]   Width
 @param [in]   Address          The PCI address of the operation.
 @param [in, out] Buffer        The destination buffer to store the results.


**/
EFI_STATUS
RootBridgeIoPciRW (
  IN BOOLEAN                    Write,
  IN EFI_WIDTH                  Width,
  IN UINT32                     Address,
  IN OUT VOID                   *Buffer
  )
{
  UINTN                             PciData;

  PciData = 0xcfc + (Address & 0x03);
  Address &= ~0x03;
  IoWrite32(0xcf8, Address);
  if (Write)
  {
    switch (Width)
    {
      case EfiWidthUint8:
        IoWrite8((UINT16)PciData, *(UINT8*)Buffer);
        break;
      case EfiWidthUint16:
        IoWrite16((UINT16)PciData, *(UINT16*)Buffer);
        break;
      case EfiWidthUint32:
        IoWrite32((UINT16)PciData, *(UINT32*)Buffer);
        break;
    }
  }
  else
  {
    switch (Width)
    {
      case EfiWidthUint8:
        *(UINT8*)Buffer = IoRead8((UINT16)PciData);
        break;
      case EfiWidthUint16:
        *(UINT16*)Buffer = IoRead16((UINT16)PciData);
        break;
      case EfiWidthUint32:
        *(UINT32*)Buffer = IoRead32((UINT16)PciData);
        break;
    }
  }
  return EFI_SUCCESS;
}

/**
 Enables a PCI driver to read PCI controller registers in a
 PCI configuration space under a PCI Root Bridge.

 @param [in]   Width
 @param [in]   Address          The PCI address of the operation.
 @param [in, out] Buffer        The destination buffer to store the results.

 @retval EFI_SUCCESS            The data was read from the PCI root bridge.
 @retval EFI_INVALID_PARAMETER  Width is invalid for this PCI root bridge.
 @retval EFI_INVALID_PARAMETER  Buffer is NULL.
 @retval EFI_OUT_OF_RESOURCES   The request could not be completed due to a lack of resources.

**/
EFI_STATUS
PciRead (
  IN EFI_WIDTH                  Width,
  IN UINT32                     Address,
  IN OUT VOID                   *Buffer
  )
{
  return RootBridgeIoPciRW (
           FALSE,
           Width,
           Address,
           Buffer
           );
}

/**
 Enables a PCI driver to write to PCI controller registers in a
 PCI configuration space under a PCI Root Bridge.

 @param [in]   Width
 @param [in]   Address          The PCI address of the operation.
 @param [in, out] Buffer        The source buffer to get the data.

 @retval EFI_SUCCESS            The data was read from the PCI root bridge.
 @retval EFI_INVALID_PARAMETER  Width is invalid for this PCI root bridge.
 @retval EFI_INVALID_PARAMETER  Buffer is NULL.
 @retval EFI_OUT_OF_RESOURCES   The request could not be completed due to a lack of resources.

**/
EFI_STATUS
PciWrite (
  IN EFI_WIDTH                  Width,
  IN UINT32                     Address,
  IN OUT VOID                   *Buffer
  )
{
  return RootBridgeIoPciRW (
           TRUE,
           Width,
           Address,
           Buffer
           );
}
