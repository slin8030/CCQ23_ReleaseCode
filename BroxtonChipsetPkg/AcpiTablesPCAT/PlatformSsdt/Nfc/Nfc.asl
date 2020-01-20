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

/*
NXP NPC100 non-secure NFC
* I2C 1(0 based)
* Slave address: 0x29
* IRQ          : GPIO_20, direct IRQ, level high
* Reset        : GPIO_150, output
* Fw update    : GPIO_27, output
*/
External(\NFCN, IntObj)

Scope (\_SB.PCI0.I2C1)
{
  Device (NFC1)
  {
      Name (_ADR, Zero)                // _ADR: Address
      Name (_HID, EISAID("NXP1001"))   // _HID: Hardware ID
      Name (_DDN, "NXP NPC100")        // _DDN: DOS Device Name
      Name (_UID, One)                 // _UID: Unique ID

      Method (_CRS, 0, Serialized)  // _CRS: Current Resource Settings
      {
          Name (SBUF, ResourceTemplate ()
          {
              I2cSerialBus (0x29, ControllerInitiated, 0x00061A80,
                  AddressingMode7Bit, "\\_SB.PCI0.I2C1",
                  0x00, ResourceConsumer, ,
                  )
              Interrupt (ResourceConsumer, Edge, ActiveHigh, Exclusive, ,, )
                  {
                      0x00000077, // GPIO_20
                  }
              GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                  "\\_SB.GPO0", 0x00, ResourceConsumer, ,
                  )
                  {   // Pin list, GPIO_27
                      0x001B
                  }
              GpioIo (Exclusive, PullDefault, 0x0000, 0x0000, IoRestrictionOutputOnly,
                  "\\_SB.GPO2", 0x00, ResourceConsumer, ,
                  )
                  {   // Pin list, GPIO_150
                      0x0014
                  }
          })
          Return (SBUF)
      }

      Method (_STA, 0, NotSerialized)  // _STA: Status
      {
        If (LEqual (NFCN, 1)) {
          Return (0xF)
        }
        Return (0)
      }

  }
}