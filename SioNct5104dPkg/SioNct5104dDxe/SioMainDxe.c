/** @file

SIO main code, it include create SCU, install device protocol, init in DXE stage

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
**/

#include "SioInitDxe.h"

UINT8                 *mExtensiveTablePtr;
SIO_DEVICE_LIST_TABLE *mTablePtr;

extern UINT16  mSioCfgPortList[];
extern UINT16  mSioCfgPortListSize;

/**
  The entry point of the SIO driver.

  @param [in] ImageHandle            A handle for the image that is initializing this driver
  @param [in] SystemTable            A pointer to the EFI system table

  @retval EFI_SUCCESS           Function complete successfully.
**/
EFI_STATUS
EFIAPI
SioDriverEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                        Status;
  EFI_EVENT                         ReadyToBootEvent;
  EFI_SIO_TABLE                     *OemTable;
  UINTN                             Index;

  mTablePtr = (SIO_DEVICE_LIST_TABLE *)PCD_SIO_CONFIG_TABLE;
  mExtensiveTablePtr = (UINT8*)PCD_SIO_EXTENSIVE_CONFIG_TABLE;
  OemTable = (EFI_SIO_TABLE*)PCD_SIO_DXE_CUSTOMIZE_SETTING_TABLE;

  //
  // Get SIO from PCD, if it fail, we don't install SIO DXE driver
  //
  Status = CheckDevice ();
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  mSioResourceFunction->ScriptFlag = TRUE;
  EnterConfigMode ();

  for (Index = 0; Index < PCD_SIO_DXE_CUSTOMIZE_SETTING_TABLE_SIZE / sizeof (EFI_SIO_TABLE); Index++) {
    IDW8 (OemTable->Register, OemTable->Value, mSioResourceFunction);
    OemTable++;
  }

  //
  // Create SCU page and Variable, then update to PCD data
  //
  if (PCD_SIO_SETUP_SUPPORT) {
    Status = SioScu ();
    ASSERT_EFI_ERROR (Status);
  }

  // Install each device protocol
  InstallEnabledDeviceProtocol ();

  //
  // Follow PcdSio$(SIO_NAME)ExtensiveCfg table to program device
  //
  ProgramExtensiveDevice();

  //
  // Unlock super IO config mode
  //
  ExitConfigMode();
  mSioResourceFunction->ScriptFlag = FALSE;

  //
  // Create a Ready to Boot Event for initialization of SIO SSDT
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             UpdateSsdt,
             NULL,
             &ReadyToBootEvent
             );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
  To get correct SIO data.

  @retval EFI_SUCCESS           Found SIO data.
  @retval EFI_NOT_FOUND         Not found.
**/
EFI_STATUS
CheckDevice (
  VOID
  )
{
  SIO_DEVICE_LIST_TABLE             *PcdPointer;

  PcdPointer = mTablePtr;

  //
  // Calculate the number of non-zero entries in the table
  //
  while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
    if (PcdPointer->Device == CFG_ID) {
      if (CheckConfig (PcdPointer) != 0) {
        return EFI_SUCCESS;
      }
    }
    PcdPointer++;
  }

  return EFI_NOT_FOUND;
}

/**
  Check ID of SIO whether it's correct

  @param[in]  *DeviceTable       SIO_DEVICE_LIST_TABLE from PCD structure.

  @retval     IndexPort          return value as zero, it means "not match".
                                 return value is not zero, it means "match".
**/
UINT16
CheckConfig (
  IN SIO_DEVICE_LIST_TABLE      *DeviceTable
  )
{
  UINT16                IndexPort;
  UINT16                DataPort;
  UINT8                 Index;

  IndexPort = DeviceTable->DeviceBaseAdr;
  DataPort  = IndexPort + 1;
  mSioResourceFunction->SioCfgPort = IndexPort;

  //
  // Verify define config port
  //
  if (IndexPort != 0) {
    //
    // Enter Config Mode
    //
    EnterConfigMode ();

    SioWrite8 (IndexPort, SIO_CHIP_ID1, mSioResourceFunction->ScriptFlag);
    if (SioRead8 (DataPort) != DeviceTable->TypeH) {
      IndexPort = 0;
    }

    if ((DeviceTable->TypeL != NONE_ID) && (IndexPort != 0)) {
      SioWrite8 (IndexPort, SIO_CHIP_ID2, mSioResourceFunction->ScriptFlag);
      if (SioRead8 (DataPort) != DeviceTable->TypeL) {
        IndexPort = 0;
      }
    }
  }

  //
  // Auto scan config port
  //
  if (IndexPort == 0) {

    for (Index = 0; Index < mSioCfgPortListSize; Index++) {
      IndexPort = mSioCfgPortList[Index];
      DataPort  = IndexPort + 1;
      mSioResourceFunction->SioCfgPort = IndexPort;
      //
      // Enter Config Mode
      //
      EnterConfigMode ();

      SioWrite8 (IndexPort, SIO_CHIP_ID1, mSioResourceFunction->ScriptFlag);
      if (SioRead8 (DataPort) == DeviceTable->TypeH) {
        if (DeviceTable->TypeL != NONE_ID) {
          SioWrite8 (IndexPort, SIO_CHIP_ID2, mSioResourceFunction->ScriptFlag);
          if (SioRead8 (DataPort) == DeviceTable->TypeL) {
            break;
          }
        } else {
          break;
        }
      }
    }

    if (Index == mSioCfgPortListSize) {  // if true, means not found SIO ID
      IndexPort = 0;
      return IndexPort;
    }
  }

  //
  // if it's right, update to EFI_SIO_RESOURCE_FUNCTION
  //
  DeviceTable->DeviceBaseAdr     = IndexPort;
  mSioResourceFunction->Instance = DeviceTable->TypeInstance;

  return IndexPort;
}

/**
 This function calculates and updates an UINT8 checksum.

 @param [in]   Buffer           Pointer to buffer to checksum
 @param [in]   Size             Number of bytes to checksum
 @param [in]   ChecksumOffset   Offset to place the checksum result in

 @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
SioAcpiChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  )
{
  UINT8 Sum;
  UINT8 *Ptr;
 
  Sum = 0;
  //
  // Initialize pointer
  //
  Ptr = Buffer;
 
  //
  // set checksum to 0 first
  //
  Ptr[ChecksumOffset] = 0;
 
  //
  // add all content of buffer
  //
  while (Size--) {
    Sum = (UINT8) (Sum + (*Ptr++));
  }
  //
  // set checksum
  //
  Ptr                 = Buffer;
  Ptr[ChecksumOffset] = (UINT8) (0xff - Sum + 1);
 
  return EFI_SUCCESS;
}

/**
  Find SSDT table from RAW section of DXE driver, then publish it to ACPI table

  @param [in]  Event           Event whose notification function is being invoked
  @param [in]  Context         Pointer to the notification function's context

**/
VOID
UpdateSsdt (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  EFI_STATUS                    Status;
  VOID                          *Buffer ;
  UINTN                         Size;
  UINTN                         Instance;
  UINTN                         AcpiTableHandle;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport;
  EFI_ACPI_DESCRIPTION_HEADER   *SioAcpiTable;
  EFI_GUID                      DriverGuid = SIO_DXE_DRIVER_GUID;
  EFI_GUID                      Guid = EFI_ACPI_SUPPORT_GUID;

  Size = 0;
  Status = EFI_SUCCESS;
  Buffer = NULL;
  Instance = 0;
  AcpiSupport = NULL;
  SioAcpiTable = NULL;
  AcpiTableHandle = 0;

  //
  // By default, a table belongs in all ACPI table versions published.
  //
  Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  //
  // Locate ACPI support protocol
  //
  Status = gBS->LocateProtocol (
                  &Guid,
                  NULL,
                  (VOID **) &AcpiSupport
                  );
  ASSERT_EFI_ERROR (Status);

  while (Status == EFI_SUCCESS) {

    Status = GetSectionFromAnyFv (&DriverGuid, EFI_SECTION_RAW, Instance, &Buffer, &Size);

    if (Status == EFI_SUCCESS) {
      //
      // Check the Signature ID to modify the table
      //
      switch (((EFI_ACPI_DESCRIPTION_HEADER*) Buffer)->OemTableId) {

      //
      // If find SSDT table, we'll update NVS area resource and name string
      //
      case (EFI_SIGNATURE_64 ('S', 'I', 'O', 'A', 'C', 'P', 'I', 0)):
        SioAcpiTable = (EFI_ACPI_DESCRIPTION_HEADER*) Buffer;
        if (PCD_SIO_UPDATE_ASL) {
          UpdateSioMbox (SioAcpiTable);
        }
      break;

      default:
      break;
      }
    }

    Instance++;
    Buffer = NULL;
  }

  if (SioAcpiTable != NULL) {
    //
    // Update the Sio SSDT table in the ACPI tables.
    //
    Status = AcpiSupport->SetAcpiTable (
                            AcpiSupport,
                            SioAcpiTable,
                            TRUE,
                            Version,
                            &AcpiTableHandle
                            );

    ASSERT_EFI_ERROR (Status);
    SioAcpiChecksum ((SioAcpiTable), SioAcpiTable->Length, EFI_FIELD_OFFSET (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

    //
    // Publish all ACPI Tables
    //
    Status = AcpiSupport->PublishTables (AcpiSupport, Version);
    ASSERT_EFI_ERROR (Status);

    //
    // ACPI support makes its own copy of the table, so clean up
    //
    gBS->FreePool (SioAcpiTable);
  }

  //
  // Close event
  //
  gBS->CloseEvent (Event);

}

