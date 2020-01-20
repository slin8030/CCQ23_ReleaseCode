/** @file
 DXE Chipset Services driver. 

 It produces an instance of the DXE Chipset Services protocol to provide the chipset related functions 
 which will be used by Kernel or Project code. These protocol functions should be used by calling the 
 corresponding functions in DxeChipsetSvcLib to have the protocol size checking

***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <PiDxe.h>

//
// Libraries
//
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>

//
// UEFI Driver Model Protocols
//
#include <Protocol/DriverSupportedEfiVersion.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/ComponentName.h>
#include <Protocol/H2ODxeChipsetServices.h>

//
// Protocol instances
//
extern EFI_COMPONENT_NAME2_PROTOCOL  gChipsetSvcDxeComponentName2;
extern EFI_COMPONENT_NAME_PROTOCOL   gChipsetSvcDxeComponentName;

//
// Driver Support EFI Version Protocol instance
//
GLOBAL_REMOVE_IF_UNREFERENCED 
EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL gChipsetSvcDxeDriverSupportedEfiVersion = {
  sizeof (EFI_DRIVER_SUPPORTED_EFI_VERSION_PROTOCOL),
  0x0002001E
};

H2O_CHIPSET_SERVICES_PROTOCOL *mDxeChipsetSvc = NULL;

//
// ChipsetSvcDxe function pototype
//
EFI_STATUS
IrqRoutingInformation (
  OUT LEGACY_MODIFY_PIR_TABLE             **VirtualBusTablePtr,
  OUT UINT8                               *VirtualBusTableEntryNumber,
  OUT EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY **IrqPoolTablePtr,
  OUT UINT8                               *IrqPoolTableNumber,
  OUT UINT8                               **PirqPriorityTablePtr,
  OUT UINT8                               *PirqPriorityTableEntryNumber,
  OUT IRQ_ROUTING_TABLE                   **IrqRoutingTableInfoPtr
);

EFI_STATUS
SkipGetPciRom (
  IN  UINTN               Segment,
  IN  UINTN               Bus,
  IN  UINTN               Device,
  IN  UINTN               Function,
  IN  UINT16              VendorId,
  IN  UINT16              DeviceId,
  OUT BOOLEAN            *SkipGetPciRom
  );

EFI_STATUS
InstallPciRomSwitchTextMode (
  IN      EFI_HANDLE      DeviceHandle,
  IN OUT  BOOLEAN        *ForceSwitchTextMode
  );

EFI_STATUS
ProgramChipsetSsid (
  IN UINT8                               Bus,
  IN UINT8                               Dev,
  IN UINT8                               Func,
  IN UINT16                              VendorId,
  IN UINT16                              DeviceId,
  IN UINT32                              SsidSvid
  );

EFI_STATUS
SataComReset (
  IN  UINTN         PortNumber
  );

/**
 Get information of recovery FV.

 @param[out]        RecoveryRegion        recovery region table
 @param[out]        NumberOfRegion        number of recovery region
 @param[out]        ChipsetEmuPeiMaskList Specified PEIMs don't to dispatch in recovery mode
                    
 @retval            EFI_SUCCESS           This function always return success.
**/
EFI_STATUS
FvRecoveryInfo (
  OUT  PEI_FV_DEFINITION       **RecoveryRegion,
  OUT  UINTN                    *NumberOfRegion,
  OUT  EFI_GUID                **ChipsetEmuPeiMaskList
  );

EFI_STATUS
GetBiosProtectTable (
  OUT BIOS_PROTECT_REGION           **BiosRegionTable,
  OUT UINT8                         *ProtectRegionNum
  );
  
EFI_STATUS
SetIsaInfo (
  IN OUT UINT8           *Function,
  IN OUT UINT8           *Device
  );

EFI_STATUS
EFIAPI
UpdateAcpiVariableSet (
  IN BOOLEAN     IsS3Ready,
  IN UINT32      S3AcpiReservedMemorySize
  );

/**
 Unloads an image.

 @param[in]         ImageHandle           Handle that identifies the image to be unloaded.
                    
 @retval            EFI_SUCCESS           The image has been unloaded.
 @retval            EFI_INVALID_PARAMETER One of the protocol interfaces was not previously installed on ImageHandle.
**/
EFI_STATUS 
EFIAPI
ChipsetSvcDxeUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  //
  // Uninstall Driver Supported EFI Version Protocol from the image handle.
  //
  Status = gBS->UninstallMultipleProtocolInterfaces (
                  ImageHandle,
                  &gEfiDriverSupportedEfiVersionProtocolGuid, &gChipsetSvcDxeDriverSupportedEfiVersion,
                  &gH2OChipsetServicesProtocolGuid, mDxeChipsetSvc,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Do any additional cleanup that is required for this driver
  //
  FreePool (mDxeChipsetSvc);
  mDxeChipsetSvc = NULL;
  
  return EFI_SUCCESS;
}

/**
This is the declaration of DXE Chipset Services driver entry point. 

 @param[in]         ImageHandle         The firmware allocated handle for the UEFI image.
 @param[in]         SystemTable         A pointer to the EFI System Table.

 @retval            EFI_SUCCESS         The operation completed successfully.
 @retval            !EFI_SUCCESS        Error to installing Chipset Services Protocol.
*/
EFI_STATUS
EFIAPI
ChipsetSvcDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;
  UINT32      Size;

  Status = EFI_SUCCESS;

  //
  // Install Driver Component Name Protocol onto ImageHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiComponentNameProtocolGuid, &gChipsetSvcDxeComponentName,
                  &gEfiComponentName2ProtocolGuid, &gChipsetSvcDxeComponentName2,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
 
  //
  // Install Driver Supported EFI Version Protocol onto ImageHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiDriverSupportedEfiVersionProtocolGuid, &gChipsetSvcDxeDriverSupportedEfiVersion,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  
  //
  // Create an instance of the H2O Chipset Services protocol.
  // Then install it on the image handle.
  //
  
  Size = sizeof (H2O_CHIPSET_SERVICES_PROTOCOL);
  if (Size < sizeof (UINT32)) {         // must at least contain Size field.
    return EFI_INVALID_PARAMETER;
  }
  
  mDxeChipsetSvc = AllocateZeroPool (Size);
  if (mDxeChipsetSvc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  mDxeChipsetSvc->Size = Size;

  //
  // Now, populate functions based on Size.
  //

  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, ProgramChipsetSsid) + sizeof (VOID*))) {
    mDxeChipsetSvc->ProgramChipsetSsid = ProgramChipsetSsid;
  }
   
  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, SetIsaInfo) + sizeof (VOID*))) {
    mDxeChipsetSvc->SetIsaInfo = SetIsaInfo;
  }
  
#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, SataComReset) + sizeof (VOID*))) {
    mDxeChipsetSvc->SataComReset = SataComReset;
  }

  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, InstallPciRomSwitchTextMode) + sizeof (VOID*))) {
    mDxeChipsetSvc->InstallPciRomSwitchTextMode = InstallPciRomSwitchTextMode;
  }

  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, SkipGetPciRom) + sizeof (VOID*))) {
    mDxeChipsetSvc->SkipGetPciRom = SkipGetPciRom;
  }
   
  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, IrqRoutingInformation) + sizeof (VOID*))) {
    mDxeChipsetSvc->IrqRoutingInformation = IrqRoutingInformation;
  }
#endif

  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, FvRecoveryInfo) + sizeof (VOID*))) {
    mDxeChipsetSvc->FvRecoveryInfo = FvRecoveryInfo;
  }
  
  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, GetBiosProtectTable) + sizeof (VOID*))) {
    mDxeChipsetSvc->GetBiosProtectTable = GetBiosProtectTable;
  }

  if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PROTOCOL, UpdateAcpiVariableSet) + sizeof (VOID*))) {
    mDxeChipsetSvc->UpdateAcpiVariableSet = UpdateAcpiVariableSet;
  }
  
  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gH2OChipsetServicesProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  mDxeChipsetSvc
                  );
 
  return Status;
}

