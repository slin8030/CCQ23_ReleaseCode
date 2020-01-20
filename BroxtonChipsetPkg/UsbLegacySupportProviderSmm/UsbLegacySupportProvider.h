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

#ifndef __EFI_USB_LEGACY_SUPPORT_PROVIDER_H__
#define __EFI_USB_LEGACY_SUPPORT_PROVIDER_H__

#include <Uefi.h>
#include <PiSmm.h>
#include <Guid/EventLegacyBios.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/UsbCore.h>
#include <Protocol/Usb3HostController.h>
#include <Protocol/UsbLegacySupportProviderProtocol.h>
#include <Protocol/SetupUtility.h>
#include <Protocol/SmmCpuSaveState.h>
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/AcpiEnableCallBackDone.h>
#include <ChipsetSetupConfig.h>
#include <ScAccess.h>
#include <SmiTable.h>
#include <ChipsetSmiTable.h>

#define IMAGE_DRIVER_NAME               L"Usb Legacy Support Provider Driver"
#define CONTROLLER_DRIVER_NAME          L""


#define USB_LEGACY_CONTROL_PROTOCOL_GUID \
 { 0x3084d2bd, 0xf589, 0x4be1, 0x8e, 0xf0, 0x26, 0xc6, 0xd6, 0x8a, 0x1b, 0xc8 }

//
// Driver Version
//
#define USB_LEGACY_SUPPORT_PROVIDER_VERSION     0x00000001
#define USB_LEGACY_SUPPORT_PROVIDER_VERSION_2   0x00000002
#define USB_LEGACY_SUPPORT_PROVIDER_VERSION_3   0x00000003

//
//
//
#define IRQ1                                            0x00
#define IRQ12                                           0x01

#define USB_LEGACY_CONTROL_SETUP_EMULATION              0
#define USB_LEGACY_CONTROL_GET_KBC_DATA_POINTER         1
#define USB_LEGACY_CONTROL_GET_KBC_STATUS_POINTER       2
#define USB_LEGACY_CONTROL_GENERATE_IRQ                 3

//
// Time constants, in 100 nano-second units
//
#define TIME_64s                                        640000000 /* 64   s  */
#define TIME_32s                                        320000000 /* 32   s  */ 
#define TIME_16s                                        160000000 /* 16   s  */ 
#define TIME_8s                                         80000000 /*  8   s  */ 
#define TIME_64ms                                       640000 /* 64   ms */
#define TIME_32ms                                       320000 /* 32   ms */
#define TIME_16ms                                       160000 /* 16   ms */
#define TIME_1_5ms                                      15000 /*  1.5 ms */

//
//
//
EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );
/**
  This is the declaration of an EFI image entry point. This entry point is
  the same for UEFI Applications, UEFI OS Loaders, and UEFI Drivers including
  both device drivers and bus drivers.

  @param  ImageHandle           The firmware allocated handle for the UEFI image.
  @param  SystemTable           A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
**/



#define INTERRUPT_LIST_SIGNATURE  SIGNATURE_32 ('i', 'n', 't', 's')

#pragma pack(1)

typedef struct {
  UINTN                           Signature;
  LIST_ENTRY                      Link;
  UINT8                           DevAddr;
  UINT8                           EndPoint;
  UINT8                           Reserved[5];
  UINTN                           Interval;
  EFI_ASYNC_USB_TRANSFER_CALLBACK InterruptCallBack;
  VOID                            *InterruptContext;
} INTERRUPT_LIST;

#pragma pack()

#define INTERRUPT_LIST_FROM_LINK(a) CR (a, INTERRUPT_LIST, Link, INTERRUPT_LIST_SIGNATURE)

//
// The body of UsbLegacyControlProtocol used for protocol installation
//
typedef struct {
  UINTN                                     Signature;
  UINTN                                     Revision;
  BOOLEAN                                   EmuEnabled;
  BOOLEAN                                   DriverBinded;
  EFI_DRIVER_BINDING_PROTOCOL               DriverBinding;
  EFI_USB3_HC_PROTOCOL                      Usb3Hc;
  EFI_USB_CORE_PROTOCOL                     *UsbCore;
  EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL  *PeriodicTimerDispatch;
  UINT32                                    TimerTicks;
  LIST_ENTRY                                AsyncIntList;
  USB_LEGACY_CONTROL                        UsbLegacyControl[2];
  
} USB_LEGACY_SUPPORT_PROVIDER_PRIVATE;

extern EFI_DRIVER_BINDING_PROTOCOL      gUsbLegacySupportProviderDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL      gUsbLegacySupportProviderComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL     gUsbLegacySupportProviderComponentName2;

#define USB_LEGACY_SUPPORT_PROVIDER_SIGNATURE             SIGNATURE_32 ('u', 's', 'b', 'p')
#define USB_LEGACY_SUPPORT_PROVIDER_FROM_DRIVER_BINDING(a) \
  BASE_CR (a, USB_LEGACY_SUPPORT_PROVIDER_PRIVATE, DriverBinding)

EFI_STATUS
EFIAPI
LegacyHidSupport (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  );

VOID
ReleaseInterruptList (
  IN USB_LEGACY_SUPPORT_PROVIDER_PRIVATE                *Private,
  IN UINT8                                              DevAddr,
  IN UINT8                                              EndPoint,
  IN UINTN                                              Interval
  );

//
// USB Hybrid Protocol
//
EFI_STATUS
EFIAPI
UsbHcGetCapability (
  IN  EFI_USB3_HC_PROTOCOL  *This,
  OUT UINT8                 *MaxSpeed,
  OUT UINT8                 *PortNumber,
  OUT UINT8                 *Is64BitCapable
  );

EFI_STATUS
EFIAPI
UsbHcGetState (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  OUT    EFI_USB_HC_STATE                               *State
  );

EFI_STATUS
EFIAPI
UsbHcInterruptTransfer (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINT8                                          DeviceAddress,
  IN     UINT8                                          EndPointAddress,
  IN     UINT8                                          DeviceSpeed,
  IN     UINTN                                          MaximumPacketLength,
  IN OUT VOID                                           *Data,
  IN OUT UINTN                                          *DataLength,
  IN OUT UINT8                                          *DataToggle,
  IN     UINTN                                          TimeOut,
  IN     EFI_USB3_HC_TRANSACTION_TRANSLATOR             *Translator,
  IN     EFI_ASYNC_USB_TRANSFER_CALLBACK                CallBackFunction OPTIONAL,
  IN     VOID                                           *Context         OPTIONAL,
  IN     UINTN                                          PollingInterval  OPTIONAL,
  OUT    UINT32                                         *TransferResult
  );

EFI_STATUS
EFIAPI
UsbHcCancelIo (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINT8                                          DeviceAddress,
  IN     UINT8                                          EndPointAddress,
  IN     UINTN                                          PollingInterval,
  IN OUT UINT8                                          *DataToggle
  );

EFI_STATUS
EFIAPI
UsbHcQueryIo (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINT8                                          DeviceAddress,
  IN     UINT8                                          EndPointAddress,
  IN     UINTN                                          PollingInterval
  );

EFI_STATUS
EFIAPI
UsbLegacyControl (
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  );


EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  );

EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer OPTIONAL
  );

#endif
