//[-start-160831-IB15590011-modify]// 
#define EC_CMD_TIMEOUT	1000
External(\_SB.PCI0.GFX0.CLID)
External(\_SB.UBTC, DeviceObj)
External(\_SB.UBTC.MGI0, IntObj)
External(\_SB.UBTC.MGI1, IntObj)
External(\_SB.UBTC.MGI2, IntObj)
External(\_SB.UBTC.MGI3, IntObj)
External(\_SB.UBTC.MGI4, IntObj)
External(\_SB.UBTC.MGI5, IntObj)
External(\_SB.UBTC.MGI6, IntObj)
External(\_SB.UBTC.MGI7, IntObj)
External(\_SB.UBTC.MGI8, IntObj)
External(\_SB.UBTC.MGI9, IntObj)
External(\_SB.UBTC.MGIA, IntObj)
External(\_SB.UBTC.MGIB, IntObj)
External(\_SB.UBTC.MGIC, IntObj)
External(\_SB.UBTC.MGID, IntObj)
External(\_SB.UBTC.MGIE, IntObj)
External(\_SB.UBTC.MGIF, IntObj)
External(\_SB.UBTC.CCI0, IntObj)
External(\_SB.UBTC.CCI1, IntObj)
External(\_SB.UBTC.CCI2, IntObj)
External(\_SB.UBTC.CCI3, IntObj)
External(\_SB.UBTC.STS0, IntObj)
External(\_SB.UBTC.STS1, IntObj)
External(\_SB.UBTC.STS2, IntObj)
External(\_SB.UBTC.STS3, IntObj)
External(\_SB.UBTC.STS4, IntObj)
External(\_SB.UBTC.STS5, IntObj)
External(\_SB.UBTC.STS6, IntObj)
External(\_SB.UBTC.STS7, IntObj)
External(\_SB.PCI0.GFX0.GLID, MethodObj)

Device(H_EC)
{
  Name(_HID, EISAID("PNP0C09"))
  Name(ECFG, Zero)
  Name(WIBT, Zero)
  Name(_UID, One)
  Name(APST, Zero)
//  Name(ECON, One) //AC debug
  Name(B1CC, 0)
  Name(B2CC, 0)
  Name(B2ST, 0)
  Name(CFAN, 0)
  Name(CMDR, 0)
  Name(DOCK, 0)
  Name(EJET, 0)
  Name(MCAP, 0)
  Name(PLMX, 0)
  Name(PECH, 0)
  Name(PECL, 0)
  Name(PENV, 0)
  Name(PINV, 0)
  Name(PPSH, 0)
  Name(PPSL, 0)
  Name(PSTP, 0)
  Name(RPWR, 0)
  Name(LIDS, 0)
  Name(SLPC, 0)
  Name(VPWR, 0)
  Name(WTMS, 0)
  Name(AWT2, 0)
  Name(AWT1, 0)
  Name(AWT0, 0)
  Name(DLED, 0)
  Name(IBT1, 0)
  Name(ECAV, 1)   //Support DPTF feature
  Name(SPT2, 0)
  Name(PB10, 0)
  Name(IWCW, 0)
  Name(IWCR, 0)
  Name(BTEN, 0)
  
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
    //Store(3, \_SB.PCI0.GFX0.CLID)
    If(LEqual(ECON, One))
    {
      Return(0x0F)
    }
    Return(Zero)
  }
    	    	
  Scope(\)
  {
    // these fields come from the Global NVS area
    Field(GNVS,AnyAcc,Lock,Preserve)
    {
      Offset(33),
      B2SC, 8,    //   (33) Battery 2 Stored Capacity
      Offset(36),
      B2SS, 8 //   (36) Battery 2 Stored Status
    }
  } // end Scope(\)    	
    	
  OperationRegion(ECF2, EmbeddedControl, Zero, 0xFF)
  Field(ECF2, ByteAcc, Lock, Preserve)
  {	
    //Offset	Description
    XXX0, 8,			    //0x00		EC Firmware main- version number.
    XXX1, 8,			    //0x01		EC Firmware sub- version number.
    XXX2, 8,			    //0x02		EC Firmware test- version number.
//[-start-170323-IB15590024-modify]// 
	Offset(0x11), 
    KBCD, 8,          //0x11    ???
 //[-end-170323-IB15590024-modify]//    
	Offset(0x20),
    RCMD, 8,				//0x20		Same function as IO 66 port to send EC command
    RCST, 8, 				//0x21		Report status for the result of command execution
    //LID status change to offset 0x2F
                    //-Offset(0x2F),   
	//Offset(0x7F), //Support LID feature
                    //-LSTE, 1,            //BIT0      LID GPI
                    //- , 7,               //Reserved
                 
    Offset(0x60),
	TSR1, 8,      			//0x60      Thermal Sensor Register 1 [CPU VR (IMVP) Temp on RVP]
	TSR2, 8,      			//0x61      Thermal Sensor Register 2 [Heat exchanger fan temp on RVP]
	TER4, 8,      			//0x62      Thermal Sensor Register 3 (skin temperature)
	Offset(0x63),   		//0x63-66   DPTF fields
	TSI,  4,				//0x63.0~3	(set value =2)
									//          # 0 = SEN1 - CPU VR temperature sensor
            						//          # 1 = SEN2 - Heat Exchanger temperature sensor
           							//          # 2 = SEN3 - Skin temperature sensor
            						//          # 3 = SEN4 - Ambient temperature sensor
            						//          # 4 = SEN5 - DIMM temperature sensor [IR sensor 1 on WSB]
            						//          # 5 = SEN6 - not used on RVP
	HYST, 4,     			//0x63.4-7 Hysteresis in degC, hysteresis selection is global and meant for all sensors. (Set default value =2)
	TSHT, 8,      			//0x64      Thermal Sensor (N) high trip point(set default value =70)
	TSLT, 8,				//0x65      Thermal Sensor (N) low trip point (set default value =70)
	TSSR, 8,     			//0x66      TSSR- thermal sensor status register (set bit2 =1)
									//         TSSR bits defined:
                  					//          BIT0:  SEN1 - CPU VR Temp Sensor Trip Flag
                  					//          BIT1:  SEN2 - Fan Temp Sensor Trip Flag
                  					//          BIT2:  SEN3 - Skin Temp Sensor Trip Flag
                  					//          BIT3:  SEN4 - Ambient Temp Sensor Trip Flag
                  					//          BIT4:  Reserved
                  					//          BIT5:  Reserved
                  					//          BIT6:  Reserved
                  					//          BIT7:  Reserved
    CHGR, 16,         	    //0x67~68   Charge Rate //Support DPTF feature
	Offset(0x72),   	 	//0x72  
	TER2, 8,				// Charger Temperature, Charger thermistor support 
	Offset(0x7F),	        //Offset(0x7F), //Support LID feature
		LSTE, 1,			//BIT0		LID GPI
		    , 7,			//Reserved
    Offset(0x80),	        //Offset(0x80), //Support DPTF feature
    ECWR, 8,			    //0x80		AC & Battery status
    XX10, 8,			    //0x81		Battery#1 Model Number Code
    XX11, 16,			    //0x82~83	Battery#1 Serial Number
    B1DC, 16,			    //0x84~85	Battery#1 Design Capacity
    B1FV, 16,			    //0x86~87	Battery#1 Design Voltage
    B1FC, 16,			    //0x88~89	Battery#1 Last Full Charge Capacity
    XX15, 16,			    //0x8A~8B	Battery#1 Trip Point
    B1ST, 8,			    //0x8C		Battery#1 State
    B1CR, 16,			    //0x8D~8E	Battery#1 Present Rate	
    B1RC, 16,			    //0x8F~90	Battery#1 Remaining Capacity
    B1VT, 16,			    //0x91~92	Battery#1 Present Voltage
    BPCN, 8,			    //0x93          Battery#1 Remaining percentage
    		
    // USB Type C Mailbox Interface  //	PPM->OPM Message In
    Offset(0xc0),
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
      MGI0, 8,  // 0xc0
      MGI1, 8,  // 0xc1
      MGI2, 8,  // 0xc2
      MGI3, 8,  // 0xc3
      MGI4, 8,  // 0xc4
      MGI5, 8,  // 0xc5
      MGI6, 8,  // 0xc6
      MGI7, 8,  // 0xc7
      MGI8, 8,  // 0xc8
      MGI9, 8,  // 0xc9
      MGIA, 8,  // 0xcA
      MGIB, 8,  // 0xcB
      MGIC, 8,  // 0xcC
      MGID, 8,  // 0xcD
      MGIE, 8,  // 0xcE
      MGIF, 8,  // 0xcF
      // USB Type C Mailbox Interface  //	OPM->PPM Message Out
      MGO0, 8,  // 0xD0		
      MGO1, 8,  // 0xD1
      MGO2, 8,  // 0xD2
      MGO3, 8,  // 0xD3
      MGO4, 8,  // 0xD4
      MGO5, 8,  // 0xD5
      MGO6, 8,  // 0xD6
      MGO7, 8,  // 0xD7
      MGO8, 8,  // 0xD8
      MGO9, 8,  // 0xD9
      MGOA, 8,  // 0xDA
      MGOB, 8,  // 0xDB
      MGOC, 8,  // 0xDC
      MGOD, 8,  // 0xDD
      MGOE, 8,  // 0xDE
      MGOF, 8,  // 0xDF    		    
  }
    	    	
  //SECC -- Send EC Command

  
  //APL_MRD >>

  //Support DPTF feature  	
  Method(ECMD,1,Serialized)
  {
  	//SECC(Arg0)	
  }
  
  Method(ECWT,2,Serialized,,,{IntObj, FieldUnitObj})
  {
    Store (Acquire(ECMT, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
    If (LEqual(Local0, Zero))  // check for Mutex acquired
    {
      If (ECAV) 
	  {
        Store(Arg0,Arg1) // Execute Write to EC
      } // If (ECAV)
      Release(ECMT)
    } // If EC Acquired
  }
 
  Method(ECRD,1,Serialized, 0, IntObj, FieldUnitObj)
  {
    Store (Acquire(ECMT, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
    If (LEqual(Local0, Zero))  // check for Mutex acquired
    {
      If (ECAV) 
	  {
        Store(DerefOf (Arg0), Local1) // Execute Read from EC
        Release(ECMT)
        Return(Local1)
      }
      Else
      {
        Release(ECMT)
      } // If (ECAV)
    } // If EC Acquired
  }
  
  Method(_GPE)
  {
    Store (0x2C,  Local0)   // GPIO_11 --> GPE_2B
    return (Local0)
  }
  Method(_Q06, 0, NotSerialized)
  {
     \_SB.HIDD.HPEM(0x14)
  }
  Method(_Q07, 0, NotSerialized)
  {
     \_SB.HIDD.HPEM(0x13)
  }
  Method(_Q54)    // Power Button Event for Control method Power Button(10sec PB Override without V-GPIO driver)
  {
    P8XH(0,0x54)
    ADBG("PB Sleep 0x80")
    If (CondRefOf(\_SB.PWRB))
    {
      Notify(\_SB.PWRB, 0x80)
    }
  } // end Method(_Q54) Power button event.
  Method(_Q79, 0, NotSerialized)
  {
    Store( MGI0, \_SB.UBTC.MGI0 )
    Store( MGI1, \_SB.UBTC.MGI1 )
    Store( MGI2, \_SB.UBTC.MGI2 )
    Store( MGI3, \_SB.UBTC.MGI3 )
    Store( MGI4, \_SB.UBTC.MGI4 )
    Store( MGI5, \_SB.UBTC.MGI5 )
    Store( MGI6, \_SB.UBTC.MGI6 )
    Store( MGI7, \_SB.UBTC.MGI7 )
    Store( MGI8, \_SB.UBTC.MGI8 )
    Store( MGI9, \_SB.UBTC.MGI9 )
    Store( MGIA, \_SB.UBTC.MGIA )
    Store( MGIB, \_SB.UBTC.MGIB )
    Store( MGIC, \_SB.UBTC.MGIC )
    Store( MGID, \_SB.UBTC.MGID )
    Store( MGIE, \_SB.UBTC.MGIE )
    Store( MGIF, \_SB.UBTC.MGIF )
    Store( CCI0, \_SB.UBTC.CCI0 )
    Store( CCI1, \_SB.UBTC.CCI1 )
    Store( CCI2, \_SB.UBTC.CCI2 )
    Store( CCI3, \_SB.UBTC.CCI3 )
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
  //APL_MRD <<	
	
  Device(ADP1)
  {
    Name(_HID,"ACPI0003")

    Method(_STA)
    {
      //0x80, BIT0, Adapter status/ 0:off, 1: on
      If(LEqual(ECON, One)) 
	    {
        Return(0x0F)
      }
      Return(0x00)
    }

    // Return the value that determines if running
    // from AC or not.

    Method(_PSR,0)
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

    // Return that everything runs off of AC.

    Method(_PCL,0)
    {
      Return (
        Package() { _SB }
      )
    }
  }
  
  Device(BAT0)
  {
    Name(_HID, EISAID("PNP0C0A"))
    Name(_UID,1)
    Method(_STA, 0, NotSerialized)
    {
      //0x80, BIT1, Battery status/ 0:not present, 1: present
      If(And(ECWR, 0x02))
      {
         Return(0x1F)//BAT0 _STA return 0 can boot to OS
      }
      Return(0x00)
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
    Name(_HID,EISAID("PNP0C0A"))

    Name(_UID,1)
    
    Method(_STA,0)
    {
      Return(0)       // Hide device 
    }
  }
  
  Device(BAT2)
  {
    Name(_HID,EISAID("PNP0C0A"))

    Name(_UID,2)
    
    Method(_STA,0)
    {
      Return(0)       // Hide device 
    }
  }
    	
  Method(_REG, 2, NotSerialized)
  {
    If (LAnd(LEqual(Arg0,3),LEqual(Arg1,1)))
    {
      Store(One, ECAV)
              
      // Unconditionally fix up the Battery and Power State.

      // Initialize the Number of Present Batteries.
      //  1 = Real Battery 1 is present
      //  2 = Real Battery 2 is present
      //  3 = Real Battery 1 and 2 are present

      Store(0,BNUM)
      Or(BNUM,ShiftRight(And(ECRD(RefOf(ECWR)),0x02),1),BNUM)

      // Save the current Power State for later.

      Store(PWRS,Local0)

      // Initialize the Power State.
      //  BNUM = 0 = Virtual Power State
      //  BNUM > 0 = Real Power State

      If (LEqual(BNUM,0))
      {
        Store(ECRD(RefOf(VPWR)),PWRS)
        //ECRD(RefOf(VPWR)),\_PR.POWS)
      }
      Else
      {
        Store(And(ECRD(RefOf(ECWR)),0x01),PWRS)
        //(And(ECRD(RefOf(ECWR)),0x01),\_PR.POWS)
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
    //Fix issue "AC/DC switch slowly after S3 or Reboot"
    //Add 500ms delay before notify battery in QEvent
    Sleep(0x01F4)
    Notify(BAT0, 0x81)
    Sleep(0x01F4)
    Notify(ADP1, 0x80)
    //FPPC()
  }
  Method(_Q0B, 0, NotSerialized)
  {
    Store(0x0B, P80H)
    //Store(Zero, BNUM)
    //If(And(ECWR, 0x02))
    //{
    //	Store(One, BNUM)
    //}
    //Add 500ms delay before notify battery in QEvent
    Sleep(0x01F4)
    Notify(BAT0, 0x81)
    Sleep(0x01F4)
    Notify(BAT0, 0x80)   	
  }
    	
  Method(_Q0C)    // Lid Close Event.
  {
    P8XH(0,0x0C)
    Store(0, LIDS)
    //Store(3, LIDS)
    \_SB.PCI0.GFX0.GLID(LIDS)
    Notify(\_SB.PCI0.LPCB.H_EC.LID0,0x80)
  }
	  
  Method(_Q0D)    // Lid Open Event.
  {
    P8XH(0,0x0D)
    Store(1, LIDS)
    //Store(0, LIDS)
    \_SB.PCI0.GFX0.GLID(LIDS)
    Notify(\_SB.PCI0.LPCB.H_EC.LID0,0x80)
  }
	  
	  
  
  Device(LID0)
  {
    Name(_HID,EISAID("PNP0C0D"))

    Method(_STA)
    {
//[-start-171226-IB07400935-modify]/
      If (LEqual (ECON, 1)){ 
        Return(0x0F)
      }
      Return(0x00)
//[-end-171226-IB07400935-modify]//
    }
    //need porting...........    
    //Method(_PRW, 0) { Return(GPRW(0x0E, 3)) }
    Method(_PRW, 0) { Return(GPRW(0x0E, 3)) }
    
    Method(_LID,0)
    {
      // 0 = Closed, 1 = Open.
      // Return(\_SB.GGIV(GPIO_SKL_LP_GPP_C8))
	    If (LEqual(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.LSTE)),1))
	    {
	      //Return(0)
	      Store(3,\_SB.PCI0.GFX0.CLID)
	      Return(1)
	    } 
	    Else 
	    {
	      //Return(1)
	      Store(0,\_SB.PCI0.GFX0.CLID)
	      Return(0)
	    }
    }
  }
  
//  Device(CIND)  // Convertible Indicators.
//  {
//     Name(_HID, "INT33D3")
//     Name(_CID, "PNP0C60")
//
//     Method(_STA, 0,Serialized)
//     {
//       If (LAnd(And(IUCE,1), LGreaterEqual(OSYS, 2012)))
//       {
//         Return(0x0F)
//       }
//       Return(0x00)
//     }
//  }
//
//  Device(DIND)  // Docking Indicators.
//  {
//     Name(_HID, "INT33D4")
//     Name(_CID, "PNP0C70")
//     Method(_STA, 0,Serialized)
//     {
//       If (LAnd(And(IUDE,1), LGreaterEqual(OSYS, 2012)))
//       {
//         Return(0x0F)
//       }
//       Return(0x00)
//     }
//  }
  

}//Device(H_EC)

Scope(\_SB)
{
  Device(PWRB)
  {
    Name(_HID,EISAID("PNP0C0C"))

    // GPI14 = GPE30 = Waketime SCI.  The PRW isn't working when
    // placed in any of the logical locations ( PS2K, PS2M,
    // H_EC ), so a Power Button Device was created specifically
    // for the WAKETIME_SCI PRW.

    //    Name(_PRW, Package(){30,4})
    //
    // Power button status flag used to communicate H_EC.VBDL
    //
    Name(PBST, 1)
    
    Method(_STA)
    {
//[-start-171226-IB07400935-modify]//
      If (LEqual (ECON, 1)){ 
        Return(0x0F)
      }
      Return(0x00)
//[-end-171226-IB07400935-modify]//
    }
  }//end device PWRB
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
//[-start-171226-IB07400935-modify]//
      If (LEqual (ECON, 1)){ 
        Return(0x0F)
      }
      Return(0x00)
//[-end-171226-IB07400935-modify]//
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
  			Store(Zero, \_SB.PWRB.PBST)
  			Notify(PWRB, One)
  		}
  		Store(One, BTLD)
  		\_SB.PCI0.LPCB.H_EC.ECWT(One, RefOf(\_SB.PCI0.LPCB.H_EC.PB10))
  		Store(0x1F, BTS1)
  		\_SB.PCI0.LPCB.H_EC.ECWT(BTS1, RefOf(\_SB.PCI0.LPCB.H_EC.BTEN))
  	}
  	Method(BTNE, 1, Serialized)
  	{
  		Store(Or(And(Arg0, 0x1E), One), BTS1)
  		\_SB.PCI0.LPCB.H_EC.ECWT(BTS1, RefOf(\_SB.PCI0.LPCB.H_EC.BTEN))
  	}
  	Method(BTNS, 0, Serialized)
  	{
  		Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.BTEN)), BTS1)
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
}//end scope _SB

Scope(\_SB.PCI0) 
{
  Device(CONV)
  {
    Method(_HID, 0x0, NotSerialized)
    {
      Return("ID9001")
    }
     	Name(_CID, "PNP0C60")
  	Method(_STA, 0, Serialized)
  	{
//[-start-171226-IB07400935-modify]//
      If (LEqual (ECON, 1)){ 
        Return(0x0F)
      }
      Return(0x00)
//[-end-171226-IB07400935-modify]//
  	}
  }
}
//[-end-160831-IB15590011-modify]//