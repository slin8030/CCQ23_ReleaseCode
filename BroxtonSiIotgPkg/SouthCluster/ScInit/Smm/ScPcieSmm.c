/** @file
  PCH Pcie SMM Driver Entry

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
#include "ScInitSmm.h"

GLOBAL_REMOVE_IF_UNREFERENCED SC_PCIE_DEVICE_OVERRIDE     *mDevAspmOverride;
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                      mNumOfDevAspmOverride;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mPolicyRevision;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mPchBusNumber;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mTempRootPortBusNumMin;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mTempRootPortBusNumMax;

GLOBAL_REMOVE_IF_UNREFERENCED SC_PCIE_ROOT_PORT_CONFIG    mPcieRootPortConfig[PCIE_MAX_ROOT_PORTS];
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                      mLtrNonSupportRpBitMap;

GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                     mPciePmSwSmiExecuted = FALSE;

extern EFI_GUID gScDeviceTableHobGuid;

/**
  Program Common Clock and ASPM of Downstream Devices

  @param[in] PortIndex                  Pcie Root Port Number
  @param[in] RpDevice                   Pcie Root Pci Device Number
  @param[in] RpFunction                 Pcie Root Pci Function Number

  @retval EFI_SUCCESS                   Root port complete successfully
  @retval EFI_UNSUPPORTED               PMC has invalid vendor ID
**/
EFI_STATUS
ScPcieSmi (
  IN  UINT8                             PortIndex,
  IN  UINT8                             RpDevice,
  IN  UINT8                             RpFunction
  )
{
  UINT16                Data16;
  UINT8                 SecBus;
  UINT8                 SubBus;
  BOOLEAN               L1SubstatesSupported;
  EFI_HANDLE            Handle;
  UINTN                 RpBase;
  BOOLEAN               LtrSupported;
  BOOLEAN               PortWithMaxPayload128;

  Handle = NULL;
  LtrSupported = TRUE;

  RpBase   = MmPciBase (
               mPchBusNumber,
               (UINT32) RpDevice,
               (UINT32) RpFunction
               );

  if (MmioRead16 (RpBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    return EFI_SUCCESS;
  }

  //
  // Check for presense detect state
  //
  Data16 = MmioRead16 (RpBase + R_PCH_PCIE_SLSTS);
  Data16 &= B_PCIE_SLSTS_PDS;

  mLtrNonSupportRpBitMap &= ~(1 << PortIndex);

  if (Data16) {
    SecBus  = MmioRead8 (RpBase + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
    SubBus  = MmioRead8 (RpBase + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
    PortWithMaxPayload128 = TRUE;
    ASSERT (SecBus != 0 && SubBus != 0);
    PchPcieInitRootPortDownstreamDevices (
      DEFAULT_PCI_BUS_NUMBER_SC,
      (UINT8) RpDevice,
      (UINT8) RpFunction,
      mTempRootPortBusNumMin,
      mTempRootPortBusNumMax,
      &PortWithMaxPayload128
      );
    PcieSetPm (
      DEFAULT_PCI_BUS_NUMBER_SC,
      (UINT8) RpDevice,
      (UINT8) RpFunction,
      mNumOfDevAspmOverride,
      mDevAspmOverride,
      mTempRootPortBusNumMin,
      mTempRootPortBusNumMax,
      &mPcieRootPortConfig[PortIndex],
      &L1SubstatesSupported,
      mPolicyRevision,
      FALSE,
      FALSE,
      FALSE,
      &LtrSupported,
      TRUE
      );

    if (!LtrSupported) {
      mLtrNonSupportRpBitMap |= 1 << PortIndex;
    }
  }

  return EFI_SUCCESS;
}

/**
  PCIE Hotplug SMI call back function for each Root port

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.

  @retval  None
**/
VOID
EFIAPI
ScPcieSmiRpHandlerFunction (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  SC_PCIE_SMI_RP_CONTEXT            *RpContext
  )
{
  ScPcieSmi (RpContext->RpIndex, RpContext->DevNum, RpContext->FuncNum);
}

/**
  PCIE Link Active State Change Hotplug SMI call back function for all Root ports

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.

  @retval  None
**/
VOID
EFIAPI
ScPcieLinkActiveStateChange (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  SC_PCIE_SMI_RP_CONTEXT            *RpContext
  )
{
  return;
}

/**
  PCIE Link Equalization Request SMI call back function for all Root ports

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.

  @retval  None
**/
VOID
EFIAPI
ScPcieLinkEqHandlerFunction (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  SC_PCIE_SMI_RP_CONTEXT            *RpContext
  )
{
  ///
  /// From PCI Express specification, the PCIe device can request for Link Equalization. When the
  /// Link Equalization is requested by the device, an SMI will be generated  by PCIe RP when
  /// enabled and the SMI subroutine would invoke the Software Preset/Coefficient Search
  /// software to re-equalize the link.
  ///

  return;

}

/**
  An IoTrap callback to config PCIE power management settings

  @param[in]  None

  @retval  None
**/
VOID
ScPciePmIoTrapSmiCallback (
  VOID
  )
{
  UINT8                                     PortIndex;
  UINTN                                     RpBase;
  UINTN                                     RpDevice;
  UINTN                                     RpFunction;
  BOOLEAN                                   L1SubstatesSupportedPerPort;
  UINT16                                    AspmVal;
  BOOLEAN                                   ClkreqPerPortSupported;
  BOOLEAN                                   ClkreqSupportedInAllEnabledPorts;
  BOOLEAN                                   L1SupportedInAllEnabledPorts;
  BOOLEAN                                   FirstRPToSetPm;
  BOOLEAN                                   LtrSupported;
  BOOLEAN                                   IsAnyRootPortEnabled;
  UINT8                                     MaxPciePortNum;

  AspmVal                          = 0;
  L1SubstatesSupportedPerPort      = FALSE;
  ClkreqPerPortSupported           = FALSE;
  ClkreqSupportedInAllEnabledPorts = TRUE;
  L1SupportedInAllEnabledPorts     = TRUE;
  FirstRPToSetPm                   = TRUE;
  LtrSupported                     = TRUE;
  IsAnyRootPortEnabled             = FALSE;
  MaxPciePortNum                   = GetScMaxPciePortNum ();

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, (UINT32) RpDevice, (UINT32) RpFunction);

    if ((MmioRead16 (RpBase) != 0xFFFF) &&
        ((MmioRead16 (RpBase + R_PCH_PCIE_SLSTS) & B_PCIE_SLSTS_PDS) != 0)) {
      PcieCheckPmConfig (
        DEFAULT_PCI_BUS_NUMBER_SC,
        (UINT8) RpDevice,
        (UINT8) RpFunction,
        mNumOfDevAspmOverride,
        mDevAspmOverride,
        mTempRootPortBusNumMin,
        mTempRootPortBusNumMax,
        &mPcieRootPortConfig[PortIndex],
        &L1SubstatesSupportedPerPort,
        mPolicyRevision,
        &AspmVal,
        &ClkreqPerPortSupported,
        &LtrSupported
        );
      if ((AspmVal & V_PCIE_LCTL_ASPM_L1) != V_PCIE_LCTL_ASPM_L1) {
        L1SupportedInAllEnabledPorts = FALSE;
      }
      if (ClkreqPerPortSupported == FALSE) {
        ClkreqSupportedInAllEnabledPorts = FALSE;
      }
      if (!LtrSupported) {
        mLtrNonSupportRpBitMap |= 1<<PortIndex;
      }
      IsAnyRootPortEnabled  = TRUE;
    }
  }

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetScPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = MmPciBase (DEFAULT_PCI_BUS_NUMBER_SC, (UINT32) RpDevice, (UINT32) RpFunction);

    if (MmioRead16 (RpBase) != 0xFFFF) {
      PcieSetPm (
        DEFAULT_PCI_BUS_NUMBER_SC,
        (UINT8) RpDevice,
        (UINT8) RpFunction,
        mNumOfDevAspmOverride,
        mDevAspmOverride,
        mTempRootPortBusNumMin,
        mTempRootPortBusNumMax,
        &mPcieRootPortConfig[PortIndex],
        &L1SubstatesSupportedPerPort,
        mPolicyRevision,
        FirstRPToSetPm,
        L1SupportedInAllEnabledPorts,
        ClkreqSupportedInAllEnabledPorts,
        &LtrSupported,
        TRUE
        );
      FirstRPToSetPm = FALSE;
    }
  }
}

/**
  The SW SMI callback to config PCIE power management settings

  @param[in]      DispatchHandle  The handle of this callback, obtained when registering
  @param[in]      CallbackContext Pointer to the EFI_SMM_SW_REGISTER_CONTEXT
  @param[in, out] CommBuffer      Point to the CommBuffer structure
  @param[in, out] CommBufferSize  Point to the Size of CommBuffer structure
**/
VOID
ScPciePmSwSmiCallback (
  IN EFI_HANDLE                  DispatchHandle,
  IN EFI_SMM_SW_REGISTER_CONTEXT *DispatchContext,
  IN OUT VOID                    *CommBuffer,
  IN OUT UINTN                   *CommBufferSize
  )
{
//[-start-160604-IB07220092-modify]//
//   if (DispatchContext->SwSmiInputValue == mPcieRpConfig->AspmSwSmiNumber) {
    if (mPciePmSwSmiExecuted == FALSE) {
      ScPciePmIoTrapSmiCallback ();
      mPciePmSwSmiExecuted = TRUE;
    }
//   } else {
//     ASSERT_EFI_ERROR (EFI_INVALID_PARAMETER);
//   }
//[-end-160604-IB07220092-modify]//
}

/**
  This function clear the Io trap executed flag before enter S3

  @param[in]  Handle             Handle of the callback
  @param[in]  Context            The dispatch context
  @param[in, out] CommBuffer     Point to the CommBuffer structure
  @param[in, out] CommBufferSize Point to the Size of CommBuffer structure

  @retval EFI_SUCCESS  PCH register saved
**/
EFI_STATUS
EFIAPI
ScPcieS3EntryCallBack (
  IN EFI_HANDLE Handle,
  IN CONST VOID *Context OPTIONAL,
  IN OUT   VOID   *CommBuffer OPTIONAL,
  IN OUT   UINTN  *CommBufferSize OPTIONAL
  )
{
  mPciePmSwSmiExecuted = FALSE;
  return EFI_SUCCESS;
}
/**
  Register PCIE Hotplug SMI dispatch function to handle Hotplug enabling

  @param[in] ImageHandle          The image handle of this module
  @param[in] SystemTable          The EFI System Table

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
EFIAPI
InitializeScPcieSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HOB_GUID_TYPE*                    Hob;
  UINT32                                DevTableSize;
  EFI_HANDLE                            SwDispatchHandle;
  EFI_SMM_SW_REGISTER_CONTEXT           SwDispatchContext;
  EFI_HANDLE                            SxDispatchHandle;
  EFI_SMM_SX_REGISTER_CONTEXT           SxDispatchContext;
  UINT8                                 MaxPciePortNum;
  SI_POLICY_HOB                         *SiPolicyHob;

  DEBUG ((DEBUG_INFO, "InitializeScPcieSmm () Start\n"));

  MaxPciePortNum = GetScMaxPciePortNum ();

  //
  // Get Silicon Policy data HOB
  //
  Hob = GetFirstGuidHob (&gSiPolicyHobGuid);
  if (Hob == NULL) {
    ASSERT (Hob != NULL);
    return EFI_NOT_FOUND;
  }

  SiPolicyHob = GET_GUID_HOB_DATA (Hob);

  mPolicyRevision = mScPolicy->TableHeader.Header.Revision;;
  mPchBusNumber   = DEFAULT_PCI_BUS_NUMBER_SC;
  mTempRootPortBusNumMin = SiPolicyHob->TempPciBusMin;
  mTempRootPortBusNumMax = SiPolicyHob->TempPciBusMax;

  ASSERT (sizeof (mPcieRootPortConfig) == sizeof (mPcieRpConfig->RootPort));
  CopyMem (
    mPcieRootPortConfig,
    &(mPcieRpConfig->RootPort),
    sizeof (mPcieRootPortConfig)
    );

  mDevAspmOverride                  = NULL;
  mNumOfDevAspmOverride             = 0;
  mLtrNonSupportRpBitMap            = 0;

  Hob = GetFirstGuidHob (&gScDeviceTableHobGuid);
  if (Hob != NULL) {
    DevTableSize = GET_GUID_HOB_DATA_SIZE (Hob);
    ASSERT ((DevTableSize % sizeof (SC_PCIE_DEVICE_OVERRIDE)) == 0);
    mNumOfDevAspmOverride = DevTableSize / sizeof (SC_PCIE_DEVICE_OVERRIDE);
    DEBUG ((DEBUG_INFO, "Found PcieDeviceTable HOB (%d entries)\n", mNumOfDevAspmOverride));
    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      DevTableSize,
                      (VOID **) &mDevAspmOverride
                      );
    CopyMem (mDevAspmOverride, GET_GUID_HOB_DATA (Hob), DevTableSize);
  }
  //
  // Register PCIe ASPM SW SMI handler
  //
  SwDispatchHandle  = NULL;
  SwDispatchContext.SwSmiInputValue = mPcieRpConfig->AspmSwSmiNumber;
  Status = mSwDispatch->Register (
                          mSwDispatch,
                          (EFI_SMM_HANDLER_ENTRY_POINT2) ScPciePmSwSmiCallback,
                          &SwDispatchContext,
                          &SwDispatchHandle
                          );
  ASSERT_EFI_ERROR (Status);
  //
  // Register the callback for S3 entry
  //

  SxDispatchContext.Type  = SxS3;
  SxDispatchContext.Phase = SxEntry;
  Status = mSxDispatch->Register (
                          mSxDispatch,
                          ScPcieS3EntryCallBack,
                          &SxDispatchContext,
                          &SxDispatchHandle
                          );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "InitializeScPcieSmm () End\n"));

  return EFI_SUCCESS;
}
