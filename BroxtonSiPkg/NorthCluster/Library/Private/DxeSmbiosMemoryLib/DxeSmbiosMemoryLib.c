/** @file
 This driver will determine memory configuration information from the chipset
 and memory and create SMBIOS memory structures appropriately.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include "SmbiosMemory.h"

/**
  This library will determine memory configuration information from the chipset
  and memory and report the memory configuration info to the DataHub.

  @param[in]  ImageHandle    Handle for the image of this driver
  @param[in]  SystemTable    Pointer to the EFI System Table

  @retval  EFI_SUCCESS   if the data is successfully reported
  @retval  EFI_NOT_FOUND if the HOB list could not be located.
**/
EFI_STATUS
EFIAPI
SmbiosMemory (
  )
{
  EFI_STATUS        Status;

  Status = InstallSmbiosType16 ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallSmbiosType17 ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallSmbiosType19 ();
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/**
  Add an SMBIOS table entry using EFI_SMBIOS_PROTOCOL.
  Create the full table record using the formatted section plus each non-null string, plus the terminating (double) null.

  @param[in]  Entry                 The data for the fixed portion of the SMBIOS entry.
                                    The format of the data is determined by EFI_SMBIOS_TABLE_HEADER.
                                    Type. The size of the formatted area is defined by
                                    EFI_SMBIOS_TABLE_HEADER. Length and either followed by a
                                    double-null (0x0000) or a set of null terminated strings and a null.
  @param[in]  TableStrings          Set of string pointers to append onto the full record.
                                    If TableStrings is null, no strings are appended. Null strings
                                    are skipped.
  @param[in]  NumberOfStrings       Number of TableStrings to append, null strings are skipped.
  @param[out] SmbiosHandle          A unique handle will be assigned to the SMBIOS record.

  @retval     EFI_SUCCESS           Table was added.
  @retval     EFI_OUT_OF_RESOURCES  Table was not added due to lack of system resources.
**/
EFI_STATUS
AddSmbiosEntry (
  IN  EFI_SMBIOS_TABLE_HEADER *Entry,
  IN  CHAR8                   **TableStrings,
  IN  UINT8                   NumberOfStrings,
  OUT EFI_SMBIOS_HANDLE       *SmbiosHandle
  )
{
  EFI_STATUS              Status;
  EFI_SMBIOS_TABLE_HEADER *Record;
  CHAR8                   *StringPtr;
  UINTN                   Size;
  UINTN                   i;

  ///
  /// Calculate the total size of the full record
  ///
  Size = Entry->Length;

  ///
  /// Add the size of each non-null string
  ///
  if (TableStrings != NULL) {
    for (i = 0; i < NumberOfStrings; i++) {
      if (TableStrings[i] != NULL) {
        Size += AsciiStrSize (TableStrings[i]);
      }
    }
  }

  ///
  /// Add the size of the terminating double null
  /// If there were any strings added, just add the second null
  ///
  if (Size == Entry->Length) {
    Size += 2;
  } else {
    Size += 1;
  }

  ///
  /// Initialize the full record
  ///
  Record = (EFI_SMBIOS_TABLE_HEADER *) AllocateZeroPool (Size);
  if (Record == NULL) {
      return EFI_OUT_OF_RESOURCES;
  }
  CopyMem(Record, Entry, Entry->Length);

  ///
  /// Copy the strings to the end of the record
  ///
  StringPtr = ((CHAR8 *) Record) + Entry->Length;
  Size = Size - Entry->Length;
  if (TableStrings != NULL) {
    for (i = 0; i < NumberOfStrings; i++) {
      if (TableStrings[i] != NULL) {
        AsciiStrCpyS (StringPtr, Size, TableStrings[i]);
        StringPtr += AsciiStrSize (TableStrings[i]);
        Size = Size - AsciiStrSize (TableStrings[i]);
      }
    }
  }

  *SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = mSmbios->Add (mSmbios, NULL, SmbiosHandle, Record);

  FreePool (Record);
  return Status;
}