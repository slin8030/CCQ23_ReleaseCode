/** @file
  IoApic policy

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

#ifndef _INTERRUPT_CONFIG_H_
#define _INTERRUPT_CONFIG_H_

#define INTERRUPT_CONFIG_REVISION 2
extern EFI_GUID gInterruptConfigGuid;

#pragma pack (push,1)

//
// --------------------- Interrupts Config ------------------------------
//
typedef struct {
  UINT8   Port;
  UINT16  PciCfgOffset;
  UINT8   PciIrqNumber;
  UINT8   IrqPin;
} PRIVATE_PCICFGCTRL;

typedef enum {
  ScNoInt,        ///< No Interrupt Pin
  ScIntA,
  ScIntB,
  ScIntC,
  ScIntD
} SC_INT_PIN;

///
/// The SC_DEVICE_INTERRUPT_CONFIG block describes interrupt pin, IRQ and interrupt mode for SC device.
///
typedef struct {
  UINT8        Device;                  ///< Device number
  UINT8        Function;                ///< Device function
  UINT8        IntX;                    ///< Interrupt pin: INTA-INTD (see SC_INT_PIN)
  UINT8        Irq;                     ///< IRQ to be set for device.
} SC_DEVICE_INTERRUPT_CONFIG;

#define SC_MAX_DEVICE_INTERRUPT_CONFIG   64      ///< Number of all SC devices
#define SC_MAX_PXRC_CONFIG                8      ///< Number of PXRC registers in ITSS
#define SC_MAX_DIRECT_IRQ_CONFIG         25      ///< Number of direct Irq Table
#define SC_NUM_IPC_REG                    4      ///< Number of ITSS IPC registers
///
/// The SC_INTERRUPT_CONFIG block describes interrupt settings for SC.
///
typedef struct {
  /**
    Revision 1: Init version
    Revision 2: Add IRQ interrupt polarity control for platform override
  **/

  CONFIG_BLOCK_HEADER          Header;                                          ///< Config Block Header
  UINT8                        NumOfDevIntConfig;                               ///< Number of entries in DevIntConfig table
  UINT8                        NumOfDirectIrqTable;
  PRIVATE_PCICFGCTRL           DirectIrqTable[SC_MAX_DIRECT_IRQ_CONFIG];
  SC_DEVICE_INTERRUPT_CONFIG   DevIntConfig[SC_MAX_DEVICE_INTERRUPT_CONFIG];    ///< Array which stores SC devices interrupts settings
  UINT8                        PxRcRouting[SC_MAX_PXRC_CONFIG];
  UINT32                       IPC[SC_NUM_IPC_REG];                             ///< Interrupt Polarity Control
} SC_INTERRUPT_CONFIG;

#pragma pack (pop)

#endif // _INTERRUPT_CONFIG_H_
