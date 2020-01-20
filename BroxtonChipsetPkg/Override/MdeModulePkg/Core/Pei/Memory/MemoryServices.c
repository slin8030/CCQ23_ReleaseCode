/** @file
  EFI PEI Core memory services
  
Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include "PeiMain.h"

/**

  Initialize the memory services.

  @param PrivateData     Points to PeiCore's private instance data.
  @param SecCoreData     Points to a data structure containing information about the PEI core's operating
                         environment, such as the size and location of temporary RAM, the stack location and
                         the BFV location.
  @param OldCoreData     Pointer to the PEI Core data.
                         NULL if being run in non-permament memory mode.

**/
VOID
InitializeMemoryServices (
  IN PEI_CORE_INSTANCE           *PrivateData,
  IN CONST EFI_SEC_PEI_HAND_OFF  *SecCoreData,
  IN PEI_CORE_INSTANCE           *OldCoreData
  )
{
  
  PrivateData->SwitchStackSignal    = FALSE;

  //
  // First entering PeiCore, following code will initialized some field
  // in PeiCore's private data according to hand off data from sec core.
  //
  if (OldCoreData == NULL) {

    PrivateData->PeiMemoryInstalled = FALSE;
    PrivateData->HobList.Raw        = SecCoreData->PeiTemporaryRamBase;
    
    PeiCoreBuildHobHandoffInfoTable (
      BOOT_WITH_FULL_CONFIGURATION,
      (EFI_PHYSICAL_ADDRESS) (UINTN) SecCoreData->PeiTemporaryRamBase,
      (UINTN) SecCoreData->PeiTemporaryRamSize
      );

    //
    // Set Ps to point to ServiceTableShadow in Cache
    //
    PrivateData->Ps = &(PrivateData->ServiceTableShadow);
  }
  
  return;
}

/**

  This function registers the found memory configuration with the PEI Foundation.

  The usage model is that the PEIM that discovers the permanent memory shall invoke this service.
  This routine will hold discoveried memory information into PeiCore's private data,
  and set SwitchStackSignal flag. After PEIM who discovery memory is dispatched,
  PeiDispatcher will migrate temporary memory to permenement memory.
  
  @param PeiServices        An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param MemoryBegin        Start of memory address.
  @param MemoryLength       Length of memory.

  @return EFI_SUCCESS Always success.

**/
EFI_STATUS
EFIAPI
PeiInstallPeiMemory (
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN EFI_PHYSICAL_ADDRESS    MemoryBegin,
  IN UINT64                  MemoryLength
  )
{
  PEI_CORE_INSTANCE                     *PrivateData;

  DEBUG ((EFI_D_INFO, "PeiInstallPeiMemory MemoryBegin 0x%LX, MemoryLength 0x%LX\n", MemoryBegin, MemoryLength));
  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);

  //
  // PEI_SERVICE.InstallPeiMemory should only be called one time during whole PEI phase.
  // If it is invoked more than one time, ASSERT information is given for developer debugging in debug tip and
  // simply return EFI_SUCESS in release tip to ignore it.
  // 
  if (PrivateData->PeiMemoryInstalled) {
    DEBUG ((EFI_D_ERROR, "ERROR: PeiInstallPeiMemory is called more than once!\n"));
    ASSERT (PrivateData->PeiMemoryInstalled);
    return EFI_SUCCESS;
  }
  
  PrivateData->PhysicalMemoryBegin   = MemoryBegin;
  PrivateData->PhysicalMemoryLength  = MemoryLength;
  PrivateData->FreePhysicalMemoryTop = MemoryBegin + MemoryLength;
   
  PrivateData->SwitchStackSignal      = TRUE;

  return EFI_SUCCESS;   
}

//[-start-161208-IB07400824-add]//
#ifdef PEI_MEMORY_CONSISTENCY_CHECK

#include <Ppi/ReadOnlyVariable2.h>
//[-start-170303-IB07400845-add]//
#include <Library/IoLib.h>
#include <ScAccess.h>
#include <SaRegs.h>
//[-end-170303-IB07400845-add]//

extern EFI_GUID gChipsetPkgTokenSpaceGuid;
//[-start-170327-IB07400850-add]//
extern EFI_GUID gFdoModeEnabledHobGuid;
//[-end-170327-IB07400850-add]//

#define MAX_PEI_RUNTIME_MEMORY_TABLE  32

BOOLEAN
IsRtMemType (
  IN EFI_MEMORY_TYPE   Type
  )
{
  if (Type == EfiReservedMemoryType ||
      Type == EfiRuntimeServicesCode ||
      Type == EfiRuntimeServicesData ||
      Type == EfiACPIReclaimMemory ||
      Type == EfiACPIMemoryNVS
      ) {
    return TRUE;
  }

  return FALSE;
}

#pragma pack(1)

typedef struct {
  UINT8                 MemIndex;
  EFI_MEMORY_TYPE       MemType;
  UINT64                MemLength;
  EFI_PHYSICAL_ADDRESS  MemAddress;
} PEI_RUNTIME_MEMORY_TABLE;

#pragma pack()


EFI_STATUS
EFIAPI
PeiAllocatePagesByVariable (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN       EFI_MEMORY_TYPE      MemoryType,
  IN       UINTN                Pages,
  OUT      EFI_PHYSICAL_ADDRESS *Memory
  )
{
  EFI_STATUS                      Status;
  UINTN                           VariableSize;
  UINT32                          Attributes;
  UINTN                           Index;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
  EFI_BOOT_MODE                   BootMode;
  UINTN                           PeiMemoryRtTableSize;
  STATIC PEI_RUNTIME_MEMORY_TABLE PeiMemoryRtTable[MAX_PEI_RUNTIME_MEMORY_TABLE];
  STATIC UINT32                   PeiMemoryRtTableIndex = 0;
  STATIC BOOLEAN                  PeiMemoryRtTableExist = TRUE;
//[-start-170303-IB07400845-add]//
  STATIC UINT32                   TSegBase = 0;
//[-end-170303-IB07400845-add]//

  Status = PeiServicesGetBootMode (&BootMode);
  if (EFI_ERROR (Status) || (!EFI_ERROR (Status) && BootMode == BOOT_IN_RECOVERY_MODE)) {
    return EFI_NOT_FOUND;
  }
  
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  if (!PeiMemoryRtTableExist) {
    return EFI_NOT_FOUND;
  }
  if (PeiMemoryRtTableIndex == 0) {
    //
    // Get Varaible
    //
    PeiMemoryRtTableSize = MAX_PEI_RUNTIME_MEMORY_TABLE * sizeof (PEI_RUNTIME_MEMORY_TABLE);
    VariableSize = PeiMemoryRtTableSize;
    ZeroMem (PeiMemoryRtTable, VariableSize);
    Status = VariableServices->GetVariable (
      VariableServices,
      L"PeiRuntimeMemoryTable",
      &gChipsetPkgTokenSpaceGuid,
      &Attributes,
      &VariableSize,
      PeiMemoryRtTable
      );
    if (EFI_ERROR (Status)) {
      PeiMemoryRtTableExist = FALSE;
      return EFI_NOT_FOUND;
    }
  }

//[-start-170303-IB07400845-add]//
  if (TSegBase == 0) {
    TSegBase = (UINT32) (MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, R_SA_TSEG)) & 0xFFF00000);
    DEBUG ((EFI_D_ERROR, "TSegBase = %x\n", TSegBase));
  }
//[-end-170303-IB07400845-add]//

  //
  // Table Exist
  //
  for (Index = 0; Index < MAX_PEI_RUNTIME_MEMORY_TABLE; Index++) {
//[-start-170109-IB07400830-modify]//
//[-start-170303-IB07400845-modify]//
    if ((TSegBase != 0) && (PeiMemoryRtTable[Index].MemAddress >= TSegBase)) {
      //
      // If the Reserved Memroy Address > TSEG, this address is invalid.
      //
      DEBUG ((EFI_D_ERROR, "PEI runtime reserved memroy address in variable > TSEG, this address is invalid. !!\n"));
      PeiMemoryRtTableExist = FALSE;
      return EFI_NOT_FOUND;
    }
    if ((PeiMemoryRtTable[Index].MemIndex != 0) &&
        (MemoryType == PeiMemoryRtTable[Index].MemType) &&
        (Pages == EFI_SIZE_TO_PAGES (PeiMemoryRtTable[Index].MemLength))) {
      PeiMemoryRtTable[Index].MemIndex = 0;
      *Memory = PeiMemoryRtTable[Index].MemAddress;
      DEBUG ((EFI_D_ERROR, "PeiAllocatePagesByVariable, Index = %x, Type = %x, Addr = %08x\n", Index, MemoryType, *Memory));
      PeiMemoryRtTableIndex++;
      return EFI_SUCCESS;
    }
//[-end-170303-IB07400845-modify]//
//[-end-170109-IB07400830-modify]//
  }
  
  return EFI_NOT_FOUND;
}
#endif
//[-end-161208-IB07400824-add]//

/**
  The purpose of the service is to publish an interface that allows 
  PEIMs to allocate memory ranges that are managed by the PEI Foundation.

  @param  PeiServices      An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param  MemoryType       The type of memory to allocate.
  @param  Pages            The number of contiguous 4 KB pages to allocate.
  @param  Memory           Pointer to a physical address. On output, the address is set to the base 
                           of the page range that was allocated.

  @retval EFI_SUCCESS           The memory range was successfully allocated.
  @retval EFI_OUT_OF_RESOURCES  The pages could not be allocated.
  @retval EFI_INVALID_PARAMETER Type is not equal to EfiLoaderCode, EfiLoaderData, EfiRuntimeServicesCode, 
                                EfiRuntimeServicesData, EfiBootServicesCode, EfiBootServicesData,
                                EfiACPIReclaimMemory, EfiReservedMemoryType, or EfiACPIMemoryNVS.

**/
EFI_STATUS
EFIAPI
PeiAllocatePages (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN       EFI_MEMORY_TYPE      MemoryType,
  IN       UINTN                Pages,
  OUT      EFI_PHYSICAL_ADDRESS *Memory
  )
{
  PEI_CORE_INSTANCE                       *PrivateData;
  EFI_PEI_HOB_POINTERS                    Hob;
  EFI_PHYSICAL_ADDRESS                    *FreeMemoryTop;
  EFI_PHYSICAL_ADDRESS                    *FreeMemoryBottom;
  UINTN                                   RemainingPages;
//[-start-161208-IB07400824-add]//
//[-start-170327-IB07400850-modify]//
#ifdef PEI_MEMORY_CONSISTENCY_CHECK
  EFI_STATUS                              Status;
  EFI_HOB_GUID_TYPE                       *FdoEnabledGuidHob = NULL;
#endif  
//[-end-170327-IB07400850-modify]//
//[-end-161208-IB07400824-add]//

  if ((MemoryType != EfiLoaderCode) &&
      (MemoryType != EfiLoaderData) &&
      (MemoryType != EfiRuntimeServicesCode) &&
      (MemoryType != EfiRuntimeServicesData) &&
      (MemoryType != EfiBootServicesCode) &&
      (MemoryType != EfiBootServicesData) &&
      (MemoryType != EfiACPIReclaimMemory) &&
      (MemoryType != EfiReservedMemoryType) &&
      (MemoryType != EfiACPIMemoryNVS)) {
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);
  Hob.Raw     = PrivateData->HobList.Raw;
  
//[-start-161208-IB07400824-add]//
//[-start-170327-IB07400850-modify]//
#ifdef PEI_MEMORY_CONSISTENCY_CHECK
  FdoEnabledGuidHob = GetFirstGuidHob (&gFdoModeEnabledHobGuid);
  if (FdoEnabledGuidHob == NULL) {
    if (PrivateData->PeiMemoryInstalled) {
      if (IsRtMemType (MemoryType)) {
        Status = PeiAllocatePagesByVariable (PeiServices, MemoryType, Pages, Memory);
        if (!EFI_ERROR (Status)) {
//[-start-170106-IB07400830-add]//
          BuildMemoryAllocationHob (
            *(Memory),
            Pages * EFI_PAGE_SIZE,
            MemoryType
            );
//[-end-170106-IB07400830-add]//
          return Status;
        }
      }
    }
  }
#endif  
//[-end-170327-IB07400850-modify]//
//[-end-161208-IB07400824-add]//
  
  //
  // Check if Hob already available
  //
  if (!PrivateData->PeiMemoryInstalled) {
    //
    // When PeiInstallMemory is called but temporary memory has *not* been moved to temporary memory,
    // the AllocatePage will depend on the field of PEI_CORE_INSTANCE structure.
    //
    if (!PrivateData->SwitchStackSignal) {
      return EFI_NOT_AVAILABLE_YET;
    } else {
      FreeMemoryTop     = &(PrivateData->FreePhysicalMemoryTop);
      FreeMemoryBottom  = &(PrivateData->PhysicalMemoryBegin);
    }
  } else {
    FreeMemoryTop     = &(Hob.HandoffInformationTable->EfiFreeMemoryTop);
    FreeMemoryBottom  = &(Hob.HandoffInformationTable->EfiFreeMemoryBottom);
  }

  //
  // Check to see if on 4k boundary, If not aligned, make the allocation aligned.
  //
  *(FreeMemoryTop) -= *(FreeMemoryTop) & 0xFFF;

  //
  // Verify that there is sufficient memory to satisfy the allocation.
  // For page allocation, the overhead sizeof (EFI_HOB_MEMORY_ALLOCATION) needs to be considered.
  //
  if ((UINTN) (*FreeMemoryTop - *FreeMemoryBottom) < (UINTN) ALIGN_VALUE (sizeof (EFI_HOB_MEMORY_ALLOCATION), 8)) {
    DEBUG ((EFI_D_ERROR, "AllocatePages failed: No space to build memory allocation hob.\n"));
    return  EFI_OUT_OF_RESOURCES;
  }
  RemainingPages = (UINTN)(*FreeMemoryTop - *FreeMemoryBottom - ALIGN_VALUE (sizeof (EFI_HOB_MEMORY_ALLOCATION), 8)) >> EFI_PAGE_SHIFT;
  //
  // The number of remaining pages needs to be greater than or equal to that of the request pages.
  //
  if (RemainingPages < Pages) {
    DEBUG ((EFI_D_ERROR, "AllocatePages failed: No 0x%lx Pages is available.\n", (UINT64) Pages));
    DEBUG ((EFI_D_ERROR, "There is only left 0x%lx pages memory resource to be allocated.\n", (UINT64) RemainingPages));
    return  EFI_OUT_OF_RESOURCES;
  } else {
    //
    // Update the PHIT to reflect the memory usage
    //
    *(FreeMemoryTop) -= Pages * EFI_PAGE_SIZE;

    //
    // Update the value for the caller
    //
    *Memory = *(FreeMemoryTop);

    //
    // Create a memory allocation HOB.
    //
    BuildMemoryAllocationHob (
      *(FreeMemoryTop),
      Pages * EFI_PAGE_SIZE,
      MemoryType
      );

    return EFI_SUCCESS;
  }
}

/**

  Pool allocation service. Before permenent memory is discoveried, the pool will 
  be allocated the heap in the temporary memory. Genenrally, the size of heap in temporary 
  memory does not exceed to 64K, so the biggest pool size could be allocated is 
  64K.

  @param PeiServices               An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation.
  @param Size                      Amount of memory required
  @param Buffer                    Address of pointer to the buffer

  @retval EFI_SUCCESS              The allocation was successful
  @retval EFI_OUT_OF_RESOURCES     There is not enough heap to satisfy the requirement
                                   to allocate the requested size.

**/
EFI_STATUS
EFIAPI
PeiAllocatePool (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN       UINTN                Size,
  OUT      VOID                 **Buffer
  )
{
  EFI_STATUS               Status;
  EFI_HOB_MEMORY_POOL      *Hob;

  //
  // If some "post-memory" PEIM wishes to allocate larger pool,
  // it should use AllocatePages service instead.
  //
  
  //
  // Generally, the size of heap in temporary memory does not exceed to 64K,
  // HobLength is multiples of 8 bytes, so the maxmium size of pool is 0xFFF8 - sizeof (EFI_HOB_MEMORY_POOL)
  //
  if (Size > (0xFFF8 - sizeof (EFI_HOB_MEMORY_POOL))) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = PeiServicesCreateHob (
             EFI_HOB_TYPE_MEMORY_POOL,
             (UINT16)(sizeof (EFI_HOB_MEMORY_POOL) + Size),
             (VOID **)&Hob
             );
  ASSERT_EFI_ERROR (Status);
  *Buffer = Hob+1;  

  return Status;
}
