/** @file
  SPI function implementation

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SpiHost.h>
#include <Protocol/SpiAcpi.h>
#include "SpiRegisters.h"
#include "SpiController.h"

#pragma optimize ("",off)
UINT32 SpiMaxReadBytes = SPI_RX_FIFO_SIZE;
UINT32 SpiMaxWriteBytes = SPI_TX_FIFO_SIZE;
UINT32 SpiMaxTransmitBytes = SPI_TX_FIFO_SIZE;


/**
  SPI Controller Assert CS

  @param [in] pRegisters      A pointer to SPI_REGISTERS instance.
  @param [in] polarity        polarity property of the SPI HW
**/
VOID
ControllerSpiAssertCs (
  IN SPI_REGISTERS            *pRegisters,
  IN UINT32                   polarity
  )
{
  if(polarity != 0)
    pRegisters->PRV_CS_CTRL |= PRV_CS_CTRL_STATE_HI_BIT;
  else
    pRegisters->PRV_CS_CTRL &= ~(UINT32)PRV_CS_CTRL_STATE_HI_BIT;
    return;
}

/**
  SPI Controller DeAssert CS

  @param [in] pRegisters      A pointer to SPI_REGISTERS instance.
  @param [in] polarity        polarity property of the SPI HW
**/
VOID
ControllerSpiDeAssertCs (
  IN SPI_REGISTERS            *pRegisters,
  IN UINT32                   polarity
  )
{
  if(polarity != 0)
    pRegisters->PRV_CS_CTRL &= ~(UINT32)PRV_CS_CTRL_STATE_HI_BIT;
  else
    pRegisters->PRV_CS_CTRL |= PRV_CS_CTRL_STATE_HI_BIT;
    return;
}

/**
  SPI Controller Acknowledge Interrupts

  @param [in] pRegisters      A pointer to SPI_REGISTERS instance.
  @param [in] InterruptMask   Input value for interrupt mask
**/
VOID
ControllerAcknowledgeInterrupts(
  IN SPI_REGISTERS            *pRegisters,
  IN UINT32                   InterruptMask
  )
{
  UINT32 ackMask = 0;

  if(InterruptMask & SSSR_TFS | SSSR_RFS) {
  }
  if(InterruptMask & SSSR_ROR) {
    ackMask |= SSSR_ROR;
  }
  if(InterruptMask & SSSR_PINT) {
     ackMask |= SSSR_PINT;
  }
  if(InterruptMask & SSSR_TINT) {
     ackMask |= SSSR_TINT;
  }
  if(InterruptMask & SSSR_EOC) {
    ackMask |= SSSR_EOC;
  }
  if(InterruptMask & SSSR_TUR) {
    ackMask |= SSSR_TUR;
   }
  if(InterruptMask & SSSR_BCE) {
    ackMask |= SSSR_BCE;
  }
  if (ackMask > 0) {
    pRegisters->SSSR = ackMask;
  }
}

/**
  Disable SPI Controller Interrupts

  @param [in] pRegisters      A pointer to SPI_REGISTERS instance.
**/
VOID
ControllerDisableInterrupts(
   IN SPI_REGISTERS           *pRegisters
   )
{
  pRegisters->SSCR0 |= SPI_INTERRUPT_SSCR0_MASK;
  pRegisters->SSCR1 &= ~SPI_INTERRUPT_SSCR1_MASK;
}


/**
  Execute SPI Reset

  @param [in] platformData    A pointer to SPI_PLATFORM_CONTEXT instance.
**/
VOID SpiReset (
  IN VOID                     *platformData
  )
{
  SPI_PLATFORM_CONTEXT * spc = (SPI_PLATFORM_CONTEXT *)platformData;
  SPI_REGISTERS *pRegisters = (SPI_REGISTERS *)(UINTN)spc->BaseAddress;
  //
  // Release resets
  //
  pRegisters->PRV_RESETS = PRV_RESETS_NOT_FUNC_RST | PRV_RESETS_NOT_APB_RST;

  pRegisters->PRV_CLOCKS = 0;
  pRegisters->PRV_CLOCKS = PRV_CLOCKS_CLK_EN | (PRV_CLOCKS_M_VAL_DEFAULT << PRV_CLOCKS_M_VAL_SHIFT) | (PRV_CLOCKS_N_VAL_DEFAULT << PRV_CLOCKS_N_VAL_SHIFT) | PRV_CLOCKS_UPDATE;
  pRegisters->PRV_CLOCKS &= ~PRV_CLOCKS_UPDATE;
  //
  //enable SW CS mode
  //
  pRegisters->PRV_CS_CTRL |= PRV_CS_CTRL_MODE_SW;
  //
  //ControllerDisableInterrupts
  //
  pRegisters->SSCR0 |= (UINT32)SPI_INTERRUPT_SSCR0_MASK;
  pRegisters->SSCR1 &= ~(UINT32)SPI_INTERRUPT_SSCR1_MASK;
  pRegisters->SSCR0 &= ~(UINT32)SSCR0_SSE_EN;
}

/**
  Execute SPI command Transfer

  @param [in] platformData              A pointer to SPI_PLATFORM_CONTEXT instance.
  @param [in] pTargetSettings           A pointer to SPI_TARGET_SETTINGS instance.
  @param [in, out] RequestPacket        A pointer to EFI_SPI_REQUEST_PACKET instance.
  @param [out] SpiStatus                Status returned by spi write command

  @retval EFI_STATUS                    Spi command transfer suscessfully
  @retval Others                        Spi command transfer failure
**/
EFI_STATUS
SpiTransfer (
  IN VOID                               *platformData,
  IN SPI_TARGET_SETTINGS                *pTargetSettings,
  IN OUT EFI_SPI_REQUEST_PACKET         *RequestPacket,
  OUT EFI_STATUS                        *SpiStatus
  )
{
  UINT32 ctrlr0Value = 0x0;
  UINT32 ctrlr1Value = 0x0;
  static UINT32 i = 0;
  SPI_PLATFORM_CONTEXT *spc = (SPI_PLATFORM_CONTEXT *)platformData;
  SPI_REGISTERS *pRegisters = (SPI_REGISTERS *)(UINTN)spc->BaseAddress;
  UINT32 minTranSize;
  EFI_STATUS Status = EFI_SUCCESS; //Assume success
  DEBUG((EFI_D_INFO, "SpiTransfer, Stall 5 seconds\r\n"));
  gBS->Stall(5000000);

  minTranSize = pTargetSettings->DataFrameSize;
  pTargetSettings->BytesPerEntry = (UINT8)(pTargetSettings->DataFrameSize + 7) / 8;
  if (pTargetSettings->BytesPerEntry > 2) pTargetSettings->BytesPerEntry = 4;
  minTranSize =  pTargetSettings->BytesPerEntry * 8;
  //
  //disable controller
  //
  pRegisters->SSCR0 &= ~(UINT32)SSCR0_SSE_EN;
  ctrlr0Value =
  ((UINT32)(DivU64x32(spc->InputFrequencyHertz, pTargetSettings->ClockSpeed) & SSCR0_SCR_MASK) << SSCR0_SCR_SHIFT) |
       (UINT32)LShiftU64 ((RShiftU64((minTranSize - 1) , 4)) , SSCR0_EDDS_SHIFT) |
    (((minTranSize - 1) & SSCR0_DSS_MASK) << SSCR0_DSS_SHIFT);
  ctrlr1Value = SSCR1_TTE |
      (pTargetSettings->LoopbackMode ? SSCR1_LBM : 0) |
      (pTargetSettings->SerialClockPhase ? SSCR1_SPH : 0) |
      (pTargetSettings->SerialClockPolarity ? SSCR1_SPO : 0) |
      (pTargetSettings->ChipSelectPolarity ? SSCR1_IFS : 0);
  DEBUG((EFI_D_INFO, "ctrl0Value=0x%08x ctrl1Value=0x%08x\r\n", ctrlr0Value, ctrlr1Value));
  DEBUG((EFI_D_INFO, "Expected ctrl0Value=0x%08x Expected ctrl1Value=0x%08x\r\n", 0x00C69F87, 0x40000000));
//  pRegisters->SSCR0 = 0x00C69F87;//69F is the divider, 100,000,000/1000
//  pRegisters->SSCR0 = 0x00C06487;//064 is the divider, 100,000,000/100,1MHZ

  pRegisters->SSCR0 =   0x00C00A87;//0A is the divider, 100,000,000/10, 10MHZ
  switch(i) {
    case 0:
    case 2:
    case 4:
    case 6:
    case 8:
      i += 2;
      break;
    case 10:
      i = 20;
      break;
    case 20:
      i = 40;
      break;
    case 40:
      i = 50;
      break;
    case 50:
      i = 100;
      break;
    case 100:
      i = 200;
      break;
    case 200:
      i = 500;
      break;
    case 500:
      i = 1000;
      break;
  }
  DEBUG((EFI_D_INFO, "Dividier:%d speed:%d\r\n",i,DivU64x32(100000000, i)));
  pRegisters->SSCR0 = (i << SSCR0_SCR_SHIFT) | 0x00C00087;
  DEBUG((EFI_D_INFO, "Dividier:%d speed:%d 0x%08x\r\n",i,DivU64x32(100000000, i),pRegisters->SSCR0));
  pRegisters->SSCR1 = 0x40000000;

  pTargetSettings->ChipSelectPolarity = 0;
  pTargetSettings->BytesPerEntry = 1;

  //
  //enable controller
  //
  pRegisters->SSCR0 |= SSCR0_SSE_EN;

 // ControllerDisableInterrupts(pRegisters);
  if ( RequestPacket->WriteBytes ) {
    pRegisters->PRV_GENERAL |= PRV_GENERAL_TERMINATE_TX_ON_RX_FULL_DISABLE;
  } else if( RequestPacket->ReadBytes ) {
    pRegisters->PRV_GENERAL &= ~(UINT32)PRV_GENERAL_TERMINATE_TX_ON_RX_FULL_DISABLE;
  }

  ControllerAcknowledgeInterrupts(pRegisters, SPI_INTERRUPT_STATUS_MASK);
  //
  //disable DMA
  //
  pRegisters->SSCR1 &= ~(UINT32)(SSCR1_TSRE | SSCR1_RSRE | SSCR1_TRAIL);
  //
  // set transmit threshold to 1 to generate interrupt when TXFIFO is empty
  //
  pRegisters->SITF = ((1 & SITF_LWMTF_MASK) << SITF_LWMTF_SHIFT) | (1 & SITF_HWMTF_MASK);
  pRegisters->SIRF = ((1 & SIRF_WMRF_MASK) << SIRF_WMRF_SHIFT);


  if ( RequestPacket->WriteBytes ) {
    ControllerSpiAssertCs(pRegisters, pTargetSettings->ChipSelectPolarity);
    Status = SpiDoWrite(
      pRegisters,
      pTargetSettings,
      RequestPacket->WriteBuffer,
      RequestPacket->WriteBytes,
      &(RequestPacket->BytesWritten),
      RequestPacket->ReadBuffer,
      RequestPacket->ReadBytes,
      &(RequestPacket->BytesRead)
      );
    if ( NULL != SpiStatus ) {
      *SpiStatus = Status;
    };
    if(EFI_ERROR(Status)) {
      return EFI_DEVICE_ERROR;
    }
  } else {
    ControllerSpiAssertCs(pRegisters, pTargetSettings->ChipSelectPolarity);
    Status = SpiDoRead(
      pRegisters,
      pTargetSettings,
      RequestPacket->ReadBuffer,
      RequestPacket->ReadBytes,
      &(RequestPacket->BytesRead)
    );
  }
  return EFI_SUCCESS;
}

/**
  Execute Spi Write

  @param [in] pRegisters                A pointer to SPI_PLATFORM_CONTEXT instance
  @param [in] pTargetSettings           A pointer to SPI_TARGET_SETTINGS instance.
  @param [in] writeBuffer               Buffer contains data to write
  @param [in] writeBytes                Specifies size to write
  @param [in, out] pBytesWritten        Actual bytes written
  @param [in] readBuffer                Buffer to store read data
  @param [in] readBytes                 Specifies size to read
  @param [in, out] pBytesRead           Actual bytes read

  @retval EFI_STATUS                    Spi Write Successfully
  @retval Others                        Spi Write failure
**/
EFI_STATUS
SpiDoWrite(
  IN SPI_REGISTERS                       *pRegisters,
  IN SPI_TARGET_SETTINGS                 *pTargetSettings,
  IN VOID                                *writeBuffer,
  IN UINT32                              writeBytes,
  IN OUT UINT32                          *pBytesWritten,
  IN VOID                                *readBuffer,
  IN UINT32                              readBytes,
  IN OUT UINT32                          *pBytesRead
  )
{
    INT32 txFifoSpace = SPI_TX_FIFO_SIZE - ((pRegisters->SITF >> SITF_SITFL_SHIFT) & SITF_SITFL_MASK);
    UINT8 bytesPerEntry = pTargetSettings->BytesPerEntry;
//
//supported granularity, 8, 16, 32
//
#define padding_fifo(bytes) ((bytesPerEntry - (bytes & (bytesPerEntry - 1))) & (bytesPerEntry - 1))
    INT32 DummyBytes = (readBytes > (writeBytes + padding_fifo(writeBytes))) ? \
      (readBytes - (writeBytes + padding_fifo(writeBytes))) : 0;
    UINT32 fifoEntry, minBytes, i;
#undef padding_fifo
    DEBUG((EFI_D_INFO, "SpiDoWrite txFifoSpace:%02d bytesPerEntry:%02d SSSR: 0x%08x SITF: 0x%08x SIRF: 0x%08x\r\n",
      txFifoSpace,
      bytesPerEntry,
      pRegisters->SSSR,
      pRegisters->SITF,
      pRegisters->SIRF
      ));
    while((writeBytes > *pBytesWritten) && (txFifoSpace > 0)) {
      fifoEntry = 0;
      for(i = 0; i < bytesPerEntry; i ++) {
        fifoEntry |= (((UINT8 *)writeBuffer)[*pBytesWritten + i]) << (i * 8);
      }
      pRegisters->SSDR = fifoEntry;
      *pBytesWritten += bytesPerEntry;
      txFifoSpace --;
    }
    while(DummyBytes > 0) {
        pRegisters->SSDR = 0x00000000;
        DummyBytes -= bytesPerEntry;
    }
    DEBUG((EFI_D_INFO,"Filled the fifo, SSSR:0x%08x SITF:0x%08x SIRF:0x%08x\r\n", pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
    do {
      DEBUG((EFI_D_INFO,"loop SSSR:0x%08x SITF:0x%08x SIRF:0x%08x\r\n", pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
    } while((pRegisters->SITF >> SITF_SITFL_SHIFT) & SITF_SITFL_MASK);
    DEBUG((EFI_D_INFO,"SSSR:0x%08x SITF:0x%08x SIRF:0x%08x\r\n", pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
    DEBUG((EFI_D_INFO,"All bytes have gone\r\n"));
    if(readBytes) {
      while (readBytes > *pBytesRead) {
      DEBUG((EFI_D_INFO, "loop read SSSR:0x%08x SITF:0x%08x SIRF:0x%08x\r\n", pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
      DEBUG((EFI_D_INFO, "%d %d\r\n", readBytes, *pBytesRead));
      if( SPI_RX_ENTRIES ) {
        fifoEntry = pRegisters->SSDR;
        DEBUG((EFI_D_INFO, "read 0x%08x\r\n",fifoEntry));
        minBytes = (bytesPerEntry < (readBytes - *pBytesRead)) ? bytesPerEntry : (readBytes - *pBytesRead);
        for (i = 0; i < minBytes; i++) {
          ((UINT8*)readBuffer)[*pBytesRead] = fifoEntry & 0xFF;
          *pBytesRead += 1;
          fifoEntry >>= 8;
        }
      }
    }
  }
  return EFI_SUCCESS;
}

/**
  Execute Spi Read

  @param [in] pRegisters                A pointer to SPI_PLATFORM_CONTEXT instance
  @param [in] pTargetSettings           A pointer to SPI_TARGET_SETTINGS instance.
  @param [in] readBuffer                Buffer to store read data
  @param [in] readBytes                 Specifies size to read
  @param [in, out] pBytesRead           Actual bytes read

  @retval EFI_STATUS                    Spi Read Successfully
  @retval Others                        Spi Read failure
**/
EFI_STATUS
SpiDoRead (
  IN SPI_REGISTERS                      *pRegisters,
  IN SPI_TARGET_SETTINGS                *pTargetSettings,
  IN VOID                               *readBuffer,
  IN UINT32                             readBytes,
  IN OUT UINT32                         *pBytesRead
  )
{
  UINT8  bytesPerEntry = pTargetSettings->BytesPerEntry;
  UINT32  fifoEntry ;
  UINT32  i = 0;
  UINT32 minBytes;
  UINT32  rxEntries;
  INT32 NullBytes = (INT32)readBytes;
  rxEntries = (pRegisters->SIRF >> SIRF_SIRFL_SHIFT) & SIRF_SIRFL_MASK;
  DEBUG((EFI_D_INFO, "SpiDoRead: rxEntries:%02d bytesPerEntry:%02d SSSR: 0x%08x SITF:0x%08x SIRF:0x%08x\r\n",
        rxEntries, bytesPerEntry, pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
  while (NullBytes > 0 ) {
      pRegisters->SSDR = 0x00;
      NullBytes -= bytesPerEntry;
  }
  DEBUG((EFI_D_INFO, "Filled the fifo, SSSR:0x%08x SITF:0x%08x SIRF:0x%08x\r\n", pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
  while ( readBytes > *pBytesRead ) {
    DEBUG((EFI_D_INFO,"loop SSSR:0x%08x SITF:0x%08x SIRF:0x%08x\r\n", pRegisters->SSSR, pRegisters->SITF, pRegisters->SIRF));
    DEBUG((EFI_D_INFO,"%d %d\r\n", readBytes, *pBytesRead));
    if( SPI_RX_ENTRIES ) {
      fifoEntry = pRegisters->SSDR;
      minBytes = (bytesPerEntry < (readBytes - *pBytesRead)) ? bytesPerEntry : (readBytes - *pBytesRead);
      for (i = 0; i < minBytes; i++) {
        ((UINT8*)readBuffer)[*pBytesRead] = fifoEntry & 0xFF;
        *pBytesRead += 1;
        fifoEntry >>= 8;
      }
    }
  }
  return EFI_SUCCESS;
}



