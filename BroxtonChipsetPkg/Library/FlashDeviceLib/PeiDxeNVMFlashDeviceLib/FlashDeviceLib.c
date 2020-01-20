/*++
  This file contains 'Framework Code' and is licensed as such   
  under the terms of your license agreement with Intel or your  
  vendor.  This file may not be modified, except as allowed by  
  additional terms of your license agreement.                   
--*/
/** @file
  Libray implementation for Flash Device Library based on Multiple Flash Support
  library intance.

Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/


#include <PiDxe.h>

#include <Library/FlashDeviceLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Protocol/SmmBase2.h>
#include <Guid/EventGroup.h>
#include <Guid/SystemNvDataGuid.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PreSiliconLib.h>
#include <protocol/Heci.h>
#include <HeciRegs.h>
#include <SeCAccess.h>
#include <Library/PciLib.h>
#include <Library/TimerLib.h>
#include <Library/HeciMsgLib.h>
//[-start-160618-IB07400744-add]//
#include <Library/CmosLib.h>
#include <ChipsetPostCode.h>
#include <ChipsetCmos.h>
//[-end-160618-IB07400744-add]//
//[-start-160807-IB07400769-add]//
#include <Private/Library/HeciInitLib.h>
//[-end-160807-IB07400769-add]//

#define NVStorageFile0 "NVS/FILE_0"
#define NVStorageFile1 "NVS/FILE_1"
#define NVStorageFile2 "NVS/FILE_2"
#define NVStorageFile3 "NVS/FILE_3"
#define NVStorageFile4 "NVS/FILE_4"
#define NVStorageFile5 "NVS/FILE_5"
#define NVStorageFile6 "NVS/FILE_6"
#define NVStorageFile7 "NVS/FILE_7"
#define NVStorageFile8 "NVS/FILE_8"



UINTN AddressOffset[] = {0, 0x10000, 0x20000, 0x30000, 0x40000, 0x50000, 0x60000, 0x70000, 0x80000};
CHAR8* FileName[] = {"", NVStorageFile0,NVStorageFile1,NVStorageFile2,NVStorageFile3,NVStorageFile4, NVStorageFile5, NVStorageFile6, NVStorageFile7};

//[-start-160509-IB03090427-add]//
/**
  Calling this function causes a system-wide initialization. The processors
  are set to their initial state, and pending cycles are not corrupted.

  System reset should not return, if it returns, it means the system does
  not support warm reset.
**/
VOID
EFIAPI
ResetWarm (
  VOID
  )
{
  IoWrite8 (0xCF9, 0x0);
  IoWrite8 (0xCF9, 0x6);
}
//[-end-160509-IB03090427-add]//

BOOLEAN IsNVMReady() {
  EFI_STATUS Status;
  UINTN      FileSize;
	
	Status = HeciGetNVMFileSize(NVStorageFile0, &FileSize);
	if (EFI_ERROR(Status)) {
	  return FALSE;
	}
	return TRUE;
}



//[-start-160216-IB03090424-modify]//
VOID * GetNvStorageBase(VOID * Address)
{

  EFI_STATUS Status;
  UINT8      *Buffer;
  UINTN      FileSize0;
  UINTN      FileSize1;
  UINTN      Index;
//[-start-160619-IB07400744-add]//
  UINTN      ReTry;
//[-end-160619-IB07400744-add]//

//[-start-160619-IB07400744-modify]//
  Buffer = AllocateAlignedPages(EFI_SIZE_TO_PAGES ((UINT32)(0x80000)), 1<<12);
  if (Buffer == NULL) {
    DEBUG((EFI_D_ERROR, "GetNvStorageBase Can't Allocate the Buffer\n"));
    return NULL;
  }
  SetMem(Buffer, 0x80000, 0xFF);
//[-end-160619-IB07400744-modify]//

  Status = HeciGetNVMFileSize(NVStorageFile0, &FileSize0);
//[-start-160619-IB07400744-add]//
  ReTry = 50;  // 5 ms wait device ready
  while ((Status == EFI_DEVICE_ERROR) && (ReTry)) {
    CHIPSET_POST_CODE (PEI_HECI_DEVICE_ERROR_RETRY);
  	DEBUG((EFI_D_ERROR, "Heci Device Error Retry %x!!\n", ReTry));
    Status = HeciGetNVMFileSize(NVStorageFile0, &FileSize0);
    CHIPSET_POST_CODE ((UINT32)ReTry);
    CHIPSET_POST_CODE ((UINT32)Status);
    ReTry--;
    MicroSecondDelay (100);
  }
//[-end-160619-IB07400744-add]//
  if (EFI_ERROR(Status)) {
  	DEBUG((EFI_D_ERROR, "GetNvStorageBase Can't Read the file\n"));
	  CopyMem(Buffer, Address, 0x80000);	
  	return Buffer;
  }
//[-start-160619-IB07400744-add]//
//[-start-160905-IB07400778-modify]//
  else if ((ReadExtCmos8(R_XCMOS_INDEX, R_XCMOS_DATA, CmosRecoveryOnFlagAddress)) == V_CMOS_FAST_RECOVERY_RESTORE_NV) {

    //
    // Restore NVMs data after recovery
    //
    CHIPSET_POST_CODE (PEI_RECOVERY_RESTORE_NV_DATA);
  	DEBUG((EFI_D_ERROR, "Restore NVMs data after recovery\n"));
    CopyMem(Buffer, Address, 0x80000);
    for (Index = 1; Index < sizeof(FileName)/sizeof(FileName[0]); Index ++) {
//[-start-160816-IB07400771-modify]//
      Status = HeciWriteNVMFile(FileName[Index], 0, (UINT8*)Address + AddressOffset[Index - 1], AddressOffset[Index] - AddressOffset[Index - 1], FALSE);
//[-end-160816-IB07400771-modify]//
      CHIPSET_POST_CODE ((UINT32)Status);
      DEBUG((EFI_D_INFO, "Init %a %x\n", FileName[Index], Status));
    }
    WriteExtCmos8(R_XCMOS_INDEX, R_XCMOS_DATA, CmosRecoveryOnFlagAddress, V_CMOS_FAST_RECOVERY_DISABLED);
  	return Buffer;
  }
//[-end-160905-IB07400778-modify]//
//[-end-160619-IB07400744-add]//

  for (Index = 1; Index < sizeof(FileName)/sizeof(FileName[0]); Index ++) {
    FileSize0 = AddressOffset[Index] - AddressOffset[Index - 1];
	if (FileSize0 > 0x8000) {
	  FileSize1 = FileSize0 - 0x8000;
	  FileSize0 = 0x8000;
      HeciReadNVMFile(FileName[Index], 0, Buffer + AddressOffset[Index - 1], &FileSize0);
      DEBUG((EFI_D_INFO, "Read %a filesize %x\n", FileName[Index], FileSize0));
      HeciReadNVMFile(FileName[Index], 0x8000, Buffer + AddressOffset[Index - 1] + 0x8000, &FileSize1);
      DEBUG((EFI_D_INFO, "Read %a filesize %x\n", FileName[Index], FileSize0));
    } else {
	  HeciReadNVMFile(FileName[Index], 0, Buffer + AddressOffset[Index - 1], &FileSize0);
	  DEBUG((EFI_D_INFO, "Read %a filesize %x\n", FileName[Index], FileSize0));
    }
  }
  return Buffer; 
}
//[-end-160216-IB03090424-modify]//



UINTN GetBlockIndex(
  UINTN Offset
  )
{
  UINTN Index;
  for (Index = 1; Index < sizeof(AddressOffset) / sizeof(AddressOffset[0]); Index ++) {
    if (AddressOffset[Index] > Offset) break;
  }
  if (Index == sizeof(AddressOffset) / sizeof(AddressOffset[0])) {
  	ASSERT(FALSE);
  }
  return Index;
}

VOID UpdateNVM_Internal (UINTN Offset, UINT8*Data, UINTN DataSize, VOID* HeciInterface) {

    UINTN Index;
	Index = GetBlockIndex(Offset);
	if (HeciInterface != NULL) {
//[-start-160819-IB07220131-modify]//
	  Heci2WriteNVMFile(FileName[Index], (UINT32)(Offset - AddressOffset[Index - 1]), Data, DataSize, FALSE);
//[-end-160819-IB07220131-modify]//
	} else {
	  if ((DataSize & 3) != 0) {
	  	DataSize = ((DataSize + 4) & (~3));
  	  }
//[-start-160808-IB07220123-modify]//
      HeciWriteNVMFile(FileName[Index], (UINT32)(Offset - AddressOffset[Index - 1]), Data, DataSize, FALSE);
//[-end-160808-IB07220123-modify]//
	}
}

VOID UpdateNVM(
  UINTN Offset,
  UINT8 *Data, 
  UINTN DataSize,
  VOID* HeciInterface
  ) 
{
  UINTN StartIndex;
  UINTN EndIndex;
  UINTN PartDataSize;
  
//[-start-160806-IB07400769-add]//
  //
  // Skip update NVM if TXE data region Unavailable
  //
  if (!IsTxeDataRegionAvailable()) {
    return;
  }
//[-end-160806-IB07400769-add]//

  StartIndex = GetBlockIndex(Offset);
  EndIndex = GetBlockIndex(Offset + DataSize - 1);
  if (StartIndex == EndIndex) {
    UpdateNVM_Internal(Offset, Data, DataSize, HeciInterface);
  } else {
    PartDataSize = AddressOffset[StartIndex] - Offset;
	UpdateNVM(Offset, Data, PartDataSize, HeciInterface);
	UpdateNVM(Offset + PartDataSize, Data + PartDataSize, DataSize - PartDataSize, HeciInterface);
  }
}

VOID
InitNVM (
  VOID     *Address,
  UINTN    Size
  )
{
//[-start-160509-IB03090427-modify]//
  EFI_STATUS              Status=0xff;
//[-end-160509-IB03090427-modify]//
  UINTN                   Index;

  

  DEBUG((EFI_D_INFO, "******************************InitNVM Start ******************************\n"));
  if (IsNVMReady()){
    DEBUG((EFI_D_INFO, "NVM File exists, Don't Init\n"));
    return;
  }
    DEBUG((EFI_D_INFO, "NVM File doesn't exist, Init\n"));

    for (Index = 1; Index < sizeof(FileName)/sizeof(FileName[0]); Index ++) {
//[-start-160808-IB07220123-modify]//
      Status = HeciWriteNVMFile(FileName[Index], 0, (UINT8*)Address + AddressOffset[Index - 1], AddressOffset[Index] - AddressOffset[Index - 1], FALSE);
//[-end-160808-IB07220123-modify]//
      DEBUG((EFI_D_INFO, "Init %a %x\n", FileName[Index], Status));
    }

//[-start-160509-IB03090427-add]//
//[-start-160819-IB07220131-remove]//
//   if(PcdGetBool(PcdFDOState) == FALSE && Status ==EFI_SUCCESS){
//   DEBUG((EFI_D_INFO, "NVM File Init Done. Perform a reset to let system boot with NVM file.\n"));
//   
//     ResetWarm ();
//   }
//[-end-160819-IB07220131-remove]//
//[-end-160509-IB03090427-add]//
  DEBUG((EFI_D_INFO, "******************************InitNVM End   ******************************\n"));
}

