/** @file
  Intel Processor Power Management IST ACPI Code.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

DefinitionBlock (
  "APIST.aml",
  "SSDT",
  2,
  "PmRef",
  "ApIst",
  0x3000
  )
{
  External(\_PR.CPU0._PSS, MethodObj)
  External(\_PR.CPU0._PCT, MethodObj)
  External(\_PR.CPU0._PPC, IntObj)
  External(\_PR.CPU0._PSD, MethodObj)
  External(\_PR.CPU1, DeviceObj)
  External(\_PR.CPU2, DeviceObj)
  External(\_PR.CPU3, DeviceObj)
  External(\_PR.CFGD, IntObj)
  External (PDC0)

  Scope(\_PR.CPU1)
  {
    Method(_PPC,0)
    {
      Return(\_PR.CPU0._PPC)  // Return P0 _PPC value.
    }

    Method(_PCT,0)
    {
      Return(\_PR.CPU0._PCT)  // Return P0 _PCT.
    }

    Method(_PSS,0)
    {
      //Return the same table as CPU0 for CMP cases.
      Return(\_PR.CPU0._PSS)
    }
    Method(_PSD,0)
    {
      Return(\_PR.CPU0._PSD)	// Return P0 _PSD.			
    }

  }

  Scope(\_PR.CPU2)
  {
    Method(_PPC,0)
    {
      Return(\_PR.CPU0._PPC)  // Return CPU0 _PPC value.
    }

    Method(_PCT,0)
    {
      Return(\_PR.CPU0._PCT)  // Return CPU0 _PCT value.
    }

    Method(_PSS,0)
    {
      Return(\_PR.CPU0._PSS)  // Return CPU0 _PSS.
    }

    Method(_PSD,0)
    {
      Return(\_PR.CPU0._PSD)	// Return P0 _PSD.			
    }

  }


  Scope(\_PR.CPU3)
  {
    Method(_PPC,0)
    {
      Return(\_PR.CPU0._PPC)  // Return CPU0 _PPC value.
    }

    Method(_PCT,0)
    {
      Return(\_PR.CPU0._PCT)  // Return CPU0 _PCT value.
    }

    Method(_PSS,0)
    {
      Return(\_PR.CPU0._PSS)  // Return CPU0 _PSS.
    }

    Method(_PSD,0)
    {
      Return(\_PR.CPU0._PSD)	// Return P0 _PSD.			
    }

  }


} // End of Definition Block
