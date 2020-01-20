/** @file
  Library implementation for Flash Device Library based on Multiple Flash Support
  library instance.

@copyright
  Copyright (c) 2006 - 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by the
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains an 'Intel Peripheral Driver' and is uniquely
  identified as "Intel Reference Module" and is licensed for Intel
  CPUs and chipsets under the terms of your license agreement with
  Intel or your vendor. This file may be modified by the user, subject
  to additional terms of the license agreement.

@par Specification
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
#include <Library/SmmServicesTableLib.h>
#include <Library/DxeInsydeChipsetLib.h>
#include <Protocol/SmmEndOfDxe.h>
//[-start-160317-IB03090425-add]//
#include <ChipsetSetupConfig.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-end-160317-IB03090425-add]//
//[-start-160426-IB10860196-add]//
#include <Guid/AuthenticatedVariableFormat.h>
//[-end-160426-IB10860196-add]//
//[-start-160807-IB07400769-add]//
#include <Private/Library/HeciInitLib.h>
//[-end-160807-IB07400769-add]//

//[-start-160317-IB03090425-add]//
#define  PLATFORM_CONFIG_STORE_DIR_NAME   "critical"
//[-end-160317-IB03090425-add]//
extern EFI_GUID gEfiBootMediaHobGuid;

UINTN mBufferAddress = 0;
//[-start-160426-IB10860196-add]//
UINTN mCacheAddress = 0;
//[-end-160426-IB10860196-add]//

EFI_HECI_PROTOCOL  *mHeci2Protocol = NULL;
//[-start-160426-IB10860196-add]//
#define SMM_HECI_FUNCTION_GET_STATUS               8

typedef struct {
  UINTN       Function;
  EFI_STATUS  ReturnStatus;
  UINT8       Data[1];
} SMM_HECI_COMMUNICATE_HEADER;

#define EFI_HECI_SMM_PROTOCOL_GUID {0xFC53F573, 0x17DD, 0x454C, 0xB0, 0x67, 0xEC, 0xB1, 0x0B, 0x7D, 0x7F, 0xC7}
STATIC EFI_GUID  mEfiHeciSmmProtocolGuid = EFI_HECI_SMM_PROTOCOL_GUID;

UINT8 mHeciBuffer[0x10100];
//[-end-160426-IB10860196-add]//


VOID
UpdateNVM (
  UINTN    Offset,
  UINT8    *Data,
  UINTN    DataSize,
  VOID     *HeciInterface
  );

VOID
FlashDeviceCheck (
  VOID
  );

VOID
InitNVM (
  VOID     *Address,
  UINTN    Size
  );
  
//[-start-160426-IB10860196-add]//
VOID 
HeciSetReadWriteCache(
  VOID*  Address
);

//[-start-161002-IB07400791-remove]//
//VOID
//FlashDeviceCheckSmm (
//  VOID
//);
//
//EFI_STATUS
//EfiHeciGetSecStatus (
//  OUT UINT32                       *Status2
//);
//
//
//EFI_STATUS
//EfiHeciGetSecStatus (
//  OUT UINT32                       *Status2
//  )
//{
//  EFI_STATUS                   Status;
//  UINTN                        PayloadSize;
//  EFI_SMM_COMMUNICATE_HEADER   *SmmCommunicateHeader;
//  SMM_HECI_COMMUNICATE_HEADER  *SmmHeciFunctionHeader; 
//  UINTN                        CommSize;
//
//  PayloadSize = sizeof(SMM_HECI_COMMUNICATE_HEADER);
//  CommSize = PayloadSize + sizeof(EFI_SMM_COMMUNICATE_HEADER) + sizeof(SMM_HECI_COMMUNICATE_HEADER);
//
//  SmmCommunicateHeader = (EFI_SMM_COMMUNICATE_HEADER *)mHeciBuffer;
//  CopyGuid (&SmmCommunicateHeader->HeaderGuid, &mEfiHeciSmmProtocolGuid);
//  SmmCommunicateHeader->MessageLength = PayloadSize + sizeof(EFI_SMM_COMMUNICATE_HEADER);
//
//  SmmHeciFunctionHeader = (SMM_HECI_COMMUNICATE_HEADER *) SmmCommunicateHeader->Data;
//  SmmHeciFunctionHeader->Function = SMM_HECI_FUNCTION_GET_STATUS;
//  
//  CommSize -= OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data);
//  Status = gSmst->SmiManage (
//                    &SmmCommunicateHeader->HeaderGuid,
//                    NULL,
//                    SmmCommunicateHeader->Data,
//                    &CommSize
//                    );
//  CommSize += OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data);
//
//  *Status2 = (UINT32)*(SmmHeciFunctionHeader->Data);
//
//  return Status;
//}
//
//VOID
//FlashDeviceCheckSmm (
//  VOID
//  )
//{
//  UINT32    Data;
////[-start-160714-IB11270157-modify]//
////[-start-160804-IB07400769-modify]//
//  if (IsTxeDataRegionAvailable()) {
////[-end-160804-IB07400769-modify]//
//    if (mHeci2Protocol != NULL){
//      Data = 1;
//  
//      while(1) {
//        EfiHeciGetSecStatus(&Data);
//        if (Data == 0) {
//          break;
//        }
//        MicroSecondDelay(1000);
//      }
//    
//    }
//  }
////[-end-160714-IB11270157-modify]//
//}
//[-end-161002-IB07400791-remove]//
//[-end-160426-IB10860196-add]//

/**
  Read NumBytes bytes of data from the address specified by
  PAddress into Buffer.

  @param[in]      PAddress      The starting physical address of the read.
  @param[in,out]  NumBytes      On input, the number of bytes to read. On output, the number
                                of bytes actually read.
  @param[out]     Buffer        The destination data buffer for the read.

  @retval         EFI_SUCCESS.      Operation is successful.
  @retval         EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceRead (
  IN      UINTN                           PAddress,
  IN  OUT UINTN                           *NumBytes,
      OUT UINT8                           *Buffer
  )
{
  DEBUG((EFI_D_INFO, "LibFvbFlashDeviceRead %x %x %x\n", PAddress, *NumBytes, Buffer ));
  CopyMem(Buffer, (VOID*)(UINTN)PAddress, *NumBytes);
  return EFI_SUCCESS;
}



/**
  Write NumBytes bytes of data from Buffer to the address specified by
  PAddresss.

  @param[in]      PAddress        The starting physical address of the write.
  @param[in,out]  NumBytes        On input, the number of bytes to write. On output,
                                  the actual number of bytes written.
  @param[in]      Buffer          The source data buffer for the write.

  @retval         EFI_SUCCESS.      Operation is successful.
  @retval         EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceWrite (
  IN        UINTN                           PAddress,
  IN OUT    UINTN                           *NumBytes,
  IN        UINT8                           *Buffer
  )
{
//[-start-160426-IB10860196-add]//
//[-start-160819-IB07220131-remove]//
//   FlashDeviceCheckSmm();
//[-end-160819-IB07220131-remove]//
  CopyMem((VOID*)PAddress,Buffer,*NumBytes);

  UpdateNVM(PAddress - mBufferAddress, (VOID*)PAddress, *NumBytes, mHeci2Protocol);
//[-start-160819-IB07220131-remove]//
//   FlashDeviceCheckSmm();
//[-end-160819-IB07220131-remove]//
//[-end-160426-IB10860196-add]//

  return EFI_SUCCESS;
}



/**
  Erase the block staring at PAddress.

  @param[in]  PAddress        The starting physical address of the block to be erased.
                              This library assume that caller guarantee that the PAddress
                              is at the starting address of this block.
  @param[in]  LbaLength       The length of the logical block to be erased.

  @retval     EFI_SUCCESS.      Operation is successful.
  @retval     EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceBlockErase (
  IN    UINTN                     PAddress,
  IN    UINTN                     LbaLength
  )
{
//[-start-160426-IB10860196-add]//
//[-start-160411-IB07220059-modify]//
//[-start-160819-IB07220131-remove]//
//   FlashDeviceCheckSmm();
//[-end-160819-IB07220131-remove]//
  SetMem((VOID*)PAddress, LbaLength, 0xFF);

  UpdateNVM(PAddress - mBufferAddress, (VOID*)PAddress, LbaLength, mHeci2Protocol);
//[-start-160819-IB07220131-remove]//
//   FlashDeviceCheckSmm();
//[-end-160819-IB07220131-remove]//
//[-end-160411-IB07220059-modify]//
//[-end-160426-IB10860196-add]//

  return EFI_SUCCESS;
}



/**
  Lock or unlock the block staring at PAddress.

  @param[in]  PAddress        The starting physical address of region to be (un)locked.
  @param[in]  LbaLength       The length of the logical block to be erased.
  @param[in]  Lock            TRUE to lock. FALSE to unlock.

  @retval     EFI_SUCCESS.      Operation is successful.
  @retval     EFI_DEVICE_ERROR  If there is any device errors.

**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceBlockLock (
  IN    UINTN                          PAddress,
  IN    UINTN                          LbaLength,
  IN    BOOLEAN                        Lock
  )
{
  return EFI_SUCCESS;
}


VOID
EFIAPI
LibFvbFlashDeviceVirtualAddressChangeNotifyEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  gRT->ConvertPointer (0, (VOID **) &mHeci2Protocol);
  gRT->ConvertPointer (0, (VOID **) &mBufferAddress);
//[-start-160426-IB10860196-add]//  
  gRT->ConvertPointer (0, (VOID **) &mCacheAddress);
  HeciSetReadWriteCache ((VOID *)(UINTN)mCacheAddress);
//[-end-160426-IB10860196-add]//
 
}


/**
  The library constructor.

  The function does the necessary initialization work for this library
  instance. Please put all initialization works in it.

  @param[in]  ImageHandle       The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable       A pointer to the EFI system table.

  @retval     EFI_SUCCESS       The function always return EFI_SUCCESS for now.
                                It will ASSERT on error for debug version.
  @retval     EFI_ERROR         Please reference LocateProtocol for error code details.
**/
EFI_STATUS
EFIAPI
LibFvbFlashDeviceSupportInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
FlashDeviceEndOfPostEvent (
	IN CONST EFI_GUID						*Protocol,
	IN VOID 								*Interface,
	IN EFI_HANDLE							Handle
	)

/*++

Routine Description:

Arguments:

  Standard event notification function arguments:
  Event         - the event that is signaled.
  Context       - not used here.

Returns:

--*/
{
//[-start-160317-IB03090425-add]//
	CHIPSET_CONFIGURATION       SystemConfiguration;
	UINTN                       DataSize;
//[-end-160317-IB03090425-add]//
	EFI_STATUS					  Status;
//[-start-160803-IB07220122-add]//
  UINT32                      VariableAttributes;
//[-end-160803-IB07220122-add]//

	  while (mHeci2Protocol == NULL) {
		Status = gSmst->SmmLocateProtocol (&gEfiHeciSmmProtocolGuid, NULL, (VOID**)&mHeci2Protocol);
	  }
	
	  DEBUG((EFI_D_INFO, "EndOfService Set Done %x\n", mHeci2Protocol));
//[-start-160317-IB03090425-add]//
	  DataSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
    ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
// 	  Status = GetSystemConfigData(&SystemConfiguration, &DataSize);
    Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    &VariableAttributes,
                    &DataSize,
                    &SystemConfiguration
                    ); 
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe (&SystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
    }
//[-end-160803-IB07220122-modify]//
	  if ( (!EFI_ERROR(Status)) && (SystemConfiguration.SeCLockDir == 1) ) {
		Heci2LockDirectory(PLATFORM_CONFIG_STORE_DIR_NAME, mHeci2Protocol);
	  }
//[-end-160317-IB03090425-add]//

return EFI_SUCCESS;
}



/*
  Event handle for Exit boot services

  @param[in] Event
  @param[in] ParentImageHandle
*/
EFI_STATUS
EFIAPI
FlashDeviceEndOfServicesEvent(
	IN CONST EFI_GUID						*Protocol,
	IN VOID 								*Interface,
	IN EFI_HANDLE							Handle
  )
{
  EFI_PEI_HOB_POINTERS		 HobPtr;
  MBP_CURRENT_BOOT_MEDIA		 *BootMediaData = NULL;
  HobPtr.Guid   = GetFirstGuidHob (&gEfiBootMediaHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  BootMediaData = (MBP_CURRENT_BOOT_MEDIA*) GET_GUID_HOB_DATA (HobPtr.Guid);
  if (BootMediaData->PhysicalData != BOOT_FROM_SPI) {  //Not Boot from SPI should Send Proxy State Message
    Heci2GetProxyStateNoResp(mHeci2Protocol);
  }
  return EFI_SUCCESS;
}



VOID
InitEventSmm()
{

	//
	// Register EFI_SMM_END_OF_DXE_PROTOCOL_GUID notify function.
	//
	VOID									*Registration;
	VOID									*Registration2;
	EFI_STATUS  Status;
	Registration = NULL;
	Status = gSmst->SmmRegisterProtocolNotify (
					  &gEfiCseEndofPostGuid,
					  FlashDeviceEndOfPostEvent,
					  &Registration
					  );
	ASSERT_EFI_ERROR (Status);
	Registration2 = NULL;

Status = gSmst->SmmRegisterProtocolNotify (
				  &gEfiCseEndofServicesGuid,
				  FlashDeviceEndOfServicesEvent,
				  &Registration2
				  );
ASSERT_EFI_ERROR (Status);
}

VOID
FlashDeviceCheck (
  VOID
  )
{
  UINT32        Data;
  UINTN         Count;
//[-start-160714-IB11270157-modify]//
//[-start-160804-IB07400769-modify]//
  if (IsTxeDataRegionAvailable()) {
//[-end-160804-IB07400769-modify]//
    if (mHeci2Protocol != NULL) {
      Data = 1;
  
      // TODO: Determine the proper delay time for CSE response and update this loop.
      for (Count = 0; Count < 100; Count ++) {
        mHeci2Protocol->GetSeCStatus(&Data);
  
        if (Data == 0) {
          break;
        }
  
        MicroSecondDelay(1000);
      }
    }
  }
//[-end-160714-IB11270157-modify]//
}
//[-start-160426-IB10860196-remove]//
//VOID 
//HeciSetReadWriteCache(
//  VOID*  Address
//);
//[-start-160426-IB10860196-remove]//

PHYSICAL_ADDRESS
EFIAPI
LibFvbFlashDeviceMemoryMap (
  VOID
  )
{
  PHYSICAL_ADDRESS    MemoryAddress;
  UINT32              Address;
  VOID                *Buffer;
//[-start-160426-IB10860196-add]//
  PHYSICAL_ADDRESS    CacheAddress;
  EFI_STATUS          Status;
//[-end-160426-IB10860196-add]//

  //
  // Retrieve the BS data buffer reserved by PEIM.
  //
  Address = PcdGet32(PcdFlashNvStorageVariableBase);

  //
  // Allocate the number of 4K pages buffer to support CSE DMA requirement.
  // Temporary Workaround: Since the address range provided to CSE can't be in SMRAM, must use the pre-allocated buffer
  //                       when needing to read/write NVM file(s). The pre-allocated 64KB buffer must be initialized
  //                       by HeciSetReadWriteCache() before the buffer can be used by the SMM variable service.
  //                       For non SMM variables in PEI/DXE phase, the pre-allocated buffer is not need because the NVM file 
  //                       service allocates the temporary pages with EfiBootServiceData memory type automatically.
  //
//[-start-160426-IB10860196-modify]//  
  Status = gBS->AllocatePages (
                  AllocateAnyPages, 
                  EfiRuntimeServicesData, 
                  EFI_SIZE_TO_PAGES(MAX_VARIABLE_SIZE), 
                  &CacheAddress);
  ASSERT_EFI_ERROR (Status);
  mCacheAddress = (UINTN)CacheAddress;  
  HeciSetReadWriteCache ((VOID *)(UINTN)mCacheAddress);
//[-end-160426-IB10860196-modify]//

  DEBUG((EFI_D_INFO, "LibFvbFlashDeviceMemoryMap Address: %x\n", Address));

  //
  // Allocate a new RT data buffer for NV storage.
  //
//   Buffer = AllocateAlignedRuntimePages(0x80, 0x10000);
  Buffer = (VOID*)(UINTN)Address;
  CopyMem(Buffer, (VOID*)(UINTN)Address, 0x80000);

//  Address = (Address + 0x1000) & (~0xFFF);
  
//  CopyMem((VOID*)(UINTN)Address, Buffer, 0x40000);
  DEBUG((EFI_D_INFO, "LibFvbFlashDeviceMemoryMap2 Address: %x\n", Buffer));

  //
  // Init NVM file based on the data of old BS data buffer.
  //
  InitNVM((VOID*)(UINTN)Address, 0x80000);
  MemoryAddress = (PHYSICAL_ADDRESS)(UINTN)Buffer;
  mBufferAddress = (UINTN)MemoryAddress;

  //
  // Update PcdFlashNvStorageVariableBase to engage new NV storage buffer residing in RT data area.
  //
  PcdSet32(PcdFlashNvStorageVariableBase, (UINT32)(UINTN)Buffer);
  PcdSet32(PcdFlashNvStorageFtwWorkingBase, (UINT32)(UINTN)Buffer + PcdGet32(PcdFlashNvStorageVariableSize));
  PcdSet32(PcdFlashNvStorageFtwSpareBase, (UINT32)(UINTN)Buffer + PcdGet32(PcdFlashNvStorageVariableSize) + PcdGet32(PcdFlashNvStorageFtwWorkingSize));
  InitEventSmm();
//[-start-160324-IB07400711-add]//
  DEBUG ((EFI_D_INFO, "FlashDeviceSmmLib.c:PcdFlashNvStorageVariableBase = %x\n", PcdGet32(PcdFlashNvStorageVariableBase)));
  DEBUG ((EFI_D_INFO, "FlashDeviceSmmLib.c:PcdFlashNvStorageFtwWorkingBase = %x\n", PcdGet32(PcdFlashNvStorageFtwWorkingBase)));
  DEBUG ((EFI_D_INFO, "FlashDeviceSmmLib.c:PcdFlashNvStorageFtwSpareBase = %x\n", PcdGet32(PcdFlashNvStorageFtwSpareBase)));
//[-end-160324-IB07400711-add]//
  return MemoryAddress;
}
