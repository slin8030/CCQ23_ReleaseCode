/** @file
  Required by Microsoft Windows to report the available debug ports on the platform.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1996 - 2016 Intel Corporation.

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

#ifndef _DBG2_H
#define _DBG2_H

#include <IndustryStandard/Acpi50.h>

//
// Definitions
//
#define EFI_ACPI_CREATOR_REVISION 0x0100000D

//
// DBG2 Definitions
//
#define EFI_ACPI_OEM_DBG2_TABLE_REVISION 0x00000000
#define NUMBER_DBG_DEVICE_INFO 1
#define DEBUG_DEVICE_INFORMATION_REVISION 0x00
#define NAMESPACE_STRING_MAX_LENGTH 32
#define EFI_ACPI_OEM_TABLE_ID_2 SIGNATURE_64('I','N','T','L','D','B','G','2') ///<Oem table Id for Dbg2

//
// DBG2 ACPI define
//
#pragma pack(1)

typedef struct _DEBUG_DEVICE_INFORMATION {
  UINT8  Revision;
  UINT16 Length;
  UINT8  NumberOfGenericAddressRegisters;
  UINT16 NameSpaceStringLength;
  UINT16 NameSpaceStringOffset;
  UINT16 OemDataLength;
  UINT16 OemDataOffset;
  UINT16 PortType;
  UINT16 PortSubtype;
  UINT16 Reserved;
  UINT16 BaseAddressRegisterOffset;
  UINT16 AddressSizeOffset;
  EFI_ACPI_5_0_GENERIC_ADDRESS_STRUCTURE BaseAddressRegister[1];
  UINT32 AddressSize[1];
  CHAR8  NamespaceString[NAMESPACE_STRING_MAX_LENGTH];
} DEBUG_DEVICE_INFORMATION;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER            Header;
  UINT32                                 OffsetDbgDeviceInfo;
  UINT32                                 NumberDbgDeviceInfo;
  DEBUG_DEVICE_INFORMATION               DbgDeviceInfoCom1;
} EFI_ACPI_DEBUG_PORT_2_TABLE;

#pragma pack()

#endif
