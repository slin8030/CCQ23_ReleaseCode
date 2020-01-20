/** @file

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

#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/DevicePathLib.h>
#include <Library/BdsCpLib.h>

#include <Guid/H2OBdsCheckPoint.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/PcAnsi.h>

#include <Protocol/AcpiSupport.h>
#include <Protocol/SerialIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/IsaIo.h>
#include <Protocol/PciIo.h>

#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/SerialPortConsoleRedirectionTable.h>
#include <IndustryStandard/Pci22.h>

#include <KernelSetupConfig.h>

#define EISA_SERIAL_DEVICE_ID    0x0501
//
// ACPI table information used to initialize tables.
//
#define ACPI_SPCR_ENABLE      1
#define SPCR_DEVICE_ISA       1
#define SPCR_DEVICE_PCI       2
#define SPCR_DEVICE_NONE      128

#define SPCR_VT100_TYPE       0
#define SPCR_VT100PLUS_TYPE   1
#define SPCR_VTUTF8_TYPE      2
#define SPCR_PCANSI_TYPE      3

#define PREFER_SPCR_DEVICE_TYPE      SPCR_DEVICE_ISA

typedef struct {
  UINT8                 DeviceType;
  UINT8                 TerminalType;
  UINT8                 FlowControl;
  EFI_SERIAL_IO_MODE    *Mode;
  UINT32                IoAddress;
  UINT32                IrqNum;
  UINTN                 PciSegment;
  UINTN                 PciBus;
  UINTN                 PciDevice;
  UINTN                 PciFunction;
  PCI_TYPE00            PciHeader;
} SPCR_SERIAL_PARAMETER;

EFI_ACPI_SUPPORT_PROTOCOL   *gAcpiSupport = NULL;

/**
  Update SPCR table content

  @param  VOID

  @return EFI_SUCCESS     Update table success
  @return EFI_NOT_FOUND   Not found the table

**/
EFI_STATUS
GetSpcrTable (
  OUT EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE  **Table,
  OUT EFI_ACPI_TABLE_VERSION                          *Version,
  OUT UINTN                                           *TableKey

  )
{
  EFI_STATUS                                      Status;
  INTN                                            Index;

  if (Table == NULL || Version == NULL || TableKey == NULL) {
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }

  //
  // Search SPCR table
  //
  Index = 0;
  while (1) {
    Status = gAcpiSupport->GetAcpiTable (gAcpiSupport, Index, (VOID **)Table, Version, TableKey);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    //
    // Check Signture and update SPCR table
    //
    if ((*Table)->Header.Signature == EFI_ACPI_3_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE) {
      return EFI_SUCCESS;
    }

    //
    // Not found, get next
    //
    gBS->FreePool ( *Table );
    Index++;
  }

}

/**
  Update SPCR table content

  @param [in] SerialParameter         The serial parameter

  @return EFI_SUCCESS       Update table success
  @return EFI_NOT_FOUND   Not found the table

**/
EFI_STATUS
UpdateSpcrTable (
  IN SPCR_SERIAL_PARAMETER        *SerialParameter
  )
{
  UINT8                                           BaudRateValue;
//-  EFI_ACPI_SUPPORT_PROTOCOL                       *AcpiSupport;
  EFI_STATUS                                      Status;
//-  INTN                                            Index;
  UINTN                                           TableKey;
  EFI_ACPI_TABLE_VERSION                          Version;
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE  *Table;


  Status = GetSpcrTable (&Table, &Version, &TableKey);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (SerialParameter->DeviceType == SPCR_DEVICE_ISA) {
    //
    // isa serial device
    //
    Table->BaseAddress.Address = SerialParameter->IoAddress;
    Table->Irq = (UINT8) SerialParameter->IrqNum;

  } else if (SerialParameter->DeviceType == SPCR_DEVICE_PCI) {
    //
    // pci serial device
    //
    Table->PciVendorId = SerialParameter->PciHeader.Hdr.VendorId;
    Table->PciDeviceId = SerialParameter->PciHeader.Hdr.DeviceId;
    Table->PciSegment = (UINT8) SerialParameter->PciSegment;
    Table->PciBusNumber = (UINT8) SerialParameter->PciBus;
    Table->PciDeviceNumber = (UINT8) SerialParameter->PciDevice;
    Table->PciFunctionNumber = (UINT8) SerialParameter->PciFunction;

  } else {
    return EFI_UNSUPPORTED;
  }

  switch (SerialParameter->Mode->BaudRate) {
  case 115200 :
    BaudRateValue = 7;
    break;
  case 57600 :
    BaudRateValue = 6;
    break;
  case 19200 :
    BaudRateValue = 4;
    break;
  case 9200 :
    BaudRateValue = 3;
    break;
  default :
    ASSERT (FALSE);
    BaudRateValue = 7;
  }
  Table->BaudRate = BaudRateValue;

  Table->Parity   = (UINT8) SerialParameter->Mode->Parity;
  if (Table->Parity != 0) {
    ASSERT(FALSE);
    Table->Parity = 0;
  }

  Table->StopBits = (UINT8) SerialParameter->Mode->StopBits;
  if (Table->StopBits != 1) {
    ASSERT(FALSE);
    Table->StopBits = 1;
  }

  Table->TerminalType = SerialParameter->TerminalType;
  //Table->FlowControl  = SerialParameter->FlowControl;

  Status = gAcpiSupport->SetAcpiTable (gAcpiSupport, Table, FALSE, Version, &TableKey);

  return Status;
}

VOID
TruncateLastNode (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *LastNode;
  EFI_DEVICE_PATH_PROTOCOL    *DevPath;

  LastNode = NULL;
  DevPath = DevicePath;

  while (!IsDevicePathEnd(DevPath)) {
      LastNode = DevPath;
      DevPath = NextDevicePathNode (DevPath);
  }

  if (LastNode != NULL) {
    CopyMem( LastNode, DevPath, sizeof(EFI_DEVICE_PATH_PROTOCOL));
  }

  return;
}

EFI_STATUS
GetParentHandle (
  EFI_HANDLE      ChildHandle,
  EFI_HANDLE      *ParentHandle
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    Handle;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL      *TmpDevPath;

  Status = gBS->HandleProtocol (ChildHandle, &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  TmpDevPath = DuplicateDevicePath (DevicePath);
  if (TmpDevPath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  TruncateLastNode (TmpDevPath);

  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &TmpDevPath, &Handle);
  if (EFI_ERROR(Status)) {
    goto Exit;
  }

  *ParentHandle = Handle;

Exit:

  gBS->FreePool (TmpDevPath);

  return Status;
}

/**
 Check whether a give device path is an ISA serial device

 @param [in] DevicePath         The device path to be processed

 @return TRUE                   The device path is an ISA serial device
 @retval FALSE                  The device path is not an ISA serial device

**/
BOOLEAN
IsIsaSerialDevice (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  ACPI_HID_DEVICE_PATH      *AcpiDevPath;

  if (DevicePath == NULL)
    return FALSE;

  while (!IsDevicePathEnd(DevicePath)) {

    if ((DevicePathType(DevicePath) == ACPI_DEVICE_PATH)&&(DevicePathSubType(DevicePath) == ACPI_DP)) {

      AcpiDevPath = (ACPI_HID_DEVICE_PATH *) DevicePath;
      if (AcpiDevPath->HID == EISA_PNP_ID(EISA_SERIAL_DEVICE_ID)) {
        return TRUE;
      }
    }
    DevicePath = NextDevicePathNode(DevicePath);
  }

  return FALSE;
}

/**
 Check whether a give device path is a PCI serial device

 @param [in] DevicePath         The device path to be processed

 @return TRUE                   The device path is a PCI serial device
 @retval FALSE                  The device path is not a PCI serial device

**/
BOOLEAN
IsPciSerialDevice (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *NextNode;

  if (DevicePath == NULL)
    return FALSE;


  while (!IsDevicePathEnd(DevicePath)) {
    if ((DevicePathType(DevicePath) == HARDWARE_DEVICE_PATH) && (DevicePathSubType(DevicePath) == HW_PCI_DP)) {
      NextNode = NextDevicePathNode(DevicePath);
      if ((DevicePathType(NextNode) == MESSAGING_DEVICE_PATH) && (DevicePathSubType(NextNode) == MSG_UART_DP)) {
        return TRUE;
      }
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }

  return FALSE;
}

/**
 Check whether a give Handle is a platform console out device

 @param [in] Handle         The handle to be processed

 @return TRUE                   The handle is a platform console out device
 @retval FALSE                  The handle is not a platform console out device

**/
BOOLEAN
IsPlatformConOutDevice (
  EFI_HANDLE        Handle
  )
{
  EFI_STATUS                          Status;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *EntryBuffer;
  UINTN                               EntryCount;
  UINTN                               Index;
  VOID                                *Interface;

  Status = gBS->OpenProtocolInformation (
                  Handle,
                  &gEfiSerialIoProtocolGuid,
                  &EntryBuffer,
                  &EntryCount
                  );
  if (EFI_ERROR(Status)) {
    return FALSE;
  }

  for (Index=0; Index< EntryCount; Index++) {
    if ((EntryBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) != 0) {
      Status = gBS->HandleProtocol ( EntryBuffer[Index].ControllerHandle, &gEfiConsoleOutDeviceGuid, &Interface);
      if (!EFI_ERROR(Status)) {
        FreePool(EntryBuffer);
        return TRUE;
      }
    }
  }

  FreePool(EntryBuffer);

  return FALSE;
}

EFI_STATUS
SpcrLocateSerialHandle (
  OUT EFI_HANDLE        *Handle
  )
{
  EFI_STATUS                Status;
  UINTN                     NumHandles;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     Index;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  UINT16                    *Priority;
  UINT16                    SelectedValue;
  UINTN                     SelectedIndex;
  UINT16                    IsaDevWeighted;
  UINT16                    PciDevWeighted;

  if (Handle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiSerialIoProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Initialize variable
  //
  IsaDevWeighted = BIT2;
  PciDevWeighted = BIT2;

  if (PREFER_SPCR_DEVICE_TYPE == SPCR_DEVICE_ISA) {
    IsaDevWeighted = BIT3;
  } else {
    PciDevWeighted = BIT3;
  }

  Priority = AllocateZeroPool(sizeof(UINT16) * NumHandles);
  if (Priority == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Evaluate priority of every serial handle.
  // The priority is as list :
  // 1. Console device
  // 2. Isa device
  // 3. Pci device
  //
  for (Index = 0; Index < NumHandles; Index++) {

    // if that is platform conout device add weighted
    if (IsPlatformConOutDevice(HandleBuffer[Index])) {
      Priority[Index] = BIT7;
    }

    // SPCR serial device must be physical controller
    Status = gBS->HandleProtocol (HandleBuffer[Index], &gEfiDevicePathProtocolGuid, &DevPath);
    if (EFI_ERROR(Status)) {
      continue;
    }

    // add weighted by device type
    if (IsIsaSerialDevice (DevPath)) {
      Priority[Index] = IsaDevWeighted;
    } else if (IsPciSerialDevice(DevPath)){
      Priority[Index] = PciDevWeighted;
    } else {
      Priority[Index] = 0;
    }
  }

  SelectedIndex = 0;
  SelectedValue = 0;
  for (Index = 0; Index < NumHandles; Index++) {
    if (Priority[Index] > SelectedValue) {
      SelectedValue = Priority[Index];
      SelectedIndex = Index;
    }
  }

  if (SelectedValue == 0) {
    return EFI_NOT_FOUND;
  }

  *Handle = HandleBuffer[SelectedIndex];

  return EFI_SUCCESS;
}

UINT8
GetSerialTerminalType (
  EFI_HANDLE              Handle
  )
{
  EFI_STATUS                            Status;
  EFI_SERIAL_IO_PROTOCOL                *SerialIo;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY   *EntryBuffer;
  UINTN                                 EntryCount;
  UINTN                                 Index;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  VENDOR_DEVICE_PATH                    *Node;
  UINT8                                 TerminalType;

  TerminalType = SPCR_VT100_TYPE;

  Status = gBS->HandleProtocol (Handle, &gEfiSerialIoProtocolGuid, &SerialIo);
  ASSERT_EFI_ERROR(Status);

  Status = gBS->OpenProtocolInformation (
                  Handle,
                  &gEfiSerialIoProtocolGuid,
                  &EntryBuffer,
                  &EntryCount
                  );
  if (EFI_ERROR(Status)) {
    ASSERT (FALSE);
    return SPCR_VT100_TYPE;
  }

  for (Index = 0; Index < EntryCount; Index++) {

    if ((EntryBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) == 0) {
      continue;
    }

    Status = gBS->HandleProtocol (EntryBuffer[Index].ControllerHandle, &gEfiDevicePathProtocolGuid, &DevicePath);
    if (EFI_ERROR(Status)) {
      ASSERT (FALSE);
      continue;
    }

    while (!IsDevicePathEnd(DevicePath)) {
      if ((DevicePathType(DevicePath) == MESSAGING_DEVICE_PATH) && (DevicePathSubType(DevicePath) == MSG_VENDOR_DP)) {
        Node = (VENDOR_DEVICE_PATH *)DevicePath;
        if ( CompareGuid (&Node->Guid, &gEfiVT100Guid)) {
          TerminalType = SPCR_VT100_TYPE;
        } else if (CompareGuid (&Node->Guid, &gEfiVT100PlusGuid)) {
          TerminalType = SPCR_VT100PLUS_TYPE;
        } else if (CompareGuid (&Node->Guid, &gEfiVTUTF8Guid)) {
          TerminalType = SPCR_VTUTF8_TYPE;
        } else if (CompareGuid (&Node->Guid, &gEfiPcAnsiGuid)) {
          TerminalType = SPCR_PCANSI_TYPE;
        } else {
          ASSERT(FALSE);
          TerminalType = SPCR_VT100_TYPE;
        }
        goto Exit;
      }
      DevicePath = NextDevicePathNode(DevicePath);
    }
  }

Exit:

  FreePool (EntryBuffer);
  return TerminalType;
}

EFI_STATUS
RetriveSerialParameter (
  IN  EFI_HANDLE              SerialHandle,
  OUT SPCR_SERIAL_PARAMETER   *SerialParameter
  )
{
  EFI_STATUS                  Status;
  EFI_SERIAL_IO_PROTOCOL      *SerialIo;
  EFI_HANDLE                  ParentHandle;
  EFI_ISA_IO_PROTOCOL         *IsaIo;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  EFI_ISA_ACPI_RESOURCE       *IsaResource;
  UINTN                       Segment;
  UINTN                       Bus;
  UINTN                       Device;
  UINTN                       Function;

  Status = gBS->HandleProtocol ( SerialHandle, &gEfiSerialIoProtocolGuid, &SerialIo);
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  SerialParameter->TerminalType = GetSerialTerminalType (SerialHandle);
  SerialParameter->Mode = SerialIo->Mode;

  Status = GetParentHandle (SerialHandle, &ParentHandle);
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (!EFI_ERROR(gBS->HandleProtocol (ParentHandle, &gEfiIsaIoProtocolGuid, &IsaIo))) {
    //
    // Isa serial device
    //
    SerialParameter->DeviceType = SPCR_DEVICE_ISA;

    IsaResource = IsaIo->ResourceList->ResourceItem;
    while (IsaResource->Type != EfiIsaAcpiResourceEndOfList) {
      switch (IsaResource->Type) {
      case EfiIsaAcpiResourceIo :
        SerialParameter->IoAddress = IsaResource->StartRange;
        break;
      case EfiIsaAcpiResourceInterrupt :
        SerialParameter->IrqNum = IsaResource->StartRange;
        break;
      }
      IsaResource++;
    }

    return EFI_SUCCESS;

  } else  if (!EFI_ERROR(gBS->HandleProtocol (ParentHandle, &gEfiPciIoProtocolGuid, &PciIo))) {
    //
    // Pci Serial device
    //
    SerialParameter->DeviceType = SPCR_DEVICE_PCI;

    Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0, sizeof (PCI_TYPE00), &SerialParameter->PciHeader);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    SerialParameter->PciSegment  = Segment;
    SerialParameter->PciBus      = Bus;
    SerialParameter->PciDevice   = Device;
    SerialParameter->PciFunction = Function;

    return EFI_SUCCESS;

  } else {
    //
    // Unknow device
    //
    ASSERT(FALSE);

    return EFI_UNSUPPORTED;
  }

}

EFI_STATUS
UnInstallSpcr (
  VOID
  )
{
  EFI_STATUS                                      Status;
  EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE  *Table;
  EFI_ACPI_TABLE_VERSION                          Version;
  UINTN                                           TableKey;

  Status = GetSpcrTable (&Table, &Version, &TableKey);
  if (EFI_ERROR(Status)) {
    return EFI_SUCCESS;
  }

  Status = gAcpiSupport->SetAcpiTable ( gAcpiSupport, NULL, FALSE, Version, &TableKey);
  return EFI_SUCCESS;
}

/**
  Console Redirection Callback

  @param  Event
  @param  *Context

  @return VOID

**/
VOID
EFIAPI
AfterConnectAllCp (
  IN EFI_EVENT         Event,
  IN H2O_BDS_CP_HANDLE CpHandle
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              Handle;
  SPCR_SERIAL_PARAMETER   SerialParameter;

  // locate the primary serial device handle
  Status = SpcrLocateSerialHandle (&Handle);
  if (EFI_ERROR(Status)) {
    UnInstallSpcr ();
    return;
  }

  // Retrive serial parameter
  Status = RetriveSerialParameter (Handle, &SerialParameter);
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status)) {
    UnInstallSpcr ();
    return;
  }

  // Use serial parameter to modify SPCR table
  Status = UpdateSpcrTable (&SerialParameter);
  ASSERT_EFI_ERROR(Status);

  return;
}

/**
  Update SPCR table content
  Depends on Setup Variable to update SPCR table.

  @param  Table           The table to update
  @param  SetupVariable   SETUP Variable pointer

  @return EFI_SUCCESS     Update table success

**/
EFI_STATUS
SpcrUpdateCommon (
  IN OUT EFI_ACPI_COMMON_HEADER       *Table,
  IN KERNEL_CONFIGURATION             *SetupVariable
)
{
  EFI_STATUS          Status;
  EFI_HANDLE          CpHandle;

  if (SetupVariable->ACPISpcr == ACPI_SPCR_ENABLE) {

    Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, &gAcpiSupport);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    Status = BdsCpRegisterHandler (
                &gH2OBdsCpConnectAllAfterProtocolGuid,
                AfterConnectAllCp,
                H2O_BDS_CP_MEDIUM,
                &CpHandle
                );
  } else {

    Status = EFI_UNSUPPORTED;
  }

  return Status;
}

