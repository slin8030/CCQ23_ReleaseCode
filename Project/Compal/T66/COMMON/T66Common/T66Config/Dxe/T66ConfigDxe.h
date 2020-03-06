/** @file
  Private data structures for OemConfig DXE driver

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __T66_CONFIG_DXE_H__
#define __T66_CONFIG_DXE_H__

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

typedef enum {
  READ_FROM_EC,
  READ_FROM_BIOS_ROM,
  READ_FROM_LPC_SPI
} EEPROM_READ_POLICY;

#define DEFAULT_READ_EEPROM_POLICY  READ_FROM_BIOS_ROM
//[PRJ]-- Move VEEPROM data to SMBIOS
#define OEMCFG_TPL_NOTIFY_H       TPL_NOTIFY+2
#define OEMCFG_TPL_NOTIFY_L       TPL_NOTIFY-2

EFI_STATUS
EFIAPI
OemCreateProtocolNotifyEvent (
  IN  EFI_GUID           *ProtocolGuid,
  IN  EFI_TPL             NotifyTpl,
  IN  EFI_EVENT_NOTIFY    NotifyFunction,
  IN  VOID               *NotifyContext,  OPTIONAL
  OUT VOID              **Registration,
  OUT EFI_EVENT          *Event
  );

#endif
