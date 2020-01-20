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

//#include <stdbool.h>
//#include <string.h>

#ifndef _FASTBOOT_H
#define _FASTBOOT_H

#define wchar_t CHAR16 

/* structure used to configure settings in the fastboot application */
typedef struct {
    uint16_t devProductId; /* Product ID value for standard USB Device Descriptor */
    uint16_t devBcd; /* BCD value for standard USB Device Descriptor */
    wchar_t  *pDevProductStr; /* Pointer to NULL terminated wide char string */
    wchar_t  *pDevSerialNumStr; /* Pointer to NULL terminated wide char string */
    uint32_t kernelLoadAddr; /* kernel base load address, not including any fastboot header */
    uint32_t kernelEntryOffset; /* offset from load address */
    uint32_t timeoutMs; /* Timeout in ms for connecting to host. If 0, waits forever */
} fbParams;

/* public routines */
bool fbInit(fbParams *pParams);
bool fbDeinit();
bool fbStart();
bool fbStop();

#endif
