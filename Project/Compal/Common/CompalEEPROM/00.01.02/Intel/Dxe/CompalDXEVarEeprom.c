/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00      06/01/16  Howard_Ding  Add Variable EEPROM feature.
*/

#include "CompalDXEVarEeprom.h"
#include <CompalOemeeprom.h>
#include <CompalEEPROMDxeProtocol.h>

COMPAL_EEPROM_DXE_PROTOCOL         *CompalEepromDxeProtocol;
EFI_GUID                            mOemFormSetGuid0 = CompalVariableBank0_GUID;
EFI_GUID                            mOemFormSetGuid1 = CompalVariableBank1_GUID;
EFI_GUID                            mOemFormSetGuid2 = CompalVariableBank2_GUID;
EFI_GUID                            mOemFormSetGuid3 = CompalVariableBank3_GUID;
EFI_GUID                            mOemFormSetGuid4 = CompalVariableBank4_GUID;
EFI_GUID                            mOemFormSetGuid5 = CompalVariableBank5_GUID;
EFI_GUID                            mOemFormSetGuid6 = CompalVariableBank6_GUID;

EFI_STATUS
CompalVarEEprominital
(
  VOID
)
{	
  EFI_STATUS Status;
  OEMDRIVER_SAMPLE_CONFIGURATION0     *OEMConfiguration0;
  OEMDRIVER_SAMPLE_CONFIGURATION1     *OEMConfiguration1;
  OEMDRIVER_SAMPLE_CONFIGURATION2     *OEMConfiguration2;
  OEMDRIVER_SAMPLE_CONFIGURATION3     *OEMConfiguration3;
  OEMDRIVER_SAMPLE_CONFIGURATION4     *OEMConfiguration4;
  OEMDRIVER_SAMPLE_CONFIGURATION5     *OEMConfiguration5;
  OEMDRIVER_SAMPLE_CONFIGURATION6     *OEMConfiguration6;
  UINTN             BufferSize;

	
 //SET OEM0
  OEMConfiguration0 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0);
  Status = gRT->GetVariable (OEMSettingVariableName0, &mOemFormSetGuid0, NULL, &BufferSize, OEMConfiguration0);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION0)) {
    FreePool(OEMConfiguration0);
    OEMConfiguration0 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName0, &mOemFormSetGuid0,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0), OEMConfiguration0);
  }
  //SET OEM1
   OEMConfiguration1 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION1));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION1);
  Status = gRT->GetVariable (OEMSettingVariableName1, &mOemFormSetGuid1, NULL, &BufferSize, OEMConfiguration1);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION1)) {
    FreePool(OEMConfiguration1);
    OEMConfiguration1 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName1, &mOemFormSetGuid1,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION1), OEMConfiguration1);
  }
  //SET OEM2
   OEMConfiguration2 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION2));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION2);
  Status = gRT->GetVariable (OEMSettingVariableName2, &mOemFormSetGuid2, NULL, &BufferSize, OEMConfiguration2);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION2)) {
    FreePool(OEMConfiguration2);
    OEMConfiguration2 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName2, &mOemFormSetGuid2,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION2), OEMConfiguration2);
  }
  //SET OEM3
    OEMConfiguration3 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION3));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION3);
  Status = gRT->GetVariable (OEMSettingVariableName3, &mOemFormSetGuid3, NULL, &BufferSize, OEMConfiguration3);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION3)) {
    FreePool(OEMConfiguration3);
    OEMConfiguration3 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName3, &mOemFormSetGuid3,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION3), OEMConfiguration3);
  }
  //SET OEM4
  OEMConfiguration4 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION4));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION4);
  Status = gRT->GetVariable (OEMSettingVariableName4, &mOemFormSetGuid4, NULL, &BufferSize, OEMConfiguration4);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION4)) {
    FreePool(OEMConfiguration4);
    OEMConfiguration4 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName4, &mOemFormSetGuid4,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION4), OEMConfiguration4);
  }
  //SET OEM5
  OEMConfiguration5 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION5));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION5);
  Status = gRT->GetVariable (OEMSettingVariableName5, &mOemFormSetGuid5, NULL, &BufferSize, OEMConfiguration5);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION5)) {
    FreePool(OEMConfiguration5);
    OEMConfiguration5 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName5, &mOemFormSetGuid5,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION5), OEMConfiguration5);
  }
  //SET OEM6
  OEMConfiguration6 = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION6));
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION6);
  Status = gRT->GetVariable (OEMSettingVariableName6, &mOemFormSetGuid6, NULL, &BufferSize, OEMConfiguration6);

  if (BufferSize != sizeof(OEMDRIVER_SAMPLE_CONFIGURATION6)) {
    FreePool(OEMConfiguration6);
    OEMConfiguration6 = NULL;
  }
  if(EFI_ERROR(Status)) {
    gRT->SetVariable(OEMSettingVariableName6, &mOemFormSetGuid6,
      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof (OEMDRIVER_SAMPLE_CONFIGURATION6), OEMConfiguration6);
  }

  return Status;
}


//Add the sub function for the duplication code

EFI_STATUS CompalVeePROMRWFun(
    IN  UINT16                                BanckNo,
    IN  UINT8                                 RWSelection,
    IN  UINT16                                OemEepromOffset,
    IN  UINT16                                OemEepromSize,
    IN  OEMDRIVER_SAMPLE_CONFIGURATION0       *OEMConfiguration,
    IN  OUT     UINT8                         *OemEepromDataBuffer 
)
/*
Routine Description:
 Arguments:
    RWSelection              Get Read or Write  0 is for Read, and 1 is for Write
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromDataBuffer      Data buffer

Returns:
*/
{
  UINTN           BufferSize;
  UINT16          EepromOffset;
  EFI_STATUS      Status;  
  UINT16          EepromIndex;
  OEM_VEEPROM_VARIABLE_TABLE OEM_VEEPROM_Table[] = {
      { CompalVariableBank0_GUID,OEMSettingVariableName0 },
      { CompalVariableBank1_GUID,OEMSettingVariableName1 },
      { CompalVariableBank2_GUID,OEMSettingVariableName2 },
      { CompalVariableBank3_GUID,OEMSettingVariableName3 },
      { CompalVariableBank4_GUID,OEMSettingVariableName4 },
      { CompalVariableBank5_GUID,OEMSettingVariableName5 },
      { CompalVariableBank6_GUID,OEMSettingVariableName6 },
  };
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0);
  EepromOffset = OemEepromOffset%((UINT16)BufferSize) ;
  
/*
  +-------------------------------------+
  | RWSelection = 0 for Read VEEPROM    |
  +-------------------------------------+
*/

  if(RWSelection == 0 ){
          Status = gRT->GetVariable (OEM_VEEPROM_Table[BanckNo].VariableName, &(OEM_VEEPROM_Table[BanckNo].VendorGuid), NULL, &BufferSize, OEMConfiguration);
          if(EFI_ERROR(Status)) {
          CompalVarEEprominital();
          }
          for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
              *OemEepromDataBuffer = OEMConfiguration->EEPROM[EepromOffset+ EepromIndex];
              OemEepromDataBuffer++;
          }
  }
/*
  +-------------------------------------+
  | RWSelection = 1 for Write VEEPROM   |
  +-------------------------------------+
*/  
  else{
          Status = gRT->GetVariable (OEM_VEEPROM_Table[BanckNo].VariableName, &(OEM_VEEPROM_Table[BanckNo].VendorGuid), NULL, &BufferSize, OEMConfiguration);
          for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
              OEMConfiguration->EEPROM[EepromOffset+ EepromIndex]=*OemEepromDataBuffer;
              OemEepromDataBuffer++;
           }
           gRT->SetVariable(OEM_VEEPROM_Table[BanckNo].VariableName, &(OEM_VEEPROM_Table[BanckNo].VendorGuid),
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, BufferSize, OEMConfiguration);

  }
  return Status;
}


EFI_STATUS
CompalDXEVarEEPROMRead (
//    IN  COMPAL_EEPROM_DXE_PROTOCOL    *This,
    IN  UINT16                        OemEepromOffset,
    IN  UINT16                        OemEepromSize,
    OUT UINT8                         *OemEepromDataBuffer
)
/*++

Routine Description:


 Arguments:
    This                     Pointer to COMPAL_EEPROM_DXE_PROTOCOL
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromDataBuffer      Data buffer

Returns:

--*/
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{	
  EFI_STATUS                            Status;
  OEMDRIVER_SAMPLE_CONFIGURATION0       *OEMConfiguration;  
  UINTN                                 BufferSize;  
  UINT16                                Bank;
  UINT16    				EepromOffset;
 
  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0);
  Bank = OemEepromOffset/((UINT16)BufferSize) ;
  EepromOffset = OemEepromOffset%((UINT16)BufferSize) ;
  OEMConfiguration = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0));
  Status = CompalVeePROMRWFun(Bank,ReadVEEPROM,OemEepromOffset,OemEepromSize,OEMConfiguration,OemEepromDataBuffer);

    return EFI_SUCCESS;
}

EFI_STATUS
CompalDXEVarEEPROMWrite (
//    IN COMPAL_EEPROM_DXE_PROTOCOL     *This,
    IN UINT16                         OemEepromOffset,
    IN UINT16                         OemEepromSize,
    IN UINT8                          *OemEepromDataBuffer
)
/*++

Routine Description:


Arguments:
    This                     Pointer to COMPAL_EEPROM_DXE_PROTOCOL
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromDataBuffer      Data buffer

Returns:

  EFI_SUCESS

--*/
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
	EFI_STATUS Status;
  OEMDRIVER_SAMPLE_CONFIGURATION0     *OEMConfiguration;
  UINTN                BufferSize;
  UINT16               Bank;
  UINT16    				   EepromOffset;

  BufferSize = sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0);
  Bank = OemEepromOffset/((UINT16)BufferSize) ;
  EepromOffset = OemEepromOffset%((UINT16)BufferSize) ;
  OEMConfiguration = AllocateZeroPool (sizeof (OEMDRIVER_SAMPLE_CONFIGURATION0));  
  Status = CompalVeePROMRWFun(Bank,WriteVEEPROM,OemEepromOffset,OemEepromSize,OEMConfiguration,OemEepromDataBuffer);

  return EFI_SUCCESS;
}


