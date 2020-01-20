/** @file
  The header file of OBB verify PEIM.

@copyright
 Copyright (c) 2010 - 2016 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.

This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.
**/
//
// This file contains an 'Intel Pre-EFI Module' and is licensed
// for Intel CPUs and Chipsets under the terms of your license 
// agreement with Intel or your vendor.  This file may be      
// modified by the user, subject to additional terms of the    
// license agreement                                           
//

#ifndef __OBB_VERIFY_H__
#define __OBB_VERIFY_H__

#include "PiPei.h"
#include <Ppi/Stall.h>
#include <Ppi/RecoveryModule.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Ppi/RecoveryLoader.h>
#include <Ppi/BlockIoPei.h>  //Override
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Guid/RecoveryDevice.h>
#include <Library/BaseCryptLib.h>
#include <MediaDeviceDriverPei.h>
#include <ScRegs/RegsPcu.h>
#include <SeCAccess.h>
#include <HeciRegs.h>
#include <Library/CpuIA32.h>
#include <CpuRegs.h>
#include <FotaCommon.h>
#include <Library/HeciMsgLib.h>
#include <Protocol/Spi.h>
#include <Library/ScPlatformLib.h>

EFI_STATUS
FotaHeciGetBootDevice(
  OUT UINT32        *BootDevice,
  OUT UINT32        *PhyDeviceArea,
  OUT UINT32        *LogicalData
  );

//
// Fota Recovery IFWI Analyze function
//
BOOLEAN
EFIAPI
VerifyFvHash (
  IN UINT8          BpmHash,
  IN UINTN          Buffer,
  IN UINTN          Size,
  IN UINTN          SignAddress
  );

EFI_STATUS
EFIAPI
ProcessBpdtHdr (
  IN  UINT8                    *DataInBuffer,
  IN  UINT32                   BpdtDescType,
  OUT UINT32                   *DataOffset,
  OUT UINT32                   *DataSize
  );

EFI_STATUS
EFIAPI
GetParDirOffsetFromBpdt (
  IN  UINT8                    *DataInBuffer,
  IN  UINT8                    BpdtDescType,
  IN  CHAR8                    *StrName,
  OUT UINT32                   *DataOffset,
  OUT UINT32                   *DataLength
  );

EFI_STATUS
EFIAPI
GetParDirOffset (
  IN  UINT8                    *DataInBuffer,
  IN  CHAR8                    *StrName,
  OUT UINT32                   *DataOffset,
  OUT UINT32                   *DataLength
  );

EFI_STATUS
FvBpmHashVerify (
  IN  UINT8                    *FileBuffer
  );

BOOLEAN
IsCorrectMicrocode (
  IN  UINT8                    *FileBuffer,
  IN  UINT8                    McuPatchIndex
  );

typedef
EFI_STATUS
(EFIAPI *FOTA_EFI_BLOCK_READ_PEI) (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN UINT8                     SelBpNum,
  IN UINTN                     Address,
  IN UINTN                     BufferSize,
  OUT VOID                     *Buffer
  );

typedef
EFI_STATUS
(EFIAPI *FOTA_EFI_BLOCK_WRITE_PEI) (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN UINT8                     SelBpNum,
  IN UINTN                     Address,
  IN UINTN                     BufferSize,
  IN VOID                      *Buffer
  );

typedef struct _FOTA_BLOCK_READ_WRITE_TABLE_PEI {
  FOTA_EFI_BLOCK_READ_PEI      ProcessReadBlock;
  FOTA_EFI_BLOCK_READ_PEI      ProcessWriteBlock;
} FOTA_BLOCK_READ_WRITE_TABLE_PEI;

#define SPI_FD_SIZE           0x1000


#endif

