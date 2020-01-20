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
#ifndef _USB_DCD_IF_H_
#define _USB_DCD_IF_H_

/* Core driver for device controller
 * @dev_core_init: Intializes device controller
 * @dev_core_deinit: De-initializes device controller
 * @dev_core_register_callback: Registers callback function for
 * an event to be called by the controller driver
 * @dev_core_unregister_callback: Unregisters callback function
 * for an event
 * @dev_core_isr_routine: core interrupt service routine for
 * device controller to be used by OS/stack-i/f layer
 * @dev_core_connect: Enable device controller to connect to USB host
 * @dev_core_disconnect: Soft disconnect device controller from
 * USB host
 * @dev_get_speed: Get USB bus speed on which device controller
 * is attached
 * @dev_core_set_address: Set USB device address in device controller
 * @dev_core_set_config: Set configuration number for device controller
 * @dev_core_set_link_state: Set link state for device controller
 * @dev_core_init_ep: Initialize non-EP0 endpoint
 * @dev_core_ep_enable: Enable endpoint
 * @dev_core_ep_disable: Disable endpoint
 * @dev_core_ep_stall: Stall/Halt endpoint
 * @dev_core_ep_clear_stall: Clear Stall/Halt on endpoint
 * @dev_core_ep_set_nrdy: Set endpoint to not ready state
 * @dev_core_ep0_rx_setup_pkt: Receive SETUP packet on EP0
 * @dev_core_ep0_rx_status_pkt: Receive status packet on EP0
 * @dev_core_ep0_tx_status_pkt: Transmit status packet from EP0
 * @dev_core_ep_tx_data: Transmit data from EP
 * @dev_core_ep_rx_data: Received data on EP
 * @dev_core_ep_cancel_transfer: Cancel transfer on EP
 */
struct usb_device_core_driver {
    enum usb_status (*dev_core_init)(
    		struct usb_dev_config_params *config_params,
    		void *parent_handle, void **core_handle);
    enum usb_status (*dev_core_deinit)(void *core_handle,
    		uint32_t flags);
    enum usb_status (*dev_core_register_callback)(
    		void *core_handle, enum usb_device_event_id event,
    		enum usb_status (*callback_fn)(
    		struct usb_device_callback_param *cb_event_params));
    enum usb_status (*dev_core_unregister_callback)(
    		void *core_handle, enum usb_device_event_id event);
    enum usb_status (*dev_core_isr_routine)(void *core_handle);
    enum usb_status (*dev_core_connect)(void *core_handle);
    enum usb_status (*dev_core_disconnect)(void *core_handle);
    enum usb_status (*dev_get_speed)(void *core_handle,
    		enum usb_speed *speed);
    enum usb_status (*dev_core_set_address)(void *core_handle,
    		uint32_t address);
    enum usb_status (*dev_core_set_config)(void *core_handle,
    		uint32_t config_num);
    enum usb_status (*dev_core_set_link_state)(void *core_handle,
    		enum usb_device_ss_link_state state);
    enum usb_status (*dev_core_init_ep)(void *core_handle,
    		struct usb_ep_info *ep_info);
    enum usb_status (*dev_core_ep_enable)(void *core_handle,
    		struct usb_ep_info *ep_info);
    enum usb_status (*dev_core_ep_disable)(void *core_handle,
    		struct usb_ep_info *ep_info);
    enum usb_status (*dev_core_ep_stall)(void *core_handle,
    		struct usb_ep_info *ep_info);
    enum usb_status (*dev_core_ep_clear_stall)(void *core_handle,
    		struct usb_ep_info *ep_info);
    enum usb_status (*dev_core_ep_set_nrdy)(void *core_handle,
    		struct usb_ep_info *ep_info);
    enum usb_status (*dev_core_ep0_rx_setup_pkt)(void *core_handle,
    		uint8_t *buffer);
    enum usb_status (*dev_core_ep0_rx_status_pkt)(
    		void *core_handle);
    enum usb_status (*dev_core_ep0_tx_status_pkt)(
    		void *core_handle);
    enum usb_status (*dev_core_ep_tx_data)(void *core_handle,
    		struct usb_xfer_request *xfer_handle);
    enum usb_status (*dev_core_ep_rx_data)(void *core_handle,
    		struct usb_xfer_request *xfer_handle);
    enum usb_status (*dev_core_ep_cancel_transfer)(
    		void *core_handle, struct usb_ep_info *ep_info);
};

/* This API is used to obtain the driver handle for HW-independent API
 * @id: The ID of the core for which this driver is requested
 */
const struct usb_device_core_driver *usb_device_get_core_driver(
		enum usb_controller_id id);

#endif
