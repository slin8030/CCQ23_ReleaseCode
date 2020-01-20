/** @file

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

/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PlatformMemorySize.h

Abstract:

  Platform Memory Size PPI as defined in Tiano

  PPI for describing the minimum platform memory size in order to successfully
  pass control into DXE

--*/

#ifndef _PEI_PLATFORM_MEMORY_SIZE_H_
#define _PEI_PLATFORM_MEMORY_SIZE_H_

#define PEI_PLATFORM_MEMORY_SIZE_PPI_GUID \
  { \
    0x9a7ef41e, 0xc140, 0x4bd1, 0xb8, 0x84, 0x1e, 0x11, 0x24, 0xb, 0x4c, 0xe6 \
  }

typedef struct _PEI_PLATFORM_MEMORY_SIZE_PPI PEI_PLATFORM_MEMORY_SIZE_PPI;

typedef
EFI_STATUS
(EFIAPI *PEI_GET_MINIMUM_PLATFORM_MEMORY_SIZE) (
  IN      EFI_PEI_SERVICES                       **PeiServices,
  IN PEI_PLATFORM_MEMORY_SIZE_PPI                * This,
  IN OUT  UINT64                                 *MemorySize
  );

struct _PEI_PLATFORM_MEMORY_SIZE_PPI {
  PEI_GET_MINIMUM_PLATFORM_MEMORY_SIZE  GetPlatformMemorySize;
};

extern EFI_GUID gPeiPlatformMemorySizePpiGuid;

#endif
