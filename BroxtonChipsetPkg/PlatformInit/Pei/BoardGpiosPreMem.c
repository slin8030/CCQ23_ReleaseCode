/** @file
  Gpio setting for multiplatform before Memory init.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
//[-start-160413-IB03090426-modify]//
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/GpioLib.h>
#include <Library/DebugLib.h>
#include <Library/SteppingLib.h>
//[-start-160803-IB07220122-add]//
#include <Library/PeiServicesLib.h>
//[-end-160803-IB07220122-add]//
#include <ChipsetSetupConfig.h>
#include <Ppi/ReadOnlyVariable2.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
#include <Library/BaseMemoryLib.h>
#include <Library/TimerLib.h>
#include "BoardGpiosPreMem.h"
//[-start-160720-IB03090429-add]//
#include <ScRegs/RegsGpio.h>
//[-end-160720-IB03090429-add]//

/// *****************************************************************************************************************************************
/// ***************************************************   BXT PreMemory GPIO CONFIGURATION    ***********************************************
/// *****************************************************************************************************************************************

//[-start-161107-IB07400810-add]//
#include <Guid/PlatformInfo.h>
#include <Library/MultiPlatformBaseLib.h>

#define GPIO_INIT_IN_PEI_PHASE_PRE_MEMORY
#ifdef APOLLOLAKE_CRB 
// CRB platform
#include <BoardGpiosRvp.h>
#include <BoardGpiosOxbowHill.h>

#else 
// OEM platform
#include <OemBoardGpios.h>
#endif

#if 0
//[-end-161107-IB07400810-add]//

// BXT GPIO Settings Note:
/*
GPIO input pin interrupt type configuration:
Interrupt type    GPI Route         Host SW               Enable/Status                               Comment
 GPI               None              GPIO Driver Mode      GPI Interrupt Status\Enable                 GPIO driver to handle it
 Direct IRQ        GPIROUTIOXAPIC    ACPI Mode                                                         IRQ number is fixed to each GPIO pin in N and NW communities
 SCI/GPE           GPIROUTSCI        ACPI Mode             GPI General Purpose Events Status\Enable    SCI is not supported in BXT A0. The reason is because the PMC lacks the ACPI registers and status tunneling. This will be fixed in derivatives.
 SMI               GPIROUTSMI        ACPI Mode             SMI Status\Enable                           Don't enable SMI for BXT0. It is currently unsupported by the PMC.
 NMI               GPIROUTNMI        ACPI Mode                                                         Not support on BXT

Interrupt trigger type             Configuration               Comment
 rising edge                        Edge+No_invert
 falling edge                       Edge+Invert
 both edge                          BothEdge+Invert
 level high                         Level+No_invert             Direct IRQ pin mostly use this config.Wake pin MUST use it.
 level low                          Level+Invert

HostSw:
* All GPIO pins which are 'M0' PMode, have to set HostSw to GPIO_D, indicating GPIO driver owns it.
* Others, such as Native function(M1,M2,M3..) and SCI/SMI/NMI/Direct IRQ, need to set it to ACPI_D or NA.

*/
#if (TABLET_PF_ENABLE == 1)
//  Need to make sure ISH I2C GPIOs are configured before ISH starts running. [HSD 1205461649]
BXT_GPIO_PAD_INIT  IshI2cGpio[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_140 ISH_I2C0_SDA",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_1K_H,    NA    ,    NA,IOS_Masked,  SAME,     0x0480,  WEST),//GPSAOB_I2C_SDA and GPS_I2C_SDA and SNSR_I2C_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_141 ISH_I2C0_SCL",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_1K_H,    NA    ,    NA,IOS_Masked,  SAME,     0x0488,  WEST),//GPSAOB_I2C_SCL and GPS_I2C_SCL and SNSR_I2C_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_142 ISH_I2C1_SDA",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_1K_H,    NA    ,    NA,IOS_Masked,  SAME,     0x0490,  WEST),//MDM_I2C_SDA and SNSR2_I2C_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_143 ISH_I2C1_SCL",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_1K_H,    NA    ,    NA,IOS_Masked,  SAME,     0x0498,  WEST),//MDM_I2C_SCL and SNSR2_I2C_SCL
  BXT_GPIO_PAD_CONF(L"GPIO_144 ISH_I2C2_SDA",    M2   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_2K_H,    NA    ,    NA,IOS_Masked,  SAME,     0x04a0,  WEST),// LPSS_I2C7_SDA and I2C_FG_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_145 ISH_I2C2_SCL",    M2   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_2K_H,    NA    ,    NA,IOS_Masked,  SAME,     0x04a8,  WEST),// LPSS_I2C7_SCL and I2C_FG_SCL
};
#else
//[-start-160801-IB03090430-modify]//
// For Apl board
BXT_GPIO_PAD_INIT  IshI2cGpio[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_134 LPSS_I2C5_SDA",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0050,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_135 LPSS_I2C5_SCL",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0058,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_136 LPSS_I2C6_SDA",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0060,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_137 LPSS_I2C6_SCL",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_1K_H,    NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0068,  WEST),
};
//[-end-160801-IB03090430-modify]//
#endif

//[-start-160720-IB03090429-modify]//
// North Peak GPIO settings before memory initialization, as it needs to be enabled before memory init
BXT_GPIO_PAD_INIT  NorthPeakGpio_x4[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0020,  NORTH),
};

// North Peak GPIO settings before memory initialization, as it needs to be enabled before memory init
BXT_GPIO_PAD_INIT  NorthPeakGpio_x8[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0028,  NORTH),//Mux with CSE_PG based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0030,  NORTH),//Mux with DISP1_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0038,  NORTH),//Mux with DISP1_TOUCH_INT_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0040,  NORTH),//Mux with DISP1_TOUCH_RST_N based on the SW3 switch
};

// North Peak GPIO settings before memory initialization, as it needs to be enabled before memory init
BXT_GPIO_PAD_INIT  NorthPeakGpio_x16[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0028,  NORTH),//Mux with CSE_PG based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0030,  NORTH),//Mux with DISP1_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0038,  NORTH),//Mux with DISP1_TOUCH_INT_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0040,  NORTH),//Mux with DISP1_TOUCH_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0048,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_10",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0050,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0058,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0060,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0068,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0070,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0078,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0080,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0088,  NORTH),
};
//[-end-160720-IB03090429-modify]//

#if (TABLET_PF_ENABLE == 1)
// LPSS SPI controller GPIO settings before memory initialization, as it is used for POST code and BIOS debug log via serial port before memory init
BXT_GPIO_PAD_INIT  LpssSpi1Gpio [] =
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,    MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_111 GP_SSP_1_CLK",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x01d8,  NORTHWEST),//SSP_5_CLK_R
  BXT_GPIO_PAD_CONF(L"GPIO_112 GP_SSP_1_FS0",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x01e0,  NORTHWEST),//GP_SSP_1_FS0_R post
  BXT_GPIO_PAD_CONF(L"GPIO_113 GP_SSP_1_FS1",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x01e8,  NORTHWEST),//GP_SSP_1_FS1_R UART
  BXT_GPIO_PAD_CONF(L"GPIO_114 GP_SSP_1_FS2",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x01f0,  NORTHWEST),//DSDA_MUX_SEL
  BXT_GPIO_PAD_CONF(L"GPIO_115 GP_SSP_1_FS3",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x01f8,  NORTHWEST),//SSP_1_FS3_R/DAQ_TRIGGER  to Trigger for PnP measurement
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0200,  NORTHWEST),//SSP_5_RXD_R
  BXT_GPIO_PAD_CONF(L"GPIO_117 GP_SSP_1_TXD",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0208,  NORTHWEST),//SSP_5_TXD_R
};
#else
BXT_GPIO_PAD_INIT  LpssSpi1Gpio [] =
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,IOSstae,  IOSTerm,    MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_111 GP_SSP_1_CLK",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0210,  NORTHWEST),//SSP_5_CLK_R
  BXT_GPIO_PAD_CONF(L"GPIO_112 GP_SSP_1_FS0",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0218,  NORTHWEST),//GP_SSP_1_FS0_R post
  BXT_GPIO_PAD_CONF(L"GPIO_113 GP_SSP_1_FS1",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0220,  NORTHWEST),//GP_SSP_1_FS1_R UART
  BXT_GPIO_PAD_CONF(L"GPIO_116 GP_SSP_1_RXD",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0228,  NORTHWEST),//SSP_5_RXD_R
  BXT_GPIO_PAD_CONF(L"GPIO_117 GP_SSP_1_TXD",    M1   ,    HI_Z  , NA   ,   NA    ,   NA      , Wake_Disabled,  P_20K_H,   NA    ,    NA,Last_Value,  SAME,     GPIO_PADBAR+0x0230,  NORTHWEST),//SSP_5_TXD_R
};
#endif

#if (TABLET_PF_ENABLE == 1)
// I2C controller GPIO settings before memory initialization, as it is used for Fab ID reading ( I2C 6) before memory init
BXT_GPIO_PAD_INIT  I2c6Gpio [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_136 LPSS_I2C6_SDA",   M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_1K_H,    NA    ,    NA, D1RxDRx1I,  EnPu,     GPIO_PADBAR+0x0060,  WEST),//EXP_I2C_SDA and I2C_PSS_SDA and I2C_2_SDA_IOEXP
  BXT_GPIO_PAD_CONF(L"GPIO_137 LPSS_I2C6_SCL",   M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_1K_H,    NA    ,    NA, D1RxDRx1I,  EnPu,     GPIO_PADBAR+0x0068,  WEST),//EXP_I2C_SCL and I2C_PSS_SCL and I2C_2_SCL_IOEXP
};
#endif

//  Need to make sure ISH I2C GPIOs are configured before ISH starts running. [HSD 1205461649]
#if (TABLET_PF_ENABLE == 1)
BXT_GPIO_PAD_INIT  IshI2cGpio2[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_140 ISH_I2C0_SDA",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_NONE,    NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0080,  WEST),//GPSAOB_I2C_SDA and GPS_I2C_SDA and SNSR_I2C_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_141 ISH_I2C0_SCL",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_NONE,    NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0088,  WEST),//GPSAOB_I2C_SCL and GPS_I2C_SCL and SNSR_I2C_SCL
};
#else
//ApolloLake
BXT_GPIO_PAD_INIT  IshI2cGpio2[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_134 ISH_I2C0_SDA",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0050,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_135 ISH_I2C0_SCL",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0058,  WEST)
};
#endif

//  Need to make sure ISH I2C GPIOs are configured before ISH starts running. [HSD 1205461649]
#if (TABLET_PF_ENABLE == 1)
BXT_GPIO_PAD_INIT  IshI2cGpio3[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_142 ISH_I2C1_SDA",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_NONE,    NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0090,  WEST),//MDM_I2C_SDA and SNSR2_I2C_SDA
  BXT_GPIO_PAD_CONF(L"GPIO_143 ISH_I2C1_SCL",    M1   ,    NA    , NA   ,   NA    ,   NA      , Wake_Disabled,  P_NONE,    NA    ,    NA,IOS_Masked,  SAME,     GPIO_PADBAR+0x0098,  WEST),//MDM_I2C_SCL and SNSR2_I2C_SCL
};
#else
//ApolloLake
BXT_GPIO_PAD_INIT  IshI2cGpio3[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_136 ISH_I2C1_SDA",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0060,  WEST),
  BXT_GPIO_PAD_CONF(L"GPIO_137 ISH_I2C1_SCL",   M2   ,    NA    , NA   ,   NA    ,   NA      ,  Wake_Disabled, P_NONE,   NA    ,    NA, D0RxDRx0I,  EnPu,  GPIO_PADBAR+0x0068,  WEST)
};
#endif

#if (TABLET_PF_ENABLE == 0)
//
// Pre-program in PlatformSecLib
//
//[-start-160720-IB03090429-modify]//
BXT_GPIO_PAD_INIT  LpcGpio [] =
{
  //                 Group Pin#:  pad_name,     PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger, Wake_Enabled, Term_H_L,Inverted,GPI_ROUT,   IOSstae, IOSTerm,    MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"LPC_ILB_SERIRQ",             M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0110 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT0",                M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_NONE  ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0118 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKOUT1",                M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_NONE  ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0120 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD0",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0128 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD1",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0130 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD2",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0138 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_AD3",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0140 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_CLKRUNB",                M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0148 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"LPC_FRAMEB",                 M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,HizRx1I   , DisPuPd,GPIO_PADBAR+0x0150 ,  SOUTHWEST),
};

BXT_GPIO_PAD_INIT  SmbusGpio [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"SMB_CLK",                    M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0100 ,  SOUTHWEST),
  BXT_GPIO_PAD_CONF(L"SMB_DATA",                   M1,     NA    ,   NA ,  NA     ,   NA      ,Wake_Disabled, P_20K_H ,   NA   ,    NA  ,IOS_Masked, SAME   ,GPIO_PADBAR+0x0108 ,  SOUTHWEST),
};
//[-end-160720-IB03090429-modify]//

//
// Pre-program in PlatformSecLib
//
BXT_GPIO_PAD_INIT  UartGpio [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  BXT_GPIO_PAD_CONF(L"GPIO_46 LPSS_UART2_RXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0170,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_47 LPSS_UART2_TXD",   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,      NA  ,     NA, GPIO_PADBAR+0x0178,  NORTH),
};

BXT_GPIO_PAD_INIT  SataGpio [] =
{
  BXT_GPIO_PAD_CONF(L"GPIO_22",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,  HizRx0I ,   SAME, GPIO_PADBAR+0x00B0,  NORTH),//Feature:Power Enable for SATA DIRECT CONNECTOR
};
#endif

#if (TABLET_PF_ENABLE == 1)
BXT_GPIO_PAD_INIT  PcieGpioBxt1 [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  //For WIFI, PFET enabled is the GPIO COMBO_WLAN_EN, according to PCIe power sequence,PFET need to be set to high as early in BIOS code as possible, and set the PERST# to low here
  //1)  Set WLAN_EN to high
  //2)  100ms delay
  //3)  Set PERST# to high
  //4)  Set BLKDQDA and BLKPLLEN to low( which starts link training)
  BXT_GPIO_PAD_CONF(L"GPIO_183 SDIO_PWR_DOWN_B",   M0,     GPO   ,  GPIO_D,  HI     ,   NA       ,Wake_Disabled, P_20K_L,    NA   ,    NA  ,  NA   ,     NA,             GPIO_PADBAR+0x00d8,  SOUTHWEST),//COMBO_WLAN_EN
  BXT_GPIO_PAD_CONF(L"GPIO_191 DBI_SDA",         M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA,             GPIO_PADBAR+0x0020,  SOUTH), // PEX0_PERST_N PCIE for wigi

  // WiGi
  BXT_GPIO_PAD_CONF(L"GPIO_192 DBI_SCL",         M0   ,    GPI   , GPIO_D,  NA    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA,             GPIO_PADBAR+0x0028,  SOUTH), // DOCk_RST_N and PEX_1_PERST_N
  BXT_GPIO_PAD_CONF(L"DGCLKDBG_PMC_2",           M2   ,    NA    , NA   ,   NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA,             GPIO_PADBAR+0x0118,  WEST),  //DOCK_INT_N and  /PEX_1_CLKREQ check
};
#else
BXT_GPIO_PAD_INIT  PcieGpio [] =
{
  //                   Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,Wake_Enabled, Term_H_L, Inverted,GPI_ROUT,IOSstae, IOSTerm,   MMIO_Offset, Community
  // slot 1
  BXT_GPIO_PAD_CONF(L"GPIO_152 ISH_GPIO_6",      M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00B0,  WEST),  // PERST#
  BXT_GPIO_PAD_CONF(L"GPIO_19",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0098,  NORTH), // PFET
  // Slot 2
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0068,  NORTH), // PERST#
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M0   ,    GPO   , GPIO_D,  HI    ,   NA       , Wake_Disabled, P_20K_H,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0088,  NORTH), // PFET
  // NGFF
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0078,  NORTH), // PERST#
  // LAN
  BXT_GPIO_PAD_CONF(L"GPIO_210 PCIE_CLKREQ1_B",  M1   ,    NA    , NA    ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x00D8,  WEST),  // CLKREQ#
  BXT_GPIO_PAD_CONF(L"GPIO_37 PWM3",             M0   ,    GPO   , GPIO_D,  LO    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,     NA   ,     NA, GPIO_PADBAR+0x0128,  NORTH), // PERST#
};
#endif

/**
  Set GPIO pins before MRC init per board design.

  @retval EFI_SUCCESS               The function completed successfully.
**/
EFI_STATUS
MultiPlatformGpioProgramPreMem (
  IN OUT UINT64   *StartTimerTick
  )
{
#if (TABLET_PF_ENABLE == 1)
  BXT_SERIES BxtSeries;
#endif

  // PAD programming
  GpioPadConfigTable (sizeof (IshI2cGpio) / sizeof (IshI2cGpio[0]), IshI2cGpio);
//[-start-160720-IB03090429-remove]//
//  GpioPadConfigTable (sizeof (NorthPeakGpio) / sizeof (NorthPeakGpio[0]), NorthPeakGpio);
//[-end-160720-IB03090429-remove]//
  GpioPadConfigTable (sizeof (LpssSpi1Gpio) / sizeof (LpssSpi1Gpio[0]), LpssSpi1Gpio);
#if (TABLET_PF_ENABLE == 1)
  GpioPadConfigTable (sizeof (I2c6Gpio) / sizeof (I2c6Gpio[0]), I2c6Gpio);
#endif

#if (TABLET_PF_ENABLE == 1)
  BxtSeries = GetBxtSeries ();
  if (BxtSeries == Bxt1) {
    GpioPadConfigTable (sizeof (PcieGpioBxt1) / sizeof (PcieGpioBxt1[0]), PcieGpioBxt1);
    *StartTimerTick = GetPerformanceCounter ();
  }
#else
  GpioPadConfigTable (sizeof (PcieGpio) / sizeof (PcieGpio[0]), PcieGpio);
  *StartTimerTick = GetPerformanceCounter ();
  GpioPadConfigTable (sizeof (SataGpio) / sizeof (SataGpio[0]), SataGpio);
  GpioPadConfigTable (sizeof (LpcGpio) / sizeof (LpcGpio[0]), LpcGpio);
//[-start-160720-IB03090429-add]//
  GpioPadConfigTable (sizeof (SmbusGpio) / sizeof (SmbusGpio[0]), SmbusGpio);
//[-end-160720-IB03090429-add]//
  GpioPadConfigTable (sizeof (UartGpio) / sizeof (UartGpio[0]), UartGpio);
#endif

  return EFI_SUCCESS;
}

/**
  Set ISH GPIO pins before MRC init per board design.
  Before call it, make sure PlatformInfoHob->BoardId&PlatformFlavor is get correctly.

  @retval EFI_SUCCESS      The function completed successfully.
**/
//[-start-160720-IB03090429-modify]//
EFI_STATUS
MultiPlatformGpioUpdatePreMem (
  VOID
  )
//[-end-160720-IB03090429-modify]//
{
  UINTN                VariableSize;
  EFI_STATUS           Status;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
//[-end-160803-IB07220122-add]//
  CHIPSET_CONFIGURATION SystemConfiguration;
//[-start-160720-IB03090429-add]//
#if (TABLET_PF_ENABLE == 0)
  UINT32                          PadCfgDw1Offset;
  UINT32                          Data32;
  UINT32                          Data32Or;
  UINT32                          Data32And;
//[-start-160801-IB03090430-add]//
  UINT8                           IshGpioSize;
  UINT8                           Index;
//[-end-160801-IB03090430-add]//
#endif
//[-end-160720-IB03090429-add]//

//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
  ZeroMem (&SystemConfiguration, VariableSize);
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigDataPreMem (&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//

  if (!EFI_ERROR (Status)) {
    //
    // HSD 1304036824 : Add option in BIOS to disable pull-up resistor on ISH I2C line
    //
    if (SystemConfiguration.IshI2c0PullUp == 0) {
      GpioPadConfigTable (sizeof(IshI2cGpio2)/sizeof(IshI2cGpio2[0]), IshI2cGpio2);
    }

    if (SystemConfiguration.IshI2c1PullUp == 0) {
      GpioPadConfigTable (sizeof(IshI2cGpio3)/sizeof(IshI2cGpio3[0]), IshI2cGpio3);
    }

//[-start-160720-IB03090429-add]//
#if (TABLET_PF_ENABLE == 0)
//    if ((SystemConfiguration.LowPowerS0Idle == TRUE) && (SystemConfiguration.Ps2KbMsEnable != FALSE)) {
    if ((SystemConfiguration.LowPowerS0Idle == TRUE)) {
        Data32And = (UINT32) ~(B_GPIO_IOSSTATE | B_GPIO_IOSTERM);
        Data32Or  = (IOS_Masked << N_GPIO_IOSSTATE) | (SAME << N_GPIO_IOSTERM);
        PadCfgDw1Offset = SW_LPC_CLKOUT0 + BXT_GPIO_PAD_CONF1_OFFSET;
        Data32 = GpioPadRead (PadCfgDw1Offset);
        Data32 &= Data32And;
        Data32 |= Data32Or;
        GpioPadWrite (PadCfgDw1Offset, Data32);

        PadCfgDw1Offset = SW_LPC_CLKRUN + BXT_GPIO_PAD_CONF1_OFFSET;
        Data32 = GpioPadRead (PadCfgDw1Offset);
        Data32 &= Data32And;
        Data32 |= Data32Or;
        GpioPadWrite (PadCfgDw1Offset, Data32);
    }

//[-start-160801-IB03090430-add]//
    if (SystemConfiguration.LowPowerS0Idle == TRUE) {
      IshGpioSize = sizeof (IshI2cGpio) / sizeof (IshI2cGpio[0]);
      for (Index = 0; Index < IshGpioSize; Index++) {
        PadCfgDw1Offset = ((IshI2cGpio[Index].Community << 16) + IshI2cGpio[Index].MMIO_ADDRESS + BXT_GPIO_PAD_CONF1_OFFSET);
        Data32 = GpioPadRead (PadCfgDw1Offset);
        Data32 &= ~(B_GPIO_IOSSTATE);
        Data32 |= (IOS_Masked << N_GPIO_IOSSTATE);
        GpioPadWrite (PadCfgDw1Offset, Data32);
      }

//[-start-160817-IB03090432-add]//
      PadCfgDw1Offset = ((SataGpio[0].Community << 16) + SataGpio[0].MMIO_ADDRESS + BXT_GPIO_PAD_CONF1_OFFSET);
      Data32 = GpioPadRead (PadCfgDw1Offset);
      Data32 &= ~(B_GPIO_IOSSTATE);
      Data32 |= (IOS_Masked << N_GPIO_IOSSTATE);
      GpioPadWrite (PadCfgDw1Offset, Data32);
//[-end-160817-IB03090432-add]//
    }
//[-end-160801-IB03090430-add]//

#endif
  
    DEBUG ((DEBUG_INFO, "Start NPK GPIO Init PtiMode= %x and Destination= %x \n",SystemConfiguration.PtiMode, SystemConfiguration.FwTraceDestination));
    if (SystemConfiguration.FwTraceDestination == 4) { // Only Configure northpeak GPIO when destination is PTI 
      if (SystemConfiguration.PtiMode == 1) {
        GpioPadConfigTable (sizeof (NorthPeakGpio_x4) / sizeof (NorthPeakGpio_x4[0]), NorthPeakGpio_x4);
      } else if (SystemConfiguration.PtiMode == 2) {
        GpioPadConfigTable (sizeof (NorthPeakGpio_x8) / sizeof (NorthPeakGpio_x8[0]), NorthPeakGpio_x8);
      } else if (SystemConfiguration.PtiMode == 8) {
        GpioPadConfigTable (sizeof (NorthPeakGpio_x16) / sizeof (NorthPeakGpio_x16[0]), NorthPeakGpio_x16);
      }
    }
  } else { // (!EFI_ERROR (Status))
    GpioPadConfigTable (sizeof (NorthPeakGpio_x8) / sizeof (NorthPeakGpio_x8[0]), NorthPeakGpio_x8);
//[-end-160720-IB03090429-add]//
  }
  return EFI_SUCCESS;
}

//[-start-161107-IB07400810-add]//
#endif

#ifdef NPK_GPIO_SUPPORT
// North Peak GPIO settings before memory initialization, as it needs to be enabled before memory init
BXT_GPIO_PAD_INIT  NorthPeakGpio_x4[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0020,  NORTH),
};

// North Peak GPIO settings before memory initialization, as it needs to be enabled before memory init
BXT_GPIO_PAD_INIT  NorthPeakGpio_x8[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0028,  NORTH),//Mux with CSE_PG based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0030,  NORTH),//Mux with DISP1_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0038,  NORTH),//Mux with DISP1_TOUCH_INT_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0040,  NORTH),//Mux with DISP1_TOUCH_RST_N based on the SW3 switch
};

// North Peak GPIO settings before memory initialization, as it needs to be enabled before memory init
BXT_GPIO_PAD_INIT  NorthPeakGpio_x16[] =
{
  /*                  Group Pin#:  pad_name,    PMode,GPIO_Config,HostSw,GPO_STATE,INT_Trigger,  Wake_Enabled ,Term_H_L,Inverted, GPI_ROUT, IOSstae, IOSTerm,MMIO_Offset,Community */
  BXT_GPIO_PAD_CONF(L"GPIO_0",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0000,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_1",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0008,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_2",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0010,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_3",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0018,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_4",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0020,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_5",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0028,  NORTH),//Mux with CSE_PG based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_6",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0030,  NORTH),//Mux with DISP1_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_7",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0038,  NORTH),//Mux with DISP1_TOUCH_INT_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_8",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0040,  NORTH),//Mux with DISP1_TOUCH_RST_N based on the SW3 switch
  BXT_GPIO_PAD_CONF(L"GPIO_9",                   M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0048,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_10",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0050,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_11",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0058,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_12",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0060,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_13",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0068,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_14",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0070,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_15",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0078,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_16",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0080,  NORTH),
  BXT_GPIO_PAD_CONF(L"GPIO_17",                  M1   ,    NA    ,  NA   ,  NA    ,   NA       , Wake_Disabled, P_20K_L,   NA    ,    NA,   HizRx0I,   SAME,    GPIO_PADBAR+0x0088,  NORTH),
};
#endif

EFI_STATUS
MultiPlatformGpioProgramPreMem (
  IN OUT UINT64   *StartTimerTick
  )
{
  UINT8  BoardId;

  BoardId = MultiPlatformGetBoardIds();
  
#ifdef APOLLOLAKE_CRB
  //
  // CRB Pre-Memory GPIO init
  //
  switch (BoardId) {
  case BOARD_ID_APL_RVP_1A:
  case BOARD_ID_APL_RVP_2A:
    DEBUG ((DEBUG_INFO, "PEI Pre-Memory GPIO Table (mPreMemGpioInitData_RVP) Init\n"));
    GpioPadConfigTable (sizeof (mPreMemGpioInitData_RVP) / sizeof (mPreMemGpioInitData_RVP[0]), mPreMemGpioInitData_RVP);
    DumpGpioPadTable (sizeof (mPreMemGpioInitData_RVP) / sizeof (mPreMemGpioInitData_RVP[0]), mPreMemGpioInitData_RVP);
    break;
  case BOARD_ID_OXH_CRB:
  case BOARD_ID_LFH_CRB:
  case BOARD_ID_JNH_CRB:
    DEBUG ((DEBUG_INFO, "PEI Pre-Memory GPIO Table (mPreMemGpioInitData_LH) Init\n"));
    GpioPadConfigTable (sizeof (mPreMemGpioInitData_LH) / sizeof (mPreMemGpioInitData_LH[0]), mPreMemGpioInitData_LH);
    DumpGpioPadTable (sizeof (mPreMemGpioInitData_LH) / sizeof (mPreMemGpioInitData_LH[0]), mPreMemGpioInitData_LH);
    break;
  default:
    break;
  }
#else
  //
  // OEM Pre-Memory GPIO init
  //
#ifdef mPreMemGpioInitData_OEM_Support  
  DEBUG ((DEBUG_INFO, "PEI Pre-Memory GPIO Table (mPreMemGpioInitData_OEM) Init\n"));
  GpioPadConfigTable (sizeof (mPreMemGpioInitData_OEM) / sizeof (mPreMemGpioInitData_OEM[0]), mPreMemGpioInitData_OEM);
  DumpGpioPadTable (sizeof (mPreMemGpioInitData_OEM) / sizeof (mPreMemGpioInitData_OEM[0]), mPreMemGpioInitData_OEM);
#endif
#endif

  *StartTimerTick = GetPerformanceCounter ();

  return EFI_SUCCESS;
}

#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// ISH I2C config
//
#define W_GPIO_134            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x0050     //LPSS_I2S5_SDA
#define W_GPIO_135            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x0058     //LPSS_I2S5_SCL
#define W_GPIO_136            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x0060     //LPSS_I2S6_SDA
#define W_GPIO_137            (((UINT32)GPIO_MMIO_OFFSET_W)<<16)+GPIO_PADBAR+0x0068     //LPSS_I2S6_SCL
//
// LPC S0Ix Config
//
#define SW_LPC_CLKOUT0        (((UINT32)GPIO_MMIO_OFFSET_SW)<<16)+GPIO_PADBAR+0x0118    //LPC_CLKOUT0
#define SW_LPC_CLKOUT1        (((UINT32)GPIO_MMIO_OFFSET_SW)<<16)+GPIO_PADBAR+0x0120    //LPC_CLKOUT1
#define SW_LPC_CLKRUN         (((UINT32)GPIO_MMIO_OFFSET_SW)<<16)+GPIO_PADBAR+0x0148    //LPC_CLKRUN

//
// RVP SATA Power Config
//
#define N_GPIO_22             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x00B0     //GPIO_22
#endif

EFI_STATUS
MultiPlatformGpioUpdatePreMem (
  VOID
  )
{
  UINTN                           VariableSize;
  EFI_STATUS                      Status;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
  CHIPSET_CONFIGURATION           SystemConfiguration;
  BXT_CONF_PAD0                   padConfg0;
  BXT_CONF_PAD1                   padConfg1;

  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  VariableSize = PcdGet32 (PcdSetupConfigSize);
  ZeroMem (&SystemConfiguration, VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );

  if (!EFI_ERROR (Status)) {
    //
    // HSD 1304036824 : Add option in BIOS to disable pull-up resistor on ISH I2C line
    //
    if (SystemConfiguration.IshI2c0PullUp == 0) {
      //
      // GPIO_134 -> ISH_I2C0_SDA
      // GPIO_135 -> ISH_I2C0_SCL
      //
//      GpioPadConfigTable (sizeof(IshI2cGpio2)/sizeof(IshI2cGpio2[0]), IshI2cGpio2);
      padConfg0.padCnf0 = GpioPadRead(W_GPIO_134 + BXT_GPIO_PAD_CONF0_OFFSET);
      if (padConfg0.r.PMode == 2) { // ISH I2C mode
        DEBUG ((DEBUG_INFO, "IshI2c0PullUp is disabled, Config ISH_I2C0_SDA GPIO!!\n"));
        padConfg1.padCnf1 = GpioPadRead(W_GPIO_134 + BXT_GPIO_PAD_CONF1_OFFSET); 
        padConfg1.r.Term = 0; // Pull None
        GpioPadWrite(W_GPIO_134 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
      }
      padConfg0.padCnf0 = GpioPadRead(W_GPIO_135 + BXT_GPIO_PAD_CONF0_OFFSET);
      if (padConfg0.r.PMode == 2) { // ISH I2C mode
        DEBUG ((DEBUG_INFO, "IshI2c0PullUp is disabled, Config ISH_I2C0_SCL GPIO!!\n"));
        padConfg1.padCnf1 = GpioPadRead(W_GPIO_135 + BXT_GPIO_PAD_CONF1_OFFSET); 
        padConfg1.r.Term = 0; // Pull None
        GpioPadWrite(W_GPIO_135 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
      }
    }

    if (SystemConfiguration.IshI2c1PullUp == 0) {
      //
      // GPIO_136 -> ISH_I2C1_SDA
      // GPIO_137 -> ISH_I2C1_SCL
      //
//      GpioPadConfigTable (sizeof(IshI2cGpio3)/sizeof(IshI2cGpio3[0]), IshI2cGpio3);
      padConfg0.padCnf0 = GpioPadRead(W_GPIO_136 + BXT_GPIO_PAD_CONF0_OFFSET);
      if (padConfg0.r.PMode == 2) { // ISH I2C mode
        DEBUG ((DEBUG_INFO, "IshI2c1PullUp is disabled, Config ISH_I2C1_SDA GPIO!!\n"));
        padConfg1.padCnf1 = GpioPadRead(W_GPIO_136 + BXT_GPIO_PAD_CONF1_OFFSET); 
        padConfg1.r.Term = 0; // Pull None
        GpioPadWrite(W_GPIO_136 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
      }
      padConfg0.padCnf0 = GpioPadRead(W_GPIO_137 + BXT_GPIO_PAD_CONF0_OFFSET);
      if (padConfg0.r.PMode == 2) { // ISH I2C mode
        DEBUG ((DEBUG_INFO, "IshI2c1PullUp is disabled, Config ISH_I2C1_SCL GPIO!!\n"));
        padConfg1.padCnf1 = GpioPadRead(W_GPIO_137 + BXT_GPIO_PAD_CONF1_OFFSET); 
        padConfg1.r.Term = 0; // Pull None
        GpioPadWrite(W_GPIO_137 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
      }
    }

    if ((SystemConfiguration.LowPowerS0Idle == TRUE)) {

        //
        // LPC clock 0 for S0Ix
        //
        padConfg0.padCnf0 = GpioPadRead(SW_LPC_CLKOUT0 + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 1) { // LPC clock mode
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config LPC_CLKOUT0 GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(SW_LPC_CLKOUT0 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSTerm = SAME; //Same as functional pull
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(SW_LPC_CLKOUT0 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
        //
        // LPC clock 1 for S0Ix
        //
        padConfg0.padCnf0 = GpioPadRead(SW_LPC_CLKOUT1 + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 1) { // LPC clock mode
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config LPC_CLKOUT1 GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(SW_LPC_CLKOUT1 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSTerm = SAME; //Same as functional pull
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(SW_LPC_CLKOUT1 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
        
        //
        // LPC clock run for S0Ix
        //
        padConfg0.padCnf0 = GpioPadRead(SW_LPC_CLKRUN + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 1) { // LPC clock run
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config LPC_RUN_N GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(SW_LPC_CLKRUN + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSTerm = SAME; //Same as functional pull
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(SW_LPC_CLKRUN + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
        //
        // ISH I2C for S0Ix
        // GPIO_134 -> ISH_I2C0_SDA
        // GPIO_135 -> ISH_I2C0_SCL
        // GPIO_136 -> ISH_I2C1_SDA
        // GPIO_137 -> ISH_I2C1_SCL
        //
        padConfg0.padCnf0 = GpioPadRead(W_GPIO_134 + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 2) { // ISH I2C mode
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config ISH_I2C0_SDA GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(W_GPIO_134 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(W_GPIO_134 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
        padConfg0.padCnf0 = GpioPadRead(W_GPIO_135 + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 2) { // ISH I2C mode
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config ISH_I2C0_SCL GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(W_GPIO_135 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(W_GPIO_135 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
        padConfg0.padCnf0 = GpioPadRead(W_GPIO_136 + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 2) { // ISH I2C mode
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config ISH_I2C1_SDA GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(W_GPIO_136 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(W_GPIO_136 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
        padConfg0.padCnf0 = GpioPadRead(W_GPIO_137 + BXT_GPIO_PAD_CONF0_OFFSET);
        if (padConfg0.r.PMode == 2) { // ISH I2C mode
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config ISH_I2C1_SCL GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(W_GPIO_137 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(W_GPIO_137 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }

        //
        // SATA GP22 for S0Ix ??
        //
#ifdef APOLLOLAKE_CRB
        if (!IsIOTGBoardIds ()) {
          DEBUG ((DEBUG_INFO, "LowPowerS0Idle is enabled, Config SATA GP22 GPIO!!\n"));
          padConfg1.padCnf1 = GpioPadRead(N_GPIO_22 + BXT_GPIO_PAD_CONF1_OFFSET); 
          padConfg1.r.IOSState = IOS_Masked; // IO Standby signal is masked for this pad.
          GpioPadWrite(N_GPIO_22 + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
        }
#endif
    }

#ifdef NPK_GPIO_SUPPORT  
    DEBUG ((DEBUG_INFO, "Start NPK GPIO Init PtiMode= %x and Destination= %x \n",SystemConfiguration.PtiMode, SystemConfiguration.FwTraceDestination));
    if (SystemConfiguration.FwTraceDestination == 4) { // Only Configure northpeak GPIO when destination is PTI 
      if (SystemConfiguration.PtiMode == 1) {
        GpioPadConfigTable (sizeof (NorthPeakGpio_x4) / sizeof (NorthPeakGpio_x4[0]), NorthPeakGpio_x4);
      } else if (SystemConfiguration.PtiMode == 2) {
        GpioPadConfigTable (sizeof (NorthPeakGpio_x8) / sizeof (NorthPeakGpio_x8[0]), NorthPeakGpio_x8);
      } else if (SystemConfiguration.PtiMode == 8) {
        GpioPadConfigTable (sizeof (NorthPeakGpio_x16) / sizeof (NorthPeakGpio_x16[0]), NorthPeakGpio_x16);
      }
    }
#endif
  } else {
#ifdef NPK_GPIO_SUPPORT
    GpioPadConfigTable (sizeof (NorthPeakGpio_x8) / sizeof (NorthPeakGpio_x8[0]), NorthPeakGpio_x8);
#endif
  }
  return EFI_SUCCESS;
}
//[-end-161107-IB07400810-add]//
//[-end-160413-IB03090426-modify]//
