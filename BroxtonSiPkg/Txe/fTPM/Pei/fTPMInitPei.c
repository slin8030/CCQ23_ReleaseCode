/** @file
    Framework PEIM to Init fTPM.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2008 - 2016 Intel Corporation.

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


#include "SeCAccess.h"
#include "HeciRegs.h"

#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Ppi/fTPMPolicy.h>

/**

  Initialize SEC after reset

  @param[in]  FileHandle  - Not used.
  @param[in]  PeiServices - General purpose services available to every PEIM.

  @retval   EFI_SUCCESS:  Heci initialization completed successfully.
  @retval   All other error conditions encountered result in an ASSERT.

**/
EFI_STATUS
PeimfTPMInit (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  UINT32                  lfTPMBase;
  HECI_FWS_REGISTER       SeCFirmwareStatus;
  EFI_STATUS              Status;
  SEC_FTPM_POLICY_PPI     *FtpmPolicyPpi;

  //
  // Get platform policy settings through the FtpmPolicy PPI
  //
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gSeCfTPMPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **) &FtpmPolicyPpi
                             );
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Can't Find fTPM Policy PPI, disable fTPM\n"));
    return EFI_SUCCESS;
  }

  if (!FtpmPolicyPpi->fTPMEnable) {
    DEBUG((EFI_D_ERROR, "According fTPM Policy PPI, disable fTPM\n"));
    return EFI_SUCCESS;
  }

  SeCFirmwareStatus.ul = HeciPciRead32 (R_SEC_FW_STS0);


  DEBUG ((EFI_D_INFO, "R_SEC_FW_STS0 is %08x %x\n", SeCFirmwareStatus.ul, SeCFirmwareStatus.r.SeCOperationMode));
  if(SeCFirmwareStatus.r.SeCOperationMode != 0) {
    DEBUG((EFI_D_ERROR, "SeC is not in Normal mode, disable fTPM\n"));
    return EFI_SUCCESS;
  }


  lfTPMBase = (UINTN)AllocateRuntimePages(EFI_SIZE_TO_PAGES(SEC_PTT_SAP_SIZE));
  if (lfTPMBase == (UINTN)NULL) {
    DEBUG((EFI_D_ERROR, "Can't allocate memory address\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  ///
  /// 3.1     SATT_PTT_SAP_SIZE.
  ///
  HeciPciWrite32(R_SATT_PTT_SAP_SIZE, SEC_PTT_SAP_SIZE);
  ///
  /// 3.2     SATT_PTT_BRG_BA_LSB.
  ///
  HeciPciWrite32 (R_SATT_PTT_BRG_BA_LSB, lfTPMBase);

  ///
  /// 3.3     SATT_PTT_CTRL.BRG_BA_MSB.
  /// bit 8:11 is MSB for IOSF address
  /// Here we only use the memory below 4G.
  /// So bit 8:11 must be 0
  ///
  HeciPciOr32 (R_SATT_PTT_CTRL, ~0xF00);
  ///
  /// 3.4     SATT_PTT_CTRL.ENTRY_VLD.
  ///
  HeciPciOr32(R_SATT_PTT_CTRL,B_ENTRY_VLD);

  DEBUG((EFI_D_INFO,"******  fTPM  ************ \n"));
  DEBUG((EFI_D_INFO,"UMA SIZE = 0x%x B regsVal %x \n",HeciPciRead32 (R_SATT_PTT_SAP_SIZE), R_SATT_PTT_SAP_SIZE));
  DEBUG((EFI_D_INFO,"UMA LSB = 0x%x  regsVal %x\n",HeciPciRead32 (R_SATT_PTT_BRG_BA_LSB),R_SATT_PTT_BRG_BA_LSB));
  DEBUG((EFI_D_INFO,"SATT PTT CTRL = 0x%x  regsVal %x\n",HeciPciRead32(R_SATT_PTT_CTRL),R_SATT_PTT_CTRL));

  SetMem((VOID*)(UINTN)lfTPMBase, SEC_PTT_SAP_SIZE, 0);
  return EFI_SUCCESS;
}

