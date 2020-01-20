/** @file
  Intel ACPI Reference Code for Dynamic Platform & Thermal Framework 

@copyright
 Copyright (c) 1999 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/


Scope(\_SB)
{

  Device(TPWR)  // Power participant
  {
    Name(_HID, EISAID("INT3407")) //Intel DPTF platform power device
    Name(_UID, "TPWR")
    Name(_STR, Unicode ("Platform Power"))
    Name(PTYP, 0x11)

    // _STA (Status)
    //
    // This object returns the current status of a device.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing a device status bitmap:
    //    Bit 0 - Set if the device is present.
    //    Bit 1 - Set if the device is enabled and decoding its resources.
    //    Bit 2 - Set if the device should be shown in the UI.
    //    Bit 3 - Set if the device is functioning properly (cleared if device failed its diagnostics).
    //    Bit 4 - Set if the battery is present.
    //    Bits 5-31 - Reserved (must be cleared).
    //
    Method(_STA)
    {
      If (LEqual(\PWRE,1)){
        Return(0x0F)
      } Else {
        Return(0x00)
      }
    }

    // _BST (Battery Status)
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   A Package containing the battery status in the format below:
    //   Package {
    //      Battery State
    //      Battery Present Rate
    //      Battery Remaining Capacity
    //      Battery Present Voltage
    //   }
    //
    Method(_BST,,,,PkgObj)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(CondRefOf(\_SB.PCI0.LPCB.H_EC.BAT1._BST))
      {
        Return(\_SB.PCI0.LPCB.H_EC.BAT1._BST())
      } Else {
        Return(Package(){0,0,0,0})
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
      Return(Package(){0,0,0,0})
//[-end-161206-IB07400822-modify]//
    }

    // _BIX (Battery Information Extended)
    //
    // The _BIX object returns the static portion of the Control Method Battery information. 
    // This information remains constant until the battery is changed. 
    // The _BIX object returns all information available via the _BIF object plus additional battery information. 
    // The _BIF object is deprecated in lieu of _BIX in ACPI 4.0.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   A Package containing the battery information as described below
    //   Package {
    //      Revision
    //      Power Unit
    //      Design Capacity
    //      Last Full Charge Capacity
    //      Battery Technology
    //      Design Voltage
    //      Design Capacity of Warning
    //      Design Capacity of Low
    //      Cycle Count
    //      Measurement Accuracy
    //      Max Sampling Time
    //      Min Sampling Time
    //      Max Averaging Interval
    //      Min Averaging Interval
    //      Battery Capacity Granularity 1
    //      Battery Capacity Granularity 2
    //      Model Number
    //      Serial Number
    //      Battery Type
    //      OEM Information
    //      Battery Swapping Capability
    //   }
    //
    Method(_BIX,,,,PkgObj)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(CondRefOf(\_SB.PCI0.LPCB.H_EC.BAT1._BIX))
      {
        Return(\_SB.PCI0.LPCB.H_EC.BAT1._BIX())
      } Else {
        Return(Package(){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"0","0","0","0",0})
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
      Return(Package(){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"0","0","0","0",0})
//[-end-161206-IB07400822-modify]//
    }

    // PSOC (Platform State of Charge)
    //
    // This object evaluates to the remaining battery state of charge in %.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   remaining battery charge in %
    //
    Method(PSOC)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(LEqual(\_SB.PCI0.LPCB.H_EC.ECAV,0)) // check EC opregion available
      {
        Return (0) // EC not available
      }
      If(LEqual(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1FC)),0)) // prevent divide by zero
      {
        Return (0) // Full charge cannot be 0, indicate malfunction in battery 
      }
      If(LGreater(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1RC)),\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1FC)))) // remaining charge > full charge
      {
        Return(0) // Remaining Charge cannot be greater than Full Charge, this illegal value indicates the battery is malfunctioning
      } 
      If(LEqual(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1RC)),\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1FC)))) // remaining charge == full charge
      {
        Return(100) // indicate battery is fully charged
      } 
      If(LLess(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1RC)),\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1FC)))) // (RC*100)/FC=percent
      {
        Multiply(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1RC)),100,Local0) // RC*100
        Divide(Local0,\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1FC)),Local2,Local1) // divide by FC
        Divide(Local2,100,,Local2) // account for the remainder in percent
        Divide(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.B1FC)),200,,Local3) // what number is 0.5%?
        If(LGreaterEqual(Local2,Local3)){ // round up if the remainder is equal or above .5
          Add(Local1,1,Local1)
        }
        Return(Local1) // current charge in percent
      } Else {
        Return (0) // avoid compiler warning
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
      Return (0) // avoid compiler warning
//[-end-161206-IB07400822-modify]//
    }

    // PMAX (Platform MAXimum)
    //
    // The PMAX object provides maximum power that can be supported by the battery in mW.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   maximum power in mW
    //
    Method(PMAX,0,Serialized)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(LEqual(\_SB.PCI0.LPCB.H_EC.ECAV,1)) // Check If EC opregion is available
      {
        // convert PMAX from signed to unsigned integer
        // PMAX Value = (NOT(OR(FFFF0000, ECValue))+1) * 10.  
        Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.BMAX)),Local0)
        If(Local0){ // don't convert a zero
          Not(Or(0xFFFF0000,Local0,Local0),Local0) // make it a 32 bit value before inverting the bits
          Multiply(Add(1,Local0,Local0),10,Local0) // add 1 and multiply by 10
        }
        Return(Local0)
      } Else {
        Return(0) // EC not available
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
      Return(0) // EC not available
//[-end-161206-IB07400822-modify]//
    }

    // NPWR (N PoWeR)
    //
    // True Platform Power: Could be AC supplied+Battery Pack supplied power or AC supplied.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   Power required to charge battery in mW.
    //
    Method(NPWR)
    {
      Return(20000) // 20 watts
    }

    // PSRC (Power SouRCe)
    //
    // The PSRC object provides power source type.
    //
    // Arguments: (0)
    //   None
    // Return Value: (enumeration which DPTF driver expects)
    //   0 = DC
    //   1 = AC
    //   2 = USB
    //   3 = WC
    //
    // Notes: (Bitmap from EC)
    //   Bit0=1 if AC
    //   Bit1=1 if USB-PD
    //   Bit2=1 if Wireless Charger
    //
    Method(PSRC, 0, Serialized)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(LEqual(\_SB.PCI0.LPCB.H_EC.ECAV,0)) // Check If EC opregion is available
      {
        Return(0) // EC not available, return DC
      } Else {
        Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.PWRT)),Local0)
        Switch(LAnd(Local0,0x07))
        {
          Case(0x0){ // Power source is DC
            Return(0)
          }
          Case(0x1){ // Power source is AC
            Return(1)
          }
          Case(0x2){ // Power source is USB-PD
            Return(2)
          }
          Case(0x4){ // Power source is Wireless Charging
            Return(3)
          }
        }
        Return(0) // Default return is DC
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
      Return(0) // Default return is DC
//[-end-161206-IB07400822-modify]//
    }

    // ARTG (Adapter RaTinG)
    //
    // The ARTG object provides AC adapter rating in mW.
    // ARTG should return 0 if PSRC is DC (0).
    // 
    // Arguments: (0)
    //   None
    // Return Value:
    //   AC adapter rating in mW
    //
    Method(ARTG)
   {
     If (LEqual(PSRC(),1)) // AC check
     {
       Return(90000) // 90 watts
     } Else {
       Return(0)
     }
   }

    // CTYP (Charger TYPe)
    //
    // The CTYP object provides charger type.
    // 
    // Arguments: (0)
    //   None
    // Return Value:
    //  0x01 = Traditional
    //  0x02 = Hybrid
    //  0x03 = NVDC
    //
    Method(CTYP)
   {
      Return(3)
   }

    // PROP (Platform Rest Of worst case Power)
    //
    // This object provides maximum worst case platform rest of power.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   power in milliwatts
    //
    Method(PROP)
   {
      Return(25000) // 25 watts
   }

    // APKP (Ac PeaK Power)
    //
    // This object provides maximum adapter power output.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   power in milliwatts
    //
    Method(APKP)
   {
      Return(90000) // 90 watts
   }

    // APKT (Ac PeaK Time)
    //
    // This object provides the maximum time the adapter can maintain peak power.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   time in milliseconds
    //
    Method(APKT)
   {
      Return(10) // 10 milliseconds
   }

    // PBSS (Power Battery Steady State)
    //
    // Returns max sustained power for battery.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   power in milliwatts
    //
    Method(PBSS)
    {
    }

    // DPSP (DPTF Power Sampling Period)
    //
    // Sets the polling interval in 10ths of seconds. A value of 0 tells the driver to use event notification for PMAX and PBSS..
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing the polling rate in tenths of seconds.
    //   A value of 0 will specify using interrupts through the ACPI notifications.
    //
    //   The granularity of the sampling period is 0.1 seconds. For example, if the sampling period is 30.0
    //   seconds, then report 300; if the sampling period is 0.5 seconds, then will report 5.
    //
    Method(DPSP,0,Serialized)
    {
      Return(\PPPR)
    }

  } // End TPWR Device
}// end Scope(\_SB)
