
#ifndef _COMPAL_PLATFORM_HOOK_DXE_H_
#define _COMPAL_PLATFORM_HOOK_DXE_H_

#include <CompalEepromDxe.h>
#include <CompalEEPROMDxeProtocol.h>
#include <CompalPlatformHookDxeProtocol.h>
#include <CompalEcLib.h>
#include <CompalEcCommoncmd.h>
#include <SetupConfig.h>
#include <Library/FdSupportLib.h>
#include <Library/BaseMemoryLib.h>
#include <Common/Include/ChipsetLib.h>
#include <Protocol/PchPlatformPolicy/PchPlatformPolicy.h>
#include <Library/FlashDeviceSupport/SpiAccessLib/SpiAccess.h>

//
// Block lock bit definitions
//
#define SPI_WRITE_LOCK                  0x01
#define SPI_FULL_ACCESS                 0x00
#define FLASH_PROTECT_RANGE             0x02

#ifndef OEM_EEPROM_DATA_LENGTH
#define OEM_EEPROM_DATA_LENGTH   0x100
#endif

// Function prototypes
//
EFI_STATUS
EFIAPI
ProjectPlatformHookDxeMain (
    IN     COMPAL_PLATFORM_HOOK_DXE_PROTOCOL            * This,
    IN     COMPAL_PLATFORM_HOOK_DXE_OVERRIDE_SETTING    OverrideType,
    ...
);

EFI_STATUS 
CompalPCHPolicyDxeOverride (
    IN OUT PCH_PLATFORM_POLICY_PROTOCOL *PchPolicyProtocol,
    IN OUT SYSTEM_CONFIGURATION *SetupVariable
);

extern EFI_BOOT_SERVICES  *gBS;

extern EFI_STATUS OemSpiFlashLock (IN UINT8 LockState, IN  UINT8 LockType);

#endif
