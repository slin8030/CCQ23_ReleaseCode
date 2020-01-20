/** @file
  Header file for the SMM SMRAM Save Info Handler Driver.

@copyright
  Copyright (c) 2004 - 2015 Intel Corporation. All rights reserved
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
#ifndef _SMM_SMRAM_SAVE_INFO_HANDLER_H_
#define _SMM_SMRAM_SAVE_INFO_HANDLER_H_

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/HobLib.h>
#include <ScAccess.h>
//[-start-151229-IB03090424-modify]//
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmReadyToLock.h>
#include <Protocol/SmmControl2.h>
//[-end-151229-IB03090424-modify]//
#include <Protocol/CpuInfo.h>

#endif
