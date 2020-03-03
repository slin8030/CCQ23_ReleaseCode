/** @file
  Definitions for SPI access Library Instance

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SPI_ACCESS_H_
#define _SPI_ACCESS_H_
#include <MmioAccess.h>
#include <Library/IoLib.h>
//
// Operation index
//
#define SPI_OPCODE_INDEX_READ_ID        0
#define SPI_OPCODE_INDEX_ERASE          1
#define SPI_OPCODE_INDEX_WRITE          2
#define SPI_OPCODE_INDEX_WRITE_S        3
#define SPI_OPCODE_INDEX_READ           4
#define SPI_OPCODE_INDEX_READ_S         5
#define SPI_OPCODE_INDEX_LOCK           6
#define SPI_OPCODE_INDEX_UNLOCK         7
#define SPI_PREFIX_INDEX_WRITE_EN       0
#define SPI_PREFIX_INDEX_WRITE_S_EN     1

//
// Block lock bit definitions
//
#define SPI_WRITE_LOCK                  0x01
#define SPI_FULL_ACCESS                 0x00


#define SPI_SOFTWARE                                 0
#define SPI_HARDWARE                                 1
#define ME_SIZE_IGNITION_BASIC                       0x10000
#define ME_SIZE_IGNITION_RUNTIME                     0x10000
#define B_PCH_LPC_BIOS_CNTL_SRC                      0x0C

extern UINT8 *mSpiBase;

#define MmSpi32(Register)                            Mmio32 (mSpiBase, Register)
#define MmSpi32Or(Register, OrData)                  Mmio32Or (mSpiBase, Register, OrData)
#define MmSpi32And(Register, AndData)                Mmio32And (mSpiBase, Register, AndData)
#define MmSpi32AndThenOr(Register, AndData, OrData)  Mmio32AndThenOr (mSpiBase, Register, AndData, OrData)
#define MmSpi16(Register)                            Mmio16 (mSpiBase, Register)
#define MmSpi16Or(Register, OrData)                  Mmio16Or (mSpiBase, Register, OrData)
#define MmSpi16And(Register, AndData)                Mmio16And (mSpiBase, Register, AndData)
#define MmSpi16AndThenOr(Register, AndData, OrData)  Mmio16AndThenOr (mSpiBase, Register, AndData, OrData)
#define MmSpi8(Register)                             Mmio8 (mSpiBase, Register)
#define MmSpi8Or(Register, OrData)                   Mmio8Or (mSpiBase, Register, OrData)
#define MmSpi8And(Register, AndData)                 Mmio8And (mSpiBase, Register, AndData)
#define MmSpi8AndThenOr(Register, AndData, OrData)   Mmio8AndThenOr (mSpiBase, Register, AndData, OrData)

#endif
