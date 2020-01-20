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

External(\ISC1, IntObj)

Scope(\_SB.PCI0.I2C1)
{
    Device (ACD0)  // Audio Codec driver I2C
    {
      Name (_ADR, 0x38)
      Name (_HID, "INT34C1")
      Name (_CID, "INT34C1")
      Name (_DDN, "Intel(R) Smart Sound Technology Audio Codec" )
      Name (_UID, 1)

      Method(_CRS, 0x0, Serialized)
      {
        Name(SBUF,ResourceTemplate ()
        {
          I2CSerialBus(
            0x38,               // SlaveAddress: bus address
            ,                   // SlaveMode: default to ControllerInitiated
            400000,             // ConnectionSpeed: in Hz
            ,                   // Addressing Mode: default to 7 bit
            "\\_SB.PCI0.I2C1",  // ResourceSource: I2C bus controller name
            ,                   // Descriptor Name: creates name for offset of resource descriptor
          )  // VendorData
          GpioInt(Level, ActiveLow, ExclusiveAndWake, PullUp, 0,"\\_SB.GPO1") {69} //  SOC_CODEC_IRQ
        })
        Return (SBUF)
      }

      Method (_STA, 0x0, NotSerialized)
      {
        If (LEqual (ISC1, 1)) {
          Return (0xF)  // I2S Codec Enabled
        }
        Return (0)
      }

      Method (_DIS, 0x0, NotSerialized)
      {
      }

    } // Device (ACD0)
}