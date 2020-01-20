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


// General Purpose Events.  This Scope handles the Run-time and
// Wake-time SCIs.  The specific method called will be determined by
// the _Lxx value, where xx equals the bit location in the General
// Purpose Event register(s).
External(HGLS, MethodObj)
External(HGAS, MethodObj)

Scope(\_GPE)
{
  Method(_L0D, 0) {
    Notify(\_SB.PCI0.XHC, 0x02)
  }

  Method(_L0E, 0) {
    Notify(\_SB.PCI0.HDAS, 0x02)
  }

  // Dummy method for the Tier 1 GPIO SCI enable bit
  Method(_L0F, 0) {}

  //
  // GPIO_12 = EC WAKETIME SCI
  //
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  //
  // CRB EC GPE event
  //
  Method(_L2C, 0) {

//[-start-170410-IB07400857-modify]//
//[-start-170529-IB07400873-modify]//
//#if BXTI_PF_ENABLE
#ifdef SIC_GPIO12_EXAMPLE_CODE
   If (LEqual(IOTP(),0x01)) { // IOTG CRB
     If( LEqual(\OSSL,3))
      { 
        If( LEqual(BDID,MNHL))
       {
         Notify (\_SB.PCI0.LPCB.H_EC.LID0, 0x80)
       }
       Else
       {
         //Store ("Gpe: Power Button(PWRB)", Debug)
         Notify(\_SB.PWRB, 0x80) 
       }
     }
     Else
     {
        Notify (\_SB.PCI0.LPCB.H_EC.LID0, 0x80)
     }
   }
   Else
   {
     Notify (\_SB.PCI0.LPCB.H_EC.LID0, 0x80)
   }
#else
    Notify (\_SB.PCI0.LPCB.H_EC.LID0, 0x80)
#endif
//[-end-170529-IB07400873-modify]//
//[-end-170410-IB07400857-modify]//
  }
#else
  //
  // OEM EC GPE event
  //
#endif
#endif
//[-end-161206-IB07400822-modify]//

  //
  // GPIO_25 = SATA_ODD_DA_IN
  //
  Method(_L39){
      // Required for ACPI 5.0 native Windows support
      Notify(\_SB.PCI0.SATA.PRT0, 2) // Device Wake (Windows)
    }

//[-start-161103-IB07400807-modify]//
  //
  // PCIe Wake0 Status (PCIE_WAKE0_N)
  // APL-I Oxbow Hill: RP5D-> PCIE Lane 2 (0/0x13/2, Port 4) => LAN,              ClkReq = 0, PCIE_WAKE0_N
  // Apollo Lake RVP : RP3D-> PCIE Lane 0 (0/0x13/0, Port 2) -> x4 Slot (Slot 1), ClkReq = 0, PCIE_WAKE0_N
  //
  Method(_L03, 0) { 
//[-start-170116-IB07400834-modify]//
//#if BXTI_PF_ENABLE
    If(LOr(LOr(LEqual(BDID,OXHL),LEqual(BDID,LFHL)),LEqual(BDID,JNHL))) {
      Store(G0S, Local2)
      Or(Local2, 0x1C8, Local2) // Clear bit 3,6,7,8 all PCIe Wake Status
      Store(Local2,G0S)
    } Else {
      Store(1, WK0S)  // Clear GPE Status
    }
//#else
//    Store(1, RP3S)  // Clear GPE Status
//#endif
//#if BXTI_PF_ENABLE
    If(LOr(LOr(LEqual(BDID,OXHL),LEqual(BDID,LFHL)),LEqual(BDID,JNHL))) {
      If(LEqual(RP5D,0))
      {
        \_SB.PCI0.RP05.HPME()
        Notify(\_SB.PCI0.RP05, 0x02)
      }
    } Else {
      If(LEqual(RP3D,0))
      {
        \_SB.PCI0.RP03.HPME()
        Notify(\_SB.PCI0.RP03, 0x02)
      }
    }
//#else
//    If(LEqual(RP3D,0))
//    {
//      \_SB.PCI0.RP03.HPME()
//      Notify(\_SB.PCI0.RP03, 0x02)
//    }
//#endif
//[-end-170116-IB07400834-modify]//
  }
  
  //
  // PCIe Wake1 Status (PCIE_WAKE1_N)
  // APL-I Oxbow Hill: RP3D-> PCIE Lane 0 (0/0x13/0, Port 2) => x4 slot (x2),     ClkReq = 1, PCIE_WAKE1_N
  // Apollo Lake RVP : RP5D-> PCIE Lane 2 (0/0x13/2, Port 4) -> i211,             ClkReq = 1, PCIE_WAKE1_N
  //
//[-start-170116-IB07400834-modify]//
//#if BXTI_PF_ENABLE
  Method(_L06, 0) { // PCIe Wake1 Status
    If(LOr(LOr(LEqual(BDID,OXHL),LEqual(BDID,LFHL)),LEqual(BDID,JNHL))) {
      Store(G0S, Local2)
      Or(Local2, 0x1C8, Local2) // Clear bit 3,6,7,8 all PCIe Wake Status
      Store(Local2,G0S)
      If(LEqual(RP3D,0))
      {
        \_SB.PCI0.RP03.HPME()
        Notify(\_SB.PCI0.RP03, 0x02)
      }
    } else {
      Store(1, WK1S) //Clear GPE Status
      If(LEqual(RP5D,0))
      {
        \_SB.PCI0.RP05.HPME()
        Notify(\_SB.PCI0.RP05, 0x02)
      }
    }
  }
//#endif
//[-end-170116-IB07400834-modify]//

  //
  // PCIe Wake2 Status (PCIE_WAKE2_N)
  // APL-I Oxbow Hill: RP1D-> PCIE Lane 4 (0/0x14/0, Port 0) => M.2 3G,           ClkReq = 2, PCIE_WAKE2_N
  // Apollo Lake RVP : RP1D-> PCIE Lane 4 (0/0x14/0, Port 0) -> x4 Slot (Slot 2), ClkReq = 2, PCIE_WAKE2_N
  //
  Method(_L07, 0) { // PCIe Wake2 Status 
    Store(1, WK2S) //Clear GPE Status
    If(LEqual(RP1D,0))
    {
      \_SB.PCI0.RP01.HPME()
      Notify(\_SB.PCI0.RP01, 0x02)
    }
  }

  //
  // PCIe Wake3 Status (PCIE_WAKE3_N)
  // APL-I Oxbow Hill: RP2D-> PCIE Lane 5 (0/0x14/1, Port 1) => M.2 Wifi/Bluetooth,  ClkReq = 3, PCIE_WAKE3_N
  // Apollo Lake RVP : RP2D-> PCIE Lane 5 (0/0x14/1, Port 1) -> M.2 WLAN,            ClkReq = 3, PCIE_WAKE3_N
  //
  Method(_L08, 0) { // PCIe Wake3 Status 
    Store(1, WK3S) //Clear GPE Status
    If(LEqual(RP2D,0))
    {
      \_SB.PCI0.RP02.HPME()
      Notify(\_SB.PCI0.RP02, 0x02)
    }
  }
//[-end-161103-IB07400807-modify]//

  Method(_L09, 0) {
//[-start-160828-IB07400775-modify]//
//#if BXTI_PF_ENABLE
    if (LEqual(IOTP(),0x01)) {
      Store(G0S, Local2)
      Or(Local2, 0x1C8, Local2)
      Store(Local2,G0S)
    }
//#endif
//[-end-160828-IB07400775-modify]//
    If(LEqual(RP1D,0))
    {
      \_SB.PCI0.RP01.HPME()
      Notify(\_SB.PCI0.RP01, 0x02)
    }

    If(LEqual(RP2D,0))
    {
      \_SB.PCI0.RP02.HPME()
      Notify(\_SB.PCI0.RP02, 0x02)
    }

    If(LEqual(RP3D,0))
    {
      \_SB.PCI0.RP03.HPME()
      Notify(\_SB.PCI0.RP03, 0x02)
    }

    If(LEqual(RP4D,0))
    {
      \_SB.PCI0.RP04.HPME()
      Notify(\_SB.PCI0.RP04, 0x02)
    }
    If(LEqual(RP5D,0))
    {
      \_SB.PCI0.RP05.HPME()
      Notify(\_SB.PCI0.RP05, 0x02)
    }
    If(LEqual(RP6D,0))
    {
      \_SB.PCI0.RP06.HPME()
      Notify(\_SB.PCI0.RP06, 0x02)
    }
  }

//[-start-161026-IB07400806-modify]//
  Method(_L02, 0) {
#ifdef FTPM_ENABLE
#if !FeaturePcdGet(PcdSecureFlashSupported)
    //
    // This workaround only for fTPM(PTT) is enabled and secure flash is disabled.
    //
    If (LEqual(SGPF,1)) { // SW GPE Function 1, disable fTPM after flash full BIOS
      Store (0xff,TPMS)
      Notify(\_SB, 0x0)
    }
#endif
#endif
    
    If (LEqual(SGPF,2)) { // SW GPE Function 2, DTS events
      //
      // Handle DTS Thermal Events.
      //
      If(CondRefOf(\_PR.DTSE))
      {
        If(LGreaterEqual(\_PR.DTSE, 0x01))
        {
          Notify(\_TZ.TZ01,0x80)
        }
      }
    }

    Store (0, SGPF) // Reset SW GPE functions
    Store (0, GPEC) // Disable SWGPE_CTRL to clear the SWGPE Status Bit.
    Store (1, SGPS) // Clear SW GPE Status
  }
//[-end-161026-IB07400806-modify]//
//[-start-161216-IB07400826-add]//
#ifdef GPIO_POWER_BUTTON_EVNET_EXAMPLE
  Method(_L4C, 0) {
    Store (1, N44S) // Clear GPE status
    If (LEqual(ECON, 0)) {
      Notify(\_SB.PWR2,0x80)
    }
  }
#endif
//[-end-161216-IB07400826-add]//
}
