
#include "CompalPlatformHookPei.h"

typedef struct _COMPAL_PLATFORM_HOOK_PEI_INSTANCE {
    COMPAL_PLATFORM_HOOK_PEI_PPI     Ppi;
    EFI_PEI_PPI_DESCRIPTOR           PpiDesc;
} COMPAL_PLATFORM_HOOK_PEI_INSTANCE;

EFI_STATUS
EFIAPI
ProjectPlatformHookPei (
    IN     CONST EFI_PEI_SERVICES                         **PeiServices,
    IN     COMPAL_PLATFORM_HOOK_PEI_PPI                   * This,
    IN     COMPAL_PLATFORM_HOOK_PEI_OVERRIDE_SETTING      OverrideType,
    ...
)
{
    EFI_STATUS  Status = EFI_SUCCESS;

    switch (OverrideType) {
    case COMPAL_PLATFORM_HOOK_PEI_PLATFORMSTAGE1_OVERRIDE:
        ProjectPlatformHookPeiPlatformStage1Override ( PeiServices );
        break;

    case COMPAL_PLATFORM_HOOK_PEI_PLATFORMSTAGE2_OVERRIDE:
        ProjectPlatformHookPeiPlatformStage2Override ( PeiServices );
        break;

    default:
        break;
    }

    return Status;
}

EFI_STATUS
EFIAPI
CompalPlatformHookPeiEntryPoint (
    IN  EFI_FFS_FILE_HEADER        *FfsHeader,
    IN  CONST EFI_PEI_SERVICES     **PeiServices
)
{

    EFI_STATUS                          Status;
    COMPAL_PLATFORM_HOOK_PEI_INSTANCE   *Instance;

    Instance = (COMPAL_PLATFORM_HOOK_PEI_INSTANCE *) AllocateZeroPool (sizeof (COMPAL_PLATFORM_HOOK_PEI_INSTANCE));
    if ( Instance == NULL ) {
        return EFI_OUT_OF_RESOURCES;
    }

    Instance->Ppi.CompalPlatformHookPeiDispatch = ProjectPlatformHookPei;

    Instance->PpiDesc.Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    Instance->PpiDesc.Guid = &gCompalPlatformHookPeiPpiGuid;
    Instance->PpiDesc.Ppi = &Instance->Ppi;

    Status = PeiServicesInstallPpi (&Instance->PpiDesc);
    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    return Status;
}
