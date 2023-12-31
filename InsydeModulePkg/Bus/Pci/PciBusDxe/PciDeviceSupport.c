/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/**
  Supporting functions implementaion for PCI devices management.

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "PciBus.h"
#include <Protocol/Security.h>
#include <Protocol/Security2.h>
#include <BusMasterOverride.h>
//
// This device structure is serviced as a header.
// Its next field points to the first root bridge device node.
//
LIST_ENTRY      mPciDevicePool;
#define IS_PCI_1394(_p)            IS_CLASS2 (_p, PCI_CLASS_SERIAL, PCI_CLASS_SERIAL_FIREWIRE)

/**
  Initialize the PCI devices pool.

**/
VOID
InitializePciDevicePool (
  VOID
  )
{
  InitializeListHead (&mPciDevicePool);
}

/**
  Insert a root bridge into PCI device pool.

  @param RootBridge     A pointer to the PCI_IO_DEVICE.

**/
VOID
InsertRootBridge (
  IN PCI_IO_DEVICE      *RootBridge
  )
{
  InsertTailList (&mPciDevicePool, &(RootBridge->Link));
}

/**
  This function is used to insert a PCI device node under
  a bridge.

  @param Bridge         The PCI bridge.
  @param PciDeviceNode  The PCI device needs inserting.

**/
VOID
InsertPciDevice (
  IN PCI_IO_DEVICE      *Bridge,
  IN PCI_IO_DEVICE      *PciDeviceNode
  )
{
  InsertTailList (&Bridge->ChildList, &(PciDeviceNode->Link));
  PciDeviceNode->Parent = Bridge;
}

/**
  Destroy root bridge and remove it from deivce tree.

  @param RootBridge     The bridge want to be removed.

**/
VOID
DestroyRootBridge (
  IN PCI_IO_DEVICE      *RootBridge
  )
{
  DestroyPciDeviceTree (RootBridge);

  FreePciDevice (RootBridge);
}

/**
  Destroy a pci device node.

  All direct or indirect allocated resource for this node will be freed.

  @param PciIoDevice  A pointer to the PCI_IO_DEVICE to be destoried.

**/
VOID
FreePciDevice (
  IN PCI_IO_DEVICE    *PciIoDevice
  )
{
  ASSERT (PciIoDevice != NULL);
  //
  // Assume all children have been removed underneath this device
  //
  if (PciIoDevice->ResourcePaddingDescriptors != NULL) {
    FreePool (PciIoDevice->ResourcePaddingDescriptors);
  }

  if (PciIoDevice->DevicePath != NULL) {
    FreePool (PciIoDevice->DevicePath);
  }

  FreePool (PciIoDevice);
}

/**
  Destroy all the pci device node under the bridge.
  Bridge itself is not included.

  @param Bridge      A pointer to the PCI_IO_DEVICE.

**/
VOID
DestroyPciDeviceTree (
  IN PCI_IO_DEVICE      *Bridge
  )
{
  LIST_ENTRY      *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  while (!IsListEmpty (&Bridge->ChildList)) {

    CurrentLink = Bridge->ChildList.ForwardLink;

    //
    // Remove this node from the linked list
    //
    RemoveEntryList (CurrentLink);

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (!IsListEmpty (&Temp->ChildList)) {
      DestroyPciDeviceTree (Temp);
    }

    FreePciDevice (Temp);
  }
}

/**
  Destroy all device nodes under the root bridge
  specified by Controller.

  The root bridge itself is also included.

  @param  Controller    Root bridge handle.

  @retval EFI_SUCCESS   Destory all devcie nodes successfully.
  @retval EFI_NOT_FOUND Cannot find any PCI device under specified
                        root bridge.

**/
EFI_STATUS
DestroyRootBridgeByHandle (
  IN EFI_HANDLE        Controller
  )
{

  LIST_ENTRY      *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = mPciDevicePool.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &mPciDevicePool) {
    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp->Handle == Controller) {

      RemoveEntryList (CurrentLink);

      DestroyPciDeviceTree (Temp);

      FreePciDevice (Temp);

      return EFI_SUCCESS;
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return EFI_NOT_FOUND;
}

/**
  Dummy function to bypass File Authentication.

  @param[in] This                   The EFI_SECURITY_ARCH_PROTOCOL instance.
  @param[in] AuthenticationStatus   This is the authentication type returned from the Section
                                    Extraction protocol.  See the Section Extraction Protocol
                                    Specification for details on this type.
  @param[in] File                   This is a pointer to the device path of the file that is
                                    being dispatched.  This will optionally be used for logging.

  @retval EFI_SUCCESS               Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
DummyFileAuthenticationState (
  IN  CONST EFI_SECURITY_ARCH_PROTOCOL     *This,
  IN  UINT32                               AuthenticationStatus,
  IN  CONST EFI_DEVICE_PATH_PROTOCOL       *File
  )
{
  return EFI_SUCCESS;
}


/**
  Dummy function to bypass security2 File Authentication.

  @param[in] This         The EFI_SECURITY2_ARCH_PROTOCOL instance.
  @param[in] File         A pointer to the device path of the file that is
                          being dispatched. This will optionally be used for logging.
  @param[in] FileBuffer   A pointer to the buffer with the UEFI file image.
  @param[in] FileSize     The size of the file.
  @param[in] BootPolicy   A boot policy that was used to call LoadImage() UEFI service. If
                          FileAuthentication() is invoked not from the LoadImage(),
                          BootPolicy must be set to FALSE.

  @retval EFI_SUCCESS     Always return EFI_SUCCESS.
**/
EFI_STATUS
EFIAPI
DummySecurity2FileAuthentication (
  IN CONST EFI_SECURITY2_ARCH_PROTOCOL *This,
  IN CONST EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
  IN VOID                              *FileBuffer,
  IN UINTN                             FileSize,
  IN BOOLEAN                           BootPolicy
  )
{
  return EFI_SUCCESS;
}

/**
  Hook Security relative protocols to ignore security check temporary.

  @param[in] FileAuthenticationState     Output original EFI_SECURITY_FILE_AUTHENTICATION_STATE function.
  @param[in] Security2FileAuthentication Output original EFI_SECURITY2_FILE_AUTHENTICATION function.

  @retval EFI_SUCCESS                    Hook security relative protocols seccessful
**/
EFI_STATUS
HookSecurityProtocols (
  OUT   EFI_SECURITY_FILE_AUTHENTICATION_STATE  *FileAuthenticationState,
  OUT   EFI_SECURITY2_FILE_AUTHENTICATION       *Security2FileAuthentication
  )
{
  EFI_STATUS                              Status;
  EFI_SECURITY_ARCH_PROTOCOL              *Security;
  EFI_SECURITY2_ARCH_PROTOCOL             *Security2;

  Status = gBS->LocateProtocol (&gEfiSecurityArchProtocolGuid, NULL, (VOID **)&Security);
  if (!EFI_ERROR (Status)) {
    *FileAuthenticationState          = Security->FileAuthenticationState;
    Security->FileAuthenticationState = DummyFileAuthenticationState;
  }
  Status = gBS->LocateProtocol (&gEfiSecurity2ArchProtocolGuid, NULL, (VOID **)&Security2);
  if (!EFI_ERROR (Status)) {
    *Security2FileAuthentication  = Security2->FileAuthentication;
    Security2->FileAuthentication = DummySecurity2FileAuthentication;
  }

  return EFI_SUCCESS;
}

/**
  Restore functions in security relative protocols.

  @param[in] FileAuthenticationState     Input original EFI_SECURITY_FILE_AUTHENTICATION_STATE function.
  @param[in] Security2FileAuthentication Input original EFI_SECURITY2_FILE_AUTHENTICATION function.

  @retval EFI_SUCCESS                    Restore security relative protocols successful.
**/
EFI_STATUS
RestoreSecurityProtocols (
  IN   EFI_SECURITY_FILE_AUTHENTICATION_STATE  FileAuthenticationState,
  IN   EFI_SECURITY2_FILE_AUTHENTICATION       Security2FileAuthentication
  )
{
  EFI_STATUS                              Status;
  EFI_SECURITY_ARCH_PROTOCOL              *Security;
  EFI_SECURITY2_ARCH_PROTOCOL             *Security2;

  Status = gBS->LocateProtocol (&gEfiSecurityArchProtocolGuid, NULL, (VOID **)&Security);
  if (!EFI_ERROR (Status)) {
    Security->FileAuthenticationState = FileAuthenticationState;
  }

  Status = gBS->LocateProtocol (&gEfiSecurity2ArchProtocolGuid, NULL, (VOID **)&Security2);
  if (!EFI_ERROR (Status)) {
    Security2->FileAuthentication  = Security2FileAuthentication;
  }

  return EFI_SUCCESS;
}

/**
  This function registers the PCI IO device.

  It creates a handle for this PCI IO device (if the handle does not exist), attaches
  appropriate protocols onto the handle, does necessary initialization, and sets up
  parent/child relationship with its bus controller.

  @param Controller     An EFI handle for the PCI bus controller.
  @param PciIoDevice    A PCI_IO_DEVICE pointer to the PCI IO device to be registered.
  @param Handle         A pointer to hold the returned EFI handle for the PCI IO device.

  @retval EFI_SUCCESS   The PCI device is successfully registered.
  @retval other         An error occurred when registering the PCI device.

**/
EFI_STATUS
RegisterPciDevice (
  IN  EFI_HANDLE          Controller,
  IN  PCI_IO_DEVICE       *PciIoDevice,
  OUT EFI_HANDLE          *Handle      OPTIONAL
  )
{
  EFI_STATUS                              Status;
  VOID                                    *PlatformOpRomBuffer;
  UINTN                                   PlatformOpRomSize;
  UINT8                                   PciExpressCapRegOffset;
  EFI_PCI_IO_PROTOCOL                     *PciIo;
  UINT8                                   Data8;
  BOOLEAN                                 HasEfiImage;
  UINTN                                   Segment;
  UINTN                                   Bus;
  UINTN                                   Device;
  UINTN                                   Function;
  UINT64                                  BootScriptPciAddress;
  BOOLEAN                                 OpRomInFV;
  EFI_SECURITY_FILE_AUTHENTICATION_STATE  OrgFileAuthenticationState;
  EFI_SECURITY2_FILE_AUTHENTICATION       Security2FileAuthentication;
  //
  // Install the pciio protocol, device path protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &PciIoDevice->Handle,
                  &gEfiDevicePathProtocolGuid,
                  PciIoDevice->DevicePath,
                  &gEfiPciIoProtocolGuid,
                  &PciIoDevice->PciIo,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Detect if PCI Express Device
  //
  PciExpressCapRegOffset = 0;
  Status = LocateCapabilityRegBlock (
             PciIoDevice,
             EFI_PCI_CAPABILITY_ID_PCIEXP,
             &PciExpressCapRegOffset,
             NULL
             );
  if (!EFI_ERROR (Status)) {
    PciIoDevice->IsPciExp = TRUE;
  }

  //
  // Force Interrupt line to "Unknown" or "No Connection"
  //
  PciIo = &(PciIoDevice->PciIo);
  Data8 = PCI_INT_LINE_UNKNOWN;
  PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, 0x3C, 1, &Data8);
  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  BootScriptPciAddress = S3_BOOT_SCRIPT_LIB_PCI_ADDRESS (Bus, Device, Function, 0x3C);
  S3BootScriptSavePciCfgWrite (
    S3BootScriptWidthUint8,
    BootScriptPciAddress,
    1,
    &Data8
    );

  //
  // Process OpRom
  //
  OpRomInFV = FALSE;
  if (!PciIoDevice->AllOpRomProcessed) {

    //
    // Get the OpRom provided by platform
    //
    if (gPciPlatformProtocol != NULL) {
      Status = gPciPlatformProtocol->GetPciRom (
                                       gPciPlatformProtocol,
                                       PciIoDevice->Handle,
                                       &PlatformOpRomBuffer,
                                       &PlatformOpRomSize
                                       );
      if (!EFI_ERROR (Status)) {
        PciIoDevice->EmbeddedRom    = FALSE;
        PciIoDevice->RomSize        = PlatformOpRomSize;
        PciIoDevice->PciIo.RomSize  = PlatformOpRomSize;
        PciIoDevice->PciIo.RomImage = PlatformOpRomBuffer;
        OpRomInFV = TRUE;
        //
        // For OpROM read from gPciPlatformProtocol:
        // Add the Rom Image to internal database for later PCI light enumeration
        //
        PciRomAddImageMapping (
          NULL,
          PciIoDevice->PciRootBridgeIo->SegmentNumber,
          PciIoDevice->BusNumber,
          PciIoDevice->DeviceNumber,
          PciIoDevice->FunctionNumber,
          (UINT64) (UINTN) PciIoDevice->PciIo.RomImage,
          PciIoDevice->PciIo.RomSize
          );
      }
    } else if (gPciOverrideProtocol != NULL) {
      Status = gPciOverrideProtocol->GetPciRom (
                                       gPciOverrideProtocol,
                                       PciIoDevice->Handle,
                                       &PlatformOpRomBuffer,
                                       &PlatformOpRomSize
                                       );
      if (!EFI_ERROR (Status)) {
        PciIoDevice->EmbeddedRom    = FALSE;
        PciIoDevice->RomSize        = PlatformOpRomSize;
        PciIoDevice->PciIo.RomSize  = PlatformOpRomSize;
        PciIoDevice->PciIo.RomImage = PlatformOpRomBuffer;
        //
        // For OpROM read from gPciOverrideProtocol:
        // Add the Rom Image to internal database for later PCI light enumeration
        //
        PciRomAddImageMapping (
          NULL,
          PciIoDevice->PciRootBridgeIo->SegmentNumber,
          PciIoDevice->BusNumber,
          PciIoDevice->DeviceNumber,
          PciIoDevice->FunctionNumber,
          (UINT64) (UINTN) PciIoDevice->PciIo.RomImage,
          PciIoDevice->PciIo.RomSize
          );
      }
    }
  }

  //
  // Determine if there are EFI images in the option rom
  //
  HasEfiImage = ContainEfiImage (PciIoDevice->PciIo.RomImage, PciIoDevice->PciIo.RomSize);

  if (HasEfiImage) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &PciIoDevice->Handle,
                    &gEfiLoadFile2ProtocolGuid,
                    &PciIoDevice->LoadFile2,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      gBS->UninstallMultipleProtocolInterfaces (
             &PciIoDevice->Handle,
             &gEfiDevicePathProtocolGuid,
             PciIoDevice->DevicePath,
             &gEfiPciIoProtocolGuid,
             &PciIoDevice->PciIo,
             NULL
             );
      return Status;
    }
  }


  if (!PciIoDevice->AllOpRomProcessed) {

    PciIoDevice->AllOpRomProcessed = TRUE;

    //
    // Dispatch the EFI OpRom for the PCI device.
    // The OpRom is got from platform in the above code
    // or loaded from device in the previous round of bus enumeration
    //
    if (HasEfiImage) {
      if (OpRomInFV) {
        HookSecurityProtocols (&OrgFileAuthenticationState, &Security2FileAuthentication);
        ProcessOpRomImage (PciIoDevice);
        RestoreSecurityProtocols (OrgFileAuthenticationState, Security2FileAuthentication);
      } else {
        ProcessOpRomImage (PciIoDevice);
      }
    }
  }

  if (PciIoDevice->BusOverride) {
    //
    // Install Bus Specific Driver Override Protocol
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &PciIoDevice->Handle,
                    &gEfiBusSpecificDriverOverrideProtocolGuid,
                    &PciIoDevice->PciDriverOverride,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      gBS->UninstallMultipleProtocolInterfaces (
             &PciIoDevice->Handle,
             &gEfiDevicePathProtocolGuid,
             PciIoDevice->DevicePath,
             &gEfiPciIoProtocolGuid,
             &PciIoDevice->PciIo,
             NULL
             );
      if (HasEfiImage) {
        gBS->UninstallMultipleProtocolInterfaces (
               &PciIoDevice->Handle,
               &gEfiLoadFile2ProtocolGuid,
               &PciIoDevice->LoadFile2,
               NULL
               );
      }

      return Status;
    }
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  (VOID **) &(PciIoDevice->PciRootBridgeIo),
                  gPciBusDriverBinding.DriverBindingHandle,
                  PciIoDevice->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Handle != NULL) {
    *Handle = PciIoDevice->Handle;
  }

  //
  // Indicate the pci device is registered
  //
  PciIoDevice->Registered = TRUE;

  return EFI_SUCCESS;
}

/**
  This function is used to remove the whole PCI devices on the specified bridge from
  the root bridge.

  @param RootBridgeHandle   The root bridge device handle.
  @param Bridge             The bridge device to be removed.

**/
VOID
RemoveAllPciDeviceOnBridge (
  EFI_HANDLE               RootBridgeHandle,
  PCI_IO_DEVICE            *Bridge
  )
{
  LIST_ENTRY      *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  while (!IsListEmpty (&Bridge->ChildList)) {

    CurrentLink = Bridge->ChildList.ForwardLink;
    Temp        = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    //
    // Check if the current node has been deregistered before
    // If it is not, then deregister it
    //
    if (Temp->Registered) {
      DeRegisterPciDevice (RootBridgeHandle, Temp->Handle);
    }

    //
    // Remove this node from the linked list
    //
    RemoveEntryList (CurrentLink);

    if (!IsListEmpty (&Temp->ChildList)) {
      RemoveAllPciDeviceOnBridge (RootBridgeHandle, Temp);
    }

    FreePciDevice (Temp);
  }
}

/**
  This function is used to de-register the PCI IO device.

  That includes un-installing PciIo protocol from the specified PCI
  device handle.

  @param Controller    An EFI handle for the PCI bus controller.
  @param Handle        PCI device handle.

  @retval EFI_SUCCESS  The PCI device is successfully de-registered.
  @retval other        An error occurred when de-registering the PCI device.

**/
EFI_STATUS
DeRegisterPciDevice (
  IN  EFI_HANDLE                     Controller,
  IN  EFI_HANDLE                     Handle
  )

{
  EFI_PCI_IO_PROTOCOL             *PciIo;
  EFI_STATUS                      Status;
  PCI_IO_DEVICE                   *PciIoDevice;
  PCI_IO_DEVICE                   *Node;
  LIST_ENTRY                      *CurrentLink;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  gPciBusDriverBinding.DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (!EFI_ERROR (Status)) {
    PciIoDevice = PCI_IO_DEVICE_FROM_PCI_IO_THIS (PciIo);

    //
    // If it is already de-registered
    //
    if (!PciIoDevice->Registered) {
      return EFI_SUCCESS;
    }

    //
    // If it is PPB, first de-register its children
    //

    if (!IsListEmpty (&PciIoDevice->ChildList)) {

      CurrentLink = PciIoDevice->ChildList.ForwardLink;

      while (CurrentLink != NULL && CurrentLink != &PciIoDevice->ChildList) {
        Node    = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
        Status  = DeRegisterPciDevice (Controller, Node->Handle);

        if (EFI_ERROR (Status)) {
          return Status;
        }

        CurrentLink = CurrentLink->ForwardLink;
      }
    }

    //
    // Close the child handle
    //
    Status = gBS->CloseProtocol (
                    Controller,
                    &gEfiPciRootBridgeIoProtocolGuid,
                    gPciBusDriverBinding.DriverBindingHandle,
                    Handle
                    );

    //
    // Un-install the Device Path protocol and PCI I/O protocol
    // and Bus Specific Driver Override protocol if needed.
    //
    if (PciIoDevice->BusOverride) {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Handle,
                      &gEfiDevicePathProtocolGuid,
                      PciIoDevice->DevicePath,
                      &gEfiPciIoProtocolGuid,
                      &PciIoDevice->PciIo,
                      &gEfiBusSpecificDriverOverrideProtocolGuid,
                      &PciIoDevice->PciDriverOverride,
                      NULL
                      );
    } else {
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Handle,
                      &gEfiDevicePathProtocolGuid,
                      PciIoDevice->DevicePath,
                      &gEfiPciIoProtocolGuid,
                      &PciIoDevice->PciIo,
                      NULL
                      );
    }

    if (!EFI_ERROR (Status)) {
      //
      // Try to uninstall LoadFile2 protocol if exists
      //
      Status = gBS->OpenProtocol (
                      Handle,
                      &gEfiLoadFile2ProtocolGuid,
                      NULL,
                      gPciBusDriverBinding.DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                      );
      if (!EFI_ERROR (Status)) {
        Status = gBS->UninstallMultipleProtocolInterfaces (
                        Handle,
                        &gEfiLoadFile2ProtocolGuid,
                        &PciIoDevice->LoadFile2,
                        NULL
                        );
      }
      //
      // Restore Status
      //
      Status = EFI_SUCCESS;
    }


    if (EFI_ERROR (Status)) {
      gBS->OpenProtocol (
             Controller,
             &gEfiPciRootBridgeIoProtocolGuid,
             (VOID **) &PciRootBridgeIo,
             gPciBusDriverBinding.DriverBindingHandle,
             Handle,
             EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
             );
      return Status;
    }

    //
    // The Device Driver should disable this device after disconnect
    // so the Pci Bus driver will not touch this device any more.
    // Restore the register field to the original value
    //
    PciIoDevice->Registered = FALSE;
    PciIoDevice->Handle     = NULL;
  } else {

    //
    // Handle may be closed before
    //
    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

/**
  Start to manage the PCI device on the specified root bridge or PCI-PCI Bridge.

  @param Controller          The root bridge handle.
  @param RootBridge          A pointer to the PCI_IO_DEVICE.
  @param RemainingDevicePath A pointer to the EFI_DEVICE_PATH_PROTOCOL.
  @param NumberOfChildren    Children number.
  @param ChildHandleBuffer   A pointer to the child handle buffer.

  @retval EFI_NOT_READY   Device is not allocated.
  @retval EFI_UNSUPPORTED Device only support PCI-PCI bridge.
  @retval EFI_NOT_FOUND   Can not find the specific device.
  @retval EFI_SUCCESS     Success to start Pci devices on bridge.

**/
EFI_STATUS
StartPciDevicesOnBridge (
  IN EFI_HANDLE                          Controller,
  IN PCI_IO_DEVICE                       *RootBridge,
  IN EFI_DEVICE_PATH_PROTOCOL            *RemainingDevicePath,
  IN OUT UINT8                           *NumberOfChildren,
  IN OUT EFI_HANDLE                      *ChildHandleBuffer
  )

{
  PCI_IO_DEVICE             *Temp;
  PCI_IO_DEVICE             *PciIoDevice;
  EFI_DEV_PATH_PTR          Node;
  EFI_DEVICE_PATH_PROTOCOL  *CurrentDevicePath;
  EFI_STATUS                Status;
  LIST_ENTRY                *CurrentLink;
  UINT64                    Supports;

  PciIoDevice = NULL;
  CurrentLink = RootBridge->ChildList.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &RootBridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
    if (RemainingDevicePath != NULL) {

      Node.DevPath = RemainingDevicePath;

      if (Node.Pci->Device != Temp->DeviceNumber ||
          Node.Pci->Function != Temp->FunctionNumber) {
        CurrentLink = CurrentLink->ForwardLink;
        continue;
      }

      //
      // Check if the device has been assigned with required resource
      //
      if (!Temp->Allocated) {
        return EFI_NOT_READY;
      }

      //
      // Check if the current node has been registered before
      // If it is not, register it
      //
      if (!Temp->Registered) {
        PciIoDevice = Temp;

        Status = RegisterPciDevice (
                   Controller,
                   PciIoDevice,
                   NULL
                   );

      }

      if (NumberOfChildren != NULL && ChildHandleBuffer != NULL && Temp->Registered) {
        ChildHandleBuffer[*NumberOfChildren] = Temp->Handle;
        (*NumberOfChildren)++;
      }

      //
      // Get the next device path
      //
      CurrentDevicePath = NextDevicePathNode (RemainingDevicePath);
      if (IsDevicePathEnd (CurrentDevicePath)) {
        return EFI_SUCCESS;
      }

      //
      // If it is a PPB
      //
      if (!IsListEmpty (&Temp->ChildList)) {
        Status = StartPciDevicesOnBridge (
                   Controller,
                   Temp,
                   CurrentDevicePath,
                   NumberOfChildren,
                   ChildHandleBuffer
                   );

        Temp->PciIo.Attributes (
                      &(Temp->PciIo),
                      EfiPciIoAttributeOperationSupported,
                      0,
                      &Supports
                      );
        Supports &= (UINT64)EFI_PCI_DEVICE_ENABLE;
        Temp->PciIo.Attributes (
                      &(Temp->PciIo),
                      EfiPciIoAttributeOperationEnable,
                      Supports,
                      NULL
                      );

        return Status;
      } else {

        //
        // Currently, the PCI bus driver only support PCI-PCI bridge
        //
        return EFI_UNSUPPORTED;
      }

    } else {

      //
      // If remaining device path is NULL,
      // try to enable all the pci devices under this bridge
      //

      if (!Temp->Registered && Temp->Allocated) {

        PciIoDevice = Temp;

        Status = RegisterPciDevice (
                   Controller,
                   PciIoDevice,
                   NULL
                   );
      }

      if (NumberOfChildren != NULL && ChildHandleBuffer != NULL && Temp->Registered) {
        ChildHandleBuffer[*NumberOfChildren] = Temp->Handle;
        (*NumberOfChildren)++;
      }

      if (!IsListEmpty (&Temp->ChildList)) {
        Status = StartPciDevicesOnBridge (
                   Controller,
                   Temp,
                   RemainingDevicePath,
                   NumberOfChildren,
                   ChildHandleBuffer
                   );
      }
      //
      //  Enable Command register whether this bridge has device inserted or not
      //
      Temp->PciIo.Attributes (
                    &(Temp->PciIo),
                    EfiPciIoAttributeOperationSupported,
                    0,
                    &Supports
                    );
      Supports &= (UINT64)EFI_PCI_DEVICE_ENABLE;
      if (FeaturePcdGet(PcdNoExternalDmaSupported)) {
        //
        // For security reason, microsoft identifies the most dangerous devices as 1394, cardbus,
        // thunderbolt, and express-card. So, By default, disalbe these devices' bus master. 
        //
        if (IsUnsafeBusMasterDev (Temp)) {
          Supports &= ~(UINT64)EFI_PCI_IO_ATTRIBUTE_BUS_MASTER;
        }
      }
      Temp->PciIo.Attributes (
                    &(Temp->PciIo),
                    EfiPciIoAttributeOperationEnable,
                    Supports,
                    NULL
                    );

      CurrentLink = CurrentLink->ForwardLink;
      continue;
    }
  }

  if (PciIoDevice == NULL) {
    return EFI_NOT_FOUND;
  } else {
    return EFI_SUCCESS;
  }
}

/**
  Start to manage all the PCI devices it found previously under
  the entire host bridge.

  @param Controller          The root bridge handle.

  @retval EFI_NOT_READY   Device is not allocated.
  @retval EFI_SUCCESS     Success to start Pci device on host bridge.

**/
EFI_STATUS
StartPciDevices (
  IN EFI_HANDLE                         Controller
  )
{
  PCI_IO_DEVICE     *RootBridge;
  EFI_HANDLE        ThisHostBridge;
  LIST_ENTRY        *CurrentLink;

  RootBridge = GetRootBridgeByHandle (Controller);
  if (RootBridge == NULL) {
    ASSERT (RootBridge != NULL);
    return EFI_NOT_READY;
  }
  ThisHostBridge = RootBridge->PciRootBridgeIo->ParentHandle;

  CurrentLink = mPciDevicePool.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &mPciDevicePool) {

    RootBridge = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
    //
    // Locate the right root bridge to start
    //
    if (RootBridge->PciRootBridgeIo->ParentHandle == ThisHostBridge) {
      StartPciDevicesOnBridge (
        RootBridge->Handle,
        RootBridge,
        NULL,
        NULL,
        NULL
        );
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return EFI_SUCCESS;
}

/**
  Create root bridge device.

  @param RootBridgeHandle    Specified root bridge hanle.

  @return The crated root bridge device instance, NULL means no
          root bridge device instance created.

**/
PCI_IO_DEVICE *
CreateRootBridge (
  IN EFI_HANDLE                   RootBridgeHandle
  )
{
  EFI_STATUS                      Status;
  PCI_IO_DEVICE                   *Dev;
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;

  Dev = AllocateZeroPool (sizeof (PCI_IO_DEVICE));
  if (Dev == NULL) {
    return NULL;
  }

  Dev->Signature  = PCI_IO_DEVICE_SIGNATURE;
  Dev->Handle     = RootBridgeHandle;
  InitializeListHead (&Dev->ChildList);

  Status = gBS->OpenProtocol (
                  RootBridgeHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath,
                  gPciBusDriverBinding.DriverBindingHandle,
                  RootBridgeHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    FreePool (Dev);
    return NULL;
  }

  //
  // Record the root bridge parent device path
  //
  Dev->DevicePath = DuplicateDevicePath (ParentDevicePath);

  //
  // Get the pci root bridge io protocol
  //
  Status = gBS->OpenProtocol (
                  RootBridgeHandle,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  (VOID **) &PciRootBridgeIo,
                  gPciBusDriverBinding.DriverBindingHandle,
                  RootBridgeHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    FreePciDevice (Dev);
    return NULL;
  }

  Dev->PciRootBridgeIo = PciRootBridgeIo;

  //
  // Initialize the PCI I/O instance structure
  //
  InitializePciIoInstance (Dev);
  InitializePciDriverOverrideInstance (Dev);
  InitializePciLoadFile2 (Dev);

  //
  // Initialize reserved resource list and
  // option rom driver list
  //
  InitializeListHead (&Dev->ReservedResourceList);
  InitializeListHead (&Dev->OptionRomDriverList);

  return Dev;
}

/**
  Get root bridge device instance by specific root bridge handle.

  @param RootBridgeHandle    Given root bridge handle.

  @return The root bridge device instance, NULL means no root bridge
          device instance found.

**/
PCI_IO_DEVICE *
GetRootBridgeByHandle (
  EFI_HANDLE RootBridgeHandle
  )
{
  PCI_IO_DEVICE   *RootBridgeDev;
  LIST_ENTRY      *CurrentLink;

  CurrentLink = mPciDevicePool.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &mPciDevicePool) {

    RootBridgeDev = PCI_IO_DEVICE_FROM_LINK (CurrentLink);
    if (RootBridgeDev->Handle == RootBridgeHandle) {
      return RootBridgeDev;
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}

/**
  Judege whether Pci device existed.

  @param Bridge       Parent bridege instance.
  @param PciIoDevice  Device instance.

  @retval TRUE        Pci device existed.
  @retval FALSE       Pci device did not exist.

**/
BOOLEAN
PciDeviceExisted (
  IN PCI_IO_DEVICE    *Bridge,
  IN PCI_IO_DEVICE    *PciIoDevice
  )
{

  PCI_IO_DEVICE   *Temp;
  LIST_ENTRY      *CurrentLink;

  CurrentLink = Bridge->ChildList.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &Bridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp == PciIoDevice) {
      return TRUE;
    }

    if (!IsListEmpty (&Temp->ChildList)) {
      if (PciDeviceExisted (Temp, PciIoDevice)) {
        return TRUE;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return FALSE;
}

/**
  Get the active VGA device on the same segment.

  @param VgaDevice    PCI IO instance for the VGA device.

  @return The active VGA device on the same segment.

**/
PCI_IO_DEVICE *
ActiveVGADeviceOnTheSameSegment (
  IN PCI_IO_DEVICE        *VgaDevice
  )
{
  LIST_ENTRY      *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = mPciDevicePool.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &mPciDevicePool) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (Temp->PciRootBridgeIo->SegmentNumber == VgaDevice->PciRootBridgeIo->SegmentNumber) {

      Temp = ActiveVGADeviceOnTheRootBridge (Temp);

      if (Temp != NULL) {
        return Temp;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}

/**
  Get the active VGA device on the root bridge.

  @param RootBridge  PCI IO instance for the root bridge.

  @return The active VGA device.

**/
PCI_IO_DEVICE *
ActiveVGADeviceOnTheRootBridge (
  IN PCI_IO_DEVICE        *RootBridge
  )
{
  LIST_ENTRY      *CurrentLink;
  PCI_IO_DEVICE   *Temp;

  CurrentLink = RootBridge->ChildList.ForwardLink;

  while (CurrentLink != NULL && CurrentLink != &RootBridge->ChildList) {

    Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

    if (IS_PCI_VGA(&Temp->Pci) &&
        (Temp->Attributes &
         (EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY |
          EFI_PCI_IO_ATTRIBUTE_VGA_IO     |
          EFI_PCI_IO_ATTRIBUTE_VGA_IO_16)) != 0) {
      return Temp;
    }

    if (IS_PCI_BRIDGE (&Temp->Pci)) {

      Temp = ActiveVGADeviceOnTheRootBridge (Temp);

      if (Temp != NULL) {
        return Temp;
      }
    }

    CurrentLink = CurrentLink->ForwardLink;
  }

  return NULL;
}


/**
  Get HPC PCI address according to its device path.

  @param RootBridge           Root bridege Io instance.
  @param RemainingDevicePath  Given searching device path.
  @param PciAddress           Buffer holding searched result.

  @retval EFI_SUCCESS         PCI address was stored in PciAddress
  @retval EFI_NOT_FOUND       Can not find the specific device path.

**/
EFI_STATUS
GetHpcPciAddressFromRootBridge (
  IN  PCI_IO_DEVICE                    *RootBridge,
  IN  EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath,
  OUT UINT64                           *PciAddress
  )
{
  EFI_DEV_PATH_PTR          Node;
  PCI_IO_DEVICE             *Temp;
  EFI_DEVICE_PATH_PROTOCOL  *CurrentDevicePath;
  LIST_ENTRY                *CurrentLink;
  BOOLEAN                   MisMatch;

  MisMatch          = FALSE;

  CurrentDevicePath = RemainingDevicePath;
  Node.DevPath      = CurrentDevicePath;
  Temp              = NULL;

  while (!IsDevicePathEnd (CurrentDevicePath)) {

    CurrentLink   = RootBridge->ChildList.ForwardLink;
    Node.DevPath  = CurrentDevicePath;

    while (CurrentLink != NULL && CurrentLink != &RootBridge->ChildList) {
      Temp = PCI_IO_DEVICE_FROM_LINK (CurrentLink);

      if (Node.Pci->Device   == Temp->DeviceNumber &&
          Node.Pci->Function == Temp->FunctionNumber) {
        RootBridge = Temp;
        break;
      }

      CurrentLink = CurrentLink->ForwardLink;
    }

    //
    // Check if we find the bridge
    //
    if (CurrentLink == &RootBridge->ChildList) {

      MisMatch = TRUE;
      break;

    }

    CurrentDevicePath = NextDevicePathNode (CurrentDevicePath);
  }

  if (MisMatch) {

    CurrentDevicePath = NextDevicePathNode (CurrentDevicePath);

    if (IsDevicePathEnd (CurrentDevicePath)) {
      *PciAddress = EFI_PCI_ADDRESS (RootBridge->BusNumber, Node.Pci->Device, Node.Pci->Function, 0);
      return EFI_SUCCESS;
    }

    return EFI_NOT_FOUND;
  }

  if (Temp != NULL) {
    *PciAddress = EFI_PCI_ADDRESS (Temp->BusNumber, Temp->DeviceNumber, Temp->FunctionNumber, 0);
  } else {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;

}

BOOLEAN
IsUnsafeBusMasterDev (
  IN PCI_IO_DEVICE  *PciIoDevice
  )
{
  BOOLEAN                         UnsafeDev;
  BUS_MASTER_OVERRIDE             *BMOverrideEntry;   //retrieved from Pcd  
  //
  //init local
  //
  UnsafeDev = FALSE;
  BMOverrideEntry = NULL;
  
  if (!IS_PCI_BRIDGE (&PciIoDevice->Pci)) {
    if (IS_PCI_BRIDGE (&PciIoDevice->Parent->Pci) ||
        IS_CARDBUS_BRIDGE (&PciIoDevice->Pci) ||
        IS_PCI_1394 (&PciIoDevice->Pci)
        ) {
      //
      // This will guarantee 1394, cardbus, thuderbolt and express-card
      // have bus master disabled
      //
      UnsafeDev = TRUE;
    } 
  }

  //
  // Override BusMaster if needed. 
  //
  BMOverrideEntry = (BUS_MASTER_OVERRIDE *)PcdGetPtr (PcdNoExternalDmaOverrideTable);
  while (BMOverrideEntry->VendorID != 0xffff) {
    if (PciIoDevice->Pci.Hdr.VendorId == BMOverrideEntry->VendorID &&
        PciIoDevice->Pci.Hdr.DeviceId == BMOverrideEntry->DeviceID
        ) {
      if (BMOverrideEntry->BMSwitch == 0) {
        //
        // BusMaster Disabled, which means that this device is considered as not safe.
        //
        UnsafeDev = TRUE;
      } else {
        UnsafeDev = FALSE;
      }
    }
    BMOverrideEntry++;
  }
  
  return UnsafeDev;
}
