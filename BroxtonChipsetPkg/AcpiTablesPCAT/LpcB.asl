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



// LPC Bridge - Device 31, Function 0
// Define the needed LPC registers used by ASL.

scope(\_SB) {
  OperationRegion(ILBR, SystemMemory, \IBAS, 0x8C)
  Field(ILBR, AnyAcc, NoLock, Preserve) {
    Offset(0x08), // 0x08
    PARC,   8,
    PBRC,   8,
    PCRC,   8,
    PDRC,   8,
    PERC,   8,
    PFRC,   8,
    PGRC,   8,
    PHRC,   8,
    Offset(0x88), // 0x88
    ,       4,
    UI4E,   1
  }

  Include ("98_LINK.ASL")
}
// LPC Bridge - Device 31, Function 0
scope (\_SB.PCI0.LPCB) {
  OperationRegion(LPC0, PCI_Config, 0x40, 0xC0)
  Field(LPC0, AnyAcc, NoLock, Preserve)
  {
    Offset(0x040), // 0x80
    C1EN,   1, // COM1 Enable
    ,      31
  }

//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
  Include ("EC.ASL")
#endif
//[-end-161206-IB07400822-modify]//
  Include ("LPC_DEV.ASL")

  // Define the KBC_COMMAND_REG-64, KBC_DATA_REG-60 Registers as an ACPI Operating
  // Region.  These registers will be used to skip kbd mouse
  // resource settings if not present.

//[-start-160923-IB07400789-remove]//
//  OperationRegion(PKBS, SystemIO, 0x60, 0x05)
//  Field(PKBS, ByteAcc, Lock, Preserve)
//  {
//    PKBD, 8,
//    ,     8,
//    ,     8,
//    ,     8,
//    PKBC, 8
//  }
//[-end-160923-IB07400789-remove]//

Device(PS2K)  // PS2 Keyboard
{
  Name(_HID,"MSFT0001")
  Name(_CID,EISAID("PNP0303"))

//[-start-160929-IB07400790-modify]//
  Method(_STA,0,Serialized)
  {
//    //
//    // If KBC I/O port is not decoded in SuperIo driver, it will read back as 0xFF.
//    //
//    If(And(LEqual(PKBD, 0xFF), LEqual(PKBC, 0xFF)))
//    {
//      Return (0x0000)
//    }
//
//    If (LGreater (P2MK, 0)) //Ps2 Keyboard and Mouse Enable
//    {
//      Return (0x000F)
//    }
    
    // Only report resources to the OS if the Keyboard is present
    If (LGreaterEqual(OSYS,2012)) { // Win8 or Later, not support
      Return(0x0000)
    }
    
    if (LEqual (LFSP, 1)) { // Legacy Free Support
      Return(0x000F)
    }
    Return (0x0000)
  }
//[-end-160929-IB07400790-modify]//
//[-start-160406-IB07400715-modify]//
//#if BXTI_PF_ENABLE
//  Method(_CRS, 0x0, Serialized)
//  {
//    Name (SBUF, ResourceTemplate ()
//    {
//      IO(Decode16,0x60,0x60,0x01,0x01)
//      IO(Decode16,0x64,0x64,0x01,0x01)
//      IRQ(Edge,ActiveHigh,Exclusive){0x01}
//    })
//    Name (RBUF, ResourceTemplate ()
//    {
//      IRQ(Edge,ActiveHigh,Exclusive){0x01}
//    })
//	If(LEqual(BDID,MNHL))
//	{
//	  Return (SBUF)
//	}
//	Return(RBUF)
//  }
//  
//  Method(_PRS, 0x0, Serialized)
//  {
//    Name (SBUF, ResourceTemplate ()
//    {
//      StartDependentFn(0, 0) {
//        FixedIO(0x60,0x01)
//        FixedIO(0x64,0x01)
//        IRQNoFlags(){1}
//        }
//      EndDependentFn()
//    })
//    Name (RBUF, ResourceTemplate ()
//    {
//      StartDependentFn(0, 0) {
//        IRQNoFlags(){1}
//        }
//      EndDependentFn()
//    })
//	  If(LEqual(BDID,MNHL))
//	  {
//	    Return (SBUF)
//	  }
//	  Return(RBUF)
//  }
//#else
  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x60,0x60,0x01,0x01)
    IO(Decode16,0x64,0x64,0x01,0x01)
    IRQ(Edge,ActiveHigh,Exclusive){0x01}
  })

  Name(_PRS, ResourceTemplate() {
    StartDependentFn(0, 0) {
      FixedIO(0x60,0x01)
      FixedIO(0x64,0x01)
      IRQNoFlags(){1}
      }
    EndDependentFn()
  })
//#endif
//[-end-160406-IB07400715-modify]//

}

Device(PS2M)  // PS/2 Mouse
{
  Name(_HID,"MSFT0003")
  Name(_CID,EISAID("PNP0F13"))
    
//[-start-160929-IB07400790-modify]//
  Method(_STA,0,Serialized)
  {
//    //
//    // If KBC I/O port is not decoded in SuperIo driver, it will read back as 0xFF.
//    //
//    If(And(LEqual(PKBD, 0xFF), LEqual(PKBC, 0xFF)))
//    {
//      Return (0x0000)
//    }
//
//    If (LAnd (LEqual(P2ME, 1), LGreater(P2MK, 0)))
//    {
//      Return (0x000F)
//    }
    
    // Only report resources to the OS if the Mouse is present
    If (LGreaterEqual(OSYS,2012)) { // Win8 or Later, not support
      Return(0x0000)
    }
    if (LEqual (LFSP, 1)) { // Legacy Free Support
      Return(0x000F)
    }
    Return (0x0000)
  }
//[-end-160929-IB07400790-modify]//

  Name(_CRS,ResourceTemplate()
  {
    IRQ(Edge,ActiveHigh,Exclusive){0x0C}
  })

  Name(_PRS, ResourceTemplate() {
    StartDependentFn(0, 0) {
      IRQNoFlags(){12}
    }
    EndDependentFn()
  })
}
} //end of SCOPE
