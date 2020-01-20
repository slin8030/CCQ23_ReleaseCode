/** @file

ASL code for SIO

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

DefinitionBlock (
  "IT8987EACPI.aml",
  "SSDT",
  0x01,
  "Insyde",
  "SIOACPI",
  0x1000
  )
{
  #define ASL_LPC_PATH \_SB.PCI0.LPCB
  #define ASL_CONFIG_PORT 0x2E

  External (ASL_LPC_PATH, DeviceObj)
  External(KBWK)
  External(MSWK)

  scope (ASL_LPC_PATH) {

    Device (SIOx) {
      Name (_HID, EISAID("PNP0A05"))
      Name (_UID, "SModule")
      Name (SIDP, ASL_CONFIG_PORT)
      Method (_STA, 0, Serialized) {
        Return (0x000F)  // Enabled, do Display.
      }
      OperationRegion (
        OFST,       // name of Operation Region for SuperIO device
        SystemIO,   // type of address space
        SIDP,       // base address for Configuration Mode
        2)          // size of region in bytes

      //  This Field definition defines the Index and Data port registers
      //  used in Configuration Mode access for the 17x device.
      //
      Field (
        OFST,       // name of Operation Region containing field
        ByteAcc,    // access in Byte mode
        NoLock,     // may change, depending on SMI behavior
        Preserve)   // Update Rule
      {
        INDX, 8,    // field named INDX is 8 bits wide
        DATA, 8     // field named DATA is 8 bits wide
      }

      //
      //  This IndexField defines the offsets of the various configuration
      //  registers in the Configuration Register Address Space, which
      //  in turn is accessed using the Field above.
      //
      IndexField (INDX,       // name of Index register field
                  DATA,       // name of Data register field is DATA
                  ByteAcc,    // both access as byte registers
                  NoLock,     // may change, depending on SMI behavior
                  Preserve)   // update rule
      {
        // Global Configuration Registers:
        //
        Offset(0x07),
        CR07, 8, //LDN, 8,  // Logical Device Number

        //
        // Per-Device Configuration Registers:
        //
        Offset(0x30),
        CR30, 8, //ACTR, 8, // Active register

        Offset(0x60),
        CR60, 8, //IOAH, 8, // Primary i/o base address, high byte
        CR61, 8, //IOAL, 8, // low byte

        Offset(0x70),
        CR70, 8, //INTR, 8, // Primary IRQ register

        Offset(0x74),
        CR74, 8, //DMCH, 8, // Primary DMA channel (some devices)

      } // end of Indexed Field

      //
      //For SIO, mailbox struct
      //
      OperationRegion(LGDN, SystemMemory, 0x55AA55AA, 0x55AA55AA)
      Field(LGDN, AnyAcc, Lock, Preserve)
      {
        Offset(0),
        ADVC, 8,  //Com1 Status
        ABIO, 16, //Com1 BaseIO
        ALDN, 8,  //Com1 LDN
        BDVC, 8,  //Com2 Status
        BBIO, 16, //Com2 BaseIO
        BLDN, 8,  //Com2 LDN
        IDVC, 8,  //KYBD Status
        IBIO, 16, //KYBD BaseIO
        ILDN, 8,  //KYBD LDN
        JDVC, 8,  //MOUSE Status
        JBIO, 16, //MOUSE BaseIO
        JLDN, 8,  //MOUSE LDN
        LDVC, 8,  //CIR Status
        LBIO, 16, //CIR BaseIO
        LLDN, 8,  //CIR LDN
      }

      Method (ENCG) {
        if (LEqual(SIDP, 0x2E)) {
          Store (0x87, INDX)
          Store (0x01, INDX)
          Store (0x55, INDX)
          Store (0x55, INDX)
        } else {
          Store (0x87, INDX)
          Store (0x01, INDX)
          Store (0x55, INDX)
          Store (0xAA, INDX)
        }
      }

      Method (EXCG) {
        Store (0x02, INDX)
        Store (0x02, DATA)
      }

      include ("Uart1.asl")
      include ("Uart2.asl")
      include ("PS2KM.asl")
      include ("Cir.asl")
    }
  }
}
