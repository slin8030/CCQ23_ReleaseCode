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

  Device(TFN1)
  {
    Name(_HID, EISAID("INT3404"))  // Intel DPTF Fan Device
    Name(_UID, "TFN1")

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
      If (LEqual(\DFAN,1)){
        Return(0x0F)
      } Else {
        Return(0x00)
      }
    }

    // _FIF (Fan Information)
    //
    // The optional _FIF object provides OSPM with fan device capability information.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //  A Package containing the fan device parameters.
    //
    Method(_FIF)
    {
        Return (Package (){
        0,                            // Revision:Integer
        1,                            // FineGrainControl:Integer Boolean
        2,                            // StepSize:Integer DWORD
        0                             // LowSpeedNotificationSupport:Integer Boolean
        })
    }

    // _FPS (Fan Performance States)
    //
    // Evaluates to a variable-length package containing a list of packages that describe the fan device's performance states.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //  A variable-length Package containing a Revision ID and a list of Packages that describe the fan device's performance states.
    //
    Method(_FPS,,,,PkgObj)
    {
        Return (Package()
        {
          0,    // Revision:Integer
          //        Control,  TripPoint,    Speed,  NoiseLevel, Power
          Package(){100,      0xFFFFFFFF,  12200,   500,        5000},
          Package(){ 95,      0xFFFFFFFF,  11600,   475,        4750},
          Package(){ 90,      0xFFFFFFFF,  11100,   450,        4500},
          Package(){ 85,      0xFFFFFFFF,  10500,   425,        4250},
          Package(){ 80,      0xFFFFFFFF,   9900,   400,        4000},
          Package(){ 75,      0xFFFFFFFF,   9300,   375,        3750},
          Package(){ 70,      0xFFFFFFFF,   8600,   350,        3500},
          Package(){ 60,      0xFFFFFFFF,   7400,   300,        3000},
          Package(){ 50,      0xFFFFFFFF,   6200,   250,        2500},
          Package(){ 40,      0xFFFFFFFF,   4850,   200,        2000},
          Package(){ 25,      0xFFFFFFFF,   2900,   125,        1250},
          Package(){  0,      0xFFFFFFFF,   0,      0,          0}   // OFF
        })
    }

    // _FSL (Fan Set Level)
    //
    // The optional _FSL object is a control method that OSPM evaluates to set a fan device's speed (performance state) to a specific level.
    //
    // Arguments: (1)
    //  Arg0 - Level (Integer): conveys to the platform the fan speed level to be set.
    // Return Value:
    //  None
    //
    // Argument Information
    //  Arg0: Level. If the fan supports fine-grained control, Level is a percentage of maximum level (0-100)
    //  that the platform is to engage the fan. If the fan does not support fine-grained control,
    //  Level is a Control field value from a package in the _FPS object's package list.
    //  A Level value of zero causes the platform to turn off the fan.
    //
    Method(_FSL,1,Serialized)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC Fan Set Level
      //
      If(\_SB.PCI0.LPCB.H_EC.ECAV)
      {
        If(LNot(LEqual(Arg0,\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.PENV)))))
        { // EC implementation-sepcific PWM port control
          \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.PPSL))      // Set bit-0 to select CPU Fan.
          \_SB.PCI0.LPCB.H_EC.ECWT(0, RefOf(\_SB.PCI0.LPCB.H_EC.PPSH))
          \_SB.PCI0.LPCB.H_EC.ECWT(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.PENV)), RefOf(\_SB.PCI0.LPCB.H_EC.PINV))
          \_SB.PCI0.LPCB.H_EC.ECWT(Arg0, RefOf(\_SB.PCI0.LPCB.H_EC.PENV))   // Fan set to Arg0 percent duty cycle.
          \_SB.PCI0.LPCB.H_EC.ECWT(100, RefOf(\_SB.PCI0.LPCB.H_EC.PSTP))
          \_SB.PCI0.LPCB.H_EC.ECMD(0x1a)          // Command to turn the fan on
          Notify(\_SB.IETM, 0x83)                 // Notify DPTF TZ that fan speed has changed
        }
      }
#else
      //
      // OEM EC Fan Set Level
      //
#endif
#endif
//[-end-161206-IB07400822-modify]//
    }

    Name(TFST, Package()
    {
        0,          // Revision:Integer
        0xFFFFFFFF, // Control:Integer DWORD
        0xFFFFFFFF  // Speed:Integer DWORD
    })

    //_FST (Fan Status)
    //
    // The optional _FST object provides status information for the fan device.
    //
    // Arguments: (0)
    //  None
    // Return Value:
    //  A Package containing fan device status information
    //
    Method(_FST,0,Serialized,,PkgObj)
    {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      If(\_SB.PCI0.LPCB.H_EC.ECAV)
      {
        Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.PENV)), Index(TFST, 1))
        Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.CFSP)), Index(TFST, 2))
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
//[-end-161206-IB07400822-modify]//
      Return(TFST)
    }

  } // End TFN1 Device
} // End \_SB
