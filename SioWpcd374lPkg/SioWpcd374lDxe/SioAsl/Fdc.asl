/** @file
  ASL code for Fdc.

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

  Fdc.asl

Abstract:

  ASL code for Fdc.

**/

#define SIO_FLOPPY    CLDN

Device (FDC0) {  // floppy disk controller

  Name (_HID, EISAID("PNP0700"))
  Name (_UID, "SModuleF")

  //*****************************************************
  // Method _STA:  Return Status
  //*****************************************************
  Method (_STA, 0, NotSerialized) { // Return Status of the FDC

    // Enter Configuration Mode
    ENCG ()

    // Select the FDC
    Store (SIO_FLOPPY, CR07)

    // Default setting: no device
    Store (0x00, Local0)

    //
    // Depend on SCU value through MailBox to enable or disable this device
    //
    if (CDVC) {
      Store (0x0D, Local0)

      //
      // Check enable register
      // local device is exist and enable
      //
      If (CR30) {
        Store (0x0F, Local0)
      }
    }
    EXCG()
    Return (Local0)
  } // end of Method _STA


  //*****************************************************
  //  Method _CRS:  Return Current Resource Settings
  //*****************************************************
  Method (_CRS, 0, NotSerialized) {

    Name (BUF0, ResourceTemplate() {
      IO (Decode16, 0x3F0,0x3F0, 0x01, 0x06)
      IO (Decode16, 0x3F7,0x3F7, 0x01, 0x01)
      IRQNoFlags(){6}
      DMA (Compatibility, NotBusMaster, Transfer8){2}
    })
    //
    // Create some ByteFields in the Buffer in order to
    // permit saving values into the data portions of
    // each of the descriptors above.
    //
    CreateByteField (BUF0, 0x02, IOLO) // IO Port Low
    CreateByteField (BUF0, 0x03, IOHI) // IO Port Hi
    CreateByteField (BUF0, 0x04, IORL) // IO Port Low
    CreateByteField (BUF0, 0x05, IORH) // IO Port Hi
    CreateByteField (BUF0, 0x0A, I2LO) // IO2 Port Low
    CreateByteField (BUF0, 0x0B, I2HI) // IO2 Port Hi
    CreateByteField (BUF0, 0x0C, I2RL) // IO2 Port Low
    CreateByteField (BUF0, 0x0D, I2RH) // IO2 Port Hi
    CreateByteField (BUF0, 0x11, IRQL) // IRQ Low
    CreateByteField (BUF0, 0x14, DMAV) // DMA

    // Enter Configuration Mode
    ENCG ()

    // Select the FDC
    Store (SIO_FLOPPY, CR07)

    //
    // Get the IO setting from the chip, and copy it
    // to both the min & max for both IO descriptors.
    //

    // Low Bytes:
    Store (CR61, IOLO)  // 1st io, min.
    Store (CR61, IORL)  // 1st io, max.
    Store (CR61, I2LO)  // 2nd io, min.
    Store (CR61, I2RL)  // 2nd io, max.

    // High Bytes:
    Store (CR60, IOHI)  // 1st io, min.
    Store (CR60, IORH)  // 1st io, max.
    Store (CR60, I2HI)  // 2nd io, min.
    Store (CR60, I2RH)  // 2nd io., max.


    //
    // Get the IRQ setting from the chip, and shift
    // it into the Descriptor byte.
    //
    ShiftLeft (0x01, CR70, IRQL)

    //
    // Get the DMA setting from the chip.
    // and shift it into the Descriptor Byte.
    //
    ShiftLeft (0x01, CR74, DMAV)

    // Exit Configuration Mode
    EXCG ()

    Return(BUF0) // return the result

  } // end _CRS Method


  //*****************************************************
  //  Method _PRS:  Return Possible Resource Settings
  //*****************************************************
  Method (_PRS, 0, NotSerialized) {    // return Possible Resource Settings

    // Since the FDC doesn't move around much, this is pretty easy.

    Name (BUF0, ResourceTemplate() {
       // Good configuration for Legacy O/S and for ACPI O/S
       StartDependentFn(0,0) {
         IO (Decode16,0x3F0, 0x3F0, 0x01, 0x06)
         IO (Decode16,0x3F7, 0x3F7, 0x01, 0x01)
         IRQNoFlags(){6}
         DMA (Compatibility, NotBusMaster, Transfer8){2}
       }
       EndDependentFn()
    })

    Return(BUF0)  // Return Buf0
  }  // End of _PRS


  //*****************************************************
  //  Method _SRS:  Set Resource Setting
  //*****************************************************
  Method (_SRS, 1, NotSerialized) {    // Set Resource Settings
    // ARG0 is PnP Resource String to set

    //
    // Define offsets into descriptor to match value
    // returned by descriptors in _PRS method
    //
    CreateByteField (ARG0, 0x02, IOLO) // IO Port Low
    CreateByteField (ARG0, 0x03, IOHI) // IO Port Hi
    CreateWordField (Arg0, 0x02, IOAD) // IO Word
    CreateByteField (ARG0, 0x11, IRQL) // IRQ
    CreateByteField (ARG0, 0x14, DMAV) // DMA

    // Enter Configuration Mode
    ENCG ()

    // Set Logical Device to select FDC
    Store (SIO_FLOPPY, CR07)

    // Set the IO Base Address -- set base 8-aligned down
    And (IOLO, 0xF8, Local0)
    Store (Local0, CR61)
    Store (IOHI, CR60)

    // Set IRQ
    FindSetRightBit (IRQL,CR70)
    If (LNotEqual (IRQL,Zero)) {
      Decrement(CR70)
    }

    // Set DMA
    If (LEqual(DMAV,Zero)) {
      Store (0x04, CR74)
    } Else {
      FindSetRightBit (DMAV, CR74) // Find DMA - 1 based
      Decrement(CR74)
    }

    // Activate the Device
    Store (0x01, CR30)

    // Exit Configuration Mode
    EXCG ()
  } // end _SRS Method

}  // end of Device FDC0

