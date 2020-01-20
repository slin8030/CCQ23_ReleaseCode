/** @file
  ACPI RTD3 SSDT table for BXT SATA

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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

/// @details
/// The following namespace(variable) must be initialized
/// when including this file under SATA scope:
/// PORT: SATA PORT number
/// PBAR: SATA command port base address
/// PWRG(PWRI): power GPIO pin {enable, group, pad, "on" polarity}
//    sample:Name(PWRG, Package() {
//                   0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
//                   0, // GPIO group #/IOEX #
//                   0, // GPIO pad #/IOEX pin #
//                   0  // power on polarity
//                 })
/// @defgroup SATA port Scope **/

/// Define _PR0, _PR3 PowerResource Package
// command port = ABAR + PBAR
// power pin = PWRG
  Name(WKEN, 0)
  
  /// _PS0 Method for SATA HBA
  Method(_PS0,0,Serialized)
  {
  }

  /// _PS3 Method for SATA HBA
  Method(_PS3,0,Serialized)
  {
  }
  
  Method(_PRW, 0)
  {
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    if (LEqual(IOTP(),0x00)) { //CCG CRB
      //
      // GPIO_25, SATA_ODD_DA_IN
      //
      Return(Package(){0x39,4}) //GPE39 ZPODD device attention
    }
#endif
//[-end-161125-IB07400818-modify]//
//[-start-170113-IB07400833-add]//
    Return(Package() {Zero, Zero})
//[-end-170113-IB07400833-add]//
  }

  /// _S0W method returns the lowest D-state can be supported in state S0.
  Method(_S0W, 0)
  {
    Return(4) // return 4 (D3cold)
  } // End _S0W
  
  
  Method(_DSW, 3){
    If(Arg1)
    { // Entering Sx, need to disable DA from generating runtime PME
      Store(0, WKEN)
    } Else { // Staying in S0
      If(LAnd(Arg0, Arg2)) // Exiting D0 and arming for wake
      { // Set WKEN to allow _OFF to enable the wake event
        Store(1, WKEN)
      } Else { // Clear WKEN to allow _OFF to enable the wake event,
        // either because staying in D0 or disabling wake
        Store(0, WKEN)
      }
    }
  } // End _DSW  

  Method(_PR0) {
   If (LAnd(LGreaterEqual(\OSYS, 2015), LEqual(\EMOD, 1))) {
      If (CondRefOf(\_SB.MODS)) {
        Return(Package(){\_SB.MODS})
      }
    } Else {
//[-start-161125-IB07400818-add]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      If (LEqual(IOTP(),0x01)) { //IOTG CRB
        If(LEqual(DeRefOf(Index(PWRI, 0)),0)) { // If Power Gating disabled then dont expose Power Resources
          Return(Package() {})
        }
      }  
#endif
//[-end-161125-IB07400818-add]//
    	If(LEqual(DeRefOf(Index(PWRG, 0)),0)) { // If Power Gating disabled then dont expose Power Resources
    		Return(Package() {})
    	}
      If (CondRefOf(SPPR)) {
        Return(Package(){SPPR})
      }
    }
    Return(Package() {})
  }
  Method(_PR3) {
    If (LAnd(LGreaterEqual(\OSYS, 2015), LEqual(\EMOD, 1))) {
      If (CondRefOf(\_SB.MODS)) {
        Return(Package(){\_SB.MODS})
      }
    } Else {
//[-start-161125-IB07400818-add]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      If (LEqual(IOTP(),0x01)) { //IOTG CRB
        If(LEqual(DeRefOf(Index(PWRI, 0)),0)) { // If Power Gating disabled then dont expose Power Resources
          Return(Package() {})
        }
      }
#endif
//[-end-161125-IB07400818-add]//
    	If(LEqual(DeRefOf(Index(PWRG, 0)),0)) { // If Power Gating disabled then dont expose Power Resources
    	    		Return(Package() {})
    	    	}
    	
      If (CondRefOf(SPPR)) {
        Return(Package(){SPPR})
      }
    }
    Return(Package() {})
  }

  PowerResource(SPPR, 0, 0)
  {
	Name(_STA, 0x01)
			
    Method(_ON) {
      If (LEqual(\EMOD, 1)) {
        // Do nothing
      } Else {
    	  Store(0x01, _STA)
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
        If (LEqual(IOTP(),0x00)) { //CCG CRB
          Store(0, ZPGE) // Disable GPIO_25 GPE39
        }
#endif    	  
//[-end-161125-IB07400818-modify]//
        SPON()    // _ON Method
        Sleep(16) // Delay for power ramp.
      }
    }

    Method(_OFF) {
      If (LEqual(\EMOD, 1)) {
        // Do nothing
      } Else {
        SPOF()
        Store(0x00, _STA)
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
        If (LEqual(IOTP(),0x00)) { //CCG CRB
          Store(1, ZPGE) // Enable GPIO_25 GPE39
        }
#endif
//[-end-161125-IB07400818-modify]//
      }
    }
  } // end SPPR

  Name(OFTM, Zero) /// OFTM: Time returned by Timer() when the Power resource was turned OFF
  Method(SPSA) {
//[-start-161109-IB07400810-add]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    if (LEqual(IOTP(),0x01)) { //IOTG CRB
      // check power control enable
      If(LNotEqual(DeRefOf(Index(PWRI, 0)), 0)) {
        // read power pin status
        If(LEqual(DeRefOf(Index(PWRI, 0)),1)) { // GPIO mode
          if(LEqual(\_SB.GGOV(DeRefOf(Index(PWRI, 2))), DeRefOf(Index(PWRI, 3)))) {
            Return(0x01)
          } Else {
            Return(0x00)
          }
        }
      }
      Return(0x00)
    }
#endif    
//[-end-161109-IB07400810-add]//
    // check power control enable
    If(LNotEqual(DeRefOf(Index(PWRG, 0)), 0)) {
      // read power pin status
      If(LEqual(DeRefOf(Index(PWRG, 0)),1)) { // GPIO mode
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
        if(LEqual(\_SB.GPO0.ODPW, 1)) { //N_GPIO_22
          Return(0x01)
        } Else {
          Return(0x00)
        }
#else
        if(LEqual(\_SB.GGOV(DeRefOf(Index(PWRG, 2))), DeRefOf(Index(PWRG, 3)))) {
          Return(0x01)
        } Else {
          Return(0x00)
        }
#endif    
//[-end-161125-IB07400818-modify]//
      }
    }
    Return(0x00) // disabled
  } /// @defgroup sata_prt1_sta     SATA Port 1 PowerResource _STA Method

  Method(SPON, 0) {
    If(LNotEqual(^OFTM, Zero)) { /// if OFTM != 0 => Disk was turned OFF by asl
      Divide(Subtract(Timer(), ^OFTM), 10000, , Local0) ///- Store Elapsed time in ms
      Store(Zero, ^OFTM) ///- Reset OFTM to zero to indicate minimum 50ms requirement does not apply when _ON called next time
      If(LLess(Local0, 50)) ///- Do not sleep if already past the delay requirement
      {
        Sleep(Subtract(50, Local0)) ///- Sleep 50ms - time elapsed
      }
    }
//[-start-161109-IB07400810-add]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    if (LEqual(IOTP(),0x01)) { //IOTG CRB
      // drive power pin "ON"
      if(LNotEqual(DeRefOf(Index(PWRI, 0)),0)) {     // if power gating enabled
        if(LEqual(DeRefOf(Index(PWRI, 0)),1)) {      // GPIO mode
          \_SB.SGOV(DeRefOf(Index(PWRI, 2)),DeRefOf(Index(PWRI, 3)))
        }
      }
      Return()
    }
#endif    
//[-end-161109-IB07400810-add]//
    // drive power pin "ON"
    if(LNotEqual(DeRefOf(Index(PWRG, 0)),0)) {     // if power gating enabled
      if(LEqual(DeRefOf(Index(PWRG, 0)),1)) {      // GPIO mode
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
        Store (1, \_SB.GPO0.ODPW) //N_GPIO_22  
#else
        \_SB.SGOV(DeRefOf(Index(PWRG, 2)),DeRefOf(Index(PWRG, 3)))
#endif
//[-end-161125-IB07400818-modify]//
      }
    }
  }

  Method(SPOF, 0) {
    Add(\_SB.PCI0.SATA.MBR6, PBAR, Local0)
    /// if S0Ix enabled
    If(LEqual(S0ID, 1)) {
      OperationRegion(PSTS, SystemMemory, Local0, 0x18)
      Field(PSTS, DWordAcc, NoLock, Preserve)
      {
        Offset(0x0),
        CMST, 1,  //PxCMD.ST
        CSUD, 1,  //PxCMD.SUD
        , 2,
        CFRE, 1,  //PxCMD.FRE
        Offset(0x10),
        SDET, 4,  //PxSSTS.DET
        Offset(0x14),
        CDET, 4   //PxSCTL.DET
      }
      // Execute offline flow only if Device detected and Phy not offline
      If(LOr(LEqual(SDET, 1), LEqual(SDET, 3))) {
        ///- Clear ST (PxCMD.ST)
        Store(0, CMST)                   // PBAR[0]
        ///- Clear FRE
        Store(0, CFRE)                   // PBAR[4]
        ///- Clear SUD (PxCMD.SUD)
        Store(0, CSUD)                   // PBAR[1]
        ///- Set DET to 4 (PxSCTL.DET)
        Store(4, CDET)                   // PBAR+0x14[3:0]
        Sleep(16)
        ///- Wait until PxSSTS.DET == 4
        While(LNotEqual(SDET, 4)){
          Sleep(16)
        }
      }
    } // if S0Ix enabled
//[-start-161109-IB07400810-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    if (LEqual(IOTP(),0x01)) { //IOTG CRB
      // drive power pin "OFF"
      If(LNotEqual(DeRefOf(Index(PWRI, 0)),0)) { // if power gating enabled
        if(LEqual(DeRefOf(Index(PWRI, 0)),1)) { // GPIO mode
           \_SB.SGOV(DeRefOf(Index(PWRI, 2)),Xor(DeRefOf(Index(PWRI, 3)),1))
        }
      }
      Store(Timer(), ^OFTM)
      Return()
    }
#endif    
//[-end-161109-IB07400810-modify]//
    // drive power pin "OFF"
    If(LNotEqual(DeRefOf(Index(PWRG, 0)),0)) { // if power gating enabled
      if(LEqual(DeRefOf(Index(PWRG, 0)),1)) { // GPIO mode
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
        Store (0, \_SB.GPO0.ODPW)  // N_GPIO_22 
#else
        \_SB.SGOV(DeRefOf(Index(PWRG, 2)),Xor(DeRefOf(Index(PWRG, 3)),1))  
#endif    
//[-end-161125-IB07400818-modify]//
      }
    }

    Store(Timer(), ^OFTM) /// Store time when Disk turned OFF(non-zero OFTM indicate minimum 50ms requirement does apply when _ON called next time)
    
  } // end _OFF
