//;******************************************************************************
//;* Copyright (c) 1983-2015, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************

#ifndef _SIO_IT8987E_H_
#define _SIO_IT8987E_H_

#include <SioCommon.h>

#define SIO_DXE_DRIVER_GUID \
  { 0xfbc9396e, 0xe7ee, 0x49fb, 0xad, 0x47, 0x54, 0xbb, 0xb0, 0x97, 0xf1, 0x9f }

//
// Sio common registers
//
#define SIO_LDN                                                                SIO_CR07

#define SIO_CHIP_ID1                                                           SIO_CR20
#define SIO_CHIP_ID2                                                           SIO_CR21
#define SIO_CHIP_VERSION                                                       SIO_CR22
#define SIO_SUPER_IO_CONTROL_REGISTER                                          SIO_CR23
#define SIO_SUPER_IO_IRQ_CONFIGURATION_REGISER                                 SIO_CR25
#define SIO_SUPER_IO_GENERAL_PURPOSE_REGISTER                                  SIO_CR26
#define SIO_SUPER_IO_POWER_MODE_REGISTER                                       SIO_CR2D
#define SIO_DEPTH_2_IO_ADDRESS                                                 SIO_CR2E
#define SIO_DEPTH_2_IO_DATA                                                    SIO_CR2F

#define SIO_DEV_ACTIVE                                                         SIO_CR30
#define SIO_BASE_IO_ADDR1_MSB                                                  SIO_CR60
#define SIO_BASE_IO_ADDR1_LSB                                                  SIO_CR61
#define SIO_BASE_IO_ADDR2_MSB                                                  SIO_CR62
#define SIO_BASE_IO_ADDR2_LSB                                                  SIO_CR63
#define SIO_IRQ_SET                                                            SIO_CR70
#define SIO_IRQ_TYPE_SEL                                                       SIO_CR71
#define SIO_DMA_SET0                                                           SIO_CR74
#define SIO_DMA_SET1                                                           SIO_CR75

#define DMA_CHANNEL_DISABLE                                                    0xFF

//
// Logical Device Number
//
#define SIO_COM1                                                               0x01
#define SIO_COM2                                                               0x02
#define SIO_SWUC                                                               0x04 // system wake-up control
#define SIO_KYBD                                                               0x05
#define SIO_MOUSE                                                              0x06
#define SIO_CIR                                                                0x0A // consumer IR
#define SIO_SMFI                                                               0x0F // shared memory flash interface
#define SIO_RCTC                                                               0x10 // rtc-like timer
#define SIO_PMC1                                                               0x11 // power management I/F channel 1
#define SIO_PMC2                                                               0x12 // power management I/F channel 2
#define SIO_SSPI                                                               0x13 // serial peripheral interfac
#define SIO_PECI                                                               0x14 // platform environment control interface
#define SIO_PMC3                                                               0x17 // power management I/F channel 3
#define SIO_PMC4                                                               0x18 // power management I/F channel 4
#define SIO_PMC5                                                               0x19 // power management I/F channel 5

//
// Define Super I/O device ID for verify component chip
//
#define SIO_ID1                                                                0x89
#define SIO_ID2                                                                0x87    //if the value is set to 0x00, it means we just check SIO ID1

//
// This define is for entering and exiting SIO config
//
#define ENTER_CONFIG                                                           0x87
#define ENTER_CONFIG_2                                                         0x01
#define ENTER_CONFIG_3                                                         0x55
#define ENTER_CONFIG_4                                                         0xAA
#define EXIT_CONFIG                                                            0x02

//
// Com port I/O address
//
#define COM_PORT                                                               0x3F8
#define COM_PORT2                                                              0x2F8
#define COM_PORT3                                                              0x3E8
#define COM_PORT4                                                              0x2E8

//
// Ps2 Kbc I/O address
//
#define PS2_KB_MS                                                              0x60
#define PS2_KB_IRQ                                                             0x01
#define PS2_MS_IRQ                                                             0x0C

//
// To ASL device
//
#pragma pack (1)
typedef struct {
  EFI_ASL_RESOURCE  DeviceA; ///< Com1
  EFI_ASL_RESOURCE  DeviceB; ///< Com2
  EFI_ASL_RESOURCE  DeviceI; ///< KYBD
  EFI_ASL_RESOURCE  DeviceJ; ///< MOUSE
  EFI_ASL_RESOURCE  DeviceL; ///< CIR
} SIO_NVS_AREA;
#pragma pack ()

#endif
