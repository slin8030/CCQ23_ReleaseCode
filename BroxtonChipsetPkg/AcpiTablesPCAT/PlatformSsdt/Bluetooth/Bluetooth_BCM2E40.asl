/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Intel Atom          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2012 - 2016 Intel Corporation. All rights reserved    *;
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
// Bluetooth controller using serial interface
//
// GPIO_214: NGFF_BT_DEV_WAKE_N
// GPIO_154: BT_DISABLE2_1P8_N
// GPIO_75 : NGFF_CONN_UART_WAKE_N (IRQ)
//

Scope(\_SB.PCI0.URT1)
{
  Device(BTH0) {
    Method(_HID) {
      // Return HID based on BIOS Setup
      Return("BCM2EA8")
    }

    Method(_INI) {
    }

    Method(_CRS, 0x0, Serialized){
      Name(SBFG, ResourceTemplate (){
        UARTSerialBus(115200,,,0xc0,,,FlowControlHardware,32,32,"\\_SB.PCI0.URT1" )
        GpioIo(Exclusive, PullDown, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {27}  // North-west(27):GPIO_214 NGFF_BT_DEV_WAKE_N  
        GpioIo(Exclusive, PullDown, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {24}  // West(24):GPIO_154, BT_DISABLE2_1P8_N
        GpioInt (Edge, ActiveLow, Exclusive, PullDefault, 0x0000, "\\_SB.GPO1", 0x00, ResourceConsumer, ) {35}
      })

      Name(SBFI, ResourceTemplate (){
        UARTSerialBus(115200,,,0xc0,,,FlowControlHardware,32,32,"\\_SB.PCI0.URT1" )
        GpioIo(Exclusive, PullDown, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO1", ) {27}  // North-west(27):GPIO_214 NGFF_BT_DEV_WAKE_N  
        GpioIo(Exclusive, PullDown, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2", ) {24}  // West(24):GPIO_154, BT_DISABLE2_1P8_N
        Interrupt(ResourceConsumer, Edge, ActiveLow, Exclusive, ) {0x34}
      })

      If(LLess(OSYS,2012)) { return (SBFI) } // For Windows 7 only report Interrupt; it doesn't support ACPI5.0 and wouldn't understand GpioInt nor I2cBus
      Return (SBFG)
    }

    Method (_STA, 0x0, NotSerialized){
      If (LEqual (SBTD, 1)) {
        Return (0xF)
      }
      Return (0)
    }

    Name (_S0W, 2)                            // required to put the device to D2 during S0 idle
  } // Device BTH0

}  //  end Scope(\_SB.PCI0.URT1)
