/*
 * (C) Copyright 2011-2012 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.02      06/01/16  Howard_Ding  Add Variable EEPROM feature.
*/

#include "CompalSMMVarEeprom.h"
#include <CompalOemeeprom.h>
#include <Protocol/SmmVariable.h>


extern EFI_GUID gEfiSmmVariableProtocolGuid;

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
  EFI_SMM_VARIABLE_PROTOCOL    *SmmVariable;
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
  Status = gSmst->SmmLocateProtocol (
                    &gEfiSmmVariableProtocolGuid,
                    NULL,
                    (VOID **)&SmmVariable
                    );
  
/*
  +-------------------------------------+
  | RWSelection = 0 for Read VEEPROM    |
  +-------------------------------------+
*/

  if(RWSelection == 0 ){
          Status = SmmVariable->SmmGetVariable (OEM_VEEPROM_Table[BanckNo].VariableName, &(OEM_VEEPROM_Table[BanckNo].VendorGuid), NULL, &BufferSize, OEMConfiguration);
          if(EFI_ERROR(Status)) {
            return Status;
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
          Status = SmmVariable->SmmGetVariable (OEM_VEEPROM_Table[BanckNo].VariableName, &(OEM_VEEPROM_Table[BanckNo].VendorGuid), NULL, &BufferSize, OEMConfiguration);
          for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
              OEMConfiguration->EEPROM[EepromOffset+ EepromIndex]=*OemEepromDataBuffer;
              OemEepromDataBuffer++;
           }
           SmmVariable->SmmSetVariable(OEM_VEEPROM_Table[BanckNo].VariableName, &(OEM_VEEPROM_Table[BanckNo].VendorGuid),
           EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, BufferSize, OEMConfiguration);

  }
  return Status;
}


EFI_STATUS
CompalSMMVarEEPROMRead (
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
//[PRJ]++ >>>> Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
  FreePool(OEMConfiguration);
//[PRJ]++ <<<< Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
    return EFI_SUCCESS;
}

EFI_STATUS
CompalSMMVarEEPROMWrite (
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
//[PRJ]++ >>>> Modify for support VirtualEEPROMVerifyTool and CMFCVerify   
  FreePool (OEMConfiguration);
//[PRJ]++ <<<< Modify for support VirtualEEPROMVerifyTool and CMFCVerify  
  return EFI_SUCCESS;
}


