/** @file
  Provide OEM to define the updatable SMBIOS string when use the DMI tool.

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmOemSvcKernelLib.h>

/**
  This service provides OEM to define the updatable SMBIOS string when use the DMI tool.
  OEM define updatable string by modifying the UpdateableString Table.

  @param[out]  *UpdateableStringCount    The number of UpdateableString instances.
  @param[out]  **mUpdatableStrings       Pointer to UpdateableString Table.

  @retval      EFI_UNSUPPORTED           Returns unsupported by default.
  @retval      EFI_SUCCESS               Get Updatable string success.
  @retval      EFI_MEDIA_CHANGED         The value of IN OUT parameter is changed. 
  @retval      Others                    Base on OEM design.
**/
EFI_STATUS
OemSvcSmmInstallPnpStringTable (
  OUT UINTN                                 *UpdateableStringCount,
  OUT DMI_UPDATABLE_STRING                  **mUpdatableStrings
  )
{
//[-start-181005-IB07401025-modify]//
//  DMI_UPDATABLE_STRING UpdatableStrings[] = {   
//        {0, 5}, //Structure Type 0 Offset 5, BIOS Version
//
//        {1, 4}, //Structure Type 1 Offset 4, SystemInfo Manufacturer Name
//        {1, 5}, //Structure Type 1 Offset 5, SystemInfo Product Name
//        {1, 6}, //Structure Type 1 Offset 6, SystemInfo Version
//        {1, 7}, //Structure Type 1 Offset 7, SystemInfo Serial Number
//
//        {2, 4}, //Structure Type 2 Offset 4, BaseBoard Manufacturer
//        {2, 5}, //Structure Type 2 Offset 5, BaseBoard ProductName
//        {2, 6}, //Structure Type 2 Offset 6, BaseBoard Version
//        {2, 7}, //Structure Type 2 Offset 7, BaseBoard SerialNumber
//
//        {3, 4}, //Structure Type 3 Offset 4, SystemEnclosure Manufacturer
//        {3, 5},
//        {3, 6}, //Structure Type 3 Offset 6, SystemEnclosure Version
//        {3, 7}, //Structure Type 3 Offset 7, SystemEnclosure SerialNumber
//        {3, 8}, //Structure Type 3 Offset 8, SystemEnclosure AssetTag
//
//        {11, 4},
//        {11, 5}, //Structure Type 11 Offset 5, OEM String 1st
//        {11, 6}, //Structure Type 11 Offset 6, OEM String 2nd
//        {11, 7}, //Structure Type 11 Offset 7, OEM String 3rd
//        {11, 8}, //Structure Type 11 Offset 8, OEM String 4th
//        {11, 9}  //Structure Type 11 Offset 9, OEM String 5th
//  };
//
//  *UpdateableStringCount = (sizeof(UpdatableStrings) / sizeof(DMI_UPDATABLE_STRING));
//  gBS->AllocatePool (EfiBootServicesData, sizeof (UpdatableStrings), (VOID **)mUpdatableStrings);
//  CopyMem ((*mUpdatableStrings), UpdatableStrings, sizeof (UpdatableStrings));
//
//  return EFI_MEDIA_CHANGED;
//
  /*++
    Todo:
      Add project specific code in here.
  --*/
  
  return EFI_UNSUPPORTED;
//[-end-181005-IB07401025-modify]//
}
