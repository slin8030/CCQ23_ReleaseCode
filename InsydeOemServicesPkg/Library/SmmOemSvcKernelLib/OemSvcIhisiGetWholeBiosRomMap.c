/** @file
  Provide OEMs to report BIOS ROM map to IHISI interface

;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmOemSvcKernelLib.h>
#include <Library/PcdLib.h>
#include <Library/FlashRegionLib.h>



//
// Sample source for OemSvcIhisiGetWholeBiosRomMap()
//
//
//extern FBTS_INTERNAL_BIOS_ROM_MAP mBiosRomMap[];
///**
//This service provides OEM to report BIOS ROM map to IHISI interface
//
//@param  BiosRomMap            pointer to the FBTS_INTERNAL_BIOS_ROM_MAP array with the
//                              last entry ends with FbtsRomMapEos region type
//@param  NumberOfRegions       The number of regions of the BiosRomMap array
//
//@retval EFI_UNSUPPORTED       Returns unsupported by default.
//@retval EFI_SUCCESS           The service is customized in the project.
//@retval EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed.
//@retval Others                Depends on customization.
//**/
//EFI_STATUS
//OemSvcIhisiGetWholeBiosRomMap (
//OUT VOID                          **BiosRomMap,
//OUT UINTN                         *NumberOfRegions
//)
//{
//UINTN  Index;
//UINT32 NvStorageRegionSize;
//
//if ((BiosRomMap == NULL) || (NumberOfRegions == NULL)) {
//  return EFI_INVALID_PARAMETER;
//}
//
//#define ADD_ROM_MAP_ENTRY(RegionType, RegionAddress, RegionSize, RegionAttr)
//          mBiosRomMap[Index].Type = (UINT8)RegionType;
//          mBiosRomMap[Index].Address = RegionAddress;
//          mBiosRomMap[Index].Size = RegionSize;
//          mBiosRomMap[Index].Attribute = RegionAttr;
//          Index++;
//          __COUNTER__
//
//#define ADD_ROM_MAP_ENTRY_FROM_PCD(RegionType, AddressPcd, SizePcd, RegionAttr)
//        if (PcdGet32(SizePcd) > 0) {
//          ADD_ROM_MAP_ENTRY(RegionType, PcdGet32(AddressPcd), PcdGet32(SizePcd), RegionAttr);
//        }
//
//Index = 0;
//ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapDxe,          PcdFlashFvMainBase,               PcdFlashFvMainSize,               0);
//ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapCpuMicrocode, PcdFlashNvStorageMicrocodeBase,   PcdFlashNvStorageMicrocodeSize,   0);
//ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapDmiFru,       PcdFlashNvStorageDmiBase,         PcdFlashNvStorageDmiSize,         0);
//ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapOemData,      PcdFlashNvStorageMsdmDataBase,    PcdFlashNvStorageMsdmDataSize,    0);
//ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapOemData,      PcdFlashNvStorageBvdtBase,        PcdFlashNvStorageBvdtSize,        0);
//ADD_ROM_MAP_ENTRY_FROM_PCD (FbtsRomMapPei,          PcdFlashFvRecoveryBase,           PcdFlashFvRecoverySize,           0);
//
//NvStorageRegionSize = PcdGet32 (PcdFlashNvStorageVariableSize) + PcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
//                      PcdGet32 (PcdFlashNvStorageFtwSpareSize) + PcdGet32 (PcdFlashNvStorageFactoryCopySize);
//if (NvStorageRegionSize > 0) {
//  ADD_ROM_MAP_ENTRY (FbtsRomMapNvStorage, PcdGet32 (PcdFlashNvStorageVariableBase), NvStorageRegionSize, 0);
//}
//
////
//// End of ROM map
////
//ADD_ROM_MAP_ENTRY (FbtsRomMapEos, 0, 0, 0);
//
//*BiosRomMap = (VOID *)mBiosRomMap;
//*NumberOfRegions = Index;
//
//return EFI_MEDIA_CHANGED;
//}
//
//
////
//// the mBiosRomMap is declared after OemSvcIhisiGetWholeRomMap() on purpose to
//// ensure the array size of mBiosRomMap is enough, DO NOT move it to the top
////
//FBTS_INTERNAL_BIOS_ROM_MAP mBiosRomMap[__COUNTER__];

//
// 
// Sample source for OemSvcIhisiGetWholeBiosRomMap() with FDM.
//
//
///**
//  This service provides OEM to report BIOS ROM map to IHISI interface
//
//  @param  BiosRomMap            pointer to the FBTS_INTERNAL_BIOS_ROM_MAP array with the
//                                last entry ends with FbtsRomMapEos region type
//  @param  NumberOfRegions       The number of regions of the BiosRomMap array
//
//  @retval EFI_UNSUPPORTED       Returns unsupported by default.
//  @retval EFI_SUCCESS           The service is customized in the project.
//  @retval EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed.
//  @retval Others                Depends on customization.
//**/
//EFI_STATUS
//OemSvcIhisiGetWholeBiosRomMap (
//  OUT VOID                          **BiosRomMap,
//  OUT UINTN                         *NumberOfRegions
//  )
//{
//  UINTN  Index;
//  extern FBTS_INTERNAL_BIOS_ROM_MAP mBiosRomMap[];
//  UINT32 NvStorageRegionSize;
//  UINTN  Conuter;
//
//  if ((BiosRomMap == NULL) || (NumberOfRegions == NULL)) {
//    return EFI_INVALID_PARAMETER;
//  }
//
//#define ADD_ROM_MAP_ENTRY(RegionType, RegionAddress, RegionSize, RegionAttr)
//            mBiosRomMap[Index].Type = (UINT8)RegionType;
//            mBiosRomMap[Index].Address = RegionAddress;
//            mBiosRomMap[Index].Size = RegionSize;
//            mBiosRomMap[Index].Attribute = RegionAttr;
//            Index++;
//            Conuter = __COUNTER__;
//
//#define ADD_ROM_MAP_ENTRY_FROM_PCD(RegionType, AddressPcd, SizePcd, RegionAttr)
//          if (PcdGet32(SizePcd) > 0) {
//            ADD_ROM_MAP_ENTRY(RegionType, PcdGet32(AddressPcd), PcdGet32(SizePcd), RegionAttr);
//          }
//
//
//#define ADD_ROM_MAP_ENTRY_FROM_FDM(RegionType, FdmRegionType, Instance, RegionAttr)
//          {
//            UINT64  Size;
//            if ((Size = FdmGetNAtSize(&FdmRegionType, Instance)) > 0) {
//              ADD_ROM_MAP_ENTRY(RegionType, (UINT32) FdmGetNAtAddr(&FdmRegionType, Instance), (UINT32) Size, RegionAttr);
//            }
//          }
//
//#define ADD_DXE_FV_ROM_ENTRY_FROM_FDM(RegionAttr)
//          {
//            UINT64  Size;
//            if ((Size = FdmGetSizeById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionDxeFvGuid, 1)) > 0) {
//              ADD_ROM_MAP_ENTRY(FbtsRomMapDxe, (UINT32) FdmGetAddressById(&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionDxeFvGuid, 1), (UINT32) Size, RegionAttr);
//            }
//          }
//
//#define ADD_PEI_FV_ROM_ENTRY_FROM_FDM(RegionAttr)
//          {
//            UINT64  Size;
//            if ((Size = FdmGetSizeById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionPeiFvGuid, 1)) > 0) {
//              ADD_ROM_MAP_ENTRY(FbtsRomMapPei, (UINT32) FdmGetAddressById(&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionPeiFvGuid, 1), (UINT32) Size, RegionAttr);
//            }
//          }
//
//  Index = 0;
//  ADD_DXE_FV_ROM_ENTRY_FROM_FDM   (0);
//  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapCpuMicrocode, gH2OFlashMapRegionMicrocodeGuid,        1,   0);
//  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapOemData,      gH2OFlashMapRegionBvdtGuid,             1,   0);
//  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapDmiFru,       gH2OFlashMapRegionSmbiosUpdateGuid,     1,   0);
//  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapOemData,      gH2OFlashMapRegionMsdmGuid,             1,   0);
//  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapOemData,      gH2OFlashMapRegionBvdtGuid,             1,   0);
//  ADD_PEI_FV_ROM_ENTRY_FROM_FDM   (0);
//
//  NvStorageRegionSize = (UINT32) FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_WORKING, 1) + 
//                        (UINT32) FdmGetNAtSize (&gH2OFlashMapRegionFtwStateGuid,1) +
//                        (UINT32) FdmGetNAtSize (&gH2OFlashMapRegionFtwBackupGuid, 1) + 
//                        (UINT32) FdmGetVariableSize (FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY, 1);
//  if (NvStorageRegionSize > 0) {
//    ADD_ROM_MAP_ENTRY (FbtsRomMapNvStorage, (UINT32) FdmGetVariableAddr (FDM_VARIABLE_DEFAULT_ID_WORKING, 1), NvStorageRegionSize, 0);
//  }
//
//  //
//  // End of ROM map
//  //
//  ADD_ROM_MAP_ENTRY (FbtsRomMapEos, 0, 0, 0);
//
//  *BiosRomMap = (FBTS_INTERNAL_BIOS_ROM_MAP *)mBiosRomMap;
//  *NumberOfRegions = Index;
// 
//  return EFI_MEDIA_CHANGED;
//}
////
//// the mBiosRomMap is declared after OemSvcIhisiGetWholeRomMap() on purpose to
//// ensure the array size of mBiosRomMap is enough, DO NOT move it to the top
////
//FBTS_INTERNAL_BIOS_ROM_MAP mBiosRomMap[__COUNTER__];

/**
  This service provides OEM to report BIOS ROM map to IHISI interface

  @param  BiosRomMap            pointer to the FBTS_INTERNAL_BIOS_ROM_MAP array with the
                                last entry ends with FbtsRomMapEos region type
  @param  NumberOfRegions       The number of regions of the BiosRomMap array

  @retval EFI_UNSUPPORTED       Returns unsupported by default.
  @retval EFI_SUCCESS           The service is customized in the project.
  @retval EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed.
  @retval Others                Depends on customization.
**/
EFI_STATUS
OemSvcIhisiGetWholeBiosRomMap (
  OUT VOID                          **BiosRomMap,
  OUT UINTN                         *NumberOfRegions
  )
{
  return EFI_UNSUPPORTED;
}

