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

 /** \file udci.c
 *
 * USB Device Controller Interface Abstraction Layer
 *
 */

//#include <string.h>
#include "udci.h"
#include "usbd.h"
#include "usb_common.h"
#include "usb_device.h"
#include "UdmWrapperLib.h"

/**
 * Initializes the DCI core
 *
 * @param mmioBar       address of MMIO BAR
 * @param ppUdciHndl    Double pointer to for DCI layer to set as an
 *                      opaque handle to the driver to be used in subsequent
 *                      interactions with the DCI layer.
 *
 * @return true if successfully initialized DCI, false otherwise
 */
bool udciInit(uint32_t mmioBar, void **ppUdciHndl) {

    bool status = false;
    struct usb_dev_config_params config_params;

   DEBUG ((DEBUG_INFO, "udciInit start\n"));



    config_params.id = USB_ID_DWC_XDCI;
    config_params.base_address = mmioBar;
    config_params.role = USB_ROLE_DEVICE;
    config_params.speed = USB_SPEED_SUPER;

    if (usb_device_init(&config_params, ppUdciHndl) == USB_OK) {
        status = true;
    }

   DEBUG ((DEBUG_INFO, "udciInit status is %x\n", status));

    return status;
}

/**
 * De-initializes the DCI core
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciDeinit(void *pUdciHndl, uint32_t flags) {
    bool status = false;

    if (usb_device_deinit(pUdciHndl, flags) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Makes a call the the DCI ISR to process USB events
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciIsr(void *pUdciHndl) {
    bool status = false;

    if (usb_device_isr_routine(pUdciHndl) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Attempts to perform a connect on the bus
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciConnect(void *pUdciHndl) {
    bool status = false;

    if (usb_device_connect(pUdciHndl) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Disconnects the device from the bus
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciDisconnect(void *pUdciHndl) {
    bool status = false;

    if (usb_device_disconnect(pUdciHndl) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Sets the device address
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param address   Address to set in the DCI for this device
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciSetAddress(void *pUdciHndl, uint8_t address) {
    bool status = false;

    if (usb_device_set_address(pUdciHndl, address) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Copies relevant endpoint data from standard USB endpoint descriptors
 * to the usb_ep_info structure used by the DCI
 *
 * @param pEpDest   destination structure
 * @param pEpSrc    source structure
 *
 * @return void
 */
static void udciSetEpInfo(struct usb_ep_info *pEpDest, usbdEpInfo *pEpSrc) {

    endpointDescriptor *pEpDesc = NULL;
    endpointCompanionDescriptor *pEpCompDesc = NULL;

    /* start by clearing all data in the destination */
    memset(pEpDest, 0, sizeof(struct usb_ep_info));

    pEpDesc = pEpSrc->pEpDesc;
    pEpCompDesc = pEpSrc->pEpCompDesc;

    if (pEpDesc != NULL) {
        pEpDest->ep_num = pEpDesc->bEndpointAddress & 0x0F; /* Bits 0-3 are ep num */
        pEpDest->ep_dir = ((pEpDesc->bEndpointAddress & USB_DIR_IN) > 0)
                            ? USB_EP_DIR_IN : USB_EP_DIR_OUT;
        pEpDest->ep_type = pEpDesc->bmAttributes & USB_EP_XFER_MASK;
        pEpDest->max_pkt_size = pEpDesc->wMaxPacketSize;
        pEpDest->interval = pEpDesc->bInterval;
    }
    if (pEpCompDesc != NULL) {
        pEpDest->max_streams = pEpCompDesc->bmAttributes & USB_EP_BULK_BM_ATTR_MASK;
        pEpDest->burst_size = pEpCompDesc->bMaxBurst;
        pEpDest->mult = pEpCompDesc->wBytesPerInterval;
    }
}

/**
 * Initializes the given endpoint
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pEpInfo   Pointer to endpoint info structure
 *                  for the endpoint to initialize
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciInitEp(void *pUdciHndl, usbdEpInfo *pEpInfo) {
    bool status = false;
    struct usb_ep_info epInfo;

    udciSetEpInfo(&epInfo, pEpInfo);

    if (usb_device_init_ep(pUdciHndl, &epInfo) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Enables the given endpoing
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pEpInfo   Pointer to endpoint info structure
 *                  for the endpoint to initialize
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciEnableEp(void *pUdciHndl, usbdEpInfo *pEpInfo) {
    bool status = false;
    struct usb_ep_info epInfo;

    udciSetEpInfo(&epInfo, pEpInfo);

    if (usb_device_ep_enable(pUdciHndl, &epInfo) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Disables the given endpoint
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pEpInfo   Pointer to endpoint info structure
 *                  for the endpoint to initialize
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciDisableEp(void *pUdciHndl, usbdEpInfo *pEpInfo) {
    bool status = false;
    struct usb_ep_info epInfo;

    udciSetEpInfo(&epInfo, pEpInfo);

    if (usb_device_ep_disable(pUdciHndl, &epInfo) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Stalls the given endpoint
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pEpInfo   Pointer to endpoint info structure
 *                  for the endpoint to initialize
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciStallEp(void *pUdciHndl, usbdEpInfo *pEpInfo) {
    bool status = false;
    struct usb_ep_info epInfo;

    udciSetEpInfo(&epInfo, pEpInfo);

    if (usb_device_ep_stall(pUdciHndl, &epInfo) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Clears the stall condition for the given endpoint
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pEpInfo   Pointer to endpoint info structure
 *                  for the endpoint to initialize
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciClearStallEp(void *pUdciHndl, usbdEpInfo *pEpInfo) {
    bool status = false;
    struct usb_ep_info epInfo;

    udciSetEpInfo(&epInfo, pEpInfo);

    if (usb_device_ep_clear_stall(pUdciHndl, &epInfo) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Transmits control endpoint status
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciEp0TxStatus(void *pUdciHndl) {
    bool status = false;

    if (usb_device_ep0_tx_status(pUdciHndl) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Callback handler used when transfer operations complete. Calls
 * upper layer routine to handle the operation.
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pXferReq  Pointer to the transfer request structure
 *
 * @return void
 */
static void udciXferDoneHndlr(void *pUdciHndl, struct usb_xfer_request *pXferReq) {

    usbdXferInfo xferInfo;

    xferInfo.epNum = (uint8_t)pXferReq->ep_info.ep_num;
    xferInfo.epDir = pXferReq->ep_info.ep_dir;
    xferInfo.epType = pXferReq->ep_info.ep_type;
    xferInfo.pBuf = pXferReq->xfer_buffer;
    xferInfo.dataLen = pXferReq->actual_xfer_len;

    usbdXferDoneHdlr(&xferInfo);
}

/**
 * Queue a request to transmit data
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pIoReq    Pointer to IO structure containing details of the
 *                  transfer request
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciEpTxData(void *pUdciHndl, usbdIoReq *pIoReq) {
    bool status = false;
    struct usb_xfer_request gTxReq;

    /* set endpoint data */
    udciSetEpInfo(&(gTxReq.ep_info), &(pIoReq->epInfo)); /* set endpoint data */

    /* if this is a control endpoint, set the number and direction */
    if (pIoReq->epInfo.pEpDesc == NULL) {
        gTxReq.ep_info.ep_num = 0;
        gTxReq.ep_info.ep_dir = USB_EP_DIR_IN;
    }

    /* setup the trasfer request */
    gTxReq.xfer_buffer = pIoReq->ioInfo.pBuf;
    gTxReq.xfer_len = pIoReq->ioInfo.dataLen;
    gTxReq.xfer_done = udciXferDoneHndlr;

    DEBUG ((DEBUG_INFO,  "\nTX REQUEST: epNum: 0x%x, epDir: 0x%x, epType: 0x%x\n",\
            gTxReq.ep_info.ep_num, gTxReq.ep_info.ep_dir, gTxReq.ep_info.ep_type));

    if (usb_device_ep_tx_data(pUdciHndl, &gTxReq) == USB_OK) {
        status = true;
    }

    return status;
}

/**
 * Queue a request to receive data
 *
 * @param pUdciHndl Pointer (handle) to the DCI driver object
 * @param pIoReq    Pointer to IO structure containing details of the
 *                  receive request
 *
 * @return true if operation succeeded, false otherwise
 */
bool udciEpRxData(void *pUdciHndl, usbdIoReq *pIoReq) {
    bool status = false;
    struct usb_xfer_request gRxReq;
    uint32_t ReqPacket;

    if (pIoReq->epInfo.pEpDesc->wMaxPacketSize == 0) {
      return false;
    }
    /* set endpoint data */
    udciSetEpInfo(&(gRxReq.ep_info), &(pIoReq->epInfo));

    /* setup the trasfer request */
    gRxReq.xfer_buffer = pIoReq->ioInfo.pBuf;

    //
    // Transfer length should be multiple of USB packet size.
    //
    ReqPacket = pIoReq->ioInfo.dataLen / pIoReq->epInfo.pEpDesc->wMaxPacketSize;
    ReqPacket = ((pIoReq->ioInfo.dataLen % pIoReq->epInfo.pEpDesc->wMaxPacketSize) == 0)? ReqPacket : ReqPacket + 1;
    gRxReq.xfer_len = ReqPacket * pIoReq->epInfo.pEpDesc->wMaxPacketSize;

    gRxReq.xfer_done = udciXferDoneHndlr;

    DEBUG ((DEBUG_INFO,  "\nRX REQUEST: epNum: 0x%x, epDir: 0x%x, epType: 0x%x\n",\
            gRxReq.ep_info.ep_num, gRxReq.ep_info.ep_dir, gRxReq.ep_info.ep_type));

    if (usb_device_ep_rx_data(pUdciHndl, &gRxReq) == USB_OK) {
        status = true;
    }

    return status;
}


/**
 * Callback used to handle Reset events from the DCI
 *
 * @param pParm Pointer to a generic callback parameter structure
 *
 * @return DCI usb status
 */
static enum usb_status udciResetEvtHndlr(struct usb_device_callback_param *pParm) {
    enum usb_status status = USB_OK;

    if (usbdResetHdlr() == false) {
        status = USB_OP_FAILED;
    }

    return status;
}

/**
 * Callback used to handle Connection done events from the DCI
 *
 * @param pParm Pointer to a generic callback parameter structure
 *
 * @return DCI usb status
 */
static enum usb_status udciConnDoneEvtHndlr(struct usb_device_callback_param *pParm) {
    enum usb_status status = USB_OK;

    if (usbdConnDoneHdlr() == false) {
        status = USB_OP_FAILED;
    }

    return status;
}

/**
 * Callback used to handle Control Endpoint Setup events from the DCI
 *
 * @param pParm Pointer to a generic callback parameter structure
 *
 * @return DCI usb status
 */
static enum usb_status udciSetupEvtHndlr(struct usb_device_callback_param *pParm) {

    enum usb_status status = USB_OK;
    deviceRequest req;

    /* Fill out request object from the incomming buffer */
    //memcpy_s(&req, sizeof(deviceRequest), pParm->buffer, sizeof(deviceRequest));
    CopyMem(&req, pParm->buffer, sizeof(deviceRequest));

    

    if (usbdSetupHdlr(&req) == false) {
        status = USB_OP_FAILED;
        DEBUG ((DEBUG_INFO, "\nUSB_OP_FAILED\n"));        
    }

    return status;
}

/**
 * Registers callbacks for event handlers with the DCI layer.
 * The functions will be called as the registered events are triggered.
 *
 * @param  Handle to UDCI core driver
 * @return true if successful, false otherwise
 */
bool udciRegisterCallbacks(void *pUdciHndl) {

    if (usb_device_register_callback(pUdciHndl, USB_DEVICE_RESET_EVENT, udciResetEvtHndlr) != USB_OK) {
        goto udciRegCallbackError;
    }

    if (usb_device_register_callback(pUdciHndl, USB_DEVICE_CONNECTION_DONE, udciConnDoneEvtHndlr) != USB_OK) {
        goto udciRegCallbackError;
    }

    if (usb_device_register_callback(pUdciHndl, USB_DEVICE_SETUP_PKT_RECEIVED, udciSetupEvtHndlr) != USB_OK) {
        goto udciRegCallbackError;
    }

    return true;

udciRegCallbackError:
    return false;
}



