
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
#ifndef __PLATFORM_CONFIG_DATA_LIB_H__
#define __PLATFORM_CONFIG_DATA_LIB_H__

#include "Uefi.h"

//[-start-160216-IB03090424-add]//
typedef struct {
  VOID   *SystemConfigDataPtr;
  UINT32  SystemConfigDataSize;
} SYSTEM_CONFIG_DATA_PREMEM_ENTRY;
//[-end-160216-IB03090424-add]//

//[-start-160727-IB07250259-modify]//
/*++

Save the memory config data to the system storage.

@param[in]	 MemoryConfigData		   DataBuffer of the Memory Config Data.
@param[in]     MemoryConfigDataSize		   Memory Config Data Size.

@return EFI_SUCCESS 		                        Memory config data save success.

--*/

EFI_STATUS
EFIAPI
SetMemoryConfigData (
  IN     EFI_GUID                  *ConfigDataGuid,
  IN     CONST VOID                *MemoryConfigData,
  IN     UINTN                      MemoryConfigDataSize
  );

/**
  Load the memory config data from the system storage.
  
  @param[out]     MemoryConfigData 		 DataBuffer for the Memory Config Data.
  @param[in,out]  MemoryConfigDataSize    Memory Config Data Size.
  
  @return EFI_SUCCESS					 Memory config data load success.
  
**/
EFI_STATUS
EFIAPI
GetMemoryConfigData (
  IN     EFI_GUID                  *ConfigDataGuid,
	OUT	   VOID	                     **MemoryConfigData,
	IN OUT UINTN					  *MemoryConfigDataSize
  );

/**
  Clear the memory config data from the system storage.
  
  @return EFI_SUCCESS					 Memory config data load success.
  
**/
EFI_STATUS
EFIAPI
ClearMemoryConfigData (
  IN     EFI_GUID                  *ConfigDataGuid
  );
//[-end-160727-IB07250259-modify]//

/*++

Save the system config data to the system storage.

@param[in]	 SystemConfigData		          DataBuffer of the System Config Data.
@param[in]   SystemConfigDataSize		   System Config Data Size.

@return EFI_SUCCESS 		                        System config data save success.

--*/
EFI_STATUS
EFIAPI
SetSystemConfigData (
  IN     CONST VOID                *SystemConfigData,
  IN     UINTN                      SystemConfigDataSize
  );

/**
  Load the memory config data from the system storage.
  
  @param[out]     SystemConfigData 		DataBuffer for the System Config Data.
  @param[in,out]  SystemConfigDataSize    System Config Data Size.
  
  @return EFI_SUCCESS					System config data load success.
  
**/
EFI_STATUS
EFIAPI
GetSystemConfigData (
	OUT	   VOID	                      *SystemConfigData,
	IN OUT UINTN					  *SystemConfigDataSize
  );

//[-start-160216-IB03090424-add]//
EFI_STATUS
EFIAPI
GetSystemConfigDataPreMem (
	OUT	   VOID	                      *SystemConfigData,
	IN OUT UINTN					  *SystemConfigDataSize
  );
//[-end-160216-IB03090424-add]//

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
	IN	   CONST VOID				 *DefaultSystemConfigData,
	IN	   UINTN					  DefaultSystemConfigDataSize,
	OUT	   VOID	                     *SystemConfigData,
	IN OUT UINTN					 *SystemConfigDataSize
  );

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
);

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
);

/**
  Clear all the platform config data from the system storage.
  
  @return EFI_SUCCESS                   Memory config data load success.
  
**/
EFI_STATUS
EFIAPI
ClearPlatformConfigData (
  );

#endif
