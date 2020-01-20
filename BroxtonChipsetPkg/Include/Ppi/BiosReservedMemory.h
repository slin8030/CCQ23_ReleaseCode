//
// This file contains an 'Intel Pre-EFI Module' and is licensed
// for Intel CPUs and Chipsets under the terms of your license 
// agreement with Intel or your vendor.  This file may be      
// modified by the user, subject to additional terms of the    
// license agreement                                           
//
/**@file
  Common header file shared by all source files.

  This file includes package header files, library classes and protocol, PPI & GUID definitions.

  Copyright (c) 2010, Intel Corporation.
  All rights reserved.
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.
**/

#ifndef __BIOS_RESERVED_MEMORY_H__
#define __BIOS_RESERVED_MEMORY_H__

//
// BIOS reserved memory config
//
#pragma pack(1)

typedef struct {
  UINT8         Pram;
  UINT16        SgxMode;
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
