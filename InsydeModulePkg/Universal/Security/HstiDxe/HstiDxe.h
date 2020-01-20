/** @file
  Header file for HSTI DXE Driver's Data Structures

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _HSTI_DXE_H_
#define _HSTI_DXE_H_

#include <Library/UefiLib.h>
#include <Guid/DebugMask.h>
#include <Guid/GlobalVariable.h>
#include <IndustryStandard/Hsti.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/HstiLib.h>

#include <Protocol/Hash.h>
#include <Protocol/CryptoServices.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/AdapterInformation.h>

#define HSTI_IBV_IMPLEMENT_ID          L"Insyde Software Secure Feature"
#define HSTI_IBV_FEATUER_SIZE          0x3

#define RSA2048_SHA256_SUPPORT         BIT0
#define CSM_SECURITY_POLICY            BIT1
#define FIRMWARE_CODE_PROTECT          BIT2
#define SEUCRE_FIRMWARE_UPDATE         BIT3
#define SEUCRE_BOOT_POLICY             BIT4
#define EXTERNAL_DMA_PROTECT           BIT5

#define HSTI_FEATURE_ALL_ENABLE       \
  (RSA2048_SHA256_SUPPORT | CSM_SECURITY_POLICY    | FIRMWARE_CODE_PROTECT |\
   SEUCRE_FIRMWARE_UPDATE | SEUCRE_BOOT_POLICY     | EXTERNAL_DMA_PROTECT)


#endif
