/** @file
  Provide OEMs to report BIOS ROM map to IHISI interface

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
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
#include <H2OIhisi.h>
//[-start-160809-IB11270161-add]//
#include <Library/FlashRegionLib.h>
//[-end-160809-IB11270161-add]//
#if 1
//
// Sample source for OemSvcIhisiGetWholeBiosRomMap()
//

extern FBTS_INTERNAL_BIOS_ROM_MAP mBiosRomMap[];
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
//[-start-160809-IB11270161-modify]//
  UINTN  Index;

  if ((BiosRomMap == NULL) || (NumberOfRegions == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  #define ADD_ROM_MAP_ENTRY(RegionType, RegionAddress, RegionSize, RegionAttr) \
            mBiosRomMap[Index].Type = (UINT8)RegionType;                     \
            mBiosRomMap[Index].Address = RegionAddress;                      \
            mBiosRomMap[Index].Size = RegionSize;                            \
            mBiosRomMap[Index].Attribute = RegionAttr;                       \
            Index++;                                                         \
            __COUNTER__
            
  #define ADD_ROM_MAP_ENTRY_FROM_PCD(RegionType, AddressPcd, SizePcd, RegionAttr)                 \
          if (PcdGet32(SizePcd) > 0) {                                                          \
            ADD_ROM_MAP_ENTRY(RegionType, PcdGet32(AddressPcd), PcdGet32(SizePcd), RegionAttr); \
          }
          
  #define ADD_ROM_MAP_ENTRY_FROM_FDM(RegionType, FdmRegionType, Instance, RegionAttr)  \
          {                                                                          \
            UINT64  Size;                                                            \
            if ((Size = FdmGetNAtSize(&FdmRegionType, Instance)) > 0) {              \
              ADD_ROM_MAP_ENTRY(RegionType, (UINT32) FdmGetNAtAddr(&FdmRegionType, Instance), (UINT32) Size, RegionAttr); \
            }  \
          }

  #define ADD_DXE_FV_ROM_ENTRY_FROM_FDM(RegionAttr)  \
            {                                                    \
              UINT64  Size;                                      \
              if ((Size = FdmGetSizeById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionDxeFvGuid, 1)) > 0) {  \
                ADD_ROM_MAP_ENTRY(FbtsRomMapDxe, (UINT32) FdmGetAddressById(&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionDxeFvGuid, 1), (UINT32) Size, RegionAttr); \
              }  \
            }
  
  #define ADD_PEI_FV_ROM_ENTRY_FROM_FDM(RegionAttr)  \
            {                                                    \
              UINT64  Size;                                      \
              if ((Size = FdmGetSizeById (&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionPeiFvGuid, 1)) > 0) {  \
                ADD_ROM_MAP_ENTRY(FbtsRomMapPei, (UINT32) FdmGetAddressById(&gH2OFlashMapRegionFvGuid, (UINT8*) &gH2OFlashMapRegionPeiFvGuid, 1), (UINT32) Size, RegionAttr); \
              }  \
            }


  
  Index = 0;
  ADD_DXE_FV_ROM_ENTRY_FROM_FDM   (0);  
  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapOemData,      gH2OFlashMapRegionMsdmGuid,             1,   0);
  ADD_ROM_MAP_ENTRY_FROM_FDM      (FbtsRomMapOemData,      gH2OFlashMapRegionBvdtGuid,             1,   0);
  ADD_PEI_FV_ROM_ENTRY_FROM_FDM   (0);

  //
  // End of ROM map
  //
  ADD_ROM_MAP_ENTRY (FbtsRomMapEos, 0, 0, 0);

  *BiosRomMap = (VOID *)mBiosRomMap;
  *NumberOfRegions = Index;

  return EFI_MEDIA_CHANGED;
//[-end-160809-IB11270161-modify]//
}


//
// the mBiosRomMap is declared after OemSvcIhisiGetWholeRomMap() on purpose to
// ensure the array size of mBiosRomMap is enough, DO NOT move it to the top
//
FBTS_INTERNAL_BIOS_ROM_MAP mBiosRomMap[__COUNTER__];

#else
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

#endif
