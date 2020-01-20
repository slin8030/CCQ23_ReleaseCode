/** @file

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "LegacyBiosInterface.h"
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SmBios.h>
#include <Library/BaseMemoryLib.h>

#define BOOT_LEGACY_OS              0
#define BOOT_EFI_OS                 1
#define BOOT_UNCONVENTIONAL_DEVICE  2

UINT32              mLoadOptionsSize    = 0;
UINTN               mBootMode           = BOOT_LEGACY_OS;
VOID                *mLoadOptions       = NULL;
BBS_BBS_DEVICE_PATH *mBbsDevicePathPtr  = NULL;
BBS_BBS_DEVICE_PATH mBbsDevicePathNode;
UDC_ATTRIBUTES      mAttributes         = { 0, 0, 0, 0 };
UINTN               mBbsEntry           = 0;
VOID                *mBeerData          = NULL;
VOID                *mServiceAreaData   = NULL;
UINT64              mLowWater           = 0xffffffffffffffffULL;

extern BBS_TABLE           *mBbsTable;

/**
  Print the BBS Table.

  @param BbsTable   The BBS table.


**/
VOID
PrintBbsTable (
  IN BBS_TABLE *BbsTable
  )
{
  UINT16 Index;
  UINT16 SubIndex;
  CHAR8  *String;

  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, " NO  Prio bb/dd/ff cl/sc Type Stat segm:offs mfgs:mfgo dess:deso\n"));
  DEBUG ((EFI_D_INFO, "=================================================================\n"));
  for (Index = 0; Index < MAX_BBS_ENTRIES; Index++) {
    //
    // Filter
    //
    if (BbsTable[Index].BootPriority == BBS_IGNORE_ENTRY) {
      continue;
    }

    DEBUG ((
      EFI_D_INFO,
      " %02x: %04x %02x/%02x/%02x %02x/%02x %04x %04x",
      (UINTN) Index,
      (UINTN) BbsTable[Index].BootPriority,
      (UINTN) BbsTable[Index].Bus,
      (UINTN) BbsTable[Index].Device,
      (UINTN) BbsTable[Index].Function,
      (UINTN) BbsTable[Index].Class,
      (UINTN) BbsTable[Index].SubClass,
      (UINTN) BbsTable[Index].DeviceType,
      (UINTN) * (UINT16 *) &BbsTable[Index].StatusFlags
      ));
    DEBUG ((
      EFI_D_INFO,
      " %04x:%04x %04x:%04x %04x:%04x",
      (UINTN) BbsTable[Index].BootHandlerSegment,
      (UINTN) BbsTable[Index].BootHandlerOffset,
      (UINTN) BbsTable[Index].MfgStringSegment,
      (UINTN) BbsTable[Index].MfgStringOffset,
      (UINTN) BbsTable[Index].DescStringSegment,
      (UINTN) BbsTable[Index].DescStringOffset
      ));

    //
    // Print DescString
    //
    String = (CHAR8 *)(UINTN)((BbsTable[Index].DescStringSegment << 4) + BbsTable[Index].DescStringOffset);
    if (String != NULL) {
      DEBUG ((EFI_D_INFO," ("));
      for (SubIndex = 0; String[SubIndex] != 0; SubIndex++) {
        DEBUG ((EFI_D_INFO, "%c", String[SubIndex]));
      }
      DEBUG ((EFI_D_INFO,")"));
    }
    DEBUG ((EFI_D_INFO,"\n"));
  }

  DEBUG ((EFI_D_INFO, "\n"));

  return ;
}

/**
  Print the BBS Table.

  @param HddInfo   The HddInfo table.


**/
VOID
PrintHddInfo (
  IN HDD_INFO *HddInfo
  )
{
  UINTN Index;

  DEBUG ((EFI_D_INFO, "\n"));
  for (Index = 0; Index < MAX_IDE_CONTROLLER; Index++) {
    DEBUG ((EFI_D_INFO, "Index - %04x\n", Index));
    DEBUG ((EFI_D_INFO, "  Status    - %04x\n", (UINTN)HddInfo[Index].Status));
    DEBUG ((EFI_D_INFO, "  B/D/F     - %02x/%02x/%02x\n", (UINTN)HddInfo[Index].Bus, (UINTN)HddInfo[Index].Device, (UINTN)HddInfo[Index].Function));
    DEBUG ((EFI_D_INFO, "  Command   - %04x\n", HddInfo[Index].CommandBaseAddress));
    DEBUG ((EFI_D_INFO, "  Control   - %04x\n", HddInfo[Index].ControlBaseAddress));
    DEBUG ((EFI_D_INFO, "  BusMaster - %04x\n", HddInfo[Index].BusMasterAddress));
    DEBUG ((EFI_D_INFO, "  HddIrq    - %02x\n", HddInfo[Index].HddIrq));
    DEBUG ((EFI_D_INFO, "  IdentifyDrive[0].Raw[0] - %x\n", HddInfo[Index].IdentifyDrive[0].Raw[0]));
    DEBUG ((EFI_D_INFO, "  IdentifyDrive[1].Raw[0] - %x\n", HddInfo[Index].IdentifyDrive[1].Raw[0]));
  }

  DEBUG ((EFI_D_INFO, "\n"));

  return ;
}

/**
  Print the PCI Interrupt Line and Interrupt Pin registers.
**/
VOID
PrintPciInterruptRegister (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINTN                       Index;
  EFI_HANDLE                  *Handles;
  UINTN                       HandleNum;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  UINT8                       Interrupt[2];
  UINTN                       Segment;
  UINTN                       Bus;
  UINTN                       Device;
  UINTN                       Function;

  gBS->LocateHandleBuffer (
         ByProtocol,
         &gEfiPciIoProtocolGuid,
         NULL,
         &HandleNum,
         &Handles
         );

  Bus      = 0;
  Device   = 0;
  Function = 0;

  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, " bb/dd/ff interrupt line interrupt pin\n"));
  DEBUG ((EFI_D_INFO, "======================================\n"));
  for (Index = 0; Index < HandleNum; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &gEfiPciIoProtocolGuid, (VOID **) &PciIo);
    if (!EFI_ERROR (Status)) {
      Status = PciIo->Pci.Read (
                            PciIo,
                            EfiPciIoWidthUint8,
                            PCI_INT_LINE_OFFSET,
                            2,
                            Interrupt
                            );
    }
    if (!EFI_ERROR (Status)) {
      Status = PciIo->GetLocation (
                        PciIo,
                        &Segment,
                        &Bus,
                        &Device,
                        &Function
                        );
    }
    if (!EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, " %02x/%02x/%02x 0x%02x           0x%02x\n",
              Bus, Device, Function, Interrupt[0], Interrupt[1]));
    }
  }
  DEBUG ((EFI_D_INFO, "\n"));

  if (Handles != NULL) {
    FreePool (Handles);
  }
}

/**
  Identify drive data must be updated to actual parameters before boot.

  @param  IdentifyDriveData       ATA Identify Data

**/
VOID
UpdateIdentifyDriveData (
  IN  UINT8     *IdentifyDriveData
  );

/**
  Update SIO data.

  @param  Private                 Legacy BIOS Instance data

  @retval EFI_SUCCESS             Removable media not present

**/
EFI_STATUS
UpdateSioData (
  IN  LEGACY_BIOS_INSTANCE      *Private
  )
{
  EFI_STATUS                          Status;
  UINTN                               Index;
  UINTN                               Index1;
  UINT8                               LegacyInterrupts[16];
  EFI_LEGACY_IRQ_ROUTING_ENTRY        *RoutingTable;
  UINTN                               RoutingTableEntries;
  EFI_LEGACY_IRQ_PRIORITY_TABLE_ENTRY *IrqPriorityTable;
  UINTN                               NumberPriorityEntries;
  EFI_TO_COMPATIBILITY16_BOOT_TABLE   *EfiToLegacy16BootTable;
  UINT8                               HddIrq;
  UINT16                              LegacyInt;
  UINT16                              LegMask;
  UINT32                              Register;
  UINTN                               HandleCount;
  EFI_HANDLE                          *HandleBuffer;
  EFI_ISA_IO_PROTOCOL                 *IsaIo;

  LegacyInt               = 0;
  HandleBuffer            = NULL;

  EfiToLegacy16BootTable  = &Private->IntThunk->EfiToLegacy16BootTable;
  LegacyBiosBuildSioData (Private);
  SetMem (LegacyInterrupts, sizeof (LegacyInterrupts), 0);

  //
  // Create list of legacy interrupts.
  //
  for (Index = 0; Index < 4; Index++) {
    LegacyInterrupts[Index] = EfiToLegacy16BootTable->SioData.Serial[Index].Irq;
  }

  for (Index = 4; Index < 7; Index++) {
    LegacyInterrupts[Index] = EfiToLegacy16BootTable->SioData.Parallel[Index - 4].Irq;
  }

  LegacyInterrupts[7] = EfiToLegacy16BootTable->SioData.Floppy.Irq;

  //
  // Get Legacy Hdd IRQs. If native mode treat as PCI
  //
  for (Index = 0; Index < 2; Index++) {
    HddIrq = EfiToLegacy16BootTable->HddInfo[Index].HddIrq;
    if ((HddIrq != 0) && ((HddIrq == 15) || (HddIrq == 14))) {
      LegacyInterrupts[Index + 8] = HddIrq;
    }
  }

  Private->LegacyBiosPlatform->GetRoutingTable (
                                Private->LegacyBiosPlatform,
                                (VOID *) &RoutingTable,
                                &RoutingTableEntries,
                                NULL,
                                NULL,
                                (VOID **) &IrqPriorityTable,
                                &NumberPriorityEntries
                                );
  //
  // Remove legacy interrupts from the list of PCI interrupts available.
  //
  for (Index = 0; Index <= 0x0b; Index++) {
    for (Index1 = 0; Index1 <= NumberPriorityEntries; Index1++) {
      if (LegacyInterrupts[Index] != 0) {
        LegacyInt = (UINT16) (LegacyInt | (1 << LegacyInterrupts[Index]));
        if (LegacyInterrupts[Index] == IrqPriorityTable[Index1].Irq) {
          IrqPriorityTable[Index1].Used = LEGACY_USED;
        }
      }
    }
  }

  Private->Legacy8259->GetMask (
                        Private->Legacy8259,
                        &LegMask,
                        NULL,
                        NULL,
                        NULL
                        );

  //
  // Set SIO interrupts and disable mouse. Let mouse driver
  // re-enable it.
  //
  LegMask = (UINT16) ((LegMask &~LegacyInt) | 0x1000);
  Private->Legacy8259->SetMask (
                        Private->Legacy8259,
                        &LegMask,
                        NULL,
                        NULL,
                        NULL
                        );

  //
  // Disable mouse in keyboard controller
  //
  Register = 0xA7;
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiIsaIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiIsaIoProtocolGuid,
                    (VOID **) &IsaIo
                    );
    ASSERT_EFI_ERROR (Status);
    IsaIo->Io.Write (IsaIo, EfiIsaIoWidthUint8, 0x64, 1, &Register);

  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

  return EFI_SUCCESS;

}

/**
  Identify drive data must be updated to actual parameters before boot.
  This requires updating the checksum, if it exists.

  @param  IdentifyDriveData       ATA Identify Data
  @param  Checksum                checksum of the ATA Identify Data

  @retval EFI_SUCCESS             checksum calculated
  @retval EFI_SECURITY_VIOLATION  IdentifyData invalid

**/
EFI_STATUS
CalculateIdentifyDriveChecksum (
  IN  UINT8     *IdentifyDriveData,
  OUT UINT8     *Checksum
  )
{
  UINTN Index;
  UINT8 LocalChecksum;
  LocalChecksum = 0;
  *Checksum     = 0;
  if (IdentifyDriveData[510] != 0xA5) {
    return EFI_SECURITY_VIOLATION;
  }

  for (Index = 0; Index < 512; Index++) {
    LocalChecksum = (UINT8) (LocalChecksum + IdentifyDriveData[Index]);
  }

  *Checksum = LocalChecksum;
  return EFI_SUCCESS;
}


/**
  Identify drive data must be updated to actual parameters before boot.

  @param  IdentifyDriveData       ATA Identify Data


**/
VOID
UpdateIdentifyDriveData (
  IN  UINT8     *IdentifyDriveData
  )
{
  UINT16          NumberCylinders;
  UINT16          NumberHeads;
  UINT16          NumberSectorsTrack;
  UINT32          CapacityInSectors;
  UINT8           OriginalChecksum;
  UINT8           FinalChecksum;
  EFI_STATUS      Status;
  ATAPI_IDENTIFY  *ReadInfo;

  //
  // Status indicates if Integrity byte is correct. Checksum should be
  // 0 if valid.
  //
  ReadInfo  = (ATAPI_IDENTIFY *) IdentifyDriveData;
  Status    = CalculateIdentifyDriveChecksum (IdentifyDriveData, &OriginalChecksum);
  if (OriginalChecksum != 0) {
    Status = EFI_SECURITY_VIOLATION;
  }
  //
  // If NumberCylinders = 0 then do data(Controller present but don drive attached).
  //
  NumberCylinders = ReadInfo->Raw[1];
  if (NumberCylinders != 0) {
    ReadInfo->Raw[54]   = NumberCylinders;

    NumberHeads         = ReadInfo->Raw[3];
    ReadInfo->Raw[55]   = NumberHeads;

    NumberSectorsTrack  = ReadInfo->Raw[6];
    ReadInfo->Raw[56]   = NumberSectorsTrack;

    //
    // Copy Multisector info and set valid bit.
    //
    ReadInfo->Raw[59] = (UINT16) (ReadInfo->Raw[47] + 0x100);
    CapacityInSectors = (UINT32) ((UINT32) (NumberCylinders) * (UINT32) (NumberHeads) * (UINT32) (NumberSectorsTrack));
    ReadInfo->Raw[57] = (UINT16) (CapacityInSectors >> 16);
    ReadInfo->Raw[58] = (UINT16) (CapacityInSectors & 0xffff);
    if (Status == EFI_SUCCESS) {
      //
      // Forece checksum byte to 0 and get new checksum.
      //
      ReadInfo->Raw[255] &= 0xff;
      CalculateIdentifyDriveChecksum (IdentifyDriveData, &FinalChecksum);

      //
      // Force new checksum such that sum is 0.
      //
      FinalChecksum = (UINT8) ((UINT8)0 - FinalChecksum);
      ReadInfo->Raw[255] = (UINT16) (ReadInfo->Raw[255] | (FinalChecksum << 8));
    }
  }
}

/**
  Identify drive data must be updated to actual parameters before boot.
  Do for all drives.

  @param  Private                 Legacy BIOS Instance data


**/
VOID
UpdateAllIdentifyDriveData (
  IN LEGACY_BIOS_INSTANCE                 *Private
  )
{
  UINTN     Index;
  HDD_INFO  *HddInfo;

  HddInfo = &Private->IntThunk->EfiToLegacy16BootTable.HddInfo[0];

  for (Index = 0; Index < MAX_IDE_CONTROLLER; Index++) {
    //
    // Each controller can have 2 devices. Update for each device
    //
    if ((HddInfo[Index].Status & HDD_MASTER_IDE) != 0) {
      UpdateIdentifyDriveData ((UINT8 *) (&HddInfo[Index].IdentifyDrive[0].Raw[0]));
    }

    if ((HddInfo[Index].Status & HDD_SLAVE_IDE) != 0) {
      UpdateIdentifyDriveData ((UINT8 *) (&HddInfo[Index].IdentifyDrive[1].Raw[0]));
    }
  }
}

/**
  Enable ide controller.  This gets disabled when LegacyBoot.c is about
  to run the Option ROMs.

  @param  Private        Legacy BIOS Instance data


**/
VOID
EnableIdeController (
  IN LEGACY_BIOS_INSTANCE              *Private
  )
{
  EFI_PCI_IO_PROTOCOL *PciIo;
  EFI_STATUS          Status;
  EFI_HANDLE          IdeController;
  UINT8               ByteBuffer;
  UINTN               HandleCount;
  EFI_HANDLE          *HandleBuffer;

  Status = Private->LegacyBiosPlatform->GetPlatformHandle (
                                          Private->LegacyBiosPlatform,
                                          EfiGetPlatformIdeHandle,
                                          0,
                                          &HandleBuffer,
                                          &HandleCount,
                                          NULL
                                          );
  if (!EFI_ERROR (Status)) {
    IdeController = HandleBuffer[0];
    Status = gBS->HandleProtocol (
                    IdeController,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    ByteBuffer = 0x1f;
    if (!EFI_ERROR (Status)) {
      PciIo->Pci.Write (PciIo, EfiPciIoWidthUint8, 0x04, 1, &ByteBuffer);
    }
  }
}


/**
  Enable ide controller.  This gets disabled when LegacyBoot.c is about
  to run the Option ROMs.

  @param  Private                 Legacy BIOS Instance data


**/
VOID
EnableAllControllers (
  IN LEGACY_BIOS_INSTANCE              *Private
  )
{
  UINTN               HandleCount;
  EFI_HANDLE          *HandleBuffer;
  UINTN               Index;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE01          PciConfigHeader;
  EFI_STATUS          Status;

  //
  //
  //
  EnableIdeController (Private);

  //
  // Assumption is table is built from low bus to high bus numbers.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo
                    );
    ASSERT_EFI_ERROR (Status);

    PciIo->Pci.Read (
                PciIo,
                EfiPciIoWidthUint32,
                0,
                sizeof (PciConfigHeader) / sizeof (UINT32),
                &PciConfigHeader
                );

    //
    // We do not enable PPB here. This is for HotPlug Consideration.
    // The Platform HotPlug Driver is responsible for Padding enough hot plug
    // resources. It is also responsible for enable this bridge. If it
    // does not pad it. It will cause some early Windows fail to installation.
    // If the platform driver does not pad resource for PPB, PPB should be in
    // un-enabled state to let Windows know that this PPB is not configured by
    // BIOS. So Windows will allocate default resource for PPB.
    //
    // The reason for why we enable the command register is:
    // The CSM will use the IO bar to detect some IRQ status, if the command
    // is disabled, the IO resource will be out of scope.
    // For example:
    // We installed a legacy IRQ handle for a PCI IDE controller. When IRQ
    // comes up, the handle will check the IO space to identify is the
    // controller generated the IRQ source.
    // If the IO command is not enabled, the IRQ handler will has wrong
    // information. It will cause IRQ storm when the correctly IRQ handler fails
    // to run.
    //
    if (!(IS_PCI_VGA (&PciConfigHeader)     ||
          IS_PCI_OLD_VGA (&PciConfigHeader) ||
          IS_PCI_IDE (&PciConfigHeader)     ||
          IS_PCI_P2P (&PciConfigHeader)     ||
          IS_PCI_P2P_SUB (&PciConfigHeader) ||
          IS_PCI_LPC (&PciConfigHeader)     )) {

      PciConfigHeader.Hdr.Command |= 0x1f;

      PciIo->Pci.Write (PciIo, EfiPciIoWidthUint32, 4, 1, &PciConfigHeader.Hdr.Command);
    }
  }
}

/**
  The following routines are identical in operation, so combine
  for code compaction:
  EfiGetPlatformBinaryGetMpTable
  EfiGetPlatformBinaryGetOemIntData
  EfiGetPlatformBinaryGetOem32Data
  EfiGetPlatformBinaryGetOem16Data

  @param  This                    Protocol instance pointer.
  @param  Id                      Table/Data identifier

  @retval EFI_SUCCESS             Success
  @retval EFI_INVALID_PARAMETER   Invalid ID
  @retval EFI_OUT_OF_RESOURCES    no resource to get data or table

**/
EFI_STATUS
LegacyGetDataOrTable (
  IN EFI_LEGACY_BIOS_PROTOCOL         *This,
  IN EFI_GET_PLATFORM_INFO_MODE       Id
  )
{
  VOID                              *Table;
  UINT32                            TablePtr;
  UINTN                             TableSize;
  UINTN                             Alignment;
  UINTN                             Location;
  EFI_STATUS                        Status;
  EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *LegacyBiosPlatform;
  EFI_COMPATIBILITY16_TABLE         *Legacy16Table;
  EFI_IA32_REGISTER_SET             Regs;
  LEGACY_BIOS_INSTANCE              *Private;

  Private             = LEGACY_BIOS_INSTANCE_FROM_THIS (This);

  LegacyBiosPlatform  = Private->LegacyBiosPlatform;
  Legacy16Table       = Private->Legacy16Table;

  //
  // Phase 1 - get an address allocated in 16-bit code
  //
  while (TRUE) {
    switch (Id) {
    case EfiGetPlatformBinaryMpTable:
    case EfiGetPlatformBinaryOemIntData:
    case EfiGetPlatformBinaryOem32Data:
    case EfiGetPlatformBinaryOem16Data:
      {
        Status = LegacyBiosPlatform->GetPlatformInfo (
                                       LegacyBiosPlatform,
                                       Id,
                                       (VOID *) &Table,
                                       &TableSize,
                                       &Location,
                                       &Alignment,
                                       0,
                                       0
                                       );
        DEBUG ((EFI_D_INFO, "LegacyGetDataOrTable - ID: %x, %r\n", (UINTN)Id, Status));
        DEBUG ((EFI_D_INFO, "  Table - %x, Size - %x, Location - %x, Alignment - %x\n", (UINTN)Table, (UINTN)TableSize, (UINTN)Location, (UINTN)Alignment));
        break;
      }

    default:
      {
        return EFI_INVALID_PARAMETER;
      }
    }

    if (EFI_ERROR (Status)) {
      return Status;
    }

    ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16GetTableAddress;
    Regs.X.CX = (UINT16) TableSize;
    Regs.X.BX = (UINT16) Location;
    Regs.X.DX = (UINT16) Alignment;
    Private->LegacyBios.FarCall86 (
                          This,
                          Private->Legacy16CallSegment,
                          Private->Legacy16CallOffset,
                          &Regs,
                          NULL,
                          0
                          );

    if (Regs.X.AX != 0) {
      DEBUG ((EFI_D_ERROR, "Table ID %x length insufficient\n", Id));
      return EFI_OUT_OF_RESOURCES;
    } else {
      break;
    }
  }
  //
  // Phase 2 Call routine second time with address to allow address adjustment
  //
  Status = LegacyBiosPlatform->GetPlatformInfo (
                                 LegacyBiosPlatform,
                                 Id,
                                 (VOID *) &Table,
                                 &TableSize,
                                 &Location,
                                 &Alignment,
                                 Regs.X.DS,
                                 Regs.X.BX
                                 );
  switch (Id) {
  case EfiGetPlatformBinaryMpTable:
    {
      Legacy16Table->MpTablePtr     = (UINT32) (Regs.X.DS * 16 + Regs.X.BX);
      Legacy16Table->MpTableLength  = (UINT32)TableSize;
      DEBUG ((EFI_D_INFO, "MP table in legacy region - %x\n", (UINTN)Legacy16Table->MpTablePtr));
      break;
    }

  case EfiGetPlatformBinaryOemIntData:
    {

      Legacy16Table->OemIntSegment  = Regs.X.DS;
      Legacy16Table->OemIntOffset   = Regs.X.BX;
      DEBUG ((EFI_D_INFO, "OemInt table in legacy region - %04x:%04x\n", (UINTN)Legacy16Table->OemIntSegment, (UINTN)Legacy16Table->OemIntOffset));
      break;
    }

  case EfiGetPlatformBinaryOem32Data:
    {
      Legacy16Table->Oem32Segment = Regs.X.DS;
      Legacy16Table->Oem32Offset  = Regs.X.BX;
      DEBUG ((EFI_D_INFO, "Oem32 table in legacy region - %04x:%04x\n", (UINTN)Legacy16Table->Oem32Segment, (UINTN)Legacy16Table->Oem32Offset));
      break;
    }

  case EfiGetPlatformBinaryOem16Data:
    {
      //
      //          Legacy16Table->Oem16Segment = Regs.X.DS;
      //          Legacy16Table->Oem16Offset  = Regs.X.BX;
      DEBUG ((EFI_D_INFO, "Oem16 table in legacy region - %04x:%04x\n", (UINTN)Legacy16Table->Oem16Segment, (UINTN)Legacy16Table->Oem16Offset));
      break;
    }

  default:
    {
      return EFI_INVALID_PARAMETER;
    }
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Phase 3 Copy table to final location
  //
  TablePtr = (UINT32) (Regs.X.DS * 16 + Regs.X.BX);

  CopyMem (
    (VOID *) (UINTN)TablePtr,
    Table,
    TableSize
    );

  return EFI_SUCCESS;
}

/**
  Search string in Smbios table by Type

  @param  Type                    SMBIOS structure type
  @param  Handle                  SMBIOS structure handle
  @param  StrOffset               String offset in the header of this SMBIOS structure type
  @param  Location                Return pointer to the string
  @param  Size                    The returned string size

  @retval EFI_SUCCESS             Success
  @retval EFI_NOT_FOUND           Smbios table not found
**/
EFI_STATUS
LocateSmbiosStringByType (
  IN UINT8                              Type,
  IN UINT16                             *Handle,   OPTIONAL
  IN UINT16                             StrOffset,
  OUT VOID                              **Location,
  OUT UINT32                            *Size
  )
{
  SMBIOS_TABLE_ENTRY_POINT          *SmbiosTableEntryPoint;
  SMBIOS_TABLE_3_0_ENTRY_POINT      *Smbios3TableEntryPoint;
  SMBIOS_STRUCTURE                  *SmbiosTable;
  SMBIOS_STRUCTURE                  *NextTable;
  EFI_STATUS                        Status;
  UINT8                             *Temp;
  UINT8                             *StringAddress;
  UINT8                             StrNum;
  UINT8                             StrCount;
  UINT8                             *StrStart;
  UINTN                             TableEndAddr;

  SmbiosTableEntryPoint = NULL;
  Smbios3TableEntryPoint= NULL;
  SmbiosTable           = NULL;
  NextTable             = NULL;

  //
  // Get SMBIOS and ACPI table pointers
  //
  Status = EfiGetSystemConfigurationTable (
             &gEfiSmbiosTableGuid,
             (VOID **)&SmbiosTableEntryPoint
             );

  EfiGetSystemConfigurationTable (
             &gEfiSmbios3TableGuid,
             (VOID **)&Smbios3TableEntryPoint
             );

  if (SmbiosTableEntryPoint == NULL && Smbios3TableEntryPoint == NULL) {
    return EFI_NOT_FOUND;
  }

  if (SmbiosTableEntryPoint != NULL){
    SmbiosTable = (SMBIOS_STRUCTURE *)(UINTN)SmbiosTableEntryPoint->TableAddress;
    TableEndAddr = (UINTN)SmbiosTable + (UINTN)SmbiosTableEntryPoint->TableLength;
  } else if (Smbios3TableEntryPoint != NULL) {
    SmbiosTable = (SMBIOS_STRUCTURE *)(UINTN)Smbios3TableEntryPoint->TableAddress;
    TableEndAddr = (UINTN)SmbiosTable + (UINTN)Smbios3TableEntryPoint->TableMaximumSize;
  } else {
    return EFI_NOT_FOUND; 
  }

  while ((UINTN)SmbiosTable < TableEndAddr){
    //
    // Get the next handle
    //
    Temp = (UINT8*)SmbiosTable + SmbiosTable->Length;
    while (TRUE) {
      //
      // The ending of a handle is '00' '00'
      //
      if ((*Temp == 0x00) && (*(Temp + 1) == 0x00)) {
        Temp = Temp + 2;
        NextTable = (SMBIOS_STRUCTURE*)Temp;
        break;
      } else {
        Temp++;
      }
    }

    if ((SmbiosTable->Type == Type) &&
        (Handle == NULL || (SmbiosTable->Handle == *Handle))) {
      //
      // Check string number. If string number is zero, it means no reference string.
      //
      Temp = (UINT8 *) SmbiosTable + StrOffset;
      StrNum = *Temp;

      if (StrNum != 0) {
        StrStart = (UINT8 *) SmbiosTable + SmbiosTable->Length;
        StrCount = 1;

        for (StringAddress = StrStart; StringAddress < (UINT8 *) NextTable; StringAddress++) {
          if (*StringAddress == 0x00) {
            if (StrCount == StrNum) {
              StringAddress++;
              break;
            } else {
              StrCount++;
              StrStart = StringAddress + 1;
            }
          }
        }

        //
        // If success to get string by this structure, update output data. Otherwise, keep searching.
        //
        if (StringAddress < (UINT8 *) NextTable) {
          *Location = (VOID *) StrStart;
          *Size = (UINT16) (StringAddress - StrStart);
          return EFI_SUCCESS;
        }
      }
    }

    SmbiosTable = NextTable;
  }

  return EFI_NOT_FOUND;
}

/**
  HCT will check 0xFFFF5 for BIOS Release Date
  And CSM already define 0xFFFF5 for bios date
  Just update the 0xFFFF5 to current bios date from SMBIOS

  @param  This                    Protocol instance pointer.

  @retval EFI_SUCCESS             Success
  @retval EFI_NOT_FOUND           Dmi String not found
**/
EFI_STATUS
UpdateBiosReleaseDate (
  IN EFI_LEGACY_BIOS_PROTOCOL         *This
  )
{
  EFI_STATUS                        Status;
  VOID                              *Table;
  UINT32                            TablePtr;
  UINTN                             TableSize;
  VOID                              *DmiLocation;
  UINT32                            DmiSize;
  CHAR8                             ReleaseDate[10];

  //
  // Read Dmi String (STR_MISC_BIOS_RELEASE_DATE)
  //
  Status = LocateSmbiosStringByType (0, NULL ,0x8, &DmiLocation, &DmiSize);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  CopyMem ((UINT8 *)ReleaseDate, (UINT8 *)DmiLocation, DmiSize);

  ReleaseDate[6] = ReleaseDate[8];
  ReleaseDate[7] = ReleaseDate[9];
  ReleaseDate[8] = 0;
  ReleaseDate[9] = 0;

  //
  // Update SystemBiosDate
  //
  TablePtr = 0x000FFFF5;
  Table=(VOID *)ReleaseDate;
  TableSize= 0x08;
  CopyMem ((VOID *)(UINTN)TablePtr, Table, TableSize);

  return EFI_SUCCESS;
}

/**
  Internal function to initalize H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL
  data and trigger gH2OBdsCpLegacyBootBeforeProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpLegacyBootBeforeProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpLegacyBootBeforeProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpLegacyBootBefore (
  VOID
  )
{
  H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL   *BeforeLegacyBoot;

  BeforeLegacyBoot = AllocateZeroPool (sizeof (H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL));
  if (BeforeLegacyBoot == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  BeforeLegacyBoot->Size = sizeof (H2O_BDS_CP_LEGACY_BOOT_BEFORE_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpLegacyBootBeforeProtocolGuid, BeforeLegacyBoot);
}

/**
  Internal function to initalize H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL
  data and trigger gH2OBdsCpLegacyBootAfterProtocolGuid checkpoint.

  @retval EFI_SUCCESS             Trigger gH2OBdsCpLegacyBootAfterProtocolGuid checkpoint successfully.
  @retval EFI_OUT_OF_RESOURCES    Allocate memory to initialize checkpoint data failed.
  @return Other                   Other error occurred while triggering gH2OBdsCpLegacyBootAfterProtocolGuid
                                  checkpoint.
**/
STATIC
EFI_STATUS
TriggerCpLegacyBootAfter (
  VOID
  )
{
  H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL   *AfterLegacyBoot;

  AfterLegacyBoot = AllocateZeroPool (sizeof (H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL));
  if (AfterLegacyBoot == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  AfterLegacyBoot->Size = sizeof (H2O_BDS_CP_LEGACY_BOOT_AFTER_PROTOCOL);
  return BdsCpTrigger (&gH2OBdsCpLegacyBootAfterProtocolGuid, AfterLegacyBoot);
}

/**
  Assign drive number to legacy HDD drives prior to booting an EFI
  aware OS so the OS can access drives without an EFI driver.
  Note: BBS compliant drives ARE NOT available until this call by
  either shell or EFI.

  @param  This                    Protocol instance pointer.

  @retval EFI_SUCCESS             Drive numbers assigned

**/
EFI_STATUS
GenericLegacyBoot (
  IN EFI_LEGACY_BIOS_PROTOCOL           *This
  )
{
  EFI_STATUS                        Status;
  LEGACY_BIOS_INSTANCE              *Private;
  EFI_IA32_REGISTER_SET             Regs;
  EFI_TO_COMPATIBILITY16_BOOT_TABLE *EfiToLegacy16BootTable;
  EFI_LEGACY_BIOS_PLATFORM_PROTOCOL *LegacyBiosPlatform;
  UINTN                             CopySize;
  VOID                              *AcpiPtr;
  HDD_INFO                          *HddInfo;
  HDD_INFO                          *LocalHddInfo;
  UINTN                             Index;
  EFI_COMPATIBILITY16_TABLE         *Legacy16Table;
  UINT32                            *BdaPtr;
  UINT16                            HddCount;
  UINT16                            BbsCount;
  BBS_TABLE                         *LocalBbsTable;
  UINT32                            *BaseVectorMaster;
  EFI_TIME                          BootTime;
  UINT32                            LocalTime;
  EFI_HANDLE                        IdeController;
  UINTN                             HandleCount;
  EFI_HANDLE                        *HandleBuffer;
  SMBIOS_TABLE_ENTRY_POINT          *SmbiosTable;
  SMBIOS_TABLE_3_0_ENTRY_POINT      *SmbiosTable64Bit; 
  SMBIOS_TABLE_3_0_ENTRY_POINT      *Smbios30TablePtr;
  EFI_PHYSICAL_ADDRESS              ReAllocSmbios30Table;
  EFI_PHYSICAL_ADDRESS              ReAllocSmbios30TableHeader;
  VOID                              *AcpiTable;
  UINTN                             ShadowAddress;
  UINT32                            Granularity;
  EFI_E820_ENTRY64                  *E820;
  UINT32                             HeaderSize;

  LocalHddInfo  = NULL;
  HddCount      = 0;
  BbsCount      = 0;
  LocalBbsTable = NULL;

  Private       = LEGACY_BIOS_INSTANCE_FROM_THIS (This);
  DEBUG_CODE (
    DEBUG ((EFI_D_INFO, "Start of legacy boot\n"));
  );

  Legacy16Table                         = Private->Legacy16Table;
  EfiToLegacy16BootTable                = &Private->IntThunk->EfiToLegacy16BootTable;
  HddInfo = &EfiToLegacy16BootTable->HddInfo[0];

  LegacyBiosPlatform = Private->LegacyBiosPlatform;

  EfiToLegacy16BootTable->MajorVersion = EFI_TO_LEGACY_MAJOR_VERSION;
  EfiToLegacy16BootTable->MinorVersion = EFI_TO_LEGACY_MINOR_VERSION;

  //
  // If booting to a legacy OS then force HDD drives to the appropriate
  // boot mode by calling GetIdeHandle.
  // A reconnect -r can force all HDDs back to native mode.
  //
  IdeController = NULL;
  if ((mBootMode == BOOT_LEGACY_OS) || (mBootMode == BOOT_UNCONVENTIONAL_DEVICE)) {
    //
    // In case the GetPlatformHandle returns error in Non-IDE system,
    // Make the error checking to prevent gBS->HandleProtocol crash
    //
    HandleCount   = 0;
    Status = LegacyBiosPlatform->GetPlatformHandle (
                                   Private->LegacyBiosPlatform,
                                   EfiGetPlatformIdeHandle,
                                   0,
                                   &HandleBuffer,
                                   &HandleCount,
                                   NULL
                                   );
    if (!EFI_ERROR (Status) && HandleCount > 0) {
      IdeController = HandleBuffer[0];
    }
  }
  //
  // Unlock the Legacy BIOS region
  //
  Private->LegacyRegion->UnLock (
                           Private->LegacyRegion,
                           0xE0000,
                           0x20000,
                           &Granularity
                           );

  //
  // Reconstruct the Legacy16 boot memory map
  //
  LegacyBiosBuildE820 (Private, &CopySize);

  if (CopySize > Private->Legacy16Table->E820Length) {
    ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16GetTableAddress;
    Regs.X.CX = (UINT16) CopySize;
    Private->LegacyBios.FarCall86 (
                          &Private->LegacyBios,
                          Private->Legacy16Table->Compatibility16CallSegment,
                          Private->Legacy16Table->Compatibility16CallOffset,
                          &Regs,
                          NULL,
                          0
                          );

    Private->Legacy16Table->E820Pointer = (UINT32) (Regs.X.DS * 16 + Regs.X.BX);
    Private->Legacy16Table->E820Length  = (UINT32) CopySize;
    if (Regs.X.AX != 0) {
      DEBUG ((EFI_D_ERROR, "Legacy16 E820 length insufficient\n"));
    } else {
      CopyMem (
        (VOID *)(UINTN) Private->Legacy16Table->E820Pointer,
        Private->E820Table,
        CopySize
        );
    }
  } else {
    CopyMem (
      (VOID *)(UINTN) Private->Legacy16Table->E820Pointer,
      Private->E820Table,
      CopySize
      );
    Private->Legacy16Table->E820Length = (UINT32) CopySize;
  }
  //
  // Get SMBIOS and ACPI table pointers
  //
  SmbiosTable = NULL;
  EfiGetSystemConfigurationTable (
    &gEfiSmbiosTableGuid,
    (VOID **)&SmbiosTable
    );
  //
  // We do not ASSERT if SmbiosTable not found. It is possbile that a platform does not produce SmbiosTable.
  //
  if (SmbiosTable == NULL) {
    DEBUG ((EFI_D_ERROR, "Smbios table is not found!\n"));
  }

  EfiToLegacy16BootTable->SmbiosTable = (UINT32)(UINTN)SmbiosTable;

  //
  // SMBIOS 3.0
  //
  SmbiosTable64Bit = NULL;
  EfiGetSystemConfigurationTable (
    &gEfiSmbios3TableGuid,
    (VOID **)&SmbiosTable64Bit
    );
  //
  // We do not ASSERT if SmbiosTable not found. It is possbile that a platform does not produce SmbiosTable.
  //
  if (SmbiosTable64Bit == NULL) {
    DEBUG ((EFI_D_ERROR, "Smbios table 3.0 is not found!\n"));
  }

  ReAllocSmbios30TableHeader = 0;  
  // Cascade Smbios 2.x & 3.0 header
  if (SmbiosTable64Bit != NULL) {
    HeaderSize = (SmbiosTable == NULL) ? sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT) : sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT) + sizeof (SMBIOS_TABLE_ENTRY_POINT) + 1;
    ReAllocSmbios30TableHeader = BASE_4GB - BASE_64KB;
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiBootServicesData,
                    EFI_SIZE_TO_PAGES(HeaderSize),
                    &ReAllocSmbios30TableHeader
                    );
    if (!EFI_ERROR (Status)){
      // Preserved 32 bytes space for 31 bytes long Smbios 2.x header
      if (HeaderSize == sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT) + sizeof (SMBIOS_TABLE_ENTRY_POINT) + 1) {

        CopyMem ((VOID*)(UINTN) ReAllocSmbios30TableHeader, (VOID*) SmbiosTable, sizeof(SMBIOS_TABLE_ENTRY_POINT));
        CopyMem ((VOID*)(UINTN) (ReAllocSmbios30TableHeader + sizeof (SMBIOS_TABLE_ENTRY_POINT) + 1), (VOID*) SmbiosTable64Bit, sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT));

      } else {
        CopyMem ((VOID*)(UINTN) ReAllocSmbios30TableHeader, (VOID*)SmbiosTable64Bit, sizeof(SMBIOS_TABLE_3_0_ENTRY_POINT));
      }
      // point to the cascaded header
      EfiToLegacy16BootTable->SmbiosTable = (UINT32)(UINTN)ReAllocSmbios30TableHeader;      
    } else {
      ReAllocSmbios30TableHeader = 0;    
    }
  }

  //
  // Item TableAddress of smbios 30 header point to smbios 2.x body
  //
  if ((SmbiosTable64Bit != NULL) && (ReAllocSmbios30TableHeader != 0)) { 
    if (SmbiosTable != NULL){
      Smbios30TablePtr = (SMBIOS_TABLE_3_0_ENTRY_POINT*)(UINTN)(ReAllocSmbios30TableHeader + sizeof (SMBIOS_TABLE_ENTRY_POINT) + 1);
      Smbios30TablePtr->TableAddress = SmbiosTable->TableAddress;
    } else if (SmbiosTable64Bit->TableAddress + SmbiosTable64Bit->TableMaximumSize > BASE_4GB){
      ReAllocSmbios30Table = BASE_4GB - SmbiosTable64Bit->TableMaximumSize; 
      Status = gBS->AllocatePages (
                      AllocateMaxAddress,
                      EfiBootServicesData,
                      EFI_SIZE_TO_PAGES (SmbiosTable64Bit->TableMaximumSize),
                      &ReAllocSmbios30Table
                      );
      if (!EFI_ERROR (Status)){
        CopyMem ((VOID*)(UINTN)ReAllocSmbios30Table, (VOID*)(UINTN)SmbiosTable64Bit->TableAddress, SmbiosTable64Bit->TableMaximumSize);
        Smbios30TablePtr = (SMBIOS_TABLE_3_0_ENTRY_POINT*)(UINTN)(ReAllocSmbios30TableHeader);
        Smbios30TablePtr->TableAddress = (UINT64) ReAllocSmbios30Table;
      }
    }
  }  


  AcpiTable = NULL;
  Status = EfiGetSystemConfigurationTable (
             &gEfiAcpi20TableGuid,
             (VOID **)&AcpiTable
             );
  if (EFI_ERROR (Status)) {
    Status = EfiGetSystemConfigurationTable (
               &gEfiAcpi10TableGuid,
               (VOID **)&AcpiTable
               );
  }
  //
  // We do not ASSERT if AcpiTable not found. It is possbile that a platform does not produce AcpiTable.
  //
  if (AcpiTable == NULL) {
    DEBUG ((EFI_D_ERROR, "ACPI table is not found!\n"));
  }
  EfiToLegacy16BootTable->AcpiTable = (UINT32)(UINTN)AcpiTable;

  //
  // Get RSD Ptr table rev at offset 15 decimal
  // Rev = 0 Length is 20 decimal
  // Rev != 0 Length is UINT32 at offset 20 decimal
  //
  if (AcpiTable != NULL) {

    AcpiPtr = AcpiTable;
    if (*((UINT8 *) AcpiPtr + 15) == 0) {
      CopySize = 20;
    } else {
      AcpiPtr   = ((UINT8 *) AcpiPtr + 20);
      CopySize  = (*(UINT32 *) AcpiPtr);
    }

    CopyMem (
      (VOID *)(UINTN) Private->Legacy16Table->AcpiRsdPtrPointer,
      AcpiTable,
      CopySize
      );
  }
  //
  // Make sure all PCI Interrupt Line register are programmed to match 8259
  //
  PciProgramAllInterruptLineRegisters (Private);

  //
  // Unlock the Legacy BIOS region as PciProgramAllInterruptLineRegisters
  // can lock it.
  //
  Private->LegacyRegion->UnLock (
                           Private->LegacyRegion,
                           Private->BiosStart,
                           Private->LegacyBiosImageSize,
                           &Granularity
                           );

  //
  // Configure Legacy Device Magic
  //
  // Only do this code if booting legacy OS
  //
  if ((mBootMode == BOOT_LEGACY_OS) || (mBootMode == BOOT_UNCONVENTIONAL_DEVICE)) {
    UpdateSioData (Private);
  }
  //
  // Setup BDA and EBDA standard areas before Legacy Boot
  //
  LegacyBiosCompleteBdaBeforeBoot (Private);
  LegacyBiosCompleteStandardCmosBeforeBoot (Private);


  UpdateAllIdentifyDriveData (Private);

  //
  // Clear IO BAR, if IDE controller in legacy mode.
  //
  InitLegacyIdeController (IdeController);

  //
  // Generate number of ticks since midnight for BDA. DOS requires this
  // for its time. We have to make assumptions as to how long following
  // code takes since after PciShadowRoms PciIo is gone. Place result in
  // 40:6C-6F
  //
  // Adjust value by 1 second.
  //
  gRT->GetTime (&BootTime, NULL);
  LocalTime = BootTime.Hour * 3600 + BootTime.Minute * 60 + BootTime.Second;
  LocalTime += 1;

  //
  // Multiply result by 18.2 for number of ticks since midnight.
  // Use 182/10 to avoid floating point math.
  //
  LocalTime = (LocalTime * 182) / 10;
  BdaPtr    = (UINT32 *) (UINTN)0x46C;
  *BdaPtr   = LocalTime;

  //
  // Shadow PCI ROMs. We must do this near the end since this will kick
  // of Native EFI drivers that may be needed to collect info for Legacy16
  //
  //  WARNING: PciIo is gone after this call.
  //
  PciShadowRoms (Private);

  //
  // Shadow PXE base code, BIS etc.
  //
  Private->LegacyRegion->UnLock (Private->LegacyRegion, 0xc0000, 0x40000, &Granularity);
  ShadowAddress = Private->OptionRom;
  Private->LegacyBiosPlatform->PlatformHooks (
                                 Private->LegacyBiosPlatform,
                                 EfiPlatformHookShadowServiceRoms,
                                 0,
                                 0,
                                 &ShadowAddress,
                                 Legacy16Table,
                                 NULL
                                 );
  Private->OptionRom = (UINT32)ShadowAddress;

  //
  // Let platform code know the boot options
  //
  LegacyBiosGetBbsInfo (
    This,
    &HddCount,
    &LocalHddInfo,
    &BbsCount,
    &LocalBbsTable
    );

  DEBUG_CODE (
    PrintPciInterruptRegister ();
    PrintBbsTable (LocalBbsTable);
    PrintHddInfo (LocalHddInfo);
    );

  Private->LegacyRegion->UnLock (
                           Private->LegacyRegion,
                           0xc0000,
                           0x40000,
                           &Granularity
                           );

  LegacyBiosPlatform->PrepareToBoot (
                        LegacyBiosPlatform,
                        mBbsDevicePathPtr,
                        mBbsTable,
                        mLoadOptionsSize,
                        mLoadOptions,
                        (VOID *) &Private->IntThunk->EfiToLegacy16BootTable
                        );

  //
  // If no boot device return to BDS
  //
  if ((mBootMode == BOOT_LEGACY_OS) || (mBootMode == BOOT_UNCONVENTIONAL_DEVICE)) {
    for (Index = 0; Index < BbsCount; Index++){
      if ((LocalBbsTable[Index].BootPriority != BBS_DO_NOT_BOOT_FROM) &&
          (LocalBbsTable[Index].BootPriority != BBS_UNPRIORITIZED_ENTRY) &&
          (LocalBbsTable[Index].BootPriority != BBS_IGNORE_ENTRY)) {
        break;
      }
    }
    if (Index == BbsCount) {
      return EFI_DEVICE_ERROR;
    }
  }
  //
  // Let the Legacy16 code know the device path type for legacy boot
  //
  EfiToLegacy16BootTable->DevicePathType = mBbsDevicePathPtr->DeviceType;

  Status = UpdateBiosReleaseDate(This);

  //
  // Copy MP table, if it exists.
  //
  CreateMpTable (Private);

  if (!Private->LegacyBootEntered) {
    //
    // Copy OEM INT Data, if it exists. Note: This code treats any data
    // as a bag of bits and knows nothing of the contents nor cares.
    // Contents are IBV specific.
    //
    LegacyGetDataOrTable (This, EfiGetPlatformBinaryOemIntData);

    //
    // Copy OEM16 Data, if it exists.Note: This code treats any data
    // as a bag of bits and knows nothing of the contents nor cares.
    // Contents are IBV specific.
    //
    LegacyGetDataOrTable (This, EfiGetPlatformBinaryOem16Data);

    //
    // Copy OEM32 Data, if it exists.Note: This code treats any data
    // as a bag of bits and knows nothing of the contents nor cares.
    // Contents are IBV specific.
    //
    LegacyGetDataOrTable (This, EfiGetPlatformBinaryOem32Data);
  }

  //
  // Call into Legacy16 code to prepare for INT 19h
  //
  ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
  Regs.X.AX = Legacy16PrepareToBoot;

  POST_CODE (BDS_LEGACY16_PREPARE_TO_BOOT);  //PostCode = 0x31, Prepare to Boot to Legacy OS
  //
  // Pass in handoff data
  //
  Regs.X.ES = NORMALIZE_EFI_SEGMENT ((UINTN)EfiToLegacy16BootTable);
  Regs.X.BX = NORMALIZE_EFI_OFFSET ((UINTN)EfiToLegacy16BootTable);

  Private->LegacyBios.FarCall86 (
                        This,
                        Private->Legacy16CallSegment,
                        Private->Legacy16CallOffset,
                        &Regs,
                        NULL,
                        0
                        );

  if (Regs.X.AX != 0) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Re-update E820 table
  //
  E820 = (EFI_E820_ENTRY64 *) Private->E820Table;
  //
  // Update First entry from BDA
  //
  E820[0].Length    = (UINT64) ((*(UINT16 *) (UINTN)0x40E) << 4);

  //
  // Second entry is (640k - EBDA) to 640k
  //
  E820[1].BaseAddr  = E820[0].Length;
  E820[1].Length    = (UINT64) ((640 * 1024) - E820[0].Length);

  CopyMem (
    (VOID *) (UINTN) Private->Legacy16Table->E820Pointer,
    Private->E820Table,
    sizeof (EFI_E820_ENTRY64) * 2
    );

  //
  // Lock the Legacy BIOS region
  //
  Private->LegacyRegion->Lock (
                           Private->LegacyRegion,
                           0xc0000,
                           0x40000,
                           &Granularity
                           );
  //
  // Lock attributes of the Legacy Region if chipset supports
  //
  Private->LegacyRegion->BootLock (
                           Private->LegacyRegion,
                           0xc0000,
                           0x40000,
                           &Granularity
                           );

  //
  // Call into Legacy16 code to do the INT 19h
  //
  EnableAllControllers (Private);
  if ((mBootMode == BOOT_LEGACY_OS) || (mBootMode == BOOT_UNCONVENTIONAL_DEVICE)) {
    //
    // Disable DXE Timer before signal legacy boot event
    // to preventing DXE services from called in timer handler
    //
    Private->Timer->SetTimerPeriod (Private->Timer, 0);
    TriggerCpLegacyBootBefore ();
    POST_CODE (BDS_LEGACY_BOOT_EVENT); //PostCode = 0x33, Last Chipset initial before boot to Legacy OS
    //
    // Signal all the events that are waiting on EVT_SIGNAL_LEGACY_BOOT
    //
    EfiSignalEventLegacyBoot ();
    DEBUG ((EFI_D_INFO, "Legacy INT19 Boot...\n"));
    //
    // Report Status Code to indicate legacy boot event was signalled
    //
    REPORT_STATUS_CODE (
      EFI_PROGRESS_CODE,
      (EFI_SOFTWARE_DXE_BS_DRIVER | EFI_SW_DXE_BS_PC_LEGACY_BOOT_EVENT)
      );
    TriggerCpLegacyBootAfter ();
    //
    // Save and disable interrupt of debug timer
    //
    SaveAndSetDebugTimerInterrupt (FALSE);
    //
    // Put the 8259 into its legacy mode by reprogramming the vector bases
    //
    Private->Legacy8259->SetVectorBase (Private->Legacy8259, LEGACY_MODE_BASE_VECTOR_MASTER, LEGACY_MODE_BASE_VECTOR_SLAVE);
    //
    // PC History
    //   The original PC used INT8-F for master PIC. Since these mapped over
    //   processor exceptions TIANO moved the master PIC to INT68-6F.
    // We need to set these back to the Legacy16 unexpected interrupt(saved
    // in LegacyBios.c) since some OS see that these have values different from
    // what is expected and invoke them. Since the legacy OS corrupts EFI
    // memory, there is no handler for these interrupts and OS blows up.
    //
    // We need to save the TIANO values for the rare case that the Legacy16
    // code cannot boot but knows memory hasn't been destroyed.
    //
    // To compound the problem, video takes over one of these INTS and must be
    // be left.
    // @bug - determine if video hooks INT(in which case we must find new
    //          set of TIANO vectors) or takes it over.
    //
    //
    BaseVectorMaster = (UINT32 *) (sizeof (UINT32) * PROTECTED_MODE_BASE_VECTOR_MASTER);
    for (Index = 0; Index < 8; Index++) {
      Private->ThunkSavedInt[Index] = BaseVectorMaster[Index];
      if (Private->ThunkSeg == (UINT16) (BaseVectorMaster[Index] >> 16)) {
        BaseVectorMaster[Index] = (UINT32) (Private->BiosUnexpectedInt);
      }
    }

    POST_CODE (BDS_ENTER_LEGACY_16_BOOT);  //PostCode = 0x34, Ready to Boot Legacy OS

    ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
    Regs.X.AX = Legacy16Boot;

    Private->LegacyBios.FarCall86 (
                          This,
                          Private->Legacy16CallSegment,
                          Private->Legacy16CallOffset,
                          &Regs,
                          NULL,
                          0
                          );

    BaseVectorMaster = (UINT32 *) (sizeof (UINT32) * PROTECTED_MODE_BASE_VECTOR_MASTER);
    for (Index = 0; Index < 8; Index++) {
      BaseVectorMaster[Index] = Private->ThunkSavedInt[Index];
    }
  }
  Private->LegacyBootEntered = TRUE;
  if ((mBootMode == BOOT_LEGACY_OS) || (mBootMode == BOOT_UNCONVENTIONAL_DEVICE)) {
    //
    // Should never return unless never passed control to 0:7c00(first stage
    // OS loader) and only then if no bootable device found.
    //
    return EFI_DEVICE_ERROR;
  } else {
    //
    // If boot to EFI then expect to return to caller
    //
    return EFI_SUCCESS;
  }
}


/**
  Assign drive number to legacy HDD drives prior to booting an EFI
  aware OS so the OS can access drives without an EFI driver.
  Note: BBS compliant drives ARE NOT available until this call by
  either shell or EFI.

  @param  This                    Protocol instance pointer.
  @param  BbsCount                Number of BBS_TABLE structures
  @param  BbsTable                List BBS entries

  @retval EFI_SUCCESS             Drive numbers assigned

**/
EFI_STATUS
EFIAPI
LegacyBiosPrepareToBootEfi (
  IN EFI_LEGACY_BIOS_PROTOCOL         *This,
  OUT UINT16                          *BbsCount,
  OUT BBS_TABLE                       **BbsTable
  )
{
  EFI_STATUS                        Status;
  EFI_TO_COMPATIBILITY16_BOOT_TABLE *EfiToLegacy16BootTable;
  LEGACY_BIOS_INSTANCE              *Private;

  Private                 = LEGACY_BIOS_INSTANCE_FROM_THIS (This);
  EfiToLegacy16BootTable  = &Private->IntThunk->EfiToLegacy16BootTable;
  mBootMode               = BOOT_EFI_OS;
  mBbsDevicePathPtr       = NULL;
  Status                  = GenericLegacyBoot (This);
  *BbsTable               = (BBS_TABLE*)(UINTN)EfiToLegacy16BootTable->BbsTable;
  *BbsCount               = (UINT16) (sizeof (Private->IntThunk->BbsTable) / sizeof (BBS_TABLE));
  return Status;
}

/**
  To boot from an unconventional device like parties and/or execute HDD diagnostics.

  @param  This            Protocol instance pointer.
  @param  Attributes      How to interpret the other input parameters
  @param  BbsEntry        The 0-based index into the BbsTable for the parent
                          device.
  @param  BeerData        Pointer to the 128 bytes of ram BEER data.
  @param  ServiceAreaData Pointer to the 64 bytes of raw Service Area data. The
                          caller must provide a pointer to the specific Service
                          Area and not the start all Service Areas.

  @retval EFI_INVALID_PARAMETER if error. Does NOT return if no error.

***/
EFI_STATUS
EFIAPI
LegacyBiosBootUnconventionalDevice (
  IN EFI_LEGACY_BIOS_PROTOCOL         *This,
  IN UDC_ATTRIBUTES                   Attributes,
  IN UINTN                            BbsEntry,
  IN VOID                             *BeerData,
  IN VOID                             *ServiceAreaData
  )
{
  EFI_STATUS                        Status;
  EFI_TO_COMPATIBILITY16_BOOT_TABLE *EfiToLegacy16BootTable;
  LEGACY_BIOS_INSTANCE              *Private;
  UD_TABLE                          *UcdTable;
  UINTN                             Index;
  UINT16                            BootPriority;
  BBS_TABLE                         *BbsTable;

  BootPriority = 0;
  Private = LEGACY_BIOS_INSTANCE_FROM_THIS (This);
  mBootMode = BOOT_UNCONVENTIONAL_DEVICE;
  mBbsDevicePathPtr = &mBbsDevicePathNode;
  mAttributes = Attributes;
  mBbsEntry = BbsEntry;
  mBeerData = BeerData, mServiceAreaData = ServiceAreaData;

  EfiToLegacy16BootTable = &Private->IntThunk->EfiToLegacy16BootTable;

  //
  // Do input parameter checking
  //
  if ((Attributes.DirectoryServiceValidity == 0) &&
      (Attributes.RabcaUsedFlag == 0) &&
      (Attributes.ExecuteHddDiagnosticsFlag == 0)
      ) {
    return EFI_INVALID_PARAMETER;
  }

  if (((Attributes.DirectoryServiceValidity != 0) && (ServiceAreaData == NULL)) ||
      (((Attributes.DirectoryServiceValidity | Attributes.RabcaUsedFlag) != 0) && (BeerData == NULL))
      ) {
    return EFI_INVALID_PARAMETER;
  }

  UcdTable = (UD_TABLE *) AllocatePool (
														sizeof (UD_TABLE)
														);
  if (NULL == UcdTable) {
    return EFI_OUT_OF_RESOURCES;
  }

  EfiToLegacy16BootTable->UnconventionalDeviceTable = (UINT32)(UINTN)UcdTable;
  UcdTable->Attributes = Attributes;
  UcdTable->BbsTableEntryNumberForParentDevice = (UINT8) BbsEntry;
  //
  // Force all existing BBS entries to DoNotBoot. This allows 16-bit CSM
  // to assign drive numbers but bot boot from. Only newly created entries
  // will be valid.
  //
  BbsTable = (BBS_TABLE*)(UINTN)EfiToLegacy16BootTable->BbsTable;
  for (Index = 0; Index < EfiToLegacy16BootTable->NumberBbsEntries; Index++) {
    BbsTable[Index].BootPriority = BBS_DO_NOT_BOOT_FROM;
  }
  //
  // If parent is onboard IDE then assign controller & device number
  // else they are 0.
  //
  if (BbsEntry < MAX_IDE_CONTROLLER * 2) {
    UcdTable->DeviceNumber = (UINT8) ((BbsEntry - 1) % 2);
  }

  if (BeerData != NULL) {
    CopyMem (
      (VOID *) UcdTable->BeerData,
      BeerData,
      (UINTN) 128
      );
  }

  if (ServiceAreaData != NULL) {
    CopyMem (
      (VOID *) UcdTable->ServiceAreaData,
      ServiceAreaData,
      (UINTN) 64
      );
  }
  //
  // For each new entry do the following:
  //   1. Increment current number of BBS entries
  //   2. Copy parent entry to new entry.
  //   3. Zero out BootHandler Offset & segment
  //   4. Set appropriate device type. BEV(0x80) for HDD diagnostics
  //      and Floppy(0x01) for PARTIES boot.
  //   5. Assign new priority.
  //
  if ((Attributes.ExecuteHddDiagnosticsFlag) != 0) {
    EfiToLegacy16BootTable->NumberBbsEntries += 1;

    CopyMem (
      (VOID *) &BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootPriority,
      (VOID *) &BbsTable[BbsEntry].BootPriority,
      sizeof (BBS_TABLE)
      );

    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootHandlerOffset  = 0;
    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootHandlerSegment = 0;
    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].DeviceType         = 0x80;

    UcdTable->BbsTableEntryNumberForHddDiag = (UINT8) (EfiToLegacy16BootTable->NumberBbsEntries - 1);

    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootPriority = BootPriority;
    BootPriority += 1;

    //
    // Set device type as BBS_TYPE_BEV for PARTIES diagnostic
    //
    mBbsDevicePathNode.DeviceType = BBS_TYPE_BEV;
  }

  if (((Attributes.DirectoryServiceValidity | Attributes.RabcaUsedFlag)) != 0) {
    EfiToLegacy16BootTable->NumberBbsEntries += 1;
    CopyMem (
      (VOID *) &BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootPriority,
      (VOID *) &BbsTable[BbsEntry].BootPriority,
      sizeof (BBS_TABLE)
      );

    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootHandlerOffset  = 0;
    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootHandlerSegment = 0;
    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].DeviceType         = 0x01;
    UcdTable->BbsTableEntryNumberForBoot = (UINT8) (EfiToLegacy16BootTable->NumberBbsEntries - 1);
    BbsTable[EfiToLegacy16BootTable->NumberBbsEntries].BootPriority = BootPriority;

    //
    // Set device type as BBS_TYPE_FLOPPY for PARTIES boot as floppy
    //
    mBbsDevicePathNode.DeviceType = BBS_TYPE_FLOPPY;
  }
  //
  // Build the BBS Device Path for this boot selection
  //
  mBbsDevicePathNode.Header.Type    = BBS_DEVICE_PATH;
  mBbsDevicePathNode.Header.SubType = BBS_BBS_DP;
  SetDevicePathNodeLength (&mBbsDevicePathNode.Header, sizeof (BBS_BBS_DEVICE_PATH));
  mBbsDevicePathNode.StatusFlag = 0;
  mBbsDevicePathNode.String[0]  = 0;

  Status                        = GenericLegacyBoot (This);
  return Status;
}

/**
  Attempt to legacy boot the BootOption. If the EFI contexted has been
  compromised this function will not return.

  @param  This             Protocol instance pointer.
  @param  BbsDevicePath    EFI Device Path from BootXXXX variable.
  @param  LoadOptionsSize  Size of LoadOption in size.
  @param  LoadOptions      LoadOption from BootXXXX variable

  @retval EFI_SUCCESS      Removable media not present

**/
EFI_STATUS
EFIAPI
LegacyBiosLegacyBoot (
  IN EFI_LEGACY_BIOS_PROTOCOL           *This,
  IN  BBS_BBS_DEVICE_PATH               *BbsDevicePath,
  IN  UINT32                            LoadOptionsSize,
  IN  VOID                              *LoadOptions
  )
{
  EFI_STATUS  Status;

  mBbsDevicePathPtr = BbsDevicePath;
  mLoadOptionsSize  = LoadOptionsSize;
  mLoadOptions      = LoadOptions;
  mBootMode         = BOOT_LEGACY_OS;
  Status            = GenericLegacyBoot (This);

  return Status;
}

/**
  Convert EFI Memory Type to E820 Memory Type.

  @param  Type  EFI Memory Type

  @return ACPI Memory Type for EFI Memory Type

**/
EFI_ACPI_MEMORY_TYPE
EfiMemoryTypeToE820Type (
  IN  UINT32    Type
  )
{
  switch (Type) {
  case EfiLoaderCode:
  case EfiLoaderData:
  case EfiBootServicesCode:
  case EfiBootServicesData:
  case EfiConventionalMemory:
    return EfiAcpiAddressRangeMemory;
  
  case EfiPersistentMemory:
    return EfiAddressRangePersistentMemory;

  case EfiACPIReclaimMemory:
    return EfiAcpiAddressRangeACPI;

  case EfiACPIMemoryNVS:
    return EfiAcpiAddressRangeNVS;

  //
  // All other types map to reserved.
  // Adding the code just waists FLASH space.
  //
  //  case  EfiReservedMemoryType:
  //  case  EfiRuntimeServicesCode:
  //  case  EfiRuntimeServicesData:
  //  case  EfiUnusableMemory:
  //  case  EfiMemoryMappedIO:
  //  case  EfiMemoryMappedIOPortSpace:
  //  case  EfiPalCode:
  //
  default:
    return EfiAcpiAddressRangeReserved;
  }
}

/**
  Adjust the E820 table.

  @param  E820     Pointer of E820 Table
  @param  Index    Counts of E820 Table

  @retval Index    Counts of E820 Table after Adjust.

**/

UINTN
AdjustE820Table (
  IN      EFI_E820_ENTRY64         *E820,
  IN      UINTN                    Index
  )
{
  UINTN                       TempIndex;
  BOOLEAN                     ChangedFlag;
  UINTN                       TempNextIndex;
  EFI_E820_ENTRY64            TempE820;
  UINTN                       IndexSort;

  for (TempIndex = 0; TempIndex < Index; TempIndex++) {
    ChangedFlag = FALSE;
    for (TempNextIndex = 1; TempNextIndex < Index - TempIndex; TempNextIndex++) {
      //
      // Sort E820Table from low to high
      //
      if (E820[TempNextIndex - 1].BaseAddr > E820[TempNextIndex].BaseAddr) {
        ChangedFlag                       = TRUE;
        TempE820.BaseAddr                 = E820[TempNextIndex - 1].BaseAddr;
        TempE820.Length                   = E820[TempNextIndex - 1].Length;
        TempE820.Type                     = E820[TempNextIndex - 1].Type;
        TempE820.ExtAttributes            = E820[TempNextIndex - 1].ExtAttributes;

        E820[TempNextIndex - 1].BaseAddr      = E820[TempNextIndex].BaseAddr;
        E820[TempNextIndex - 1].Length        = E820[TempNextIndex].Length;
        E820[TempNextIndex - 1].Type          = E820[TempNextIndex].Type;
        E820[TempNextIndex - 1].ExtAttributes = E820[TempNextIndex].ExtAttributes;

        E820[TempNextIndex].BaseAddr      = TempE820.BaseAddr;
        E820[TempNextIndex].Length        = TempE820.Length;
        E820[TempNextIndex].Type          = TempE820.Type;
        E820[TempNextIndex].ExtAttributes = TempE820.ExtAttributes;
      } else if ((E820[TempNextIndex - 1].Type          == E820[TempNextIndex].Type) &&
                 (E820[TempNextIndex - 1].ExtAttributes == E820[TempNextIndex].ExtAttributes) &&
                 (E820[TempNextIndex - 1].BaseAddr      == E820[TempNextIndex].BaseAddr)) {
        if (E820[TempNextIndex - 1].Length < E820[TempNextIndex].Length) {
          E820[TempNextIndex - 1].Length = E820[TempNextIndex].Length;
        }

        //
        // Filter the same type / base / length / ExtAttributes
        //
        E820[TempNextIndex].BaseAddr      = 0;
        E820[TempNextIndex].Length        = 0;
        E820[TempNextIndex].Type          = 0;
        E820[TempNextIndex].ExtAttributes = 0;

        for (IndexSort = TempNextIndex; IndexSort < Index ; IndexSort ++) {
          E820[IndexSort].BaseAddr      = E820[IndexSort + 1].BaseAddr;
          E820[IndexSort].Length        = E820[IndexSort + 1].Length;
          E820[IndexSort].Type          = E820[IndexSort + 1].Type;
          E820[IndexSort].ExtAttributes = E820[IndexSort + 1].ExtAttributes;
        }
        TempNextIndex--;
        Index--;
      }
    }

    if (!ChangedFlag) {
      break;
    }
  }

  //
  // Remove the overlap range
  //
  for (TempIndex = 1; TempIndex < Index; TempIndex++) {
    if (E820[TempIndex - 1].BaseAddr <= E820[TempIndex].BaseAddr &&
        ((E820[TempIndex - 1].BaseAddr + E820[TempIndex - 1].Length) >=
         (E820[TempIndex].BaseAddr + E820[TempIndex].Length))) {
      //
      // Overlap range is found
      //
      ASSERT (E820[TempIndex - 1].Type == E820[TempIndex].Type);

      if (TempIndex == Index - 1) {
        E820[TempIndex].BaseAddr      = 0;
        E820[TempIndex].Length        = 0;
        E820[TempIndex].Type          = 0;
        E820[TempIndex].ExtAttributes = 0;
        Index--;
        break;
      } else {
        for (IndexSort = TempIndex; IndexSort < Index; IndexSort ++) {
          E820[IndexSort].BaseAddr      = E820[IndexSort + 1].BaseAddr;
          E820[IndexSort].Length        = E820[IndexSort + 1].Length;
          E820[IndexSort].Type          = E820[IndexSort + 1].Type;
          E820[IndexSort].ExtAttributes = E820[IndexSort + 1].ExtAttributes;
        }
        TempIndex--;
        Index--;
      }
    }
  }

  for (TempIndex = 1; TempIndex < Index; TempIndex++) {
    //
    //  Merge the same type and contiguous memory
    //
    if ((E820[TempIndex - 1].Type == E820[TempIndex].Type) &&
        (E820[TempIndex - 1].ExtAttributes == E820[TempIndex].ExtAttributes) &&
        ((E820[TempIndex - 1].BaseAddr + E820[TempIndex - 1].Length) == E820[TempIndex].BaseAddr )) {

      E820[TempIndex - 1].Length    = E820[TempIndex - 1].Length + E820[TempIndex].Length;

      for (IndexSort = TempIndex; IndexSort < Index ; IndexSort ++) {
        E820[IndexSort].BaseAddr      = E820[IndexSort + 1].BaseAddr;
        E820[IndexSort].Length        = E820[IndexSort + 1].Length;
        E820[IndexSort].Type          = E820[IndexSort + 1].Type;
        E820[IndexSort].ExtAttributes = E820[IndexSort + 1].ExtAttributes;
      }
      TempIndex--;
      Index--;
    }
    //
    // Filter the same type/base/length
    //
    if ((E820[TempIndex - 1].Type == E820[TempIndex].Type) &&
        (E820[TempIndex - 1].ExtAttributes == E820[TempIndex].ExtAttributes) &&
        (E820[TempIndex - 1].BaseAddr == E820[TempIndex].BaseAddr) &&
        (E820[TempIndex - 1].Length == E820[TempIndex].Length)) {

      E820[TempIndex].BaseAddr      = 0;
      E820[TempIndex].Length        = 0;
      E820[TempIndex].Type          = 0;
      E820[TempIndex].ExtAttributes = 0;

      for (IndexSort = TempIndex; IndexSort < Index ; IndexSort ++) {
        E820[IndexSort].BaseAddr      = E820[IndexSort + 1].BaseAddr;
        E820[IndexSort].Length        = E820[IndexSort + 1].Length;
        E820[IndexSort].Type          = E820[IndexSort + 1].Type;
        E820[IndexSort].ExtAttributes = E820[IndexSort + 1].ExtAttributes;
      }
      TempIndex--;
      Index--;
    }
  }
  return Index;
}

/**
  Build the E820 table.

  @param  Private  Legacy BIOS Instance data
  @param  Size     Size of E820 Table

  @retval EFI_SUCCESS  It should always work.
**/
EFI_STATUS
LegacyBiosBuildE820 (
  IN  LEGACY_BIOS_INSTANCE    *Private,
  OUT UINTN                   *Size
  )
{
  EFI_STATUS                  Status;
  EFI_E820_ENTRY64            *E820;
  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMap;
  EFI_MEMORY_DESCRIPTOR       *EfiMemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR       *EfiEntry;
  EFI_MEMORY_DESCRIPTOR       *NextEfiEntry;
  EFI_MEMORY_DESCRIPTOR       TempEfiEntry;
  UINTN                       EfiMemoryMapSize;
  UINTN                       EfiMapKey;
  UINTN                       EfiDescriptorSize;
  UINT32                      EfiDescriptorVersion;
  UINTN                       Index;
  EFI_PEI_HOB_POINTERS        Hob;
  EFI_HOB_RESOURCE_DESCRIPTOR *ResourceHob;
  UINTN                       TempIndex;
  EFI_ACPI_MEMORY_TYPE        TempType;
  UINTN                       Above1MIndex;
  UINT64                      MemoryBlockLength;

  E820 = (EFI_E820_ENTRY64 *) Private->E820Table;

  //
  // Get the EFI memory map.
  //
  EfiMemoryMapSize  = 0;
  EfiMemoryMap      = NULL;
  Status = gBS->GetMemoryMap (
                  &EfiMemoryMapSize,
                  EfiMemoryMap,
                  &EfiMapKey,
                  &EfiDescriptorSize,
                  &EfiDescriptorVersion
                  );
  while (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Use size returned back plus 1 descriptor for the AllocatePool.
    // We don't just multiply by 2 since the "for" loop below terminates on
    // EfiMemoryMapEnd which is dependent upon EfiMemoryMapSize. Otherwize
    // we process bogus entries and create bogus E820 entries.
    //
    Status = gBS->AllocatePool (EfiBootServicesData, EfiMemoryMapSize, (VOID **)&EfiMemoryMap);
    ASSERT_EFI_ERROR (Status);
    Status = gBS->GetMemoryMap (
                    &EfiMemoryMapSize,
                    EfiMemoryMap,
                    &EfiMapKey,
                    &EfiDescriptorSize,
                    &EfiDescriptorVersion
                    );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (EfiMemoryMap);
    }
  }

  ASSERT_EFI_ERROR (Status);
  if(EfiMemoryMap == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }


  //
  // Punch in the E820 table for memory less than 1 MB.
  // Assume ZeroMem () has been done on data structure.
  //
  //
  // First entry is 0 to (640k - EBDA)
  //
  E820[0].BaseAddr  = 0;
  E820[0].Length    = (UINT64) ((*(UINT16 *) (UINTN)0x40E) << 4);
  E820[0].Type      = EfiAcpiAddressRangeMemory;
  E820[0].ExtAttributes = AddressRangeEnabled;

  //
  // Second entry is (640k - EBDA) to 640k
  //
  E820[1].BaseAddr  = E820[0].Length;
  E820[1].Length    = (UINT64) ((640 * 1024) - E820[0].Length);
  E820[1].Type      = EfiAcpiAddressRangeReserved;
  E820[1].ExtAttributes = AddressRangeEnabled;

  //
  // Third Entry is legacy BIOS
  // DO NOT CLAIM region from 0xA0000-0xDFFFF. OS can use free areas
  // to page in memory under 1MB.
  // Omit region from 0xE0000 to start of BIOS, if any. This can be
  // used for a multiple reasons including OPROMS.
  //

  //
  // The CSM binary image size is not the actually size that CSM binary used,
  // to avoid memory corrupt, we declare the 0E0000 - 0FFFFF is used by CSM binary.
  //
  E820[2].BaseAddr  = 0xE0000;
  E820[2].Length    = 0x20000;
  E820[2].Type      = EfiAcpiAddressRangeReserved;
  E820[2].ExtAttributes = AddressRangeEnabled;

  Above1MIndex = 2;

  //
  // Process the EFI map to produce E820 map;
  //

  //
  // Sort memory map from low to high
  //
  EfiEntry        = EfiMemoryMap;
  NextEfiEntry    = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
  while (EfiEntry < EfiMemoryMapEnd) {
    while (NextEfiEntry < EfiMemoryMapEnd) {
      if (EfiEntry->PhysicalStart > NextEfiEntry->PhysicalStart) {
        CopyMem (&TempEfiEntry, EfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
        CopyMem (EfiEntry, NextEfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
        CopyMem (NextEfiEntry, &TempEfiEntry, sizeof (EFI_MEMORY_DESCRIPTOR));
      }

      NextEfiEntry = NEXT_MEMORY_DESCRIPTOR (NextEfiEntry, EfiDescriptorSize);
    }

    EfiEntry      = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
    NextEfiEntry  = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  }

  EfiEntry        = EfiMemoryMap;
  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
  for (Index = Above1MIndex; (EfiEntry < EfiMemoryMapEnd) && (Index < EFI_MAX_E820_ENTRY - 1); ) {
    MemoryBlockLength = (UINT64) (LShiftU64 (EfiEntry->NumberOfPages, 12));
    if ((EfiEntry->PhysicalStart + MemoryBlockLength) < 0x100000) {
      //
      // Skip the memory block is under 1MB
      //
    } else {
      if (EfiEntry->PhysicalStart < 0x100000) {
        //
        // When the memory block spans below 1MB, ensure the memory block start address is at least 1MB
        //
        MemoryBlockLength       -= 0x100000 - EfiEntry->PhysicalStart;
        EfiEntry->PhysicalStart =  0x100000;
      }

      //
      // Convert memory type to E820 type
      //
      TempType = EfiMemoryTypeToE820Type (EfiEntry->Type);

      if (FeaturePcdGet(PcdE820TableNetBsdSupported)) {
        //
        // Win98 SE and NetBSD have a problem with intermixed E820 types 1,3,4. If type
        // 1 is 1 page it walks into the next type. If see a type 1 above
        // a type 3 or 4 then force type 1 to type 3.
        //
        if ((TempType == EfiAcpiAddressRangeMemory) && (EfiEntry->PhysicalStart > mLowWater)) {
          TempType = EfiAcpiAddressRangeACPI;
        } else {
          if (((TempType == EfiAcpiAddressRangeACPI) || (TempType == EfiAcpiAddressRangeNVS)) &&
              (EfiEntry->PhysicalStart < mLowWater)) {
            mLowWater = EfiEntry->PhysicalStart;
          }
        }
      }

      if ((E820[Index].Type == TempType) && (EfiEntry->PhysicalStart == (E820[Index].BaseAddr + E820[Index].Length))) {
        //
        // Grow an existing entry
        //
        E820[Index].Length += MemoryBlockLength;
      } else {
        //
        // Make a new entry
        //
        ++Index;
        E820[Index].BaseAddr  = EfiEntry->PhysicalStart;
        E820[Index].Length    = MemoryBlockLength;
        E820[Index].Type      = TempType;
        E820[Index].ExtAttributes = AddressRangeEnabled;
      }
    }
    EfiEntry = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  }

  FreePool (EfiMemoryMap);

  //
  // Process the reserved memory map to produce E820 map ;
  //
  for (Hob.Raw = GetHobList (); !END_OF_HOB_LIST (Hob); Hob.Raw = GET_NEXT_HOB (Hob)) {
    if (Hob.Raw != NULL && GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
      ResourceHob = Hob.ResourceDescriptor;
      if (((ResourceHob->ResourceType == EFI_RESOURCE_MEMORY_MAPPED_IO) ||
          (ResourceHob->ResourceType == EFI_RESOURCE_FIRMWARE_DEVICE)   ||
          (ResourceHob->ResourceType == EFI_RESOURCE_MEMORY_RESERVED))  &&
          (ResourceHob->PhysicalStart > 0x100000) &&
          (Index < EFI_MAX_E820_ENTRY - 1)) {
        ++Index;
        E820[Index].BaseAddr  = ResourceHob->PhysicalStart;
        E820[Index].Length    = ResourceHob->ResourceLength;
        E820[Index].Type      = EfiAcpiAddressRangeReserved;
      }
    }
  }

  Index ++;

  //
  // Sort / Merge / Remove the overlap range
  //
  Index = AdjustE820Table (E820, Index);
  //
  //  If set, the Address Range Descriptor represents non-volatile memory, this address
  //  is remapped System BIOS at end of address space.
  //
  if ((E820[Index].BaseAddr + E820[Index].Length) == 0x100000000) {
    E820[Index].ExtAttributes |= AddressRangeNonVolatile;
  }
  Private->IntThunk->EfiToLegacy16InitTable.NumberE820Entries = (UINT32)Index;
  Private->IntThunk->EfiToLegacy16BootTable.NumberE820Entries = (UINT32)Index;
  Private->NumberE820Entries = (UINT32)Index;
  *Size = (UINTN) (Index * sizeof (EFI_E820_ENTRY64));

  if (FeaturePcdGet(PcdE820TableNetBsdSupported)) {
    for (TempIndex = Above1MIndex; TempIndex < Index; TempIndex++) {
      if ((E820[TempIndex].Type == EfiAcpiAddressRangeMemory) && (E820[TempIndex].BaseAddr > mLowWater)) {
        E820[TempIndex].Type = EfiAcpiAddressRangeACPI;
      }
    }
  }

  //
  // Determine OS usable memory above 1Mb
  //
  Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb = 0x0000;
  for (TempIndex = Above1MIndex; TempIndex < Index; TempIndex++) {
    if (E820[TempIndex].BaseAddr >= 0x100000) {
      //
      // ACPIReclaimMemory is also usable memory for ACPI OS, after OS dumps all ACPI tables.
      //
      if ((E820[TempIndex].Type == EfiAcpiAddressRangeMemory) || (E820[TempIndex].Type == EfiAcpiAddressRangeACPI)) {
        Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb += (UINT32) (E820[TempIndex].Length);
      } else {
        continue;
      }
    }
  }

  //
  // Store the total memory, if more than 4G, assume 4G - 4k.
  //
  if (Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb > 0x100000000) {
    Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb = 0xfffff000;
  }

  Private->IntThunk->EfiToLegacy16InitTable.OsMemoryAbove1Mb = Private->IntThunk->EfiToLegacy16BootTable.OsMemoryAbove1Mb;

  //
  // Print DEBUG information
  //
  for (TempIndex = 0; TempIndex < Index; TempIndex++) {
    DEBUG((EFI_D_INFO, "E820[%2d]: 0x%16lx ---- 0x%16lx, Type = 0x%x, ExtAttributes = 0x%x\n",
      TempIndex,
      E820[TempIndex].BaseAddr,
      (E820[TempIndex].BaseAddr + E820[TempIndex].Length),
      E820[TempIndex].Type,
      E820[TempIndex].ExtAttributes
      ));
  }

  return EFI_SUCCESS;
}

/**
  Fill in the standard BDA and EBDA stuff prior to legacy Boot

  @param  Private      Legacy BIOS Instance data

  @retval EFI_SUCCESS  It should always work.

**/
EFI_STATUS
LegacyBiosCompleteBdaBeforeBoot (
  IN  LEGACY_BIOS_INSTANCE    *Private
  )
{
  BDA_STRUC                   *Bda;
  UINT16                      MachineConfig;
  DEVICE_PRODUCER_DATA_HEADER *SioPtr;

  Bda           = (BDA_STRUC *) ((UINTN) 0x400);
  MachineConfig = 0;

  SioPtr        = &(Private->IntThunk->EfiToLegacy16BootTable.SioData);
  Bda->Com1     = SioPtr->Serial[0].Address;
  Bda->Com2     = SioPtr->Serial[1].Address;
  Bda->Com3     = SioPtr->Serial[2].Address;
  Bda->Com4     = SioPtr->Serial[3].Address;

  if (SioPtr->Serial[0].Address != 0x00) {
    MachineConfig += 0x200;
  }

  if (SioPtr->Serial[1].Address != 0x00) {
    MachineConfig += 0x200;
  }

  if (SioPtr->Serial[2].Address != 0x00) {
    MachineConfig += 0x200;
  }

  if (SioPtr->Serial[3].Address != 0x00) {
    MachineConfig += 0x200;
  }

  Bda->Lpt1 = SioPtr->Parallel[0].Address;
  Bda->Lpt2 = SioPtr->Parallel[1].Address;
  Bda->Lpt3 = SioPtr->Parallel[2].Address;

  if (SioPtr->Parallel[0].Address != 0x00) {
    MachineConfig += 0x4000;
  }

  if (SioPtr->Parallel[1].Address != 0x00) {
    MachineConfig += 0x4000;
  }

  if (SioPtr->Parallel[2].Address != 0x00) {
    MachineConfig += 0x4000;
  }

  Bda->NumberOfDrives = (UINT8) (Bda->NumberOfDrives + Private->IdeDriveCount);
  if (SioPtr->Floppy.NumberOfFloppy != 0x00) {
    MachineConfig     = (UINT16) (MachineConfig + 0x01 + (SioPtr->Floppy.NumberOfFloppy - 1) * 0x40);
    Bda->FloppyXRate  = 0x07;
  }

  Bda->Lpt1_2Timeout  = 0x1414;
  Bda->Lpt3_4Timeout  = 0x1414;
  Bda->Com1_2Timeout  = 0x0101;
  Bda->Com3_4Timeout  = 0x0101;

  //
  // Force VGA and Coprocessor, indicate 101/102 keyboard
  //
  MachineConfig       = (UINT16) (MachineConfig + 0x00 + 0x02 + (SioPtr->MousePresent * 0x04));
  Bda->MachineConfig  = MachineConfig;

  return EFI_SUCCESS;
}

/**
  Fill in the standard BDA for Keyboard LEDs

  @param  This         Protocol instance pointer.
  @param  Leds         Current LED status

  @retval EFI_SUCCESS  It should always work.

**/
EFI_STATUS
EFIAPI
LegacyBiosUpdateKeyboardLedStatus (
  IN EFI_LEGACY_BIOS_PROTOCOL           *This,
  IN  UINT8                             Leds
  )
{
  LEGACY_BIOS_INSTANCE  *Private;
  BDA_STRUC             *Bda;
  UINT8                 LocalLeds;
  EFI_IA32_REGISTER_SET Regs;

  Bda                 = (BDA_STRUC *) ((UINTN) 0x400);

  Private             = LEGACY_BIOS_INSTANCE_FROM_THIS (This);
  LocalLeds           = Leds;
  Bda->LedStatus      = (UINT8) ((Bda->LedStatus &~0x07) | LocalLeds);
  LocalLeds           = (UINT8) (LocalLeds << 4);
  Bda->ShiftStatus    = (UINT8) ((Bda->ShiftStatus &~0x70) | LocalLeds);
  LocalLeds           = (UINT8) (Leds & 0x20);
  Bda->KeyboardStatus = (UINT8) ((Bda->KeyboardStatus &~0x20) | LocalLeds);
  //
  // Call into Legacy16 code to allow it to do any processing
  //
  ZeroMem (&Regs, sizeof (EFI_IA32_REGISTER_SET));
  Regs.X.AX = Legacy16SetKeyboardLeds;
  Regs.H.CL = Leds;

  Private->LegacyBios.FarCall86 (
    &Private->LegacyBios,
    Private->Legacy16Table->Compatibility16CallSegment,
    Private->Legacy16Table->Compatibility16CallOffset,
    &Regs,
    NULL,
    0
    );

  return EFI_SUCCESS;
}


/**
  Fill in the standard CMOS stuff prior to legacy Boot

  @param  Private      Legacy BIOS Instance data

  @retval EFI_SUCCESS  It should always work.

**/
EFI_STATUS
LegacyBiosCompleteStandardCmosBeforeBoot (
  IN  LEGACY_BIOS_INSTANCE    *Private
  )
{
  UINT8   Bda;
  UINT8   Floppy;
  UINT32  Size;
  UINT8   Temp;
  UINT8   Temp1;

  //
  // Update CMOS locations
  // 10 floppy
  // 12,19,1A - ignore as OS don't use them and there is no standard due
  //            to large capacity drives
  // CMOS 14 = BDA 40:10 plus bit 3(display enabled)
  //
  Bda = (UINT8)(*((UINT8 *)((UINTN)0x410)) | BIT3);

  //
  // Force display enabled
  //
  Floppy = 0x00;
  if ((Bda & BIT0) != 0) {
    Floppy = BIT6;
  }

  //
  // Check if 2.88MB floppy set
  //
  if ((Bda & (BIT7 | BIT6)) != 0) {
    Floppy = (UINT8)(Floppy | BIT1);
  }

  LegacyWriteStandardCmos (CMOS_10, Floppy);
  LegacyWriteStandardCmos (CMOS_14, Bda);

  //
  // Force Status Register A to set rate selection bits and divider
  //
  LegacyWriteStandardCmos (CMOS_0A, 0x26);

  //
  // redo memory size since it can change
  //
  Size = 63 * SIZE_1MB;
  Temp  = 0x00;
  Temp1 = 0xFC;
  if (Private->IntThunk->EfiToLegacy16InitTable.OsMemoryAbove1Mb < (63 * SIZE_1MB)) {
    Size  = Private->IntThunk->EfiToLegacy16InitTable.OsMemoryAbove1Mb >> 10;
    Temp      = (UINT8) (Size & 0xFF);
    Temp1     = (UINT8) (Size >> 8);
  }

  LegacyWriteStandardCmos (CMOS_17, Temp);
  LegacyWriteStandardCmos (CMOS_30, Temp);
  LegacyWriteStandardCmos (CMOS_18, Temp1);
  LegacyWriteStandardCmos (CMOS_31, Temp1);

  LegacyCalculateWriteStandardCmosChecksum ();

  return EFI_SUCCESS;
}

/**
  Relocate this image under 4G memory for IPF.

  @param  ImageHandle  Handle of driver image.
  @param  SystemTable  Pointer to system table.

  @retval EFI_SUCCESS  Image successfully relocated.
  @retval EFI_ABORTED  Failed to relocate image.

**/
EFI_STATUS
RelocateImageUnder4GIfNeeded (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  return EFI_SUCCESS;
}
