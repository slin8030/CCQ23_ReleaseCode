/** @file
  Macros to simplify and abstract the interface to PCI configuration.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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

@par Specification
**/

#ifndef _SEC_ACCESS_H_
#define _SEC_ACCESS_H_

#include "SeCChipset.h"
#include <ScAccess.h>
#include <SaAccess.h>
#include <Library/PciLib.h>


//
// HECI PCI Access Macro
//
#define HeciPciRead32(Register) PciRead32 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register))

#define HeciPciWrite32(Register, Data) \
  PciWrite32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define HeciPciOr32(Register, Data) \
  PciOr32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define HeciPciAnd32(Register, Data) \
  PciAnd32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define HeciPciAndThenOr32(Register, AndData, OrData) \
  PciAndThenOr32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT32) AndData, \
  (UINT32) OrData \
  )

#define HeciPciRead16(Register) PciRead16 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register))

#define HeciPciWrite16(Register, Data) \
  PciWrite16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT16) Data \
  )

#define HeciPciOr16(Register, Data) \
  PciOr16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT16) Data \
  )

#define HeciPciAnd16(Register, Data) \
  PciAnd16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT16) Data \
  )

#define HeciPciAndThenOr16(Register, AndData, OrData) \
  PciAndThenOr16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT16) AndData, \
  (UINT16) OrData \
  )

#define HeciPciRead8(Register)  PciRead8 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register))

#define HeciPciWrite8(Register, Data) \
  PciWrite8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT8) Data \
  )

#define HeciPciOr8(Register, Data) \
  PciOr8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT8) Data \
  )

#define HeciPciAnd8(Register, Data) \
  PciAnd8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT8) Data \
  )

#define HeciPciAndThenOr8(Register, AndData, OrData) \
  PciAndThenOr8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI_FUNCTION_NUMBER, \
  Register), \
  (UINT8) AndData, \
  (UINT8) OrData \
  )

//
// HECI2 PCI Access Macro
//
#define Heci2PciRead32(Register)  PciRead32 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI2_FUNCTION_NUMBER, Register))

#define Heci2PciWrite32(Register, Data) \
  PciWrite32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define Heci2PciOr32(Register, Data) \
  PciOr32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define Heci2PciAnd32(Register, Data) \
  PciAnd32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define Heci2PciAndThenOr32(Register, AndData, OrData) \
  PciAndThenOr32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT32) AndData, \
  (UINT32) OrData \
  )

#define Heci2PciRead16(Register)  PciRead16 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI2_FUNCTION_NUMBER, Register))

#define Heci2PciWrite16(Register, Data) \
  PciWrite16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT16) Data \
  )

#define Heci2PciOr16(Register, Data) \
  PciOr16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT16) Data \
  )

#define Heci2PciAnd16(Register, Data) \
  PciAnd16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT16) Data \
  )

#define Heci2PciAndThenOr16(Register, AndData, OrData) \
  PciAndThenOr16 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT16) AndData, \
  (UINT16) OrData \
  )

#define Heci2PciRead8(Register) PciRead8 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI2_FUNCTION_NUMBER, Register))

#define Heci2PciWrite8(Register, Data) \
  PciWrite8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT8) Data \
  )

#define Heci2PciOr8(Register, Data) \
  PciOr8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT8) Data \
  )

#define Heci2PciAnd8(Register, Data) \
  PciAnd8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT8) Data \
  )

#define Heci2PciAndThenOr8(Register, AndData, OrData) \
  PciAndThenOr8 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI2_FUNCTION_NUMBER, \
  Register), \
  (UINT8) AndData, \
  (UINT8) OrData \
  )

//
// HECI3 PCI Access Macro
//
#define Heci3PciRead32(Register) PciRead32 (PCI_LIB_ADDRESS (SEC_BUS, SEC_DEVICE_NUMBER, HECI3_FUNCTION_NUMBER, Register))

#define Heci3PciWrite32(Register, Data) \
  PciWrite32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI3_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define Heci3PciOr32(Register, Data) \
  PciOr32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI3_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define Heci3PciAnd32(Register, Data) \
  PciAnd32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI3_FUNCTION_NUMBER, \
  Register), \
  (UINT32) Data \
  )

#define Heci3PciAndThenOr32(Register, AndData, OrData) \
  PciAndThenOr32 ( \
  PCI_LIB_ADDRESS (SEC_BUS, \
  SEC_DEVICE_NUMBER, \
  HECI3_FUNCTION_NUMBER, \
  Register), \
  (UINT32) AndData, \
  (UINT32) OrData \
  )

#endif
