/** @file
  This file contains routines that support PCI Express initialization

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#include "ScPciExpressHelpersLibrary.h"

//
// Tpower-on Scale
//
GLOBAL_REMOVE_IF_UNREFERENCED const UINT8 mPcieL1sTposMultiplier[] = {2, 10, 100};
GLOBAL_REMOVE_IF_UNREFERENCED const UINT8 mPclkdL1trefCfg[] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 60, 70, 80, 90, 100};

/**
  Find the Offset to a given Capabilities ID
  CAPID list:
    0x01 = PCI Power Management Interface
    0x04 = Slot Identification
    0x05 = MSI Capability
    0x10 = PCI Express Capability

  @param[in] Bus                  Pci Bus Number
  @param[in] Device               Pci Device Number
  @param[in] Function             Pci Function Number
  @param[in] CapId                CAPID to search for

  @retval 0                       CAPID not found
  @retval Other                   CAPID found, Offset of desired CAPID
**/
UINT8
PcieFindCapId (
  IN UINT8   Bus,
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT8   CapId
  )
{
  UINT8 CapHeaderOffset;
  UINT8 CapHeaderId;
  UINTN DeviceBase;

  DeviceBase = MmPciBase (Bus, Device, Function);
#ifdef EFI_DEBUG
  DEBUG((DEBUG_INFO,"PcieFindCapId () BDF %0x: %0x :%0x, CapId = %0x \n", Bus, Device, Function, CapId));
#endif
  if ((MmioRead8 (DeviceBase + PCI_PRIMARY_STATUS_OFFSET) & EFI_PCI_STATUS_CAPABILITY) == 0x00) {
    ///
    /// Function has no capability pointer
    ///
    return 0;
  } else {
    ///
    /// Check the header layout to determine the Offset of Capabilities Pointer Register
    ///
    if ((MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_LAYOUT_CODE) == (HEADER_TYPE_CARDBUS_BRIDGE)) {
      ///
      /// If CardBus bridge, start at Offset 0x14
      ///
      CapHeaderOffset = EFI_PCI_CARDBUS_BRIDGE_CAPABILITY_PTR;
    } else {
      ///
      /// Otherwise, start at Offset 0x34
      ///
      CapHeaderOffset = PCI_CAPBILITY_POINTER_OFFSET;
    }
    ///
    /// Get Capability Header, A pointer value of 00h is used to indicate the last capability in the list.
    ///
    CapHeaderId     = 0;
    CapHeaderOffset = MmioRead8 (DeviceBase + CapHeaderOffset) & ((UINT8) ~(BIT0 | BIT1));
    while (CapHeaderOffset != 0 && CapHeaderId != 0xFF) {
      CapHeaderId = MmioRead8 (DeviceBase + CapHeaderOffset);
      if (CapHeaderId == CapId) {
        if (CapHeaderOffset > PCI_MAXLAT_OFFSET) {
          ///
          /// Return valid capability offset
          ///
          return CapHeaderOffset;
        } else {
          ASSERT((FALSE));
          return 0;
        }
      }
      ///
      /// Each capability must be DWORD aligned.
      /// The bottom two bits of all pointers (including the initial pointer at 34h) are reserved
      /// and must be implemented as 00b although software must mask them to allow for future uses of these bits.
      ///
      CapHeaderOffset = MmioRead8 (DeviceBase + CapHeaderOffset + 1) & ((UINT8) ~(BIT0 | BIT1));
    }
    return 0;
  }
}

/**
  Search and return the offset of desired Pci Express Capability ID
  CAPID list:
    0x0001 = Advanced Error Reporting Capability
    0x0002 = Virtual Channel Capability
    0x0003 = Device Serial Number Capability
    0x0004 = Power Budgeting Capability

  @param[in] Bus                  Pci Bus Number
  @param[in] Device               Pci Device Number
  @param[in] Function             Pci Function Number
  @param[in] CapId                Extended CAPID to search for

  @retval 0                       CAPID not found
  @retval Other                   CAPID found, Offset of desired CAPID
**/
UINT16
PcieFindExtendedCapId (
  IN UINT8   Bus,
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT16  CapId
  )
{
  UINT16  CapHeaderOffset;
  UINT16  CapHeaderId;
  UINTN   DeviceBase;

  DeviceBase = MmPciBase (Bus, Device, Function);

  ///
  /// Start to search at Offset 0x100
  /// Get Capability Header, A pointer value of 00h is used to indicate the last capability in the list.
  ///
  CapHeaderId     = 0;
  CapHeaderOffset = R_PCH_PCIE_EXCAP_OFFSET;
  while (CapHeaderOffset != 0 && CapHeaderId != DEVICE_ID_NOCARE) {
    CapHeaderId = MmioRead16 (DeviceBase + CapHeaderOffset);
    if (CapHeaderId == CapId) {
      return CapHeaderOffset;
    }
    ///
    /// Each capability must be DWORD aligned.
    /// The bottom two bits of all pointers are reserved and must be implemented as 00b
    /// although software must mask them to allow for future uses of these bits.
    ///
    CapHeaderOffset = (MmioRead16 (DeviceBase + CapHeaderOffset + 2) >> 4) & ((UINT16) ~(BIT0 | BIT1));
  }

  return 0;
}

/**
  This function returns PID according to Root Port Number

  @param[in] RpPort      Root Port Number

  @retval SC_SBI_PID    Returns PID for SBI Access
**/
SC_SBI_PID
GetRpSbiPid (
  IN  UINTN  RpPort
)
{
  if (RpPort < 2) {
    return PID_PCIE0;
  } else {
    return PID_PCIE1;
  }
}

/**
  This function returns the Config Hide bit according to to Root Port Number

  @param[in]    RpPort          Root Port Number

  @retval       UINT32          Port Config Hide bit
**/
UINT32
GetRpConfigHideBit (
  IN UINTN              RpPort
  )
{

  switch (RpPort) {
    case 0:
      return B_PCH_PCR_SPX_PCD_RP1CH;
    case 1:
      return B_PCH_PCR_SPX_PCD_RP2CH;
    case 2:
      return B_PCH_PCR_SPX_PCD_RP1CH;
    case 3:
      return B_PCH_PCR_SPX_PCD_RP2CH;
    case 4:
      return B_PCH_PCR_SPX_PCD_RP3CH;
    case 5:
      return B_PCH_PCR_SPX_PCD_RP4CH;
    default:
      return 0;
  }
}

/**
  This function reads Pci Config register via SBI Access

  @param[in]  RpDevice            Root Port Device Number
  @param[in]  RpPort              Root Port Number
  @param[in]  Offset              Offset of Config register
  @param[out] *Data32             Value of Config register

  @retval EFI_SUCCESS             SBI Read successful.
**/
EFI_STATUS
PchSbiRpPciRead32 (
  IN    UINTN   RpDevice,
  IN    UINTN   RpPort,
  IN    UINTN   Offset,
  OUT   UINT32  *Data32
)
{
  EFI_STATUS    Status;
  UINT8         Response;
  UINT16        Fid;

  if (RpPort < 2) {
    Fid = (UINT16) ((RpDevice << 3) | RpPort);
  } else {
    Fid = (UINT16) ((RpDevice << 3) | (RpPort - 2 ));
  }
  Status = PchSbiExecutionEx (
             GetRpSbiPid (RpPort),
             Offset,
             PciConfigRead,
             FALSE,
             0xF,
             0,
             Fid,
             Data32,
             &Response
             );
  if (Status != EFI_SUCCESS) {
    DEBUG((DEBUG_ERROR,"Side band Read Failed\n"));
  }
  return Status;
}

/**
  This function And then Or Pci Config register via SBI Access

  @param[in]  RpDevice            Root Port Device Number
  @param[in]  RpPort              Root Port Number
  @param[in]  Offset              Offset of Config register
  @param[in]  Data32And           Value of Config register to be And-ed
  @param[in]  Data32AOr           Value of Config register to be Or-ed

  @retval EFI_SUCCESS             SBI Read and Write successful.
**/
EFI_STATUS
PchSbiRpPciAndThenOr32 (
  IN  UINTN   RpDevice,
  IN  UINTN   RpPort,
  IN  UINTN   Offset,
  IN  UINT32  Data32And,
  IN  UINT32  Data32Or
)
{
  EFI_STATUS  Status;
  UINT32      Data32;
  UINT8       Response;
  UINT16      Fid;

  Status = PchSbiRpPciRead32 (RpDevice, RpPort, Offset, &Data32);
  if (Status == EFI_SUCCESS) {
    Data32 &= Data32And;
    Data32 |= Data32Or;
    if (RpPort < 2) {
      Fid = (UINT16) ((RpDevice << 3) | RpPort);
    } else {
      Fid = (UINT16) ((RpDevice << 3) | (RpPort - 2 ));
    }
    Status = PchSbiExecutionEx (
               GetRpSbiPid (RpPort),
               Offset,
               PciConfigWrite,
               FALSE,
               0xF,
               0,
               Fid,
               &Data32,
               &Response
               );
    if (Status != EFI_SUCCESS) {
      DEBUG((DEBUG_ERROR,"Side band Write Failed\n"));
    }
  }
  return Status;
}

/**
  This function returns the maximum number of ClkReq signals available

  @retval UINT8     Returns maximum number of ClkReq signals
**/
UINT8
GetClkReqMax ()
{
   return SC_PCIE_MAX_CLK_REQ;
}

/**
  This returns ClkReq Number from Port Number

  @param[in]    PortIndex     PCIe Port Number (Zero Base)

  @retval       ClkReq Number
**/
UINT8
GetPortClkReqNumber (
  IN      UINT8           PortIndex
)
{
  UINT8 ClkReqNum;

  if (GetBxtSeries () == BxtP) {
    if (PortIndex < 2) {
      PortIndex += 4;
    } else {
      PortIndex -= 2;
    }
  }
  PchPcrRead8 (PID_FIA, R_SC_PCR_FIA_DRCRM1 + (PortIndex / 2), &ClkReqNum);
  if (PortIndex % 2 == 0) {
    ClkReqNum &= 0x0F;
  } else {
    ClkReqNum = ClkReqNum >> 4;
  }
  return ClkReqNum;
}

/**
  This function assigns a ClkReq signal to Pcie ports and returns updated ClkReq Number

  @param[in]    PcieConfig      PCH Pcie Configuration
  @param[in]    PortIndex       PCIe Port Number (Zero Base)

  @retval EFI_SUCCESS   Successfully set ClkReq Number to Root Port
**/
EFI_STATUS
SetPortClkReqNumber (
  IN      SC_PCIE_CONFIG         *PcieConfig,
  IN      UINT8                   PortIndex
)
{

#ifdef EFI_DEBUG
  UINT32      Drcrm1;
#endif
  EFI_STATUS  Status;
  UINT8       ClkReqNum;

  Status = EFI_SUCCESS;

  ClkReqNum = (UINT8) PcieConfig->RootPort[PortIndex].ClkReqNumber;
  ///
  /// CLKREQ to Root Port Mapping
  /// The mapping of the PCIeExpress Ports to the CLKREQ# pins can be
  /// specified through the DRCRM{x} registers
  ///
  if (GetBxtSeries () == BxtP) {
    if (PortIndex < 2) {
      PortIndex += 4;
    } else {
      PortIndex -= 2;
    }
  }
  if (PortIndex % 2 == 0) {
    PchPcrAndThenOr8 (PID_FIA, R_SC_PCR_FIA_DRCRM1 + (PortIndex / 2), 0xF0, ClkReqNum);
  } else {
    PchPcrAndThenOr8 (PID_FIA, R_SC_PCR_FIA_DRCRM1 + (PortIndex / 2), 0x0F, (ClkReqNum) << 4);
  }
#ifdef EFI_DEBUG
  PchPcrRead32 (PID_FIA, R_SC_PCR_FIA_DRCRM1, &Drcrm1);
  DEBUG((DEBUG_INFO,"AssignClkReq = %0x\n", Drcrm1));
#endif
  return Status;
}
/**
  Set Common clock to Root port and Endpoint PCI device

  @param[in] Bus1                 Root port Pci Bus Number
  @param[in] Device1              Root port Pci Device Number
  @param[in] Function1            Root port Pci Function Number
  @param[in] Bus2                 Endpoint Pci Bus Number
  @param[in] Device2              Endpoint Pci Device Number

  @exception EFI_UNSUPPORTED      Unsupported operation.
  @retval EFI_SUCCESS             VC mapping correctly initialized
**/
EFI_STATUS
PcieSetCommonClock (
  IN UINT8   Bus1,
  IN UINT8   Device1,
  IN UINT8   Function1,
  IN UINT8   Bus2,
  IN UINT8   Device2
  )
{
  UINT8       CapOffset1;
  UINT8       CapOffset2;
  BOOLEAN     CommonClockSupport;
  EFI_STATUS  Status;
  UINTN       DeviceBase1;
  UINTN       DeviceBase2;
  UINT16      RegData16;
  UINT8       FunctionIndex;
  UINT8       Function2;

  DeviceBase1 = MmPciBase (Bus1, Device1, Function1);

  ///
  /// Get the pointer to the Port PCI Express Capability Structure.
  ///
  CommonClockSupport  = FALSE;
  CapOffset1          = PcieFindCapId (Bus1, Device1, Function1, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (CapOffset1 == 0) {
    return EFI_UNSUPPORTED;
  }
  ///
  /// Check the Port Slot Clock Configuration Bit.
  ///
  if ((MmioRead16 (DeviceBase1 + CapOffset1 + R_PCIE_LSTS_OFFSET) & B_PCIE_LSTS_SCC) == 0) {
    return EFI_UNSUPPORTED;
  }

  DeviceBase2 = MmPciBase (Bus2, Device2, 0);
  ///
  /// Check if EndPoint device is Multi-Function Device
  ///
  if (MmioRead8 (DeviceBase2 + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
    ///
    /// If multi-function Device, check function 0-7
    ///
    Function2 = PCI_MAX_FUNC;
  } else {
    ///
    /// Otherwise, check function 0 only
    ///
    Function2 = 0;
  }

  for (FunctionIndex = 0; FunctionIndex <= Function2; FunctionIndex++) {
    DeviceBase2 = MmPciBase (Bus2, Device2, FunctionIndex);
    ///
    /// Check the Endpoint Slot Clock Configuration Bit.
    ///
    CapOffset2 = PcieFindCapId (Bus2, Device2, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
    if ((CapOffset2 != 0) && ((MmioRead16 (DeviceBase2 + CapOffset2 + R_PCIE_LSTS_OFFSET) & B_PCIE_LSTS_SCC) != 0)) {
      ///
      /// Common clock is supported, set common clock bit on root port
      /// and the endpoint
      ///
      if (CommonClockSupport == FALSE) {
        MmioOr8 (DeviceBase1 + CapOffset1 + R_PCIE_LCTL_OFFSET, B_PCIE_LCTL_CCC);
        CommonClockSupport = TRUE;
      }
      MmioOr8 (DeviceBase2 + CapOffset2 + R_PCIE_LCTL_OFFSET, B_PCIE_LCTL_CCC);
    }
  }
  ///
  /// If common clock not supported on root port and endpoint, return EFI_UNSUPPORTED
  ///
  if (CommonClockSupport == FALSE) {
    Status = EFI_UNSUPPORTED;
  } else {
    Status = EFI_SUCCESS;
  }
  ///
  /// Retrain the Link per PCI Express Specification.
  ///
  MmioOr8 (DeviceBase1 + CapOffset1 + R_PCIE_LCTL_OFFSET, B_PCIE_LCTL_RL);

  ///
  /// Wait until Re-Training has completed.
  ///
  do {
    RegData16 = MmioRead16 (DeviceBase1 + CapOffset1 + R_PCIE_LSTS_OFFSET) & B_PCIE_LSTS_LT;
  } while (RegData16 != 0);

  return Status;
}

/**
  This function checks whether PHY lane power gating is enable on the port.

  @param[in] RpBase                 Root Port base address

  @retval TRUE                      PHY power gating is enabled
  @retval FALSE                     PHY power gating disabled
**/
STATIC
BOOLEAN
PcieIsPhyLanePgEnabled (
  IN     UINTN  RpBase
  )
{
  UINT32 Data32;
  Data32 = MmioRead32(RpBase + R_PCH_PCIE_PCIEPMECTL);
  return (Data32 & B_PCH_PCIE_PCIEPMECTL_DLSULPPGE) != 0;
}

/**
  This function enables the CLKREQ# PM on all the end point functions

  @param[in] Bus                  Pci Bus Number
  @param[in] Device               Pci Device Number
  @param[in] RootDevice           Rootport Device Number
  @param[in] RootFunction         Rootport Function Number

  @retval None
**/
VOID
PcieSetClkreq (
  IN  UINT8   EndPointBus,
  IN  UINT8   EndPointDevice,
  IN  UINT8   RootDevice,
  IN  UINT8   RootFunction
  )
{
  UINT8       CapOffset;
  UINTN       DeviceBase;
  UINT8       FunctionIndex;
  UINT8       Function;
  BOOLEAN     ClkreqPerPortSupported;

  DeviceBase = MmPciBase (EndPointBus, EndPointDevice, 0);
  ClkreqPerPortSupported = TRUE;

  ///
  /// Check if EndPoint device is Multi-Function Device
  ///
  if (MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
    ///
    /// If multi-function Device, check function 0-7
    ///
    Function = PCI_MAX_FUNC;
  } else {
    ///
    /// Otherwise, check function 0 only
    ///
    Function = 0;
  }
  ///
  /// Parse thro all the functions of the endpoint and find the PCIe Cap ID (offset 10h) and if
  /// exists then enable the CLKREQ# bit (BIT8) on that function
  ///
  for (FunctionIndex = 0; FunctionIndex <= Function; FunctionIndex++) {
    ///
    /// Find the PCIe Cap Id (offset 10h)
    ///
    CapOffset = PcieFindCapId (EndPointBus, EndPointDevice, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
    if (CapOffset == 0) {
      continue;
    }

    DeviceBase = MmPciBase (EndPointBus, EndPointDevice, FunctionIndex);
    ///
    /// Check if CLKREQ# is supported by the endpoints
    ///
    if ((MmioRead32 (DeviceBase + CapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_CPM) == 0) {
      ///
      /// CLKREQ# is not supported so dont do anything
      ///
      ClkreqPerPortSupported = FALSE;
      break;
    }
  }

  if (ClkreqPerPortSupported == FALSE) {
    return;
  }
  ///
  /// Now enable the CLKREQ#
  ///
  for (FunctionIndex = 0; FunctionIndex <= Function; FunctionIndex++) {
    ///
    /// Find the PCIe Cap Id (offset 10h)
    ///
    CapOffset = PcieFindCapId (EndPointBus, EndPointDevice, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
    if (CapOffset == 0) {
      continue;
    }

    DeviceBase = MmPciBase (EndPointBus, EndPointDevice, FunctionIndex);
    MmioOr16 (DeviceBase + CapOffset + R_PCIE_LCTL_OFFSET, B_PCIE_LCTL_ECPM);
  }
}
/**
  This function get or set the Max Payload Size on all the end point functions

  @param[in] EndPointBus          The Bus Number of the Endpoint
  @param[in] EndPointDevice       The Device Number of the Endpoint
  @param[in, out] MaxPayload      The Max Payolad Size of the root port
  @param[in] Operation            True:  Set the Max Payload Size on all the end point functions
                                  False: Get the Max Payload Size on all the end point functions

  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieMaxPayloadSize (
  IN      UINT8           EndPointBus,
  IN      UINT8           EndPointDevice,
  IN OUT  UINT16          *MaxPayload,
  IN      BOOLEAN         Operation
  )
{
  UINTN   DeviceBase;
  UINT8   PcieCapOffset;
  UINT16  EndPointMaxPayload;
  UINT8   FunctionIndex;
  UINT8   EndPointFunction;

  ///
  /// Obtain the Max Payload Size for all the end point functions
  ///
  DeviceBase = MmPciBase (EndPointBus, EndPointDevice, 0);
  ///
  /// Check if EndPoint device is Multi-Function Device
  ///
  if (MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
    ///
    /// If multi-function Device, check function 0-7
    ///
    EndPointFunction = PCI_MAX_FUNC;
  } else {
    ///
    /// Otherwise, check function 0 only
    ///
    EndPointFunction = 0;
  }

  for (FunctionIndex = 0; FunctionIndex <= EndPointFunction; FunctionIndex++) {
    DeviceBase = MmPciBase (EndPointBus, EndPointDevice, FunctionIndex);
    if (MmioRead16 (DeviceBase + PCI_VENDOR_ID_OFFSET) != DEVICE_ID_NOCARE) {
      ///
      /// Get the pointer to the Endpoint PCI Express Capability Structure.
      ///
      PcieCapOffset = PcieFindCapId (EndPointBus, EndPointDevice, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
      if (PcieCapOffset == 0) {
        continue;
      }

      if (Operation == TRUE) {
        ///
        /// Set the Max Payload Size of the end point function
        ///
        MmioAndThenOr16 (
          DeviceBase + PcieCapOffset + R_PCIE_DCTL_OFFSET,
          (UINT16)~(BIT7 | BIT6 | BIT5),
          *MaxPayload << 5
          );
      } else {
        ///
        /// Get the end point function Max Payload Size support
        ///
        EndPointMaxPayload = MmioRead16 (DeviceBase + PcieCapOffset + R_PCIE_DCAP_OFFSET) & B_PCIE_DCAP_MPS;
        ///
        /// Obtain the minimum Max Payload Size between the PCIE root Port and the end point functions
        ///
        if (*MaxPayload > EndPointMaxPayload) {
          *MaxPayload = EndPointMaxPayload;
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  This function disable the forwarding of EOI messages unless it discovers
  an IOAPIC behind this root port.

  @param[in] RootBus              The Bus Number of the root port
  @param[in] RootDevice           The Device Number of the root port
  @param[in] RootFunction         The Function Number of the root port
  @param[in] EndPointBus          The Bus Number of the Endpoint
  @param[in] EndPointDevice       The Device Number of the Endpoint

  @exception EFI_UNSUPPORTED      Unsupported operation.
  @retval EFI_SUCCESS             Successfully completed.
**/
EFI_STATUS
PcieSetEoiFwdDisable (
  IN  UINT8   RootBus,
  IN  UINT8   RootDevice,
  IN  UINT8   RootFunction,
  IN  UINT8   EndPointBus,
  IN  UINT8   EndPointDevice
  )
{
/* TODO
  BOOLEAN IoApicBehind;
  UINTN   RootDeviceBase;
  UINTN   DeviceBase;
  UINT8   ProgInterface;
  UINT8   SubClassCode;
  UINT8   BaseClassCode;
  UINT8   FunctionIndex;
  UINT8   EndPointFunction;
  UINTN   RootPortNumber;
  UINT32  Data32;
  UINT8   NumMcVal;
  UINT32  RpDestId;
  static UINT32  PchLpRpDestId[PCH_LP_PCIE_MAX_ROOT_PORTS] = {0x18100, 0x18101, 0x18102, 0x18103, 0x18300, 0x18301, 0x18302, 0x18303, 0x18500, 0x18501, 0x18502, 0x18503};
  static UINT32  PchHRpDestId[PCH_H_PCIE_MAX_ROOT_PORTS] = {0x18000, 0x18001, 0x18002, 0x18003, 0x18100, 0x18101, 0x18102, 0x18103, 0x18300, 0x18301, 0x18302, 0x18303, 0x18500, 0x18501, 0x18502, 0x18503, 0x18700, 0x18701, 0x18702, 0x18703};
  PCH_SERIES  PchSeries;
  UINT16      PsfReg;
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  IoApicBehind    = FALSE;
  RootDeviceBase  = MmPciBase (RootBus, RootDevice, RootFunction);
  PchSeries = GetPchSeries ();

  ///
  /// Check if an IOAPIC behind the root port
  ///
  DeviceBase = MmPciBase (EndPointBus, EndPointDevice, 0);
  ///
  /// Check if EndPoint device is Multi-Function Device
  ///
  if (MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
    ///
    /// If multi-function Device, check function 0-7
    ///
    EndPointFunction = PCI_MAX_FUNC;
  } else {
    ///
    /// Otherwise, check function 0 only
    ///
    EndPointFunction = 0;
  }

  for (FunctionIndex = 0; FunctionIndex <= EndPointFunction; FunctionIndex++) {
    DeviceBase    = MmPciBase (EndPointBus, EndPointDevice, FunctionIndex);
    BaseClassCode = MmioRead8 (DeviceBase + PCI_CLASSCODE_OFFSET + 2);
    SubClassCode  = MmioRead8 (DeviceBase + PCI_CLASSCODE_OFFSET + 1);
    ProgInterface = MmioRead8 (DeviceBase + PCI_CLASSCODE_OFFSET);
     if ((BaseClassCode == PCI_CLASS_SYSTEM_PERIPHERAL) &&
        (SubClassCode == PCI_SUBCLASS_PIC) &&
        ((ProgInterface == PCI_IF_APIC_CONTROLLER) ||
         (ProgInterface == PCI_IF_APIC_CONTROLLER2))) {
      IoApicBehind = TRUE;
    }
  }
  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Step 27
  /// If there is no IOAPIC behind the root port, set EOI Forwarding Disable bit (PCIE RP PCI offset D4h[1]) to 1b.
  ///
  if (IoApicBehind == FALSE) {
    MmioOr8 (RootDeviceBase + R_PCH_PCIE_MPC2, (UINT8) (BIT1));
  } else {
    ///
    /// If there is an IOAPIC discovered behind root port program PSF Multicast registers
    /// accordingly to SKL PCH BWG 5.14.4 PSF EOI Multicast Configuration
    ///
    ///
    ///
    ///
    /// Read PSF_1_PSF_MC_CONTROL_MCAST0_RS0_EOI bit [7:1], increase it by 1, write back
    ///
    if (PchSeries == PchLp) {
      PsfReg = R_PCH_LP_PCR_PSF1_PSF_MC_CONTROL_MCAST0_RS0_EOI;
    } else {
      PsfReg = R_PCH_H_PCR_PSF1_PSF_MC_CONTROL_MCAST0_RS0_EOI;
    }
    PchPcrRead32 (PID_PSF1, PsfReg, &Data32);
    NumMcVal = (UINT8)(Data32 >> 1);
    Data32 += 0x2;
    PchPcrWrite32 (PID_PSF1, PsfReg, Data32);

    ///
    /// Program PSF_1_PSF_MC_AGENT_MCAST0_RS0_TGT<x>_EOI, the next empty register,
    /// where x is the same as the value read in step 1 before the increment.
    /// Program this register according to the RP# and the table below
    ///
    /// PCH-LP:
    /// RP Destination ID  RP  Destination ID
    ///  1     0x18100      7     0x18302
    ///  2     0x18101      8     0x18303
    ///  3     0x18102      9     0x18500
    ///  4     0x18103      10    0x18501
    ///  5     0x18300      11    0x18502
    ///  6     0x18301      12    0x18503
    ///
    /// PCH-H:
    /// RP Destination ID  RP  Destination ID
    ///  1     0x18000      11    0x18302
    ///  2     0x18001      12    0x18303
    ///  3     0x18002      13    0x18500
    ///  4     0x18003      14    0x18501
    ///  5     0x18100      15    0x18502
    ///  6     0x18101      16    0x18503
    ///  7     0x18102      17    0x18700
    ///  8     0x18103      18    0x18701
    ///  9     0x18300      19    0x18702
    ///  10    0x18301      20    0x18703
    ///

    Status = GetScPcieRpNumber ((UINTN)RootDevice, (UINTN)RootFunction,&RootPortNumber);
    ASSERT_EFI_ERROR (Status);

    if (PchSeries == PchLp) {
      PsfReg = R_PCH_LP_PCR_PSF1_PSF_MC_AGENT_MCAST0_RS0_TGT0_EOI;
      RpDestId = PchLpRpDestId[RootPortNumber - 1];
    } else {
      PsfReg = R_PCH_H_PCR_PSF1_PSF_MC_AGENT_MCAST0_RS0_TGT0_EOI;
      RpDestId = PchHRpDestId[RootPortNumber - 1];
    }

    PchPcrAndThenOr32 (
      PID_PSF1, PsfReg + (NumMcVal * 4),
      (UINT32)~0,
      RpDestId
      );
  }

  return Status;
*/
  return 0; //TO remove
}

typedef enum {
  CalculateAspm,
  ManualAspm,
  SetAspm
} OPERATION;

/**
  This function compares the actual latency in LatencyValue1
  with actual latency in LatencyValue2 and stores the minimum
  back to LatencyValue1, in the required format.
  If this is the first call, then LatencyValue1 will be replaced by LatencyValue2.

  @param[in, out] LatencyValue1  - Current latency value
  @param[in] LatencyValue2       - Latency value from the Table

  @retval    None
**/
VOID
DetermineLatencyValue (
  IN OUT  UINT16   *LatencyValue1,
  IN      UINT16   LatencyValue2
  )
{
  ASSERT (LTR_SCALE_VALUE (*LatencyValue1) < 6);
  ASSERT (LTR_SCALE_VALUE (LatencyValue2) < 6);
  ///
  /// If there are more than one device behind a bridge that are part of the override table,
  /// store the lower latency value and corresponding scale bits back to LatencyValue1
  ///
  if ((LTR_LATENCY_NS(*LatencyValue1) == 0) || (LTR_LATENCY_NS(*LatencyValue1) > LTR_LATENCY_NS(LatencyValue2))) {
    *LatencyValue1 = LatencyValue2;
  }
}

/**
  This function checks exit latency of L1 and L0s and disables the ASPM state if it is longer than
  the acceptable latency

  @param[in] EndPointBase             End Point Base Address
  @param[in] EndPointPcieCapOffset    The pointer to the End Point PCI Express Capability Structure
  @param[in] RootDeviceBase           The Root Port PCI Express Base address
  @param[in] RootPcieCapOffset        The pointer to the Root Port PCI Express Capability Structure
  @param[in, out] EndPointAspm        End Point ASPM setting

  @retval none
**/
VOID AspmCheckExitLatency (
  IN     UINTN                           EndPointBase,
  IN     UINT8                           EndPointPcieCapOffset,
  IN     UINTN                           RootDeviceBase,
  IN     UINT32                          RootPcieCapOffset,
  IN OUT UINT16*                         EndPointAspm
  )
{

  UINT32      PortLxLat;
  UINT32      EndPointLxLat;
  UINT32      LxLat;

  ///
  /// Check if L1 should be enabled based on port and endpoint L1 exit latency.
  ///
  if ((*EndPointAspm) & BIT1) {
    PortLxLat     = MmioRead32 (RootDeviceBase + RootPcieCapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_EL1;
    EndPointLxLat = MmioRead32 (EndPointBase + EndPointPcieCapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_EL1;

    LxLat = PortLxLat;
    if (PortLxLat < EndPointLxLat) {
      LxLat = EndPointLxLat;
    }
    ///
    /// check if the value is bigger than endpoint L1 acceptable exit latency, if it is
    /// larger than accepted value, then we should disable L1
    ///
    LxLat >>= N_PCIE_LCAP_EL1;
    if (LxLat > ((MmioRead32 (EndPointBase + EndPointPcieCapOffset + R_PCIE_DCAP_OFFSET) & B_PCIE_DCAP_E1AL) >> N_PCIE_DCAP_E1AL)) {
      (*EndPointAspm) &= ~BIT1;
    }
  }
  ///
  /// Check if L0s should be enabled based on port and endpoint L0s exit latency.
  ///
  if ((*EndPointAspm) & BIT0) {
    PortLxLat     = MmioRead32 (RootDeviceBase + RootPcieCapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_EL0;
    EndPointLxLat = MmioRead32 (EndPointBase + EndPointPcieCapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_EL0;

    LxLat = PortLxLat;
    if (PortLxLat < EndPointLxLat) {
      LxLat = EndPointLxLat;
    }
    ///
    /// check if the value is bigger than endpoint L0s acceptable exit latency, if it is
    /// larger than accepted value, then we should disable L0s
    ///
    LxLat >>= N_PCIE_LCAP_EL0;
    if (LxLat > ((MmioRead32 (EndPointBase + EndPointPcieCapOffset + R_PCIE_DCAP_OFFSET) & B_PCIE_DCAP_E0AL) >> N_PCIE_DCAP_E0AL)) {
      (*EndPointAspm) &= ~BIT0;
    }
  }
  return;
}
/**
  This function gets override Aspm values if the end point is found in the override look up table

  @param[in] EndPointBase             End Point Base Address
  @param[in] NumOfDevAspmOverride     Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride          Pointer to array of Device specific ASPM policy override items
  @param[in] EndPointVendorId         End Point Vendor Id
  @param[in] EndPointDeviceId         End Point Device Id
  @param[in] EndPointRevId            End Point Revision Id
  @param[in, out] EndPointAspm        End Point ASPM setting

  @retval none
**/

VOID GetOverrideAspm (
  IN      UINTN                           EndPointBase,
  IN      UINT32                          NumOfDevAspmOverride,
  IN      CONST SC_PCIE_DEVICE_OVERRIDE   *DevAspmOverride,
  IN      UINT16                          EndPointVendorId,
  IN      UINT16                          EndPointDeviceId,
  IN      UINT8                           EndPointRevId,
  IN OUT  UINT16                          *EndPointAspm
  )
{
  UINT8 EndPointBaseClassCode;
  UINT8 EndPointSubClassCode;
  UINT8 PcieDeviceIndex;
  ///
  /// Mask APMC with values from lookup table.
  /// RevID of 0xFF applies to all steppings.
  ///
  EndPointBaseClassCode = MmioRead8 (EndPointBase + R_PCI_BCC_OFFSET);
  EndPointSubClassCode = MmioRead8 (EndPointBase + R_PCI_SCC_OFFSET);
  for (PcieDeviceIndex = 0; PcieDeviceIndex < NumOfDevAspmOverride; PcieDeviceIndex++) {
    if (((DevAspmOverride[PcieDeviceIndex].OverrideConfig & ScPcieL1L2Override) == ScPcieL1L2Override) &&
      ((DevAspmOverride[PcieDeviceIndex].VendorId == EndPointVendorId) ||
        (DevAspmOverride[PcieDeviceIndex].VendorId == 0xFFFF)) &&
        ((DevAspmOverride[PcieDeviceIndex].DeviceId == EndPointDeviceId) ||
            (DevAspmOverride[PcieDeviceIndex].DeviceId == 0xFFFF)) &&
        ((DevAspmOverride[PcieDeviceIndex].RevId == EndPointRevId) ||
            (DevAspmOverride[PcieDeviceIndex].RevId == 0xFF)) &&
        ((DevAspmOverride[PcieDeviceIndex].BaseClassCode == EndPointBaseClassCode) ||
            (DevAspmOverride[PcieDeviceIndex].BaseClassCode == 0xFF)) &&
        ((DevAspmOverride[PcieDeviceIndex].SubClassCode == EndPointSubClassCode) ||
            (DevAspmOverride[PcieDeviceIndex].SubClassCode == 0xFF))) {
      ///
      /// Override value of 0xFF applies to all.
      ///
      *EndPointAspm = DevAspmOverride[PcieDeviceIndex].EndPointAspm;
      break;
    }
  }
}

/**
  This function gets override L1 Substate Capability offset pointer
  if the end point is found in the override look up table

  @param[in] EndPointBase                      End Point Base Address
  @param[in] NumOfDevAspmOverride              Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride                   Pointer to array of Device specific ASPM policy override items
  @param[in] EndPointVendorId                  End Point Vendor Id
  @param[in] EndPointDeviceId                  End Point Device Id
  @param[in] EndPointRevId                     End Point Revision Id
  @param[in, out] EndPointL1SubStateCapOffset  Pointer to L1 Substate Capability Structure
  @param[in, out] EndPointL1SubStateCapMask    L1 Substate Capability Mask

  @retval none
**/

VOID GetOverrideL1sCapOffset (
  IN     UINTN                            EndPointBase,
  IN     UINT32                           NumOfDevAspmOverride,
  IN     CONST SC_PCIE_DEVICE_OVERRIDE    *DevAspmOverride,
  IN     UINT16                           EndPointVendorId,
  IN     UINT16                           EndPointDeviceId,
  IN     UINT8                            EndPointRevId,
  IN OUT UINT16*                          EndPointL1SubStateCapOffset,
  IN OUT UINT32*                          EndPointL1SubStateCapMask
  )
{
  UINT8 PcieDeviceIndex;
  ///
  /// Get the endpoint supports L1 Substates Capabilities from Override Table
  ///
  for (PcieDeviceIndex = 0; PcieDeviceIndex < NumOfDevAspmOverride; PcieDeviceIndex++) {
    if (((DevAspmOverride[PcieDeviceIndex].OverrideConfig & ScPcieL1SubstatesOverride) == ScPcieL1SubstatesOverride) &&
      (EndPointVendorId == DevAspmOverride[PcieDeviceIndex].VendorId) &&
      (EndPointDeviceId == DevAspmOverride[PcieDeviceIndex].DeviceId) &&
      ((EndPointRevId == DevAspmOverride[PcieDeviceIndex].RevId) ||
        (DevAspmOverride[PcieDeviceIndex].RevId == 0xFF))) {
      if ((EndPointVendorId == V_INTEL_VENDOR_ID) &&
        ((EndPointDeviceId == 0x08B1) || (EndPointDeviceId == 0x08B2) ||
          (EndPointDeviceId == 0x08B3) || (EndPointDeviceId == 0x08B4))
          && ((MmioRead32(EndPointBase + DevAspmOverride[PcieDeviceIndex].L1SubstatesCapOffset) & 0xFFFF) != 0xCAFE)){
        continue;
      }
      *EndPointL1SubStateCapOffset =
        DevAspmOverride[PcieDeviceIndex].L1SubstatesCapOffset;
      *EndPointL1SubStateCapMask =
        DevAspmOverride[PcieDeviceIndex].L1SubstatesCapMask;
      break;
    }
  }
}

/**
  This function configures the L1 Substates.
  It can be used for Rootport and endpoint devices.

  @param[in] RootPortConfig               Rootport PCI Express Configuration
  @param[in] Bus                          Rootport/Endpoint Bus Number
  @param[in] Device                       Rootport/Endpoint Device Number
  @param[in] Function                     Rootport/Endpoint Function Number
  @param[in] EndPointL1SubStateCapOffset  Pointer to L1 Substate Capability Structure
  @param[in] PortCommonModeRestoreTime    Common Mode Restore Time
  @param[in] PortTpowerOnValue            Tpower_on Power On Wait Time
  @param[in] PortTpowerOnScale            Tpower-on Scale

  @retval none
**/
VOID ConfigureL1s (
  IN const  SC_PCIE_ROOT_PORT_CONFIG* RootPortConfig,
  IN UINT8  Bus,
  IN UINT8  Device,
  IN UINT8  Function,
  IN UINTN  DeviceBase,
  IN UINT16 L1SubstateExtCapOffset,
  IN UINT32 PortCommonModeRestoreTime,
  IN UINT32 PortTpowerOnValue,
  IN UINT32 PortTpowerOnScale
  )
{
  UINT32 Data32Or;
  UINT32 Data32;
  UINT32 Tpoweron;
  UINT8  ClkReqNum;
  UINTN  PortIndex;

  ///
  /// PCH BIOS Spec Section 8.3 ASPM on DMI and the PCI Express* Root Ports
  /// Step 6
  /// a. Set L1 Sub-States Extended Capability Offset + 0x08[4:0] to 01111b for both Pcie root port and device
  ///
  Data32Or = (BIT3 | BIT2 | BIT1 | BIT0);
  if (RootPortConfig->L1Substates == ScPcieL1SubstatesL1_1) {
    Data32Or &= (UINT32) ~(BIT0);
  }
  if (RootPortConfig->L1Substates == ScPcieL1SubstatesL1_2) {
    Data32Or &= (UINT32) ~(BIT1);
  }
  MmioAndThenOr32 (
                   DeviceBase + L1SubstateExtCapOffset + R_PCIE_EX_L1SCTL1_OFFSET,
                   (UINT32) ~(BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
                   Data32Or
                   );
  ///
  /// b. Read L1 Sub-States Extended Capability Offset + 0x04[15:8], and Set the highest value advertised
  ///    between PCIe rootport and device to L1 Sub-States Extended Capability Offset + 0x08[15:8] on both
  ///    Pcie root port and device.
  ///
  MmioAndThenOr32 (
                   DeviceBase + L1SubstateExtCapOffset + R_PCIE_EX_L1SCTL1_OFFSET,
                   (UINT32) ~(0xFF00),
                   (UINT32) PortCommonModeRestoreTime << 8
                   );
  ///
  /// c. Read L1 Sub-States Extended Capability Offset + 0x04[23:19] and [17:16], and Set the highest value
  ///    advertised between PCIe root port and device to L1 Sub-States Extended Capability Offset + 0x0C [7:0] on
  ///    both Pcie root port and device.
  ///
  MmioAndThenOr32 (
                   DeviceBase + L1SubstateExtCapOffset + R_PCIE_EX_L1SCTL2_OFFSET,
                   0xFFFFFF04,
                   (UINT32) ((PortTpowerOnValue << N_PCIE_EX_L1SCTL2_POWT) | PortTpowerOnScale)
                   );
  ///
  /// d. Set L1 Sub-States Extended Capability Offset + 0x08[31:29] to 010b for both Pcie root port and device
  /// e. Set L1 Sub-States Extended Capability Offset + 0x08[25:16] to 0010100000b for both Pcie root port and device
  ///
  MmioAndThenOr32 (
                   DeviceBase + L1SubstateExtCapOffset
                       + R_PCIE_EX_L1SCTL1_OFFSET,
                   (UINT32) ~(0xE3FF0000),
                   (UINT32) (BIT30 | BIT23 | BIT21));

  ///
  /// If Root port,
  ///
  if (Bus == DEFAULT_PCI_BUS_NUMBER_SC) {
    GetScPcieRpNumber (Device, Function, &PortIndex);
    ClkReqNum = GetPortClkReqNumber ((UINT8) PortIndex);
    ///
    /// f.  Set Dxx:Fn:420h[0] to 1b prior to L1 enabling
    ///
    MmioOr32 (DeviceBase + R_PCH_PCIE_PCIEPMECTL, B_PCH_PCIE_PCIEPMECTL_L1FSOE);
    ///
    /// g. Set PCR[PCLKD] +1010h[23:0] to a value (in time unit) bigger than Tpoweron from step c.
    /// For the hotplug port but empty, set it to 45us just in case SMI is not enabled to handle this.
    ///
    Tpoweron = PortTpowerOnValue * mPcieL1sTposMultiplier[PortTpowerOnScale];
    Data32 = 0;
    if ((MmioRead16 (DeviceBase + R_PCH_PCIE_SLSTS) & B_PCIE_SLSTS_PDS) != 0) {
      while ((Tpoweron > mPclkdL1trefCfg[Data32]) && (Data32 < 15)) {
        Data32++;
      }
    } else {
      Data32 = 9;
    }
    Data32 = (Data32 << (S_PCR_PCLKD_L1TREF_CFG_TCLKREQSRC * ClkReqNum));
    SideBandAndThenOr32 (PID_PCLKD, R_PCR_PCLKD_L1TREF_CFG, ~0u, Data32);
  }
}

/**
  This function gets the Latency Tolerance Reporting settings from override table
  if the end point is found in the override look up table

  @param[in] NumOfDevAspmOverride   Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride        Pointer to array of Device specific ASPM policy override items
  @param[in] EndPointVendorId       End Point Vendor Id
  @param[in] EndPointDeviceId       End Point Device Id
  @param[in] EndPointRevId          End Point Revision Id
  @param[in, out] LtrOverrideVal    Snoop and Non Snoop Latency Values

  @retval none
**/
VOID GetLtrOverride (
  IN      UINT32                        NumOfDevAspmOverride,
  IN      CONST SC_PCIE_DEVICE_OVERRIDE *DevAspmOverride,
  IN      UINT16                        EndPointVendorId,
  IN      UINT16                        EndPointDeviceId,
  IN      UINT8                         EndPointRevId,
  IN OUT  UINT32                        *LtrOverrideVal
  )
{
  UINT8   PcieDeviceIndex;
  UINT16  Data16;
  UINT32  Data32;
  ///
  /// For each device detected, scan the LTR override table
  /// If there are endpoints connected directly to the rootport then
  /// LtrOverrideVal will be replaced by the value from the table for that endpoint
  /// If there are endpoints that are behind a bridge and that are also part of the table then
  /// LtrOverrideVal will maintain the minimum of all such values.
  /// A non zero value of LtrOverrideVal will indicate:
  ///   i):That there is atleast one entry in the LTR override Table
  ///   ii):The final value to be programmed in offset 0x400. This value will be applied for all the devices
  ///        connected to this root port
  ///
  Data32 = *LtrOverrideVal;
  for (PcieDeviceIndex = 0; PcieDeviceIndex < NumOfDevAspmOverride; PcieDeviceIndex++) {
    if (((DevAspmOverride[PcieDeviceIndex].OverrideConfig & ScPcieLtrOverride) == ScPcieLtrOverride) &&
      (DevAspmOverride[PcieDeviceIndex].VendorId == EndPointVendorId) &&
      ((DevAspmOverride[PcieDeviceIndex].DeviceId == EndPointDeviceId) ||
        (DevAspmOverride[PcieDeviceIndex].DeviceId == 0xFFFF)) &&
        ((DevAspmOverride[PcieDeviceIndex].RevId == EndPointRevId) ||
          (DevAspmOverride[PcieDeviceIndex].RevId == 0xFF))) {
      ///
      /// Get the Non-Snoop latency value from the table, compare and store the minimum
      ///
      if (DevAspmOverride[PcieDeviceIndex].NonSnoopLatency & BIT15) {
        Data16 = (UINT16) ((Data32 & 0xFFFF0000) >> 16);
        DetermineLatencyValue (
                               &Data16,
                               DevAspmOverride[PcieDeviceIndex].NonSnoopLatency);
        Data32 = (Data32 & 0xFFFF) | ((UINT32) (Data16 << 16));
      }
      ///
      /// Get the Snoop latency value from the table, compare and store the minimum
      ///
      if (DevAspmOverride[PcieDeviceIndex].SnoopLatency & BIT15) {
        Data16 = (UINT16) (Data32 & 0xFFFF);
        DetermineLatencyValue (
                               &Data16,
                               DevAspmOverride[PcieDeviceIndex].SnoopLatency);
        Data32 = (Data32 & 0xFFFF0000) | (UINT32) Data16;
      }
      *LtrOverrideVal = Data32;
      break;
    }
  }
  return;
}


/**
  This function configures the Latency Tolerance Reporting Settings for endpoint devices

  @param[in] RootPortConfig         Rootport PCI Express Configuration
  @param[in] EndPointBus            Endpoint Bus Number
  @param[in] EndPointDevice         Endpoint Device Number
  @param[in] EndPointFunction       Endpoint Function Number
  @param[in] EndPointBase           Endpoint PCI Express Address
  @param[in] EndPointPcieCapOffset  Pointer to Endpoint PCI Express Capability Structure
  @param[in] DeviceCapabilities2    Endpoint Value of Device Capabilities 2 Register (PciE Cap offset + 0x24)
  @param[in] PchSeries              Pch Series
  @param[in] LtrOverrideVal         Snoop and Non Snoop Latency Values

  @retval none
**/
VOID ConfigureLtr (
    IN const SC_PCIE_ROOT_PORT_CONFIG*  RootPortConfig,
    IN UINT8                            EndPointBus,
    IN UINT8                            EndPointDevice,
    IN UINT8                            EndPointFunction,
    IN UINTN                            EndPointBase,
    IN UINT8                            EndPointPcieCapOffset,
    IN UINT32                           DeviceCapabilities2,

    IN UINT32*                          LtrOverrideVal
    )
{
  UINT32 Data32;
  UINT16 Data16;
  UINT16 LtrExtendedCapOffset;
  UINT16 DefaultMaxLatency;
  DefaultMaxLatency = 0;
  ///
  /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
  /// Step 3
  /// If Endpoint device supports LTR, Device Capabilities 2 Register Offset 24h [11] = 1b,
  ///
  if ((DeviceCapabilities2 & B_PCIE_DCAP2_LTRMS) && (RootPortConfig->LtrEnable == TRUE)) {
    ///
    /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
    /// Step 3.1
    /// Program Endpoint LTR Mechanism Enable, Device Control 2 Register Offset 28h [10] = 1b
    /// when device supports LTR but is not found in override table (table listing correct
    /// latency requirements for devices that supports LTR and also for devices that do not
    /// support LTR).
    ///
    MmioOr16 (EndPointBase + EndPointPcieCapOffset + R_PCIE_DCTL2_OFFSET, B_PCIE_DCTL2_LTREN);
  }
  ///
  /// Get the pointer to the Endpoint PCI Express Extended Capability Structure
  /// and configure the Max Snoop and Max No-Snoop Latency for the endpoint
  ///
  LtrExtendedCapOffset = PcieFindExtendedCapId (EndPointBus,
                                                EndPointDevice,
                                                EndPointFunction,
                                                R_PCH_PCIE_LTRECH_CID);
  if (LtrExtendedCapOffset != 0) {
    Data32 = *LtrOverrideVal;
    ///
    /// PCH BIOS Spec Section 8.14.1 Power Optimizer Configuration
    /// Step 3.2
    /// If B0:Dxx:Fn + 400h is not programmed with snoop latency override value,
    /// program endpoint max snoop latency register, Latency Tolerance Reporting (LTR)
    /// Capability Offset 04h [15:0] = 1003h

    DefaultMaxLatency = RootPortConfig->LtrMaxSnoopLatency;
    Data16 = (UINT16) (Data32 & 0xFFFF);
    ///
    /// Set the max snoop latency to either the default max snoop latency or to the snoop latency override value
    /// that is being programmed for this root port
    ///
    DetermineLatencyValue (&Data16, DefaultMaxLatency);
    MmioAndThenOr16 (EndPointBase + LtrExtendedCapOffset + R_PCH_PCIE_LTRECH_MSLR_OFFSET,
      (UINT16) (~0x1FFF),
      Data16);
    ///
    /// PCH BIOS Spec Section 8.14.1 Power Optimizer Configuration
    /// Step 3.3
    /// If B0:Dxx:Fn + 400h is not programmed with non-snoop latency override value,
    /// program endpoint max non-snoop Latency Register, Latency Tolerance Reporting (LTR)
    /// Capability Offset 06h [15:0] = 1003h

    DefaultMaxLatency = RootPortConfig->LtrMaxNoSnoopLatency;
    Data16 = (UINT16) ((Data32 & 0xFFFF0000) >> 16);
    DetermineLatencyValue (&Data16, DefaultMaxLatency);
    MmioAndThenOr16 (
                     EndPointBase + LtrExtendedCapOffset
                         + R_PCH_PCIE_LTRECH_MNSLR_OFFSET,
                     (UINT16) (~0x1FFF),
                     Data16);
    ///
    /// Step 4
    /// If not all devices support LTR
    /// Program PWRMBASE + 20h = 00010003h
    /// (Note this register should be saved and restored during S3 transitions)
    /// Done in PchPcieSmm.c PchPciePmIoTrapSmiCallback ()
    ///
  }
}

/**
  Calculate/Set EndPoint device Power management settings

  @param[in] RootDeviceBase                   The Root Port PCI Express address
  @param[in] RootPcieCapOffset                The pointer to the Root Port PCI Express Capability Structure
  @param[in] EndPointBus                      The Bus Number of the Endpoint
  @param[in] NumOfDevAspmOverride             Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride                  Pointer to array of Device specific ASPM policy override items
  @param[in, out] LinkAspmVal                 Resulting Link ASPM value programmed
  @param[in] Operation                        Operation Types
  @param[in, out] LtrOverrideVal              Resulting LTR override value to be programmed
  @param[in] RootL1SubstateExtCapOffset       The register offset of Root Port L1 Substates
  @param[in, out] L1SubstatesSupported        Input and return the result of L1 Substates support
  @param[in, out] PortCommonModeRestoreTime   Input and return common mode restore time of L1 Substate setting
  @param[in, out] PortTpowerOnValue           Input and return power on value of L1 Substate setting
  @param[in, out] PortTpowerOnScale           Input and return power on scale of L1 Substate setting
  @param[in] RootPortConfig                   Pcie Power Optimizer Configuration
  @param[in, out] AspmOverride                Input and return the Aspm Override enable for pre-1.1 devices
  @param[in, out] ClkreqPerPortSupported      Input to check if clkreq per port is supportted
  @param[in, out] RpAndEndPointsLtrSupported  Input to check if LTR per port is supportted
  @param[in] PolicyRevision                   Policy revision for codes compatibility

  @retval EFI_SUCCESS                         Successfully completed
  @retval EFI_NOT_FOUND                       Can not find device

  @retval EFI_OUT_OF_RESOURCES                The endpoint device is a bridge, but the Subordinate Bus Number of
                                              the root port is not greater than its Secondary Bus Number. You may
                                              get this error if PCI emulation is not done before this function gets
                                              called and the Policy settings of "TempRootPortBusNumMax" and
                                              "TempRootPortBusNumMin" do not provide enough resource for temp bus
                                              number usage.
**/
EFI_STATUS
PcieEndPointPm (
  IN      UINTN                                 RootDeviceBase,
  IN      UINT32                                RootPcieCapOffset,
  IN      UINT8                                 EndPointBus,
  IN      UINT32                                NumOfDevAspmOverride,
  IN      CONST SC_PCIE_DEVICE_OVERRIDE         *DevAspmOverride,
  IN OUT  UINT16                                *LinkAspmVal,
  IN      OPERATION                             Operation,
  IN OUT  UINT32                                *LtrOverrideVal,
  IN      UINT16                                RootL1SubstateExtCapOffset,
  IN OUT  BOOLEAN                               *L1SubstatesSupported,
  IN OUT  UINT32                                *PortCommonModeRestoreTime,
  IN OUT  UINT32                                *PortTpowerOnValue,
  IN OUT  UINT32                                *PortTpowerOnScale,
  IN      CONST SC_PCIE_ROOT_PORT_CONFIG        *RootPortConfig,
  IN OUT  BOOLEAN                               *AspmOverride,
  IN      BOOLEAN                               *ClkreqPerPortSupported,
  IN OUT  BOOLEAN                               *RpAndEndPointsLtrSupported,
  IN      UINT8                                 PolicyRevision
  )
{
  EFI_STATUS  Status;
  UINTN       EndPointBase;
  UINT8       EndPointFunction;
  UINT8       EndPointPcieCapOffset;
  UINT16      EndPointAspm;
  UINT16      EndPointVendorId;
  UINT16      EndPointDeviceId;
  UINT8       EndPointRevId;
  UINT8       DownStreamBusMin;
  UINT8       ClassCode;
  UINT8       RootDevSubBusNum;
  BOOLEAN     BusAssign;
  UINT8       DeviceIndex;
  UINT8       FunctionIndex;
  UINT32      DeviceCapabilities2;
  UINT16      EndPointL1SubStateCapOffset;
  UINT32      RootDeviceL1Substates;
  UINT32      EndPointL1Substates;
  UINT8       EndPointPortCommonModeRestoreTime;
  UINT8       EndPointTpowerOnScale;
  UINT8       EndPointTpowerOnValue;
  UINT32      Multiplier[4] = {2, 10, 100, 0};
  UINT32      EndPointL1SubStateCapMask;

  DEBUG ((DEBUG_INFO, "PcieEndPointPm () Start EndPointBus %0x\n", EndPointBus));
  for (DeviceIndex = 0; DeviceIndex <= PCI_MAX_DEVICE; DeviceIndex++) {
    EndPointBase = MmPciBase (EndPointBus, DeviceIndex, 0);
    if (MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET) == DEVICE_ID_NOCARE) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (EndPointBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      EndPointFunction = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      EndPointFunction = 0;
    }

    for (FunctionIndex = 0; FunctionIndex <= EndPointFunction; FunctionIndex++) {
      EndPointBase = MmPciBase (EndPointBus, DeviceIndex, FunctionIndex);
      if (MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET) == DEVICE_ID_NOCARE) {
        continue;
      }
      //
      // Get the pointer to the Endpoint PCI Express Capability Structure.
      //
      EndPointPcieCapOffset = PcieFindCapId (EndPointBus, DeviceIndex, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
      if (EndPointPcieCapOffset == 0) {
        return EFI_NOT_FOUND;
      }
      EndPointVendorId      = MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET);
      EndPointDeviceId      = MmioRead16 (EndPointBase + PCI_DEVICE_ID_OFFSET);
      EndPointRevId         = MmioRead8  (EndPointBase + PCI_REVISION_ID_OFFSET);
      EndPointL1SubStateCapOffset = 0;
      EndPointL1SubStateCapMask   = 0x0000001F;
      RootDeviceL1Substates       = 0;
      EndPointL1Substates         = 0;
      ///
      /// Get the endpoint supports L1 Substates Capabilities
      ///
      GetOverrideL1sCapOffset (
        EndPointBase,
        NumOfDevAspmOverride,
        DevAspmOverride,
        EndPointVendorId,
        EndPointDeviceId,
        EndPointRevId,
        &EndPointL1SubStateCapOffset,
        &EndPointL1SubStateCapMask
      );
      if (EndPointL1SubStateCapOffset == 0) {
        EndPointL1SubStateCapOffset = PcieFindExtendedCapId (
          EndPointBus,
          DeviceIndex,
          FunctionIndex,
          V_PCIE_EX_L1S_CID
        );
      }
      if (EndPointL1SubStateCapOffset != 0) {
        RootDeviceL1Substates = MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET);
        EndPointL1Substates   = MmioRead32 (EndPointBase + EndPointL1SubStateCapOffset + R_PCIE_EX_L1SCAP_OFFSET);
      }
      DeviceCapabilities2 = MmioRead32 (EndPointBase + EndPointPcieCapOffset + R_PCIE_DCAP2_OFFSET);
      if (((DeviceCapabilities2 & B_PCIE_DCAP2_LTRMS) == 0) || (RootPortConfig->LtrEnable != TRUE)) {
        *RpAndEndPointsLtrSupported = FALSE;
      }
      ///
      /// Configure downstream device if present.
      ///
      if (Operation == CalculateAspm || Operation == ManualAspm) {
        if ((MmioRead32 (EndPointBase + EndPointPcieCapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_CPM) != B_PCIE_LCAP_CPM) {
          *ClkreqPerPortSupported = FALSE;
        }
        EndPointAspm = (MmioRead16 (EndPointBase + EndPointPcieCapOffset + R_PCIE_LCAP_OFFSET) >> N_PCIE_LCAP_APMS) & 3;
        DEBUG ((DEBUG_INFO, "Endpoint Device %0x Capability ASPM: %0x\n", DeviceIndex, EndPointAspm));
        if (Operation == CalculateAspm) {
          ///
          /// Check endpoint for pre-1.1 devices based on the Role based Error Reporting Capability bit
          /// and enable Aspm Override
          ///
          if (!(MmioRead16 (EndPointBase + EndPointPcieCapOffset + R_PCIE_DCAP_OFFSET) & BIT15)) {
            DEBUG((DEBUG_INFO, "Override root port ASPM to L1 for pre-1.1 devices\n"));
            *AspmOverride = TRUE;
          }
          GetOverrideAspm (
            EndPointBase,
            NumOfDevAspmOverride,
            DevAspmOverride,
            EndPointVendorId,
            EndPointDeviceId,
            EndPointRevId,
            &EndPointAspm
            );
          AspmCheckExitLatency (
            EndPointBase,
            EndPointPcieCapOffset,
            RootDeviceBase,
            RootPcieCapOffset,
            &EndPointAspm
            );
        }

        *LinkAspmVal &= EndPointAspm;
        DEBUG ((DEBUG_INFO, "Calculate Endpoint Device %0x Aspm Value: %0x\n", DeviceIndex, EndPointAspm));
        ///
        /// Check if the endpoint supports L1 Substates Capabilities
        ///
        if ((EndPointL1SubStateCapOffset != 0) && (RootL1SubstateExtCapOffset != 0)) {
          ///
          /// If both Root and endpoint's L1 Sub-States Extended Capability Offset + 0x04[4:0] are 11111b,
          /// a. Read L1 Sub-States Extended Capability Offset + 0x04[15:8], and program the highest value advertised
          ///    between PCIe rootport and device to L1 Sub-States Extended Capability Offset + 0x08[15:8] on
          ///    Pcie root port.
          /// b. Read L1 Sub-States Extended Capability Offset + 0x04[23:19] and [17:16], and program the highest value
          ///    advertised between PCIe root port and device.to L1 Sub-States Extended Capability Offset + 0x08 [7:0] on
          ///    both Pcie root port and device.
          /// c. Program L1 Sub-States Extended Capability Offset + 0x08[31:29] to 010b for both Pcie root port and device
          /// d. Program L1 Sub-States Extended Capability Offset + 0x08[25:16] to 0010100000b for both Pcie root port and device
          /// e. Program L1 Sub-States Extended Capability Offset + 0x08[4:0] to 01111b for both Pcie root port and device
          ///
          if (((RootDeviceL1Substates & 0x1F) == 0x1F) &&
            ((EndPointL1Substates & EndPointL1SubStateCapMask) == EndPointL1SubStateCapMask) &&
            (RootPortConfig->L1Substates != ScPcieL1SubstatesDisabled))
          {
            *L1SubstatesSupported             = TRUE;
            EndPointPortCommonModeRestoreTime = (EndPointL1Substates >> 8) & 0xFF;
            EndPointTpowerOnScale             = (EndPointL1Substates >> 16) & 0x3;
            EndPointTpowerOnValue             = (EndPointL1Substates >> 19) & 0x1F;

            if (EndPointPortCommonModeRestoreTime > *PortCommonModeRestoreTime) {
              *PortCommonModeRestoreTime = EndPointPortCommonModeRestoreTime;
            }

            if ((EndPointTpowerOnValue * Multiplier[EndPointTpowerOnScale]) >
            (*PortTpowerOnValue * Multiplier[*PortTpowerOnScale])) {
              *PortTpowerOnValue = EndPointTpowerOnValue;
              *PortTpowerOnScale = EndPointTpowerOnScale;
            }
          }
        }
        ///
        /// For each device detected, scan the LTR override table
        /// If there are endpoints connected directly to the rootport then
        /// LtrOverrideVal will be replaced by the value from the table for that endpoint
        /// If there are endpoints that are behind a bridge and that are also part of the table then
        /// LtrOverrideVal will maintain the minimum of all such values.
        /// A non zero value of LtrOverrideVal will indicate:
        ///   i):That there is atleast one entry in the LTR override Table
        ///   ii):The final value to be programmed in offset 0x400. This value will be applied for all the devices
        ///        connected to this root port
        ///
        GetLtrOverride (
          NumOfDevAspmOverride,
          DevAspmOverride,
          EndPointVendorId,
          EndPointDeviceId,
          EndPointRevId,
          LtrOverrideVal
          );
      } else if (Operation == SetAspm) {
        if ((EndPointL1SubStateCapOffset != 0) && (*L1SubstatesSupported)) {
          if (((RootDeviceL1Substates & 0x1F) == 0x1F) &&
            ((EndPointL1Substates & EndPointL1SubStateCapMask) == EndPointL1SubStateCapMask)) {
            ConfigureL1s (
              RootPortConfig,
              EndPointBus,
              DeviceIndex,
              FunctionIndex,
              EndPointBase,
              EndPointL1SubStateCapOffset,
              *PortCommonModeRestoreTime,
              *PortTpowerOnValue,
              *PortTpowerOnScale
            );
          }
        }
        ///
        /// Write it to the Link Control register
        ///
        DEBUG ((DEBUG_INFO, "Program Endpoint Device %0x Aspm Value: %0x\n", DeviceIndex, *LinkAspmVal));
        MmioAndThenOr16 (EndPointBase + EndPointPcieCapOffset + R_PCIE_LCTL_OFFSET, (UINT16)~B_PCIE_LCTL_ASPM, *LinkAspmVal);
        ///
        /// PCH BIOS Spec Section 8.14.1 Power Optimizer Configuration
        /// Step 3
        ///
        ConfigureLtr (
          RootPortConfig,
          EndPointBus,
          DeviceIndex,
          FunctionIndex,
          EndPointBase,
          EndPointPcieCapOffset,
          DeviceCapabilities2,
          LtrOverrideVal
          );

      }
      ///
      /// Check if this device is a bridge
      ///
      ClassCode = MmioRead8 (EndPointBase + R_PCI_BCC_OFFSET);

      if (ClassCode == PCI_CLASS_BRIDGE) {
        ///
        /// Get the downstream Bus number
        ///
        DownStreamBusMin = MmioRead8 (EndPointBase + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
        ///
        /// If the Secondary Bus Number of endpoint device is not assigned
        ///
        if (DownStreamBusMin == 0) {
          RootDevSubBusNum = MmioRead8 (RootDeviceBase + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
          ///
          /// If the endpoint device is a bridge, the Subordinate Bus Number of the root port will need to be greater
          /// than the Secondary Bus Number of the root port (the Bus Number of endpoint device).
          ///
          if (RootDevSubBusNum > EndPointBus) {
            ///
            /// Assign the Primary, Secondary and Subordinate Bus Number to endpoint device
            ///
            MmioAndThenOr32 (
              EndPointBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
              0xFF000000,
              EndPointBus | (((UINT32) (EndPointBus + 1) << 8)) | ((UINT32) (RootDevSubBusNum << 16))
              );
            DownStreamBusMin = EndPointBus + 1;
          } else {
            return EFI_OUT_OF_RESOURCES;
          }

          BusAssign = FALSE;
        } else {
          BusAssign = TRUE;
        }

        if (DownStreamBusMin > EndPointBus) {
          Status = PcieEndPointPm (
                     RootDeviceBase,
                     RootPcieCapOffset,
                     DownStreamBusMin,
                     NumOfDevAspmOverride,
                     DevAspmOverride,
                     LinkAspmVal,
                     Operation,
                     LtrOverrideVal,
                     RootL1SubstateExtCapOffset,
                     L1SubstatesSupported,
                     PortCommonModeRestoreTime,
                     PortTpowerOnValue,
                     PortTpowerOnScale,
                     RootPortConfig,
                     AspmOverride,
                     ClkreqPerPortSupported,
                     RpAndEndPointsLtrSupported,
                     PolicyRevision
                     );
          if (Status == EFI_NOT_FOUND) {
            DEBUG ((DEBUG_INFO, "Check DownStreamBus:%d and no device found!\n", DownStreamBusMin));
          }

          if (BusAssign == FALSE) {
            ///
            /// Clear Bus Numbers.
            ///
            MmioAnd32 (EndPointBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF000000);
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  This function checks if the root port and downstream device support Clkreq per port, ASPM L1 and L1 substates

  @param[in] RootBus                     Pci Bus Number of the root port
  @param[in] RootDevice                  Pci Device Number of the root port
  @param[in] RootFunction                Pci Function Number of the root port
  @param[in] NumOfDevAspmOverride        Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride             Pointer to array of Device specific ASPM policy override items
  @param[in] TempBusNumberMin            Minimal temp bus number that can be assigned to the root port (as secondary
                                         bus number) and its down stream switches
  @param[in] TempBusNumberMax            Maximal temp bus number that can be assigned to the root port (as subordinate
                                         bus number) and its down stream switches
  @param[in] RootPortConfig               Pcie Power Optimizer Configuration
  @param[in, out] L1SubstatesSupported   Flag to indicate if L1 Substates are supported
  @param[in] PolicyRevision              Revision of the policy
  @param[in, out] AspmVal                Aspm value for both rootport and end point devices
  @param[in, out] ClkreqPerPortSupported Clkreq support for both rootport and endpoint devices
  @param[out] LtrSupported               Check and return if all endpoints support LTR

  @retval EFI_SUCCESS                    The function completed successfully
  @exception EFI_UNSUPPORTED             The pointer to the Port PCI Express Capability Structure is not found
**/
EFI_STATUS
PcieCheckPmConfig (
  IN  UINT8                                RootBus,
  IN  UINT8                                RootDevice,
  IN  UINT8                                RootFunction,
  IN  UINT32                               NumOfDevAspmOverride,
  IN  SC_PCIE_DEVICE_OVERRIDE              *DevAspmOverride,
  IN  UINT8                                TempBusNumberMin,
  IN  UINT8                                TempBusNumberMax,
  IN  CONST SC_PCIE_ROOT_PORT_CONFIG       *RootPortConfig,
  IN OUT BOOLEAN                           *L1SubstatesSupported,
  IN  UINT8                                PolicyRevision,
  IN OUT UINT16                            *AspmVal,
  IN OUT BOOLEAN                           *ClkreqPerPortSupported,
  OUT BOOLEAN                              *LtrSupported
  )
{
  EFI_STATUS  Status;
  UINTN       RootDeviceBase;
  UINT32      RootPcieCapOffset;
  UINT8       EndPointBus;
  OPERATION   Operation;
  UINT16      SlotStatus;
  BOOLEAN     BusAssign;
  UINT32      LtrOvrVal;
  UINT16      RootL1SubstateExtCapOffset;
  UINT32      PortCommonModeRestoreTime;
  UINT32      PortTpowerOnValue;
  UINT32      PortTpowerOnScale;
  BOOLEAN     AspmOverride;

  DEBUG ((DEBUG_INFO, "PcieCheckPmConfig () Start BDF: %0x : %0x : %0x\n", RootBus, RootDevice, RootFunction));
  Status         = EFI_SUCCESS;
  RootDeviceBase = MmPciBase (RootBus, RootDevice, RootFunction);
  PortCommonModeRestoreTime = 0;
  PortTpowerOnValue         = 0;
  PortTpowerOnScale         = 0;
  *L1SubstatesSupported     = FALSE;
  AspmOverride              = FALSE;
  *ClkreqPerPortSupported   = FALSE;

  if (MmioRead16 (RootDeviceBase + PCI_VENDOR_ID_OFFSET) == DEVICE_ID_NOCARE) {
    return EFI_NOT_FOUND;
  }

  //
  // Assume CLKREQ# is supported on the port if PHY lane power gating is already enabled.
  //
  *ClkreqPerPortSupported = PcieIsPhyLanePgEnabled (RootDeviceBase);

  ///
  /// Get the pointer to the Port PCI Express Capability Structure.
  ///
  RootPcieCapOffset = PcieFindCapId (RootBus, RootDevice, RootFunction, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (RootPcieCapOffset == 0) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  *AspmVal = (MmioRead16 (RootDeviceBase + RootPcieCapOffset + R_PCIE_LCAP_OFFSET) >> N_PCIE_LCAP_APMS) & 3;
  if (RootPortConfig->Aspm == ScPcieAspmAutoConfig) {
    Operation = CalculateAspm;
  } else {
    Operation = ManualAspm;
    *AspmVal &= RootPortConfig->Aspm;
  }
  ///
  /// Get the downstream Bus number
  ///
  EndPointBus = MmioRead8 (RootDeviceBase + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
  ///
  /// If the Secondary Bus Number of the root port is not assigned
  /// Note:
  /// It will be better that PCI emulation has been done before PcieSetPm(). Or, you will need to assign
  /// a larger number to TempRootPortBusNumMax to support the specific card which has many bridges behind.
  /// If it is not, the Policy settings of "TempRootPortBusNumMax" and "TempRootPortBusNumMin"
  /// will be assigned to the Subordinate and Secondary Bus Number of the root ports.
  /// The assigned bus number will be cleared in the end of PcieSetPm().
  ///
  if (EndPointBus == 0) {
    MmioAndThenOr32 (
      RootDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
      0xFF0000FF,
      ((UINT32) (TempBusNumberMin << 8)) | ((UINT32) (TempBusNumberMax << 16))
      );
    EndPointBus = TempBusNumberMin;
    BusAssign   = FALSE;
  } else {
    BusAssign   = TRUE;
  }
  ///
  /// Check whether the slot has a device connected
  ///
  SlotStatus = MmioRead16 (RootDeviceBase + RootPcieCapOffset + R_PCIE_SLSTS_OFFSET);
  LtrOvrVal = 0;

  RootL1SubstateExtCapOffset = 0;
  RootL1SubstateExtCapOffset = PcieFindExtendedCapId (RootBus, RootDevice, RootFunction, V_PCIE_EX_L1S_CID);
  if (RootL1SubstateExtCapOffset != 0) {
    PortCommonModeRestoreTime = (MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET) >> 8) & 0xFF;
    PortTpowerOnScale = (MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET) >> 16) & 0x3;
    PortTpowerOnValue = (MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET) >> 19) & 0x1F;
  }
  ///
  /// Obtain initial ASPM settings from respective port capability registers.
  /// Scan LTR override table for device match and calculate the lowest override
  /// value to be programmed into PCIE RP PCI offset 400h
  ///
  if (EndPointBus != 0 && (SlotStatus & BIT6) != 0) {
    Status = PcieEndPointPm (
              RootDeviceBase,
              RootPcieCapOffset,
              EndPointBus,
              NumOfDevAspmOverride,
              DevAspmOverride,
              AspmVal,
              Operation,
              &LtrOvrVal,
              RootL1SubstateExtCapOffset,
              L1SubstatesSupported,
              &PortCommonModeRestoreTime,
              &PortTpowerOnValue,
              &PortTpowerOnScale,
              RootPortConfig,
              &AspmOverride,
              ClkreqPerPortSupported,
              LtrSupported,
              PolicyRevision
              );
  }

  if (BusAssign == FALSE) {
    ///
    /// Clear Bus Numbers.
    ///
    MmioAnd32 (RootDeviceBase + 0x018, 0xFF0000FF);
  }
  return Status;
}

/**
  This function performs the Power Management settings for root port and downstream device

  @param[in] RootBus                          Pci Bus Number of the root port
  @param[in] RootDevice                       Pci Device Number of the root port
  @param[in] RootFunction                     Pci Function Number of the root port
  @param[in] NumOfDevAspmOverride             Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride                  Pointer to array of Device specific ASPM policy override items
  @param[in] TempBusNumberMin                 Minimal temp bus number that can be assigned to the root port (as secondary
                                              bus number) and its down stream switches
  @param[in] TempBusNumberMax                 Maximal temp bus number that can be assigned to the root port (as subordinate
                                              bus number) and its down stream switches
  @param[in] RootPortConfig                   Pcie Power Optimizer Configuration
  @param[in, out] L1SubstatesSupported        Flag to indicate if L1 Substates are supported
  @param[in] PolicyRevision                   Policy revision for codes compatibility
  @param[in] FirstRpToSetPm                   Indicates if this is the first root port to be set
  @param[in] L1SupportedInAllEnabledPorts     Check if L1 is supported in all enabled ports
  @param[in] ClkreqSupportedInAllEnabledPorts Check if clkreq is supported in all enabled ports
  @param[out] LtrSupported                    Check and return if all endpoints support LTR
  @param[in] AllowRpAspmProgramming           Allow fine grain control on when the RP ASPM programming is to be done,
                                              particularly used by the RST PCIe storage remapping feature

  @retval EFI_SUCCESS                         The function completed successfully
  @exception EFI_UNSUPPORTED                  The pointer to the Port PCI Express Capability Structure is not found
**/
EFI_STATUS
PcieSetPm (
  IN  UINT8                                RootBus,
  IN  UINT8                                RootDevice,
  IN  UINT8                                RootFunction,
  IN  UINT32                               NumOfDevAspmOverride,
  IN  CONST SC_PCIE_DEVICE_OVERRIDE        *DevAspmOverride,
  IN  UINT8                                TempBusNumberMin,
  IN  UINT8                                TempBusNumberMax,
  IN  CONST SC_PCIE_ROOT_PORT_CONFIG       *RootPortConfig,
  IN OUT BOOLEAN                           *L1SubstatesSupported,
  IN  UINT8                                PolicyRevision,
  IN  BOOLEAN                              FirstRPToSetPm,
  IN  BOOLEAN                              L1SupportedInAllEnabledPorts,
  IN  BOOLEAN                              ClkreqSupportedInAllEnabledPorts,
  OUT BOOLEAN                              *LtrSupported,
  IN  BOOLEAN                              AllowRpAspmProgramming
  )
{
  ///
  /// PCH BIOS Spec Section 8.3 ASPM on DMI and the PCI Express* Root Ports
  ///
  /// When enabling L0s / L1 support, BIOS should enable upstream device before downstream
  /// device. When disabling ASPM, BIOS should make sure downstream device is disabled
  /// before upstream device.
  /// The System BIOS must perform the following steps to enable
  /// L0s/L1 on the root ports:
  ///
  /// 1. Determine whether the endpoint supports L1 by checking the Active State Link PM
  ///    Support field of the endpoint Link Capability Register.  If the endpoint does not
  ///    support L1, the System BIOS can skip the L1 calculations below.  Likewise, System
  ///    BIOS should not enable L1 on the root port or the endpoint if the endpoint does not
  ///    support L1.
  /// 2. Calculate the total L0s and L1 exit latency. A description of this calculation
  ///    is provided in Section 8.3.1.
  /// 3. Compare the calculated total exit latency with Endpoint L0s/L1 Acceptable Latency
  ///    read from the Device Capabilities Register of the Endpoint to determine if L0s or
  ///    L1 can be enabled for all or some of the links on the entire path to satisfy the
  ///    Acceptable Latency reported by the Endpoint.  The Exit Latency fields reported by
  ///    the registers are given as a range.  It is recommended that System BIOS uses the
  ///    high end of the range for the latency calculation and comparison.  For example, if
  ///    the latency field reports "2 us to less than 4 us", then 4 us should be used for
  ///    the calculation.
  /// 4. If the comparison in step 3 indicates L0s and L1 can be enabled on a root port and
  ///    the endpoints attached to the root port,
  ///    set the root port register Dxx:Fn:E8h[1], set the APMC field, Dxx:Fx:50h[1:0] to 11b
  ///    and write the same value to the APMC field of the endpoint Link Control register. If the
  ///    comparison in step 1 indicates only L0s can be enabled on a root port and the
  ///    endpoints attached to the root port, then set the APMC field, Dxx:Fn:50h[1:0]
  ///    to 01b and write the same value to the APMC field of the endpoint Link Control
  ///    register.
  ///
  /// NOTE: current implementation does not support full length exit latency calculation
  ///
  UINT16      AspmVal;
  EFI_STATUS  Status;
  UINTN       RootDeviceBase;
  UINT32      RootPcieCapOffset;
  UINT8       EndPointBus;
  OPERATION   Operation;
  UINT16      SlotStatus;
  BOOLEAN     BusAssign;
  UINT32      DeviceCapabilities2;
  UINT32      LtrOvrVal;
  UINT32      Data32And;
  UINT32      Data32Or;
  BOOLEAN     ClkreqPerPortSupported;
  UINT16      RootL1SubstateExtCapOffset;
  UINT32      PortCommonModeRestoreTime;
  UINT32      PortTpowerOnValue;
  UINT32      PortTpowerOnScale;
  BOOLEAN     AspmOverride;
  UINTN       PortIndex;
  UINT8       FirstRpIndex;

  DEBUG ((DEBUG_INFO, "PcieSetPm () Start BDF: %0x : %0x : %0x\n", RootBus, RootDevice, RootFunction));
  Status          = EFI_SUCCESS;
  RootDeviceBase  = MmPciBase (RootBus, RootDevice, RootFunction);
  PortCommonModeRestoreTime = 0;
  PortTpowerOnValue         = 0;
  PortTpowerOnScale         = 0;
  *L1SubstatesSupported     = FALSE;
  AspmOverride              = FALSE;
  ClkreqPerPortSupported    = FALSE;

  if (MmioRead16 (RootDeviceBase + PCI_VENDOR_ID_OFFSET) == DEVICE_ID_NOCARE) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  //
  // Assume CLKREQ# is supported on the port if PHY lane power gating is already enabled.
  //
  ClkreqPerPortSupported = PcieIsPhyLanePgEnabled (RootDeviceBase);

  ///
  /// Get the pointer to the Port PCI Express Capability Structure.
  ///
  RootPcieCapOffset = PcieFindCapId (RootBus, RootDevice, RootFunction, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (RootPcieCapOffset == 0) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }
  DeviceCapabilities2 = MmioRead32 (RootDeviceBase + RootPcieCapOffset + R_PCIE_DCAP2_OFFSET);

  ///
  /// Enable LTR mechanism for this root port if it is capable
  ///
  if ((DeviceCapabilities2 & B_PCIE_DCAP2_LTRMS) && (RootPortConfig->LtrEnable == TRUE)) {
    MmioOr16 (RootDeviceBase + RootPcieCapOffset + R_PCIE_DCTL2_OFFSET, B_PCIE_DCTL2_LTREN);
  }

  AspmVal = (MmioRead16 (RootDeviceBase + RootPcieCapOffset + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_APMS) >> N_PCIE_LCAP_APMS;
  if (RootPortConfig->Aspm == ScPcieAspmAutoConfig) {
    Operation = CalculateAspm;
  } else {
    Operation = ManualAspm;
    AspmVal &= RootPortConfig->Aspm;
  }
  ///
  /// Get the downstream Bus number
  ///
  EndPointBus = MmioRead8 (RootDeviceBase + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
  ///
  /// If the Secondary Bus Number of the root port is not assigned
  /// Note:
  /// It will be better that PCI enumeration has been done before PcieSetPm(). Or, you will need to assign
  /// a larger number to TempRootPortBusNumMax to support the specific card which has many bridges behind.
  /// If it is not, the Policy settings of "TempRootPortBusNumMax" and "TempRootPortBusNumMin"
  /// will be assigned to the Subordinate and Secondary Bus Number of the root ports.
  /// The assigned bus number will be cleared in the end of PcieSetPm().
  ///
  if (EndPointBus == 0) {
    MmioAndThenOr32 (
      RootDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
      0xFF0000FF,
      ((UINT32) (TempBusNumberMin << 8)) | ((UINT32) (TempBusNumberMax << 16))
      );
    EndPointBus = TempBusNumberMin;
    BusAssign   = FALSE;
  } else {
    BusAssign   = TRUE;
  }
  ///
  /// Check whether the slot has a device connected
  ///
  SlotStatus = MmioRead16 (RootDeviceBase + RootPcieCapOffset + R_PCIE_SLSTS_OFFSET);
  LtrOvrVal = 0;

  RootL1SubstateExtCapOffset = 0;
  RootL1SubstateExtCapOffset = PcieFindExtendedCapId (RootBus, RootDevice, RootFunction, V_PCIE_EX_L1S_CID);
  if (RootL1SubstateExtCapOffset != 0) {
    PortCommonModeRestoreTime = (MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET) >> 8) & 0xFF;
    PortTpowerOnScale = (MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET) >> 16) & 0x3;
    PortTpowerOnValue = (MmioRead32 (RootDeviceBase + RootL1SubstateExtCapOffset + R_PCIE_EX_L1SCAP_OFFSET) >> 19) & 0x1F;
  }
  ///
  /// Obtain initial ASPM settings from respective port capability registers.
  /// Scan LTR override table for device match and calculate the lowest override
  /// value to be programmed into PCIE RP PCI offset 400h
  ///
  if (EndPointBus != 0 && (SlotStatus & BIT6) != 0) {
    Status = PcieEndPointPm (
              RootDeviceBase,
              RootPcieCapOffset,
              EndPointBus,
              NumOfDevAspmOverride,
              DevAspmOverride,
              &AspmVal,
              Operation,
              &LtrOvrVal,
              RootL1SubstateExtCapOffset,
              L1SubstatesSupported,
              &PortCommonModeRestoreTime,
              &PortTpowerOnValue,
              &PortTpowerOnScale,
              RootPortConfig,
              &AspmOverride,
              &ClkreqPerPortSupported,
              LtrSupported,
              PolicyRevision
              );
    if (RootPortConfig->LtrEnable == TRUE) {
      if (RootPortConfig->SnoopLatencyOverrideMode == 1) {
        LtrOvrVal &= 0xFFFF0000;
        LtrOvrVal |=  (UINT32) BIT15 |
                      (UINT32) (RootPortConfig->SnoopLatencyOverrideMultiplier << 10) |
                      (UINT32) (RootPortConfig->SnoopLatencyOverrideValue);
      }

      if (RootPortConfig->NonSnoopLatencyOverrideMode == 1) {
        LtrOvrVal &= 0x0000FFFF;
        LtrOvrVal |=  (UINT32) BIT31 |
                      (UINT32) (RootPortConfig->NonSnoopLatencyOverrideMultiplier << 26) |
                      (UINT32) (RootPortConfig->NonSnoopLatencyOverrideValue << 16);
      }
      if (LtrOvrVal != 0) {
        ///
        /// Program PCIE RP PCI offset 400h only if we find a device in the LTR override table
        ///
        MmioWrite32 (RootDeviceBase + R_PCH_PCIE_LTROVR, LtrOvrVal);
        ///
        /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
        /// Step 1.1
        /// If B0:Dxx:Fn + 400h is programmed, BIOS will also program B0:Dxx:Fn + 404h [1:0] = 11b,
        /// to enable these override values.
        /// - Fn refers to the function number of the root port that has a device attached to it.
        /// - Default override value for B0:Dxx:Fn + 400h should be 880F880Fh
        /// - Also set 404h[2] to lock down the configuration
        ///
        Data32Or = BIT1 | BIT0;
        if (RootPortConfig->SnoopLatencyOverrideMode == 0) {
          Data32Or &= (UINT32) ~BIT0;
        }
        if (RootPortConfig->NonSnoopLatencyOverrideMode == 0) {
          Data32Or &= (UINT32) ~BIT1;
        }

        if (RootPortConfig->LtrConfigLock == TRUE) {
          ///
          /// Set the lock bit
          ///
          Data32Or |= BIT2;
        }
        MmioWrite32 (RootDeviceBase + R_PCH_PCIE_LTROVR2, Data32Or);
      }
    }
  }
    ///
    /// PCH BIOS Spec Section 8.3 ASPM on DMI and the PCI Express* Root Ports
    /// Step 6
    /// If both Root and endpoint's L1 Sub-States Extended Capability Offset + 0x04[4:0] are 11111b,
    ///
    if (*L1SubstatesSupported) {
      ConfigureL1s (
        RootPortConfig,
        RootBus,
        RootDevice,
        RootFunction,
        RootDeviceBase,
        RootL1SubstateExtCapOffset,
        PortCommonModeRestoreTime,
        PortTpowerOnValue,
        PortTpowerOnScale
        );
    }
    ///
    /// PCH BIOS Spec Section 8.3 ASPM on DMI and the PCI Express* Root Ports
    /// Step 7
    /// If L1.SNOOZ and L1.OFF (L1 Sub-States) are not supported,
    /// and per-port CLKREQ# is supported, and LTR is supported:
    ///
    if ((!(*L1SubstatesSupported)) && ClkreqPerPortSupported) {
      if ((DeviceCapabilities2 & BIT11) && (RootPortConfig->LtrEnable == TRUE)) {
        ///
        /// Set Dxx:Fn:420[13:4] = 32h
        ///
        MmioAndThenOr32 (
          RootDeviceBase + R_PCH_PCIE_PCIEPMECTL,
          (UINT32) ~B_PCH_PCIE_PCIEPMECTL_L1LTRTLV,
          (V_PCH_PCIE_PCIEPMECTL_L1LTRTLV << N_PCH_PCIE_PCIEPMECTL_L1LTRTLV)
          );
        ///
        /// Enable L1.LOW by setting Dxx:Fn:420[17] = 1b
        ///
        MmioOr32 (RootDeviceBase + R_PCH_PCIE_PCIEPMECTL, (UINT32)B_PCH_PCIE_PCIEPMECTL_L1LE);
        ///
        ///  Set Dxx:Fn:420h[0] to 1b
        ///
        if ((AspmVal & V_PCIE_LCTL_ASPM_L1) == V_PCIE_LCTL_ASPM_L1) {
          MmioOr32 (RootDeviceBase + R_PCH_PCIE_PCIEPMECTL, B_PCH_PCIE_PCIEPMECTL_L1FSOE);
        }
      }
    }

    if ((AspmVal & V_PCIE_LCTL_ASPM_L1) == V_PCIE_LCTL_ASPM_L1) {
      ///
      /// PCH BIOS Spec Section 8.3 ASPM on DMI and the PCI Express* Root Ports
      /// Step 5
      /// Prior to enabling ASPM L1, set Dxx:Fn:E2h[4] = 1b.
      ///
      Data32Or = (B_PCH_PCIE_RPPGEN_SEOSCGE << 16);
      ///
      /// If dedicated CLKREQ# per-port is supported on all enabled ports,
      /// set Dxx:F0:E1h[6] = 1b prior to enabling ASPM L1.
      ///
      if (ClkreqSupportedInAllEnabledPorts) {
        Data32Or |= (B_PCH_PCIE_RPDCGEN_PTOCGE << 8);
      }
      ///
      /// Note: for each PCIe controller, only its first root port register field E2h[4] and E1h[6] value are used,
      /// thus program them using sideband access
      ///
      GetScPcieRpNumber (RootDevice, RootFunction, &PortIndex);
      if (PortIndex < 2) {
        FirstRpIndex = 0;
      } else {
         FirstRpIndex = 2;
      }
      Status = PchSbiRpPciAndThenOr32 (RootDevice, FirstRpIndex, 0xE0, (UINT32)(~(0)),Data32Or);
      Data32Or = 0;
      Data32And = (UINT32)~0;
      if (ClkreqPerPortSupported) {
        /// If L1.SNOOZ and L1.OFF (L1 Sub-States) are supported , set Dxx:F0:420[30, 29] = [1b, 1b],
        if (*L1SubstatesSupported) {
          ///
          /// If L1.SNOOZ and L1.OFF (L1 Sub-States) are supported, then disable L1.LOW by setting Dxx:Fn:420h[17] = 0b
          ///
          Data32And = (UINT32)~B_PCH_PCIE_PCIEPMECTL_L1LE;
          Data32Or = B_PCH_PCIE_PCIEPMECTL_DLSULPPGE | B_PCH_PCIE_PCIEPMECTL_DLSULDLSD;
        }
        MmioAndThenOr32 (RootDeviceBase + R_PCH_PCIE_PCIEPMECTL, Data32And, Data32Or);
      }
    }
  ///
  /// Set Root Port Aspm and enable LTR capability of the device if the Root Port's ASPM programming is allowed
  ///
  if (AllowRpAspmProgramming == TRUE) {
    MmioAndThenOr16 (RootDeviceBase + RootPcieCapOffset + R_PCIE_LCTL_OFFSET, (UINT16)~B_PCIE_LCTL_ASPM, AspmVal);
  }

  ///
  /// Based on the Role based Error Reporting Capability bit, for pre-1.1 devices,
  /// program root port 0xD4[4] to 1 and 0xD4[3:2] to 10.
  ///
  if (AspmOverride) {
    MmioAndThenOr8 (RootDeviceBase + R_PCH_PCIE_MPC2,
                     (UINT8)~(B_PCH_PCIE_MPC2_ASPMCOEN | B_PCH_PCIE_MPC2_ASPMCO),
                     (B_PCH_PCIE_MPC2_ASPMCOEN | V_PCH_PCIE_MPC2_ASPMCO_L1)
                   );
  } else {
    MmioAnd8 (RootDeviceBase + R_PCH_PCIE_MPC2, (UINT8)~(B_PCH_PCIE_MPC2_ASPMCOEN | B_PCH_PCIE_MPC2_ASPMCO));
  }
  ///
  /// PCH BIOS Spec Section 8.14.1 Power Optimizer Configuration
  /// Step 1
  /// Enable support Latency Tolerance Reporting (LTR)
  ///

  if (EndPointBus != 0 && (SlotStatus & BIT6) != 0) {
    ///
    /// Set Endpoint Aspm and LTR capabilities
    ///
    Status = PcieEndPointPm (
              RootDeviceBase,
              RootPcieCapOffset,
              EndPointBus,
              NumOfDevAspmOverride,
              DevAspmOverride,
              &AspmVal,
              SetAspm,
              &LtrOvrVal,
              RootL1SubstateExtCapOffset,
              L1SubstatesSupported,
              &PortCommonModeRestoreTime,
              &PortTpowerOnValue,
              &PortTpowerOnScale,
              RootPortConfig,
              &AspmOverride,
              &ClkreqPerPortSupported,
              LtrSupported,
              PolicyRevision
              );
  }

  if (BusAssign == FALSE) {
    ///
    /// Clear Bus Numbers.
    ///
    MmioAnd32 (RootDeviceBase + 0x018, 0xFF0000FF);
  }

  return Status;
}

/**
  Initializes the root port and its down stream devices

  @param[in] RootPortBus          Pci Bus Number of the root port
  @param[in] RootPortDevice       Pci Device Number of the root port
  @param[in] RootPortFunc         Pci Function Number of the root port
  @param[in] CurrentPortBus       Pci Bus Number of the current port. Can be a root port or Switch Upstream/Downstream port
  @param[in] CurrentPortDevice    Pci Device Number of the current port. Can be a root port or Switch Upstream/Downstream port
  @param[in] CurrentPortFunc      Pci Function Number of the current port. Can be a root port or Switch Upstream/Downstream port
  @param[in] TempBusNumberMin     Minimal temp bus number that can be assigned to the root port (as secondary
                                  bus number) and its down stream switches
  @param[in] TempBusNumberMax     Maximal temp bus number that can be assigned to the root port (as subordinate
                                  bus number) and its down stream switches
  @param[in, out] MaxPayload      The Max Payolad Size of the root port

  @retval EFI_SUCCESS             Successfully completed
  @retval EFI_NOT_FOUND           Can not find device.
**/
EFI_STATUS
PchPcieInitDownstreamDevices (
  IN      UINT8           RootPortBus,
  IN      UINT8           RootPortDevice,
  IN      UINT8           RootPortFunc,
  IN      UINT8           CurrentPortBus,
  IN      UINT8           CurrentPortDevice,
  IN      UINT8           CurrentPortFunc,
  IN      UINT8           TempBusNumberMin,
  IN      UINT8           TempBusNumberMax,
  IN OUT  UINT16          *MaxPayload
  )
{
  UINT32      Index;
  UINTN       CurrentDeviceBase;
  UINTN       EndPointBase;
  UINT8       EndPointBus;
  UINT8       PcieCapOffset;
  UINT8       PcieDeviceType;
  UINT8       PcieEndCapOffset;
  UINT8       PcieEndDeviceType;
  UINT16      SlotStatus;
  DEBUG((DEBUG_INFO,"PchPcieInitDownstreamDevices RootBDF %0x : %0x :%0x\n",RootPortBus, RootPortDevice, RootPortFunc));
  DEBUG((DEBUG_INFO,"PchPcieInitDownstreamDevices CurrentBDF %0x : %0x :%0x\n",CurrentPortBus, CurrentPortDevice, CurrentPortFunc));
  DEBUG((DEBUG_INFO,"PchPcieInitDownstreamDevices TempBusMin %0x , TempBusMax %0x\n",TempBusNumberMin, TempBusNumberMax));
  DEBUG((DEBUG_INFO,"PchPcieInitDownstreamDevices MaxPayLoad %0x\n", *MaxPayload));
  CurrentDeviceBase = MmPciBase (CurrentPortBus, CurrentPortDevice, CurrentPortFunc);
  ///
  /// Get the downstream Bus number
  ///
  EndPointBus = MmioRead8 (CurrentDeviceBase + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
  ///
  /// If the Secondary Bus Number of the root port is not assigned,
  /// then temporarily Hardcode the Root Port Bridge Number to TempBusNumberMin,
  /// otherwise use the assigned Bus Number
  ///
  if (EndPointBus == 0) {
    MmioAndThenOr32 (
      CurrentDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
      0xFF000000,
      CurrentPortBus | ((UINT32) (TempBusNumberMin << 8)) | ((UINT32) (TempBusNumberMax << 16))
      );
  } else {
    TempBusNumberMin = EndPointBus;
    TempBusNumberMax = MmioRead8 (CurrentDeviceBase + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
  }
  PcieCapOffset = PcieFindCapId (
                    CurrentPortBus,
                    CurrentPortDevice,
                    CurrentPortFunc,
                    EFI_PCI_CAPABILITY_ID_PCIEXP
                    );
  PcieDeviceType = (UINT8) ((MmioRead16 (CurrentDeviceBase + PcieCapOffset + R_PCIE_XCAP_OFFSET) & B_PCIE_XCAP_DT) >> N_PCIE_XCAP_DT);

  ///
  /// Skip this section if Upstream device
  ///
  if ((PcieDeviceType != 0x05) ||
    ((MmioRead8 (CurrentDeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_LAYOUT_CODE) == HEADER_TYPE_DEVICE)) {
    ///
    /// This Endpoint check should immediately pass.
    /// Completion Retry Status Replay Enable is set in PchRootPorts.c PchInitRootPorts ()
    /// to bypass the timing requirements of the PCI Express Base
    /// Specification, Revision 1.0A, Section 6.6 ("...software must allow 1.0s
    /// after a reset of a device, before it may determine that a device which
    /// fails to return a Successful Completion status for a valid Configuration
    /// Request is a broken device").
    ///
    EndPointBase = MmPciBase (TempBusNumberMin, 0, 0);
    ///
    /// A config write is required in order for the device to re-capture the Bus number,
    /// according to PCI Express Base Specification, 2.2.6.2 ("Note that the Bus Number
    /// and Device Number may be changed at run time, and so it is necessary to re-capture
    /// this information with each and every Configuration Write Request")
    ///
    MmioWrite8 (EndPointBase + 0x0, 0);

    if (MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET) == DEVICE_ID_NOCARE) {
      ///
      /// Clear Bus Numbers.
      ///
      if (EndPointBus == 0) {
        MmioAnd32 (CurrentDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF000000);
      }
      return EFI_NOT_FOUND;
    }
  }
  for (Index = 0; Index <= PCI_MAX_DEVICE; Index++) {
    EndPointBase = MmPciBase (TempBusNumberMin, Index, 0);
    if (MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET) == DEVICE_ID_NOCARE) {
      continue;
    }
    ///
    /// Get the Max Payload Size on all the end point functions
    ///
    PcieMaxPayloadSize (TempBusNumberMin, (UINT8) Index, MaxPayload, FALSE);
    ///
    /// Disable the forwarding of EOI messages unless it discovers an IOAPIC behind this root port
    ///
    if (CurrentPortBus == 0) {  // DEFAULT_PCI_BUS_NUMBER_SC in platforPkg
      PcieSetEoiFwdDisable (CurrentPortBus, CurrentPortDevice, CurrentPortFunc, TempBusNumberMin, (UINT8) Index);
    }
    PcieEndCapOffset = PcieFindCapId (
                      TempBusNumberMin,
                      (UINT8) Index,
                      0,
                      EFI_PCI_CAPABILITY_ID_PCIEXP
                      );

    PcieEndDeviceType = (UINT8) ((MmioRead16 (EndPointBase + PcieEndCapOffset + R_PCIE_XCAP_OFFSET) & B_PCIE_XCAP_DT) >> N_PCIE_XCAP_DT);

    ///
    /// Check if this device is a bridge
    /// Pcie to Pci/PciX bridges are skipped
    ///
    if ((MmioRead8 (EndPointBase + R_PCI_BCC_OFFSET) == PCI_CLASS_BRIDGE) && (PcieEndCapOffset != 0)){
      DEBUG((DEBUG_INFO,"PchPcieInitDownstreamDevices Bridge Detected\n"));
      SlotStatus = MmioRead16 (EndPointBase + PcieEndCapOffset + R_PCIE_SLSTS_OFFSET);

      ///
      /// Initialize the downstream device in following conditions:
      /// PcieDeviceType is 5 (endpoint is an upstream device).
      /// PcieDeviceType is 6 (endpoint is a downstream device). And the slot has a device.
      ///
      if ((PcieEndDeviceType == 0x05) || ((PcieEndDeviceType == 0x06) && ((SlotStatus & B_PCIE_SLSTS_PDS) != 0))) {
        ///
        /// Initialize downstream devices
        ///
        if (TempBusNumberMax > TempBusNumberMin) {
          PchPcieInitDownstreamDevices (
            RootPortBus,
            RootPortDevice,
            RootPortFunc,
            TempBusNumberMin,
            (UINT8) Index,
            0,
            TempBusNumberMin + 1,
            TempBusNumberMax,
            MaxPayload
            );
        } else {
          ASSERT (FALSE);
        }
      }
    }

    ///
    /// Set Common Clock for inserted cards
    ///
    ///
    /// PCH BIOS Spec Section 8.3 ASPM on DMI and the PCI Express* Root Ports
    /// Before determining whether ASPM can be enabled or not,
    /// the System BIOS must perform the following steps:
    ///
    /// For PCH H
    /// 1. Update the Link Capabilities of the DMI link to indicate L0s/L1 is
    /// supported by programming the LCAP Register,
    /// PCR[DMI] + 21A4h[11:10] = 11b. (for PCH with DMI ONLY)
    /// (Done in PchDmiPei.c)
    ///
    /// 2. Enable L0s on DMI for Desktop platforms by setting the APMC field,
    /// PCR[DMI] + 21A8h[1:0] to 01b.
    /// Enable L0s/L1 on DMI by setting PCR[DMI] + 21A8h[1:0] to 11b. (for PCH with DMI ONLY)
    /// (Done in PchDmiPei.c)
    ///
    /// 3. For each root port, read the Slot Clock Configuration bit, Dxx:Fn:52h[12],
    /// of the root port and the endpoint device connected to the port (i.e., D0:F0 on the
    /// secondary bus behind the root port). If both components have this bit set, then the
    /// System BIOS should set  the Common Clock Configuration (CCC) bit, Dxx:Fn:50h[6],
    /// for both components at both sides of the link to indicate that components at both ends
    /// of the link use a common clock source.
    ///
    /// 4. If the CCC bit was changed by the System BIOS in step 3, System BIOS should initiate
    /// a link training by setting the Retrain Link (RL) bit, Dxx:Fn:50h[5], and then poll the Link
    /// Training (LT) bit, Dxx:Fn:52h[11], until it is clear.
    /// Note that System BIOS should save and restore CCC bit on S3.
    PcieSetCommonClock (CurrentPortBus, CurrentPortDevice, CurrentPortFunc, TempBusNumberMin, (UINT8) Index);

    ///
    /// Enable the PCIe CLKREQ#
    ///
    PcieSetClkreq (TempBusNumberMin, (UINT8) Index, RootPortDevice, RootPortFunc);

    ///
    /// Set the Max Payload Size on all the end point functions
    ///
    PcieMaxPayloadSize (TempBusNumberMin, (UINT8) Index, MaxPayload, TRUE);

  }
  ///
  /// Clear Bus Numbers if it's not assigned yet
  ///
  if (EndPointBus == 0) {
    MmioAnd32 (CurrentDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF000000);
  }
  return EFI_SUCCESS;
}

/**
  Initializes the root port and its down stream devices

  @param[in]  RootPortBus           Pci Bus Number of the root port
  @param[in]  RootPortDevice        Pci Device Number of the root port
  @param[in]  RootPortFunc          Pci Function Number of the root port
  @param[in]  TempBusNumberMin      Minimal temp bus number that can be assigned to the root port (as secondary
                                    bus number) and its down stream switches
  @param[in]  TempBusNumberMax      Maximal temp bus number that can be assigned to the root port (as subordinate
                                    bus number) and its down stream switches
  @param[out] PortWithMaxPayload128 At least one Port with MaxPayload set to 128 bits

  @retval EFI_SUCCESS             Successfully completed
  @retval EFI_NOT_FOUND           Can not find device.
**/
EFI_STATUS
PchPcieInitRootPortDownstreamDevices (
  IN  UINT8                          RootPortBus,
  IN  UINT8                          RootPortDevice,
  IN  UINT8                          RootPortFunc,
  IN  UINT8                          TempBusNumberMin,
  IN  UINT8                          TempBusNumberMax,
  OUT BOOLEAN                        *PortWithMaxPayload128
  )
{
  UINT16      SlotStatus;
  UINTN       RpBase;
  UINT16      RootPortMaxPayload;
  UINT8       PcieCapOffset;
  EFI_STATUS  Status;
  UINT8       CurrentPortBus;
  UINT8       CurrentPortDevice;
  UINT8       CurrentPortFunc;
  UINT16      DeviceId;

  RpBase = MmPciBase (RootPortBus, RootPortDevice, RootPortFunc);
  ///
  /// Check for a Presence Detect Change.
  ///
  SlotStatus = MmioRead16 (RpBase + R_PCH_PCIE_SLSTS);

  ///
  /// Check whether the slot has a device connected
  ///
  if ((SlotStatus & B_PCIE_SLSTS_PDS) == 0) {
    return EFI_NOT_FOUND;
  }
  ///
  /// Get the pointer to the Endpoint PCI Express Capability Structure.
  ///
  PcieCapOffset = PcieFindCapId (
                    RootPortBus,
                    RootPortDevice,
                    RootPortFunc,
                    EFI_PCI_CAPABILITY_ID_PCIEXP
                    );
  if (PcieCapOffset == 0) {
    DEBUG((DEBUG_INFO,"PcieCapOffset Not Found"));
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  ///
  /// Get the root port Max Payload Size support
  ///
  RootPortMaxPayload = MmioRead16 (RpBase + PcieCapOffset + R_PCIE_DCAP_OFFSET) & B_PCIE_DCAP_MPS;

  ///
  /// Initialize downstream devices
  ///
  CurrentPortBus    = RootPortBus;
  CurrentPortDevice = RootPortDevice;
  CurrentPortFunc   = RootPortFunc;

  Status = PchPcieInitDownstreamDevices (
             RootPortBus,
             RootPortDevice,
             RootPortFunc,
             CurrentPortBus,
             CurrentPortDevice,
             CurrentPortFunc,
             TempBusNumberMin,
             TempBusNumberMax,
             &RootPortMaxPayload);
  ///
  /// PCH BIOS Spec Section 8.2.8 Maximum Payload Size
  /// 1.  Program DCTLMPS, RP PCI offset 48h[7:5] to 000b for 128B or 001b for 256B
  /// 2.  Program IPCS. IMPS, RP PCI offset F0h[10:8] to the same value
  ///
  DEBUG((DEBUG_INFO,"RootPortMaxPayload = %0x \n",RootPortMaxPayload));
  MmioAndThenOr16 (RpBase + PcieCapOffset + R_PCIE_DCTL_OFFSET,
    (UINT16) ~B_PCIE_DCTL_MPS,
    RootPortMaxPayload << N_PCIE_DCTL_MPS);

  DeviceId = MmioRead16 (RpBase + PCI_DEVICE_ID_OFFSET);
  if (!(IS_BXT_P_PCIE_DEVICE_ID (DeviceId) || IS_BXT_PCIE_DEVICE_ID (DeviceId)) ) {
    MmioAndThenOr16 (RpBase + R_PCH_PCIE_IPCS,
      (UINT16) ~B_PCH_PCIE_IPCS_IMPS,
      RootPortMaxPayload << N_PCH_PCIE_IPCS_IMPS);
    /// If peer memory writes are supported in the platform and there is at least one device connected
    /// to any of the PCH Root Port that supports only 128B max payload size, BIOS must program all
    /// the PCH Root Ports such that upstream posted writes are split at 128B boundary by setting
    /// CCFG.UPSD to 0 and CCFG.UPRS to 000
    /// Checking is done here. Setting is done in ScRootPorts.c ScInitRootPorts ().
    if (RootPortMaxPayload == 0) {
      *PortWithMaxPayload128 = TRUE;
      ///
      /// For the PCH Root Port that supports only 128B max payload size, BIOS must program the
      /// CCFG.UNRS for that particular port to 000
      ///
      MmioAnd32 (RpBase + R_PCH_PCIE_CCFG, (UINT32)~(B_PCH_PCIE_CCFG_UNRS));
    }
  } else {
    // Force IMPS to 64B split on BXT, since there is no DMI
    MmioAndThenOr16 (RpBase + R_PCH_PCIE_IPCS,
      (UINT16) ~B_PCH_PCIE_IPCS_IMPS,
      V_PCIE_IMPS_64B_MAX_PAYLOAD << N_PCH_PCIE_IPCS_IMPS);
    // Skip this check "RootPortMaxPayload == 0" for BXT since it does not have DMI and all upstream need to be split to 64B
    // UNRS is set in ScInitRootPorts() instead.
  }

  return Status;
}
