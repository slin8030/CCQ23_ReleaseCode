//;******************************************************************************
//;* Copyright (c) 1983-2016, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//;
//; Module Name :
//;
//;   VBTRegs.h
//;

#ifndef _VBT_REGS_H_
#define _VBT_REGS_H_

#include "Numbers.h"
#include "GenVbiosVbtHeader.h"

#define MAX_EFP_NUMBER                                            0x3 // EFP 1 ~ 3

//
// VBT Offset
//
#define R_VBT_CHECKSUM                                            0x001A
#define B_VBT_CHECKSUM                                            0xFF
#define N_VBT_CHECKSUM                                            0

#define R_VBT_LFP1_ACTIVE_CONFIG                                  INT_LFP_TYPE
#define B_VBT_LFP_ACTIVE_CONFIG                                   0xFFFF
#define N_VBT_LFP_ACTIVE_CONFIG                                   0x0000
#define V_VBT_LFP_ACTIVE_CONFIG_NONE                              0x0000
#define V_VBT_LFP_ACTIVE_CONFIG_eDP                               0x1806
#define V_VBT_LFP_ACTIVE_CONFIG_MIPI                              0x1400 // Not exist

#define R_VBT_LFP1_OUTPUT_PORT                                    INT_EDP_PORT
#define B_VBT_LFP_OUTPUT_PORT                                     0xFF
#define N_VBT_LFP_OUTPUT_PORT                                     0x00
#define V_VBT_LFP_OUTPUT_PORT_EDP_NONE                            0x00
#define V_VBT_LFP_OUTPUT_PORT_EDP_PORTA                           0x0A
#define V_VBT_LFP_OUTPUT_PORT_MIPI_PORTA                          0x15 // Not exist

#define R_VBT_LFP1_AUX                                            INT_LFP_AUX_CHANNEL
#define B_VBT_LFP_AUX                                             0xFF
#define N_VBT_LFP_AUX                                             0x00
#define V_VBT_LFP_AUX_NONE                                        0x00
#define V_VBT_LFP_AUX_EDPA                                        0x40

//[-start-161013-IB07400797-modify]//
#define R_VBT_LFP_HPD_INVERSION                                   HPD0_INVERSION_ENABLE
//[-end-161013-IB07400797-modify]//
#define B_VBT_LFP_HPD_INVERSION                                   (BIT4)
#define N_VBT_LFP_HPD_INVERSION                                   0x00
#define V_VBT_LFP_HPD_INVERSION_DISABLED                          0x00
#define V_VBT_LFP_HPD_INVERSION_ENABLED                           (BIT4)

#define R_VBT_LFP_DDI_LANE_REVERSAL                               LFP_LANE_REVERSAL
#define B_VBT_LFP_DDI_LANE_REVERSAL                               (BIT1)
#define N_VBT_LFP_DDI_LANE_REVERSAL                               0x00
#define V_VBT_LFP_DDI_LANE_REVERSAL_DISABLED                      0x00
#define V_VBT_LFP_DDI_LANE_REVERSAL_ENABLED                       (BIT1)

#define R_VBT_LFP1_PANEL_TYPE                                     BMP_PANEL_TYPE
#define B_VBT_LFP_PANEL_TYPE                                      0xFF
#define N_VBT_LFP_PANEL_TYPE                                      0x00

#define R_VBT_LFP1_EDID                                           BMP_PANEL_EDID
#define B_VBT_LFP_EDID                                            (BIT6)
#define N_VBT_LFP_EDID                                            0x00
#define V_VBT_LFP_EDID_DISABLED                                   0x00
#define V_VBT_LFP_EDID_ENABLED                                    0x40

#define R_VBT_LFP_180D_ROTATION                                   _180_DEG_ROTATION_ENABLE
#define B_VBT_LFP_180D_ROTATION                                   (BIT2)
#define N_VBT_LFP_180D_ROTATION                                   0x00
#define V_VBT_LFP_180D_ROTATION_DISABLED                          0x00
#define V_VBT_LFP_180D_ROTATION_ENABLED                           0x04

#define R_VBT_EFP1_DEVICE_TYPE                                    INT_EFP1_TYPE
#define R_VBT_EFP2_DEVICE_TYPE                                    INT_EFP2_TYPE
#define R_VBT_EFP3_DEVICE_TYPE                                    INT_EFP3_TYPE
#define B_VBT_EFP_DEVICE_TYPE                                     0xFFFF
#define N_VBT_EFP_DEVICE_TYPE                                     0x0000
#define V_VBT_EFP_DEVICE_TYPE_NONE                                0x0000
#define V_VBT_EFP_DEVICE_TYPE_DP                                  0x68C6
#define V_VBT_EFP_DEVICE_TYPE_HDMI_DVI                            0x60D2
#define V_VBT_EFP_DEVICE_TYPE_DVI_ONLY                            0x68D2
#define V_VBT_EFP_DEVICE_TYPE_DP_HDMI_DVI                         0x60D6
#define V_VBT_EFP_DEVICE_TYPE_DP_DVI                              0x68D6

#define GetEfpDevTypeOffset(Num)                                  (R_VBT_EFP1_DEVICE_TYPE + Num * (R_VBT_EFP2_DEVICE_TYPE - R_VBT_EFP1_DEVICE_TYPE))

#define R_VBT_EFP1_OUTPUT_PORT                                    INT_EFP1_PORT
#define R_VBT_EFP2_OUTPUT_PORT                                    INT_EFP2_PORT
#define R_VBT_EFP3_OUTPUT_PORT                                    INT_EFP3_PORT
#define B_VBT_EFP_OUTPUT_PORT                                     0xFF
#define N_VBT_EFP_OUTPUT_PORT                                     0x00
#define V_VBT_EFP_OUTPUT_PORT_NONE                                0x00
#define V_VBT_EFP_OUTPUT_PORT_HDMI_PORTB                          0x01
#define V_VBT_EFP_OUTPUT_PORT_HDMI_PORTC                          0x02
#define V_VBT_EFP_OUTPUT_PORT_HDMI_PORTD                          0x03
#define V_VBT_EFP_OUTPUT_PORT_DP_PORTA                            0x0A
#define V_VBT_EFP_OUTPUT_PORT_DP_PORTB                            0x07
#define V_VBT_EFP_OUTPUT_PORT_DP_PORTC                            0x08
#define V_VBT_EFP_OUTPUT_PORT_DP_PORTD                            0x09
#define V_VBT_EFP_OUTPUT_PORT_DP_PORTE                            0x0B

#define GetEfpOutputPortOffset(Num)                               (R_VBT_EFP1_OUTPUT_PORT + Num * (R_VBT_EFP2_OUTPUT_PORT - R_VBT_EFP1_OUTPUT_PORT))

#define R_VBT_EFP1_DDC_CONFIG                                     INT_EFP1_DDC_PIN
#define R_VBT_EFP2_DDC_CONFIG                                     INT_EFP2_DDC_PIN
#define R_VBT_EFP3_DDC_CONFIG                                     INT_EFP3_DDC_PIN
#define B_VBT_EFP_DDC_CONFIG                                      0xFF
#define N_VBT_EFP_DDC_CONFIG                                      0x00
#define V_VBT_EFP_DDC_CONFIG_NONE                                 0x00
#define V_VBT_EFP_DDC_CONFIG_HDMI_B                               0x01
#define V_VBT_EFP_DDC_CONFIG_HDMI_C                               0x02

#define GetEfpDdcConfigOffset(Num)                                (R_VBT_EFP1_DDC_CONFIG + Num * (R_VBT_EFP2_DDC_CONFIG - R_VBT_EFP1_DDC_CONFIG))

#define R_VBT_EFP1_AUX                                            INT_EFP1_AUX_CHANNEL
#define R_VBT_EFP2_AUX                                            INT_EFP2_AUX_CHANNEL
#define R_VBT_EFP3_AUX                                            INT_EFP3_AUX_CHANNEL
#define B_VBT_EFP_AUX                                             0xFF
#define N_VBT_EFP_AUX                                             0x00
#define V_VBT_EFP_AUX_NONE                                        0x00
#define V_VBT_EFP_AUX_DPA                                         0x40
#define V_VBT_EFP_AUX_DPB                                         0x10
#define V_VBT_EFP_AUX_DPC                                         0x20
#define V_VBT_EFP_AUX_DPD                                         0x30

#define GetEfpAuxConfigOffset(Num)                                (R_VBT_EFP1_AUX + Num * (R_VBT_EFP2_AUX - R_VBT_EFP1_AUX))

#define R_VBT_EFP1_HDMI_LS                                        INT_EFP1_HDMI_LS_TYPE
#define R_VBT_EFP2_HDMI_LS                                        INT_EFP2_HDMI_LS_TYPE
#define R_VBT_EFP3_HDMI_LS                                        INT_EFP3_HDMI_LS_TYPE
#define B_VBT_EFP_HDMI_LS                                         0x1F
#define N_VBT_EFP_HDMI_LS                                         0x00
#define V_VBT_EFP_HDMI_LS_0P4V_0P0DB                              0x00
#define V_VBT_EFP_HDMI_LS_0P4V_3P5DB                              0x01
#define V_VBT_EFP_HDMI_LS_0P4V_6P0DB                              0x02
#define V_VBT_EFP_HDMI_LS_0P4V_9P5DB                              0x03
#define V_VBT_EFP_HDMI_LS_0P6V_0P0DB                              0x04
#define V_VBT_EFP_HDMI_LS_0P6V_3P5DB                              0x05
#define V_VBT_EFP_HDMI_LS_0P6V_6P0DB                              0x06
#define V_VBT_EFP_HDMI_LS_0P8V_0P0DB                              0x07
#define V_VBT_EFP_HDMI_LS_0P8V_3P5DB                              0x08
#define V_VBT_EFP_HDMI_LS_1P2V_0P0DB                              0x09

#define GetEfpHdmiLsConfigOffset(Num)                             (R_VBT_EFP1_HDMI_LS + Num * (R_VBT_EFP2_HDMI_LS - R_VBT_EFP1_HDMI_LS))

#define R_VBT_EFP1_LSPCON                                         LSPCON1_OPTIONS
#define R_VBT_EFP2_LSPCON                                         LSPCON2_OPTIONS
#define R_VBT_EFP3_LSPCON                                         EFP3_LSPCON_OPTIONS
#define B_VBT_EFP_LSPCON                                          (BIT2)
#define N_VBT_EFP_LSPCON                                          0x00
#define V_VBT_EFP_LSPCON_DISABLED                                 0x00
#define V_VBT_EFP_LSPCON_ENABLED                                  (BIT2)

#define GetEfpLspconConfigOffset(Num)                             (R_VBT_EFP1_LSPCON + Num * (R_VBT_EFP2_LSPCON - R_VBT_EFP1_LSPCON))

//[-start-160914-IB07400784-modify]//
#define R_VBT_EFP1_HPD_INVERSION                                  HPD1_INVERSION_ENABLE
#define R_VBT_EFP2_HPD_INVERSION                                  HPD2_INVERSION_ENABLE
#define R_VBT_EFP3_HPD_INVERSION                                  HPD3_INVERSION_ENABLE
//[-end-160914-IB07400784-modify]//
#define B_VBT_EFP_HPD_INVERSION                                   (BIT4)
#define N_VBT_EFP_HPD_INVERSION                                   0x00
#define V_VBT_EFP_HPD_INVERSION_DISABLED                          0x00
#define V_VBT_EFP_HPD_INVERSION_ENABLED                           (BIT4)

#define GetEfpHpdInversionConfigOffset(Num)                       (R_VBT_EFP1_HPD_INVERSION + Num * (R_VBT_EFP2_HPD_INVERSION - R_VBT_EFP1_HPD_INVERSION))

#define R_VBT_EFP1_DDI_LANE_REVERSAL                              INT_EFP1_LANE_REVERSAL
#define R_VBT_EFP2_DDI_LANE_REVERSAL                              INT_EFP2_LANE_REVERSAL
#define R_VBT_EFP3_DDI_LANE_REVERSAL                              INT_EFP3_LANE_REVERSAL
#define B_VBT_EFP_DDI_LANE_REVERSAL                               (BIT1)
#define N_VBT_EFP_DDI_LANE_REVERSAL                               0x00
#define V_VBT_EFP_DDI_LANE_REVERSAL_DISABLED                      0x00
#define V_VBT_EFP_DDI_LANE_REVERSAL_ENABLED                       (BIT1)

#define GetEfpDdiLaneReversalConfigOffset(Num)                    (R_VBT_EFP1_DDI_LANE_REVERSAL + Num * (R_VBT_EFP2_DDI_LANE_REVERSAL - R_VBT_EFP1_DDI_LANE_REVERSAL))

#define R_VBT_EFP1_DOCK                                           INT_EFP1_PORT_DOCKABLE
#define R_VBT_EFP2_DOCK                                           INT_EFP2_PORT_DOCKABLE
#define R_VBT_EFP3_DOCK                                           INT_EFP3_PORT_DOCKABLE
#define B_VBT_EFP_DOCK                                            (BIT0)
#define N_VBT_EFP_DOCK                                            0x00
#define V_VBT_LFP_DOCK_DISABLED                                   0x00
#define V_VBT_LFP_DOCK_ENABLED                                    (BIT0)

#define GetEfpDockConfigOffset(Num)                               (R_VBT_EFP1_DOCK + Num * (R_VBT_EFP2_DOCK - R_VBT_EFP1_DOCK))

#define R_VBT_EFP1_USB_TYPEC                                      EFP1_USB_C_DONGLEFEATURE_ENABLED
#define R_VBT_EFP2_USB_TYPEC                                      EFP2_USB_C_DONGLEFEATURE_ENABLED
#define R_VBT_EFP3_USB_TYPEC                                      EFP3_USB_C_DONGLEFEATURE_ENABLED
#define B_VBT_EFP_USB_TYPEC                                       0xFF
#define N_VBT_EFP_USB_TYPEC                                       0x00
#define V_VBT_LFP_USB_TYPEC_DISABLED                              0x00
#define V_VBT_LFP_USB_TYPEC_ENABLED                               0x01

#define GetEfpUsbTypeCConfigOffset(Num)                           (R_VBT_EFP1_USB_TYPEC + Num * (R_VBT_EFP2_USB_TYPEC - R_VBT_EFP1_USB_TYPEC))

#define R_VBT_PANEL1_WIDTH                                        PANEL_WIDTH_01
#define R_VBT_PANEL2_WIDTH                                        PANEL_WIDTH_02
#define R_VBT_PANEL3_WIDTH                                        PANEL_WIDTH_03
#define B_VBT_PANEL_WIDTH                                         0xFFFF
#define N_VBT_PANEL_WIDTH                                         0x0000

#define R_VBT_PANEL1_HEIGHT                                       PANEL_HEIGHT_01
#define R_VBT_PANEL2_HEIGHT                                       PANEL_HEIGHT_02
#define R_VBT_PANEL3_HEIGHT                                       PANEL_HEIGHT_03
#define B_VBT_PANEL_HEIGHT                                        0xFFFF
#define N_VBT_PANEL_HEIGHT                                        0x0000

#define R_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PANEL1              BLC_INV_TYPE_01
#define R_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PANEL2              BLC_INV_TYPE_02
#define R_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PANEL3              BLC_INV_TYPE_03
#define B_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE                     (BIT1 | BIT0)
#define N_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE                     0
#define V_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_NONE                0
#define V_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_EXT                 0
#define V_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PWM                 2

#define R_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY_PANEL1          BLC_INV_POLARITY_01
#define R_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY_PANEL2          BLC_INV_POLARITY_02
#define R_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY_PANEL3          BLC_INV_POLARITY_03
#define B_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY                 (BIT2)
#define N_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY                 2
#define V_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY_NORMAL          0
#define V_VBT_BACKLIGHT_CONTROL_INVERTER_POLARITY_INVERTED        (BIT2)

#define R_VBT_PANEL1_POST_BL_LEVEL                                POST_BL_BRIGHTNESS_01
#define R_VBT_PANEL2_POST_BL_LEVEL                                POST_BL_BRIGHTNESS_02
#define R_VBT_PANEL3_POST_BL_LEVEL                                POST_BL_BRIGHTNESS_03
#define B_VBT_PANEL_POST_BL_LEVEL                                 0xFF
#define N_VBT_PANEL_POST_BL_LEVEL                                 0x00

#define GetWidthOffset(Panel)                                     (R_VBT_PANEL1_WIDTH + Panel * (R_VBT_PANEL2_WIDTH - R_VBT_PANEL1_WIDTH))
#define GetHeightOffset(Panel)                                    (GetWidthOffset (Panel) + 0x02)
#define GetBlcOffset(Panel)                                       (R_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PANEL1 + Panel * (R_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PANEL2 - R_VBT_BACKLIGHT_CONTROL_INVERTER_TYPE_PANEL1))
#define GetBlLevelOffset(Panel)                                   (R_VBT_PANEL1_POST_BL_LEVEL + Panel)

#endif
