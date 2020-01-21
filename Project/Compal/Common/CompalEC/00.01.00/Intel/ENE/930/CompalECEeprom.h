
#ifndef _COMPAL_EC_EEPROM_H_
#define _COMPAL_EC_EEPROM_H_

VOID
CompalECEepromWriteByte (
    IN    UINT16    EepromOffset,
    IN    UINT8     WriteData
);

UINT8
CompalECEepromReadByte (
    IN    UINT16    DataIndex
);

UINT8
CompalECGetEepromSlaveAdd(
    VOID
);

#endif
