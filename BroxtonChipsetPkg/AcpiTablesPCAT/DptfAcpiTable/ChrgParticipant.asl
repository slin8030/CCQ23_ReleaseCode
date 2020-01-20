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
  Device(CHRG)  // Charger participant
  {
    Name(_HID, EISAID("INT3403"))
    Name(_UID, "CHRG")
    Name(_STR, Unicode ("Charger participant"))
    Name(PTYP, 0x0B)

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
      If (LEqual(CHGE,1)){
        Return(0x0F)
      } Else {
        Return(0x00)
      }
    }

    // PPSS (Participant Performance Supported States)
    //
    // This object evaluates to a packaged list of information about available power performance states.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   Package of packages
    //
    Name (PPSS, Package()
    {
      Package ()     // Zeroth Power/Performance State
      {
         100,        // DWordConst: Performance
        0,           // DWordConst: Power
        0,           // DWordConst: TransitionLatency
        0,           // Boolean DWordConst : Linear
        0,           // DWordConst: Control
        2000,        // DWordConst: RawPerformance
        "MilliAmps", // String: RawUnit
        0            // DWordConst: Reserved1
      },
      Package ()     // Power/Performance State
      {
          75,        // DWordConst: Performance
        0,           // DWordConst: Power
        0,           // DWordConst: TransitionLatency
        0,           // Boolean DWordConst : Linear
        1,           // DWordConst: Control
        1500,        // DWordConst: RawPerformance
        "MilliAmps", // String: RawUnit
        0            // DWordConst: Reserved1
      },
      Package ()     // Power/Performance State
      {
          50,        // DWordConst: Performance
        0,           // DWordConst: Power
        0,           // DWordConst: TransitionLatency
        0,           // Boolean DWordConst : Linear
        2,           // DWordConst: Control
        1000,        // DWordConst: RawPerformance
        "MilliAmps", // String: RawUnit
        0            // DWordConst: Reserved1
      },
      Package ()     // Power/Performance State
      {
         25,         // DWordConst: Performance
        0,           // DWordConst: Power
        0,           // DWordConst: TransitionLatency
        0,           // Boolean DWordConst : Linear
        3,           // DWordConst: Control
        500,         // DWordConst: RawPerformance
        "MilliAmps", // String: RawUnit
        0            // DWordConst: Reserved1
      },
      Package ()     // Last Power/Performance State
      {
        0,           // DWordConst: Performance
        0,           // DWordConst: Power
        0,           // DWordConst: TransitionLatency
        0,           // Boolean DWordConst : Linear
        4,           // DWordConst: Control
        0,           // DWordConst: RawPerformance
        "MilliAmps", // String: RawUnit
        0            // DWordConst: Reserved1
      }
    }) // End of PPSS object

    // PPPC (Participant Performance Present Capabilities)
    //
    // PPPC evaluates to an integer that will indicate the maximum allowable P-State for OSPM 
    // to use at any given time.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   P states
    //
    Method(PPPC, 0)
    {
      If(\PWRS){ // return 0 for AC, Pn for DC
        Return(0)
      } Else {
        Return(Subtract(SizeOf(PPSS),1))
      }
    }

    // SPPC (Set Participant Performance Capability)
    //
    // SPPC is a control method object that takes one integer parameter that will indicate the maximum 
    // allowable P-State for OSPM to use at any given time.
    //
    // Arguments: (1)
    //   Arg0 - integer
    // Return Value:
    //   None
    //
    Method(SPPC,1,Serialized)
    {
      // bios translates P-state to charge amps and writes CMD/DATA to EC
      If(LLessEqual(ToInteger(Arg0), Subtract(SizeOf(PPSS),1))){ // bounds check requested P-state
        Store(DeRefOf(Index(DeRefOf(Index(PPSS,Arg0)),5)),Local1) // get Charge Rate amp value
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
        //
        // CRB EC ASL code
        //
        \_SB.PCI0.LPCB.H_EC.ECWT(Local1, RefOf(\_SB.PCI0.LPCB.H_EC.BCLM)) // write amp value to EC
        \_SB.PCI0.LPCB.H_EC.ECMD(0x12) // Set Charge Rate command
#else
        //
        // OEM EC ASL code
        //
#endif
#endif        
//[-end-161206-IB07400822-modify]//
      }
    }

    // PPDL (Participant P-state Depth Limit)
    //
    // Indicates dynamically a lower limit on the number of control states currently supported.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing the P-state Depth Limit _PSS entry number:
    //   0 - P0 is the only P-state available for OSPM use
    //   1 - state 1 is the lowest power P-state available
    //   2 - state 2 is the lowest power P-state available
    //   n - state n is the lowest power P-state available
    //
    Method(PPDL, 0)
    {
      Return(Subtract(SizeOf(PPSS),1))
    }

  } // End CHRG Device
}// end Scope(\_SB)
