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

 /** \file usbd_util.h
 *
 * USB Device driver utility header
 *
 */

#ifndef _USBD_UTIL_H_
#define _USBD_UTIL_H_

#include "usbd.h"

void printDeviceDescriptor(deviceDescriptor *pDevDesc);
void printConfigDescriptor(configurationDescriptor *pConfigDesc);
void printInterfaceDescriptor(interfaceDescriptor *pIfDesc);
void printEpDescriptor(endpointDescriptor *pEpDesc);
void printEpCompDescriptor(endpointCompanionDescriptor *pEpDesc);
void printStringDescriptor(stringDescriptor *pStrDesc);
void printDeviceRequest(deviceRequest *pDevReq);

#endif

