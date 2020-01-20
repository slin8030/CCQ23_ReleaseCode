/** @file
  The main funcitons of BDS platform recovery

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _RECOVERY_DRIVER_H
#define _RECOVERY_DRIVER_H

#include "InternalBdsLib.h"

EFI_STATUS
RecoveryDriverEntry (
  IN CHAR16 *Command
  );

#endif