/** @file 
  Intel ACPI Reference Code for Dynamic Platform & Thermal Framework 

@copyright
 Copyright (c) 2015-2016 Intel Corporation. All rights reserved
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

  Device(VIR3)
  {
    Name(_HID, EISAID("INT3409"))  // Intel DPTF Virtual Sensor Participant Device
    Name(_UID, "VIR3")
    Name(PTYP, 0x15) // TypeVirtualSensor
    Name(_STR, Unicode ("Ambient"))

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
      If (LEqual(\VSP3,1)){
        Return(0x0F)
      } Else {
        Return(0x00)
      }
    }

    // Returns Number of Aux Trips available
    Name(PATC, 0)

    Name(LSTM,0)  // Last temperature reported

    // _DTI (Device Temperature Indication)
    //
    // Conveys the temperature of a device's internal temperature sensor to the platform when a temperature trip point
    // is crossed or when a meaningful temperature change occurs.
    //
    // Arguments: (1)
    //   Arg0 - An Integer containing the current value of the temperature sensor (in tenths Kelvin)
    // Return Value:
    //   None
    //
    Method(_DTI, 1)
    {
      Store(Arg0,LSTM)
      Notify(\_SB.VIR3, 0x91) // notify the participant of a trip point change event
    }

    // _NTT (Notification Temperature Threshold)
    //
    // Returns the temperature change threshold for devices containing native temperature sensors to cause
    // evaluation of the _DTI object 
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing the temperature threshold in tenths of degrees Kelvin.
    //
    Method(_NTT, 0)
    {
      Return(2782)  // 5 degree Celcius
    }

    // VSCT (Virtual Sensor Calibration Table)
    //
    // Provides parameters for virtual temperature sensor calculation.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   Package of VSCT packages
    //
    // Name (VSCT, Package())
    // {
    //   Revision,           // DWordConst
    //   Package ()
    //   {
    //     target,          // DeviceReference or "NA" string if no target present,
    //     domain,          // DwordConst,  Enumerated domain type.
    //     coefficientType, // DwordConst,  Enumerated co-efficient type, 0=TEMPERATURE, 1=POWER, 2=CONSTANT
    //     coefficient      // DwordConst,  Co-efficient to be used by virtual temperature sensor policy.
    //     operation        // DwordConst,  Enumerated operation, 0 = Addition (+), 1 = Subtraction (-)
    //     alpha,           // DwordConst,  Gain in .01 increments so 1 would be represented by 100.
    //     triggerPoint     // DwordConst,  Indicates what temperature VTS calculation should be performed. Unit is .1 Kelvin.
    //   }
    // }) // End of VSCT object
    //
    Name (VSCT, Package()
    {
      1,
      Package()
      {
        Package () {\_SB.GEN3, 14,  0, 500,  0,  200, 2932},
        Package () {\_SB.GEN4, 14,  0, 500,  0,  300, 2932}
      }
    })

    // VSPT (Virtual Sensor Polling Table)
    //
    // Used to provide polling intervals for target temperature.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   Package of VSPT packages
    //
    // Name (VSPT, Package()
    // {
    //   Revision,           // DWordConst
    //   Package ()
    //   {
    //     virtualTemp,      // DwordConst, Virtual Temperature in .1 Kelvin.
    //     pollingPeriod     // DwordConst, Period of polling period in .1 Second.
    //   }
    // }) // End of VSPT object
    //
    Name (VSPT, Package()
    {
      1,
      Package()
      {
        Package () {3031, 100},
        Package () {3131, 50},
        Package () {3231, 10}
      }
    })

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC0,0,Serialized)
    {
      If (LEqual(\V3AT,0)) {Return(0xFFFFFFFF)} // signal DPTF driver to disable trip point
      Store(\_SB.IETM.CTOK(\V3AT),Local1)
      If(LGreaterEqual(LSTM,Local1))
      {
        Return(Subtract(Local1,20)) // subtract 2 degrees which is the Hysteresis
      }
      Else
      {
        Return(Local1)
      }
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC1,0,Serialized)
    {
      Return(Subtract(_AC0(), 40))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC2,0,Serialized)
    {
      Return(Subtract(_AC0(), 80))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC3,0,Serialized)
    {
      Return(Subtract(_AC0(), 120))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC4,0,Serialized)
    {
      Return(Subtract(_AC0(), 160))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC5,0,Serialized)
    {
      Return(Subtract(_AC0(), 200))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC6,0,Serialized)
    {
      Return(Subtract(_AC0(), 240))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC7,0,Serialized)
    {
      Return(Subtract(_AC0(), 280))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC8,0,Serialized)
    {
      Return(Subtract(_AC0(), 320))
    }

    // _ACx (Active Cooling)
    //
    // This optional object, if present under a thermal zone, returns the
    //  temperature trip point at which OSPM must start or stop Active cooling,
    //  where x is a value between 0 and 9 that designates multiple active cooling levels of the thermal zone.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the active cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_AC9,0,Serialized)
    {
      Return(Subtract(_AC0(), 360))
    }

    // _PSV (Passive)
    //
    // This optional object, if present under a thermal zone, evaluates to the temperature
    //  at which OSPM must activate passive cooling policy.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the passive cooling temperature threshold in tenths of degrees Kelvin
    //
    Method(_PSV,0,Serialized)
    {
      If (LEqual(\V3PV,0)) {Return(0xFFFFFFFF)} // signal DPTF driver to disable trip point
      Return(\_SB.IETM.CTOK(\V3PV))
    }

    // _CRT (Critical Temperature)
    //
    // This object, when defined under a thermal zone, returns the critical temperature at which OSPM must shutdown the system.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the critical temperature threshold in tenths of degrees Kelvin
    //
    Method(_CRT,0,Serialized)
    {
      If (LEqual(\V3CR,0)) {Return(0xFFFFFFFF)} // signal DPTF driver to disable trip point
      Return(\_SB.IETM.CTOK(\V3CR))
    }

    // _CR3 (Critical Temperature for S3/CS)
    //
    // This object, when defined under a thermal zone, returns the critical temperature at which OSPM
    // must transition to Standby or Connected Standy.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    An Integer containing the critical temperature threshold in tenths of degrees Kelvin
    //
    Method(_CR3,0,Serialized)
    {
      If (LEqual(\V3C3,0)) {Return(0xFFFFFFFF)} // signal DPTF driver to disable trip point
      Return(\_SB.IETM.CTOK(\V3C3))
    }

    // _HOT (Hot Temperature)
    //
    // This optional object, when defined under a thermal zone, returns the critical temperature
    //  at which OSPM may choose to transition the system into the S4 sleeping state.
    //
    //  Arguments: (0)
    //    None
    //  Return Value:
    //    The return value is an integer that represents the critical sleep threshold tenths of degrees Kelvin.
    //
    Method(_HOT,0,Serialized)
    {
      If (LEqual(\V3HT,0)) {Return(0xFFFFFFFF)} // signal DPTF driver to disable trip point
      Return(\_SB.IETM.CTOK(\V3HT))
    }
    
    // _SCP (Set Cooling Policy)
    //
    //  Arguments: (3)
    //    Arg0 - Mode An Integer containing the cooling mode policy code
    //    Arg1 - AcousticLimit An Integer containing the acoustic limit
    //    Arg2 - PowerLimit An Integer containing the power limit
    //  Return Value:
    //    None
    //
    //  Argument Information:
    //    Mode - 0 = Active, 1 = Passive
    //    Acoustic Limit - Specifies the maximum acceptable acoustic level that active cooling devices may generate.
    //    Values are 1 to 5 where 1 means no acoustic tolerance and 5 means maximum acoustic tolerance.
    //    Power Limit - Specifies the maximum acceptable power level that active cooling devices may consume.
    //    Values are from 1 to 5 where 1 means no power may be used to cool and 5 means maximum power may be used to cool.
    //
    Method(_SCP, 3, Serialized)
    {
      If(LOr(LEqual(Arg0,0),LEqual(Arg0,1)))
      {
        Store(Arg0, CTYP)
        Notify(\_SB.VIR3, 0x91)
      }
    }

    Name(VERS,0)  // Version
    Name(CTYP,0)  // Mode
    Name(ALMT,0)  // Acoustic Limit
    Name(PLMT,0)  // Power Limit
    Name(WKLD,0)  // Workload Hint
    Name(DSTA,0)  // Device State Hint
    Name(RES1,0)  // Reserved 1

    // DSCP (DPTF Set Cooling Policy)
    //
    //  Arguments: (7)
    //    Arg0 - Version: For DPTF 8.0, this value is always 0.
    //    Arg1 - Mode: Integer containing the cooling mode policy code
    //    Arg2 - Acoustic Limit: Acoustic Limit value as defined in ACPI specification
    //    Arg3 - Power Limit: Power Limit value as defined in ACPI specification
    //    Arg4 - Workload Hint: Arbitrary Platform defined Integer that indicates to the platform the type of workload run in the OS.
    //    Arg5 - Device State Hint: An integer value that indicates the state of the device.
    //    Arg6 - Reserved 1
    //  Return Value:
    //    None
    //
    //  Argument Information:
    //    Mode: 0 = Active(Typically AC Power Source), 1 = Passive(Typically Battery Power Source)
    //    Acoustic Limit: Specifies the maximum acceptable acoustic level that active cooling devices may generate.
    //      Values are 1 to 5 where 1 means no acoustic tolerance and 5 means maximum acoustic tolerance.
    //    Power Limit: Specifies the maximum acceptable power level that active cooling devices may consume.
    //      Values are from 1 to 5 where 1 means no power may be used to cool and 5 means maximum power may be used to cool.
    //    Workload Hint: Refer to Intel DPTF Configuration Guide on how to set this value for various workloads for each Operating System.
    //    Device State Hint: An integer value that indicates the state of the device.
    //      First byte indicates portrait or landscape mode.
    //      Second byte indicates horizontal or vertical orientation.
    //      Third byte indicates proximity sensor status (if available).
    //      Fourth byte is unused.
    //      xxxxxx00h: Portrait
    //      xxxxxx01h: Landscape
    //      xxxx00xxh: Horizontal
    //      xxxx01xxh: Vertical
    //      xx00xxxxh: Proximity sensor Off (Device not in proximity to user)
    //      xx01xxxxh: Proximity sensor On (Device in proximity to user)
    //
    Method(DSCP, 7, Serialized)
    {
      Name(CHNG,0)  // Semaphore to record policy change

      If(LNotEqual(Arg0, 0)){
        Return // unsupported version
      }

      If(LOr(LEqual(Arg1,0),LEqual(Arg1,1))) // bounds check
      {
        If(LNotEqual(Arg1, CTYP)){
          Store(1,CHNG)
          Store(Arg1, CTYP)
        }
      }

      If(LOr(LGreaterEqual(Arg1,0),LLessEqual(Arg1,5))) // bounds check
      {
        If(LNotEqual(Arg2, ALMT)){
          Store(1, CHNG)
          Store(Arg2, ALMT)
        }
      }

      If(LOr(LGreaterEqual(Arg1,0),LLessEqual(Arg1,5))) // bounds check
      {
        If(LNotEqual(Arg3, PLMT)){
          Store(1, CHNG)
          Store(Arg3, PLMT)
        }
      }

      If(LNotEqual(Arg4, WKLD)){
        Store(1, CHNG)
        Store(Arg4, WKLD)
      }

      If(LNotEqual(Arg5, DSTA)){
        Store(1,CHNG)
        Store(Arg5, DSTA)
      }

      If(LNotEqual(Arg6, RES1)){
        Store(1, CHNG)
        Store(Arg6, RES1)
      }

      If(CHNG){ // only notify when change occurrs
        Notify(\_SB.VIR3, 0x91)
      }
    } // end Method(DSCP)

  } // End VIR3 Device
}// end Scope(\_SB)
