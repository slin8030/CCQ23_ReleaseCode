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
 GPIO_10 for Wi-Fi direct IRQ 0x6D.
 GPIO_15 for Wi-Fi reset
 PMIC_STDBY for Wi-Fi disable, NW index 30
*/
External(ADBG, MethodObj)

Scope(\_SB.PCI0.SDIO)
{
  Device (BRCM)
  {
    Name (_ADR, One)  // _ADR: Address
    Name (_DEP, Package() {\_SB.GPO0})
    Name (_S4W, 2)
    Name (_S0W, 2)

    Method (_STA, 0, NotSerialized)
    {
//[-start-161201-IB07400821-add]//
      If(LEqual(HLPS, 0))
      {
        Return(0x0)
      }
//[-end-161201-IB07400821-add]//
      Return (0xF)
    }

    Method (_RMV, 0, NotSerialized)
    {
      Return (Zero)
    }

    Name (_PRW, Package (0x02)
    {
      Zero,
      Zero
    })

    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Interrupt (ResourceConsumer, Edge, ActiveHigh, ExclusiveAndWake, , , ) {0x6D}         // GPIO_10 to IOAPIC IRQ 0x6D
      })
      Return (RBUF)
    }

    Method (_PS3, 0, NotSerialized)
    {
       ADBG("BRCM:PS3")
       Store( 0x00, \_SB.GPO0.CWLE )        // Put WiFi chip in Reset 
       Sleep(150)
    }

    Method (_PS0, 0, NotSerialized)
    {
      ADBG("BRCM:PS0")
      Store( 0x01, \_SB.GPO0.CWLE )      // Take WiFi chip out in Reset
      Sleep(150)
    }

  }
}