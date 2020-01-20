/** @file
  ChipsetFotaOemProtect

***************************************************************************
* Copyright (c) 2014-2015, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#ifndef _FOTA_OEM_PROTECT_H
#define _FOTA_OEM_PROTECT_H

#include <ChipsetFota.h>
#include <DmiStringInformation.h>

BOOLEAN
IsVariableServiceSupported (
  VOID
  );

BOOLEAN
CheckVariableDelete (
  IN CHAR16                        *VariableName,
  IN EFI_GUID                      *VendorGuid,
  IN FOTA_PRESERVED_VARIABLE_TABLE *VariablePreservedTablePtr,
  IN BOOLEAN                       IsKeepVariableInList
  );

EFI_STATUS
RelocateNextVariableName (
  IN OUT UINTN                  *VariableSize,
  IN OUT CHAR16                 **VariableName,
  IN OUT EFI_GUID               *VendorGuid,
  IN OUT UINTN                  *MaxVariableNameSize
  );

BOOLEAN
IsZeroGuid (
  IN EFI_GUID                   *Guid
  );

UINTN
GetNumberOfVariable (
  IN UINT8                      *TablePtr
  );

VOID
GetDefaultTable (
  OUT FOTA_PRESERVED_VARIABLE_TABLE   **TablePtr
  );

EFI_STATUS
PurifyVariable (
  VOID
  );

#endif
