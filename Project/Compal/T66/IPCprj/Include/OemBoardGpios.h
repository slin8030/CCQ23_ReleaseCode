/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _OEM_BOARD_GPIOS_H_
#define _OEM_BOARD_GPIOS_H_


#ifndef USE_CRB_HW_CONFIG

//
// GPIO Setting for Apollo Lake OEM platform, 
// Please modify for OEM platform, default is Apollo Lake Oxbow Hill CRB GPIO Table. 
//
//[-start-161207-IB07400824-modify]//
#ifdef GPIO_INIT_IN_PEI_PHASE_POST_MEMORY
//=======================================================//
//                                                       //
// PEI phase Post-Memory GPIO init                       //
//                                                       //
//=======================================================//

// North Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_N_OEM[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,     MMIO_Offset  ,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0000,  NORTH), // J4C1.13 PTI_0_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0008,  NORTH), // J4C1.19 PTI_0_DATA0
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0010,  NORTH), // J4C1.21 PTI_0_DATA1
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0018,  NORTH), // J4C1.23 PTI_0_DATA2
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0020,  NORTH), // J4C1.25 PTI_0_DATA3
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0028,  NORTH), // J4C1.27 PTI_0_DATA4
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0030,  NORTH), // J4C1.29 PTI_0_DATA5
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0038,  NORTH), // J4C1.31 PTI_0_DATA6
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0040,  NORTH), // J4C1.33 PTI_0_DATA7
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0048,  NORTH), //*J6M1.06 MIPIA_DSI_RST_1_8V & J6M1.29 MIPIA_DSI_RST_3.3V
//BXT_GPIO_PAD_CONF(L"GPIO_10",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Enabled , P_20K_L, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0050,  NORTH), //*J2P1.56 NGFF_3P3_KILL_WIFI_N
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0058,  NORTH), //*J1B1.12 EDP_TCH_PNL_RST
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Enabled , P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0060,  NORTH), //*J2M1.44 GPIO12
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0068,  NORTH), //*J2P1.62 NGFF_R_ALERT NGFF_3P3_ALERT 
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0070,  NORTH), //*J6M2.06 MIPIC_DSI_RST_1_8V & J6M2.29 MIPIC_DSI_RST_3.3V
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA     ,NA   ,     NA, GPIO_PADBAR+0x0078,  NORTH), // NC, WIFI_PCIE_RST_GPIO WIFI_PCIE_RST_3P3_GPIO
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_H, Inverted,IOAPIC,  HizRx0I ,DisPuPd, GPIO_PADBAR+0x0080,  NORTH), //#J6A1.DS SIM_DETECT Feature:SIM Card Detect
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0088,  NORTH), //*J1B1.11 EDP_TCH_PNL_INT
  BXT_GPIO_PAD_CONF(L"GPIO_18",                  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0090,  NORTH), // J4C1.14 PTI_1_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_19",                  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0098,  NORTH), // J4C1.18 PTI_1_DATA0
  BXT_GPIO_PAD_CONF(L"GPIO_20",                  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00A0,  NORTH), //*SOC_IERR
  BXT_GPIO_PAD_CONF(L"GPIO_21",                  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00A8,  NORTH), //*J6E3.2 TACHO_FAN
  /* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_22",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled,  P_NONE,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B0,  NORTH), // WG1210AT.28 LAN_DEV_OFF_N
  */
  BXT_GPIO_PAD_CONF(L"GPIO_23",                  M0   ,    GPO   , NA   ,   HI    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B8,  NORTH), // U5B1.4&5 USB_PWR_EN, Feature: LB USB Power in LFH
  BXT_GPIO_PAD_CONF(L"GPIO_24",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00C0,  NORTH), //*J5C1.11 NFC_IRQ_CONN
  BXT_GPIO_PAD_CONF(L"GPIO_25",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00C8,  NORTH), //#J2M1.38 SATA_R_DEVSLP1 M.2
  BXT_GPIO_PAD_CONF(L"GPIO_26",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D0,  NORTH), // J6E4.03 SATA_LEDN
  BXT_GPIO_PAD_CONF(L"GPIO_27",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D8,  NORTH), // J5C1.05 NFC_DFU Feature:DFU
  BXT_GPIO_PAD_CONF(L"GPIO_28",                  M2   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00E0,  NORTH), //*J4C1.44 PTI_2_DATA0 & J6C2.06 PTI_2_DATA0_R
  BXT_GPIO_PAD_CONF(L"GPIO_29",                  M2   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00E8,  NORTH), //*J4C1.46 PTI_2_DATA1 & J6C2.08 PTI_2_DATA1_R
  BXT_GPIO_PAD_CONF(L"GPIO_30",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00F0,  NORTH), //*J4C1.52 PTI_2_DATA2 & J6C2.10 PTI_2_DATA2_R
  BXT_GPIO_PAD_CONF(L"GPIO_31",                  M5   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00F8,  NORTH), //*J5C1.06 NFC_RESET_CONN NFC_RESET_N
  BXT_GPIO_PAD_CONF(L"GPIO_32",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0100,  NORTH), //*J2M1.26 NGFF_KEY_B_W_DISABLE
  BXT_GPIO_PAD_CONF(L"GPIO_33",                  M5   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0108,  NORTH), //*J2P1.21 NGFF_SDIO_WAKE_N
  BXT_GPIO_PAD_CONF(L"GPIO_34 PWM0",             M1   ,    NA   ,  NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0110,  NORTH), // J6C2.09 SOC_PWM0, Pull 4.22K Low
  BXT_GPIO_PAD_CONF(L"GPIO_35 PWM1",             M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled,  P_5K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0118,  NORTH),  //*J5D1.17 SPI_TPM_IRQ_N
  BXT_GPIO_PAD_CONF(L"GPIO_36 PWM2",             M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0120,  NORTH), // J6C2.11 SOC_PWM2, Pull 4.22K Low
  BXT_GPIO_PAD_CONF(L"GPIO_37 PWM3",             M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0128,  NORTH), // J6E3.01 PWM_FAN
  BXT_GPIO_PAD_CONF(L"GPIO_38 LPSS_UART0_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0130,  NORTH), // J2P1.22 NGFF_UART_RX
  BXT_GPIO_PAD_CONF(L"GPIO_39 LPSS_UART0_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0138,  NORTH), // J2P1.32 NGFF_UART_TX
  BXT_GPIO_PAD_CONF(L"GPIO_40 LPSS_UART0_RTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0140,  NORTH), // J2P1.36 NGFF_UART_RTS SOC_COM0_RTS_N
  BXT_GPIO_PAD_CONF(L"GPIO_41 LPSS_UART0_CTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0148,  NORTH), // J2P1.34 NGFF_UART_CTS SOC_COM0_CTS_N
  BXT_GPIO_PAD_CONF(L"GPIO_42 LPSS_UART1_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0150,  NORTH), // J6B2.08 SOC_COM1_RXD
  BXT_GPIO_PAD_CONF(L"GPIO_43 LPSS_UART1_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx0I,DisPuPd, GPIO_PADBAR+0x0158,  NORTH), // J6B2.07 SOC_COM1_TXD, Pull 4.22k to 1.8V High
  BXT_GPIO_PAD_CONF(L"GPIO_44 LPSS_UART1_RTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0160,  NORTH), // J6B2.09 SOC_COM1_RTS_N, Pull 4.22K to 1.8V High
  BXT_GPIO_PAD_CONF(L"GPIO_45 LPSS_UART1_CTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,   HizRx0I,DisPuPd, GPIO_PADBAR+0x0168,  NORTH), // J6B2.10 SOC_COM1_CTS_N
  /* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0170,  NORTH), // U6F1.24 FT2232D DEBUG_UART2_TXD SOC_COM2_RXD
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0178,  NORTH), // U6F1.23 FT2232D DEBUG_UART2_RXD SOC_COM2_TXD
  */
  BXT_GPIO_PAD_CONF(L"GPIO_48 LPSS_UART2_RTS_B", M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0180,  NORTH), // NC, SOC_COM2_RTS_N
  BXT_GPIO_PAD_CONF(L"GPIO_49 LPSS_UART2_CTS_B", M1   ,    NA   ,  NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0188,  NORTH), // NC, SOC_COM2_CTS_N
  BXT_GPIO_PAD_CONF(L"GPIO_62 GP_CAMERASB00",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x0190,  NORTH), //*J3M2.01 MCSI1_STROBE
  BXT_GPIO_PAD_CONF(L"GPIO_63 GP_CAMERASB01",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x0198,  NORTH), //*J3M2.21 GP_CAMERASB01
//BXT_GPIO_PAD_CONF(L"GPIO_64 GP_CAMERASB02",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01A0,  NORTH), //*****J3M2.19 & J3M2.09 MCSI1_XSHUTDN, Pull 0k to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_65 GP_CAMERASB03",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01A8,  NORTH), //*J3M2.23 GP_CAMERASB03
  BXT_GPIO_PAD_CONF(L"GPIO_66 GP_CAMERASB04",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01B0,  NORTH), //*J3M1.21 GP_CAMERASB04
//BXT_GPIO_PAD_CONF(L"GPIO_67 GP_CAMERASB05",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01B8,  NORTH), //*****J3M1.19 MCSI2_XSHUTDN & J3M1.09 MCSI2_XSHUTDN, Pull 0k to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_68 GP_CAMERASB06",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01C0,  NORTH), //*J3M1.23 GP_CAMERASB06
  BXT_GPIO_PAD_CONF(L"GPIO_69 GP_CAMERASB07",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01C8,  NORTH), //*J3M1.03 MCSI2_STROBE
  BXT_GPIO_PAD_CONF(L"GPIO_70 GP_CAMERASB08",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01D0,  NORTH), // NC
  BXT_GPIO_PAD_CONF(L"GPIO_71 GP_CAMERASB09",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01D8,  NORTH), //*J3M2.17 MCSI1_PWDNB, Pull 10K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_72 GP_CAMERASB10",    M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled,  P_NONE,   NA    ,    NA,IOS_Masked,     NA, GPIO_PADBAR+0x01E0,  NORTH), //#BOARD_ID3, Pull 10K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_73 GP_CAMERASB11",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01E8,  NORTH), //*J3M1.17 MCSI2_PWDNB, Pull 10K to 1.8V
  BXT_GPIO_PAD_CONF(L"TCK",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x01F0,  NORTH), // J4C1.03 TCK
  BXT_GPIO_PAD_CONF(L"TRST_B",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x01F8,  NORTH), // J4C1.09 TRST_N
  BXT_GPIO_PAD_CONF(L"TMS",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0200,  NORTH), // J4C1.02 TMS
  BXT_GPIO_PAD_CONF(L"TDI",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0208,  NORTH), // J4C1.05 TDI
  BXT_GPIO_PAD_CONF(L"CX_PMODE",                 M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0210,  NORTH), // NC
  BXT_GPIO_PAD_CONF(L"CX_PREQ_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0218,  NORTH), // J4C1.10 PREQ_N
  BXT_GPIO_PAD_CONF(L"JTAGX",                    M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0220,  NORTH), // NC
  BXT_GPIO_PAD_CONF(L"CX_PRDY_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0228,  NORTH), // J4C1.11 PRDY_N
  BXT_GPIO_PAD_CONF(L"TDO",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0230,  NORTH), // J4C1.04 TDO
  BXT_GPIO_PAD_CONF(L"GPIO_216 CNV_BRI_DT",      M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0238,  NORTH), //!NC, SOC_GPIO216, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_217 CNV_BRI_RSP",     M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0240,  NORTH), //!NC, SOC_GPIO217, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_218 CNV_RGI_DT",      M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_1K_H ,   NA    ,    NA,NA        ,     NA, GPIO_PADBAR+0x0248,  NORTH), //!NC, SOC_GPIO218, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_219 CNV_RGI_RSP",     M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0250,  NORTH), // U2T1.K5 KLMCG8GEND-B031 EMMC0_RST_N
  BXT_GPIO_PAD_CONF(L"SVID0_ALERT_B",            M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0258,  NORTH), // U2B2.A51 IDTP9180 VR_SVID_ALERT_B SOC_SVID_ALERT_B
  BXT_GPIO_PAD_CONF(L"SVID0_DATA",               M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0260,  NORTH), // U2B2.A50 VR_SVID_DATA SOC_SVID_DATA
  BXT_GPIO_PAD_CONF(L"SVID0_CLK",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0268,  NORTH), // U2B2.B48 VR_SVID_CLK SOC_SVID_CLK
};

// North West Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_NW_OEM []=
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,     MMIO_Offset    ,  Community */
  BXT_GPIO_PAD_CONF(L"GPIO_187 HV_DDI0_DDC_SDA", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0000,  NORTHWEST), // J2A1.16 HDMI_DDC_SDA Pull 2.2K to 3.3V HDMI_SMB_SOC_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_188 HV_DDI0_DDC_SCL", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0008,  NORTHWEST), // J2A1.15 HDMI_DDC_SCL Pull 2.2K to 3.3V HDMI_SMB_SOC_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_189 HV_DDI1_DDC_SDA", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled,  P_2K_H,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0010,  NORTHWEST), // J2A2.17 DP_AUX_INTEROP_DN DP_DDC_SDA, control by DP_DDC_EN
  BXT_GPIO_PAD_CONF(L"GPIO_190 HV_DDI1_DDC_SCL", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled,  P_2K_H,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0018,  NORTHWEST), // J2A2.15 DP_AUX_INTEROP_DP DP_DDC_SCL control by DP_DDC_EN
  BXT_GPIO_PAD_CONF(L"GPIO_191 DBI_SDA",         M1   ,    NA    , NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,   GPIO_PADBAR+0x0020,  NORTHWEST), // J6M1.31 MDSI_I2C_SDA & J6M2.31 Pull 2.2K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_192 DBI_SCL",         M1   ,    NA    , NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,   GPIO_PADBAR+0x0028,  NORTHWEST), // J6M1.33 MDSI_I2C_SCL & J6M2.33 Pull 2.2K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_193 PANEL0_VDDEN",    M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0030,  NORTHWEST), // U6L2.02 V3P3_EDP_EN           & J6M2.15 PANEL0_VDDEN_1_8V_R   & J6M2.09 PANEL0_VDDEN_3_3V_R
  BXT_GPIO_PAD_CONF(L"GPIO_194 PANEL0_BKLTEN",   M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0038,  NORTHWEST), // J1B1.37 PANEL0_BKLTEN_3_3V_R  & J6M2.17 PANEL0_BKLTEN_1_8V_R  & J6M2.11 PANEL0_BKLTEN_3_3V_R
  BXT_GPIO_PAD_CONF(L"GPIO_195 PANEL0_BKLTCTL",  M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0040,  NORTHWEST), // J1B1.36 PANEL0_BKLTCTL_3_3V_R & J6M2.19 PANEL0_BKLTCTL_1_8V_R & J6M2.13 PANEL0_BKLTCTL_3_3V_R
  BXT_GPIO_PAD_CONF(L"GPIO_196 PANEL1_VDDEN",    M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0048,  NORTHWEST), //#U6M2.02 V1P8_SW_CTRL          & J6M1.15 PANEL1_VDDEN_1_8V_R   & J6M1.09 PANEL1_VDDEN_3_3V_R
  BXT_GPIO_PAD_CONF(L"GPIO_197 PANEL1_BKLTEN",   M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0050,  NORTHWEST), //#U6L1.02 OLED_EN               & J6M1.17 PANEL1_BKLTEN_1_8V_R  & J6M1.11 PANEL1_BKLTEN_3_3V_R
  BXT_GPIO_PAD_CONF(L"GPIO_198 PANEL1_BKLTCTL",  M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0058,  NORTHWEST), //#U1B2.17 PANEL1_BKLTCTL_3_3V_R & J6M1.19 PANEL1_BKLTCTL_1_8V_R & J6M1.13 PANEL1_BKLTCTL_3_3V_R
  BXT_GPIO_PAD_CONF(L"GPIO_199 DBI_CSX",         M2  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0060,  NORTHWEST), // J2A2.18 DP_HPD SOC_DP_HPD (DDI1 DP++ HPD)
  BXT_GPIO_PAD_CONF(L"GPIO_200 DBI_RESX",        M2  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0068,  NORTHWEST), // J2A1.19 HDMI_CBL_HPD SOC_HDMI_HPD_N (DDI0 HDMI HPD)
  BXT_GPIO_PAD_CONF(L"GPIO_201 GP_INTD_DSI_TE1", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0070,  NORTHWEST), // J6M1.04 SOC_TE1
  BXT_GPIO_PAD_CONF(L"GPIO_202 GP_INTD_DSI_TE2", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0078,  NORTHWEST), // J6M2.04 SOC_TE2
  BXT_GPIO_PAD_CONF(L"GPIO_203 USB_OC0_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0080,  NORTHWEST), // U4A3.06&10 USB_OC0_N -> V5_USB3_OTG
  BXT_GPIO_PAD_CONF(L"GPIO_204 USB_OC1_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0088,  NORTHWEST), // U5B1.06&10/U4B2.06&10 USB_OC0_N -> V5_USB3_1 V5_USB3_2 V5_USB2_5+6 V5_USB2_7
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS0",              M0  ,     GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,    NA   ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x0090,  NORTHWEST), //#ECO_ID3
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS1",              M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x0098,  NORTHWEST), // J1B1.10 EDP_CONN_HPD SOC_EDP_HPD
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS2",              M0  ,     GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,    NA   ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00A0,  NORTHWEST), //#ECO_ID2
  BXT_GPIO_PAD_CONF(L"PMC_SPI_RXD",              M0  ,     GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,    NA   ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00A8,  NORTHWEST), //#ECO_ID1
  BXT_GPIO_PAD_CONF(L"PMC_SPI_TXD",              M0  ,     GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,    NA   ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00B0,  NORTHWEST), //#ECO_ID0
  BXT_GPIO_PAD_CONF(L"PMC_SPI_CLK",              M0  ,     GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,    NA   ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00B8,  NORTHWEST), //#ECO_ID4
  BXT_GPIO_PAD_CONF(L"PMIC_PWRGOOD",             M0   ,    GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_1K_H ,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x00C0,  NORTHWEST), // NC, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_223 PMIC_RESET_B",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00C8,  NORTHWEST), //#BOARD_ID2
  BXT_GPIO_PAD_CONF(L"GPIO_213 PMIC_SDWN_B",     M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00D0,  NORTHWEST), //#BOARD_ID1
  BXT_GPIO_PAD_CONF(L"GPIO_214 PMIC_BCUDISW2",   M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00D8,  NORTHWEST), //#FAB_ID2
  BXT_GPIO_PAD_CONF(L"GPIO_215 PMIC_BCUDISCRIT", M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00E0,  NORTHWEST), //#FAB_ID3
  BXT_GPIO_PAD_CONF(L"PMIC_THERMTRIP_B",         M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x00E8,  NORTHWEST), // U2B2.B35 PMIC_THERMTRIP PMIC_THERMTRIP_N, DS3B1 Thermal LED
  BXT_GPIO_PAD_CONF(L"GPIO_224 PMIC_STDBY",      M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x00F0,  NORTHWEST), //#BOARD_ID0
  BXT_GPIO_PAD_CONF(L"PROCHOT_B",                M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,HizRx1I   ,   SAME,  GPIO_PADBAR+0x00F8,  NORTHWEST), // U2B2.B34 PROCHOT_R_N
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SCL",             M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x0100,  NORTHWEST), //#FAB_ID1
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SDA",             M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled,  P_NONE,   NA    ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x0108,  NORTHWEST), //#FAB_ID0
  // J2M1 M.2 Key B for GPIO, Follow Intel BIOS config.
  BXT_GPIO_PAD_CONF(L"GPIO_74 AVS_I2S1_MCLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0110,  NORTHWEST), // J2T1.01 AVS_I2S_MCLK_HDR SOC_AVS_I2S1_MCLK
  BXT_GPIO_PAD_CONF(L"GPIO_75 AVS_I2S1_BCLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0118,  NORTHWEST), //*J2M1.20 NGFF_I2S_1_CLK_R_BT SOC_AVS_I2S1_BCLK
  BXT_GPIO_PAD_CONF(L"GPIO_76 AVS_I2S1_WS_SYNC", M0   ,    GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0120,  NORTHWEST), //*J2M1.28 NGFF_I2S_1_FS_R_BT SOC_AVS_I2S1_WS_SYNC
  BXT_GPIO_PAD_CONF(L"GPIO_77 AVS_I2S1_SDI",     M0   ,    GPI   , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0128,  NORTHWEST), //*J2M1.22 NGFF_I2S_1_RXD_R_BT SOC_AVS_I2S1_SDI
  BXT_GPIO_PAD_CONF(L"GPIO_78 AVS_I2S1_SDO",     M0   ,    GPI   , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0130,  NORTHWEST), //*J2M1.24 NGFF_I2S_1_TXD_R_BT SOC_AVS_I2S1_SDO Pull 4.22K to LOW
  // J2T1 LPE Header for I2S, Set to Mode 1: AVS_I2S1
  BXT_GPIO_PAD_CONF(L"GPIO_79 AVS_M_CLK_A1",     M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0138,  NORTHWEST), // J2T1.24 SOC_AVS_I2S4_BCLK
  BXT_GPIO_PAD_CONF(L"GPIO_80 AVS_M_CLK_B1",     M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0140,  NORTHWEST), // J2T1.21 SOC_AVS_I2S4_WS_SYNC
  BXT_GPIO_PAD_CONF(L"GPIO_81 AVS_M_DATA_1",     M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,TxDRxE    ,   EnPd,  GPIO_PADBAR+0x0148,  NORTHWEST), // J2T1.23 SOC_AVS_I2S4_SDI
  BXT_GPIO_PAD_CONF(L"GPIO_82 AVS_M_CLK_AB2",    M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0150,  NORTHWEST), // J2T1.22 SOC_AVS_I2S4_SDO
  // Follow Intel BIOS Config
  BXT_GPIO_PAD_CONF(L"GPIO_83 AVS_M_DATA_2",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,TxDRxE    ,   EnPd,  GPIO_PADBAR+0x0158,  NORTHWEST), // J2T1.26 SOC_AVS_M_DATA_2
  // ALC662 for HDA, Set to Mode 2: AVS_HDA_RST_N
  BXT_GPIO_PAD_CONF(L"GPIO_84 AVS_I2S2_MCLK",    M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0160,  NORTHWEST), // U6B1.11 HDA_RST_N (ALC662), Mode2:AVS_HDA_RST_N
  // J2T1 LPE Header for I2S, Set to Mode 1: AVS_I2S2
  BXT_GPIO_PAD_CONF(L"GPIO_85 AVS_I2S2_BCLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0168,  NORTHWEST), // J2T1.12 SOC_AVS_I2S2_BCLK
  BXT_GPIO_PAD_CONF(L"GPIO_86 AVS_I2S2_WS_SYNC", M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0170,  NORTHWEST), // J2T1.09 SOC_AVS_I2S2_WS_SYNC
  BXT_GPIO_PAD_CONF(L"GPIO_87 AVS_I2S2_SDI",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0178,  NORTHWEST), // J2T1.11 SOC_AVS_I2S2_SDI
  BXT_GPIO_PAD_CONF(L"GPIO_88 AVS_I2S2_SDO",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0180,  NORTHWEST), // J2T1.10 SOC_AVS_I2S2_SDO, Pull 4.22K to LOW
  // J2P1 M.2 Key E (WIFI/BT) for I2S, Set to Mode 1: AVS_I2S3
  BXT_GPIO_PAD_CONF(L"GPIO_89 AVS_I2S3_BCLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0188,  NORTHWEST), // J2P1.08 NGFF_I2S_3_CLK_R_BT SOC_AVS_I2S3_BCLK
  BXT_GPIO_PAD_CONF(L"GPIO_90 AVS_I2S3_WS_SYNC", M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0190,  NORTHWEST), // J2P1.10 NGFF_I2S_3_FS_R_BT SOC_AVS_I2S3_WS_SYNC
  BXT_GPIO_PAD_CONF(L"GPIO_91 AVS_I2S3_SDI",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,   EnPd,  GPIO_PADBAR+0x0198,  NORTHWEST), // J2P1.12 NGFF_I2S_3_RXD_R_BT SOC_AVS_I2S3_SDI
  BXT_GPIO_PAD_CONF(L"GPIO_92 AVS_I2S3_SDO",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01A0,  NORTHWEST), // J2P1.14 NGFF_I2S_3_TXD_R_BT SOC_AVS_I2S3_SDO, Pull 4.22K to LOW
  // W25Q128FW & J5D1 Header, Set to Mode 1: FST_SPI
  BXT_GPIO_PAD_CONF(L"GPIO_97 FST_SPI_CS0_B",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01A8,  NORTHWEST), // U5E1.01 SPI0_CS_N FAST_SPI_CS_N (W25Q128FW)
  BXT_GPIO_PAD_CONF(L"GPIO_98 FST_SPI_CS1_B",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01B0,  NORTHWEST), // J5D1.04 SPI0_CS1_N & J5D1.13 SPI0_CS2_N FAST_SPI_CS1_N
  BXT_GPIO_PAD_CONF(L"GPIO_99 FST_SPI_MOSI_IO0", M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01B8,  NORTHWEST), // U5E1.05/J5D1.12 SPI0_MOSI FAST_SPI_D0 
  BXT_GPIO_PAD_CONF(L"GPIO_100 FST_SPI_MISO_IO1",M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01C0,  NORTHWEST), // U5E1.02/J5D1.10 SPI0_MISO FAST_SPI_D1
  BXT_GPIO_PAD_CONF(L"GPIO_101 FST_SPI_IO2",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01C8,  NORTHWEST), // U5E1.03/J5D1.08 SPI0_WP_N Pull 20K to 1.8V FAST_SPI_D2, 
  BXT_GPIO_PAD_CONF(L"GPIO_102 FST_SPI_IO3",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01D0,  NORTHWEST), // U5E1.07/J5D1.09 SPI0_HOLD_N Pull 20K to 1.8V FAST_SPI_D3
  BXT_GPIO_PAD_CONF(L"GPIO_103 FST_SPI_CLK",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, Native_control,NA,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01D8,  NORTHWEST), // U5E1.06/J5D1.07 SPI0_CLK FAST_SPI_CLK
  BXT_GPIO_PAD_CONF(L"FST_SPI_CLK_FB",           M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01E0,  NORTHWEST), // no pin out
  // J6C1 & J6C2 Header, Set to Mode 1:SIO_SPI_0 
  BXT_GPIO_PAD_CONF(L"GPIO_104 GP_SSP_0_CLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x01E8,  NORTHWEST), // J6C1.03 GP_SSP_0_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_105 GP_SSP_0_FS0",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x01F0,  NORTHWEST), // J6C1.02 GP_SSP_0_FS0
  BXT_GPIO_PAD_CONF(L"GPIO_106 GP_SSP_0_FS1",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x01F8,  NORTHWEST), // J6C1.01 GP_SSP_0_FS1 Pull 4.22K to 1.8V & J4C1.36 BOOT_HALT_N_STRAP
  BXT_GPIO_PAD_CONF(L"GPIO_109 GP_SSP_0_RXD",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x0200,  NORTHWEST), // J6B2.20 GP_SSP_0_RXD
  BXT_GPIO_PAD_CONF(L"GPIO_110 GP_SSP_0_TXD",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x0208,  NORTHWEST), // J6B2.19 GP_SSP_0_TXD, Pull 4.22K to LOW
  // J6C1 Header, SIO_SPI_1 & LPSS_UART3 not support, follow Intel BIOS config.
  BXT_GPIO_PAD_CONF(L"GPIO_111 GP_SSP_1_CLK",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0210,  NORTHWEST), // J6C1.10 GP_SSP_1_CLK, Pull 4.22K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_112 GP_SSP_1_FS0",    M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0218,  NORTHWEST), // J6C1.09 GP_SSP_1_R_FS0 GP_SSP_1_FS0, 
  BXT_GPIO_PAD_CONF(L"GPIO_113 GP_SSP_1_FS1",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x0220,  NORTHWEST), // GP_SSP_1_FS1, Pull 4.22K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0228,  NORTHWEST), // J6C1.07 GP_SSP_1_RXD
  BXT_GPIO_PAD_CONF(L"GPIO_117 GP_SSP_1_TXD",    M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0230,  NORTHWEST), // J6C1.06 GP_SSP_1_TXD, Pull 4.22K to LOW
  // J6C1 Header, Set to Mode 1:SIO_SPI_2
  BXT_GPIO_PAD_CONF(L"GPIO_118 GP_SSP_2_CLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0238,  NORTHWEST), // J6C1.17 GP_SSP_2_CLK, Pull 4.22K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_119 GP_SSP_2_FS0",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0240,  NORTHWEST), // J6C1.16 GP_SSP_2_FS0, Pull 4.22K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_120 GP_SSP_2_FS1",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0248,  NORTHWEST), // J6C1.15 GP_SSP_2_FS1, Pull 4.22K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_121 GP_SSP_2_FS2",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0250,  NORTHWEST), // J6C1.18 GP_SSP_2_FS2
  BXT_GPIO_PAD_CONF(L"GPIO_122 GP_SSP_2_RXD",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0258,  NORTHWEST), // J6C1.14 GP_SSP_2_RXD
  BXT_GPIO_PAD_CONF(L"GPIO_123 GP_SSP_2_TXD",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0260,  NORTHWEST), // J6C1.13 GP_SSP_2_TXD, Pull 4.22K to 1.8V
};

// West Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_W_OEM [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,     MMIO_Offset    , Community */
  BXT_GPIO_PAD_CONF(L"GPIO_124 LPSS_I2C0_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0000,  WEST), // J3M2.15 I2C0_R_SDA, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_125 LPSS_I2C0_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0008,  WEST), // J3M2.06 I2C0_R_SCL, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_126 LPSS_I2C1_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0010,  WEST), // J5C1.09 I2C1_R_SDA, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_127 LPSS_I2C1_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0018,  WEST), // J5C1.08 I2C1_R_SCL, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_128 LPSS_I2C2_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0020,  WEST), // J3M2.05 I2C2_R_SDA, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_129 LPSS_I2C2_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0028,  WEST), // J3M2.07 I2C2_R_SCL, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_130 LPSS_I2C3_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0030,  WEST), // J2T1.38 I2C3_SDA_AUD_CONN I2C3_R_SDA, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_131 LPSS_I2C3_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0038,  WEST), // J2T1.37 I2C3_SCL_AUD_CONN I2C3_R_SCL, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_132 LPSS_I2C4_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0040,  WEST), // J3M1.05 I2C4_R_SDA, Pull 1K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_133 LPSS_I2C4_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0048,  WEST), // J3M1.07 I2C4_R_SCL, Pull 1K to 1.8V
  // [HSD 1205461649] Program ISH I2C GPIO pins before MRC as request.
//BXT_GPIO_PAD_CONF(L"GPIO_134 ISH_I2C0_SDA",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0050,  WEST), // U4B3.34/J6C2.05 I2C5_R_3P3_SCL & J6C2.01 I2C5_R_SCL
//BXT_GPIO_PAD_CONF(L"GPIO_135 ISH_I2C0_SCL",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0058,  WEST), // U4B3.36/J6C2.15 I2C5_R_3P3_SDA & J6C2.03 I2C5_R_SDA
//BXT_GPIO_PAD_CONF(L"GPIO_136 ISH_I2C1_SDA",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0060,  WEST), // J2P1.58/J6C2.20 I2C6_R_3P3_SDA & J6C2.04 I2C6_R_SDA
//BXT_GPIO_PAD_CONF(L"GPIO_137 ISH_I2C1_SCL",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0068,  WEST), // J2P1.60/J6C2.14 I2C6_R_3P3_SCL & J6C2.02 I2C6_R_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_138 LPSS_I2C7_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0070,  WEST), //*J2M1.42 I2C7_R_3P3_SDA & J3M1.15 I2C7_R_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_139 LPSS_I2C7_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0078,  WEST), //*J2M1.40 I2C7_R_3P3_SCL & J3M1.06 I2C7_R_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_146 ISH_GPIO_0",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0080,  WEST), // U6B1.06 HDA_BITCLK SOC_AVS_I2S6_BCLK (AVS_HDA_BCLK)
  BXT_GPIO_PAD_CONF(L"GPIO_147 ISH_GPIO_1",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0088,  WEST), // U6B1.10 HDA_SYNC SOC_AVS_I2S6_WS_SYNC (AVS_HDA_WS_SYNC)
  BXT_GPIO_PAD_CONF(L"GPIO_148 ISH_GPIO_2",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0090,  WEST), // U6B1.08 AUD_LINK_SDI0_SOC SOC_AVS_I2S6_SDI (AVS_HDA_SDI)
  BXT_GPIO_PAD_CONF(L"GPIO_149 ISH_GPIO_3",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0098,  WEST), // U6B1.05 HDA_SDOUT SOC_AVS_I2S6_SDO (AVS_HDA_SDO)
  BXT_GPIO_PAD_CONF(L"GPIO_150 ISH_GPIO_4",      M2   ,    NA   ,  NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00A0,  WEST), //*J6B2.01 ISH_GPIO4 SOC_AVS_I2S5_BCLK
  BXT_GPIO_PAD_CONF(L"GPIO_151 ISH_GPIO_5",      M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00A8,  WEST), // NC, ISH_3P3_GPIO5 ISH_GPIO5, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_152 ISH_GPIO_6",      M2   ,    NA  ,   NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00B0,  WEST), //*J6B2.03 ISH_GPIO6 SOC_AVS_I2S5_SDI
  BXT_GPIO_PAD_CONF(L"GPIO_153 ISH_GPIO_7",      M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x00B8,  WEST), //*J6B2.02 SOC_AVS_I2S5_SDO
  BXT_GPIO_PAD_CONF(L"GPIO_154 ISH_GPIO_8",      M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00C0,  WEST), // J2P1.54 NGFF_3P3_KILL_BT_N ISH_GPIO8
  BXT_GPIO_PAD_CONF(L"GPIO_155 ISH_GPIO_9",      M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x00C8,  WEST), //*J6B2.04 ISH_GPIO9
  BXT_GPIO_PAD_CONF(L"GPIO_209 PCIE_CLKREQ0_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,   HizRx0I,  EnPd,  GPIO_PADBAR+0x00D0,  WEST), // PCIE_CLKREQ0_N, Pull 4.7K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,  EnPd,  GPIO_PADBAR+0x00D8,  WEST), // J6B1.B31&B17 PCIE_CLKREQ1_N, Pull 10K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_211 PCIE_CLKREQ2_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,  EnPd,  GPIO_PADBAR+0x00E0,  WEST), // J2M1.52 PCIE_CLKREQ2_N, Pull 4.7K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_212 PCIE_CLKREQ3_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,  EnPd,  GPIO_PADBAR+0x00E8,  WEST), // J2P1.53 PCIE_CLKREQ3_N, Pull 4.7K to 1.8V 
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_0",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00F0,  WEST), // J3M2.34 MCSI1_XVCLK
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_1",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00F8,  WEST), // J3M1.34 MCSI2_XVCLK
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_2",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0100,  WEST), // NC
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_3",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0108,  WEST), // NC
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_4",            M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0110,  WEST), // NC
  BXT_GPIO_PAD_CONF(L"PMU_AC_PRESENT",           M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0118,  WEST), // PMU_AC_PRESENT, Pull 10K to 3.3V
  BXT_GPIO_PAD_CONF(L"PMU_BATLOW_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0120,  WEST), // PMU_BATLOW_N, Pull 10K to 3.3V
  BXT_GPIO_PAD_CONF(L"PMU_PLTRST_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0128,  WEST), // All platform Reset Pin, PMU_PLTRST_N, Pull 4.7K to 3.3V
  BXT_GPIO_PAD_CONF(L"PMU_PWRBTN_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0130,  WEST), // U2B2.B3 PWRBTN_N PWRBTN_3P3_N
  BXT_GPIO_PAD_CONF(L"PMU_RESETBUTTON_B",        M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0138,  WEST), // J4C1.06 HOOK_7 & U2T2.2 RESETBTN_N & J6E4.07 RSTBTN_N
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S0_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0140,  WEST), // U2B1.12 HDMI_PD_N & U2B2.B37 PMIC_SLP_S0IX_N SOC_SLP_S0IX_N
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S3_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0148,  WEST), // U2B2.B36 PMIC_SLP_S3_N SOC_SLP_S3_N
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S4_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0150,  WEST), // U2B2.A42 PMIC_SLP_S4_N SOC_SLP_S4_N
  BXT_GPIO_PAD_CONF(L"PMU_SUSCLK",               M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0158,  WEST), // J2M1.68 3G_SUSCLK_3P3 & J2P1.50 WIFI_SUSCLK SUSCLK_3P3
  BXT_GPIO_PAD_CONF(L"PMU_WAKE_B",               M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0160,  WEST), //*U2B2.A15 PMU_WAKE_N, Pull 1K to 3.3V
  BXT_GPIO_PAD_CONF(L"SUS_STAT_B",               M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0168,  WEST), // J6E1.19 LPCPD_N SUS_STAT_N
  BXT_GPIO_PAD_CONF(L"SUSPWRDNACK",              M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0170,  WEST), // U2B2.A40 PMIC_SUSPWRDNACK, Pull 100K to LOW SUSPWRDNACK
};

 // South West Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_SW_OEM []=
{
  //                  Group Pin#:  pad_name,       PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,   MMIO_Offset     ,  Community */
  BXT_GPIO_PAD_CONF(L"GPIO_205 PCIE_WAKE0_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0000 ,  SOUTHWEST), // U4B3.16 LAN_WAKE PCIE_WAKE0_LAN_N PCIE_WAKE0_N (LAN)
  BXT_GPIO_PAD_CONF(L"GPIO_206 PCIE_WAKE1_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0008 ,  SOUTHWEST), // J6B1.B11 PCIE_WAKE1_3P3_N PCIE_WAKE1_N (PCIe x4 Slot)
  BXT_GPIO_PAD_CONF(L"GPIO_207 PCIE_WAKE2_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0010 ,  SOUTHWEST), // J2M1.54 PEWAKE2_N PCIE_WAKE2_N (M.2 Key B)
  BXT_GPIO_PAD_CONF(L"GPIO_208 PCIE_WAKE3_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0018 ,  SOUTHWEST), // J2P1.21 PEWAKE0_N PCIE_WAKE3_N (M.2 Key E)
  BXT_GPIO_PAD_CONF(L"GPIO_156 EMMC0_CLK",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I , SAME,GPIO_PADBAR+0x0020 ,  SOUTHWEST), // U2T1.K5 EMMC0_CLK_R EMMC0_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_157 EMMC0_D0",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0028 ,  SOUTHWEST), // U2T1.A3 EMMC0_D0_R EMMC0_D0
  BXT_GPIO_PAD_CONF(L"GPIO_158 EMMC0_D1",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0030 ,  SOUTHWEST), // U2T1.A4 EMMC0_D1_R EMMC0_D1
  BXT_GPIO_PAD_CONF(L"GPIO_159 EMMC0_D2",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0038 ,  SOUTHWEST), // U2T1.A5 EMMC0_D2_R EMMC0_D2
  BXT_GPIO_PAD_CONF(L"GPIO_160 EMMC0_D3",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0040 ,  SOUTHWEST), // U2T1.B2 EMMC0_D3_R EMMC0_D3
  BXT_GPIO_PAD_CONF(L"GPIO_161 EMMC0_D4",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0048 ,  SOUTHWEST), // U2T1.B3 EMMC0_D4_R EMMC0_D4
  BXT_GPIO_PAD_CONF(L"GPIO_162 EMMC0_D5",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0050 ,  SOUTHWEST), // U2T1.B4 EMMC0_D5_R EMMC0_D5
  BXT_GPIO_PAD_CONF(L"GPIO_163 EMMC0_D6",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0058 ,  SOUTHWEST), // U2T1.B5 EMMC0_D6_R EMMC0_D6
  BXT_GPIO_PAD_CONF(L"GPIO_164 EMMC0_D7",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0060 ,  SOUTHWEST), // U2T1.B6 EMMC0_D7_R EMMC0_D7
  BXT_GPIO_PAD_CONF(L"GPIO_165 EMMC0_CMD",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0068 ,  SOUTHWEST), // U2T1.M5 EMMC0_CMD_R EMMC0_CMD, Pull 10K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_166 SDIO_CLK",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I , SAME,GPIO_PADBAR+0x0070 ,  SOUTHWEST), // J2P1.09 NGFF_SDEMMC2_CLK SDIO_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_167 SDIO_D0",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0078 ,  SOUTHWEST), // J2P1.13 NGFF_SDEMMC2_D0 SDIO_D0
  BXT_GPIO_PAD_CONF(L"GPIO_168 SDIO_D1",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0080 ,  SOUTHWEST), // J2P1.15 NGFF_SDEMMC2_D1 SDIO_D1
  BXT_GPIO_PAD_CONF(L"GPIO_169 SDIO_D2",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0088 ,  SOUTHWEST), // J2P1.17 NGFF_SDEMMC2_D2 SDIO_D2
  BXT_GPIO_PAD_CONF(L"GPIO_170 SDIO_D3",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0090 ,  SOUTHWEST), // J2P1.19 NGFF_SDEMMC2_D3 SDIO_D3
  BXT_GPIO_PAD_CONF(L"GPIO_171 SDIO_CMD",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0098 ,  SOUTHWEST), // J2P1.11 NGFF_SDEMMC2_CMD SDIO_CMD
  BXT_GPIO_PAD_CONF(L"GPIO_172 SDCARD_CLK",        M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx1I ,DisPuPd,GPIO_PADBAR+0x00A0 ,  SOUTHWEST), // J6E2.05 SDCARD_ESD_CLK, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_179 SDCARD_CLK_FB",     M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,   NA,GPIO_PADBAR+0x00A8 ,  SOUTHWEST), // No this pin
  BXT_GPIO_PAD_CONF(L"GPIO_173 SDCARD_D0",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I ,DisPuPd,GPIO_PADBAR+0x00B0 ,  SOUTHWEST), // J6E2.07 SDCARD_ESD_D0, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_174 SDCARD_D1",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x00B8 ,  SOUTHWEST), // J6E2.08 SDCARD_ESD_D1
  BXT_GPIO_PAD_CONF(L"GPIO_175 SDCARD_D2",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x00C0 ,  SOUTHWEST), // J6E2.01 SDCARD_ESD_D2
  BXT_GPIO_PAD_CONF(L"GPIO_176 SDCARD_D3",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x00C8 ,  SOUTHWEST), // J6E2.02 SDCARD_ESD_D3
  BXT_GPIO_PAD_CONF(L"GPIO_177 SDCARD_CD_B",       M0,     GPI   , GPIO_D ,  NA     ,   Edge     ,Wake_Disabled,  P_NONE,    NA   ,    NA  ,TxDRxE    ,   NA,GPIO_PADBAR+0x00D0 ,  SOUTHWEST), // J6E2.10 SDCARD_ESD_CD_N, Pull 100K to 1.8V
  BXT_GPIO_PAD_CONF(L"GPIO_178 SDCARD_CMD",        M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I ,DisPuPd,GPIO_PADBAR+0x00D8 ,  SOUTHWEST), // J6E2.03 SDCARD_ESD_CMD, Follow Intel BIOS
  BXT_GPIO_PAD_CONF(L"GPIO_186 SDCARD_LVL_WP",     M1,     GPI   , GPIO_D ,  NA     ,   Edge     ,Wake_Disabled, P_20K_L,No_invert,    NA  ,Last_Value, SAME,GPIO_PADBAR+0x00E0 ,  SOUTHWEST), // J6E2.09 SDCARD_LVL_R_WP SDCARD_LVL_WP
  BXT_GPIO_PAD_CONF(L"GPIO_182 EMMC0_STROBE",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,SAME  ,GPIO_PADBAR+0x00E8 ,  SOUTHWEST), // U2T1.H5 EMMC0_STROBE, Pull 100K to LOW
  BXT_GPIO_PAD_CONF(L"GPIO_183 SDIO_PWR_DOWN_B",   M0,     GPO   , GPIO_D ,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA       ,NA    ,GPIO_PADBAR+0x00F0 ,  SOUTHWEST), // SDMMC3_PWR_EN_N (SD Card Power)
  BXT_GPIO_PAD_CONF(L"SMB_ALERTB",                 M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x00F8 ,  SOUTHWEST), // J1B1.43 SOC_SMB_ALERTB, Pull 1K to 3.3V
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0100 ,  SOUTHWEST), // SMBUS SOC_SMB_CLK, Pull 1K to 3.3V
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0108 ,  SOUTHWEST), // SMBUS SOC_SMB_DATA, Pull 1K to 3.3V
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0110 ,  SOUTHWEST), // J5D1.16 SPI_TPM_SERIRQ SOC_ILB_3P3_SERIRQ & J6E1.16 LPC_TPM_SERIRQ SOC_ILB_SERIRQ, Pull 10K to 3.3V
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0118 ,  SOUTHWEST), // NC
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0120 ,  SOUTHWEST), // J6E1.01 L_CLKOUT1
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0128 ,  SOUTHWEST), // J6E1.11 LPC_AD0
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0130 ,  SOUTHWEST), // J6E1.10 LPC_AD1
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0138 ,  SOUTHWEST), // J6E1.08 LPC_AD2
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0140 ,  SOUTHWEST), // J6E1.07 LPC_AD3
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0148 ,  SOUTHWEST), // J6E1.18 L_CLKRUN_N
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0150 ,  SOUTHWEST), // J6E1.03 L_FRAME_N
*/
};
#endif

#ifdef GPIO_INIT_IN_PEI_PHASE_PRE_MEMORY
//=======================================================//
//                                                       //
// PEI phase Pre-Memory GPIO init                        //
//                                                       //
//=======================================================//

//
// Please modify for OEM platform, default is Apollo Lake Oxbow Hill CRB GPIO Table.
//
#define mPreMemGpioInitData_OEM_Support
BXT_GPIO_PAD_INIT  mPreMemGpioInitData_OEM[] =
{
  //
  // ISH I2C0 (LPSS I2C5) -> J6C2 (1.8V and 3.3V) & U4B3 WGI210AT (3.3V)
  // ISH I2C1 (LPSS I2C6) -> J6C2 (1.8V and 3.3V) & J2P1 M.2 (3.3V)
  //
  // Default set to ISH I2C0/I2C1 mode, GPIOs will switch to LPSS I2C5/I2C6 mode 
  // if set SystemConfiguration.ScIshEnabled = 0 for OxbowHill/LeafHill/JunperHill
  //
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,       MMIO_Offset,Community */
//BXT_GPIO_PAD_CONF(L"GPIO_134 ISH_I2C0_SDA",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA,IOS_Masked,  EnPu,  GPIO_PADBAR+0x0050,  WEST), // U4B3.34/J6C2.05 I2C5_R_3P3_SCL & J6C2.01 I2C5_R_SCL
//BXT_GPIO_PAD_CONF(L"GPIO_135 ISH_I2C0_SCL",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA,IOS_Masked,  EnPu,  GPIO_PADBAR+0x0058,  WEST), // U4B3.36/J6C2.15 I2C5_R_3P3_SDA & J6C2.03 I2C5_R_SDA
//BXT_GPIO_PAD_CONF(L"GPIO_136 ISH_I2C1_SDA",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA,IOS_Masked,  EnPu,  GPIO_PADBAR+0x0060,  WEST), // J2P1.58/J6C2.20 I2C6_R_3P3_SDA & J6C2.04 I2C6_R_SDA
//BXT_GPIO_PAD_CONF(L"GPIO_137 ISH_I2C1_SCL",    M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA,IOS_Masked,  EnPu,  GPIO_PADBAR+0x0068,  WEST), // J2P1.60/J6C2.14 I2C6_R_3P3_SCL & J6C2.02 I2C6_R_SCL

  //
  // LAN
  // GPIO_22 -> LAN_DEV_OFF_N, Pull High to enable LAN device (Require LAN FW support)
  //
//BXT_GPIO_PAD_CONF(L"GPIO_22",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00B0, NORTH), // U4B3.28 LAN_DEV_OFF_N SOC_GPIO22

};
#endif

#ifdef GPIO_INIT_IN_SEC_PHASE
//=======================================================//
//                                                       //
// OEM SEC phase GPIO init for UART debug                //
//                                                       //
//=======================================================//

//
// Please modify for OEM platform, default is Apollo Lake Oxbow Hill CRB GPIO Table.
//
#define mSecGpioInitData_OEM_Support
BXT_GPIO_PAD_INIT  mSecGpioInitData_OEM [] =
{
  //
  // UART
  //
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0170,  NORTH), // U6F1.24 FT2232D DEBUG_UART2_TXD SOC_COM2_RXD
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0178,  NORTH), // U6F1.23 FT2232D DEBUG_UART2_RXD SOC_COM2_TXD

  //
  // LPC
  //
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,   IOSstae, IOSTerm,        MMIO_Offset,  Community
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,IOS_Masked,    SAME,GPIO_PADBAR+0x0110 ,  SOUTHWEST), // J5D1.16 SPI_TPM_SERIRQ SOC_ILB_3P3_SERIRQ & J6E1.16 LPC_TPM_SERIRQ SOC_ILB_SERIRQ, Pull 10K to 3.3V
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   , NA  ,IOS_Masked,    SAME,GPIO_PADBAR+0x0118 ,  SOUTHWEST), // NC
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   , NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0120 ,  SOUTHWEST), // J6E1.01 L_CLKOUT1
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0128 ,  SOUTHWEST), // J6E1.11 LPC_AD0
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0130 ,  SOUTHWEST), // J6E1.10 LPC_AD1
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0138 ,  SOUTHWEST), // J6E1.08 LPC_AD2
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0140 ,  SOUTHWEST), // J6E1.07 LPC_AD3
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,IOS_Masked,    SAME,GPIO_PADBAR+0x0148 ,  SOUTHWEST), // J6E1.18 L_CLKRUN_N
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   , NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0150 ,  SOUTHWEST), // J6E1.03 L_FRAME_N

  //
  // Smbus
  //
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0100 ,  SOUTHWEST), // SMBUS SOC_SMB_CLK, Pull 1K to 3.3V
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0108 ,  SOUTHWEST), // SMBUS SOC_SMB_DATA, Pull 1K to 3.3V
};
#endif
//[-end-161207-IB07400824-modify]//

#else // USB CRB setting by build platform type

#ifdef BUILD_APL_RVP1
#include <BoardGpiosRvp.h>
#endif

#ifdef BUILD_APL_RVP2
#include <BoardGpiosRvp.h>
#endif

#ifdef BUILD_APLI_OXH
#include <BoardGpiosOxbowHill.h>
#endif

#ifdef BUILD_APLI_JNH
#include <BoardGpiosOxbowHill.h>
#endif

#ifdef BUILD_APLI_LFH
#include <BoardGpiosOxbowHill.h>
#endif

#endif

#endif
