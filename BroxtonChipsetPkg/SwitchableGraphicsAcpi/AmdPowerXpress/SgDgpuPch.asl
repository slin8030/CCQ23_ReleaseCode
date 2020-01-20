/** @file

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

Scope (\_SB.PCI0)
{
    //
    // SG/HG exist or not
    //
    Method (SGHG, 0)
    {
        Return (0x01)
    }
}

Scope (DGPU_BRIDGE_SCOPE)
{
    OperationRegion (RPCX, SystemMemory, DGPU_SCOPE.DBPA, 0x1000)
    Field (RPCX, DWordAcc, NoLock, Preserve)
    {
        Offset (0),
        PVID, 16,
        PDID, 16,
        Offset (0x50),  // LCTL - Link Control Register of (PCI Express* -> B00:D28:F04)
        ASPM,  2,       // 1:0, ASPM //Not referenced in code
        ,      2,
        LNKD,  1,       // Link Disable
        Offset (0x328), // PCIESTS1 - PCI Express Status 1
        ,     19,
        LNKS,  4,       // Link Status (LNKSTAT) {22:19}
    }

    //-----------------------------------------
    // Runtime Device Power Management - Begin
    //-----------------------------------------
    // Note:
    //      Runtime Device Power Management can be achieved by using _PRx or _PSx or both

    Name (WKEN, 0)
    OperationRegion (PEGB, SystemMemory, DGPU_SCOPE.DBPA, 0x100)
    Field (PEGB, ByteAcc, NoLock, Preserve)
    {
        Offset (0x04), // Command Register
        PCMR, 8,
        Offset (0xA4), // PM D0
        PMST, 2,
    }

    //
    // Name: PC01
    // Description: Declare a PowerResource object for RP01 slot device
    //
    PowerResource (PC01, 0, 0)
    {
        Name (_STA, 1)
        Name (MSD3, 0)

        Method (_ON, 0, Serialized)
        {
            If (LEqual (MSD3, 0x03))
            {
                P8XH (0, 0x55)
                P8XH (1, 0x55)
                If (LEqual (DGPU_SCOPE.SGGP, 0x01))
                {
                    Store (0x07, PCMR)
                    Store (0x00, PMST)
                    DGPU_SCOPE.HGON ()
                    Store (DGPU_SCOPE.DSSV, DGPU_SCOPE.SSSV) // restore SSID SVID
                }
                Store (1, _STA)
                Store (0, MSD3)
            }
        }

        Method (_OFF, 0, Serialized)
        {
            If (LEqual (MSD3, Zero))
            {
                P8XH (0, 0x66)
                P8XH (1, 0x66)
                If (LEqual (DGPU_SCOPE.SGGP, 0x01))
                {
                    DGPU_SCOPE.HGOF ()
                }
                Store (0, _STA)
                Store (3, MSD3)
            }
        }
    } // End of PowerResource (PC01, 0, 0)

    Name (_PR0, Package() {PC01})
    Name (_PR2, Package() {PC01})
    Name (_PR3, Package() {PC01})

    //
    // Name: _PS0
    // Description: D0 Method for RP05 slot device
    // Input: Nothing
    // Return: Nothing
    //
    Method (_PS0, 0, Serialized)
    {
//        DGPU_SCOPE.HGON ()
    }

    //
    // Name: _PS3
    // Description: D3 Method for RP05 slot device
    // Input: Nothing
    // Return: Nothing
    //
    Method (_PS3, 0, Serialized)
    {
//        DGPU_SCOPE.HGOF ()
    }

    Method (_S0W, 0, NotSerialized)
    {
        If (And (\_SB.OSCO, 0x04))
        {
            Return (0x04)
        }
        Else
        {
            Return (0x03)
        }
    }

    Method (_DSW, 3)
    {
        If (Arg1)
        { // Entering Sx, need to disable WAKE# from generating runtime PME
            Store (0, WKEN)
        } Else {  // Staying in S0
            If (LAnd (Arg0, Arg2)) // Exiting D0 and arming for wake
            { // Set PME
                Store (1, WKEN)
            } Else { // Disable runtime PME, either because staying in D0 or disabling wake 
                Store (0, WKEN)
            }
        }
    } // End _DSW

    //-----------------------------------------
    // Runtime Device Power Management - End
    //-----------------------------------------

    Device (DGPU_AUDIO_DEVICE) { // PEG Audio Device 0 Function 1
        Name (_ADR, 0x00000001)
        OperationRegion (ACAP, PCI_Config, DGPU_SCOPE.EECP, 0x14)
        Field (ACAP, DWordAcc, NoLock, Preserve)
        {
            Offset (0x10),
            LCT1,  16,  // Link Control register
        }
    }
}

Scope (DGPU_SCOPE)
{
    Name (ONOF, 0x1) // Endpoint On-Off flag status. Assume Endpoint is ON by default {1-ON, 0-OFF}
    Name (IVID, 0xFFFF) // Invalid Vendor ID
    Name (ELCT, 0x00000000)
    Name (DVID, 0x00000000)
    Name (TCNT, 0)
    Name (LDLY, 100) //100 ms

    OperationRegion (PCIS, PCI_Config, 0x00, 0x100)
    Field (PCIS, DWordAcc, Lock, Preserve)
    {
        Offset (0x00),
        DVVD,  32,
        Offset (0x0B),
        CBCC, 8,
        Offset (0x2C),
        SVID,  16,
        SDID,  16,
        Offset (0x4C), // SSID SVID
        SSSV,  32,
    }

    OperationRegion (PCAP, PCI_Config, EECP, 0x14)
    Field (PCAP, DWordAcc, NoLock, Preserve)
    {
        Offset (0x10),                    
        LCTL,  16,      // Link Control register
    }

    Method (_INI)
    {
        Store (0x0, DGPU_SCOPE._ADR)
    }

    Method (HGON, 0, Serialized)
    {
        If (LEqual (CCHK (1), 0))
        {
            Return ()
        }

        Store (1, ONOF) // Indicate Endpoint is in ON state

        // ACTION TODO:
        //........................................................................................
        // While powering up the slot again, the only requirement is that the Reset# should be 
        // de-asserted 100ms after the power to slot is up (Standard requirement as per PCIe spec).

        // Note:
        // Before power enable, and for 100ms after power enable, the reset should be in hold condition.
        // The 100 ms time is given for power rails and clocks to become stable.
        // So during this period, reset must not be released.
        //........................................................................................
        // Power on the dGPU card
        HGPO(HRCA, HRPA, HRAB, ENBA) 
        Sleep (AHDT)    // Wait for 'given'ms
        HGPO(PECA, PEPA, PEAB, ENPA)                  //Assert dGPU_PWR_EN# {Power on the dGPU}
        Sleep (APDT)    // Wait for 300ms if dGPU_PWROK has h/w issues
        HGPO(HRCA, HRPA, HRAB, DISA) 
        Sleep (IHDT)    // Wait for 100ms

        // Re-store the Link Control register - Common Clock Control and ASPM
        Or (And (ELCT, 0x0043), And (LCTL, 0xFFBC), LCTL)
        Or (And (ELCT, 0x0043), And (DGPU_AUDIO_SCOPE.LCT1, 0xFFBC), DGPU_AUDIO_SCOPE.LCT1)

        Return ()                      
    }

    Method (_ON, 0, Serialized)
    {
        If (LEqual (CCHK (1), 0))
        {
            Return ()
        }

        If (LEqual (DGPU_SCOPE.SGGP, 0x01))
        {
            HGON ()

            // Re-store the DGPU SSID
            Store (DVID, SSSV)
        }

        // Ask OS to do a PnP rescan
        Notify (DGPU_BRIDGE_SCOPE, 0)

        Return ()
    }

    Method (HGOF, 0, Serialized)
    {
        If (LEqual (CCHK (0), 0))
        {
            Return ()
        }

        Store (0, ONOF) // Indicate Endpoint is in OFF state

        // ACTION TODO:
        //........................................................................................
        // To turn off the power to the slot, all you would need to do is assert the RESET# 
        // and then take off the power using the power enable GPIO.
        // Once the power goes off, the clock request from the slot to the PCH is also turned off, 
        // so no clocks will be going to the PCIe slot anymore.
        //........................................................................................

        // Save the Link Control register
        Store (LCTL, ELCT)

        HGPO(HRCA, HRPA, HRAB, ENBA) 
        HGPO(PECA, PEPA, PEAB, DIPA)  

        Return ()
    }

    Method (_OFF, 0, Serialized)
    {
        If (LEqual (CCHK (0), 0))
        {
            Return ()
        }

        If (LEqual (DGPU_SCOPE.SGGP, 0x01))
        {
            // Save the DGPU SSID
            Store (SSSV, DVID)

            HGOF ()
        }

        // Ask OS to do a PnP rescan
        Notify (DGPU_BRIDGE_SCOPE, 0)

        Return ()
    }

    Method (EPON, 0, Serialized)
    {
      Store (1, ONOF) // Indicate Endpoint is in ON state

      Return ()
    }

    Name (GRSC, Buffer(102) {
        0x55, 0x59, 0x5d, 0x60, 0x63, 0x66, 0x62, 0x65, 0x22, 0x25,
        0x24, 0x26, 0x27, 0x23, 0x28, 0x54, 0x3e, 0x3d, 0x40, 0x3b,
        0x36, 0x38, 0x3c, 0x37, 0x3f, 0x39, 0x33, 0x32, 0x35, 0x2f,
        0x34, 0x31, 0x30, 0x2b, 0x2e, 0x29, 0x2d, 0x2a, 0x3a, 0x2c,
        0x5f, 0x69, 0x46, 0x44, 0x43, 0x42, 0x45, 0x47, 0x41, 0x48,
        0x56, 0x5a, 0x58, 0x5c, 0x67, 0x4d, 0x4f, 0x53, 0x4e, 0x51,
        0x50, 0x52, 0x0d, 0x0c, 0x0f, 0x0e, 0x11, 0x12, 0x13, 0x10,
        0x02, 0x01, 0x00, 0x04, 0x06, 0x07, 0x09, 0x08, 0x21, 0x20,
        0x1f, 0x1e, 0x1d, 0x1b, 0x19, 0x1c, 0x1a, 0x17, 0x15, 0x14,
        0x18, 0x16, 0x05, 0x03, 0x0a, 0x0b, 0x6a, 0x57, 0x5b, 0x68,
        0x61, 0x64
    })
    Name (GRNC, Buffer(27) {
        0x13, 0x12, 0x11, 0x14, 0x15, 0x16, 0x18, 0x19, 0x17, 0x10,
        0x0e, 0x0f, 0x0c, 0x1a, 0x1b, 0x01, 0x04, 0x08, 0x0b, 0x00,
        0x03, 0x06, 0x0a, 0x0d, 0x02, 0x05, 0x09
    })
    Name (GSUS, Buffer(44) {
        0x1d, 0x21, 0x1e, 0x1f, 0x20, 0x22, 0x24, 0x23, 0x26, 0x25,
        0x12, 0x07, 0x0b, 0x14, 0x11, 0x01, 0x08, 0x0a, 0x13, 0x0c,
        0x00, 0x02, 0x17, 0x27, 0x1c, 0x1b, 0x16, 0x15, 0x18, 0x19,
        0x1a, 0x33, 0x38, 0x36, 0x31, 0x37, 0x30, 0x39, 0x32, 0x3a,
        0x34, 0x35, 0x3b, 0x28
    })

    //
    // GPIO Read/Write Parameter Check
    // Arg0 = GPIO Group Offset
    // Arg1 = IO Base Registers Offset
    // Arg2 = GPIO Pin Number
    //
    // Return = 0:Parameter Checked Passed, 1:Invalid Parameter 
    //
    Method (RWPC, 3, Serialized)
    {
        Name (RTFG, 1)
        If (LOr (LEqual (ToInteger (Arg1), 0x00), LEqual (ToInteger (Arg1), 0x04)))
        {
            Store (0, RTFG)
        }
        If (LOr (LEqual (ToInteger (Arg1), 0x08), LEqual (ToInteger (Arg1), 0x0C)))
        {
            Store (0, RTFG)
        }
        If (LEqual (RTFG, 1))
        {
            Return (1)
        }

        If (LEqual (ToInteger (Arg0), 0x0000))
        {
            If (LGreaterEqual (ToInteger (Arg2), 102))
            {
                Return (1)
            }
        }
        ElseIf (LEqual (ToInteger (Arg0), 0x1000))
        {
            If (LGreaterEqual (ToInteger (Arg2), 27))
            {
                Return (1)
            }
        }
        ElseIf (LEqual (ToInteger (Arg0), 0x2000))
        {
            If (LGreaterEqual (ToInteger (Arg2), 44))
            {
                Return (1)
            }
        }
        Else
        {
            Return (1)
        }

        Return (0)
    }

    //
    // GPIO Read
    // Arg0 = GPIO Group Offset
    // Arg1 = IO Base Registers Offset
    // Arg2 = GPIO Pin Number 
    //
    Method (GIOR, 3, Serialized)
    {
        If (LEqual (RWPC (Arg0, Arg1, Arg2), 1))
        {
            Return (0xFFFFFFFF)
        }

        If (LEqual (ToInteger (Arg0), 0x0000))
        {
            CreateByteField (GRSC, ToInteger (Arg2), GTP0)
            Store (GTP0, Local0)
        }
        ElseIf (LEqual (ToInteger (Arg0), 0x1000))
        {
            CreateByteField (GRNC, ToInteger (Arg2), GTP1)
            Store (GTP1, Local0)
        }
        ElseIf (LEqual (ToInteger (Arg0), 0x2000))
        {
            CreateByteField (GSUS, ToInteger (Arg2), GTP2)
            Store (GTP2, Local0)
        }

        Store (Add (Add (ToInteger (Arg0), ToInteger (Arg1)), DGPU_SCOPE.GBAS), Local1)
        Store (Add (Multiply (Local0, 0x10), Local1), Local2)
        OperationRegion (GPRG, SystemMemory, Local2, 4)
        Field (GPRG, DWordAcc, NoLock, Preserve) {
            GPDT,   32,
        }

        Return (GPDT)
    }

    //
    // GPIO Write
    // Arg0 = GPIO Group Offset
    // Arg1 = IO Base Registers Offset
    // Arg2 = GPIO Pin Number 
    // Arg3 = GPIO Value
    //
    Method (GIOW, 4, Serialized)
    {
        If (LNotEqual (RWPC (Arg0, Arg1, Arg2), 1))
        {
            If (LEqual (ToInteger (Arg0), 0x0000))
            {
                CreateByteField (GRSC, ToInteger (Arg2), GTP0)
                Store (GTP0, Local0)
            }
            ElseIf (LEqual (ToInteger (Arg0), 0x1000))
            {
                CreateByteField (GRNC, ToInteger (Arg2), GTP1)
                Store (GTP1, Local0)
            }
            ElseIf (LEqual (ToInteger (Arg0), 0x2000))
            {
                CreateByteField (GSUS, ToInteger (Arg2), GTP2)
                Store (GTP2, Local0)
            }

            Store (Add (Add (ToInteger (Arg0), ToInteger (Arg1)), DGPU_SCOPE.GBAS), Local1)
            Store (Add (Multiply (Local0, 0x10), Local1), Local2)
            OperationRegion (GPRG, SystemMemory, Local2, 4)
            Field (GPRG, DWordAcc, NoLock, Preserve) {
                GPDT,   32,
            }
            Store (Arg3, GPDT)
        }
    }

    // SG GPIO Read
    // Arg0 = GPIO Number + GPIO active info + GPIO Group.
    //        Bit 0 to 6 decide which pin will be read,
    //        bit 7 decide this pin high active or low active,
    //        bit 8 to 15 decide the GPIO Group.
    Method (SGPI, 1, Serialized)
    {
        If (LEqual (DGPU_SCOPE.SGGP, 0x01))
        {
            ShiftRight (Arg0, 8, Local0) // GPIO Group
            And (Arg0, 0x00FF, Local1)
            ShiftRight (Local1, 7, Local2) // GPIO active info
            And (Arg0, 0x007F, Local3) // GPIO Number

            Store (GIOR (Multiply (Local0, 0x1000), 0x08, Local3), Local4)

            // 
            // Check if Active Low
            //
            If (LEqual (Local2, 0))
            {
                Not (Local4, Local4)
            }
            Return (And (Local4, 0x01))
        }

        Return (0)
    }

    // SG GPIO Write
    // Arg0 = GPIO Number + GPIO active info + GPIO Group.
    //        Bit 0 to 6 decide which pin will be read,
    //        bit 7 decide this pin high active or low active,
    //        bit 8 to 15 decide the GPIO Group.
    // Arg1 = Value (0/1)
    Method (SGPO, 2, Serialized)
    {
        If (LEqual (DGPU_SCOPE.SGGP, 0x01))
        {
            ShiftRight (Arg0, 8, Local0) // GPIO Group
            And (Arg0, 0x00FF, Local1)
            ShiftRight (Local1, 7, Local2) // GPIO active info
            And (Arg0, 0x007F, Local3) // GPIO Number

            If (LEqual (Local2, 0))
            {
                Not (Arg1, Arg1)
            }
            And (Arg1, 0x01, Arg1)

            Store (GIOR (Multiply (Local0, 0x1000), 0x08, Local3), Local4)
            And (Local4, 0xFFFFFFFE, Local4)
            Or (Arg1, Local4, Arg1)
            GIOW (Multiply (Local0, 0x1000), 0x08, Local3, Arg1)
        }
    }

    //
    // Name: CCHK
    // Description: Function to check whether _ON/_OFF sequence is allowed to execute for the given PEG0 controller or not
    // Input: Arg0 -> 0 means _OFF sequence, 1 means _ON sequence
    // Return: 0 - Don't execute the flow, 1 - Execute the flow
    //
    Method (CCHK, 1)
    {
        // Check for PEG0 controller presence
        If (LEqual (DGPU_BRIDGE_SCOPE.PVID, IVID))
        {
            Return (0)
        }
        // If Endpoint is not present[already disabled] before executing _OFF then don't call the _OFF method
        // If Endpoint is present[already enabled] before executing _ON then don't call the _ON method
        If (LEqual (Arg0, 0))
        {
            // _OFF sequence condition check
            If (LEqual (ONOF, 0))
            {
                Return (0)
            }
        }
        ElseIf (LEqual (Arg0, 1))
        {
            // _ON sequence condition check
            If (LEqual (ONOF, 1))
            {
                Return (0)
            }
        }
        Return (1)
    } // End of Method (CCHK, 1)
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
    }
    OperationRegion(GIWR,SystemMemory,Add(DGPU_SCOPE.GBAS,Add(Arg0,Arg1)),0x20)
    Field(GIWR,AnyAcc,NoLock,Preserve)
    {
        Offset(0x0),
            , 1,
        TEMP, 1,
    }

    Store(Arg3,TEMP)    
 }
}
