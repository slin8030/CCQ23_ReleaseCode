
#include <CompalECLib.h>

#define EC_RAM_BASE_0xF554 0xF554
#define EC_RAM_BASE_0xFF10 0xFF10
#define EC_RAM_BASE_0xFF14 0xFF14

EFI_STATUS
CompalECWait (
    IN  BOOLEAN     EnableWrites
)
{
    UINT8    Buffer;

    //
    // If EC is in Idle mode, do nothing
    //
    Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
    if (Buffer == 0xA5) {
        return EFI_SUCCESS;
    }

    if (EnableWrites == TRUE) {
        //
        // If EC in Wait Mode
        //
        Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
        if (Buffer == 0x33) {
            return EFI_SUCCESS;
        }
        //
        // Let EC into Wait
        //
        CompalECWaitMode();

        //
        // Check EC RAM 0xF554 = 33 , If no, keep loop
        //
        do {
            Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
        } while (Buffer !=0x33 );

        // Delay
        CompalAcpiStall(20000);
    } else {
        // Let EC wake up
        // Exit EC idle mode through set 0xFF10 = 0xFF
        CompalECIndexIOWrite(EC_RAM_BASE_0xFF10,0xFF);

        // Check EC RAM 0xF554 = 0
        // Check EC already exit idle mode
        do {
            Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
        } while (Buffer !=0x00 );
    }
    return EFI_SUCCESS;
}

EFI_STATUS
CompalECIdle (
    IN  BOOLEAN    EnableWrites
)
{
    UINT8    Buffer = 0;

    //
    // Save Generic IO Decode 2
    //
    if (EnableWrites == TRUE) {
        //
        // If EC in Idle Mode
        //
        Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
        if (Buffer == 0xA5) {
            return EFI_SUCCESS;
        }

        // Let EC into IDLE
        CompalECIdleMode();

        // Check EC RAM 0xF554 = 0xA5
        // Make sure EC in idle mode
        do {
            Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
        } while (Buffer !=0xA5 );

        // Reset 8051
        Buffer = CompalECIndexIORead (EC_RAM_BASE_0xFF14);
        CompalECIndexIOWrite(EC_RAM_BASE_0xFF14, (Buffer | 0x01));

        // Delay
        CompalAcpiStall(1000000);
    } else {
        // Exit 8051 reset
        Buffer = CompalECIndexIORead (EC_RAM_BASE_0xFF14);
        CompalECIndexIOWrite(EC_RAM_BASE_0xFF14,(Buffer & (~0x01)));

        // Check EC RAM 0xF554 = 0
        // Check EC already exit idle mode
        do {
            Buffer = CompalECMemoryRead(EC_RAM_BASE_0xF554);
        } while ( Buffer !=0x00 );
    }
    return EFI_SUCCESS;
}

VOID
CompalECIdleMode (
    VOID
)
{
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, EC_CMD_SYSTEM_NOTIFICATION_IDLE_MODE);
    CompalAcpiStall(5000);	//5ms
}

VOID
CompalECWaitMode (
    VOID
)
{
    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT, EC_CMD_SYSTEM_NOTIFICATION, EC_CMD_SYSTEM_NOTIFICATION_WAIT_MODE);
}

EFI_STATUS
CompalEcSmmWait (
    IN  BOOLEAN     EnableWrites
)
{
    CompalECWait (EnableWrites);
    return EFI_SUCCESS;
}

EFI_STATUS
CompalEcSmmIdle (
    IN  BOOLEAN     EnableWrites
)
{
    CompalECIdle (EnableWrites);
    return EFI_SUCCESS;
}
