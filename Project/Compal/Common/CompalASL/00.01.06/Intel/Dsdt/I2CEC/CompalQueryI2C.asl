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
 1.00                             Init version.
 1.01      16/08/01  Vanessa      1.Add Mutex for GPE event(I2C Query) to make sure system finish one event.
                                  2.Support second battery.
 1.02      16/08/31  Thomas       Modified secondary battery and docking feature.
*/

#define EC_QUERY_AC_PLUG_OUT              0x01          //[COM] AC plug out
#define EC_QUERY_AC_PLUT_IN               0x02          //[COM] AC plug in
#define EC_QUERY_BATTERY_LOWLOW           0x03          //[COM] Low low battery
#define EC_QUERY_BATTERY_LOW              0x04          //[COM] Low battery
#define EC_QUERY_BATTERY1_PLUG_IN         0x05          //[COM] First Battery
#define EC_QUERY_LID_PRESSED              0x06          //[COM] Lid pressed
#define EC_QUERY_BAT_CHARGE_COMPLETION    0x07          //[COM] Battery charge completion for connected standby specification
#define EC_QUERY_DOCK                     0x08          //[COM] Dock
#define EC_QUERY_UNDOCK                   0x09          //[COM] Undock
#define EC_QUERY_AC_WRONG_ADP             0x0A          //[COM] Wrong adaptor
#define EC_QUERY_BATTERY2_PLUG_IN         0x0B          //[COM] Second battery
#define EC_QUERY_P_SENSOR                 0x0C          //[COM] P-Sensor
#define EC_QUERY_HP                       0x0D          //[COM] HP IN, OUT
#define EC_QUERY_START_OS                 0x0E          //[COM] Start OS
#define EC_QUERY_BATTERY_TRIP_POINT       0x0F          //[COM] Win8 _BTP method  
#define EC_QUERY_THROTTLE                 0x1D          //[COM] Throttle event  

#define P_SENSOR_SERVICE_EVENT                  0
#define P_SENSOR_TRIGER_EVENT                   1


Scope(\_SB.GPO1){
      Name(QECS, 0x00)                                    //[COM] Query Event command
      //Name(PSES, 0x00)                                    //[COM] P-Sensor status
      Mutex(CQMX,0x0)
  
      // Soc EC_INT pin for GPE events define at oemasl.asl
      Method(I2CEC_SCI, 0x0, NotSerialized)
        {
          Acquire(CQMX, 0xFFFF)

          Store ("ENEIO373X issue an event", Debug)
            If(COMMON_ASL_EC_PATH.AVBL){ 
              COMMON_ASL_EC_PATH.ECRW(0x84, 0x00, 0x00, 0x00, 0x00) //[COM] Query Event command
              Store(COMMON_ASL_EC_PATH.ECR0, QECS)
            }
            
              Switch (QECS){
                Case (EC_QUERY_AC_PLUG_OUT){                
                  Notify(\_SB.ADP1, 0x80)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
                  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif
                  Notify(\_SB.DPTF, 0x86)                       //notify DPTF device
                  Notify(\_SB.TCHG, 0x80)                       //notify Charger device
                  break
                }
                Case (EC_QUERY_AC_PLUT_IN){                
                  Notify(\_SB.ADP1, 0x80)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
                  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif
                  Notify(\_SB.DPTF, 0x86)                       //notify DPTF device
                  Notify(\_SB.TCHG, 0x80)                       //notify Charger device
                  break
                }            
                Case (EC_QUERY_BATTERY_LOWLOW){
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
                  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif
                  break
                }
                Case (EC_QUERY_BATTERY_LOW){
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
                  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif
                  break
                }            
                Case (EC_QUERY_BATTERY1_PLUG_IN){
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x81)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x81)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #endif
                  Notify(\_SB.DPTF, 0x86)                       //notify DPTF device
                  Notify(\_SB.TCHG, 0x80)                       //notify Charger device
                  break
                }            
                Case (EC_QUERY_LID_PRESSED){
                  Notify(COMMON_ASL_EC_PATH.LID, 0x80)
                  break
                }
                Case (EC_QUERY_BAT_CHARGE_COMPLETION){
		  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
		  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif
                  break
                }            
                Case (EC_QUERY_DOCK){
                #if (DOCK_BAT1_SUPPORT == 1)
		  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #endif 
                #if (DOCK_BAT2_SUPPORT == 1)
		  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif 
                  break
                }
                Case (EC_QUERY_UNDOCK){
                #if (DOCK_BAT1_SUPPORT == 1)
		  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #endif
                #if (DOCK_BAT2_SUPPORT == 1)
		  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif 
                  break
                }            
                Case (EC_QUERY_AC_WRONG_ADP){
    				    	break
    				    }  
    			            
                Case (EC_QUERY_BATTERY2_PLUG_IN){  
                #if (SECOND_BAT_SUPPORT == 1)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x81)
                  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
		  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
		  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x81)
                #endif 
                  break
                }         

                Case (EC_QUERY_P_SENSOR) {

                  break
                }

                Case (EC_QUERY_HP) {

                  break
                }

                Case (EC_QUERY_START_OS) {

                  break
                }                
                Case (EC_QUERY_BATTERY_TRIP_POINT) {
                  Sleep(100)
		  Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x80)
                #if (SECOND_BAT_SUPPORT == 1)
		  Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x80)
                #endif
                  break
                }
                Case (EC_QUERY_THROTTLE) {

                  COMMON_ASL_EC_PATH.ECNR(ECNAME_PCVL)          // read throttle level
                  Store(COMMON_ASL_EC_PATH.ECR0, Local0)
                  Store(Local0, \_PR.CPU0._PPC)                  
                  PNOT()                                        // Notify OS Performance Present Capabilities Changed.
                  Notify(\_SB.DPTF, 0x86)                       //notify DPTF device
                  Notify(\_SB.TCHG, 0x80)                       //notify Charger device
                  break
                }                
              }
          Release (CQMX)
        }

}

