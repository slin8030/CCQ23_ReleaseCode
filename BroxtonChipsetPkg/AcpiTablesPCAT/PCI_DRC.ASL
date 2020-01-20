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


Scope (\_SB.PCI0) {

  Device(PDRC)  // PCI Device Resource Consumption
  {
    Name(_HID,EISAID("PNP0C02"))

    Name(_UID,1)

    Name(BUF0,ResourceTemplate()
    {
      //
      // PCI Express BAR _BAS and _LEN will be updated in _CRS below according to B0:D0:F0:Reg.60h
      // Forced hard code at the moment.
      //
      //Memory32Fixed (ReadWrite, 0, 0, PCIX)  // PCIEX BAR
      Memory32Fixed (ReadWrite, 0x0E0000000, 0x010000000, PCIX)

      //
      // MPHY BAR. Check if the hard code meets the real configuration.
      // If not, dynamically update it like the _CRS method below.
      //
      Memory32Fixed (ReadWrite, 0x0FEA00000, 0x0100000, MPHB)  // MPHY BAR

      //
      // SPI BAR. Check if the hard code meets the real configuration.
      // If not, dynamically update it like the _CRS method below.
      //
      Memory32Fixed (ReadWrite, 0x0FED01000, 0x01000,SPIB)  // SPI BAR

      //
      // PMC BAR. Check if the hard code meets the real configuration.
      // If not, dynamically update it like the _CRS method below.
      //
      Memory32Fixed (ReadWrite, 0x0FED03000, 0x01000,PMCB)  // PMC BAR

      //
      // PUNIT BAR. Check if the hard code meets the real configuration.
      // If not, dynamically update it like the _CRS method below.
      //
      Memory32Fixed (ReadWrite, 0x0FED06000, 0x01000, PUNB)  // PUNIT BAR

      //
      // ILB BAR. Check if the hard code meets the real configuration.
      // If not, dynamically update it like the _CRS method below.
      //
      Memory32Fixed (ReadWrite, 0x0FED08000, 0x02000, ILBB)  // ILB BAR

      //
      // IO BAR. Check if the hard code meets the real configuration.
      // If not, dynamically update it like the _CRS method below.
      //
      Memory32Fixed (ReadWrite, 0x0FED80000, 0x040000, IOBR)  // IO BAR

      //
      // RCRB BAR _BAS will be updated in _CRS below according to B0:D31:F0:Reg.F0h
      //
      Memory32Fixed (ReadWrite, 0x0FED1C000, 0x01000, RCRB)  // RCRB BAR

      //
      // Local APIC range(0xFEE0_0000 to 0xFEEF_FFFF)
      //
      Memory32Fixed (ReadOnly, 0x0FEE00000, 0x0100000, LIOH)
    })

    Method(_CRS,0,Serialized)
    {
/*
      //
      // Update RCBA Base Address
      //
      CreateDwordField(BUF0,^RCRB._BAS,RBR0)
      ShiftLeft(\_SB.PCI0.LPCB.RCBA,14,RBR0)

      //
      // Update PCIEX Base Address
      //
      CreateDwordField(BUF0,^PCIX._BAS,XBR0)
      ShiftLeft(\_SB.PCI0.PXBR,26,XBR0)

      //
      // Update PCIEX Length
      //
      CreateDwordField(BUF0,^PCIX._LEN,XSZ0)
      ShiftRight(0x10000000, \_SB.PCI0.PXSZ,XSZ0)
*/
      Return(BUF0)
    }
  }
}
