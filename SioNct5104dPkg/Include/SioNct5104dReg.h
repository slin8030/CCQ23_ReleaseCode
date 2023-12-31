//;******************************************************************************
//;* Copyright (c) 1983-2014, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************

#ifndef _SIO_NCT5104D_H_
#define _SIO_NCT5104D_H_

#include <SioCommon.h>

#define SIO_DXE_DRIVER_GUID \
  { 0xEC2A99BD, 0x3A69, 0x4b0d, 0x90, 0x2B, 0x06, 0x47, 0xC6, 0xC2, 0xFD, 0x37 }

//
// Sio common registers
//
#define SIO_SW_RESET                SIO_CR02
#define SIO_LDN                     SIO_CR07
#define SIO_IRQ_TYPE_SEL1           SIO_CR10
#define SIO_IRQ_TYPE_SEL2           SIO_CR11
#define SIO_IRQ_POLARITY_SEL1       SIO_CR13
#define SIO_IRQ_POLARITY_SEL2       SIO_CR14
#define SIO_MULTI_FUN_SEL_REG1      SIO_CR1C

#define SIO_CHIP_ID1                SIO_CR20
#define SIO_CHIP_ID2                SIO_CR21
#define SIO_POWER_DOWN              SIO_CR22
#define SIO_MULTI_FUN_SEL_REG2      SIO_CR24
#define SIO_GLOBAL_OPTION1          SIO_CR24
#define SIO_INTERFACE_TRI_EN        SIO_CR25
#define SIO_GLOBAL_OPTION2          SIO_CR26
#define SIO_GLOBAL_OPTION3          SIO_CR27
#define SIO_GLOBAL_OPTION4          SIO_CR29
#define SIO_STRAPPING_FUN_RESULT    SIO_CR2F

#define SIO_DEV_ACTIVE              SIO_CR30
#define SIO_BASE_IO_ADDR1_MSB       SIO_CR60
#define SIO_BASE_IO_ADDR1_LSB       SIO_CR61
#define SIO_BASE_IO_ADDR2_MSB       SIO_CR62
#define SIO_BASE_IO_ADDR2_LSB       SIO_CR63
#define SIO_IRQ_SET                 SIO_CR70
#define SIO_IRQ_TYPE_SEL            SIO_CR71
#define SIO_MOUSE_IRQ_SET           SIO_CR72
#define SIO_DMA_SET0                SIO_CR74
#define SIO_DMA_SET1                SIO_CR75

#define DMA_CHANNEL_DISABLE         0xFF

//
// Logical Device Number
//
#define SIO_COM1                    0x02
#define SIO_COM2                    0x03
#define SIO_GPIO_SET0               0x07
#define SIO_GPIO_SET1               0x08
#define SIO_WDT                     0x08
#define SIO_GPIO_PUSH               0x0F
#define SIO_COM3                    0x10
#define SIO_COM4                    0x11
#define SIO_PROT80                  0x14


#define SIO_COM3                    0x10
#define SIO_COM4                    0x11
#define SIO_COM5                    0x12
#define SIO_COM6                    0x13 
#define SIO_PORT80                  0x14
#define SIO_IR                      0x14
#define SIO_FADING_LED              0x15
#define SIO_DEEP_SLEEP              0x16

//
// Watch dog timer
//
#define SIO_WDT_CONTROL_MODE_REG    0xF0
#define SIO_WDT_COUNTER_REG         0xF1
#define SIO_WDT_STATUS_CMOS_INDEX   0x48
#define WDT_ENABLE_CMOS_STATUS_BIT  BIT0
#define WDAT_MINIMUM_SECONDS        0x50
#define DEFAULT_TIME_OUT_VALUE      0x20

//
// Define Super I/O device ID for verify component chip
//
#define SIO_ID1                     0xC4   
#define SIO_ID2                     0x52    //if the value set 0x00, it's mean we just check SIO ID1

//
// This define is for enter and exit SIO config
//
#define ENTER_CONFIG                0x87
#define EXIT_CONFIG                 0xAA

//
// Com port I/O address
//
#define COM_PORT                    0x3F8
#define COM_PORT2                   0x2F8
#define COM_PORT3                   0x3E8
#define COM_PORT4                   0x2E8

//
// Ps2 Kbc I/O address
//
#define PS2_KB_MS                   0x60
#define SIO_KBC_TIMEOUT             50000
#define SIO_KBC_DATA_PORT           0x60
#define SIO_KBC_STATUS_PORT         0x64

//
// KBC Status Register
//
#define B_SIO_KBC_OBF_FULL       BIT0
#define  V_SIO_KBC_OBF_EMPTY     0x00
#define  V_SIO_KBC_OBF_FULL      BIT0
#define B_SIO_KBC_IBF_FULL       BIT1
#define  V_SIO_KBC_IBF_EMPTY     0x00
#define  V_SIO_KBC_IBF_FULL      BIT1
#define B_SIO_KBC_SYS_FLAG       BIT2
#define B_SIO_KBC_CMD_DAT_BYTE   BIT3
#define  V_SIO_KBC_DATA_BYTE     0x00
#define  V_SIO_KBC_CMD_BYTE      BIT3
#define B_SIO_KBC_INH_SWITCH     BIT4
#define B_SIO_KBC_AUX_OBF        BIT5
#define B_SIO_KBC_GP_TIME_OUT    BIT6
#define B_SIO_KBC_PARITY_ERROR   BIT7

//
// KBC Command
//
#define SIO_KBC_SELF_TEST        0xAA


//
// To ASL device, define you need
//
#pragma pack (1)
typedef struct {
  EFI_ASL_RESOURCE  DeviceA; ///< Com1
  EFI_ASL_RESOURCE  DeviceB; ///< Com2
  EFI_ASL_RESOURCE  DeviceC; ///< Com3
  EFI_ASL_RESOURCE  DeviceD; ///< Com4

} SIO_NVS_AREA;
#pragma pack ()

// Hwm registers
#define HARDWARE_MONITOR_FAN_SPEED_COUNT 1500000

#endif
