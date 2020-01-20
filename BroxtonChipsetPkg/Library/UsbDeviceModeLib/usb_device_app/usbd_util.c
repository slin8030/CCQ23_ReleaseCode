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

 /** \file usbd_util.c
 *
 * USB Device driver utility implementation
 *
 */

#include "usbd_util.h"

void printDeviceDescriptor(deviceDescriptor *pDevDesc) {
    DEBUG ((DEBUG_INFO, "\n --- Device Descriptor ---\n"));
    DEBUG ((DEBUG_INFO, "\n bLenght : 0x%x", pDevDesc->bLength));
    DEBUG ((DEBUG_INFO, "\n bDescriptor type : 0x%x", pDevDesc->bDescriptorType));
    DEBUG ((DEBUG_INFO, "\n bcdUSB : 0x%x", pDevDesc->bcdUSB));
    DEBUG ((DEBUG_INFO, "\n bDeviceClass : 0x%x", pDevDesc->bDeviceClass));
    DEBUG ((DEBUG_INFO, "\n bDeviceSubClass : 0x%x", pDevDesc->bDeviceSubClass));
    DEBUG ((DEBUG_INFO, "\n bDeviceProtocol : 0x%x", pDevDesc->bDeviceProtocol));
    DEBUG ((DEBUG_INFO, "\n bMaxPacketSize0 : 0x%x", pDevDesc->bMaxPacketSize0));
    DEBUG ((DEBUG_INFO, "\n idVendor : 0x%x", pDevDesc->idVendor));
    DEBUG ((DEBUG_INFO, "\n idProduct : 0x%x", pDevDesc->idProduct));
    DEBUG ((DEBUG_INFO, "\n bcdDevice : 0x%x", pDevDesc->bcdDevice));
    DEBUG ((DEBUG_INFO, "\n iManufacturer : 0x%x", pDevDesc->iManufacturer));
    DEBUG ((DEBUG_INFO, "\n iProduct : 0x%x", pDevDesc->iProduct));
    DEBUG ((DEBUG_INFO, "\n iSerial Number : 0x%x", pDevDesc->iSerialNumber));
    DEBUG ((DEBUG_INFO, "\n bNumConfigurations : 0x%x", pDevDesc->bNumConfigurations));
    DEBUG ((DEBUG_INFO, "\n\n"));
}

void printConfigDescriptor(configurationDescriptor *pConfigDesc) {
    DEBUG ((DEBUG_INFO, "\n --- Configuration Descriptor ---\n"));
    DEBUG ((DEBUG_INFO, "\n bLenght : 0x%x", pConfigDesc->bLength));
    DEBUG ((DEBUG_INFO, "\n bDescriptor type : 0x%x", pConfigDesc->bDescriptorType));
    DEBUG ((DEBUG_INFO, "\n wTotalLength : 0x%x", pConfigDesc->wTotalLength));
    DEBUG ((DEBUG_INFO, "\n bNumInterfaces : 0x%x", pConfigDesc->bNumInterfaces));
    DEBUG ((DEBUG_INFO, "\n bConfigurationValue : 0x%x", pConfigDesc->bConfigurationValue));
    DEBUG ((DEBUG_INFO, "\n iConfiguration : 0x%x", pConfigDesc->iConfiguration));
    DEBUG ((DEBUG_INFO, "\n bmAttributes : 0x%x", pConfigDesc->bmAttributes));
    DEBUG ((DEBUG_INFO, "\n bMaxPower : 0x%x", pConfigDesc->bMaxPower));
    DEBUG ((DEBUG_INFO, "\n\n"));
}

void printInterfaceDescriptor(interfaceDescriptor *pIfDesc) {
    DEBUG ((DEBUG_INFO, "\n --- Interface Descriptor ---\n"));
    DEBUG ((DEBUG_INFO, "\n bLenght : 0x%x", pIfDesc->bLength));
    DEBUG ((DEBUG_INFO, "\n bDescriptor type : 0x%x", pIfDesc->bDescriptorType));
    DEBUG ((DEBUG_INFO, "\n bInterfaceNumber : 0x%x", pIfDesc->bInterfaceNumber));
    DEBUG ((DEBUG_INFO, "\n bAlternateSetting : 0x%x", pIfDesc->bAlternateSetting));
    DEBUG ((DEBUG_INFO, "\n bNumEndpoints : 0x%x", pIfDesc->bNumEndpoints));
    DEBUG ((DEBUG_INFO, "\n bInterfaceClass : 0x%x", pIfDesc->bInterfaceClass));
    DEBUG ((DEBUG_INFO, "\n binterfaceSubClass : 0x%x", pIfDesc->bInterfaceSubClass));
    DEBUG ((DEBUG_INFO, "\n bInterfaceProtocol : 0x%x", pIfDesc->bInterfaceProtocol));
    DEBUG ((DEBUG_INFO, "\n iInterface : 0x%x", pIfDesc->iInterface));
    DEBUG ((DEBUG_INFO, "\n\n"));
}

void printEpDescriptor(endpointDescriptor *pEpDesc) {
    DEBUG ((DEBUG_INFO, "\n --- Endpoint Descriptor ---\n"));
    DEBUG ((DEBUG_INFO, "\n bLenght : 0x%x", pEpDesc->bLength));
    DEBUG ((DEBUG_INFO, "\n bDescriptor type : 0x%x", pEpDesc->bDescriptorType));
    DEBUG ((DEBUG_INFO, "\n bEndpointAddress : 0x%x", pEpDesc->bEndpointAddress));
    DEBUG ((DEBUG_INFO, "\n bmAttributes : 0x%x", pEpDesc->bmAttributes));
    DEBUG ((DEBUG_INFO, "\n wMaxPacketSize : 0x%x", pEpDesc->wMaxPacketSize));
    DEBUG ((DEBUG_INFO, "\n bInterval : 0x%x", pEpDesc->bInterval));
    DEBUG ((DEBUG_INFO, "\n\n"));
}

void printEpCompDescriptor(endpointCompanionDescriptor *pEpDesc) {
    DEBUG ((DEBUG_INFO, "\n --- Endpoint Companion Descriptor ---\n"));
    DEBUG ((DEBUG_INFO, "\n bLenght : 0x%x", pEpDesc->bLength));
    DEBUG ((DEBUG_INFO, "\n bDescriptor type : 0x%x", pEpDesc->bDescriptorType));
    DEBUG ((DEBUG_INFO, "\n bMaxBurst : 0x%x", pEpDesc->bMaxBurst));
    DEBUG ((DEBUG_INFO, "\n bmAttributes : 0x%x", pEpDesc->bmAttributes));
    DEBUG ((DEBUG_INFO, "\n wBytesPerInterval : 0x%x", pEpDesc->wBytesPerInterval));
    DEBUG ((DEBUG_INFO, "\n\n"));
}

void printStringDescriptor(stringDescriptor *pStrDesc) {
    uint16_t itr = 0;
    uint16_t strLen = 0;

    if (pStrDesc->bLength > 2) {
        strLen = ((pStrDesc->bLength - 2) >> 1);

        DEBUG ((DEBUG_INFO, "\n --- String Descriptor ---"));
        DEBUG ((DEBUG_INFO, "\n bLenght : 0x%x", pStrDesc->bLength));
        DEBUG ((DEBUG_INFO, "\n bDescriptor type : 0x%x", pStrDesc->bDescriptorType));

        for (itr = 0; itr < strLen; itr++) {
            DEBUG ((DEBUG_INFO, "\n wStr[%d]: 0x%04x", itr, pStrDesc->wLangID[itr]));
        }
    }

    DEBUG ((DEBUG_INFO, "\n\n"));
}

void printDeviceRequest(deviceRequest *pDevReq) {
    DEBUG ((DEBUG_INFO, "\n --- Device Request ---"));
    DEBUG ((DEBUG_INFO, "\n bmRequestType : 0x%x", pDevReq->bmRequestType));
    DEBUG ((DEBUG_INFO, "\n bRequest : 0x%x", pDevReq->bRequest));
    DEBUG ((DEBUG_INFO, "\n wValue : 0x%x", pDevReq->wValue));
    DEBUG ((DEBUG_INFO, "\n wIndex : 0x%x", pDevReq->wIndex));
    DEBUG ((DEBUG_INFO, "\n wLength : 0x%x", pDevReq->wLength));
    DEBUG ((DEBUG_INFO, "\n\n"));
}

