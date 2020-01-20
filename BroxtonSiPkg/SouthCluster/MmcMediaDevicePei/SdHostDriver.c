/** @file
 SD Host Driver Entry and support.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#include <PlatformBaseAddresses.h>
#include <ScAccess.h>
#include <Library/Private/PeiScInitLib/ScInitPei.h>
#include "SdHostDriver.h"

//
// MMCSDIOController Driver Global Variables
//
STATIC PEI_SD_CONTROLLER_PPI mSdControllerPpi = {
  EFI_SD_HOST_IO_PROTOCOL_REVISION_01,
  {
    0, ///< HighSpeedSupport
    0, ///< V18Support
    0, ///< V30Support
    0, ///< V33Support
    0, ///< SDR50Support
    0, ///< SDR104Support
    0, ///< DDR50Support
    0, ///< Reserved0
    0, ///< BusWidth4
    0, ///< BusWidth8
    0, ///< Reserved1
    0,
    0,
    0,
    0,
    0,
    0,
    (512 * 1024) ///<BoundarySize

  },
  HostSendCommand,
  SetClockFrequency,
  SetBusWidth,
  SetHostVoltage,
  SetHostDdrMode,
  ResetSdHost,
  EnableAutoStopCmd,
  DetectCardAndInitHost,
  SetBlockLength,
  SetupDevice,
  SetHostSpeedMode
};

typedef struct {
  UINT8 portid;
  UINT16 RegOff;
  UINT32 mask;
  UINT32 value;
} SCCCONFIG;

STATIC EFI_PEI_PPI_DESCRIPTOR mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiSdhcPpiGuid,
  NULL
};

extern PEI_SDHOST_DATA  gSdHostData;

/**
  Enable Sdhc controller

  @param[in]  PeiSdhostData  - PEI_SDHOST_DATA
  @param[in]  SdControllerId - UINT8

  @retval  EFI_SUCCESS        - Success
  @retval  EFI_DEVICE_ERROR   - Fail
**/
EFI_STATUS
EnableSdhcController (
  IN PEI_SDHOST_DATA          *PeiSdhostData,
  IN UINT8                    SdControllerId
  );

/**
  Dump Emmc Converge Layer Regs

  @retval  None
**/
VOID
DumpEmmcConvergeLayerRegs (
  VOID
  )
{
DEBUG ((EFI_D_INFO, " R_SCC_MEM_SW_LTR_VALUE, = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_SW_LTR_VALUE)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_AUTO_LTR_VALUE, = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_AUTO_LTR_VALUE)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_CAP_BYPASS_CNTL, = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_CAP_BYPASS_CNTL)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_CAP_BYPASS_REG1 = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_CAP_BYPASS_REG1)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_CAP_BYPASS_REG2 = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_CAP_BYPASS_REG2)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_IDLE_CTRL = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_IDLE_CTRL)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_TX_CMD_DLL_CNTL, = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_TX_CMD_DLL_CNTL)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_TX_DATA_DLL_CNTL1 = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_TX_DATA_DLL_CNTL1)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_TX_DATA_DLL_CNTL2 = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_TX_DATA_DLL_CNTL2)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1 = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_RX_STROBE_DLL_CNTL = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_RX_STROBE_DLL_CNTL)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2 = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2)));
DEBUG ((EFI_D_INFO, " R_SCC_MEM_MASTER_DLL_SW_CNTL = 0x%08x\n", *(UINT32 *)(mSdBaseAddress+R_SCC_MEM_MASTER_DLL_SW_CNTL)));

return;
}


/**
  Configure SCS eMMC Clock Source for Rx Path

  @param[in]  PeiSdhostData - PEI_SDHOST_DATA

  @retval  None
**/
VOID
ConfigureEmmcRxClkSrc (
  IN     PEI_SDHOST_DATA       *PeiSdhostData

  )
{
  ///
  ///
  return;

}


/**
  Configure Storage and Communication Subsystems Controller eMMC
  default DLL registers for silicon.

  @param[in] ScsConfig     - The SCS Config Block instance
  @param[in] PeiSdhostData - PEI_SDHOST_DATA

  @retval  None
**/
VOID
ConfigureEmmcDefaultDll (
   IN SC_SCS_CONFIG         *ScsConfig,
   IN PEI_SDHOST_DATA       *PeiSdhostData
  )
{

#ifndef FSP_FLAG
  if (ScsConfig->SccEmmcTraceLength == SCC_EMMC_SHORT_TRACE_LEN) {
#else
  FSPM_UPD *FspmUpd;
  FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();

  if (FspmUpd->FspmConfig.eMMCTraceLen == SCC_EMMC_SHORT_TRACE_LEN) {
#endif
    DEBUG((EFI_D_INFO, "Setting eMMC Dll values for short trace length.\n"));
    ///
    /// Set Tx CMD Delay Control Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_TX_CMD_DLL_CNTL, 0x505);
    ///
    /// Set Tx Data Delay Control 1 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_TX_DATA_DLL_CNTL1, 0xC15);
    ///
    /// Set Tx Data Delay Control 2 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_TX_DATA_DLL_CNTL2, 0x1C1C1C00);
    ///
    /// Set Rx CMD + Data Delay Control 1 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1, 0x1C1C1C00);
    ///
    /// Set Rx Strobe Delay Control Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_RX_STROBE_DLL_CNTL, 0x0A0A);
    ///
    /// Set Rx CMD + Data Delay Control 2 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2, 0x1001C);
    ///
    /// Set Master DLL software Ctrl Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_MASTER_DLL_SW_CNTL, 0x1);
  } else {
    DEBUG((EFI_D_INFO, "Setting eMMC Dll values for long trace length.\n"));
    ///
    /// Set Tx CMD Delay Control Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_TX_CMD_DLL_CNTL, 0x505);
    ///
    /// Set Tx Data Delay Control 1 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_TX_DATA_DLL_CNTL1, 0xC11);
    ///
    /// Set Tx Data Delay Control 2 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_TX_DATA_DLL_CNTL2, 0x1C2A2927);
    ///
    /// Set Rx CMD + Data Delay Control 1 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_RX_CMD_DATA_DLL_CNTL1, 0x000D162F);
    ///
    /// Set Rx Strobe Delay Control Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_RX_STROBE_DLL_CNTL, 0x0A0A);
    ///
    /// Set Rx CMD + Data Delay Control 2 Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_RX_CMD_DATA_DLL_CNTL2, 0x1003b);
    ///
    /// Set Master DLL software Ctrl Register value
    ///
    SdHostWrite32 ( PeiSdhostData, R_SCC_MEM_MASTER_DLL_SW_CNTL, 0x1);
  }
  return;
}

/**
  Entry point for EFI drivers.

  @param[in]  FileHandle  - EFI_PEI_FILE_HANDLE
  @param[in]  PeiServices - EFI_PEI_SERVICES

  @retval  EFI_SUCCESS        - Success
  @retval  EFI_DEVICE_ERROR   - Fail
**/
EFI_STATUS
EFIAPI
SdHostDriverEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS              Status;
  EFI_PHYSICAL_ADDRESS    AllocateAddress;
  PEI_SDHOST_DATA         *PeiSdhostData;
  EFI_PEI_PCI_CFG2_PPI    *PciCfgPpi;
  UINT32                   Data;
  SC_SCS_CONFIG           *ScsConfig = NULL;
#ifndef FSP_FLAG
  SC_POLICY_PPI           *ScPolicyPpi;

  Status = PeiServicesLocatePpi (
              &gScPolicyPpiGuid,
              0,
              NULL,
              (VOID **)&ScPolicyPpi
              );
   ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gScsConfigGuid, (VOID *) &ScsConfig);
   ASSERT_EFI_ERROR (Status);
#endif

  Status = (**PeiServices).AllocatePages (
                             PeiServices,
                             EfiLoaderData,
                             1,
                             &AllocateAddress
                             );

  ASSERT_EFI_ERROR (Status);

  PciCfgPpi = (**PeiServices).PciCfg;

  PeiSdhostData = (PEI_SDHOST_DATA *)((UINTN)AllocateAddress);
  ZeroMem (PeiSdhostData, sizeof(PEI_SDHOST_DATA));

  PeiSdhostData->Signature            = PEI_SDHOST_DATA_SIGNATURE;
  CopyMem(&(PeiSdhostData->SdControllerPpi), &mSdControllerPpi, sizeof(PEI_SD_CONTROLLER_PPI));
  CopyMem(&(PeiSdhostData->PpiList), &mPpiList, sizeof(mPpiList));
  PeiSdhostData->PpiList.Ppi          = &PeiSdhostData->SdControllerPpi;
  PeiSdhostData->PciCfgPpi            = PciCfgPpi;

  //
  // Assign resources and enable controller
  //
  PeiSdhostData->BaseAddress = PCI_LIB_ADDRESS (0, EMMC_DEV_NUMBER, 0, 0);
  Status = EnableSdhcController (PeiSdhostData, 0);

  //
  // Assert if eMMC host controller is not found
  //
  ASSERT_EFI_ERROR(Status);

  //
  // In case ASSERTs are disabled
  //
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "eMMC Host Controller Not Found\n"));
    CpuDeadLoop();
  }

  //
  //Configure Storage and Communication Subsystems Controller eMMC default DLL registers for silicon.
  //
  ConfigureEmmcDefaultDll (ScsConfig, PeiSdhostData);

  //
  //Configure SCS eMMC Clock Source for Rx Path
  //
  ConfigureEmmcRxClkSrc (PeiSdhostData);

  DumpEmmcConvergeLayerRegs();

  PeiSdhostData->SdControllerPpi.ResetSdHost (&PeiSdhostData->SdControllerPpi, Reset_All);
  Data = *(UINT32 *)(mSdBaseAddress+MMIO_CAP);

  DEBUG ((EFI_D_INFO, " MMIO_CAP = 0x%08x\n", Data));

  if ((Data & BIT18) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.BusWidth8 = TRUE;
    DEBUG ((EFI_D_INFO, " BusWidth8\n"));
  }

  if ((Data & BIT21) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.HighSpeedSupport = TRUE;
    DEBUG ((EFI_D_INFO, " HighSpeedSupport\n"));
  }

  if ((Data & BIT24) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.V33Support = TRUE;
    DEBUG ((EFI_D_INFO, " V33Support\n"));
  }

  if ((Data & BIT25) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.V30Support = TRUE;
    DEBUG ((EFI_D_INFO, " V30Support\n"));
  }

  if ((Data & BIT26) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.V18Support = TRUE;
    DEBUG ((EFI_D_INFO, " V18Support\n"));
  }

  PeiSdhostData->SdControllerPpi.HostCapability.BusWidth4 = TRUE;
  DEBUG ((EFI_D_INFO, " BusWidth4\n"));


  //PeiSdhostData->BaseClockInMHz = (Data >> 8) & 0x3F;

  PeiSdhostData->BaseClockInMHz = (Data >> 8) & 0xFF;

  Data = *(UINT32 *)(mSdBaseAddress+MMIO_CAP+4);

  DEBUG ((EFI_D_INFO, "  High MMIO_CAP = 0x%08x\n", Data));


  if ((Data & 0x1<<(32-32)) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.SDR50Support= TRUE;
    DEBUG ((EFI_D_INFO, " SDR50Support\n"));
  }

  if ((Data & 0x1<<(33-32)) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.SDR104Support= TRUE;
    DEBUG ((EFI_D_INFO, " SDR104Support\n"));
  }

  if ((Data & 0x1<<(34-32)) != 0) {
    PeiSdhostData->SdControllerPpi.HostCapability.DDR50Support= TRUE;
    DEBUG ((EFI_D_INFO, " DDR50Support\n"));
  }

  if (PeiSdhostData->ControllerVersion >= 2) {
    PeiSdhostData->SdControllerPpi.HostCapability.ReTuneMode = (Data >> (46-32)) & 0x3;
    DEBUG ((EFI_D_INFO, "  ReTuneMode = 0x%08x\n", PeiSdhostData->SdControllerPpi.HostCapability.ReTuneMode));
    PeiSdhostData->SdControllerPpi.HostCapability.ReTuneTimer = (Data>>(40-32)) & 0xF;
    DEBUG ((EFI_D_INFO, "  ReTuneTimer = 0x%08x\n", PeiSdhostData->SdControllerPpi.HostCapability.ReTuneTimer));
  }

  PeiSdhostData->BlockLength    = BLOCK_SIZE;
  PeiSdhostData->IsAutoStopCmd  = TRUE;
  PeiSdhostData->SdControllerPpi.SetHostVoltage (&PeiSdhostData->SdControllerPpi, 0);
  DEBUG ((EFI_D_INFO, " Copy to gSdHostData\n"));
  CopyMem(&gSdHostData, PeiSdhostData, sizeof(PEI_SDHOST_DATA));

  //
  // Install SD Controller PPI
  //
  Status = (**PeiServices).InstallPpi (
                             PeiServices,
                             &PeiSdhostData->PpiList
                             );
  ASSERT_EFI_ERROR (Status);



  return Status;
}

/**
  Enable Sdhc Controller

  @param[in]  PeiSdhostData  - PEI_SDHOST_DATA
  @param[in]  SdControllerId - UINT8

  @retval  EFI_SUCCESS       - Success
  @retval  EFI_DEVICE_ERROR  - Fail
**/
EFI_STATUS
EnableSdhcController (
  IN PEI_SDHOST_DATA          *PeiSdhostData,
  IN UINT8                    SdControllerId
  )
{
  EFI_PEI_PCI_CFG2_PPI     *PciCfgPpi;
  PCI_CLASSC               PciClass;
  UINT32                   VidDid;
  UINT8                    Data8;
  UINTN                    BaseAddress;
  CONST EFI_PEI_SERVICES   **PeiServices;
//UINT32                   temp;

  PeiServices = GetPeiServicesTablePointer ();

  PciCfgPpi   = PeiSdhostData->PciCfgPpi;
  BaseAddress = PeiSdhostData->BaseAddress;

  VidDid = MmPci32(0,0,EMMC_DEV_NUMBER,0,PCI_VENDOR_ID_OFFSET);

  if (VidDid == 0xFFFFFFFF) {
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((EFI_D_INFO, "SdHostDriver : VidDid = 0x%08x\n", VidDid));
  PeiSdhostData->PciVid = (UINT16)(VidDid & 0xffff);
  PeiSdhostData->PciDid = (UINT16)(VidDid >> 16);
  PeiSdhostData->EnableVerboseDebug = FALSE;  // Enable verbose message
  PeiSdhostData->IsEmmc = FALSE;

  PciClass.BaseCode = MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_CLASSCODE_OFFSET+2);
  PciClass.SubClassCode = MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_CLASSCODE_OFFSET+1);
  PciClass.PI = MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_CLASSCODE_OFFSET);

  if ((PciClass.BaseCode != PCI_CLASS_SYSTEM_PERIPHERAL) ||
      (PciClass.SubClassCode != PCI_SUBCLASS_SD_HOST_CONTROLLER) ||
      ((PciClass.PI != PCI_IF_STANDARD_HOST_NO_DMA) && (PciClass.PI != PCI_IF_STANDARD_HOST_SUPPORT_DMA))
      ) {
    return EFI_UNSUPPORTED;
  }

  //
  // Enable SDHC
  //
  Data8 = MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_COMMAND_OFFSET);
  Data8 &= ~0x06;
  MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_COMMAND_OFFSET) = Data8;

//  MmPci32 (0, 0, EMMC_DEV_NUMBER, 0, PCI_BASE_ADDRESSREG_OFFSET) = 0xFFFFFFFF;
//  temp = MmPci32 (0, 0, EMMC_DEV_NUMBER, 0, PCI_BASE_ADDRESSREG_OFFSET);

  MmPci32(0, 0, EMMC_DEV_NUMBER, 0, PCI_BASE_ADDRESSREG_OFFSET) =   mSdBaseAddress;

  //
  // Enable STATUSCOMMAND BME & MSE
  // Bit 1 : MSE   ;  Bit 2 : BME
  //
  Data8 = MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_COMMAND_OFFSET);
  Data8 |= 0x06;

  MmPci8(0,0,EMMC_DEV_NUMBER,0,PCI_COMMAND_OFFSET) = Data8;

  return EFI_SUCCESS;
}


