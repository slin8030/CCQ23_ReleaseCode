/** @file
  This function offers an interface to Hook IHISI Sub function AH=12h,function "GetOemFlashMap"
;******************************************************************************
;* Copyright (c) 2019, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#if 0 // Sample Implementation
#include <Library/IhisiLib.h>
#endif

#include <Library/SmmOemSvcChipsetLib.h>
//[-start-190220-IB07401083-add]//
#ifdef SMART_OEM_FLASH_MAP_SUPPORT
#include <Library/BootMediaLib.h>
#include <Library/DebugLib.h>
#include <Library/FlashRegionLib.h>


EFI_STATUS
SmartGetOemFlashMap (
  IN OUT UINTN                          *RomMapSize,
  IN OUT FBTS_PLATFORM_ROM_MAP          **RomMapBuffer,
  IN OUT UINTN                          *PrivateRomMapSize,
  IN OUT FBTS_PLATFORM_PRIVATE_ROM      **PrivateRomMapBuffer
  );
#endif
//[-end-190220-IB07401083-add]//

#if 0 // Sample Implementation

FBTS_PLATFORM_ROM_MAP mmOemRomMap[] = {
//  {FbtsRomMapDxe,          FixedPcdGet32 (PcdFlashFvMainBase),              FixedPcdGet32 (PcdFlashFvMainSize)                 },
//  {FbtsRomMapNVRam,        FixedPcdGet32 (PcdFlashNvStorageVariableBase),   FixedPcdGet32 (PcdFlashNvStorageVariableSize)      },
//  {FbtsRomMapCpuMicrocode, FixedPcdGet32 (PcdFlashNvStorageMicrocodeBase),  FixedPcdGet32 (PcdFlashNvStorageMicrocodeSize)},
//  {FbtsRomMapFtwState,     FixedPcdGet32 (PcdFlashNvStorageFtwWorkingBase), FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize)},
//  {FbtsRomMapFtwBackup,    FixedPcdGet32 (PcdFlashNvStorageFtwSpareBase),   FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize)},
//  {FbtsRomMapPei,          FixedPcdGet32 (PcdFlashFvRecoveryBase),          FixedPcdGet32 (PcdFlashFvRecoverySize)},
  {FbtsRomMapEos,          0,          0      },
};

//
// Add OEM private rom map table,
//
FBTS_PLATFORM_PRIVATE_ROM mmOemPrivateRomMap[] = {
//  {FixedPcdGet32 (PcdFlashNvStorageMsdmDataBase), FixedPcdGet32 (PcdFlashNvStorageMsdmDataSize)},
  {FbtsRomMapEos,  0},
};

#endif

/**
 This function provides a hook for IHISI sub-function AH=12h and should be implemented to get OemFlashMap. 
 Please refer to Insyde H2O internal Soft-SMI interface (IHISI) Specification.

 @param[in out]     *MediaOemRomMapSize           On entry, the size of FBTS_PLATFORM_ROM_MAP matrix.
                                                  On exit , the size of updated FBTS_PLATFORM_ROM_MAP matrix.
 @param[in out]     **MediaOemRomMap              On entry, points to FBTS_PLATFORM_ROM_MAP matrix.
                                                  On exit , points to updated FBTS_PLATFORM_ROM_MAP matrix.
 @param[in out]     *MediaOemPrivateRomMapSize    On entry, the size of FBTS_PLATFORM_PRIVATE_ROM matrix.
                                                  On exit , the size of updated FBTS_PLATFORM_PRIVATE_ROM matrix.
 @param[in out]     **MediaOemPrivateRomMap       On entry, points to FBTS_PLATFORM_PRIVATE_ROM matrix.
                                                  On exit , points to updated FBTS_PLATFORM_PRIVATE_ROM matrix.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS12HookGetOemFlashMap (
  IN OUT UINTN                          *MediaOemRomMapSize,
  IN OUT FBTS_PLATFORM_ROM_MAP          **MediaOemRomMap,
  IN OUT UINTN                          *MediaOemPrivateRomMapSize,
  IN OUT FBTS_PLATFORM_PRIVATE_ROM      **MediaOemPrivateRomMap
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/
#if 0 // Sample Implementation
  *MediaOemRomMapSize = sizeof (mmOemRomMap) / sizeof (mmOemRomMap[0]);
  (*MediaOemRomMap) = mmOemRomMap;
  *MediaOemPrivateRomMapSize = sizeof (mmOemPrivateRomMap) / sizeof (mmOemPrivateRomMap[0]);
  (*MediaOemPrivateRomMap) = mmOemPrivateRomMap;
#endif
//[-start-190220-IB07401083-modify]//
#ifdef SMART_OEM_FLASH_MAP_SUPPORT
  return SmartGetOemFlashMap (MediaOemRomMapSize, MediaOemRomMap, MediaOemPrivateRomMapSize, MediaOemPrivateRomMap);
#else
  return EFI_UNSUPPORTED;
#endif
//[-end-190220-IB07401083-modify]//
}

//[-start-190220-IB07401083-add]//
#ifdef SMART_OEM_FLASH_MAP_SUPPORT
typedef enum {
  OemDescriptor = 0x00,
  OemNvCommon,
  OemFdmRegion,
  OemBvdtRegion,
  OemDmiRegion,
  OemMsdmRegion,
  OemVarDefault,
  OemOpRomRegion,
  OemObbRegion,
  OemNvRamRegion,
  OemNvStorage,
  OemNvFtwWork,
  OemNvFtwBackup,
  OemFactoryCopy,
  OemTxeDataRegion,
  OemEcRegion,
  OemReserved01,
  OemRegionEos = 0xFF    // End Of Structure
} OEM_REGION_TYPE;

typedef enum {
  OemProtectDisabled = 0x00, // Protect Disabled
  OemNormalProtect,          // Add to OEM Protect Table
  OemPrivateProtect          // Add to Private Protect Table
} OEM_PROTECT_TYPE;

typedef enum {
  FbtsRomMapDescriptor = 0x51, 
  FbtsRomMapFdm, 
  FbtsRomMapBvdt, 
  FbtsRomMapMsdm, 
  FbtsRomMapVarDefault,
  FbtsRomMapOpRom,
  FbtsRomMapTxeData,
  FbtsRomMapReserved01,
} FBTS_ROM_MAP_OEM_DEFINED;

#pragma pack(1)

typedef struct {
  OEM_REGION_TYPE   OemRegion;
  OEM_PROTECT_TYPE  OemProtectType; 
  UINT8             FbstType;    // FBST_ROM_MAP_CODE
  UINT32            Address;
  UINT32            Length;
} OEM_PROTECT_MAP;

#pragma pack()

// _ _ _ _ _ _   _ _ _ _ _ _  ________________ 0
//             |             |                |
// Descriptor  |             |   Descriptor   | 
//   Region    |             |     Region     | 
// _ _ _ _ _ _ | _ _ _ _ _ _ |________________| _ _ _ _ _ _ _
//             |             |                |  
//             |             |  Primary Boot  |  IFWI region
//    Boot     |             |    Partition   | 
// Partition 1 | _ _ _ _ _ _ |________________| 
//  (Signed)   |             |                |
//             |             | Secondary Boot | 
//             |             |    Partition   | 
// _ _ _ _ _ _ | _ _ _ _ _ _ |________________| 
//             |             |                |
//             |             |  Primary Boot  | 
//             |             |Partition Header| 
//             | _ _ _ _ _ _ |________________| 
//             |             |                |
//             |             | Secondary Boot | 
//             |             |Partition Header| 
//             | _ _ _ _ _ _ |________________| 
//             |             |                |
//             |             |   FDM region   | PcdH2OFlashDeviceMapSize
//             |             |________________|
//             |             |                |
//             |             |  BVDT region   | PcdFlashNvStorageBvdtSize
//             |             |________________|
//             |             |                |
//             |             |   DMI region   | PcdFlashNvStorageDmiSize
//             |  NV Common  |________________|
//             |             |                |
//             |             |  MSDM region   | PcdFlashNvStorageMsdmDataSize
//    Boot     | _ _ _ _ _ _ |________________|
// Partition 2 |             |                |
//             |   FV Main   |   OBB region   | PcdFlashFvMainSize
//             | _ _ _ _ _ _ |________________|
//             |             |                |
//             |             |NvStorage region| PcdFlashNvStorageVariableSize (Unused)
//             |             |________________|
//             |             |                |
//             |             | FtwWork region | PcdFlashNvStorageFtwWorkingSize (Unused)
//             |             |________________|
//             |   NvRam     |                |
//             |             |FtwBackup region| PcdFlashNvStorageFtwSpareSize (Unused)
//             |             |________________|
//             |             |                |
//             |             | FactoryCopy RG | PcdFlashNvStorageFactoryCopySize (if PcdH2OSecureBootSupported is enabled)
// _ _ _ _ _ _ | _ _ _ _ _ _ |________________| _ _ _ _ _ _ _
//             |             |                |
// TXE Data RG |             |  TXE Data RG   | 
// _ _ _ _ _ _ | _ _ _ _ _ _ |________________|
//             |             |                |
//   Reserved  |             |    Reserved    | 0x1000
// _ _ _ _ _ _ | _ _ _ _ _ _ |________________|
//

//
// Protect SPI region method
// OemNormalProtect  : Normal protect, this region can be flashed by H2OFFT utiltiy with "-all" command (View table via H2OFFT -pq)
// OemPrivateProtect : Private protect, this region can not be flashed by H2OFFT utility. (View table via H2OFFT -ppriv)
// OemProtectDisabled: Disable protect this region.
//
OEM_PROTECT_MAP mOemProtectTable [] = {   
  {OemDescriptor,     OemNormalProtect,   FbtsRomMapDescriptor,          0x0,    0x0},
  {OemBvdtRegion,     OemProtectDisabled, FbtsRomMapBvdt,                0x0,    0x0},
  {OemDmiRegion,      OemProtectDisabled, FbtsRomMapDmiFru,              0x0,    0x0},
  {OemMsdmRegion,     OemNormalProtect,   FbtsRomMapMsdm,                0x0,    0x0},
  {OemNvStorage,      OemProtectDisabled, FbtsRomMapNvStorage,           0x0,    0x0},
  {OemNvFtwWork,      OemProtectDisabled, FbtsRomMapFtwState,            0x0,    0x0},
  {OemNvFtwBackup,    OemProtectDisabled, FbtsRomMapFtwBackup,           0x0,    0x0},
  {OemFactoryCopy,    OemNormalProtect,   FbtsRomMapFactoryCopy,         0x0,    0x0},
  {OemTxeDataRegion,  OemNormalProtect,   FbtsRomMapTxeData,             0x0,    0x0},
  {OemReserved01,     OemPrivateProtect,  FbtsRomMapReserved01,   0xFFFFF000, 0x1000}
};

FBTS_PLATFORM_ROM_MAP mEmmcRomMap[] = {
// eMMC map.   
 {FbtsRomMapEos,          0,                                 0}
};

FBTS_PLATFORM_PRIVATE_ROM mEmmcPrivateRomMap[] = {
  {FbtsRomMapEos,   0}
};

EFI_STATUS
SpiRomMapAddressUpdate (
  VOID
  )
{
  UINTN   Media_mOemProtectTableSize;
  UINTN   Index;

  Media_mOemProtectTableSize = (sizeof (mOemProtectTable) / sizeof (OEM_PROTECT_MAP));
        

  //
  // Update Protect Table Address
  //
  for (Index = 0; Index < Media_mOemProtectTableSize; Index++) {
    switch (mOemProtectTable[Index].OemRegion) {
    case OemDescriptor:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - PcdGet32(PcdFlashSpiRomSize)); // Flash Base
      mOemProtectTable[Index].Length = 0x1000; 
      break;
    case OemNvCommon:
      break;
    case OemFdmRegion:
      break;
    case OemBvdtRegion:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (((UINT32)FdmGetNAtAddr (&gH2OFlashMapRegionBvdtGuid, 1) & 0xFFFFF000) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageBvdtSize); 
      break;
    case OemDmiRegion:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (((UINT32)FdmGetNAtAddr (&gH2OFlashMapRegionSmbiosUpdateGuid, 1) & 0xFFFFF000) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageDmiSize); 
      break;
    case OemMsdmRegion:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (((UINT32)FdmGetNAtAddr (&gH2OFlashMapRegionMsdmGuid, 1) & 0xFFFFF000) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageMsdmDataSize); 
      break;
//    case OemVarDefault:
//      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
//      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (((UINT32)FdmGetNAtAddr (&gH2OFlashMapRegionVarDefaultGuid, 1) & 0xFFFFF000) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
//      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageVariableDefaultsSize); 
//      break;
    case OemOpRomRegion:
      break;
    case OemObbRegion:
      break;
    case OemNvRamRegion:
      break;
    case OemNvStorage:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (PcdGet32(PcdFlashNvStorageVariableBase) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageVariableSize);
      break;
    case OemNvFtwWork:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (PcdGet32(PcdFlashNvStorageFtwWorkingBase) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageFtwWorkingSize);
      break;
    case OemNvFtwBackup:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
      mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (PcdGet32(PcdFlashNvStorageFtwSpareBase) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageFtwSpareSize);
      break;
    case OemFactoryCopy:
      if (PcdGet32(PcdFlashNvStorageFactoryCopySize) != 0) {
        mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashBootPartition2Base))); // Boot Partition 2 Base
        mOemProtectTable[Index].Address = (UINT32)(mOemProtectTable[Index].Address + (PcdGet32(PcdFlashNvStorageFactoryCopyBase) - PcdGet32(PcdH2OFlashDeviceMapBp2Start)));
        mOemProtectTable[Index].Length = PcdGet32(PcdFlashNvStorageFactoryCopySize);
      }
      break;
    case OemTxeDataRegion:
      mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashDeviceExpansionBase)));
      mOemProtectTable[Index].Length = PcdGet32(PcdFlashDeviceExpansionSize);
      break;
//    case OemEcRegion:
//      if (PcdGet32(PcdFlashEcRegionSize) != 0) {
//        mOemProtectTable[Index].Address = (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashEcRegionBase)));
//        mOemProtectTable[Index].Length = PcdGet32(PcdFlashEcRegionSize);
//      }
//      break;
    default:
      break;
    }
    DEBUG ((EFI_D_ERROR, "Index = %x, OemRegion = %x, Addr = 0x%08x, Len = 0x%08x\n", 
        Index, mOemProtectTable[Index].OemRegion, mOemProtectTable[Index].Address, mOemProtectTable[Index].Length));
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
SmartGetOemFlashMap (
  IN OUT UINTN                          *RomMapSize,
  IN OUT FBTS_PLATFORM_ROM_MAP          **RomMapBuffer,
  IN OUT UINTN                          *PrivateRomMapSize,
  IN OUT FBTS_PLATFORM_PRIVATE_ROM      **PrivateRomMapBuffer
  )
{
  UINTN                         Media_mOemRomMapSize;
  UINTN                         Media_mOemPrivateRomMapSize;
  UINTN                         Media_mOemProtectTableSize;
  FBTS_PLATFORM_ROM_MAP         *Media_mOemRomMap;
  FBTS_PLATFORM_PRIVATE_ROM     *Media_mOemPrivateRomMap;
  UINTN                         Index;
  BOOT_MEDIA_TYPE               BootMedia;
  EFI_STATUS                    Status;

  Media_mOemRomMapSize = 0;
  Media_mOemPrivateRomMapSize = 0;
  
  Status = GetBootMediaType (&BootMedia);
  if (!EFI_ERROR (Status)) {
    if (BootMedia == BootMediaEmmc) {

  
      Media_mOemRomMapSize = sizeof (mEmmcRomMap) / sizeof (mEmmcRomMap[0]);
      if (Media_mOemRomMapSize > (UINTN)(sizeof (FBTS_PLATFORM_ROM_MAP_BUFFER) / sizeof (FBTS_PLATFORM_ROM_MAP))) {
        return EFI_UNSUPPORTED;
      }

      Media_mOemPrivateRomMapSize = sizeof (mEmmcPrivateRomMap) / sizeof (mEmmcPrivateRomMap[0]);
      if (Media_mOemPrivateRomMapSize > (UINTN)(sizeof (FBTS_PLATFORM_PRIVATE_ROM_BUFFER) / sizeof (FBTS_PLATFORM_PRIVATE_ROM))) {
        return EFI_UNSUPPORTED;
      }


      Media_mOemRomMap = (*RomMapBuffer);
      for (Index = 0; Index < Media_mOemRomMapSize; Index++) {
        Media_mOemRomMap[Index].Type = mEmmcRomMap[Index].Type;
        Media_mOemRomMap[Index].Address = mEmmcRomMap[Index].Address;
        Media_mOemRomMap[Index].Length = mEmmcRomMap[Index].Length;
        if (Media_mOemRomMap[Index].Type == FbtsRomMapEos) {
          break;
        }
      }
  
      Media_mOemPrivateRomMap = (*PrivateRomMapBuffer);
      for (Index = 0; Index < Media_mOemPrivateRomMapSize; Index++) {
        Media_mOemPrivateRomMap[Index].LinearAddress = mEmmcPrivateRomMap[Index].LinearAddress;
        Media_mOemPrivateRomMap[Index].Size = mEmmcPrivateRomMap[Index].Size;
        if (Media_mOemPrivateRomMap[Index].LinearAddress == (UINT32)FbtsRomMapEos) {
          break;
        }
      }
    } else {
    
      SpiRomMapAddressUpdate();

      Media_mOemProtectTableSize = (sizeof (mOemProtectTable) / sizeof (OEM_PROTECT_MAP));
      Media_mOemRomMap = (*RomMapBuffer);
      Media_mOemPrivateRomMap = (*PrivateRomMapBuffer);
      Media_mOemRomMapSize = 0;
      Media_mOemPrivateRomMapSize = 0;
      
      for (Index = 0; Index < Media_mOemProtectTableSize; Index++) {
        if ((mOemProtectTable[Index].OemProtectType == OemNormalProtect) &&
            (mOemProtectTable[Index].Length != 0x00)) {
          Media_mOemRomMap[Media_mOemRomMapSize].Type = mOemProtectTable[Index].FbstType;
          Media_mOemRomMap[Media_mOemRomMapSize].Address = mOemProtectTable[Index].Address;
          Media_mOemRomMap[Media_mOemRomMapSize].Length = mOemProtectTable[Index].Length;
          Media_mOemRomMapSize ++;
        } else if ((mOemProtectTable[Index].OemProtectType == OemPrivateProtect)&&
                  (mOemProtectTable[Index].Length != 0x00)) {
          Media_mOemPrivateRomMap[Media_mOemPrivateRomMapSize].LinearAddress = mOemProtectTable[Index].Address;
          Media_mOemPrivateRomMap[Media_mOemPrivateRomMapSize].Size = mOemProtectTable[Index].Length;
          Media_mOemPrivateRomMapSize ++;
        }
      }
      
      Media_mOemRomMap[Media_mOemRomMapSize].Type = FbtsRomMapEos;
      Media_mOemRomMap[Media_mOemRomMapSize].Address = 0;
      Media_mOemRomMap[Media_mOemRomMapSize].Length = 0;   
      Media_mOemRomMapSize++;
        
      Media_mOemPrivateRomMap[Media_mOemPrivateRomMapSize].LinearAddress = FbtsRomMapEos;
      Media_mOemPrivateRomMap[Media_mOemPrivateRomMapSize].Size = 0;   
      Media_mOemPrivateRomMapSize++;
    }
  }
  *RomMapSize = Media_mOemRomMapSize;
  *PrivateRomMapSize = Media_mOemPrivateRomMapSize;
  return EFI_SUCCESS;
}
#endif
//[-end-190220-IB07401083-add]//
