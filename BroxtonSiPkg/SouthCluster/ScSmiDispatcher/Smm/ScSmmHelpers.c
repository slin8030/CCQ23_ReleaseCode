/** @file
  Helper functions for SC SMM dispatcher.

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

@par Specification Reference:
**/
#include "ScSmmHelpers.h"

//
// #define BIT_ZERO 0x00000001
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT32  BIT_ZERO = 0x00000001;

//
// SUPPORT / HELPER FUNCTIONS (SC version-independent)
//

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
  )
{
  BOOLEAN IsEqual;
  UINTN   DescIndex;

  IsEqual = TRUE;
  for (DescIndex = 0; DescIndex < NUM_EN_BITS; DescIndex++) {
    //
    // It's okay to compare a NULL bit description to a non-NULL bit description.
    // They are unequal and these tests will generate the correct result.
    //
    if (Src1->En[DescIndex].Bit != Src2->En[DescIndex].Bit ||
        Src1->En[DescIndex].Reg.Type != Src2->En[DescIndex].Reg.Type ||
        Src1->En[DescIndex].Reg.Data.raw != Src2->En[DescIndex].Reg.Data.raw
        ) {
      IsEqual = FALSE;
      break;
      //
      // out of for loop
      //
    }
  }

  return IsEqual;
}

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
  )
{
  BOOLEAN IsEqual;
  UINTN   DescIndex;

  IsEqual = TRUE;

  for (DescIndex = 0; DescIndex < NUM_STS_BITS; DescIndex++) {
    //
    // It's okay to compare a NULL bit description to a non-NULL bit description.
    // They are unequal and these tests will generate the correct result.
    //
    if (Src1->Sts[DescIndex].Bit != Src2->Sts[DescIndex].Bit ||
        Src1->Sts[DescIndex].Reg.Type != Src2->Sts[DescIndex].Reg.Type ||
        Src1->Sts[DescIndex].Reg.Data.raw != Src2->Sts[DescIndex].Reg.Data.raw
        ) {
      IsEqual = FALSE;
      break;
      //
      // out of for loop
      //
    }
  }

  return IsEqual;
}

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
  )
{
  return (BOOLEAN) (CompareEnables (Src1, Src2) && CompareStatuses (Src1, Src2));
}

/**
  Check if an SMM source is active.

  @param[in] Src                  Pointer to the SC SMI source description table

  @retval TRUE                    It is active.
  @retval FALSE                   It is inactive.
**/
BOOLEAN
SourceIsActive (
  const IN SC_SMM_SOURCE_DESC *Src
  )
{
  BOOLEAN IsActive;
  UINTN   loopvar;

  BOOLEAN SciEn;

  IsActive  = TRUE;

  SciEn     = ScSmmGetSciEn ();

  if ((Src->Flags & SC_SMM_SCI_EN_DEPENDENT) && (SciEn)) {
    //
    // This source is dependent on SciEn, and SciEn == 1.  An ACPI OS is present,
    // so we shouldn't do anything w/ this source until SciEn == 0.
    //
    IsActive = FALSE;

  } else {
    //
    // Read each bit desc from hardware and make sure it's a one
    //
    for (loopvar = 0; loopvar < NUM_EN_BITS; loopvar++) {

      if (!IS_BIT_DESC_NULL (Src->En[loopvar])) {

        if (ReadBitDesc (&Src->En[loopvar]) == 0) {
          IsActive = FALSE;
          break;
          //
          // out of for loop
          //
        }

      }
    }

    if (IsActive) {
      //
      // Read each bit desc from hardware and make sure it's a one
      //
      for (loopvar = 0; loopvar < NUM_STS_BITS; loopvar++) {

        if (!IS_BIT_DESC_NULL (Src->Sts[loopvar])) {

          if (ReadBitDesc (&Src->Sts[loopvar]) == 0) {
            IsActive = FALSE;
            break;
            //
            // out of for loop
            //
          }

        }
      }
    }
  }

  return IsActive;
}

/**
  Enable the SMI source event by set the SMI enable bit, this function would also clear SMI
  status bit to make initial state is correct

  @param[in] SrcDesc              Pointer to the SC SMI source description table

**/
VOID
ScSmmEnableSource (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  )
{
  UINTN DescIndex;

  //
  // Set enables to 1 by writing a 1
  //
  for (DescIndex = 0; DescIndex < NUM_EN_BITS; DescIndex++) {
    if (!IS_BIT_DESC_NULL (SrcDesc->En[DescIndex])) {
      WriteBitDesc (&SrcDesc->En[DescIndex], 1, FALSE);
    }
  }
  //
  // Clear statuses to 0 by writing a 1
  //
  for (DescIndex = 0; DescIndex < NUM_STS_BITS; DescIndex++) {
    if (!IS_BIT_DESC_NULL (SrcDesc->Sts[DescIndex])) {
      WriteBitDesc (&SrcDesc->Sts[DescIndex], 1, TRUE);
    }
  }
}

/**
  Disable the SMI source event by clear the SMI enable bit

  @param[in] SrcDesc              Pointer to the SC SMI source description table

**/
VOID
ScSmmDisableSource (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  )
{
  UINTN DescIndex;

  for (DescIndex = 0; DescIndex < NUM_EN_BITS; DescIndex++) {
    if (!IS_BIT_DESC_NULL (SrcDesc->En[DescIndex])) {
      WriteBitDesc (&SrcDesc->En[DescIndex], 0, FALSE);
    }
  }
}

/**
  Clear the SMI status bit by set the source bit of SMI status register

  @param[in] SrcDesc              Pointer to the SC SMI source description table

**/
VOID
ScSmmClearSource (
  IN CONST SC_SMM_SOURCE_DESC *SrcDesc
  )
{
  UINTN DescIndex;

  for (DescIndex = 0; DescIndex < NUM_STS_BITS; DescIndex++) {
    if (!IS_BIT_DESC_NULL (SrcDesc->Sts[DescIndex])) {
      WriteBitDesc (&SrcDesc->Sts[DescIndex], 1, TRUE);
    }
  }
}

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
  )
{
  UINTN   DescIndex;
  BOOLEAN IsSet;

  for (DescIndex = 0; DescIndex < NUM_STS_BITS; DescIndex++) {

    if (!IS_BIT_DESC_NULL (SrcDesc->Sts[DescIndex])) {
      //
      // Write the bit
      //
      WriteBitDesc (&SrcDesc->Sts[DescIndex], 1, TRUE);

      //
      // Don't return until the bit actually clears.
      //
      IsSet = TRUE;
      while (IsSet) {
        IsSet = ReadBitDesc (&SrcDesc->Sts[DescIndex]);
        //
        // IsSet will eventually clear -- or else we'll have
        // an infinite loop.
        //
      }
    }
  }
}

