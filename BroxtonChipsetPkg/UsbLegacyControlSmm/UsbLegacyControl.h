/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _USB_LEGACY_CONTROL_H
#define _USB_LEGACY_CONTROL_H

#include <Uefi.h>
#include <PiSmm.h>

#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>

#include <Protocol/SmmUsbDispatch2.h>
#include <Protocol/SmmBase2.h>
#include <ScAccess.h>

//[-start-160923-IB07400789-add]//
#include <Library/DebugLib.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/Usb3HostController.h>
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
#include <ChipsetSmiTable.h>
//[-end-160923-IB07400789-add]//

#define  USB_LEGACY_CONTROL_PROTOCOL_GUID \
 { 0x3084d2bd, 0xf589, 0x4be1, 0x8e, 0xf0, 0x26, 0xc6, 0xd6, 0x8a, 0x1b, 0xc8 }

#define  LPC_BUS                                        DEFAULT_PCI_BUS_NUMBER_SC
//[-start-160829-IB07400775-modify]//
#define  LPC_DEV                                        PCI_DEVICE_NUMBER_PCH_LPC
#define  LPC_FUN                                        PCI_FUNCTION_NUMBER_PCH_LPC
//[-end-160829-IB07400775-modify]//

//[-start-160630-IB07400750-modify]//
//#define  R_ACPI_PM_BASE                                 0x40 
//#define  ACPI_PM_BASE_MASK                              0xFFF8
//#define  R_LUKMC                                        R_ILB_ULKMC  
//#define  B_LUKMC_TRAP_60R                               B_ILB_ULKMC_60REN
//#define  B_LUKMC_TRAP_60W                               B_ILB_ULKMC_60WEN
//#define  B_LUKMC_TRAP_64R                               B_ILB_ULKMC_64REN
//#define  B_LUKMC_TRAP_64W                               B_ILB_ULKMC_64WEN
#define  R_LUKMC                                        0x94
#define  B_LUKMC_TRAP_60R                               0x0001
#define  B_LUKMC_TRAP_60W                               0x0002
#define  B_LUKMC_TRAP_64R                               0x0004
#define  B_LUKMC_TRAP_64W                               0x0008
//[-end-160630-IB07400750-modify]//
#define  B_LUKMC_TRAP_STATUS                            0x0f00
#define  N_LUKMC_TRAP_STATUS                            8     

//[-start-160630-IB07400750-add]//
#define  SMBUS_BUS                                      DEFAULT_PCI_BUS_NUMBER_SC
#define  SMBUS_DEV                                      PCI_DEVICE_NUMBER_SMBUS
#define  SMBUS_FUN                                      PCI_FUNCTION_NUMBER_SMBUS
#define  SMBUS_TCOBASE                                  R_SMBUS_TCOBASE
#define  SMBUS_TCOBASE_BAR                              B_SMBUS_TCOBASE_BAR
#define  R_TCO_SW_IRQ_GEN                               0x10

#define  SMBUS_TCO_BASE                                 PCI_LIB_ADDRESS (SMBUS_BUS, SMBUS_DEV, SMBUS_FUN, SMBUS_TCOBASE)
#define  LPC_PCI_LUKMC                                  PCI_LIB_ADDRESS (LPC_BUS, LPC_DEV, LPC_FUN, R_LUKMC)
//[-end-160630-IB07400750-add]//

#define  IRQ1                                           0x00
#define  IRQ12                                          0x01

#define  USB_LEGACY_CONTROL_SETUP_EMULATION             0
#define  USB_LEGACY_CONTROL_GET_KBC_DATA_POINTER        1
#define  USB_LEGACY_CONTROL_GET_KBC_STATUS_POINTER      2
#define  USB_LEGACY_CONTROL_GENERATE_IRQ                3
//[-start-160630-IB07400750-add]//
#define  USB_LEGACY_CONTROL_GET_CAPABILITY              4
//[-end-160630-IB07400750-add]//


//[-start-160928-IB07400789-add]//
#define  XHCI_BUS                                       DEFAULT_PCI_BUS_NUMBER_SC
#define  XHCI_DEV                                       PCI_DEVICE_NUMBER_XHCI
#define  XHCI_FUN                                       PCI_FUNCTION_NUMBER_XHCI
#define  XHCI_MMIO_BASE                                 R_XHCI_MEM_BASE
#define  XHCI_MMIO_BASE_BAR                             B_XHCI_MEM_BASE_BA

#define  XHCI_LEGACY_SUPPORT_CAP                        0x846C
#define  XHCI_LEGACY_OS_OWNED                           BIT24
//[-end-160928-IB07400789-add]//

typedef
VOID
(EFIAPI *USB_LEGACY_CONTROL_SMI_HANDLER) (
  IN     UINTN                                          Event,
  IN     VOID                                           *Context
  );

typedef
EFI_STATUS
(EFIAPI *USB_LEGACY_CONTROL) (
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  );

//[-start-160923-IB07400789-remove]//
//typedef struct {
//  USB_LEGACY_CONTROL                                    UsbLegacyControl;
//  USB_LEGACY_CONTROL_SMI_HANDLER                        SmiHandler;
//  VOID                                                  *SmiContext;
//  BOOLEAN                                               InSmm;
//} USB_LEGACY_CONTROL_PROTOCOL;
//[-end-160923-IB07400789-remove]//

EFI_STATUS
EFIAPI
UsbLegacyControl (
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  );

VOID
UsbLegacyControlSmiHandler (
  IN        EFI_HANDLE                                  Handle,
  IN  CONST EFI_SMM_USB_REGISTER_CONTEXT                *Context
  );

#endif
