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

    LpcDriver.h

Abstract:

    EFI Lpc Driver for a Generic PC Platform


--*/

#ifndef _LPC_DRIVER_H
#define _LPC_DRIVER_H
//[-start-160216-IB03090424-modify]//

#include "LpcSio.h"
#include "LpcIsaAcpi.h"
#include "Protocol/IsaAcpi.h"
#include "Protocol/PciIo.h"
#include "Protocol/DriverBinding.h"
#include "Library/UefiBootServicesTableLib.h"
#include "IsaAcpiDxe/PcatIsaAcpi.h"
#include "IndustryStandard/Pci22.h"
#include "Protocol/LpcWpce791Policy.h"
#include <Library/DebugLib.h>
#include <Library/IoLib.h>


#define ICH_LPC_BRIDGE_BUS_DEV_FUNC 0x1F0000

//
// LPC device private data structure
//
#define LPC_DEV_SIGNATURE SIGNATURE_32('X', '7', '8', 'W') //'W87X'
#define EFI_WPCE791_PS2_KEYBOARD_ENABLE       0x01
#define EFI_WPCE791_PS2_KEYBOARD_DISABLE      0x00

#define EFI_WPCE791_PS2_MOUSE_ENABLE       0x01
#define EFI_WPCE791_PS2_MOUSE_DISABLE      0x00



typedef struct {
  UINTN                 Signature;
  EFI_HANDLE            Handle;
  EFI_ISA_ACPI_PROTOCOL IsaAcpi;
  EFI_PCI_IO_PROTOCOL   *PciIo;

} LPC_DEV;

#define LPC_ISA_ACPI_FROM_THIS(a) BASE_CR (a, LPC_DEV, IsaAcpi)

//
// Driver entry point
//
EFI_STATUS
EFIAPI
LpcDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes for Driver model protocol interface
//
EFI_STATUS
EFIAPI
LpcDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
LpcDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
LpcDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );
//[-end-160216-IB03090424-modify]//
#endif
