/** @file

Declaration file for SioMainPei.c

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SIO_IT8987E_PEI_H_
#define _SIO_IT8987E_PEI_H_

#include <PostCode.h>
#include <../SioIt8987eDxe/SioHiiResource/SioHiiResourceNvData.h>
#include <IndustryStandard/Pci22.h>
#include <Include/SioIt8987eReg.h>
#include <Ppi/SioInitialized.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/IoLib.h>
#include <Library/SioLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PostCodeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/SioIoDecodeLib.h>
#include <SioCommon.h>
#include <Guid/SioHiiResourceForm.h>
#include <Include/SioGetPcd.h>

/**
 Enter SIO mode.

 @param[in]   ConfigPort             SIO config port.
**/
VOID
EnterConfigMode (
  IN UINT16  ConfigPort
  );

/**
 Exit SIO mode.

 @param[in]   ConfigPort             SIO config port.
**/
VOID
ExitConfigMode (
  IN UINT16  ConfigPort
  );

/**
  Decode IO Port By Different Platform, Like AMD¡BINTEL¡K

  @retval     EFI_SUCCESS          The operation performed successfully.
**/
EFI_STATUS
SioDecode (
  VOID
  );

/**
  To get correct SIO data

  @param[out]  *SioInstance         if success, return which SIO instance pointer.

  @retval     IndexPort             return value as zero, it means "not match"
                                    return value is not zero, it means "match".
**/
UINT16
CheckDevice (
  UINT8               *SioInstance
  );

/**
  Check ID of SIO whether it's correct.

  @param[in]  *DeviceTable          SIO_DEVICE_LIST_TABLE from PCD structure.
  @param[in]  *IndexPoart           SIO config port.

  @retval     IndexPort             return value as zero, it means "not match" .                                 return value is not zero, it means "match"
**/
VOID
CheckConfig (
  IN SIO_DEVICE_LIST_TABLE      *DeviceTable,
  IN UINT16                     *IndexPoart
  );

EFI_STATUS
WriteOnOff (
  UINT16            IndexPort,
  UINT8             Ldn,
  BOOLEAN           OnOff
  );

EFI_STATUS
WriteMode (
  UINT16            IndexPort,
  UINT8             Ldn,
  UINT8             Mode
  );

EFI_STATUS
WriteCounter (
  UINT16            IndexPort,
  UINT8             Ldn,
  UINT16            Counter
  );

EFI_STATUS
ProgramExtensiveDevicePei (
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN VOID                        *Ppi
  );

EFI_STATUS
SioInitPs2KBMS (
  IN UINT16 IndexPort,
  IN UINT8  SioInstance
  );

EFI_STATUS
SioDebugPortInit (
  IN UINT16 IndexPort
  );

EFI_STATUS
SioSpecificInit (
  IN UINT16 IndexPort
  );

extern CHAR16 *mSioVariableName;
#endif
