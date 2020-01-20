/** @file

  Header file of BIOS Storage Kernel Use Variable Dxe implementation.

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

#ifndef _BIOS_STORAGE_KERNEL_USE_VARIABLE_DXE_H_
#define _BIOS_STORAGE_KERNEL_USE_VARIABLE_DXE_H_

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/VariableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/CmosLib.h>
#include <Library/EventLogDxeLib.h>
#include <Library/EventLogGetTimeFromCmosLib.h>
#include <Protocol/EventLog.h>
#include <Protocol/BiosEventLogUseVariable.h>
#include <Protocol/VariableWrite.h>
#include <Guid/BiosStorageVariable.h>

EFI_STATUS
EFIAPI
WriteEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL        *This,
  IN  UINT8                                           EventLogType,
  IN  UINT32                                          PostBitmap1,
  IN  UINT32                                          PostBitmap2,
  IN  UINTN                                           OptionDataSize,
  IN  UINT8                                           *OptionLogData
  );

EFI_STATUS
EFIAPI
ClearEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This
  );

EFI_STATUS
EFIAPI
ReadEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL         *This,
  OUT VOID                                             **Buffer,
  OUT UINTN                                            *EventCount
  );

EFI_STATUS
EFIAPI
OverWriteEventLog (
  IN  H2O_BIOS_EVENT_LOG_USE_VARIABLE_PROTOCOL       *This,
  IN  BIOS_EVENT_LOG_ORGANIZATION                    *InputBuffer
  );

#endif
