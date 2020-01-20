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

External(\PSSI, IntObj)

Scope(\_SB.PCI0.I2C7){
//------------------------
// PSS on I2C7
//------------------------
/* 
PSS: Monza_X-2K_Dura
* GPIO: None
* I2C bus: I2C 7 (0 based)
* I2C slave address: 0x6E
* I2C frequency: 400K
* ACPI ID: IMPJ0003
*/
  Device (IMP3)
  {
    Name (_ADR, Zero)               // _ADR: Address
    Name (_HID, "IMPJ0003")         // _HID: Hardware ID
    Name (_CID, "IMPJ0003")         // _CID: Compatible ID
    Name (_UID, One)                // _UID: Unique ID

    Method(_STA, 0x0, NotSerialized) {                            
      If (LEqual (PSSI, 1)) {
        Return (0xF)
      }
      Return (0)
    }

    Method (_CRS, 0, Serialized) {
      Name (SBUF, ResourceTemplate () {
        I2cSerialBus (0x6E,ControllerInitiated,400000,AddressingMode7Bit,"\\_SB.PCI0.I2C7",0x00,ResourceConsumer,,)
      })
      Return (SBUF)
    }

  } // Device (IMP3)
}  //Scope(\_SB.PCI0.I2C7)