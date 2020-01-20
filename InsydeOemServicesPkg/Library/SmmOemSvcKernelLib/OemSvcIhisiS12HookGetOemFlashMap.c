/** @file
  This function offers an interface to Hook IHISI Sub function AH=12h,function "GetOemFlashMap"
;******************************************************************************
;* Copyright (c) 2015 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmOemSvcKernelLib.h>
#include <Library/FlashRegionLib.h>

// #if 0 // Sample Implementation
// 
// FBTS_PLATFORM_ROM_MAP mmOemRomMap[] = {
//   {FbtsRomMapDxe,          FixedPcdGet32 (PcdFlashFvMainBase),              FixedPcdGet32 (PcdFlashFvMainSize)              },
//   {FbtsRomMapNVRam,        FixedPcdGet32 (PcdFlashNvStorageVariableBase),   FixedPcdGet32 (PcdFlashNvStorageVariableSize)   },
//   {FbtsRomMapCpuMicrocode, FixedPcdGet32 (PcdFlashNvStorageMicrocodeBase),  FixedPcdGet32 (PcdFlashNvStorageMicrocodeSize)  },
//   {FbtsRomMapFtwState,     FixedPcdGet32 (PcdFlashNvStorageFtwWorkingBase), FixedPcdGet32 (PcdFlashNvStorageFtwWorkingSize) },
//   {FbtsRomMapFtwBackup,    FixedPcdGet32 (PcdFlashNvStorageFtwSpareBase),   FixedPcdGet32 (PcdFlashNvStorageFtwSpareSize)   },
//   {FbtsRomMapPei,          FixedPcdGet32 (PcdFlashFvRecoveryBase),          FixedPcdGet32 (PcdFlashFvRecoverySize)          },
//   {FbtsRomMapEos,          0,          0      },
// };
// 
// //
// // Add OEM private rom map table,
// //
// FBTS_PLATFORM_PRIVATE_ROM mmOemPrivateRomMap[] = {
//   {FixedPcdGet32 (PcdFlashNvStorageDmiBase),      FixedPcdGet32 (PcdFlashNvStorageDmiSize)},
//   {FixedPcdGet32 (PcdFlashNvStorageMsdmDataBase), FixedPcdGet32 (PcdFlashNvStorageMsdmDataSize)},
//   {FbtsRomMapEos,  0},
// };
// #endif

// #if 0   // FDM Implementation Sample
// 
// FBTS_PLATFORM_ROM_MAP mmOemRomMap[] = {
//   {FbtsRomMapDxe,          0,          0      },
//   {FbtsRomMapNVRam,        0,          0      },
//   {FbtsRomMapCpuMicrocode, 0,          0      },
//   {FbtsRomMapFtwState,     0,          0      },
//   {FbtsRomMapFtwBackup,    0,          0      },
//   {FbtsRomMapPei,          0,          0      },
//   {FbtsRomMapEos,          0,          0      }
// };
// 
// //
// // Add OEM private rom map table,
// //
// FBTS_PLATFORM_PRIVATE_ROM mmOemPrivateRomMap[] = {
//   {0,              0 },
//   {FbtsRomMapEos,  0 },
// };
// 
// EFI_STATUS
// UpdateDefaultRomMap (
//   )
// {
// 
//   //
//   // Update default protect rom map
//   //
//   mmOemRomMap[0].Type    = FbtsRomMapDxe;
//   mmOemRomMap[0].Address = (UINT32) FdmGetAddressById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionDxeFvGuid, 1);
//   mmOemRomMap[0].Length  = (UINT32) FdmGetSizeById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionDxeFvGuid, 1);
// 
//   mmOemRomMap[1].Type    = FbtsRomMapNVRam;
//   mmOemRomMap[1].Address = (UINT32) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1);
//   mmOemRomMap[1].Length  = (UINT32) FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_WORKING, 1);
// 
//   mmOemRomMap[2].Type    = FbtsRomMapCpuMicrocode;
//   mmOemRomMap[2].Address = (UINT32) FdmGetNAtAddr (&gH2OFlashMapRegionMicrocodeGuid, 1);
//   mmOemRomMap[2].Length  = (UINT32) FdmGetNAtSize (&gH2OFlashMapRegionMicrocodeGuid, 1);
// 
//   mmOemRomMap[3].Type    = FbtsRomMapFtwState;
//   mmOemRomMap[3].Address = (UINT32) FdmGetNAtAddr (&gH2OFlashMapRegionFtwStateGuid, 1);
//   mmOemRomMap[3].Length  = (UINT32) FdmGetNAtSize (&gH2OFlashMapRegionFtwStateGuid, 1);
// 
//   mmOemRomMap[4].Type    = FbtsRomMapFtwBackup;
//   mmOemRomMap[4].Address = (UINT32) FdmGetNAtAddr (&gH2OFlashMapRegionFtwStateGuid, 1);
//   mmOemRomMap[4].Length  = (UINT32) FdmGetNAtSize (&gH2OFlashMapRegionFtwStateGuid, 1);
// 
//   mmOemRomMap[5].Type    = FbtsRomMapPei;
//   mmOemRomMap[5].Address = (UINT32) FdmGetAddressById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionPeiFvGuid, 1);
//   mmOemRomMap[5].Length  = (UINT32) FdmGetSizeById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionPeiFvGuid, 1);
// 
//   //
//   // Update default private rom map
//   //
//   mmOemPrivateRomMap[0].LinearAddress = (UINT32) FdmGetNAtAddr (&gH2OFlashMapRegionMsdmGuid, 1);
//   mmOemPrivateRomMap[0].Size = (UINT32) FdmGetNAtSize (&gH2OFlashMapRegionMsdmGuid, 1);
// 
//   return EFI_SUCCESS;
// }
// #endif

/**
 This function offers an interface to Hook IHISI Sub function AH=12h,function "GetOemFlashMap"

 @param[in out]     *Media_mOemRomMapSize         On entry, the size of FBTS_PLATFORM_ROM_MAP matrix.
                                                  On exit , the size of updated FBTS_PLATFORM_ROM_MAP matrix.
 @param[in out]     **Media_mOemRomMap            On entry, points to FBTS_PLATFORM_ROM_MAP matrix.
                                                  On exit , points to updated FBTS_PLATFORM_ROM_MAP matrix.
 @param[in out]     *Media_mOemPrivateRomMapSize  On entry, the size of FBTS_PLATFORM_PRIVATE_ROM matrix.
                                                  On exit , the size of updated FBTS_PLATFORM_PRIVATE_ROM matrix.
 @param[in out]     **Media_mOemPrivateRomMap     On entry, points to FBTS_PLATFORM_PRIVATE_ROM matrix.
                                                  On exit , points to updated FBTS_PLATFORM_PRIVATE_ROM matrix.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter or hook code.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS12HookGetOemFlashMap (
  IN OUT UINTN                          *Media_mOemRomMapSize,
  IN OUT FBTS_PLATFORM_ROM_MAP          **Media_mOemRomMap,
  IN OUT UINTN                          *Media_mOemPrivateRomMapSize,
  IN OUT FBTS_PLATFORM_PRIVATE_ROM      **Media_mOemPrivateRomMap
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/
//   FBTS_PLATFORM_ROM_MAP          *FbtsPlatformRomMapPtr;
//   FBTS_PLATFORM_PRIVATE_ROM      *FbtsPlatformRomPrivateRomPtr;
//   UINTN                          OemRomMapSize;
//   UINTN                          OemPrivateRomMapSize;
//   UINTN                          Index;
//   UINTN                          OldIndex;
//   UINTN                          OldRomMapSize;
//   UINTN                          OldPrivateRomMapSize;
//   
//   //=============================================================================
//   // Overlap Sample Code: Overlap EDI & ESI pointer 
//   //=============================================================================
//   OemRomMapSize        = sizeof (mmOemRomMap) / sizeof (mmOemRomMap[0]);
//   OemPrivateRomMapSize = sizeof (mmOemPrivateRomMap) / sizeof (mmOemPrivateRomMap[0]);
//   
//   if (OemRomMapSize > (UINTN)(sizeof (FBTS_PLATFORM_ROM_MAP_BUFFER) / sizeof (FBTS_PLATFORM_ROM_MAP))) {
//     return EFI_UNSUPPORTED;
//   }
// 
//   if (OemPrivateRomMapSize > (UINTN)(sizeof (FBTS_PLATFORM_PRIVATE_ROM_BUFFER) / sizeof (FBTS_PLATFORM_PRIVATE_ROM))) {
//     return EFI_UNSUPPORTED;
//   }
//   
//   *Media_mOemRomMapSize        = OemRomMapSize;
//   *Media_mOemPrivateRomMapSize = OemPrivateRomMapSize;
// 
// // #if 0 // FDM Implementation Sample
// //   UpdateDefaultRomMap ();
// // #endif
//   FbtsPlatformRomMapPtr = (*Media_mOemRomMap);
//   for (Index = 0; Index < OemRomMapSize; Index++) {
//     FbtsPlatformRomMapPtr[Index].Type = mmOemRomMap[Index].Type;
//     FbtsPlatformRomMapPtr[Index].Address = mmOemRomMap[Index].Address;
//     FbtsPlatformRomMapPtr[Index].Length = mmOemRomMap[Index].Length;
//     if (FbtsPlatformRomMapPtr[Index].Type == FbtsRomMapEos) {
//       break;
//     }
//   }
// 
//   FbtsPlatformRomPrivateRomPtr = (*Media_mOemPrivateRomMap);
//   for (Index = 0; Index < OemPrivateRomMapSize; Index++) {
//     FbtsPlatformRomPrivateRomPtr[Index].LinearAddress = mmOemPrivateRomMap[Index].LinearAddress;
//     FbtsPlatformRomPrivateRomPtr[Index].Size = mmOemPrivateRomMap[Index].Size;
//     if (FbtsPlatformRomPrivateRomPtr[Index].LinearAddress == (UINT32)FbtsRomMapEos) {
//       break;
//     }
//   }
//   //=============================================================================
//   // Overlap Sample Code End 
//   //=============================================================================
// 
//   //=============================================================================
//   // Merge Sample Code: Append OEM's protect table to EDI & ESI pointer
//   //=============================================================================
//   OldRomMapSize        = *Media_mOemRomMapSize;
//   OldPrivateRomMapSize = *Media_mOemPrivateRomMapSize;
//   
//   OemRomMapSize        = sizeof (mmOemRomMap) / sizeof (mmOemRomMap[0]);
//   OemPrivateRomMapSize = sizeof (mmOemPrivateRomMap) / sizeof (mmOemPrivateRomMap[0]);
//   
// // #if 0 // FDM Implementation Sample
// //   UpdateDefaultRomMap ();
// // #endif
//   FbtsPlatformRomMapPtr = (*Media_mOemRomMap);
//   for (OldIndex = OldRomMapSize, Index = 0; Index < OemRomMapSize; Index++) {
//     FbtsPlatformRomMapPtr[OldIndex + Index].Type = mmOemRomMap[Index].Type;
//     FbtsPlatformRomMapPtr[OldIndex + Index].Address = mmOemRomMap[Index].Address;
//     FbtsPlatformRomMapPtr[OldIndex + Index].Length = mmOemRomMap[Index].Length;
//     if (FbtsPlatformRomMapPtr[OldIndex + Index].Type == FbtsRomMapEos) {
//       break;
//     }
//   }
// 
//   FbtsPlatformRomPrivateRomPtr = (*Media_mOemPrivateRomMap);
//   for (OldIndex = OldPrivateRomMapSize, Index = 0; Index < OemPrivateRomMapSize; Index++) {
//     FbtsPlatformRomPrivateRomPtr[OldIndex + Index].LinearAddress = mmOemPrivateRomMap[Index].LinearAddress;
//     FbtsPlatformRomPrivateRomPtr[OldIndex + Index].Size = mmOemPrivateRomMap[Index].Size;
//     if (FbtsPlatformRomPrivateRomPtr[OldIndex + Index].LinearAddress == (UINT32)FbtsRomMapEos) {
//       break;
//     }
//   }
//   
//   *Media_mOemRomMapSize = OldRomMapSize + OemRomMapSize;
//   *Media_mOemPrivateRomMapSize = OldPrivateRomMapSize + OemPrivateRomMapSize;
//   //=============================================================================
//   // Merge Sample Code End
//   //=============================================================================
// 
//   return EFI_MEDIA_CHANGED;
  return EFI_UNSUPPORTED;
}


