/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 0 - 2016 Intel Corporation.

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/
//#define EFI_ACPI_5_0_CORE_SYSTEM_RESOURCE_TABLE_SIGNATURE  SIGNATURE_32('C', 'S', 'R', 'T')
//
// CSRT Definitions
//
//#define EFI_ACPI_CSRT_SIGNATURE SIGNATURE_32 ('C', 'S', 'R', 'T') // "CSRT"

#define EFI_ACPI_CSRT_TABLE_REVISION 0x00000000

#define NUMBER_RESOURCE_GROUP_INFO 1 //2

#define MAX_NO_CHANNEL1_SUPPORTED 7
#define MAX_NO_CHANNEL2_SUPPORTED 9

#define NAMESPACE_STRING_MAX_LENGTH 16

//
// Ensure proper structure formats
//
#pragma pack (1)


typedef struct _SHARED_INFO_SECTION {
  UINT16 MajVersion;
  UINT16 MinVersion;
  UINT32 MMIOLowPart;
  UINT32 MMIOHighPart;
  UINT32 IntGSI;
  UINT8 IntPol;
  UINT8 IntMode;
  UINT8 NoOfCh;
  UINT8 DMAAddressWidth;
  UINT16 BaseReqLine;
  UINT16 NoOfHandSig;
  UINT32 MaxBlockTransferSize;
  } SHARED_INFO_SECTION;

typedef struct _RESOURCE_GROUP_HEADER {
  UINT32 Length;
  UINT32 VendorId;
  UINT32 SubVendorId;
  UINT16 DeviceId;
  UINT16 SubDeviceId;
  UINT16 Revision;
  UINT16 Reserved;
  UINT32 SharedInfoLength;
  SHARED_INFO_SECTION SharedInfoSection;
} RESOURCE_GROUP_HEADER;

typedef struct _RESOURCE_DESCRIPTOR {
  UINT32 Length;
  UINT16 ResourceType;
  UINT16 ResourceSubType;
  UINT32 UUID;
  } RESOURCE_DESCRIPTOR;

typedef struct {
  RESOURCE_GROUP_HEADER          ResourceGroupHeaderInfo;
  RESOURCE_DESCRIPTOR            ResourceDescriptorInfo[MAX_NO_CHANNEL1_SUPPORTED];
 } RESOURCE_GROUP_INFO1;

typedef struct {
  RESOURCE_GROUP_HEADER          ResourceGroupHeaderInfo;
  RESOURCE_DESCRIPTOR            ResourceDescriptorInfo[MAX_NO_CHANNEL2_SUPPORTED];
 } RESOURCE_GROUP_INFO2;

//
// DBGP structure
//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER            Header;
  RESOURCE_GROUP_INFO1				ResourceGroupsInfo1;
  RESOURCE_GROUP_INFO2				ResourceGroupsInfo2;
} EFI_ACPI_CSRT_TABLE;

#pragma pack ()