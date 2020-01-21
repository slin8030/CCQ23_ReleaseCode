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
 1.01                Thomas       Fix that Windows 10 can't show correctly time for "until fully charged".
 1.02                Thomas       Modify EC_ACCESS_WAIT definition for sometime blue screen when enter Windows.
                                  Change using define from PROJECT_ASL_LPC_PATH to COMMON_ASL_LPC_PATH.

*/

//
// [COMMON ASL] Battery using EDK II
//

#include "OemASL.asl"
#include "CompalSMBATAHKS.ASL"

//
// Battery parameters define.
//
#define BatteryReport_mW        10      // 10 mW - reported from EC
#define BatteryReport_mA        1       // 1 mA  - reported from EC

// BIX battery table
#define BAT_REV         0x00            //  Revision
#define BAT_PWR_UNIT    0x01            //  Report capacity/current in mW/mA
#define BAT_DESIGN_CAP  0x02            //  Design capacity
#define BAT_LAST_FCC    0x03            //  Last full charge capacity
#define BAT_RECHARG     0x04            //  Rechargeable
#define BAT_DESGIN_VOL  0x05            //  Design voltage in mV (must always be known)
#define BAT_WARNING_CAP 0x06            //  Warning capacity level
#define BAT_LOW_LEVEL   0x07            //  Low capacity level
#define BAT_CYCLE_COUNT 0x08            //  Cycle Count
#define BAT_MASURE_ACC  0x09            //  Measurement Accuracy
#define BAT_MAX_SAMPLT  0x0A            //  Max Sampling Time
#define BAT_MIN_SAMPLT  0x0B            //  Min Sampling Time
#define BAT_MAX_AVERAG  0x0C            //  Max Averaging Interval
#define BAT_MIN_AVERAG  0x0D            //  Min Averaging Interval
#define BAT_GRAN        0x0E            //  Capacity granularity between low and warning
#define BAT_GRAN1       0x0F            //  Capacity granularity above warning
#define BAT_MODEL_NUM   0x10            //  Model number = battery name 
#define BAT_SERIAL_NUM  0x11            //  Serial number
#define BAT_TYPE        0x12            //  Battery Type
#define BAT_MAMFACTURER 0x13            //  Battery Manyfacturer

#if (ECRAM_MEMMAP == ENABLE)
  #define EC_ACCESS_WAIT
#else
  #define EC_ACCESS_WAIT        Sleep(20)
#endif


Device(BAT1)
{
        Name(_HID,EISAID("PNP0C0A"))            // PnP ID for Battery Slot
        Name(_UID,0x1)
        Name(_PCL, Package() {\_SB} )           // Devices powered by BAT1
        Name(BMDL, 0x00)                        // Battery Model
        Name(BASC, BatteryReport_mA)            // define battery source in mA
        Name(BFFW, 0x00)                        // FFCC workaround flag, function is not supported for default.

        Method (_STA)
        {

             #ifdef BAT_Statushook
                COMMON_ASL_EC_PATH.BSHK()       // For OEM Battery _STA hooks.
             #endif

                If(ECOK())
                {
                        If(COMMON_ASL_EC_PATH.BOL0)
                        {
                                EC_ACCESS_WAIT
                                Return (0x1F)
                        }
                        Else
                        {
                                EC_ACCESS_WAIT
                                Return (0x0F)
                        }
                }
                Else
                {
                        EC_ACCESS_WAIT
                        Return (0x1F)
                }
         } // end STA

         Method (_BIF)
         {
                Return( BIFX(0) )
         } // end BIF

   #ifdef COMMON_WIN8_BIX_SUPPORT

         //-------------------------------------------------------------------
         // _BIX Method, battery extended static information.
         //              (This object is depecated in ACPI 4.0)
         //-------------------------------------------------------------------
         Method (_BIX)
         {
                Return( BIFX(1) )
         } // end BIX

   #endif
        // --------------------------------------------------------------------------------
        //  Method: BIFX, provide _BIF and _BIX these two table in
        //                one method.
        //
        //  Input: Arg0 = 0, return _BIF table. (Normally using on Windows 7 or earlier)
        //                1, return _BIX table. (Windows 8 built 6370 beginning supported)
        // --------------------------------------------------------------------------------
        Method (BIFX, 1)
        {   
            //
            // Package For _BIX method.
            //
                Name( STAX, Package()
                {
                        0,                 //  0x00, Revision
                        BAT_PWR_Unit,      //  0x01*, report capacity/current in mW/mA
                        OEM_DesignCap,     //  0x02, design capacity
                        OEM_FullChgCap,    //  0x03, last full charge capacity
                        BAT_Rechargeable,  //  0x04*, rechargeable
                        OEM_DesignVol,     //  0x05, design voltage in mV (must always be known)
                        WARN_TP,           //  0x06*, warning capacity level
                        LOW_TP,            //  0x07*, low capacity level
                        0,                 //  0x08, Cycle Count

                        0,                 //  0x09, Measurement Accuracy
                        0,                 //  0x0A, Max Sampling Time
                        0,                 //  0x0B, Min Sampling Time
                        0,                 //  0x0C, Max Averaging Interval
                        0,                 //  0x0D, Min Averaging Interval

                        C_GRAN,            //  0x0E*, capacity granularity between low and warning
                        C_GRAN1,           //  0x0F*, capacity granularity above warning
                        BAT_NAME,          //  0x10, model number = battery name 
                        BAT_SN,            //  0x11, serial number
                        BATT_TYPE,         //  0x12, technology 
                        OEM_MANUFTR        //  0x13, OEM = manufacturer
                })

            //
            // Package For _BIF method.
            //
                Name( STAF, Package()
                {
                        BAT_PWR_Unit,      //  0x0*, report capacity/current in mW/mA
                        OEM_DesignCap,     //  0x1, design capacity
                        OEM_FullChgCap,    //  0x2, last full charge capacity
                        BAT_Rechargeable,  //  0x3*, rechargeable
                        OEM_DesignVol,     //  0x4, design voltage in mV (must always be known)
                        WARN_TP,           //  0x5*, warning capacity level
                        LOW_TP,            //  0x6*, low capacity level
                        C_GRAN,            //  0x7*, capacity granularity between low and warning
                        C_GRAN1,           //  0x8*, capacity granularity above warning
                        BAT_NAME,          //  0x9, model number = battery name
                        BAT_SN,            //  0xA, serial number
                        BATT_TYPE,         //  0xB, technology
                        OEM_MANUFTR        //  0xC, OEM = manufacturer
                })

                //
                // Battery information hooks for each customers.
                //
                #ifdef BAT_UpdateModNum
                  Store(COMMON_ASL_LPC_PATH.GBMN(), Index(STAX, BAT_MODEL_NUM))        // Get Battery Model Number
                #endif

                #ifdef BAT_UpdateSerNum
                  Store(COMMON_ASL_LPC_PATH.GUBS(), Index(STAX, BAT_SERIAL_NUM))       // Update Battery SN
                #endif

                #ifdef BAT_UpdateType
                  Store(COMMON_ASL_LPC_PATH.GUBT(), Index(STAX, BAT_TYPE))             // Update Battery type
                #endif

                #ifdef BAT_UpdateInf
                  Store(COMMON_ASL_LPC_PATH.GUBI(), Index(STAX, BAT_MAMFACTURER))      // Update Battery information
                #endif

                If(ECOK())
                {
                  //
                  // Setting battery parameters
                  //
                  #ifdef BAT_FFCC_Support
                          Store(COMMON_ASL_EC_PATH.ORRF, BFFW)       // Set FFCC workaround flag from EC.
                  #endif

                  if(LEqual(COMMON_ASL_EC_PATH.BAM0, 0x0))                                // if Battery type report /10mW
                  {
                          Store(BatteryReport_mW ,BASC)
                  }

                  Store(COMMON_ASL_EC_PATH.BAM0, Index(STAX, BAT_PWR_UNIT))               // Get Battery Power Mode
                  
                  // BDN0 - Bit0 : 3 cell battery exists
                  //        Bit1 : 4 cell battery exists
                  //        Bit2 : 6 cell battery exists
                  //        Bit3 : 8 cell battery exists
                  //        Bit4 : 9 cell battery exists
                  //        Bit5 :12 cell battery exists
                  //
                  Store(COMMON_ASL_EC_PATH.BDN0, Local0)             // Battery Device Name
                  Store(Local0, BMDL)                                // Store battery model for
                                                                     // swap battery from Sx
                  
                  Store(Multiply(COMMON_ASL_EC_PATH.BDC0, BASC), Index(STAX, BAT_DESIGN_CAP))  // Design Capacity
                  EC_ACCESS_WAIT
                  
                  Store(COMMON_ASL_EC_PATH.BDV0, Index(STAX, BAT_DESGIN_VOL))  // Design Voltage
                  EC_ACCESS_WAIT

                  // Get Batt Last Full Charge Capacity 
                  If(LEqual(BFFW, 0x1))
                  {
                          // [WORKAROUND] If FFCC workaround available,
                          // read FFCC to report OS FCC by Local2.

                          Store(COMMON_ASL_EC_PATH.BFCB, Local2)
                  }else{
                          Store(COMMON_ASL_EC_PATH.BFC0, Local2)
                  }
                  // 1st Batt Last Full Charge Capacity
                  Store(Multiply(Local2, BASC), Local2)
                  EC_ACCESS_WAIT
                  
                  // set design capacity and last full capacity the same
                  Store(Local2, Index(STAX, BAT_LAST_FCC))                // 1st Batt Full Charge Capacity
                  
                  Divide(Local2, 100, Local0, Local1)                     // full capacity div BAT_DCL save to D100
                  Multiply(Local1, BAT_DCW, Local1)
                  Store(Local1, Index(STAX, BAT_WARNING_CAP))             // Modify battery percent for Wranging
                  
                  Divide(Local2, 100, Local0, Local1)                     // full capacity div BAT_DCL save to D100
                  Multiply(Local1, BAT_DCL, Local1)       
                  Store(Local1, Index(STAX, BAT_LOW_LEVEL))               // Modify battery percent for low to 
                  
                  // --------------------------------------
                  // Sync STAX table with STAF table
                  // --------------------------------------
                  Store(DerefOf(Index(STAX, 0x01)), Index(STAF, 0x00))     //  0x0*, report capacity/current in mW/mA
                  Store(DerefOf(Index(STAX, 0x02)), Index(STAF, 0x01))     //  0x1, design capacity
                  Store(DerefOf(Index(STAX, 0x03)), Index(STAF, 0x02))     //  0x2, last full charge capacity
                  Store(DerefOf(Index(STAX, 0x04)), Index(STAF, 0x03))     //  0x3*, rechargeable
                  Store(DerefOf(Index(STAX, 0x05)), Index(STAF, 0x04))     //  0x4, design voltage in mV (must always be known)
                  Store(DerefOf(Index(STAX, 0x06)), Index(STAF, 0x05))     //  0x5*, warning capacity level
                  Store(DerefOf(Index(STAX, 0x07)), Index(STAF, 0x06))     //  0x6*, low capacity level
                  Store(DerefOf(Index(STAX, 0x0E)), Index(STAF, 0x07))     //  0x7*, capacity granularity between low and warning
                  Store(DerefOf(Index(STAX, 0x0F)), Index(STAF, 0x08))     //  0x8*, capacity granularity above warning
                  Store(DerefOf(Index(STAX, 0x10)), Index(STAF, 0x09))     //  0x9, model number = battery name 
                  Store(DerefOf(Index(STAX, 0x11)), Index(STAF, 0x0A))     //  0xA, serial number
                  Store(DerefOf(Index(STAX, 0x12)), Index(STAF, 0x0B))     //  0xB, technology 
                  Store(DerefOf(Index(STAX, 0x13)), Index(STAF, 0x0C))     //  0xC, OEM = manufacturer
                  
                  If(LEqual(Arg0, 0x01))       // If the running system is not Windows 8.
                  {
                          Return(STAX)         // _BIX for Windows 8 function.
                  }
                  else
                  {
                          Return(STAF)         // _BIF without Windows 8 function .
                  }
                  
                }

        } // End of BIFX method

        Method (_BST)
        {
                Name(PBST, Package()
                {                               // Index
                        0,                      // 0x0, battery state
                        _Unknown,               // 0x1, battery current (unsigned)
                        _Unknown,               // 0x2, battery remaining capacity
                        OEM_DesignCap           // 0x3, battery voltage in mV (must always be known)
                })

                If(ECOK())
                {
                        Store(COMMON_ASL_EC_PATH.BST0, Index(PBST, 0))     // Battery Status
                        EC_ACCESS_WAIT

                        // Windows 10 add a feature which battery shows time for "until fully charged".
                        // Battery charge, _BST package offset 0x01 need to fill charged current in integer.
                        // Battery discharge, _BST package offset 0x01 need to fill discharged current in integer.
                        //
                        // Modify battery code as below -
                        // 
                        // 1. if a negative number, battery should be discharge. 
                        //    Reverse number and add 1 for positive.
                        //
                        // 2. if not a negative number, do not reverse number for charge.
                        //
                        If(ShiftRight(COMMON_ASL_EC_PATH.BAC0, 15))
                        {
                               Add(And(Not(COMMON_ASL_EC_PATH.BAC0), 0xFFFF), 1, Local4)
                        }
                        else
                        {
                                Store(COMMON_ASL_EC_PATH.BAC0, Local4) 
                        }

                        If(LEqual(COMMON_ASL_EC_PATH.BAM0, 0))
                        {
                          // Check battery Present rate, if battery mode is reported in mW.
                          Divide(Multiply(Local4, COMMON_ASL_EC_PATH.BPV0), 1000, ,Local4)
                        }

                        Store(Local4, Index(PBST, 1))                      // Battery Present Rate
                        EC_ACCESS_WAIT


                        Store(Multiply(COMMON_ASL_EC_PATH.BRC0, BASC), Index(PBST, 0x02))  // Remaining Capacity
                        EC_ACCESS_WAIT

                        Store(COMMON_ASL_EC_PATH.BPV0, Index(PBST, 3))     // Precent Voltage
                        EC_ACCESS_WAIT

                        // Below code to check battery swap after Sx resume
                        if(LNotEqual(COMMON_ASL_EC_PATH.BDN0, BMDL))       // Battery Device Name
                        {
                                // If the battery swaped, notify OS to update battery information.
                                Notify(COMMON_ASL_LPC_PATH.BAT1, 0x81)

                        }
                }

                Return(PBST)
        } // end BST

   #ifdef COMMON_WIN8_BTP_SUPPORT
        //-------------------------------------------------------------------
        // _BTP Method, battery trap point.
        // 
        // Extra information - 
        //     Please add Query event 0x24 and notify battery device 0x80
        //     to update Battery status.
        //-------------------------------------------------------------------
        Method (_BTP, 1)
        {
                  Divide(Arg0, BASC,, Local0)
                  Store(Local0, COMMON_ASL_EC_PATH.BTPB)
        }                                           
   #endif

}// end device BAT1
