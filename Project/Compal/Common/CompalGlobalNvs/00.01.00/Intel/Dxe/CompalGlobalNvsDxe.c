
#include <CompalGlobalNvsProtocol.h>
#include <Library/BaseMemoryLib.h>

//EFI_GUID gCompalGlobalNvsProtocolGuid = COMPAL_GLOBAL_NVS_PROTOCOL_GUID;

COMPAL_GLOBAL_NVS_PROTOCOL       CompalGlobalNvsProtocol={0};

extern EFI_BOOT_SERVICES  *gBS;

EFI_STATUS
CompalGlobalNvsDxeEntryPoint (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
    EFI_STATUS                        Status = EFI_SUCCESS;
    COMPAL_GLOBAL_NVS_AREA            *CompalGlobalNvsAreaPtr;
    UINTN                             BufferSize;

    BufferSize = sizeof (COMPAL_GLOBAL_NVS_AREA);

    Status = (gBS->AllocatePool) (EfiACPIMemoryNVS, BufferSize, &CompalGlobalNvsAreaPtr);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    ZeroMem (CompalGlobalNvsAreaPtr, BufferSize);

    CompalGlobalNvsProtocol.CompalArea = CompalGlobalNvsAreaPtr;

    Status = gBS->InstallMultipleProtocolInterfaces (
                 &ImageHandle,
                 &gCompalGlobalNvsProtocolGuid,
                 &CompalGlobalNvsProtocol,
                 NULL
             );

    return Status;
}
