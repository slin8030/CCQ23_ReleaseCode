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


#include "usb.h"
#include "usbd.h"
#include "usb_fb.h"
//#include "delay.h"
#include "UdmWrapperLib.h"
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Library/PrintLib.h>

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

#define CFG_MAX_POWER               (0xFA) /* Max power consumption of the USB device from the bus for this config */

#define IN_ENDPOINT_NUM             (1)
#define OUT_ENDPOINT_NUM            (2)

#define VENDOR_ID                   (0x8087)  /* Intel Inc. */

#define FB_IF_SUBCLASS              (0x42) /* Fastboot subclass */
#define FB_IF_PROTOCOL              (0x03) /* Fastboot protocol */

#define FB_BULK_EP_PKT_SIZE         USB_BULK_EP_PKT_SIZE_HS /* default to using high speed */
#define FB_RX_BUFF_MAX_SIZE         (8 * 1024 * 1024) // Enlarge the size to speed up transfer. /* size of buffer for receive transactions */
#define FB_TX_BUFF_MAX_SIZE         (0x80) /* size of buffer for transmit transactions */

#define FB_DOWNLOAD_MAX             (0x10000000) /* maximum allowed download size from a host */

#define FB_DROIDBOOT_LOAD_ADDRESS   (0x30000000)//(0x76311010)
#define FB_COMMANDLINE_SIZE         (40)
#define FB_EFILINUX_LOADER_SIZE     (400000)


/*
 * Note: The host fastboot application adds an image header to images
 * transfered to the device of size 2K.
 */
#define FB_MK_IMAGE_HDR_SIZE        (2048)

/* Addresses/Offsets for loading and starting a downloaded kernel */

static CHAR8 *gpKernelLoadAddr = NULL;
static CHAR8 *gOsloader = NULL;
//static UINT32 *gFlag = 0;

static EFI_HANDLE mFastBootHandle;

static UINT32 gKernelEntryOffset;

/* USB connect timeout */
static UINT32 gTimeoutMs = 0;

/* global variables to dynamically allocated data buffers */
static VOID *gpRxBuf = NULL;
static VOID *gpTxBuf = NULL;

/* global vars for command operations */
static UINT32 gDownloadSize = 0;
static UINT32 gLoaderSize = 0;
static UINT32 gDownloadBytesLeft = 0;

/* Complete Configuration structure for Fastboot */
#pragma pack(1)
typedef struct {
    configurationDescriptor     config;
    interfaceDescriptor         interface;
    endpointDescriptor          endpointIn;
    endpointDescriptor          endpointOut;
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
static BOOLEAN gIsInitialized = false;

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
    {
       sizeof(endpointDescriptor),
       USB_ENDPOINT,
       OUT_ENDPOINT_NUM | USB_DIR_OUT,
       USB_EP_XFER_BULK,
       FB_BULK_EP_PKT_SIZE,
       0x00 /* Not specified for bulk endpoint */
    }
};

/**
 * Initializes string descriptor table with all strings required
 * by this Fastboot device.
 *
 * @return VOID
 */
static VOID fbInitStringDescriptors() {

    gStrTable[STR_TBL_LANG].bDescriptorType = USB_STRING;
    gStrTable[STR_TBL_LANG].wLangID[0] = 0x0409; /* English defined as 0x0409 */
    gStrTable[STR_TBL_LANG].bLength = 4;

    gStrTable[STR_TBL_MANUFACTURER].bDescriptorType = USB_STRING;
    CopyMem(gStrTable[STR_TBL_MANUFACTURER].wLangID,gStrManufacturer, STRING_ARR_SIZE);


    DEBUG ((DEBUG_INFO, "STR_TBL_MANUFACTURER=%s, Length=%x\n", gStrTable[STR_TBL_MANUFACTURER].wLangID, STRING_ARR_SIZE));


    gStrTable[STR_TBL_MANUFACTURER].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_MANUFACTURER].wLangID)));

    gStrTable[STR_TBL_PRODUCT].bDescriptorType = USB_STRING;
    CopyMem(gStrTable[STR_TBL_PRODUCT].wLangID, gpStrProduct,STRING_ARR_SIZE);

    gStrTable[STR_TBL_PRODUCT].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_PRODUCT].wLangID)));

    gStrTable[STR_TBL_SERIAL_NUMBER].bDescriptorType = USB_STRING;
    CopyMem(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID,gpStrSerialNumber, STRING_ARR_SIZE);
  
  
    gStrTable[STR_TBL_SERIAL_NUMBER].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_SERIAL_NUMBER].wLangID)));

    gStrTable[STR_TBL_CONFIG].bDescriptorType = USB_STRING;
//    strcpy_s((wchar_t *)(gStrTable[STR_TBL_CONFIG].wLangID), STRING_ARR_SIZE, (const wchar_t *)gStrConfiguration);
//    strcpy((wchar_t *)(gStrTable[STR_TBL_CONFIG].wLangID), (const wchar_t *)gStrConfiguration);
    //memcpy_s(gStrTable[STR_TBL_CONFIG].wLangID, STRING_ARR_SIZE, gStrConfiguration, STRING_ARR_SIZE);
    CopyMem(gStrTable[STR_TBL_CONFIG].wLangID, gStrConfiguration, STRING_ARR_SIZE);

    DEBUG ((DEBUG_INFO, "STR_TBL_CONFIG=%s, Length=%x\n", gStrTable[STR_TBL_CONFIG].wLangID, STRING_ARR_SIZE));


    gStrTable[STR_TBL_CONFIG].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_CONFIG].wLangID)));

    gStrTable[STR_TBL_INTERFACE].bDescriptorType = USB_STRING;

    CopyMem(gStrTable[STR_TBL_INTERFACE].wLangID, gStrInterface, STRING_ARR_SIZE);

    DEBUG ((DEBUG_INFO, "STR_TBL_INTERFACE=%s, Length=%x\n", gStrTable[STR_TBL_INTERFACE].wLangID, STRING_ARR_SIZE));
      
    gStrTable[STR_TBL_INTERFACE].bLength = (uint8_t)(2 + (2 * wcslen(gStrTable[STR_TBL_INTERFACE].wLangID)));
}

/* Convert strings to an unsigned long-integer value */
unsigned long Strtok (const CHAR8 *nptr, int base)
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
static BOOLEAN fbQueueRx(UINT32 rxSize) {
    BOOLEAN status = false;
    usbdIoReq ioReq;

    DEBUG ((DEBUG_INFO, "fbQueueRx()Rx request rxSize is 0x%x\n", rxSize));
    ioReq.epInfo.pEpDesc = &gConfigDesc.endpointOut;
    ioReq.epInfo.pEpCompDesc = NULL;
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
static BOOLEAN fbQueueTx(VOID *pBuf, UINT32 size) {
    BOOLEAN status = false;
    usbdIoReq ioReq;

    ioReq.epInfo.pEpDesc = &gConfigDesc.endpointIn;
    ioReq.epInfo.pEpCompDesc = NULL;
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

/**
 * Extracts the size from "DATA:<size>"
 *
 * @param   pCmd    Input pointer to data received
 * @param   SizeAscii    Input pointer where size needs to be 
 *                       stored. Output pointer to Size in
 *                       ASCII.
 *
 * @return None
 */

VOID
ExtractSize (
  CHAR8    *pCmd,
  CHAR8    *SizeAscii
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

/**
 * Extracts the size from "DATA:<size>"
 *
 * @param   Source        Input pointer to ASCII string
 * @param   Destination   Input pointer to empty unicode 
 *                        string. Output pointer to unicode
 *                        string.
 *
 *
 * @return None
 */

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

/**
 * Extracts the size from "DATA:<size>"
 *
 * @param   String1    Input pointer to ASCII string 1
 * @param   String2    Input pointer to ASCII string 2
 * @param   Size       Number of bytes to compare
 *
 *
 *
 * @return TRUE if equal, FALSE if not.
 */


BOOLEAN
FbAsciiStrCmp (
  IN CHAR8     *String1,
  IN CHAR8     *String2,
  IN UINT32    Size
  )
{
  UINT32 i = 0;

  for (i = 0; i<Size; i++) {
    if (*(String1 + i) != *(String2 + i)) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
 * Copies from one ASCII string to another
 *
 * @param   String1    Input pointer to ASCII string 1
 * @param   String2    Output pointer to ASCII string 2
 * @param   Size       Number of bytes to copy
 *
 *
 *
 * @return None
 */


VOID
FbAsciiStrCpy (
  IN  CHAR8     *String1,
  OUT CHAR8     *String2,
  IN  UINT32    Size
  )
{
  UINT32 i = 0;

  for (i = 0; i<Size; i++) {
    *(String2 + i) = *(String1 + i);
  }

  *(String2 + i) = '\0';
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

static BOOLEAN fbProcessCmd (
  VOID      *pBuf,
  UINT32    len
  )
{
  BOOLEAN                    status = false;
  CHAR8                      *pCmd = NULL;
  CHAR8                      *Temp = NULL;  
  CHAR8                      *Resp = NULL;
  UINT32                     rxBytes = len;
  UINT32                     nextRxSize = FB_RX_BUFF_MAX_SIZE;
  UINT32                     i;
  CHAR16                     *SizeUnicode = NULL;
  CHAR8                      *SizeAscii = NULL;
  CHAR16                     *ParameterList = NULL;
  EFI_STATUS                 Status = EFI_SUCCESS;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImage;
  EFI_GUID                   gEfiLoadedImageProtocolGuid = {0x5B1B31A1, 0x9562, 0x11d2, 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B};
  BOOLEAN                    DownloadComplete = TRUE;

  //Responses
  CHAR8                      AsciiMsgOkay[4] = {'O', 'K', 'A', 'Y'};
  CHAR8                      AsciiMsgData[4] = {'D', 'A', 'T', 'A'};
  CHAR8                      AsciiMsgFail[19] = {'F', 'A', 'I', 'L','u', 'n', 'k', 'n','o', 'w', 'n', ' ' , 'c', 'o', 'm', 'm','a', 'n', 'd'};

  //Commands
  CHAR8                      AsciiMsgDownload[9] = {'d', 'o', 'w', 'n', 'l', 'o', 'a', 'd', ':'};
  CHAR8                      AsciiMsgBoot[4] = {'b', 'o', 'o', 't'};
  CHAR8                      AsciiMsgFlashOsLoader[14] = {'f', 'l', 'a', 's', 'h', ':', 'o', 's', 'l', 'o', 'a', 'd', 'e', 'r'};
  CHAR8                      AsciiMsgContinue[8] = {'c', 'o', 'n', 't', 'i', 'n', 'u', 'e'};

  SizeUnicode = AllocateZeroPool (2 * 20);
  ASSERT (SizeUnicode != NULL);
  SizeAscii = AllocateZeroPool (2 * 20);
  ASSERT (SizeAscii != NULL);
  ParameterList = AllocateZeroPool (2 * FB_COMMANDLINE_SIZE);
  Resp = AllocateZeroPool (20);
  ASSERT (Resp != NULL);

  DEBUG((DEBUG_INFO, "fbProcessCmd() - Entered - Data Length Received: 0x%x\n", len)); 

  //Process only if we have received any data
  if (len > 0) {
    pCmd = (CHAR8*)pBuf; //pCmd holds the received data
    DEBUG ((DEBUG_INFO, "\n"));

    /* check if we are in the middle of a data download */
    if (gDownloadBytesLeft > 0) {
      DEBUG ((DEBUG_INFO, "There are download bytes left. gDownloadBytesLeft: 0x%x\n", gDownloadBytesLeft));

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
        gLoaderSize = gDownloadSize;
        gDownloadSize = 0;
        DownloadComplete = TRUE;

        nextRxSize = FB_RX_BUFF_MAX_SIZE;
        //Response should be "OKAY"
        FbAsciiStrCpy(AsciiMsgOkay, Resp, 4);
        //sprintf_s(gpTxBuf, FB_TX_BUFF_MAX_SIZE, "OKAY");
        DEBUG ((DEBUG_INFO, "Receive data OK\n"));

      } else {
        /* if the left over bytes are less than the transfer buffer size,
        * we need to specify to receive only what is left. The host will
        * not send a short packet at the end of the last data packet */
        DownloadComplete = FALSE;
        DEBUG ((DEBUG_INFO, "\nWe are not done receiving data yet..\n"));

        if (gDownloadBytesLeft < FB_RX_BUFF_MAX_SIZE) {
          DEBUG ((DEBUG_INFO, "\nAnd gDownloadBytesLeft < FB_RX_BUFF_MAX_SIZE\n"));
          nextRxSize = gDownloadBytesLeft;
        }
      }

    } else { /* No data download active, process commands */

      DEBUG ((DEBUG_INFO, "\nCommand Received is:\n"));

      for (i = 0; i < len; i++) {
        DEBUG ((DEBUG_INFO, "%c", pCmd[i]));
      }

      //If the command is "download:"
      if (FbAsciiStrCmp(pCmd, AsciiMsgDownload, 9)) {
        *(SizeAscii + 0) = pCmd[9];
        *(SizeAscii + 1) = pCmd[10];
        *(SizeAscii + 2) = pCmd[11];
        *(SizeAscii + 3) = pCmd[12];
        *(SizeAscii + 4) = pCmd[13];
        *(SizeAscii + 5) = pCmd[14];
        *(SizeAscii + 6) = pCmd[15];
        *(SizeAscii + 7) = pCmd[16];
        *(SizeAscii + 8) = '\0';

        FbAsciiToUnicode (SizeAscii, SizeUnicode);
        gDownloadSize = (UINT32)(StrHexToUintn (SizeUnicode));
        DEBUG ((DEBUG_INFO, "\nConverted gDownloadSize is = 0x%x\n", gDownloadSize));
        DEBUG ((DEBUG_INFO, "\nEntered into processing 'download:' command!\n"));
        //gDownloadSize = 0x9C9800; //Hardcode size for now, use asciisprint() from UEFI later
        DEBUG ((DEBUG_INFO, "fbProcessCmd() - download size: 0x%x\n", gDownloadSize));

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

        //Response should be "DATA<Size prepadded with zeroes to make it 8 digits>
        FbAsciiStrCpy(AsciiMsgData, Resp, 4);
        FbAsciiStrCpy(SizeAscii, (Resp + 4), 8);

      } else if (FbAsciiStrCmp(pCmd, AsciiMsgBoot, 4)) { //If the command is "boot"
        DEBUG ((DEBUG_INFO, "\nEntered into processing 'boot' command!\n"));
        FbAsciiStrCpy(AsciiMsgOkay, Resp, 4);
        gpTxBuf = (VOID*) Resp;

        fbQueueTx(gpTxBuf, (UINT32)(strlen(gpTxBuf)));
        delay(1000); /* wait for a second */

        DEBUG ((DEBUG_INFO, "\nIn BOOT command process - gpKernelLoadAddr is located at = 0x%x\n", gpKernelLoadAddr));
        DEBUG ((DEBUG_INFO, "\nIn BOOT command process - gOsloader is located at = 0x%x\n", gOsloader));

        //This time gpKernelLoadAddr has droidboot.img
        UnicodeSPrint (ParameterList, FB_COMMANDLINE_SIZE, L" -a 0x%x", (gpKernelLoadAddr));
        DEBUG ((DEBUG_INFO, "Parameter List is: "));

        for (i = 0; i < FB_COMMANDLINE_SIZE; i++) {
          if (ParameterList != NULL) {
            DEBUG ((DEBUG_INFO, "%c", *(ParameterList + i)));
            if (*(ParameterList + i) == '\0') {
              break;
            }
          }
        }

        Status = gBS->HandleProtocol (mFastBootHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);

        if (!EFI_ERROR(Status)) {
          DEBUG ((EFI_D_INFO, "[Fastboot] - HandleProtocol() for Loaded Image Protocol passed. Status = 0x%x\n", Status));
          if (ParameterList != NULL) {
            LoadedImage->LoadOptionsSize  = (UINT32) StrLen (ParameterList);
          }
        } else {
          DEBUG ((EFI_D_INFO, "[Fastboot] - HandleProtocol() for Loaded Image Protocol failed. Status = 0x%x\n", Status));
        }

        LoadedImage->LoadOptions = (VOID*) ParameterList;

        //DEBUG ((EFI_D_INFO, "[Fastboot] - ParameterList is %s.\n", ParameterList));
        fbStop();

        Status = gBS->StartImage (mFastBootHandle, NULL, NULL);
        if (!EFI_ERROR(Status)) {
          DEBUG ((EFI_D_INFO, "[Fastboot] - StartImage() passed. Status = 0x%x\n", Status));
        } else {
          DEBUG ((EFI_D_INFO, "[Fastboot] - StartImage() failed. Status = 0x%x\n", Status));
        }

      } else if (FbAsciiStrCmp (pCmd, AsciiMsgContinue, 8)) {//If the command is "continue"
        DEBUG ((DEBUG_INFO, "\nEntered into processing 'continue' command!\n"));
        DEBUG ((DEBUG_INFO, "\nSending response to host before booting Kernel.\n"));
        FbAsciiStrCpy(AsciiMsgOkay, Resp, 4);
        gpTxBuf = (VOID*) Resp;
        fbQueueTx(gpTxBuf, (UINT32)strlen(gpTxBuf));

        delay(1000); /* wait for a second */

        if (gOsloader != NULL) {
          if (ParameterList != NULL) {
            UnicodeSPrint (ParameterList, FB_COMMANDLINE_SIZE, L" -a");
            DEBUG ((DEBUG_INFO, "Parameter List is: "));
            for (i = 0; i < FB_COMMANDLINE_SIZE; i++) {
              DEBUG ((DEBUG_INFO, "%c", *(ParameterList + i)));
              if (*(ParameterList + i) == '\0') {
                break;
              }
            }
            DEBUG ((DEBUG_INFO, "\n\n"));
        
            Status = gBS->HandleProtocol (mFastBootHandle, &gEfiLoadedImageProtocolGuid, (VOID **) &LoadedImage);
            if (!EFI_ERROR(Status)) {
              if (ParameterList != NULL)
                LoadedImage->LoadOptionsSize  = (UINT32) StrLen (ParameterList);
            
              LoadedImage->LoadOptions = (VOID *) ParameterList;
              DEBUG ((DEBUG_INFO, "LoadedImage->LoadOptionsSize is: 0x%x \n", LoadedImage->LoadOptionsSize));
            }
          }

          //
          // Osloader.efi has been sent to dram
          //
          Status = gBS->StartImage (mFastBootHandle, NULL, NULL);
          if (!EFI_ERROR(Status)) {
            DEBUG ((EFI_D_INFO, "[Fastboot] - StartImage() passed. Status = 0x%x\n", Status));
          } else {
            DEBUG ((EFI_D_INFO, "[Fastboot] - StartImage() failed. Status = 0x%x\n", Status));
          }
        }


        /*
         * Setup the kernel base address and the kernel start.
         * We need to skip the fastboot image header.
         */
        //pKernelBase = gpKernelLoadAddr + FB_MK_IMAGE_HDR_SIZE;
        //pKernelStart = pKernelBase + gKernelEntryOffset;

        /*
        DEBUG ((DEBUG_INFO, "fbProcessCmd() - Booting Kernel at address 0x%x\n", pKernelStart));

        FastbootStartImage ((UINT8*) pKernelBase);
        */

      } else if (FbAsciiStrCmp(pCmd, AsciiMsgFlashOsLoader, 14)) {//If the command is "flash:osloader"

        DEBUG ((DEBUG_INFO, "\nEntered into processing 'flash:osloader' command!\n"));
        DEBUG ((DEBUG_INFO, "\ngDownloadSize = %d\n", gLoaderSize));

        //Allocate memory for Osloader.efi
        gOsloader = AllocateZeroPool(gLoaderSize);

        //This time gpKernelLoadAddr has Osloader.efi
        //Relocate Osloader.efi
        CopyMem ((VOID*)gOsloader, (VOID*)(gpKernelLoadAddr), gLoaderSize);

        DEBUG ((DEBUG_INFO, "\nIn BOOT command process - gOsloader is located at = 0x%x\n", gOsloader));

        //Shreesh - Load Osloader.efi starts

        //Load Image
        Status = gBS->LoadImage (
                        FALSE,
                        gImageHandle,
                        NULL,
                        gOsloader,
                        gLoaderSize,
                        &mFastBootHandle
                        );

        if (EFI_ERROR(Status)) {
          DEBUG ((EFI_D_INFO, "[Fastboot] - LoadImage() failed. Status = 0x%x\n", Status));
          if (mFastBootHandle != NULL) {
            gBS->UnloadImage (mFastBootHandle);
          }
        } else {
          DEBUG ((EFI_D_INFO, "[Fastboot] - LoadImage() passed. Status = 0x%x\n", Status));
          //FreePool(gOsloader);
        }

        FbAsciiStrCpy(AsciiMsgOkay, Resp, 4);

      } else {
        DEBUG ((DEBUG_INFO, "fbProcessCmd() - new command - not part of fastboot code\n"));

        //Respond as "FAILunknow command" for not breaking the link with Fastboot.exe application
        FbAsciiStrCpy(AsciiMsgFail, Resp, 19);
      }
    }
  } else {
    /* we don't have any command or data */
    DEBUG ((DEBUG_INFO, "\nfbProcessCmd: No data or command received\n"));

    //Anyway respond as "FAILunknow command" for not breaking the link with Fastboot.exe application
    FbAsciiStrCpy(AsciiMsgFail, Resp, 19);

  }

  if (DownloadComplete == TRUE) {
    //Prepare response buffer
    gpTxBuf = (VOID*) Resp;
    delay(1000); /* wait for a second */
    status = fbQueueTx(gpTxBuf, (UINT32)strlen(gpTxBuf));
    DEBUG ((DEBUG_INFO, "\nEnd of cmd processing - size of Tx buffer = 0x%x", strlen(gpTxBuf)));

    //For printing only 
    Temp = (CHAR8*) gpTxBuf;
    DEBUG ((DEBUG_INFO, "\nResponse is: "));
    for (i = 0; i < strlen(gpTxBuf); i++) {
      DEBUG ((DEBUG_INFO, "%c", Temp[i]));
    }
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
static BOOLEAN fbConfigHdlr(uint8_t cfgVal) {

    BOOLEAN status = false;

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
 * @return BOOLEAN status
 */
static BOOLEAN fbSetupHdlr(deviceRequest *pCtrlReq, usbdIoInfo *pIo) {

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
static BOOLEAN fbDataHdlr(usbdXferInfo *pXferInfo) {
    BOOLEAN status = false;

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
 * @return VOID
 */
static VOID fbInitDriverObjs() {

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

    gEndpointObjs[1].pEndpointDesc      = &gConfigDesc.endpointOut;
}

/**
 * Initializes the Fastboot driver, configures basic
 * descriptors and binds to the USB device driver core.
 *
 * @param pParams   Pointer to parameter structure for configuring Fastboot
 *
 * @return true if initialization was successful, false otherwise
 */
BOOLEAN fbInit(fbParams *pParams) {

    BOOLEAN status = false;


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
        gpKernelLoadAddr = (CHAR8*) (UINTN)FB_DROIDBOOT_LOAD_ADDRESS;
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
            gpRxBuf = (VOID*)malloc(FB_RX_BUFF_MAX_SIZE);
            if (gpRxBuf != NULL) {
                gpTxBuf = (VOID*)malloc(FB_TX_BUFF_MAX_SIZE);
                if (gpTxBuf != NULL) {
                    gIsInitialized = true;
                    status = true;
                } else {
                    DEBUG ((DEBUG_INFO, "fbInit() - Failed to allocate Tx buffer\n"));
                }
            } else {
                DEBUG ((DEBUG_INFO, "fbInit() - Failed to allocate Rx buffer\n"));
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
BOOLEAN fbDeinit() {
    BOOLEAN status = false;

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
BOOLEAN fbStart() {
    BOOLEAN status = false;

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
 * @return BOOLEAN if able to successfully disconnect, false otherwise
 */
BOOLEAN fbStop() {
    BOOLEAN status = false;

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

