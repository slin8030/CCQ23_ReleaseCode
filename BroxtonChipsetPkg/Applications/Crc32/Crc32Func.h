/** @file
  Crc32 shell tool.

  Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef __CRC32_FUNC_H__
#define __CRC32_FUNC_H__

#include <Uefi.h>


VOID
Usage(
 VOID
);
 
EFI_STATUS
Crc32Mem(
  IN VOID      		*StartAddr,
  IN UINTN			Length,
  IN OUT UINT32     *CrcOut
  );

EFI_STATUS
Crc32File (
  IN UINT64		    Size64,
  IN OUT UINT32     *CrcOut
  );
  
 #endif