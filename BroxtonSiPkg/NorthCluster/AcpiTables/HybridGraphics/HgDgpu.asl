/** @file
  This file contains the system BIOS Hybrid graphics code for
  PCH DGPU.

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


External(\_SB.PCI0.RP03, DeviceObj)
External(\_SB.PCI0.RP03._ADR, MethodObj)
External(\_SB.SGOV, MethodObj)
External(\_SB.GGOV, MethodObj)
External(\XBAS)
External(\GBAS)
External(\HGEN)
External(\DLPW)
External(\DLHR)
External(\HRCO)
External(\HRPO)
External(\HRAI)
External(\PECO)
External(\PEPO)
External(\PEAI)
External(\EECP)
External(\RPBA)
External(OSYS)

Scope(\_SB.PCI0.RP03)
{

    // Define a Memory Region that will allow access to the PCH root port
    // Register Block.
    //

    OperationRegion(RPCX,SystemMemory,\RPBA,0x1000)
    Field(RPCX,DWordAcc,NoLock,Preserve)
    {
        Offset(0),
        PVID,   16,
        PDID,   16,
        Offset(0x19),
        SCBN,   8,                      // Secondary Bus Number
        Offset(0x50),                   // LCTL - Link Control Register of (PCI Express* -> B00:D28:F04)
        ASPM,   2,                      // 1:0, ASPM //Not referenced in code
        ,       2,
        LNKD,   1,                      // Link Disable
        Offset(0x328),                  //PCIESTS1 - PCI Express Status 1
        ,       19,
        LNKS,   4,                      //Link Status (LNKSTAT) {22:19}
    }

    //-----------------------------------------
    // Runtime Device Power Management - Begin
    //-----------------------------------------
    // Note:
    //      Runtime Device Power Management can be achieved by using _PRx or _PSx or both

    //
    // Name: PC01
    // Description: Declare a PowerResource object for RP03 slot device
    //
    PowerResource(PC01, 0, 0)
    {
      Name(_STA, One)

      Method(_ON, 0, Serialized)
      {

        If(LNotEqual(OSYS,2009)) {
          \_SB.PCI0.RP03.PEGP.HGON()
          Store(One, _STA)
        }
      }

      Method(_OFF, 0, Serialized)
      {
        If(LNotEqual(OSYS,2009)) {
          \_SB.PCI0.RP03.PEGP.HGOF()
          Store(Zero, _STA)
        }
      }
    } //End of PowerResource(PC01, 0, 0)

    Name(_PR0,Package(){PC01})
    Name(_PR2,Package(){PC01})
    Name(_PR3,Package(){PC01})

    Method(_S0W, 0)
    {
      Return(4) //D3cold is supported
    }

    //-----------------------------------------
    // Runtime Device Power Management - End
    //-----------------------------------------

    Device(PEGP) { // (PCI Express* -> B00:D28:F04) Slot Device D0F0
      Name(_ADR, 0x00000000)
    } // (PCI Express* -> B00:D28:F04) Slot Device D0F0

    Device(PEGA) { // (PCI Express* -> B00:D28:F04) Slot Device D0F1
      Name(_ADR, 0x00000001)

      OperationRegion(ACAP, PCI_Config, \EECP,0x14)
      Field(ACAP,DWordAcc, NoLock,Preserve)
      {
        Offset(0x10),
        LCT1,   16,  // Link Control register
      }
    } // (PCI Express* -> B00:D28:F04) Slot Device D0F1
}

Scope(\_SB.PCI0.RP03.PEGP)
{
    Name (IVID, 0xFFFF) //Invalid Vendor ID
    Name (ELCT, 0x00000000)
    Name (HVID, 0x0000)
    Name (HDID, 0x0000)
    Name (TCNT, 0)
    Name (LDLY, 100) //100 ms

    OperationRegion (PCIS, SystemMemory,Add(\XBAS,ShiftLeft(SCBN,20)), 0xF0)
    Field(PCIS, AnyAcc, Lock, Preserve)
    {
        Offset(0x0),
        DVID, 16,
        Offset(0xB),
        CBCC, 8,
        Offset(0x2C),
        SVID, 16,
        SDID, 16,
    }        

    OperationRegion(PCAP, SystemMemory, Add(Add(\XBAS,ShiftLeft(SCBN,20)),\EECP),0x14)
    Field(PCAP,DWordAcc, NoLock,Preserve)
    {
        Offset(0x10),                    
        LCTL,   16,                      // Link Control register
    }

    Method (_INI)
    {
        Store (0x0, \_SB.PCI0.RP03.PEGP._ADR)
    }

    Method(HGON,0,Serialized)
    {

        //Return if HG is not supported.
        If (LEqual(HGEN,0))
        {
          Return ()
        }

        If (LEqual(CCHK(1), 0))
        {

          Return ()
        }

        //ACTION TODO:
        //........................................................................................
        //While powering up the slot again, the only requirement is that the Reset# should be 
        //de-asserted 100ms after the power to slot is up (Standard requirement as per PCIe spec).

        //Note:
        //Before power enable, and for 100ms after power enable, the reset should be in hold condition.
        //The 100 ms time is given for power rails and clocks to become stable.
        //So during this period, reset must not be released.
        //........................................................................................

        //Power on the dGPU card

        HGPO(HRCO, HRPO, HRAI, 1)                  //Assert dGPU_HOLD_RST# {Hold the dGPU Reset}

        Sleep(DLHR)                                // Wait for 'given'ms

        HGPO(PECO, PEPO, PEAI, 1)                  //Assert dGPU_PWR_EN# {Power on the dGPU}

        Sleep(DLPW)                                // Wait for 'given'ms if dGPU_PWROK has h/w issues

        HGPO(HRCO, HRPO, HRAI, 0)                  //Deassert dGPU_HOLD_RST# {Release the dGPU Reset}

        Sleep(DLHR)                                // Wait for 'given'ms

        //Enable x4 Link
        //This bit should already be set to 0 in the _Off method. But do it anyway.
        Store(0,LNKD)

        //Wait until link has trained to x4
        Store(0, TCNT)
        While(LLess(TCNT, LDLY))
        {
          If(LGreaterEqual(LNKS,7))
          {
            Break
          }
          Sleep(16) //In some OS one tick is equal to 1/64 second (15.625ms)
          Add(TCNT, 16, TCNT)
        }

        // Re-store the DGPU SSID
        Store(HVID,SVID)
        Store(HDID,SDID)

        // Re-store the Link Control register - Common Clock Control and ASPM
        Or(And(ELCT,0x0043),And(LCTL,0xFFBC),LCTL)
        Or(And(ELCT,0x0043),And(\_SB.PCI0.RP03.PEGA.LCT1,0xFFBC),\_SB.PCI0.RP03.PEGA.LCT1)

        Return ()
    }

    Method(_ON,0,Serialized)
    {

        HGON()

        //Ask OS to do a PnP rescan
        Notify(\_SB.PCI0.RP03,0)

        Return ()
    }

    Method(HGOF,0,Serialized)
    {

        //Return if HG is not supported.
        If (LEqual(HGEN,0))
        {
          Return ()
        }

        If (LEqual(CCHK(0), 0))
        {

          Return ()
        }

        //ACTION TODO:
        //........................................................................................
        //To turn off the power to the slot, all you would need to do is assert the RESET# 
        //and then take off the power using the power enable GPIO.
        //Once the power goes off, the clock request from the slot to the PCH is also turned off, 
        //so no clocks will be going to the PCIe slot anymore.
        //........................................................................................
        
        // Save the Link Control register
        Store(LCTL,ELCT)

        // Save the DGPU SSID
        Store(SVID,HVID)
        Store(SDID,HDID)

        //Force disable the x4 link
        Store(1, LNKD)
        //Wait till link is actually in disabled state
        Store(0, TCNT)
        While(LLess(TCNT, LDLY))
        {
          If(LEqual(LNKS,0))
          {
            Break
          }
          Sleep(16) //In some OS one tick is equal to 1/64 second (15.625ms)
          Add(TCNT, 16, TCNT)
        }

        //Power-off the dGPU card

        HGPO(HRCO, HRPO, HRAI, 1)                   // Assert dGPU_HOLD_RST# (PERST#) {Hold the dGPU Reset}
        HGPO(PECO, PEPO, PEAI, 0)                   // Deassert dGPU_PWR_EN#  {Power off the dGPU}

        Return ()
    }

    Method(_OFF,0,Serialized)
    {

        HGOF()

        //Ask OS to do a PnP rescan
        Notify(\_SB.PCI0.RP03,0)

        Return ()
    }

  //
  // Name: HGPO [HG GPIO Write]
  // Description: Function to write into GPIO
  // Input: Arg0 -> GPIO Community Offset
  //        Arg1 -> GPIO Pin Offset
  //        Arg2 -> Active Information
  //        Arg3 -> Value to write
  // Return: Nothing
  //
  Method(HGPO, 4, Serialized)
  {
    // Invert if Active Low
    If (LEqual(Arg2,0))
    {
      Not(Arg3, Arg3)
      And(Arg3, 0x01,Arg3)
    }

    If (LEqual(HGEN, 0x01))
    {
      If (CondRefOf(\_SB.SGOV))
      {
        \_SB.SGOV(Add(Arg0,Arg1), Arg3)
      }
    }

  } // End of Method(HGPO)

  //
  // Name: HGPI [HG GPIO Read]
  // Description: Function to read from GPIO
  // Input: Arg0 -> GPIO Community Offset
  //        Arg1 -> GPIO Pin Offset
  //        Arg2 -> Active Information
  // Return: Value
  //
  Method(HGPI, 3, Serialized)
  {
    If (LEqual(HGEN, 0x01))
    {
      If (CondRefOf(\_SB.GGOV))
      {
        Store(\_SB.GGOV(Add(Arg0,Arg1)), Local0)
      }
    }

    //
    // Invert if Active Low
    //
    If (LEqual(Arg2,0))
    {
      Not(Local0, Local0)
      And(Local0, 0x01, Local0)
    }

    Return(Local0)

  } // End of Method(HGPI)


  //
  // Name: CCHK
  // Description: Function to check whether _ON/_OFF sequence is allowed to execute for the given RP03 controller or not
  // Input: Arg0 -> 0 means _OFF sequence, 1 means _ON sequence
  // Return: 0 - Don't execute the flow, 1 - Execute the flow
  //
  Method(CCHK,1)
  {

    //Check for RP03 controller presence
    If(LEqual(PVID, IVID))
    {

      Return(0)
    }

    //If Endpoint is not present[already disabled] before executing _OFF then don't call the _OFF method
    //If Endpoint is present[already enabled] before executing _ON then don't call the _ON method
    If(LEqual(Arg0, 0))
    {
      //_OFF sequence condition check
      If(LEqual(HGPI(PECO, PEPO, PEAI), 0))
      {

        Return(0)
      }
    }
    ElseIf(LEqual(Arg0, 1))
    {
      //_ON sequence condition check
      If(LEqual(HGPI(PECO, PEPO, PEAI), 1))
      {

        Return(0)
      }
    }


    Return(1)
  } // End of Method(CCHK,1)

}