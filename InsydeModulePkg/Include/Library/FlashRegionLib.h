/** @file
  Library for enumerate Flash Device Region Layout

;******************************************************************************
;* Copyright (c) 2014 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _FLASH_REGION_LIB_H_
#define _FLASH_REGION_LIB_H_

#include <FlashRegionLayout.h>

#define PCD_FLASH_MAP_START_EX_TOKEN_NUMBER          0x30000017

#define SHA256_DIGEST_SIZE          32
#define FDM_ENTRY_REGION_ID_SIZE    16

#define ENTRY_HASH_SHA256               0

#define H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE       0x1

#define FDM_VARIABLE_DEFAULT_ID_STANDARD             0x0000
#define FDM_VARIABLE_DEFAULT_ID_MANUFACTURE          0x0001
#define FDM_VARIABLE_DEFAULT_ID_SAFE                 0x0002
#define FDM_VARIABLE_DEFAULT_ID_FACTORY_COPY         0x8000
#define FDM_VARIABLE_DEFAULT_ID_WORKING              0xFFFF

#define DEFAULT_SKU_ID                               0
#define VARIABLE_REGION_INACTIVE                     0x8000


#pragma pack(push, 1)
typedef struct _H2O_FLASH_DEVICE_MAP_HEADER {
  UINT32 Signature;
  UINT32 Size;
  UINT32 Offset;
  UINT32 EntrySize;
  UINT8  EntryFormat;
  UINT8  Revision;
  UINT8  Reserved;
  UINT8  Checksum;
  UINT64 FdBaseAddr;
} H2O_FLASH_DEVICE_MAP_HEADER;

typedef struct _H2O_FLASH_DEVICE_MAP_ENTRY {
  EFI_GUID RegionType;
  UINT8    RegionId[16];
  UINT64   RegionOffset;
  UINT64   RegionSize;
  UINT32   Attribs;
//UINT8    Hash[¡K];
} H2O_FLASH_DEVICE_MAP_ENTRY;

typedef struct _FDM_REGION_GUID_TABLE {
  UINT8      Index;
  EFI_GUID   Guid;
}FDM_REGION_GUID_TABLE;


typedef struct _FDM_VARIABLE_REGION_ID {
  UINT16      SkuId;
  UINT16      DefaultId;
  UINT16      Status;
}FDM_VARIABLE_REGION_ID;
#pragma pack(pop)

/**
  Get count of entries that FDM recorded 

  @param[out] Count             Unsigned integer that specifies the count of entries that FDM recorded.
  
  @return EFI_SUCCESS           Get region count success.
  @return EFI_NOT_FOUND         Can't find region type in the FDM.
  @return EFI_UNSUPPORTED       Function called in runtime phase

**/
EFI_STATUS
EFIAPI
FdmGetCount (
  OUT  UINT8  *Count
);


/**
  Get Region type record in FDM by specifying order in the list.

  @param[in]   RegionType            GUID that specifies the type of region that search for.
  @param[in]   Instance              Unsigned integer that specifies the N-th Region type instance
  @param[out]  RegionId              Identifier that specifies the identifier of this region..
  @param[out]  RegionOffset          Unsigned integer that specifies the offset of this region relative to the 
                                     base of the flash device image.
  @param[out]  RegionSize            Unsigned integer that specifies the region size.
  @param[out]  Attribs               Bitmap that specifies the attributes of the flash device map entry. 

                                
  @return EFI_SUCCESS                find region type success.
  @return EFI_INVALID_PARAMETER      Incorrect parameter.
  @return EFI_SECURITY_VIOLATION     Region hash is not correct.
  @return EFI_NOT_FOUND              Can't find region type in the FDM.
  @return EFI_NOT_READY              EFI_HASH_PROTOCOL not installed yet
  @return EFI_UNSUPPORTED            Function called in runtime phase

**/
EFI_STATUS
EFIAPI
FdmGetNAt (
  IN CONST EFI_GUID  *RegionType,  
  IN CONST UINT8     Instance,
  OUT UINT8          *RegionId,
  OUT UINT64         *RegionOffset,
  OUT UINT64         *RegionSize,
  OUT UINT32         *Attribs
  );
  
  
/**
  Get entry recorded in FDM

  @param[in]   RegionType            GUID that specifies the type of region that N-th entry is.
  @param[out]  Instance              Unsigned integer that specifies entry instance of FDM
  @param[out]  RegionId              Identifier that specifies the identifier of this region..
  @param[out]  RegionOffset          Unsigned integer that specifies the offset of this region relative to the 
                                     base of the flash device image.
  @param[out]  RegionSize            Unsigned integer that specifies the region size.
  @param[out]  Attribs               Bitmap that specifies the attributes of the flash device map entry. 

  
  @return EFI_SUCCESS                find region type success.
  @return EFI_INVALID_PARAMETER      Incorrect parameter.
  @return EFI_SECURITY_VIOLATION     Region hash is not correct.
  @return EFI_NOT_FOUND              Can't find region type in the FDM.
  @return EFI_NOT_READY              EFI_HASH_PROTOCOL not installed yet
  @return EFI_UNSUPPORTED            Function called in runtime phase

**/
EFI_STATUS
EFIAPI
FdmGetAt (
  IN CONST UINT8   Instance,
  OUT EFI_GUID     *RegionType,
  OUT UINT8        *RegionId,
  OUT UINT64       *RegionOffset,
  OUT UINT64       *RegionSize,
  OUT UINT32       *Attribs
  );
  


/**
  Get Address of Region recorded in FDM

  @param[in]   RegionType            GUID that specifies the type of region
  @param[in]   Instance              Unsigned integer that specifies entry instance of Region Type in FDM

  @return      address of the Region.
  
**/  
UINT64
EFIAPI
FdmGetNAtAddr (
  IN CONST EFI_GUID  *RegionType,
  IN UINT8           Instance
);


/**
  Get Size of Region recorded in FDM

  @param[in]   RegionType            GUID that specifies the type of region
  @param[in]   Instance              Unsigned integer that specifies entry instance of Region Type in FDM

  @return      size of the Region.

**/
UINT64
EFIAPI
FdmGetNAtSize (
  IN CONST EFI_GUID  *RegionType,
  IN UINT8           Instance
);


/**
  Get address of Region recorded in FDM by specific Id and region type.

  @param[in]   RegionType            GUID that specifies the type of region.
  @param[in]   Id                    Pointer of value that specifies the Id field must matched.
  @param[in]   Instance              Unsigned integer that specifies instance of Region Type in FDM  

  @return      address of the Region.

**/
UINT64
EFIAPI
FdmGetAddressById (
  IN CONST EFI_GUID  *RegionType,
  IN CONST UINT8     *Id,
  IN UINT8           Instance
);


/**
  Get size of Region recorded in FDM by specific Id and region type.

  @param[in]   RegionType            GUID that specifies the type of region.
  @param[in]   Id                    Pointer of value that specifies the Id field must matched.
  @param[in]   Instance              Unsigned integer that specifies instance of Region Type in FDM

  @return      size of the Region.

**/
UINT64
EFIAPI
FdmGetSizeById (
  IN CONST EFI_GUID  *RegionType,
  IN CONST UINT8     *Id,
  IN UINT8           Instance
);


/**
  Get address of variable region recorded in FDM by specific DefaultId.

  @param[in]   DefaultId             Default Id that variable region must matched.
  @param[in]   Instance              Unsigned integer that specifies instance of Region Type in FDM

  @return      address of the Region.
  
**/
UINT64
EFIAPI
FdmGetVariableAddr (
  IN  UINT16   DefaultId,
  IN  UINT8    Instance
);


/**
  Get size of variable region recorded in FDM by specific DefaultId.

  @param[in]   DefaultId           Default Id that variable region must matched.
  @param[in]   Instance            Unsigned integer that specifies instance of Region Type in FDM

  @return      size of the Region.

**/
UINT64
EFIAPI
FdmGetVariableSize (
  IN  UINT16    DefaultId,
  IN  UINT8     Instance
);


/**
  Get Base Address

  @return    Base Address
  
**/
UINT64
EFIAPI
FdmGetBaseAddr (
  VOID
 );
 
 
/**
  Get Flash Size

  @return    Flash Size

**/ 
UINT64
EFIAPI
FdmGetFlashAreaSize (
  VOID
);
 
#endif
