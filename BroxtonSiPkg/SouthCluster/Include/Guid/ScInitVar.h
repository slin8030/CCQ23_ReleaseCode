/** @file
  This file defines variable shared between SC Init DXE driver and SC
  Init S3 Resume PEIM.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
#ifndef _SC_INIT_VAR_H_
#define _SC_INIT_VAR_H_

//
// Define the SC Init Var GUID
//
#define INIT_VARIABLE_GUID {0xe6c2f70a, 0xb604, 0x4877,{0x85, 0xba, 0xde, 0xec, 0x89, 0xe1, 0x17, 0xeb}}
//
// Extern the GUID for PPI users.
//
extern EFI_GUID gScInitVariableGuid;

#define INIT_VARIABLE_NAME  L"PchInit"

//
// Define the SC Init Variable structure
//
typedef struct {
  UINT32  StorePosition;
  UINT32  ExecutePosition;
} SC_S3_PARAMETER_HEADER;

#pragma pack(1)
typedef struct _SC_INIT_VARIABLE {
  SC_S3_PARAMETER_HEADER *ScS3Parameter;
} SC_INIT_VARIABLE;
#pragma pack()

#endif
