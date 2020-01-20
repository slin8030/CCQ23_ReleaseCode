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

#ifndef _DEVICE_REGISTER_TABLE_H
#define _DEVICE_REGISTER_TABLE_H

//[-start-160421-IB10860195-remove]//
//[-start-160411-IB10860194-add]//
//extern UINT32 SaveSmiEnRegisters[];
//extern UINT32 SmiEnRegistersSave[];
//[-end-160411-IB10860194-add]//
//[-end-160421-IB10860195-remove]//

//*******************************************************************************************
//
// PMC registers
//
extern UINT32 SavePMCRegisters[];
extern UINT32 PMCRegistersSave[];

//*******************************************************************************************
//
//  MTRR registers
//
#define L2_ENABLE                       0x101
#define EFI_MTRR_DEF_TYPE_ENABLE        0xC00
//#define EFI_CACHE_VARIABLE_MTRR_END     0x213
//#define EFI_CACHE_VARIABLE_MTRR_BASE    0x200
//#define EFI_CACHE_IA32_MTRR_DEF_TYPE    0x2FF
#define EFI_MSR_IA32_APIC_BASE          0x1B
#define EFI_APIC_GLOBAL_ENABLE          0x800
//#define EFI_CACHE_MTRR_VALID            0x800

extern UINT32 FixedMtrr        [];

extern UINT64 FixedMtrrSave    [];
extern UINT64 VariableMtrrSave [];


#endif
