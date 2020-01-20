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
//#include <stddef.h>
//#include <mem_util.h>
//#include <stdio.h>
//#include <stdlib.h>
#include "usb.h"
#include "usb_common.h"
#include "usb_device.h"
#include "usb_dcd_if.h"

/* 
 * This function is used to initialize the device controller
 * @config_params: Parameters from app to configure the core
 * @dev_core_handle: Return parameter for upper layers to use
 * for all HW-independent APIs
 */ 
enum usb_status usb_device_init(
	struct usb_dev_config_params *config_params,
    void **dev_core_handle)
{
    struct usb_dev_core *device_core_ptr;
    enum usb_status status = USB_INVALID_PARAM;

    	DEBUG ((DEBUG_INFO, "\n Call usb_device_init start\n"));

    /* Allocate device handle */
    device_core_ptr = pmalloc(sizeof(struct usb_dev_core));

    	DEBUG ((DEBUG_INFO, "\n device handle=%x\n", device_core_ptr));

    if (NULL == device_core_ptr) {
    	DEBUG ((DEBUG_INFO, "\n usb_device_init. ERROR: Failed to allocate memory"));

    	return USB_OUT_OF_MEMORY;
    }

    	DEBUG ((DEBUG_INFO, "\n call usb_device_get_core_driver, ID=%x, \n", config_params->id));

    /* Get the driver for this USB device core */
    device_core_ptr->core_driver = usb_device_get_core_driver(config_params->id);
   
    if (NULL != device_core_ptr->core_driver) {

    	DEBUG ((DEBUG_INFO, "\n call dev_core_init\n"));

        status =
            device_core_ptr->core_driver->dev_core_init(
            		config_params, 
            		(void*)device_core_ptr,
            		&device_core_ptr->controller_handle);
    } else {
        DEBUG ((DEBUG_INFO, "\nusb_device_init. ERROR: Driver not found"));

        return USB_INVALID_PARAM;
    }

    *dev_core_handle = (void *)device_core_ptr;

    return status;
}

/* 
 * This function is used to de-initialize the device controller
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @flags: Flags indicating what type of de-initialization is required
 */ 
enum usb_status usb_device_deinit(void *dev_core_handle,
		uint32_t flags)
{
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
    	DEBUG ((DEBUG_INFO, "\nusb_device_deinit: ERROR: INVALID HANDLE"));
    } else {
		if (NULL != core->core_driver) {
			status =
				core->core_driver->dev_core_deinit(
						core->controller_handle, flags);
		} else {
			DEBUG ((DEBUG_INFO, "\nusb_device_deinit: Driver not found"));
			status = USB_INVALID_PARAM;
		}
    }

    return status;
}

/* 
 * This function is used to register callback function for
 * specified event
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @event: Event for which callback is to be registered
 * @callback_fn: Callback function to be called by the
 * controller driver for above event after critical processing
 */ 
enum usb_status usb_device_register_callback(void *dev_core_handle,
		enum usb_device_event_id event,
		enum usb_status (*callback_fn)(
				struct usb_device_callback_param *cb_event_params))
{
	enum usb_status status = USB_INVALID_HANDLE;
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;

    	DEBUG ((DEBUG_INFO, "\n usb_device_register_callback start"));
    
    if (NULL == core) {
    	DEBUG ((DEBUG_INFO, "\nusb_device_register_callback: ERROR: INVALID HANDLE"));
    } else {
		if (NULL != core->core_driver) {
    	DEBUG ((DEBUG_INFO, "\n Call dev_core_register_callback"));
			
			
			status =
					core->core_driver->dev_core_register_callback(
						core->controller_handle, event, callback_fn);
		}
    }

    return status;
}

/* 
 * This function is used to register callback function for
 * specified event
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @event_id: Event for which callback is to be unregistered
 */ 
enum usb_status usb_device_unregister_callback(void *dev_core_handle,
		enum usb_device_event_id event_id)
{
	enum usb_status status = USB_INVALID_HANDLE;
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;
    
    if (NULL == core) {
    	DEBUG ((DEBUG_INFO, "\nusb_device_unregister_callback: ERROR: INVALID HANDLE"));
    } else {
		if (NULL != core->core_driver) {
			status =
				core->core_driver->dev_core_unregister_callback(
					core->controller_handle, event_id);    	
		}
    }

    return status;
}

/* 
 * This function is used to service interrupt events on device
 * controller. Use this API in your OS/stack-specific ISR framework
 * In polled mode scenario, invoke this API in a loop to service the
 * events
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 */ 
enum usb_status usb_device_isr_routine(void *dev_core_handle)
{
    struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
    	DEBUG ((DEBUG_INFO, "\nusb_device_isr_routine: ERROR: INVALID HANDLE"));
    } else {
		if (NULL != core->core_driver) {
			status =
				core->core_driver->dev_core_isr_routine(
						core->controller_handle);
		}
    }

    return status;
}

/* 
 * This function is used to enable device controller to connect
 * to USB host
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 */ 
enum usb_status usb_device_connect(void *dev_core_handle)
{
    struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
    	DEBUG ((DEBUG_INFO, "\nusb_device_connect: ERROR: INVALID HANDLE"));
    } else {
        status = core->core_driver->dev_core_connect(
        		core->controller_handle);
    }

    return status;
}

/* 
 * This function is used to disconnect device controller
 * from USB host
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 */ 
enum usb_status usb_device_disconnect(void *dev_core_handle)
{
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_disconnect: ERROR: INVALID HANDLE"));
    } else {
        status = core->core_driver->dev_core_disconnect(
        		core->controller_handle);
    }

    return status;
}

/* 
 * This function is used to obtain USB bus speed after bus reset complete
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @speed: negotiated speed
 */ 
enum usb_status usb_device_get_speed(void *dev_core_handle,
		enum usb_speed *speed)
{
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;
    
    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_get_speed: ERROR: INVALID HANDLE"));
    } else {
        status = core->core_driver->dev_get_speed(
        		core->controller_handle, speed);
    }

    return status;
}

/* 
 * This function is used to set USB device address
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @address: USB device address to set
 */ 
enum usb_status usb_device_set_address(void *dev_core_handle,
		uint32_t address)
{
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_set_address: ERROR: INVALID HANDLE"));
    } else {    
    	status = core->core_driver->dev_core_set_address(
    		core->controller_handle, address);
    }

    return status;
}

/* 
 * This function is used to do device controller-specific processing
 * of set configuration device framework request
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @config_num: configuration number selected by USB host 
 */ 
enum usb_status usb_device_set_configuration(void *dev_core_handle,
		uint32_t config_num)
{
    struct usb_dev_core *core =
    		(struct usb_dev_core *)dev_core_handle;
    enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_set_configuration: ERROR: INVALID HANDLE"));
    } else {
        status = core->core_driver->dev_core_set_config(
        		core->controller_handle, config_num);
    }

    return status;	
}

/* 
 * This function is used to set desired link state in device controller
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @state: Desired link state 
 */ 
enum usb_status usb_device_set_link_state(void *dev_core_handle,
		enum usb_device_ss_link_state state)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_set_link_state: ERROR: INVALID HANDLE"));
    } else {
	    status = core->core_driver->dev_core_set_link_state(
	    		core->controller_handle, state);
    }

	return status;
}

/* 
 * This function is used to initialize non-EP0 endpoints
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint information for EP to be initialized 
 */ 
enum usb_status usb_device_init_ep(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_init_ep: ERROR: INVALID HANDLE"));
    } else {
	    status = core->core_driver->dev_core_init_ep(
	    		core->controller_handle, ep_info);
    }

	return status;
}

/* 
 * This function is used to enable an endpoint
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint information for EP to be enabled 
 */ 
enum usb_status usb_device_ep_enable(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_ep_enable: ERROR: INVALID HANDLE"));
    } else {
	    status = core->core_driver->dev_core_ep_enable(
	    		core->controller_handle, ep_info);
    }

	return status;
}

/* 
 * This function is used to disable an endpoint
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint information for EP to be disabled 
 */ 
enum usb_status usb_device_ep_disable(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_ep_disable ERROR: INVALID HANDLE"));
    } else {
	    status = core->core_driver->dev_core_ep_disable(
	    		core->controller_handle, ep_info);
    }

	return status;
}

/* 
 * This function is used to STALL an endpoint
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint information for EP to be stalled 
 */ 
enum usb_status usb_device_ep_stall(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

    if (NULL == core) {
        DEBUG ((DEBUG_INFO, "\nusb_device_ep_stall ERROR: INVALID HANDLE"));
    } else {
	    status = core->core_driver->dev_core_ep_stall(
	    		core->controller_handle, ep_info);
    }

	return status;
}

/* 
 * This function is used to clear STALL on an endpoint
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint information for which STALL needs to be cleared 
 */ 
enum usb_status usb_device_ep_clear_stall(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
	    DEBUG ((DEBUG_INFO, "\nusb_device_ep_clear_stall ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep_clear_stall(
	    		core->controller_handle, ep_info);
	}

	return status;
}

/* 
 * This function is used to set EP not ready state
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint information for EP that needs to be
 * set in not ready state 
 */ 
enum usb_status usb_device_ep_set_nrdy(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core =
			(struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
	    DEBUG ((DEBUG_INFO, "\nusb_device_ep_set_nrdy ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep_set_nrdy(
	    		core->controller_handle, ep_info);
	}

	return status;
}

/* 
 * This function is used to queue request to receive SETUP packet
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @buffer: Buffer (bus-width aligned) where SETUP packet
 * needs to be received
 */ 
enum usb_status usb_device_ep0_rx_setup(void *dev_core_handle,
		uint8_t *buffer)
{
	struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
	    DEBUG ((DEBUG_INFO, "\nusb_device_ep0_rx_setup ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep0_rx_setup_pkt(
	    		core->controller_handle, buffer);
	}

	return status;
}

/* 
 * This function is used to queue request to receive status phase
 * for control transfer on EP0
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 */ 
enum usb_status usb_device_ep0_rx_status(void *dev_core_handle)
{
	struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
	    DEBUG ((DEBUG_INFO, "\nusb_device_ep0_rx_status ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep0_rx_status_pkt(
	    		core->controller_handle);
	}
	return status;
}

/* 
 * This function is used to queue request to send status phase for
 * control transfer on EP0
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 */ 
enum usb_status usb_device_ep0_tx_status(void *dev_core_handle)
{
	struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
	    DEBUG ((DEBUG_INFO, "\nusb_device_ep0_tx_status ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep0_tx_status_pkt(
	    		core->controller_handle);
	}

	return status;
}

/* 
 * This function is used to queue a single request to transmit data on 
 * an endpoint. If more than one request need to be queued before
 * previous requests complete then a request queue needs to be
 * implemented in upper layers. This API should be not be invoked until
 * current request completes.
 * Callback for transfer completion is invoked when requested transfer length
 * is reached or if a short packet is received
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @xfer_req: Address to transfer request describing this transfer
 */ 
enum usb_status usb_device_ep_tx_data(void *dev_core_handle,
		struct usb_xfer_request *xfer_req)
{
	struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
	    DEBUG ((DEBUG_INFO, "\nusb_device_ep_tx_data ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep_tx_data(
	    		core->controller_handle, xfer_req);
	}

	return status;
}

/* 
 * This function is used to queue a single request to receive data on 
 * an endpoint. If more than one request need to be queued before
 * previous requests complete then a request queue needs to be implemented
 * in upper layers. This API should be not be invoked until current request
 * completes.
 * Callback for transfer completion is invoked when requested transfer length
 * is reached or if a short packet is received
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @xfer_req: Address to transfer request describing this transfer
 */ 
enum usb_status usb_device_ep_rx_data(void *dev_core_handle,
		struct usb_xfer_request *xfer_req)
{
	struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
		DEBUG ((DEBUG_INFO, "\nusb_device_ep_rx_data ERROR: INVALID HANDLE"));
	} else {
	    status = core->core_driver->dev_core_ep_rx_data(
	    		core->controller_handle, xfer_req);
	}

	return status;
}

/* 
 * This function is used to cancel a transfer request that was
 * previously queued on an endpoint
 * @dev_core_handle: Handle to HW-independent APIs for device
 * controller
 * @ep_info: Endpoint info where transfer needs to be cancelled
 */ 
enum usb_status usb_device_ep_cancel_transfer(void *dev_core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dev_core *core = (struct usb_dev_core *)dev_core_handle;
	enum usb_status status = USB_INVALID_HANDLE;

	if (NULL == core) {
		DEBUG ((DEBUG_INFO, "\nusb_device_ep_cancel_transfer ERROR: INVALID HANDLE"));
	} else {
		status = core->core_driver->dev_core_ep_cancel_transfer(
					core->controller_handle, ep_info);
	}

	return status;
}
