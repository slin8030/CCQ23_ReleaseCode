/** @file
  Register names for System Agent (SA) registers
  <b>Conventions</b>:
  - Prefixes:
    - Definitions beginning with "R_" are registers
    - Definitions beginning with "B_" are bits within registers
    - Definitions beginning with "V_" are meaningful values of bits within the registers
    - Definitions beginning with "S_" are register sizes
    - Definitions beginning with "N_" are the bit position
  - In general, SA registers are denoted by "_SA_" in register names
  - Registers / bits that are different between SA generations are denoted by
    "_SA_[generation_name]_" in register/bit names. e.g., "_SA_BXT_"
  - Registers / bits that are different between SKUs are denoted by "_[SKU_name]"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SA generation will be just named
    as "_SA_" without [generation_name] inserted.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2017 Intel Corporation.

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

#ifndef _SA_REGS_H_
#define _SA_REGS_H_

#include "PlatformBaseAddresses.h"

/*
< Extended Configuration Base Address.*/

//BXT do not open MCR/MDR interface for side-band message. use MCHBAR instead.
//
//Ranges defined below is following PND2_Cunit_HAS.
//
#define MCHBAR_RANGE_DUNIT0            0x0000
#define MCHBAR_RANGE_DUNIT1            0x0100
#define MCHBAR_RANGE_REUT0             0x0200
#define MCHBAR_RANGE_REUT1             0x0300
#define MCHBAR_RANGE_DDR0_DQ0          0x3000
#define MCHBAR_RANGE_DDR0_DQ1          0x3200
#define MCHBAR_RANGE_DDR1_DQ0          0x3400
#define MCHBAR_RANGE_DDR1_DQ1          0x3600
#define MCHBAR_RANGE_DDR0_CA           0x3800
#define MCHBAR_RANGE_DDR1_CA           0x3A00
#define MCHBAR_RANGE_CUNIT             0x6000
#define MCHBAR_RANGE_AUNIT             0x6400
#define MCHBAR_RANGE_BUNIT             0x6800
#define MCHBAR_RANGE_TUNIT             0x6C00
#define MCHBAR_RANGE_PUNIT             0x7000
#define BUNIT_BMISC_MCHBAR_OFFSET      0x6800
#define BUNIT_BSMRCP_MCHBAR_OFFSET     0x6838
#define BUNIT_BSMRRAC_MCHBAR_OFFSET    0x6840
#define BUNIT_BSMRWAC_MCHBAR_OFFSET    0x6848
#define BSMR_SMM_OPEN_FOR_IA           0x14

#define R_BUNIT_RT_EN                  0x6D78
#define B_BUNIT_RT_EN_RT_ENABLE        BIT0
#define B_BUNIT_RT_EN_RT_IDI_AGENT     (BIT17 | BIT16)
#define N_BUNIT_RT_EN_RT_IDI_AGENT     16

//
// Default Vendor ID and Subsystem ID
//
#define V_INTEL_VENDOR_ID 0x8086      ///< Default Intel Vendor ID
#define V_SA_DEFAULT_SID  0x7270      ///< Default Intel Subsystem ID

//
// DEVICE 0 (Memory Controller Hub)
//
#define SA_MC_BUS                      0x00
#define SA_MC_DEV                      0x00
#define SA_MC_FUN                      0x00
#define SA_MC_FUN_1                    0x01
#define R_SA_MC_VENDOR_ID              0x00
#define R_SA_MC_DEVICE_ID              0x02
#define R_SA_MC_REVISION_ID            0x08
#define V_SA_MC_VID                    0x8086
#define V_SA_MC_DID0                   0x0AF0  //Broxton
#define V_SA_MC_DID1                   0x1AF0  //Broxton1
#define V_SA_MC_DID2                   0x4AF0  //Broxton-X
#define V_SA_MC_DID3                   0x5AF0  //Broxton-P

#define R_SA_MC_CAPID0_A               0xE4
#define R_SA_MC_CAPID0_B               0xE8
#define R_SA_MCHBAR_REG                0x48

//
// Silicon Steppings
//
#define V_SA_MC_RID_0                  0x00
#define V_SA_MC_RID_1                  0x01
#define V_SA_MC_RID_3                  0x03
#define V_SA_MC_RID_4                  0x04
#define V_SA_MC_RID_5                  0x05
#define V_SA_MC_RID_6                  0x06
#define V_SA_MC_RID_7                  0x07
#define V_SA_MC_RID_8                  0x08
#define V_SA_MC_RID_9                  0x09
#define V_SA_MC_RID_A                  0x0A
#define V_SA_MC_RID_B                  0x0B
#define V_SA_MC_RID_C                  0x0C
#define V_SA_MC_RID_D                  0x0D


///
/// Maximum number of SDRAM channels supported by the memory controller
///
#define SA_MC_MAX_CHANNELS 4
///
/// Maximum number of DIMM sockets supported by each channel
///
#define SA_MC_MAX_SLOTS 1

///
/// Maximum number of sides supported per DIMM
///
#define SA_MC_MAX_SIDES 2

///
/// Maximum number of DIMM sockets supported by the memory controller
///
#define SA_MC_MAX_SOCKETS (SA_MC_MAX_CHANNELS * SA_MC_MAX_SLOTS)

///
/// Maximum number of rows supported by the memory controller
///
#define SA_MC_MAX_RANKS (SA_MC_MAX_SOCKETS * SA_MC_MAX_SIDES)

///
/// Maximum number of rows supported by the memory controller
///
#define SA_MC_MAX_ROWS (SA_MC_MAX_SIDES * SA_MC_MAX_SOCKETS)

///
/// Maximum memory supported by the memory controller
/// 4 GB in terms of KB
///
#define SA_MC_MAX_MEM_CAPACITY (4 * 1024 * 1024)

///
/// Define the SPD Address for DIMM 0
///
#define SA_MC_DIMM0_SPD_ADDRESS 0xA0

///
/// Define the maximum number of data bytes on a system with no ECC memory support.
///
#define SA_MC_MAX_BYTES_NO_ECC (8)

///
/// Define the maximum number of SPD data bytes on a DIMM.
///
#define SA_MC_MAX_SPD_SIZE (512)

//
// AUNIT MMIO
//
#define SA_UPARB_GCNT_A2B_0            0x6404
#define SA_UPARB_GCNT_A2B_1            0x6408
#define SA_UPARB_GCNT_A2B_2            0x640C
#define SA_UPARB_GCNT_A2B_3            0x6410
#define SA_UPARB_GCNT_A2B_4            0x6414
#define SA_UPARB_GCNT_A2B_5            0x6418
#define SA_UPARB_GCNT_A2B_6            0x641C
#define SA_UPARB_GCNT_A2B_7            0x6420
#define SA_UPARB_GCNT_A2T_0            0x6424
#define SA_UPARB_GCNT_P2P_0            0x6428
#define SA_UPARB_GCNT_P2P_1            0x642C


/**
 <b>Description</b>:
 - GMCH Graphics Control Register
**/
#define R_SA_GGC (0x50)
/**
 Description of GGCLCK (0:0)
 - When set to 1b, this bit will lock all bits in this register.
**/
#define N_SA_GGC_GGCLCK_OFFSET   (0x0)
#define S_SA_GGC_GGCLCK_WIDTH    (0x1)
#define B_SA_GGC_GGCLCK_MASK     (0x1)
#define V_SA_GGC_GGCLCK_DEFAULT  (0x0)
/**
 Description of IVD (1:1)
 - 0: Enable.  Device 2 (IGD) claims VGA memory and IO cycles, the Sub-Class Code within Device 2 Class Code register is 00.
 - 1: Disable.  Device 2 (IGD) does not claim VGA cycles (Mem and IO), and the Sub- Class Code field within Device 2 function 0 Class Code register is 80.
 - BIOS Requirement:  BIOS must not set this bit to 0 if the GMS field (bits 7:3 of this register) pre-allocates no memory.
 - This bit MUST be set to 1 if Device 2 is disabled disabled DEVEN_0_0_0_PCI.D2F0EN 0.
 **/
#define N_SA_GGC_IVD_OFFSET      (0x1)
#define S_SA_GGC_IVD_WIDTH       (0x1)
#define B_SA_GGC_IVD_MASK        (0x2)
#define V_SA_GGC_IVD_DEFAULT     (0x0)
/**
 Description of VAMEM (2:2) Enables the use of the iGFX enbines for Versatile Acceleration.
 - 1  iGFX engines are in Versatile Acceleration Mode. Device 2 Class Code is 048000h.
 - 0  iGFX engines are in iGFX Mode. Device 2 Class Code is 030000h.
**/
#define N_SA_GGC_VAMEM_OFFSET    (0x2)
#define S_SA_GGC_VAMEM_WIDTH     (0x1)
#define B_SA_GGC_VAMEM_MASK      (0x4)
#define V_SA_GGC_VAMEM_DEFAULT   (0x0)
/**
 Description of RSVD0 (5:3)
**/
#define N_SA_GCC_RSVD0_OFFSET    (0x3)
#define S_SA_GCC_RSVD0_WIDTH     (0x3)
#define B_SA_GCC_RSVD0_MASK      (0x38)
#define V_SA_GCC_RSVD0_DEFAULT   (0x0)
/**
  Description of GGMS (7:6)
 - This field is used to select the amount of Main Memory that is pre-allocated to support the Internal Graphics Translation Table.  The BIOS ensures that memory is pre-allocated only when Internal graphics is enabled.
 - GSM is assumed to be a contiguous physical DRAM space with DSM, and BIOS needs to allocate a contiguous memory chunk.  Hardware will derive the base of GSM from DSM only using the GSM size programmed in the register.
 - Valid options:
 - 0h: 0 MB of memory pre-allocated for GTT.
 - 1h: 2 MB of memory pre-allocated for GTT.
 - 2h: 4 MB of memory pre-allocated for GTT.
 - 3h: 8 MB of memory pre-allocated for GTT.
**/
#define N_SA_GGC_GGMS_OFFSET  (0x6)
#define S_SA_GGC_GGMS_WIDTH   (0x2)
#define B_SA_GGC_GGMS_MASK    (0xc0)
#define V_SA_GGC_GGMS_DEFAULT (0x0)
#define V_SA_GGC_GGMS_DIS     0
#define V_SA_GGC_GGMS_2MB     1
#define V_SA_GGC_GGMS_4MB     2
#define V_SA_GGC_GGMS_8MB     3
/**
 Description of GMS (15:8)
 - This field is used to select the amount of Main Memory that is pre-allocated to support the Internal Graphics device in VGA (non-linear) and Native (linear) modes.  The BIOS ensures that memory is pre-allocated only when Internal graphics is enabled.
 - This register is also LT lockable.
 - Valid options are 0 (0x0) to 2016MB (0x3F) in multiples of 32 MB
 - Default is 32MB
 - All other values are reserved
 - Hardware does not clear or set any of these bits automatically based on IGD being disabled/enabled.
 - BIOS Requirement: BIOS must not set this field to 0h if IVD (bit 1 of this register) is 0.
**/
#define N_SA_GGC_GMS_OFFSET  (0x8)
#define S_SA_GGC_GMS_WIDTH   (0x8)
#define B_SA_GGC_GMS_MASK    (0xff00)
#define V_SA_GGC_GMS_DEFAULT (0x01)
#define V_SA_GGC_GMS_2016MB  0x3F
/**
 Description of RSVD1 (31:16)
**/
#define N_SA_GCC_RSVD1_OFFSET    (0x10)
#define S_SA_GCC_RSVD1_WIDTH     (0xff)
#define B_SA_GCC_RSVD1_MASK      (0xffff0000)
#define V_SA_GCC_RSVD1_DEFAULT   (0x0)


/**
 Description:
 - Allows for enabling/disabling of PCI devices and functions that are within the CPU package. The table below the bit definitions describes the behavior of all combinations of transactions to devices controlled by this register.
  All the bits in this register are LT Lockable.
**/
#define R_SA_DEVEN (0x54)
/**
 Description of D0F0EN (0:0)
 - Bus 0 Device 0 Function 0 may not be disabled and is therefore hardwired to 1.
**/
#define N_SA_DEVEN_D0F0EN_OFFSET   (0x0)
#define S_SA_DEVEN_D0F0EN_WIDTH    (0x1)
#define B_SA_DEVEN_D0F0EN_MASK     (0x1)
#define V_SA_DEVEN_D0F0EN_DEFAULT  (0x1)
/**
 Description of D0F1EN (1:1)
 - 0: Bus 0 Device 1 Function 2 is disabled and hidden.
 - 1: Bus 0 Device 1 Function 2 is enabled and visible.
 - This bit will remain 0 if PEG12 capability is disabled.
**/
#define N_SA_DEVEN_D0F1EN_OFFSET   (0x1)
#define S_SA_DEVEN_D0F1EN_WIDTH    (0x1)
#define B_SA_DEVEN_D0F1EN_MASK     (0x2)
#define V_SA_DEVEN_D0F1EN_DEFAULT  (0x2)
/**
 Description of RSVD0 (2:2)
**/
#define N_SA_DEVEN_RSVD0_OFFSET    (0x2)
#define S_SA_DEVEN_RSVD0_WIDTH     (0x1)
#define B_SA_DEVEN_RSVD0_MASK      (0x4)
#define V_SA_DEVEN_RSVD0_DEFAULT   (0x0)
/**
 Description of RSVD (3:3)
**/
#define N_SA_DEVEN_RSVD_OFFSET     (0x3)
#define S_SA_DEVEN_RSVD_WIDTH      (0x1)
#define B_SA_DEVEN_RSVD_MASK       (0x8)
#define V_SA_DEVEN_RSVD_DEFAULT    (0x0)
/**
 Description of D2F0EN (4:4)
 - 0:  Bus 0 Device 2 is disabled and hidden
 - 1:  Bus 0 Device 2 is enabled and visible
 - This bit will remain 0 if Device 2 capability is disabled.
**/
#define N_SA_DEVEN_D2F0EN_OFFSET   (0x4)
#define S_SA_DEVEN_D2F0EN_WIDTH    (0x1)
#define B_SA_DEVEN_D2F0EN_MASK     (0x10)
#define V_SA_DEVEN_D2F0EN_DEFAULT  (0x10)
/**
 Description of D3F0EN (5:5)
 - 0:  Bus 0 Device 3 is disabled and hidden
 - 1:  Bus 0 Device 3 is enabled and visible
 - This bit will remain 0 if Device 3 capability is disabled.
**/
#define N_SA_DEVEN_D3F0EN_OFFSET   (0x5)
#define S_SA_DEVEN_D3F0EN_WIDTH    (0x1)
#define B_SA_DEVEN_D3F0EN_MASK     (0x20)
#define V_SA_DEVEN_D3F0EN_DEFAULT  (0x20)
/**
 Description of RSVD1 (31:6)
**/
#define N_SA_DEVEN_RSVD1_OFFSET    (0x6)
#define S_SA_DEVEN_RSVD1_WIDTH     (0x1A)
#define B_SA_DEVEN_RSVD1_MASK      (0xFFFFFFC0)
#define V_SA_DEVEN_RSVD1_DEFAULT   (0x0)

#define R_SA_PAVPC (0x58)
/**
 Description of PCME (0:0)
 - This field enables Protected Content Memory within Graphics Stolen Memory.
 - This register is locked (becomes read-only) when PAVPLCK = 1b.
 - This register is read-only (stays at 0b) when PAVP fuse is set to "disabled"
 - 0: Protected Content Memory is disabled
 - 1: Protected Content Memory is enabled
**/
#define N_SA_PAVPC_PCME_OFFSET   (0x0)
#define S_SA_PAVPC_PCME_WIDTH    (0x1)
#define B_SA_PAVPC_PCME_MASK     (0x1)
#define V_SA_PAVPC_PCME_MASK     (0x0)
/**
 Description of PAVPE (1:1)
 - 0: PAVP path is disabled
 - 1: PAVP path is enabled
 - This register is locked (becomes read-only) when PAVPLCK = 1b
 - This register is read-only (stays at 0b) when PAVP capability is set to "disabled" as defined by CAPID0_B[PAVPE].
**/
#define N_SA_PAVPC_PAVPE_OFFSET  (0x1)
#define S_SA_PAVPC_PAVPE_WIDTH   (0x1)
#define B_SA_PAVPC_PAVPE_MASK    (0x2)
#define V_SA_PAVPC_PAVPE_DEFAULT (0x0)
/**
 Description of PAVPLCK (2:2)
 - This bit will lock all writeable contents in this register when set (including itself).
 - This bit will be locked if PAVP is fused off.
**/
#define N_SA_PAVPC_PAVPLCK_OFFSET  (0x2)
#define S_SA_PAVPC_PAVPLCK_WIDTH   (0x1)
#define B_SA_PAVPC_PAVPLCK_MASK    (0x4)
#define V_SA_PAVPC_PAVPLCK_DEFAULT (0x0)
/**
 Description of HVYMODSEL (3:3)
**/
#define N_SA_PAVPC_HVYMODSEL_OFFSET  (0x3)
#define S_SA_PAVPC_HVYMODSEL_WIDTH   (0x1)
#define B_SA_PAVPC_HVYMODSEL_MASK    (0x8)
#define V_SA_PAVPC_HVYMODSEL_DEFAULT (0x0)
/**
 Description of OVTATTACK (4:4)
 - 0b - Disable Override. Attack Terminate allowed
 - 1b - Enable Override. Attack Terminate disallowed.
**/
#define N_SA_PAVPC_OVTATTACK_OFFSET  (0x4)
#define S_SA_PAVPC_OVTATTACK_WIDTH   (0x1)
#define B_SA_PAVPC_OVTATTACK_MASK    (0x10)
#define V_SA_PAVPC_OVTATTACK_DEFAULT (0x0)
/**
 Description of ASMFEN  (6:6)
 - 0b - ASMF method disabled
 - 1b - ASMF method enabled
**/
#define N_SA_PAVPC_ASMFEN_OFFSET  (0x6)
#define S_SA_PAVPC_ASMFEN_WIDTH   (0x1)
#define B_SA_PAVPC_ASMFEN_MASK    (0x10)
#define V_SA_PAVPC_ASMFEN_DEFAULT (0x0)
/**
 Description of PCMBASE (20:31)
 - This field is used to set the base of Protected Content Memory.
 - This corresponds to bits 31:20 of the system memory address range, giving a 1MB granularity. This value MUST be at least 1MB above the base and below the top of stolen memory.
 - This register is locked (becomes read-only) when PAVPE = 1b.
**/
#define N_SA_PAVPC_PCMBASE_OFFSET  (0x14)
#define S_SA_PAVPC_PCMBASE_WIDTH   (0xc)
#define B_SA_PAVPC_PCMBASE_MASK    (0xfff00000)
#define V_SA_PAVPC_PCMBASE_DEFAULT (0x0)


/*
  Description:
  This register contains base of Base of Data Stolen Memory
*/
#define R_SA_BDSM (0xb0)
/*
  Description of LOCK (0:0)
  This bit will lock all writeable settings in this register, including itself.
*/
#define N_SA_BDSM_LOCK_OFFSET   (0x0)
#define S_SA_BDSM_LOCK_WIDTH    (0x1)
#define B_SA_BDSM_LOCK_MASK     (0x1)
#define V_SA_BDSM_LOCK_DEFAULT  (0x0)
/*
  Description of BDSM (20:31)
  This register contains the base address of the Data Stolen Memory. The limit for the Data Stolen Memory is TOLUD-1. This range is not decoded by the system agent,
  but is a sub-region of BGSM decoded by the Integrated Graphics Device. Incoming Request Address[31:20] is compared against BDSM[31:20] and TOLUD[31:20] to determine if the address falls in the range.
*/

#define N_SA_BDSM_BDSM_OFFSET    (0x14)
#define S_SA_BDSM_BDSM_WIDTH     (0xc)
#define B_SA_BDSM_BDSM_MASK      (0xfff00000)
#define V_SA_BDSM_BDSM_DEFAULT   (0x0)


/*
  Description:
  This register contains base of GTT Stolen memory
*/
#define R_SA_BGSM (0xb4)
/*
  Description of LOCK (0:0)
  This bit will lock all writeable settings in this register, including itself.
*/
#define N_SA_BGSM_LOCK_OFFSET   (0x0)
#define S_SA_BGSM_LOCK_WIDTH    (0x1)
#define B_SA_BGSM_LOCK_MASK     (0x1)
#define V_SA_BGSM_LOCK_DEFAULT  (0x0)
/*
  Description of BGSM (20:31)
  This register contains the base address of stolen DRAM memory for the GTT. BIOS determines the base of GTT stolen memory by subtracting the GTT graphics stolen memory size PCI Device 0 offset 52 bits 11:8 from
  the Graphics Base of Data Stolen Memory PCI Device 0 offset B0 bits 31:20.
*/
#define N_SA_BGSM_BGSM_OFFSET    (0x14)
#define S_SA_BGSM_BGSM_WIDTH     (0xc)
#define B_SA_BGSM_BGSM_MASK      (0xfff00000)
#define V_SA_BGSM_BGSM_DEFAULT   (0x0)

/*
  Description:
  This register contains base of TSEG
*/
#define R_SA_TSEG (0xb8)

/*
  Description:
  This register contains the Top of low memory address.
*/
#define R_SA_TOLUD (0xbc)

#define TOUUD_HI_0_0_0_PCI_CUNIT_REG     0x000000AC
#define TOUUD_LO_0_0_0_PCI_CUNIT_REG     0x000000A8

/*
  Description of LOCK (0:0)
  This bit will lock all writeable settings in this register, including itself.
*/
#define N_SA_TOLUD_LOCK_OFFSET   (0x0)
#define S_SA_TOLUD_LOCK_WIDTH    (0x1)
#define B_SA_TOLUD_LOCK_MASK     (0x1)
#define V_SA_TOLUD_LOCK_DEFAULT  (0x0)
/*
  Description of TOLUD (20:31)
  This register contains bits 31 to 20 of an address one byte above the maximum DRAM memory below 4G that is usable by the operating system. Address bits 31 down to 20 programmed to 01h implies a minimum memory size of 1MB. Configuration software must set this value to the smaller of the following 2 choices: maximum amount memory in the system minus ME stolen memory plus one byte or the minimum address allocated for PCI memory. Address bits 19:0 are assumed to be 0_0000h for the purposes of address comparison. The Host interface positively decodes an address towards DRAM if the incoming address is less than the value programmed in this register.
  The Top of Low Usable DRAM is the lowest address above both Graphics Stolen memory and Tseg. BIOS determines the base of Graphics Stolen Memory by subtracting the Graphics Stolen Memory Size from TOLUD and further decrements by Tseg size to determine base of Tseg. All the Bits in this register are locked in LT mode.
  This register must be 1MB aligned when reclaim is enabled.
*/
#define N_SA_TOLUD_TOLUD_OFFSET    (0x14)
#define S_SA_TOLUD_TOLUD_WIDTH     (0xc)
#define B_SA_TOLUD_TOLUD_MASK      (0xfff00000)
#define V_SA_TOLUD_TOLUD_DEFAULT   (0x100000)


//
// Temporary Device & Function Number used for Hybrid Graphics DGPU
//
#define SA_TEMP_DGPU_DEV      0x00
#define SA_TEMP_DGPU_FUN      0x00


//
// Device 3 Equates
//
#define SA_IPU_BUS_NUM    0x00
#define SA_IPU_DEV_NUM    0x03
#define SA_IPU_FUN_NUM    0x00

/* SideBand Units Port ID*/

#define AUNIT_PORT_ID          0x00     /*SSA-A Unit (IO Arbiter)*/
#define DUNIT_PORT_ID          0x01   /*D Unit*/
#define TUNIT_PORT_ID          0x02   /*SSA-T Unit*/
#define BUNIT_PORT_ID          0x03    /*SSA-B Unit*/
#define PUNIT_PORT_ID          0x04     /*P Unit*/
#define DFXUNIT_PORT_ID    0x05     /*Test Controller SSA-DFX Unit*/
#define GUNIT_PORT_ID          0x06   /*G Unit*/
#define CUNIT_PORT_ID          0x08     /*C Unit*/
#define sVID_PORT_ID           0x0A     /*VID Controller*/
#define DDRIO_PORT_ID          0x0C   /*DDR IO Unit*/
#define REUT_PORT_ID           0x0D   /*Memory REUT*/
#define GENX_PORT_ID           0x0E   /*Graphics Adapter-Genx*/
#define DRNG_PORT_ID           0x0F   /*Random Number Generator*/
#define  DISPCONT_PORT_ID    0x10   /*Display Controller*/
#define  FUSEEPNC_PORT_ID    0x11   /*Fuse EndPoint Nort*/
#define DISPIO_PORT_ID       0x12       /*DISPPHY*/
#define GPIONC_PORT_ID       0x13       /*GPIO (North) Controller*/
#define CCK_PORT_ID            0x14     /*Clock Controller*/


// SideBand Register Definitions
//Common for A/B/C/D/T/SVID/CCK/I units
#define SBR_READ_CMD    0x10000000
#define SBR_WRITE_CMD   0x11000000

//Common for Gunit/DISPIO/DFXLAKSEMORE/DISPCONT units
#define SBR_GDISPIOREAD_CMD     0x00000000
#define SBR_GDISPIOWRITE_CMD    0x01000000

//Common for Smbus units
#define SMB_REGREAD_CMD     0x04000000
#define SMB_REGWRITE_CMD    0x05000000

//Common for Punit/DFX/GPIONC/DFXSOC/DFXNC/DFXVISA units
#define MBR_PDFXGPIODDRIOREAD_CMD   0x06000000
#define MBR_PDFXGPIODDRIOWRITE_CMD  0x07000000

//Msg Bus Registers
#define MC_MCR          0x000000D0      //Cunit Message Control Register
#define MC_MDR          0x000000D4      //Cunit Message Data Register
#define MC_MCRX         0x000000D8      //Cunit Message Control Register Extension


#define MC_DEVEN_OFFSET     0x54        //Device Enable
#define B_DEVEN_D2F0EN      BIT3        // Internal Graphics Engine F0 Enable


//smBiosMemory.c use this
//PunitDriver.c
#define MC_TSEGMB_OFFSET    0xAC        //TSEG Memory Base
//
//BXT Units Registers Definition
//Register Symbol       Register Start      //Register Name
//
// SSA-AUnit (IO Aribter)  Register Offset
// Updated to Cpsec-12ww49.4
#define AUNIT_ACRCP         0x00
#define AUNIT_ACRRAC        0x01
#define AUNIT_ACRWAC        0x02
#define AUNIT_ADCRCP        0x03
#define AUNIT_ADCRRAC       0x04
#define AUNIT_ADCRWAC       0x05
#define AUNIT_ATCCRCP       0x06
#define AUNIT_ATCCRRAC      0x07
#define AUNIT_ATCCRWAC      0x08
#define AUNIT_ASECCRCP      0x09
#define AUNIT_ASECCRRAC     0x0A
#define AUNIT_ASECCRWAC     0x0B
#define AUNIT_ACF8          0x10
#define AUNIT_ADMIOCMP      0x11
#define AUNIT_ACKGATE       0x18
#define AUNIT_AISOCHCTL     0x20
#define AUNIT_AVCCTL        0x21
#define AUNIT_AISOCHCTL2    0x22
#define AUNIT_APEERBASE     0x30
#define AUNIT_APEERLIMIT    0x31
#define AUNIT_AVIB          0x32
#define AUNIT_AVQR          0x33
#define AUNIT_ATBR          0x40
#define AUNIT_AB0BR         0x48
#define AUNIT_AB1BR         0x49
#define AUNIT_AB2BR         0x4A
#define AUNIT_ACFCACV       0x60
#define AUNIT_ASBACV0       0x61
#define AUNIT_ASBACV1       0x62
#define AUNIT_ACF8SAI       0x63
#define AUNIT_AMIRRORCTL    0x8A
#define AUNIT_AIODCTL0      0x90
#define AUNIT_AIODCTL1      0x91
#define AUNIT_AIODCTL2      0x92
#define AUNIT_AIODUSMCH0    0x93
#define AUNIT_AIODUSMCH1    0x94
#define AUNIT_AIODUSMCH2    0x95
#define AUNIT_AIODUSMCH3    0x96
#define AUNIT_AIODUSMCH4    0x97
#define AUNIT_AIODUSMCH5    0x98
#define AUNIT_AIODUSMSK0    0x99
#define AUNIT_AIODUSMSK1    0x9A
#define AUNIT_AIODUSMSK2    0x9B
#define AUNIT_AIODUSMSK3    0x9C
#define AUNIT_AIODUSMSK4    0x9D
#define AUNIT_AIODUSMSK5    0x9E
#define AUNIT_AIODDSMCH0    0x9F
#define AUNIT_AIODDSMCH1    0xA0
#define AUNIT_AIODDSMCH2    0xA1
#define AUNIT_AIODDSMCH3    0xA2
#define AUNIT_AIODDSMCH4    0xA3
#define AUNIT_AIODDSMCH5    0xA4
#define AUNIT_AIODDSMSK0    0xA5
#define AUNIT_AIODDSMSK1    0xA6
#define AUNIT_AIODDSMSK2    0xA7
#define AUNIT_AIODDSMSK3    0xA8
#define AUNIT_AIODDSMSK4    0xA9
#define AUNIT_AIODDSMSK5    0xAA
#define AUNIT_AARBCTL0      0xC0
#define AUNIT_AARBCTL1      0xC1
#define AUNIT_AARBCTL2      0xC2
#define AUNIT_AARBCTL3      0xC3
#define AUNIT_AARBCTL4      0xC4
#define AUNIT_ADNARBCTL     0xD0
#define AUNIT_AMISR0        0xF0
#define AUNIT_AMISR1        0xF1


//
// SSA-BUnit (System Memory Arbiter)  Register Offset
// Updated to Cpsec-12ww49.4
#define BUNIT_BSECCP            0x00
#define BUNIT_BSECRAC           0x01
#define BUNIT_BSECWAC           0x02
#define BUNIT_BARBCTRL0         0x03
#define BUNIT_BARBCTRL1         0x04
#define BUNIT_BARBCTRL2         0x05
#define BUNIT_BARBCTRL3         0x06
#define BUNIT_BWFLUSH           0x07
#define BUNIT_BBANKMASK         0x08
#define BUNIT_BROWMASK          0x09
#define BUNIT_BRANKMASK         0x0A
#define BUNIT_BALIMIT0          0x0B
#define BUNIT_BALIMIT1          0x0C
#define BUNIT_BALIMIT2          0x0D
#define BUNIT_BALIMIT3          0x0E
#define BUNIT_BARES0            0x0F
#define BUNIT_BARES1            0x10
#define BUNIT_BISOC             0x11
#define BUNIT_BCOSCAT           0x12
#define BUNIT_BDPT              0x13
#define BUNIT_BFLWT             0x14
#define BUNIT_BBWC              0x15
#define BUNIT_BISOCWT           0x16
#define BUNIT_BSCHCTRL0         0x18
#define BUNIT_BIMRDATA          0x1a
#define BUNIT_BPMRVCTL          0x1b
#define BUNIT_B_SECURITY_STAT0  0x1c
#define BUNIT_B_SECURITY_STAT1  0x1d
#define BUNIT_B_SECURITY_STAT2  0x1e
#define BUNIT_BMRCP             0x20
#define BUNIT_BMRRAC            0x21
#define BUNIT_BMRWAC            0x22
#define BUNIT_BNOCACHE          0x23
#define BUNIT_BNOCACHECTL       0x24
#define BUNIT_BMBOUND           0x25
#define BUNIT_BMBOUND_HI        0x26
#define BUNIT_BECREG            0x27
#define BUNIT_BMISC             0x28
#define B_BMISC_RESDRAM         0x01    //Bit 0 - When this bit is set, reads targeting E-segment are routed to DRAM.
#define B_BMISC_RFSDRAM         0x02    //Bit 1 - When this bit is set, reads targeting F-segment are routed to DRAM.
//[-start-160712-IB07400756-add]//
#define B_BMISC_ABSEGINDRAM     0x04    //Bit 2 - When this bit is set, reads/write targeting AB-segment are routed to DRAM.
//[-end-160712-IB07400756-add]//
#define BUNIT_BSMRCP            0x2B
#define BSMRCP_SMM_CTRL_REG_LOCK 0x00   //Dont allow any access to the register until the system is reset
#define BUNIT_BSMRRAC           0x2C
#define BSMRRAC_SMM_WRITE_OPEN_FOR_ALL_CORE 0xFF    //Allow access only to all CPU HOST
#define BSMRRAC_SMM_WRITE_CLOSED_FOR_IA_SMM 0x04    //Allow access only to CPU HOST IA SMM
#define BUNIT_BSMRWAC   0x2D
#define BSMRWAC_SMM_WRITE_OPEN_FOR_ALL_CORE 0xFF    //Allow access only to all CPU HOST
#define BSMRWAC_SMM_WRITE_CLOSED_FOR_IA_SMM 0x04    //Allow access only to CPU HOST IA SMM
#define BUNIT_BSMMRRL   0x2E
#define BUNIT_BSMMRRH   0x2F
#define BUNIT_BC0AHASHCFG   0x30
#define BUNIT_BDBCP     0x38
#define BUNIT_BDRRAC    0x39
#define BUNIT_BDRWAC    0x3A
#define BUNIT_BDEBUG0   0x3B
#define BUNIT_BDEBUG1   0x3C
#define BUNIT_BCTRL     0x3D
#define BUNIT_BTHCTRL   0x3E
#define BUNIT_BTHMASK   0x3F
#define BUNIT_BIACP     0x40
#define BUNIT_BIARAC    0x41
#define BUNIT_BIAWAC    0x42
#define BUNIT_BEXMCP    0x43
#define BUNIT_BEXMRAC   0x44
#define BUNIT_BEXMWAC   0x45
#define BUNIT_EXML      0x46
#define BUNIT_EXMH      0x47
#define BUNIT_LP0Mode   0x48
#define BUNIT_LP1Mode   0x49
#define BUNIT_LP2Mode   0x4A
#define BUNIT_LP3Mode   0x4B
#define BUNIT_MCi_CTL_LOW   0x54
#define BUNIT_MCi_CTL_HIGH  0x55
#define BUNIT_MCi_STATUS_LOW    0x56
#define BUNIT_MCi_STATUS_HIGH   0x57
#define BUNIT_MCi_ADDR_LOW  0x58
#define BUNIT_MCi_ADDR_HIGH 0x59
#define BUNIT_BCERRTHRESH_LOW   0x5A
#define BUNIT_BCERRTHRESH_HIGH  0x5B
#define BUNIT_BMCMODE_LOW   0x5C
#define BUNIT_BMCMODE_HIGH  0x5D
#define BUNIT_BIMR0CP       0x60
#define BUNIT_BIMR1CP       0x61
#define BUNIT_BIMR2CP       0x62
#define BUNIT_BIMR3CP       0x63
#define BUNIT_BIMR4CP       0x64
#define BUNIT_BIMR5CP       0x65
#define BUNIT_BIMR6CP       0x66
#define BUNIT_BIMR7CP       0x67
#define BUNIT_BIMR0L        0x80
#define BUNIT_BIMR0H        0x81
#define BUNIT_BIMR0RAC      0x82
#define BUNIT_BIMR0WAC      0x83
#define BUNIT_BIMR1L        0x84
#define BUNIT_BIMR1H        0x85
#define BUNIT_BIMR1RAC      0x86
#define BUNIT_BIMR1WAC      0x87
#define BUNIT_BIMR2L        0x88
#define BUNIT_BIMR2H        0x89
#define BUNIT_BIMR2RAC      0x8a
#define BUNIT_BIMR2WAC      0x8b
#define BUNIT_BIMR3L        0x8c
#define BUNIT_BIMR3H        0x8d
#define BUNIT_BIMR3RAC      0x8e
#define BUNIT_BIMR3WAC      0x8f
#define BUNIT_BIMR4L        0x90
#define BUNIT_BIMR4H        0x91
#define BUNIT_BIMR4RAC      0x92
#define BUNIT_BIMR4WAC      0x93
#define BUNIT_BIMR5L        0x94
#define BUNIT_BIMR5H        0x95
#define BUNIT_BIMR5RAC      0x96
#define BUNIT_BIMR5WAC      0x97
#define BUNIT_BIMR6L        0x98
#define BUNIT_BIMR6H        0x99
#define BUNIT_BIMR6RAC      0x9a
#define BUNIT_BIMR6WAC      0x9b
#define BUNIT_BIMR7L        0x9c
#define BUNIT_BIMR7H        0x9d
#define BUNIT_BIMR7RAC      0x9e
#define BUNIT_BIMR7WAC      0x9f
#define BUNIT_PTIBASE       0x0100
#define BUNIT_PTIRSIZE      0x0101
#define BUNIT_PTIWWMODCFG   0x0102
#define BUNIT_PTIUCOUNTER   0x0103
#define BUNIT_PTITSELOP     0x0104
#define BUNIT_PTITSELGRP    0x0105
#define BUNIT_PTI0CTL       0x0110
#define BUNIT_PTI0SAIMATCH  0x0111
#define BUNIT_PTI0IDIREQ    0x0112
#define BUNIT_PTI0ADDRHI1   0x0113
#define BUNIT_PTI0ADDRHI0   0x0114
#define BUNIT_PTI0ADDRLO1   0x0115
#define BUNIT_PTI0ADDRLO0   0x0116
#define BUNIT_PTI0DATA      0x0117
#define BUNIT_PTI0DMASK     0x0118
#define BUNIT_PTI1CTL       0x0120
#define BUNIT_PTI1SAIMATCH  0x0121
#define BUNIT_PTI1IDIREQ    0x0122
#define BUNIT_PTI1ADDRHI1   0x0123
#define BUNIT_PTI1ADDRHI0   0x0124
#define BUNIT_PTI1ADDRLO1   0x0125
#define BUNIT_PTI1ADDRLO0   0x0126
#define BUNIT_PTI1DATA      0x0127
#define BUNIT_PTI1DMASK     0x0128
#define BUNIT_PTI2CTL       0x0130
#define BUNIT_PTI2SAIMATCH  0x0131
#define BUNIT_PTI2IDIREQ    0x0132
#define BUNIT_PTI2ADDRHI1   0x0133
#define BUNIT_PTI2ADDRHI0   0x0134
#define BUNIT_PTI2ADDRLO1   0x0135
#define BUNIT_PTI2ADDRLO0   0x0136
#define BUNIT_PTI2DATA      0x0137
#define BUNIT_PTI2DMASK     0x0138
#define BUNIT_PTI3CTL       0x0140
#define BUNIT_PTI3SAIMATCH  0x0141
#define BUNIT_PTI3IDIREQ    0x0142
#define BUNIT_PTI3ADDRHI1   0x0143
#define BUNIT_PTI3ADDRHI0   0x0144
#define BUNIT_PTI3ADDRLO1   0x0145
#define BUNIT_PTI3ADDRLO0   0x0146
#define BUNIT_PTI3DATA      0x0147
#define BUNIT_PTI3DMASK     0x0148
#define BUNIT_MISRCTL       0x0150
#define BUNIT_MISRS2CREQSIG 0x0151
#define BUNIT_MISRC2SREQSIG 0x0152
#define BUNIT_MISRS2CDATASIG    0x0153
#define BUNIT_MISRC2SDATASIG    0x0154
#define BUNIT_MISRRPLSIG    0x0155
#define BUNIT_BDBGCTL   0x0160
#define BUNIT_BDBGADD   0x0161
#define BUNIT_BDBGDAT   0x0162
//
// SSA-CUnit (Message Bus Controller)  Register Offset
// Updated to Cpsec-12ww49.4
#define CUNIT_REG_DEVICEID     0x00
#define CUNIT_CFG_REG_PCISTATUS 0x01
#define CUNIT_CFG_REG_CLASSCODE 0x02
#define CUNIT_CFG_REG_HDR_TYPE  0x03
#define CUNIT_CFG_REG_STRAP_SSID    0x0B
#define CUNIT_SB_MSG_REG        0x34
#define CUNIT_SB_DATA_REG       0x35
#define CUNIT_SB_PCKET_ADDR_EXT 0x36
#define CUNIT_SB_PACKET_REG_RW  0x37
#define CUNIT_SB_PCKET_ADDR_EXT_FUNNYIO 0x38
#define CUNIT_SB_PCKET_REG_RW_FUNNYIO   0x39
#define CUNIT_SB_PACKET_FUNNYIO_ADDR_EXT1 0x3A
#define CUNIT_SB_PACKET_FUNNYIO_REG1      0x3B
#define CUNIT_SCRATCHPAD_REG    0x3C
#define CUNIT_MANUFACTURING_ID  0x3E
#define CUNIT_LOCAL_CONTROL_MODE    0x40
#define CUNIT_ACCESS_CTRL_VIOL  0x41
#define CUNIT_PDM_REGISTER      0x42
#define CUNIT_SSA_REGIONAL_TRUNKGATE_CTL 0x43
#define CUNIT_MCRS_SAI  0x45
#define CUNIT_MDR_SAI  0x46
#define CUNIT_MCHBAR_REG 0x48
#define CUNIT_SB_PACKET_FUNNYIO_ADDR_EXT2 0x58
#define CUNIT_SB_PACKET_FUNNYIO_REG2      0x59
#define CUNIT_SB_PACKET_FUNNYIO_ADDR_EXT3 0x5A
#define CUNIT_SB_PACKET_FUNNYIO_REG3      0x5B

//
// SSA-CUnit (Message Bus Controller)  Register Offset
// Accessing by PCI Config B0D0F0
// Updated to Cpsec-12ww49.4
#define CUNIT_PCICFG_REG_DEVICEID                   0x00
#define CUNIT_PCICFG_CFG_REG_PCISTATUS              0x04
#define CUNIT_PCICFG_CFG_REG_CLASSCODE              0x08
#define CUNIT_PCICFG_CFG_REG_HDR_TYPE               0x0C
#define CUNIT_PCICFG_CFG_REG_STRAP_SSID             0x2C
// BXT do not have MCR/MDR.
#define CUNIT_PCICFG_PCIEXBAR_REG                   0x60
#define CUNIT_PCICFG_MSG_CTRL_REG_EXT               0xD8
#define CUNIT_PCICFG_MSG_CTRL_PACKET_REG            0xDC
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT0    0xE0
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG0         0xE4
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT1    0xE8
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG1         0xEC
#define CUNIT_PCICFG_SCRATCHPAD_REG                 0xF0
#define CUNIT_PCICFG_MANUFACTURING_ID               0xF8
#define CUNIT_PCICFG_LOCAL_CONTROL_MODE             0x100
#define CUNIT_PCICFG_ACCESS_CTRL_VIOL               0x104
#define CUNIT_PCICFG_PDM_REGISTER                   0x108
#define CUNIT_PCICFG_SSA_REGIONAL_TRUNKGATE_CTL     0x10C
#define CUNIT_PCICFG_MCRS_SAI                       0x114
#define CUNIT_PCICFG_MDR_SAI                        0x118
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT2    0x160
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG2         0x164
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT3    0x168
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG3         0x16C


//
// SSA-TUnit (CPU Bus Interface Controller)  Register Offset
// Updated to Cpsec-12ww49.4
#define TUNIT_INTR_REDIR_CTL    0x00
#define TUNIT_X2B_ARB_CTL0  0x01
#define TUNIT_APIC_CTL  0x02
#define TUNIT_CTL   0x03
#define TUNIT_MISC_CTL  0x04
#define TUNIT_CLKGATE_CTL   0x05
#define TUNIT_X2BARB_CTL1   0x06
#define TUNIT_PSMI_CTL  0x07
#define TUNIT_MONADDR_LPID0 0x10
#define TUNIT_MONADDR_LPID1 0x11
#define TUNIT_MONADDR_LPID2 0x12
#define TUNIT_MONADDR_LPID3 0x13
#define TUNIT_IDI0_SNPCNTR  0x24
#define TUNIT_IDI1_SNPCNTR  0x25
#define TUNIT_SEMAPHORE 0x30
#define TUNIT_SCRPAD0   0x31
#define TUNIT_SCRPAD1   0x32
#define TUNIT_SCRPAD2   0x33
#define TUNIT_SCRPAD3   0x34
#define TUNIT_SECURITY_CTL_DBG  0x40
#define TUNIT_SECURITY_RD_CTL_DBG   0x41
#define TUNIT_SECURITY_WR_CTL_DBG   0x42
#define TUNIT_SECURITY_CTL_UCD  0x44
#define TUNIT_SECURITY_RD_CTL_UCD   0x45
#define TUNIT_SECURITY_WR_CTL_UCD   0x46
#define TUNIT_SECURITY_CTL_PWR  0x48
#define TUNIT_SECURITY_RD_CTL_PWR   0x49
#define TUNIT_SECURITY_WR_CTL_PWR   0x4A
#define TUNIT_SECURITY_CTL_SECCTL   0x4C
#define TUNIT_SECURITY_RD_CTL_SECCTL    0x4D
#define TUNIT_SECURITY_WR_CTL_SECCTL    0x4E
#define TUNIT_SECURITY_STAT0    0x50
#define TUNIT_SECURITY_STAT1    0x51
#define TUNIT_APICINFO_LPID0    0x60
#define TUNIT_APICINFO_LPID1    0x61
#define TUNIT_APICINFO_LPID2    0x62
#define TUNIT_APICINFO_LPID3    0x63
#define TUNIT_MCERR_STATUS 0x70
#define TUNIT_IERR_STATUS 0x71
#define TUNIT_MCERR_STATUS_UPDATE 0x72
#define TUNIT_IERR_STATUS_UPDATE 0x73
#define TUNIT_ERR_LOGSTATUS 0x74

//
// SSA-DUnit (System Memory Controller)  Register Offset
// Port ID: 1h
#define DUNIT_DRP       0x00
#define DUNIT_DTR0      0x01
#define DUNIT_DTR1      0x02
#define DUNIT_DTR2      0x03
#define DUNIT_DTR3      0x04
#define DUNIT_DTR4      0x05
#define DUNIT_DPMC0     0x06
#define DUNIT_DPMC1     0x07
#define DUNIT_DRFC      0x08
#define DUNIT_DSCH      0x09
#define DUNIT_DCAL      0x0A
#define DUNIT_DRMC      0x0B
#define DUNIT_PMSTS     0x0C
#define DUNIT_DCO       0x0F
#define DUNIT_DTRC      0x10
#define DUNIT_DCBR      0x12
#define DUNIT_DSTAT     0x20
#define DUNIT_PGTBL     0x21
#define DUNIT_MISRCCCLR     0x31
#define DUNIT_MISRDDCLR     0x32
#define DUNIT_MISRCCSIG     0x34
#define DUNIT_MISRDDSIG     0x35
#define DUNIT_MISRECCSIG    0x37
#define DUNIT_SSKPD0        0x4A
#define DUNIT_SSKPD1    0x4B
#define DUNIT_BONUS0        0x50
#define DUNIT_BONUS1        0x51
#define DUNIT_DECCCTRL      0x60
#define DUNIT_DECCSTAT      0x61
#define DUNIT_DECCSBECNT    0x62
#define DUNIT_DFUSESTAT     0x70
#define DUNIT_DSCRMSEED     0x80
#define DUNIT_DSCRMLO       0x81
#define DUNIT_DSCRMHI       0x82
#define DUNIT_PMSEL0        0xE0
#define DUNIT_PMSEL1        0xE1
#define DUNIT_PMSEL2        0xE2
#define DUNIT_PMSEL3        0xE3
#define DUNIT_PMAUXMAX      0xE8
#define DUNIT_PMAUXMIN      0xE9
#define DUNIT_PMAUX     0xEA

//
// P-UNIT (Power Management Control) Registers Offset
// Port ID:4
// Updated based on Cspec ww30.1
#define PUNIT_CONTROL   0x00
#define PUNIT_THERMAL_SOC_TRIGGER   0x01
#define PUNIT_SOC_POWER_BUDGET  0x02
#define PUNIT_SOC_ENERGY_CREDIT 0x03
#define PUNIT_TURBO_SOC_OVERRIDE    0x04
#define PUNIT_BIOS_RESET_CPL    0x05
#define PUNIT_BIOS_CONFIG   0x06
#define PUNIT_PKG_TURBO_POWER_LIMIT_L32 0x07
#define PUNIT_PKG_TURBO_POWER_LIMIT_H32 0x08
#define PUNIT_PP1_TURBO_POWER_LIMIT 0x09
#define PUNIT_SPARE_TURBO_REG0  0x0A
#define PUNIT_SPARE_TURBO_REG1  0x0B
#define PUNIT_FW_VISA_REG0  0x0C
#define PUNIT_FW_VISA_REG1  0x0D
#define PUNIT_WARM_RST_INDICATOR    0x0E
#define PUNIT_SAI_VIOLATION 0x0F
#define PUNIT_DFX_DEBUG_STRAPS  0x10
#define PUNIT_SOC_DEBUG_PGE 0x11
#define PUNIT_SOC_DEBUG_FWE 0x12
#define PUNIT_CPU_DEBUG_PGE 0x13
#define PUNIT_CPU_DEBUG_FWE 0x14
#define PUNIT_CSTATEINFO    0x15
#define PUNIT_FWCNTRLINFO   0x16
#define PUNIT_SAIVIOLATION_STS  0x17
#define PUNIT_AG1CP 0x18
#define PUNIT_AG2CP 0x19
#define PUNIT_AG1WAC    0x1A
#define PUNIT_AG2WAC    0x1B
#define PUNIT_CLKGATEOVER   0x1C
#define PUNIT_BIOSCFG_IPDEVTYPE 0x1D
#define PUNIT_S0i2_PREWAKE  0x1E
#define PUNIT_S0i3_PREWAKE  0x1F
#define PUNIT_PKG_POWER_INFO_L32    0x20
#define PUNIT_PKG_POWER_INFO_H32    0x21
#define PUNIT_PKG_PERF_STATUS   0x22
#define PUNIT_DRAM_POWER_INFO_L32   0x23
#define PUNIT_DRAM_POWER_INFO_H32   0x24
#define PUNIT_DRAM_PERF_STATUS  0x25
#define PUNIT_DRAM_ENERGY_STATUS    0x26
#define PUNIT_DRAM_POWER_LIMIT  0x27
#define PUNIT_PP2_POWER_LIMIT   0x28
#define PUNIT_PP2_ENERGY_STATUS 0x29
#define PUNIT_L2SIZE_CONTROL0   0x2A
#define PUNIT_L2SIZE_CONTROL1   0x2B
#define PUNIT_L2SIZE_CONTROL2   0x2C
#define PUNIT_L2SIZE_CONTROL3   0x2D
#define PUNIT_SPARE_PM_CONTROL0 0x2E
#define PUNIT_SPARE_PM_CONTROL1 0x2F
#define PUNIT_VEDSSPM0  0x32
#define PUNIT_VEDSSPM1  0x33
#define PUNIT_DSPSSPM   0x36
#define PUNIT_IPUSSPM0  0x39
#define B_IPUSSPM0_FUSDIS   BIT26       //Bit26 - When this bit is set, IPU Disable by fuse
#define PUNIT_IPUSSPM1  0x3A
#define PUNIT_MIOSSPM   0x3B
#define PUNIT_GUNIT_SS_PM   0x3E
#define PUNIT_RTC_GTSC_INCRAMT1 0x40
#define PUNIT_RTC_GTSC_INCRAMT2 0x41
#define PUNIT_RTC_GTSC_INCRAMT3 0x42
#define PUNIT_RTC_GTSC_HIGH 0x43
#define PUNIT_RTC_GTSC_LOW  0x44
#define PUNIT_RTC_GTSC_UPDTCYC  0x45
#define PUNIT_CPUTAPMISC_M0_FORCEONE_OVR    0x48
#define PUNIT_CPUTAPMISC_M1_FORCEONE_OVR    0x49
#define PUNIT_CPUTAPMISC_M0_FORCEZERO_OVR   0x4C
#define PUNIT_CPUTAPMISC_M1_FORCEZERO_OVR   0x4D
#define PUNIT_CPUTAPPG_M0_FORCEONE_OVR  0x50
#define PUNIT_CPUTAPPG_M1_FORCEONE_OVR  0x51
#define PUNIT_CPUTAPPG_M0_FORCEZERO_OVR 0x54
#define PUNIT_CPUTAPPG_M1_FORCEZERO_OVR 0x55
#define PUNIT_CPUTAPFW_M0_FORCEONE_OVR  0x58
#define PUNIT_CPUTAPFW_M1_FORCEONE_OVR  0x59
#define PUNIT_CPUTAPFW_M0_FORCEZERO_OVR 0x5C
#define PUNIT_CPUTAPFW_M1_FORCEZERO_OVR 0x5D
#define PUNIT_PWRGT_CNT_CTRL    0x60
#define PUNIT_PWRGT_STATUS  0x61
#define PUNIT_PWRGT_INTREN  0x62
#define PUNIT_TAP_PG_FORCEONE_OVR   0x63
#define PUNIT_TAP_PG_FORCEZERO_OVR  0x64
#define PUNIT_TAP_CLKEN_FORCEONE_OVR    0x65
#define PUNIT_TAP_CLKEN_FORCEZERO_OVR   0x66
#define PUNIT_TAP_FW_FORCEONE_OVR   0x67
#define PUNIT_TAP_FW_FORCEZERO_OVR  0x68
#define PUNIT_TAP_RST_FORCEONE_OVR  0x69
#define PUNIT_TAP_RST_FORCEZERO_OVR 0x6A
#define PUNIT_PWRGT_EN_OUT  0x6B
#define PUNIT_PWRGT_RF_EN_OUT   0x6C
#define PUNIT_PUNIT_INTR_PAYLOAD    0x6D
#define PUNIT_TAPMISC_FORCEONE_OVR  0x6F
#define PUNIT_TAPMISC_FORCEZERO_OVR 0x70
#define PUNIT_PCR_EXTERNAL  0x71
#define PUNIT_TAPMISC2_FORCEONE_OVR 0x72
#define PUNIT_TAPMISC2_FORCEZERO_OVR    0x73
#define PUNIT_OPTION_REG10  0x74
#define PUNIT_PUNIT_CPU_RST 0x7B
#define PUNIT_CPU_SOFT_STRAPS   0x7C
#define PUNIT_MAILBOX_DATA 0x80
#define PUNIT_PTMC  0x80
#define PUNIT_TTR0  0x81
#define PUNIT_TTR1  0x82
#define PUNIT_TTS   0x83
#define PUNIT_MAILBOX_INTERFACE 0x84
#define PUNIT_TELB  0x84
#define PUNIT_TELT  0x85
#define PUNIT_TQPR  0x86
#define PUNIT_GFXT  0x88
#define PUNIT_VEDT  0x89
#define PUNIT_IPUT  0x8C
#define PUNIT_FW_SHAID0 0x90
#define PUNIT_FW_SHAID1 0x91
#define PUNIT_FW_SHAID2 0x92
#define PUNIT_FW_SHAID3 0x93
#define PUNIT_FW_SHAID4 0x94
#define PUNIT_IUNITSETID 0x98
#define PUNIT_FW_OR0    0xA0
#define PUNIT_FW_OR1    0xA1
#define PUNIT_FW_OR2    0xA2
#define PUNIT_FW_OR3    0xA3
#define PUNIT_CORE0_AONTL   0xA4
#define PUNIT_CORE0_AONTH   0xA5
#define PUNIT_CORE1_AONTL   0xA6
#define PUNIT_CORE1_AONTH   0xA7
#define PUNIT_FW_OR8    0xA8
#define PUNIT_FW_OR9   0xA9
#define PUNIT_FW_ORA    0xAA
#define PUNIT_FW_ORB    0xAB
#define PUNIT_FW_ORC    0xAC
#define PUNIT_FW_ORD    0xAD
#define PUNIT_FW_ORE    0xAE
#define PUNIT_FW_ORF    0xAF
#define PUNIT_DTSC  0xB0
#define PUNIT_TRR   0xB1
#define PUNIT_PTPS  0xB2
#define PUNIT_PTTS  0xB3
#define PUNIT_PTTSS 0xB4
#define PUNIT_TE_AUX0   0xB5
#define PUNIT_TE_AUX1   0xB6
#define PUNIT_TE_AUX2   0xB7
#define PUNIT_TE_AUX3   0xB8
#define PUNIT_TTE_VRIccMax  0xB9
#define PUNIT_TTE_VRHot 0xBA
#define PUNIT_TTE_XXPROCHOT 0xBB
#define PUNIT_TTE_SLM0  0xBC
#define PUNIT_TTE_SLM1  0xBD
#define PUNIT_BWTE  0xBE
#define PUNIT_TTE_SWT   0xBF
#define PUNIT_PMU_DDR_0 0xC0
#define PUNIT_PMU_DDR_1 0xC1
#define PUNIT_PMU_DDR_2 0xC2
#define PUNIT_PMU_DDR_3 0xC3
#define PUNIT_PMU_DDR_4 0xC4
#define PUNIT_PMU_DDR_5 0xC5
#define PUNIT_PMU_DDR_6 0xC6
#define PUNIT_PMU_DDR_7 0xC7
#define PUNIT_PMU_DDR_ADDR  0xC8
#define PUNIT_GENLC_CZCOUNTER_L32   0xCE
#define PUNIT_GENLC_CZCOUNTER_H32   0xCF
#define PUNIT_GVD_SPARE0    0xD0
#define PUNIT_GPU_EC    0xD1
#define PUNIT_GPU_EC_VIRUS  0xD2
#define PUNIT_GPU_LFM   0xD3
#define PUNIT_GPU_FREQ_REQ  0xD4
#define PUNIT_GPU_TURBO_MIN_ENERGY  0xD5
#define PUNIT_GVD_SPARE1    0xD6
#define PUNIT_GVD_SPARE2    0xD7
#define PUNIT_GPU_FREQ_STS  0xD8
#define PUNIT_GVD_SPARE3    0xD9
#define PUNIT_GVD_SPARE4    0xDA
#define PUNIT_GVD_SPARE5    0xDB
#define PUNIT_MEDIA_TURBO_REQ   0xDC
#define PUNIT_GENLC_COUNTER_L32 0xDD
#define PUNIT_GENLC_COUNTER_H32 0xDE
#define PUNIT_GVD_SPARE6    0xDF
#define PUNIT_RTC_AONT_UPDTCYC  0xE1
#define PUNIT_RTC_AONT_INCRAMT1 0xE3
#define PUNIT_RTC_AONT_INCRAMT2 0xE4
#define PUNIT_RTC_AONT_INCRAMT3 0xE5
#define PUNIT_RTC_AONT_HIGH 0xE6
#define PUNIT_RTC_AONT_LOW  0xE7
#define PUNIT_SEND_MSG_TO_MTSC  0xF0
#define PUNIT_CORE_AONT_DATA_L  0xF1
#define PUNIT_CORE_AONT_DATA_H  0xF2
#define PUNIT_AONT_CLOCK_CONFIG 0xF3
#define PUNIT_FUSE_BUS0 0xF4
#define PUNIT_FUSE_BUS1 0xF5
#define PUNIT_FUSE_BUS2 0xF6
#define PUNIT_FUSE_BUS3 0xF7
#define PUNIT_FUSE_BUS4 0xFA
#define PUNIT_FUSE_BUS5 0xFB
#define PUNIT_FUSE_BUS6 0xFC
#define PUNIT_FUSE_BUS7 0xFD
#define PUNIT_FUSE_BUS8 0xFE
#define PUNIT_PGOVR_SBRSTOVR_FORCE1  0x100
#define PUNIT_PGOVR_SBRSTOVR_FORCE0  0x101
#define PUNIT_PGOVR_SBRSTOVR1_FORCE1 0x102
#define PUNIT_PGOVR_SBRSTOVR1_FORCE0 0x103
#define PUNIT_MMIO_SPARE0            0x104
#define PUNIT_MMIO_SPARE1            0x105
#define PUNIT_MMIO_SPARE2            0x106
#define PUNIT_MMIO_SPARE3            0x107
#define DPTF_TELB                    0x108
#define DPTF_GFXT                    0x109
#define DPTF_VEDT                    0x10A
#define DPTF_VECT                    0x10B
#define DPTF_VSPT                    0x10C
#define DPTF_ISPT                    0x10D
#define PUNIT_MMIO_SPARE10           0x10E
#define PUNIT_MMIO_SPARE11           0x10F

#define R_PUINT_INTR_LAT                0x3C
#define R_PUNIT_THERMAL_DEVICE_IRQ      0x700C
#define V_PUINT_THERMAL_DEVICE_IRQ      0x18
#define B_PUINT_THERMAL_DEVICE_IRQ_LOCK BIT31
//
// Device 2 Register Equates
//
#define SA_IGD_BUS              0x00
#define SA_IGD_DEV              0x02
#define SA_IGD_FUN_0            0x00
#define SA_IGD_FUN_1            0x01
#define SA_IGD_DEV_FUN          (SA_IGD_DEV << 3)
#define SA_IGD_BUS_DEV_FUN      (SA_MC_BUS << 8) + SA_IGD_DEV_FUN

#define R_SA_IGD_VID            0x00
#define V_SA_IGD_VID            0x8086
#define V_SA_IGD_DID            0x0A84
#define V_SA_IGD_DID_BXTP       0x5A84 // 18EU SKU
#define V_SA_IGD_DID_BXTP_1     0x5A85 // 12EU SKU
#define R_SA_IGD_CMD            0x04
#define R_SA_IGD_GTTMMADR       0x10
#define R_SA_IGD_GMADR          0x18
#define R_SA_IGD_MSAC_OFFSET    0x62  ///< Multisize Aperture Control


#define IGD_SWSCI_OFFSET        0x00E0      //Software SCI 0xE0 2
#define IGD_ASLS_OFFSET         0x00FC      // ASL Storage
//
// CCK Registers Offset
// Port ID: 14h
#define CCK_FUSE_REGISTER_0     0x0008

#define R_SA_IGD_AUD_FREQ_CNTRL_OFFSET 0x65900 ///< iDisplay Audio BCLK Frequency Control
#define B_SA_IGD_AUD_FREQ_CNTRL_TMODE  BIT15   ///< T-Mode: 0b - 2T, 1b - 1T
#define B_SA_IGD_AUD_FREQ_CNTRL_96MHZ  BIT4    ///< 96 MHz BCLK
#define B_SA_IGD_AUD_FREQ_CNTRL_48MHZ  BIT3    ///< 48 MHz BCLK


//
// IUNIT Registers Offset
// Port ID: 32h
#define IUNIT_SENSOR_FREQ_CTL        0x16C
#define IUNIT_SENSOR_CLK_CTL         0x170
#define B_OSC_CLK0_EN                BIT0
#define B_OSC_CLK0_SEL               BIT1
#define B_OSC_CLK1_EN                BIT2
#define B_OSC_CLK1_SEL               BIT3
#define B_OSC_CLK2_EN                BIT4
#define B_OSC_CLK2_SEL               BIT5
#define B_OSC_CLK3_EN                BIT6
#define B_OSC_CLK3_SEL               BIT7


#endif
