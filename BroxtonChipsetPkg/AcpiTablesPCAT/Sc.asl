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

Include ("GpioLib.asl")
Scope(\)
{
  //
  // Define ABASE I/O as an ACPI operating region. The base address
  // can be found in Device 31, Registers 40-43h.
  //
//[-start-161125-IB07400818-modify]//
  OperationRegion(PMIO, SystemIo, \PMBS, 0x54)
  Field(PMIO, ByteAcc, NoLock, Preserve) {
      ,      8,
      PWBS,  1,    // Power Button Status
    Offset(0x50),  // General Purpose Control
      ,      17,
      GPEC,  1
  }
  Field(PMIO, ByteAcc, NoLock, WriteAsZeros) {
  Offset(0x20),  // GPE0 Status
      ,      2,
      SGPS,  1,    // SW GPE Status
      WK0S,  1,    // PCIe Wake 0 GPE Status (pcie_wake0_sts)
      PSCI,  1,    // PUNIT SCI Status
      SCIS,  1,    // GUNIT SCI Status
      WK1S,  1,    // PCIe Wake 1 GPE Status (pcie_wake1_sts)
      WK2S,  1,    // PCIe Wake 2 GPE Status (pcie_wake2_sts)
      WK3S,  1,    // PCIe Wake 3 GPE Status (pcie_wake3_sts)
//[-start-161216-IB07400826-modify]//
  Offset(0x24),    // GPE0b Status
  Offset(0x28),    // GPE0c Status
#ifdef GPIO_POWER_BUTTON_EVNET_EXAMPLE
      ,     12,
      N44S,  1,    // N_GPIO_44 Status
#endif      
//[-end-161216-IB07400826-modify]//
  Offset(0x34),
      ,     25,
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      ZPGE,  1,    // Gpe0b Enable for ZPODD, GPIO_25, GPE39
#endif
  }
//[-end-161125-IB07400818-modify]//

  //
  // Define a Memory Region that will allow access to the PMC
  // Register Block.  Note that in the Intel Reference Solution, the PMC
  // will get fixed up dynamically during POST.
  //
  OperationRegion(PMCR, SystemMemory, Add(DD1A,0x1000), 0x80) // PMC Space
  Field(PMCR,DWordAcc,Lock,Preserve) {
    Offset(0x34),  //  Function Disable Register
    ,      22,
    RP2D,  1,      //  (22) Root Port 2 Disable
    RP1D,  1,      //  (23) Root Port 1 Disable
    Offset(0x38),  //  Function Disable Register 1
    ,      3,
    RP3D,  1,      //  (03) Root Port 3 Disable
    RP4D,  1,      //  (04) Root Port 4 Disable
    RP5D,  1,      //  (05) Root Port 5 Disable
    RP6D,  1,      //  (06) Root Port 6 Disable
  }

//[-start-161103-IB07400807-add]//
  //
  // ApolloLake RVP PCI-E HW port map:
  //
  // RP3D-> PCIE Lane 0 (0/0x13/0, Port 2) -> x4 Slot (Slot 1), ClkReq = 0, PCIE_WAKE0_N
  // RP4D-> PCIE Lane 1 (0/0x13/1, Port 3) -> x4 Slot (Slot 1)
  // RP5D-> PCIE Lane 2 (0/0x13/2, Port 4) -> i211,             ClkReq = 1, PCIE_WAKE1_N
  // RP6D-> PCIE Lane 3 (0/0x13/3, Port 5) -> x
  // RP1D-> PCIE Lane 4 (0/0x14/0, Port 0) -> x4 Slot (Slot 2), ClkReq = 2, PCIE_WAKE2_N
  // RP2D-> PCIE Lane 5 (0/0x14/1, Port 1) -> M.2 WLAN,         ClkReq = 3, PCIE_WAKE3_N
  //
  //
  // ApolloLake-I OxbowHill PCI-E HW port map:
  //
  // RP3D-> PCIE Lane 0 (0/0x13/0, Port 2) => x4 slot (x2),        ClkReq = 1, PCIE_WAKE1_N
  // RP4D-> PCIE Lane 1 (0/0x13/1, Port 3) => x4 slot (x2)
  // RP5D-> PCIE Lane 2 (0/0x13/2, Port 4) => LAN,                 ClkReq = 0, PCIE_WAKE0_N
  // RP6D-> PCIE Lane 3 (0/0x13/3, Port 5) => x
  // RP1D-> PCIE Lane 4 (0/0x14/0, Port 0) => M.2 3G,              ClkReq = 2, PCIE_WAKE2_N 
  // RP2D-> PCIE Lane 5 (0/0x14/1, Port 1) => M.2 Wifi/Bluetooth,  ClkReq = 3, PCIE_WAKE3_N 
  //
//[-end-161103-IB07400807-add]//

  //
  // Support S0, S3, S4, and S5.  The proper bits to be set when
  // entering a given sleep state are found in the Power Management
  // 1 Control ( PM1_CNT ) register, located at ACPIBASE + 04h,
  // bits 10d - 12d.
  //

} //end Scope(\)

scope (\_SB.PCI0) {
  Name(LTRN, 0)
  Name(OBFN, 0)

  Name(LMSL, 0)
  Name(LNSL, 0)

  //
  // LPC Bridge - Device 31, Function 0, this is only for PCH register Memory Region declare,
  // it's better to be declared as early as possible since it's widely used in whole ACPI name space.
  // Please add any code which needs to reference any register of it after this
  //
  Device(LPCB) {
    Name(_ADR, 0x001F0000)

    OperationRegion(LPC, PCI_Config, 0x00, 0x100)
    Field(LPC, AnyAcc, NoLock, Preserve)
    {
      Offset(0x02),
      CDID, 16,
      Offset(0x08),
      CRID,  8,
      Offset(0x80),
      IOD0,  8,
      IOD1,  8,
      Offset(0xA0),
      , 9,
      PRBL,  1,
      Offset(0xAC),
      ,      8,
      ,      8,
      XUSB,  1,
      Offset(0xB8),
          ,  22,
      GR0B,  2,
      ,      8,
      Offset(0xBC),
      ,      2,
      GR19,  2,
      ,     28,
    }
  }

  //
  // PCIE Root Port #01
  //
  Device(RP01) {
    Method (_ADR, 0) {
      If (LNotEqual(RPA1,0)) {
        Return (RPA1)
      } Else {
        Return (0x00140000)
      }
    }
    //
    // Pass LTRx to LTRN so PchPcie.asl can be reused for PCIes.
    //
    Method(_INI)
    {
      Store (LTR1, LTRN)
      Store (PML1, LMSL)
      Store (PNL1, LNSL)
      Store (OBF1, OBFN)
    }
    Include("ScPcie.asl")
//[-start-161103-IB07400807-modify]//
    //
    // APL-I Oxbow Hill: RP1D-> PCIE Lane 4 (0/0x14/0, Port 0) => M.2 3G,           ClkReq = 2, PCIE_WAKE2_N 
    // Apollo Lake RVP : RP1D-> PCIE Lane 4 (0/0x14/0, Port 0) -> x4 Slot (Slot 2), ClkReq = 2, PCIE_WAKE2_N
    // 
    Method(_PRW, 0) { 
      Return(GPRW(0x07, 4)) // PCIE_WAKE2_N
    }  // can wakeup from S4 state
//[-end-161103-IB07400807-modify]//
    Method(_PRT, 0) {
//[-start-170510-IB07400866-modify]//
#ifdef USE_DEFAULT_SIC_IRQ_TABLE
      If(PICM) { Return(AR04) }// APIC mode
      Return (PR04) // PIC Mode
#else
      If(PICM) { Return(AR08) }// APIC mode
      Return (PR08) // PIC Mode
#endif
//[-end-170510-IB07400866-modify]//
    } // end _PRT
  } // end "PCIE Root Port #01"

  //
  // PCIE Root Port #02
  //
  Device(RP02) {
    Method (_ADR, 0) {
      If (LNotEqual(RPA2,0)) {
        Return (RPA2)
      } Else {
        Return (0x00140001)
      }
    }
    //
    // Pass LTRx to LTRN so PchPcie.asl can be reused for PCIes.
    //
    Method(_INI)
    {
      Store (LTR2, LTRN)
      Store (PML2, LMSL)
      Store (PNL2, LNSL)
      Store (OBF2, OBFN)
    }
    Include("ScPcie.asl")
//[-start-161103-IB07400807-modify]//
    //
    // APL-I Oxbow Hill: RP2D-> PCIE Lane 5 (0/0x14/1, Port 1) => M.2 Wifi/Bluetooth,  ClkReq = 3, PCIE_WAKE3_N 
    // Apollo Lake RVP : RP2D-> PCIE Lane 5 (0/0x14/1, Port 1) -> M.2 WLAN,            ClkReq = 3, PCIE_WAKE3_N
    // 
    Method(_PRW, 0) { 
      Return(GPRW(0x08, 4)) // PCIE_WAKE3_N
    }  //GPE enable bit for RP2 is 8, can wake up from S4 state
//[-end-161103-IB07400807-modify]//
    Method(_PRT, 0) {
//[-start-170510-IB07400866-modify]//
#ifdef USE_DEFAULT_SIC_IRQ_TABLE
      If(PICM) { Return(AR05) }// APIC mode
      Return (PR05) // PIC Mode
#else      
      If(PICM) { Return(AR09) }// APIC mode
      Return (PR09) // PIC Mode
#endif      
//[-end-170510-IB07400866-modify]//
    } // end _PRT
  } // end "PCIE Root Port #02"

  //
  // PCIE Root Port #03
  //
  Device(RP03) {
    Method (_ADR, 0) {
      If (LNotEqual(RPA3,0)) {
        Return (RPA3)
      } Else {
        Return (0x00130000)
      }
    }
    //
    // Pass LTRx to LTRN so PchPcie.asl can be reused for PCIes.
    //
    Method(_INI)
    {
      Store (LTR3, LTRN)
      Store (PML3, LMSL)
      Store (PNL3, LNSL)
      Store (OBF3, OBFN)
    }
    Include("ScPcie.asl")
//[-start-161103-IB07400807-modify]//
    //
    // APL-I Oxbow Hill: RP3D-> PCIE Lane 0 (0/0x13/0, Port 2) => x4 slot (x2),     ClkReq = 1, PCIE_WAKE1_N
    // Apollo Lake RVP : RP3D-> PCIE Lane 0 (0/0x13/0, Port 2) -> x4 Slot (Slot 1), ClkReq = 0, PCIE_WAKE0_N
    // 
    Method(_PRW, 0) { 
      If (LEqual(IOTP(),0x01)) { // IOTG Board IDs
        Return(GPRW(0x06, 4)) // PCIE_WAKE1_N
      } else {
        Return(GPRW(0x03, 4)) // PCIE_WAKE0_N
      }
    }  // can wakeup from S4 state
//[-end-161103-IB07400807-modify]//
    Method(_PRT, 0) {
      If(PICM) { Return(AR04) }// APIC mode
      Return (PR04) // PIC Mode
    } // end _PRT
  } // end "PCIE Root Port #03"

  //
  // PCIE Root Port #04
  //
  Device(RP04) {
    Method (_ADR, 0) {
      If (LNotEqual(RPA4,0)) {
        Return (RPA4)
      } Else {
        Return (0x00130001)
      }
    }
    //
    // Pass LTRx to LTRN so PchPcie.asl can be reused for PCIes.
    //
    Method(_INI)
    {
      Store (LTR4, LTRN)
      Store (PML4, LMSL)
      Store (PNL4, LNSL)
      Store (OBF4, OBFN)
    }
    Include("ScPcie.asl")
//[-start-161103-IB07400807-modify]//
    //
    // APL-I Oxbow Hill: RP4D-> PCIE Lane 1 (0/0x13/1, Port 3) => x4 slot (x2)
    // Apollo Lake RVP : RP4D-> PCIE Lane 1 (0/0x13/1, Port 3) -> x4 Slot (Slot 1)
    // 
//    Method(_PRW, 0) { Return(GPRW(0x09, 4)) }  // can wakeup from S4 state
//[-end-161103-IB07400807-modify]//
    Method(_PRT, 0) {
      If(PICM) { Return(AR05) }// APIC mode
      Return (PR05) // PIC Mode
    } // end _PRT
  } // end "PCIE Root Port #04"

  //
  // PCIE Root Port #05
  //
  Device(RP05) {
    Method (_ADR, 0) {
      If (LNotEqual(RPA5,0)) {
        Return (RPA5)
      } Else {
        Return (0x00130002)
      }
    }
    //
    // Pass LTRx to LTRN so PchPcie.asl can be reused for PCIes.
    //
    Method(_INI)
    {
      Store (LTR5, LTRN)
      Store (PML5, LMSL)
      Store (PNL5, LNSL)
      Store (OBF5, OBFN)
    }
    Include("ScPcie.asl")
//[-start-161103-IB07400807-modify]//
    //
    // APL-I Oxbow Hill: RP5D-> PCIE Lane 2 (0/0x13/2, Port 4) => LAN,              ClkReq = 0, PCIE_WAKE0_N
    // Apollo Lake RVP : RP5D-> PCIE Lane 2 (0/0x13/2, Port 4) -> i211,             ClkReq = 1, PCIE_WAKE1_N
    // 
    Method(_PRW, 0) { 
      If (LEqual(IOTP(),0x01)) { // IOTG Board IDs
        Return(GPRW(0x03, 4)) // PCIE_WAKE0_N
      } else {
        Return(GPRW(0x06, 4)) // PCIE_WAKE1_N
      }
    } 
//[-end-161103-IB07400807-modify]//
    Method(_PRT, 0) {
      If(PICM) { Return(AR06) }// APIC mode
      Return (PR06) // PIC Mode
    } // end _PRT
  } // end "PCIE Root Port #05"

  //
  // PCIE Root Port #06
  //
  Device(RP06) {
    Method (_ADR, 0) {
      If (LNotEqual(RPA6,0)) {
        Return (RPA6)
      } Else {
        Return (0x00130003)
      }
    }
    //
    // Pass LTRx to LTRN so PchPcie.asl can be reused for PCIes.
    //
    Method(_INI)
    {
      Store (LTR6, LTRN)
      Store (PML6, LMSL)
      Store (PNL6, LNSL)
      Store (OBF6, OBFN)
    }
    Include("ScPcie.asl")
//[-start-161103-IB07400807-modify]//
    //
    // APL-I Oxbow Hill: RP6D-> PCIE Lane 3 (0/0x13/3, Port 5) => x
    // Apollo Lake RVP : RP6D-> PCIE Lane 3 (0/0x13/3, Port 5) -> x
    // 
//    Method(_PRW, 0) { Return(GPRW(0x09, 4)) }  // can wakeup from S4 state
//[-end-161103-IB07400807-modify]//
    Method(_PRT, 0) {
      If(PICM) { Return(AR07) }// APIC mode
      Return (PR07) // PIC Mode
    } // end _PRT
  } // end "PCIE Root Port #06"

  include ("ScSata.asl")
  // xHCI Controller - Device 20, Function 0
  include ("ScXhci.asl")
  include ("ScXdci.asl")
  include ("ScScc.asl")
  include ("ScLpss.asl")
  include ("ScAudio.asl")
}
