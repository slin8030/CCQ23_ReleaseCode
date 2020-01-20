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
#include "CrServiceMisc.h"

/**

  Free pool buffer safely.

  @param Buffer      Point to the specific buffer that want to free.

**/
VOID
CrSafeFreePool (
  IN  VOID             *Buffer
  )
{
  if (Buffer != NULL) {
    gBS->FreePool (Buffer);
  }
}

/**

  Point to the device path's last node.

  @param        DevicePath              A pointer to device path protocol,
                                        and retrun a pointer to last node's device path.

  @retval       None.

**/
VOID
PointToLastNode (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *LastNode = NULL;
  EFI_DEVICE_PATH_PROTOCOL    *DevPath;

  DevPath = *DevicePath;

  while (!IsDevicePathEnd(DevPath)) {
    LastNode = DevPath;
    DevPath  = NextDevicePathNode (DevPath);
  }
  *DevicePath = LastNode;
  return;
}

/**

  Truncate the device path's last node.

  @param        DevicePath              Input a pointer to device path protocol,
                                        and retrun a truncated device path.

  @retval       None.

**/
VOID
TruncateLastNode (
  IN OUT EFI_DEVICE_PATH_PROTOCOL   *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL    *LastNode = NULL;
  EFI_DEVICE_PATH_PROTOCOL    *DevPath;

  DevPath = DevicePath;

  while (!IsDevicePathEnd(DevPath)) {
    LastNode = DevPath;
    DevPath  = NextDevicePathNode (DevPath);
  }

  if (LastNode != NULL) {
    CopyMem( LastNode, DevPath, sizeof(EFI_DEVICE_PATH_PROTOCOL));
  }

  return;
}

/**

  Give the ChildHandle and ParentHandle

  @param        ChildHandle             Child device handle.
  @param        ParentHandle            Parent device handle.

  @retval       EFI_SUCCESS             Get parent handle.
  @retval       EFI_NOT_FOUND           Can't find child device's parent handle.
  @retval       EFI_ABORTED             Can't duplicate device path.

**/
EFI_STATUS
GetParentHandle (
  IN  EFI_HANDLE      ChildHandle,
  OUT EFI_HANDLE      *ParentHandle
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    Handle;
  EFI_DEVICE_PATH_PROTOCOL      *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL      *TmpDevPath;
  EFI_DEVICE_PATH_PROTOCOL      *OrgDevPath;

  Status = gBS->HandleProtocol (ChildHandle, &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  OrgDevPath = DuplicateDevicePath (DevicePath);
  if (OrgDevPath == NULL) {
    return EFI_ABORTED;
  }
  TmpDevPath = OrgDevPath;
  TruncateLastNode (TmpDevPath);
  Status = gBS->LocateDevicePath (&gEfiDevicePathProtocolGuid, &TmpDevPath, &Handle);

  if (!EFI_ERROR(Status) && IsDevicePathEnd(TmpDevPath)) {
    *ParentHandle = Handle;

    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_FOUND;
  }

  gBS->FreePool (OrgDevPath);

  return Status;
}

EFI_STATUS
GetProtocolWithLocateDevicePath (
  IN     EFI_GUID                   *Protocol,
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **DevicePath,
  OUT    VOID                       **Interface
  )
{
  EFI_HANDLE  Handle;
  EFI_STATUS  Status;

  Status = gBS->LocateDevicePath (Protocol, DevicePath, &Handle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return gBS->HandleProtocol (Handle, Protocol, Interface);
}

