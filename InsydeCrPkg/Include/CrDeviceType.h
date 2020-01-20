/** @file
  Console Redirection Device type definition.

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
#ifndef _CR_DEVICE_TYPE_H_
#define _CR_DEVICE_TYPE_H_

#pragma pack (1)

typedef enum {
  UNKNOW_SERIAL_DEVICE = 0,
  ISA_SERIAL_DEVICE,
  PCI_SERIAL_DEVICE,
  USB_SERIAL_DEVICE,
  PCH_HS_SERIAL_DEVICE,
  PCI_HS_SERIAL_DEVICE,
  NET_SERIAL_DEVICE,
  INVALID_SERIAL_DEVICE
} CR_DEVICE_TYPE;

typedef struct {
  UINT8                   ComPortIrq;
  UINT16                  ComPortAddress;
} CR_OPROM_ISA_DEVICE;

typedef struct {
  UINT8                   Bus;
  UINT8                   Device;
  UINT8                   Function;
} CR_OPROM_PCI_DEVICE;

typedef struct {
  UINT8                   Bus;
  UINT8                   Device;
  UINT8                   Function;
  UINT8                   AccessWidth;
} CR_OPROM_PCI_HS_DEVICE;

typedef struct {
  UINT8                   AccessWidth;
  UINT32                  BaseAddr_Irq;  //IRQ:bit0-7  BaseAddress:bit8-bit31
} CR_OPROM_PCH_HS_DEVICE;

typedef union {
  CR_OPROM_ISA_DEVICE     IsaSerial;
  CR_OPROM_PCI_DEVICE     PciSerial;
  CR_OPROM_PCI_HS_DEVICE  PciHsSerial;
  CR_OPROM_PCH_HS_DEVICE  PchHsSerial;
} CR_LEGACY_SUPPORT_DEVICE;

#pragma pack()

#endif
