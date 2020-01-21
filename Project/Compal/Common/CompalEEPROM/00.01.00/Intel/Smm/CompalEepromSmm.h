
#ifndef _COMPAL_EEPROM_SMM_H_
#define _COMPAL_EEPROM_SMM_H_

#include <Protocol/SmmBase2.h>
#include <Pi/PiSmmCis.h>
#include <CompalEepromDefinition.h>
#include <Library/PcdLib.h>

#define OEM_EEPROM_READ_FROM_EC            0x00  // Read EEPROM by EC command
#define OEM_EEPROM_READ_FROM_BIOS_ROM      0x01  // Read EEPROM from BIOS SPI ROM
#define OEM_EEPROM_READ_FROM_LPC_SPI       0x02  // Read EEPROM from SPI direct

#define SAVE_KBC_STATE                     FALSE // Save keyboard controller state
#define RESTORE_KBC_STATE                  TRUE  // Restore keyboard controller state

VOID
 CompalECEepromWriteByte  (
    IN    UINT16    EepromOffset,
    IN    UINT8     WriteData
);


UINT8
CompalECEepromReadByte   (
    IN    UINT16    DataIndex
);

UINT8
CompalECGetEepromSlaveAdd(
    VOID
);

extern EFI_BOOT_SERVICES  *gBS;
 
#endif
