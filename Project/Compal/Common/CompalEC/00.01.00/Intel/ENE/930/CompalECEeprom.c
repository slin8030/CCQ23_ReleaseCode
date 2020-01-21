
#include <CompalECLib.h>

UINT8
CompalECEepromReadByte (
    IN    UINT16    DataIndex
)
{
    UINT8     Byte=0;
    UINT8     RetryCount;

    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_BANK_NUM,(UINT8)(DataIndex >> 8));
    for (RetryCount=0; RetryCount < EC_CMD_EEPROM_RETRYCOUNT; RetryCount++) {
        Byte = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_READ,(UINT8)DataIndex);
        if ( Byte !=  EC_CMD_EEPROM_READ_ERROR) {
            break;
        }
    }
    return Byte;
}


VOID
CompalECEepromWriteByte (
    IN    UINT16    EepromOffset,
    IN    UINT8     WriteData
)
{
    UINT8     Byte;
    UINT8     RetryCount;

    CompalECWriteCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_BANK_NUM,(UINT8)(EepromOffset >> 8));

    for (RetryCount =0; RetryCount < EC_CMD_EEPROM_RETRYCOUNT; RetryCount++) {
        CompalECWriteCmdTwoByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_WRITE,(UINT8)EepromOffset,WriteData);
        Byte = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT,EC_CMD_EEPROM_READ,(UINT8)EepromOffset);
        if ( Byte == WriteData ) {
            break;
        }
    }
}

UINT8
CompalECGetEepromSlaveAdd (
    VOID
)
{
    UINT8    Eeprom_Smbus_Slave_Addr;

    Eeprom_Smbus_Slave_Addr = CompalECReadCmdByte(EC_MEMORY_MAPPING_PORT, EC_GET_SLAVE_ADDR_CMD, EC_GET_SLAVE_ADDR_DATA);
    return Eeprom_Smbus_Slave_Addr;
}
