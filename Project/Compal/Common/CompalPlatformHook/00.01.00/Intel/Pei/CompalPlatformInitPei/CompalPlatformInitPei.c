
#include "CompalPlatformInitPei.h"

typedef struct _COMPAL_PLATFORM_INIT_PEI_INSTANCE {
    COMPAL_PLATFORM_INIT_PEI_PPI     Ppi;
    EFI_PEI_PPI_DESCRIPTOR           PpiDesc;
} COMPAL_PLATFORM_INIT_PEI_INSTANCE;

EFI_STATUS
ProjectPlatformInitPeiGpio (
    IN   CONST EFI_PEI_SERVICES    **PeiServices,
    IN   COMPAL_GPIO_SETTINGS      *CompalGpioTable,
    IN   UINT16                    CompalGpioTableCount,
    IN   UINT16                    CompalGpioTableVersion
)
/*++
Routine Description:
  Check Vender ID and Revision ID to call difference function to program GPIO

Arguments:
  PeiServices               Pointer's pointer to EFI_PEI_SERVICES
  CompalGpioTable           Table defined GPIO setting
  CompalGpioTableCount      Number of GPIO setting
  CompalGpioTableVersion    Reserved for GPIO Table Version Control

Returns:
  EFI_SUCCESS        Program GPIO successfully

--*/
{
    UINT8            RevisionId = 0x00;
    UINT16           VendorId;

    VendorId = PciRead16 (PCI_LIB_ADDRESS (0,0,0,PCI_VENDOR_ID_OFFSET));

    if (VendorId == INTEL_VID) {
        //
        // Init Intel GPIO
        //
        CompalPlatformInitPeiIntelGpio (PeiServices, CompalGpioTable, CompalGpioTableCount, CompalGpioTableVersion);
    } else {
        //
        // Initial AMD GPIO
        //
        RevisionId = PciRead8 (PCI_LIB_ADDRESS (0,0x14,0,PCI_REVISION_ID_OFFSET));
        if (RevisionId >= 0x40) {
            //
            // Program GPIO for AMD SB800
            //
            //CompalPlatformInitPeiAMDSB800Gpio (PeiServices, CompalGpioTable, CompalGpioTableCount, CompalGpioTableVersion);
        } else {
            //
            // Program GPIO for AMD SB700
            //
            //CompalPlatformInitPeiAMDSB700Gpio (PeiServices, CompalGpioTable, CompalGpioTableCount, CompalGpioTableVersion);
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS
ProjectPlatformInitPeiAudioVerbTable (
    IN    CONST EFI_PEI_SERVICES   **PeiServices,
    IN    PCH_AZALIA_VERB_TABLE    *CompalVerbTable,
    IN    UINT8                    NumberOfVerbTable
)
/*++
Routine Description:

Arguments:
  PeiServices        Pointer's pointer to EFI_PEI_SERVICES

Returns:
  EFI_SUCCESS        Program Verb Table successfully

--*/
{
    UINT16           VendorId;

    VendorId = PciRead16( PCI_LIB_ADDRESS ( 0,0,0,PCI_VENDOR_ID_OFFSET) );
    if (VendorId == INTEL_VID) {
        //
        // Init Intel Audio verb table
        //
        CompalPlatformInitPeiIntelAudioVerbTable ( PeiServices, CompalVerbTable, NumberOfVerbTable );
    } else {
        //
        // Init AMD Audio verb table
        //
    }

    return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CompalPlatformInitPeiEntryPoint (
    IN  EFI_FFS_FILE_HEADER        *FfsHeader,
    IN  CONST EFI_PEI_SERVICES     **PeiServices
)
{
    EFI_STATUS                          Status;
    COMPAL_PLATFORM_INIT_PEI_INSTANCE   *Instance;

    Instance = (COMPAL_PLATFORM_INIT_PEI_INSTANCE *) AllocateZeroPool (sizeof (COMPAL_PLATFORM_INIT_PEI_INSTANCE));
    if ( Instance == NULL ) {
        return EFI_OUT_OF_RESOURCES;
    }

    Instance->Ppi.CompalPlatformInitPeiGpio = ProjectPlatformInitPeiGpio;
    Instance->Ppi.CompalPlatformInitPeiAudioVerbTable = ProjectPlatformInitPeiAudioVerbTable;

    Instance->PpiDesc.Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    Instance->PpiDesc.Guid = &gCompalPlatformInitPeiPpiGuid;
    Instance->PpiDesc.Ppi = &Instance->Ppi;

    Status = PeiServicesInstallPpi (&Instance->PpiDesc);
    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    return Status;
}
