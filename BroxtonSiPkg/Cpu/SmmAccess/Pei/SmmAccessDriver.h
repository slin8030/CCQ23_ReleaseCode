/** @file
  Header file for SMM Access Driver.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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


#ifndef _PEI_SMM_ACCESS_DRIVER_H
#define _PEI_SMM_ACCESS_DRIVER_H

#include "PiPei.h"
#include "Library/HobLib.h"
#include "Guid/SmramMemoryReserve.h"
#include "Ppi/SmmAccess.h"
#include "Library/DebugLib.h"
#include "Library/BaseMemoryLib.h"
#include "SaAccess.h"

//
// Chipset specific constants
//
#define MAX_SMRAM_RANGES 2

//
// Chipset register(s)
//
typedef UINT8 SMRAM;        // System Management RAM Control

//
// Private data structure
//
#define  SMM_ACCESS_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('i', 's', 'm', 'a')

typedef struct {
  UINTN                           Signature;
  EFI_HANDLE                      Handle;
  PEI_SMM_ACCESS_PPI              SmmAccess;
  //
  // Local Data for SMM Access interface goes here
  //
  UINTN                           NumberRegions;
  EFI_SMRAM_DESCRIPTOR            SmramDesc[MAX_SMRAM_RANGES];
} SMM_ACCESS_PRIVATE_DATA;

#define SMM_ACCESS_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, SMM_ACCESS_PRIVATE_DATA, SmmAccess, SMM_ACCESS_PRIVATE_DATA_SIGNATURE)


/**
  Routine Description:
    This is the constructor for the SMM Access Ppi

  @param[in] FileHandle      - FileHandle.
  @param[in] PeiServices     - General purpose services available to every PEIM.

  @retval EFI_SUCCESS        - Protocol successfully started and installed
  @retval EFI_UNSUPPORTED    - Protocol can't be started
**/
EFI_STATUS
EFIAPI
SmmAccessDriverEntryPoint(
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  );

/**
  Routine Description:

    This routine accepts a request to "open" a region of SMRAM.  The
    region could be legacy ABSEG, HSEG, or TSEG near top of physical memory.
    The use of "open" means that the memory is visible from all PEIM
    and SMM agents.

  @param[in] PeiServices           - General purpose services available to every PEIM
  @param[in] This                  - Pointer to the SMM Access Interface
  @param[in] DescriptorIndex       - Region of SMRAM to Open

  @retval EFI_SUCCESS              - The region was successfully opened.
  @retval EFI_DEVICE_ERROR         - The region could not be opened because locked by chipset.
  @retval EFI_INVALID_PARAMETER    - The descriptor index was out of bounds.
**/
EFI_STATUS
EFIAPI
Open (
  IN EFI_PEI_SERVICES        **PeiServices,
  IN PEI_SMM_ACCESS_PPI      *This,
  IN UINTN                   DescriptorIndex
  );

/**
  Routine Description:

  This routine accepts a request to "close" a region of SMRAM.  This is valid for
  compatible SMRAM region.

  @param[in] PeiServices           - General purpose services available to every PEIM
  @param[in] This                  - Pointer to the SMM Access Interface
  @param[in] DescriptorIndex       - Region of SMRAM to Close

  @retval EFI_SUCCESS              - The region was successfully closed.
  @retval EFI_DEVICE_ERROR         - The region could not be closed because locked by chipset.
  @retval EFI_INVALID_PARAMETER    - The descriptor index was out of bounds.
**/
EFI_STATUS
EFIAPI
Close (
  IN EFI_PEI_SERVICES        **PeiServices,
  IN PEI_SMM_ACCESS_PPI      *This,
  IN UINTN                   DescriptorIndex
  );

/**
  Routine Description:

    This routine accepts a request to "lock" SMRAM.  The
    region could be legacy AB or TSEG near top of physical memory.
    The use of "lock" means that the memory can no longer be opened
    to PEIM.

  @param[in] PeiServices           - General purpose services available to every PEIM.
  @param[in] This                  - Pointer to the SMM Access Interface.
  @param[in] DescriptorIndex       - Region of SMRAM to Lock.

  @retval EFI_SUCCESS              - The region was successfully locked.
  @retval EFI_DEVICE_ERROR         - The region could not be locked because at least one range is still open.
  @retval EFI_INVALID_PARAMETER    - The descriptor index was out of bounds.
**/
EFI_STATUS
EFIAPI
Lock (
  IN EFI_PEI_SERVICES        **PeiServices,
  IN PEI_SMM_ACCESS_PPI      *This,
  IN UINTN                   DescriptorIndex
  );

/**
  Routine Description:

    This routine services a user request to discover the SMRAM
    capabilities of this platform.  This will report the possible
    ranges that are possible for SMRAM access, based upon the
    memory controller capabilities.

  @param[in] PeiServices           - General purpose services available to every PEIM.
  @param[in] This                  - Pointer to the SMRAM Access Interface.
  @param[in, out] SmramMapSize     - Pointer to the variable containing size of the
                                     buffer to contain the description information.
  @param[in, out] SmramMap         - Buffer containing the data describing the Smram
                                     region descriptors.

  @retval EFI_BUFFER_TOO_SMALL     - The user did not provide a sufficient buffer.
  @retval EFI_SUCCESS              - The user provided a sufficiently-sized buffer.
**/
EFI_STATUS
EFIAPI
GetCapabilities (
  IN EFI_PEI_SERVICES            **PeiServices,
  IN PEI_SMM_ACCESS_PPI          *This,
  IN OUT   UINTN                       *SmramMapSize,
  IN OUT   EFI_SMRAM_DESCRIPTOR        *SmramMap
  );

#endif
