

#ifndef _COMPAL_EC_LIB_H_
#define _COMPAL_EC_LIB_H_

#include <Library/IoLib.h>
#include <CompalEcCommoncmd.h>
#include <CompalECEEPROM.h>
#include <CompalAcpiStall.h>

#define WORD_SHIFT_GET_HByte                0x08

// ENE's Index I/O Base Address
#define ENEIOBASE                           0xFD60
#define ENEIOBASE_INTEL                     0xFF2C
#define ENEIOBASE_AMD                       0xFD60
#define EC_INDEXIO_BASE_OFFSET_HIGH_BYTE    0x01
#define EC_INDEXIO_BASE_OFFSET_LOW_BYTE     0x02
#define EC_INDEXIO_BASE_OFFSET_DATA_BYTE    0x03


// Port 64h/60h, 66h/62h, 6Ch/68h constant define
#define EC_8254_METRONOME_TICK_PERIOD       300             // TickPeriod is 300 (30 us).

#define EC_8042_CMD_PORT                    0x64
#define EC_8042_DATA_PORT                   0x60
#define EC_8042_STATUS_PORT                 0x64
#define   EC_8042_STATUS_BIT_OBF            0x01
#define   EC_8042_STATUS_BIT_IBF            0x02

#define EC_ACPI_CMD_PORT                    0x66
#define EC_ACPI_DATA_PORT                   0x62
#define EC_ACPI_STATUS_PORT                 0x66
#define EC_ACPI_STATUS_BIT_OBF              0x01
#define EC_ACPI_STATUS_BIT_IBF              0x02

#define EC_EXT_8042_CMD_PORT                0x6C
#define EC_EXT_8042_DATA_PORT               0x68
#define EC_EXT_8042_STATUS_PORT             0x6C
#define EC_EXT_8042_BUSY_FLAG               0x80

#define EC_OBF_TIME_OUT                     1000            // 10us in unit, 10ms
#define EC_NO_DATACMD_BYTE                  0x00
#define EC_OBF_EMPTY_TIME                   1000             // unit = 1us -> 1ms
#define EC_ERROR                            0x02
#define EC_NO_DATACMD_BYTE                  0x00

#define EC_IBF_CLEAR_TIMEOUT_RETRY_COUNT    0xFFFF
#define EC_OBF_EMPTY_TIMEOUT_RETRY_COUNT    0xFFFF
#define EC_OBF_SET_TIMEOUT_RETRY_COUNT      500            	// retry 500 times
#define EC_OBF_SET_WAIT_TIME                100             // 1us in unit, 100us
#define EC_CMD_RETRY_COUNT                  0x03


#define EC_EEPROM_CMD_WRITE                 0x4B
#define EC_EEPROM_CMD_READ                  0x4C
#define EC_OEM_EEPROM_SMBUS_ADDR            0xA0
#define EC_OEM_EEPROM_START_ADDR            0x00
#define EC_OEM_EEPROM_BANK_NUM              0x00
#define EC_OEM_EEPROM_LEN                   0x100

#define EC_CMD_BUF_LEN                      0x03
#define EC_GET_SLAVE_ADDR_CMD               0x43
#define EC_GET_SLAVE_ADDR_DATA              0x23

#define EC_SYSTEM_NOTIFI_CMD                0x59
//  #define EC_KBC_REVERSION_CMD                0x51
#define ECDebugModeCmd                      0x6B
//  #define EC_ENTER_S4_MODE                    0xBB
//  #define EC_EXIT_S4_MODE                     0xBC
#define EC_ENTER_IDLE_MODE                  0xF2

#define TurnOnDebug                         1
#define TurnOffDebug                        0
#define EcDebugFeature1                     1
#define EcDebugFeature2                     2
#define EcDebugFeature3                     3
#define EcDebugFeature4                     4

#define MAX_DBG_SIZE                        0xFF
#define ECDebugMsgCmd                       0xB4
#define EcGetDebugModeCmd                   0xE1

#define OEM_EEPROM_ADDRESS                  0xA0
#define Read_EEPROM_CMD                     0x4C
#define Write_EEPROM_CMD                    0x4B
#define R_EC_PLATFORM_CRISIS_CMD            0xA4

// 8259 mask port
#define I8259_MASK                          0x21
#define I8259A_MASK                         0xA1

// Keyboard Controller Command
#define KBC_READ_CMD_BYTE                   0x20
#define KBC_WRITE_CMD_BYTE                  0x60

// Keyboard Command
#define DISABLE_PS2_MOUSE                   0xA7
#define ENABLE_PS2_MOUSE                    0xA8
#define DISABLE_PS2_KEYBOARD                0xAD
#define ENABLE_PS2_KEYBOARD                 0xAE

VOID
CompalEcSendDebugBufferCmd (
    UINT8              DataLength,
    IN   OUT    UINT8  *InputBuffer
);

VOID
CompalEcGetDebugMode (
    IN   OUT    UINT8   *Mode
);

VOID
CompalEcSetDebugMode (
    IN   UINT8     ECCmdPortNum,
    IN   UINT8     Mode
);


VOID
CompalECReadENEData (
    IN    UINT8    LowAddress,
    IN    UINT8    HighAddress,
    OUT   UINT8    *OutData
);
VOID
CompalECWriteENEData (
    IN    UINT8    LowAddress,
    IN    UINT8    HighAddress,
    IN    UINT8    InputData
);

EFI_STATUS
CompalECWaitOBFEmpty (
    IN    UINT8    ECStatusPortNum
);

EFI_STATUS
CompalECWaitOBF (
    IN    UINT8    ECStatusPortNum
);

UINT8
CompalECReadCmdByte (
    IN    UINT8    ECCmdPortNum,
    IN    UINT8    ECCmd,
    IN    UINT8    ECData
);

EFI_STATUS
CompalECReadCmd (
    IN    UINT8    ECCmdPortNum,
    IN    UINT8    ECCmd,
    IN    UINT8    ECDataLen,
    IN    UINTN    ECRetrunDataLen,
    IN    UINT8    *ECReturnBufPTR
);

VOID
CompalECWriteCmdByte (
    IN    UINT8    ECCmdPortNum,
    IN    UINT8    ECCmd,
    IN    UINT8    ECData
);

VOID
CompalECWriteCmdTwoByte (
    IN    UINT8    ECCmdPortNum,
    IN    UINT8    ECCmd,
    IN    UINT8    ECData1,
    IN    UINT8    ECData2
);

EFI_STATUS
CompalECWriteCmd (
    IN    UINT8    ECCmdPortNum,
    IN    UINT8    ECCmd,
    IN    UINT8    ECDataLen,
    IN    UINT8    *ECDataCmdPTR
);

EFI_STATUS
CompalECCheckBusyFlag (
    IN    UINT8    ECStatusPortNum
);

EFI_STATUS
CompalECWaitECIBFClr (
    IN    UINT8    ECStatusPortNum
);

UINT8
CompalECACPINVSReadByte(
    IN    UINT8    NVRAMOffset
);

VOID
CompalECACPINVSWriteByte(
    IN    UINT8    NVRAMOffset,
    IN    UINT8    NVRAMData
);

VOID
CompalECStall(
    IN    UINTN    Microseconds
);

EFI_STATUS
CompalECReadEEPROM (
    IN    UINT8    EcEEPROM_ADR_Low,
    IN    UINT8    EcEEPROM_Bank_Num,
    IN    UINTN    EcEEPROM_Data_Len,
    OUT   UINT8    *EcEEPROM_Buf_PTR
);

EFI_STATUS
CompalECReadKBCVersion(
    IN    OUT    UINT8     *InputBuffer
);

VOID
CompalECAcpiMode (
    IN    BOOLEAN    EnableEcMode
);

EFI_STATUS
CompalECWait (
    IN    BOOLEAN    EnableWrites
);

EFI_STATUS
CompalECIdle (
    IN    BOOLEAN    EnableWrites
);

VOID
CompalECIdleMode (
    VOID
);

VOID
CompalECWaitMode (
    VOID
);

EFI_STATUS
CompalEcSmmWait (
    IN    BOOLEAN    EnableWrites
);

EFI_STATUS
CompalEcSmmIdle (
    IN    BOOLEAN    EnableWrites
);

UINT8
CompalECCrisisStatus (
    VOID
);

BOOLEAN
CompalECCheeckPowerState (
    VOID
);

UINT8
CompalECACPINVSReadByteByIndexIO (
    IN    UINT8     NVRAMOffset
);

VOID
CompalECACPINVSWriteByteByIndexIO (
    IN    UINT8     NVRAMOffset,
    IN    UINT8     NVRAMData
);

EFI_STATUS
CompalECReadKBCVersionByIndexIO (
    IN    OUT    UINT8    *InputBuffer
);

EFI_STATUS
CompalEcReadSPIData (
    IN    BOOLEAN   IdleMode,
    IN    UINT8     *DstAddress,
    IN    UINT8     *FlashAddress,
    IN    UINTN     BufferSize
);

VOID
CompalECIndexIOWrite (
    IN    UINTN     Offset,
    IN    UINT8     WriteData
);

UINT8
CompalECIndexIORead (
    IN    UINTN     Offset
);

VOID
CompalECMemoryMappingWrite (
    IN    UINTN     Offset,
    IN    UINT8     WriteData
);

UINT8
CompalECMemoryMappingRead (
    IN    UINTN     Offset
);

VOID
CompalECMemoryWrite (
    IN    UINTN     Offset,
    IN    UINT8     WriteData
);

UINT8
CompalECMemoryRead (
    IN    UINTN     Offset
);

#endif
