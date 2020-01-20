/** @file
  This driver provides IHISI interface in SMM mode

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "Fbts.h"
#include <Library/VariableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseOemSvcKernelLib.h>

UINT8                                     mA0A1MEdelta;

//[-start-151124-IB10860189-modify]//
EFI_STATUS
EFIAPI
ChvFlashPartInfo (
  VOID
  );


/*
STATIC
IHISI_REGISTER_TABLE
CHIPSET_FBTS_REGISTER_TABLE[] = {
  //
  // AH=17h
  //
  { FBTSGetRomFileAndPlatformTable, "S17Cs_GetPlatformTb", GetRomFileAndPlatformTable}, \

  //
  // AH=1Ch
  //
  { FBTSGetATpInformation, "S1CCs_GetATpInfo000", FbtsGetATpInformation}, \

  //
  // AH=1Fh
  //
  { FBTSApHookPoint, "S1FCs_ApHookForBios", FbtsApHookForBios}, \

};*/
//[-end-151124-IB10860189-modify]//


/**
  Check the flash region whether is used.

  @param[in] FlashRegion  Flash Region x Register (x = 0 - 3).

  @retval TRUE            This  region is used
  @retval FALSE           This  region is not used
**/
STATIC
BOOLEAN
CheckFlashRegionIsValid (
  IN UINT32   FlashRegion
  )
{
  BOOLEAN         Flag = TRUE;

//[-start-160809-IB11270161-modify]//
  //
  // the Region Base must be programmed to 0x7FFFh and the Region Limit
  // to 0x0000h to disable the region.
  //
  // Now, the tool programmed 0x0fff to base and 0x000 to limit to indicate
  // this is region is not used.
  //

  //
  //The limit block address must be greater than or equal to the base block address
  //
  if ((FlashRegion & 0x7fff) > (FlashRegion >> 16 & 0x7fff)) {
    Flag = FALSE;
  }
//[-end-160809-IB11270161-modify]//

  return Flag;
}

//[-start-151124-IB10860189-add]//
EFI_STATUS
EFIAPI
ChvFlashPartInfo(
  VOID
  )
{
  FBTS_FLASH_DEVICE                         FlashDevice;
  UINT16                                    BlockMap[3];
  UINT8                                     *FlashInfo;
  UINTN                                     FlashInfoSize;
  UINT8                                     *FlashBlockMap;
 
  //
  // DS:EDI - Pointer to flash part information structure.
  //
  FlashInfo = (UINT8 *) (UINTN) mH2OIhisi->ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RDI);
  FlashInfoSize = sizeof(FBTS_FLASH_DEVICE);
  FlashBlockMap = (UINT8 *) (UINTN) mH2OIhisi->ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RSI);

  //Due to rom size is different with image size, we need to update the correct image size to flash tool.
//[-start-170215-IB07400840-modify]//
  if (PcdGet32(PcdFlashSpiRomSize) == 0x1000000) {
    //
    // 16M BIOS Image
    //
    BlockMap[0] = 0x10; 
    BlockMap[1] = 0x1000; 
    BlockMap[2] = 0xFFFF;
    FlashDevice.Size = 0x08;
  } else {
    //
    // 8M BIOS Image
    //
    BlockMap[0] = 0x10; 
    BlockMap[1] = 0x800; 
    BlockMap[2] = 0xFFFF;
    FlashDevice.Size = 0x07;
  }
//[-end-170215-IB07400840-modify]//

  CopyMem (
    (VOID *) FlashInfo,
    &FlashDevice,
    FlashInfoSize
    );

  CopyMem ((VOID *) FlashBlockMap, BlockMap, sizeof (FD_BLOCK_MAP));
//[-start-170215-IB07400840-add]//
  DEBUG ((EFI_D_ERROR, "FlashDevice.Size = %x\n", FlashDevice.Size));
//[-end-170215-IB07400840-add]//
  return IHISI_SUCCESS;

  
}
//[-end-151124-IB10860189-add]//

/**
  AH=1Ch, Get AT-p information.
  Passing information to flash program on whether
  if current BIOS (AT-p capable) can be flashed with
  other BIOS ROM file
  AT-p: (Anti-Theft PC Protection).

  @retval EFI_SUCCESS   Success returns.
**/
EFI_STATUS
FbtsGetATpInformation (
  VOID
  )
{
  UINT32                                Ecx_data = 0;


  if (!PcdGetBool (PcdAntiTheftSupported)) {
      Ecx_data &= ~(bit(7));
      //
      // Return ECX for tool to determin proper error message.
      // Please refer to IHISI spec.
      //
      mH2OIhisi->WriteCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RCX, Ecx_data);
      return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

/**
  Get Flash table from Rom file.
  if DescriptorMode is true, the FlashTable will be filled.
  if the descriptor is false,the FlashTable will be filled RegionTypeEos(0xff) directly.

  @param[in]  InputDataBuffer    the pointer to Rom file.
  @param[in]  DataBuffer         IN: the input buffer address.
                                 OUT:the flash region table from rom file.
  @param[in]  DescriptorMode     TRUE is indicate this Rom file is descriptor mode
                                 FALSE is indicate this Rom file is non-descriptor mode

  @retval EFI_SUCCESS            Successfully returns
**/
STATIC
EFI_STATUS
GetRomFileFlashTable (
  IN       UINT8           *InputDataBuffer,
  IN OUT   FLASH_REGION    *DataBuffer,
  IN       BOOLEAN         DescriptorMode
  )
{
  UINT32          Frba;
  UINT32          Fmba;
  UINT8           Index;
  UINT32          ReadAccess;
  UINT32          WriteAccess;
  UINT32          FlashMap0Reg;
  UINT32          FlashMap1Reg;
  UINT32         *FlashMasterPtr;
  UINT32         *FlashRegionPtr;

  //
  //calulate Flash region base address
  //
  FlashMap0Reg = *((UINT32 *)(InputDataBuffer + FLASH_MAP_0_OFFSET + mA0A1MEdelta));
  Frba = (FlashMap0Reg & FLASH_REGION_BASE_MASK) >> 12;
  FlashRegionPtr = (UINT32 *)(InputDataBuffer + Frba);
  //
  //calulate Flash master base address
  //

  FlashMap1Reg = *((UINT32 *)(InputDataBuffer + FLASH_MAP_1_OFFSET + mA0A1MEdelta));

  Fmba = (FlashMap1Reg & FLASH_MASTER_BASE_MASK) << 4;
  FlashMasterPtr = (UINT32 *)(InputDataBuffer + Fmba);
//[-start-160809-IB11270161-modify]//
  if (DescriptorMode) {
    for (Index = APL_DESC_REGION; Index < APL_MAX_FLASH_REGION; Index++,  FlashRegionPtr++) {
      if (CheckFlashRegionIsValid (*FlashRegionPtr)){

        DataBuffer->Type = Index;
        DataBuffer->Offset = (*FlashRegionPtr & 0x7fff) << 12;
        DataBuffer->Size = ((*FlashRegionPtr >> 16 & 0x7fff) - (*FlashRegionPtr & 0x7fff) + 1) << 12;
        //
        //Bios primary master always has access permissions to it's primary region
        //
        if (Index == APL_BIOS_REGION) {
          DataBuffer->Access = ACCESS_AVAILABLE;
        } else {
          ReadAccess = (*FlashMasterPtr >> (FLASH_MASTER_1_READ_ACCESS_BIT + Index)) & ACCESS_AVAILABLE;
          WriteAccess = (*FlashMasterPtr >> (FLASH_MASTER_1_WRITE_ACCESS_BIT + Index)) & ACCESS_AVAILABLE;
          if (ReadAccess == ACCESS_AVAILABLE && WriteAccess == ACCESS_AVAILABLE) {
            DataBuffer->Access = ACCESS_AVAILABLE;
          } else {
            DataBuffer->Access = ACCESS_NOT_AVAILABLE;
          }
        }
        DataBuffer++;
      }
    }
  }
  DataBuffer->Type = APL_REGION_TYPE_OF_EOS;
//[-end-160810-IB11270161-modify]//

  return EFI_SUCCESS;
}


/**
  AH = 17h , Get platform and Rom file flash descriptor region.

  @retval EFI_SUCCESS   Success returns.
**/
EFI_STATUS
GetRomFileAndPlatformTable (
  VOID
  )
{
  EFI_STATUS      Status;
  UINTN           DataSize;
  FLASH_REGION   *FlashRegionPtr;
  UINT8          *InputDataBuffer;
  UINT8          *OutputDataBuffer;

  Status = EFI_UNSUPPORTED;
  DataSize = (UINTN) mH2OIhisi->ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RCX);
  if (DataSize == 0) {
    return Status;
  }
  OutputDataBuffer = (UINT8 *)(UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RDI);

  InputDataBuffer  = (UINT8 *)(UINTN) mH2OIhisi->ReadCpuReg32 (EFI_SMM_SAVE_STATE_REGISTER_RSI);
  if (mH2OIhisi->BufferOverlapSmram ((VOID *) OutputDataBuffer, sizeof(FBTS_REGION_INFORMATION_STRUCTURE)) ||
      mH2OIhisi->BufferOverlapSmram ((VOID *) InputDataBuffer, DataSize)) {
    return IHISI_INVALID_PARAMETER;
  }

  //
  //we must get platform region table through read chipset register, because
  //1. we can override access permissions in the Flash Descriptor through BMWAG and BMRAG
  //2. Flash regions may be haven't read/write access.
  //
  Status = mSmmFwBlockService->GetFlashTable (
                                 mSmmFwBlockService,
                                 OutputDataBuffer
                                 );

  OutputDataBuffer += ROM_FILE_REGION_OFFSET;
  FlashRegionPtr = (FLASH_REGION *) OutputDataBuffer;


  if (*((UINT32 *) (InputDataBuffer + 0x10)) == FLASH_VALID_SIGNATURE) {
    mA0A1MEdelta = 0x10; // B0 stepping, ME has been shift 0x10 bytes
  }
  else{
    mA0A1MEdelta = 0x0; // else keep what it is
  }

  //
  //check Rom file is whether descriptor mode
  //

  if (*((UINT32 *)(InputDataBuffer + mA0A1MEdelta)) == FLASH_VALID_SIGNATURE) {
    Status = GetRomFileFlashTable (InputDataBuffer, FlashRegionPtr, TRUE);
  } else {
    Status = GetRomFileFlashTable (InputDataBuffer, FlashRegionPtr, FALSE);
  }
  return Status;
}


/*++

Routine Description:

  This call back function will be invoked several times during flash process.
  BIOS can know which step is running now. 
  BIOS can base on it to do specific hook such as EC idle and weak up. 

Arguments:

  CL - denote the start of AP process.
       CL = 0x00, AP terminate. (Before IHISI 0x16)
       CL = 0x01, AP start. (After IHISI 0x10) 
       CL = 0x02, Start to read ROM. (Before IHISI 0x14) 
       CL = 0x03, Start to write ROM. (Before IHISI 0x15)
       CL = 0x04, Start to write EC. (Before IHISI 0x20) 
       CL = 0x05, Before dialog popup. 
       CL = 0x06, After dialog close and continue running. 
       
Returns:

--*/
EFI_STATUS
FbtsApHookForBios (
  VOID
   )
{
  UINT8         ApState;
  EFI_STATUS    Status;

  ApState = (UINT8) mH2OIhisi->ReadCpuReg32(EFI_SMM_SAVE_STATE_REGISTER_RCX);  // CL

  switch (ApState) {
    case 0x00:
      //
      // Here provided a hook before AP terminate.
      //
      if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
        OemSvcEcIdle (FALSE);
      }
      break;
      
    case 0x01:
      //
      // Here provided a hook before AP start.
      //
      
      break;

    case 0x02:
      //
      // Here provided a hook before FbtsRead.
      //
      if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
        OemSvcEcIdle (TRUE);
      }
      break;

    case 0x03:
      //
      // Here provided a hook before FbtsWrite.
      //
      if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
        OemSvcEcIdle (TRUE);
      }
      break;

    case 0x04:
      //
      // Here provided a hook before FetsWrite.
      //
      if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
        OemSvcEcIdle (TRUE);
      }
      break;

    case 0x05:
      //
      // Here provided a hook before dialog popup. (Winflash)
      //
      if (PcdGetBool (PcdEcSharedFlashSupport) && !PcdGetBool (PcdEcIdlePerWriteBlock)) {
        OemSvcEcIdle (FALSE);
      }
      break;

    case 0x06:
      //
      // Here provided a hook after dialog close and continue running. (Winflash)
      //
      
      break;
  }

  Status = OemSvcIhisiS1FHookFbtsApHookForBios (ApState);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "SmmOemSvcChipsetLib OemSvcIhisiS1FHookFbtsApHookForBios, Status : %r\n", Status));

  return EFI_SUCCESS;
}

/**
  Initialize Fbts relative services

  @retval EFI_SUCCESS        Initialize Fbts services successful.
  @return Others             Any error occurred.
**/
EFI_STATUS
FbtsInit (
  VOID
  )
{
  EFI_STATUS   Status;
//[-start-151124-IB10860189-remove]//
//IHISI_REGISTER_TABLE   *FuncTable;
//UINT16                  TableCount;
//[-end-151124-IB10860189-remove]//

  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmFwBlockServiceProtocolGuid,
                    NULL,
                    (VOID **) &mSmmFwBlockService
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
//[-start-151124-IB10860189-modify]//
//  FuncTable = CHIPSET_FBTS_REGISTER_TABLE;
//  TableCount = sizeof(CHIPSET_FBTS_REGISTER_TABLE)/sizeof(CHIPSET_FBTS_REGISTER_TABLE[0]);
//  Status = RegisterIhisiSubFunction (FuncTable, TableCount);

  Status = mH2OIhisi->RegisterCommand (FBTSGetFlashPartInfo, ChvFlashPartInfo,IhisiBelowNormalPriority);

  Status = mH2OIhisi->RegisterCommand (FBTSGetRomFileAndPlatformTable, GetRomFileAndPlatformTable,IhisiNormalPriority);

  Status = mH2OIhisi->RegisterCommand (FBTSGetATpInformation, FbtsGetATpInformation,IhisiNormalPriority);

//[-start-151229-IB07220030-remove]//
//   Status = mH2OIhisi->RegisterCommand (FBTSApHookPoint, FbtsApHookForBios,IhisiNormalPriority);
//[-end-151229-IB07220030-remove]//
//[-end-151124-IB10860189-modify]//

  if (EFI_ERROR(Status)) {
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

