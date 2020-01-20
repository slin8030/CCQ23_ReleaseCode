/** @file

@copyright
 Copyright (c) 2012 - 2016 Intel Corporation. All rights reserved
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

Scope(\_SB.PCI0.I2C2)
{
//[-start-170516-IB08450375-add]//
//[-start-170518-IB07400867-remove]//
//#if !BXTI_PF_ENABLE
//[-end-170518-IB07400867-remove]//
//[-end-170516-IB08450375-add]//
  Device (PMC1) {
    Name (_ADR, Zero)
    Name (_HID, "INT3472")
    Name (_CID, "INT3472")
    Name (_DDN, "INCL-CRDD")
    Name (_UID, "0")

    Method (_CRS, 0x0, Serialized) {
      Name (SBUF, ResourceTemplate() {
        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,) {
          0x37     // GPIO_67
        }
        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,) {
          0x3C     // GPIO_72
        }
      })

      Return (SBUF)
    }

    Method (_STA, 0, NotSerialized) {
//[-start-170518-IB07400867-add]//
      If (LEqual(IOTP(),0x01)) { // IOTG CRB
          Return (Zero)
      }
//[-end-170518-IB07400867-add]//
      If (LEqual (WCAS, 2)) {  // 2-IMX135
        Return (0x0F)
      }
      Return (0x0)
    }

    Method (CLDB, 0, Serialized) {
      Name (PAR, Buffer(0x20) {
        0x00,     //Version
        0x01,     //Control logic Type 0:  UNKNOWN  1: DISCRETE 2: PMIC TPS68470  3: PMIC uP6641
        0x00,     //Control logic ID: Control Logic 0
        0x70,     //CRD board Type, 0: UNKNOWN  0x20: CRD-D  0x30: CRD-G  0x40: PPV 0x50:  CRD-G2  0x70: CRD_G_BXT
        0x00, 0x00, 0x00, 0x00,     // Reserved
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Reserved
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Reserved
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     // Reserved
      })
      Return (PAR)
    }

    Method (_DSM, 4, NotSerialized) {
      If (LEqual (Arg0, ToUUID("79234640-9E10-4FEA-A5C1-B5AA8B19756F"))) {
        If (LEqual (Arg2, One)) {
          Return (0x02)           // number
        }
        If (LEqual (Arg2, 0x02)) {
          Return (0x01004300)     // RESET
        }
        If (LEqual (Arg2, 0x03)) {
          Return (0x01004801)    //  POWER DOWN
        }
      }
      Return (Zero)
    }
  }

  //
  // Rear Camera IMX135
  //
  Device (CAM1) {
    Name (_ADR, Zero)
    Name (_HID, "INT3471")
    Name (_CID, "INT3471")
    Name (_SUB, "INTL0000")
    Name (_DDN, "SONY IMX135")
    Name (_UID, One)
    Name (_DEP, Package () {
      \_SB.PCI0.I2C2.PMC1
    })
    Name (PLDB, Package(1) {
      Buffer(0x14)
      {
        0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x69, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF
      }
    })
    Method (_PLD, 0, Serialized) {
//[-start-160718-IB06720413-add]//
      CreateField(DerefOf(Index(PLDB,0)), 115,  4, RPOS) // Rotation field
      Store(CROT, RPOS)
//[-end-160718-IB06720413-add]//

      Return(PLDB)
    }

    Method (_STA, 0, NotSerialized) {
//[-start-170518-IB07400867-add]//
      If (LEqual(IOTP(),0x01)) { // IOTG CRB
          Return (Zero)
      }
//[-end-170518-IB07400867-add]//
      If(LEqual (WCAS, 2)) {  // 2-IMX135
        Return (0x0F)
      }
      Return (0x0)
    }

    Method (SSDB, 0, Serialized) {
      Name (PAR, Buffer(0x6C) {
        0x00,                              //Version
        0x70,                              //SKU: CRD_G_BXT
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     //GUID for CSI2 host controller
        0x00,                              //DevFunction
        0x00,                              //Bus
        0x00, 0x00, 0x00, 0x00,            //DphyLinkEnFuses
        0x00, 0x00, 0x00, 0x00,            //ClockDiv
        0x06,                              //LinkUsed
        0x04,                              //LaneUsed
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_TERMEN_CLANE
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_SETTLE_CLANE
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_TERMEN_DLANE0
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_SETTLE_DLANE0
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_TERMEN_DLANE1
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_SETTLE_DLANE1
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_TERMEN_DLANE2
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_SETTLE_DLANE2
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_TERMEN_DLANE3
        0x00, 0x00, 0x00, 0x00,            //CSI_RX_DLY_CNT_SETTLE_DLANE3
        0x00, 0x00, 0x00, 0x00,            //MaxLaneSpeed
        0x00,                              //SensorCalibrationFileIdx
        0x00, 0x00, 0x00,                  //SensorCalibrationFileIdxInMBZ
        0x00,                              //RomType: NONE
        0x03,                              //VcmType: AD5816
        0x08,                              //Platform info  BXT
        0x00,                              //Platform sub info
        0x03,                              //Flash ENABLED
        0x00,                              //Privacy LED  not supported
        0x00,                              //0 degree
        0x01,                              //MIPI link/lane defined in ACPI
        0x00, 0xF8, 0x24, 0x01,            // MCLK: 19200000Hz
        0x00,                              //Control logic ID
        0x00, 0x00, 0x00,
        0x02,                              // M_CLK Port
        0x00, 0x00, 0x00,                  //Reserved
        0x00, 0x00, 0x00, 0x00, 0x00,      //Reserved
        0x00, 0x00, 0x00, 0x00, 0x00,      //Reserved
      })
      Return (PAR)
    }

    Method (_CRS, 0, Serialized) {
      Name (SBUF, ResourceTemplate() {
        I2CSerialBus(0x0010, ControllerInitiated, 0x00061A80, AddressingMode7Bit,
          "\\_SB.PCI0.I2C2", 0x00, ResourceConsumer,,)
        I2CSerialBus(0x000E, ControllerInitiated, 0x00061A80, AddressingMode7Bit,
          "\\_SB.PCI0.I2C2", 0x00, ResourceConsumer,,)
      })
      Return(SBUF)
    }

    Method (_DSM, 4, NotSerialized)
    {
      If (LEqual (Arg0, ToUUID ("822ACE8F-2814-4174-A56B-5F029FE079EE")))
      {
        Return("13P2BAD33")
      }

      If (LEqual (Arg0, ToUUID ("26257549-9271-4CA4-BB43-C4899D5A4881")))
      {
        If (LEqual (Arg2, One))
        {
          Return (0x02)
        }
        If (LEqual (Arg2, 0x02)) /* I2C 0:bit31-24:BUS. 23-16:Speed.15-8:Addr. 0-7:Function*/
        {
          Return (0x02001000)  //  SENSOR
        }
        If (LEqual (Arg2, 0x03)) /* I2C 0:bit31-24:BUS. 23-16:Speed.15-8:Addr. 0-7:Function*/
        {
          Return (0x02000E01)   // VCM
        }
      }

      Return(Zero)
    }
  }
//[-start-170516-IB08450375-add]//
//[-start-170518-IB07400867-remove]//
//#else
//[-end-170518-IB07400867-remove]//
//[-start-170518-IB07400867-modify]//
   Device (PMC3)
//[-end-170518-IB07400867-modify]//
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_HID, "INT3472")  // _HID: Hardware ID
                Name (_CID, "INT3472")  // _CID: Compatible ID
                Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
                Name (_UID, "3")  // _UID: Unique ID
                Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
                {
                    Name (SBUF, ResourceTemplate ()
                    {
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,      IoRestrictionOutputOnly,
                            "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                            )
                            {   // Pin list GPIO_64 (GP_CAMERASB02)
                                0x0034
                            }
                        GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,   IoRestrictionOutputOnly,
                            "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                            )
                            {   // Pin list GPIO_71 (GP_CAMERASB09)
                                0x003B
                            }
                    })
                    Return (SBUF) /* \_SB_.PCI0.I2C2.PMC3._CRS.SBUF */
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
//[-start-170518-IB07400867-add]//
                    If (LEqual(IOTP(),0x00)) { // CCG CRB
                        Return (Zero)
                    }
//[-end-170518-IB07400867-add]//
                    If (LEqual (UCAS, One)){
                        Return (0x0F)
                    }
                   else {
                        Return (Zero)
                   }
                }

                Method (CLDB, 0, Serialized)
                {
                    Name (PAR, Buffer (0x20)
                    {
                        /* 0000 */  0x00, 0x01, 0x03, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
                        /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
                    })
                    Return (PAR) /* \_SB_.PCI0.I2C2.PMC3.CLDB.PAR_ */
                }

                Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
                {
                    If (LEqual (Arg0, ToUUID("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
                    {
                        If (LEqual (Arg2, One)){
                            Return (0x02)
                        }

                        If (LEqual (Arg2, 0x02)){
                            Return (0x01004000)		// RESET  MCSI1_XSHUTDN = CAMERASB02 = GPIO_64
                        }

                        If (LEqual (Arg2, 0x03)) { // Power Down GPIO_71
                            Return (0x01004701)
                        }
                    }

                    Return (Zero)
                }
            }
            
//[-start-170518-IB07400867-modify]//
      Device (CAM3)
//[-end-170518-IB07400867-modify]//
            {
                Name (_ADR, Zero)  // _ADR: Address
                Name (_HID, "INT3474")  // _HID: Hardware ID
                Name (_CID, "INT3474")  // _CID: Compatible ID
                Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
                Name (_DDN, "OV2740")  // _DDN: DOS Device Name
                Name (_UID, One)  // _UID: Unique ID
                Name (_DEP, Package (0x01)  // _DEP: Dependencies
                {
//[-start-170518-IB07400867-modify]//
                    \_SB.PCI0.I2C2.PMC3
//[-end-170518-IB07400867-modify]//
                })
                Name (PLDB, Package (0x01)
                {
                    Buffer (0x14)
                    {
                        /* 0000 */  0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0008 */  0x61, 0x0C, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,  /* a....... */
                        /* 0010 */  0xFF, 0xFF, 0xFF, 0xFF                           /* .... */
                    }
                })
                Method (_PLD, 0, Serialized)  // _PLD: Physical Location of Device
                {
                    CreateField (DerefOf (Index (PLDB, Zero)), 0x73, 0x04, RPOS)
//[-start-170518-IB07400867-modify]//
//                    RPOS = CROT /* External reference */
                    Store(CROT, RPOS)
//[-end-170518-IB07400867-modify]//
                    Return (PLDB) /* \_SB_.PCI0.I2C2.CAM0.PLDB */
                }

                Method (_STA, 0, NotSerialized)  // _STA: Status
                {
//[-start-170518-IB07400867-add]//
                    If (LEqual(IOTP(),0x00)) { // CCG CRB
                        Return (Zero)
                    }
//[-end-170518-IB07400867-add]//
                    If (LEqual (UCAS, One)){
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
                        /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,  /* ........ */
                        /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0030 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0038 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0040 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
                        /* 0050 */  0x08, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0xF8,  /* ........ */
                        /* 0058 */  0x24, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
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
                            AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                            0x00, ResourceConsumer, ,
                            )
                    })
                    Return (SBUF) /* \_SB_.PCI0.I2C0.CAM0._CRS.SBUF */
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
                         Return (0x02003600)
                         }
                     }
             Return (Zero)
           }
	    
       }   
//[-start-170518-IB07400867-remove]//
//#endif
//[-end-170518-IB07400867-remove]//
//[-end-170516-IB08450375-add]//
//[-start-180316-IB07400953-add]//
#if 0 // example code to support DPHY1.1 & DPHY1.2 switch
  Device (PMC7)
  {
    Name (_ADR, Zero)  // _ADR: Address
    Name (_HID, "INT3472")  // _HID: Hardware ID
    Name (_CID, "INT3472")  // _CID: Compatible ID
    Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
    Name (_UID, "7")  // _UID: Unique ID
    Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
    {
        Name (SBUF, ResourceTemplate ()
        {
            GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,      IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                )
                {   // Pin list GPIO_64 (GP_CAMERASB02)
                    0x0034
                }
            GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,   IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                )
                {   // Pin list GPIO_71 (GP_CAMERASB09)
                    0x003B
                }
        })
        Return (SBUF) /* \_SB_.PCI0.I2C2.PMC3._CRS.SBUF */
    }

    Method (_STA, 0, NotSerialized)  // _STA: Status
    {
       If (LEqual(IOTP(),0x00)) { // CCG CRB
            Return (Zero)
       }
       If (LEqual (UCAS, 0x50)){ // OV8856
            Return (0x0F)
       } else {
            Return (Zero)
       }
    }

    Method (CLDB, 0, Serialized)
    {
        Name (PAR, Buffer (0x20)
        {
            /* 0000 */  0x00, 0x01, 0x07, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
            /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
        })
        Return (PAR) /* \_SB_.PCI0.I2C2.PMC3.CLDB.PAR_ */
    }

    Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
    {
        If (LEqual (Arg0, ToUUID("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
        {
            If (LEqual (Arg2, One)){
                Return (0x02)
            }

            If (LEqual (Arg2, 0x02)){
                Return (0x01004000)		// RESET  MCSI1_XSHUTDN = CAMERASB02 = GPIO_64
            }

            If (LEqual (Arg2, 0x03)) { // Power Down GPIO_71
                Return (0x01004701)
            }
        }

        Return (Zero)
    }
  }
  Device (CAM7)
  {
    Name (_ADR, Zero)  // _ADR: Address
    Name (_HID, "OVTI8856")  // _HID: Hardware ID
    Name (_CID, "OVTI8856")  // _CID: Compatible ID
    Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
    Name (_DDN, "OV8856")  // _DDN: DOS Device Name
    Name (_UID, "7")  // _UID: Unique ID
    Name (_DEP, Package (0x01)  // _DEP: Dependencies
    {
        \_SB.PCI0.I2C2.PMC7
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
        Return (PLDB) /* \_SB_.PCI0.I2C2.CAM7.PLDB */
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
        If (LEqual (UCAS, 0x50)) // OV8856
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
            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,  /* ........ */
            /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0030 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0038 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0040 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02,  /* ........ */
            /* 0050 */  0x08, 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0xF8,  /* ........ */
            /* 0058 */  0x24, 0x01, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
            /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0068 */  0x00, 0x00, 0x00, 0x00                           /* .... */
        })
        Return (PAR) /* \_SB_.PCI0.I2C2.CAM7.SSDB.PAR_ */
    }

    Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
    {
        Name (SBUF, ResourceTemplate ()
        {
            I2cSerialBus (0x0010, ControllerInitiated, 0x00061A80,
                AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                0x00, ResourceConsumer, ,
                )
            I2cSerialBus (0x000C, ControllerInitiated, 0x00061A80,
                AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                0x00, ResourceConsumer, ,
                )
            I2cSerialBus (0x0050, ControllerInitiated, 0x00061A80,
                AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                0x00, ResourceConsumer, ,
                )
            I2cSerialBus (0x0051, ControllerInitiated, 0x00061A80,
                AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                0x00, ResourceConsumer, ,
                )
            I2cSerialBus (0x0052, ControllerInitiated, 0x00061A80,
                AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                0x00, ResourceConsumer, ,
                )
            I2cSerialBus (0x0053, ControllerInitiated, 0x00061A80,
                AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                0x00, ResourceConsumer, ,
                )
        })
        Return (SBUF) /* \_SB_.PCI0.I2C2.CAM7._CRS.SBUF */
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
              Return (0x02001000)
          }

          If (LEqual (Arg2, 0x03))
          {
              Return (0x02000C01)
          }
          
          If (LEqual (Arg2, 0x04))
          {
              Return (0x02005002)
          }
          
          If (LEqual (Arg2, 0x05))
          {
              Return (0x02005103)
          }
          
          If (LEqual (Arg2, 0x06))
          {
              Return (0x02005204)
          }
          
          If (LEqual (Arg2, 0x07))
          {
              Return (0x02005305)
          }
      }
      Return (Zero)
    }
  }
  //
  // OV13858
  //
  Device (PMC9)
  {
    Name (_ADR, Zero)  // _ADR: Address
    Name (_HID, "INT3472")  // _HID: Hardware ID
    Name (_CID, "INT3472")  // _CID: Compatible ID
    Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
    Name (_UID, "9")  // _UID: Unique ID
    Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
    {
        Name (SBUF, ResourceTemplate ()
        {
            GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,      IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                )
                {   // Pin list GPIO_64 (GP_CAMERASB02)
                    0x0034
                }
            GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,   IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                )
                {   // Pin list GPIO_71 (GP_CAMERASB09)
                    0x003B
                }
        })
        Return (SBUF) /* \_SB_.PCI0.I2C2.PMC9._CRS.SBUF */
    }

    Method (_STA, 0, NotSerialized)  // _STA: Status
    {
       If (LEqual(IOTP(),0x00)) { // CCG CRB
            Return (Zero)
       }
       If (LEqual (UCAS, 0x51)){ // OV13858
            Return (0x0F)
       } else {
            Return (Zero)
       }
    }

    Method (CLDB, 0, Serialized)
    {
        Name (PAR, Buffer (0x20)
        {
            /* 0000 */  0x00, 0x01, 0x09, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
            /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
        })
        Return (PAR) /* \_SB_.PCI0.I2C2.PMC9.CLDB.PAR_ */
    }

    Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
    {
        If (LEqual (Arg0, ToUUID("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
        {
            If (LEqual (Arg2, One)){
                Return (0x02)
            }

            If (LEqual (Arg2, 0x02)){
                Return (0x01004000)		// RESET  MCSI1_XSHUTDN = CAMERASB02 = GPIO_64
            }

            If (LEqual (Arg2, 0x03)) { // Power Down GPIO_71
                Return (0x01004701)
            }
        }

        Return (Zero)
    }
  }
  Device (CAM9)
  {
      Name (_ADR, Zero)  // _ADR: Address
      Name (_HID, "OVTID858")  // _HID: Hardware ID
      Name (_CID, "OVTID858")  // _CID: Compatible ID
      Name (_SUB, "INTL0000")  // _SUB: Subsystem ID
      Name (_DDN, "OV13858")  // _DDN: DOS Device Name
      Name (_UID, "9")  // _UID: Unique ID
      Name (_DEP, Package (0x01)  // _DEP: Dependencies
      {
          \_SB.PCI0.I2C2.PMC9
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
          CreateField (DerefOf (Index (PLDB, Zero)), 0x73, 0x04, RPOS)
          Store(CROT, RPOS)
          Return (PLDB) /* \_SB_.PCI0.I2C2.CAM9.PLDB */
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
          If (LEqual (UCAS, 0x51)) // OV13858
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
              /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,  /* ........ */
              /* 0020 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
              /* 0028 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
              /* 0030 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
              /* 0038 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
              /* 0040 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
              /* 0048 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x02,  /* ........ */
              /* 0050 */  0x08, 0x00, 0x03, 0x00, 0x00, 0x01, 0x00, 0xF8,  /* ........ */
              /* 0058 */  0x24, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,  /* $....... */
              /* 0060 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
              /* 0068 */  0x00, 0x00, 0x00, 0x00                           /* .... */
          })
          Return (PAR) /* \_SB_.PCI0.I2C2.CAM9.SSDB.PAR_ */
      }

      Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
      {
          Name (SBUF, ResourceTemplate ()
          {
              I2cSerialBus (0x0010, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                  0x00, ResourceConsumer, ,
                  )
              I2cSerialBus (0x000C, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                  0x00, ResourceConsumer, ,
                  )
              I2cSerialBus (0x0050, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                  0x00, ResourceConsumer, ,
                  )
              I2cSerialBus (0x0051, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                  0x00, ResourceConsumer, ,
                  )
              I2cSerialBus (0x0052, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                  0x00, ResourceConsumer, ,
                  )
              I2cSerialBus (0x0053, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C2",
                  0x00, ResourceConsumer, ,
                  )
          })
          Return (SBUF) /* \_SB_.PCI0.I2C2.CAM9._CRS.SBUF */
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
                  Return (0x02001000)
              }

              If (LEqual (Arg2, 0x03))
              {
                  Return (0x02000C01)
              }
              
              If (LEqual (Arg2, 0x04))
              {
                  Return (0x02005002)
              }
              
              If (LEqual (Arg2, 0x05))
              {
                  Return (0x02005103)
              }
              
              If (LEqual (Arg2, 0x06))
              {
                  Return (0x02005204)
              }
              
              If (LEqual (Arg2, 0x07))
              {
                  Return (0x02005305)
              }
         }

         Return (Zero)
      }
  }
  //
  // OV5648
  //
  Device (PMCA)
  {
    Name (_ADR, Zero)  // _ADR: Address
    Name (_HID, "INT3472")  // _HID: Hardware ID
    Name (_CID, "INT3472")  // _CID: Compatible ID
    Name (_DDN, "INCL-CRDD")  // _DDN: DOS Device Name
    Name (_UID, "A")  // _UID: Unique ID
    Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
    {
        Name (SBUF, ResourceTemplate ()
        {
            GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,      IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                )
                {   // Pin list GPIO_64 (GP_CAMERASB02)
                    0x0034
                }
            GpioIo (Exclusive, PullDefault, 0x0000, 0x0000,   IoRestrictionOutputOnly,
                "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                )
                {   // Pin list GPIO_71 (GP_CAMERASB09)
                    0x003B
                }
        })
        Return (SBUF) /* \_SB_.PCI0.I2C2.PMC9._CRS.SBUF */
    }

    Method (_STA, 0, NotSerialized)  // _STA: Status
    {
       If (LEqual(IOTP(),0x00)) { // CCG CRB
            Return (Zero)
       }
       If (LEqual (UCAS, 0x52)){ // OV5648
            Return (0x0F)
       } else {
            Return (Zero)
       }
    }

    Method (CLDB, 0, Serialized)
    {
        Name (PAR, Buffer (0x20)
        {
            /* 0000 */  0x00, 0x01, 0x0A, 0x70, 0x00, 0x00, 0x00, 0x00,  /* ...p.... */
            /* 0008 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0010 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* ........ */
            /* 0018 */  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00   /* ........ */
        })
        Return (PAR) /* \_SB_.PCI0.I2C2.PMC9.CLDB.PAR_ */
    }

    Method (_DSM, 4, NotSerialized)  // _DSM: Device-Specific Method
    {
        If (LEqual (Arg0, ToUUID("79234640-9e10-4fea-a5c1-b5aa8b19756f")))
        {
            If (LEqual (Arg2, One)){
                Return (0x02)
            }

            If (LEqual (Arg2, 0x02)){
                Return (0x01004000)		// RESET  MCSI1_XSHUTDN = CAMERASB02 = GPIO_64
            }

            If (LEqual (Arg2, 0x03)) { // Power Down GPIO_71
                Return (0x01004701)
            }
        }

        Return (Zero)
    }
  }
  Device(CAMA)
  {
    Name(_ADR,Zero)//_ADR:Address
    Name(_HID,"OVTI5648")//_HID:HardwareID
    Name(_CID,"OVTI5648")//_CID:CompatibleID
    Name(_SUB,"INTL0000")//_SUB:SubsystemID
    Name(_DDN,"OV5648")//_DDN:DOSDeviceName
    Name(_UID,"A")//_UID:UniqueID
    Name(_DEP,Package(0x01)//_DEP:Dependencies
    {
      \_SB.PCI0.I2C2.PMCA
    })
    Name(PLDB,Package(0x01)
    {
      Buffer(0x14)
      {
      /*0000*/0x82,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
      /*0008*/0x61,0x0C,0x00,0x00,0x03,0x00,0x00,0x00,/*i.......*/
      /*0010*/0xFF,0xFF,0xFF,0xFF/*....*/
      }
    })
    Method(_PLD,0,Serialized)//_PLD:PhysicalLocationofDevice
    {
      CreateField(DerefOf(Index(PLDB,Zero)),0x73,0x04,RPOS)
      Store(CROT, RPOS)
      Return(PLDB)/*\_SB_.PCI0.I2C0.CAM3.PLDB*/
    }
  
    Method(_STA,0,NotSerialized)//_STA:Status
    {
       If (LEqual(IOTP(),0x00)) { // CCG CRB
            Return (Zero)
       }
       If (LEqual (UCAS, 0x52)){ // OV5648
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
        /*0018*/0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,/*........*/
        /*0020*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
        /*0028*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
        /*0030*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
        /*0038*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
        /*0040*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
        /*0048*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,/*........*/
        /*0050*/0x08,0x00,0x02,0x00,0x00,0x01,0x00,0xF8,/*........*/
        /*0058*/0x24,0x01,0x0A,0x00,0x00,0x00,0x00,0x00,/*$.......*/
        /*0060*/0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*........*/
        /*0068*/0x00,0x00,0x00,0x00/*....*/
      })
      Return(PAR)/*\_SB_.PCI0.I2C2.CAMA.SSDB.PAR_*/
    }
  
    Method(_CRS,0,Serialized)//_CRS:CurrentResourceSettings
    {
      Name(SBUF,ResourceTemplate()
      {
        I2cSerialBus(0x0036,ControllerInitiated,0x00061A80,
        AddressingMode7Bit,"\\_SB.PCI0.I2C2",
        0x00,ResourceConsumer,,
        )
        I2cSerialBus(0x000C,ControllerInitiated,0x00061A80,
        AddressingMode7Bit,"\\_SB.PCI0.I2C2",
        0x00,ResourceConsumer,,
        )
      })
      Return(SBUF)/*\_SB_.PCI0.I2C2.CAMA._CRS.SBUF*/
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
          Return (0x02003600)    //  SENSOR
        }
        If (LEqual (Arg2, 0x03)) /* I2C 0:bit31-24:BUS. 23-16:Speed.15-8:Addr. 0-7:Function*/
        {
          Return (0x02000C01)    // VCM
        }
      }
      Return(Zero)
    }
  }
#endif  
//[-end-180316-IB07400953-add]//
} //  Scope(\_SB.PCI0.I2C2)
