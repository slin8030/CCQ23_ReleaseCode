#include "CompalISCT.h"

typedef struct _COMPAL_ISCT_INSTANCE {
    COMPAL_ISCT_PPI                Ppi;
    EFI_PEI_PPI_DESCRIPTOR         PpiDesc;
} COMPAL_ISCT_INSTANCE;

EFI_STATUS
EFIAPI
CompalISCTWakeReasonGet (
    IN COMPAL_ISCT_PPI                * This
)
/*++
Routine Description:
  Get ISCT Wake Reason from EC Name Space/PM1 Status.

Arguments:
  * This      - Pointer to COMPAL_ISCT_PPI

Returns:
  EFI_SUCCESS - Get ISCT Wake Reason success

--*/
{
    UINT8          EcISCTStatus;
    UINT16         Pm1Status;

    // Get PM1 Status for Wake Event check
    Pm1Status = IoRead16 ( ACPI_IO_BASE_ADDRESS + R_PCH_ACPI_PM1_STS );

    // Get EC ISCT Stauts
    EcISCTStatus = CompalECACPINVSReadByte(ISCT_STATUS_OFFSET);

    // Make sure ISCT enable by ISCT agent
    if (EcISCTStatus & ISCT_S0_ISCT_STATUS) {
        // Check if it is RTC wake
        if (Pm1Status & B_PCH_ACPI_PM1_STS_RTC) {
            // Set ISCT_STATUS bit3: ISCT_RTC_WAKE_STATUS
            EcISCTStatus |= ISCT_RTC_WAKE_STATUS;
        }
    }

    // Update EC ISCT Stauts
    CompalECACPINVSWriteByte(ISCT_STATUS_OFFSET, EcISCTStatus);

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CompalEcIFFSEntryNotify (
    IN COMPAL_ISCT_PPI                * This,
    IN BOOLEAN                         IFFSEntry
)
/*++
Routine Description:
  Notify EC to Enter/Exit IFFS Status.

Arguments:
  * This      - Pointer to COMPAL_ISCT_PPI
  IFFSEntry   - BOOLEAN to Enter/Exit IFFS Status

Returns:
  EFI_SUCCESS - Notify EC success

--*/
{
    UINT8          EcIFFSStatus;

    // Get EC IFFS Stauts
    EcIFFSStatus = CompalECACPINVSReadByte(IFFS_STATUS_OFFSET);

    // Notify EC to enter iFFS
    if (IFFSEntry == TRUE) {
        EcIFFSStatus |= IFFS_FUNCTION_STATUS;
    }

    // Notify EC to exit iFFS
    else {
        EcIFFSStatus &= ~(IFFS_FUNCTION_STATUS);
    }

    // Update EC IFFS Stauts
    CompalECACPINVSWriteByte(IFFS_STATUS_OFFSET, EcIFFSStatus);

    return EFI_SUCCESS;
}

EFI_STATUS
CompalISCTEntryPoint (
    IN  EFI_FFS_FILE_HEADER           *FfsHeader,
    IN  CONST EFI_PEI_SERVICES        **PeiServices
)
{
    COMPAL_ISCT_INSTANCE              *Instance;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI   *Variable;
    UINTN                             VariableSize = 0;
    SYSTEM_CONFIGURATION              SystemConfiguration;
    EFI_GUID                          SystemConfigurationGuid = SYSTEM_CONFIGURATION_GUID;
    UINT8                             EcISCTStatus;
    UINT8                             EcIFFSStatus;
    EFI_STATUS                        Status;

    Instance = (COMPAL_ISCT_INSTANCE *) AllocateZeroPool (sizeof (COMPAL_ISCT_INSTANCE));
    ASSERT (Instance != NULL);
    if (Instance == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    Instance->Ppi.CompalISCTWakeReasonGet = CompalISCTWakeReasonGet;
    Instance->Ppi.CompalEcIFFSEntryNotify = CompalEcIFFSEntryNotify;

    Instance->PpiDesc.Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    Instance->PpiDesc.Guid = &gCompalISCTPpiGuid;
    Instance->PpiDesc.Ppi = &Instance->Ppi;

    Status = PeiServicesInstallPpi (&Instance->PpiDesc);
    ASSERT_EFI_ERROR (Status);

    Variable = NULL;
    Status = PeiServicesLocatePpi (&gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, &Variable);
    VariableSize = sizeof ( SYSTEM_CONFIGURATION );

    // Get SystemConfiguration
    Status = Variable->GetVariable (
                 Variable,
                 L"Setup",
                 &SystemConfigurationGuid,
                 NULL,
                 &VariableSize,
                 &SystemConfiguration
             );

    // Check if ISCT enable in SCU
    if (SystemConfiguration.IsctConfiguration == 1) {
        // Get EC ISCT Stauts & Set ISCT_STATUS bit4 = 1 (ISCT enable)
        EcISCTStatus = CompalECACPINVSReadByte(ISCT_STATUS_OFFSET);
        EcISCTStatus |= ISCT_SUPPORT_ENABLE;

        // Exit S0-ISCT when System not wake from EC for ISCT
        if ((EcISCTStatus & ISCT_EC_WAKE_STATUS) != ISCT_EC_WAKE_STATUS) {
            EcISCTStatus &= ~(ISCT_S0_ISCT_STATUS | ISCT_NET_DETECT_ENABLE);
        }

        CompalECACPINVSWriteByte(ISCT_STATUS_OFFSET, EcISCTStatus);

        // Get ISCT Wake Reason
        CompalISCTWakeReasonGet(&Instance->Ppi);
    } else {
        // Get EC ISCT Stauts & Set ISCT_STATUS bit4 = 0 (ISCT disable)
        EcISCTStatus = CompalECACPINVSReadByte(ISCT_STATUS_OFFSET);
        EcISCTStatus &= ~(ISCT_SUPPORT_ENABLE);
        CompalECACPINVSWriteByte(ISCT_STATUS_OFFSET, EcISCTStatus);
    }

    // Check if IFFS enable in SCU
    if (SystemConfiguration.EnableRapidStart == 1) {
        // Get EC IFFS Stauts  & Set IFFS_STATUS bit4 = 1 (IFFS enable)
        EcIFFSStatus = CompalECACPINVSReadByte(IFFS_STATUS_OFFSET);
        EcIFFSStatus |= IFFS_SUPPORT_ENABLE;
        CompalECACPINVSWriteByte(IFFS_STATUS_OFFSET, EcIFFSStatus);
    } else {
        // Get EC IFFS Stauts  & Set IFFS_STATUS bit4 = 0 (IFFS disable)
        EcIFFSStatus = CompalECACPINVSReadByte(IFFS_STATUS_OFFSET);
        EcIFFSStatus &= ~(IFFS_SUPPORT_ENABLE);
        CompalECACPINVSWriteByte(IFFS_STATUS_OFFSET, EcIFFSStatus);
    }

    return EFI_SUCCESS;
}
