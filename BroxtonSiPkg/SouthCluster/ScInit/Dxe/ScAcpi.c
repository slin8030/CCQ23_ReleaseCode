/** @file
  This is the driver that initializes the Intel SC devices

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#include "ScInit.h"

/**
  Update ASL object before Boot

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_STATUS
  @retval EFI_NOT_READY         The Acpi protocols are not ready.
**/
EFI_STATUS
UpdateNvsArea (
  IN     SC_POLICY_HOB         *ScPolicy
  )
{
  EFI_STATUS                   Status;
  UINTN                        Index;
  UINTN                        RpDev;
  UINTN                        RpFun;
  UINT32                       Data32;
  EFI_GLOBAL_NVS_AREA_PROTOCOL *GlobalNvsArea;
  BXT_SERIES                   BxtSeries;
  UINT8                        MaxPciePortNum;
  SC_PCIE_CONFIG               *PcieRpConfig;

  Status = GetConfigBlock ((VOID *) ScPolicy, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  Status = EFI_SUCCESS;
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );
  ASSERT_EFI_ERROR (Status);
  //
  // Update ASL PCIE port address according to root port device and function
  //
  BxtSeries = GetBxtSeries ();
  if ((BxtSeries == BxtP) || (BxtSeries == Bxt1)){
    MaxPciePortNum = GetScMaxPciePortNum ();
    for (Index = 0; Index < MaxPciePortNum; Index++) {
      Status = GetScPcieRpDevFun (Index, &RpDev, &RpFun);
      ASSERT_EFI_ERROR (Status);

      Data32 = ((UINT8) RpDev << 16) | (UINT8) RpFun;
      GlobalNvsArea->Area->RpAddress[Index] = Data32;
      //
      // Update Maximum Snoop Latency and Maximum No-Snoop Latency values for PCIE
      //
      GlobalNvsArea->Area->PcieLtrMaxSnoopLatency[Index]   = PcieRpConfig->RootPort[Index].LtrMaxSnoopLatency;
      GlobalNvsArea->Area->PcieLtrMaxNoSnoopLatency[Index] = PcieRpConfig->RootPort[Index].LtrMaxNoSnoopLatency;
    }
  }

  return Status;
}