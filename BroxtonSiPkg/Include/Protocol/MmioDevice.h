/** @file
  MMIO device protocol as defined in the UEFI 2.x.x specification.

  The MMIO device protocol defines a memory mapped I/O device
  for use by the system.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/

#ifndef __MMIO_DEVICE_H__
#define __MMIO_DEVICE_H__

///
/// Protocol to define for the MMIO device
//
typedef struct {
  ///
  /// Address of a GUID
  ///
  EFI_GUID *Guid;

  ///
  /// Context for the protocol
  ///
  VOID *Context;
} EFI_MMIO_DEVICE_PROTOCOL_ITEM;


typedef struct _EFI_MMIO_DEVICE_PROTOCOL  EFI_MMIO_DEVICE_PROTOCOL;

///
///  The MMIO device protocol defines a memory mapped I/O device
///  for use by the system.
///
struct _EFI_MMIO_DEVICE_PROTOCOL {
  ///
  /// Pointer to an ACPI_EXTENDED_HID_DEVICE_PATH structure
  /// containing HID/HidStr and CID/CidStr values.
  ///
  /// See the note below associated with the UnitIdentification
  /// field.
  ///
  ACPI_HID_DEVICE_PATH *AcpiPath;

  ///
  /// Allow the use of a shared template for the AcpiPath.
  ///
  /// If this value is non-zero UID value then the AcpiPath must
  /// be a template which contains only the HID/HidStr and CID/CidStr
  /// values.  The UID/UidStr values in the AcpiPath must be zero!
  //
  /// If this value is zero then the AcpiPath is not shared and
  /// must contain either a non-zero UID value or a UidStr value.
  ///
  UINT32 UnitIdentification;

  ///
  /// Hardware revision - ACPI _HRV value
  ///
  UINT32 HardwareRevision;

  ///
  /// Pointer to a data structure containing the controller
  /// resources and configuration.  At a minimum this points
  /// to an EFI_PHYSICAL_ADDRESS for the base address of the
  /// MMIO device.
  ///
  VOID *DriverResources;

  ///
  /// Number of protocols in the array
  ///
  UINTN ProtocolCount;

  ///
  /// List of protocols to define
  ///
  EFI_MMIO_DEVICE_PROTOCOL_ITEM *ProtocolArray;
};

extern EFI_GUID gEfiMmioDeviceProtocolGuid;

#endif  //  __MMIO_DEVICE_H__
