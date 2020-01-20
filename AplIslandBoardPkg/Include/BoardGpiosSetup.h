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

#ifndef _BOARDGPIOS_SETUP_H_
#define _BOARDGPIOS_SETUP_H_

BXT_GPIO_PAD_INIT  mBXT_GpioInitData_Audio_SSP6 []=
{
  //                  Group Pin#:  pad_name,       PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae, IOSTerm,MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_146 ISH_GPIO_0",      M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0080,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_147 ISH_GPIO_1",      M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0088,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_148 ISH_GPIO_2",      M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0090,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_149 ISH_GPIO_3",      M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0098,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_84 AVS_I2S2_MCLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0160,  NORTHWEST),//Spare signal, set to GPI.  Net in Sch:HDA_RSTB
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M0   ,    GPI   , NA   ,   NA    ,   Level   ,  Wake_Disabled, P_20K_H,   NA    ,IOAPIC,TxDRxE    ,  NA  ,  GPIO_PADBAR+0x0228,  NORTHWEST),//Feature: Interrput          Net in Sch: SOC_CODEC_IRQ
  BXT_GPIO_PAD_CONF(L"GPIO_192 DBI_SCL",         M0  ,     HI_Z  ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled,  P_2K_H,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0028,  NORTHWEST),//Feature: Codec Power Down PD Net in Sch: SOC_CODEC_PD_N
};

// GPIO 191 is only used if EPI reworks are applied on the board. This GPIO switches between SD Card data (if set to 1) and EPI data (if set to 0).
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_EPI_Override[] =
{
  //                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,     MMIO_Offset  ,Community
  BXT_GPIO_PAD_CONF(L"GPIO_191 DBI_SDA",        M0,   GPO,        GPIO_D,LO,       NA,           Wake_Disabled,P_20K_L, NA,       NA,       NA,      NA,          GPIO_PADBAR + 0x0020, NORTHWEST),//Feature: SD_I2C MUX SEL     Net in Sch: INA_MUX_SEL
};

BXT_GPIO_PAD_INIT  mBXT_GpioInitData_LPSS_I2C[] =
{
  BXT_GPIO_PAD_CONF(L"GPIO_134 LPSS_I2C5_SDA",   M1   ,    NA   ,  NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,  HizRx0I,  EnPd,    GPIO_PADBAR+0x0050,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_135 LPSS_I2C5_SCL",   M1   ,    NA   ,  NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,  HizRx0I,  EnPd,    GPIO_PADBAR+0x0058,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_136 LPSS_I2C6_SDA",   M1   ,    NA   ,  NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,  HizRx0I,  EnPd,    GPIO_PADBAR+0x0060,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_137 LPSS_I2C6_SCL",   M1   ,    NA   ,  NA    ,  NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,  HizRx0I,  EnPd,    GPIO_PADBAR+0x0068,  WEST),
};

BXT_GPIO_PAD_INIT  LomDisableGpio[] =
{
  // LAN
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D8,  WEST),  // CLKREQ#
  BXT_GPIO_PAD_CONF(L"GPIO_37 PWM3",             M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0128,  NORTH), // PERST#
};

//[-start-160816-IB07400771-add]//
//[-start-170518-IB07400867-modify]//
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_GPIO_177 []=
{
  BXT_GPIO_PAD_CONF(L"GPIO_177 SDCARD_CD_B",       M0,     GPI   ,   GPIO_D   ,  NA     ,   Edge     ,Wake_Disabled, P_NONE,     NA  ,    NA  ,TxDRxE,   NA, GPIO_PADBAR+ 0x00D0 , SOUTHWEST),
};
//[-end-170518-IB07400867-modify]//

// North peak GPIO
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_Trace[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,     MMIO_Offset  ,Community */
/*  Below pins are always programmed before MRC in BoaardGpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0028,  NORTH),//Mux with CSE_PG based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0030,  NORTH),//Mux with DISP1_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0038,  NORTH),//Mux with DISP1_TOUCH_INT_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0040,  NORTH),//Mux with DISP1_TOUCH_RST_N based on the SW3 switch
*/
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_L, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0048,  NORTH),//Feature:Interrupt              Net in Sch: SPI_TPM_HDR_IRQ_N
  BXT_GPIO_PAD_CONF(L"GPIO_10",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Enabled , P_20K_L, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0050,  NORTH),//Feature:WAKE                   Net in Sch: SLTA_SDIO_WAKE_N
//[-start-160906-IB03090434-modify]//
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0   ,    GPI   , ACPI_D,  NA    ,   Edge     , Wake_Disabled, P_20K_L, Inverted,   SCI,   TxDRxE ,     NA, GPIO_PADBAR+0x0058,  NORTH),//Feature:Runtime SCI            Net in Sch: SOC_RUNTIME_SCI_N
//[-end-160906-IB03090434-modify]//
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_L, Inverted,   SCI,   TxDRxE ,     NA, GPIO_PADBAR+0x0060,  NORTH),//Feature:Wake SCI               Net in Sch: EC_WAKE_SCI_N
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0068,  NORTH),//Feature:Reset                  Net in Sch: SOC_PCIE_SLOT2_RST_N
*/
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M0   ,    GPI   , GPIO_D,  NA    ,   Edge     , Wake_Disabled, P_20K_L, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0070,  NORTH),//Feature:Interrupt              Net in Sch: FGR_INT
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0078,  NORTH),//Feature:Reset                  Net in Sch: WIFI_RST_GPIO_N
*/
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_H, Inverted,IOAPIC,  HizRx0I ,DisPuPd, GPIO_PADBAR+0x0080,  NORTH),//Feature:SIM Card Detect        Net in Sch: SIM_CON_CD1, falling edge trigger
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0088,  NORTH),//Feature:Power Enable           Net in Sch: PCIE_SLOT2_PWREN
*/
};

BXT_GPIO_PAD_INIT  mBXT_GpioInitData_N_RVP2[] =
{
  //                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,     MMIO_Offset  ,Community
  BXT_GPIO_PAD_CONF(L"GPIO_33",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_L,   NA    ,IOAPIC,IOS_Masked,   SAME, GPIO_PADBAR+0x0108,  NORTH),//Feature: Interrput          Net in Sch: PMIC_IRQ_R_N
};

BXT_GPIO_PAD_INIT  mBXT_GpioInitData_FAB2[] =
{
  //                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,     MMIO_Offset  ,Community
  BXT_GPIO_PAD_CONF(L"GPIO_76 AVS_I2S1_WS_SYNC", M0   ,    GPI   ,GPIO_D,   NA    ,   Level   ,  Wake_Disabled, P_20K_L,   NA    ,IOAPIC,    TxDRxE,    NA,  GPIO_PADBAR+0x0120,  NORTHWEST),//Feature:SSIC_WWAN_Wake
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M0   ,    GPI   ,GPIO_D,   NA    ,   Level    , Wake_Disabled, P_20K_L,   NA    ,IOAPIC,    TxDRxE,    NA,  GPIO_PADBAR+0x0030,  NORTH),    //Feature:DGPU Power OK
  BXT_GPIO_PAD_CONF(L"GPIO_217 CNV_BRI_RSP",     M0   ,    GPO   ,GPIO_D,   LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,        NA,    NA,  GPIO_PADBAR+0x0240,  NORTH),    //Feature:DGPU_SEL
};
//[-end-160816-IB07400771-add]//

#endif
