/** 
;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _VBT_HOOK_SETUP_CONFIG_H_
#define _VBT_HOOK_SETUP_CONFIG_H_

#define SCU_VBT_DEFAULT                       0x00  // GOP/VBIOS Default
#define SCU_VBT_DP                            0x02  // Disaply Port
#define SCU_VBT_HDMI_DVI                      0x03  // HDMI/DVI
#define SCU_VBT_DVI_ONLY                      0x04  // DVI only
#define SCU_VBT_DP_HDMI_DVI                   0x05  // DP with HDMI/DVI Compatible
#define SCU_VBT_DP_DVI                        0x06  // DP with DVI Compatible
#define SCU_VBT_NO_DEVICE                     0xFF  // No Device

#define SCU_VBT_LFP_EDP                       0x01  // eDP
#define SCU_VBT_LFP_MIPI                      0x02  // MIPI
#define SCU_VBT_LFP_EDP_PORTA                 0x01  // eDP-PortA
#define SCU_VBT_LFP_MIPI_PORTA                0x02  // MIPI-PortA

#define SCU_VBT_DP_PORTA                      0x01  // DP Port A
#define SCU_VBT_DP_PORTB                      0x02  // DP Port B
#define SCU_VBT_DP_PORTC                      0x03  // DP Port C
#define SCU_VBT_DP_PORTD                      0x04  // DP Port D
#define SCU_VBT_HDMI_PORTB                    0x05  // HDMI Port B
#define SCU_VBT_HDMI_PORTC                    0x06  // HDMI Port B

#define SCU_VBT_ENABLE                        0x01  // Enable
#define SCU_VBT_DISABLE                       0x02  // Disable

//[-start-160913-IB07400784-add]//
#define SCU_VBT_400MV_0P0_DB                  0x01
#define SCU_VBT_400MV_3P5_DB                  0x02
#define SCU_VBT_400MV_6P0_DB                  0x03
#define SCU_VBT_400MV_9P5_DB                  0x04
#define SCU_VBT_600MV_0P0_DB                  0x05
#define SCU_VBT_600MV_3P5_DB                  0x06
#define SCU_VBT_600MV_6P0_DB                  0x07
#define SCU_VBT_800MV_0P0_DB                  0x08
#define SCU_VBT_800MV_3P5_DB                  0x09
#define SCU_VBT_1200MV_0P0_DB                 0x0A
//[-end-160913-IB07400784-add]//

#endif
