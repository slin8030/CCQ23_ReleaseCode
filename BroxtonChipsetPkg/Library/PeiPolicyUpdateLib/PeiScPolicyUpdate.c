/** @file
  This file is SampleCode of the library for Intel PCH PEI Policy initialization.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

//[-start-151228-IB03090424-modify]//
#include "PeiScPolicyUpdate.h"
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Guid/PlatformEmmcHs400Info.h>
//[-start-160317-IB03090425-modify]//
#include "HdaVerbTables.h"
//[-end-160317-IB03090425-modify]//
//[-start-160301-IB07220042-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
#include <Ppi/EmuPei.h>
#endif
//[-end-161123-IB07250310-modify]//
//[-end-160301-IB07220042-add]//
#include "HdaVerbTables.h"
#include <Guid/PlatformInfo.h>
#include <Library/PeiScPolicyLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-160817-IB03090432-modify]//
#include <Library/BootMediaLib.h>
//[-end-160817-IB03090432-modify]//
#include <SeCAccess.h>
#include <SeCChipset.h>
//[-start-160509-IB03090427-modify]//
#include <Library/BpdtLib.h>
//[-end-160509-IB03090427-modify]//
#include <Library/ConfigBlockLib.h>
//[-start-160413-IB03090426-add]//
#include <SeCState.h>
#include <Library/MmPciLib.h>
//[-end-160413-IB03090426-add]//
//[-start-151123-IB08450329-add]//
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
//[-end-151123-IB08450329-add]//
//[-start-160812-IB11270159-add]//
#include <Library/VariableLib.h>
#include <SecureFlash.h>   //For SecureFlash feature
//[-end-160812-IB11270159-add]//
//[-start-161229-IB04530802-add]//
#include <Guid/PlatformHardwareSwitch.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/PeiChipsetSvcLib.h>
//[-end-161229-IB04530802-add]//
//[-start-160914-IB07400784-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160914-IB07400784-add]//

extern EFI_GUID gEfiBootMediaHobGuid;

//[-start-160519-IB03090427-add]//
#define BPDT_SIGNATURE     0x000055AA
//[-end-160519-IB03090427-add]//

typedef struct {
  UINT32  IbbOffset;
  UINT32  IbbSize;
  UINT32  ObbOffset;
  UINT32  ObbSize;
} IBB_OBB_INFORMATION;

//[-start-160301-IB07220042-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
BOOLEAN  mInRecoveryPei = FALSE;

BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  )
{
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    return mInRecoveryPei;
  } else {
    return FALSE;
  }
}

VOID
FastCrisisRecoveryCheck (
  VOID    
  )
{
  VOID           *EmuPeiPpi;
  EFI_STATUS     Status;
  
  if (FeaturePcdGet (PcdUseFastCrisisRecovery)) {
    Status = PeiServicesLocatePpi (
                    &gEmuPeiPpiGuid,
                    0,
                    NULL,
                    (VOID **) &EmuPeiPpi
                    );
    mInRecoveryPei = EFI_ERROR(Status) ? FALSE : TRUE;
  }
}
#else
BOOLEAN
IsRunCrisisRecoveryMode (
  VOID
  )
{
  if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
    return TRUE;
  }
  return FALSE;
} 
#endif
//[-end-161123-IB07250310-modify]//
//[-end-160301-IB07220042-add]//

//[-start-160812-IB11270159-add]//
BOOLEAN
IsRunSecureFlash (
  VOID
  )
{
  IMAGE_INFO                 ImageInfo;
  UINTN                      Size;
  EFI_STATUS                 Status;
  
  Size = sizeof (ImageInfo);
  ZeroMem (&ImageInfo, sizeof (Size));
  Status = CommonGetVariable (
            SECURE_FLASH_INFORMATION_NAME,
            &gSecureFlashInfoGuid,
            &Size,
            &ImageInfo
            );
  
  if (!(EFI_ERROR(Status)) && (ImageInfo.FlashMode) && !IsRunCrisisRecoveryMode()) {  
    return TRUE;  
  } else {
    return FALSE;
  }
}
//[-end-160812-IB11270159-add]//

//[-start-160317-IB03090425-modify]//
/**
  Add verb table helper function.
  This function calculates verb table number and shows verb table information.

  @param[in,out] VerbTableEntryNum      Input current VerbTable number and output the number after adding new table
  @param[in,out] VerbTableArray         Pointer to array of VerbTable
  @param[in]     VerbTable              VerbTable which is going to add into array
**/
STATIC
VOID
InternalAddVerbTable (
  IN OUT  UINT8                   *VerbTableEntryNum,
  IN OUT  UINT32                  *VerbTableArray,
  IN      HDAUDIO_VERB_TABLE      *VerbTable
  )
{
  if (VerbTable == NULL) {
    DEBUG ((DEBUG_INFO, "InternalAddVerbTable wrong input: VerbTable == NULL\n"));
    return;
  }

  VerbTableArray[*VerbTableEntryNum] = (UINT32) VerbTable;
  *VerbTableEntryNum += 1;

  DEBUG ((DEBUG_INFO,
    "Add verb table for VendorDeviceId = 0x%08X (size = %d DWords)\n",
    VerbTable->VerbTableHeader.VendorDeviceId,
    VerbTable->VerbTableHeader.DataDwords)
    );
  return;
}


STATIC
VOID
InstallPlatformVerbTables (
  IN  SC_HDAUDIO_CONFIG           *HdaConfig,
  IN  UINT16                      BoardId,
  IN  UINTN                       CodecType
  )
{
  UINT8                           VerbTableEntryNum;
  UINT32                          VerbTableArray[32];
  UINT32                          *VerbTablePtr;
//[-start-151123-IB08450329-add]//
  EFI_STATUS                               Status;
  COMMON_CHIPSET_AZALIA_VERB_TABLE         *VerbTableHeaderDataAddress;
  UINT32                                   *VerbTableDataBuffer;
  COMMON_CHIPSET_AZALIA_VERB_TABLE_HEADER  *VerbTableHeaderBuffer;
  HDAUDIO_VERB_TABLE                       *VerbTable;
  UINT8                                    Index;
  UINT16                                   TotalJackNum;
//[-end-151123-IB08450329-add]//

  VerbTableEntryNum = 0;

//[-start-151123-IB08450329-modify]//
  //
  // OemServices
  //
  Status = OemSvcGetVerbTable (
             &VerbTableHeaderDataAddress
             );

  if (Status == EFI_MEDIA_CHANGED) {
    //
    // Install verb table for each entry
    //
    for (Index = 0; VerbTableHeaderDataAddress[Index].VerbTableHeader != NULL; Index++) {
      VerbTableHeaderBuffer = VerbTableHeaderDataAddress[Index].VerbTableHeader;
      TotalJackNum = VerbTableHeaderBuffer->NumberOfFrontJacks + VerbTableHeaderBuffer->NumberOfRearJacks; 
      
      VerbTable = (HDAUDIO_VERB_TABLE *)AllocatePool(sizeof(HDAUDIO_VERB_TABLE) + TotalJackNum * 4 * sizeof(UINT32));
      if (VerbTable == NULL) {
        continue;
      }
      VerbTable->VerbTableHeader.VendorDeviceId = VerbTableHeaderBuffer->VendorDeviceId;  
      VerbTable->VerbTableHeader.RevisionId = VerbTableHeaderBuffer->RevisionId;
      VerbTable->VerbTableHeader.SdiNo = 0xFF;
      VerbTable->VerbTableHeader.DataDwords = TotalJackNum * 4;
      
      VerbTableDataBuffer = VerbTableHeaderDataAddress[Index].VerbTableData;    
      CopyMem (VerbTable->VerbTableData, VerbTableDataBuffer, VerbTable->VerbTableHeader.DataDwords * 4);
      InternalAddVerbTable (&VerbTableEntryNum, VerbTableArray, VerbTable);
    }
  } else if (Status == EFI_UNSUPPORTED) {
//[-end-151123-IB08450329-modify]//
    //
    // @todo: left switch cases defined which can be PlatformInfo or stepping
    //
    if (CodecType == HdaCodecPlatformOnboard) {
//[-start-160406-IB07400715-modify]//
//#if !BXTI_PF_ENABLE
//      InternalAddVerbTable (&VerbTableEntryNum, VerbTableArray, &HdaVerbTableAlc298);
//#else
      switch (BoardId) {
        case BOARD_ID_APL_RVP_1A:
        case BOARD_ID_APL_RVP_2A:
        case BOARD_ID_APL_RVP_1C_LITE:
        case BOARD_ID_MNH_RVP:
          InternalAddVerbTable (&VerbTableEntryNum, VerbTableArray, &HdaVerbTableAlc298);
          break;

        case BOARD_ID_OXH_CRB:
        case BOARD_ID_LFH_CRB:
        case BOARD_ID_JNH_CRB:
          InternalAddVerbTable (&VerbTableEntryNum, VerbTableArray, &HdaVerbTableAlc662);
          break;

        default:
          break;
      }
//#endif
//[-end-160406-IB07400715-modify]//
    } else {
      DEBUG ((DEBUG_INFO, "HD-Audio Warning: External codec kit selected or platform verb table not found, installing all!\n"));
    }
  } else { 
    return;
  }

  HdaConfig->VerbTableEntryNum = VerbTableEntryNum;

  VerbTablePtr = (UINT32 *) AllocateZeroPool (sizeof (UINT32) * VerbTableEntryNum);
  CopyMem (VerbTablePtr, VerbTableArray, sizeof (UINT32) * VerbTableEntryNum);
  HdaConfig->VerbTablePtr = (UINT32) VerbTablePtr;

  return;
}
//[-end-160317-IB03090425-modify]//

//[-start-160817-IB03090432-remove]//
///**
//  Check it's eMMC boot path or not.
//
//  @retval TRUE                       eMMC Boot path
//  @retval FALSE                      Not eMMC boot path
//**/
//BOOLEAN
//IseMMCBoot(
//  VOID
//  )
//{
//#ifndef VP_BIOS_ENABLE
//  VOID                                  *HobList;
//  MBP_CURRENT_BOOT_MEDIA                *BootMediaData;
//  DEBUG ((EFI_D_INFO, "IseMMCBoot Start!\n"));
//  HobList = GetFirstGuidHob (&gEfiBootMediaHobGuid);
//  if (HobList != NULL) {
//    DEBUG ((EFI_D_INFO, "IseMMCBoot HobList != NULL\n"));
//    BootMediaData = GET_GUID_HOB_DATA (HobList);
//    if (BootMediaData->PhysicalData == BOOT_FROM_EMMC) {
//      DEBUG ((EFI_D_INFO, "BootMediaData->PhysicalData ==  IseMMCBoot\n"));
//      return TRUE;
//    } else {
//      DEBUG ((EFI_D_INFO, "Not boot from eMMC\n"));
//      return FALSE;
//    }
//  }
//  return FALSE;
//#else
//  return TRUE;
//#endif
//}
//
///**
//  Check it's SPI boot path or not.
//
//  @retval TRUE                       SPI Boot path
//  @retval FALSE                      Not SPI boot path
//**/
//BOOLEAN
//IsSpiBoot(
//  VOID
//  )
//{
//  VOID                                  *HobList;
//  MBP_CURRENT_BOOT_MEDIA                *BootMediaData;
//  DEBUG ((EFI_D_INFO, "IsSpiBoot Start!\n"));
//  HobList = GetFirstGuidHob (&gEfiBootMediaHobGuid);
//  if (HobList != NULL) {
//    DEBUG ((EFI_D_INFO, "IsSpiBoot HobList != NULL\n"));
//    BootMediaData = GET_GUID_HOB_DATA (HobList);
//    if (BootMediaData->PhysicalData == BOOT_FROM_SPI) {
//      DEBUG ((EFI_D_INFO, "BootMediaData->PhysicalData ==  IsSpiBoot\n"));
//      return TRUE;
//    } else {
//      DEBUG ((EFI_D_INFO, "Not boot from SPI\n"));
//      return FALSE;
//    }
//  }
//  return FALSE;
//}
//[-end-160817-IB03090432-remove]//

/*++

Routine Description:
  Calculate the Address of Boot Partition 1

Arguments:
  Address             The address

Returns:

  EFI_SUCCESS         The operation completed successfully.
  EFI_DEVICE_ERROR


--*/

EFI_STATUS
EFIAPI
FindBootPartition1 (
  OUT UINT32                    *Address
  )
{
  UINT32  SecondBPFlashLinearAddress;
  UINT32  BiosAddr;

  //
  //Compute Second BP FlashLinearAddress
  //
  SecondBPFlashLinearAddress = 0x1000;

  DEBUG ((DEBUG_INFO, "SecondBPFlashLinearAddress = %X\n", SecondBPFlashLinearAddress));
  //
  // Calculate Boot partition #2 physical address
  // FLA[26:0] <= (Flash_Regionn_Limit) - (FFFF_FFFCh - bios_address)
  //
  BiosAddr = GetSpiFlashRegionLimit (BIOS_Region) + 0xFFC - SecondBPFlashLinearAddress;
  *Address = 0xFFFFFFFC - BiosAddr;

  DEBUG ((DEBUG_INFO, "system BP1 Address = %X\n", *Address));

  return EFI_SUCCESS;
}

/*++

Routine Description:
  Calculate the Address of Boot Partition 2

Arguments:
  Address             The address

Returns:

  EFI_SUCCESS         The operation completed successfully.
  EFI_DEVICE_ERROR

--*/
EFI_STATUS
EFIAPI
FindBootPartition2 (
  OUT UINT32                    *Address
  )
{
  UINT32  SecondBPFlashLinearAddress;
  UINT32  BiosAddr;

  //
  // Compute Second BP FlashLinearAddress
  //
  if (HeciPciRead16(R_SEC_DevID_VID) != 0xFFFF) {
    //
    // BP2 linear address is the midpoint between BIOS base and expansion data base
    //
    SecondBPFlashLinearAddress = (GetSpiFlashRegionBase (BIOS_Region) + GetSpiFlashRegionBase (DeviceExpansion1)) / 2;
  } else {
    //
    // W/A for non-secure boot
    //
    SecondBPFlashLinearAddress = FixedPcdGet32 (PcdSpiFlashSize) >> 1;
  }

  DEBUG ((DEBUG_INFO, "SecondBPFlashLinearAddress = %X\n", SecondBPFlashLinearAddress));
  //
  // Calculate Boot partition #2 physical address
  // FLA[26:0] <= (Flash_Regionn_Limit) - (FFFF_FFFCh - bios_address)
  //
  BiosAddr = GetSpiFlashRegionLimit (BIOS_Region) + 0xFFC - SecondBPFlashLinearAddress;
  *Address = 0xFFFFFFFC - BiosAddr;

  DEBUG ((DEBUG_INFO, "FlashRegionLimit = %X\n", *Address));

  return EFI_SUCCESS;
}

/**
  This function performs PCH PEI Policy initialization.

  @param[in, out] ScPolicy       The PCH Policy PPI instance

  @retval EFI_SUCCESS             The PPI is installed and initialized.
  @retval EFI ERRORS              The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
UpdatePeiScPolicy (
  IN OUT SC_POLICY_PPI *ScPolicyPpi
  )
{
  EFI_STATUS                 Status;
  EFI_PEI_HOB_POINTERS       Hob;
  EFI_PLATFORM_INFO_HOB      *PlatformInfo;
//[-start-160803-IB07220122-add]//
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *VariableServices;
//[-end-160803-IB07220122-add]//
  UINT16                     BoardId;
  UINTN                      VariableSize;
  CHIPSET_CONFIGURATION      SystemConfiguration;
  UINT8                      Index;
  UINT8                      PortIndex;
//[-start-160509-IB03090427-modify]//
  BPDT_HEADER                *BpdtHeader;
  BPDT_HEADER                *sBpdtHeader;
  BPDT_ENTRY                 *BpdtEntryPtr;
  VOID                       *FvBuffer;
  IBB_OBB_INFORMATION        IbbObbInformation = {0};
  UINT32                     Offset;
//[-end-160509-IB03090427-modify]//
  UINT32                     SpiHsfsReg;
  UINT32                     SpiFdodReg;
  UINT8                      DevIndex;
  UINT8                      HdaIndex;
  BOOLEAN                    FlashProtectionEnabled;
  SC_GENERAL_CONFIG          *GeneralConfig;
  SC_SATA_CONFIG             *SataConfig;
  SC_PCIE_CONFIG             *PcieConfig;
  SC_USB_CONFIG              *UsbConfig;
  SC_HPET_CONFIG             *HpetConfig;
  SC_IOAPIC_CONFIG           *IoApicConfig;
  SC_HDAUDIO_CONFIG          *HdaConfig;
  SC_GMM_CONFIG              *GmmConfig;
  SC_PM_CONFIG               *PmConfig;
  SC_LOCK_DOWN_CONFIG        *LockDownConfig;
  SC_LPSS_CONFIG             *LpssConfig;
  SC_SCS_CONFIG              *ScsConfig;
  SC_VTD_CONFIG              *VtdConfig;
  SC_ISH_CONFIG              *IshConfig;
  SC_FLASH_PROTECTION_CONFIG *FlashProtectionConfig;
//[-start-160413-IB03090426-add]//
  SC_DCI_CONFIG              *DciConfig;
//[-start-160509-IB03090427-remove]//
//  UINTN                       HeciBaseAddress;
//  UINT32                      SecMode;
//[-end-160509-IB03090427-remove]//
//[-end-160413-IB03090426-add]//
//[-start-160429-IB10860197-add]//
  UINT32                     BootPartitionSize;
  UINT32                     BP1MemAddress;
  UINT32                     BP2MemAddress;
  UINT32                     IBBSpiAddress;
  UINT32                     OBBSpiAddress;
//[-end-160429-IB10860197-add]//
//[-start-161227-IB4530802-add]//
  PLATFORM_HARDWARE_SWITCH   *PlatformHardwareSwitch;
//[-end-161227-IB04530802-add]//
//[-start-170222-IB07400843-add]//
  SC_LPC_SIRQ_CONFIG         *SerialIrqConfig;
//[-end-170222-IB07400843-add]//


//[-start-160301-IB07220042-add]//
//[-start-161123-IB07250310-modify]//
#ifndef FSP_WRAPPER_FLAG
  FastCrisisRecoveryCheck ();
#endif
//[-end-161123-IB07250310-modify]//
//[-end-160301-IB07220042-add]//

  //
  // @todo: The PlatformInfoHob is default at this point, which is initialized after memory installed in PlatformInit.c
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  BoardId = PlatformInfo->BoardId;

//[-start-161227-IB04530802-add]//
  //
  // Find Platform Hardware switch Hob
  //
  PlatformHardwareSwitch = NULL;
  Hob.Raw = GetHobList ();
  Hob.Raw = GetNextGuidHob (&gPlatformHardwareSwitchGuid, Hob.Raw);
  if (Hob.Raw == NULL) {
    DEBUG ((EFI_D_ERROR, "Platform Hardware switch Hob not found!!!)\n"));
    return EFI_NOT_FOUND;
  }
  PlatformHardwareSwitch = (PLATFORM_HARDWARE_SWITCH *)GET_GUID_HOB_DATA (Hob.Guid);
//[-end-161227-IB04530802-add]//

  //
  // Retrieve Setup variable
  //
//[-start-160803-IB07220122-add]//
  Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &VariableServices);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
//[-end-160803-IB07220122-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//
  ASSERT_EFI_ERROR (Status);

  //
  // Install SC Policy PPI. As we depend on SC Init PPI so we are executed after
  // ScInit PEIM. Thus we can insure SC Initialization is performed when we install the SC Policy PPI,
  // as ScInit PEIM registered a notification function on our policy PPI.
  //
  // For better code structure / modularity, we should use a notification function on SC Init PPI to perform
  // actions that depend on ScInit PEIM's initialization.
  //
  DEBUG ((DEBUG_INFO, "UpdatePeiScPolicy() - Start\n"));
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gScGeneralConfigGuid, (VOID *) &GeneralConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gSataConfigGuid, (VOID *) &SataConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gHpetConfigGuid, (VOID *) &HpetConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gIoApicConfigGuid, (VOID *) &IoApicConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gHdAudioConfigGuid, (VOID *) &HdaConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gGmmConfigGuid, (VOID *) &GmmConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gLockDownConfigGuid, (VOID *) &LockDownConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gLpssConfigGuid, (VOID *) &LpssConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gScsConfigGuid, (VOID *) &ScsConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gVtdConfigGuid, (VOID *) &VtdConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gIshConfigGuid, (VOID *) &IshConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gFlashProtectionConfigGuid, (VOID *) &FlashProtectionConfig);
  ASSERT_EFI_ERROR (Status);
//[-start-160413-IB03090426-add]//
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gDciConfigGuid, (VOID *) &DciConfig);
  ASSERT_EFI_ERROR (Status);
//[-end-160413-IB03090426-add]//
//[-start-170222-IB07400843-add]//
  Status = GetConfigBlock ((VOID *) ScPolicyPpi, &gSerialIrqConfigGuid, (VOID *) &SerialIrqConfig);
  ASSERT_EFI_ERROR (Status);
//[-end-170222-IB07400843-add]//

  //
  // Read ACPI and P2SB Base Addresses
  //
//[-start-160801-IB03090430-modify]//
  GeneralConfig->PmcBase  = (UINT32)PcdGet32(PcdPmcIpc1BaseAddress0);
//[-end-160801-IB03090430-modify]//
  GeneralConfig->AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  GeneralConfig->P2sbBase = (UINT32)PcdGet32(PcdP2SBBaseAddress);
  GeneralConfig->Crid     = SystemConfiguration.CRIDSettings;
  GeneralConfig->ResetSelect = SystemConfiguration.ResetSelect;

  HpetConfig->Enable           = SystemConfiguration.Hpet;
  HpetConfig->Base             = HPET_BASE_ADDRESS;
  HpetConfig->BdfValid         = 0x01;
  HpetConfig->BusNumber        = 0xFA;
  HpetConfig->DeviceNumber     = 0x0F;
  HpetConfig->FunctionNumber   = 0;
  IoApicConfig->IoApicId       = 0x01;
  IoApicConfig->BdfValid       = 1;
  IoApicConfig->BusNumber      = 0xFA;
  IoApicConfig->DeviceNumber   = 0x1F;
  IoApicConfig->FunctionNumber = 0;

  SpiHsfsReg = MmioRead32 (SPI_BASE_ADDRESS + R_SPI_HSFS);
  if ((SpiHsfsReg & B_SPI_HSFS_FDV) == B_SPI_HSFS_FDV) {
    MmioWrite32 (SPI_BASE_ADDRESS + R_SPI_FDOC, V_SPI_FDOC_FDSS_FSDM);
    SpiFdodReg = MmioRead32 (SPI_BASE_ADDRESS + R_SPI_FDOD);
    if (SpiFdodReg == V_SPI_FDBAR_FLVALSIG) {
    }
  }

  //
  // Update PCIe config
  //
  PcieConfig->DisableRootPortClockGating   = SystemConfiguration.PcieClockGatingDisabled;
  PcieConfig->EnablePort8xhDecode          = SystemConfiguration.PcieRootPort8xhDecode;
//[-start-170321-IB07400848-modify]//
  PcieConfig->ScPciePort8xhDecodePortIndex = SystemConfiguration.Pcie8xhDecodePortIndex - 1;
//[-end-170321-IB07400848-modify]//
  PcieConfig->EnablePeerMemoryWrite        = SystemConfiguration.PcieRootPortPeerMemoryWriteEnable;
//[-start-160317-IB03090425-add]//
  PcieConfig->ComplianceTestMode           = SystemConfiguration.PcieComplianceMode;
//[-end-160317-IB03090425-add]//
  for (PortIndex = 0; PortIndex < SC_MAX_PCIE_ROOT_PORTS; PortIndex++) {
//[-start-161228-IB04530802-modify]//
//    PcieConfig->RootPort[PortIndex].Enable                        = SystemConfiguration.PcieRootPortEn[PortIndex];
    PcieConfig->RootPort[PortIndex].Enable                        = (PlatformHardwareSwitch->PciePortSwitch.CONFIG >> (PortIndex*2)) & 0x3;
//[-end-161228-IB04530802-modify]//
//[-start-160317-IB03090425-modify]//
    PcieConfig->RootPort[PortIndex].SlotImplemented               = TRUE;
//[-end-160317-IB03090425-modify]//
    PcieConfig->RootPort[PortIndex].PhysicalSlotNumber            = (UINT8) PortIndex;
    PcieConfig->RootPort[PortIndex].Aspm                          = SystemConfiguration.PcieRootPortAspm[PortIndex];
    PcieConfig->RootPort[PortIndex].L1Substates                   = SystemConfiguration.PcieRootPortL1SubStates[PortIndex];
    PcieConfig->RootPort[PortIndex].AcsEnabled                    = SystemConfiguration.PcieRootPortACS[PortIndex];
    PcieConfig->RootPort[PortIndex].PmSci                         = SystemConfiguration.PcieRootPortPMCE[PortIndex];
    PcieConfig->RootPort[PortIndex].HotPlug                       = SystemConfiguration.PcieRootPortHPE[PortIndex];
    PcieConfig->RootPort[PortIndex].AdvancedErrorReporting        = FALSE;
    PcieConfig->RootPort[PortIndex].UnsupportedRequestReport      = SystemConfiguration.PcieRootPortURE[PortIndex];
    PcieConfig->RootPort[PortIndex].FatalErrorReport              = SystemConfiguration.PcieRootPortFEE[PortIndex];
    PcieConfig->RootPort[PortIndex].NoFatalErrorReport            = SystemConfiguration.PcieRootPortNFE[PortIndex];
    PcieConfig->RootPort[PortIndex].CorrectableErrorReport        = SystemConfiguration.PcieRootPortCEE[PortIndex];
    PcieConfig->RootPort[PortIndex].PmeInterrupt                  = 0;
    PcieConfig->RootPort[PortIndex].SystemErrorOnFatalError       = SystemConfiguration.PcieRootPortSFE[PortIndex];
    PcieConfig->RootPort[PortIndex].SystemErrorOnNonFatalError    = SystemConfiguration.PcieRootPortSNE[PortIndex];
    PcieConfig->RootPort[PortIndex].SystemErrorOnCorrectableError = SystemConfiguration.PcieRootPortSCE[PortIndex];
    PcieConfig->RootPort[PortIndex].TransmitterHalfSwing          = SystemConfiguration.PcieRootPortTHS[PortIndex];
    PcieConfig->RootPort[PortIndex].CompletionTimeout             = ScPcieCompletionTO_Default;
    PcieConfig->RootPort[PortIndex].PcieSpeed                     = SystemConfiguration.PcieRootPortSpeed[PortIndex];
//[-start-160509-IB03090427-add]//
    PcieConfig->RootPort[PortIndex].SelectableDeemphasis          = SystemConfiguration.PcieRootPortSelectableDeemphasis[PortIndex];
//[-end-160509-IB03090427-add]//
    // LTR settings
    PcieConfig->RootPort[PortIndex].LtrEnable                         = SystemConfiguration.PchPcieLtrEnable[PortIndex];
//[-start-160801-IB03090430-modify]//
    if (SystemConfiguration.PchPcieLtrEnable[PortIndex] == TRUE) {
      PcieConfig->RootPort[PortIndex].LtrConfigLock                     = SystemConfiguration.PchPcieLtrConfigLock[PortIndex];
      PcieConfig->RootPort[PortIndex].LtrMaxSnoopLatency                = 0x1003;
      PcieConfig->RootPort[PortIndex].LtrMaxNoSnoopLatency              = 0x1003;
      PcieConfig->RootPort[PortIndex].SnoopLatencyOverrideMode          = SystemConfiguration.PchPcieSnoopLatencyOverrideMode[PortIndex];
      PcieConfig->RootPort[PortIndex].SnoopLatencyOverrideMultiplier    = SystemConfiguration.PchPcieSnoopLatencyOverrideMultiplier[PortIndex];
      PcieConfig->RootPort[PortIndex].SnoopLatencyOverrideValue         = SystemConfiguration.PchPcieSnoopLatencyOverrideValue[PortIndex];
      PcieConfig->RootPort[PortIndex].NonSnoopLatencyOverrideMode       = SystemConfiguration.PchPcieNonSnoopLatencyOverrideMode[PortIndex];
      PcieConfig->RootPort[PortIndex].NonSnoopLatencyOverrideMultiplier = SystemConfiguration.PchPcieNonSnoopLatencyOverrideMultiplier[PortIndex];
      PcieConfig->RootPort[PortIndex].NonSnoopLatencyOverrideValue      = SystemConfiguration.PchPcieNonSnoopLatencyOverrideValue[PortIndex];
    }
//[-end-160801-IB03090430-modify]//
//[-start-161022-IB07400803-modify]//
//#if BXTI_PF_ENABLE
    if (IsIOTGBoardIds()) {
      PcieConfig->RootPort[PortIndex].PtmEnable = TRUE;
    }
//#endif
//[-end-161022-IB07400803-modify]//
  }
//[-start-160317-IB03090425-modify]//
#if (ENBDT_PF_ENABLE == 1)
  //
  // ApolloLake RVP PCI-E HW port map:
  //
  // PCIE Lane 0 (0/0x13/0, Port 2) -> x4 Slot (Slot 1), ClkReq = 0, PCIE_WAKE0_N
  // PCIE Lane 1 (0/0x13/1, Port 3) -> x4 Slot (Slot 1)
  // PCIE Lane 2 (0/0x13/2, Port 4) -> i211,             ClkReq = 1, PCIE_WAKE1_N
  // PCIE Lane 3 (0/0x13/3, Port 5) -> x
  // PCIE Lane 4 (0/0x14/0, Port 0) -> x4 Slot (Slot 2), ClkReq = 2, PCIE_WAKE2_N
  // PCIE Lane 5 (0/0x14/1, Port 1) -> M.2 WLAN,         ClkReq = 3, PCIE_WAKE3_N
  //
  // for PCIE x2 controller 4~5
  PcieConfig->RootPort[0].ClkReqSupported = TRUE;
  PcieConfig->RootPort[0].ClkReqNumber    = 2;
  PcieConfig->RootPort[1].ClkReqSupported = TRUE;
  PcieConfig->RootPort[1].ClkReqNumber    = 3;
  // for PCIE x4 controller 0~3
  PcieConfig->RootPort[2].ClkReqSupported = TRUE;
  PcieConfig->RootPort[2].ClkReqNumber    = 0;
  PcieConfig->RootPort[3].ClkReqSupported = FALSE;
  PcieConfig->RootPort[3].ClkReqNumber    = 0xF;
  PcieConfig->RootPort[4].ClkReqSupported = TRUE;
  PcieConfig->RootPort[4].ClkReqNumber    = 0x1;
  PcieConfig->RootPort[5].ClkReqSupported = FALSE;
  PcieConfig->RootPort[5].ClkReqNumber    = 0xF;
#endif
//[-start-160406-IB07400715-modify]//
//#if (BXTI_PF_ENABLE == 1)
  if ((PlatformInfo->BoardId == BOARD_ID_OXH_CRB) || (PlatformInfo->BoardId == BOARD_ID_LFH_CRB) || (PlatformInfo->BoardId == BOARD_ID_JNH_CRB)) {
    //
    // ApolloLake-I OxbowHill PCI-E HW port map:
    //
    // PCIE Lane 0 (0/0x13/0, Port 2) => x4 slot (x2),        ClkReq = 1, PCIE_WAKE1_N
    // PCIE Lane 1 (0/0x13/1, Port 3) => x4 slot (x2)
    // PCIE Lane 2 (0/0x13/2, Port 4) => LAN,                 ClkReq = 0, PCIE_WAKE0_N
    // PCIE Lane 3 (0/0x13/3, Port 5) => x
    // PCIE Lane 4 (0/0x14/0, Port 0) => M.2 3G,              ClkReq = 2, PCIE_WAKE2_N 
    // PCIE Lane 5 (0/0x14/1, Port 1) => M.2 Wifi/Bluetooth,  ClkReq = 3, PCIE_WAKE3_N 
    //
    
    // for PCIE x2 controller port 4~5
    PcieConfig->RootPort[0].ClkReqSupported = TRUE;
    PcieConfig->RootPort[0].ClkReqNumber    = 2;
    PcieConfig->RootPort[1].ClkReqSupported = TRUE;
    PcieConfig->RootPort[1].ClkReqNumber    = 3;
    // for PCIE x4 controller port 0~3
    PcieConfig->RootPort[2].ClkReqSupported = TRUE;
    PcieConfig->RootPort[2].ClkReqNumber    = 1;
    PcieConfig->RootPort[3].ClkReqSupported = FALSE;
    PcieConfig->RootPort[3].ClkReqNumber    = 0xF;
    PcieConfig->RootPort[4].ClkReqSupported = TRUE;
    PcieConfig->RootPort[4].ClkReqNumber    = 0;
    PcieConfig->RootPort[5].ClkReqSupported = FALSE;
    PcieConfig->RootPort[5].ClkReqNumber    = 0xF;
//[-start-160817-IB03090432-add]//
//[-start-160914-IB07400784-remove]//
    //
    // Default already set to Auto
    //
//    if (SystemConfiguration.PlatformSettingEn == 0) {
//      for (PortIndex = 0; PortIndex < SC_MAX_PCIE_ROOT_PORTS; PortIndex++) {
//        PcieConfig->RootPort[PortIndex].Enable  = DEVICE_AUTO;
//      }
//    }
//[-end-160914-IB07400784-remove]//
//[-end-160817-IB03090432-add]//
  }
//#endif
//[-end-160406-IB07400715-modify]//
//[-end-160317-IB03090425-modify]//
  //
  // Update SATA config
  //
  SataConfig->Enable   = SystemConfiguration.Sata;
  SataConfig->SataMode = SystemConfiguration.SataInterfaceMode;
//[-start-160429-IB07400722-add]//
  SataConfig->SpeedLimit = SystemConfiguration.SATASpeed;
//[-end-160429-IB07400722-add]//
  for (PortIndex = 0; PortIndex < SC_MAX_SATA_PORTS; PortIndex++) {
    if (SystemConfiguration.SataTestMode == TRUE) {
      SataConfig->PortSettings[PortIndex].Enable = TRUE;
    } else {
//[-start-161227-IB04530802-modify]//
//      SataConfig->PortSettings[PortIndex].Enable = SystemConfiguration.SataPort[PortIndex];
      SataConfig->PortSettings[PortIndex].Enable = (PlatformHardwareSwitch->SataPortSwitch.CONFIG >> PortIndex) & 0x1;
//[-end-161227-IB04530802-modify]//
    }
    SataConfig->PortSettings[PortIndex].HotPlug          = SystemConfiguration.SataHotPlug[PortIndex];
    SataConfig->PortSettings[PortIndex].SpinUp           = SystemConfiguration.SataSpinUp[PortIndex];
    SataConfig->PortSettings[PortIndex].External         = FALSE;
    SataConfig->PortSettings[PortIndex].DevSlp           = SystemConfiguration.PxDevSlp[PortIndex];
    SataConfig->PortSettings[PortIndex].EnableDitoConfig = SystemConfiguration.EnableDitoConfig[PortIndex];
    SataConfig->PortSettings[PortIndex].DmVal            = SystemConfiguration.DmVal[PortIndex];
    SataConfig->PortSettings[PortIndex].DitoVal          = SystemConfiguration.DitoVal[PortIndex];
    SataConfig->PortSettings[PortIndex].SolidStateDrive  = SystemConfiguration.SataType[PortIndex];
  }
  SataConfig->SalpSupport = SystemConfiguration.SataSalp;
  SataConfig->TestMode    = SystemConfiguration.SataTestMode;

  //
  // Flash Security Recommendation,
  // Intel strongly recommends that BIOS sets the BIOS Interface Lock Down bit. Enabling this bit
  // will mitigate malicious software attempts to replace the system BIOS option ROM with its own code.
  //BIOS lock down bit is enabled by default as per Platform policy, except that when system is in recovery mode or FDO is enabled. In this case this will be disabled as part of Firmware Update / Recovery update
  //

//[-start-160509-IB03090427-modify]//
  if ((GetBxtSeries() == BxtP)&& ((FALSE == PcdGetBool(PcdFDOState)))) {
    LockDownConfig->BiosInterface = TRUE;
//[-start-160301-IB07220042-modify]//
//[-start-161012-IB07220142-modify]//
    LockDownConfig->BiosLock      = (GetBootModeHob() == BOOT_IN_RECOVERY_MODE) ? FALSE : SystemConfiguration.ScBiosLock;
//[-end-161012-IB07220142-modify]//
//[-start-160719-IB03090429-modify]//
//[-start-161012-IB07220142-modify]//
    if (SystemConfiguration.ScBiosLock && (GetBootModeHob() != BOOT_IN_RECOVERY_MODE)) {
      LockDownConfig->SpiEiss     = TRUE;
    } else {
      LockDownConfig->SpiEiss     = FALSE;
    }
//[-end-161012-IB07220142-modify]//
//[-end-160719-IB03090429-modify]//
//[-end-160301-IB07220042-modify]//
  } else {
    DEBUG ((DEBUG_INFO, " BIOS lock is not done  -CSE Status -FDO ASSERTED\n"));
    LockDownConfig->BiosInterface = FALSE;
    LockDownConfig->BiosLock      = FALSE;
    LockDownConfig->SpiEiss       = FALSE;
  }
  LockDownConfig->RtcLock       = SystemConfiguration.RtcLock;
//[-start-160719-IB03090429-modify]//
  LockDownConfig->TcoLock         = SystemConfiguration.TcoLock;
//[-end-160719-IB03090429-modify]//

  HdaConfig->Enable               = SystemConfiguration.ScHdAudio;
  HdaConfig->DspEnable            = SystemConfiguration.ScHdAudioDsp;
//[-end-160509-IB03090427-modify]//
//[-start-161018-IB06740518-add]//
  HdaConfig->DspUaaCompliance     = SystemConfiguration.HdAudioDspUaaCompliance;
//[-end-161018-IB06740518-add]//
  HdaConfig->Mmt                  = SystemConfiguration.ScHdAudioMmt;
  HdaConfig->Hmt                  = SystemConfiguration.ScHdAudioHmt;
  HdaConfig->IoBufferOwnership    = SystemConfiguration.ScHdAudioIoBufferOwnership;
  HdaConfig->BiosCfgLockDown      = SystemConfiguration.ScHdAudioBiosCfgLockDown;
  HdaConfig->PwrGate              = SystemConfiguration.ScHdAudioPwrGate;
  HdaConfig->ClkGate              = SystemConfiguration.ScHdAudioClkGate;
  HdaConfig->Pme                  = SystemConfiguration.ScHdAudioPme;

//[-start-161018-IB06740518-modify]//
  if (HdaConfig->DspUaaCompliance) {
    HdaConfig->DspEndpointDmic      = FALSE;
    HdaConfig->DspEndpointBluetooth = FALSE;
  } else {
    HdaConfig->DspEndpointDmic      = SystemConfiguration.ScHdAudioNhltEndpointDmic;
    HdaConfig->DspEndpointBluetooth = SystemConfiguration.ScHdAudioNhltEndpointBt;
  }

  if ((HdaConfig->IoBufferOwnership == ScHdaIoBufOwnerHdaLink) || (HdaConfig->DspUaaCompliance)) {
    HdaConfig->DspEndpointI2sSkp = FALSE;
    HdaConfig->DspEndpointI2sHp  = FALSE;
  } else {
    HdaConfig->DspEndpointI2sSkp   = SystemConfiguration.ScHdAudioNhltEndpointI2sSKP;
    HdaConfig->DspEndpointI2sHp    = SystemConfiguration.ScHdAudioNhltEndpointI2sHP;
  }
//[-end-161018-IB06740518-modify]//
  HdaConfig->VcType                = SystemConfiguration.SvHdaVcType;
  HdaConfig->HdAudioLinkFrequency  = SystemConfiguration.HdAudioLinkFrequency;
  HdaConfig->IDispLinkFrequency    = SystemConfiguration.IDispLinkFrequency;
  HdaConfig->IDispLinkTmode        = SystemConfiguration.IDispLinkTmode;
  for(HdaIndex = 0; HdaIndex < HDAUDIO_FEATURES; HdaIndex++) {
    HdaConfig->DspFeatureMask |= (UINT32)(SystemConfiguration.ScHdAudioFeature[HdaIndex] ? (1 << HdaIndex) : 0);
  }
  for(HdaIndex = 0; HdaIndex < HDAUDIO_PP_MODULES; HdaIndex++) {
    HdaConfig->DspPpModuleMask |= (UINT32)(SystemConfiguration.ScHdAudioPostProcessingMod[HdaIndex] ? (1 << HdaIndex) : 0);
  }
  HdaConfig->ResetWaitTimer = 300;
//[-start-161022-IB07400803-modify]//
//#if (BXTI_PF_ENABLE == 1)
  if (IsIOTGBoardIds()) {
    if (HdaConfig->DspEndpointDmic == FALSE && HdaConfig->DspEndpointBluetooth == FALSE) {
      HdaConfig->I2sConfiguration = 2;
    } else {
      HdaConfig->I2sConfiguration = 1;
    }
  }
//#endif
//[-end-161022-IB07400803-modify]//

//[-start-160317-IB03090425-add]//
#if (TABLET_PF_ENABLE == 0)
  //
  // Install Verb Table
  //
  if (SystemConfiguration.ScHdAudio) {
    //
    // @todo: set default to on board
    //
    InstallPlatformVerbTables (HdaConfig, BoardId, HdaCodecPlatformOnboard);
  }
#endif
//[-end-160317-IB03090425-add]//

  //
  // Update GMM config
  //
  GmmConfig->Enable                  = SystemConfiguration.Gmm;
  GmmConfig->ClkGatingPgcbClkTrunk   = SystemConfiguration.GmmCgPGCBEnabled;
  GmmConfig->ClkGatingSb             = SystemConfiguration.GmmCgSBDEnabled;
  GmmConfig->ClkGatingSbClkTrunk     = SystemConfiguration.GmmCgSBTEnabled;
  GmmConfig->ClkGatingSbClkPartition = SystemConfiguration.GmmCgSBPEnabled;
  GmmConfig->ClkGatingCore           = SystemConfiguration.GmmCgCoreEnabled;
  GmmConfig->ClkGatingDma            = SystemConfiguration.GmmCgDmaEnabled;
  GmmConfig->ClkGatingRegAccess      = SystemConfiguration.GmmCgRAEnabled;
  GmmConfig->ClkGatingHost           = SystemConfiguration.GmmCgHostEnabled;
  GmmConfig->ClkGatingPartition      = SystemConfiguration.GmmCgPEnabled;
  GmmConfig->ClkGatingTrunk          = SystemConfiguration.GmmCgTEnabled;
  if (SystemConfiguration.GmmPgHwAutoEnabled) {
    GmmConfig->SvPwrGatingHwAutoEnable = TRUE;
    GmmConfig->SvPwrGatingD3HotEnable  = FALSE;
    GmmConfig->SvPwrGatingI3Enable     = FALSE;
    GmmConfig->SvPwrGatingPmcReqEnable = FALSE;
  } else {
    GmmConfig->SvPwrGatingHwAutoEnable = FALSE;
    GmmConfig->SvPwrGatingD3HotEnable  = SystemConfiguration.GmmPgD3HotEnabled;
    GmmConfig->SvPwrGatingI3Enable     = SystemConfiguration.GmmPgI3Enabled;
    GmmConfig->SvPwrGatingPmcReqEnable = SystemConfiguration.GmmPgPMCREnabled;
  }
  //
  // Set ISH configuration according to setup value.
  //
  IshConfig->Enable   = SystemConfiguration.ScIshEnabled;

  //
  // USB Device 21 configuration
  //
  UsbConfig->DisableComplianceMode      = SystemConfiguration.DisableComplianceMode;
  UsbConfig->UsbPerPortCtl              = SystemConfiguration.ScUsbPerPortCtl;
  //
  // xHCI (USB 3.0) related settings from setup variable
  //
  UsbConfig->Usb30Settings.Mode = SystemConfiguration.ScUsb30Mode;

  UsbConfig->SsicConfig.SsicPort[0].Enable = SystemConfiguration.Ssic1Support;
  UsbConfig->SsicConfig.SsicPort[1].Enable = SystemConfiguration.Ssic2Support;
  UsbConfig->SsicConfig.SsicPort[0].Rate   = SystemConfiguration.Ssic1Rate;
  UsbConfig->SsicConfig.SsicPort[1].Rate   = SystemConfiguration.Ssic2Rate;
  UsbConfig->SsicConfig.DlanePwrGating     = SystemConfiguration.SsicDlanePg;
  UsbConfig->XdciConfig.Enable             = SystemConfiguration.ScUsbOtg;
  //
  // Overcurrent applies to walk-up xHCI ports only - not SSIC or HSIC
  //
  //  OC0: Used for the OTG port (port 0)
  //  OC1: Used for the 2 host walk-up ports
  //
#if (ENBDT_PF_ENABLE == 1)
  UsbConfig->PortUsb20[0].OverCurrentPin = ScUsbOverCurrentPin0;
  UsbConfig->PortUsb20[1].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[2].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[3].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[4].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[5].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[6].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb20[7].OverCurrentPin = ScUsbOverCurrentPinSkip;

  UsbConfig->PortUsb30[0].OverCurrentPin = ScUsbOverCurrentPin0;
  UsbConfig->PortUsb30[1].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb30[2].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb30[3].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb30[4].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb30[5].OverCurrentPin = ScUsbOverCurrentPin1;

//[-start-161228-IB04530802-modify]//
//  UsbConfig->PortUsb20[0].Enable = TRUE;
//  UsbConfig->PortUsb20[1].Enable = TRUE;
//  UsbConfig->PortUsb20[2].Enable = TRUE;
//  UsbConfig->PortUsb20[3].Enable = TRUE;
//  UsbConfig->PortUsb20[4].Enable = TRUE;
//  UsbConfig->PortUsb20[5].Enable = TRUE;
//  UsbConfig->PortUsb20[6].Enable = TRUE;
//  UsbConfig->PortUsb20[7].Enable = TRUE;

//  UsbConfig->PortUsb30[0].Enable = TRUE;
//  UsbConfig->PortUsb30[1].Enable = TRUE;
//  UsbConfig->PortUsb30[2].Enable = TRUE;
//  UsbConfig->PortUsb30[3].Enable = TRUE;
//  UsbConfig->PortUsb30[4].Enable = TRUE;
//  UsbConfig->PortUsb30[5].Enable = TRUE;

  // USB 2.0 Ports
  for (Index = 0; Index < SC_MAX_USB2_PORTS; Index++) {
    if ((PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG >> Index) & 0x1) {
      UsbConfig->PortUsb20[Index].Enable = TRUE;
    } else {
      UsbConfig->PortUsb20[Index].Enable = FALSE;
    }      
  }

  // USB 3.0 Ports
  for (Index = 0; Index < SC_MAX_USB3_PORTS; Index++) {
    if ((PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG >> Index) & 0x1) {
      UsbConfig->PortUsb30[Index].Enable = TRUE;
    } else {
      UsbConfig->PortUsb30[Index].Enable = FALSE;
    }
  }

//[-end-161228-IB04530802-modify]//
#else
  UsbConfig->PortUsb20[0].OverCurrentPin = ScUsbOverCurrentPin0;
  UsbConfig->PortUsb20[1].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[2].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb20[3].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb20[4].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb20[5].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb20[6].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb20[7].OverCurrentPin = ScUsbOverCurrentPinSkip;

  UsbConfig->PortUsb30[0].OverCurrentPin = ScUsbOverCurrentPin0;
  UsbConfig->PortUsb30[1].OverCurrentPin = ScUsbOverCurrentPin1;
  UsbConfig->PortUsb30[2].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb30[3].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb30[4].OverCurrentPin = ScUsbOverCurrentPinSkip;
  UsbConfig->PortUsb30[5].OverCurrentPin = ScUsbOverCurrentPinSkip;
//[-start-161228-IB04530802-modify]//
//  UsbConfig->PortUsb20[0].Enable = TRUE;
//  UsbConfig->PortUsb20[1].Enable = TRUE;
//  UsbConfig->PortUsb20[2].Enable = TRUE;
//  UsbConfig->PortUsb20[3].Enable = FALSE;
//  UsbConfig->PortUsb20[4].Enable = FALSE;
//  UsbConfig->PortUsb20[5].Enable = FALSE;
//  UsbConfig->PortUsb20[6].Enable = FALSE;
//  UsbConfig->PortUsb20[7].Enable = FALSE;

//  UsbConfig->PortUsb30[0].Enable = TRUE;
//  UsbConfig->PortUsb30[1].Enable = TRUE;
//  UsbConfig->PortUsb30[2].Enable = FALSE;
//  UsbConfig->PortUsb30[3].Enable = FALSE;
//  UsbConfig->PortUsb30[4].Enable = FALSE;
//  UsbConfig->PortUsb30[5].Enable = FALSE;

  // USB 2.0 Ports
  for (Index = 0; Index < SC_MAX_USB2_PORTS; Index++) {
    if ((PlatformHardwareSwitch->UsbHsPortSwitch.CONFIG >> Index) & 0x1) {
      UsbConfig->PortUsb20[Index].Enable = TRUE;
    } else {
      UsbConfig->PortUsb20[Index].Enable = FALSE;
    }      
  }

  // USB 3.0 Ports
  for (Index = 0; Index < SC_MAX_USB3_PORTS; Index++) {
    if ((PlatformHardwareSwitch->UsbSsPortSwitch.CONFIG >> Index) & 0x1) {
      UsbConfig->PortUsb30[Index].Enable = TRUE;
    } else {
      UsbConfig->PortUsb30[Index].Enable = FALSE;
    }
  }

//[-end-161228-IB04530802-modify]//
#endif
//[-start-160414-IB07400716-add]//
  if ((PlatformInfo->BoardId == BOARD_ID_OXH_CRB) || (PlatformInfo->BoardId == BOARD_ID_LFH_CRB) || (PlatformInfo->BoardId == BOARD_ID_JNH_CRB)) {
    //
    // ApolloLake-I OxbowHill USB2.0 HW port map:
    //
    // USB 2.0 Port 0 (USB2_DN0/DP0)=> USB OTG,      J4A1, V5_USB3_OTG, USB_OC0_N
    // USB 2.0 Port 1 (USB2_DN1/DP1)=> USB 2/3 Port, J4A2, V5_USB3_1  , USB_OC1_N
    // USB 2.0 Port 2 (USB2_DN2/DP2)=> M.2 Key-B,    J2M1, x
    // USB 2.0 Port 3 (USB2_DN3/DP3)=> M.2 Key-E,    J2P1, x
    // USB 2.0 Port 4 (USB2_DN4/DP4)=> USB 2/3 Port, J4A2, V5_USB3_2  , USB_OC1_N
    // USB 2.0 Port 5 (USB2_DN5/DP5)=> USB 2.0 Port, J2C1, V5_USB2_5+6, USB_OC1_N
    // USB 2.0 Port 6 (USB2_DN6/DP6)=> USB 2.0 Port, J2B2, V5_USB2_5+6, USB_OC1_N
    // USB 2.0 Port 7 (USB2_DN7/DP7)=> USB 2.0 Port, J2B3, V5_USB2_7,   USB_OC1_N
    //
    UsbConfig->PortUsb20[0].Enable = TRUE;
    UsbConfig->PortUsb20[0].OverCurrentPin = ScUsbOverCurrentPin0;
    
    UsbConfig->PortUsb20[1].Enable = TRUE;
    UsbConfig->PortUsb20[1].OverCurrentPin = ScUsbOverCurrentPin1;
    
    UsbConfig->PortUsb20[2].Enable = TRUE;
    UsbConfig->PortUsb20[2].OverCurrentPin = ScUsbOverCurrentPinSkip;
    
    UsbConfig->PortUsb20[3].Enable = TRUE;
    UsbConfig->PortUsb20[3].OverCurrentPin = ScUsbOverCurrentPinSkip;
    
    UsbConfig->PortUsb20[4].Enable = TRUE;
    UsbConfig->PortUsb20[4].OverCurrentPin = ScUsbOverCurrentPin1;
    
    UsbConfig->PortUsb20[5].Enable = TRUE;
    UsbConfig->PortUsb20[5].OverCurrentPin = ScUsbOverCurrentPin1;
    
    UsbConfig->PortUsb20[6].Enable = TRUE;
    UsbConfig->PortUsb20[6].OverCurrentPin = ScUsbOverCurrentPin1;
    
    UsbConfig->PortUsb20[7].Enable = TRUE;
    UsbConfig->PortUsb20[7].OverCurrentPin = ScUsbOverCurrentPin1;

    //
    // ApolloLake-I OxbowHill USB3.0 HW port map:
    //
    // USB 3.0 Port 0 (USB3_P0_TXP/TXN/RXP/RXN)         => USB OTG,      J4A1, V5_USB3_OTG, USB_OC0_N
    // USB 3.0 Port 1 (USB3_P1_TXP/TXN/RXP/RXN)         => USB 2/3 Port, J4A2, V5_USB3_1  , USB_OC1_N
    // USB 3.0 Port 2 (PCIE_P5_USB3_P2_TXP/TXN/RXP/RXN) => PCIE Port 5
    // USB 3.0 Port 3 (PCIE_P4_USB3_P3_TXP/TXN/RXP/RXN) => PCIE Port 4
    // USB 3.0 Port 4 (PCIE_P3_USB3_P4_TXP/TXN/RXP/RXN) => USB 2/3 Port, J4A2, V5_USB3_2  , USB_OC1_N
    // USB 3.0 Port 5 (SATA_P1_USB3_P5_TXP/TXN/RXP/RXN) => SATA Port 1
    //
    UsbConfig->PortUsb30[0].Enable = TRUE;
    UsbConfig->PortUsb30[0].OverCurrentPin = ScUsbOverCurrentPin0;
    
    UsbConfig->PortUsb30[1].Enable = TRUE;
    UsbConfig->PortUsb30[1].OverCurrentPin = ScUsbOverCurrentPin1;
    
    UsbConfig->PortUsb30[2].Enable = FALSE;
    UsbConfig->PortUsb30[2].OverCurrentPin = ScUsbOverCurrentPinSkip;
    
    UsbConfig->PortUsb30[3].Enable = FALSE;
    UsbConfig->PortUsb30[3].OverCurrentPin = ScUsbOverCurrentPinSkip;
    
    UsbConfig->PortUsb30[4].Enable = TRUE;
    UsbConfig->PortUsb30[4].OverCurrentPin = ScUsbOverCurrentPin1;
    
    UsbConfig->PortUsb30[5].Enable = FALSE;  
    UsbConfig->PortUsb30[5].OverCurrentPin = ScUsbOverCurrentPinSkip;
  }
//[-end-160414-IB07400716-add]//
//[-start-161112-IB07400813-add]//
  //
  // Pre-Port Control
  //
  if (SystemConfiguration.ScUsbPerPortCtl) {
    //
    // USB 2.0
    //
    for (PortIndex = 0; PortIndex < 8; PortIndex++) {
      UsbConfig->PortUsb20[PortIndex].Enable = SystemConfiguration.ScUsbPort[PortIndex];
    }
    //
    // USB 3.0
    //
    for (PortIndex = 0; PortIndex < 6; PortIndex++) {
      UsbConfig->PortUsb30[PortIndex].Enable = SystemConfiguration.ScUsbPort[PortIndex];
    }
  }
//[-end-161112-IB07400813-add]//
  //
  // Set LPSS configuration according to setup value.
  //
  LpssConfig->I2c0Enable    = SystemConfiguration.LpssI2C0Enabled;
  LpssConfig->I2c1Enable    = SystemConfiguration.LpssI2C1Enabled;
  LpssConfig->I2c2Enable    = SystemConfiguration.LpssI2C2Enabled;
  LpssConfig->I2c3Enable    = SystemConfiguration.LpssI2C3Enabled;
  LpssConfig->I2c4Enable    = SystemConfiguration.LpssI2C4Enabled;
  LpssConfig->I2c5Enable    = SystemConfiguration.LpssI2C5Enabled;
  LpssConfig->I2c6Enable    = SystemConfiguration.LpssI2C6Enabled;
  LpssConfig->I2c7Enable    = SystemConfiguration.LpssI2C7Enabled;
  LpssConfig->Hsuart0Enable = SystemConfiguration.LpssHsuart0Enabled;
  LpssConfig->Hsuart1Enable = SystemConfiguration.LpssHsuart1Enabled;
  LpssConfig->Hsuart2Enable = SystemConfiguration.LpssHsuart2Enabled;
  LpssConfig->Hsuart3Enable = SystemConfiguration.LpssHsuart3Enabled;
  LpssConfig->Spi0Enable    = SystemConfiguration.LpssSpi0Enabled;
  LpssConfig->Spi1Enable    = SystemConfiguration.LpssSpi1Enabled;
  LpssConfig->Spi2Enable    = SystemConfiguration.LpssSpi2Enabled;
  LpssConfig->Uart2KernelDebugBaseAddress = SystemConfiguration.Uart2KernelDebugBaseAddress;
  for (DevIndex = 0; DevIndex < LPSS_I2C_DEVICE_NUM; DevIndex++) {
    LpssConfig->I2cClkGateCfg[DevIndex] = SystemConfiguration.LpssI2cClkGateCfg[DevIndex];
  }
  for (DevIndex = 0; DevIndex < LPSS_HSUART_DEVICE_NUM; DevIndex++) {
    LpssConfig->HsuartClkGateCfg[DevIndex] = SystemConfiguration.LpssHsuartClkGateCfg[DevIndex];
  }
  for (DevIndex = 0; DevIndex < LPSS_SPI_DEVICE_NUM; DevIndex++) {
    LpssConfig->SpiClkGateCfg[DevIndex] = SystemConfiguration.LpssSpiClkGateCfg[DevIndex];
  }
  //
  // Kernel Debugger (e.g. WinDBG) Enable
  //
  LpssConfig->OsDbgEnable = (BOOLEAN)SystemConfiguration.OsDbgEnable;
  LpssConfig->S0ixEnable  = (BOOLEAN)SystemConfiguration.LowPowerS0Idle;
//[-start-160413-IB03090426-remove]//
//  LpssConfig->ExiEnable   = SystemConfiguration.EXIEnabled;
//[-end-160413-IB03090426-remove]//
  //
  // Set SCS configuration according to setup value.
  //
  ScsConfig->SdcardEnable     = SystemConfiguration.SccSdcardEnabled;
  ScsConfig->UfsEnable        = SystemConfiguration.SccUfsEnabled;
  ScsConfig->EmmcEnable       = SystemConfiguration.ScceMMCEnabled;
  ScsConfig->SdioEnable       = SystemConfiguration.SccSdioEnabled;
  ScsConfig->EmmcHostMaxSpeed = SystemConfiguration.ScceMMCHostMaxSpeed;
  ScsConfig->GppLock          = SystemConfiguration.GPPLock;
//[-start-160901-IB03090433-modify]//
#if (TABLET_PF_ENABLE == 1)
  if (BoardId == BOARD_ID_BXT_FFD) {
    ScsConfig->SccEmmcTraceLength = SCC_EMMC_SHORT_TRACE_LEN;
//[-start-160509-IB03090427-add]//
    ScsConfig->EmmcRegDllConfig.TxCmdCntl      = 0x505;
    ScsConfig->EmmcRegDllConfig.TxDataCntl1    = 0xC15;
    ScsConfig->EmmcRegDllConfig.TxDataCntl2    = 0x1C1C1C00;
    ScsConfig->EmmcRegDllConfig.RxCmdDataCntl1 = 0x1C1C1C00;
    ScsConfig->EmmcRegDllConfig.RxStrobeCntl   = 0x0a0a;
    ScsConfig->EmmcRegDllConfig.RxCmdDataCntl2 = 0x1001C;
    ScsConfig->EmmcRegDllConfig.MasterSwCntl   = 0x001;
//[-end-160509-IB03090427-add]//
  } else {
    ScsConfig->SccEmmcTraceLength = SCC_EMMC_LONG_TRACE_LEN;
  }
#endif
//[-end-160901-IB03090433-modify]//

//[-start-160817-IB03090432-modify]//
  //
  // If boot from eMMC, disable UFS controller.
  //
  if (SystemConfiguration.SccUfsEnabled == DEVICE_AUTO) {

  if (BootMediaIsUfs ()) {
      ScsConfig->UfsEnable = DEVICE_ENABLE;
    } else {
      ScsConfig->UfsEnable = DEVICE_DISABLE;
    }
  }
  if (SystemConfiguration.ScceMMCEnabled == DEVICE_AUTO) {
    if (BootMediaIsEmmc ()) {
      ScsConfig->EmmcEnable = DEVICE_ENABLE;
    } else {
      ScsConfig->EmmcEnable = DEVICE_DISABLE;
    }
  }
//[-end-160817-IB03090432-modify]//

#if (TABLET_PF_ENABLE == 1)
  if (SystemConfiguration.SccSdioEnabled == DEVICE_AUTO) {

    if (SystemConfiguration.WifiSel == 2) { //PCIe WiFi for HR16 BOM
      SystemConfiguration.SccSdioEnabled = 0;
    } else {
      SystemConfiguration.SccSdioEnabled = 1;
    }
  }
#endif

  VtdConfig->VtdEnable = SystemConfiguration.VTdEnable;
  //
  // Power management Configuration
  //
//  if (SystemConfiguration.CsmControl == 1 || SystemConfiguration.Cg8254 == 0) {
  if (SystemConfiguration.Cg8254 == 0) {
    PmConfig->Timer8254ClkGateEn = FALSE;
  }
//[-start-160509-IB03090427-add]//
  PmConfig->PowerButterDebounceMode = SystemConfiguration.PowerButterDebounceMode;
//[-end-160509-IB03090427-add]//
//[-start-160719-IB03090429-add]//
  if ((SystemConfiguration.LowPowerS0Idle == 1) && (SystemConfiguration.TenSecondPowerButtonEnable == 1)) {
    PmConfig->PwrBtnOverridePeriod              = 0x3; // 10sec
  } else {
    PmConfig->PwrBtnOverridePeriod              = 0;   // 4sec
  }
//[-end-160719-IB03090429-add]//
//[-start-160727-IB07400763-add]//
  PmConfig->PciClockRun = SystemConfiguration.LpcClockRun;
//[-end-160727-IB07400763-add]//
  
//[-start-160817-IB03090432-modify]//
  if ((GetBxtSeries() == BxtP) && (BootMediaIsSpi ())) {
//[-end-160817-IB03090432-modify]//
    //
    // Configure Flash Protection Range Registers
    //
//[-start-160301-IB07220042-modify]//
//[-start-161012-IB07220142-modify]//
    FlashProtectionEnabled = (SystemConfiguration.FprrEnable == TRUE && (GetBootModeHob() != BOOT_IN_RECOVERY_MODE)) ? TRUE : FALSE;
//[-end-161012-IB07220142-modify]//
//[-end-160301-IB07220042-modify]//
//[-start-160812-IB11270159-add]//
    if (IsRunSecureFlash()) {
      LockDownConfig->BiosLock       = FALSE;
      LockDownConfig->SpiEiss        = FALSE;
      FlashProtectionEnabled         = FALSE;
    }  
//[-end-160812-IB11270159-add]//
    //
    // Enabling Flash Protection Range Registers
    // Enable FPRR policy and set up ranges on non-Capsule Update flow with Flash Wear-Out Protection enabled
    // PrintFlashProtectionConfig() dumps FPRR information during ScPrintPolicyPpi()
    //FPRR  bit is enabled by default as per Platform policy, except that when system is in recovery mode or FDO is enabled. In this case this will be disabled as part of Firmware Update / Recovery update
    //
//[-start-160509-IB03090427-modify]//
    if (FlashProtectionEnabled && (FALSE == PcdGetBool(PcdFDOState))) {
      //
      // Flash Protection Range Register initialization
      //
      for (Index = 0; Index < SC_FLASH_PROTECTED_RANGES; Index++) {
        FlashProtectionConfig->ProtectRange[Index].WriteProtectionEnable  = TRUE;
        FlashProtectionConfig->ProtectRange[Index].ReadProtectionEnable   = FALSE;
      }

//[-start-160525-IB03090427-modify]//
      Status = FindBootPartition1( (UINT32 *)&FvBuffer);
//[-start-160429-IB10860197-add]//
      BP1MemAddress = (UINT32)FvBuffer;
//[-end-160429-IB10860197-add]//  
      DEBUG ((DEBUG_INFO, "SPI Boot Partition 1 at = %X\n", FvBuffer));
      BpdtHeader = (BPDT_HEADER *)FvBuffer;
      sBpdtHeader = BpdtHeader;
      while (TRUE) {
        DEBUG ((DEBUG_INFO, "Signature = %X, DscCount = %X\n",BpdtHeader->Signature, BpdtHeader->DscCount));

        if (BpdtHeader->Signature != BPDT_SIGNATURE) {
          DEBUG ((DEBUG_ERROR, "SPI Boot Partition1 layout invalid\n"));
        } else {
          sBpdtHeader = BpdtHeader;
          BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeader + sizeof(BPDT_HEADER));

          for (Index = 0; Index < (UINT8)BpdtHeader->DscCount; Index++, BpdtEntryPtr++)  {
            DEBUG ((DEBUG_INFO, "Type = %X\n",BpdtEntryPtr->Type));

            if (BpdtIbb == BpdtEntryPtr->Type) {
              Offset = BpdtEntryPtr->LbpOffset;
              IbbObbInformation.IbbOffset = (UINTN) FvBuffer + Offset;
              IbbObbInformation.IbbSize   = BpdtEntryPtr->Size;
              DEBUG ((EFI_D_INFO, "IbbOffset = %x , IbbSize = %x\n", IbbObbInformation.IbbOffset, IbbObbInformation.IbbSize));
            } // if bpIBB
          } //for Index
        } //else
        if (sBpdtHeader != BpdtHeader) {
          BpdtHeader = sBpdtHeader;
        } else {
          break;
        }
      } //while

      Status = FindBootPartition2( (UINT32 *)&FvBuffer);
//[-start-160429-IB10860197-add]//
      BP2MemAddress = (UINT32)FvBuffer;
//[-end-160429-IB10860197-add]//  
      DEBUG ((DEBUG_INFO, "SPI Boot Partition 2 at = %X\n", FvBuffer));
      BpdtHeader = (BPDT_HEADER *)FvBuffer;
      sBpdtHeader = BpdtHeader;
      BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeader + sizeof(BPDT_HEADER));
      
      while (TRUE) {
        DEBUG ((DEBUG_INFO, "Signature = %X, DscCount = %X\n",BpdtHeader->Signature, BpdtHeader->DscCount));

        if (BpdtHeader->Signature != BPDT_SIGNATURE){
          DEBUG ((DEBUG_ERROR, "SPI Boot Partition layout invalid\n"));
        } else {
          sBpdtHeader = BpdtHeader;
          BpdtEntryPtr = (BPDT_ENTRY *)((UINTN)BpdtHeader + sizeof(BPDT_HEADER));
          for (Index = 0; Index < (UINT8)BpdtHeader->DscCount; Index++, BpdtEntryPtr++) {
            DEBUG ((DEBUG_INFO, "Type = %X\n",BpdtEntryPtr->Type));

            if (BpdtSbpdt == BpdtEntryPtr->Type) {
               sBpdtHeader = (BPDT_HEADER *)((UINTN)FvBuffer + BpdtEntryPtr->LbpOffset);
            }
            if (BpdtObb == BpdtEntryPtr->Type) {
              Offset = BpdtEntryPtr->LbpOffset;
              IbbObbInformation.ObbOffset = (UINTN) FvBuffer + Offset;
              IbbObbInformation.ObbSize   = BpdtEntryPtr->Size;
              DEBUG ((EFI_D_INFO, "ObbOffset = %x , ObbSize = %x\n", IbbObbInformation.ObbOffset, IbbObbInformation.ObbSize));
            } // if bpObb
          } //for Index
        } //else
        if (sBpdtHeader != BpdtHeader) {
          BpdtHeader = sBpdtHeader;
        } else {
          break;
        }
      }
//[-end-160525-IB03090427-modify]//

      //
      // Assign FPRR ranges
      //
//[-start-160429-IB10860197-modify]//
      BootPartitionSize = GetSpiFlashRegionLimit (BIOS_Region) / 2;
      IBBSpiAddress = 0x1000 + (IbbObbInformation.IbbOffset - BP1MemAddress);
      OBBSpiAddress = 0x1000 + BootPartitionSize + (IbbObbInformation.ObbOffset - BP2MemAddress);
      
      FlashProtectionConfig->ProtectRange[0].ProtectedRangeBase    = (UINT16)(IBBSpiAddress >> 12);
      FlashProtectionConfig->ProtectRange[0].ProtectedRangeLimit   = (UINT16)((IBBSpiAddress + IbbObbInformation.IbbSize - 1) >> 12);
//[-start-181004-IB07401023-remove]//
      //
      // do not pretect OBB for DMI/OA
      //
//      FlashProtectionConfig->ProtectRange[1].ProtectedRangeBase    = (UINT16)(OBBSpiAddress >> 12);
//      FlashProtectionConfig->ProtectRange[1].ProtectedRangeLimit   = (UINT16)((OBBSpiAddress + IbbObbInformation.ObbSize - 1) >> 12);
//[-end-181004-IB07401023-remove]//
//[-end-160429-IB10860197-modify]//
  } else {
		DEBUG ((DEBUG_INFO, " BIOS FPRR is not done  -FDO ASSERT Status "));
    }
  }

  DciConfig->DciEn         = SystemConfiguration.DciEn;
  DciConfig->DciAutoDetect = SystemConfiguration.DciAutoDetect;
//[-end-160509-IB03090427-modify]//
//[-start-170222-IB07400843-add]//
  SerialIrqConfig->SirqEnable = SystemConfiguration.SirqEnabled;
  SerialIrqConfig->SirqMode   = SystemConfiguration.SirqMode;
//[-end-170222-IB07400843-add]//
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
  PcdSet8 (PcdMmioMem64Enable, (UINT8)SystemConfiguration.PciMem64DecodeSupport);
  PcdSet8 (PcdMmioMem64SkipOnboard, (UINT8)SystemConfiguration.PciMem64SkipOnboard);
#endif  
//[-end-170119-IB07401072-add]//
  return Status;
}
//[-end-151228-IB03090424-modify]//
