/** @file
  Misc. function in HDD Password Protocol

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#include "HddPassword.h"

EFI_SMM_SYSTEM_TABLE2                   *mSmst2;

extern HDD_PASSWORD_PRIVATE             *mHddPasswordPrivate;
extern DRIVER_INSTALL_INFO              *mDriverInstallInfo;

/**
  According Bus, Device, Function, PrimarySecondary, SlaveMaster to get corresponding
  SATA port number

  @param[in]        Bus                 PCI bus number
  @param[in]        Device              PCI device number
  @param[in]        Function            PCI function number
  @param[in]        PrimarySecondary    primary or scondary
  @param[in]        SlaveMaster         slave or master
  @param[out]       PortNum             output port number

  @retval           EFI_SUCCESS         Get corresponding port number successfully
  @retval           EFI_NOT_FOUND       Can't get corresponding port number
**/
EFI_STATUS
ChangeChannelDevice2PortNum (
  IN     UINT32                         Bus,
  IN     UINT32                         Device,
  IN     UINT32                         Function,
  IN     UINT8                          PrimarySecondary,
  IN     UINT8                          SlaveMaster,
  OUT    UINTN                          *PortNum
  )
{
  UINTN                                 Index;
  PORT_NUMBER_MAP                       *PortMappingTable;
  PORT_NUMBER_MAP                       EndEntry;
  UINTN                                 NoPorts;

  if (PortNum == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  NoPorts = 0;
  PortMappingTable   = NULL;

  ZeroMem (&EndEntry, sizeof (PORT_NUMBER_MAP));

  PortMappingTable = (PORT_NUMBER_MAP *)PcdGetPtr (PcdPortNumberMapTable);

  while (CompareMem (&EndEntry, &PortMappingTable[NoPorts], sizeof (PORT_NUMBER_MAP)) != 0) {
    NoPorts++;
  }

  if (NoPorts == 0) {
    return EFI_NOT_FOUND;
  }

  for (Index = 0; Index < NoPorts; Index++) {
    if ((PortMappingTable[Index].Bus == Bus) &&
        (PortMappingTable[Index].Device == Device) &&
        (PortMappingTable[Index].Function == Function) &&
        (PortMappingTable[Index].PrimarySecondary == PrimarySecondary) &&
        (PortMappingTable[Index].SlaveMaster == SlaveMaster)) {
      *PortNum = PortMappingTable[Index].PortNum;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

/**
  According the Bus, Device, Function to check this controller is in Port Number Map table or not.
  If yes, then this is a on board PCI device.

  @param[in]        Bus                 PCI bus number
  @param[in]        Device              PCI device number
  @param[in]        Function            PCI function number

  @retval           TRUE                Is on board device
  @retval           FALSE               Not on board device
**/
BOOLEAN
IsOnBoardPciDevice (
  IN     UINT32                         Bus,
  IN     UINT32                         Device,
  IN     UINT32                         Function
  )
{
  UINTN                                 Index;
  PORT_NUMBER_MAP                       *PortMappingTable;
  PORT_NUMBER_MAP                       EndEntry;
  UINTN                                 NumOfPorts;

  PortMappingTable      = NULL;

  ZeroMem (&EndEntry, sizeof (PORT_NUMBER_MAP));

  PortMappingTable = (PORT_NUMBER_MAP *)PcdGetPtr (PcdPortNumberMapTable);

  NumOfPorts = 0;
  while (CompareMem (&EndEntry, &PortMappingTable[NumOfPorts], sizeof (PORT_NUMBER_MAP)) != 0) {
    NumOfPorts++;
  }

  if (NumOfPorts == 0) {
    return FALSE;
  }

  for (Index = 0; Index < NumOfPorts; Index++) {
    if ((PortMappingTable[Index].Bus == Bus) &&
        (PortMappingTable[Index].Device == Device) &&
        (PortMappingTable[Index].Function == Function)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Get memory space from allocated reserved pages.
  This function is ONLY used in Hdd Password driver due to not allocated more memory when not enough.

  @param[in]        AllocSize           how many size to be allocated
  @param[out]       AllocAddr           where to be allocated

  @retval EFI_SUCCESS
**/
EFI_STATUS
GetMem (
  IN  UINTN                             RequestSize,
  OUT  VOID                             **AllocAddr
  )
{
  if (AllocAddr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (mHddPasswordPrivate->MemRecord->RemainSize == 0 ||
      mHddPasswordPrivate->MemRecord->RemainSize < RequestSize) {
    return EFI_OUT_OF_RESOURCES;
  }

  *AllocAddr = *(VOID **)&(mHddPasswordPrivate->MemRecord->RemainMemAddr);

  mHddPasswordPrivate->MemRecord->RemainMemAddr += ALIGN_SIZEOF_UINTN (RequestSize);
  mHddPasswordPrivate->MemRecord->RemainSize    -= ALIGN_SIZEOF_UINTN (RequestSize);

  return EFI_SUCCESS;
}

/**
  Copy SourceBufferPtr string to DescBufferPtr.

  @param[in]        DescBufferPtr       Destination buffer address.
  @param[in]        SourceBufferPtr     Source buffer addess.

  @retval           EFI_SUCCESS
**/
EFI_STATUS
GetModelNumber (
  IN VOID                               *DescBufferPtr,
  IN VOID                               *SourceBufferPtr
  )
{
  UINT8                                 *IdentifyPtr;
  UINT16                                *HddInfoPtr;
  UINT16                                Index;
  UINT32                                *SwapPtr;
  UINTN                                 DwordConunt;

  IdentifyPtr = SourceBufferPtr;
  HddInfoPtr  = DescBufferPtr;
  SwapPtr     = DescBufferPtr;

  for (Index = 0; Index < MODEL_NUMBER_LENGTH; Index++) {
    HddInfoPtr[Index] = (UINT16) (IdentifyPtr[Index] & 0x0ff);
  }

  //
  // MAX_SUPPORT_SERIAL_NUM_COUNT is Serial Count.
  // It's unit is CHAR16.
  //
  DwordConunt = MODEL_NUMBER_LENGTH >> 1;
  for (Index = 0; Index < DwordConunt; Index++) {
    SwapPtr[Index] = (UINT32) (((SwapPtr[Index] & 0xffff0000) >> 16) + \
                               ((SwapPtr[Index] & 0x0000ffff) << 16));
  }
  return EFI_SUCCESS;
}

/**
 Get all Storage controller PCI Class to connect.

 @retval EFI_SUCCESS  Connect SATA controller successfully.
 @retval Other        Connect SATA controller failed.
**/
EFI_STATUS
EFIAPI
GetStorageControllerPciClass (
  OUT SATA_CLASS_CODE                  **ClassCodeBuffer,
  OUT UINTN                            *ClassCodeCount
  )
{
  EFI_STATUS                            Status;

  UINTN                                 AdapterInfoHandleCount;
  EFI_HANDLE                            *AdapterInfoHandleBuffer;
  UINTN                                 AdapterInfoHandleIndex;

  EFI_ADAPTER_INFORMATION_PROTOCOL      *AdapterInfo;
  VOID                                  *InformationBlock;
  UINTN                                 InformationBlockSize;
  UINTN                                 InformationBlockCount;

  SATA_CLASS_CODE                       *ClassCodeArray;
  UINTN                                 ClassCodeIndex;
  BOOLEAN                               DuplicateClassCodeFound;

  SATA_CLASS_CODE                       *InternalClassCodeBuffer;
  UINTN                                 InternalClassCodeCount;

  LIST_ENTRY                            ControllerClassInfoListHead;
  ATA_CONTROLLER_INFO                   *ControllerClassInfo;
  LIST_ENTRY                            *Link;



  InitializeListHead (&ControllerClassInfoListHead);

  AdapterInfoHandleCount  = 0;
  AdapterInfoHandleBuffer = NULL;
  DuplicateClassCodeFound = FALSE;
  InternalClassCodeCount  = 0;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiAdapterInformationProtocolGuid,
                  NULL,
                  &AdapterInfoHandleCount,
                  &AdapterInfoHandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    *ClassCodeBuffer = NULL;
    *ClassCodeCount  = 0;
    return Status;
  }


  for (AdapterInfoHandleIndex = 0; AdapterInfoHandleIndex < AdapterInfoHandleCount; AdapterInfoHandleIndex++) {

    Status = gBS->HandleProtocol (
                    AdapterInfoHandleBuffer[AdapterInfoHandleIndex],
                    &gEfiAdapterInformationProtocolGuid,
                    (VOID **)&AdapterInfo
                    );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    InformationBlock = NULL;
    InformationBlockSize = 0;

    Status = AdapterInfo->GetInformation (
                            AdapterInfo,
                            &gEfiHddPasswordServiceProtocolGuid,
                            &InformationBlock,
                            &InformationBlockSize
                            );
    if (Status != EFI_SUCCESS || InformationBlock == NULL || InformationBlockSize == 0) {
      continue;
    }

    InformationBlockCount = InformationBlockSize / sizeof (SATA_CLASS_CODE);

    ClassCodeArray = (SATA_CLASS_CODE *)InformationBlock;

    for (ClassCodeIndex = 0; ClassCodeIndex < InformationBlockCount; ClassCodeIndex++) {
      Link = GetFirstNode (&ControllerClassInfoListHead);

      //
      // if the first node is null, skip checking duplicated class code
      //
      while (!IsNull(&ControllerClassInfoListHead, Link)) {
        ControllerClassInfo = (ATA_CONTROLLER_INFO *)Link;
        DuplicateClassCodeFound = FALSE;
	    //
        // if the classinfo is already exist in the linked list, break from the do-while loop 
        // and skip adding the node to the linked list
        //
        if (ControllerClassInfo->BaseCode     == ClassCodeArray[ClassCodeIndex].BaseCode &&
            ControllerClassInfo->SubClassCode == ClassCodeArray[ClassCodeIndex].SubClassCode &&
            ControllerClassInfo->PI           == ClassCodeArray[ClassCodeIndex].PI) {
          DuplicateClassCodeFound = TRUE;
          break;
        }
        Link = GetNextNode (&ControllerClassInfoListHead, Link);
      } 
      
      if (DuplicateClassCodeFound == FALSE) {
        ControllerClassInfo = NULL;
        ControllerClassInfo = (ATA_CONTROLLER_INFO *)AllocateZeroPool (sizeof (ATA_CONTROLLER_INFO));

        if (ControllerClassInfo == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }

        ControllerClassInfo->BaseCode     = ClassCodeArray[ClassCodeIndex].BaseCode;
        ControllerClassInfo->SubClassCode = ClassCodeArray[ClassCodeIndex].SubClassCode;
        ControllerClassInfo->PI           = ClassCodeArray[ClassCodeIndex].PI;

        InsertTailList (
          &ControllerClassInfoListHead,
          &ControllerClassInfo->Link
          );

        InternalClassCodeCount++;

      }
    }
  }

  InternalClassCodeBuffer = NULL;
  InternalClassCodeBuffer = (SATA_CLASS_CODE *)AllocateZeroPool (sizeof (SATA_CLASS_CODE) * InternalClassCodeCount);

  if (InternalClassCodeBuffer == NULL) {
    gBS->FreePool (AdapterInfoHandleBuffer);
    return EFI_OUT_OF_RESOURCES;
  }

  ControllerClassInfo = (ATA_CONTROLLER_INFO *)GetFirstNode (&ControllerClassInfoListHead);
  for (ClassCodeIndex = 0; ClassCodeIndex < InternalClassCodeCount; ClassCodeIndex++) {

    InternalClassCodeBuffer[ClassCodeIndex].BaseCode     = ControllerClassInfo->BaseCode;
    InternalClassCodeBuffer[ClassCodeIndex].SubClassCode = ControllerClassInfo->SubClassCode;
    InternalClassCodeBuffer[ClassCodeIndex].PI           = ControllerClassInfo->PI;

    ControllerClassInfo = (ATA_CONTROLLER_INFO *)GetNextNode (
                                                   &ControllerClassInfoListHead,
                                                   &ControllerClassInfo->Link
                                                   );
  }


  gBS->FreePool (AdapterInfoHandleBuffer);

  *ClassCodeBuffer = InternalClassCodeBuffer;
  *ClassCodeCount  = InternalClassCodeCount;

  return EFI_SUCCESS;
}

/**
 Connect all Storage controller.

 @retval EFI_SUCCESS  Connect SATA controller successfully.
 @retval Other        Connect SATA controller failed.
**/
EFI_STATUS
EFIAPI
CollectStorageController (
  IN BOOLEAN                            NeedConnectController
  )
{
  EFI_STATUS                            Status;
  UINTN                                 PciIoHandleCount;
  EFI_HANDLE                            *PciIoHandleBuffer;
  UINTN                                 PciIoHandleIndex;
  EFI_PCI_IO_PROTOCOL                   *PciIo;
  UINTN                                 Device;
  UINTN                                 Seg;
  UINTN                                 Bus;
  UINTN                                 Function;
  UINT8                                 SataClassCReg[3];

  BOOLEAN                               SpecificControllerFound;

  ATA_CONTROLLER_INFO                   *AtaControllerInfo;
  DRIVER_INSTALL_INFO                   *DriverInstallInfo;

  SATA_CLASS_CODE                       *ClassCodeBuffer;
  UINTN                                 ClassCodeCount;
  UINTN                                 ClassCodeIndex;

  PciIoHandleCount  = 0;
  PciIoHandleBuffer = NULL;

  DriverInstallInfo = mDriverInstallInfo;

  ClassCodeCount  = 0;
  ClassCodeBuffer = NULL;

  Status = GetStorageControllerPciClass (
             &ClassCodeBuffer,
             &ClassCodeCount
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &PciIoHandleCount,
                  &PciIoHandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InitializeListHead (&DriverInstallInfo->AtaControllerInfoListHead);

  for (PciIoHandleIndex = 0; PciIoHandleIndex < PciIoHandleCount; PciIoHandleIndex++) {
    SpecificControllerFound = FALSE;

    Status = gBS->HandleProtocol (
                    PciIoHandleBuffer[PciIoHandleIndex],
                    &gEfiPciIoProtocolGuid,
                    (VOID **)&PciIo
                    );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    Status = PciIo->GetLocation (
                      PciIo,
                      &Seg,
                      &Bus,
                      &Device,
                      &Function
                      );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    Status = PciIo->Pci.Read (
                          PciIo,
                          EfiPciIoWidthUint8,
                          PCI_CLASSCODE_OFFSET,
                          3,
                          SataClassCReg
                          );
    if (Status != EFI_SUCCESS) {
      continue;
    }

    for (ClassCodeIndex = 0; ClassCodeIndex < ClassCodeCount; ClassCodeIndex++) {
      //
      // Test whether the controller belongs to SD HOST type
      //
      if (SataClassCReg[2] == ClassCodeBuffer[ClassCodeIndex].BaseCode &&
          SataClassCReg[1] == ClassCodeBuffer[ClassCodeIndex].SubClassCode) {
         SpecificControllerFound = TRUE;
         break;
      }
    }


    if (SpecificControllerFound == TRUE) {
      if (NeedConnectController == TRUE) {
        Status = gBS->ConnectController (
                        PciIoHandleBuffer[PciIoHandleIndex],
                        NULL,
                        NULL,
                        TRUE
                        );
      }

      AtaControllerInfo = NULL;

      AtaControllerInfo = AllocateZeroPool(sizeof (ATA_CONTROLLER_INFO));

      if (AtaControllerInfo != NULL) {
        AtaControllerInfo->AtaControllerHandle = PciIoHandleBuffer[PciIoHandleIndex];
        AtaControllerInfo->AtaMode             = SataClassCReg[1];
        AtaControllerInfo->PciSeg              = Seg;
        AtaControllerInfo->PciBus              = Bus;
        AtaControllerInfo->PciDevice           = Device;
        AtaControllerInfo->PciFunction         = Function;
        AtaControllerInfo->BaseCode            = SataClassCReg[2];
        AtaControllerInfo->SubClassCode        = SataClassCReg[1];
        AtaControllerInfo->PI                  = SataClassCReg[0];

        InsertTailList (
          &DriverInstallInfo->AtaControllerInfoListHead,
          &AtaControllerInfo->Link
          );

        DriverInstallInfo->NumOfController++;
      }

    }

  }

  gBS->FreePool (PciIoHandleBuffer);

  DriverInstallInfo->AtaControllerSearched = TRUE;

  return Status;
}

/**
 Find mapped controller.

 @retval Others        found.
 @retval 0xFFFF        not found.
**/
UINT16
EFIAPI
FindMappedController (
  IN UINTN                                 Seg,
  IN UINTN                                 Bus,
  IN UINTN                                 Device,
  IN UINTN                                 Function
  )
{
  ATA_CONTROLLER_INFO                   *AtaControllerInfo;
  DRIVER_INSTALL_INFO                   *DriverInstallInfo;
  UINT16                                ControllerIndex;
  LIST_ENTRY                            *AtaControllerInfoListHead;
  LIST_ENTRY                            *Link;

  DriverInstallInfo = mDriverInstallInfo;

  AtaControllerInfoListHead = &DriverInstallInfo->AtaControllerInfoListHead;
  for (Link = GetFirstNode (AtaControllerInfoListHead), ControllerIndex = 0;
       !IsNull (AtaControllerInfoListHead, Link) && ControllerIndex < DriverInstallInfo->NumOfController;
       Link = GetNextNode (AtaControllerInfoListHead, Link), ControllerIndex++) {
    AtaControllerInfo = (ATA_CONTROLLER_INFO *)Link;
    if (AtaControllerInfo->PciSeg == Seg &&
        AtaControllerInfo->PciBus == Bus &&
        AtaControllerInfo->PciDevice == Device &&
        AtaControllerInfo->PciFunction == Function) {
      return ControllerIndex;
    }

  }

  return 0xFFFF;
}

