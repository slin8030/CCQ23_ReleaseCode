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

#ifndef _BOARDGPIOS_RVVP_H_
#define _BOARDGPIOS_RVVP_H_

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
BXT_GPIO_PAD_INIT  mBXT_RVVP_GpioInitData_N[] =
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_N_OEM[] =
#endif
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,     MMIO_Offset  ,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0028,  NORTH),//Mux with CSE_PG based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0030,  NORTH),//Mux with DISP1_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0038,  NORTH),//Mux with DISP1_TOUCH_INT_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME, GPIO_PADBAR+0x0040,  NORTH),//Mux with DISP1_TOUCH_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_L, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0048,  NORTH),//Feature:Interrupt              Net in Sch: SPI_TPM_HDR_IRQ_N
  BXT_GPIO_PAD_CONF(L"GPIO_10",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Enabled , P_20K_L,   NA    ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x0050,  NORTH),//Feature:WAKE                   Net in Sch: SLTA_SDIO_WAKE_N
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_L, Inverted,   SCI,   TxDRxE ,     NA, GPIO_PADBAR+0x0058,  NORTH),//Feature:Runtime SCI            Net in Sch: SOC_RUNTIME_SCI_N
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_L, Inverted,   SCI,   TxDRxE ,     NA, GPIO_PADBAR+0x0060,  NORTH),//Feature:Wake SCI               Net in Sch: EC_WAKE_SCI_N
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0068,  NORTH),//Feature:Reset                  Net in Sch: SOC_PCIE_SLOT2_RST_N
*/
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_L, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0070,  NORTH),//Feature:Interrupt              Net in Sch: FGR_INT
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0078,  NORTH),//Feature:Reset                  Net in Sch: WIFI_RST_GPIO_N
*/
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_H, Inverted,IOAPIC,   TxDRxE ,     NA, GPIO_PADBAR+0x0080,  NORTH),//Feature:SIM Card Detect        Net in Sch: SIM_CON_CD1, falling edge trigger
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0088,  NORTH),//Feature:Power Enable           Net in Sch: PCIE_SLOT2_PWREN
*/
  BXT_GPIO_PAD_CONF(L"GPIO_18",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H, Inverted ,IOAPIC,  TxDRxE  ,     NA, GPIO_PADBAR+0x0090,  NORTH),//Feature:Interrupt              Net in Sch: TCHPAD_INT_N
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_19",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0098,  NORTH),//Feature:Power Enable           Net in Sch: PCIE_SLOT1_SWTCH_GFX_PWREN
*/
  BXT_GPIO_PAD_CONF(L"GPIO_20",                  M0   ,    GPI   ,  NA   ,  NA    ,   Edge     , Wake_Disabled, P_20K_L,   NA    ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x00A0,  NORTH),//Feature:Interrupt              Net in Sch: NFC_IRQ_CONN
  BXT_GPIO_PAD_CONF(L"GPIO_21",                  M0   ,    GPI   ,  NA   ,  NA    ,   Level    , Wake_Disabled, P_20K_H,   NA    ,IOAPIC,    TxDRxE,     NA, GPIO_PADBAR+0x00A8,  NORTH),//Feature:Interrupt              Net in Sch: TCH_PNL_INTR_LS_N
  BXT_GPIO_PAD_CONF(L"GPIO_22",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled,  P_NONE,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B0,  NORTH),//Feature:Power Enable           Net in Sch: SATA_ODD_PWRGT_R
  BXT_GPIO_PAD_CONF(L"GPIO_23",                  M0   ,    GPI   , GPIO_D,  NA    ,   Level    , Wake_Disabled, P_20K_L,Inverted ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B8,  NORTH),//Feature:Present Pin            Net in Sch: SATA_ODD_PRSNT_N
  BXT_GPIO_PAD_CONF(L"GPIO_24",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00C0,  NORTH),//SATA_DEVSLP0
  BXT_GPIO_PAD_CONF(L"GPIO_25",                  M0   ,    GPI   , ACPI_D,  NA    , BothEdge   , Wake_Disabled, P_20K_L,No_invert,   SCI,     NA   ,     NA, GPIO_PADBAR+0x00C8,  NORTH),//Feature:ODD MD/DA SCI          Net in Sch: SATA_ODD_DA_IN
  BXT_GPIO_PAD_CONF(L"GPIO_26",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D0,  NORTH),//SATA_LEDN
  BXT_GPIO_PAD_CONF(L"GPIO_27",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D8,  NORTH),//Feature:DFU                    Net in Sch: NFC_DFU
  BXT_GPIO_PAD_CONF(L"GPIO_28",                  M2   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00E0,  NORTH),//                               Net in Sch: ISH_GPIO10
  BXT_GPIO_PAD_CONF(L"GPIO_29",                  M2   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00E8,  NORTH),//                               Net in Sch: ISH_GPIO11
  BXT_GPIO_PAD_CONF(L"GPIO_30",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00F0,  NORTH),//                               Net in Sch: ISH_GPIO12
  BXT_GPIO_PAD_CONF(L"GPIO_31",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x00F8,  NORTH),//                               Net in Sch: SUSCLK1
  BXT_GPIO_PAD_CONF(L"GPIO_32",                  M5   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0100,  NORTH),//                               Net in Sch: SUSCLK2
  BXT_GPIO_PAD_CONF(L"GPIO_33",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0108,  NORTH),//                               Net in Sch: ISH_GPIO15, SUSCLK3
  BXT_GPIO_PAD_CONF(L"GPIO_34 PWM0",             M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0110,  NORTH),//Feature:Power Enable           Net in Sch: USB2_CAM_PWR_EN
  BXT_GPIO_PAD_CONF(L"GPIO_35 PWM1",             M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_5K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0118,  NORTH),//Feature:Power Enable           Net in Sch: TCH_PNL_PG
  BXT_GPIO_PAD_CONF(L"GPIO_36 PWM2",             M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_5K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0120,  NORTH),//Feature:Reset                  Net in Sch: TCH_PNL_RST_LS_N
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_37 PWM3",             M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0128,  NORTH),//Feature:Reset                  Net in Sch: LAN_RST_N
*/
  BXT_GPIO_PAD_CONF(L"GPIO_38 LPSS_UART0_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0130,  NORTH),//BT
  BXT_GPIO_PAD_CONF(L"GPIO_39 LPSS_UART0_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0138,  NORTH),//BT
  BXT_GPIO_PAD_CONF(L"GPIO_40 LPSS_UART0_RTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0140,  NORTH),//BT
  BXT_GPIO_PAD_CONF(L"GPIO_41 LPSS_UART0_CTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0148,  NORTH),//BT
  BXT_GPIO_PAD_CONF(L"GPIO_42 LPSS_UART1_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,   HizRx1I,DisPuPd, GPIO_PADBAR+0x0150,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_43 LPSS_UART1_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0158,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_44 LPSS_UART1_RTS_B", M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,DisPuPd, GPIO_PADBAR+0x0160,  NORTH),// Audio reset
  BXT_GPIO_PAD_CONF(L"GPIO_45 LPSS_UART1_CTS_B", M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0168,  NORTH),//
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0170,  NORTH),//DIGITIZER_DET
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0178,  NORTH),//DIGITIZER_RESET
  BXT_GPIO_PAD_CONF(L"GPIO_48 LPSS_UART2_RTS_B", M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0180,  NORTH),//Not used on RVP
  BXT_GPIO_PAD_CONF(L"GPIO_49 LPSS_UART2_CTS_B", M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0188,  NORTH),//Feature:SOC_EXTSMI_N           Net in Sch: SOC_EXTSMI_N
  BXT_GPIO_PAD_CONF(L"GPIO_62 GP_CAMERASB00",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x0190,  NORTH),//CAM_FLASH_RST_N
  BXT_GPIO_PAD_CONF(L"GPIO_63 GP_CAMERASB01",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x0198,  NORTH),//CAM_FLASH_TORCH
  BXT_GPIO_PAD_CONF(L"GPIO_64 GP_CAMERASB02",    M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01A0,  NORTH),//Camera TODO: Need to check with Driver team
  BXT_GPIO_PAD_CONF(L"GPIO_65 GP_CAMERASB03",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01A8,  NORTH),//Camera TODO: Need to check with Driver team
  BXT_GPIO_PAD_CONF(L"GPIO_66 GP_CAMERASB04",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01B0,  NORTH),//CAM_GRP2_RST_N
  BXT_GPIO_PAD_CONF(L"GPIO_67 GP_CAMERASB05",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01B8,  NORTH),//CAM_GRP3_RST_N
  BXT_GPIO_PAD_CONF(L"GPIO_68 GP_CAMERASB06",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01C0,  NORTH),//CAM_GRP4_RST_N
  BXT_GPIO_PAD_CONF(L"GPIO_69 GP_CAMERASB07",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01C8,  NORTH),//CAM_XENON_CHRG
  BXT_GPIO_PAD_CONF(L"GPIO_70 GP_CAMERASB08",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01D0,  NORTH),//Camera
  BXT_GPIO_PAD_CONF(L"GPIO_71 GP_CAMERASB09",    M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x01D8,  NORTH),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_72 GP_CAMERASB10",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x01E0,  NORTH),//Camera
  BXT_GPIO_PAD_CONF(L"GPIO_73 GP_CAMERASB11",    M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,     NA, GPIO_PADBAR+0x01E8,  NORTH),//CAM_SPARE
  BXT_GPIO_PAD_CONF(L"TCK",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x01F0,  NORTH),
  BXT_GPIO_PAD_CONF(L"TRST_B",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x01F8,  NORTH),
  BXT_GPIO_PAD_CONF(L"TMS",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0200,  NORTH),
  BXT_GPIO_PAD_CONF(L"TDI",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0208,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PMODE",                 M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0210,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PREQ_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0218,  NORTH),
  BXT_GPIO_PAD_CONF(L"JTAGX",                    M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0220,  NORTH),
  BXT_GPIO_PAD_CONF(L"CX_PRDY_B",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0228,  NORTH),
  BXT_GPIO_PAD_CONF(L"TDO",                      M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0230,  NORTH),
  BXT_GPIO_PAD_CONF(L"CNV_BRI_DT",               M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0238,  NORTH),//Not used on RVP
  BXT_GPIO_PAD_CONF(L"CNV_BRI_RSP",              M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0240,  NORTH),//Not used on RVP
  BXT_GPIO_PAD_CONF(L"CNV_RGI_DT",               M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0248,  NORTH),//Not used on RVP
  BXT_GPIO_PAD_CONF(L"CNV_RGI_RSP",              M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0250,  NORTH),//Not used on RVP
  BXT_GPIO_PAD_CONF(L"SVID0_ALERT_B",            M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_NONE ,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0258,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_DATA",               M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0260,  NORTH),
  BXT_GPIO_PAD_CONF(L"SVID0_CLK",                M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,   SAME, GPIO_PADBAR+0x0268,  NORTH),
};


// North West Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_RVVP_GpioInitData_NW [] =
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_NW_OEM[] =
#endif
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,     MMIO_Offset    ,  Community */
  BXT_GPIO_PAD_CONF(L"GPIO_187 HV_DDI0_DDC_SDA", M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,SAME  ,  GPIO_PADBAR+0x0000,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_188 HV_DDI0_DDC_SCL", M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,SAME  ,  GPIO_PADBAR+0x0008,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_189 HV_DDI1_DDC_SDA", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0010,  NORTHWEST),//DDI1_CTRL_DATA
  BXT_GPIO_PAD_CONF(L"GPIO_190 HV_DDI1_DDC_SCL", M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0018,  NORTHWEST),//DDI1_CTRL_CLK
  BXT_GPIO_PAD_CONF(L"GPIO_191 DBI_SDA",         M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0020,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_192 DBI_SCL",         M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0028,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_193 PANEL0_VDDEN",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0030,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_194 PANEL0_BKLTEN",   M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0038,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_195 PANEL0_BKLTCTL",  M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0040,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_196 PANEL1_VDDEN",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0048,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_197 PANEL1_BKLTEN",   M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0050,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_198 PANEL1_BKLTCTL",  M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0058,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_199 DBI_CSX",         M2  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0060,  NORTHWEST),//HDMI_HPD
  BXT_GPIO_PAD_CONF(L"GPIO_200 DBI_RESX",        M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0068,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_201 GP_INTD_DSI_TE1", M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0070,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_202 GP_INTD_DSI_TE2", M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0078,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_203 USB_OC0_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME  ,  GPIO_PADBAR+0x0080,  NORTHWEST),//HOST_USB_OC_N
  BXT_GPIO_PAD_CONF(L"GPIO_204 USB_OC1_B",       M1  ,     NA    , NA   ,  NA     ,   NA      ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked,SAME  ,  GPIO_PADBAR+0x0088,  NORTHWEST),//OTG_USB_OC_N, PMIC_GPIO0_P0_UIBT
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS0",              M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,Last_Value,  SAME,  GPIO_PADBAR+0x0090,  NORTHWEST),//PMC_FS0
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS1",              M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,Last_Value,  SAME,  GPIO_PADBAR+0x0098,  NORTHWEST),//PMC_FS1
  BXT_GPIO_PAD_CONF(L"PMC_SPI_FS2",              M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_H,   NA    ,    NA  ,Last_Value,  SAME,  GPIO_PADBAR+0x00A0,  NORTHWEST),//PMC_FS2
  BXT_GPIO_PAD_CONF(L"PMC_SPI_RXD",              M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,Last_Value,  SAME,  GPIO_PADBAR+0x00A8,  NORTHWEST),//PMC_RXD
  BXT_GPIO_PAD_CONF(L"PMC_SPI_TXD",              M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,Last_Value,  SAME,  GPIO_PADBAR+0x00B0,  NORTHWEST),//PMC_TXD
  BXT_GPIO_PAD_CONF(L"PMC_SPI_CLK",              M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_20K_L,   NA    ,    NA  ,Last_Value,  SAME,  GPIO_PADBAR+0x00B8,  NORTHWEST),//PMC_CLK
  BXT_GPIO_PAD_CONF(L"PMIC_PWRGOOD",             M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_1K_H,    NA    ,    NA  ,NA        ,    NA,  GPIO_PADBAR+0x00C0,  NORTHWEST),//PMIC_PWRGOOD             Net in Sch: GNSS_RST_N
  BXT_GPIO_PAD_CONF(L"GPIO_223 PMIC_RESET_B",    M1   ,    NA    , NA   ,   NA    ,   NA      ,Wake_Disabled, P_NONE,    NA    ,    NA  ,NA        ,  SAME,  GPIO_PADBAR+0x00C8,  NORTHWEST),//PMIC_RESET_L TODO: why Mike use L ??
  BXT_GPIO_PAD_CONF(L"GPIO_213 PMIC_SDWN_B",     M0   ,    GPI   , GPIO_D,  NA    ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x00D0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_214 PMIC_BCUDISW2",   M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x00D8,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_215 PMIC_BCUDISCRIT", M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x00E0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"PMIC_THERMTRIP_B",         M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,Last_Value,  SAME,  GPIO_PADBAR+0x00E8,  NORTHWEST),//PMIC_THERMTRIP_L TODO: what is mean by L?
  BXT_GPIO_PAD_CONF(L"GPIO_224 PMIC_STDBY",      M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x00F0,  NORTHWEST),//PMIC_STDBY
  BXT_GPIO_PAD_CONF(L"PROCHOT_B",                M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,HizRx1I   ,  SAME,  GPIO_PADBAR+0x00F8,  NORTHWEST),//PROCHOT_B
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SCL",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0100,  NORTHWEST),//PMIC I2C
  BXT_GPIO_PAD_CONF(L"PMIC_I2C_SDA",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0108,  NORTHWEST),//PMIC I2C
  BXT_GPIO_PAD_CONF(L"GPIO_74 AVS_I2S1_MCLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x0110,  NORTHWEST),//I2S1 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_75 AVS_I2S1_BCLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA, GPIO_PADBAR+0x0118,  NORTHWEST), //I2S1 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_76 AVS_I2S1_WS_SYNC", M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x0120,  NORTHWEST),//I2S1 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_77 AVS_I2S1_SDI",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x0128,  NORTHWEST),//I2S1 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_78 AVS_I2S1_SDO",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x0130,  NORTHWEST),//I2S1 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_79 AVS_M_CLK_A1",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0138,  NORTHWEST),//DMIC
  BXT_GPIO_PAD_CONF(L"GPIO_80 AVS_M_CLK_B1",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0140,  NORTHWEST),//DMIC
  BXT_GPIO_PAD_CONF(L"GPIO_81 AVS_M_DATA_1",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,TxDRxE    ,  EnPd,  GPIO_PADBAR+0x0148,  NORTHWEST),//DMIC
  BXT_GPIO_PAD_CONF(L"GPIO_82 AVS_M_CLK_AB2",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0150,  NORTHWEST),//DMIC
  BXT_GPIO_PAD_CONF(L"GPIO_83 AVS_M_DATA_2",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,TxDRxE    ,  EnPd,  GPIO_PADBAR+0x0158,  NORTHWEST),//DMIC
  BXT_GPIO_PAD_CONF(L"GPIO_84 AVS_I2S2_MCLK",    M0   ,    GPI   , GPIO_D,  NA    ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0160,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_85 AVS_I2S2_BCLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0168,  NORTHWEST),//I2S2 Audio BT
  BXT_GPIO_PAD_CONF(L"GPIO_86 AVS_I2S2_WS_SYNC", M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0170,  NORTHWEST),//I2S2 Audio BT
  BXT_GPIO_PAD_CONF(L"GPIO_87 AVS_I2S2_SDI",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,TxDRxE    ,  EnPd,  GPIO_PADBAR+0x0178,  NORTHWEST),//I2S2 Audio BT
  BXT_GPIO_PAD_CONF(L"GPIO_88 AVS_I2S2_SDO",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0180,  NORTHWEST),//I2S2 Audio BT
  BXT_GPIO_PAD_CONF(L"GPIO_89 AVS_I2S3_BCLK",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0188,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_80 AVS_I2S3_WS_SYNC", M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0190,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_91 AVS_I2S3_SDI",     M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0198,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_92 AVS_I2S3_SDO",     M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x01A0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_97 FST_SPI_CS0_B",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01A8,  NORTHWEST),//FLASH
  BXT_GPIO_PAD_CONF(L"GPIO_98 FST_SPI_CS1_B",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x01B0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_99 FST_SPI_MOSI_IO0", M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01B8,  NORTHWEST),//FLASH
  BXT_GPIO_PAD_CONF(L"GPIO_100 FST_SPI_MISO_IO1",M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01C0,  NORTHWEST),//FLASH
  BXT_GPIO_PAD_CONF(L"GPIO_101 FST_SPI_IO2",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01C8,  NORTHWEST),//FLASH
  BXT_GPIO_PAD_CONF(L"GPIO_102 FST_SPI_IO3",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01D0,  NORTHWEST),//FLASH
  BXT_GPIO_PAD_CONF(L"GPIO_103 FST_SPI_CLK",     M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01D8,  NORTHWEST),//FLASH
  BXT_GPIO_PAD_CONF(L"FST_SPI_CLK_FB",           M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x01E0,  NORTHWEST),//no pin out
  BXT_GPIO_PAD_CONF(L"GPIO_104 GP_SSP_0_CLK",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x01E8,  NORTHWEST),//no pin out
  BXT_GPIO_PAD_CONF(L"GPIO_105 GP_SSP_0_FS0",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x01F0,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_106 GP_SSP_0_FS1",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x01F8,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_109 GP_SSP_0_RXD",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0200,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_110 GP_SSP_0_TXD",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0208,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_111 GP_SSP_1_CLK",    M1   ,    NA    , NA    ,   NA    ,   NA     ,  Wake_Disabled, P_20K_L,   NA    ,  NA  ,NA        ,    NA,  GPIO_PADBAR+0x0210,  NORTHWEST),//SPI1 CLK Radio
  BXT_GPIO_PAD_CONF(L"GPIO_112 GP_SSP_1_FS0",    M1   ,    NA    , NA    ,   HI    ,   NA     ,  Wake_Disabled, P_20K_L,   NA    ,  NA  ,NA        ,    NA,  GPIO_PADBAR+0x0218,  NORTHWEST),//SPI1 FS0 Radio
  BXT_GPIO_PAD_CONF(L"GPIO_113 GP_SSP_1_FS1",    M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,Last_Value,  SAME,  GPIO_PADBAR+0x0220,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M1   ,    NA   , NA    ,   NA    ,   Level   ,  Wake_Disabled, P_20K_L,   NA    ,IOAPIC,TxDRxE    ,    NA,  GPIO_PADBAR+0x0228,  NORTHWEST),//SPI1 RXD Radio
  BXT_GPIO_PAD_CONF(L"GPIO_117 GP_SSP_1_TXD",    M1   ,    NA   , NA    ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,    NA,  GPIO_PADBAR+0x0230,  NORTHWEST),//SPI1 TXD Radio
  BXT_GPIO_PAD_CONF(L"GPIO_118 GP_SSP_2_CLK",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,Last_Value,  SAME,  GPIO_PADBAR+0x0238,  NORTHWEST),//SPI2 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_119 GP_SSP_2_FS0",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,Last_Value,  SAME,  GPIO_PADBAR+0x0240,  NORTHWEST),//SPI2 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_120 GP_SSP_2_FS1",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0248,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_121 GP_SSP_2_FS2",    M0  ,     GPI   , GPIO_D, NA     ,   NA      ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA        ,NA    ,  GPIO_PADBAR+0x0250,  NORTHWEST),//NC
  BXT_GPIO_PAD_CONF(L"GPIO_122 GP_SSP_2_RXD",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,HizRx0I   ,  SAME,  GPIO_PADBAR+0x0258,  NORTHWEST),//SPI2 Audio
  BXT_GPIO_PAD_CONF(L"GPIO_123 GP_SSP_2_TXD",    M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,Last_Value,  SAME,  GPIO_PADBAR+0x0260,  NORTHWEST),//SPI2 Audio
};

// West Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_RVVP_GpioInitData_W [] =
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_W_OEM[] =
#endif
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,     MMIO_Offset    , Community */
  BXT_GPIO_PAD_CONF(L"GPIO_124 LPSS_I2C0_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0000,  WEST),//I2C0 Audio Codec
  BXT_GPIO_PAD_CONF(L"GPIO_125 LPSS_I2C0_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0008,  WEST),//I2C0 Audio Codec
  BXT_GPIO_PAD_CONF(L"GPIO_126 LPSS_I2C1_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0010,  WEST),//I2C1 Fuel Guage
  BXT_GPIO_PAD_CONF(L"GPIO_127 LPSS_I2C1_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0018,  WEST),//I2C1 Fuel Guage
  BXT_GPIO_PAD_CONF(L"GPIO_128 LPSS_I2C2_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0020,  WEST),//I2C2 Camera
  BXT_GPIO_PAD_CONF(L"GPIO_129 LPSS_I2C2_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0028,  WEST),//I2C2 Camera
  BXT_GPIO_PAD_CONF(L"GPIO_130 LPSS_I2C3_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0030,  WEST),//I2C3 Camera
  BXT_GPIO_PAD_CONF(L"GPIO_131 LPSS_I2C3_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0038,  WEST),//I2C3 Camera
  BXT_GPIO_PAD_CONF(L"GPIO_132 LPSS_I2C4_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0040,  WEST),//I2C4 USB FC
  BXT_GPIO_PAD_CONF(L"GPIO_133 LPSS_I2C4_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D1RxDRx1I,  EnPu,  GPIO_PADBAR+0x0048,  WEST),//I2C4 USB FC
  BXT_GPIO_PAD_CONF(L"GPIO_134 LPSS_I2C5_SDA",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0050,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_135 LPSS_I2C5_SCL",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0058,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_136 LPSS_I2C6_SDA",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0060,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_137 LPSS_I2C6_SCL",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0068,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_138 LPSS_I2C7_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0070,  WEST),// RFKILL_N
  BXT_GPIO_PAD_CONF(L"GPIO_139 LPSS_I2C7_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H ,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0078,  WEST),//HALL_STATE
  BXT_GPIO_PAD_CONF(L"GPIO_146 ISH_GPIO_0",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0080,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_147 ISH_GPIO_1",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0088,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_148 ISH_GPIO_2",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0090,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_149 ISH_GPIO_3",      M3   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0098,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_150 ISH_GPIO_4",      M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00A0,  WEST),//Feature: Reset                  Net in Sch: NFC_RESET_N
//[-start-160803-IB07400768-modify]//
  BXT_GPIO_PAD_CONF(L"GPIO_151 ISH_GPIO_5",      M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00A8,  WEST),//Feature: RF_KILL_WWAN           Net in Sch: NGFF_WWAN_RF_KILL_1P8_N
//[-end-160803-IB07400768-modify]//
/* Below GPIO Pin is programmed in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"GPIO_152 ISH_GPIO_6",      M0   ,    GPO   ,GPIO_D,   LO    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00B0,  WEST),//Feature: Reset                  Net in Sch: SOC_PCIE_SLOT1_RST_N,
*/
  BXT_GPIO_PAD_CONF(L"GPIO_153 ISH_GPIO_7",      M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x00B8,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_154 ISH_GPIO_8",      M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00C0,  WEST),//Feature: BT_Disable             Net in Sch: BT_DISABLE2_1P8_N
  BXT_GPIO_PAD_CONF(L"GPIO_155 ISH_GPIO_9",      M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x00C8,  WEST),//CG2000 PDB: If PDB = 0: power-down; If PDB = 1: power-up, it is the same in ISH/LPSS mode
/* default value is correct, no need to program
  BXT_GPIO_PAD_CONF(L"GPIO_209 PCIE_CLKREQ0_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00D0,  WEST),// Slot 1 CLKREQ#
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00D8,  WEST),// LAN CLKREQ#
  BXT_GPIO_PAD_CONF(L"GPIO_211 PCIE_CLKREQ2_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00E0,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_212 PCIE_CLKREQ3_B",  M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00E8,  WEST),//DOCK_INT_N and  /PEX_1_CLKREQ check
*/
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_0",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00F0,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_1",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x00F8,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_2",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0100,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_3",            M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0108,  WEST),
  BXT_GPIO_PAD_CONF(L"OSC_CLK_OUT_4",            M0   ,    GPI   ,GPIO_D,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,NA        ,  NA  ,  GPIO_PADBAR+0x0110,  WEST),//Not used on RVP

  BXT_GPIO_PAD_CONF(L"PMU_AC_PRESENT",           M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_L,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0118,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_BATLOW_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0120,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_PLTRST_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0128,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_PWRBTN_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0130,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_RESETBUTTON_B",        M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0138,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S0_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0140,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S3_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0148,  WEST),//Check, OBS
  BXT_GPIO_PAD_CONF(L"PMU_SLP_S4_B",             M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0150,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_SUSCLK",               M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0158,  WEST),
  BXT_GPIO_PAD_CONF(L"PMU_WAKE_B",               M0   ,    GPO   ,GPIO_D,   HI    ,   NA      ,  Wake_Disabled, P_20K_H,   NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0160,  WEST),//Power cycling for EMMC/RVP
  BXT_GPIO_PAD_CONF(L"SUS_STAT_B",               M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0168,  WEST),
  BXT_GPIO_PAD_CONF(L"SUSPWRDNACK",              M1   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,    NA    ,    NA,IOS_Masked,  SAME,  GPIO_PADBAR+0x0170,  WEST),
};

 // South West Community
#ifdef APOLLOLAKE_CRB
BXT_GPIO_PAD_INIT  mBXT_RVVP_GpioInitData_SW[]=
#else
BXT_GPIO_PAD_INIT  mBXT_GpioInitData_SW_OEM[] =
#endif
{
  //                  Group Pin#:  pad_name,       PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,   MMIO_Offset     ,  Community */
  BXT_GPIO_PAD_CONF(L"GPIO_205 PCIE_WAKE0_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0000 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_206 PCIE_WAKE1_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0008 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_207 PCIE_WAKE2_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0010 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_208 PCIE_WAKE3_B",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0018 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_156 EMMC0_CLK",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I,SAME  ,GPIO_PADBAR+0x0020 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_157 EMMC0_D0",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0028 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_158 EMMC0_D1",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0030 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_159 EMMC0_D2",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0038 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_160 EMMC0_D3",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0040 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_161 EMMC0_D4",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0048 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_162 EMMC0_D5",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0050 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_163 EMMC0_D6",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0058 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_164 EMMC0_D7",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0060 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_165 EMMC0_CMD",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0068 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_166 SDIO_CLK",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I,SAME  ,GPIO_PADBAR+0x0070 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_167 SDIO_D0",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0078 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_168 SDIO_D1",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0080 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_169 SDIO_D2",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0088 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_170 SDIO_D3",           M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0090 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_171 SDIO_CMD",          M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x0098 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_172 SDCARD_CLK",        M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,D0RxDRx0I,NA    ,GPIO_PADBAR+0x00A0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_179 SDCARD_CLK_FB",     M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA       ,NA    ,GPIO_PADBAR+0x00A8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_173 SDCARD_D0",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x00B0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_174 SDCARD_D1",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x00B8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_175 SDCARD_D2",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x00C0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_176 SDCARD_D3",         M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,HizRx1I  ,SAME  ,GPIO_PADBAR+0x00C8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_177 SDCARD_CD_B",       M0,     GPI   , GPIO_D ,  NA     ,   Edge     ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,TxDRxE   ,NA    ,GPIO_PADBAR+0x00D0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_178 SDCARD_CMD",        M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,NA       ,NA    ,GPIO_PADBAR+0x00D8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_186 SDCARD_LVL_WP",     M0,     GPI   , GPIO_D ,  NA     ,   Edge     ,Wake_Disabled, P_20K_L,Inverted ,    NA  ,Last_Value,SAME ,GPIO_PADBAR+0x00E0 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_182 EMMC0_STROBE",      M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,HizRx0I  ,SAME  ,GPIO_PADBAR+0x00E8 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"GPIO_183 SDIO_PWR_DOWN_B",   M0,     GPO   , GPIO_D ,  LO     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,NA       ,NA    ,GPIO_PADBAR+0x00F0 ,  SOUTHWEST),// Feature:Power Enable  Net in Sch:SD_CARD_PWR_EN_N
  BXT_GPIO_PAD_CONF(L"SMB_ALERTB",                 M0,     GPI   , GPIO_D ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x00F8 ,  SOUTHWEST),//not used on RVP
/* below pins are programmed before MRC in GpioPreMem.c
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0100 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0108 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0110 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0118 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_NONE,     NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0120 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0128 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0130 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0138 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0140 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0148 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA   ,  NA     ,   NA       ,Wake_Disabled, P_20K_H,    NA   ,    NA  ,IOS_Masked, SAME,GPIO_PADBAR+0x0150 ,  SOUTHWEST),
*/
};

//[-start-161108-IB07400810-add]//
#endif
//[-end-161108-IB07400810-add]//
#endif
