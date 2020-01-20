/** @file
  High Speed Uart 16550 Compatible hardware driver

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

#include "PchHsUartDxe.h"

#define PCH_HS_UART_DEVICE_NAME_SIZE            30
#define PCH_HS_UART_DEVICE_NAME                 L"PCH_HS_UART"
#define PCH_HS_UART_FIFO_DEPTH                  16

#define HS_UART_DEVICE_DATA_SIGNATURE           SIGNATURE_32 ('H','S','U','R')
#define HS_UART_DEVICE_DATA_FROM_THIS(This)     CR (This, PCH_HS_UART_DEVICE_DATA, U16550Access, HS_UART_DEVICE_DATA_SIGNATURE)

MMIO_ADDR                 *gPchBaseAddr;

PCH_HS_UART_DEVICE_DATA   mHsUartTemplate[] = {
    HS_UART_DEVICE_DATA_SIGNATURE,
    NULL,    // Handle
    { // DeviceInfo
      0
    },
    {        // DevicePath
       {  // HS_Uart_device_path Node
          HARDWARE_DEVICE_PATH,
          HW_VENDOR_DP,
          sizeof (HS_UART_DEVICE_PATH_NODE),
          0,
          UART_16550_COMPATIBLE_HW_GUID,      //GUID
          0,                                  //BaseAddress
          0                                   //UID
        },
        { //End_of_Device_path_Node
          END_DEVICE_PATH_TYPE,
          END_ENTIRE_DEVICE_PATH_SUBTYPE,
          sizeof (EFI_DEVICE_PATH),
          0
        }
    },
    { // Uart16550AccessProtocol
      HsUartRegRead,
      HsUartRegWrite,
      0
    }
};


/**
  Returns UART's currently active access mode, 8 or 32 bit

  @param[in]  UartNumber     Selects Serial IO UART device (0-2)
  @retval     AccessMode8bit
  @retval     AccessMode32bit
**/
STATIC
UINT8
InternalDetectAccessMode (
  IN UINT16 UartNumber
  )
{
  UINT32 ByteMode;

  ByteMode = MmioRead32 (FixedPcdGet32(PcdH2OSerialIoPchPcrAddress));

  if (ByteMode <= 1) {
    //
    // Some platform didn't support PchPcrAddress.
    // Set 1 = Access8bit
    // Set 0 = Access32bit
    //
    if (ByteMode == 1) {
      return Access8bit;
    } else {
      return Access32bit;
    }

  } else {

    if ((ByteMode>>UartNumber) & 0x1) {
      return Access8bit;
    } else {
      return Access32bit;
    }
  }

}

EFI_STATUS
InitHsUartDeviceData (
  IN PCH_HS_UART_DEVICE_DATA  *PchHsUartDev
  )
{
  UINT16                    UID;
  CHAR16                    *Name;

  UID = PchHsUartDev->DeviceInfo.UID;

  //
  // Set Device Name
  //
  Name = AllocateZeroPool (PCH_HS_UART_DEVICE_NAME_SIZE);

  UnicodeSPrint (
    Name,
    PCH_HS_UART_DEVICE_NAME_SIZE,
    L"%s %d",
    PCH_HS_UART_DEVICE_NAME,
    UID
    );

  //
  // Initialize HS_UART_DEVICE_DATA structure
  //
  PchHsUartDev->DeviceInfo.DeviceName        = Name;
  PchHsUartDev->DeviceInfo.DeviceType        = PCH_HS_SERIAL_DEVICE;
  PchHsUartDev->DeviceInfo.RegisterByteWidth = InternalDetectAccessMode (UID);
  PchHsUartDev->DeviceInfo.BaseAddressType   = UBAT_MEMORY;
  PchHsUartDev->DeviceInfo.BaseAddress       = (UINTN)(gPchBaseAddr[UID].BaseAddr) ;
  PchHsUartDev->DeviceInfo.DevIRQ            = gPchBaseAddr[UID].Irq;
  PchHsUartDev->DeviceInfo.FifoSize          = PCH_HS_UART_FIFO_DEPTH;
  PchHsUartDev->DeviceInfo.SerialClockFreq   = (UINTN)FixedPcdGet32(PcdH2OHsUartSerialClock);
  PchHsUartDev->DeviceInfo.SampleRate        = FixedPcdGet16(PcdH2OHsUartSampleRate);
  PchHsUartDev->DeviceInfo.LegacySupport     = TRUE;

  PchHsUartDev->DevicePath.HsUartNode.BaseAddress = (UINTN)gPchBaseAddr[UID].BaseAddr;
  PchHsUartDev->DevicePath.HsUartNode.UID         = UID;
  PchHsUartDev->U16550Access.DeviceInfo           = &(PchHsUartDev->DeviceInfo);

  return EFI_SUCCESS;
}

/**
 Read data from HsUart register.

 @param    This            The H2O_UART_16550_ACCESS_PROTOCOL.
 @param    Index           Reg offset.
 @param    Data            Write data.

 @retval EFI_SUCCESS            CrPolicy protocol installed
 @return Other                       No protocol installed.

**/
EFI_STATUS
HsUartRegRead (
  H2O_UART_16550_ACCESS_PROTOCOL    *This,
  UINT16                            Index,
  UINT8                             *Data
  )
{
  UINTN                     BaseAddress;
  UINT8                     RegisterWidth;

  BaseAddress   = This->DeviceInfo->BaseAddress;
  RegisterWidth = This->DeviceInfo->RegisterByteWidth;

  if (RegisterWidth == Access32bit) {
    *Data = (UINT8) (0xFF & MmioRead32 (BaseAddress + 4 * Index));
  } else {
    *Data = MmioRead8 (BaseAddress + Index);
  }

  return EFI_SUCCESS;
}

/**
 Write data to HsUart register.

 @param    This            The H2O_UART_16550_ACCESS_PROTOCOL.
 @param    Index           Reg offset.
 @param    Data            Write data.

 @retval EFI_SUCCESS            CrPolicy protocol installed
 @return Other                       No protocol installed.

**/
EFI_STATUS
HsUartRegWrite (
  IN H2O_UART_16550_ACCESS_PROTOCOL    *This,
  IN UINT16                            Index,
  IN UINT8                             Data
  )
{
  UINTN                     BaseAddress;
  UINT8                     RegisterWidth;

  BaseAddress   = This->DeviceInfo->BaseAddress;
  RegisterWidth = This->DeviceInfo->RegisterByteWidth;

  if (RegisterWidth == Access32bit) {
    MmioWrite32 (BaseAddress + 4 * Index, Data);
  } else {
    MmioWrite8 (BaseAddress + Index, Data);
  }

  return EFI_SUCCESS;
}

BOOLEAN
IsHsUartEnable (
  IN UINT8    Index
  )
{
  return gPchBaseAddr[Index].Enable == 0x01 ? TRUE : FALSE;
}

/**
 Install Driver to produce UART_16550_ACCESS protocol.

 @param    ImageHandle
 @param    SystemTable

 @retval EFI_SUCCESS            HsUart installed
 @return Other                  Install HsUart fail.

**/
EFI_STATUS
EFIAPI
HsUartEntryPoint (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                Status;
  PCH_HS_UART_DEVICE_DATA   *PchHsUartDev;
  UINT8                     Index;

  Index  = 0;
  Status = EFI_UNSUPPORTED;

  gPchBaseAddr = FixedPcdGetPtr(PcdH2OPchHsUartMmioAddress);

  while (gPchBaseAddr[Index].Enable != 0xFF) {
    if (!IsHsUartEnable(Index)) {
      Index++;
      continue;
    }

    PchHsUartDev = AllocateCopyPool (sizeof(PCH_HS_UART_DEVICE_DATA), &mHsUartTemplate);
    if (PchHsUartDev == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    PchHsUartDev->DeviceInfo.UID = Index;

    //
    // Initialize the device Private data
    //
    InitHsUartDeviceData (PchHsUartDev);

    //
    // Install Uart16550Access protocol on the contorller handle
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &PchHsUartDev->Handle,
                    &gEfiDevicePathProtocolGuid,
                    &PchHsUartDev->DevicePath,
                    &gH2OUart16550AccessProtocolGuid,
                    &PchHsUartDev->U16550Access,
                    NULL
                    );
    if (EFI_ERROR(Status)) {
      return Status;
    }
    Index ++;
  }
  return Status;
}

