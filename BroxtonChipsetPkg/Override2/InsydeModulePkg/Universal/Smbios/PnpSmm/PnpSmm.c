/** @file
  PnpSmm driver initialization

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
#include <Library/IoLib.h>
#include <Guid/FlashMapHob.h>
#include <EfiFlashMap.h>             // for GPNV
#include <Library/MemoryAllocationLib.h>

#define FixedPcdGetPtrSize(TokenName)      (UINTN)_PCD_PATCHABLE_##TokenName##_SIZE


//[-start-181001-IB07401020-modify]//
UINT8                                   *SmmSMBIOSPnpBuffer = NULL;
//[-end-181001-IB07401020-modify]//


EFI_SMM_SYSTEM_TABLE2                   *mSmst;
EFI_SMM_FW_BLOCK_SERVICE_PROTOCOL       *mSmmFwBlockService;
H2O_IHISI_PROTOCOL                      *mH2OIhisiPnp = NULL;
PLATFORM_GPNV_MAP_BUFFER                GPNVMapBuffer;
PLATFORM_GPNV_HEADER                    *GPNVHeader = NULL;
UINTN                                   UpdateableStringCount;
DMI_UPDATABLE_STRING                    *mUpdatableStrings;
UINTN                                   OemGPNVHandleCount = 0;
BOOLEAN                                 mIsOemGPNVMap = FALSE;

typedef struct {
  UINT8        Type;
  BOOLEAN      Updatable;
  UINTN        StringNum;
  UINT8        StringOffset[1];
} MODIFY_UPDATABLE_INFO;

/**

  Count number of strings in a string array.

  @param[IN]  StringArray    A string contain a string array.
                             Each element must end by ';'

  @retval     Integer        The number of strings. If StringArray is NULL will return zero.

**/
STATIC
UINTN
ParseStringArray (
  IN   CHAR8 *StringArray
  )
{
  CHAR8   *Ptr;
  UINTN    StrCount;

  if (StringArray == NULL) {
    return 0;
  }

  StrCount = 0;
  Ptr = StringArray;
  while(*Ptr != '\0') {
    if (*Ptr == ';') {
      ++StrCount;
    }
    ++Ptr;
  }

  return StrCount;
}


/**
  This callback fucntion gets the SMBIOS table entry point and pass into SmmSmbiosPnpHandler.

  @param[In]  Event       Event type
  @param[In]  Context     Context for the event

**/
VOID
EFIAPI
GetSmbiosEntryCallback (
  IN EFI_EVENT                              Event,
  IN VOID                                   *Cotext
  )
{
  EFI_STATUS                                Status;
  EFI_SMM_COMMUNICATE_HEADER                *SmmCommunicateBufHeader;
  SMM_SMBIOS_PNP_COMMUNICATE_HEADER         *SmmSMBIOSPnpHeader;
  EFI_SMM_COMMUNICATION_PROTOCOL            *SmmCommunication;
  EFI_PHYSICAL_ADDRESS                      SMBIOSTableEntryAddress;
  EFI_PHYSICAL_ADDRESS                      SMBIOSTableEntryAddress64Bit;
  SMM_SMBIOS_PNP_ADDRESS                    *SmmSMBIOSPnpAddress;
  UINTN                                     CommSize;
//[-start-181001-IB07401020-remove]//
//  UINT8                                     SmmSMBIOSPnpBuffer[SMM_SMBIOS_PNP_COMM_BUFF_SIZE];
//[-end-181001-IB07401020-remove]//
 
  gBS->CloseEvent (Event);

//[-start-181001-IB07401020-add]//
  if (SmmSMBIOSPnpBuffer == NULL){
    Status = gBS->AllocatePool (
                    EfiReservedMemoryType,
                    SMM_SMBIOS_PNP_COMM_BUFF_SIZE,
                    (VOID**)&SmmSMBIOSPnpBuffer
                    );
    if (EFI_ERROR(Status)){
      return;
    }
  }
//[-end-181001-IB07401020-add]//

  //
  // Initial Communication buffer
  //
  SmmCommunicateBufHeader = (EFI_SMM_COMMUNICATE_HEADER *) SmmSMBIOSPnpBuffer;
  CopyGuid (&SmmCommunicateBufHeader->HeaderGuid, &gH2OSmmPnPCommunicationGuid);
  SmmCommunicateBufHeader->MessageLength = (SMM_SMBIOS_PNP_COMM_BUFF_SIZE - OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data));
  SmmSMBIOSPnpHeader = (SMM_SMBIOS_PNP_COMMUNICATE_HEADER *) SmmCommunicateBufHeader->Data;
  SmmSMBIOSPnpHeader->Function = SMM_COMM_SMBIOS_PNP_ENTRY_SET;
  SmmSMBIOSPnpAddress = (SMM_SMBIOS_PNP_ADDRESS *)SmmSMBIOSPnpHeader->Data;
  
  //
  // Need to pass SMBIOS table entry point into SmmSmbiosPnpHandler. 
  //
  Status = EFI_SUCCESS;
  SMBIOSTableEntryAddress = 0;
  SMBIOSTableEntryAddress64Bit = 0;

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) < 0x3) || 
      (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT0) == BIT0))) {
    Status = EfiGetSystemConfigurationTable (&gEfiSmbiosTableGuid, (VOID **) &SMBIOSTableEntryAddress);
  }

  ASSERT_EFI_ERROR (Status);

  if (((PcdGet16 (PcdSmbiosVersion) >> 8) >= 0x3) && ((PcdGet32 (PcdSmbiosEntryPointProvideMethod) & BIT1) == BIT1)) {
    Status = EfiGetSystemConfigurationTable (&gEfiSmbios3TableGuid, (VOID **) &SMBIOSTableEntryAddress64Bit);
  }

  ASSERT_EFI_ERROR (Status);
  SmmSMBIOSPnpAddress->Address = SMBIOSTableEntryAddress;
  SmmSMBIOSPnpAddress->Address64Bit = SMBIOSTableEntryAddress64Bit;

  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **) &SmmCommunication);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }

  CommSize = SMM_SMBIOS_PNP_COMM_BUFF_SIZE;
  Status = SmmCommunication->Communicate (SmmCommunication, SmmSMBIOSPnpBuffer, &CommSize);
  ASSERT_EFI_ERROR (Status);
}

EFI_STATUS
FillPlatformGPNVMapBuffer (
  IN OEM_GPNV_MAP       *GPNVMap
  )
{
  UINTN               Index;

  for (Index = 0; Index < OemGPNVHandleCount; Index++) {
    GPNVMapBuffer.PlatFormGPNVMap[Index].Handle      = GPNVMap->Handle;
    GPNVMapBuffer.PlatFormGPNVMap[Index].MinGPNVSize = GPNVMap->GPNVSize;
    GPNVMapBuffer.PlatFormGPNVMap[Index].GPNVSize    = GPNVMap->GPNVSize;
    GPNVMapBuffer.PlatFormGPNVMap[Index].BaseAddress = GPNVMap->GPNVBaseAddress;
    ++GPNVMap;
  }

  return EFI_SUCCESS;
}


/**
  Entry point for SmmPnp drivers.

  @param[In]  ImageHandle           EFI_HANDLE
  @param[In]  SystemTable           EFI_SYSTEM_TABLE

  @retval EFI_SUCCESS           Success
  @retval EFI_DEVICE_ERROR      Fail

**/
EFI_STATUS
EFIAPI
SmmPnpInit (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            SwHandle;
  EFI_SMM_SW_DISPATCH2_PROTOCOL        *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT           SwContext;
  EFI_EVENT                             SmbiosGetTableEvent;
  DMI_UPDATABLE_STRING                 *TempStringTable;
  OEM_GPNV_MAP                         *OemGPNVMap;
  EFI_SMM_BASE2_PROTOCOL                *SmmBase;
  BOOLEAN                               InSmm;
  BOOLEAN                               NeedFreePnpTable;
  CHAR8                                *RecordStrings;
  UINTN                                 RecordLength;
  STRING_COUNT_TABLE                   *CountTable;
  UINTN                                 CountTableSize;
  UINTN                                 StrCount;
  UINTN                                 TempPnpTableCount;
  UINTN                                 StrIndex;
  UINTN                                 PnpTableIndex;
  MODIFY_UPDATABLE_INFO               **ModifyTable = NULL;
  EFI_PCD_PROTOCOL                     *Pcd = NULL;
  UINTN                                 CountTableIndex = 0;
  EFI_HANDLE                            DispatchHandle;

  SmmBase = NULL;
  TempStringTable = NULL;
  Status = gBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, (VOID **)&SmmBase);
  InSmm = FALSE;
  if (!EFI_ERROR (Status)) {
    SmmBase->InSmm (SmmBase, &InSmm);
  }

  if (InSmm) {
    Status = SmmBase->GetSmstLocation (SmmBase, &mSmst);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = mSmst->SmmLocateProtocol (&gEfiSmmFwBlockServiceProtocolGuid, NULL, (VOID **)&mSmmFwBlockService);

    UpdateableStringCount = 0;
    mUpdatableStrings     = NULL;

    //
    // OemServices (Smm)
    //
    NeedFreePnpTable = TRUE;
    Status = OemSvcSmmInstallPnpStringTable (
               &UpdateableStringCount,
               &TempStringTable
               );

    if (Status == EFI_UNSUPPORTED) {
      Status = gBS->LocateProtocol (&gEfiPcdProtocolGuid, NULL, (VOID **) &Pcd);
      if (EFI_ERROR(Status)) {
        return Status;
      }

      TempStringTable = PcdGetPtr (PcdSmbiosUpdatableStringTable);
      UpdateableStringCount = FixedPcdGetPtrSize (PcdSmbiosUpdatableStringTable) / sizeof (DMI_UPDATABLE_STRING);
      TempPnpTableCount = UpdateableStringCount;
      CountTable = (STRING_COUNT_TABLE *)Pcd->GetPtr (&gSmbiosTokenSpaceGuid, PcdToken (PcdSmbiosStringCountFieldOffset));
      CountTableSize = Pcd->GetSize (&gSmbiosTokenSpaceGuid, PcdToken (PcdSmbiosStringCountFieldOffset)) / sizeof (STRING_COUNT_TABLE);

      ModifyTable = AllocateZeroPool (sizeof (ModifyTable) * CountTableSize);
      if (ModifyTable == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      TempPnpTableCount += CountTableSize;
      for (CountTableIndex = 0; CountTableIndex < CountTableSize; CountTableIndex++) {
        RecordLength  = Pcd->GetSize (&gSmbiosTokenSpaceGuid, PcdToken (PcdType000Record) + CountTable[CountTableIndex].Type);
        RecordStrings = Pcd->GetPtr (&gSmbiosTokenSpaceGuid, PcdToken (PcdType000Strings) + CountTable[CountTableIndex].Type);
        StrCount = ParseStringArray (RecordStrings);
        TempPnpTableCount += StrCount;
        ModifyTable[CountTableIndex] = AllocateZeroPool (sizeof (MODIFY_UPDATABLE_INFO) + StrCount);
        if (ModifyTable[CountTableIndex] == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        ModifyTable[CountTableIndex]->Type = CountTable[CountTableIndex].Type;
        ModifyTable[CountTableIndex]->StringNum = StrCount;
        ModifyTable[CountTableIndex]->Updatable = FALSE;
        ModifyTable[CountTableIndex]->StringOffset[0] = CountTable[CountTableIndex].StringCountField;
        for (PnpTableIndex = 0; PnpTableIndex < UpdateableStringCount; PnpTableIndex++) {
          if (TempStringTable[PnpTableIndex].Type == CountTable[CountTableIndex].Type &&
            (TempStringTable[PnpTableIndex].FixedOffset == CountTable[CountTableIndex].StringCountField || TempStringTable[PnpTableIndex].FixedOffset >= RecordLength)) {
            ModifyTable[CountTableIndex]->Updatable = TRUE;
            for (StrIndex = 0; StrIndex < StrCount; StrIndex++) {
              ModifyTable[CountTableIndex]->StringOffset[StrIndex + 1] = (UINT8)(RecordLength + StrIndex);
            }
            break;
          }
        }
      }

      TempStringTable = AllocateZeroPool (TempPnpTableCount * sizeof (DMI_UPDATABLE_STRING));
      if (TempStringTable == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
      CopyMem (TempStringTable,PcdGetPtr (PcdSmbiosUpdatableStringTable),FixedPcdGetPtrSize (PcdSmbiosUpdatableStringTable));
      TempPnpTableCount = UpdateableStringCount;

      for (CountTableIndex = 0; CountTableIndex < CountTableSize; CountTableIndex++) {
        if (ModifyTable[CountTableIndex]->Updatable) {
          for (StrIndex = 0; StrIndex <= ModifyTable[CountTableIndex]->StringNum; StrIndex++) {
            for (PnpTableIndex = 0; PnpTableIndex < TempPnpTableCount; PnpTableIndex++) {
              if (TempStringTable[PnpTableIndex].Type == ModifyTable[CountTableIndex]->Type &&
                TempStringTable[PnpTableIndex].FixedOffset == ModifyTable[CountTableIndex]->StringOffset[StrIndex]) {
                break;
              }
            }
              if (PnpTableIndex == TempPnpTableCount) {
                TempStringTable[UpdateableStringCount].Type = ModifyTable[CountTableIndex]->Type;
                TempStringTable[UpdateableStringCount].FixedOffset = ModifyTable[CountTableIndex]->StringOffset[StrIndex];
                UpdateableStringCount++;
              }
            }
          }
        FreePool (ModifyTable[CountTableIndex]);
      }
      FreePool (ModifyTable);
    }

    if ((TempStringTable != NULL) && (UpdateableStringCount != 0)) {
      Status = mSmst->SmmAllocatePool (
                        EfiRuntimeServicesData,
                        UpdateableStringCount * sizeof (DMI_UPDATABLE_STRING),
                        (VOID **)&mUpdatableStrings
                        );
      if (!EFI_ERROR (Status)) {
        CopyMem (mUpdatableStrings, TempStringTable, UpdateableStringCount * sizeof (DMI_UPDATABLE_STRING));
      }
      if (NeedFreePnpTable) {
        FreePool (TempStringTable);
      }
    }

    //
    // OemServices (Smm)
    //
    OemGPNVHandleCount = 0;
    OemGPNVMap = NULL;
    Status = OemSvcSmmInstallPnpGpnvTable (&OemGPNVHandleCount, &OemGPNVMap);
    if (Status == EFI_MEDIA_CHANGED && OemGPNVMap != NULL) {
      FillPlatformGPNVMapBuffer (OemGPNVMap);
      mIsOemGPNVMap = TRUE;
    } else {
      mIsOemGPNVMap = FALSE;
      }

    //
    // Get the Sw dispatch protocol
    //
    Status = mSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID **)&SwDispatch);
    ASSERT_EFI_ERROR (Status);

    //
    // Register SMBIOS call SMI function
    //
    SwContext.SwSmiInputValue = SMM_PnP_BIOS_CALL;
    SwHandle = NULL;
    Status = SwDispatch->Register (SwDispatch, PnPBiosCallback, &SwContext, &SwHandle);
    ASSERT_EFI_ERROR (Status);

    //
    // Register SMBIOS PnP SMI Communication handler
    //
    Status = mSmst->SmiHandlerRegister (
                      SmmSmbiosPnpHandler,
                      &gH2OSmmPnPCommunicationGuid,
                      &DispatchHandle
                    );
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // This event will be triggered after SMBIOS table entry point is ready.
    // So, we can get the address of entry point and save it.
    //
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK - 1,
               GetSmbiosEntryCallback,
               NULL,
               &SmbiosGetTableEvent
               );
  }

 return EFI_SUCCESS;
}

/**
  Check the input memory buffer is whether overlap the SMRAM ranges.

  @param[in] Buffer       The pointer to the buffer to be checked.
  @param[in] BufferSize   The size in bytes of the input buffer

  @retval  TURE        The buffer overlaps SMRAM ranges.
  @retval  FALSE       The buffer doesn't overlap SMRAM ranges.
**/
BOOLEAN
EFIAPI
BufferOverlapSmramPnp (
  IN VOID              *Buffer,
  IN UINTN              BufferSize
  )
{
  EFI_STATUS                                Status; 
  if (mH2OIhisiPnp == NULL) {
    Status = mSmst->SmmLocateProtocol (
                      &gH2OIhisiProtocolGuid,
                      NULL,
                      (VOID **) &mH2OIhisiPnp
                      );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return TRUE;
    }
  }
  if (mH2OIhisiPnp->BufferOverlapSmram((VOID *) Buffer, BufferSize)) {
    return TRUE;
  }
  return FALSE;
}

