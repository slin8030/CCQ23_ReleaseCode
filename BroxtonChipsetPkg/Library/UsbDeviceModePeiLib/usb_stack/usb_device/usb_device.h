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

/* @usb_dev_config_params: Struct to be filled in with configuration
 * parameters and passed to the init routine for device controller 
 */
struct usb_dev_config_params {
	/* controller ID (from driver software infrastructure)
	 *  of the core
	 */ 
    enum usb_controller_id id;
    
    /* Base address of the controller registers and on-chip memory */
    uint32_t base_address;
    
    /* Initialization flags */
    uint32_t flags;
    
    /* Desired USB bus speed */
    enum usb_speed speed;
    
    /* Default USB role */
    enum usb_role role;
};

/* @usb_dev_core: Struct used as a handle for all
 * hardware-independent APIs 
 */
struct usb_dev_core {
    const struct usb_device_core_driver *core_driver;
    void *controller_handle;
};

enum usb_status usb_device_init(struct usb_dev_config_params *config_params, void **core_handle);
enum usb_status usb_device_deinit(void *core_handle, uint32_t flags);
enum usb_status usb_device_register_callback(void *core_handle,
		enum usb_device_event_id event,
		enum usb_status (*callback_fn)(
			struct usb_device_callback_param *));
enum usb_status usb_device_unregister_callback(void *core_handle, enum usb_device_event_id event_id);
enum usb_status usb_device_isr_routine(void *core_handle);
enum usb_status usb_device_connect(void *core_handle);
enum usb_status usb_device_disconnect(void *core_handle);
enum usb_status usb_device_get_speed(void *dev_core_handle, enum usb_speed *speed);
enum usb_status usb_device_set_link_state(void *core_handle, enum usb_device_ss_link_state state);
enum usb_status usb_device_set_address(void *core_handle, uint32_t address);
enum usb_status usb_device_set_configuration(void *core_handle, uint32_t config_num);
enum usb_status usb_device_init_ep(void *core_handle, struct usb_ep_info *ep_info);
enum usb_status usb_device_ep_enable(void *core_handle, struct usb_ep_info *ep_info);
enum usb_status usb_device_ep_disable(void *core_handle, struct usb_ep_info *ep_info);
enum usb_status usb_device_ep_stall(void *dev_core_handle, struct usb_ep_info *ep_info);
enum usb_status usb_device_ep_clear_stall(void *dev_core_handle, struct usb_ep_info *ep_info);
enum usb_status usb_device_ep_set_nrdy(void *dev_core_handle, struct usb_ep_info *ep_info);
enum usb_status usb_device_ep0_rx_setup(void *dev_core_handle, uint8_t *buffer);
enum usb_status usb_device_ep0_rx_status(void *dev_core_handle);
enum usb_status usb_device_ep0_tx_status(void *dev_core_handle);
enum usb_status usb_device_ep_tx_data(void *core_handle, struct usb_xfer_request *xfer_req);
enum usb_status usb_device_ep_rx_data(void *core_handle, struct usb_xfer_request *xfer_req);
enum usb_status usb_device_ep_cancel_transfer(void *core_handle, struct usb_ep_info *ep_info);

