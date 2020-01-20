/** @file
  Initializes SC USB Controllers.

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

@par Specification Reference:
**/

#include "ScInitPei.h"

/**
  The function performing USB init in PEI phase. This could be used by USB recovery
  or debug features that need USB initialization during PEI phase.
  Note: Before executing this function, please be sure that SC_INIT_PPI.Initialize
  has been done and ScUsbPolicyPpi has been installed.

  @param[in] SiPolicy     The Silicon Policy PPI instance
  @param[in] ScPolicy     The SC Policy PPI instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS       The function completed successfully
  @retval Others            All other error conditions encountered result in an ASSERT.
**/
EFI_STATUS
EFIAPI
ScUsbInit (
  IN  SI_POLICY_PPI     *SiPolicy,
  IN  SC_POLICY_PPI     *ScPolicy,
  IN OUT UINT32         *FuncDisableReg
  )
{
  EFI_STATUS          Status;
  UINT32              XhciMmioBase;
  EFI_BOOT_MODE       BootMode;
  SC_USB_CONFIG       *UsbConfig;

  DEBUG ((DEBUG_INFO, "ScUsbInit() - Start\n"));
  XhciMmioBase = (UINT32) SiPolicy->TempMemBaseAddr;
  Status = GetConfigBlock ((VOID *) ScPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);
  Status = CommonUsbInit (
            UsbConfig,
            XhciMmioBase,
            DEFAULT_PCI_BUS_NUMBER_SC,
            FuncDisableReg,
            BootMode
            );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "ScUsbInit() - End\n"));

  return Status;

}
