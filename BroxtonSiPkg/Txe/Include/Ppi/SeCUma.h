/** @file
  Interface definition details for SEC and UMA.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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

@par Specification
**/

#ifndef _SEC_UMA_PPI_H_
#define _SEC_UMA_PPI_H_

#define SEC_UMA_PPI_GUID \
  { \
    0xcbd86677, 0x362f, 0x4c04, 0x94, 0x59, 0xa7, 0x41, 0x32, 0x6e, 0x05, 0xcf \
  }

// {1E30E33D-1854-437a-BD68-FC1553AA8BE4}
#define CSE_EMMC_SELECT_PPI_GUID \
  { \
    0x1e30e33d, 0x1854, 0x437a, 0xbd, 0x68, 0xfc, 0x15, 0x53, 0xaa, 0x8b, 0xe4 \
  }

// {C5A6189E-8C33-4ac6-AE9A-AED18CABE26D}
#define CSE_UFS_SELECT_PPI_GUID  \
  { \
    0xc5a6189e, 0x8c33, 0x4ac6, 0xae, 0x9a, 0xae, 0xd1, 0x8c, 0xab, 0xe2, 0x6d \
  }

// {D35EDA81-07D0-4142-9409-B07233ED2D07}
#define CSE_SPI_SELECT_PPI_GUID  \
  { \
    0xd35eda81, 0x07d0, 0x4142, 0x94, 0x9,  0xb0, 0x72, 0x33, 0xed, 0x2d, 0x7 \
  }

extern EFI_GUID  gSeCUmaPpiGuid;
extern EFI_GUID  gCseEmmcSelectPpiGuid;
extern EFI_GUID  gCseUfsSelectPpiGuid;
extern EFI_GUID  gCseSpiSelectPpiGuid;

//
// Revision
//
#define SEC_UMA_PPI_REVISION 1

//
// define the MRC recommended boot modes.
//
typedef enum {
  s3Boot, // In current implementation, bmS3 == bmWarm
  warmBoot,
  coldBoot,
  fastBoot,
} MRC_BOOT_MODE_T;

typedef
EFI_STATUS
(EFIAPI *SEC_SEND_UMA_SIZE) (
  IN EFI_PEI_SERVICES **PeiServices
  );

typedef
EFI_STATUS
(EFIAPI *SEC_CONFIG_DID_REG) (
  IN CONST EFI_PEI_SERVICES **PeiServices,
  MRC_BOOT_MODE_T           MrcBootMode,
  UINT8                     InitStat,
  UINT32                    SeCUmaBase,
  UINT32                    *SeCUmaSize
  );

typedef
EFI_STATUS
(EFIAPI *SEC_TAKE_OWNER_SHIP) (
    );

///
/// Interface definition details for SEC and UMA
///
typedef struct SEC_UMA_PPI {
  SEC_SEND_UMA_SIZE      SeCSendUmaSize;
  SEC_CONFIG_DID_REG     SeCConfigDidReg;
  SEC_TAKE_OWNER_SHIP    SeCTakeOwnerShip;
} SEC_UMA_PPI;

#endif
