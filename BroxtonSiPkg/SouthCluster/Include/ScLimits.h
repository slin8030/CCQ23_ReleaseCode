/** @file
  Build time limits of SC resources.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
#ifndef _SC_LIMITS_H_
#define _SC_LIMITS_H_

//
// PCIe limits
//
#define SC_MAX_PCIE_ROOT_PORTS       SC_BXTP_PCIE_MAX_ROOT_PORTS
#define SC_BXT_PCIE_MAX_ROOT_PORTS   2
#define SC_BXTP_PCIE_MAX_ROOT_PORTS  6

#define SC_MAX_PCIE_CONTROLLERS      SC_BXTP_PCIE_MAX_CONTROLLERS
#define SC_PCIE_CONTROLLER_PORTS     4
#define SC_BXT_PCIE_MAX_CONTROLLERS  1
#define SC_BXTP_PCIE_MAX_CONTROLLERS 2

#define SC_PCIE_MAX_CLK_REQ          4

//
// PCIe clocks limits
//
#define SC_MAX_PCIE_CLOCKS                 6

//
// SATA limits
//
#define SC_MAX_SATA_PORTS                  2
#define SC_SATA_MAX_DEVICES_PER_PORT       1       ///< Max support device numner per port, Port Multiplier is not support.

//
// USB limits
//
#define HSIC_MAX_PORTS                  2
#define XHCI_MAX_USB3_PORTS             1
#define XHCI_MAX_HSIC_PORTS             1     ///< BXT has only 1 HSIC port
#define XHCI_MAX_SSIC_PORTS             2     ///< BXT has 2 SSIC port

#define SC_MAX_USB2_PORTS               SC_BXTP_MAX_USB2_PORTS
#define SC_BXT_MAX_USB2_PORTS           3
#define SC_BXTP_MAX_USB2_PORTS          8

#define SC_MAX_USB3_PORTS               SC_BXTP_MAX_USB3_PORTS
#define SC_BXT_MAX_USB3_PORTS           2
#define SC_BXTP_MAX_USB3_PORTS          6

//
// Flash Protection Range Register
//
#define SC_FLASH_PROTECTED_RANGES       5

#endif ///< _SC_LIMITS_H_

