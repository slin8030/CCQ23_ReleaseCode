/** @file
  TXE device register definitions

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _TXE_SECURE_BOOT_REGS_H_
#define _TXE_SECURE_BOOT_REGS_H_

#include <Numbers.h>

//
// Refer TXE BWG 8.4.4 Secure boot Status Register
//
#define R_SECURE_BOOT_STS                 0x50
#define B_SECURE_BOOT_EN                  BIT0
#define B_SECURE_BOOT_RECOVERY            BIT1
#define B_SECURE_BOOT_WAS_DEBUG           BIT2
#define B_SECURE_BOOT_DEBUG               BIT3
#define B_SECURE_BOOT_EMULATION           BIT4
#define B_SECURE_BOOT_ALT_BIOS_LIMIT      0xFFF80000

#define SecureBootStatusToAltBiosLimit(SecureBootStatus) \
             (((SecureBootStatus) & B_SECURE_BOOT_ALT_BIOS_LIMIT) >> 19) 

#define SPI_FLASH_SIZE                    0x800000
#define IBB_SIZE                          0x21000
#endif

