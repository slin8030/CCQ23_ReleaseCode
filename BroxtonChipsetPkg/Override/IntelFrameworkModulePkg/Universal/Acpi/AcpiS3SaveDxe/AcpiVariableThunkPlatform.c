/** @file
  This is an implementation of the AcpiVariable platform field for ECP platform.

Copyright (c) 2006 - 2014, Intel Corporation. All rights reserved.<BR>

This program and the accompanying materials
are licensed and made available under the terms and conditions
of the BSD License which accompanies this distribution.  The
full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

==

typedef struct {
  EFI_PHYSICAL_ADDRESS  AcpiReservedMemoryBase;  <<===
  UINT32                AcpiReservedMemorySize;  <<===
  EFI_PHYSICAL_ADDRESS  S3ReservedLowMemoryBase;
  EFI_PHYSICAL_ADDRESS  AcpiBootScriptTable;
  EFI_PHYSICAL_ADDRESS  RuntimeScriptTableBase;
  EFI_PHYSICAL_ADDRESS  AcpiFacsTable;
  UINT64                SystemMemoryLength;      <<===
  ACPI_CPU_DATA_COMPATIBILITY         AcpiCpuData;
  EFI_PHYSICAL_ADDRESS  VideoOpromAddress;
  UINT32                VideoOpromSize;
  EFI_PHYSICAL_ADDRESS  S3DebugBufferAddress; 
  EFI_PHYSICAL_ADDRESS  S3ResumeNvsEntryPoint;    
} ACPI_VARIABLE_SET_COMPATIBILITY;

**/

//[-start-160216-IB03090424-modify]//
#include <FrameworkDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/DxeInsydeChipsetLib.h>
//#include <Library/CmosAccessLib.h>
#include <Library/CmosLib.h>
#include <Protocol/FrameworkMpService.h>
#include <Protocol/VariableLock.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Guid/AcpiS3Context.h>
#include "ChipsetCmos.h"
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <ChipsetSetupConfig.h>

GLOBAL_REMOVE_IF_UNREFERENCED
ACPI_VARIABLE_SET_COMPATIBILITY               *mAcpiVariableSetCompatibility = NULL;

/**
  Allocate memory below 4G memory address.

  This function allocates memory below 4G memory address.

  @param  MemoryType   Memory type of memory to allocate.
  @param  Size         Size of memory to allocate.
  
  @return Allocated address for output.

**/
VOID*
AllocateMemoryBelow4G (
  IN EFI_MEMORY_TYPE    MemoryType,
  IN UINTN              Size
  );

/**
  Hook point for AcpiVariableThunkPlatform for S3Ready.

  @param AcpiS3Context   ACPI s3 context
**/
VOID
S3ReadyThunkPlatform (
  IN ACPI_S3_CONTEXT      *AcpiS3Context
  )
{
  EFI_PHYSICAL_ADDRESS                          AcpiMemoryBase;
  UINT32                                        AcpiMemorySize;
  EFI_PEI_HOB_POINTERS                          Hob;
  UINT64                                        MemoryLength;

  DEBUG ((EFI_D_INFO, "S3ReadyThunkPlatform\n"));

  if (mAcpiVariableSetCompatibility == NULL) {
    return;
  }

  //
  // Allocate ACPI reserved memory under 4G
  //
  AcpiMemoryBase = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocateMemoryBelow4G (EfiReservedMemoryType, PcdGet32 (PcdS3AcpiReservedMemorySize));
  ASSERT (AcpiMemoryBase != 0);
  AcpiMemorySize = PcdGet32 (PcdS3AcpiReservedMemorySize);

  //
  // Calculate the system memory length by memory hobs
  //
  MemoryLength  = 0x100000;
  Hob.Raw = GetFirstHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR);
  ASSERT (Hob.Raw != NULL);
  while ((Hob.Raw != NULL) && (!END_OF_HOB_LIST (Hob))) {
    if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
      //
      // Skip the memory region below 1MB
      //
      if (Hob.ResourceDescriptor->PhysicalStart >= 0x100000) {
        MemoryLength += Hob.ResourceDescriptor->ResourceLength;
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
    Hob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, Hob.Raw);
  }

  mAcpiVariableSetCompatibility->AcpiReservedMemoryBase = AcpiMemoryBase;
  mAcpiVariableSetCompatibility->AcpiReservedMemorySize = AcpiMemorySize;
  mAcpiVariableSetCompatibility->SystemMemoryLength     = MemoryLength;

  DEBUG((EFI_D_INFO, "AcpiVariableThunkPlatform: AcpiMemoryBase is 0x%8x\n", mAcpiVariableSetCompatibility->AcpiReservedMemoryBase));
  DEBUG((EFI_D_INFO, "AcpiVariableThunkPlatform: AcpiMemorySize is 0x%8x\n", mAcpiVariableSetCompatibility->AcpiReservedMemorySize));
  DEBUG((EFI_D_INFO, "AcpiVariableThunkPlatform: SystemMemoryLength is 0x%8x\n", mAcpiVariableSetCompatibility->SystemMemoryLength));
  return ;
}

/**
  Register callback function upon VariableLockProtocol
  to lock ACPI_GLOBAL_VARIABLE variable to avoid malicious code to update it.

  @param[in] Event    Event whose notification function is being invoked.
  @param[in] Context  Pointer to the notification function's context.
**/
VOID
EFIAPI
VariableLockAcpiGlobalVariable (
  IN  EFI_EVENT                             Event,
  IN  VOID                                  *Context
  )
{

}

/**
  Hook point for AcpiVariableThunkPlatform for InstallAcpiS3Save.
**/
VOID
InstallAcpiS3SaveThunk (
  VOID
  )
{
  EFI_STATUS                           Status;
  FRAMEWORK_EFI_MP_SERVICES_PROTOCOL   *FrameworkMpService;
  UINTN                                VarSize;
  VOID                                 *Registration;
  CHIPSET_CONFIGURATION                SystemConfiguration;
//[-start-160803-IB07220122-add]//
  UINT32                               VariableAttributes;
//[-end-160803-IB07220122-add]//
  
  Status = gBS->LocateProtocol (
                  &gFrameworkEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID**) &FrameworkMpService
                  );
  if (!EFI_ERROR (Status)) {
    //
    // On ECP platform, if framework CPU drivers are in use, The compatible version of ACPI variable set 
    // should be produced by CPU driver. 
    //
    VarSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
    ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//     Status = GetSystemConfigData (&SystemConfiguration, &VarSize);
    Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    &VariableAttributes,
                    &VarSize,
                    &SystemConfiguration
                    ); 
//[-end-160803-IB07220122-modify]//
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe (&SystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
    }
    if (EFI_ERROR (Status) || (VarSize != sizeof (mAcpiVariableSetCompatibility))) { //BUG!BUG!
      DEBUG ((EFI_D_ERROR, "FATAL ERROR: AcpiVariableSetCompatibility was not saved by CPU driver correctly. OS S3 may fail!\n"));
      mAcpiVariableSetCompatibility = NULL;
    }
  } else {
    //
    // Allocate/initialize the compatible version of Acpi Variable Set since Framework chipset/platform 
    // driver need this variable. ACPI_GLOBAL_VARIABLE variable is not used in runtime phase,
    // so RT attribute is not needed for it.
    //
    mAcpiVariableSetCompatibility = AllocateMemoryBelow4G (EfiACPIMemoryNVS, sizeof(ACPI_VARIABLE_SET_COMPATIBILITY));

    VarSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
    ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//     Status = GetSystemConfigData (&SystemConfiguration, &VarSize);
    Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    &VariableAttributes,
                    &VarSize,
                    &SystemConfiguration
                    ); 
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe (&SystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Get setup variable default value failed! \n"));
		    return;
      }
    }

//[-end-160803-IB07220122-modify]//
    SystemConfiguration.AcpiVariableSetCompatibility = (UINTN)mAcpiVariableSetCompatibility;
//[-start-160824-IB07220130-add]//
    SystemConfiguration.NvStorageVariableBase = PcdGet32(PcdFlashNvStorageVariableBase);
//[-end-160824-IB07220130-add]//
//[-start-160803-IB07220122-modify]//
//     Status = SetSystemConfigData (&SystemConfiguration, PcdGet32 (PcdSetupConfigSize));
    Status = gRT->SetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    VariableAttributes,
                    VarSize,
                    &SystemConfiguration
                    );
//[-end-160803-IB07220122-modify]//

    if (!EFI_ERROR (Status)) {
      //
      // Register callback function upon VariableLockProtocol
      // to lock ACPI_GLOBAL_VARIABLE variable to avoid malicious code to update it.
      //
      EfiCreateProtocolNotifyEvent (
        &gEdkiiVariableLockProtocolGuid,
        TPL_CALLBACK,
        VariableLockAcpiGlobalVariable,
        NULL,
        &Registration
        );
    } else {
      DEBUG ((EFI_D_ERROR, "FATAL ERROR: AcpiVariableSetCompatibility cannot be saved: %r. OS S3 may fail!\n", Status));
      gBS->FreePages (
             (EFI_PHYSICAL_ADDRESS) (UINTN) mAcpiVariableSetCompatibility,
             EFI_SIZE_TO_PAGES (sizeof (ACPI_VARIABLE_SET_COMPATIBILITY))
             );
      mAcpiVariableSetCompatibility = NULL;
    }
  }

  DEBUG((EFI_D_INFO, "AcpiVariableSetCompatibility is 0x%8x\n", mAcpiVariableSetCompatibility));
}
//[-end-160216-IB03090424-modify]//
