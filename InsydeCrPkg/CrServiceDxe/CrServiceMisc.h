/** @file
  CrServiceMisc

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
#ifndef _CR_SERVICE_MISC_H_
#define _CR_SERVICE_MISC_H_

#include "CrServiceDxe.h"

VOID
CrSafeFreePool (
  IN  VOID                      *Buffer
  );

EFI_STATUS
GetParentHandle (
  IN  EFI_HANDLE      ChildHandle,
  OUT EFI_HANDLE      *ParentHandle
  );

VOID
TruncateLastNode (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  );

VOID
PointToLastNode (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath
  );

EFI_STATUS
GetProtocolWithLocateDevicePath (
  IN     EFI_GUID                   *Protocol,
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath,
  OUT    VOID                       **Interface
  );

#endif

