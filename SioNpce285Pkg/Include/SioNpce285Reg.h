//;******************************************************************************
//;* Copyright (c) 1983-2015, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************

#ifndef _SIO_NPCE285_H_
#define _SIO_NPCE285_H_

#include <SioCommon.h>

#define SIO_DXE_DRIVER_GUID \
  { 0xDB35BFD1, 0x0487, 0x4A0C, 0x90, 0x75, 0x9B, 0x35, 0x39, 0x1A, 0x21, 0x7A }

//
// Sio common registers
//
#define SIO_CONGIG_CTRL             SIO_CR02
#define SIO_LDN                     SIO_CR07

#define SIO_CHIP_ID1                SIO_CR20
#define SIO_CHIP_ID2                SIO_CR21


#define SIO_DEV_ACTIVE              SIO_CR30
#define SIO_BASE_IO_ADDR1_MSB       SIO_CR60
#define SIO_BASE_IO_ADDR1_LSB       SIO_CR61
#define SIO_BASE_IO_ADDR2_MSB       SIO_CR62
#define SIO_BASE_IO_ADDR2_LSB       SIO_CR63
#define SIO_IRQ_SET                 SIO_CR70
#define SIO_IRQ_TYPE                SIO_CR71
#define SIO_DMA_SET0                SIO_CR74
#define SIO_DMA_SET1                SIO_CR75

#define DMA_CHANNEL_DISABLE         0xFF

//
// Logical Device Number
//
#define SIO_COM1                   0x03
#define SIO_MSWC                   0x04
#define SIO_MOUSE                  0x05
#define SIO_KEYBOARD               0x06
#define SIO_SHM                    0x0F
#define SIO_PM1                    0x11
#define SIO_PM2                    0x12
#define SIO_PM3                    0x17
#define SIO_ESHM                   0x1D

//
// Define Super I/O device ID for verify component chip
//
#define SIO_ID1                     0xFC  
#define SIO_ID2                     0x00    //if the value set 0x00, it's mean we just check SIO ID1

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

//
// To ASL device, define you need
//
#pragma pack (1)
typedef struct {
  EFI_ASL_RESOURCE  DeviceA; ///< Com1
  EFI_ASL_RESOURCE  DeviceI; ///< KYBD
  EFI_ASL_RESOURCE  DeviceJ; ///< MOUSE
} SIO_NVS_AREA;
#pragma pack ()

#endif
