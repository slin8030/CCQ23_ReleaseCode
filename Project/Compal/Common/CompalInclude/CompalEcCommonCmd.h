/* (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00                             Init version.
 1.01      04/15/16  ThomasChen   Modify code for nuvoton EC.

*/
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
// EC ACPI Command set
//----------------------------------------------------------
#define EC_ACPI_READ_CMD                    0x80
#define EC_ACPI_WRITE_CMD                   0x81

//----------------------------------------------------------
// EC WRITE COMMAND SET DEFINITION For Compal Common EC
//----------------------------------------------------------
//**********************************************************
// EC Function: Assign Bank of EEPROM
//    CMD: 0x42
//      Parameter 1: number of Bank of EEPROM
//
//**********************************************************
#define EC_CMD_EEPROM_BANK_NUM              0x42
#define EC_CMD_EEPROM_BANK_NUM_CMD_DATA_LEN 0x02    //DATA byte length part of command set

//**********************************************************
// EC Function: Get Board ID
//    CMD: 0x45
//      Parameter: 0xAB
//
//    RETURN:
//      BYTE 1
//**********************************************************
#define EC_CMD_BOARD_ID                    0x45
#define EC_DATA_BOARD_ID                   0xAB

//**********************************************************
// EC Function: Write eeprom
//    CMD: 0x4D
//      Parameter 1: address of eeprom
//      Parameter 2: data of eeprom address
//**********************************************************
#define EC_CMD_EEPROM_WRITE                 0x4D
#define EC_CMD_EEPROM_WRITE_CMD_DATA_LEN    0x02

//**********************************************************
// EC Function: Read eeprom
//    CMD: 0x4E
//      Parameter 1: address of eeprom
//      Parameter 2: value
//
//    RETURN: None
//
//**********************************************************
#define EC_CMD_EEPROM_READ                  0x4E
#define EC_CMD_EEPROM_READ_CMD_DATA_LEN     0x02
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
#define EC_CMD_BIOS_REVISION                                    0x51
#define EC_CMD_BIOS_REVISION_CMD_DATA_LEN                       0x00
#define EC_CMD_BIOS_REVISION_RETURN_DATA_LEN                    0x03

//**********************************************************
// EC Function: Get Platform information
//    CMD: 0x52
//      refer to EC common spec
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
#define EC_CMD_GET_EC_INFORMATION_EEPROM_ADDRESS                0xA6
#define EC_CMD_GET_EC_INFORMATION_EEPROM_ADDRESS_DATA_LEN       0x01

//**********************************************************
// EC Function: Get EC SMI event number
//    CMD: 0x56
//      Parameter 1: 0x56
//
//    RETURN:
//      BYTE 1
//**********************************************************
#define EC_CMD_SMIID_INDEX                                      0x56
#define EC_CMD_SMIID_INDEX_DATA_LEN                             0x01

//**********************************************************
// EC Function: Write EDID Index Value
//    CMD: 0x58
//      Parameter 1: 0x58
//      Parameter 2: Data is Index of EDID
//**********************************************************
#define EC_CMD_EDID_INDEX                                       0x58
#define EC_CMD_EDID_INDEX_DATA_LEN                              0x01

//**********************************************************
// EC Function: SYSTEM NOTIFICATION
//    CMD: 0x59
//      Refer to EC Common Spec
//
//    RETURN:
//			None
//**********************************************************
#define EC_CMD_SYSTEM_NOTIFICATION                                       0x59
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
	 //EC command for write EC name space



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

#define EC_CMD_METHOD_8042                          0x64
#define EC_CMD_METHOD_ACPI                          0x66
#define EC_CMD_METHOD_APP                           0x68
// Bit0: EC CMD/DATA Procesing
// Bit1: EC CMD/DATA EC Start
// Bit2-6: Reserved
#define EC_INDEXIO_CMD_CNTL_BIT_PROCESS             0x01
#define EC_INDEXIO_CMD_CNTL_BIT_PROCESS_MASK        0x01
#define EC_INDEXIO_CMD_CNTL_BIT_START               0x02
#define EC_INDEXIO_CMD_CNTL_BIT_START_MASK          0x02
//---------------------------------------------------------
//-[COM] VC20110408

//#define EC_INDEXIO_PORT_NUMBER                    0x01
//#define EC_MEMORY_MAPPING_PORT_NUMBER             0x02

//**********************************************************
// EC Function: Read ec namespace
//    CMD: 0x0B
//**********************************************************
#define EC_CMD_EcNameSpace                	0xB0
    #define SYS_STATUS                    	0xA3

typedef enum {
  EC_INDEXIO_PORT = 0,
  EC_MEMORY_MAPPING_PORT,
} EC_CMD_PORT_SETTING;


#endif
