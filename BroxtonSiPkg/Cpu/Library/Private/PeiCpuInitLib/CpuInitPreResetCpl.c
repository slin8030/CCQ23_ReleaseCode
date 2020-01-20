/** @file
  Power Management EarlyPost initializations.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#include <Private/Library/CpuPowerOnConfigLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/CpuPlatformLib.h>
#include <Library/PcdLib.h>
#include <CpuAccess.h>
#include <Library/CpuPolicyLib.h>
#include <SaRegs.h>
#include <Library/IoLib.h>
#include <Ppi/CpuPolicy.h>
#include <Library/MmPciLib.h>
#include <Library/PostCodeLib.h>
#include <Library/CpuMailboxLib.h>

/**
  Configure VNN VR
**/
VOID
ConfigureVrs(
 )
{
  EFI_STATUS                  Status;
  UINT32                      MailboxData;
  UINT32                      MailboxCmd;
  UINT32                      MailboxStatus;
  UINT32                      MailboxType;

  MailboxType = MAILBOX_TYPE_PCODE;
  MailboxCmd  = WRITE_VR_CURRENT_CONFIG_CMD;
  MailboxData = 0x00001000;
  Status = MailboxWrite(MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
  if (EFI_ERROR(Status) || (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS)) {
    DEBUG ((DEBUG_ERROR, "VR: Error writing VR CURRENT CONFIG command. EFI_STATUS = %X, Mailbox Status = %X\n", Status, MailboxStatus));
  }
}

/**
  Initialize performance and power management features before RESET_CPL at Post-memory phase.

  @param[in] CpuPolicyPpi     The Cpu Policy PPI instance.
**/
EFI_STATUS
CpuInitPreResetCpl (
  IN SI_CPU_POLICY_PPI        *SiCpuPolicyPpi
  )
{
  CPU_CONFIG         *CpuConfig;
  EFI_STATUS         Status;
  UINT32             Data32;
  UINT32             NumberOfActiveCores = 0;
  UINT32             MchBarBase;
  UINTN              McD0BaseAddress;

  DEBUG ((DEBUG_INFO, "Cpu Initialize performance and power management features before RESET_CPL Done\n"));
  PostCode (0xC31);
  Status = GetConfigBlock ((CONFIG_BLOCK_TABLE_HEADER *)SiCpuPolicyPpi, &gCpuConfigGuid , (VOID *)&CpuConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Configure VNN VR's
  //
  DEBUG ((DEBUG_INFO, "Configure VNN VR's\n"));
  ConfigureVrs();


  McD0BaseAddress = MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN);
  MchBarBase  = MmioRead32 (McD0BaseAddress + R_SA_MCHBAR_REG) &~BIT0;

  if (CpuConfig->ActiveProcessorCores == 1) {
    DEBUG ((DEBUG_INFO, "Check and Update Core Disable Mask Register\n"));
    if(!(CpuConfig->DisableCore1)){
      NumberOfActiveCores |=BIT1;
    }
    if(!(CpuConfig->DisableCore2)){
      NumberOfActiveCores |=BIT2;
    }
    if(!(CpuConfig->DisableCore3)){
      NumberOfActiveCores |=BIT3;
    }
  }

  Data32 = MmioRead32(MchBarBase + P_CR_CORE_DISABLE_MASK_0_0_0_MCHBAR_PUNIT_REG);
  DEBUG ((DEBUG_INFO, "P_CR_CORE_DISABLE_MASK_0_0_0_MCHBAR_PUNIT_REG = %x, NumberOfActiveCores = %x \n", Data32, NumberOfActiveCores));
  if(Data32!=NumberOfActiveCores){
    MmioWrite32((MchBarBase + P_CR_CORE_DISABLE_MASK_0_0_0_MCHBAR_PUNIT_REG), NumberOfActiveCores);
//[-start-160819-IB03090432-modify]//
    PcdSet32 (PcdSetCoreCount, 1);
//[-end-160819-IB03090432-modify]//
    PostCode (0xC34);
  }
  PostCode (0xC36);
  return Status;
}
