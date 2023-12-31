;; @file
#   IA32 architecture MSRs
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 1999 - 2016 Intel Corporation.
;
;  The source code contained or described herein and all documents related to the
;  source code ("Material") are owned by Intel Corporation or its suppliers or
;  licensors. Title to the Material remains with Intel Corporation or its suppliers
;  and licensors. The Material may contain trade secrets and proprietary and
;  confidential information of Intel Corporation and its suppliers and licensors,
;  and is protected by worldwide copyright and trade secret laws and treaty
;  provisions. No part of the Material may be used, copied, reproduced, modified,
;  published, uploaded, posted, transmitted, distributed, or disclosed in any way
;  without Intel's prior express written permission.
;
;  No license under any patent, copyright, trade secret or other intellectual
;  property right is granted to or conferred upon you by disclosure or delivery
;  of the Materials, either expressly, by implication, inducement, estoppel or
;  otherwise. Any license under such intellectual property rights must be
;  express and approved by Intel in writing.
;
;  Unless otherwise agreed by Intel in writing, you may not remove or alter
;  this notice or any other notice embedded in Materials by Intel or
;  Intel's suppliers or licensors in any way.
;
;  This file contains a 'Sample Driver' and is licensed as such under the terms
;  of your license agreement with Intel or your vendor. This file may be modified
;  by the user, subject to the additional terms of the license agreement.
;
; @par Specification
;;
#
#------------------------------------------------------------------------------

#
# WSM CPUID vs. stepping
#
.equ WSM_A0_STEPPING,               0x206C0
.equ IA32_MTRR_CAP,                 0xFE
.equ MTRR_PHYS_BASE_0,              0x200
.equ MTRR_PHYS_MASK_0,              0x201
.equ MTRR_PHYS_BASE_1,              0x202
.equ MTRR_PHYS_MASK_1,              0x203
.equ MTRR_PHYS_BASE_2,              0x204
.equ MTRR_PHYS_MASK_2,              0x205
.equ MTRR_PHYS_BASE_3,              0x206
.equ MTRR_PHYS_MASK_3,              0x207
.equ MTRR_PHYS_BASE_4,              0x208
.equ MTRR_PHYS_MASK_4,              0x209
.equ MTRR_PHYS_BASE_5,              0x20A
.equ MTRR_PHYS_MASK_5,              0x20B
.equ MTRR_PHYS_BASE_6,              0x20C
.equ MTRR_PHYS_MASK_6,              0x20D
.equ MTRR_PHYS_BASE_7,              0x20E
.equ MTRR_PHYS_MASK_7,              0x20F
.equ MTRR_PHYS_BASE_8,              0x210
.equ MTRR_PHYS_MASK_8,              0x211
.equ MTRR_PHYS_BASE_9,              0x212
.equ MTRR_PHYS_MASK_9,              0x213
.equ MTRR_FIX_64K_00000,            0x250
.equ MTRR_FIX_16K_80000,            0x258
.equ MTRR_FIX_16K_A0000,            0x259
.equ MTRR_FIX_4K_C0000,             0x268
.equ MTRR_FIX_4K_C8000,             0x269
.equ MTRR_FIX_4K_D0000,             0x26A
.equ MTRR_FIX_4K_D8000,             0x26B
.equ MTRR_FIX_4K_E0000,             0x26C
.equ MTRR_FIX_4K_E8000,             0x26D
.equ MTRR_FIX_4K_F0000,             0x26E
.equ MTRR_FIX_4K_F8000,             0x26F
.equ MTRR_DEF_TYPE,                 0x2FF

.equ MTRR_MEMORY_TYPE_UC,           0x00
.equ MTRR_MEMORY_TYPE_WC,           0x01
.equ MTRR_MEMORY_TYPE_WT,           0x04
.equ MTRR_MEMORY_TYPE_WP,           0x05
.equ MTRR_MEMORY_TYPE_WB,           0x06

.equ MTRR_DEF_TYPE_E,               0x0800
.equ MTRR_DEF_TYPE_FE,              0x0400
.equ MTRR_PHYSMASK_VALID,           0x0800

#
# Define the high 32 bits of MTRR masking
# This should be read from CPUID EAX = 080000008h, EAX bits [7:0]
# But for most platforms this will be a fixed supported size so it is 
# fixed to save space.
#
.equ MTRR_PHYS_MASK_VALID,          0x0800
.equ MTRR_PHYS_MASK_HIGH,           0x0000000F      # For 36 bit addressing

.equ IA32_MISC_ENABLE,              0x1A0
.equ FAST_STRING_ENABLE_BIT,        0x01

.equ CR0_CACHE_DISABLE,             0x40000000
.equ CR0_NO_WRITE,                  0x20000000

.equ IA32_PLATFORM_ID,              0x17 
.equ IA32_BIOS_UPDT_TRIG,           0x79 
.equ IA32_BIOS_SIGN_ID,             0x8B 
.equ PLATFORM_INFO,                 0xCE
.equ NO_EVICT_MODE,                 0x2E0
.equ NO_EVICTION_ENABLE_BIT,        0x01

#
# MSR definitions
#
.equ MSR_IA32_PLATFORM_ID,          0x017
.equ MSR_APIC_BASE,                 0x01B
.equ MSR_SOCKET_ID,                 0x039
.equ MSR_IA32_FEATURE_CONTROL,      0x03A
.equ MSR_CLOCK_CST_CONFIG_CONTROL,  0x0E2
.equ MSR_CLOCK_FLEX_MAX,            0x194
.equ MSR_IA32_PERF_STS,             0x198
.equ MSR_IA32_PERF_CTL,             0x199
.equ MSR_IA32_MISC_ENABLES,         0x1A0
.equ MSR_IA32_MC8_MISC2,            0x288
.equ MSR_IA32_MC7_CTL,              0x41C

#
# CSR definitions
#
.equ CSR_DESIRED_CORES,                       0x80            # D0:F0:R80h
.equ CSR_DESIRED_CORES_LOCK_BIT,              16
.equ CSR_DESIRED_CORES_SMT_BIT,               8
.equ CSR_DESIRED_CORES_COUNT_MASK,            7
.equ CSR_DESIRED_SMT_CORE_COUNT_MASK,         0x107

.equ CSR_DESIRED_CORES_HT_DISABLED_BIT,       0xFFFFF100
.equ CSR_DESIRED_CORES_MC_DISABLED_BIT,       0xFFFC
.equ CSR_DESIRED_CORES_HT_DISABLED,           0x100
.equ CSR_DESIRED_CORES_MC_DISABLED,           0x01

.equ CSR_SVID_SDID,                           0x2C            # D0:F0:R2Ch
.equ DEAFULT_SVID_SDID,                       0x80868086      # DWORD Access & Write Once ONLY

#
# Processor MSR definitions
#
.equ MSR_BBL_CR_CTL3,                         0x11E # L2 cache configuration MSR
.equ B_MSR_BBL_CR_CTL3_L2_NOT_PRESENT,        23    # L2 not present
.equ B_MSR_BBL_CR_CTL3_L2_ENABLED,            8     # L2 enabled
.equ B_MSR_BBL_CR_CTL3_L2_HARDWARE_ENABLED,   0     # L2 hardware enabled

#
# Local APIC Register Equates
#
.equ LOCAL_APIC_ID_REG,             0xFEE00020
.equ APIC_ICR_HI,                   0xFEE00310
.equ APIC_ICR_LO,                   0xFEE00300
.equ ANDICRMask,                    0xFFF32000      # AND mask for ICR Saving reserved bits
.equ ORSelfINIT,                    0x00004500      # OR mask to send INIT IPI to itself
.equ ORAllButSelf,                  0x000C0000      # OR mask to set dest field = "All But Self"

#
# Cache control macro
#
.macro DISABLE_CACHE
    movl   %cr0, %eax
    orl    $(CR0_CACHE_DISABLE + CR0_NO_WRITE), %eax
    wbinvd
    movl   %eax, %cr0
.endm

.macro ENABLE_CACHE
    movl   %cr0, %eax
    andl   $(~(CR0_CACHE_DISABLE + CR0_NO_WRITE)), %eax
    wbinvd
    movl   %eax, %cr0
.endm

.equ VENDOR_ID_REG,                           0
.equ PCI_REVISION_ID_REG,                     8
.equ CPU_GENERIC_UNCORE_DEV,                  0
.equ CPU_GENERIC_UNCORE_FUNC,                 0
.equ CPU_LINK_1_DEV,                          2
.equ CPU_LINK_1_FUNC,                         4

.equ B0_CPU_STEPPING,                         0x10

.equ BLOCK_LENGTH_BYTES,                      2048

# define the structure of UpdateHeaderStruc
.struct 0
dHeaderVersion:      .struct dHeaderVersion + 4       #size 4           # Header version#
dUpdateRevision:     .struct dUpdateRevision + 4      #size 4           # Update revision#
dDate:               .struct dDate + 4                #size 4           # Date in binary (08/13/07 as 0x08132007)
dProcessorSignature: .struct dProcessorSignature + 4  #size 4           # CPU type, family, model, stepping
dChecksum:           .struct dChecksum + 4            #size 4           # Checksum
dLoaderRevision:     .struct dLoaderRevision + 4      #size 4           # Update loader version#
dProcessorFlags:     .struct dProcessorFlags + 4      #size 4           # Processor Flags
dDataSize:           .struct dDataSize + 4            #size 4           # Size of encrypted data
dTotalSize:          .struct dTotalSize + 4           #size 4           # Total size of update in bytes
bReserved:           .struct dTotalSize + 12          #size 12          # 12 bytes reserved
# end of UpdateHeaderStruc

# define the structure of HobStruc
.struct 0
Sign:       .struct Sign + 4                          #size 4           # Signiture#
CarBase:    .struct CarBase + 4                       #size 4           # Cache As Ram Base Address
CarSize:    .struct CarSize + 4                       #size 4           # Cache As Ram Size
IBBSource:  .struct IBBSource + 4                     #size 4           # IBB Address in SRAM
IBBBase:    .struct IBBBase + 4                       #size 4           # IBB Base in CAR.
IBBSize:    .struct IBBSize + 4                       #size 4           # IBB Size
IBBLSource: .struct IBBLSource + 4                    #size 4           # IBBL Address in SRAM
IBBLBase:   .struct IBBLBase + 4                      #size 4           # IBBL Base in CAR.
IBBLSize:   .struct IBBLSize + 4                      #size 4           # IBBL Size
# end of HobStruc
