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
#ifndef _USB_COMMON_H_
#define _USB_COMMON_H_


#define UINT8         uint8_t;
#define UINT16         uint16_t;
#define UINT32         uint32_t;
#define BOOLEAN        bool


#define USB_DEBUG 0
#if USB_DEBUG
    #define usb_dbg printf
#else
    #define usb_dbg(...)
#endif

#define USB_SETUP_DATA_PHASE_DIRECTION_MASK	(0x80)

/* EP type values are based on USB device framework */
enum usb_ep_type {
    USB_EP_TYPE_CONTROL = 0,
    USB_EP_TYPE_ISOCHRONOUS = 1,
    USB_EP_TYPE_BULK = 2,
    USB_EP_TYPE_INTERRUPT = 3
};

/* EP direction */
enum usb_ep_dir {
    USB_EP_DIR_OUT = 0,
    USB_EP_DIR_IN = 1
};

/* USB speeds */
enum usb_speed {
    USB_SPEED_HIGH = 0,
    USB_SPEED_FULL,
    USB_SPEED_LOW,
    USB_SPEED_SUPER = 4
};

enum usb_controller_id {
    USB_ID_DWC_XDCI = 0,
    USB_CORE_ID_MAX
};

enum usb_role {
	USB_ROLE_HOST = 1,
	USB_ROLE_DEVICE,
	USB_ROLE_OTG
};

enum usb_xfer_status {
    USB_XFER_QUEUED = 0,
    USB_XFER_SUCCESSFUL,
    USB_XFER_STALL
};

enum usb_status {
    USB_OK = 0,
    USB_INVALID_HANDLE,
    USB_INVALID_PARAM,
    USB_INVALID_OP,
    USB_OUT_OF_MEMORY,
    USB_OP_FAILED
};

enum usb_device_event_id {
    USB_DEVICE_DISCONNECT_EVENT = 0,
    USB_DEVICE_RESET_EVENT,
    USB_DEVICE_CONNECTION_DONE,
    USB_DEVICE_STATE_CHANGE_EVENT,
    USB_DEVICE_WAKEUP_EVENT,
    USB_DEVICE_HIBERNATION_REQ_EVENT,
    USB_DEVICE_SOF_EVENT = 7,
    USB_DEVICE_ERRATIC_ERR_EVENT = 9,
    USB_DEVICE_CMD_CMPLT_EVENT,
    USB_DEVICE_BUFF_OVERFLOW_EVENT,
    USB_DEVICE_TEST_LMP_RX_EVENT,
    USB_DEVICE_SETUP_PKT_RECEIVED,
    USB_DEVICE_XFER_NRDY,
    USB_DEVICE_XFER_DONE
};

enum usb_device_ss_link_state {
	U0 = 0,
	U1,
	U2,
	U3,
	SS_DIS,
	RX_DET,
	SS_INACT,
	POLL,
	RECOV,
	HRESET,
	CMPLY,
	LPBK,
	RESUME_RESET = 15
};

enum usb_device_state {
	USB_DEVICE_STATE_DEFAULT,
	USB_DEVICE_STATE_ADDRESS,
	USB_DEVICE_STATE_CONFIGURED,
	USB_DEVICE_STATE_SUSPENDED,
	USB_DEVICE_STATE_ERROR
};

enum usb_control_xfer_phase {
	CTRL_SETUP_PHASE,
	CTRL_DATA_PHASE,
	CTRL_STATUS_PHASE
};

enum usb_ep_state {
	USB_EP_STATE_DISABLED = 0,
	USB_EP_STATE_ENABLED,
	USB_EP_STATE_STALLED,
	USB_EP_STATE_SETUP,
	USB_EP_STATE_IN_DATA,
	USB_EP_STATE_OUT_DATA,
	USB_EP_STATE_DATA,
	USB_EP_STATE_STATUS
};

struct usb_device_callback_param {
	void *parent_handle;
	uint32_t hird;
	uint32_t ep_num;
	enum usb_speed speed;
	enum usb_ep_state ep_state;
	enum usb_ep_dir ep_dir;
	enum usb_ep_type ep_type;
	enum usb_device_ss_link_state link_state;
	uint8_t *buffer;
	bool ss_event;
};

/* USB endpoint */
struct usb_ep_info {
    uint32_t ep_num;
    enum usb_ep_dir ep_dir;
    enum usb_ep_type ep_type;
	uint32_t max_pkt_size;
    uint32_t max_streams;
    uint32_t burst_size;
    uint32_t interval;
    uint32_t mult;
};

/* USB transfer request */
struct usb_xfer_request {
    /* Buffer address. bus-width aligned */
    void *xfer_buffer;

    /* Requested transfer length */
    uint32_t xfer_len;

    /* Actual transfer length at completion callback stage */
    uint32_t actual_xfer_len;

    /* Stream ID. Only relevant for bulk streaming */
    uint32_t stream_id;
    
    /* Only relevant for periodic transfer */
    uint32_t frame_num;

    /* Transfer status */
    enum usb_xfer_status xfer_status;
    
    /* EP info */
    struct usb_ep_info ep_info;

    /* Transfer completion callback */
    void (*xfer_done)(void *core_handle,
    		struct usb_xfer_request *xfer_handle);

    /* Do zero-length transfer */
    bool zlp;
};

#endif
