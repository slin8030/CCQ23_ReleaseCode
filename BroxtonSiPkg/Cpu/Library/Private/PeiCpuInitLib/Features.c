/** @file
  CPU feature control module

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

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
#include <Library/ReportStatusCodeLib.h>

#include "Features.h"
#include "MachineCheck.h"
#include <Library/CpuPlatformLib.h>
#include <Private/Library/CpuPowerOnConfigLib.h>
#include <Library/CpuPolicyLib.h>
#include <Library/PreSiliconLib.h>
#include <Library/PostCodeLib.h>
#ifdef FSP_FLAG
#include <Library/FspCommonLib.h>
#include <FspsUpd.h>
#endif

/**
  Provide access to the CPU misc enables MSR

  @param[in] Enable  - Enable or Disable Misc Features
  @param[in] BitMask - The register bit offset of MSR MSR_IA32_MISC_ENABLE
**/
VOID
CpuMiscEnable (
  BOOLEAN Enable,
  UINT64  BitMask
  )
{
  UINT64 MsrValue;

  MsrValue = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  if (Enable) {
    MsrValue |= BitMask;
  } else {
    MsrValue &= ~BitMask;
  }

  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, MsrValue);
}

///
/// DCA contains processor code and chipset code
/// CPU driver has the following assumption on the initialization flow
/// 1. Chipset pre-initialization should detect DCA support per chipset capability after SiCpuPolicy
/// 2. If not support, it should update SiCpuPolicy DCA to disable state
/// 3. If support, it should enable the DCA related registers
/// 4. CPU initialization for DCA (CPU may change SiCpuPolicy DCA states per CPU capability)
/// 5. Normal chipset driver (IOH) should look at SiCpuPolicy DCA policy again in PCI enumeration
/// 6. Chipset enable or disable DCA according to SiCpuPolicy DCA state
///
/**
  Detect DCA supported or not

  @retval DCA_SUPPORT if supported or 0 if not supported
**/
UINTN
IsDcaSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegisters;
  UINTN                Support;

  Support = 0;
  AsmCpuid (
          CPUID_VERSION_INFO,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  if ((CpuidRegisters.RegEcx & B_CPUID_VERSION_INFO_ECX_DCA) != 0) {
    ///
    /// Execute Disable Bit feature is not supported on this processor.
    ///
    Support = DCA_SUPPORT;
  }
  return Support;
}

/**
  Detect HT supported or not

  @retval HT_SUPPORT if supported or 0 if not supported
**/
UINTN
IsHTSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegisters;
  UINTN              Support;

  Support = 0;

  AsmCpuidEx (
          CPUID_CORE_TOPOLOGY,
          0,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  if ((CpuidRegisters.RegEbx & 0x00FF) == 2) {
    Support = HT_SUPPORT;
  }
  return Support;

}

/**
  Detect if AES supported or not

  @retval AES_SUPPORT if supported or 0 if not supported
**/
UINTN
IsAesSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegisters;
  UINTN              Support;

  Support = 0;
  AsmCpuid (
          CPUID_VERSION_INFO,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  if ((CpuidRegisters.RegEcx & B_CPUID_VERSION_INFO_ECX_AES) != 0) {
    Support = AES_SUPPORT;
  }
  return Support;
}

/**
  Detect if XD supported or not

  @retval XD_SUPPORT if supported or 0 if not supported
**/
UINTN
IsXdSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegisters;
  UINTN              Support;

  Support = 0;
  AsmCpuid (
          CPUID_EXTENDED_FUNCTION,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  if (CpuidRegisters.RegEax > CPUID_EXTENDED_FUNCTION) {
    AsmCpuid (
            CPUID_EXTENDED_CPU_SIG,
            &CpuidRegisters.RegEax,
            &CpuidRegisters.RegEbx,
            &CpuidRegisters.RegEcx,
            &CpuidRegisters.RegEdx
            );
    if ((CpuidRegisters.RegEdx & B_CPUID_VERSION_INFO_EDX_XD) != 0) {
      ///
      /// Execute Disable Bit feature is supported on this processor.
      ///
      Support = XD_SUPPORT;
    }
  }
  return Support;
}

/**
  Program XD if supported or disable it if not supported

  @param[in] Support  - bitmap that indicate XD supported or not
**/
VOID
ProgramXd (
  IN UINTN Support
  )
{
  BOOLEAN XdSupport;

  XdSupport = (BOOLEAN)((Support & XD_SUPPORT) == XD_SUPPORT);
  ///
  /// MSR MISC_ENABLE[34] has negative logic: 0 - XD Enabled, 1 - XD Disabled
  ///
  CpuMiscEnable (!XdSupport, B_MSR_IA32_MISC_ENABLE_XD);
}

/**
  Check on the processor if VMX/TXT is supported.

  @retval VMX_SUPPORT and TXT_SUPPORT if supported or 0 if not supported
**/
UINTN
IsVmxSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuIdRegister;
  UINTN              Support;

  Support = 0;

  ///
  /// Get CPUID to check if the processor supports Vanderpool Technology.
  ///
  AsmCpuid (
          CPUID_VERSION_INFO,
          &CpuIdRegister.RegEax,
          &CpuIdRegister.RegEbx,
          &CpuIdRegister.RegEcx,
          &CpuIdRegister.RegEdx
          );
  if ((CpuIdRegister.RegEcx & B_CPUID_VERSION_INFO_ECX_VME) != 0) {
    ///
    /// VT is supported.
    ///
    Support |= VMX_SUPPORT;
  }
  if ((CpuIdRegister.RegEcx & B_CPUID_VERSION_INFO_ECX_SME) != 0) {
    ///
    /// TXT is supported.
    ///
    Support |= TXT_SUPPORT;
  }
  return Support;
}

/**
  Enable VMX/TXT on the processor.

  @param[in] Support  - To enable or disable VMX/TXT feature.
**/
VOID
EnableDisableVmx (
  IN UINTN Support
  )
{
  UINT64 Ia32FeatCtrl;
  UINT64 NewFeatCtrl;
  MP_SYSTEM_DATA *MpSystemData;

  MpSystemData = GetMpSystemData ();

  Ia32FeatCtrl = AsmReadMsr64 (MSR_IA32_FEATURE_CONTROL);
  Ia32FeatCtrl &= ~((UINT64) OPTION_FEATURE_RESERVED_MASK);

  NewFeatCtrl = Ia32FeatCtrl;

  ///
  /// If Vmx is Disabled, Enable it.
  ///
  if (MpSystemData->CommonFeatures & VMX_SUPPORT) {
    if (Support & VMX_SUPPORT) {
      NewFeatCtrl |= B_MSR_IA32_FEATURE_CONTROL_EVT;
    } else {
      NewFeatCtrl &= ~B_MSR_IA32_FEATURE_CONTROL_EVT;
    }
  }

  ///
  /// Check the Feature Lock Bit.
  /// If it is already set, which indicates we are executing POST
  /// due to a warm RESET (i.e., PWRGOOD was not de-asserted).
  ///
  if ((Ia32FeatCtrl & B_MSR_IA32_FEATURE_CONTROL_LOCK) == 0) {
    AsmWriteMsr64WithScript (MSR_IA32_FEATURE_CONTROL, NewFeatCtrl);
  } else {
    ///
    /// if Lock bit is set
    ///
    NewFeatCtrl &= ~(B_MSR_IA32_FEATURE_CONTROL_LOCK);
    WriteMsr64ToScript (MSR_IA32_FEATURE_CONTROL, NewFeatCtrl);
  }
}

/**
  Enable / Disable AES on the processor.

  @param[in] Support  - To enable or disable AES feature.
**/
VOID
EnableDisableAes (
  IN UINTN Support
  )
{
  UINT64 MsrValue;
  MP_SYSTEM_DATA *MpSystemData;

  MpSystemData = GetMpSystemData ();

  if (!(MpSystemData->CommonFeatures & AES_SUPPORT) || (IsSecondaryThread ())) {
    return;
  }

  ///
  /// The processor was manufactured with AES-NI feature
  ///
  MsrValue = AsmReadMsr64 (MSR_IA32_FEATURE_CONFIG);

  ///
  /// Check the Feature Lock Bit.
  /// If it is already set, which indicates we are executing POST
  /// due to a warm RESET (i.e., PWRGOOD was not de-asserted).
  ///
  if ((MsrValue & B_IA32_FEATURE_CONFIG_LOCK) == 0) {
    if (Support & AES_SUPPORT) {
      ///
      /// Enabled AES, writes of 00b, 01b pr 10b to the MSR will result in AES enable.
      /// Should lock this MSR always, so write 01b to the MSR.
      ///
      MsrValue &= ~B_IA32_FEATURE_CONFIG_AES_DIS;
      MsrValue |= B_IA32_FEATURE_CONFIG_LOCK;
    } else {
      ///
      /// To disable AES, system BIOS must write 11b to this MSR.
      ///
      MsrValue |= (B_IA32_FEATURE_CONFIG_AES_DIS + B_IA32_FEATURE_CONFIG_LOCK);
    }
    AsmWriteMsr64WithScript (MSR_IA32_FEATURE_CONFIG, MsrValue);
  }
  return;
}

/**
  Check on the processor if Debug Interface is supported

  @retval Value of DEBUG_SUPPORT and DEBUG_LOCK_SUPPORT
**/
UINTN
IsDebugInterfaceSupported (
  VOID
  )
{
  UINTN              Support;
  EFI_CPUID_REGISTER CpuIdRegister;

  Support = 0;

  ///
  /// Debug interface is supported if CPUID (EAX=1): ECX[11] = 1,
  ///
  AsmCpuid (
          CPUID_VERSION_INFO,
          &CpuIdRegister.RegEax,
          &CpuIdRegister.RegEbx,
          &CpuIdRegister.RegEcx,
          &CpuIdRegister.RegEdx
          );

  if (CpuIdRegister.RegEcx & BIT11) {
    Support |= DEBUG_SUPPORT;
    Support |= DEBUG_LOCK_SUPPORT;
  }

  return Support;
}

/**
  Enable/Disable Debug Interfaces in the processor.

  @param[in] Support  - To enable or disable Debug Interface feature.
**/
VOID
EnableDisableDebugInterface (
  IN UINTN Support
  )
{
  UINT64  Ia32DebugInterface;
  MP_SYSTEM_DATA *MpSystemData;

  MpSystemData = GetMpSystemData ();

  ///
  /// IA32_DEBUG_INTERFACE_MSR scope is "Package", program on BSP only
  ///
  if (!(MpSystemData->CommonFeatures & DEBUG_SUPPORT) || (IsBsp() == FALSE)) {
    return;
  }

  ///
  /// Check if the processor supports debug interface
  ///
  if (IsDebugInterfaceSupported()) {
    Ia32DebugInterface = AsmReadMsr64 (MSR_IA32_DEBUG_INTERFACE);
    if (!(Ia32DebugInterface & B_DEBUG_INTERFACE_LOCK)) {
      if (Support & DEBUG_SUPPORT) {
        ///
        /// Enable Debug Interface (MSR 0xC80.Bit0 = 1)
        ///
        Ia32DebugInterface |= B_DEBUG_INTERFACE_ENABLE;
      } else {
        ///
        /// Disable Debug Interface (MSR 0xC80.Bit0 = 0)
        ///
        Ia32DebugInterface &= ~B_DEBUG_INTERFACE_ENABLE;
      }
      if (Support & DEBUG_LOCK_SUPPORT) {
        Ia32DebugInterface |= B_DEBUG_INTERFACE_LOCK;
      }
      AsmWriteMsr64WithScript (MSR_IA32_DEBUG_INTERFACE, Ia32DebugInterface);
    }
  }
  return;
}

/**
  Lock VMX/TXT feature bits on the processor.
  Set "CFG Lock" (MSR 0E2h Bit[15]
**/

VOID
LockFeatureBit (
  VOID
  )
{
  UINT64 Ia32FeatCtrl;

  ///
  /// MSR 3Ah for VMX/TXT Lock
  ///
  Ia32FeatCtrl = AsmReadMsr64 (MSR_IA32_FEATURE_CONTROL);
  Ia32FeatCtrl &= ~((UINT64) OPTION_FEATURE_RESERVED_MASK);

  if ((Ia32FeatCtrl & B_MSR_IA32_FEATURE_CONTROL_LOCK) == 0) {
    ///
    /// Set Feature Lock bits.
    ///
    Ia32FeatCtrl |= B_MSR_IA32_FEATURE_CONTROL_LOCK;
    AsmWriteMsr64WithScript (MSR_IA32_FEATURE_CONTROL, Ia32FeatCtrl);
  } else {
    WriteMsr64ToScript (MSR_IA32_FEATURE_CONTROL, Ia32FeatCtrl);
  }

  return;
}

/**
  Detect if X2APIC supported or not

  @retval X2APIC_SUPPORT if supported or 0 if not supported
**/
UINTN
IsX2apicSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegisters;
  UINTN              Support;
  UINT64             MsrValue;

  Support   = 0;

  MsrValue  = AsmReadMsr64 (MSR_IA32_APIC_BASE);
  AsmCpuid (
          CPUID_VERSION_INFO,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  if ((CpuidRegisters.RegEcx & B_CPUID_VERSION_INFO_ECX_X2APIC) != 0) {
    if (MsrValue & B_MSR_IA32_APIC_BASE_G_XAPIC) {
      ///
      /// X2APIC Mode feature is supported on this processor.
      ///
      Support = X2APIC_SUPPORT;
    }
  }
  return Support;
}

/**
  Detect if Processor Trace Feature is supported or not

  @retval PROC_TRACE_SUPPORT if supported or 0 if not supported
**/
UINTN
IsProcTraceSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegisters;
  UINTN              Support;

  Support = 0;
  AsmCpuidEx (
          CPUID_FUNCTION_7,
          0,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  if ((CpuidRegisters.RegEbx & B_CPUID_FUNCTION7_EBX_RTIT_SUPPORT) != 0) {
    Support = PROC_TRACE_SUPPORT;
  }
  return Support;
};


/**
  Detect which PT output schemes are supported.

  @retval BIT0 Indicates support for ToPA,
  @retval BIT2 Indicates support for Single Range Output.

**/
UINT8
ProcTraceSchemesSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER  CpuidRegisters;
  UINT8               Support;

  Support = 0;
  AsmCpuidEx (
          CPUID_FUNCTION_20,
          0,
          &CpuidRegisters.RegEax,
          &CpuidRegisters.RegEbx,
          &CpuidRegisters.RegEcx,
          &CpuidRegisters.RegEdx
          );
  Support = (UINT8) CpuidRegisters.RegEcx;

  return Support;
};


/**
  Initialize Processor Trace Feature

  @param[in] Support    Bitmap that indicate HT supported or not
**/
VOID
InitializeProcTrace (
  IN UINTN Support
  )
{
  UINT64                        MsrValue;
  UINT32                        MemRegionSize;
  UINTN                         Pages;
  UINTN                         Alignment;
  UINTN                         MemRegionBaseAddr = 0;
  STATIC UINTN                  *ThreadMemRegionTable;
  STATIC UINTN                  ThreadMemRegionIndex = 0;
  STATIC UINTN                  AllocatedThreads = 0;
  UINTN                         Index;
  STATIC UINT8                  Lock = 0;
  UINTN                         TopaTableBaseAddr;
  UINTN                         AlignedAddress;
  STATIC UINTN                  *TopaMemArray;
  STATIC UINTN                  TopaMemIndex = 0;
  STATIC UINT8                  TopaLock = 0;
  PROC_TRACE_TOPA_TABLE         *TopaTable;
  MP_SYSTEM_DATA                *MpSystemData;
  UINT8                         SupportedSchemes;
  BOOLEAN                       IsTopaSupported;
  BOOLEAN                       IsSingleRangeSupported;
  BOOLEAN                       IsBspInt;

  MpSystemData = GetMpSystemData ();
  IsBspInt     = IsBsp();

  if (IsBspInt) {
    DEBUG ((DEBUG_INFO, "Initialize Processor Trace\n"));
  }

  ///
  /// Check if Processor Trace is supported, or ProcTraceMemorySize option is enabled (0xFF means disable by user)
  ///
  if ((!IsProcTraceSupported ()) || (MpSystemData->ProcTraceMemSize >= EnumProcTraceMemDisable)) {
    return;
  }

  SupportedSchemes = ProcTraceSchemesSupported();
  IsTopaSupported = (SupportedSchemes & B_CPUID_FUNCTION20_ECX_TOPA_SUPPORT) ? TRUE : FALSE;
  IsSingleRangeSupported = (SupportedSchemes & B_CPUID_FUNCTION20_ECX_SINGLE_RANGE_SUPPORT) ? TRUE : FALSE;

  if (!(IsTopaSupported || IsSingleRangeSupported)) {
    return;
  }
  ///
  /// Refer to PROC_TRACE_MEM_SIZE Table for Size Encoding
  ///
  MemRegionSize = (UINT32) (1 << (MpSystemData->ProcTraceMemSize + 12));
  if (IsBspInt) {
    DEBUG ((DEBUG_INFO, "ProcTrace: MemSize requested: 0x%X \n", MemRegionSize));
  }

  //
  // Clear MSR_IA32_RTIT_CTL[0] and IA32_RTIT_STS only if MSR_IA32_RTIT_CTL[0]==1b
  //
  MsrValue = AsmReadMsr64 (MSR_IA32_RTIT_CTL);
  if (MsrValue & B_RTIT_CTL_TRACE_ENABLE) {
    ///
    /// Clear bit 0 in MSR IA32_RTIT_CTL (570)
    ///
    MsrValue &= ~ B_RTIT_CTL_TRACE_ENABLE;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_CTL, MsrValue);

    ///
    /// Clear MSR IA32_RTIT_STS (571h) to all zeros
    ///
    MsrValue = AsmReadMsr64 (MSR_IA32_RTIT_STATUS);
    MsrValue &= 0x0;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_STATUS, MsrValue);
  }

  if (IsBspInt) {
    /**
       Let BSP allocate and create the necessary memory region (Aligned to the size of
       the memory region from setup option(ProcTraceMemSize) which is an integral multiple of 4kB)
       for the all the enabled threads for storing Processor Trace debug data. Then Configure the trace
       address base in MSR, IA32_RTIT_OUTPUT_BASE (560h) bits 47:12. Note that all regions must be
       aligned based on their size, not just 4K. Thus a 2M region must have bits 20:12 clear.
    **/
    ThreadMemRegionTable = (UINTN *) AllocatePool (MpSystemData->MaximumCpusForThisSystem * sizeof(UINTN *));
    if (ThreadMemRegionTable == NULL) {
      DEBUG ((DEBUG_ERROR, "Allocate ProcTrace ThreadMemRegionTable Failed\n"));
      return;
    }

    for (Index=0; Index < MpSystemData->MaximumCpusForThisSystem; Index++, AllocatedThreads++) {
      Pages = EFI_SIZE_TO_PAGES (MemRegionSize);
      Alignment = MemRegionSize;
      AlignedAddress = (UINTN) AllocateAlignedReservedPages (Pages, Alignment);
      if (AlignedAddress == 0) {
        DEBUG ((DEBUG_ERROR, "ProcTrace: Out of mem, allocated only for %d threads\n", AllocatedThreads));
        if (Index == 0) {
           //
           // Could not allocate for BSP even
           //
           FreePool ((VOID *) ThreadMemRegionTable);
           ThreadMemRegionTable = NULL;
           return;
        }
        break;
      }

      ThreadMemRegionTable[Index] = AlignedAddress;
      DEBUG ((DEBUG_INFO, "ProcTrace: PT MemRegionBaseAddr(aligned) for thread %d: 0x%llX \n", Index, (UINT64) ThreadMemRegionTable[Index]));
    }

    DEBUG ((DEBUG_INFO, "ProcTrace: Allocated PT mem for %d thread \n", AllocatedThreads));
    //
    // BSP gets the first block
    //
    MemRegionBaseAddr = ThreadMemRegionTable[0];
  } else {
    //
    // Each AP gets different index
    //
    AsmAcquireMPLock (&Lock);
    //
    // Count for currently executing AP.
    //
    ThreadMemRegionIndex++;
    if (ThreadMemRegionIndex < AllocatedThreads) {
      MemRegionBaseAddr = ThreadMemRegionTable[ThreadMemRegionIndex];
    } else {
      AsmReleaseMPLock (&Lock);
      return;
    }
    AsmReleaseMPLock (&Lock);
  }

  ///
  /// Check Processor Trace output scheme: Single Range output or ToPA table
  ///
  //
  //  Single Range output scheme
  //
  if (IsSingleRangeSupported && (MpSystemData->ProcTraceOutputScheme == 0)) {
    if (IsBspInt) {
      DEBUG ((DEBUG_INFO, "ProcTrace: Enabling Single Range Output scheme \n"));
    }

    //
    // Clear MSR IA32_RTIT_CTL (0x570) ToPA (Bit 8)
    //
    MsrValue = AsmReadMsr64 (MSR_IA32_RTIT_CTL);
    MsrValue &= ~BIT8;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_CTL, MsrValue);

    //
    // Program MSR IA32_RTIT_OUTPUT_BASE (0x560) bits[47:12] with the allocated Memory Region
    //
    MsrValue = (UINT64) MemRegionBaseAddr;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_OUTPUT_BASE, MsrValue);

    //
    // Program the Mask bits for the Memory Region to MSR IA32_RTIT_OUTPUT_MASK_PTRS (561h)
    //
    MsrValue = (UINT64) MemRegionSize - 1;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_OUTPUT_MASK, MsrValue);
  }

  //
  //  ToPA(Table of physical address) scheme
  //
  if (IsTopaSupported && (MpSystemData->ProcTraceOutputScheme == 1)) {
    /**
      Create ToPA structure aligned at 4KB for each logical thread
      with at least 2 entries by 8 bytes size each. The first entry
      should have the trace output base address in bits 47:12, 6:9
      for Size, bits 4,2 and 0 must be cleared. The second entry
      should have the base address of the table location in bits
      47:12, bits 4 and 2 must be cleared and bit 0 must be set.
    **/
    if (IsBspInt) {
      DEBUG ((DEBUG_INFO, "ProcTrace: Enabling ToPA scheme \n"));
      /**
         Let BSP allocate ToPA table mem for all threads
      **/
      TopaMemArray = (UINTN *) AllocatePool (AllocatedThreads * sizeof(UINTN *));
      if (TopaMemArray == NULL) {
        DEBUG ((DEBUG_ERROR, "ProcTrace: Allocate mem for ToPA Failed\n"));
        return;
      }

      for (Index=0; Index < AllocatedThreads; Index++) {
        Pages = EFI_SIZE_TO_PAGES (sizeof (PROC_TRACE_TOPA_TABLE));
        Alignment = 0x1000;
        AlignedAddress = (UINTN) AllocateAlignedReservedPages (Pages, Alignment);
        if (AlignedAddress == 0) {
          if ( Index < AllocatedThreads) {
            AllocatedThreads = Index;
          }
          DEBUG ((DEBUG_ERROR, "ProcTrace:  Out of mem, allocating ToPA mem only for %d threads\n", AllocatedThreads));
          if (Index == 0) {
           //
           // Could not allocate for BSP
           //
           FreePool ((VOID *) TopaMemArray);
           TopaMemArray = NULL;
           return;
          }
          break;
        }

        TopaMemArray[Index] = AlignedAddress;
        DEBUG ((DEBUG_INFO, "ProcTrace: Topa table address(aligned) for thread %d is 0x%llX \n", Index,  (UINT64) TopaMemArray[Index]));
      }

      DEBUG ((DEBUG_INFO, "ProcTrace: Allocated ToPA mem for %d thread \n", AllocatedThreads));
      //
      // BSP gets the first block
      //
      TopaTableBaseAddr = TopaMemArray[0];
    } else {
      AsmAcquireMPLock (&TopaLock);
      //
      // Count for currently executing AP.
      //
      TopaMemIndex++;
      if (TopaMemIndex < AllocatedThreads) {
        TopaTableBaseAddr = TopaMemArray[TopaMemIndex];
      } else {
        AsmReleaseMPLock (&TopaLock);
        return;
      }
      AsmReleaseMPLock (&TopaLock);
    }

    TopaTable = (PROC_TRACE_TOPA_TABLE *) TopaTableBaseAddr;
    TopaTable->TopaEntry[0] = (UINT64) (MemRegionBaseAddr | ((MpSystemData->ProcTraceMemSize) << 6)) & ~BIT0;
    TopaTable->TopaEntry[1] = (UINT64) TopaTableBaseAddr | BIT0;

    //
    // Program the MSR IA32_RTIT_OUTPUT_BASE (0x560) bits[47:12] with ToPA base
    //
    MsrValue = (UINT64) TopaTableBaseAddr;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_OUTPUT_BASE, MsrValue);
    //
    // Set the MSR IA32_RTIT_OUTPUT_MASK (0x561) bits[63:7] to 0
    //
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_OUTPUT_MASK, 0x7f);
    //
    // Enable ToPA output scheme by enabling MSR IA32_RTIT_CTL (0x570) ToPA (Bit 8)
    //
    MsrValue = AsmReadMsr64 (MSR_IA32_RTIT_CTL);
    MsrValue |= BIT8;
    AsmWriteMsr64WithScript (MSR_IA32_RTIT_CTL, MsrValue);
  }

  ///
  /// Enable the Processor Trace feature from MSR IA32_RTIT_CTL (570h)
  ///
  MsrValue = AsmReadMsr64 (MSR_IA32_RTIT_CTL);
  MsrValue |= BIT0 + BIT2 + BIT3 + BIT13;
  if (!(MpSystemData->ProcTraceEnable)) {
      MsrValue &= ~BIT0;
  }
  AsmWriteMsr64WithScript (MSR_IA32_RTIT_CTL, MsrValue);

}

/**
  Create feature control structure which will be used to program each feature on each core.
**/
VOID
InitializeFeaturePerSetup (
  VOID
  )
{
  MP_SYSTEM_DATA *MpSystemData;

  MpSystemData = GetMpSystemData ();
  PostCode (0xC0E);

  MpSystemData->FeatureLock    = VacantFlag;
  MpSystemData->CommonFeatures = (UINTN) -1;
  MpSystemData->SetupFeatures  = (UINTN) -1;

  if (!MpSystemData->VmxEnable) {
    MpSystemData->SetupFeatures &= ~VMX_SUPPORT;
  }
  if (!MpSystemData->AesEnable) {
    MpSystemData->SetupFeatures &= ~AES_SUPPORT;
  }
  if (!MpSystemData->DebugInterfaceEnable) {
    MpSystemData->SetupFeatures &= ~DEBUG_SUPPORT;
  }
  if (!MpSystemData->DebugInterfaceLockEnable) {
    MpSystemData->SetupFeatures &= ~DEBUG_LOCK_SUPPORT;
  }
  if (!MpSystemData->ProcTraceEnable) {
    MpSystemData->SetupFeatures &= ~PROC_TRACE_SUPPORT;
  }
  PostCode (0xC12);
}

/**
  Detect each processor feature and log all supported features
**/
VOID
EFIAPI
CollectProcessorFeature (
  VOID
  )
{
  UINTN Support;
  MP_SYSTEM_DATA *MpSystemData;
  MpSystemData = GetMpSystemData ();

  Support = 0;
  Support |= IsXdSupported ();
  Support |= IsVmxSupported ();
  Support |= IsDcaSupported ();
  Support |= IsAesSupported ();
  Support |= IsX2apicSupported ();
  Support |= IsHTSupported ();
  Support |= IsDebugInterfaceSupported ();
  Support |= IsProcTraceSupported ();

  AsmAcquireMPLock (&MpSystemData->FeatureLock);
  MpSystemData->CommonFeatures &= Support;
  AsmReleaseMPLock (&MpSystemData->FeatureLock);
  return;
}

/**
  Program all processor features basing on desired settings
**/
VOID
EFIAPI
ProgramProcessorFeature (
  VOID
  )
{
  UINTN  Supported;
  BOOLEAN IsBspInt;
  MP_SYSTEM_DATA *MpSystemData;

  MpSystemData = GetMpSystemData ();
  IsBspInt = IsBsp();
  Supported = MpSystemData->CommonFeatures & MpSystemData->SetupFeatures;

  ///
  /// Configure features such as Xd, Vmx, Smx, XAPIC, AES, DebugInterface, Processor Trace feature, Debug Interface
  ///
  ProgramXd (Supported);
  EnableDisableVmx (Supported);
  EnableDisableAes (Supported);
  InitializeProcTrace (Supported);
  EnableDisableDebugInterface (Supported);

  ///
  /// Program XApic register
  ///
  if (IsBspInt) {
    DEBUG ((DEBUG_INFO, "Program xAPIC\n"));
  }
  ProgramXApic (IsBspInt);

  ///
  /// Initialize MonitorMWait register
  ///
  if (IsBspInt) {
    DEBUG ((DEBUG_INFO, "Initialize Monitor Wait register\n"));
  }
  CpuMiscEnable (MpSystemData->MonitorMwaitEnable, B_MSR_IA32_MISC_ENABLE_MONITOR);

  ///
  /// Initialize Machine Check registers
  ///
  if (IsBspInt) {
    DEBUG ((DEBUG_INFO, "Initialize Machine check registers\n"));
  }
  InitializeMachineCheckRegisters (NULL, (BOOLEAN) MpSystemData->MachineCheckEnable);
  return;
}

/**
  Program CPUID Limit before booting to OS
**/
VOID
EFIAPI
ProgramCpuidLimit (
   VOID
  )
{
  UINT64 MsrValue;

  ///
  /// Finally record the MISC MSR into CPU S3 script table
  /// to avoid access for multiple times
  ///
  MsrValue = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  AsmWriteMsr64WithScript (MSR_IA32_MISC_ENABLE, MsrValue);

  return;
}

/**
  Initialize prefetcher settings

  @param[in] MlcStreamerprefecterEnabled - Enable/Disable MLC streamer prefetcher
  @param[in] MlcSpatialPrefetcherEnabled - Enable/Disable MLC spatial prefetcher
**/
VOID
InitializeProcessorsPrefetcher (
  IN UINTN MlcStreamerprefecterEnabled,
  IN UINTN MlcSpatialPrefetcherEnabled
  )
{
  UINT64 MsrValue;
  MsrValue = AsmReadMsr64 (MISC_FEATURE_CONTROL);

  if (MlcStreamerprefecterEnabled == CPU_FEATURE_DISABLE) {
    MsrValue |= B_MISC_FEATURE_CONTROL_MLC_STRP;
  }

  if (MlcSpatialPrefetcherEnabled == CPU_FEATURE_DISABLE) {
    MsrValue |= B_MISC_FEATURE_CONTROL_MLC_SPAP;
  }

  if ((MsrValue & (B_MISC_FEATURE_CONTROL_MLC_STRP | B_MISC_FEATURE_CONTROL_MLC_SPAP)) != 0) {
    AsmWriteMsr64 (MISC_FEATURE_CONTROL, MsrValue);
  }

  return;
}

/**
  Initialize processor features, performance and power management features, BIOS GUARD, and Overclocking etc features before RESET_CPL at post-memory phase.

  @param[in] SiCpuPolicyPpi The Cpu Policy PPI instance

  @retval EFI_SUCCESS     The driver installed/initialized correctly.
**/
EFI_STATUS
CpuInit (
  IN  SI_CPU_POLICY_PPI     *SiCpuPolicyPpi
  )
{
  EFI_STATUS                  Status;
  EFI_BOOT_MODE               BootMode;
#ifdef FSP_FLAG
  FSPS_UPD                   *FspsUpd;
#endif

  DEBUG((DEBUG_INFO, "CpuInit Start \n"));
  PostCode (0xC30);
#ifdef FSP_FLAG
  FspsUpd = (FSPS_UPD *)GetFspSiliconInitUpdDataPointer ();
#endif
  Status = PeiServicesGetBootMode (&BootMode);
  DEBUG ((DEBUG_INFO, "CpuInit: BootMode = %X\n", BootMode));
  if ((Status == EFI_SUCCESS) && (BootMode == BOOT_ON_S3_RESUME)) {
#ifdef FSP_FLAG
    if (!(FspsUpd->FspsConfig.InitS3Cpu)) {
#endif
    return EFI_SUCCESS;
#ifdef FSP_FLAG
    }
#endif
  }
  DEBUG ((DEBUG_INFO, "Cpu Initialization in PostMem start\n"));

  if (PLATFORM_ID == VALUE_REAL_PLATFORM){
    //
    // Check and Update the Active Cores
    //
    DEBUG((DEBUG_INFO, "Check and Update the Active Cores Start \n"));
    CheckAndResetActiveCoresAfterCpl();
  }

#ifdef FSP_FLAG
  if (FspsUpd->FspsConfig.SkipMpInit == 0) {
#endif
    ///
    /// Initialize feature control structure
    ///
    DEBUG((DEBUG_INFO, "Initialize feature control Start \n"));
    InitializeFeaturePerSetup ();
    ///
    /// Detect and log all processor supported features
    ///
    DEBUG((DEBUG_INFO, "CollectProcessorFeature Start \n"));
    PostCode (0xC37);
    CollectProcessorFeature ();
    StartupAllAps ((EFI_AP_PROCEDURE) CollectProcessorFeature, NULL);

    DEBUG((DEBUG_INFO, "ProgramProcessorFeature Start \n"));
    PostCode (0xC38);
    ProgramProcessorFeature ();
    StartupAllAps ((EFI_AP_PROCEDURE) ProgramProcessorFeature, NULL);


    DEBUG((DEBUG_INFO, "Lock Feature bit Start \n"));
    PostCode (0xC3A);
    LockFeatureBit();
    StartupAllAps ((EFI_AP_PROCEDURE) LockFeatureBit, NULL);

    ///
    /// Initialize CPU Data Hob
    ///
    InitializeCpuDataHob ();
#ifdef FSP_FLAG    
  }
#endif

  DEBUG((DEBUG_INFO, "CpuInit Done \n"));
  PostCode (0xC3F);
  return EFI_SUCCESS;
}