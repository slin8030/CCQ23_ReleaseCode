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

#include <CompalEepromSmm.h>
#include <CompalEEPROMSmmProtocol.h>

EFI_SMM_SYSTEM_TABLE2              *mSmst;
COMPAL_EEPROM_SMM_PROTOCOL         *CompalEepromSmmProtocol;

EFI_STATUS
CompalEepromSmmRead (
    IN  COMPAL_EEPROM_SMM_PROTOCOL    *This,
    IN  UINT16                        OemEepromOffset,
    IN  UINT16                        OemEepromSize,
    IN  UINT8                         OemEepromReadPolicy,
    OUT UINT8                         *OemEepromDataBuffer
)
{
    UINT8     BlockValid;
    UINT16    EepromIndex;
    UINT32    OemEepromBaseAddr;

    OemEepromBaseAddr = This->CompalEepromSmmBaseAddr;

    switch (OemEepromReadPolicy) {

    case OEM_EEPROM_READ_FROM_EC:
      //#if (CompilerIBV == InsydeCode)
      //  SaveRestoreKbc(SAVE_KBC_STATE);
      //#endif
      
        for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
            *OemEepromDataBuffer = CompalECEepromReadByte ( (UINT8) OemEepromOffset + EepromIndex);
            OemEepromDataBuffer++;
        }
      //#if (CompilerIBV == InsydeCode)
      //  SaveRestoreKbc(RESTORE_KBC_STATE);
      //#endif
        break;

    case OEM_EEPROM_READ_FROM_BIOS_ROM:
        for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
            *OemEepromDataBuffer = *((volatile UINT8*)((UINTN)(OemEepromBaseAddr + OemEepromOffset + EepromIndex)));
            OemEepromDataBuffer++;
        }
        break;
        //
        // Read directly from BIOS SPI ROM
        //
    default:
// (notice) Below program code should need to be removed from new platform as ChiefRiver/ENE9012,
// because in new EC platform it is only supported one block of 2KB EC EEPROM,
// so we do not need to check "BlockValid" flag anymore from now on, here is only for backward compatible.
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
        break;
    }

    return EFI_SUCCESS;
}

EFI_STATUS
CompalEepromSmmWrite (
    IN COMPAL_EEPROM_SMM_PROTOCOL     * This,
    IN UINT16                         OemEepromOffset,
    IN UINT16                         OemEepromSize,
    IN UINT8                          *OemEepromDataBuffer
)
/*++

Routine Description:


Arguments:
    This                     Pointer to COMPAL_EEPROM_SMM_PROTOCOL
    OemEepromOffset          Pointer to address of data
    OemEepromSize            Data size
    OemEepromDataBuffer      Data buffer

Returns:

  EFI_SUCESS

--*/
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
    UINT16                                EepromIndex;

    for ( EepromIndex = 0 ; EepromIndex < OemEepromSize ; EepromIndex++ ) {
        CompalECEepromWriteByte((UINT8) OemEepromOffset + EepromIndex, *OemEepromDataBuffer);
        OemEepromDataBuffer++;
    }

    CompalECEepromWriteByte(OEM_EEPROM_MODIFY_FLAG, OEM_EEPROM_BE_MODIFIED_FLAG); // Set EEPROM be modified flag

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CompalEepromSmmEntryPoint (
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
/*++

Routine Description:

  Initialize the oem eeprom protocol

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
    EFI_STATUS                            Status;
    EFI_SMM_BASE2_PROTOCOL                *mSmmBase;
    BOOLEAN                               InSmm;

    InSmm        = FALSE;
    mSmmBase      = NULL;
    Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **)&mSmmBase
                  );
    if (!EFI_ERROR (Status)) {
      mSmmBase->InSmm (mSmmBase, &InSmm);
    }

    if (InSmm) {
        //
        // Great!  We're now in SMM!
        //
        //
        mSmmBase->GetSmstLocation (mSmmBase, &mSmst);

        //
        // Allocate SMM Pool for CompalEepromSmmProtocol
        //
        Status = mSmst->SmmAllocatePool (
                     EfiRuntimeServicesData,
                     sizeof (COMPAL_EEPROM_SMM_PROTOCOL),
                     &CompalEepromSmmProtocol
                 );

        if ( EFI_ERROR (Status) ) {
            return Status;
    	}
    }

    //
    // Initialize the CompalEepromSmmProtocol
    //
    CompalEepromSmmProtocol->CompalEepromSmmBaseAddr = (UINTN) FixedPcdGet32 (PcdFlashOemEepromBase);
    CompalEepromSmmProtocol->CompalEepromSmmRead = CompalEepromSmmRead;
    CompalEepromSmmProtocol->CompalEepromSmmWrite = CompalEepromSmmWrite;

    //
    // Install the CompalEepromSmmProtocol
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gCompalEEPROMSmmProtocolGuid,
                 CompalEepromSmmProtocol,
                 NULL
             );

    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    return EFI_SUCCESS;
}
