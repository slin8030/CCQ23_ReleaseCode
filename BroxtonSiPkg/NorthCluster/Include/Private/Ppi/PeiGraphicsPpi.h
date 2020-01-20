/** @file
  Header file for the GraphicsPpi.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

@par Specification Reference:
**/

#ifndef _PEI_GRAPHICS_PPI_H_
#define _PEI_GRAPHICS_PPI_H_

#include <Protocol/GraphicsOutput.h>

//extern EFI_GUID gPeiGraphicsPpiGuid;

//
// Forward reference for ANSI C compatibility
//
typedef struct _PEI_GRAPHICS_PPI PEI_GRAPHICS_PPI;

//
//  Define the PeiGraphicsPpi structures
//
enum {
    ONE_SHOT = 0,
    PHASE_1 = 1,
    PHASE_2 = 2,
    PHASE_3 = 3
} PHASE_MODEL;

//
// Define the Graphics Platform PPIs
//
/**
  Graphics initial on pei phase.

  @param[in] Phase  The initial phase.

  @retval  EFI_STATUS
**/
typedef
EFI_STATUS
(EFIAPI *PEI_GRAPHICS_INIT) (
    IN CONST UINTN Phase
    );

/**
  Get graphics mode on pei phase.

  @param[in,out]  Mode  Get the type of mode.

  @retval  EFI_STATUS
**/
typedef
EFI_STATUS
(EFIAPI *PEI_GRAPHICS_GET_MODE)(
    IN OUT EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode
    );

struct _PEI_GRAPHICS_PPI {
    PEI_GRAPHICS_INIT       GraphicsPpiInit;
    PEI_GRAPHICS_GET_MODE   GraphicsPpiGetMode;
    };

#endif
