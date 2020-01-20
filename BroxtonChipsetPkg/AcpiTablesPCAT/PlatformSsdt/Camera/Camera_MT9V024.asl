/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2017 Intel Corporation.

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
External(\OSSL, IntObj)

Scope (\_SB.PCI0.I2C4)
 {
//[-start-170829-IB07400902-remove]//
//	if( LEqual(\OSSL,1))   //OS - Android
//	{
//[-end-170829-IB07400902-remove]//
        Device (PMC4)
        {
            Name (_ADR, Zero)  // _ADR: Address
            Name (_HID, "INT3472")  // _HID: Hardware ID
            Name (_CID, "INT3472")  // _CID: Compatible ID
            Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
            Name (_UID, "4")  // _UID: Unique ID
            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (SBUF, ResourceTemplate ()
                {
                    GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                        "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list GPIO_67
                            0x0037
                        }

                })
                Return (SBUF) /* \_SB_.PCI0.I2C4.PMC4._CRS.SBUF */
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
//[-start-161202-IB07400821-add]//
//[-start-180116-IB07400945-remove]//
//                If(LEqual(HLPS, 0))
//                {
//                  Return(0x0)
//                }
//[-end-180116-IB07400945-remove]//
//[-end-161202-IB07400821-add]//
//[-start-170829-IB07400902-add]//
	              If( LNotEqual(\OSSL,1))   //OS - Not Android
                {
                  Return(0x0)
                }
//[-end-170829-IB07400902-add]//
//[-start-171031-IB07400922-modify]//
                If (LEqual(IOTP(),0x00)) { // CCG CRB
                  Return (0x0)
                }
                If (LEqual (WCAS, 0x03)) // MT9V024
                {
                  Return (0x0F)
                }
                Return (Zero)
//[-end-171031-IB07400922-modify]//
            }

            Method(CLDB)
            {
                Name(PAR, Buffer(0x20)
                {
                    0x00,     //Version
                    0x01,     //Control logic Type 0:  UNKNOWN  1: DISCRETE 2: PMIC TPS68470  3: PMIC uP6641
                    0x04,     //Control logic ID: Control Logic 4
                    0x70,     //CRD board Type, 0: UNKNOWN  0x20: CRD-D  0x30: CRD-G  0x40: PPV 0x50:  CRD-G2  0x70: CRD_G_BXT
                    0x00, 0x00, 0x00, 0x00,     // Reserved
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Reserved
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Reserved
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Reserved
                })
                Return (PAR)
            }            

            Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
            {
                //If ((Arg0 == ToUUID ("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
                  If (LEqual (Arg0, ToUUID("79234640-9E10-4FEA-A5C1-B5AA8B19756F")))
                {

                  If (LEqual (Arg2, One)) 
                    {
                        Return (0x01)
                    }

                  If (LEqual (Arg2,0x02))
                    {
                        Return (0x01004300)  //RESET
                    }
                }

                Return (Zero)
            }
        }

        Device (CAM4)
        {
            Name (_ADR, Zero)  // _ADR: Address
            Name (_HID, "INTCF0A")  // _HID: Hardware ID
            Name (_CID, "INTCF0A")  // _CID: Compatible ID
            Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
            Name (_DDN, "MT9V024")  // _DDN: DOS Device Name
            Name (_UID, 0)  // _UID: Unique ID
            Name (_DEP, Package (0x01)  // _DEP: Dependencies
            {
                \_SB.PCI0.I2C4.PMC4
            })
            Name (PLDB, Package (0x01)
            {
                Buffer (0x14)
                {
                    /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0008 */  0x61, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,  /* i....... */
                    /* 0010 */  0xFF, 0xFF, 0xFF, 0xFF                           /* .... */
                }
            })
            Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
            {
                Return (PLDB) 
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
//[-start-161202-IB07400821-add]//
//[-start-180116-IB07400945-remove]//
//                If(LEqual(HLPS, 0))
//                {
//                  Return(0x0)
//                }
//[-end-180116-IB07400945-remove]//
//[-end-161202-IB07400821-add]//
//[-start-170829-IB07400902-add]//
	              If( LNotEqual(\OSSL,1))   //OS - Not Android
                {
                  Return(0x0)
                }
//[-end-170829-IB07400902-add]//
//[-start-171031-IB07400922-modify]//
                If (LEqual(IOTP(),0x00)) { // CCG CRB
                  Return (0x0)
                }
                If (LEqual (WCAS, 0x03)) // MT9V024
                {
                  Return (0x0F)
                }
                Return (Zero)
//[-end-171031-IB07400922-modify]//
            }

            Method(SSDB)
            {
                Name(PAR, Buffer(0x6C)
                {
                    0x00,                              //Version
                    0x70,                              //CRD board Type, 0: UNKNOWN  0x20: CRD-D  0x30: CRD-G  0x40: PPV 0x50:  CRD-G2  0x70: CRD_G_BXT
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //GUID for CSI2 host controller
                    0x00,                              //DevFunction
                    0x00,                              //Bus
                    0x00, 0x00, 0x00, 0x00,          //DphyLinkEnFuses
                    0x00, 0x00, 0x00, 0x00,          //ClockDiv
                    0x04,                              //LinkUsed
                    0x01,                              //LaneUsed
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_TERMEN_CLANE
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_SETTLE_CLANE
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_TERMEN_DLANE0
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_SETTLE_DLANE0
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_TERMEN_DLANE1
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_SETTLE_DLANE1
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_TERMEN_DLANE2
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_SETTLE_DLANE2
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_TERMEN_DLANE3
                    0x00, 0x00, 0x00, 0x00,          //CSI_RX_DLY_CNT_SETTLE_DLANE3
                    0x00, 0x00, 0x00, 0x00,          //MaxLaneSpeed
                    0x00,                            //SensorCalibrationFileIdx
                    0x00, 0x00, 0x00,                //SensorCalibrationFileIdxInMBZ
                    0x00,                            //RomType: None
                    0x00,                            //VcmType: None
                    0x08,                            //Platform info
                    0x00,                            //Platform sub info
                    0x02,                            //Flash Disabled
                    0x01,                            //Privacy LED supported
                    0x00,                            //0 degree
                    0x01,                            //MIPI link/lane number defined in ACPI --------
                    0x00, 0xF8, 0x24, 0x01,          // MCLK: 19200000Hz
                    0x04,                            //Control logic ID
                    0x00, 0x00, 0x00,                          
                    0x01,                             // M_CLK                    
                    0x00, 0x00, 0x00,                 //Reserved
                    0x00, 0x00, 0x00, 0x00, 0x00,    //Reserved
                    0x00, 0x00, 0x00, 0x00, 0x00,    //Reserved

                })
                Return (PAR)
            }

            Method (_CRS, 0, NotSerialized)  // _CRS: Current Resource Settings
            {
                Name (SBUF, ResourceTemplate ()
                {
                    I2cSerialBus (0x000E, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                        0x00, ResourceConsumer, ,
                        )
                })
                Return (SBUF) /* \_SB_.PCI0.I2C4.CAM4._CRS.SBUF */
            }

            Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
            {
                If (LEqual (Arg0, ToUUID ("822ace8f-2814-4174-a56b-5f029fe079ee")))
                {
                    Return ("5BF801T2")
                }

                If (LEqual (Arg0, ToUUID ("26257549-9271-4ca4-bb43-c4899d5a4881")))
                {
                   If (LEqual (Arg2, One))
                    {
                        Return (0x01)
                    }

                   If (LEqual (Arg2, 0x02))
                    {
                        Return (0x04000E00)
                    }
                }

                Return (Zero)
            }
        }
//[-start-170829-IB07400902-remove]//
//    }
//	  Else  //13M changes are for Windows and Linux
//	  {
//[-end-170829-IB07400902-remove]//
//[-start-170829-IB07400902-modify]//
		        Device (PMC5)
//[-end-170829-IB07400902-modify]//
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_HID, "INT3472")  // _HID: Hardware ID
                Name (_CID, "INT3472")  // _CID: Compatible ID
                Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
                Name (_UID, "0")  // _UID: Unique ID
                Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
                {
                    Name (SBUF, ResourceTemplate ()
                    {
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                            "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                            )
                            {   // Pin list GPIO_67 (GP_CAMERASB05)
                                0x0037
                            }
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                            "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                            )
                            {   // Pin list GPIO_73 (GP_CAMERASB11)
//[-start-180117-IB07400945-modify]//
                                0x003D
//[-end-180117-IB07400945-modify]//
                            }
                    })
                    Return (SBUF) /* \_SB_.PCI0.I2C4.PMC5._CRS.SBUF */
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
//[-start-170829-IB07400902-modify]//
    	              If( LEqual(\OSSL,1))   //OS - Android
                    {
                      Return(0x0)
                    }
//[-start-180116-IB07400945-remove]//
//                    If(LEqual(HLPS, 0))
//                    {
//                      Return(0x0)
//                    }
//[-end-180116-IB07400945-remove]//
//[-start-171031-IB07400922-modify]//
                    If (LEqual(IOTP(),0x00)) { // CCG CRB
                      Return (0x0)
                    }
                    If (LEqual (WCAS, 0x02)) // OV13858
                    {
                      Return (0x0F)
                    }
//[-end-171031-IB07400922-modify]//
//[-end-170829-IB07400902-modify]//

                    Return (Zero)
                }

                Method (CLDB, 0, Serialized)
                {
                    Name (PAR, Buffer (0x20)
                    {
                        /* 0000 */  0x00, 0x01, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
                        /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
                    })
                    Return (PAR) /* \_SB_.PCI0.I2C4.PMC5.CLDB.PAR_ */
                }

                Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
                {
                    
                    If (LEqual (Arg0, ToUUID ("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
                    {
                        If (LEqual (Arg2, One))
                        {
                            Return (0x02)
                        }

                        If (LEqual (Arg2, 0x02))
                        {
                            Return (0x01004300)		// RESET  MCSI2_XSHUTDN = CAMERASB05 = GPIO_67
                        }

                        If (LEqual (Arg2, 0x03))
                        {
//[-start-180117-IB07400945-modify]//
                            Return (0x01004901)   // POWER DOWN PIN GPIO_73 (GP_CAMERASB11)
//[-end-180117-IB07400945-modify]//
                        }
                    }

                    Return (Zero)
                }
            }

//[-start-170829-IB07400902-modify]//
            Device (CAM5)
//[-end-170829-IB07400902-modify]//
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_HID, "OVTID858")  // _HID: Hardware ID
                Name (_CID, "OVTID858")  // _CID: Compatible ID
                Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
                Name (_DDN, "OV13858")  // _DDN: DOS Device Name
                Name (_UID, One)  // _UID: Unique ID
//[-start-170829-IB07400902-modify]//
                Name (_DEP, Package (0x01)  // _DEP: Dependencies
                {
                    \_SB.PCI0.I2C4.PMC5
                })
//[-end-170829-IB07400902-modify]//
                Name (PLDB, Package (0x01)
                {
                    Buffer (0x14)
                    {
                        /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0008 */  0x69, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,  /* i....... */
                        /* 0010 */  0xFF, 0xFF, 0xFF, 0xFF                           /* .... */
                    }
                })
                Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                {
                    CreateField (DerefOf (Index (PLDB, Zero)), 0x73, 0x04, RPOS)
//[-start-170829-IB07400902-modify]//
//                    RPOS = CROT /* External reference */
                    Store(CROT, RPOS)
//[-end-170829-IB07400902-modify]//
                    Return (PLDB) /* \_SB_.PCI0.I2C4.CAM1.PLDB */
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
//[-start-170829-IB07400902-modify]//
    	              If( LEqual(\OSSL,1))   //OS - Android
                    {
                      Return(0x0)
                    }
//[-start-180116-IB07400945-remove]//
//                    If(LEqual(HLPS, 0))
//                    {
//                      Return(0x0)
//                    }
//[-end-180116-IB07400945-remove]//
//[-start-171031-IB07400922-modify]//
                    If (LEqual(IOTP(),0x00)) { // CCG CRB
                      Return (0x0)
                    }
                    If (LEqual (WCAS, 0x02)) // OV13858
                    {
                      Return (0x0F)
                    }
//[-end-171031-IB07400922-modify]//
//[-end-170829-IB07400902-modify]//

                    Return (Zero)
                }

                Method (SSDB, 0, Serialized)
                {
                    Name (PAR, Buffer (0x6C)
                    {
                        /* 0000 */  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* .p...... */
                        /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x06, 0x04, 0x00, 0x00,  /* ........ */
                        /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0030 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0038 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0040 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02,  /* ........ */
                        /* 0050 */  0x08, 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0xF8,  /* ........ */
                        /* 0058 */  0x24, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,  /* $....... */
                        /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0068 */  0x00, 0x00, 0x00, 0x00                           /* .... */
                    })
                    Return (PAR) /* \_SB_.PCI0.I2C4.CAM1.SSDB.PAR_ */
                }

                Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
                {
                    Name (SBUF, ResourceTemplate ()
                    {
                        I2cSerialBus (0x0010, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x000C, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0050, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0051, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0052, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0053, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                    })
                    Return (SBUF) /* \_SB_.PCI0.I2C4.CAM1._CRS.SBUF */
                }

                Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
                {
                    If (LEqual (Arg0, ToUUID ("822ace8f-2814-4174-a56b-5f029fe079ee")))
                    {
                        Return ("CYAGC27")	//13P2BAD33
                    }

                    If (LEqual (Arg0, ToUUID ("26257549-9271-4ca4-bb43-c4899d5a4881")))
                    {
                        If (LEqual (Arg2, One))
                        {
                            Return (0x06)
                        }

                        If (LEqual (Arg2, 0x02))
                        {
                            Return (0x04001000)
                        }

                        If (LEqual (Arg2, 0x03))
                        {
                            Return (0x04000C01)
                        }
                        
                        If (LEqual (Arg2, 0x04))
                        {
                            Return (0x04005002)
                        }
                        
                        If (LEqual (Arg2, 0x05))
                        {
                            Return (0x04005103)
                        }
                        
                        If (LEqual (Arg2, 0x06))
                        {
                            Return (0x04005204)
                        }
                        
                        If (LEqual (Arg2, 0x07))
                        {
                            Return (0x04005305)
                        }
                    }

               Return (Zero)
            }
        }            
//[-start-180117-IB07400945-add]//
            Device (PMC6)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_HID, "INT3472")  // _HID: Hardware ID
                Name (_CID, "INT3472")  // _CID: Compatible ID
                Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
                Name (_UID, "6")  // _UID: Unique ID
                Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
                {
                    Name (SBUF, ResourceTemplate ()
                    {
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                            "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                            )
                            {   // Pin list GPIO_67 (GP_CAMERASB05)
                                0x0037
                            }
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                            "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                            )
                            {   // Pin list GPIO_73 (GP_CAMERASB11)
                                0x003D
                            }
                    })
                    Return (SBUF) /* \_SB_.PCI0.I2C4.PMC5._CRS.SBUF */
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
    	              If( LEqual(\OSSL,1))   //OS - Android
                    {
                      Return(0x0)
                    }
                    If (LEqual(IOTP(),0x00)) { // CCG CRB
                      Return (0x0)
                    }
                    If (LEqual (WCAS, 0x04)) // OV8856
                    {
                      Return (0x0F)
                    }
                    Return (Zero)
                }

                Method (CLDB, 0, Serialized)
                {
                    Name (PAR, Buffer (0x20)
                    {
                        /* 0000 */  0x00, 0x01, 0x06, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
                        /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
                    })
                    Return (PAR) /* \_SB_.PCI0.I2C4.PMC5.CLDB.PAR_ */
                }

                Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
                {
                    
                    If (LEqual (Arg0, ToUUID ("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
                    {
                        If (LEqual (Arg2, One))
                        {
                            Return (0x02)
                        }

                        If (LEqual (Arg2, 0x02))
                        {
                            Return (0x01004300)		// RESET  MCSI2_XSHUTDN = CAMERASB05 = GPIO_67
                        }

                        If (LEqual (Arg2, 0x03))
                        {
                            Return (0x01004901)  // POWER DOWN PIN GPIO_73 (GP_CAMERASB11)
                        }
                    }

                    Return (Zero)
                }
            }
            Device (CAM6)
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_HID, "OVTI8856")  // _HID: Hardware ID
                Name (_CID, "OVTI8856")  // _CID: Compatible ID
                Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
                Name (_DDN, "OV8856")  // _DDN: DOS Device Name
                Name (_UID, "6")  // _UID: Unique ID
                Name (_DEP, Package (0x01)  // _DEP: Dependencies
                {
                    \_SB.PCI0.I2C4.PMC6
                })
                Name (PLDB, Package (0x01)
                {
                    Buffer (0x14)
                    {
                        /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0008 */  0x69, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,  /* i....... */
                        /* 0010 */  0xFF, 0xFF, 0xFF, 0xFF                           /* .... */
                    }
                })
                Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                {
                    Return (PLDB) /* \_SB_.PCI0.I2C4.CAM6.PLDB */
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
    	              If( LEqual(\OSSL,1))   //OS - Android
                    {
                      Return(0x0)
                    }
                    If (LEqual(IOTP(),0x00)) { // CCG CRB
                      Return (0x0)
                    }
                    If (LEqual (WCAS, 0x04)) // OV8856
                    {
                      Return (0x0F)
                    }

                    Return (Zero)
                }

                Method (SSDB, 0, Serialized)
                {
                    Name (PAR, Buffer (0x6C)
                    {
                        /* 0000 */  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* .p...... */
                        /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x06, 0x04, 0x00, 0x00,  /* ........ */
                        /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0030 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0038 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0040 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02,  /* ........ */
                        /* 0050 */  0x08, 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0xF8,  /* ........ */
                        /* 0058 */  0x24, 0x01, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00,  /* $....... */
                        /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0068 */  0x00, 0x00, 0x00, 0x00                           /* .... */
                    })
                    Return (PAR) /* \_SB_.PCI0.I2C4.CAM6.SSDB.PAR_ */
                }

                Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
                {
                    Name (SBUF, ResourceTemplate ()
                    {
                        I2cSerialBus (0x0010, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x000C, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0050, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0051, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0052, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                        I2cSerialBus (0x0053, ControllerInitiated, 0x00061A80,
                            AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                            0x00, ResourceConsumer, ,
                            )
                    })
                    Return (SBUF) /* \_SB_.PCI0.I2C4.CAM6._CRS.SBUF */
                }

                Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
                {
                    If (LEqual (Arg0, ToUUID ("822ace8f-2814-4174-a56b-5f029fe079ee")))
                    {
                        Return ("CJAG824")	//13P2BAD33
                    }

                    If (LEqual (Arg0, ToUUID ("26257549-9271-4ca4-bb43-c4899d5a4881")))
                    {
                        If (LEqual (Arg2, One))
                        {
                            Return (0x06)
                        }

                        If (LEqual (Arg2, 0x02))
                        {
                            Return (0x04001000)
                        }

                        If (LEqual (Arg2, 0x03))
                        {
                            Return (0x04000C01)
                        }
                        
                        If (LEqual (Arg2, 0x04))
                        {
                            Return (0x04005002)
                        }
                        
                        If (LEqual (Arg2, 0x05))
                        {
                            Return (0x04005103)
                        }
                        
                        If (LEqual (Arg2, 0x06))
                        {
                            Return (0x04005204)
                        }
                        
                        If (LEqual (Arg2, 0x07))
                        {
                            Return (0x04005305)
                        }
                    }

               Return (Zero)
            }
        }   
//[-end-180117-IB07400945-add]//
//[-start-170829-IB07400902-remove]//
//	}
//[-end-170829-IB07400902-remove]//
//[-start-180319-IB07400953-add]//
#if 0 // example code to support DPHY1.1 & DPHY1.2 switch

        Device (PMC8)
        {
            Name (_ADR, Zero)  // _ADR: Address
            Name (_HID, "INT3472")  // _HID: Hardware ID
            Name (_CID, "INT3472")  // _CID: Compatible ID
            Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
            Name (_UID, "8")  // _UID: Unique ID
            Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
            {
                Name (SBUF, ResourceTemplate ()
                {
                    GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                        "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list GPIO_67 (GP_CAMERASB05)
                            0x0037
                        }
                    GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                        "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list GPIO_73 (GP_CAMERASB11)
                            0x003D
                        }
                })
                Return (SBUF) /* \_SB_.PCI0.I2C4.PMC8._CRS.SBUF */
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
	              If( LEqual(\OSSL,1))   //OS - Android
                {
                  Return(0x0)
                }
                If (LEqual(IOTP(),0x00)) { // CCG CRB
                  Return (0x0)
                }
                If (LEqual (WCAS, 0x50)) // OV2740
                {
                  Return (0x0F)
                }
                Return (Zero)
            }

            Method (CLDB, 0, Serialized)
            {
                Name (PAR, Buffer (0x20)
                {
                    /* 0000 */  0x00, 0x01, 0x08, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
                    /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
                })
                Return (PAR) /* \_SB_.PCI0.I2C4.PMC8.CLDB.PAR_ */
            }

            Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
            {
                
                If (LEqual (Arg0, ToUUID ("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
                {
                    If (LEqual (Arg2, One))
                    {
                        Return (0x02)
                    }

                    If (LEqual (Arg2, 0x02))
                    {
                        Return (0x01004300)		// RESET  MCSI2_XSHUTDN = CAMERASB05 = GPIO_67
                    }

                    If (LEqual (Arg2, 0x03))
                    {
                        Return (0x01004901)  // POWER DOWN PIN GPIO_73 (GP_CAMERASB11)
                    }
                }

                Return (Zero)
            }
        }
            
        Device (CAM8)
        {
            Name (_ADR, Zero)  // _ADR: Address
            Name (_HID, "INT3474")  // _HID: Hardware ID
            Name (_CID, "INT3474")  // _CID: Compatible ID
            Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
            Name (_DDN, "OV2740")  // _DDN: DOS Device Name
            Name (_UID, "8")  // _UID: Unique ID
            Name (_DEP, Package (0x01)  // _DEP: Dependencies
            {
                \_SB.PCI0.I2C4.PMC8
            })
            Name (PLDB, Package (0x01)
            {
                Buffer (0x14)
                {
                    /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0008 */  0x69, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,  /* a....... */
                    /* 0010 */  0xFF, 0xFF, 0xFF, 0xFF                           /* .... */
                }
            })
            Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
            {
                CreateField (DerefOf (Index (PLDB, Zero)), 0x73, 0x04, RPOS)
                Store(CROT, RPOS)
                Return (PLDB) /* \_SB_.PCI0.I2C4.CAM8.PLDB */
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
                If (LEqual(IOTP(),0x00)) { // CCG CRB
                    Return (Zero)
                }
                If (LEqual (WCAS, 0x50)){ // OV2740
                    Return (0x0F)
                }
                else{
                    Return (Zero)
                }
            }

            Method (SSDB, 0, Serialized)
            {
                Name (PAR, Buffer (0x6C)
                {
                    /* 0000 */  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* .p...... */
                    /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x06, 0x02, 0x00, 0x00,  /* ........ */
                    /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0030 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0038 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0040 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0050 */  0x08, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0xF8,  /* ........ */
                    /* 0058 */  0x24, 0x01, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00,  /* $....... */
                    /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0068 */  0x00, 0x00, 0x00, 0x00                           /* .... */
                })
                Return (PAR) /* \_SB_.PCI0.I2C2.CAM0.SSDB.PAR_ */
            }
            Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
            {
                Name (SBUF, ResourceTemplate ()
                {
                    I2cSerialBus (0x0036, ControllerInitiated, 0x00061A80,
                        AddressingMode7Bit, "\\_SB.PCI0.I2C4",
                        0x00, ResourceConsumer, ,
                        )
                })
                Return (SBUF) /* \_SB_.PCI0.I2C4.CAM8._CRS.SBUF */
            }
            
            Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
            {
              If (LEqual (Arg0, ToUUID("822ace8f-2814-4174-a56b-5f029fe079ee"))){
                  Return ("CIFGF18")	//4SF259T2
              }

        			If (LEqual (Arg0, ToUUID("26257549-9271-4ca4-bb43-c4899d5a4881")))
        			{
                If (LEqual (Arg2, One)){
                  Return (One)
                }

                If (LEqual (Arg2, 0x02)){
                  Return (0x04003600)
                }
              }
              Return (Zero)
            }
        }
        //
        // OV5648
        //
        Device (PMCB)
        {
            Name (_ADR, Zero)  // _ADR: Address
            Name (_HID, "INT3472")  // _HID: Hardware ID
            Name (_CID, "INT3472")  // _CID: Compatible ID
            Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
            Name (_UID, "B")  // _UID: Unique ID
            Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
            {
                Name (SBUF, ResourceTemplate ()
                {
                    GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                        "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list GPIO_67 (GP_CAMERASB05)
                            0x0037
                        }
                    GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                        "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                        )
                        {   // Pin list GPIO_73 (GP_CAMERASB11)
                            0x003D
                        }
                })
                Return (SBUF) /* \_SB_.PCI0.I2C4.PMC8._CRS.SBUF */
            }

            Method (_STA, 0, NotSerialized)  // _STA: Status
            {
	              If( LEqual(\OSSL,1))   //OS - Android
                {
                  Return(0x0)
                }
                If (LEqual(IOTP(),0x00)) { // CCG CRB
                  Return (0x0)
                }
                If (LEqual (WCAS, 0x52)) // OV5648
                {
                  Return (0x0F)
                }
                Return (Zero)
            }

            Method (CLDB, 0, Serialized)
            {
                Name (PAR, Buffer (0x20)
                {
                    /* 0000 */  0x00, 0x01, 0x0B, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
                    /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                    /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
                })
                Return (PAR) /* \_SB_.PCI0.I2C4.PMCB.CLDB.PAR_ */
            }

            Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
            {
                
                If (LEqual (Arg0, ToUUID ("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
                {
                    If (LEqual (Arg2, One))
                    {
                        Return (0x02)
                    }

                    If (LEqual (Arg2, 0x02))
                    {
                        Return (0x01004300)		// RESET  MCSI2_XSHUTDN = CAMERASB05 = GPIO_67
                    }

                    If (LEqual (Arg2, 0x03))
                    {
                        Return (0x01004901)  // POWER DOWN PIN GPIO_73 (GP_CAMERASB11)
                    }
                }

                Return (Zero)
            }
        }
        Device(CAMB)
        {
          Name(_ADR,Zero)//_ADR:Address
          Name(_HID,"OVTI5648")//_HID:HardwareID
          Name(_CID,"OVTI5648")//_CID:CompatibleID
          Name(_SUB,"INTL0000")//_SUB:SubsystemID
          Name(_DDN,"OV5648")//_DDN:DOSDeviceName
          Name(_UID,"B")//_UID:UniqueID
          Name(_DEP,Package(0x01)//_DEP:Dependencies
          {
            \_SB.PCI0.I2C4.PMCB
          })
          Name(PLDB,Package(0x01)
          {
            Buffer(0x14)
            {
            /*0000*/0x82,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
            /*0008*/0x69,0x0C,0x00,0x00,0x03,0x00,0x00,0x00,/*i.......*/
            /*0010*/0xFF,0xFF,0xFF,0xFF/*....*/
            }
          })
          Method(_PLD,0,Serialized)//_PLD:PhysicalLocationofDevice
          {
            CreateField(DerefOf(Index(PLDB,Zero)),0x73,0x04,RPOS)
            Store(CROT, RPOS)
            Return(PLDB)/*\_SB_.PCI0.I2C4.CAMB.PLDB*/
          }
        
          Method(_STA,0,NotSerialized)//_STA:Status
          {
             If (LEqual(IOTP(),0x00)) { // CCG CRB
                  Return (Zero)
             }
             If (LEqual (WCAS, 0x52)){ // OV5648
                  Return (0x0F)
             } else {
                  Return (Zero)
             }
          }
        
          Method(SSDB,0,Serialized)
          {
            Name(PAR,Buffer(0x6C)
            {
              /*0000*/0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,/*.p......*/
              /*0008*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0010*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0018*/0x00,0x00,0x00,0x00,0x06,0x02,0x00,0x00,/*........*/
              /*0020*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0028*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0030*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0038*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0040*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0048*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,/*........*/
              /*0050*/0x08,0x00,0x02,0x00,0x00,0x01,0x00,0xF8,/*........*/
              /*0058*/0x24,0x01,0x0B,0x00,0x00,0x00,0x01,0x00,/*$.......*/
              /*0060*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
              /*0068*/0x00,0x00,0x00,0x00/*....*/
            })
            Return(PAR)/*\_SB_.PCI0.I2C4.CAMB.SSDB.PAR_*/
          }
        
          Method(_CRS,0,Serialized)//_CRS:CurrentResourceSettings
          {
            Name(SBUF,ResourceTemplate()
            {
              I2cSerialBus(0x0036,ControllerInitiated,0x00061A80,
              AddressingMode7Bit,"\\_SB.PCI0.I2C4",
              0x00,ResourceConsumer,,
              )
              I2cSerialBus(0x000C,ControllerInitiated,0x00061A80,
              AddressingMode7Bit,"\\_SB.PCI0.I2C4",
              0x00,ResourceConsumer,,
              )
            })
            Return(SBUF)/*\_SB_.PCI0.I2C4.CAMB._CRS.SBUF*/
          }
        
          Method(_DSM,4,NotSerialized)//_DSM:Device-SpecificMethod
          {
            If (LEqual (Arg0, ToUUID ("822ACE8F-2814-4174-A56B-5F029FE079EE")))
            {
              Return("APLMRD")
            }

            If (LEqual (Arg0, ToUUID ("26257549-9271-4CA4-BB43-C4899D5A4881")))
            {
              If (LEqual (Arg2, One))
              {
                Return (0x02)
              }
              If (LEqual (Arg2, 0x02)) /* I2C 0:bit31-24:BUS. 23-16:Speed.15-8:Addr. 0-7:Function*/
              {
                Return (0x04003600)    //  SENSOR
              }
              If (LEqual (Arg2, 0x03)) /* I2C 0:bit31-24:BUS. 23-16:Speed.15-8:Addr. 0-7:Function*/
              {
                Return (0x04000C01)    // VCM
              }
            }
            Return(Zero)
          }
        }
#endif	
//[-end-180319-IB07400953-add]//
 }