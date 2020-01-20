//
// This file contains 'Framework Code' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.
//
/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SmmIoLib.h

Abstract:

  This library provides SMM functions for IO and PCI IO access.
  These can be used to save size and simplify code.
  All contents must be runtime and SMM safe.

--*/

#ifndef _SMM_IO_LIB_H_
#define _SMM_IO_LIB_H_

//#include "EdkIIGlueDxe.h"
#include "PiDxe.h"
#include <Protocol/SmmCpuIo2.h>
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
//
// Utility consumed protocols
//
#include <Protocol/SmmBase2.h>
//#include EFI_PROTOCOL_DEFINITION (SmmBase)

//
// Global variables that must be defined and initialized to use this library
//
extern EFI_SMM_SYSTEM_TABLE2                *mSmst;

//
// Pci I/O related data structure deifinition
//
typedef enum {
  SmmPciWidthUint8  = 0,
  SmmPciWidthUint16 = 1,
  SmmPciWidthUint32 = 2,
  SmmPciWidthUint64 = 3,
  SmmPciWidthMaximum
} SMM_PCI_IO_WIDTH;

#define SMM_PCI_ADDRESS(bus,dev,func,reg) \
   ((UINT64) ( (((UINT32)bus) << 24) + \
               (((UINT32)dev) << 16) + \
               (((UINT32)func) << 8) + \
               ( (UINT32)reg)) )

typedef struct {
  UINT8     Register;
  UINT8     Function;
  UINT8     Device;
  UINT8     Bus;
  UINT32    ExtendedRegister;
} SMM_PCI_IO_ADDRESS;

//
// CPU I/O Access Functions
// 

UINT8
SmmIoRead8 (
  IN  UINT16    Address
  );
  
VOID
SmmIoWrite8 (
  IN  UINT16    Address,
  IN  UINT8     Data
  );
  
UINT16
SmmIoRead16 (
  IN  UINT16    Address
  );

VOID
SmmIoWrite16 (
  IN  UINT16    Address,
  IN  UINT16    Data
  );

UINT32
SmmIoRead32 (
  IN  UINT16    Address
  );

VOID
SmmIoWrite32 (
  IN  UINT16    Address,
  IN  UINT32    Data
  );

VOID
SmmMemWrite8 (
  IN  UINT64    Dest,
  IN  UINT8     Data
  );

UINT8
SmmMemRead8 (
  IN  UINT64    Dest
  );

VOID
SmmMemWrite16 (
  IN  UINT64    Dest,
  IN  UINT16    Data
  );

UINT16
SmmMemRead16 (
  IN  UINT64    Dest
  );

VOID
SmmMemWrite32 (
  IN  UINT64    Dest,
  IN  UINT32    Data
  );

UINT32
SmmMemRead32 (
  IN  UINT64    Dest
  );

VOID
SmmMemAnd32 (
  IN  UINT64    Dest,
  IN  UINT32    Data
  );
//
// Pci Configuration Space access functions definition
//

EFI_STATUS
SmmPciCfgRead (
  IN SMM_PCI_IO_WIDTH       Width,
  IN SMM_PCI_IO_ADDRESS     *Address,
  IN OUT VOID               *Buffer
  );
/*++
  
Routine Description:

  Read value from the specified PCI config space register
    
Arguments:

  Width   - The width (8, 16 or 32 bits) of accessed pci config space register
  Address - The address of the accessed pci register (bus, dev, func, offset)
  Buffer  - The returned value
            
Returns:

  EFI_SUCCESS           - All operations successfully
  EFI_INVALID_PARAMETER - Width is not valid or dosn't match register address
  Other error code      - If any error occured when calling libiary functions 
    
--*/

EFI_STATUS
SmmPciCfgWrite (
  IN SMM_PCI_IO_WIDTH       Width,
  IN SMM_PCI_IO_ADDRESS     *Address,
  IN OUT VOID               *Buffer
  );
/*++

Routine Description:

  Write value into the specified PCI config space register
    
Arguments:

  Width   - The width (8, 16 or 32 bits) of accessed pci config space register
  Address - The address of the accessed pci register (bus, dev, func, offset)
  Buffer  - The returned value
            
Returns:

  EFI_SUCCESS           - All operations successfully
  EFI_INVALID_PARAMETER - Width is not valid or dosn't match register address
  Other error code      - If any error occured when calling libiary functions 
  
--*/

#endif
