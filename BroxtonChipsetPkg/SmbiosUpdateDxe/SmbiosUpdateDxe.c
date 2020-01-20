/** @file
  SmbiosUpdateDxe.c 

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


#include <SmbiosUpdateDxe.h>

EFI_STATUS
SmbiosType20 (
  IN   EFI_SMBIOS_PROTOCOL        *Smbios
  )
{
  //
  // SMBIOS_TABLE_TYPE20
  //
  SMBIOS_TABLE_TYPE20             *Type20Record;
  UINT8                            Type20RecordSize;
  EFI_STATUS                       Status;
  EFI_SMBIOS_HANDLE                SmbiosHandle;
  MEM_INFO_PROTOCOL               *MemInfoHob;
  UINT8                            ChannelASlotMap;
  UINT8                            ChannelBSlotMap;
  UINT8                            Dimm;
  UINT8                            DimmIndex;
  BOOLEAN                          SlotPresent;
  UINT8                            MaxSockets;
  UINT64                           TotalMemorySizeInKB;
  UINT16                          *Record;
  UINTN                            HandleCount;
  UINT8                            BitIndex;
  UINT8                            ChannelASlotNum;
  UINT8                            ChannelBSlotNum;

  Type20Record        = NULL;
  Type20RecordSize    = sizeof(SMBIOS_TABLE_TYPE20);

  MemInfoHob          = NULL;
  DimmIndex           = 0;
  MaxSockets          = 0;
  TotalMemorySizeInKB = 0;
  HandleCount         = 0;
  BitIndex            = 0;

  Status = gBS->LocateProtocol (&gMemInfoProtocolGuid, NULL, (VOID **) &MemInfoHob);

//[-start-160218-IB08450338-modify]//
  ChannelASlotMap = 0x03;
  ChannelBSlotMap = 0x00;
//[-end-160218-IB08450338-modify]//
  ChannelASlotNum = 0;
  ChannelBSlotNum = 0;
  for (BitIndex = 0; BitIndex < 8; BitIndex++) {
    if ((ChannelASlotMap >> BitIndex) & BIT0) {
      ChannelASlotNum++;
    }
    if ((ChannelBSlotMap >> BitIndex) & BIT0) {
      ChannelBSlotNum++;
    }
  }
  MaxSockets  = ChannelASlotNum + ChannelBSlotNum;

  for (Dimm = 0; Dimm < MAX_SOCKETS; Dimm++) {
    ///
    /// Use channel slot map to check whether the Socket is supported in this SKU, some SKU only has 2 Sockets totally
    ///
    SlotPresent = FALSE;
    if (Dimm < 2) {
      if (ChannelASlotMap & (1 << Dimm)) {
        SlotPresent = TRUE;
      }
    } else {
      if (ChannelBSlotMap & (1 << (Dimm - 2))) {
        SlotPresent = TRUE;
      }
    }
    if (!SlotPresent) {
      continue;
    }

    Type20Record = (SMBIOS_TABLE_TYPE20 *) AllocateZeroPool (Type20RecordSize + 2);   //add double 0x00 at the end
    ASSERT (Type20Record != NULL);
    if (Type20Record == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

//[-start-170524-IB07400869-modify]//
    if (SlotPresent && MemInfoHob->MemInfoData.DimmPresent[Dimm]) {
      Type20Record->Hdr.Type                 = 20;
      Type20Record->Hdr.Length               = Type20RecordSize;
      Type20Record->StartingAddress          = 0;

      Type20Record->PartitionRowPosition = 0xFF;
      GetLinkTypeHandle ( EFI_SMBIOS_TYPE_MEMORY_DEVICE, &Record,&HandleCount);
      ASSERT (Record != NULL);
      if(Record == NULL) {
        return EFI_NOT_FOUND;
      }
      if (DimmIndex < HandleCount){
        Type20Record->MemoryDeviceHandle = Record[DimmIndex];
      }
      GetLinkTypeHandle(EFI_SMBIOS_TYPE_MEMORY_ARRAY_MAPPED_ADDRESS,&Record,&HandleCount);
      ASSERT (Record != NULL);
      if(Record == NULL) {
        return EFI_NOT_FOUND;
      }

      Type20Record->MemoryArrayMappedAddressHandle = Record[0]; //Record->Handle;
      SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;

      TotalMemorySizeInKB = LShiftU64 (MemInfoHob->MemInfoData.dimmSize[Dimm], 10);
      if (TotalMemorySizeInKB > 0xFFFFFFFF) {
        Type20Record->StartingAddress = 0xFFFFFFFF;
        Type20Record->EndingAddress = 0xFFFFFFFF;
        Type20Record->ExtendedEndingAddress = TotalMemorySizeInKB - 1;
      } else {
        Type20Record->EndingAddress = (UINT32) (TotalMemorySizeInKB - 1);
      }
      Type20Record->InterleavePosition    = (UINT8)(1 << (Dimm >= ( MaxSockets >> 1)));
      Type20Record->InterleavedDataDepth  = (UINT8)(1);
      
      Status = Smbios->Add (Smbios, NULL, &SmbiosHandle,  (EFI_SMBIOS_TABLE_HEADER*) Type20Record);
      DimmIndex++;
    }
//[-end-170524-IB07400869-modify]//
  }

  return EFI_SUCCESS;
}


/**
  Create an event to install a smbios type 14 (Group associations) data for the Intel firmware version info.

  @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
  @param [in]   SystemTable      Pointer to the EFI System Table

  @retval Status                 EFI_SUCCESS
  @return Assert, otherwise.

**/
EFI_STATUS
EFIAPI
SmbiosUpdateInit (
  IN      EFI_HANDLE              ImageHandle,
  IN      EFI_SYSTEM_TABLE        *SystemTable
  )
{
  EFI_STATUS                 Status;
  EFI_SMBIOS_PROTOCOL        *Smbios;

  Smbios           = NULL;

  //
  // Find the SMBIOS Protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **) &Smbios);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Locate SmbiosProtocol failed in CreateFviGroupCallBack event.\n"));
    return EFI_NOT_FOUND;
  }

  //
  // Create Smbios Type 20 information
  //
  Status = SmbiosType20(Smbios);

  return EFI_SUCCESS;
}
