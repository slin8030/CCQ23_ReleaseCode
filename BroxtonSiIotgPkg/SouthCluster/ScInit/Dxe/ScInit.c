/** @file
  This is the driver that initializes the Intel SC devices

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

@par Specification Reference:
**/

#include "ScInit.h"

#ifndef FSP_FLAG
#include <Library/UefiDriverEntryPoint.h>
#include <Library/PreSiliconLib.h>
#include <Protocol/PciIo.h>
#include <Library/PmcIpcLib.h>
#include <Library/DxeVtdLib.h>
#else
#include <Library/FspCommonLib.h>
#include <FspEas.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Guid/EventGroup.h>
#include <FspmUpd.h>
#include <SaRegs.h>
#include <FspEas.h>
#include <FspsUpd.h>
#endif

#include <SaAccess.h>
#include <ScAccess.h>
#include <ScRegs/RegsPcu.h>
#include <Library/SteppingLib.h>
#include <Library/HeciMsgLib.h>
#include <SeCChipset.h>
#include <SeCState.h>
#include <Private/ScPmcFunctionDisableResetHob.h>
#include <Protocol/Spi.h>
#include <Library/ScSpiCommonLib.h>

//[-start-160511-IB03090427-add]//
#include <Protocol/PciEnumerationComplete.h>
//[-end-160511-IB03090427-add]//

//[-start-160802-IB03090430-add]//
extern EFI_GUID gFdoModeEnabledHobGuid;
//[-end-160802-IB03090430-add]//
//[-start-160620-IB07400745-add]//
#include <ChipsetPostCode.h>
//[-end-160620-IB07400745-add]//


//
// Global Variables
//
EFI_HANDLE  mImageHandle;
//[-start-160705-IB07400752-add]//
BOOLEAN     mIsLegacyBoot = FALSE;
//[-end-160705-IB07400752-add]//

GLOBAL_REMOVE_IF_UNREFERENCED  SC_POLICY_HOB   *mScPolicy;

#ifdef FSP_FLAG
EFI_STATUS
EFIAPI
FspReadyToBootCallback (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  );

EFI_STATUS
EFIAPI
FspPciEnumCompleteCallback (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  );

EFI_STATUS
EFIAPI
FspEndOfFirmwareCallback (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  );

EFI_PEI_NOTIFY_DESCRIPTOR  mNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK),
    &gEfiEventReadyToBootGuid,
    FspReadyToBootCallback
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK),
    &gEfiPciEnumerationCompleteProtocolGuid,
    FspPciEnumCompleteCallback
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gFspEventEndOfFirmwareGuid,
    FspEndOfFirmwareCallback
  }
};
#endif


#ifndef FSP_FLAG
/**
  SC initialization before booting to OS

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScOnReadyToBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS Status;
  BXT_SERIES BxtSeries;
  SC_LOCK_DOWN_CONFIG        *LockDownConfig;

  DEBUG ((DEBUG_INFO, "ScOnReadyToBoot() Start\n"));

  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  BxtSeries = GetBxtSeries ();
  Status = GetConfigBlock ((VOID *) mScPolicy, &gLockDownConfigGuid, (VOID *) &LockDownConfig);
  ASSERT_EFI_ERROR (Status);
  //
  // Trigger an SW SMI to config PCIE power management setting after PCI enumeration is done
  //
  if (BxtSeries == BxtP || (BxtSeries == Bxt1)) {
#if (VP_BIOS_ENABLE == 0) || (TABLET_PF_ENABLE == 0)
    IoWrite8 (R_APM_CNT, SW_SMI_PCIE_ASPM_OVERRIDE);
#endif
  }
  //
  // Trigger an SW SMI to do BiosWriteProtect
  //
  if ((BxtSeries == BxtP) && (LockDownConfig->BiosLock == TRUE)) {
    IoWrite8 (R_APM_CNT, (UINT8)LockDownConfig->BiosLockSwSmiNumber);
  }
  DEBUG ((DEBUG_INFO, "ScOnReadyToBoot() End\n"));
}
#endif

//[-start-160705-IB07400752-add]//
extern EFI_GUID gEfiPciEnumerationStartCompleteProtocolGuid;

VOID
EFIAPI
ScOnPciEnumStartComplete (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS          Status;
  VOID                *ProtocolPointer;
  
  Status = gBS->LocateProtocol (&gEfiPciEnumerationStartCompleteProtocolGuid, NULL, (VOID **) &ProtocolPointer);
  if (EFI_SUCCESS != Status) {
    return;
  }
  
  gBS->CloseEvent (Event);
  
  ConfigureIrqAtBoot (mScPolicy);
}
//[-end-160705-IB07400752-add]//

/**
  This is the callback function for PCI Enumeration Complete.
**/
VOID
EFIAPI
ScOnPciEnumComplete (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
#ifndef FSP_FLAG
  EFI_STATUS                        Status;
  VOID                              *ProtocolPointer;
#endif
  UINT8                             Reset;
  EFI_PEI_HOB_POINTERS              HobPtr;
  SC_PMC_FUNCTION_DISABLE_RESET_HOB *FuncDisHob;
  EFI_HOB_GUID_TYPE                 *FdoEnabledGuidHob = NULL;
#ifdef FSP_FLAG
  FSPM_UPD                          *FspmUpd;
#endif
//[-start-170210-IB08450362-add]//
  BXT_SERIES                        BxtSeries;
//[-end-170210-IB08450362-add]//

  DEBUG ((DEBUG_INFO, "ScOnPciEnumComplete ()\n"));

#ifdef FSP_FLAG
  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
#endif
  ///
  /// Check if this is first time called by EfiCreateProtocolNotifyEvent() or not,
  /// if it is, we will skip it until real event is triggered
  ///
#ifndef FSP_FLAG
  Status = gBS->LocateProtocol (&gEfiPciEnumerationCompleteProtocolGuid, NULL, (VOID **) &ProtocolPointer);
  if (EFI_SUCCESS != Status) {
    return;
  }
  gBS->CloseEvent (Event);
#endif

//[-start-170210-IB08450362-add]//
  BxtSeries = GetBxtSeries ();  
  if (BxtSeries == BxtP){
    ConfigureSataDxe (mScPolicy);
  }
//[-end-170210-IB08450362-add]//

  // Get SC PMC fuction disable reset HOB.
  HobPtr.Guid = GetFirstGuidHob (&gScPmcFunctionDisableResetHobGuid);
  if (HobPtr.Guid != NULL) {
    FuncDisHob = GET_GUID_HOB_DATA (HobPtr.Guid);

    // CSE status. When CSE is in recovery mode. CSE convert the  cold reset to the Global reset
    // so function disable register is not preserved
    FdoEnabledGuidHob = GetFirstGuidHob (&gFdoModeEnabledHobGuid);
    if (FdoEnabledGuidHob == NULL) {
      //
      // Do not change the order of Global reset and Cold reset.
      // Global reset clears the PMC function disable register.
      // If done after cold reset, it will end up in a dead loop.
      //
      if (FuncDisHob->ResetType == SC_PMC_FUNCTION_DISABLE_GLOBAL_RESET) {
//[-start-160620-IB07400745-add]//
        CHIPSET_POST_CODE (PEI_FRC_FUN_DISABLE_UPDATE_REBOOT);
//[-end-160620-IB07400745-add]//
        DEBUG ((DEBUG_INFO, "PMC function disable updated, do global reset. \n"));
#ifdef FSP_FLAG
        if (!(FspmUpd->FspmConfig.EnableResetSystem)) {
          FspApiReturnStatusReset (FSP_STATUS_RESET_REQUIRED_5);
        }
#endif
        HeciSendResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
        CpuDeadLoop();    // Should not continue execution after global reset
      }
      if (FuncDisHob->ResetType == SC_PMC_FUNCTION_DISABLE_COLD_RESET) {
//[-start-160620-IB07400745-add]//
        CHIPSET_POST_CODE (PEI_FRC_FUN_DISABLE_UPDATE_REBOOT);
//[-end-160620-IB07400745-add]//
        DEBUG ((DEBUG_INFO, "PMC function disable updated, do cold reset. \n"));
        Reset = IoRead8 (R_RST_CNT);
        Reset &= 0xF1;
        Reset |= 0xE;
#ifdef FSP_FLAG
          if (!(FspmUpd->FspmConfig.EnableResetSystem)) {
            FspApiReturnStatusReset (FSP_STATUS_RESET_REQUIRED_COLD);
          }
#endif
          IoWrite8(R_RST_CNT, Reset);
          CpuDeadLoop();    // Should not continue execution after cold reset
      }
    }
  }
  return;
}

//[-start-160705-IB07400752-add]//
VOID
EFIAPI
ScLegacyBootEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  mIsLegacyBoot = TRUE;
  ScExitBootServicesEvent (Event, Context);
}

VOID
EFIAPI
ScLpssLegacyBootEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  mIsLegacyBoot = TRUE;
  ScLpssExitBootServicesEvent (Event, Context);
}
//[-end-160705-IB07400752-add]//

/**
  Register EndOfDXE, ReadyToBoot, ExitBootService event functions
  for SC configuration in different stages
**/
VOID
ScRegisterNotifications (
  VOID
  )
{
#ifndef FSP_FLAG
  EFI_STATUS  Status;
  EFI_EVENT   ReadyToBoot;
  EFI_EVENT   LegacyBootEvent;
  EFI_EVENT   ExitBootServicesEvent;
  VOID        *Registration;

  //
  // Create PCI Enumeration Completed callback
  //
  EfiCreateProtocolNotifyEvent (
    &gEfiPciEnumerationCompleteProtocolGuid,
    TPL_CALLBACK,
    ScOnPciEnumComplete,
    NULL,
    &Registration
    );
//[-start-160705-IB07400752-add]//
  EfiCreateProtocolNotifyEvent (
    &gEfiPciEnumerationStartCompleteProtocolGuid,
    TPL_CALLBACK,
    ScOnPciEnumStartComplete,
    NULL,
    &Registration
    );
//[-end-160705-IB07400752-add]//
  //
  // Register an end of DXE event for PCH to do tasks before invoking any UEFI drivers,
  // applications, or connecting consoles,...
  // Create an ExitPmAuth protocol call back event.
  //
  EfiCreateProtocolNotifyEvent (
    &gExitPmAuthProtocolGuid,
    TPL_CALLBACK,
    ScOnEndOfDxe,
    NULL,
    &Registration
    );

  //
  // Register a Ready to boot event to config PCIE power management setting after OPROM executed
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             ScOnReadyToBoot,
             NULL,
             &ReadyToBoot
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Create events for SC to do the task before ExitBootServices/LegacyBoot.
  // It is guaranteed that only one of two events below will be signalled
  //
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_CALLBACK,
                  ScExitBootServicesEvent,
                  NULL,
                  &ExitBootServicesEvent
                  );
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_NOTIFY,
                  ScLpssExitBootServicesEvent,
                  NULL,
                  &ExitBootServicesEvent
                  );

  ASSERT_EFI_ERROR (Status);

  Status = EfiCreateEventLegacyBootEx (
             TPL_CALLBACK,
//[-start-160705-IB07400752-modify]//
             ScLegacyBootEvent,
//[-end-160705-IB07400752-modify]//
             NULL,
             &LegacyBootEvent
             );
  Status = EfiCreateEventLegacyBootEx (
             TPL_NOTIFY,
//[-start-160705-IB07400752-modify]//
             ScLpssLegacyBootEvent,
//[-end-160705-IB07400752-modify]//
             NULL,
             &LegacyBootEvent
             );
  ASSERT_EFI_ERROR (Status);
#endif
}

#ifndef FSP_FLAG
/**
  South cluster initialization entry point

  @param[in] ImageHandle                Handle for the image of this driver
  @param[in] SystemTable                Pointer to the EFI System Table

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
EFIAPI
ScInitEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
#else
/**
  South cluster initialization entry point for Fsp

  @param[in]  FileHandle           Not used.
  @param[in]  PeiServices          General purpose services available to every PEIM.

  @retval     EFI_SUCCESS          The function completes successfully
**/
EFI_STATUS
EFIAPI
ScInitEntryPoint (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
#endif
{
  EFI_STATUS               Status;
  UINT32                   PmcBase;
  EFI_PEI_HOB_POINTERS     HobPtr;
  SC_INSTANCE_PRIVATE_DATA *ScInstance;
  UINT16                   AcpiBaseAddr;
#ifndef FSP_FLAG
  EFI_HANDLE               Handle;
#endif

#ifdef FSP_FLAG  //Dptf driver
  UINTN                 McD0BaseAddress;
  FSPS_UPD              *FspsUpd;
#endif

  DEBUG ((DEBUG_INFO, "ScInitEntryPoint() Start\n"));

  ScInstance   = NULL;
  mScPolicy    = NULL;
  AcpiBaseAddr = 0;

#ifndef FSP_FLAG
  Handle       = NULL;
  mImageHandle = ImageHandle;
#endif

  //
  // Get SC Policy HOB.
  //
  Status = EFI_NOT_FOUND;
  HobPtr.Guid = GetFirstGuidHob (&gScPolicyHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  mScPolicy = GET_GUID_HOB_DATA (HobPtr.Guid);

  //
  // Allocate and install the SC Info protocol
  //
  PmcBase      = PMC_BASE_ADDRESS;
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  DEBUG ((DEBUG_INFO, "SC Base Addresses:\n-------------------\n"));
  DEBUG ((DEBUG_INFO, "  PmcBase   0x%X\n", PmcBase));
  DEBUG ((DEBUG_INFO, "  AcpiBase  0x%X\n", AcpiBaseAddr));
  DEBUG ((DEBUG_INFO, "-------------------\n"));

  ASSERT (PmcBase != 0);
  ASSERT (AcpiBaseAddr != 0);
  DEBUG ((DEBUG_INFO, "SC Base Addresses:\n----START---------------\n"));
  DEBUG ((DEBUG_INFO, "  AcpiBase  0x%X\n", AcpiBaseAddr));
#if (ENBDT_PF_ENABLE == 1)
  DEBUG ((DEBUG_INFO, "CAUTION Broxton P ACPI offsets\n"));
#else
  DEBUG ((DEBUG_INFO, "Broxton M ACPI offsets\n"));
#endif
  DEBUG ((DEBUG_INFO, "  PM1_STS_EN  0x%X  0x%X\n",R_ACPI_PM1_STS, IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_STS))));;
  DEBUG ((DEBUG_INFO, "  PM1_CNT     0x%X  0x%X\n",R_ACPI_PM1_CNT, IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT))));;
  DEBUG ((DEBUG_INFO, "  PM1_TMR     0x%X  0x%X\n",R_ACPI_PM1_TMR, IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_TMR))));;
  DEBUG ((DEBUG_INFO, "  GPE0a_STS   0x%X  0x%X\n",R_ACPI_GPE0a_STS, IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_GPE0a_STS))));;
  DEBUG ((DEBUG_INFO, "  GPE0a_EN    0x%X  0x%X\n",R_ACPI_GPE0a_EN, IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_GPE0a_EN))));;
  DEBUG ((DEBUG_INFO, "  SMI_EN      0x%X  0x%X\n",R_SMI_EN, IoRead32 ((UINTN) (AcpiBaseAddr + R_SMI_EN))));;
  DEBUG ((DEBUG_INFO, "  SMI_STS     0x%X  0x%X\n",R_SMI_STS, IoRead32 ((UINTN) (AcpiBaseAddr + R_SMI_STS))));;
  DEBUG ((DEBUG_INFO, "  GPE_CTRL    0x%X  0x%X\n",R_ACPI_GPE_CNTL, IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_GPE_CNTL))));;
  DEBUG ((DEBUG_INFO, "  TCO_RLD     0x%X  0x%X\n",R_TCO_RLD, IoRead32 ((UINTN) (AcpiBaseAddr + R_TCO_RLD))));;
  DEBUG ((DEBUG_INFO, "  TCO_STS     0x%X  0x%X\n",R_TCO_STS, IoRead32 ((UINTN) (AcpiBaseAddr + R_TCO_STS))));;
  DEBUG ((DEBUG_INFO, "  TCO1_CNT    0x%X  0x%X\n",R_TCO1_CNT, IoRead32 ((UINTN) (AcpiBaseAddr + R_TCO1_CNT))));;
  DEBUG ((DEBUG_INFO, "  TCO_TMR     0x%X  0x%X\n",R_TCO_TMR, IoRead32 ((UINTN) (AcpiBaseAddr + R_TCO_TMR))));;
  DEBUG ((DEBUG_INFO, "SC Base Addresses:\n----END---------------\n"));

  //
  // Initialize the SC device
  //
  InitializeScDevice (ScInstance, mScPolicy, PmcBase, AcpiBaseAddr);

  ScRegisterNotifications ();

#ifndef FSP_FLAG
  ScInstance = AllocateZeroPool (sizeof (SC_INSTANCE_PRIVATE_DATA));
  if (ScInstance == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }
  ScInstance->ScInfo.Revision   = INFO_PROTOCOL_REVISION_1;
  ScInstance->ScInfo.BusNumber  = DEFAULT_PCI_BUS_NUMBER_SC;
  ScInstance->ScInfo.RCVersion  = SC_POLICY_REVISION;

  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiInfoProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &(ScInstance->ScInfo)
                  );
#else
  Status = (**PeiServices).NotifyPpi (PeiServices, &mNotifyList[0]);
  ASSERT_EFI_ERROR (Status);
#endif

  ///
  ///  Created DPTF Driver to disable DPTF Config Space.
  ///  Entry Point for this driver.This procedure does all the DPTF initialization and loads the ACPI tables.
  ///

#ifdef FSP_FLAG
  FspsUpd = (FSPS_UPD *)GetFspSiliconInitUpdDataPointer ();

  if (!FspsUpd->FspsConfig.DptfEnabled) {
    McD0BaseAddress = MmPciAddress (0, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
    DEBUG ((EFI_D_INFO, "DPTF: McD0BaseAddress = 0x%x \n",McD0BaseAddress));
    DEBUG ((EFI_D_INFO, "DPTF: R_SA_MC_CAPID0_A = 0x%x \n",MmioRead32
		(McD0BaseAddress + R_SA_MC_CAPID0_A)));
    if (MmioRead32 (McD0BaseAddress + R_SA_MC_CAPID0_A) & BIT15 ) {
      DEBUG ((EFI_D_INFO,
		"DPTF: Write PSF Reg R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_OFFSET5 "));
      PchPcrAndThenOr32 (PID_PSF1, R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_CSE_BXTP
	  ,(UINT32) ~0, B_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_FUNDIS);
    }

    //
    // PCI CFG space function disable register.
    //
    MmioAnd32 (MmPciBase (SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_DEVEN, (UINT32)~BIT1);

    PchPcrAndThenOr32 (PID_PSF1, R_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_CSE_BXTP
	,(UINT32) ~0, B_PCH_PCR_PSF1_T0_SHDW_PCIEN_CUNIT_RS0_D0_F1_FUNDIS);

  }
#endif

  DEBUG ((DEBUG_INFO, "ScInitEntryPoint() End\n"));

  return EFI_SUCCESS;
}

/**
  Initialize the SC device according to the SC Platform Policy

  @param[in,out] ScInstance          SC instance private data. May get updated by this function
  @param[in]     ScPolicy            The SC Platform Policy protocol instance
  @param[in]     PmcBase             PMC base address of this SC device
  @param[in]     AcpiBaseAddr        ACPI IO base address of this SC device

  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
InitializeScDevice (
  IN OUT SC_INSTANCE_PRIVATE_DATA       *ScInstance,
  IN     SC_POLICY_HOB                  *ScPolicy,
  IN     UINT32                         PmcBase,
  IN     UINT16                         AcpiBaseAddr
  )
{
#ifndef FSP_FLAG
  EFI_STATUS            Status;
#else
  EFI_STATUS            Status=TRUE;
#endif

  UINT32                FuncDisableReg;
  BXT_SERIES            BxtSeries;

  DEBUG ((DEBUG_INFO, "InitializeScDevice() Start\n"));
  FuncDisableReg  = MmioRead32 (PmcBase + R_PMC_FUNC_DIS);
  BxtSeries = GetBxtSeries ();
  //
  // Configure root port function number mapping
  // For UEFI bios, execute RPFN mapping before PCI enumeration.
  //
  if ((BxtSeries == BxtP) || (BxtSeries == Bxt1)) {
    PcieConfigureRpfnMapping ();
  }
  ///
  ///
  /// VT-d Initialization
  ///
#ifndef FSP_FLAG
  DEBUG ((DEBUG_INFO, "Initializing VT-d in Dxe to update DMAR table\n"));
  VtdInit ();
#endif

  ///
  /// Configure PMC Devices
  ///
#ifndef FSP_FLAG
  Status = ConfigurePmcAfterPciEnum (ScPolicy);
  ASSERT_EFI_ERROR (Status);
#endif

  ///
  /// Direct IRQ (PMC) programming
  ///
  ConfigurePlatformDirectIrqs (AcpiBaseAddr);

  DEBUG ((DEBUG_INFO, "InitializeScDevice() End\n"));

  return Status;
}

/**
  Program Sc devices dedicated IRQ#.

  @param[in] ScPolicy                   The SC Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
#ifdef FSP_FLAG
EFI_STATUS
ConfigureIrqAtBoot (
  IN      SC_POLICY_HOB                 *ScPolicy
  )
{
  UINTN   Index;
  UINTN   TblIndex;
  UINTN   BusNum;
  UINTN   DevNum;
  UINTN   FunNum;
  UINTN   HandleCount;
  UINT32  PciCfgBase;

  STATIC SC_IRQ_INIT_ENTRY  IrqInitTable[] = {
    /*Bus  Dev  Fun  Irq*/
    {0,    0,    2,  23},  // NPK
    {0,    0,    1,  24},  // PUINT
    {0,    0,    3,  23},  // gmm
    {0,    3,    0,  21},  // iunit

    {0,    0xE,  0,  25},  // HD-Audio

    {0,    0x11, 0,  26},  // ISH

    //{0,    0x15, 0,  17},  // XHCI
    {0,    0x15, 1,  13},  // xDCI

    {0,    0x16, 0,  27},  // I2C1
    {0,    0x16, 1,  28},  // I2C2
    {0,    0x16, 2,  29},  // I2C3
    {0,    0x16, 3,  30},  // I2C4

    {0,    0x17, 0,  31},  // I2C5
    {0,    0x17, 1,  32},  // I2C6
    {0,    0x17, 2,  33},  // I2C7
    {0,    0x17, 3,  34},  // I2C8

    {0,    0x18, 0,   4},  // UART1
    {0,    0x18, 1,   5},  // UART2
    {0,    0x18, 2,   6},  // UART3
    {0,    0x18, 3,   7},  // UART4

    {0,    0x19, 0,   35}, // SPI1
    {0,    0x19, 1,   36}, // SPI2
    {0,    0x19, 2,   37}, // SPI3

    {0,    0x1B, 0,    3 }, // SDCard
    {0,    0x1C, 0,   39 }, // eMMC
    {0,    0x1D, 0,   38 }, // UFS
    {0,    0x1E, 0,   42 }, // SDIO
  };

  DEBUG ((EFI_D_INFO, "ConfigureIrqAtBoot() Start\n"));

  HandleCount = 1;
  for(Index = 0; Index < HandleCount; Index ++) {
    for (TblIndex = 0; TblIndex < (sizeof (IrqInitTable) / sizeof (SC_IRQ_INIT_ENTRY)); TblIndex++) {
      BusNum = IrqInitTable[TblIndex].BusNumber;
      DevNum = IrqInitTable[TblIndex].DeviceNumber;
      FunNum = IrqInitTable[TblIndex].FunctionNumber;
      PciCfgBase = MmPciAddress (0,
                     BusNum,
                     DevNum,
                     FunNum,
                     0);
      if (MmioRead32 (PciCfgBase) == 0xFFFFFFFF) {
        continue;
      }
      MmioWrite8 (PciCfgBase + PCI_INT_LINE_OFFSET, (UINT8)IrqInitTable[TblIndex].Irq);
      DEBUG ((EFI_D_INFO, "Writing IRQ#%d for B%d/D%d/F%d\n", IrqInitTable[TblIndex].Irq, BusNum, DevNum, FunNum));
    }
  }

  DEBUG ((EFI_D_INFO, "ConfigureIrqAtBoot() End\n"));
  return EFI_SUCCESS;
}

#else
EFI_STATUS
ConfigureIrqAtBoot (
  IN      SC_POLICY_HOB    *ScPolicy
  )
{
  UINTN                                      Index;
  UINTN                                      TblIndex;
  UINTN                                      Segment;
  UINTN                                      BusNum;
  UINTN                                      DevNum;
  UINTN                                      FunNum;
  UINTN                                      HandleCount;
  EFI_STATUS                                 Status;
  EFI_HANDLE                                 *Handles;
  EFI_PCI_IO_PROTOCOL                        *PciIo;
  STATIC SC_IRQ_INIT_ENTRY                   IrqInitTable[] = {
    /*Bus  Dev  Fun  Irq*/
    {0,    0,    2,  23},  // NPK
    {0,    0,    1,  24},  // PUINT
    {0,    0,    3,  23},  // gmm
    {0,    3,    0,  21},  // iunit


    {0,    0xE,  0,  25},  // HD-Audio

    {0,    0x11, 0,  26},  // ISH

    {0,    0x15, 0,  17},  // XHCI
    {0,    0x15, 1,  13},  // xDCI

    {0,    0x16, 0,  27},  // I2C1
    {0,    0x16, 1,  28},  // I2C2
    {0,    0x16, 2,  29},  // I2C3
    {0,    0x16, 3,  30},  // I2C4

    {0,    0x17, 0,  31},  // I2C5
    {0,    0x17, 1,  32},  // I2C6
    {0,    0x17, 2,  33},  // I2C7
    {0,    0x17, 3,  34},  // I2C8

//[-start-160827-IB07400774-modify]/
#ifdef LEGACY_IRQ_SUPPORT
    {0,    0x18, 0,  44},  // UART1
    {0,    0x18, 1,  45},  // UART2
    {0,    0x18, 2,   6},  // UART3
    {0,    0x18, 3,  47},  // UART4
#else
    {0,    0x18, 0,   4},  // UART1
    {0,    0x18, 1,   5},  // UART2
    {0,    0x18, 2,   6},  // UART3
    {0,    0x18, 3,   7},  // UART4
#endif
//[-end-160827-IB07400774-modify]//

    {0,    0x19, 0,   35},  // SPI1
    {0,    0x19, 1,   36},  // SPI2
    {0,    0x19, 2,   37},  // SPI3

//[-start-160827-IB07400774-modify]//
#ifdef LEGACY_IRQ_SUPPORT
    {0,    0x1B, 0,   43 }, // SDCard
#else
    {0,    0x1B, 0,    3 }, // SDCard
#endif
//[-end-160827-IB07400774-modify]//
    {0,    0x1C, 0,   39 }, // eMMC
    {0,    0x1D, 0,   38 }, // UFS
    {0,    0x1E, 0,   42 }, // SDIO
  };

  DEBUG ((EFI_D_INFO, "ConfigureIrqAtBoot() Start\n"));

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &Handles
                  );

  DEBUG ((EFI_D_INFO, "Status = %r\n", Status));

  if (EFI_ERROR(Status)) {
    return Status;
  }

  for(Index = 0; Index < HandleCount; Index ++) {
    Status = gBS->HandleProtocol (
                    Handles[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );

    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = PciIo->GetLocation(
                      PciIo,
                      &Segment,
                      &BusNum,
                      &DevNum,
                      &FunNum
                      );

    for (TblIndex = 0; TblIndex < (sizeof (IrqInitTable) / sizeof (SC_IRQ_INIT_ENTRY)); TblIndex++) {
      if((BusNum == IrqInitTable[TblIndex].BusNumber) &&
          (DevNum == IrqInitTable[TblIndex].DeviceNumber) &&
          (FunNum == IrqInitTable[TblIndex].FunctionNumber)) {

          Status = PciIo->Pci.Write(
                                PciIo,
                                EfiPciIoWidthUint8,
                                0x3c,//Offset 0x3c :PCI Interrupt Line
                                1,
                                &IrqInitTable[TblIndex].Irq
                                );

          DEBUG ((EFI_D_INFO, "Writing IRQ#%d for B%d/D%d/F%d\n", IrqInitTable[TblIndex].Irq, BusNum, DevNum, FunNum));
      } else {
        continue;
      }

    }

  }

  DEBUG ((EFI_D_INFO, "ConfigureIrqAtBoot() End\n"));
  return EFI_SUCCESS;
}
#endif

//[-start-161128-IB07400820-add]//
//
// Please refer IBL doc#559360 for more detail
//
typedef struct {
  UINT8  GpioGroup;
  UINT8  GpeDwVaule;
} APL_GPIO_GROUP_GPE_MAP;

APL_GPIO_GROUP_GPE_MAP mGpioGroupGpeMapTable [] = {
  0x00, 0x00,
  0x01, 0x01,
  0x02, 0x09,
  0x04, 0x03,
  0x05, 0x04,
  0x06, 0x05,
  0x07, 0x06,
  0x08, 0x07
};

UINT8
GetGpeDwValue (
  IN UINT8 GpioGruop
  )
{
  UINTN Index;
  UINTN TableSize;

  TableSize = sizeof (mGpioGroupGpeMapTable)/sizeof (APL_GPIO_GROUP_GPE_MAP);
  for (Index = 0; Index < TableSize; Index ++) {
    if (GpioGruop == mGpioGroupGpeMapTable[Index].GpioGroup) {
      return (mGpioGroupGpeMapTable[Index].GpeDwVaule & 0xF);
    }
  }
  return 0;
}

UINT32 
GetGpioGpeCfgValue (
  VOID
  )
{
  UINT32 Gpe0bValue;
  UINT32 Gpe0cValue;
  UINT32 Gpe0dValue;

  Gpe0bValue = (UINT32)GetGpeDwValue ((UINT8)PcdGet8 (PcdGpe0bGpioGroup)) << 4;
  Gpe0cValue = (UINT32)GetGpeDwValue ((UINT8)PcdGet8 (PcdGpe0cGpioGroup)) << 8;
  Gpe0dValue = (UINT32)GetGpeDwValue ((UINT8)PcdGet8 (PcdGpe0dGpioGroup)) << 12;

  return (Gpe0bValue | Gpe0cValue | Gpe0dValue);
}
//[-end-161128-IB07400820-add]//

/**
  Configure and Enable Direct IRQs (IOAPIC) for PMC

  Note: These settings are OS-agnostic.

  If the IP never enters D0i3, then Direct IRQ has no impact
  and is never triggered.

  @param[in]     AcpiBaseAddr        ACPI IO base address

  @retval EFI_SUCCESS                The function completed successfully
**/
VOID
ConfigurePlatformDirectIrqs (
  IN     UINT16            AcpiBaseAddr
  )
{
    UINT32 Data32;
    DEBUG ((EFI_D_INFO, "InitPlatformDirectIrqs() - Start\n"));

    Data32 = 0;
    //
    // Program the Direct IRQ Interrupt Vectors
    //
    MmioAndThenOr32 (
      PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_0,
      0x00000000,
      ((7 << N_PMC_IRQ_SEL_0_DIR_IRQ_UART3) +
      (6 << N_PMC_IRQ_SEL_0_DIR_IRQ_UART2) +
      (5 << N_PMC_IRQ_SEL_0_DIR_IRQ_UART1) +
      (4 << N_PMC_IRQ_SEL_0_DIR_IRQ_UART0))
      );

    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN)(PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_0),
      1,
      (VOID *)(UINTN)(PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_0)
      );

    MmioAndThenOr32 (
      PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_1,
      0x00000000,
      ((17 << N_PMC_IRQ_SEL_1_DIR_IRQ_XHCI) +
      (13 << N_PMC_IRQ_SEL_1_DIR_IRQ_XDCI) +
      (3 << N_PMC_IRQ_SEL_1_DIR_IRQ_SDCARD_CD) +
      (3 << N_PMC_IRQ_SEL_1_DIR_IRQ_SDCARD_D1))
      );

    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN)(PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_1),
      1,
      (VOID *)(UINTN)(PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_1)
      );

    //
    // Bits  ->  SCI Map
    // ------------------
    // A Stepping:
    //  000  ->   IRQ9  *BXTM
    //  001  ->   IRQ10
    //  010  ->   IRQ11
    //  100  ->   IRQ20 (APIC must be enabled)
    //  101  ->   IRQ21 (APIC must be enabled)
    //  110  ->   IRQ22 (APIC must be enabled)
    //  111  ->   IRQ23 (APIC must be enabled)
    //
    // B Stepping / BXTP:
    //  9    ->   IRQ9
    //  ...
    //
    // Write the SCI IRQ to the ACPI Bridge
    //
    MmioAndThenOr32 (
      PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_2,
      0x0000FFFF,
      (V_PMC_IRQ_SEL_2_SCI_IRQ_9 << N_PMC_IRQ_SEL_2_SCIS) +
      (41 << N_PMC_IRQ_SEL_2_DIR_IRQ_SEL_PMIC)
      );

    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN)(PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_2),
      1,
      (VOID *)(UINTN)(PMC_BASE_ADDRESS + R_PMC_IRQ_SEL_2)
      );

    if (GetBxtSeries () == BxtP) {
#ifdef FSP_FLAG
      Data32 = MmioRead32(PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG);
      DEBUG ((EFI_D_INFO, "PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG = 0x%x\n", Data32));
#endif
#ifndef FSP_FLAG
      //
      // Map Gpe0b to the correct tier in GPIO_GPE_CFG register
      //
//[-start-161128-IB07400820-modify]//
      MmioAndThenOr32 (
        PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG,
        0xFFFF000F,
        (UINT32)GetGpioGpeCfgValue()
        );
//[-end-161128-IB07400820-modify]//
#endif
#ifdef FSP_FLAG
      Data32 = MmioRead32(PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG);
      DEBUG ((EFI_D_INFO, "PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG = 0x%x\n", Data32));
#endif

      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN)(PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG),
        1,
        (VOID *)(UINTN)(PMC_BASE_ADDRESS + R_PMC_GPIO_GPE_CFG)
        );

    } else {
      //
      // Program the Direct IRQ Enables
      //
      //  - Not executed on BXTP since Windows does not currently support Direct IRQ
      //    - However, should have no impact if enabled
      //  - Windows will use SCI for wake flows on xHCI/xDCI
      //


      IoOr32 (
        AcpiBaseAddr + R_DIRECT_IRQ_EN,
        (UINT32)(
        B_DIRECT_IRQ_EN_PMIC_EN |
        B_DIRECT_IRQ_EN_XHCI_EN |
        B_DIRECT_IRQ_EN_XDCI_EN |
        // B_DIRECT_IRQ_EN_SDIO_D1_EN |
        B_DIRECT_IRQ_EN_SDCARD_CD_EN)
        );
    }

    // The OS ACPI driver will control ACPI.PM1_CNT_SCI_EN and populate the
    // SCI interrupt vector in the fixed ACPI block.

    DEBUG ((EFI_D_INFO, "InitPlatformDirectIrqs() - End\n"));
}

#ifdef FSP_FLAG
/**
  Initialize SC configuration triggered by ReadyToBoot event for Fsp

  @param[in] PeiServices      General purpose services available to every PEIM.
  @param[in] NotifyDescriptor Notify that this module published, here is
  @param[in] Ppi              PPI that was installed.

  @retval    EFI_SUCCESS      The function completed successfully.
**/
EFI_STATUS
EFIAPI
FspReadyToBootCallback (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
{
  EFI_BOOT_MODE     BootMode;
  EFI_STATUS        Status;
  UINTN             P2SBBase;
  SC_P2SB_CONFIG    *P2sbConfig;

  DEBUG ((DEBUG_INFO, "FspReadyToBootCallback() - Start\n"));

  ScOnEndOfDxe (NULL,  NULL);

  Status = (*PeiServices)->GetBootMode (
                             (const EFI_PEI_SERVICES **)PeiServices,
                             &BootMode
                             );
  if (BootMode == BOOT_ON_S3_RESUME) {
    //
    // Hide P2SB device in OS phase
    //
    Status = GetConfigBlock ((VOID *) mScPolicy, &gP2sbConfigGuid, (VOID *) &P2sbConfig);
    ASSERT_EFI_ERROR (Status);
    if (P2sbConfig->P2sbUnhide == 0) {
      DEBUG ((DEBUG_INFO, "Hide P2SB on S3 resume mode..........\n"));
    P2SBBase = MmPciAddress (
               0,
               0,
               PCI_DEVICE_NUMBER_P2SB,
               PCI_FUNCTION_NUMBER_P2SB,
               R_P2SB_P2SBC
               );
    MmioOr32(P2SBBase, B_P2SB_P2SBC_HIDE);
    }
  }
  DEBUG ((DEBUG_INFO, "FspReadyToBootCallback() - End\n"));
  return EFI_SUCCESS;
}

/**
  Initialize SC configuration triggered by PCI Enumeration Completed for Fsp

  @param[in] PeiServices      General purpose services available to every PEIM.
  @param[in] NotifyDescriptor Notify that this module published, here is
  @param[in] Ppi              PPI that was installed.

  @retval    EFI_SUCCESS      The function completed successfully.
**/
EFI_STATUS
EFIAPI
FspPciEnumCompleteCallback (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
{
  DEBUG ((DEBUG_INFO, "FspPciEnumCompleteCallback() - Start\n"));

  ScOnPciEnumComplete (NULL,  NULL);

  DEBUG ((DEBUG_INFO, "FspPciEnumCompleteCallback() - End\n"));
  return EFI_SUCCESS;
}

/**
  Initialize SC configuration triggered by ExitBootService event for Fsp

  @param[in] PeiServices      General purpose services available to every PEIM.
  @param[in] NotifyDescriptor Notify that this module published, here is
  @param[in] Ppi              PPI that was installed.

  @retval    EFI_SUCCESS      The function completed successfully.
**/
EFI_STATUS
EFIAPI
FspEndOfFirmwareCallback (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
{
  DEBUG ((EFI_D_INFO, "FspEndOfFirmwareCallback() - Start\n"));

  ScLpssExitBootServicesEvent (NULL, NULL);
  ScExitBootServicesEvent (NULL, NULL);

  DEBUG ((EFI_D_INFO, "FspEndOfFirmwareCallback() - End\n"));
  return EFI_SUCCESS;
}
#endif

/**
  SC initialization triggered by ExitBootServices events
  Useful for operations which must happen later than at the end of post

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScExitBootServicesEvent (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  UINTN      P2SBBase;
  UINT32     Buffer9;
  BXT_SERIES BxtSeries;

  Buffer9   = 0;
  BxtSeries = GetBxtSeries ();
#ifndef FSP_FLAG
  //
  // Closed the event to avoid call twice
  //
  gBS->CloseEvent (Event);
#endif

#ifndef FSP_FLAG
  UpdateNvsArea (mScPolicy);
  ConfigureHdaAtBoot (mScPolicy);
if (PLATFORM_ID == VALUE_REAL_PLATFORM){
  ConfigureOtgAtBoot (mScPolicy);
}
#endif
#ifdef FSP_FLAG
  ConfigurePmcAtBoot (mScPolicy);
#endif
  ConfigureIrqAtBoot (mScPolicy);
  StopLpssAtBoot ();
  // Hide SPI(B0/D13/F2) device in OS Phase
  //
  if (BxtSeries != BxtP) {
    SideBandAndThenOr32(
      SB_PORTID_PSF3,
      R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_SPI_RS0_D13_F2,
      0xFFFFFFFF,
      B_PCH_PCR_PSFX_T0_SHDW_CFG_DIS_CFGDIS
     );
  }

  //
  // Hide P2SB device in OS phase
  //
  DEBUG ((DEBUG_INFO, "Hide P2SB on normal mode..........\n"));
  P2SBBase = MmPciAddress (
             0,
             0,
             PCI_DEVICE_NUMBER_P2SB,
             PCI_FUNCTION_NUMBER_P2SB,
             R_P2SB_P2SBC
             );
  MmioOr32(P2SBBase, B_P2SB_P2SBC_HIDE);

  //
  // Set the P2SB bit in FUNC_ACPI_ENUM_1.
  //
  Buffer9 = MmioRead32(PMC_BASE_ADDRESS + R_PMC_FUNC_ACPI_ENUM_1);
  Buffer9 = Buffer9|BIT0;
  MmioWrite32 (PMC_BASE_ADDRESS + R_PMC_FUNC_ACPI_ENUM_1, Buffer9);

  return;
}

/**
  SC initialization triggered by ExitBootServices for LPSS events

  This should be at a higher TPL than the callback that sets Untrusted Mode.

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScLpssExitBootServicesEvent (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
#ifndef FSP_FLAG
  //
  // Closed the event to avoid call twice
  //
  gBS->CloseEvent (Event);
#endif

  ConfigureLpssAtBoot (mScPolicy);
}

/**
  SC initialization before Boot Script Table is closed

  @param[in] Event                      A pointer to the Event that triggered the callback.
  @param[in] Context                    A pointer to private data registered with the callback function.
**/
VOID
EFIAPI
ScOnEndOfDxe (
  IN EFI_EVENT                          Event,
  IN VOID                               *Context
  )
{
  EFI_STATUS                 Status;
  UINTN                      Index;
  UINT32                     FuncDisableReg;
  UINT16                     Data16;
  UINT32                     Data32;
  UINT32                     Data32And;
  UINT32                     Data32Or;
  UINT16                     AcpiBaseAddr;
  UINT32                     PmcBase;
  UINT8                      NumOfDevltrOverride;
  UINT32                     DlockValue;
  UINTN                      PciLpcRegBase;
  UINTN                      PciSpiRegBase;
  UINTN                      SpiBar0;
  UINTN                      Timer;
  BXT_SERIES                 BxtSeries;
  UINT8                      Data8;
  SC_LOCK_DOWN_CONFIG        *LockDownConfig;
  SC_FLASH_PROTECTION_CONFIG *FlashProtectionConfig;
  SI_POLICY_HOB              *SiPolicyHob;
  EFI_PEI_HOB_POINTERS       HobPtr;
#if RVVP_ENABLE == 0
  UINT16                     Data16And;
  UINT16                     Data16Or;
#endif
//[-start-160506-IB07220074-add]//
  VOID         *Interface;
//[-end-160506-IB07220074-add]//

//[-start-160506-IB07220074-add]//
  //
  // Try to locate it because EfiCreateProtocolNotifyEvent will trigger it once when registration.
  // Just return if it is not found.
  //
  Status = gBS->LocateProtocol (
                  &gExitPmAuthProtocolGuid,
                  NULL,
                  &Interface
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }
//[-end-160506-IB07220074-add]//

  NumOfDevltrOverride = 0;
  PciLpcRegBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_PCH_LPC,
                    PCI_FUNCTION_NUMBER_PCH_LPC
                    );
  PciSpiRegBase = MmPciBase (
                    DEFAULT_PCI_BUS_NUMBER_SC,
                    PCI_DEVICE_NUMBER_SPI,
                    PCI_FUNCTION_NUMBER_SPI
                    );

  DEBUG ((DEBUG_INFO, "ScOnEndOfDxe() Start\n"));

  ///
  /// Closed the event to avoid call twice when launch shell
  ///
#ifndef FSP_FLAG
  gBS->CloseEvent (Event);
#endif

  BxtSeries = GetBxtSeries ();
//[-start-170210-IB08450362-remove]//
  //if (BxtSeries == BxtP){
  //  ConfigureSataDxe (mScPolicy);
  //}
//[-end-170210-IB08450362-remove]//
  AcpiBaseAddr   = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  PmcBase        = PMC_BASE_ADDRESS;
  FuncDisableReg = MmioRead32 (PmcBase + R_PMC_FUNC_DIS);

  ///
  /// CHV BIOS Spec, Section 29.5 Additional Power Management Programming
  /// Step 1
  /// It is recommended to clear the "CF9h Global Reset" bit, PBASE + 0x48 [20] = 0h
  /// prior to loading the OS.
  ///
  MmioAnd32 (
    (UINTN) (PmcBase + R_PMC_ETR),
    (UINT32) ~(B_PMC_ETR_CF9GR)
    );
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PmcBase + R_PMC_ETR),
    1,
    (VOID *) (UINTN) (PmcBase + R_PMC_ETR)
    );

  if (BxtSeries == BxtP){
    SpiBar0 = MmioRead32 (PciSpiRegBase + R_SPI_BASE) &~(B_SPI_BAR0_MASK);
    Status = GetConfigBlock ((VOID *) mScPolicy, &gFlashProtectionConfigGuid, (VOID *) &FlashProtectionConfig);
    ASSERT_EFI_ERROR (Status);
    ///
    /// Program the Flash Protection Range Register based on policy
    ///
    DlockValue = MmioRead32 (SpiBar0 + R_SPI_DLOCK);
    for (Index = 0; Index < SC_FLASH_PROTECTED_RANGES; ++Index) {
      if ((FlashProtectionConfig->ProtectRange[Index].WriteProtectionEnable ||
           FlashProtectionConfig->ProtectRange[Index].ReadProtectionEnable) != TRUE) {
        continue;
      }

      ///
      /// Proceed to program the register after ensure it is enabled
      ///
      Data32 = 0;
      Data32 |= (FlashProtectionConfig->ProtectRange[Index].WriteProtectionEnable == TRUE) ? B_SPI_PRX_WPE : 0;
      Data32 |= (FlashProtectionConfig->ProtectRange[Index].ReadProtectionEnable == TRUE) ? B_SPI_PRX_RPE : 0;
      Data32 |= ((UINT32) FlashProtectionConfig->ProtectRange[Index].ProtectedRangeLimit << N_SPI_PRX_PRL) & B_SPI_PRX_PRL_MASK;
      Data32 |= ((UINT32) FlashProtectionConfig->ProtectRange[Index].ProtectedRangeBase << N_SPI_PRX_PRB) & B_SPI_PRX_PRB_MASK;
      DEBUG ((DEBUG_INFO, "Protected range %d: 0x%08x \n", Index, Data32));

      DlockValue |= (UINT32) (B_SPI_DLOCK_PR0LOCKDN << Index);
      MmioWrite32 ((UINTN) (SpiBar0 + (R_SPI_PR0 + (Index * S_SPI_PRX))), Data32);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint32,
        (UINTN) (SpiBar0 + (R_SPI_PR0 + (Index * S_SPI_PRX))),
        1,
        (VOID *) (UINTN) (SpiBar0 + (R_SPI_PR0 + (Index * S_SPI_PRX)))
        );
    }
    //
    // Program DLOCK register
    //
    MmioWrite32 ((UINTN) (SpiBar0 + R_SPI_DLOCK), DlockValue);
    S3BootScriptSaveMemWrite (
      S3BootScriptWidthUint32,
      (UINTN) (SpiBar0 + R_SPI_DLOCK),
      1,
      (VOID *) (UINTN) (SpiBar0 + R_SPI_DLOCK)
      );
    ///
    /// In SPI controller the BIOS should set the Flash Configuration Lock-Down bit
    /// (SPI_BAR0 + 04[15]) at end of post.  When set to 1, those Flash Program Registers
    /// that are locked down by this FLOCKDN bit cannot be written.
    /// Additionally BIOS must program SPI_BAR0 + 0x04 BIT11 (WRSDIS) to disable Write Status in HW sequencing
    ///
    //

    //
    // Ensure there is no pending SPI trasaction before setting lock bits
    //
    Timer = 0;
    while (MmioRead16 (SpiBar0 + R_SPI_HSFS) & B_SPI_HSFS_SCIP) {
      if (Timer > WAIT_TIME) {
        //
        // SPI transaction is pending too long at this point, exit with error.
        //
        DEBUG ((DEBUG_ERROR, "SPI Cycle timeout\n"));
        ASSERT (FALSE);
        break;
      }
      MicroSecondDelay (WAIT_PERIOD);
      Timer += WAIT_PERIOD;
    }

    Data16And = B_SPI_HSFS_SCIP;
    Data16    = 0;
    S3BootScriptSaveMemPoll (
      S3BootScriptWidthUint16,
      SpiBar0 + R_SPI_HSFS,
      &Data16And,
      &Data16,
      WAIT_PERIOD,
      WAIT_TIME / WAIT_PERIOD
      );

    //
    // Clear any outstanding status
    //
    Data16Or  = B_SPI_HSFS_AEL
              | B_SPI_HSFS_FCERR
              | B_SPI_HSFS_FDONE;

    Data16And = 0xFFFF;
    MmioAndThenOr16 (SpiBar0 + R_SPI_HSFS, Data16And, Data16Or);
    S3BootScriptSaveMemReadWrite (
      S3BootScriptWidthUint16,
      SpiBar0 + R_SPI_HSFS,
      &Data16Or,
      &Data16And
      );

    //
    // Set WRSDIS
    //
    Data16Or  = B_SPI_HSFS_WRSDIS;
    Data16And = 0xFFFF;
    MmioAndThenOr16 (SpiBar0 + R_SPI_HSFS, Data16And, Data16Or);
    S3BootScriptSaveMemReadWrite (
      S3BootScriptWidthUint16,
      SpiBar0 + R_SPI_HSFS,
      &Data16Or,
      &Data16And
      );

    //
    // Set FLOCKDN
    //
    Data16Or  = B_SPI_HSFS_FLOCKDN;
    Data16And = 0xFFFF;
    MmioAndThenOr16 (SpiBar0 + R_SPI_HSFS, Data16And, Data16Or);
    S3BootScriptSaveMemReadWrite (
      S3BootScriptWidthUint16,
      SpiBar0 + R_SPI_HSFS,
      &Data16Or,
      &Data16And
      );
  }

  ///
  /// Flash Security Recommendation
  /// Step 1
  /// Intel strongly recommends that BIOS enables the BIOS Lock Enable (BLE) feature of the SC.
  /// Left to platform code to register an callback function to handle IchnBiosWp SMI
  ///
  /// Step 2
  /// Intel strongly recommends that BIOS enables SMI_LOCK (PBASE + 0x24 [4] = 1)
  /// which prevent writes to the Global SMI Enable bit (GLB_SMI_EN, ABASE + 0x30 [0]).
  /// Enabling this bit will mitigate malicious software attempts to gain system management
  /// mode privileges.
  ///
  Status = GetConfigBlock ((VOID *) mScPolicy, &gLockDownConfigGuid, (VOID *) &LockDownConfig);
  ASSERT_EFI_ERROR (Status);
  if (LockDownConfig->GlobalSmi == TRUE) {
    ///
    /// Save Global SMI Enable bit setting before BIOS enables SMI_LOCK during S3 resume
    ///
    Data32Or  = IoRead32 ((UINTN) (AcpiBaseAddr + R_SMI_EN));
    if ((Data32Or & B_SMI_EN_GBL_SMI) != 0) {
      Data32And = 0xFFFFFFFF;
      Data32Or &= B_SMI_EN_GBL_SMI;
      S3BootScriptSaveIoReadWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (AcpiBaseAddr + R_SMI_EN),
        &Data32Or,  // Data to be ORed
        &Data32And  // Data to be ANDed
        );
    }
    MmioOr8 ((UINTN) (PmcBase + R_PMC_GEN_PMCON_2), B_PMC_GEN_PMCON_SMI_LOCK);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (PmcBase + R_PMC_GEN_PMCON_2),
      1,
      (VOID *) (UINTN) (PmcBase + R_PMC_GEN_PMCON_2)
      );
  }

  ///
  /// Flash Security Recommendation
  /// Step 1
  /// BIOS needs to enable the BIOS Lock Enable (BLE) feature of the SoC by setting
  /// SBASE + 0xFC [1] = 1b. When this bit is set, attempts to write the BIOS Write
  /// Enable (BIOSWE) bit in SC will cause a SMI which will allow the BIOS to verify
  /// that the write is from a valid source. Remember that BIOS needs to set SBASE
  /// Offset 0xFC [0] = 0b to enable BIOS region protection before exiting the SMI handler.
  /// Also, TCO_EN bit needs to be set (SMI_EN Register, ABASE + 30h [13] = 1b) to keep
  /// BLE feature enabled after booting to the OS.
  ///
  /// Generate ScBiosLock SW SMI to register IchnBiosWp callback function in
  /// ScBiosLockSwSmiCallback() to handle TCO BIOSWR SMI
  ///
  if (GetBxtSeries() == BxtP) {
    if (LockDownConfig->BiosLock == TRUE) {
      //
      // LPC
      //
      if (! (MmioRead8 (PciLpcRegBase + R_PCH_LPC_BC) & B_PCH_LPC_BC_LE)) {
        DEBUG ((DEBUG_INFO, "Set LPC bios lock\n"));
        MmioOr8 ((UINTN) (PciLpcRegBase + R_PCH_LPC_BC), B_PCH_LPC_BC_LE);
        S3BootScriptSaveMemWrite (
          S3BootScriptWidthUint8,
          (UINTN) (PciLpcRegBase + R_PCH_LPC_BC),
          1,
          (VOID *) (UINTN) (PciLpcRegBase + R_PCH_LPC_BC)
          );
      }
      //
      // SPI
      //
      if (! (MmioRead8 (PciSpiRegBase + R_SPI_BCR) & B_SPI_BCR_BLE)) {
        DEBUG ((DEBUG_INFO, "Set SPI bios lock\n"));
        MmioOr8 ((UINTN) (PciSpiRegBase + R_SPI_BCR), (UINT8) B_SPI_BCR_BLE);
        S3BootScriptSaveMemWrite (
          S3BootScriptWidthUint8,
          (UINTN) (PciSpiRegBase + R_SPI_BCR),
          1,
          (VOID *) (UINTN) (PciSpiRegBase + R_SPI_BCR)
          );
      }
    }

    if (LockDownConfig->BiosInterface == TRUE) {
      //
      // LPC
      //
      MmioOr8 ((UINTN) (PciLpcRegBase + R_PCH_LPC_BC), (UINT8) B_PCH_LPC_BC_BILD);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint8,
        (UINTN) (PciLpcRegBase + R_PCH_LPC_BC),
        1,
        (VOID *) (UINTN) (PciLpcRegBase + R_PCH_LPC_BC)
        );
      //
      // Reads back for posted write to take effect
      //
      Data8 = MmioRead8 ((UINTN) (PciLpcRegBase + R_PCH_LPC_BC));
      S3BootScriptSaveMemPoll  (
        S3BootScriptWidthUint8,
        (UINTN) (PciLpcRegBase + R_PCH_LPC_BC),
        &Data8,  // BitMask
        &Data8,  // BitValue
        1,          // Duration
        1           // LoopTimes
        );
      //
      // SPI
      //
      MmioOr8 ((UINTN) (PciSpiRegBase + R_SPI_BCR), (UINT8) B_SPI_BCR_BILD);
      S3BootScriptSaveMemWrite (
        S3BootScriptWidthUint8,
        (UINTN) (PciSpiRegBase + R_SPI_BCR),
        1,
        (VOID *) (UINTN) (PciSpiRegBase + R_SPI_BCR)
        );

      //
      // Reads back for posted write to take effect
      //
      Data8 = MmioRead8 ((UINTN) (PciSpiRegBase + R_SPI_BCR));
      S3BootScriptSaveMemPoll  (
        S3BootScriptWidthUint8,
        (UINTN) (PciSpiRegBase + R_SPI_BCR),
        &Data8,     // BitMask
        &Data8,     // BitValue
        1,          // Duration
        1           // LoopTimes
        );
    }
  }

  if (LockDownConfig->TcoLock) {
    //
    // Enable TCO and Lock Down TCO
    //
#if RVVP_ENABLE == 0   // TODO: Need to file sighting.
    DEBUG ((DEBUG_INFO, "Enable TCO and Lock Down TCO\n"));
//[-start-161230-IB07400829-remove]//
    //
    // Enable in platform.c (SMM)
    //
//    Data16And = 0xFFFF;
//    Data16Or = B_SMI_EN_TCO;
//    IoOr16 (AcpiBaseAddr + R_SMI_EN, Data16Or);
//[-end-161230-IB07400829-remove]//

    Data16And = 0xFFFF;
    Data16Or = B_TCO1_CNT_LOCK;
    IoOr16 (AcpiBaseAddr + R_TCO1_CNT, Data16Or);

    S3BootScriptSaveIoReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (AcpiBaseAddr + R_TCO1_CNT),
      &Data16Or,  // Data to be ORed
      &Data16And  // Data to be ANDed
    );

    DEBUG ((DEBUG_INFO, "Enable TCO and Lock Down TCO ---- END\n"));
#else
    DEBUG ((DEBUG_INFO, "Not enable TCO and Lock Down TCO for RVVP\n"));
#endif
  }

  HobPtr.Guid = GetFirstGuidHob (&gSiPolicyHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  SiPolicyHob = GET_GUID_HOB_DATA (HobPtr.Guid);
  UsbInitBeforeBoot (SiPolicyHob, mScPolicy);

  ///
  /// For Data integrity protection, set RTC Memory locks at SBCR[RTC] + 3400h[4] and SBCR[RTC] + 3400h[3].
  /// Note once locked bytes 0x38 - 0x3F in each of the Upper and Lower Byte blocks cannot be accessed.
  /// Writes will be dropped and reads will not return any guaranteed data. Bit reset on system reset.
  ///
  if (LockDownConfig->RtcLock == TRUE) {
    Data32And = 0xFFFFFFFF;
    Data32Or  = (B_PCH_PCR_RTC_CONF_UCMOS_LOCK | B_PCH_PCR_RTC_CONF_LCMOS_LOCK | B_PCH_PCR_RTC_CONF_BILD);
    PchPcrAndThenOr32 (
      PID_RTC,
      R_PCH_PCR_RTC_CONF,
      Data32And,
      Data32Or
      );
    PCR_BOOT_SCRIPT_READ_WRITE (
      S3BootScriptWidthUint32,
      PID_RTC,
      R_PCH_PCR_RTC_CONF,
      &Data32Or,
      &Data32And
      );
  }

  //
  // Trigger an SW SMI to config PCIE power management setting after PCI enumeration is done
  //
  if ((BxtSeries == BxtP) || (BxtSeries == Bxt1)){
    Data8 = SW_SMI_PCIE_ASPM_OVERRIDE;
    S3BootScriptSaveIoWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (R_APM_CNT),
      1,
      &Data8
      );
  }

  DEBUG ((DEBUG_INFO, "ScOnEndOfDxe() End\n"));

  return;
}
