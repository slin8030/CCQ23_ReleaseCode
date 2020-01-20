/** @file
  Functions for Dmitool

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "PnpSmm.h"
#include <Protocol/LegacyBios.h>
#include <Protocol/SmmCpu.h>
#include <Library/FlashRegionLib.h>

SMBIOS_TABLE_ENTRY_POINT      *mSmbiosTableEntryPoint = NULL;
SMBIOS_TABLE_3_0_ENTRY_POINT  *mSmbiosTableEntryPoint64Bit = NULL;
SMBIOS_STRUCTURE              *mSmbiosTable = NULL;
SMBIOS_STRUCTURE              *mSmbiosTable64Bit = NULL;
INT32                         TotalIncreasedStringLength = 0;
EFI_PHYSICAL_ADDRESS          mSMBIOSTableEntryAddress = 0;
EFI_PHYSICAL_ADDRESS          mSMBIOSTableEntryAddress64Bit = 0;
EFI_SMM_CPU_PROTOCOL          *mSmmCpu = NULL;
UINTN                         mCpuIndex =0;
CHAR8                         mSmbio30Signature[] = SMBIOS_30_SIGNATURE;

PNP_REDIRECTED_ENTRY_POINT mPnpDmiFunctions[] = {
  Pnp0x50,
  Pnp0x51,
  Pnp0x52,
  UnsupportedPnpFunction, //Pnp0x53,
  Pnp0x54,
  Pnp0x55,
  Pnp0x56,
  Pnp0x57
};

STATIC
EFI_STATUS
ClearBiosEventLog (
  IN UINTN                              GPNVBase,
  IN UINTN                              GPNVLength
  );

/**
  This fucntion uses to read saved CPU double word register by CPU index

  @param[In]   RegisterNum   Register number which want to get
  @param[In]   CpuIndex      CPU index number to get register.
  @param[Out]  RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Read double word register successfully
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
GetDwordRegisterByCpuIndex (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINTN                             CpuIndex,
  OUT UINT32                            *RegisterData
  )
{
  UINTN                       Width;

  Width = sizeof (UINT32);
  *RegisterData = 0;

  if (RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_CS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_DS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_ES ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_SS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_FS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_GS) {
    Width = sizeof (UINT16);
  }
  return mSmmCpu->ReadSaveState (
                    mSmmCpu,
                    Width,
                    RegisterNum,
                    CpuIndex,
                    RegisterData
                    );
}

/**
  This fucntion uses to set saved CPU double word register by CPU index

  @param[In]  RegisterNum   Register number which want to get
  @param[In]  CpuIndex      CPU index number to get register.
  @param[In]  RegisterData  pointer to output register data

  @retval EFI_SUCCESS   Set double word register successfully
  @return Other         Any error occured while disabling all secure boot SMI functions successful.

**/
EFI_STATUS
SetDwordRegisterByCpuIndex (
  IN  EFI_SMM_SAVE_STATE_REGISTER       RegisterNum,
  IN  UINTN                             CpuIndex,
  IN  UINT32                            *RegisterData
  )
{
  UINTN                       Width;

  Width = sizeof (UINT32);

  if (RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_CS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_DS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_ES ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_SS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_FS ||
      RegisterNum == EFI_SMM_SAVE_STATE_REGISTER_GS) {
    Width = sizeof (UINT16);
  }

  return mSmmCpu->WriteSaveState (
                    mSmmCpu,
                    Width,
                    RegisterNum,
                    CpuIndex,
                    RegisterData
                    );
}

/**
  Ensures that mSmbiosTableEntryPoint and mSmbiosTable are filled in.

  @param  none

**/
VOID
LocateSmbiosTable (
  )
{
  UINT32             *Address;
  BOOLEAN            IsSmbios64BitTable;
  BOOLEAN            IsSmbios32BitTable;
  BOOLEAN            LegacySmbios64BitTableFound;
  BOOLEAN            LegacySmbios32BitTableFound;

  IsSmbios32BitTable = FALSE;
  IsSmbios64BitTable = FALSE; 
  LegacySmbios32BitTableFound = FALSE;
  LegacySmbios64BitTableFound = FALSE;

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) < 0x3) || 
      (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT0) == BIT0))) {
    IsSmbios32BitTable = TRUE;
  }

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT1) == BIT1)) {
    IsSmbios64BitTable = TRUE;
  }

  //
  // Locate SMBIOS structure table entry point when Legacy boot.
  //
  if (IsSmbios32BitTable && ((mSmbiosTableEntryPoint == NULL) || ((UINT32)(UINTN)mSmbiosTableEntryPoint->AnchorString != (UINT32)SMBIOS_SIGNATURE))) {
    for (Address = (UINT32 *)(UINTN) 0xF0000; (UINTN)Address < 0x100000; Address += 4) {
      mSmbiosTableEntryPoint = (SMBIOS_TABLE_ENTRY_POINT *)(UINTN) Address;
      if (*(UINT32 *)mSmbiosTableEntryPoint == (UINT32)SMBIOS_SIGNATURE) {
        mSmbiosTable = (SMBIOS_STRUCTURE *)(UINTN)mSmbiosTableEntryPoint->TableAddress;
        LegacySmbios32BitTableFound = TRUE;
        break;
      }
    }
  }

  if (IsSmbios64BitTable && ((mSmbiosTableEntryPoint64Bit == NULL) || (AsciiStrnCmp ((CHAR8*)mSmbiosTableEntryPoint64Bit->AnchorString, mSmbio30Signature, sizeof(mSmbio30Signature)) != 0 ))) {
    for (Address = (UINT32 *)(UINTN) 0xF0000; (UINTN)Address < 0x100000; Address += 4) {
      mSmbiosTableEntryPoint64Bit = (SMBIOS_TABLE_3_0_ENTRY_POINT *)(UINTN) Address;
      if (AsciiStrnCmp ((CHAR8*)mSmbiosTableEntryPoint64Bit->AnchorString, mSmbio30Signature, AsciiStrLen (mSmbio30Signature)) == 0 ) {
        mSmbiosTable64Bit = (SMBIOS_STRUCTURE *)(UINTN)mSmbiosTableEntryPoint64Bit->TableAddress;
        LegacySmbios64BitTableFound = TRUE;
        break;
      }
    }
  }

  if (IsSmbios32BitTable && IsSmbios64BitTable){
    if (LegacySmbios32BitTableFound == TRUE && LegacySmbios64BitTableFound == TRUE){
      return;
    }
  } else if (IsSmbios32BitTable){
    if (LegacySmbios32BitTableFound == TRUE){
      return;
    }
  } else if (IsSmbios64BitTable){
    if (LegacySmbios64BitTableFound == TRUE){
      return;
    }
  }


  //
  // If the entry point is not found in CSM,
  // locate SMBIOS structure table entry point at Runtime.
  //
  if (IsSmbios32BitTable) {
    mSmbiosTableEntryPoint = (SMBIOS_TABLE_ENTRY_POINT*)(UINTN)mSMBIOSTableEntryAddress;
    mSmbiosTable = (SMBIOS_STRUCTURE*)(UINTN)mSmbiosTableEntryPoint->TableAddress;
  }

  if (IsSmbios64BitTable) {
    mSmbiosTableEntryPoint64Bit = (SMBIOS_TABLE_3_0_ENTRY_POINT*)(UINTN)mSMBIOSTableEntryAddress64Bit;
    mSmbiosTable64Bit = (SMBIOS_STRUCTURE*)(UINTN)((SMBIOS_TABLE_3_0_ENTRY_POINT*)mSmbiosTableEntryPoint64Bit)->TableAddress;
  }
}

/**
  Locates the smbios structure with a handle of *Structure.

  @param[In]   Structure      Handle of structure to attempt to locate.
                              - If 0 on entry, then the first avaiable structure is found.
                              - On exit, *Structure is updated with the next structure handle.
                              A return handle of 0xffff indicates there are no more structures.
                              *Structure is always updated with useful information.
  @param[Out]  Location       Return the structure location if it was found.
  @param[Out]  Size           The size of structure location if it was found.

  @retval EFI_SUCCESS    The structure was found. **Location and *Size are valid.
  @retval EFI_NOT_FOUND  The structure was not found.

**/
EFI_STATUS
LocateSmbiosStructure (
  IN BOOLEAN                            IsSmbios32BitTable,
  IN OUT UINT16                         *Structure,
  OUT SMBIOS_STRUCTURE                  **Location,
  OUT UINTN                             *Size
  )
{
  SMBIOS_STRUCTURE                      *Iter;
  SMBIOS_STRUCTURE                      *Next;
  SMBIOS_STRUCTURE                      *End;

  if (IsSmbios32BitTable &&(*Structure == 0) && ((*Location) != NULL) && (*(UINT32 *)(*Location) == (UINT32)SMBIOS_SIGNATURE)) {
    *Structure = mSmbiosTable->Handle;
  }

  if (!IsSmbios32BitTable &&(*Structure == 0) && ((*Location) != NULL) && ( AsciiStrnCmp ((CHAR8*)*Location, mSmbio30Signature, AsciiStrLen(mSmbio30Signature)) == 0)) {
    *Structure = mSmbiosTable64Bit->Handle;
  }

  if (IsSmbios32BitTable){
    Iter = mSmbiosTable;
  } else {
    Iter = mSmbiosTable64Bit;
  }
  
  if (Iter == NULL){
    return EFI_NOT_FOUND;
  }
  //
  // Adjust the end pointer if SMBIOS data is modified, such as strings.
  //
  if (IsSmbios32BitTable) {
    End = (SMBIOS_STRUCTURE *)(UINTN)(((SMBIOS_TABLE_ENTRY_POINT*)mSmbiosTableEntryPoint)->TableAddress + ((SMBIOS_TABLE_ENTRY_POINT*)mSmbiosTableEntryPoint)->TableLength);
  } else {
    End = (SMBIOS_STRUCTURE *)(UINTN)(((SMBIOS_TABLE_3_0_ENTRY_POINT*)mSmbiosTableEntryPoint64Bit)->TableAddress + ((SMBIOS_TABLE_3_0_ENTRY_POINT*)mSmbiosTableEntryPoint64Bit)->TableMaximumSize);
  }

  if (TotalIncreasedStringLength >= 0x00) {
    End = (SMBIOS_STRUCTURE *)((UINTN)End + (UINTN)TotalIncreasedStringLength);
  } else {
    End = (SMBIOS_STRUCTURE *)((UINTN)End - (UINTN)(~(TotalIncreasedStringLength - 1)));
  }

  while (TRUE) {
    for (Next = (SMBIOS_STRUCTURE *)((UINT8 *)Iter + Iter->Length); *(UINT16 *)Next != 0; Next = (SMBIOS_STRUCTURE *)((UINTN)Next + 1));
    Next = (SMBIOS_STRUCTURE *)((UINTN)Next + 2);
    if (Iter >= End) {
      //
      // End-of-list indicator
      //
      *Structure = 0xffff;
      return EFI_NOT_FOUND;
    } else if (Iter->Handle == *Structure) {
      *Structure = (UINT16)((Next < End) ? Next->Handle : 0xffff);
      *Location = Iter;
      *Size = (UINTN)Next - (UINTN)Iter;
      return EFI_SUCCESS;
    }
    Iter = Next;
  }
}

/**
  Find the location and size of a string within a smbios structure.

  @param[In]   StringRef      The string number to search for within the structure.
  @param[In]   Structure      The Pointer to the structure to search.
  @param[Out]  Location       Return the string location if it was found.
  @param[Out]  Size           The size of string location if it was found.

  @retval EFI_SUCCESS    the string was found. **Location and *Size are valid.
  @retval EFI_NOT_FOUND  The structure was not found.

**/
EFI_STATUS
LocateStringOfStructure (
  IN UINT8                              StringRef,
  IN SMBIOS_STRUCTURE                   *Structure,
  OUT CHAR8                             **Location,
  OUT UINTN                             *Size
  )
{
  CHAR8                                 *Next;

  if (StringRef == 0) {
    return EFI_NOT_FOUND;
  }

  Next = (CHAR8 *)(((UINTN)Structure) + Structure->Length);
  do {
    *Location = Next;
    *Size = AsciiStrLen (*Location);
    Next = *Location + *Size + 1;
  } while ((--StringRef) != 0 && (*Next != 0));

  return (StringRef == 0) ? EFI_SUCCESS : EFI_NOT_FOUND;
}

/**
  If OEM_DMI_STORE address full to arrange.

  @param[Out]  DmiEnd         Pointer to function 0x52 data buffer input.

  @retval EFI_SUCCESS    The function return valid value.
  @retval EFI_NOT_FOUND  Error occurs.

**/
EFI_STATUS
DMISpaceNotEnough (
  OUT UINT16                            *DmiEnd
  )
{
  EFI_STATUS                  Status;
  UINTN                       BlockSize;
  UINTN                       NvStorageDmiBase;
  UINT8                       *NotEnoughBuffer;
  UINT16                      NotEnoughPtr;
  UINT8                       *Buffer;
  DMI_STRING_STRUCTURE        *CurrentPtr;

  NotEnoughBuffer  = NULL;
  Buffer           = NULL;
  CurrentPtr       = NULL;
  NotEnoughPtr     = 4;

  NvStorageDmiBase = (UINTN) FdmGetNAtAddr (&gH2OFlashMapRegionSmbiosUpdateGuid, 1);
  BlockSize        = (UINTN) FdmGetNAtSize (&gH2OFlashMapRegionSmbiosUpdateGuid, 1);

  Status = mSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    BlockSize,  //0x1000
                    (VOID **)&NotEnoughBuffer
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SetMem (NotEnoughBuffer, BlockSize, 0xFF);
  *(UINT32 *)NotEnoughBuffer = DMI_UPDATE_STRING_SIGNATURE;

  //
  // Search smbios Valid, If it is found, set OEM_DMI_STORE address
  //
  Buffer = (UINT8 *)(NvStorageDmiBase + sizeof (DMI_UPDATE_STRING_SIGNATURE));
  while ((UINTN)Buffer < (NvStorageDmiBase + BlockSize)) {
    CurrentPtr = (DMI_STRING_STRUCTURE *)Buffer;
    if (CurrentPtr->Type == 0xFF) {
      break;
    }
    if (CurrentPtr->Valid == 0xFF) {
      CopyMem ((NotEnoughBuffer+NotEnoughPtr), Buffer, CurrentPtr->Length);
      NotEnoughPtr = NotEnoughPtr + CurrentPtr->Length;
    }
    Buffer = Buffer + CurrentPtr->Length;
  }

  //
  // Flash whole buffer to rom
  //
  Status = mSmmFwBlockService->EraseBlocks (
                                 mSmmFwBlockService,
                                 NvStorageDmiBase,
                                 &BlockSize
                                 );
  if (!EFI_ERROR (Status)) {
    Status = mSmmFwBlockService->Write (
                                   mSmmFwBlockService,
                                   NvStorageDmiBase,
                                   &BlockSize,
                                   NotEnoughBuffer
                                   );
  }

  mSmst->SmmFreePool (NotEnoughBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((UINTN)NotEnoughPtr >= (NvStorageDmiBase + BlockSize)) {
    return EFI_OUT_OF_RESOURCES;
  }

  *DmiEnd = NotEnoughPtr;
  return EFI_SUCCESS;
}

/**
  Set SMBIOS Structure to OEM_DMI_STORE address.

  @param[In]  Data           Pointer to function 0x52 data buffer input.
  @param[In]  Structure      The Pointer to the structure to search.

  @retval EFI_SUCCESS    The function sets valid value for Data.
  @retval EFI_NOT_FOUND  Error occurs.

**/
EFI_STATUS
SetDMI (
  IN FUNC_0x52_DATA_BUFFER              *Data,
  IN SMBIOS_STRUCTURE                   *Structure
  )
{
  EFI_STATUS                  Status;
  UINTN                       Index;
  UINTN                       BlockSize;
  UINTN                       BSize;
  UINT8                       Invalid;
  UINT8                       *Buffer;
  UINT8                       *Ptr;
  UINT8                       *SavePtr;
  UINT16                      DmiEnd;
  DMI_STRING_STRUCTURE        *CurrentPtr;
  UINT64                      DmiAddr;
  UINT64                      DmiSize;

  BlockSize  = 0x1000;
  BSize      = 0x1;
  Invalid    = 0;
  SavePtr    = NULL;
  CurrentPtr = NULL;
  DmiEnd     = 4;

  DmiAddr = FdmGetNAtAddr(&gH2OFlashMapRegionSmbiosUpdateGuid, 1);
  DmiSize = FdmGetNAtSize(&gH2OFlashMapRegionSmbiosUpdateGuid, 1);

  //
  // First 4 bytes are "$DMI"
  //
  Ptr        = (UINT8 *)(UINTN) DmiAddr;
  Buffer = Ptr + sizeof (DMI_UPDATE_STRING_SIGNATURE);

  //
  // Search OEM_DMI_STORE free space
  //
  while (DmiEnd < DmiSize) {
    CurrentPtr = (DMI_STRING_STRUCTURE *)Buffer;

    if (CurrentPtr->Type == 0xFF) {
      //
      // The space is free, we could use it, so break.
      //
      break;
    }

    if ((CurrentPtr->Type == Structure->Type) &&
        (CurrentPtr->Offset == Data->FieldOffset) &&
        (CurrentPtr->Valid == 0xFF)) {
      //
      // If we find the same data in OEM_DMI_STORE space, set invalid byte in it.
      //
      Status = mSmmFwBlockService->Write (
                                     mSmmFwBlockService,
                                     (UINTN)(DmiAddr + DmiEnd + 2),
                                     &BSize,
                                     &Invalid
                                     );
    }

    //
    // Try to find next.
    //
    DmiEnd = DmiEnd + CurrentPtr->Length;
    Buffer = Buffer + CurrentPtr->Length;
  }

  if (DmiEnd + Data->DataLength + (sizeof (DMI_STRING_STRUCTURE) - sizeof(UINT8) - 1) >
      DmiSize) {
    //
    // If OEM_DMI_STORE space full, reclaim area, and return new address of free space address.
    //
    Status = DMISpaceNotEnough (&DmiEnd);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // Set update data to OEM_DMI_STORE address.
  //
  Status = mSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    Data->DataLength + (sizeof (DMI_STRING_STRUCTURE) - sizeof (UINT8) - 1),
                    (VOID **)&SavePtr
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CurrentPtr         = (DMI_STRING_STRUCTURE *)SavePtr;
  CurrentPtr->Type   = Structure->Type;
  CurrentPtr->Offset = Data->FieldOffset;
  CurrentPtr->Valid  = 0xFF;

  switch (Data->Command) {

  case DMI_STRING_CHANGE:
    CurrentPtr->Length = Data->DataLength + (sizeof (DMI_STRING_STRUCTURE) - sizeof (UINT8) - 1);
    for (Index = 0; Index + 1 < Data->DataLength; Index++) {
      CurrentPtr->String[Index] = Data->StructureData[Index];
    }
    break;

  default:
    CurrentPtr->Length = Data->DataLength + (sizeof (DMI_STRING_STRUCTURE) - sizeof (UINT8));
    for (Index = 0; Index <= Data->DataLength; Index++) {
      CurrentPtr->String[Index] = Data->StructureData[Index];
    }
    break;
  }

  BlockSize = CurrentPtr->Length;
  Status = mSmmFwBlockService->Write (
                                 mSmmFwBlockService,
                                 (UINTN)(DmiAddr + DmiEnd),
                                 &BlockSize,
                                 SavePtr
                                 );

  mSmst->SmmFreePool (SavePtr);
  return Status;
}

BOOLEAN
VaildGPNVCheck (
  IN OUT UINT16                     *Index,
  IN UINT16                         Handle
  )
{
  UINT16                            TempIndex;

  if (mIsOemGPNVMap == FALSE ||
      OemGPNVHandleCount >= FixedPcdGet16 (PcdDefaultGpnvMapBufferSize) ||
      *Index >= FixedPcdGet16 (PcdDefaultGpnvMapBufferSize) ||
	  *Index >= OemGPNVHandleCount) {
    return FALSE;
  }

  for (TempIndex = *Index; TempIndex < OemGPNVHandleCount; TempIndex++) {
    if (GPNVMapBuffer.PlatFormGPNVMap[TempIndex].Handle == Handle) {
      *Index = TempIndex;
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
WriteGPNV (
  IN UINT16                             Handle,
  IN UINT8                              *GPNVBuffer
  )
{
  EFI_STATUS                            Status;
  UINTN                                 BlockSize;
  UINT8                                 *WritingBuffer;
  UINTN                                 BlockBaseAddress;
  UINTN                                 WritingBufferOffset;
  UINT8                                 EraseCount;
  UINT8                                 WriteCount;

  BlockSize = 0x10000;
  Status = mSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    BlockSize,
                    (VOID **)&WritingBuffer
                    );

  BlockBaseAddress = (GPNVMapBuffer.PlatFormGPNVMap[Handle].BaseAddress) & (~0xFFFF);
  //
  // Copy whole block data to buffer
  //
  CopyMem (WritingBuffer, (VOID *)(UINTN)BlockBaseAddress, BlockSize);

  //
  // Copy modified GPNV data to buffer
  //
  WritingBufferOffset = GPNVMapBuffer.PlatFormGPNVMap[Handle].BaseAddress - BlockBaseAddress;
  CopyMem ((VOID *)(WritingBuffer + WritingBufferOffset), (VOID *)GPNVBuffer, GPNVMapBuffer.PlatFormGPNVMap[Handle].GPNVSize);

  //
  // Flash whole buffer to rom
  //
  EraseCount = 0;
  WriteCount = 0;
  do {
    Status = mSmmFwBlockService->EraseBlocks (
                                   mSmmFwBlockService,
                                   BlockBaseAddress,
                                   (UINTN *)&BlockSize
                                   );

    if (!EFI_ERROR (Status)) {
      Status = mSmmFwBlockService->Write (
                                     mSmmFwBlockService,
                                     BlockBaseAddress,
                                     (UINTN *)&BlockSize,
                                     WritingBuffer
                                     );
      if (!EFI_ERROR (Status)) {
        mSmst->SmmFreePool (WritingBuffer);
        return Status;
      } else {
        WriteCount++;
      }
    } else {
      EraseCount++;
    }
  } while ((EraseCount < 100) && (WriteCount < 100));

  mSmst->SmmFreePool (WritingBuffer);
  return Status;
}

UINT32
LogicalToPhysicalAddress (
  IN UINT32  LinearAddress,
  IN UINT32  CR3Value,
  IN UINT32  CR4Value,
  IN UINTN   SegementSelector
  )
{
  UINT32  *TmpPtr;
  UINT32  Buffer32;
  UINT32  Edi;

  if ((CR3Value & 0x00000001) == 0) {
    return ((UINT32)SegementSelector << 4 ) + LinearAddress;
  }

  //
  // 32bit Mode SMI: transfer to physical address
  // IA-32 Intel Architecture Software Developer's Manual
  // Volume 3 - Chapter 3.7
  // there is 2 necessay condition:
  // 1.Base address in Segement Selector(GDT) must be 0
  //   if not 0, the Linear address need add the base address first
  //   Check Chapter 3.4 "Logical and Linear Addresses"
  if (SegementSelector != 0x10) {
    //
    // Base Address in SegementSelector 0x10 is set to 0 in SEC Phase.
    // Other will need to be read GDT...not impelement yet.Just return.
    //
    GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RDI, mCpuIndex, &Edi);
    LinearAddress += Edi;
  }

  //
  // 2.Only support 4MByte Pages now for XP-SP2
  //   4KByte pages isn't verified.
  //   if need support 4KByte Pages for Win2K...etc,
  //   check Chapter 3.7.1
  //

  // Use CR4 Bit5 to check 2MBytes Page for XD enable
  if (CR4Value & 0x20) {
    Buffer32 = CR3Value & 0xFFFFFFE0;

    //
    // Page Directory Pointer Table Entry
    //
    TmpPtr = (UINT32 *)(UINTN)(Buffer32 + ((LinearAddress & 0xC0000000) >> 27));
    Buffer32 = *TmpPtr;

    //
    // 2MByte Page - Page Directory Entry
    //
    TmpPtr = (UINT32 *)(UINTN)((Buffer32 & 0xFFFFF000) + ((LinearAddress & 0x3FE00000) >> 18));
    Buffer32 = *TmpPtr;

    if ((Buffer32 & 0x80) == 0) {
      //
      // 4KByte Pages - Page Table
      //
      TmpPtr = (UINT32 *)(UINTN)((Buffer32 & 0xFFFFF000) + ((LinearAddress & 0x001FF000) >> 9));
      Buffer32 = *TmpPtr;
      //
      // 4-KByte Page - Physical Address
      //
      Buffer32 = (Buffer32 & 0xFFFFF000) + (LinearAddress & 0x00000FFF);
    } else {
      //
      // 2MByte Page - Physical Address
      //
      Buffer32 = (Buffer32 & 0xFFE00000) + (LinearAddress & 0x001FFFFF);
    }
  } else {
    //
    // Get Page-Directory from CR3
    //
    Buffer32 = CR3Value & 0xFFFFF000;

    //
    // Page Directory Entry
    //
    TmpPtr = (UINT32 *)(UINTN)(Buffer32 + ((LinearAddress & 0xFFC00000) >> 20));
    Buffer32 = *TmpPtr;

    //
    // Check 4KByte/4MByte Pages
    //
    if ((Buffer32 & 0x80) == 0) {
      //
      // 4KByte Pages - Page Table
      //
      TmpPtr = (UINT32 *)(UINTN)((Buffer32 & 0xFFFFF000) + ((LinearAddress & 0x003FF000) >> 10));
      Buffer32 = *TmpPtr;
      //
      // 4-KByte Page - Physical Address
      //
      Buffer32 = (Buffer32 & 0xFFFFF000) + (LinearAddress & 0x00000FFF);
    } else {
      //
      // 4MByte Pages - Physical Address
      //
      Buffer32 = (Buffer32 & 0xFFC00000) + (LinearAddress & 0x003FFFFF);
    }
  }

  return Buffer32;
}

/**
  Does a quick conversion from a 16-bit C far pointer to a linear address.
  Unfortunately, this can only support far pointers from 16-bit real mode.

  @param[In]  Ptr     16-bit far pointer.

  @return  VOID*      The linear address of Ptr.

**/
VOID *
PnpFarToLinear (
  IN PNP_FAR_PTR                        Ptr
  )
{
  UINT32                                CR3Value;
  UINT32                                CR4Value;
  UINT32                                Ebx, Ecx;
  UINTN                                 SegementSelector;


  GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RBX, mCpuIndex, &Ebx);
  GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RCX, mCpuIndex, &Ecx);

  //
  // Call by new way that AP trigger software SMI directly when the signature is "$ISB'
  //
  if (Ebx == SIGNATURE_32 ('$', 'I', 'S', 'B')) {
    return (VOID *)(UINTN)(UINT32)(Ptr.Offset | Ptr.Segment << 16);
  }

  CR3Value = Ebx;
  CR4Value = Ecx;
  SegementSelector = (UINTN)Ptr.Segment;

  return (VOID *)(UINTN)LogicalToPhysicalAddress ((UINT32)Ptr.Offset, CR3Value, CR4Value, SegementSelector);
}

/**
  Helper function for Pnp0x52, process command DMI_BYTE_CHANGE, DMI_WORD_CHANGE, and DMI_DWORD_CHANGE.

  @param[In]  Data           Buffer of data to SetStructure.
  @param[In]  Structure      The structure to be set.

  @retval DMI_SUCCESS    The function executed successfully.
  @retval DMI_READ_ONLY  The structure is not Type1, the block to be set is not at offset 8.

**/
INT16
Pnp0x52ChangeFixedLength (
  IN FUNC_0x52_DATA_BUFFER             *Data,
  IN SMBIOS_STRUCTURE                  *Structure
  )
{
  UINT8                       *ByteTmpPtr;
  UINT16                      *WordTmpPtr;
  UINT32                      *DWordTmpPtr;
  EFI_STATUS                  Status;
  UINT8                       ByteTemp;
  UINT16                      WordTemp;
  UINT32                      DWordTemp;
  UINTN                       Index;

  ByteTmpPtr  = NULL;
  WordTmpPtr  = NULL;
  DWordTmpPtr = NULL;
  ByteTemp    = 0;
  WordTemp    = 0;
  DWordTemp   = 0;

  for (Index = 0; Index < UpdateableStringCount; Index++) {
    if ((Structure->Type == mUpdatableStrings[Index].Type) &&
        (Data->FieldOffset == mUpdatableStrings[Index].FixedOffset)) {
      break;
    }
  }

  if (Index == UpdateableStringCount) {
    return DMI_READ_ONLY;
  }

  switch (Data->Command) {

  case DMI_BYTE_CHANGE:
    ByteTmpPtr = (UINT8 *)((UINTN)Structure + Data->FieldOffset);
    ByteTemp   = *ByteTmpPtr;
    ByteTemp   = ByteTemp & ((UINT8)(Data->ChangeMask));
    ByteTemp   = ByteTemp | ((UINT8)((Data->ChangeValue) & ~(Data->ChangeMask)));

    Data->DataLength = sizeof (UINT8);
    CopyMem ((VOID *)&Data->StructureData, (VOID *)&ByteTemp, Data->DataLength);
    break;

  case DMI_WORD_CHANGE:
    WordTmpPtr = (UINT16 *) ((UINTN)Structure + Data->FieldOffset);
    WordTemp   = *WordTmpPtr;
    WordTemp   = WordTemp & ((UINT16)(Data->ChangeMask));
    WordTemp   = WordTemp | ((UINT16)((Data->ChangeValue) & ~(Data->ChangeMask)));

    Data->DataLength = sizeof (UINT16);
    CopyMem ((VOID *)&Data->StructureData, (VOID *)&WordTemp, Data->DataLength);
    break;

  case DMI_DWORD_CHANGE:
    DWordTmpPtr = (UINT32 *) ((UINTN)Structure + Data->FieldOffset);
    DWordTemp   = *DWordTmpPtr;
    DWordTemp   = DWordTemp & ((UINT32)(Data->ChangeMask));
    DWordTemp   = DWordTemp | ((UINT32)((Data->ChangeValue) & ~(Data->ChangeMask)));

    Data->DataLength = sizeof (UINT32);
    CopyMem ((VOID *)&Data->StructureData, (VOID *)&DWordTemp, Data->DataLength);

    break;
  }

  Status = SetDMI (Data, Structure);
  if (EFI_ERROR (Status)) {
    return DMI_READ_ONLY;
  }

  switch (Data->Command) {

  case DMI_BYTE_CHANGE:
    CopyMem (ByteTmpPtr, &ByteTemp, sizeof (UINT8));
    break;

  case DMI_WORD_CHANGE:
    CopyMem (WordTmpPtr, &WordTemp, sizeof (UINT16));
    break;

  case DMI_DWORD_CHANGE:
    CopyMem (DWordTmpPtr, &DWordTemp, sizeof (UINT32));
    break;

  default:
    break;
  }

  return DMI_SUCCESS;
}

/**
  Helper function for Pnp0x52, process command DMI_STRING_CHANGE.

  @param[In]  Data           Buffer of data to SetStructure.
  @param[In]  Structure      The structure to be set.

  @retval DMI_SUCCESS          The function executed successfully.
  @retval DMI_BAD_PARAMETER    The length of data is invalid or can not locate the string in the structure.
  @retval DMI_READ_ONLY        Cannot set the data in the structure.

**/
INT16
Pnp0x52ChangeString (
  IN BOOLEAN                               IsSmbios32BitTable,
  IN FUNC_0x52_DATA_BUFFER                 *Data,
  IN SMBIOS_STRUCTURE                      *Structure
  )
{
  CHAR8                                 *StructureString;
  CHAR8                                 *TempstructureString;
  UINTN                                 StructureStringSize;
  UINTN                                 TempstructureStringSize;
  UINTN                                 Index;
  EFI_STATUS                            Status;
  UINT8                                 *SmbiosStoreArea;
  UINTN                                 CurrentTableSize;
  UINT8                                 *DmiPtr;
  UINTN                                 TotalSmbiosBufferSize;
  UINTN                                 NewTableSize;
  STRING_COUNT_TABLE                    *CountFieldTable;
  UINTN                                 CountFieldTableSize;
  UINT8                                 RecordLength;
  BOOLEAN                               CountFieldType = FALSE;
  EFI_PCD_PROTOCOL                      *Pcd;
  UINT32                                SmbiosTableLength;
  UINT64                                SmbiosTableAddress;


  TempstructureString = NULL;
  TempstructureStringSize = 0;
  SmbiosStoreArea = NULL;
  
  DmiPtr = (UINT8 *)(UINTN) FdmGetNAtAddr(&gH2OFlashMapRegionSmbiosUpdateGuid, 1);

  if (!(*(UINT32 *)DmiPtr == DMI_UPDATE_STRING_SIGNATURE)){
    return DMI_READ_ONLY;
  }

  if ((Data->DataLength > 0xff) || (Data->DataLength <= 1)) {
    return DMI_BAD_PARAMETER;
  }

  Status = mSmst->SmmLocateProtocol (
                    &gEfiPcdProtocolGuid,
                    NULL,
                    (VOID **)&Pcd
                    );
  if (EFI_ERROR (Status)) {
    return DMI_FUNCTION_NOT_SUPPORTED;
  }

  CountFieldTable = (STRING_COUNT_TABLE *)Pcd->GetPtr (&gSmbiosTokenSpaceGuid, PcdToken (PcdSmbiosStringCountFieldOffset));
  CountFieldTableSize = Pcd->GetSize (&gSmbiosTokenSpaceGuid, PcdToken (PcdSmbiosStringCountFieldOffset)) / sizeof (STRING_COUNT_TABLE);
  RecordLength = 0;
  for (Index = 0; Index < CountFieldTableSize; Index++) {
    if (Structure->Type == CountFieldTable[Index].Type) {
      RecordLength = (UINT8)Pcd->GetSize (&gSmbiosTokenSpaceGuid, PcdToken (PcdType000Record) + CountFieldTable[Index].Type) - 1;
      CountFieldType = TRUE;
      break;
    }
  }

  //
  // Find string within the SMBIOS structure
  //
  if (CountFieldType) {
    Status = LocateStringOfStructure (
               (UINT8)Data->FieldOffset,
               Structure,
               &StructureString,
               &StructureStringSize
               );
    Data->FieldOffset += RecordLength;
  } else {
    Status = LocateStringOfStructure (
               ((UINT8 *)Structure)[Data->FieldOffset],
               Structure,
               &StructureString,
               &StructureStringSize
               );
  }
  if (EFI_ERROR (Status)) {
    return DMI_BAD_PARAMETER;
  }

  for (Index = 0; Index < UpdateableStringCount; Index++) {
    if ((Structure->Type == mUpdatableStrings[Index].Type) &&
        (Data->FieldOffset == mUpdatableStrings[Index].FixedOffset)) {
      break;
    }
  }
  if (Index == UpdateableStringCount) {
    return DMI_READ_ONLY;
  }

  Status = SetDMI (Data, Structure);
  if (EFI_ERROR (Status)) {
    return DMI_READ_ONLY;
  }

  if (IsSmbios32BitTable) {
    SmbiosTableLength = ((SMBIOS_TABLE_ENTRY_POINT*)mSmbiosTableEntryPoint)->TableLength;
    SmbiosTableAddress = ((SMBIOS_TABLE_ENTRY_POINT*)mSmbiosTableEntryPoint)->TableAddress;
  } else {
    SmbiosTableLength = ((SMBIOS_TABLE_3_0_ENTRY_POINT*)mSmbiosTableEntryPoint64Bit)->TableMaximumSize;
    SmbiosTableAddress = ((SMBIOS_TABLE_3_0_ENTRY_POINT*)mSmbiosTableEntryPoint64Bit)->TableAddress;
  }

  //
  // This value is based on SmbiosDxe driver. If the buffer allocation method is changed, this size will be different.
  //
  TotalSmbiosBufferSize = EFI_PAGES_TO_SIZE (EFI_SIZE_TO_PAGES (SmbiosTableLength));
  TempstructureString = StructureString + StructureStringSize + 1;
  if (TotalIncreasedStringLength >= 0x00) {
    CurrentTableSize = (UINTN)SmbiosTableLength + (UINTN)TotalIncreasedStringLength;
    TempstructureStringSize = (UINTN)SmbiosTableLength + (UINTN)TotalIncreasedStringLength -
                              (UINTN)(TempstructureString - SmbiosTableAddress);
  } else {
    CurrentTableSize = (UINTN)SmbiosTableLength - (UINTN)(~(TotalIncreasedStringLength - 1));
    TempstructureStringSize = (UINTN)SmbiosTableLength - (UINTN)(~(TotalIncreasedStringLength - 1)) -
                              (UINTN)(TempstructureString - SmbiosTableAddress);
  }

  //
  // If new SMBIOS data is over than the buffer, nothing will change.
  // Data->DataLength : new string length, including '0'
  // structureStringSize : original string length
  //
  if ((UINTN)Data->DataLength > (StructureStringSize + 1)) {
    NewTableSize = CurrentTableSize + (UINTN)Data->DataLength - (StructureStringSize + 1);
    if (NewTableSize > TotalSmbiosBufferSize) {
      return DMI_NO_CHANGE;
    }
  }

  Status = mSmst->SmmAllocatePool (EfiRuntimeServicesData, CurrentTableSize, (VOID **)&SmbiosStoreArea);
  if (EFI_ERROR (Status)) {
    return PNP_BUFFER_TOO_SMALL;
  }

  CopyMem (SmbiosStoreArea, TempstructureString, TempstructureStringSize);
  CopyMem (StructureString, Data->StructureData, Data->DataLength);
  TempstructureString = StructureString + Data->DataLength;
  CopyMem (TempstructureString, SmbiosStoreArea, TempstructureStringSize);
  Status = mSmst->SmmFreePool (SmbiosStoreArea);

  TotalIncreasedStringLength = TotalIncreasedStringLength + (UINT32)Data->DataLength - (UINT32)(StructureStringSize + 1);
  return DMI_SUCCESS;
}

/**
  Helper function for Pnp0x52, process command DMI_BLOCK_CHANGE.

  @param[In]  Data           Buffer of data to SetStructure.
  @param[In]  Structure      The structure to be set.

  @retval DMI_SUCCESS          The function executed successfully.
  @retval DMI_BAD_PARAMETER    The length of data is invalid or can not locate the string in the structure.
  @retval DMI_READ_ONLY        The structure is not Type1, the block to be set is not at offset 8.

**/
INT16
Pnp0x52ChangeBlock (
  IN FUNC_0x52_DATA_BUFFER                 *Data,
  IN SMBIOS_STRUCTURE                      *Structure
  )
{
  UINT8                       *TmpPtr;
  UINT8                       *DataPtr;
  UINTN                       CopyLength;
  EFI_STATUS                  Status;


  TmpPtr     = (UINT8 *)((UINTN)Structure + Data->FieldOffset);
  DataPtr    = (UINT8 *)Data->StructureData;
  CopyLength = (UINTN)Data->DataLength;

  if ((Structure->Type != 1) || (Data->FieldOffset != 8)) {
    return DMI_READ_ONLY;
  }

  if (CopyLength != sizeof (EFI_GUID)){
    return DMI_BAD_PARAMETER;
  }

  Status = SetDMI (Data, Structure);
  if (EFI_ERROR (Status)) {
    return DMI_READ_ONLY;
  }
  CopyMem (TmpPtr, Data->StructureData, Data->DataLength);

  return DMI_SUCCESS;
}


    /**

  Get the full size of SMBIOS structure including optional strings that follow the formatted structure.

  @param This                   The EFI_SMBIOS_PROTOCOL instance.
  @param Head                   Pointer to the beginning of SMBIOS structure.
  @param Size                   The returned size.
  @param NumberOfStrings        The returned number of optional strings that follow the formatted structure.

  @retval EFI_SUCCESS           Size retured in Size.
  @retval EFI_INVALID_PARAMETER Input SMBIOS structure mal-formed or Size is NULL.
  
**/
EFI_STATUS
EFIAPI
GetSmbiosStructureSize (
  IN   EFI_SMBIOS_TABLE_HEADER          *Head,
  OUT  UINTN                            *Size,
  OUT  UINTN                            *NumberOfStrings
  )
{
  UINTN  FullSize;
  UINTN  StrLen;
  UINTN  MaxLen;
  INT8*  CharInStr;

  if (Size == NULL || NumberOfStrings == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FullSize = Head->Length;
  CharInStr = (INT8*)Head + Head->Length;
  *Size = FullSize;
  *NumberOfStrings = 0;
  StrLen = 0;

  //
  // SMBIOS 3.0 defines the Structure table maximum size as DWORD field limited to 0xFFFFFFFF bytes.
  // Locate the end of string as long as possible.
  //  
  MaxLen = SMBIOS_3_0_TABLE_MAX_LENGTH;

  //
  // look for the two consecutive zeros, check the string limit by the way.
  //
  while (*CharInStr != 0 || *(CharInStr+1) != 0) { 
    if (*CharInStr == 0) {
      *Size += 1;
      CharInStr++;
    }

    for (StrLen = 0 ; StrLen < MaxLen; StrLen++) {
      if (*(CharInStr+StrLen) == 0) {
        break;
      }
    }

    if (StrLen == MaxLen) {
      return EFI_INVALID_PARAMETER;
    }

    //
    // forward the pointer
    //
    CharInStr += StrLen;
    *Size += StrLen;
    *NumberOfStrings += 1;
  }

  //
  // count ending two zeros.
  //
  *Size += 2;
  return EFI_SUCCESS;
}


/**
  PnP function 0x50, Get SMBIOS Information.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS     The function was executed successfully.

**/
INT16
Pnp0x50 (
  IN VOID            *Frame
  )
{
  UINT8                                 *DmiBIOSRevision;
  UINT16                                *NumStructures;
  UINT16                                *StructureSize;
  UINT32                                *DmiStorageBase;
  UINT16                                *DmiStorageSize;
  BOOLEAN     IsSmbios32BitTable;

  IsSmbios32BitTable = FALSE;

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) < 0x3) || 
      (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT0) == BIT0))) {
    IsSmbios32BitTable = TRUE;
  }

  LocateSmbiosTable ();

  if (IsSmbios32BitTable){

    DmiBIOSRevision = (UINT8 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->DmiBIOSRevision);
    if (BufferOverlapSmramPnp ((VOID *) DmiBIOSRevision, sizeof(UINT8))) {
      return DMI_BAD_PARAMETER;
    }
    *DmiBIOSRevision = mSmbiosTableEntryPoint->SmbiosBcdRevision;

    NumStructures = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->NumStructures);
    if (BufferOverlapSmramPnp ((VOID *) NumStructures, sizeof(UINT16))) {
      return DMI_BAD_PARAMETER;
    }
    *NumStructures = mSmbiosTableEntryPoint->NumberOfSmbiosStructures;

    StructureSize = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->StructureSize);
     if (BufferOverlapSmramPnp ((VOID *) StructureSize, sizeof(UINT16))) {
      return DMI_BAD_PARAMETER;
    }
    *StructureSize = mSmbiosTableEntryPoint->MaxStructureSize;

    DmiStorageBase = (UINT32 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->DmiStorageBase);
    if (BufferOverlapSmramPnp ((VOID *) DmiStorageBase, sizeof(UINT32))) {
      return DMI_BAD_PARAMETER;
    }
    *DmiStorageBase = (UINT32)(UINTN)mSmbiosTable;

    DmiStorageSize = (UINT16 *)PnpFarToLinear(((PNP_FUNCTION_0x50_FRAME *)Frame)->DmiStorageSize);
    if (BufferOverlapSmramPnp ((VOID *) DmiStorageSize, sizeof(UINT16))) {
      return DMI_BAD_PARAMETER;
    }
    *DmiStorageSize = mSmbiosTableEntryPoint->TableLength;

  } else {

    UINT64  TableAddress;
    UINT64  TableEndAddress;
    UINTN   NumberOfStrings;
    UINTN   Size;
    UINT16  NumberOfStructure; 
    UINTN   MaxStructureSize;
    EFI_STATUS  Status;

    NumberOfStructure = 0;
    MaxStructureSize = 0;

    if (mSmbiosTableEntryPoint64Bit == NULL){
      return DMI_FUNCTION_NOT_SUPPORTED;
    }
    TableAddress = mSmbiosTableEntryPoint64Bit->TableAddress;
    TableEndAddress = mSmbiosTableEntryPoint64Bit->TableAddress + mSmbiosTableEntryPoint64Bit->TableMaximumSize;

    while (TableAddress < TableEndAddress){
      Status = GetSmbiosStructureSize ((EFI_SMBIOS_TABLE_HEADER*)(UINTN)TableAddress, &Size, &NumberOfStrings);
      if (EFI_ERROR(Status)){
        break;  
      }

      if (MaxStructureSize < Size){
        MaxStructureSize = Size;  
      }
      NumberOfStructure++;
      TableAddress += Size;
    }

    DmiBIOSRevision = (UINT8 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->DmiBIOSRevision);
    if (BufferOverlapSmramPnp ((VOID *) DmiBIOSRevision, sizeof(UINT8))) {
      return DMI_BAD_PARAMETER;
    }
    *DmiBIOSRevision = (UINT8) ((PcdGet16 (PcdSmbiosVersion) >> 4) & 0xf0) | (UINT8) (PcdGet16 (PcdSmbiosVersion) & 0x0f);

    NumStructures = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->NumStructures);
    if (BufferOverlapSmramPnp ((VOID *) NumStructures, sizeof(UINT16))) {
      return DMI_BAD_PARAMETER;
    }
    *NumStructures = NumberOfStructure;

    StructureSize = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->StructureSize);
     if (BufferOverlapSmramPnp ((VOID *) StructureSize, sizeof(UINT16))) {
      return DMI_BAD_PARAMETER;
    }
    *StructureSize = (UINT16) MaxStructureSize;

    DmiStorageBase = (UINT32 *)PnpFarToLinear (((PNP_FUNCTION_0x50_FRAME *)Frame)->DmiStorageBase);
    if (BufferOverlapSmramPnp ((VOID *) DmiStorageBase, sizeof(UINT32))) {
      return DMI_BAD_PARAMETER;
    }
    *DmiStorageBase = (UINT32)(UINTN)mSmbiosTable64Bit;

    DmiStorageSize = (UINT16 *)PnpFarToLinear(((PNP_FUNCTION_0x50_FRAME *)Frame)->DmiStorageSize);
    if (BufferOverlapSmramPnp ((VOID *) DmiStorageSize, sizeof(UINT16))) {
      return DMI_BAD_PARAMETER;
    }
    *DmiStorageSize = (UINT16)((SMBIOS_TABLE_3_0_ENTRY_POINT*)mSmbiosTableEntryPoint64Bit)->TableMaximumSize;
  }

  return DMI_SUCCESS;
}

/**
  PnP function 0x51, Get SMBIOS Structure.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS            The function was executed successfully.
  @retval     DMI_INVALID_HANDLE     Invalid handle.

**/
INT16
Pnp0x51 (
  IN VOID            *Frame
  )
{
  UINT16                                *Structure;
  UINT8                                 *DmiStrucBuffer;
  SMBIOS_STRUCTURE                      *Location;
  UINTN                                 Size;
  BOOLEAN                               IsSmbios32BitTable = FALSE;

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) < 0x3) || 
      (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT0) == BIT0))) {
    IsSmbios32BitTable = TRUE;
  } else {
    IsSmbios32BitTable = FALSE;
  }

  LocateSmbiosTable ();

  Structure      = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x51_FRAME *)Frame)->Structure);
  DmiStrucBuffer = (UINT8 *)PnpFarToLinear (((PNP_FUNCTION_0x51_FRAME *)Frame)->DmiStrucBuffer);
  Location = (SMBIOS_STRUCTURE *)DmiStrucBuffer;
  if (BufferOverlapSmramPnp ((VOID *) Structure, sizeof(UINT16))) {
     return DMI_BAD_PARAMETER;
  }
  if (!EFI_ERROR (LocateSmbiosStructure (IsSmbios32BitTable, Structure, &Location, &Size))) {
    if (BufferOverlapSmramPnp ((VOID *) DmiStrucBuffer, Size)) {
      return DMI_BAD_PARAMETER;
    }
    CopyMem (DmiStrucBuffer, (VOID *)Location, Size);
    return DMI_SUCCESS;
  } else {
    return DMI_INVALID_HANDLE;
  }
}

/**
  PnP function 0x52, Set SMBIOS Structure.

  @param[In]      Frame                 Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS           The function was executed successfully.
  @retval     DMI_BAD_PARAMETER     Invalid command.

**/
INT16
Pnp0x52 (
  IN VOID            *Frame
  )
{
  FUNC_0x52_DATA_BUFFER                 *Data;
  SMBIOS_STRUCTURE                      *StructureLocation;
  SMBIOS_STRUCTURE                      *StructureLocation64Bit;
  UINT16                                StructureHandle;
  UINT16                                StructureHandle64Bit;
  UINTN                                 StructureSize;
  UINTN                                 StructureSize64Bit;
  INT16                                 ReturnStatus;
  INT16                                 ReturnStatus64Bit;  
  EFI_STATUS                            Status;
  BOOLEAN                               IsSmbios32BitTable;
  BOOLEAN                               IsSmbios64BitTable;

  IsSmbios32BitTable = FALSE;
  IsSmbios64BitTable = FALSE;
  ReturnStatus = DMI_SUCCESS;
  ReturnStatus64Bit = DMI_SUCCESS;
  StructureLocation  = NULL;
  StructureLocation64Bit  = NULL;

  Data = (FUNC_0x52_DATA_BUFFER *)PnpFarToLinear (((PNP_FUNCTION_0x52_FRAME *)Frame)->DmiDataBuffer);
  if (BufferOverlapSmramPnp ((VOID *) Data, sizeof(FUNC_0x52_DATA_BUFFER))) {
    return DMI_BAD_PARAMETER;
  }
  StructureLocation = NULL;
  StructureHandle = Data->StructureHeader.Handle;

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) < 0x3) || 
      (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT0) == BIT0))) {
        IsSmbios32BitTable = TRUE;  
  }

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT1) == BIT1)) {
    IsSmbios64BitTable = TRUE;
  }

  StructureHandle64Bit = StructureHandle;

  if (IsSmbios32BitTable) {
    Status = LocateSmbiosStructure (IsSmbios32BitTable, &StructureHandle, &StructureLocation, &StructureSize);
    if(EFI_ERROR(Status)){
      return ReturnStatus;
    }
  }

  if (IsSmbios64BitTable) {
    Status = LocateSmbiosStructure (FALSE, &StructureHandle64Bit, &StructureLocation64Bit, &StructureSize64Bit);
    if(EFI_ERROR(Status)){
      return ReturnStatus;
    }
  }


  switch (Data->Command) {

  case DMI_BYTE_CHANGE:
  case DMI_WORD_CHANGE:
  case DMI_DWORD_CHANGE:
    if (IsSmbios32BitTable){
      ReturnStatus = Pnp0x52ChangeFixedLength (Data, StructureLocation);
    }

    if (IsSmbios64BitTable){
      ReturnStatus64Bit = Pnp0x52ChangeFixedLength (Data, StructureLocation64Bit);
    }
    break;

  case DMI_STRING_CHANGE:
    if (IsSmbios32BitTable){
      ReturnStatus = Pnp0x52ChangeString (IsSmbios32BitTable, Data, StructureLocation);
    }

    if (IsSmbios64BitTable){
      ReturnStatus64Bit = Pnp0x52ChangeString (FALSE, Data, StructureLocation64Bit);
    }
    break;

  case DMI_BLOCK_CHANGE:
    if (IsSmbios32BitTable){
      ReturnStatus = Pnp0x52ChangeBlock (Data, StructureLocation);
    }

    if (IsSmbios64BitTable){
      ReturnStatus64Bit = Pnp0x52ChangeBlock (Data, StructureLocation64Bit);
    }
    break;

  default:
    ReturnStatus = DMI_BAD_PARAMETER;
    break;
  }

  ReturnStatus = EFI_ERROR (ReturnStatus64Bit) ? ReturnStatus64Bit : ReturnStatus;  

  return ReturnStatus;
}

/**
  default function returning that the call was to an unsupported function.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.(unused)

  @retval     DMI_SUCCESS                 The function was executed successfully.
  @retval     DMI_INVALID_SUBFUNCTION     Invalid command.

**/
INT16
UnsupportedPnpFunction (
  IN VOID               *Frame
  )
{
  return PNP_FUNCTION_NOT_SUPPORTED;
}

/**
  PnP function 0x54.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS     The function was executed successfully.

**/
INT16
Pnp0x54 (
  IN VOID            *Frame
  )
{
  INT16                                SubFunction;
  UINT8                                *Data;
  UINT8                                Control;
  UINTN                                Index;

  SubFunction = (UINT16)((PNP_FUNCTION_0x54_FRAME *)Frame)->SubFunction;
  Data = (UINT8 *)PnpFarToLinear (((PNP_FUNCTION_0x54_FRAME *)Frame)->Data);
  if (BufferOverlapSmramPnp ((VOID *) Data, sizeof(UINT8))) {
    return DMI_BAD_PARAMETER;
  }
  Control = (UINT8)((PNP_FUNCTION_0x54_FRAME *)Frame)->Control;

  if ((Control & 0x01) == 0) {
    if (SubFunction >= 1) {
      return DMI_INVALID_SUBFUNCTION;
    }
    return DMI_SUCCESS;
  }

  switch (SubFunction) {

  case 0:
    //
    // for event log
    //
    if (!mIsOemGPNVMap) {
      return DMI_FUNCTION_NOT_SUPPORTED;
    }
    for (Index = 0; Index < OemGPNVHandleCount; Index++) {
      ClearBiosEventLog (GPNVMapBuffer.PlatFormGPNVMap[Index].BaseAddress, GPNVMapBuffer.PlatFormGPNVMap[Index].GPNVSize);
    }
    break;

  case 1:
    return DMI_INVALID_SUBFUNCTION;
    break;

  case 2:
    return DMI_INVALID_SUBFUNCTION;
    break;

  default:
    return DMI_INVALID_SUBFUNCTION;
    break;
  }

  return DMI_SUCCESS;
}

/**
  PnP function 0x55, Set SMBIOS Structure.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS            The function was executed successfully.
  @retval     DMI_INVALID_HANDLE     Invalid handle.

**/
INT16
Pnp0x55 (
  IN VOID            *Frame
  )
{
  UINT16                                *Handle;
  UINT16                                HandleTemp;
  UINT16                                *MinGPNVRWSize;
  UINT16                                *GPNVSize;
  UINT32                                *NVStorageBase;
  UINT16                                Index;


  Handle = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x55_FRAME *)Frame)->Handle);
  if (BufferOverlapSmramPnp ((VOID *) Handle, sizeof(UINT16))) {
    return DMI_BAD_PARAMETER;
  }

  Index = HandleTemp = *Handle;

  if (!VaildGPNVCheck (&Index, HandleTemp)) {
    return  DMI_INVALID_HANDLE;
  }

  MinGPNVRWSize = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x55_FRAME *)Frame)->MinGPNVRWSize);
  if (BufferOverlapSmramPnp ((VOID *) MinGPNVRWSize, sizeof(UINT16))) {
    return DMI_BAD_PARAMETER;
  }
  *MinGPNVRWSize = (UINT16)GPNVMapBuffer.PlatFormGPNVMap[Index].MinGPNVSize;

  GPNVSize = (UINT16 *)PnpFarToLinear (((PNP_FUNCTION_0x55_FRAME *)Frame)->GPNVSize);
  if (BufferOverlapSmramPnp ((VOID *) GPNVSize, sizeof(UINT16))) {
    return DMI_BAD_PARAMETER;
  }
  *GPNVSize = (UINT16)GPNVMapBuffer.PlatFormGPNVMap[Index].GPNVSize;

  NVStorageBase = (UINT32 *)PnpFarToLinear (((PNP_FUNCTION_0x55_FRAME *)Frame)->NVStorageBase);
  if (BufferOverlapSmramPnp ((VOID *) NVStorageBase, sizeof(UINT32))) {
    return DMI_BAD_PARAMETER;
  }
  *NVStorageBase = (UINT32)GPNVMapBuffer.PlatFormGPNVMap[Index].BaseAddress;

  Index++;
  if (Index >= FixedPcdGet16 (PcdDefaultGpnvMapBufferSize)) {
    return DMI_INVALID_HANDLE;
  }

  *Handle = GPNVMapBuffer.PlatFormGPNVMap[Index].Handle;

  return DMI_SUCCESS;
}

/**
  PnP function 0x56, Set SMBIOS Structure.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS            The function was executed successfully.
  @retval     DMI_INVALID_HANDLE     Invalid handle.

**/
INT16
Pnp0x56 (
  IN VOID            *Frame
  )
{
  UINT16                                Handle;
  UINT8                                 *GPNVBuffer;
  UINT16                                Index;


  Handle = (UINT16)((PNP_FUNCTION_0x56_FRAME *)Frame)->Handle;
  GPNVBuffer = (UINT8 *)PnpFarToLinear (((PNP_FUNCTION_0x56_FRAME *)Frame)->GPNVBuffer);

  Index = Handle;
  if (!VaildGPNVCheck (&Index, Handle)) {
    return DMI_INVALID_HANDLE;
  }

  if (BufferOverlapSmramPnp ((VOID *) GPNVBuffer, GPNVMapBuffer.PlatFormGPNVMap[Index].GPNVSize)) {
    return DMI_BAD_PARAMETER;
  }
 
  CopyMem (
    GPNVBuffer,
    (VOID *)(UINTN)GPNVMapBuffer.PlatFormGPNVMap[Index].BaseAddress,
    GPNVMapBuffer.PlatFormGPNVMap[Index].GPNVSize
    );

  return DMI_SUCCESS;
}

/**
  PnP function 0x57, Set SMBIOS Structure.

  @param[In]      Frame           Pointer to input stack frame of the PnP call.

  @retval     DMI_SUCCESS            The function was executed successfully.
  @retval     DMI_INVALID_HANDLE     Invalid handle.

**/
INT16
Pnp0x57 (
  IN VOID            *Frame
  )
{
  UINT16                                Handle;
  UINT8                                 *GPNVBuffer;
  UINT16                                Index;


  Handle = (UINT16)((PNP_FUNCTION_0x57_FRAME *)Frame)->Handle;
  GPNVBuffer = (UINT8 *)PnpFarToLinear (((PNP_FUNCTION_0x57_FRAME *)Frame)->GPNVBuffer);

  Index = Handle;
  if (! VaildGPNVCheck (&Index, Handle)) {
    return DMI_INVALID_HANDLE;
  }

  if (BufferOverlapSmramPnp ((VOID *) GPNVBuffer, GPNVMapBuffer.PlatFormGPNVMap[Index].GPNVSize)) {
    return DMI_BAD_PARAMETER;
  }
  WriteGPNV (Index, GPNVBuffer);

  return DMI_SUCCESS;
}

VOID
PnpRedirectedEntryPoint (
  IN OUT EFI_DWORD_REGS                 *RegBufferPtr
  )
{
  PNP_GENERIC_ENTRY_FRAME               *Frame;
  INT16                                 ReturnStatus;
  UINT32                                CR3Value;
  UINT32                                CR4Value;


  GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RBX, mCpuIndex, &CR3Value);
  GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RCX, mCpuIndex, &CR4Value);

  Frame = (PNP_GENERIC_ENTRY_FRAME *)(UINTN)LogicalToPhysicalAddress (
                                              (UINT32)RegBufferPtr->ESP,
                                              CR3Value,
                                              CR4Value,
                                              (UINTN)RegBufferPtr->SS
                                              );

  if ((Frame->Function & ~0x7) == 0x50) {
    ReturnStatus = mPnpDmiFunctions[Frame->Function & 0x7]((VOID *)Frame);
  } else {
    ReturnStatus = PNP_FUNCTION_NOT_SUPPORTED;
  }

  *((INT16 *)&(RegBufferPtr->EAX)) = ReturnStatus;
}

/**
  This fucntion is triggered by SMI function call 0x47.

  @param[In]      DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[In]      Context         Points to an optional handler context which was specified when the
                                  handler was registered.
  @param[In]      CommBuffer      A pointer to a collection of data in memory that will
                                  be conveyed from a non-SMM environment into an SMM environment.
  @param[In]      CommBufferSize  The size of the CommBuffer.

  @retval     EFI_SUCCESS              The callabck was handled successfully.
  @retval     EFI_OUT_OF_RESOURCES     Cannot locate SmmCpuProtocol.
  @retval     EFI_NOT_FOUND            Cannot determine the source of SMI.

**/
EFI_STATUS
EFIAPI
PnPBiosCallback (
  IN  EFI_HANDLE                   DispatchHandle,
  IN  CONST  VOID                  *DispatchContext,
  IN  OUT  VOID                    *CommBuffer,
  IN  OUT  UINTN                   *CommBufferSize
  )
{
  EFI_DWORD_REGS                        *RegBufferPtr;
  UINT32                                CR3Value;
  UINT32                                CR4Value;
  UINTN                                 SegementSelector;
  UINTN                                 Index;
  UINTN                                 CPUFoundIndex;
  PNP_GENERIC_ENTRY_FRAME               *Frame;
  INT16                                 ReturnStatus;
  EFI_STATUS                            Status;
  UINT32                                Eax;
  UINT32                                Edx;
  UINT32                                Ebx;
  UINT32                                Ecx;
  UINT32                                Esi;
  UINT32                                Ss;

  //
  // Locate Smm Cpu protocol for Cpu save state manipulation
  //
  Status = mSmst->SmmLocateProtocol (
                    &gEfiSmmCpuProtocolGuid,
                    NULL,
                    (VOID **)&mSmmCpu
                    );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  CPUFoundIndex = mSmst->NumberOfCpus;
  for (Index = 0; Index < mSmst->NumberOfCpus; Index++) {
    Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RAX, Index, &Eax);
    Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RDX, Index, &Edx);
    //
    // Find out which CPU triggered PnP SMI with use Edx, this is first priority.
    //
    if (((Eax & 0xff) == SMM_PnP_BIOS_CALL) && ((Edx & 0xffff) == PcdGet16 (PcdSoftwareSmiPort))) {
      //
      // Cpu found!
      //
      CPUFoundIndex = Index;
      break;
    }
    //
    // Re-check again for SMI trigger without use Edx, this is second priority.
    // if cpu found, continue to look the first priority at next CPU.
    //
    if ((Eax & 0xff) == SMM_PnP_BIOS_CALL) {
      //
      // Cpu found!
      //
      CPUFoundIndex = Index;
    }
  }
  if (CPUFoundIndex == mSmst->NumberOfCpus) {
    //
    // Error out due to CPU not found
    //
    return EFI_NOT_FOUND;
  }
  mCpuIndex = CPUFoundIndex;

  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RBX, mCpuIndex, &Ecx);
  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RBX, mCpuIndex, &Ebx);
  Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RSI, mCpuIndex, &Esi);
  if (Ebx == SIGNATURE_32 ('$', 'I', 'S', 'B')) {
    //
    // Call by new way that AP trigger software SMI directly when the signature is "$ISB'
    //
    Frame = (PNP_GENERIC_ENTRY_FRAME *)(UINTN)Esi;

    if ((Frame->Function & ~0x7) == 0x50) {
      ReturnStatus = mPnpDmiFunctions[Frame->Function & 0x7](Frame);
    } else {
      ReturnStatus = PNP_FUNCTION_NOT_SUPPORTED;
    }

    Eax = (UINT32)ReturnStatus;
    Status = SetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_RAX, mCpuIndex, &Eax);
  } else {
    Status = GetDwordRegisterByCpuIndex (EFI_SMM_SAVE_STATE_REGISTER_SS, mCpuIndex, &Ss);
    //
    // Call by csm16
    //
    CR3Value = Ebx;
    CR4Value = Ecx;
    SegementSelector = (UINTN)Ss;
    RegBufferPtr = (EFI_DWORD_REGS *)(UINTN)LogicalToPhysicalAddress (Esi, CR3Value, CR4Value, SegementSelector);
    PnpRedirectedEntryPoint (RegBufferPtr);
  }

  return EFI_SUCCESS;
}

/**
  This function is SMM Communication call back for SMBIOS PNP functions.  

  @param[in]     DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     Context         Points to an optional handler context which was specified when the
                                 handler was registered.
  @param[in,out] CommBuffer      A pointer to a collection of data in memory that will
                                 be conveyed from a non-SMM environment into an SMM environment.
  @param[in,out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS            The interrupt was handled and quiesced. No other handlers 
                                 should still be called.
**/
EFI_STATUS
EFIAPI
SmmSmbiosPnpHandler (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  CONST VOID                        *RegisterContext,
  IN  OUT VOID                          *CommBuffer,
  IN  OUT UINTN                         *CommBufferSize
  )
{
  SMM_SMBIOS_PNP_COMMUNICATE_HEADER     *SmmSmbiosPnpHeader;
  SMM_SMBIOS_PNP_ADDRESS                *SmmSmbiosPnpEntry;

  SmmSmbiosPnpHeader = (SMM_SMBIOS_PNP_COMMUNICATE_HEADER *) CommBuffer; 
  SmmSmbiosPnpEntry  = (SMM_SMBIOS_PNP_ADDRESS *) SmmSmbiosPnpHeader->Data;

  switch (SmmSmbiosPnpHeader->Function) {
  case SMM_COMM_SMBIOS_PNP_ENTRY_SET:
    if (mSMBIOSTableEntryAddress == 0) {
      mSMBIOSTableEntryAddress = (EFI_PHYSICAL_ADDRESS) SmmSmbiosPnpEntry->Address;
    }

    if (mSMBIOSTableEntryAddress64Bit == 0) {
      mSMBIOSTableEntryAddress64Bit = (EFI_PHYSICAL_ADDRESS) SmmSmbiosPnpEntry->Address64Bit;
    }

    break;

  default:
    break;  
  }

  return EFI_SUCCESS;
}

/**
  Clear BIOS event log data from GPNV area.

  @param[In]  GPNVBase               GPNV Start address
  @param[In]  GPNVLength             GPNV Length

  @retval     DMI_SUCCESS            The function was executed successfully.
  @retval     DMI_INVALID_HANDLE     Invalid handle.

**/
STATIC
EFI_STATUS
ClearBiosEventLog (
  IN UINTN                              GPNVBase,
  IN UINTN                              GPNVLength
  )
{
  EFI_STATUS                            Status;
  UINT8                                 *Buffer;
  UINT32                                Index;
  UINTN                                 BlockSize;
  UINTN                                 FdSupportEraseSize;
  UINTN                                 EraseCount;
  UINTN                                 EraseStartAddress;

  FdSupportEraseSize = GetFlashBlockSize();
  EraseStartAddress = GPNVBase & (~(FdSupportEraseSize - 1));
  EraseCount = GET_ERASE_SECTOR_NUM (
                                 GPNVBase,
                                 GPNVLength,
                                 EraseStartAddress,
                                 FdSupportEraseSize
                                 );

  BlockSize = FdSupportEraseSize * EraseCount;

  Status = mSmst->SmmAllocatePool(
                    EfiRuntimeServicesData,
                    BlockSize,
                    (VOID **)&Buffer
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  //Copy whole block data to buffer
  //
  CopyMem(Buffer, (VOID *)(UINTN)EraseStartAddress, BlockSize);

  //
  //Copy modified GPNV data to buffer
  //
  for (Index = 0; Index < GPNVLength; Index++) {
    *(UINT8 *)((Buffer + (GPNVBase) - (GPNVBase & ((UINT32)(~0)) & (~(FdSupportEraseSize - 1))))+Index) = 0xFF;
  }

  //
  //Flash GPNV
  //
  for (Index = 0; Index < EraseCount; Index++) {

    Status = FlashErase (
                         EraseStartAddress + FdSupportEraseSize * Index,
                         FdSupportEraseSize
                         );
  }

  Status = FlashProgram (
                        (UINT8 *)EraseStartAddress,
                        Buffer,
                        &BlockSize,
                        EraseStartAddress
                        );

  Status = mSmst->SmmFreePool (Buffer);

  return Status;
}

