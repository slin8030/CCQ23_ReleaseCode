/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Intel Atom          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2015 - 2016 Intel Corporation. All rights reserved           *;
;*    This software and associated documentation (if any) is furnished    *;
;*    under a license and may only be used or copied in accordance        *;
;*    with the terms of the license. Except as permitted by such          *;
;*    license, no part of this software or documentation may be           *;
;*    reproduced, stored in a retrieval system, or transmitted in any     *;
;*    form or by any means without the express written consent of         *;
;*    Intel Corporation.                                                  *;
;*                                                                        *;
;*                                                                        *;
;**************************************************************************/
/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
--*/
//
// Fingerprint controller using serial interface
//
// GPIO_112: FGR_RESET_N
// GPIO_14 : FGR_INT
//
External(\SDS1, IntObj)

//-----------------------------
//  Serial IO SPI1 Controller
//-----------------------------
Scope(\_SB.PCI0.SPI1)
{
  Device(FPNT) {
    Method(_HID) {
      //
      // Return FPS HID based on BIOS Setup
      //
      if(LEqual(SDS1, 1)){ Return("FPC1020") }
      if(LEqual(SDS1, 2)){ Return("FPC1021") }
      Return("FPNT_DIS")
    }

    Method(_STA) {
      //
      // Is SerialIo SPI1 FPS enabled in BIOS Setup?
      //
      If(LNotEqual(SDS1, 0)) {
        Return(0x0F)
      }
      Return(0x00)
    }
    Method(_CRS, 0x0, Serialized) {
      Name(BBUF,ResourceTemplate () {
        SPISerialBus(0x00,PolarityLow,FourWireMode,8,ControllerInitiated,3000000,ClockPolarityLow,ClockPhaseFirst,"\\_SB.PCI0.SPI1",,,SPIR)
        GpioIo(Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1",,,GSLP) {67} //North-west(67):GPIO_112 FGR_RESET_N
	    GpioInt (Edge, ActiveHigh, ExclusiveAndWake, PullDefault, 0x0000, "\\_SB.GPO0", 0x00, ResourceConsumer,GINT ) {14} //North(14):GPIO_14 : FGR_INT
      })
      Return (BBUF)
    }
  } // Device (FPNT)
}  //  end Scope(\_SB.PCI0.SPI1)
