/** @file
  
;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <AzaliaPolicyPei.h>

EFI_STATUS
EFIAPI
AzaliaPolicyPeiEntry (
  IN      EFI_PEI_FILE_HANDLE        FileHandle,
  IN      CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                        Status;
  CHIPSET_CONFIGURATION             SystemConfiguration;
  VOID                              *FullSystemConfig;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *Variable;
  UINTN                             VariableSize;
  EFI_GUID                          SystemConfigurationGuid = SYSTEM_CONFIGURATION_GUID;
  EFI_PEI_PPI_DESCRIPTOR            *PchAzaliaPolicyPpiDesc;
  AZALIA_POLICY                     *PchAzaliaPolicyPpi;
  UINT8                             Index;

  Variable                = NULL;
  VariableSize            = 0;
  PchAzaliaPolicyPpiDesc  = NULL;
  PchAzaliaPolicyPpi      = NULL;
  Index                   = 0;

  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "\nAzalia Policy PEI Entry\n"));

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariable2PpiGuid,
             0,
             NULL,
             (VOID **)&Variable
             );
  ASSERT_EFI_ERROR (Status);

  VariableSize = PcdGet32 (PcdSetupConfigSize);
  FullSystemConfig = AllocatePool (VariableSize);
  ASSERT (FullSystemConfig != NULL);
  
  Status = Variable->GetVariable (
             Variable,
             L"Setup",
             &SystemConfigurationGuid,
             NULL,
             &VariableSize,
             FullSystemConfig
             );
  ASSERT_EFI_ERROR (Status);

  ASSERT (sizeof(CHIPSET_CONFIGURATION) <= VariableSize);
  CopyMem (&SystemConfiguration, FullSystemConfig, sizeof(CHIPSET_CONFIGURATION));
  FreePool (FullSystemConfig);

  //
  // Allocate memory for Azalia Policy PPI and Descriptor
  //
  PchAzaliaPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *)AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  ASSERT (PchAzaliaPolicyPpiDesc != NULL);
  if (PchAzaliaPolicyPpiDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PchAzaliaPolicyPpi = (AZALIA_POLICY *)AllocateZeroPool (sizeof (AZALIA_POLICY));
  ASSERT (PchAzaliaPolicyPpi != NULL);
  if (PchAzaliaPolicyPpi == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Setting Azalia policy
  //
  PchAzaliaPolicyPpi->Pme                = SystemConfiguration.AzaliaPme;   // 0 : Disable ; 1 : Enable
  PchAzaliaPolicyPpi->DS                 = SystemConfiguration.AzaliaDs;   // 0 : Docking is not supported ; 1 : Docking is supported
  PchAzaliaPolicyPpi->DA                 = PcdGet8 (PcdPchAzaliaPolicyPpiDockAttached);   // 0 : Docking is not attached ; 1 : Docking is attached
  PchAzaliaPolicyPpi->HdmiCodec          = SystemConfiguration.HdmiCodec;
  PchAzaliaPolicyPpi->AzaliaVCi          = SystemConfiguration.AzaliaVCiEnable;
  PchAzaliaPolicyPpi->ResetWaitTimer     = PcdGet16 (PcdPchAzaliaPolicyPpiResetWaitTimer);

  //
  // Update Azalia Policy PPI Descriptor
  //
  PchAzaliaPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  PchAzaliaPolicyPpiDesc->Guid  = &gAzaliaPolicyPpiGuid;
  PchAzaliaPolicyPpiDesc->Ppi   = PchAzaliaPolicyPpi;

  Status = OemSvcUpdateAzaliaPolicy (PchAzaliaPolicyPpi, PchAzaliaPolicyPpiDesc);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcUpdateAzaliaPolicy, Status : %r\n", Status));
  if (Status != EFI_SUCCESS) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Installing Azalia Policy ( PEI ).\n"));
    //
    // Install SA PEI Platform Policy PPI
    //
    Status = PeiServicesInstallPpi (PchAzaliaPolicyPpiDesc);
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Dump policy
  //
  DumpAzaliaPolicyPEI (PeiServices, PchAzaliaPolicyPpi);

  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Azalia Policy PEI Exit\n"));

  return Status;
}

VOID
DumpAzaliaPolicyPEI (
  IN      CONST EFI_PEI_SERVICES        **PeiServices,
  IN      AZALIA_POLICY                 *PchAzaliaPolicyPpi
  )
{

  DEBUG ((EFI_D_INFO | EFI_D_ERROR, "\n"));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, "AzaliaPlatformPolicyPpi ( Address : 0x%x )\n", PchAzaliaPolicyPpi));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-Pme                            : %x\n", PchAzaliaPolicyPpi->Pme));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-DS                             : %x\n", PchAzaliaPolicyPpi->DS));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-DA                             : %x\n", PchAzaliaPolicyPpi->DA));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-HdmiCodec                      : %x\n", PchAzaliaPolicyPpi->HdmiCodec));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-AzaliaVCi                      : %x\n", PchAzaliaPolicyPpi->AzaliaVCi));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-Rsvdbits                       : %x\n", PchAzaliaPolicyPpi->Rsvdbits));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-AzaliaVerbTableNum             : %x\n", PchAzaliaPolicyPpi->AzaliaVerbTableNum));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-AzaliaVerbTable                : %x\n", PchAzaliaPolicyPpi->AzaliaVerbTable));
  DEBUG ((EFI_D_INFO | EFI_D_ERROR, " |-ResetWaitTimer                 : %x\n", PchAzaliaPolicyPpi->ResetWaitTimer));
  return;
}
