/** @file

   The definition of H2O Event Log Config Manager HII.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _H2O_EVENT_LOG_CONFIG_MANAGER_HII_H_
#define _H2O_EVENT_LOG_CONFIG_MANAGER_HII_H_

#define H2O_EVENT_LOG_CONFIG_MANAGER_FORMSET_GUID \
  { \
    0x236A7970, 0x5B07, 0x442B, {0x93, 0xDB, 0x32, 0x74, 0x00, 0xE9, 0x6C, 0xFA} \
  } 

#define LINK_INCLUDE_H2O_EVENT_LOG_CONFIG_PAGE_CLASS_GUID \
  { \
    0x2449086C, 0xFC42, 0x4FD2, {0x88, 0xB3, 0x3F, 0xAA, 0x35, 0x2A, 0xC4, 0x1D} \
  } 

#define LINK_INCLUDE_H2O_EVENT_LOG_EVENT_AND_MESSAGE_PAGE_CLASS_GUID \
  { \
    0x8568BD98, 0xA787, 0x4C3A, {0xBA, 0xCD, 0x21, 0xDC, 0xB9, 0xB5, 0x93, 0x0B} \
  } 

extern EFI_GUID gH2OEventLogConfigManagerFormsetGuid;

#endif
