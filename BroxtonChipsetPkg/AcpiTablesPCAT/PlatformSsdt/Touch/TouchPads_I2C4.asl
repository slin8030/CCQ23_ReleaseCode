/** @file

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

@par Specification Reference:
**/

External(\TCPD, IntObj)

Scope(\_SB.PCI0.I2C4) {
//------------------------
// Touch Pads on I2C4
// Note: instead of adding more touch devices, parametrize this one with appropriate _HID value and GPIO numbers
// GPIO_18:TCHPAD_INT_N     North West Community, IRQ number 0x75.
//------------------------

  Device(TPD0)
  {
    Name(_ADR, One)
    Name(_HID, "ALPS0001")
    Name(_CID, "PNP0C50")
    Name(_UID, One)
    Name(_S0W, 4) // required to put the device to D3 Cold during S0 idle
    Name (SBFB, ResourceTemplate () {
      I2cSerialBus (
        0x2C,
        ControllerInitiated,
        400000,
        AddressingMode7Bit,
        "\\_SB.PCI0.I2C4",
        0x00,
        ResourceConsumer,
        ,
        )
    })
    Name (SBFG, ResourceTemplate () {
      GpioInt (Level, ActiveLow, ExclusiveAndWake, PullDefault, 0x0000, "\\_SB.GPO0", 0x00, ResourceConsumer, ,)
        {   // Pin list
          0x0012
        }
    })
    Name (SBFI, ResourceTemplate () {
      Interrupt(ResourceConsumer, Level, ActiveLow, ExclusiveAndWake,,,)
      {
        0x75,
      }
    })
    Method (_INI, 0, NotSerialized)
    {
    }
    Method (_STA, 0, NotSerialized)
    {
      If (LEqual (TCPD, 1)) {
        Return (0x0F)
      }
      Return (0x00)
    }
    Method (_CRS, 0, NotSerialized)
    {
      If (LLess (OSYS, 2012))
      {
        Return (SBFI)
      }
      Return (ConcatenateResTemplate (SBFB, SBFG))
    }
    Method(_DSM, 0x4, NotSerialized)
    {
      // DSM UUID for HIDI2C. Do Not change.
      If(LEqual(Arg0, ToUUID("3CDFF6F7-4267-4555-AD05-B30A3D8938DE")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 1
          If(LEqual(Arg1, One))
          {
            Return (Buffer (One) {0x03})
          }
          Else
          {
            Return (Buffer (One) {0x00})
          }
        } ElseIf (LEqual(Arg2, One)) {  // Function 1 : HID Function
          // HID Descriptor Address (IHV Specific)
          Return(0x0020)
        } Else {
          Return (Buffer (One) {0x00})
        }
      }
      Else
      {
        If(LEqual(Arg0, ToUUID("EF87EB82-F951-46DA-84EC-14871AC6F84B")))
        {
          If (LEqual (Arg2, Zero))
          {
            If (LEqual (Arg1, One))
            {
              Return (Buffer (One) {0x03})
            }
          }

          If (LEqual (Arg2, One))
          {
            Return (ConcatenateResTemplate (SBFB, SBFG))
          }

          Return (Buffer (One) {0x00})
        }
        Else
        {
          Return (Buffer (One) {0x00})
        }
      }
    }
  }
}  //Scope(\_SB.PCI0.I2C4)