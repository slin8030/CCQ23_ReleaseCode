/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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


scope (\_SB) {

  Device(RTC)  // RTC
  {
    Name(_HID,EISAID("PNP0B00"))

//[-start-160829-IB07400775-modify]//
//    Name(_CRS,ResourceTemplate()
//    {
//      IO(Decode16,0x70,0x70,0x01,0x08)
//#if BXTI_PF_ENABLE
//      IRQNoFlags() {8}
//#endif
//    })
    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        IO(Decode16,0x70,0x70,0x01,0x08)
      })
      Name (RBU2, ResourceTemplate ()
      {
        IO(Decode16,0x70,0x70,0x01,0x08)
        IRQNoFlags() {8}
      })
      if (LEqual(IOTP(),0x01)) {
        Return (RBU2)
      }
      Return (RBUF)
    }
//[-end-160829-IB07400775-modify]//
  }

  Device(HPET)  // High Performance Event Timer
  {
    Name (_HID, EisaId ("PNP0103"))
    Name (_UID, 0x00)
    Method (_STA, 0, NotSerialized)
    {
      Return (0x0F)
    }

//[-start-160828-IB07400775-modify]//
    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite,
                       0xFED00000,  // Address Base
                       0x00000400,  // Address Length
                       )
//#if !BXTI_PF_ENABLE
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, , , )
        {
          0x00000008,   //0xB HPET-2
        }
//#endif
     })
     Name (RBU2, ResourceTemplate ()
     {
        Memory32Fixed (ReadWrite,
                       0xFED00000,  // Address Base
                       0x00000400,  // Address Length
                       )
     })
     if (LEqual(IOTP(),0x01)) {
       Return (RBU2)
     } else {
       Return (RBUF)
     }
   }
//[-end-160828-IB07400775-modify]//
 }
}

Device(IPIC) // 8259 PIC
{
  Name(_HID,EISAID("PNP0000"))

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x20,0x20,0x01,0x02)
    IO(Decode16,0x24,0x24,0x01,0x02)
    IO(Decode16,0x28,0x28,0x01,0x02)
    IO(Decode16,0x2C,0x2C,0x01,0x02)
    IO(Decode16,0x30,0x30,0x01,0x02)
    IO(Decode16,0x34,0x34,0x01,0x02)
    IO(Decode16,0x38,0x38,0x01,0x02)
    IO(Decode16,0x3C,0x3C,0x01,0x02)
    IO(Decode16,0xA0,0xA0,0x01,0x02)
    IO(Decode16,0xA4,0xA4,0x01,0x02)
    IO(Decode16,0xA8,0xA8,0x01,0x02)
    IO(Decode16,0xAC,0xAC,0x01,0x02)
    IO(Decode16,0xB0,0xB0,0x01,0x02)
    IO(Decode16,0xB4,0xB4,0x01,0x02)
    IO(Decode16,0xB8,0xB8,0x01,0x02)
    IO(Decode16,0xBC,0xBC,0x01,0x02)
    IO(Decode16,0x4D0,0x4D0,0x01,0x02)
    IRQNoFlags() {2}
  })
}

Device(LDRC) // LPC Device Resource Consumption
{
  Name(_HID,EISAID("PNP0C02"))

  Name(_UID,2)

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x2E,0x2E,0x1,0x02)      // WPCN381U SIO Config Index + Data.
    IO(Decode16,0x4E,0x4E,0x1,0x02)      // LPC Slot Access.
    IO(Decode16,0x61,0x61,0x1,0x1)       // NMI Status.
    IO(Decode16,0x63,0x63,0x1,0x1)       // Processor I/F.
    IO(Decode16,0x65,0x65,0x1,0x1)       // Processor I/F.
    IO(Decode16,0x67,0x67,0x1,0x1)       // Processor I/F.
    IO(Decode16,0x70,0x70,0x1,0x1)       // NMI Enable.
    IO(Decode16,0x80,0x80,0x1,0x10)      // Postcode.
    IO(Decode16,0x92,0x92,0x1,0x1)       // Processor I/F.
    IO(Decode16,0xB2,0xB2,0x01,0x02)     // Software SMI.
    IO(Decode16,0x680,0x680,0x1,0x20)    // 32 Byte I/O.
    IO(Decode16,0x400,0x400,0x1,0x80)    // ACPI Base.
    IO(Decode16,0x500,0x500,0x1,0xFF)    // GPIO Base.
//[-start-160906-IB07400779-remove]//
//    IO(Decode16,0x600,0x600,0x1,0x20)    // WPCN381U SIO SWC + ACPI Base.
//    IO(Decode16,0x164e,0x164e,0x1,0x02)  // WPCN381U SIO Config Index1 + Data1.
//[-end-160906-IB07400779-remove]//
  })
}
Device(TIMR) // 8254 Timer
{
  Name(_HID,EISAID("PNP0100"))

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x40,0x40,0x01,0x04)
    IO(Decode16,0x50,0x50,0x10,0x04)
    IRQNoFlags() {0}
  })
}
