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

*/

#include "CompalPlatformHookDxe.h"

EFI_STATUS
CompalUpdateEcEEPROMToBIOSRom (
    VOID
)
{
    EFI_STATUS                  Status;
//    FLASH_DEVICE                *FlashOperationPtr;
    UINT8                       *TmpData, *OemEepromBuffer;
    UINTN                       EepromBufferSize;
    UINT8                       EcDataBuf[3];

// Start - Add Common Intel Region Protect
//    Status = OemSpiFlashLock (SPI_WRITE_LOCK, FLASH_PROTECT_RANGE);
// End - Add Common Intel Region Protect

    //
    // Check OEM_EEPROM_MODIFY_FLAG
    //
    if ( CompalECEepromReadByte(OEM_EEPROM_MODIFY_FLAG) == 0xAA ) {

        EnableFvbWrites (TRUE);

        EepromBufferSize = OEM_EEPROM_DATA_LENGTH; //2K
        Status = (gBS->AllocatePool) (
                     EfiACPIMemoryNVS,
                     EepromBufferSize,
                     &OemEepromBuffer
                 );

        if ( EFI_ERROR (Status) ) {
            return Status;
        }

        ZeroMem (OemEepromBuffer, EepromBufferSize);

        // the new command set C52/DA6 to get EC EEPROM offset
        EcDataBuf[0] = EC_CMD_GET_EC_INFORMATION_EEPROM_ADDRESS;
        CompalECReadCmd(EC_MEMORY_MAPPING_PORT, EC_CMD_GET_EC_INFORMATION, EC_CMD_GET_EC_INFORMATION_EEPROM_ADDRESS_DATA_LEN, 0x03, EcDataBuf);

        if(EcDataBuf[0] == 0x00 && EcDataBuf[1] == 0x00 && EcDataBuf[2] == 0x00) {
            TmpData = (UINT8 *)(UINTN)(60 * EepromBufferSize); // 120K        
        } else {
            TmpData = (UINT8 *)(UINTN)((EcDataBuf[0] << 16) + (EcDataBuf[1] << 8) + EcDataBuf[2]); // 12xK
        }

        CompalECWait (TRUE);
        CompalEcReadSPIData (0,OemEepromBuffer,TmpData,EepromBufferSize);
        CompalECWait (FALSE);

        Status = FlashErase ((UINTN) FixedPcdGet32 (PcdFlashOemEepromBase), (UINTN) FixedPcdGet32 (PcdFlashOemEepromSize));
        if ( EFI_ERROR (Status) ) {
            return Status;
        }

        Status = FlashProgram((UINT8 *)(UINTN)FixedPcdGet32 (PcdFlashOemEepromBase),
                               &OemEepromBuffer[0], &EepromBufferSize, (UINTN)FixedPcdGet32 (PcdFlashOemEepromBase));
        if ( EFI_ERROR (Status) ) {
            return Status;
        }

        CompalECEepromWriteByte(OEM_EEPROM_MODIFY_FLAG,0x55);    // Clear EEPROM modify flag
        FreePool (OemEepromBuffer);

        EnableFvbWrites (FALSE);
    }

    return EFI_SUCCESS;
}

EFI_STATUS
CompalPlatformHookDxeEntryPoint (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
    EFI_STATUS                        Status;
    COMPAL_EEPROM_DXE_PROTOCOL        *CompalEEPROMDxeProtocol;

    //
    // Initialize the CompalEepromDxeProtocol Base Address
    //
    Status = gBS->LocateProtocol (&gCompalEEPROMDxeProtocolGuid, NULL, &CompalEEPROMDxeProtocol);
    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    CompalEEPROMDxeProtocol->CompalEepromDxeBaseAddr = (UINTN) FixedPcdGet32 (PcdFlashOemEepromBase);

    //
    // Update Ec EEPROM to BIOS ROM
    //
    Status = CompalUpdateEcEEPROMToBIOSRom();
    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    return Status;
}
