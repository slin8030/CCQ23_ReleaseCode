/** @file

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

#include "UsbDeviceModePei.h"

//
// The handle onto which the Runtime UsbDeviceMode Protocol instance is installed
//
EFI_HANDLE  mUsbDeviceModetHandle = NULL;



/**
  Retrieves the size, in bytes, of the context buffer required for SHA-256 operations.

  @return  The size, in bytes, of the context buffer required for SHA-256 operations.

**/
BOOLEAN
EFIAPI
UsbDeviceSetMmioBar(
  IN UINT32 MmioBar
  )
{
     usbdSetMmioBar (MmioBar);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceInitDciDxe (
  VOID
  )
{
		
   usbdInitDCI ();
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceDeInitDciDxe (
  VOID    *pUdciHndl,
  UINT32  flags
  )
{
		
   udciDeinit (pUdciHndl, flags);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceISR (
  VOID    *pUdciHndl
  )
{
		
   udciIsr (pUdciHndl);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceConnect(
  VOID    *pUdciHndl
  )
{
   udciConnect (pUdciHndl);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceDisConnect(
  VOID    *pUdciHndl
  )
{
   udciDisconnect (pUdciHndl);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceSetAddress(
  VOID    *pUdciHndl,
  UINT8   Address
  )
{
   udciSetAddress (pUdciHndl, Address);
   return TRUE;
}


BOOLEAN
EFIAPI
UsbDeviceInitEP(
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  )
{
   udciInitEp (pUdciHndl, pEpInfo);
   return TRUE;
}


BOOLEAN
EFIAPI
UsbDeviceEnableEP(
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  )
{
   udciEnableEp (pUdciHndl, pEpInfo);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceDisableEP(
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  )
{
   udciDisableEp (pUdciHndl, pEpInfo);
   return TRUE;
}


BOOLEAN
EFIAPI
UsbDeviceStallEP(
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  )
{
   udciStallEp (pUdciHndl, pEpInfo);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceClearStallEP(
  VOID            *pUdciHndl,
  USB_DEV_EP_INFO *pEpInfo
  )
{
   udciClearStallEp (pUdciHndl, pEpInfo);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceEp0TxStatus(
  VOID            *pUdciHndl
  )
{
   udciEp0TxStatus (pUdciHndl);
   return TRUE;
}


BOOLEAN
EFIAPI
UsbDeviceEpTxData(
  VOID        *pUdciHndl,
  USBD_IO_REQ *pIoReq
  )
{
   udciEpTxData (pUdciHndl, pIoReq);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceEpRxData(
  VOID        *pUdciHndl,
  USBD_IO_REQ *pIoReq
  )
{
   udciEpTxData (pUdciHndl, pIoReq);
   return TRUE;
}

BOOLEAN
EFIAPI
UsbDeviceRegCallBack(
  VOID            *pUdciHndl
  )
{
   udciRegisterCallbacks (pUdciHndl);
   return TRUE;
}


/**

**/
BOOLEAN
EFIAPI
FastBootInitDxe (
  VOID   *FbParams
  )
{
  return fbInit(FbParams);
}

/**

**/

BOOLEAN
EFIAPI
FastBootDeInitDxe (
  VOID
)
{
  return fbDeinit();
}

/**

**/
BOOLEAN
EFIAPI
FastBootStartDxe (
  VOID
)
{
  return fbStart ();
}

/**

**/
BOOLEAN
EFIAPI
FastBootStopDxe (
  VOID
  )
{
  return fbStop();
}

//
// The UsbDeviceMode Ppi instance produced by this driver
//
static EFI_PEI_USBDEVICEMODE_PPI  mUsbDeviceModePpi = {
  UsbDeviceSetMmioBar,
  UsbDeviceInitDciDxe,
  UsbDeviceDeInitDciDxe,
  UsbDeviceISR,
  UsbDeviceConnect,
  UsbDeviceDisConnect,
  UsbDeviceSetAddress,
  UsbDeviceInitEP,
  UsbDeviceEnableEP,
  UsbDeviceDisableEP,
  UsbDeviceStallEP,
  UsbDeviceClearStallEP,
  UsbDeviceEp0TxStatus,
  UsbDeviceEpTxData,
  UsbDeviceEpRxData,
  UsbDeviceRegCallBack,
  FastBootInitDxe,
  FastBootDeInitDxe,
  FastBootStartDxe,
  FastBootStopDxe,
  //Below for testing
};

static EFI_PEI_PPI_DESCRIPTOR mInstallUsbDeviceModePpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gPeiUsbDeviceModePpiGuid,
  &mUsbDeviceModePpi
};

/**
  Entry Point for UsbDeviceMode Driver.

  This function installs UsbDeviceMode Ppi.

  @param ImageHandle     Image handle of this driver.
  @param SystemTable     a Pointer to the EFI System Table.

  @retval  EFI_SUCEESS  Runtime Crypt Protocol is successfully installed
  @return  Others       Some error occurs when installing Runtime Crypt Protocol.

**/
EFI_STATUS
EFIAPI
UsbDeviceModeDriverInitialize (
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "\n UsbDeviceModeDriverInitialize start\n"));

  //
  // Install the Runtime Crypt Protocol onto a new handle
  //
  Status = (*PeiServices)->InstallPpi (PeiServices, &mInstallUsbDeviceModePpi);
  ASSERT_EFI_ERROR (Status);

 DEBUG ((DEBUG_INFO, "\n mUsbDeviceModeProtocol INstall : %r start\n", Status));

  return Status;
}
