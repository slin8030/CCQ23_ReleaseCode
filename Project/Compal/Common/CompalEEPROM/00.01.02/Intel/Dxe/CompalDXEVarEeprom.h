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

#ifndef _COMPAL_VAREEPROM_DXE_H_
#define _COMPAL_VAREEPROM_DXE_H_

#include <CompalEEPROMDxeProtocol.h>
#include <CompalEepromDefinition.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <CompalOemeeprom.h>

#define  WriteVEEPROM    1
#define  ReadVEEPROM     0

#define NOT_SUPPORT         0                   // Not Support (Access EC EEPROM)
#define ROM_VEEPROM         1                   // ROM VEEPROM (EC EEPROM with BIOS ROM Mapping) 
#define VARIABLE_VEEPROM    2                   // BIOS Variable EEPROM (BIOS variable bank0~7)
#define NONEC_VEEPROM       3                   // Non-EC VEEPROM (BIOS variable bank0~6)

extern EFI_BOOT_SERVICES  *gBS;
extern EFI_RUNTIME_SERVICES  *gRT;
 
#pragma pack (1)
typedef struct {
  EFI_GUID                              VendorGuid;
  CHAR16                                *VariableName;
} OEM_VEEPROM_VARIABLE_TABLE;

#pragma pack ()
/*++

Routine Description for ProjectEEPROMDxeRead:

 Arguments:
    This                     Pointer to COMPAL_VarEEPROM_DXE_PROTOCOL
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromReadPolicy      N/A (non-used)
    OemEepromDataBuffer      Data buffer

Returns:

  EFI_SUCCESS       - Command success
  EFI_DEVICE_ERROR  - Command error

--*/
EFI_STATUS
CompalDXEVarEEPROMRead (
//    IN  COMPAL_EEPROM_DXE_PROTOCOL    *This,
    IN  UINT16                        OemEepromOffset,
    IN  UINT16                        OemEepromSize,
    OUT UINT8                         *OemEepromDataBuffer
);

/*++

Routine Description for ProjectkEEPROMDxeWrite:


Arguments:
    This                     Pointer to COMPAL_EEPROM_DXE_PROTOCOL
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromDataBuffer      Data buffer

Returns:

  EFI_SUCESS

--*/
EFI_STATUS             
CompalDXEVarEEPROMWrite (
//    IN COMPAL_EEPROM_DXE_PROTOCOL     *This,
    IN UINT16                         OemEepromOffset,
    IN UINT16                         OemEepromSize,
    IN UINT8                          *OemEepromDataBuffer
);

EFI_STATUS 
CompalVeePROMRWFun (
    IN  UINT16                                BanckNo,
    IN  UINT8                                 RWSelection,
    IN  UINT16                                OemEepromOffset,
    IN  UINT16                                OemEepromSize,
    IN  OEMDRIVER_SAMPLE_CONFIGURATION0       *OEMConfiguration,
    IN  OUT     UINT8                         *OemEepromDataBuffer 
);

EFI_STATUS
CompalVarEEprominital
(
  VOID
);

#endif
