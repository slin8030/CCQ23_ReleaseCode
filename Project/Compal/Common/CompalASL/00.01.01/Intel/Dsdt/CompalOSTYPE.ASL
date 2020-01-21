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
// [COMMON ASL] using EDK II
//

Method(OSTP)
{
  Store(0x1,Local1)       // Default set Local1 is Vista
  
  If(LEqual(OSYS, 2001))  // XP
  {
    Store(0x0,Local1)
  }
  If(LEqual(OSYS, 2002))  // XP
  {
    Store(0x0,Local1)
  }
  If(LEqual(OSYS, 2006))  // Vista
  {
    Store(0x1,Local1)
  }
  If(LEqual(OSYS, 1000))  // Linux
  {
    Store(0x2,Local1)
  }
  If(LEqual(OSYS, 2009)) // Win7
  {
    Store(0x3,Local1)
  }
  If(LEqual(OSYS, 2012)) // Win8
  {
    Store(0x4,Local1)
  }
  If(LEqual(OSYS, 2013)) // Win8.1
  {
    Store(0x5,Local1)
  }

                
  Store(Local1, COMMON_ASL_EC_PATH.OSTY)
  
  // ---------------------------------------------------------------------  
  // Please follow below steps to add project code if necessary 
  //                  
  // In OEMASL.ASL -
  // #define OS_Type_HOOKS   COMMON_ASL_LPC_PATH.OTHK(Arg0)
  //
  // Create a new method in one of OEM ASL files -
  // Method(OTHK, 1) {....}
  //
  #ifdef OS_Type_HOOKS
    OS_Type_HOOKS
  #endif
                          
}
