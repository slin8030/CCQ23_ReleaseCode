/** @file
  PlatformBdsLib

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Guid/PlatformInfo.h>
#include <Guid/FileInfo.h>

#include <Library/IoLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BdsCpLib.h>
#include <Library/DxeInsydeChipsetLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/GlobalNvsArea.h>

#include <PlatformBaseAddresses.h>
#include <ScAccess.h>
#include <ChipsetSetupConfig.h>


extern EFI_GUID gEfiOSImageIdGuid;

#define EFI_VMM_MEDIA_FILE_NAME L"\\sl_vmm.efi"
#define ANDROID_MEDIA_FILE_NAME L"\\gummiboot.efi"
#define ANDROID_MEDIA_FILE_NAME_SLE L"\\liveboot.img"
#define MODEM_DBG_MASK                    0xFFFFFFFD

#define PCI_DEVICE_PATH_NODE(Func, Dev) \
  { \
    HARDWARE_DEVICE_PATH, \
    HW_PCI_DP, \
    { \
      (UINT8) (sizeof (PCI_DEVICE_PATH)), \
      (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8) \
    }, \
    (Func), \
    (Dev) \
  }

#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, \
    { \
      END_DEVICE_PATH_LENGTH, \
      0 \
    } \
  }
#define PNPID_DEVICE_PATH_NODE(PnpId) \
  { \
    { \
      ACPI_DEVICE_PATH, \
      ACPI_DP, \
      { \
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), \
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8) \
      } \
    }, \
    EISA_PNP_ID((PnpId)), \
    0 \
  }
#define gPciRootBridge \
  PNPID_DEVICE_PATH_NODE(0x0A03)

//
// Below is the platform PCI device path
//
typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PCI_DEVICE_PATH;
PLATFORM_PCI_DEVICE_PATH mEmmcDevPath = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x10),
  gEndEntire
};


extern
EFI_STATUS
SetAndroidDefaultSettings(
);

UINT16 *mEfiRemovableMediaFileName = NULL;

//
// This function populates
// Global NVS variable OSID
// As 1 if bootloader is a Mirosoft Windows Bootloader.
// As 2 if bootloader is an Android Bootloader.
//
VOID
PopulateOsid(
  IN EFI_FILE_HANDLE                        File,
  IN EFI_GLOBAL_NVS_AREA_PROTOCOL          *GlobalNvsArea
  )
{
  EFI_STATUS                            Status;
  EFI_FILE_INFO                         *EfiFileInfo = NULL;
  UINTN                                 Size = 0;
  UINTN                                 BufferSize = 0;
  UINT8                                 *Buffer = NULL;
  UINTN         Index = 0;
  BOOLEAN       OsImageFound = FALSE;
  UINT16        *mBuffer = NULL;

  //
  // Get the required size to allocate
  //
  Status = File->GetInfo(
                  File,
                  &gEfiFileInfoGuid,
                  &Size,
                  NULL
                  );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Error is expected.  getting size to allocate
    //
    EfiFileInfo = AllocateZeroPool(Size);
    ASSERT(EfiFileInfo != NULL);
    //
    // Now get the information
    //
    Status = File->GetInfo(
                    File,
                    &gEfiFileInfoGuid,
                    &Size,
                    EfiFileInfo
                    );
    DEBUG ((EFI_D_INFO, "[PopulateOsid] - GetInfo() Status = %r\n", Status));
    if (EFI_ERROR(Status) && (EfiFileInfo != NULL)) {
      FreePool(EfiFileInfo);
      return;
    }
  }
  
  DEBUG ((EFI_D_INFO, "[PopulateOsid] - EfiFileInfo->FileSize = %d\n", EfiFileInfo->FileSize));
  Buffer = AllocateZeroPool((UINTN) EfiFileInfo->FileSize);
  if (Buffer == NULL) {
    FreePool(EfiFileInfo);
    return;
  }

  for (Index = 0; Index < EfiFileInfo->FileSize; Index++) {
    BufferSize = 1;
    Status = File->Read(File, &BufferSize, (Buffer + Index));
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[PopulateOsid] - Read() failed. Status = 0x%x\n", Status));
      break;
    }

    if (*(Buffer + Index) == 't' && *(Buffer + Index - 1) == 'f' && *(Buffer + Index - 2) == 'o' &&
    *(Buffer + Index - 3) == 's' && *(Buffer + Index - 4) == 'o' && *(Buffer + Index - 5) == 'r' &&
    *(Buffer + Index - 6) == 'c' && *(Buffer + Index - 7) == 'i' && *(Buffer + Index - 8) == 'M') {
      GlobalNvsArea->Area->OSImageId = 1;
      OsImageFound = TRUE;
      DEBUG ((EFI_D_INFO, "[PopulateOsid] - Found Microsoft Windows Bootloader.\n"));
      break;
    }

    mBuffer = (UINT16 *)(Buffer + Index);

    if (*(mBuffer) == L'r' && *(mBuffer - 1) == L'e' && *(mBuffer - 2) == L'g' &&
    *(mBuffer - 3) == L'n' && *(mBuffer - 4) == L'i' && *(mBuffer - 5) == L'l' &&
    *(mBuffer - 6) == L'f' && *(mBuffer - 7) == L'l' && *(mBuffer - 8) == L'e' &&
    *(mBuffer - 9) == L'n' && *(mBuffer - 10) == L'r' && *(mBuffer - 11) == L'e' &&
    *(mBuffer - 12) == L'k') {
      GlobalNvsArea->Area->OSImageId = 4;
      OsImageFound = TRUE;
      DEBUG ((EFI_D_INFO, "[PopulateOsid] - Found GMIN Bootloader.\n"));
      break;
    }
  }

  if (!OsImageFound) {
    GlobalNvsArea->Area->OSImageId = 2;
    DEBUG ((EFI_D_INFO, "[PopulateOsid] - Found Legacy Android Bootloader.\n"));
  }

  FreePool(Buffer);
  FreePool(EfiFileInfo);
}

/**

  This routine is called to check which OS image is present in the EMMC
  and update the OSId based on the OS present.
  OSId = 1 for Windows OS
  OSId = 2 for Android OS

**/
VOID 
BdsIdentifyOSImage(
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL          *GlobalNvsArea;
  
  CHAR16*                               OSFileNames[] = {
                                                          EFI_REMOVABLE_MEDIA_FILE_NAME,
                                                          ANDROID_MEDIA_FILE_NAME,
                                                          ANDROID_MEDIA_FILE_NAME_SLE
                                                       };

CHIPSET_CONFIGURATION     *SystemConfiguration;
#if (_PSSD_FIX_)
  BOOLEAN SLE_Android = FALSE;
#endif

  EFI_HANDLE                            *HandleArray;
  UINTN                                 HandleArrayCount;
  UINTN                                 Index;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Fs;
  EFI_FILE_HANDLE                       Root;
  EFI_FILE_HANDLE                       File;
  EFI_PLATFORM_INFO_HOB                 *PlatformInfo=NULL;
  EFI_PEI_HOB_POINTERS                  GuidHob;
  UINTN                                 VarSize;
  UINT8                                 KnownFilenameIndex;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  BOOLEAN                         FoundEmmcPartition = FALSE;
  DEBUG((EFI_D_INFO, "Bds Identify OS Image\n"));
#ifdef SILENT_LAKE_ENABLE
  EFI_SILENT_LAKE_PROTOCOL        *SilentLakeProtocol;
#endif

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **)&GlobalNvsArea
                  );
  DEBUG((EFI_D_INFO, "Bds Identify OS Image Status %r\n", Status));

  ASSERT_EFI_ERROR(Status);

  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
      PlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &HandleArrayCount, &HandleArray);
  DEBUG((EFI_D_INFO, "LocateHandleBuffer %r\n", Status));
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_INFO, "BdsIdentifyOsImage() - LocateHandleBuffer() failed. Status = %r\n", Status));
    return;
  }
  for (Index = 0; Index < HandleArrayCount; Index++) {
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);
    if (EFI_ERROR (Status)) {
      continue;
    }
    if (CompareMem(DevicePath, &mEmmcDevPath, sizeof(mEmmcDevPath) - 4) == 0) {
      FoundEmmcPartition = TRUE;
      DEBUG((EFI_D_INFO, "Found eMMC partition!\n"));
	    break;
    } 
  }
  //
  // Find bootable image in eMMC partition.
  //
  if(FoundEmmcPartition) {
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Fs);
     DEBUG((EFI_D_INFO, "HandleProtocol %r\n", Status));
    if (EFI_ERROR (Status)) {
      return;
    }
    Status = Fs->OpenVolume (Fs, &Root);
    if (EFI_ERROR (Status)) {
      return;
    }

      for (KnownFilenameIndex = 0; KnownFilenameIndex < sizeof(OSFileNames)/sizeof(OSFileNames[0]); KnownFilenameIndex++) {
      Status = Root->Open (Root, &File, OSFileNames[KnownFilenameIndex], EFI_FILE_MODE_READ, 0);
      DEBUG((EFI_D_INFO, " Root->Open %r\n", Status));
      if (EFI_ERROR (Status)) {
      continue;
    }

#if (_PSSD_FIX_)
    Status = Root->Open (Root, &File, OSFileNames[KnownFilenameIndex + 2], EFI_FILE_MODE_READ, 0);
    if (!EFI_ERROR (Status)) {
          GlobalNvsArea->Area->OSImageId = 3;
    }
#endif

     PopulateOsid(File, GlobalNvsArea);
     Status = Root->Close (Root);
     mEfiRemovableMediaFileName = OSFileNames[KnownFilenameIndex];
#ifdef SILENT_LAKE_ENABLE
      if (StrCmp (mEfiRemovableMediaFileName, EFI_VMM_MEDIA_FILE_NAME) == 0) {
        //
        // Force VMM as GMIN
        //
        GlobalNvsArea->Area->OSImageId = 4;
        Status = gBS->LocateProtocol(
                        &gEfiSilentLakeProtocolGuid,
                        NULL,
                        (VOID **)&SilentLakeProtocol
                        );
      
        if (!EFI_ERROR (Status)) {
          //
          // Note: GetVmmInfo needed to be executed before SEC sends EndOfPost command.
          //
          DEBUG((EFI_D_INFO, "Locate gEfiSilentLakeProtocolGuid success\n"));
          Status = SilentLakeProtocol->GetVmmInfo (SilentLakeProtocol);
        }
      }
#endif

    break;
    }

  DEBUG((EFI_D_INFO, "mEfiRemovableMediaFileName = %S\n",mEfiRemovableMediaFileName));
  DEBUG((EFI_D_INFO, "GlobalNvsArea->Area->OSImageId = %d\n", GlobalNvsArea->Area->OSImageId));
   
  VarSize = PcdGet32 (PcdSetupConfigSize);
  SystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VarSize);  

    Status = gRT->GetVariable(
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    NULL,
                    &VarSize,
                    SystemConfiguration
                    );
    ASSERT_EFI_ERROR(Status);
	
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe (SystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
    }

    if (GlobalNvsArea->Area->OSImageId != 0 && SystemConfiguration->OsSelection != GlobalNvsArea->Area->OSImageId) { 
      //
      // Configuration changed, Set OS default configurations
      //
      SystemConfiguration->OsSelection = GlobalNvsArea->Area->OSImageId;

      if ((GlobalNvsArea->Area->OSImageId == 2) || (GlobalNvsArea->Area->OSImageId == 4)) { // Android Legacy and GMIN
        SystemConfiguration->PanelConfig = 0;
        SystemConfiguration->PanelVendor = 0;
        SystemConfiguration->FirstBootIndicator = 1;
        if (PlatformInfo != NULL) {
          if (PlatformInfo->BoardId == BOARD_ID_CHV_T) {
            SystemConfiguration->BTHStatus = 2; // Expose WIFI/BT 4354 module / 2E64 Setup option as default for CHT RVP
          }
        }
      }

      Status = gRT->SetVariable(
                      SETUP_VARIABLE_NAME,
                      &gSystemConfigurationGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS|EFI_VARIABLE_RUNTIME_ACCESS,
                      VarSize,
                      SystemConfiguration
                      );
      ASSERT_EFI_ERROR(Status);
      gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
    }

    //
    //  HSD TABLET_PLATFORM [sighting]   [5088700][Rev:3][New][To support OS_TYPE pin on touch]
    //
    if ((GlobalNvsArea->Area->OSImageId == 2)|| (GlobalNvsArea->Area->OSImageId == 4)) { // Android Legacy and GMIN
    // OS_TYPE/I2C_MODE GPIO pin connect to touch panel, it's polarity decide which OS the touch FW will serve, low for windows & high for android.
    // Setting GPIO State to HIGH in Android Only
    //
    if(PlatformInfo != NULL) {
      if (PlatformInfo->BoardId == BOARD_ID_CHT_FFD) {
          MmioOr32(IO_BASE_ADDRESS+R_PCH_CFIO_NORTH + 0x5400, BIT1);
        } else if (PlatformInfo->BoardId == BOARD_ID_CHT_CR) {
          MmioOr32(IO_BASE_ADDRESS + R_PCH_CFIO_NORTH + 0x5458, BIT1);  // OS_SEL : N61
          MmioOr32(IO_BASE_ADDRESS + R_PCH_CFIO_NORTH + 0x4810, BIT22 | BIT23); // TOUCH_INT : N17
    } else {
          MmioOr32(IO_BASE_ADDRESS+R_PCH_CFIO_NORTH +0x4838, BIT1);
          //
          // HSD[5284562] - [AOS Bx] Touchscreen gpio (SE77) should be configured to 1K pull up
          //
          MmioOr32(IO_BASE_ADDRESS + R_PCH_CFIO_SOUTHEAST + 0x5810, BIT22 | BIT23); // Touch_INT_N (GPIO_SE_77)		  
    }
    }

      //
      // [Cherrytrail Tablet Platform]  [5089697][Rev:5][Assigned][[AOS][CHT PO A0] MODEM_DBG_SEL GPIO should be modified to rout USB-HS modem signal to CHT-RVP usb mdm debug]
      // MODEM_DBG_SEL_GPIO should be High for Windows(USB WWAN), Low for Android(Modem Debug)
      //
      MmioAnd32((IO_BASE_ADDRESS+R_PCH_CFIO_SOUTHWEST + 0x4420) , MODEM_DBG_MASK);
        
    //
    // HSD[5218583] Needs BIOS to add two more GPIO resource for Android SD ACPI table
    // Set SDMMC3_1P8_EN (GPIO_SE_85) and SDMMC3_PWR_EN (GPIO_SE_78) to be normal GPIO
    //
    MmioAnd32(IO_BASE_ADDRESS + R_PCH_CFIO_SOUTHEAST + 0x5850, (UINT32)~(BIT19 | BIT18 | BIT17 | BIT16) ); // SDMMC3_1P8_EN (GPIO_SE_85)
    MmioOr32(IO_BASE_ADDRESS + R_PCH_CFIO_SOUTHEAST + 0x5850, BIT15 ); 
    MmioAnd32(IO_BASE_ADDRESS + R_PCH_CFIO_SOUTHEAST + 0x5818, (UINT32)~(BIT19 | BIT18 | BIT17 | BIT16) ); // SDMMC3_PWR_EN (GPIO_SE_78)
    MmioOr32(IO_BASE_ADDRESS + R_PCH_CFIO_SOUTHEAST + 0x5818, BIT15); 
    //
    //N00: GPIO_DFX0 - Set high for Android WWAN enumeration
    //
    MmioWrite32(IO_BASE_ADDRESS + R_PCH_CFIO_NORTH + 0x4400, 0x00108102);

    } else if (GlobalNvsArea->Area->OSImageId == 0) {
      //
      // DnX Fastboot Requirement - Case 1: BIOS is not able to locate <OSloader>.efi into any eMMC ESP
      //
      DEBUG((EFI_D_INFO, "BdsIdentifyOSImage(): No bootloaders found in any of the EFI system partition. Going to DnX Fastboot mode.\n"));
    ///  PcdSetBool(PcdDnxFastboot, TRUE);
    }
  }
}

VOID
BeforeReadyToBootCpHandler (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE Handle
  ) 
{
  BdsIdentifyOSImage();
}
