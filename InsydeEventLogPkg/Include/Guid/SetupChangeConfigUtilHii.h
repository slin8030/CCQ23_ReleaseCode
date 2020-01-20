/** @file

   The definition of Setup Change Config utility HII.

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

#ifndef _SETUP_CHANGE_CONFIG_UTIL_HII_H_
#define _SETUP_CHANGE_CONFIG_UTIL_HII_H_

#define H2O_SETUP_CHANGE_CONFIG_UTIL_EVENT_AND_MESSAGE_PAGE_FORMSET_GUID \
  { \
    0x7478B246, 0xFB05, 0x42D9, {0x9E, 0x8C, 0x72, 0x6F, 0x30, 0x15, 0xD0, 0x9E} \
  } 

extern EFI_GUID gH2OSetupChangeConfigUtilEventAndMessageFormsetGuid;

#endif
