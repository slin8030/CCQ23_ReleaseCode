/** @file
  This sample application bases on HelloWorld PCD setting 
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2008, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>

#include <Guid/PlatformInfo.h>
#include <PlatformBaseAddresses.h>
#include <ScAccess.h>

typedef struct {
  CHAR16  PadName[50];
  CHAR16  Usage[10];
  CHAR16  GpioDefaut[8];
  CHAR16  Func[8];
  CHAR16  IntType[8];
  CHAR16  Pull[8];
  UINT16  MmioOffset;
  UINT32  PadConfig;
  UINT32  PadValue;
} GPIO_TALBE_INFO;

GPIO_TALBE_INFO mGpioScInfo[] = {
  // PadName                     Usage GpioDefaut Func IntType Pull MmioOffset PadConfig PadValue
  {L"SATA_GP0         GPIOC_0  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x55      ,0        ,0},
  {L"SATA_GP1         GPIOC_1  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x59      ,0        ,0},
  {L"SATA_LEDN        GPIOC_2  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x5d      ,0        ,0},
  {L"PCIE_CLKREQ0B    GPIOC_3  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x60      ,0        ,0},
  {L"PCIE_CLKREQ1B    GPIOC_4  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x63      ,0        ,0},
  {L"PCIE_CLKREQ2B    GPIOC_5  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x66      ,0        ,0},
  {L"PCIE_CLKREQ3B    GPIOC_6  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x62      ,0        ,0},
  {L"PCIE_CLKREQ4B    GPIOC_7  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x65      ,0        ,0},
  {L"HDA_RSTB         GPIOC_8  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x22      ,0        ,0},
  {L"HDA_SYNC         GPIOC_9  " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x25      ,0        ,0},
  {L"HDA_CLK          GPIOC_10 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x24      ,0        ,0},
  {L"HDA_SDO          GPIOC_11 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x26      ,0        ,0},
  {L"HDA_SDI0         GPIOC_12 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x27      ,0        ,0},
  {L"HDA_SDI1         GPIOC_13 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x23      ,0        ,0},
  {L"HDA_DOCKRSTB     GPIOC_14 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x28      ,0        ,0},
  {L"HDA_DOCKENB      GPIOC_15 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x54      ,0        ,0},
  {L"SDMMC1_CLK       GPIOC_16 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3e      ,0        ,0},
  {L"SDMMC1_D0        GPIOC_17 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3d      ,0        ,0},
  {L"SDMMC1_D1        GPIOC_18 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x40      ,0        ,0},
  {L"SDMMC1_D2        GPIOC_19 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3b      ,0        ,0},
  {L"SDMMC1_D3_CD_B   GPIOC_20 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x36      ,0        ,0},
  {L"MMC1_D4_SD_WE    GPIOC_21 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x38      ,0        ,0},
  {L"MMC1_D5          GPIOC_22 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3c      ,0        ,0},
  {L"MMC1_D6          GPIOC_23 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x37      ,0        ,0},
  {L"MMC1_D7          GPIOC_24 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3f      ,0        ,0},
  {L"SDMMC1_CMD       GPIOC_25 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x39      ,0        ,0},
  {L"MMC1_RESET_B     GPIOC_26 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x33      ,0        ,0},
  {L"SDMMC2_CLK       GPIOC_27 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x32      ,0        ,0},
  {L"SDMMC2_D0        GPIOC_28 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x35      ,0        ,0},
  {L"SDMMC2_D1        GPIOC_29 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x2f      ,0        ,0},
  {L"SDMMC2_D2        GPIOC_30 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x34      ,0        ,0},
  {L"SDMMC2_D3_CD_B   GPIOC_31 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x31      ,0        ,0},
  {L"SDMMC2_CMD       GPIOC_32 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x30      ,0        ,0},
  {L"SDMMC3_CLK       GPIOC_33 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x2b      ,0        ,0},
  {L"SDMMC3_D0        GPIOC_34 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x2e      ,0        ,0},
  {L"SDMMC3_D1        GPIOC_35 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x29      ,0        ,0},
  {L"SDMMC3_D2        GPIOC_36 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x2d      ,0        ,0},
  {L"SDMMC3_D3        GPIOC_37 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x2a      ,0        ,0},
  {L"SDMMC3_CD_B      GPIOC_38 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3a      ,0        ,0},
  {L"SDMMC3_CMD       GPIOC_39 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x2c      ,0        ,0},
  {L"SDMMC3_1P8_EN    GPIOC_40 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x5f      ,0        ,0},
  {L"SDMMC3_PWR_EN_B  GPIOC_41 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x69      ,0        ,0},
  {L"LPC_AD0          GPIOC_42 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x46      ,0        ,0},
  {L"LPC_AD1          GPIOC_43 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x44      ,0        ,0},
  {L"LPC_AD2          GPIOC_44 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x43      ,0        ,0},
  {L"LPC_AD3          GPIOC_45 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x42      ,0        ,0},
  {L"LPC_FRAMEB       GPIOC_46 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x45      ,0        ,0},
  {L"LPC_CLKOUT0      GPIOC_47 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x47      ,0        ,0},
  {L"LPC_CLKOUT1      GPIOC_48 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x41      ,0        ,0},
  {L"LPC_CLKRUNB      GPIOC_49 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x48      ,0        ,0},
  {L"ILB_SERIRQ       GPIOC_50 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x56      ,0        ,0},
  {L"SMB_DATA         GPIOC_51 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x5a      ,0        ,0},
  {L"SMB_CLK          GPIOC_52 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x58      ,0        ,0},
  {L"SMB_ALERTB       GPIOC_53 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x5c      ,0        ,0},
  {L"SPKR             GPIOC_54 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x67      ,0        ,0},
  {L"MHSI_ACDATA      GPIOC_55 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x4d      ,0        ,0},
  {L"MHSI_ACFLAG      GPIOC_56 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x4f      ,0        ,0},
  {L"MHSI_ACREADY     GPIOC_57 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x53      ,0        ,0},
  {L"MHSI_ACWAKE      GPIOC_58 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x4e      ,0        ,0},
  {L"MHSI_CADATA      GPIOC_59 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x51      ,0        ,0},
  {L"MHSI_CAFLAG      GPIOC_60 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x50      ,0        ,0},
  {L"MHSI_CAREADY     GPIOC_61 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x52      ,0        ,0},
  {L"GP_SSP_2_CLK     GPIOC_62 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0d      ,0        ,0},
  {L"GP_SSP_2_FS      GPIOC_63 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0c      ,0        ,0},
  {L"GP_SSP_2_RXD     GPIOC_64 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0f      ,0        ,0},
  {L"GP_SSP_2_TXD     GPIOC_65 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0e      ,0        ,0},
  {L"SPI1_CS0_B       GPIOC_66 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x11      ,0        ,0},
  {L"SPI1_MISO        GPIOC_67 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x12      ,0        ,0},
  {L"SPI1_MOSI        GPIOC_68 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x13      ,0        ,0},
  {L"SPI1_CLK         GPIOC_69 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x10      ,0        ,0},
  {L"UART1_RXD        GPIOC_70 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x02      ,0        ,0},
  {L"UART1_TXD        GPIOC_71 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x01      ,0        ,0},
  {L"UART1_RTS_B      GPIOC_72 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x00      ,0        ,0},
  {L"UART1_CTS_B      GPIOC_73 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x04      ,0        ,0},
  {L"UART2_RXD        GPIOC_74 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x06      ,0        ,0},
  {L"UART2_TXD        GPIOC_75 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x07      ,0        ,0},
  {L"UART2_RTS_B      GPIOC_76 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x09      ,0        ,0},
  {L"UART2_CTS_B      GPIOC_77 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x08      ,0        ,0},
  {L"I2C0_SDA         GPIOC_78 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x21      ,0        ,0},
  {L"I2C0_SCL         GPIOC_79 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x20      ,0        ,0},
  {L"I2C1_SDA         GPIOC_80 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1f      ,0        ,0},
  {L"I2C1_SCL         GPIOC_81 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1e      ,0        ,0},
  {L"I2C2_SDA         GPIOC_82 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1d      ,0        ,0},
  {L"I2C2_SCL         GPIOC_83 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1b      ,0        ,0},
  {L"I2C3_SDA         GPIOC_84 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x19      ,0        ,0},
  {L"I2C3_SCL         GPIOC_85 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1c      ,0        ,0},
  {L"I2C4_SDA         GPIOC_86 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1a      ,0        ,0},
  {L"I2C4_SCL         GPIOC_87 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x17      ,0        ,0},
  {L"I2C5_SDA         GPIOC_88 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x15      ,0        ,0},
  {L"I2C5_SCL         GPIOC_89 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x14      ,0        ,0},
  {L"I2C6_SDA         GPIOC_90 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x18      ,0        ,0},
  {L"I2C6_SCL         GPIOC_91 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x16      ,0        ,0},
  {L"I2C_NFC_SDA      GPIOC_92 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x05      ,0        ,0},
  {L"I2C_NFC_SCL      GPIOC_93 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x03      ,0        ,0},
  {L"PWM0             GPIOC_94 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0a      ,0        ,0},
  {L"PWM1             GPIOC_95 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0b      ,0        ,0},
  {L"PLT_CLK0         GPIOC_96 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x6a      ,0        ,0},
  {L"PLT_CLK1         GPIOC_97 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x57      ,0        ,0},
  {L"PLT_CLK2         GPIOC_98 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x5b      ,0        ,0},
  {L"PLT_CLK3         GPIOC_99 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x68      ,0        ,0},
  {L"PLT_CLK4         GPIOC_100" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x61      ,0        ,0},
  {L"PLT_CLK5         GPIOC_101" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x64      ,0        ,0}
};

GPIO_TALBE_INFO mGpioNcInfo[] = {
  // PadName                     Usage GpioDefaut Func IntType Pull MmioOffset PadConfig PadValue
  {L"HV_DDI0_HPD      GPIONC_0 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x13      ,0        ,0},
  {L"HV_DDI0_DDC_SDA  GPIONC_1 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x12      ,0        ,0},
  {L"HV_DDI0_DDC_SCL  GPIONC_2 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x11      ,0        ,0},
  {L"PANEL0_VDDEN     GPIONC_3 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x14      ,0        ,0},
  {L"PANEL0_BKLTEN    GPIONC_4 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x15      ,0        ,0},
  {L"PANEL0_BKLTCTL   GPIONC_5 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x16      ,0        ,0},
  {L"HV_DDI1_HPD      GPIONC_6 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x18      ,0        ,0},
  {L"HV_DDI1_DDC_SDA  GPIONC_7 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x19      ,0        ,0},
  {L"HV_DDI1_DDC_SCL  GPIONC_8 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x17      ,0        ,0},
  {L"PANEL1_VDDEN     GPIONC_9 " ,L""  ,L""       ,L"" ,L""    ,L"" ,0x10      ,0        ,0},
  {L"PANEL1_BKLTEN    GPIONC_10" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0e      ,0        ,0},
  {L"PANEL1_BKLTCTL   GPIONC_11" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0f      ,0        ,0},
  {L"GP_INTD_DSI_TE1  GPIONC_12" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0c      ,0        ,0},
  {L"HV_DDI2_DDC_SDA  GPIONC_13" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1a      ,0        ,0},
  {L"HV_DDI2_DDC_SCL  GPIONC_14" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1b      ,0        ,0},
  {L"GP_CAMERASB00    GPIONC_15" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x01      ,0        ,0},
  {L"GP_CAMERASB01    GPIONC_16" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x04      ,0        ,0},
  {L"GP_CAMERASB02    GPIONC_17" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x08      ,0        ,0},
  {L"GP_CAMERASB03    GPIONC_18" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0b      ,0        ,0},
  {L"GP_CAMERASB04    GPIONC_19" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x00      ,0        ,0},
  {L"GP_CAMERASB05    GPIONC_20" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x03      ,0        ,0},
  {L"GP_CAMERASB06    GPIONC_21" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x06      ,0        ,0},
  {L"GP_CAMERASB07    GPIONC_22" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0a      ,0        ,0},
  {L"GP_CAMERASB08    GPIONC_23" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0d      ,0        ,0},
  {L"GP_CAMERASB09    GPIONC_24" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x02      ,0        ,0},
  {L"GP_CAMERASB10    GPIONC_25" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x05      ,0        ,0},
  {L"GP_CAMERASB11    GPIONC_26" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x09      ,0        ,0}
};


GPIO_TALBE_INFO mGpioSusInfo[]= {
  // PadName                     Usage GpioDefaut Func IntType Pull MmioOffset PadConfig PadValue
  {L"PMIC_INT         GPIO_SUS0" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1d      ,0        ,0},
  {L"FINGERPRINT_INT  GPIO_SUS1" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x21      ,0        ,0},
  {L"CHG_INT          GPIO_SUS2" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1e      ,0        ,0},
  {L"SENS_INT1        GPIO_SUS3" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1f      ,0        ,0},
  {L"AUD_INT          GPIO_SUS4" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x20      ,0        ,0},
  {L"WIFI_32K_CLK     GPIO_SUS5" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x22      ,0        ,0},
  {L"GYRO_R_INT1      GPIO_SUS6" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x24      ,0        ,0},
  {L"GPS_32K_CLK      GPIO_SUS7" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x23      ,0        ,0},
  {L"NFC_HOST_INT     GPIO_SUS8" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x26      ,0        ,0},
  {L"NFC_FW_RESET     GPIO_SUS9" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x25      ,0        ,0},
  {L"NFC_ENABLE_N    GPIO_SUS10" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x12      ,0        ,0},
  {L"SUSPWRDNACK       GPIOS_11" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x07      ,0        ,0},
  {L"PMU_SUSCLK        GPIOS_12" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0b      ,0        ,0},
  {L"PMU_SLP_S0IX_B    GPIOS_13" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x14      ,0        ,0},
  {L"PMU_SLP_LAN_B     GPIOS_14" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x11      ,0        ,0},
  {L"PMU_WAKE_B        GPIOS_15" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x01      ,0        ,0},
  {L"PMU_PWRBTN_B      GPIOS_16" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x08      ,0        ,0},
  {L"PMU_WAKE_LAN_B    GPIOS_17" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0a      ,0        ,0},
  {L"SUS_STAT_B        GPIOS_18" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x13      ,0        ,0},
  {L"USB_OC0_B         GPIOS_19" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x0c      ,0        ,0},
  {L"USB_OC1_B         GPIOS_20" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x00      ,0        ,0},
  {L"SPI_CS1_B         GPIOS_21" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x02      ,0        ,0},
  {L"GPIO_DFX0         GPIOS_22" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x17      ,0        ,0},
  {L"GPIO_DFX1         GPIOS_23" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x27      ,0        ,0},
  {L"GPIO_DFX2         GPIOS_24" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1c      ,0        ,0},
  {L"GPIO_DFX3         GPIOS_25" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1b      ,0        ,0},
  {L"GPIO_DFX4         GPIOS_26" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x16      ,0        ,0},
  {L"GPIO_DFX5         GPIOS_27" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x15      ,0        ,0},
  {L"GPIO_DFX6         GPIOS_28" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x18      ,0        ,0},
  {L"GPIO_DFX7         GPIOS_29" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x19      ,0        ,0},
  {L"GPIO_DFX8         GPIOS_30" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x1a      ,0        ,0},
  {L"USB_ULPI_0_CLK    GPIOS_31" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x33      ,0        ,0},
  {L"USB_ULPI_0_DATA0  GPIOS_32" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x38      ,0        ,0},
  {L"USB_ULPI_0_DATA1  GPIOS_33" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x36      ,0        ,0},
  {L"USB_ULPI_0_DATA2  GPIOS_34" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x31      ,0        ,0},
  {L"USB_ULPI_0_DATA3  GPIOS_35" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x37      ,0        ,0},
  {L"USB_ULPI_0_DATA4  GPIOS_36" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x30      ,0        ,0},
  {L"USB_ULPI_0_DATA5  GPIOS_37" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x39      ,0        ,0},
  {L"USB_ULPI_0_DATA6  GPIOS_38" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x32      ,0        ,0},
  {L"USB_ULPI_0_DATA7  GPIOS_39" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3a      ,0        ,0},
  {L"USB_ULPI_0_DIR    GPIOS_40" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x34      ,0        ,0},
  {L"USB_ULPI_0_NXT    GPIOS_41" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x35      ,0        ,0},
  {L"USB_ULPI_0_STP    GPIOS_42" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x3b      ,0        ,0},
  {L"USB_ULPI_0_REFCLK GPIOS_43" ,L""  ,L""       ,L"" ,L""    ,L"" ,0x28      ,0        ,0}
};

VOID
DumpGpio (
  IN UINT32           GpioGropuOffset,
  IN UINT32           PinNumber,
  IN GPIO_TALBE_INFO* GpioInfoData
  )
{
  UINT32     PadValue;
  UINT32     Usage;
  UINT32     DefaultVale;
  UINT32     PadConfigTemp;
  PAD_CONF0  PadConfig;
  UINT8      IntType;
  UINT8      Index;
  CHAR16     Title[50];

  switch(GpioGropuOffset){
  case GPIO_SCORE_OFFSET:
    StrCpy (Title, L"GPIO SouthCore Dump");
    break;
    
  case GPIO_NCORE_OFFSET:
    StrCpy (Title, L"GPIO NorthCore Dump");
    break;
    
  case GPIO_SSUS_OFFSET:
    StrCpy (Title, L"GPIO  Suspend  Dump");
    break;

  }
  Print (L"=======================================================================================================================================\n");  
  Print (L"                                                       %s\n", Title);
  Print (L"        PadName              UsageAs   GpioDefaut   Function#  InterruptType  PULL_H/L  MmioOffset  MemAddr     PadConfig   PadValue   \n");
  Print (L"=======================================================================================================================================\n");
  
  //
  // Get GPIO Pin(Pad) Data (Offset 0x08 CFIO PAD_VAL)
  //
  for (Index = 0; Index < PinNumber;Index++){
    Print (L"%s   ,",GpioInfoData[Index].PadName);
    PadValue =  MmioRead32(IO_BASE_ADDRESS + GpioGropuOffset + R_PCH_CFIO_PAD_VAL + GpioInfoData[Index].MmioOffset*16);
    GpioInfoData[Index].PadValue=PadValue;

    //
    // Usage
    //
    Usage = PadValue & 0x06;  //Bit 1:2 for Usage
    if (Usage == 0x00) {
      StrCpy (GpioInfoData[Index].Usage,L"GPIO  ");
    } else if (Usage == 0x02) {
      StrCpy (GpioInfoData[Index].Usage,L"GPI   ");
    } else if (Usage == 0x04) {
      StrCpy (GpioInfoData[Index].Usage,L"GPO   ");
    } else {
      StrCpy (GpioInfoData[Index].Usage,L"Native");
    }
    Print (L"%s   ,",GpioInfoData[Index].Usage);
  
    //
    // Default Value
    //
    if (Usage==0x04) {
      //
      // If the GPIO is GPO, get the default value.
      //
      DefaultVale = PadValue & BIT0;
      if (DefaultVale == 0x00) {
        StrCpy (GpioInfoData[Index].GpioDefaut, L"LO");
      } else if (DefaultVale == 0x01) {
        StrCpy (GpioInfoData[Index].GpioDefaut, L"HI");
      }
    } else {
      StrCpy (GpioInfoData[Index].GpioDefaut, L"NA");
    }
  
    Print (L"%s          ,",GpioInfoData[Index].GpioDefaut);
    
    //
    // Get GPIO Pin(Pad) Configuration Data (Offset 0x00 CFIO PAD_VAL)
    //
    PadConfigTemp = MmioRead32(IO_BASE_ADDRESS + GpioGropuOffset + R_PCH_CFIO_PAD_CONF0 + GpioInfoData[Index].MmioOffset * 16);
    GpioInfoData[Index].PadConfig = PadConfigTemp;
    CopyMem (&PadConfig, &PadConfigTemp, sizeof(UINT32));
  
    //
    // Function
    //
    if (PadConfig.r.Func_Pin_Mux == 0) {
      StrCpy (GpioInfoData[Index].Func, L"F0");
    } else if (PadConfig.r.Func_Pin_Mux == 1) {
      StrCpy (GpioInfoData[Index].Func, L"F1");
    } else if (PadConfig.r.Func_Pin_Mux == 2) {
      StrCpy (GpioInfoData[Index].Func, L"F2");
    } else if (PadConfig.r.Func_Pin_Mux == 3) {
      StrCpy (GpioInfoData[Index].Func, L"F3");
    } else if (PadConfig.r.Func_Pin_Mux == 4) {
      StrCpy (GpioInfoData[Index].Func, L"F4");
    } else if (PadConfig.r.Func_Pin_Mux == 5) {
      StrCpy (GpioInfoData[Index].Func, L"F5");
    } else if (PadConfig.r.Func_Pin_Mux == 6) {
      StrCpy (GpioInfoData[Index].Func, L"F6");
    } else if (PadConfig.r.Func_Pin_Mux == 7) {
      StrCpy (GpioInfoData[Index].Func, L"F7");
    }
    Print (L"%s        ,",GpioInfoData[Index].Func);
    
    //
    // Interrupt Type
    //
    IntType = (UINT8)((PadConfig.dw & 0x0f000000) >>24);
    if ((IntType & BIT2) && IntType & BIT0) {
       StrCpy (GpioInfoData[Index].IntType, L"Level_Low ");
    } else if ((IntType & BIT1) && IntType & BIT0) {
       StrCpy (GpioInfoData[Index].IntType, L"Level_High");
    } else if ((IntType&BIT2) && IntType&BIT1) {
       StrCpy (GpioInfoData[Index].IntType, L"Edge_Both ");
    } else if (IntType&BIT2) {
       StrCpy (GpioInfoData[Index].IntType, L"Edge_Low  ");
    } else if (IntType&BIT1) {
       StrCpy (GpioInfoData[Index].IntType, L"Edge_High ");
    } else {
       StrCpy (GpioInfoData[Index].IntType, L"          ");
    }
    Print (L"%s    ,",GpioInfoData[Index].IntType);
    
    //
    // Pull Assign and Pull Strength
    //
    if (PadConfig.r.Pull_assign == 0) {
      StrCpy (GpioInfoData[Index].Pull, L"NONE ");
    } else if ((PadConfig.r.Pull_assign == 0x1) && (PadConfig.r.Pull_strength == 0x2)) {
      StrCpy (GpioInfoData[Index].Pull, L"20K_H");
    } else if ((PadConfig.r.Pull_assign == 0x2) && (PadConfig.r.Pull_strength == 0x2)) {
      StrCpy (GpioInfoData[Index].Pull, L"20K_L");
    }

    Print (L"%s    ,",GpioInfoData[Index].Pull);
    Print (L"0x%04x",(GpioInfoData[Index].MmioOffset));
    
    Print (L"     ,0x%08x", (UINTN)(IO_BASE_ADDRESS + GpioGropuOffset + R_PCH_CFIO_PAD_CONF0)+(GpioInfoData[Index].MmioOffset * 16));
    Print (L" ,0x%08x", GpioInfoData[Index].PadConfig);
    Print (L" ,0x%08x", GpioInfoData[Index].PadValue);
    
    Print (L"\n");
  }
  Print (L"=======================================================================================================================================\n");

}

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
GpioDumpMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  DumpGpio(GPIO_SCORE_OFFSET, (sizeof(mGpioScInfo)/sizeof(GPIO_TALBE_INFO)) , mGpioScInfo);
  Print (L"\n");
  DumpGpio(GPIO_NCORE_OFFSET, (sizeof(mGpioNcInfo)/sizeof(GPIO_TALBE_INFO)) , mGpioNcInfo);
  Print (L"\n");
  DumpGpio(GPIO_SSUS_OFFSET , (sizeof(mGpioSusInfo)/sizeof(GPIO_TALBE_INFO)), mGpioSusInfo);
  Print (L"\n");

  return EFI_SUCCESS;
}
