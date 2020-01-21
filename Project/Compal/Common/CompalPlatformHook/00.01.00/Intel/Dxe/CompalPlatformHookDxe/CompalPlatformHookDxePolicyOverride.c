#include <SetupConfig.h>
#include <Protocol/PchPlatformPolicy/PchPlatformPolicy.h>

EFI_STATUS
CompalPCHPolicyDxeOverride (
    IN OUT PCH_PLATFORM_POLICY_PROTOCOL            *PchPolicyProtocol,
    IN OUT SYSTEM_CONFIGURATION                        *SetupVariable
)
//
// OEM override PCH PLATFORM POLICY
//
{
    return EFI_SUCCESS;
}
