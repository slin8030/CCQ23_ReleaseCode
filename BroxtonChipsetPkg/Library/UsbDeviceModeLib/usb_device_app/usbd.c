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

 /** \file usbd.c
 *
 * USB Device driver implementation
 *
 * This USB driver is meant to be a USB device layer that abstracts
 * generic operations needed by all types of USB classes/functions.
 * It interfaces with a device interface abstraction layer and
 * not directly with a device controller driver to allow for easily
 * changing device controllers without affecting this layer.
 *
 * The USB Device stack is therefore layered as follows:
 *
 *    Application layer
 *         |
 *    Class/Function Driver layer
 *         |
 *    USBD core driver layer (this driver)
 *         |
 *    UDCI Abstraction layer
 *         |
 *    DCI Driver
 */

//#include <string.h>
//#include <stdlib.h>
#include "usbd.h"
#include "udci.h"
#include "UdmWrapperLib.h"
#include "usbd_util.h"
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/SimpleTextOut.h>

#define malloc(size)  AllocatePool (size);


/* global macros for min/max computation */
#ifndef MAX
#define MAX( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

/* Global USBD driver object. This is the main private driver object
 * that contains all data needed for this driver to operate. */
static usbdDrvObj gDrvObj;

/* global data IO transaction request object */
static usbdIoReq gCtrlIoReq = {
    /* IO information containing the buffer and data size */
    {  
    	NULL,
      0,
    },
    /* Note: This object is used for Control Ep transfers only
     * therefore the endpoint info must always be NULL */
    {
      NULL,
      NULL,
    }

#if 0
    .ioInfo = {
        .pBuf = NULL,
        .dataLen = 0,
    },
    /* Note: This object is used for Control Ep transfers only
     * therefore the endpoint info must always be NULL */
    .epInfo = {
        .pEpDesc = NULL,
        .pEpCompDesc = NULL,
    }
#endif    
};

/* global flag to signal device event processing loop to run/stop */
static bool gRun = false;

/**
 * Returns the configuration descriptor for this device. The data
 * buffer returned will also contain all downstream interface and
 * endpoint buffers.
 *
 * @param pBuf      Pointer to destination buffer to copy descriptor data to
 * @param descIndex the index of the descriptor to return
 * @param reqLen    the length in bytes of the request buffer
 * @param pDataLen  Pointer whos value is to be filled with the byte count of
 *                  data copied to the output buffer
 *
 * @return true if descritor successfully copied, false otherwise
 */
static bool usbdGetConfigDesc(void *pBuf, uint8_t descIndex, uint32_t reqLen, uint32_t *pDataLen) {

    bool status = false;
    uint8_t numConfigs = 0;
    uint32_t configLen = 0;
    usbdConfigObj *pConfigObj = 0;
    void *pDesc = 0;
    uint32_t dataLen = 0;

    DEBUG ((DEBUG_INFO, "\nusbdGetConfigDesc()\n"));

    /*
     * For a CONFIGURATION request we send back all descriptors branching out
     * from this descriptor including the INTERFACE and ENDPOINT descriptors
     */

    /* Verify the requested configuration exists - check valid index */
    numConfigs = gDrvObj.pUsbdDevObj->pDeviceDesc->bNumConfigurations;

    if (descIndex < numConfigs) {
        /* get the configuration object using the index offset */
        pConfigObj = (gDrvObj.pUsbdDevObj->pConfigObjs + descIndex);
        /* get the complete configuration buffer block including Interface and Endpoint data */
        pDesc = pConfigObj->pConfigAll;
        /* The config descriptor wTotalLength has the full value for all desc buffers */
        configLen = pConfigObj->pConfigDesc->wTotalLength;
        /* copy the data to the output buffer */
        dataLen = MIN(reqLen, configLen);
        //memcpy_s(pBuf, dataLen, pDesc, dataLen);
        CopyMem(pBuf,pDesc, dataLen);
        *pDataLen = dataLen;
        status = true;
    } else {
       DEBUG ((DEBUG_INFO, "usbdGetConfigDesc() - Invalid Config index: %i\n", descIndex));
    }

    return status;
}

/**
 * Sets the active configuration to the selected configuration index if it exists
 *
 * @param cfgValue  the configuration value to set
 *
 * @return true if the configuration was set, false otherwise
 */
static bool usbdSetConfig(uint8_t cfgValue) {
    bool status = false;
    uint8_t numConfigs = 0;
    usbdConfigObj *pConfigObj = NULL;
    usbdInterfaceObj *pIfObj = NULL;
    usbdEndpointObj *pEpObj = NULL;
    uint8_t cfgItr = 0;
    uint8_t ifItr = 0;
    uint8_t epItr = 0;
    usbdEpInfo epInfo;

    DEBUG ((DEBUG_INFO, "\nusbdSetConfig()\n"));

    /* Verify the requested configuration exists - check valid index */
    numConfigs = gDrvObj.pUsbdDevObj->pDeviceDesc->bNumConfigurations;

    if (cfgValue != 0) {
        /* Search for a matching configuration */
        for (cfgItr = 0; cfgItr < numConfigs; cfgItr++) {
            pConfigObj = (gDrvObj.pUsbdDevObj->pConfigObjs + cfgItr);
            if (pConfigObj->pConfigDesc->bConfigurationValue == cfgValue) {

                /* Set the active configuration object */
                gDrvObj.pActiveConfigObj = pConfigObj;

                /* Find all interface objects for this configuration */
                for (ifItr = 0; ifItr < pConfigObj->pConfigDesc->bNumInterfaces; ifItr++) {
                    pIfObj = (pConfigObj->pInterfaceObjs + ifItr);

                    /* Configure the Endpoints in the DCI */
                    for (epItr = 0; epItr < pIfObj->pInterfaceDesc->bNumEndpoints; epItr++) {
                        pEpObj = (pIfObj->pEndpointObjs + epItr);

                        epInfo.pEpDesc = pEpObj->pEndpointDesc;
                        epInfo.pEpCompDesc = pEpObj->pEndpointCompDesc;

                        if (udciInitEp(gDrvObj.pDciDrvObj, &epInfo) == true) {
                            if (udciEnableEp(gDrvObj.pDciDrvObj, &epInfo) == true) {
                                status = true;
                            } else {
                               DEBUG ((DEBUG_INFO, "usbdSetConfig() - Failed to enable endpoint\n"));
                            }
                        } else {
                           DEBUG ((DEBUG_INFO, "usbdSetConfig() - Failed to initialize endpoint\n"));
                        }
                    }
                }

                /* Let the class driver know it is configured */
                if (status == true) {
                    if (gDrvObj.pUsbdDevObj->pConfigCallback != NULL) {
                        gDrvObj.pUsbdDevObj->pConfigCallback(cfgValue);
                    }
                }

                gDrvObj.state = USBD_STATE_CONFIGURED; /* we are now configured */

                break; /* break from config search loop */
            }
        }
    }

    if (status == false) {
       DEBUG ((DEBUG_INFO, "usbdSetConfig() - Invalid requested configuration value: %i\n", cfgValue));
    }

    return status;
}

/**
 * Returns the currently active configuration value
 *
 * @param pBuf      Pointer to destination buffer to copy configuration value to
 * @param reqLen    the length in bytes of the request buffer
 * @param pDataLen  Pointer whos value is to be filled with the byte count of
 *                  data copied to the output buffer
 *
 * @return true if config value is successfully copied, false otherwise
 */
static bool usbdGetConfig( void *pBuf, uint32_t reqLen, uint32_t *pDataLen ) {
    bool status = false;

    DEBUG ((DEBUG_INFO, "\nusbdGetConfig()\n"));

    if (reqLen >= 1) { /* length of data expected must be 1 */
        if (gDrvObj.pActiveConfigObj != NULL) { /* assure we have a config active */
            *pDataLen = 1; /* one byte for bConfigurationValue */
            *(uint8_t*)pBuf = gDrvObj.pActiveConfigObj->pConfigDesc->bConfigurationValue;

            status = true;
        } else {
           DEBUG ((DEBUG_INFO, "usbdGetConfig() - No active configuration available\n"));
        }
    } else {
       DEBUG ((DEBUG_INFO, "usbdGetConfig() - Invalid data length\n"));
    }

    return status;
}

/**
 * Returns the requested string descriptor if it exists
 *
 * @param pBuf      Pointer to destination buffer to copy descriptor data to
 * @param descIndex the index of the descriptor to return
 * @param reqLen    the length in bytes of the request buffer
 * @param pDataLen  Pointer whos value is to be filled with the byte count of
 *                  data copied to the output buffer
 *
 * @return true if descritor successfully copied, false otherwise
 *
 */
static bool usbdGetStringDesc(void *pBuf, uint8_t descIndex, uint16_t langId, uint32_t reqLen, uint32_t *pDataLen) {

    bool status = false;
    uint32_t dataLen = 0;
    stringDescriptor *pStringDesc;
    uint8_t itr = 0;
    uint8_t strLangEntries = 0;
    bool strLangFound = false;

    DEBUG ((DEBUG_INFO, "\nusbdGetStringDesc() - index: 0x%x, langId: 0x%x, reqLen: 0x%x\n", descIndex, langId, reqLen));

    /* index zero of the string table contains the supported language codes */
    if (descIndex == 0) {
        pStringDesc = (gDrvObj.pUsbdDevObj->pStringTable);
        dataLen = MIN(reqLen, pStringDesc->bLength);
        //memcpy_s(pBuf, dataLen, pStringDesc, dataLen);
        CopyMem(pBuf,pStringDesc, dataLen);
        *pDataLen = dataLen;
        status = true;
    } else {

        /*
         * Verify the requested language ID is supported. String descriptor Zero
         * (First entry in the string table) is expected to contain the language list.
         * The requested language ID is specified in the wIndex member of the request.
         */
        pStringDesc = gDrvObj.pUsbdDevObj->pStringTable; /* get language string descriptor */
        strLangEntries = ((pStringDesc->bLength - 2) >> 1);
        DEBUG ((DEBUG_INFO, "strLangEntries=%x\n", strLangEntries));

        DEBUG ((DEBUG_INFO, "\n Looking LangID: "));

        for (itr = 0; itr < strLangEntries; itr++) {
        DEBUG ((DEBUG_INFO, "\n wLangID[ %x ]= %x\n", itr, pStringDesc->wLangID[ itr ]));

            if (pStringDesc->wLangID[ itr ] == langId) {
                DEBUG ((DEBUG_INFO, "Found it\n"));
                strLangFound = true;
            }
        }



        /* If we found a matching language, attempt to get the string index requested */
        if (strLangFound == true) {
                DEBUG ((DEBUG_INFO, " strLangFound=Found, descIndex=%x, strTblEntries=%x\n", descIndex, gDrvObj.pUsbdDevObj->strTblEntries));

            if (descIndex < gDrvObj.pUsbdDevObj->strTblEntries) {
                /* get the string descriptor for the requested index */
                pStringDesc = (gDrvObj.pUsbdDevObj->pStringTable + descIndex);
                
                dataLen = MIN(reqLen, pStringDesc->bLength);
                DEBUG ((DEBUG_INFO, "reqLen=%x, bLength=%x, dataLen=%x\n", reqLen, pStringDesc->bLength, dataLen));

                //memcpy_s(pBuf, dataLen, pStringDesc, dataLen);
                CopyMem(pBuf, pStringDesc, dataLen);
                *pDataLen = dataLen;
                status = true;
                
                /* Commenting the logging of actual descriptor string to avoid timeout in fastboot */
#if 0
                printStringDescriptor(pStringDesc);//Ken_Debug
#endif
            } else {
               DEBUG ((DEBUG_INFO, "usbdSetup() - Invalid String index in USB_GET_DESCRIPTOR request\n"));
            }
        } else {
           DEBUG ((DEBUG_INFO, "usbdSetup() - Unsupported String Language ID for USB_GET_DESCRIPTOR request\n"));
        }
    }

    return status;
}

/**
 * Returns the current status for Device/Interface/Endpoint
 *
 * @param pBuf      Pointer to destination buffer to copy descriptor data to
 * @param reqType   The type of status to get
 * @param reqLen    the length in bytes of the request buffer
 * @param pDataLen  Pointer whos value is to be filled with the byte count of
 *                  data copied to the output buffer
 *
 * @return true if status successfully copied, false otherwise
 */
static bool usbdGetStatus( void *pBuf, uint8_t reqType, uint32_t reqLen, uint32_t *pDataLen ) {
    bool status = false;

    DEBUG ((DEBUG_INFO, "\nusbdGetStatus()\n"));

    if (reqLen >= 2) { /* length of data must be at least 2 bytes */
        switch (reqType & USB_RT_RECIPIENT_MASK) {
            case USB_RT_RECIPIENT_DEVICE: {
                *pDataLen = 2; /* two byte for status */
                *(uint16_t*)pBuf = USB_STATUS_SELFPOWERED;
                status = true;
                break;
            }
            case USB_RT_RECIPIENT_INTERFACE: {
                /* No implementation needed at this time */
                break;
            }
            case USB_RT_RECIPIENT_ENDPOINT: {
                /* No implementation needed at this time */
                /* Should specify if endpoint is halted. Implement as necessary. */
                break;
            }
            case USB_RT_RECIPIENT_OTHER: {
                /* No implementation needed at this time */
                break;
            }
            default: {
                break;
            }
        }
    } else {
       DEBUG ((DEBUG_INFO, "usbdGetStatus() - Invalid data length\n"));
    }

    return status;
}

/**
 * Sets the address of the device
 *
 * @param address   the address value to set
 *
 * @return true if address was set, false otherwise
 */
static bool usbdSetAddress(uint8_t address) {
    bool status = false;

    DEBUG ((DEBUG_INFO, "\nusbdSetAddress() - setting address: 0x%x\n", address));

    if (address <= 0x7F) { /* address must not be > 127 */
        gDrvObj.address = address;

        /* Configure Address in the DCI */
        status = udciSetAddress(gDrvObj.pDciDrvObj, gDrvObj.address);
        if (status == true) {
            gDrvObj.state = USBD_STATE_ADDRESS;
        } else {
           DEBUG ((DEBUG_INFO, "usbdSetAddress() - Failed to set address in DCI\n"));
        }
    } else {
       DEBUG ((DEBUG_INFO, "usbdSetAddress() - Invalid address: %i\n", address));
    }

    return status;
}

/**
 * Handles Setup device requests. Standard requests are immediately
 * handled here, and any Class/Vendor specific requests are forwarded
 * to the class driver
 *
 * @param pCtrlReq  Pointer to a device request
 *
 * @return true if request successfully handled, false otherwise
 */
bool usbdSetupHdlr(deviceRequest *pCtrlReq) {

    bool status = false;
    uint8_t descIndex = 0;
    deviceDescriptor *pDevDesc = 0;

    /* Initialize the IO object */
    gCtrlIoReq.ioInfo.dataLen = 0;

   DEBUG ((DEBUG_INFO, "\nusbdSetupHdlr start\n"));
  printDeviceRequest(pCtrlReq);

    /* Handle Standard Device Requests */
    if ((pCtrlReq->bmRequestType & USB_RT_TYPE_MASK) == USB_RT_TYPE_STANDARD) {
        switch (pCtrlReq->bRequest) {
            case USB_GET_DESCRIPTOR: {
                if (pCtrlReq->bmRequestType == USB_RT_TX_DIR_D_TO_H) {

                    descIndex = (pCtrlReq->wValue & 0xff); /* low byte is the index requested */

                    switch (pCtrlReq->wValue >> 8) { /* high byte contains request type */
                        case USB_DEVICE: {
                            DEBUG ((DEBUG_INFO, "\nusbdSetupHdlr() - Get Device Descriptor\n"));
                            pDevDesc = gDrvObj.pUsbdDevObj->pDeviceDesc;
                            /* copy the data to the output buffer */
                            gCtrlIoReq.ioInfo.dataLen = MIN(pCtrlReq->wLength, pDevDesc->bLength);
                            //memcpy_s(gCtrlIoReq.ioInfo.pBuf, gCtrlIoReq.ioInfo.dataLen, pDevDesc, gCtrlIoReq.ioInfo.dataLen);
                            CopyMem(gCtrlIoReq.ioInfo.pBuf,pDevDesc, gCtrlIoReq.ioInfo.dataLen);
                            break;
                        }
                        case USB_CONFIGURATION: {
                            status = usbdGetConfigDesc(gCtrlIoReq.ioInfo.pBuf, descIndex,
                                            pCtrlReq->wLength, &(gCtrlIoReq.ioInfo.dataLen));
                            break;
                        }
                        case USB_STRING: {
                            status = usbdGetStringDesc(gCtrlIoReq.ioInfo.pBuf, descIndex, pCtrlReq->wIndex,\
                                            pCtrlReq->wLength, &(gCtrlIoReq.ioInfo.dataLen));
                            break;
                        }
                        default: {
                           DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Unsupported USB_GET_DESCRIPTOR request: 0x%x\n", (pCtrlReq->wValue >> 8)));
                            break;
                        }
                    }
                }
                else {
                   DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Invalid direction for USB_GET_DESCRIPTOR request\n"));
                }
                break;
            }
            case USB_GET_CONFIGURATION: {
                if (pCtrlReq->bmRequestType == USB_RT_TX_DIR_D_TO_H) {
                   status = usbdGetConfig(gCtrlIoReq.ioInfo.pBuf, pCtrlReq->wLength, &(gCtrlIoReq.ioInfo.dataLen));
                } else {
                   DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Invalid direction for USB_GET_CONFIGURATION request\n"));
                }
                break;
            }
            case USB_SET_CONFIGURATION: {
                if (pCtrlReq->bmRequestType == USB_RT_TX_DIR_H_TO_D) {
                    status = usbdSetConfig((uint8_t)pCtrlReq->wValue);
                }
                else {
                   DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Invalid direction for USB_SET_CONFIGURATION request\n"));
                }
                break;
            }
            case USB_SET_ADDRESS: {
                if (pCtrlReq->bmRequestType == USB_RT_TX_DIR_H_TO_D) {
                    status = usbdSetAddress((uint8_t)pCtrlReq->wValue);
                }
                else {
                   DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Invalid direction for USB_SET_ADDRESS request\n"));
                }
                break;
            }
            case USB_GET_STATUS: {
                if (pCtrlReq->bmRequestType & USB_RT_TX_DIR_D_TO_H) {
                    status = usbdGetStatus(gCtrlIoReq.ioInfo.pBuf, pCtrlReq->bmRequestType, pCtrlReq->wLength, &(gCtrlIoReq.ioInfo.dataLen));
                } else {
                   DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Invalid direction for USB_GET_STATUS request\n"));
                }
                break;
            }
            default: {
               DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Unsupported Standard Request: 0x%x\n", pCtrlReq->bRequest));
                break;
            }
        }
    } else { /* This is not a Standard request, it specifies Class/Vendor handling */
        /* Forward request to class driver */
        DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Class/Vendor Request\n"));
        if (gDrvObj.pUsbdDevObj->pSetupCallback != NULL) {
            gDrvObj.pUsbdDevObj->pSetupCallback(pCtrlReq, &(gCtrlIoReq.ioInfo));
        }
    }

    DEBUG ((DEBUG_INFO, "dataLen=%x\n", gCtrlIoReq.ioInfo.dataLen));

    /* Transfer data according to request if necessary */
    if (gCtrlIoReq.ioInfo.dataLen > 0) {
        status = udciEpTxData(gDrvObj.pDciDrvObj, &gCtrlIoReq);
        if (status == false) {
           DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Failed to TX data\n"));
        }
    } else {
        /* If we are not responding with data, send control status */
        status = udciEp0TxStatus(gDrvObj.pDciDrvObj);
        if (status == false) {
           DEBUG ((DEBUG_INFO, "usbdSetupHdlr() - Failed to Tx Ep0 Status\n"));
        }
    }

    return status;
}

/**
 * Handles device reset events. Sets the device address to zero.
 *
 * @return true if able to set the address, false otherwise
 */
bool usbdResetHdlr() {
    bool status = false;

    DEBUG ((DEBUG_INFO, "\nusbdResetHdlr()\n"));

    /* reset device address to 0 */
    status = udciSetAddress(gDrvObj.pDciDrvObj, 0x0);
    if (status == false) {
       DEBUG ((DEBUG_INFO, "usbdResetHdlr() - Failed to set address in DCI\n"));
    }

    return status;
}

/**
 * Handles Connection done events. Sets the device address to zero.
 *
 * @return true if able to set the address, false otherwise
 */
bool usbdConnDoneHdlr() {
    bool status = false;

    DEBUG ((DEBUG_INFO, "\nusbdConnDoneHdlr()\n"));

    /* reset device address to 0 */
    status = udciSetAddress(gDrvObj.pDciDrvObj, 0x0);
    if (status == false) {
       DEBUG ((DEBUG_INFO, "usbdConnDoneHdlr() - Failed to set address in DCI\n"));
    }

    /* set the device state to attached/connected */
    gDrvObj.state = USBD_STATE_ATTACHED;

    return status;
}

/**
 * Handles transmit/receive completion events. Directly handles
 * control endpoint events and forwards class/vendor specific events
 * to the class drivers.
 *
 * @param   pXferInfo   Pointer to xfer structure
 *
 * @return  true if successfully handled event, false otherwise
 */
bool usbdXferDoneHdlr(usbdXferInfo *pXferInfo) {
    bool status = false;

    /*DEBUG ((DEBUG_INFO, "\nusbdXferDoneHdlr() - epNum: 0x%x, epDir: 0x%x, epType: 0x%x, len: 0x%x\n",
                pXferInfo->epNum, pXferInfo->epDir, pXferInfo->epType, pXferInfo->dataLen)); */

    /* If this is a non-control transfer complete, notify the class driver */
    if (pXferInfo->epNum > 0) {
        if (gDrvObj.pUsbdDevObj->pDataCallback != NULL) {
            gDrvObj.pUsbdDevObj->pDataCallback(pXferInfo);
        }
    }

    return status;
}

/**
 * Sets the MMIO BAR to use for the USB device controller interface
 *
 * @param mmioBar   The base address for MMIO
 *
 * @return void
 */
void usbdSetMmioBar(uint32_t mmioBar) {
    gDrvObj.mmioBar = mmioBar;
    DEBUG ((DEBUG_INFO, "DCI Controller MMIO BAR: 0x%08x\n", gDrvObj.mmioBar));
}

/**
 * Initializes the Device Controller interface
 *
 * @return true if initialization was successful, false otherwise
 */
bool usbdInitDCI() {

    bool status = false;

    if (gDrvObj.dciInitialized == false) {
        if (gDrvObj.mmioBar != 0) {

            /* Initialize device controller driver */
            DEBUG ((DEBUG_INFO, "usbdInitDCI() - Initializing Controller...\n"));

            /* Initialize the device controller interface */
            if (udciInit(gDrvObj.mmioBar, &gDrvObj.pDciDrvObj) == true) {

                /* Setup callbacks */
                if (udciRegisterCallbacks(gDrvObj.pDciDrvObj) == true) {

                    gDrvObj.dciInitialized = true;
                    status = true;

                    DEBUG ((DEBUG_INFO, "usbdInitDCI() - Controller initialization complete\n"));
                } else {
                   DEBUG ((DEBUG_INFO, "usbdInitDCI() - Failed to register UDCI callbacks\n"));
                }
            } else {
               DEBUG ((DEBUG_INFO, "usbdInitDCI() - Failed to initialize UDCI\n"));
            }
        } else {
           DEBUG ((DEBUG_INFO, "usbdInitDCI() - DCI MMIO BAR not set\n"));
        }
    } else {
       DEBUG ((DEBUG_INFO, "usbdInitDCI() - DCI already initialized\n"));
    }

    return status;
}

/**
 * Binds a USB class driver with this USB device driver core.
 * After calling this routine, the driver is ready to begin
 * USB processing.
 *
 * @param pUsbdDevObj   Pointer to a usbd device object which contains
 *                      all relevant information for the class driver device
 *
 * @return true if binding was successful, false otherwise
 */
bool usbdBind(usbdDevObj *pUsbdDevObj) {
    bool status = true;

    /* allocate Tx buffer */
    gCtrlIoReq.ioInfo.pBuf = malloc(USB_EPO_MAX_PKT_SIZE_ALL);
    if (gCtrlIoReq.ioInfo.pBuf != NULL) {
        gDrvObj.pUsbdDevObj = pUsbdDevObj;
        gDrvObj.pActiveConfigObj = NULL;
        gDrvObj.address = 0;
        gDrvObj.state = USBD_STATE_INIT;
    } else {
       DEBUG ((DEBUG_INFO, "usbdBind() - Failed to allocate IO Buffer\n"));
        status = false;
    }

    return status;
}

/**
 * Unbinds the USB class driver from this USB device driver core.
 *
 * @return true if successful, false otherwise
 */
bool usbdUnbind() {
    bool status = true;

    gDrvObj.pUsbdDevObj = NULL;
    gDrvObj.pActiveConfigObj = NULL;
    gDrvObj.address = 0;
    gDrvObj.state = USBD_STATE_OFF;

    /* release allocated buffer data */
    if (gCtrlIoReq.ioInfo.pBuf) {
        free(gCtrlIoReq.ioInfo.pBuf);
    }

    return status;
}

/**
 * Performs continual USB device event processing until a cancel
 * event occurs
 *
 * @param   timeoutMs   Connection timeout in ms. If 0, waits forever.
 * @return  true if run executed normally, false if error ocurred
 */
bool usbdRun(uint32_t timeoutMs) {

    uint8_t status = false;

	gST->ConOut->EnableCursor(gST->ConOut, FALSE);
	gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_YELLOW, EFI_BLACK));
	gST->ConOut->OutputString(gST->ConOut, L"\nDNX FASTBOOT MODE..\n");
	gST->ConOut->SetAttribute (gST->ConOut, EFI_TEXT_ATTR (EFI_WHITE, EFI_BLACK));

    /* can only run if DCI is initialized */
    if ((gDrvObj.dciInitialized == true)) {

        gRun = true; /* set the run flag to active */
        status = true;

        /* start the Event processing loop */
        DEBUG ((DEBUG_INFO, "usbdRun() - Starting event processing...\n"));
        while (1) {
            if (udciIsr(gDrvObj.pDciDrvObj) == false) {
               DEBUG ((DEBUG_INFO, "\nusbdRun() - Failed to execute event ISR\n"));
            }

            /* Check if a run cancel request exists, if so exit processing loop */
            if (gRun == false) {
                DEBUG ((DEBUG_INFO, "\nusbdRun() - processing was cancelled\n"));
                break;
            }

            /* check for timeout */
            if (timeoutMs == 0) {
              gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
            }
                delayMicro(50);
                timeoutMs--;
		}
	}

    return status;
}

/**
 * Sets a flag to stop the running device processing loop
 *
 * @return true always
 */
bool usbdStop() {

    bool status = true;

    gRun = false; /* set run flag to false to stop processing */

    return status;
}

/**
 * Attempts to connect the device to a USB host
 *
 * @return true if DCI connection was successful, false otherwise
 */
bool usbdConnect() {

    bool status = false;

    /* Attempt to connect the device to the host */
    if (udciConnect(gDrvObj.pDciDrvObj) == true) {
        status = true;
        DEBUG ((DEBUG_INFO, "usbdConnect() - Device Connected\n"));

    } else {
       DEBUG ((DEBUG_INFO, "usbdConnect() - Failed to connect device\n"));
    }

    return status;
}

/**
 * Attempts to disconnect the device from the Bus
 *
 * @return true if DCI disconnected, false otherwise
 */
bool usbdDisconnect() {

    bool status = false;

    /* Attempt to disconnect the device */
    if (udciDisconnect(gDrvObj.pDciDrvObj) == true) {
        gDrvObj.state = USBD_STATE_INIT;
        status = true;
        DEBUG ((DEBUG_INFO, "usbdDisconnect() - Device disconnected\n"));

    } else {
       DEBUG ((DEBUG_INFO, "usbdDisconnect() - Failed to disconnect device\n"));
    }

    return status;
}

/**
 * Submits a receive request to the DCI driver
 *
 * @param pIoReq    Pointer to an IO request structure
 *
 * @return true if request was successful, false otherwise
 */
bool usbdRx(usbdIoReq *pIoReq) {
    bool status = false;

    status = udciEpRxData(gDrvObj.pDciDrvObj, pIoReq);

    return status;
}

/**
 * Submits a transmit request to the DCI driver
 *
 * @param pIoReq    Pointer to an IO request structure
 *
 * @return true if request was successful, false otherwise
 */
bool usbdTx(usbdIoReq *pIoReq) {
    bool status = false;

    status = udciEpTxData(gDrvObj.pDciDrvObj, pIoReq);

    return status;
}

/**
 * Tells the DCI driver to stall the given endpoint
 *
 * @param pEpInfo   Pointer to an endpoint info structure
 *
 * @return true if request was successful, false otherwise
 */
bool usbdStall(usbdEpInfo *pEpInfo) {
    bool status = false;

    status = udciStallEp(gDrvObj.pDciDrvObj, pEpInfo);

    return status;
}




