/** @file
  This file contains an implementation of the function call interfaces
  required by the main TXT PEIM file. Hopefully, future platform porting
  tasks will be mostly limited to modifying the functions in this file.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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
@todo code cleanup for txt memoryinit overlap
**/

#include <Private/Library/TxtPeiLib.h>
#include <Library/TxtLib.h>


/**
  This routine initializes and collects all PPIs and data required
  by the routines in this file.

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure

  @exception EFI_UNSUPPORTED   - If any of the required PPIs or data are unavailable
  @retval EFI_SUCCESS       - In all cases not listed above
**/
EFI_STATUS
InitializeTxtPeiLib (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  EFI_STATUS Status;

  ///
  /// Make sure our pointers start life as NULL pointers
  ///
  pctx->BiosAcmBase = NULL;
  pctx->ApStartup   = NULL;
  pctx->McuStart    = NULL;

  Status = PeiServicesLocatePpi (
                  &gEfiPciCfg2PpiGuid,
                  0,
                  NULL,
                  (VOID **) &(pctx->PciCfgPpi)
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  /// Find TxtInfoHob by platform code
  ///
  Status = CreateTxtInfoHob (pctx);

  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "TXTPEI: TXT Info Hob not found or TXT is not supported.\n"));
    return EFI_UNSUPPORTED;
  }
  ///
  /// Print out the TxtInfo HOB if TXT_DEBUG_INFO is set
  ///
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtInfoHob passed from platform as:\n"));
  DEBUG ((DEBUG_INFO, "TXTPEI: ChipsetIsTxtCapable = %x\n", pctx->Hob->Data.ChipsetIsTxtCapable));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtMode             = %x\n", pctx->Hob->Data.TxtMode));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtAcheckRequest    = %x\n", pctx->Hob->Data.TxtAcheckRequest));
  DEBUG ((DEBUG_INFO, "TXTPEI: ResetAux            = %x\n", pctx->Hob->Data.ResetAux));
  DEBUG ((DEBUG_INFO, "TXTPEI: AcpiBase            = %x\n", pctx->Hob->Data.AcpiBase));
  DEBUG ((DEBUG_INFO, "TXTPEI: SinitMemorySize     = %x\n", pctx->Hob->Data.SinitMemorySize));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtHeapMemorySize   = %x\n", pctx->Hob->Data.TxtHeapMemorySize));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtDprMemoryBase    = %x\n", pctx->Hob->Data.TxtDprMemoryBase));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtDprMemorySize    = %x\n", pctx->Hob->Data.TxtDprMemorySize));
  DEBUG ((DEBUG_INFO, "TXTPEI: BiosAcmBase         = %x\n", pctx->Hob->Data.BiosAcmBase));
  DEBUG ((DEBUG_INFO, "TXTPEI: BiosAcmSize         = %x\n", pctx->Hob->Data.BiosAcmSize));
  DEBUG ((DEBUG_INFO, "TXTPEI: McuUpdateDataAddr   = %x\n", pctx->Hob->Data.McuUpdateDataAddr));
  DEBUG ((DEBUG_INFO, "TXTPEI: SinitAcmBase        = %x\n", pctx->Hob->Data.SinitAcmBase));
  DEBUG ((DEBUG_INFO, "TXTPEI: SinitAcmSize        = %x\n", pctx->Hob->Data.SinitAcmSize));
  DEBUG ((DEBUG_INFO, "TXTPEI: TgaSize             = %x\n", pctx->Hob->Data.TgaSize));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtLcpPdBase        = %x\n", pctx->Hob->Data.TxtLcpPdBase));
  DEBUG ((DEBUG_INFO, "TXTPEI: TxtLcpPdSize        = %x\n", pctx->Hob->Data.TxtLcpPdSize));
  DEBUG ((DEBUG_INFO, "TXTPEI: Flags               = %x\n", pctx->Hob->Data.Flags));

  ///
  /// Check if platform specify BIOS ACM addrss by itself, BIOS ACM address must be 4K alignment in FLASH address space
  ///
  if (pctx->Hob->Data.BiosAcmBase != 0) {
    DEBUG ((DEBUG_INFO, "TXTPEI: Customized BIOS ACM location at %x\n", pctx->Hob->Data.BiosAcmBase));
    ///
    /// Check BIOS ACM is 4K alignment or not
    ///
    if ((pctx->Hob->Data.BiosAcmBase & 0xFFF) != 0) {
      DEBUG ((DEBUG_ERROR, "TXTPEI: Customized BIOS ACM is not 4K aligned, force TxtMode=0 and unloaded!!\n"));
      pctx->Hob->Data.TxtMode = 0;
      return EFI_UNSUPPORTED;
    }
    ///
    /// Get BIOS ACM base from TxtInfoHob provided by platform code
    ///
    pctx->BiosAcmBase = (ACM_HEADER *) (UINT32) pctx->Hob->Data.BiosAcmBase;
  } else {
    ///
    /// Get BIOS ACM by seaching PEI firmware volume
    ///
    Status = FindBiosAcmInVolume (pctx, (UINT32 **) &pctx->BiosAcmBase);

    ///
    /// BIOS ACM not found, disable TXT and return EFI_UNLOAD_IMAGE
    ///
    if (((pctx->BiosAcmBase) == 0) || (Status == EFI_NOT_FOUND)) {
      DEBUG ((DEBUG_ERROR, "TXTPEI: BIOS ACM not found, force TxtMode=0 and unloaded!\n"));
      pctx->Hob->Data.TxtMode = 0;
      return EFI_UNSUPPORTED;
    }

    DEBUG ((DEBUG_INFO, "TXTPEI: Found BIOS ACM location at %x\n", pctx->BiosAcmBase));

    ///
    /// Check BIOS ACM is 4K alignment, if not disable TXT and return EFI_UNLOAD_IMAGE
    ///
    if (((UINT32) pctx->BiosAcmBase & 0xFFF) != 0) {
      DEBUG ((DEBUG_ERROR, "TXTPEI: BIOS ACM is not 4K aligned, force TxtMode=0 and unloaded!!\n"));
      pctx->Hob->Data.TxtMode = 0;
      return EFI_UNSUPPORTED;
    }
  }
  ///
  /// Check ACM is matched to chipset or not, if not, disable TXT and return EFI_UNLOAD_IMAGE
  ///
  if (!CheckTxtAcmMatch (pctx, pctx->BiosAcmBase)) {
    DEBUG ((DEBUG_ERROR, "TXTPEI: BIOS ACM is not matched to chipset!! Force TxtMode=0 and unloaded!!\n"));
    pctx->Hob->Data.TxtMode = 0;
    return EFI_UNSUPPORTED;
  }

  pctx->BiosAcmSize = (pctx->BiosAcmBase->Size) << 2;

  Status              = FindApStartupInVolume (pctx, (UINT32 **) &(pctx->ApStartup));

  if (Status == EFI_NOT_FOUND) {
    DEBUG ((DEBUG_ERROR, "TXTPEI: AP Startup code not found.\n"));
    pctx->Hob->Data.TxtMode = 0;
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "TXTPEI: Found AP init code at %x\n", pctx->ApStartup));

  ///
  /// Check if platform specify MCU addrss by itself
  ///
  if (pctx->Hob->Data.McuUpdateDataAddr != 0) {
    DEBUG ((DEBUG_INFO, "Customized MCU location at %x\n", pctx->Hob->Data.McuUpdateDataAddr));
    pctx->McuStart = (UINT32 *) (UINTN) pctx->Hob->Data.McuUpdateDataAddr;
  } else {
    ///
    /// Find microcode update by searching PEI FV
    ///
    Status = FindMcuInVolume (pctx, (UINT32 **) &(pctx->McuStart));

    if (Status == EFI_NOT_FOUND) {
      DEBUG ((DEBUG_ERROR, "TXTPEI: MCU not found.\n"));
      pctx->Hob->Data.TxtMode = 0;
      return EFI_UNSUPPORTED;
    }

    DEBUG ((DEBUG_INFO, "TXTPEI: Found MCU at %x\n", pctx->McuStart));
  }
  ///
  /// Initialize local APIC
  ///
  if ((((AsmReadMsr64 (MSR_IA32_APIC_BASE)) & (BIT11 + BIT10)) == BIT11 + BIT10)) {
    AsmWriteMsr64 (EFI_MSR_EXT_XAPIC_SVR, 0x1FF);
  } else {
    ///
    /// Determine address of Local APIC
    ///
    pctx->Ia32ApicBase  = ((UINT32) AsmReadMsr64 (MSR_IA32_APIC_BASE)) & BASE_ADDR_MASK;
    *(UINT32 *) (pctx->Ia32ApicBase + APIC_SPURIOUS_VECTOR_REGISTER) = 0x1FF;
  }
  ///
  /// Initialize TxtInfoHob fields
  ///
  pctx->Hob->Data.BiosAcmBase       = (UINTN) pctx->BiosAcmBase;
  pctx->Hob->Data.BiosAcmSize       = (UINTN) pctx->BiosAcmSize;
  pctx->Hob->Data.McuUpdateDataAddr = (UINTN) pctx->McuStart;

  ///
  /// Make sure none of our pointers are still NULL
  ///
  if (!(pctx->BiosAcmBase && pctx->ApStartup && pctx->McuStart)) {
    pctx->Hob->Data.TxtMode = 0;
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Determines whether or not the current processor is TXT Capable.

  @retval TRUE          - If the current processor supports TXT
  @retval FALSE         - If the current processor does not support TXT
**/
BOOLEAN
IsTxtProcessor (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegs;

  AsmCpuid (1, &CpuidRegs.RegEax, &CpuidRegs.RegEbx, &CpuidRegs.RegEcx, &CpuidRegs.RegEdx);

  return (CpuidRegs.RegEcx & B_CPUID_VERSION_INFO_ECX_SME) ? TRUE : FALSE;
}

/**
  Determines whether or not the current chipset is TXT Capable.

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure

  @retval TRUE          - If the current chipset supports TXT
  @retval FALSE         - If the current chipset doesn't supports TXT
**/
BOOLEAN
IsTxtChipset (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  BOOLEAN TxtCapable;
  UINT32  Data32;

  TxtCapable  = FALSE;
  Data32      = CheckSmxCapabilities();

  if ((Data32 & BIT0) != 0) {
    TxtCapable = TRUE;
    DEBUG ((DEBUG_INFO, "Platform/PCH - TXT supported\n"));
  } else {
    TxtCapable = FALSE;
    DEBUG ((DEBUG_INFO, "Platform/PCH - TXT not supported!!!\n"));
  }

  return TxtCapable;
}

/**
  Determines whether TXT is enabled by platform setting

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure

  @retval TRUE          - If TXT is enabled by platform setting
  @retval FALSE         - If TXT is disabled by platform setting
**/
BOOLEAN
IsTxtEnabled (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  if (pctx->Hob == 0) {
    return FALSE;
  }

  return (BOOLEAN) (pctx->Hob->Data.TxtMode);
}

/**
  Determines whether Alias Check Request is enabled by platform setting

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure

  @retval TRUE          - If ACHECK REQUEST is enabled by platform setting
  @retval FALSE         - If ACHECK REQUEST is disabled by platform setting
**/
BOOLEAN
IsAcheckRequested (
  TXT_PEI_LIB_CONTEXT *pctx
  )
{
  if (pctx->Hob == 0) {
    return FALSE;
  }

  return (BOOLEAN) (pctx->Hob->Data.TxtAcheckRequest);
}

/**
  Determines ACM is matched to chipset or not

  @param[in] pctx         - Point to TXT_PEI_LIB_CONTEXT structure
  @param[in] BiosAcmBase  - A pointer to BIOS ACM location

  @retval TRUE          - BIOS ACM is matched to chipset
  @retval FALSE         - BIOS ACM is NOT matched to chipset
**/
BOOLEAN
CheckTxtAcmMatch (
  IN TXT_PEI_LIB_CONTEXT *pctx,
  IN ACM_HEADER          *BiosAcmBase
  )
{
  BOOLEAN ChipsetIsProduction;
  BOOLEAN BiosAcmIsProduction;

  if (BiosAcmBase == NULL) {
    return FALSE;

  }
  ///
  /// Initializing ChipsetIsProduction default value
  ///
  ChipsetIsProduction = (*(UINT32 *) (TXT_PUBLIC_BASE + 0x200) & BIT31) ? TRUE : FALSE;

  ///
  /// Check ACM is production or not
  ///
  BiosAcmIsProduction = (BiosAcmBase->ModuleID & BIT31) ? FALSE : TRUE;

  return ChipsetIsProduction == BiosAcmIsProduction;
}

/**
  Create TXT Info HOB

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure

  @retval EFI_SUCCESS   - If TXT Info Hob is found
  @retval EFI_NOT_FOUND - If TXT Info Hob is not found
  @retval EFI_UNSUPPORTED - If the platform is not TXT capable.
**/
EFI_STATUS
CreateTxtInfoHob (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  EFI_STATUS                  Status;
  SI_CPU_POLICY_PPI           *SiCpuPolicy;
  TXT_INFO_HOB                *TxtInfoHob;

  TxtInfoHob = NULL;
  //
  // Get TxtInfoHob if it is already present, then this is the reinitalize, just reload the hob and exit
  //
  TxtInfoHob = (TXT_INFO_HOB *)GetFirstGuidHob (&gTxtInfoHobGuid);
  if (TxtInfoHob != NULL) {
    pctx->Hob = TxtInfoHob;
    return EFI_SUCCESS;
  }
  ///
  /// Locate Cpu policy ppi to initialized.
  ///
  Status = PeiServicesLocatePpi (
                  &gSiCpuPolicyPpiGuid,
                  0,
                  NULL,
                  (VOID **) &SiCpuPolicy
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Create hob for storing TXT data
  ///
  Status = PeiServicesCreateHob (
                  EFI_HOB_TYPE_GUID_EXTENSION,
                  sizeof (TXT_INFO_HOB),
                  (VOID **) &TxtInfoHob
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  pctx->Hob = TxtInfoHob;
  TxtInfoHob->EfiHobGuidType.Name = gTxtInfoHobGuid;
  ///
  /// Initiate Txt Info Hob
  ///
  ZeroMem (&(TxtInfoHob->Data), sizeof (TXT_INFO_DATA));

  TxtInfoHob->Data.ChipsetIsTxtCapable = IsTxtChipset (pctx);
  if (SiCpuPolicy->CpuConfigPreMem.Txt == 1) {
    TxtInfoHob->Data.TxtMode = 1;
  }

  if (SiCpuPolicy->CpuConfigPreMem.TxtConfig != NULL) {
    PchAcpiBaseGet (&(TxtInfoHob->Data.AcpiBase));

    TxtInfoHob->Data.TxtAcheckRequest   = (BOOLEAN) SiCpuPolicy->CpuConfigPreMem.TxtConfig->TxtAcheckRequest;
    TxtInfoHob->Data.ResetAux           = (BOOLEAN) SiCpuPolicy->CpuConfigPreMem.TxtConfig->ResetAux;
    TxtInfoHob->Data.SinitMemorySize    = SiCpuPolicy->CpuConfigPreMem.TxtConfig->SinitMemorySize;
    TxtInfoHob->Data.TxtHeapMemorySize  = SiCpuPolicy->CpuConfigPreMem.TxtConfig->TxtHeapMemorySize;
    TxtInfoHob->Data.TxtDprMemoryBase   = SiCpuPolicy->CpuConfigPreMem.TxtConfig->TxtDprMemoryBase;
    TxtInfoHob->Data.TxtDprMemorySize   = SiCpuPolicy->CpuConfigPreMem.TxtConfig->TxtDprMemorySize;
    TxtInfoHob->Data.BiosAcmBase        = SiCpuPolicy->CpuConfigPreMem.TxtConfig->BiosAcmBase;
    TxtInfoHob->Data.BiosAcmSize        = SiCpuPolicy->CpuConfigPreMem.TxtConfig->BiosAcmSize;
    TxtInfoHob->Data.McuUpdateDataAddr  = SiCpuPolicy->CpuConfigPreMem.TxtConfig->McuUpdateDataAddr;
    TxtInfoHob->Data.TgaSize            = SiCpuPolicy->CpuConfigPreMem.TxtConfig->TgaSize;
    TxtInfoHob->Data.TxtLcpPdBase       = SiCpuPolicy->CpuConfigPreMem.TxtConfig->TxtLcpPdBase;
    TxtInfoHob->Data.TxtLcpPdSize       = SiCpuPolicy->CpuConfigPreMem.TxtConfig->TxtLcpPdSize;
  } else {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

BOOLEAN
IsSptPtt(
  )
{
  UINT32 RegRead;

  RegRead = MmioRead32 (TXT_PUBLIC_BASE + TXT_PTT_FTIF_OFF);
  DEBUG ((DEBUG_INFO, "IsSptPtt: LT FTIF = %x\n", RegRead));
  RegRead = RegRead & TXT_PTT_PRESENT;
  if (RegRead == TXT_PTT_PRESENT) {
    DEBUG ((DEBUG_INFO, "IsSptPtt: PTT cycle\n"));
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Determines whether or not the platform has executed an TXT launch by
  examining the TPM Establishment bit.

  @param[in] pctx      - A pointer to an initialized TXT PEI Context data structure

  @retval TRUE          - If the TPM establishment bit is asserted.
  @retval FALSE         - If the TPM establishment bit is unasserted.
**/
BOOLEAN
TxtIsEstablishmentBitAsserted (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  UINT8  Access;
  UINT16 TimeOutCount;

  if (IsSptPtt()) {
    return FALSE;
  }
  ///
  /// Set TPM.ACCESS polling timeout about 750ms
  ///
  TimeOutCount = TPM_TIME_OUT;
  do {
    ///
    /// Read TPM status register
    ///

      Access = MmioRead8 (TPM_STATUS_REG_ADDRESS);

    ///
    /// if TPM.Access == 0xFF, TPM is not present
    ///
    if (Access == 0xFF) {
      return FALSE;
    }
    ///
    /// Check tpmRegValidSts bit before checking establishment bit
    ///
    if ((Access & 0x80) != 0x80) {
      ///
      /// Delay 1ms
      ///
      MicroSecondDelay (1000);
    } else {
      ///
      /// tpmRegValidSts set, we can check establishment bit now.
      ///
      break;
    }

    TimeOutCount--;
  } while (TimeOutCount != 0);
  ///
  /// if tpmRegValidSts is not set, TPM is not usable
  ///
  if ((Access & 0x80) != 0x80) {
    DEBUG ((DEBUG_ERROR, "TXTPEI: TPM Valid Status is not set!! TPM.ACCESS=%x\n", Access));
    ASSERT (TRUE);
    CpuDeadLoop ();
  }
  ///
  /// The bit we're interested in uses negative logic:
  /// If bit 0 == 1 then return False
  /// Else return True
  ///
  return (Access & 0x1) ? FALSE : TRUE;
}

/**
  Determines whether or not the platform has encountered an error during
  a sleep or power-off state.

  @retval TRUE          - If the TXT_WAKE_ERROR bit is asserted.
  @retval FALSE         - If the TXT_WAKE_ERROR bit is unasserted.
**/
BOOLEAN
IsTxtWakeError (
  VOID
  )
{
  UINT8 Ests;

  ///
  /// Read TXT.ESTS register
  ///
  Ests = MmioRead8 (TXT_PUBLIC_BASE + TXT_ERROR_STATUS_REG_OFF);

  DEBUG ((DEBUG_INFO, "TXTPEI: TXT.ESTS=%x\n", Ests));

  return (Ests & (0x1 << 6)) ? TRUE : FALSE;
}

/**
  Determines whether or not the platform memory has been locked

  @retval TRUE          - If memroy is locked
  @retval FALSE         - If memory is unlocked
**/
BOOLEAN
IsMemoryLocked (
  VOID
  )
{
  UINT32 CpuMemLockStatus;

  ///
  /// Check status register for now.
  ///
  CpuMemLockStatus = MmioRead32 (TXT_PUBLIC_BASE + TXT_E2STS_REG_OFF);

  DEBUG ((DEBUG_INFO, "TXTPEI: CPU_UNCORE_MEMLOCK_STATUS=%x\n", CpuMemLockStatus));
  ///
  /// if BLOCK_MEM_STS (BIT2) is set to 1, memory is in unlock.
  ///
  return (CpuMemLockStatus & TXT_BLOCK_MEM_STS) ? TRUE : FALSE;
}

/**
  Determines whether or not POISON bit is set in status register

  @retval TRUE          - If the TXT_WAKE_ERROR bit is asserted.
  @retval FALSE         - If the TXT_WAKE_ERROR bit is unasserted.
**/
BOOLEAN
IsTxtResetSet (
  VOID
  )
{
  UINT8 Ests;
  ///
  /// Read TXT.ESTS register
  ///
  Ests = MmioRead8 (TXT_PUBLIC_BASE + TXT_ERROR_STATUS_REG_OFF);

  return (Ests & (0x1 << 0)) ? TRUE : FALSE;
}

/**
  Determines whether or not SECRETS.STS bit is set in E2STS status register

  @retval TRUE          - If the LT.SECRETS.STS bit is asserted.
  @retval FALSE         - If the LT.SECRETS.STS bit is unasserted.
**/
BOOLEAN
IsTxtSecretsSet (
  VOID
  )
{
  UINT8 E2sts;

  ///
  /// Read TXT.E2STS register
  ///
  E2sts = MmioRead8 (TXT_PUBLIC_BASE + TXT_E2STS_REG_OFF);

  return (E2sts & TXT_SECRETS_STS) ? TRUE : FALSE;
}

/**
  Determines presence of TPM in system

  @param[in] pctx           - Point to TXT_PEI_LIB_CONTEXT structure
  @param[in] TxtPeiCtx      - A pointer to an initialized TXT PEI Context data structure

  @retval EFI_SUCCESS          - If the TPM is present.
  @retval EFI_NOT_FOUND         - If the TPM is not present.
**/
EFI_STATUS
IsTpmPresent (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  EFI_STATUS Status;
  VOID       *TpmInitialize;

  ///
  /// Locate TPM Initialized Ppi to determine TPM is present and initialized properly.
  ///
  Status = PeiServicesLocatePpi (
                  &gPeiTpmInitializedPpiGuid,
                  0,
                  NULL,
                  (VOID **) &TpmInitialize
                  );
  if (EFI_ERROR (Status)) {
    ///
    /// TPM initiated failed
    ///
    pctx->Hob->Data.Flags |= TPM_INIT_FAILED;
  }

  return Status;
}

/**
  Clear Sleep Type register.

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure

  @retval EFI_SUCCESS   - Always
**/
EFI_STATUS
ClearSlpTyp (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  UINT32 Pm1Addr;
  UINT32 Pm1Value;

  ///
  /// Make address for PM1_CNT
  ///
  Pm1Addr = ((UINT32) pctx->Hob->Data.AcpiBase) + 4;

  ///
  /// Read 32-bits from PM1_CNT
  ///
  Pm1Value = IoRead32 ((UINTN) Pm1Addr);

  ///
  /// Clear SLP_TYP bits 10-12
  ///
  Pm1Value = Pm1Value & 0xffffe3ff;
  IoWrite32 (
          (UINTN) Pm1Addr,
          (UINT32) (Pm1Value)
          );

  return EFI_SUCCESS;
}

/**
  Searches PEI firemare volume (FV_BB) for file containig BIOS ACM.

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure
  @param[in] BiosAcBase    - A pointer to pointer to variable to hold found address

  @retval EFI_SUCCESS   - If address has been found
  @retval EFI_NOT_FOUND     - If address has not been found
**/
EFI_STATUS
FindBiosAcmInVolume (
  IN TXT_PEI_LIB_CONTEXT *pctx,
  OUT UINT32             **BiosAcBase
  )
{
  EFI_STATUS Status;

  Status = FindModuleInFlash (pctx, &gTxtBiosAcmPeiFileGuid, BiosAcBase);

  ///
  /// If BIOS ACM is not found - don't hang system. Assume that TXT
  /// must be disabled.
  ///
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  ASSERT (((((UINT32) (*BiosAcBase)) & 0xFFF) == 0) ? TRUE : FALSE);

  return EFI_SUCCESS;

}

/**
  Searches PEI firmware volume (FV_BB) for file containig AP Startup code

  @param[in] pctx           - A pointer to an initialized TXT PEI Context data structure
  @param[in] ApStartupBase  - A pointer to pointer to variable to hold address

  @retval EFI_SUCCESS   - If address has been found
  @retval EFI_NOT_FOUND     - If address has not been found
**/
EFI_STATUS
FindApStartupInVolume (
  IN TXT_PEI_LIB_CONTEXT *pctx,
  OUT UINT32             **ApStartupBase
  )
{
  EFI_STATUS Status;
  EFI_GUID   PeiBiosApStartupFileGuid;
  BOOLEAN    GoodPlacement;

  PeiBiosApStartupFileGuid  = gTxtApStartupPeiFileGuid;
  Status                    = FindModuleInFlash (pctx, &PeiBiosApStartupFileGuid, ApStartupBase);
  ///
  /// If AP Startup code is not found - don't hang system. Assume that TXT
  /// must be disabled.
  ///
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  ASSERT (((((UINT32) (*ApStartupBase)) & 0xFFF) == 0) ? TRUE : FALSE);

  GoodPlacement = (BOOLEAN) (((UINT32) (*ApStartupBase) >= 0xFFFE0000) && ((UINT32) (*ApStartupBase) <= 0xFFFFF000));

  ASSERT (GoodPlacement);

  return EFI_SUCCESS;
}

/**
  Searches PEI firmware volume (FV_BB) for offset of currently loaded MCU patch

  @param[in] pctx          - A pointer to an initialized TXT PEI Context data structure
  @param[in] McuBase       - A pointer to pointer to variable to hold found offset
  @param[in] address.

  @retval EFI_SUCCESS   - If address has been found
  @retval EFI_NOT_FOUND     - If address has not been found
**/
EFI_STATUS
FindMcuInVolume (
  IN TXT_PEI_LIB_CONTEXT *pctx,
  OUT UINT32             **McuBase
  )
{
  EFI_STATUS               Status;
  EFI_GUID                 MicrocodeGuid;
  MSR_REGISTER             Reg;
  UINT32                   PlatformId;
  UINT32                   McuRevision;
  EFI_CPUID_REGISTER       CpuidRegs;
  EFI_CPU_MICROCODE_HEADER *McuAddr;
  UINT8                    *i;
  UINT8                    *j;
  UINT8                    *b;
  UINT32                   c;

  McuAddr       = NULL;
  McuRevision   = 0;
  MicrocodeGuid = gCpuMicrocodeFileGuid;
  Status        = FindModuleInFlash (pctx, &MicrocodeGuid, (UINT32 **) &McuAddr);
  ///
  /// If MCU update is not found - don't hang system. Assume that TXT
  /// must be disabled.
  ///
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }
  ///
  /// MCU base address has been found. Find exact address of MCU
  /// loaded in BSP
  ///
  Reg.Qword   = AsmReadMsr64 (MSR_IA32_PLATFORM_ID);
  PlatformId  = (Reg.Dwords.High >> (N_PLATFORM_ID_SHIFT - 32)) & B_PLATFORM_ID_MASK;

  AsmWriteMsr64 (MSR_IA32_BIOS_SIGN_ID, 0);

  AsmCpuid (CPUID_VERSION_INFO, &CpuidRegs.RegEax, &CpuidRegs.RegEbx, &CpuidRegs.RegEcx, &CpuidRegs.RegEdx);

  Reg.Qword   = AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID);

  McuRevision = Reg.Dwords.High;

  ///
  /// if BSP MCU is not loaded before. Return EFI_NOT_FOUND
  /// This is pre-requisit.
  ///
  if (McuRevision == 0) {
    DEBUG ((DEBUG_ERROR, "TXTPEI: BSP microcode is not loaded!! TXT aborted!\n"));
    return EFI_NOT_FOUND;
  }

  for (i = (UINT8 *) McuAddr; i < ((UINT8 *) McuAddr) + 0x10000; i = i + 0x400) {
    if (((EFI_CPU_MICROCODE_HEADER *) i)->UpdateRevision != McuRevision) {
      continue;
    }

    if (!(((EFI_CPU_MICROCODE_HEADER *) i)->ProcessorFlags & (UINT32) LShiftU64 (1, PlatformId))) {
      continue;
    }

    if ((((EFI_CPU_MICROCODE_HEADER *) i)->ProcessorId) == CpuidRegs.RegEax) {
      *McuBase = (UINT32 *) i;
      return EFI_SUCCESS;
    }

    if ((((EFI_CPU_MICROCODE_HEADER *) i)->DataSize) == 0) {
      continue;
    }

    if ((((EFI_CPU_MICROCODE_HEADER *) i)->DataSize + sizeof (EFI_CPU_MICROCODE_HEADER)) >= (((EFI_CPU_MICROCODE_HEADER *) i)->TotalSize)) {
      continue;
    }
    ///
    /// Extended signature table exists.
    ///
    b = i + (((EFI_CPU_MICROCODE_HEADER *) i)->DataSize);
    ///
    /// Base of EST table
    ///
    c = ((EFI_CPU_MICROCODE_EXTENDED_TABLE_HEADER *) b)->ExtendedSignatureCount;
    ///
    /// Count of entries
    ///
    b += sizeof (EFI_CPU_MICROCODE_EXTENDED_TABLE_HEADER);
    ///
    /// Base of PSS table entries
    ///
    for (j = b; j < b + (c * sizeof (PSS)); j = j + sizeof (PSS)) {
      if ((((PSS *) j)->signature) == CpuidRegs.RegEax) {
        *McuBase = (UINT32 *) i;
        return EFI_SUCCESS;
      }
    }

    continue;
  }

  return EFI_NOT_FOUND;
}

/**
  Searches PEI firmware volume (FV_BB) for the file with specified GUID through pGuid

  @param[in] pctx         - A pointer to an initialized TXT PEI Context data structure
  @param[in] Guid         - A pointer GUID
  @param[in] Module       - A pointer to pointer to variable to hold address

  @retval EFI_SUCCESS   - If address has been found
  @retval EFI ERROR     - If address has not been found
**/
EFI_STATUS
FindModuleInFlash (
  IN TXT_PEI_LIB_CONTEXT *pctx,
  IN EFI_GUID            *Guid,
  OUT UINT32             **Module
  )
{
  EFI_FIRMWARE_VOLUME_HEADER *FvHeader;
  EFI_FFS_FILE_HEADER        *FfsFile;
  EFI_STATUS                 Status;
  UINTN                      Instance;

  Instance = BFV;

  while (TRUE) {
    Status = PeiServicesFfsFindNextVolume (Instance, (EFI_PEI_FV_HANDLE *)&FvHeader);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    FfsFile = NULL;
    ///
    /// Start new search in volume
    ///
    while (TRUE) {
      Status = PeiServicesFfsFindNextFile (EFI_FV_FILETYPE_RAW, FvHeader, (VOID **)&FfsFile);
      if (Status == EFI_NOT_FOUND) {
        break;
      }

      if (CompareGuid (&(FfsFile->Name), Guid)) {
        *Module = (UINT32 *) ((UINT8 *) FfsFile + sizeof (EFI_FFS_FILE_HEADER));
        return EFI_SUCCESS;
      }
    }

    Instance += 1;
  }
}

/**
  Initializes values passed to AP

  @param[in] pctx      - A pointer to an initialized TXT PEI Context data structure
**/
VOID
PrepareApParams (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  UINT32 TxtPublicSpace;

  TxtPublicSpace  = TXT_PUBLIC_BASE;

  *(UINT32 *) (TxtPublicSpace + BIOACM_ADDR)    = (UINT32) (pctx->BiosAcmBase);

  *(UINT32 *) (TxtPublicSpace + MCU_BASE_ADDR)  = (UINT32) (pctx->McuStart);

  *(UINT32 *) (TxtPublicSpace + APINIT_ADDR)    = (UINT32) (pctx->ApStartup);

  *(UINT32 *) (TxtPublicSpace + SEMAPHORE)      = 0;

}

/**
  Invokes the SCLEAN/ACHECK function from the TXT BIOS ACM.
  1. Clearing of sleep type is necessary because SCLEAN/ACHECK destroys memory
  context, so S3 after it is run and system is reset is impossible. We
  do it here since there is no any indication that can sustain reset
  for any other module to do it on our behalf.
  2. APs are initialized before calling of SCLEAN/ACHECK
  3. SCLEAN/ACHECK function is invoked.

  @param[in] pctx      - A pointer to an initialized TXT PEI Context data structure
  @param[in] func      - Acm function to be launch

  @retval EFI_SUCCESS   - Always.
**/
EFI_STATUS
DoAcmLaunch (
  IN TXT_PEI_LIB_CONTEXT *pctx,
  IN UINT8               func
  )
{

  EFI_STATUS                    Status;
  SI_CPU_POLICY_PPI             *SiCpuPolicy;

  Status = PeiServicesLocatePpi (
                  &gSiCpuPolicyPpiGuid,
                  0,
                  NULL,
                  (VOID **) &SiCpuPolicy
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  ClearSlpTyp (pctx);

  PrepareApParams (pctx);

  if (SiCpuPolicy->CpuConfigPreMem.SkipStopPbet != TRUE) {
    ///
    /// Disable PBET before send IPI to APs
    ///
    StopPbeTimer ();
  }

  if (func == TXT_LAUNCH_SCLEAN) {
    LaunchBiosAcmSclean ();
  } else if (func == TXT_LAUNCH_ACHECK) {
    LaunchBiosAcmAcheck ();
  }

  return EFI_SUCCESS;
}

/**
  Unlock memory when security is set and TxT is not enabled

  @param[in] pctx      - A pointer to an initialized TXT PEI Context data structure

  @retval EFI_SUCCESS     - Complete memory unlock
  @exception EFI_UNSUPPORTED - CPU doesn't support TxT.
**/
EFI_STATUS
TxtUnlockMemory (
  IN TXT_PEI_LIB_CONTEXT *pctx
  )
{
  BOOLEAN EstablishmentBitAsserted;

  EstablishmentBitAsserted = TxtIsEstablishmentBitAsserted (pctx);

  ///
  /// Need to read FED40000 before unlocking memory
  ///
  if (!EstablishmentBitAsserted) {
    DEBUG ((DEBUG_INFO, "TXTPEI::Unlock memory\n"));
    AsmWriteMsr64 (MSR_LT_UNLOCK_MEMORY, 0);
  } else {
    ///
    /// Lunch SCLEAN if wake error bit is set.
    ///
    if (IsTxtWakeError ()) {
      ///
      /// If TXTRESET is set , we must clean TXTRESET bit otherwise SCLEAN
      /// will fail
      ///
      if (IsTxtResetSet ()) {
        DoGlobalReset (); /// @todo use pch rest calls
      }

      if ((pctx->BiosAcmBase == 0) || (pctx->ApStartup == 0) || (pctx->McuStart == 0)) {
        return EFI_UNSUPPORTED;
      }
      ///
      /// Setup and Launch SCLEAN
      ///
      DEBUG ((DEBUG_INFO, "TXTPEI::Entering SCLEAN to unlock memory\n"));
      DoAcmLaunch (pctx, TXT_LAUNCH_SCLEAN);

      ///
      /// Reset platform - performed by DoSclean, should not return to execute the following dead looping
      ///
      CpuDeadLoop ();
    } else {
      return EFI_UNSUPPORTED;
    }
  }

  return EFI_SUCCESS;
}
