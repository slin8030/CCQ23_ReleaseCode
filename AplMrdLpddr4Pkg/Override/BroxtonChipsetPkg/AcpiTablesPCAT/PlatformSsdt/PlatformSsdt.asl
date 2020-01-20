/** @file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 Intel Corporation.

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

DefinitionBlock (
    "PlatformSsdt.aml",
    "SSDT",
    1,
    "Intel_",
    "PlatformTable",
    0x1000
    )
{
  External(\_SB.PCI0.I2C0, DeviceObj)
  External(\_SB.PCI0.I2C1, DeviceObj)
  External(\_SB.PCI0.I2C2, DeviceObj)
  External(\_SB.PCI0.I2C3, DeviceObj)
  External(\_SB.PCI0.I2C4, DeviceObj)
//[-start-170323-IB15590024-add]// 
  External(\_SB.PCI0.I2C5, DeviceObj)
//[-end-170323-IB15590024-add]// 
  External(\_SB.PCI0.I2C7, DeviceObj)
  External(\_SB.PCI0.URT1, DeviceObj)
  External(\_SB.PCI0.URT2, DeviceObj)
  External(\_SB.PCI0.SDIO, DeviceObj)
  External(\_SB.PCI0.SPI1, DeviceObj)
  External(\_SB.GPO0.CWLE, IntObj)
  //External(\_SB.GPO1, DeviceObj)
  External(\_SB.GPO0.AVBL, IntObj)

  External(\_SB.PCI0.SDIO.PSTS, IntObj)
  External(HIDG, MethodObj)
  External(OSYS, IntObj)
  External(SBTD, IntObj)
  External(WCAS, IntObj)
  External(UCAS, IntObj)
  External(CROT, IntObj)
  External(TP7G)
  External(IPUD)  
//[-start-160503-IB07400722-add]//
  External(\HLPS, IntObj)
//[-end-160503-IB07400722-add]//
//[-start-170518-IB07400867-add]//
  External(IOTP, MethodObj)
//[-end-170518-IB07400867-add]//

  include ("Audio/AudioCodec_INT34C1.asl")
  include ("Audio/AudioCodec_INT343A.asl")
  
  If(LEqual(IPUD, 1)) {
    include ("Camera/Camera_INT3471.asl")
//[-start-170516-IB08450375-modify]//
//[-start-170518-IB07400867-modify]//
// #if !BXTI_PF_ENABLE
    include ("Camera/Camera_INT3474.asl")
// #endif
//[-end-170518-IB07400867-modify]//
//[-end-170516-IB08450375-modify]//
    include ("Camera/Camera_Sony214A.asl")
    include ("Camera/Flash_TPS61311.asl")
//[-start-161201-IB07400821-modify]//
//    #if BXTI_PF_ENABLE
      include  ("Camera/Camera_MT9V024.asl")
//    #endif
//[-end-161201-IB07400821-modify]//
  }

  include ("Touch/TouchPanel_I2C3.asl")
  include ("Touch/TouchPads_I2C4.asl")

  include ("PSS/MonzaX2K_IMPJ0003.asl")

  include ("Bluetooth/Bluetooth_BCM2E40.asl")
//  include ("Bluetooth/Bluetooth_BCM2EA1.asl")

  include ("Wifi/WIFI_Broadcom1.asl")
  include ("Wifi/WIFI_Broadcom2.asl")

  include ("Gps/Gps.asl")

  include ("Nfc/Nfc.asl")

  include ("Fingerprint/Fingerprint_FPC.asl")
//[-start-170323-IB15590024-add]// 
  include ("TouchScreen/TouchScreen_I2C3.asl")
  include ("Sensors/Sensors.asl")
//[-end-170323-IB15590024-add]// 
}