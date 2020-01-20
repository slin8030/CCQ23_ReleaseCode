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
*/

#include "SioInitDxe.h"

SIO_DEVICE_LIST_TABLE               *mTablePtr;
UINT8                               *mDmiTablePtr;
extern UINT16                       mSioCfgPortList[];
extern EFI_SIO_TABLE                mSioTable[];
extern BOOLEAN                      mFirstBoot;
extern EFI_GUID                     mSioFormSetGuid;
extern VOID                         EnterConfigMode ();
extern VOID                         ExitConfigMode ();
extern VOID  ProgramGpio (SIO_EXTENSIVE_TABLE_MAINTYPE*, SIO_DEVICE_LIST_TABLE*, SIO_CONFIGURATION*);
extern VOID  ProgramLpt  (SIO_EXTENSIVE_TABLE_MAINTYPE*, SIO_DEVICE_LIST_TABLE*, SIO_CONFIGURATION*); 
extern VOID  ProgramCom  (SIO_EXTENSIVE_TABLE_MAINTYPE*, SIO_DEVICE_LIST_TABLE*, SIO_CONFIGURATION*);
extern CHAR16 *mSioVariableName;

//[-start-140116-IB12970054-modify]//
extern UINT16 mSioCfgPortListSize;
//[-end-140116-IB12970054-modify]//

extern EFI_INSTALL_DEVICE_FUNCTION mDeviceFunction[];
extern EFI_SIO_RESOURCE_FUNCTION mSioResourceFunction[];

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

  mTablePtr = (SIO_DEVICE_LIST_TABLE *)PcdGetPtr (PcdSioWpcd374lCfg);
  mDmiTablePtr = (UINT8*)PcdGetPtr (PcdSioWpcd374lDmi);

  //
  // Get SIO from PCD, if it fail, we don't install SIO DXE driver
  //
  Status = CheckDevice ();  
  if (Status != EFI_SUCCESS) {
    return Status;
  }
  mSioResourceFunction->ScriptFlag = TRUE;
  EnterConfigMode ();

  //
  // OemService
  //
  Status = OemSvcRegSioDxe (
           mSioResourceFunction->Instance,
           &OemTable
           );
  if (Status != EFI_SUCCESS) {
    OemTable = mSioTable;
  } 

  while ((OemTable->Register != 0) || (OemTable->Value != 0)) {
    IDW8 (OemTable->Register, OemTable->Value, mSioResourceFunction);
    OemTable++;
  }

  //
  // Create SCU page and Variable, then update to PCD data
  //
  if (FeaturePcdGet (PcdSioWpcd374lSetup)) {
    Status = SioScu ();
    ASSERT_EFI_ERROR (Status);
  }

  //
  // OemService, modify PCD data
  //
  OemSvcDeviceResourceSioDxe (
    mSioResourceFunction->Instance,
    mTablePtr, 
    mSioResourceFunction->SioCfgPort
    );

  // Install each device protocol
  InstallEnabledDeviceProtocol ();

  //
  // Follow PcdSio$(SIO_NAME)Dmi table to program device
  //
  ProgramDmi();
  
  //
  // OemService
  //
  OemSvcAfterInitSioDxe (
    mSioResourceFunction->Instance,
    mTablePtr, 
    mSioResourceFunction->SioCfgPort
    );

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
  To program extend function for Super IO device. 
**/
VOID
ProgramDmi (
  VOID
  ) 
{
  UINT8                  Different;
  UINT8                  *PcdDmiPointer;
  UINTN                  BufferSize;
  EFI_STATUS             Status;
  SIO_CONFIGURATION      *SioConfiguration;
  SIO_DEVICE_LIST_TABLE  *PcdPointer;

  Status           = EFI_SUCCESS;
  PcdPointer       = mTablePtr;
  BufferSize       = sizeof (SIO_CONFIGURATION);
  SioConfiguration = NULL;

  if (FeaturePcdGet (PcdSioWpcd374lSetup)) {
    SioConfiguration = AllocateZeroPool (sizeof (SIO_CONFIGURATION));
    Status = gRT->GetVariable (
                    mSioVariableName,
                    &mSioFormSetGuid,
                    NULL,
                    &BufferSize,
                    SioConfiguration
                    );
    ASSERT_EFI_ERROR (Status);
  }

  while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
    Different = 0;
    PcdDmiPointer = mDmiTablePtr;
    //
    // Check which device enable
    //
    if ((PcdPointer->TypeInstance == mSioResourceFunction->Instance) && (PcdPointer->DeviceEnable == TRUE)) {
      switch(PcdPointer->Device){
			case COM:
        Different = COM;
			  break;
			case LPT:
        Different = LPT;
			  break;
		  case SGPIO:
        Different = SGPIO;
			  break;
		  }
    }

    if (Different != 0) {
      FindExtensiveDevice (
        &PcdDmiPointer,
        PcdPointer->TypeInstance,
        PcdPointer->Device,
        PcdPointer->DeviceInstance
        );

      if (PcdDmiPointer != NULL) {
	    	switch(Different){
			  case COM:
          ProgramCom((SIO_EXTENSIVE_TABLE_MAINTYPE*)PcdDmiPointer, PcdPointer, SioConfiguration);
			    break;
			  case LPT:
          ProgramLpt((SIO_EXTENSIVE_TABLE_MAINTYPE*)PcdDmiPointer, PcdPointer, SioConfiguration);
			    break;
			  case SGPIO:
          ProgramGpio((SIO_EXTENSIVE_TABLE_MAINTYPE*)PcdDmiPointer, PcdPointer, SioConfiguration);
			    break;
		    }
      }
    }
    PcdPointer++;
  }

  if (FeaturePcdGet (PcdSioWpcd374lSetup)) {
    if (mFirstBoot) {
      Status = gRT->SetVariable(
                      mSioVariableName,
                      &mSioFormSetGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      BufferSize,
                      SioConfiguration
                      );
      ASSERT_EFI_ERROR (Status);
    }
    FreePool (SioConfiguration);
  }
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
      SioWrite8 (IndexPort, SIO_CR21, mSioResourceFunction->ScriptFlag);
      if (SioRead8 (DataPort) != DeviceTable->TypeL) {
        IndexPort = 0;
      }
    }
  }  

  //
  // Auto scan config port
  //
  if (IndexPort == 0) {
    
//[-start-140116-IB12970054-modify]//
    for (Index = 0; Index < mSioCfgPortListSize; Index++) {
//[-end-140116-IB12970054-modify]//

      IndexPort = mSioCfgPortList[Index];
      DataPort  = IndexPort + 1;
      mSioResourceFunction->SioCfgPort = IndexPort;
      //
      // Enter Config Mode
      //
      EnterConfigMode (
        );
  
      SioWrite8 (IndexPort, SIO_CHIP_ID1, mSioResourceFunction->ScriptFlag);
      if (SioRead8 (DataPort) == DeviceTable->TypeH) {
        if (DeviceTable->TypeL != NONE_ID) {
          SioWrite8 (IndexPort, SIO_CR21, mSioResourceFunction->ScriptFlag);
          if (SioRead8 (DataPort) == DeviceTable->TypeL) {
            break;
          }
        } else {
          break;
        }
      }
    }

//[-start-140116-IB12970054-modify]//
    if (Index == mSioCfgPortListSize) {  // if true, means not found SIO ID
//[-end-140116-IB12970054-modify]//

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
  Transfer from SIO_DEVICE_LIST_TABLE to EFI_ISA_DEVICE_RESOURCE for using of device protocol

  @param[in]  *DeviceList            SIO_DEVICE_LIST_TABLE structure.
  @param[in]  *DeviceResource        EFI_ISA_DEVICE_RESOURCE structure.

  @retval EFI_SUCCESS                The function completed successfully.
**/
EFI_STATUS
DeviceListToDeviceResource (
  IN SIO_DEVICE_LIST_TABLE        *DeviceList,
  IN EFI_ISA_DEVICE_RESOURCE      *DeviceResource
  )
{
  DeviceResource->IoPort = DeviceList->DeviceBaseAdr;
  DeviceResource->IrqNumber = DeviceList->DeviceIrq;
  DeviceResource->DmaChannel = DeviceList->DeviceDma;

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
                  &gEfiAcpiSupportProtocolGuid, 
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
        if (FeaturePcdGet(PcdSioWpcd374lUpdateAsl)) {
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
    AcpiChecksum ((SioAcpiTable), SioAcpiTable->Length, EFI_FIELD_OFFSET (EFI_ACPI_DESCRIPTION_HEADER, Checksum));
     
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

/**
  Install Device protocol from PCD structure.
**/
VOID
InstallEnabledDeviceProtocol (
  VOID
  )
{
  EFI_STATUS                         Status;
  UINT8                              Index;
  SIO_DEVICE_LIST_TABLE              *PcdPointer;

  Status       = EFI_NOT_FOUND;
  
  Index = 0;
  while (mDeviceFunction[Index].Device != NULL_ID) {
    PcdPointer   = mTablePtr;
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      if ((PcdPointer->TypeInstance == mSioResourceFunction->Instance)  && (PcdPointer->Device == mDeviceFunction[Index].Device) 
        && (PcdPointer->DeviceInstance == mDeviceFunction[Index].Instance)) {  
        PcdPointer->DeviceLdn= mDeviceFunction[Index].Ldn;
        //
        // if the device is enable, then install it
        //
        if ((PcdPointer->DeviceEnable != FALSE) || (PcdPointer->DeviceDma == NULL_ID)) {
          if (mDeviceFunction[Index].InstallDeviceProtocol != NULL) {
            Status = mDeviceFunction[Index].InstallDeviceProtocol (
                       PcdPointer
                       ); 
          }
        }
        if (PcdPointer->DeviceEnable == FALSE) {
          IDW8 (SIO_LDN, mDeviceFunction[Index].Ldn, mSioResourceFunction);
          IDW8 (SIO_DEV_ACTIVE, FALSE, mSioResourceFunction);
        } 
        break;
      } 
      PcdPointer++;
    }
    Index++;
  }  
}

/**
  Find SSDT table from RAW section of DXE driver, then publish it to ACPI table.

  @param[in]  Event           Event whose notification function is being invoked.
  @param[in]  Context         Pointer to the notification function's context.
**/
EFI_STATUS
UpdateSioMbox (
  IN EFI_ACPI_DESCRIPTION_HEADER      *SioAcpiTable
  )
{
  EFI_STATUS                            Status;
  UINT8                                 *SsdtPointer;
  UINT8                                 NumOfEntries;
  UINT8                                 DeviceEntries;
  UINT8                                 DeviceH;
  UINT8                                 DeviceL;  
  UINT8                                 SioInstance;
  EFI_ACPI_DESCRIPTION_HEADER           *Table;
  SIO_DEVICE_LIST_TABLE                 *PcdPointer;
  SIO_DEVICE_LIST_TABLE                 *DevicePointer;  
  SIO_NVS_AREA                          *SioNvsArea;
  EFI_ASL_RESOURCE                      *SioAslResource;
  BOOLEAN                               UpdateConfig;  
  UINT8                                 *PcdDmiPointer;
  UINT8                                 UpdateNum;  
  SIO_EXTENSIVE_TABLE_TYPE3             *DdnFunction;

  DdnFunction = NULL;
  UpdateNum = 0;
  UpdateConfig = FALSE;
  PcdPointer = mTablePtr;
  Status = EFI_SUCCESS;
  Table = SioAcpiTable;
  DevicePointer = NULL;  
  DeviceEntries = 0;
  NumOfEntries = sizeof (SIO_NVS_AREA) / sizeof (EFI_ASL_RESOURCE);
  DeviceH = mSioResourceFunction->TypeIdHigh;
  DeviceL = mSioResourceFunction->TypeIdLow;
  SioInstance = mSioResourceFunction->Instance;

  Status = gBS->AllocatePool(
                  EfiACPIMemoryNVS,
                  sizeof (SIO_NVS_AREA),
                  &SioNvsArea
                  );
                  
  ASSERT_EFI_ERROR (Status);  
  ZeroMem (SioNvsArea, sizeof (SIO_NVS_AREA));
  SioAslResource = (EFI_ASL_RESOURCE*) SioNvsArea;
  
  for (SsdtPointer = ((UINT8 *)((UINTN)Table)); SsdtPointer <= (UINT8 *)((UINTN)Table + (Table->Length)); SsdtPointer++) {
    switch ( *((UINT32*) SsdtPointer)) {

    //
    //  Search LGDN String
    //
    case (EFI_SIGNATURE_32 ('L', 'G', 'D', 'N')):
      //
      // Calculate the number of non-zero entries in the debug code table
      //
      while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
        //
        // Update PCD data to NVS area
        //		
        if (DeviceEntries >= NumOfEntries) {
          break;
        }
		
        if ((PcdPointer->Device != CFG_ID) && (mSioResourceFunction->Instance == PcdPointer->TypeInstance)) {
          SioAslResource->Device = PcdPointer->DeviceEnable;
          SioAslResource->DeviceBaseIo = UpsideDown(PcdPointer->DeviceBaseAdr);
          SioAslResource->DeviceLdn = PcdPointer->DeviceLdn;  
          //
          // Before setting KYBD/MOUSE, we need to check their flag(PcdPointer->Device),
          // because if there is no PS2 device, system will hang up during KbcExistence () and MouseExistence()
          //
          if (PcdPointer->Device == KYBD) {
            if (!KbcExistence ()) {
              SioAslResource->Device = 0;
            }
          }
          if (PcdPointer->Device == MOUSE) {
            if (!MouseExistence()) {
              SioAslResource->Device = 0;
            }
          }
          SioAslResource++;
          DeviceEntries++;
        }
        PcdPointer++;
      }
      
      if (IsAmlOpRegionObject (SsdtPointer)) {
        ASSERT_EFI_ERROR (Status);
        SetOpRegion (SsdtPointer, SioNvsArea, sizeof (SIO_NVS_AREA));   
      }
      break; 

    //
    // Update config port
    //   
    case (EFI_SIGNATURE_32 ('S','I','D','P')):
      if (!UpdateConfig) {      
        *(UINT8*) (SsdtPointer + 5) = (UINT8) mSioResourceFunction->SioCfgPort;
        UpdateConfig = TRUE;        
      }
      break;

    //
    // Update ASL name string to avoid the same device name in multi SIOs
    //
    case (EFI_SIGNATURE_32 ('M', 'o', 'd', 'u')):
      *(UINT16*) (SsdtPointer) = UintToChar (DeviceH);
      *((UINT16*) SsdtPointer + 1) = UintToChar (DeviceL);
      *((UINT16*) SsdtPointer + 2) = UintToChar (SioInstance);
      break;

    case (EFI_SIGNATURE_32 ('S', 'I', 'O', 'X')):
      *(SsdtPointer + 3) = (UINT8) (UintToChar (SioInstance) >> 8);
      break;

    case (EFI_SIGNATURE_32 ('C', 'o', 'm', '1')):
    case (EFI_SIGNATURE_32 ('C', 'o', 'm', '2')):    
    case (EFI_SIGNATURE_32 ('C', 'o', 'm', '3')): 
    case (EFI_SIGNATURE_32 ('C', 'o', 'm', '4')): 
    case (EFI_SIGNATURE_32 ('C', 'o', 'm', '5')):  
    case (EFI_SIGNATURE_32 ('C', 'o', 'm', '6')):    
      PcdDmiPointer = mDmiTablePtr;      
      UpdateNum = AsciiToHexadecimal(*((UINT8*)SsdtPointer + 3));      
      if ((UpdateNum <= 6) && (UpdateNum > 0)){
        FindExtensiveDevice (
          &PcdDmiPointer,
          SioInstance,
          COM,
          (UpdateNum - 1)
          );          
        //
        // Update DDN if Sio$(SIO_MODULE_NAME)Dmi table enable
        //
        if (PcdDmiPointer != NULL) {
          do {
            if (*PcdDmiPointer == SUBTYPE3) {    
              DdnFunction = (SIO_EXTENSIVE_TABLE_TYPE3*)PcdDmiPointer;            
              if (DdnFunction->Enable) {       
                CopyMem (
                  SsdtPointer, 
                  &DdnFunction->Ddn[0] , 
                  sizeof(DdnFunction->Ddn)
                  );    
              }else {
                *((UINT8*)SsdtPointer - 5) = 'X';
              }   
            }       
            PcdDmiPointer += *(UINT8*)(PcdDmiPointer + 1);
          } while ((*PcdDmiPointer != 0) && (*PcdDmiPointer != 0xFF)); 
        }
      }        
      break;

    case (EFI_SIGNATURE_32 ('L', 'p', 't', '1')):
    case (EFI_SIGNATURE_32 ('L', 'p', 't', '2')):    
      PcdDmiPointer = mDmiTablePtr;      
      UpdateNum = AsciiToHexadecimal(*((UINT8*)SsdtPointer + 3));      
      if ((UpdateNum <= 2) && (UpdateNum > 0)){
        FindExtensiveDevice (
          &PcdDmiPointer,
          SioInstance,
          LPT,
          (UpdateNum - 1)
          );        
        //
        // Update DDN if Sio$(SIO_MODULE_NAME)Dmi table enable
        //
        if (PcdDmiPointer != NULL) {
          do {
            if (*PcdDmiPointer == SUBTYPE3) {    
              DdnFunction = (SIO_EXTENSIVE_TABLE_TYPE3*)PcdDmiPointer;            
              if (DdnFunction->Enable) {       
                CopyMem (
                  SsdtPointer, 
                  &DdnFunction->Ddn[0] , 
                  sizeof(DdnFunction->Ddn)
                  );    
              }else {
                *((UINT8*)SsdtPointer - 5) = 'X';
              }   
            }       
            PcdDmiPointer += *(UINT8*)(PcdDmiPointer + 1);
          } while ((*PcdDmiPointer != 0) && (*PcdDmiPointer != 0xFF));
        }
      }        
      break;

    default:
      break;
    }   
  } 
  return EFI_SUCCESS;
}

/**
  Create SCU by SIO and update value to PCD data.

  @retval      EFI_SUCCESS            Function complete successfully. 
**/
EFI_STATUS
SioScu (
  VOID
  )
{
  UINT8                             Index;
  UINT8                             ConfigurationNum;
  EFI_STATUS                        Status;
  UINTN                             BufferSize; 
  SIO_DEVICE_LIST_TABLE             *GetByConfiguration;
  SIO_CONFIGURATION                 *SioConfiguration;
  SIO_DEVICE_LIST_TABLE             *PcdPointer;

  PcdPointer = mTablePtr;
  ConfigurationNum = 0;
  SioConfiguration = NULL;

  //
  // Create variable and SIO page itself
  //
  Status = CreateSetupPage ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  BufferSize = sizeof (SIO_CONFIGURATION);
  SioConfiguration = AllocateZeroPool (sizeof (SIO_CONFIGURATION));
  Status = gRT->GetVariable (
                  mSioVariableName,
                  &mSioFormSetGuid,
                  NULL,
                  &BufferSize,
                  SioConfiguration
                  );

  if (Status == EFI_SUCCESS) {
    ConfigurationNum = SioConfiguration->AutoUpdateNum;
    GetByConfiguration = (SIO_DEVICE_LIST_TABLE*) SioConfiguration;

    for (Index = 0; Index < ConfigurationNum; Index++) {
      //
      // Calculate the number of non-zero entries in the table
      //
      while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
        //
        // Check device and instance, if it's right, update to PCD data 
        //
        if ((PcdPointer->Device == GetByConfiguration->Device) && (PcdPointer->DeviceInstance == GetByConfiguration->DeviceInstance)
          && (GetByConfiguration->DeviceEnable != SELECT_AUTO)) {
          PcdPointer->DeviceEnable = GetByConfiguration->DeviceEnable;
          PcdPointer->DeviceBaseAdr = GetByConfiguration->DeviceBaseAdr;
          PcdPointer->DeviceIrq = GetByConfiguration->DeviceIrq;
          //
          // if user disable device, it still need to install protocol, but doesn't create the table of the IsaAcpi
          //          
          if (GetByConfiguration->DeviceEnable == SELECT_DISABLE) {
            PcdPointer->DeviceDma = NULL_ID;          
          }            
          PcdPointer++;
          break;
        }
     
        PcdPointer++;
      }
      GetByConfiguration++;
      PcdPointer = mTablePtr; 
    }
  }
  FreePool (SioConfiguration);
  return Status;
}
