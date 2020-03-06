/*---------------------------------------------------------------------------
 *
 *	$Workfile:   CMFC_ProjectCommon.ASL  $
 *
 *	$Header:   $
 *
 *	Copyright (c) 2010~ Compal Company
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

//
// OEM Function No (0x80~0xFE)
//
#define CMFC_OEM_FUNCTION_1          0x80
#define CMFC_OEM_FUNCTION_2          0x81
#define CMFC_OEM_FUNCTION_3          0x82

//
// OEM CMFC Triger SMI List Table
//
Name (OCSL, Package() {
CMFC_OEM_FUNCTION_1,
CMFC_OEM_FUNCTION_2,
CMFC_OEM_FUNCTION_3
})
