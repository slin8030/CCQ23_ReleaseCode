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
// [COMMON ASL] Using EDK II
//

//
// Common Function No (0x10 - 0x7F)
//
#define CMFC_VERSION_CTRL               0x10
  #define SET_CMFC_AP_VERSION             0x01
  #define GET_CMFC_FW_VERSION             0x02
#define CMFC_GET_INFO_OF_BIOS_SWSMI     0x11
#define CMFC_LCD_EDID_INFO              0x12
#define CMFC_BOOT_DEVICE_SEQUENCE       0x13
#define CMFC_RELOAD_BIOS_DEFUAL_TVALUE  0x14
#define CMFC_EC_COMMUNICATE_INTERFACE   0x15
#define CMFC_S35_FUNCTION               0x16
#define CMFC_EC_SMBUS_ACCESS            0x18
#define CMFC_CHECK_SUM_UTILITY          0x60
#define CMFC_LANGUAGE_OF_SETUP_MENU     0x61
#define CMFC_PHM_FUNCTION               0x62
#define CMFC_PASSWORD_UTILITY           0x63
#define CMFC_SPIRegionProtectControl    0x64

#define CMFC_CLEAR_PASSWORD             0x17
#define CMFC_BOOT_TYPE_FUNCTION         0x20
#define CMFC_SECURE_BOOT_FUNCTION       0x21
#define CMFC_OS_TYPE_FUNCTION           0x22
#define CMFC_VENDOR_NAME_FUNCTION       0x23
#define CMFC_BATTERY_INFO               0x26
#define CMFC_PLATFORM_SETTING           0x27
  #define GET_EC_I2C_DATA                 0x01
  #define CMFC_GET_THE_EC_INTERFACE       0x02
//
// OEM Function No (0x80~0xFE)
// Define in CMFC_OEM_Function_TABLE.ASL
//

//
// CMFC Triger SMI List Table (CTSL)
//
Name (CTSL, Package() {
CMFC_GET_INFO_OF_BIOS_SWSMI,
CMFC_LCD_EDID_INFO,
CMFC_BOOT_DEVICE_SEQUENCE,
CMFC_RELOAD_BIOS_DEFUAL_TVALUE,
CMFC_EC_COMMUNICATE_INTERFACE,
CMFC_S35_FUNCTION,
CMFC_CHECK_SUM_UTILITY,
CMFC_LANGUAGE_OF_SETUP_MENU,
CMFC_PHM_FUNCTION,
CMFC_PASSWORD_UTILITY,
CMFC_SPIRegionProtectControl,
CMFC_BOOT_TYPE_FUNCTION,
CMFC_SECURE_BOOT_FUNCTION,
CMFC_OS_TYPE_FUNCTION,
CMFC_VENDOR_NAME_FUNCTION,
CMFC_BATTERY_INFO,
CMFC_CLEAR_PASSWORD
})

#define WRITE_EC_SMBUS            0x01
#define READ_EC_SMBUS             0x02

// Parameter
#define EC_SMBUS_RETRY_COUNT      5
#define EC_SMBUS_DELAY_MS         20 //Delay 20 mS per check

Mutex(CFMX,0x0)
//
// Common Function Control
//
// Input:
//    Arg0, function call (DWord)
//    Arg1, sub-function call (DWord)
//    Arg2, Input Data (128 Bytes)
//    Arg3, reserved
//
// Return:
//    STDT, return status data (NVS), [1=success, 0=failure] (Word)
//    BFDT, return buffer data (NVS) (128 Bytes)
//
Method  (CFUN,4,Serialized)
{
   // EC SMBUS retry count
   Name(ESRC, EC_SMBUS_RETRY_COUNT)

   //
   // Compare with CMFC Triger SMI List Table (CTSL)
   //
   If(LNotEqual(Match(CTSL, MEQ, DerefOf(Index(Arg0,0)), MTR, 0, 0), ONES)){


      Acquire(CFMX, 0xFFFF)
      Store(Arg0, SW_SMI_DATA_PORT)
      Store(Arg1, SFNO)
      Store(Arg2, BFDT)
      Store(CMFC_SW_SMI, SW_SMI_CMD_PORT)
      Release (CFMX)
   } Else {    // Reset of special command

      // CMFC VERSION CONTROL
      If (LEqual(DerefOf(Index(Arg0,0)), CMFC_VERSION_CTRL)){

         #if CMFC_DEBUG
         Store ( "=== case:CMFC_VERSION_CTRL  ===", Debug )
         #endif

         // sub function call

         // Set CMFC AP Version
         If (LEqual(DerefOf(Index(Arg1,0)), SET_CMFC_AP_VERSION)){

            #if CMFC_DEBUG
            Store ( "=== case:SET_CMFC_AP_VERSION ===", Debug )
            #endif

            CreateByteField (Arg2, 0 , CAPV)
            Store ( CAPV,CAVR)
            Store ( 1,  STDT )

         } // end of SET_CMFC_AP_VERSION

         // Get CMFC FW Version
         ElseIf(LEqual(DerefOf(Index(Arg1,0)), GET_CMFC_FW_VERSION)){

            #if CMFC_DEBUG
            Store ( "=== case:GET_CMFC_CODE_VERSION ===", Debug )
            #endif

            Store (Buffer(128){}, Local0)
            CreateByteField (Local0, 0, BFD0)
            Store ( CMFC_Version, BFD0 )
            Store (1, STDT)
            Store (Local0, BFDT)

         } // end of GET_CMFC_FW_VERSION

         Else {
            Store (0, STDT)
         }
      } // end of CMFC_VERSION_CTRL
      ElseIf(LEqual(DerefOf(Index(Arg0,0)), CMFC_PLATFORM_SETTING)){ // Function 0x27
         If (LEqual(DerefOf(Index(Arg1,0)), GET_EC_I2C_DATA)){ // Sub-Function 0x01
/*
            // get parameters from CMFC tool
            CreateByteField(arg2, 0x00, ECST ) // 0x24 EC 1 = 0x01, EC 2 = 0x02
            CreateByteField(arg2, 0x01, MODE ) // 0x25 Mode : 00 : Byte, 01 : Word, 02: Block
            CreateByteField(arg2, 0x02, CMLN ) // 0x26 Len : ( CMD + Data X )
            CreateByteField(arg2, 0x03, ECCM ) // 0x27 EC CMD
            CreateByteField(arg2, 0x04, ECD0 ) // 0x28 Data0
            CreateByteField(arg2, 0x05, ECD1 ) // 0x29 Data1
            CreateByteField(arg2, 0x06, ECD2 ) // 0x2A Data2
            CreateByteField(arg2, 0x07, ECD3 ) // 0x2B Data3
            CreateField(COMMON_ASL_EC_PATH.ECRD, 8, 56, BUFF) 

            // clear all of status
            Store(0x00, STDT)
            Store(0x00, BFDT)

            // send command and data
            COMMON_ASL_EC_PATH.ECRW(ECCM, ECD0, ECD1, ECD2, ECD3)

            Store(BUFF, BFDT)
            Store (1, STDT)
*/
            // clear all of status
            Store(0x00, STDT)
            Store(0x00, BFDT)
         } // End of Sub-Function 0x09
         ElseIf (LEqual(DerefOf(Index(Arg1,0)), CMFC_GET_THE_EC_INTERFACE )){ // Sub-Function 0x02
            // clear all of status
            Store(0x00, STDT)
            Store(0x00, BFDT)

            Store(EC_PLATFORM_SETTING, BFDT)
            Store (1, STDT)
         }
         Else{
            Store (0, STDT)
         }
      }

      ElseIf (LEqual(DerefOf(Index(Arg0,0)), CMFC_EC_SMBUS_ACCESS))
      {
         Acquire(CFMX, 0xFFFF)

         If (LEqual(DerefOf(Index(Arg1,0)), READ_EC_SMBUS)){

            // Clear all data in EC's return buffer
            Store(0, \_SB.PCI0.LPCB.EC0.SMD0)
            // Write SMBUS slave address
            Store(DerefOf(Index(Arg2,1)), \_SB.PCI0.LPCB.EC0.SMAD)

            // Write command value
            Store(DerefOf(Index(Arg2,2)), \_SB.PCI0.LPCB.EC0.SMCM)

            // Write protocol
            Store(DerefOf(Index(Arg2,0)), \_SB.PCI0.LPCB.EC0.SMPR)

            // Check if command processed
            While(LAnd(Not(LEqual(ESRC, 0)),Not(LEqual(And(\_SB.PCI0.LPCB.EC0.SMST, 0x80),0x80))))
            {
               Sleep(EC_SMBUS_DELAY_MS)
               Subtract(ESRC, 1, ESRC)
            }

            // Check if command is processed (bit7 in \_SB.PCI0.LPCB.EC0.SMST)
            Store(\_SB.PCI0.LPCB.EC0.SMST, Local2)
            If (LEqual(And(Local2, 0x80),0x80))
            {

               Store (Buffer(128){}, Local1)

               // Status byte stores in return buffer offset 0
               Store(Local2, Index(Local1, 0))

               If(LEqual(Local2, 0x80))
               {
                  Store(0xc4,P80H)

                  // Bytes count stores in return buffer offset 1
                  Store(\_SB.PCI0.LPCB.EC0.BCNT, Index(Local1, 1))

                  // Get 32 bytes from EC name space
                  Store(\_SB.PCI0.LPCB.EC0.SMD0, Local3)

                  // Store in return buffer offset 2~33
                  Store(DerefOf(Index(Local3, 0)), Index(Local1, 2))
                  Store(DerefOf(Index(Local3, 1)), Index(Local1, 3))
                  Store(DerefOf(Index(Local3, 2)), Index(Local1, 4))
                  Store(DerefOf(Index(Local3, 3)), Index(Local1, 5))
                  Store(DerefOf(Index(Local3, 4)), Index(Local1, 6))
                  Store(DerefOf(Index(Local3, 5)), Index(Local1, 7))
                  Store(DerefOf(Index(Local3, 6)), Index(Local1, 8))
                  Store(DerefOf(Index(Local3, 7)), Index(Local1, 9))

                  Store(DerefOf(Index(Local3, 8)), Index(Local1,10))
                  Store(DerefOf(Index(Local3, 9)), Index(Local1,11))
                  Store(DerefOf(Index(Local3,10)), Index(Local1,12))
                  Store(DerefOf(Index(Local3,11)), Index(Local1,13))
                  Store(DerefOf(Index(Local3,12)), Index(Local1,14))
                  Store(DerefOf(Index(Local3,13)), Index(Local1,15))
                  Store(DerefOf(Index(Local3,14)), Index(Local1,16))
                  Store(DerefOf(Index(Local3,15)), Index(Local1,17))

                  Store(DerefOf(Index(Local3,16)), Index(Local1,18))
                  Store(DerefOf(Index(Local3,17)), Index(Local1,19))
                  Store(DerefOf(Index(Local3,18)), Index(Local1,20))
                  Store(DerefOf(Index(Local3,19)), Index(Local1,21))
                  Store(DerefOf(Index(Local3,20)), Index(Local1,22))
                  Store(DerefOf(Index(Local3,21)), Index(Local1,23))
                  Store(DerefOf(Index(Local3,22)), Index(Local1,24))
                  Store(DerefOf(Index(Local3,23)), Index(Local1,25))

                  Store(DerefOf(Index(Local3,24)), Index(Local1,26))
                  Store(DerefOf(Index(Local3,25)), Index(Local1,27))
                  Store(DerefOf(Index(Local3,26)), Index(Local1,28))
                  Store(DerefOf(Index(Local3,27)), Index(Local1,29))
                  Store(DerefOf(Index(Local3,28)), Index(Local1,30))
                  Store(DerefOf(Index(Local3,29)), Index(Local1,31))
                  Store(DerefOf(Index(Local3,30)), Index(Local1,32))
                  Store(DerefOf(Index(Local3,31)), Index(Local1,33))

               }

               Store (Local1, BFDT)
               Store (1, STDT)
            }
            Else
            {
               Store(0xc5,P80H)
               Store (0, STDT)
            }
         } //End of READ_EC_SMBUS
         Else
         {
            Store(0xc6,P80H)
            Store (0, STDT)
         }
         Release(CFMX)
      }  // EC_SMBUS_ACCESS
      Else{
         Store (0, STDT)
      }
   }// End of Reset of special command
} // End of CFUN
