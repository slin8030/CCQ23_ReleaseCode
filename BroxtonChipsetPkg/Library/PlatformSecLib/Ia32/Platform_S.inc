;; @file
#   Platform Specific Definitions
; @copyright
;  INTEL CONFIDENTIAL
;  Copyright 2008 - 2016 Intel Corporation.
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
#INCLUDE FlashMap.inc #ECP porting

.equ MKF_SOFTSDV_FLAG,              0

# Set "MINIMUM_BOOT_SUPPORT" flag allows BIOS boot as minimum feature in SEC phase.
.equ MINIMUM_BOOT_SUPPORT,          0               # ="0", Normal Boot#
                                                    # ="1", Minimum Feature Boot
# "RESET_IN_SEC" flag allows BIOS doing RESET in SEC phase
.equ RESET_IN_SEC,                  0               # ="0", RESET occurs in OemIohInit.c
                                                    # ="1", RESET occurs in SEC phase

.equ EARLY_MICROCODE_SUPPORT,       1
.equ DETERMINISTIC_BSP_SUPPORT,     0
.equ DEBUG,                         1

#
# IO port to access the upper 128-byte of RTC RAM
#
.equ RTC_UPPER_INDEX,               0x72
.equ RTC_UPPER_DATA,                0x73

#
# Offset of data stored in the upper 128-byte of RTC RAM.
#
.equ CMOS_CPU_BSP_SELECT,           0x10            # BspSelection
.equ CMOS_CPU_UP_MODE,              0x11            # UpBootSelection

#
# Cpu Ratio and Vid stored in the upper 128-byte of RTC RAM.
#
.equ CMOS_CPU_RATIO_OFFSET,         0x12            # ProcessorFlexibleRatio
.equ CMOS_CPU_CORE_HT_OFFSET,       0x13            # ProcessorHyperThreadingEnable & EnableCoresInSbsp & EnableCoresInNbsp

#
# CPU Feature
#
.equ CMOS_CPU_BIST_OFFSET,          0x15            # ProcessorBistEnable
.equ CMOS_CPU_VMX_OFFSET,           0x16            # ProcessorVmxEnable

#
# Port80 Selection
#
.equ CMOS_PORT80_OFFSET,            0x17            # Port80Route

.equ BIOS_REGION_UPDATABLE_STATUS,  0x58            # Offset
#----------------------------------------------------------------------------------------
# "Merlin" support used equates
#----------------------------------------------------------------------------------------
.equ MAGIC_ADDRESS_IN_SEG,          0x0FFF0
.equ MAGIC_SEG,                     0x0F000

#
# -- Equates for CAR initialization
#    TileSize (must be a power of 2)
#
# Define the tile size
# The tile size and tile placement are critical to ensuring that no data loss occurs
# See BWG - chapter "Determining Tile Size"
#
.equ TILE_SIZE,                     0x00000000

#
# See BWG - chapter "Determining Cacheable Code Region Base Addresses and Ranges".
#
# Now FvRecovery base address and size is determined by PcdFlashFvIBBMBase and PcdFlashFvIBBMSize.
#

.equ WDB_REGION_BASE_ADDRESS,       0x40000000
.equ WDB_REGION_SIZE,               0x1000
.equ WDB_REGION_SIZE_MASK,          (~(WDB_REGION_SIZE - 1))

#
# See BWG - chapter "Determining Data Stack Base Address and Range"
#
# Now Data Stack base address and size is determined by PcdTemporaryRamBase and PcdTemporaryRamSize.
#

#
# Cache init and test values
# These are inverted to flip each bit at least once
#
.equ CACHE_INIT_VALUE,              0xA5A5A5A5

#ECP porting
.equ CACHE_TEST_VALUE,              0x5aa55aa5

.equ PEI_CORE_ENTRY_BASE,           0xFFFFFFE0
.equ FV_MAIN_BASE,                  0xFFFFFFFC

.equ MAX_NR_BUS,                    0xFF
.equ MAX_NR_CPU_SOCKETS,            2               # DP example, MP may have 4 or more

.equ BIT0,                          0x01
.equ BIT1,                          0x02
.equ BIT2,                          0x04
.equ BIT3,                          0x08
.equ BIT4,                          0x10
.equ BIT5,                          0x20
.equ BIT6,                          0x40
.equ BIT7,                          0x80
.equ BIT8,                          0x100
.equ BIT9,                          0x200
.equ BIT10,                         0x400
.equ BIT11,                         0x800
.equ BIT12,                         0x1000
.equ BIT13,                         0x2000
.equ BIT14,                         0x4000
.equ BIT15,                         0x8000
.equ BIT16,                         0x10000
.equ BIT17,                         0x20000
.equ BIT18,                         0x40000
.equ BIT19,                         0x80000
.equ BIT20,                         0x100000
.equ BIT23,                         0x800000
.equ BIT31,                         0x80000000
# Bit definition in MM1
.equ BadCMOSDetected,               (BIT0 << 17)
.equ BSPApicIDSaveStart,            24
