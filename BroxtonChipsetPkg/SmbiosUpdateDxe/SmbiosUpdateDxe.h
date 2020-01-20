/** @file
  SmbiosUpdateDxe.h 

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SMBIOS_UPDATE_DXE_H_
#define _SMBIOS_UPDATE_DXE_H_

#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CommonSmbiosLib.h>
#include <IndustryStandard/SmBios.h>
#include <Protocol/Smbios.h>
//#include <Protocol/Runtime.h>
#include <Protocol/Smbios.h>
#include <Protocol/MemInfo.h>
//[-start-160218-IB08450338-remove]//
//#include "NorthCluster\SmBiosMemory\Dxe\SmbiosMemory.h"
//[-end-160218-IB08450338-remove]//
#include <CommonSmbiosMemory.h>

#endif
