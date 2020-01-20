/** @file
  Intel Processor Power Management ACPI Code.

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

#include "CpuPowerMgmt.h"

DefinitionBlock (
  "CPU0CST.aml",
  "SSDT",
  2,
  "PmRef",
  "Cpu0Cst",
  0x3001
  )
{
External(\_PR.CPU0, DeviceObj)
External(PWRS)
External(\_PR.CFGD, IntObj)
External(PDC0)
External(FMBL)
External(FEMD)
External(PFLV)
External(C3MW)       // Mwait Hint value for C3
External(C6MW)       // Mwait Hint value for C6
External(C7MW)       // Mwait Hint value for C7
External(CDMW)       // Mwait Hint value for C8/C9/C10
External(C3LT)       // Latency value for C3
External(C6LT)       // Latency Value for C6
External(C7LT)       // Latency Value for C7
External(CDLT)       // Latency Value for C8/C9/C10
External(CDLV)       // IO Level value for C8/C9/C10
External(CDPW)       // Power value for  C8/C9/C10
External(MWEN)

Scope(\_PR.CPU0)
{
        Method (_CST, 0)
        {
            If (LEqual(MWEN, 0))
            {
                    Return( Package()
                    {
                        3,
                        Package()
                        {   // C1, LVT
                            ResourceTemplate(){Register(FFixedHW, 0, 0, 0)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {
                            // C6L, LVT
                            ResourceTemplate(){Register(SystemIO, 8, 0, 0x415)},
                            2,
                            50,
                            10
                        },
                        Package()
                        {
                            // C10, LVT
                            ResourceTemplate(){Register(SystemIO, 8, 0, 0x419)},
                            3,
                            150,   //Pre-silicon setting, 11 ms for worst-case exit latency
                            10
                        }
                    })
            }
                    Return( Package()
                    {
                        3,
                        Package()
                        {   // C1, MWAIT
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x01, 1)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {
                            // C6L, MWAIT Extension
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x21, 1)},
                            2,
                            50,
                            10
                        },
                        Package()
                        {
                            // C10, MWAIT Extension
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x60, 1)},
                            3,
                            150,   //Pre-silicon setting, 11 ms for worst-case exit latency
                            10
                        }
                    })
        }
}
}
