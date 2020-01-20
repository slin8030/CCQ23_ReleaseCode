/** @file
  OC CPU Early Post initializations.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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

#include <Private/Library/MpServiceLib.h>
#include <Private/Library/CpuOcLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/CpuPolicyLib.h>

/**
  Initializes Overclocking settings in the processor.

  @param[in] OverclockingtConfig      Pointer to Policy protocol instance

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
CpuOcInit (
  IN SI_CPU_POLICY_PPI *SiCpuPolicyPpi
  )
{
  EFI_STATUS             Status;
  OC_CAPABILITIES_ITEM   OcCaps;
  VOLTAGE_FREQUENCY_ITEM CurrentVfItem;
  VOLTAGE_FREQUENCY_ITEM RequestedVfItem;
  MSR_REGISTER           RingRatioMsr;
  UINT32                 LibStatus;
  UINT8                  DomainId;
  UINT8                  MinDomainId;
  UINT8                  MaxDomainId;
  UINT8                  ResetRequired;
  WDT_PPI                *gWdtPei;
  CPU_OVERCLOCKING_CONFIG *CpuOverclockingConfig;

  LibStatus = 0; //DEBUG
  ResetRequired = FALSE;
  MinDomainId = OC_LIB_DOMAIN_ID_DDR;
  MaxDomainId = OC_LIB_DOMAIN_ID_SYSTEM_AGENT;

  Status = GetConfigBlock ((CONFIG_BLOCK_TABLE_HEADER *)SiCpuPolicyPpi, &gCpuOverclockingConfigGuid, (VOID *)&CpuOverclockingConfig);
  ASSERT_EFI_ERROR (Status);

  if (CpuOverclockingConfig->OcSupport == 0) {
    ///
    /// Overclocking is disabled
    ///
    DEBUG ((DEBUG_ERROR, "(OC) Overclocking is disabled. Bypassing CPU core overclocking flow.\n"));
    return EFI_SUCCESS;
  }

  Status = EFI_SUCCESS;
  //
  // Locate WDT_PPI (ICC WDT PPI)
  //
  Status = PeiServicesLocatePpi (
                  &gWdtPpiGuid,
                  0,
                  NULL,
                  (VOID **) &gWdtPei
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  /// For Overclocking parts, if  a non-default ring ratio is specified, we need to
  /// update the ring ratio limit MSR
  ///
  if (CpuOverclockingConfig->RingMaxOcTurboRatio != 0) {
    RingRatioMsr.Qword = AsmReadMsr64 (MSR_RING_RATIO_LIMIT);
    RingRatioMsr.Bytes.FirstByte &= ~MSR_MAX_RING_RATIO_LIMIT_MASK;
    RingRatioMsr.Bytes.FirstByte |= CpuOverclockingConfig->RingMaxOcTurboRatio & MSR_MAX_RING_RATIO_LIMIT_MASK;
    AsmWriteMsr64 (MSR_RING_RATIO_LIMIT, RingRatioMsr.Qword);
  }

  ///
  /// We will loop on the CPU domains to manage the voltage/frequency settings
  ///
  for (DomainId = MinDomainId; DomainId < MaxDomainId; DomainId++) {
    ///
    /// Only IA_CORE and Ring are valid for CPU Core
    ///
    if ((DomainId == OC_LIB_DOMAIN_ID_IA_CORE) || (DomainId == OC_LIB_DOMAIN_ID_RING)) {
      ///
      /// Get OC Capabilities of the domain
      ///
      ZeroMem(&OcCaps,sizeof(OcCaps));
      OcCaps.DomainId = DomainId;
      Status = GetOcCapabilities(&OcCaps,&LibStatus);

      if (LibStatus == OC_LIB_COMPLETION_CODE_SUCCESS) {
        ///
        /// If any OC is supported on this domain, then proceed
        ///
        if (OcCaps.RatioOcSupported || OcCaps.VoltageOverridesSupported || OcCaps.VoltageOffsetSupported) {
          ///
          /// Need to populate the user requested settings from the Policy
          /// to determine if OC changes are desired.
          ///
          ZeroMem(&CurrentVfItem,sizeof(CurrentVfItem));
          CurrentVfItem.DomainId = DomainId;

          ///
          /// Get a copy of the current domain VfSettings from the Mailbox Library
          ///
          Status = GetVoltageFrequencyItem(&CurrentVfItem,&LibStatus);
          if ((Status != EFI_SUCCESS) || (LibStatus != OC_LIB_COMPLETION_CODE_SUCCESS)) {
            continue;
          }

          ///
          /// Populate the user requested VfSettings struct
          ///
          ZeroMem(&RequestedVfItem,sizeof(RequestedVfItem));
          RequestedVfItem.DomainId = DomainId;
          if (DomainId == OC_LIB_DOMAIN_ID_IA_CORE) {
            RequestedVfItem.VfSettings.MaxOcRatio = (UINT8) CpuOverclockingConfig->CoreMaxOcTurboRatio;
          } else if (DomainId == OC_LIB_DOMAIN_ID_RING) {
            RequestedVfItem.VfSettings.MaxOcRatio = CpuOverclockingConfig->RingMaxOcTurboRatio;
          }

          ///
          /// VoltageTarget has 2 uses and we need to update the target based
          /// on the voltagemode requested
          ///
          if (DomainId == OC_LIB_DOMAIN_ID_IA_CORE) {
            RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) CpuOverclockingConfig->CoreVoltageMode;
            if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverclockingConfig->CoreExtraTurboVoltage;
            } else {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverclockingConfig->CoreVoltageOverride;
            }
            RequestedVfItem.VfSettings.VoltageOffset = CpuOverclockingConfig->CoreVoltageOffset;
          } else if (DomainId == OC_LIB_DOMAIN_ID_RING) {
            RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) CpuOverclockingConfig->RingVoltageMode;
            if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverclockingConfig->RingExtraTurboVoltage;
            } else {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverclockingConfig->RingVoltageOverride;
            }
            RequestedVfItem.VfSettings.VoltageOffset = CpuOverclockingConfig->RingVoltageOffset;
          }

          ///
          /// Compare current settings with user requested settings to see if changes are needed
          ///
          if (CompareMem((VOID *)&RequestedVfItem,(VOID *)&CurrentVfItem,sizeof(VOLTAGE_FREQUENCY_ITEM))) {
            ///
            /// Arm watchdog timer for OC changes
            ///
            Status = gWdtPei->ReloadAndStart (WDT_TIMEOUT_BETWEEN_PEI_DXE);

            ///
            /// Need to update the requested voltage/frequency values
            ///
            DEBUG ((DEBUG_INFO, "(OC) Set Voltage Frequency for Domain = %X\n", DomainId));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.MaxOcRatio     = %X\n", RequestedVfItem.VfSettings.MaxOcRatio));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.TargetMode     = %X\n", RequestedVfItem.VfSettings.VoltageTargetMode));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.VoltageTarget  = %X\n", RequestedVfItem.VfSettings.VoltageTarget));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.VoltageOffset  = %X\n", RequestedVfItem.VfSettings.VoltageOffset));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.MaxOcRatio       = %X\n", CurrentVfItem.VfSettings.MaxOcRatio));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.TargetMode       = %X\n", CurrentVfItem.VfSettings.VoltageTargetMode));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.VoltageTarget    = %X\n", CurrentVfItem.VfSettings.VoltageTarget));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.VoltageOffset    = %X\n", CurrentVfItem.VfSettings.VoltageOffset));

            Status = SetVoltageFrequencyItem(RequestedVfItem,&LibStatus);
            if ((Status != EFI_SUCCESS) || (LibStatus != OC_LIB_COMPLETION_CODE_SUCCESS)) {
              DEBUG ((DEBUG_ERROR, "(OC) Set Voltage Frequency failed. EFI Status = %X, Library Status = %X\n", Status, LibStatus));
            }
          }
        }
      } else {
        DEBUG ((DEBUG_ERROR, "(OC) GetOcCapabilities message failed. Library Status = %X, Domain = %X\n", LibStatus, DomainId));
      }
    }
  }


  ///
  /// Command was successful and SVID config has changed. CPU must perform a reset
  /// for SVID settings to take effect.
  ///
  if (ResetRequired) {
    DEBUG ((DEBUG_INFO, "(OC) Perform Cold Reset\n"));
    PerformWarmOrColdReset (COLD_RESET);
  }

  return Status;
}

