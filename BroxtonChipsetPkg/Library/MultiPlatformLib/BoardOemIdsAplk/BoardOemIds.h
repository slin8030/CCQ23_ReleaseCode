/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/**@file
  ACPI oem ids setting for multiplatform.

  This file includes package header files, library classes.

  Copyright (c) 2010-2016, Intel Corporation. All rights reserved.<BR>
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.
**/

#include <Guid/PlatformInfo_Apl.h>
#include <Library/BaseMemoryLib.h>

#define EFI_ACPI_OEM_ID_DEFAULT    SIGNATURE_64('I', 'N', 'T', 'E', 'L', ' ', ' ', ' ')     // max 6 chars
#define EFI_ACPI_OEM_ID1           SIGNATURE_64('I', 'N', 'T', 'E', 'L', '1', ' ', ' ')     // max 6 chars
#define EFI_ACPI_OEM_ID2           SIGNATURE_64('I', 'N', 'T', 'E', 'L', '2', ' ', ' ')     // max 6 chars

#define EFI_ACPI_OEM_TABLE_ID_DEFAULT   SIGNATURE_64('E', 'D', 'K', '2', ' ', ' ', ' ', ' ')
#define EFI_ACPI_OEM_TABLE_ID1          SIGNATURE_64('E', 'D', 'K', '2', '_', '1', ' ', ' ')
#define EFI_ACPI_OEM_TABLE_ID2          SIGNATURE_64('E', 'D', 'K', '2', '_', '2', ' ', ' ')


EFI_STATUS
InitializeBoardOemId (
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PLATFORM_INFO_HOB        *PlatformInfoHob
  );
