/** @file
  EFI 2.0 PEIM for BIOS Reserved Memory Init.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include <Ppi/BiosReservedMemory.h>
#include <Ppi/MemoryDiscovered.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Guid/BiosReservedMemoryHob.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>

/**
  Entry point for the BIOS Reserved Memory Init PEIM.

  @param[in]  FileHandle   Handle of the file being invoked.
  @param[in]  PeiServices  General purpose services available to every PEIM.

  @retval  EFI_SUCCESS           Initialize reserved memory successfully.
  @retval  EFI_OUT_OF_RESOURCES  Reserved memory HOB is empty.
**/
EFI_STATUS
BiosReservedMemoryInitEntry (
  IN  EFI_PEI_FILE_HANDLE     FileHandle,
  IN  CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS                            Status;
  UINTN                                 BufferSize = 0;
  PEI_BIOS_RESERVED_MEMORY_POLICY_PPI   *BiosReservedMemoryPolicyPpi;
  BIOS_RESERVED_MEMORY_HOB              *BiosReservedMemoryHob = NULL;
  BIOS_RESERVED_MEMORY_CONFIG           *BiosReservedMemoryConfig = NULL;

  DEBUG ((DEBUG_INFO, "BiosReservedMemoryInitEntry() - Start\n"));

  BufferSize = sizeof (BIOS_RESERVED_MEMORY_HOB);
  BiosReservedMemoryHob = BuildGuidHob (&gBiosReserveMemoryHobGuid, BufferSize);
  if (BiosReservedMemoryHob == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

#ifdef PRAM_SUPPORT
  //
  // Assume Pram is disabled
  //
  BiosReservedMemoryHob->Pram = 0x30;
#endif


  //
  // Get Pram setting
  //
  BiosReservedMemoryConfig = (BIOS_RESERVED_MEMORY_CONFIG *) AllocateZeroPool (sizeof (BIOS_RESERVED_MEMORY_CONFIG));
  if (BiosReservedMemoryConfig != NULL) {
    Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gBiosReservedMemoryPolicyPpiGuid,
                                 0,
                                 NULL,
                                 &BiosReservedMemoryPolicyPpi
                                 );

    if (Status == EFI_SUCCESS) {
      Status = BiosReservedMemoryPolicyPpi->GetBiosReservedMemoryPolicy(PeiServices, BiosReservedMemoryPolicyPpi, BiosReservedMemoryConfig);
      if (Status == EFI_SUCCESS) {
#ifdef PRAM_SUPPORT
        BiosReservedMemoryHob->Pram = BiosReservedMemoryConfig->Pram;
#endif

      }
    }
  }
#ifdef PRAM_SUPPORT
  DEBUG((EFI_D_INFO , "BiosReservedMemoryHob->Pram = %x\n", BiosReservedMemoryHob->Pram));
#endif


  DEBUG ((DEBUG_INFO, "BiosReservedMemoryInitEntry() - End\n"));

  return EFI_SUCCESS;
}