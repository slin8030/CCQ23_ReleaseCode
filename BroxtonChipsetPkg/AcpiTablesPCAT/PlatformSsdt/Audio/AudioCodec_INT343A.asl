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

External(\IOBF, IntObj) // ScHdAudioIoBufferOwnership
External(\IS3A, IntObj)

Scope(\_SB.PCI0.I2C0) {
  //-----------------------------------
  //  HD Audio I2S Codec device
  //  ALC 298
  //  I2C1
  //  GPIO_192:SOC_CODEC_PD_N    North West Community number 5
  //  GPIO_116:SOC_CODEC_IRQ     North West Community, direct IRQ, IRQ number 0x5B.
  //-----------------------------------
  Device (HDAC)
  {
    Name (_HID, "INT343A")
    Name (_CID, "INT343A")
    Name (_DDN, "Intel(R) Smart Sound Technology Audio Codec")
    Name (_UID, 1)
    Name (CADR, 0) // Codec I2C address
    Name (DBIT, Zero) // Disable BIT

    Method(_INI) {
    }

    Method (_CRS, 0, Serialized) {
      Name (SBFB, ResourceTemplate () {
        I2cSerialBus (0x1C, ControllerInitiated, 400000, AddressingMode7Bit, "\\_SB.PCI0.I2C0",,,)
      })

      Name (SBFI, ResourceTemplate () {
        Interrupt (ResourceConsumer, Level, ActiveLow, ExclusiveAndWake,,,) {0x5B}
      })

      Return (ConcatenateResTemplate(SBFB, SBFI))
    }

    Method(_SRS, 1, Serialized)
    {
      Store(One, DBIT)
    }

    Method(_DIS, 0, NotSerialized)
    {
      Store(Zero, DBIT)
    }

    Method (_STA, 0, NotSerialized)
    {
      If (LAnd (LEqual (IOBF, 3), LEqual(IS3A, 1))) {
        If(LEqual(DBIT, Zero))
        {
          Return(0xD)
        }
        Return (0xF)  // I2S Codec Enabled
      }
      Return (0)
    }
  }  // Device (HDAC)
}