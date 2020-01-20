/** @file
  Crisis Recovery Implementation

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "CrisisRecovery.h"
#include <Library/BaseOemSvcKernelLib.h>
#include <Library/PeiOemSvcKernelLib.h>
#include <Library/FlashRegionLib.h>
#include <PostCode.h>
//[-start-151229-IB06740454-add]//
#include <H2OIhisi.h>
//[-end-151229-IB06740454-add]//

FLASH_ENTRY mProtectRegion[] = {
  {
    0,  // RecoveryFV Address to be updated
    0,  // RecoveryFv Size to be updated
    0   // RecoveryFv Offset to be updated
  },
  {0x0, 0x0, 0x0}
};



EFI_FFS_FILE_HEADER  *mFfsHeader;

//
// Module globals
//
EFI_PEI_RECOVERY_MODULE_PPI mRecoveryPpi = {PlatformRecoveryModule};

EFI_PEI_PPI_DESCRIPTOR mRecoveryPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiRecoveryModulePpiGuid,
  &mRecoveryPpi
};

STATIC
EFI_STATUS
EFIAPI
GetDefaultProtectRegion (
  OUT  FLASH_ENTRY     **DefaultProtectTable,
  OUT  UINTN           *TableSize
  )
{

  FLASH_ENTRY   *Pool;
  UINT64        RecoveryFvAddr;
  UINT64        RecoveryFvSize;
  UINTN         DefaultSize;
  EFI_STATUS    Status;

  RecoveryFvAddr = 0;
  DefaultSize = sizeof(FLASH_ENTRY) * 2;
  *TableSize = DefaultSize; 

  Status = PeiServicesAllocatePool (DefaultSize, (VOID**)&Pool);
  *DefaultProtectTable = Pool;

  if (EFI_ERROR(Status)){
    return EFI_OUT_OF_RESOURCES;
  }

  RecoveryFvAddr = FdmGetAddressById (
                     &gH2OFlashMapRegionFvGuid,  
                     (UINT8*)&gH2OFlashMapRegionPeiFvGuid,
                     1
                     );

  RecoveryFvSize = FdmGetSizeById (
                     &gH2OFlashMapRegionFvGuid,  
                     (UINT8*)&gH2OFlashMapRegionPeiFvGuid,
                     1
                     );

  Pool->WriteAddress = (UINT32) RecoveryFvAddr;
  Pool->WriteSize    = (UINT32) RecoveryFvSize;
  Pool->SourceOffset = (UINT32) (RecoveryFvAddr - FdmGetBaseAddr());
  Pool++;

  Pool->WriteAddress = 0;
  Pool->WriteSize    = 0;
  Pool->SourceOffset = 0;

  return Status;
}

/**
 Pei crisis recovery in PEI stage

 @param [in]   FileHandle
 @param [in]   PeiServices      General purpose services available to every PEIM

 @retval EFI_SUCCESS            Success
 @retval EFI_UNSUPPORTED        Some required PPIs are not available
 @retval EFI_OUT_OF_RESOURCES   Not enough memory to allocate

**/
EFI_STATUS
CrisisRecoveryEntry (
  IN EFI_PEI_FILE_HANDLE             FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  )
{
  EFI_STATUS                      Status;
  EFI_PEI_PPI_DESCRIPTOR          *oldRecoveryPpiDescriptor;
  EFI_PEI_RECOVERY_MODULE_PPI     *oldRecoveryPpi;

  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiRecoveryModulePpiGuid,
                             0,
                             &oldRecoveryPpiDescriptor,
                             (VOID **)&oldRecoveryPpi
                             );

  if (Status == EFI_SUCCESS) {
    Status = (*PeiServices)->ReInstallPpi (
                               PeiServices,
                               oldRecoveryPpiDescriptor,
                               &mRecoveryPpiList
                               );
  } else {
    Status = (*PeiServices)->InstallPpi (
                               PeiServices,
                               &mRecoveryPpiList
                               );
  }

  return Status;
}

/**
 Get BIOS image information from recovery capsule

 @param [in]   Capsule          Recovery capsule buffer
 @param [in]   CapsuleSize      Recovery capsule file size
 @param [out]  BiosImageOffset  The offset of BIOS image from recovery capsule file buffer
 @param [out]  BiosImageSize    The size of the BIOS image


 @retval EFI_SUCCESS            BIOS image information is successfully retrieved
 @return others                 Failed to get BIOS image information from recovery capsule

**/
EFI_STATUS
GetBiosImageFromCapsule (
  IN UINT8    *Capsule,
  IN UINTN    CapsuleSize,
  OUT UINTN   *BiosImageOffset,
  OUT UINTN   *BiosImageSize
  )
{
  UINTN                       Index;
  ISFLASH_DATA_REGION_HEADER  *DataRegion;

  if ( (Capsule == NULL) || (CapsuleSize == 0) ) {
    return EFI_INVALID_PARAMETER;
  }
  if (!((Capsule[0] == (UINT8)'M') && (Capsule[1] == (UINT8)'Z'))) {
    *BiosImageOffset = 0;
    *BiosImageSize = CapsuleSize;
    if (FeaturePcdGet(PcdSecureFlashSupported)) {
      *BiosImageSize -= SIGNATURE_SIZE;
    }
    return EFI_SUCCESS;
  }

  //
  // Capsule image is PE32 image
  // Search for BIOS image
  //
  for (Index = 0; Index < CapsuleSize - ISFLASH_TAG_SIZE; Index++) {
    if (CompareMem(Capsule + Index, ISFLASH_BIOS_IMAGE_TAG_HALF_1, ISFLASH_HALF_TAG_SIZE) == 0){
      if (CompareMem(Capsule + Index + ISFLASH_HALF_TAG_SIZE, ISFLASH_BIOS_IMAGE_TAG_HALF_2, ISFLASH_HALF_TAG_SIZE) == 0) {
        break;
      }
    }
  }
  if (Index == CapsuleSize - ISFLASH_TAG_SIZE) {
    return EFI_NOT_FOUND;
  }

  //
  // Copy BIOS image to the start of FirmwareBin pointer
  //
  DataRegion = (ISFLASH_DATA_REGION_HEADER *)(Capsule + Index);
  *BiosImageOffset = Index + sizeof(ISFLASH_DATA_REGION_HEADER);
  *BiosImageSize = DataRegion->DataSize;
  if (*BiosImageOffset + *BiosImageSize > CapsuleSize) {
    return EFI_NOT_FOUND;
  }
  return EFI_SUCCESS;
}

VOID
ClearFirmwareUpdatingFlag (
  VOID
  )
{
  EFI_STATUS  Status;

  if (!IsFirmwareFailureRecovery()) {
    return;
  }

  Status = FlashErase( (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionFtwBackupGuid ,1), GetFlashBlockSize());
  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR(Status);
    return;
  }
}

/**
 Provide the functionality of the Ea Recovery Module.

 @param [in]   PeiServices      General purpose services available to every PEIM.
 @param [in]   This

 @retval Status                 EFI_SUCCESS if the interface could be successfully
                                installed

**/
EFI_STATUS
EFIAPI
PlatformRecoveryModule (
  IN EFI_PEI_SERVICES                     **PeiServices,
  IN EFI_PEI_RECOVERY_MODULE_PPI          *This
  )
{
  EFI_STATUS                            Status;
  EFI_PEI_DEVICE_RECOVERY_MODULE_PPI    *DeviceRecoveryModule;
  UINTN                                 NumberOfImageProviders;
  BOOLEAN                               ProviderAvailable;
  UINTN                                 NumberRecoveryCapsules;
  UINTN                                 RecoveryCapsuleSize;
  EFI_GUID                              DeviceId;
  BOOLEAN                               ImageFound;
  EFI_PHYSICAL_ADDRESS                  Address;
  VOID                                  *Buffer;
  UINTN                                 Index;
  UINTN                                 BiosImageOffset;
  UINTN                                 BiosImageSize;
  UINT8                                 DeviceRecoveryModuleFound;
//[-start-161123-IB07250310-add]//
  BOOLEAN                               FindFv;
  EFI_FIRMWARE_VOLUME_HEADER            *FvHeader;
  UINTN                                 FvHeaderAddress;
  VOID                                  *SetupData;
  UINTN                                 VariableSize;
  UINT8                                 *SupervisorPasswordBuf;
  UINT8                                 *UserPasswordBuf;
//[-end-161123-IB07250310-add]//

  Index                   = 0;
  Status                  = EFI_SUCCESS;
  ProviderAvailable       = TRUE;
  ImageFound              = FALSE;
  NumberOfImageProviders  = 0;
  DeviceRecoveryModule    = NULL;
  DeviceRecoveryModuleFound = FALSE;

  POST_CODE (PEI_ENTER_RECOVERY_MODE);
  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "     ############ PEI Crisis Recovery Entry ############\n\n"));
  
  while (!DeviceRecoveryModuleFound){
    //
    // Search the platform for some recovery capsule if the DXE IPL
    // discovered a recovery condition and has requested a load.
    //
    Status = PeiServicesLocatePpi (
               &gEfiPeiDeviceRecoveryModulePpiGuid,
               Index,
               NULL,
               (VOID **)&DeviceRecoveryModule
               );

    if (EFI_ERROR (Status)) {
      return Status;
    }

    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Device Recovery PPI located\n"));

    Status = DeviceRecoveryModule->GetNumberRecoveryCapsules (
                                     PeiServices,
                                     DeviceRecoveryModule,
                                     &NumberRecoveryCapsules
                                     );

    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Get Number of Recovery Capsules Returns: %r\n", Status));
    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Number Of Recovery Capsules: %d\n", NumberRecoveryCapsules));

    if (EFI_ERROR(Status) || (NumberRecoveryCapsules == 0)) {
      Index++;
      continue;
    }
    DeviceRecoveryModuleFound = TRUE;
  }
  //
  // If there is an image provider, get the capsule ID
  //
  RecoveryCapsuleSize = 0;

  Status = DeviceRecoveryModule->GetRecoveryCapsuleInfo (
                                   PeiServices,
                                   DeviceRecoveryModule,
                                   0,
                                   &RecoveryCapsuleSize,
                                   &DeviceId
                                   );

  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Get Recovery Capsule Info Returns: %r\n", Status));
  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Recovery Capsule Size: %d\n", RecoveryCapsuleSize));
  
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Only support the 2 capsule types known
  // Future enhancement is to rank-order the selection
  //
  if ((!CompareGuid (&DeviceId, &gRecoveryOnFatIdeDiskGuid)) &&
      (!CompareGuid (&DeviceId, &gRecoveryOnDataCdGuid)) &&
      (!CompareGuid (&DeviceId, &gRecoveryOnFatUsbDiskGuid))
      ) {
    return EFI_UNSUPPORTED;
  }

  Status = PeiServicesAllocatePages (
             EfiBootServicesCode,
             (RecoveryCapsuleSize - 1) / 0x1000 + 1,
             &Address
             );

  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Allocate buffer for loading recovery capsule Returns: %r\n", Status));

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Buffer = (UINT8 *) (UINTN) Address;

  Status = DeviceRecoveryModule->LoadRecoveryCapsule (
                                   PeiServices,
                                   DeviceRecoveryModule,
                                   0,
                                   Buffer
                                   );
  
  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "LoadRecoveryCapsule Returns: %r\n", Status));
  
  if (!EFI_ERROR (Status)) {
    Status = GetBiosImageFromCapsule (
               (UINT8 *)Buffer,
               RecoveryCapsuleSize,
               &BiosImageOffset,
               &BiosImageSize
               );
    if (EFI_ERROR(Status)) {
      return Status;
    }

//[-start-161123-IB07250310-modify]//
    if (!FeaturePcdGet(PcdUseFastCrisisRecovery)) {
      POST_CODE (PEI_RECOVERY_LOAD_FILE_DONE);
      DEBUG ((EFI_D_INFO | EFI_D_LOAD, "\n     ######### Flash BIOS #########   \n"));    
      FlashBios((UINT8 *)Buffer + BiosImageOffset, BiosImageSize);
    }
//[-end-161123-IB07250310-modify]//
  }

//[-start-161123-IB07250310-add]//
  if (FeaturePcdGet(PcdUseFastCrisisRecovery)) {
    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Fast Crisis to handoff to Dxe phase preparation.\n"));
	if (PcdGetBool (PcdSecureSysPasswordSupported)) {
      SupervisorPasswordBuf = NULL;
      CommonGetVariableDataAndSize (
        EFI_ADMIN_PASSWORD_NAME,
        &gInsydeSecureFirmwarePasswordGuid,
        &VariableSize,
        (VOID **) &SupervisorPasswordBuf
        );

      if (SupervisorPasswordBuf != NULL) {
        BuildGuidDataHob (
        &gInsydeSecureFirmwarePasswordHobGuid,
        SupervisorPasswordBuf,
        VariableSize
        );
      }
    } else {
      SupervisorPasswordBuf = NULL;
      CommonGetVariableDataAndSize (
        L"SystemSupervisorPw",
        &gEfiSupervisorPwGuid,
        &VariableSize,
        (VOID **) &SupervisorPasswordBuf
        );

      if (SupervisorPasswordBuf != NULL) {
        BuildGuidDataHob (
          &gEfiSupervisorPwHobGuid,
          SupervisorPasswordBuf,
          VariableSize
          );
      }
    }

    UserPasswordBuf = NULL;
    CommonGetVariableDataAndSize (
      L"SystemUserPw",
      &gEfiUserPwGuid,
      &VariableSize,
      (VOID **) &UserPasswordBuf
      );
    if (UserPasswordBuf != NULL) {
      BuildGuidDataHob (
        &gEfiUserPwHobGuid,
        UserPasswordBuf,
        VariableSize
        );
    }

    SetupData = NULL;
    CommonGetVariableDataAndSize (
      L"SetupOrg",
      &gSystemConfigurationGuid,
      &VariableSize,
      (VOID **) &SetupData
      );
    if (SetupData == NULL) {
      SetupData = NULL;
      CommonGetVariableDataAndSize (
        SETUP_VARIABLE_NAME,
        &gSystemConfigurationGuid,
        &VariableSize,
        (VOID **) &SetupData
        );
    }

    if (SetupData != NULL) {
      BuildGuidDataHob (
        &gEfiPowerOnPwSCUHobGuid,
        SetupData,
        VariableSize
        );
    }

    {
      EFI_PEI_FV_HANDLE    FvHandle;
      EFI_PEI_FILE_HANDLE  FileHandle;
      EFI_STATUS           Status;
      EFI_PEI_HOB_POINTERS Hob;

      FvHandle   = NULL;

      //
      //  OemSvcBootModeCreateFv(); could utilize to create customer's FvHobs
      //
      DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Checking any exist of FvHobs with DxeCore Ffs for DxeIpl handoff.\n"));
      Hob.Raw = GetHobList ();
      while ((Hob.Raw = GetNextHob (EFI_HOB_TYPE_FV, Hob.Raw)) != NULL) {
        FvHandle = (VOID *)(UINTN) (Hob.FirmwareVolume->BaseAddress);
        Status = PeiServicesFfsFindNextFile (EFI_FV_FILETYPE_DXE_CORE, FvHandle, &FileHandle);
        if (!EFI_ERROR (Status)) {
          DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Finding the DxeCore existed in FV at memory address %x \n", (UINTN) (Hob.FirmwareVolume->BaseAddress)));
          return Status;
        }
        Hob.Raw = GET_NEXT_HOB (Hob);
      }
    }
	
    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "No DxeCore Ffs exist at currently FvHobs.\n"));
      
    //
    //  Creating FV to HOB. It make the FV visible for DxeIplFindDxeCore().
    //
    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Creating FvHobs for DxeIpl to handoff from image which read from storage\n"));
    FvHeaderAddress = (UINTN) Buffer;
    do {
      FindFv = FALSE;
      FvHeader = (EFI_FIRMWARE_VOLUME_HEADER*)FvHeaderAddress;
      if (FvHeader->Signature == EFI_FVH_SIGNATURE) {

        //
        // Find the usable of Firmware Volume
        //
        if ((CompareGuid (&gEfiFirmwareFileSystem2Guid, &FvHeader->FileSystemGuid)) || 
            (CompareGuid (&gEfiFirmwareFileSystem3Guid, &FvHeader->FileSystemGuid))) {
          FindFv = TRUE;
          BuildFvHob (FvHeaderAddress, FvHeader->FvLength);
          DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Find the valid FVs at memory address %x \n", FvHeaderAddress));
          FvHeaderAddress += (UINTN)FvHeader->FvLength;
        }
      }
      if (!FindFv) {
        FvHeaderAddress += 0x10;
      }
    } while (((FvHeaderAddress - ((UINTN) Buffer))+ sizeof (EFI_FIRMWARE_VOLUME_HEADER)) < RecoveryCapsuleSize);
  }
//[-end-161123-IB07250310-add]//

  return Status;
}

VOID
FlashBios (
  IN VOID                               *Buffer,
  IN UINTN                              RecoveryFileSize
  )
{
  EFI_STATUS                            Status;
  UINTN                                 WriteCount;
  UINT16                                ProtectNumCount;
  UINT32                                WriteAddress;
  UINT32                                WriteSize;
  UINT32                                SrcAddress;
  PEI_SPEAKER_IF_PPI                    *SpeakerPPI;
  BOOLEAN                               SpeakerOn;
  BOOLEAN                               DescriptorBIOS;
  EFI_PHYSICAL_ADDRESS                  RomBinFile;
  UINTN                                 NumOfProtectRegion;
  FLASH_ENTRY                           *OemProtectTable;
  UINTN                                 TableCount;
  BOOLEAN                               UseEcIdle;
  BOOLEAN                               UseCrisisProtectTable;
  BOOLEAN                               SkipPeiRegion;
  FLASH_ENTRY                           *ProtectRegion;
  UINTN                                 IsMeProtectRegion;
  UINTN                                 RecoveryBlocks;
  EFI_PEI_SERVICES                      **PeiServices;
  UINT8                                 *ReadBuffer;
  UINT64                                RecoveryFvAddr;
  UINT64                                RecoveryFvSize;
  UINTN                                 DefaultProtectTableSize;
  FLASH_ENTRY                           *DefaultProtectTable;
//[-start-151229-IB06740454-add]//
  UINT8                                 FlashMode;
//[-end-151229-IB06740454-add]//

  TableCount            = 0;
  NumOfProtectRegion    = 0;
  IsMeProtectRegion     = 0;
  RecoveryBlocks        = 0;
  UseEcIdle             = FALSE;
  OemProtectTable       = NULL;
  ProtectRegion         = NULL;
  SpeakerOn             = FALSE;
  DescriptorBIOS        = FALSE;
  UseCrisisProtectTable = TRUE;
  SkipPeiRegion         = FALSE;
  ReadBuffer            = NULL;
  DefaultProtectTable   = NULL;
  PeiServices = (EFI_PEI_SERVICES **) GetPeiServicesTablePointer();

  Status = PeiServicesLocatePpi (
             &gPeiSpeakerInterfacePpiGuid,
             0,
             NULL,
             (VOID **)&SpeakerPPI
             );
  if (!EFI_ERROR (Status)) {
    SpeakerOn = TRUE;
  }

  RecoveryFvAddr = FdmGetAddressById (
                     &gH2OFlashMapRegionFvGuid,
                     (UINT8*)&gH2OFlashMapRegionPeiFvGuid,
                     1
                     );

  RecoveryFvSize = FdmGetSizeById (
                     &gH2OFlashMapRegionFvGuid,
                     (UINT8*)&gH2OFlashMapRegionPeiFvGuid,
                     1
                     );

  Status = GetDefaultProtectRegion (&DefaultProtectTable, &DefaultProtectTableSize);
  ASSERT(DefaultProtectTable);

  Status = EFI_SUCCESS;

  //
  // Get OEM protect Regions
  // (OemServices)
  //
  OemSvcGetProtectTable (
    &TableCount,
    &UseEcIdle,
    &OemProtectTable
    );

  if (UseEcIdle) {
    OemSvcEcIdle (TRUE);
  }
  PeiCsSvcEnableFdWrites (TRUE);

  //
  // Get onboard BIOS base address
  //
  if (((*((UINT32 *)Buffer)) == FLVALSIG) || ((*((UINT32 *)((UINT8 *)Buffer + 0x10))) == FLVALSIG)) {
    WriteAddress   = 0xFFFFFFFF - RecoveryFileSize + 1;
    DescriptorBIOS = TRUE;
  } else {
    WriteAddress = (UINT32 ) FdmGetBaseAddr ();
  }
  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Write ROM base address %x \n", WriteAddress));

  WriteSize = GetFlashBlockSize ();
  SrcAddress = (UINT32) Buffer;
  MicroSecondDelay(50);

//[-start-151229-IB06740454-add]//
  InitFlashMode (&FlashMode);
  FlashMode = FW_FLASH_MODE;
//[-start-170215-IB07400840-modify]//
  if (PcdGet32(PcdFlashSpiRomSize) != 0) {
    WriteAddress = WriteAddress - (UINT32)((UINT64)0x100000000 - PcdGet32(PcdFlashSpiRomSize));
  } else {  
    WriteAddress = WriteAddress - PcdGet32 (PcdSpiFlashBase); //Chang MMIO address to SPI address
  }
//[-end-170215-IB07400840-modify]//
//[-end-151229-IB06740454-add]//
//[-start-160801-IB10860205-add]//
  RecoveryFileSize = RecoveryFileSize - 0x1000; 
//[-end-160801-IB10860205-add]//
  //
  //OEM protect table is empty or table return whole ROM size, PEI Crisis default not protect any region
  //
  if ((OemProtectTable == NULL) ||
      (OemProtectTable[0].WriteSize == 0xFFFFFFFF)) {
    //
    // Update whole FD image
    //
    UseCrisisProtectTable = FALSE;
    DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Write whole image\n"));
  } else {
    //
    //Analyze the OEM protect table
    //
    for (ProtectNumCount = 0; ProtectNumCount < (TableCount - 1); ProtectNumCount++) {
      if ((OemProtectTable[ProtectNumCount].WriteAddress < FdmGetBaseAddr ())) {
        IsMeProtectRegion ++;
      }
      if (OemProtectTable[ProtectNumCount].WriteAddress == RecoveryFvAddr) {
        SkipPeiRegion = TRUE;
      }
    }
    if (DescriptorBIOS == FALSE) {
      //
      //All of the OEM protect region are ME region for Non-ME bin file then it will use kernel default protect region.
      //
      if (IsMeProtectRegion == ((TableCount - 1))) {
        if (((DefaultProtectTableSize / sizeof (FLASH_ENTRY)) == 2) && (DefaultProtectTable->WriteAddress == RecoveryFvAddr)) {
          UseCrisisProtectTable = FALSE;
          SkipPeiRegion         = TRUE;
        } else {
          NumOfProtectRegion = (DefaultProtectTableSize / sizeof (FLASH_ENTRY)) - 1;
          ProtectRegion = DefaultProtectTable;
        }
      } else {
        NumOfProtectRegion = TableCount - 1;
        ProtectRegion      = OemProtectTable;
      }
    } else {
      NumOfProtectRegion = TableCount - 1;
      ProtectRegion      = OemProtectTable;
    }
  }

  if (UseCrisisProtectTable) {
    //
    //Read all of the ROM part bin files
    //
    Status = PeiServicesAllocatePages (
               EfiBootServicesCode,
               (RecoveryFileSize - 1) / 0x1000 + 1,
               &RomBinFile
               );

    if (EFI_ERROR (Status)) {
      goto Done;
    }

    Status = FlashRead (
              (UINT8 *)((UINT32)RomBinFile),
              (UINT8 *)WriteAddress,
              RecoveryFileSize
              );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    for (ProtectNumCount = 0; ProtectNumCount < NumOfProtectRegion; ProtectNumCount++) {
      //
      //Non-ME bin file need not to reference ME protect region
      //
      if ((ProtectRegion[ProtectNumCount].WriteAddress < FdmGetBaseAddr ()) && (DescriptorBIOS == FALSE)) {
        continue;
      } else {
        //
        //Non-ME bin file to reference the BIOS protect region
        //
        if (DescriptorBIOS == FALSE) {
          CopyMem (
            (((UINT8 *)SrcAddress) + ProtectRegion[ProtectNumCount].SourceOffset),
            (((UINT8 *)((UINT32)RomBinFile)) + ProtectRegion[ProtectNumCount].SourceOffset),
            ProtectRegion[ProtectNumCount].WriteSize
          );
        } else {
          //
          //Include ME bin file to reference ME protect region
          //
          if (ProtectRegion[ProtectNumCount].WriteAddress < FdmGetBaseAddr ()) {
            CopyMem (
              (((UINT8 *)SrcAddress) + ProtectRegion[ProtectNumCount].SourceOffset),
              (((UINT8 *)((UINT32)RomBinFile)) + ProtectRegion[ProtectNumCount].SourceOffset),
              ProtectRegion[ProtectNumCount].WriteSize
            );
          } else {
            //
            //Include ME bin file to reference BIOS protect region
            //
            CopyMem (
              (((UINT8 *)SrcAddress) + (ProtectRegion[ProtectNumCount].SourceOffset + (RecoveryFileSize - FdmGetFlashAreaSize()))),
              (((UINT8 *)((UINT32)RomBinFile)) + (ProtectRegion[ProtectNumCount].SourceOffset + (RecoveryFileSize - FdmGetFlashAreaSize ()))),
              ProtectRegion[ProtectNumCount].WriteSize
            );
          }
        }
      }
    }
  }

  if (SkipPeiRegion) {
    RecoveryBlocks = ((RecoveryFileSize - (UINTN) RecoveryFvSize) / WriteSize);
  } else {
    RecoveryBlocks = ((RecoveryFileSize) / WriteSize);
  }

  ReadBuffer = AllocatePool(WriteSize);
  ASSERT(ReadBuffer != NULL);


  for (WriteCount = 0; WriteCount < RecoveryBlocks; WriteCount ++) {
    UINTN Retry;

    POST_CODE (START_FLASH);

    //
    //Make a Beep sound
    //
    if (SpeakerOn && ((WriteCount % 0xF) == 0)) {
      SpeakerPPI->GenerateBeep (1, 500000, 0);
    }

    for (Retry = 0; Retry < FLASH_FAILURE_RETRY_COUNT; Retry++) {
      WriteSize = GetFlashBlockSize();
      Status = FlashErase (WriteAddress, WriteSize);

      if (EFI_ERROR (Status)) {
        continue;
      }

      POST_CODE (ERASE_DONE);

      MicroSecondDelay (50);

      Status = FlashProgram (
                 (UINT8 *)WriteAddress,
                 (UINT8 *)SrcAddress,
                 (UINTN *)&WriteSize,
                 (WriteAddress & ~(0xFFFF))
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }

      Status = FlashRead (
                 ReadBuffer,
                 (UINT8 *)WriteAddress,
                 WriteSize
                 );
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (CompareMem (ReadBuffer, (UINT8 *)SrcAddress, WriteSize) == 0) {
        break;
      }
    }

    POST_CODE (PROGRAM_DONE);

    WriteAddress += WriteSize;
    SrcAddress += WriteSize;

    MicroSecondDelay (50);
  }

  MicroSecondDelay (50);

  DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Flash BIOS Success and Restart System\n"));
  //
  // The status of "EC idle" and  "Flash device be writable" should be cleared, when error occur.
  //
Done:
  ClearFirmwareUpdatingFlag();
  if (ReadBuffer != NULL) {
    FreePool (ReadBuffer);
  }
  if (UseEcIdle) {
    OemSvcEcIdle (FALSE);
  }

  PeiCsSvcEnableFdWrites (FALSE);
  
//[-start-151229-IB06740454-add]//
  FlashMode = FW_DEFAULT_MODE;
//[-end-151229-IB06740454-add]//

  Status = OemSvcPeiCrisisRecoveryReset ();
  if (Status == EFI_SUCCESS) {
    return;
  }
  (*PeiServices)->ResetSystem ((CONST EFI_PEI_SERVICES **)PeiServices);

  //
  // Wait For shutdown
  //
  CpuDeadLoop ();
}
