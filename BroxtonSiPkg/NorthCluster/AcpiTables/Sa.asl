/**@file

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
External(\_SB.PCI0, DeviceObj)


#if (TABLET_PF_ENABLE == 1)
External(BREV, IntObj)
External(BDID, IntObj)


External(\_SB.PCI0.PEPD, DeviceObj)
#else
External(PNSL)
External(\_SB.PEPD, DeviceObj)
#endif

Scope (\_SB.PCI0) {

  Device(GFX0) { // Mobile I.G.D
    Name(_ADR, 0x00020000)
#if (ENBDT_PF_ENABLE == 1)
    Name (_S0W, 3)

    Method (_DEP, 0, NotSerialized) {
      Return (Package() {\_SB.PEPD})
    }


    Name (CBUF, ResourceTemplate() {
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {37}  //  MIPI_DSI_RST_1_8V, GPIO_77 (NorthWest, 37)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {9}   //  PANEL1_VDDEN_1_8V_R, Used as GPIO_196 for MIPI (NorthWest, 6)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {10}  //  PANEL1_BKLTEN_1_8V_R, Used as GPIO_197 for MIPI (NorthWest, 7)
    })

    Name (DBUF, ResourceTemplate() {})//ed

    Method (_CRS, 0, Serialized)      // _CRS: Current Resource Settings
    {
      If (LAnd (LNotEqual (PNSL, 0), LNotEqual (PNSL, 4))) {
        Return (CBUF)
      } Else {
        Return (DBUF)
      }
    }
#else

    Name (_S0W, 3)

    Method (_DEP, 0, NotSerialized) {
      Return (Package() {\_SB.PCI0.PEPD})
    }


    Name (CBUF, ResourceTemplate() {
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {27}  // N27: DISP0_RST_N, GPIO_27 (North, 27)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {6}   //  DISP0_VDDEN, Used as GPIO_193 for MIPI (South, 6)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {7}   //  DISP0_BKLTEN, Used as GPIO_194 for MIPI (South, 7)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {9}   //  DISP1_VDDEN, Used as GPIO_196 for MIPI (South, 9)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {10}  //  DISP1_BKLTEN, Used as GPIO_197 for MIPI (South, 10)
      // DISP0 I2C bus for Display backlight DC/DC LM3631 (0x29, 400k) on I2C BUS6 (1 based) 
      I2CSerialBus(0x29,                 //SlaveAddress: bus address
                   ,                       //SlaveMode: default to ControllerInitiated
                   400000,                 //ConnectionSpeed: in Hz
                   ,                       //Addressing Mode: default to 7 bit
                   "\\_SB.PCI0.I2C6",      //ResourceSource: I2C bus controller name 
                   ,                       //Descriptor Name: creates name for offset of resource descriptor
                  )                       //VendorData
      // DISP1 I2C bus for Display backlight DC/DC LM3631 (0x29, 400k) on I2C BUS4 (1 based) for FAB B/C
      I2CSerialBus(0x29,                 //SlaveAddress: bus address
                   ,                       //SlaveMode: default to ControllerInitiated
                   400000,                 //ConnectionSpeed: in Hz
                   ,                       //Addressing Mode: default to 7 bit
                   "\\_SB.PCI0.I2C4",           //ResourceSource: I2C bus controller name
                   ,                       //Descriptor Name: creates name for offset of resource descriptor
                  )
    })

    Name (DBUF, ResourceTemplate() {
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0", ) {27}  // N27: DISP0_RST_N, GPIO_27 (North, 27)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {6}   //  DISP0_VDDEN, Used as GPIO_193 for MIPI (South, 6)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {7}   //  DISP0_BKLTEN, Used as GPIO_194 for MIPI (South, 7)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {9}   //  DISP1_VDDEN, Used as GPIO_196 for MIPI (South, 9)
      GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO4", ) {10}  //  DISP1_BKLTEN, Used as GPIO_197 for MIPI (South, 10)
      // DISP0 I2C bus for Display backlight DC/DC LM3631 (0x29, 400k) on I2C BUS6 (1 based) 
      I2CSerialBus(0x29,                 //SlaveAddress: bus address
                   ,                       //SlaveMode: default to ControllerInitiated
                   400000,                 //ConnectionSpeed: in Hz
                   ,                       //Addressing Mode: default to 7 bit
                   "\\_SB.PCI0.I2C6",      //ResourceSource: I2C bus controller name 
                   ,                       //Descriptor Name: creates name for offset of resource descriptor
                  )                       //VendorData
      // DISP1 I2C bus for Display backlight DC/DC LM3631 (0x29, 400k) on I2C BUS5 (1 based) for FAB D
      I2CSerialBus(0x29,                 //SlaveAddress: bus address
                   ,                       //SlaveMode: default to ControllerInitiated
                   400000,                 //ConnectionSpeed: in Hz
                   ,                       //Addressing Mode: default to 7 bit
                   "\\_SB.PCI0.I2C5",           //ResourceSource: I2C bus controller name
                   ,                       //Descriptor Name: creates name for offset of resource descriptor
                  )                       //VendorData
    })

    Method (_CRS, 0, NotSerialized)      // _CRS: Current Resource Settings
    {
      If (LAnd(LEqual(BDID, 0x00), LLess(BREV, 0x4)))
      {
        Return(CBUF)  // This resource buffer is only exposed for RVP A/B/C
      }
      Return (DBUF) //Not FAB D
    }
#endif
    include("Igfx.asl")
  } // end "IGD Device"

  include("Ipu.asl")
}
