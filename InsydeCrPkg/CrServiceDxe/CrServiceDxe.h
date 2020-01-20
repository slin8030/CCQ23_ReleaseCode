/** @file
  CrServiceDxe driver Header

;******************************************************************************
;* Copyright (c) 2012, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _CR_SERVICE_H_
#define _CR_SERVICE_H_

#include <Uefi.h>
#include <CrPolicyType.h>
#include <CrServiceMisc.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BdsCpLib.h>
#include <Library/CrDeviceVariableLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/VariableLib.h>
#include <Library/SelfDebugLib.h>

#include <Protocol/CROpROMInfo.h>
#include <Protocol/ConsoleRedirectionService.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LegacyBiosPlatform.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/TerminalEscCode.h>
#include <Protocol/Uart16550Access.h>
#include <Protocol/UsbSerialControllerIo.h>

#include <Guid/CrConfigHii.h>
#include <Guid/GlobalVariable.h>
#include <Guid/H2OBdsCheckPoint.h>

#include <IndustryStandard/Pci22.h>

#define CR_DEVICE_SETTING_FROM_THIS(a)  BASE_CR (a, CR_DEVICES_SETTING_NODE, Link)

extern BOOLEAN                               mCrEnable;
extern BOOLEAN                               mHeadless;
extern EFI_HANDLE                            mCrServiceImageHandle;
extern CR_POLICY_VARIABLE                    mCrPolicy;

typedef struct _CR_DEVICES_SETTING_NODE {
  LIST_ENTRY         Link;
  CR_DEVICE_SETTING* CrSetting;
  UINT8              VarIndex;
  BOOLEAN            AddIntoConVar;
} CR_DEVICES_SETTING_NODE;

typedef struct _CR_DEVICES_SETTING_HEAD_NODE {
  LIST_ENTRY         Link;
  UINTN              CrDevVarCount;
} CR_DEVICES_SETTING_HEAD_NODE;



#endif

