/** @file
  ASL code for Wpcd374l.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

SModule Name:

  SioWpcd374l.asl

Abstract:

  ASL code for Wpcd374l.

**/

DefinitionBlock (
  "Wpcd374lACPI.aml",
  "SSDT",
  0x01,
  "Insyde",
  "SIOACPI",
  0x1000
  )
{
  #define ASL_LPC_PATH \_SB.PCI0.LPCB // Intel
//  #define ASL_LPC_PATH \_SB.PCI0.LPC0 // AMD
  #define ASL_CONFIG_PORT 0x2E
  #define ASL_ENTRY_REG   0x87
  #define ASL_EXIT_REG    0xAA

  External (ASL_LPC_PATH, DeviceObj)

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
          INDX, 8,  // field named INDX is 8 bits wide
          DATA, 8   // field named DATA is 8 bits wide
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

        Offset(0x22),
        CR22, 8, //PWRC, 8, // Power Control Register
        Offset(0x2D),
        CR2D, 8, //GR2D,8,
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

        Offset(0x72),
        CR72, 8, //INT1, 8, // Secondary IRQ register (some devices)

        Offset(0x74),
        CR74, 8, //DMCH, 8, // Primary DMA channel (some devices)

        Offset(0xC0),
        CRC0, 8, //GP40, 8,

        Offset(0xE0),
        CRE0, 8, //RGE0, 8, //
        CRE1, 8, //RGE1, 8, //
        CRE2, 8, //RGE2, 8, //
        CRE3, 8, //RGE3, 8, //
        CRE4, 8, //RGE4, 8, //
        CRE5, 8, //RGE5, 8, //
        CRE6, 8, //RGE6, 8, //
        CRE7, 8, //RGE7, 8, //
        Offset(0xEB),
        CREB, 8,
        CREC, 8,
        Offset(0xF0),
        CRF0, 8, //OPT0, 8, // first Options registers (some devices)
        CRF1, 8, //OPT1, 8,
        CRF2, 8, //OPT2, 8,
        CRF3, 8, //OPT3, 8,
        CRF4, 8, //OPT4, 8,
        CRF5, 8, //OPT5, 8,
        CRF6, 8, //OPT6, 8,
        CRF7, 8, //OPT7, 8,
        CRF8, 8, //OPT8, 8,
        CRF9, 8, //OPT9, 8

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
        BDVC, 8,  //Com3 Status
        BBIO, 16, //Com3 BaseIO
        BLDN, 8,  //Com3 LDN
        CDVC, 8,  //Floppy Status
        CBIO, 16, //Floppy BaseIO
        CLDN, 8,  //Floppy LDN
        DDVC, 8,  //LPT Status
        DBIO, 16, //LPT BaseIO
        DLDN, 8,  //LPT LDN
        EDVC, 8,  //KYBD Status
        EBIO, 16, //KYBD BaseIO
        ELDN, 8,  //KYBD LDN
        FDVC, 8,  //MOUSE Status
        FBIO, 16, //MOUSE BaseIO
        FLDN, 8,  //MOUSE LDN
      }

      // Enter Configuration mode
      Method (ENCG) {
        Store (ASL_ENTRY_REG, INDX)
        Store (ASL_ENTRY_REG, INDX)
      }

      // Exit Configuation mode
      Method (EXCG) {
        Store (ASL_EXIT_REG, INDX)
      }

      include ("ParallelPort1.asl")
      include ("PS2KM.asl")
      include ("UART1.asl")
      include ("UART2.asl")
      include ("Fdc.asl")
    }
  }
}