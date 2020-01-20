/**@file

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/


  // NOTE:  The _PDC Implementation is out of the scope of this
  // reference code.  Please see the latest Hyper-Threading Technology
  // Reference Code for complete implementation details.

  Scope(\_PR)
  {
    Processor(CPU0,  // Unique name for Processor 0.
      1,             // Unique ID for Processor 0.
      0x00,          // CPU0 ACPI P_BLK address = ACPIBASE + 10h.
      0)             // CPU0 ICH7M P_BLK length = 6 bytes.
      {}

    Processor(CPU1,  // Unique name for Processor 1.
      2,             // Unique ID for Processor 1.
      0x00,
      0)             // CPU1 P_BLK length = 6 bytes.
      {}

    Processor(CPU2,  // Unique name for Processor 2.
      3,             // Unique ID for Processor 2.
      0x00,
      0)             // CPU2 P_BLK length = 6 bytes.
      {}

    Processor(CPU3,  // Unique name for Processor 3.
      4,             // Unique ID for Processor 3.
      0x00,
      0)             // CPU3 P_BLK length = 6 bytes.
      {}
  }	// End _PR
