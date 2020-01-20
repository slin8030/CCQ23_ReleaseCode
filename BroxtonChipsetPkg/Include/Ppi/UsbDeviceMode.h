/** @file
  The runtime cryptographic protocol.
  Only limited crypto primitives (SHA-256 and RSA) are provided for runtime
  authenticated variable service.

Copyright (c) 2010 - 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EFI_USBDEVMODE_H__
#define __EFI_USBDEVMODE_H__

#include <Library/UsbDeviceModeLib.h>

///
/// Runtime UsbDeviceMode Ppi GUID.
///
#define EFI_PEI_FASTBOOT_PPI_GUID \
  { 0xd80fa2a0, 0x6a36, 0x4fc6, { 0xa0, 0xab, 0x56, 0x68, 0xf3, 0x5c, 0xc9, 0x3a }};

#define EFI_USBDEVICEMODE_PPI_GUID \
  { 0xfd5c7e4a, 0xf7b6, 0x4811, { 0x94, 0x96, 0xc0, 0x25, 0xbf, 0xee, 0x7b, 0x04 }};

typedef struct _EFI_PEI_USBDEVICEMODE_PPI EFI_PEI_USBDEVICEMODE_PPI;


typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_SET_MMIO) (
  IN UINT32 MmioBar
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_INIT_DCI) (
  VOID
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_DE_INIT_DCI) (
  VOID    *pUdciHndl,
  UINT32  flags
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_ISR) (
  VOID    *pUdciHndl
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_CONNECT) (
  VOID    *pUdciHndl
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_DIS_CONNECT) (
  VOID    *pUdciHndl
  );


typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_SET_ADD) (
  VOID    *pUdciHndl,
  UINT8   Address
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_INIT_EP) (
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  );

typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_ENABLE_EP) (
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  );
 
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_DISABLE_EP) (
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  );  
  
  
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_STALL_EP) (
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  );    
  
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_CLR_STALL_EP) (
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  );     
  
  
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_EP0_TX_STATUS) (
  VOID            *pUdciHndl
  );     
    
    
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_EP_TX_DATA) (
  VOID        *pUdciHndl,
  USBD_IO_REQ *pIoReq
  );         
    
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_EP_RX_DATA) (
  VOID        *pUdciHndl,
  USBD_IO_REQ *pIoReq
  );         
        
        
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_REG_CALLBACK) (
  VOID        *pUdciHndl
  );            
        
/**

**/
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_FB_INIT) (
  IN  VOID  *FbParams  
  );


/**

**/
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_FB_DEINIT) (
  VOID
  );


/**

**/
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_FB_START) (
  VOID
  );


/**

**/
typedef
BOOLEAN
(EFIAPI *EFI_USB_DEVICE_MODE_FB_START) (
  VOID
  );

/**

**/
typedef
VOID
(EFIAPI *EFI_USB_DEVICE_MODE_FB_STOP) (
  VOID
  );


struct _EFI_PEI_USBDEVICEMODE_PPI {
  EFI_USB_DEVICE_MODE_SET_MMIO             UsbdSetMmio;
  EFI_USB_DEVICE_MODE_INIT_DCI             UsbdInitDCI;
  EFI_USB_DEVICE_MODE_DE_INIT_DCI          UsbdDeInitDCI;
  EFI_USB_DEVICE_MODE_ISR                  UsbdISR;
  EFI_USB_DEVICE_MODE_CONNECT              UsbdConnect;
  EFI_USB_DEVICE_MODE_DIS_CONNECT          UsbdDisConnect;
  EFI_USB_DEVICE_MODE_SET_ADD              UsbdSetAddress;
  EFI_USB_DEVICE_MODE_INIT_EP              UsbdInitEP;
  EFI_USB_DEVICE_MODE_ENABLE_EP            UsbdEnableEP;
  EFI_USB_DEVICE_MODE_DISABLE_EP           UsbdDisableEP;
  EFI_USB_DEVICE_MODE_STALL_EP             UsbdStallEP;
  EFI_USB_DEVICE_MODE_CLR_STALL_EP         UsbdClearStallEP;
  EFI_USB_DEVICE_EP0_TX_STATUS             UsbdEp0TxStatus;
  EFI_USB_DEVICE_EP_TX_DATA                UsbdEpTxData;
  EFI_USB_DEVICE_EP_RX_DATA                UsbdEpRxData;
  EFI_USB_DEVICE_REG_CALLBACK              UsbdRegCallBack;
  EFI_USB_DEVICE_MODE_FB_INIT              FBInit;
  EFI_USB_DEVICE_MODE_FB_DEINIT            FBDeinit;
  EFI_USB_DEVICE_MODE_FB_START             FBStart;
  EFI_USB_DEVICE_MODE_FB_STOP              FBStop;
};


typedef struct _EFI_PEI_FAST_BOOT_PPI EFI_PEI_FAST_BOOT_PPI;

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_FAST_BOOT_ENTRY) (
  IN EFI_PEI_SERVICES             **PeiServices
  );

struct _EFI_PEI_FAST_BOOT_PPI {
  EFI_PEI_FAST_BOOT_ENTRY             FastBootEntry;
};
extern EFI_GUID gPeiFastbootPpiGuid;
extern EFI_GUID gPeiUsbDeviceModePpiGuid;

#endif
