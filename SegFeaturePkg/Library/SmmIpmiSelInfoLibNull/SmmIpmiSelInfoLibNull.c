/** @file
 H2O IPMI SEL Info library implement code.

 This c file contains SEL Info library instance in SMM phase.

;******************************************************************************
;* Copyright (c) 2013 - 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/
  
#include <Library/IpmiLibDefine.h>

/**
 Convert SEL entry to string based SEL entry read from "Get SEL Entry" IPMI Command.

 @param[in]         SelEntry            SEL entry to be coverted.
 @param[out]        SelInfo             Structure contains SEL entry information description strings.
                                        Callers should pass in structure buffer.

 @retval EFI_UNSUPPORTED                Function Unsupported.
*/
EFI_STATUS
IpmiLibGetSelInfo (
  IN  VOID                              *SelEntry,
  OUT SEL_INFO_STRUCT                   *SelInfo
  )
{
  return EFI_UNSUPPORTED;
}

