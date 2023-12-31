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
/*++

Copyright (c) 2012 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiPlatform.c

Abstract:

  ACPI Platform Driver


--*/

#include <Npkt.h>
//[-start-170227-IB15550024-add]//
#include <KernelSetupConfig.h>
//[-end-170227-IB15550024-add]//
//[-start-160413-IB03090426-add]//
#include <Dbgp.h>
//[-end-160413-IB03090426-add]//
#include <PiDxe.h>
#include <Protocol/TcgService.h>
#include <Protocol/FirmwareVolume2.h>
//[-start-160216-IB03090424-remove]//
//#include <IncludePrivate/Guid/ScPolicyHobGuid.h>
//[-end-160216-IB03090424-remove]//
#include "AcpiPlatform.h"
#include "AcpiPlatformHooks.h"
#include "AcpiPlatformHooksLib.h"
#include "Platform.h"
#include <Hpet.h>
#include <Mcfg.h>
#include "Osfr.h"
#include <Dbg2t.h>
#include <Guid/GlobalVariable.h>
#include <Guid/PlatformInfo.h>
#include <Protocol/CpuIo.h>
#include <Slic.h>

#include <Library/SpiFlash.h>

#include <Guid/BoardFeatures.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/AcpiS3Save.h>
#include <Protocol/Ps2Policy.h>
#include <Guid/AcpiTableStorage.h>
#include <Guid/EfiVpdData.h>
#include <ScAccess.h>
#include <Protocol/SeCOperation.h>
#include <Guid/BxtVariable.h>
#include <Protocol/PublishAcpiTableDone.h>
#include <Bemt.h>
#include <Library/PcdLib.h>
#include <Library/DxeOemSvcChipsetLib.h>
#include <Library/DxeVtdLib.h>
#include <Library/DxeInsydeChipsetLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-160216-IB03090424-add]//
#include <Library/SideBandLib.h>
#include <Library/SteppingLib.h>
//[-end-160216-IB03090424-add]//
#include "CpuRegs.h"
//[-start-160509-IB03090427-add]//
#include <SaRegs.h>
//[-end-160509-IB03090427-add]//

#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/FirmwareVolume2.h>
#ifdef PRAM_SUPPORT

#include <Pram.h>
extern EFI_GUID gPramAddrDataGuid;
//[-start-160216-IB03090424-add]//
extern EFI_GUID gEfiPramConfGuid;
//[-end-160216-IB03090424-add]//
#endif
EFI_HANDLE  mAcpiPlatformHandle = NULL;


#include "CpuType.h"
#include <Guid/PlatformCpuInfo.h>
//[-start-160914-IB07400784-add]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-160914-IB07400784-add]//
//[-start-160923-IB07400789-add]//
#include <Library/CmosLib.h>
#include <ChipsetCmos.h>
//[-end-160923-IB07400789-add]//
//[-start-161027-IB07400806-add]//
#include <Protocol/CpuGlobalNvsArea.h>
//[-end-161027-IB07400806-add]//
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
#include <SaRegs.h>
#define MEM_EQU_4GB     0x100000000ULL
#endif
//[-end-170119-IB07401072-add]//

CHAR16    EfiPlatformCpuInfoVariable[]        = L"PlatformCpuInfo";
CHAR16    gACPIOSFRModelStringVariableName[]  = ACPI_OSFR_MODEL_STRING_VARIABLE_NAME;
CHAR16    gACPIOSFRRefDataBlockVariableName[] = ACPI_OSFR_REF_DATA_BLOCK_VARIABLE_NAME;
CHAR16    gACPIOSFRMfgStringVariableName[]    = ACPI_OSFR_MFG_STRING_VARIABLE_NAME;

EFI_CPU_IO2_PROTOCOL          *mCpuIo;

BOOLEAN                       mFirstNotify;
EFI_PLATFORM_INFO_HOB         *mPlatformInfo;
CHIPSET_CONFIGURATION     *mSystemConfiguration;
UINT8 mSmbusRsvdAddresses[] = PLATFORM_SMBUS_RSVD_ADDRESSES;
UINT8 mNumberSmbusAddress = sizeof( mSmbusRsvdAddresses ) / sizeof( mSmbusRsvdAddresses[0] );

extern EFI_GUID gPlatformSsdtImageGuid;

//[-start-160509-IB03090427-add]//
UINTN                         mMaximumNumberOfCpus;
UINTN                         mNumberOfEnabledCpus;
EFI_MP_SERVICES_PROTOCOL      *mMpService;  
//[-end-160509-IB03090427-add]//

//[-start-151228-IB03090424-add]//
//
// APCI Watchdog Action Table (WDAT)
//
UINT8                     EntryNumber;

EFI_ACPI_WATCHDOG_ACTION_TABLE mWatchDogActionTableAcpiTemplate = {
  // EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE
  {
    {
      EFI_ACPI_5_0_WATCHDOG_ACTION_TABLE_SIGNATURE,
      0x00,                                         // Length, will be updated
      EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE_REVISION,
      //
      // Compiler initializes the remaining bytes to 0
      // These fields should be filled in in production
      //
    },
    sizeof (EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE) - sizeof (EFI_ACPI_DESCRIPTION_HEADER),
    0x00FF,                                         // PCI Segment
    0xFF,                                           // PCI Bus Number
    0xFF,                                           // PCI Device Number
    0xFF,                                           // PCI Function Number
    0x00, 0x00, 0x00,                               // Reserved_45[3]
    0x00000258,                                     // TimerPeriod, 600 milliseconds
    0x000003FF,                                     // MaxCount
    0x00000002,                                     // MinCount
    0x00,                                           // WatchdogFlags, will be updated
    0x00, 0x00, 0x00,                               // Reserved_61[3]
    0x00,                                           // NumberWatchdogInstructionEntries, will be updated
  },
  // EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY
  /*
  {
    UINT8                                   WatchdogAction;
    UINT8                                   InstructionFlags;
    UINT8                                   Reserved_2[2];
    EFI_ACPI_2_0_GENERIC_ADDRESS_STRUCTURE  RegisterRegion;
    UINT32                                  Value;
    UINT32                                  Mask;
  },
  */
};

EFI_STATUS
PublishWatchDogActionTable (
  VOID
  );
//[-end-151228-IB03090424-add]//


//[-start-160413-IB03090426-add]//
EFI_STATUS
UpdateDbgpTable (
  IN  EFI_ACPI_SUPPORT_PROTOCOL   *AcpiSupport
  );
//[-end-160413-IB03090426-add]//

EFI_STATUS
UpdateDbg2Table (
  IN  EFI_ACPI_SUPPORT_PROTOCOL   *AcpiSupport
  );

//[-start-160422-IB03090426-add]//
//[-start-160506-IB07400723-modify]//
//#if BXTI_PF_ENABLE
extern EFI_STATUS AcpiHrotDxeEntryPoint (
  );
//#endif
//[-end-160506-IB07400723-modify]//
//[-end-160422-IB03090426-add]//



EFI_STATUS
SlicUpdateCommon (
  IN OUT EFI_ACPI_COMMON_HEADER       *Table
  );

EFI_STATUS
FacsUpdateCommon (
  IN OUT EFI_ACPI_COMMON_HEADER       *Table,
  IN CHIPSET_CONFIGURATION             *SetupVariable
);

EFI_STATUS
FacpUpdateCommon (
  IN OUT EFI_ACPI_COMMON_HEADER       *Table,
  IN CHIPSET_CONFIGURATION             *SetupVariable
  );

//[-start-161023-IB07400803-add]//
VOID
DebugDumpHex (
  IN UINT8   *Data8,
  IN UINTN   DataSize
  )
{
  UINTN      Index;

  for (Index = 0; Index < DataSize; Index++) {
    if (Index % 0x10 == 0) {
      DEBUG ((EFI_D_INFO, "\n%08X:", Index));
    }
    DEBUG ((EFI_D_INFO, " %02X", *Data8++));
  }
  DEBUG ((EFI_D_INFO, "\n"));
}
//[-end-161023-IB07400803-add]//

VOID
PrintGnvs (
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea
)
{
//[-start-161023-IB07400803-modify]//
//  UINTN Index;
  UINT8 *Area;

  Area = (UINT8 *) (GlobalNvsArea->Area);
  DEBUG ((EFI_D_ERROR, "Print GNVS on ready boot\n"));
  DEBUG ((EFI_D_ERROR, "Size of area is = %d\n", sizeof(EFI_GLOBAL_NVS_AREA)));
//  for (Index=0; Index<sizeof(EFI_GLOBAL_NVS_AREA); Index++) {
//    DEBUG ((EFI_D_ERROR, "Area[%d] = 0x%X\n", Index, Area[Index]));
//  }
  DebugDumpHex (Area, sizeof(EFI_GLOBAL_NVS_AREA));
//[-end-161023-IB07400803-modify]//
}
/*++

Routine Description:

Locate the first instance of a protocol.  If the protocol requested is an
FV protocol, then it will return the first FV that contains the ACPI table
storage file.

Arguments:

Protocol  -  The protocol to find.
Instance  -  Return pointer to the first instance of the protocol.
Type      -  The type of protocol to locate.

Returns:

EFI_SUCCESS           -  The function completed successfully.
EFI_NOT_FOUND         -  The protocol could not be located.
EFI_OUT_OF_RESOURCES  -  There are not enough resources to find the protocol.

--*/
EFI_STATUS
LocateSupportProtocol (
  IN   EFI_GUID       *Protocol,
  OUT  VOID           **Instance,
  IN   UINT32         Type
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              *HandleBuffer;
  UINTN                   NumberOfHandles;
  EFI_FV_FILETYPE         FileType;
  UINT32                  FvStatus;
  EFI_FV_FILE_ATTRIBUTES  Attributes;
  UINTN                   Size;
  UINTN                   Index;

  FvStatus = 0;
  //
  // Locate protocol.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  Protocol,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    //
    // Defined errors at this time are not found and out of resources.
    //
    return Status;
  }

  //
  // Looking for FV with ACPI storage file
  //
  for (Index = 0; Index < NumberOfHandles; Index++) {
    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    Protocol,
                    Instance
                    );
    ASSERT (!EFI_ERROR (Status));

    if (!Type) {
      //
      // Not looking for the FV protocol, so find the first instance of the
      // protocol.  There should not be any errors because our handle buffer
      // should always contain at least one or LocateHandleBuffer would have
      // returned not found.
      //
      break;
    }

    //
    // See if it has the ACPI storage file
    //
    Status = ((EFI_FIRMWARE_VOLUME2_PROTOCOL*) (*Instance))->ReadFile (
                                                               *Instance,
                                                               &gEfiAcpiTableStorageGuid,
                                                               NULL,
                                                               &Size,
                                                               &FileType,
                                                               &Attributes,
                                                               &FvStatus
                                                               );

    //
    // If we found it, then we are done
    //
    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //
  //
  // Free any allocated buffers
  //
  gBS->FreePool (HandleBuffer);

  return Status;
}

//[-start-160216-IB03090424-modify]//
/**
Installing all the newly created SSDTs
**/
EFI_STATUS
InstallAcpiTableForPlatformSsdt (
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *FwVol,
  EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport
  )
{
  UINTN                         TableHandle;
  UINTN                         Instance;
  UINTN                         Size;
  UINT32                        FvStatus;
  EFI_ACPI_COMMON_HEADER        *CurrentTable;
  EFI_STATUS                    Status;
  BOOLEAN                       FoundSsdt;
  EFI_ACPI_TABLE_VERSION        TableVersion;
  BOOLEAN                       InstallTable;

  //
  // Read SSDT tables from the storage file.
  //
  Status    = EFI_SUCCESS;
  Instance  = 0;
  FoundSsdt = FALSE;
  TableVersion = EFI_ACPI_TABLE_VERSION_2_0;
  InstallTable = FALSE;

  while (!EFI_ERROR (Status)) {
    CurrentTable = NULL;

    Status = FwVol->ReadSection (
                      FwVol,
                      &gPlatformSsdtImageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      &CurrentTable,
                      (UINTN *) &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      //
      // Add the table
      //
      TableHandle = 0;
      InstallTable = TRUE;
      DEBUG ((DEBUG_INFO, "Found SSDT %x index = %d \n", CurrentTable, Instance));

#if (ENBDT_PF_ENABLE == 1)
      if  (((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->OemTableId == SIGNATURE_64 ('I', 'r', 'm', 't', 'T', 'a', 'b', 'l')) {
        if (mSystemConfiguration->IrmtConfiguration == 1) {
          DEBUG((DEBUG_INFO, "IRMT: Publishing IrmtTabl\n"));
        } else {
          DEBUG((DEBUG_INFO, "IRMT: Found IrmtTabl but disabled, will not publish\n"));
          InstallTable = FALSE;
        }
      }
#endif

      if (InstallTable) {
        //
        // Install the table
        //
        Status = AcpiSupport->SetAcpiTable (
                                      AcpiSupport,
                                      CurrentTable,
                                      TRUE,
                                      TableVersion,
                                      &TableHandle
                                      );
        ASSERT_EFI_ERROR (Status);
      }

      FoundSsdt = TRUE;
      Instance++;
    }
  }

  if (FoundSsdt) {
    Status = EFI_SUCCESS;
  }

  return Status;

}
//[-end-160216-IB03090424-modify]//
/*++

Routine Description:

This function will update any runtime platform specific information.
This currently includes:
Setting OEM table values, ID, table ID, creator ID and creator revision.
Enabling the proper processor entries in the APIC tables.

Arguments:

Table  -  The table to update

Returns:

EFI_SUCCESS  -  The function completed successfully.

--*/
EFI_STATUS
PlatformUpdateTables (
  IN OUT EFI_ACPI_COMMON_HEADER  *Table
  )
{
  EFI_ACPI_DESCRIPTION_HEADER                                 *TableHeader;
  UINT8                                                       *CurrPtr;
  UINT8                                                       *EndPtr;
  ACPI_APIC_STRUCTURE_PTR                                     *ApicPtr;
  UINT8                                                       CurrProcessor;
  EFI_STATUS                                                  Status;
//[-start-160509-IB03090427-remove]//
//  EFI_MP_SERVICES_PROTOCOL                                    *MpService;
//  UINTN                                                       MaximumNumberOfCPUs;
//  UINTN                                                       NumberOfEnabledCPUs;
//[-end-160509-IB03090427-remove]//
  UINTN                                                       BufferSize;
  ACPI_APIC_STRUCTURE_PTR                                     *ProcessorLocalApicEntry;
  UINTN                                                       BspIndex;
  EFI_ACPI_1_0_ASF_DESCRIPTION_TABLE                          *AsfEntry;
  EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER            *HpetTbl;
  UINT64                                                      OemIdValue;
  UINT8                                                       Index;
  UINT8                                                       Data;
  EFI_ACPI_OSFR_TABLE                                         *pOsfrTable;
  EFI_ACPI_OSFR_OCUR_OBJECT                                   *pOcurObject;
  EFI_ACPI_OSFR_OCUR_OBJECT                                   OcurObject = {{0xB46F133D, 0x235F, 0x4634, 0x9F, 0x03, 0xB1, 0xC0, 0x1C, 0x54, 0x78, 0x5B}, 0, 0, 0, 0, 0};
  UINT32                                                      NpkTPciMmBase = 0xFFFFFFFF;
  CHAR16                                                      *OcurMfgStringBuffer = NULL, *OcurModelStringBuffer = NULL;
  UINT8                                                       *OcurRefDataBlockBuffer = NULL;
  UINTN                                                       OcurMfgStringBufferSize, OcurModelStringBufferSize, OcurRefDataBlockBufferSize;
  UINT16                                                      NumberOfHpets;
  UINT16                                                      HpetCapIdValue;
  UINT32                                                      HpetBlockID;
  UINTN                                                       LocalApicCounter;
  EFI_PROCESSOR_INFORMATION                                   ProcessorInfoBuffer;
  UINT8                                                       TempVal;
//[-start-160216-IB03090424-remove]//
//  EFI_ACPI_3_0_IO_APIC_STRUCTURE                              *IOApicType;
//[-end-160216-IB03090424-remove]//
  EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER         *APICTableHeader;
  EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE                   *pFACP;
#ifdef PRAM_SUPPORT
//[-start-160216-IB03090424-add]//
  CHIPSET_CONFIGURATION                                       SetupVarBuffer;
//[-end-160216-IB03090424-add]//
  UINTN                                                       VariableSize;
  EFI_HOB_GUID_TYPE                                           *GuidHob = NULL;
  EFI_PHYSICAL_ADDRESS                                        PramBaseAddress = 0;
  EFI_PHYSICAL_ADDRESS                                        PramBaseAddressVar = 0;
  UINT32                                                      PramSizeVar = 0;
  UINT32                                                      PramSize = 0;
  EFI_STATUS                                                  Status1;
//[-start-160216-IB03090424-add]//
  UINT8                                                       PramConfigVariable;
//[-end-160216-IB03090424-add]//
#endif
//[-start-160216-IB03090424-add]//
  UINT8                                                       IoApicId;
//[-end-160216-IB03090424-add]//
//[-start-160803-IB07220122-add]//
  UINT32                                                      VariableAttributes;
//[-end-160803-IB07220122-add]//

  CurrPtr                 = NULL;
  EndPtr                  = NULL;
  ApicPtr                 = NULL;
  LocalApicCounter        = 0;
  CurrProcessor           = 0;
  ProcessorLocalApicEntry = NULL;


  // Check for presence of SLP 2.0 data in proper variables
 if (Table->Signature != EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) {
    TableHeader = (EFI_ACPI_DESCRIPTION_HEADER *) Table;

    //
    // Update the OEMID
    //
    OemIdValue = mPlatformInfo->AcpiOemId;

    // Override ACPI OEMID from valid SLP 2.0 data
    *(UINT32 *)(TableHeader->OemId)     = (UINT32)OemIdValue;
    *(UINT16 *)(TableHeader->OemId + 4) = *(UINT16*)(((UINT8 *)&OemIdValue) + 4);

    if (Table->Signature != EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
    //
    // Update the OEM Table ID
    //
      TableHeader->OemTableId = mPlatformInfo->AcpiOemTableId;
    }
    //
    // Update the OEM Table ID
    //
    TableHeader->OemRevision = EFI_ACPI_OEM_REVISION;

    //
    // Update the creator ID
    //
    TableHeader->CreatorId = EFI_ACPI_CREATOR_ID;

    //
    // Update the creator revision
    //
    TableHeader->CreatorRevision = EFI_ACPI_CREATOR_REVISION;
  }
  //
  // Complete this function
  //
//[-start-160509-IB03090427-remove]//
//  //
//  // Locate the MP services protocol
//  //
//  //
//  // Find the MP Protocol. This is an MP platform, so MP protocol must be
//  // there.
//  //
//  Status = gBS->LocateProtocol (
//                  &gEfiMpServiceProtocolGuid,
//                  NULL,
//                  (VOID **) &MpService
//                  );
//
//  if (EFI_ERROR (Status)) {
//    return Status;
//  }
//
//  //
//  // Determine the number of processors
//  //
//  MpService->GetNumberOfProcessors (
//               MpService,
//               &MaximumNumberOfCPUs,
//               &NumberOfEnabledCPUs
//               );
//
//  ASSERT (MaximumNumberOfCPUs <= MAX_CPU_NUM && NumberOfEnabledCPUs >= 1);
//[-end-160509-IB03090427-remove]//

  //
  // Assign a invalid initial value for update
  //
  //
  // Update the processors in the APIC table
  //
  switch (Table->Signature) {
    case EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE:
      FacsUpdateCommon(Table, mSystemConfiguration);
      break;

    case EFI_ACPI_SLIC_TABLE_SIGNATURE:
      Status = SlicUpdateCommon (Table);
      break;

    case EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
      pFACP = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *) Table;
//[-start-160216-IB03090424-modify]//
//[-start-160914-IB07400784-remove]//
      //
      // Hook by OsResetSmi (platform.c - SMM)
      //
//      pFACP->ResetValue = mSystemConfiguration->ResetSelect;
//      pFACP->Flags |= BIT10;
//      DEBUG ((DEBUG_INFO, "FACP ResetValue = %x\n", pFACP->ResetValue));
//[-end-160914-IB07400784-remove]//

      //
      // if Native ASPM is disabled, set FACP table to skip Native ASPM
      //
      if ((mSystemConfiguration->PciExpNative == 0) || (mSystemConfiguration->NativeAspmEnable == 0x0)) {
        pFACP->IaPcBootArch |= 0x10;
      }
//[-end-160216-IB03090424-modify]//
      if(mSystemConfiguration->LowPowerS0Idle) {
        DEBUG((DEBUG_INFO,"LowPowerS0Idle enabled.\n"));
        pFACP->Flags |= BIT21;
      }

//[-start-161101-IB03090435-modify]//
      //
      // Modify FADT Fixed Feature Flag to Configure Power button as Control Method.
      //
//[-start-161125-IB07400818-modify]//
//#if (ENBDT_PF_ENABLE == 1)
    if (!IsIOTGBoardIds()) { // CCG CRB
//[-start-170829-IB07400902-modify]//
      if ((mSystemConfiguration->OsSelection != 1) && (mSystemConfiguration->OsSelection != 3)) {
//[-end-170829-IB07400902-modify]//
        pFACP->Flags |= EFI_ACPI_2_0_PWR_BUTTON; // Set Fixed Power Button flag (Disabling Fixed Power button and enabling Control Method PB)
      } else {
      	pFACP->Flags &= ~(EFI_ACPI_2_0_PWR_BUTTON);
      }
//#else
    } else {
      pFACP->Flags |= EFI_ACPI_2_0_PWR_BUTTON; // Set Fixed Power Button flag (Disabling Fixed Power button and enabling Control Method PB)
    }  
//#endif
//[-end-161125-IB07400818-modify]//
//[-end-161101-IB03090435-modify]//
      FacpUpdateCommon(Table, mSystemConfiguration);
      break;

    case EFI_ACPI_1_0_ASF_DESCRIPTION_TABLE_SIGNATURE:
      //
      // Update the table if ASF is enabled. Otherwise, return error so caller will not install
      //
      if (mSystemConfiguration->Asf == 1) {
        return  EFI_UNSUPPORTED;
      }
      AsfEntry = (EFI_ACPI_1_0_ASF_DESCRIPTION_TABLE *) Table;
      TempVal = (mNumberSmbusAddress < ASF_ADDR_DEVICE_ARRAY_LENGTH)? mNumberSmbusAddress : ASF_ADDR_DEVICE_ARRAY_LENGTH;
      for (Index = 0; Index < TempVal; Index++) {
        AsfEntry->AsfAddr.FixedSmbusAddresses[Index] = mSmbusRsvdAddresses[Index];
      }
      break;

    case EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE:

//[-start-160509-IB03090427-modify]//
      Status = mMpService->WhoAmI (
                            mMpService,
                            &BspIndex
                            );
//[-end-160509-IB03090427-modify]//
      //
      // PCAT_COMPAT Set to 1 indicate 8259 vectors should be disabled
      //
      APICTableHeader = (EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER *)Table;
      APICTableHeader->Flags |= EFI_ACPI_3_0_PCAT_COMPAT;
      CurrPtr = (UINT8 *) &((EFI_ACPI_DESCRIPTION_HEADER *) Table)[1];
      CurrPtr = CurrPtr + 8;

      //
      // Size of Local APIC Address & Flag
      //
      EndPtr  = (UINT8 *) Table;
      EndPtr  = EndPtr + Table->Length;

      while (CurrPtr < EndPtr) {
        ApicPtr = (ACPI_APIC_STRUCTURE_PTR *) CurrPtr;

        switch (ApicPtr->AcpiApicCommon.Type) {
          case EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC:
            //
            // ESS override
            // Fix for Ordering of MADT to be maintained as it is in MADT table.
            //
            // Update processor enabled or disabled and keep the local APIC
            // order in MADT intact
            //
            // Sanity check to make sure proc-id is not arbitrary
            //
//[-start-160509-IB03090427-modify]//
            DEBUG ((DEBUG_INFO, "ApicPtr->AcpiLocalApic.AcpiProcessorId = %x, MaximumNumberOfCPUs = %x\n", ApicPtr->AcpiLocalApic.AcpiProcessorId, mMaximumNumberOfCpus));
//[-end-160509-IB03090427-modify]//

            BufferSize                    = 0;
            ApicPtr->AcpiLocalApic.Flags  = 0;

//[-start-160509-IB03090427-modify]//
            for (CurrProcessor = 0; CurrProcessor < mMaximumNumberOfCpus; CurrProcessor++) {
              Status = mMpService->GetProcessorInfo (
                                    mMpService,
                                    CurrProcessor,
                                    &ProcessorInfoBuffer
                                    );
//[-end-160509-IB03090427-modify]//

              if (Status == EFI_SUCCESS && ProcessorInfoBuffer.ProcessorId == ApicPtr->AcpiLocalApic.ApicId) {
                //
                // Check to see whether or not a processor (or thread) is enabled
                //
                if (BspIndex == CurrProcessor || ((ProcessorInfoBuffer.StatusFlag & PROCESSOR_ENABLED_BIT) != 0)) {
//[-start-160928-IB07400789-add]//
                  //
                  // BugBug: Legacy Free work unstable with multi-core, force one core here
                  //
                  if ((mGlobalNvsArea.Area->LegacyFreeSupport) && (BspIndex != CurrProcessor)) {
                    break;
                  }
//[-end-160928-IB07400789-add]//
//[-start-151228-IB03090424-modify]//
                  ApicPtr->AcpiLocalApic.Flags = EFI_ACPI_1_0_LOCAL_APIC_ENABLED;
//[-end-151228-IB03090424-modify]//

                  AppendCpuMapTableEntry (&(ApicPtr->AcpiLocalApic));
                }
                break;
              }
            }

            //
            // If no APIC-ID match, the CPU may not be populated
            //
            break;

          case EFI_ACPI_3_0_IO_APIC:
//[-start-160216-IB03090424-modify]//
            //
            // IO APIC entries can be patched here
            //
            DEBUG ((DEBUG_INFO, " Original ApicPtr->AcpiIoApic.IoApicId = %x\n", \
               ApicPtr->AcpiIoApic.IoApicId));

            // Get IO APIC ID
            MmioWrite8 ((UINTN) R_IO_APIC_INDEX, R_IO_APIC_ID);
            IoApicId = MmioRead32 ((UINTN) R_IO_APIC_WINDOW) >> 24;
            ApicPtr->AcpiIoApic.IoApicId = IoApicId;

            DEBUG ((DEBUG_INFO, " Update ApicPtr->AcpiIoApic.IoApicId = %x\n", \
              ApicPtr->AcpiIoApic.IoApicId));
//[-end-160216-IB03090424-modify]//
            break;
        }

        CurrPtr = CurrPtr + ApicPtr->AcpiApicCommon.Length;
      }
      break;

    case EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      //
      // Patch the memory resource
      //
      PatchDsdtTable ((EFI_ACPI_DESCRIPTION_HEADER *) Table);
      break;

    case EFI_ACPI_3_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
      //
      // Gv3 support
      //
      // TBD: Need re-design based on the Broxton platform.
      //

      break;

    case EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE:
      //
      // Adjust HPET Table to correct the Base Address
      //
      // Enable HPET always as Hpet.asi always indicates that Hpet is enabled.
      MmioOr8 (R_HPET + R_HPET_GCFG, B_HPET_GCFG_EN);

      //
      // Update CMOS to reflect HPET enable/disable for ACPI usage
      //
      Data = ReadCmosBank1Byte (CmosAcpiTableFlagAddress);
      Data &= ~B_CMOS_HPET_ENABLED;

      Data |= B_CMOS_HPET_ENABLED;
      WriteCmosBank1Byte (CmosAcpiTableFlagAddress, Data);

      HpetTbl = (EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER *) Table;
      HpetTbl->BaseAddressLower32Bit.Address = HPET_BASE_ADDRESS;
      HpetTbl->EventTimerBlockId = *((UINT32*)(UINTN)HPET_BASE_ADDRESS);

      HpetCapIdValue = *(UINT16 *)(UINTN)(HPET_BASE_ADDRESS);
      NumberOfHpets = HpetCapIdValue & B_HPET_GCID_NT;  // Bits [8:12] contains the number of Hpets
      HpetBlockID = EFI_ACPI_EVENT_TIMER_BLOCK_ID;

      if((NumberOfHpets) && (NumberOfHpets & B_HPET_GCID_NT)) {
        HpetBlockID |= (NumberOfHpets);
      }
      HpetTbl->EventTimerBlockId = HpetBlockID;
      break;

    case EFI_ACPI_3_0_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE:
      //
      // Update MCFG base and end bus number
      //
      ((EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE *) Table)->Segment[0].BaseAddress
        = mPlatformInfo->PciData.PciExpressBase;

      ((EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE *) Table)->Segment[0].EndBusNumber
        = (UINT8)RShiftU64 (mPlatformInfo->PciData.PciExpressSize, 20) - 1;
      break;
    case EFI_ACPI_OSFR_TABLE_SIGNATURE:
      // Get size of OSFR variable
      OcurMfgStringBufferSize = 0;
      Status = gRT->GetVariable (gACPIOSFRMfgStringVariableName, &gACPIOSFRMfgStringVariableGuid, NULL, &OcurMfgStringBufferSize, NULL);

      if (Status != EFI_BUFFER_TOO_SMALL) {
        // Variable must not be present on the system
        return EFI_UNSUPPORTED;
      }

      // Allocate memory for variable data
      OcurMfgStringBuffer = AllocatePool (OcurMfgStringBufferSize);

      Status = gRT->GetVariable (gACPIOSFRMfgStringVariableName, &gACPIOSFRMfgStringVariableGuid, NULL, &OcurMfgStringBufferSize, OcurMfgStringBuffer);

      if (!EFI_ERROR (Status)) {
        OcurModelStringBufferSize = 0;
        Status = gRT->GetVariable (gACPIOSFRModelStringVariableName, &gACPIOSFRModelStringVariableGuid, NULL, &OcurModelStringBufferSize, NULL);

        if (Status != EFI_BUFFER_TOO_SMALL) {
          // Variable must not be present on the system
          return EFI_UNSUPPORTED;
        }

        // Allocate memory for variable data
        OcurModelStringBuffer = AllocatePool (OcurModelStringBufferSize);
        Status = gRT->GetVariable (gACPIOSFRModelStringVariableName, &gACPIOSFRModelStringVariableGuid, NULL, &OcurModelStringBufferSize, OcurModelStringBuffer);

        if (!EFI_ERROR (Status)) {
          OcurRefDataBlockBufferSize = 0;
          Status = gRT->GetVariable (gACPIOSFRRefDataBlockVariableName, &gACPIOSFRRefDataBlockVariableGuid, NULL, &OcurRefDataBlockBufferSize, NULL);

          if (Status == EFI_BUFFER_TOO_SMALL) {
            // Allocate memory for variable data
            OcurRefDataBlockBuffer = AllocatePool (OcurRefDataBlockBufferSize);
            Status = gRT->GetVariable (gACPIOSFRRefDataBlockVariableName, &gACPIOSFRRefDataBlockVariableGuid, NULL, &OcurRefDataBlockBufferSize, OcurRefDataBlockBuffer);
          }

          pOsfrTable = (EFI_ACPI_OSFR_TABLE *) Table;

          // Currently only one object is defined: OCUR_OSFR_TABLE
          pOsfrTable->ObjectCount = 1;

          // Initialize table length to fixed portion of the ACPI OSFR table
          pOsfrTable->Header.Length = sizeof (EFI_ACPI_OSFR_TABLE_FIXED_PORTION);
          *(UINT32 *)((UINTN) pOsfrTable + sizeof (EFI_ACPI_OSFR_TABLE_FIXED_PORTION)) = \
              (UINT32) (sizeof (EFI_ACPI_OSFR_TABLE_FIXED_PORTION) + sizeof (UINT32));

          pOcurObject = (EFI_ACPI_OSFR_OCUR_OBJECT *)((UINTN) pOsfrTable + sizeof (EFI_ACPI_OSFR_TABLE_FIXED_PORTION) + \
              sizeof (UINT32));

          CopyMem (pOcurObject, &OcurObject, sizeof (EFI_ACPI_OSFR_OCUR_OBJECT));
          pOcurObject->ManufacturerNameStringOffset = (UINT32)((UINTN) pOcurObject - (UINTN) pOsfrTable + sizeof (EFI_ACPI_OSFR_OCUR_OBJECT));
          pOcurObject->ModelNameStringOffset = (UINT32)((UINTN) pOcurObject - (UINTN) pOsfrTable + sizeof (EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize);

          if (OcurRefDataBlockBufferSize > 0) {
            pOcurObject->MicrosoftReferenceOffset = (UINT32)((UINTN) pOcurObject - (UINTN) pOsfrTable + sizeof (EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize + OcurModelStringBufferSize);
          }

          CopyMem ((UINTN *)((UINTN) pOcurObject + sizeof (EFI_ACPI_OSFR_OCUR_OBJECT)), OcurMfgStringBuffer, OcurMfgStringBufferSize);
          CopyMem ((UINTN *)((UINTN) pOcurObject + sizeof (EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize), \
            OcurModelStringBuffer, OcurModelStringBufferSize);

          if (OcurRefDataBlockBufferSize > 0) {
            CopyMem ((UINTN *)((UINTN) pOcurObject + sizeof (EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize + OcurModelStringBufferSize), \
              OcurRefDataBlockBuffer, OcurRefDataBlockBufferSize);
          }

          pOsfrTable->Header.Length += (UINT32)(OcurMfgStringBufferSize + OcurModelStringBufferSize + OcurRefDataBlockBufferSize);
          pOsfrTable->Header.Length += sizeof (EFI_ACPI_OSFR_OCUR_OBJECT) + sizeof (UINT32);
        }
      }

      gBS->FreePool (OcurMfgStringBuffer);
      gBS->FreePool (OcurModelStringBuffer);
      gBS->FreePool (OcurRefDataBlockBuffer);
      break;

    case EFI_ACPI_NORTH_PEAK_TABLE_SIGNATURE:
      NpkTPciMmBase = (UINT32)MmPciAddress (0,
                                DEFAULT_PCI_BUS_NUMBER_SC,
                                PCI_DEVICE_NUMBER_NPK,
                                PCI_FUNCTION_NUMBER_NPK,
                                0
                                );

      if (MmioRead32 (NpkTPciMmBase) != 0xFFFFFFFF) {
        DEBUG((DEBUG_INFO," Patching NPKT Table \n"));
        PatchNpktTable ((EFI_ACPI_DESCRIPTION_HEADER *) Table);
      }
      break;

#ifdef PRAM_SUPPORT
    case EFI_ACPI_PRAM_BASE_ADDRESS_TABLE_SIGNATURE:
//[-start-160216-IB03090424-modify]//
      VariableSize = sizeof(UINTN);
      Status = gRT->GetVariable (
                      L"PRAM_Conf",
                      &gEfiPramConfGuid,
                      NULL,
                      &VariableSize,
                      &PramConfigVariable
                      );
      if (Status == EFI_SUCCESS && PramConfigVariable != mSystemConfiguration->Pram) {
        DEBUG ((DEBUG_INFO, "PramConfigVariable =  0x%x\n", PramConfigVariable));
        VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160803-IB07220122-modify]//
//         Status = GetSystemConfigData(&SetupVarBuffer, &VariableSize);
        Status = gRT->GetVariable (
                        SETUP_VARIABLE_NAME,
                        &gSystemConfigurationGuid,
                        &VariableAttributes,
                        &VariableSize,
                        &SetupVarBuffer
                        ); 
//[-end-160803-IB07220122-modify]//
        ASSERT_EFI_ERROR (Status);

        if (EFI_ERROR (Status)) {
          Status = GetChipsetSetupVariableDxe (&SetupVarBuffer, sizeof (CHIPSET_CONFIGURATION));  
          if (EFI_ERROR (Status)) {
            return Status;  
          }
        }

        SetupVarBuffer.Pram = PramConfigVariable;
//[-start-160803-IB07220122-modify]//
//         Status = SetSystemConfigData(&SetupVarBuffer, VariableSize);
        Status =  gRT->SetVariable (
                        SETUP_VARIABLE_NAME,
                        &gSystemConfigurationGuid,
                        VariableAttributes,
                        VariableSize,
                        &SetupVarBuffer
                        );
//[-end-160803-IB07220122-modify]//
        ASSERT_EFI_ERROR (Status);

        //
        // WA for variable cannot be accessed before memory init
        //
        DEBUG ((DEBUG_INFO, "EfiResetWarm for Pram \n"));
        gRT->ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
        CpuDeadLoop();
      } else {
        PramConfigVariable = mSystemConfiguration->Pram;
      }

      if (PramConfigVariable !=  0x30) {
        switch (PramConfigVariable) {
          case 0x31:  // 4MB
              PramSize = 0x400000;
              break;
          case 0x32: // 16MB
              PramSize = 0x1000000;
              break;
          case 0x33: // 64MB
              PramSize = 0x4000000;
              break;
        }

        GuidHob = GetFirstGuidHob(&gPramAddrDataGuid);
        if (GuidHob == NULL) {
          ASSERT_EFI_ERROR (Status);
          return EFI_NOT_FOUND;
        }
        PramBaseAddress = *(EFI_PHYSICAL_ADDRESS *)GET_GUID_HOB_DATA (GuidHob);
        DEBUG ((DEBUG_INFO, "PramBaseAddress =  0x%lx\n", PramBaseAddress));

        VariableSize = sizeof (EFI_PHYSICAL_ADDRESS);
        Status = gRT->GetVariable (
                        L"PramBaseAddress",
                        &gPramAddrDataGuid,
                        NULL,
                        &VariableSize,
                        &PramBaseAddressVar
                        );
        VariableSize = sizeof (UINT32);
        Status1 = gRT->GetVariable (
                         L"PramSize",
                         &gPramAddrDataGuid,
                         NULL,
                         &VariableSize,
                         &PramSizeVar
                         );
        if ( EFI_ERROR(Status) || EFI_ERROR(Status1) || (PramBaseAddressVar!=PramBaseAddress) || (PramSizeVar!=PramSize)) {
          Status = gRT->SetVariable (
                          L"PramBaseAddress",
                          &gPramAddrDataGuid,
                          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                          sizeof (EFI_PHYSICAL_ADDRESS),
                          &PramBaseAddress
                          );
          Status = gRT->SetVariable (
                          L"PramSize",
                          &gPramAddrDataGuid,
                          EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                          sizeof (UINT32),
                          &PramSize
                          );
          ((EFI_ACPI_PRAM_BASE_ADDRESS_TABLE *) Table)->PramSize = PramSize;
          ((EFI_ACPI_PRAM_BASE_ADDRESS_TABLE *) Table)->PramBaseAddress = PramBaseAddress;
        } else {
          ((EFI_ACPI_PRAM_BASE_ADDRESS_TABLE *) Table)->PramSize = PramSize;
          ((EFI_ACPI_PRAM_BASE_ADDRESS_TABLE *) Table)->PramBaseAddress = PramBaseAddress;
        }
      }
//[-start-170323-IB07400848-add]//
      else {
        //
        // PRAM disabled, disable this ACPI table
        //
        return EFI_NOT_FOUND;
      }
//[-end-170323-IB07400848-add]//
//[-end-160216-IB03090424-modify]//
      break;
#endif

    default:
      break;
  }
  //
  //
  // Update the hardware signature in the FACS structure
  //
  //
  // Locate the SPCR table and update based on current settings.
  // The user may change CR settings via setup or other methods.
  // The SPCR table must match.
  //
  return EFI_SUCCESS;
}

/*++

Routine Description:

GC_TODO: Add function description

Arguments:

Event   - GC_TODO: add argument description
Context - GC_TODO: add argument description

Returns:

GC_TODO: add return values

--*/
//[-start-151204-IB02950555-modify]//
//[-start-160216-IB03090424-modify]//
STATIC
VOID
EFIAPI
OnReadyToBoot (
  IN      EFI_EVENT             Event,
  IN      VOID                  *Context
  )
{
  EFI_STATUS                    Status;

  EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport;
  EFI_ACPI_S3_SAVE_PROTOCOL     *AcpiS3Save;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
  EFI_ACPI_TABLE_VERSION        TableVersion;
  CHIPSET_CONFIGURATION         SetupVarBuffer;
  EFI_PLATFORM_CPU_INFO         *PlatformCpuInfoPtr = NULL;
  EFI_PLATFORM_CPU_INFO         PlatformCpuInfo;
  EFI_PEI_HOB_POINTERS          GuidHob;
  UINTN                         VariableSize;
  UINTN                         PciMmBase             = 0;
  UINT32                        BaseAddress           = 0;

  if (mFirstNotify) {
    return;
  }

  Status = LocateSupportProtocol (&gEfiAcpiSupportProtocolGuid, (VOID **) &AcpiSupport, 0);
  ASSERT_EFI_ERROR (Status);

  TableVersion = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  mFirstNotify = TRUE;

  //
  // To avoid compiler warning of "C4701: potentially uninitialized local variable 'PlatformCpuInfo' used"
  //
  PlatformCpuInfo.CpuVersion.FullCpuId = 0;

  //
  // Get Platform CPU Info HOB
  //
  PlatformCpuInfoPtr = NULL;

  ZeroMem (&PlatformCpuInfo, sizeof(EFI_PLATFORM_CPU_INFO));
  VariableSize = sizeof(EFI_PLATFORM_CPU_INFO);

  Status = gRT->GetVariable(
                  EfiPlatformCpuInfoVariable,
                  &gEfiBxtVariableGuid,
                  NULL,
                  &VariableSize,
                  PlatformCpuInfoPtr
                  );

  if (EFI_ERROR(Status)) {
    GuidHob.Raw = GetHobList ();

    if (GuidHob.Raw != NULL) {
      if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformCpuInfoGuid, GuidHob.Raw)) != NULL) {
        PlatformCpuInfoPtr = GET_GUID_HOB_DATA (GuidHob.Guid);
      }
    }
  }

  if ((PlatformCpuInfoPtr != NULL)) {
    CopyMem(&PlatformCpuInfo, PlatformCpuInfoPtr, sizeof(EFI_PLATFORM_CPU_INFO));
  }

  //
  // Update the ACPI parameter blocks
  //
  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SetupVarBuffer, &VariableSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &VariableSize,
                  &SetupVarBuffer
                  ); 
//[-end-160803-IB07220122-modify]//
  ASSERT_EFI_ERROR (Status);
  
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SetupVarBuffer, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Get setup variable default value failed! \n"));
      return;  
    }
  }
#if (VP_BIOS_ENABLE == 0)

//[-start-160413-IB03090426-add]//
  //
  // Update the DBGP Table
  //
  if (GetBxtSeries() == BxtP) { 
    Status = UpdateDbgpTable(AcpiSupport);
  }
//[-end-160413-IB03090426-add]//

  //
  // Update the DBG2 Table
  //
  Status = UpdateDbg2Table(AcpiSupport);

  if (Status == EFI_DEVICE_ERROR) {
    DEBUG((EFI_D_INFO, "UART2 Device Not Found Or OsDbgEnable is Disabled  - DBG2 UART2 Base Address Not Updated\n"));
  } else if (Status == EFI_NOT_FOUND) {
    DEBUG((EFI_D_ERROR, "DBG2 ACPI Table Not Found - Table Not Updated\n"));
  }

  //
  // Update the DMAR Table
  //
//[-start-160413-IB03090426-modify]//
  UpdateDmarOnReadyToBoot (SetupVarBuffer.VTdEnable);
//[-end-160413-IB03090426-modify]//

#endif

  //
  // Publish ACPI 1.0 or 2.0 Tables
  //
  Status = AcpiSupport->PublishTables (
                          AcpiSupport,
                          TableVersion
                          );
  ASSERT_EFI_ERROR (Status);

  //
  // S3 script save
  //
  Status = gBS->LocateProtocol (&gEfiAcpiS3SaveProtocolGuid, NULL, (VOID **) &AcpiS3Save);
  if (!EFI_ERROR (Status)) {
    AcpiS3Save->S3Save (AcpiS3Save, NULL);
  }

  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  (VOID **) &GlobalNvsArea
                  );

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Error loading GlobalNvsAreaProtocol - P2SB and eMMC Base Addresses Not Updated\n"));
    return;
  }

  //
  // Update P2SB Base Address in Global NVS
  //
  PciMmBase = MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_SC,
                PCI_DEVICE_NUMBER_P2SB,
                PCI_FUNCTION_NUMBER_P2SB,
                0
                );

//[-start-160815-IB03090432-modify]//
  BaseAddress = MmioRead32 (PciMmBase + R_PCH_P2SB_SBREG_BAR) & B_PCH_P2SB_SBREG_RBA;
//[-end-160815-IB03090432-modify]//

  GlobalNvsArea->Area->P2SBBaseAddress = BaseAddress;

  //
  // Update eMMC Base Address in Global NVS
  //
  PciMmBase = MmPciAddress (
                0,
                DEFAULT_PCI_BUS_NUMBER_SC,
                PCI_DEVICE_NUMBER_SCC_EMMC,
                PCI_FUNCTION_NUMBER_SCC_FUNC0,
                0
                );

  BaseAddress = MmioRead32 (PciMmBase + R_SCC_BAR) & B_SCC_BAR_BA;

  GlobalNvsArea->Area->eMMCAddr = BaseAddress;

  //
  // Install WatchDog Action Table
  //
  DEBUG ((DEBUG_INFO,"Publish WDAT...\n"));
  Status = PublishWatchDogActionTable ();
  ASSERT_EFI_ERROR (Status);
  PrintGnvs(GlobalNvsArea);
}
//[-end-160216-IB03090424-modify]//
//[-start-161027-IB07400806-add]//
STATIC
VOID
EFIAPI
AutoThermalReportingCheck (
  VOID
  )
{
  EFI_STATUS                        Status;
  CPU_GLOBAL_NVS_AREA_PROTOCOL      *CpuGlobalNvsAreaProtocol;
  
  DEBUG ((EFI_D_ERROR, "AutoThermalReportingCheck!\n"));
  Status = gBS->LocateProtocol (&gCpuGlobalNvsAreaProtocolGuid, NULL, (VOID **) &CpuGlobalNvsAreaProtocol);
  DEBUG ((EFI_D_ERROR, "Status = %r, CpuGlobalNvsArea = %x\n", Status, CpuGlobalNvsAreaProtocol->Area));
  ASSERT_EFI_ERROR (Status);
  
  if (!EFI_ERROR (Status) && mSystemConfiguration->AutoThermalReporting) {
    mGlobalNvsArea.Area->CriticalThermalTripPoint = CpuGlobalNvsAreaProtocol->Area->AutoCriticalTripPoint;
    mGlobalNvsArea.Area->PassiveTc1Value          = 1;
    mGlobalNvsArea.Area->PassiveTc2Value          = 5;
    mGlobalNvsArea.Area->PassiveTspValue          = 10;
    mGlobalNvsArea.Area->PassiveThermalTripPoint  = CpuGlobalNvsAreaProtocol->Area->AutoPassiveTripPoint;
    mGlobalNvsArea.Area->ActiveTripPoint          = CpuGlobalNvsAreaProtocol->Area->AutoActiveTripPoint;
  }
}
//[-end-161027-IB07400806-add]//
/**
  Ready to Boot Event to uninstall RTD3 SSDT ACPI Table

  @retval EFI_SUCCESS             Operation completed successfully.
  @retval other                   Some error occurred when executing this function.

**/
VOID
EFIAPI
OnReadyToBootUninstallRtd3SSDTAcpiTable(
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS                    Status;
  UINTN                         Count = 0;
  UINTN                         Handle;
  EFI_ACPI_DESCRIPTION_HEADER   *Table;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport = NULL;

  //
  // Locate ACPI Support protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, (VOID **) &AcpiSupport);
  ASSERT_EFI_ERROR (Status);

  if (!EFI_ERROR(Status)) {
    do {
      Version = 0;
      Status = AcpiSupport->GetAcpiTable (AcpiSupport, Count, (VOID **) &Table, &Version, &Handle);
      if (EFI_ERROR(Status)) {
        break;
      }
      //
      // Check if this is a DPTF SSDT table. If so, uninstall the table
      //
      if (Table->Signature == EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE &&
          Table->OemTableId == SIGNATURE_64('R','V','P','R','t','d','3',0x0)) {
//[-start-160216-IB03090424-modify]//
        DEBUG((DEBUG_INFO, "RTD3 SSDT Table found.Uninstalling it\n"));
//[-end-160216-IB03090424-modify]//
        Status = AcpiSupport->SetAcpiTable (AcpiSupport, NULL, TRUE, Version, &Handle);
        ASSERT_EFI_ERROR(Status);
        Status = AcpiSupport->PublishTables(
                                AcpiSupport,
                                EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0
                                );
        ASSERT_EFI_ERROR(Status);
        break;
      }
      Count++;
    } while (1);
  }
  gBS->CloseEvent(Event);
}
//[-end-151204-IB02950555-modify]//

/*++

Routine Description:

Entry point for Acpi platform driver.

Arguments:

ImageHandle  -  A handle for the image that is initializing this driver.
SystemTable  -  A pointer to the EFI system table.

Returns:

EFI_SUCCESS           -  Driver initialized successfully.
EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
//[-start-151204-IB02950555-modify]//
//[-start-160216-IB03090424-modify]//
EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS                    Status;
  EFI_STATUS                    AcpiStatus;
  EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiTableProtocol;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol;
  INTN                          Instance;
  EFI_ACPI_COMMON_HEADER        *CurrentTable;
  UINTN                         TableHandle;
  UINT32                        FvStatus;
  UINT32                        Size;
  EFI_EVENT                     Event;
  EFI_ACPI_TABLE_VERSION        TableVersion;
  UINTN                         VarSize;
  EFI_HANDLE                    Handle;
  EFI_PEI_HOB_POINTERS          GuidHob;
  UINTN                         Pages;
  EFI_PHYSICAL_ADDRESS          Address;
  UINTN                         McD2BaseAddress;
//[-start-160509-IB03090427-add]//
  UINTN                         Data32;
//[-end-160509-IB03090427-add]//
//[-start-160413-IB03090426-add]//
  VOID*                 		    Buffer;
//[-end-160413-IB03090426-add]//
//[-start-160803-IB07220122-add]//
  UINT32                        VariableAttributes;
//[-end-160803-IB07220122-add]//
//[-start-160923-IB07400789-add]//
  UINT8                         CmosData;
//[-end-160923-IB07400789-add]//
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
  UINT64                        Touud;
#endif
//[-end-170119-IB07401072-add]//

  Address = 0xffffffff;

  mFirstNotify      = FALSE;

  TableVersion      = EFI_ACPI_TABLE_VERSION_2_0;
  Instance          = 0;
  CurrentTable      = NULL;
  TableHandle       = 0;
  McD2BaseAddress   = MmPciBase (0, 2, 0);

  //
  // Update HOB variable for PCI resource information
  // Get the HOB list.  If it is not present, then ASSERT.
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
      mPlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  //
  // Search for the Memory Configuration GUID HOB.  If it is not present, then
  // there's nothing we can do. It may not exist on the update path.
  //

  VarSize = PcdGet32 (PcdSetupConfigSize);
  mSystemConfiguration = (CHIPSET_CONFIGURATION *)AllocatePool (VarSize);
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(mSystemConfiguration, &VarSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &VarSize,
                  mSystemConfiguration
                  ); 
//[-end-160803-IB07220122-modify]//
  ASSERT_EFI_ERROR (Status);
  
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (mSystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      return Status;  
    }
  }

  //
  // Locate the AcpiTable protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID**) &AcpiTableProtocol);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return EFI_ABORTED;
  }


  //
  // Find the AcpiSupport protocol
  //
  Status = LocateSupportProtocol (&gEfiAcpiSupportProtocolGuid, (VOID **) &AcpiSupport, 0);
  ASSERT_EFI_ERROR (Status);

  //
  // Locate the firmware volume protocol
  //
  Status = LocateSupportProtocol (&gEfiFirmwareVolume2ProtocolGuid, (VOID **) &FwVol, 1);
  ASSERT_EFI_ERROR (Status);

//[-start-160509-IB03090427-add]//
  //
  // Locate the MP services protocol
  //
  //
  // Find the MP Protocol. This is an MP platform, so MP protocol must be
  // there.
  //
  Status = gBS->LocateProtocol (
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID **) &mMpService
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
//[-end-160509-IB03090427-add]//
  
  //
  // Allocate and initialize the NVS area for SMM and ASL communication.
  Pages = EFI_SIZE_TO_PAGES ( sizeof (EFI_GLOBAL_NVS_AREA) );
  Status  = gBS->AllocatePages (
                   AllocateMaxAddress,
                   EfiACPIMemoryNVS,
                   Pages,
                   &Address
                   );
  ASSERT_EFI_ERROR (Status);
  mGlobalNvsArea.Area = (EFI_GLOBAL_NVS_AREA*) (UINTN)Address;
  //
  // Allocate and initialize the NVS area for SMM and ASL communication.
  //
  gBS->SetMem (mGlobalNvsArea.Area, sizeof (EFI_GLOBAL_NVS_AREA), 0);
  DEBUG ((EFI_D_ERROR, "mGlobalNvsArea.Area is at 0x%X\n", mGlobalNvsArea.Area));
  
//[-start-160509-IB03090427-add]//
  //
  // Determine the number of processors
  //
  mMpService->GetNumberOfProcessors (
               mMpService,
               &mMaximumNumberOfCpus,
               &mNumberOfEnabledCpus
               );
			   
  ASSERT (mMaximumNumberOfCpus <= MAX_CPU_NUM && mNumberOfEnabledCpus >= 1);
  //
  // Dual Core Override
  //  
//[-start-161111-IB07400811-modify]//
//  if (mMaximumNumberOfCpus == DUAL_CORE_CONFIG) {
    DEBUG ((DEBUG_INFO, "Dual Core Config - Update Setup Variable\n"));
    VarSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160803-IB07220122-modify]//
//     Status = GetSystemConfigData(mSystemConfiguration, &VarSize);
    Status = gRT->GetVariable (
                    SETUP_VARIABLE_NAME,
                    &gSystemConfigurationGuid,
                    &VariableAttributes,
                    &VarSize,
                    mSystemConfiguration
                    ); 
//[-end-160803-IB07220122-modify]//
    ASSERT_EFI_ERROR (Status);
	
    if (EFI_ERROR (Status)) {
      Status = GetChipsetSetupVariableDxe (mSystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
      if (EFI_ERROR (Status)) {
        return Status;  
      }
    }
    //
    // Update Core Number if Max Core number is changed
    //
    if (mSystemConfiguration->NumOfProcessors != PcdGet8 (PcdMaxCpuCoreNumber)) {
      mSystemConfiguration->NumOfProcessors = (UINT8) PcdGet8 (PcdMaxCpuCoreNumber);
//[-start-160803-IB07220122-modify]//
//     Status = SetSystemConfigData(mSystemConfiguration, VarSize);
      Status = gRT->SetVariable (
                      SETUP_VARIABLE_NAME,
                      &gSystemConfigurationGuid,
                      VariableAttributes,
                      VarSize,
                      mSystemConfiguration
                      );
//[-end-160803-IB07220122-modify]//
      ASSERT_EFI_ERROR (Status);
    }
//  }
//[-end-161111-IB07400811-modify]//
//[-end-160509-IB03090427-add]//
  
  //
  // Update Global NVS area for ASL and SMM init code to use
  //
  if (GetBxtSeries() == BxtP) {
    mGlobalNvsArea.Area->NumberOfBatteries          = 1;
    mGlobalNvsArea.Area->BatteryCapacity0           = 100;
//[-start-160718-IB06720413-add]//
    mGlobalNvsArea.Area->Mmio32Base                 = (MmioRead32 ((UINTN)PcdGet64 (PcdPciExpressBaseAddress) + 0xBC) & 0xFFF00000);;
    mGlobalNvsArea.Area->Mmio32Length               = ACPI_MMIO_BASE_ADDRESS - mGlobalNvsArea.Area->Mmio32Base;
    mGlobalNvsArea.Area->CsmEnable                  = 0;
//[-start-170227-IB15550024-add]//
    if (FeaturePcdGet (PcdH2OCsmSupported)) {
      if (mSystemConfiguration->BootType != EFI_BOOT_TYPE) {
        mGlobalNvsArea.Area->CsmEnable              = 1;
      }
    }
//[-end-170227-IB15550024-add]//
//[-end-160718-IB06720413-add]//

    ///
    /// Initialize IGD state by checking if IGD Device 2 Function 0 is enabled in the chipset
    ///
    if (MmioRead16 (McD2BaseAddress) != 0xFFFF) {
      mGlobalNvsArea.Area->IgdState = 1;
    } else {
      mGlobalNvsArea.Area->IgdState = 0;
    }

    //
    // RTD3 Settings
    //
    mGlobalNvsArea.Area->Rtd3Support                = mSystemConfiguration->Rtd3Support;
    /*
    mGlobalNvsArea.Area->Rtd3P0dl                   = mSystemConfiguration->Rtd3P0dl;
    mGlobalNvsArea.Area->Rtd3P3dl                   = mSystemConfiguration->Rtd3P3dl;
    mGlobalNvsArea.Area->RTD3AudioDelay             = mSystemConfiguration->RTD3AudioDelay;
    mGlobalNvsArea.Area->RTD3SensorHub              = mSystemConfiguration->RTD3SensorHub;
    mGlobalNvsArea.Area->RTD3TouchPanelDelay        = mSystemConfiguration->RTD3TouchPanelDelay;
    mGlobalNvsArea.Area->RTD3TouchPadDelay          = mSystemConfiguration->RTD3TouchPadDelay;
    mGlobalNvsArea.Area->RTD3I2C0ControllerPS0Delay = mSystemConfiguration->RTD3I2C0ControllerPS0Delay;
    mGlobalNvsArea.Area->RTD3I2C1ControllerPS0Delay = mSystemConfiguration->RTD3I2C1ControllerPS0Delay;
    mGlobalNvsArea.Area->VRStaggeringDelay          = mSystemConfiguration->VRStaggeringDelay;
    mGlobalNvsArea.Area->VRRampUpDelay              = mSystemConfiguration->VRRampUpDelay;
    mGlobalNvsArea.Area->PstateCapping              = mSystemConfiguration->PstateCapping;
    mGlobalNvsArea.Area->UsbPowerResourceTest       = (mSystemConfiguration->RTD3UsbPt2 << 4)
                                                      | mSystemConfiguration->RTD3UsbPt1;
    mGlobalNvsArea.Area->RTD3I2C0SensorHub          = mSystemConfiguration->RTD3I2C0SensorHub;
    mGlobalNvsArea.Area->SensorStandby              = mSystemConfiguration->SensorStandby;
    mGlobalNvsArea.Area->RTD3Config0                = (mSystemConfiguration->RTD3SataPort2 << 5)
                                                      | (mSystemConfiguration->RTD3SataPort1 << 4)
                                                      | (mSystemConfiguration->RTD3SataPort3 << 3)
                                                      | (mSystemConfiguration->RTD3Camera << 1)
                                                      | mSystemConfiguration->RTD3ZPODD;
    */

    mGlobalNvsArea.Area->RTD3Config0                  = mSystemConfiguration->RTD3ZPODD;
    mGlobalNvsArea.Area->EnableModernStandby          = mSystemConfiguration->ConsolidatedPR;

    mGlobalNvsArea.Area->SelectBtDevice               = mSystemConfiguration->SelectBtDevice;
    mGlobalNvsArea.Area->ScHdAudioIoBufferOwnership   = mSystemConfiguration->ScHdAudioIoBufferOwnership;
    mGlobalNvsArea.Area->XdciEnable                   = mSystemConfiguration->ScUsbOtg;

    mGlobalNvsArea.Area->PciDelayOptimizationEcr      = mSystemConfiguration->PciDelayOptimizationEcr;
    mGlobalNvsArea.Area->PMCBase                      = PcdGet32(PcdPmcIpc1BaseAddress0);
    mGlobalNvsArea.Area->PciePortRtd3BitMap           = 0;
	
//[-start-160803-IB07400768-add]//
// #if BXTI_PF_ENABLE
    // LPSS I2C0 Speed                   
    if(mSystemConfiguration->I2C0Speed == 0) {
       mGlobalNvsArea.Area->I2C0Speed = 100000;
    } else if(mSystemConfiguration->I2C0Speed == 1) { 
       mGlobalNvsArea.Area->I2C0Speed = 400000;
    } else if(mSystemConfiguration->I2C0Speed == 2) {
       mGlobalNvsArea.Area->I2C0Speed = 1000000;
    } else {
       mGlobalNvsArea.Area->I2C0Speed = 3400000;
    }
    // LPSS I2C1 Speed
    if(mSystemConfiguration->I2C1Speed == 0) {
       mGlobalNvsArea.Area->I2C1Speed = 100000;
    } else if(mSystemConfiguration->I2C1Speed == 1) { 
       mGlobalNvsArea.Area->I2C1Speed = 400000;
    } else if(mSystemConfiguration->I2C1Speed == 2) {
       mGlobalNvsArea.Area->I2C1Speed = 1000000;
    } else {
       mGlobalNvsArea.Area->I2C1Speed = 3400000;
    }
   // LPSS I2C2 Speed                   
    if(mSystemConfiguration->I2C2Speed == 0) {
      mGlobalNvsArea.Area->I2C2Speed = 100000;
    } else if(mSystemConfiguration->I2C2Speed == 1) {
      mGlobalNvsArea.Area->I2C2Speed = 400000;
    } else if(mSystemConfiguration->I2C2Speed == 2) {
      mGlobalNvsArea.Area->I2C2Speed = 1000000;
    } else {
      mGlobalNvsArea.Area->I2C2Speed = 3400000;
    }
    // LPSS I2C3 Speed
    if(mSystemConfiguration->I2C3Speed == 0) {
      mGlobalNvsArea.Area->I2C3Speed = 100000;
    } else if(mSystemConfiguration->I2C3Speed == 1) {
      mGlobalNvsArea.Area->I2C3Speed = 400000;
    } else if(mSystemConfiguration->I2C3Speed == 2) {
      mGlobalNvsArea.Area->I2C3Speed = 1000000;
    } else {
      mGlobalNvsArea.Area->I2C3Speed = 3400000;
    }
    // LPSS I2C4 Speed                   
    if(mSystemConfiguration->I2C4Speed == 0) {
      mGlobalNvsArea.Area->I2C4Speed = 100000;
    } else if(mSystemConfiguration->I2C4Speed == 1) {
      mGlobalNvsArea.Area->I2C4Speed = 400000;
    } else if(mSystemConfiguration->I2C4Speed == 2) { 
      mGlobalNvsArea.Area->I2C4Speed = 1000000;
    } else {
      mGlobalNvsArea.Area->I2C4Speed = 3400000;
    }
    // LPSS I2C5 Speed
    if(mSystemConfiguration->I2C5Speed == 0) {
      mGlobalNvsArea.Area->I2C5Speed = 100000;
    } else if(mSystemConfiguration->I2C5Speed == 1) { 
      mGlobalNvsArea.Area->I2C5Speed = 400000;
    } else if(mSystemConfiguration->I2C5Speed == 2) {
      mGlobalNvsArea.Area->I2C5Speed = 1000000;
    } else {
      mGlobalNvsArea.Area->I2C5Speed = 3400000;
    }    
    // LPSS I2C6 Speed                   
    if(mSystemConfiguration->I2C6Speed == 0){
      mGlobalNvsArea.Area->I2C6Speed = 100000;
    } else if(mSystemConfiguration->I2C6Speed == 1) { 
      mGlobalNvsArea.Area->I2C6Speed = 400000;
    } else if(mSystemConfiguration->I2C6Speed == 2) {
      mGlobalNvsArea.Area->I2C6Speed = 1000000;
    } else {
      mGlobalNvsArea.Area->I2C6Speed = 3400000;
    }
    // LPSS I2C7 Speed
    if(mSystemConfiguration->I2C7Speed == 0) {
      mGlobalNvsArea.Area->I2C7Speed = 100000;
    } else if(mSystemConfiguration->I2C7Speed == 1) {
      mGlobalNvsArea.Area->I2C7Speed = 400000;
    } else if(mSystemConfiguration->I2C7Speed == 2) {
      mGlobalNvsArea.Area->I2C7Speed = 1000000;
    } else {
      mGlobalNvsArea.Area->I2C7Speed = 3400000;
    }
//#endif
//[-end-160803-IB07400768-add]//
    //
    // LPSS devices
    //
    mGlobalNvsArea.Area->I2s343A                      = mSystemConfiguration->I2s343A;
    mGlobalNvsArea.Area->I2s34C1                      = mSystemConfiguration->I2s34C1;
    mGlobalNvsArea.Area->I2cNfc                       = mSystemConfiguration->I2cNfc;
    mGlobalNvsArea.Area->I2cPss                       = mSystemConfiguration->I2cPss;
    mGlobalNvsArea.Area->UartBt                       = mSystemConfiguration->UartBt;
    mGlobalNvsArea.Area->UartGps                      = mSystemConfiguration->UartGps;
    mGlobalNvsArea.Area->Spi1SensorDevice             = mSystemConfiguration->Spi1SensorDevice;
//[-start-160718-IB06720413-add]//
    mGlobalNvsArea.Area->I2cTouchPanel                = mSystemConfiguration->I2cTouchPanel;
    mGlobalNvsArea.Area->I2cTouchPad                  = mSystemConfiguration->I2cTouchPad;
//[-end-160718-IB06720413-add]//
//[-start-160429-IB07400722-add]//
    mGlobalNvsArea.Area->HideUnusedLpssDevices        = mSystemConfiguration->HideUnusedLpssDevices;
//[-end-160429-IB07400722-add]//

    //EPI
    mGlobalNvsArea.Area->EPIEnable = mSystemConfiguration->EPIEnable;

    if(mSystemConfiguration->TypeCEnable == 2) {
      //
      // AUTO
      //
      if (mPlatformInfo->BoardId == BOARD_ID_APL_RVP_1C_LITE) {
        mGlobalNvsArea.Area->TypeCEnable = 1; // Enable TypeC for RVP 1C
      } else {
        mGlobalNvsArea.Area->TypeCEnable = 0; // Disable TypeC for All other RVP.
      }
    } else {
      mGlobalNvsArea.Area->TypeCEnable = mSystemConfiguration->TypeCEnable;
    }
    DEBUG((DEBUG_INFO, "TypeCEnable = %d\n", mGlobalNvsArea.Area->TypeCEnable));
  }
//  } else {
//    mGlobalNvsArea.Area->VirtualKeyboard = mSystemConfiguration->VirtualKbEnable;
//    DEBUG((DEBUG_INFO, "VirtualKeyboard = %d\n", mGlobalNvsArea.Area->VirtualKeyboard));
//
//    mGlobalNvsArea.Area->WifiSel = mSystemConfiguration->WifiSel;
//    DEBUG((DEBUG_INFO, "GlobalNvsArea.Area->WifiSel: [%02x], WiFi selection: 0: SDIO Lightning Peak, 1: SDIO Broadcom; 2: PCIe Lightning Peak\n", mGlobalNvsArea.Area->WifiSel));
//
//    mGlobalNvsArea.Area->NfcSelect = mSystemConfiguration->NfcSelect;
//    DEBUG((DEBUG_INFO, "GlobalNvsArea.Area->NfcSelect: [%02x], NFC deivce select: 0: disabled; 1: NFC (IPT)/secure NFC; 2: NFC;\n", mGlobalNvsArea.Area->NfcSelect));
//
//    mGlobalNvsArea.Area->AudioSel       = mSystemConfiguration->AudioSel;
//    mGlobalNvsArea.Area->PanelSel       = mSystemConfiguration->PanelSel;
//  }
//
//#if (ENBDT_PF_ENABLE == 0)
//  //
//  // Input PMIC stepping to Global NVS area.
//  //
//  mGlobalNvsArea.Area->PmicStepping = PmicStepping();
//  DEBUG ((DEBUG_INFO, "PmicStepping = 0x%x\n", mGlobalNvsArea.Area->PmicStepping));
//#endif
//[-start-160922-IB03090434-modify]//
  //
  // Enable 10sec Power Button Override only if CS is enabled.
  //
  if ((mSystemConfiguration->LowPowerS0Idle == 1) && (mSystemConfiguration->TenSecondPowerButtonEnable == 1)) {
    mGlobalNvsArea.Area->PowerButtonSupport |= BIT0; // Enable 10sec Power Button OVR.
  } else {
    mGlobalNvsArea.Area->PowerButtonSupport &= ~(BIT0); // Disable 10sec Power Button OVR.
  }
//[-start-161018-IB06740518-add]//
  mGlobalNvsArea.Area->PowerButtonSupport |= mSystemConfiguration->SlateIndicator << 3;
  mGlobalNvsArea.Area->PowerButtonSupport |= mSystemConfiguration->DockIndicator << 4;
  mGlobalNvsArea.Area->ConvertibleEnable = mSystemConfiguration->ConvertibleEnable;
  mGlobalNvsArea.Area->DockEnable = mSystemConfiguration->DockEnable;
  mGlobalNvsArea.Area->VirtualGpioButtonSxBitmask  = 0x00;
  mGlobalNvsArea.Area->VirtualGpioButtonSxBitmask |=  mSystemConfiguration->SlateIndicatorSx << 0;
  mGlobalNvsArea.Area->VirtualGpioButtonSxBitmask |=  mSystemConfiguration->DockIndicatorSx << 1;
//[-end-161018-IB06740518-add]//
  if (mSystemConfiguration->IpuEn == 0) {
    //
    // If IPU is disabled, both camera can't be used.
    //
    mGlobalNvsArea.Area->WorldCameraSel = 0;
    mGlobalNvsArea.Area->UserCameraSel  = 0;
  } else {
    mGlobalNvsArea.Area->WorldCameraSel = mSystemConfiguration->WorldCameraSel;
    mGlobalNvsArea.Area->UserCameraSel  = mSystemConfiguration->UserCameraSel;
  }
//[-end-160922-IB03090434-modify]//
//[-start-160718-IB06720413-add]//
  mGlobalNvsArea.Area->CameraRotationAngle      = mSystemConfiguration->CameraRotationAngle;
//[-end-160718-IB06720413-add]//

  // OS Selection [0]:WOS [1]:AOS [2]:Win7
  mGlobalNvsArea.Area->OsSelection                = mSystemConfiguration->OsSelection;
  DEBUG((DEBUG_INFO, "GlobalNvsArea.Area->OsSelection: [%02x] [0:WOS 1:AOS 2:Win7]\n", mGlobalNvsArea.Area->OsSelection));

  mGlobalNvsArea.Area->ApicEnable               = 1;
  mGlobalNvsArea.Area->PowerState               = 0;

//[-start-160509-IB03090427-remove]//
//  mGlobalNvsArea.Area->CriticalThermalTripPoint = mSystemConfiguration->CriticalThermalTripPoint;
//[-end-160509-IB03090427-remove]//

  mGlobalNvsArea.Area->IgdPanelType             = mSystemConfiguration->IgdFlatPanel;
  mGlobalNvsArea.Area->IgdPanelScaling          = mSystemConfiguration->PanelScaling;
  mGlobalNvsArea.Area->IgdSciSmiMode            = 0;
  mGlobalNvsArea.Area->IgdTvFormat              = 0;
  mGlobalNvsArea.Area->IgdTvMinor               = 0;
  mGlobalNvsArea.Area->IgdSscConfig             = 1;
  mGlobalNvsArea.Area->IgdBiaConfig             = mSystemConfiguration->IgdLcdIBia;
  mGlobalNvsArea.Area->IgdBlcConfig             = mSystemConfiguration->IgdLcdIGmchBlc;
  mGlobalNvsArea.Area->IgdDvmtMemSize           = mSystemConfiguration->IgdDvmt50TotalAlloc;
  mGlobalNvsArea.Area->IgdHpllVco               = 0; //Todo:  Need to lookup broxton spec  // SKL: MmioRead8 (mMchBarBase + 0xC0F) & 0x07;


  mGlobalNvsArea.Area->AlsEnable                = mSystemConfiguration->AlsEnable;
  mGlobalNvsArea.Area->BacklightControlSupport  = 2;
  mGlobalNvsArea.Area->BrightnessPercentage     = 100;
  mGlobalNvsArea.Area->LidState = 1;

  mGlobalNvsArea.Area->DeviceId1 = 0x80000100;
  mGlobalNvsArea.Area->DeviceId2 = 0x80000400;
  mGlobalNvsArea.Area->DeviceId3 = 0x80000200;
  mGlobalNvsArea.Area->DeviceId4 = 0x04;
  mGlobalNvsArea.Area->DeviceId5 = 0x05;
  mGlobalNvsArea.Area->NumberOfValidDeviceId = 4;
//[-start-160817-IB03090432-remove]//
//  mGlobalNvsArea.Area->CurrentDeviceList = 0x0F;
//[-end-160817-IB03090432-remove]//

  //
  //Broxton-P(ApolloLake only)
  //
  if (GetBxtSeries() == BxtP) {
//[-start-160509-IB03090427-add]//
    //
    // Legacy thermal zone Active trip point
    //
    mGlobalNvsArea.Area->ActiveTripPoint = mSystemConfiguration->ActiveTripPoint;
//[-end-160509-IB03090427-add]//

//[-start-160317-IB03090425-modify]//
    //
    // Broxton-P(ApolloLake only)
    //
//[-start-160413-IB03090426-remove]//
//    mGlobalNvsArea.Area->EnableDCFG                   = mSystemConfiguration->EnableDCFG;
//[-end-160413-IB03090426-remove]//
    mGlobalNvsArea.Area->DptfProcActiveTemperature    = mSystemConfiguration->DptfProcActiveTemperature;
    mGlobalNvsArea.Area->DptfFanDevice                = mSystemConfiguration->DptfFanDevice;

    //
    // DPTF Policies
    //
    mGlobalNvsArea.Area->EnableActivePolicy           = mSystemConfiguration->EnableActivePolicy;
    mGlobalNvsArea.Area->TrtRevision                  = mSystemConfiguration->TrtRevision;

    //
    // DPTF Participants
    //
    mGlobalNvsArea.Area->EnableSen1Participant        = mSystemConfiguration->EnableSen1Participant;
    mGlobalNvsArea.Area->ActiveThermalTripPointSen1   = mSystemConfiguration->ActiveThermalTripPointSen1;
    mGlobalNvsArea.Area->PassiveThermalTripPointSen1  = mSystemConfiguration->PassiveThermalTripPointSen1;
    mGlobalNvsArea.Area->CriticalThermalTripPointSen1 = mSystemConfiguration->CriticalThermalTripPointSen1;
    mGlobalNvsArea.Area->CriticalThermalTripPointSen1S3 = mSystemConfiguration->CriticalThermalTripPointSen1S3;
    mGlobalNvsArea.Area->HotThermalTripPointSen1      = mSystemConfiguration->HotThermalTripPointSen1;
    mGlobalNvsArea.Area->SensorSamplingPeriodSen1     = mSystemConfiguration->SensorSamplingPeriodSen1;

    mGlobalNvsArea.Area->EnableGen1Participant        = mSystemConfiguration->EnableGen1Participant;
    mGlobalNvsArea.Area->ActiveThermalTripPointGen1   = mSystemConfiguration->ActiveThermalTripPointGen1;
    mGlobalNvsArea.Area->PassiveThermalTripPointGen1  = mSystemConfiguration->PassiveThermalTripPointGen1;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen1 = mSystemConfiguration->CriticalThermalTripPointGen1;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen1S3 = mSystemConfiguration->CriticalThermalTripPointGen1S3;
    mGlobalNvsArea.Area->HotThermalTripPointGen1      = mSystemConfiguration->HotThermalTripPointGen1;
    mGlobalNvsArea.Area->ThermistorSamplingPeriodGen1 = mSystemConfiguration->ThermistorSamplingPeriodGen1;

    mGlobalNvsArea.Area->EnableGen2Participant        = mSystemConfiguration->EnableGen2Participant;
    mGlobalNvsArea.Area->ActiveThermalTripPointGen2   = mSystemConfiguration->ActiveThermalTripPointGen2;
    mGlobalNvsArea.Area->PassiveThermalTripPointGen2  = mSystemConfiguration->PassiveThermalTripPointGen2;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen2 = mSystemConfiguration->CriticalThermalTripPointGen2;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen2S3 = mSystemConfiguration->CriticalThermalTripPointGen2S3;
    mGlobalNvsArea.Area->HotThermalTripPointGen2      = mSystemConfiguration->HotThermalTripPointGen2;
    mGlobalNvsArea.Area->ThermistorSamplingPeriodGen2 = mSystemConfiguration->ThermistorSamplingPeriodGen2;

    mGlobalNvsArea.Area->EnableGen3Participant        = mSystemConfiguration->EnableGen3Participant;
    mGlobalNvsArea.Area->ActiveThermalTripPointGen3   = mSystemConfiguration->ActiveThermalTripPointGen3;
    mGlobalNvsArea.Area->PassiveThermalTripPointGen3  = mSystemConfiguration->PassiveThermalTripPointGen3;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen3 = mSystemConfiguration->CriticalThermalTripPointGen3;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen3S3 = mSystemConfiguration->CriticalThermalTripPointGen3S3;
    mGlobalNvsArea.Area->HotThermalTripPointGen3      = mSystemConfiguration->HotThermalTripPointGen3;
    mGlobalNvsArea.Area->ThermistorSamplingPeriodGen3 = mSystemConfiguration->ThermistorSamplingPeriodGen3;

    mGlobalNvsArea.Area->EnableGen4Participant        = mSystemConfiguration->EnableGen4Participant;
    mGlobalNvsArea.Area->ActiveThermalTripPointGen4   = mSystemConfiguration->ActiveThermalTripPointGen4;
    mGlobalNvsArea.Area->PassiveThermalTripPointGen4  = mSystemConfiguration->PassiveThermalTripPointGen4;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen4 = mSystemConfiguration->CriticalThermalTripPointGen4;
    mGlobalNvsArea.Area->CriticalThermalTripPointGen4S3 = mSystemConfiguration->CriticalThermalTripPointGen4S3;
    mGlobalNvsArea.Area->HotThermalTripPointGen4      = mSystemConfiguration->HotThermalTripPointGen4;
    mGlobalNvsArea.Area->ThermistorSamplingPeriodGen4 = mSystemConfiguration->ThermistorSamplingPeriodGen4;

    mGlobalNvsArea.Area->ActiveThermalTripPointVS1     = mSystemConfiguration->ActiveThermalTripPointVS1;
    mGlobalNvsArea.Area->CriticalThermalTripPointVS1   = mSystemConfiguration->CriticalThermalTripPointVS1;
    mGlobalNvsArea.Area->CriticalThermalTripPointVS1S3 = mSystemConfiguration->CriticalThermalTripPointVS1S3;
    mGlobalNvsArea.Area->HotThermalTripPointVS1        = mSystemConfiguration->HotThermalTripPointVS1;

    mGlobalNvsArea.Area->ActiveThermalTripPointVS2     = mSystemConfiguration->ActiveThermalTripPointVS2;
    mGlobalNvsArea.Area->CriticalThermalTripPointVS2   = mSystemConfiguration->CriticalThermalTripPointVS2;
    mGlobalNvsArea.Area->CriticalThermalTripPointVS2S3 = mSystemConfiguration->CriticalThermalTripPointVS2S3;
    mGlobalNvsArea.Area->HotThermalTripPointVS2        = mSystemConfiguration->HotThermalTripPointVS2;

    mGlobalNvsArea.Area->ActiveThermalTripPointVS3     = mSystemConfiguration->ActiveThermalTripPointVS3;
    mGlobalNvsArea.Area->CriticalThermalTripPointVS3   = mSystemConfiguration->CriticalThermalTripPointVS3;
    mGlobalNvsArea.Area->CriticalThermalTripPointVS3S3 = mSystemConfiguration->CriticalThermalTripPointVS3S3;
    mGlobalNvsArea.Area->HotThermalTripPointVS3        = mSystemConfiguration->HotThermalTripPointVS3;

//[-start-160413-IB03090426-remove]//
//    //
//    // DPTF OEM Design Variables
//    //
//    mGlobalNvsArea.Area->OemDesignVariable0           = mSystemConfiguration->OemDesignVariable0;
//    mGlobalNvsArea.Area->OemDesignVariable1           = mSystemConfiguration->OemDesignVariable1;
//    mGlobalNvsArea.Area->OemDesignVariable2           = mSystemConfiguration->OemDesignVariable2;
//    mGlobalNvsArea.Area->OemDesignVariable3           = mSystemConfiguration->OemDesignVariable3;
//    mGlobalNvsArea.Area->OemDesignVariable4           = mSystemConfiguration->OemDesignVariable4;
//    mGlobalNvsArea.Area->OemDesignVariable5           = mSystemConfiguration->OemDesignVariable5;
//[-end-160413-IB03090426-remove]//
//[-end-160317-IB03090425-modify]//
  }
//  } else {
//    //
//    // Broxton-M only
//    //
//    mGlobalNvsArea.Area->EnableSen0Participant             = mSystemConfiguration->EnableSen0Participant;
//    mGlobalNvsArea.Area->PassiveThermalTripPointSen0       = mSystemConfiguration->PassiveThermalTripPointSen0;
//    mGlobalNvsArea.Area->CriticalThermalTripPointSen0      = mSystemConfiguration->CriticalThermalTripPointSen0;
//    mGlobalNvsArea.Area->CriticalThermalTripPointSen0S3    = mSystemConfiguration->CriticalThermalTripPointSen0S3;
//    mGlobalNvsArea.Area->HotThermalTripPointSen0           = mSystemConfiguration->HotThermalTripPointSen0;
//
//    mGlobalNvsArea.Area->EnableSen3Participant             = mSystemConfiguration->EnableSen3Participant;
//    mGlobalNvsArea.Area->PassiveThermalTripPointSen3       = mSystemConfiguration->PassiveThermalTripPointSen3;
//    mGlobalNvsArea.Area->CriticalThermalTripPointSen3      = mSystemConfiguration->CriticalThermalTripPointSen3;
//    mGlobalNvsArea.Area->CriticalThermalTripPointSen3S3    = mSystemConfiguration->CriticalThermalTripPointSen3S3;
//    mGlobalNvsArea.Area->HotThermalTripPointSen3           = mSystemConfiguration->HotThermalTripPointSen3;
//
//    mGlobalNvsArea.Area->DptfGenericCriticalTemperature0   = mSystemConfiguration->GenericCriticalTemp0;
//    mGlobalNvsArea.Area->DptfGenericPassiveTemperature0    = mSystemConfiguration->GenericPassiveTemp0;
//    mGlobalNvsArea.Area->DptfGenericCriticalTemperature1   = mSystemConfiguration->GenericCriticalTemp1;
//    mGlobalNvsArea.Area->DptfGenericPassiveTemperature1    = mSystemConfiguration->GenericPassiveTemp1;
//    mGlobalNvsArea.Area->DptfGenericCriticalTemperature3   = mSystemConfiguration->GenericCriticalTemp3;
//    mGlobalNvsArea.Area->DptfGenericPassiveTemperature3    = mSystemConfiguration->GenericPassiveTemp3;
//    mGlobalNvsArea.Area->DptfGenericCriticalTemperature4   = mSystemConfiguration->GenericCriticalTemp4;
//    mGlobalNvsArea.Area->DptfGenericPassiveTemperature4    = mSystemConfiguration->GenericPassiveTemp4;
//
//    mGlobalNvsArea.Area->DptfWwanDevice                    = mSystemConfiguration->DptfWwanDevice;
//    mGlobalNvsArea.Area->PassiveThermalTripPointWWAN       = mSystemConfiguration->PassiveThermalTripPointWWAN;
//    mGlobalNvsArea.Area->CriticalThermalTripPointWWANS3    = mSystemConfiguration->CriticalThermalTripPointWWANS3;
//    mGlobalNvsArea.Area->HotThermalTripPointWWAN           = mSystemConfiguration->HotThermalTripPointWWAN;
//    mGlobalNvsArea.Area->CriticalThermalTripPointWWAN      = mSystemConfiguration->CriticalThermalTripPointWWAN;
//
//  }

//[-start-160317-IB03090425-modify]//
  //
  // Common code for Broxton-M and Broxton-P(ApolloLake)
  //
//[-start-160413-IB03090426-add]//
  mGlobalNvsArea.Area->EnableDCFG                         = mSystemConfiguration->EnableDCFG;

  //
  // DPTF OEM Design Variables
  //
  mGlobalNvsArea.Area->OemDesignVariable0                 = mSystemConfiguration->OemDesignVariable0;
  mGlobalNvsArea.Area->OemDesignVariable1                 = mSystemConfiguration->OemDesignVariable1;
  mGlobalNvsArea.Area->OemDesignVariable2                 = mSystemConfiguration->OemDesignVariable2;
  mGlobalNvsArea.Area->OemDesignVariable3                 = mSystemConfiguration->OemDesignVariable3;
  mGlobalNvsArea.Area->OemDesignVariable4                 = mSystemConfiguration->OemDesignVariable4;
  mGlobalNvsArea.Area->OemDesignVariable5                 = mSystemConfiguration->OemDesignVariable5;
//[-end-160413-IB03090426-add]//

  mGlobalNvsArea.Area->CriticalThermalTripPoint           = mSystemConfiguration->CriticalThermalTripPoint;
  mGlobalNvsArea.Area->PassiveThermalTripPoint            = mSystemConfiguration->PassiveThermalTripPoint;
  mGlobalNvsArea.Area->PassiveTc1Value                    = mSystemConfiguration->PassiveTc1Value;
  mGlobalNvsArea.Area->PassiveTc2Value                    = mSystemConfiguration->PassiveTc2Value;
  mGlobalNvsArea.Area->PassiveTspValue                    = mSystemConfiguration->PassiveTspValue;
  mGlobalNvsArea.Area->DptfEnable                         = mSystemConfiguration->EnableDptf;
  mGlobalNvsArea.Area->DptfProcessor                      = mSystemConfiguration->DptfProcessor;
  mGlobalNvsArea.Area->DptfProcCriticalTemperature        = mSystemConfiguration->DptfProcCriticalTemperature;
  mGlobalNvsArea.Area->DptfProcPassiveTemperature         = mSystemConfiguration->DptfProcPassiveTemperature;
  mGlobalNvsArea.Area->DptfProcCriticalTemperatureS3      = mSystemConfiguration->DptfProcCriticalTemperatureS3;
  mGlobalNvsArea.Area->DptfProcHotThermalTripPoint        = mSystemConfiguration->DptfProcHotThermalTripPoint;
  mGlobalNvsArea.Area->ThermalSamplingPeriodTCPU          = mSystemConfiguration->ThermalSamplingPeriodTCPU;
  mGlobalNvsArea.Area->EnablePowerParticipant             = mSystemConfiguration->EnablePowerParticipant;
  mGlobalNvsArea.Area->PowerParticipantPollingRate        = mSystemConfiguration->PowerParticipantPollingRate;
  mGlobalNvsArea.Area->DptfChargerDevice                  = mSystemConfiguration->DptfChargerDevice;
  mGlobalNvsArea.Area->DptfDisplayDevice                  = mSystemConfiguration->DptfDisplayDevice;
//[-start-160413-IB03090426-add]//
  mGlobalNvsArea.Area->DisplayHighLimit                   = mSystemConfiguration->DisplayHighLimit;
  mGlobalNvsArea.Area->DisplayLowLimit                    = mSystemConfiguration->DisplayLowLimit;
//[-end-160413-IB03090426-add]//

  mGlobalNvsArea.Area->EnableVS1Participant               = mSystemConfiguration->EnableVS1Participant;
  mGlobalNvsArea.Area->PassiveThermalTripPointVS1         = mSystemConfiguration->PassiveThermalTripPointVS1;

  mGlobalNvsArea.Area->EnableVS2Participant               = mSystemConfiguration->EnableVS2Participant;
  mGlobalNvsArea.Area->PassiveThermalTripPointVS2         = mSystemConfiguration->PassiveThermalTripPointVS2;

  mGlobalNvsArea.Area->EnableVS3Participant               = mSystemConfiguration->EnableVS3Participant;
  mGlobalNvsArea.Area->PassiveThermalTripPointVS3         = mSystemConfiguration->PassiveThermalTripPointVS3;

//[-start-161027-IB07400806-add]//
  AutoThermalReportingCheck();
//[-end-161027-IB07400806-add]//

  //
  // DPTF Policies
  //
  mGlobalNvsArea.Area->EnablePassivePolicy                = mSystemConfiguration->EnablePassivePolicy;
  mGlobalNvsArea.Area->EnableAPPolicy                     = mSystemConfiguration->EnableAPPolicy;
  mGlobalNvsArea.Area->EnableCriticalPolicy               = mSystemConfiguration->EnableCriticalPolicy;
  mGlobalNvsArea.Area->EnablePowerBossPolicy              = mSystemConfiguration->EnablePowerBossPolicy;
  mGlobalNvsArea.Area->EnableVSPolicy                     = mSystemConfiguration->EnableVSPolicy;

  //
  // Dump DPTF Settings
  //
  DEBUG((DEBUG_INFO, "Dumping DPTF settings in Global NVS init...\n"));
  DEBUG((DEBUG_INFO, "DPTFEnabled = %d\n", mGlobalNvsArea.Area->DptfEnable));
  DEBUG((DEBUG_INFO, "DptfProcessor = %d\n", mGlobalNvsArea.Area->DptfProcessor));
//[-end-160317-IB03090425-modify]//

  if (GetBxtSeries() == Bxt1) {
    mGlobalNvsArea.Area->WiGigEnable                      = mSystemConfiguration->WiGigEnable;
    mGlobalNvsArea.Area->WiGigSPLCPwrLimit                = mSystemConfiguration->WiGigSPLCPwrLimit;
    mGlobalNvsArea.Area->WiGigSPLCTimeWindow              = mSystemConfiguration->WiGigSPLCTimeWindow;
    mGlobalNvsArea.Area->RfemSPLCPwrLimit                 = mSystemConfiguration->RfemSPLCPwrLimit;
    mGlobalNvsArea.Area->RfemSPLCTimeWindow               = mSystemConfiguration->RfemSPLCTimeWindow;

    mGlobalNvsArea.Area->PsmEnable                        = mSystemConfiguration->PsmEnable;
    mGlobalNvsArea.Area->PsmSPLC0DomainType               = mSystemConfiguration->PsmSPLC0DomainType;
    mGlobalNvsArea.Area->PsmSPLC0PwrLimit                 = mSystemConfiguration->PsmSPLC0PwrLimit;
    mGlobalNvsArea.Area->PsmSPLC0TimeWindow               = mSystemConfiguration->PsmSPLC0TimeWindow;
    mGlobalNvsArea.Area->PsmSPLC1DomainType               = mSystemConfiguration->PsmSPLC1DomainType;
    mGlobalNvsArea.Area->PsmSPLC1PwrLimit                 = mSystemConfiguration->PsmSPLC1PwrLimit;
    mGlobalNvsArea.Area->PsmSPLC1TimeWindow               = mSystemConfiguration->PsmSPLC1TimeWindow;

    mGlobalNvsArea.Area->PsmDPLC0DomainType               = mSystemConfiguration->PsmDPLC0DomainType;
    mGlobalNvsArea.Area->PsmDPLC0DomainPerference         = mSystemConfiguration->PsmDPLC0DomainPerference;
    mGlobalNvsArea.Area->PsmDPLC0PowerLimitIndex          = mSystemConfiguration->PsmDPLC0PowerLimitIndex;
    mGlobalNvsArea.Area->PsmDPLC0PwrLimit                 = mSystemConfiguration->PsmDPLC0PwrLimit;
    mGlobalNvsArea.Area->PsmDPLC0TimeWindow               = mSystemConfiguration->PsmDPLC0TimeWindow;

    mGlobalNvsArea.Area->PsmDPLC1DomainType               = mSystemConfiguration->PsmDPLC1DomainType;
    mGlobalNvsArea.Area->PsmDPLC1DomainPerference         = mSystemConfiguration->PsmDPLC1DomainPerference;
    mGlobalNvsArea.Area->PsmDPLC1PowerLimitIndex          = mSystemConfiguration->PsmDPLC1PowerLimitIndex;
    mGlobalNvsArea.Area->PsmDPLC1PwrLimit                 = mSystemConfiguration->PsmDPLC1PwrLimit;
    mGlobalNvsArea.Area->PsmDPLC1TimeWindow               = mSystemConfiguration->PsmDPLC1TimeWindow;
  }

//[-start-160422-IB03090426-add]//
//[-start-161022-IB07400803-modify]//
//#if BXTI_PF_ENABLE
  if (IsIOTGBoardIds()) {
    //
    // Doc#570618, Rev 1.4, Section 6, Inte PSD support
    //
    AcpiHrotDxeEntryPoint();
  }
//#endif
//[-end-161022-IB07400803-modify]//
//[-end-160422-IB03090426-add]//

//[-start-160317-IB03090425-modify]//
  //
  // Dump Settings.
  //
  DEBUG((DEBUG_INFO, "OFFSET_OF BacklightControlSupport = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,BacklightControlSupport)));
  DEBUG((DEBUG_INFO, "OFFSET_OF OsSelection = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,OsSelection)));
  DEBUG((DEBUG_INFO, "OFFSET_OF WorldCameraSel = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,WorldCameraSel)));
  DEBUG((DEBUG_INFO, "OFFSET_OF I2C61Len = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,I2C61Len)));
  DEBUG((DEBUG_INFO, "OFFSET_OF PepList = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,PepList)));
  DEBUG((DEBUG_INFO, "OFFSET_OF PlatformFlavor = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,PlatformFlavor)));
  DEBUG((DEBUG_INFO, "OFFSET_OF OsSelection = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,OsSelection)));
  DEBUG((DEBUG_INFO, "OFFSET_OF WorldCameraSel = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,WorldCameraSel)));
  DEBUG((DEBUG_INFO, "OFFSET_OF I2C61Len = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,I2C61Len)));
  DEBUG((DEBUG_INFO, "OFFSET_OF PepList = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,PepList)));
  DEBUG((DEBUG_INFO, "OFFSET_OF SPI1Addr = %d\n", OFFSET_OF(EFI_GLOBAL_NVS_AREA,SPI1Addr)));
//[-end-160317-IB03090425-modify]//

  //
  // PCIe Native Mode
  //
  mGlobalNvsArea.Area->NativePCIESupport = mSystemConfiguration->PciExpNative;
  //
  // Low Power S0 Idle - Enabled/Disabled
  //
  mGlobalNvsArea.Area->LowPowerS0Idle = mSystemConfiguration->LowPowerS0Idle;
//[-start-160906-IB03090434-add]//
  mGlobalNvsArea.Area->CSNotifyEC = mSystemConfiguration->CSNotifyEC;
  mGlobalNvsArea.Area->ECLowPowerMode = mSystemConfiguration->EcLowPowerMode;
//[-start-161018-IB06740518-add]//
  mGlobalNvsArea.Area->CSDebugLightEC = mSystemConfiguration->CSDebugLightEC;
//[-end-161018-IB06740518-add]//
//[-end-160906-IB03090434-add]//
//[-start-160509-IB03090427-modify]//
  //
  // Kernel Debugger Enable/Disable
  // 
  mGlobalNvsArea.Area->OsDbgEnable = mSystemConfiguration->OsDbgEnable;
//[-end-160509-IB03090427-modify]//
//[-start-160817-IB03090432-add]//
  mGlobalNvsArea.Area->SdCardEnable = mSystemConfiguration->SccSdcardEnabled;
  mGlobalNvsArea.Area->SdioEnable = mSystemConfiguration->SccSdioEnabled;
  mGlobalNvsArea.Area->EmmcEnable = mSystemConfiguration->ScceMMCEnabled;
  mGlobalNvsArea.Area->EmmcHostMaxSpeed = mSystemConfiguration->ScceMMCHostMaxSpeed;
//[-end-160817-IB03090432-add]//

  //
  //
  // Platform Flavor
  //
  mGlobalNvsArea.Area->PlatformFlavor = mPlatformInfo->PlatformFlavor;

  // Update the Platform id
  mGlobalNvsArea.Area->BoardId = mPlatformInfo->BoardId;

  // Update the Platform id
  mGlobalNvsArea.Area->BoardRev =  mPlatformInfo->BoardRev;

  mGlobalNvsArea.Area->XhciMode = mSystemConfiguration->ScUsb30Mode;
  //
  // Override invalid Pre-Boot Driver and XhciMode combination
  //
//[-start-160413-IB03090426-remove]//
//  if ((mSystemConfiguration->UsbXhciSupport == 0) && (mSystemConfiguration->ScUsb30Mode == 3)) {
//    mGlobalNvsArea.Area->XhciMode = 2;
//  }
//  if ((mSystemConfiguration->UsbXhciSupport == 1) && (mSystemConfiguration->ScUsb30Mode == 2)) {
//    mGlobalNvsArea.Area->XhciMode = 3;
//  }
//[-end-160413-IB03090426-remove]//
  // PMIC is enabled by default. When it is disabled, we will not expose it in DSDT.
  mGlobalNvsArea.Area->PmicEnable                   = mSystemConfiguration->PmicEnable;
  mGlobalNvsArea.Area->BatteryChargingSolution      = mSystemConfiguration->BatteryChargingSolution;
//[-start-160510-IB07400726-modify]//
  if (mSystemConfiguration->IpuEn == 0) {
    mGlobalNvsArea.Area->IpuAcpiMode                = 0; //Disable Ipu ACPI device if IPU is disabled
  } else {
//[-start-160509-IB03090427-modify]//
    //
    // If I-unit is fused off set the IPU ACPI mode to disabled.
    //
    Data32 = MmioRead32(MmPciBase(SA_MC_BUS, SA_MC_DEV, SA_MC_FUN) + R_SA_MC_CAPID0_B);
    if (Data32 & BIT31) {
      DEBUG ((DEBUG_INFO, "IPU Fused off\n"));
      mGlobalNvsArea.Area->IpuAcpiMode = 2;
//[-start-160623-IB07400747-remove]//
////[-start-160527-IB06720411-add]//
//  } else if (mSystemConfiguration->IpuEn == 0) {
//    DEBUG ((DEBUG_INFO, "IPU function disabled\n"));
//    mGlobalNvsArea.Area->IpuAcpiMode = 0; //IGFX disabled
////[-end-160527-IB06720411-add]//
//[-end-160623-IB07400747-remove]//
    } else {
      mGlobalNvsArea.Area->IpuAcpiMode = mSystemConfiguration->IpuAcpiMode;
    }
//[-end-160509-IB03090427-modify]//
  }
//[-end-160510-IB07400726-modify]//
//[-start-160406-IB07400715-modify]//
//#if !(BXTI_PF_ENABLE)
//#if (VP_BIOS_ENABLE == 0)
//  mGlobalNvsArea.Area->ECAvailability               = 1;
//#else
//  mGlobalNvsArea.Area->ECAvailability               = 0; 
//#endif
//#else
  mGlobalNvsArea.Area->ECAvailability               = mPlatformInfo->ECPresent;
//#endif
//[-end-160406-IB07400715-modify]//
//[-start-160704-IB08450349-add]//
//[-start-160711-IB07400754-modify]//
//[-start-160923-IB07400789-remove]//
//  if ( IoRead8 (0x64) != 0xFF) { // Enable PS2 Keyboard/Mouse if not legacy free.
//    mGlobalNvsArea.Area->Ps2KbMsEnable   = 1;
//    mGlobalNvsArea.Area->Ps2MouseEnable = 1;
//  }
//[-end-160923-IB07400789-remove]//
//[-end-160711-IB07400754-modify]//
//[-end-160704-IB08450349-add]//
  
//[-start-160923-IB07400789-add]//
  if (IoRead8 (0x64) == 0xFF) { // No KBC exist
    CmosData = ReadExtCmos8 (R_XCMOS_INDEX, R_XCMOS_DATA, CmosChipsetFeature); 
    if (CmosData & B_CMOS_WIN7_VIRTUAL_KBC_SUPPORT) { // Win7 Virtual KBC support
      mGlobalNvsArea.Area->LegacyFreeSupport = 1;
    }
  }
  DEBUG ((EFI_D_ERROR, "mGlobalNvsArea.Area->LegacyFreeSupport = %x\n", mGlobalNvsArea.Area->LegacyFreeSupport));
//[-end-160923-IB07400789-add]//

  mGlobalNvsArea.Area->ModemSel = mSystemConfiguration->ModemSel;

  mGlobalNvsArea.Area->GpsModeSel = mSystemConfiguration->GpsModeSel;

//[-start-160413-IB03090426-add]//
  //
  // IPC1 (PMI) device
  //
  mGlobalNvsArea.Area->Ipc1Enable = mSystemConfiguration->Ipc1Enable;
//[-end-160413-IB03090426-add]//

  mGlobalNvsArea.Area->BxtStepping = BxtStepping();
  DEBUG((DEBUG_INFO, "GlobalNvsArea.Area->BxtStepping: 0x%x;\n", mGlobalNvsArea.Area->BxtStepping));

//  if (mSystemConfiguration->ScHdAudioPostProcessingMod[29]){
//     DEBUG((DEBUG_INFO,"HDA: AudioDSP Pre/Post-Processing custom module 'Alpha' enabled (BIT29)\n"));
//
//     CopyMem(LocalGuidString, mSystemConfiguration->ScHdAudioPostProcessingModCustomGuid1, GUID_CHARS_NUMBER * sizeof(CHAR16));
//     GuidStringToAcpiBuffer (LocalGuidString, &AcpiGuidPart1, &AcpiGuidPart2);
//     mGlobalNvsArea.Area->HdaDspPpModCustomGuid1Low  = AcpiGuidPart1;
//     mGlobalNvsArea.Area->HdaDspPpModCustomGuid1High = AcpiGuidPart2;
//     DEBUG((DEBUG_INFO,"HdaDspPpModCustomGuid1Low  = 0x%016Lx\nHdaDspPpModCustomGuid2High = 0x%016Lx\n",
//             mGlobalNvsArea.Area->HdaDspPpModCustomGuid1Low, mGlobalNvsArea.Area->HdaDspPpModCustomGuid1High));
//   }
//
//   if (mSystemConfiguration->ScHdAudioPostProcessingMod[30]){
//     DEBUG((DEBUG_INFO,"HDA: AudioDSP Pre/Post-Processing custom module 'Beta' enabled (BIT30)\n"));
//
//     CopyMem(LocalGuidString, mSystemConfiguration->ScHdAudioPostProcessingModCustomGuid2, GUID_CHARS_NUMBER * sizeof(CHAR16));
//     GuidStringToAcpiBuffer (LocalGuidString, &AcpiGuidPart1, &AcpiGuidPart2);
//     mGlobalNvsArea.Area->HdaDspPpModCustomGuid2Low  = AcpiGuidPart1;
//     mGlobalNvsArea.Area->HdaDspPpModCustomGuid2High = AcpiGuidPart2;
//     DEBUG((DEBUG_INFO,"HdaDspPpModCustomGuid2Low  = 0x%016Lx\nHdaDspPpModCustomGuid2High = 0x%016Lx\n",
//             mGlobalNvsArea.Area->HdaDspPpModCustomGuid2Low, mGlobalNvsArea.Area->HdaDspPpModCustomGuid2High));
//   }
//
//   if (mSystemConfiguration->ScHdAudioPostProcessingMod[31]){
//     DEBUG((DEBUG_INFO,"HDA: AudioDSP Pre/Post-Processing custom module 'Gamma' enabled (BIT31)\n"));
//
//     CopyMem(LocalGuidString, mSystemConfiguration->ScHdAudioPostProcessingModCustomGuid3, GUID_CHARS_NUMBER * sizeof(CHAR16));
//     GuidStringToAcpiBuffer (LocalGuidString, &AcpiGuidPart1, &AcpiGuidPart2);
//     mGlobalNvsArea.Area->HdaDspPpModCustomGuid3Low  = AcpiGuidPart1;
//     mGlobalNvsArea.Area->HdaDspPpModCustomGuid3High = AcpiGuidPart2;
//     DEBUG((DEBUG_INFO,"HdaDspPpModCustomGuid3Low  = 0x%016Lx\nHdaDspPpModCustomGuid3High = 0x%016Lx\n",
//             mGlobalNvsArea.Area->HdaDspPpModCustomGuid3Low, mGlobalNvsArea.Area->HdaDspPpModCustomGuid3High));
//   }

   
//[-start-160413-IB03090426-modify]//
  //
  // USB TYPE-C UCSI
  //
  if(mSystemConfiguration->OsSelection == 0x0) {
    Pages = EFI_SIZE_TO_PAGES (0x1000);
    Address = 0xFFFFFFFF;
    Status  = gBS->AllocatePages (
                     AllocateMaxAddress,
                     EfiACPIMemoryNVS,
                     Pages,
                     &Address
                     );
    ASSERT_EFI_ERROR(Status);
    mGlobalNvsArea.Area->UsbTypeCOpBaseAddr = (UINT32)Address;
    Buffer = (VOID *) (UINTN) Address;
    Size = 0x1000;
    ZeroMem (Buffer, Size);
    DEBUG((EFI_D_INFO, "Status = %r, Address = %x\n", Status, mGlobalNvsArea.Area->UsbTypeCOpBaseAddr));
  }
//[-end-160413-IB03090426-modify]//
   

#if (ENBDT_PF_ENABLE == 1)
  //
  // Install ACPI interface for Intel Ready Mode Technology (IRMT)
  //
  if (mSystemConfiguration->IrmtConfiguration == 1) {
    //
    // Initialize the Irmt configuration in ACPI GNVS
    //
    mGlobalNvsArea.Area->IrmtCfg = BIT0;  // Irmt Enabled
  }

  //
  // Uninstall RTD3 table if it is disabled
  //
  if (mSystemConfiguration->Rtd3Support == 0) {
    // Register for OnReadyToBoot event
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               OnReadyToBootUninstallRtd3SSDTAcpiTable,
               NULL,
               &Event
               );
  }
#endif
//[-start-170628-IB07400881-add]//
  if (mSystemConfiguration->S5WakeOnUsb) {
    mGlobalNvsArea.Area->S5WakeOnUsbWaitTime = mSystemConfiguration->S5WakeOnUsbWaitTime;
  } else {
    mGlobalNvsArea.Area->S5WakeOnUsbWaitTime = 0;
  }
//[-end-170628-IB07400881-add]//
//[-start-160809-IB07400769-add]//
  if (mSystemConfiguration->SeCPrepareForUpdate == 1) {
    mGlobalNvsArea.Area->TpmStatus = 0xFF;
  } else {
    mGlobalNvsArea.Area->TpmStatus = 0x00;
  }
//[-end-160809-IB07400769-add]//
//[-start-170119-IB07401072-add]//
#ifdef PCI_64BITS_MMIO_DECODE_SUPPORT
  //
  // Provide available above 4GB MMIO resource if PCI 64-bit MMIO is enabled
  //
  if (PcdGet8 (PcdMmioMem64Enable) == 0x01) {
    Touud = (EFI_PHYSICAL_ADDRESS)LShiftU64 ((MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, TOUUD_HI_0_0_0_PCI_CUNIT_REG)) & 0xFFFFFFFF),32);
    Touud = (EFI_PHYSICAL_ADDRESS) (Touud + (MmioRead32 (MmPciAddress (0,SA_MC_BUS, 0, 0, TOUUD_LO_0_0_0_PCI_CUNIT_REG)) & 0xFFF00000));
    if (Touud > MEM_EQU_4GB) {
      mGlobalNvsArea.Area->MmioMem64Base = Touud;
    } else {
      mGlobalNvsArea.Area->MmioMem64Base = MEM_EQU_4GB;
    }
    mGlobalNvsArea.Area->MmioMem64Length = PcdGet64 (PcdMmioMem64Length);
    mGlobalNvsArea.Area->MmioMem64Limit = mGlobalNvsArea.Area->MmioMem64Base + mGlobalNvsArea.Area->MmioMem64Length - 1;
  
    //
    // Pass 64 bit MMIO support to ASL
    //
    if (FeaturePcdGet (Pcd64BitAmlSupported)) {
      mGlobalNvsArea.Area->Mmio64AslSupport = mSystemConfiguration->PciMem64DecodeSupport;
    }
  }
#endif  
//[-end-170119-IB07401072-add]//

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiGlobalNvsAreaProtocolGuid,
                  &mGlobalNvsArea,
                  NULL
                  );

  //
  // Read tables from the storage file.
  //
  while (!EFI_ERROR (Status)) {
    CurrentTable = NULL;

    Status = FwVol->ReadSection (
                      FwVol,
                      &gEfiAcpiTableStorageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID **) &CurrentTable,
                      (UINTN *) &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      //
      // Allow platform specific code to reject the table or update it
      //
      AcpiStatus = AcpiPlatformHooksIsActiveTable (CurrentTable);

      if (!EFI_ERROR (AcpiStatus)) {
        //
        // Perform any table specific updates.
        //
        AcpiStatus = PlatformUpdateTables (CurrentTable);

        if (!EFI_ERROR (AcpiStatus)) {
          //
          // Add the table
          //
          TableHandle = 0;
          AcpiStatus = AcpiSupport->SetAcpiTable (
                                      AcpiSupport,
                                      CurrentTable,
                                      TRUE,
                                      TableVersion,
                                      &TableHandle
                                      );
          ASSERT_EFI_ERROR (AcpiStatus);
        } else {
          DEBUG((EFI_D_ERROR, "Skip %x\n", CurrentTable));
        }
      }
      //
      // Increment the instance
      //
      Instance++;
    }
  }

  Status = InstallAcpiTableForPlatformSsdt(FwVol, AcpiSupport);
  if (Status != EFI_SUCCESS){
    DEBUG ((EFI_D_ERROR, "UBMS SSDT - Not found \n" ));
  }


  Status = EfiCreateEventReadyToBootEx (
            TPL_CALLBACK,
            OnReadyToBoot,
            NULL,
            &Event
            );


  Status = OemSvcUpdateGlobalNvs (mGlobalNvsArea.Area);
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "DxeOemSvcChipsetLib OemSvcUpdateGlobalNvs, Status : %r\n", Status));

  //
  // Finished
  //

  //
  // Install Publish ACPI Tables Done Protocol for notification and dependency
  //
  Status = gBS->InstallProtocolInterface (
                  &mAcpiPlatformHandle,
                  &gPublishAcpiTableDoneProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  return EFI_SUCCESS;
}
//[-end-160216-IB03090424-modify]//
//[-end-151204-IB02950555-modify]//
//[-start-160216-IB03090424-modify]//
/**
This function updates the UART 2 device base address in the DBG ACPI table.

@param AcpiSupport        Instance of the ACPI Support protocol.

@retval EFI_SUCCESS       The function completed successfully.
@retval EFI_NOT_FOUND     The requested table does not exist.

**/
EFI_STATUS
UpdateDbg2Table (
  IN EFI_ACPI_SUPPORT_PROTOCOL  *AcpiSupport
)
{
  EFI_STATUS                    Status;

  EFI_ACPI_DESCRIPTION_HEADER   *Table;
  EFI_ACPI_DEBUG_PORT_2_TABLE   *Dbg2Table;
  EFI_ACPI_TABLE_VERSION        Version;
  CHIPSET_CONFIGURATION         SetupVarBuffer;
  UINTN                         VariableSize;
  UINTN                         Handle;
  UINT8                         Index = 0;
  UINTN                         Uart2BaseAddressRegister = 0;

  //
  // Find the DBG2 ACPI table
  //
  do {
    Status = AcpiSupport->GetAcpiTable(AcpiSupport, Index, (VOID **)&Table, &Version, &Handle);
//[-start-170414-IB07400859-add]//
    if (Status == EFI_NOT_FOUND) {
      return EFI_NOT_FOUND;
    }
//[-end-170414-IB07400859-add]//
    Index++;
  } while (Table->Signature != EFI_ACPI_5_0_DEBUG_PORT_2_TABLE_SIGNATURE);

  if (Status == EFI_NOT_FOUND) {
    return EFI_NOT_FOUND;
  }

  Dbg2Table = (EFI_ACPI_DEBUG_PORT_2_TABLE *) Table;

  //
  // Update the UART2 BAR in the DBG2 ACPI table
  //
  Uart2BaseAddressRegister = MmPciAddress(
    0,
    DEFAULT_PCI_BUS_NUMBER_SC,
    PCI_DEVICE_NUMBER_LPSS_HSUART,
    PCI_FUNCTION_NUMBER_LPSS_HSUART2,
    R_LPSS_IO_BAR
    );

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SetupVarBuffer, &VariableSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &VariableSize,
                  &SetupVarBuffer
                  );
//[-end-160803-IB07220122-modify]//
  ASSERT_EFI_ERROR (Status);
  
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SetupVarBuffer, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      return Status;  
    }
  }
  if (SetupVarBuffer.OsDbgEnable && (MmioRead32(Uart2BaseAddressRegister) != 0xFFFFFFFF)) {
    Dbg2Table->DbgDeviceInfoCom1.BaseAddressRegister->Address = MmioRead32(Uart2BaseAddressRegister) & B_LPSS_IO_BAR_BA;

    DEBUG((DEBUG_INFO, "UART2 Base Address in DBG2 ACPI Table: 0x%x\n", Dbg2Table->DbgDeviceInfoCom1.BaseAddressRegister->Address));

//[-start-160817-IB03090432-modify]//
    if (GetBxtSeries() == Bxt1) {
//[-end-160817-IB03090432-modify]//
      // Hide the UART2 device to prevent an OS driver from loading against it
      SideBandAndThenOr32(
        SB_PORTID_PSF3,
        R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2,
        0xFFFFFFFF,
        B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS
        );
      DEBUG((DEBUG_INFO, "Kernel Debugger Mode Enabled - LPSS D24 F2 CFG_DIS: 0x%08X\n", SideBandRead32(SB_PORTID_PSF3, R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2)));
    }

    Status = AcpiSupport->SetAcpiTable(
      AcpiSupport,
      Dbg2Table,
      TRUE,
      Version,
      &Handle
      );

    ASSERT_EFI_ERROR(Status);
  } else {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
//[-end-160216-IB03090424-modify]//

//[-start-160413-IB03090426-add]//
/**
This function updates the UART 2 device base address in the DBGP ACPI table.

@param AcpiSupport        Instance of the ACPI Support protocol.

@retval EFI_SUCCESS       The function completed successfully.
@retval EFI_NOT_FOUND     The requested table does not exist.

**/
EFI_STATUS
UpdateDbgpTable (
  IN EFI_ACPI_SUPPORT_PROTOCOL  *AcpiSupport
)
{
  EFI_STATUS                    Status;

  EFI_ACPI_DESCRIPTION_HEADER   *Table;
  EFI_ACPI_DEBUG_PORT_DESCRIPTION_TABLE   *DbgTable;
  EFI_ACPI_TABLE_VERSION        Version;
  CHIPSET_CONFIGURATION         SetupVarBuffer;
  UINTN                         VariableSize;
  UINTN                         Handle;
  UINT8                         Index = 0;
  UINTN                         Uart2BaseAddressRegister = 0;

  //
  // Find the DBGP ACPI table
  //
  do {
    Status = AcpiSupport->GetAcpiTable(AcpiSupport, Index, (VOID **)&Table, &Version, &Handle);
//[-start-170414-IB07400859-add]//
    if (Status == EFI_NOT_FOUND) {
      return EFI_NOT_FOUND;
    }
//[-end-170414-IB07400859-add]//
    Index++;
  } while (Table->Signature != EFI_ACPI_5_0_DEBUG_PORT_TABLE_SIGNATURE);

  if (Status == EFI_NOT_FOUND) {
    return EFI_NOT_FOUND;
  }

  DbgTable = (EFI_ACPI_DEBUG_PORT_DESCRIPTION_TABLE *) Table;

  //
  // Update the UART2 BAR in the DBGP ACPI table
  //
  Uart2BaseAddressRegister = MmPciAddress(
                               0,
                               DEFAULT_PCI_BUS_NUMBER_SC,
                               PCI_DEVICE_NUMBER_LPSS_HSUART,
                               PCI_FUNCTION_NUMBER_LPSS_HSUART2,
                               R_LPSS_IO_BAR
                               );

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SetupVarBuffer, &VariableSize);
  Status = gRT->GetVariable (
                  SETUP_VARIABLE_NAME,
                  &gSystemConfigurationGuid,
                  NULL,
                  &VariableSize,
                  &SetupVarBuffer
                  );
//[-end-160803-IB07220122-modify]//
  ASSERT_EFI_ERROR (Status);
  
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SetupVarBuffer, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      return Status;  
    }
  }
  if (SetupVarBuffer.OsDbgEnable && (MmioRead32(Uart2BaseAddressRegister) != 0xFFFFFFFF)) {
    DbgTable->BaseAddress.Address = MmioRead32(Uart2BaseAddressRegister) & B_LPSS_IO_BAR_BA;
	
    DEBUG((DEBUG_INFO, "UART2 Base Address in DBGP ACPI Table: 0x%x\n", DbgTable->BaseAddress.Address));

//[-start-160817-IB03090432-modify]//
    if (GetBxtSeries() == Bxt1) {
//[-end-160817-IB03090432-modify]//
      // Hide the UART2 device to prevent an OS driver from loading against it
      SideBandAndThenOr32(
        SB_PORTID_PSF3,
        R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2,
        0xFFFFFFFF,
        B_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2_CFGDIS
        );
      DEBUG((DEBUG_INFO, "Kernel Debugger Mode Enabled - LPSS D24 F2 CFG_DIS: 0x%08X\n", SideBandRead32(SB_PORTID_PSF3, R_PCH_PCR_PSF_3_AGNT_T0_SHDW_CFG_DIS_LPSS_RS0_D24_F2)));
    }

    Status = AcpiSupport->SetAcpiTable(
                            AcpiSupport,
                            DbgTable,
                            TRUE,
                            Version,
                            &Handle
                            );

    ASSERT_EFI_ERROR(Status);
  } else {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
//[-end-160413-IB03090426-add]//


//[-start-151228-IB03090424-add]//
/**
  Publish WatchDog Action ACPI Table

  @retval   EFI_SUCCESS     The WatchDog Action ACPI table is published successfully.
  @retval   Others          The WatchDog Action ACPI table is not published.
**/
EFI_STATUS
PublishWatchDogActionTable (
  VOID
  )
{
  EFI_STATUS                     Status;
  UINT16                         AcpiBase;
  UINT32                         Value;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINTN                          TableKey;

  //
  // Enable reset on TCO time out
  //
//[-start-161229-IB07400829-remove]//
  //
  // Controlled by SMI
  //
//  MmioAnd8 (PMC_BASE_ADDRESS + R_PMC_PM_CFG, (UINT8) ~B_PMC_PM_CFG_NO_REBOOT);
//[-end-161229-IB07400829-remove]//

  //
  // Read ACPI Base Address
  //
  AcpiBase = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Construct WatchDog Action Table
  //

  //
  // Set Watchdog Flags
  //
  mWatchDogActionTableAcpiTemplate.Table.WatchdogFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_STOPPED_IN_SLEEP_STATE;

  Value = IoRead32 (AcpiBase + R_SMI_EN) & B_SMI_EN_TCO;
  if (Value) {
    //
    // TCO is enabled
    //
    mWatchDogActionTableAcpiTemplate.Table.WatchdogFlags |= EFI_ACPI_WDAT_1_0_WATCHDOG_ENABLED;
  }

  EntryNumber = 0;
  //
  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET
  //
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE | EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_PRESERVE_REGISTER;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x0A;  // 10 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO_RLD;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x04;  // It can be Any value
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x3FF;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_COUNTDOWN | EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_PRESERVE_REGISTER;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x0A;  // 10 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x10;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO_TMR;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x3FF;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x3FF;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_RUNNING_STATE
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_RUNNING_STATE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_VALUE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x01;  // 1 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x0B;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO1_CNT;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x01;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE | EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_PRESERVE_REGISTER;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x01;  // 1 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x0B;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO1_CNT;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x01;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_STOPPED_STATE
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_STOPPED_STATE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_VALUE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x01;  // 1 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x0B;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO1_CNT;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x01;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x01;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE | EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_PRESERVE_REGISTER;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x01;  // 1 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x0B;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO1_CNT;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x01;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x01;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_WATCHDOG_STATUS
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_WATCHDOG_STATUS;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_VALUE;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x01;  // 1 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x11;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO_STS;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x01;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x01;
  EntryNumber ++;

  // Watchdog action EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_WATCHDOG_STATUS
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].WatchdogAction   = EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_WATCHDOG_STATUS;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].InstructionFlags = EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE | EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_PRESERVE_REGISTER;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[0]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Reserved_2[1]    = 0x00;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.AddressSpaceId    = 0x01;  // 0 System Memory, 1 System IO
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitWidth  = 0x01;  // 1 bit
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.RegisterBitOffset = 0x11;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Reserved          = 0x03;  // AccessSize, Dword Access
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].RegisterRegion.Address           = AcpiBase + R_TCO_STS;
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Value                            = 0x01;  // Write '1' to clear
  mWatchDogActionTableAcpiTemplate.Entry[EntryNumber].Mask                             = 0x01;
  EntryNumber ++;

  // Set Length and Entry number
  mWatchDogActionTableAcpiTemplate.Table.Header.Length = sizeof(EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE) + EntryNumber*sizeof(EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY);
  mWatchDogActionTableAcpiTemplate.Table.NumberWatchdogInstructionEntries = EntryNumber;

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO,"Installing WDAT...\n"));
  //
  // Install ACPI table
  //
  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        &mWatchDogActionTableAcpiTemplate,
                        mWatchDogActionTableAcpiTemplate.Table.Header.Length,
                        &TableKey
                        );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO,"Install WDAT %r\n", Status));

  return Status;
}
//[-end-151228-IB03090424-add]//

UINT8
ReadCmosBank1Byte (
  IN  UINT8                           Index
  )
{
  UINT8                               Data;

  IoWrite8 (0x72, Index);
  Data = IoRead8 (0x73);
  return Data;
}

VOID
WriteCmosBank1Byte (
  IN  UINT8                           Index,
  IN  UINT8                           Data
  )
{
  IoWrite8 (0x72, Index);
  IoWrite8 (0x73, Data);
}
