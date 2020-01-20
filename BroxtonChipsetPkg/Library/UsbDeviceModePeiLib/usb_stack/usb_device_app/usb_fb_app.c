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
 /**
  * @file usb_fb_app.c
  *
  * USB Fastboot Application
  */

#include "usbd.h" /* for debug print access only */
#include "usb_fb.h"

/* Fastboot configurable device strings. Must be NULL terminated */
static wchar_t gStrProduct[] = {L'B', L'a', L'y', L't', L'r', L'a', L'i', L'l', L'\0'};
static wchar_t gStrSerialNumber[] = {L'I', L'N', L'T', L'1', L'2', L'3', L'4', L'5', L'6', L'\0'};

/**
 * USB Fastboot Application
 *
 * @return true if execution completed without error, false otherwise.
 */
bool usbFb() {

    bool status = false;
    fbParams params;

    /* setup Fastboot configurable parameters */
    params.devProductId = 0x1234;
    params.devBcd = 0x0100;
    params.pDevProductStr = gStrProduct;
    params.pDevSerialNumStr = gStrSerialNumber;
    params.kernelLoadAddr = 0x01100000;
    params.kernelEntryOffset = 0x1000;
    params.timeoutMs = 0;

    /* initialize and start processing device data */
    if (fbInit(&params) == true) {
        if (fbStart() == true) {
            DEBUG ((DEBUG_INFO, "usbFastboot - Completed\n"));
            status = true;
        } else {
          DEBUG ((DEBUG_INFO,"usbFastboot - Failed to start\n"));
        } 
    } else {
        DEBUG ((DEBUG_INFO, "usbFastboot - Failed to initialize fastboot driver\n"));
    }

    /* we are done with the receive process, stop the device and disconnect */
    if (fbStop() == true) {
        if (fbDeinit() == true) {
            DEBUG ((DEBUG_INFO, "usbFastboot - Device stopped and disconnected\n"));
        } else {
            DEBUG ((DEBUG_INFO, "usbFastboot - Failed to de-initialize the device\n"));
        }
    } else {
        DEBUG ((DEBUG_INFO, "usbFastboot - Failed to stop device\n"));
    }

    return status;
}

