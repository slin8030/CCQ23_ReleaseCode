/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright (c) 1999-2013 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code (Material) are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by Intel in writing.
 *
 ******************************************************************************/

 /** \file usb_fb.h
 *
 * USB Fastboot driver
 *
 */
#define _CRT_SECURE_NO_WARNINGS

//#include <stdint.h>
//#include <stdbool.h>
#include <stdio.h>
#include <string.h>
//#include <UdmSupport.h>
#include <stdlib.h>
#include <Manifest.h>
//#include <EfiFastBoot.h>
#include "usb.h"
#include "usbd.h"
#include "usb_fb.h"
//#include "delay.h"
#include "UdmWrapperPeiLib.h"
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>

#include <MediaDeviceDriverPei.h>
#include <Ppi/RecoveryModule.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Ppi/DeviceRecoveryModule.h>
//#include <Ppi/Sha256Hash.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesTablePointerLib.h>

//#include <Guid/FastbootHob.h>
#ifndef _MSC_VER
#define memcpy_s(p1,s1,p2,s2) memcpy (p1,p2,s1)
#endif

#define wcslen(s) StrLen(s)
#define strlen(s) AsciiStrLen(s)

EFI_STATUS
EFIAPI
FastbootStartImage (
  IN UINT8  *Buffer
  );

/*
 * Global Constants
 */
#define CONFIG_COUNT                (1)
#define INTERFACE_COUNT             (1)
#define ENDPOINT_COUNT              (2)

#define CFG_MAX_POWER               (0x00) /* Max power consumption of the USB device from the bus for this config */

#define IN_ENDPOINT_NUM             (1)
#define OUT_ENDPOINT_NUM            (2)

#define VENDOR_ID                   (0x8087)  /* Intel Inc. */

#define FB_IF_SUBCLASS              (0x42) /* Fastboot subclass */
#define FB_IF_PROTOCOL              (0x03) /* Fastboot protocol */

#define FB_BULK_EP_PKT_SIZE         USB_BULK_EP_PKT_SIZE_HS /* default to using high speed */
#define FB_RX_BUFF_MAX_SIZE         (0x5000) /* size of buffer for receive transactions */
#define FB_TX_BUFF_MAX_SIZE         (0x80)   /* size of buffer for transmit transactions */

#define FB_DOWNLOAD_MAX             (0x10000000) /* maximum allowed download size from a host */

/*
 * Note: The host fastboot application adds an image header to images
 * transfered to the device of size 2K.
 */
#define FB_MK_IMAGE_HDR_SIZE        (2048)

#define FB_FLASH_STAGE2            "flash:fw_stage2"   /*fastboot command to program & loads 2nd stage to Emmc */
#define FB_DOWNLOAD                "download:"         /*fastboot command to download the image */

/* Addresses/Offsets for loading and starting a downloaded kernel */
//static void *gpKernelLoadAddr = 0;
static char *gpKernelLoadAddr = 0;

static uint32_t gKernelEntryOffset;

/* USB connect timeout */
static uint32_t gTimeoutMs = 0;

/* global variables to dynamically allocated data buffers */
static void *gpRxBuf = NULL;
static void *gpTxBuf = NULL;

/* global vars for command operations */
static uint32_t gDownloadSize = 0;
static uint32_t gDownloadBytesLeft = 0;
static uint32_t Stage2DownloadSize = 0;

/* Complete Configuration structure for Fastboot */
#pragma pack(1)
typedef struct {
    configurationDescriptor     config;
    interfaceDescriptor         interface;
    endpointDescriptor          endpointIn;
    endpointCompanionDescriptor endpointCompIn;
    endpointDescriptor          endpointOut;
    endpointCompanionDescriptor endpointCompOut;
} fbConfigDescriptor;
#pragma pack()

/* String descriptor table indexes */
typedef enum {
    STR_TBL_LANG            = 0,
    STR_TBL_MANUFACTURER    = 1,
    STR_TBL_PRODUCT         = 2,
    STR_TBL_SERIAL_NUMBER   = 3,
    STR_TBL_CONFIG          = 4,
    STR_TBL_INTERFACE       = 5,
    STR_TBL_COUNT           = 6
} strTblIndex;


/* static USBD driver objects */
static usbdDevObj gDevObj;
static usbdConfigObj gConfigObjs[CONFIG_COUNT];
static usbdInterfaceObj gInterfaceObjs[INTERFACE_COUNT];
static usbdEndpointObj gEndpointObjs[ENDPOINT_COUNT];

/* Flag to specify if this driver is initialized */
static bool gIsInitialized = false;

/*
 * String descriptors values
 * Must be defined here as NULL terminated
 */
static wchar_t gEmptyString[] = {L'\0'};
static wchar_t gStrManufacturer[] = {L'I', L'n', L't', L'e', L'l', L'(', L'R', L')', L'C', L'o', L'r', L'p', L'o', L'r', L'a', L't', L'i', L'o', L'n', L'\0'};
static wchar_t gStrConfiguration[] = {L'U', L'S', L'B', L'-', L'U', L'p', L'd', L'a', L't', L'e',L'\0'};
static wchar_t gStrInterface[] = {L'F', L'a', L's', L't', L'b', L'o', L'o', L't', L'\0'};
static wchar_t *gpStrProduct = gEmptyString; /* default to empty, fill in dynamically */
static wchar_t *gpStrSerialNumber = gEmptyString; /* default to empty, fill in dynamically */

/* String descriptor Table */
static stringDescriptor gStrTable[STR_TBL_COUNT];

/* Default device descriptor for this Fastboot device */
static deviceDescriptor gDevDesc = {
    sizeof(deviceDescriptor),
    USB_DEVICE,
    USB_BCD_VERSION_HS, /* Default to High Speed */
    0x00, /* specified in interface descriptor */
    0x00, /* specified in interface descriptor */
    0x00, /* specified in interface descriptor */
    USB_EP0_MAX_PKT_SIZE_HS, /* Default to high speed */
    VENDOR_ID,
    0x0, /* filled in dynamically */
    0x0, /* filled in dynamically */
    STR_TBL_MANUFACTURER,
    STR_TBL_PRODUCT,
    STR_TBL_SERIAL_NUMBER,
    CONFIG_COUNT

};


/*
 * Full Configuration Descriptor and attached
 * Interfaces/Endpoints for this FB device, filled in with default data
 */
static fbConfigDescriptor gConfigDesc = {
    //config 
    {
       sizeof(configurationDescriptor),
       USB_CONFIGURATION,
       sizeof(fbConfigDescriptor),
       INTERFACE_COUNT,
       1,
       STR_TBL_CONFIG,
       USB_BM_ATTR_RESERVED | USB_BM_ATTR_SELF_POWERED,
       CFG_MAX_POWER
    },
    //interface 
    {
       sizeof(interfaceDescriptor),
       USB_INTERFACE,
       0x0,
       0x0,
       ENDPOINT_COUNT,
       USB_DEVICE_VENDOR_CLASS,
       FB_IF_SUBCLASS,
       FB_IF_PROTOCOL,
       STR_TBL_INTERFACE
    },
    //endpointIn 
    {
       sizeof(endpointDescriptor),
       USB_ENDPOINT,
       IN_ENDPOINT_NUM | USB_DIR_IN,
       USB_EP_XFER_BULK,
       FB_BULK_EP_PKT_SIZE,
       0x00 /* Not specified for bulk endpoint */
    },
    //endpointCompIn  
    {
       sizeof(endpointCompanionDescriptor),
       USB_SS_ENDPOINT_COMPANION,
       0x00, /* No bursting supported */
       0x00, /* No streams */
       0x0000, /* Only used for Periodic Endpoints */
    },
    //endpointOut 
    {
       sizeof(endpointDescriptor),
       USB_ENDPOINT,
       OUT_ENDPOINT_NUM | USB_DIR_OUT,
       USB_EP_XFER_BULK,
       FB_BULK_EP_PKT_SIZE,
       0x00 /* Not specified for bulk endpoint */
    },
    //endpointCompOut  
    {
       sizeof(endpointCompanionDescriptor),
       USB_SS_ENDPOINT_COMPANION,
       0x00, /* No bursting supported */
       0x00, /* No streams */
       0x0000, /* Only used for Periodic Endpoints */
    }
};

EFI_STATUS
EFIAPI
PlatformRecoveryModule (
  IN CONST EFI_PEI_SERVICES             **PeiServices,
  IN EFI_PEI_RECOVERY_MODULE_PPI        *This
  );

static EFI_PEI_RECOVERY_MODULE_PPI mRecoveryPpi = {
  PlatformRecoveryModule
};

static EFI_PEI_PPI_DESCRIPTOR mRecoveryPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiRecoveryModulePpiGuid,
  &mRecoveryPpi
};

EFI_STATUS
EFIAPI
PlatformRecoveryModule (
  IN CONST EFI_PEI_SERVICES               **PeiServices,
  IN EFI_PEI_RECOVERY_MODULE_PPI          *This
  )
{
//  PeiServicesInstallFvInfoPpi (
//    NULL,
//    (VOID *) ((UINTN) gpKernelLoadAddr + PcdGet32 (PcdFlashFvMainBase) - PcdGet32 (PcdFlashFvShellBase)),
//    PcdGet32 (PcdFlashFvMainSize),
//    NULL,
//    NULL
//    );
//
//  BuildFvHob (
//    (UINTN) gpKernelLoadAddr + PcdGet32 (PcdFlashFvMainBase) - PcdGet32 (PcdFlashFvShellBase),
//    PcdGet32 (PcdFlashFvMainSize)
//    );
//
//  PeiServicesInstallFvInfoPpi (
//    NULL,
//    (VOID *) (UINTN) gpKernelLoadAddr,
//    PcdGet32 (PcdFlashFvShellSize),
//    NULL,
//    NULL
//    );
//
//  BuildFvHob (
//    (UINTN) gpKernelLoadAddr,
//    PcdGet32 (PcdFlashFvShellSize)
//    );

  return EFI_SUCCESS;
}


/**
 * Initializes string descriptor table with all strings required
 * by this Fastboot device.
 *
 * @return void
 */
static void fbInitStringDescriptors() {

    gStrTable[STR_TBL_LANG].bDescriptorType = USB_STRING;
    gStrTable[STR_TBL_LANG].wLangID[0] = 0x0409; /* English defined as 0x0409 */
    gStrTable[STR_TBL_LANG].bLength = 4;

    gStrTable[STR_TBL_MANUFACTURER].bDescriptorType = USB_STRING;
//    wstrcpy_s((wchar_t *)(gStrTable[STR_TBL_MANUFACTURER].wLangID), STRING_ARR_SIZE, (const wchar_t *)gStrManufacturer);
//    strcpy((wchar_t *)(gStrTable[STR_TBL_MANUFACTURER].wLangID), (const wchar_t *)gStrManufacturer);
//    strcpy_s((wchar_t *)(gStrTable[STR_TBL_MANUFACTURER].wLangID), STRING_ARR_SIZE, (const wchar_t *)gStrManufacturer);
//    strcpy((wchar_t *)(gStrTable[STR_TBL_MANUFACTURER].wLangID), (const wchar_t *)gStrManufacturer);
        //memcpy_s(gStrTable[STR_TBL_MANUFACTURER].wLangID, STRING_ARR_SIZE, gStrManufacturer, STRING_ARR_SIZE);
        CopyMem(gStrTable[STR_TBL_MANUFACTURER].wLangID,gStrManufacturer, STRING_ARR_SIZE);
        DEBUG ((DEBUG_INFO, "STR_TBL_MANUFACTURER=%s, Length=%x\n", gStrTable[STR_TBL_MANUFACTURER].wLangID, STRING_ARR_SIZE));


    gStrTable[STR_TBL_MANUFACTURER].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_MANUFACTURER].wLangID)));

    gStrTable[STR_TBL_PRODUCT].bDescriptorType = USB_STRING;
//    strcpy_s((wchar_t *)(gStrTable[STR_TBL_PRODUCT].wLangID), STRING_ARR_SIZE, (const wchar_t *)gpStrProduct);
//    strcpy((wchar_t *)(gStrTable[STR_TBL_PRODUCT].wLangID), (const wchar_t *)gpStrProduct);
    //memcpy_s(gStrTable[STR_TBL_PRODUCT].wLangID, STRING_ARR_SIZE, gpStrProduct, STRING_ARR_SIZE);
    CopyMem(gStrTable[STR_TBL_PRODUCT].wLangID, gpStrProduct,STRING_ARR_SIZE);
        DEBUG ((DEBUG_INFO, "STR_TBL_PRODUCT=%s, Length=%x\n", gStrTable[STR_TBL_PRODUCT].wLangID, STRING_ARR_SIZE));


    gStrTable[STR_TBL_PRODUCT].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_PRODUCT].wLangID)));

    gStrTable[STR_TBL_SERIAL_NUMBER].bDescriptorType = USB_STRING;
//    strcpy_s((wchar_t *)(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID), STRING_ARR_SIZE, (const wchar_t *)gpStrSerialNumber);
//    strcpy((wchar_t *)(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID),(const wchar_t *)gpStrSerialNumber);
    //memcpy_s(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID, STRING_ARR_SIZE, gpStrSerialNumber, STRING_ARR_SIZE);
    CopyMem(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID,gpStrSerialNumber, STRING_ARR_SIZE);
        DEBUG ((DEBUG_INFO, "STR_TBL_SERIAL_NUMBER=%s, Length=%x\n", gStrTable[STR_TBL_SERIAL_NUMBER].wLangID, STRING_ARR_SIZE));
  
  
  
    gStrTable[STR_TBL_SERIAL_NUMBER].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID)));

    gStrTable[STR_TBL_CONFIG].bDescriptorType = USB_STRING;
//    strcpy_s((wchar_t *)(gStrTable[STR_TBL_CONFIG].wLangID), STRING_ARR_SIZE, (const wchar_t *)gStrConfiguration);
//    strcpy((wchar_t *)(gStrTable[STR_TBL_CONFIG].wLangID), (const wchar_t *)gStrConfiguration);
    //memcpy_s(gStrTable[STR_TBL_CONFIG].wLangID, STRING_ARR_SIZE, gStrConfiguration, STRING_ARR_SIZE);
    CopyMem(gStrTable[STR_TBL_CONFIG].wLangID, gStrConfiguration, STRING_ARR_SIZE);

        DEBUG ((DEBUG_INFO, "STR_TBL_CONFIG=%s, Length=%x\n", gStrTable[STR_TBL_CONFIG].wLangID, STRING_ARR_SIZE));


    gStrTable[STR_TBL_CONFIG].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_CONFIG].wLangID)));

    gStrTable[STR_TBL_INTERFACE].bDescriptorType = USB_STRING;
//    strcpy_s((wchar_t *)(gStrTable[STR_TBL_INTERFACE].wLangID), STRING_ARR_SIZE, (const wchar_t *)gStrInterface);
//      strcpy((wchar_t *)(gStrTable[STR_TBL_INTERFACE].wLangID), (const wchar_t *)gStrInterface);
    //memcpy_s(gStrTable[STR_TBL_INTERFACE].wLangID, STRING_ARR_SIZE, gStrInterface, STRING_ARR_SIZE);
    CopyMem(gStrTable[STR_TBL_INTERFACE].wLangID, gStrInterface, STRING_ARR_SIZE);
      
        DEBUG ((DEBUG_INFO, "STR_TBL_INTERFACE=%s, Length=%x\n", gStrTable[STR_TBL_INTERFACE].wLangID, STRING_ARR_SIZE));
      
    gStrTable[STR_TBL_INTERFACE].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_INTERFACE].wLangID)));
}

/* Convert strings to an unsigned long-integer value */
unsigned long Strtok (const char *nptr, int base)
{
  //
  // Null strtoul() function implementation to satisfy the linker, since there is
  // no direct functionality logic dependency in present UEFI cases.
  //
  return 0;
}


/**
 * Queues a data receive request with USB device stack. The data will be
 * received into the global Rx data buffer.
 *
 * @param   rxSize  Maximum amount of data in bytes to receive
 *
 * @return true if able to successfully queue the Rx request, false otherwise
 */
static bool fbQueueRx(uint32_t rxSize) {
    bool status = false;
    usbdIoReq ioReq;

    ioReq.epInfo.pEpDesc = &gConfigDesc.endpointOut;
    ioReq.epInfo.pEpCompDesc = &gConfigDesc.endpointCompOut;
    ioReq.ioInfo.pBuf = gpRxBuf;
    ioReq.ioInfo.dataLen = rxSize;

    /* queue the  receive request */
    if (usbdRx(&ioReq) == true) {
        status = true;
    } else {
        DEBUG ((DEBUG_INFO, "fbQueueRx() - failed to queue Rx request\n"));
    }

    return status;
}

/**
 * Queues a data transmit request with USB device stack
 *
 * @param   pBuf    Pointer to data buffer to transmit
 * @param   size    Amount of data in bytes to transmit
 *
 * @return true if able to successfully queue the Tx request, false otherwise
 */
static bool fbQueueTx(void *pBuf, uint32_t size) {
    bool status = false;
    usbdIoReq ioReq;

    ioReq.epInfo.pEpDesc = &gConfigDesc.endpointIn;
    ioReq.epInfo.pEpCompDesc = &gConfigDesc.endpointCompIn;
    ioReq.ioInfo.pBuf = pBuf;
    ioReq.ioInfo.dataLen = size;

    /* queue the Tx request */
    if (usbdTx(&ioReq) == true) {
        status = true;
    } else {
        DEBUG ((DEBUG_INFO, "fbQueueTx() - failed to queue Tx request\n"));
    }

    return status;
}

VOID
ExtractSize	(
  CHAR8	*pCmd,
  CHAR8	*SizeAscii
  )
{
  UINTN i = 0;
  UINTN j = 0;

  while (*(pCmd + i) != '\0') {
    if (*(pCmd + i) == 0x3A) { //Found the size
      *(SizeAscii + j) = *(pCmd + i + 1);
    }
    i++;
    j++;
  }

  *(SizeAscii + 1) = '\0';
}


VOID
FbAsciiToUnicode (
  IN UINT8   *Source,
  OUT CHAR16  *Destination
  )
{
  while (*Source != '\0')  {
   *(Destination++) = (CHAR16) *(Source++);
  }
  *(Destination) = '\0';
}

VOID
FbRespAck(char 		*Resp )
{
  if (NULL == Resp) return;
  
  *(Resp)     = 0x4F;  //'O'
  *(Resp + 1) = 0x4B;  //'K'
  *(Resp + 2) = 0x41;  //'A'
  *(Resp + 3) = 0x59;  //'Y'
  *(Resp + 4) = 0x0;   //'\0'
  *(Resp + 5) = 0x0;
  *(Resp + 6) = 0x0;
  *(Resp + 7) = 0x0;
  *(Resp + 8) = 0x0;

}

/**
 * Processes Fastboot commands accordingly. Maintains the Fastboot
 * protocol.
 *
 * @param   pBuf    Pointer to the received data buffer containing command or data
 * @param   len     The length in bytes of the data contained in the buffer
 *
 * @return  true if successfully processed command/data, false otherwise
 */

static bool fbProcessCmd(void *pBuf, uint32_t len) {
  bool      status = false;
  char      *pCmd = NULL;
  char      *Temp = NULL;  
  char      Resp[40];
  uint32_t  rxBytes = len;
  uint32_t  nextRxSize = FB_RX_BUFF_MAX_SIZE;
  uint32_t  i;
  CHAR16    *SizeUnicode = NULL;
  CHAR8     *SizeAscii = NULL;

  UINTN                                 BlockIoPpiInstance;
  EFI_PEI_RECOVERY_BLOCK_IO_PPI         *BlockIoPpi;
  PEI_MMC_PARTITION_DATA                *Partition;
  UINT8                                 PartitionNum;
  EFI_STATUS                            Status;
  CONST EFI_PEI_SERVICES                **PeiServices;
  UINTN                                 Size;
  //FASTBOOT_HOB_DATA                     *FastbootHobData;

//  BOOLEAN                               ProviderAvailable;
//  UINTN                                 Index;
//  UINTN                                 CheckBuffer;
//
//  UINT8                                 Digest[MAX_DIGEST_SIZE];
//  UINTN                                 CtxSize;
//  VOID                                  *HashCtx;
//  BOOLEAN                               HashResult;
//  PEI_SHA256_HASH_PPI                   *Sha256HashPpi;
//  MANIFEST_OEM_DATA                     *ManifestAddress;

  SizeUnicode = AllocateZeroPool(2*20);
  SizeAscii = AllocateZeroPool(2*20);

  PeiServices = GetPeiServicesTablePointer ();

  DEBUG ((DEBUG_INFO, ">>fbProcessCmd() - Entered - Data Length Received: 0x%x\n", len));

  //Process only if we have received any data
  if (len > 0) {
    pCmd = (char*)pBuf; //pCmd holds the received data
    DEBUG ((DEBUG_INFO, "\n"));

    /* (Data download branch) check if we are in the middle of a data download */
    if (gDownloadBytesLeft > 0) {
      DEBUG ((DEBUG_INFO, "There are 0x%x bytes left.\n", gDownloadBytesLeft));
      /* protect against overlowing the bytes left counter */
      if (rxBytes > gDownloadBytesLeft) {
        rxBytes = gDownloadBytesLeft;
      }

      /* copy data to system memory */
      CopyMem ((gpKernelLoadAddr + (gDownloadSize - gDownloadBytesLeft)), pBuf, rxBytes);
      DEBUG ((DEBUG_INFO, "copy data to system memory\n"));

      gDownloadBytesLeft -= rxBytes;

      /* check if we are done receiving data */
      if (gDownloadBytesLeft == 0) {
        DEBUG ((DEBUG_INFO, "fbProcessCmd() - Download complete: bytes: 0x%x, address: 0x%x\n", gDownloadSize, gpKernelLoadAddr));
        Stage2DownloadSize = gDownloadSize;
        gDownloadSize = 0;
        DEBUG ((DEBUG_INFO, "Receive data OK\n"));
      } else {
        /* if the left over bytes are less than the transfer buffer size,
              * we need to specify to receive only what is left. The host will
              * not send a short packet at the end of the last data packet */
        DEBUG ((DEBUG_INFO, "\nWe are not done receiving data yet..\n"));

        if (gDownloadBytesLeft < FB_RX_BUFF_MAX_SIZE) {
          DEBUG ((DEBUG_INFO, "\nAnd gDownloadBytesLeft < FB_RX_BUFF_MAX_SIZE\n"));
          nextRxSize = gDownloadBytesLeft;
        }
      }
      //Response should be "OKAY"
      FbRespAck(Resp);
    } else { /* No data download active, process commands */

    DEBUG ((DEBUG_INFO, "\nCommand Received is:"));
    for(i=0;i<len;i++){
    DEBUG ((DEBUG_INFO, "%c", pCmd[i]));
    }
    DEBUG ((DEBUG_INFO, "\n"));
      
    //If the command is "download:"
    if (AsciiStrnCmp ((CHAR8*)pCmd,FB_DOWNLOAD,AsciiStrLen(FB_DOWNLOAD))== 0) {

      //ExtractSize(pCmd, SizeAscii);

      *(SizeAscii + 0) = pCmd[9];
      *(SizeAscii + 1) = pCmd[10];
      *(SizeAscii + 2) = pCmd[11];
      *(SizeAscii + 3) = pCmd[12];
      *(SizeAscii + 4) = pCmd[13];
      *(SizeAscii + 5) = pCmd[14];
      *(SizeAscii + 6) = pCmd[15];
      *(SizeAscii + 7) = pCmd[16];
      *(SizeAscii + 8) = '\0';

      DEBUG ((DEBUG_INFO, "\nString in ASCII is:"));
      for(i=0;i<30;i++){
        DEBUG ((DEBUG_INFO, "%c", SizeAscii[i]));
        if (SizeAscii[i] == '\0') {
          break;
        }
      }
        DEBUG ((DEBUG_INFO, "\n"));

      FbAsciiToUnicode (SizeAscii, SizeUnicode);

      DEBUG ((DEBUG_INFO, "\nString in Unicode is:\n"));

      for(i=0;i<30;i++){
        DEBUG ((DEBUG_INFO, "%c", SizeUnicode[i]));
        if (SizeUnicode[i] == '\0') {
          break;
        }
      }

      gDownloadSize = StrHexToUintn (SizeUnicode);

      DEBUG ((DEBUG_INFO, "\nConverted gDownloadSize is = 0x%x\n", gDownloadSize));
      DEBUG ((DEBUG_INFO, "\nEntered into processing 'download:' command!\n"));

      //gDownloadSize = 0x9C9800; //Hardcode size for now, use asciisprint() from UEFI later
      DEBUG ((DEBUG_INFO, "fbProcessCmd() - download size (harcoded): 0x%x\n", gDownloadSize));
      if (gDownloadSize == 0) {
        sprintf_s(gpTxBuf, FB_TX_BUFF_MAX_SIZE, "FAILdata invalid size");
        DEBUG ((DEBUG_INFO, "fbProcessCmd() - Invalid size 1\n"));
      } else if (gDownloadSize > FB_DOWNLOAD_MAX) {
        gDownloadSize = 0;
        DEBUG ((DEBUG_INFO, "fbProcessCmd() - Invalid size 2\n"));
        sprintf_s(gpTxBuf, FB_TX_BUFF_MAX_SIZE, "FAILdata invalid size");
      } else {
        DEBUG ((DEBUG_INFO, "\nValid size is received from host!\n"));
        gDownloadBytesLeft = gDownloadSize;
        /* if the left over bytes are less than the transfer buffer size,
              * we need to specify to receive only what is left. The host will
              * not send a short packet at the end of the last data packet */
        if (gDownloadBytesLeft < FB_RX_BUFF_MAX_SIZE) {
          nextRxSize = gDownloadBytesLeft;
        }
      }
      //Response should be "DATA<Size prepadded with zeroes to make it 8 digits>"
      *(Resp) = 0x44;      //'D'
      *(Resp + 1) = 0x41;  //'A'
      *(Resp + 2) = 0x54;  //'T'
      *(Resp + 3) = 0x41;  //'A'

      *(Resp + 4)  = *(SizeAscii + 0);
      *(Resp + 5)  = *(SizeAscii + 1);
      *(Resp + 6)  = *(SizeAscii + 2);
      *(Resp + 7)  = *(SizeAscii + 3);
      *(Resp + 8)  = *(SizeAscii + 4);
      *(Resp + 9)  = *(SizeAscii + 5);
      *(Resp + 10) = *(SizeAscii + 6);
      *(Resp + 11) = *(SizeAscii + 7); 

      *(Resp + 12) = 0x0;
      *(Resp + 12) = 0x0;
      *(Resp + 12) = 0x0;
      *(Resp + 12) = 0x0;
      *(Resp + 12) = 0x0;
 
    }else if (AsciiStrnCmp ((CHAR8*)pCmd,FB_FLASH_STAGE2,AsciiStrLen(FB_FLASH_STAGE2))== 0) {
      //If the command is "flash:fw_stage2"

      if (0x400000 == Stage2DownloadSize) {
        //
        // If the size of file is 4M, the offset need adjust.
        //
        DEBUG((EFI_D_INFO, "\nStage2 is 4M.\n"));
        gpKernelLoadAddr += 0x200000;
      }

//      CheckBuffer = (UINTN) gpKernelLoadAddr + PcdGet32 (PcdFlashFvMainBase) - PcdGet32 (PcdFlashFvShellBase);
//      Size = PcdGet32 (PcdFlashFvRecovery2Base) - PcdGet32 (PcdFlashFvMainBase) + PcdGet32 (PcdFlashFvRecovery2Size);
//      
//      Status = (**PeiServices).LocatePpi (
//                    PeiServices,
//                    &gPeiSha256HashPpiGuid,    // GUID
//                    0,                         // Instance
//                    NULL,                      // EFI_PEI_PPI_DESCRIPTOR
//                    &Sha256HashPpi             // PPI
//                    );
//      
//      DEBUG((EFI_D_INFO, "\n Stage2 Check 0x%x 0x%x\n", CheckBuffer, Size));
//      
//      ZeroMem (Digest, MAX_DIGEST_SIZE);
//      CtxSize = Sha256HashPpi->GetContextSize ();
//      HashCtx = AllocatePool (CtxSize);
//      
//      DEBUG((EFI_D_INFO, "Init... \n"));
//      HashResult  = Sha256HashPpi->HashInit (HashCtx);
//      if (!HashResult) {
//        DEBUG((EFI_D_INFO, "[Fail]\n"));
//        return false;
//      }
//      
//      DEBUG((EFI_D_INFO, "Update... \n"));
//      HashResult  = Sha256HashPpi->HashUpdate (HashCtx, (VOID*)CheckBuffer, Size);
//      if (!HashResult) {
//        DEBUG((EFI_D_INFO, "[Fail]\n"));
//        return false;
//      }
//      
//      DEBUG((EFI_D_INFO, "Finalize... \n"));
//      HashResult  = Sha256HashPpi->HashFinal (HashCtx, Digest);
//      if (!HashResult) {
//        DEBUG((EFI_D_INFO, "[Fail]\n"));
//        return false;
//      }
//      
//      ProviderAvailable = TRUE;
//      ManifestAddress = (MANIFEST_OEM_DATA*)(UINTN)MANIFEST_ADDRESS;
//      for (Index = 0; Index < ManifestAddress->Bios2ndStageHashLen; Index ++) {
//        if (Digest[Index] != *(UINT8*)(UINTN)(ManifestAddress->Sha256Hash + Index)) {
//          DEBUG((EFI_D_ERROR, "Stage2 Signed UnMatch\n"));
//          ProviderAvailable = FALSE;
//          break;
//        }
//      }
//      
//      if (ProviderAvailable == TRUE) {
        for (BlockIoPpiInstance = 0; BlockIoPpiInstance < MAX_NUMBER_OF_PARTITIONS; BlockIoPpiInstance++) {
            Status = (*PeiServices)->LocatePpi (
                                        PeiServices,
                                        &gEfiPeiVirtualBlockIoPpiGuid,
                                        BlockIoPpiInstance,
                                        NULL,
                                        (VOID **) &BlockIoPpi
                                        );
            if (EFI_ERROR (Status)) {
            //
            // Done with all Block Io Ppis
            //
             continue;
          }
          
          Partition    = PEI_CARD_PARTITION_DATA_FROM_THIS (BlockIoPpi);
          PartitionNum = (UINT8)PEI_CARD_DATA_PARTITION_NUM (Partition);
          Size         = PcdGet32 (PcdFlashArea2Size);
          
          if ((BOOT_PARTITION_1 == PartitionNum) || (BOOT_PARTITION_2 == PartitionNum)) {
            DEBUG((EFI_D_ERROR, "Find recovery partition\n"));
            Status = Partition->WriteBlocks (
                                      (EFI_PEI_SERVICES**)PeiServices,
                                      BlockIoPpi,
                                      0,
                                      0,
                                      Size,
                                      (VOID*)gpKernelLoadAddr
                                      );
            if (EFI_ERROR (Status)) {
              continue;
            }
            DEBUG((EFI_D_ERROR, "Successfully Load Fv\n"));
          }
        }
//      
//        PeiServicesInstallFvInfoPpi (
//          NULL,
//          (VOID *) ((UINTN) gpKernelLoadAddr + PcdGet32 (PcdFlashFvRecovery2Base) - PcdGet32 (PcdFlashFvShellBase)),
//          PcdGet32 (PcdFlashFvRecovery2Size),
//          NULL,
//          NULL
//          );
//        
//        Status = (*PeiServices)->InstallPpi (PeiServices, &mRecoveryPpiList);
      
        //
        //Build the Hob to enter DXE fastboot later.
        //Comment for Cherrytrail for now
		/*
        FastbootHobData = BuildGuidHob (
                             &gEfiFastbootHobGuid,
                             sizeof (UINT8)
                             );
        FastbootHobData->EnterFastboot = ENTER_FAST_BOOT;
		*/

        FbRespAck(Resp);
      
        //Prepare response buffer
        gpTxBuf = (void*) Resp;     
        status = fbQueueTx(gpTxBuf, strlen(gpTxBuf));
        DEBUG ((DEBUG_INFO, "\nEnd of cmd processing - size of Tx buffer = 0x%x", strlen(gpTxBuf)));
      
        //tell top loop to exit to continue to boot to DXE
        fbStop();
//      }
      
    }else{
        //no 'boot' command is support at PEI phase
        DEBUG ((DEBUG_INFO, "fbProcessCmd: Unsupported command\n"));
        //Anyway respond as "OKAY" for not breaking the link with Fastboot.exe application
        FbRespAck(Resp);
      }
    }
  } else {
    /* we don't have any command or data */
    DEBUG ((DEBUG_INFO, "\nfbProcessCmd: No data or command received\n"));
    //Anyway respond as "OKAY" for not breaking the link with Fastboot.exe application
    FbRespAck(Resp);
  }

  //Prepare response buffer
  gpTxBuf = (void*) Resp;
  delay(1000); /* wait for a second */

  status = fbQueueTx(gpTxBuf, strlen(gpTxBuf));
  DEBUG ((DEBUG_INFO, "\nEnd of cmd processing - size of Tx buffer = 0x%x", strlen(gpTxBuf)));

  //For printing only 
  Temp = (char*) gpTxBuf;
  DEBUG ((DEBUG_INFO, "\nResponse is: "));
  for(i=0;i<strlen(gpTxBuf);i++){
    DEBUG ((DEBUG_INFO, "%c", Temp[i]));
  }
  DEBUG ((DEBUG_INFO, "\n"));
  delay(1000); /* wait for a second */

  /* get ready for the next Rx data */
  fbQueueRx(nextRxSize);
  DEBUG ((DEBUG_INFO, ">>fbProcessCmd() - End\n"));
  return status;
}

/**
 * Callback handler for a Set Configuration event for this device
 *
 * @param cfgVal The configuration value set
 *
 * @return true if successfully handled event, false otherwise
 */
static bool fbConfigHdlr(uint8_t cfgVal) {

    bool status = false;

    DEBUG ((DEBUG_INFO, "\nfbConfigHdlr() - config value: 0x%x\n", cfgVal));

    if (cfgVal == gConfigDesc.config.bConfigurationValue) {

        /* we've been configured, get ready to receive Commands */
        fbQueueRx(FB_RX_BUFF_MAX_SIZE);
    } else {
        DEBUG ((DEBUG_INFO, "fbConfigHdlr() - invalid configuration value\n"));
    }

    return status;
}

/**
 * Callback to Handle Fastboot specific Control Setup requests
 *
 * @param pCtrlReq  Pointer to the control request
 * @param pIo       Pointer to the IO data structure
 *
 * @return boolean status
 */
static bool fbSetupHdlr(deviceRequest *pCtrlReq, usbdIoInfo *pIo) {

    /* Fastboot doesn't handle any Class/Vendor specific setup requests */

    return true;
}

/**
 * Callback to handle data transaction operations. Call the Command
 * processing routine to handle the request.
 *
 * @param   pXferInfo   Pointer to the transfer data structure
 *
 * @return true if successfully handled data, false otherwise
 */
static bool fbDataHdlr(usbdXferInfo *pXferInfo) {
    bool status = false;

    /* if we are receiving a command or data, call the processing routine */
    if (pXferInfo->epDir == USB_DIR_OUT) {
        status = fbProcessCmd(pXferInfo->pBuf, pXferInfo->dataLen);
    }

    return status;
}

/**
 * Initializes all USB device driver core objects with the data with
 * the specifics for this Fastboot driver details.
 *
 * @return void
 */
static void fbInitDriverObjs() {

    /* Device driver objects */
    gDevObj.pDeviceDesc                 = &gDevDesc;
    gDevObj.pConfigObjs                 = &gConfigObjs[0];
    gDevObj.pStringTable                = &gStrTable[0];
    gDevObj.strTblEntries               = STR_TBL_COUNT;
    gDevObj.pConfigCallback             = fbConfigHdlr;
    gDevObj.pSetupCallback              = fbSetupHdlr;
    gDevObj.pDataCallback               = fbDataHdlr;

    /* Config driver objects */
    gConfigObjs[0].pConfigDesc          = &gConfigDesc.config;
    gConfigObjs[0].pConfigAll           = &gConfigDesc;
    gConfigObjs[0].pInterfaceObjs       = &gInterfaceObjs[0];

    /* Interface driver objects */
    gInterfaceObjs[0].pInterfaceDesc    = &gConfigDesc.interface;
    gInterfaceObjs[0].pEndpointObjs     = &gEndpointObjs[0];

    /* Endpoint Data In/Out objects */
    gEndpointObjs[0].pEndpointDesc      = &gConfigDesc.endpointIn;
    gEndpointObjs[0].pEndpointCompDesc  = &gConfigDesc.endpointCompIn;

    gEndpointObjs[1].pEndpointDesc      = &gConfigDesc.endpointOut;
    gEndpointObjs[1].pEndpointCompDesc  = &gConfigDesc.endpointCompOut;
}

/**
 * Initializes the Fastboot driver, configures basic
 * descriptors and binds to the USB device driver core.
 *
 * @param pParams   Pointer to parameter structure for configuring Fastboot
 *
 * @return true if initialization was successful, false otherwise
 */
bool fbInit(fbParams *pParams) {

    bool status = false;


    DEBUG ((DEBUG_INFO, "fbInit() - Start\n"));
    DEBUG ((DEBUG_INFO, "devProductId=%x\n", pParams->devProductId));
    DEBUG ((DEBUG_INFO, "devBcd=%x\n", pParams->devBcd));
    DEBUG ((DEBUG_INFO, "gStrProduct=%s\n", pParams->pDevProductStr));
    DEBUG ((DEBUG_INFO, "pDevSerialNumStr=%s\n", pParams->pDevSerialNumStr));
    DEBUG ((DEBUG_INFO, "kernelLoadAddr=%x\n", pParams->kernelLoadAddr));
    DEBUG ((DEBUG_INFO, "kernelEntryOffset=%x\n", pParams->kernelEntryOffset));
    DEBUG ((DEBUG_INFO, "timeoutMs=%x\n", pParams->timeoutMs));
    
    if (gIsInitialized == false) {

        /* configure according to parameters */
        gpKernelLoadAddr = (char*)(pParams->kernelLoadAddr - FB_MK_IMAGE_HDR_SIZE);
        gKernelEntryOffset = pParams->kernelEntryOffset;
        gTimeoutMs = pParams->timeoutMs;
        gDevDesc.idProduct = pParams->devProductId;
        gDevDesc.bcdDevice = pParams->devBcd;

        if (pParams->pDevProductStr != NULL) {
            gpStrProduct = pParams->pDevProductStr;
        } else {
            gpStrProduct = gEmptyString;
        }
        if (pParams->pDevSerialNumStr != NULL) {
            gpStrSerialNumber = pParams->pDevSerialNumStr;
        } else {
            gpStrSerialNumber = gEmptyString;
        }

        /* setup string descriptors for this device class driver */
        fbInitStringDescriptors();

        /* setup all driver objects within this device stack */
        fbInitDriverObjs();

        /* Bind this Fastboot layer to the USB device driver layer */
        if (usbdBind(&gDevObj) == true) {

            /* allocate the data buffers */
            gpRxBuf = (void*)AllocatePool(FB_RX_BUFF_MAX_SIZE);
            if (gpRxBuf != NULL) {
                gpTxBuf = (void*)AllocatePool(FB_TX_BUFF_MAX_SIZE);
                if (gpTxBuf != NULL) {
                    gIsInitialized = true;
                    status = true;
                } else {
                    DEBUG ((DEBUG_INFO, "fbInit() - Failed to allocate Tx buffer\n"));
                }
            } else {
                DEBUG ((DEBUG_INFO, "fbInit() - Failed to allocate Rx buffer (len:0x%x)\n", FB_RX_BUFF_MAX_SIZE));
            }
        } else {
            DEBUG ((DEBUG_INFO, "fbInit() - Failed to initialize USB Device driver layer\n"));
        }
    } else {
        DEBUG ((DEBUG_INFO, "fbInit() - Already initialized\n"));
        status = true; /* just ignore the attempt to re-initialize */
    }

    return status;
}

/**
 * Attempt to de-initialize the device including unbinding from the device core
 *
 * @return true if able to de-initialize, false otherwise
 */
bool fbDeinit() {
    bool status = false;

    if (gIsInitialized == true) {

        /* Unbind this Fastboot layer to the USB device driver layer */
        if (usbdUnbind(&gDevObj) == true) {

            /* reset parameter data */
            gpKernelLoadAddr = 0x0;
            gKernelEntryOffset = 0x0;
            gTimeoutMs = 0x0;
            gDevDesc.idProduct = 0x0;
            gDevDesc.bcdDevice = 0x0;

            /* Disable all driver objects */
            gDevObj.pDeviceDesc     = NULL;
            gDevObj.pConfigObjs     = NULL;
            gDevObj.pStringTable    = NULL;
            gDevObj.strTblEntries   = 0;
            gDevObj.pSetupCallback  = NULL;
            gDevObj.pDataCallback   = NULL;

            /* free allocated data */
            free(gpRxBuf);
            free(gpTxBuf);

            gIsInitialized = false;
            status = true;
        } else {
            DEBUG ((DEBUG_INFO, "fbDeinit() - Failed to deinitialize USB Device driver layer\n"));
        }
    } else {
        DEBUG ((DEBUG_INFO, "fbDeinit() - Not initialized\n"));
        status = true; /* just ignore the attempt to de-initialize */
    }

    return status;
}

/**
 * Starts USB device interactions with a Host. It connects and
 * begins to attempt to receive/send data transactions.
 *
 * @return  true if data processing started and ended correctly,
 *          false if failed to start or error occurred in processing
 */
bool fbStart() {
    bool status = false;

    if (gIsInitialized == true) {
        if (usbdConnect() == true) {
        	  DEBUG ((DEBUG_INFO, "gTimeoutMs=%x\n", gTimeoutMs));
            if (usbdRun(gTimeoutMs) == true) {
                status = true;
            } else {
                DEBUG ((DEBUG_INFO, "fbStart() - Error occurred during run\n"));
            }
        } else {
            DEBUG ((DEBUG_INFO, "fbStart() - Failed to connect\n"));
        }
    } else {
        DEBUG ((DEBUG_INFO, "fbStart() - Not initialized\n"));
    }

    return status;
}

/**
 * Stop and disconnect the device from the bus
 *
 * @return bool if able to successfully disconnect, false otherwise
 */
bool fbStop() {
    bool status = false;

    if (gIsInitialized == true) {
        usbdStop();
        if (usbdDisconnect() == true) {
            status = true;
        } else {
            DEBUG ((DEBUG_INFO, "fbStop() - Failed to disconnect\n"));
        }
    }

    return status;
}

