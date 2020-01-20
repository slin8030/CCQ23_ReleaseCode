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

#ifndef _BOARDGPIOS_BFH_H_
#define _BOARDGPIOS_BFH_H_

//[-start-161108-IB07400810-add]//
#ifdef GPIO_INIT_IN_PEI_PHASE_POST_MEMORY
//=======================================================//
//                                                       //
// PEI phase Post-Memory GPIO init                       //
//                                                       //
//=======================================================//
//[-end-161108-IB07400810-add]//

// North Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_N_BFH_IVI[] =
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_N_OEM[] =
#endif
{
 /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0028,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0030,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0038,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0040,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0048,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_10",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0050,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0058,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0060,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0068,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0070,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   ,  NA   ,   LO    ,   NA      , Wake_Disabled,  P_NONE ,   NA    ,    NA,NA        ,  NA  ,    GPIO_PADBAR+0x0078,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0080,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0088,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_18",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0090,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_19",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0098,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_20",                  M0   ,    GPO   ,  NA   ,   LO    ,   NA      , Wake_Disabled,  P_20K_H ,   NA    ,    NA,NA        ,  NA  ,    GPIO_PADBAR+0x00A0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_21",                  M0   ,    GPO   ,  NA   ,   LO    ,   NA      , Wake_Disabled,  P_20K_H ,   NA    ,    NA,NA        ,  NA  ,    GPIO_PADBAR+0x00A8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_22",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00B0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_23",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00B8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_24",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00C0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_25",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00C8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_26",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00D0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_27",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00D8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_28",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00E0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_29",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00E8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_30",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00F0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_31",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x00F8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_32",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0100,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_33",                  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0108,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_34 PWM0",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0110,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_35 PWM1",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0118,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_36 PWM2",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0120,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_37 PWM3",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0128,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_38 LPSS_UART0_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0130,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_39 LPSS_UART0_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0138,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_40 LPSS_UART0_RTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0140,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_41 LPSS_UART0_CTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0148,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_42 LPSS_UART1_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0150,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_43 LPSS_UART1_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0158,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_44 LPSS_UART1_RTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0160,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_45 LPSS_UART1_CTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0168,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0170,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0178,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_48 LPSS_UART2_RTS_B", M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0180,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_49 LPSS_UART2_CTS_B", M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0188,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_62 GP_CAMERASB00",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0190,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_63 GP_CAMERASB01",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0198,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_64 GP_CAMERASB02",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01A0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_65 GP_CAMERASB03",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01A8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_66 GP_CAMERASB04",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01B0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_67 GP_CAMERASB05",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01B8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_68 GP_CAMERASB06",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01C0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_69 GP_CAMERASB07",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01C8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_70 GP_CAMERASB08",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01D0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_71 GP_CAMERASB09",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01D8,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_72 GP_CAMERASB10",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01E0,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_73 GP_CAMERASB11",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x01E8,  NORTH),
  BXT_GPIO_PAD_CONF(L"TCK",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME,    GPIO_PADBAR+0x01F0,  NORTH),
  BXT_GPIO_PAD_CONF(L"TRST_B",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME,    GPIO_PADBAR+0x01F8,  NORTH),
  BXT_GPIO_PAD_CONF(L"TMS",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME,    GPIO_PADBAR+0x0200,  NORTH),
  BXT_GPIO_PAD_CONF(L"TDI",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME,    GPIO_PADBAR+0x0208,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PMODE",                 M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0210,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PREQ_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   SAME,    GPIO_PADBAR+0x0218,  NORTH),
  BXT_GPIO_PAD_CONF(L"JTAGX",                    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0220,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PRDY_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I,   SAME,    GPIO_PADBAR+0x0228,  NORTH),
  BXT_GPIO_PAD_CONF(L"TDO",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME,    GPIO_PADBAR+0x0230,  NORTH),
  BXT_GPIO_PAD_CONF(L"CNV_BRI_DT",               M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0238,  NORTH),
  BXT_GPIO_PAD_CONF(L"CNV_BRI_RSP",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0240,  NORTH),
  BXT_GPIO_PAD_CONF(L"CNV_RGI_DT",               M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0248,  NORTH),
  BXT_GPIO_PAD_CONF(L"CNV_RGI_RSP",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,    GPIO_PADBAR+0x0250,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_ALERT_B",            M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0258,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_DATA",               M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0260,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_CLK",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I,   EnPd,    GPIO_PADBAR+0x0268,  NORTH),
};


// North West Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_NW_BFH_IVI [] =
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_NW_OEM[] =
#endif
{
 //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,    MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_187 HV_DDI0_DDC_SDA", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0000,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_188 HV_DDI0_DDC_SCL", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0008,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_189 HV_DDI1_DDC_SDA", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0010,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_190 HV_DDI1_DDC_SCL", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0018,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_191 DBI_SDA",         M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0020,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_192 DBI_SCL",         M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0028,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_193 PANEL0_VDDEN",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0030,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_194 PANEL0_BKLTEN",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0038,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_195 PANEL0_BKLTCTL",  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0040,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_196 PANEL1_VDDEN",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0048,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_197 PANEL1_BKLTEN",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0050, NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_198 PANEL1_BKLTCTL",  M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0058,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_199 DBI_CSX",         M2   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0060,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_200 DBI_RESX",        M2   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0068,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_201 GP_INTD_DSI_TE1", M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0070,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_202 GP_INTD_DSI_TE2", M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0078,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_203 USB_OC0_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0080,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_204 USB_OC1_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,EnPd  ,     GPIO_PADBAR+0x0088,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS0",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0090,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS1",              M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,  SAME,  GPIO_PADBAR+0x0098,  NORTHWEST),   //to solve GOP hang at postcode 96h
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS2",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00A0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_RXD",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00A8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_TXD",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00B0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMC_SPI_CLK",              M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00B8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_PWRGOOD",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_NONE,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00C0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_223 PMIC_RESET_B",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_NONE,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00C8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_213 PMIC_SDWN_B",     M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_NONE,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00D0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_214 PMIC_BCUDISW2",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00D8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_215 PMIC_BCUDISCRIT", M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00E0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_THERMTRIP_B",         M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked   ,SAME  ,     GPIO_PADBAR+0x00E8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_224 PMIC_STDBY",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00F0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PROCHOT_B",                M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx1I   ,  SAME,     GPIO_PADBAR+0x00F8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SCL",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0100,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SDA",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0108,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_74 AVS_I2S1_MCLK",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0110,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_75 AVS_I2S1_BCLK",    M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0118,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_76 AVS_I2S1_WS_SYNC", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0120,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_77 AVS_I2S1_SDI",     M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0128,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_78 AVS_I2S1_SDO",     M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_NONE,    NA   ,    NA  ,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0130,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_79 AVS_M_CLK_A1",     M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0138,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_80 AVS_M_CLK_B1",     M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0140,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_81 AVS_M_DATA_1",     M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0148,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_82 AVS_M_CLK_AB2",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0150,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_83 AVS_M_DATA_2",     M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0158,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_84 AVS_I2S2_MCLK",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0160,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_85 AVS_I2S2_BCLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0168,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_86 AVS_I2S2_WS_SYNC", M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0170,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_87 AVS_I2S2_SDI",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0178,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_88 AVS_I2S2_SDO",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0180,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_89 AVS_I2S3_BCLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0188,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_80 AVS_I2S3_WS_SYNC", M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0190,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_91 AVS_I2S3_SDI",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0198,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_92 AVS_I2S3_SDO",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x01A0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_97 FST_SPI_CS0_B",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01A8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_98 FST_SPI_CS1_B",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01B0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_99 FST_SPI_MOSI_IO0", M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01B8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_100 FST_SPI_MISO_IO1",M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01C0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_101 FST_SPI_IO2",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01C8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_102 FST_SPI_IO3",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01D0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_103 FST_SPI_CLK",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, Native_control,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x01D8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_104 GP_SSP_0_CLK",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x01E8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_105 GP_SSP_0_FS0",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x01F0,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_106 GP_SSP_0_FS1",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x01F8,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_109 GP_SSP_0_RXD",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0200,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_110 GP_SSP_0_TXD",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0208,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_111 GP_SSP_1_CLK",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0210,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_112 GP_SSP_1_FS0",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0218,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_113 GP_SSP_1_FS1",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0220,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0228,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_117 GP_SSP_1_TXD",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0230,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_118 GP_SSP_2_CLK",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0238,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_119 GP_SSP_2_FS0",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0240,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_120 GP_SSP_2_FS1",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0248,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_121 GP_SSP_2_FS2",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0250,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_122 GP_SSP_2_RXD",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0258,  NORTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_123 GP_SSP_2_TXD",    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0260,  NORTHWEST),
};

// West Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_W_BFH_IVI [] =
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_W_OEM[] =
#endif
{
 //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_124 LPSS_I2C0_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPd,     GPIO_PADBAR+0x0000,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_125 LPSS_I2C0_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0008,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_126 LPSS_I2C1_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0010,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_127 LPSS_I2C1_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0018,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_128 LPSS_I2C2_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0020,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_129 LPSS_I2C2_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0028,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_130 LPSS_I2C3_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0030,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_131 LPSS_I2C3_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0038,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_132 LPSS_I2C4_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0040,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_133 LPSS_I2C4_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, HizRx0I,  EnPu,     GPIO_PADBAR+0x0048,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_134 LPSS_I2C5_SDA",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0050,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_135 LPSS_I2C5_SCL",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0058,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_136 LPSS_I2C6_SDA",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0060,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_137 LPSS_I2C6_SCL",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0068,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_138 LPSS_I2C7_SDA",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0070,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_139 LPSS_I2C7_SCL",   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0078,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_146 ISH_GPIO_0",      M0,     GPO   ,   NA   ,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,NA    ,     GPIO_PADBAR+0x0080,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_147 ISH_GPIO_1",      M0,     GPO   ,   NA   ,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,NA    ,     GPIO_PADBAR+0x0088,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_148 ISH_GPIO_2",      M0,     GPO   ,   NA   ,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,NA    ,     GPIO_PADBAR+0x0090,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_149 ISH_GPIO_3",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0098,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_150 ISH_GPIO_4",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00A0,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_151 ISH_GPIO_5",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00A8,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_152 ISH_GPIO_6",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00B0,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_153 ISH_GPIO_7",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00B8,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_154 ISH_GPIO_8",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00C0,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_155 ISH_GPIO_9",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00C8,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_209 PCIE_CLKREQ0_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA, HizRx0I,  EnPd,     GPIO_PADBAR+0x00D0,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA, HizRx0I,  EnPd,     GPIO_PADBAR+0x00D8,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_211 PCIE_CLKREQ2_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA, HizRx0I,  EnPd,     GPIO_PADBAR+0x00E0,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_212 PCIE_CLKREQ3_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA, HizRx0I,  EnPd,     GPIO_PADBAR+0x00E8,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_0",            M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00F0,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_1",            M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x00F8,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_2",            M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0100,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_3",            M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0108,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_4",            M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0110,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_AC_PRESENT",           M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0118,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_BATLOW_B",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0120,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_PLTRST_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,   Wake_Disabled, P_NONE,   NA    ,    NA,IOS_Masked,  NA  ,     GPIO_PADBAR+0x0128,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_PWRBTN_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0130,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_RESETBUTTON_B",        M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0138,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S0_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,   Wake_Disabled, P_NONE,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0140,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S3_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,   Wake_Disabled, P_NONE,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0148,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S4_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,   Wake_Disabled, P_NONE,   NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0150,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SUSCLK",               M1   ,    NA    , NA   ,   NA    ,   NA      ,   Wake_Disabled, P_NONE,   NA    ,    NA,HizRx0I   ,  EnPd,     GPIO_PADBAR+0x0158,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_WAKE_B",               M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0160,  WEST),
  BXT_GPIO_PAD_CONF(L"SUS_STAT_B",               M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,     GPIO_PADBAR+0x0168,  WEST),
  BXT_GPIO_PAD_CONF(L"SUSPWRDNACK",              M1   ,    NA    , NA   ,   NA    ,   NA      ,   Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked   ,  EnPd,     GPIO_PADBAR+0x0170,  WEST),
};

// South West Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_SW_BFH_IVI[]=
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_SW_OEM[] =
#endif
{
 //                  Group Pin#:  pad_name,       PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae, IOSTerm,MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_205 PCIE_WAKE0_B",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0000 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_206 PCIE_WAKE1_B",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0008 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_207 PCIE_WAKE2_B",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0010 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_208 PCIE_WAKE3_B",      M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0018 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_156 EMMC0_CLK",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0020 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_157 EMMC0_D0",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0028 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_158 EMMC0_D1",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0030 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_159 EMMC0_D2",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0038 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_160 EMMC0_D3",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0040 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_161 EMMC0_D4",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0048 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_162 EMMC0_D5",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0050 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_163 EMMC0_D6",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0058 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_164 EMMC0_D7",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0060 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_165 EMMC0_CMD",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0068 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_166 SDIO_CLK",          M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0070 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_167 SDIO_D0",           M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0078 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_168 SDIO_D1",           M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0080 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_169 SDIO_D2",           M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0088 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_170 SDIO_D3",           M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0090 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_171 SDIO_CMD",          M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0098 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_172 SDCARD_CLK",        M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00A0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_173 SDCARD_D0",         M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00B0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_174 SDCARD_D1",         M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00B8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_175 SDCARD_D2",         M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00C0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_176 SDCARD_D3",         M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00C8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_177 SDCARD_CD_B",       M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00D0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_178 SDCARD_CMD",        M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00D8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_186 SDCARD_LVL_WP",     M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00E0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_182 EMMC0_STROBE",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x00E8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_183 SDIO_PWR_DOWN_B",   M0,     GPO   ,   NA   ,  HI     ,   NA       ,Wake_Disabled, P_NONE,    NA   ,    NA  ,HizRx0I  ,NA    ,   GPIO_PADBAR+0x00F0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_ALERTB",                 M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x00F8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0100 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0108 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0110 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA   ,  NA     ,   NA       , Wake_Disabled, P_NONE,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0118 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0120 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0128 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0130 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0138 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0140 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M0  ,     GPI   ,GPIO_D,  NA     ,   Level,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,   GPIO_PADBAR+0x0148 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx0I  ,EnPd  ,   GPIO_PADBAR+0x0150 ,  SOUTHWEST),
};

//[-start-161108-IB07400810-add]//
#endif
//[-end-161108-IB07400810-add]//
#endif
