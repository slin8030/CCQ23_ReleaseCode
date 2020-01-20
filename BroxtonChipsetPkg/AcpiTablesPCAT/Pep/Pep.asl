/** @file
  ACPI uPEP Support

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

External(\_PR.NLPC, IntObj)

//@todo MSFT has NOT implemented uPEP for Non-CS configuration on Win10. Need to uncomment once MSFT fully implements uPEP
If(LOr(LEqual(S0ID, 1),LGreaterEqual(OSYS, 2015))){
  //Comment out the GFX0, since the _DEP is added in NorthCluster/AcpiTables/Sa.asl
  //Scope(\_SB.PCI0.GFX0) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SATA) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C0) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C1) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C2) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C3) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C4) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C5) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C6) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.I2C7) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SPI1) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SPI2) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SPI3) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.URT1) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.URT2) { Name(_DEP, Package(){\_SB.PEPD}) }
  //Scope(\_SB.PCI0.URT3) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.URT4) { Name(_DEP, Package(){\_SB.PEPD}) }
  //Scope(\_SB.PCI0.PWM)  { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.XHC)  { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_PR.CPU0) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_PR.CPU1) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_PR.CPU2) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_PR.CPU3) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SDIO) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SDHA) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.HDAS) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.SDC) { Name(_DEP, Package(){\_SB.PEPD}) }
  Scope(\_SB.PCI0.RP02.PXSX) { Name(_DEP, Package(){\_SB.PEPD}) }
}

Scope(\_SB)
{
  Device (PEPD)
  {
    Name (_HID, "INT33A1")
    Name (_CID, EISAID ("PNP0D80"))
    Name (_UID, 0x1)

    Name(PEPP, Zero)
    Name (DEVS, Package()
    {
      1,
      Package() {"\\_SB.PCI0.GFX0"},
      //Package() {"\\_SB.PCI0.SAT0.PRT1"}
    })

	
    Name(DEVY, Package() // uPEP Device List
    {
      //
      // 1: ACPI Device Descriptor: Fully Qualified name-string
      // 2: Enabled/Disabled Field
      //      0 = This device is disabled and applies no constraints
      //     1+ = This device is enabled and applies constraints
      // 3: Constraint Package: entry per LPI state in LPIT
      //     a. Associated LPI State UID
      //         ID == 0xFF: same constraints apply to all states in LPIT
      //     b: minimum Dx state as pre-condition
      //     c: (optional) OEM specific OEM may provide an additional encoding
      //         which further defines the D-state Constraint
      //            0x0-0x7F - Reserved
      //            0x80-0xFF - OEM defined
      //
      Package() {"\\_PR.CPU0",                    0x1, Package() {0, Package() {0xFF, 0}}},
      Package() {"\\_PR.CPU1",                    0x1, Package() {0, Package() {0xFF, 0}}},
      Package() {"\\_PR.CPU2",                    0x1, Package() {0, Package() {0xFF, 0}}},
      Package() {"\\_PR.CPU3",                    0x1, Package() {0, Package() {0xFF, 0}}},
      Package() {"\\_SB.PCI0.GFX0",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.SATA",               0x1, Package() {0, Package() {0xFF, 0, 0x81}}},
      Package() {"\\_SB.PCI0.URT1",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.URT2",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.URT3",               0x0, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.URT4",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.SPI1",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.SPI2",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.SPI3",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C0",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C1",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C2",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C3",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C4",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C5",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C6",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.I2C7",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.PWM",                0x0, Package() {0, Package() {0xFF, 3}}},

      Package() {"\\_SB.PCI0.SDIO",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.SDHA",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.HDAS",               0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.SDC",                0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.XHC",                0x1, Package() {0, Package() {0xFF, 3}}},
      Package() {"\\_SB.PCI0.RP02.PXSX",          0x0, Package() {0, Package() {0xFF, 0}}},
    })

    Name(BCCD, Package() // Bugcheck Critical Device(s)
    {
      //
      // 1: ACPI Device Descriptor: Fully Qualified name string
      // 2: Package of packages: 1 or more specific commands to power up critical device
      //  2a: Package: GAS-structure describing location of PEP accessible power control
      //    Refer to ACPI 5.0 spec section 5.2.3.1 for details
      //    a: Address Space ID (0 = System Memory)
      //       NOTE: A GAS Address Space of 0x7F (FFH) indicates remaining package
      //             elements are Intel defined
      //    b: Register bit width (32 = DWORD)
      //    c: Register bit offset
      //    d: Access size (3 = DWORD Access)
      //    e: Address (for System Memory = 64-bit physical address)
      //  2b: Package containing:
      //    a: AND mask !V not applicable for all Trigger Types
      //    b: Value (bits required to power up the critical device)
      //    c: Trigger Type:
      //         0 = Read
      //         1 = Write
      //         2 = Write followed by Read
      //         3 = Read Modify Write
      //         4 = Read Modify Write followed by Read
      //  2c: Power up delay: Time delay before next operation in uSec
      //
      Package() {"\\_SB.PCI0.SATA", Package() {
        Package() {Package() {1, 8, 0, 1, 0xB2}, // GAS Structure 8-bit IO Port
                   Package() {0x0, 0xCD, 0x1},   // Write 0xCD
                   16000}                        // Power up delay = 16ms
        }
      },
      Package() {"\\_SB.PCI0.SATA.PRT0", Package(){
        Package() {Package() {1, 8, 0, 1, 0xB2}, // GAS Structure 8-bit IO Port
                   Package() {0x0, 0xCD, 0x1},   // Write 0xCD
                   16000}                        // Power up delay = 16ms
        }
      },
      Package() {"\\_SB.PCI0.SATA.PRT1", Package(){
        Package() {Package() {1, 8, 0, 1, 0xB2}, // GAS Structure 8-bit IO Port
                   Package() {0x0, 0xCD, 0x1},   // Write 0xCD
                   16000}                        // Power up delay = 16ms
        }
      },
    })

    Method(_STA, 0x0, NotSerialized)
    {
      If(LOr(LGreaterEqual(OSYS,2015), LAnd(LGreaterEqual(OSYS,2012),LEqual(S0ID, 1))))
      {
        Return(0xf)
      }
      Return(0)
    }

    Method(_DSM, 0x4, Serialized)
    {
      ADBG(Concatenate("PEPY = ", ToHexString(PEPY)))
      ADBG(Concatenate("PEPC = ", ToHexString(PEPC)))
      ADBG(Concatenate("OSYS = ", ToHexString(OSYS)))

      If(LEqual(Arg0,ToUUID("c4eb40a0-6cd2-11e2-bcfd-0800200c9a66")))
      {
        // Number of Functions (including this one)
        If(LEqual(Arg2, Zero))
        {
            Return(Buffer(One){0x7F})
        }

        If (LEqual(\_PR.NLPC, 1))
        {
          Store (0x01, Index (DeRefOf(Index (DEVY, 0)), 1))  // 0 - CPU-0
          Store (0x00, Index (DeRefOf(Index (DEVY, 1)), 1))  // 1 - CPU-1
          Store (0x00, Index (DeRefOf(Index (DEVY, 2)), 1))  // 2 - CPU-2
          Store (0x00, Index (DeRefOf(Index (DEVY, 3)), 1))  // 3 - CPU-3
        }

        If (LEqual(\_PR.NLPC, 2))
        {
          Store (0x01, Index (DeRefOf(Index (DEVY, 0)), 1))  // 0 - CPU-0
          Store (0x00, Index (DeRefOf(Index (DEVY, 1)), 1))  // 1 - CPU-1
          Store (0x01, Index (DeRefOf(Index (DEVY, 2)), 1))  // 2 - CPU-2
          Store (0x00, Index (DeRefOf(Index (DEVY, 3)), 1))  // 3 - CPU-3
        }

        If (LEqual(\_PR.NLPC, 3))
        {
          Store (0x01, Index (DeRefOf(Index (DEVY, 0)), 1))  // 0 - CPU-0
          Store (0x01, Index (DeRefOf(Index (DEVY, 1)), 1))  // 1 - CPU-1
          Store (0x01, Index (DeRefOf(Index (DEVY, 2)), 1))  // 2 - CPU-2
          Store (0x00, Index (DeRefOf(Index (DEVY, 3)), 1))  // 3 - CPU-3
        }

        If (LEqual(\_PR.NLPC, 4))
        {
          Store (0x01, Index (DeRefOf(Index (DEVY, 0)), 1))  // 0 - CPU-0
          Store (0x01, Index (DeRefOf(Index (DEVY, 1)), 1))  // 1 - CPU-1
          Store (0x01, Index (DeRefOf(Index (DEVY, 2)), 1))  // 2 - CPU-2
          Store (0x01, Index (DeRefOf(Index (DEVY, 3)), 1))  // 3 - CPU-3
        }

        If (LEqual(ODBG, 0))
        {
          Store (0x01, Index (DeRefOf(Index (DEVY, 8)), 1))  // URT-3
        }

        If (LEqual(ODBG, 1))
        {
          Store (0x00, Index (DeRefOf(Index (DEVY, 8)), 1))  // URT-3       
        }
        
        If (LEqual(SIOE, 0))
        {
          Store (0x00, Index (DeRefOf(Index (DEVY, 22)), 1))  // SDIO disable      
        }
        
        If (LEqual(EMCE, 0))
        {
          Store (0x00, Index (DeRefOf(Index (DEVY, 23)), 1))  // eMMC disable      
        }
                              
        If (LEqual(SDEN, 0))
        {
          Store (0x00, Index (DeRefOf(Index (DEVY, 24)), 1))  // SD card disable      
        }

        If(\_SB.PCI0.RP02.PXSX.WIST()){
          Store (3, Index(DeRefOf(Index(DeRefOf(Index (DeRefOf(Index (DEVY, 27)), 2)), 1)), 1)) // 27 - RP02
          Store (0x01, Index (DeRefOf(Index (DEVY, 27)), 1)) // 27 - RP02
        }

        // Device Constraints Enumeration
        If(LEqual(Arg2, One))
        {
          If (LEqual(S0ID, 1)) {
            Return(DEVY)
          }
          Return(Package() {0})
        }
        // BCCD
        If(LEqual(Arg2, 2))
        {
          Return(BCCD)
        }
        // Screen off notification
        If(LEqual(Arg2, 0x3))
        {
          ADBG("Screen Off Notification")
        }
          
        // Screen on notification
        If(LEqual(Arg2, 0x4))
        {
          ADBG("Screen On Notification")
        }
          
        // resiliency phase entry (deep standby entry)
        If(LEqual(Arg2, 0x5))
        {
          ADBG("PEP DSM:5")
          P8XH (0, 0xC5)
          P8XH (1, 00)
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
          //
          // CRB EC code
          //
          If(LAnd(LAnd(LEqual(S0ID, 1), LEqual(ECLP, 1)), LEqual(ECNO, 1))) { //S0ID: >=1: CS 0: non-CS 
            if(LEqual(ECDB,0x01)){
              ADBG("EC Debug")
              \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.DLED)) //Set EC CS Debug Light (CAPS LOCK)
            }
            \_SB.PCI0.LPCB.H_EC.ECMD (0x2C) // Notify EC of CS entry
          }
#else
          //
          // OEM EC code
          //
#endif
#endif
//[-end-161206-IB07400822-modify]//
        }
         
        // resiliency phase exit (deep standby exit)
        If(LEqual(Arg2, 0x6))
        {
          ADBG("PEP DSM:6")
          P8XH (0, 0x50)
          P8XH (1, 00)
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
          //
          // CRB EC code
          //
          If(LAnd(LAnd(LEqual(S0ID, 1), LEqual(ECLP, 1)), LEqual(ECNO, 1))) { //S0ID: >=1: CS 0: non-CS 
            \_SB.PCI0.LPCB.H_EC.ECMD (0x2D) // Notify EC of CS exit
            If(LEqual(ECDB,0x01)){
              ADBG("EC Debug")
              \_SB.PCI0.LPCB.H_EC.ECWT(0, RefOf(\_SB.PCI0.LPCB.H_EC.DLED)) //Clear EC CS Debug Light (CAPS LOCK)
            }
          } 
#else
          //
          // OEM EC code
          //
#endif
#endif
//[-end-161206-IB07400822-modify]//
        }
      }// If(LEqual(Arg0,ToUUID("c4eb40a0-6cd2-11e2-bcfd-0800200c9a66")))
      Return(One)
    } // Method(_DSM)
  } //device (PEPD)
} // End Scope(\_SB)