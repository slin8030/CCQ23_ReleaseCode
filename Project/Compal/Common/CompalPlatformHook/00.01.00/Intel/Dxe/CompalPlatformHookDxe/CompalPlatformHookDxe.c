
#include "CompalPlatformHookDxe.h"

const COMPAL_PLATFORM_HOOK_DXE_PROTOCOL CompalOverrideInstance = {
    ProjectPlatformHookDxeMain
};


//
// Functions
//
EFI_STATUS
EFIAPI
ProjectPlatformHookDxeMain (
    IN     COMPAL_PLATFORM_HOOK_DXE_PROTOCOL            * This,
    IN     COMPAL_PLATFORM_HOOK_DXE_OVERRIDE_SETTING    OverrideType,
    ...
)
{
    EFI_STATUS                            Status = EFI_SUCCESS;
    VA_LIST                               Marker;
    PCH_PLATFORM_POLICY_PROTOCOL          *mPchPolicyData;
    SYSTEM_CONFIGURATION                  *TmpSysTemConfig;

    switch (OverrideType) {
    case COMPAL_PCH_POLICY_OVERRIDE:
        VA_START(Marker, OverrideType);
        mPchPolicyData = VA_ARG (Marker, PCH_PLATFORM_POLICY_PROTOCOL *);
        TmpSysTemConfig = VA_ARG (Marker, SYSTEM_CONFIGURATION *);
        CompalPCHPolicyDxeOverride (mPchPolicyData, TmpSysTemConfig);
        VA_END(Marker);
        break;

    default:
        break;
    }

    return Status;
}

EFI_STATUS
UpdateEcEEPROMToBIOSRom (
    VOID
)
{
    EFI_STATUS                  Status;
//    FLASH_DEVICE                *FlashOperationPtr;
    UINT8                       *TmpData, *OemEepromBuffer;
    UINTN                       EepromBufferSize;
    UINT8                       EcDataBuf[3];

    EnableFvbWrites (TRUE);

// Start - Add Common Intel Region Protect
    Status = OemSpiFlashLock (SPI_WRITE_LOCK, FLASH_PROTECT_RANGE);
// End - Add Common Intel Region Protect

    //
    // Check OEM_EEPROM_MODIFY_FLAG
    //
    if ( CompalECEepromReadByte(OEM_EEPROM_MODIFY_FLAG) == 0xAA ) {

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
    }

    EnableFvbWrites (FALSE);

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
    gBS->LocateProtocol (&gCompalEEPROMDxeProtocolGuid, NULL, &CompalEEPROMDxeProtocol);
    CompalEEPROMDxeProtocol->CompalEepromDxeBaseAddr = (UINTN) FixedPcdGet32 (PcdFlashOemEepromBase);

    //
    // Update Ec EEPROM to BIOS ROM
    //
    Status = UpdateEcEEPROMToBIOSRom();
    if ( EFI_ERROR (Status) ) {
        return Status;
    }

    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gCompalPlatformHookDxeProtocolGuid,
                 &CompalOverrideInstance,
                 NULL
             );

    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    return Status;
}
