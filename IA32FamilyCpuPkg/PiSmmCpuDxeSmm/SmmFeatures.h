//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/** @file
  The CPU specific programming for PiSmmCpuDxeSmm module.

  Copyright (c) 2010 - 2014, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#ifndef __SMM_FEATURES_H__
#define __SMM_FEATURES_H__

////////
// Below definition is from IA32 SDM
////////
#define EFI_CPUID_VERSION_INFO                 0x1
#define EFI_CPUID_CORE_TOPOLOGY                0x0B
#define EFI_CPUID_EXTENDED_FUNCTION            0x80000000
#define EFI_CPUID_VIR_PHY_ADDRESS_SIZE         0x80000008

#define EFI_MSR_IA32_MTRR_CAP                  0xFE
#define  IA32_MTRR_SMRR_SUPPORT_BIT            BIT11
#define  IA32_MTRR_EMRR_SUPPORT_BIT            BIT12
#define  IA32_MTRR_SMRR2_SUPPORT_BIT           BIT13

#define EFI_MSR_IA32_FEATURE_CONTROL           0x3A
#define  IA32_SMRR_ENABLE_BIT                  BIT3

#define  EFI_MSR_SMRR_PHYS_MASK_VALID          BIT11
#define  EFI_MSR_SMRR_MASK                     0xFFFFF000
#define  EFI_MSR_EMRR_PHYS_MASK_LOCK           BIT10
#define  EFI_MSR_EMRR_MASK                     0xFFFFF000

#define EFI_MSR_CORE2_SMRR_PHYS_BASE           0xA0
#define EFI_MSR_CORE2_SMRR_PHYS_MASK           0xA1

#define EFI_MSR_NEHALEM_SMRR_PHYS_BASE         0x1F2
#define EFI_MSR_NEHALEM_SMRR_PHYS_MASK         0x1F3
//OVERRIDE:Start
#define EFI_MSR_SMRR_PHYS_BASE                 0x1F2
#define EFI_MSR_SMRR_PHYS_MASK                 0x1F3
//OVERRIDE:End

#define MTRR_CACHE_UNCACHEABLE                 0
#define CACHE_WRITE_PROTECT                    5
#define CACHE_WRITE_BACK                       6
#define SMM_DEFAULT_SMBASE                     0x30000

////////
// Below definition is from CPU BWG
////////
#define EFI_MSR_NEHALEM_EMRR_PHYS_BASE         0x1F4
#define EFI_MSR_NEHALEM_EMRR_PHYS_MASK         0x1F5
//OVERRIDE:Start
#define EFI_MSR_EMRR_PHYS_BASE                 0x1F4
#define EFI_MSR_EMRR_PHYS_MASK                 0x1F5
//OVERRIDE:End

#define EFI_MSR_HASWELL_SMRR2_PHYS_BASE        0x1F6
#define EFI_MSR_HASWELL_SMRR2_PHYS_MASK        0x1F7

#define EFI_MSR_HASWELL_UNCORE_EMRR_BASE       0x2F4

#define EFI_MSR_HASWELL_UNCORE_EMRR_MASK       0x2F5
#define  MSR_UNCORE_EMRR_MASK_LOCK_BIT         BIT10
#define  MSR_UNCORE_EMRR_MASK_RANGE_ENABLE_BIT BIT11

#define EFI_MSR_HASWELL_SMM_MCA_CAP            0x17D
#define  SMM_PROT_MODE_BASE_BIT                BIT54
#define  TARGETED_SMI_BIT                      BIT56
#define  SMM_CPU_SVRSTR_BIT                    BIT57
#define  SMM_CODE_ACCESS_CHK_BIT               BIT58
#define  LONG_FLOW_INDICATION_BIT              BIT59

#define EFI_MSR_HASWELL_SMM_FEATURE_CONTROL    0x4E0
#define  SMM_FEATURE_CONTROL_LOCK_BIT          BIT0
#define  SMM_CPU_SAVE_EN_BIT                   BIT1
#define  SMM_CODE_CHK_EN_BIT                   BIT2

#define EFI_MSR_HASWELL_EVENT_CTL_HLT_IO       0xC1F
#define EFI_MSR_HASWELL_SMBASE                 0xC20
#define EFI_MSR_HASWELL_SMMREVID               0xC21
#define EFI_MSR_HASWELL_RIP                    0xC04
#define EFI_MSR_HASWELL_EFER                   0xC03

#define SMM_HASWELL_CLIENT_LOG_PROC_EN_BIT_LENGTH     12
#define SMM_HASWELL_SERVER_LOG_PROC_EN_BIT_LENGTH     64

#define EFI_MSR_HASWELL_SMM_ENABLE             0x4E1

#define EFI_MSR_HASWELL_SMM_DELAYED            0x4E2

#define EFI_MSR_HASWELL_SMM_BLOCKED            0x4E3

#define EFI_MSR_HASWELL_IEDBASE                0xC22

#define EFI_MSR_NEHALEM_PCIEXBAR               0x300

#define EFI_MSR_SMM_SAVE_CONTROL               0x3e
#define SAVE_FLOATING_POINT_POINTERS           BIT0

#define EFI_MSR_THREAD_ID_INFO                 0x53

#define EFI_MSR_SMM_PROT_MODE_BASE             0x9D
#define  SMM_PROT_MODE_BASE_ENABLE_BIT         BIT0
#define  SMM_PROT_MODE_BASE_PADDR_MASK         0x00000000FFFFFF00ULL

#define PLATFORM_INFO_SMM_SAVE_CONTROL         BIT16

#define SMM_PENTIUM4_IEDBASE_OFFSET            0xFF04
#define SMM_CORE2_IEDBASE_OFFSET               0xFEEC
#define SMM_NEHALEM_IEDBASE_OFFSET             0xFEEC
#define SMM_PENTIUM4_DEFAULT_IEDBASE           0x30000
#define SMM_CORE2_DEFAULT_IEDBASE              0x30000
#define SMM_NEHALEM_DEFAULT_IEDBASE            0x50000
#define IED_STRING                             "INTEL RSVD"

#define NEHALEM_EX_UU_CR_U_PCSR_FW_SCRATCH_8   0xFEB204A0

#define NEHALEM_SAD_MCSEG_BASE(Bus)            PCI_LIB_ADDRESS (Bus, 0, 1, 0x60)
#define NEHALEM_SAD_MCSEG_MASK(Bus)            PCI_LIB_ADDRESS (Bus, 0, 1, 0x68)

#define NEHALEM_SAD_MCSEG_MASK_LOCK            BIT10
#define NEHALEM_SAD_MCSEG_MASK_ENABLE          BIT11

#define EFI_MSR_CPU_BUS_NO                     0x300
#define IVYBRIDGE_NON_STICKY_SCRATCHPAD3(Bus)  PCI_LIB_ADDRESS (Bus, 11, 3, 0x6C)
#define HASWELL_NON_STICKY_SCRATCHPAD3(Bus)    PCI_LIB_ADDRESS (Bus, 16, 7, 0x6C)
//
// Server register defines
// 
#define HASWELL_SMM_DELAYED0_REG(Bus)          PCI_LIB_ADDRESS (Bus, 16, 6, 0x40)
#define HASWELL_SMM_DELAYED1_REG(Bus)          PCI_LIB_ADDRESS (Bus, 16, 6, 0x44)
#define HASWELL_SMM_BLOCKED0_REG(Bus)          PCI_LIB_ADDRESS (Bus, 16, 6, 0x48)
#define HASWELL_SMM_BLOCKED1_REG(Bus)          PCI_LIB_ADDRESS (Bus, 16, 6, 0x4C)
#define HASWELL_SMM_ENABLE0_REG(Bus)           PCI_LIB_ADDRESS (Bus, 16, 6, 0x50)
#define HASWELL_SMM_ENABLE1_REG(Bus)           PCI_LIB_ADDRESS (Bus, 16, 6, 0x54)
#define HASWELL_SMM_FEATURE_CONTROL_REG(Bus)   PCI_LIB_ADDRESS (Bus, 16, 6, 0x58)

////////
// Below section is definition for CPU SMM Feature context
////////

//
// Structure to describe CPU identification mapping
// if ((CPUID_EAX(1) & Mask) == (Signature & Mask)), it means matched.
//
typedef struct {
  UINT32  Signature;
  UINT32  Mask;
} CPUID_MAPPING;

//
// CPU SMM familiy
//
typedef enum {
  CpuPentium4,
  CpuCore2,
  CpuNehalem,
  CpuSilvermont,
  CpuHaswell,
//OVERRIDE:Start
  CpuCedarView,
  CpuValleyView,
  CpuBroxton,
//OVERRIDE:END
  CpuSmmFamilyMax
} CPU_SMM_FAMILY;

//
// Structure to describe CPU SMM class
//
typedef struct {
  CPU_SMM_FAMILY    Family;
  UINT32            MappingCount;
  CPUID_MAPPING     *MappingTable;
} CPU_SMM_CLASS;

//
// Structure to describe CPU_SMM_FEATURE_CONTEXT
//
typedef struct {
  BOOLEAN          SmrrEnabled;
  BOOLEAN          EmrrSupported;
  BOOLEAN          Smrr2Enabled;
} CPU_SMM_FEATURE_CONTEXT;

//
// Pentium4 CPUID signatures
//
#define CPUID_SIGNATURE_PENTIUM                 0x00000F00

//
// Core2 CPUID signatures
//
#define CPUID_SIGNATURE_CONROE                  0x000006F0
#define CPUID_SIGNATURE_CONROE_L                0x00010660
#define CPUID_SIGNATURE_WOLFDALE                0x00010670
#define CPUID_SIGNATURE_DUNNINGTON              0x000106D0
#define CPUID_SIGNATURE_SILVERTHORNE            0x000106C0
#define CPUID_SIGNATURE_TUNNELCREEK             0x00020660

//
// Nehalem CPUID signatures
//
#define CPUID_SIGNATURE_NEHALEM                 0x000106A0
#define CPUID_SIGNATURE_LYNNFIELD               0x000106E0
#define CPUID_SIGNATURE_HAVENDALE               0x000106F0
#define CPUID_SIGNATURE_NEHALEM_EX              0x000206E0
#define CPUID_SIGNATURE_CLARKDALE               0x00020650
#define CPUID_SIGNATURE_WESTMERE                0x000206C0
#define CPUID_SIGNATURE_WESTMERE_EX             0x000206F0
#define CPUID_SIGNATURE_SANDYBRIDGE             0x000206A0
#define CPUID_SIGNATURE_SANDYBRIDGE_EP          0x000206D0
#define CPUID_SIGNATURE_IVYBRIDGE_CLIENT        0x000306A0
#define CPUID_SIGNATURE_IVYBRIDGE_SERVER        0x000306E0

//
// SilverMont CPUID signatures
//
#define CPUID_SIGNATURE_VALLEYVIEW              0x00030670
#define CPUID_SIGNATURE_AVOTON                  0x000406D0

//
// Haswell CPUID signatures
//
#define CPUID_SIGNATURE_HASWELL_CLIENT          0x000306C0
#define CPUID_SIGNATURE_HASWELL_SERVER          0x000306F0

//OVERRIDE:Start

//
// ATOM CPUID signatures
//
#define CPUID_SIGNATURE_CEDARVIEW               0x00030660
#define CPUID_SIGNATURE_VALLEYVIEW2             0x00030671
#define CPUID_SIGNATURE_CHERRYVIEW              0x000406C0
#define CPUID_SIGNATURE_BROXTON                 0x000506C0
//OVERRIDE:END

//
// CPUID masks
//
#define CPUID_MASK_NO_STEPPING                  0x0FFF0FF0
#define CPUID_MASK_NO_STEPPING_MODEL            0x0FFF0F00

//
// Enumerate registers which differ between client and server
//
typedef enum {
  SmmRegFeatureControl,
  SmmRegSmmEnable,
  SmmRegSmmDelayed,
  SmmRegSmmBlocked
} SMM_REG_NAME;

extern BOOLEAN          mSmmCodeAccessCheckEnable;
extern BOOLEAN          mSmmMsrSaveStateEnable;
extern UINT32           mSmrr2Base;
extern UINT32           mSmrr2Size;
extern UINT8            mSmrr2CacheType;
extern BOOLEAN          mSmmUseDelayIndication;
extern BOOLEAN          mSmmUseBlockIndication;
extern BOOLEAN          mSmmEnableIndication;
extern CPU_SMM_CLASS    *mThisCpu;
extern SPIN_LOCK        mCsrAccessLock;
extern BOOLEAN          mIEDEnabled;
extern UINT32           mIEDRamSize;

/**
  Disable SMRR register when SmmInit set SMM MTRRs.
**/
VOID
DisableSmrr (
  VOID
  );

/**
  Enable SMRR register when SmmInit restore non SMM MTRRs.
**/
VOID
ReenableSmrr (
  VOID
  );

/**
  Return if it is needed to configure MTRR to set TSEG cacheability.

  @retval  TRUE  - we need configure MTRR
  @retval  FALSE - we do not need configure MTRR
**/
BOOLEAN
NeedConfigureMtrrs (
  VOID
  );

/**
  Processor specific hook point at each SMM entry.

  @param  CpuIndex    The index of the cpu which need to check.
**/
VOID
SmmRendezvousEntry (
  IN UINTN  CpuIndex
  );

/**
  Processor specific hook point at each SMM exit.

  @param  CpuIndex    The index of the cpu which need to check.
**/
VOID
SmmRendezvousExit (
  IN UINTN  CpuIndex
  );

/**
  Initialize SMRR context in SMM Init.
**/
VOID
InitializeSmmMtrrManager (
  VOID
  );

/**
  Initialize SMRR/EMRR/SMBASE/IED/SMM Sync features in SMM Relocate.

  @param  ProcessorNumber    The processor number
  @param  SmrrBase           The base address of SMRR.
  @param  SmrrSize           The size of SMRR.
  @param  SmBase             The SMBASE value.
  @param  IedBase            The base address of IED. -1 when IED is disabled.
  @param  IsBsp              If this processor treated as BSP.
**/
VOID
SmmInitiFeatures (
  IN UINTN   ProcessorNumber,
  IN UINT32  SmrrBase,
  IN UINT32  SmrrSize,
  IN UINT32  SmBase,
  IN UINT32  IedBase,
  IN BOOLEAN IsBsp
  );

/**
  Configure SMM Code Access Check feature for all processors.
  SMM Feature Control MSR will be locked after configuration.
**/
VOID
ConfigSmmCodeAccessCheck (
  VOID
  );

/**
  Read MSR or CSR based on the CPU type Register to read.

  NOTE: Since platform may uses I/O ports 0xCF8 and 0xCFC to access 
        CSR, we need to use SPIN_LOCK to avoid collision on MP System.

  @param[in]  CpuIndex  The processor index.
  @param[in]  RegName   Register name.
  
  @return 64-bit value read from register.
    
**/
UINT64
SmmReadReg64 (
  IN  UINTN           CpuIndex,
  IN  SMM_REG_NAME    RegName
  );

/**
  Write MSR or CSR based on the CPU type Register to write.

  NOTE: Since platform may uses I/O ports 0xCF8 and 0xCFC to access 
        CSR, we need to use SPIN_LOCK to avoid collision on MP System.

  @param[in]  CpuIndex  The processor index.
  @param[in]  RegName   Register name.
  @param[in]  RegValue  64-bit Register value.
    
**/
VOID
SmmWriteReg64 (
  IN  UINTN           CpuIndex,
  IN  SMM_REG_NAME    RegName,
  IN  UINT64          RegValue
  );
  
//
// SMMSEG_FEATURE_ENABLES bitmap
//
#define SMMSEG_FEATURE_ENABLE                  BIT0
#define SMMSEG_FEATURE_CR4_MCE_CTL_ENABLE      BIT1

//
// Structure to describe CPU SMM Protected Mode
//
typedef struct {
  UINT32  Reserved1;
  UINT32  SmmSegFeatureEnables;   // SMM_SEG_FEATURE_ENABLES
  UINT32  GDTRLimit;
  UINT32  GDTRBaseOffset;
  UINT32  CSSelector;
  UINT32  Reserved3;
  UINT32  ESPOffset;
  UINT32  Reserved4;
  UINT32  IDTRLimit;
  UINT32  IDTRBaseOffset;
} SMMSEG;

#define  SMM_PROT_MODE_GDT_ENTRY_COUNT    27

//
// SMM PROT MODE CONTEXT (total 0x100 bytes)
//
typedef struct {
  SMMSEG                    SmmProtectedModeSMMSEG;                       // 40 bytes
  IA32_SEGMENT_DESCRIPTOR   SmmProtectedModeGdt[SMM_PROT_MODE_GDT_ENTRY_COUNT];  // 27 * 8 = 216 bytes
} SMM_PROT_MODE_CONTEXT;

extern SMM_PROT_MODE_CONTEXT    *mSmmProtModeContext;

/**
  Setup SMM Protected Mode context for processor.

  @param  ProcessorNumber    The processor number.
  @param  SmBase             The SMBASE value of the processor.
  @param  StackAddress       Stack address of the processor.
  @param  GdtBase            Gdt table base address of the processor.
  @param  GdtSize            Gdt table size of the processor.
  
**/
VOID
SetupSmmProtectedModeContext (
  IN UINTN                    ProcessorNumber,
  IN UINT32                   SmBase,
  IN UINT32                   StackAddress,
  IN UINTN                    GdtBase,
  IN UINTN                    GdtSize
  );

#endif
