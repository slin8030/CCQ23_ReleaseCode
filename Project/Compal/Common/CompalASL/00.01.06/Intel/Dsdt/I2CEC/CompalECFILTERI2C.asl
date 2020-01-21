/*
 * (C) Copyright 2012-2020 Compal Electronics, Inc.
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
 1.01	   07/15/16  Joy Hsiao	  Update SW SMI to 0xD9 and definition of sensor
*/

//
// [COMMON ASL] EC functions using EDK II
//

//
// Please refer below for more information to set ECFILTER.
// -----------------------------------------------------------------
// If set INTEL_ECFILTER == "ENABLE" on OEMASL.asl,
//     you can only find Intel function after ASL compile.
// If set INTEL_ECFILTER == "DISABLE" on OEMASL.asl,
//     you can only find AMD function after ASL compile.
//
// Default setting - INTEL_ECFILTER == ENABLE
// -----------------------------------------------------------------
// If set CPU_THROTTLING_SUPPORT == "ENABLE" on OEMASL.asl 
// CPU throttling information will show in Thermal utility.
// If set CPU_THROTTLING_SUPPORT == "DISABLE" on OEMASL.asl
// CPU throttling information will not show in Thermal utility.
// 
// Default setting - CPU_THROTTLING_SUPPORT == DISABLE

Mutex(TMSX,0x0)

Method(TMSF, 1)
{
	Acquire(TMSX, 0xFFFF)

	Store(Arg0,Local0)
	Switch(Local0)                     // TMSF function Paramater
      {
        Case (0x00){		           // Function Support , return 0x00(Not support),return 0x01(support)
          Store(0x01, Local1)               
        }

        Case (0x01){			   // Sensor 1 , EC ram 0x840E

          COMMON_ASL_EC_PATH.ECNR(ECNAME_STP1)
          Store(COMMON_ASL_EC_PATH.ECR0, Local1)
        }

        Case (0x02){			   // Sensor 2 , EC ram 0x840D

          COMMON_ASL_EC_PATH.ECNR(ECNAME_STP2)
          Store(COMMON_ASL_EC_PATH.ECR0, Local1)
        }


        Case (0x06){			   // Battery temp, EC ram 0x84D7

          COMMON_ASL_EC_PATH.ECNR(ECNAME_AVT0)
          Store(COMMON_ASL_EC_PATH.ECR0, Local1)          
        }

        Case (0x08){			   // CPU Temperature 

	  CSMI(STPM_Thermal_Utility_SW_SMI,Read_DTS_Temperature)	
          Store(TMUD,Local1)
        }
	
        Default {                          
          Store(0x00, Local1)
        }
      }
	
	Release (TMSX)
		
	Return (Local1)
}
