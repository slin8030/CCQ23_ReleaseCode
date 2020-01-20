/** @file
  TrEE Physical Presence Protocol

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

#ifndef _TREE_PHYSICAL_PRESENCE_PROTOCOL_H_
#define _TREE_PHYSICAL_PRESENCE_PROTOCOL_H_

#include "IndustryStandard/Tpm12.h"
#include <Guid/TrEEPhysicalPresenceData.h>
#include <Guid/Tcg2PhysicalPresenceData.h>
#include <Library/Tcg2PhysicalPresenceLib.h>

#define EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL_GUID  \
  {0x469718df, 0x6f19, 0x43e7, 0x90, 0x3b, 0xea, 0x6b, 0x3, 0x55, 0x55, 0xbb}

typedef struct _EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL;

#ifdef TCG2_BIOS_TPM_MANAGEMENT_FLAG_DEFAULT
#undef TCG2_BIOS_TPM_MANAGEMENT_FLAG_DEFAULT
#endif
//
// Default value
// TODO: The recommended value in PPI spec 1.30 includes TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF; 
// TODO: However, for backward compatibility with 1.20, TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_TURN_OFF is not set currently.
//
#define TCG2_BIOS_TPM_MANAGEMENT_FLAG_DEFAULT  (TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CLEAR | \
                                                TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_EPS | \
                                                TCG2_BIOS_TPM_MANAGEMENT_FLAG_PP_REQUIRED_FOR_CHANGE_PCRS)
/**
  Execute physical presence operation requested by the OS.

  @param[in]      This                        EFI TrEE Protocol instance.
  @param[in]      CommandCode                 Physical presence operation value.
  @param[out]     LastReturnedCode            Execute result
  @param[in, out] PpiFlags                    The physical presence interface flags.

  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
**/
typedef
EFI_STATUS
(EFIAPI *EFI_EXECUTE_TREE_PHYSICAL_PRESENCE) (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TREE_PHYSICAL_PRESENCE_FLAGS    *PpiFlags OPTIONAL
  );

/**
  Execute physical presence operation with TCG2 definition.

  @param[in]      This                        EFI TrEE Protocol instance.
  @param[in]      CommandCode                 Physical presence operation value.
  @param[out]     LastReturnedCode            Execute result
  @param[in, out] PpiFlags                    Physical presence interface flags.
  @param[in]      CommandParameter            Physical presence operation parameter.

  @retval EFI_SUCCESS                         Operation completed successfully.
  @retval EFI_INVALID_PARAMETER               One or more of the parameters are incorrect.
  @retval Others                              Unexpected error.
  
**/
typedef
EFI_STATUS
(EFIAPI *EFI_EXECUTE_TREE_PHYSICAL_PRESENCE_EX) (
  IN      EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL *This,
  IN      UINT32                              CommandCode,
  OUT     UINT32                              *LastReturnedCode,
  IN OUT  EFI_TCG2_PHYSICAL_PRESENCE_FLAGS    *PpiFlags OPTIONAL,
  IN      UINT32                              CommandParameter
  );

struct _EFI_TREE_PHYSICAL_PRESENCE_PROTOCOL {
  EFI_EXECUTE_TREE_PHYSICAL_PRESENCE          ExecutePhysicalPresence;
  EFI_EXECUTE_TREE_PHYSICAL_PRESENCE_EX       ExecutePhysicalPresenceEx;
};

extern EFI_GUID gEfiTrEEPhysicalPresenceProtocolGuid;
#endif //_TREE_PHYSICAL_PRESENCE_PROTOCOL_H_
