/*++

Copyright (c)  1999 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    LpcSio.h

Abstract:

    Lpc driver's sio interface

--*/

#ifndef _LPC_SIO_H
#define _LPC_SIO_H

#include "Protocol/PciRootBridgeIo.h"

#define VARSIOINSTALLED L"VarSIOProcotolInstalled"

//
// Port address
//
#define CONFIG_PORT               0x04E
#define INDEX_PORT                0x04E
#define DATA_PORT                 INDEX_PORT + 1

//
// Logical Device
//
#define SIO_COM                   0x3
#define SIO_MSWC                  0x4
#define SIO_MOUSE                 0x5
#define SIO_KEYBOARD              0x6
#define SIO_SHM                   0xF
#define SIO_PM1                   0x11
#define SIO_PM2                   0x12
#define SIO_PM3                   0x17
#define SIO_ESHM                  0x1D

//
// Global register 
//
#define REG_LOGICAL_DEVICE        0x07
#define REG_DEVICE_ID             0x20
#define SIO_CONFIG_1              0x21
#define REG_CHIP_REV              0x24
#define SIO_CONFIG_5              0x25
#define SIO_CONFIG_6              0x26
#define REG_DEVICE_REV            0x27
#define SIO_CONFIG_9              0x29
#define SIO_CONFIG_D              0x2D

#define ACTIVATE                  0x30
#define BASE_ADDRESS_HIGH         0x60
#define BASE_ADDRESS_LOW          0x61
#define BASE_ADDRESS_HIGH2        0x62
#define BASE_ADDRESS_LOW2         0x63
#define PRIMARY_INTERRUPT_SELECT  0x70
#define DMA_CHANNEL_SELECT        0x74

EFI_STATUS
InitializeLpcSio (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *RootBridgeIo
  );

//
// Prototypes for the sio internal function
//
//
// Internal function
//
VOID
LPCWPCE791SetDefault (
  VOID
  );

VOID
WriteRegisterAndSaveToScript (
  IN  UINT8   Index,
  IN  UINT8   Data
  );

VOID
FloppyWriteProtect (
  VOID
  );

VOID
DisableLogicalDevice (
  UINT8       DeviceId
  );

#endif
