/** @file

  Header file of BIOS Storage Chipset SMM implementation.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _BIOS_STORAGE_CHIPSET_SMM_H_
#define _BIOS_STORAGE_CHIPSET_SMM_H_

#include <Protocol/BiosEventLog.h>
#include <Protocol/EventLog.h>
#include <Protocol/EventStorage.h>
#include <Protocol/SmmBase2.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/BiosEventLogUseVariable.h>
#include <Library/DebugLib.h>
#include <Library/VariableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/BiosStorageVariable.h>

#define EVENT_LOG_FULL_ADJUST_EVENT_NUM    3
#define CHANGE_BIOS_STORAGE_DATA3          0x01


typedef struct {
  BIOS_EVENT_LOG_ORGANIZATION       *EventList;
} VE_BIOS_EVENT_LOG_STRUCTURE;

EFI_STATUS
EFIAPI
BiosWriteEvent (
  IN  EVENT_TYPE_ID                    *EventID,
  IN  UINT8                            *Data,
  IN  UINTN                            DataSize
  );

EFI_STATUS
EFIAPI
BiosReadEvent (
  IN      UINTN                        Index,
  IN OUT  UINT8                        **Data
  );

EFI_STATUS
EFIAPI
BiosGetStorageNameString (
  CHAR16                               **String
);

EFI_STATUS
EFIAPI
BiosGetEventCount (
  IN  OUT UINTN                        *Count
  );

EFI_STATUS
EFIAPI
BiosRefreshDatabase (
  IN OUT  UINTN                        *DataCount
  );

EFI_STATUS
EFIAPI
BiosClearEvent (
  VOID
);

UINT8
EFIAPI
TimeTranslator (
  IN  UINT8                            OrgTime
);

VOID
EFIAPI
FreeBiosEventDatabase (
  VOID
  );

EFI_STATUS
EFIAPI
AddEventAfterArrangeEventStorage (
  VOID
  );

EFI_STATUS
EFIAPI
ArrangeEventStorage (
  VOID
  );

EFI_STATUS
EFIAPI
ShiftDataOfStorage (
  VOID
  );

#endif
