/** @file
  Header file for the SC Init PEIM

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

@par Specification Reference:
**/
#ifndef _SC_INIT_PEI_H_
#define _SC_INIT_PEI_H_

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/TimerLib.h>
#include <Library/PreSiliconLib.h>
#include <ScAccess.h>
#include <SaAccess.h>
#include <Library/MmPciLib.h>
#include <Library/ScPlatformLib.h>
#include <Ppi/ScPolicy.h>
#include <Ppi/ScPolicyPreMem.h>
#include <Ppi/SiPolicyPpi.h>
#include <Ppi/MemoryDiscovered.h>
#include <Ppi/ScPcieDeviceTable.h>
#include <IndustryStandard/Pci30.h>
#include <Library/SideBandLib.h>
#include <Library/SteppingLib.h>
#include <Private/Guid/ScPolicyHobGuid.h>
#include <Private/Library/ScInitCommonLib.h>
#include <Private/Library/SataInitLib.h>
#include <Private/Library/PeiPciExpressInitLib.h>
#include <Private/Library/UsbCommonLib.h>
#include <Library/PmcIpcLib.h>
#include <Library/PeiVtdLib.h>
#include <Library/GpioLib.h>
#include <Library/SteppingLib.h>
#include <Library/PcdLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PeiScPolicyLib.h>
#include <Library/PostCodeLib.h>

extern EFI_GUID gScDeviceTableHobGuid;

#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND  1
#endif
#ifndef STALL_ONE_MILLI_SECOND
#define STALL_ONE_MILLI_SECOND  1000
#endif
#ifndef STALL_100_MILLI_SECOND
#define STALL_100_MILLI_SECOND 100
#endif

///
/// Using SC_PM_ENABLE macro to add PM supporting for specific HSLE modules. Don't enable it for normal release, which will cause driver
/// issue (PCI header reset) around WW23'14 modules.
///
#define SC_PM_ENABLE
/**
  The function performs RTC specific programming.

  @param[in]  None

  @retval EFI_SUCCESS           The required settings programmed successfully
**/
EFI_STATUS
EFIAPI
ConfigureRtc (
  );

/**
  The function performs SMBUS specific programming.

  @param[in] ScPolicyPpi       The SC Policy PPI instance

  @retval EFI_SUCCESS           The required settings programmed successfully
**/
EFI_STATUS
EFIAPI
ScSmbusConfigure (
  IN  SC_POLICY_PPI      *ScPolicyPpi
  );

/**
  Initialize the Intel High Definition Audio Controller

  @param[in] SiPolicy             The SI Policy ppi instance
  @param[in] ScPolicy             The SC Policy ppi instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS             Codec is detected and initialized.
  @retval EFI_OUT_OF_RESOURCES    Failed to allocate resources to initialize the codec.
**/
EFI_STATUS
ConfigureHda (
  IN     SI_POLICY_PPI  *SiPolicy,
  IN     SC_POLICY_PPI  *ScPolicy,
  IN OUT UINT32         *FuncDisableReg
  );

/**
  This function performs SC initialization stage after memory is available.
  Only the feature must be executed right after memory installed should be done here.

  @param[in] PeiServices   Pointer to PEI Services Table.
  @param[in] NotifyDesc    Pointer to the descriptor for the Notification event that
                           caused this function to execute.
  @param[in] Ppi           Pointer to the PPI data associated with this function.

  @retval EFI_SUCCESS      Always returns EFI_SUCCESS
**/
EFI_STATUS
ScOnMemoryInstalled (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
  IN  VOID                      *Ppi
  );

/**
  Program Sc devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID).

  @param[in] ScPolicy    The SC Policy PPI instance

  @retval EFI_SUCCESS     The function completed successfully
**/
EFI_STATUS
ProgramSvidSid (
  IN  SC_POLICY_PPI     *ScPolicy
  );

/**
  Initialize P2SB according to P2SB policy of the SC
  Platform Policy PPI

  @param[in] P2sbBase             P2SB MMIO address

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.
**/
EFI_STATUS
ScP2sbInit (
  IN UINT32 P2sbBase
  );

/**
  The function perform all the ModPhy programming that required by silicon
  in early PEI phase before devices get initialized.

  @param[in] SiPolicy     The Silicon Policy PPI instance
  @param[in] ScPolicy     The SC Policy PPI instance

  @retval EFI_SUCCESS       The function completed successfully
  @retval Others            All other error conditions encountered result in an ASSERT.
**/
EFI_STATUS
EFIAPI
ScModPhyProgramming (
  IN  SI_POLICY_PPI  *SiPolicy,
  IN  SC_POLICY_PPI  *ScPolicy
  );

/**
  Internal function performing miscellaneous init needed in early PEI phase

  @param[in] ScPolicyPpi  The SC Platform Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.
**/
EFI_STATUS
ScMiscInit (
  IN  SC_POLICY_PPI      *ScPolicyPpi
  );

/**
  Initialize IOAPIC according to IoApicConfig policy of the SC
  Policy PPI

  @param[in] ScPolicyPpi         The SC Policy PPI instance

  @retval EFI_SUCCESS            Succeeds.
  @retval EFI_DEVICE_ERROR       Device error, aborts abnormally.
**/
EFI_STATUS
ScIoApicInit (
  IN  SC_POLICY_PPI     *ScPolicyPpi
  );

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
  IN  OUT UINT32        *FuncDisableReg
  );

/**
  Configure LPC device on early PEI.
**/
VOID
ConfigureLpcOnEarlyPei (
  VOID
  );

/**
  Configure LPC device on Policy callback.

  @param[in] ScPreMemPolicyPpi  The SC Policy PPI instance
**/
VOID
ConfigureLpcOnPolicy (
  IN  SC_PREMEM_POLICY_PPI  *ScPreMemPolicyPpi
  );

/**
  The function performs LPC specific programming.

  @param[in]  None

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
ScLpcInit (
  VOID
  );

/**
  Perform power management initialization

  @param[in] SchPolicy                  The SC Policy PPI instance

  @retval EFI_SUCCESS                   Succeeds.
**/
EFI_STATUS
ScPmInit (
  IN  SC_POLICY_PPI                    *ScPolicy
  );

/**
  The function performs Serial IRQ specific programming.

  @param[in] ScPolicyPpi               The SC Policy PPI instance

  @retval[None]
**/
VOID
EFIAPI
ScConfigureSerialIrq (
  IN  SC_POLICY_PPI                    *ScPolicyPpi
  );

/**
  Perform Clock Gating programming
  Enables clock gating in various SC interfaces and the registers

  @param[in] ScPolicy     The SC Policy Ppi instance

  @retval EFI_SUCCESS     The function completed successfully
**/
EFI_STATUS
ConfigureClockGating (
  IN  SC_POLICY_PPI *ScPolicy
  );

/**
  Perform Power Gating programming
  Enables power gating in various SC interfaces and the registers.

  @param[in] ScPolicyPpi     The SC Policy Ppi instance

  @retval EFI_SUCCESS        The function completed successfully
**/
EFI_STATUS
ConfigurePowerGating (
  IN  SC_POLICY_PPI                  *ScPolicyPpi
  );

/**
  Configure miscellaneous power management settings

  @param[in] ScPolicy     The SC Platform Policy Ppi instance
  @param[in] PmcBase      PMC base address of this SC device

  @retval EFI_SUCCESS     The function completed successfully
**/
EFI_STATUS
ConfigureMiscPm (
  IN  SC_POLICY_PPI  *ScPolicy,
  IN  UINT32         PmcBase
  );

/**
  Configure S0ix Settings

  @param[in] PmcBase     PMC Base Address value of this SC device

  @retval EFI_SUCCESS     Successfully completed.
**/
EFI_STATUS
ConfigureS0ix (
  IN  UINT32  PmcBase
  );

/**
  Perform miscellany SC initialization

  @param[in] PmcBase   PmcBase value of this SC device

  @retval EFI_SUCCESS  The function completed successfully
**/
EFI_STATUS
ConfigureMiscItems (
  IN  UINT32  PmcBase
  );

/**
  Configure OTG devices.

  @param[in] SiPolicy                The Silicon Policy PPI instance
  @param[in] ScPolicy                The SC Policy Ppi instance
  @param[in, out] FuncDisableReg     The value of Function disable register to be updated

  @retval EFI_SUCCESS                The function completed successfully
**/
EFI_STATUS
ConfigureXdci (
  IN  SI_POLICY_PPI                        *SiPolicy,
  IN  SC_POLICY_PPI                        *ScPolicy,
  IN  OUT UINT32                           *FuncDisableReg
  );

/**
  Configure GMM

  @param[in] SiPolicyPpi          The Silicon Policy PPI instance
  @param[in] ScPolicyPpi          The SC Policy Ppi instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureGmm (
  IN  SI_POLICY_PPI               *SiPolicyPpi,
  IN  SC_POLICY_PPI               *ScPolicyPpi,
  IN OUT UINT32                   *FuncDisableReg
  );

/**
  Configure LPSS devices.

  @param[in] SiPolicy             The Silicon Policy PPI instance
  @param[in] ScPolicy             The SC Policy Ppi instance
  @param[in, out] FuncDisableReg  The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureLpss (
  IN SI_POLICY_PPI                 *SiPolicy,
  IN SC_POLICY_PPI                 *ScPolicy,
  IN OUT UINT32                    *FuncDisableReg
  );

/**
  Configure SCC devices.

  @param[in] SiPolicy             The Silicon Policy PPI instance
  @param[in] ScPolicy             The SC Policy Ppi instance
  @param[in, out] FuncDisableReg  The value of Function disable register
  @param[in, out] FuncDisableReg1 The value of Function disable 1 register

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureScs (
  IN SI_POLICY_PPI                 *SiPolicy,
  IN SC_POLICY_PPI                 *ScPolicy,
  IN OUT UINT32                    *FuncDisableReg,
  IN OUT UINT32                    *FuncDisableReg1
  );

/**
  Configure ISH devices.

  @param[in]  None

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureIsh (
  );

/**
  Configure Iosf devices.

  @param[in]  None

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureIosf (
  );

/**
  Configure Itss device

  @param[in]  None

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigureItss (
  );

/**
  Configure Pmc device

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
ConfigurePmc (
  IN SI_POLICY_PPI                 *SiPolicy
  );

/**
  The function perform all the HSIO programming that required by silicon
  in early PEI phase before devices get initialized.

  @param[in] SiPolicy       The Silicon Policy PPI instance
  @param[in] ScPolicy       The SC Policy Ppi instance

  @retval EFI_SUCCESS       The function completed successfully
  @retval Others            All other error conditions encountered result in an ASSERT.
**/
EFI_STATUS
EFIAPI
ScHsioBiosProg (
  IN  SI_POLICY_PPI  *SiPolicy,
  IN  SC_POLICY_PPI  *ScPolicy
  );

/**
  Configures SC devices interrupts

  @param[in] SiPolicy          Policy

  @retval EFI_SUCCESS          The function completed successfully
**/
EFI_STATUS
ScConfigureInterrupts (
  IN  SC_POLICY_PPI *ScPolicyPpi
  );
#endif
