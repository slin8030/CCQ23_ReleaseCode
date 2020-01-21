/*
 * (C) Copyright 2011-2012 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _COMPAL_THERMAL_LIB_H_
#define _COMPAL_THERMAL_LIB_H_

//****************************************************************************************
//*                            Common Parameter
//****************************************************************************************
// Vendor ID Definition
#define INTEL_VID       0x8086
#define AMD_VID         0x1022
#define ATI_VID         0x1002
#define NVIDIA_VID      0x10DE
#define VIA_VID         0x1106

// Common Parameters
#define ENABLED         1
#define DISABLED        0
#define DOS             0
#define WINDOWS         2


// If "COMPAL_COMMON_PROCHOT_LINK_TO_EC = NO"
// PROCHOT# Trigger Temp = Tjmax - OemSetting
// Ex: Tjmax = 100, OemSetting = 5, PROCHOT# Trigger Temp = 95 degree C
#define OemSetting      5


//****************************************************************************************
//*                            EC Parameter
//****************************************************************************************
// EC Name Space Offset Definition for Store CPU Max P-State
// EC Name Space Offset 0xAF, BIT[7:3]
#define EC_NAME_CPU_MAX_PSTATE_OFFSET           0xAF
#define EC_NAME_CPU_MAX_PSTATE_MASK_BIT         0xF8
#define EC_NAME_CPU_MAX_PSTATE_SHIFT            0x03

// EC Name Space Offset Definition for Store cTDP Flag
// EC Name Space Offset 0xAF, BIT[3]
#define EC_NAME_CTDP_FLAG_OFFSET                0xAF
#define EC_NAME_CTDP_FLAG_MASK_BIT              0x04
#define EC_NAME_CTDP_FLAG_SHIFT                 0x02


//****************************************************************************************
//*                            Intel Parameter
//****************************************************************************************
// Intel MSR Definition
#define PLATFORM_INFORMATION                    0x000000CE
#define MAXIMUM_NON_TURBO_RATIO                 8
#define NUMBER_OF_CTDP_LEVELS_OFFSET            33
#define NUMBER_OF_CTDP_LEVELS_MASK              0x03
#define MAXIMUM_EFFICIENCY_RATIO                40
#define LIMIT_MAX_PSTATE                        16
#define PROGRAMMABLE_TCC_ACTIVATION_OFFSET      (1 << 30)

#define POWER_CTL                               0x000001FC
#define BI_DIRECTIONAL_ENABLE_OFFSET            0
#define BI_DIRECTIONAL_ENABLE_MASK              0x01

#define IA32_MISC_ENABLES                       0x1A0
#define TURBO_MODE_DISABLE                      0x4000000000

#define IA32_TEMPERATURE_TARGET                 0x1A2
#define TCC_ACTIVATION_TEMPERATURE              16
#define TCC_OFFSET                              24
#define TCC_OFFSET_WIDTH_MASK                   0x0F

// Intel CPUID Definition
#define FEATURE_INFORMATION                     0x01
#define SANDY_BRIDGE                            0x206A3
#define IVY_BRIDGE                              0x306A2

#define POWER_MANAGEMENT_PARAMETERS             0x06
#define TURBO_MODE_SUPPORTED                    (1 << 1)

VOID
CompalSendCpuMaxPstateToEC (
  UINT8         MaxPstate
);

VOID
CompalIntelProchotBiDirectional (
  UINT8         Setting
);

UINT8
CompalGetIntelCpuMaxPstate (
);

VOID
CompalDetectIntelConfigTDPSupported (
);

VOID
CompalSetIntelTccTemp (
);

#endif