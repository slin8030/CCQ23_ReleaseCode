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

#ifndef _PCI_H
#define _PCI_H

typedef enum {
  EfiWidthUint8,
  EfiWidthUint16,
  EfiWidthUint32,
  EfiWidthUint64,
  EfiWidthFifoUint8,
  EfiWidthFifoUint16,
  EfiWidthFifoUint32,
  EfiWidthFifoUint64,
  EfiWidthFillUint8,
  EfiWidthFillUint16,
  EfiWidthFillUint32,
  EfiWidthFillUint64,
  EfiWidthMaximum
} EFI_WIDTH;

#define EFI_PCI_ADDRESS(bus,dev,func,reg) \
  ((UINT32)(0x80000000 + (((UINT32)bus) << 16) + (((UINT32)dev) << 11) + (((UINT32)func) << 8) + ((UINT32)reg)))

EFI_STATUS
PciRead (
  IN EFI_WIDTH                  Width,
  IN UINT32                     Address,
  IN OUT VOID                   *Buffer
  );

EFI_STATUS
PciWrite (
  IN EFI_WIDTH                  Width,
  IN UINT32                     Address,
  IN OUT VOID                   *Buffer
  );

UINT8
EFIAPI
CpuIoRead8 (
  IN  UINT16  Port
  );

UINT16
EFIAPI
CpuIoRead16 (
  IN  UINT16  Port
  );

UINT32
EFIAPI
CpuIoRead32 (
  IN  UINT16  Port
  );

VOID
EFIAPI
CpuIoWrite8 (
  IN  UINT16  Port,
  IN  UINT8   Data
  );

VOID
EFIAPI
CpuIoWrite16 (
  IN  UINT16  Port,
  IN  UINT16  Data
  );

VOID
EFIAPI
CpuIoWrite32 (
  IN  UINT16  Port,
  IN  UINT32  Data
  );
  
#endif