/** @file
  Platform Config Data library instance

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include "PlatformConfigData.h"
#include <Guid/PlatformInfo.h>
//[-start-160104-IB03090424-add]//
#include "MmrcData.h"
//[-end-160104-IB03090424-add]//

//[-start-161002-IB07400791-modify]//
//[-start-160608-IB05400680-add]//
VOID  *mPlatformConfigDataLibHeci2Protocol = NULL;
//[-end-160608-IB05400680-add]//
//[-end-161002-IB07400791-modify]//

//[-start-160104-IB03090424-modify]//
//[-start-160727-IB07250259-modify]//
PLATFORM_CONFIG_DATA_MAPPING_TABLE mMappingTable[] = {
  {
    PLATFORM_MEMORY_CONFIG_DATA_GUID,
	  0
  },
  {
  	BOOT_VARIABLE_NV_DATA_GUID,
	  PLATFORM_CONFIG_DATA_TABLE_SIZE(sizeof(MRC_PARAMS_SAVE_RESTORE))
  },
  {
  	PLATFORM_SYSTEM_CONFIG_DATA_GUID,
	  PLATFORM_CONFIG_DATA_TABLE_SIZE(sizeof(BOOT_VARIABLE_NV_DATA)) + PLATFORM_CONFIG_DATA_TABLE_SIZE(sizeof(MRC_PARAMS_SAVE_RESTORE))
  }
};
//[-end-160727-IB07250259-modify]//
//[-end-160104-IB03090424-modify]//

//[-start-160530-IB07220086-modify]//
EFI_STATUS STATIC
//[-end-160530-IB07220086-modify]//
GetConfigData (
  EFI_GUID            *ConfigDataGuid,
  VOID               **ConfigData,
  UINTN               *ConfigDataSize
  )
{
  EFI_PEI_HOB_POINTERS *DataHob;
  DataHob = GetFirstGuidHob(ConfigDataGuid);
  if (DataHob == NULL) {
	return EFI_NOT_FOUND;
  }
  *ConfigData = GET_GUID_HOB_DATA(DataHob);
  *ConfigDataSize = GET_GUID_HOB_DATA_SIZE(DataHob);
  return EFI_SUCCESS;
}

EFI_STATUS
GetSMIPConfigData (
  VOID                 **SMIPConfigData,
  UINTN                 *SMIPConfigDataSize
)
{
  return GetConfigData(&gPlatformSmipConfigDataGuid, SMIPConfigData, SMIPConfigDataSize);
}

EFI_STATUS
GetPlatformConfigData (
  UINT8             **PlatformConfigData,
  UINTN              *PlatformConfigDataSize
)
{
  return GetConfigData(&gPlatformConfigDataGuid, PlatformConfigData, PlatformConfigDataSize);
}

//[-start-160216-IB03090424-add]//
EFI_STATUS
GetPlatformConfigDataPreMem (
  UINT8             **PlatformConfigDataPreMem,
  UINTN              *PlatformConfigDataPreMemSize
)
{
	EFI_PEI_HOB_POINTERS *DataHob;
	SYSTEM_CONFIG_DATA_PREMEM_ENTRY *SystemConfigDataPreMemEntry;
	
	DEBUG((EFI_D_INFO, "GetPlatformConfigDataPreMem\n"));
  DataHob = GetFirstGuidHob(&gPlatformSystemConfigDataPreMemGuid);
  DEBUG((EFI_D_INFO, "DataHob = %x\n", DataHob));
  if (DataHob == NULL) {
	  return EFI_NOT_FOUND;
  }
  SystemConfigDataPreMemEntry   = (SYSTEM_CONFIG_DATA_PREMEM_ENTRY *)GET_GUID_HOB_DATA(DataHob);
  DEBUG((EFI_D_INFO, "SystemConfigDataPreMemEntry = %x\n", SystemConfigDataPreMemEntry));
  *PlatformConfigDataPreMem     = (UINT8 *)SystemConfigDataPreMemEntry->SystemConfigDataPtr;
  *PlatformConfigDataPreMemSize = SystemConfigDataPreMemEntry->SystemConfigDataSize; 
  
  DEBUG((EFI_D_INFO, "PlatformConfigDataPreMem = %x, PlatformConfigDataPreMemSize = %x\n", *PlatformConfigDataPreMem, *PlatformConfigDataPreMemSize));
  
  return EFI_SUCCESS;	
}
//[-end-160216-IB03090424-add]//

PLATFORM_CONFIG_DATA_MAPPING_TABLE *
GetMappingTable (
  IN EFI_GUID                          *ConfigDataGuid
  )
{
  UINTN Index;
  for(Index = 0; Index < sizeof(mMappingTable) / sizeof(mMappingTable[0]); Index ++) {
    if (CompareGuid(&mMappingTable[Index].ConfigDataGuid, ConfigDataGuid)) {
		return &mMappingTable[Index];
    }
  }
  return NULL;
}

PLATFORM_CONFIG_DATA_TABLE *
FindConfigData (
//[-start-160216-IB03090424-modify]//
  IN EFI_GUID                          *ConfigDataGuid,
  IN BOOLEAN                            IsPreMem
//[-end-160216-IB03090424-modify]//
  )
{
  PLATFORM_CONFIG_DATA_MAPPING_TABLE *TableIndex;
  PLATFORM_CONFIG_DATA_TABLE         *ConfigData; 
  UINT8                              *Data;
  UINTN                              DataSize;
  EFI_STATUS                         Status;
  TableIndex = GetMappingTable(ConfigDataGuid);
  if (TableIndex == NULL) {
    DEBUG((EFI_D_ERROR, "TableIndex is NULL. GUID not found in GetMappingTable\n"));
    return NULL;
  }
//[-start-160216-IB03090424-modify]//
  if (!IsPreMem)
    Status = GetPlatformConfigData(&Data, &DataSize);
  else
  	Status = GetPlatformConfigDataPreMem(&Data, &DataSize);
//[-end-160216-IB03090424-modify]//
	
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "GetPlatformConfigData failed\n"));
  	return NULL;
  }
  if (TableIndex->Offset > DataSize) {
    DEBUG((EFI_D_ERROR, "TableIndex->Offset > DataSize\n"));
    return NULL;
  }
  ConfigData = (PLATFORM_CONFIG_DATA_TABLE*)(Data + TableIndex->Offset);
  if (!CompareGuid(ConfigDataGuid, &ConfigData->ConfigDataGuid)) {
//[-start-151228-IB03090424-modify]//
    DEBUG((EFI_D_ERROR, "Guid Mismatch\n"));
//[-end-151228-IB03090424-modify]//
    return NULL;
  }
  if (CalculateCheckSum32((UINT32*)ConfigData, PLATFORM_CONFIG_DATA_TABLE_SIZE(ConfigData->ConfigDataSize)) != 0) {
//[-start-151228-IB03090424-modify]//
    DEBUG((EFI_D_ERROR, "CheckSum Mismatch\n"));
//[-end-151228-IB03090424-modify]//
    return NULL;
  }
  
  return ConfigData;
}

EFI_STATUS
SaveConfigData (
  IN EFI_GUID                       *ConfigDataGuid,
  IN CONST VOID                     *ConfigData,
  IN UINTN                           ConfigDataSize,
  IN PLATFORM_CONFIG_DATA_TABLE     *StorageData
  )
{
  PLATFORM_CONFIG_DATA_TABLE         *ConfigTable;
  PLATFORM_CONFIG_DATA_MAPPING_TABLE *TableIndex;
  UINTN						          TableSize;
//[-start-160317-IB03090425-add]//
  EFI_STATUS                          Status;
//[-end-160317-IB03090425-add]//
  
//[-start-160317-IB03090425-add]//
  Status = EFI_SUCCESS;
//[-end-160317-IB03090425-add]//
  TableSize = PLATFORM_CONFIG_DATA_TABLE_SIZE(ConfigDataSize);
  if (StorageData == NULL) {
    ConfigTable = AllocatePool(TableSize);
	if (ConfigTable == NULL) {
	  return EFI_OUT_OF_RESOURCES;
	}
  } else {
    ConfigTable = StorageData;
  }
  ZeroMem(ConfigTable, TableSize);
  CopyMem(&ConfigTable->ConfigDataGuid, ConfigDataGuid, sizeof(EFI_GUID));
  ConfigTable->ConfigDataSize = (UINT32)ConfigDataSize;
  CopyMem(ConfigTable->Data, ConfigData, ConfigDataSize);
  ConfigTable->CheckSum = CalculateCheckSum32((UINT32*)ConfigTable, TableSize);
  TableIndex = GetMappingTable(ConfigDataGuid);
//[-start-160729-IB07220120-modify]//
//[-start-160807-IB07400769-modify]//
  if (TableIndex != NULL && 
      (GetBootModeHob () != BOOT_IN_RECOVERY_MODE) &&
      IsTxeDataRegionAvailable()) {
//[-end-160807-IB07400769-modify]//
//[-end-160729-IB07220120-modify]//
//[-start-161002-IB07400791-modify]//
//[-start-160608-IB05400680-modify]//
    if (mPlatformConfigDataLibHeci2Protocol != NULL) {
	    Status = Heci2WriteNVMFile(PLATFORM_CONFIG_STORE_FILE_NAME, (UINT32)TableIndex->Offset, (UINT8*)ConfigTable, (UINT32)TableSize, FALSE);
    } else {
//[-start-160317-IB03090425-modify]//
//[-start-160808-IB07220123-modify]//
      Status = HeciWriteNVMFile(PLATFORM_CONFIG_STORE_FILE_NAME, (UINT32)TableIndex->Offset, (UINT8*)ConfigTable,(UINT32)TableSize, FALSE);
//[-end-160808-IB07220123-modify]//
//[-end-160317-IB03090425-modify]//
    }
//[-end-160608-IB05400680-modify]//
//[-end-161002-IB07400791-modify]//
  }
  if (StorageData == NULL) {
    FreePool(ConfigTable);
  }
//[-start-160317-IB03090425-modify]//
  return Status;
//[-end-160317-IB03090425-modify]//
}

/*++

Save the memory config data to the system storage.

@param[in]	 MemoryConfigData		   DataBuffer of the Memory Config Data.
@param[in]     MemoryConfigDataSize		   Memory Config Data Size.

@return EFI_SUCCESS 		                        Memory config data save success.

--*/
//[-start-160727-IB07250259-modify]//
EFI_STATUS
EFIAPI
SetMemoryConfigData (
  IN     EFI_GUID                  *ConfigDataGuid,
  IN     CONST VOID                *MemoryConfigData,
  IN     UINTN                      MemoryConfigDataSize
  )
//[-end-160727-IB07250259-modify]//
{
  PLATFORM_CONFIG_DATA_TABLE *MemoryConfigTable;
//[-start-160216-IB03090424-modify]//
//[-start-160727-IB07250259-modify]//
  MemoryConfigTable = FindConfigData(ConfigDataGuid, FALSE);
//[-end-160727-IB07250259-modify]//
//[-end-160216-IB03090424-modify]//
  if (MemoryConfigTable != NULL) {
    ASSERT (MemoryConfigTable->ConfigDataSize == MemoryConfigDataSize);
	if (CompareMem(MemoryConfigTable, MemoryConfigTable->Data, MemoryConfigDataSize) == 0) {
	  return EFI_SUCCESS;
	}
  }
//[-start-160317-IB03090425-modify]//
//[-start-160727-IB07250259-modify]//
  return SaveConfigData(ConfigDataGuid, MemoryConfigData, MemoryConfigDataSize, MemoryConfigTable);
//[-end-160727-IB07250259-modify]//
//[-end-160317-IB03090425-modify]//
}  

/**
  Load the memory config data from the system storage.
  
  @param[out]     MemoryConfigData 		 DataBuffer for the Memory Config Data.
  @param[out]  MemoryConfigDataSize    Memory Config Data Size.
  
  @return EFI_SUCCESS					 Memory config data load success.
  
**/
//[-start-160727-IB07250259-modify]//
EFI_STATUS
EFIAPI
GetMemoryConfigData (
  IN     EFI_GUID                  *ConfigDataGuid,
	OUT	   VOID	                     **MemoryConfigData,
	IN OUT UINTN					  *MemoryConfigDataSize
  )
//[-end-160727-IB07250259-modify]//
{
  PLATFORM_CONFIG_DATA_TABLE *MemoryConfigTable;
  if (MemoryConfigData == NULL) {
	DEBUG((EFI_D_INFO, "GetMemoryConfigData EFI_INVALID_PARAMETER\n"));
    return EFI_INVALID_PARAMETER;
  }
//[-start-160216-IB03090424-modify]//
//[-start-160727-IB07250259-modify]//
  MemoryConfigTable = FindConfigData(ConfigDataGuid, FALSE);
//[-end-160727-IB07250259-modify]//
//[-end-160216-IB03090424-modify]//
  if (MemoryConfigTable == NULL) {
	DEBUG((EFI_D_INFO, "GetMemoryConfigData EFI_NOT_FOUND\n"));
    return EFI_NOT_FOUND;
  }
  DEBUG((EFI_D_INFO, "GetMemoryConfigData Find MemoryConfig %x %x\n", MemoryConfigTable->Data, MemoryConfigTable->ConfigDataSize));

  if (*MemoryConfigData == NULL) {
  	*MemoryConfigData = MemoryConfigTable->Data;
	*MemoryConfigDataSize = MemoryConfigTable->ConfigDataSize;
	return EFI_SUCCESS;
  }
  if (*MemoryConfigDataSize < MemoryConfigTable->ConfigDataSize) {
  	*MemoryConfigDataSize = MemoryConfigTable->ConfigDataSize;
	DEBUG((EFI_D_INFO, "GetMemoryConfigData EFI_BUFFER_TOO_SMALL\n"));
	return EFI_BUFFER_TOO_SMALL;
  }
  *MemoryConfigDataSize = MemoryConfigTable->ConfigDataSize;
  CopyMem(*MemoryConfigData, MemoryConfigTable->Data, *MemoryConfigDataSize);
  return EFI_SUCCESS;
}

/**
  Clear the memory config data from the system storage.
  
  @return EFI_SUCCESS					 Memory config data load success.
  
**/
//[-start-160727-IB07250259-modify]//
EFI_STATUS
EFIAPI
ClearMemoryConfigData (
  IN     EFI_GUID                  *ConfigDataGuid
  )
//[-end-160727-IB07250259-modify]//
{
  PLATFORM_CONFIG_DATA_TABLE         *MemoryConfigTable;
  PLATFORM_CONFIG_DATA_MAPPING_TABLE *TableIndex;
//[-start-160216-IB03090424-modify]//
//[-start-160727-IB07250259-modify]//
  MemoryConfigTable = FindConfigData(ConfigDataGuid, FALSE);
//[-end-160727-IB07250259-modify]//
//[-end-160216-IB03090424-modify]//
  if (MemoryConfigTable == NULL) {
    return EFI_SUCCESS;
  }
//[-start-160728-IB07250259-modify]//
  TableIndex = GetMappingTable(ConfigDataGuid);
//[-end-160728-IB07250259-modify]//
  ZeroMem(&MemoryConfigTable->ConfigDataGuid, sizeof(EFI_GUID));
//[-start-160317-IB03090425-modify]//
//[-start-160807-IB07400769-modify]//
  if (IsTxeDataRegionAvailable()) {
//[-start-161002-IB07400791-modify]//
//[-start-160808-IB07220123-modify]//
    if (mPlatformConfigDataLibHeci2Protocol != NULL) {
	    return Heci2WriteNVMFile(PLATFORM_CONFIG_STORE_FILE_NAME, (UINT32)TableIndex->Offset, (UINT8*)&MemoryConfigTable->ConfigDataGuid, sizeof(EFI_GUID), FALSE);
    } else {
      return HeciWriteNVMFile(PLATFORM_CONFIG_STORE_FILE_NAME, (UINT32)TableIndex->Offset, (UINT8*)&MemoryConfigTable->ConfigDataGuid, sizeof(EFI_GUID), FALSE);
    }
//[-end-160808-IB07220123-modify]//
//[-end-161002-IB07400791-modify]//
  } 

  return EFI_SUCCESS;
//[-end-160807-IB07400769-modify]//
//[-end-160317-IB03090425-modify]//
}

/*++

Save the system config data to the system storage.

@param[in]	 SystemConfigData		          DataBuffer of the System Config Data.
@param[in]     SystemConfigDataSize		   System Config Data Size.

@return EFI_SUCCESS 		                        System config data save success.

--*/

EFI_STATUS
EFIAPI
SetSystemConfigData (
  IN     CONST VOID                *SystemConfigData,
  IN     UINTN                      SystemConfigDataSize
  )
{
  PLATFORM_CONFIG_DATA_TABLE *SystemConfigTable;
  UINTN                       TableSize;
  EFI_STATUS                  Status;

//[-start-160216-IB03090424-modify]//
  SystemConfigTable = FindConfigData(&gPlatformSystemConfigDataGuid, FALSE);
//[-end-160216-IB03090424-modify]//
  if (SystemConfigTable == NULL) {
  	Status = GetConfigData(&gPlatformSystemConfigDataGuid, &SystemConfigTable, &TableSize);
	if (EFI_ERROR(Status)) {
      SystemConfigTable = BuildGuidHob(&gPlatformSystemConfigDataGuid, PLATFORM_CONFIG_DATA_TABLE_SIZE(SystemConfigDataSize));
      CopyMem(&SystemConfigTable->ConfigDataGuid, &gPlatformSystemConfigDataGuid, sizeof(EFI_GUID));
      SystemConfigTable->ConfigDataSize = (UINT32)SystemConfigDataSize;
	}
  }
  if (SystemConfigTable != NULL) {
    ASSERT (SystemConfigTable->ConfigDataSize == SystemConfigDataSize);
	if (CompareMem(SystemConfigData, SystemConfigTable->Data, SystemConfigDataSize) == 0) {
	  return EFI_SUCCESS;
	}
  }
//[-start-160317-IB03090425-modify]//
  return SaveConfigData(&gPlatformSystemConfigDataGuid, SystemConfigData, SystemConfigDataSize, SystemConfigTable);
//[-end-160317-IB03090425-modify]//
}

/**
  Load the memory config data from the system storage.
  
  @param[out]     SystemConfigData 		DataBuffer for the System Config Data.
  @param[in,out]  SystemConfigDataSize    System Config Data Size.
  
  @return EFI_SUCCESS					System config data load success.
  
**/
EFI_STATUS
EFIAPI
GetSystemConfigData (
	OUT	   VOID	            *SystemConfigData,
	IN OUT UINTN					  *SystemConfigDataSize
  )
{
  PLATFORM_CONFIG_DATA_TABLE *SystemConfigTable;
  UINTN                       TableSize;
  EFI_STATUS                  Status;

//[-start-160216-IB03090424-modify]//
  DEBUG((EFI_D_INFO, "GetSystemConfigData\n"));  
  SystemConfigTable = FindConfigData(&gPlatformSystemConfigDataGuid, FALSE);
//[-end-160216-IB03090424-modify]//
  if (SystemConfigTable == NULL) {
    DEBUG((EFI_D_INFO, "SystemConfigTable is NULL Going to GetConfigData\n"));
  	Status = GetConfigData(&gPlatformSystemConfigDataGuid, &SystemConfigTable, &TableSize);
	if (EFI_ERROR(Status)) {
	  DEBUG((EFI_D_INFO, "GetSystemConfigData EFI_NOT_FOUND\n"));
      return EFI_NOT_FOUND;
	}
  }
  
  if (*SystemConfigDataSize < SystemConfigTable->ConfigDataSize) {
  	*SystemConfigDataSize = SystemConfigTable->ConfigDataSize;
	DEBUG((EFI_D_INFO, "GetSystemConfigData EFI_BUFFER_TOO_SMALL\n"));
	return EFI_BUFFER_TOO_SMALL;
  }
  
  *SystemConfigDataSize = SystemConfigTable->ConfigDataSize;
  CopyMem(SystemConfigData, SystemConfigTable->Data, *SystemConfigDataSize);
  return EFI_SUCCESS;
}

//[-start-160216-IB03090424-add]//
EFI_STATUS
EFIAPI
GetSystemConfigDataPreMem (
	OUT	   VOID	            *SystemConfigData,
	IN OUT UINTN					  *SystemConfigDataSize
  )
{
  PLATFORM_CONFIG_DATA_TABLE *SystemConfigTable;
  UINTN                       TableSize;
  EFI_STATUS                  Status;
  
  DEBUG((EFI_D_INFO, "GetSystemConfigDataPreMem\n"));
  SystemConfigTable = FindConfigData(&gPlatformSystemConfigDataGuid, TRUE);
  if (SystemConfigTable == NULL) {
    DEBUG((EFI_D_INFO, "SystemConfigTable is NULL Going to GetConfigData\n"));
  	Status = GetConfigData(&gPlatformSystemConfigDataGuid, &SystemConfigTable, &TableSize);
	if (EFI_ERROR(Status)) {
	  DEBUG((EFI_D_INFO, "GetSystemConfigData EFI_NOT_FOUND\n"));
      return EFI_NOT_FOUND;
	}
  }
  if (*SystemConfigDataSize < SystemConfigTable->ConfigDataSize) {
  	*SystemConfigDataSize = SystemConfigTable->ConfigDataSize;
	DEBUG((EFI_D_INFO, "GetSystemConfigData EFI_BUFFER_TOO_SMALL\n"));
	return EFI_BUFFER_TOO_SMALL;
  }
  *SystemConfigDataSize = SystemConfigTable->ConfigDataSize;
  CopyMem(SystemConfigData, SystemConfigTable->Data, *SystemConfigDataSize);
  return EFI_SUCCESS;
}
//[-end-160216-IB03090424-add]//


/**
  Add the SMIP config data into the system.
  
  @param[in]     SmipConfigData 		       DataBuffer for the SMIP Config Data.
  @param[in]     SmipConfigDataSize          SMIP Config Data Size.
  
  @return EFI_SUCCESS					System config data load success.
  
**/
EFI_STATUS
EFIAPI
CreateSMIPConfigData (
	IN	   CONST VOID               *SMIPConfigData,
	IN	   UINTN                     SMIPConfigDataSize
)
{
  VOID   *DataHob;
//[-start-160728-IB07250259-modify]//
  DataHob = BuildGuidDataHob(&gPlatformSmipConfigDataGuid,(VOID*)SMIPConfigData,SMIPConfigDataSize);
//[-end-160728-IB07250259-modify]//
  if (DataHob == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  return EFI_SUCCESS;
}

/**
  Add the platform config data into the system.
  
  @param[in]     SystemConfigData 		DataBuffer for the System Config Data.
  @param[in]  SystemConfigDataSize          System Config Data Size.
  
  @return EFI_SUCCESS					System config data load success.
  
**/
EFI_STATUS
EFIAPI
CreatePlatformConfigData (
  VOID                 *PlatformConfigData,
  UINTN                 PlatformConfigDataSize
)
{
  VOID   *DataHob;
  DEBUG((EFI_D_INFO, "CreatePlatformConfigData %x %x\n", PlatformConfigData, PlatformConfigDataSize));
  DataHob = BuildGuidDataHob(&gPlatformConfigDataGuid,PlatformConfigData,PlatformConfigDataSize);
  if (DataHob == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  DEBUG((EFI_D_INFO, "Hob Data at %x\n", DataHob));
  return EFI_SUCCESS;
}

/**
 Override the Multi-BOM system setup config.
  
  @param[in]     SystemConfigData 		DataBuffer for the System Config Data.
  @param[in]  SystemConfigDataSize          System Config Data Size.
  
  @return EFI_SUCCESS					System config data load success.
  
**/
EFI_STATUS
EFIAPI
OverrideMultiBOMCongfig (
  IN       VOID                       *DefaultSystemConfigData,
  IN       EFI_PLATFORM_INFO_HOB      *PlatformInfoHob
)
{
  CHIPSET_CONFIGURATION *SystemConfiguration;
  SystemConfiguration = (CHIPSET_CONFIGURATION*) DefaultSystemConfigData;

  return EFI_SUCCESS;
}


/**
  According platform info to generate system config data.
  
  @param[in]     DefaultSystemConfigData 		DataBuffer of the Default System Config Data.
  @param[in]     DefaultSystemConfigDataSize   Default System Config Data Size.
  @param[out]     SystemConfigData 		       DataBuffer for the System Config Data.
  @param[in,out]  SystemConfigDataSize           System Config Data Size.
  
  @return EFI_SUCCESS					System config data load success.
  
**/
EFI_STATUS
EFIAPI
GenerateSystemConfigData (
  IN          VOID                      *DefaultSystemConfigData,
  IN          UINTN                      DefaultSystemConfigDataSize,
  OUT         VOID                       *SystemConfigData,
  IN OUT      UINTN                      *SystemConfigDataSize
  )
{
  EFI_PLATFORM_INFO_HOB         *PlatformInfoHob;          
  EFI_PEI_HOB_POINTERS          Hob;

  if (SystemConfigData == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (*SystemConfigDataSize < DefaultSystemConfigDataSize) {
    *SystemConfigDataSize = DefaultSystemConfigDataSize;
    return EFI_BUFFER_TOO_SMALL;
  }
  
  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfoHob = GET_GUID_HOB_DATA(Hob.Raw);

  DEBUG((EFI_D_INFO, "GenerateSystemConfigData OverrideMultiBOMCongfig\n"));

  //
  //Override the DefaultSystemConfigData
  //
  OverrideMultiBOMCongfig(DefaultSystemConfigData, PlatformInfoHob);

  //
  //Copy to system config data
  //
  CopyMem(SystemConfigData, DefaultSystemConfigData, DefaultSystemConfigDataSize);

  *SystemConfigDataSize = DefaultSystemConfigDataSize;
  return EFI_SUCCESS;
}

/**
  Clear all the platform config data from the system storage.
  
  @return EFI_SUCCESS					 Memory config data load success.
  
**/
EFI_STATUS
EFIAPI
ClearPlatformConfigData (
  ){
  UINTN                              FileSize;
  EFI_STATUS                         Status;
  UINT8                              *Data;

  FileSize = 0;
//[-start-161002-IB07400791-modify]//
  if (mPlatformConfigDataLibHeci2Protocol != NULL) {
    Status = Heci2GetNVMFileSize(PLATFORM_CONFIG_STORE_FILE_NAME, &FileSize, (EFI_HECI_PROTOCOL*)mPlatformConfigDataLibHeci2Protocol);
  } else {
    Status = HeciGetNVMFileSize(PLATFORM_CONFIG_STORE_FILE_NAME, &FileSize);
  }
//[-end-161002-IB07400791-modify]//
  if ((!EFI_ERROR(Status)) && (FileSize > 0)) {
    Data = AllocatePool(FileSize);
    SetMem(Data, FileSize, 0xFF);
//[-start-160317-IB03090425-modify]//
//[-start-160808-IB07220123-modify]//
//[-start-160807-IB07400769-modify]//
//[-start-161002-IB07400791-modify]//
    if (IsTxeDataRegionAvailable()) {
      if (mPlatformConfigDataLibHeci2Protocol != NULL) {
  	    Status = Heci2WriteNVMFile(PLATFORM_CONFIG_STORE_FILE_NAME, 0, Data, FileSize, FALSE);
      } else {
        Status = HeciWriteNVMFile(PLATFORM_CONFIG_STORE_FILE_NAME, 0, Data, FileSize, FALSE);
      }  
    }
//[-end-161002-IB07400791-modify]//
//[-end-160807-IB07400769-modify]//
//[-end-160808-IB07220123-modify]//
//[-end-160317-IB03090425-modify]//
//[-start-160803-IB07400768-add]//
    FreePool (Data);
//[-end-160803-IB07400768-add]//
  }
//[-start-160317-IB03090425-modify]//
  return Status;
//[-end-160317-IB03090425-modify]//
}
