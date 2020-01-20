/** @file
  Header file for CrPolicyType.
;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CR_POLICY_TYPE_H_
#define _CR_POLICY_TYPE_H_

#include <Uefi.h>

#define BIT_0              (1<<0)
#define BIT_1              (1<<1)
#define BIT_2              (1<<2)
#define BIT_3              (1<<3)
#define BIT_4              (1<<4)
#define BIT_5              (1<<5)
#define BIT_6              (1<<6)
#define BIT_7              (1<<7)
#define BIT_8              (1<<8)
#define BIT_9              (1<<9)
#define BIT_10             (1<<10)
#define BIT_11             (1<<11)
#define BIT_12             (1<<12)
#define BIT_13             (1<<13)
#define BIT_14             (1<<14)
#define BIT_15             (1<<15)

// #define CR_DISABLE                      0
// #define CR_ENABLE                       1
//
// #define CR_TERMINAL_VT100               0
// #define CR_TERMINAL_VT100P              1
// #define CR_TERMINAL_UTF8                2
// #define CR_TERMINAL_ANSI                3

#define CR_TERMINAL_VIDEO_FLAG          BIT0
#define CR_TERMINAL_VIDEO_COLOR         0
#define CR_TERMINAL_VIDEO_MONO          BIT0

#define CR_TERMINAL_KEY_FLAG            BIT1
#define CR_TERMINAL_KEY_SENSELESS       0
#define CR_TERMINAL_KEY_SENSITIVE       BIT1

#define CR_TERMINAL_COMBO_KEY_FLAG      BIT2
#define CR_TERMINAL_COMBO_KEY_DISABLE   0
#define CR_TERMINAL_COMBO_KEY_ENABLE    BIT2

#define CR_TERMINAL_ROW_FLAG            BIT3
#define CR_TERMINAL_ROW_25              0
#define CR_TERMINAL_ROW_24              BIT3

#define CR_24ROWS_POLICY_FLAG           BIT4
#define CR_24ROWS_POLICY_DEL_FIRST      0
#define CR_24ROWS_POLICY_DEL_LAST       BIT4

#define CR_UNKBC_SUPPORT_FLAG           BIT5
#define CR_UNKBC_SUPPORT_DISABLE        0
#define CR_UNKBC_SUPPORT_ENABLE         BIT5

#define CR_AUTO_REFRESH_FLAG            BIT6
#define CR_AUTO_REFRESH_DISABLE         0
#define CR_AUTO_REFRESH_ENABLE          BIT6

#define CR_MANUAL_REFRESH_FLAG          BIT7
#define CR_MANUAL_REFRESH_DISABLE       0
#define CR_MANUAL_REFRESH_ENABLE        BIT7

#define CR_TERMINAL_CHARSET_FLAG        BIT8
#define CR_TERMINAL_CHARSET_ASCII       0
#define CR_TERMINAL_CHARSET_GRAPHIC     BIT8

#define CR_ASYNC_TERMINAL_FLAG          BIT9
#define CR_ASYNC_TERMINAL_DISABLE       0
#define CR_ASYNC_TERMINAL_ENABLE        BIT9

#define CR_ISA_SERIAL_PORTA             0x00
#define CR_ISA_SERIAL_PORTB             0x01
#define CR_ISA_SERIAL_PORTC             0x02
#define CR_ISA_SERIAL_PORTD             0x03
#define CR_LAN_PORT_1                   0x10
#define CR_USB_SERIAL_ONLY              0x40
#define CR_PCI_SERIAL_ONLY              0x80
#define CR_ALL_PORTS                    0xFF

#define CR_BAUD_RATE_115200             0
#define CR_BAUD_RATE_57600              1
#define CR_BAUD_RATE_38400              2
#define CR_BAUD_RATE_19200              3
#define CR_BAUD_RATE_9600               4
#define CR_BAUD_RATE_4800               5
#define CR_BAUD_RATE_2400               6
#define CR_BAUD_RATE_1200               7
#define CR_BAUD_RATE_300                8

#define CR_DATA_7BIT                    7
#define CR_DATA_8BIT                    8

#define CR_PARITY_NONE                  1
#define CR_PARITY_EVEN                  2
#define CR_PARITY_ODD                   3

#define CR_STOP_1BIT                    1
#define CR_STOP_2BIT                    3

#define CR_FLOW_CONTROL_NONE            0
#define CR_FLOW_CONTROL_HARDWARE        1
#define CR_FLOW_CONTROL_SOFTWARE        2

#define CR_HEADLESS_USE_VBUFFER         0
#define CR_HEADLESS_USE_EBDA            1

#define CR_POLICY_NAME                  L"CrPolicy"
//
// -------PCD Structure----------------------
//
#pragma pack (1)
typedef struct _CR_POLICY_PCD {
  //
  // Feature flag
  //
  UINT8                  CRVideoType;
  UINT8                  CRTerminalKey;
  UINT8                  CRComboKey;
  UINT8                  Reserve1;            //This column from SCU setting, PCD reserve
  UINT8                  Reserve2;            //This column from SCU setting, PCD reserve
  UINT8                  CRUnKbcSupport;
  UINT8                  Reserve3;            //This column from SCU setting, PCD reserve
  UINT8                  CRManualRefresh;
  UINT8                  CRTerminalCharSet;
  UINT8                  CRAsyncTerm;
  UINT8                  CRScreenOffCheck;
  UINT8                  CRShowHelp;
  UINT8                  CRHeadlessVBuffer;
  UINT8                  Reserve4;            //This column from SCU setting, PCD reserve
  UINT8                  Reserve5;            //This column from SCU setting, PCD reserve
  UINT8                  Reserve6;            //This column from SCU setting, PCD reserve
  UINT8                  CROpROMLoadOnESegment;
  UINT8                  EndofFeatureFlag;    //Add new Feature flag before EndofFeatureFlag here
  //
  // Other setting
  //
  UINT8                  CRFifoLength;
  UINT8                  CRWriteCharInterval;
} CR_POLICY_PCD;

//
// -------Variable Structure----------------------
//
typedef struct _CR_FEATURE_FLAG {
  //
  // Feature flag that compatible old 16bit feature flag
  // OptionROM need flag: item 0~10
  //
  UINT8                  CRVideoType           :1;
  UINT8                  CRTerminalKey         :1;
  UINT8                  CRComboKey            :1;
  UINT8                  CRTerminalRows        :1;  //from SCU setting
  UINT8                  CR24RowsPolicy        :1;  //from SCU setting
  UINT8                  CRUnKbcSupport        :1;
  UINT8                  CRAutoRefresh         :1;  //from SCU setting
  UINT8                  CRManualRefresh       :1;
  UINT8                  CRTerminalCharSet     :1;
  UINT8                  CRAsyncTerm           :1;
  UINT8                  CRScreenOffCheck      :1;
  UINT8                  CRShowHelp            :1;
  UINT8                  CRHeadlessVBuffer     :1;
  UINT8                  CRForce80x25          :1;  //from SCU setting
  UINT8                  CRAfterPost           :1;  //from SCU setting
  UINT8                  CRFailSaveBaudrate    :1;  //from SCU setting
  UINT8                  CROpROMLoadOnESegment :1;
  //
  // Add new Feature flag below
  //
  UINT8                  CRTerminalResize      :1;  //from SCU setting
  UINT8                  CRForce128x40         :1;  //from SCU setting

} CR_FEATURE_FLAG;

typedef union _CR_FEATURE_FLAG_TYPE {
  UINT16                 Data16;                    // Compatible Old 16bit flag
  UINT32                 Data32;
  CR_FEATURE_FLAG        Bit;
} CR_FEATURE_FLAG_TYPE;

typedef struct _CR_POLICY_VARIABLE {
  CR_FEATURE_FLAG_TYPE   Feature;
  UINT8                  GlobalBaudRate;            //from SCU setting
  UINT8                  GlobalParity;              //from SCU setting
  UINT8                  GlobalDataBits;            //from SCU setting
  UINT8                  GlobalStopBits;            //from SCU setting
  UINT8                  GlobalFlowControl;         //from SCU setting
  UINT8                  GlobalTerminalType;        //from SCU setting
  UINT8                  CRInfoWaitTime;            //from SCU setting
  UINT8                  CRFifoLength;
  UINT8                  CRWriteCharInterval;
} CR_POLICY_VARIABLE;

#pragma pack ()

extern EFI_GUID gCrConfigurationGuid;

#endif

