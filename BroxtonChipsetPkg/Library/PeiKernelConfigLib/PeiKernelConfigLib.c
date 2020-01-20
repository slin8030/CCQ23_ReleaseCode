/** @file
 Library Instance for PeiKernelConfigLib Library Class.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <KernelSetupConfig.h>


/**
 Retrieve kernel setup configuration data

 @param[out] KernelConfig       Pointer to the structure of KERNEL_CONFIGURATION,
                                this pointer must be allocated with sizeof(KERNEL_CONFIGURATION)
                                before being called

 @retval EFI_SUCCESS            The kernel configuration is successfully retrieved
 @retval EFI_INVALID_PARAMETER  NULL pointer for input KernelConfig paramater
 @return others                 Failed to retrieve kernel configuration
**/
EFI_STATUS
EFIAPI
GetKernelConfiguration (
  OUT KERNEL_CONFIGURATION         *KernelConfig
  )
{
  EFI_STATUS                       Status;
  VOID                             *SystemConfiguration;
  UINTN                            Size;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *VariablePpi;

  if (KernelConfig == NULL) {
    ASSERT_EFI_ERROR (KernelConfig != NULL);
    return EFI_INVALID_PARAMETER;
  }

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **) &VariablePpi
           );
  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  Size = PcdGet32 (PcdSetupConfigSize);
  SystemConfiguration = AllocatePool (Size);
  ASSERT (SystemConfiguration != NULL);

  Status = VariablePpi->GetVariable (
                          VariablePpi,
                          SETUP_VARIABLE_NAME,
                          &gSystemConfigurationGuid,
                          NULL,
                          &Size,
                          SystemConfiguration
                          );
  if (Status == EFI_SUCCESS) {
    CopyMem (KernelConfig, SystemConfiguration, sizeof(KERNEL_CONFIGURATION));
  }
  FreePool (SystemConfiguration);
  return Status;
}

/**
 Retrieve the size of Setup variable

 @return                        Size of the setup variable data
**/
UINTN
EFIAPI
GetSetupVariableSize (
  VOID
  )
{
  return (UINTN)PcdGet32 (PcdSetupConfigSize);
}

