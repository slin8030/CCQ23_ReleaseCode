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

  MemoryPeim.c

Abstract:

  Tiano PEIM to provide the platform support functionality.
  This file implements the Platform Memory Range PPI

--*/

#include "PlatformInit.h"

//
// Need min. of 48MB PEI phase
//
#define  PEI_MIN_MEMORY_SIZE               (6 * 0x800000)
#define  PEI_RECOVERY_MIN_MEMORY_SIZE      (6 * 0x800000)

//
// This is the memory needed for PEI to start up DXE.
//
// Over-estimating this size will lead to higher fragmentation
// of main memory.  Under-estimation of this will cause catastrophic
// failure of PEI to load DXE.  Generally, the failure may only be
// realized during capsule updates.
//
#define PRERESERVED_PEI_MEMORY ( \
  EFI_SIZE_TO_PAGES (3 * 0x800000)   /* PEI Core memory based stack          */ \
  )

//[-start-160616-IB06720412-modify]//
EFI_MEMORY_TYPE_INFORMATION mDefaultMemoryTypeInformation[] = {
  { EfiACPIReclaimMemory,       0x40 },    // 0x40 pages = 256k for ASL
  { EfiACPIMemoryNVS,           0x60 },    // 0x100 pages = 1 MB for S3, SMM, HII, etc
  { EfiReservedMemoryType,      0x10 },    // 48k for BIOS Reserved
  { EfiMemoryMappedIO,          0     },
  { EfiMemoryMappedIOPortSpace, 0     },
  { EfiPalCode,                 0     },
  { EfiRuntimeServicesCode,     0x50  },
  { EfiRuntimeServicesData,     0x13B },
  { EfiLoaderCode,              0x100 },
  { EfiLoaderData,              0x100 },
  { EfiBootServicesCode,        0x800 },
  { EfiBootServicesData,        0x2500},
  { EfiConventionalMemory,      0     },
  { EfiUnusableMemory,          0     },
  { EfiMaxMemoryType,           0     }
};
//[-end-160616-IB06720412-modify]//

STATIC
EFI_STATUS
GetMemorySize (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  OUT UINT64              *LowMemoryLength,
  OUT UINT64              *HighMemoryLength
  );

STATIC
EFI_STATUS
GetMemorySize (
  IN  CONST EFI_PEI_SERVICES    **PeiServices,
  OUT UINT64              *LowMemoryLength,
  OUT UINT64              *HighMemoryLength
  )
{
  EFI_STATUS              Status;
  EFI_PEI_HOB_POINTERS    Hob;

  *HighMemoryLength = 0;
  *LowMemoryLength = 0x100000;
  //
  // Get the HOB list for processing
  //
  Status = (*PeiServices)->GetHobList (PeiServices, (VOID **) &Hob.Raw);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Collect memory ranges
  //
  while (!END_OF_HOB_LIST (Hob)) {
    if (Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        //
        // Need memory above 1MB to be collected here
        //
        if (Hob.ResourceDescriptor->PhysicalStart >= 0x100000 &&
            Hob.ResourceDescriptor->PhysicalStart < (EFI_PHYSICAL_ADDRESS) 0x100000000) {
          *LowMemoryLength += (UINT64) (Hob.ResourceDescriptor->ResourceLength);
        } else if (Hob.ResourceDescriptor->PhysicalStart >= (EFI_PHYSICAL_ADDRESS) 0x100000000) {
          *HighMemoryLength += (UINT64) (Hob.ResourceDescriptor->ResourceLength);
        }
      }
    }
    Hob.Raw = GET_NEXT_HOB (Hob);
  }

  return EFI_SUCCESS;
}


/*++

Routine Description:

  Publish Memory Type Information.

Arguments:

  NULL

Returns:

  EFI_SUCCESS  -  Success.
  Others       -  Errors have occurred.
--*/

EFI_STATUS
EFIAPI
PublishMemoryTypeInfo (
  void
  )
{
  EFI_STATUS                      Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *Variable;
  UINTN                           DataSize;
  EFI_MEMORY_TYPE_INFORMATION     MemoryData[EfiMaxMemoryType + 1];

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **) &Variable
             );
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "WARNING: Locating Pei variable failed 0x%x \n", Status));
    DEBUG((EFI_D_ERROR, "Build Hob from default\n"));
    //
    // Build the default GUID'd HOB for DXE
    //
    BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, mDefaultMemoryTypeInformation, sizeof (mDefaultMemoryTypeInformation) );

    return Status;
  }


  DataSize = sizeof (MemoryData);
  // This variable is saved in BDS stage. Now read it back
  Status = Variable->GetVariable (
                      Variable,
                      EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME,
                      &gEfiMemoryTypeInformationGuid,
                      NULL,
                      &DataSize,
                      &MemoryData
                      );
  if (EFI_ERROR (Status)) {
    //build default
    DEBUG((EFI_D_ERROR, "Build Hob from default\n"));
    BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, mDefaultMemoryTypeInformation, sizeof (mDefaultMemoryTypeInformation) );

  } else {
    // Build the GUID'd HOB for DXE from variable
    DEBUG((EFI_D_INFO, "Build Hob from variable \n"));
    BuildGuidDataHob (&gEfiMemoryTypeInformationGuid, MemoryData, DataSize);
  }

  return Status;
}

