/** @file
  SetPcuUart

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

#include <ScAccess.h>
#include <PlatformBaseAddresses.h>
#include <Library/IoLib.h>

#define PCIEX_BASE_ADDRESS                        0xE0000000
#define PciD31F0RegBase                           PCIEX_BASE_ADDRESS + (UINT32) (31 << 15)
#define SB_RCBA                                   0xfed1c000

#ifdef PO_FLAG
#if TABLET_PF_ENABLE
#include <ChvA0PowerOn.h>
#endif
#endif

/**

 Enable PCU UART

**/
EFI_STATUS
EnableInternalUart(
  VOID
  )
{
  //
  // Enable internal UART interrupt.
  //
  MmioOr32 (ILB_BASE_ADDRESS + R_ILB_IRQE, (UINT32) (B_ILB_IRQE_UARTIRQEN_IRQ4));
  //
  // GPIO: N67(HV_DDI2_DDC_SCL) - Setting Mode 3
  // UART0_TXD
  //
  MmioWrite32 ((UINTN) (IO_BASE_ADDRESS + 0xD438), (UINT32) 0x00930300);
  //
  // GPIO: N62(HV_DDI2_DDC_SDA) - Setting Mode 3
  // UART0_RXD
  //
  MmioWrite32 ((UINTN) (IO_BASE_ADDRESS + 0xD410), (UINT32) 0x00930300);

  MmioOr32 (
    (UINTN) (PciD31F0RegBase + R_LPC_UART_CTRL),
    (UINT32) (B_LPC_UART_CTRL_COM1_EN)
    );

  return  EFI_SUCCESS;
}
