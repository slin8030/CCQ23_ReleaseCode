/** @file
  Definitions for register of the WPCD374L.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Module Name:

  SioWpcd374lReg.h

Abstract:

  Definitions for register of the WPCD374L.

**/

#ifndef _SIO_WPCD374L_H_
#define _SIO_WPCD374L_H_

#include <Sio/SioCommon.h>

#define SIO_DXE_DRIVER_GUID \
  { 0xf4b5117, 0xb7a7, 0x4a64,  0xa7, 0xa5, 0x81, 0xb2, 0x9c, 0x45, 0xb2, 0xb6 }

//
// SIO common registers
//
#define SIO_CONFIG_CTRL             SIO_CR02
#define SIO_LDN                     SIO_CR07
#define SIO_MEM_MAPPING_CTRL        SIO_CR10
#define SIO_MEM_BASE_ADDR1          SIO_CR12
#define SIO_MEM_BASE_ADDR2          SIO_CR13
#define SIO_CHIP_ID1                SIO_CR20
#define SIO_CONFIG_REG1             SIO_CR21
#define SIO_CONFIG_REG2             SIO_CR22
#define SIO_CONFIG_REG3             SIO_CR23
#define SIO_CONFIG_REG4             SIO_CR24
#define SIO_CONFIG_REG6             SIO_CR26
#define SIO_REVERSION_ID            SIO_CR27
#define SIO_CLOCK_CFG               SIO_CR29

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
// Define each local device number
//
#define SIO_FLOPPY                  0x0
#define SIO_LPT1                    0x1
#define SIO_COM2                    0x2
#define SIO_COM1                    0x3
#define SIO_SWC                     0x4
#define SIO_MOUSE                   0x5
#define SIO_KYBD                    0x6
#define SIO_GPIO                    0x7
#define SIO_HW_MONITOR              0x8

//
// Define Super I/O device ID for verify component chip
//
#define SIO_ID1                     0xF1   // hight byte
#define SIO_ID2                     0x00   // Low byte

#define NULL_ID                     0xFF

//
// This define is for enter and exit SIO config
//
#define CFG_PORT                    0x2E
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
// To ASL device, define you need
//
#pragma pack (1)
typedef struct {
  EFI_ASL_RESOURCE  DeviceA; ///< Com1
  EFI_ASL_RESOURCE  DeviceB; ///< Com2
  EFI_ASL_RESOURCE  DeviceC; ///< Floppy
  EFI_ASL_RESOURCE  DeviceD; ///< LPT
  EFI_ASL_RESOURCE  DeviceE; ///< KYBD
  EFI_ASL_RESOURCE  DeviceF; ///< MOUSE
} SIO_NVS_AREA;
#pragma pack ()

#endif
