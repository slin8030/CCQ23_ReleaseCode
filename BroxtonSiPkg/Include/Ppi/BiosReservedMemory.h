/**@file
  Common header file shared by all source files.

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

#ifndef __BIOS_RESERVED_MEMORY_H__
#define __BIOS_RESERVED_MEMORY_H__

//
// BIOS reserved memory config
//
#pragma pack(1)

typedef struct {
  UINT8         Pram;


} BIOS_RESERVED_MEMORY_CONFIG;

#pragma pack()

typedef struct _PEI_BIOS_RESERVED_MEMORY_POLICY_PPI PEI_BIOS_RESERVED_MEMORY_POLICY_PPI;

typedef
EFI_STATUS
(EFIAPI *GET_BIOS_RESERVED_MEMORY_POLICY) (
  IN CONST EFI_PEI_SERVICES               **PeiServices,
  IN PEI_BIOS_RESERVED_MEMORY_POLICY_PPI  *This,
  IN OUT BIOS_RESERVED_MEMORY_CONFIG      *BiosReservedMemoryPolicy
  );

//
//Function Prototypes only - please do not add #includes here
//

  /**
  This function provides a blocking stall for reset at least the given number of microseconds
  stipulated in the final argument.

  @param  PeiServices General purpose services available to every PEIM.
  @param  this Pointer to the local data for the interface.
  @param  Microseconds number of microseconds for which to stall.

  @retval EFI_SUCCESS the function provided at least the required stall.
**/


typedef struct _PEI_BIOS_RESERVED_MEMORY_POLICY_PPI {
  GET_BIOS_RESERVED_MEMORY_POLICY  GetBiosReservedMemoryPolicy;
} PEI_BIOS_RESERVED_MEMORY_POLICY_PPI;

extern EFI_GUID gBiosReservedMemoryPolicyPpiGuid;

#endif //__BIOS_RESERVED_MEMORY_H__
