/** @file
  ACPI RTD3 SSDT table

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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification Reference:
**/

External(\_SB.OSCO)
External(\_SB.PCI0, DeviceObj)

External(\_SB.PCI0.RP01, DeviceObj)
External(\_SB.PCI0.RP02, DeviceObj)
External(\_SB.PCI0.RP03, DeviceObj)
External(\_SB.PCI0.RP04, DeviceObj)
External(\_SB.PCI0.RP05, DeviceObj)
External(\_SB.PCI0.RP06, DeviceObj)

External(\_SB.PCI0.RP01.VDID)
External(\_SB.PCI0.RP02.VDID)
External(\_SB.PCI0.RP03.VDID)
External(\_SB.PCI0.RP04.VDID)
External(\_SB.PCI0.RP05.VDID)
External(\_SB.PCI0.RP06.VDID)
External(\_SB.PCI0.SATA, DeviceObj)
External(\_SB.PCI0.SATA.PRT0, DeviceObj)
External(\_SB.PCI0.SATA.PRT1, DeviceObj)

External(\_SB.PCI0.XHC, DeviceObj)
External(\_SB.PCI0.XHC.RHUB, DeviceObj)

External(\_SB.PCI0.XHC.RHUB.HS01, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS02, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS03, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS04, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS05, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS06, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS07, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.HS08, DeviceObj)

External(\_SB.PCI0.XHC.RHUB.SSP1, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.SSP2, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.SSP3, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.SSP4, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.SSP5, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.SSP6, DeviceObj)
External(\_SB.PCI0.XHC.RHUB.SSP7, DeviceObj)

External(\_SB.PCI0.XDCI, DeviceObj)

External(\_SB.PCI0.SDHA, DeviceObj)
External(\_SB.PCI0.SDIO, DeviceObj)

External(\_SB.PCI0.PWM,  DeviceObj)
External(\_SB.PCI0.I2C0, DeviceObj)
External(\_SB.PCI0.I2C1, DeviceObj)
External(\_SB.PCI0.I2C2, DeviceObj)
External(\_SB.PCI0.I2C3, DeviceObj)
External(\_SB.PCI0.I2C4, DeviceObj)
External(\_SB.PCI0.I2C5, DeviceObj)
External(\_SB.PCI0.I2C6, DeviceObj)
External(\_SB.PCI0.I2C7, DeviceObj)
External(\_SB.PCI0.SPI1, DeviceObj)
External(\_SB.PCI0.SPI2, DeviceObj)
External(\_SB.PCI0.SPI3, DeviceObj)
External(\_SB.PCI0.URT1, DeviceObj)
External(\_SB.PCI0.URT2, DeviceObj)
External(\_SB.PCI0.URT3, DeviceObj)
External(\_SB.PCI0.URT4, DeviceObj)

//External(\UAMS)

External(\GPRW, MethodObj)
External(P8XH, MethodObj)
External(XDST, IntObj)
//[-start-161125-IB07400818-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
External(ZPGE, IntObj) // GPIO_25 GPE39
#endif
//[-end-161125-IB07400818-modify]//


//
// Externs common to ULT0RTD3.asl and FFRDRTD3.asl and exclude for BRRTD3.asl
//

// GPIO methods
External(\_SB.GPC0, MethodObj)
External(\_SB.SPC0, MethodObj)

//[-start-160828-IB07400775-modify]//
//#if BXTI_PF_ENABLE
External(\G0EN, IntObj)
External(\G0S, IntObj)
External(\OSSL, IntObj)
External(IOTP, MethodObj)
//#endif
//[-end-160828-IB07400775-modify]//

// IO expander methods

// RTD3 devices and variables
