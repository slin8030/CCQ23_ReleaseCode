/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/


/*++
Module Name:

  Emmc.h

Abstract:

  Media Device Driver header

--*/
#ifndef _EMMC_H_
#define _EMMC_H_

#include <Library/ShellLib.h>
#include <MediaDeviceDriver.h>
#include <Protocol/EmmcCardInfoProtocol.h>
#include <Library/BaseLib.h>

VOID
EFIAPI
HelpMessage (
  );

EFI_STATUS
EFIAPI
MmcDecodeCID (
  IN CID *CIDReg
  );

EFI_STATUS
EFIAPI
MmcDecodeCSD (
  IN CSD * CSDReg
  );

EFI_STATUS
EFIAPI
MmcDecodeExtCSD (
  EXT_CSD * ExtCSDReg
  );

EFI_STATUS
EFIAPI
MmcDecodeOCR (
  IN OCR *OCRReg
  );
#endif
