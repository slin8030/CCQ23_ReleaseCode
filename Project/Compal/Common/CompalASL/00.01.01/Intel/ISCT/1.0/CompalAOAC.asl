/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
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

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00                Vanessa      Init version.

*/

//
// [COMMON ASL] AOAC 1.0 using EDK II
//

DefinitionBlock (
    "AoacAcpi.aml",
    "SSDT",
    1,
    "Intel_",
    "AoacTabl",
    0x1000
  )

{
// Global NVS
External(\AOAC, IntObj)

// EC Name Space
External(\_SB.PCI0.LPCB.EC0.AATL, FieldUnitObj)            // A0h.0
External(\_SB.PCI0.LPCB.EC0.AACL, FieldUnitObj)            // A0h.1
External(\_SB.PCI0.LPCB.EC0.AAST, FieldUnitObj)            // A0h.2
External(\_SB.PCI0.LPCB.EC0.AARW, FieldUnitObj)            // A0h.3
External(\_SB.PCI0.LPCB.EC0.AAEN, FieldUnitObj)            // A0h.4
External(\_SB.PCI0.LPCB.EC0.AAWR, FieldUnitObj)            // A0h.6-7

  Scope (\_SB)
  {
    Device (IAOE)
    {
        Name (_HID, "INT33A0")
        Name (_UID, 0x00)
        Name (AOS1, 0)
        Name (ANS1, 0)
        Name (WLS1, 1)
        Name (WWS1, 1)
        Name (ASDS, 0)
                                       
        //
        // GABS - Get Intel Smart Connect Technology BIOS Enabled Setting
        // Input:   None
        // Return:   
        // Bits   Description
        // 0      Intel Smart Connect Technology Configured: 0 = Disabled, 1 = Enabled
        // 1      Intel Smart Connect Technology Notification Control: 0 = Unsupported, 1 = Supported
        // 2      Intel Smart Connect Technology WLAN Power Control:0 = Unsupported, 1 = Supported
        // 3      Intel Smart Connect Technology WWAN Power Control: 0 = Unsupported, 1 = Supported
        // 4 - 7  Reserved 
        //
        Method (GABS, 0, NotSerialized)
        {
          Return (AOAC)
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
          // Code should be changed according to HW defintion              
          Store(Arg0, WLS1)  
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
          // Code should be changed according to HW defintion 
          Store(Arg0, WWS1)  
        }

        // 
        // SASD - Set Intel Smart Connect Technology Sleep Duration 
        // Input:   
        // Bits   Description  
        // 0 - 31 Sleep Duration  
        // 
        Method (SASD, 1, NotSerialized) 
        { 
          // Code should be changed if EC utilizes this value
          Store(Arg0, ASDS)
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
          // Check if it is RTC wake
          If ( LEqual(\_SB.PCI0.LPCB.EC0.AAWR, 0x02) )
          {
            // Notify EC RTC wake
            Store (1, \_SB.PCI0.LPCB.EC0.AARW)
              
            // Return RTC Timer caused wake: Bit2
            Return (0x04)
          }

          // Default:
          Else
          {
            // Notify EC non-RTC wake
            Store (0, \_SB.PCI0.LPCB.EC0.AARW)
            
            // Return User Pressed Power Button: Bit0
            Return (0x01)
          } 
        }

    } // Device (IAOE)
  } // Scope (\_SB)
} 
