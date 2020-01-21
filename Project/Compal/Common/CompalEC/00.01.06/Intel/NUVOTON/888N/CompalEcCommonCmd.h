#ifndef _COMPAL_EC_COMMON_CMD_H_
#define _COMPAL_EC_COMMON_CMD_H_

#define EC_CMD_NO_DATA                      0x00
#define EC_CMD_NO_EXT_DATA_LEN              0x00
//----------------------------------------------------------
// Standard 8042 Command Set
//----------------------------------------------------------
#define Disable_Keyboard_Interface          0xAD
#define Enable_Keyboard_Interface           0xAE
#define Disable_Auxiliary_Device_Interface  0xA7
#define Enable_Auxiliary_Device_Interface   0xA8

//----------------------------------------------------------
// EC READ COMMAND SET DEFINITION For Compal Common EC
//----------------------------------------------------------
//**********************************************************
// EC Function: Read eeprom
//    CMD: 0x4D
//      Parameter 1: address of eeprom
//      Parameter 2: value
//
//    RETURN: None
//
//**********************************************************
#define EC_CMD_EEPROM_READ                  0x4E
#define EC_CMD_EEPROM_READ_CMD_DATA_LEN     0x02    //DATA byte length part of command set
#define EC_CMD_EEPROM_RETRYCOUNT            0x10
#define EC_CMD_EEPROM_READ_ERROR            0xFE
//**********************************************************
// EC Function: Get KBC version
//    CMD: 0x51
//      Parameter: None
//
//    RETURN:
//      BYTE 1: sub data 1
//           2: sub data 2
//           3: sub data 3
//**********************************************************
#define EC_CMD_BIOS_REVISION                 0x51
#define EC_CMD_BIOS_REVISION_CMD_DATA_LEN    0x00    //DATA byte length part of command set
#define EC_CMD_BIOS_REVISION_RETURN_DATA_LEN 0x03    //Return length of Data

//**********************************************************
// EC Function: Get Platform information
//    CMD: 0x52
//      refer to EC common spec
//
//
//    RETURN:
//				refer to EC common spec
//**********************************************************
#define EC_CMD_GET_EC_INFORMATION                               0x52
#define EC_CMD_GET_EC_INFORMATION_PROJECT_NAME                  0xA0
#define EC_CMD_GET_EC_INFORMATION_PROJECT_NAME_CMD_DATA_LEN     0x01
#define EC_CMD_GET_EC_INFORMATION_PROJECT_ID                    0xA2
#define EC_CMD_GET_EC_INFORMATION_PROJECT_ID_DATA_LEN           0x01
#define EC_CMD_GET_EC_INFORMATION_BOOTCODE_SIZE                 0xA3
#define EC_CMD_GET_EC_INFORMATION_BOOTCODE_SIZE_DATA_LEN        0x01
#define EC_CMD_GET_EC_INFORMATION_CRISIS_STATUS                 0xA4
#define EC_CMD_GET_EC_INFORMATION_CRISIS_STATUS_DATA_LEN        0x01


//----------------------------------------------------------
// EC ACPI Command set
//----------------------------------------------------------
#define EC_ACPI_READ_CMD                    0x80
#define EC_ACPI_WRITE_CMD                   0x81

//----------------------------------------------------------
// EC WRITE COMMAND SET DEFINITION For Compal Common EC
//----------------------------------------------------------
//**********************************************************
// EC Function: EC Reset
//    CMD: 0x40
//      Refer to EC Common Spec
//
//    RETURN:
//			None
//
//**********************************************************
#define EC_RESTART_CMD                      0x40

//**********************************************************
// EC Function: SYSTEM NOTIFICATION
//    CMD: 0x59
//      Refer to EC Common Spec
//
//
//    RETURN:
//			None
//
//**********************************************************
#define EC_CMD_SYSTEM_NOTIFICATION					 0x59
#define EC_CMD_SYSTEM_NOTIFICATION_SET_CRITICAL_BATTERY_WAKE_THRESHOLD	 0x6D	 //Set Critical Battery wake threshold(%)
#define EC_CMD_SYSTEM_NOTIFICATION_CLEAR_WAKE_STATUS	                 0x6E    //Return critical bettery wake status
#define EC_CMD_SYSTEM_NOTIFICATION_FAN_FULL_ON_DISABLE                   0x76
#define EC_CMD_SYSTEM_NOTIFICATION_FAN_FULL_ON_DISABLE_CMD_DATA_LEN      0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_FAN_FULL_ON_ENABLE                    0x77
#define EC_CMD_SYSTEM_NOTIFICATION_FAN_FULL_ON_ENABLE_CMD_DATA_LEN       0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_EC_SHUTDOWN                           0xA2
#define EC_CMD_SYSTEM_NOTIFICATION_EC_SHUTDOWN_CMD_DATA_LEN              0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_EC_LB_BEEP_ENABLE                     0xA3
#define EC_CMD_SYSTEM_NOTIFICATION_EC_LB_BEEP_ENABLE_CMD_DATA_LEN        0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_EC_LB_BEEP_DISABLE                    0xA4
#define EC_CMD_SYSTEM_NOTIFICATION_EC_LB_BEEP_DISABLE_CMD_DATA_LEN       0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_EC_CONTROL_FAN                        0xA5
#define EC_CMD_SYSTEM_NOTIFICATION_EC_CONTROL_FAN_CMD_DATA_LEN           0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_OS_CONTROL_FAN                        0xA8
#define EC_CMD_SYSTEM_NOTIFICATION_OS_CONTROL_FAN_CMD_DATA_LEN           0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_RTC_WAKE_ENABLE                       0xBD
#define EC_CMD_SYSTEM_NOTIFICATION_RTC_WAKE_ENABLE_CMD_DATA_LEN          0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_RTC_WAKE_DISABLE                      0xBE
#define EC_CMD_SYSTEM_NOTIFICATION_RTC_WAKE_DISABLE_CMD_DATA_LEN         0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_ACPI_ENABLE                           0xE8
#define EC_CMD_SYSTEM_NOTIFICATION_ACPI_ENABLE_CMD_DATA_LEN              0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_ACPI_DISABLE                          0xE9
#define EC_CMD_SYSTEM_NOTIFICATION_ACPI_DISABLE_CMD_DATA_LEN             0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_WAIT_MODE                             0xF1
#define EC_CMD_SYSTEM_NOTIFICATION_WAIT_MODE_CMD_DATA_LEN                0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_IDLE_MODE                             0xF2
#define EC_CMD_SYSTEM_NOTIFICATION_IDLE_MODE_CMD_DATA_LEN                0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_FLASH_COMPLETE                 0xF6
#define EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_REBOOT                         0xF7
#define EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_SHUTDOWN                       0xF8
#define EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_DONOTHING                      0xF9
#define EC_CMD_SYSTEM_NOTIFICATION_SYSTEM_SHUTDOWN_CMD_DATA_LEN          0x01    //DATA byte length part of command set
#define EC_CMD_SYSTEM_NOTIFICATION_EC_NAME_SPACE_READ                    0xB0	 //EC command for read EC name space
#define EC_CMD_SYSTEM_NOTIFICATION_EC_NAME_SPACE_OFFSET                  0xB0	 //EC name space offset 0xB0 for DTS temperature
#define EC_CMD_SYSTEM_NOTIFICATION_EC_NAME_SPACE_WRITE                   0xB1	 //EC command for write EC name space

//**********************************************************
// EC Function: Assign Bank of EEPROM
//    CMD: 0x42
//      Parameter 1: number of Bank of EEPROM
//
//**********************************************************
#define EC_CMD_EEPROM_BANK_NUM              0x42
#define EC_CMD_EEPROM_BANK_NUM_CMD_DATA_LEN 0x02    //DATA byte length part of command set
//**********************************************************
// EC Function: Write eeprom
//    CMD: 0x4D
//      Parameter 1: address of eeprom
//      Parameter 2: data of eeprom address
//**********************************************************
#define EC_CMD_EEPROM_WRITE                 0x4D
#define EC_CMD_EEPROM_WRITE_CMD_DATA_LEN    0x02    //DATA byte length part of command set

//**********************************************************
// EC Function: Write EDID Index Value
//    CMD: 0x58
//      Parameter 1: 0x58
//      Parameter 2: Data is Index of EDID
//**********************************************************
#define EC_CMD_EDID_INDEX                   0x58
#define EC_CMD_EDID_INDEX_DATA_LEN          0x01    //DATA byte length part of command set

//**********************************************************
// EC Function: Get Board ID
//    CMD: 0x45
//      Parameter: 0xAB
//
//    RETURN:
//      BYTE 1
//
//
//**********************************************************
#define EC_CMD_BOARD_ID           0x45
#define EC_DATA_BOARD_ID          0xAB

#define ECRAM_HIGH                0x08
#define ECRAM_LO                  0x00
#define ECRAM_BASE		  0x10800

#define EC_IndexIO_HighAddress_F3                   0xF3

#define EC_IndexIO_LowAddress_C4                    0xC4
#define EC_IndexIO_WAKE_ON_KB_EN                    (1 << 0)        // Enable Wake on Keyboard from S5
#define EC_IndexIO_WAKE_ON_KB_DIS                   (1 << 1)        // Disable Wake on Keyboard from S5
#define EC_IndexIO_ACPI_MODE_EN_CMD                 (1 << 2)        // ACPI Enable
#define EC_IndexIO_ACPI_MODE_DIS_CMD                (1 << 3)        // ACPI Disable
#define	EC_IndexIO_CMD_WAIT_DATA                    (1 << 4)        // EC Wait Command
#define	EC_IndexIO_CMD_IDLE_DATA                    (1 << 5)        // EC IDLE Command
#define	EC_IndexIO_WAKE_ON_PANEL_EN                 (1 << 6)        // Panel Open on Enable
#define	EC_IndexIO_WAKE_ON_PANEL_DIS                (1 << 7)        // Panel Open on Disable

#define EC_IndexIO_HighAddress_F6                   0xF6
#define EC_IndexIO_LowAddress_3A                    0x3A

//=======================================================================================================

//+[COM] VC20110408
//---------------------------------------------------------

#define EC_CMD_METHOD_INDEXIO                       0xFD60
#define EC_INDEXIO_BASE                             0xFD60
#define EEPROM_START_ADDRESS_NUVOTON                0x11000

#define EC_CMD_METHOD_8042                          0x64
#define EC_CMD_METHOD_ACPI                          0x66
#define EC_INDEXIO_CMD_CNTL                         0x10D82

// Bit0: EC CMD/DATA Procesing
// Bit1: EC CMD/DATA EC Start
// Bit2-6: Reserved
#define EC_INDEXIO_CMD_CNTL_BIT_PROCESS             0x01
#define EC_INDEXIO_CMD_CNTL_BIT_PROCESS_MASK        0x01
#define EC_INDEXIO_CMD_CNTL_BIT_START               0x02
#define EC_INDEXIO_CMD_CNTL_BIT_START_MASK          0x02

//EC RETURN DATA BUFFER
#define EC_INDEXIO_CMD_RETURN_DATA_BUFFER           0x10D83
#define EC_INDEXIO_CMD_RETURN_DATA_BUFFER_LEN       0x08

//Write Data buffer from 0xF9B9 - 0xF9BE
#define EC_INDEXIO_CMD_BUFFER                       0x10D8B
#define EC_INDEXIO_DATA_OF_CMD_BUFFER               0x10D8C
#define EC_INDEXIO_CMD_BUFFER_LEN                   0x06
//---------------------------------------------------------
//-[COM] VC20110408

//#define EC_INDEXIO_PORT_NUMBER                    0x01
//#define EC_MEMORY_MAPPING_PORT_NUMBER             0x02

typedef enum {
  EC_INDEXIO_PORT = 0,
  EC_MEMORY_MAPPING_PORT,
} EC_CMD_PORT_SETTING;

#define EC_RAM_START_ADDRESS                        0x10000
#define EC_MEMORY_MAPPING_BASE_ADDRESS              0xFF000000
#endif
