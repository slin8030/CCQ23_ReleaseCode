/** @file

@copyright
 Copyright (c) 2011 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _PEI_SC_POLICY_UPDATE_LIB_H_
#define _PEI_SC_POLICY_UPDATE_LIB_H_

//extern PCH_PCIE_DEVICE_OVERRIDE mPcieDeviceTable[];

//[-start-160308-IB03090425-remove]//
//VOID
//EFIAPI
//InstallPlatformVerbTablesEx (
//  VOID
//  );
//[-end-160308-IB03090425-remove]//

#if 0
/**
  Install PCIE Device Table.

  @param[in]  DeviceTable            The pointer to the PCH_PCIE_DEVICE_OVERRIDE

  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI ERRORS             The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
InstallPcieDeviceTable (
  IN PCH_PCIE_DEVICE_OVERRIDE         *DeviceTable
  );
#endif

/**
  This function performs SC PEI Policy initialization.

  @param[in, out] ScPolicy       The SC Policy PPI instance

  @retval EFI_SUCCESS             The PPI is installed and initialized.
  @retval EFI ERRORS              The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
UpdatePeiScPolicy (
  IN OUT  SC_POLICY_PPI         *ScPolicyPpi
  );

#endif
