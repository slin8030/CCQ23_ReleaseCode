

#include <CompalECLib.h>

VOID
CompalECAcpiMode (
    IN  BOOLEAN                           EnableEcMode
)
{
    UINT8         EcCommand;

    if (EnableEcMode) {
        EcCommand = EC_CMD_SYSTEM_NOTIFICATION_ACPI_ENABLE;
    } else {
        EcCommand = EC_CMD_SYSTEM_NOTIFICATION_ACPI_DISABLE;
    }
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_SYSTEM_NOTIFICATION,EcCommand);
}
