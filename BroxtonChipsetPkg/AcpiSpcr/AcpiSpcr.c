/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <PiDxe.h>
#include <AcpiSpcr.h>
#include <Protocol/AcpiTable.h>
#include <IndustryStandard/Acpi30.h>
#include <IndustryStandard/Acpi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/KernelConfigLib.h>
#include <Library/UefiBootServicesTableLib.h>

//
// Serial Port Console Redirection Table
// Please modify all values in Spcr.h only.
//
EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE SPCR = {
  {
    EFI_ACPI_3_0_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_SIGNATURE,
    sizeof (EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE),
    EFI_ACPI_SERIAL_PORT_CONSOLE_REDIRECTION_TABLE_REVISION,
    0,
    EFI_ACPI_OEM_ID,
    EFI_ACPI_OEM_TABLE_ID,
    EFI_ACPI_OEM_REVISION,
    EFI_ACPI_CREATOR_ID,
    EFI_ACPI_CREATOR_REVISION,
  },

  SPCR_INTERFACE_TYPE,
  SPCR_RESERVED01,
  SPCR_BASE_ADDRESS,
  SPCR_INTERRUPT_TYPE,
  SPCR_IRQ_NUM,
  SPCR_GLOBAL_SYS_INTERRUPT,
  SPCR_BAUD_RATE,
  SPCR_PARITY,
  SPCR_STOP_BITS,
  SPCR_FLOW_CONTROL,
  SPCR_TERMINAL_TYPE,
  SPCR_RESERVED02,
  SPCR_PCI_DID,
  SPCR_PCI_VID,
  SPCR_PCI_BUS,
  SPCR_PCI_DEV,
  SPCR_PCI_FUNC,
  SPCR_PCI_FLAGS,
  SPCR_PCI_SEG,
  SPCR_RESERVED03
};


EFI_STATUS
SpcrUpdateCommon (
  IN OUT EFI_ACPI_COMMON_HEADER       *Table,
  IN KERNEL_CONFIGURATION             *SetupVariable
);

/**
  This function calculates and updates an UINT8 checksum.

  @param  Buffer          Pointer to buffer to checksum
  @param  Size            Number of bytes to checksum

**/
VOID
AcpiPlatformChecksum (
  IN UINT8      *Buffer,
  IN UINTN      Size
  )
{
  UINTN ChecksumOffset;

  ChecksumOffset = OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum);

  //
  // Set checksum to 0 first
  //
  Buffer[ChecksumOffset] = 0;

  //
  // Update checksum value
  //
  Buffer[ChecksumOffset] = CalculateCheckSum8 (Buffer, Size);
}



/**
  Entrypoint of Acpi SPCR driver.

  @param  ImageHandle Handle for this drivers loaded image protocol.
  @param  SystemTable EFI system table.

  @return EFI_SUCCESS The driver installed without error.
  @return EFI_ABORTED the driver encountered an error and could not complete installation of the ACPI tables.

**/
EFI_STATUS
EFIAPI
AcpiSpcrEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTableProtocol;
  EFI_ACPI_COMMON_HEADER         *CurrentTable;
  UINTN                          TableSize;
  UINTN                          Size;
  KERNEL_CONFIGURATION           KernelConfiguration;
  EFI_STATUS                     TableStatus;
  UINTN                          TableHandle;

  //
  // Find the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**)&AcpiTableProtocol);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return EFI_ABORTED;
  }

  Status = GetKernelConfiguration (&KernelConfiguration);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  

  CurrentTable = (EFI_ACPI_COMMON_HEADER *)&SPCR;
  Size = sizeof(SPCR);
  
  TableStatus = SpcrUpdateCommon (CurrentTable, &KernelConfiguration);
  if (TableStatus == EFI_SUCCESS) {
    //
    // Add the table
    //
    TableHandle = 0;
    TableSize = ((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->Length;
    ASSERT (Size >= TableSize);

    //
    // Checksum ACPI table
    //
    AcpiPlatformChecksum ((UINT8*)CurrentTable, TableSize);

    //
    // Install ACPI table
    //
    Status = AcpiTableProtocol->InstallAcpiTable (
                                  AcpiTableProtocol,
                                  CurrentTable,
                                  TableSize,
                                  &TableHandle
                                  );
  }
  
  if (EFI_ERROR (TableStatus)) {
    DEBUG ((EFI_D_ERROR, "\nWARNING: Table signature=0x%x was removed!!\n", CurrentTable->Signature));
  }

  
  return EFI_SUCCESS;
}
