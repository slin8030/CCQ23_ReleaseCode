//;******************************************************************************
//;* Copyright (c) 1983-2006, Insyde Software Corporation. All Rights Reserved.
//;*
//;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
//;* transmit, broadcast, present, recite, release, license or otherwise exploit
//;* any part of this publication in any form, by any means, without the prior
//;* written permission of Insyde Software Corporation.
//;*
//;
//; Abstract:  ver 2
//;

#ifndef _UTILITY_SCU_ALL_PAGE_H_
#define _UTILITY_SCU_ALL_PAGE_H_

#include <BiosSet.h>

#define scu_PARM            L"-scu"
#define scu_HELP_STRING     L"    -scu  :Enable/Disable BIOS SCU show all page(1:Enable; 0:Disable).\n"
#define scu_HELP_EXAMPLE    L"           Example: BiosSet.efi -scu 1\n"

EFI_STATUS
SCU_AllPage (
  LIST_ENTRY                *List
  );

#endif
