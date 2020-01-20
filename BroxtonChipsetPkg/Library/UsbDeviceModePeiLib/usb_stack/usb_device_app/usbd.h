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

 /** \file usbd.h
 *
 * USB Device driver header
 *
 */

#ifndef _USBD_H_
#define _USBD_H_

#include "usb.h"
//#include <stdio.h>
//#include <stdbool.h>

#define USBD_DEBUG_ERR  0
#define USBD_DEBUG_INFO 0
#if USBD_DEBUG_ERR
    #define usbdDbgErr printf
#else
    #define usbdDbgErr(...)
#endif

/*++
#if USBD_DEBUG_INFO
    #define usbdDbgInfo printf
#else
    #define usbdDbgInfo(...)
#endif
--*/


/* Device RequestType Flags */
#define USB_RT_TX_DIR_H_TO_D        (0x0)       /* Tx direction Host to Device */
#define USB_RT_TX_DIR_D_TO_H        (0x1 << 7)  /* Tx direction Device to Host */
#define USB_RT_TX_DIR_MASK          (0X80)

#define USB_RT_TYPE_STANDARD        (0x00)
#define USB_RT_TYPE_CLASS           (0x01 << 5)
#define USB_RT_TYPE_VENDOR          (0x02 << 5)
#define USB_RT_TYPE_RESERVED        (0x03 << 5)
#define USB_RT_TYPE_MASK            (0x60)

#define USB_RT_RECIPIENT_DEVICE     (0x00)
#define USB_RT_RECIPIENT_INTERFACE  (0x01)
#define USB_RT_RECIPIENT_ENDPOINT   (0x02)
#define USB_RT_RECIPIENT_OTHER      (0x03)
#define USB_RT_RECIPIENT_MASK       (0x1F)

/* Device GetStatus bits */
#define USB_STATUS_SELFPOWERED      (0x01)
#define USB_STATUS_REMOTEWAKEUP     (0x02)

/* USB Device class identifiers */
#define USB_DEVICE_MS_CLASS         (0x08)
#define USB_DEVICE_VENDOR_CLASS     (0xFF)


/* USB device states from USB spec sec 9.1*/
typedef enum {
    USBD_STATE_OFF = 0,
    USBD_STATE_INIT,
    USBD_STATE_ATTACHED,
    USBD_STATE_POWERED,
    USBD_STATE_DEFAULT,
    USBD_STATE_ADDRESS,
    USBD_STATE_CONFIGURED,
    USBD_STATE_SUSPENDED,
    USBD_STATE_ERROR
} usbdState;

/*
 * The following set of structs are used during USB data transaction
 * operatitions, including requests and completion events.
 */
typedef struct {
    endpointDescriptor          *pEpDesc;
    endpointCompanionDescriptor *pEpCompDesc;
} usbdEpInfo;

typedef struct {
    void        *pBuf;
    uint32_t    dataLen;
} usbdIoInfo;

typedef struct {
    usbdIoInfo  ioInfo;
    usbdEpInfo  epInfo;
} usbdIoReq;

typedef struct {
    uint32_t     epNum;
    uint8_t     epDir;
    uint8_t     epType;
    uint32_t    dataLen;
    void        *pBuf;
} usbdXferInfo;


/*
 * The following structures abstract the device descriptors a class
 * driver needs to provide to the USBD core.
 * These structures are filled & owned by the class/function layer.
 */
typedef struct {
    endpointDescriptor          *pEndpointDesc;
    endpointCompanionDescriptor *pEndpointCompDesc;
} usbdEndpointObj;

typedef struct {
    interfaceDescriptor         *pInterfaceDesc;
    usbdEndpointObj             *pEndpointObjs;
} usbdInterfaceObj;

typedef struct {
    configurationDescriptor     *pConfigDesc;
    void                        *pConfigAll;
    usbdInterfaceObj            *pInterfaceObjs;
} usbdConfigObj;

typedef struct {
    deviceDescriptor            *pDeviceDesc;
    usbdConfigObj               *pConfigObjs;
    stringDescriptor            *pStringTable;
    uint8_t                     strTblEntries;
    bool                        (*pConfigCallback)(uint8_t cfgVal);
    bool                        (*pSetupCallback)(deviceRequest *pCtrlReq, usbdIoInfo *pIo);
    bool                        (*pDataCallback)(usbdXferInfo *pXferInfo);
} usbdDevObj;


/*
 * Main USBD driver object structure containing all data necessary
 * for USB device mode processing at this layer
 */
typedef struct {
    usbdDevObj                  *pUsbdDevObj; /* pointer to a Device Object */
    void                        *pDciDrvObj; /* Opaque handle to DCI driver */
    uint32_t                    mmioBar; /* MMIO BAR */
    bool                        dciInitialized; /* flag to specify if the DCI driver is initialized */
    usbdConfigObj               *pActiveConfigObj; /* pointer to currently active configuraiton */
    usbdState                   state; /* current state of the USB Device state machine */
    uint8_t                     address; /* configured device address */
} usbdDrvObj;


/*
 * declaration of public routines
 */

void usbdSetMmioBar(uint32_t mmioBar);
bool usbdInitDCI();
bool usbdBind(usbdDevObj *pUsbdDevObj);
bool usbdUnbind();
bool usbdConnect();
bool usbdDisconnect();
bool usbdRun(uint32_t timeoutMs);
bool usbdStop();
bool usbdRx(usbdIoReq *pIoReq);
bool usbdTx(usbdIoReq *pIoReq);
bool usbdStall(usbdEpInfo *pEpInfo);

bool usbdSetupHdlr(deviceRequest *pCtrlReq);
bool usbdResetHdlr();
bool usbdConnDoneHdlr();
bool usbdXferDoneHdlr(usbdXferInfo *pXferInfo);

#endif
