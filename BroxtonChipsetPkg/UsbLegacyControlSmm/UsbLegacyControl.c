/** @file
  Usb legacy keyboard/mouse control driver

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <UsbLegacyControl.h>

UINT32                          mHidOutputData = 0;
UINT32                          mHidStatusData = 0;
//[-start-160630-IB07400750-add]//
UINT16                          mSoftIrqGeneratePort;
//[-end-160630-IB07400750-add]//
EFI_GUID                        mUsbLegacyControlProtocolGuid = USB_LEGACY_CONTROL_PROTOCOL_GUID;

//
// The device path used for SMI registration. Use dev:0x1f fun:0x00 to it
//
struct {
  ACPI_HID_DEVICE_PATH          Acpi;
  PCI_DEVICE_PATH               Pci;
  EFI_DEVICE_PATH_PROTOCOL      End;
} mDevicePath = {
  { ACPI_DEVICE_PATH, ACPI_DP, (UINT8)(sizeof(ACPI_HID_DEVICE_PATH)), (UINT8)((sizeof(ACPI_HID_DEVICE_PATH)) >> 8), EISA_PNP_ID(0x0A03), 0 },
  { HARDWARE_DEVICE_PATH, HW_PCI_DP, (UINT8)(sizeof(PCI_DEVICE_PATH)), (UINT8)((sizeof(PCI_DEVICE_PATH)) >> 8), 0x00, 0x1f },
  { END_DEVICE_PATH_TYPE, END_ENTIRE_DEVICE_PATH_SUBTYPE, END_DEVICE_PATH_LENGTH, 0 }
};
//
// The body of UsbLegacyControlProtocol used for protocol installation
//
typedef struct {
  USB_LEGACY_CONTROL_PROTOCOL   UsbLegacyControlProtocol;
  BOOLEAN                       SmiHandlerInstalled;
} USB_LEGACY_CONTROL_PRIVATE;

USB_LEGACY_CONTROL_PRIVATE      mPrivate = {
  { UsbLegacyControl, NULL, NULL, FALSE },
  FALSE
};

EFI_SMM_SYSTEM_TABLE2                   *mSmst;


//[-start-160923-IB07400789-add]//
#define SW_SMI_OUTPUT_PORT                  0xB2
#define SW_SMI_DATA_PORT                    0xB3

#define LEGACY_CONTROLE_ENABLE_IO_TRAP      0x01
#define LEGACY_CONTROLE_DISABLE_IO_TRAP     0x02
#define LEGACY_CONTROLE_SETUP_SMI_HANDLER   0x03

EFI_STATUS
EFIAPI
UsbLegacyControlSwSmiCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
 )
{
  UINT8                   Cmd;
  UINT8                   *DataPtr;
  EFI_USB3_HC_PROTOCOL    *Usb3Hc;
  EFI_STATUS              Status;
  
  Cmd = IoRead8 (SW_SMI_DATA_PORT);

  if (Cmd == LEGACY_CONTROLE_ENABLE_IO_TRAP) { // Enable 0x60/0x64 IO Trap
    DataPtr = &Cmd;
    UsbLegacyControl (USB_LEGACY_CONTROL_SETUP_EMULATION, DataPtr);
  } else if (Cmd == LEGACY_CONTROLE_DISABLE_IO_TRAP) { // Disable 0x60/0x64 IO Trap
    DataPtr = NULL;
    UsbLegacyControl (USB_LEGACY_CONTROL_SETUP_EMULATION, DataPtr);
  } else if (Cmd == LEGACY_CONTROLE_SETUP_SMI_HANDLER) {
    //
    // Call from UsbCoreDxe.c, ExitBootServicesNotifyFunction to setup SmiHandler.
    //
    Status = gBS->LocateProtocol (&gEfiUsb3HcProtocolGuid, NULL, &Usb3Hc);
    if (!EFI_ERROR (Status)) {
      Usb3Hc->LegacyHidSupport(
                Usb3Hc,
                USB3_HC_HID_SUPPORT_SETUP_EMULATION,
                (VOID*)(UINTN)(0x00)
                );
    }
  }

  IoWrite8 (SW_SMI_DATA_PORT, 0x00);

  return EFI_SUCCESS;
}

//[-end-160923-IB07400789-add]//

/**

  Entry point for EFI drivers.

  @param  ImageHandle           EFI_HANDLE
  @param  SystemTable           EFI_SYSTEM_TABLE

  @retval EFI_SUCCESS           Success
  @retval Others                Fail

**/
EFI_STATUS
EFIAPI
UsbLegacyControlDriverEntryPoint (
  IN  EFI_HANDLE                                ImageHandle,
  IN  EFI_SYSTEM_TABLE                          *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            Handle;
  EFI_SMM_USB_DISPATCH2_PROTOCOL        *UsbDispatch;
  EFI_SMM_USB_REGISTER_CONTEXT          UsbContext;
  EFI_SMM_BASE2_PROTOCOL                *SmmBase;
  BOOLEAN                               InSmm;
//[-start-160630-IB07400750-add]//
  UINT32                                TcoBase;
//[-end-160630-IB07400750-add]//
//[-start-160923-IB07400789-add]//
  EFI_SMM_SW_DISPATCH2_PROTOCOL         *SwDispatch2;
  EFI_SMM_SW_REGISTER_CONTEXT           SwContext2;
//[-end-160923-IB07400789-add]//
  
  //
  // if port 64 is "0xff", it's legacyFree
  //

  if ( IoRead8 (0x64) != 0xFF) { 
    return EFI_UNSUPPORTED ;
  }  

  SmmBase = NULL;
  Status = gBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, (VOID **)&SmmBase);
  InSmm = FALSE;
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  }
  
  //
  // Initialize the EFI Runtime Library
  //
  if (InSmm){
    Status = SmmBase->GetSmstLocation (SmmBase, &mSmst);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    //
    // Register USB legacy SMI handler
    //
    Status = mSmst->SmmLocateProtocol (
					 &gEfiSmmUsbDispatch2ProtocolGuid,
					 NULL,
					 (VOID **)&UsbDispatch
					 );
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
    
    UsbContext.Type = UsbLegacy;
    UsbContext.Device = (EFI_DEVICE_PATH_PROTOCOL*)&mDevicePath;
    Status = UsbDispatch->Register(
                            UsbDispatch,
                            (EFI_SMM_HANDLER_ENTRY_POINT2)UsbLegacyControlSmiHandler,
                            &UsbContext,
                            &Handle
                            );
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
//[-start-160923-IB07400789-add]//
    //
    //  Get the Sw dispatch protocol
    //
    Status = mSmst->SmmLocateProtocol (
            &gEfiSmmSwDispatch2ProtocolGuid,
					  NULL,
					  (VOID**)&SwDispatch2
					  );
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
    
    Handle = NULL;
    SwContext2.SwSmiInputValue = USB_LEGACY_CONTROL_SW_SMI;
    Status = SwDispatch2->Register (
      SwDispatch2,
      UsbLegacyControlSwSmiCallback,
      &SwContext2,
      &Handle
      );
    
    if (EFI_ERROR(Status)) {
      return EFI_UNSUPPORTED;
    }
//[-end-160923-IB07400789-add]//
    //
    // This is SMM instance
    //
    mPrivate.UsbLegacyControlProtocol.InSmm = TRUE;
    }
  //
  // Install USB_LEGACY_CONTROL_PROTOCOL
  //
  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &mUsbLegacyControlProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPrivate.UsbLegacyControlProtocol
                  );
  //
  // Setup the Soft IRQ generate port
  //
//[-start-160630-IB07400750-add]//
  TcoBase = PciRead32 (SMBUS_TCO_BASE) & SMBUS_TCOBASE_BAR;
  mSoftIrqGeneratePort = (UINT16)(TcoBase + R_TCO_SW_IRQ_GEN);
//[-end-160630-IB07400750-add]//

  return Status;
}

/**

  Main routine for the USB legacy control

  @param  Command               USB_LEGACY_CONTROL_SETUP_EMULATION
                                USB_LEGACY_CONTROL_GET_KBC_DATA_POINTER
                                USB_LEGACY_CONTROL_GET_KBC_STATUS_POINTER
                                USB_LEGACY_CONTROL_GENERATE_IRQ  
  @param  Param                 The parameter for the command

  @retval EFI_SUCCESS           Success
  @retval Others                Fail

**/
EFI_STATUS
EFIAPI
UsbLegacyControl (
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  )
{
//[-start-160630-IB07400750-modify]//
  UINT8         Data8;
  UINT32        Data32;
//[-start-160923-IB07400789-add]//
  UINT32        XhciMmioBase;
  UINT32        LegacySupportCap;
  UINT8         CmosData;
//[-end-160923-IB07400789-add]//

//[-start-160923-IB07400789-add]//
  XhciMmioBase = PciRead32 (PCI_LIB_ADDRESS (XHCI_BUS, XHCI_DEV, XHCI_FUN, XHCI_MMIO_BASE)) & XHCI_MMIO_BASE_BAR;
  LegacySupportCap = MmioRead32 (XhciMmioBase + XHCI_LEGACY_SUPPORT_CAP);
//[-end-160923-IB07400789-add]//

  switch (Command) {
    case USB_LEGACY_CONTROL_SETUP_EMULATION:
//[-start-160923-IB07400789-add]//
      //
      // OS own XHCI, Disable Win7 Virtual KBC
      //
      if ((LegacySupportCap & XHCI_LEGACY_OS_OWNED) && (!Param)) { 
        CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
        CmosData &= (~B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT);
        WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
        break;
      }
//[-end-160923-IB07400789-add]//
      //
      // Used for emulation on/off
      //
      Data32 = PciRead32(LPC_PCI_LUKMC);
      if (!Param) {
        //
        // Turn off the trap SMI
        //
        Data32 &= ~(B_LUKMC_TRAP_60R | B_LUKMC_TRAP_60W | B_LUKMC_TRAP_64R | B_LUKMC_TRAP_64W);
      } else {
        //
        // Turn on the trap SMI
        //
        Data32 |= (B_LUKMC_TRAP_60R | B_LUKMC_TRAP_60W | B_LUKMC_TRAP_64R | B_LUKMC_TRAP_64W);
      }
      PciWrite32(LPC_PCI_LUKMC, Data32);
      //
      // Inspect the SMI handler data to see is it need to update
      //
      if (Param && !mPrivate.SmiHandlerInstalled && mPrivate.UsbLegacyControlProtocol.SmiHandler) {
        mPrivate.SmiHandlerInstalled = TRUE;
        //
        // Trap SMI if now in BS to pass the SMI handler related data to SMM instance
        //
        if (mPrivate.UsbLegacyControlProtocol.InSmm == FALSE) {
          //
          // Trigger SMI through 64 port read
          //
          IoRead8 (0x64);
        }
      }
      break;
    case USB_LEGACY_CONTROL_GET_KBC_DATA_POINTER:
      //
      // Get the "pointer" of KBC data (port 0x60)
      //
      *(UINT32**)Param = &mHidOutputData;
      break;
    case USB_LEGACY_CONTROL_GET_KBC_STATUS_POINTER:
      //
      // Get the "pointer" of KBC status (port 0x64)
      //
      *(UINT32**)Param = &mHidStatusData;
      break;
    case USB_LEGACY_CONTROL_GENERATE_IRQ:
      //
      // IRQ 1/12 generating
      //
      Data8 = IoRead8 (mSoftIrqGeneratePort);
      //
      // Make signal as low
      //
      Data8 &= ~(0x01 << ((mHidStatusData & 0x20) ? IRQ12 : IRQ1));
      IoWrite8 (mSoftIrqGeneratePort, Data8);
      //
      // Stall a while
      //
      Data8 = IoRead8 (0x61) & 0x10;
      while (Data8 == (IoRead8 (0x61) & 0x10));
      while (Data8 != (IoRead8 (0x61) & 0x10));
      //
      // make signal as high
      //
      IoWrite8 (mSoftIrqGeneratePort, 0x03);
      //
      // We don't need the output data right now
      //
      *(BOOLEAN*)Param = FALSE;
      break;
      
    case USB_LEGACY_CONTROL_GET_CAPABILITY:
      *(UINT32*)Param = 0;
      break;
      
    default:
      break;
  }
  return EFI_SUCCESS;
//[-end-160630-IB07400750-modify]//
}

/**

  The UsbLegacyControl SMI main handler

  @param  Handle
  @param  Context

  @retval VOID
**/
VOID
UsbLegacyControlSmiHandler (
	IN		  EFI_HANDLE								  Handle,
	IN	CONST EFI_SMM_USB_REGISTER_CONTEXT				  *Context
  )
{
  EFI_STATUS                    Status;
  UINT32                        Data32;
  UINT32                        TrapEvent;
  UINTN                         HandleCount;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         Index;
  USB_LEGACY_CONTROL_PRIVATE    *Private;
  EFI_TPL                       OldTpl;
  
//[-start-160630-IB07400750-modify]//
  TrapEvent = 0;
  Data32 = PciRead32(LPC_PCI_LUKMC);
  if (Data32 & B_LUKMC_TRAP_STATUS) {
    //
    // Clear the status
    //
    PciWrite32(LPC_PCI_LUKMC, Data32);
    TrapEvent = (Data32 & B_LUKMC_TRAP_STATUS) >> N_LUKMC_TRAP_STATUS;
  }
//[-end-160630-IB07400750-modify]//

  //
  // Call sub handler
  //
  if (mPrivate.UsbLegacyControlProtocol.SmiHandler) {
    mPrivate.UsbLegacyControlProtocol.SmiHandler (TrapEvent, mPrivate.UsbLegacyControlProtocol.SmiContext);
  }
  //
  // Check is the sub handler "SmiHandler" been installed. Must launched in POST
  //
  if (!mPrivate.SmiHandlerInstalled) {
    OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
    gBS->RestoreTPL (OldTpl);
    if (OldTpl == TPL_HIGH_LEVEL) {
      gBS->RestoreTPL (TPL_CALLBACK);
    }
    mPrivate.SmiHandlerInstalled = TRUE;
    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &mUsbLegacyControlProtocolGuid,
                    NULL,
                    &HandleCount,
                    &HandleBuffer
                    );
    if (!EFI_ERROR (Status)) {
      for (Index = 0; Index < HandleCount; Index++) {
        Status = gBS->HandleProtocol (
                        HandleBuffer[Index],
                        &mUsbLegacyControlProtocolGuid,
                        (VOID **)&Private
                        );
        if (!EFI_ERROR (Status) && Private->UsbLegacyControlProtocol.InSmm == FALSE) {
          //
          // Get the SMI handler related data from BS driver
          //
          mPrivate.UsbLegacyControlProtocol.SmiHandler = Private->UsbLegacyControlProtocol.SmiHandler;
          mPrivate.UsbLegacyControlProtocol.SmiContext = Private->UsbLegacyControlProtocol.SmiContext;
          break;
        }
      }
    }
    gBS->FreePool (HandleBuffer);
    if (OldTpl == TPL_HIGH_LEVEL) {
      gBS->RaiseTPL (TPL_HIGH_LEVEL);
    }
  }
}
