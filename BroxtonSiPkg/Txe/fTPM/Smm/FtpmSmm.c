/** @file
  It updates TPM2 items in ACPI table and registers SMI2 callback
  functions for TrEE physical presence, ClearMemory, and sample
  for dTPM StartMethod.

  Caution: This module requires additional review when modified.
  This driver will have external input - variable and ACPINvs data in SMM mode.
  This external input must be validated carefully to avoid security issue.

  PhysicalPresenceCallback() and MemoryClearCallback() will receive untrusted input and do some check.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

#include "FtpmSmm.h"
#include "HeciRegs.h"
#include <Library/Tpm2DeviceLib.h>
#include <Protocol/GlobalNvsArea.h>

UINT32 getfTPMBaseAddr();

EFI_TPM2_ACPI_TABLE  mTpm2AcpiTemplate = {
  {
    EFI_ACPI_5_0_TRUSTED_COMPUTING_PLATFORM_2_TABLE_SIGNATURE,
    sizeof (mTpm2AcpiTemplate),
    EFI_TPM2_ACPI_TABLE_REVISION,
    ///
    /// Compiler initializes the remaining bytes to 0
    /// These fields should be filled in in production
    ///
  },
  0, // Flags
  (EFI_PHYSICAL_ADDRESS)(UINTN)0xFFFFFFFF, ///< Control Area
  EFI_TPM2_ACPI_TABLE_START_METHOD_ACPI,
};

EFI_SMM_VARIABLE_PROTOCOL  *mSmmVariable;
TCG_NVS                    *mTcgNvs;
EFI_TPM2_ACPI_CONTROL_AREA mControlArea; ///< Smm copy, because we need cache Command & Response address

/**
  Software SMI callback for TPM physical presence which is called from ACPI method.

  Caution: This function may receive untrusted input.
  Variable and ACPINvs are external input, so this function will validate
  its data structure to be valid value.

  @param[in]      DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]      Context         Points to an optional handler context which was specified when the
                                  handler was registered.

  @retval EFI_SUCCESS             The interrupt was handled successfully.
**/
EFI_STATUS
EFIAPI
PhysicalPresenceCallback (
  IN EFI_HANDLE                  DispatchHandle,
  IN CONST VOID                  *Context,
  IN OUT VOID                    *CommBuffer,
  IN OUT UINTN                   *CommBufferSize
  )
{
  EFI_STATUS                     Status;
  UINTN                          DataSize;
  EFI_TREE_PHYSICAL_PRESENCE     PpData;
  UINT8                          Flags;
  BOOLEAN                        RequestConfirmed;

  //
  // Get the Physical Presence variable
  //
  DataSize = sizeof (EFI_TREE_PHYSICAL_PRESENCE);
  Status = mSmmVariable->SmmGetVariable (
                           TREE_PHYSICAL_PRESENCE_VARIABLE,
                           &gEfiTrEEPhysicalPresenceGuid,
                           NULL,
                           &DataSize,
                           &PpData
                           );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  DEBUG ((EFI_D_INFO, "[TPM2] PP callback, Parameter = %x, Request = %x\n", mTcgNvs->PhysicalPresence.Parameter, mTcgNvs->PhysicalPresence.Request));

  if (mTcgNvs->PhysicalPresence.Parameter == ACPI_FUNCTION_RETURN_REQUEST_RESPONSE_TO_OS) {
    mTcgNvs->PhysicalPresence.LastRequest = PpData.LastPPRequest;
    mTcgNvs->PhysicalPresence.Response    = PpData.PPResponse;
  } else if ((mTcgNvs->PhysicalPresence.Parameter == ACPI_FUNCTION_SUBMIT_REQUEST_TO_BIOS)
             || (mTcgNvs->PhysicalPresence.Parameter == ACPI_FUNCTION_SUBMIT_REQUEST_TO_BIOS_2)) {
    if (mTcgNvs->PhysicalPresence.Request > TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_4) {
      //
      // This command requires UI to prompt user for Auth data.
      //
      mTcgNvs->PhysicalPresence.ReturnCode = PP_SUBMIT_REQUEST_NOT_IMPLEMENTED;
      return EFI_SUCCESS;
    }

    if (PpData.PPRequest != mTcgNvs->PhysicalPresence.Request) {
      PpData.PPRequest = (UINT8) mTcgNvs->PhysicalPresence.Request;
      DataSize = sizeof (EFI_TREE_PHYSICAL_PRESENCE);
      Status = mSmmVariable->SmmSetVariable (
                               TREE_PHYSICAL_PRESENCE_VARIABLE,
                               &gEfiTrEEPhysicalPresenceGuid,
                               EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                               DataSize,
                               &PpData
                               );
    }

    if (EFI_ERROR (Status)) {
      mTcgNvs->PhysicalPresence.ReturnCode = PP_SUBMIT_REQUEST_GENERAL_FAILURE;
      return EFI_SUCCESS;
    }
    mTcgNvs->PhysicalPresence.ReturnCode = PP_SUBMIT_REQUEST_SUCCESS;
  } else if (mTcgNvs->PhysicalPresence.Parameter == ACPI_FUNCTION_GET_USER_CONFIRMATION_STATUS_FOR_REQUEST) {
    Flags = PpData.Flags;
    RequestConfirmed = FALSE;

    switch (mTcgNvs->PhysicalPresence.Request) {

      case TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR:
      case TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_2:
      case TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_3:
      case TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_4:
        if ((Flags & TREE_FLAG_NO_PPI_CLEAR) != 0) {
          RequestConfirmed = TRUE;
        }
        break;

      case TREE_PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_FALSE:
        RequestConfirmed = TRUE;
        break;

      case TREE_PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_TRUE:
        break;

      default:
        if (mTcgNvs->PhysicalPresence.Request <= TREE_PHYSICAL_PRESENCE_NO_ACTION_MAX) {
          RequestConfirmed = TRUE;
        } else {
          mTcgNvs->PhysicalPresence.ReturnCode = PP_REQUEST_NOT_IMPLEMENTED;
          return EFI_SUCCESS;
        }
        break;
    }

    if (RequestConfirmed) {
      mTcgNvs->PhysicalPresence.ReturnCode = PP_REQUEST_ALLOWED_AND_PPUSER_NOT_REQUIRED;
    } else {
      mTcgNvs->PhysicalPresence.ReturnCode = PP_REQUEST_ALLOWED_AND_PPUSER_REQUIRED;
    }
  }

  return EFI_SUCCESS;
}


/**
  Software SMI callback for MemoryClear which is called from ACPI method.

  Caution: This function may receive untrusted input.
  Variable and ACPINvs are external input, so this function will validate
  its data structure to be valid value.

  @param[in]      DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]      Context         Points to an optional handler context which was specified when the
                                  handler was registered.
  @param[in, out] CommBuffer      A pointer to a collection of data in memory that will
                                  be conveyed from a non-SMM environment into an SMM environment.
  @param[in, out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS             The interrupt was handled successfully.

**/
EFI_STATUS
EFIAPI
MemoryClearCallback (
  IN EFI_HANDLE                  DispatchHandle,
  IN CONST VOID                  *Context,
  IN OUT VOID                    *CommBuffer,
  IN OUT UINTN                   *CommBufferSize
  )
{
  EFI_STATUS                     Status;
  UINTN                          DataSize;
  UINT8                          MorControl;

  mTcgNvs->MemoryClear.ReturnCode = MOR_REQUEST_SUCCESS;
  if (mTcgNvs->MemoryClear.Parameter == ACPI_FUNCTION_DSM_MEMORY_CLEAR_INTERFACE) {
    MorControl = (UINT8) mTcgNvs->MemoryClear.Request;
  } else if (mTcgNvs->MemoryClear.Parameter == ACPI_FUNCTION_PTS_CLEAR_MOR_BIT) {
    DataSize = sizeof (UINT8);
    Status = mSmmVariable->SmmGetVariable (
                             MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                             &gEfiMemoryOverwriteControlDataGuid,
                             NULL,
                             &DataSize,
                             &MorControl
                             );
    if (EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }

    if (MOR_CLEAR_MEMORY_VALUE (MorControl) == 0x0) {
      return EFI_SUCCESS;
    }
    MorControl &= ~MOR_CLEAR_MEMORY_BIT_MASK;
  }

  DataSize = sizeof (UINT8);
  Status = mSmmVariable->SmmSetVariable (
                           MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                           &gEfiMemoryOverwriteControlDataGuid,
                           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                           DataSize,
                           &MorControl
                           );
  if (EFI_ERROR (Status)) {
    mTcgNvs->MemoryClear.ReturnCode = MOR_REQUEST_GENERAL_FAILURE;
  }

  return EFI_SUCCESS;
}

/**
  Find the operation region in TCG ACPI table by given Name and Size,
  and initialize it if the region is found.

  @param[in, out] Table          The TPM item in ACPI table.
  @param[in]      Name           The name string to find in TPM table.
  @param[in]      Size           The size of the region to find.

  @return                        The allocated address for the found region.

**/
VOID *
AssignOpRegion (
  EFI_ACPI_DESCRIPTION_HEADER    *Table,
  UINT32                         Name,
  UINT16                         Size
  )
{
  EFI_STATUS                     Status;
  AML_OP_REGION_32_8             *OpRegion;
  EFI_PHYSICAL_ADDRESS           MemoryAddress;

  MemoryAddress = SIZE_4GB - 1;

  //
  // Patch some pointers for the ASL code before loading the SSDT.
  //
  for (OpRegion  = (AML_OP_REGION_32_8 *) (Table + 1);
       OpRegion <= (AML_OP_REGION_32_8 *) ((UINT8 *) Table + Table->Length);
       OpRegion  = (AML_OP_REGION_32_8 *) ((UINT8 *) OpRegion + 1)) {
    if ((OpRegion->OpRegionOp  == AML_EXT_REGION_OP) &&
        (OpRegion->NameString  == Name) &&
        (OpRegion->DWordPrefix == AML_DWORD_PREFIX) &&
        (OpRegion->BytePrefix  == AML_BYTE_PREFIX)) {

      Status = gBS->AllocatePages(AllocateMaxAddress, EfiACPIMemoryNVS, EFI_SIZE_TO_PAGES (Size), &MemoryAddress);
      ASSERT_EFI_ERROR (Status);
      ZeroMem ((VOID *)(UINTN)MemoryAddress, Size);
      OpRegion->RegionOffset = (UINT32) (UINTN) MemoryAddress;
      OpRegion->RegionLen    = (UINT8) Size;
      break;
    }
  }

  return (VOID *) (UINTN) MemoryAddress;
}

/**
  Initialize and publish TPM items in ACPI table.

  @retval   EFI_SUCCESS     The TCG ACPI table is published successfully.
  @retval   Others          The TCG ACPI table is not published.
**/
EFI_STATUS
PublishAcpiTable (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINTN                          TableKey;
  EFI_ACPI_DESCRIPTION_HEADER    *Table;
  UINTN                          TableSize;
  UINT32                         Addr;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;

  Status = GetSectionFromFv (
             &gEfiCallerIdGuid,
             EFI_SECTION_RAW,
             0,
             (VOID **) &Table,
             &TableSize
             );
  ASSERT_EFI_ERROR (Status);

  ASSERT (Table->OemTableId == SIGNATURE_64 ('T', 'p', 'm', '2', 'T', 'a', 'b', 'l'));
  mTcgNvs = AssignOpRegion (Table, SIGNATURE_32 ('T', 'N', 'V', 'S'), (UINT16) sizeof (TCG_NVS));
  ASSERT (mTcgNvs != NULL);
  Addr = getfTPMBaseAddr();

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  ASSERT_EFI_ERROR (Status);
  GlobalNvsArea->Area->TPMAddress = (UINT32)Addr;
  GlobalNvsArea->Area->TPMLength  = (UINT32)SEC_PTT_SAP_SIZE;
  //
  // Publish the TPM ACPI table
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR (Status);

  TableKey = 0;
  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        Table,
                        TableSize,
                        &TableKey
                        );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
/**
  Publish TPM2 ACPI table

  @retval   EFI_SUCCESS     The TPM2 ACPI table is published successfully.
  @retval   Others          The TPM2 ACPI table is not published.
**/
EFI_STATUS
PublishFtpmAcpiTable (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINTN                          TableKey;
  EFI_TPM2_ACPI_CONTROL_AREA     *ControlArea;

  //
  // Construct ACPI table
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR (Status);
  mTpm2AcpiTemplate.AddressOfControlArea = (UINTN)getfTPMBaseAddr();

  ControlArea = (EFI_TPM2_ACPI_CONTROL_AREA *)(UINTN)mTpm2AcpiTemplate.AddressOfControlArea;
  ZeroMem (ControlArea, sizeof(*ControlArea));
  ControlArea->CommandSize  = 0x5DC;
  ControlArea->ResponseSize = 0x5DC;
  ControlArea->Command      = (UINTN)mTpm2AcpiTemplate.AddressOfControlArea + 0x80;
  ControlArea->Response     = (UINTN)mTpm2AcpiTemplate.AddressOfControlArea + 0x80;
  CopyMem (&mControlArea, ControlArea, sizeof(mControlArea));

  DEBUG ((EFI_D_INFO, "Ftpm Windows Buffer Control Area Address = %x\n", mTpm2AcpiTemplate.AddressOfControlArea));
  DEBUG ((EFI_D_INFO, "Ftpm Windows Command/Response Buffer Address = %x\n", ControlArea->Command));

  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        &mTpm2AcpiTemplate,
                        sizeof(mTpm2AcpiTemplate),
                        &TableKey
                        );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  The driver's entry point.

  It install callbacks for TPM physical presence and MemoryClear, and locate
  SMM variable to be used in the callback function.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval Others          Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
InitializeFtpmSmm (
  IN EFI_HANDLE                  ImageHandle,
  IN EFI_SYSTEM_TABLE            *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_SMM_SW_DISPATCH2_PROTOCOL  *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT    SwContext;
  EFI_HANDLE                     SwHandle;


  Status = Tpm2RequestUseTpm ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "TPM not detected!\n"));
    return EFI_SUCCESS;
  }

  Status = PublishAcpiTable ();
  ASSERT_EFI_ERROR (Status);

  //
  // Get the Sw dispatch protocol and register SMI callback functions.
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID**)&SwDispatch);
  ASSERT_EFI_ERROR (Status);
  SwContext.SwSmiInputValue = (UINTN) - 1;
  Status = SwDispatch->Register (SwDispatch, PhysicalPresenceCallback, &SwContext, &SwHandle);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mTcgNvs->PhysicalPresence.SoftwareSmi = (UINT8) SwContext.SwSmiInputValue;

  SwContext.SwSmiInputValue = (UINTN) - 1;
  Status = SwDispatch->Register (SwDispatch, MemoryClearCallback, &SwContext, &SwHandle);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mTcgNvs->MemoryClear.SoftwareSmi = (UINT8) SwContext.SwSmiInputValue;

  //
  // Locate SmmVariableProtocol.
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmVariableProtocolGuid, NULL, (VOID**)&mSmmVariable);
  ASSERT_EFI_ERROR (Status);

  //
  // Publish TPM2 ACPI table
  //
  Status = PublishFtpmAcpiTable();
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

