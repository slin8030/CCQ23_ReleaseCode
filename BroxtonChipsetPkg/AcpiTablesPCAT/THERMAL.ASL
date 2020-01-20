/**@file

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/


Scope(\_TZ)
{

   //
  // Memory window to MCHBAR+7000h.
  //
  OperationRegion (MBAR, SystemMemory, Add(ShiftLeft(\_SB.PCI0.VLVC.MHBR,15),0x7000), 0x100)
  Field (MBAR, ByteAcc, NoLock, Preserve)
  {
    Offset (0x1C), // P_CR_PKG_THERM_STATUS_0_0_0_MCHBAR (MCHBAR+0x701C)
      ,      1,       // THRESHOLD1_STATUS[0]
      ,      1,       // THRESHOLD1_LOG[1]
      ,      1,       // THRESHOLD2_STATUS[2]
      ,      1,       // THRESHOLD2_LOG[3]
      ,      1,       // CRITICAL_THERMAL_EVENT_STATUS[4]
      ,      1,       // CRITICAL_THERMAL_EVENT_LOG[5]
      ,     10,       // RESERVED[15:6]
      PKGT,  8,       // TEMPERATURE[23:16]
  }

//[-start-161201-IB07400821-modify]//
//#if BXTI_PF_ENABLE
  OperationRegion(PWMA,SystemMemory,0xE00D0000,0x1000) // PMC MMIO Address
  Field(PWMA, AnyAcc, NoLock, Preserve)
  {
    Offset (0x10),  
      FADR, 32,                    
  }
//#endif
//[-end-161201-IB07400821-modify]//
  
  // Notes:
  //  1) WIN2K strictly uses interrupt driven thermal events.
  //  2) Temperature values are stored in tenths of Kelvin to eliminate the decimal place.
  //  3) Kelvin = Celsius + 273.2.
  //  4) All temperature must be >= 289K.

  // Flag to indicate Legacy thermal control.
  // 1 = Legacy thermal control
  // 0 = other thermal control, like DPTF
  Name(LEGA, 1) // for Active Policy
  Name(LEGP, 1) // for Passive Policy
  Name(LEGC, 1) // for Critical Policy

  // Fan 0 = Package Processor Fan.

  PowerResource(FN00,0,0)
  {

    // Return Virtual Fan 0 status.
    //
    Method(_STA,0,Serialized)
    {
      Return(0x0F)
    }

    Name(FANS,0) // Fan State: 1=ON, 0=OFF

    Method(FCTL,1,Serialized)
    {
//[-start-161202-IB07400821-modify]//
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(LEqual(\ECON,1)) {
        \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.PPSL))      // Set bit-0 to select CPU Fan.
        \_SB.PCI0.LPCB.H_EC.ECWT(0, RefOf(\_SB.PCI0.LPCB.H_EC.PPSH))
        \_SB.PCI0.LPCB.H_EC.ECWT(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.PENV)), RefOf(\_SB.PCI0.LPCB.H_EC.PINV))
        \_SB.PCI0.LPCB.H_EC.ECWT(Arg0, RefOf(\_SB.PCI0.LPCB.H_EC.PENV))   // Fan set to Arg0 percent duty cycle.
        \_SB.PCI0.LPCB.H_EC.ECWT(100, RefOf(\_SB.PCI0.LPCB.H_EC.PSTP))
        \_SB.PCI0.LPCB.H_EC.ECMD(0x1a)          // Command to turn the fan on
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
//[-end-161206-IB07400822-modify]//
//[-end-161202-IB07400821-modify]//
    }

//[-start-161201-IB07400821-modify]//
//#if BXTI_PF_ENABLE
    Method(FPWM,0,Serialized)
    {
      Store (FADR,Local0)  
      If (LNotEqual(Local0, 0xFFFFFFFF))
        {
        OperationRegion(PWM3,SystemMemory,And(FADR,0xFFFFFFF0),0x1000)
        Field(PWM3, AnyAcc, NoLock, Preserve)
          {
            Offset (0xC00),  
            TDIV,  8,       // PWM On Time Divisor
            BUNT, 16,       // PWM Base Unit
            ,      6,       // Reserved
            SWEN,  1,       // PWM SW Update
            PENA,  1,       // PWM Enable    
          }
	
  // Notes:
  //  Initialize PWM Fan 
  //  1) Program the Base Unit and On Time Divisor values.
  //  2) Set the Software Update Bit.
  //  3) Enable the PWM Output by setting PWM Enable.

        Store(0x15, BUNT) 
        Store(0x80, TDIV)
        Store(0x1, SWEN)
        Stall(50)
        Store(0x1, PENA)
        }
    }
//#endif
//[-end-161201-IB07400821-modify]//

    Method(_ON,0,Serialized)
    {
      // If EC access is enabled.
      If(\_TZ.LEGA)
      {
        Store(1,FANS) // save fan state as on
//[-start-161202-IB07400821-modify]//
//#if !BXTI_PF_ENABLE
//        FCTL(100)
//#else
//        If(LGreater(BDID,MNHL)){	
        If (LEqual(IOTP(),0x01)) { // IOTG Board IDs
          FPWM()
        } Else {
          FCTL(100)
        }
//#endif
//[-end-161202-IB07400821-modify]//
      }
    }

    Method(_OFF,0,Serialized)
    {
      // If EC access is enabled.
      If(\_TZ.LEGA)
      {
        Store(0,FANS) // save fan state as off
//[-start-161202-IB07400821-modify]//
//#if !BXTI_PF_ENABLE
//        FCTL(0)
//#else
//        If(LGreater(BDID,MNHL)){	
        If (LEqual(IOTP(),0x01)) { // IOTG Board IDs
          FPWM()
        } Else {
          FCTL(0)
        }
//#endif
//[-end-161202-IB07400821-modify]//
      }
    }
  }

  // Associate Virtual Fan 0 Power Resource with the FAN0 Device.
  //
  Device(FAN0)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID,0)
    Name(_PR0, Package(1){FN00})
  }

  // Thermal Zone 1 = DTS Thermal Zone.
  //
  ThermalZone(TZ01)
  {
    // Return the temperature at which the OS initiates Active Cooling.
    //
    Method(_AC0,0,Serialized)
    {
      If (LEqual(\_TZ.LEGA,0)){ Return(Add(2732,Multiply(210,10)))} // 210 degree C
      Return(Add(2732,Multiply(ACTT,10)))
    }

    // Returns the Device to turn on when _ACx is exceeded.
    //
    Name(_AL0, Package(1){FAN0})

    // Return the temperature at which the OS must perform a Critcal Shutdown.
    //
    Method(_CRT,0,Serialized)
    {
      If (LEqual(\_TZ.LEGC,0)){ Return(Add(2732,Multiply(210,10)))} // 210 degree C
      Return(Add(2732,Multiply(\CRTT,10)))
    }

    // Notifies ASL Code the current cooling mode.
    //  0 - Active cooling
    //  1 - Passive cooling
    //
    Method(_SCP,1,Serialized)
    {
      Store(Arg0,\CTYP)
    }

    // _TMP (Temperature)
    //
    // Return the highest of the CPU temperatures to the OS.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing the current temperature of the thermal zone (in tenths of degrees Kelvin)
    //
    Method(_TMP,0,Serialized)
    {
      //
      // DTS Enabled Case
      //
//[-start-161028-IB07400806-modify]//
      If(\_PR.DTSE)
      {
        //
        // If DTS support is enabled, simply return the
        // higher of the two DTS Temperatures.
        If(LGreaterEqual(\_PR.DTS1, \_PR.DTS2))
        {
          Store (\_PR.DTS1, Local0)
        } Else {
          Store (\_PR.DTS2, Local0)
        }
      }
//[-end-161028-IB07400806-modify]//
      //
      // Max Platform temperature returned by EC
      //
      Else {
        Store (PKGT, Local0) // This depends on EC sending a SCI every second to simulate polling.
      }
      // Data format is signed, 2's complement with the LSB representing 1'C resolution
      If (And (Local0, BIT7)) //MSB is set: Negative number!
      {
        Add (And (Not (Local0), 0xFF), 1, Local0) //Invert and add 1 for 2's complement (AND with 0xFF to prevent extension)
        Return (Subtract (2732, Multiply (Local0, 10)))
      } Else {
        Return (Add (2732, Multiply (Local0, 10)))
      }
    } // End of _TMP

    // Return the Processor(s) used for Passive Cooling.
    //
    Method(_PSL,0,Serialized)
    {
      If(LEqual(MPEN, 4))
      {
        //  CMP - Throttling controls all four logical CPUs.
        Return(Package(){\_PR.CPU0,\_PR.CPU1,\_PR.CPU2,\_PR.CPU3})
      }
      If(MPEN)
      {
        //  CMP - Throttling controls both CPUs.
        Return(Package(){\_PR.CPU0,\_PR.CPU1})
      }
      Return(Package(){\_PR.CPU0})
    }

    // Returns the temperature at which the OS initiates CPU throttling.
    //
    Method(_PSV,0,Serialized)
    {
      If (LEqual(\_TZ.LEGP,0)){ Return(Add(2732,Multiply(110,10)))} // 110 degree C
      Return(Add(2732,Multiply(\PSVT,10)))
    }

    // Returns TC1 value used in the passive cooling formula.
    //
    Method(_TC1,0,Serialized)
    {
      Return(\TC1V)
    }

    // Returns TC2 value used in the passive cooling formula.
    //
    Method(_TC2,0,Serialized)
    {
      Return(\TC2V)
    }

    // Returns the sampling period used in the passive cooling formula.
    //
    Method(_TSP,0,Serialized)
    {
      Return(\TSPV)
    }

  }// end ThermalZone(TZ01)
} // end Scope(\_TZ)
