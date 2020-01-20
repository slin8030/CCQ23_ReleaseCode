/** @file
  Library for Getting Information of Flash Device Region Layout

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/BaseCryptLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/FlashRegionLib.h>
#include <Ppi/Pcd.h>
#include <Ppi/PcdInfo.h>
//
// FDM signature, for not having signature in machine code of this library, assign value one by one byte
//
STATIC CHAR8   mSignature[] = {'H', 'F', 'D', 'M'};


/**
  Get Base Address

  @return    Base Address

**/
UINT64
EFIAPI
FdmGetBaseAddr (
  VOID
 ){

  H2O_FLASH_DEVICE_MAP_HEADER   *FdmHeader;

  FdmHeader =(H2O_FLASH_DEVICE_MAP_HEADER*)(UINTN) PcdGet64(PcdH2OFlashDeviceMapStart);
  if (FdmHeader == NULL){
    return 0;
  }

  if ( CompareMem(FdmHeader, mSignature, sizeof(mSignature))){
    return 0;
  }

  return FdmHeader->FdBaseAddr;

 }


STATIC
BOOLEAN
EFIAPI
IsFdmHeaderValid (
  IN H2O_FLASH_DEVICE_MAP_HEADER    *FdmHeader
 ){

  UINT32 Index;
  UINT8  Sum;
  UINT8  *Data;
  
  Sum = 0;
  Data = (UINT8*) FdmHeader;

  //
  // Verify check sum
  //
  for (Index = 0; Index < sizeof (H2O_FLASH_DEVICE_MAP_HEADER); Index++, Data++){
    Sum += (*Data);
  }

  return (Sum == 0);

}


 /**
 SHA256 HASH calculation

 @param [in]   Message          The message data to be calculated
 @param [in]   MessageSize      The size in byte of the message data
 @param [out]  Digest           The caclulated HASH digest

 @return EFI_SUCCESS            The HASH value is calculated
 @return EFI_SECURITY_VIOLATION  Failed to calculate the HASH

**/
STATIC
EFI_STATUS
EFIAPI
CalculateSha256Hash (
  IN UINT8                      *Message,
  IN UINTN                      MessageSize,
  OUT UINT8                     *Digest
  )
{
  VOID       *HashCtx;
  UINTN      CtxSize;
  EFI_STATUS Status;

  SetMem (Digest, SHA256_DIGEST_SIZE, 0);
  CtxSize = Sha256GetContextSize ();
  HashCtx = NULL;
  HashCtx = AllocatePool (CtxSize);
  if (HashCtx == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (!Sha256Init (HashCtx)) {
    Status = EFI_SECURITY_VIOLATION;
    goto Done;
  }
  if(!Sha256Update (HashCtx, Message, MessageSize)) {
    Status = EFI_SECURITY_VIOLATION;
    goto Done;
  }
  if(!Sha256Final (HashCtx, Digest)) {
    Status = EFI_SECURITY_VIOLATION;
  } else {
    Status = EFI_SUCCESS;
  }

Done:
  FreePool (HashCtx);
  return Status;
}

STATIC
EFI_STATUS
EFIAPI
VerifyHash (
 IN UINT8                         HashMethod,
 IN UINT8                         *Hash,
 IN UINT8                         *Message,
 IN UINTN                         MessageSize
 ){

  EFI_STATUS    Status;
  UINT8         *Digest;
  UINTN                           DigestSize;


  if (Hash == NULL){
    return EFI_INVALID_PARAMETER;
  }

  if (Message == NULL){
    return EFI_INVALID_PARAMETER;
  }

  switch (HashMethod){

  case ENTRY_HASH_SHA256:

    Digest = AllocatePool (SHA256_DIGEST_SIZE);
    if (Digest == NULL){
      return EFI_OUT_OF_RESOURCES;
    }

    Status = CalculateSha256Hash (Message, MessageSize, Digest);
    if (EFI_ERROR(Status)){
      return Status;
    }

    DigestSize =  SHA256_DIGEST_SIZE;
    break;

  default:
    return EFI_UNSUPPORTED;
    break;

  }

  if (CompareMem (Digest, Hash, DigestSize)){
    Status = EFI_SECURITY_VIOLATION;
    goto done;
  } else {
    Status = EFI_SUCCESS;
    goto done;
  }
  
done:

  if (Digest != NULL){
    FreePool (Digest);
  }

  return Status;
 }

/**
  Get count of entries that FDM recorded

  @param  Count                 Unsigned integer that specifies the count of entries that FDM recorded.
  @return EFI_SUCCESS           find region type success.
  @return EFI_NOT_FOUND         Can't find region type in the FDM.

**/
EFI_STATUS
EFIAPI
FdmGetCount (
  OUT  UINT8  *Count
)
{

  H2O_FLASH_DEVICE_MAP_HEADER   *FdmHeader;
  UINT32                        FirstEntryOffset;
  UINT32                        EntrySize;
  UINT32                        Size;

  FdmHeader =(H2O_FLASH_DEVICE_MAP_HEADER*)(UINTN) PcdGet64(PcdH2OFlashDeviceMapStart);

  if ((FdmHeader == NULL) || CompareMem(FdmHeader, mSignature, sizeof(mSignature))){
    return EFI_NOT_FOUND;
  }

  FirstEntryOffset = FdmHeader->Offset;
  Size        = FdmHeader->Size;
  EntrySize   = FdmHeader->EntrySize;

  if (!IsFdmHeaderValid ((H2O_FLASH_DEVICE_MAP_HEADER *) FdmHeader)) {
    return EFI_SECURITY_VIOLATION;
  }

  *Count = ((Size - FirstEntryOffset) / EntrySize) & 0xFF;

  return EFI_SUCCESS;
}

/**
  Get Region type record in FDM by specifying order in the list.

  @param  RegionType            EFI_GUID that specifies the type of region that search for.
  @param  Instance              Unsigned integer that specifies the N-th Region type instance
  @param  RegionId              Identifier that specifies the identifier of this region..
  @param  RegionOffset          Unsigned integer that specifies the offset of this region relative to the
                                base of the flash device image.
  @param  RegionSize            Unsigned integer that specifies the region size.
  @param  Attribs               Bitmap that specifies the attributes of the flash device map entry.


  @return EFI_SUCCESS           find region type success.
  @return EFI_INVALID_PARAMETER Incorrect parameter.
  @return EFI_SECURITY_VIOLATION  Region hash is not correct.
  @return EFI_NOT_FOUND         Can't find region type in the FDM.

**/
EFI_STATUS
EFIAPI
FdmGetNAt (
  CONST IN  EFI_GUID  *RegionType,
  CONST IN  UINT8     Instance,
  OUT UINT8           *RegionId,
  OUT UINT64          *RegionOffset,
  OUT UINT64          *RegionSize,
  OUT UINT32          *Attribs
  ){

  UINT8                         EntryFormat;
  UINT8                         *EndPtr;
  UINT8                         Matched;
  UINT8                         *Hash;
  UINT32                        FirstEntryOffset;
  UINT32                        EntrySize;
  UINT32                        Size;
  EFI_STATUS                    Status;
  H2O_FLASH_DEVICE_MAP_ENTRY    *Entry;
  UINT64                        BaseAddr;
  UINT8                         *Fdm;

  Fdm = (UINT8*)(UINTN) PcdGet64(PcdH2OFlashDeviceMapStart);

  if ((Fdm == NULL) || CompareMem(Fdm, mSignature, sizeof(mSignature))){
    return EFI_NOT_FOUND;
  }

  if (!IsFdmHeaderValid ((H2O_FLASH_DEVICE_MAP_HEADER *) Fdm)) {
    return EFI_SECURITY_VIOLATION;
  }

  //
  // Instance number start from 1
  //
  if (Instance == 0){
    return EFI_INVALID_PARAMETER;
  }

  FirstEntryOffset = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->Offset;
  Size         = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->Size;
  EntrySize    = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->EntrySize;
  EntryFormat  = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->EntryFormat;
  Entry        = (H2O_FLASH_DEVICE_MAP_ENTRY*) (Fdm + FirstEntryOffset);

  EndPtr = Fdm + Size;
  Matched = 0;

  for (; (UINT8*)Entry < EndPtr;  Entry = (H2O_FLASH_DEVICE_MAP_ENTRY*)(((UINT8*)Entry) + EntrySize)){
    if (!CompareMem(&Entry->RegionType, RegionType, sizeof(EFI_GUID))){
      Matched++;

      if (Matched == Instance){
        break;
      }
    }
  }

  if (Matched != Instance){
    return EFI_NOT_FOUND;
  }

  Hash = (UINT8*)(Entry + 1);
  BaseAddr = ((H2O_FLASH_DEVICE_MAP_HEADER *)Fdm)->FdBaseAddr;
  Status = EFI_SUCCESS;
  //
  // Check hash when H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE is not set
  //
  if ((Entry->Attribs & H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE) != H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE){
    Status = VerifyHash (
               EntryFormat,
               Hash,
               (UINT8*)(UINTN) (BaseAddr + Entry->RegionOffset),
               (UINTN)Entry->RegionSize
               );
  }

  if (EFI_ERROR(Status)){
    return EFI_SECURITY_VIOLATION;
  }

  CopyMem (RegionId, Entry->RegionId, FDM_ENTRY_REGION_ID_SIZE);
  *RegionOffset = Entry->RegionOffset;
  *RegionSize   = Entry->RegionSize;
  *Attribs      = Entry->Attribs;

  return EFI_SUCCESS;

}


/**
  Get entry recorded in FDM

  @param  RegionType            EFI_GUID that specifies the type of region that N-th entry is.
  @param  Instance              Unsigned integer that specifies entry instance of FDM
  @param  RegionId              Identifier that specifies the identifier of this region..
  @param  RegionOffset          Unsigned integer that specifies the offset of this region relative to the
                                base of the flash device image.
  @param  RegionSize            Unsigned integer that specifies the region size.
  @param  Attribs               Bitmap that specifies the attributes of the flash device map entry.


  @return EFI_SUCCESS           find region type success.
  @return EFI_INVALID_PARAMETER Incorrect parameter.
  @return EFI_SECURITY_VIOLATION  Region hash is not correct.
  @return EFI_NOT_FOUND         Can't find region type in the FDM.

**/
EFI_STATUS
EFIAPI
FdmGetAt (
  CONST IN  UINT8     Instance,
  OUT EFI_GUID       *RegionType,
  OUT UINT8          *RegionId,
  OUT UINT64         *RegionOffset,
  OUT UINT64         *RegionSize,
  OUT UINT32         *Attribs
  ){

  UINT8        EntryFormat;
  UINT8        *EndPtr;
  UINT8        Matched;
  UINT8        *Hash;
  UINT32       FirstEntryOffset;
  UINT32       EntrySize;
  UINT32       Size;
  EFI_STATUS   Status;
  H2O_FLASH_DEVICE_MAP_ENTRY   *Entry;
  UINTN        DigestSize;
  UINT64       BaseAddr;
  UINT8        *Fdm;


  Fdm = (UINT8*)(UINTN) PcdGet64(PcdH2OFlashDeviceMapStart);

  if ((Fdm == NULL) || CompareMem(Fdm, mSignature, sizeof(mSignature))){
    return EFI_NOT_FOUND;
  }

  if (!IsFdmHeaderValid ((H2O_FLASH_DEVICE_MAP_HEADER *) Fdm)) {
    return EFI_SECURITY_VIOLATION;
  }

  //
  // Instance number start from 1
  //
  if (Instance == 0){
    return EFI_INVALID_PARAMETER;
  }

  FirstEntryOffset = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->Offset;
  Size        = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->Size;
  EntrySize   = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->EntrySize;
  EntryFormat = ((H2O_FLASH_DEVICE_MAP_HEADER*)Fdm)->EntryFormat;
  Entry = (H2O_FLASH_DEVICE_MAP_ENTRY*) (Fdm + FirstEntryOffset);

  EndPtr = Fdm + Size;
  Matched = 0;
  DigestSize = 0;

  Entry = (H2O_FLASH_DEVICE_MAP_ENTRY*)(((UINT8*)Entry) + (Instance - 1) * EntrySize);

  if (Entry > (H2O_FLASH_DEVICE_MAP_ENTRY*)EndPtr){
    return EFI_NOT_FOUND;
  }

  Hash = (UINT8*)(Entry + 1);
  BaseAddr = ((H2O_FLASH_DEVICE_MAP_HEADER *)Fdm)->FdBaseAddr;
  Status = EFI_SUCCESS;

  //
  // Check hash when H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE is not set
  //  
  if ((Entry->Attribs & H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE) != H2O_FLASH_DEVICE_MAP_ENTRY_ATTRIB_MUTABLE){
    Status = VerifyHash (
               EntryFormat,
               Hash,
               (UINT8*)(UINTN) (BaseAddr + Entry->RegionOffset),
               (UINTN)Entry->RegionSize
               );
  }

  if (EFI_ERROR(Status)){
    return EFI_SECURITY_VIOLATION;
  }

  CopyMem (RegionType, &Entry->RegionType, sizeof(Entry->RegionType));
  CopyMem (RegionId, Entry->RegionId, FDM_ENTRY_REGION_ID_SIZE);
  *RegionOffset = Entry->RegionOffset;
  *RegionSize   = Entry->RegionSize;
  *Attribs      = Entry->Attribs;

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
FdmGetNAtWorker (
  IN  CONST EFI_GUID  *RegionType,
  IN  UINT8           Instance,
  OUT UINT64          *Address,
  OUT UINT64          *Size
){

  UINT8   Id[FDM_ENTRY_REGION_ID_SIZE]; 
  UINT64  Offset;
  UINT64  RegionSize;
  UINT32  Attr;

  if (EFI_ERROR (FdmGetNAt (RegionType, Instance, Id, &Offset, &RegionSize, &Attr))){
    return EFI_NOT_FOUND;
  }

  *Address = Offset + FdmGetBaseAddr ();
  *Size = RegionSize;

  return EFI_SUCCESS;

}

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
  ){

  UINT64 Address;
  UINT64 Size;

  if ( EFI_ERROR (FdmGetNAtWorker (RegionType, Instance, &Address, &Size))){
    return 0;
  } 
  return Address;
}


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
){
  
  UINT64 Address;
  UINT64 Size;

  if ( EFI_ERROR (FdmGetNAtWorker (RegionType, Instance, &Address, &Size))){
    return 0;
  } 
  return Size;
}


EFI_STATUS
EFIAPI
FdmGetByIdWorker (
  IN CONST EFI_GUID  *RegionType,
  IN CONST UINT8     *Id,
  IN  UINT8          Instance,
  OUT UINT64         *Address,
  OUT UINT64         *Size  
){
  UINT8          MaxRegion;
  UINT8          Index;
  UINT8          Matched; 
  EFI_STATUS     Status;  
  UINT8          RegionId[FDM_ENTRY_REGION_ID_SIZE];
  UINT64         RegionOffset;
  UINT64         RegionSize;
  UINT32         Attribs;


  Status = FdmGetCount (&MaxRegion);

  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Matched = 0;

  for (Index = 1; Index < MaxRegion + 1; Index++){
    Status = FdmGetNAt (
               RegionType, 
               Index, 
               RegionId, 
               &RegionOffset, 
               &RegionSize, 
               &Attribs
               );
    if (EFI_ERROR (Status)) {
      return EFI_NOT_FOUND;
    }

    if (!CompareMem(RegionId, Id, sizeof(EFI_GUID))){
      Matched++;
    }

    if (Matched == Instance) {
      *Address = RegionOffset + FdmGetBaseAddr ();
      *Size = RegionSize;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}


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
  ){
    
  EFI_STATUS     Status;  
  UINT64         RegionAddress;
  UINT64         RegionSize;

  Status = FdmGetByIdWorker (
             RegionType,
             Id,
             Instance,
             &RegionAddress,
             &RegionSize
             );

  if (EFI_ERROR(Status)){
    return 0;
  }

  return RegionAddress;
  
}


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
  ){

  EFI_STATUS     Status;  
  UINT64         RegionAddress;
  UINT64         RegionSize;

  Status = FdmGetByIdWorker (
             RegionType,
             Id,
             Instance,
             &RegionAddress,
             &RegionSize
             );
             
  if (EFI_ERROR(Status)){
    return 0;
  }

  return RegionSize;

}


STATIC
EFI_STATUS
EFIAPI
FdmGetVariableWorker (
  IN  UINT16 DefaultId,
  IN  UINT8     Instance,
  OUT UINT64 *Address,
  OUT UINT64 *Size    
){
  UINT8                   RegionCount;
  UINT8                   Index;    
  UINT8                   RegionId[FDM_ENTRY_REGION_ID_SIZE];
  UINT8                   Matched;
  BOOLEAN                 DefaultIdSearched;
  UINT16                  SkuId;
  UINT64                  RegionOffset;
  UINT64                  RegionSize;
  UINT32                  Attribs;
  EFI_STATUS              Status;    
  FDM_VARIABLE_REGION_ID  *Id;
  GET_PCD_INFO_PPI        *PcdInfoPpi;

  DefaultIdSearched = FALSE; 

  Status = FdmGetCount (&RegionCount);

  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  //
  // Get SkuId
  //
  Status = PeiServicesLocatePpi (
             &gGetPcdInfoPpiGuid,
             0,
             NULL,
             (VOID**)&PcdInfoPpi
             );

  if (!EFI_ERROR (Status)) {
    SkuId = (UINT16) PcdInfoPpi->GetSku ();
  } else {
    SkuId = DEFAULT_SKU_ID;
  }

  //
  // first a search is made of the variable store associated with the SKU ID. If the value is not found , 
  // then the variable store associated with SKU ID 0 (Default) is used.
  // 
  do {
    Matched = 0;    
    for (Index = 1; Index < RegionCount + 1; Index++){

      Status = FdmGetNAt (
                 &gH2OFlashMapRegionVarGuid,
                 Index,
                 RegionId, 
                 &RegionOffset,
                 &RegionSize,
                 &Attribs
                 );

      if (EFI_ERROR (Status)) {
        if (SkuId == DEFAULT_SKU_ID){
          DefaultIdSearched = TRUE; 
          break;
        } else {
          SkuId = DEFAULT_SKU_ID;
          break;
        }
      }

      Id = (FDM_VARIABLE_REGION_ID*) RegionId;

      if ((Id->Status < VARIABLE_REGION_INACTIVE)  && (Id->SkuId == SkuId) && (Id->DefaultId == DefaultId)){
        Matched++;
      }

      if (Matched == Instance){
        *Address = RegionOffset + FdmGetBaseAddr();
        *Size = RegionSize;
        return EFI_SUCCESS;
      }
    }

  } while (!DefaultIdSearched);

  return EFI_NOT_FOUND;

}



/**
  Get address of variable region recorded in FDM by specific DefaultId.

  @param[in]   DefaultId             Default Id that variable region must matched.
  @param[in]   Instance              Unsigned integer that specifies instance of Region Type in FDM

  @return      address of the Region.
  
**/
UINT64
EFIAPI
FdmGetVariableAddr (
  IN  UINT16        DefaultId,
  IN  UINT8         Instance
){

  EFI_STATUS              Status;
  UINT64                  RegionAddress;
  UINT64                  RegionSize;

  Status = FdmGetVariableWorker (
             DefaultId,
             Instance,
             &RegionAddress,
             &RegionSize
             );

  if (EFI_ERROR (Status)){
    return 0;
  }

  return RegionAddress;

}


/**
  Get size of variable region recorded in FDM by specific DefaultId.

  @param[in]   DefaultId           Default Id that variable region must matched.
  @param[in]   Instance            Unsigned integer that specifies instance of Region Type in FDM

  @return      size of the Region.

**/
UINT64
EFIAPI
FdmGetVariableSize (
  IN  UINT16        DefaultId,
  IN UINT8         Instance
){
    
  EFI_STATUS              Status;
  UINT64                  RegionAddress;
  UINT64                  RegionSize;

  Status = FdmGetVariableWorker (
             DefaultId,
             Instance,
             &RegionAddress,
             &RegionSize
             );

  if (EFI_ERROR (Status)){
    return 0;
  }

  return RegionSize;

}


/**
  Get Flash Size

  @return    Flash Size

**/ 
UINT64
EFIAPI
FdmGetFlashAreaSize (
  VOID
){

  UINT64  FlashSize;
  UINT8   Index;
  UINT8   MaxRegion;
  EFI_STATUS              Status;
  UINT8                   RegionId[FDM_ENTRY_REGION_ID_SIZE];
  UINT64                  RegionOffset;
  UINT64                  RegionSize;
  UINT32                  Attribs;
  EFI_GUID                RegionType;

  FlashSize = 0;

  Status = FdmGetCount (&MaxRegion);

  if (EFI_ERROR (Status)) {
    return 0;
  }

  //
  // Cumulate all regions size to get flash size
  //
  for (Index = 1; Index < MaxRegion + 1; Index++){
    Status = FdmGetAt (
               Index,
               &RegionType,
               RegionId,
               &RegionOffset,
               &RegionSize, 
               &Attribs
               );
    if (EFI_ERROR (Status)) {
      return 0;
    }
    FlashSize += RegionSize;
  }
  return FlashSize;  
}



