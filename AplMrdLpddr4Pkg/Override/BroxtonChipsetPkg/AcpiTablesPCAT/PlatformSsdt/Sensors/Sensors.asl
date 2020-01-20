//[-start-170323-IB15590024-add]// 
/** @file
  I2C5 Sensors

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

External(\_SB.PCI0.LPCB.H_EC.KBCD, IntObj)
  
Scope (\_SB.PCI0.I2C5)
{

    Device (ACC)
    {
        Name (_ADR, Zero)  // _ADR: Address
        Name (_HID, "BOSC0200")  // _HID: Hardware ID
        Name (_CID, "BOSC0200")  // _CID: Compatible ID
        Name (_DDN, "Accelerometer")  // _DDN: DOS Device Name
        Name (_UID, One)  // _UID: Unique ID
        Name(_DEP, Package(1) {I2C5})
        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Name (RBUF, ResourceTemplate ()
            {
//                GpioInt (Level, ActiveHigh, Exclusive, PullDefault, 0x0000, 
//                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
//                )
//                {   // Pin list
//                    0x001C
//                }
                I2cSerialBus (0x0019, ControllerInitiated, 0x00061A80,
                    AddressingMode7Bit, "\\_SB.PCI0.I2C5",
                    0x00, ResourceConsumer, ,
                    )
                I2cSerialBus (0x0018, ControllerInitiated, 0x00061A80,
                    AddressingMode7Bit, "\\_SB.PCI0.I2C5",
                    0x00, ResourceConsumer, ,
                    )
            })
            Return (RBUF)
        }

        Method (ROMS, 0, NotSerialized)
        {
            Name (SBUF, Package (0x03)
            {
                "0 1 0", 
                "-1 0 0", 
                "0 0 -1"
            })
            Return (SBUF)
        }

        Method (ROMK, 0, NotSerialized)
        {
            Name (SBUF, Package (0x03)
            {
                "1 0 0", 
                "0 1 0", 
                "0 0 -1"
            })
            Return (SBUF)
        }

        Method (CALS, 1, NotSerialized)
        {
            Return (0xFFFFFFFFFF010203)
        }

        Method (CALK, 1, NotSerialized)
        {
            Return (0xFFFFFFFFFF010203)
        }

        Method (ANGL, 1, NotSerialized)
        {
        }

        Method (SKBC, 1, NotSerialized)
        {
            If (LEqual (Arg0, Zero))
            {
//[-start-171221-IB07400935-modify]//
#ifdef KSC_SUPPORT
                Store (Zero, \_SB.PCI0.LPCB.H_EC.KBCD)
#endif                
//[-end-171221-IB07400935-modify]//
            }
            Else
            {
                If (LEqual (Arg0, 0x03))
                {
//[-start-171221-IB07400935-modify]//
#ifdef KSC_SUPPORT
                    Store (0x03, \_SB.PCI0.LPCB.H_EC.KBCD)
#endif                
//[-end-171221-IB07400935-modify]//
                }
            }
        }

        Method (_STA, 0, NotSerialized)  // _STA: Status
        {
            Return (0x0F)
        }
    }

    Device (CMSR)
    {
        Name (_ADR, Zero)  // _ADR: Address
        Name (_HID, "CPLM3218")  // _HID: Hardware ID
        Name (_CID, "CPLM3218")  // _CID: Compatible ID
        Name (_UID, One)  // _UID: Unique ID
        Name (_DEP, Package (0x01)  // _DEP: Dependencies
        {
            I2C5
        })
        Method (_STA, 0, NotSerialized)  // _STA: Status
        {
//            If (LEqual (CLSE, One))   //CLSE is software switch from GlobalNVS. Will be added if requires.
//            {
//                Return (0x0F)
//            }
            Return (Zero)
        }

        Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
        {
            Name (SBUF, ResourceTemplate ()
            {
                I2cSerialBus (0x0048, ControllerInitiated, 0x00061A80,
                    AddressingMode7Bit, "\\_SB.PCI0.I2C5",
                    0x00, ResourceConsumer, ,
                    )
                Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, ,, )
                {
                    0x0000003A,
                }
            })
            Return (SBUF)
        }

        Name (CPM0, Package (0x07)
        {
            0x7DB5, 
            One, 
            0x0F, 
            0x0840, 
            0x013B, 
            0x011D, 
            Zero
        })
        Name (CPM1, Package (0x03)
        {
            0x0898, 
            0x000186A0, 
            0x000186A0
        })
        Name (CPM2, Package (0x06)
        {
            0x61A8, 
            0x000186A0, 
            0x000186A0, 
            0x000186A0, 
            0x00030D40, 
            0x00186A00
        })
        Name (CPM3, Package (0x04)
        {
            Package (0x02)
            {
                0x3C, 
                0x0A
            }, 

            Package (0x02)
            {
                0x64, 
                0x0B
            }, 

            Package (0x02)
            {
                0x64, 
                0x1F40
            }, 

            Package (0x02)
            {
                0x01F4, 
                0x1F41
            }
        })
        Name (CPM5, Package (0x07)
        {
            Zero, 
            0x000249F0, 
            0x0190, 
            0x03E8, 
            0x02, 
            0x32, 
            0x05
        })
        Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
        {
            If (LEqual (Arg0, Buffer (0x10)
                    {
                        /* 0000 */   0xB6, 0xC6, 0x03, 0x07, 0xCA, 0x1C, 0x44, 0x41,
                        /* 0008 */   0x9F, 0xE7, 0x46, 0x54, 0xF5, 0x3A, 0x0B, 0xD9
                    }))
            {
                If (LEqual (Arg2, Zero))
                {
                    Return (Buffer (One)
                    {
                         0x03
                    })
                }

                If (LEqual (Arg2, One))
                {
                    If (LEqual (ToInteger (Arg3), Zero))
                    {
                        Return (CPM0)
                    }

                    If (LEqual (ToInteger (Arg3), One))
                    {
                        Return (CPM1)
                    }

                    If (LEqual (ToInteger (Arg3), 0x02))
                    {
                        Return (CPM2)
                    }

                    If (LEqual (ToInteger (Arg3), 0x03))
                    {
                        Return (CPM3)
                    }

                    If (LEqual (ToInteger (Arg3), 0x05))
                    {
                        Return (CPM5)
                    }
                }
            }

            Return (Buffer (One)
            {
                 0x00
            })
        }
    }
    
}
//[-end-170323-IB15590024-add]// 