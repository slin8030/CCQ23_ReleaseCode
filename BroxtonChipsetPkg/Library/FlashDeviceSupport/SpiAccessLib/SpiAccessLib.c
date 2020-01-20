/** @file
  SpiAccessLib implementation for SPI Flash Type devices

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/FdSupportLib.h>
#include <Library/SpiAccessLib.h>
#include <ScAccess.h>
#include <MmioAccess.h>
#include <Library/DebugLib.h>
#include <Library/IrsiRegistrationLib.h>
#include <Library/SpiAccessInitLib.h>
#include <ChipsetSpiAccess.h>

#define SIZE_OF_SPI_VTBA_ENTRY          (S_SPI_VTBA_JID0 + S_SPI_VTBA_VSCC0)
#ifndef B_SPI_LVSCC_CAP_MASK
#define B_SPI_LVSCC_CAP_MASK             0x0000FFFF // Capabilities Mask
#endif
#ifndef B_SPI_UVSCC_CAP_MASK
#define B_SPI_UVSCC_CAP_MASK             0x0000FFFF // Capabilities Mask
#endif

//[-start-151124-IB08450330-add]//
#define R_SPI_FRBA                       0x40               //Descriptor FRBA offset 
#define R_SPI_FLREG0                     R_SPI_FRBA + 0x00  //Region entry of Descriptor
#define R_SPI_FLREG1                     R_SPI_FRBA + 0x04  //Region entry of IFWI
#define R_SPI_FLREG2                     R_SPI_FRBA + 0x08  //Region entry of TXE
#define R_SPI_FLREG4                     R_SPI_FRBA + 0x10  //Region entry of PDR
#define R_SPI_FLREG5                     R_SPI_FRBA + 0x14  //Region entry of Device Expansion
//[-end-151124-IB08450330-add]//
#ifdef __GNUC__
#pragma GCC optimize ("O0")
#else
#pragma optimize("", off)
#endif
//[-start-161021-IB07400801-add]//
#define REGION_ACCESS_DESC_REGION       BIT0
#define REGION_ACCESS_IFWI_REGION       BIT1
#define REGION_ACCESS_TXE_ROM_REGION    BIT2
#define REGION_ACCESS_PDR_REGION        BIT4
#define REGION_ACCESS_DE_REGION         BIT5
//[-end-161021-IB07400801-add]//

//[-start-151124-IB08450330-add]//
#define MAXIMUM_SPI_TRANSFER_SIZE    64
//[-end-151124-IB08450330-add]//
SPI_CONFIG_BLOCK        mSpiConfigBlock;

//[-start-151124-IB08450330-modify]//
UINT32                  mValidWriteRegionStart;
UINT32                  mValidWriteRegionEnd;
UINT32                  mValidReadRegionStart;
UINT32                  mValidReadRegionEnd;
UINTN                   mSpiMmioBeginMemoryAddress = 0;
UINT32                  mSpiMmioBeginOffset = 0;
UINT32                  mSpiMmioSize = 0;

UINT8                   *mSpiBase = NULL;
//[-end-151124-IB08450330-modify]//
//[-start-160810-IB11270161-add]//
typedef enum {
  APL_DescRegionType,
  APL_BiosRegionType,
  APL_MeRegionType,
  APL_GbeRegionType,
  APL_PlatformDataRegionType,
  APL_DeviceExpansionRegionType,
  APL_MaxFlashRegionType,
  APL_EndOfRegionType  = 0xff,
} APL_FLASH_REGION_TYPE1; 
//[-end-160810-IB11270161-add]//
//[-start-151124-IB08450330-remove]//
///**
//  Read SPI configuration from PCH SPI register
//
//  @param CacheEnable            It indicates whether the cache is enabled (TRUE)
//                                or not
//
//  @retval EFI_SUCCESS           Function successfully returned
//
//**/
//EFI_STATUS
//SpiReadConfiguration (
//  IN  BOOLEAN     CacheEnable
//  )
//{
//  STATIC UINT8        BiosCtlSave = 0;
//
//
//  if (CacheEnable) {
//    BiosCtlSave = (MmSpi8(R_SPI_BCR) & B_LPC_BIOS_CNTL_SRC);
//    MmSpi8AndThenOr (R_SPI_BCR, ~B_LPC_BIOS_CNTL_SRC, (0x02 << 2));
//  } else {
//    MmSpi8AndThenOr (R_SPI_BCR, ~B_LPC_BIOS_CNTL_SRC, BiosCtlSave);
//  }
//
//  return EFI_SUCCESS;
//}
//[-end-151124-IB08450330-remove]//


/**
  Check the SPI offset is fall in valid region or not

  @param Offset                 Target offset of SPI ROM to be validated
  @param Size                   Region size
  @param WriteOperation         TRUE: Check for write operation
                                FALSE: Check for read operation

  @retval TRUE                  The specified region is valid
  @retval FALSE                 The specified region is invalid

**/
//[-start-151216-IB07220025-modify]//
//[-start-151124-IB08450330-modify]//
BOOLEAN
CheckValidRegion(
  IN UINT32                     Offset,
  IN UINTN                      Size,
  IN BOOLEAN                    WriteOperation
  )
{

//[-start-161021-IB07400801-modify]//
   if (WriteOperation) 
   {
    return (Offset >= mValidWriteRegionStart) && (Offset + Size - 1 <= mValidWriteRegionEnd) ? TRUE : FALSE;
   }
   return (Offset >= mValidReadRegionStart) && (Offset + Size - 1 <= mValidReadRegionEnd) ? TRUE : FALSE;
//[-end-161021-IB07400801-modify]//
}
//[-end-151124-IB08450330-modify]//
//[-end-151216-IB07220025-modify]//

/**
  Wait for SPI device not busy

  @retval EFI_SUCCESS           Function successfully returned

**/
EFI_STATUS
WaitForSpiDeviceNotBusy(
//[-start-151124-IB08450330-remove]//
//  IN BOOLEAN                    Type
//[-end-151124-IB08450330-remove]//
  )
{
  volatile  UINT16   SpiStatus;

//[-start-151124-IB08450330-modify]//
  //
  // Wait until "SPI Cycle In Progress" bit is 0
  //
  do
  {
    SpiStatus = MmSpi16(R_SPI_HSFS);
  }
  while ((SpiStatus & B_SPI_HSFS_SCIP) == B_SPI_HSFS_SCIP);
  //
  // Clear status bits
  //
  MmSpi16Or(R_SPI_HSFS, B_SPI_HSFS_AEL | B_SPI_HSFS_FCERR | B_SPI_HSFS_FDONE);
//[-end-151124-IB08450330-modify]//

  return EFI_SUCCESS;
}

/**
  Wait for SPI cycle completed

  @retval EFI_SUCCESS           Function successfully returned
  @retval EFI_DEVICE_ERROR      There is error during operation

**/
EFI_STATUS
//[-start-170215-IB07400840-modify]//
WaitForSpiCycleComplete2(
//[-end-170215-IB07400840-modify]//
//[-start-151124-IB08450330-remove]//
//  IN BOOLEAN                    Type
//[-end-151124-IB08450330-remove]//
  )
{
  volatile  UINT16   SpiStatus;

//[-start-151124-IB08450330-modify]//
  //
  // Wait until "Flash Cycle Done" bit is 1
  //
  do
  {
    SpiStatus = MmSpi16(R_SPI_HSFS);
  }
  //while ((SpiStatus & B_SPI_HSFS_FDONE ) != B_SPI_HSFS_FDONE && !(SpiStatus & B_SPI_HSFS_FCERR));
  while ((SpiStatus & B_SPI_HSFS_SCIP) != 0);
  //
  // Clear status bits
  //
  MmSpi16Or(R_SPI_HSFS, B_SPI_HSFS_FCERR | B_SPI_HSFS_FDONE);
  //
  // Check for errors
  //
  if ( (SpiStatus & B_SPI_HSFS_FCERR) != 0 ) return EFI_DEVICE_ERROR;
//[-end-151124-IB08450330-modify]//
  return EFI_SUCCESS;
}

/**
  Starts an SPI transaction

  @param Address                Flash linear address
  @param Length                 Flash data byte count
  @param FlashCycle             Flash cycle code 

  @retval EFI_SUCCESS           Function successfully returned
  @retval EFI_DEVICE_ERROR      Error occured during transfering

**/
EFI_STATUS
SpiTransfer(
//[-start-151124-IB08450330-modify]//
  IN  UINT32   Address,
  IN  UINT8    Length,
  IN  UINT8    FlashCycle
//[-end-151124-IB08450330-modify]//
  )
{
//[-start-151124-IB08450330-modify]//
  WaitForSpiDeviceNotBusy();
  
  Length -= 1;

  MmSpi32(R_SPI_FADDR) = Address & B_SPI_FADDR_MASK;
  MmSpi32AndThenOr(R_SPI_HSFS, ~B_SPI_HSFS_FDBC_MASK, Length << N_SPI_HSFS_FDBC);
  MmSpi32AndThenOr(R_SPI_HSFS, ~B_SPI_HSFS_CYCLE_MASK, FlashCycle << N_SPI_HSFS_CYCLE);
  MmSpi32Or(R_SPI_HSFS, B_SPI_HSFS_CYCLE_FGO);

//[-start-170215-IB07400840-modify]//
  return WaitForSpiCycleComplete2();
//[-end-170215-IB07400840-modify]//
//[-end-151124-IB08450330-modify]//
}

//[-start-151124-IB08450330-add]//
/**
  Transfer data between SPI data registers

  @param Address                Pointer to data for transfering
  @param Length                 Data size in bytes
  @param ToDevice               TRUE for transfreing to device, Otherwise from
                                device.
**/
VOID
SpiCopyTransferData (
  IN OUT UINT8      *Address,
  IN     UINT8      Length,
  IN     BOOLEAN    ToDevice
)
{
  UINT8 Index;
  
  ASSERT (Length <= MAXIMUM_SPI_TRANSFER_SIZE);

  if (Length & 0x03) {
    //
    // Byte access
    //
    for (Index = 0; Index < Length; Index++) {
      if (ToDevice) {
        MmSpi8(R_SPI_FDATA00 + Index) = Address[Index];
      } else {
        Address[Index] = MmSpi8(R_SPI_FDATA00 + Index);
      }
    }
  } else {
    //
    // DWORD access
    //
    for (Index = 0; Index < Length; Index+= 4) {
      if (ToDevice) {
        MmSpi32(R_SPI_FDATA00 + Index) = *(UINT32 *)(Address + Index);
      } else {
        *(UINT32 *)(Address + Index) = MmSpi32(R_SPI_FDATA00 + Index);
      }
    }
  }  
}

/**
 Read SPI VID DID
 
  @param SpiVidDid              Pointer a buffer to receieve VidDid. 
**/
VOID
SpiReadId (
  IN OUT UINT8      *SpiVidDid
  )
{
  EFI_STATUS    Status;
  
  Status = SpiTransfer (
                0, 
                3,    // 3 bytes
                V_SPI_HSFS_CYCLE_READ_JEDEC_ID
                );
  if (!EFI_ERROR(Status)) {
    SpiCopyTransferData (
      SpiVidDid,
      3,
      FALSE
      );
  } else {
    SpiVidDid[0] = SpiVidDid[1] = SpiVidDid[2] = 0;
  }
}
//[-end-151124-IB08450330-add]//


/**
  Udates the block lock register for the FW block indicated by the input parameters

  @param SpiOffset              SPI ROM offse
  @param Size                   Size in bytes
  @param LockState              FULL_ACCESS                         Full access
                                WRITE_LOCK                          Write Locked
                                LOCK_DOWN                           Locked open (full access locked down).
                                LOCK_DOWN + WRITE_LOCK              Write-locked down.
                                READ_LOCK                           Read locked.
                                READ_LOCK + WRITE_LOCK              Read and write locked.
                                READ_LOCK + LOCK_DOWN               Read-locked down.
                                READ_LOCK + LOCK_DOWN + WRITE_LOCK  Read- and write-locked down.

  @retval EFI_SUCCESS           Function successfully returned

**/
EFI_STATUS
SpiFlashLock (
//[-start-151124-IB08450330-modify]//
  IN  UINT32  SpiOffset,
//[-end-151124-IB08450330-modify]//
  IN  UINTN   Size,
  IN  UINT8   LockState
  )
{
//[-start-151124-IB08450330-modify]//

  switch (LockState)
  {
    case SPI_WRITE_LOCK:
      if (mSpiConfigBlock.GlobalProtect)
      {
        //
        // Write to SPI Device Status Register (set block protection bits)
        //
        SpiCopyTransferData (
          &mSpiConfigBlock.GlobalProtectCode,
          1,
          TRUE
          );
          
        SpiTransfer (
          0,
          1,
          V_SPI_HSFS_CYCLE_WRITE_STATUS
          );
      }
      if (mSpiConfigBlock.BlockProtect)
      {
      //
      // Not support
      //
      
      //  BaseAddr = (UINTN)BaseAddress & ~(mSpiConfigBlock.BlockProtectSize - 1);
      //  MmSpi8(R_SPI_FDATA00) = mSpiConfigBlock.BlockProtectCode;
      //	for (Index = 0; Index < Size; Index += mSpiConfigBlock.BlockProtectSize)
      //	{
      //    SpiTransfer(
      //      SPI_SOFTWARE,
      //      (UINT8*)BaseAddr + Index,
      //      0,
      //      SPI_OPCODE_INDEX_LOCK,
      //      SPI_PREFIX_INDEX_WRITE_EN,
      //      1,
      //      (BOOLEAN)mSpiConfigBlock.BlockProtectDataRequired);
      //  }
      }
      break;
    case SPI_FULL_ACCESS:
      if (mSpiConfigBlock.GlobalProtect)
      {


        //
        // Write to SPI Device Status Register (clear block protection bits)
        //
        SpiCopyTransferData (
          &mSpiConfigBlock.GlobalUnprotectCode,
          1,
          TRUE
          );
          
        SpiTransfer (
          0,
          1,
          V_SPI_HSFS_CYCLE_WRITE_STATUS
          );
      }

      //
      // Not supported
      //
      
      //if (mSpiConfigBlock.BlockProtect)
      //{
      
      //  BaseAddr = (UINTN)BaseAddress & ~(mSpiConfigBlock.BlockProtectSize - 1);
      //	MmSpi8(R_SPI_FDATA00) = mSpiConfigBlock.BlockUnprotectCode;
      //	for (Index = 0; Index < Size; Index += mSpiConfigBlock.BlockProtectSize)
      //	{
      //
      //    SpiTransfer(
      //      SPI_SOFTWARE,
      //      (UINT8*)BaseAddr + Index,
      //      0,
      //      SPI_OPCODE_INDEX_UNLOCK,
      //      SPI_PREFIX_INDEX_WRITE_EN,
      //      1,
      //      (BOOLEAN)mSpiConfigBlock.BlockProtectDataRequired);
      //  }
      //}
      break;
    }
//[-end-151124-IB08450330-modify]//


  return EFI_SUCCESS;
}

/**
  Get flash number from SPI Descriptor

  @param FlashNumber            Number of SPI flash devices returned

  @retval EFI_SUCCESS           Function successfully returned
  @retval EFI_UNSUPPORTED       The SPI flash is not in Descriptor mode

**/
EFI_STATUS
GetSpiFlashNumber (
  OUT   UINT8    *FlashNumber
  )
{
  UINT32    NcValue;

  if ((MmSpi32(R_SPI_HSFS) & B_SPI_HSFS_FDV) == B_SPI_HSFS_FDV) { // is descriptor mode
    //
    // Get SPI flash number from descriptor
    //
    MmSpi32(R_SPI_FDOC) = V_SPI_FDOC_FDSS_FSDM | R_SPI_FDBAR_FLASH_MAP0;
    NcValue = (MmSpi32(R_SPI_FDOD) & B_SPI_FDBAR_NC) >> 8;
    *FlashNumber = (UINT8)(NcValue + 1);
  } else {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Offset the BIOS address from top of 4G memory address to correct BIOS region
  described in SPI Descriptor

  @param MemoryAddress          BIOS memory mapped address
  @param BiosRegionAddress      Flash ROM start address + BIOS address in flash ROM

  @retval EFI_SUCCESS           Function successfully returned
  @retval EFI_UNSUPPORTED       The SPI flash is not in Descriptor mode

**/
EFI_STATUS
MemoryToBiosRegionAddress (
  IN UINTN       MemoryAddress,
  OUT UINTN      *BiosRegionAddress
  )
{
//[-start-151124-IB08450330-modify]//
  //
  // Should be same for original code.
  //
  *BiosRegionAddress = MemoryAddress;   
//[-end-151124-IB08450330-modify]//

  return EFI_SUCCESS;
}


/**
  Update data structures related to Spi device size base on Descriptor

  @param SpiFlashNumber         Number of SPI devices in system.

  @param FdInfo                 pointer to FD_INFO of FLASH_SPI_DEVICE,
                                if COMMON SPI DEVICE is used, the contents
                                related to flash size is updated here

  @param SpiCfgBlk              pointer to SPI_CONFIG_BLOCK of FLASH_SPI_DEVICE,
                                if COMMON SPI DEVICE is used, the contents
                                related to flash size is updated here

  @retval EFI_SUCCESS           Function successfully returned
  @retval EFI_DEVICE_ERROR      Failed to update SPI device size

**/
EFI_STATUS
UpdateSpiDeviceSize(
  IN     UINT8            SpiFlashNumber,
  IN OUT FD_INFO          *FdInfo,
  IN OUT SPI_CONFIG_BLOCK *SpiCfgBlk
  )
{
  EFI_STATUS Status;
  UINT8      FlashCompDensity12;
  UINT32     FlashSize1;
  UINT32     FlashSize2;
  UINT8      CommonSpiDeviceSize;

  Status              = EFI_SUCCESS;
  FlashSize1          = 0;
  FlashSize2          = 0;
  CommonSpiDeviceSize = 0xff;

  MmSpi32 (R_SPI_FDOC) = V_SPI_FDOC_FDSS_COMP | R_SPI_FCBA_FLCOMP;
  FlashCompDensity12 = (MmSpi32 (R_SPI_FDOD) & B_SPI_FDBAR_FCBA);

  switch (FlashCompDensity12 & B_SPI_FLCOMP_COMP1_MASK) {
  case V_SPI_FLCOMP_COMP1_512KB:
    FlashSize1 = 0x80000;
    break;
  case V_SPI_FLCOMP_COMP1_1MB:
    FlashSize1 = 0x100000;
    break;
  case V_SPI_FLCOMP_COMP1_2MB:
    FlashSize1 = 0x200000;
    break;
  case V_SPI_FLCOMP_COMP1_4MB:
    FlashSize1 = 0x400000;
    break;
  case V_SPI_FLCOMP_COMP1_8MB:
    FlashSize1 = 0x800000;
    break;
  case V_SPI_FLCOMP_COMP1_16MB:
    FlashSize1 = 0x1000000;
    break;
  default:
    Status = EFI_DEVICE_ERROR;
    break;
  }

  mSpiConfigBlock.DeviceSize = FlashSize1 + FlashSize2;
  if (FdInfo->Id == ID_COMMON_SPI_DEVICE) {
    //
    // BUGBUG: Now, assume Common SPI device size to FlashSize1.
    // BUGBUG: To be modified if ROM part of different size is supported
    //
    SpiCfgBlk->DeviceSize = FlashSize1;

    switch (FlashSize1) {
    case 0x20000:
      CommonSpiDeviceSize = FLASH_SIZE_128K;
      break;
    case 0x40000:
      CommonSpiDeviceSize = FLASH_SIZE_256K;
      break;
    case 0x80000:
      CommonSpiDeviceSize = FLASH_SIZE_512K;
      break;
    case 0x100000:
      CommonSpiDeviceSize = FLASH_SIZE_1024K;
      break;
    case 0x200000:
      CommonSpiDeviceSize = FLASH_SIZE_2048K;
      break;
    case 0x400000:
      CommonSpiDeviceSize = FLASH_SIZE_4096K;
      break;
    case 0x800000:
      CommonSpiDeviceSize = FLASH_SIZE_8192K;
      break;
    case 0x1000000:
      CommonSpiDeviceSize = FLASH_SIZE_16384K;
      break;
    }
    //
    // BlockSize is 0x100, so multiply 0x100 here
    //
    FdInfo->BlockMap.Mutiple = (UINT16)(FlashSize1 / (FdInfo->BlockMap.BlockSize * 0x100));
    FdInfo->Size = CommonSpiDeviceSize;
  }
  return Status;
}

//[-start-151124-IB08450330-remove]//
///**
//  Get VSCC values from the Descriptor Region (VSCC Table).
//
//  @param[in,out] Vscc0Value        VSCC0 (Vendor Specific Component Capabilities) Value
//  @param[in,out] Vscc1Value        VSCC1 (Vendor Specific Component Capabilities) Value
//
//  @retval EFI_SUCCESS               Found the VSCC values on Descriptor Region
//  @retval EFI_NOT_FOUND             Couldn't find the VSCC values on Descriptor Region
//  @exception EFI_UNSUPPORTED        ReadDataCmdOpcodeIndex is out of range
//**/
//EFI_STATUS
//EFIAPI
//GetDescriptorVsccValues (
//  IN OUT UINT32                 *Vscc0Value,
//  IN OUT UINT32                 *Vscc1Value
//  )
//{
//  UINT32                        SpiDescFlashUpperMap1;
//  UINT32                        VsccTableBaseAddr;
//  UINT32                        VsccTableLength;
//  UINT32                        JedecIdRegIndex;
//  EFI_STATUS                    Status;
//  UINT32                        FlashDescriptor;
//  BOOLEAN                       MatchedVtbEntryFound;
//  UINT8                         SpiIndex;
//  UINT32                        Data32;
//  UINT16                        NumberComponents;
//  UINT8                         SpiVidDid[3];
//
//  //
//  // Read VSCC Table address in Descriptor Upper Map
//  //
//  Status = SpiTransfer (
//             SPI_HARDWARE,
//             (UINT8 *)(UINTN)R_SPI_FLASH_UMAP1,
//             sizeof (SpiDescFlashUpperMap1) - 1, // make byte count zero-based
//             V_SPI_HSFC_FCYCLE_READ,
//             0,
//             0,
//             0
//             );
//  SpiDescFlashUpperMap1 = MmSpi32 (R_SPI_FDATA00);
//  //
//  // B_SPI_FLASH_UMAP1_VTBA represents address bits [11:4]
//  //
//  VsccTableBaseAddr = ((SpiDescFlashUpperMap1 & B_SPI_FLASH_UMAP1_VTBA) << 4);
//  //
//  // Multiplied by 4? B_SPI_FDBAR_VTL is the 1-based number of DWORDs.
//  //
//  VsccTableLength = (((SpiDescFlashUpperMap1 & B_SPI_FLASH_UMAP1_VTL) >> 8) << 2);
//  if (VsccTableLength < (S_SPI_VTBA_JID0 + S_SPI_VTBA_VSCC0)) {
//    //
//    // Non-existent or invalid Vscc Table
//    //
//    return EFI_NOT_FOUND;
//  }
//
//  NumberComponents = (MmSpi16 (R_SPI_FDOD) & B_SPI_FDBAR_NC) >> N_SPI_FDBAR_NC;
//
//  MmSpi32 (R_SPI_FDATA00) = 0;
//  SpiTransfer (SPI_SOFTWARE, 0, sizeof (SpiVidDid) - 1, SPI_OPCODE_INDEX_READ_ID, 0, 1, 1);
//  Data32 = MmSpi32 (R_SPI_FDATA00);
//  CopyMem (SpiVidDid, &Data32, sizeof (SpiVidDid));
//
//  for (SpiIndex = 0; SpiIndex <= NumberComponents; SpiIndex++) {
//    JedecIdRegIndex = 0;
//    MatchedVtbEntryFound = FALSE;
//    while (JedecIdRegIndex <= (VsccTableLength - SIZE_OF_SPI_VTBA_ENTRY)) {
//      MmSpi32 (R_SPI_FDATA00) = 0;
//      Status = SpiTransfer (
//                 SPI_HARDWARE,
//                 (UINT8 *)(UINTN)(VsccTableBaseAddr + JedecIdRegIndex),
//                 sizeof (UINT32) - 1, // make byte count zero-based
//                 V_SPI_HSFC_FCYCLE_READ,
//                 0,
//                 0,
//                 0
//                 );
//      FlashDescriptor = MmSpi32 (R_SPI_FDATA00);
//      if ((EFI_ERROR (Status)) || (FlashDescriptor == 0xFFFFFFFF)) {
//        break;
//      }
//
//      if (((FlashDescriptor & B_SPI_VTBA_JID0_VID) != SpiVidDid[0]) ||
//         (((FlashDescriptor & B_SPI_VTBA_JID0_DID0) >> N_SPI_VTBA_JID0_DID0) != SpiVidDid[1] ||
//         (((FlashDescriptor & B_SPI_VTBA_JID0_DID1) >> N_SPI_VTBA_JID0_DID1) != SpiVidDid[2]))) {
//        JedecIdRegIndex += SIZE_OF_SPI_VTBA_ENTRY;
//      } else {
//        MatchedVtbEntryFound = TRUE;
//        break;
//      }
//    }
//
//    if (!MatchedVtbEntryFound) {
//      return EFI_NOT_FOUND;
//    }
//
//    MmSpi32 (R_SPI_FDATA00) = 0;
//    Status = SpiTransfer (
//               SPI_HARDWARE,
//               (UINT8 *)(UINTN)(VsccTableBaseAddr + JedecIdRegIndex + R_SPI_VTBA_VSCC0),
//               sizeof (UINT32) - 1,
//               SPI_OPCODE_INDEX_READ_ID,
//               0,
//               1,
//               1
//               );
//    Data32 = MmSpi32 (R_SPI_FDATA00);
//    if ((EFI_ERROR (Status)) || (Data32 == 0xFFFFFFFF)) {
//      return EFI_NOT_FOUND;
//    }
//    //
//    // Copy correct VSCCn value
//    //
//    if (SpiIndex == 0) {
//      *Vscc0Value = Data32;
//    } else {
//      *Vscc1Value = Data32;
//      return EFI_SUCCESS;
//    }
//  }
//
//  return EFI_SUCCESS;
//}
//[-start-151124-IB08450330-remove]//

/**
  Detect and Initialize SPI flash part OpCode and other parameter through PCH

  @param FlashDevice            pointer to FLASH_DEVICE structure

  @retval EFI_SUCCESS           The SPI device was successfully recognized
  @retval EFI_UNSUPPORTED       The flash device is not supported by this function
  @retval EFI_DEVICE_ERROR      Failed to Recoginize the SPI device

**/
EFI_STATUS
EFIAPI
SpiRecognize (
  IN FLASH_DEVICE                       *FlashDevice
  )
{
  UINT32                  BiosAccessState;
  UINT32                  ValidWriteRegionStart;
  UINT32                  ValidWriteRegionEnd;
  UINT32                  ValidReadRegionStart;
  UINT32                  ValidReadRegionEnd;
//[-start-151124-IB08450330-remove]//
  //UINT8                   EraseSizeCode;
//[-end-151124-IB08450330-remove]//
  SPI_CONFIG_BLOCK        *SpiConfigBlock;
//[-start-151124-IB08450330-remove]//
  //UINT32                  Vscc0;
  //UINT32                  Vscc1;
//[-end-151124-IB08450330-remove]//
  UINT8                   SpiFlashNumber;
  EFI_STATUS              Status;
  UINTN                   Index;
  FD_INFO                 *FdInfo;
//[-start-171229-IB15410087-remove]//
//  UINT16                  Reg16;
//  UINT16                  SpiOpType;
//  UINT32                  SpiOpMenu0;
//  UINT32                  SpiOpMenu1;
//[-end-171229-IB15410087-remove]//
//[-start-151124-IB08450330-add]//
  UINT32                  SpiVidDid;
  UINT32                  IfwiRegionEntry;
  UINT32                  DeviceExpansionRegionEntry;
//[-end-151124-IB08450330-add]//

//[-start-171229-IB15410087-remove]//
//  SpiOpType  = 0;
//  SpiOpMenu0 = 0;
//  SpiOpMenu1 = 0;
//[-end-171229-IB15410087-remove]//
//[-start-151124-IB08450330-remove]//
  //Vscc0 = 0;
  //Vscc1 = 0;
//[-end-151124-IB08450330-remove]//
  Index         = 0;
//[-start-151124-IB08450330-remove]//
  //EraseSizeCode = 0;
//[-end-151124-IB08450330-remove]//
//[-start-151124-IB08450330-add]//
  SpiVidDid     = 0;
//[-end-151124-IB08450330-add]//

  SpiConfigBlock = (SPI_CONFIG_BLOCK *)FlashDevice->TypeSpecificInfo;
  FdInfo = &FlashDevice->DeviceInfo;

//[-start-171229-IB15410087-remove]//
//  //
//  // Intel TXEManuf tool access TXE using Software sequence.
//  // Change optype and opmenu to make Txemanuf tool read process success.
//  // Tool check the First two opcode must Read ID and Read opcode,
//  // so change the opcode order for tool, swap the AO1(EraseOp) and AO4(ReadOp).
//  // Original opcode order is:
//  //   (0)ReadIdOp (1)EraseOp (2)WriteOp (3)WriteStatusOp (4)ReadOp (5)ReadStatusOp
//  // Change the opcode order to:
//  //   (0)ReadIdOp (4)ReadOp (2)WriteOp (3)WriteStatusOp (1)EraseOp (5)ReadStatusOp
//  //
//  SpiOpMenu0 = SpiConfigBlock->OpCodeMenu[0] | (SpiConfigBlock->OpCodeMenu[4] << 0x08) |
//               (SpiConfigBlock->OpCodeMenu[2] << 0x10) | (SpiConfigBlock->OpCodeMenu[3] << 0x18);
//  SpiOpMenu1 = SpiConfigBlock->OpCodeMenu[1] | (SpiConfigBlock->OpCodeMenu[5] << 0x08) |
//               (SpiConfigBlock->OpCodeMenu[6] << 0x10) | (SpiConfigBlock->OpCodeMenu[7] << 0x18);
//  SpiOpType = (SpiConfigBlock->OpType & ~(B_SPI_OPTYPE1_MASK | B_SPI_OPTYPE4_MASK)) |
//              (((SpiConfigBlock->OpType & B_SPI_OPTYPE1_MASK) >> (1 * 2)) << (4 * 2)) |
//              (((SpiConfigBlock->OpType & B_SPI_OPTYPE4_MASK) >> (4 * 2)) << (1 * 2));
//  MmSpi32(R_SPI_OPMENU0) = SpiOpMenu0;
//  MmSpi32(R_SPI_OPMENU1) = SpiOpMenu1;
//  MmSpi16(R_SPI_OPTYPE) = SpiOpType;
//  MmSpi16(R_SPI_PREOP) = *(UINT16*)SpiConfigBlock->PrefixMenu;
//  Reg16 = MmSpi16(R_SPI_PREOP);
//[-end-171229-IB15410087-remove]//

//[-start-151124-IB08450330-remove]//
  //Status = GetDescriptorVsccValues (&Vscc0, &Vscc1);
  //if (EFI_ERROR (Status)) {
  //  switch(SpiConfigBlock->BlockEraseSize) {
  //    case 0x100:
  //      EraseSizeCode = 0;
  //      break;
  //    case 0x1000:
  //      EraseSizeCode = 1;
  //      break;
  //    case 0x10000:
  //      EraseSizeCode = 3;
  //      break;
  //  }
  //  Vscc0 = (SpiConfigBlock->OpCodeMenu[SPI_OPCODE_INDEX_ERASE] << 8) |
  //         (SpiConfigBlock->GlobalProtect << 3) |
  //         (SpiConfigBlock->ProgramGranularity << 2) |
  //         (EraseSizeCode);
  //
  //  if (!SpiConfigBlock->NVStatusBit) {
  //     if ((SpiConfigBlock->PrefixMenu[SPI_PREFIX_INDEX_WRITE_EN] == 0x06) ||
  //     	  (SpiConfigBlock->PrefixMenu[SPI_PREFIX_INDEX_WRITE_S_EN] == 0x06)) {
  //       Vscc0 = Vscc0 | (UINT32) (B_SPI_LVSCC_WEWS);
  //     }
  //     else if ((SpiConfigBlock->PrefixMenu[SPI_PREFIX_INDEX_WRITE_EN]== 0x50) ||
  //     	      (SpiConfigBlock->PrefixMenu[SPI_PREFIX_INDEX_WRITE_S_EN]== 0x50)) {
  //       Vscc0 = Vscc0 | (UINT32) (B_SPI_LVSCC_WSR);
  //     }
  //  }
  //}
  ////
  //// The VCL locks itself when set, it will assert because we have no way to update VSCC value
  ////
  //if ((MmSpi32 (R_SPI_LVSCC) & B_SPI_LVSCC_VCL) != 0) {
  //  return EFI_ACCESS_DENIED;
  //}
  //MmSpi32 (R_SPI_LVSCC) = Vscc0 & B_SPI_LVSCC_CAP_MASK;
  //if ((MmSpi16 (R_SPI_FDOD) & B_SPI_FDBAR_NC) >> N_SPI_FDBAR_NC == 0) {
  //  Vscc1 = Vscc0;
  //}
  //MmSpi32 (R_SPI_UVSCC) = Vscc1 & B_SPI_UVSCC_CAP_MASK;
//[-end-151124-IB08450330-remove]//

  //
  // Read device ID
  //
  if (FlashDevice->DeviceInfo.Id != ID_COMMON_SPI_DEVICE) {
//[-start-151124-IB08450330-modify]//
    
    SpiReadId ((UINT8 *)&SpiVidDid);

    if (SpiVidDid != FlashDevice->DeviceInfo.Id)
    {
      return EFI_UNSUPPORTED;
    }
//[-end-151124-IB08450330-modify]//
  }

//[-start-151124-IB08450330-remove]//
//  if (!AtRuntime()) {
//    SpiReadConfiguration (FALSE);
//  }
//[-end-151124-IB08450330-remove]//
  //
  //check is whether mutiple flash parts
  //
  CopyMem (&mSpiConfigBlock, FlashDevice->TypeSpecificInfo, sizeof (SPI_CONFIG_BLOCK));


  Status = GetSpiFlashNumber (&SpiFlashNumber);
  if (!EFI_ERROR (Status)) {
    //
    // Get SPI device number from SPI descriptor success
    //
    Status = UpdateSpiDeviceSize (SpiFlashNumber, FdInfo, SpiConfigBlock);
    ASSERT_EFI_ERROR (Status);
  }


  mValidWriteRegionStart = 0;
  mValidWriteRegionEnd = mSpiConfigBlock.DeviceSize - 1;
  mValidReadRegionStart = 0;
  mValidReadRegionEnd = mSpiConfigBlock.DeviceSize - 1;


//[-start-151124-IB08450330-modify]//

  //
  // Read FLMSTR1 to get BIOS access right
  //
  MmSpi32(R_SPI_FDOC) = 0x00003000;
  BiosAccessState = MmSpi32(R_SPI_FDOD);
    
//[-start-161024-IB07400804-remove]//
//  //
//  // Check FLMSTR1 write access
//  //
//   if (!(BiosAccessState & 0x0f000000))
//   {
//    ValidWriteRegionStart = (MmSpi32(R_SPI_BFPR) & 0x0fff) << 12;
//    ValidWriteRegionEnd = ((MmSpi32(R_SPI_BFPR) >> 16) & 0x0fff) << 12;
//    if (ValidWriteRegionStart < ValidWriteRegionEnd)
//    {
//    	mValidWriteRegionStart = ValidWriteRegionStart;
//      mValidWriteRegionEnd = ValidWriteRegionEnd | 0x0fff;
//    }
//  }
//  //
//  // Setup read regions for SPI read command
//  //
//  ValidReadRegionStart = (MmSpi32(R_SPI_BFPR) & 0x0fff) << 12;
//  ValidReadRegionEnd = ((MmSpi32(R_SPI_BFPR) >> 16) & 0x0fff) << 12;
//  if (ValidReadRegionStart < ValidReadRegionEnd)
//  {
//    mValidReadRegionStart = ValidReadRegionStart;
//    mValidReadRegionEnd = ValidReadRegionEnd | 0x0fff;
//  }
//[-end-161024-IB07400804-remove]//
//[-end-151124-IB08450330-modify]//
//[-start-161021-IB07400801-add]//
  //
  // Ovrride original setting, only check descriptor region now
  //
  if (!(BiosAccessState & (REGION_ACCESS_DESC_REGION << 20))) {
    //
    // DESC region write lock, only support BIOS region for write
    //
    ValidWriteRegionStart = (MmSpi32(R_SPI_BFPR) & 0x0fff) << 12;
    ValidWriteRegionEnd = ((MmSpi32(R_SPI_BFPR) >> 16) & 0x0fff) << 12;
    if (ValidWriteRegionStart < ValidWriteRegionEnd)
    {
    	mValidWriteRegionStart = ValidWriteRegionStart;
      mValidWriteRegionEnd = ValidWriteRegionEnd | 0x0fff;
    }

    //
    // Support DESC + BIOS region for read.
    //
    ValidReadRegionStart = 0;
    ValidReadRegionEnd = ((MmSpi32(R_SPI_BFPR) >> 16) & 0x0fff) << 12;
    if (ValidReadRegionStart < ValidReadRegionEnd)
    {
      mValidReadRegionStart = ValidReadRegionStart;
      mValidReadRegionEnd = ValidReadRegionEnd | 0x0fff;
    }
  }
  if (PcdGetBool(PcdFDOState) == 1) {
    DEBUG ((EFI_D_ERROR, "FDO mode, Override Vaild Region!!\n"));
    mValidWriteRegionStart = 0;
    mValidWriteRegionEnd = mSpiConfigBlock.DeviceSize - 1;
    mValidReadRegionStart = 0;
    mValidReadRegionEnd = mSpiConfigBlock.DeviceSize - 1;
  }
  DEBUG ((EFI_D_ERROR, "mValidWriteRegion = (%08x, %08x)\n", mValidWriteRegionStart, mValidWriteRegionEnd));
  DEBUG ((EFI_D_ERROR, "mValidReadRegion  = (%08x, %08x)\n", mValidReadRegionStart, mValidReadRegionEnd));
//[-end-161021-IB07400801-add]//
//[-start-151124-IB08450330-add]//
  //
  // Read IFWI and Deice Expansion region entries
  //
  Status = SpiTransfer (
             R_SPI_FLREG1,
             sizeof (UINT32),
             V_SPI_HSFS_CYCLE_READ
             );
  if (EFI_ERROR(Status)) {
    IfwiRegionEntry = 0;
  } else {
    IfwiRegionEntry = MmSpi32 (R_SPI_FDATA00);
    if ((IfwiRegionEntry & B_SPI_FREG0_BASE_MASK) == V_SPI_FLREG_DISABLED) {
      IfwiRegionEntry = 0;
    } 
  }

  Status = SpiTransfer (
             R_SPI_FLREG5,
             sizeof (UINT32),
             V_SPI_HSFS_CYCLE_READ
             );
  if (EFI_ERROR(Status)) {
    DeviceExpansionRegionEntry = 0;
  } else {
    DeviceExpansionRegionEntry = MmSpi32 (R_SPI_FDATA00);
    if ((DeviceExpansionRegionEntry & B_SPI_FREG0_BASE_MASK) == V_SPI_FLREG_DISABLED) {
      DeviceExpansionRegionEntry = 0;
    } 
  }
  
  mSpiMmioBeginOffset = (((DeviceExpansionRegionEntry & B_SPI_FREG5_BASE_MASK) << N_SPI_FREG1_BASE) + ((IfwiRegionEntry & B_SPI_FREG1_BASE_MASK) << N_SPI_FREG1_BASE)) >> 1; 

  ASSERT (mSpiMmioBeginOffset < mSpiConfigBlock.DeviceSize);
  ASSERT (mSpiMmioBeginOffset + mSpiMmioSize <= mSpiConfigBlock.DeviceSize);
//[-end-151124-IB08450330-add]//

  return EFI_SUCCESS;
}


//[-start-151124-IB08450330-add]//
/**
  Convery SPI ROM memory mapping IO address to offset for SPI transfer.

  @param MemoryAddress          BIOS memory mapped address
  @param Offset                 Corresponding offset for SPI transfer.

  @retval EFI_SUCCESS           Function successfully returned
  @retval EFI_UNSUPPORTED       Memory address is invalid.

**/
EFI_STATUS
SPIMmioAddressToOffset (
  IN UINTN       MemoryAddress,
  OUT UINT32     *Offset
  )
{
//[-start-151229-IB06740454-remove]//
////[-start-151216-IB07220025-add]//
//  if (MemoryAddress >= ((UINTN)((UINT32)0 - mSpiConfigBlock.DeviceSize))) {
//    *Offset = (UINT32)(MemoryAddress - ((UINTN)((UINT32)0 - mSpiConfigBlock.DeviceSize)));
//    return EFI_SUCCESS;
//  }
////[-end-151216-IB07220025-add]//
//[-end-151229-IB06740454-remove]//
  if ((mSpiMmioSize == 0) || (mSpiMmioBeginMemoryAddress == 0) || (MemoryAddress < mSpiMmioBeginMemoryAddress)) {
    *Offset = 0;
    return EFI_UNSUPPORTED;     
  }
  
  *Offset = mSpiMmioBeginOffset + (UINT32)(MemoryAddress - mSpiMmioBeginMemoryAddress);
  return EFI_SUCCESS;
}
//[-end-151124-IB08450330-add]//

/**
  Erase the SPI flash device from LbaWriteAddress through PCH

  @param FlashDevice            pointer to FLASH_DEVICE structure
  @param FlashAddress           Target address to be erased
  @param Size                   The size in bytes to be erased

  @retval EFI_SUCCESS           The SPI device was successfully recognized
  @retval EFI_UNSUPPORTED       The flash device is not supported by this function
  @retval EFI_DEVICE_ERROR      Failed to erase the target address

**/
EFI_STATUS
EFIAPI
SpiErase(
  IN  FLASH_DEVICE              *FlashDevice,
  IN  UINTN                     FlashAddress,
  IN  UINTN                     Size
  )
{
//[-start-151124-IB08450330-modify]//
  EFI_STATUS    Status;
  UINTN         NumBytes;
  UINT32        SpiOffset;
  UINT8         SpiEraseCycle;
//[-end-151124-IB08450330-modify]//

  NumBytes = Size;

//[-start-151124-IB08450330-remove]//
//  if (GetFlashMode () == FW_DEFAULT_MODE) {
//    Status = MemoryToBiosRegionAddress (FlashAddress, &SpiAddress);
//    if (Status == EFI_SUCCESS) {
//      FlashAddress = SpiAddress;
//    }
//  }
//[-end-151124-IB08450330-remove]//
  //
  // Make sure the flash address alignment on sector/block
  //
  FlashAddress &= ~(mSpiConfigBlock.BlockEraseSize - 1);
//[-start-151124-IB08450330-add]//
  //
  // Determine SpiEraseCycle code
  //
  switch (mSpiConfigBlock.BlockEraseSize) {
  
  case 0x1000:
    SpiEraseCycle = V_SPI_HSFS_CYCLE_4K_ERASE;
    break;
    
  case 0x10000:
    SpiEraseCycle = V_SPI_HSFS_CYCLE_64K_ERASE;
    break;
    
  default:
    SpiEraseCycle = 0xff;
  }
  if (SpiEraseCycle == 0xff) {
    return EFI_DEVICE_ERROR;
  }
//[-end-151124-IB08450330-add]//

//[-start-151124-IB08450330-modify]//
  //
  // Calculate device offset
  //
  if (GetFlashMode () == FW_DEFAULT_MODE) {
    Status = SPIMmioAddressToOffset (FlashAddress, &SpiOffset);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  } else {
    SpiOffset = (UINT32)FlashAddress;
  }

  SpiFlashLock(SpiOffset, Size, SPI_FULL_ACCESS);
  while ((INTN)NumBytes > 0)
  {
    if (CheckValidRegion(SpiOffset, mSpiConfigBlock.BlockEraseSize, TRUE))
    {
      Status = SpiTransfer(
                 SpiOffset,
                 1,
                 SpiEraseCycle
                 );
      if (EFI_ERROR(Status)) {
        return Status;
      }
    }
    SpiOffset  += mSpiConfigBlock.BlockEraseSize;
    NumBytes   -= mSpiConfigBlock.BlockEraseSize;
  }
  SpiFlashLock(SpiOffset, Size, SPI_WRITE_LOCK);
//[-end-151124-IB08450330-modify]//

  return EFI_SUCCESS;
}

//[-start-151124-IB08450330-add]//

/**
  Read or write to SPI ROM

  @param FlashDevice            pointer to FLASH_DEVICE structure
  @param FlashAddress           Target address to be access
  @param SrcAddress             Data pointer that hold source data or receive data
  @param BufferSize             The size in bytes to be accessed
  @param WriteOperation         TRUE: Write opreation
                                FALSE: Read opreation

  @retval EFI_SUCCESS           The SPI device was successfully recognized
  @retval EFI_UNSUPPORTED       The flash device is not supported by this function
  @retval EFI_DEVICE_ERROR      Failed to erase the target address

**/
EFI_STATUS
SpiDoReadWrite (
  IN  FLASH_DEVICE              *FlashDevice,
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINT32                    BufferSize,
  IN  BOOLEAN                   WriteOperation 
)
{
  EFI_STATUS           Status;
  UINT32               RemainSize;
  UINT32               SpiOffset;
  UINT8                *CurrentAddress;
  UINT8                NumBytesThisTransfer;
//[-start-151229-IB06740454-add]//
  UINT8                AlignedNumBytesThisTransfer;
//[-end-151229-IB06740454-add]//

  Status = EFI_SUCCESS;
  NumBytesThisTransfer = 0;

  //
  // Calculate device offset
  //
  if (GetFlashMode () == FW_DEFAULT_MODE) {
    Status = SPIMmioAddressToOffset ((UINTN)FlashAddress, &SpiOffset);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  } else {
    SpiOffset = (UINT32)(UINTN)FlashAddress;
  }

  if (!CheckValidRegion(SpiOffset, BufferSize, WriteOperation)) {
    return EFI_UNSUPPORTED;
  }

  if (WriteOperation) {
    SpiFlashLock(SpiOffset, BufferSize, SPI_FULL_ACCESS);
  }
  for (RemainSize = BufferSize, CurrentAddress = SrcAddress; 
      RemainSize != 0; 
      RemainSize -= NumBytesThisTransfer, CurrentAddress += NumBytesThisTransfer, SpiOffset += NumBytesThisTransfer) {

    if (RemainSize > MAXIMUM_SPI_TRANSFER_SIZE) {
      NumBytesThisTransfer = MAXIMUM_SPI_TRANSFER_SIZE; 
    } else {
      NumBytesThisTransfer = (UINT8)RemainSize;
    }
    
    if (((SpiOffset + NumBytesThisTransfer) & (MAXIMUM_SPI_TRANSFER_SIZE - 1)) != 0) {
//[-start-151229-IB06740454-modify]//
      AlignedNumBytesThisTransfer = (UINT8)(((SpiOffset + NumBytesThisTransfer) & (~(MAXIMUM_SPI_TRANSFER_SIZE - 1))) - SpiOffset);
      //
      // If caller provided final address is not aligned, in last transfer
      // AlignedNumBytesThisTransfer will becomes 0. We do not adjust this
      // transfer. 
      //         
//[-start-160511-IB11270156-modify]//
      if ((AlignedNumBytesThisTransfer != 0) && (AlignedNumBytesThisTransfer < MAXIMUM_SPI_TRANSFER_SIZE))
//[-end-160511-IB11270156-modify]//
      {
        NumBytesThisTransfer = AlignedNumBytesThisTransfer;
      }
//[-end-151229-IB06740454-modify]//
    }

    if (WriteOperation) {
      SpiCopyTransferData (
        CurrentAddress,
        NumBytesThisTransfer,
        TRUE
        );
    }      
    Status = SpiTransfer(
      SpiOffset,
      NumBytesThisTransfer,
      WriteOperation ? V_SPI_HSFS_CYCLE_WRITE : V_SPI_HSFS_CYCLE_READ
      );
    if (EFI_ERROR(Status)) {
      goto FinalClean;
    }
    if (!WriteOperation) {
      SpiCopyTransferData (
        CurrentAddress,
        NumBytesThisTransfer,
        FALSE
        );
    }      
  }

FinalClean:
  if (WriteOperation) {
    SpiFlashLock(SpiOffset, BufferSize, SPI_WRITE_LOCK);
  }
  return Status;  

}
//[-end-151124-IB08450330-add]//

/**
  Write the SPI flash device with given address and size through PCH

  @param FlashDevice            pointer to FLASH_DEVICE structure
  @param FlashAddress           Destination Offset
  @param SrcAddress             Source Offset
  @param SPIBufferSize          The size for programming
  @param LbaWriteAddress        Write Address

  @retval EFI_SUCCESS           The SPI device was successfully recognized
  @retval EFI_UNSUPPORTED       The flash device is not supported by this function
  @retval EFI_DEVICE_ERROR      Failed to erase the target address

**/
EFI_STATUS
SpiProgram (
  IN  FLASH_DEVICE              *FlashDevice,
  IN  UINT8                     *FlashAddress,
  IN  UINT8                     *SrcAddress,
  IN  UINTN                     *SPIBufferSize,
  IN  UINTN                     LbaWriteAddress
  )
{
//[-start-151124-IB08450330-modify]//
  //
  // FlashAddress should equals to LbaWriteAddress. useless. 
  //  
  LbaWriteAddress = LbaWriteAddress;

  return SpiDoReadWrite (FlashDevice, FlashAddress, SrcAddress, (UINT32)*SPIBufferSize, TRUE);
//[-end-151124-IB08450330-modify]//
}

/**
  Read the SPI flash device with given address and size through PCH

  @param FlashDevice            pointer to FLASH_DEVICE structure
  @param DstAddress             Destination buffer address
  @param FlashAddress           The flash device address to be read
  @param BufferSize             The size to be read

  @retval EFI_SUCCESS           The SPI device was successfully recognized
  @retval EFI_UNSUPPORTED       The flash device is not supported by this function
  @retval EFI_DEVICE_ERROR      Failed to erase the target address

**/
EFI_STATUS
SpiRead (
  IN  FLASH_DEVICE              *FlashDevice,
  IN  UINT8                     *DstAddress,
  IN  UINT8                     *FlashAddress,
  IN  UINTN                     BufferSize
  )
{
//[-start-151124-IB08450330-modify]//
  return SpiDoReadWrite (FlashDevice, FlashAddress, DstAddress, (UINT32)BufferSize, FALSE);
//[-end-151124-IB08450330-modify]//
}

/**
  Check whether the flash region is used or not

  @param FlashRegion            Flash Region x Register (x = 0 - 3)

  @retval TRUE                  The region is used
  @retval FALSE                 The region is not used

**/
BOOLEAN
CheckFlashRegionIsValid (
  IN       UINT32    FlashRegion
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

/**
  Get flash table from platform

  @param DataBuffer             IN: the input buffer address
                                OUT:the flash region table from rom file

  @retval EFI_SUCCESS           Function successfully returned

**/
EFI_STATUS
GetSpiPlatformFlashTable (
  IN OUT   UINT8    *DataBuffer
  )
{

  UINT8           Index;
  UINT32          FlashRegionReg;
  UINT32          FlashSize;
  FLASH_REGION    *FlashTable;
  UINT32          ReadAccess;
  UINT32          WriteAccess;
  UINT32          Frap;
  BOOLEAN         DescriptorValid;

  DescriptorValid = FALSE;

  FlashTable = (FLASH_REGION *)DataBuffer;

  if ((MmSpi32(R_SPI_HSFS) & B_SPI_HSFS_FDV) == B_SPI_HSFS_FDV) {
    DescriptorValid = TRUE;
  }
  //
  // Check whether descriptor is valid
  //
  if (DescriptorValid) {
    Frap = MmSpi32(R_SPI_FRAP);

//[-start-160810-IB11270161-modify]//
    for (Index = APL_DescRegionType; Index < APL_MaxFlashRegionType; Index++) {


      FlashRegionReg = MmSpi32(R_SPI_FREG0_FLASHD + Index * 4);

      if (CheckFlashRegionIsValid (FlashRegionReg)){
        //
        // Skip region if FlashRegionReg is empty except DescRegionType
        //
        if ((Index != APL_DescRegionType) && (!FlashRegionReg)) {
          continue;
        }
        FlashTable->Type = Index;
        FlashTable->Offset = (FlashRegionReg & 0x7fff) << 12;
        FlashSize = ((FlashRegionReg >> 16 & 0x7fff) - (FlashRegionReg & 0x7fff) + 1) << 12;
        FlashTable->Size = FlashSize;
        //
        // We can override  BIOS region, ME region, and GBE regiron the permissions
        // in the Flash Descriptor through BMWAG and BMRAG.
        //
        if (Index >= APL_BiosRegionType && Index <= APL_GbeRegionType) {
          ReadAccess = (Frap >> (BIOS_REGION_READ_ACCESS + Index)) & ACCESS_AVAILABLE;
          ReadAccess |= ((Frap >> (BIOS_MASTER_READ_ACCESS_GRANT + Index)) & ACCESS_AVAILABLE);
          WriteAccess = (Frap >> (BIOS_REGION_WRITE_ACCESS + Index)) & ACCESS_AVAILABLE;
          WriteAccess |= ((Frap >> (BIOS_MASTER_WRITE_ACCESS_GRANT + Index)) & ACCESS_AVAILABLE);
        } else {
          ReadAccess = (Frap >> (BIOS_REGION_READ_ACCESS + Index)) & ACCESS_AVAILABLE;
          WriteAccess = (Frap >> (BIOS_REGION_WRITE_ACCESS + Index)) & ACCESS_AVAILABLE;
        }
        if (ReadAccess == ACCESS_AVAILABLE && WriteAccess == ACCESS_AVAILABLE) {
          FlashTable->Access = ACCESS_AVAILABLE;
        } else {
          FlashTable->Access = ACCESS_NOT_AVAILABLE;
        }

        FlashTable++;
      }
    }
  }


  FlashTable->Type = APL_EndOfRegionType;
//[-end-160809-IB11270161-modify]//
  return EFI_SUCCESS;
}



/**
  SpiAccessLib Library Class Constructor

  @retval EFI_SUCCESS           Module initialized successfully
  @retval Others                Module initialization failed

**/
EFI_STATUS
EFIAPI
SpiAccessLibInit (
  VOID
  )
{
//[-start-151124-IB08450330-add]//
  UINT32 BiosFlashBase;
  UINT32 BiosFlashLimit;
  UINT32 DeviceExpansionFlashBase;
//[-end-151124-IB08450330-add]//
//[-start-170215-IB07400840-add]//
  UINT32 DeviceExpansionFlashLimit;
  UINT32 SpiRomSize;
//[-end-170215-IB07400840-add]//
//[-start-181001-IB07401020-add]//
  UINT32 PdrFlashBase;
  UINT32 PdrFlashLimit;
//[-end-181001-IB07401020-add]//

//[-start-151124-IB08450330-add]//
  //
  // Enable memory mapping IO
  //
  MmioOr32 (
          MmPciAddress (
              0,
              DEFAULT_PCI_BUS_NUMBER_SC,
              PCI_DEVICE_NUMBER_SPI,
              PCI_FUNCTION_NUMBER_SPI,
              PCI_COMMAND_OFFSET
              ),
          EFI_PCI_COMMAND_MEMORY_SPACE
          );
//[-end-151124-IB08450330-add]//
  //
  // Setup the PCH_SPI base address and Flash base address for both of PostTime and RunTime phase
  //
  mSpiBase = (UINT8*)((UINTN)MmioRead32 (
                              MmPciAddress (0,
                                DEFAULT_PCI_BUS_NUMBER_SC,
                                PCI_DEVICE_NUMBER_SPI,
                                PCI_FUNCTION_NUMBER_SPI,
                                R_SPI_BASE
                              )
                            ) & B_SPI_BASE_BAR);
//[-start-151124-IB08450330-remove]//
  //mFlashBase   = (VOID *)(UINTN)PcdGet32(PcdFlashAreaBaseAddress);
//[-end-151124-IB08450330-remove]//
//[-start-151124-IB08450330-add]//
  BiosFlashBase = MmSpi32(R_SPI_FREG1_BIOS);
  if ((BiosFlashBase & B_SPI_FREG0_BASE_MASK) == V_SPI_FLREG_DISABLED) {
    BiosFlashLimit = 0;
    BiosFlashBase = 0;
  } else {
    BiosFlashLimit = (BiosFlashBase & B_SPI_FREG0_LIMIT_MASK) >> N_SPI_FREG0_LIMIT; 
    BiosFlashBase = (BiosFlashBase & B_SPI_FREG0_BASE_MASK) << N_SPI_FREG0_BASE;
//[-start-170214-IB07400840-add]//
    if (PcdGet32(PcdFlashIfwiRegionBase) == 0) {
      PcdSet32 (PcdFlashIfwiRegionBase, BiosFlashBase);
      PcdSet32 (PcdFlashIfwiRegionSize, BiosFlashLimit + 0x1000 - BiosFlashBase);
      DEBUG ((EFI_D_ERROR, "PcdFlashIfwiRegionBase = %x\n", PcdGet32(PcdFlashIfwiRegionBase)));
      DEBUG ((EFI_D_ERROR, "PcdFlashIfwiRegionSize = %x\n", PcdGet32(PcdFlashIfwiRegionSize)));
    }
//[-end-170214-IB07400840-add]//
  }
  DeviceExpansionFlashBase = MmSpi32(R_SPI_FREG5_DEVICE_EXPANSION_1); 
  if ((DeviceExpansionFlashBase & B_SPI_FREG0_BASE_MASK) == V_SPI_FLREG_DISABLED) {
    DeviceExpansionFlashBase = 0;
  } else {
//[-start-170215-IB07400840-modify]//
    DeviceExpansionFlashLimit = (DeviceExpansionFlashBase & B_SPI_FREG0_LIMIT_MASK) >> N_SPI_FREG0_LIMIT;
    DeviceExpansionFlashBase = (DeviceExpansionFlashBase & B_SPI_FREG0_BASE_MASK) << N_SPI_FREG0_BASE;
    if (PcdGet32(PcdFlashDeviceExpansionBase) == 0) {
      PcdSet32 (PcdFlashDeviceExpansionBase, DeviceExpansionFlashBase);
      PcdSet32 (PcdFlashDeviceExpansionSize, DeviceExpansionFlashLimit + 0x1000 - DeviceExpansionFlashBase);
      DEBUG ((EFI_D_ERROR, "PcdFlashDeviceExpansionBase = %x\n", PcdGet32(PcdFlashDeviceExpansionBase)));
      DEBUG ((EFI_D_ERROR, "PcdFlashDeviceExpansionSize = %x\n", PcdGet32(PcdFlashDeviceExpansionSize)));
    }
//[-end-170215-IB07400840-modify]//
  }
//[-start-181001-IB07401020-add]//
  PdrFlashBase = MmSpi32(R_SPI_FREG4_PLATFORM_DATA); 
  if ((PdrFlashBase & B_SPI_FREG0_BASE_MASK) == V_SPI_FLREG_DISABLED) {
    PdrFlashBase = 0;
  } else {
    PdrFlashLimit = (PdrFlashBase & B_SPI_FREG0_LIMIT_MASK) >> N_SPI_FREG0_LIMIT;
    PdrFlashBase  = (PdrFlashBase & B_SPI_FREG0_BASE_MASK) << N_SPI_FREG0_BASE;
    if ((PcdGet32(PcdFlashPdrRegionBase) == 0)) {
      PcdSet32 (PcdFlashPdrRegionBase, PdrFlashBase);
      PcdSet32 (PcdFlashPdrRegionSize, PdrFlashLimit + 0x1000 - PdrFlashBase);
      DEBUG ((EFI_D_INFO, "PcdFlashPdrRegionBase = %x\n", PcdGet32(PcdFlashPdrRegionBase)));
      DEBUG ((EFI_D_INFO, "PcdFlashPdrRegionSize = %x\n", PcdGet32(PcdFlashPdrRegionSize)));
    }
  }
//[-end-181001-IB07401020-add]//
//[-start-170214-IB07400840-add]//
  if ((PcdGet32(PcdFlashIfwiRegionBase) != 0) && 
      (PcdGet32(PcdFlashDeviceExpansionBase) != 0) &&
      (PcdGet32(PcdFlashBootPartition2Base) == 0)) {
    PcdSet32 (PcdFlashBootPartition2Base, (UINT32)(RShiftU64(((UINT64)PcdGet32(PcdFlashIfwiRegionBase) + (UINT64)PcdGet32(PcdFlashDeviceExpansionBase)),1)));
    DEBUG ((EFI_D_ERROR, "PcdFlashBootPartition2Base = %x\n", PcdGet32(PcdFlashBootPartition2Base)));
  }
  if (PcdGet32(PcdFlashSpiRomSize) == 0) {
//[-start-181001-IB07401020-modify]//
    SpiRomSize = 0x1000; // Descriptor region
    SpiRomSize += PcdGet32(PcdFlashIfwiRegionSize); // IFWI Region
    SpiRomSize += PcdGet32(PcdFlashDeviceExpansionSize); // Device Expansion Region
    SpiRomSize += PcdGet32(PcdFlashPdrRegionSize); // PDR Region
    SpiRomSize += 0x1000; // Reserved Size
    if (SpiRomSize % 0x800000 == 0) { // 8MB, 16MB, 32MB, 64MB
      PcdSet32(PcdFlashSpiRomSize, SpiRomSize);
      DEBUG ((EFI_D_INFO, "PcdFlashSpiRomSize = %x\n\n\n", PcdGet32(PcdFlashSpiRomSize)));
      DEBUG ((EFI_D_INFO, "########################### <---- (0x%08x,0xFFFFFFFF)\n", PcdGet32(PcdFlashSpiRomSize)));
      if (PcdGet32(PcdFlashPdrRegionSize) != 0) {
      DEBUG ((EFI_D_INFO, "#       PDR Region        # Size:0x%08x\n", PcdGet32(PcdFlashPdrRegionSize)));
      DEBUG ((EFI_D_INFO, "########################### <---- (0x%08x,0x%08x)\n", PcdGet32(PcdFlashPdrRegionBase), (UINT32)((UINT64)0x100000000 - PcdGet32(PcdFlashPdrRegionSize))));
      }
      DEBUG ((EFI_D_INFO, "# Device Expansion Region # Size:0x%08x\n", PcdGet32(PcdFlashDeviceExpansionSize)));
      DEBUG ((EFI_D_INFO, "########################### <---- (0x%08x,0x%08x)\n", PcdGet32(PcdFlashDeviceExpansionBase), (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashDeviceExpansionBase)))));
      DEBUG ((EFI_D_INFO, "#       IFWI Region       # Size:0x%08x\n", PcdGet32(PcdFlashIfwiRegionSize)));
      DEBUG ((EFI_D_INFO, "########################### <---- (0x%08x,0x%08x)\n", PcdGet32(PcdFlashIfwiRegionBase), (UINT32)((UINT64)0x100000000 - (PcdGet32(PcdFlashSpiRomSize) - PcdGet32(PcdFlashIfwiRegionBase)))));
      DEBUG ((EFI_D_INFO, "#       DESC Region       # Size:0x1000\n"));
      DEBUG ((EFI_D_INFO, "########################### <---- (0x00000000,0x%08x)\n\n\n", (UINT32)((UINT64)0x100000000 - PcdGet32(PcdFlashSpiRomSize))));
    } else {
      SpiRomSize = ((SpiRomSize / 0x800000) + 1) * 0x800000;
      PcdSet32(PcdFlashSpiRomSize, SpiRomSize);
      DEBUG ((EFI_D_INFO, "Unkown SPI Layout, assign SPI ROM size: 0x%08x", SpiRomSize));
    }
//[-end-181001-IB07401020-modify]//
  }
//[-end-170214-IB07400840-add]//
  mSpiMmioSize =BiosFlashLimit + 0x1000 - ((DeviceExpansionFlashBase + BiosFlashBase) >> 1); 
  mSpiMmioBeginMemoryAddress = (UINTN)((UINT32)0 - mSpiMmioSize);
//[-end-151124-IB08450330-add]//
  SpiAccessInit();
//[-start-151124-IB08450330-remove]//
  //IrsiAddVirtualPointer((VOID **)&mFlashBase);
//[-end-151124-IB08450330-remove]//
  IrsiAddVirtualPointer((VOID **)&mSpiBase);

//  PHYSICAL_ADDRESS					MemoryAddress;
//	UINT32							   Address;
//	VOID * Buffer;
//
//  Address = PcdGet32(PcdFlashNvStorageVariableBase);
//  
//  Buffer =	AllocateAlignedRuntimePages(0x80, 0x10000);
//  CopyMem(Buffer, (VOID*)(UINTN)Address, 0x80000);
//  MemoryAddress = (PHYSICAL_ADDRESS)(UINTN)Buffer;
//  InitNVM(Buffer, 0x80000);
//  mBufferAddress = (UINTN)MemoryAddress;
//  PcdSet32(PcdFlashNvStorageVariableBase, (UINT32)(UINTN)Buffer);
//  DEBUG((EFI_D_INFO, "LibFvbFlashDeviceMemoryMap %x\n", MemoryAddress));
//  DumpBuffer((UINT8*)(UINTN)MemoryAddress);
//  PcdSet32(PcdFlashNvStorageFtwWorkingBase, (UINT32)(UINTN)Buffer + PcdGet32(PcdFlashNvStorageVariableSize));
//  PcdSet32(PcdFlashNvStorageFtwSpareBase, (UINT32)(UINTN)Buffer + PcdGet32(PcdFlashNvStorageVariableSize) + PcdGet32(PcdFlashNvStorageFtwWorkingSize));
//  DumpBuffer((VOID*)(UINTN)PcdGet32(PcdFlashNvStorageFtwWorkingBase));
//  DumpBuffer((VOID*)(UINTN)PcdGet32(PcdFlashNvStorageFtwSpareBase));
//  InitEvent();

  return EFI_SUCCESS;
}

/**
  If a  driver exits with an error, it must call this routine
  to free the allocated resource before the exiting.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval  EFI_SUCCESS      The Driver Lib shutdown successfully.
**/
EFI_STATUS
EFIAPI
SpiAccessLibDestruct (
  VOID
  )
{
  SpiAccessDestroy ();
//[-start-151124-IB08450330-remove]//
  //IrsiRemoveVirtualPointer ((VOID **)&mFlashBase);
//[-end-151124-IB08450330-remove]//
  IrsiRemoveVirtualPointer ((VOID **)&mSpiBase);
  return EFI_SUCCESS;
}
