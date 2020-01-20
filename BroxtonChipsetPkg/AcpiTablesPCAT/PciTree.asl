/**@file

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

Scope(\_SB) {
//[-start-170510-IB07400866-modify]//
#ifdef USE_DEFAULT_SIC_IRQ_TABLE
  Name(PR00, Package(){
// Host Bridge
    Package(){0x0000FFFF, 2, LNKA, 0 },   // NPK
    Package(){0x0002FFFF, 0, LNKD, 0 },   // Mobile IGFX
    Package(){0x0003FFFF, 0, LNKF, 0 },   // IUNIT
    Package(){0x000AFFFF, 0, LNKE, 0 },   // ISH
    Package(){0x000BFFFF, 0, LNKF, 0 },   // PUNIT
    Package(){0x000CFFFF, 0, LNKH, 0 },   // GMM

    Package(){0x000EFFFF, 0, LNKA, 0 },   // D14: Low Power Audio Engine
    Package(){0x000FFFFF, 0, LNKE, 0 },   // CSE
//    Package(){0x0010FFFF, 0, LNKA, 0 },   // SD Host #0 - eMMC
//    Package(){0x0011FFFF, 0, LNKB, 0 },   // SD Host #1 - SDIO
//    Package(){0x0012FFFF, 0, LNKC, 0 },   // SD Host #2 - SD Card
    Package(){0x0012FFFF, 0, LNKD, 0 },   // D18: SATA Controller

    Package(){0x0013FFFF, 0, LNKG, 0 },   // D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 1, LNKH, 0 },   // D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 2, LNKE, 0 },   // D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 3, LNKF, 0 },   // D19: PCI Express Port 3-6

    Package(){0x0014FFFF, 0, LNKG, 0 },   // D20: PCI Express Port 1-2
    Package(){0x0014FFFF, 1, LNKH, 0 },   // D20: PCI Express Port 1-2

    Package(){0x0015FFFF, 0, LNKB, 0 },   // D21: xHCI Host, xDCI
    Package(){0x0015FFFF, 1, LNKC, 0 },

    Package(){0x001FFFFF, 0, LNKE, 0 },   // SMBus
//    Package(){0x001EFFFF, 2, LNKC, 0 },   // LPC Bridge
  })

  Name(AR00, Package() {

// Fields: Address,
//         PCI Pin,
//         Source (0 is global interrupt pool),
//         Source Index (IRQ if Source=0)

    Package(){0x0000FFFF, 0, 0, 16 },  // NPK Device
    Package(){0x0000FFFF, 1, 0, 24 },  // PUNIT Device (INTB -> 24)
    Package(){0x0002FFFF, 0, 0, 19 },  // GEN (Intel GFX)
    Package(){0x0003FFFF, 0, 0, 21 },  // IUNIT
    Package(){0x000DFFFF, 1, 0, 40 },  // PMC
    Package(){0x000EFFFF, 0, 0, 25 },  // HD-Audio
    Package(){0x000FFFFF, 0, 0, 20 },  // CSE
    Package(){0x0011FFFF, 0, 0, 26 },  // ISH
    Package(){0x0012FFFF, 0, 0, 19 },  // SATA

// D19: PCI Express Port 3-6
    Package(){0x0013FFFF, 0, 0, 22 },  // PCIE0
    Package(){0x0013FFFF, 1, 0, 23 },  // PCIE0
    Package(){0x0013FFFF, 2, 0, 20 },  // PCIE0
    Package(){0x0013FFFF, 3, 0, 21 },  // PCIE0
    
// D20: PCI Express Port 1-2
    Package(){0x0014FFFF, 0, 0, 22 },  // PCIE1
    Package(){0x0014FFFF, 1, 0, 23 },  // PCIE1

    Package(){0x0015FFFF, 0, 0, 17 },  // xHCI
    Package(){0x0015FFFF, 1, 0, 13 },  // xDCI

    Package(){0x0016FFFF, 0, 0, 27 },  // I2C0
    Package(){0x0016FFFF, 1, 0, 28 },  // I2C1
    Package(){0x0016FFFF, 2, 0, 29 },  // I2C2
    Package(){0x0016FFFF, 3, 0, 30 },  // I2C3   // usage note: taking this line as an example, device 0x16, function 3 will use apic irq 30

    Package(){0x0017FFFF, 0, 0, 31 },  // I2C4
    Package(){0x0017FFFF, 1, 0, 32 },  // I2C5
    Package(){0x0017FFFF, 2, 0, 33 },  // I2C6
    Package(){0x0017FFFF, 3, 0, 34 },  // I2C7

//[-start-160813-IB07400770-modify]//
#ifdef LEGACY_IRQ_SUPPORT
    Package(){0x0018FFFF, 0, 0, 44 },   // UART1
    Package(){0x0018FFFF, 1, 0, 45 },   // UART2
    Package(){0x0018FFFF, 2, 0, 6 },    // UART3
    Package(){0x0018FFFF, 3, 0, 47 },   // UART4
#else    
    Package(){0x0018FFFF, 0, 0, 4 },   // UART1
    Package(){0x0018FFFF, 1, 0, 5 },   // UART2
    Package(){0x0018FFFF, 2, 0, 6 },   // UART3
    Package(){0x0018FFFF, 3, 0, 7 },   // UART4
#endif    
//[-end-160813-IB07400770-modify]//

    Package(){0x0019FFFF, 0, 0, 35 },  // SPI1
    Package(){0x0019FFFF, 1, 0, 36 },  // SPI2
    Package(){0x0019FFFF, 2, 0, 37 },  // SPI3

//[-start-160813-IB07400770-modify]//
#ifdef LEGACY_IRQ_SUPPORT
    Package(){0x001BFFFF, 0, 0, 43 },   // SDCard
#else    
    Package(){0x001BFFFF, 0, 0, 3 },   // SDCard
#endif    
//[-end-160813-IB07400770-modify]//
    Package(){0x001CFFFF, 0, 0, 39 },  // eMMC
    Package(){0x001EFFFF, 0, 0, 42 },  // SDIO

    Package(){0x001FFFFF, 0, 0, 20 }   // SMBus
  })

  Name(PR04, Package(){
// PCIE Port #1 Slot
    Package(){0x0000FFFF, 0, LNKG, 0 },
    Package(){0x0000FFFF, 1, LNKH, 0 },
    Package(){0x0000FFFF, 2, LNKE, 0 },
    Package(){0x0000FFFF, 3, LNKF, 0 },
  })

  Name(AR04, Package(){
// PCIE Port #1 Slot
    Package(){0x0000FFFF, 0, 0, 22 },
    Package(){0x0000FFFF, 1, 0, 23 },
    Package(){0x0000FFFF, 2, 0, 20 },
    Package(){0x0000FFFF, 3, 0, 21 },
  })

  Name(PR05, Package(){
// PCIE Port #2 Slot
    Package(){0x0000FFFF, 0, LNKH, 0 },
    Package(){0x0000FFFF, 1, LNKE, 0 },
    Package(){0x0000FFFF, 2, LNKF, 0 },
    Package(){0x0000FFFF, 3, LNKG, 0 },
  })

  Name(AR05, Package(){
// PCIE Port #2 Slot
    Package(){0x0000FFFF, 0, 0, 23 },
    Package(){0x0000FFFF, 1, 0, 20 },
    Package(){0x0000FFFF, 2, 0, 21 },
    Package(){0x0000FFFF, 3, 0, 22 },
  })

  Name(PR06, Package(){
// PCIE Port #3 Slot
    Package(){0x0000FFFF, 0, LNKE, 0 },
    Package(){0x0000FFFF, 1, LNKF, 0 },
    Package(){0x0000FFFF, 2, LNKG, 0 },
    Package(){0x0000FFFF, 3, LNKH, 0 },
  })

  Name(AR06, Package(){
// PCIE Port #3 Slot
    Package(){0x0000FFFF, 0, 0, 20 },
    Package(){0x0000FFFF, 1, 0, 21 },
    Package(){0x0000FFFF, 2, 0, 22 },
    Package(){0x0000FFFF, 3, 0, 23 },
  })

  Name(PR07, Package(){
// PCIE Port #4 Slot
    Package(){0x0000FFFF, 0, LNKF, 0 },
    Package(){0x0000FFFF, 1, LNKG, 0 },
    Package(){0x0000FFFF, 2, LNKH, 0 },
    Package(){0x0000FFFF, 3, LNKE, 0 },
  })

  Name(AR07, Package(){
// PCIE Port #4 Slot
    Package(){0x0000FFFF, 0, 0, 21 },
    Package(){0x0000FFFF, 1, 0, 22 },
    Package(){0x0000FFFF, 2, 0, 23 },
    Package(){0x0000FFFF, 3, 0, 20 },
  })
#else

  include ("Dsdt/BusPRT/B00PRT.asl")
  include ("Dsdt/BusPRT/B04PRT.asl")
  include ("Dsdt/BusPRT/B05PRT.asl")
  include ("Dsdt/BusPRT/B06PRT.asl")
  include ("Dsdt/BusPRT/B07PRT.asl")
  include ("Dsdt/BusPRT/B08PRT.asl")
  include ("Dsdt/BusPRT/B09PRT.asl")

#endif
//[-end-170510-IB07400866-modify]//

//---------------------------------------------------------------------------
// List of IRQ resource buffers compatible with _PRS return format.
//---------------------------------------------------------------------------
// Naming legend:
// RSxy, PRSy - name of the IRQ resource buffer to be returned by _PRS, "xy" - last two characters of IRQ Link name.
// Note. PRSy name is generated if IRQ Link name starts from "LNK".
// HLxy , LLxy - reference names, can be used to access bit mask of available IRQs. HL and LL stand for active High(Low) Level triggered Irq model.
//---------------------------------------------------------------------------
  Name(PRSA, ResourceTemplate(){	// Link name: LNKA
    IRQ(Level, ActiveLow, Shared, LLKA) {3,4,5,6,10,11,12,14,15}
  })
  Alias(PRSA,PRSB)  // Link name: LNKB
  Alias(PRSA,PRSC)  // Link name: LNKC
  Alias(PRSA,PRSD)  // Link name: LNKD
  Alias(PRSA,PRSE)  // Link name: LNKE
  Alias(PRSA,PRSF)  // Link name: LNKF
  Alias(PRSA,PRSG)  // Link name: LNKG
  Alias(PRSA,PRSH)  // Link name: LNKH
//---------------------------------------------------------------------------
// Begin PCI tree object scope
//---------------------------------------------------------------------------

  Device(PCI0) { // PCI Bridge "Host Bridge"
    Name(_HID, EISAID("PNP0A08"))  // Indicates PCI Express/PCI-X Mode2 host hierarchy
    Name(_CID, EISAID("PNP0A03"))  // To support legacy OS that doesn't understand the new HID
    Name(_ADR, 0x00000000)
    Method(^BN00, 0) { return(0x0000) }  // Returns default Bus number for Peer PCI buses. Name can be overriden with control method placed directly under Device scope
    Method(_BBN, 0) { return(BN00()) }   // Bus number, optional for the Root PCI Bus
    Name(_UID, 0x0000)  // Unique Bus ID, optional

    Method(_PRT,0) {
      If(PICM) {Return(AR00)} // APIC mode
      Return (PR00)           // PIC Mode
    } // end _PRT
    include("HOST_BUS.ASL")
  } // end PCI0 Bridge "Host Bridge"
} // end _SB scope
