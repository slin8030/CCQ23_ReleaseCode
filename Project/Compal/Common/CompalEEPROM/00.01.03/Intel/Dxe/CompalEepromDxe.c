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
 1.01                Stan         Init version.
 1.02      06/01/16  Howard_Ding  Add Variable EEPROM feature.
 1.03      04/08/16  Vanessa      Fix "PcdVEEPROMTYPE" pcd can't entry binary module.
 1.04      22/09/16  Thomas       Reduce VEEPROM PcdVEEPROMTYPE times when VEEPROM is accessed. 
*/

#include "ProjectDefinition.h"
#include "CompalDXEVarEeprom.h"
#include "CompalEepromDxe.h"
#include <CompalEEPROMDxeProtocol.h>

// ----------------------------------------------------------------------------------------------------------
// Please read this before you modify EEPROM code.
//
// EEPROM Variable feature -
// For CompalEepromDxeRead and CompalEepromDxeWrite, NO MORE SUPPORT BIOS EEPROM ROM block when Variable
// EEPROM feature enable (gCompalCommonCodeGuid.PcdVEEPROMTYPE = 2). When read/write EEPROM bank 7, send EC command
// directly.
//
// Non-EC Variable support -
// If need to support Non-EC EEPROM feature, please override gCompalCommonCodeGuid.PcdVEEPROMTYPE = 3 and  
// This feature will stop to send command(Access EC EEPROM) to EC, also
// EEPROM bank 7 was not accessed anymore.
//
// Please choose the project VEEPROM support flag in CompalCommon.dec 
//
//   gCompalCommonCodeGuid.PcdVEEPROMTYPE|0x0|UINT8|0x00000019               # VEEPROM Type
//                                                                           # 0 : Not Support (Access EC EEPROM)
//                                                                           # 1 : ROM VEEPROM (EC EEPROM with BIOS ROM Mapping)
//                                                                           # 2 : BIOS Variable EEPROM (BIOS variable bank0~7)
//                                                                           # 3 : Non-EC VEEPROM (BIOS variable bank0~6)

COMPAL_EEPROM_DXE_PROTOCOL         *CompalEepromDxeProtocol;

EFI_STATUS
CompalEepromDxeRead (
    IN  COMPAL_EEPROM_DXE_PROTOCOL    *This,
    IN  UINT16                        OemEepromOffset,
    IN  UINT16                        OemEepromSize,
    IN  UINT8                         OemEepromReadPolicy,
    OUT UINT8                         *OemEepromDataBuffer
)
/*++

Routine Description:


 Arguments:
    This                     Pointer to COMPAL_EEPROM_DXE_PROTOCOL
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromReadPolicy      Policy choice for read EEPROM
    OemEepromDataBuffer      Data buffer

Returns:

--*/
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{

    UINT8     BlockValid;
    UINT8     EepromType;
    UINT16    EepromIndex;  
    UINT32    OemEepromBaseAddr;

    OemEepromBaseAddr = This->CompalEepromDxeBaseAddr;
    EepromType = PcdGetEx8(&gCompalCommonCodeGuid, PcdVEEPROMTYPE);

    switch (OemEepromReadPolicy) {

    case OEM_EEPROM_READ_FROM_EC:
        
      if (EepromType == VARIABLE_VEEPROM || EepromType == NONEC_VEEPROM){
       //------------------------------------ 
       // For BIOS Virtual EEPROM access.     
       //------------------------------------ 
        if (OemEepromOffset < 0x700) {
            CompalDXEVarEEPROMRead(OemEepromOffset, OemEepromSize, OemEepromDataBuffer);
        }
        else if (EepromType == VARIABLE_VEEPROM)
        {
            for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
                  *OemEepromDataBuffer = CompalECEepromReadByte ( (UINT16) OemEepromOffset + EepromIndex);
                 OemEepromDataBuffer++;
            }
        }  // end of Vareeprom checked
      }else{
       //------------------------------------ 
       // For Normal EC command access.     
       //------------------------------------ 
            
        for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
          *OemEepromDataBuffer = CompalECEepromReadByte ( (UINT16) OemEepromOffset + EepromIndex);
           OemEepromDataBuffer++;
        }
      }
        break;

    case OEM_EEPROM_READ_FROM_BIOS_ROM:
      if (EepromType == VARIABLE_VEEPROM || EepromType == NONEC_VEEPROM){
       //------------------------------------ 
       // For BIOS Virtual EEPROM access.     
       //------------------------------------ 
        if (OemEepromOffset < 0x700) {
            CompalDXEVarEEPROMRead(OemEepromOffset, OemEepromSize, OemEepromDataBuffer);
        }
      }        
      else{
       //------------------------------------ 
       // For ROM block access.     
       //------------------------------------ 
        for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
            *OemEepromDataBuffer = *((volatile UINT8*)((UINTN)(OemEepromBaseAddr + OemEepromOffset + EepromIndex)));
            OemEepromDataBuffer++;
        }
      }          
        break;
        //
        // Read directly from BIOS SPI ROM
        //
    default:
// (notice) Below program code should need to be removed from new platform as ChiefRiver/ENE9012,
// because in new EC platform it is only supported one block of 2KB EC EEPROM,
// so we do not need to check "BlockValid" flag anymore from now on, here is only for backward compatible.
        
      if (EepromType == ROM_VEEPROM){
       //------------------------------------ 
       // For ROM block access.     
       //------------------------------------ 
        BlockValid = *((volatile UINT8*)((UINTN)(OemEepromBaseAddr + OEM_EEPROM_OFFSET_SWTBANK_VAILD_FLAG)));

        if ( BlockValid != OEM_EEPROM_SWTBANK_VALID_FLAG ) {
            for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
                *OemEepromDataBuffer = *((volatile UINT8*)((UINTN)(OemEepromBaseAddr + OEM_EEPROM_SWTBANK_LENGTH + OemEepromOffset + EepromIndex)));
                OemEepromDataBuffer++;
            }
        } else {
            for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
                *OemEepromDataBuffer = *((volatile UINT8*)((UINTN)(OemEepromBaseAddr + OemEepromOffset + EepromIndex)));
                OemEepromDataBuffer++;
            }
        }
      }
        
        break;
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CompalEepromDxeWrite (
    IN COMPAL_EEPROM_DXE_PROTOCOL     *This,
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

    UINT8               EepromType;
    UINT16              EepromIndex;

    EepromType = PcdGetEx8(&gCompalCommonCodeGuid, PcdVEEPROMTYPE);
  
  if (EepromType == VARIABLE_VEEPROM || EepromType == NONEC_VEEPROM){
    //------------------------------------ 
    // For BIOS Virtual EEPROM access.     
    //------------------------------------ 
    if (OemEepromOffset < 0x700) {
        CompalDXEVarEEPROMWrite (OemEepromOffset, OemEepromSize, OemEepromDataBuffer);
    }
    else if(EepromType == VARIABLE_VEEPROM)
    {
       for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
            CompalECEepromWriteByte((UINT16) OemEepromOffset + EepromIndex, *OemEepromDataBuffer);
            OemEepromDataBuffer++;
       }
    }  // end of Vareeprom checked
  }
  else
  {
    //------------------------------------ 
    // For Normal EC command access.     
    //------------------------------------ 
    
    for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
        CompalECEepromWriteByte((UINT16) OemEepromOffset + EepromIndex, *OemEepromDataBuffer);
        OemEepromDataBuffer++;
    }

    CompalECEepromWriteByte(OEM_EEPROM_MODIFY_FLAG, OEM_EEPROM_BE_MODIFIED_FLAG); // Set EEPROM be modified flag
  }
    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CompalEepromDxeEntryPoint (
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
/*++

Routine Description:

  Initialize the Compal Eeprom Dxe protocol

Arguments:

  ImageHandle       ImageHandle of the loaded driver
  SystemTable       Pointer to the System Table

Returns:

  Status

  EFI_SUCCESS           - thread can be successfully created
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure
  EFI_DEVICE_ERROR      - cannot create the timer service

--*/
{
    UINT8               EepromType;
    EFI_STATUS          Status;
    
    //
    // Allocate pool for CompalEepromDxeProtocol
    //
    CompalEepromDxeProtocol = AllocateZeroPool (sizeof (COMPAL_EEPROM_DXE_PROTOCOL));
    if (CompalEepromDxeProtocol == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    EepromType = PcdGetEx8(&gCompalCommonCodeGuid, PcdVEEPROMTYPE);
    if (EepromType == VARIABLE_VEEPROM || EepromType == NONEC_VEEPROM){
    CompalVarEEprominital ();                   // Initial Variable EEPROM.
    }
    
    //
    // Initialize the CompalEepromDxeProtocol
    //
    //------------------------------------ 
    // For ROM block access.     
    //------------------------------------ 
    CompalEepromDxeProtocol->CompalEepromDxeBaseAddr = (UINTN) FixedPcdGet32 (PcdFlashOemEepromBase);   
    CompalEepromDxeProtocol->CompalEepromDxeRead = CompalEepromDxeRead;
    CompalEepromDxeProtocol->CompalEepromDxeWrite = CompalEepromDxeWrite;

    //
    // Install the CompalEepromDxeProtocol
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gCompalEEPROMDxeProtocolGuid,
                 CompalEepromDxeProtocol,
                 NULL
             );

    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    return EFI_SUCCESS;
}
