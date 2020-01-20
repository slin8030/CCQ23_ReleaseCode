/** @file

  Header file of Store DEBUG() Message Dxe implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _STORE_DEBUG_MESSAGE_DXE_H_
#define _STORE_DEBUG_MESSAGE_DXE_H_

//
// Statements that include other header files
//
#include <Protocol/VariableWrite.h>
#include <Protocol/ReportStatusCodeHandler.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
//[-start-180503-IB08400586-add]//
#include <Library/VariableLib.h>
//[-end-180503-IB08400586-add]//

#include <Guid/StatusCodeDataTypeDebug.h>
#include <Guid/EventGroup.h>
#include <Guid/DebugMessageVariable.h>
#include <Guid/StatusCodeDataTypeId.h>
//[-start-170413-IB08400444-add]//
#include <Protocol/SmmCommunication.h>
//[-end-170413-IB08400444-add]//

#define MAX_DEBUG_MESSAGE_LENGTH                0x100

VOID
EFIAPI
InstalledVarNotificationFunction (
  IN  EFI_EVENT       Event,
  IN  VOID            *Context
  );

EFI_STATUS
EFIAPI
DebugMessageMemBufferInit (
  VOID
  );

static
VOID
EFIAPI
RscHandlerProtocolCallback (
  IN EFI_EVENT   Event,
  IN VOID        *Context
  );

//[-start-170413-IB08400444-modify]//
VOID
EFIAPI
UnregisterBootTimeHandlers (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );
//[-end-170413-IB08400444-modify]//

EFI_STATUS
EFIAPI
DebugMessageStatusCode (
  IN EFI_STATUS_CODE_TYPE           CodeType,
  IN EFI_STATUS_CODE_VALUE          CodeValue,
  IN UINT32                         Instance    OPTIONAL,
  IN EFI_GUID                       * CallerId  OPTIONAL,
  IN EFI_STATUS_CODE_DATA           * Data      OPTIONAL  
  ); 

EFI_STATUS
EFIAPI
StoreAsciiDebugMessage (
  IN CHAR8                                 *MessageString,
  IN UINTN                                 StringSize
  ); 

BOOLEAN
EFIAPI
ExtractAssertInfo (
  IN EFI_STATUS_CODE_TYPE        CodeType,
  IN EFI_STATUS_CODE_VALUE       Value,
  IN CONST EFI_STATUS_CODE_DATA  *Data,
  OUT CHAR8                      **Filename,
  OUT CHAR8                      **Description,
  OUT UINT32                     *LineNumber
  );

EFI_STATUS
EFIAPI
StoreAsciiDebugMessage (
  IN CHAR8                                 *MessageString,
  IN UINTN                                 StringSize
  );

//[-start-180503-IB08400586-add]//
EFI_STATUS
EFIAPI
SaveDebugMessageVariable (
  VOID
  );
//[-end-180503-IB08400586-add]//
#endif
