/** @file
  H2ORamDiskServices Protocol

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

#ifndef _H2O_RAM_DISK_SERVICES_PROTOCOL_H_
#define _H2O_RAM_DISK_SERVICES_PROTOCOL_H_

#include <Protocol/BlockIo.h>

#define H2O_RAM_DISK_SERVICES_PROTOCOL_GUID \
  { 0x2bd4fcac, 0x5b23, 0x4fb1, 0xae, 0x10, 0x6e, 0x6e, 0xfe, 0xe9, 0x6a, 0x77 }

typedef struct _H2O_RAM_DISK_SERVICES_PROTOCOL H2O_RAM_DISK_SERVICES_PROTOCOL;

#pragma pack(1)

/**
  RAM Disk device path structure.
  Will use Vendor Messaging Device Path.
**/
typedef struct {
  EFI_DEVICE_PATH_PROTOCOL      Header;
  EFI_GUID                      Guid;
  UINT8                         DiskId[8];
  EFI_DEVICE_PATH_PROTOCOL      EndDevicePath;
} H2O_RAM_DISK_DEVICE_PATH;

#pragma pack()

/**
  To create a new RAM Disk.

  @param[in]  Size            The size for new RAM Disk.
  @param[in]  Removable       If TRUE, then new created RAM Disk could be remove.
  @param[out] RamDiskId       The DiskId for new RAM Disk.

  @retval EFI_SUCCESS         Create new RAM Disk successfully.
  @retval other               Some error occurred when executing function.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_CREATE_RAM_DISK) (
  IN UINT32                    Size,
  IN BOOLEAN                   Removable,
  OUT UINT64                   *RamDiskId
  );

/**
  To remove a specified RAM Disk selected by DiskId.

  @param[in] RamDiskId      The RAM Disk with the same DiskId will be removed.

  @retval EFI_SUCCESS       Remove successfully.
  @retval other             Some error occurred when executing function.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_REMOVE_RAM_DISK) (
  IN UINT64                    RamDiskId
  );

/**
  Return current RAM Disk devices counts.

  @retval value      Number of RAM Disk devices.

**/ 
typedef
UINT32
(EFIAPI *H2O_GET_RAM_DISK_COUNT) (
  VOID
  );

/**
  Return current RAM Disk device information.

  @param[in]  DeviceNum         Number of RAM Disk device.
  @param[out] DiskSize          Size of the RAM Disk.
  @param[out] DiskId            DiskId of the RAM Disk.
  @param[out] Removable         If TRUE, it is a removable RAM Disk.
  
  @retval EFI_SUCCESS           Get RAM Disk information success.
  @retval other                 Cannot find specific RAM Disk.

**/
typedef
EFI_STATUS
(EFIAPI *H2O_GET_RAM_DISK_INFO) (
  IN  UINT32                    DeviceNum,
  OUT UINT64                    *DiskSize,
  OUT UINT64                    *DiskId,
  OUT BOOLEAN                   *Removable
  );

struct _H2O_RAM_DISK_SERVICES_PROTOCOL {
  H2O_CREATE_RAM_DISK                Create;
  H2O_REMOVE_RAM_DISK                Remove;
  H2O_GET_RAM_DISK_COUNT             GetRamDiskCount;
  H2O_GET_RAM_DISK_INFO              GetRamDiskInfo;
};

extern EFI_GUID gH2ORamDiskServicesProtocolGuid;
#endif
