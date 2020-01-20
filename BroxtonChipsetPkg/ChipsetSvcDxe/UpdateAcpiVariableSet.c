/** @file
 DXE Chipset Services Library.
	
 Update ACPI variable set called upon S3 ready. Chipset can use it to update the content 
 of customized ACPI_VARIABLE_SET variable to compliant with specific chipset RC

;***************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <FrameworkDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FrameworkMpService.h>
#include <Guid/AcpiVariableCompatibility.h>

/**
  Allocate EfiACPIMemoryNVS below 4G memory address.

  This function allocates EfiACPIMemoryNVS below 4G memory address.

  @param  Size         Size of memory to allocate.
  
  @return Allocated address for output.

**/
VOID*
AllocateAcpiNvsMemoryBelow4G (
  IN   UINTN   Size
  )
{
  UINTN                 Pages;
  EFI_PHYSICAL_ADDRESS  Address;
  EFI_STATUS            Status;
  VOID*                 Buffer;

  Pages = EFI_SIZE_TO_PAGES (Size);
  Address = 0xffffffff;

  Status  = gBS->AllocatePages (
                   AllocateMaxAddress,
                   EfiACPIMemoryNVS,
                   Pages,
                   &Address
                   );
  ASSERT_EFI_ERROR (Status);

  Buffer = (VOID *) (UINTN) Address;
  ZeroMem (Buffer, Size);

  return Buffer;
}

/**
 Update ACPI variable set called upon S3 ready. Chipset can use it to update the content 
 of customized ACPI_VARIABLE_SET variable to compliant with specific chipset RC

 @param[in]         S3AcpiReservedMemorySize    The size of S3AcpiReservedMemory

 @retval            EFI_SUCCESS         Function returns successfully
 @retval            EFI_UNSUPPORTED     Use ordinary path to update standard ACPI_VARIABLE_SET content
*/
EFI_STATUS
EFIAPI
UpdateAcpiVariableSet (
  IN BOOLEAN     IsS3Ready,
  IN UINT32      S3AcpiReservedMemorySize
  )
{
  EFI_STATUS                           Status;
  UINTN                                VarSize;
  ACPI_VARIABLE_SET_COMPATIBILITY      *AcpiVariableSetCompatibility;
  FRAMEWORK_EFI_MP_SERVICES_PROTOCOL   *FrameworkMpService;
  EFI_PEI_HOB_POINTERS                 Hob;
  UINT64                               MemoryLength;
  UINT64                               MemoryLengthAbove4G;

  if (!IsS3Ready) {
    //
    // Now in initial state, see is it need to create new AcpiVariableSet based on ECP MpService protocol
    //
    Status = gBS->LocateProtocol (
                    &gFrameworkEfiMpServiceProtocolGuid,
                    NULL,
                    (VOID**) &FrameworkMpService
                    );
    if (EFI_ERROR (Status)) {
      //
      // Allocate/initialize the compatible version of Acpi Variable Set since Framework chipset/platform 
      // driver need this variable
      //
      AcpiVariableSetCompatibility = AllocateAcpiNvsMemoryBelow4G (sizeof(ACPI_VARIABLE_SET_COMPATIBILITY));
      Status = gRT->SetVariable (
                      ACPI_GLOBAL_VARIABLE,
                      &gEfiAcpiVariableCompatiblityGuid,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                      sizeof(ACPI_VARIABLE_SET_COMPATIBILITY*),
                      &AcpiVariableSetCompatibility
                      );
    }
  } else {
    //
    // Now in S3 ready state, update related fields in AcpiVariableSetCompatibility for use of S3 resume
    //
    AcpiVariableSetCompatibility = NULL;
    VarSize = sizeof (ACPI_VARIABLE_SET_COMPATIBILITY*);
    Status = gRT->GetVariable (
                    ACPI_GLOBAL_VARIABLE,
                    &gEfiAcpiVariableCompatiblityGuid,
                    NULL,
                    &VarSize,
                    &AcpiVariableSetCompatibility
                    );
    if (EFI_ERROR (Status) || AcpiVariableSetCompatibility == NULL) {
      //
      // Unable to get ACPI variable set, quit directly
      //
      return EFI_UNSUPPORTED;
    }
    //
    // Allocate ACPI reserved memory under 4G
    //
    AcpiVariableSetCompatibility->AcpiReservedMemoryBase = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocateAcpiNvsMemoryBelow4G (S3AcpiReservedMemorySize);
    if (AcpiVariableSetCompatibility->AcpiReservedMemoryBase == 0) {
      return EFI_UNSUPPORTED;
    }
    AcpiVariableSetCompatibility->AcpiReservedMemorySize = S3AcpiReservedMemorySize;
    //
    // Calculate the system memory length by memory hobs
    //
    MemoryLength  = 0x100000;
    MemoryLengthAbove4G  = 0;
    Hob.Raw = GetFirstHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR);
    ASSERT (Hob.Raw != NULL);
    while ((Hob.Raw != NULL) && (!END_OF_HOB_LIST (Hob))) {
      if (Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
        if (Hob.ResourceDescriptor->PhysicalStart >= 0x100000000) {
          //
          // Currently drop the MemoryLengthAbove4G size, because it is not used in current code.
          // Will check to see how this is going to be implemented later.
          //
          MemoryLengthAbove4G += Hob.ResourceDescriptor->ResourceLength;
        }
        //
        // Skip the memory region below 1MB
        //
        else if (Hob.ResourceDescriptor->PhysicalStart >= 0x100000) {
          MemoryLength += Hob.ResourceDescriptor->ResourceLength;
        }
      }
      Hob.Raw = GET_NEXT_HOB (Hob);
      Hob.Raw = GetNextHob (EFI_HOB_TYPE_RESOURCE_DESCRIPTOR, Hob.Raw);
    }
    AcpiVariableSetCompatibility->SystemMemoryLength         = MemoryLength;

    DEBUG((EFI_D_INFO, "AcpiVariableThunkPlatform: AcpiMemoryBase is 0x%8x\n", AcpiVariableSetCompatibility->AcpiReservedMemoryBase));
    DEBUG((EFI_D_INFO, "AcpiVariableThunkPlatform: AcpiMemorySize is 0x%8x\n", AcpiVariableSetCompatibility->AcpiReservedMemorySize));
    DEBUG((EFI_D_INFO, "AcpiVariableThunkPlatform: SystemMemoryLength is 0x%8x\n", AcpiVariableSetCompatibility->SystemMemoryLength));
  }
  return EFI_SUCCESS;
}
