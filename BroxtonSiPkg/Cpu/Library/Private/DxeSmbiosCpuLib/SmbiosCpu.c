/** @file
  This library will determine processor configuration information to
  create SMBIOS CPU tables appropriately.

@copyright
 Copyright (c) 2014 - 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.

@par Specification Reference:
  System Management BIOS (SMBIOS) Reference Specification v2.8.0
  dated 2013-Mar-28 (DSP0134)
  http://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.8.0.pdf
**/

#include "SmbiosCpu.h"

/**
  This function installs SMBIOS type 7 (cache) entries, and an SMBIOS type 4 (processor) entry.

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_NOT_FOUND         - if a protocol or the Hii PackageList could not be located.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
EFIAPI
SmbiosCpu (
  VOID
  )
{
  EFI_STATUS           Status;

  Status = InstallSmbiosType7 ();
  ASSERT_EFI_ERROR (Status);

  Status = InstallSmbiosType4 ();
  ASSERT_EFI_ERROR (Status);
  
  return Status;
}