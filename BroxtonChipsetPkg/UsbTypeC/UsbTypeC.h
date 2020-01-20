/**@file

@copyright
 Copyright (c) 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef __USBTYPEC__H__
#define __USBTYPEC__H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <PiDxe.h>
//#include "PlatformBoardId.h"

#pragma pack (1)
typedef struct  {
	UINT16		Version;		// PPM -> OPM, Version JJ.M.N (JJ - major version, M - minor version, N - sub-minor version). Version 2.1.3 is represented with value 0x0213
	UINT16		Reserved;		// Reversed
	UINT8		CCI[4];			// PPM->OPM CCI indicator
	UINT8		Control[8];		// OPM->PPM Control message
	UINT8		MsgIn[16];		// PPM->OPM Message In, 128 bits, 16 bytes
	UINT8		MsgOut[16];  	// OPM->PPM Message Out
} USBTYPEC_OPM_BUF ;
#pragma pack ()

#ifdef __cplusplus
}
#endif
#endif
