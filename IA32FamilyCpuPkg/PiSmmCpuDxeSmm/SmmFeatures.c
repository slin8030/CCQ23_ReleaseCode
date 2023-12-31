/** @file
  The CPU specific programming for PiSmmCpuDxeSmm module, such as SMRR, EMRR, IED.
  Currently below CPUs are supported.

  0x00000F00 // Pentium4

  0x000006F0 // Conroe
  0x00010660 // Conroe-L
  0x00010670 // Wolfdale
  0x000106D0 // Dunnington
  0x000106C0 // Silverthorne
  0x00020660 // Tunnel Creek

  0x000106A0 // Nehalem
  0x000106E0 // Lynnfield
  0x000106F0 // Havendale
  0x000206E0 // Nehalem-EX
  0x00020650 // Clarkdale
  0x000206C0 // Westmere
  0x000206F0 // Westmere-EX
  0x000206A0 // SandyBridge
  0x000206D0 // SandyBridge-EP
  0x00030670 // ValleyView
  0x000306A0 // Ivybridge Client
  0x000306E0 // Ivybridge Server
  0x000306C0 // Haswell Client
  0x000306F0 // Haswell Server
  0x000406D0 // Avoton

@copyright
  Copyright (c) 1999 - 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains an 'Intel Peripheral Driver' and is uniquely
  identified as "Intel Reference Module" and is licensed for Intel
  CPUs and chipsets under the terms of your license agreement with
  Intel or your vendor. This file may be modified by the user, subject
  to additional terms of the license agreement.

@par Specification
**/

#include <Base.h>

#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/CpuLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Library/LocalApicLib.h>

#include "PiSmmCpuDxeSmm.h"
#include "SmmFeatures.h"

//
// The CPUID mapping for Pentium4
//
CPUID_MAPPING  mPentium4Map[] = {
  {CPUID_SIGNATURE_PENTIUM, CPUID_MASK_NO_STEPPING_MODEL},      // Pentium
  };

//
// The CLASS for Pentium4
//
CPU_SMM_CLASS mPentium4Class = {
  CpuPentium4,
  sizeof(mPentium4Map)/sizeof(mPentium4Map[0]),
  mPentium4Map,
  };

//
// The CPUID mapping for Core2
//
CPUID_MAPPING  mCore2Map[] = {
  {CPUID_SIGNATURE_CONROE, CPUID_MASK_NO_STEPPING},             // Conroe
  {CPUID_SIGNATURE_CONROE_L, CPUID_MASK_NO_STEPPING},           // Conroe-L
  {CPUID_SIGNATURE_WOLFDALE, CPUID_MASK_NO_STEPPING},           // Wolfdale
  {CPUID_SIGNATURE_DUNNINGTON, CPUID_MASK_NO_STEPPING},         // Dunnington
  {CPUID_SIGNATURE_SILVERTHORNE, CPUID_MASK_NO_STEPPING},       // Silverthorne
  {CPUID_SIGNATURE_TUNNELCREEK, CPUID_MASK_NO_STEPPING},        // Tunnel Creek
  };

//
// The CPUID mapping for Atom
//
CPUID_MAPPING  mAtomMap[] = {
  {CPUID_SIGNATURE_SILVERTHORNE, CPUID_MASK_NO_STEPPING},       // Silverthorne
  {CPUID_SIGNATURE_TUNNELCREEK, CPUID_MASK_NO_STEPPING},        // Tunnel Creek
  };

//
// The CLASS for Core2
//
CPU_SMM_CLASS mCore2Class = {
  CpuCore2,
  sizeof(mCore2Map)/sizeof(mCore2Map[0]),
  mCore2Map,
  };

//
// The CPUID mapping for Nehalem
//
CPUID_MAPPING  mNehalemMap[] = {
  {CPUID_SIGNATURE_NEHALEM, CPUID_MASK_NO_STEPPING},            // Nehalem
  {CPUID_SIGNATURE_LYNNFIELD, CPUID_MASK_NO_STEPPING},          // Lynnfield
  {CPUID_SIGNATURE_HAVENDALE, CPUID_MASK_NO_STEPPING},          // Havendale
  {CPUID_SIGNATURE_NEHALEM_EX, CPUID_MASK_NO_STEPPING},         // Nehalem-EX
  {CPUID_SIGNATURE_CLARKDALE, CPUID_MASK_NO_STEPPING},          // Clarkdale
  {CPUID_SIGNATURE_WESTMERE, CPUID_MASK_NO_STEPPING},           // Westmere
  {CPUID_SIGNATURE_WESTMERE_EX, CPUID_MASK_NO_STEPPING},        // Westmere-EX
  {CPUID_SIGNATURE_SANDYBRIDGE, CPUID_MASK_NO_STEPPING},        // SandyBridge
  {CPUID_SIGNATURE_SANDYBRIDGE_EP, CPUID_MASK_NO_STEPPING},     // SandyBridge-EP
  {CPUID_SIGNATURE_IVYBRIDGE_CLIENT, CPUID_MASK_NO_STEPPING},   // Ivybridge Client
  {CPUID_SIGNATURE_IVYBRIDGE_SERVER, CPUID_MASK_NO_STEPPING},   // Ivybridge Server
  };

//
// The CPUID mapping for NehalemEx
//
CPUID_MAPPING  mNehalemExMap[] = {
  {CPUID_SIGNATURE_NEHALEM_EX, CPUID_MASK_NO_STEPPING},         // Nehalem-EX
  {CPUID_SIGNATURE_WESTMERE_EX, CPUID_MASK_NO_STEPPING},        // Westmere-EX
  };

//
// The CLASS for Nehalem
//
CPU_SMM_CLASS mNehalemClass = {
  CpuNehalem,
  sizeof(mNehalemMap)/sizeof(mNehalemMap[0]),
  mNehalemMap,
  };

//
// The CPUID mapping for Silvermont
//
CPUID_MAPPING mSilvermontMap[] = {
  {CPUID_SIGNATURE_VALLEYVIEW, CPUID_MASK_NO_STEPPING},         // ValleyView
  {CPUID_SIGNATURE_AVOTON, CPUID_MASK_NO_STEPPING},             // Avoton
};

//
// The CLASS for Silvermont
//
CPU_SMM_CLASS mSilvermontClass = {
  CpuSilvermont,
  sizeof(mSilvermontMap)/sizeof(mSilvermontMap[0]),
  mSilvermontMap,
  };

//
// The CPUID mapping for Haswell
//
CPUID_MAPPING  mHaswellMap[] = {
  {CPUID_SIGNATURE_HASWELL_CLIENT, CPUID_MASK_NO_STEPPING},    // Haswell Client
  {CPUID_SIGNATURE_HASWELL_SERVER, CPUID_MASK_NO_STEPPING},    // Haswell Server
  };

//
// The CLASS for Haswell
//
CPU_SMM_CLASS mHaswellClass = {
  CpuHaswell,
  sizeof(mHaswellMap)/sizeof(mHaswellMap[0]),
  mHaswellMap,
  };

//OVERRIDE:Start
//
// The CPUID mapping for CedarView
//
CPUID_MAPPING mCedarViewMap[] = {
  {CPUID_SIGNATURE_CEDARVIEW, CPUID_MASK_NO_STEPPING},
};

//
// The CLASS for CedarView
//
CPU_SMM_CLASS mCedarViewClass = {
  CpuCedarView,
  sizeof(mCedarViewMap)/sizeof(mCedarViewMap[0]),
  mCedarViewMap,
  };

//
// The CPUID mapping for ValleyView
//
CPUID_MAPPING mValleyViewMap[] = {
  {CPUID_SIGNATURE_VALLEYVIEW, CPUID_MASK_NO_STEPPING},
};

//
// The CLASS for VALLEYView
//
CPU_SMM_CLASS mValleyViewClass = {
  CpuValleyView,
  sizeof(mValleyViewMap)/sizeof(mValleyViewMap[0]),
  mValleyViewMap,
  };

//
// The CPUID mapping for Broxton
//
CPUID_MAPPING mBroxtonMap[] = {
  {CPUID_SIGNATURE_BROXTON, CPUID_MASK_NO_STEPPING},
};

//
// The CLASS for Broxton
//
CPU_SMM_CLASS mWillowClass = {
  CpuBroxton,
  sizeof(mBroxtonMap)/sizeof(mBroxtonMap[0]),
  mBroxtonMap,
  };

//OVERRIDE:End

//
// This table defines supported CPU class
//
CPU_SMM_CLASS *mCpuClasstable[] = {
  &mPentium4Class,
  &mCore2Class,
  &mNehalemClass,
  &mSilvermontClass,
  &mHaswellClass,
//OVERRIDE:Start
  &mCedarViewClass,
  &mValleyViewClass,
  &mWillowClass
//OVERRIDE:End
  };

////////
// Below section is common definition
////////

//
// Assumes UP, or MP with identical feature set
//
CPU_SMM_FEATURE_CONTEXT  mFeatureContext;

CPU_SMM_CLASS            *mThisCpu;
BOOLEAN                  mSmmCodeAccessCheckEnable = FALSE;
BOOLEAN                  mSmmMsrSaveStateEnable;
BOOLEAN                  mSmmInitSaveStateInMsr;
BOOLEAN                  mSaveStateInMsr = FALSE;
UINT32                   mSmrr2Base;
UINT32                   mSmrr2Size;
UINT8                    mSmrr2CacheType;
BOOLEAN                  mSmmUseDelayIndication;
BOOLEAN                  mSmmUseBlockIndication;
BOOLEAN                  mSmmEnableIndication;
UINT8                    mNonIioBusNum[FixedPcdGet32 (PcdCpuMaxLogicalProcessorNumber)];
SPIN_LOCK                mCsrAccessLock;
BOOLEAN                  mIEDEnabled;
UINT32                   mIEDRamSize;
BOOLEAN                  mSmmProtectedModeEnable;
SMM_PROT_MODE_CONTEXT    *mSmmProtModeContext;

/**
  Return if SMRR is supported

  @retval TRUE  SMRR is supported.
  @retval FALSE SMRR is not supported.

**/
BOOLEAN
IsSmrrSupported (
  VOID
  )
{
  UINT64                            MtrrCap;

  MtrrCap = AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP);
  if ((MtrrCap & IA32_MTRR_SMRR_SUPPORT_BIT) == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Return if SMRR2 is supported

  @retval TRUE  SMRR2 is supported.
  @retval FALSE SMRR2 is not supported.

**/
BOOLEAN
IsSmrr2Supported (
  VOID
  )
{
  UINT64                            MtrrCap;

  MtrrCap = AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP);
  if ((MtrrCap & IA32_MTRR_SMRR2_SUPPORT_BIT) == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Return if EMRR is supported

  @retval TRUE  EMRR is supported.
  @retval FALSE EMRR is not supported.

**/
BOOLEAN
IsEmrrSupported (
  VOID
  )
{
  UINT64                            MtrrCap;

  MtrrCap = AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP);
  if ((MtrrCap & IA32_MTRR_EMRR_SUPPORT_BIT) == 0) {
    return FALSE;
  } else {
    return TRUE;
  }
}

////////
// Below section is definition for CpuPentium4
////////

/**
  Programming IED.

  @param  IedBase            The base address of IED. -1 when IED is disabled.
  @param  IsBsp              If this processor treated as BSP.
**/
VOID
Pentium4RelocateIedBase (
  IN UINT32                IedBase,
  IN BOOLEAN               IsBsp
  )
{
  UINT32 StrSize;
  UINT32 *IedOffset;

  IedOffset = (UINT32 *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_PENTIUM4_IEDBASE_OFFSET);
  if (*IedOffset != SMM_PENTIUM4_DEFAULT_IEDBASE) {
    return ;
  }
  *IedOffset = IedBase;

  if ((IedBase != (UINT32)-1) && IsBsp) {
    // 48 byte header
    ZeroMem ((UINT8 *)(UINTN)IedBase, 48);
    // signature
    StrSize = sizeof(IED_STRING) - 1;
    CopyMem ((UINT8 *)(UINTN)IedBase, IED_STRING, StrSize);
    // IED size
    *(UINT32 *)(UINTN)(IedBase + 10) = mIEDRamSize;
  }

  return ;
}

////////
// Below section is definition for CpuCore2
////////

/**
  Return if SMRR is enabled

  @retval TRUE  SMRR is enabled.
  @retval FALSE SMRR is not enabled.

**/
BOOLEAN
Core2IsSmrrEnabled (
  VOID
  )
{
  UINT64                            SmrrMtrr;

  SmrrMtrr = AsmReadMsr64(EFI_MSR_IA32_FEATURE_CONTROL);
  if ((SmrrMtrr & IA32_SMRR_ENABLE_BIT) != 0)	{
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Initialize SMRR in SMM relocate.

  @param  SmrrBase           The base address of SMRR.
  @param  SmrrSize           The size of SMRR.
**/
VOID
Core2InitSmrr (
  IN UINT32                SmrrBase,
  IN UINT32                SmrrSize
  )
{
  AsmWriteMsr64 (EFI_MSR_CORE2_SMRR_PHYS_BASE, SmrrBase);
  AsmWriteMsr64 (EFI_MSR_CORE2_SMRR_PHYS_MASK, (~(SmrrSize - 1) & EFI_MSR_SMRR_MASK) | EFI_MSR_SMRR_PHYS_MASK_VALID);
}

/**
  Disable SMRR register when SmmInit replace non-SMM MTRRs.
**/
VOID
Core2DisableSmrr (
  VOID
  )
{
  AsmWriteMsr64(EFI_MSR_CORE2_SMRR_PHYS_MASK, AsmReadMsr64(EFI_MSR_CORE2_SMRR_PHYS_MASK) & ~EFI_MSR_SMRR_PHYS_MASK_VALID);
}

/**
  Enable SMRR register when SmmInit restores non-SMM MTRRs.
**/
VOID
Core2EnableSmrr (
  VOID
  )
{
  AsmWriteMsr64(EFI_MSR_CORE2_SMRR_PHYS_MASK, AsmReadMsr64(EFI_MSR_CORE2_SMRR_PHYS_MASK) | EFI_MSR_SMRR_PHYS_MASK_VALID);
}

/**
  Check if it is Atom processor.

  @retval TRUE  It is Atom.
  @retval FALSE It is not Atom.
**/
BOOLEAN
IsAtom (
  VOID
  )
{
  UINT32 RegEax;
  UINTN  Index;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);
  for (Index = 0; Index < sizeof(mAtomMap)/sizeof(mAtomMap[0]); Index++) {
    if ((RegEax & mAtomMap[Index].Mask) == (mAtomMap[Index].Signature & mAtomMap[Index].Mask)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Programming IED.

  @param  IedBase            The base address of IED. -1 when IED is disabled.
  @param  IsBsp              If this processor treated as BSP.
**/
VOID
Core2RelocateIedBase (
  IN UINT32                IedBase,
  IN BOOLEAN               IsBsp
  )
{
  UINT32 StrSize;
  UINT32 *IedOffset;

  if (IsAtom ()) {
    // No IED for Atom
    return ;
  }

  IedOffset = (UINT32 *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_CORE2_IEDBASE_OFFSET);
  if (*IedOffset != SMM_CORE2_DEFAULT_IEDBASE) {
    return ;
  }
  *IedOffset = IedBase;

  if ((IedBase != (UINT32)-1) && IsBsp) {
    // 48 byte header
    ZeroMem ((UINT8 *)(UINTN)IedBase, 48);
    // signature
    StrSize = sizeof(IED_STRING) - 1;
    CopyMem ((UINT8 *)(UINTN)IedBase, IED_STRING, StrSize);
    // IED size
    *(UINT32 *)(UINTN)(IedBase + 10) = mIEDRamSize;
  }

  return ;
}

////////
// Below section is definition for CpuNehalem
////////

/**
  Initialize EMRR in SMM relocate.

  @param  IedBase           IEDRAM base address.
  
**/
VOID
NehalemInitEmrr (
  IN UINT32          IedBase
  )
{
  UINT32                ApicId;
  UINT32                IntraPackageIdBits;
  UINT64                McSegBase;
  UINT64                McSegMask;
  UINT8                 PhysicalAddressBits;
  UINT64                ValidMtrrAddressMask;
  UINT32                RegEax;
  UINTN                 PciExBarSize;
  UINTN                 BusNumber;

  //
  // Note that Nehalem-EX and Westmere-EX does not support EMRR.
  //
  ApicId = GetInitialApicId ();

  AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, 1, &IntraPackageIdBits, NULL, NULL, NULL);
  IntraPackageIdBits &= 0x1f;
  
  AsmCpuid (EFI_CPUID_EXTENDED_FUNCTION, &RegEax, NULL, NULL, NULL);
  PhysicalAddressBits = 36;
  if (RegEax >= EFI_CPUID_VIR_PHY_ADDRESS_SIZE) {
    AsmCpuid (EFI_CPUID_VIR_PHY_ADDRESS_SIZE, &RegEax, NULL, NULL, NULL);
    PhysicalAddressBits = (UINT8) RegEax;
  }
  ValidMtrrAddressMask = (LShiftU64 (1, PhysicalAddressBits) - 1) & 0xfffffffffffff000ull;

  if ((ApicId >> IntraPackageIdBits) != 0) {
    //
    // IEDBASE + 3MB for socket 1
    //
    McSegBase = IedBase + 0x300000;
  } else {
    //
    // IEDBASE + 2MB for socket 0
    //
    McSegBase = IedBase + 0x200000;
  }
  McSegBase &= ValidMtrrAddressMask; 

  McSegMask = (UINT64)(~(UINT64)(0x200000 - 1)); 
  McSegMask &= ValidMtrrAddressMask; 

  //
  // SAD_MCSEG_BASE and SAD_MCSEG_MASK programming described in NHM BWG 0.70.
  // SAD_MCSEG_BASE and SAD_MCSEG_MASK programming is NOT specified in SNB and IVB BWG.
  // 0x206a0: SandyBridge
  // 0x206d0: SandyBridge-EP
  // 0x306A0: Ivybridge Client
  // 0x306E0: Ivybridge Server
  //
  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);
  if (((RegEax & 0x0FFF0FF0) != 0x000206A0) && 
      ((RegEax & 0x0FFF0FF0) != 0x000206D0) &&
      ((RegEax & 0x0FFF0FF0) != 0x000306A0) &&
      ((RegEax & 0x0FFF0FF0) != 0x000306E0)) {

    //
    // Determine PCI bus number of this processor
    //

    PciExBarSize = (((UINTN)AsmReadMsr64 (EFI_MSR_NEHALEM_PCIEXBAR)) & (BIT3 | BIT2 | BIT1)) >> 1;

    switch (PciExBarSize) {
      default:
      case 0x00:
        BusNumber = 0xFF;
        break;
      case 0x07:
        BusNumber = 0x7F;
        break;
      case 0x06:
        BusNumber = 0x3F;
        break;
    }

    BusNumber -= (ApicId >> IntraPackageIdBits);

    if (!(PciRead32 (NEHALEM_SAD_MCSEG_MASK (BusNumber)) & NEHALEM_SAD_MCSEG_MASK_LOCK)) {
      PciWrite32 (NEHALEM_SAD_MCSEG_BASE (BusNumber), (UINT32)McSegBase);
      PciWrite32 (NEHALEM_SAD_MCSEG_BASE (BusNumber) + 4, (UINT32)RShiftU64 (McSegBase, 32));

      PciWrite32 (NEHALEM_SAD_MCSEG_MASK (BusNumber) + 4, (UINT32)RShiftU64 (McSegMask, 32));    
      PciWrite32 (
        NEHALEM_SAD_MCSEG_MASK (BusNumber),
        ((UINT32)McSegMask) | NEHALEM_SAD_MCSEG_MASK_LOCK | NEHALEM_SAD_MCSEG_MASK_ENABLE
        );
    }
  }

  if (!(AsmReadMsr64 (EFI_MSR_NEHALEM_EMRR_PHYS_MASK) & EFI_MSR_EMRR_PHYS_MASK_LOCK)) {
    AsmWriteMsr64 (EFI_MSR_NEHALEM_EMRR_PHYS_BASE, McSegBase | CACHE_WRITE_BACK);
    AsmWriteMsr64 (EFI_MSR_NEHALEM_EMRR_PHYS_MASK, McSegMask | EFI_MSR_EMRR_PHYS_MASK_LOCK);
  }
}

/**
  Initialize SMRR in SMM relocate.

  @param  SmrrBase           The base address of SMRR.
  @param  SmrrSize           The size of SMRR.
**/
VOID
NehalemInitSmrr (
  IN UINT32                SmrrBase,
  IN UINT32                SmrrSize
  )
{
  AsmWriteMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_BASE, SmrrBase | CACHE_WRITE_BACK);
  AsmWriteMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_MASK, (~(SmrrSize - 1) & EFI_MSR_SMRR_MASK)); // Valid bit will be set in ConfigSmrr() at first SMI
}

/**
  Configure SMRR register at each SMM entry.
**/
VOID
NehalemConfigSmrr (
  VOID
  )
{
  UINT64 SmrrMask;

  SmrrMask = AsmReadMsr64 (EFI_MSR_NEHALEM_SMRR_PHYS_MASK);
  if ((SmrrMask & EFI_MSR_SMRR_PHYS_MASK_VALID) == 0) {
    AsmWriteMsr64(EFI_MSR_NEHALEM_SMRR_PHYS_MASK, SmrrMask | EFI_MSR_SMRR_PHYS_MASK_VALID);
  }
}

/**
  Check if it is Nehalem-EX processor.

  @retval TRUE  It is Nehalem-EX.
  @retval FALSE It is not Nehalem-EX.
**/
BOOLEAN
IsNehalemEx (
  VOID
  )
{
  UINT32 RegEax;
  UINTN  Index;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);
  for (Index = 0; Index < sizeof(mNehalemExMap)/sizeof(mNehalemExMap[0]); Index++) {
    if ((RegEax & mNehalemExMap[Index].Mask) == (mNehalemExMap[Index].Signature & mNehalemExMap[Index].Mask)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Check if it is Ivy Bridge-EX processor.

  @retval TRUE  It is Ivy Bridge-EX.
  @retval FALSE It is not Ivy Bridge-EX.
**/
BOOLEAN
IsIvyBridgeEx (
  VOID
  )
{
  UINT32 RegEax;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);
  if ((RegEax & CPUID_MASK_NO_STEPPING) == CPUID_SIGNATURE_IVYBRIDGE_SERVER) {
    return TRUE;
  }
  return FALSE;
}

/**
  Check if it is Haswell-EX processor.

  @retval TRUE  It is Haswell-EX.
  @retval FALSE It is not Haswell-EX.
**/
BOOLEAN
IsHaswellEx (
  VOID
  )
{
  UINT32 RegEax;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);
  if ((RegEax & CPUID_MASK_NO_STEPPING) == CPUID_SIGNATURE_HASWELL_SERVER) {
    return TRUE;
  }
  return FALSE;
}

/**
  Programming IED.

  @param  IedBase            The base address of IED. -1 when IED is disabled.
  @param  IsBsp              If this processor treated as BSP.
**/
VOID
NehalemRelocateIedBase (
  IN UINT32                IedBase,
  IN BOOLEAN               IsBsp
  )
{
  UINT32  StrSize;
  UINT32  *IedOffset;
  UINT32  ScratchPad;
  UINT32  Bus;
  BOOLEAN IedTraceInit;

  //
  // A minimum of 4MB IDERAM is required for Nehalem processors.
  //
  ASSERT (mIEDRamSize >= 0x400000);

  IedOffset = (UINT32 *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_NEHALEM_IEDBASE_OFFSET);
  *IedOffset = IedBase;

  if ((IedBase != (UINT32)-1) && IsBsp) {
    // 48 byte header
    ZeroMem ((UINT8 *)(UINTN)IedBase, 48);
    // signature
    StrSize = sizeof(IED_STRING) - 1;
    CopyMem ((UINT8 *)(UINTN)IedBase, IED_STRING, StrSize);
    // IED size
    *(UINT32 *)(UINTN)(IedBase + 10) = mIEDRamSize;

    // Set IED trace
    IedTraceInit = TRUE;
    ScratchPad = 0;
    if (IsNehalemEx ()) {
      ScratchPad = *(UINT32 *)(UINTN)NEHALEM_EX_UU_CR_U_PCSR_FW_SCRATCH_8;
    } else if (IsIvyBridgeEx ()) {
      Bus = (UINT32)AsmReadMsr64 (EFI_MSR_CPU_BUS_NO);
      //if MSR has valid Bus#
      if((Bus & BIT31) != 0) { 
        Bus = (Bus >> 8) & 0xff;
        //get base of IedTrace from BIOSNONSTICKYSCRATCHPAD3
        ScratchPad = PciRead32 (IVYBRIDGE_NON_STICKY_SCRATCHPAD3 (Bus));
      }
    } else {
      IedTraceInit = FALSE;
    }

    if (IedTraceInit) {
      if (ScratchPad == 0) {
        *(UINT64 *)(UINTN)(IedBase + 16) = (UINT64)-1;
      } else {
        // IED Trace Ptr = ScrtachPad * 1MB
        *(UINT64 *)(UINTN)(IedBase + 16) = LShiftU64 (ScratchPad, 20);
      }
    }
    // patch
    ZeroMem ((UINT8 *)(UINTN)IedBase + 0x100000, 32 * 0x1024);
  }
}

/**
  Programming IED.

  @param  ProcessorNumber    The processor number.
  @param  IedBase            The base address of IED. -1 when IED is disabled.
  @param  IsBsp              If this processor treated as BSP.
**/
VOID
HaswellRelocateIedBase (
  IN UINTN                 ProcessorNumber,
  IN UINT32                IedBase,
  IN BOOLEAN               IsBsp
  )
{
  UINT32 StrSize;
  UINT32 *IedOffset;
  UINT32 ScratchPad;
  UINT32 Bus;

  //
  // IED must be enabled for Haswell processor.
  // A minimum of 4MB IDERAM is required for Haswell processors.
  //
  ASSERT (mIEDEnabled);
  ASSERT (mIEDRamSize >= 0x400000);

  if (mSmmInitSaveStateInMsr) {
    AsmWriteMsr64 (EFI_MSR_HASWELL_IEDBASE, LShiftU64 (IedBase, 32));
  } else {
    IedOffset = (UINT32 *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_NEHALEM_IEDBASE_OFFSET);
    *IedOffset = IedBase;
  }

  if ((IedBase != (UINT32)-1) && IsBsp) {
    // 48 byte header
    ZeroMem ((UINT8 *)(UINTN)IedBase, 48);
    // signature
    StrSize = sizeof(IED_STRING) - 1;
    CopyMem ((UINT8 *)(UINTN)IedBase, IED_STRING, StrSize);
    // IED size
    *(UINT32 *)(UINTN)(IedBase + 10) = mIEDRamSize;

    // Set IED trace
    if (IsHaswellEx ()) {
      Bus = (UINT32)AsmReadMsr64 (EFI_MSR_CPU_BUS_NO);
      //if MSR has valid Bus#
      if((Bus & BIT31) != 0) { 
        Bus = (Bus >> 8) & 0xff;
        //get base of IedTrace from BIOSNONSTICKYSCRATCHPAD3
        ScratchPad = PciRead32 (HASWELL_NON_STICKY_SCRATCHPAD3 (Bus));

        if (ScratchPad == 0) {
          *(UINT64 *)(UINTN)(IedBase + 16) = (UINT64)-1;
        } else {
          // IED Trace Ptr = ScrtachPad * 1MB
          *(UINT64 *)(UINTN)(IedBase + 16) = LShiftU64 (ScratchPad, 20);
        }
      }
    }

    // patch
    ZeroMem ((UINT8 *)(UINTN)IedBase + 0x100000, 32 * 0x1024);
  }
}

/**
  Initialize EMRR in SMM relocation for Haswell processors.

  @param  IedBase           IEDRAM base address.
  
**/
VOID
HaswellInitEmrr (
  IN UINT32          IedBase
  )
{
  UINT32                ApicId;
  UINT32                IntraPackageIdBits;
  UINT64                McSegBase;
  UINT64                McSegMask;
  UINT8                 PhysicalAddressBits;
  UINT64                ValidMtrrAddressMask;
  UINT32                RegEax;

  ApicId = GetInitialApicId ();

  AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, 1, &IntraPackageIdBits, NULL, NULL, NULL);
  IntraPackageIdBits &= 0x1f;
  
  AsmCpuid (EFI_CPUID_EXTENDED_FUNCTION, &RegEax, NULL, NULL, NULL);
  PhysicalAddressBits = 36;
  if (RegEax >= EFI_CPUID_VIR_PHY_ADDRESS_SIZE) {
    AsmCpuid (EFI_CPUID_VIR_PHY_ADDRESS_SIZE, &RegEax, NULL, NULL, NULL);
    PhysicalAddressBits = (UINT8) RegEax;
  }
  ValidMtrrAddressMask = (LShiftU64 (1, PhysicalAddressBits) - 1) & 0xfffffffffffff000ull;

  if ((ApicId >> IntraPackageIdBits) != 0) {
    //
    // IEDBASE + 3MB for socket 1
    //
    McSegBase = IedBase + 0x300000;
  } else {
    //
    // IEDBASE + 2MB for socket 0
    //
    McSegBase = IedBase + 0x200000;
  }
  McSegBase &= ValidMtrrAddressMask; 

  McSegMask = (UINT64)(~(UINT64)(0x200000 - 1)); 
  McSegMask &= ValidMtrrAddressMask; 

  if ((AsmReadMsr64 (EFI_MSR_HASWELL_UNCORE_EMRR_MASK) & MSR_UNCORE_EMRR_MASK_LOCK_BIT) == 0) {
    AsmWriteMsr64 (EFI_MSR_HASWELL_UNCORE_EMRR_BASE, McSegBase);
    AsmWriteMsr64 (EFI_MSR_HASWELL_UNCORE_EMRR_MASK, McSegMask | MSR_UNCORE_EMRR_MASK_LOCK_BIT | MSR_UNCORE_EMRR_MASK_RANGE_ENABLE_BIT);
  }

  if (!(AsmReadMsr64 (EFI_MSR_NEHALEM_EMRR_PHYS_MASK) & EFI_MSR_EMRR_PHYS_MASK_LOCK)) {
    AsmWriteMsr64 (EFI_MSR_NEHALEM_EMRR_PHYS_BASE, McSegBase | CACHE_WRITE_BACK);
    AsmWriteMsr64 (EFI_MSR_NEHALEM_EMRR_PHYS_MASK, McSegMask | EFI_MSR_EMRR_PHYS_MASK_LOCK);
  }
}

/**
  Config MSR Save State feature for Haswell processors.

  @param[in]  ProcessorNumber        The processor number.

**/
VOID
HaswellConfigMsrSaveState (
  IN UINTN   ProcessorNumber
  )
{
  UINT64 SmmFeatureControlMsr;

  if (mSmmMsrSaveStateEnable) {
    if ((AsmReadMsr64 (EFI_MSR_HASWELL_SMM_MCA_CAP) & SMM_CPU_SVRSTR_BIT) != 0) {
      SmmFeatureControlMsr = SmmReadReg64 (ProcessorNumber, SmmRegFeatureControl);
      ASSERT ((SmmFeatureControlMsr & SMM_FEATURE_CONTROL_LOCK_BIT) == 0);

      if ((SmmFeatureControlMsr & SMM_CPU_SAVE_EN_BIT) == 0) {
        //
        // SMM_CPU_SAVE_EN_BIT is package scope, so when a thead in a package sets this bit,
        // MSR Save State feature will be enabled for all threads in the package in subsequent SMIs.
        //
        // Lock bit is not set here. It will be set when configuring SMM Code Access Check feature later.
        //
        SmmFeatureControlMsr |= SMM_CPU_SAVE_EN_BIT;
        SmmWriteReg64 (ProcessorNumber, SmmRegFeatureControl, SmmFeatureControlMsr);
        mSaveStateInMsr = TRUE;
      }
    }
  }
}

/**
  Initialize SMRR2 in SMM relocate.

  @param  Smrr2Base          The base address of SMRR2.
  @param  Smrr2Size          The size of SMRR2.
**/
VOID
HaswellInitSmrr2 (
  IN UINT32                Smrr2Base,
  IN UINT32                Smrr2Size
  )
{
  ASSERT (Smrr2Size != 0);

  if ((AsmReadMsr64 (EFI_MSR_HASWELL_SMRR2_PHYS_MASK) & EFI_MSR_SMRR_PHYS_MASK_VALID) == 0) {
    //
    // Program SMRR2 Base and Mask
    //
    AsmWriteMsr64 (EFI_MSR_HASWELL_SMRR2_PHYS_BASE, Smrr2Base | mSmrr2CacheType);
    AsmWriteMsr64 (EFI_MSR_HASWELL_SMRR2_PHYS_MASK, (~(Smrr2Size - 1) & EFI_MSR_SMRR_MASK) | EFI_MSR_SMRR_PHYS_MASK_VALID); 
  }
}

//OVERRIDE:Start
/**
  Initialize SMRR in SMM relocate.

  @param  SmrrBase           The base address of SMRR.
  @param  SmrrSize           The size of SMRR.
**/
VOID
InitSmrr (
  IN UINT32                SmrrBase,
  IN UINT32                SmrrSize
  )
{
  AsmWriteMsr64 (EFI_MSR_SMRR_PHYS_BASE, SmrrBase| CACHE_WRITE_BACK);
  AsmWriteMsr64 (EFI_MSR_SMRR_PHYS_MASK, (~(SmrrSize - 1) & EFI_MSR_SMRR_MASK)); // Valid bit will be set in ConfigSmrr() at first SMI
}

/**
  Configure SMRR register at each SMM entry.
**/
VOID
ConfigSmrr (
  VOID
  )
{
  UINT64 SmrrMask;

  SmrrMask = AsmReadMsr64 (EFI_MSR_SMRR_PHYS_MASK);
  if ((SmrrMask & EFI_MSR_SMRR_PHYS_MASK_VALID) == 0) {
    AsmWriteMsr64(EFI_MSR_SMRR_PHYS_MASK, SmrrMask | EFI_MSR_SMRR_PHYS_MASK_VALID);
  }
}
//OVERRIDE:End

/**
  Enable the Save Floating Point Pointers feature on every logical processor.
  Refer to SNB/IVB BWG "SMM Handler Considerations".

**/
VOID
EnableSmmSaveControl (
  VOID
)
{
  //
  // MSR_PLATFORM_INFO[16] = 1 indicates the Save Floating Point Pointers feature exists.
  //
  if ((AsmReadMsr64 (MSR_PLATFORM_INFO) & PLATFORM_INFO_SMM_SAVE_CONTROL) != 0) {
    AsmMsrOr64 (EFI_MSR_SMM_SAVE_CONTROL, SAVE_FLOATING_POINT_POINTERS);
  }
}

////////
// Below section is definition for the supported class
////////

/**
  This function will return current CPU_SMM_CLASS accroding to CPUID mapping.

  @return The point to current CPU_SMM_CLASS

**/
CPU_SMM_CLASS *
GetCpuFamily (
  VOID
  )
{
  UINT32         ClassIndex;
  UINT32         Index;
  UINT32         Count;
  CPUID_MAPPING  *CpuMapping;
  UINT32         RegEax;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &RegEax, NULL, NULL, NULL);
  for (ClassIndex = 0; ClassIndex < sizeof(mCpuClasstable)/sizeof(mCpuClasstable[0]); ClassIndex++) {
    CpuMapping = mCpuClasstable[ClassIndex]->MappingTable;
    Count = mCpuClasstable[ClassIndex]->MappingCount;
    for (Index = 0; Index < Count; Index++) {
      if ((CpuMapping[Index].Signature & CpuMapping[Index].Mask) == (RegEax & CpuMapping[Index].Mask)) {
        return mCpuClasstable[ClassIndex];
      }
    }
  }

  // Not found!!! Should not happen
  ASSERT (FALSE);
  return NULL;
}

////////
// Below section is external function
////////
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
  )
{
  UINT64      RetVal; 
  
  RetVal = 0;
  if (IsHaswellEx()) {
    AcquireSpinLock (&mCsrAccessLock);
    switch (RegName) {
    //
    // Server uses CSR
    //
//OVERRIDE:Start
    case SmmRegFeatureControl:
//      RetVal = PciRead32 (HASWELL_SMM_FEATURE_CONTROL_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
      RetVal = PciRead32 (HASWELL_SMM_FEATURE_CONTROL_REG (mNonIioBusNum[CpuIndex]));
      break;
    case SmmRegSmmEnable:
//      RetVal = PciRead32 (HASWELL_SMM_ENABLE1_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
//      RetVal = LShiftU64 (RetVal, 32) | PciRead32 (HASWELL_SMM_ENABLE0_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
      RetVal = PciRead32 (HASWELL_SMM_ENABLE1_REG (mNonIioBusNum[CpuIndex]));
      RetVal = LShiftU64 (RetVal, 32) | PciRead32 (HASWELL_SMM_ENABLE0_REG (mNonIioBusNum[CpuIndex]));
      break;
    case SmmRegSmmDelayed:
//      RetVal = PciRead32 (HASWELL_SMM_DELAYED1_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
//      RetVal = LShiftU64 (RetVal, 32) | PciRead32 (HASWELL_SMM_DELAYED0_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
      RetVal = PciRead32 (HASWELL_SMM_DELAYED1_REG (mNonIioBusNum[CpuIndex]));
      RetVal = LShiftU64 (RetVal, 32) | PciRead32 (HASWELL_SMM_DELAYED0_REG (mNonIioBusNum[CpuIndex]));
      break;
    case SmmRegSmmBlocked:
//      RetVal = PciRead32 (HASWELL_SMM_BLOCKED1_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
//      RetVal = LShiftU64 (RetVal, 32) | PciRead32 (HASWELL_SMM_BLOCKED0_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]));
      RetVal = PciRead32 (HASWELL_SMM_BLOCKED1_REG (mNonIioBusNum[CpuIndex]));
      RetVal = LShiftU64 (RetVal, 32) | PciRead32 (HASWELL_SMM_BLOCKED0_REG (mNonIioBusNum[CpuIndex]));
      break;
//OVERRIDE:End
    default:
      ASSERT (FALSE);
    }
    ReleaseSpinLock (&mCsrAccessLock);
  } else {
    switch (RegName) {
    //
    // Client uses MSR
    //
    case  SmmRegFeatureControl:
      RetVal = AsmReadMsr64 (EFI_MSR_HASWELL_SMM_FEATURE_CONTROL);
      break;
    case  SmmRegSmmEnable:
      RetVal = AsmReadMsr64 (EFI_MSR_HASWELL_SMM_ENABLE);
      break;
    case  SmmRegSmmDelayed:
      RetVal = AsmReadMsr64 (EFI_MSR_HASWELL_SMM_DELAYED);
      break;
    case  SmmRegSmmBlocked:
      RetVal = AsmReadMsr64 (EFI_MSR_HASWELL_SMM_BLOCKED);
      break;
    default:
      ASSERT (FALSE);
    }
  }  
  return  RetVal;
}

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
  )
{
  if (IsHaswellEx()) {
    AcquireSpinLock (&mCsrAccessLock);
    switch (RegName) {
    //
    // Server uses CSR
    //
    case SmmRegFeatureControl:
      PciWrite32 (HASWELL_SMM_FEATURE_CONTROL_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]), (UINT32)RegValue);
      break;
    case SmmRegSmmEnable:
      PciWrite32 (HASWELL_SMM_ENABLE1_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]), (UINT32) RShiftU64 (RegValue, 32));
      PciWrite32 (HASWELL_SMM_ENABLE0_REG (gSmmCpuPrivate->NonIioBusNum[CpuIndex]), (UINT32)RegValue);
      break;
    default:
      ASSERT (FALSE);
    }
    ReleaseSpinLock (&mCsrAccessLock);
  } else {
    switch (RegName) {
    //
    // Client uses MSR
    //
    case  SmmRegFeatureControl:
      AsmWriteMsr64 (EFI_MSR_HASWELL_SMM_FEATURE_CONTROL, RegValue);
      break;
    case  SmmRegSmmEnable:
      AsmWriteMsr64 (EFI_MSR_HASWELL_SMM_ENABLE, RegValue);
      break;
    default:
      ASSERT (FALSE);
    }
  }  
}

/**
  This function will return logical processor index in package.
    
  @param[in]  ProcessorNumber        The processor number.
  @param[out] LogProcIndexPackage    The logical processor index.

  @retval EFI_NOT_FOUND Cannot find the specified processor by ProcessorNumber.
  @retval EFI_SUCCESS   Logical processor index return in LogProcIndexPackage.

**/
EFI_STATUS
GetLogProcIndexInPackage (
  IN  UINTN   ProcessorNumber,
  OUT UINT16  *LogProcIndexPackage 
  )
{
  UINT64      ProcessorId;
  UINT32      PackageId;
  UINTN       Index;
  UINT16      LogProcIndex;

  ProcessorId  = gSmmCpuPrivate->ProcessorInfo[ProcessorNumber].ProcessorId;
  if (ProcessorId == INVALID_APIC_ID) {
    return EFI_NOT_FOUND;
  }

  PackageId    = gSmmCpuPrivate->ProcessorInfo[ProcessorNumber].Location.Package;
  LogProcIndex = 0;   
  for (Index = 0; Index < mMaxNumberOfCpus; Index++) {
    if (gSmmCpuPrivate->ProcessorInfo[Index].Location.Package == PackageId) {
      if (gSmmCpuPrivate->ProcessorInfo[Index].ProcessorId < ProcessorId) { 
        //
        // The logical processor number in the same packet
        //
        LogProcIndex++;
      }
    }
  }

  *LogProcIndexPackage = LogProcIndex;
  return  EFI_SUCCESS;
}

/**
  Disable SMRR register when SmmInit set SMM MTRRs.
**/
VOID
DisableSmrr (
  VOID
  )
{
  ASSERT (mThisCpu != NULL);

  switch (mThisCpu->Family) {
  case CpuCore2:
    if (mFeatureContext.SmrrEnabled) {
      Core2DisableSmrr ();
    }
    return ;
  case CpuPentium4:
  case CpuNehalem:
  default:
    return ;
  }
}

/**
  Enable SMRR register when SmmInit restore non SMM MTRRs.
**/
VOID
ReenableSmrr (
  VOID
  )
{
  ASSERT (mThisCpu != NULL);

  switch (mThisCpu->Family) {
  case CpuCore2:
    if (mFeatureContext.SmrrEnabled) {
      Core2EnableSmrr ();
    }
    return ;
  case CpuPentium4:
  case CpuNehalem:
  default:
    return ;
  }
}

/**
  Return if it is needed to configure MTRR to set TSEG cacheability.

  @retval  TRUE  - we need configure MTRR
  @retval  FALSE - we do not need configure MTRR
**/
BOOLEAN
NeedConfigureMtrrs (
  VOID
  )
{
  ASSERT (mThisCpu != NULL);

  switch (mThisCpu->Family) {
  case CpuSilvermont:
  case CpuNehalem:
  case CpuHaswell:
//OVERRIDE:Start
  case CpuValleyView:
  case CpuBroxton:
//OVERRIDE:End
    return FALSE;
  case CpuPentium4:
  case CpuCore2:
  default:
    return TRUE;
  }
}

/**
  Processor specific hook point at each SMM entry.

  @param  CpuIndex    The index of the cpu which need to check.

**/
VOID
SmmRendezvousEntry (
  IN UINTN  CpuIndex
  )
{
  SOCKET_LGA_775_SMM_CPU_STATE *CpuSaveState;
  UINT64                       EventCtlHaltIoMsr;

  ASSERT (mThisCpu != NULL);

  switch (mThisCpu->Family) {
  case CpuSilvermont:
  case CpuNehalem:
    //
    // Configure SMRR/EMRR register at each SMM entry.
    //
    if (mFeatureContext.SmrrEnabled) {
      NehalemConfigSmrr ();
    }
    //
    // T-state fix
    // NHM & SNB BWG: T-state throttling should be disabled while executing the SMM handler.
    //
    gSmmCpuPrivate->TstateMsr[CpuIndex] = AsmReadMsr64 (EFI_MSR_IA32_CLOCK_MODULATION);
    AsmWriteMsr64 (EFI_MSR_IA32_CLOCK_MODULATION, 0);
    return ;
  case CpuHaswell:
    //
    // Configure SMRR/EMRR register at each SMM entry.
    //
    if (mFeatureContext.SmrrEnabled) {
      NehalemConfigSmrr ();
    }
    if (mFeatureContext.Smrr2Enabled) {
      //
      // Note that SMRR2 registers can't be written in the first SMI because
      // MSR Save State feature is not enabled.
      //   
      HaswellInitSmrr2 (mSmrr2Base, mSmrr2Size);
    }

    if (FeaturePcdGet (PcdFrameworkCompatibilitySupport) && mSaveStateInMsr) {
      //
      // Copy SMBASE, SMMRevId, IORestart, AutoHALTRestart values in MSR to CPU Save State in memory
      // to co-work with the SMM thunk driver.
      //
      CpuSaveState = (SOCKET_LGA_775_SMM_CPU_STATE *)gSmmCpuPrivate->CpuSaveState[CpuIndex];
      CpuSaveState->x86.SMBASE = (UINT32)mCpuHotPlugData.SmBase[CpuIndex];
      CpuSaveState->x86.SMMRevId = (UINT32)AsmReadMsr64 (EFI_MSR_HASWELL_SMMREVID);
      EventCtlHaltIoMsr = AsmReadMsr64 (EFI_MSR_HASWELL_EVENT_CTL_HLT_IO);
      CpuSaveState->x86.IORestart = (UINT16)EventCtlHaltIoMsr;
      CpuSaveState->x86.AutoHALTRestart = (UINT16)RShiftU64 (EventCtlHaltIoMsr, 16);
    }
    return ;
//OVERRIDE:Start
//
//
  case CpuBroxton:
// SV:RestrictedBegin
//
#ifdef SV_HOOKS
// Do not config SMRR for SV BIOS
//
    return;
#endif
//
// SV:RestrictedEnd
//

  case CpuValleyView:
    if (mFeatureContext.SmrrEnabled) {
      ConfigSmrr ();
    }
    return;
//OVERRIDE:End
  case CpuPentium4:
  case CpuCore2:
  default:
    return ;
  }
}

/**
  Processor specific hook point at each SMM exit.

  @param  CpuIndex    The index of the cpu which need to check.
**/
VOID
SmmRendezvousExit (
  IN UINTN  CpuIndex
  )
{
  SOCKET_LGA_775_SMM_CPU_STATE *CpuSaveState;
  UINT64                       EventCtlHaltIoMsr;

  ASSERT (mThisCpu != NULL);

  switch (mThisCpu->Family) {
  case CpuSilvermont:
  case CpuNehalem:
    //
    // T-state fix
    // NHM & SNB BWG: T-state throttling should be disabled while executing the SMM handler.
    //
    AsmWriteMsr64 (EFI_MSR_IA32_CLOCK_MODULATION, gSmmCpuPrivate->TstateMsr[CpuIndex]);
    return ;
  case CpuHaswell:
    if (FeaturePcdGet (PcdFrameworkCompatibilitySupport) && mSaveStateInMsr) {
      //
      // Copy SMBASE, IORestart, AutoHALTRestart values in CPU Save State in memory to MSR
      // to co-work with the SMM thunk driver.
      //
      CpuSaveState = (SOCKET_LGA_775_SMM_CPU_STATE *)gSmmCpuPrivate->CpuSaveState[CpuIndex];
      if (CpuSaveState->x86.SMBASE != (UINT32)mCpuHotPlugData.SmBase[CpuIndex]) {
        //
        // Note that if SMBASE is changed, the SMI management is out of control of this SMM CPU driver.
        //
        AsmWriteMsr64 (EFI_MSR_HASWELL_SMBASE, CpuSaveState->x86.SMBASE);
      }
      EventCtlHaltIoMsr = AsmReadMsr64 (EFI_MSR_HASWELL_EVENT_CTL_HLT_IO);
      BitFieldWrite64 (EventCtlHaltIoMsr, 0, 15, CpuSaveState->x86.IORestart);
      BitFieldWrite64 (EventCtlHaltIoMsr, 16, 31, CpuSaveState->x86.AutoHALTRestart );
      AsmWriteMsr64 (EFI_MSR_HASWELL_EVENT_CTL_HLT_IO, EventCtlHaltIoMsr);
    }
    return ;
//OVERRIDE:Start
  case CpuBroxton:
  case CpuValleyView:
//OVERRIDE:End
  case CpuPentium4:
  case CpuCore2:
  default:
    return ;
  }
}

/**
  Initialize SMRR context in SMM Init.
**/
VOID
InitializeSmmMtrrManager (
  VOID
  )
{
  mThisCpu = GetCpuFamily ();
  ASSERT (mThisCpu != NULL);

  switch (mThisCpu->Family) {
  case CpuCore2:
    if (!IsSmrrSupported ()) {
      return ;
    }
    mFeatureContext.SmrrEnabled = Core2IsSmrrEnabled ();
    return ;
  case CpuSilvermont:
  case CpuNehalem:
  case CpuHaswell:
    if (!IsSmrrSupported ()) {
      return ;
    }
    mFeatureContext.SmrrEnabled = TRUE;
    mFeatureContext.EmrrSupported = IsEmrrSupported ();
    if (IsSmrr2Supported () && mSmrr2Size != 0 && mSaveStateInMsr) {
      mFeatureContext.Smrr2Enabled = TRUE;
    } else {
      mFeatureContext.Smrr2Enabled = FALSE;
    }    
    return ;
//OVERRIDE:Start
  case CpuBroxton:
//
// SV:RestrictedBegin
//
#ifdef SV_HOOKS
      return ;
#endif
//
// SV:RestrictedEnd
//
  case CpuValleyView:
    if (!IsSmrrSupported ()) {
      return ;
    }
    mFeatureContext.SmrrEnabled = TRUE;
    return ;
  case CpuPentium4:
//OVERRIDE:End
  default:
    return ;
  }
}

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
  )
{
  IA32_DESCRIPTOR             *ProtModeIdt;
  SMMSEG                      *ThreadSmmSeg;
  IA32_SEGMENT_DESCRIPTOR     *GdtDescriptor;
  UINTN                       GdtEntryIndex;

  ProtModeIdt = (IA32_DESCRIPTOR *)(UINTN)gProtModeIdtr;
  //
  // Fill SMMSEG structure fields
  //
  ThreadSmmSeg = &mSmmProtModeContext[ProcessorNumber].SmmProtectedModeSMMSEG;
  ThreadSmmSeg->GDTRLimit      = (UINT32)(GdtSize - 1);
  ThreadSmmSeg->GDTRBaseOffset = (UINT32)GdtBase - SmBase;
  ThreadSmmSeg->CSSelector     = SMMSEG_PROTECT_MODE_CODE_SEGMENT;
  ThreadSmmSeg->ESPOffset      = StackAddress - SmBase;
  ThreadSmmSeg->IDTRLimit      = ProtModeIdt->Limit;
  ThreadSmmSeg->IDTRBaseOffset = (UINT32)ProtModeIdt->Base - SmBase;
  //
  // Patch 32bit CS/SS segment base to SMBASE, SS = CS + 8 decided by hardware
  //
  GdtEntryIndex = ThreadSmmSeg->CSSelector / sizeof (IA32_SEGMENT_DESCRIPTOR);
  GdtDescriptor = (IA32_SEGMENT_DESCRIPTOR *)GdtBase;
  GdtDescriptor[GdtEntryIndex].Bits.BaseLow      = (UINT16) 0x0000FFFF & SmBase;
  GdtDescriptor[GdtEntryIndex].Bits.BaseMid      = (UINT8) 0x000000FF & (SmBase >> 16);
  GdtDescriptor[GdtEntryIndex].Bits.BaseHigh     = (UINT8) 0x000000FF & (SmBase >> 24);
  GdtDescriptor[GdtEntryIndex + 1].Bits.BaseLow  = (UINT16) 0x0000FFFF & SmBase;
  GdtDescriptor[GdtEntryIndex + 1].Bits.BaseMid  = (UINT8) 0x000000FF & (SmBase >> 16);
  GdtDescriptor[GdtEntryIndex + 1].Bits.BaseHigh = (UINT8) 0x000000FF & (SmBase >> 24);
}


/**
  Enable SMM Protected Mode.

  @param  ProcessorNumber    The processor number.

**/
VOID
EnableSmmProtectedMode (
  IN UINTN   ProcessorNumber
  )
{
  SMMSEG     *ThreadSmmSeg;

  //
  // Check if hardware support SMM PROT MODE feature
  //
  if (!(SMM_PROT_MODE_BASE_BIT & AsmReadMsr64 (EFI_MSR_HASWELL_SMM_MCA_CAP))) {
    mSmmProtectedModeEnable = FALSE;
  }

  //
  // Enable the SMM PROT MODE feature in the SMMSEG structure
  //
  ThreadSmmSeg = &mSmmProtModeContext[ProcessorNumber].SmmProtectedModeSMMSEG;
  ThreadSmmSeg->SmmSegFeatureEnables = (SMMSEG_FEATURE_ENABLE | SMMSEG_FEATURE_CR4_MCE_CTL_ENABLE);

  //
  // SMMSEG should be 256-byte aligned 32-bit address
  //
  ASSERT (((UINT64)(UINTN)ThreadSmmSeg & ~SMM_PROT_MODE_BASE_PADDR_MASK) == 0);
  AsmWriteMsr64 (EFI_MSR_SMM_PROT_MODE_BASE, ((UINT32)(UINTN)ThreadSmmSeg | SMM_PROT_MODE_BASE_ENABLE_BIT));
}

/**
  Initialize SMRR/EMRR/SMBASE/IED/SMM Sync features in SMM Relocate.

  @param  ProcessorNumber    The processor number
  @param  SmrrBase           The base address of SMRR.
  @param  SmrrSize           The size of SMRR.
  @param  SmBase             The SMBASE value.
  @param  IEDBase            The base address of IED. -1 when IED is disabled.
  @param  IsBsp              If this processor treated as BSP.
**/
VOID
SmmInitiFeatures (
  IN UINTN   ProcessorNumber,
  IN UINT32  SmrrBase,
  IN UINT32  SmrrSize,
  IN UINT32  SmBase,
  IN UINT32  IEDBase,
  IN BOOLEAN IsBsp
  )
{
  EFI_STATUS                        Status;
  SOCKET_LGA_775_SMM_CPU_STATE      *CpuState;
  UINT64                            McaCap;
  SMM_CPU_SYNC_FEATURE              *SyncFeature;
  UINT16                            LogProcIndexInPackage;
  UINT32                            CpuBusNum;

  SyncFeature = &(gSmmCpuPrivate->SmmSyncFeature[ProcessorNumber]);
  SyncFeature->TargetedSmiSupported = FALSE;
  SyncFeature->DelayIndicationSupported = FALSE;
  SyncFeature->BlockIndicationSupported = FALSE;
  SyncFeature->HaswellLogProcEnBit = (UINT64)(INT64)(-1);

  mThisCpu = GetCpuFamily ();
  ASSERT (mThisCpu != NULL);

  mSmmInitSaveStateInMsr = FALSE;

  //
  // Configure SMBASE.
  //
  switch (mThisCpu->Family) {
  case CpuHaswell:
    if (IsHaswellEx()) {
      //
      // Get the Non-IIO bus number assigned by BIOS
      //
      CpuBusNum = (UINT32) AsmReadMsr64 (EFI_MSR_CPU_BUS_NO);
      ASSERT ((CpuBusNum & BIT31) != 0);
      gSmmCpuPrivate->NonIioBusNum[ProcessorNumber] = (UINT8) ((CpuBusNum >> 8) & 0xff);
    }
    
    if ((AsmReadMsr64 (EFI_MSR_HASWELL_SMM_MCA_CAP) & SMM_CPU_SVRSTR_BIT) != 0 &&
        (SmmReadReg64 (ProcessorNumber, SmmRegFeatureControl) & SMM_CPU_SAVE_EN_BIT) != 0) {         
      AsmWriteMsr64 (EFI_MSR_HASWELL_SMBASE, SmBase);
      mSmmInitSaveStateInMsr = TRUE;
      break;
    }
    //
    // Fall back to legacy SMBASE setup.
    //
  case CpuPentium4:
  case CpuCore2:
  case CpuSilvermont:
  case CpuNehalem:
//OVERRIDE:Start
  case CpuValleyView:
  case CpuBroxton:
//OVERRIDE:End
    CpuState = (SOCKET_LGA_775_SMM_CPU_STATE *)(UINTN)(SMM_DEFAULT_SMBASE + SMM_CPU_STATE_OFFSET);
    CpuState->x86.SMBASE = SmBase;
    break ;
  default:
    return ;
  }

  switch (mThisCpu->Family) {
  case CpuPentium4:
    Pentium4RelocateIedBase (IEDBase, IsBsp);
    return ;
  case CpuCore2:
    Core2RelocateIedBase (IEDBase, IsBsp);
    if (!IsSmrrSupported ()) {
      return ;
    }
    if (!Core2IsSmrrEnabled ()) {
      return ;
    }
    Core2InitSmrr (SmrrBase, SmrrSize);
    return ;
  case CpuNehalem:
    NehalemRelocateIedBase (IEDBase, IsBsp);
    if (!IsSmrrSupported ()) {
      return ;
    }
    NehalemInitSmrr (SmrrBase, SmrrSize);
    if (IsEmrrSupported ()) {
      NehalemInitEmrr (IEDBase);
    }
    EnableSmmSaveControl ();
    return ;
  case CpuSilvermont:
    if (!IsSmrrSupported ()) {
      return ;
    }
    NehalemInitSmrr (SmrrBase, SmrrSize);
    return ;
  case CpuHaswell:
    HaswellRelocateIedBase (ProcessorNumber, IEDBase, IsBsp);
    if (IsSmrrSupported ()) {
      NehalemInitSmrr (SmrrBase, SmrrSize);
    }
    if (IsEmrrSupported ()) {
      HaswellInitEmrr (IEDBase);
    }
    
    HaswellConfigMsrSaveState (ProcessorNumber);
     
    EnableSmmSaveControl ();

    if (mSmmUseDelayIndication ||
        mSmmUseBlockIndication ||
        mSmmEnableIndication) {
      if (IsHaswellEx()) {
        //
        // Bit [7:0] of this MSR, CURRENT_THREAD_ID, holds an unique number of a thread within
        // the package
        //
        LogProcIndexInPackage = (UINT16) AsmMsrBitFieldRead64 (EFI_MSR_THREAD_ID_INFO, 0, 7);
        ASSERT (LogProcIndexInPackage < SMM_HASWELL_SERVER_LOG_PROC_EN_BIT_LENGTH);
      } else {
        LogProcIndexInPackage = 0;
        Status = GetLogProcIndexInPackage (ProcessorNumber, &LogProcIndexInPackage);
        ASSERT (Status == EFI_SUCCESS);
        ASSERT (LogProcIndexInPackage < SMM_HASWELL_CLIENT_LOG_PROC_EN_BIT_LENGTH);
      }
      SyncFeature->HaswellLogProcEnBit = LShiftU64 (1ull, LogProcIndexInPackage);
    }
    if (mSmmUseDelayIndication) {
      McaCap = AsmReadMsr64(EFI_MSR_HASWELL_SMM_MCA_CAP);
      SyncFeature->DelayIndicationSupported = (BOOLEAN)((McaCap & LONG_FLOW_INDICATION_BIT) != 0);
    }
    if (mSmmUseBlockIndication) {
      SyncFeature->BlockIndicationSupported = TRUE;
    }
    if (mSmmEnableIndication) {
      McaCap = AsmReadMsr64(EFI_MSR_HASWELL_SMM_MCA_CAP);
      SyncFeature->TargetedSmiSupported = (BOOLEAN)((McaCap & TARGETED_SMI_BIT) != 0);
    }
    if (mSmmProtectedModeEnable) {
      EnableSmmProtectedMode (ProcessorNumber);
    }
    return ;
//OVERRIDE:Start
  case CpuBroxton:
//
// SV
//
#ifdef SV_HOOKS
    return;
#else
    if (IsSmrrSupported ()) {
      InitSmrr (SmrrBase, (SmrrSize));
    }
    return;
#endif
//
// SV
//

  case CpuValleyView:
    if (IsSmrrSupported ()) {
      InitSmrr (SmrrBase, SmrrSize);
    }
    EnableSmmSaveControl ();
    return ;
//OVERRIDE:End
  default:
    ASSERT (FALSE);
    return ;
  }
}

/**
  Configure SMM Code Access Check feature on an AP.
  SMM Feature Control MSR will be locked after configuration.

  @param[in,out] Buffer  Pointer to private data buffer.
**/
VOID
EFIAPI
ConfigSmmCodeAccessCheckOnCurrentProcessor (
  IN OUT VOID  *Buffer
  )
{
  UINT64 SmmFeatureControlMsr;
  UINTN  CpuIndex;

  CpuIndex = *(UINTN *)Buffer;

  SmmFeatureControlMsr = SmmReadReg64 (CpuIndex, SmmRegFeatureControl);
  //
  // SV
  //
#ifdef SV_HOOKS
  // For SV BIOS, SMMC Feature Control register cannot be locked and 
  // SMMC Code Access check must be disabled. SV Test content will reconfigure this register
  // else it will fail.
  //
#else
  //
  // The SMM Feature Control MSR is package scope. If lock bit is set, don't set it again.
  //
  if ((SmmFeatureControlMsr & SMM_FEATURE_CONTROL_LOCK_BIT) == 0) {
    if (mSmmCodeAccessCheckEnable) {
      SmmFeatureControlMsr |= (UINT64)(SMM_CODE_CHK_EN_BIT | SMM_FEATURE_CONTROL_LOCK_BIT);
    } else {
      SmmFeatureControlMsr |= (UINT64)SMM_FEATURE_CONTROL_LOCK_BIT;
    }
    SmmWriteReg64 (CpuIndex, SmmRegFeatureControl, SmmFeatureControlMsr);
  }
#endif
  //
  //SV
  //
}

/**
  Configure SMM Code Access Check feature for all processors.
  SMM Feature Control MSR will be locked after configuration.
**/
VOID
ConfigSmmCodeAccessCheck (
  VOID
  )
{
#if (SMM_FEATURE_LOCK_WA == 0)
  UINTN      Index;
  EFI_STATUS Status;

  //
  // SMM Code Access Check feature is supported since Broxton.
  //
  if (mThisCpu->Family == CpuBroxton) {
    if ((AsmReadMsr64 (EFI_MSR_HASWELL_SMM_MCA_CAP) & SMM_CODE_ACCESS_CHK_BIT) == 0) {
      mSmmCodeAccessCheckEnable = FALSE;
      if (!FeaturePcdGet (PcdCpuSmmFeatureControlMsrLock)) {
        return;
      }
    }
    //
    // Enable SMM Code Access Check feature for the BSP.
    //
    Index = gSmmCpuPrivate->SmmCoreEntryContext.CurrentlyExecutingCpu;
    ConfigSmmCodeAccessCheckOnCurrentProcessor (&Index);
    //
    // Enable SMM Code Access Check feature for the APs.
    //
    for (Index = 0; Index < mNumberOfCpus; Index++) {
      if (Index != gSmmCpuPrivate->SmmCoreEntryContext.CurrentlyExecutingCpu) {
        //
        // Don't call gSmst->SmmStartupThisAp() because it may be implemented in a blocking or non-blocking fashion.
        //
        Status = SmmBlockingStartupThisAp (ConfigSmmCodeAccessCheckOnCurrentProcessor, Index, &Index);
        ASSERT_EFI_ERROR (Status);
      }
    }
  }
#endif
}

