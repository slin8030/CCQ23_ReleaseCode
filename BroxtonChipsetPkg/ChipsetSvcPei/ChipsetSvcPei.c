/** @file
 PEI Chipset Services driver. 
 
 It produces an instance of the PEI Chipset Services PPI to provide the chipset related functions 
 which will be used by Kernel or Project code. These PPI functions should be used by calling the 
 corresponding functions in PeiChipsetSvcLib to have the protocol size checking

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
#include <PiPei.h>
#include <Pi/PIPeiCis.h>

//
// Libraries
//
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>


//
// Produced Ppi
//
#include <Ppi/H2OPeiChipsetServices.h>



//
// Structure for H2O Chipsete Services PPI Descriptor
//
typedef struct {
  EFI_PEI_PPI_DESCRIPTOR    PpiDescriptor;
  H2O_CHIPSET_SERVICES_PPI  H2OChipsetSvcPpi;
} H2O_CHIPSET_SERVICES_PPI_DESCRIPTOR;
//
//function Prototypes
//



/**
 Platform initialization in PEI phase stage 1.

 @param[in]         None

 @retval            EFI_SUCCESS         This function alway return successfully
*/
EFI_STATUS
PlatformStage1Init (
VOID
  );
  
/**
 Install Firmware Volume Hob's once there is main memory

 @param[in]         BootMode            Boot mode

 @retval            EFI_SUCCESS         EFI_SUCCESS if the interface could be successfully installed
*/
EFI_STATUS
Stage1MemoryDiscoverCallback (
  IN  EFI_BOOT_MODE       *BootMode
  );  


/**
 Platform initialization in PEI phase stage 2.

 @param[in]         None

 @retval            EFI_SUCCESS         This function alway return successfully
*/
EFI_STATUS
PlatformStage2Init(
VOID
  );


/**
 Install Firmware Volume Hob's once there is main memory

 @param[in]         BootMode            Boot mode

 @retval            EFI_SUCCESS         EFI_SUCCESS if the interface could be successfully installed
*/
EFI_STATUS
Stage2MemoryDiscoverCallback (
  IN  EFI_BOOT_MODE       *BootMode
  );

/**
 Platform specific function to enable/disable flash device write access.

 @param[in]         EnableWrites        TRUE  - Enable
                                        FALSE - Disable
                    
 @retval            EFI_SUCCESS         Function returns successfully
*/
EFI_STATUS
EFIAPI
EnableFdWrites (
  IN  BOOLEAN           EnableWrites
  );

/**
 To identify sleep state.

 @param[in, out]    SleepState          3 - This is an S3 restart
                                        4 - This is an S4 restart

 @retval            EFI_SUCCESS         Function always returns successfully
*/
EFI_STATUS
GetSleepState (
  IN OUT  UINT8              *SleepState
  );

EFI_STATUS
HdaInitHook (
  IN UINT32                         HdaBar
  );

EFI_STATUS
SetPlatformHardwareSwitch (
  OUT PLATFORM_HARDWARE_SWITCH          **PlatformHardwareSwitchDptr
  );


/**
 This is the declaration of PEI Chipset Services PEIM entry point. 
 
 @param[in]         FileHandle          Handle of the file being invoked.
 @param[in]         PeiServices         Describes the list of possible PEI Services.
                    
 @retval            EFI_SUCCESS         The operation completed successfully.
 @retval            Others              An unexpected error occurred.
*/
EFI_STATUS
EFIAPI
ChipsetSvcPeiEntryPoint (
  IN EFI_PEI_FILE_HANDLE         FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS                            Status;
  UINT32                                Size;
  H2O_CHIPSET_SERVICES_PPI_DESCRIPTOR  *ChipsetSvcPpiDescriptor;
  EFI_PEI_PPI_DESCRIPTOR               *PpiDescriptor;
  VOID                                 *Ppi;

  PpiDescriptor = NULL;
  Ppi = NULL;

  //
  // Register so it will be automatically shadowed to memory
  //
  //
  // Create an instance of the H2O Chipset Services ppi,
  //
  ChipsetSvcPpiDescriptor = AllocateZeroPool (sizeof (H2O_CHIPSET_SERVICES_PPI_DESCRIPTOR));
  if (ChipsetSvcPpiDescriptor == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Size = sizeof (H2O_CHIPSET_SERVICES_PPI);
  if (Size < sizeof (UINT32)) {         // must at least contain Size field.
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Build ChipsetSvcPpi Private Data
  //
  ChipsetSvcPpiDescriptor->PpiDescriptor.Flags   = (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  ChipsetSvcPpiDescriptor->PpiDescriptor.Guid    = &gH2OChipsetServicesPpiGuid;
  ChipsetSvcPpiDescriptor->PpiDescriptor.Ppi     = &ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi;
  ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.Size = Size;

  //
  // Populate all functions based on Size.
  //
   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, EnableFdWrites) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.EnableFdWrites = EnableFdWrites;
   }
//   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, LegacyRegionAccessCtrl) + sizeof (VOID*))) {
//     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.LegacyRegionAccessCtrl = LegacyRegionAccessCtrl;
//   }
   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, GetSleepState) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.GetSleepState = GetSleepState;
   }

#if ENBDT_PF_ENABLE   
   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, HdaInitHook) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.HdaInitHook = HdaInitHook;
   }
#endif

   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, PlatformStage1Init) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.PlatformStage1Init = PlatformStage1Init;
   }
   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, Stage1MemoryDiscoverCallback) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.Stage1MemoryDiscoverCallback = Stage1MemoryDiscoverCallback;
   }

   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, PlatformStage2Init) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.PlatformStage2Init = PlatformStage2Init;
   }

#if ENBDT_PF_ENABLE      
   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, SetPlatformHardwareSwitch) + sizeof (VOID*))) {
    ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.SetPlatformHardwareSwitch = SetPlatformHardwareSwitch;
   }     
#endif   
   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, Stage2MemoryDiscoverCallback) + sizeof (VOID*))) {
     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.Stage2MemoryDiscoverCallback = Stage2MemoryDiscoverCallback;
   }
   
//   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, ModifyMemoryRange) + sizeof (VOID*))) {
//     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.ModifyMemoryRange = ModifyMemoryRange;
//   }
//   if (Size >= (OFFSET_OF (H2O_CHIPSET_SERVICES_PPI, SetSetupVariableDefault) + sizeof (VOID*))) {
//     ChipsetSvcPpiDescriptor->H2OChipsetSvcPpi.SetSetupVariableDefault = SetSetupVariableDefault;
//   }

  Status = (**PeiServices).InstallPpi (
             PeiServices, 
             &ChipsetSvcPpiDescriptor->PpiDescriptor
             );

//  Status = PeiServicesLocatePpi (
//             &gH2OChipsetServicesPpiGuid,
//             0,
//             &PpiDescriptor,
//             &Ppi
//             );
//  ASSERT_EFI_ERROR (Status);             
//  Status = PeiServicesReInstallPpi (
//             PpiDescriptor, 
//             &ChipsetSvcPpiDescriptor->PpiDescriptor
//             );
  
  return Status;
}

