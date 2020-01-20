/** @file
  SC Init Smm module for SC specific SMI handlers.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
#include "ScInitSmm.h"

GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMM_SW_DISPATCH2_PROTOCOL *mSwDispatch;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMM_SX_DISPATCH2_PROTOCOL *mSxDispatch;
GLOBAL_REMOVE_IF_UNREFERENCED SC_NVS_AREA                   *mScNvsArea;
GLOBAL_REMOVE_IF_UNREFERENCED UINT16                        mAcpiBaseAddr;
GLOBAL_REMOVE_IF_UNREFERENCED SC_POLICY_HOB                 *mScPolicy;
GLOBAL_REMOVE_IF_UNREFERENCED SC_PCIE_CONFIG                *mPcieRpConfig;
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                        mAspmSwSmiNumber;
//
// The reserved MMIO range to be used in Sx handler
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_PHYSICAL_ADDRESS                      mResvMmioBaseAddr;
GLOBAL_REMOVE_IF_UNREFERENCED UINTN                                     mResvMmioSize;


/**
  PCH Sx entry SMI handler.

  @param[in]     Handle          Handle of the callback
  @param[in]     Context         The dispatch context
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
PchSxHandler (
  IN  EFI_HANDLE                   Handle,
  IN CONST VOID                    *Context OPTIONAL,
  IN OUT VOID                      *CommBuffer OPTIONAL,
  IN OUT UINTN                     *CommBufferSize OPTIONAL
  )
{
  //PchGpioSxIsolationCallBack ();
  //PchLanSxCallback ();

  return EFI_SUCCESS;
}

/**
  Initialize PCH Sx entry SMI handler.

  @param[in] ImageHandle - Handle for the image of this driver

  @retval  None
**/
VOID
InitializeSxHandler (
  IN EFI_HANDLE        ImageHandle
  )
{
  EFI_SMM_SX_REGISTER_CONTEXT               SxDispatchContext;
  EFI_HANDLE                                SxDispatchHandle;
  EFI_SLEEP_TYPE                            SxType;
  EFI_STATUS                                Status;

  DEBUG ((DEBUG_INFO, "InitializeSxHandler() Start\n"));

  //  InitializeGpioSxIsolationSmm ();

  //
  // Register the callback for S3/S4/S5 entry
  //
  SxDispatchContext.Phase = SxEntry;
  for (SxType = SxS3; SxType <= SxS5; SxType++) {
    SxDispatchContext.Type  = SxType;
    Status = mSxDispatch->Register (
                            mSxDispatch,
                            PchSxHandler,
                            &SxDispatchContext,
                            &SxDispatchHandle
                            );
    ASSERT_EFI_ERROR (Status);
  }

  DEBUG ((DEBUG_INFO, "InitializeSxHandler() End\n"));
}

/**
  Allocates reserved MMIO for Sx SMI handler use.

  This function is only called from entry point therefore DXE/boot Services can be used here.
  Updates ACPI NVS location to reserve allocated MMIO range as system resource.

  @param[in]  ImageHandle

  @retval  None
**/
VOID
AllocateReservedMmio (
  IN EFI_HANDLE        ImageHandle
  )
{
  //  SC_NVS_AREA_PROTOCOL  *ScNvsAreaProtocol;
  EFI_STATUS             Status;

  mResvMmioSize = 1 << N_LAN_MBARA_ALIGN;
  mResvMmioBaseAddr = 0xFFFFFFFF;

  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  N_LAN_MBARA_ALIGN,
                  mResvMmioSize,
                  &mResvMmioBaseAddr,
                  ImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "mResvMmioBaseAddr %x\n", mResvMmioBaseAddr));

  //
  // Locate the PCH shared data area and update reserved memory base address
  //
/*
  Status = gBS->LocateProtocol (&gScNvsAreaProtocolGuid, NULL, (VOID **) &ScNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);

  ScNvsAreaProtocol->Area->SxMemSize = (UINT32) mResvMmioSize;
  ScNvsAreaProtocol->Area->SxMemBase = (UINT32) mResvMmioBaseAddr;
*/
}

/**
  Initializes the PCH SMM handler for for PCIE hot plug support
  <b>PchInit SMM Module Entry Point</b>\n
  - <b>Introduction</b>\n
      The PchInitSmm module is a SMM driver that initializes the Intel Platform Controller Hub
      SMM requirements and services. It consumes the PCH_POLICY_PROTOCOL for expected
      configurations per policy.

  - <b>Details</b>\n
    This module provides SMI handlers to services PCIE HotPlug SMI, LinkActive SMI, and LinkEq SMI.
    And also provides port 0x61 emulation support, GPIO Sx isolation requirement, and register BIOS WP
    handler to process BIOSWP status.

  - @pre
    - @link _EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL @endlink
      - This is to ensure that PCI MMIO and IO resource has been prepared and available for this driver to allocate.
    - EFI_SMM_BASE2_PROTOCOL
    - @link _EFI_SMM_IO_TRAP_DISPATCH2_PROTOCOL EFI_SMM_IO_TRAP_DISPATCH2_PROTOCOL @endlink
    - @link _EFI_SMM_SX_DISPATCH2_PROTOCOL EFI_SMM_SX_DISPATCH2_PROTOCOL @endlink
    - @link _EFI_SMM_CPU_PROTOCOL EFI_SMM_CPU_PROTOCOL @endlink
    - @link _PCH_SMI_DISPATCH_PROTOCOL PCH_SMI_DISPATCH_PROTOCOL @endlink
    - @link _PCH_PCIE_SMI_DISPATCH_PROTOCOL PCH_PCIE_SMI_DISPATCH_PROTOCOL @endlink
    - @link _PCH_TCO_SMI_DISPATCH_PROTOCOL PCH_TCO_SMI_DISPATCH_PROTOCOL @endlink
    - @link _PCH_ESPI_SMI_DISPATCH_PROTOCOL PCH_ESPI_SMI_DISPATCH_PROTOCOL @endlink
    - SC_NVS_AREA_PROTOCOL
    - @link PeiDxeSmmPchPlatformLib.h PeiDxeSmmPchPlatformLib @endlink library
    - @link PeiDxeSmmPchPciExpressHelpersLib.h PchPciExpressHelpersLib @endlink library

  - <b>References</b>\n
    @link _PCH_POLICY PCH_POLICY_PROTOCOL @endlink.

  - <b>Integration Checklists</b>\n
    - Verify prerequisites are met. Porting Recommendations.
    - No modification of this module should be necessary
    - Any modification of this module should follow the PCH BIOS Specification and EDS

  @param[in] ImageHandle - Handle for the image of this driver
  @param[in] SystemTable - Pointer to the EFI System Table

  @retval EFI_SUCCESS    - PCH SMM handler was installed
**/
EFI_STATUS
EFIAPI
ScInitSmmEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_PEI_HOB_POINTERS                      HobPtr;

  DEBUG ((DEBUG_INFO, "ScInitSmmEntryPoint()\n"));

  //
  // Locate the SMM SW dispatch protocol
  //
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmSwDispatch2ProtocolGuid,
                    NULL,
                    (VOID **) &mSwDispatch
                    );
  ASSERT_EFI_ERROR (Status);

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmSxDispatch2ProtocolGuid,
                    NULL,
                    (VOID**) &mSxDispatch
                    );
  ASSERT_EFI_ERROR (Status);

  //  Status = gBS->LocateProtocol (&gScNvsAreaProtocolGuid, NULL, (VOID **) &ScNvsAreaProtocol);
  //  ASSERT_EFI_ERROR (Status);
  //  mScNvsArea = ScNvsAreaProtocol->Area;

  //
  // Get Sc Policy Hob
  //
  HobPtr.Guid   = GetFirstGuidHob (&gScPolicyHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  mScPolicy = (SC_POLICY_HOB*) GET_GUID_HOB_DATA (HobPtr.Guid);

  Status = GetConfigBlock ((VOID *) mScPolicy, &gPcieRpConfigGuid, (VOID *) &mPcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData, //MemoryType don't care
                    sizeof (UINT32),
                    (VOID **) &mAspmSwSmiNumber
                    );
  ASSERT_EFI_ERROR (Status);

  mAspmSwSmiNumber = mPcieRpConfig->AspmSwSmiNumber;
  //  PchAcpiBaseGet (&mAcpiBaseAddr);

  //  AllocateReservedMmio (ImageHandle);

  InitializeSxHandler (ImageHandle);

  Status = InitializeScPcieSmm (ImageHandle, SystemTable);
  ASSERT_EFI_ERROR (Status);

  //  Status = InstallPchBiosWriteProtect (ImageHandle, SystemTable);
  //  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
