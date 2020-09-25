/** @file
  ACPI DSDT table

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2018 Intel Corporation.

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


// Define the following External variables to prevent a WARNING when
// using ASL.EXE and an ERROR when using IASL.EXE.

External(PDC0)
External(PDC1)
External(PDC2)
External(PDC3)
//External(CFGD)
External(\_PR.CPU0._PPC, IntObj)
External(\_SB.PCI0.LPCB.TPM.PTS, MethodObj)
//External(\_SB.PCI0.I2C1.BATC, DeviceObj)
Name(ECUP, 1)  // EC State indicator: 1- Normal Mode 0- Low Power Mode
Mutex(EHLD, 0) // EC Hold indicator: 0- No one accessing the EC Power State 1- Someone else is accessing the EC Power State

External(\_SB.CHRG,DeviceObj)
External(\_SB.IETM,DeviceObj)
External(\_SB.IETM.ODVP,MethodObj)
External(\_SB.IETM.ODVX,PkgObj)
External(\_SB.TPM.PTS, MethodObj)
//External(\_SB.PCI0.SBIM, MethodObj)

#define CONVERTIBLE_BUTTON   6
#define DOCK_INDICATOR       7

//[-start-161109-IB07400810-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// Apollo Lake RVP : GPIO_116(H57) -- SOC_CODEC_IRQ -- ALC298.HD_I2S_SEL/IRQOUT/GPIO0
// APL-I Oxbow Hill: GPIO_116(H57) -- GP_SSP_1_RXD -- J6C1.7 (Header)
//
#define NW_GPIO_116           0x00C40728    //GP_SSP_1_RXD
#endif
//[-end-161109-IB07400810-add]//

Name (ADW1, 0)


//
// Create a Global MUTEX.
//
Mutex(MUTX,0)

// Define Port 80 as an ACPI Operating Region to use for debugging.  Please
// note that the Intel CRBs have the ability to ouput an entire DWord to
// Port 80h for debugging purposes, so the model implemented here may not be
// able to be used on OEM Designs.

OperationRegion(PRT0,SystemIO,0x80,4)
Field(PRT0,DwordAcc,Lock,Preserve)
{
  P80H, 32
}

// Port 80h Update:
//    Update 8 bits of the 32-bit Port 80h.
//
//  Arguments:
//    Arg0: 0 = Write Port 80h, Bits 7:0 Only.
//            1 = Write Port 80h, Bits 15:8 Only.
//            2 = Write Port 80h, Bits 23:16 Only.
//            3 = Write Port 80h, Bits 31:24 Only.
//    Arg1: 8-bit Value to write
//
//  Return Value:
//    None

Method(P8XH,2,Serialized)
{
  If(LEqual(Arg0,0))    // Write Port 80h, Bits 7:0.
  {
    Store(Or(And(P80D,0xFFFFFF00),Arg1),P80D)
  }

  If(LEqual(Arg0,1))    // Write Port 80h, Bits 15:8.
  {
    Store(Or(And(P80D,0xFFFF00FF),ShiftLeft(Arg1,8)),P80D)
  }

  If(LEqual(Arg0,2))    // Write Port 80h, Bits 23:16.
  {
    Store(Or(And(P80D,0xFF00FFFF),ShiftLeft(Arg1,16)),P80D)
  }

  If(LEqual(Arg0,3))    // Write Port 80h, Bits 31:24.
  {
    Store(Or(And(P80D,0x00FFFFFF),ShiftLeft(Arg1,24)),P80D)
  }

  Store(P80D,P80H)
}

//[-start-160828-IB07400775-add]//
//
// Check is Embedded CRB or not
//
//  Arguments:
//    None
//
//  Return Value:
//    1 - IOTG CRB
//    0 - Not IOTG CRB
//
Method(IOTP,0)
{
//[-start-160907-IB07400780-remove]//
//  P8XH(0,BDID)
//[-end-160907-IB07400780-remove]//
  
  If(LEqual(BDID,0x06)) { // Oxbow Hill
    Return(1)  
  }
  If(LEqual(BDID,0x07)) { // Leaf Hill
    Return(1)  
  }
  If(LEqual(BDID,0x08)) { // Juniper Hill
    Return(1)  
  }
  If(LEqual(BDID,0x0D)) { // Gordon Ridge BMP Interposer
    Return(1)  
  }
  If(LEqual(BDID,0x0F)) { // Gordon Ridge BMP MRB
    Return(1)  
  }
  
  Return(0)  
}
//[-end-160828-IB07400775-add]//

Method(ADBG,1,Serialized)
{
  Return(0)
}

//
// Define SW SMI port as an ACPI Operating Region to use for generate SW SMI.
//
OperationRegion (SPRT, SystemIO, 0xB2, 2)
Field (SPRT, ByteAcc, Lock, Preserve) {
  SSMP, 8,
//[-start-160923-IB07400789-add]//
  SSMD, 8
//[-end-160923-IB07400789-add]//
}

// The _PIC Control Method is optional for ACPI design.  It allows the
// OS to inform the ASL code which interrupt controller is being used,
// the 8259 or APIC.  The reference code in this document will address
// PCI IRQ Routing and resource allocation for both cases.
//
// The values passed into _PIC are:
//   0 = 8259
//   1 = IOAPIC

Method(\_PIC,1)
{
  Store(Arg0,GPIC)
  Store(Arg0,PICM)
}

OperationRegion(SWC0, SystemIO, 0x610, 0x0F)
Field(SWC0, ByteAcc, NoLock, Preserve)
{
  G1S, 8,      //SWC GPE1_STS
  Offset(0x4),
  G1E, 8,
  Offset(0xA),
  G1S2, 8,     //SWC GPE1_STS_2
  G1S3, 8      //SWC GPE1_STS_3
}

OperationRegion (SWC1, SystemIO, \PMBS, 0x34)
Field(SWC1, DWordAcc, NoLock, Preserve)
{
  Offset(0x20),
  G0S, 32,      //GPE0_STS
  Offset(0x30),
  G0EN, 32      //GPE0_EN
}


OperationRegion (PMCM, SystemMemory, Add(DD1A,0x1000), 0x1000)
Field (PMCM, ByteAcc, NoLock, Preserve)
{
  Offset (0x94),
  DHPD, 32,       // DISPLAY_HPD_CTL
}

// Prepare to Sleep.  The hook is called when the OS is about to
// enter a sleep state.  The argument passed is the numeric value of
// the Sx state.

Method(_PTS,1)
{
//[-start-160609-IB07400742-modify]//
//  Store(0,P80D)   // Zero out the entire Port 80h DWord.
//  P8XH(0,Arg0)    // Output Sleep State to Port 80h, Byte 0.
  //
  // Please refer PostCode.h
  //
//PRJ+ >>>> Modify MIPI panel power sequency
  Store(0,\_SB.GPO1.MBKL)   //Set GPIO_197 PANEL1_BKLTEN to low
  Sleep(10)
  Store(0,\_SB.GPO0.MRST)   //Set GPIO_9 MIPI RST to low
  Sleep(130)
  Store(0,\_SB.GPO1.MVDD)   //Set GPIO_196 PANEL1_VDDEN Panel
  Sleep(5)
//PRJ+ <<<< Modify MIPI panel power sequency  
  If (LEqual (Arg0, 1))
  {
    P8XH(0,0x51) // ASL_ENTER_S1
  }
  
  If (LEqual (Arg0, 3))
  {
    P8XH(0,0x53) // ASL_ENTER_S3
  }
  
  If (LEqual (Arg0, 4))
  {
    P8XH(0,0x54) // ASL_ENTER_S4
  }
  
  If (LEqual (Arg0, 5))
  {
    P8XH(0,0x55) // ASL_ENTER_S5
  }
//[-end-160609-IB07400742-modify]//

  //clear the 3 SWC status bits
  Store(Ones, G1S3)
  Store(Ones, G1S2)
  Store(1, G1S)

  //set SWC GPE1_EN
  Store(1,G1E)

  //clear GPE0_STS
  Store(Ones, G0S)

  //
  // Call TPM PTS method
  //
  If(CondRefOf(\_SB.TPM.PTS))
  {
    \_SB.TPM.PTS (Arg0)
  }
  
//[-start-161109-IB07400810-modify]//
#ifdef APOLLOLAKE_CRB
  //
  // Set GPIO_116 (SOC_CODEC_IRQ) 20k pull-down for device I2S audio codec INT343A before enter S3/S4
  //
  If (LAnd (LEqual (IOBF, 3), LEqual (IS3A, 1))) {
    If (LEqual(IOTP(),0x00)) { // CCG Board IDs
      Store (\_SB.GPC1 (NW_GPIO_116), ADW1)
      Or (ADW1, 0xFFFFC3FF, ADW1)
      And (ADW1, 0x00001000, ADW1)
      \_SB.SPC1 (NW_GPIO_116, ADW1)
    }
  }
#endif
//[-end-161109-IB07400810-modify]//
//[-start-180803-IB07400993-modify]//
  //
  // Below codes is come from Intel platform code,
  // Refer Intel IOTG BIOS to remove this code to fix wake on USB issue under Linux OS.
  //
//  //
//  // Fixed unexpected wakeup event from XHCI when system is entering S3.
//  //
//  If (LEqual(Arg0, 3)) {
//    If(CondRefOf(\_SB.PCI0.SBIM)) {
//      Store (\_SB.PCI0.SBIM (0xA20080A4, 0, 0x0, 0x30A8), local0)
//      Or (Local0, 0x10000000, Local0)
//      \_SB.PCI0.SBIM (0xA20080A4, local0, 0x1, 0x30A8)
//  }
//[-end-180803-IB07400993-modify]//
//[-start-170626-IB07400880-add]//
  If(Arg0)
  {
    COMMON_ASL_LPC_PATH.ECFG(0)                 // Clear ACPI EC driver ready flag
  }

  If (LEqual (Arg0, 5))
  {
    Store(0x01, SSMD)
    Store(0x61, SSMP) // PTS S5 SMI
    Sleep(Multiply(WS5T,1000))
    Store(0x02, SSMD)
    Store(0x61, SSMP) // PTS S5 SMI
  }
//[-end-170626-IB07400880-add]//
}

// Wake.  This hook is called when the OS is about to wake from a
// sleep state.  The argument passed is the numeric value of the
// sleep state the system is waking from.

Method(_WAK,1,Serialized)
{
//[-start-190314-IB07401090-add]//
#ifndef DISABLE_MSR_CTR_OVERRIDE_HANDLER
  Store(0x5C,SSMP) //Triggering SMM 
#endif  
//[-end-190314-IB07401090-add]//
//[-start-160609-IB07400742-modify]//
  Notify(\_SB.PWR2,0x02)
  
//  P8XH(1,0xAB) // Beginning of _WAK.
  COMMON_ASL_LPC_PATH.ECFG(1)  //Set ACPI EC driver ready flag

  If(LEqual(Arg0,1))
  {
    //
    // #define ASL_WAKEUP_S1   0xE1    //System wakeup from S1 (PostCode.h)
    //
    P8XH (0, 0xE1)
  }

  If(LEqual(Arg0,3))
  {
	  //
    // #define ASL_WAKEUP_S3   0xE3    //System wakeup from S3 (PostCode.h)
    //
    P8XH (0, 0xE3)
//PRJ+ >>>> Update GOP to 10.0.1039    
    Store(1,\_SB.GPO1.MVDD)   //Set GPIO_196 PANEL1_VDDEN Panel
    Sleep(20)
    Store(1,\_SB.GPO0.MRST)   //Set GPIO_9 MIPI RST to low
    Sleep(980)
    Store(1,\_SB.GPO1.MBKL)   //Set GPIO_198 PANEL1_BKLCTL to low
//PRJ+ <<<< Update GOP to 10.0.1039    
  }
  If(LEqual(Arg0,4))
  {
    //
    // #define ASL_WAKEUP_S4   0xE4    //System wakeup from S4 (PostCode.h)
    //
    P8XH (0, 0xE4)
  }
//[-end-160609-IB07400742-modify]//

  If(NEXP)
  {
    // Reinitialize the Native PCI Express after resume
    If(And(OSCC,0x02))
    {
      \_SB.PCI0.NHPG()
    }

    If(And(OSCC,0x04)) // PME control granted?
    {
      \_SB.PCI0.NPME()
    }
  }
//[-start-161201-IB07400821-modify]//
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  //
  // CRB EC code
  //
  If(LEqual(\ECON,1)) {
    If(\_SB.PCI0.LPCB.H_EC.ECAV) {
      If (LEqual(And(PB1E,One),One)){ // 10 Sec Power Button Enabled in setup?
        // Set EC 10s enable bit.
        \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.PB10))
      } Else {
        // ReSet EC 10s enable bit.
        \_SB.PCI0.LPCB.H_EC.ECWT(0, RefOf(\_SB.PCI0.LPCB.H_EC.PB10))
      }
    }
  }
#else
  //
  // OEM EC code
  //
#endif
#endif
//[-end-161206-IB07400822-modify]//
//[-end-161201-IB07400821-modify]//

  If(LOr(LEqual(Arg0,3), LEqual(Arg0,4))) // If S3 or S4 Resume
  {
    If(LEqual(PFLV,FMBL))
    {

    }

//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // CRB EC code
    //
    If(LEqual(\ECON,1))
    {
      // Update Lid state after S3 or S4 resume
      Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.LSTE)), LIDS)

      If(IGDS)
      {
        If (LEqual(LIDS, 0))
        {
          Store(0x0,\_SB.PCI0.GFX0.CLID)
        }
        If (LEqual(LIDS, 1))
        {
          Store(0x3,\_SB.PCI0.GFX0.CLID)
        }
        If (LEqual(IRMC, 0))
        {
          Notify(\_SB.PCI0.LPCB.H_EC.LID0, 0x80)
        }
      }
    }
#else
    //
    // OEM EC code
    //
#endif
#endif
//[-end-161206-IB07400822-modify]//

    //
    // if battery has changed from previous state i.e after Hibernate or Standby
    // then update the PWRS and update the SMM Power state
    //

    // Detect the change of Power State.
//[-start-161201-IB07400821-modify]//
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // CRB EC code
    //
    If(LEqual(\ECON,1)) {
      If(LEqual(BNUM,0))
      {
        If(LNotEqual(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.VPWR)),PWRS))
        {
          Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.VPWR)),PWRS)
          // Perform needed ACPI Notifications.
          PNOT()
        }
      }
      Else
      {
        If(LNotEqual(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.RPWR)),PWRS))
        {
          Store(\_SB.PCI0.LPCB.H_EC.ECRD(RefOf(\_SB.PCI0.LPCB.H_EC.RPWR)),PWRS)
          // Perform needed ACPI Notifications.
          PNOT()
        }
      }
    }
#else
    //
    // OEM EC code
    //
#endif
#endif
//[-end-161206-IB07400822-modify]//
//[-end-161201-IB07400821-modify]//

    // If CMP is enabled, we may need to restore the C-State and/or
    // P-State configuration, as it may have been saved before the
    // configuration was finalized based on OS/driver support.
    //
    //   CFGD[24]  = Two or more cores enabled
    //
    //If(And(CFGD,0x01000000))
    //{
      //
      // If CMP and the OSYS is WinXP SP1, we will enable C1-SMI if
      // C-States are enabled.
      //
      //   CFGD[7:4] = C4, C3, C2, C1 Capable/Enabled
      //
      //
    //}

    // Windows XP SP2 does not properly restore the P-State
    // upon resume from S4 or S3 with degrade modes enabled.
    // Use the existing _PPC methods to cycle the available
    // P-States such that the processor ends up running at
    // the proper P-State.
    //
    // Note:  For S4, another possible W/A is to always boot
    // the system in LFM.
    //
    If(LEqual(OSYS,2002))
    {
      //If(And(CFGD,0x01))
      //{
        If(LGreater(\_PR.CPU0._PPC,0))
        {
          Subtract(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
          Add(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
        }
        Else
        {
          Add(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
          Subtract(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
        }
      //}
    }

//[-start-161201-IB07400821-modify]//
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // CRB EC code
    //
    If(LEqual(\ECON,1)) {

      // Check to send Convertible/Dock state changes upon resume from Sx.
      If(And(GBSX,0x01))
      {
        //
        //  Do the same thing for Virtul Button device.
        //  Toggle Bit3 of PB1E(Slate/Notebook status)
        //
        Xor(PB1E, 0x08, PB1E)

        //
        // Update VGBS(V-GPIO Button state) accordingly.
        //
        If(And(PB1E, 0x08)){
          If(LAnd(CondRefOf(\_SB.PCI0.LPCB.H_EC.VGBI), \ECON))
          {
            \_SB.PCI0.LPCB.H_EC.VGBI.UPBT(CONVERTIBLE_BUTTON, One)
          }
        }
        Else
        {
          If(LAnd(CondRefOf(\_SB.PCI0.LPCB.H_EC.VGBI), \ECON))
          {
            \_SB.PCI0.LPCB.H_EC.VGBI.UPBT(CONVERTIBLE_BUTTON, Zero)
          }
        }
      }

      If(And(GBSX,0x02))
      {
        //
        //  Do the same thing for Virtul Button device.
        //  Toggle Bit4 of PB1E (Dock/Undock status)
        //
        Xor(PB1E, 0x10, PB1E)

        //
        // Update VGBS(Virtual Button state) accordingly.
        //
        If(And(PB1E, 0x10))
        {
          If(LAnd(CondRefOf(\_SB.PCI0.LPCB.H_EC.VGBI), \ECON))
          {
            \_SB.PCI0.LPCB.H_EC.VGBI.UPBT(DOCK_INDICATOR, One)
          }
        }
        Else
        {
          If(LAnd(CondRefOf(\_SB.PCI0.LPCB.H_EC.VGBI), \ECON))
          {
            \_SB.PCI0.LPCB.H_EC.VGBI.UPBT(DOCK_INDICATOR, Zero)
          }
        }
      }

//#if BXTI_PF_ENABLE
      If (LEqual(IOTP(),0x01)) { // IOTG Board IDs
        Notify(\_SB.SLPB, 0x02) 
      } 
//#endif
    }  
#else
    //
    // OEM EC code
    //
#endif
#endif
//[-end-161206-IB07400822-modify]//
//[-end-161201-IB07400821-modify]//
    //
    // Invoke SD card wake up method
    //
    \_SB.PCI0.SDC.WAK()
  }
  Return(Package(){0,0})
}

/*
// Get Buffer:
//    This method will take a buffer passed into the method and
//    create then return a smaller buffer based on the pointer
//    and size parameters passed in.
//
//  Arguments:
//    Arg0: Pointer to start of new Buffer in passed in Buffer.
//    Arg1: Size of Buffer to create.
//    Arg2: Original Buffer
//
//  Return Value:
//    Newly created buffer.

Method(GETB,3,Serialized)
{
  Multiply(Arg0,8,Local0)     // Convert Index.
  Multiply(Arg1,8,Local1)     // Convert Size.
  CreateField(Arg2,Local0,Local1,TBF3)  // Create Buffer.

  Return(TBF3)        // Return Buffer.
}
*/

// Power Notification:
//    Perform all needed OS notifications during a
//    Power Switch.
//
//  Arguments:
//    None
//
//  Return Value:
//    None

Method(PNOT,0,Serialized)
{
  //
  // If MP enabled and driver support is present, notify both
  // processors.
  //
  If(MPEN)
  {
    If(And(PDC0,0x0008))
    {
      Notify(\_PR.CPU0,0x80)    // Eval CPU0 _PPC.

      If(And(PDC0,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU0,0x81)  // Eval _CST.
      }
    }

    If(And(PDC1,0x0008))
    {
      Notify(\_PR.CPU1,0x80)    // Eval CPU1 _PPC.

      If(And(PDC1,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU1,0x81)  // Eval _CST.
      }
    }

    If(And(PDC2,0x0008))
    {
      Notify(\_PR.CPU2,0x80)    // Eval CPU2 _PPC.

      If(And(PDC2,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU2,0x81)  // Eval _CST.
      }
    }

    If(And(PDC3,0x0008))
    {
      Notify(\_PR.CPU3,0x80)    // Eval CPU3 _PPC.

      If(And(PDC3,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU3,0x81)  // Eval _CST.
      }
    }
  } Else {
    Notify(\_PR.CPU0,0x80)      // Eval _PPC.
    Sleep(100)
    Notify(\_PR.CPU0,0x81)      // Eval _CST
  }

  //
  // Perform update to all Batteries in the System.
  //
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  //
  // CRB EC code
  //
  If(\_SB.PCI0.LPCB.H_EC.ECAV)
  {
    Notify(\_SB.PCI0.LPCB.H_EC.BAT0,0x81)       // Eval BAT0 _BST.
    Notify(\_SB.PCI0.LPCB.H_EC.BAT1,0x81)       // Eval BAT1 _BST.
  }
#else
  //
  // OEM EC code
  //
#endif
#endif
//[-end-161206-IB07400822-modify]//

  If (LEqual(\DPTE,1)) {
    Notify(\_SB.IETM, 0x86) // Notification sent to DPTF driver (Policy) for PDRT reevaluation after AC/DC transtion has occurred.
    If (LEqual(\CHGE,1)){
      Notify(\_SB.CHRG, 0x80) // PPPC/PPDL reevaluation after AC/DC transtion has occurred.
    }
  }
} //end of PNOT

//
//
//
//
// Memory window to the CTDP registers starting at MCHBAR+5000h.
//
//  OperationRegion (MBAR, SystemMemory, Add(\_SB.PCI0.GMHB(),0x5000), 0x1000)
//  Field (MBAR, ByteAcc, NoLock, Preserve)
//  {
//    Offset (0x938), // PACKAGE_POWER_SKU_UNIT (MCHBAR+0x5938)
//    PWRU,  4,       // Power Units [3:0] unit value is calculated by 1 W / Power(2,PWR_UNIT). The default value of 0011b corresponds to 1/8 W.
//    Offset (0x9A0), // TURBO_POWER_LIMIT1 (MCHBAR+0x59A0)
//    PPL1, 15,       // PKG_PWR_LIM_1 [14:0]
//    PL1E,1,         // PKG_PWR_LIM1_EN [15]
//    CLP1,1,         // Package Clamping Limitation 1
//  }
Name(CLMP, 0) // save the clamp bit
Name(PLEN, 0) // save the power limit enable bit
Name(PLSV, 0x8000) // save value of PL1 upon entering CS
Name(CSEM, 0) //semaphore to avoid multiple calls to SPL1.  SPL1/RPL1 must always be called in pairs, like push/pop off a stack
//
// SPL1 (Set PL1 to 4.5 watts with clamp bit set)
//   Per Legacy Thermal management CS requirements, we would like to set the PL1 limit when entering CS to 4.5W with clamp bit set via MMIO.
//   This can be done in the ACPI object which gets called by graphics driver during CS Entry.
//   Likewise, during CS exit, the BIOS must reset the PL1 value to the previous value prior to CS entry and reset the clamp bit.
//
//  Arguments:
//    None
//
//  Return Value:
//    None
Method(SPL1,0,Serialized)
{
    If (LEqual(CSEM, 1))
    {
      Return() // we have already been called, must have CS exit before calling again
    }
    Store(1, CSEM) // record first call
}
//
// RPL1 (Restore the PL1 register to the values prior to CS entry)
//
//  Arguments:
//    None
//
//  Return Value:
//    None
Method(RPL1,0,Serialized)
{
    Store(0, CSEM)      // restore semaphore
}

Name(DDPS, 0) // Current Display Power Status. 0= D0; non-zero = Dx state;. Initial value is zero.
Name(UAMS, 0) // User Absent Mode state, Zero - User Present; non-Zero - User not present
// GUAM - Global User Absent Mode
//    Run when a change to User Absent mode is made,  e.g. screen/display on/off events.
//    Any device that needs notifications of these events includes its own UAMN Control Method.
//
//    Arguments:
//      Power State:
//        00h = On
//        01h = Standby
//        02h = Suspend
//        04h = Off
//        08h = Reduced On
//
//    Return Value:
//      None
//
Method(GUAM,1,Serialized)
{
  If(LNotEqual(Arg0, DDPS)){ // Display controller D-State changed?
    Store(Arg0, DDPS) //Update DDPS to current state
    Store(LAnd(Arg0, LNot(PWRS)), UAMS) // UAMS: User Absent Mode state, Zero - User Present; non-Zero - User not present

    //Port 80 code for CS
    If(Arg0){
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      if(LEqual(ECNO,0x01)){ // Check EC notification enabled in Setup
        ADBG("EC Notify")
        if(LEqual(ECDB,0x01)){
          ADBG("EC Debug")
          \_SB.PCI0.LPCB.H_EC.ECWT(1, RefOf(\_SB.PCI0.LPCB.H_EC.DLED)) //Set EC CS Debug Light (CAPS LOCK)
        }
        \_SB.PCI0.LPCB.H_EC.ECMD (0x2C) // Notify EC of CS entry
        If(LEqual(ECLP, 0x1)) {
          Store(Zero,\ECUP)
        }
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
//[-end-161206-IB07400822-modify]//

//      P8XH(0, 0xC5)
//      P8XH(1, 00)
//      ADBG("Enter CS")
//      If(PSCP){
//       // if P-state Capping is enabled
//        If (LAnd(CondRefOf(\_PR.CPU0._PSS), CondRefOf(\_PR.CPU0._PPC)))
//        {
//          Subtract(SizeOf(\_PR.CPU0._PSS), One, \_PR.CPU0._PPC)
//          PNOT()
//        }
//      }
      If(PLCS){
        SPL1() // set PL1 to low value upon CS entry
      }
    } Else {
//[-start-161206-IB07400822-modify]//
#if FeaturePcdGet(PcdKscSupport)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      //
      // CRB EC code
      //
      if(LEqual(ECNO,0x01)){ // Check EC notification enabled in Setup
        ADBG("EC Notify")
        Store (Acquire(\EHLD, 0xFFFF), Local0) // Wait for Mutex for telling EC to exit Low Power Mode
        if (LEqual(Local0, Zero)) {
          \_SB.PCI0.LPCB.H_EC.ECMD (0x2D) // Notify EC of CS exit
          If(LEqual(ECLP, 0x1)) {
            Store(One,\ECUP)
          }
          Release(\EHLD)
        }
        if(LEqual(ECDB,0x01)){
          ADBG("EC Debug")
          \_SB.PCI0.LPCB.H_EC.ECWT(0, RefOf(\_SB.PCI0.LPCB.H_EC.DLED)) //Clear EC CS Debug Light (CAPS LOCK)
        }
      }
#else
      //
      // OEM EC code
      //
#endif
#endif
//[-end-161206-IB07400822-modify]//
      P8XH(0, 0xC5)
      P8XH(1, 0xAB)

//      ADBG("Exit CS")
//      If(PSCP){
//        // if P-state Capping s enabled
//        If (CondRefOf(\_PR.CPU0._PPC))
//        {
//          Store(Zero, \_PR.CPU0._PPC)
//          PNOT()
//        }
//      }
//      If(PLCS){
//        RPL1() // restore PL1 to pre-CS value upon exiting CS
//      }
    }

    P_CS() // Powergating during CS
  }
}

// Power CS Powergated Devices:
//    Method to enable/disable power during CS
Method(P_CS,0,Serialized)
{
    // NOTE: Do not turn ON Touch devices from here. Touch does not have PUAM
//    If(CondRefOf(\_SB.PCI0.PAUD.PUAM)){           // Notify Codec(HD-A/ADSP)
//        \_SB.PCI0.PAUD.PUAM()
//    }
    // Adding back USB powergating (ONLY for Win8) until RTD3 walkup port setup implementation is complete */
//    If(LEqual(OSYS,2012)){    // ONLY for Win8 OS
//      If(CondRefOf(\_SB.PCI0.XHC.DUAM)){ // Notify USB port- RVP
//        \_SB.PCI0.XHC.DUAM()
//      }
//    }
    // TODO: Add calls to UAMN methods for
    //    * USB controller(s)
    //    * Embedded Controller
    //    * Sensor devices
    //    * Audio DSP?
    //    * Any other devices dependent on User Absent mode for power controls
}

//
//
//

//
// System Bus
//
Scope(\_SB)
{
  Scope(PCI0)
  {
//[-start-160923-IB07400789-add]//
    Name(OSCF, 0)   // OS Check Flag
//[-end-160923-IB07400789-add]//
    Method(_INI,0)
    {
//[-start-190314-IB07401090-add]//
#ifndef DISABLE_MSR_CTR_OVERRIDE_HANDLER
      Store(0x5C,SSMP)   //Triggering SMM
#endif      
//[-end-190314-IB07401090-add]//
      // Determine the OS and store the value, where:
      //
      //   OSYS = 2000 = WIN2000.
      //   OSYS = 2001 = WINXP, RTM or SP1.
      //   OSYS = 2002 = WINXP SP2.
      //   OSYS = 2006 = Vista.
      //   OSYS = 2009 = Windows 7 and Windows Server 2008 R2.
      //   OSYS = 2012 = Windows 8 and Windows Server 2012.
      //   OSYS = 2013 = Windows Blue.
      //   OSYS = 2015 = Windows 10.
      //
      // Assume Windows 2000 at a minimum.

      Store(2000,OSYS)

      // Check for a specific OS which supports _OSI.

      If(CondRefOf(\_OSI))
      {
        // Linux returns _OSI = TRUE for numerous Windows
        // strings so that it is fully compatible with
        // BIOSes available in the market today.  There are
        // currently 2 known exceptions to this model:
        //  1) Video Repost - Linux supports S3 without
        //    requiring a Driver, meaning a Video
        //    Repost will be required.
        //  2) On-Screen Branding - a full CMT Logo
        //    is limited to the WIN2K and WINXP
        //    Operating Systems only.

        // Use OSYS for Windows Compatibility.

        If(\_OSI("Windows 2001")) // Windows XP
        {
          Store(2001,OSYS)
        }

        If(\_OSI("Windows 2001 SP1")) // Windows XP SP1
        {
          Store(2001,OSYS)
        }

        If(\_OSI("Windows 2001 SP2")) // Windows XP SP2
        {
          Store(2002,OSYS)
        }

        If(\_OSI("Windows 2006")) // Windows Vista
        {
          Store(2006,OSYS)
        }

        If(\_OSI("Windows 2009")) // Windows 7 and Windows Server 2008 R2
        {
          Store(2009,OSYS)
        }

        If(\_OSI("Windows 2012")) //Windows 8 and Windows Server 2012
        {
          Store(2012,OSYS)
        }

        If(\_OSI("Windows 2013")) //Windows 8.1 and Windows Server 2012 R2
        {
          Store(2013,OSYS)
        }

        If(\_OSI("Windows 2015")) //Windows 10
        {
          Store(2015,OSYS)
        }
        //
        // If CMP is enabled, enable SMM C-State
        // coordination.  SMM C-State coordination
        // will be disabled in _PDC if driver support
        // for independent C-States deeper than C1
        // is indicated.
      }
//[-start-160923-IB07400789-add]//
      if (LEqual(OSCF, 0)) {
        If(LLess(OSYS,2012)) {   // Older OS
          if (LEqual (LFSP, 1)) { // Legacy Free Support, 
            Store(0x01, SSMD) // Enable 0x60/0x64 IO Trap
            Store(0x91, SSMP) // USB_LEGACY_CONTROL_SW_SMI 
          } 
        } 
        Store(0x1, OSCF) 
      }
//[-end-160923-IB07400789-add]//
    }

    Method(NHPG,0,Serialized)
    {
      Store(0,^RP01.HPEX) // clear the hot plug SCI enable bit
      Store(0,^RP02.HPEX) // clear the hot plug SCI enable bit
      Store(0,^RP03.HPEX) // clear the hot plug SCI enable bit
      Store(0,^RP04.HPEX) // clear the hot plug SCI enable bit
      Store(0,^RP05.HPEX) // clear the hot plug SCI enable bit
      Store(0,^RP06.HPEX) // clear the hot plug SCI enable bit
      Store(1,^RP01.HPSX) // clear the hot plug SCI status bit
      Store(1,^RP02.HPSX) // clear the hot plug SCI status bit
      Store(1,^RP03.HPSX) // clear the hot plug SCI status bit
      Store(1,^RP04.HPSX) // clear the hot plug SCI status bit
      Store(1,^RP05.HPSX) // clear the hot plug SCI status bit
      Store(1,^RP06.HPSX) // clear the hot plug SCI status bit
    }

    Method(NPME,0,Serialized)
    {
      Store(0,^RP01.PMEX) // clear the PME SCI enable bit
      Store(0,^RP02.PMEX) // clear the PME SCI enable bit
      Store(0,^RP03.PMEX) // clear the PME SCI enable bit
      Store(0,^RP04.PMEX) // clear the PME SCI enable bit
      Store(0,^RP05.PMEX) // clear the PME SCI enable bit
      Store(0,^RP06.PMEX) // clear the PME SCI enable bit
      Store(1,^RP01.PMSX) // clear the PME SCI status bit
      Store(1,^RP02.PMSX) // clear the PME SCI status bit
      Store(1,^RP03.PMSX) // clear the PME SCI status bit
      Store(1,^RP04.PMSX) // clear the PME SCI status bit
      Store(1,^RP05.PMSX) // clear the PME SCI status bit
      Store(1,^RP06.PMSX) // clear the PME SCI status bit
    }
  } // end Scope(PCI0)

  //--------------------
  //  GPIO
  //--------------------

  Device (GPO0) // North Community for DFx GPIO, SATA GPIO, PWM, LPSS/ISH UARTs, IUnit GPIO, JTAG, and SVID
  {
    Name (_ADR, 0)
    Name (_HID, "INT3452")
    Name (_CID, "INT3452")
    Name (_DDN, "General Purpose Input/Output (GPIO) Controller - North" )
    Name (_UID, 1)
    Name (LINK, "\\_SB.GPO0") // Support for Windows 7

    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x00000000, 0x00004000, BAR0)
      Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {14}
    })

    Method (_CRS, 0x0, NotSerialized)
    {
      CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
      CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
      Store(GP0A, B0BA)
      Store(GP0L, B0LN)
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      Return(0xf)
    }
    // Track status of GPIO OpRegion availability for this controller
    Name(AVBL, 0)
    Method(_REG,2) {
       If (Lequal(Arg0, 8)) {
         Store(Arg1, ^AVBL)
       }
    }

    OperationRegion(GPOP, SystemMemory, GP0A, GP0L)
    Field(\_SB.GPO0.GPOP, ByteAcc, NoLock, Preserve) {
//PRJ+ >>>> Modify MIPI panel power sequency    
      Offset(0x548), //GPIO_9 MIPI RST
      MRST,1,   
//PRJ+ <<<< Modify MIPI panel power sequency        
      Offset(0x578), //PIN 15:15 * 8 + 0x500  // WiFi Reset
      CWLE, 1,
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      Offset(0x5B0), //PIN 22:22 * 8 + 0x500  // SATA_ODD_PWRGT_R, Apollo Lake RVP, N_GPIO_22
      ODPW, 1
#endif      
//[-end-161125-IB07400818-modify]//
    }
//[PRJ]+ >>>> implement Touch Button function   
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
    	
    	}
  	  Method(BTNE, 1, Serialized)
  	  {
  	
  	  }
  	  Method(BTNS, 0, Serialized)
  	  {
        Store(0xC, BTS1)
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
//[PRJ]+ >>>> implement Touch Button function
    
  }   //  Device (GPO0)

  Device (GPO1) // Northwest Community for Display GPIO, PMC, Audio, and SPI
  {
    Name (_ADR, 0)
    Name (_HID, "INT3452")
    Name (_CID, "INT3452")
    Name (_DDN, "General Purpose Input/Output (GPIO) Controller - Northwest" )
    Name (_UID, 2)

    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x00000000, 0x00004000, BAR0)
      Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {14}
    })

    Method (_CRS, 0x0, NotSerialized)
    {
      CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
      CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
      Store(GP1A, B0BA)
      Store(GP1L, B0LN)
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      If(LLess(OSYS,2012)) {
        // Don't report this GPIO for WIN7
        Return (0)
      }
      Return(0xf)
    }
//PRJ+ >>>> Modify MIPI panel power sequency
    OperationRegion(GPOQ, SystemMemory, GP1A, GP1L)
    Field(\_SB.GPO1.GPOQ, ByteAcc, NoLock, Preserve) {
      Offset(0x548), //PIN 196 MIPI VDD_EN
      MVDD,1,     
      Offset(0x550), //PIN 197 MIPI BKLTEN
      MBKL,1, 
//PRJ+ >>>> Update GOP to 10.0.1039      
      Offset(0x558), //PIN 198 MIPI BKLTCTL
      MBKC,1,
//PRJ+ <<<< Update GOP to 10.0.1039      
    }
//PRJ+ <<<< Modify MIPI panel power sequency    
  }   //  Device (GPO1)


  Device (GPO2) // West Community for LPSS/ISH I2C, ISH GPIO, iCLK, and PMU
  {
    Name (_ADR, 0)
    Name (_HID, "INT3452")
    Name (_CID, "INT3452")
    Name (_DDN, "General Purpose Input/Output (GPIO) Controller - West" )
    Name (_UID, 3)

    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x00000000, 0x00004000, BAR0)
      Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {14}
    })

    Method (_CRS, 0x0, NotSerialized)
    {
      CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
      CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
      Store(GP2A, B0BA)
      Store(GP2L, B0LN)
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      If(LLess(OSYS,2012)) {
        // Don't report this GPIO for WIN7
        Return (0)
      }
      Return(0xf)
    }
  }   //  Device (GPO2)

  Device (GPO3) // Southwest Community for EMMC, SDIO, SDCARD, SMBUS, and LPC
  {
    Name (_ADR, 0)
    Name (_HID, "INT3452")
    Name (_CID, "INT3452")
    Name (_DDN, "General Purpose Input/Output (GPIO) Controller - Southwest" )
    Name (_UID, 4)

    Name (RBUF, ResourceTemplate ()
    {
      Memory32Fixed (ReadWrite, 0x00000000, 0x00004000, BAR0)
      Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , ) {14}
    })

    Method (_CRS, 0x0, NotSerialized)
    {
      CreateDwordField(^RBUF, ^BAR0._BAS, B0BA)
      CreateDwordField(^RBUF, ^BAR0._LEN, B0LN)
      Store(GP3A, B0BA)
      Store(GP3L, B0LN)
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      If(LLess(OSYS,2012)) {
        // Don't report this GPIO for WIN7
        Return (0)
      }
      Return(0xf)
    }

    // Track status of GPIO OpRegion availability for this controller
    Name(AVBL, 0)
    Method(_REG,2) {
      If (Lequal(Arg0, 8)) {
        Store(Arg1, ^AVBL)
      }
    }

    OperationRegion(GPOP, SystemMemory, GP3A, GP3L)
    Field(\_SB.GPO3.GPOP, ByteAcc, NoLock, Preserve) {
      Offset(0x5F0), //PIN 30: 30 * 8 + 0x500 // GPIO_183 SD_CARD_PWR_EN_N
      SDPC, 1
    }
  }   //  Device (GPO3)

} // end Scope(\_SB)

Scope (\)
{
  //
  // Global Name, returns current Interrupt controller mode;
  // updated from _PIC control method
  //
  Name(PICM, 0)

  //
  // Procedure: GPRW
  //
  // Description: Generic Wake up Control Method ("Big brother")
  //              to detect the Max Sleep State available in ASL Name scope
  //              and Return the Package compatible with _PRW format.
  // Input: Arg0 =  bit offset within GPE register space device event will be triggered to.
  //        Arg1 =  Max Sleep state, device can resume the System from.
  //                If Arg1 = 0, Update Arg1 with Max _Sx state enabled in the System.
  // Output:  _PRW package
  //
  Name(PRWP, Package(){Zero, Zero})   // _PRW Package

  Method(GPRW, 2)
  {
    Store(Arg0, Index(PRWP, 0))             // copy GPE#
    //
    // SS1-SS4 - enabled in BIOS Setup Sleep states
    //
    Store(ShiftLeft(SS1,1),Local0)          // S1 ?
    Or(Local0,ShiftLeft(SS2,2),Local0)      // S2 ?
    Or(Local0,ShiftLeft(SS3,3),Local0)      // S3 ?
    Or(Local0,ShiftLeft(SS4,4),Local0)      // S4 ?
    //
    // Local0 has a bit mask of enabled Sx(1 based)
    // bit mask of enabled in BIOS Setup Sleep states(1 based)
    //
    If(And(ShiftLeft(1, Arg1), Local0))
    {
      //
      // Requested wake up value (Arg1) is present in Sx list of available Sleep states
      //
      Store(Arg1, Index(PRWP, 1))           // copy Sx#
    }
    Else
    {
      //
      // Not available -> match Wake up value to the higher Sx state
      //
      ShiftRight(Local0, 1, Local0)
      // If(LOr(LEqual(OSFL, 1), LEqual(OSFL, 2))) {  // ??? Win9x
      // FindSetLeftBit(Local0, Index(PRWP,1))  // Arg1 == Max Sx
      // } Else {           // ??? Win2k / XP
     FindSetLeftBit(Local0, Index(PRWP,1))  // Arg1 == Min Sx
      // }
    }

    Return(PRWP)
  }
}

Scope (\_SB)
{
  Name(OSCI, 0)  // \_SB._OSC DWORD2 input
  Name(OSCO, 0)  // \_SB._OSC DWORD2 output
  Name(OSCP, 0)  // \_SB._OSC CAPABILITIES
  // _OSC (Operating System Capabilities)
  //    _OSC under \_SB scope is used to convey platform wide OSPM capabilities.
  //    For a complete description of _OSC ACPI Control Method, refer to ACPI 5.0
  //    specification, section 6.2.10.
  // Arguments: (4)
  //    Arg0 - A Buffer containing the UUID "0811B06E-4A27-44F9-8D60-3CBBC22E7B48"
  //    Arg1 - An Integer containing the Revision ID of the buffer format
  //    Arg2 - An Integer containing a count of entries in Arg3
  //    Arg3 - A Buffer containing a list of DWORD capabilities
  // Return Value:
  //    A Buffer containing the list of capabilities
  //
  Method(_OSC,4,Serialized)
  {
    //
    // Point to Status DWORD in the Arg3 buffer (STATUS)
    //
    CreateDWordField(Arg3, 0, STS0)
    //
    // Point to Caps DWORDs of the Arg3 buffer (CAPABILITIES)
    //
    CreateDwordField(Arg3, 4, CAP0)

    //
    // Check UUID
    //
    If(LEqual(Arg0,ToUUID("0811B06E-4A27-44F9-8D60-3CBBC22E7B48")))
    {
      //
      // Check Revision
      //
      If(LEqual(Arg1,One))
      {
        Store(CAP0, OSCP)
        If(And(CAP0,0x04)) // Check _PR3 Support(BIT2)
        {
          Store(0x04, OSCO)
          If(LEqual(RTD3,0)) // Is RTD3 support disabled in Bios Setup?
          {
            // RTD3 is disabled via BIOS Setup.
            And(CAP0, 0x3B, CAP0) // Clear _PR3 capability
            Or(STS0, 0x10, STS0) // Indicate capability bit is cleared
          }
        }
      } Else{
        And(STS0,0xFFFFFF00,STS0)
        Or(STS0,0xA, STS0) // Unrecognised Revision and report OSC failure
      }
    } Else {
      And(STS0,0xFFFFFF00,STS0)
      Or (STS0,0x6, STS0) // Unrecognised UUID and report OSC failure
    }

    Return(Arg3)
  } // End _OSC
}

//[-start-160609-IB07400742-add]//
//
// System Bus
//
Scope (\_SB)
{

  Device (PWR2) // ACPI Power Button2 when EC not exist
  {
    Name (_HID,EISAID ("PNP0C0C"))
    Name (_UID, 2)
    
    Method (_STA, 0)
    {
//#if FeaturePcdGet(PcdKscSupport)
      If (LEqual (ECON, 1)){ 
        Return (0x00)
      } Else {
        Return (0x0F)
      }
//#endif
      Return (0x0F)
    }
    
  } // END PWR2
}
//[-end-160609-IB07400742-add]//
