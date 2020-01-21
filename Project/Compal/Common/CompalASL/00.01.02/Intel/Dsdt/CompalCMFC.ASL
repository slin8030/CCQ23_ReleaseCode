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

// CMFC Debug
#define CMFC_DEBUG            0           // 0, disable CMFC debug message
                                          // 1, enable CMFC debug message

#define SW_SMI_CMD_PORT       SMIC
#define SW_SMI_DATA_PORT      SMID
#define CMFC_SW_SMI           0xCE
#define CMFC_OEM_SW_SMI       0xCF
#define CMFC_Version          0x11

// return code
#define SUCCESS               0x00000000
#define FUNCTION_NOT_SUPPORT  0x00008000  // Sub-function error or system does not support
#define INPUT_DATA_ERROR      0x00008300
#define WRITE_PROTECT_ERROR   0x00008400
#define FUNCTION_FAILURE      0x00008C00

// return value index
#define RET0                  0
#define RET1                  1
#define RET2                  2
#define RET3                  3

#include "CompalCMFCFunction.ASL"
#include "CompalCMFC_OEM_Function.ASL"

//
// Compal Common Function Control
//
// Input:
//    Arg0, function call
//    Arg1, sub-function call
//    Arg2, reserved
//    Arg3, reserved
//
// Return:
//    Arg0, return code, [0=success]
//    Arg1~3, depend on specification
//      Arg1=manufacture code (2 bytes)
//      Arg2=product call (2 bytes)
//      Arg3=reserved
//
Method (CMFC,4,Serialized)
{
  // Return Value, initial all as 0
  Name (RTVL, Package(0x04) { 0x00000000,
                              0x00000000,
                              0x00000000,
                              0x00000000
                            })

  #if CMFC_DEBUG
  Store ( "=== In CMFC method ===", Debug )
  #endif

   If (LEqual(ObjectType(Arg0), 1)){  // type 1 is integer in 2010 CMFC
   // remove 2010 CMFC function (read EDID manufacture/product ID)
            Store ( FUNCTION_NOT_SUPPORT, Index( RTVL, RET0 ) )     // function not support
   } // End of Arg0 in integer
   Else {  // Arg0 is not integer, assume 2010&2011 CMFC argument type
      If (LLess(DerefOf(Index(Arg0,0)), 0x00FE)) {

         #if CMFC_DEBUG
         Store ( "=== Trigger 0xCE SW SMI ===", Debug )
         #endif

         Store (Buffer(130){}, Local1)                 // Maximum Return Bytes: 130 bytes
         CreateWordField (Local1, 0, RTST)             // Return Status Bytes :   2 bytes
         CreateField (Local1, 16, 1024, RTDT)          // Maximum Data Bytes  : 128 bytes
        if (LLessEqual(ToInteger(Arg0), 0x7F)) {
          // Common Function: 0x00~0x7F
          CFUN(Arg0, Arg1, Arg2, Arg3)
        } else{
          // Project Function: 0x80~0xFE
          PFUN(Arg0, Arg1, Arg2, Arg3)
        }
         Store(STDT, RTST)
         Store(BFDT, RTDT)

         #if CMFC_DEBUG
         Store ( Local1, Debug )
         Store ( "=== Out CMFC method ===", Debug )
         #endif

         return (Local1)

      } else {
         Store ( FUNCTION_NOT_SUPPORT, Index( RTVL, RET0 ) )         // function not support
      }
   } // end of 2010&2011 CMFC

   #if CMFC_DEBUG
   Store ( RTVL, Debug )
   Store ( "=== Out CMFC method ===", Debug )
   #endif

   return (RTVL)
} // End of CMFC

