/** @file 
PEI Chipset Services.

 This file contains only one function that is PeiCsSvcPlatformStage1Init().
 The function PeiCsSvcPlatformStage1Init() use chipset services to be
 Platform initialization in PEI phase stage 1.

;******************************************************************************
;* Copyright (c) 2013-2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
//
// Libraries
//
//#include <Library/PeiServicesLib.h>
//#include <Library/MemoryAllocationLib.h>
//#include <Library/BaseLib.h>
//#include <Library/IoLib.h>
//#include <Library/BaseMemoryLib.h>
//#include <Library/DebugLib.h>
//#include <Library/PeiServicesTablePointerLib.h>
//#include <Library/HobLib.h>
//#include <Library/BaseCryptLib.h>
////[-start-150424-IB0309APL-add]//
//#include <Library/PreSiliconLib.h>
//#include <Library/ScPlatformLib.h>
////[-end-150424-IB0309APL-add]//
//#include <Library/PmicLib.h>
//#include <Ppi/Stall.h>
//#include <Ppi/TargetTpmSelectDtpm.h>
//#include <Guid/PlatformInfo.h>
//#include <ScAccess.h>
//#include <PlatformBaseAddresses.h>
//#include <ScAccess.h>
//#include <SaRegs.h>
//#include <Ppi/ReadOnlyVariable2.h>
//#include <ChipsetSetupConfig.h>
//#include <Guid/GlobalVariable.h>
//#include <Ppi/fTPMPolicy.h>
//#include <Library/PciLib.h>
//
//#include <SaCommonDefinitions.h>
//#include <SaAccess.h>
//#include <Ppi/SaPolicy.h>
//#include <WPCE791_LpcSio.h>
//#include <Guid/TpmInstance.h>
//#include <Guid/BxtVariable.h>
//#include <Ppi/DramPolicyPpi.h>
//#include "Smip.h"
//
//#define GTT_SIZE_1MB        1
//#define GTT_SIZE_2MB        2
//
//static EFI_PEI_PPI_DESCRIPTOR   mTargetTpmSelectDtpm2Ppi = {
//  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
//  &gTargetTpmSelectDtpmPpiGuid,
//  NULL
//};
//
//extern
//EFI_STATUS
//GetBoardIdFabId(
//  OUT UINT8                     *BoardId,
//  OUT UINT8                     *FabId
//);
//
//EFI_STATUS
//EFIAPI
//PeimfTPMInit (
//  IN EFI_PEI_FILE_HANDLE             *FfsHeader,
//  IN CONST EFI_PEI_SERVICES          **PeiServices
//  );
//
//  
// /**
// This function reads PlatformID and then save to PlatformInfoHob. 
// 
// @param[in]         PlatformInfoHob     HOB that stores platform-related information.
// @param[in]         PeiServices         Describes the list of possible PEI Services.
//                    
// @retval            EFI_SUCCESS         The operation completed successfully.
// @retval            Others              An unexpected error occurred.
//*/
//EFI_STATUS
//ReadPlatformIds (
//  IN CONST EFI_PEI_SERVICES             **PeiServices,
//  IN OUT EFI_PLATFORM_INFO_HOB          *PlatformInfoHob
//  )
//{
//   UINT8  BoardId;
//   UINT8  FABID;
//
//   //Get Board ID
//   BoardId = 0;
//   FABID =0;
//
//  //pre-silicon start
//  if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
//    PmicGpioInit(PlatformInfoHob);    //init PMIC GPIO in order to get boardid/memcfgid/.. etc later.
////?function_not_defined    PlatformInfoHob->BoardId    = PmicGetBoardID();
//    GetBoardIdFabId(&BoardId, &FABID);
//    PlatformInfoHob->BoardId = (UINT16)BoardId;
//    PlatformInfoHob->FABID = (UINT16)FABID;
//    DEBUG((DEBUG_INFO, "BoardId:  [0x%08x]\n", PlatformInfoHob->BoardId));
//    DEBUG((DEBUG_INFO, "FABID:  [0x%08x]\n", PlatformInfoHob->FABID));
////?function_not_defined    PlatformInfoHob->BoardRev   = PmicGetFABID();
//    PlatformInfoHob->BoardRev   = 0;//?until function defined
//    DEBUG((DEBUG_INFO, "BoardRev:  [0x%08x]\n", PlatformInfoHob->BoardRev));
//  } else {
//    PlatformInfoHob->BoardId    = BOARD_ID_BL_RVP;
//    PlatformInfoHob->BoardRev   = 0;
//  }//pre-silicon end
//
//  return EFI_SUCCESS;
//}
//
//
///**
// This function checks if RTC UPDATE IN PROGRESS bit is set. 
// 
// @param[in]         PeiServices         Describes the list of possible PEI Services.
//                    
// @retval            TRUE                Update in progress
// @retval            Others              Update not in progress
//*/
//BOOLEAN
//IsRtcUipAlwaysSet (
//  IN CONST EFI_PEI_SERVICES       **PeiServices
//  )
//{
//  UINTN                 Count;
//  EFI_PEI_STALL_PPI     *StallPpi;
//  EFI_STATUS            Status;
//
//  Status = (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **)&StallPpi);
//  ASSERT_EFI_ERROR (Status);
//
//  for (Count = 0; Count < 500; Count++) { // Maximum waiting approximates to 1.5 seconds (= 3 msec * 500)
//    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERA);
//    if ((IoRead8 (R_RTC_TARGET2) & B_RTC_REGISTERA_UIP) == 0) {
//      return FALSE;
//    }
//
//    StallPpi->Stall (PeiServices, StallPpi, 3000);
//  }
//
//  return TRUE;
//}
//
//
//VOID
//RtcPowerFailureHandler (
//  VOID
//  )
//{
//  UINT16          Data16;
//  UINT8           Data8;
//
//  ///
//  /// CHV BIOS Specification 0.5.0 - Section 29.4.3, "Power Failure Consideration"
//  ///
//  /// When the RTC_PWR_STS bit is set, it indicates that the RTCRST# signal went low.
//  /// Software should clear this bit. For example, changing the RTC battery sets this bit.
//  /// System BIOS should reset CMOS to default values if the RTC_PWR_STS bit is set.
//  /// The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set
//  /// before memory initialization. This will ensure that the RTC state machine has been
//  /// initialized.
//  /// 1. If the RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion or a
//  ///    battery failure, steps 2 through 5 should be executed.
//  /// 2.  Set RTC Register 0x0A[6:4] to 110b or 111b.
//  /// 3.  Set RTC Register 0x0B[7].
//  /// 4.  Set RTC Register 0x0A[6:4] to 010b.
//  /// 5.  Clear RTC Register 0x0B[7].
//  ///
//
//  Data16 = MmioRead16 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
//
//  if ((Data16 & B_PMC_GEN_PMCON_RTC_PWR_STS) != 0) {
//    ///
//    /// 2. Set RTC Register 0Ah[6:4] to 110b or 111b
//    ///
//    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERA);
//    Data8 = IoRead8 (R_RTC_TARGET2) & (UINT8) ~(B_RTC_REGISTERA_DV);
//    Data8 |= (UINT8) (V_RTC_REGISTERA_DV_DIV_RST1);
//    IoWrite8 (R_RTC_TARGET2, Data8);
//
//    ///
//    /// 3. Set RTC Register 0Bh[7].
//    ///
//    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERB);
//    IoOr8 (R_RTC_TARGET2, (UINT8) B_RTC_REGISTERB_SET);
//
//    ///
//    /// 4. Set RTC Register 0Ah[6:4] to 010b
//    ///
//    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERA);
//    Data8 = IoRead8 (R_RTC_TARGET2) & (UINT8) ~(B_RTC_REGISTERA_DV);
//    Data8 |= (UINT8) (V_RTC_REGISTERA_DV_NORM_OP);
//    IoWrite8 (R_RTC_TARGET2, Data8);
//
//    ///
//    /// 5. Clear RTC Register 0Bh[7].
//    ///
//    IoWrite8 (R_RTC_INDEX2, (UINT8) R_RTC_REGISTERB);
//    IoAnd8 (R_RTC_TARGET2, (UINT8) ~B_RTC_REGISTERB_SET);
//  }
//
//  return;
//}
//
//
//VOID
//ScBaseInit (
//  VOID
//  )
//{
//  DEBUG ((DEBUG_INFO, "ScBaseInit() - Start\n"));
//
//  
//  // Set BARs for PMC SSRAM (0/13/3)
//  // Allocation for these regions is done in MemoryCallback.c via call to BuildResourceDescriptorHob()
//  MmioWrite32(
//    MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC_SSRAM, 0x10),  //Write BAR0-lower
//    PcdGet32(PcdPmcSsramBaseAddress0)
//    );
//  MmioWrite32(
//    MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC_SSRAM, 0x18),  //Write BAR1-lower
//    PcdGet32(PcdPmcSsramBaseAddress1)
//    );
//  MmioWrite16(
//    MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC_SSRAM, 0x4),   //Set BME and MSE
//    0x6
//    );
//
//  //
//  // Set SPI Base Address
//  //
//  MmioWrite32 (
//    MmPciAddress (0,
//      DEFAULT_PCI_BUS_NUMBER_SC,
//      PCI_DEVICE_NUMBER_SPI,
//      PCI_FUNCTION_NUMBER_SPI,
//      R_SPI_BASE
//    ),
//    (UINT32)((SPI_BASE_ADDRESS & B_SPI_BASE_BAR))
//  );
//
//  MmioWrite32 (
//    MmPciAddress (0,
//      DEFAULT_PCI_BUS_NUMBER_SC,
//      PCI_DEVICE_NUMBER_SPI,
//      PCI_FUNCTION_NUMBER_SPI,
//      R_SPI_BDE
//    ),
//    0x0000ffff
//  );
//
//  PchLpcIoDecodeRangesSet (
//    (V_PCH_LPC_IOD_LPT_378  << N_PCH_LPC_IOD_LPT)  |
//    (V_PCH_LPC_IOD_COMB_3E8 << N_PCH_LPC_IOD_COMB) |
//    (V_PCH_LPC_IOD_COMA_3F8 << N_PCH_LPC_IOD_COMA)
//    );
//  PchLpcIoEnableDecodingSet (
//    B_PCH_LPC_IOE_ME2  |
//    B_PCH_LPC_IOE_SE   |
//    B_PCH_LPC_IOE_ME1  |
//    B_PCH_LPC_IOE_KE   |
//    B_PCH_LPC_IOE_HGE  |
//    B_PCH_LPC_IOE_LGE  |
//    B_PCH_LPC_IOE_FDE  |
//    B_PCH_LPC_IOE_PPE  |
//    B_PCH_LPC_IOE_CBE  |
//    B_PCH_LPC_IOE_CAE
//    );
//
//  DEBUG ((DEBUG_INFO, "ScBaseInit() - End\n"));
//}
//
//#if defined (ENBDT_PF_ENABLE) && (ENBDT_PF_ENABLE == 1)
//VOID
//WriteSioReg (
//  UINT8     Index,
//  UINT8     Data
//  )
//{
//  IoWrite8 (CONFIG_PORT, Index);
//  IoWrite8 (DATA_PORT, Data);
//}
//
//typedef struct {
//  UINT8 Register;
//  UINT8 Value;
//} EFI_SIO_TABLE;
//
//EFI_SIO_TABLE mSioTable[] = {
//  //
//  // Init keyboard controller
//  //
//  { REG_LOGICAL_DEVICE, SIO_KEYBOARD }, 
//  { BASE_ADDRESS_HIGH, 0x00 }, 
//  { BASE_ADDRESS_LOW, 0x60 }, 
//  { BASE_ADDRESS_HIGH2, 0x00 }, 
//  { BASE_ADDRESS_LOW2, 0x64 },
//  { PRIMARY_INTERRUPT_SELECT, 0x01 }, 
//  { ACTIVATE, 0x1 }, 
//  
//  //
//  // Init Mouse controller
//  //
//  { REG_LOGICAL_DEVICE, SIO_MOUSE }, 
//  { BASE_ADDRESS_HIGH, 0x00 }, 
//  { BASE_ADDRESS_LOW, 0x60 }, 
//  { BASE_ADDRESS_HIGH2, 0x00 }, 
//  { BASE_ADDRESS_LOW2, 0x64 },
//  { PRIMARY_INTERRUPT_SELECT, 0x0c }, 
//  { ACTIVATE, 0x1 }, 
//
//  { REG_LOGICAL_DEVICE, SIO_COM }, 
//  { BASE_ADDRESS_HIGH, 0x03 }, 
//  { BASE_ADDRESS_LOW, 0xf8 }, 
//  { PRIMARY_INTERRUPT_SELECT, 0x04 }, 
//  { ACTIVATE, 0x1 }, 
//  
//};
//
//VOID
//CrbInitSio ()
//{
//  UINTN           Index;
//
//  for (Index = 0; Index < sizeof(mSioTable)/sizeof(EFI_SIO_TABLE); Index++) {
//    WriteSioReg (mSioTable[Index].Register, mSioTable[Index].Value);
//  }
//}
//#endif

/**
 Platform initialization in PEI phase stage 1.

 @param[in]         None

 @retval            EFI_SUCCESS         This function alway return successfully
*/
EFI_STATUS
PlatformStage1Init (
  VOID
  )
{
  EFI_STATUS                      Status;
//  EFI_PLATFORM_INFO_HOB           PlatformInfo;
//  CONST EFI_PEI_SERVICES          **PeiServices;
//  EFI_PEI_READ_ONLY_VARIABLE2_PPI *Variable;
//  UINTN                           DataSize;
//  CHIPSET_CONFIGURATION           *SystemConfiguration;
//  UINT32                          Temp32;
//  UINT32                          Ipc1PciBase;
//  UINT32                          PciCfgReg;
//  UINT32                          PmuBaseAddress;
//  DRAM_POLICY_PPI                 *DramPolicyPpi = NULL;
  
  Status = EFI_SUCCESS;
//  
//  ZeroMem (&PlatformInfo, sizeof(PlatformInfo));
//
//  PeiServices = GetPeiServicesTablePointer ();
// 
//  //
//  // Get SystemConfiguration
//  //
//  Status = (*PeiServices)->LocatePpi (
//                             PeiServices,
//                             &gEfiPeiReadOnlyVariable2PpiGuid,
//                             0,
//                             NULL,
//                             (VOID **)&Variable
//                             );
//  ASSERT_EFI_ERROR (Status);
//
//  //
//  // Since PEI has no PCI enumerator, set the BAR & I/O space enable ourselves
//  //
////[-start-150415-IB0309APL-modify]//
//  ScBaseInit();
////[-end-150415-IB0309APL-modify]//
//
//  //
//  // After ScBaseInit(), Check the PlatformID, if invalid then set to Safe_Warning_Value.
//  // Safe_Warning_Value for PcdIafwPlatformInfo = Real_Silicon + Max_RevId
//  //
//  if (PcdGet8(PcdPlatformIdRegisterOffset) != 0) {
//    Temp32 = MmioRead32( 0xFF03A02C );  //Simics PlatId w/a for BXT
//  } else {
//    Temp32 = MmioRead32( PcdGet32(PcdPmcSsramBaseAddress0) + PcdGet8(PcdPlatformIdRegisterOffset) );
//  }
//  if (Temp32 == 0 || Temp32 == 0xFFFFFFFF) {
//    PcdSet32(PcdIafwPlatformInfo, 0x0000FF00);
//    DEBUG ((DEBUG_INFO, "\nWarning: PcdIafwPlatformInfo set to Safe_Warning_Value\n"));
//  } else {
//    PcdSet32(PcdIafwPlatformInfo, Temp32);
//  }
//  DEBUG ((DEBUG_INFO, "\nPcdIafwPlatformInfo:0x%X  PlatID:0x%X\n", PcdGet32(PcdIafwPlatformInfo), PLATFORM_ID));
//    
//  //
//  // RTC power failure handling
//  //
//  RtcPowerFailureHandler ();
//
//#if ENBDT_PF_ENABLE
//  CrbInitSio ();
//#endif
//
//  // 
//  //512MB of memory was preallocated for IGD
//  //2MB preallocated for support Internal Grapic Traslation Table
//  //
//  MmPci32( 0, 0, 0, 0, 0x50) = 0x2C0;
//
//  //
//  // Initialize PlatformInfo HOB
//  //
//
//  //pre-silicon start
//  if (PLATFORM_ID == VALUE_REAL_PLATFORM) {
//    Status = ReadPlatformIds(PeiServices, &PlatformInfo);
//    ASSERT_EFI_ERROR (Status);
//  }else{
//    PlatformInfo.BoardId = BOARD_ID_AV_SVP;
//  }//pre-silicon end
//
//  //
//  // Build HOB for PlatformInfo
//  //
//  BuildGuidDataHob (
//    &gEfiPlatformInfoGuid,
//    &PlatformInfo,
//    sizeof (EFI_PLATFORM_INFO_HOB)
//    );
//
//
//  //
//  // Attempt to locate SMIP and publish its data to PPI's and PCDs.
//  // Currently no reason to check Status, but could add in future.
//  //
//  Status = SmipInit (PlatformInfo.BoardId);
//
//  //
//  //Todo:add boardID check code in ReadPlatformIds for  (Mineral Hill ) .
//  //
//  //Print out Patch version string (BXT)
//  AsmWriteMsr64 (0x8B, 0);
//  AsmCpuid (0x1, NULL, NULL, NULL, NULL);
//  Temp32 = (UINT32)(AsmReadMsr64(0x8B)>>32);
//  DEBUG ((EFI_D_INFO, "\nPatchInfo:  0x%08x ", Temp32 ));
//  DEBUG ((EFI_D_INFO, "%08x \n", (UINT32)(AsmReadMsr64(0x8B)) ));
//  
////  SeCUmaEntry(NULL, PeiServices); //Done in RC SiInitPrePolicy()
//
//   PmuBaseAddress = PcdGet32(PcdPmcIpc1BaseAddress0);
//   Ipc1PciBase = 0;
//   if (PlatformInfo.BoardId == BOARD_ID_APL_RVP_2A) {
//     PciCfgReg = MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC_IPC1,
//                                           PCI_FUNCTION_NUMBER_PMC_IPC1,
//                                           PCI_COMMAND_OFFSET));
//     // Set PMC base address
//     if ((PciCfgReg & (EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER)) == 0) {
//       Ipc1PciBase = MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC_IPC1,
//                                  PCI_FUNCTION_NUMBER_PMC_IPC1,
//                                  R_PMC_IPC1_BASE));
//
//       MmioWrite32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC_IPC1,
//                                  PCI_FUNCTION_NUMBER_PMC_IPC1,
//                                  R_PMC_IPC1_BASE),
//                    PmuBaseAddress);
//
//       MmioWrite32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC_IPC1,
//                                  PCI_FUNCTION_NUMBER_PMC_IPC1,
//                                  PCI_COMMAND_OFFSET),
//                    EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);
//     }
//
//     //
//     // PMIC base address value refer WhiskeyCove_V1.0.
//     // Section 15.1 Device1 Register Chapter
//     // The register base address 0x4E
//     //
//
//     // Set Power Button Status register
//     PmicWrite8(0x4E, 0x40, 0x55);
//
//     // Set UI Button Status register
//     PmicWrite8(0x4E, 0x41, 0x05);
//
//     // Set PMIC shutdown
//     PmicWrite8(0x4E, 0x43, 0x07);
//
//     // restore PMU base address
//     if ((PciCfgReg & (EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER)) == 0) {
//       MmioWrite32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC_IPC1,
//                                  PCI_FUNCTION_NUMBER_PMC_IPC1,
//                                  PCI_COMMAND_OFFSET),
//                    PciCfgReg);
//
//       MmioWrite32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PMC_IPC1,
//                                  PCI_FUNCTION_NUMBER_PMC_IPC1,
//                                  R_PMC_IPC1_BASE),
//                    Ipc1PciBase);
//     }
//
//   } // end of if (PlatformInfo.BoardId == BOARD_ID_APL_RVP_2A) {
//  
//  //
//  // Initialize FTPMPolicy PPI
//  //
//  if (FeaturePcdGet(FtpmSupport)) {
////    Status = PeimfTPMInit(NULL, PeiServices);
////    ASSERT_EFI_ERROR (Status);
//    if (FeaturePcdGet(PcdRuntimeFtpmDtpmSwitch)) {
//      DataSize = PcdGet32 (PcdSetupConfigSize);
//      SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (DataSize);
//      ASSERT (SystemConfiguration != NULL);
//
//      Status = Variable->GetVariable (
//                         Variable,
//                         SETUP_VARIABLE_NAME,
//                         &gSystemConfigurationGuid,
//                         NULL,
//                         &DataSize,
//                         SystemConfiguration
//                         );
//      ASSERT_EFI_ERROR (Status);
//      if (SystemConfiguration->TargetTPM != TARGET_TPM_FTPM) {
//        Status = PeiServicesInstallPpi (&mTargetTpmSelectDtpm2Ppi);
//      }
//      ASSERT_EFI_ERROR (Status);
//      FreePool (SystemConfiguration);
//    }
//  }
//  
  return Status;
}

