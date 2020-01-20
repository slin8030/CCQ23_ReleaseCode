/** @file
  EFI 2.0 PEIM for Npk Init.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

@par Specification Reference:
**/


#include "Npkt.h"
#include <Ppi/MemoryDiscovered.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/PeiNpkInitLib.h>
#include <Guid/NpkInfoHob.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SteppingLib.h>
#include <Library/SerialPortLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/SideBandLib.h>
#include <Library/PmcIpcLib.h>
#include <ScRegs/RegsPsf.h>
#include <SaAccess.h>
#include <ScRegs/RegsLpss.h>
#include <ScRegs/RegsPcu.h>
#include <Library/PeiSaPolicyLib.h>

#define PCODE_BIOS_MAILBOX_INTERFACE_OFFSET  0x7084
#define PCODE_BIOS_MAILBOX_DATA_OFFSET       0x7080

#define READ_TELEMETRY_TRACE_CTL             0x8000000A
#define WRITE_TELEMETRY_TRACE_CTL            0x8000000B

typedef struct {
  UINT8        Master;
  UINT8        Port;
} NPK_MASTER;

NPK_MASTER    mMaster [] = {
  {0, 0},   // TSU
//  {1, 0},   // LakeMore/VISA
//  {2, 0},   // SocHAP
//  {3, 0},   // GT
  {15, 0},  // RTIT core 0-3
  {16, 0},  // CSE 1
  {17, 0},  // CSE 2
  {18, 0},  // CSE 3
//  {19, 0},  // ISH
//  {22, 0},  // HDMI Audio reserved
  {23, 0},  // PMC
  {24, 0},  // AET
//  {25, 0},  // AET extented
//  {26, 0},  // Master to tester (Internal only)
  {27, 0},  // I-Unit
  {28, 0},  // FTH
  {29, 0},  // P-unit
//  {31, 0},  // MLMC
//  {127, 0}  // Trace tool MetaData
};


/**
  GetMscMemSize

  @param[in]  MscSizVariable    Msc buffer size.

  @retval  UINT32
**/
UINT32
GetMscMemSize (
  IN  UINT8          MscSizVariable
  )
{
  switch(MscSizVariable) {
    case 1:
      return 0x100000;

    case 2:
      return 0x800000;

    case 3:
      return 0x4000000;

    case 4:
      return 0x8000000;

    case 5:
      return 0x10000000;

    case 6:
      return 0x20000000;

    case 7:
      return 0x40000000;
    default:
      return 0;
  }
}


/**
  Create NPK reserve memory info HOB.

  @param[in]  NpkPreMemConfig    Pointer to NPK_PRE_MEM_CONFIG

  @retval  EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
NpkCreatReserveHob (
  IN  NPK_PRE_MEM_CONFIG       *NpkPreMemConfig
  )
{
  UINTN                      BufferSize = 0;
  NPK_RESERVE_HOB            *NpkReserveHob = NULL;

  BufferSize = sizeof (NPK_RESERVE_HOB);
  NpkReserveHob = BuildGuidHob (&gNPKReserveMemGuid, BufferSize);
  if (NpkReserveHob == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NpkReserveHob->NpkPreMemConfig.NpkEn               = NpkPreMemConfig->NpkEn;
  NpkReserveHob->NpkPreMemConfig.NpkVrcTapEn         = NpkPreMemConfig->NpkVrcTapEn;
  NpkReserveHob->NpkPreMemConfig.FwTraceEn           = NpkPreMemConfig->FwTraceEn;
  NpkReserveHob->NpkPreMemConfig.RecoverDump         = NpkPreMemConfig->RecoverDump;
  NpkReserveHob->NpkPreMemConfig.FwTraceDestination  = NpkPreMemConfig->FwTraceDestination;
  NpkReserveHob->NpkPreMemConfig.Msc0Size            = NpkPreMemConfig->Msc0Size;
  NpkReserveHob->NpkPreMemConfig.Msc0Wrap            = NpkPreMemConfig->Msc0Wrap;
  NpkReserveHob->NpkPreMemConfig.Msc1Size            = NpkPreMemConfig->Msc1Size;
  NpkReserveHob->NpkPreMemConfig.Msc1Wrap            = NpkPreMemConfig->Msc1Wrap;
  NpkReserveHob->NpkPreMemConfig.PtiMode             = NpkPreMemConfig->PtiMode;
  NpkReserveHob->NpkPreMemConfig.PtiTraining         = NpkPreMemConfig->PtiTraining;
  NpkReserveHob->NpkPreMemConfig.PtiSpeed            = NpkPreMemConfig->PtiSpeed;
  NpkReserveHob->NpkPreMemConfig.PunitMlvl           = NpkPreMemConfig->PunitMlvl;
  NpkReserveHob->NpkPreMemConfig.PmcMlvl             = NpkPreMemConfig->PmcMlvl;
  NpkReserveHob->NpkPreMemConfig.SwTraceEn           = NpkPreMemConfig->SwTraceEn;
  NpkReserveHob->NpkPreMemConfig.NpkDCIEn            = NpkPreMemConfig->NpkDCIEn;

  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.NpkEn = 0x%x\n",NpkReserveHob->NpkPreMemConfig.NpkEn));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.NpkVrcTapEn = 0x%x\n",NpkReserveHob->NpkPreMemConfig.NpkVrcTapEn));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.FwTraceEn = 0x%x\n",NpkReserveHob->NpkPreMemConfig.FwTraceEn));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.RecoverDump = 0x%x\n",NpkReserveHob->NpkPreMemConfig.RecoverDump));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.FwTraceDestination = 0x%x\n",NpkReserveHob->NpkPreMemConfig.FwTraceDestination));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.Msc0Size = 0x%x\n",NpkReserveHob->NpkPreMemConfig.Msc0Size));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.Msc0Wrap = 0x%x\n",NpkReserveHob->NpkPreMemConfig.Msc0Wrap));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.Msc1Size = 0x%x\n",NpkReserveHob->NpkPreMemConfig.Msc1Size));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.Msc1Wrap = 0x%x\n",NpkReserveHob->NpkPreMemConfig.Msc1Wrap));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.PtiMode = 0x%x\n",NpkReserveHob->NpkPreMemConfig.PtiMode));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.PtiTraining = 0x%x\n",NpkReserveHob->NpkPreMemConfig.PtiTraining));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.PtiSpeed = 0x%x\n",NpkReserveHob->NpkPreMemConfig.PtiSpeed));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.PunitMlvl = 0x%x\n",NpkReserveHob->NpkPreMemConfig.PunitMlvl));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.PmcMlvl = 0x%x\n",NpkReserveHob->NpkPreMemConfig.PmcMlvl));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.SwTraceEn = 0x%x\n",NpkReserveHob->NpkPreMemConfig.SwTraceEn));
  DEBUG((EFI_D_INFO , "NpkReserveHob->NpkPreMemConfig.NpkDCIEn = 0x%x\n",NpkReserveHob->NpkPreMemConfig.NpkDCIEn));

  return EFI_SUCCESS;
}

/**
  Performs NPK initialization stage after memory is available.
  Only the feature must be executed right after memory installed should be done here.

  @param[in]  PeiServices   Pointer to PEI Services Table.
  @param[in]  NotifyDesc    Pointer to the descriptor for the Notification event that caused this function to execute.
  @param[in]  Ppi           Pointer to the PPI data associated with this function.

  @retval  EFI_SUCCESS
  @retval  EFI_NOT_FOUND
**/
EFI_STATUS
NpkOnMemoryInstalled (
  IN      EFI_PEI_SERVICES          **PeiServices,
  IN      EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
  IN      VOID                      *Ppi
  )
{
  EFI_STATUS                Status;
  UINT32                    Data32;
  UINT8                     MSCnMode;
  UINTN                     PMCAddress;
  UINT32                    ResetType;
  EFI_PHYSICAL_ADDRESS      MTBRecoveryRamBuffer = 0;
  EFI_PHYSICAL_ADDRESS      MTBCurrentRamBuffer = 0;
  EFI_PHYSICAL_ADDRESS      CSRCurrentRamBuffer = 0;
  UINT32                    *Mtb0SramAddress = 0;
  UINTN                     NpkBaseAddress = 0;
  UINT32                    MSCnSts =0;
  UINT32                    MSCntbwp =0;
  EFI_NORTH_PEAK_INFO_HOB   NpkInfoHob;
  UINTN                     mMtbBar = MTB_BASE_ADDRESS;
  EFI_PEI_HOB_POINTERS      GuidHob;
  NPK_RESERVE_HOB           *NpkReserveHob = NULL;
  UINT32                    SCRPDData;

  ZeroMem (&NpkInfoHob, sizeof(NpkInfoHob));
  DEBUG((EFI_D_INFO , "NpkOnMemoryInstalled\n"));

  //
  // Get NPK reserve memory address from HOB.
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gNPKReserveMemGuid, GuidHob.Raw)) != NULL) {
      NpkReserveHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  if (NpkReserveHob == NULL) {
    DEBUG((EFI_D_ERROR , "Npk HOB Not Found\n"));
    return EFI_NOT_FOUND;
  }

  //
  // Return from memory callback when Memory Region size is 0.
  //
  if (!(((NpkReserveHob->NpkPreMemConfig.Msc0Size != 0) || (NpkReserveHob->NpkPreMemConfig.Msc1Size != 0)) && ((NpkReserveHob->NpkPreMemConfig.NpkEn == 1) || (NpkReserveHob->NpkPreMemConfig.NpkEn == 2)))) {
    DEBUG ((DEBUG_INFO, " NPK Memory Callback Return :Memory Region 0 or NpkEn disabled \n"));
    return EFI_NOT_FOUND;
  }

  if(NpkReserveHob->NpkPreMemConfig.Msc0Size != 0) {
        MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0BAR), (UINT32)NpkReserveHob->MSC0Address >> 12);
        MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0SIZE), ((UINT32)(NpkReserveHob->NpkPreMemConfig.Msc0Size) << 8));  // 4KB align
        DEBUG((EFI_D_INFO , "    MSC0 selected, size = %d MB\n", (UINT32)(NpkReserveHob->NpkPreMemConfig.Msc0Size)));
        DEBUG((EFI_D_INFO , "    MSC0 selected, Address = %x\n", (UINT32)NpkReserveHob->MSC0Address ));
  }

  if(NpkReserveHob->NpkPreMemConfig.Msc1Size != 0) {
        MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC1BAR), (UINT32)NpkReserveHob->MSC1Address >> 12);
        MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC1SIZE), ((UINT32)(NpkReserveHob->NpkPreMemConfig.Msc1Size) << 8));  // 4KB align
        DEBUG((EFI_D_INFO , "    MSC1 selected, size = %d MB\n", (UINT32)NpkReserveHob->NpkPreMemConfig.Msc1Size));
        DEBUG((EFI_D_INFO , "    MSC1 selected, Address = %x\n", (UINT32)NpkReserveHob->MSC1Address ));
  }

  //
  // Check if debug tool connected or only Memory region 0 or 1 are used.
  //
  Data32 = MmioRead32 ((UINTN)mMtbBar + B_PCH_NPK_MTB_SCRPD);
  DEBUG ((EFI_D_ERROR , " B_PCH_NPK_MTB_SCRPD = %x\n", Data32));
  SCRPDData = Data32;
  if (((Data32 & B_PCH_NPK_MTB_SCRPD_DEBUGGER_IN_USE) == B_PCH_NPK_MTB_SCRPD_DEBUGGER_IN_USE) || (NpkReserveHob->NpkPreMemConfig.FwTraceEn == 0 && NpkReserveHob->NpkPreMemConfig.SwTraceEn == 0)) {
    DEBUG ((DEBUG_INFO, " Remote host connected or only use NPK memory to trace log \n"));
    return EFI_SUCCESS;
  }


  if ((NpkReserveHob->NpkPreMemConfig.FwTraceEn == 1 || NpkReserveHob->NpkPreMemConfig.SwTraceEn == 1) && (NpkReserveHob->NpkPreMemConfig.FwTraceDestination == NPK_TRACE_TO_MEMORY)) {
    DEBUG((EFI_D_INFO , "NPK FW/SW Trace enabled and dest is memory.\n"));

    ///
    /// 1. Reallocating Trace Data from Internal Buffer
    ///
    Status = PeiServicesAllocatePages (EfiACPIMemoryNVS, 0x1000/EFI_PAGE_SIZE, &MTBCurrentRamBuffer);
    DEBUG((EFI_D_INFO , "NPK pre-MRC buffer address is = %x  ,Status = %x\n", MTBCurrentRamBuffer, Status));
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
    }

    ///
    /// 2. Inform driver of block address by saving its physical address in GTH SCRPD1 register (bits 43:12 should be saved in SCRPD register).
    ///
    MmioWrite32((UINTN)(mMtbBar + R_PCH_NPK_CSR_MTB_SCRATCHPAD1), (UINT32)(MTBCurrentRamBuffer) >> 12);

    Mtb0SramAddress = (UINT32 *) ((UINTN) (mMtbBar + 0x80000));
    MSCnSts = MmioRead32((UINTN)(mMtbBar + R_PCH_NPK_MTB_MSC0STS));
    MSCntbwp = MmioRead32((UINTN)(mMtbBar + R_PCH_NPK_MTB_MSC0TBWP));
    if (((MSCnSts & B_PCH_NPK_MTB_MSCnSTS_WRAPSTAT) == 0) && (MSCntbwp !=0)) {
       //
       // MTB not Full.
       //
       CopyMem ((VOID*)(UINT32)MTBCurrentRamBuffer, Mtb0SramAddress, (MSCntbwp -1));
       DEBUG((EFI_D_INFO , " MTB not Full,  trace data size is = %x. \n", MSCntbwp -1));
    } else {
       //
       // MTB Full.
       //
       CopyMem ((VOID*)(UINT32)MTBCurrentRamBuffer, Mtb0SramAddress, 0x1000);
       DEBUG((EFI_D_INFO , " MTB is Full,  save 4KB trace data.\n"));
    }

    //
    // Save MTB address as MTB to NPKT acpi table.
    //
    NpkInfoHob.MtbAddress = MTBCurrentRamBuffer;
    NpkInfoHob.MtbSize = 0x1000;
    NpkInfoHob.MtbWriteOffset = MmioRead32 ((UINTN)mMtbBar + R_PCH_NPK_MTB_MSC0MWP);
    NpkInfoHob.MtbWrapStatus = (MSCnSts & B_PCH_NPK_MTB_MSCnSTS_WRAPSTAT);

    if (NpkReserveHob->NpkPreMemConfig.RecoverDump == 1) {
      ///
      /// For android, copy MTB internal buffer to MTB recovery RAM buffer according to the PMC's GCR.GEN_PMCON1.
      /// Recovery buffer holds the data from previous boot for postmortem analysis.
      /// GCR base address is Bus0-DevD-Fun1 Bar 0 +4K.
      ///
      PMCAddress = MmPciAddress (0, 0, 0xD, 0x1, 0x10);
      PMCAddress &= 0xFFFFFFF0;
      ResetType = MmioRead32((UINTN) (PMCAddress + 0x1000 + 0x20));

      if (ResetType) {
        Status = PeiServicesAllocatePages (EfiACPIMemoryNVS, 0x1000/EFI_PAGE_SIZE, &MTBRecoveryRamBuffer);
        DEBUG((EFI_D_INFO , "NPK  pre-MRC recovery buffer address is = %x  ,Status = %x\n", MTBRecoveryRamBuffer, Status));
        if (EFI_ERROR (Status)) {
          ASSERT_EFI_ERROR (Status);
        }
        CopyMem ((VOID*)(UINT32)MTBRecoveryRamBuffer, Mtb0SramAddress, 0x1000);
      }

      NpkInfoHob.MtbRecoveryAddress = MTBRecoveryRamBuffer;
      NpkInfoHob.MtbRecoverySize = 0x1000;
      NpkInfoHob.MtbRecoveryWriteOffset = MmioRead32 ((UINTN)mMtbBar + R_PCH_NPK_MTB_MSC0MWP);

      //
      // Need to allocate reserved buffer.
      //
      CSRCurrentRamBuffer = 0;
    }

    ///
    /// 3. Pause trace collection by de-asserting the StoreEn overrides
    ///
    MmioWrite32((UINTN)(mMtbBar + R_PCH_NPK_MTB_SCR2), 0xFE);

    ///
    /// 4. Flush any residual data from the BPB0
    ///
    MmioOr32((UINTN)(mMtbBar + R_PCH_NPK_MTB_GTHOPT0), B_PCH_NPK_MTB_GTHOPT0_P0FLUSH);

    DEBUG((EFI_D_INFO , "Npk pipeline   = %x  ,Status = %x\n", (MmioRead32((UINTN)(mMtbBar + R_PCH_NPK_MTB_GTHSTAT)) & B_PCH_NPK_MTB_GTHSTAT_PLE0)));
    ///
    /// 5. No need to confirm that the MSC pipeline is empty or not.
    ///
    //while ((MmioRead32((UINTN)(mMtbBar + R_PCH_NPK_MTB_GTHSTAT)) & B_PCH_NPK_MTB_GTHSTAT_PLE0) != 0);

    ///
    /// 6. Clear flush bit
    ///
    MmioAnd32((UINTN)(mMtbBar + R_PCH_NPK_MTB_GTHOPT0), (UINT32)(~B_PCH_NPK_MTB_GTHOPT0_P0FLUSH));

    NpkInfoHob.CsrAddress = (UINT32)NpkReserveHob->MSC0Address;
    NpkInfoHob.CsrSize = (UINT32)(NpkReserveHob->NpkPreMemConfig.Msc0Size << 20) ;
    NpkInfoHob.CsrRecoveryAddress = 0;
    NpkInfoHob.CsrRecoverySize = 0;

    ///
    /// 1. Pause trace collection by de-asserting the StoreEn overrides if it is enabled:
    ///
    MmioWrite32((UINTN)(mMtbBar + R_PCH_NPK_MTB_SCR2), 0xFE);
    //
    // set npkdsc.flr =1 to clear the prior config and make the start async packet as the first line in memory .
    //
    NpkBaseAddress = MmPciAddress (0, 0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK, 0);
    Data32 = MmioRead32 (NpkBaseAddress + R_PCH_NPK_DSC);
    MmioWrite32 (NpkBaseAddress + R_PCH_NPK_DSC, (Data32 | BIT1));

    MmioWrite32 ((UINTN)mMtbBar + B_PCH_NPK_MTB_SCRPD, SCRPDData);

    ///
    /// 2. Disable MSCn if it was enabled
    ///
    Data32 = MmioRead32 (mMtbBar + R_PCH_NPK_MTB_MSC0CTL);
    if ((Data32 & B_PCH_NPK_MTB_MSCNEN) == B_PCH_NPK_MTB_MSCNEN) {
      MmioAnd8 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), (UINT8)(~B_PCH_NPK_MTB_MSCNEN));
      NpkBaseAddress = MmPciAddress (0, 0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK, 0);
      //
      // WA for NPK 1x Errata 1.5 MSCnTBWP not reset when MSCnEN is cleared.
      // Set FON bit.
      //
      Data32 = MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK, 0) + R_PCH_NPK_DSD);
      MmioWrite32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK, 0) + R_PCH_NPK_DSD, (Data32 | BIT0));
    }

    ///
    /// 3 and 4. Write base address and size of the allocated memory block to the MSC0BAR and MSC0SIZE registers.
    ///
    MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0BAR), (UINT32)NpkReserveHob->MSC0Address >> 12);
    MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0SIZE), ((UINT32)NpkReserveHob->NpkPreMemConfig.Msc0Size) << 8);

    ///
    /// 5. Set MCS0 single-block mode.
    ///
    MSCnMode = V_PCH_NPK_MTB_MSCNMODE_SINGLE;
    Data32 = 0x300 | (MSCnMode << N_PCH_NPK_MTB_MSCNMODE);

    ///
    /// 6. Set WRAPEn based on user selection
    ///
    if (NpkReserveHob->NpkPreMemConfig.Msc0Wrap == 1) {
      Data32 |= B_PCH_NPK_MTB_WRAPENN;
    }

    ///
    /// 7. Re-enable MSC0
    ///
    MmioWrite32 ((UINT32) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), Data32);
    MmioOr8 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), B_PCH_NPK_MTB_MSCNEN);

    //
    // Enable STH again as only trace source, WriteRegister(SCR.StoreEnOvrd4,1);
    //
    MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_SCR), 0x100000);

    ///
    /// 8. Resume tracing by clearing the StoreEn overrides:
    ///
    MmioWrite32((UINT32)(mMtbBar + R_PCH_NPK_MTB_SCR2), 0x0);

    //
    // save MSC0 address as CSR to NPKT acpi table.
    //
    NpkInfoHob.CsrAddress = NpkReserveHob->MSC0Address;
  }

  //
  // Build HOB for PlatformInfo.
  //
  BuildGuidDataHob (
    &gEfiNorthPeakGuid,
    &NpkInfoHob,
    sizeof (EFI_NORTH_PEAK_INFO_HOB)
    );

  DEBUG ((DEBUG_INFO, " Config NPK Trace to DRAM End\n"));
  return EFI_SUCCESS;
}

/**
  Entry point for config HW and FW master.

  @param[in]  NpkPreMemConfig   North peak setup option config.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
ProgramMaster(
  IN  NPK_PRE_MEM_CONFIG                 *NpkPreMemConfig
  )
{
  UINT8  MasterIndex;
  UINT8  TraceDest;
  UINT32 SwDestData;
  UINTN  mMtbBar = MTB_BASE_ADDRESS;

  switch (NpkPreMemConfig->FwTraceDestination) {
    case NPK_TRACE_TO_MEMORY:
    case NPK_TRACE_TO_DCI:
    case NPK_TRACE_TO_BSSB :
      TraceDest = 0;
      break;

    case NPK_TRACE_TO_PTI:
    default:
      TraceDest = 2;
      break;
  }


  if (NpkPreMemConfig->FwTraceEn == 1) {
    TraceDest |= 0x8;
  } else {
    TraceDest = 0;
  }

  //Set FW master IDs [0..255].
  for (MasterIndex = 0; MasterIndex < 32; MasterIndex++) {
    SwDestData  = TraceDest | TraceDest << 4 | TraceDest << 8 | TraceDest << 12;
    SwDestData |= TraceDest << 16 | TraceDest << 20 | TraceDest << 24 | TraceDest << 28;
    MmioWrite32 (mMtbBar + R_PCH_NPK_MTB_SWDEST_0 + MasterIndex * 4, SwDestData);
  }

  if (NpkPreMemConfig->SwTraceEn == 1) {
    TraceDest |= 0x8;
  } else {
    TraceDest = 0;
  }

  //Enable SW master IDs [256..259].
  SwDestData = MmioRead32(mMtbBar + B_PCH_NPK_MTB_GSWDEST) & 0xFFFFFFF0;
  SwDestData |= TraceDest;
  MmioWrite32(mMtbBar + B_PCH_NPK_MTB_GSWDEST,SwDestData);
  return EFI_SUCCESS;
}

/**
  Entry point for enable trace to PTI.

  @param[in]  NpkPreMemConfig       North peak setup option config.
  @param[in]  NpkBaseAddress        North peak pci space address.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
EnableTraceToPTI(
  IN  NPK_PRE_MEM_CONFIG        *NpkPreMemConfig,
  IN  UINTN                     NpkBaseAddress
  )
{
  UINT32   Data32;
  UINTN    mMtbBar = MTB_BASE_ADDRESS;

  //
  // 2. 3.3.2.11 Config PTI by default requirement.
  //
  Data32 = (NpkPreMemConfig->PtiMode << N_PCH_NPK_MTB_PTIMODESEL) | (NpkPreMemConfig->PtiSpeed << N_PCH_NPK_MTB_PTICLKDIV) | (NpkPreMemConfig->PtiTraining << N_PCH_NPK_MTB_PATGENMOD) ;
  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_PTI_CTL), Data32);
  MmioOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_PTI_CTL), B_PCH_NPK_MTB_PTI_EN);


  //tracing to PTI
  Data32 =  MmioRead32 ((UINTN)mMtbBar + B_PCH_NPK_MTB_SCRPD);
  Data32 |= B_PCH_NPK_MTB_SCRPD_PTIISPRIMDEST;
  MmioWrite32 ((UINTN) (mMtbBar + B_PCH_NPK_MTB_SCRPD), Data32);

  return EFI_SUCCESS;
}

/**
  Entry point for enable trace to Memory.

  @param[in]  NpkPreMemConfig       North peak setup option config.
  @param[in]  NpkBaseAddress        North peak pci space address.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
EnableTraceToMTB (
  IN  NPK_PRE_MEM_CONFIG       *NpkPreMemConfig,
  IN  UINTN                    NpkBaseAddress
  )
{
  UINT32   Data32;
  UINTN    mMtbBar = MTB_BASE_ADDRESS;

  ///
  /// WA for NPK 1x Errata 1.5 MSCnTBWP not reset when MSCnEN is cleared.
  ///
  Data32 = MmioRead32 (NpkBaseAddress + R_PCH_NPK_DSD);
  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_DSD, (Data32 | BIT0));

  ///
  /// NPK HAS 7.3.3
  /// 7.3.3  1. clear store enable override to stop trace to pti.
  ///
  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_SCR), 0);

  ///
  /// 7.3.3  2. Set the destination override bit to MTB0
  ///
  MmioWrite32 ((UINTN) (mMtbBar + B_PCH_NPK_MTB_DESTOVR), 0x00000008);

  ///
  /// 7.3.3  3. Set MSC0 to internal buffer mode and enable it (wrapping disabled):
  ///
  Data32 = 0x00000031;
  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), Data32);

  ///
  /// 7.3.3  4 enable STH again as only trace source, WriteRegister(SCR.StoreEnOvrd4,1);
  ///
  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_SCR), 0x100000);


  //tracing to MSC0
  Data32 =  MmioRead32 ((UINTN)mMtbBar + B_PCH_NPK_MTB_SCRPD);
  Data32 |= B_PCH_NPK_MTB_SCRPD_MEMISPRIMDEST|B_PCH_NPK_MTB_SCRPD_MSC0ISENABLED;
  MmioWrite32 ((UINTN) (mMtbBar + B_PCH_NPK_MTB_SCRPD), Data32);

  if (NpkPreMemConfig->Msc1Size != 0) {
    //tracing to MSC1
    Data32 =  MmioRead32 ((UINTN)mMtbBar + B_PCH_NPK_MTB_SCRPD);
    Data32 |= B_PCH_NPK_MTB_SCRPD_MEMISPRIMDEST|B_PCH_NPK_MTB_SCRPD_MSC1ISENABLED;
    MmioWrite32 ((UINTN) (mMtbBar + B_PCH_NPK_MTB_SCRPD), Data32);
  }

  return EFI_SUCCESS;
}

/**
  Entry point for enable trace to DCI.

  @param[in]  NpkPreMemConfig       North peak setup option config.
  @param[in]  NpkBaseAddress        North peak pci space address.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
EnableTraceToDCI(
  IN  NPK_PRE_MEM_CONFIG        *NpkPreMemConfig,
  IN  UINTN                     NpkBaseAddress
  )
{
  //
  // Discovered USB3 controller and config.
  //

  //
  // NPKH enable DbC.Trace mode and initialized with pointers to DvC,Trace endpoint.
  //

  //
  // SMU packet frequency selected.
  //

  UINT8   MSCnMode;
  UINT32  Data32;
  UINTN   mMtbBar = MTB_BASE_ADDRESS;
  ///
  /// 1.   Configure the DbC.Trace
  ///
  /// 2.  Program DbC.Trace address into NPKH.
  ///     Program this value into the DBCBASEHI, DBCBASELO, and STREAMCFG1.DBCOFFSET registers/fields.
  ///
  /// 3.  Set up NPKH.TIMEOUT and NPKH.NPKH_RETRY registers
  ///
  MmioOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_TIMEOUT), 0x00);
  MmioOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_NPKHRETRY), 0x00);

  ///
  /// 4.  Configure MSCs for ExI Mode
  ///     For each MSC, set the mode to ExI mode, and enable the MSC by setting the MSCnEN bit.
  ///
  MSCnMode = V_PCH_NPK_MTB_MSCNMODE_EXI;
  Data32 = 0x300 | (MSCnMode << N_PCH_NPK_MTB_MSCNMODE);
  if (NpkPreMemConfig->Msc0Wrap == 1) {
    Data32 |= B_PCH_NPK_MTB_WRAPENN;
  }
  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), Data32);
  MmioOr8 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), B_PCH_NPK_MTB_MSCNEN);

  MSCnMode = V_PCH_NPK_MTB_MSCNMODE_EXI;
  Data32 = 0x300 | (MSCnMode << N_PCH_NPK_MTB_MSCNMODE);
  if (NpkPreMemConfig->Msc1Wrap == 1) {
    Data32 |= B_PCH_NPK_MTB_WRAPENN;
  }

  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC1CTL), Data32);
  MmioOr8 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC1CTL), B_PCH_NPK_MTB_MSCNEN);

  ///
  /// 5.  Set the PCI CMD.BME (Bus Master Enable) bit
  ///
  MmioOr8 (NpkBaseAddress + PCI_COMMAND_OFFSET,(UINT8) (EFI_PCI_COMMAND_BUS_MASTER));

  ///
  /// 6.  Configure the STREAMCFG1 register:
  ///     Write the xHCI.DbC.Trace MMIO address offset to the DBCOFFSET field
  //     SETSTRMMODE = 0
  //     ENABLE = 1
  //   MmioAndThenOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_STREAMCFG1),  ~B_PCH_NPK_MTB_STREAMCFG1_SETSTRMMODE, B_PCH_NPK_MTB_STREAMCFG1_ENABLE);
  MmioAndThenOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_STREAMCFG1), (UINT32)(~B_PCH_NPK_MTB_STREAMCFG1_SETSTRMMODE),(UINT32)( B_PCH_NPK_MTB_STREAMCFG1_ENABLE));

  return EFI_SUCCESS;
}

/**
  Entry point for enable trace to BSSB.

  @param[in]  NpkPreMemConfig  North peak setup option config.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
EnableTraceToBSSB(
  IN    NPK_PRE_MEM_CONFIG                 *NpkPreMemConfig
  )
{
  //
  // ExI bridge and USB PHY init.
  //

  //
  // NPKH set to BSSB mode.
  //

  //
  // MSCc set to ExI mode
  //

  //
  // SMU frequency.

  //
  UINT32  Data32;
  UINT8   MSCnMode;
  UINTN   mMtbBar = MTB_BASE_ADDRESS;
  ///
  /// 1.  Configure the USB3 PHY and ExI Bridge
  ///
  /// 2.  Program the location of the ExI Bridge Trace data base and offset into the NPKH.
  ///      Program this value into the EXIBASEHI, EXIBASELO, and STREAMCFG1.EXIOFFSET registers/fields.
  ///
  /// 3.  Set up NPKH.TIMEOUT and NPKH.NPKH_RETRY registers
  ///
  MmioOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_TIMEOUT), 0x00);
  MmioOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_NPKHRETRY), 0x00);

  ///
  /// 4.  Configure MSCs for ExI Mode
  ///      For each MSC, set the mode to ExI mode, and enable the MSC by setting the MSCnEN bit.
  ///

  MSCnMode = V_PCH_NPK_MTB_MSCNMODE_EXI;
  Data32 = 0x300 | (MSCnMode << N_PCH_NPK_MTB_MSCNMODE);
  if (NpkPreMemConfig->Msc0Wrap == 1) {
    Data32 |= B_PCH_NPK_MTB_WRAPENN;
  }
    MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), Data32);
    MmioOr8 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC0CTL), B_PCH_NPK_MTB_MSCNEN);

  MSCnMode = V_PCH_NPK_MTB_MSCNMODE_EXI;
  Data32 = 0x300 | (MSCnMode << N_PCH_NPK_MTB_MSCNMODE);
  if (NpkPreMemConfig->Msc1Wrap == 1) {
    Data32 |= B_PCH_NPK_MTB_WRAPENN;
  }
  MmioWrite32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC1CTL), Data32);
  MmioOr8 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_MSC1CTL), B_PCH_NPK_MTB_MSCNEN);

  ///
  ///5.  Enable streaming through the STREAMCFG1.ENABLE = 1, and set the streaming mode through STREAMCFG1.SETSTRMMODE = 1.
  ///
  MmioOr32 ((UINTN) (mMtbBar + R_PCH_NPK_MTB_STREAMCFG1), (B_PCH_NPK_MTB_STREAMCFG1_SETSTRMMODE + B_PCH_NPK_MTB_STREAMCFG1_ENABLE));

  return  EFI_SUCCESS;
}

/**
  Entry point for the Destination select.

  @param[in]  NpkPreMemConfig        North peak setup option config.
  @param[in]  NpkBaseAddress         North peak pci space address.
  @param[in]  DestType               Destination type.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
DestinationSelect (
  IN  NPK_PRE_MEM_CONFIG    *NpkPreMemConfig,
  IN  UINTN                 NpkBaseAddress,
  IN  UINT32                DestType
  )
{
  switch (DestType) {
    case NPK_TRACE_TO_MEMORY:
      EnableTraceToMTB(NpkPreMemConfig, NpkBaseAddress);
      break;

    case NPK_TRACE_TO_PTI:
      EnableTraceToPTI(NpkPreMemConfig, NpkBaseAddress);
      break;

    case NPK_TRACE_TO_DCI:
      EnableTraceToDCI(NpkPreMemConfig, NpkBaseAddress);
      break;

    case NPK_TRACE_TO_BSSB:
      EnableTraceToBSSB(NpkPreMemConfig);
      break;

    default:
      EnableTraceToPTI(NpkPreMemConfig, NpkBaseAddress);
      break;
  }
  return EFI_SUCCESS;
}

/**
  Pei NPK Init

  @param[in]  NpkPreMemConfig       North peak setup option config.

  @retval  EFI_SUCCESS
**/
EFI_STATUS
NpkInit (
  IN  NPK_PRE_MEM_CONFIG      *NpkPreMemConfig
  )
{
  EFI_STATUS               Status;
  UINTN                    HostBaseAddress;
  UINTN                    NpkBaseAddress = 0;
  UINT32                   Data32;
  UINT32                   Buffer;
  UINT32                   Buffersize;
  UINT16                   NpkVendorID;
  UINT16                   NpkDeviceID;
  UINT32                   MailboxData=0;
  BOOLEAN                  DisableDebugCpu = FALSE;
  UINT16                   PolicyValue;
  //
  // Install npk policy to record the DRAM size.
  //
  Status = NpkCreatReserveHob (NpkPreMemConfig);
  if (Status != EFI_SUCCESS) {
    DEBUG((EFI_D_INFO , "NPK reserve memory HOB Creation failed\n"));
    return Status;
  }

  //
  // NPK needs to be disabled by early BIOS if DCD bit is set and DFX Policy is not Red (value 2 or 4) and not Orange ( value 5)
  //
  Data32 = SideBandRead32 (SB_CSE_GHOST_PORT, R_SB_HOST_SECBOOT_0_REG_BASE);
  DisableDebugCpu = ((Data32 & B_SB_HOST_SECBOOT_0_DCD) >> 1);
  DEBUG((EFI_D_INFO , "CSE Host_Secboot_0 value 0x%x \n", Data32));

  Data32      = SideBandRead32 (SB_DFX_AGG_PORT, R_DFX_AGG_STATUS_CSR_LOWER_MAP);
  PolicyValue = (UINT16)((Data32 & B_DFX_AGG_STATUS_CSR_LOWER_MAP_DFX_SECURE_POLICY_MASK) >> N_DFX_AGG_STATUS_CSR_LOWER_MAP_DFX_SECURE_POLICY_OFFSET);

  if (NpkPreMemConfig->NpkEn == 0) {
    IpcSendCommandEx (IPC_CMD_ID_NPK, IPC_SUBCMD_ID_NPK_DISABLE, NULL, 0);
    DEBUG((EFI_D_INFO , "Disable NPK by sending IPC1 message.\n"));
    return EFI_SUCCESS;
  }

  if (DisableDebugCpu == TRUE && PolicyValue != V_POLICY_RED2 && PolicyValue != V_POLICY_RED4 && PolicyValue != V_POLICY_ORANGE) {
    IpcSendCommandEx (IPC_CMD_ID_NPK, IPC_SUBCMD_ID_NPK_DISABLE, NULL, 0);
    DEBUG((EFI_D_INFO , "Disable NPK by sending IPC1 message.\n"));
    return EFI_SUCCESS;
  }

  HostBaseAddress = MmPciAddress (0, 0, 0, 0, 0);
  MmioWrite32 (HostBaseAddress + 0x54, (MmioRead32 (HostBaseAddress + 0x54)| BIT2));
  NpkBaseAddress = MmPciAddress (0, 0, PCI_DEVICE_NUMBER_NPK, PCI_FUNCTION_NUMBER_NPK, 0);
  NpkVendorID = MmioRead16 (NpkBaseAddress);
  NpkDeviceID = MmioRead16 (NpkBaseAddress + 2);
  if ((NpkVendorID != V_PCH_NPK_VENDOR_ID) || !(IS_BXT_M_NPK_DEVICE_ID (NpkDeviceID) || IS_BXT_P_NPK_DEVICE_ID (NpkDeviceID))) {
    DEBUG((EFI_D_INFO , "**Unsupported** Npk VID:0x%x,DID:0x%x.\n",MmioRead16 (NpkBaseAddress),(MmioRead16 (NpkBaseAddress + 2))));
    return EFI_UNSUPPORTED;
  }
  //
  // Shadow FW BAR.
  //
  SideBandWrite32(SB_PORTID_PSF1, R_SC_PCR_PSF1_T0_SHDW_NPK_ACPI_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_BAR0, (UINT32)FW_TEMP_BASE_ADDRESS);
  SideBandWrite32(SB_PORTID_PSF1, R_SC_PCR_PSF1_T0_SHDW_NPK_ACPI_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_BAR1, 0);

  SideBandAndThenOr32 (
    SB_PORTID_PSF1,
    R_SC_PCR_PSF1_T0_SHDW_NPK_ACPI_REG_BASE + R_PCH_PCR_PSFX_T0_SHDW_PCIEN,
    0xFFFFFFFF,
    3
  );

  SideBandAndThenOr32 (
    SB_PORTID_PSF1,
    R_SC_PCR_PSF1_T0_SHDW_NPK_ACPI_REG_BASE + PSF_1_AGNT_T0_SHDW_CFG_DIS_NPK_RS0_D0_F5,
    0xFFFFFFFF,
    B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
  );

  //
  // Programe MTB, SW, RTIT, FW BAR base address.
  //
  MmioAnd32 (NpkBaseAddress + R_PCH_NPK_CMD,(UINT32) ~(B_PCH_NPK_CMD_MSE));

  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_CSR_MTB_LBAR, (UINT32)MTB_BASE_ADDRESS);
  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_CSR_MTB_UBAR, 0);

  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_SW_LBAR, (UINT32)SW_BASE_ADDRESS);
  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_SW_UBAR, 0);

  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_RTIT_LBAR, (UINT32)RTIT_BASE_ADDRESS);
  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_RTIT_UBAR, 0);

  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_FW_LBAR, (UINT32)FW_TEMP_BASE_ADDRESS);
  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_FW_UBAR, 0);

  Data32 = MmioRead32 (NpkBaseAddress + R_PCH_NPK_CMD);
  MmioWrite32 (NpkBaseAddress + R_PCH_NPK_CMD, (Data32 | 6));

  // Read SCRPD register & check if BIT24 is set or not
  // IAFW should program the SWDEST_0 to SWDEST_31 registers & GSWDEST register only if the BIT24 is not set
  Data32 = MmioRead32((UINTN)MTB_BASE_ADDRESS + B_PCH_NPK_MTB_SCRPD);
  if (!(Data32 & B_PCH_NPK_MTB_SCRPD_DEBUGGER_IN_USE)) {
    DEBUG((EFI_D_INFO, "NPK Programming SWDEST_x & GSWDEST\n"));
    ProgramMaster(NpkPreMemConfig);
  }

  //
  // Reset MTB SCRPD.
  //
  Data32 = MmioRead32 ((UINTN)MTB_BASE_ADDRESS + B_PCH_NPK_MTB_SCRPD);
  Data32 &= ~(B_PCH_NPK_MTB_SCRPD_MEMISPRIMDEST|B_PCH_NPK_MTB_SCRPD_MSC0ISENABLED|B_PCH_NPK_MTB_SCRPD_MSC1ISENABLED|B_PCH_NPK_MTB_SCRPD_PTIISPRIMDEST);
  MmioWrite32 ((UINTN) (MTB_BASE_ADDRESS + B_PCH_NPK_MTB_SCRPD), Data32);
  //
  // Select destination.
  //
  DestinationSelect (NpkPreMemConfig, NpkBaseAddress, NpkPreMemConfig->FwTraceDestination);

  if ((NpkPreMemConfig->FwTraceEn) || (NpkPreMemConfig->SwTraceEn)) {
    //
    // Clear Bypass.
    //
    MmioAnd32 (NpkBaseAddress + R_PCH_NPK_DSC,(UINT32) ~(B_PCH_NPK_BYP));
  }

  //
  // Read MTB SCRPD
  //
  Data32 = MmioRead32 ((UINTN)MTB_BASE_ADDRESS + B_PCH_NPK_MTB_SCRPD);
  // Configure Message verbosity level only if DEBUGGER IN USE bit is not set
  if ( ! (Data32 & B_PCH_NPK_MTB_SCRPD_DEBUGGER_IN_USE) ) {
    if( NpkPreMemConfig->PunitMlvl != PUNIT_DEBUG_MESSAGE_VERBOSITY_LEVEL_DEFAULT ) {
      //
      // Set PUNIT verbosity level to the level used in Setup Menu
      // (For A0 it was bug so Bios has to use BIT[0:1] ,from B0 onwards its fixed so use BIT[27:28] for punit)
      //
      if ((GetBxtSeries() == Bxt1 && BxtStepping() >= BxtB0) || (GetBxtSeries() == BxtP && BxtStepping() >= BxtPB0)) {
        MailboxData = (NpkPreMemConfig->PunitMlvl << 27);
      } else {
        MailboxData = NpkPreMemConfig->PunitMlvl;
      }
      MmioWrite32((UINTN)(MCH_BASE_ADDRESS + PCODE_BIOS_MAILBOX_DATA_OFFSET), MailboxData);
      MmioWrite32((UINTN)(MCH_BASE_ADDRESS + PCODE_BIOS_MAILBOX_INTERFACE_OFFSET), WRITE_TELEMETRY_TRACE_CTL);
    }

    if( NpkPreMemConfig->PmcMlvl != PMC_DEBUG_MESSAGE_VERBOSITY_LEVEL_DEFAULT ) {
      //Set PMC verbosity level to the level used in Setup Menu
      Buffer = 0;
      Buffersize = 4;
      IpcSendCommandEx (IPC_CMD_ID_TELEMETRY, IPC_SUBID_TELEMETRY_TRACE_CTL_READ, &Buffer, Buffersize);
      Buffer = (Buffer & 0xE7FFFFFF) | (((UINT32)NpkPreMemConfig->PmcMlvl) << 27);
      IpcSendCommandEx (IPC_CMD_ID_TELEMETRY, IPC_SUBID_TELEMETRY_TRACE_CTL_WRITE, &Buffer, Buffersize);
    }
  }

  //
  // Enable Bus Master.
  //
  MmioOr8 (NpkBaseAddress + R_PCH_NPK_CMD,(UINT8) (EFI_PCI_COMMAND_BUS_MASTER));

  return EFI_SUCCESS;
}

/**
  NPK PEI Initialization.

  @param[in]  FileHandle

  @retval  EFI_SUCCESS
**/
EFI_STATUS
PeiNpkInit (
  IN  CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS            Status;
  SI_SA_POLICY_PPI      *SiSaPolicyPpi;
  NPK_PRE_MEM_CONFIG    *NpkPreMemConfig = NULL;

  DEBUG ((DEBUG_INFO, "PeiNpkInit() - Start\n"));

  Status = (*PeiServices)->LocatePpi (
                                 PeiServices,
                                 &gSiSaPreMemPolicyPpiGuid,
                                 0,
                                 NULL,
                                 (void **)&SiSaPolicyPpi
                                 );

  Status = GetConfigBlock((VOID *)SiSaPolicyPpi, &gNpkPreMemConfigGuid, (VOID *)&NpkPreMemConfig);
  ASSERT_EFI_ERROR(Status);

  //
  // PEI NPK Initialize
  //
  Status = NpkInit (NpkPreMemConfig);


  return Status;
}
