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
//#include <stdbool.h>
//#include <mem_util.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "usb.h"
#include "usb_common.h"
#include "usb_device.h"
#include "usb_dcd_if.h"
#include "usb_dwc_xdci.h"
//#include "delay.h"
#include "UdmWrapperPeiLib.h"


/* Internal utility function:
 * This function is used to obtain physical endpoint number
 * xDCI needs physical endpoint number for EP registers
 * We also use it to index into our EP array
 * Note: Certain data structures/commands use logical EP numbers
 * as opposed to physical endpoint numbers so one should be
 * careful when interpreting EP numbers
 * @ep_num: Logical endpoint number
 * @ep_dir: Direction for the endpoint
 */ 
static uint32_t dwc_xdci_get_physical_ep_num(uint32_t ep_num,
		enum usb_ep_dir ep_dir)
{
	return ep_dir ? ((ep_num << 1) | ep_dir) : (ep_num << 1);
}

/* Internal utility function:
 * This function is used to obtain the MPS for control transfers
 * based on the speed. If this is called before bus reset completes
 * then it returns MPS based on desired speed. If it is after bus
 * reset then MPS returned is based on actual negotiated speed
 * @core_handle: xDCI controller handle address
 * @mps: address of 32-bit variable to return the MPS
 */
static enum usb_status dwc_xdci_core_get_ctrl_mps(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t *mps)
{
	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_get_ctrl_mps: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	if (NULL == mps) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_get_ctrl_mps: INVALID parameter"));
		return USB_INVALID_PARAM;
	}

	switch (core_handle->actual_speed) {
		case USB_SPEED_HIGH:
			*mps = DWC_XDCI_HS_CTRL_EP_MPS;
			break;
		case USB_SPEED_FULL:
			*mps = DWC_XDCI_FS_CTRL_EP_MPS;
			break;
		case USB_SPEED_LOW:
			*mps = DWC_XDCI_LS_CTRL_EP_MPS;
			break;
		case USB_SPEED_SUPER:
			*mps = DWC_XDCI_SS_CTRL_EP_MPS;
			break;
		default:
			*mps = 0;
			DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_get_ctrl_mps: UNKNOWN speed"));
			break;
		}

	return USB_OK;
}

/* Internal utility function:
 * This function is used to initialize the parameters required
 * for executing endpoint command
 * @core_handle: xDCI controller handle address
 * @ep_info: EP info address
 * @config_action: Configuration action specific to EP command
 * @ep_cmd: xDCI EP command for which parameters are initialized
 * @ep_cmd_params: address of struct to return EP params
 */ 
static enum usb_status dwc_xdci_core_init_ep_cmd_params(
		struct usb_dcd_xdci_core_handle *core_handle,
		struct usb_ep_info *ep_info,
		uint32_t config_action,
		enum dwc_xdci_ep_cmd ep_cmd,
		struct dwc_xdci_ep_cmd_params *ep_cmd_params)
{
	enum usb_status status = USB_OK;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_init_ep_cmd_params: INVALID handle"));

		return USB_INVALID_HANDLE;
	}

    /* Reset params */
    ep_cmd_params->param0 = ep_cmd_params->param1 =
    		ep_cmd_params->param2 = 0;

	switch (ep_cmd) {
	case EPCMD_SET_EP_CONFIG:
	    /* Issue DEPCFG command for EP */
		/* Issue a DEPCFG (Command 1) command for endpoint */

		if (ep_info->max_streams) {
			ep_cmd_params->param1 =
				DWC_XDCI_PARAM1_SET_EP_CFG_STRM_CAP_MASK;
		}

		if (ep_info->interval) {
		    ep_cmd_params->param1 |=
		    		((ep_info->interval-1) <<
		    		  DWC_XDCI_PARAM1_SET_EP_CFG_BINTM1_BIT_POS);
		}

	    /* Set EP num */
	    ep_cmd_params->param1 |=
            (ep_info->ep_num <<
            		DWC_XDCI_PARAM1_SET_EP_CFG_EP_NUM_BIT_POS);
	    
	    /* Set EP direction */
	    ep_cmd_params->param1 |=
            (ep_info->ep_dir <<
            		DWC_XDCI_PARAM1_SET_EP_CFG_EP_DIR_BIT_POS);
	    
	    /* Set EP-specific Event enable for not ready and
	     * complete events 
	     */
        ep_cmd_params->param1 &=
	    		~DWC_XDCI_PARAM1_SET_EP_CFG_EVT_EN_MASK;

        /* Setup the events we want enabled for this EP */
	    ep_cmd_params->param1 |= (
	    		DWC_XDCI_PARAM1_SET_EP_CFG_EVT_XFER_NRDY_MASK |
	    		DWC_XDCI_PARAM1_SET_EP_CFG_EVT_XFER_IN_PRG_MASK |
	    		DWC_XDCI_PARAM1_SET_EP_CFG_EVT_XFER_CMPLT_MASK);

	    /* We only have one interrupt line for this core.
	     * Set interrupt number to 0
	     */
	    ep_cmd_params->param1 &=
	    		~DWC_XDCI_PARAM1_SET_EP_CFG_INTR_NUM_MASK;

	    /* Set FIFOnum = 0 for control EP0 */
	    ep_cmd_params->param0 &= ~DWC_XDCI_PARAM0_SET_EP_CFG_FIFO_NUM_MASK;
	    
	    /* Program FIFOnum for non-EP0 EPs */
	    if (ep_info->ep_num && ep_info->ep_dir) {
	        ep_cmd_params->param0 |=
	        	(ep_info->ep_num <<
	        		DWC_XDCI_PARAM0_SET_EP_CFG_FIFO_NUM_BIT_POS);
	    }

	    /* Program max packet size */
	    ep_cmd_params->param0 &=
	    		~DWC_XDCI_PARAM0_SET_EP_CFG_MPS_MASK;
	    ep_cmd_params->param0 |=
	    		(ep_info->max_pkt_size <<
	    			DWC_XDCI_PARAM0_SET_EP_CFG_MPS_BIT_POS);

	    /* Set Burst size. 0 means burst size of 1 */
	    ep_cmd_params->param0 &=
	    		~DWC_XDCI_PARAM0_SET_EP_CFG_BRST_SIZE_MASK;
	    ep_cmd_params->param0 |= (ep_info->burst_size << 
	    		DWC_XDCI_PARAM0_SET_EP_CFG_BRST_SIZE_BIT_POS);

	    /* Set EP type */
	    ep_cmd_params->param0 &=
	    		~DWC_XDCI_PARAM0_SET_EP_CFG_EP_TYPE_MASK;
	    ep_cmd_params->param0 |=
	    		(ep_info->ep_type <<
	    			DWC_XDCI_PARAM0_SET_EP_CFG_EP_TYPE_BIT_POS);
	    
	    /* Set config action */
	    ep_cmd_params->param0 &=
	    		~DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_MASK;
	    ep_cmd_params->param0 |= (config_action <<
	    		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_BIT_POS);
	    break;

	case EPCMD_SET_EP_XFER_RES_CONFIG:
		/* Set param0 to 1. Same for all EPs when resource
		 *  configuration is done 
		 */
		ep_cmd_params->param0 = 1;
		break;

	case EPCMD_END_XFER:
		/* Nothing to set. Already reset params for all cmds */
		break;

	case EPCMD_START_NEW_CONFIG:
		/* Nothing to set. Already reset params for all cmds */
		break;

	default:
		status = USB_INVALID_PARAM;
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init_ep_cmd_params: INVALID Parameter"));
		break;
	}

	return status;
}

/* Internal utility function:
 * This function is used to issue the xDCI endpoint command
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical EP num
 * @ep_cmd: xDCI EP command
 * @ep_cmd_params: EP command parameters address
 */ 
static enum usb_status dwc_xdci_core_issue_ep_cmd(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num,
		uint32_t ep_cmd, 
		struct dwc_xdci_ep_cmd_params *ep_cmd_params)
{
    uint32_t base_addr;
    uint32_t max_delay_iter = 5000;//DWC_XDCI_MAX_DELAY_ITERATIONS;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_issue_ep_cmd: INVALID handle"));

		return USB_INVALID_HANDLE;
	}

	base_addr = core_handle->base_address;

	/* Set EP command parameter values */
	usb_reg_write(base_addr,
			DWC_XDCI_EPCMD_PARAM2_REG(ep_num),
			ep_cmd_params->param2);
	
	usb_reg_write(base_addr,
			DWC_XDCI_EPCMD_PARAM1_REG(ep_num),
					ep_cmd_params->param1);
	
	usb_reg_write(base_addr,
			DWC_XDCI_EPCMD_PARAM0_REG(ep_num),
			ep_cmd_params->param0);

	/* Set the command code and activate it */
	usb_reg_write(base_addr, DWC_XDCI_EPCMD_REG(ep_num),
					    ep_cmd |
					    DWC_XDCI_EPCMD_CMD_ACTIVE_MASK);
	
    /* Wait until command completes */
    do {
    	if (!(usb_reg_read(base_addr,
    			DWC_XDCI_EPCMD_REG(ep_num)) &
    			DWC_XDCI_EPCMD_CMD_ACTIVE_MASK))
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_issue_ep_cmd. ERROR: Failed to issue Command"));

    	return USB_OP_FAILED;
    }

	return USB_OK;
}

/* Internal utility function:
 * This function is used to flush all FIFOs
 * @core_handle: xDCI controller handle address
 */ 
static enum usb_status dwc_xdci_core_flush_all_fifos(
		struct usb_dcd_xdci_core_handle *core_handle)
{
	uint32_t base_addr;
	uint32_t max_delay_iter = DWC_XDCI_MAX_DELAY_ITERATIONS;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_flush_all_fifos: INVALID handle"));

		return USB_INVALID_HANDLE;
	}
	
	base_addr = core_handle->base_address;

	/* Write the command to flush all FIFOs */
	usb_reg_write(base_addr, DWC_XDCI_DGCMD_REG, 
			(usb_reg_read(base_addr, DWC_XDCI_DGCMD_REG) |
			  DWC_XDCI_DGCMD_CMD_ALL_FIFO_FLUSH |
			  DWC_XDCI_DGCMD_CMD_ACTIVE_MASK));
	
    /* Wait until command completes */
    do {
    	if (!(usb_reg_read(base_addr,
    			DWC_XDCI_DGCMD_REG) &
    			DWC_XDCI_DGCMD_CMD_ACTIVE_MASK))
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\nFailed to issue Command"));
    	return USB_OP_FAILED;
    }

    return USB_OK;
}

/* Internal utility function:
 * This function is used to flush Tx FIFO specific to an endpoint
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical EP num
 */ 
static enum usb_status dwc_xdci_core_flush_ep_tx_fifo(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num)
{
	uint32_t base_addr;
	uint32_t max_delay_iter = DWC_XDCI_MAX_DELAY_ITERATIONS;
	uint32_t fifo_num;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_flush_ep_tx_fifo: INVALID handle"));

		return USB_INVALID_HANDLE;
	}

	base_addr = core_handle->base_address;

	/* Translate to FIFOnum
	 * NOTE: Assuming this is a Tx EP
	 */
	fifo_num = (ep_num >> 1);

	/* TODO: Currently we are only using TxFIFO 0. Later map these
	 * Write the FIFO num/dir param for the generic command.  
	 */
	usb_reg_write(base_addr, DWC_XDCI_DGCMD_PARAM_REG, 
			((usb_reg_read(base_addr, DWC_XDCI_DGCMD_PARAM_REG) & 
					~DWC_XDCI_DGCMD_PARAM_TX_FIFO_NUM_MASK) |
					DWC_XDCI_DGCMD_PARAM_TX_FIFO_DIR_MASK));
	
	/* Write the command to flush all FIFOs */
	usb_reg_write(base_addr, DWC_XDCI_DGCMD_REG, 
			(usb_reg_read(base_addr, DWC_XDCI_DGCMD_REG) |
			  DWC_XDCI_DGCMD_CMD_SEL_FIFO_FLUSH |
			  DWC_XDCI_DGCMD_CMD_ACTIVE_MASK));


    /* Wait until command completes */
    do {
    	if (!(usb_reg_read(base_addr,
    			DWC_XDCI_DGCMD_REG) &
    			DWC_XDCI_DGCMD_CMD_ACTIVE_MASK))
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\nFailed to issue Command"));
    	return USB_OP_FAILED;
    }

    return USB_OK;
}

/* Internal utility function:
 * This function is used to initialize transfer request block
 * @core_handle: xDCI controller handle address
 * @trb: Address of TRB to initialize
 * @trb_ctrl: TRB control value
 * @buff_ptr: Transfer buffer address
 * @size: Size of the transfer
 */ 
static enum usb_status dwc_xdci_core_init_trb(
		struct usb_dcd_xdci_core_handle *core_handle,
		struct dwc_xdci_trb *trb,
		enum dwc_xdci_trb_control trb_ctrl,
		uint8_t *buffer_ptr,
		uint32_t size)
{
    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_init_trb: INVALID handle"));

		return USB_INVALID_HANDLE;
	}
    
    if (NULL == trb) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_init_trb: INVALID handle"));
        return USB_INVALID_PARAM;
    }

    /* Init TRB fields
     * NOTE: Assuming we are only using 32-bit addresses
     * TODO: update for 64-bit addresses
     */
    trb->buff_ptr_low = (uint32_t)(UINTN)buffer_ptr;
    trb->buff_ptr_high = 0;
    trb->len_xfer_params = size;
    trb->trb_ctrl = ((trb_ctrl <<
                    DWC_XDCI_TRB_CTRL_TYPE_BIT_POS) |
                    DWC_XDCI_TRB_CTRL_HWO_MASK |
                    DWC_XDCI_TRB_CTRL_LST_TRB_MASK |
                    DWC_XDCI_TRB_CTRL_IOSP_MISOCH_MASK);

	return USB_OK;
}

/* Internal function:
 * This function is used to start a SETUP phase on control endpoint
 * @core_handle: xDCI controller handle address
 */ 
static enum usb_status dwc_xdci_core_start_ep0_setup_xfer(
		struct usb_dcd_xdci_core_handle *core_handle)
{
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status = USB_INVALID_OP;
	struct dwc_xdci_trb *trb;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_core_start_ep0_setup_xfer: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

    
	core_handle->ep_handles[0].state = USB_EP_STATE_SETUP;
	trb = core_handle->trbs;

	status = dwc_xdci_core_init_trb(core_handle,
			trb,
			TRBCTL_SETUP,
			core_handle->aligned_setup_buffer, 8);

	if (status)
		return status;

    /* Issue a DEPSTRTXFER for EP0 */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
       ep_cmd_params.param2 = 0;

    /* Init the lower re-bits for TRB address */
    ep_cmd_params.param1 = (uint32_t)(UINTN)trb;

     /* Issue the command to start transfer on physical
      * endpoint 0
      */
    status = dwc_xdci_core_issue_ep_cmd(core_handle,
    		0, EPCMD_START_XFER, 
    		&ep_cmd_params);

    /* Save new resource index for this transfer */
    core_handle->ep_handles[0].currentXferRscIdx =
    		((usb_reg_read(core_handle->base_address,
    				DWC_XDCI_EPCMD_REG(0)) &
    		  DWC_XDCI_EPCMD_RES_IDX_MASK) >>
    				DWC_XDCI_EPCMD_RES_IDX_BIT_POS);

	return status;
}

/* Internal function:
 * This function is used to process the state change event
 * @core_handle: xDCI controller handle address
 * @event: device event dword
 */ 
static enum usb_status dwc_xdci_process_device_state_change_event(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t event)
{
    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_device_state_change_event: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

	core_handle->hird_val = (event &
			DWC_XDCI_EVENT_BUFF_DEV_HIRD_MASK) >>
			DWC_XDCI_EVENT_BUFF_DEV_HIRD_BIT_POS;

	core_handle->link_state =
			((event &
					DWC_XDCI_EVENT_BUFF_DEV_LINK_STATE_MASK) >>
					DWC_XDCI_EVENT_BUFF_DEV_LINK_STATE_BIT_POS);

	if (core_handle->event_callbacks.dev_link_state_callback) {
		core_handle->event_callbacks.cb_event_params.
			parent_handle = core_handle->parent_handle;
		core_handle->event_callbacks.cb_event_params.
			link_state = core_handle->link_state;
		core_handle->event_callbacks.cb_event_params.
					hird = core_handle->hird_val;

		core_handle->event_callbacks.cb_event_params.ss_event =
			(event & DWC_XDCI_EVENT_BUFF_DEV_SS_EVENT_MASK) ?
		      1 : 0;

		core_handle->event_callbacks.
				  dev_link_state_callback(
						  &core_handle->event_callbacks.
						  cb_event_params);
	}

	return USB_OK;
}

/* Internal function:
 * This function is used to issue a command to end transfer
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical EP num for which transfer is to be ended
 */ 
static enum usb_status dwc_xdci_end_xfer(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num)
{
	enum usb_status status;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	uint32_t cmd_params;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_end_xfer: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

    /* Issue a DEPENDXFER for EP */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    cmd_params = ((core_handle->ep_handles[ep_num].
    		        currentXferRscIdx <<
    		          DWC_XDCI_EPCMD_RES_IDX_BIT_POS) |
    		            DWC_XDCI_EPCMD_FORCE_RM_MASK);

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(core_handle,
    		ep_num, cmd_params | DWC_XDCI_EPCMD_END_XFER, 
    		&ep_cmd_params);
    
    if (!status) {
    	core_handle->ep_handles[ep_num].
    	    		        currentXferRscIdx = 0;
    	core_handle->trbs[ep_num].trb_ctrl = 0;
    }
    
    return status;
}

/* Internal function:
 * This function is used to process bus reset detection event
 * @core_handle: xDCI controller handle address
 */ 
static enum usb_status dwc_xdci_process_device_reset_det(
		struct usb_dcd_xdci_core_handle *core_handle)
{
	enum usb_status status = USB_OK;

    if (NULL == core_handle) {
        return USB_INVALID_HANDLE;
    }

	/* Flush all FIFOs */
	status = dwc_xdci_core_flush_all_fifos(core_handle);
	if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_device_reset_det: Failed to flush FIFOs"));
	}
	
	/* NOTE: Not treating flush FIFOs status to be fatal */

    /* Start SETUP phase on EP0 */
    status = dwc_xdci_core_start_ep0_setup_xfer(core_handle);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_device_reset_det: Failed to start SETUP phase for EP0"));
    	return status;
    }

    /* Notify upper layer if a callback is registerd for
     *  this event
     */
	if (core_handle->event_callbacks.dev_bus_reset_callback) {
		core_handle->event_callbacks.cb_event_params.
		    parent_handle = core_handle->parent_handle;

		status =
			core_handle->event_callbacks.dev_bus_reset_callback(
			&core_handle->event_callbacks.cb_event_params);
	}
	
	return status;
}

/* Internal function:
 * This function is used to process connection done (means reset
 * complete) event
 * @core_handle: xDCI controller handle address
 */ 
static enum usb_status dwc_xdci_process_device_reset_done(
		struct usb_dcd_xdci_core_handle *core_handle)
{
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	uint32_t base_addr;
	enum usb_status status = USB_OK;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_device_reset_done: INVALID handle"));
		return USB_INVALID_HANDLE;
    }

    base_addr = core_handle->base_address;
    core_handle->actual_speed = (usb_reg_read(base_addr,
    		                      DWC_XDCI_DSTS_REG) &
    		                        DWC_XDCI_DSTS_CONN_SPEED_MASK);

    /* Program MPS based on the negotiated speed */
    dwc_xdci_core_get_ctrl_mps(core_handle,
    		&core_handle->ep_handles[0].ep_info.max_pkt_size);
    dwc_xdci_core_get_ctrl_mps(core_handle,
    		&core_handle->ep_handles[1].ep_info.max_pkt_size);

    /* Init DEPCFG cmd params for EP0 */
    status = dwc_xdci_core_init_ep_cmd_params(
    		core_handle, &core_handle->ep_handles[0].ep_info,
        		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_MDFY_STATE,
        		EPCMD_SET_EP_CONFIG,
        		&ep_cmd_params);

    if (status)
        	return status;

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(core_handle,
    		0, EPCMD_SET_EP_CONFIG, 
    		&ep_cmd_params);

    if (status)
    	return status;

    /* Init DEPCFG cmd params for EP1 */
    status = dwc_xdci_core_init_ep_cmd_params(
    		core_handle, &core_handle->ep_handles[1].ep_info,
    		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_MDFY_STATE,
    		EPCMD_SET_EP_CONFIG,
    		&ep_cmd_params);

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(core_handle,
    		1, EPCMD_SET_EP_CONFIG, 
    		&ep_cmd_params);

    /* Put the other PHY into suspend */
    if (core_handle->actual_speed == USB_SPEED_SUPER) {
    	/* Put HS PHY to suspend */
        usb_reg_write(base_addr, DWC_XDCI_GUSB2PHYCFG_REG(0),
    	    (usb_reg_read(base_addr, DWC_XDCI_GUSB2PHYCFG_REG(0)) | 
            DWC_XDCI_GUSB2PHYCFG_SUSPEND_PHY_MASK));
   

        /* Clear SS PHY's suspend mask */
    	usb_reg_write(base_addr, DWC_XDCI_GUSB3PIPECTL_REG(0),
    	    	    (usb_reg_read(base_addr,
    	    	    	DWC_XDCI_GUSB3PIPECTL_REG(0)) & 
    	    	    	~DWC_XDCI_GUSB3PIPECTL_SUSPEND_PHY_MASK));

    } else {
    	/* Put SS PHY to suspend */
    	usb_reg_write(base_addr, DWC_XDCI_GUSB3PIPECTL_REG(0),
    	    	    (usb_reg_read(base_addr,
    	    	    	DWC_XDCI_GUSB3PIPECTL_REG(0)) | 
    	    	    	DWC_XDCI_GUSB3PIPECTL_SUSPEND_PHY_MASK));
    	
    	/* Clear HS PHY's suspend mask */
        usb_reg_write(base_addr, DWC_XDCI_GUSB2PHYCFG_REG(0),
    	    (usb_reg_read(base_addr,
    	    		DWC_XDCI_GUSB2PHYCFG_REG(0)) & 
                    ~DWC_XDCI_GUSB2PHYCFG_SUSPEND_PHY_MASK));
    }

    /* Notify upper layer if callback is registered */
    if (core_handle->event_callbacks.dev_reset_done_callback) {
        core_handle->event_callbacks.cb_event_params.
            parent_handle = core_handle->parent_handle; 
        core_handle->event_callbacks.cb_event_params.
                    speed = core_handle->actual_speed;

		core_handle->event_callbacks.
		  dev_reset_done_callback(
		    &core_handle->event_callbacks.cb_event_params);
    }

   	return status;
}

/* Internal function:
 * This function is used to process device event
 * @core_handle: xDCI controller handle address
 * @int_line_event_buffer: event buffer pointing to device event
 * @processed_event_size: address of variable to save the size of
 * the event that was processed
 */ 
static enum usb_status dwc_xdci_process_device_event(
		struct usb_dcd_xdci_core_handle *core_handle,
		struct dwc_xdci_evt_buff *int_line_event_buffer,
		uint32_t *processed_event_size)
{
	uint32_t event;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_device_event: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

	/* Extract device event */
	event = (int_line_event_buffer->event &
			  DWC_XDCI_EVENT_BUFF_DEV_EVT_MASK);
	event >>= DWC_XDCI_EVENT_BUFF_DEV_EVT_BIT_POS;

	/* Assume default event size. Change it in switch case if
	 *  different
	 */
	*processed_event_size =
			DWC_XDCI_DEV_EVENT_DEFAULT_SIZE_IN_BYTES;
	
	switch (event) {
	case DWC_XDCI_EVENT_BUFF_DEV_DISCONN_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_DISCONN_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_USB_RESET_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_USB_RESET_EVENT"));
		dwc_xdci_process_device_reset_det(core_handle);
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_CONN_DONE_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_CONN_DONE_EVENT"));
		dwc_xdci_process_device_reset_done(core_handle); 
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_STATE_CHANGE_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_STATE_CHANGE_EVENT"));
		dwc_xdci_process_device_state_change_event(core_handle,
				int_line_event_buffer->event);
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_WKUP_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_WKUP_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_HBRNTN_REQ_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_HBRNTN_REQ_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_SOF_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_SOF_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_ERRATIC_ERR_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_ERRATIC_ERR_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_CMD_CMPLT_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_CMD_CMPLT_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_BUFF_OVFL_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_BUFF_OVFL_EVENT"));
		break;
	case DWC_XDCI_EVENT_BUFF_DEV_TST_LMP_RX_EVENT:
		DEBUG ((DEBUG_INFO, "\nDevice DWC_XDCI_EVENT_BUFF_DEV_TST_LMP_RX_EVENT"));
		*processed_event_size =
				DWC_XDCI_DEV_EVENT_TST_LMP_SIZE_IN_BYTES;
		break;
	default:
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_device_event: UNHANDLED device event: %x", event));
		break;
	}

	return USB_OK;
}

/* Internal function:
 * This function is used to process EP not ready for
 * non-control endpoints
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical endpoint number
 */ 
static enum usb_status dwc_xdci_process_ep_xfer_not_ready(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num)
{
	/* TODO: Not doing on-demand transfers
	 * Revisit if required for later use
	 */
	return USB_OK;
}

/* Internal function:
 * This function is used to process EP not ready for
 * control endpoints
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical endpoint number
 * @data_stage: EP not ready when data stage token was received
 * @status_stage: EP not ready when status stage token was received
 */ 
static enum usb_status dwc_xdci_process_ep0_xfer_not_ready(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num, enum usb_ep_state ep_state)
{
    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_ep0_xfer_not_ready: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

	/* Notify upper layer if a callback is registered for
	 * this event
	 */	
	if (core_handle->event_callbacks.
			dev_xfer_nrdy_callback) {
		core_handle->event_callbacks.cb_event_params.parent_handle =
			core_handle->parent_handle;
		core_handle->event_callbacks.cb_event_params.ep_state =
			ep_state;

	    core_handle->event_callbacks.
	    	dev_xfer_nrdy_callback(
	    		&core_handle->event_callbacks.cb_event_params);
	}

	return USB_OK;
}

/* Internal function:
 * This function is used to process transfer phone done for EP0
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical endpoint number (0 for OUT and 1 for IN)
 */ 
static enum usb_status dwc_xdci_process_ep0_xfer_phase_done(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num)
{
	struct dwc_xdci_ep *ep_handle;
	struct dwc_xdci_trb *trb;
	enum usb_status status = USB_OK;
	uint32_t trb_sts;
	uint32_t trb_ctrl;
	
    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_ep0_xfer_phase_done: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

	ep_handle = &core_handle->ep_handles[ep_num];

    trb = core_handle->trbs + ep_num;
    
    if (trb->trb_ctrl & DWC_XDCI_TRB_CTRL_HWO_MASK) {
        DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_ep0_xfer_phase_done. HW owns TRB: %x!!!", (uint32_t)(UINTN)trb));
    }

	ep_handle->currentXferRscIdx = 0;

	trb_ctrl = ((trb->trb_ctrl &
			       DWC_XDCI_TRB_CTRL_TYPE_MASK) >>
			       DWC_XDCI_TRB_CTRL_TYPE_BIT_POS);

	trb_sts = ((trb->len_xfer_params &
			    DWC_XDCI_TRB_STATUS_MASK) >>
			      DWC_XDCI_TRB_STATUS_BIT_POS);

	switch (trb_ctrl) {
	case DWC_XDCI_TRB_CTRL_TYPE_SETUP:
		if (core_handle->event_callbacks.
				dev_setup_pkt_received_callback) {

			core_handle->event_callbacks.cb_event_params.
				parent_handle = core_handle->parent_handle;
			core_handle->event_callbacks.cb_event_params.buffer =
					core_handle->aligned_setup_buffer;
			
			status = core_handle->event_callbacks.
			  dev_setup_pkt_received_callback(
			    &core_handle->event_callbacks.cb_event_params);
		}
		
		if (core_handle->aligned_setup_buffer[0] &
				USB_SETUP_DATA_PHASE_DIRECTION_MASK) {
		    /* Keep a buffer ready for status phase */
		    dwc_xdci_ep0_receive_status_pkt(core_handle);
		} else {
			/* Keep a buffer ready for setup phase */
			dwc_xdci_core_start_ep0_setup_xfer(core_handle);
		}

        break;
    case DWC_XDCI_TRB_CTRL_TYPE_STATUS2:
        break;
    case DWC_XDCI_TRB_CTRL_TYPE_STATUS3:
        /* Notify upper layer of control transfer completion
         * if a callback function was registerd
         */
        if (core_handle->event_callbacks.dev_xfer_done_callback) {
            core_handle->event_callbacks.cb_event_params.
                parent_handle = core_handle->parent_handle;
            core_handle->event_callbacks.cb_event_params.
                ep_num = (ep_num >> 1);
            core_handle->event_callbacks.cb_event_params.
                ep_dir = (ep_num & 1);
            core_handle->event_callbacks.cb_event_params.
                buffer = (uint8_t *)(UINTN)(trb->buff_ptr_low);
            core_handle->event_callbacks.
                dev_xfer_done_callback(
                        &core_handle->event_callbacks.
                        cb_event_params);
        }

		/* Status phase done. Queue next SETUP packet */
		status = dwc_xdci_core_start_ep0_setup_xfer(core_handle);

        if (status) {
            DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_ep0_xfer_phase_done: FAILED to queue SETUP"));
        }
        break;
    case DWC_XDCI_TRB_CTRL_TYPE_DATA:
        if (core_handle->event_callbacks.dev_xfer_done_callback) {
            core_handle->event_callbacks.cb_event_params.
                parent_handle = core_handle->parent_handle;
            core_handle->event_callbacks.cb_event_params.
                ep_num = (ep_num >> 1);
            core_handle->event_callbacks.cb_event_params.
                ep_dir = (ep_num & 1);
            core_handle->event_callbacks.cb_event_params.
                buffer = (uint8_t *)(UINTN)(trb->buff_ptr_low);
            core_handle->event_callbacks.
                dev_xfer_done_callback(
                        &core_handle->event_callbacks.
                        cb_event_params);
        }
        break;
    default:
        DEBUG ((DEBUG_INFO, "\nndwc_xdci_process_ep0_xfer_phase_done: UNHANDLED STATE in TRB"));
        break;
    }

	return status;
}

/* Internal function:
 * This function is used to process transfer done for 
 * non-control endpoints
 * @core_handle: xDCI controller handle address
 * @ep_num: Physical endpoint number
 */ 
static enum usb_status dwc_xdci_process_ep_xfer_done(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t ep_num)
{
	struct dwc_xdci_ep *ep_handle;
	struct dwc_xdci_trb *trb;
	struct usb_xfer_request *xfer_req;
	uint32_t remaining_len;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_ep_xfer_done: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

	ep_handle = &core_handle->ep_handles[ep_num];
	ep_handle->currentXferRscIdx = 0;
	trb = ep_handle->trb;
	xfer_req = &ep_handle->xfer_handle;
	
	if ((NULL == trb) || (NULL == xfer_req)) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_ep_xfer_done: INVALID parameter"));

		return USB_INVALID_PARAM;
	}

	/* Compute the actual transfer length */
	xfer_req->actual_xfer_len = xfer_req->xfer_len;
	remaining_len = (trb->len_xfer_params &
                        DWC_XDCI_TRB_BUFF_SIZE_MASK);

	if (remaining_len > xfer_req->xfer_len) {
		/* Buffer overrun? This should never happen */
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_ep_xfer_done: Possible buffer overrun"));		
	} else {
		xfer_req->actual_xfer_len -= remaining_len; 
	}

	/* Notify upper layer of request-specific transfer completion
	 * if there is a callback specifically for this request
	 */
	if (xfer_req->xfer_done) {
		xfer_req->xfer_done(core_handle->parent_handle, xfer_req);
	}

    /* Notify upper layer if a callback was registered */
    if (core_handle->event_callbacks.dev_xfer_done_callback) {
        core_handle->event_callbacks.cb_event_params.
            parent_handle = core_handle->parent_handle;
        core_handle->event_callbacks.cb_event_params.
            ep_num = (ep_num >> 1);
        core_handle->event_callbacks.cb_event_params.
            ep_dir = (ep_num & 1);
        core_handle->event_callbacks.cb_event_params.
            ep_type = ep_handle->ep_info.ep_type;
        core_handle->event_callbacks.cb_event_params.
            buffer = (uint8_t *)(UINTN)(ep_handle->trb->buff_ptr_low);

		core_handle->event_callbacks.dev_xfer_done_callback(
			&core_handle->event_callbacks.cb_event_params);
	}

	return USB_OK;
}

/* Internal function:
 * This function is used to process endpoint events
 * @core_handle: xDCI controller handle address
 * @int_line_event_buffer:  address of buffer containing event
 * to process
 * @processed_event_size: address to save the size of event
 * processed
 */ 
static enum usb_status dwc_xdci_process_ep_event(
		struct usb_dcd_xdci_core_handle *core_handle,
		struct dwc_xdci_evt_buff *int_line_event_buffer,
		uint32_t *processed_event_size)
{
	uint32_t ep_num;
	uint32_t ep_event;
	uint32_t ep_event_status;
	enum usb_ep_state ep_state = USB_EP_STATE_DATA;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_ep_event: INVALID handle"));

		return USB_INVALID_HANDLE;
    }

	ep_event = int_line_event_buffer->event;

	*processed_event_size =
			DWC_XDCI_DEV_EVENT_DEFAULT_SIZE_IN_BYTES;

	/* Get EP num */
	ep_num = ((ep_event & DWC_XDCI_EVENT_BUFF_EP_NUM_MASK) >>
			  DWC_XDCI_EVENT_BUFF_EP_NUM_BIT_POS);

	ep_event_status = (ep_event &
			           DWC_XDCI_EVENT_BUFF_EP_EVENT_STATUS_MASK);

	/* Interpret event and handle transfer completion here */
	ep_event = ((ep_event & DWC_XDCI_EVENT_BUFF_EP_EVENT_MASK) >>
			DWC_XDCI_EVENT_BUFF_EP_EVENT_BIT_POS);
	
	switch (ep_event) {
	case DWC_XDCI_EVENT_BUFF_EP_XFER_CMPLT:
		if (ep_num > 1) {
			dwc_xdci_process_ep_xfer_done(core_handle,
					ep_num);
		} else {
			dwc_xdci_process_ep0_xfer_phase_done(core_handle,
					ep_num);
		}
		break;
	case DWC_XDCI_EVENT_BUFF_EP_XFER_IN_PROGRESS:
		break;
	case DWC_XDCI_EVENT_BUFF_EP_XFER_NOT_READY:
		if (ep_num > 1) {
			/* Endpoint transfer is not ready */
			dwc_xdci_process_ep_xfer_not_ready(core_handle, ep_num);
		} else {
			/* Is it data stage or status stage */
			if (ep_event_status &
				  DWC_XDCI_EVENT_BUFF_EP_CTRL_DATA_REQ_MASK) {
				ep_state = USB_EP_STATE_DATA;
			} else if (ep_event_status &
				  DWC_XDCI_EVENT_BUFF_EP_CTRL_STATUS_REQ_MASK) {
				ep_state = USB_EP_STATE_STATUS;
			}

			/* Process transfer not ready case */
		    dwc_xdci_process_ep0_xfer_not_ready(core_handle,
		    		ep_num, ep_state);
		}
		break;
	default:
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_ep_event: UNKNOWN EP event"));
		break;
	}

	return USB_OK;
}

/* Internal function:
 * This function is used to process events on single interrupt line
 * @core_handle: xDCI controller handle address
 * @event_count:  event bytes to process
 * @processed_event_count: address to save the size
 * (in bytes) of event processed
 * processed
 */ 
static enum usb_status dwc_xdci_process_interrupt_line_events(
		struct usb_dcd_xdci_core_handle *core_handle,
		uint32_t event_count,
		uint32_t *processed_event_count)
{
	uint32_t processed_event_size = 0;
	uint32_t current_event_addr;

    if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_interrupt_line_events: INVALID handle"));

		return USB_INVALID_HANDLE;
    }
    
    if (NULL == core_handle->current_event_buffer) {
		DEBUG ((DEBUG_INFO, "\nERROR: dwc_xdci_process_interrupt_line_events: INVALID event buffer"));

		return USB_INVALID_PARAM;
    }
    
    current_event_addr =
                (uint32_t)(UINTN)(core_handle->current_event_buffer);

	/* Process event_count/event_size number of events
	 * in this run
	 */
    while (event_count) {
    	if (core_handle->current_event_buffer->event &
    			DWC_XDCI_EVENT_DEV_MASK) {
    		dwc_xdci_process_device_event(core_handle,
    				core_handle->current_event_buffer,
    				&processed_event_size);
    	} else {
    		dwc_xdci_process_ep_event(core_handle,
    				core_handle->current_event_buffer,
    				&processed_event_size);
    	}

        event_count -= processed_event_size;
        *processed_event_count += processed_event_size;
        if ((current_event_addr + processed_event_size) >=
              ((uint32_t)(UINTN)(core_handle->aligned_event_buffers) +
              (sizeof(struct dwc_xdci_evt_buff)*
                DWC_XDCI_MAX_EVENTS_PER_BUFFER))) {
            current_event_addr =
                    (uint32_t)(UINTN)(core_handle->aligned_event_buffers);
            DEBUG ((DEBUG_INFO, "\ndwc_xdci_process_interrupt_line_events: Event buffer bound reached"));
    	} else {
    	    current_event_addr += processed_event_size;
    	}

        core_handle->current_event_buffer =
                (struct dwc_xdci_evt_buff *)(UINTN)current_event_addr;
    }

	return USB_OK;
}

/* DWC XDCI APIs */

/* Interface:
 * 
 * This function is used to initialize the xDCI core
 * @config_params: Parameters from app to configure the core
 * @device_core_ptr:  HW-independent APIs handle for device core
 * @core_handle: xDCI controller handle retured
 */ 
enum usb_status dwc_xdci_core_init(
		struct usb_dev_config_params *config_params,
		void *device_core_ptr,
        void **core_handle)
{
	enum usb_status status = USB_INVALID_OP;
	uint32_t base_addr;
	struct usb_dcd_xdci_core_handle *local_core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	uint32_t max_delay_iter =
			DWC_XDCI_MAX_DELAY_ITERATIONS;
	uint8_t i;
	
	local_core_handle =
			(struct usb_dcd_xdci_core_handle *)
			  pmalloc(sizeof(struct usb_dcd_xdci_core_handle));

	if (NULL == core_handle) {
		return USB_INVALID_PARAM;
	}

    if (NULL == local_core_handle) {
        DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to allocate handle for xDCI"));
        return USB_OUT_OF_MEMORY;
    }
    
    //memset(local_core_handle, 0,
    //       sizeof(struct usb_dcd_xdci_core_handle));
    ZeroMem(local_core_handle, sizeof(struct usb_dcd_xdci_core_handle));

	local_core_handle->parent_handle = device_core_ptr;
	
    *core_handle = (void *)local_core_handle;

	local_core_handle->id = config_params->id;
	local_core_handle->base_address = base_addr = 
			config_params->base_address;
	local_core_handle->flags = config_params->flags;
	local_core_handle->desired_speed =
			local_core_handle->actual_speed =
					config_params->speed;
	local_core_handle->role = config_params->role;

    DEBUG ((DEBUG_INFO, "\nResetting the USB core"));
    usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
        usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) |
            DWC_XDCI_DCTL_CSFTRST_MASK);

    /* Wait until core soft reset completes */
    do {
    	if (!(usb_reg_read(base_addr,
                DWC_XDCI_DCTL_REG) &
                DWC_XDCI_DCTL_CSFTRST_MASK))
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\nFailed to reset device controller"));
    	return USB_OP_FAILED;
    }

    DEBUG ((DEBUG_INFO, "\nUSB core has been reset"));

    /* All FIFOs are flushed at this point */
    
    /* Ensure we have EP0 Rx/Tx handles initialized */
    local_core_handle->ep_handles[0].ep_info.ep_num = 0;
    local_core_handle->ep_handles[0].ep_info.ep_dir =
    		USB_EP_DIR_OUT;
    local_core_handle->ep_handles[0].ep_info.ep_type =
    		USB_EP_TYPE_CONTROL;
    local_core_handle->ep_handles[0].ep_info.max_pkt_size =
    		DWC_XDCI_SS_CTRL_EP_MPS;
    /* 0 means burst size of 1 */
    local_core_handle->ep_handles[0].ep_info.burst_size = 0;

    local_core_handle->ep_handles[1].ep_info.ep_num = 0;
    local_core_handle->ep_handles[1].ep_info.ep_dir =
    		USB_EP_DIR_IN;
    local_core_handle->ep_handles[1].ep_info.ep_type =
    		USB_EP_TYPE_CONTROL;
    local_core_handle->ep_handles[1].ep_info.max_pkt_size =
    		DWC_XDCI_SS_CTRL_EP_MPS;
    /* 0 means burst size of 1 */
    local_core_handle->ep_handles[1].ep_info.burst_size = 0;

    local_core_handle->dev_state = USB_DEVICE_STATE_DEFAULT;

	/* Clear KeepConnect bit so we can allow disconnect and
	 * re-connect. Stay in RX_DETECT state
	 */
	usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
		    		usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) &
		    		   (~DWC_XDCI_DCTL_KEEP_CONNECT_MASK) &
		    		   (~DWC_XDCI_DCTL_STATE_CHANGE_REQ_MASK) |
		    		   (DWC_XDCI_DCTL_STATE_CHANGE_REQ_RX_DETECT <<
		    			  DWC_XDCI_DCTL_STATE_CHANGE_REQ_BIT_POS));

    DEBUG ((DEBUG_INFO, "\nDevice controller Synopsys ID: %x", usb_reg_read(base_addr, DWC_XDCI_GSNPSID_REG)));

    DEBUG ((DEBUG_INFO, "\nDefault value of xDCI GSBUSCFG0 and GSBUSCFG1: %x, %x",
    		usb_reg_read(base_addr,
    				DWC_XDCI_GSBUSCFG0_REG),
    		usb_reg_read(base_addr,
    				DWC_XDCI_GSBUSCFG1_REG)));    

    DEBUG ((DEBUG_INFO, "\nDefault value of xDCI GTXTHRCFG and GRXTHRCFG: %x, %x",
    		usb_reg_read(base_addr,
    				DWC_XDCI_GTXTHRCFG_REG),
    		usb_reg_read(base_addr,
    				DWC_XDCI_GRXTHRCFG_REG)));

	/* Clear ULPI auto-resume bit */
	usb_reg_write(base_addr, DWC_XDCI_GUSB2PHYCFG_REG(0),
		    (usb_reg_read(base_addr, DWC_XDCI_GUSB2PHYCFG_REG(0)) &
		    	~DWC_XDCI_GUSB2PHYCFG_ULPI_AUTO_RESUME_MASK));

    DEBUG ((DEBUG_INFO, "\nDefault value of xDCI GUSB2PHYCFG and"
    		" GUSB3PIPECTL: %x, %x",
    		usb_reg_read(base_addr,
    				DWC_XDCI_GUSB2PHYCFG_REG(0)),
    		usb_reg_read(base_addr,
    				DWC_XDCI_GUSB3PIPECTL_REG(0))));

    /* Only one RxFIFO */
    DEBUG ((DEBUG_INFO, "Default value of DWC_XDCI_GRXFIFOSIZ: %x ",
    		usb_reg_read(base_addr, DWC_XDCI_GRXFIFOSIZ_REG(0))));

    for (i=0; i<DWC_XDCI_MAX_ENDPOINTS; i++) {
        DEBUG ((DEBUG_INFO, "\nDefault value of xDCI DWC_XDCI_GTXFIFOSIZ %d: %x",
    			    i, usb_reg_read(base_addr,
    	    				DWC_XDCI_GTXFIFOSIZ_REG(i))));
    }
    
    /* TODO: Need to check if TxFIFO should start where RxFIFO ends
     * or default is correct i.e. TxFIFO starts at 0 just like RxFIFO
     */

    /* Allocate and Initialize Event Buffers */
    local_core_handle->max_dev_int_lines =
    		((usb_reg_read(base_addr,
    		DWC_XDCI_GHWPARAMS1_REG) &
    		  DWC_XDCI_GHWPARAMS1_NUM_INT_MASK) >>
    				  DWC_XDCI_GHWPARAMS1_NUM_INT_BIT_POS);

    DEBUG ((DEBUG_INFO, "\nMax dev int lines: %d", local_core_handle->max_dev_int_lines));
    
    /* One event buffer per interrupt line.
     *  Need to align it to size of event buffer
     *  Buffer needs to be big enough. Otherwise the core
     *  won't operate  
     */   
    local_core_handle->aligned_event_buffers =
        (struct dwc_xdci_evt_buff *)
          ((uint32_t)(UINTN)(local_core_handle->event_buffers) +
            ((sizeof(struct dwc_xdci_evt_buff)*
                    DWC_XDCI_MAX_EVENTS_PER_BUFFER) - 
                (((uint32_t)(UINTN)(local_core_handle->event_buffers)) %
                (sizeof(struct dwc_xdci_evt_buff)*
                        DWC_XDCI_MAX_EVENTS_PER_BUFFER))));

    for (i=0; i < local_core_handle->max_dev_int_lines; i++) {
        usb_reg_write(base_addr, DWC_XDCI_GEVNTADR_REG(i),
            (uint32_t)(UINTN)(local_core_handle->aligned_event_buffers +
                    i*sizeof(struct dwc_xdci_evt_buff)*
                    DWC_XDCI_MAX_EVENTS_PER_BUFFER));

    	local_core_handle->current_event_buffer =
    			local_core_handle->aligned_event_buffers;
    	/* Write size and clear the mask */
    	usb_reg_write(base_addr, DWC_XDCI_EVNTSIZ_REG(i),
    			sizeof(struct dwc_xdci_evt_buff)*
    			DWC_XDCI_MAX_EVENTS_PER_BUFFER);

    	/* Write 0 to the event count register as the last step
    	 *  for event configuration
    	 */
        usb_reg_write(base_addr, DWC_XDCI_EVNTCOUNT_REG(i), 0);

        DEBUG ((DEBUG_INFO, "\nValue of xDCI Event buffer %d: %x, Size: %x, Count: %x",
            			    i, usb_reg_read(base_addr,
            			    		DWC_XDCI_GEVNTADR_REG(i)),
            	    		usb_reg_read(base_addr,
            	    				DWC_XDCI_EVNTSIZ_REG(i)),
        					usb_reg_read(base_addr,
        							DWC_XDCI_EVNTCOUNT_REG(i))));        
    }

    /* Program Global Control Register to disable scaledown,
     * disable clock gating
     */
    usb_reg_write(base_addr, DWC_XDCI_GCTL_REG,
    		((usb_reg_read(base_addr, DWC_XDCI_GCTL_REG) &
    		  ~DWC_XDCI_GCTL_SCALE_DOWN_MODE_MASK) |
    				DWC_XDCI_GCTL_DISABLE_CLK_GATING_MASK));

    /* TODO: Program desired speed and set LPM capable
     * We will do this when Superspeed works. For now,
     * force into High-speed mode to avoid anyone trying this
     * on Super speed port
     */
#if 0
    usb_reg_write(base_addr, DWC_XDCI_DCFG_REG,
    		(usb_reg_read(base_addr, DWC_XDCI_DCFG_REG) & 
    				~DWC_XDCI_DCFG_DESIRED_DEV_SPEED_MASK) |
    				local_core_handle->desired_speed);
#else
    usb_reg_write(base_addr, DWC_XDCI_DCFG_REG,
        		(usb_reg_read(base_addr, DWC_XDCI_DCFG_REG) & 
        				~DWC_XDCI_DCFG_DESIRED_DEV_SPEED_MASK) |
        				DWC_XDCI_DCFG_DESIRED_HS_SPEED);
#endif

    /* Enable Device Interrupt Events */
    usb_reg_write(base_addr, DWC_XDCI_DEVTEN_REG,
    		DWC_XDCI_DEVTEN_DEVICE_INTS);

    /* Program the desired role */
    usb_reg_write(base_addr, DWC_XDCI_GCTL_REG,
    		(usb_reg_read(base_addr, DWC_XDCI_GCTL_REG) &
    		   ~DWC_XDCI_GCTL_PRT_CAP_DIR_MASK) |
    		     (local_core_handle->role <<
    		       DWC_XDCI_GCTL_PRT_CAP_DIR_BIT_POS));

    /* Clear USB2 suspend for start new config command */
    usb_reg_write(base_addr, DWC_XDCI_GUSB2PHYCFG_REG(0),
    		    (usb_reg_read(base_addr,
    		    	DWC_XDCI_GUSB2PHYCFG_REG(0)) &
    		    	   ~DWC_XDCI_GUSB2PHYCFG_SUSPEND_PHY_MASK));

    /* Clear USB3 suspend for start new config command */
    usb_reg_write(base_addr, DWC_XDCI_GUSB3PIPECTL_REG(0),
    		(usb_reg_read(base_addr, DWC_XDCI_GUSB3PIPECTL_REG(0)) &
    		    ~DWC_XDCI_GUSB3PIPECTL_SUSPEND_PHY_MASK));
  
    /* Issue DEPSTARTCFG command for EP0 */
    status = dwc_xdci_core_init_ep_cmd_params(
    		local_core_handle,
    		&local_core_handle->ep_handles[0].ep_info,
    		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_NONE,
    		EPCMD_START_NEW_CONFIG,
    		&ep_cmd_params);

    if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to init params for START_NEW_CONFIG EP command on xDCI"));
    	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		0, EPCMD_START_NEW_CONFIG, 
    		&ep_cmd_params);

    if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to issue START_NEW_CONFIG EP command on xDCI"));
    	return status;
    }
    
    /* Issue DEPCFG command for EP0 */
    status = dwc_xdci_core_init_ep_cmd_params(
    		local_core_handle,
    		&local_core_handle->ep_handles[0].ep_info,
    		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_INIT_STATE,
    		EPCMD_SET_EP_CONFIG,
    		&ep_cmd_params);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to init params for SET_EP_CONFIG command on xDCI for EP0"));
    	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		0, EPCMD_SET_EP_CONFIG, 
    		&ep_cmd_params);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to issue SET_EP_CONFIG command on xDCI for EP0"));
    	return status;
    }

    /* Issue DEPCFG command for EP1 */
    status = dwc_xdci_core_init_ep_cmd_params(
    		local_core_handle,
    		&local_core_handle->ep_handles[1].ep_info,
    		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_INIT_STATE,
    		EPCMD_SET_EP_CONFIG,
    		&ep_cmd_params);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to init params for SET_EP_CONFIG command on xDCI for EP1"));
    	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		1, EPCMD_SET_EP_CONFIG, 
    		&ep_cmd_params);

    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to issue SET_EP_CONFIG command on xDCI for EP1"));
    	return status;
    }
    
    /* Issue DEPXFERCFG command for EP0 */
    status = dwc_xdci_core_init_ep_cmd_params(
        		local_core_handle,
        		&local_core_handle->ep_handles[0].ep_info,
        		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_NONE,
        		EPCMD_SET_EP_XFER_RES_CONFIG,
        		&ep_cmd_params);
        
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to init params for EPCMD_SET_EP_XFER_RES_CONFIG command on xDCI for EP0"));
    	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		0, EPCMD_SET_EP_XFER_RES_CONFIG, 
    		&ep_cmd_params);

    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to issue EPCMD_SET_EP_XFER_RES_CONFIG command on xDCI for EP0"));
    	return status;
    }

    /* Issue DEPXFERCFG command for EP1 */
    status = dwc_xdci_core_init_ep_cmd_params(
           		local_core_handle,
           		&local_core_handle->ep_handles[1].ep_info,
           		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_NONE,
           		EPCMD_SET_EP_XFER_RES_CONFIG,
           		&ep_cmd_params);
            
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to init params for EPCMD_SET_EP_XFER_RES_CONFIG command on xDCI for EP1"));
       	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		1, EPCMD_SET_EP_XFER_RES_CONFIG, 
    		&ep_cmd_params);

    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_init: Failed to issue EPCMD_SET_EP_XFER_RES_CONFIG command on xDCI for EP1"));
    	return status;
    }
  
    /* Prepare a buffer for SETUP packet */
    local_core_handle->trbs =
        (struct dwc_xdci_trb *)(UINTN)((uint32_t)(UINTN)
            local_core_handle->unaligned_trbs +
                (DWC_XDCI_TRB_BYTE_ALIGNMENT -
                    ((uint32_t)(UINTN)local_core_handle->unaligned_trbs %
                        DWC_XDCI_TRB_BYTE_ALIGNMENT)));
       
    /* Allocate Setup buffer that is 8-byte aligned */
    local_core_handle->aligned_setup_buffer =
            local_core_handle->default_setup_buffer +
                   (DWC_XDCI_SETUP_BUFF_SIZE -
                    ((uint32_t)(UINTN)(local_core_handle->
                            default_setup_buffer) %
                            DWC_XDCI_SETUP_BUFF_SIZE));   

    /* Aligned buffer for status phase */
    local_core_handle->aligned_misc_buffer =
            local_core_handle->misc_buffer +
            (DWC_XDCI_SETUP_BUFF_SIZE -
                ((uint32_t)(UINTN)(local_core_handle->aligned_misc_buffer) %
                 DWC_XDCI_SETUP_BUFF_SIZE));
    
    /* We will queue SETUP request when we see bus reset */
 
    /* Enable Physical Endpoints 0 */
    usb_reg_write(base_addr, DWC_XDCI_EP_DALEPENA_REG,
    		usb_reg_read(base_addr, DWC_XDCI_EP_DALEPENA_REG) |
    		  (1 << 0));
    
    /* Enable Physical Endpoints 1 */
    usb_reg_write(base_addr, DWC_XDCI_EP_DALEPENA_REG,
        		usb_reg_read(base_addr, DWC_XDCI_EP_DALEPENA_REG) |
        		  (1 << 1));

    return status;
}

/* Interface:
 * This function is used to de-initialize the xDCI core
 * @core_handle: xDCI controller handle
 * @flags: Special flags for de-initializing the core in
 * particular way
 */ 
enum usb_status dwc_xdci_core_deinit(void *core_handle,
		uint32_t flags)
{
	/* TODO: Need to implement this */
    return USB_OK;
}

/* Interface:
 * This function is used to register event callback function
 * @core_handle: xDCI controller handle
 * @event: Event for which callback is to be registered
 * @callback_fn: Callback function to invoke after event occurs
 */
enum usb_status dwc_xdci_core_register_callback(void *core_handle,
		enum usb_device_event_id event,
		enum usb_status (*callback_fn)(
			struct usb_device_callback_param *cb_event_params))
{

	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;

	if (NULL == local_core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_register_callback: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	switch (event) {
	case USB_DEVICE_DISCONNECT_EVENT:
		local_core_handle->event_callbacks.dev_disconnect_callback =
				callback_fn;
		break;
	case USB_DEVICE_RESET_EVENT:
		local_core_handle->event_callbacks.dev_bus_reset_callback =
				callback_fn;
		break;
	case USB_DEVICE_CONNECTION_DONE:
		local_core_handle->event_callbacks.dev_reset_done_callback =
				callback_fn;
		break;
	case USB_DEVICE_STATE_CHANGE_EVENT:
		local_core_handle->event_callbacks.dev_link_state_callback =
				callback_fn;
		break;
	case USB_DEVICE_WAKEUP_EVENT:
		local_core_handle->event_callbacks.dev_wakeup_callback =
				callback_fn;
		break;
	case USB_DEVICE_HIBERNATION_REQ_EVENT:
		local_core_handle->event_callbacks.dev_hibernation_callback =
				callback_fn;
		break;
	case USB_DEVICE_SOF_EVENT:
		local_core_handle->event_callbacks.dev_sof_callback =
				callback_fn;
		break;
	case USB_DEVICE_ERRATIC_ERR_EVENT:
		local_core_handle->event_callbacks.dev_erratic_err_callback =
				callback_fn;
		break;
	case USB_DEVICE_CMD_CMPLT_EVENT:
		local_core_handle->event_callbacks.dev_cmd_cmplt_callback =
				callback_fn;
		break;
	case USB_DEVICE_BUFF_OVERFLOW_EVENT:
		local_core_handle->event_callbacks.dev_buff_ovflw_callback =
				callback_fn;
		break;
	case USB_DEVICE_TEST_LMP_RX_EVENT:
		local_core_handle->event_callbacks.dev_test_lmp_rx_callback =
				callback_fn;
		break;
	case USB_DEVICE_SETUP_PKT_RECEIVED:
		local_core_handle->event_callbacks.dev_setup_pkt_received_callback =
				callback_fn;
		break;
	case USB_DEVICE_XFER_NRDY:
		local_core_handle->event_callbacks.dev_xfer_nrdy_callback =
				callback_fn;
		break;
	case USB_DEVICE_XFER_DONE:
		local_core_handle->event_callbacks.dev_xfer_done_callback =
				callback_fn;
		break;
	default:
		break;
	}

    return USB_OK;
}

/* Interface:
 * This function is used to unregister event callback function
 * @core_handle: xDCI controller handle
 * @event: Event for which callback function is to be unregistered
 */ 
enum usb_status dwc_xdci_core_unregister_callback(void *core_handle,
		enum usb_device_event_id event)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;

	if (NULL == local_core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_unregister_callback: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	switch (event) {
	case USB_DEVICE_DISCONNECT_EVENT:
		local_core_handle->event_callbacks.dev_disconnect_callback =
				NULL;
		break;
	case USB_DEVICE_RESET_EVENT:
		local_core_handle->event_callbacks.dev_bus_reset_callback =
				NULL;
		break;
	case USB_DEVICE_CONNECTION_DONE:
		local_core_handle->event_callbacks.dev_reset_done_callback =
				NULL;
		break;
	case USB_DEVICE_STATE_CHANGE_EVENT:
		local_core_handle->event_callbacks.dev_link_state_callback =
				NULL;
		break;
	case USB_DEVICE_WAKEUP_EVENT:
		local_core_handle->event_callbacks.dev_wakeup_callback =
				NULL;
		break;
	case USB_DEVICE_HIBERNATION_REQ_EVENT:
		local_core_handle->event_callbacks.dev_hibernation_callback =
				NULL;
		break;
	case USB_DEVICE_SOF_EVENT:
		local_core_handle->event_callbacks.dev_sof_callback =
				NULL;
		break;
	case USB_DEVICE_ERRATIC_ERR_EVENT:
		local_core_handle->event_callbacks.dev_erratic_err_callback =
				NULL;
		break;
	case USB_DEVICE_CMD_CMPLT_EVENT:
		local_core_handle->event_callbacks.dev_cmd_cmplt_callback =
				NULL;
		break;
	case USB_DEVICE_BUFF_OVERFLOW_EVENT:
		local_core_handle->event_callbacks.dev_buff_ovflw_callback =
				NULL;
		break;
	case USB_DEVICE_TEST_LMP_RX_EVENT:
		local_core_handle->event_callbacks.dev_test_lmp_rx_callback =
				NULL;
		break;
	case USB_DEVICE_SETUP_PKT_RECEIVED:
		local_core_handle->event_callbacks.dev_setup_pkt_received_callback =
				NULL;
		break;
	case USB_DEVICE_XFER_NRDY:
		local_core_handle->event_callbacks.dev_xfer_nrdy_callback =
				NULL;
		break;
	case USB_DEVICE_XFER_DONE:
		local_core_handle->event_callbacks.dev_xfer_done_callback =
				NULL;
		break;
	default:
		break;
	}

    return USB_OK;
}

/* Interface:
 * This function is used as an interrupt service routine
 * @core_handle: xDCI controller handle
 */ 
enum usb_status dwc_xdci_core_isr_routine(void *core_handle)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
			(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t base_addr;
	uint32_t event_count;
	uint32_t processed_event_count;
	uint32_t i;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_isr_routine: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	base_addr = local_core_handle->base_address;

	/* Event buffer corresponding to each interrupt line needs
	 * to be processed
	 */
	for (i = 0; i < local_core_handle->max_dev_int_lines; i++) {

		/* Get the number of events HW has written for this
		 *  interrupt line
		 */
		event_count = usb_reg_read(base_addr,
				DWC_XDCI_EVNTCOUNT_REG(i));
		event_count &= DWC_XDCI_EVNTCOUNT_MASK;
		processed_event_count = 0;

		/* Process interrupt line buffer only if count is non-zero */
		if (event_count) {
		    /* Process events in this buffer */
		    dwc_xdci_process_interrupt_line_events(local_core_handle,
				event_count, &processed_event_count);
		
		    /* Write back the processed number of events so HW decrements it from current
		     * event count 
		     */
		    usb_reg_write(base_addr, DWC_XDCI_EVNTCOUNT_REG(i),
				processed_event_count);

		}
	}
    return USB_OK;
}

/* Interface:
 * This function is used to enable xDCI to connect to the host
 * @core_handle: xDCI controller handle
 */ 
enum usb_status dwc_xdci_core_connect(void *core_handle)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
			(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t max_delay_iter =
			DWC_XDCI_MAX_DELAY_ITERATIONS;
	uint32_t base_addr; 

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_connect: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	base_addr =
			local_core_handle->base_address;

	/* Clear KeepConnect bit so we can allow disconnect and re-connect
	 * Also issue No action on state change to avoid any link change
	 */
	usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
		    		(usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) &
		    		   ~DWC_XDCI_DCTL_KEEP_CONNECT_MASK) &
		    		   ~DWC_XDCI_DCTL_STATE_CHANGE_REQ_MASK);

	/* Set Run bit to connect to the host */
	usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
	    		usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) |
	    				DWC_XDCI_DCTL_RUN_STOP_MASK);

    /* Wait until core starts running */
    do {
    	if (!(usb_reg_read(base_addr,
    			DWC_XDCI_DSTS_REG) &
    			DWC_XDCI_DSTS_DEV_CTRL_HALTED_MASK))
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\nFailed to run the device controller"));
    	return USB_OP_FAILED;
    }

    return USB_OK;
}

/* Interface:
 * This function is used to disconnect xDCI from the host
 * @core_handle: xDCI controller handle
 */ 
enum usb_status dwc_xdci_core_disconnect(void *core_handle)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
			(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t max_delay_iter =
			DWC_XDCI_MAX_DELAY_ITERATIONS;
	uint32_t base_addr; 

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_disconnect: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	base_addr =
			local_core_handle->base_address;

	/* Clear Run bit to disconnect from host */
	usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
	    		usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) &
	    		   ~DWC_XDCI_DCTL_RUN_STOP_MASK);

    /* Wait until core is halted */
    do {
    	if (usb_reg_read(base_addr,
    			DWC_XDCI_DSTS_REG) &
    			DWC_XDCI_DSTS_DEV_CTRL_HALTED_MASK)
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_disconnect: Failed to halt the device controller"));

    	return USB_OP_FAILED;
    }

    return USB_OK;
}

/* Interface:
 * This function is used to obtain current USB bus speed
 * @core_handle: xDCI controller handle
 * @speed: Address of variable to save the speed
 */ 
enum usb_status dwc_xdci_core_get_speed(void *core_handle,
		enum usb_speed *speed)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
			(struct usb_dcd_xdci_core_handle *)core_handle;
	
	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_get_speed: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	if (NULL == speed) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_get_speed: INVALID parameter"));
		return USB_INVALID_PARAM;
	}

	*speed = (usb_reg_read(local_core_handle->base_address,
			    DWC_XDCI_DSTS_REG) &
			      DWC_XDCI_DSTS_CONN_SPEED_MASK);
	
	return USB_OK;
}

/* Interface:
 * This function is used to obtain current USB bus speed
 * @core_handle: xDCI controller handle
 * @address: USB address to set (assigned by USB host)
 */
enum usb_status dwc_xdci_core_set_address(void *core_handle,
		uint32_t address)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
			(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t base_addr; 

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_set_address: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	base_addr =
			local_core_handle->base_address;

	/* Program USB device address */
	usb_reg_write(base_addr, DWC_XDCI_DCFG_REG,
	    		(usb_reg_read(base_addr, DWC_XDCI_DCFG_REG) &
	    		   ~DWC_XDCI_DCFG_DEV_ADDRESS_MASK) |
	    		   (address <<
	    			DWC_XDCI_DCFG_DEV_ADDRESS_BIT_POS));

	local_core_handle->dev_state = USB_DEVICE_STATE_ADDRESS;

    return USB_OK;
}

/* Interface:
 * This function is used to set configuration
 * @core_handle: xDCI controller handle
 * @config_num: config num to set (assigned by USB host)
 */
enum usb_status dwc_xdci_core_set_config(void *core_handle,
		uint32_t config_num)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
			(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_set_config: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	/* TODO: Disable all non-CTRL endpoints here if
	 * stack is not doing it
	 */
	/* TODO: Cancel all non-CTRL transfers here on all EPs
	 *  if stack is not doing it
	 */
	/* TODO: Change EP1 TXFIFO allocation if necessary */

	/* Re-initialize transfer resource allocation */

    /* Issue DEPSTARTCFG command on EP0 (new config for
     * non-control EPs)
     */
    status = dwc_xdci_core_init_ep_cmd_params(
    		local_core_handle,
    		&local_core_handle->ep_handles[0].ep_info,
    		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_NONE,
    		EPCMD_START_NEW_CONFIG,
    		&ep_cmd_params);
    
    if (status) {
        DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_set_config: Failed to init  params for EPCMD_START_NEW_CONFIG command"));
        return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		0, (EPCMD_START_NEW_CONFIG |
        			(2 << DWC_XDCI_EPCMD_RES_IDX_BIT_POS)), 
    		&ep_cmd_params);

    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_core_set_config: Failed to issue  EPCMD_START_NEW_CONFIG command"));
    	return status;
    }

	return status;
}

/* Interface:
 * This function is used to set link state
 * @core_handle: xDCI controller handle
 * @state: Desired link state
 */
enum usb_status dwc_xdci_set_link_state(void *core_handle,
		enum usb_device_ss_link_state state)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_set_link_state: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	base_addr = local_core_handle->base_address;

	/* Clear old mask */
	usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
			    		usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) &
			    		   ~DWC_XDCI_DCTL_STATE_CHANGE_REQ_MASK);
	
	/* Request new state */
	usb_reg_write(base_addr, DWC_XDCI_DCTL_REG,
			usb_reg_read(base_addr, DWC_XDCI_DCTL_REG) |
			(state << DWC_XDCI_DCTL_STATE_CHANGE_REQ_BIT_POS));

    return USB_OK;
}

/* Interface:
 * This function is used to initialize endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * to be initialized
 */
enum usb_status dwc_xdci_init_ep(void *core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status;
	uint32_t ep_num;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_init_ep: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);

	/* Save EP properties */
	local_core_handle->ep_handles[ep_num].ep_info = *ep_info;
	
	/* Init DEPCFG cmd params for EP */
	status = dwc_xdci_core_init_ep_cmd_params(
               core_handle,
               &local_core_handle->ep_handles[ep_num].ep_info,
                    DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_INIT_STATE,
	        		EPCMD_SET_EP_CONFIG,
	        		&ep_cmd_params);

    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_init_ep: Failed to init params for  EPCMD_SET_EP_CONFIG command"));
       	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(core_handle,
    		   ep_num, EPCMD_SET_EP_CONFIG, &ep_cmd_params);

	if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_init_ep: Failed to issue  EPCMD_SET_EP_CONFIG command"));
	   return status;
	}

	/* Issue a DEPXFERCFG command for endpoint */
    status = dwc_xdci_core_init_ep_cmd_params(
        		local_core_handle,
        		&local_core_handle->ep_handles[ep_num].ep_info,
        		DWC_XDCI_PARAM0_SET_EP_CFG_ACTN_NONE,
        		EPCMD_SET_EP_XFER_RES_CONFIG,
        		&ep_cmd_params);
        
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_init_ep: Failed to init params for  EPCMD_SET_EP_XFER_RES_CONFIG command"));
    	return status;
    }

    /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		    ep_num, EPCMD_SET_EP_XFER_RES_CONFIG,
    		    &ep_cmd_params);

    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_init_ep: Failed to issue EPCMD_SET_EP_XFER_RES_CONFIG command"));
    }

    return status;
}

/* Interface:
 * This function is used to enable non-Ep0 endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * to be enabled
 */
enum usb_status dwc_xdci_ep_enable(void *core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t ep_num;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_enable: INVALID handle"));
		return USB_INVALID_HANDLE;
	}
	
	base_addr = local_core_handle->base_address;

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);

    /* Enable Physical Endpoint ep_num */
    usb_reg_write(base_addr, DWC_XDCI_EP_DALEPENA_REG,
        		usb_reg_read(base_addr, DWC_XDCI_EP_DALEPENA_REG) |
        		  (1 << ep_num));
    
    return USB_OK;
}

/* Interface:
 * This function is used to disable non-Ep0 endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * to be enabled
 */
enum usb_status dwc_xdci_ep_disable(void *core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t ep_num;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_disable: INVALID handle"));
		return USB_INVALID_HANDLE;
	}
	
	base_addr = local_core_handle->base_address;

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);

    /* Disable Physical Endpoint ep_num */
    usb_reg_write(base_addr, DWC_XDCI_EP_DALEPENA_REG,
    		usb_reg_read(base_addr, DWC_XDCI_EP_DALEPENA_REG) &
    		  ~(1 << ep_num));
    
    return USB_OK;
}

/* Interface:
 * This function is used to STALL and endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * to be enabled
 */
enum usb_status dwc_xdci_ep_stall(void *core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status;
	uint32_t ep_num;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_stall: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);

    /* Issue a DWC_XDCI_EPCMD_SET_STALL for EP */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    /* Issue the command */
   status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
   		ep_num, DWC_XDCI_EPCMD_SET_STALL, 
   		&ep_cmd_params);
   
   if (status) {
	   DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_stall: Failed to issue EP stall command"));
   }

   return status;
}

/* Interface:
 * This function is used to clear endpoint STALL
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * to be enabled
 */
enum usb_status dwc_xdci_ep_clear_stall(void *core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status;
	uint32_t ep_num;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_clear_stall: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);

    /* Issue a DWC_XDCI_EPCMD_CLEAR_STALL for EP */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    /* Issue the command */
   status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
   		ep_num, DWC_XDCI_EPCMD_CLEAR_STALL, 
   		&ep_cmd_params);

   if (status) {
	   DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_stall: Failed to issue EP clea stall command"));
   }

   return status;
}

/* Interface:
 * This function is used to set endpoint in NOT READY state
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * to be enabled
 */
enum usb_status dwc_xdci_ep_set_nrdy(void *core_handle,
		struct usb_ep_info *ep_info)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	uint32_t ep_num;
	uint32_t base_addr;
	uint32_t max_delay_iter = DWC_XDCI_MAX_DELAY_ITERATIONS;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_set_nrdy: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	base_addr = local_core_handle->base_address;

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);

	/* Program the EP number in command's param reg */
	usb_reg_write(base_addr, DWC_XDCI_DGCMD_PARAM_REG, ep_num);
	
	/* Issue EP not ready generic device command */
	usb_reg_write(base_addr, DWC_XDCI_DGCMD_REG, 
			(usb_reg_read(base_addr, DWC_XDCI_DGCMD_REG) |
					DWC_XDCI_DGCMD_CMD_SET_EP_NRDY));
	
	/* Activate the command */
	usb_reg_write(base_addr, DWC_XDCI_DGCMD_REG, 
			(usb_reg_read(base_addr, DWC_XDCI_DGCMD_REG) |
					DWC_XDCI_DGCMD_CMD_ACTIVE_MASK));

    /* Wait until command completes */
    do {
    	if (!(usb_reg_read(base_addr,
    			DWC_XDCI_DGCMD_REG) &
    			DWC_XDCI_DGCMD_CMD_ACTIVE_MASK))
    		break;
    	else
    		delayMicro(DWC_XDCI_MAX_DELAY_ITERATIONS);
    } while (--max_delay_iter);

    if (!max_delay_iter) {
    	DEBUG ((DEBUG_INFO, "\nFailed to issue Command"));
    	return USB_OP_FAILED;

    }

    return USB_OK;
}

/* Interface:
 * This function is used to queue receive SETUP packet request
 * @core_handle: xDCI controller handle
 * @buffer: Address of buffer to receive SETUP packet
 */
enum usb_status dwc_xdci_ep0_receive_setup_pkt(void *core_handle,
		uint8_t *buffer)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
					(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status = USB_INVALID_OP;
	struct dwc_xdci_trb *trb;
	
	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_receive_setup_pkt: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	local_core_handle->ep_handles[0].ep_info.ep_num = 0;
	local_core_handle->ep_handles[0].ep_info.ep_dir = 0;
	local_core_handle->ep_handles[0].state = USB_EP_STATE_SETUP;
	trb = local_core_handle->trbs;

	status = dwc_xdci_core_init_trb(local_core_handle,
			trb,
			TRBCTL_SETUP,
			buffer, 8);

	if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_receive_setup_pkt: Init TRB Failed"));
		return status;
	}

    /* Issue a DEPSTRTXFER for EP0 */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    /* Init the lower re-bits for TRB address */
    ep_cmd_params.param1 = (uint32_t)(UINTN)trb;

     /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		0, EPCMD_START_XFER, 
    		&ep_cmd_params);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_receive_setup_pkt: Failed to issue Start Transfer command"));
    }

    /* Save new resource index for this transfer */
    local_core_handle->ep_handles[0].currentXferRscIdx =
    		((usb_reg_read(local_core_handle->base_address,
    				DWC_XDCI_EPCMD_REG(0)) &
    		          DWC_XDCI_EPCMD_RES_IDX_MASK) >>
    				    DWC_XDCI_EPCMD_RES_IDX_BIT_POS);

	return status;	
}

/* Interface:
 * This function is used to queue receive status packet on EP0
 * @core_handle: xDCI controller handle
 */
enum usb_status dwc_xdci_ep0_receive_status_pkt(void *core_handle)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_trb *trb;
	enum dwc_xdci_trb_control trb_ctrl;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_receive_status_pkt: INVALID handle"));
		return USB_INVALID_HANDLE;
	}
	
	base_addr = local_core_handle->base_address;

	/* We are receiving on EP0 so physical EP is 0 */
	trb = local_core_handle->trbs;

	local_core_handle->ep_handles[0].ep_info.ep_num = 0;
	local_core_handle->ep_handles[0].ep_info.ep_dir = 0;

	/* OUT data phase for 3-phased control transfer */
	trb_ctrl = TRBCTL_3_PHASE;

    /* Init TRB for the transfer */
    status = dwc_xdci_core_init_trb(local_core_handle,
              trb,
              trb_ctrl,
              local_core_handle->aligned_setup_buffer,
              local_core_handle->ep_handles[0].ep_info.max_pkt_size);
    
    if (!status) {
        /* Issue a DEPSTRTXFER for EP0 */
        /* Reset params */
        ep_cmd_params.param0 = ep_cmd_params.param1 =
                ep_cmd_params.param2 = 0;
    
        /* Init the lower bits for TRB address */
        ep_cmd_params.param1 = (uint32_t)(UINTN)trb;
    
         /* Issue the command */
        status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
                0, EPCMD_START_XFER, 
                &ep_cmd_params);
        
        if (status) {
            DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_receive_status_pkt: Failed to issue Start Transfer command for EP0"));
        }
        
        /* Save new resource index for this transfer */
        local_core_handle->ep_handles[0].currentXferRscIdx =
                ((usb_reg_read(base_addr, DWC_XDCI_EPCMD_REG(0)) &
                  DWC_XDCI_EPCMD_RES_IDX_MASK) >>
                        DWC_XDCI_EPCMD_RES_IDX_BIT_POS);
    
        /* TODO: We are not using the EP state for control transfers
         * right now simply because we're only supporting IN
         * data phase. For the current use case, we don't
         * need OUT data phase. We can add that later and we will
         * add some of the state and SETUP packet awareness code
         */
        local_core_handle->ep_handles[0].state = USB_EP_STATE_STATUS;
    }

    return status;		
}

/* Interface:
 * This function is used to send status packet on EP0
 * @core_handle: xDCI controller handle
 */
enum usb_status dwc_xdci_ep0_send_status_pkt(void *core_handle)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_trb *trb;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	enum usb_status status;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_send_status_pkt: INVALID handle"));
		return USB_INVALID_HANDLE;
	}
	
	base_addr = local_core_handle->base_address;

	/* We are sending on EP0 so physical EP is 1 */
	trb = (local_core_handle->trbs + 1);

	local_core_handle->ep_handles[0].state = USB_EP_STATE_STATUS;
	status = dwc_xdci_core_init_trb(local_core_handle,
			  trb,
			  TRBCTL_2_PHASE,
			  local_core_handle->aligned_misc_buffer, 0);
	
	if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_send_status_pkt: TRB failed during status phase"));
		return status;
	}

    /* Issue a DEPSTRTXFER for EP1 */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    /* Init the lower re-bits for TRB address */
    ep_cmd_params.param1 = (uint32_t)(UINTN)trb;

     /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		1, EPCMD_START_XFER, 
    		&ep_cmd_params);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep0_send_status_pkt: Failed to issue Start Transfer on EP0"));
    }

    /* Save new resource index for this transfer */
    local_core_handle->ep_handles[1].currentXferRscIdx =
    		((usb_reg_read(base_addr, DWC_XDCI_EPCMD_REG(1)) &
    		  DWC_XDCI_EPCMD_RES_IDX_MASK) >>
    				DWC_XDCI_EPCMD_RES_IDX_BIT_POS);

    local_core_handle->ep_handles[0].state = USB_EP_STATE_STATUS;

    return status;	
}

/* Interface:
 * This function is used to send data on non-EP0 endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * @buffer: buffer containing data to transmit
 * @size: Size of transfer (in bytes)
 */
enum usb_status dwc_xdci_ep_tx_data(void *core_handle,
		struct usb_xfer_request *xfer_req)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	struct dwc_xdci_trb *trb;
	enum dwc_xdci_trb_control trb_ctrl;
	enum usb_status status;
	uint32_t ep_num;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_tx_data: INVALID handle"));
		return USB_INVALID_HANDLE;
	}
	
	if (NULL == xfer_req) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_tx_data: INVALID transfer request"));
		return USB_INVALID_PARAM;
	}

	base_addr = local_core_handle->base_address;

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(
			    xfer_req->ep_info.ep_num,
			    xfer_req->ep_info.ep_dir);

	trb = (local_core_handle->trbs + ep_num);


	if (ep_num > 1)
		trb_ctrl = TRBCTL_NORMAL;
	else
		trb_ctrl = TRBCTL_CTRL_DATA_PHASE;

	if (trb->trb_ctrl & DWC_XDCI_TRB_CTRL_HWO_MASK) {
		status = dwc_xdci_end_xfer(local_core_handle, ep_num);
		if (status) {
			DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_tx_data: Failed to end previous transfer"));
		}

		status = dwc_xdci_core_flush_ep_tx_fifo(
				    local_core_handle, ep_num);
		if (status) {
			DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_tx_data: Failed to end previous transfer"));
		}
	}

	/* Data phase */
	local_core_handle->ep_handles[ep_num].xfer_handle =
			*xfer_req;
	local_core_handle->ep_handles[ep_num].state =
			USB_EP_STATE_DATA;

	local_core_handle->ep_handles[ep_num].trb = trb;

	status = dwc_xdci_core_init_trb(local_core_handle,
			   trb,
			   trb_ctrl,
			   xfer_req->xfer_buffer, xfer_req->xfer_len);
				
	if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_tx_data: TRB failed"));
		return status;
	}

    /* Issue a DEPSTRTXFER for EP */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    /* Init the lower re-bits for TRB address */
    ep_cmd_params.param1 = (uint32_t)(UINTN)trb;

     /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		ep_num, EPCMD_START_XFER, 
    		&ep_cmd_params);

    /* Save new resource index for this transfer */
    local_core_handle->ep_handles[ep_num].currentXferRscIdx =
    	((usb_reg_read(base_addr, DWC_XDCI_EPCMD_REG(ep_num)) &
    	  DWC_XDCI_EPCMD_RES_IDX_MASK) >>
    			DWC_XDCI_EPCMD_RES_IDX_BIT_POS);

    return status;
}

/* Interface:
 * This function is used to receive data on non-EP0 endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 * @buffer: buffer containing data to transmit
 * @size: Size of transfer (in bytes)
 */
enum usb_status dwc_xdci_ep_rx_data(void *core_handle,
		struct usb_xfer_request *xfer_req)
{
	struct usb_dcd_xdci_core_handle *local_core_handle =
				(struct usb_dcd_xdci_core_handle *)core_handle;
	struct dwc_xdci_ep_cmd_params ep_cmd_params;
	struct dwc_xdci_trb *trb;
	enum dwc_xdci_trb_control trb_ctrl;
	enum usb_status status;
	uint32_t ep_num;
	uint32_t base_addr;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_rx_data: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	if (NULL == xfer_req) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_rx_data: INVALID transfer request"));
		return USB_INVALID_PARAM;
	}

	base_addr = local_core_handle->base_address;

	/* Convert to physical endpoint */
	ep_num = dwc_xdci_get_physical_ep_num(xfer_req->ep_info.ep_num,
			xfer_req->ep_info.ep_dir);

	trb = (local_core_handle->trbs + ep_num);
	
	if (ep_num > 1)
		trb_ctrl = TRBCTL_NORMAL;
	else
		trb_ctrl = TRBCTL_CTRL_DATA_PHASE;

	/* Data phase */
	local_core_handle->ep_handles[ep_num].xfer_handle =
				*xfer_req;

	local_core_handle->ep_handles[ep_num].state =
			USB_EP_STATE_DATA;

	local_core_handle->ep_handles[ep_num].trb = trb;

	status = dwc_xdci_core_init_trb(local_core_handle,
			   trb, trb_ctrl,
			   xfer_req->xfer_buffer, xfer_req->xfer_len);
				
	if (status) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_rx_data: TRB failed"));
		return status;
	}
	
    /* Issue a DEPSTRTXFER for EP */
    /* Reset params */
    ep_cmd_params.param0 = ep_cmd_params.param1 =
    		ep_cmd_params.param2 = 0;

    /* Init the lower re-bits for TRB address */
    ep_cmd_params.param1 = (uint32_t)(UINTN)trb;

     /* Issue the command */
    status = dwc_xdci_core_issue_ep_cmd(local_core_handle,
    		ep_num, EPCMD_START_XFER, 
    		&ep_cmd_params);
    
    if (status) {
    	DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_rx_data: Failed to start transfer"));
    }
    
    /* Save new resource index for this transfer */
    local_core_handle->ep_handles[ep_num].currentXferRscIdx =
       	((usb_reg_read(base_addr, DWC_XDCI_EPCMD_REG(ep_num)) &
       	  DWC_XDCI_EPCMD_RES_IDX_MASK) >>
       			DWC_XDCI_EPCMD_RES_IDX_BIT_POS);

    return status;
}

/* Interface:
 * This function is used to cancel a transfer on non-EP0 endpoint
 * @core_handle: xDCI controller handle
 * @ep_info: Address of structure describing properties of EP
 */
enum usb_status dwc_xdci_ep_cancel_transfer(void *core_handle,
		struct usb_ep_info *ep_info)
{
	enum usb_status status = USB_INVALID_HANDLE;
	uint32_t ep_num;

	if (NULL == core_handle) {
		DEBUG ((DEBUG_INFO, "\ndwc_xdci_ep_cancel_transfer: INVALID handle"));
		return USB_INVALID_HANDLE;
	}

	/* Get physical EP num */
	ep_num = dwc_xdci_get_physical_ep_num(ep_info->ep_num,
			ep_info->ep_dir);
	
	status = dwc_xdci_end_xfer(core_handle, ep_num);
   
    return status;
}
