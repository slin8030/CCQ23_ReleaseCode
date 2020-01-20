/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/
/*++

Copyright (c) 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  DptfPlatformPolicyUpdateDxeLib.h

Abstract:

  Header file for DXE DptfPlatformPolicyUpdate Library. 
  
--*/
#ifndef _DPTF_PLATFORM_POLICY_UPDATE_DXE_LIB_H_
#define _DPTF_PLATFORM_POLICY_UPDATE_DXE_LIB_H_

EFI_STATUS
EFIAPI
UpdateDxeDptfPlatformPolicy (
  IN OUT  DXE_DPTF_PLATFORM_POLICY_PROTOCOL  *DxePlatformDptfPolicy
  )
/*++
 
Routine Description:
 
  Get data for platform policy from setup options
  
Arguments:
 
  DxePlatformSaPolicy   - Pointer to DXE DPTF platform policy structure
 
Returns:
 
  EFI_SUCCESS                     Initialization complete.
 
--*/
;
#endif
