/** @file
  GenericUtilityLib

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

/** 
  declares interface functions

Copyright (c) 2004 - 2008, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _EFI_BDS_BBS_SUPPORT_H_
#define _EFI_BDS_BBS_SUPPORT_H_

#include "BootMaint.h"


/**
  Group the legacy boot options in the BootOption.

  The routine assumes the boot options in the beginning that covers all the device 
  types are ordered properly and re-position the following boot options just after
  the corresponding boot options with the same device type.
  For example:
  1. Input  = [Harddisk1 CdRom2 Efi1 Harddisk0 CdRom0 CdRom1 Harddisk2 Efi0]
     Assuming [Harddisk1 CdRom2 Efi1] is ordered properly
     Output = [Harddisk1 Harddisk0 Harddisk2 CdRom2 CdRom0 CdRom1 Efi1 Efi0]

  2. Input  = [Efi1 Efi0 CdRom1 Harddisk0 Harddisk1 Harddisk2 CdRom0 CdRom2]
     Assuming [Efi1 Efi0 CdRom1 Harddisk0] is ordered properly
     Output = [Efi1 Efi0 CdRom1 CdRom0 CdRom2 Harddisk0 Harddisk1 Harddisk2]

  @param BootOption      Pointer to buffer containing Boot Option Numbers
  @param BootOptionCount Count of the Boot Option Numbers
**/
VOID
GroupMultipleLegacyBootOption4SameType (
  UINT16                   *BootOption,
  UINTN                    BootOptionCount
  );

/**
  Re-order the Boot Option according to the DevOrder.

  The routine re-orders the Boot Option in BootOption array according to
  the order specified by DevOrder.

  @param BootOption         Pointer to buffer containing the Boot Option Numbers
  @param BootOptionCount    Count of the Boot Option Numbers
  @param DevOrder           Pointer to buffer containing the BBS Index,
                            high 8-bit value 0xFF indicating a disabled boot option
  @param DevOrderCount      Count of the BBS Index
  @param EnBootOption       Pointer to buffer receiving the enabled Boot Option Numbers
  @param EnBootOptionCount  Count of the enabled Boot Option Numbers
  @param DisBootOption      Pointer to buffer receiving the disabled Boot Option Numbers
  @param DisBootOptionCount Count of the disabled Boot Option Numbers
**/
VOID
OrderLegacyBootOption4SameType (
  UINT16                   *BootOption,
  UINTN                    BootOptionCount,
  UINT16                   *DevOrder,
  UINTN                    DevOrderCount,
  UINT16                   *EnBootOption,
  UINTN                    *EnBootOptionCount,
  UINT16                   *DisBootOption,
  UINTN                    *DisBootOptionCount
  );
#endif
