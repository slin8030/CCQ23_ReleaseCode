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
 1.06      16/07/05  Vanessa      Add more notes for file. 
                                  Add Flag to control device need by the platform.
 1.07      16/09/26  Vaneesa      Updated device flag definition to control device need by the platform.

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
//
// Compal Global region 'COMP' of ACPI NVS definition
//
  #include "CompalGloblNvs.asl"
//
// Compal SW SMI Method support
//
  #include "CompalSwSmi.asl"  
//
// I2C sensor devicse definitions
//  
  #include "CompalSensorHubI2C.asl"
//
// System RTC definition
//
  #ifdef PROJECT_RTC_ENABLE
  #include "CompalRTC.asl"
  #endif
//
// System button (Power/Sleep..) definition
//
  #include "CompalSysButton.asl"
//
// I2C/PS2 TouchPad devicse definitions
//
  #ifdef PROJECT_TOUCHPAD_ENABLE
  #include "CompalTouchpad.asl"
  #endif
//
// I2C TouchPanel devicse definitions
//
  #ifdef PROJECT_TOUCHPANEL_ENABLE
  #include "CompalTouchPanel.asl" 
  #endif
}

#if (EC_PLATFORM_SETTING == 0 )||(EC_PLATFORM_SETTING == 2 )    // 0: LPC|2:eSPI
Scope(COMMON_ASL_LPC_PATH)
{
//
// LPC/eSPI ACPI devicse 'EC0' definition
//    
  #include "CompalEC0.asl"
//
// Define the Battery Control Method
//
  #ifdef PROJECT_BATTERY_ENABLE
  #include "CompalSMBATA.asl"
  #endif
//
// Define an AC Device for NB Platforms
//
  #ifdef PROJECT_ADAPTER_ENABLE
  #include "CompalACAD.asl"
  #endif
//
// Store OS the value and inform to EC
//
  #include "CompalOSTYPE.asl"
//
// ACPI devicse of LID definition
//
  #ifdef PROJECT_LID_ENABLE
  #include "CompalLID.asl"
  #endif 
//
// ACPI EC driver ready flag
//      
  #include "CompalECDriverReady.asl"
}
#endif

#if (EC_PLATFORM_SETTING == 1)                                  // 1: I2C
//
// I2C EC Name Space definition
//    
  #include "CompalEcnameI2C.asl"                                // ENE Chip IO373x
//
// Define the Battery Control Method
//
  #ifdef PROJECT_BATTERY_ENABLE
  #include "CompalSMBATAI2C.asl"
  #endif
//
// I2C EC Command R/W Interface
//    
  #include "CompalEcCmdI2C.asl"                                 // EC CMD read write by block access.
//
// Define an AC Device for NB Platforms.
//
  #ifdef PROJECT_ADAPTER_ENABLE
  #include "CompalACADI2C.asl" 
  #endif
//
// Query Event of I2C EC definition
//
  #include "CompalQueryI2C.asl"                                 
#endif  
