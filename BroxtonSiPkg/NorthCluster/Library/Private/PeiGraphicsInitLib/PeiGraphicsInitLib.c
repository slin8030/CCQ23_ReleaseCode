/** @file
  PEIM to initialize both IGD and PCI graphics card.

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

#include <Library/PeiGraphicsInitLib.h>
#include <Library/DebugLib.h>

#include <ScAccess.h>
//[-start-160706-IB07400752-add]//
#include <SaAccess.h>
//[-end-160706-IB07400752-add]//
#include <Library/PeiSaPolicyLib.h>

//[-start-160705-IB07400752-add]//
typedef struct {
  UINT8 Dev;
  UINT8 Fun;
} PCIE_CHECK_TABLE;

PCIE_CHECK_TABLE mAplPcie[] = {
  {0x13, 0},
  {0x13, 1},
  {0x13, 2},
  {0x13, 3},
  {0x14, 0},
  {0x14, 1}
};

UINTN mAplPcieSize = sizeof(mAplPcie) / sizeof (PCIE_CHECK_TABLE);

UINT8
EnumerateDownstream (
  IN UINT8  BusNum
  )
/**

  This function enumerate all downstream bridge.

  @param[in]

  BusNum  - Primary bus number of current bridge.

  @retval

  BusNum: return current bus number if current bus is an enpoint device.
  SubBus: return subordinate bus number if current bus is a bridge.

**/
{
  UINT8   DevNum;
  UINT16  Buffer16;
  UINT8   SubBus;
  UINT8   SecBus;

  SubBus  = 0;

  SecBus  = BusNum;

  for (DevNum = 0; DevNum < 32; DevNum++) {
    //
    // Read Vendor ID to check if device exists
    // if no device exists, then check next device
    //
    if (MmPci16 (0, BusNum, DevNum, 0, R_PCI_PCI_VID) == 0xFFFF) {
      continue;
    }

    Buffer16 = MmPci16 (0, BusNum, DevNum, 0, R_PCI_PCI_SCC);
    //
    // Check for PCI/PCI Bridge Device Base Class 6 with subclass 4
    //
    if (Buffer16 == 0x0604) {
      SecBus++;
      MmPci8 (0, BusNum, DevNum, 0, R_PCI_PCI_PBN)  = BusNum;
      MmPci8 (0, BusNum, DevNum, 0, R_PCI_PCI_SCBN) = SecBus;
      //
      // Assign temporary subordinate bus number so that device behind this bridge can be seen
      //
      MmPci8 (0, BusNum, DevNum, 0, R_PCI_PCI_SBBN) = 0xff;

      //
      // A config write is required in order for the device to re-capture the Bus number,
      // according to PCI Express Base Specification, 2.2.6.2
      // Write to a read-only register VendorID to not cause any side effects.
      //
      MmPci16 (0, SecBus, 0, 0, PCI_VID) = 0;

      //
      // Enumerate bus behind this bridge by calling this funstion recursively
      //
      SubBus = EnumerateDownstream (SecBus);
      //
      // Update the correct subordinate bus number
      //
      MmPci8 (0, BusNum, DevNum, 0, R_PCI_PCI_SBBN) = SubBus;
      SecBus = SubBus;
    }
  }

  if (SubBus == 0) {
    return BusNum;
  } else {
    return SubBus;
  }
}

VOID
CheckOffboardPcieVga (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN OUT UINT32               *PchPcieMmioLength,
  IN OUT DISPLAY_DEVICE       *PrimaryDisplay
  )
/**

  CheckOffboardPcieVga: Check if off board PCIe graphics Card is present

  @param[in]

  PeiServices        - Pointer to the PEI services table
  PchPcieMmioLength  - Total PCIe MMIO length on all PCH root ports
  PrimaryDisplay     - Primary Display - default is IGD

  @retval

  None

**/
{
  UINT8   PortNo;
  UINT32  PcieBusNum;
  UINT8   Bus;
  UINT8   Dev;
  UINT8   Func;
  UINT8   MaxFunction;
  UINT8   SubBusNum;
  UINT8   HeaderType;
  UINT16  Buffer16;
  BOOLEAN CardDetect;
  UINT32  MmioLength;

  MmioLength = 0;

  //
  // Initialize Secondary and Subordinate bus number for first Pcie root port
  //
  PcieBusNum  = 0x00010100;

  SubBusNum   = 0;

  CardDetect  = FALSE;

  for (PortNo = 0; PortNo < mAplPcieSize; PortNo++) {
    //
    // Check if root port exists
    //
    if (MmPci16 (0, 0, mAplPcie[PortNo].Dev, mAplPcie[PortNo].Fun, R_PCI_PCI_VID) == 0xFFFF) {
      continue;
    }

    MmPci32 (0, 0, mAplPcie[PortNo].Dev, mAplPcie[PortNo].Fun, R_PCI_PCI_PBN) = PcieBusNum;
    Bus = MmPci8 (0, 0, mAplPcie[PortNo].Dev, mAplPcie[PortNo].Fun, R_PCI_PCI_SCBN);

    //
    // Assign temporary subordinate bus number so that device this bridge can be seen
    //
    MmPci8 (0, 0, mAplPcie[PortNo].Dev, mAplPcie[PortNo].Fun, R_PCI_PCI_SBBN) = 0xff;

    //
    // A config write is required in order for the device to re-capture the Bus number,
    // according to PCI Express Base Specification, 2.2.6.2
    // Write to a read-only register VendorID to not cause any side effects.
    //
    MmPci16 (0, Bus, 0, 0, PCI_VID) = 0;

    SubBusNum = EnumerateDownstream (Bus);
    //
    // Update the actual subordinate bus number
    //
    MmPci8 (0, 0, mAplPcie[PortNo].Dev, mAplPcie[PortNo].Fun, R_PCI_PCI_SBBN) = SubBusNum;
    PcieBusNum = (SubBusNum + 1) << 8;
  }

  for (Bus = 1; Bus <= SubBusNum; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      //
      // Read Vendor ID to check if device exists
      // if no device exists, then check next device
      //
      if (MmPci16 (0, Bus, Dev, 0, R_PCI_PCI_VID) == 0xFFFF) {
        continue;
      }

      //
      // Check for a multifunction device
      //
      HeaderType = MmPci8 (0, Bus, Dev, 0, R_PCI_PCI_HEADTYP);
      if ((HeaderType & B_PCI_PCI_HEADTYP_MFD) != 0) {
        MaxFunction = 7;
      } else {
        MaxFunction = 0;
      }

      for (Func = 0; Func <= MaxFunction; Func++) {
        if (MmPci16 (0, Bus, Dev, Func, R_PCI_PCI_VID) == 0xFFFF) {
          continue;
        }

//        FindPciDeviceMmioLength (Bus, Dev, Func, &MmioLength);
//        *PchPcieMmioLength += MmioLength;

        //
        // Video cards can have Base Class 0 with Sub-class 1
        // or Base Class 3.
        //
        if (MmPci16 (0, Bus, Dev, Func, R_PCI_PCI_SCC) == 0x0300) {
          if (CardDetect != TRUE) {
            *PrimaryDisplay = PCI;
            DEBUG ((EFI_D_ERROR, "SC PCIe Graphics Card enabled.\n"));
            CardDetect = TRUE;
          }
        }
      }
    }
  }

  //
  // Clear bus number on all the bridges that we have opened so far.
  // We have to do it in the reverse Bus number order.
  //
  for (Bus = SubBusNum; Bus >= 1; Bus--) {
    for (Dev = 0; Dev < 32; Dev++) {
      //
      // Read Vendor ID to check if device exists
      // if no device exists, then check next device
      //
      if (MmPci16 (0, Bus, Dev, 0, R_PCI_PCI_VID) == 0xFFFF) {
        continue;
      }

      Buffer16 = MmPci16 (0, Bus, Dev, 0, R_PCI_PCI_SCC);
      //
      // Clear Bus Number for PCI/PCI Bridge Device
      //
      if (Buffer16 == 0x0604) {
        MmPci32 (0, Bus, Dev, 0, R_PCI_PCI_PBN) = 0;
      }
    }
  }
  //
  // Clear bus numbers. PCIe retrain will use temporary bus number.
  //
  for (PortNo = 0; PortNo < mAplPcieSize; PortNo++) {
    //
    // Clear bus numbers so that PCIe slots are hidden
    //
    MmPci32 (0, 0, mAplPcie[PortNo].Dev, mAplPcie[PortNo].Fun, R_PCI_PCI_PBN) = 0;
  }
}
//[-end-160705-IB07400752-add]//

/**
  GraphicsInit: Initialize the IGD if no other external graphics is present

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] SaPolicyPpi          Instance of _SA_POLICY_PPI

  @retval  None
**/
VOID
GraphicsPreMemInit (
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  UINT16                GMSData;
  BOOLEAN               IGfxSupported;
  UINTN                 McD0BaseAddress;
  UINTN                 McD2BaseAddress;
  SA_PRE_MEM_CONFIG     *SaPreMemConfig = NULL;
  EFI_STATUS            Status;
  SI_SA_POLICY_PPI     *SiSaPolicyPpi;
//[-start-160705-IB07400752-add]//
  DISPLAY_DEVICE        PrimaryDisplay;
  UINT32                PchPcieMmioLength;
//[-end-160705-IB07400752-add]//

  DEBUG ((DEBUG_INFO, "iGFX initialization Start\n"));

  Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gSiSaPreMemPolicyPpiGuid,
                                 0,
                                 NULL,
                                 (void **)&SiSaPolicyPpi
                                 );

  Status = GetConfigBlock((VOID *)SiSaPolicyPpi, &gSaPreMemConfigGuid, (VOID *)&SaPreMemConfig);
  ASSERT_EFI_ERROR(Status);

//[-start-160705-IB07400752-add]//
  PrimaryDisplay = IGD;
  PchPcieMmioLength = 0;
  CheckOffboardPcieVga (PeiServices, &PchPcieMmioLength, &PrimaryDisplay);
//[-end-160705-IB07400752-add]//

  ///
  /// Set the VGA Decode bits to a good known starting point where both PEG and
  /// IGD VGA Decode Bits are both disabled.
  ///
  McD0BaseAddress   = MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN);
  MmioOr16 (McD0BaseAddress + R_SA_GGC, B_SA_GGC_IVD_MASK);
  ///
  /// Check if IGfx is supported
  ///
  McD2BaseAddress  = MmPciBase (SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0);
  IGfxSupported    = (BOOLEAN) (MmioRead16 (McD2BaseAddress + R_SA_IGD_VID) != 0xFFFF);
  if (!IGfxSupported) {
    DEBUG ((DEBUG_INFO, "iGFX not detected, skipped!\n"));
    return;
  }
  ///
  /// If primary display device is IGD or no other display detected then enable IGD
  ///
  if (IGfxSupported && SaPreMemConfig->InternalGraphics != IGD_DISABLE) {

    DEBUG ((EFI_D_INFO, "IGD enabled.\n"));
    ///
    /// Program GFX Memory by setting D0.F0.R 050h [15:8]
    ///
    GMSData = (UINT8) SaPreMemConfig->IgdDvmt50PreAlloc;
    DEBUG ((EFI_D_INFO, "GMSData: 0x%x\n",GMSData));
    MmioAnd16 (McD0BaseAddress + R_SA_GGC, (UINT16) ~(B_SA_GGC_GMS_MASK));
    MmioOr16 (McD0BaseAddress + R_SA_GGC, ((GMSData & 0x3F) << N_SA_GGC_GMS_OFFSET));
    ///
    /// Program Graphics GTT Memory D0:F0:R50h[7:6]
    ///   01b => 2MB of GTT
    ///   10b => 4MB of GTT
    ///   11b => 8MB of GTT
    ///
    if (SaPreMemConfig->GttSize != V_SA_GGC_GGMS_DIS) {
        ASSERT (SaPreMemConfig->GttSize <= 3);
        MmioAndThenOr16 (McD0BaseAddress + R_SA_GGC, (UINT16) ~(B_SA_GGC_GGMS_MASK), (SaPreMemConfig->GttSize << N_SA_GGC_GGMS_OFFSET) & B_SA_GGC_GGMS_MASK);
    }
    ///
    /// Set register D2.F0.R 062h [4:0] = `00001b' to set a 256MByte aperture.
    /// This must be done before Device 2 registers are enumerated.
    ///
    MmioAndThenOr8 (McD2BaseAddress + R_SA_IGD_MSAC_OFFSET, (UINT8) ~(BIT4 + BIT3 + BIT2 + BIT1 + BIT0), (UINT8) SaPreMemConfig->ApertureSize);
    ///
    /// Enable IGD VGA Decode.  This is needed so the Class Code will
    /// be correct for the IGD Device when determining which device
    /// should be primary.  If disabled, IGD will show up as a non VGA device.
    ///
//[-start-160706-IB07400752-modify]//
    //
    //  Setup  Detection  Primary
    //   Auto     IGD     -> IGD
    //   Auto     PCI     -> PCI
    //   IGD      IGD     -> IGD
    //   IGD      PCI     -> IGD
    //   PCI      IGD     -> IGD
    //   PCI      PCI     -> PCI
    //
    if ((PrimaryDisplay == PCI) && (SaPreMemConfig->PrimaryDisplay != IGD)) {
//[-end-160706-IB07400752-modify]//
      ///
      /// If IGD is forced to be enabled, but is a secondary display, disable IGD VGA Decode
      ///
      MmioOr16 (McD0BaseAddress + R_SA_GGC, B_SA_GGC_IVD_MASK);
      DEBUG ((EFI_D_INFO, "IGD VGA Decode is disabled because it's not a primary display.\n"));
    } else {
      MmioAnd16 (McD0BaseAddress + R_SA_GGC, (UINT16) ~(B_SA_GGC_IVD_MASK));
    }

  } else {

    DEBUG ((EFI_D_INFO, "Disable IGD Device.\n"));

    ///
    /// Disable IGD device
    /// Set Register D0:F0 Offset 50h [15:8] (GMS) = '00000000b'.
    /// This prevents UMA memory from being pre-allocated to IGD.
    /// Set D0:F0 Offset 50h [9:8] (GGMS) = '00b'.
    /// Set GTT Graphics Memory Size to 0
    ///
    MmioAndThenOr16 (McD0BaseAddress + R_SA_GGC, (UINT16) ~(B_SA_GGC_GGMS_MASK | B_SA_GGC_GMS_MASK), B_SA_GGC_IVD_MASK);

    SaPreMemConfig->GttSize           = 0;
    SaPreMemConfig->IgdDvmt50PreAlloc = 0;
    ///
    /// Disable IGD. D0.F0.R 054h [4] = '0b'.
    ///
    MmioAnd8 (McD0BaseAddress + R_SA_DEVEN, (UINT8) ~(B_SA_DEVEN_D2F0EN_MASK));
  }
  DEBUG ((DEBUG_INFO, "iGFX initialization End\n"));
}

