/** @file
  Header file for H2O Console Redirection Configuration

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CR_CONFIG_H_
#define _CR_CONFIG_H_

#include <Guid/CrConfigHii.h>
#include <Guid/SolSetupConfigHii.h>

#define CR_CONFIGURATION_VARSTORE_ID        0x7777

#define CR_DEVICE_LABEL_START               0x1000
#define CR_DEVICE_LABEL_END                 0x1001

#define CONSOLE_REDIRECTION_ROOT_FORM_ID    0x2001
#define CR_DEVICE_FORM_ID                   0x2002

#define CR_DEVICE_QUESTION_ID_BASE          0xC000

#define KEY_GLOBAL_TERMINAL_TYPE            0x0D00
#define KEY_GLOBAL_BAUD_RATE                0x0D01
#define KEY_GLOBAL_DATA_BITS                0x0D02
#define KEY_GLOBAL_PARITY                   0x0D03
#define KEY_GLOBAL_STOP_BITS                0x0D04
#define KEY_GLOBAL_FLOW_CONTROL             0x0D05

#define KEY_PORT_ENABLE                     0x0D06
#define KEY_USE_GLOBAL_SETTING              0x0D07

#define KEY_TERMINAL_TYPE                   0x0D08
#define KEY_BAUD_RATE                       0x0D09
#define KEY_DATA_BITS                       0x0D0A
#define KEY_PARITY                          0x0D0B
#define KEY_STOP_BITS                       0x0D0C
#define KEY_FLOW_CONTROL                    0x0D0D

#define KEY_SCAN_F9                         0xF0D2

//
// Console Redirection Text Mode Resolution (CRTextModeResolution)
//
#define CR_TEXT_MODE_AUTO                   0
#define CR_TEXT_MODE_80X25                  1
#define CR_TEXT_MODE_80X24DF                2
#define CR_TEXT_MODE_80X24DL                3
#define CR_TEXT_MODE_128X40                 4

#endif

