/** @file
  Usb legacy support provider driver

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <UsbLegacySupportProvider.h>

//
// Usb driver load protocol
//
//
// Driver Binding Protocol instance
//
EFI_DRIVER_BINDING_PROTOCOL gUsbLegacySupportProviderDriverBinding = {
  UsbLegacySupportProviderDriverBindingSupported,
  UsbLegacySupportProviderDriverBindingStart,
  UsbLegacySupportProviderDriverBindingStop,
  USB_LEGACY_SUPPORT_PROVIDER_VERSION,
  NULL,
  NULL
};

//
//
//
UINT32    mHidOutputData          = 0;
UINT32    mHidStatusData          = 0;
EFI_GUID  mEfiUsbCoreProtocolGuid = EFI_USB_CORE_PROTOCOL_GUID;


EFI_USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL  mUsbLegacySupportProtocol;
USB_LEGACY_SUPPORT_PROVIDER_PRIVATE       *mPrivate = NULL;

EFI_GUID                                  mUsbLegacyControlProtocolGuid = USB_LEGACY_CONTROL_PROTOCOL_GUID;
EFI_HANDLE                                mPeriodicHandle  = NULL;

EFI_STATUS
EFIAPI
PeriodicTimerHandOffCallbackFunction (
  IN CONST EFI_GUID                       *Protocol,
  IN VOID                                 *Interface,
  IN EFI_HANDLE                           Handle
  )
{
  INTERRUPT_LIST                      *PtrList;
  LIST_ENTRY                          *Link;
  EFI_STATUS                          Status      = EFI_SUCCESS;
  EFI_USB_CORE_PROTOCOL               *UsbCore    = NULL;
  EFI_USB3_HC_PROTOCOL                *Usb3Hc     = NULL;
  
  //
  // Process all callback before hand-off
  //
  Link = mPrivate->AsyncIntList.ForwardLink;
  do {
    PtrList = INTERRUPT_LIST_FROM_LINK (Link);
    Link    = Link->ForwardLink;

    if ((PtrList->InterruptCallBack != NULL) ) {
      (PtrList->InterruptCallBack) (
                  NULL,
                  0,
                  PtrList->InterruptContext,
                  0
                  );
    }
  } while (Link != &mPrivate->AsyncIntList);


  UsbCore = mPrivate->UsbCore;
  
  //
  // Remove PeriodicTimerProvider
  //
  UsbCore->RemovePeriodicTimerProvider (&mPrivate->Usb3Hc);
  DEBUG ((EFI_D_ERROR, "Remove Periodic Timer Provider (%r)\n", Status));

  //
  // Remove LegacySupportProvider
  //
  Usb3Hc = NULL;
  Status = UsbCore->GetLegacySupportProvider (&Usb3Hc);
  if (!EFI_ERROR (Status)) {
    Status = UsbCore->RemoveLegacySupportProvider (Usb3Hc);
    DEBUG ((EFI_D_ERROR, "Remove Legacy Support Provider (%r)\n", Status));
    if (!EFI_ERROR (Status)) {
      mPrivate->EmuEnabled                        = FALSE;
      mPrivate->DriverBinded                      = FALSE;
    }
  }
  
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.LegacyHidSupport
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.GetCapability
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.GetState
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.InterruptTransfer
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.CancelIo
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.QueryIo
            );

  //
  // UnRegister Periodic SMI Timer
  //
  if (mPrivate->PeriodicTimerDispatch != NULL) {
    Status = mPrivate->PeriodicTimerDispatch->UnRegister (
      mPrivate->PeriodicTimerDispatch,
      mPeriodicHandle
      );
    mPrivate->PeriodicTimerDispatch = NULL;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LegacySupportSwSmiCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
  )
{
  INTERRUPT_LIST                      *PtrList;
  LIST_ENTRY                          *Link;

  Link = mPrivate->AsyncIntList.ForwardLink;
  do {
    PtrList = INTERRUPT_LIST_FROM_LINK (Link);
    Link    = Link->ForwardLink;

    if ((PtrList->InterruptCallBack != NULL) ) {
      (PtrList->InterruptCallBack) (
                  NULL,
                  0,
                  PtrList->InterruptContext,
                  0
                  );
    }
  } while (Link != &mPrivate->AsyncIntList);
  return EFI_SUCCESS;
}
EFI_STATUS
PeriodicTimerCallbackFunction (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
  )
{
  INTERRUPT_LIST                      *PtrList;
  LIST_ENTRY                          *Link;
#ifdef USB_LEGACY_PROVIDER_DEBUG //Debug  
  EFI_SMM_CPU_PROTOCOL                *SmmCpu = NULL;
  UINTN                               CpuIndex;
  STATIC EFI_SMM_CPU_STATE            CpuSaveState;
  UINTN                               Width;
  EFI_SMM_SAVE_STATE_REGISTER         Register;
  UINT32                              RegData32;
#endif
  EFI_SMM_PERIODIC_TIMER_CONTEXT        *TimerContext = NULL;

  // reset the periodic timer dispatch context
  if (CommBuffer != NULL) {
    TimerContext = (EFI_SMM_PERIODIC_TIMER_CONTEXT *)CommBuffer;
  }  
  if (TimerContext->ElapsedTime != 0) {
    TimerContext->ElapsedTime ^= TimerContext->ElapsedTime;
  }
  
  // return this function if there are no callback functions be registered
  if ((mPrivate == NULL) || (IsListEmpty (&mPrivate->AsyncIntList))) {
    return EFI_SUCCESS;
  }

#ifdef USB_LEGACY_PROVIDER_DEBUG // Debug
  gSmst->SmmLocateProtocol (&gEfiSmmCpuProtocolGuid, NULL, (VOID**) &SmmCpu);
  
  //
  // Search all CPU
  //
  for (CpuIndex = 0; CpuIndex < gSmst->NumberOfCpus; CpuIndex++) {
    CopyMem (&CpuSaveState, gSmst->CpuSaveState[CpuIndex], sizeof (EFI_SMM_CPU_STATE));
    
    Width = sizeof (UINT16);
    Register = EFI_SMM_SAVE_STATE_REGISTER_CS;
    SmmCpu->ReadSaveState (
               SmmCpu,
               Width,
               Register,
               CpuIndex,
#ifdef MDE_CPU_IA32
               &CpuSaveState.x86._CS
#else
               &CpuSaveState.x64._CS
#endif
               );
    
#ifdef MDE_CPU_IA32
    Width = sizeof (UINT32);
#else
    Width = sizeof (UINT64);
#endif
    Register = EFI_SMM_SAVE_STATE_REGISTER_RIP;
    SmmCpu->ReadSaveState (
               SmmCpu,
               Width,
               Register,
               CpuIndex,
#ifdef MDE_CPU_IA32
               &CpuSaveState.x86._EIP
#else
               &CpuSaveState.x64._RIP
#endif
               );

#ifdef MDE_CPU_IA32
    DEBUG ((EFI_D_ERROR, "CpuIndex = %x, CpuSaveState.x86._EIP = %x\n", CpuIndex, CpuSaveState.x86._EIP));
#else
  if (CpuSaveState.x64._CS != 0x38) {
    DEBUG ((EFI_D_ERROR, "CpuIndex = %x, ", CpuIndex));
    DEBUG ((EFI_D_ERROR, "CS:RIP = 0x%04x:0x%016lx\n", CpuSaveState.x64._CS, CpuSaveState.x64._RIP));
  }
#endif
  }
#endif  

  Link = mPrivate->AsyncIntList.ForwardLink;
  do {
    PtrList = INTERRUPT_LIST_FROM_LINK (Link);
    Link    = Link->ForwardLink;

    //
    // add for real platform debug
    //
    if (
      (PtrList->InterruptCallBack != NULL) &&
      (mPrivate->TimerTicks != 0) &&
      ((PtrList->Interval / 32) % mPrivate->TimerTicks == 0)
      ) {
      (PtrList->InterruptCallBack) (
                  NULL,
                  0,
                  PtrList->InterruptContext,
                  0
                  );
    }
  } while (Link != &mPrivate->AsyncIntList);

  mPrivate->TimerTicks ++;

  return EFI_SUCCESS;
}


EFI_STATUS 
EFIAPI
UsbLegacySupportProviderUnload (
  IN EFI_HANDLE  ImageHandle
  )
/**
  Unloads an image.

  @param  ImageHandle           Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
  @retval EFI_INVALID_PARAMETER ImageHandle is not a valid image handle.

**/
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleCount;
  UINTN       Index;

  Status = EFI_SUCCESS;
  //
  // Retrieve array of all handles in the handle database
  //
  Status = gBS->LocateHandleBuffer (
                  AllHandles,
                  NULL,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Disconnect the current driver from handles in the handle database 
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->DisconnectController (HandleBuffer[Index], gImageHandle, NULL);
  }

  //
  // Free the array of handles
  //
  gBS->FreePool (HandleBuffer);

  //
  // Uninstall driver binding protocols
  //
  gBS->UninstallProtocolInterface (
         gUsbLegacySupportProviderDriverBinding.DriverBindingHandle,
         &gEfiDriverBindingProtocolGuid,
         &gUsbLegacySupportProviderDriverBinding
         );
  gBS->UninstallProtocolInterface (
         gUsbLegacySupportProviderDriverBinding.DriverBindingHandle,
         &gEfiComponentName2ProtocolGuid,
         &gUsbLegacySupportProviderComponentName2
         );
  gBS->UninstallProtocolInterface (
         gUsbLegacySupportProviderDriverBinding.DriverBindingHandle,
         &gEfiComponentNameProtocolGuid,
         &gUsbLegacySupportProviderComponentName
         );


  mPrivate->UsbCore->RemoveAddressConvertTable (
                    ACT_INSTANCE_BODY,
                    mPrivate
                    );
  mPrivate->UsbCore->RemoveAddressConvertTable (
                    ACT_INSTANCE_POINTER,
                    &mPrivate
                    );
  mPrivate->UsbCore->RemoveAddressConvertTable (
                    ACT_FUNCTION_POINTER,
                    &mPrivate->DriverBinding
                    );
  mPrivate->UsbCore->RemoveAddressConvertTable (
                    ACT_INSTANCE_POINTER,
                    &mPrivate->UsbCore
                    );
  mPrivate->UsbCore->RemoveAddressConvertTable (
                    ACT_LINKING_LIST,
                    &mPrivate->AsyncIntList
                    );

  // Release resources
  if ((mPrivate != NULL) && (mPrivate->UsbCore != NULL)) {
    mPrivate->UsbCore->FreeBuffer (sizeof (USB_LEGACY_SUPPORT_PROVIDER_PRIVATE), mPrivate);
  }

  //
  // Do any additional cleanup that is required for this driver
  //

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/**
  This is the declaration of an EFI image entry point. This entry point is
  the same for UEFI Applications, UEFI OS Loaders, and UEFI Drivers including
  both device drivers and bus drivers.

  @param  ImageHandle           The firmware allocated handle for the UEFI image.
  @param  SystemTable           A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval Others                An unexpected error occurred.
**/
{
  EFI_STATUS                                Status          = EFI_SUCCESS;
  BOOLEAN                                   InSmm           = FALSE;
  EFI_USB_CORE_PROTOCOL                     *UsbCore        = NULL;
  EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT   PeriodicContext = {0};
  EFI_SETUP_UTILITY_PROTOCOL                *EfiSetupUtility = NULL;
  CHIPSET_CONFIGURATION                     *SystemConfiguration = NULL;
  EFI_HANDLE                                UsbLegacySupportHandle = NULL;
  EFI_USB_LEGACY_SUPPORT_PROVIDER_PROTOCOL  *UsbLegacySupportProtocol = NULL;
  EFI_HANDLE                                Handle = NULL;   
  EFI_SMM_SW_DISPATCH2_PROTOCOL              *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT               SwContext;
  EFI_EVENT                                 AcpiEnableCallbackStartEvent;
//[-start-160421-IB08450342-remove]//
  //UINTN                                     Size;
  //UINT8                                     BootType;
//[-end-160421-IB08450342-remove]//

  Status = gBS->LocateProtocol (&gEfiSetupUtilityProtocolGuid, NULL, (VOID **)&EfiSetupUtility);
  if (EFI_ERROR (Status)) {
    return EFI_PROTOCOL_ERROR;
  }

  SystemConfiguration = (CHIPSET_CONFIGURATION *)EfiSetupUtility->SetupNvData;

//[-start-160421-IB08450342-remove]//
  //BootType = DUAL_BOOT_TYPE;
  //Size = sizeof (UINT8);
  //Status = gRT->GetVariable (
  //                L"BootType",
  //                &gSystemConfigurationGuid,
  //                NULL,
  //                &Size,
  //                &BootType
  //                );
  //if (EFI_ERROR (Status)) {
  //  return EFI_PROTOCOL_ERROR;
  //}
//[-end-160421-IB08450342-remove]//

  //
  // Only Use in non-UEFI mode and XHCI enabled
  //
//[-start-160421-IB08450342-modify]//
  if (!((SystemConfiguration->BootType != EFI_BOOT_TYPE) && (SystemConfiguration->UsbXhciSupport == TRUE))) {
    return EFI_UNSUPPORTED;
  }
//[-end-160421-IB08450342-modify]//

  //
  // Locate EFI_USB_CORE_PROTOCOL for further use
  //
  Status = gBS->LocateProtocol (
                  &gEfiUsbCoreProtocolGuid,
                  NULL,
                  (VOID **)&UsbCore
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Locate UsbCore protocol (%r)\n", Status));
    return Status;
  }

  //
  // Check the phase of instance
  //
  UsbCore->IsInSmm (&InSmm);
  if (!InSmm) {
    //
    // Pre-allocate private storage
    //
    Status = UsbCore->AllocateBuffer (
      sizeof (USB_LEGACY_SUPPORT_PROVIDER_PRIVATE),
      ALIGNMENT_32,
      (VOID **) &mPrivate
      );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }

    //
    // Initialize Private data
    //
    mPrivate->Signature                           = USB_LEGACY_SUPPORT_PROVIDER_SIGNATURE;
    mPrivate->Revision                            = USB_LEGACY_SUPPORT_PROVIDER_VERSION_3;
    mPrivate->EmuEnabled                          = FALSE;
    // Set DriverBinded initial value to true, for avoiding start function be invoked at DXE phase
    mPrivate->DriverBinded                        = TRUE;
    mPrivate->UsbCore                             = UsbCore;
    // Revision 3 added
    mPrivate->UsbLegacyControl[0]                 = UsbLegacyControl; // Instance in boot service code type memory

    InitializeListHead (&mPrivate->AsyncIntList);

    //
    // Install protocols
    //
    Status = EfiLibInstallDriverBindingComponentName2 (
               ImageHandle,
               SystemTable,
               &gUsbLegacySupportProviderDriverBinding,
               ImageHandle,
               &gUsbLegacySupportProviderComponentName,
               &gUsbLegacySupportProviderComponentName2
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    mUsbLegacySupportProtocol.Private = mPrivate;
    UsbLegacySupportHandle = NULL;
    Status = gBS->InstallProtocolInterface (
      &UsbLegacySupportHandle,
      &gUsbLegacySupportProviderProtocolGuid,
      EFI_NATIVE_INTERFACE,
      &mUsbLegacySupportProtocol
      );

    //
    // Relocate module
    //
    Status = UsbCore->ModuleRegistration (ImageHandle);
  } else {
    //
    // Now in SMM, get private storage first
    //
    Status = gBS->LocateProtocol (&gUsbLegacySupportProviderProtocolGuid, NULL, (VOID **)&UsbLegacySupportProtocol);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    
    mPrivate = UsbLegacySupportProtocol->Private;

  //
  // Locate the SMM Periodic Timer Dispatch 2 Protocol
  //
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmPeriodicTimerDispatch2ProtocolGuid,
                    NULL,
                    (VOID **)&mPrivate->PeriodicTimerDispatch
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    mPrivate->UsbLegacyControl[1]                 = UsbLegacyControl; // Instance in runtime service code type memory (SMRAM)

    //
    // W/A for system hangs on SMM when H2ODDT is enabled
    //
    // Register a periodic timer SMI event, callback amount every 32ms
    PeriodicContext.Period          = TIME_32ms;
    PeriodicContext.SmiTickInterval = TIME_32ms;
    Status = mPrivate->PeriodicTimerDispatch->Register (
                                                mPrivate->PeriodicTimerDispatch,
                                                PeriodicTimerCallbackFunction,
                                                &PeriodicContext,
                                                &mPeriodicHandle
                                                );

    //
    // Get the Sw dispatc h2 protocol
    //
    Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID **)&SwDispatch);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Register Legacy Support Provider Software SMI
    //
    SwContext.SwSmiInputValue = USB_LEGACY_SUPPORT_SW_SMI;
    Status = SwDispatch->Register (
      SwDispatch,
      LegacySupportSwSmiCallback,
      &SwContext,
      &Handle
      );
    ASSERT_EFI_ERROR(Status);

    Status = gSmst->SmmRegisterProtocolNotify (
      &gAcpiEnableCallbackStartProtocolGuid,
      PeriodicTimerHandOffCallbackFunction,
      &AcpiEnableCallbackStartEvent
      );
    ASSERT_EFI_ERROR (Status);

    // Set DriverBinded value to false, we need start function be invoked at BDS phase
    mPrivate->DriverBinded                        = FALSE;

    //
    // Setup Smm address convert table for Smm security policy
    //
    UsbCore->InsertAddressConvertTable (
                ACT_INSTANCE_BODY,
                mPrivate,
                sizeof (USB_LEGACY_SUPPORT_PROVIDER_PRIVATE)
                );
    UsbCore->InsertAddressConvertTable (
                ACT_INSTANCE_POINTER,
                &mPrivate,
                1
                );
    UsbCore->InsertAddressConvertTable (
                ACT_FUNCTION_POINTER,
                &mPrivate->DriverBinding,
                ((sizeof (EFI_DRIVER_BINDING_PROTOCOL) - sizeof (UINT32) - sizeof (EFI_HANDLE) * 2) / sizeof (VOID *))
                );
    UsbCore->InsertAddressConvertTable (
                ACT_INSTANCE_POINTER,
                &mPrivate->UsbCore,
                1
                );
    UsbCore->InsertAddressConvertTable (
                ACT_LINKING_LIST,
                &mPrivate->AsyncIntList,
                1
                );
  }

  return Status;
}

EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
/**
  Tests to see if this driver supports a given controller. If a child device is provided, 
  it further tests to see if this driver supports creating a handle for the specified child device.

  This function checks to see if the driver specified by This supports the device specified by 
  ControllerHandle. Drivers will typically use the device path attached to 
  ControllerHandle and/or the services from the bus I/O abstraction attached to 
  ControllerHandle to determine if the driver supports ControllerHandle. This function 
  may be called many times during platform initialization. In order to reduce boot times, the tests 
  performed by this function must be very small, and take as little time as possible to execute. This 
  function must not change the state of any hardware devices, and this function must be aware that the 
  device specified by ControllerHandle may already be managed by the same driver or a 
  different driver. This function must match its calls to AllocatePages() with FreePages(), 
  AllocatePool() with FreePool(), and OpenProtocol() with CloseProtocol().  
  Because ControllerHandle may have been previously started by the same driver, if a protocol is 
  already in the opened state, then it must not be closed with CloseProtocol(). This is required 
  to guarantee the state of ControllerHandle is not modified by this function.

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to test. This handle 
                                   must support a protocol interface that supplies 
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This 
                                   parameter is ignored by device drivers, and is optional for bus 
                                   drivers. For bus drivers, if this parameter is not NULL, then 
                                   the bus driver must determine if the bus controller specified 
                                   by ControllerHandle and the child controller specified 
                                   by RemainingDevicePath are both supported by this 
                                   bus driver.

  @retval EFI_SUCCESS              The device specified by ControllerHandle and
                                   RemainingDevicePath is supported by the driver specified by This.
  @retval EFI_ALREADY_STARTED      The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by the driver
                                   specified by This.
  @retval EFI_ACCESS_DENIED        The device specified by ControllerHandle and
                                   RemainingDevicePath is already being managed by a different
                                   driver or an application that requires exclusive access.
                                   Currently not implemented.
  @retval EFI_UNSUPPORTED          The device specified by ControllerHandle and
                                   RemainingDevicePath is not supported by the driver specified by This.
**/
{
  EFI_STATUS                            Status;

  if (mPrivate->DriverBinded) {
    Status = EFI_UNSUPPORTED;
  } else {
    Status = EFI_SUCCESS;
  }

  return Status;
}

EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
/**
  Starts a device controller or a bus controller.

  The Start() function is designed to be invoked from the EFI boot service ConnectController().
  As a result, much of the error checking on the parameters to Start() has been moved into this 
  common boot service. It is legal to call Start() from other locations, 
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE.
  2. If RemainingDevicePath is not NULL, then it must be a pointer to a naturally aligned
     EFI_DEVICE_PATH_PROTOCOL.
  3. Prior to calling Start(), the Supported() function for the driver specified by This must
     have been called with the same calling parameters, and Supported() must have returned EFI_SUCCESS.  

  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle     The handle of the controller to start. This handle 
                                   must support a protocol interface that supplies 
                                   an I/O abstraction to the driver.
  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This 
                                   parameter is ignored by device drivers, and is optional for bus 
                                   drivers. For a bus driver, if this parameter is NULL, then handles 
                                   for all the children of Controller are created by this driver.  
                                   If this parameter is not NULL and the first Device Path Node is 
                                   not the End of Device Path Node, then only the handle for the 
                                   child device specified by the first Device Path Node of 
                                   RemainingDevicePath is created by this driver.
                                   If the first Device Path Node of RemainingDevicePath is 
                                   the End of Device Path Node, no child handle is created by this
                                   driver.

  @retval EFI_SUCCESS              The device was started.
  @retval EFI_DEVICE_ERROR         The device could not be started due to a device error.Currently not implemented.
  @retval EFI_OUT_OF_RESOURCES     The request could not be completed due to a lack of resources.
  @retval Others                   The driver failded to start the device.

**/
{
  EFI_STATUS                            Status      = EFI_SUCCESS;
  EFI_USB_CORE_PROTOCOL                 *UsbCore    = NULL;
  
  if (mPrivate->DriverBinded) {
    return EFI_ALREADY_STARTED;
  }

  DEBUG ((EFI_D_INFO, "Enter UsbLegacySupportProviderDriverBindingStart ()\n"));

  UsbCore = mPrivate->UsbCore;

  //
  // Init EFI_USB3_HC_PROTOCOL interface and private data structure
  //
  mPrivate->Usb3Hc.GetCapability                = UsbHcGetCapability;
  mPrivate->Usb3Hc.Reset                        = NULL;
  mPrivate->Usb3Hc.GetState                     = UsbHcGetState;
  mPrivate->Usb3Hc.SetState                     = NULL;
  mPrivate->Usb3Hc.ControlTransfer              = NULL;
  mPrivate->Usb3Hc.BulkTransfer                 = NULL;
  mPrivate->Usb3Hc.InterruptTransfer            = UsbHcInterruptTransfer;
  mPrivate->Usb3Hc.IsochronousTransfer          = NULL;
  mPrivate->Usb3Hc.GetRootHubPortStatus         = NULL;
  mPrivate->Usb3Hc.SetRootHubPortFeature        = NULL;
  mPrivate->Usb3Hc.ClearRootHubPortFeature      = NULL;
  mPrivate->Usb3Hc.CancelIo                     = UsbHcCancelIo;
  mPrivate->Usb3Hc.QueryIo                      = UsbHcQueryIo;
  mPrivate->Usb3Hc.RegisterEvent                = NULL;
  mPrivate->Usb3Hc.UnregisterEvent              = NULL;
  mPrivate->Usb3Hc.LegacyHidSupport             = LegacyHidSupport;
  mPrivate->Usb3Hc.MajorRevision                = 0x2;
  mPrivate->Usb3Hc.MinorRevision                = 0x0;

  //
  // Register itself as a periodic timer provider to UsbCore
  //
  Status = UsbCore->InsertPeriodicTimerProvider (&mPrivate->Usb3Hc);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Insert Periodic Timer Provider (%r)\n", Status));
    goto Done;
  }


  // System already got a USB legacy HID provider or this driver registers success
  mPrivate->DriverBinded                        = TRUE;

  //
  // Setup Smm address convert table for Smm security policy
  //
  UsbCore->InsertAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.LegacyHidSupport,
            1
            );
  UsbCore->InsertAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.GetCapability,
            1
            );
  UsbCore->InsertAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.GetState,
            1
            );
  UsbCore->InsertAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.InterruptTransfer,
            1
            );
  UsbCore->InsertAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.CancelIo,
            1
            );
  UsbCore->InsertAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.QueryIo,
            1
            );

Done:

  DEBUG ((EFI_D_INFO, "Exit UsbLegacySupportProviderDriverBindingStart ()\n"));

  //
  // Always return unsupported here, we are not really want to binding with
  // a controller handle
  //
  return EFI_UNSUPPORTED;
}

/**
  Stops a device controller or a bus controller.
  
  The Stop() function is designed to be invoked from the EFI boot service DisconnectController(). 
  As a result, much of the error checking on the parameters to Stop() has been moved 
  into this common boot service. It is legal to call Stop() from other locations, 
  but the following calling restrictions must be followed, or the system behavior will not be deterministic.
  1. ControllerHandle must be a valid EFI_HANDLE that was used on a previous call to this
     same driver's Start() function.
  2. The first NumberOfChildren handles of ChildHandleBuffer must all be a valid
     EFI_HANDLE. In addition, all of these handles must have been created in this driver's
     Start() function, and the Start() function must have called OpenProtocol() on
     ControllerHandle with an Attribute of EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.
  
  @param[in]  This              A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.
  @param[in]  ControllerHandle  A handle to the device being stopped. The handle must 
                                support a bus specific I/O protocol for the driver 
                                to use to stop the device.
  @param[in]  NumberOfChildren  The number of child device handles in ChildHandleBuffer.
  @param[in]  ChildHandleBuffer An array of child handles to be freed. May be NULL 
                                if NumberOfChildren is 0.

  @retval EFI_SUCCESS           The device was stopped.
  @retval EFI_DEVICE_ERROR      The device could not be stopped due to a device error.

**/
EFI_STATUS
EFIAPI
UsbLegacySupportProviderDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer OPTIONAL
  )
{
  EFI_STATUS                            Status      = EFI_SUCCESS;
  EFI_USB_CORE_PROTOCOL                 *UsbCore    = NULL;
  EFI_USB3_HC_PROTOCOL                  *Usb3Hc     = NULL;

  if ((!mPrivate->DriverBinded) || (mPrivate->UsbCore == NULL)) {
    return EFI_UNSUPPORTED;
  }

  UsbCore = mPrivate->UsbCore;

  //
  // W/A for system hangs on SMM when H2ODDT is enabled
  //
  Status = UsbCore->RemovePeriodicTimerProvider (&mPrivate->Usb3Hc);
  DEBUG ((EFI_D_ERROR, "Remove Periodic Timer Provider (%r)\n", Status));

  //
  // Get legacy support provider. If system already has a legacy support
  // provider then we don't need this driver provides service.
  //
  Usb3Hc = &mPrivate->Usb3Hc;
  Status = UsbCore->GetLegacySupportProvider (&Usb3Hc);
  if (!EFI_ERROR (Status)) {
    Status = UsbCore->RemoveLegacySupportProvider (Usb3Hc);
    DEBUG ((EFI_D_ERROR, "Remove Legacy Support Provider (%r)\n", Status));
    if (!EFI_ERROR (Status)) {
      mPrivate->EmuEnabled                        = FALSE;
      mPrivate->DriverBinded                      = FALSE;
    }
  }

  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.LegacyHidSupport
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.GetCapability
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.GetState
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.InterruptTransfer
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.CancelIo
            );
  UsbCore->RemoveAddressConvertTable (
            ACT_FUNCTION_POINTER,
            &mPrivate->Usb3Hc.QueryIo
            );

  return Status;
}

EFI_STATUS
EFIAPI
UsbLegacyControl (
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  )
/*

  Routine Description:

    Main routine for the USB legacy control

  Arguments:

    Command - USB_LEGACY_CONTROL_SETUP_EMULATION
              USB_LEGACY_CONTROL_GET_KBC_DATA_POINTER
              USB_LEGACY_CONTROL_GET_KBC_STATUS_POINTER
              USB_LEGACY_CONTROL_GENERATE_IRQ
    Param   - The parameter for the command

  Returns:

    EFI_SUCCESS

*/
{
  UINT8         Data8 = 0xff;

  switch (Command) {
  case USB_LEGACY_CONTROL_SETUP_EMULATION:
    //
    // Used for emulation on/off
    //
    if (!Param) {
      //
      // Turn off the emulation
      //
      mPrivate->EmuEnabled = FALSE;
    } else {
      //
      // Turn on the emulation
      //
      mPrivate->EmuEnabled = TRUE;
    }

    break;

  case USB_LEGACY_CONTROL_GET_KBC_DATA_POINTER:
    //
    // Get the "pointer" of KBC data (port 0x60)
    //
    *(UINT32 **) Param = &mHidOutputData;
    break;

  case USB_LEGACY_CONTROL_GET_KBC_STATUS_POINTER:
    //
    // Get the "pointer" of KBC status (port 0x64)
    //
    *(UINT32 **) Param = &mHidStatusData;
    break;

  case USB_LEGACY_CONTROL_GENERATE_IRQ:
    if (mPrivate->EmuEnabled) {
      Data8 = ((mHidStatusData & 0x20) ? IRQ12 : IRQ1);

      if (Data8 == IRQ1) {
      } else if (Data8 == IRQ12) {
        // TODO: Add handle of Mouse event here
      }
    }

    //
    // We need to output data right now!
    //
    *(BOOLEAN *) Param = TRUE;
    break;
  }

  return EFI_SUCCESS;
}

VOID
ReleaseInterruptList (
  IN USB_LEGACY_SUPPORT_PROVIDER_PRIVATE                *Private,
  IN UINT8                                              DevAddr,
  IN UINT8                                              EndPoint,
  IN UINTN                                              Interval
  )
/*++

Routine Description:

  Release Interrupt List
Arguments:

  HcDev     - USB_HC_DEV
  DevAddr   - Device address, 0 means all
  DnePoint  - EndPoint

Returns:

  VOID

--*/
{
  LIST_ENTRY      *Link;
  LIST_ENTRY      *SavedLink;
  INTERRUPT_LIST  *PtrList;
  BOOLEAN         RemoveAll;

  if (
    (Private == NULL) ||
    (Private->UsbCore == NULL)
    ) {
    return;
  }

  RemoveAll = (!DevAddr && !EndPoint && !Interval) ? TRUE : FALSE;
  Link = &Private->AsyncIntList;
  if (IsListEmpty (Link)) return;
  //
  // Free the resources in the interrupt list
  //
  SavedLink = Link->ForwardLink;
  while (SavedLink != &mPrivate->AsyncIntList) {
    Link      = SavedLink;
    SavedLink = Link->ForwardLink;
    PtrList   = INTERRUPT_LIST_FROM_LINK (Link);
    if (RemoveAll ||
        (DevAddr && PtrList->DevAddr == DevAddr && PtrList->EndPoint == EndPoint) ||
        (!DevAddr && PtrList->DevAddr == DevAddr && PtrList->Interval == Interval)) {
      RemoveEntryList (&PtrList->Link);
      Private->UsbCore->FreeBuffer(
                        sizeof(INTERRUPT_LIST),
                        PtrList
                        );
      //
      // Remove Smm address convert table
      //
      Private->UsbCore->RemoveAddressConvertTable (
                        ACT_FUNCTION_POINTER,
                        &PtrList->InterruptCallBack
                        );
      Private->UsbCore->RemoveAddressConvertTable (
                        ACT_INSTANCE_BODY,
                        PtrList
                        );
      Private->UsbCore->RemoveAddressConvertTable (
                        ACT_INSTANCE_POINTER,
                        &PtrList->InterruptContext
                        );
      if (!RemoveAll) break;
    }
  }
}

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
  )
/*++

  Routine Description:
    Retrieves capabilities of USB host controller.

  Arguments:
    This                      - A pointer to the EFI_USB3_HC_PROTOCOL instance.

    MaxSpeed             - A pointer to the max speed USB host controller supports.

    PortNumber           - A pointer to the number of root hub ports.

    Is64BitCapable      - A pointer to an integer to show whether USB host controller
                                  supports 64-bit memory addressing.
  Returns:
    EFI_SUCCESS
        The host controller capabilities were retrieved successfully.
    EFI_INVALID_PARAMETER
        MaxSpeed or PortNumber or Is64BitCapable is NULL.
    EFI_DEVICE_ERROR
  An error was encountered while attempting to retrieve the capabilities.

--*/
{
  if ((NULL == MaxSpeed)
    ||(NULL == PortNumber)
    || (NULL == Is64BitCapable))
  {
    return EFI_INVALID_PARAMETER;
  }

  *MaxSpeed       = EFI_USB_SPEED_FULL;
  *Is64BitCapable = (UINT8)FALSE;
  *PortNumber     = (UINT8)0;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UsbHcGetState (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  OUT    EFI_USB_HC_STATE                               *State
  )
/*++

  Routine Description:
    Retrieves current state of the USB host controller.

  Arguments:

    This      A pointer to the EFI_USB3_HC_PROTOCOL instance.

    State     A pointer to the EFI_USB_HC_STATE data structure that
              indicates current state of the USB host controller.
              Type EFI_USB_HC_STATE is defined below.

    typedef enum {
      EfiUsbHcStateHalt,
      EfiUsbHcStateOperational,
      EfiUsbHcStateSuspend,
      EfiUsbHcStateMaximum
    } EFI_USB_HC_STATE;

  Returns:
    EFI_SUCCESS
            The state information of the host controller was returned in State.
    EFI_INVALID_PARAMETER
            State is NULL.
    EFI_DEVICE_ERROR
            An error was encountered while attempting to retrieve the
            host controller's current state.
--*/
{
  if ((State == NULL) || (This != &mPrivate->Usb3Hc)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Return Halt state due to HC not owned by BIOS
  //
  *State = EfiUsbHcStateHalt;
  if (mPrivate->PeriodicTimerDispatch != NULL) {
    *State = EfiUsbHcStateOperational;
  }

  return EFI_SUCCESS;
}

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
  )
/*++
  
  Routine Description:
  
    Submits an asynchronous interrupt transfer to an interrupt endpoint of a USB device.
  
  Arguments:
    
    This                : A pointer to the EFI_USB3_HC_PROTOCOL instance.
    DeviceAddress       : Target device address
    EndPointAddress     : Endpoint number with direction
    DeviceSpeed         : Indicates transfer speed of device.
    MaximumPacketLength : Maximum packet size of the target endpoint
    Data                : Data to transmit or receive
    DataLength          : A pointer to the length of data to receive    
    DataToggle          : On input, the data toggle to use; On output, next data toggle
    TimeOut             : Indicates the maximum time, in milliseconds
    Translator          : A pointr to the transaction translator data.
    CallBackFunction    : Function to call periodically
    Context             : The context that is passed to the CallBackFunction
    PollingInterval     : Interrupt poll rate in milliseconds
    TransferResult      : Variable to receive transfer result
  
  Returns:
  
    EFI_SUCCESS           : Request is submitted or cancelled
    EFI_INVALID_PARAMETER : Some parameters are invalid.
    EFI_OUT_OF_RESOURCES  : Failed due to a lack of resources.  
    EFI_DEVICE_ERROR      : Failed to due to device error
        
--*/
{
  INTERRUPT_LIST        *AsyncList;
  EFI_STATUS            Status;
  EFI_USB_HC_STATE      State;

  UsbHcGetState (This, &State);
  if (
    (DeviceAddress) ||
    (!CallBackFunction) ||
    (PollingInterval == 0) ||
    (mPrivate == NULL) ||
    (mPrivate->UsbCore == NULL) ||
    (State == EfiUsbHcStateHalt)  // Return error if HC in halt state or not owned by BIOS
    ) {
    return EFI_UNSUPPORTED;
  }

  if (This != &mPrivate->Usb3Hc) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Async interrupt transfer. Allocate pool for Interrput list
  //
  Status = mPrivate->UsbCore->AllocateBuffer (
                             sizeof(INTERRUPT_LIST),
                             ALIGNMENT_32,
                             (VOID **)&AsyncList
                             );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  AsyncList->Signature         = INTERRUPT_LIST_SIGNATURE;
  AsyncList->DevAddr           = DeviceAddress;
  AsyncList->EndPoint          = EndPointAddress;
  AsyncList->Interval          = (UINT8) PollingInterval;
  AsyncList->InterruptCallBack = CallBackFunction;
  AsyncList->InterruptContext  = Context;

  InsertHeadList (&mPrivate->AsyncIntList, &AsyncList->Link);

  //
  // Setup Smm address convert table for Smm security policy
  //
  mPrivate->UsbCore->InsertAddressConvertTable (
                    ACT_FUNCTION_POINTER,
                    &AsyncList->InterruptCallBack,
                    1
                    );
  mPrivate->UsbCore->InsertAddressConvertTable (
                    ACT_INSTANCE_BODY,
                    AsyncList,
                    sizeof (INTERRUPT_LIST)
                    );
  mPrivate->UsbCore->InsertAddressConvertTable (
                    ACT_INSTANCE_POINTER,
                    &AsyncList->InterruptContext,
                    1
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UsbHcCancelIo (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINT8                                          DeviceAddress,
  IN     UINT8                                          EndPointAddress,
  IN     UINTN                                          PollingInterval,
  IN OUT UINT8                                          *DataToggle
  )
{
  //
  // Delete Async interrupt transfer request
  //
  ReleaseInterruptList (
    mPrivate,
    DeviceAddress,
    EndPointAddress,
    PollingInterval
    );
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UsbHcQueryIo (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINT8                                          DeviceAddress,
  IN     UINT8                                          EndPointAddress,
  IN     UINTN                                          PollingInterval
  )
/*++

  Routine Description:

    Query the AsyncInterruptTransfer exist or not.

  Arguments:

    This            - A pointer to the EFI_USB3_HC_PROTOCOL instance.
    DeviceAddress   - Represents the address of the target device on the USB,
                      which is assigned during USB enumeration.
    EndPointAddress - End point address
    PollingInterval - Indicates the interval, in milliseconds, that the
                      asynchronous interrupt transfer is polled.
                      This parameter is required request asynchronous interrupt
                      transfer.
  Returns:

    EFI_SUCCESS       Exist
    EFI_NOT_FOUND     Not found

--*/
{
  LIST_ENTRY      *Link;
  LIST_ENTRY      *SavedLink;
  INTERRUPT_LIST  *PtrList;

  if (This != &mPrivate->Usb3Hc) {
    return EFI_INVALID_PARAMETER;
  }

  if (DeviceAddress) {
    return EFI_UNSUPPORTED;
  }

  Link = &mPrivate->AsyncIntList;
  if (IsListEmpty (Link)) {
    return EFI_NOT_FOUND;
  }
  SavedLink = Link->ForwardLink;
  while (SavedLink != &mPrivate->AsyncIntList) {
    Link      = SavedLink;
    SavedLink = Link->ForwardLink;
    PtrList   = INTERRUPT_LIST_FROM_LINK (Link);
    if (!DeviceAddress && PtrList->DevAddr == DeviceAddress && PtrList->Interval == PollingInterval) {
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
LegacyHidSupport (
  IN     EFI_USB3_HC_PROTOCOL                           *This,
  IN     UINTN                                          Command,
  IN     VOID                                           *Param
  )
{
  EFI_STATUS                  Status    = EFI_UNSUPPORTED;
  BOOLEAN                     InSmm     = FALSE;

  if (
    (mPrivate != NULL) &&
    (mPrivate->UsbCore != NULL)
    ) {

    mPrivate->UsbCore->IsInSmm (&InSmm);

    //
    // Use UsbLegacyControl to take care about legacy HID support
    //
    if (mPrivate->UsbLegacyControl[InSmm] != NULL) {
      Status = mPrivate->UsbLegacyControl[InSmm] (Command, Param);
    }
  }

  return Status;
}

