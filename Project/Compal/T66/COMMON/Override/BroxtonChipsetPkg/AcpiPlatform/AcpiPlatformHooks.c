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
Copyright (c) 1996 - 2014, Intel Corporation.

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.


Module Name:

  AcpiPlatformHooks.c

Abstract:

  ACPI Platform Driver Hooks

--*/

//
// Statements that include other files
//
#include "Npkt.h"
#include "AcpiPlatform.h"
#include "AcpiPlatformHooks.h"
#include <Library/BaseLib.h>
#include <Guid/PlatformInfo.h>
#include "Rsci.h"
#include <Guid/NpkInfoHob.h>
//[-start-160216-IB03090424-add]//
#include <Library/SteppingLib.h>
//[-end-160216-IB03090424-add]//
//[-start-160216-IB03090424-remove]//
//#include "TdtHi.h"
//[-end-160216-IB03090424-remove]//
//
// Prototypes of the various hook functions.
//
#include "AcpiPlatformHooksLib.h"
//[PRJ]+ >>>> Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
#include <CompalGlobalNvsProtocol.h>
#include <CompalEepromDefinition.h>
#include <CompalEEPROMDxeProtocol.h>
COMPAL_GLOBAL_NVS_PROTOCOL       *CompalGlobalNvsProtocol;
COMPAL_GLOBAL_NVS_AREA           *mCompalGlobalNvsArea;
//[PRJ]+ <<<< Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
extern EFI_GLOBAL_NVS_AREA_PROTOCOL  mGlobalNvsArea;

extern EFI_PLATFORM_INFO_HOB         *mPlatformInfo;
extern CHIPSET_CONFIGURATION         *mSystemConfiguration;

ENHANCED_SPEEDSTEP_PROTOCOL          *mEistProtocol = NULL;


EFI_CPU_ID_MAP              mCpuApicIdAcpiIdMapTable[MAX_CPU_NUM];

EFI_STATUS
AppendCpuMapTableEntry (
  IN EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE   *AcpiLocalApic
  )
{
  BOOLEAN Added;
  UINTN   Index;

  for (Index = 0; Index < MAX_CPU_NUM; Index++) {
    if ((mCpuApicIdAcpiIdMapTable[Index].ApicId == AcpiLocalApic->ApicId) && mCpuApicIdAcpiIdMapTable[Index].Flags) {
      return EFI_SUCCESS;
    }
  }

  Added = FALSE;
  for (Index = 0; Index < MAX_CPU_NUM; Index++) {
      if (!mCpuApicIdAcpiIdMapTable[Index].Flags) {
        mCpuApicIdAcpiIdMapTable[Index].Flags           = 1;
        mCpuApicIdAcpiIdMapTable[Index].ApicId          = AcpiLocalApic->ApicId;
        mCpuApicIdAcpiIdMapTable[Index].AcpiProcessorId = AcpiLocalApic->AcpiProcessorId;
      Added = TRUE;
      break;
    }
  }

  ASSERT (Added);
  return EFI_SUCCESS;
}

UINT32
ProcessorId2ApicId (
  UINT32  AcpiProcessorId
  )
{
  UINTN Index;

  ASSERT (AcpiProcessorId < MAX_CPU_NUM);
  for (Index = 0; Index < MAX_CPU_NUM; Index++) {
    if (mCpuApicIdAcpiIdMapTable[Index].Flags && (mCpuApicIdAcpiIdMapTable[Index].AcpiProcessorId == AcpiProcessorId)) {
      return mCpuApicIdAcpiIdMapTable[Index].ApicId;
    }
  }

  return (UINT32) -1;
}

UINT8
GetProcNumberInPackage (
  IN UINT8  Package
  )
{
  UINTN Index;
  UINT8 Number;

  Number = 0;
  for (Index = 0; Index < MAX_CPU_NUM; Index++) {
    if (mCpuApicIdAcpiIdMapTable[Index].Flags && (((mCpuApicIdAcpiIdMapTable[Index].ApicId >> 0x04) & 0x01) == Package)) {
      Number++;
    }
  }

  return Number;
}

EFI_STATUS
LocateCpuEistProtocol (
  IN UINT32                           CpuIndex,
  OUT ENHANCED_SPEEDSTEP_PROTOCOL     **EistProtocol
  )
{
  UINTN                       HandleCount;
  EFI_HANDLE                  *HandleBuffer;
  ENHANCED_SPEEDSTEP_PROTOCOL *EistProt;
  UINTN                       Index;
  UINT32                      ApicId;
  EFI_STATUS                  Status;

  HandleCount = 0;
  gBS->LocateHandleBuffer (
        ByProtocol,
        &gEnhancedSpeedstepProtocolGuid,
        NULL,
        &HandleCount,
        &HandleBuffer
        );

  Index     = 0;
  EistProt  = NULL;
  Status    = EFI_NOT_FOUND;
  while (Index < HandleCount) {
    gBS->HandleProtocol (
          HandleBuffer[Index],
          &gEnhancedSpeedstepProtocolGuid,
          (VOID **) &EistProt
          );
    //
    // Adjust the CpuIndex by +1 due to the AcpiProcessorId is 1 based.
    //
    ApicId = ProcessorId2ApicId (CpuIndex+1);
    if (ApicId == (UINT32) -1) {
      break;
    }

    if (EistProt->ProcApicId == ApicId) {
      Status = EFI_SUCCESS;
      break;
    }

    Index++;
  }

  if (HandleBuffer != NULL) {
    gBS->FreePool (HandleBuffer);
  }

  if (!EFI_ERROR (Status)) {
    *EistProtocol = EistProt;
  } else {
    *EistProtocol = NULL;
  }

  return Status;
}

EFI_STATUS
PlatformHookInit (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (
                  &gEnhancedSpeedstepProtocolGuid,
                  NULL,
                  (VOID **) &mEistProtocol
                  );
  
  ASSERT_EFI_ERROR (Status);

  return Status;
}

EFI_STATUS
AcpiPlatformHooksIsActiveTable (
  IN OUT EFI_ACPI_COMMON_HEADER     *Table
  )
/*++

Routine Description:

  Called for every ACPI table found in the BIOS flash.
  Returns whether a table is active or not. Inactive tables
  are not published in the ACPI table list. This hook can be
  used to implement optional SSDT tables or enabling/disabling
  specific functionality (e.g. SPCR table) based on a setup
  switch or platform preference. In case of optional SSDT tables,
  the platform flash will include all the SSDT tables but will
  return EFI_SUCCESS only for those tables that need to be
  published.

Arguments:

  *Table - Pointer to the active table

Returns:

  Status  - EFI_SUCCESS if the table is active
  Status  - EFI_UNSUPPORTED if the table is not active
--*/
{
  EFI_ACPI_DESCRIPTION_HEADER *TableHeader;

  TableHeader = (EFI_ACPI_DESCRIPTION_HEADER *) Table;

  if (TableHeader->Signature == EFI_ACPI_2_0_STATIC_RESOURCE_AFFINITY_TABLE_SIGNATURE) {
    
  }
//[-start-170414-IB07400859-add]//
  //
  // Hide DBG2 & DBGP table, if OS debug is disabled.
  //
  if ((mSystemConfiguration->OsDbgEnable == 0) && (CompareMem (&TableHeader->Signature, "DBG2", 4) == 0)){
    return EFI_UNSUPPORTED;
  }
  if ((mSystemConfiguration->OsDbgEnable == 0) && (CompareMem (&TableHeader->Signature, "DBGP", 4) == 0)){
    return EFI_UNSUPPORTED;
  }
//[-end-170414-IB07400859-add]//
  return EFI_SUCCESS;
}

EFI_STATUS
PatchGv3SsdtTable (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader
  )
/*++

  Routine Description:

    Update the GV3 SSDT table

  Arguments:

    *TableHeader   - The table to be set

  Returns:

    EFI_SUCCESS -  Returns Success

--*/
{
  EFI_STATUS                  Status;
  UINT8                       *CurrPtr;
  UINT8                       *SsdtPointer;
  UINT32                      Signature;
  UINT32                      CpuFixes;
  UINT32                      NpssFixes;
  UINT32                      SpssFixes;
  UINT32                      CpuIndex;
  UINT32                      PackageSize;
  UINT32                      NewPackageSize;
  UINT32                      AdjustSize;
  UINTN                       EntryIndex;
  UINTN                       TableIndex;
  EFI_ACPI_NAME_COMMAND       *PssTable;
  EFI_PSS_PACKAGE             *PssTableItemPtr;
  ENHANCED_SPEEDSTEP_PROTOCOL *EistProt;
  EIST_INFORMATION            *EistInfo;
  EFI_ACPI_CPU_PSS_STATE      *PssState;
  EFI_ACPI_NAMEPACK_DWORD     *NamePtr;
  //
  // Loop through the ASL looking for values that we must fix up.
  //
  NpssFixes = 0;
  SpssFixes = 0;
  CpuFixes  = 0;
  CpuIndex  = 0;
  CurrPtr   = (UINT8 *) TableHeader;

  EistProt  = NULL;
  for (SsdtPointer = CurrPtr; SsdtPointer <= (CurrPtr + ((EFI_ACPI_COMMON_HEADER *) CurrPtr)->Length); SsdtPointer++) {
    Signature = *(UINT32 *) SsdtPointer;
    switch (Signature) {

    case SIGNATURE_32 ('_', 'P', 'R', '_'):
      //
      // _CPUX ('0' to '0xF')
      //
      CpuIndex = *(SsdtPointer + 7);
      if (CpuIndex >= '0' && CpuIndex <= '9') {
        CpuIndex -= '0';
      } else {
        if (CpuIndex > '9') {
          CpuIndex -= '7';
        }
      }

      CpuFixes++;
      LocateCpuEistProtocol (CpuIndex, &EistProt);
      break;

    case SIGNATURE_32 ('D', 'O', 'M', 'N'):

      NamePtr = ACPI_NAME_COMMAND_FROM_NAMEPACK_STR (SsdtPointer);
      if (NamePtr->StartByte != AML_NAME_OP) {
        continue;
      }

      if (NamePtr->Size != AML_NAME_DWORD_SIZE) {
        continue;
      }

      NamePtr->Value = 0;

        if (mCpuApicIdAcpiIdMapTable[CpuIndex].Flags) {
          NamePtr->Value = (mCpuApicIdAcpiIdMapTable[CpuIndex].ApicId >> 0x04) & 0x01;
      } 
      break;

    case SIGNATURE_32 ('N', 'C', 'P', 'U'):

      NamePtr = ACPI_NAME_COMMAND_FROM_NAMEPACK_STR (SsdtPointer);
      if (NamePtr->StartByte != AML_NAME_OP) {
        continue;
      }

      if (NamePtr->Size != AML_NAME_DWORD_SIZE) {
        continue;
      }

        NamePtr->Value = 0;
        if (mCpuApicIdAcpiIdMapTable[CpuIndex].Flags) {
          NamePtr->Value = GetProcNumberInPackage ((mCpuApicIdAcpiIdMapTable[CpuIndex].ApicId >> 0x04) & 0x01);
      } 
      break;

    case SIGNATURE_32 ('N', 'P', 'S', 'S'):
    case SIGNATURE_32 ('S', 'P', 'S', 'S'):
      if (EistProt == NULL) {
        continue;
      }

      PssTable = ACPI_NAME_COMMAND_FROM_NAME_STR (SsdtPointer);
      if (PssTable->StartByte != AML_NAME_OP) {
        continue;
      }

      Status      = EistProt->GetEistTable (EistProt, &EistInfo, (VOID **) &PssState);

      AdjustSize  = PssTable->NumEntries * sizeof (EFI_PSS_PACKAGE);
      AdjustSize -= EistInfo->NumStates * sizeof (EFI_PSS_PACKAGE);
      PackageSize     = (PssTable->Size & 0xF) + ((PssTable->Size & 0xFF00) >> 4);
      NewPackageSize  = PackageSize - AdjustSize;
      PssTable->Size  = (UINT16) ((NewPackageSize & 0xF) + ((NewPackageSize & 0x0FF0) << 4));

      //
      // Set most significant two bits of byte zero to 01, meaning two bytes used
      //
      PssTable->Size |= 0x40;

      //
      // Set unused table to Noop Code
      //
      SetMem( (UINT8 *) PssTable + NewPackageSize + AML_NAME_PREFIX_SIZE, AdjustSize, AML_NOOP_OP);
      PssTable->NumEntries  = (UINT8) EistInfo->NumStates;
      PssTableItemPtr       = (EFI_PSS_PACKAGE *) ((UINT8 *) PssTable + sizeof (EFI_ACPI_NAME_COMMAND));

      //
      // Update the size
      //
      for (TableIndex = 0; TableIndex < EistInfo->NumStates; TableIndex++) {
        EntryIndex                = EistInfo->NumStates - TableIndex - 1;
        PssTableItemPtr->CoreFreq = PssState[EntryIndex].CoreFrequency * PssState[EntryIndex].Control;
        PssTableItemPtr->Power    = PssState[EntryIndex].Power * 1000;
        if (PssTable->NameStr == SIGNATURE_32 ('N', 'P', 'S', 'S')) {
          PssTableItemPtr->BMLatency    = PssState[EntryIndex].BusMasterLatency;
          PssTableItemPtr->TransLatency = PssState[EntryIndex].TransitionLatency;
        } else {
          //
          // This method should be supported by SMM PPM Handler
          //
          PssTableItemPtr->BMLatency    = PssState[EntryIndex].BusMasterLatency * 2;
          PssTableItemPtr->TransLatency = PssState[EntryIndex].TransitionLatency * 10;
        }

        PssTableItemPtr->Control  = PssState[EntryIndex].Control;
        PssTableItemPtr->Status   = PssState[EntryIndex].Status;
        PssTableItemPtr++;
      }

      if (PssTable->NameStr == SIGNATURE_32 ('N', 'P', 'S', 'S')) {
        NpssFixes++;
      } else {
        SpssFixes++;
      }

      SsdtPointer = (UINT8 *) PssTable + PackageSize;
      break;
    } // switch
  } // for

  //
  // N fixes together currently
  //
  ASSERT (CpuFixes == (UINT32) MAX_CPU_NUM);
  ASSERT (SpssFixes == NpssFixes);
  ASSERT (CpuFixes >= SpssFixes);

  return EFI_SUCCESS;
}

//[-start-160317-IB03090425-modify]//
EFI_STATUS
PatchNpktTable (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader
  )
/*++

  Routine Description:

    Update the Npkt table

  Arguments:

    *TableHeader   - The table to be set

  Returns:

    EFI_SUCCESS - Returns EFI_SUCCESS

--*/
{
  EFI_ACPI_NORTH_PEAK_TABLE_HEADER *NpktTable;
  UINTN                            NpkBaseAddress;
  MMIOBAR64                        MtbBar;
  UINT8                            Data8;
  EFI_PEI_HOB_POINTERS             GuidHob;
  EFI_NORTH_PEAK_INFO_HOB          *NpkInfoHob = NULL;

  NpktTable     = (EFI_ACPI_NORTH_PEAK_TABLE_HEADER *)(UINTN)TableHeader;

  //
  // Update HOB variable for PCI resource information
  // Get the HOB list.  If it is not present, then ASSERT.
  //
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiNorthPeakGuid, GuidHob.Raw)) != NULL) {
      NpkInfoHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

//[-start-160216-IB03090424-add]//
  if (NpkInfoHob == NULL) {
    return EFI_NOT_FOUND;
  }
//[-end-160216-IB03090424-add]//

  NpkBaseAddress = MmPciAddress (
      0,
      0,
      PCI_DEVICE_NUMBER_NPK,
      PCI_FUNCTION_NUMBER_NPK,
      0
      );

  MtbBar.r.BarLow = (MmioRead32 (NpkBaseAddress + R_PCH_NPK_CSR_MTB_LBAR) & B_PCH_NPK_CSR_MTB_RBAL); 
  MtbBar.r.BarHigh = MmioRead32 (NpkBaseAddress + R_PCH_NPK_CSR_MTB_UBAR); 

  DEBUG((EFI_D_INFO, "  NpkInfoHob->MtbAddress = %x \n",NpkInfoHob->MtbAddress));
  DEBUG((EFI_D_INFO, "  NpkInfoHob->MtbSize = %x \n",NpkInfoHob->MtbSize));
  DEBUG((EFI_D_INFO, "  NpkInfoHob->MtbRecoveryAddress = %x \n",NpkInfoHob->MtbRecoveryAddress));
  DEBUG((EFI_D_INFO, "  NpkInfoHob->MtbRecoverySize = %x \n",NpkInfoHob->MtbRecoverySize));
  DEBUG((EFI_D_INFO, "  NpkInfoHob->CsrAddress = %x \n",NpkInfoHob->CsrAddress));
  DEBUG((EFI_D_INFO, "  NpkInfoHob->CsrSize = %x \n",NpkInfoHob->CsrSize));
  DEBUG((EFI_D_INFO, "  NpkInfoHob->CsrRecoveryAddress = %x \n",NpkInfoHob->CsrRecoveryAddress));

  NpktTable->SpecificTable.MtbAddress = NpkInfoHob->MtbAddress ;
  NpktTable->SpecificTable.MtbSize = NpkInfoHob->MtbSize;
  NpktTable->SpecificTable.MtbWriteOffset = 0;
  NpktTable->SpecificTable.MtbRecoveryAddress = NpkInfoHob->MtbRecoveryAddress;
  NpktTable->SpecificTable.MtbRecoverySize = NpkInfoHob->MtbRecoverySize;
  NpktTable->SpecificTable.MtbRecoveryWriteOffset = 0;  
  NpktTable->SpecificTable.CsrAddress = NpkInfoHob->CsrAddress;
  NpktTable->SpecificTable.CsrSize = NpkInfoHob->CsrSize;
  NpktTable->SpecificTable.CsrWriteOffset = 0; //Bios is still tracing , so this should kept to "0" 
  NpktTable->SpecificTable.CsrRecoveryAddress = NpkInfoHob->CsrRecoveryAddress;
  NpktTable->SpecificTable.CsrRecoverySize = NpkInfoHob->CsrRecoverySize;
  NpktTable->SpecificTable.CsrRecoveryWriteOffset = 0; // Recovery should be set to zero as BIOS cann't figure out this address.

  Data8 = 0;
 
  if(NpkInfoHob->MtbWrapStatus){
    Data8 |= MTB_WRITE_WRAPPED;
  }

  Data8 |= CSR_IN_USE;  // Bios will be tracing when windows kernel comes up , so always set it

  NpktTable->Flags = Data8; 
  DEBUG((EFI_D_INFO, "  NpktTable->Flags = %x \n",NpktTable->Flags));

  return EFI_SUCCESS;
}
//[-end-160317-IB03090425-modify]//

EFI_STATUS
PatchDsdtTable (
  IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader
  )
/*++

  Routine Description:

    Update the DSDT table

  Arguments:

    *TableHeader   - The table to be set

  Returns:

    EFI_SUCCESS - Returns EFI_SUCCESS

--*/
{

  UINT8   *CurrPtr;
  UINT8   *DsdtPointer;
  UINT32  *Signature;
  UINT8   *EndPtr;
//[-start-160801-IB07220121-modify]//
  UINT16  *Operation;
//[-end-160801-IB07220121-modify]//
  UINT32  *Address;
  UINT16  *Size;
//  COMPAL_EEPROM_DXE_PROTOCOL         *EepromDefineProtocol;
//[-start-161027-IB07400806-add]//
  UINT8   *Operation8;
//[-end-161027-IB07400806-add]//
//[PRJ]+ >>>> Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
    EFI_STATUS                        Status = EFI_SUCCESS;

    Status = gBS->LocateProtocol (&gCompalGlobalNvsProtocolGuid, NULL, &CompalGlobalNvsProtocol);
    if ( EFI_ERROR (Status) ) {
        return Status;
    }
//[PRJ]+ <<<< Modify for support VirtualEEPROMVerifyTool and CMFCVerify  	
  // Fix PCI32 resource "FIX0" -- PSYS system status area
  CurrPtr = (UINT8*) &((EFI_ACPI_DESCRIPTION_HEADER*) TableHeader)[0];
  EndPtr = (UINT8*) TableHeader;
  EndPtr = EndPtr + TableHeader->Length;
  while (CurrPtr < (EndPtr-2)) {

//[-start-161018-IB06740518-add]//
    //
	// LAN/PCIe Wake should not be supported when PCIe root ports are disabled
	// Hence making _L** methods do dummy , else it will cause Interrupt storm  due to controller issue on FAB1 baords
	//
    if (mSystemConfiguration->PcieRootPortEn[0]  == 0){
      if ((CurrPtr[0] == '_') && (CurrPtr[1] == 'L') && (CurrPtr[2] == '0') && (CurrPtr[3] == '7')) {
        CurrPtr[0] = 'X';
      }
    }

    if (mSystemConfiguration->PcieRootPortEn[1]  == 0){
      if ((CurrPtr[0] == '_') && (CurrPtr[1] == 'L') && (CurrPtr[2] == '0') && (CurrPtr[3] == '8')) {
        CurrPtr[0] = 'X';
      }
    }

    if((mSystemConfiguration->PcieRootPortEn[2]  == 0) && (mSystemConfiguration->PcieRootPortEn[3]  == 0) && (mSystemConfiguration->PcieRootPortEn[5]  == 0)){
      if ((CurrPtr[0] == '_') && (CurrPtr[1] == 'L') && (CurrPtr[2] == '0') && (CurrPtr[3] == '3')) {
        CurrPtr[0] = 'X';
      }
    }

    if (mSystemConfiguration->PcieRootPortEn[4]  == 0){
      if ((CurrPtr[0] == '_') && (CurrPtr[1] == 'L') && (CurrPtr[2] == '0') && (CurrPtr[3] == '6')) {
        CurrPtr[0] = 'X';
      }
    }
//[-end-161018-IB06740518-add]//

    //
    // Removed the _S3 tag to indicate that we do not support S3. The 4th byte is blank space
    // since there are only 3 char "_S3".
    //
    if (mSystemConfiguration->AcpiSuspendState == 0) {
      // for iasl compiler version 20061109
      if ((CurrPtr[0] == '_') && (CurrPtr[1] == 'S') && (CurrPtr[2] == '3') && (CurrPtr[3] == '_')) {
     //   CurrPtr[0] = 'N';
      //  CurrPtr[1] = 'O';
      //  CurrPtr[2] = 'S';
      //  CurrPtr[3] = '3';
        break;
      }
      // for iasl compiler version 20040527
      if ((CurrPtr[0] == '\\') && (CurrPtr[1] == '_') && (CurrPtr[2] == 'S') && (CurrPtr[3] == '3')) {
    //    CurrPtr[1] = 'N';
    //    CurrPtr[2] = 'O';
    //    CurrPtr[3] = 'S';
     //   CurrPtr[4] = '3';
        break;
      }
    }
    CurrPtr++;
  }

  //
  // Loop through the ASL looking for values that we must fix up.
  //
  CurrPtr = (UINT8 *) TableHeader;
  for (DsdtPointer = CurrPtr; DsdtPointer <= (CurrPtr + ((EFI_ACPI_COMMON_HEADER *) CurrPtr)->Length); DsdtPointer++) {
    Signature = (UINT32 *) DsdtPointer;

    switch (*Signature) {
    //
    // GNVS operation region
    //
    case (SIGNATURE_32 ('G', 'N', 'V', 'S')):
      //
      // Conditional match.  For Region Objects, the Operator will always be the
      // byte immediately before the specific name.  Therefore, subtract 1 to check
      // the Operator.
      //
//[-start-160801-IB07220121-modify]//
      Operation = (UINT16*)(DsdtPointer - 2);
//[-end-160801-IB07220121-modify]//
      if (*Operation == AML_OPREGION_OP) {
        Address   = (UINT32 *) (DsdtPointer + 6);
        *Address  = (UINT32) (UINTN) mGlobalNvsArea.Area;
        Size      = (UINT16 *) (DsdtPointer + 11);
        *Size     = sizeof (EFI_GLOBAL_NVS_AREA);
      }
      break;
      
//[-start-160216-IB03090424-add]//
    case SIGNATURE_32 ('B', 'P', 'R', 'W'):
      //
      // For BXT-M B-stepping Change 'B' in 'BPRW' to '_' 
      //
      if (BxtStepping() >= BxtB0) {
        DsdtPointer[0] = '_';
      }
      
      break;
//[-end-160216-IB03090424-add]//
//[-start-161027-IB07400806-add]//
//    //
//    // _S3_ method
//    //
//    case (SIGNATURE_32 ('_', 'S', '3', '_')):
//      Operation = DsdtPointer - 1;
//      if (*Operation == AML_NAME_OP) {
//        //
//        // Check if we want _S3 enabled
//        //
//        if (mSystemConfiguration->AcpiS3Support == 0) {
//          DEBUG ((EFI_D_ERROR, "Disable ACPI S3 support!!\n"));
//          Signature   = (UINT32 *) DsdtPointer;
//          *Signature  = SIGNATURE_32 ('X', 'S', '3', 'X');
//        }
//      }
//      break;
//      
//    //
//    // _S4_ method
//    //
//    case (SIGNATURE_32 ('_', 'S', '4', '_')):
//      Operation = DsdtPointer - 1;
//      if (*Operation == AML_NAME_OP) {
//        //
//        // Check if we want _S4 enabled
//        //
//        if ((mSystemConfiguration->AcpiS4Support == 0)) {
//          DEBUG ((EFI_D_ERROR, "Disable ACPI S4 support!!\n"));
//          Signature   = (UINT32 *) DsdtPointer;
//          *Signature  = SIGNATURE_32 ('X', 'S', '4', 'X');
//        }
//      }
//      break;
    //
    // _AC0 method
    //
    case (SIGNATURE_32 ('_', 'A', 'C', '0')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {

        //
        // Check if we want _AC0 enabled
        //
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->ActiveTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'A', 'C', '0');
        }
      }
      break;
    //
    // _AL0 method
    //
    case (SIGNATURE_32 ('_', 'A', 'L', '0')):
      Operation8 = (UINT8*)(DsdtPointer - 1);
      if (*Operation8 == AML_NAME_OP) {
        //
        // Check if we want _AL0 enabled
        //
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->ActiveTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'A', 'L', '0');
        }
      }
      break;
    //
    // _CRT method
    //
    case (SIGNATURE_32 ('_', 'C', 'R', 'T')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        //
        // Check if we want _CRT enabled
        //
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->CriticalThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'C', 'R', 'T');
        }
      }
      break;
    //
    // _HOT method
    //
    case (SIGNATURE_32 ('_', 'H', 'O', 'T')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        //
        // Check if we want _CRT enabled
        //
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->CriticalThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'H', 'O', 'T');
        }
      }
      break;
    //
    // _PSV method
    //
    case (SIGNATURE_32 ('_', 'P', 'S', 'V')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        //
        // Check if we want _PSV enabled
        //
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->PassiveThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'P', 'S', 'V');
        }
      }
      break;
    //
    // _PSL method
    //
    case (SIGNATURE_32 ('_', 'P', 'S', 'L')):
      Operation8 = (UINT8*)(DsdtPointer - 3);
      if (*Operation8 == AML_METHOD_OP) {
        //
        // Check if we want _PSL enabled
        //
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->PassiveThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'P', 'S', 'L');
        }
      }
      break;
    //
    // _TC1 method
    //
    case (SIGNATURE_32 ('_', 'T', 'C', '1')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->PassiveThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'T', 'C', '1');
        }
      }
      break;
    //
    // _TC2 method
    //
    case (SIGNATURE_32 ('_', 'T', 'C', '2')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->PassiveThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'T', 'C', '2');
        }
      }
      break;
    //
    // _TSP method
    //
    case (SIGNATURE_32 ('_', 'T', 'S', 'P')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->PassiveThermalTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'T', 'S', 'P');
        }
      }
      break;
    //
    // _SCP method
    //
    case (SIGNATURE_32 ('_', 'S', 'C', 'P')):
      Operation8 = (UINT8*)(DsdtPointer - 2);
      if (*Operation8 == AML_METHOD_OP) {
        if ((mSystemConfiguration->AutoThermalReporting == 0) && 
            (mSystemConfiguration->PassiveThermalTripPoint == 0xFF) && 
            (mSystemConfiguration->ActiveTripPoint == 0xFF)) {
          Signature   = (UINT32 *) DsdtPointer;
          *Signature  = SIGNATURE_32 ('X', 'S', 'C', 'P');
        }
      }
      break;
//[-end-161027-IB07400806-add]//
//[PRJ]+ >>>> Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
        case (SIGNATURE_32 ('C', 'O', 'M', 'P')):
          Operation = (UINT16*)(DsdtPointer - 2);
          if (*Operation == AML_OPREGION_OP) {
            Address   = (UINT32 *) (DsdtPointer + 6);
            *Address  = (UINT32) (UINTN) CompalGlobalNvsProtocol->CompalArea;
            Size      = (UINT16 *) (DsdtPointer + 11);
            *Size     = sizeof (COMPAL_GLOBAL_NVS_AREA );
          }
          break;
//[PRJ]+ <<<< Modify for support VirtualEEPROMVerifyTool and CMFCVerify  		  
    default:
      break;
    } // switch
  } // for
  return EFI_SUCCESS;
}

