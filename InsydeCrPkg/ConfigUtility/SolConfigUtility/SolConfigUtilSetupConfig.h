/** @file
  Console Redirection Setup Configuration Definitions

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#ifndef _SOL_CONFIG_UTIL_SETUP_CONFIG_H_
#define _SOL_CONFIG_UTIL_SETUP_CONFIG_H_


#include <Guid/SolSetupConfigHii.h>


#define SOL_CONFIGURATION_VARSTORE_ID        0x7778

#define SOL_SERIAL_PORT_LABEL                0x1094
#define SOL_SERIAL_DEV_FORM_SUBTITTLE_LABEL  0x1095
#define SOL_FORM_LABEL                       0x1096
#define SOL_STATIC_IP_LABEL                  0x1097


#define SOL_FORM_ID                         0xC24

#define KEY_SOL_ENABLE                      0xC1A
#define KEY_SOL_USE_DHCP                    0xC1B

#define KEY_SOL_PORT_DHCP                   0x3609
#define KEY_SOL_PORT_IP                     0x360A
#define KEY_SOL_PORT_MASK                   0x360B
#define KEY_SOL_PORT_GATEWAY                0x360C
#define KEY_SOL_LOGIN_NAME                  0x360D
#define KEY_SOL_LOGIN_PASSWORD              0x360E
#define KEY_SOL_ADMIN_IP                    0x360F
#define KEY_SOL_ADMIN_MAC                   0x3610

#define KEY_LOAD_DEFAULT                    0xD03
//#define KEY_LOAD_DEFAULT2                   0xD01
//#define KEY_LOAD_DEFAULT3                   0xD02



#endif
