/** @file
  I2C5 Sensors

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

External(\_SB.PCI0.LPCB.H_EC.KBCD, IntObj)
  
Scope (\_SB.PCI0.I2C5)
{
	Device(ACC)
	{
		Name(_ADR, Zero)
		Name(_HID, "BOSC0200")
		Name(_CID, "BOSC0200")
		Name(_DDN, "Accelerometer")
		Name(_UID, One)
		Name(_DEP, Package(1) {I2C5})
		Method(_CRS, 0, NotSerialized)
		{
			Name(RBUF, ResourceTemplate()
			{
				I2CSerialBus(0x0019, ControllerInitiated, 0x00061A80, AddressingMode7Bit, "\\_SB.PCI0.I2C5", 0x00, ResourceConsumer,,)
				I2CSerialBus(0x0018, ControllerInitiated, 0x00061A80, AddressingMode7Bit, "\\_SB.PCI0.I2C5", 0x00, ResourceConsumer,,)
			})
			Return(RBUF)
		}
		Method(ROMS, 0, NotSerialized)
		{
			Name(SBUF, Package(3) {"0 1 0", "-1 0 0", "0 0 -1"})
			Return(SBUF)
		}
		Method(ROMK, 0, NotSerialized)
		{
			Name(SBUF, Package(3) {"0 1 0", "-1 0 0", "0 0 -1"})
			Return(SBUF)
		}
		Method(CALS, 1, NotSerialized)
		{
			Return(0x14FFFFFFFFFF0102)
		}
		Method(CALK, 1, NotSerialized)
		{
			Return(0x14FFFFFFFFFF0102)
		}
		Method(ANGL, 1, NotSerialized)
		{
		}
		Method(SKBC, 1, NotSerialized)
		{
			If(LEqual(Arg0, Zero))
			{
				Store(Zero, ^^^SBRG.H_EC.KBCD)
			}
			Else
			{
				If(LEqual(Arg0, 0x03))
				{
					Store(0x03, ^^^SBRG.H_EC.KBCD)
				}
			}
		}
		Method(HALL, 1, NotSerialized)
		{
			Store(^^^SBRG.H_EC.LID0._LID(), Local0)
			If(LEqual(Local0, One))
			{
				Return(Zero)
			}
			Else
			{
				Return(One)
			}
		}
		Method(_STA, 0, NotSerialized)
		{
			Return(0x0F)
		}
	}
	Device(CMSR)
	{
		Name(_ADR, Zero)
		Name(_HID, "CPLM3218")
		Name(_CID, "CPLM3218")
		Name(_UID, One)
		Name(_DEP, Package(1) {I2C5})
		Method(_STA, 0, NotSerialized)
		{
			If(LEqual(CLSE, One))
			{
				Return(0x0F)
			}
			Return(Zero)
		}
		Method(_CRS, 0, NotSerialized)
		{
			Name(SBUF, ResourceTemplate()
			{
				I2CSerialBus(0x0048, ControllerInitiated, 0x00061A80, AddressingMode7Bit, "\\_SB.PCI0.I2C5", 0x00, ResourceConsumer,,)
				Interrupt(ResourceConsumer, Level, ActiveLow, Exclusive,,,)
				{
					0x0000003A
				}
			})
			Return(SBUF)
		}
		Name(CPM0, Package(7) {0x7DB5, One, 0x0F, 0x0840, 0x013B, 0x011D, Zero})
		Name(CPM1, Package(3) {0x0898, 0x000186A0, 0x000186A0})
		Name(CPM2, Package(6) {0x61A8, 0x000186A0, 0x000186A0, 0x000186A0, 0x00030D40, 0x00186A00})
		Name(CPM3, Package(4)
		{
			Package(2) {0x3C, 0x0A}, 
			Package(2) {0x64, 0x0B}, 
			Package(2) {0x64, 0x1F40}, 
			Package(2) {0x01F4, 0x1F41}
		})
		Name(CPM5, Package(7) {Zero, 0x000249F0, 0x0190, 0x03E8, 0x02, 0x32, 0x05})
		Method(_DSM, 4, NotSerialized)
		{
			If(LEqual(Arg0, ToUUID("0703C6B6-1CCA4144-9FE74654F53A0BD9")))
			{
				If(LEqual(Arg2, Zero))
				{
					Return(Buffer(One)
					{
						0x03
					})
				}
				If(LEqual(Arg2, One))
				{
					If(LEqual(ToInteger(Arg3), Zero))
					{
						Return(CPM0)
					}
					If(LEqual(ToInteger(Arg3), One))
					{
						Return(CPM1)
					}
					If(LEqual(ToInteger(Arg3), 0x02))
					{
						Return(CPM2)
					}
					If(LEqual(ToInteger(Arg3), 0x03))
					{
						Return(CPM3)
					}
					If(LEqual(ToInteger(Arg3), 0x05))
					{
						Return(CPM5)
					}
				}
			}
			Return(Buffer(One)
			{
				0x00
			})
		}
	}		
}