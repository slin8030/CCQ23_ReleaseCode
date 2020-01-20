/** @file
  Power resource and wake capability for USB ports hosting WWAN module

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

//[-start-161109-IB07400810-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// Apollo Lake RVP : GPIO_78(K62) -- USB2_WWAN_PWR_EN -- V3P3A_WWAN_GATED (M.2 WWAN Power??)
// APL-I Oxbow Hill: GPIO_78(K62) -- SOC_AVS_I2S1_SDO -- NGFF_I2S_1_TXD_R_BT -- J2M1.GPIO_8
//
#define NW_GPIO_78            0x00C40630     //AVS_I2S1_SDO

//
// Apollo Lake RVP : GPIO_117(H58) -- NGFF_MODEM_RESET_N -- WWAN_GPS_RESET_N ( Reset Pin )
// APL-I Oxbow Hill: GPIO_117(H58) -- GP_SSP_1_TXD -- J6C1.6 (Header)
//
#define NW_GPIO_117           0x00C40730     //GP_SSP_1_TXD
#endif
//[-end-161109-IB07400810-add]//

  // USB Port 3 power resource
  
  PowerResource(PX03, 0, 0){                            
    Name(WOFF, 0)  // Last OFF Time stamp (WOFF): The time stamp of the last power resource _OFF method evaluation   
        
    Method(_STA)
    {       
//[-start-161109-IB07400810-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      If (LEqual(IOTP(),0x00)) { // CCG Board IDs
        If(LEqual(\_SB.GGOV(NW_GPIO_78), 1)) {
          Return(0x01)
        } Else {
          Return(0x00)
        }
      }
#endif
      Return(0x00)    
//[-end-161109-IB07400810-modify]//
    }
    
    Method(_ON, 0)                                        /// _ON Method
    { 
      If(LNotEqual(^WOFF, Zero)) {
        Divide(Subtract(Timer(), ^WOFF), 10000, , Local0) // Store Elapsed time in ms, ignore remainder
        If(LLess(Local0,100)) {                           // If Elapsed time is less than 100ms
          Sleep(Subtract(100,Local0))                     // Sleep for the remaining time
        } 
      }
      \_SB.SGOV(NW_GPIO_78,1)                             // set power pin to high
      \_SB.SGOV(NW_GPIO_117,1)                            // set reset pin to high
    }
    
    Method(_OFF, 0)                                       /// _OFF Method
    {
      \_SB.SGOV(NW_GPIO_117,0)                            // set reset pin to low
      \_SB.SGOV(NW_GPIO_78,0)                             // set power pin to low
       
      Store(Timer(), ^WOFF)                               // Start OFF timer here.
    }
  } // End PX03
        
  Name(_PR0,Package(){PX03})                 // Power Resource required to support D0
  Name(_PR2,Package(){PX03})                 // Power Resource required to support D2
  Name(_PR3,Package(){PX03})                 // Power Resource required to support D3
  
  //
  // WWAN Modem device with the same power resource as its composite parent device
  //
  Device (MODM) {
    Name(_ADR, 0x3)
    Name(_PR0,Package(){PX03})               // Power Resource required to support D0
    Name(_PR2,Package(){PX03})               // Power Resource required to support D2
    Name(_PR3,Package(){PX03})               // Power Resource required to support D3
  }

  //
  // _SxW, in Sx, the lowest power state supported to wake up the system
  // _SxD, in Sx, the highest power state supported by the device
  // If OSPM supports _PR3 (_OSC, Arg3[2]), 3 represents D3hot; 4 represents D3cold, otherwise 3 represents D3.
  //  
  Method(_S0W, 0, Serialized)
  {
    Return(0x3) // return 3 (D3hot)
  }
  Method (_S3D, 0, Serialized)
  {
    Return (0x2)
  }
  Method (_S3W, 0, Serialized)
  {
    Return (0x3)
  }
  Method (_S4D, 0, Serialized)
  {
    Return (0x2)
  }
  Method (_S4W, 0, Serialized)
  {
    Return (0x3)
  }

