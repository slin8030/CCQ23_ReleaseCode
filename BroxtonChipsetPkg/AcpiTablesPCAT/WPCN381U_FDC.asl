/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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


Device(FDC1)	// Floppy Disk controller
{
	NAME(_HID, EISAID("PNP0700"))

	// Status Method for FDC.
	Method(_STA,0,Serialized)			
	{
		// Only report resources to the OS if the SIO Device is
		// not set to Disabled in BIOS Setup.

		If(W381)
		{
			// Set the SIO to UART 5.

			Store(0,WR07)

			// Display UART 5 and return status.

			If(WR30) 
			{
				Return(0x000F)
			}

			Return(0x000D)
		}

		Return(0x0000)

	}

	// Disable Method for FDC.

	Method(_DIS,0,Serialized)
	{
		Store(0,WR07)
		Store(0,WR30)
	}

	// Current Resource Setting Method for FDC.

	Method(_CRS,0,Serialized)			
	{
		// Create the Buffer that stores the Resources to
		// be returned.

		Name(BUF0,ResourceTemplate()
		{
			IO(Decode16,0x3F0,0x3F0,0x01,0x06)
			IO(Decode16,0x3F7,0x3F7,0x01,0x01)
			IRQNoFlags(){6}
			DMA(Compatibility,NotBusMaster,Transfer8_16){2}
		})
		
		// Set SIO to FDC.
		Store(0,WR07)

		// Skip this sequence if the UART 5 Port is Disabled
		// in BIOS Setup.

		If(W381)
		{
			// Create pointers to the specific byte.
			CreateByteField(BUF0,0x02,IOL0)
			CreateByteField(BUF0,0x03,IOH0)
			CreateByteField(BUF0,0x04,IOL1)
			CreateByteField(BUF0,0x05,IOH1)
			CreateByteField(BUF0,0x07,LEN0)
			CreateByteField(BUF0,0x0A,IOL2)
			CreateByteField(BUF0,0x0B,IOH2)
			CreateByteField(BUF0,0x0C,IOL3)
			CreateByteField(BUF0,0x0D,IOH3)
			CreateByteField(BUF0,0x0F,LEN1)
	 		CreateWordField(BUF0,0x11,IRQW)
			CreateByteField(BUF0,0x14,DMA0)

			// Write IO and Length values into the Buffer.
		
			Store(And(WR61,0xF0),IOL0)
			Store(WR60,IOH0)
			If(LAnd(IOL0,IOH0))
			{
				Store(IOL0,IOL1)
				Store(IOH0,IOH1)
				Store(Or(IOL0,7),IOL2)
				Store(IOH0,IOH2)
				Store(IOL2,IOL3)
				Store(IOH2,IOH3)
				Store(6,LEN0)
				Store(1,LEN1)
			}

			// Write the IRQ value into the Buffer.
		
			And(WR70,0x0F,Local0)
			If(Local0)
			{
				ShiftLeft(One,Local0,IRQW)
			}
			Else
			{
				Store(Zero,IRQW)
			}

			// Write the DMA value into the Buffer.

			Store(WR74,Local0)
			If(LEqual(Local0,0x04))
			{
				Store(Zero,DMA0)
			}
			Else
			{
				ShiftLeft(One,Local0,DMA0)
			}
		}

		Return(BUF0) 
	}

	// Possible Resource Setting Method for FDC.

		// Build a Buffer with all valid FDC Resources.

		Name(_PRS,ResourceTemplate()
		{

			StartDependentFn(0,2)
			{
				IO(Decode16,0x3f0,0x3f0,0x01,0x06)
				IO(Decode16,0x3f7,0x3f7,0x01,0x01)
				IRQNoFlags(){6}
				DMA(Compatibility,NotBusMaster,Transfer8_16){2}
			}
			StartDependentFn(0,2)
			{
				IO(Decode16,0x370,0x370,0x01,0x06)
				IO(Decode16,0x377,0x377,0x01,0x01)
				IRQNoFlags(){6}
				DMA(Compatibility,NotBusMaster,Transfer8_16){2}
			}
	
			EndDependentFn()
		})


	// Set Resource Setting Method for FDC.

	Method(_SRS,1,Serialized)
	{
		// Point to the specific information in the passed
		// in Buffer.

		CreateByteField(Arg0,0x02,IOLO)
		CreateByteField(Arg0,0x03,IOHI)
		CreateWordField(Arg0,0x11,IRQW)
		//CreateWordField(Arg0,0x14,DMAC)

		// Set the SIO to FDC.

		Store(0,WR07)

		// Disable the device.

		Store(0,WR30)

		// Set the Base IO Address.

		Store(IOLO,WR61)
		Store(IOHI,WR60)

 		// Set the IRQ.

		FindSetRightBit(IRQW,Local0)
		If(LNotEqual(IRQW,Zero))
		{
			Decrement(Local0)
		}
		Store(Local0,WR70)

 		// Set the DMA Channel.

		/*FindSetRightBit(DMAC,Local0)
		If(LNotEqual(DMAC,Zero))
		{
			Decrement(Local0)
		}*/
		Store(Local0,WR74)

		// Enable the device.

		Store(1,WR30)
	}

	// D0 Method for FDC.

	Method(_PS0,0,Serialized)
	{
		Store(0,WR07)
		Store(1,WR30)
	}

	// D3 Method for FDC.

	Method(_PS3,0,Serialized)
	{
		Store(0,WR07)
		Store(0,WR30)
    	}
}