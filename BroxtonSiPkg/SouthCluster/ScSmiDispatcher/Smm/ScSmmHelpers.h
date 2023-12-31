/** @file
  Helper functions for SC SMM

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
#ifndef SC_SMM_HELPERS_H
#define SC_SMM_HELPERS_H

#include "ScSmm.h"
#include "ScxSmmHelpers.h"

//
// SUPPORT / HELPER FUNCTIONS (SC version-independent)
//
/**
  Publish SMI Dispatch protocols.
**/
VOID
ScSmmPublishDispatchProtocols (
  VOID
  );

/**
  Compare 2 SMM source descriptors' enable settings.

  @param[in] Src1                 Pointer to the SC SMI source description table 1
  @param[in] Src2                 Pointer to the SC SMI source description table 2

  @retval TRUE                    The enable settings of the 2 SMM source descriptors are identical.
  @retval FALSE                   The enable settings of the 2 SMM source descriptors are not identical.
**/
BOOLEAN
CompareEnables (
  CONST IN SC_SMM_SOURCE_DESC *Src1,
  CONST IN SC_SMM_SOURCE_DESC *Src2
  );

/**
  Compare 2 SMM source descriptors' statuses.

  @param[in] Src1                 Pointer to the SC SMI source description table 1
  @param[in] Src2                 Pointer to the SC SMI source description table 2

  @retval TRUE                    The statuses of the 2 SMM source descriptors are identical.
  @retval FALSE                   The statuses of the 2 SMM source descriptors are not identical.
**/
BOOLEAN
CompareStatuses (
  IN CONST SC_SMM_SOURCE_DESC *Src1,
  IN CONST SC_SMM_SOURCE_DESC *Src2
  );

/**
  Compare 2 SMM source descriptors, based on Enable settings and Status settings of them.

  @param[in] Src1                 Pointer to the SC SMI source description table 1
  @param[in] Src2                 Pointer to the SC SMI source description table 2

  @retval TRUE                    The 2 SMM source descriptors are identical.
  @retval FALSE                   The 2 SMM source descriptors are not identical.
**/
BOOLEAN
CompareSources (
  IN CONST SC_SMM_SOURCE_DESC *Src1,
  IN CONST SC_SMM_SOURCE_DESC *Src2
  );

/**
  Check if an SMM source is active.

  @param[in] Src                  Pointer to the SC SMI source description table

  @retval TRUE                    It is active.
  @retval FALSE                   It is inactive.
**/
BOOLEAN
SourceIsActive (
  IN CONST  SC_SMM_SOURCE_DESC *Src
  )
;

/**
  Enable the SMI source event by set the SMI enable bit, this function would also clear SMI
  status bit to make initial state is correct

  @param[in] SrcDesc              Pointer to the SC SMI source description table
**/
VOID
ScSmmEnableSource (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  );

/**
  Disable the SMI source event by clear the SMI enable bit

  @param[in] SrcDesc              Pointer to the SC SMI source description table
**/
VOID
ScSmmDisableSource (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  );
/**
  Clear the SMI status bit by set the source bit of SMI status register

  @param[in] SrcDesc              Pointer to the SC SMI source description table
**/
VOID
ScSmmClearSource (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  );

/**
  Sets the source to a 1 and then waits for it to clear.
  Be very careful when calling this function -- it will not
  ASSERT.  An acceptable case to call the function is when
  waiting for the NEWCENTURY_STS bit to clear (which takes
  3 RTCCLKs).

  @param[in] SrcDesc              Pointer to the SC SMI source description table
**/
VOID
ScSmmClearSourceAndBlock (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  );

#endif
