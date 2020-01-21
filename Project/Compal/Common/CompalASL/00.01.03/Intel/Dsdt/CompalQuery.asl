/*
 * (C) Copyright 2014-2020 Compal Electronics, Inc.
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
 1.01                Vanessa      Fix lid value incorrect. 
*/

#include "Oemasl.asl"

#ifdef COMPAL_ISCT_SUPPORT
External(\_SB.IAOE.SAOS, MethodObj)
#endif
 
//================================================================
// Query 11 - Hot key Fn + ¡õ event of brightness down
//================================================================
  Method (_Q11)
  {
    Store("=====QUERY_11=====", Debug)      // DEBUG

    IF(IGDS)
    {
		Notify(\_SB.PCI0.GFX0.DD1F,0x87)	   //ULT UMA
    }
    else
    {
      //Notify(\_SB.PCI0.PEG0.VGA.LCD,0x87)    //DISCRETE
    }
  }

//================================================================
// Query 12 - Hot key Fn + ¡ô event of brightness up
//================================================================
  Method (_Q12)
  {
    Store("=====QUERY_12=====", Debug)      // DEBUG

    IF(IGDS)
    {
		Notify(\_SB.PCI0.GFX0.DD1F,0x86)	   //ULT UMA
	}
    else
    {
      //Notify(\_SB.PCI0.PEG0.VGA.LCD,0x86)    //DISCRETE
    }
   }

//================================================================
// Query 15 - Lid open close method
//================================================================
  Method (_Q15)
  {
    Store(0x15, P80H)
    
      If(IGDS)
      {
        // Upon waking a lid event may fail if driver is not ready.
        // If it does, set the high bit of the variable to indicate that
        // another notification is required during system callbacks.
        Store(COMMON_ASL_EC_PATH.LIDF, Local0)
        Not(Local0, Local0)
        And(Local0, 0x01, Local0) 
        Add(Local0, 2, Local0)
        If(\_SB.PCI0.GFX0.GLID(Local0))
        {
          Or(0x80000000,\_SB.PCI0.GFX0.CLID, \_SB.PCI0.GFX0.CLID)
        }
      Notify(COMMON_ASL_LPC_PATH.LID0,0x80)  //Notify OS lid status change
    }
  }
//================================================================
// Query 1D - Event of EC set throttle function and P-state 
//================================================================
  Method (_Q1D)
  {
    Store(0x1D, P80H)
    
#if COMPAL_COMMON_THERMAL_SUPPORT
    PCLK()
#endif
  }
//================================================================
// Query 24 - Battery Status update.
//================================================================
  Method (_Q24)
  {
  Store(0x24, P80H)
  Store("=====QUERY_24=====", Debug)      // DEBUG

  Sleep(1000)
  Notify (COMMON_ASL_LPC_PATH.BAT1, 0x80) 
  }
  
//================================================================
// Query 25 - Battery In/Out event
//================================================================
  Method (_Q25)
  {
    Store(0x25, P80H)
    Store("=====QUERY_25=====", Debug)      // DEBUG
  
    Sleep(1000)
    Notify (COMMON_ASL_LPC_PATH.BAT1, 0x81) // CMB Device Information Changed
  
    Sleep(1000)
    Notify (COMMON_ASL_LPC_PATH.BAT1, 0x80) // CMB Device Information Changed
  }

//================================================================
// Query 37/38 - AC in/out event
//================================================================
  Method (_Q37)
  {
    Store(0x37, P80H)
    
    Store("=====QUERY_37=====", Debug)      // DEBUG
    Notify (COMMON_ASL_LPC_PATH.ACAD, 0x80)
    Sleep(1000)
  
    Notify (COMMON_ASL_LPC_PATH.BAT1, 0x80) // CMB Status Changed
  
    Store (1, PWRS)

    PNOT ()
  }
  
  Method (_Q38)
  {
    Store(0x38, P80H)
    
    Store("=====QUERY_38=====", Debug)      // DEBUG
    Notify (COMMON_ASL_LPC_PATH.ACAD, 0x80)
    Sleep(1000)
  
    Notify (COMMON_ASL_LPC_PATH.BAT1, 0x80) // CMB Status Changed
  
    Store (0, PWRS)

    PNOT ()
  }  

#ifdef COMPAL_ISCT_SUPPORT
//================================================================
// Query 02Ch - Any event in S0_ISCT to announce BIOS set S0_ISCT status to be "0"
//================================================================
  Method (_Q2C)
  { 
    \_SB.IAOE.SAOS(0)
  }
#endif

