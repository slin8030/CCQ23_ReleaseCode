/** @file

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/** @file
 Chipset2 Platform Initialization Driver.

@copyright
  Copyright (c) 1999 - 2016 Intel Corporation. All rights reserved.
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor. This file may be modified by the user, subject to
  the additional terms of the license agreement.

@par Specification Reference:
**/
#include "PlatformDxe.h"
//[-start-170329-IB07400852-modify]//
#include <Library/CmosLib.h>
#include <Protocol/SetupUtility.h>
#include <ChipsetCmos.h>
#include <ChipsetPostCode.h>

extern UINT32   mSetupVariableAttributes;
extern EFI_GUID gChipsetPkgTokenSpaceGuid;
//[-end-170329-IB07400852-modify]//

//[-start-160923-IB07400788-add]//
//[-start-160923-IB07400789-modify]//
VOID
EFIAPI
Chipset2ReadyToBootNotifyCallback (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  UINT8           CmosData;
  
  if (mSystemConfiguration.BootNormalPriority == 2) {
    //
    // Smart Mode & UEFI boot, Set EFI Device First
    //
    CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
    CmosData &= (~B_CMOS_EFI_LEGACY_BOOT_ORDER);
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
  }
  
  gBS->CloseEvent (Event);
}

VOID
EFIAPI
Chipset2ExitBootServicesCallback (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  UINT8           CmosData;
  
  if (mSystemConfiguration.BootNormalPriority == 2) {
    //
    // Smart Mode & UEFI boot, Set EFI Device First
    //
    CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
    CmosData &= (~B_CMOS_EFI_LEGACY_BOOT_ORDER);
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
  }
  
  gBS->CloseEvent (Event);
}

VOID
EFIAPI
Chipset2LegacyBootNotifyCallback (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  UINT8   CmosData;
  
  if (mSystemConfiguration.BootNormalPriority == 2) {
    //
    // Smart Mode & Legacy boot, Set Legacy Device First
    //
    CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature);
    CmosData |= (B_CMOS_EFI_LEGACY_BOOT_ORDER);
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData);
  }
  
  gBS->CloseEvent (Event);
}

VOID
EFIAPI
Chipset2RegisterBootEvent (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_EVENT     ReadyToBootEvent;
  EFI_EVENT     EfiExitBootServicesEvent;
  EFI_EVENT     LegacyBootEvent;
  
  Status = EfiCreateEventReadyToBootEx (
    TPL_CALLBACK,
    Chipset2ReadyToBootNotifyCallback,
    NULL,
    &ReadyToBootEvent
    );
  ASSERT_EFI_ERROR (Status);
  
  Status = gBS->CreateEventEx (
    EVT_NOTIFY_SIGNAL,
    TPL_NOTIFY,
    Chipset2ExitBootServicesCallback,
    NULL,
    &gEfiEventExitBootServicesGuid,
    &EfiExitBootServicesEvent
    );
  ASSERT_EFI_ERROR (Status);
  
  Status = gBS->CreateEventEx (
    EVT_NOTIFY_SIGNAL,
    TPL_CALLBACK - 1,
    Chipset2LegacyBootNotifyCallback,
    NULL,
    &gEfiEventLegacyBootGuid,
    &LegacyBootEvent
    );
  ASSERT_EFI_ERROR (Status);
}
//[-end-160923-IB07400789-modify]//

//[-start-160923-IB07400789-add]//
EFI_STATUS
EFIAPI
Chipset2UpdateSetupBuffer (
  IN  CHIPSET_CONFIGURATION  *SetupBuffer
  )
{
  UINT8     CmosData;

  if (SetupBuffer->BootType == EFI_BOOT_TYPE) { 
    //
    // Win7 Virtual KBC only support when boot type is dual or Legacy mode.
    // Disable this feature when boot type is UEFI mode.
    //
    CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature); 
    CmosData &= (~B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT);
    WriteExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature, CmosData); 
  }

  CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature); 
  if (CmosData & B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT) { // Win7 Virtual KBC support
    SetupBuffer->Win7KbMsSupport = 1;
  } else {
    SetupBuffer->Win7KbMsSupport = 0;
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Chipset2DynamicUpdateSetupVariable (
  VOID
  )
{
  EFI_STATUS                          Status;
  UINTN                               VarSize;
  EFI_SETUP_UTILITY_PROTOCOL          *SetupUtility;  
  
  //
  // Update Setup Variable
  //
  VarSize = PcdGet32 (PcdSetupConfigSize);
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
  Status = gRT->GetVariable (
    SETUP_VARIABLE_NAME,
    &gSystemConfigurationGuid,
    &mSetupVariableAttributes,
    &VarSize,
    &mSystemConfiguration
    );  
  if (!EFI_ERROR (Status)) {
    Chipset2UpdateSetupBuffer(&mSystemConfiguration);
    Status = gRT->SetVariable (
      SETUP_VARIABLE_NAME,
      &gSystemConfigurationGuid,
      mSetupVariableAttributes,
      sizeof(CHIPSET_CONFIGURATION),
      &mSystemConfiguration
      );
  }
  Status = gBS->LocateProtocol (
    &gEfiSetupUtilityProtocolGuid,
    NULL,
    &SetupUtility
    );
  if (!EFI_ERROR (Status)) {
    Chipset2UpdateSetupBuffer((CHIPSET_CONFIGURATION *)SetupUtility->SetupNvData);
  }
  return Status;
}
//[-start-161208-IB07400824-add]//
#ifdef PEI_MEMORY_CONSISTENCY_CHECK

#define MAX_PEI_RUNTIME_MEMORY_TABLE  32
//[-start-170109-IB07400830-add]//
#include <Guid/MemoryTypeInformation.h>
//[-end-170109-IB07400830-add]//

BOOLEAN
IsRtMemType (
  IN EFI_MEMORY_TYPE   Type
  )
{
  if (Type == EfiReservedMemoryType ||
      Type == EfiRuntimeServicesCode ||
      Type == EfiRuntimeServicesData ||
      Type == EfiACPIReclaimMemory ||
      Type == EfiACPIMemoryNVS
      ) {
    return TRUE;
  }

  return FALSE;
}

#pragma pack(1)

typedef struct {
  UINT8                 MemIndex;
  EFI_MEMORY_TYPE       MemType;
  UINT64                MemLength;
  EFI_PHYSICAL_ADDRESS  MemAddress;
} PEI_RUNTIME_MEMORY_TABLE;

//[-start-170109-IB07400830-add]//
typedef struct {
  EFI_MEMORY_TYPE       Type;
  EFI_PHYSICAL_ADDRESS  PhyStart;
  EFI_PHYSICAL_ADDRESS  PhyEnd;
} PRESERVED_MEMORY_ADDRESS_TABLE;
//[-end-170109-IB07400830-add]//

#pragma pack()

//[-start-170109-IB07400830-add]//
PRESERVED_MEMORY_ADDRESS_TABLE mPreservedMemoryAddrTable[] = {
  {EfiReservedMemoryType,   0x0, 0x0},
  {EfiRuntimeServicesCode,  0x0, 0x0},
  {EfiRuntimeServicesData,  0x0, 0x0},
  {EfiACPIReclaimMemory,    0x0, 0x0},
  {EfiACPIMemoryNVS,        0x0, 0x0}
};

UINTN mPreservedMemoryAddrTableSize = sizeof (mPreservedMemoryAddrTable) / sizeof (PRESERVED_MEMORY_ADDRESS_TABLE);

BOOLEAN
IsRtPreservedMemory (
  IN EFI_MEMORY_TYPE   Type,
  IN UINT64            NumberOfPages
  )
{
  UINTN                         Index;
  UINTN                         PreservedMemorySize;
  EFI_MEMORY_TYPE_INFORMATION   *PreservedMemoryTable;

  PreservedMemoryTable = (EFI_MEMORY_TYPE_INFORMATION *)PcdGetPtr (PcdPreserveMemoryTable);
  PreservedMemorySize =  LibPcdGetSize (PcdToken (PcdPreserveMemoryTable));
  
  if (Type == EfiReservedMemoryType ||
      Type == EfiRuntimeServicesCode ||
      Type == EfiRuntimeServicesData ||
      Type == EfiACPIReclaimMemory ||
      Type == EfiACPIMemoryNVS
      ) {
    for (Index = 0; Index < PreservedMemorySize; Index++) {
      if ((PreservedMemoryTable[Index].Type == (UINT32)Type) &&
          (PreservedMemoryTable[Index].NumberOfPages == (UINT32)NumberOfPages)) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

EFI_STATUS
FindPreservedMemoryTableRange (
  VOID
  )
{
  EFI_STATUS                            Status;
  EFI_MEMORY_DESCRIPTOR                 *EfiMemoryMap;
  EFI_MEMORY_DESCRIPTOR                 *EfiMemoryMapEnd;
  EFI_MEMORY_DESCRIPTOR                 *EfiEntry;
  UINTN                                 EfiMemoryMapSize;
  UINTN                                 EfiMapKey;
  UINTN                                 EfiDescriptorSize;
  UINT32                                EfiDescriptorVersion;
  UINT64                                MemoryBlockLength;
  EFI_PHYSICAL_ADDRESS                  PhysicalEnd;
  UINTN                                 TableIndex;

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
    Status = gBS->AllocatePool (EfiBootServicesData, EfiMemoryMapSize, (VOID **) &EfiMemoryMap);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "FindPreservedMemoryRange:AllocatePool %r\n", Status));
      return Status;
    }
    Status = gBS->GetMemoryMap (
                    &EfiMemoryMapSize,
                    EfiMemoryMap,
                    &EfiMapKey,
                    &EfiDescriptorSize,
                    &EfiDescriptorVersion
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "FindPreservedMemoryRange:GetMemoryMap %r\n", Status));
      EfiMemoryMap = NULL;
    }
  }

  if (EfiMemoryMap == NULL) {
    DEBUG ((EFI_D_ERROR, "FindPreservedMemoryRange:EfiMemoryMap == NULL\n"));
    return EFI_ABORTED;
  }
  
  //
  // Calculate the system memory size from EFI memory map and resourceHob
  //
  EfiEntry        = EfiMemoryMap;
  EfiMemoryMapEnd = (EFI_MEMORY_DESCRIPTOR *) ((UINT8 *) EfiMemoryMap + EfiMemoryMapSize);
  
  while (EfiEntry < EfiMemoryMapEnd) {
    if (IsRtPreservedMemory (EfiEntry->Type, EfiEntry->NumberOfPages)) {
      for (TableIndex = 0; TableIndex < mPreservedMemoryAddrTableSize; TableIndex++){
        if ((UINT32)mPreservedMemoryAddrTable[TableIndex].Type == EfiEntry->Type) {
          MemoryBlockLength = (UINT64) (LShiftU64 (EfiEntry->NumberOfPages, 12));
          PhysicalEnd = EfiEntry->PhysicalStart + (EFI_PHYSICAL_ADDRESS)MemoryBlockLength;
          mPreservedMemoryAddrTable[TableIndex].PhyStart = EfiEntry->PhysicalStart;
          mPreservedMemoryAddrTable[TableIndex].PhyEnd = PhysicalEnd;
        }
      }
    }
    EfiEntry = NEXT_MEMORY_DESCRIPTOR (EfiEntry, EfiDescriptorSize);
  }
  
  DEBUG_CODE (
    DEBUG ((EFI_D_ERROR, "Preserved Runtime Memory Range Table\n"));
    DEBUG ((EFI_D_ERROR, "Type AddressStart     AddressEnd\n"));
    for (TableIndex= 0; TableIndex < mPreservedMemoryAddrTableSize; TableIndex++) {
      DEBUG ((EFI_D_ERROR, 
        "%04x 0x%016lx 0x%016lx\n", 
        mPreservedMemoryAddrTable[TableIndex].Type,
        mPreservedMemoryAddrTable[TableIndex].PhyStart,
        mPreservedMemoryAddrTable[TableIndex].PhyEnd
        ));
    }
  );  
  
  return EFI_SUCCESS;
}
//[-end-170109-IB07400830-add]//

EFI_STATUS
EFIAPI
Chipset2MemoryConsistencyCheck (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINTN                       VarSize;
  UINT32                      Attributes;
  BOOLEAN                     RebuildRequired;
  BOOLEAN                     BuildTableDone;
  UINTN                       Pages;
  EFI_PHYSICAL_ADDRESS        Address;
  EFI_BOOT_MODE               BootMode;
  EFI_PEI_HOB_POINTERS        Hob;
  UINT8                       Index;
  PEI_RUNTIME_MEMORY_TABLE    PeiMemoryRtTable[MAX_PEI_RUNTIME_MEMORY_TABLE];
  UINTN                       PeiMemoryRtTableSize;
//[-start-170109-IB07400830-add]//
  EFI_MEMORY_TYPE             TempMemType;
  EFI_PHYSICAL_ADDRESS        TempMemStart;
  EFI_PHYSICAL_ADDRESS        TempMemEnd;
  UINTN                       TableIndex;
//[-end-170109-IB07400830-add]//
  
  RebuildRequired = FALSE;
  
  BootMode = GetBootModeHob ();
  if (BootMode == BOOT_IN_RECOVERY_MODE) {
    return EFI_SUCCESS;
  }
//[-start-170109-IB07400830-add]//
  FindPreservedMemoryTableRange();
//[-end-170109-IB07400830-add]//

  //
  // Get Current PEI runtime Table
  //
  PeiMemoryRtTableSize = MAX_PEI_RUNTIME_MEMORY_TABLE * sizeof (PEI_RUNTIME_MEMORY_TABLE);
  VarSize = PeiMemoryRtTableSize;
  Status = gRT->GetVariable (
    L"PeiRuntimeMemoryTable",
    &gChipsetPkgTokenSpaceGuid,
    &Attributes,
    &VarSize,
    PeiMemoryRtTable
    );
  if (!EFI_ERROR (Status)) {
//[-start-170109-IB07400830-modify]//
    //
    // Check Range
    //
    DEBUG ((EFI_D_ERROR, "Check System Runtime Memory Map!\n"));
    DEBUG ((EFI_D_ERROR, "Type AddressStart     AddressEnd\n"));
    Hob.Raw = GetFirstHob (EFI_HOB_TYPE_MEMORY_ALLOCATION);
    ASSERT (Hob.Raw != NULL);
    while ((Hob.Raw != NULL) && (!END_OF_HOB_LIST (Hob))) {
      if (Hob.Header->HobType == EFI_HOB_TYPE_MEMORY_ALLOCATION &&
        IsRtMemType (Hob.MemoryAllocation->AllocDescriptor.MemoryType)) {

        TempMemType = Hob.MemoryAllocation->AllocDescriptor.MemoryType;
        TempMemStart = Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress;
        TempMemEnd = Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress + (EFI_PHYSICAL_ADDRESS)Hob.MemoryAllocation->AllocDescriptor.MemoryLength;

        DEBUG ((EFI_D_ERROR, "%04x 0x%016lx 0x%016lx\n", TempMemType, TempMemStart, TempMemEnd));
        
        for (TableIndex = 0; TableIndex < mPreservedMemoryAddrTableSize; TableIndex++){
          if (mPreservedMemoryAddrTable[TableIndex].Type == TempMemType) {
            if (!((TempMemStart >= mPreservedMemoryAddrTable[TableIndex].PhyStart) &&
                (TempMemEnd < mPreservedMemoryAddrTable[TableIndex].PhyEnd))) {
              RebuildRequired = TRUE;
            }
          }
        }
        if (RebuildRequired) break;
      }
      Hob.Raw = GET_NEXT_HOB (Hob);
      Hob.Raw = GetNextHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, Hob.Raw);
    }
//[-end-170109-IB07400830-modify]//
  } else {
    RebuildRequired = TRUE;
  }
//[-start-170112-IB07400833-add]//
  if (!RebuildRequired) {
    //
    // Re-Allocate Memory, because GCD will not add PEI Runtime Memory.
    //
    for (Index = 0; Index < MAX_PEI_RUNTIME_MEMORY_TABLE; Index++) {
      if (PeiMemoryRtTable[Index].MemIndex != 0) {
        Address = PeiMemoryRtTable[Index].MemAddress;
        Pages = EFI_SIZE_TO_PAGES ((UINTN)PeiMemoryRtTable[Index].MemLength);
        Status  = gBS->AllocatePages (
                         AllocateAddress,
                         PeiMemoryRtTable[Index].MemType,
                         Pages,
                         &Address
                         );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_ERROR, "AllocatePages = %r\n", Status));
          DEBUG ((EFI_D_ERROR, "Index = %x, ", Index));
          DEBUG ((EFI_D_ERROR, "PeiMemoryRtTable[Index].MemAddress = %016x\n", PeiMemoryRtTable[Index].MemAddress));
          RebuildRequired = TRUE;
          break;
        }
      }
    }
  }
//[-end-170112-IB07400833-add]//
  if (RebuildRequired) {
    //
    // Rebuild Table
    //
    ZeroMem (PeiMemoryRtTable, PeiMemoryRtTableSize);
    
    //
    // Count PEI runtime memory
    //
    Index = 0;
    Hob.Raw = GetFirstHob (EFI_HOB_TYPE_MEMORY_ALLOCATION);
    ASSERT (Hob.Raw != NULL);
    while ((Hob.Raw != NULL) && (!END_OF_HOB_LIST (Hob))) {
      if (Hob.Header->HobType == EFI_HOB_TYPE_MEMORY_ALLOCATION &&
          IsRtMemType (Hob.MemoryAllocation->AllocDescriptor.MemoryType)) {
          Index++;
      }
      Hob.Raw = GET_NEXT_HOB (Hob);
      Hob.Raw = GetNextHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, Hob.Raw);
    }
  
    //
    // Allocate PEI runtime memory
    //
    Index = 0;
    BuildTableDone = TRUE;
    Hob.Raw = GetFirstHob (EFI_HOB_TYPE_MEMORY_ALLOCATION);
    ASSERT (Hob.Raw != NULL);
    while ((Hob.Raw != NULL) && (!END_OF_HOB_LIST (Hob))) {
      if (Hob.Header->HobType == EFI_HOB_TYPE_MEMORY_ALLOCATION &&
          IsRtMemType (Hob.MemoryAllocation->AllocDescriptor.MemoryType)) {
          
          PeiMemoryRtTable[Index].MemIndex = Index + 1;
          PeiMemoryRtTable[Index].MemLength = Hob.MemoryAllocation->AllocDescriptor.MemoryLength;
          PeiMemoryRtTable[Index].MemType = Hob.MemoryAllocation->AllocDescriptor.MemoryType;
            
          Address = 0xFFFFFFFF;
          Pages = EFI_SIZE_TO_PAGES ((UINTN)PeiMemoryRtTable[Index].MemLength);
          Status  = gBS->AllocatePages (
                           AllocateMaxAddress,
                           PeiMemoryRtTable[Index].MemType,
                           Pages,
                           &Address
                           );
          if (EFI_ERROR (Status)) {
            BuildTableDone = FALSE;
            break;
          }
          PeiMemoryRtTable[Index].MemAddress = Address;
          
          Index++;
      }
      Hob.Raw = GET_NEXT_HOB (Hob);
      Hob.Raw = GetNextHob (EFI_HOB_TYPE_MEMORY_ALLOCATION, Hob.Raw);
    }

    if (BuildTableDone) {
      Status = gRT->SetVariable (
        L"PeiRuntimeMemoryTable",
        &gChipsetPkgTokenSpaceGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        PeiMemoryRtTableSize,
        PeiMemoryRtTable
        ); 
      if (!EFI_ERROR (Status)) {
        DEBUG_CODE (
          
          DEBUG ((EFI_D_ERROR, "PEI Runtime Memory Table\n"));
          DEBUG ((EFI_D_ERROR, "Index Type Length    Address\n"));
          for (Index = 0; Index < MAX_PEI_RUNTIME_MEMORY_TABLE; Index++) {
//[-start-161221-IB07400829-modify]//
            DEBUG ((EFI_D_ERROR, 
              "[%02d]  0x%02x 0x%08lx 0x%08lx\n", 
              PeiMemoryRtTable[Index].MemIndex,
              PeiMemoryRtTable[Index].MemType,
              PeiMemoryRtTable[Index].MemLength,
              PeiMemoryRtTable[Index].MemAddress
              ));
//[-end-161221-IB07400829-modify]//
          }
        );    
        CHIPSET_POST_CODE (DXE_MEMORY_CONSISTENCY_CHECK);
        IoWrite8 (0xCF9, 0x06);
      }
    }
  }
  
  return EFI_SUCCESS;
}
#else

//[-start-170406-IB07400855-remove]//
//EFI_STATUS
//EFIAPI
//Chipset2MemoryConsistencyEasyCheck (
//  VOID
//  )
//{
//  EFI_STATUS                  Status;
//  UINTN                       VarSize;
//  UINT8                       VarData;
//  UINT32                      Attributes;
//  EFI_BOOT_MODE               BootMode;
//  
//  BootMode = GetBootModeHob ();
//  if (BootMode == BOOT_IN_RECOVERY_MODE) {
//    return EFI_SUCCESS;
//  }
//  //
//  // Get Variable
//  //
//  VarSize = sizeof (UINT8);
//  Status = gRT->GetVariable (
//    L"MemoryConsistencyEasyCheck",
//    &gChipsetPkgTokenSpaceGuid,
//    &Attributes,
//    &VarSize,
//    &VarData
//    );
//  if (EFI_ERROR (Status)) {
//    //
//    // First Boot, Reboot 1 time for Memory Consistency Issue
//    //
//    VarData = 0x01;
//    VarSize = sizeof (UINT8);
//    Status = gRT->SetVariable (
//      L"MemoryConsistencyEasyCheck",
//      &gChipsetPkgTokenSpaceGuid,
//      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
//      VarSize,
//      &VarData
//      );
//    CHIPSET_POST_CODE (DXE_MEMORY_CONSISTENCY_CHECK);
//    IoWrite8 (0xCF9, 0x06);
//  } 
//  
//  return EFI_SUCCESS;
//}
//[-end-170406-IB07400855-remove]//
#endif
//[-end-161208-IB07400824-add]//

//[-start-170329-IB07400852-add]//
#ifdef DEFAULT_PASSWORD_SUPPORT

#include <Protocol/SysPasswordService.h>

EFI_STATUS
Chipset2DefaultPassword (
  VOID
  )
{
  EFI_STATUS                              Status;
  UINTN                                   VarSize;
  UINT8                                   PswExist = 0;
  EFI_SYS_PASSWORD_SERVICE_PROTOCOL       *SysPasswordService;
  UINTN                                   PswSize;
  CHAR16                                  DefaultPassword[] = DEFAULT_PASSWORD_STRING;
    
  VarSize = sizeof (UINT8);
  Status = gRT->GetVariable (
    L"DefSetupPswd",
    &gSystemConfigurationGuid,
    NULL,
    &VarSize,
    &PswExist
    );
  if (EFI_ERROR(Status)) {
    //
    // Default Password do not exist, set default password
    //
    Status = gBS->LocateProtocol (
                  &gEfiSysPasswordServiceProtocolGuid,
                  NULL,
                  (VOID **) &SysPasswordService
                  );
    if (!EFI_ERROR (Status)) {
      PswSize = sizeof (DefaultPassword);
      Status = SysPasswordService->SetPassword (
        SysPasswordService,
        DefaultPassword,
        PswSize,
        SystemSupervisor
        );
      ASSERT_EFI_ERROR (Status);

      PswExist = 0x01;
      Status = gRT->SetVariable (
        L"DefSetupPswd",
        &gSystemConfigurationGuid,
        EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
        VarSize,
        &PswExist
        );
      ASSERT_EFI_ERROR (Status);
    }
  }
  return EFI_SUCCESS;
}
#endif
//[-end-170329-IB07400852-add]//

EFI_STATUS
EFIAPI
Chipset2InitializePlatform (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{

  Chipset2DynamicUpdateSetupVariable ();
  Chipset2RegisterBootEvent ();
//[-start-161208-IB07400824-add]//
//[-start-170327-IB07400850-modify]//
  //
  // Only support in non-FDO mode
  //
  if (PcdGetBool (PcdFDOState) == 0) {
#ifdef PEI_MEMORY_CONSISTENCY_CHECK
    Chipset2MemoryConsistencyCheck ();
#else
//[-start-170406-IB07400855-remove]//
//    Chipset2MemoryConsistencyEasyCheck ();
//[-end-170406-IB07400855-remove]//
#endif
  }
//[-end-170327-IB07400850-modify]//
//[-end-161208-IB07400824-add]//

//[-start-170329-IB07400852-add]//
#ifdef DEFAULT_PASSWORD_SUPPORT
  Chipset2DefaultPassword ();
#endif
//[-end-170329-IB07400852-add]//

  return EFI_SUCCESS;
}
//[-end-160923-IB07400789-add]//
//[-end-160923-IB07400788-add]//

