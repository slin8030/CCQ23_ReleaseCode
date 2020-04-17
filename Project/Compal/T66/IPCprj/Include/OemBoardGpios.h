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
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0048,  NORTH),//NA->EDP, GPO->Mipi
  BXT_GPIO_PAD_CONF(L"GPIO_10",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0050,  NORTH),//VOL_UP
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0058,  NORTH),//VOL_DOWN
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0060,  NORTH),//BU_BR+
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0068,  NORTH),//BU_BR-
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0070,  NORTH),//INT
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0078,  NORTH),//INT
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0080,  NORTH),//CRISIS_IN(GPIO16)
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0088,  NORTH),//EDP:1, LVDS: 0
  BXT_GPIO_PAD_CONF(L"GPIO_18",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0090,  NORTH),//MIPI_ID1
  BXT_GPIO_PAD_CONF(L"GPIO_19",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0098,  NORTH),//MIPI_ID2
  BXT_GPIO_PAD_CONF(L"GPIO_20",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,   TxDRxE,     NA, GPIO_PADBAR+0x00A0,  NORTH),//9555IO_INT
  BXT_GPIO_PAD_CONF(L"GPIO_21",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00A8,  NORTH),//EC_SCI#_SOC
  BXT_GPIO_PAD_CONF(L"GPIO_22",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B0,  NORTH),//SATA_GP0
  BXT_GPIO_PAD_CONF(L"GPIO_23",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B8,  NORTH),//SATA_GP1
  BXT_GPIO_PAD_CONF(L"GPIO_24",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00C0,  NORTH),//SENSE2_IRQ
  BXT_GPIO_PAD_CONF(L"GPIO_25",                  M5   ,    GPO   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00C8,  NORTH),//DEVSLP1
  BXT_GPIO_PAD_CONF(L"GPIO_26",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D0,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_27",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D8,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_28",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_H,Inverted ,   SCI,    TxDRxE,     NA, GPIO_PADBAR+0x00E0,  NORTH),//EC_SCI#_SOC
  BXT_GPIO_PAD_CONF(L"GPIO_29",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00E8,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_30",                  M0   ,    GPO   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00F0,  NORTH),//GPIO30
  BXT_GPIO_PAD_CONF(L"GPIO_31",                  M0   ,    GPO   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00F8,  NORTH),//GPIO31
  BXT_GPIO_PAD_CONF(L"GPIO_32",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0100,  NORTH),//SPK_ID
  BXT_GPIO_PAD_CONF(L"GPIO_33",                  M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0108,  NORTH),//MIC_JD#
  BXT_GPIO_PAD_CONF(L"GPIO_34 PWM0",             M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0110,  NORTH),//PWM0_GPIO_34
  BXT_GPIO_PAD_CONF(L"GPIO_35 PWM1",             M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0118,  NORTH),//PWM1_GPIO_35
  BXT_GPIO_PAD_CONF(L"GPIO_36 PWM2",             M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0120,  NORTH),//[COM\PRJ]VCC1P24_1P35_SEL (0:1.24V,1:1.35V)
  BXT_GPIO_PAD_CONF(L"GPIO_37 PWM3",             M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0128,  NORTH),//NC

  BXT_GPIO_PAD_CONF(L"GPIO_38 LPSS_UART0_RXD",   M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0130,  NORTH),//UART_0_CRXD
  BXT_GPIO_PAD_CONF(L"GPIO_39 LPSS_UART0_TXD",   M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0138,  NORTH),//UART_0_CTXD_GPIO_39
  BXT_GPIO_PAD_CONF(L"GPIO_40 LPSS_UART0_RTS_B", M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0140,  NORTH),//[COM] UART_0_CRTS#_GPIO_40
  BXT_GPIO_PAD_CONF(L"GPIO_41 LPSS_UART0_CTS_B", M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0148,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_42 LPSS_UART1_RXD",   M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0150,  NORTH),//[COM] UART_1_CRXD
  BXT_GPIO_PAD_CONF(L"GPIO_43 LPSS_UART1_TXD",   M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx0I,DisPuPd, GPIO_PADBAR+0x0158,  NORTH),//[COM] UART_1_CTXD_GPIO_43
  BXT_GPIO_PAD_CONF(L"GPIO_44 LPSS_UART1_RTS_B", M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0160,  NORTH),//[COM] UART_1_CTXD_GPIO_44
  BXT_GPIO_PAD_CONF(L"GPIO_45 LPSS_UART1_CTS_B", M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,DisPuPd, GPIO_PADBAR+0x0168,  NORTH),//[COM] test point
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0170,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0178,  NORTH),
*/
  BXT_GPIO_PAD_CONF(L"GPIO_48 LPSS_UART2_RTS_B", M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0180,  NORTH),//[COM] UART_2_CRTS_DCTS
  BXT_GPIO_PAD_CONF(L"GPIO_49 LPSS_UART2_CTS_B", M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0188,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_62 GP_CAMERASB00",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0190,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_63 GP_CAMERASB01",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0198,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_64 GP_CAMERASB02",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01A0,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_65 GP_CAMERASB03",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01A8,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_66 GP_CAMERASB04",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01B0,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_67 GP_CAMERASB05",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01B8,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_68 GP_CAMERASB06",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01C0,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_69 GP_CAMERASB07",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01C8,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_70 GP_CAMERASB08",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01D0,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_71 GP_CAMERASB09",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01D8,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_72 GP_CAMERASB10",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01E0,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_73 GP_CAMERASB11",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01E8,  NORTH),//[COM] test point
  BXT_GPIO_PAD_CONF(L"TCK",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x01F0,  NORTH),
  BXT_GPIO_PAD_CONF(L"TRST_B",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x01F8,  NORTH),
  BXT_GPIO_PAD_CONF(L"TMS",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0200,  NORTH),
  BXT_GPIO_PAD_CONF(L"TDI",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0208,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PMODE",                 M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0210,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PREQ_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0218,  NORTH),
  BXT_GPIO_PAD_CONF(L"JTAGX",                    M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0220,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PRDY_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0228,  NORTH),
  BXT_GPIO_PAD_CONF(L"TDO",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0230,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_216 CNV_BRI_DT",      M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0238,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_217 CNV_BRI_RSP",     M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0240,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_218 CNV_RGI_DT",      M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,     NA, GPIO_PADBAR+0x0248,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_219 CNV_RGI_RSP",     M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0250,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"SVID0_ALERT_B",            M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0258,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_DATA",               M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0260,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_CLK",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0268,  NORTH),
};

// North West Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_NW_OEM []=
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,     MMIO_Offset    ,  Community */
  BXT_GPIO_PAD_CONF(L"GPIO_187 HV_DDI0_DDC_SDA", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0000,  NORTHWEST),//SOC_DP0_CTRL_DATA
  BXT_GPIO_PAD_CONF(L"GPIO_188 HV_DDI0_DDC_SCL", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0008,  NORTHWEST),//SOC_DP0_CTRL_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_189 HV_DDI1_DDC_SDA", M0  ,     GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0010,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_190 HV_DDI1_DDC_SCL", M0  ,     GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0018,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_191 DBI_SDA",         M0  ,     GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0020,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_192 DBI_SCL",         M0  ,     GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0028,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_193 PANEL0_VDDEN",    M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0030,  NORTHWEST),//PANEL_VDD_EN
  BXT_GPIO_PAD_CONF(L"GPIO_194 PANEL0_BKLTEN",   M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0038,  NORTHWEST),//PNL0_BKLTEN
  BXT_GPIO_PAD_CONF(L"GPIO_195 PANEL0_BKLTCTL",  M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0040,  NORTHWEST),//PNL0_BKLTCTL
  BXT_GPIO_PAD_CONF(L"GPIO_196 PANEL1_VDDEN",    M0  ,     GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0048,  NORTHWEST),//PANEL1_VDD_EN
  BXT_GPIO_PAD_CONF(L"GPIO_197 PANEL1_BKLTEN",   M0  ,     GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0050,  NORTHWEST),//PNL1_BKLTEN
  BXT_GPIO_PAD_CONF(L"GPIO_198 PANEL1_BKLTCTL",  M0  ,     GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   SAME,  GPIO_PADBAR+0x0058,  NORTHWEST),//PNL1_BKLTCTL
  BXT_GPIO_PAD_CONF(L"GPIO_199 DBI_CSX",         M0  ,     GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0060,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_200 DBI_RESX",        M2  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,HizRx1I   ,DisPuPd,  GPIO_PADBAR+0x0068,  NORTHWEST),//SOC_DP0_HPD#
  BXT_GPIO_PAD_CONF(L"GPIO_201 GP_INTD_DSI_TE1", M0  ,     GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0070,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_202 GP_INTD_DSI_TE2", M0  ,     GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I ,   SAME,  GPIO_PADBAR+0x0078,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_203 USB_OC0_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0080,  NORTHWEST),//USB_OC0#
  BXT_GPIO_PAD_CONF(L"GPIO_204 USB_OC1_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0088,  NORTHWEST),//USB_OC1#
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS0",              M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x0090,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS1",              M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x0098,  NORTHWEST),//EDP_HPD#
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS2",              M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x00A0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_RXD",              M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x00A8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_TXD",              M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x00B0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_CLK",              M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x00B8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_PWRGOOD",             M0   ,    GPO   ,GPIO_D,  HI     ,   NA      ,Wake_Disabled, P_1K_H ,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x00C0,  NORTHWEST),//Feature: Reset             Net in Sch: GNSS_RST_N
  BXT_GPIO_PAD_CONF(L"PMIC_RESET_B",             M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x00C8,  NORTHWEST),//[COM] Test Point
  BXT_GPIO_PAD_CONF(L"GPIO_213 PMIC_SDWN_B",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x00D0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_214 PMIC_BCUDISW2",   M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x00D8,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_215 PMIC_BCUDISCRIT", M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x00E0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"PMIC_THERMTRIP_B",         M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,Last_Value,   SAME,  GPIO_PADBAR+0x00E8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_STDBY",               M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x00F0,  NORTHWEST),//[COM] Test Point
  BXT_GPIO_PAD_CONF(L"PROCHOT_B",                M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   ,   SAME,  GPIO_PADBAR+0x00F8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SCL",             M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_1K_H ,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0100,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SDA",             M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_1K_H ,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0108,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_74 AVS_I2S1_MCLK",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0110,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_75 AVS_I2S1_BCLK",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0118,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_76 AVS_I2S1_WS_SYNC", M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0120,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_77 AVS_I2S1_SDI",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0128,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_78 AVS_I2S1_SDO",     M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0130,  NORTHWEST),//I2S1_SDO_GPIO_78
  BXT_GPIO_PAD_CONF(L"GPIO_79 AVS_M_CLK_A1",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0138,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_80 AVS_M_CLK_B1",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x0140,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_81 AVS_M_DATA_1",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,   EnPd,  GPIO_PADBAR+0x0148,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_82 AVS_M_CLK_AB2",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0150,  NORTHWEST),//DMIC_CLK_AB2_GPIO_82
  BXT_GPIO_PAD_CONF(L"GPIO_83 AVS_M_DATA_2",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,   EnPd,  GPIO_PADBAR+0x0158,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_84 AVS_I2S2_MCLK",    M2   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0160,  NORTHWEST),//HDA_RST#
  BXT_GPIO_PAD_CONF(L"GPIO_85 AVS_I2S2_BCLK",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0168,  NORTHWEST),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_86 AVS_I2S2_WS_SYNC", M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0170,  NORTHWEST),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_87 AVS_I2S2_SDI",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0178,  NORTHWEST),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_88 AVS_I2S2_SDO",     M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0180,  NORTHWEST),//I2S2_SDO_GPIO_88
  BXT_GPIO_PAD_CONF(L"GPIO_89 AVS_I2S3_BCLK",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0188,  NORTHWEST),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_90 AVS_I2S3_WS_SYNC", M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0190,  NORTHWEST),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_91 AVS_I2S3_SDI",     M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x0198,  NORTHWEST),//[COM] test point
  BXT_GPIO_PAD_CONF(L"GPIO_92 AVS_I2S3_SDO",     M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x01A0,  NORTHWEST),//I2S3_SDO_GPIO_92
  BXT_GPIO_PAD_CONF(L"GPIO_97 FST_SPI_CS0_B",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01A8,  NORTHWEST),//SOC_SPI_CS#0
  BXT_GPIO_PAD_CONF(L"GPIO_98 FST_SPI_CS1_B",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE    ,     NA,  GPIO_PADBAR+0x01B0,  NORTHWEST),//SOC_SPI_CS#1
  BXT_GPIO_PAD_CONF(L"GPIO_99 FST_SPI_MOSI_IO0", M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01B8,  NORTHWEST),//SOC_SPI_SI
  BXT_GPIO_PAD_CONF(L"GPIO_100 FST_SPI_MISO_IO1",M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01C0,  NORTHWEST),//SOC_SPI_SO
  BXT_GPIO_PAD_CONF(L"GPIO_101 FST_SPI_IO2",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01C8,  NORTHWEST),//SOC_SPI_IO2
  BXT_GPIO_PAD_CONF(L"GPIO_102 FST_SPI_IO3",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01D0,  NORTHWEST),//SOC_SPI_IO3
  BXT_GPIO_PAD_CONF(L"GPIO_103 FST_SPI_CLK",     M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,IOS_Masked,   SAME,  GPIO_PADBAR+0x01D8,  NORTHWEST),//SOC_SPI_CLK
  BXT_GPIO_PAD_CONF(L"FST_SPI_CLK_FB",           M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,IOS_Masked,     NA,  GPIO_PADBAR+0x01E0,  NORTHWEST),//[PRJ] EE request
  BXT_GPIO_PAD_CONF(L"GPIO_104 GP_SSP_0_CLK",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x01E8,  NORTHWEST),//SIO_SPI_0_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_105 GP_SSP_0_FS0",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x01F0,  NORTHWEST),//SIO_SPI_0_FS0
  BXT_GPIO_PAD_CONF(L"GPIO_106 GP_SSP_0_FS1",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x01F8,  NORTHWEST),//SPI_0_FS1_TPM_CS#
  BXT_GPIO_PAD_CONF(L"GPIO_109 GP_SSP_0_RXD",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x0200,  NORTHWEST),//SIO_SPI_0_RX
  BXT_GPIO_PAD_CONF(L"GPIO_110 GP_SSP_0_TXD",    M1   ,    NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE ,    NA   ,    NA  ,HizRx0I   ,   EnPd,  GPIO_PADBAR+0x0208,  NORTHWEST),//SPI_0_TX_GPIO_110
  BXT_GPIO_PAD_CONF(L"GPIO_111 GP_SSP_1_CLK",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0210,  NORTHWEST),//SPI_1_CLK_GPIO_111
  BXT_GPIO_PAD_CONF(L"GPIO_112 GP_SSP_1_FS0",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0218,  NORTHWEST),//SPI_1_FS0_GPIO_112
  BXT_GPIO_PAD_CONF(L"GPIO_113 GP_SSP_1_FS1",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0220,  NORTHWEST),//SPI_1_FS1_GPIO_113
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0228,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_117 GP_SSP_1_TXD",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0230,  NORTHWEST),//SPI_1_TX_GPIO_117
  BXT_GPIO_PAD_CONF(L"GPIO_118 GP_SSP_2_CLK",    M0   ,    GPI   , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0238,  NORTHWEST),//SPI_2_CLK_GPIO_118
  BXT_GPIO_PAD_CONF(L"GPIO_119 GP_SSP_2_FS0",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0240,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_120 GP_SSP_2_FS1",    M0   ,    GPO   , NA   ,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0248,  NORTHWEST),//SPI_2_FS1_GPIO_120
  BXT_GPIO_PAD_CONF(L"GPIO_121 GP_SSP_2_FS2",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0250,  NORTHWEST),//SPI_2_FS2_GPIO_121
  BXT_GPIO_PAD_CONF(L"GPIO_122 GP_SSP_2_RXD",    M0   ,    GPO   ,GPIO_D,  LO     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0258,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_123 GP_SSP_2_TXD",    M0   ,    GPI   ,GPIO_D,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,     NA,  GPIO_PADBAR+0x0260,  NORTHWEST),//SPI_2_TX_GPIO_123
};

// West Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_W_OEM [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,     MMIO_Offset    , Community */
  BXT_GPIO_PAD_CONF(L"GPIO_124 LPSS_I2C0_SDA",   M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  , HizRx1I  ,   EnPu,  GPIO_PADBAR+0x0000,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_125 LPSS_I2C0_SCL",   M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  , HizRx1I  ,   EnPu,  GPIO_PADBAR+0x0008,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_126 LPSS_I2C1_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0010,  WEST),//I2C_1_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_127 LPSS_I2C1_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0018,  WEST),//I2C_1_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_128 LPSS_I2C2_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0020,  WEST),//I2C_2_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_129 LPSS_I2C2_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0028,  WEST),//I2C_2_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_130 LPSS_I2C3_SDA",   M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0030,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_131 LPSS_I2C3_SCL",   M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0038,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_132 LPSS_I2C4_SDA",   M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0040,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_133 LPSS_I2C4_SCL",   M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0048,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_134 ISH_I2C0_SDA",    M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0050,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_135 ISH_I2C0_SCL",    M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  , D1RxDRx1I,   EnPu,  GPIO_PADBAR+0x0058,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_136 ISH_I2C1_SDA",    M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  , D0RxDRx0I,   EnPu,  GPIO_PADBAR+0x0060,  WEST),//I2C_6_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_137 ISH_I2C1_SCL",    M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  , D0RxDRx0I,   EnPu,  GPIO_PADBAR+0x0068,  WEST),//I2C_6_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_138 LPSS_I2C7_SDA",   M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,TxDRxE    ,   NA  ,  GPIO_PADBAR+0x0070,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_139 LPSS_I2C7_SCL",   M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,TxDRxE    ,   NA  ,  GPIO_PADBAR+0x0078,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_146 ISH_GPIO_0",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0080,  WEST), // HDA_BIT_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_147 ISH_GPIO_1",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0088,  WEST), // HDA_SYNC
  BXT_GPIO_PAD_CONF(L"GPIO_148 ISH_GPIO_2",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0090,  WEST), // HDA_SDIN0
  BXT_GPIO_PAD_CONF(L"GPIO_149 ISH_GPIO_3",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0098,  WEST), // HDA_SDOUT
  BXT_GPIO_PAD_CONF(L"GPIO_150 ISH_GPIO_4",      M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,   NA  ,  GPIO_PADBAR+0x00A0,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_151 ISH_GPIO_5",      M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  ,TxDRxE    ,   NA  ,  GPIO_PADBAR+0x00A8,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_152 ISH_GPIO_6",      M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  ,TxDRxE    ,   NA  ,  GPIO_PADBAR+0x00B0,  WEST),//MEM_STRAP_0
/* Below GPIO Pin is programed in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_152 ISH_GPIO_6",      M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00B0,  WEST),//Feature: Reset                  Net in Sch: SOC_PCIE_SLOT1_RST_N,
*/
  BXT_GPIO_PAD_CONF(L"GPIO_153 ISH_GPIO_7",      M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  ,TxDRxE    ,   NA  ,  GPIO_PADBAR+0x00B8,  WEST),//MEM_STRAP_1
  BXT_GPIO_PAD_CONF(L"GPIO_154 ISH_GPIO_8",      M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,Wake_Disabled, P_NONE ,   NA    ,    NA  ,TxDRxE    ,   NA  ,  GPIO_PADBAR+0x00C0,  WEST),//MEM_STRAP_2
  BXT_GPIO_PAD_CONF(L"GPIO_155 ISH_GPIO_9",      M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,   NA  ,  GPIO_PADBAR+0x00C8,  WEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA   ,   LO    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,NA        ,   NA  ,  GPIO_PADBAR+0x00D8,  WEST),//PCIE_CLKREQ1#

  BXT_GPIO_PAD_CONF(L"GPIO_209 PCIE_CLKREQ0_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00D0,  WEST),// WLAN_CLKREQ#
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00D8,  WEST),// LAN_CLKREQ1#
  BXT_GPIO_PAD_CONF(L"GPIO_211 PCIE_CLKREQ2_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00E0,  WEST),// PCIE_CLKREQ2#
  BXT_GPIO_PAD_CONF(L"GPIO_212 PCIE_CLKREQ3_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00E8,  WEST),// PCIE_CLKREQ3#

  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_0",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00F0,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_1",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00F8,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_2",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0100,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_3",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0108,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_4",            M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0110,  WEST),//Not used on RVP

  BXT_GPIO_PAD_CONF(L"PMU_AC_PRESENT",           M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0118,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_BATLOW_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0120,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_PLTRST_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0128,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_PWRBTN_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0130,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_RESETBUTTON_B",        M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0138,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S0_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0140,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S3_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0148,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S4_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0150,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SUSCLK",               M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0158,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_WAKE_B",               M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0160,  WEST),//[COM] WAKE#
  BXT_GPIO_PAD_CONF(L"SUS_STAT_B",               M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0168,  WEST),
  BXT_GPIO_PAD_CONF(L"SUSPWRDNACK",              M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0170,  WEST),
};

 // South West Community
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_SW_OEM []=
{
  //                  Group Pin#:  pad_name,       PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,   MMIO_Offset     ,  Community */
  BXT_GPIO_PAD_CONF(L"GPIO_205 PCIE_WAKE0_B",      M0,     GPO   ,GPIO_D,  HI     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0000 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_206 PCIE_WAKE1_B",      M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0008 ,  SOUTHWEST),//LAN_WAKE#_SOC
  BXT_GPIO_PAD_CONF(L"GPIO_207 PCIE_WAKE2_B",      M0,     GPO   ,GPIO_D,  HI     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0010 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_208 PCIE_WAKE3_B",      M0,     GPO   ,GPIO_D,  HI     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I   , SAME,GPIO_PADBAR+0x0018 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_156 EMMC0_CLK",         M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I,  SAME,GPIO_PADBAR+0x0020 ,  SOUTHWEST),//EMMC_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_157 EMMC0_D0",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0028 ,  SOUTHWEST),//EMMC_D0
  BXT_GPIO_PAD_CONF(L"GPIO_158 EMMC0_D1",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0030 ,  SOUTHWEST),//EMMC_D1
  BXT_GPIO_PAD_CONF(L"GPIO_159 EMMC0_D2",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0038 ,  SOUTHWEST),//EMMC_D2
  BXT_GPIO_PAD_CONF(L"GPIO_160 EMMC0_D3",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0040 ,  SOUTHWEST),//EMMC_D3
  BXT_GPIO_PAD_CONF(L"GPIO_161 EMMC0_D4",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0048 ,  SOUTHWEST),//EMMC_D4
  BXT_GPIO_PAD_CONF(L"GPIO_162 EMMC0_D5",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0050 ,  SOUTHWEST),//EMMC_D5
  BXT_GPIO_PAD_CONF(L"GPIO_163 EMMC0_D6",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0058 ,  SOUTHWEST),//EMMC_D6
  BXT_GPIO_PAD_CONF(L"GPIO_164 EMMC0_D7",          M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0060 ,  SOUTHWEST),//EMMC_D7
  BXT_GPIO_PAD_CONF(L"GPIO_165 EMMC0_CMD",         M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  SAME,GPIO_PADBAR+0x0068 ,  SOUTHWEST),//EMMC_CMD
  BXT_GPIO_PAD_CONF(L"GPIO_166 SDIO_CLK",          M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE   ,  NA  ,GPIO_PADBAR+0x0070 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_167 SDIO_D0",           M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE   ,  NA  ,GPIO_PADBAR+0x0078 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_168 SDIO_D1",           M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE   ,  NA  ,GPIO_PADBAR+0x0080 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_169 SDIO_D2",           M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE   ,  NA  ,GPIO_PADBAR+0x0088 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_170 SDIO_D3",           M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE   ,  NA  ,GPIO_PADBAR+0x0090 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_171 SDIO_CMD",          M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,TxDRxE   ,  NA  ,GPIO_PADBAR+0x0098 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_172 SDCARD_CLK",        M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00A0 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_173 SDCARD_D0",         M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00B0 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_174 SDCARD_D1",         M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00B8 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_175 SDCARD_D2",         M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00C0 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_176 SDCARD_D3",         M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00C8 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_177 SDCARD_CD_B",       M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00D0 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_178 SDCARD_CMD",        M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00D8 ,  SOUTHWEST),//NC
//BXT_GPIO_PAD_CONF(L"GPIO_179 SDCARD_CLK_FB",     M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx1I  ,  NA  ,GPIO_PADBAR+0x00A8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_186 SDCARD_LVL_WP",     M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,  NA  ,GPIO_PADBAR+0x00E0 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_182 EMMC0_STROBE",      M1,     NA    ,   NA ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,  SAME,GPIO_PADBAR+0x00E8 ,  SOUTHWEST),//EMMC_RCLK
  BXT_GPIO_PAD_CONF(L"GPIO_183 SDIO_PWR_DOWN_B",   M0,     GPO   ,GPIO_D,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,  NA  ,GPIO_PADBAR+0x00F0 ,  SOUTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"SMB_ALERTB",                 M0,     GPI   , GPIO_D ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x00F8 ,  SOUTHWEST),//not used on RVP
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0100 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0108 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0110 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0118 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0120 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0128 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0130 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0138 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0140 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0148 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME   ,GPIO_PADBAR+0x0150 ,  SOUTHWEST),
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
/*
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
*/
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
/*
};*/
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
