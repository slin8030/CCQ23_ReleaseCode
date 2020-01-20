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


//[-start-160216-IB03090424-modify]//
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
#include <Library/DxeInsydeChipsetLib.h>
//[-start-160317-IB03090425-add]//
#include <ChipsetSetupConfig.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-end-160317-IB03090425-add]//
//[-start-160807-IB07400769-add]//
#include <Private/Library/HeciInitLib.h>
//[-end-160807-IB07400769-add]//

//[-start-160317-IB03090425-add]//
#define  PLATFORM_CONFIG_STORE_DIR_NAME   "critical"
//[-end-160317-IB03090425-add]//

extern EFI_GUID gEfiBootMediaHobGuid;

UINTN mBufferAddress = 0;
EFI_HECI_PROTOCOL  *mHeci2Protocol = NULL;


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
  if (!EfiAtRuntime()) {
    DEBUG((EFI_D_INFO, "Write Variable to NVM. Offset: 0x%x, Size: 0x%x\n", PAddress - mBufferAddress, *NumBytes));
  }

  FlashDeviceCheck();
  CopyMem((VOID*)PAddress,Buffer,*NumBytes);
//[-start-160808-IB07220123-modify]//
  UpdateNVM(PAddress - mBufferAddress, (VOID*)PAddress, *NumBytes, NULL);
//[-end-160808-IB07220123-modify]//
  FlashDeviceCheck();
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
//[-start-160411-IB07220059-modify]//
  if (!EfiAtRuntime()) {
    DEBUG((EFI_D_INFO, "Erase Variable to NVM. Offset: 0x%x, Size: 0x%x\n", PAddress - mBufferAddress, LbaLength));
  }

  FlashDeviceCheck();
  SetMem((VOID*)PAddress, LbaLength, 0xFF);
//[-start-160808-IB07220123-modify]//
  UpdateNVM(PAddress - mBufferAddress, (VOID*)PAddress, LbaLength, NULL);
//[-end-160808-IB07220123-modify]//
  FlashDeviceCheck();
//[-end-160411-IB07220059-modify]//
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
}


VOID
InitHeci2 (
  VOID
  )
{
  EFI_STATUS    Status;

  Status = gBS->LocateProtocol (
                  &gEfiHeciSmmRuntimeProtocolGuid,
                  NULL,
                  &mHeci2Protocol
                  );
  ASSERT_EFI_ERROR(Status);
}


VOID
EFIAPI
FlashDeviceEndOfPostEvent (
  IN EFI_EVENT    Event,
  IN VOID*        Context
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
  CHIPSET_CONFIGURATION         SystemConfiguration;
  UINTN                         DataSize;
  EFI_STATUS                    Status;
//[-end-160317-IB03090425-add]//
//[-start-160803-IB07220122-add]//
  UINT32                        VariableAttributes;
//[-end-160803-IB07220122-add]//

  DEBUG((EFI_D_INFO, "Get End of Post Heci Event\n"));
  while (mHeci2Protocol == NULL) {
    DEBUG((EFI_D_INFO, "Get Heci Protocol %x\n", mHeci2Protocol));
    InitHeci2();
  }

//[-start-160317-IB03090425-add]//
  DEBUG((EFI_D_INFO, "Get Heci Protocol %x\n", mHeci2Protocol));
  DataSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SystemConfiguration, &DataSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  &VariableAttributes,
                  &DataSize,
                  &SystemConfiguration
                  ); 
//[-end-160803-IB07220122-modify]//
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
  }

  if ( (!EFI_ERROR(Status)) && (SystemConfiguration.SeCLockDir == 1) ) {
    Heci2LockDirectory(PLATFORM_CONFIG_STORE_DIR_NAME, mHeci2Protocol);
  }
//[-end-160317-IB03090425-add]//
  DEBUG((EFI_D_INFO, "EndOfService Set Done\n"));
  gBS->CloseEvent (Event);
}

/*
  Event handle for Exit boot services

  @param[in] Event
  @param[in] ParentImageHandle
*/
VOID
EFIAPI
FlashDeviceEndOfServicesEvent (
  IN EFI_EVENT           Event,
  IN VOID                *Context
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
  gBS->CloseEvent (Event);
}


EFI_STATUS
InitEvent (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_EVENT     Event;
  VOID          *RegistrationLocal;

  //
  // Create event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  FlashDeviceEndOfServicesEvent,
                  (VOID *) NULL,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for an installation of protocol interface
  //

  Status = gBS->RegisterProtocolNotify (
                  &gEfiCseEndofServicesGuid,
                  Event,
                  &RegistrationLocal
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
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
  EFI_STATUS	  Status;
  EFI_EVENT	  Event;
  VOID		  *RegistrationLocal;

//[-start-160712-IB07220113-add]//
  if (PcdGetBool (PcdUseFastCrisisRecovery)) {
    if (GetBootModeHob () == BOOT_IN_RECOVERY_MODE) {
      return EFI_SUCCESS;
    }
  }
//[-end-160712-IB07220113-add]//
  
	//
	// Create event
	//
	Status = gBS->CreateEvent (
					EVT_NOTIFY_SIGNAL,
					TPL_NOTIFY,
					FlashDeviceEndOfPostEvent,
					(VOID *) NULL,
					&Event
					);
	ASSERT_EFI_ERROR (Status);
	
	//
	// Register for an installation of protocol interface
	//
	
  Status = gBS->RegisterProtocolNotify (
					&gEfiHeciSmmRuntimeProtocolGuid,
					Event,
					&RegistrationLocal
					);
  ASSERT_EFI_ERROR (Status);
	
  Status = gBS->CreateEventEx (
  				EVT_NOTIFY_SIGNAL,
  				TPL_NOTIFY,
  				LibFvbFlashDeviceVirtualAddressChangeNotifyEvent,
   				NULL,
				&gEfiEventVirtualAddressChangeGuid,
				&Event
				);
	
  ASSERT_EFI_ERROR (Status);
  return Status;
}


PHYSICAL_ADDRESS
EFIAPI
LibFvbFlashDeviceMemoryMap (
  VOID
  )
{
  PHYSICAL_ADDRESS    MemoryAddress;
  UINT32              Address;
  VOID                *Buffer;

  Address = PcdGet32(PcdFlashNvStorageVariableBase);

  DEBUG((EFI_D_INFO, "LibFvbFlashDeviceMemoryMap Address: %x\n", Address));

  Buffer = AllocateAlignedRuntimePages(0x80, 0x10000);
  CopyMem(Buffer, (VOID*)(UINTN)Address, 0x80000);
  MemoryAddress = (PHYSICAL_ADDRESS)(UINTN)Buffer;
  InitNVM(Buffer, 0x80000);
  mBufferAddress = (UINTN)MemoryAddress;
  PcdSet32(PcdFlashNvStorageVariableBase, (UINT32)(UINTN)Buffer);
  DEBUG((EFI_D_INFO, "LibFvbFlashDeviceMemoryMap %x\n", MemoryAddress));
  PcdSet32(PcdFlashNvStorageFtwWorkingBase, (UINT32)(UINTN)Buffer + PcdGet32(PcdFlashNvStorageVariableSize));
  PcdSet32(PcdFlashNvStorageFtwSpareBase, (UINT32)(UINTN)Buffer + PcdGet32(PcdFlashNvStorageVariableSize) + PcdGet32(PcdFlashNvStorageFtwWorkingSize));
  InitEvent();
  PcdSet32(PcdFlashNvStorageFactoryCopyBase, PcdGet32(PcdFlashNvStorageFtwSpareBase) + PcdGet32(PcdFlashNvStorageFtwSpareSize));
//[-start-160324-IB07400711-add]//
  DEBUG ((EFI_D_INFO, "FlashDeviceLib.c:PcdFlashNvStorageVariableBase = %x\n", PcdGet32(PcdFlashNvStorageVariableBase)));
  DEBUG ((EFI_D_INFO, "FlashDeviceLib.c:PcdFlashNvStorageFtwWorkingBase = %x\n", PcdGet32(PcdFlashNvStorageFtwWorkingBase)));
  DEBUG ((EFI_D_INFO, "FlashDeviceLib.c:PcdFlashNvStorageFtwSpareBase = %x\n", PcdGet32(PcdFlashNvStorageFtwSpareBase)));
  DEBUG ((EFI_D_INFO, "FlashDeviceLib.c:PcdFlashNvStorageFactoryCopyBase = %x\n", PcdGet32(PcdFlashNvStorageFactoryCopyBase)));
//[-end-160324-IB07400711-add]//
  return MemoryAddress;
}
//[-end-160216-IB03090424-modify]//
