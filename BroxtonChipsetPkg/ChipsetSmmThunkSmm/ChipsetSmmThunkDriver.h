/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

/*++

Copyright (c)  2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SmmThunkDriver.h

Abstract:

  Header file for SMM Thunk Driver.

Revision History

--*/

#ifndef _CHIPSET_SMM_THUNK_DRIVER_H
#define _CHIPSET_SMM_THUNK_DRIVER_H

#include <Uefi.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmThunk.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PciCf8Lib.h>
#include <Library/TimerLib.h>
#include <Library/IoLib.h>
#include <ThunkLib.h>
#include <CpuRegs.h>
#include <ScAccess.h>
#include <MmioAccess.h>

//
// Driver Consumed Protocol Prototypes
//

#define LOW_STACK_SIZE            (EFI_PAGE_SIZE * 1)

#define EFI_SMM_THUNK_SIGNATURE SIGNATURE_32 ('S', 'M', 'T', 'H')
#define SMM_INT10_ENABLE
//
// BDA and EBDA  
//
#define EBDA(a)                         (*(UINT8*)(UINTN)(((*(UINT16*)(UINTN)0x40e) << 4) + (a)))
#define BDA(a)                          (*(UINT16*)((UINTN)0x400 +(a)))

typedef struct {
  UINT32                          Signature;
  EFI_HANDLE                      Handle;
  EFI_SMM_THUNK_PROTOCOL          SmmThunk;
  THUNK16_CONTEXT                 ThunkContext;
  EFI_PHYSICAL_ADDRESS            RealModeBuffer;
  UINTN                           BufferSize;
} SMM_THUNK_PRIVATE_DATA;

//
// Prototypes
//
/**
 Initializes the Chipset SMM Thunk Driver

 @param [in]   ImageHandle      Handle for the image of this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval EFI_SUCCESS            SMM thunk driver has been initialized successfully
 @return Other        SMM thunk driver init failed

**/
EFI_STATUS
EFIAPI
InitializeChipsetSmmThunkProtocol (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

/**
 Thunk to 16-bit real mode and call Segment:Offset. Regs will contain the
 16-bit register context on entry and exit. Arguments can be passed on
 the Stack argument

 @param [in]   This             Protocol instance pointer.
 @param [in]   Segment          Segemnt of 16-bit mode call
 @param [in]   Offset           Offset of 16-bit mdoe call
 @param [in]   Regs
 @param [in]   Stack            Caller allocated stack used to pass arguments
 @param [in]   StackSize        Size of Stack in bytes

 @retval FALSE                  Thunk completed, and there were no BIOS errors in the target code.
                                See Regs for status.
 @retval TRUE                   There was a BIOS erro in the target code.

**/
BOOLEAN
EFIAPI
SmmFarCall86 (
  IN  EFI_SMM_THUNK_PROTOCOL                   *This,
  IN  UINT16                                   Segment,
  IN  UINT16                                   Offset,
  IN  EFI_IA32_REGISTER_SET                    *Regs,
  IN  VOID                                     *Stack,
  IN  UINTN                                    StackSize
  );

/**
 Thunk to 16-bit Int10 real mode. Regs will contain the
 16-bit register context on entry and exit. Arguments can be passed on
 the Stack argument

 @param [in]   This             Protocol instance pointer.
 @param [in, out] Regs          Register contexted passed into (and returned) from thunk to
                                16-bit mode

 @retval FALSE                  Thunk completed, and there were no BIOS errors in the target code.
                                See Regs for status.
 @retval TRUE                   There was a BIOS erro in the target code.

**/
BOOLEAN
EFIAPI
SmmInt10 (
   IN EFI_SMM_THUNK_PROTOCOL            *This,
   IN OUT EFI_IA32_REGISTER_SET         *Regs
   );

/**
 Thunk to 16-bit Int10 real mode. Regs will contain the
 16-bit register context on entry and exit. Arguments can be passed on
 the Stack argument

 @param [in]   This             Protocol instance pointer.
 @param [in]   IntNumber        Int vector number
 @param [in, out] Regs          Register contexted passed into (and returned) from thunk to
                                16-bit mode

 @retval FALSE                  Thunk completed, and there were no BIOS errors in the target code.
                                See Regs for status.
 @retval TRUE                   There was a BIOS error in the target code.

**/
BOOLEAN
EFIAPI
SmmInt86 (
   IN EFI_SMM_THUNK_PROTOCOL            *This,
   IN UINT8                             IntNumber,
   IN OUT EFI_IA32_REGISTER_SET         *Regs
   );
#endif
