/** @file

@copyright
Copyright (c) 2012 - 2015 Intel Corporation. All rights reserved
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

#ifndef _DXE_SA_POLICY_UPDATE_LIB_H_
#define _DXE_SA_POLICY_UPDATE_LIB_H_

//[-start-160216-IB03090424-modify]//
/**
Get data for platform policy from setup options.

@param[in] DxeSaPolicy               The pointer to get SA Policy protocol instance
@param[in] SystemConfiguration		The pointer to get System Setup

@retval EFI_SUCCESS                  Operation success.

**/
EFI_STATUS
EFIAPI
UpdateDxeSaPolicy(
	IN OUT  SA_POLICY_PROTOCOL *DxeSaPolicy,
	IN CHIPSET_CONFIGURATION* SystemConfiguration
);
//[-end-160216-IB03090424-modify]//

#endif
