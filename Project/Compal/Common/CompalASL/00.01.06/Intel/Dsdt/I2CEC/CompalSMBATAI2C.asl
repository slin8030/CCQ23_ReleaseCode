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
 1.01	   07/15/16  Joy Hsiao	  Add CompalEcfilterI2C.asl for thermal utility.
 1.02      08/31/16  Thomas       Modified FFCC, secondary battery function and add definition flags.
*/
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

Scope(COMMON_ASL_EC_PATH){
// Battery Device Object
Device(BAT1){
    Name(_HID, EISAID("PNP0C0A"))
    Name(_UID, 0x01)    
    Name (_DEP, Package() {COMMON_ASL_I2CBAT1_PATH})
    Name(_PCL, Package(1) {\_SB})
    Name(BMDL, 0x00)                                    // Battery Model
    Name(BASC, BatteryReport_mA)                        // define battery source in mA
    Name(BFFW, 0x00)                        		// FFCC workaround flag, function is not supported for default.

    Method(_DSM, 0x4, NotSerialized)
    {
      // Set Battery Charge Throttle
      If(LEqual(Arg0, ToUUID("4c2067e3-887d-475c-9720-4af1d3ed602e")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 0
          If(LEqual(Arg1, 0x0))
          {
            Store("Battery _DSM Query ", Debug)
            Return(Buffer(One) { 0x0f })  // Support 0,1,2,3
          }
        }
        // Revision 0
        If(LEqual(Arg1, 0))
        {
          // Function 1
          If(LEqual(Arg2, 0x1))
          {
            Store ("Set Battery Charge Throttle", Debug)
            Store(Arg3, Debug)
            // taking the current thermal limit into account when engaging charging
            // Thermal Limt(Integer value from 0 to 100)
            // Control battery Charging here.
          }
          // Function 2 User-Serviceable Battery
          // 0x0 if the battery is not user-serviceable
          // and cannot be replaced by the end user,
          // or can be replaced by the end user with additional tools.
          // 0x1 if the battery can be replaced by the end-user without additional tools.
          If(LEqual(Arg2, 0x2))
          {
            Store ("User-Serviceable Battery", Debug)
            return(0x1)   // battery can be replaced by the end-user without additional tools.
          }
          // Function 3 Charging Watchdog
          If(LEqual(Arg2, 0x3))
          {
            Store ("Charging Watchdog ", Debug)
            return(0x1e)   // 30 sec
          }
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }
    } // End of _DSM

    Method(_STA){
        If(COMMON_ASL_EC_PATH.AVBL){ 
          //sample : get platform id
          COMMON_ASL_EC_PATH.ECRW(0x41, 0xA1, 0x00, 0x00, 0x00)
 
          //sample
          COMMON_ASL_EC_PATH.ECNR(ECNAME_BOL0)
          And(COMMON_ASL_EC_PATH.ECR0, 0x01, Local0)
          If(Local0)
          {
              Return (0x1F)
          }             
          Else
          {
          #if (DOCK_BAT1_SUPPORT == 1)
              Return (0x00)
          #else
              Return (0x0F)
          #endif
          }
        }
        Return (0x1F)
    }

    Method(_BIF)
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
                        BAT_CYCLE_COUNT,   //  0x08, Cycle Count

                        BAT_MASURE_ACC,    //  0x09, Measurement Accuracy
                        BAT_MAX_SAMPLT,    //  0x0A, Max Sampling Time
                        BAT_MIN_SAMPLT,    //  0x0B, Min Sampling Time
                        BAT_MAX_AVERAG,    //  0x0C, Max Averaging Interval
                        BAT_MIN_AVERAG,    //  0x0D, Min Averaging Interval

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

        If(COMMON_ASL_EC_PATH.AVBL){
#if (BAT1_FFCC_Support == 1)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_ORRF)             //Set FFCC workaround flag from EC.
            Store(COMMON_ASL_EC_PATH.ECR0, Local0)
            And(COMMON_ASL_EC_PATH.ECR0, 0x40, Local0)
            Store(ShiftRight(Local0, 6),BFFW)
#endif     
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAM0)
            And(COMMON_ASL_EC_PATH.ECR0, 0x02, Local0)
            if(LEqual(Local0, 0x00))           // if Battery type report /10mW
            {
                Store(BatteryReport_mW ,BASC)
            }
            
            Store(ShiftRight(Local0, 1), Index(STAX, BAT_PWR_UNIT))
                
            // BCN0 - Bit0 : 2 cell battery exists
            //        Bit1 : 4 cell battery exists
            //        Bit2 : 6 cell battery exists
            //        Bit3 : 8 cell battery exists
            //        Bit4 : 9 cell battery exists
            //        Bit5 :12 cell battery exists
            //

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BCN0)             // Battery Device Name
            Store(COMMON_ASL_EC_PATH.ECR0, Local0)
            Store(Local0, BMDL)           // Store battery model for

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDC1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local3)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDC0)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local3, 8), Local3)
            Store(Multiply(Local3, BASC), Index(STAX, BAT_DESIGN_CAP))    // Design Capacity
            Sleep(20)

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDV1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local4)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDV0)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local4, 8), Local4)
            Store(Local4, Index(STAX, BAT_DESGIN_VOL))  // Design Voltage
            Sleep(20)

            // Get Batt Last Full Charge Capacity 
            If(LEqual(BFFW, 0x1))
            {
                // [WORKAROUND] If FFCC workaround available,
                // read FFCC to report OS FCC by Local2.
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFF1)
                Store(COMMON_ASL_EC_PATH.ECR0, Local5)
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFF0)
                Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local5, 8), Local2)		
            }else{
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFC1)
                Store(COMMON_ASL_EC_PATH.ECR0, Local5) 
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFC0)
                Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local5, 8), Local2)
            }
            // 1st Batt Last Full Charge Capacity
            Store(Multiply(Local2, BASC), Local2)
            Sleep(20)

            // set design capacity and last full capacity the same
            Store(Local2, Index(STAX, BAT_LAST_FCC))            // 1st Batt Full Charge Capacity

            Divide(Local2, 100, Local0, Local1)                 // full capacity div BAT_DCL save to D100
            Multiply(Local1, BAT_DCW, Local1)
            Store(Local1, Index(STAX, BAT_WARNING_CAP))         // Modify battery percent for Wranging

            Divide(Local2, 100, Local0, Local1)                 // full capacity div BAT_DCL save to D100
            Multiply(Local1, BAT_DCL, Local1)       
            Store(Local1, Index(STAX, BAT_LOW_LEVEL))           // Modify battery percent for low to 

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BYC1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local0)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BYC0)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local0, 8), Local0)
            Store(Local0, Index(STAX, BAT_CYCLE_COUNT))         // Modify battery cycle counter

            COMMON_ASL_EC_PATH.ECNR(ECNAME_ERR0)
            Store(COMMON_ASL_EC_PATH.ECR0, Local0) 
            Store(Local0, Index(STAX, BAT_MASURE_ACC))          //  Measurement Accuracy

            Store(250, Index(STAX, BAT_MAX_SAMPLT))  		    //  Max Sampling Time
            Store(250, Index(STAX, BAT_MIN_SAMPLT))  		    //  Min Sampling Time

            Store(1000, Index(STAX, BAT_MAX_AVERAG))  		    //  Max Averaging Interval
            Store(1000, Index(STAX, BAT_MIN_AVERAG))  		    //  Min Averaging Interval
            
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BSN1)                // Updated battery serail number
            Store(COMMON_ASL_EC_PATH.ECR0, Local0) 
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BSN0)                // Updated battery serail number
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local0, 8), Local0)
            ToDecimalString(Local0, Local1)
            Store(Local1, Index(STAX, BAT_SERIAL_NUM))


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
                  
            If(LEqual(Arg0, 0x01))      // If the running system is not Windows 8.
            {
                Return(STAX)            // _BIX for Windows 8 function.
            }
            else
            {
                Return(STAF)            // _BIF without Windows 8 function .
            }
        }
    } // End of BIFX method

    Method(_BST, 0, NotSerialized)
    {
        Name(PBST, Package()
        {                           // Index
            0,                      // 0x0, battery state
            0xFFFFFFFF,             // 0x1, battery current (unsigned)
            0xFFFFFFFF,             // 0x2, battery remaining capacity
            OEM_DesignCap           // 0x3, battery voltage in mV (must always be known)
        })

        If(COMMON_ASL_EC_PATH.AVBL){     

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BST0)
            Store(COMMON_ASL_EC_PATH.ECR0, Index(PBST, 0))                         // Battery Status
            Sleep(20)

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
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAC1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local4) 
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAC0)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local4, 8), Local4)              // Battery Present Rate

            If(ShiftRight(Local4, 15))
            {
                Add(And(Not(Local4), 0xFFFF), 1, Local4)
            }


            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAM0)
            And(COMMON_ASL_EC_PATH.ECR0, 0x02, Local0)
            If(LEqual(Local0, 0))
            {
                // Check battery Present rate, if battery mode is reported in mW.
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV1)
                Store(COMMON_ASL_EC_PATH.ECR0, Local0)
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV0)
                Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local0, 8), Local3)

                Divide(Multiply(Local4, Local3), 1000, ,Local4)
            }
            Store(Local4, Index(PBST, 1))                       // Battery Present Rate
            Sleep(20)

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BRC1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local5)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BRC0)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local5, 8), Local5)
            Store(Multiply(Local5, BASC), Index(PBST, 0x02))	// Remaining Capacity
            Sleep(20)

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local3)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV0)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local3, 8), Local3)
            Store(Local3, Index(PBST, 3))                       // Precent Voltage
            Sleep(20)

            // Below code to check battery swap after Sx resume 
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BCN0)
            if(LNotEqual(COMMON_ASL_EC_PATH.ECR0, BMDL))        // Battery Device Name, VC20120505 might need to be modified again
            {
                // If the battery swaped, notify OS to update battery information.
                Notify(COMMON_ASL_I2CBAT1_PATH.BAT1, 0x81)
             #if (SECOND_BAT_SUPPORT == 1)
                Notify(COMMON_ASL_I2CBAT2_PATH.BAT2, 0x81)
             #endif
            }
        }
        Return(PBST)
    }

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
          Divide(Arg0, BASC, , Local0)
          COMMON_ASL_EC_PATH.ECNW(ECNAME_BTP0,Local0)
          
          Store(ShiftRight(Local0, 8), Local1)
          COMMON_ASL_EC_PATH.ECNW(ECNAME_BTP1,Local1)
        }                                           
    #endif
    #include "CompalCMFC.asl"
}// End of Device(BAT1)
}// End of Scope(COMMON_ASL_EC_PATH)


//----------------------------------------------------------------------------------------------------------
//  Secondary Battery Device
//----------------------------------------------------------------------------------------------------------
#if (SECOND_BAT_SUPPORT == 1)
Scope(COMMON_ASL_I2CBAT2_PATH){
Device(BAT2){
    Name(_HID, EISAID("PNP0C0A"))
    Name(_UID, 0x02)    
    Name (_DEP, Package() {COMMON_ASL_I2CBAT2_PATH})
    Name(_PCL, Package(1) {\_SB})
    Name(BMDL, 0x00)                                    // Battery Model
    Name(BASC, BatteryReport_mA)                        // define battery source in mA
    Name(BFFW, 0x00)                        		// FFCC workaround flag, function is not supported for default.

   
    Method(_DSM, 0x4, NotSerialized)
    {
      // Set Battery Charge Throttle
      If(LEqual(Arg0, ToUUID("4c2067e3-887d-475c-9720-4af1d3ed602e")))
      {
        // Function 0 : Query Function
        If(LEqual(Arg2, Zero))
        {
          // Revision 0
          If(LEqual(Arg1, 0x0))
          {
            Store("Battery _DSM Query ", Debug)
            Return(Buffer(One) { 0x0f })  // Support 0,1,2,3
          }
        }
        // Revision 0
        If(LEqual(Arg1, 0))
        {
          // Function 1
          If(LEqual(Arg2, 0x1))
          {
            Store ("Set Battery Charge Throttle", Debug)
            Store(Arg3, Debug)
            // taking the current thermal limit into account when engaging charging
            // Thermal Limt(Integer value from 0 to 100)
            // Control battery Charging here.
          }
          // Function 2 User-Serviceable Battery
          // 0x0 if the battery is not user-serviceable
          // and cannot be replaced by the end user,
          // or can be replaced by the end user with additional tools.
          // 0x1 if the battery can be replaced by the end-user without additional tools.
          If(LEqual(Arg2, 0x2))
          {
            Store ("User-Serviceable Battery", Debug)
            return(0x1)   // battery can be replaced by the end-user without additional tools.
          }
          // Function 3 Charging Watchdog
          If(LEqual(Arg2, 0x3))
          {
            Store ("Charging Watchdog ", Debug)
            return(0x1e)   // 30 sec
          }
        }
      }
      Else
      {
        Return(Buffer(One) { 0x00 })
      }
    } // End of _DSM

    Method(_STA){
     
     // if support ducking, default 2nd battery is not existed when
     // battery pulg out.

        If(COMMON_ASL_EC_PATH.AVBL){ 
          //sample : get platform id
          COMMON_ASL_EC_PATH.ECRW(0x41, 0xA1, 0x00, 0x00, 0x00)
 
          //sample
          COMMON_ASL_EC_PATH.ECNR(ECNAME_BOL0)
          And(COMMON_ASL_EC_PATH.ECR0, 0x02, Local0)
          If(Local0)
          {
              Return (0x1F)
          }             
          Else
          {
          #if (DOCK_BAT2_SUPPORT == 1)
              Return (0x00)
          #else
              Return (0x0F)
          #endif
          }
        }
        Return (0x1F)
    }

    Method(_BIF)
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
                        BAT_CYCLE_COUNT,   //  0x08, Cycle Count

                        BAT_MASURE_ACC,    //  0x09, Measurement Accuracy
                        BAT_MAX_SAMPLT,    //  0x0A, Max Sampling Time
                        BAT_MIN_SAMPLT,    //  0x0B, Min Sampling Time
                        BAT_MAX_AVERAG,    //  0x0C, Max Averaging Interval
                        BAT_MIN_AVERAG,    //  0x0D, Min Averaging Interval

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

        If(COMMON_ASL_EC_PATH.AVBL){

#if (BAT2_FFCC_Support == 1)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_ORRF)             //Set FFCC workaround flag from EC.
            Store(COMMON_ASL_EC_PATH.ECR0, Local0)
            And(COMMON_ASL_EC_PATH.ECR0, 0x40, Local0)
            Store(ShiftRight(Local0, 6),BFFW)
#endif     
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAM1)
            And(COMMON_ASL_EC_PATH.ECR0, 0x02, Local0)
            if(LEqual(Local0, 0x00))           // if Battery type report /10mW
            {
                Store(BatteryReport_mW ,BASC)
            }
            
            Store(ShiftRight(Local0, 1), Index(STAX, BAT_PWR_UNIT))
                
            // BCN0 - Bit0 : 2 cell battery exists
            //        Bit1 : 4 cell battery exists
            //        Bit2 : 6 cell battery exists
            //        Bit3 : 8 cell battery exists
            //        Bit4 : 9 cell battery exists
            //        Bit5 :12 cell battery exists
            //

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BCN1)             // Battery Device Name
            Store(COMMON_ASL_EC_PATH.ECR0, Local0)
            Store(Local0, BMDL)           // Store battery model for

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDC3)
            Store(COMMON_ASL_EC_PATH.ECR0, Local3)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDC2)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local3, 8), Local3)
            Store(Multiply(Local3, BASC), Index(STAX, BAT_DESIGN_CAP))    // Design Capacity
            Sleep(20)

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDV3)
            Store(COMMON_ASL_EC_PATH.ECR0, Local4)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BDV2)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local4, 8), Local4)
            Store(Local4, Index(STAX, BAT_DESGIN_VOL))  // Design Voltage
            Sleep(20)

            // Get Batt Last Full Charge Capacity 

            If(LEqual(BFFW, 0x1))
            {
                // [WORKAROUND] If FFCC workaround available,
                // read FFCC to report OS FCC by Local2.
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFF3)
                Store(COMMON_ASL_EC_PATH.ECR0, Local5)
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFF2)
                Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local5, 8), Local2)
            }else{
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFC3)
                Store(COMMON_ASL_EC_PATH.ECR0, Local5) 
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BFC2)
                Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local5, 8), Local2)
            }
            // 1st Batt Last Full Charge Capacity
            Store(Multiply(Local2, BASC), Local2)
            Sleep(20)

            // set design capacity and last full capacity the same
            Store(Local2, Index(STAX, BAT_LAST_FCC))            // 1st Batt Full Charge Capacity

            Divide(Local2, 100, Local0, Local1)                 // full capacity div BAT_DCL save to D100
            Multiply(Local1, BAT_DCW, Local1)
            Store(Local1, Index(STAX, BAT_WARNING_CAP))         // Modify battery percent for Wranging

            Divide(Local2, 100, Local0, Local1)                 // full capacity div BAT_DCL save to D100
            Multiply(Local1, BAT_DCL, Local1)       
            Store(Local1, Index(STAX, BAT_LOW_LEVEL))           // Modify battery percent for low to 

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BYC3)
            Store(COMMON_ASL_EC_PATH.ECR0, Local0)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BYC2)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local0, 8), Local0)
            Store(Local0, Index(STAX, BAT_CYCLE_COUNT))         // Modify battery cycle counter

            COMMON_ASL_EC_PATH.ECNR(ECNAME_ERR1)
            Store(COMMON_ASL_EC_PATH.ECR0, Local0) 
            Store(Local0, Index(STAX, BAT_MASURE_ACC))          //  Measurement Accuracy

            Store(250, Index(STAX, BAT_MAX_SAMPLT))  		    //  Max Sampling Time
            Store(250, Index(STAX, BAT_MIN_SAMPLT))  		    //  Min Sampling Time

            Store(1000, Index(STAX, BAT_MAX_AVERAG))  		    //  Max Averaging Interval
            Store(1000, Index(STAX, BAT_MIN_AVERAG))  		    //  Min Averaging Interval
            
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BSN3)                // Updated battery serail number
            Store(COMMON_ASL_EC_PATH.ECR0, Local0) 
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BSN2)                // Updated battery serail number
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local0, 8), Local0)
            ToDecimalString(Local0, Local1)
            Store(Local1, Index(STAX, BAT_SERIAL_NUM))


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
                  
            If(LEqual(Arg0, 0x01))      // If the running system is not Windows 8.
            {
                Return(STAX)            // _BIX for Windows 8 function.
            }
            else
            {
                Return(STAF)            // _BIF without Windows 8 function .
            }
        }
    } // End of BIFX method

    Method(_BST, 0, NotSerialized)
    {
        Name(PBST, Package()
        {                           // Index
            0,                      // 0x0, battery state
            0xFFFFFFFF,             // 0x1, battery current (unsigned)
            0xFFFFFFFF,             // 0x2, battery remaining capacity
            OEM_DesignCap           // 0x3, battery voltage in mV (must always be known)
        })

        If(COMMON_ASL_EC_PATH.AVBL){     

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BST1)
            Store(COMMON_ASL_EC_PATH.ECR0, Index(PBST, 0))                         // Battery Status
            Sleep(20)

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
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAC3)
            Store(COMMON_ASL_EC_PATH.ECR0, Local4) 
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAC2)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local4, 8), Local4)              // Battery Present Rate

            If(ShiftRight(Local4, 15))
            {
                Add(And(Not(Local4), 0xFFFF), 1, Local4)
            }

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BAM1)
            And(COMMON_ASL_EC_PATH.ECR0, 0x02, Local0)
            If(LEqual(Local0, 0))
            {
                // Check battery Present rate, if battery mode is reported in mW.
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV3)
                Store(COMMON_ASL_EC_PATH.ECR0, Local0)
                COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV2)
                Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local0, 8), Local3)

                Divide(Multiply(Local4, Local3), 1000, ,Local4)
            }
            Store(Local4, Index(PBST, 1))                       // Battery Present Rate
            Sleep(20)

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BRC3)
            Store(COMMON_ASL_EC_PATH.ECR0, Local5)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BRC2)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local5, 8), Local5)
            Store(Multiply(Local5, BASC), Index(PBST, 0x02))    // Remaining Capacity
            Sleep(20)

            COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV3)
            Store(COMMON_ASL_EC_PATH.ECR0, Local3)
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BPV2)
            Or(COMMON_ASL_EC_PATH.ECR0, ShiftLeft(Local3, 8), Local3)
            Store(Local3, Index(PBST, 3))                 	// Precent Voltage
            Sleep(20)

            // Below code to check battery swap after Sx resume 
            COMMON_ASL_EC_PATH.ECNR(ECNAME_BCN1)

        }
        Return(PBST)
    }

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
          Divide(Arg0, BASC, , Local0)
          COMMON_ASL_EC_PATH.ECNW(ECNAME_BTP2,Local0)
          
          Store(ShiftRight(Local0, 8), Local1)
          COMMON_ASL_EC_PATH.ECNW(ECNAME_BTP3,Local1)
        }                                           
    #endif
}// End of Device(BAT2)
}// End of SCOPE(COMMON_ASL_I2CBAT2_PATH)
#endif
//}// End of Scope(COMMON_ASL_EC_PATH)
