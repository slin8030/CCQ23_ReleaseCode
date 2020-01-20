/** @file
  Tpm Physical Presence Protocol

;******************************************************************************
;* Copyright (c) 2012 - 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _TPM_PHYSICAL_PRESENCE_PROTOCOL_H_
#define _TPM_PHYSICAL_PRESENCE_PROTOCOL_H_

#include <Protocol/TcgService.h>
#include <Guid/PhysicalPresenceData.h>

#define EFI_TPM_PHYSICAL_PRESENCE_PROTOCOL_GUID  \
  {0x15f81fc9, 0x9304, 0x407b, 0x89, 0xc4, 0x2c, 0xac, 0x35, 0x86, 0x10, 0xa7}

typedef struct _EFI_TPM_PHYSICAL_PRESENCE_PROTOCOL EFI_TPM_PHYSICAL_PRESENCE_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_TPM_PHYSICAL_PRESENCE) (
  IN  EFI_TCG_PROTOCOL                  *TcgProtocol,
  IN  UINT32                            CommandCode,
  OUT UINT32                            *Result
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TCG_GET_TPM_MANAGEMENT_FLAG) (
  IN  EFI_TCG_PROTOCOL                  *TcgProtocol,
  OUT EFI_PHYSICAL_PRESENCE_FLAGS       *TpmMangFlag
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TCG_MANIPULATE_PHYSICAL_PRESENCE) (
  IN  EFI_TCG_PROTOCOL                  *TcgProtocol,
  IN  TPM_PHYSICAL_PRESENCE             PhysicalPresence
  );

typedef
UINT32
(EFIAPI *EFI_TPM_NV_LOCK) (
  IN  EFI_TCG_PROTOCOL                  *TcgProtocol
  );

struct _EFI_TPM_PHYSICAL_PRESENCE_PROTOCOL {
  EFI_TPM_PHYSICAL_PRESENCE             TpmPhysicalPresence;
  EFI_TCG_GET_TPM_MANAGEMENT_FLAG       TcgGetTpmMangFlag;
  EFI_TCG_MANIPULATE_PHYSICAL_PRESENCE  ManipulatePhysicalPresence;
  EFI_TPM_NV_LOCK                       TpmNvLock;
};

extern EFI_GUID gEfiTpmPhysicalPresenceProtocolGuid;

#endif
