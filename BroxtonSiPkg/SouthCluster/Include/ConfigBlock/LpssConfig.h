/** @file
  LPSS policy

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/
#ifndef _LPSS_CONFIG_H_
#define _LPSS_CONFIG_H_

#define LPSS_CONFIG_REVISION 2
extern EFI_GUID gLpssConfigGuid;

#pragma pack (push,1)

#define  LPSS_I2C_DEVICE_NUM     8
#define  LPSS_HSUART_DEVICE_NUM  4
#define  LPSS_SPI_DEVICE_NUM     3

/**
  The SC_LPSS_CONFIG block describes Low Power Sub System (LPSS) settings for SC.
  @note: the order defined below is per the PCI BDF sequence, and MUST not change.
  Items defined will be accessed by its index in ScInit module
**/
typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Deprecated ExiEnable
  **/
  CONFIG_BLOCK_HEADER Header;           ///< Config Block Header
  /**
    Determine if I2C_n is enabled.
    0: Disabled; <b>1: PCI Mode</b>; 2: ACPI Mode;
  **/
  SC_DEV_MODE         I2c0Enable;
  SC_DEV_MODE         I2c1Enable;
  SC_DEV_MODE         I2c2Enable;
  SC_DEV_MODE         I2c3Enable;
  SC_DEV_MODE         I2c4Enable;
  SC_DEV_MODE         I2c5Enable;
  SC_DEV_MODE         I2c6Enable;
  SC_DEV_MODE         I2c7Enable;
  /**
    Determine if UART_n is enabled.
    0: Disabled; <b>1: PCI Mode</b>; 2: ACPI Mode;
  **/
  SC_DEV_MODE         Hsuart0Enable;
  SC_DEV_MODE         Hsuart1Enable;
  SC_DEV_MODE         Hsuart2Enable;
  SC_DEV_MODE         Hsuart3Enable;
  /**
    Determine if SPI_n is enabled.
    0: Disabled; <b>1: PCI Mode</b>; 2: ACPI Mode;
  **/
  SC_DEV_MODE         Spi0Enable;
  SC_DEV_MODE         Spi1Enable;
  SC_DEV_MODE         Spi2Enable;
  UINT32              Uart2KernelDebugBaseAddress;
  UINT8               I2cClkGateCfg[LPSS_I2C_DEVICE_NUM];
  UINT8               HsuartClkGateCfg[LPSS_HSUART_DEVICE_NUM];
  UINT8               SpiClkGateCfg[LPSS_SPI_DEVICE_NUM];
  UINT32              S0ixEnable  : 1;  ///< <b>0: Disabled</b>; 1: Enabled
  UINT32              OsDbgEnable : 1;  ///< <b>0: Disabled</b>; 1: Enabled
  UINT32              ExiEnable   : 1;  ///< @deprecated, please use DciEn from SC_DCI_CONFIG
  UINT32              RsvdBits    : 29;
} SC_LPSS_CONFIG;

#pragma pack (pop)

#endif // _LPSS_CONFIG_H_
