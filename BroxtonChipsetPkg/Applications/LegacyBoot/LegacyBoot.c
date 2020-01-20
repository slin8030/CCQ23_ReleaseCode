/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/*++
  This file contains a 'EFI Shell Code' and is licensed as      
  such under the terms of your license agreement with Intel or  
  your vendor.  This file may be modified by the user, subject  
  to the additional terms of the license agreement.             
--*/
/*++

Copyright (c)  1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  LegacyBoot.c
  
Abstract:

  LegacyBoot shell command. Boot from a device through the Furball

  LegacyBoot [FD|HD|CDROM|PCMCIA|USB|NET|BEV]

Revision History

--*/


#include <Uefi.h>

#include <Guid/LegacyBios.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DevicePathToText.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>



#include "Legacyboot.h"

EFI_DEVICE_PATH_PROTOCOL  EndDevicePath[] = {
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      END_DEVICE_PATH_LENGTH,
      0
    }
  }
};

//usage
void usage(void)
{
  Print(L"Usage: \n");
  Print(L"   LegacyBoot [FD|HD|CDROM|PCMCIA|USB|NET|BEV] \n");
  return;
}

EFI_STATUS
RefreshBbsTableForBoot (
  IN     UINT16        DevType,
  IN     UINT16        BbsCount,
  IN OUT BBS_TABLE     *BbsTable
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  Status = EFI_NOT_FOUND;
  //
  // Find the first present boot device whose device type
  // matches the DevType, we use it to boot first.
  //
  // And set other present boot devices' priority to BBS_UNPRIORITIZED_ENTRY
  // their priority will be set by LegacyBiosPlatform protocol by default
  //
  for (Index = 0; Index < BbsCount; Index++) {
    if (BBS_IGNORE_ENTRY == BbsTable[Index].BootPriority ||
        BBS_DO_NOT_BOOT_FROM == BbsTable[Index].BootPriority ||
        BBS_LOWEST_PRIORITY == BbsTable[Index].BootPriority
        ) {
      continue;
    }

    if (BbsTable[Index].DeviceType == DevType && EFI_SUCCESS != Status) {
      BbsTable[Index].BootPriority  = 0;
      Status                        = EFI_SUCCESS;
    } else {
      BbsTable[Index].BootPriority = BBS_UNPRIORITIZED_ENTRY;
    }
  }

  return Status;
}

/**
  This function converts an input device structure to a Unicode string.

  @param DevPath                  A pointer to the device path structure.

  @return A new allocated Unicode string that represents the device path.

**/
CHAR16 *
DevicePathToStr (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath
  )
{
  EFI_STATUS                       Status;
  CHAR16                           *ToText;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DevPathToText;

  if (DevPath == NULL) {
    return NULL;
  }

  Status = gBS->LocateProtocol (
                  &gEfiDevicePathToTextProtocolGuid,
                  NULL,
                  (VOID **) &DevPathToText
                  );
  ASSERT_EFI_ERROR (Status);
  ToText = DevPathToText->ConvertDevicePathToText (
                            DevPath,
                            FALSE,
                            TRUE
                            );
  ASSERT (ToText != NULL);
  return ToText;
}


/**
  UEFI application entry point which has an interface similar to a
  standard C main function.

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  EFI_STATUS                Status = EFI_SUCCESS;
  EFI_LEGACY_BIOS_PROTOCOL  *LegacyBios;
  UINT16                    DeviceType;
  BBS_BBS_DEVICE_PATH       BbsDevicePathNode;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BBS_TABLE                 *BbsTable;
  UINT16                    HddCount = 0;
  UINT16                    BbsCount = 0;
  HDD_INFO                  *LocalHddInfo = NULL;

  CHAR8 AsciiStr[20]={0};
  
  if (Argc != 2) {
    usage();
    return EFI_INVALID_PARAMETER;
  }
 
  UnicodeStrToAsciiStr(Argv[1], AsciiStr);

  DeviceType = BBS_TYPE_UNKNOWN;
  if (AsciiStrCmp (AsciiStr, "FD") == 0) {
    DeviceType = BBS_TYPE_FLOPPY;
  }

  if (AsciiStrCmp (AsciiStr, "HD") == 0) {
    DeviceType = BBS_TYPE_HARDDRIVE;
  }

  if (AsciiStrCmp (AsciiStr, "CDROM") == 0) {
    DeviceType = BBS_TYPE_CDROM;
  }

  if (AsciiStrCmp (AsciiStr, "PCMCIA") == 0) {
    DeviceType = BBS_TYPE_PCMCIA;
  }

  if (AsciiStrCmp (AsciiStr, "USB") == 0) {
    DeviceType = BBS_TYPE_USB;
  }

  if (AsciiStrCmp (AsciiStr, "NET") == 0) {
    DeviceType = BBS_TYPE_EMBEDDED_NETWORK;
  }

  if (AsciiStrCmp (AsciiStr, "BEV") == 0) {
    DeviceType = BBS_TYPE_BEV;
  }
  //
  // Make sure the Legacy Boot Protocol is available
  //
  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&LegacyBios);
  if (EFI_ERROR (Status)) {
	Print (L"Can not find the LegacyBios protocol.\n");
    Status = EFI_UNSUPPORTED;
    goto Quit;
  }
  //
  // Get BBS table instance
  //
  Status = LegacyBios->GetBbsInfo (
                        LegacyBios,
                        &HddCount,
                        &LocalHddInfo,
                        &BbsCount,
                        &BbsTable
                        );
  if (EFI_ERROR (Status)) {
	  Print (L"Can not locate the BBS table.\n");
    goto Quit;
  }
  //TODO:  force add extra BbsEntry here for debugging
  Status = RefreshBbsTableForBoot (
            DeviceType,
            BbsCount,
            BbsTable
            );
  if (EFI_ERROR (Status)) {
    Print (L"Didn't find the specified device.\n");
    goto Quit;
  }

//
  // Build the BBS Device Path for this boot selection
  //
  BbsDevicePathNode.Header.Type     = BBS_DEVICE_PATH;
  BbsDevicePathNode.Header.SubType  = BBS_BBS_DP;
  SetDevicePathNodeLength (&BbsDevicePathNode.Header, sizeof (BBS_BBS_DEVICE_PATH));
  BbsDevicePathNode.DeviceType  = DeviceType;
  BbsDevicePathNode.StatusFlag  = 0;
  BbsDevicePathNode.String[0]   = 0;

  DevicePath                    = AppendDevicePathNode (EndDevicePath, (EFI_DEVICE_PATH_PROTOCOL *) &BbsDevicePathNode);

  //
  // Call the Legacy Bios Protcol
  //
  Print (L"Call the Legacy Bios Protocol with %S\n", DevicePathToStr(DevicePath));
  Status = LegacyBios->LegacyBoot (LegacyBios, (BBS_BBS_DEVICE_PATH *) DevicePath, 0, NULL);
  Print (L"Returned %r\n",Status);

  FreePool (DevicePath);

Quit:

  return Status;

}
