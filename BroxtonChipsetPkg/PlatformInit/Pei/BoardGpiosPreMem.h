/** @file
  Gpio setting for multiplatform before Memory init.

@copyright
  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
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

#ifndef _BOARDGPIOS_PREMEM_H_
#define _BOARDGPIOS_PREMEM_H_

//
// Function Prototypes Only - Do not add #includes here
//



EFI_STATUS
MultiPlatformGpioProgramPreMem (
//[-start-160413-IB03090426-modify]//
  IN OUT UINT64   *StartTimerTick
//[-end-160413-IB03090426-modify]//
  );
  
//[-start-160720-IB03090429-modify]//
EFI_STATUS
MultiPlatformGpioUpdatePreMem (
  VOID
  );  
//[-end-160720-IB03090429-modify]//
  
#endif
