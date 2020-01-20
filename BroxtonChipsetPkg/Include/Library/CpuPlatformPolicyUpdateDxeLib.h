/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/
/*++

Copyright (c) 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  CpuPlatformPolicyUpdateDxeLib.h

Abstract:

  Header file for DXE CpuPlatformPolicyUpdate Library. 
  
--*/
#ifndef _CPU_PLATFORM_POLICY_UPDATE_DXE_LIB_H_
#define _CPU_PLATFORM_POLICY_UPDATE_DXE_LIB_H_

EFI_STATUS
EFIAPI
UpdateDxeCpuPlatformPolicy (
  IN OUT  EFI_PLATFORM_CPU_PROTOCOL  *PlatformCpu
  )
/*++
 
Routine Description:
 
  This function updates Dxe Cpu Platform Policy Protocol
  
Arguments:
 
  DxePlatformCpuPolicy            The CPU Platform Policy protocol instance
 
Returns:
 
  EFI_SUCCESS                     Initialization complete.
  EFI_UNSUPPORTED                 The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES            Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR                Device error, driver exits abnormally.
 
--*/
;
#endif
