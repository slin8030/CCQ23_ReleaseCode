/** @file
  ACPI DSDT table

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2017 Intel Corporation.

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

@par Specification Reference:
**/


//[-start-170516-IB08450375-remove]//
//External(IGDS)
//[-end-170516-IB08450375-remove]//
External(\_SB.PCI0.GFX0.GHDS, MethodObj)
External(\_SB.PCI0.GFX0.CBLV, IntObj)
External(\_SB.PCI0.GFX0.AINT, MethodObj)

External(\_SB.PCI0.GFX0.GLID, MethodObj)
External(\_TZ.ETMD, IntObj)
External(\_TZ.FN00.FCTL, MethodObj)
External(\_TZ.FN00.FANS, IntObj)
External(\_SB.TPWR, DeviceObj)

//External(RTIP)
//External(HGAS, MethodObj)
//External(HPFS, MethodObj)
//External(HBRT, MethodObj)
//External(CHPS, MethodObj)
//External(UP1D)
//External(SVRF, MethodObj)
External(\_SB.PCI0.CTCU, MethodObj)
External(\_SB.PCI0.CTCN, MethodObj)
//External(\_SB.PCI0.CTCD, MethodObj)
External(\_SB.PCI0.GFX0.CLID)
//External(\_PR.POWS)

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

External(\_SB.PCI0.GFX0.IUER, IntObj)

#define POWER_BUTTON         0
#define WINDOWS_BUTTON       1
#define VOLUME_UP_BUTTON     2
#define VOLUME_DOWN_BUTTON   3
#define ROTATION_LOCK_BUTTON 4
#define CONVERTIBLE_BUTTON   6
#define DOCK_INDICATOR       7

Device(H_EC)  // Nuvoton Embedded Controller
{
  Name(_HID, EISAID("PNP0C09"))

  Name(_UID,1)


  Name(ECAV, Zero)   // OS Bug Checks if EC OpRegion accessed before Embedded Controller Driver loaded

  Mutex(ECMT, 0)  // EC Mutex

  Method(_CRS, 0, Serialized)
  {
    Name(BFFR, ResourceTemplate()
    {
      IO(Decode16,0x62,0x62,0,1)  // DIN/DOUT
      IO(Decode16,0x66,0x66,0,1)  // CMD/STS
    })

    Return(BFFR)
  }

  Method(_STA)
  {
    If (LEqual(\ECON,1)){
      Return(0x0F)
    }
    Return(0x00)
  }


  OperationRegion(ECF2,EmbeddedControl,0,0xFF)
  Field(ECF2,ByteAcc,Lock,Preserve)
  {
    Offset(1),
    TSR1, 8,      // 1       Thermal Sensor Register 1
    LTMP, 8,      // 2       Sensor Temperature
    RPWR, 1,      // 3.0     Real AC Power (AC Present = 1)
        , 1,      // 3.1     Reserved
    DCKS, 1,      // 3.2     Dock Status (1 if Docked)
    CFAN, 1,      // 3.3     CPU Fan (Fan On = 1)
        , 2,      // 3.5:4   Reserved
    LSTE, 1,      // 3.6     Lid State (Lid Open = 1)
        , 1,      // 3.7     Reserved
    // USB Type C UCSI offset 4~43 (Message in/out CCI CCA)
    MGI0, 8,      // 4       PPM->OPM Message In, 128 bits, 16 bytes
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
    MGO0, 8,      // 20      OPM->PPM Message Out, 128 bits, 16 bytes
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
    CCI0, 8,      // 36      PPM->OPM CCI indicator, 4 bytes,
    CCI1, 8,
    CCI2, 8,
    CCI3, 8,

    Offset(47),
    CTMP, 8,      // 47      EC Critical Temperature
    DOCK, 1,      // 48.0    Docking state (docked = 1)
    EJET, 1,      // 48.1    Eject switch state (Eject (Q-Switch off) = 1)
        , 1,      // 48.2    Reserved
    PBNS, 1,      // 48.3    Power Button State (0 = Pressed)
    VPWR, 1,      // 48.4    Virtual AC Power (AC Present = 1)
        , 3,      // 48.7:5  Reserved
    SCAN, 8,      // 49      Hot-Key Scan Code
    B1ST, 8,      // 50      Battery 1 Status
    B1CR, 8,      // 51      Battery 1 Current Rate
    B1CC, 8,      // 52      Battery 1 Current Capacity
    B1VT, 8,      // 53      Battery 1 Voltage
    B2ST, 8,      // 54      Battery 2 Status
    B2CR, 8,      // 55      Battery 2 Current Rate
    B2CC, 8,      // 56      Battery 2 Current Capacity
    B2VT, 8,      // 57      Battery 2 Voltage
    CMDR, 8,      // 58      KSC command register
    LUXL, 8,      // 59      LUX - Low Byte
    LUXH, 8,      // 60      LUX - High Byte
    ACH0, 8,      // 61      Channel 0 RAW Value
    ACH1, 8,      // 62      Channel 1 RAW Value
    Offset(65),
    PPSL, 8,      // 65      PWM Port Select low byte
    PPSH, 8,      // 66      PWM Port Select high byte
    PINV, 8,      // 67      PWM initial value
    PENV, 8,      // 68      PWM ending value
    PSTP, 8,      // 69      PWM ms delay between steps
    Offset(71),
    CPUP, 16,     // 71      CPU Power mW
    BPWR, 16,     // 73      Brick Power cW(100ths)
    PPWR, 16,     // 75      Platform Power cW(100ths)
    Offset(77),
    TER1, 8,      // 77      Thermistor 1
    TER2, 8,      // 78      Thermistor 2
    Offset(80),
    TER3, 8,      // 80      Thermistor 3
    TER4, 8,      // 81      Thermistor 4
    Offset(82),   // 82      TSI - Thermal Sensor Interrupt numbers
    TSI,  4,      //          # 0 = SEN1
                  //          # 1 = SEN2
                  //          # 2 = SEN3
                  //          # 3 = SEN4
                  //          # 4 = SEN5
                  //          # 5 = SEN6
                  //          # 6 = SEN7
                  //          # 7 = SEN8
                  //          # 8 = TER1
                  //          # 9 = TER2
                  //          # A = TER3
                  //          # B = TER4
                  //          # C = TER5
                  //          # D = TER6
                  //          # E = TER7
                  //          # F = TER8
    HYST, 4,      // 82.4-7  Hysteresis in deg C, hysteresis selection is global and meant for all sensors.
    TSHT, 8,      // 83      Thermal Sensor (N) high trip point
    TSLT, 8,      // 84      Thermal Sensor (N) low trip point
    Offset(85),
    TSSR, 8,      // 85      TSSR - Thermal Sensor Status Register:
                  //         TSSR bits defined:
                  //          BIT0:  SEN1
                  //          BIT1:  SEN2
                  //          BIT2:  SEN3
                  //          BIT3:  SEN4
                  //          BIT4:  SEN5
                  //          BIT5:  SEN6
                  //          BIT6:  SEN7
                  //          BIT7:  SEN8
    Offset(86),
    TESR, 8,      // 86      TESR - Thermistor Status Register:
                  //         TESR bits defined:
                  //          BIT0:    TER1
                  //          BIT1:    TER2
                  //          BIT2:    TER3
                  //          BIT3:    TER4
                  //          BIT4:    TER5
                  //          BIT5:    TER6
                  //          BIT6:    TER7
                  //          BIT7:    TER8
    Offset(87),
    B1DC, 16,     // 87      Battery 1 Design Capacity (mWh)
    B1RC, 16,     // 89      Battery 1 Remaining Capacity (mWh)
    B1FC, 16,     // 91      Battery 1 Full Charge Capacity (mWh)
    B1FV, 16,     // 93      Battery 1 Full Resolution Voltage (mV)
    B1DI, 16,     // 95      Battery 1 Full Resolution Discharge Current (mA)
    B1CI, 16,     // 97      Battery 1 Full Resolution Charge Current (mA)
    B2RC, 16,     // 99      Battery 2 Remaining Capacity (mWh)
    B2FC, 16,     // 101     Battery 2 Full Charge Capacity (mWh)
    B2FV, 16,     // 103     Battery 2 Full Resolution Voltage (mV)
    B2DI, 16,     // 105     Battery 2 Full Resolution Discharge Current (mA)
    B2CI, 16,     // 107     Battery 2 Full Resolution Charge Current (mA)
    CPAP, 16,     // 109     CPU average power in mW
    BKAP, 16,     // 111     Brick average power in cW(0.01)
    PLAP, 16,     // 113     Platform average power in cW(0.01)
    CFSP, 16,     // 115     CPU Fan speed in rpm
    BRTV, 8,      // 117     Brightness value for San Rafael second panel
       ,  8,      // 118     reserved
    SLPC, 8,      // 119     Select Performance Counters
    Offset(120),  // (0x78)
    DLED, 1,      // 120.0   EC Connected Standby Debug LED (CAPS LOCK)
    PB10, 1,      // 120.1   10 Second Power Button Control
        , 1,      // 120.2     Reserved
    DOCO, 1,      // 120.3   Dock Orientation - 1 if normal, 0 for reverse
    Offset(121),
    PWRT, 8,      // 121d(0x79)   PwrSrcType Bit0=1 if AC, Bit1=1 if USB-PD, Bit2=1 if Wireless Charging
    Offset(124),
    PTIM, 8,      // 124     CPU,GFX pwr sample timer
    DHPD, 8,      // 125     Digital display hot-plug detect
    TSR6, 8,      // 126     SEN6
    TSR7, 8,      // 127     SEN7
    PMAX, 8,      // 128     CPU,MCH & PCH Max temp
    PPDT, 8,      // 129     PCH DTS reading from PCH
    PECL, 8,      // 130     CPU PECI reading fractional value (1/64 Celsius)
    PECH, 8,      // 131     CPU PECI reading integer value (unit Celsius)
    PMDT, 8,      // 132     MCH DTS reading from PCH
    TSD0, 8,      // 133     TS-on-DIMM0 temperature
    TSD1, 8,      // 134     TS-on-DIMM1 temperature
    TSD2, 8,      // 135     TS-on-DIMM2 temperature
    TSD3, 8,      // 136     TS-on-DIMM3 temperature
    PAKN, 8,      // 137     Packet sequence number
    CPUE, 32,     // 138     CPU energy
    Offset(155),
    PRCL, 8,      // 155     Repeat cycle (LSB)
    PRC0, 8,      // 156     Repeat cycle
    PRC1, 8,      // 157     Repeat cycle
    PRCM, 8,      // 158     Repeat cycle (MSB)
    PRIN, 8,      // 159     Repeat Interval
    PSTE, 8,      // 160     Stop on Error
    PCAD, 8,      // 161     Peci Client Address
    PEWL, 8,      // 162     Write Length
    PWRL, 8,      // 163     Read length
    PECD, 8,      // 164     Command Code
    PEHI, 8,      // 165     Host ID
    PECI, 8,      // 166     Index
    PEPL, 8,      // 167     Parameter (LSB)
    PEPM, 8,      // 168     Parameter (MSB)
    PWFC, 8,      // 169     Write FCS
    PECC, 8,      // 170     Completion code
    PDT0, 8,      // 171     Data 0
    PDT1, 8,      // 172     Data 1
    PDT2, 8,      // 173     Data 2
    PDT3, 8,      // 174     Data 3
    PRFC, 8,      // 175     Read FCS
    PRS0, 8,      // 176     Reserved
    PRS1, 8,      // 177     Reserved
    PRS2, 8,      // 178     Reserved
    PRS3, 8,      // 179     Reserved
    PRS4, 8,      // 180     Reserved
    Offset(187),
    PRCS, 8,      // 187     Peci Repeat Command Status
    PEC0, 8,      // 188     Peci Error Count(LSB)
    PEC1, 8,      // 189     Peci Error Count
    PEC2, 8,      // 190     Peci Error Count
    PEC3, 8,      // 191     Peci Error Count(MSB)
    TSR8, 8,      // 192     SEN8
    MCRT, 8,      // 193     Minicard Region Temp
    Offset(196),
    WTMS, 8,      // 196     Wake timer Settings
                  //          196.0   Timer Enabled in S3
                  //          196.1   Timer Enabled in S4
                  //          196.2   Timer Enabled in S5
                  //          196.3:6 Reserved
                  //          196.7   Enable/Disable EC Timer
    AWT2, 8,      // 197     ACPIWakeTmrByte2 - Wake timer value (BIT23-16)
    AWT1, 8,      // 198     ACPIWakeTmrByte1 - Wake timer value (BIT15-8)
    AWT0, 8,      // 199     ACPIWakeTmrByte0 - Wake timer value (BIT7-0)
    SPT2, 1,      // 200.0   SATA Por2 - Cable connect power control. 1=ON;0=OFF
    ,7,           // 200.7-1  Reserved
    Offset(201),
    BTEN, 8,      // 201     Button Enable/Disable
                  //          201.0: Power Button N/A to disable
                  //          201.1: Windows Button
                  //          201.2: Volume Up Button
                  //          201.3: Volume Down Button
                  //          201.4: Rotation Lock Button
                  //          201.5-8: Reserved
    Offset(203),
    CMDM, 8,      // 203     Command Mirror updated by EC whenever it received command through offset 58
    Offset(209),
    B2DC, 16,     // 209     Battery 2 Design Capacity (mWh)
    B1DV, 16,     // 211     Battery 1 Design Voltage (mV)
    B2DV, 16,     // 213     Battery 2 Design Voltage (mV)
    Offset(215),
    BMAX, 16,     // 215   Battery A maximum
    Offset(217),
    B2ML, 8,      // 217   BattBPmaxL - Battery Pack B maximum low byte
    B2MH, 8,      // 218   BattBPmaxH - Battery Pack B maximum high byte
    BTP1, 8,      // 219   Battery 1 Trip Point in %
    B1TL, 8,      // 220   Battery 1 Trip Point lower byte in mAh
    B1TH, 8,      // 221   Battery 1 Trip Point higher byte in mAh
    KBDB, 8,      // 222   Keyboard brightness Percentage
    CHGR, 16,     // 223   Charge Rate
    Offset(231),
    SCCK, 8,      // 231   Ctrl+Alt+Shit(CAS)+Key Scan Code
    B2TL, 8,      // 232   Battery 2 Trip Point lower byte in mAh
    Offset(233),  // 233   OTG flag
    OTGI, 1,      //       OTG ID
    VBUS, 1,      //       VBUS Sense
    ,6,
    // USB Type C UCSI offset 234~250 (CONTROL and STATUS)
    Offset(234),
    CTL0, 8,      // Offset 234, OPM->PPM Control message, 8 bytes,
    CTL1, 8,
    CTL2, 8,
    CTL3, 8,
    CTL4, 8,
    CTL5, 8,
    CTL6, 8,
    CTL7, 8,
    Offset(250),
    BCLM, 16,     // 250(0xFA) Battery Charge limit
    ICLM, 16,     // 252(0xFC) Input charge limit
  } //end of OperationRegion(ECF2,EmbeddedControl,0,0xFF)


  // ECRD (Embedded Read Method)
  //
  // Handle all commands sent to EC by BIOS
  //
  //  Arguments: (1)
  //    Arg0 - Object to Read
  //  Return Value:
  //    Read Value
  //
  Method(ECRD,1,Serialized, 0, IntObj, FieldUnitObj)
  {
    Store (Acquire(ECMT, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
    If (LEqual(Local0, Zero))  // check for Mutex acquired
    {
      If (ECAV) {
        Store(DerefOf (Arg0), Local1) // Execute Read from EC
        Release(ECMT)
        Return(Local1)
      } Else {
        Release(ECMT)
      } // If (ECAV)
    } // If EC Acquired
    Return(0) // return 0 if EC not available and stop compiler warnings
  }

  // ECWT (Embedded Write Method)
  //
  // Handle all commands sent to EC by BIOS
  //
  //  Arguments: (2)
  //    Arg0 - Value to Write
  //    Arg1 - Object to Write to
  //  Return Value:
  //    None
  //
  Method(ECWT,2,Serialized,,,{IntObj, FieldUnitObj})
  {
    Store (Acquire(ECMT, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
    If (LEqual(Local0, Zero))  // check for Mutex acquired
    {
      If (ECAV) {
        Store(Arg0,Arg1) // Execute Write to EC
      } // If (ECAV)
      Release(ECMT)
    } // If EC Acquired
  }


  // ECWT (Embedded Write Method)
  //
  // Handle all commands sent to EC by BIOS
  //
  //  Arguments: (2)
  //    Arg0 - Value to Write
  //    Arg1 - Object to Write to
  //  Return Value:
  //    None
  //
  Method(ECWR,2,Serialized,,,{IntObj, FieldUnitObj})
  {
    Store (Acquire(ECMT, 1000), Local0)  // save Acquire result so we can check for Mutex acquired
    If (LEqual(Local0, Zero))  // check for Mutex acquired
    {
      If (ECAV) {
        Store(Arg0,Arg1) // Execute Write to EC
        Store(0, Local1)
        While(1) {
          If(LEqual(Arg0, DerefOf (Arg1))){
            break;
          }
          Sleep(1)
          Store(Arg0, Arg1)
          Add(Local1, 1, Local1)
          If(LEqual(Local1, 3)) {
            break
          }
        }
      } // If (ECAV)
      Release(ECMT)
    } // If EC Acquired
  }


  // ECMD (Embedded Controller Command)
  //
  // Handle all commands sent to EC by BIOS
  //
  //  Arguments: (1)
  //    Arg0 - EC command
  //  Return Value:
  //    None
  //
  Method(ECMD,1,Serialized)
  {
    If (ECAV) {
      While(ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.CMDR))){Stall(20)} // make sure no command is currently being processed so we don't overwrite it
      ECWT(Arg0, RefOf(\_SB.PCI0.LPCB.H_EC.CMDR))
    }
  }


// ECMD (Embedded Controller Command)
  //
  // Handle all commands sent to EC by BIOS
  //
  //  Arguments: (1)
  //    Arg0 - EC command
  //  Return Value:
  //    None
  //
  Method(ECM1,1,Serialized)
  {
    If (ECAV) {
      While(ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.CMDR))){Stall(50)} // make sure no command is currently being processed so we don't overwrite it
      ECWT(Arg0, RefOf(\_SB.PCI0.LPCB.H_EC.CMDR))

      While(1)
      {
         If(LEqual(Arg0, ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.CMDM))))
         {
           break;
         }
         Stall(50)
          If(LEqual(Arg0, ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.CMDM))))
          {
           break;
          } Else {
            While(ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.CMDR))){Stall(50)} // make sure no command is currently being processed so we don't overwrite it
            ECWT(Arg0, RefOf(\_SB.PCI0.LPCB.H_EC.CMDR))
          }
          P8XH(0,0xDC)
      }
      ECWT(0, RefOf(\_SB.PCI0.LPCB.H_EC.CMDM))
    }
  }

  // Virtual battery code
  //
  Include("Bat0Virt.asl")

  // Real battery code
  //
  Include("Bat1Real.asl")

  // The _REG Method is needed because communication with the EC
  // before the driver is loaded is prohibited in WIN2000/WINXP.

  Method(_REG,2)
  {
    Store(0x1,ECAV) // ECAV is used mostly in DPTF/BATTERY.
    Store(0x1,\ECON) // ECON is used in other places.

    If (LAnd(LEqual(Arg0,3),LEqual(Arg1,1)))
    {
      // Must be running NT 5.0 OS.
      // Update ECAV Object. ASL should check for this value to be One before accessing EC OpRegion.
      Store(One, ECAV)

      // Turn off the CPU Fan if Active Cooling
      //   is disabled.
      If (LEqual(0,ACTT))
      {
        ECWT(0,RefOf(CFAN))
      }

      // Legacy thermal code attempts to operate fan before _REG is called and fan is not yet available.
      // So OS falsely believes Fan is in a known state.  Set Fan to a known state here.
      If(LEqual(1,\_TZ.FN00.FANS)){
        \_TZ.FN00.FCTL(100) // Fan on
      } Else {
        \_TZ.FN00.FCTL(0) // Fan off
      }

      // Save the Lid State in global NVS and IGD OpRegion.
      //Store(LSTE,\_SB.PCI0.GFX0.CLID)
      If (LEqual(ECRD(RefOf(LSTE)), 0))
      {
        Store(0,\_SB.PCI0.GFX0.CLID)
      }
      If (LEqual(ECRD(RefOf(LSTE)), 1))
      {
        Store(3,\_SB.PCI0.GFX0.CLID)
      }
      Store(ECRD(RefOf(LSTE)),LIDS)

      // Initialize the Number of Present Batteries.
      //  1 = Real Battery 1 is present
      //  2 = Real Battery 2 is present
      //  3 = Real Battery 1 and 2 are present

      Store(0,BNUM)
      Or(BNUM,ShiftRight(And(ECRD(RefOf(B1ST)),0x08),3),BNUM)

      // Initialize the Power State.
      //  BNUM = 0 = Virtual Power State
      //  BNUM > 0 = Real Power State

      If (LEqual(BNUM,0))
      {
        Store(ECRD(RefOf(VPWR)),PWRS)
      }
      Else
      {
        Store(ECRD(RefOf(RPWR)),PWRS)
      }

      // Perform needed ACPI Notifications.
      PNOT()
    }
  }

  Method(_GPE)
  {
    Store (0x2B,  Local0)   // GPIO_11 --> GPE_2B
    return (Local0)
  }

  //
  // SCI handler for USB device Attach/Detach notification
  //

  // XDAT: XDCI device attach/detach
  // Return 1 when device attach
  // Return 0 when device detach
  Method(XDAT)
  {
    If(LEqual(VBUS,1)){
      Return(1)
    } Else {
      Return(0)
    }
  }

  // For the below _Qxx Methods, The Runtime SCI has been asserted,
  // the EC Driver performed it's query, and the value returned from
  // the Query = xx.

  Method(_Q30)    // Real Battery AC Insertion Event.
  {
    P8XH(0,0x30)
    ADBG("_Q30 AC INS")

    // Set Global Power State = AC Mode.
    Store(1,PWRS)

    // Perform needed ACPI Notifications.
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
       Notify(\_SB.TPWR, 0x81) // notify Power participant
    }

    PNOT()     // Platform Notifications.
  } //end of Method(_Q30)


  Method(_Q31)    // Real Battery AC Removal Event.
  {
    P8XH(0,0x31)
    ADBG("_Q31 AC RMV")

    // Set Global Power State = Battery Mode.
    Store(0,PWRS)

    // Perform needed ACPI Notifications.
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
       Notify(\_SB.TPWR, 0x81) // notify Power participant
    }

    PNOT()     // Platform Notifications.
  }  //end of Method(_Q31)


  Method(_Q32)    // Real Battery Capacity Change.
  {
    P8XH(0,0x32)
    ADBG("_Q32 BAT CAP CHG")
    // Perform needed ACPI Notifications.
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
       Notify(\_SB.TPWR, 0x86) // notify Power participant
    }

    PNOT()     // Platform Notifications.
  } //end of Method(_Q32)


  Method(_Q33)    // Real Battery Insertion/Removal Event.
  {
    P8XH(0,0x33)
    ADBG("_Q33 BAT INOUT")

    // Initialize the Number of Present Batteries.
    //  1 = Real Battery 1 is present
    //  2 = Real Battery 2 is present
    //  3 = Real Battery 1 and 2 are present
    Store(0,BNUM)
    Or(BNUM,ShiftRight(And(ECRD(RefOf(B1ST)),0x08),3),BNUM)

    // Perform needed ACPI Notifications.
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
       Notify(\_SB.TPWR, 0x81) // notify Power participant
    }

    PNOT()     // Platform Notifications.
  } //end of Method(_Q33)


  Method(_Q34)    // PMAX changed by 250mw
  {
    P8XH(0,0x34)
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
      Notify(\_SB.TPWR, 0x80) // notify Power participant
    }
  }


  Method(_Q35)    // PBSS changed by 100mw
  {
    P8XH(0,0x35)
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
      Notify(\_SB.TPWR, 0x83) // notify Power participant
    }
  }


  Method(_Q36)    // _BIX.CycleCount changed by 1
  {
    P8XH(0,0x36)
    If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
      Notify(\_SB.TPWR, 0x82) // notify Power participant
    }
  }

  Method(_Q37)    //PWR_SRC_CHANGE_SCI
  {
   //PWRT 121d(0x79) PwrSrcType Bit0=1 if AC, Bit1=1 if USB-PD, Bit2=1 if Wireless Charging
   ADBG("_Q37 PWR_SRC_CHANGE ")
   P8XH (0,0x37)

   And(\_SB.PCI0.LPCB.H_EC.PWRT, 0x03, Local0)

   If (LOr(LEqual(Local0, 1), LEqual(Local0, 2))) {
     Store(1, PWRS)    // AC or USB-PD Insertion
   }

   If (LEqual (Local0, 0x0)) {
     Store(0, PWRS)  // AC and USB-PD Removal
   }

   // Perform needed ACPI Notifications.
   If(LAnd(LEqual(\DPTE,1), LEqual(\PWRE,1))) {
      Notify(\_SB.TPWR, 0x81) // notify Power participant
   }
   PNOT()
  }

  Method(_Q40, 0, Serialized)   // Dock complete Event
  {
    P8XH(0,0x40)
//
//    Sleep(1000) // Delay 1 second for hot docking stability
//    Store(DKSM, \SSMP)
//    Store(1, \DSTS)
//    Sleep(1000) // Delay 1 second for hot docking stability
//    Notify(\_SB.PCI0.DOCK, 0x00)
//
//    If (LAnd(CondRefOf(^VGBI._STA),LEqual(And(^VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
//    {
//      Or(PB1E, 0x10, PB1E) // Set Dock status
//      ^VGBI.UPBT(DOCK_INDICATOR, One)
//     // ADBG("Notify 0xCA")
//      Notify(^VGBI, 0xCA) // Notify Virtual GPIO driver that Docking event happens.
//    }
//
//    If (IGDS)
//    {
//      \_SB.PCI0.GFX0.GDCK(1)
//    }
  }  //end of Method(_Q40)

  Method(_Q41)    // Surprise Removal
  {
    P8XH(0,0x41)

//    Store(0, \DSTS)
//    Notify(\_SB.PCI0.DOCK, 0x01)
//
//    If (LAnd(CondRefOf(^VGBI._STA),LEqual(And(^VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
//    {
//      And(PB1E, Not(0x10), PB1E) // Clear Dock Status
//      ^VGBI.UPBT(DOCK_INDICATOR, Zero)
//     // ADBG("Notify 0xCB")
//      Notify(^VGBI, 0xCB) // Notify Virtual GPIO driver that Undocking event happens.
//    } Else {
//      If (IGDS)
//      {
//        \_SB.PCI0.GFX0.GDCK(0)
//      }
//    }
  }  //end of Method(_Q41)

  Method(_Q42)    // Undocking request Event
  {
    P8XH(0,0x42)

//    Notify(\_SB.PCI0.DOCK, 0x01)
//
//    If (LAnd(CondRefOf(^VGBI._STA),LEqual(And(^VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
//    {
//      And(PB1E, Not(0x10), PB1E) // Clear Dock Status
//      ^VGBI.UPBT(DOCK_INDICATOR, Zero)
//      If (CondRefOf(\_SB.PCI0.GFX0.IUER))
//      {
//        Store(\_SB.PCI0.GFX0.IUER, Local0)
//        And(Local0, Not(0x80), \_SB.PCI0.GFX0.IUER)
//      }
//     // ADBG("Notify 0xCB")
//      Notify(^VGBI, 0xCB) // Notify Virtual GPIO driver that Undocking event happens.
//    } Else {
//      If (IGDS)
//      {
//        \_SB.PCI0.GFX0.GDCK(0)
//      }
//    }
  }  //end of Method(_Q42)

  Method(_Q43)    // Express Card Presence Changed on Saddlestring (Docking)
  {
    P8XH(0,0x43)

//    Notify(\_SB.PCI0.RP07, 0x0)
//  }
//
//  Method(_Q44)  // Virtual Dock Presence Changed
//  {
//    If (\DSTS)
//    {
//      Notify(\_SB.PCI0.DOCK, 1) //UnDocked
//      Store(0, \DSTS)
//    }
//    Else
//    {
//      Notify(\_SB.PCI0.DOCK, 0) //Docked
//      Store(1, \DSTS)
//    }
  }  //end of Method(_Q43)

  Method(_Q51)    // Lid Switch Event.
  {
    P8XH(1, 0)
    P8XH(0,0x51)

    // Update Lid NVS State.

    Store(ECRD(RefOf(LSTE)),LIDS)

    If (CondRefOf(\_SB.PCI0.GFX0.GLID))
    {
      \_SB.PCI0.GFX0.GLID(LIDS)
    }

    // If ETM is not disabled, notify IETM device
   If (CondRefOf(\_TZ.ETMD))
   {
     If (LEqual(\_TZ.ETMD, 0))
     {
       If (CondRefOf(\_SB.IETM))
       {
         Notify(\_SB.IETM,0x83)
       }
     }
   }
   Notify(\_SB.PCI0.LPCB.H_EC.LID0, 0x80)

   If (LEqual(LIDS, 0)) {
     P8XH (1, 1)
   } Else {
     P8XH (1, 2)
   }

  }  //end of Method(_Q51)

  Method(_Q79)    // Any event on USBTypeC.
  {
    P8XH(0, 0x79)
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

    Notify(\_SB.UBTC, 0x80)
  } //end of Method(_Q79)

// Display Hotkey Control Function
//    Handle display hotkey control functions, such as display switch, display brightness increase/decrease
//
//  Arguments:
//    Arg0: Function Number
//            1 = Display Switch
//            2 = Display Brightness Increase
//            3 = Display Brightness Decrease
//            4+ = Reserved
//
//    Arg1: Sub Function Number
//            if Arg0 == 1, then
//              x = Display Switch Table Index x ( 0<= x <= 3)
//
//  Return Value:
//    None

  Method(DHCF, 2, Serialized)
  {
    Switch(ToInteger(Arg0))
    {
      Case(1) // GMCH SCI hotkey display switch, table index x
      {
        If (LAnd(IGDS, LLess(Arg1, 4)))
        {
          \_SB.PCI0.GFX0.GHDS(Arg1)
        }
      }
      Case(2) // Brightness Increase.
      {
        If (And(4,DSEN))
        {
          BRTN(0x86)
        } Else {
          // Current brightness is a percentage number. This must be done if ALS is enabled,
          // as the driver will change the brightness based on ambient light.
          Store(\_SB.PCI0.GFX0.CBLV, Local0)

          // 1 % will be lost in the conversion, so need to make it up first.
          AND(Add(Local0, 1),0xFE, Local0)

          If (LLessEqual(Local0, 90))
          {
            Add(Local0, 10, Local0)
          }
          Store(Local0, BRTL)
          \_SB.PCI0.GFX0.AINT(1, Local0)
        }
      }
      Case(3) // Brightness Decrease.
      {
        If (And(4,DSEN))
        {
          BRTN(0x87)
        } Else {
          // Current brightness is a percentage number. This must be done if ALS is enabled,
          // as the driver will change the brightness based on ambient light.
          Store(\_SB.PCI0.GFX0.CBLV, Local0)

          // 1 % will be lost in the conversion, so need to make it up first.
          AND(Add(Local0, 1),0xFE, Local0)

          If (LGreaterEqual(Local0, 10))
          {
            Subtract(Local0, 10, Local0)
          }
          Store(Local0, BRTL)
          \_SB.PCI0.GFX0.AINT(1, Local0)
        }
      }
    }
  }  //end of Method(DHCF)

  Method(_Q54)    // Power Button Event for Control method Power Button(10sec PB Override without V-GPIO driver)
  {
    P8XH(0,0x54)

    //
    // Check if 10sec PB Override and EC 10sec mode.
    // If 10sec PB OVR is enabled but EC 10sec mode is not enabled, then BIOS has to handle
    // Power button event.
    // PB1E Bit0 -> Setup setting for 10sec PB OVR
    // PB10 -> EC setting for 10sec PB OVR
    //
    If (LAnd(And(PB1E, 1), LNot(ECRD(RefOf(PB10))))){

      //
      // Check UAMS(User Absent Mode State) to notify the power button event.
      //
      If (UAMS){ // UAMS has a non-Zero value, means the system is under User Absent Mode. Send Wake up event.
        ADBG("PB Wake up 0x02")
        If (CondRefOf(\_SB.PWRB)){
          Notify(\_SB.PWRB, 0x02)
        }
      } Else { // UAMS is Zero, means the system is ON. Send Sleep event.
        ADBG("PB Sleep 0x80")
        If (CondRefOf(\_SB.PWRB)){
          Notify(\_SB.PWRB, 0x80)
        }
      }
    }
  } // end Method(_Q54) Power button event.

  Name(ABCD,0) // used as counter by hotkeys

  Method(_Q52)    // Hot-Key Event.
  {
    P8XH(0,0x52)
    Store(ECRD(RefOf(SCAN)),Local0)
    P8XH(1,Local0)
    ADBG(Concatenate("CAS+SCNCODE=", ToHexString(Local0)))

    // This event will be generated whenever a
    // CRTL+ALT+SHIFT+"Next Key" is hit.  This event handler
    // will base code execution on the "Next Key" Scan Code
    // stored in the EC Memory Space.

    // The scan code is between 1 - 0(0x02-0x0B), and -/+(0x0C, 0x0D).
    If (LAnd(LGreaterEqual(Local0,0x02), LLessEqual(Local0, 0x0D)))
    {
      // Virtual Battery Hot Keys(VBHK) - VBHK uses keys 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, + and F5.
      If (LEqual(BNUM,0))  // Check for Virtual Battery Mode before testing the hot-keys specific to Virtual Battery Support.
      {
        VBHK()  //  remove this call if no virtual battery is needed.
      }
    }

    // "BKSP" Scan Code = 0x0E
    If (LEqual(Local0,0x0E))
    {
      ADBG("CAS+BKSP")
      \_SB.HIDD.HPEM(14)        // HID Event Index 14: Stop.
    }

    // "Tab" Scan Code = 0x0F
    If (LEqual(Local0,0x0F))
    {
      ADBG("CAS+TAB")
      \_SB.HIDD.HPEM(15)        // HID Event Index 15: Play/Pause.
    }

    // "Q" Scan Code = 0x10
    If (LEqual(Local0,0x10))
    {
      ADBG("CAS+Q")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(1)         // HID Event Index 1: Windows Button (Keyboard Left GUI).
    }

    // "W" Scan Code = 0x11
    ElseIf (LEqual(Local0,0x11))
    {
      ADBG("CAS+W")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(2)         // HID Event Index 2: Rotation Lock
    }

    // "E" Scan Code = 0x12
    ElseIf (LEqual(Local0,0x12))
    {
      ADBG("CAS+E")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(3)         // HID Event Index 3: Num Lock
    }

    // "R" Scan Code = 0x13
    ElseIf (LEqual(Local0,0x13))
    {
      ADBG("CAS+R")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(4)         // HID Event Index 4: Home
    }

    // "T" Scan Code = 0x14
    ElseIf (LEqual(Local0,0x14))
    {
      ADBG("CAS+T")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(5)         // HID Event Index 5: End
    }

    // "Y" Scan Code = 0x15
    ElseIf (LEqual(Local0,0x15))
    {
      ADBG("CAS+Y")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(6)         // HID Event Index 6: Page Up
    }

    // "U" Scan Code = 0x16
    ElseIf (LEqual(Local0,0x16))
    {
      ADBG("CAS+U")
      Sleep(1000)               // Index 1-7 are Keyboard/Keypad HIDs.  Add delay.
      \_SB.HIDD.HPEM(7)         // HID Event Index 7: Page Down
    }

    // "I" Scan Code = 0x17
    ElseIf (LEqual(Local0,0x17))
    {
      ADBG("CAS+I")
      \_SB.HIDD.HPEM(8)         // HID Event Index 8: Wireless Radio Button
    }

    // "O" Scan Code = 0x18
    ElseIf (LEqual(Local0,0x18))
    {
      ADBG("CAS+O")
      \_SB.HIDD.HPEM(9)         // HID Event Index 9: System Power Down
    }

    // HK: "P" Scan Code = 0x19
    ElseIf (LEqual(Local0,0x19))   // CTDP Up
    {
      /*If (LAnd(CondRefOf(\_SB.PCI0.CTCU),LEqual(CTDB,1))){ // only allow this feature if it is enabled in SETUP
        \_SB.PCI0.CTCU()
      }*/
    }

    // HK: "[" Scan Code = 0x1A
    ElseIf (LEqual(Local0,0x1A))   // CTDP Nominal
    {
      /*If (LAnd(CondRefOf(\_SB.PCI0.CTCN),LEqual(CTDB,1))){ // only allow this feature if it is enabled in SETUP
        \_SB.PCI0.CTCN()
      }*/
    }

    // HK: "]" Scan Code = 0x1B
    ElseIf (LEqual(Local0,0x1B))   // CTDP Down
    {
      /*If (LAnd(CondRefOf(\_SB.PCI0.CTCD),LEqual(CTDB,1))){ // only allow this feature if it is enabled in SETUP
        \_SB.PCI0.CTCD()
      }*/
    }

    // Calculator (Enter) Key = 0x1C
    ElseIf (LEqual(Local0,0x1C))
    {
      ADBG("CAS+Enter")
      \_SB.HIDD.HPEM(27)         // HID Event Index 27: System Wake
    }

    // "A" Scan Code = 0x1E
    If (LEqual(Local0,0x1E))
    {
      ADBG("CAS+A")
      \_SB.HIDD.HPEM(16)        // HID Event Index 16: Mute.
    }


    // "F" Scan Code = 0x21
    ElseIf (LEqual(Local0,0x21))
    {
      ADBG("CAS+F")
      // Virtual Button Event - Screenshot

      If (LAnd(CondRefOf(^VGBI._STA),LEqual(And(^VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
      {
        //
        // CAS+F test emulation mode
        // 1. Windows press notify 0xC2
        // 2. Volume down press notify 0xC6
        // 3. Volume down release notify 0xC7
        // 4. Windows release notify 0xC3
        //

        Sleep(1000) // 1sec delay is needed for Powerbutton and Windows Home button

        ^VGBI.UPBT(WINDOWS_BUTTON, One) // Update VBDS
       // ADBG("Notify 0xC2")
        Notify(^VGBI, 0xC2) // Notify Virtual GPIO driver that Windows Home button is pressed.

        ^VGBI.UPBT(VOLUME_DOWN_BUTTON, One)
       // ADBG("Notify 0xC6")
        Notify(^VGBI, 0xC6) // Notify Virtual GPIO driver that Vol down button is pressed.

        ^VGBI.UPBT(VOLUME_DOWN_BUTTON, Zero)
       // ADBG("Notify 0xC7")
        Notify(^VGBI, 0xC7) // Notify Virtual GPIO driver that Vol down button is released.

        ^VGBI.UPBT(WINDOWS_BUTTON, Zero) // Update VBDS
       // ADBG("Notify 0xC3")
        Notify(^VGBI, 0xC3) // Notify Virtual GPIO driver that Windows Home button is released.
      }
    }

    // "G" Scan Code = 0x22
    ElseIf (LEqual(Local0,0x22))
    {
      ADBG("CAS+G")
      // Virtual Button Event - Ctrl + Alt + Del Screen

      Sleep(1000) // 1sec delay is needed for Powerbutton and Windows Home button

      If (And(PB1E, 0x20)) {   // Check if VBDL Flag(PB1E Bit5) is set
        //
        // CAS+G test emulation mode
        // 1. Windows press notify 0xC2
        // 2. Powerbutton press notify 0xC0
        // 3. Powerbutton release notify 0xC1
        // 4. Windows release notify 0xC3
        //

        ^VGBI.UPBT(WINDOWS_BUTTON, One)
       // ADBG("Notify 0xC2")
        Notify(^VGBI, 0xC2) // Notify Virtual GPIO driver that Windows Home button is pressed.
        If (And(PB1E, 1))
        {
          ^VGBI.UPBT(POWER_BUTTON, One)
         // ADBG("Notify 0xC0")
          Notify(^VGBI, 0xC0) // Notify Virtual GPIO driver that Power button is pressed.

          ^VGBI.UPBT(POWER_BUTTON, Zero)
         // ADBG("Notify 0xC1")
          Notify(^VGBI, 0xC1) // Notify Virtual GPIO driver that Power button is released.
        }

        ^VGBI.UPBT(WINDOWS_BUTTON, Zero)
       // ADBG("Notify 0xC3")
        Notify(^VGBI, 0xC3) // Notify Virtual GPIO driver that Windows Home button is released.
      }
    }

    // "H" Scan Code = 0x23
    ElseIf (LEqual(Local0,0x23))
    {
      ADBG("CAS+H")
      // Virtual Button Event - Windows Button
      Sleep(1000)

      If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Windows Home
       // ADBG("Notify _HID 0xC2")
        Notify(\_SB.HIDD,0xC2) // Notify HID driver that Windows Home button is pressed.
       // ADBG("Notify _HID 0xC3")
        Notify(\_SB.HIDD,0xC3) // Notify HID driver that Windows Home button is released.
      } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
        ^VGBI.UPBT(WINDOWS_BUTTON, One)
       // ADBG("Notify VBDR 0xC2")
        Notify(^VGBI, 0xC2) // Notify Virtual GPIO driver that Windows Home button is pressed.
        ^VGBI.UPBT(WINDOWS_BUTTON, Zero)
       // ADBG("Notify VBDR 0xC3")
        Notify(^VGBI, 0xC3) // Notify Virtual GPIO driver that Windows Home button is released.
      }
    }

    // "J" Scan Code = 0x24
    ElseIf (LEqual(Local0,0x24))
    {
      ADBG("CAS+J")
      // Virtual Button Event - Volume Up

      If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Volume UP Press
       // ADBG("Notify _HID 0xC4")
        Notify(\_SB.HIDD,0xC4) // Notify HID driver that Volume UP button is pressed.
       // ADBG("Notify _HID 0xC5")
        Notify(\_SB.HIDD,0xC5) // Notify HID driver that Volume UP button is released.
      } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
        ^VGBI.UPBT(VOLUME_UP_BUTTON, One)
       // ADBG("Notify VBDR 0xC4")
        Notify(^VGBI, 0xC4) // Notify Virtual GPIO driver that Volume UP button is pressed.

        ^VGBI.UPBT(VOLUME_UP_BUTTON, Zero)
       // ADBG("Notify VBDR 0xC5")
        Notify(^VGBI, 0xC5) // Notify Virtual GPIO driver that Volume UP button is released.
      }
    }

    // "K" Scan Code = 0x25
    ElseIf (LEqual(Local0,0x25))
    {
      ADBG("CAS+K")
      // Virtual Button Event - Volume Down

      If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Volume Down Press
       // ADBG("Notify _HID 0xC6")
        Notify(\_SB.HIDD,0xC6) // Notify HID driver that Volume Down button is pressed.
       // ADBG("Notify _HID 0xC7")
        Notify(\_SB.HIDD,0xC7) // Notify HID driver that Volume Down button is released.
      } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
        ^VGBI.UPBT(VOLUME_DOWN_BUTTON, One)
       // ADBG("Notify VBDR 0xC6")
        Notify(^VGBI, 0xC6) // Notify Virtual GPIO driver that Volue Down button is pressed.

        ^VGBI.UPBT(VOLUME_DOWN_BUTTON, Zero)
       // ADBG("Notify VBDR 0xC7")
        Notify(^VGBI, 0xC7) // Notify Virtual GPIO driver that Volue Down button is released.
      }
    }

    // "L" Scan Code = 0x26
    ElseIf (LEqual(Local0,0x26))
    {
      ADBG("CAS+L")
      // Virtual Button Event - Rotation Lock
      Sleep(1000)
      If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: RotationLock Press
       // ADBG("Notify _HID 0xC8")
        Notify(\_SB.HIDD,0xC8) // Notify HID driver that Rotation Lock button is pressed.
       // ADBG("Notify _HID 0xC9")
        Notify(\_SB.HIDD,0xC9) // Notify HID driver that Rotation Lock button is released.
      } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
        Xor(PB1E, 0x04, PB1E) // Toggle Rotation Lock Status (bit2 of PB1E)
        ^VGBI.UPBT(ROTATION_LOCK_BUTTON, One)
       // ADBG("Notify VBDR 0xC8")
        Notify(^VGBI, 0xC8) // Notify Virtual GPIO driver that Rotation Lock button is pressed.

        ^VGBI.UPBT(ROTATION_LOCK_BUTTON, Zero)
       // ADBG("Notify VBDR 0xC9")
        Notify(^VGBI, 0xC9) // Notify Virtual GPIO driver that Rotation Lock button is released.
      }
    }

    // ";" Scan Code = 0x27
    ElseIf (LEqual(Local0,0x27))
    {
      ADBG("CAS+;")
      // Virtual Button Event - Convertible Indicator Toggle

      If (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
        Xor(PB1E, 0x08, PB1E) // Toggle Slate/Laptop Lock Status
        If (And(PB1E, 0x08)) // Slave/Laptop Mode changed 0 -> 1 Laptop mode
        {
          ^VGBI.UPBT(CONVERTIBLE_BUTTON, One)
         // ADBG("Notify 0xCD")
          Notify(^VGBI, 0xCD) // Notify Virtual GPIO driver that the system is now in Laptop mode.
        } Else { // Slave/Laptop Mode Changed 1 -> 0 Slate/Tablet Mode
          ^VGBI.UPBT(CONVERTIBLE_BUTTON, Zero)
         // ADBG("Notify 0xCC")
          Notify(^VGBI, 0xCC) // Notify Virtual GPIO driver that the system is now in Slate/Tablet mode.
        }
      }
    }

    // "'" Scan Code = 0x28
    ElseIf (LEqual(Local0,0x28))
    {
      ADBG("CAS+'")
      // Virtual Button Event - Docking Indicator Toggle

      If (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
        Xor(PB1E, 0x10, PB1E) // Toggle Dock/Undock Status
        If (And(PB1E, 0x10)) // Dock/Undock status changed 0 -> 1 Dock mode
        {
          ^VGBI.UPBT(DOCK_INDICATOR, One)
         // ADBG("Notify 0xCA")
          Notify(^VGBI, 0xCA) // Notify Virtual GPIO driver that the system is now in Dock mode.
        } Else { // Dock/Undock status Changed 1 -> 0 Undock mode
          ^VGBI.UPBT(DOCK_INDICATOR, Zero)
         // ADBG("Notify 0xCB")
          Notify(^VGBI, 0xCB) // Notify Virtual GPIO driver that the system is now in Undock mode.
        }
      }
    }


    // "Z" Scan Code = 0x2C
    ElseIf (LEqual(Local0,0x2C))
    {
      ADBG("CAS+Z")
      // Virtual Button Event - Narrator Settings

      If (LAnd(CondRefOf(^VGBI._STA),LEqual(And(^VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
      {
        //
        // CAS+Z test emulation mode
        // 1. Windows press notify 0xC2
        // 2. Volume up press notify 0xC4
        // 3. Volume up release notify 0xC5
        // 4. Windows release notify 0xC3
        //

        Sleep(1000) // 1sec delay is needed for Powerbutton and Windows Home button.

        ^VGBI.UPBT(WINDOWS_BUTTON, One) // Update VBDS
       // ADBG("Notify 0xC2")
        Notify(^VGBI, 0xC2) // Notify Virtual GPIO driver that Windows Home button is pressed.

        ^VGBI.UPBT(VOLUME_UP_BUTTON, One)
       // ADBG("Notify 0xC4")
        Notify(^VGBI, 0xC4) // Notify Virtual GPIO driver that Vol up button is pressed.

        ^VGBI.UPBT(VOLUME_UP_BUTTON, Zero)
       // ADBG("Notify 0xC5")
        Notify(^VGBI, 0xC5) // Notify Virtual GPIO driver that Vol up button is released.

        ^VGBI.UPBT(WINDOWS_BUTTON, Zero)
       // ADBG("Notify 0xC3")
        Notify(^VGBI, 0xC3) // Notify Virtual GPIO driver that Windows Home button is released.
      }
    }


    // F1 Scan Code = 0x3B
    ElseIf (LEqual(Local0,0x3B))
    {
      ADBG("CAS+F1")
      DHCF(1, 0)              // IGfx Display Switch using Toggle List 1.
    }

    // F2 Scan Code = 0x3C
    ElseIf (LEqual(Local0,0x3C))
    {
      ADBG("CAS+F2")
      DHCF(1, 1)              // IGfx Display Switch using Toggle List 2.
    }

    // F3 Scan Code = 0x3D
    ElseIf (LEqual(Local0,0x3D))
    {
      ADBG("CAS+F3")
      DHCF(1, 2)              // IGfx Display Switch using Toggle List 3
    }

    // F4 Scan Code = 0x3E
    ElseIf (LEqual(Local0,0x3E))
    {
      ADBG("CAS+F4")
      DHCF( 1, 3)             // IGfx Display Switch using Toggle List 4
    }

    // F5 Scan Code = 0x3F
    ElseIf (LEqual(Local0,0x3F))
    {
      ADBG("CAS+F5")
      // Virtual Battery Hot Keys(VBHK) - VBHK uses keys 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, -, + and F5.
      If (LEqual(BNUM,0))      // Check for Virtual Battery Mode before testing the hot-keys specific to Virtual Battery Support.
      {
        VBHK()                //  remove this call if no virtual battery is needed.
      }
    }

    // F8 Scan Code = 0x42
    ElseIf (LEqual(Local0,0x42))
    {
     // ADBG("CAS+F8")
      // Panel Fitting Hot Key.
      If (IGDS)
      {
        \_SB.PCI0.GFX0.AINT(2, 0)
      }
    }

    // F9 Scan Code = 0x43
    ElseIf (LEqual(Local0,0x43))
    {
      ADBG("CAS+F9")
      DHCF(3, 0)              // Decrease Brightness Level.
    }

    // F10 Scan Code = 0x44
    ElseIf (LEqual(Local0,0x44))
    {
      ADBG("CAS+F10")
      DHCF(2, 0)              // Increase Brightness Level.
    }

    // F12 Scan Code = 0x58
    ElseIf (LEqual(Local0,0x58))
    {
      ADBG("CAS+F12")
      \_SB.HIDD.HPEM(11)      // HID Event Index 11: System Sleep
    }

    // Home Scan Code = 0x47
    // Don't use: SDS shares this key with Left Arrow key and should not be used for Hotkey.

    // UpArw Scan Code = 0x48
    ElseIf (LEqual(Local0,0x48))
    {
      ADBG("CAS+UpArw")
      \_SB.HIDD.HPEM(17)         // HID Event Index 17: Volume Increment
    }

    // PgUp Scan Code = 0x49
    // Don't use: SDS shares this key with Up Arrow key and should not be used for Hotkey.

    // LftArw Scan Code = 0x4B
    ElseIf (LEqual(Local0,0x4B))
    {
      ADBG("CAS+LftArw")
      \_SB.HIDD.HPEM(20)         // HID Event Index 20: Display Brightness Decrement
    }

    // RtArw Scan Code = 0x4D
    ElseIf (LEqual(Local0,0x4D))
    {
      ADBG("CAS+RtArw")
      \_SB.HIDD.HPEM(19)         // HID Event Index 19: Display Brightness Increment
    }

    // End Scan Code = 0x4F
    // Don't use: SDS shares this key with Right Arrow key and should not be used for Hotkey.

    // DwnArw Scan Code = 0x50
    ElseIf (LEqual(Local0,0x50))
    {
      ADBG("CAS+DwnArw")
      \_SB.HIDD.HPEM(18)         // HID Event Index 18: Volume Decrement
    }

    // PgDn Scan Code = 0x51
    // Don't use: SDS shares this key with Down Arrow key and should not be used for Hotkey.

    // Ins Scan Code = 0x52
    ElseIf (LEqual(Local0,0x52))
    {
      ADBG("CAS+Ins")
      \_SB.HIDD.HPEM(13)         // HID Event Index 13: Scan Prev Track
    }

    // Del Scan Code = 0x53
    ElseIf (LEqual(Local0,0x53))
    {
      ADBG("CAS+Del")
      \_SB.HIDD.HPEM(12)        // HID Event Index 12: Scan Next Track
    }
  } // end Method(_Q52) CAS hot key event


  Method (_Q70)   // ALS "Light Intensity" event
  {
    P8XH(0,0x70)
//
//    // Update the LUX Values.
//
//    Store(ECRD(RefOf(LUXH)),LHIH)
//    Store(ECRD(RefOf(LUXL)),LLOW)
//
//    If (LAnd(LEqual(ALSE,2),IGDS))
//    {
//      // Handle the ALS event via the OpRegion method
//
//      Store(ECRD(RefOf(LUXH)), Local0)
//      Or(ShiftLeft(Local0, 8), ECRD(RefOf(LUXL)), Local0)
////      \_SB.PCI0.GFX0.AINT(0, Local0)
////      Notify(ALSD,0x80) // notify MS driver of change
//    }
  }  //end of Method(_Q70)

  Method(_QD5)    // 10 second power button press.
  {
    P8XH(0,0xD5)
    \_SB.PWPR()
  }

  Method(_QD6)    // 10 second power button de-press.
  {
    P8XH(0,0xD6)
    \_SB.PWRR()
  }

  Method(_Q80)    // Volume Up
  {

    P8XH(0,0x80)
    // ADBG("Volume Up")
    // Volume Up.
    // If VirtualButton driver loaded
    //   use VirtualButton driver
    // Else If HID Event Driver loaded
    //   use HID Event Driver
    // Else
    //   use GFX Driver.
    If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Volume UP
     // ADBG("Notify _HID 0xC4")
      Notify(\_SB.HIDD,0xC4) // Notify HID driver that Volume UP button is pressed.
    } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
      ^VGBI.UPBT(VOLUME_UP_BUTTON, One)
     // ADBG("Notify 0xC4")
      Notify(^VGBI, 0xC4) // Notify Virtual GPIO driver that Volume UP button is pressed.

      // Skylake SDS now has a separate event for Button release.
      //@todo Need to verify if this is required for PantherMountain or not
/*
      If (LEqual(BID, )) {
        ^VGBI.UPBT(VOLUME_UP_BUTTON, Zero)
       // ADBG("Notify 0xC5")
        Notify(^VGBI, 0xC5) // Notify Virtual GPIO driver that Volume UP button is released.
      }
*/
    } Else {
      If (LEqual(\_SB.HIDD.HRDY, 1)){
        \_SB.HIDD.HPEM(17)
      }// End of If (LEqual(\_SB.HIDD.HRDY, 1))
    } // If (LAnd(CondRefOf(^VGBI._STA)),(LEqual(And(^VGBI._STA(),One),One)))
  }  //end of Method(_Q80)

  Method(_Q81)    // Volume Down
  {
    P8XH(0,0x81)
   // Volume Down.
   // If VirtualButton driver loaded
   //   use VirtualButton driver
   // Else If HID Event Driver loaded
   //   use HID Event Driver
   // Else
   //   use GFX Driver.
   // ADBG("Volume Down")

   If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Volume Down
    // ADBG("Notify _HID 0xC6")
     Notify(\_SB.HIDD,0xC6) // Notify HID driver that Volume Down button is pressed.
   } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
     ^VGBI.UPBT(VOLUME_DOWN_BUTTON, One)
    // ADBG("Notify 0xC6")
     Notify(^VGBI, 0xC6) // Notify Virtual GPIO driver that Volue Down button is pressed.

     // Skylake SDS now has a separate event for Button release.
     //@todo Need to verify if this is required for PantherMountain or not
/*
     If (LEqual(BID, )) {
       ^VGBI.UPBT(VOLUME_DOWN_BUTTON, Zero)
      // ADBG("Notify 0xC7")
       Notify(^VGBI, 0xC7) // Notify Virtual GPIO driver that Volue Down button is released.
     }
*/
   } Else {
     If (LEqual(\_SB.HIDD.HRDY, 1)){
       \_SB.HIDD.HPEM(18)
     }
   }
  } //end of Method(_Q81)

  Method(_Q85)    // Windows Home button
  {
    P8XH(0,0x85)

    // Windows Home Button
    // If VirtualButton driver loaded
    //   use VirtualButton driver
    // Else If HID Event Driver loaded
    //   use HID Event Driver
    // Else
    //   use GFX Driver.
    // ADBG("Windows Home")

    If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Windows Home Press
     // ADBG("Notify _HID 0xC2")
      Notify(\_SB.HIDD,0xC2) // Notify HID driver that Windows Home button is pressed.
    } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
      ^VGBI.UPBT(WINDOWS_BUTTON, One)
     // ADBG("Notify 0xC2")
      Notify(^VGBI, 0xC2) // Notify Virtual GPIO driver that Windows Home button is pressed.

      // Skylake SDS now has a separate event for Button release.
      //@todo Need to verify if this is required for PantherMountain or not
/*
        If (LEqual(BID, )) {
          ^VGBI.UPBT(WINDOWS_BUTTON, Zero)
         // ADBG("Notify 0xC3")
          Notify(^VGBI, 0xC3) // Notify Virtual GPIO driver that Windows Home button is released.
        }
*/
    } Else {
        If (LEqual(\_SB.HIDD.HRDY, 1)){
          \_SB.HIDD.HPEM(1)
        }
    }

  } //end of Method(_Q85)

  Method(_Q86)    // Rotation Lock press event
  {
    P8XH(0,0x86)

    // Rotation Lock button
    // If VirtualButton driver loaded
    //   use VirtualButton driver
    // Else If HID Event Driver loaded
    //   use HID Event Driver
    // Else
    //   use GFX Driver.
    // ADBG("Rotation Lock P")

    If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: RotationLock Press
     // ADBG("Notify _HID 0xC8")
      Notify(\_SB.HIDD,0xC8) // Notify HID driver that Rotation Lock button is pressed.
    } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
      ^VGBI.UPBT(ROTATION_LOCK_BUTTON, One)
     // ADBG("Notify 0xC8")
      Notify(^VGBI, 0xC8) // Notify Virtual GPIO driver that Rotation Lock button is pressed.
      ^VGBI.UPBT(ROTATION_LOCK_BUTTON, Zero)
     // ADBG("Notify 0xC9")
      Notify(^VGBI, 0xC9) // Notify Virtual GPIO driver that Rotation Lock button is released.
    } Else {
      If (LEqual(\_SB.HIDD.HRDY, 1)){
        \_SB.HIDD.HPEM(2)
      }
    }

  }  // endof Method(_Q86)

  Method(_Q87)    // Mode Switch: Clam Shell (Laptop) mode and Slate mode switch
  {
    P8XH(0,0x87)

    // Rotation Lock button
    // If VirtualButton driver loaded
    //   use VirtualButton driver
    // Else
    //   use GFX Driver.
    // ADBG("Convertible Button")

    If (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
      Xor(PB1E, 0x08, PB1E) // Toggle Slate/Laptop Lock Status
      If (And(PB1E, 0x08)) // Slave/Laptop Mode changed 0 -> 1 Laptop mode
      {
        ^VGBI.UPBT(CONVERTIBLE_BUTTON, One)
       // ADBG("Notify 0xCD")
        Notify(^VGBI, 0xCD) // Notify Virtual GPIO driver that the system is now in Laptop mode.
      } Else { // Slave/Laptop Mode Changed 1 -> 0 Slate/Tablet Mode
        ^VGBI.UPBT(CONVERTIBLE_BUTTON, Zero)
       // ADBG("Notify 0xCC")
        Notify(^VGBI, 0xCC) // Notify Virtual GPIO driver that the system is now in Slate/Tablet mode.
      }
    }
  }  //End of Method(_Q87)

  Method(_Q88)    // Volume Up release event
  {
    P8XH(0,0x88)

     // ADBG("Vol Up Release")
     // Volume Up button release event.
     // If VirtualButton driver loaded
     //   use VirtualButton driver
     If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Volume UP Release
      // ADBG("Notify _HID 0xC5")
       Notify(\_SB.HIDD,0xC5) // Notify HID driver that Volume UP button is released.
     } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
       ^VGBI.UPBT(VOLUME_UP_BUTTON, Zero)
      // ADBG("Notify 0xC5")
       Notify(^VGBI, 0xC5) // Notify Virtual GPIO driver that Volume UP button is released.
     }
  }  //end of Method(_Q88)

  Method(_Q89)    // Volume Down release event
  {
    P8XH(0,0x89)

    // Volume Down button release event.
    // If VirtualButton driver loaded
    //   use VirtualButton driver
    // ADBG("Vol Down Release")

    If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Volume Down Release
     // ADBG("Notify _HID 0xC7")
      Notify(\_SB.HIDD,0xC7) // Notify HID driver that Volume UP button is released.
    } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
      ^VGBI.UPBT(VOLUME_DOWN_BUTTON, Zero)
     // ADBG("Notify 0xC7")
      Notify(^VGBI, 0xC7) // Notify Virtual GPIO driver that Volue Down button is released.
    }
  }  // end of Method(_Q89)

  Method(_Q8A)    // Windows Home button release event
  {
    P8XH(0,0x8A)

    // Windows Home Button release event
    // If VirtualButton driver loaded
    //   use VirtualButton driver
    // ADBG("Win Home release")

    If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Windows Home Button Release
     // ADBG("Notify _HID 0xC3")
      Notify(\_SB.HIDD,0xC3) // Notify HID driver that Windows Home button is released.
    } ElseIf (And(PB1E, 0x20)) {  // Check if VBDL Flag(PB1E Bit5) is set
      ^VGBI.UPBT(WINDOWS_BUTTON, Zero)
     // ADBG("Notify 0xC3")
      Notify(^VGBI, 0xC3) // Notify Virtual GPIO driver that Windows Home button is released.
    }
  }  // end of Method(_Q8A)

  Method(_Q8B)    // Rotation Lock release event
  {
    P8XH(0,0x8B)

    // Windows Rotation Lock release event
    // ADBG("Rotation Lock R")

    If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Rotation Lock Release
     // ADBG("Notify _HID 0xC9")
      Notify(\_SB.HIDD,0xC9) // Notify HID driver that Windows Home button is released.
    }
  } // end of Method(_Q8B)

  Method(_QF0)    // Thermal Event.
  {
    If (LEqual(DBGS,0))
    {
      // Only handle the numerous Thermal Events if we are NOT doing ACPI Debugging.
      If (CondRefOf(\_TZ.TZ01)) {
        Notify(\_TZ.TZ01,0x80)
      }
    }
  }  // end of Method(_QF0)

  Device(WDT0)  // WDT Device Resource Consumption
  {
    Name(_HID,EISAID("PNP0C02"))

    Name(_UID,3)

    Name(_CRS,ResourceTemplate()
    {
      IO(Decode16,0x6A4,0x6A4,0x1,0x1)  // 1 Byte EC Prv Intfc.
      IO(Decode16,0x6A0,0x6A0,0x1,0x1)  // 1 Byte EC Prv Intfc.
    })
  }  //end of Device(WDT0)

/************************************************************************;
;*
;* Name:  CHDK
;*
;* Description: Check DOCK status, returen True if Dock status equal Arg0
;*
;************************************************************************/

  Method(CHDK, 1)
  {
    // If not Mobile Platform then skip the code and return 0
    If (LEqual(ECON,1)){
      If (LEqual(ECRD(RefOf(DOCK)), Arg0))
        { // If not docked then it's hot plug
          Return(1)
        }
    }
    Return(0)
  }

  //
  // Hardware Button Array support
  //

  Device(BIND)  // Button Indicators.
  {
     Name(_HID, "INT33D2")
     Name(_CID, "PNP0C40")

     Method(_STA, 0,Serialized)
     {
//       If (LAnd(And(IUBE,1), LGreaterEqual(OSYS, 2013)))
//       {
         If (And(PB1E, 1)) // 10Sec Power Button is enabled?
         {
           Return(0x000B) // Not visible in UI
         }
         Else
         {
           Return(0x000F)
         }
//       }
       Return(0x00)
     }
     //
     // _DSM : Device Specific Method for the Windows Compatible Button Array.
     //
     // Arg0: UUID Unique function identifier
     // Arg1: Integer Revision Level
     // Arg2: Integer Function Index
     // Arg3: Package Parameters
     //
     Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
     {
       // Compare passed in UUID to supported UUID.

       If (LEqual(Arg0, ToUUID ("DFBCF3C5-E7A5-44E6-9C1F-29C76F6E059C")))
       {
         If (LEqual(0,ToInteger(Arg1)))        // Revision 0.
         {
           Switch (ToInteger(Arg2))            // Switch to Function Index.
           {
             //
             // Function 0, Query of supported functions.
             //

             Case (0)
             {
               Return (Buffer() {0x03})
             }

             //
             // Function 1, Windows Compatible Button Array Power Button Properties.
             //

             Case (1)
             {
               // Only return support if platform enabled via setup.

               If (LEqual(And(PB1E, 1), 1))
               {
                 Return (0x07)
               }

               Return(0x00)

             }
           } // End Switch statement
         }  // End Revision check
       }  // End UUID check

       // If the code falls through to this point, just return a buffer of 0.

       Return (Buffer() {0x00})

    }  // End _DSM Method
  }

  Device(CIND)  // Convertible Indicators.
  {
     Name(_HID, "INT33D3")
     Name(_CID, "PNP0C60")

     Method(_STA, 0,Serialized)
     {
       If (LAnd(And(IUCE,1), LGreaterEqual(OSYS, 2012)))
       {
         Return(0x0F)
       }
       Return(0x00)
     }
  }

  Device(DIND)  // Docking Indicators.
  {
     Name(_HID, "INT33D4")
     Name(_CID, "PNP0C70")
     Method(_STA, 0,Serialized)
     {
       If (LAnd(And(IUDE,1), LGreaterEqual(OSYS, 2012)))
       {
         Return(0x0F)
       }
       Return(0x00)
     }
  }

  Device(VGBI)                                  // Virtual GPIO Button "Interface."
  {

    Name(_HID, EISAID("INT33D6"))               // HID for Intel Virtual GPIO Button Interface.
    Name(VBDS,0)
    Name(ONTM,0) // Temporary variable to initialise VBDS only once in the boot

    Method(_STA,0,Serialized)
    {
      If (LGreaterEqual(OSYS,2013))              // Virtual Button applicable for Winblue and higher version of WinOS
      {
        ADBG("WinB VGBI STA")
        Return(0x0F)                            // Device present,enabled and should be shown in UI.
      } Else {
        Return(0x00)                            // Device NOT present
      }
    }// End of _STA

    Method(VBDL,0,Serialized) // Virtual Button Driver Load - Refer Bios Arch Spec
    {
      ADBG("VBDL")

      Or(PB1E, 0x20, PB1E) // Set VBDL Flag (Bit5).  This state must be kept after resuming from S3/S4.
                           // Virtual Button Driver can be loaded even the system doesn't support 10sec PB OVR.

      If (LEqual(And(PB1E,One),One)){ // 10 Sec Power Button Enabled in setup?
        ADBG("VBDL EN")

        //
        // Clear PBST so that we can hide the default power button.
        //
        If (CondRefOf(\_SB.PWRB.PBST))
        {
          Store(0, \_SB.PWRB.PBST)
          Notify(\_SB.PWRB, 1) // Device check
        }
        // First send 0x74 to EC to disable _Q54 event for Power button.
        If (CondRefOf(ECMD)){
          ADBG("Disable _Q54")
          ECMD(0x74)
        }
        // Set EC 10s enable bit.
        ECWT(1, RefOf(PB10))
      } Else {
        // 10s power button disabled, clear EC 10s enable bit.
        ADBG("VBDL DIS")
        ECWT(0, RefOf(PB10))

        //
        // Here we know V-GPIO driver is loaded but 10s PB is disabled.
        // Set PBST so that the default power button can handle the request.
        //
        If (CondRefOf(\_SB.PWRB.PBST))
        {
          Store(1, \_SB.PWRB.PBST)
          Notify(\_SB.PWRB, 1) // Device check
        }
      }
    }

    Method(VGBS,0,Serialized)  // Virtual GPIO Button Status - Refer Bios Arch Spec
    {
      // Bit[7] : Docking Indicatory Status
      // Bit[6] : Convertible/Slate Indicator Status
      // Bit[5] : Reserved
      // Bit[4] : Rotation Lock Button Status
      // Bit[3] : Volume Down Button Status
      // Bit[2] : Volume Up Button Status
      // Bit[1] : Windows Home Button Status
      // Bit[0] : Power Button Status

      ADBG("VGBS")
      If (LEqual(ONTM,0))
      {
        ADBG("ONTM = 0")
        // Initial setup option on VBDS and thereafter hotkeys should be updating the VBDS
        If (LEqual(And(PB1E,0x04),0x04)) // Rotation Lock
        {
          UPBT(ROTATION_LOCK_BUTTON,One)
        }

        If (LEqual(And(PB1E,0x08),0x08)) // Slate/Laptop
        {
          UPBT(CONVERTIBLE_BUTTON,One)
        }

        If (LEqual(And(PB1E,0x10),0x10))  // Undock/Dock
        {
          UPBT(DOCK_INDICATOR,One)
        }
        Store(One,ONTM)
      }
      Return(VBDS)
    }// End of Method

    //
    // UPBT Update Button Status
    //
    // Arg0: Bit location of the target button
    //       0: Power Button
    //       1: Windows Button
    //       2: Volume up Button
    //       3: Volume down Button
    //       4: Rotation Lock Button
    //       5: Reserved
    //       6: Convertible state 0 - Slate, 1 - Notebook
    //       7: Dock Indicator 0 - Undock, 1 - Dock
    //
    // Arg1: On/Off state, 0 - Clear the target bit, 1 - Set the target bit.
    //
    Method(UPBT,2,Serialized)  // Update Button Status
    {
      ShiftLeft(One, Arg0, Local0)
      If (Arg1){                // Button Press/ON
        Or(VBDS, Local0, VBDS)
      } Else {                 // Button Press/OFF
        And(VBDS, Not(Local0),VBDS)
      }
    } // End of UPBT

    Method(PBIN, 0, Serialized)
    {
      ADBG("VGBI PB INIT")

      // Clear VBDL Flag (PB1E Bit5)
      And(PB1E, Not(0x20), PB1E)

      //
      // Send EC 0x73 command to make 0x54 Power button notification is available.
      // This is needed to support no Virtual GPIO driver case.
      // Once the driver calls VBDL, VBDL sends 0x74(disable PB 0x54) to EC.
      //
      If (LAnd(And(PB1E, 1), LNot(ECRD(RefOf(PB10)))))
      {
        If (CondRefOf(\_SB.PWRB.PBST))
        {
          Store(1, \_SB.PWRB.PBST)
          Notify(\_SB.PWRB, 1) // Device check
        }
        If (CondRefOf(ECMD)){
          ADBG("Enable _Q54")
          ECMD(0x73)  // Enable Power Button 0x54 Notification
        }
      }
    }
  } // End of VGBI

  //
  // Define a Lid Switch.
  //
  Device(LID0)
  {
    Name(_HID,EISAID("PNP0C0D"))
    Method(_STA)
    {
      If (LEqual(ECON,1)){
        Return(0x0F)
      }
      Return(0x00)
    }
    Method (_LID, 0)
    {
//[-start-1609012-IB10860210-remove]//
//      P8XH (0, 0x15)
//[-end-1609012-IB10860210-remove]//
      // 0 = Closed, 1 = Open.
      Store (\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.LSTE)), Local0)
//[-start-1609012-IB10860210-remove]//
//      If (LEqual(Local0, 0)) {
//        P8XH (1, 1)
//      } Else {
//        P8XH (1, 2)
//      }
//[-end-1609012-IB10860210-remove]//
      Return(Local0)
    }
    Method(_PRW, 0)
    {
      Return(Package(){0x2C,4})       //GPIO_12  Waketime
    }
  }
} // End H_EC

//
// System Bus
//

Scope(\_SB)
{

  // Define an AC Device for ULT, ULX and Halo Platforms.

  Device(ADP1)
  {
    Name(_HID,"ACPI0003")

    Method(_STA)
    {
      If (LEqual(ECON,1)){
        Return(0x0F)
      }
      Return(0x00)
    }

    // Return the value that determines if running
    // from AC or not.

    Method(_PSR,0)
    {
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

  //
  // Define a (Control Method) Power Button.
  //

  Device(PWRB)
  {
    Name(_HID,EISAID("PNP0C0C"))

    // The PRW isn't working when
    // placed in any of the logical locations ( PS2K, PS2M,
    // H_EC ), so a Power Button Device was created specifically
    // for the WAKETIME_SCI PRW.

    //
    // Power button status flag used to communicate H_EC.VBDL
    //
    Name(PBST, 1)

    //
    // Up Press Register flag. Set when OS register to receive the up press of the power button
    //
    Name(UPPS, 0)

    //
    // Status of Power Button Level when EC is in mode where SCI is sent for both press and release of power button
    //
    Name(PBLV, 0)

    Method(PKG2, 2, Serialized) {
      Name (PKG, Package(2){0,0})
      Store(Arg0, Index(PKG,0))
      Store(Arg1, Index(PKG,1))
      Return (PKG)
    }

    Method(_PRW, 0)
    {
      Return(Package(){0x2C,4})       //GPIO_12  Waketime
    }

    Method(_STA, 0)
    {
      If (LAnd(LEqual(ECON,1), PBST)){
        Return(0x0F)
      }
      Return(0x00)
    }

    Method(PBUP, 0)
    {
      If(UPPS) {
        Notify(\_SB.PWRB, 0xC0) // Send release notification to Power Button device
      }
    }

    //
    // _DSM : Device Specific Method for the Power Button.
    //
    // Arg0: UUID Unique function identifier
    // Arg1: Integer Revision Level
    // Arg2: Integer Function Index
    // Arg3: Package Parameters
    //
    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
    {
      // Compare passed in UUID to supported UUID.
      If (LEqual(Arg0, ToUUID ("9C355BCB-35FA-44f7-8A67-447359C36A03")))
      {
        If (LEqual(0,ToInteger(Arg1)))        // Revision 0.
        {
          Switch (ToInteger(Arg2)) {           // Switch to Function Index.
            //
            // Function 0, Query of supported functions.
            //
            Case (0) {
              // Need check for platforms that support this
              Return (Buffer() {0x07}) // Supports Function 1,2.
            }

            //
            // Function 1, Register for Power Button Release Notify.
            //
            Case (1) {
              Store(0x1, UPPS) // OS has registered to receive notify on release of power button
              //
              // Place EC into mode where SCI is sent for both press and release of power button
              //
              \_SB.PCI0.LPCB.H_EC.ECMD(0x74)
              \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.PB10))
              Return(0x00)
            }

            //
            // Function 2, Power Button Level.
            //
            Case (2) {
              If (UPPS) { // If OS has registered to receive notify on release of power button
                If (LNot(PBLV)) {
                  return(0x00) // Power button not pressed
                } else {
                  return(0x01) // Power button pressed
                }
              } else {
                If (\_SB.PCI0.LPCB.PRBL) {
                  return(0x00) // Power button not pressed
                } else {
                  return(0x01) // Power button pressed
                }
              }
            }
          } // End Switch statement
        } // End Revision check
      } // End UUID check

      //
      // If the code falls through to this point, just return a buffer of 0.
      //
      Return (Buffer() {0x00})
    } // End _DSM Method

  }//end device PWRB

//[-start-161201-IB07400821-modify]//
//#if BXTI_PF_ENABLE
  Device(SLPB)
  {
    Name(_HID, EISAID("PNP0C0E"))
    Method(_STA, 0)
    {
      If (LEqual(IOTP(),0x00)) { // CCG CRB
        Return(0x00)
      }
      If (LEqual(ECON,1)){
        Return(0x0F)
      }
      Return(0x00)
    }
  } 
//#endif
//[-end-161201-IB07400821-modify]//

  Device(HIDD)                                       // HID Device.
  {
    Name(_HID,"INT33D5")                            // Intel Ultrabook HID Platform Event Driver.
    //Name(_CID,"PNP0C02")                            // Generic Motherboard Resources.

    Name (HBSY, 0)   // HID Busy
    Name (HIDX, 0)   // HID Index
    Name (HMDE, 0)   // HID Mode
    Name (HRDY, 0)   // HID Ready
    Name (BTLD, 0)   // Button Driver Loaded
    Name (BTS1, 0)   // Button Status
    Name (HEB1, 0x233F3)
    Name (HEB2, 0)

    Method(_STA,0,Serialized)                       // Status Method.
    {
//[-start-160506-IB07400723-modify]//
//[-start-160524-IB07400731-modify]//
//  #if BXTI_PF_ENABLE
      If (LEqual(ECON,1)){
        If(LLess(OSYS,2012)) {
          // Don't report this for WIN7
          Return(0)
        } else {
          Return(0xf)
        }
      } else {
        Return(0)
      }
//  #else
//      Return(0xf)
//  #endif
//[-end-160524-IB07400731-modify]//
//[-end-160506-IB07400723-modify]//
    }

    //
    // HID Driver Descriptor Method - Called by HID Driver during initialization
    // to obtain HID Descriptor information.
    //
    // Input: None
    //
    // Output:  Package containing a complete HID Descriptor information.
    //
    Method(HDDM,0,Serialized)
    {
       // Placeholder.
       Name(DPKG, Package(4) {0x11111111, 0x22222222, 0x33333333, 0x44444444})
       Return(DPKG)
    }

    //
    // HID Driver Event Method - Called by HID Driver to get the specific
    // platform event.
    //
    // Input: None
    //
    // Output: Platform HID Event.
    // Mode 0 = Index of HID Input Report, per pre-defined Table.
    // Mode 1 = Package containing a complete HID Input Report.
    //
    Method(HDEM,0,Serialized)
    {
       Store(0,HBSY)                          // Clear HID Busy.
       // Simple Mode is hardcoded for now.  Return Simple Mode HID Index Value.
       If (LEqual(HMDE,0))
       {
          Return(HIDX)
       }
       Return(HMDE)
    }

    //
    // HID Driver Mode Method - Called by HID Driver during initialization to get
    // the platform mode of operation.
    //
    // Input: None
    //
    // Output:  Mode the platform is running in.
    // 0 = Simple Mode.
    // 1 = Advanced Mode.
    //
    Method(HDMM,0,Serialized)
    {
       Return(HMDE)               // Return Mode of operation.
    }

    //
    // HID Driver Status Method - called by HID Driver to report platform readiness status.
    // Input: Driver Status.
    // 0 = Driver Unloaded.
    // 1 = Driver Loaded and Ready.
    //
    // Output: None
    //
    Method(HDSM,1,Serialized)
    {
      Store(Arg0,HRDY)              // Store HID Ready Status.
      // Eventually code will communicate to platform the Driver status (enabled/disabled).
    }

    //
    // HID Platform Event Method - called by Platform to communicate HID Event to Driver.
    // Input:
    // Mode 0 = Index of HID Event.
    // Mode 1 = Package containing a complete HID Report.
    //
    Method(HPEM,1,Serialized)                        // HID Platform Event Method.
    {
      Store(1,HBSY)                                 // Set HID Busy.
      // Simple Mode is hardcoded for now.  Simply store HID Index value.
      If (LEqual(HMDE,0))
      {
         Store(Arg0,HIDX)
      } Else {
         Store(Arg0,HIDX)
      }
      Notify(\_SB.HIDD,0xC0)                         // Notify Driver to get HID Event.
      Store(0,Local0)                                // Initialize Local0 as a timeout counter.
      While(LAnd(LLess(Local0,250),HBSY))            // Wait <= 1 second for Driver to ACK success.
      {
        Sleep(4)                                     // Delay 4 ms.
        Increment(Local0)                            // Increment Timeout.
      }
      If (LEqual(HBSY,1))                             // Failure?
      {
        Store(0,HBSY)                                // Yes.  Clear HID Busy Flag.
        Store(0,HIDX)                                // Set HID Simple Mode Index = 0 = Undefined.
        Return(1)                                    // Return Failure.
      } Else {
        Return(0)                                    // Return Success.
      }
    }  //end of Method(HPEM)

    //
    // HID Button Load Method - called by Platform to say HID driver is capable of receiving 5-button array notifies.
    // Input: None
    //
    // Output: None
    //
    Method(BTNL,0,Serialized) // HID Button Enable/Disable Method
    {
      //
      // Clear PBST so that we can hide the default power button.
      //
      If (CondRefOf(\_SB.PWRB.PBST))
      {
        Store(0, \_SB.PWRB.PBST)
        Notify(\_SB.PWRB, 1) // Device check
      }
      Store(One, \_SB.HIDD.BTLD)
      //
      // Enable all buttons
      //
        // Set EC 10s enable bit.
        \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.PB10))

        //Enable SCIs for HID driver in EC.
      Store(0x1F, BTS1)
      \_SB.PCI0.LPCB.H_EC.ECWT(BTS1, RefOf(\_SB.PCI0.LPCB.H_EC.BTEN)) //Button Enable/Disable field in ACPI name space
      \_SB.PCI0.LPCB.H_EC.ECMD(0x38) //Enable/Disable SCIs from buttons
    }  //end of MEthod(BTNL)

    //
    // HID Button Enable/Disable Method - called by Platform to disable/enable notification based on button press
    // Input:
    // Arg0 = Bit mask of buttons to Enable or Disable: 1- Button should be Enabled, 0- Button should be Disabled
    //   Bits[0]: Power Button N/A to disable
    //   Bits[1]: Windows Button
    //   Bits[2]: Volume Up Button
    //   Bits[3]: Volume Down Button
    //   Bits[4]: Rotation Lock Button
    //   Bits[5:31]: Reserved
    //
    // Output: None
    //
    Method(BTNE,1,Serialized) // HID Button Enable/Disable Method
    {
      Store(Or(And(Arg0, 0x1E),0x01), BTS1) //Mask off PB Enable/Disable
      \_SB.PCI0.LPCB.H_EC.ECWT(BTS1, RefOf(\_SB.PCI0.LPCB.H_EC.BTEN)) //Button Enable/Disable field in ACPI name space
      \_SB.PCI0.LPCB.H_EC.ECMD(0x38) //Enable/Disable SCIs from buttons
    } //end of Method(BTNE)

    //
    // HID Button Status - called by Platform to get what buttons are enabled and disabled
    // Input: None
    //
    // Output: Bit mask of buttons' current status: 1- Button is Enabled, 0- Button is Disabled
    //   Bits[0]: Power Button N/A to disable
    //   Bits[1]: Windows Button
    //   Bits[2]: Volume Up Button
    //   Bits[3]: Volume Down Button
    //   Bits[4]: Rotation Lock Button
    //   Bits[5:31]: Reserved
    //
    Method(BTNS,0,Serialized)
    {
      Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.BTEN)), BTS1) //Button Enable/Disable field in ACPI name space
      Return(BTS1)
    }  //end of Method(BTNS)

    //
    // HID Button Capabilities Method - called by Platform to determine what buttons are supported
    // Input: None
    //
    // Output: Bit mask of buttons supported: 1- Button is Supported, 0- Button is not Supported
    //   Bits[0]: Power Button (Must be 1)
    //   Bits[1]: Windows Button
    //   Bits[2]: Volume Up Button
    //   Bits[3]: Volume Down Button
    //   Bits[4]: Rotation Lock Button
    //   Bits[5:31]: Reserved
    //
    Method(BTNC,0,Serialized) // HID Button Capabilities Method
    {
      Return(0x1F)
    }  //end of MEthos(BTNC)

    //
    // HEBC: HID Event Base Capabilities [31:0]- To specify the base button capabilities supported on platform by returning a ULONG value with the following bit level definition
    // Input: None
    //
    // 0 = Button not supported
    // 1 = Button supported
    // Output:
    // Bits [0] - Windows Button (Windows 8.1 supported), Rotation Lock (Windows 8.1 supported), Num Lock, Home, End, Page Up, Page Down
    // Bits [1] - Wireless Radio Control
    // Bits [2] - System Power Down (Windows 8.1 supported)
    // Bits [3] - System Hibernate
    // Bits [4] - System Sleep/ System Wake
    // Bits [5] - Scan Next Track
    // Bits [6] - Scan Previous Track
    // Bits [7] - Stop
    // Bits [8] - Play/Pause
    // Bits [9] - Mute
    // Bits [10] - Volume Increment (Windows 8.1 supported)
    // Bits [11] - Volume Decrement (Windows 8.1 supported)
    // Bits [12] - Display Brightness Increment
    // Bits [13] - Display Brightness Decrement
    // Bits [14] - Lock Tablet
    // Bits [15] - Release Tablet
    // Bits [16] - Toggle Bezel
    // Bits [17] - 5 button array (Windows 10 supported - Power, Windows Home, Volume Up, Volume Down, Rotation Lock)
    // Bits [18-31] - reserved
    Method(HEBC,0,Serialized)
    {
      Return(HEB1)
    }

    //
    // HEEC- Hid Event Extended Capabilities [32:63]
    //
    Method(HEEC,0,Serialized)
    {
      Return(HEB2)
    }
  }  //End of Device(HIDD)

    Method(PWPR, 0, Serialized) // Power Button Press Helper Method
    {
      If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Power Button Press
       // ADBG("Notify _HID 0xCE")
        Notify(\_SB.HIDD,0xCE) // Notify HID driver that Power button is pressed.
      } Else {
        If(LEqual(\_SB.PWRB.PBST, 0x1)) { // Using Control Method Power Button
          Notify(\_SB.PWRB, 0x80) // Send press notification to Power Button device
          Store(1, \_SB.PWRB.PBLV) // Keep track of Power Button Level
        }
        If (LAnd(CondRefOf(\_SB.PCI0.LPCB.H_EC.VGBI._STA),LEqual(And(\_SB.PCI0.LPCB.H_EC.VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
        {
          \_SB.PCI0.LPCB.H_EC.VGBI.UPBT(POWER_BUTTON, One)
         // ADBG("Notify 0xC0")
          \_SB.PCI0.LPCB.H_EC.ECMD (0x2D) // Wake up EC first. This is needed to get the release event immediately after press event.
          Notify(\_SB.PCI0.LPCB.H_EC.VGBI, 0xC0) // Notify Virtual GPIO driver that the power button is pressed.
        } Else {
          If (CondRefOf(\_SB.PCI0.GFX0.IUER))
          {
            Store(\_SB.PCI0.GFX0.IUER, Local0)
            And(Local0, 0xC0, \_SB.PCI0.GFX0.IUER)  // Clear 4:0 button events on entry.
            Store(\_SB.PCI0.GFX0.IUER, Local0)
            Or(Local0, 0x01, \_SB.PCI0.GFX0.IUER)   // Set Power Button Status = pressed.
          }
        } // End IF
      } //If Win10
    } //End Method (PWPR)

    Method(PWRR, 0, Serialized) // Power Button Release Helper Method
    {
      If (LAnd(LGreaterEqual(OSYS, 2015), \_SB.HIDD.BTLD)) { //Win10 and Button Driver loaded: Power Button Release
      // ADBG("Notify _HID 0xCF")
        Notify(\_SB.HIDD,0xCF) // Notify HID driver that Power button is released.
      } Else {
        If(LEqual(\_SB.PWRB.PBST, 0x1)) { // Using Control Method Power Button
          \_SB.PWRB.PBUP() // Send release notification to Power Button device if requested
          Store(0, \_SB.PWRB.PBLV) //// Keep track of Power Button Level
        }
        If (LAnd(CondRefOf(\_SB.PCI0.LPCB.H_EC.VGBI._STA),LEqual(And(\_SB.PCI0.LPCB.H_EC.VGBI._STA(),One),One))) // VirtualButton driver is loaded(means it is Winblue)
        {
          \_SB.PCI0.LPCB.H_EC.VGBI.UPBT(POWER_BUTTON, Zero)
         // ADBG("Notify 0xC1")
          Notify(\_SB.PCI0.LPCB.H_EC.VGBI, 0xC1)  // Notify Virtual GPIO driver that the power button is released.
        } Else {
          If (CondRefOf(\_SB.PCI0.GFX0.IUER))
          {
            Store(\_SB.PCI0.GFX0.IUER, Local0)
            And(Local0, 0xC0, \_SB.PCI0.GFX0.IUER)  // Clear 4:0 button events on entry.
          }
        }
      } //If Win10
    } //End Method(PWRR)
}//end scope _SB
