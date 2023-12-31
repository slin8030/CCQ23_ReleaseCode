/** @file
  Initializes the HD-Audio Controller and Codec.

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

@par Specification
**/
#include "ScInit.h"
#include <ScAccess.h>
#include <Guid/Acpi.h>
#include <Library/PcdLib.h>
#include <Library/SteppingLib.h>
#include <Private/Library/ScHdaLib.h>

SC_HDA_NHLT_ENDPOINTS mScHdaNhltEndpoints[HdaEndpointMax] =
{
  {HdaDmicX2,        FALSE},
  {HdaDmicX4,        FALSE},
  {HdaBtRender,      FALSE},
  {HdaBtCapture,     FALSE},
  {HdaI2sRenderSKP,  FALSE},
  {HdaI2sCaptureSKP, FALSE},
  {HdaI2sRenderHP,   FALSE},
  {HdaI2sCaptureHP,  FALSE},
  {HdaModem1Render,  FALSE},
  {HdaModem1Capture, FALSE},
  {HdaModem2Render,  FALSE},
  {HdaModem2Capture, FALSE}
};

/**
  Retrieves address of NHLT table from XSDT/RSDT.

  @retval NHLT_ACPI_TABLE*              Pointer to NHLT table if found
  @retval NULL                          NHLT could not be found
**/
NHLT_ACPI_TABLE *
LocateNhltAcpiTable (
  VOID
  )
{
  EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Rsdp;
  EFI_ACPI_DESCRIPTION_HEADER                   *Xsdt;
  NHLT_ACPI_TABLE                               *Nhlt;
  UINTN                                         Index;
  UINT32                                        Data32;
  EFI_STATUS                                    Status;
  Rsdp  = NULL;
  Xsdt  = NULL;
  Nhlt  = NULL;

  //
  // Find the AcpiSupport protocol returns RSDP (or RSD PTR) address.
  //
  DEBUG ((DEBUG_INFO, "LocateNhltAcpiTable() Start\n"));

  Status = EfiGetSystemConfigurationTable (&gEfiAcpiTableGuid, (VOID *) &Rsdp);
  if (EFI_ERROR (Status) || (Rsdp == NULL)) {
    DEBUG ((DEBUG_ERROR, "EFI_ERROR or Rsdp == NULL\n"));
    return NULL;
  }

  Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) Rsdp->XsdtAddress;
  if (Xsdt == NULL || Xsdt->Signature != EFI_ACPI_5_0_EXTENDED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
    // If XSDT has not been found, check RSDT
    Xsdt = (EFI_ACPI_DESCRIPTION_HEADER *)(UINTN) Rsdp->RsdtAddress;
    if (Xsdt == NULL || Xsdt->Signature != EFI_ACPI_5_0_ROOT_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
      DEBUG ((DEBUG_ERROR, "XSDT/RSDT == NULL or wrong signature\n"));
      return NULL;
    }
  }

  for (Index = sizeof (EFI_ACPI_DESCRIPTION_HEADER); Index < Xsdt->Length; Index = Index + sizeof (UINT32)) {
    Data32  = *(UINT32 *) ((UINT8 *) Xsdt + Index);
    Nhlt    = (NHLT_ACPI_TABLE *) (UINT32 *) (UINTN) Data32;
    if (Nhlt->Header.Signature == NHLT_ACPI_TABLE_SIGNATURE) {
      break;
    }
  }

  if (Nhlt == NULL || Nhlt->Header.Signature != NHLT_ACPI_TABLE_SIGNATURE) {
    DEBUG ((DEBUG_ERROR, "Nhlt == NULL or wrong signature\n"));
    return NULL;
  }

  DEBUG ((DEBUG_INFO, "Found NhltTable, Address = 0x%x\n", Nhlt));

  return Nhlt;
}

/**
  Constructs and installs NHLT table.

  @retval EFI_SUCCESS                   ACPI Table installed successfully
  @retval EFI_UNSUPPORTED               ACPI Table protocol not found
**/
EFI_STATUS
PublishNhltAcpiTable (
 IN SC_POLICY_HOB  *ScPolicy
 )
{
  UINTN                                 AcpiTableKey;
  EFI_ACPI_TABLE_PROTOCOL               *AcpiTable;
  NHLT_ACPI_TABLE                       *NhltTable;
  UINT32                                TableLength;
  EFI_STATUS                            Status;

  AcpiTable = NULL;
  NhltTable = NULL;
  AcpiTableKey = 0;

  DEBUG ((DEBUG_INFO, "PublishNhltAcpiTable() Start\n"));

  //
  // Locate ACPI support protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  if( EFI_ERROR(Status) || AcpiTable == NULL) {
    return EFI_UNSUPPORTED;
  }

  NhltConstructor (mScHdaNhltEndpoints, &NhltTable, &TableLength, ScPolicy);
  NhltAcpiHeaderConstructor (NhltTable, TableLength);

  Status = AcpiTable->InstallAcpiTable (AcpiTable, NhltTable, NhltTable->Header.Length, &AcpiTableKey);

  DEBUG ((DEBUG_INFO, "PublishNhltAcpiTable() End\n"));
  return Status;
}

/**
  Sets NVS ACPI variables for _DSM accordingly to policy.

  @param[in] NhltAcpiTableAddress       NHLT ACPI Table address
  @param[in] NhltAcpiTableLength        NHLT ACPI Table length
  @param[in] DspFeatureMask             Dsp feature mask
  @param[in] DspPpModuleMask            Dsp post process module mask
**/
VOID
UpdateHdaAcpiData (
  IN      UINT32   NhltAcpiTableAddress,
  IN      UINT32   NhltAcpiTableLength,
  IN      UINT32   DspFeatureMask,
  IN      UINT32   DspPpModuleMask
  )
{
  EFI_STATUS                    Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "UpdateHdaAcpiData():\n NHLT Address = 0x%08x, Length = 0x%08x\n", NhltAcpiTableAddress, NhltAcpiTableLength));
  DEBUG ((DEBUG_INFO, " FeatureMask = 0x%08x\n PostProcessingModuleMask = 0x%08x\n", DspFeatureMask, DspPpModuleMask));

  GlobalNvsArea->Area->HdaNhltAcpiAddr   = NhltAcpiTableAddress;
  GlobalNvsArea->Area->HdaNhltAcpiLength = NhltAcpiTableLength;
  GlobalNvsArea->Area->HdaDspFeatureMask = DspFeatureMask;
  GlobalNvsArea->Area->HdaDspModMask     = DspPpModuleMask;
}

/**
  Initialize and publish NHLT (Non-HDA Link Table), update NVS variables.

  @param[in] *HdaConfig                 Pointer to SC_HDAUDIO_CONFIG

  @retval EFI_SUCCESS                   The function completed successfully
  @retval EFI_UNSUPPORTED               No NHLT ACPI Table available
**/
EFI_STATUS
SetHdaAcpiTable (
  IN CONST SC_HDAUDIO_CONFIG    *HdaConfig,
  IN SC_POLICY_HOB              *ScPolicy
  )
{
  NHLT_ACPI_TABLE     *NhltTable;
  EFI_STATUS          Status;
  NhltTable = NULL;

  // DMIC X2
  switch (HdaConfig->DspEndpointDmic) {
    case ScHdaDmic2chArray:
      mScHdaNhltEndpoints[HdaDmicX2].Enable   = TRUE;
      break;
    case ScHdaDmic4chArray:
      mScHdaNhltEndpoints[HdaDmicX4].Enable   = TRUE;
      break;
    case ScHdaDmicDisabled:
    default:
      mScHdaNhltEndpoints[HdaDmicX2].Enable   = FALSE;
      mScHdaNhltEndpoints[HdaDmicX4].Enable   = FALSE;
  }

  if (HdaConfig->DspEndpointBluetooth) {
    mScHdaNhltEndpoints[HdaBtRender].Enable   = TRUE;
    mScHdaNhltEndpoints[HdaBtCapture].Enable  = TRUE;
  }

  if (HdaConfig->DspEndpointI2sSkp) {
    mScHdaNhltEndpoints[HdaI2sRenderSKP].Enable  = TRUE;
    mScHdaNhltEndpoints[HdaI2sCaptureSKP].Enable = TRUE;
  }

  if (HdaConfig->DspEndpointI2sHp) {
    mScHdaNhltEndpoints[HdaI2sRenderHP].Enable  = TRUE;
    mScHdaNhltEndpoints[HdaI2sCaptureHP].Enable = TRUE;
  }
  Status    = PublishNhltAcpiTable (ScPolicy);
  NhltTable = LocateNhltAcpiTable ();
  if(NhltTable == NULL) {
     return EFI_LOAD_ERROR;
  }

  UpdateHdaAcpiData ((UINT32)(UINTN)NhltTable, (UINT32)(NhltTable->Header.Length), HdaConfig->DspFeatureMask, HdaConfig->DspPpModuleMask);

  NhltAcpiTableDump (NhltTable);
  return Status;
}

/**
  Print Audio clock gating settings for debug usage
**/
VOID
DumpAudioClockGating(
  VOID
)
{
  UINTN  HdaPciBase;
  UINT32 Tmp32;
  HdaPciBase = MmPciBase (
                 DEFAULT_PCI_BUS_NUMBER_SC,
                 PCI_DEVICE_NUMBER_HDA,
                 PCI_FUNCTION_NUMBER_HDA
                 );
  Tmp32 = MmioRead32 ((UINTN) (HdaPciBase + R_HDA_PGCTL));
  DEBUG ((DEBUG_INFO, "DumpAudioClockGating() R_HDA_PGCTL = 0x%x\n", Tmp32));
}

/**
  Hide PCI config space of HD-Audio device and do any final initialization.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
  @retval EFI_UNSUPPORTED               Device non-exist or Dsp is disabled
**/
EFI_STATUS
ConfigureHdaAtBoot (
  IN SC_POLICY_HOB                      *ScPolicy
  )
{
  EFI_STATUS         Status;
  UINTN              HdaPciBase;
  SC_HDAUDIO_CONFIG  *HdaConfig;

  DEBUG ((DEBUG_INFO, "ConfigureHdaAtBoot() Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicy, &gHdAudioConfigGuid, (VOID *) &HdaConfig);
  ASSERT_EFI_ERROR (Status);

  HdaPciBase = MmPciBase (
                 DEFAULT_PCI_BUS_NUMBER_SC,
                 PCI_DEVICE_NUMBER_HDA,
                 PCI_FUNCTION_NUMBER_HDA
                 );

  if((MmioRead32 (HdaPciBase) == 0xFFFFFFFF) || (HdaConfig->DspEnable == FALSE)) {
    // Do not set ACPI tables if HD-Audio or audio DSP is function disabled.
    DEBUG ((DEBUG_INFO, "HD-Audio ACPI table is not set!\n"));
    return EFI_UNSUPPORTED;
  }
  Status = SetHdaAcpiTable (HdaConfig, ScPolicy);
  DumpAudioClockGating ();
  DEBUG ((DEBUG_INFO, "ConfigureHdaAtBoot() End\n"));

  return Status;
}
