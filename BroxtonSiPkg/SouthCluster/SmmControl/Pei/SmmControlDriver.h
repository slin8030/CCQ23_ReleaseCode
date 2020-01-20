/**@file
  Header file for SMM Control Driver.

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
#ifndef _EFI_PEI_SMM_CONTROL_DRIVER_H_
#define _EFI_PEI_SMM_CONTROL_DRIVER_H_

///
/// Driver private data
///
#include <PiPei.h>
#include "Ppi/SmmControl.h"
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <ScAccess.h>


///
/// Prototypes
///
/**

  @brief
  This is the constructor for the SMM Control ppi

  @param[in] FfsHeader            FfsHeader.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_STATUS              Results of the installation of the SMM Control Ppi

**/
EFI_STATUS
EFIAPI
SmmControlPeiDriverEntryInit (
  IN      EFI_PEI_FILE_HANDLE             FfsHeader,
  IN      CONST EFI_PEI_SERVICES          **PeiServices
  )
;

/**

  @brief
  This routine generates an SMI

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] This                 The EFI SMM Control ppi instance
  @param[in] ArgumentBuffer       The buffer of argument
  @param[in] ArgumentBufferSize   The size of the argument buffer
  @param[in] Periodic             Periodic or not
  @param[in] ActivationInterval   Interval of periodic SMI

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
EFI_STATUS
EFIAPI
PeiActivate (
  IN       EFI_PEI_SERVICES          **PeiServices,
  IN       PEI_SMM_CONTROL_PPI       *This,
  IN OUT  INT8                       *ArgumentBuffer OPTIONAL,
  IN OUT  UINTN                      *ArgumentBufferSize OPTIONAL,
  IN      BOOLEAN                    Periodic OPTIONAL,
  IN      UINTN                      ActivationInterval OPTIONAL
  )
;


/**

  @brief
  This routine clears an SMI

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] This                 The EFI SMM Control ppi instance
  @param[in] Periodic             Periodic or not

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
EFI_STATUS
EFIAPI
PeiDeactivate (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN PEI_SMM_CONTROL_PPI          *This,
  IN  BOOLEAN                     Periodic OPTIONAL
  )
;

#endif
