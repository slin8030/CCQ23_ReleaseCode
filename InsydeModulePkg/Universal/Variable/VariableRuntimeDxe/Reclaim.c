/** @file
  Handles non-volatile variable store garbage collection, using FTW
  (Fault Tolerant Write Lite) protocol.

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "Reclaim.h"

/**
  Gets LBA of block and offset by given address.

  This function gets the Logical Block Address (LBA) of a firmware
  volume block containing the given address, and the offset of the
  address on the block.

  @param  Address        Address which should be contained
                         by returned FVB handle.
  @param  Lba            Pointer to LBA for output.
  @param  Offset         Pointer to offset for output.

  @retval EFI_SUCCESS    LBA and offset successfully returned.
  @retval EFI_NOT_FOUND  Fail to find FVB handle by address.
  @retval EFI_ABORTED    Fail to find valid LBA and offset.

**/
EFI_STATUS
GetVariableStoreInfo (
  IN  EFI_PHYSICAL_ADDRESS   VariableBase,
  OUT EFI_PHYSICAL_ADDRESS   *FvbBaseAddress,
  OUT EFI_LBA                *Lba,
  OUT UINTN                  *Offset
  )
{
  EFI_STATUS                          Status;
  EFI_HANDLE                          FvbHandle;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *Fvb;
  EFI_FIRMWARE_VOLUME_HEADER          *FwVolHeader;
  EFI_FV_BLOCK_MAP_ENTRY              *FvbMapEntry;
  UINT32                              LbaIndex;


  *Lba    = (EFI_LBA) (-1);
  *Offset = 0;

  //
  // Get the proper FVB
  //
  Status = GetFvbInfoByAddress (VariableBase, &FvbHandle, &Fvb);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Get the Base Address of FV
  //
  Status = Fvb->GetPhysicalAddress (Fvb, FvbBaseAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FwVolHeader = (EFI_FIRMWARE_VOLUME_HEADER *) ((UINTN) *FvbBaseAddress);

  //
  // Get the (LBA, Offset) of Address
  //
  if ((VariableBase >= *FvbBaseAddress) && (VariableBase <= (*FvbBaseAddress + FwVolHeader->FvLength))) {
    if ((FwVolHeader->FvLength) > (FwVolHeader->HeaderLength)) {
      //
      // BUGBUG: Assume one FV has one type of BlockLength
      //
      FvbMapEntry = &FwVolHeader->BlockMap[0];
      for (LbaIndex = 1; LbaIndex <= FvbMapEntry->NumBlocks; LbaIndex += 1) {
        if (VariableBase < (*FvbBaseAddress + FvbMapEntry->Length * LbaIndex)) {
          //
          // Found the (Lba, Offset)
          //
          *Lba    = LbaIndex - 1;
          *Offset = (UINTN) (VariableBase - (*FvbBaseAddress + FvbMapEntry->Length * (LbaIndex - 1)));
          return EFI_SUCCESS;
        }
      }
    }
  }

  return EFI_ABORTED;
}


/**
  Writes a buffer to variable storage space, in the working block.

  This function writes a buffer to variable storage space into a firmware
  volume block device. The destination is specified by the parameter
  VariableBase. Fault Tolerant Write protocol is used for writing.

  @param  VariableBase   Base address of the variable to write.
  @param  Buffer         Point to the data buffer.

  @retval EFI_SUCCESS    The function completed successfully.
  @retval EFI_NOT_FOUND  Fail to locate Fault Tolerant Write protocol.
  @retval EFI_ABORTED    The function could not complete successfully.

**/
EFI_STATUS
FtwVariableSpace (
  IN EFI_PHYSICAL_ADDRESS   VariableBase,
  IN UINT8                  *Buffer
  )
{
  EFI_STATUS            Status;
  EFI_FTW_LITE_PROTOCOL *FtwLiteProtocol;
  EFI_LBA               VarLba;
  UINTN                 VarOffset;
  UINT8                 *FtwBuffer;
  UINTN                 FtwBufferSize;
  EFI_SMM_FTW_PROTOCOL  *SmmFtwProtocol;


  FtwBufferSize = GetNonVolatileVariableStoreSize ();
  FtwBuffer     = Buffer;
  if (mSmst == NULL) {
    //
    // Prepare for the variable data
    //
    FtwLiteProtocol = mVariableReclaimInfo->FtwLiteProtocol;
    VarOffset = mVariableReclaimInfo->VariableOffset;
    VarLba = mVariableReclaimInfo->VariableLba;
    //
    // FTW write record
    //
    Status = FtwLiteProtocol->Write (
                                FtwLiteProtocol,
                                NULL,
                                VarLba,         // LBA
                                VarOffset,      // Offset
                                &FtwBufferSize, // NumBytes,
                                FtwBuffer
                                );

  } else {
    SmmFtwProtocol = NULL;
    if (mSmmVariableGlobal->SmmRT != NULL) {
      Status = mSmmVariableGlobal->SmmRT->LocateProtocol (
                                            &gEfiSmmFtwProtocolGuid,
                                            NULL,
                                            (VOID **)&SmmFtwProtocol
                                            );
    }
    if (SmmFtwProtocol == NULL) {
      Status = mSmst->SmmLocateProtocol (
                        &gEfiSmmFtwProtocolGuid,
                        NULL,
                        (VOID **)&SmmFtwProtocol
                        );
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
    //
    // FTW write record
    //
    Status = SmmFtwProtocol->Write(
                              SmmFtwProtocol,
                              mSmmVariableGlobal->VariableStoreInfo.FvbBaseAddress,
                              mSmmVariableGlobal->VariableStoreInfo.Lba,
                              mSmmVariableGlobal->VariableStoreInfo.Offset,
                              &FtwBufferSize,
                              FtwBuffer
                              );
  }
  return Status;
}
