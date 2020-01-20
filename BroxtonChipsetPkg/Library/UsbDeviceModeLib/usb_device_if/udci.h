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

 /** \file udci.h
 *
 * USB Device Controller Interface Abstraction Layer
 *
 */

#ifndef _UDCI_H_
#define _UDCI_H_

//#include <stdbool.h>
//#include <stdint.h>
#include <Uefi.h>
#include "usb.h"
#include "usbd.h"


bool udciInit(uint32_t mmioBar, void **ppUdciHndl);
bool udciDeinit(void *pUdciHndl, uint32_t flags);
bool udciIsr(void *pUdciHndl);
bool udciConnect(void *pUdciHndl);
bool udciDisconnect(void *pUdciHndl);
bool udciSetAddress(void *pUdciHndl, uint8_t address);
bool udciInitEp(void *pUdciHndl, usbdEpInfo *pEpInfo);
bool udciEnableEp(void *pUdciHndl, usbdEpInfo *pEpInfo);
bool udciDisableEp(void *pUdciHndl, usbdEpInfo *pEpInfo);
bool udciStallEp(void *pUdciHndl, usbdEpInfo *pEpInfo);
bool udciClearStallEp(void *pUdciHndl, usbdEpInfo *pEpInfo);
bool udciEp0TxStatus(void *pUdciHndl);
bool udciEpTxData(void *pUdciHndl, usbdIoReq *pIoReq);
bool udciEpRxData(void *pUdciHndl, usbdIoReq *pIoReq);


bool udciRegisterCallbacks(void *pUdciHndl);

#endif

