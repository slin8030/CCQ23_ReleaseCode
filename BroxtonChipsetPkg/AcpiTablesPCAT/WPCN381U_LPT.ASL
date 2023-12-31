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




// This ASL file will support the LPT in Output, BiDirectional, EPP,
// & ECP mode.  Only 1 of these devices will be shown by the OS at any
// given time, and that one will depend on the BIOS configuration.
Scope(\_SB.PCI0.LPCB.WPCN){

Device(LPT2)	// Printer, Output Mode
{
	Name(_HID,EISAID("PNP0400"))
	Name(_UID,2)

	// Status Method for LPT-Output Mode.

	Method(_STA,0,Serialized) 
	{
		// Output mode = 000x xxxxb - check that no bits are set.

		Store(1,WR07)
		If(LEqual(And(WRF0,0xE0),0x00))
		{
			// LPT is in Output Mode.  Only report resources
			// to the OS if the SIO Device is not set to
			// Disabled in BIOS Setup.

			If(W381) 
			{
				// Set SIO to LPT.

				Store(1,WR07)

				// Display POUT and return Status.

				If(WR30)
				{
					Return(0x000F)
				}

				Return(0x000D)
			}
		}

		Return(0x0000)
	}

	// Disable Method for LPT-Output Mode.

	Method(_DIS,0,Serialized)
	{
		// Set SIO to LPT.

		Store(1,WR07)

		If(LEqual(And(WRF0,0xE0),0x00))
		{
			Store(0,WR30)
		}	
		
	}
	 
	// Current Resource Setting Method for LPT-Output Mode.

	Method(_CRS,0,Serialized)
	{
		// Create a buffer to return to OS.

		Name(BUF0,ResourceTemplate()
		{
			IO(Decode16,0x0378,0x0378,0x01,0x04)	   
			IRQNoFlags(){7}
		})

		// Skip this sequence if the LPT Device is not set
		// to Output Mode.

		If(LEqual(And(WRF0,0xE0),0x00))
		{
			// Create pointers to the specific byte.

			CreateByteField(BUF0,0x02,IOL0)
			CreateByteField(BUF0,0x03,IOH0)
			CreateByteField(BUF0,0x04,IOL1)
			CreateByteField(BUF0,0x05,IOH1)
			CreateByteField(BUF0,0x07,LEN0)
	 		CreateWordField(BUF0,0x09,IRQW)
	
			// Set SIO to LPT.
			Store(1,WR07)

			// Write IO and Length values into the Buffer.
		
			Store(WR61,IOL0)
			Store(WR60,IOH0)
			Store(IOL0,IOL1)
			Store(IOH0,IOH1)
			Store(4,LEN0)
			
			// Write the IRQ value into the Buffer.

			If(And(WR70,0x0F))
			{
				ShiftLeft(One,And(WR70,0x0F),IRQW)
			}
			Else
			{
				Store(Zero,IRQW)
			}
		}

		Return(BUF0) 
	}

	// Possible Resource Setting Method for LPT-Output Mode.
	// Build a Buffer with all valid LPT Port Resources.

		Name(_PRS,ResourceTemplate()
		{
			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x04)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x04)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x3BC,0x3BC,0x01,0x04)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x04)
				IRQNoFlags(){5}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x04)
				IRQNoFlags(){5}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x3BC,0x3BC,0x01,0x04)
				IRQNoFlags(){5}
			}

			EndDependentFn()
		})
		

	// Set Resource Setting Method for LPT-Output Mode.

	Method(_SRS,1,Serialized) 
	{
		// Point to the specific information in the passed
		// in Buffer.

		CreateByteField(Arg0,0x02,IOL0)
		CreateByteField(Arg0,0x03,IOH0)
		CreateWordField(Arg0,0x09,IRQW)

		// Set SIO to LPT.

		Store(1,WR07)

		// Disable the LPT.

		Store(0,WR30)

		// Disable the DMA channels.

          	Store(4,WR74)
		
		// Set the Base IO Address.
		
          	Store(IOL0,WR61)
          	Store(IOH0,WR60)

		// Set the IRQ.

		FindSetRightBit(IRQW,Local0)
		If (LNotEqual(IRQW,Zero))
		{
			Decrement(Local0)
		}
		Store(Local0,WR70)

		// Enable the LPT.
		Store(1,WR30)

               // Set to Output Mode.
               And(WRF0,0x0F,WRF0) 


		
	}

	// D0 Method for LPT-Output Mode.

	Method(_PS0,0,Serialized)
	{
		Store(1,WR07)
		Store(1,WR30)
		
	}

	// D3 Method for LPT-Output Mode.

	Method(_PS3,0,Serialized)
	{
		Store(1,WR07)
		Store(0,WR30)
		
    	}
}

Device(PBI2)	// Printer, BiDirectional Mode
{
	Name(_HID,EISAID("PNP0400"))
	Name(_UID,2)

	// Status Method for LPT-BiDirectional Mode.

	Method(_STA,0,Serialized) 
	{
		// BiDirectional mode = 001x xxxxb, so check that only 
		// bit 5 is set.
		Store(1,WR07)
		
		If(LEqual(And(WRF0,0xE0),0x20))
		{
			// LPT is in BiDirectional Mode.  Only report
			// resources to the OS if the SIO Device is not
			// set to Disabled in BIOS Setup.

			If(W381) 
			{
				// Display PBID and return Status.

				If(WR30)
				{
					Return(0x000F)
				}

				Return(0x000D)
			}
		}

		Return(0x0000)
	}

	// Disable Method for LPT-BiDirectional Mode.

	Method(_DIS,0,Serialized)
	{
		// Set SIO to LPT.

		Store(1,WR07)

		If(LEqual(And(WRF0,0xE0),0x20))
		{
			Store(0,WR30)
		}	
	}
	 
	// Current Resource Setting Method for LPT-BiDirectional Mode.

	Method(_CRS,0,Serialized)
	{
		// Create a buffer to return to OS.

		Name(BUF0,ResourceTemplate()
		{
			IO(Decode16,0x0378,0x0378,0x01,0x04)	   
			IRQNoFlags(){7}
		})

		// Skip this sequence if the LPT Device is not set
		// to BiDirectional Mode.

		If(LEqual(And(WRF0,0xE0),0x20))
		{
			// Create pointers to the specific byte.

			CreateByteField(BUF0,0x02,IOL0)
			CreateByteField(BUF0,0x03,IOH0)
			CreateByteField(BUF0,0x04,IOL1)
			CreateByteField(BUF0,0x05,IOH1)
			CreateByteField(BUF0,0x07,LEN0)
	 		CreateWordField(BUF0,0x09,IRQW)
	
			// Set SIO to LPT.

			Store(1,WR07)

			// Write IO and Length values into the Buffer.
		
			Store(WR61,IOL0)
			Store(WR60,IOH0)
			Store(IOL0,IOL1)
			Store(IOH0,IOH1)
			Store(4,LEN0)

			// Write the IRQ value into the Buffer.

			If(And(WR70,0x0F))
			{
				ShiftLeft(One,And(WR70,0x0F),IRQW)
			}
			Else
			{
				Store(Zero, IRQW)
			}
		}

		Return(BUF0) 
	}

	// Possible Resource Setting Method for LPT-BiDirectional Mode.

		Name(_PRS,ResourceTemplate()
	{
			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x04)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x04)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x3BC,0x3BC,0x01,0x04)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x04)
				IRQNoFlags(){5}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x04)
				IRQNoFlags(){5}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x3BC,0x3BC,0x01,0x04)
				IRQNoFlags(){5}
			}

			EndDependentFn()
		})
		

	// Set Resource Setting Method for LPT-BiDirectional Mode.

	Method(_SRS,1,Serialized) 
	{
		// Arg0 passed into _SRS is the buffer identical to BUF0.

		CreateByteField(Arg0,0x02,IOL0)
		CreateByteField(Arg0,0x03,IOH0)
		CreateWordField(Arg0,0x09,IRQW)

		Store(1,WR07)

		// Disable the LPT.

		Store(0,WR30)

		// Disable the DMA channels.

          	Store(4,WR74)
		
		// Set the Base IO Address.
		
          	Store(IOL0,WR61)
          	Store(IOH0,WR60)

		// Set the IRQ.

		FindSetRightBit(IRQW,Local0)
		If (LNotEqual(IRQW,Zero))
		{
			Decrement(Local0)
		}
		Store(Local0,WR70)

		// Enable the LPT.

		Store(1,WR30)

               // Set to Bi-Directional Mode.

	       Or(And(WRF0,0x0F),0x20,WRF0)

		
	}

	// D0 Method for LPT-BiDirectional Mode.

	Method(_PS0,0,Serialized)
	{
		Store(1,WR07)
		Store(1,WR30)
		
	}

	// D3 Method for LPT-BiDirectional Mode.

	Method(_PS3,0,Serialized)
	{
		Store(1,WR07)
		Store(0,WR30)
   	}
}

Device(PEP2)	// Printer, EPP Mode
{
	Name(_HID,EISAID("PNP0400"))
	Name(_UID,2)

	// Status Method for LPT-EPP Mode.

	Method(_STA,0,Serialized) 
	{
		// EPP 1.9 mode = 011x xxxxb.

		Store(1,WR07)
		If(LOr(LEqual(And(WRF0,0xE0),0x60), LEqual(And(WRF0,0xE0),0x40)))
		{
			// LPT is in EPP Mode.  Only report resources
			// to the OS if the SIO Device is not set to
			// Disabled in BIOS Setup.

			// Display PEPP and return Status.

			If(WR30)
			{
				Return(0x000F)
			}
			Else
			{
				Return(0x000D)
			}
		}
		

		Return(0x0000)
	}

	// Disable Method for LPT-EPP Mode.

	Method(_DIS,0,Serialized)
	{
		// Set SIO to LPT.

		Store(1,WR07)

		If(LOr(LEqual(And(WRF0,0xE0),0x60), LEqual(And(WRF0,0xE0),0x40)))
		{
			Store(0,WR30)
		}	
	}
	 
	// Current Resource Setting Method for LPT-EPP Mode.

	Method(_CRS,0,Serialized)
	{
		// Create a buffer to return to OS.

		Name(BUF0,ResourceTemplate()
		{
			IO(Decode16,0x0378,0x0378,0x01,0x08)	   
			IRQNoFlags(){7}
		})

		// Skip this sequence if the LPT Device is not set
		// to EPP Mode.

		If(LOr(LEqual(And(WRF0,0xE0),0x60), LEqual(And(WRF0,0xE0),0x40)))
		{
			// Create pointers to the specific byte.

			CreateByteField(BUF0,0x02,IOL0)
			CreateByteField(BUF0,0x03,IOH0)
			CreateByteField(BUF0,0x04,IOL1)
			CreateByteField(BUF0,0x05,IOH1)
			CreateByteField(BUF0,0x07,LEN0)
	 		CreateWordField(BUF0,0x09,IRQW)
	
			// Set SIO to LPT.

			Store(1,WR07)

			// Write IO and Length values into the Buffer.
		
			Store(WR61,IOL0)
			Store(WR60,IOH0)
			Store(IOL0,IOL1)
			Store(IOH0,IOH1)
			Store(8,LEN0)

			// Write the IRQ value into the Buffer.

			If(And(WR70,0x0F))
			{
				ShiftLeft(One,And(WR70,0x0F),IRQW)
			}
			Else
			{
				Store(Zero, IRQW)
			}
		}

		Return(BUF0) 
	}

	// Possible Resource Setting Method for LPT-EPP Mode.

		Name(_PRS,ResourceTemplate()
	{
			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x08)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x08)
				IRQNoFlags(){7}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x08)
				IRQNoFlags(){5}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x08)
				IRQNoFlags(){5}
			}

			EndDependentFn()
		})
		

	// Set Resource Setting Method for LPT-EPP Mode.

	Method(_SRS,1,Serialized) 
	{
		// Arg0 passed into _SRS is the buffer identical to BUF0.

		CreateByteField(Arg0,0x02,IOL0)
		CreateByteField(Arg0,0x03,IOH0)
		CreateWordField(Arg0,0x09,IRQW)

		Store(1,WR07)

		// Disable the LPT.

		Store(0,WR30)

		// Disable the DMA channels.

          	Store(4,WR74)
		
		// Set the Base IO Address.
		
          	Store(IOL0,WR61)
          	Store(IOH0,WR60)

		// Set the IRQ.

		FindSetRightBit(IRQW,Local0)
		If (LNotEqual(IRQW,Zero))
		{
			Decrement(Local0)
		}
		Store(Local0,WR70)

		// Enable the LPT.

		Store(1,WR30)

                // Set to SPP + EPP 1.9 Mode.

		Or(And(WRF0,0x0F),0x60,WRF0)

	}

	// D0 Method for LPT-EPP Mode.

	Method(_PS0,0,Serialized)
	{
		Store(1,WR07)
		Store(1,WR30)
	}

	// D3 Method for LPT-EPP Mode.

	Method(_PS3,0,Serialized)
	{
		Store(1,WR07)
		Store(0,WR30)
    	}
}

Device(PEC2)	// Printer, ECP Mode
{
	Name(_HID,EISAID("PNP0401"))
	Name(_UID,2)

	// Status Method for LPT-ECP Mode.

	Method(_STA,0,Serialized) 
	{
		// ECP mode = 111x xxxxb.

		Store(1,WR07)
		If(LOr(LEqual(And(WRF0,0xE0),0xE0), LEqual(And(WRF0,0xE0),0x80)))
		{
			// LPT is in ECP Mode.  Only report resources
			// to the OS if the SIO Device is not set to
			// Disabled in BIOS Setup.

			
			
			// Display PECP and return Status.

			If(WR30)
			{
				Return(0x000F)
			}
			Else
			{
				Return(0x000D)
			}
			
		}

		Return(0x0000)
	}

	// Disable Method for LPT-ECP Mode.

	Method(_DIS,0,Serialized)
	{
		// Set SIO to LPT.

		Store(1,WR07)

		If(LOr(LEqual(And(WRF0,0xE0),0xE0), LEqual(And(WRF0,0xE0),0x80)))
		{
			Store(0,WR30)
		}	
	}
	 
	// Current Resource Setting Method for LPT-ECP Mode.

	Method(_CRS,0,Serialized)
	{
		// Create a buffer to return to OS.

		Name(BUF0,ResourceTemplate()
		{
			IO(Decode16,0x0378,0x0378,0x01,0x08)	   
			IO(Decode16,0x0778,0x0778,0x01,0x08)
			IRQNoFlags(){7}
			DMA(Compatibility,NotBusMaster,Transfer8_16,){1}
		})

		// Skip this sequence if the LPT Device is not set
		// to ECP Mode.

		If(LOr(LEqual(And(WRF0,0xE0),0xE0), LEqual(And(WRF0,0xE0),0x80)))
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
	
			// Set SIO to LPT.

			Store(1,WR07)

			// Write IO and Length values into the Buffer.
		
			Store(WR61,IOL0)
			Store(WR60,IOH0)
			Store(IOL0,IOL1)
			Store(IOH0,IOH1)
			Store(IOL0,IOL2)
			Store(Add(0x4,IOH0),IOH2)
			Store(IOL0,IOL3)
			Store(Add(0x4,IOH0),IOH3)
			Store(8,LEN0)
			Store(8,LEN1)

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

	// Possible Resource Setting Method for LPT-ECP Mode.

		Name(_PRS,ResourceTemplate()
	{
			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x08)
				IO(Decode16,0x778,0x778,0x01,0x08)
				IRQNoFlags(){7}
				DMA(Compatibility,NotBusMaster,Transfer8_16){3}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x08)
				IO(Decode16,0x678,0x678,0x01,0x08)
				IRQNoFlags(){7}
				DMA(Compatibility,NotBusMaster,Transfer8_16){3}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x08)
				IO(Decode16,0x778,0x778,0x01,0x08)
				IRQNoFlags(){5}
				DMA(Compatibility,NotBusMaster,Transfer8_16){3}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x08)
				IO(Decode16,0x678,0x678,0x01,0x08)
				IRQNoFlags(){5}
				DMA(Compatibility,NotBusMaster,Transfer8_16){3}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x08)
				IO(Decode16,0x778,0x778,0x01,0x08)
				IRQNoFlags(){7}
				DMA(Compatibility,NotBusMaster,Transfer8_16){1}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x08)
				IO(Decode16,0x678,0x678,0x01,0x08)
				IRQNoFlags(){7}
				DMA(Compatibility,NotBusMaster,Transfer8_16){1}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x378,0x378,0x01,0x08)
				IO(Decode16,0x778,0x778,0x01,0x08)
				IRQNoFlags(){5}
				DMA(Compatibility,NotBusMaster,Transfer8_16){1}
			}

			StartDependentFn(0,2)
			{
				IO(Decode16,0x278,0x278,0x01,0x08)
				IO(Decode16,0x678,0x678,0x01,0x08)
				IRQNoFlags(){5}
				DMA(Compatibility,NotBusMaster,Transfer8_16){1}
			}

			EndDependentFn()
		})
		

	// Set Resource Setting Method for LPT-ECP Mode.

	Method(_SRS,1,Serialized) 
	{
		// Arg0 passed into _SRS is the buffer identical to BUF0.

		CreateByteField(Arg0,0x02,IOL0)
		CreateByteField(Arg0,0x03,IOH0)
		CreateWordField(Arg0,0x11,IRQW)
		CreateByteField(Arg0,0x14,DMA0)

		Store(1,WR07)

		// Disable the LPT.

		Store(0,WR30)

		
		// Set the Base I/O Address

          	Store(IOL0,WR61)
          	Store(IOH0,WR60)

		// Set the IRQ

		FindSetRightBit(IRQW,Local0)
		If (LNotEqual(IRQW,Zero))
		{
			Decrement(Local0)
		}
		Store(Local0,WR70)

		// Set the DMA Channel

		FindSetRightBit(DMA0,Local0)
		If (LNotEqual(DMA0,Zero))
		{
			Decrement(Local0)
		}
		Store(Local0,WR74)

		// Enable the LPT.

		Store(1,WR30)

		// Set to ECP Mode with EPP Mode 4 support.

		Or(WRF0,0xF0,WRF0)

	}

	// D0 Method for LPT-ECP Mode.

	Method(_PS0,0,Serialized)
	{
		Store(1,WR07)
		Store(1,WR30)
	}

	// D3 Method for LPT-ECP Mode.

	Method(_PS3,0,Serialized)
	{
		Store(1,WR07)
		Store(0,WR30)
   	}
}

} // End of Scope()
