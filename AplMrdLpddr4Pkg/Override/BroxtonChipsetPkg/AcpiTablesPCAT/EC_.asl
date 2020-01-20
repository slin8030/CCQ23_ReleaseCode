Device(H_EC)
		{
			Name(_HID, EISAID("PNP0C09"))
			Name(ECFG, Zero)
			Name(WIBT, Zero)
			Name(_UID, One)
			Name(APST, Zero)
			Name(ECON, One)
			Name(B1CC, Zero)
			Name(B2CC, Zero)
			Name(B2ST, Zero)
			Name(CFAN, Zero)
			Name(CMDR, Zero)
			Name(DOCK, Zero)
			Name(EJET, Zero)
			Name(MCAP, Zero)
			Name(PLMX, Zero)
			Name(PECH, Zero)
			Name(PECL, Zero)
			Name(PENV, Zero)
			Name(PINV, Zero)
			Name(PPSH, Zero)
			Name(PPSL, Zero)
			Name(PSTP, Zero)
			Name(RPWR, Zero)
			Name(LIDS, Zero)
			Name(SLPC, Zero)
			Name(VPWR, Zero)
			Name(WTMS, Zero)
			Name(AWT2, Zero)
			Name(AWT1, Zero)
			Name(AWT0, Zero)
			Name(DLED, Zero)
			Name(IBT1, Zero)
			Name(ECAV, One)
			Name(SPT2, Zero)
			Name(PB10, Zero)
			Name(IWCW, Zero)
			Name(IWCR, Zero)
			Name(BTEN, Zero)
			Mutex(ECMT, 0)
			Method(_CRS, 0, NotSerialized)
			{
				Name(BFFR, ResourceTemplate()
				{
					IO(Decode16, 0x0062, 0x0062, 0x00, 0x01)
					IO(Decode16, 0x0066, 0x0066, 0x00, 0x01)
				})
				Return(BFFR)
			}
			Method(_STA, 0, NotSerialized)
			{
				Store(0x03, ^^^GFX0.CLID)
				If(LEqual(ECON, One))
				{
					Return(0x0F)
				}
				Return(Zero)
			}
			Scope(\)
			{
				Field(GNVS, AnyAcc, Lock, Preserve)
				{
					Offset(0x21),	//Offset(33),
					B2SC, 8,
					Offset(0x24),	//Offset(36),
					B2SS, 8,
				}
			}
			OperationRegion(ECF2, EmbeddedControl, Zero, 0xFF)
			Field(ECF2, ByteAcc, Lock, Preserve)
			{
				XXX0, 8,
				XXX1, 8,
				XXX2, 8,
				Offset(0x11),	//Offset(17),
				KBCD, 8,
				Offset(0x20),	//Offset(32),
				RCMD, 8,
				RCST, 8,
				Offset(0x60),	//Offset(96),
				TSR1, 8,
				TSR2, 8,
				TER4, 8,
				TSI, 4,
				HYST, 4,
				TSHT, 8,
				TSLT, 8,
				TSSR, 8,
				CHGR, 16,
				Offset(0x72),	//Offset(114),
				TER2, 8,
				Offset(0x7F),	//Offset(127),
				LSTE, 1,
				, 7,
				ECWR, 8,
				XX10, 8,
				XX11, 16,
				B1DC, 16,
				B1FV, 16,
				B1FC, 16,
				XX15, 16,
				B1ST, 8,
				B1CR, 16,
				B1RC, 16,
				B1VT, 16,
				BPCN, 8,
				Offset(0xC0),	//Offset(192),
				VER1, 8,
				VER2, 8,
				RSV1, 8,
				RSV2, 8,
				CCI0, 8,
				CCI1, 8,
				CCI2, 8,
				CCI3, 8,
				CTL0, 8,
				CTL1, 8,
				CTL2, 8,
				CTL3, 8,
				CTL4, 8,
				CTL5, 8,
				CTL6, 8,
				CTL7, 8,
				MGI0, 8,
				MGI1, 8,
				MGI2, 8,
				MGI3, 8,
				MGI4, 8,
				MGI5, 8,
				MGI6, 8,
				MGI7, 8,
				MGI8, 8,
				MGI9, 8,
				MGIA, 8,
				MGIB, 8,
				MGIC, 8,
				MGID, 8,
				MGIE, 8,
				MGIF, 8,
				MGO0, 8,
				MGO1, 8,
				MGO2, 8,
				MGO3, 8,
				MGO4, 8,
				MGO5, 8,
				MGO6, 8,
				MGO7, 8,
				MGO8, 8,
				MGO9, 8,
				MGOA, 8,
				MGOB, 8,
				MGOC, 8,
				MGOD, 8,
				MGOE, 8,
				MGOF, 8,
			}
			Method(ECMD, 1, Serialized)
			{
			}
			Method(ECWT, 2, Serialized)
			{
				Store(Acquire(ECMT, 0x03E8), Local0)
				If(LEqual(Local0, Zero))
				{
					If(ECAV)
					{
						Store(Arg0, Arg1)
					}
					Release(ECMT)
				}
			}
			Method(ECRD, 1, Serialized)
			{
				Store(Acquire(ECMT, 0x03E8), Local0)
				If(LEqual(Local0, Zero))
				{
					If(ECAV)
					{
						Store(DerefOf(Arg0), Local1)
						Release(ECMT)
						Return(Local1)
					}
					Else
					{
						Release(ECMT)
					}
				}
			}
			Method(_GPE, 0, NotSerialized)
			{
				Store(0x2C, Local0)
				Return(Local0)
			}
			Method(_Q06, 0, NotSerialized)
			{
				^^^^HIDD.HPEM(0x14)
			}
			Method(_Q07, 0, NotSerialized)
			{
				^^^^HIDD.HPEM(0x13)
			}
			Method(_Q54, 0, NotSerialized)
			{
				P8XH(Zero, 0x54)
				ADBG("PB Sleep 0x80")
				If(CondRefOf(\_SB.PWRB))
				{
					Notify(PWRB, 0x80)
				}
			}
			Method(_Q79, 0, NotSerialized)
			{
				Store(MGI0, ^^^^UBTC.MGI0)
				Store(MGI1, ^^^^UBTC.MGI1)
				Store(MGI2, ^^^^UBTC.MGI2)
				Store(MGI3, ^^^^UBTC.MGI3)
				Store(MGI4, ^^^^UBTC.MGI4)
				Store(MGI5, ^^^^UBTC.MGI5)
				Store(MGI6, ^^^^UBTC.MGI6)
				Store(MGI7, ^^^^UBTC.MGI7)
				Store(MGI8, ^^^^UBTC.MGI8)
				Store(MGI9, ^^^^UBTC.MGI9)
				Store(MGIA, ^^^^UBTC.MGIA)
				Store(MGIB, ^^^^UBTC.MGIB)
				Store(MGIC, ^^^^UBTC.MGIC)
				Store(MGID, ^^^^UBTC.MGID)
				Store(MGIE, ^^^^UBTC.MGIE)
				Store(MGIF, ^^^^UBTC.MGIF)
				Store(CCI0, ^^^^UBTC.CCI0)
				Store(CCI1, ^^^^UBTC.CCI1)
				Store(CCI2, ^^^^UBTC.CCI2)
				Store(CCI3, ^^^^UBTC.CCI3)
				Store(Zero, CCI0)
				Store(Zero, CCI3)
				Notify(UBTC, 0x80)
			}
			Method(_QD5, 0, NotSerialized)
			{
				P8XH(Zero, 0xD5)
				PWPR()
			}
			Method(_QD6, 0, NotSerialized)
			{
				P8XH(Zero, 0xD6)
				PWRR()
			}
			Method(_QF0, 0, NotSerialized)
			{
				If(LEqual(DBGS, Zero))
				{
					If(CondRefOf(\_TZ.TZ01))
					{
						Notify(\_TZ.TZ01, 0x80)
					}
				}
			}
			Device(ADP1)
			{
				Name(_HID, "ACPI0003")
				Method(_STA, 0, NotSerialized)
				{
					If(LEqual(ECON, One))
					{
						Return(0x0F)
					}
					Return(Zero)
				}
				Method(_PSR, 0, NotSerialized)
				{
					If(And(ECWR, One))
					{
						Store(One, PWRS)
					}
					Else
					{
						Store(Zero, PWRS)
					}
					Return(PWRS)
				}
				Method(_PCL, 0, NotSerialized)
				{
					Return(Package(1) {_SB})
				}
			}
			Device(BAT0)
			{
				Name(_HID, EISAID("PNP0C0A"))
				Name(_UID, One)
				Method(_STA, 0, NotSerialized)
				{
					If(And(ECWR, 0x02))
					{
						Return(0x1F)
					}
					Return(Zero)
				}
				Method(_BIF, 0, NotSerialized)
				{
					Name(BPKG, Package(13)
					{
						Zero, Ones, Ones, One, Ones, Zero, Zero, 0x64, Zero, "SR Real Battery",
						"123456789", "Real", "Intel SR 1"
					})
					Store(Multiply(B1DC, 0x0A), Index(BPKG, One))
					Store(Multiply(B1FC, 0x0A), Index(BPKG, 0x02))
					If(B1FC)
					{
						Store(Divide(Multiply(B1FC, 0x0A), 0x0A, , ), Index(BPKG, 0x05))
						Store(Divide(Multiply(B1FC, 0x0A), 0x19, , ), Index(BPKG, 0x06))
						Store(Divide(Multiply(B1DC, 0x0A), 0x64, , ), Index(BPKG, 0x07))
					}
					Return(BPKG)
				}
				Method(_BST, 0, NotSerialized)
				{
					Name(PKG1, Package(4) {Ones, Ones, Ones, Ones})
					Store(And(B1ST, 0x07), Index(PKG1, Zero))
					If(And(B1ST, One))
					{
						Store(Multiply(B1CR, B1FV), Local0)
						Store(Divide(Local0, 0x03E8, , ), Local0)
						Store(Local0, Index(PKG1, One))
					}
					Else
					{
						Store(Multiply(B1CR, B1FV), Local0)
						Store(Divide(Local0, 0x03E8, , ), Local0)
						Store(Local0, Index(PKG1, One))
					}
					Store(Multiply(B1RC, 0x0A), Index(PKG1, 0x02))
					Store(B1FV, Index(PKG1, 0x03))
					Return(PKG1)
				}
				Method(_PCL, 0, NotSerialized)
				{
					Return(_SB)
				}
			}
			Device(BAT1)
			{
				Name(_HID, EISAID("PNP0C0A"))
				Name(_UID, One)
				Method(_STA, 0, NotSerialized)
				{
					Return(Zero)
				}
			}
			Device(BAT2)
			{
				Name(_HID, EISAID("PNP0C0A"))
				Name(_UID, 0x02)
				Method(_STA, 0, NotSerialized)
				{
					Return(Zero)
				}
			}
			Method(_REG, 2, NotSerialized)
			{
				If(LAnd(LEqual(Arg0, 0x03), LEqual(Arg1, One)))
				{
					Store(One, ECAV)
					Store(Zero, BNUM)
					Or(BNUM, ShiftRight(And(ECRD(RefOf(ECWR)), 0x02), One), BNUM)
					Store(PWRS, Local0)
					If(LEqual(BNUM, Zero))
					{
						Store(ECRD(RefOf(VPWR)), PWRS)
					}
					Else
					{
						Store(And(ECRD(RefOf(ECWR)), One), PWRS)
					}
					PNOT()
				}
			}
			Method(_Q0A, 0, NotSerialized)
			{
				Store(0x0A, P80H)
				If(And(ECWR, One))
				{
					Store(One, PWRS)
				}
				Else
				{
					Store(Zero, PWRS)
				}
				Sleep(0x01F4)
				Notify(BAT0, 0x81)
				Sleep(0x01F4)
				Notify(ADP1, 0x80)
			}
			Method(_Q0B, 0, NotSerialized)
			{
				Store(0x0B, P80H)
				Sleep(0x01F4)
				Notify(BAT0, 0x81)
				Sleep(0x01F4)
				Notify(BAT0, 0x80)
			}
			Method(_Q0C, 0, NotSerialized)
			{
				P8XH(Zero, 0x0C)
				Store(Zero, LIDS)
				^^^GFX0.GLID(LIDS)
				Notify(LID0, 0x80)
			}
			Method(_Q0D, 0, NotSerialized)
			{
				P8XH(Zero, 0x0D)
				Store(One, LIDS)
				^^^GFX0.GLID(LIDS)
				Notify(LID0, 0x80)
			}
			Device(LID0)
			{
				Name(_HID, EISAID("PNP0C0D"))
				Method(_STA, 0, NotSerialized)
				{
					Return(0x0F)
				}
				Method(_PRW, 0, NotSerialized)
				{
					Return(GPRW(0x0E, 0x03))
				}
				Method(_LID, 0, NotSerialized)
				{
					If(LEqual(ECRD(RefOf(LSTE)), One))
					{
						Store(0x03, ^^^^GFX0.CLID)
						Return(One)
					}
					Else
					{
						Store(Zero, ^^^^GFX0.CLID)
						Return(Zero)
					}
				}
			}
		}
	}
Scope(_SB)
	{
		Device(PWRB)
		{
			Name(_HID, EISAID("PNP0C0C"))
			Name(PBST, One)
			Method(_STA, 0, NotSerialized)
			{
				Return(0x0F)
			}
		}
		Device(HIDD)
		{
			Name(_HID, "INT33D5")
			Name(HBSY, Zero)
			Name(HIDX, Zero)
			Name(HMDE, Zero)
			Name(HRDY, Zero)
			Name(BTLD, Zero)
			Name(BTS1, Zero)
			Name(HEB1, 0x000233F3)
			Name(HEB2, Zero)
			Method(_STA, 0, Serialized)
			{
				Return(0x0F)
			}
			Method(HDDM, 0, Serialized)
			{
				Name(DPKG, Package(4) {0x11111111, 0x22222222, 0x33333333, 0x44444444})
				Return(DPKG)
			}
			Method(HDEM, 0, Serialized)
			{
				Store(Zero, HBSY)
				If(LEqual(HMDE, Zero))
				{
					Return(HIDX)
				}
				Return(HMDE)
			}
			Method(HDMM, 0, Serialized)
			{
				Return(HMDE)
			}
			Method(HDSM, 1, Serialized)
			{
				Store(Arg0, HRDY)
			}
			Method(HPEM, 1, Serialized)
			{
				Store(One, HBSY)
				If(LEqual(HMDE, Zero))
				{
					Store(Arg0, HIDX)
				}
				Else
				{
					Store(Arg0, HIDX)
				}
				Notify(HIDD, 0xC0)
				Store(Zero, Local0)
				While(LAnd(LLess(Local0, 0xFA), HBSY))
				{
					Sleep(0x04)
					Increment(Local0)
				}
				If(LEqual(HBSY, One))
				{
					Store(Zero, HBSY)
					Store(Zero, HIDX)
					Return(One)
				}
				Else
				{
					Return(Zero)
				}
			}
			Method(BTNL, 0, Serialized)
			{
				If(CondRefOf(\_SB.PWRB.PBST))
				{
					Store(Zero, ^^PWRB.PBST)
					Notify(PWRB, One)
				}
				Store(One, BTLD)
				^^PCI0.SBRG.H_EC.ECWT(One, RefOf(^^PCI0.SBRG.H_EC.PB10))
				Store(0x1F, BTS1)
				^^PCI0.SBRG.H_EC.ECWT(BTS1, RefOf(^^PCI0.SBRG.H_EC.BTEN))
			}
			Method(BTNE, 1, Serialized)
			{
				Store(Or(And(Arg0, 0x1E), One), BTS1)
				^^PCI0.SBRG.H_EC.ECWT(BTS1, RefOf(^^PCI0.SBRG.H_EC.BTEN))
			}
			Method(BTNS, 0, Serialized)
			{
				Store(^^PCI0.SBRG.H_EC.ECRD(RefOf(^^PCI0.SBRG.H_EC.BTEN)), BTS1)
				Return(BTS1)
			}
			Method(BTNC, 0, Serialized)
			{
				Return(0x1F)
			}
			Method(HEBC, 0, Serialized)
			{
				Return(HEB1)
			}
			Method(HEEC, 0, Serialized)
			{
				Return(HEB2)
			}
		}
		Method(PWPR, 0, Serialized)
		{
			Notify(HIDD, 0xCE)
		}
		Method(PWRR, 0, Serialized)
		{
			Notify(HIDD, 0xCF)
		}
	}
	Scope(_SB.PCI0)
	{
		Device(CONV)
		{
			Method(_HID, 0, NotSerialized)
			{
				Return("ID9001")
			}
			Name(_CID, "PNP0C60")
		}
	}