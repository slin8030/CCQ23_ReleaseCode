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
#ifndef _USB_H_
#define _USB_H_

#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
//#include <stdint.h>

#define true TRUE
#define false FALSE

#define ENDPOINT0 0
#define ADDRESS0 0
#define CONFIG0 0

#define MAX_DESCRIPTOR_SIZE         64
#define STRING_ARR_SIZE ((MAX_DESCRIPTOR_SIZE - 2) >> 1)
#define USB_ADDRESS_TABLE_SIZE      16  /*4 */

/* Endpoint Zero */
#define USB_EP0_MAX_PKT_SIZE_HS     0x40 /* High Speed mode is explicitly set as 64 bytes */
#define USB_EP0_MAX_PKT_SIZE_SS     0x9 /* Must be 0x9 (2^9 = 512 Bytes) in SuperSpeed mode */
#define USB_EPO_MAX_PKT_SIZE_ALL    512 /* Overall max bytes for any type */

/* Bulk Endpoints */
#define USB_BULK_EP_PKT_SIZE_HS     0x200 /* Bulk-Endpoint HighSpeed */
#define USB_BULK_EP_PKT_SIZE_SS     0x400 /* Bulk-Endpoint SuperSpeed */
#define USB_BULK_EP_PKT_SIZE_MAX    USB_BULK_EP_PKT_SIZE_SS


/* EFI_USB_PORT_STATUS.PortChangeStatus bit definition */
#define USB_PORT_STAT_C_CONNECTION  0x0001
#define USB_PORT_STAT_C_ENABLE      0x0002
#define USB_PORT_STAT_C_SUSPEND     0x0004
#define USB_PORT_STAT_C_OVERCURRENT 0x0008
#define USB_PORT_STAT_C_RESET       0x00

/* EFI_USB_PORT_STATUS.PortStatus bit definition */
#define USB_PORT_STAT_CONNECTION    0x0001
#define USB_PORT_STAT_HIGH_SPEED    0x0400

/* Transmit Direction Bits */
#define USB_DIR_OUT                 0x00
#define USB_DIR_IN                  0x80

/* Endpoint Transfer types */
#define USB_EP_XFER_CONTROL         0x00
#define USB_EP_XFER_ISOC            0x01
#define USB_EP_XFER_BULK            0x02
#define USB_EP_XFER_INT             0x03
#define USB_EP_XFER_MASK            0x03

/* Endpoint Companion Bulk Attributes */
#define USB_EP_BULK_BM_ATTR_MASK    0x1F

/* Configuration Modifiers (bmAttributes) */
#define USB_BM_ATTR_RESERVED        0x80
#define USB_BM_ATTR_SELF_POWERED    0x40
#define USB_BM_ATTR_REMOTE_WAKE     0X20


/* Standard Request Codes */
typedef enum {
    USB_GET_STATUS          = 0,
    USB_CLEAR_FEATURE       = 1,
    USB_SET_FEATURE         = 3,
    USB_SET_ADDRESS         = 5,
    USB_GET_DESCRIPTOR      = 6,
    USB_SET_DESCRIPTOR      = 7,
    USB_GET_CONFIGURATION   = 8,
    USB_SET_CONFIGURATION   = 9,
    USB_GET_INTERFACE       = 10,
    USB_SET_INTERFACE       = 11,
    USB_GET_SYNC_FRAME      = 12
} StandardRequestCodes;

/* Descriptor Types */
typedef enum {
    USB_DEVICE                  = 1,
    USB_CONFIGURATION           = 2,
    USB_STRING                  = 3,
    USB_INTERFACE               = 4,
    USB_ENDPOINT                = 5,
    USB_DEVICE_QUALIFIER        = 6,
    USB_OTHER_SPEED_CONFIG      = 7,
    USB_INTERFACE_POWER         = 8,
    USB_OTG                     = 9,
    USB_DEBUG                   = 10,
    USB_INTERFACE_ASSOCIATION   = 11,
    USB_BOS                     = 15,
    USB_DEVICE_CAPABILITY       = 16,
    USB_SS_ENDPOINT_COMPANION   = 48,
    USB_HUB                     = 0x29
} DescriptorTypes;

/* Standard Device Requests */
typedef enum {
    USB_HOST_TO_DEVICE            = 0x00,
    USB_HOST_TO_INTERFACE         = 0x01,
    USB_HOST_TO_ENDPOINT          = 0x02,
    USB_HOST_TO_OTHER             = 0x03,
    USB_HOST_TO_CLASS_DEVICE      = 0x20,
    USB_HOST_TO_CLASS_INTERFACE   = 0x21,
    USB_HOST_TO_CLASS_ENDPOINT    = 0x22,
    USB_HOST_TO_CLASS_OTHER       = 0x23,
    USB_HOST_TO_VENDOR_DEVICE     = 0x40,
    USB_HOST_TO_VENDOR_INTERFACE  = 0x41,
    USB_HOST_TO_VENDOR_ENDPOINT   = 0x42,
    USB_HOST_TO_VENDOR_OTHER      = 0x43,
    USB_DEVICE_TO_HOST            = 0x80,
    USB_INTERFACE_TO_HOST         = 0x81,
    USB_ENDPOINT_TO_HOST          = 0x82,
    USB_OTHER_TO_HOST             = 0x83,
    USB_CLASS_DEVICE_TO_HOST      = 0xA0,
    USB_CLASS_INTERFACE_TO_HOST   = 0xA1,
    USB_CLASS_ENDPOINT_TO_HOST    = 0xA2,
    USB_CLASS_OTHER_TO_HOST       = 0xA3,
    USB_VENDOR_DEVICE_TO_HOST     = 0xC0,
    USB_VENDOR_INTERFACE_TO_HOST  = 0xC1,
    USB_VENDOR_ENDPOINT_TO_HOST   = 0xC2,
    USB_VENDOR_OTHER_TO_HOST      = 0xC3
} UsbDeviceRequests;

/* Standard Feature Selectors */
typedef enum {
    USB_END_POINT_HALT         = 0,
    USB_DEVICE_REMOTE_WAKEUP   = 1,
    USB_TEST_MODE              = 2
} UsbFeatureSelectors;


/* Standard Device Requests */
typedef enum {
    USB_PORT_ENABLE            = 1,
    USB_PORT_SUSPEND           = 2,
    USB_PORT_RESET             = 4,
    USB_PORT_POWER             = 8,
    USB_PORT_OWNER             = 13,
    USB_PORT_CONNECT_CHANGE    = 16,
    USB_PORT_ENABLE_CHANGE     = 17,
    USB_PORT_SUSPEND_CHANGE    = 18,
    USB_PORT_OVERCURRENT_CHANGE= 19,
    USB_PORT_RESETCHANGE       = 20
} UsbPortFeature;

typedef enum {
    USB_BCD_VERSION_LS  = 0x0110,
    USB_BCD_VERSION_HS  = 0x0200,
    USB_BCD_VERSION_SS  = 0x0300
} UsbBcdVersion;

typedef UINT8 uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef BOOLEAN bool;
#define inline __inline

#define pmalloc AllocatePool
#define free FreePool

/* Descriptor Header */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
} usbDescriptorHdr;
#pragma pack()

/* Device Request */
#pragma pack(1)
typedef struct {
    uint8_t  bmRequestType;
    uint8_t  bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} deviceRequest;
#pragma pack()

/* Device descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t bcdUSB;
    uint8_t  bDeviceClass;
    uint8_t  bDeviceSubClass;
    uint8_t  bDeviceProtocol;
    uint8_t  bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t  iManufacturer;
    uint8_t  iProduct;
    uint8_t  iSerialNumber;
    uint8_t  bNumConfigurations;
} deviceDescriptor;
#pragma pack()

/* Configuration Descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} configurationDescriptor;
#pragma pack()

/* Endpoint descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bEndpointAddress;
    uint8_t  bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t  bInterval;
} endpointDescriptor;
#pragma pack()

/* SuperSpeed Endpoint companion descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bMaxBurst;
    uint8_t  bmAttributes;
    uint16_t wBytesPerInterval;
} endpointCompanionDescriptor;

/* Interface descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
} interfaceDescriptor;
#pragma pack()

/* Optional string descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wLangID[STRING_ARR_SIZE];
} stringDescriptor;
#pragma pack()

/* Other speed configuration descriptor */
#pragma pack(1)
typedef struct {
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;
} otherSpeedDescriptor;
#pragma pack()


#endif
