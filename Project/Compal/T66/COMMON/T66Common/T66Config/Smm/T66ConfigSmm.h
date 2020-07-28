/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _T66_CONFIG_SMM_H_
#define _T66_CONFIG_SMM_H_

#include <PiDxe.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/SmmBase2.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <CompalEcLib.h>

#define  Compal_EC_ENTRY_CALLBACK_SMI  0xCC
#define EC_CMD_PLATFORM_NOTIFICATION                  0x40
#define EC_CMD_PLATFORM_NOTIFICATION_SYSTEM_REBOOT    0x02

#define  TXE_UNLOCK  0x7C
#define  TXE_LOCK    0x7D


#endif
