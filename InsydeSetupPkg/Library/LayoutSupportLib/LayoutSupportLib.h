/** @file
 Function definition for Layout Package Lib

;******************************************************************************
;* Copyright (c) 2013 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

*/

#ifndef _LAYOUT_PACKAGE_LIB_H_
#define _LAYOUT_PACKAGE_LIB_H_

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/LayoutLib.h>
#include <Library/HiiLib.h>
#include <Library/HiiExLib.h>
#include <Library/UefiHiiServicesLib.h>

#include <Protocol/H2OFormBrowser.h>
#include <Protocol/H2OKeyDesc.h>
#include <Protocol/LayoutDatabase.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiFont.h>

H2O_FORM_INFO *
GetFormLayoutByFormId (
  IN H2O_FORMSET_INFO                            *Formset,
  IN UINT32                                      FormId
  );

EFI_STATUS
GetPropertyFromVfrInVfcf (
  IN     EFI_GUID                              *FormsetGuid,
  IN     UINT32                                FormId,
  IN     UINT32                                HotkeyId,
  IN     UINT32                                StyleType,
  IN     UINT32                                PseudoClass,
  IN     CHAR8                                 *IdentifierStr,
  IN     RECT                                  *RelativeField OPTIONAL,
  OUT    H2O_PROPERTY_INFO                     **Property,
  OUT    H2O_PROPERTY_VALUE                    *PropValue
  );


EFI_STATUS
GetLayoutTreeAndVfrTree (
  OUT    LIST_ENTRY                              **LayoutListHead,
  OUT    LIST_ENTRY                              **VfrListHead
  );

#endif

