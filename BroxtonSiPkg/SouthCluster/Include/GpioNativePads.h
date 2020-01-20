/** @file
  GPIO Native function group and pad numbers

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
#ifndef _SC_GPIO_NATIVE_PADS_H_
#define _SC_GPIO_NATIVE_PADS_H_

//
// SATA
//
//
// GPIO SATA0GP is the Sata port 0 reset pin.
//
#define SC_GPIO_SATA_PORT0_RESET_GROUP    4 //ScGpio_GPP_E
#define SC_GPIO_SATA_PORT0_RESET_PAD      0
//
// GPIO SATA1GP is the Sata port 1 reset pin.
//
#define SC_GPIO_SATA_PORT1_RESET_GROUP    4 //ScGpio_GPP_E
#define SC_GPIO_SATA_PORT1_RESET_PAD      1

//
//
// GPIO SATA DEVSLEEP 0-2
//
#define SC_GPIO_SATA_DEVSLPx_GROUP          4 //ScGpio_GPP_E
#define SC_GPIO_SATA_DEVSLP0_PAD            4
#define SC_GPIO_SATA_DEVSLP1_PAD            5

/*
//
// Serial GPIO feature
//
#define PCH_H_GPIO_SERIAL_BLINK_GROUP        ScGpio_GPP_D
#define PCH_GPIO_SERIAL_BLINK_MAX_PIN        4 // GPP_D_0-4 support Serial Blink feature

//
// PCIe
//
#define PCH_LP_GPIO_PCIE_SRCCLKREQB_GROUP    PchLpGpio_GPP_B
#define PCH_GPIO_PCIE_SRCCLKREQB_0_PAD       5
#define PCH_GPIO_PCIE_SRCCLKREQB_1_PAD       6
#define PCH_GPIO_PCIE_SRCCLKREQB_2_PAD       7
#define PCH_GPIO_PCIE_SRCCLKREQB_3_PAD       8
#define PCH_GPIO_PCIE_SRCCLKREQB_4_PAD       9
#define PCH_GPIO_PCIE_SRCCLKREQB_5_PAD       10

//
// Thermal
//
#define PCH_H_GPIO_PCHHOT_GROUP              PchHGpio_GPP_B
#define PCH_LP_GPIO_PCHHOT_GROUP             PchLpGpio_GPP_B
#define PCH_GPIO_PCHHOT_PAD                  23
#define PCH_GPIO_PAD_MODE_PCHHOT             V_PCH_GPIO_PAD_MODE_NAT_2

//
// gSPI chip select
//
#define PCH_H_GPIO_GSPIX_CSB_GROUP           PchHGpio_GPP_B
#define PCH_LP_GPIO_GSPIX_CSB_GROUP          PchLpGpio_GPP_B
#define PCH_GPIO_GSPI0_CSB_PAD               15
#define PCH_GPIO_GSPI1_CSB_PAD               19
*/
#endif
