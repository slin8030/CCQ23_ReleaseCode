/** @file
  The header file for TcgMor.

;******************************************************************************
;* Copyright (c) 2013 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __TCG_MOR_H__
#define __TCG_MOR_H__

#include <PiDxe.h>

#include <Guid/MemoryOverwriteControl.h>

#include <Protocol/TcgService.h>
#include <Protocol/Tcg2Protocol.h>
#include <Protocol/StorageSecurityCommand.h>
#include <Protocol/BlockIo.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryOverwriteLib.h>
#include <Library/MemoryAllocationLib.h>

#include <TpmPolicy.h>

#pragma pack(1)
typedef struct {
  TPM_RQU_COMMAND_HDR       Header;
  UINT32                    capArea;
  UINT32                    subCapSize;
  UINT32                    subCap;
} TPM_GET_PROP_OWNER_COMMAND;

typedef struct {
  TPM_RSP_COMMAND_HDR       Header;
  UINT32                    respSize;
  UINT8                     resp;
} TPM_GET_PROP_OWNER_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPM_CAP                   Capability;
  UINT32                    Property;
  UINT32                    PropertyCount;
} TPM2_GET_PERMANENT_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER      Header;
  TPMI_YES_NO               MoreData;
  TPMS_CAPABILITY_DATA      CapabilityData;
} TPM2_GET_PERMANENT_RESPONSE;
#pragma pack()

//
// Supported Security Protocols List Description.
// Refer to ATA8-ACS Spec 7.57.6.2 Table 69 or SPC4 7.7.1.3 Table 511.
//
typedef struct  {
  UINT8                            Reserved1[6];
  UINT8                            SupportedSecurityListLength[2];
  UINT8                            SupportedSecurityProtocol[1];
} SUPPORTED_SECURITY_PROTOCOLS_PARAMETER_DATA;

#define SECURITY_PROTOCOL_TCG      0x02
#define SECURITY_PROTOCOL_IEEE1667 0xEE

#define ROUNDUP512(x) (((x) % 512 == 0) ? (x) : ((x) / 512 + 1) * 512)

#endif
