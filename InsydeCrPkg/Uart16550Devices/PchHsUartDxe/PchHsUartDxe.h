/** @file
  Header file for High Speed Uart driver.
;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _HS_UART_H_
#define _HS_UART_H_

#include <Uefi.h>

#include <Protocol/Uart16550Access.h>
#include <Protocol/DevicePath.h>

#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/IoLib.h>

#include <IndustryStandard/Pci22.h>

#pragma pack(1)

typedef struct {
  VENDOR_DEVICE_PATH      VendorHw;
  UINT32                  BaseAddress;
  UINT32                  UID;
} HS_UART_DEVICE_PATH_NODE;

typedef struct {
  HS_UART_DEVICE_PATH_NODE  HsUartNode;
  EFI_DEVICE_PATH           End;
} HS_UART_DEVICE_PATH;

typedef struct _MMIO_ADDR {
  UINT8    Enable;
  UINT32   BaseAddr;
  UINT8    Irq;
} MMIO_ADDR;

#pragma pack()

//
// data type definitions
//
typedef struct {
  UINT32                            Signature;
  EFI_HANDLE                        Handle;
  UART_16550_DEVICE_INFO            DeviceInfo;
  HS_UART_DEVICE_PATH               DevicePath;
  H2O_UART_16550_ACCESS_PROTOCOL    U16550Access;
} PCH_HS_UART_DEVICE_DATA;

typedef enum {
  Access8bit  = 1,
  Access32bit = 4
} ACCESS_WIDTH;

EFI_STATUS
HsUartRegRead (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             *Data
  );

EFI_STATUS
HsUartRegWrite (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             Data
  );


#endif
