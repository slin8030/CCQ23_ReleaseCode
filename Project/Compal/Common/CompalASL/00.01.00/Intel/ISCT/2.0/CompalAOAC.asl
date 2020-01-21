/*
 * (C) Copyright 2011-2012 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// [COMMON ASL] AOAC 2.0 using EDK II
//

DefinitionBlock (
    "IsctAcpi.aml",
    "SSDT",
    1,
    "Intel_",
    "IsctTabl",
    0x1000
  )

{
// Global NVS
External(ICNF) //Isct Configuration

// EC Name Space
External(\_SB.PCI0.LPCB.EC0.AASD, FieldUnitObj)            // 5Ah
External(\_SB.PCI0.LPCB.EC0.AATL, FieldUnitObj)            // A0h.0
External(\_SB.PCI0.LPCB.EC0.AACL, FieldUnitObj)            // A0h.1
External(\_SB.PCI0.LPCB.EC0.AAST, FieldUnitObj)            // A0h.2
External(\_SB.PCI0.LPCB.EC0.AARW, FieldUnitObj)            // A0h.3
External(\_SB.PCI0.LPCB.EC0.AAEN, FieldUnitObj)            // A0h.4
External(\_SB.PCI0.LPCB.EC0.AAEW, FieldUnitObj)            // A0h.5
External(\_SB.PCI0.LPCB.EC0.AAND, FieldUnitObj)            // A0h.6

// CMOS Times
External(\_SB.PCI0.LPCB.RTC.RTCS, FieldUnitObj)            // 0x00 Second
External(\_SB.PCI0.LPCB.RTC.RTCM, FieldUnitObj)            // 0x02 Minute
External(\_SB.PCI0.LPCB.RTC.RTCH, FieldUnitObj)            // 0x04 Hour
External(\_SB.PCI0.LPCB.RTC.RTCD, FieldUnitObj)            // 0x07 Day

  Scope (\_SB)
  {
    Device (IAOE)
    {
      OperationRegion(ISCT,SystemMemory,0xFFFF0008,0xAA58)
        Field(ISCT,AnyAcc,Lock,Preserve) {
          WKRS,  8,      // (0) ISCT Wake Reason
          ISEF,  8,      // (1) ISCT Enabled Features
          FFSE,  8,      // (2) IFFS Enabled
          ITMR,  8,      // (3) ISCT Timer Type: 0 = EC, 1 = RTC  
          ECTM,  32,     // (4) ISCT EC Timer
          RCTM,  32,     // (8) ISCT RTC Timer
          ATOW,  8,      // (16)ISCT timer over write, 1 = overwrited as ISCT timer
        }
        Name (_HID, "INT33A0")
        Name (_CID, "PNP0C02")
        Name (_UID, 0x00)
        Name (AOS1, 0)
        Name (ANS1, 0)
        Name (WLS1, 1)
        Name (WWS1, 1)
        Name (ASDS, 0)

        Name (ECWM, 0)
        Name (ISCS, 0)
        Name (ISCM, 0) 
        Name (ISCH, 0)
        Name (PRTS, 0)  
        Name (PRTM, 0)
        Name (PRTH, 0)
                                       
        //
        // GABS - Get Intel Smart Connect Technology BIOS Enabled Setting
        // Input:   None
        // Return:   
        // Bits   Description
        // 0      Intel Smart Connect Technology Configured: 0 = Disabled, 1 = Enabled
        // 1      Intel Smart Connect Technology Notification Control: 0 = Unsupported, 1 = Supported
        // 2      Intel Smart Connect Technology WLAN Power Control:0 = Unsupported, 1 = Supported
        // 3      Intel Smart Connect Technology WWAN Power Control: 0 = Unsupported, 1 = Supported
        // 4      Intel Smart Connect Technology EC Timer Supported
        // 5      Sleep duration value format: 0 = Actual time, 1 = duration in 
        //        seconds (see SASD for actual format)
        // 6 - 7  Reserved (must set to 0)
        //
        Method (GABS, 0, NotSerialized)
        {
          Or (ICNF, 0x30, ICNF)                       // 4 Set EC Timer Supported
                                                      // 5 Set Sleep Duration in Seconds
          And (ICNF, 0x3F, ICNF)                      // 6 - 7  Reserved (Must be set to 0)  
    
          // Set WLAN Power Control Supported/Unsupported for GWLS/SWLS
          If ( Lequal(And(ICNF, 0x04), 0x04) ) {
            Store(1, WLS1)
          } Else {
            Store(0, WLS1)
          }
          
          // Set WWAN Power Control Supported/Unsupported for GWWS/SWWS
          If ( Lequal(And(ICNF, 0x08), 0x08) ) {
            Store(1, WWS1)
          } Else {
            Store(0, WWS1)
          }          
    
          Return (ICNF)
        }
    
        //
        // GAOS - Get Intel Smart Connect Technology Function Status
        // Input:   None
        // Return:   
        // Bits   Description   
        // 0      Intel Smart Connect Technology Mode: 0 = Disabled, 1 = Enabled
        // 1 - 7  Reserved 
        //
        Method (GAOS, 0, NotSerialized)
        {
          Return (AOS1)
        }
    
        //
        // SAOS - Set Intel Smart Connect Technology Function Status
        // Input:   
        // Bits   Description 
        // 0      Intel Smart Connect Technology Mode: 0 = Disabled, 1 = Enabled
        // 1 - 7  Reserved 
        //
        Method (SAOS, 1, NotSerialized)
        {      
           If (LEqual (And(Arg0, 0x01), 0x01))
           {
             Store(1, AOS1)                      

             // Notify EC to enter S0-ISCT mode
             Store (1, \_SB.PCI0.LPCB.EC0.AAST)
           }
           Else
           {
             Store(0, AOS1)
             
             // Notify EC to enter S0 mode
             Store (0, \_SB.PCI0.LPCB.EC0.AAST)
           }
        }
    
        //
        // GANS - Get Intel Smart Connect Technology Notification Status
        // Input:   None
        // Return:   
        // Bits   Description 
        // 0      Intel Smart Connect Technology Notification: 0 = Disabled, 1 = Enabled
        // 1 - 7  Reserved 
        //
        Method (GANS, 0, NotSerialized)
        {
          Return (ANS1)
        }
    
        //
        // SANS - Set Intel Smart Connect Technology Notification Status
        // Input:   
        // Bits   Description 
        // 0      Intel Smart Connect Technology Notification: 0 = Disabled, 1 = Enabled
        // 1 - 7  Reserved 
        //
        Method (SANS, 1, NotSerialized)
        {
           If (LEqual (And(Arg0, 0x01), 0x01))
           {
             Store(1, ANS1)                      
           }
           Else
           {
             Store(0, ANS1)  
           }            
        }
    
        //
        // GWLS - Get WLAN Module Status
        // Input:   None
        // Return:   
        // Bits   Description 
        // 0      WLAN Wireless Disable (W_DISABLE#):0 = Disabled, 1 = Enabled
        // 1      WLAN Module Powered in S3: 0 = Disabled, 1 = Enabled
        // 2      WLAN Module Powered in S4 (FFS): 0 = Disabled, 1 = Enabled
        // 3      WLAN Module Powered in S5: 0 = Disabled, 1 = Enabled
        // 4 - 7  Reserved
        //
        Method (GWLS, 0, NotSerialized)
        {
          // Code should be changed according to HW defintion 
          Return (WLS1)
        }
        
        //
        // SWLS - Set WLAN Module Status
        // Input:  
        // Bits   Description 
        // 0      N/A (WLAN Wireless Disable is Read only)  Always set to 0 
        // 1      WLAN Module Powered in S3: 0 = Disabled, 1 = Enabled
        // 2      WLAN Module Powered in S4 (FFS): 0 = Disabled, 1 = Enabled
        // 3      WLAN Module Powered in S5: 0 = Disabled, 1 = Enabled
        // 4 - 7  Reserved
        //
        Method (SWLS, 1, NotSerialized)
        {
          // Keep Bit0 as original setting            
          Store(And(Arg0, 0xFE), Local0)
          Store(And(WLS1, 0x01), WLS1)
          Store(Or(Local0, WLS1), WLS1)

#ifdef NET_DETECT_WAKE_SUPPORT
          // Bit1/2: WLAN Module Powered in S3/S4 (FFS)
          If (LOR (LEqual (And (WLS1, 0x02), 0x02), LEqual (And (WLS1, 0x04), 0x04) ) )
          {
            // Notify EC enter Net Detect Mode
            Store(1, \_SB.PCI0.LPCB.EC0.AAND)
          } 

          // Disable WLAN power
          Else
          {
            // Notify EC enter non-Net Detect Mode
            Store(0, \_SB.PCI0.LPCB.EC0.AAND)
          }
#endif
        }
    
        //
        // GWWS - Get WWAN Module Status
        // Input:   None
        // Return:   
        // Bits   Description 
        // 0      WWAN Wireless Disable (W_DISABLE#):0 = Disabled, 1 = Enabled
        // 1      WWAN Module Powered in S3: 0 = Disabled, 1 = Enabled
        // 2      WWAN Module Powered in S4 (FFS): 0 = Disabled, 1 = Enabled
        // 3      WWAN Module Powered in S5: 0 = Disabled, 1 = Enabled
        // 4 - 7  Reserved
        //
        Method (GWWS, 0, NotSerialized)
        {
          // Code should be changed according to HW defintion 
          Return (WWS1)
        }
    
        //
        // SWWS - Set WWAN Module Status
        // Input:  
        // Bits   Description 
        // 0      N/A (WWAN Wireless Disable is Read only)  Always set to 0 
        // 1      WWAN Module Powered in S3: 0 = Disabled, 1 = Enabled
        // 2      WWAN Module Powered in S4 (FFS): 0 = Disabled, 1 = Enabled
        // 3      WWAN Module Powered in S5: 0 = Disabled, 1 = Enabled
        // 4 - 7  Reserved
        //
        Method (SWWS, 1, NotSerialized)
        {
          // Keep Bit0 as original setting            
          Store(And(Arg0, 0xFE), Local0)
          Store(And(WWS1, 0x01), WWS1)
          Store(Or(Local0, WWS1), WWS1)
        }

        // 
        // SASD - Set Intel Smart Connect Technology Sleep Duration 
        // Input:   
        // Bits   Description  
        // Bit 31: 0 = Sleep Duration in seconds for Bits 0 thru 30   
        // Bit 31: 1 = Actual time with Bits 0 thru defined as: 
        //               Bits 26-30: Year offset from 2000 (e.g.11 denotes 2011)
        //               Bits 22-25: Month 
        //               Bits 17-21: Day 
        //               Bits 12-16: Hour (24 hour format)
        //               Bits 6-11: Minutes
        //               Bits 0-5: Seconds
        // 
        Method (SASD, 1, NotSerialized) 
        { 
          Store(Arg0, ASDS)
          
          If (LEqual (ShiftRight (ASDS, 31), 0x01))             // Bit 31: 1 = Actual time
          {         
            // Due to the MAX of duration of Sleep is 2 hours, we calculate Hour, Minutes, Seconds for Sleep Duration in Minutes
            // Calculate for Hour:
            Store(FromBCD(\_SB.PCI0.LPCB.RTC.RTCH), PRTH)       // Get the Hour of Current time
            Store(ShiftRight(ASDS, 12), ISCH)                   // Get the Hour of Wake up time
            Store(And(ISCH, 0x1F),  ISCH)

            If ( LGreater(PRTH, ISCH) ) {                       // Check if it is different Day: Current Hour > Wake up Hour
              Add (ISCH, 24, ISCH)                              // Borrow from Hour to calculate
            }          
            
            Subtract (ISCH, PRTH, ISCH)                         // Calculate for Hour
            Multiply (ISCH, 60, ECWM)                           // Transfer Hour to Minutes
            
            // Calculate for Minutes:
            Store(FromBCD(\_SB.PCI0.LPCB.RTC.RTCM), PRTM)       // Get the Minutes of current time
            Store(ShiftRight(ASDS, 6), ISCM)                    // Get the Minutes of wake up time   
            Store(And(ISCM, 0x3F),  ISCM)
            
            If ( LGreater(PRTM, ISCM) ) {                       // Check if it is different Hour: Current Minutes > Wake up Minutes
              Subtract (ECWM, 60, ECWM)                         // Borrow from Hour to calculate
              Add (ISCM, 60, ISCM)            
            }            

            Subtract (ISCM, PRTM, ISCM)                         // Calculate for Minutes
            Add (ECWM, ISCM, ECWM)                              // Sum of Minutes

            // Calculate for Seconds:
            Store(FromBCD(\_SB.PCI0.LPCB.RTC.RTCS), PRTS)       // Get the Seconds of current time
            Store(And(ASDS, 0x3F), ISCS)                        // Get the Seconds of wake up time
            
            If ( LGreater(PRTS, ISCS) ) {                       // Check if it is different Minutes: Current Seconds > Wake up Seconds
              Subtract (ECWM, 1, ECWM)                          // Borrow from Hour to calculate
              Add (ISCS, 60, ISCS)  
            } 

            Subtract (ISCS, PRTS, ISCS)                         // Calculate for Seconds   
            If ( LGreaterEqual(ISCS, 30) ) {                    // Carrying if >= 30 sec
              Add (ECWM, 1, ECWM)
            }
            
            Store(ECWM, \_SB.PCI0.LPCB.EC0.AASD)                // Save Sleep Duration in Minutes to EC 
          }
          Else                                                  // Bit 31: 0 = Sleep Duration in seconds
          {
            Divide(ASDS, 60, Local0, ECWM)                      // Transfer Seconds to Minutes
            
            If ( LGreaterEqual(Local0, 30) ) {                  // Carrying if >= 30 sec
              Add (ECWM, 1, ECWM)
            }
            
            Store(ECWM, \_SB.PCI0.LPCB.EC0.AASD)                // Save Sleep Duration in Minutes to EC
          }
        } 
 
        // 
        // GPWR - Get Platform Wake Reason
        // Input:   None 
        // Return:    
        // Bits   Description  
        // 0      User pressed power button or HID event
        // 1      EC timer caused wake
        // 2      RTC timer caused wake
        // 3      Wake due to PME (This can be from any PCI-Ex based device 
        //        including from USB xHCI controllers caused by USB device 
        //        wake-up events).  BIOS must set PME_EN = 1 and examine the 
        //        PME_STS value and set this bit appropriately
        // 4      Set to 1 if BIOS programs EC or RTC timer for Intel Smart 
        //        Connect Technology wake (bit 1 or 2 set also) 
        // 5 - 7  Reserved (set to 0)
        // 
        Method (GPWR, 0, NotSerialized) 
        { 
          // Check if it is EC wake
          If ( Land( LEqual(\_SB.PCI0.LPCB.EC0.AAEW, 0x01), LEqual(\_SB.PCI0.LPCB.EC0.AAND, 0x00) ) ) 
          {         
            // Return EC timer caused wake: Bit1 & Bit4
            Return (0x12)
          }

          // Check if it is RTC wake
          ElseIf (LEqual(\_SB.PCI0.LPCB.EC0.AARW, 0x01))
          {
            // Return RTC Timer caused wake: Bit2
            Return (0x04)
          }

          // Check if it is Net Detect wake
          ElseIf ( Land( LEqual(\_SB.PCI0.LPCB.EC0.AAEW, 0x01), LEqual(\_SB.PCI0.LPCB.EC0.AAND, 0x01) ) )
          {
            // Return Net Detect caused wake: Bit1 & Bit4
            Return (0x08)           
          }
          
          // Default:
          Else
          {            
            // Return User Pressed Power Button: Bit0
            Return (0x01)
          } 
        }

    } // Device (IAOE)
  } // Scope (\_SB)
} 
