/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _IDE_CONFIG_H_
#define _IDE_CONFIG_H_


#include "SetupUtility.h"

#define bit(a)   (1 << (a))
EFI_STATUS
UpdateTransferMode (
  IN  EFI_HII_HANDLE                    HiiHandle,
  IN  EFI_ATAPI_IDENTIFY_DATA           *IdentifyInfo,
  IN  STRING_REF                        TokenToUpdate,
  IN  CHAR8                             *LanguageString
  );

#endif
