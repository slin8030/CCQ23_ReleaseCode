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
//#include <stdint.h>
//#include <stddef.h>
//#include <stdbool.h>
#include "usb.h"
#include "usb_common.h"
#include "usb_device.h"
#include "usb_dcd_if.h"
#include "usb_dwc_xdci.h"


/* TODO: Document these later */


static const struct usb_device_core_driver core_driver_tbl[USB_CORE_ID_MAX] = {
        dwc_xdci_core_init,
        dwc_xdci_core_deinit,
        dwc_xdci_core_register_callback,
        dwc_xdci_core_unregister_callback,
        dwc_xdci_core_isr_routine,
        dwc_xdci_core_connect,
        dwc_xdci_core_disconnect,
        dwc_xdci_core_get_speed,
        dwc_xdci_core_set_address,
        dwc_xdci_core_set_config,
        dwc_xdci_set_link_state,
        dwc_xdci_init_ep,
        dwc_xdci_ep_enable,
        dwc_xdci_ep_disable,
        dwc_xdci_ep_stall,
        dwc_xdci_ep_clear_stall,
        dwc_xdci_ep_set_nrdy,
        dwc_xdci_ep0_receive_setup_pkt,
        dwc_xdci_ep0_receive_status_pkt,
        dwc_xdci_ep0_send_status_pkt,
        dwc_xdci_ep_tx_data,
        dwc_xdci_ep_rx_data,
        dwc_xdci_ep_cancel_transfer
#if 0
        .dev_core_init = dwc_xdci_core_init,
        .dev_core_deinit = dwc_xdci_core_deinit,
        .dev_core_register_callback = dwc_xdci_core_register_callback,
        .dev_core_unregister_callback = dwc_xdci_core_unregister_callback,
        .dev_core_isr_routine = dwc_xdci_core_isr_routine,
        .dev_core_connect = dwc_xdci_core_connect,
        .dev_core_disconnect = dwc_xdci_core_disconnect,
        .dev_get_speed = dwc_xdci_core_get_speed,
        .dev_core_set_address = dwc_xdci_core_set_address,
        .dev_core_set_config = dwc_xdci_core_set_config,
        .dev_core_set_link_state = dwc_xdci_set_link_state,
        .dev_core_init_ep = dwc_xdci_init_ep,
        .dev_core_ep_enable = dwc_xdci_ep_enable,
        .dev_core_ep_disable = dwc_xdci_ep_disable,
        .dev_core_ep_stall = dwc_xdci_ep_stall,
        .dev_core_ep_clear_stall = dwc_xdci_ep_clear_stall,
        .dev_core_ep_set_nrdy = dwc_xdci_ep_set_nrdy,
        .dev_core_ep0_rx_setup_pkt = dwc_xdci_ep0_receive_setup_pkt,
        .dev_core_ep0_rx_status_pkt = dwc_xdci_ep0_receive_status_pkt,
        .dev_core_ep0_tx_status_pkt = dwc_xdci_ep0_send_status_pkt,
        .dev_core_ep_tx_data = dwc_xdci_ep_tx_data,
        .dev_core_ep_rx_data = dwc_xdci_ep_rx_data,
        .dev_core_ep_cancel_transfer = dwc_xdci_ep_cancel_transfer
    },
#endif    
};

const struct usb_device_core_driver *usb_device_get_core_driver(enum usb_controller_id id)
{
    if (id >= USB_CORE_ID_MAX)
        return NULL;

    return &core_driver_tbl[id];
}

