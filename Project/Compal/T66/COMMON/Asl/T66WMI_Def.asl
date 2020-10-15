/*---------------------------------------------------------------------------
 *
 *	$Workfile:   T66WMI.ASL  $
 *
 *	$Header:   $
 *
 *	Copyright (c) 2016~ Compal Company
 *
 *	This program contains proprietary and confidential information.
 *	All rights reserved except as may be permitted by prior
 *	written consent.
 *
 *	Content:
 *
 *		R E V I S I O N    H I S T O R Y
 *	$Log:   $
 *
 *  Date        Who             Change
 *	----------  ------------    -------------------------------------------
 *-------------------------------------------------------------------------*/


#ifndef _T66_WMI_DEF_ASI_
#define _T66_WMI_DEF_ASI_

#include "T66WMI_Error.asl"

#define WMI_SW_SMI           0xCE
#define WMI_OEM_SW_SMI       0xCF
#define PRJ_DTS_SMI_NUM      0xC1
#define ThermalUtility_SW_SMI 0xD9
  #define Read_DTS_Temperature  0x01

#define CMFC_VERSION_CTRL               0x10
  #define SET_CMFC_AP_VERSION             0x01
  #define GET_CMFC_FW_VERSION             0x02
#define CMFC_BOOT_DEVICE_SEQUENCE       0x13
#define CMFC_RELOAD_BIOS_DEFUAL_TVALUE  0x14
#define CMFC_EC_COMMUNICATE_INTERFACE   0x15

//********-[ WMAB :BIOS function list ]-*****************
#define Reload_BIOS_Default_Value       0x01
#define SetPxeBootFirst                 0x02
//**************************************************


//********-[ WMAC :DIO function list ]-*****************
#ifdef WMI_WMAC_SUPPORT
#define DIOP DIOP_ASL
#endif

#define DIO_GetSupport         0x01
#define DIO_GetAmount          0x02
#define DIO_GetSettings        0x03
#define DIO_SetDirection       0x04
#define DIO_SetDirectionmany   0x05
#define DIO_GetValue           0x06
#define DIO_GetValuemany       0x07
#define DIO_SetValue           0x08
#define DIO_SetValuemany       0x09
#define SINGLE_PIN        0
#define MULTIPLE_PIN      1
//**************************************************

//********-[ WMAD :CPU function list ]-*****************
#define WMAD_FUNC_GetCPUDTS             0x01
//**************************************************

//********-[ WMAE :EC function list ]-*****************
#define EC_ReadWriteCommend             0x01
//**************************************************
#endif
