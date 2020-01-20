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

Scope(\_SB.PCI0.SDIO)
{
  Device (BRC2)
  {
    Name (_ADR, 0x02)

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

    Method (_CRS, 0, Serialized)
    {
      Name (NAM, Buffer ()
      { // 20
        "\\_SB.PCI0.SDIO.BRCM"
      })
      Name (SPB, Buffer ()
      {
        /* 0000 */   0x8E, 0x1D, 0x00, 0x01, 0x00, 0xC0, 0x02, 0x00,
        /* 0008 */   0x00, 0x01, 0x00, 0x00
      })
      Name(END, Buffer() {0x79, 0x00})

      Concatenate (SPB, NAM, Local0)
      Concatenate (Local0, END, Local1)
      Return (Local1)
    }

  }
}
