//;******************************************************************************
//;* Copyright (c) 1983-2006, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;******************************************************************************
//; Revision History:
//;
//; $Log: $
//;
//;------------------------------------------------------------------------------
//;
//; Abstract:
//;

#ifndef _EC_SPI_LIBRARY_H
#define _EC_SPI_LIBRARY_H

//[PRJ] Start - ENE9012 flash
#define ISP_SPIADD0     0xFEA8
#define ISP_SPIADD1     0xFEA9
#define ISP_SPIADD2     0xFEAA
#define ISP_SPIDAT      0xFEAB
#define ISP_SPICMD      0xFEAC
#define ISP_SPICFG      0xFEAD
#define ISP_ISPDATR     0xFEAE
#define ISP_SCON3       0xFEAF
#define ISP_LPCCSR      0xFE9E
#define ISP_ECSTS       0xFF1D
#define ISP_CLKCFG2     0xFF1E 
//[PRJ] End - ENE9012 flash

//[PRJ]Start - EC 901x flash function
#define EFCMD_PageLatch 0x02
#define EFCMD_Read      0x03
#define EFCMD_PageErase 0x20
#define EFCMD_ProgramSP 0x70
#define EFCMD_ClrHVPL   0x80
//[PRJ]End
#define SPI_REMS        0x9F

#define EC_SYSTEM_NOTIFI_CMD               0x59
#define EC_ENTER_WAIT_MODE                 0xF1
#define EC_ENTER_IDLE_MODE                 0xF2

#define ICH_ACPI_TIMER_ADDR       0x0408
#define ICH_ACPI_TIMER_MAX_VALUE  0x1000000 // The timer is 24 bit overflow

EFI_STATUS
EcRecognizeSPIDevice (
  IN VOID                       *SPiFlashDevice
  );

EFI_STATUS
EcEraseSPI (
  IN  UINTN                     FlashAddress,
  IN  UINTN                     Size
  );

EFI_STATUS
EcProgramSPI (
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINTN                     *SpiBufferSize,
  IN  UINTN                     LbaWriteAddress
  );

EFI_STATUS
EcProgramSPI_SST (
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINTN                     *SpiBufferSize,
  IN  UINTN                     LbaWriteAddress
  );


EFI_STATUS
EcReadSPI (
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINTN                     BufferSize
  );

VOID
EcSpiStall (
  IN  UINTN   Microseconds
  );


typedef struct {
  BOOLEAN       Inited;
  BOOLEAN       ConvertPointer;
  UINTN         FlashAddressBase;
  UINTN         FlashAddressDelta;
  UINTN         RcrbRebaseDelta;
} METHOD_SPIINIT;

EFI_STATUS
EcGetSpiPlatformFlashTable (
  IN OUT   UINT8    *DataBuffer
  );

//[PRJ]Start - EC 901x flash function
VOID
EC_WaitSPI_Cycle (
  );

VOID
WriteEcRom (
  IN UINTN  Adr,
  IN UINT8  *Data,
  IN UINTN  Len
);

EFI_STATUS
Stop_SPI ();

EFI_STATUS
Start_SPI ();

VOID
  PageErase (
  IN UINTN Adr
);

VOID
ReadENEData_9012 (
  IN  UINT8  LowAddress,
  IN  UINT8  HighAddress,
  OUT UINT8  *OutData
);
//[PRJ]End
#endif

