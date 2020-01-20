/** @file
  ACPI RTD3 SSDT table for APLK

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
// ApolloLake RVP PCI-E HW map:
// ----------------------------------------------------------------------------------------------
// |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
// |RP1D|   4|   0|0/0x14/0|x4 Slot (Slot 2)|2(W_GPIO_211)|2(SW_GPIO_207)|N_GPIO_13 | N_GPIO_17 |
// |RP2D|   5|   1|0/0x14/1|M.2 WLAN        |3(W_GPIO_212)|3(SW_GPIO_208)|N_GPIO_15 | n/a       |
// |RP3D|   0|   2|0/0x13/0|x4 Slot (Slot 1)|0(W_GPIO_209)|0(SW_GPIO_205)|W_GPIO_152| N_GPIO_19 |
// |RP4D|   1|   3|0/0x13/1|x4 Slot (Slot 1)|n/a          |n/a           |n/a       | n/a       |
// |RP5D|   2|   4|0/0x13/2|i211            |1(W_GPIO_210)|1(SW_GPIO_206)|N_GPIO_37 | n/a       |
// |RP6D|   3|   5|0/0x13/3|n/a             |n/a          |n/a           |n/a       | n/a       |
// ----------------------------------------------------------------------------------------------
//
// ApolloLake-I OxBowHill PCI-E HW map:
// ----------------------------------------------------------------------------------------------
// |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
// |RP1D|   4|   0|0/0x14/0|M.2 3G          |2(W_GPIO_211)|2(SW_GPIO_207)|n/a       | n/a       |
// |RP2D|   5|   1|0/0x14/1|M.2 Wifi        |3(W_GPIO_212)|3(SW_GPIO_208)|n/a       | n/a       |
// |RP3D|   0|   2|0/0x13/0|x4 slot (x2)    |1(W_GPIO_210)|1(SW_GPIO_206)|n/a       | n/a       |
// |RP4D|   1|   3|0/0x13/1|x4 slot (x2)    |n/a          |n/a           |n/a       | n/a       |
// |RP5D|   2|   4|0/0x13/2|i210            |0(W_GPIO_209)|0(SW_GPIO_205)|n/a       | n/a       |
// |RP6D|   3|   5|0/0x13/3|n/a             |n/a          |n/a           |n/a       | n/a       |
// ----------------------------------------------------------------------------------------------
//
// Apollo Lake RVP PCIe Reset Pins:
//
#define N_GPIO_13             0x00C50568     //GPIO_13, Reset Pin
#define N_GPIO_15             0x00C50578     //GPIO_15, Reset Pin
#define W_GPIO_152            0x00C705b0     //ISH_GPIO_6, Reset Pin
#define N_GPIO_37             0x00C50628     //PWM3, Reset Pin

//
// Apollo Lake RVP PCIe Power Pins:
//
#define N_GPIO_17             0x00C50588     //GPIO_17, Power Pin
#define N_GPIO_19             0x00C50598     //GPIO_19, Power Pin

//
// Apollo Lake RVP SATA Power Pin
//
#define N_GPIO_22             0x00C505B0     //GPIO_22

//
// PCIe Clock Req Pins
//
#define W_GPIO_209            0x00C705d0     //PCIE_CLKREQ0_B
#define W_GPIO_211            0x00C705e0     //PCIE_CLKREQ2_B
#define W_GPIO_210            0x00C705d8     //PCIE_CLKREQ1_B
#define W_GPIO_212            0x00C705e8     //PCIE_CLKREQ3_B

//
// PCIe Wake Pins
//
#define SW_GPIO_205           0x00C00500     //PCIE_WAKE0_B
#define SW_GPIO_206           0x00C00508     //PCIE_WAKE1_B
#define SW_GPIO_207           0x00C00510     //PCIE_WAKE2_B
#define SW_GPIO_208           0x00C00518     //PCIE_WAKE3_B

//[-start-170410-IB07400857-add]//
//
// UART2 GPIO Pins
//
#define N_GPIO_46             0x00C50670     //LPSS_UART2_RXD
//#define N_GPIO_47             0x00C50678     //LPSS_UART2_TXD
//#define N_GPIO_48             0x00C50680     //LPSS_UART2_RTS_B
#define N_GPIO_49             0x00C50688     //LPSS_UART2_CTS_B
//[-end-170410-IB07400857-add]//
#endif
//[-end-161109-IB07400810-add]//

DefinitionBlock (
    "Rtd3.aml",
    "SSDT",
    2,
    "AcpiRef",
    "RVPRtd3",
    0x1000
    )
{
External(RTD3, IntObj)
External(EMOD, IntObj)
External(RCG0, IntObj)
External(RPA1, IntObj)
External(RPA2, IntObj)
External(RPA3, IntObj)
External(RPA4, IntObj)
External(RPA5, IntObj)
External(RPA6, IntObj)
External(\_SB.GPO0.AVBL, IntObj)
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
External(\_SB.GPO0.ODPW, IntObj) // N_GPIO_22
#endif
//[-end-161125-IB07400818-modify]//
External(\_SB.GGOV, MethodObj)
External(\_SB.SGOV, MethodObj)
//External(\_SB.SHPO, MethodObj)
External(OSYS)
External(HGEN)
External(S0ID)

External(PPBM)
External(PBAS)

//[-start-161104-IB07400809-modify]//
//#if BXTI_PF_ENABLE
External(RP1D, IntObj)
External(RP2D, IntObj)
External(RP3D, IntObj)
External(RP4D, IntObj)
External(RP5D, IntObj)
External(RP6D, IntObj)
//#endif
//[-end-161104-IB07400809-modify]//

Include ("BxtPGpioDefine.asl")
Include ("Rtd3Common.asl")

//
// PCIe root ports - START
//
  ///
  /// PCIe RTD3 - SLOT#1
  ///
  Scope(\_SB.PCI0.RP01)
  {
    // reset pin = N_GPIO_13
    // power pin = N_GPIO_17
    // wake pin  = SW_GPIO_207
    // CLK_REQ   = Mask Enable of Mapped CLKREQ# for CLKOUT_SRC2 (MSKCRQSRC2):
    Name(SLOT, 1)   // port #1

//[-start-161109-IB07400810-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // ApolloLake RVP PCI-E Port 0 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP1D|   4|   0|0/0x14/0|x4 Slot (Slot 2)|2(W_GPIO_211)|2(SW_GPIO_207)|N_GPIO_13 | N_GPIO_17 |
    // ----------------------------------------------------------------------------------------------
    //
    Name(RSTG, Package() {
              1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
              0, // IOEX #
      N_GPIO_13, // GPIO pad #/IOEX pin #
              1  // reset pin de-assert polarity
    })
    Name(PWRG, Package() {
              1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
              0, // IOEX #
      N_GPIO_17, // GPIO pad #/IOEX pin #
              1  // power on polarity
    })
    Name(WAKG, Package() {
                1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                0, // IOEX #
      SW_GPIO_207, // MSKCRQSRC2
                1  // wake en polarity
    })
    Name(SCLK, Package() {
               1, // 0-disable, 1-enable
      W_GPIO_211, // MSKCRQSRC2
               0  // assert polarity, ICC should be LOW-activated
    })
    //
    // ApolloLake-I OxBowHill PCI-E Port 0 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP1D|   4|   0|0/0x14/0|M.2 3G          |2(W_GPIO_211)|2(SW_GPIO_207)|n/a       | n/a       |
    // ----------------------------------------------------------------------------------------------
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTI, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRI, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKI, Package() {    1,     0,SW_GPIO_207,        1}) // Wake   Pin  
    Name(SCLI, Package() {    1,        W_GPIO_211,        0}) // ClkReq Pin  
#else
    //
    // OEM Lane 4 (0/0x14/0, Port 0) RTD3 setting, default disable
    //
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTG, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRG, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKG, Package() {    0,     0,          0,        0}) // Wake   Pin  
    Name(SCLK, Package() {    0,                 0,        0}) // ClkReq Pin  
#endif
//[-end-161109-IB07400810-modify]//
    Include("Rtd3BxtPcie.asl")
  }

  ///
  /// PCIe RTD3 - SLOT#2
  ///
  Scope(\_SB.PCI0.RP02)
  {
    // reset pin = N_GPIO_15
    // power pin = N_GPIO_17
    // wake pin  = SW_GPIO_208
    // CLK_REQ   = Mask Enable of Mapped CLKREQ# for CLKOUT_SRC1 (MSKCRQSRC1):
    Name(SLOT, 2)   // port #2

//[-start-161110-IB07400810-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // ApolloLake RVP PCI-E Port 1 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP2D|   5|   1|0/0x14/1|M.2 WLAN        |3(W_GPIO_212)|3(SW_GPIO_208)|N_GPIO_15 | n/a       |
    // ----------------------------------------------------------------------------------------------
    //
    Name(RSTG, Package() {
              1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
              0, // IOEX #
      N_GPIO_15, // GPIO pad #/IOEX pin #
              1  // reset pin de-assert polarity
    })
    Name(PWRG, Package() {
              0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
              0, // IOEX #
              0, // GPIO pad #/IOEX pin #
              0  // power on polarity
    })
    Name(WAKG, Package() {
              0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
              0, // IOEX #
    SW_GPIO_208, // GPIO pad #/IOEX pin #
              1  // wake en polarity
    })
    Name(SCLK, Package() {
              1, // 0-disable, 1-enable
     W_GPIO_212, // MSKCRQSRC1
              0  // assert polarity, ICC should be LOW-activated
    })
    //
    // ApolloLake-I OxBowHill PCI-E Port 1 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP2D|   5|   1|0/0x14/1|M.2 Wifi        |3(W_GPIO_212)|3(SW_GPIO_208)|n/a       | n/a       |
    // ----------------------------------------------------------------------------------------------
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTI, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRI, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKI, Package() {    1,     0,SW_GPIO_208,        1}) // Wake   Pin  
    Name(SCLI, Package() {    1,        W_GPIO_212,        0}) // ClkReq Pin  
#else
    //
    // OEM Lane 5 (0/0x14/1, Port 1) RTD3 setting, default disable
    //
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTG, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRG, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKG, Package() {    0,     0,          0,        0}) // Wake   Pin  
    Name(SCLK, Package() {    0,                 0,        0}) // ClkReq Pin  
#endif
//[-end-161110-IB07400810-modify]//

    Include("Rtd3BxtPcie.asl")
  }

  ///
  /// PCIe RTD3 - SLOT#3
  ///
  If (LEqual (HGEN, 0)) {
    Scope(\_SB.PCI0.RP03)
    {
//[-start-161110-IB07400810-modify]//
//[-start-161104-IB07400809-modify]//
      Name(SLOT, 3)   // port #3
      
//#if BXTI_PF_ENABLE
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      // reset pin = none
      // power pin = none
      // wake pin  = SW_GPIO_206
      // CLK_REQ   = W_GPIO_210
//      Name(SLOT, 3)   // port #3
      //
      // ApolloLake-I OxBowHill PCI-E Port 2 HW map:
      // ----------------------------------------------------------------------------------------------
      // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
      // |RP3D|   0|   2|0/0x13/0|x4 slot (x2)    |1(W_GPIO_210)|1(SW_GPIO_206)|n/a       | n/a       |
      // ----------------------------------------------------------------------------------------------
      //
      Name(RSTI, Package() {
                 0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // IOEX #
                 0, // GPIO pad #/IOEX pin #
                 0  // reset pin de-assert polarity
      })
      Name(PWRI, Package() {
                 0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // IOEX #
                 0, // GPIO pad #/IOEX pin #
                 0  // power on polarity
      })
      Name(WAKI, Package() {
                 1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // IOEX #
       SW_GPIO_206, // GPIO pad #/IOEX pin #
                 1  // wake en polarity
      })
      Name(SCLI, Package() {
                 1, // 0-disable, 1-enable
        W_GPIO_210, // MSKCRQSRCx
                 0  // assert polarity, ICC should be LOW-activated
      })
//#else
      // reset pin = W_GPIO_152
      // power pin = N_GPIO_19
      // wake pin  = SW_GPIO_205
      // CLK_REQ   = W_GPIO_209
//      Name(SLOT, 3)   // port #3
      //
      // ApolloLake RVP PCI-E Port 2 HW map:
      // ----------------------------------------------------------------------------------------------
      // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
      // |RP3D|   0|   2|0/0x13/0|x4 Slot (Slot 1)|0(W_GPIO_209)|0(SW_GPIO_205)|W_GPIO_152| N_GPIO_19 |
      // ----------------------------------------------------------------------------------------------
      //
      Name(RSTG, Package() {
                 1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // IOEX #
        W_GPIO_152, // GPIO pad #/IOEX pin #
                 1  // reset pin de-assert polarity
      })
      Name(PWRG, Package() {
                 1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // IOEX #
         N_GPIO_19, // GPIO pad #/IOEX pin #
                 1  // power on polarity
      })
      Name(WAKG, Package() {
                 1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // IOEX #
       SW_GPIO_205, // GPIO pad #/IOEX pin #
                 1  // wake en polarity
      })
      Name(SCLK, Package() {
                 1, // 0-disable, 1-enable
        W_GPIO_209, // MSKCRQSRCx
                 0  // assert polarity, ICC should be LOW-activated
      })
//#endif
#else
      //
      // OEM Lane 0 (0/0x13/0, Port 2) RTD3 setting, default disable
      //
      //                    EnDis,IOEX #,       GPIO, polarity
      Name(RSTG, Package() {    0,     0,          0,        0}) // Reset  Pin
      Name(PWRG, Package() {    0,     0,          0,        0}) // Power  Pin
      Name(WAKG, Package() {    0,     0,          0,        0}) // Wake   Pin  
      Name(SCLK, Package() {    0,                 0,        0}) // ClkReq Pin  
#endif
//[-end-161104-IB07400809-modify]//
//[-end-161110-IB07400810-modify]//
      Include("Rtd3BxtPcie.asl")
    }
  }

  ///
  /// PCIe RTD3 - SLOT#4
  ///
  Scope(\_SB.PCI0.RP04)
  {
    // reset pin = none
    // power pin = none
    // wake pin  = none
    // CLK_REQ   = Mask Enable of Mapped CLKREQ# for CLKOUT_SRC5 (MSKCRQSRC5)
    Name(SLOT, 4)   ///- Slot #4
    //  
    // ApolloLake RVP PCI-E port 3 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP4D|   1|   3|0/0x13/1|x4 Slot (Slot 1)|n/a          |n/a           |n/a       | n/a       |
    // ----------------------------------------------------------------------------------------------
    //
    Name(RSTG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // reset pin de-assert polarity
        })
    Name(PWRG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // power on polarity
        })
    Name(WAKG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // wake en polarity
        })
    Name(SCLK, Package() {
            0, // 0-disable, 1-enable
            0, // MSKCRQSRC5
            0  // assert polarity, ICC should be LOW-activated
        })
//[-start-161110-IB07400810-add]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //    
    // ApolloLake-I OxBowHill PCI-E port 3 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP4D|   1|   3|0/0x13/1|x4 slot (x2)    |n/a          |n/a           |n/a       | n/a       |
    // ----------------------------------------------------------------------------------------------
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTI, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRI, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKI, Package() {    0,     0,          0,        0}) // Wake   Pin  
    Name(SCLI, Package() {    0,                 0,        0}) // ClkReq Pin  
#endif
//[-end-161110-IB07400810-add]//
    Include("Rtd3BxtPcie.asl")
  }

  ///
  /// PCIe RTD3 - SLOT#5
  ///
  Scope(\_SB.PCI0.RP05)
  {
    // reset pin = none
    // power pin = none
    // wake pin  = none
    // CLK_REQ   = Mask Enable of Mapped CLKREQ# for CLKOUT_SRC4 (MSKCRQSRC4)
    Name(SLOT, 5)   ///- port #5
      
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // ApolloLake RVP PCI-E HW Port 4 map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP5D|   2|   4|0/0x13/2|i211            |1(W_GPIO_210)|1(SW_GPIO_206)|N_GPIO_37 | n/a       |
    // ----------------------------------------------------------------------------------------------
    //
    Name(RSTG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // reset pin de-assert polarity
        })
    Name(PWRG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // power on polarity
        })
//#if BXTI_PF_ENABLE
//    Name(WAKG, Package() {
//            1, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
//            0, // IOEX #
//            SW_GPIO_205, // GPIO pad #/IOEX pin #
//            1  // wake en polarity
//        })
//#else
    Name(WAKG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
  SW_GPIO_206, // GPIO pad #/IOEX pin #
            0  // wake en polarity
         })
//#endif
    Name(SCLK, Package() {
            0, // 0-disable, 1-enable
   W_GPIO_210, // MSKCRQSRC4
            0  // assert polarity, ICC should be LOW-activated
         })
    //        
    // ApolloLake-I OxBowHill PCI-E Port 4 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP5D|   2|   4|0/0x13/2|i210            |0(W_GPIO_209)|0(SW_GPIO_205)|n/a       | n/a       |
    // ----------------------------------------------------------------------------------------------
    //
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTI, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRI, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKI, Package() {    0,     0,SW_GPIO_205,        1}) // Wake   Pin  
    Name(SCLI, Package() {    0,        W_GPIO_209,        0}) // ClkReq Pin   
#else
    //
    // OEM Lane 2 (0/0x13/2, Port 4) RTD3 setting, default disable
    //
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTG, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRG, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKG, Package() {    0,     0,          0,        0}) // Wake   Pin  
    Name(SCLK, Package() {    0,                 0,        0}) // ClkReq Pin  
#endif
    Include("Rtd3BxtPcie.asl")
  }

  ///
  /// PCIe RTD3 - SLOT#6
  ///
  Scope(\_SB.PCI0.RP06)
  {
    // reset pin = none
    // power pin = none
    // wake pin  = none
    // CLK_REQ   = Mask Enable of Mapped CLKREQ# for CLKOUT_SRC4 (MSKCRQSRC4)
    Name(SLOT, 6)   ///- port #6
      
    //
    // ApolloLake RVP PCI-E Port 5 HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP6D|   3|   5|0/0x13/3|n/a             |n/a          |n/a           |n/a       | n/a       |
    // ----------------------------------------------------------------------------------------------
    //
    Name(RSTG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // reset pin de-assert polarity
        })
    Name(PWRG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // power on polarity
        })
    Name(WAKG, Package() {
            0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
            0, // IOEX #
            0, // GPIO pad #/IOEX pin #
            0  // wake en polarity
        })
    Name(SCLK, Package() {
          0,    // 0-disable, 1-enable
          0, // MSKCRQSRC4
          0     // assert polarity, ICC should be LOW-activated
        })
              
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // ApolloLake-I OxBowHill PCI-E HW map:
    // ----------------------------------------------------------------------------------------------
    // |ACPI|Lane|Port|B/D/F   |PCIe Device     |ClkReq#      |Wake#         |ResetPin  | PowerPin  |
    // |RP6D|   3|   5|0/0x13/3|n/a             |n/a          |n/a           |n/a       | n/a       |
    // ---------------------------------------------------------------------------------------------- 
    //                    EnDis,IOEX #,       GPIO, polarity
    Name(RSTI, Package() {    0,     0,          0,        0}) // Reset  Pin
    Name(PWRI, Package() {    0,     0,          0,        0}) // Power  Pin
    Name(WAKI, Package() {    0,     0,          0,        0}) // Wake   Pin  
    Name(SCLI, Package() {    0,                 0,        0}) // ClkReq Pin  
#endif
    Include("Rtd3BxtPcie.asl")
  }
//
// PCIe root ports - END
//

//
// SATA - START
//

  Scope(\_SB.PCI0.SATA) {

    OperationRegion (PMCS, PCI_Config, 0x74, 0x4)
    Field (PMCS, WordAcc, NoLock, Preserve) {
      PMSR, 32,   // 0x74, PMCSR - Power Management Control and Status
    }

    /// _PS0 Method for SATA HBA
    Method(_PS0,0,Serialized)
    {
    }

    /// _PS3 Method for SATA HBA
    Method(_PS3,0,Serialized)
    {
      // dummy read PMCSR
      Store (PMSR, Local0)
	    And (Local0, 1, Local0) // Dummy operation on Local0
    }

    /// Define SATA PCI Config OperationRegion
    OperationRegion(SMIO,PCI_Config,0x24,4)
    Field(SMIO,AnyAcc, NoLock, Preserve) {
      Offset(0x00), ///- SATA MABR6
      MBR6, 32,     ///- SATA ABAR
    }

    Scope(PRT0) {
      // Define _PR0, _PR3 PowerResource Package
      // P0 command port = ABAR + 0x118
      // power pin = N_GPIO_22
      Name(PORT, 0)
      Name(PBAR, 0x118)       // port0 command port address
//[-start-161110-IB07400810-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      Name(PWRG, Package() {
                 1,         // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0,         // GPIO group #/IOEX #
                 N_GPIO_22, // GPIO pad #/IOEX pin #
                 1          // power on polarity
               })
      // Power Pin For IOTG board
      Name(PWRI, Package() {
                 0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // GPIO group #/IOEX #
                 0, // GPIO pad #/IOEX pin #
                 0  // power on polarity
               })
#else
      // Power Pin For OEM board, default disable
      Name(PWRG, Package() {
                 0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // GPIO group #/IOEX #
                 0, // GPIO pad #/IOEX pin #
                 0  // power on polarity
               })
#endif               
//[-end-161110-IB07400810-modify]//
        Include("Rtd3BxtSata.asl")
    } // end device(PRT0)

    Scope(PRT1) {
      // Define _PR0, _PR3 PowerResource Package
      // P1 command port = ABAR + 0x198
      // power pin = none
      Name(PORT, 1)
      Name(PBAR, 0x198)       // port1 command port address
      Name(PWRG, Package() {  // all the fields are 0 for PRT1 because its not routed on reference platform
                 0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // GPIO group #/IOEX #
                 0, // GPIO pad #/IOEX pin #
                 0  // power on polarity
               })
//[-start-161109-IB07400810-add]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
      // Power Pin For IOTG board
      Name(PWRI, Package() {
                 0, // 0-disable, 1-enable GPIO, 2-enable IOEX(IO Expander)
                 0, // GPIO group #/IOEX #
                 0, // GPIO pad #/IOEX pin #
                 0  // power on polarity
               })
#endif
//[-end-161109-IB07400810-add]//
      Include("Rtd3BxtSata.asl")
    } // end device(PRT1)

    ///
    /// _DSM Device Specific Method supporting AHCI DEVSLP
    ///
    /// Arg0: UUID Unique function identifier \n
    /// Arg1: Integer Revision Level \n
    /// Arg2: Integer Function Index \n
    /// Arg3: Package Parameters \n
    ///
    /// Variables:
    Name(DRV, 0)  /// Storage for _DSM Arg3 parameter 0
    Name(PWR, 0)  /// Storage for _DSM Arg3 parameter 1

    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {

      /// Check UUID of _DSM defined by Microsoft
      If (LEqual(Arg0, ToUUID ("E4DB149B-FCFE-425b-A6D8-92357D78FC7F"))) {
        ///
        /// Switch by function index
        ///
        Switch (ToInteger(Arg2)) {
          Case (0) {  /// case 0:
            ///- Standard query - A bitmask of functions supported
            ///- Supports function 0-3
        	  If (LAnd(And(RCG0, 0x01), 0x01)) { 
        		  Return(0x03) // Functions supported when ZPODD RTD3 enabled
        	  }
        	  Else {
        		  Return(0x0F) // Functions supported when ZPODD RTD3 disabled
        	  }
          }
          Case (1) {  /// case 1:
            ///- Query Device IDs (Addresses) of children where drive power and/or DevSleep are supported.
            ///- SATA HBA provides autonomous link (DevSleep) support, return a package of 0 elements
            Return( Package(){})     ///- SATA HBA provides native DevSleep
          }
          Case (2) { /// Case 2: Control power to device.
            Store(ToInteger(DerefOf(Index(Arg3, Zero))), DRV)
            Store(ToInteger(DerefOf(Index(Arg3, One))), PWR)

            Switch(ToInteger(DRV)){
              Case (0x0000FFFF){
                If(PWR){ // Applying Power
                  \_SB.PCI0.SATA.PRT0.SPPR._ON() ///- apply power to port 0
                }
              }
            } //Switch(DRV)
            Return (0)
          } //Case (2)
          Case (3){ /// Case 3: Current status of Device/Link of Port
            Store(ToInteger(DerefOf(Index(Arg3, Zero))), DRV)
            Switch(ToInteger(DRV)){  /// Check for SATA port
              Case (0x0000FFFF) {  ///- SATA Port 0
                ///- Bit0 => Device power state
                Return (\_SB.PCI0.SATA.PRT0.SPPR._STA)
              }
              Case (0x0001FFFF){  ///- SATA Port 1
                ///- Bit0 => Device power state
                Return (\_SB.PCI0.SATA.PRT1.SPPR._STA)
              }
              Default { ///- Invalid SATA Port - error
                Return (Ones)
              }
            }
          }
          Default {
            Return (0)
          }
        }
      } Else { // UUID does not match
        Return (0)
      }
    } /// @defgroup sata_dsm SATA _DSM Method

  } //Scope(\_SB.PCI0.SATA)
//
// SATA - END
//

//
// USB - START
//
  Scope(\_SB.PCI0.XHC.RHUB) { //USB XHCI RHUB
    //
    // No specific power control (GPIO) to USB connectors (p2/p4/p5),
    // declare power resource with null functions and specify its wake ability
    // Please refer to ACPI 5.0 spec CH7.2
    //
    Scope (HS02) {
      Name(PORT, 2)
      Include ("Rtd3BxtUsb.asl")
    }
    Scope (SSP2) {
      Name(PORT, 10)
      Include ("Rtd3BxtUsb.asl")
    }

    Scope (HS04) {
      Name(PORT, 4)
      Include ("Rtd3BxtUsb.asl")
    }
    Scope (SSP4) {
      Name(PORT, 12)
      Include ("Rtd3BxtUsb.asl")
    }

    Scope (HS05) {
      Name(PORT, 5)
      Include ("Rtd3BxtUsb.asl")
    }
    Scope (SSP5) {
      Name(PORT, 13)
      Include ("Rtd3BxtUsb.asl")
    }
    
	///  
    /// WWAN RTD3 support, associate _PR0, PR2, PR3 for USB High speed Port 3    
    Scope (HS03) {
      Name(PORT, 3)
      Include ("Rtd3BxtUsbWwan.asl")
    }    
    
  } // RHUB
//
// USB - END
//
  Scope(\_SB){
    PowerResource(SDPR, 0, 0) {
      Name(_STA, One)
      Method(_ON, 0, Serialized) {
      }

      Method(_OFF, 0, Serialized) {
      }

    } //End of PowerResource(SDPR, 0, 0)
  } //End of Scope(\_SB)
//
// eMMC - START
//
   Scope(\_SB.PCI0.SDHA)
   {

     Name(_PR0, Package(){SDPR})   // TBD
     Name(_PR3, Package(){SDPR})   // TBD

//     Method (_DSW, 3, NotSerialized) {   // _DSW: Device Sleep Wake
//     }
   }  //Scope(\_SB.PCI0.SDHA)
//
// eMMC - END
//

//
// SDIO - START
//
   Scope(\_SB.PCI0.SDIO)
   {

     Name(_PR0, Package(){SDPR})   // TBD
     Name(_PR3, Package(){SDPR})   // TBD

//     Method (_DSW, 3, NotSerialized) {   // _DSW: Device Sleep Wake
//     }
   }  //Scope(\_SB.PCI0.SDIO)
//
// SDIO - END
//
//
// XDCI - start
//

    Scope(\_SB)
    {
      //
      // Dummy power resource for USB D3 cold support
      //
      PowerResource(USBC, 0, 0)
      {
        Method(_STA) { Return (0x1) }
        Method(_ON) {}
        Method(_OFF) {}
      }
    }

    Scope(\_SB.PCI0.XDCI)
    {
      Method (_RMV, 0, NotSerialized)  // _RMV: Removal Status
      {
        Return (Zero)
      }

      Method (_PR3, 0, NotSerialized)  // _PR3: Power Resources for D3hot
      {
        Return (Package (0x01)
        {
          USBC // return dummy package
        })
      }

      OperationRegion (PMCS, PCI_Config, 0x74, 0x4)
      Field (PMCS, WordAcc, NoLock, Preserve) {
        PMSR, 32,   // 0x74, PMCSR - Power Management Control and Status
      }

      Method (_PS0, 0, NotSerialized) { // _PS0: Power State 0
      }

      Method (_PS3, 0, NotSerialized) { // _PS3: Power State 3
        //
        // dummy read PMCSR
        //
        Store (PMSR, Local0)
		And (Local0, 1, Local0) // Dummy operation on Local0
      }
    } // Scope(\_SB.PCI0.XDCI)

//
// XDCI - end
//


//
// Serial IO - START
//
//
// Serial IO End
//
//Power Resource for Audio Codec
    Scope(\_SB.PCI0)
    {
      PowerResource(PAUD, 0, 0) {

        /// Namespace variable used:
        Name(PSTA, One) /// PSTA: Physical Power Status of Codec 0 - OFF; 1-ON
        Name(ONTM, Zero) /// ONTM: 0 - Not in Speculative ON ; Non-Zero - elapsed time in Nanosecs after Physical ON

        Name(_STA, One) /// _STA: PowerResource Logical Status 0 - OFF; 1-ON

        ///@defgroup pr_paud Power Resource for onboard Audio CODEC

        Method(_ON, 0){     /// _ON method \n
          Store(One, _STA)        ///- Set Logocal power state
          PUAM() ///- Call PUAM() to tansition Physical state to match current logical state
                    ///@addtogroup pr_paud
        } // End _ON

        Method(_OFF, 0){    /// _OFF method \n
          Store(Zero, _STA)    ///- Set the current power state
          PUAM() ///- Call PUAM() to tansition Physical state to match current logical state
        ///@addtogroup pr_paud
        } // End _OFF

        ///  PUAM - Power Resource User Absent Mode for onboard Audio CODEC
        ///  Arguments:
        ///
        ///  Uses:
        ///      _STA - Variable updated by Power Resource _ON/_OFF methods \n
        ///      \\UAMS - Variable updated by GUAM method to show User absent present \n
        ///      ONTM - Local variable to store ON time during Speculative ON \n
        /// ______________________________
        // |  Inputs      |   Outputs    |
        // ______________________________
        // | _STA | \UAMS | GPIO | ONTM |
        // ______________________________
        // |   1  |   0   | ON   |   !0 |
        // |   1  |   !0  | ON   |   !0 |
        // |   0  |   0   | ON   |   !0 |
        // |   0  |   !0  | OFF  |   0  |
        // ______________________________
                    /**
                    <table>
                    <tr> <th colspan="2"> Inputs <th colspan="2"> Output
                    <tr> <th>_STA <th> \\UAMS <th> GPIO <th>ONTM
                    <tr> <td>1 <td>0 <td>ON <td>!0
                    <tr> <td>1 <td>!0<td>ON <td>!0
                    <tr> <td>0 <td>0 <td>ON <td>!0
                    <tr> <td>0 <td>!0<td>OFF<td> 0
                    </table>
                    **/
        ///@addtogroup pr_paud_puam
        Method(PUAM, 0, Serialized)
        {
          // power rail = (IOEX2, 5)
//          If (LAnd(LEqual(^_STA, Zero), LNotEqual(\UAMS, Zero))) { ///New state = OFF Check if (_STA ==0 && \UAMS != 0) \n
         If (LEqual(^_STA, Zero)) {
//            // skip below if Power Resource is already in OFF \n
//            If(LEqual(\_SB.PCI0.GEXP.GEPS(1, 5), 1)) {
//              // drive pwr low
//              \_SB.PCI0.GEXP.SGEP(1, 5, 0)
//              Store(Zero, ^PSTA)  ///- >> Clear PSTA
//              Store(Zero, ^ONTM)  ///- >> Clear ONTM
//            }
          } Else { /// New state = ON (_STA=1) or (_STA=0 and \UAMS=0)
            /// Turn power on \n
//            If(LNotEqual(^PSTA, One)) { ///- Skip below if Power Resource is already in ON
//              // drive pwr high
//              \_SB.PCI0.GEXP.SGEP(1, 5, 1)
//              Store(One, ^PSTA)  ///- >> Set PSTA to 1
//              Store(Timer(), ^ONTM) ///- >> Start the timer for this PR
//            }
          }
        ///@defgroup pr_paud_puam Power Resource User Absent Mode for onboard Audio CODEC
        } //PUAM
      } //PAUD
    } //Scope(\_SB.PCI0)
//Power Resource for Audio Codec End

// I2C1 - TouchPanel Power control

//Serial IO End

//GPE Event handling - Start
//GPE Event handling - End

  // Modern Standby N:1 Power Resource definition. Place Holder.
  If (LEqual(\EMOD, 1)) {
    Scope(\_SB){
      Name(GBPS, 0) // Power state flag for Modern Standby. Initial value = 1 (On).
      PowerResource(MODS, 0, 0){                  /// Modern Standby Power Resource

        Method(_STA) {  /// _STA method
          If(LGreaterEqual(OSYS, 2015)){
            Return(GBPS)
          }
          Return(0)
        }

        Method(_ON, 0) {                            /// _ON Method
          If(LGreaterEqual(OSYS, 2015)){
/*
            // USB Camera

            If (And(RCG0, 0x02)) { // USB Camera RTD3 is enabled.
              If (LEqual(\_SB.PCI0.XHC.RHUB.CMSA(), 0)) {
                \_SB.PCI0.XHC.RHUB.CMON()
              }
            }
            \_SB.PCI0.I2C1.PON()
*/
            \_SB.PCI0.SATA.PRT0.SPON()
            Store(1, GBPS) // Indicate devices are ON
          }
        }

        Method(_OFF, 0) {                           /// _OFF Method
          If(LGreaterEqual(OSYS, 2015)){
/*
            // USB Camera
            If (And(RCG0, 0x02)) { // USB Camera RTD3 is enabled.
              If (LEqual(\_SB.PCI0.XHC.RHUB.CMSA(), 1)){
                \_SB.PCI0.XHC.RHUB.COFF()
              }
            }
            \_SB.PCI0.I2C1.POFF()
*/
            \_SB.PCI0.SATA.PRT0.SPOF()
            Store(0, GBPS)
          }
        }
      } // End MODS
    } // \_SB
  } // End If ((LEqual(EMOD, 1))

  Scope(\_SB.PCI0) {
    Method(LPD3, 0, Serialized) {
      //
      // Memory Region to access to the PCI Configuration Space
      // and dummy read PMCSR
      //
      OperationRegion (PMCS, PCI_Config, 0x84, 0x4)
      Field (PMCS, WordAcc, NoLock, Preserve) {
        PMSR, 32,   // 0x84, PMCSR - Power Management Control and Status
      }
	    //
	    // Dummy PMCSR read
	    //
      Store (PMSR, Local0)
	    And (Local0, 1, Local0) // Dummy operation on Local0
    }

    PowerResource (LSPR, 0, 0) {
      //
      // LPSS Power Resource
      //
      Name (_STA, One)
      Method (_ON, 0, Serialized) {
      }

      Method (_OFF, 0, Serialized) {
      }
    } //End of PowerResource(SDPR, 0, 0)
  }

  Scope(\_SB.PCI0.PWM)
  {
    //
    // Dummy Power Resource declaration
    //
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.PWM)

  Scope(\_SB.PCI0.I2C0)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C0)

  Scope(\_SB.PCI0.I2C1)
  {
    //
    // Dummy Power Resource declaration
    //
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C1)

  Scope(\_SB.PCI0.I2C2)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C2)

  Scope(\_SB.PCI0.I2C3)
  {
    //
    // Dummy Power Resource declaration
    //
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C3)

  Scope(\_SB.PCI0.I2C4)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C4)

  Scope(\_SB.PCI0.I2C5)
  {
    //
    // Dummy Power Resource declaration
    //
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C5)

  Scope(\_SB.PCI0.I2C6)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C6)

  Scope(\_SB.PCI0.I2C7)
  {
    //
    // Dummy Power Resource declaration
    //
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.I2C7)

  Scope(\_SB.PCI0.SPI1)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.SPI1)

  Scope(\_SB.PCI0.SPI2)
  {
    //
    // Dummy Power Resource declaration
    //
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.SPI2)

  Scope(\_SB.PCI0.SPI3)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.SPI3)

  Scope(\_SB.PCI0.URT1)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.URT1)

  Scope(\_SB.PCI0.URT2)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
	Method (_PS0) { }
    Method (_PS3) { LPD3 () }
  }  //Scope(\_SB.PCI0.URT2)

  Scope(\_SB.PCI0.URT3)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
//[-start-170410-IB07400857-modify]//
//#if BXTI_PF_ENABLE
    Method (_PS0) {
      If (LEqual(IOTP(),0x01)) { // IOTG CRB
        If (LEqual(\OSSL, 3)) {
          // Set pmode = 1 for N_GPIO_46 and N_GPIO_49
          Store (\_SB.GPC0 (N_GPIO_46), Local1)
          And (Local1, 0xF961E0FF, Local1)
          Or (Local1, 0x04000400, Local1) // TX-enabled, RX-enabled, Mode 1, RX drive 0 
          \_SB.SPC0 (N_GPIO_46, Local1)
          Store (\_SB.GPC0 (N_GPIO_49), Local1)
          And (Local1, 0xF961E0FF, Local1)
          Or (Local1, 0x04000400, Local1) // TX-enabled, RX-enabled, Mode 1, RX drive 0 
          \_SB.SPC0 (N_GPIO_49, Local1)
        }
      }
    }
    Method (_PS3) {
      If (LEqual(IOTP(),0x01)) { // IOTG CRB
        If (LEqual(\OSSL, 3)) {
          LPD3 ()
          // Set pmode = 0 for N_GPIO_46 and N_GPIO_49
          Store (\_SB.GPC0 (N_GPIO_46), Local1)
          And (Local1, 0xF961E0FF, Local1)
          Or (Local1, 0x02900100, Local1)  // TX-disabled, RX-enabled, Mode 0, GPI-Route to APIC, RX Invert, Level 
          \_SB.SPC0 (N_GPIO_46, Local1)
          Store (\_SB.GPC0 (N_GPIO_49), Local1)
          And (Local1, 0xF961E0FF, Local1)
          Or (Local1, 0x02900100, Local1)  // TX-disabled, RX-enabled, Mode 0, GPI-Route to APIC, RX Invert, Level 
          \_SB.SPC0 (N_GPIO_49, Local1)
        }
      } Else {
        LPD3 ()
      }
    }
//#else
//    Method (_PS0) { }
//    Method (_PS3) {LPD3 ()}
//#endif
//[-end-170410-IB07400857-modify]//
  }  //Scope(\_SB.PCI0.URT3)

  Scope(\_SB.PCI0.URT4)
  {
    Name(_PR0, Package(){LSPR})  // TBD
    Name(_PR3, Package(){LSPR})  // TBD
    Method (_PS0) { }
    Method (_PS3) {LPD3 ()}
  }  //Scope(\_SB.PCI0.URT4)

} // End SSDT
