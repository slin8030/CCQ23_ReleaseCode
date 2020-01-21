/*
 * (C) Copyright 2013-2020 Compal Electronics, Inc.
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
 1.01      14/9/10   Vanessa      Support INVN,CM3218,KM61G,KXCJ9 and MTK NFC device
 1.02      14/11/28  Vanessa      Correct KXCJ9/MTK NFC device define.
 1.03      16/03/20  Vanessa      Move device asl code to "Device" folder for each device
 1.04      16/04/13  Vanessa      Add EC I2C SensorHub Solution.
 1.05      16/09/26  Vaneesa      Updated device flag definition to control device need by the platform.
*/

//
// [COMMON ASL] Using EDK II
//
  #ifdef PROJECT_I2C_INVN_ENABLE
  #include "INVN_SENSORHUB.ASL"
  #endif

  #ifdef PROJECT_I2C_CM3218_ENABLE
  #include "CM32181_ALS.ASL"
  #endif

  #ifdef PROJECT_I2C_KM61G_ENABLE
  #include "KM61G_SENSOR.ASL"
  #endif

  #ifdef PROJECT_I2C_KXCJ9_ENABLE
  #include "KXCJ9_SENSOR.ASL"
  #endif

  #ifdef PROJECT_I2C_MTKNFC_ENABLE
  #include "MTK6605_NFC.ASL"
  #endif

  #ifdef PROJECT_EC_I2C_SHUB_ENABLE
  #include "EC_SENSORHUB.ASL"
  #endif

