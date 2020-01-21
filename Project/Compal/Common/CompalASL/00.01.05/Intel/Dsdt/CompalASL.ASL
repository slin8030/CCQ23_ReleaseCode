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
 1.01      14/9/10   Vanessa      Change CompalSensorHubI2C.asl path.
 1.02      16/03/02  Stan         Implement Compal touchpad feature.
 1.03      16/03/17  Vanessa      Add flag to switch LPC/I2C EC platform
 1.04      16/03/30  Stan         Implement Compal touchpanel feature.
 1.05      16/05/16  Vanessa      Add Power button device to common code.
                                  Add eSPI EC flag to switch ASL code.
*/

//
// [COMMON ASL] Using EDK II
//

#include "OemASL.asl"

//
// [COM] Common ASL Scope Assignment
//
Scope(\)
{
  #include "CompalGloblNvs.asl"
  #include "CompalSensorHubI2C.asl"
}

#if (EC_PLATFORM_SETTING == 0 )||(EC_PLATFORM_SETTING == 2 )    // 0: LPC|2:eSPI
Scope(COMMON_ASL_LPC_PATH)
{
  #include "CompalEC0.asl"
  #include "CompalSMBATA.asl"
  #include "CompalACAD.asl"
  #include "CompalSysButton.asl"
  #include "CompalRTC.asl"
  #include "CompalOSTYPE.asl"
  #include "CompalLID.asl"
  #include "CompalECDriverReady.asl"
  #include "CompalTouchpad.asl"     
  #include "CompalTouchPanel.asl"     
}
#endif

#if (EC_PLATFORM_SETTING == 1)                                  // 1: I2C
  #include "CompalEcnameI2C.asl"                                // ENE Chip IO373x
  #include "CompalSMBATAI2C.asl"                                // Battery device common method. 
  #include "CompalEcCmdI2C.asl"                                 // EC CMD read write by block access.
  #include "CompalACADI2C.asl"                                  // AC Adapter
  #include "CompalQueryI2C.asl"                                 // Query Event for I2C
#endif  
