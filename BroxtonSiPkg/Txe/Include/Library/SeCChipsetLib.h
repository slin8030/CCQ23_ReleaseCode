/** @file
  Header file for SEC Chipset Lib
 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

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

#ifndef _SEC_CHIPSET_LIB_H_
#define _SEC_CHIPSET_LIB_H_

///
/// CSME PCI devices should be numbered according to their PCI function number
/// HECI1 = 0, HECI2 = 1, HECI3 = 2, FTPM = 7
///
typedef enum {
  HECI1 = 0,
  HECI2,
  HECI3,
  FTPM = 7
} SEC_DEVICE;

typedef enum {
  Disabled = 0,
  Enabled
} SEC_DEVICE_FUNC_CTRL;

/**
  Put SEC device into D0I3.

  @param[in] Function          Select of SEC device

  @retval VOID                 None
**/
VOID
SetD0I3Bit(
IN  UINT32   Function
)
;

/**
  Enable/Disable SEC devices

  @param[in] WhichDevice       Select of SEC device
  @param[in] DeviceFuncCtrl    Function control

  @retval VOID                 None
**/
VOID
SeCDeviceControl(
IN  SEC_DEVICE                   WhichDevice,
IN  SEC_DEVICE_FUNC_CTRL         DeviceFuncCtrl
)
;

#endif
