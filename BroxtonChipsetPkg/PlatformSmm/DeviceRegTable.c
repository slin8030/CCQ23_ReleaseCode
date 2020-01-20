/** @file

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Base.h>
#include <DeviceRegTable.h>
#include <ScAccess.h>
#include <Library/CpuIA32.h>



//*******************************************************************************************
//
// PMC registers
//
//[-start-160802-IB03090430-modify]//
UINT32 SavePMCRegisters[] = {
                    R_PMC_ETR,      // For B_PCH_PMC_PMIR_CF9LOCK bit
                    (UINT32)-1
                    };
//[-end-160802-IB03090430-modify]//

UINT32 PMCRegistersSave[sizeof(SavePMCRegisters) / sizeof(UINT32) - 1] = {0};

//[-start-160421-IB10860195-remove]//
//[-start-160411-IB10860194-add]//
//UINT32 SaveSmiEnRegisters[] = {
//                    R_SMI_EN,      // For B_PCH_PMC_PMIR_CF9LOCK bit
//                    (UINT32)-1
//                    };
//UINT32 SmiEnRegistersSave[sizeof(SaveSmiEnRegisters) / sizeof(UINT32) - 1] = {0};
//[-end-160411-IB10860194-add]//
//[-end-160421-IB10860195-remove]//

//*******************************************************************************************
//
//  MTRR registers
//
UINT32
FixedMtrr[] = {
        0x250,                           // EFI_IA32_MTRR_FIX64K_00000,
        0x258,                           // EFI_IA32_MTRR_FIX16K_80000,
        0x259,                           // EFI_IA32_MTRR_FIX16K_A0000,
        0x268,                           // EFI_IA32_MTRR_FIX4K_C0000,
        0x269,                           // EFI_IA32_MTRR_FIX4K_C8000,
        0x26A,                           // EFI_IA32_MTRR_FIX4K_D0000,
        0x26B,                           // EFI_IA32_MTRR_FIX4K_D8000,
        0x26C,                           // EFI_IA32_MTRR_FIX4K_E0000,
        0x26D,                           // EFI_IA32_MTRR_FIX4K_E8000,
        0x26E,                           // EFI_IA32_MTRR_FIX4K_F0000,
        0x26F,                           // EFI_IA32_MTRR_FIX4K_F8000,
        0xFFFFFFFF
};

UINT64 FixedMtrrSave [sizeof (FixedMtrr) / 4];
UINT64 VariableMtrrSave [EFI_CACHE_VARIABLE_MTRR_END - EFI_CACHE_VARIABLE_MTRR_BASE + 1];

