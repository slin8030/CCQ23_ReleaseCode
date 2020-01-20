/** @file

  @brief
  This code supports a the private implementation
  of the Legacy Interrupt protocol

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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
#include "LegacyInterrupt.h"

///
/// Handle for the Legacy Interrupt Protocol instance produced by this driver
///
EFI_HANDLE                    mLegacyInterruptHandle = NULL;

///
/// The Legacy Interrupt Protocol instance produced by this driver
///
EFI_LEGACY_INTERRUPT_PROTOCOL mLegacyInterrupt = {
  GetNumberPirqs,
  GetLocation,
  ReadPirq,
  WritePirq
};

///
/// Module Global:
///  Since this driver will only ever produce one instance of the Private Data
///  protocol you are not required to dynamically allocate the PrivateData.
///
//[-start-160704-IB07400752-remove]//
//UINT8                         PirqReg[MAX_PIRQ_NUMBER] = { PIRQA, PIRQB, PIRQC, PIRQD, PIRQE, PIRQF, PIRQG, PIRQH };
//[-end-160704-IB07400752-remove]//

///
///
///
/**

  @brief
  Return the number of PIRQs supported by this chipset.

  @param[in]  This                 Pointer to LegacyInterrupt Protocol
  @param[out] NumberPirqs          The pointer which point to the max IRQ number supported by this SC.

  @retval EFI_SUCCESS             Legacy BIOS protocol installed

**/
EFI_STATUS
EFIAPI
GetNumberPirqs (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *NumberPirqs
  )
{
  *NumberPirqs = MAX_PIRQ_NUMBER;

  return EFI_SUCCESS;
}

/**

  @brief
  Return PCI location of this device. $PIR table requires this info.

  @param[in]  This                 Protocol instance pointer.
  @param[out] Bus                  PCI Bus
  @param[out] Device               PCI Device
  @param[out] Function             PCI Function

  @retval EFI_SUCCESS             Bus/Device/Function returned

**/
EFI_STATUS
EFIAPI
GetLocation (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *Bus,
  OUT UINT8                          *Device,
  OUT UINT8                          *Function
  )
{
//[-start-160704-IB07400752-add]//
  *Bus      = DEFAULT_PCI_BUS_NUMBER_SC;
  *Device   = PCI_DEVICE_NUMBER_PCH_LPC;
  *Function = PCI_FUNCTION_NUMBER_PCH_LPC;
//[-end-160704-IB07400752-add]//

  return EFI_SUCCESS;
}

//[-start-160704-IB07400752-remove]//
///**
//
//  @brief
//  Builds the PCIE configuration address for the register specified by PirqNumber
//
//  @param[in] PirqNumber           The PIRQ number to build the PCIE configuration address for
//
//  @retval UINTN                   The PCIE Configuration address for interrupt controller in SC
//
//**/
//UINTN
//GetAddress (
//  IN  UINT8  PirqNumber
//  )
//{
//
//  return 0xFFFFFFFF;
//}
//[-end-160704-IB07400752-remove]//

/**

  @brief
  Read the given PIRQ register

  @param[in]  This                 Pointer to LegacyInterrupt Protocol
  @param[in]  PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[out] PirqData             Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number

**/
EFI_STATUS
EFIAPI
ReadPirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  OUT UINT8                          *PirqData
  )
{
  if (PirqNumber >= MAX_PIRQ_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-160704-IB07400752-modify]//
#ifdef LEGACY_IRQ_SUPPORT
  if (PirqNumber < 4) { // PIRQA~D
    *PirqData = (UINT8)(SideBandRead32 (SB_ITSS_PORT, R_ITSS_SB_PARC) >> (PirqNumber * 8));
  } else { // PIRQE~H
    *PirqData = (UINT8)(SideBandRead32 (SB_ITSS_PORT, R_ITSS_SB_PERC) >> ((PirqNumber - 4) * 8));
  }
#else
  *PirqData = 0xFF;
#endif
//[-end-160704-IB07400752-modify]//

  *PirqData = (UINT8) (*PirqData & 0x7f);

  return EFI_SUCCESS;
}

/**

  @brief
  Read the given PIRQ register

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[in] PirqData             Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number

**/
EFI_STATUS
EFIAPI
WritePirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  IN  UINT8                          PirqData
  )
{
//[-start-160704-IB07400752-add]//
#ifdef LEGACY_IRQ_SUPPORT
  UINT32        Data32;
#endif
//[-end-160704-IB07400752-add]//

  if (PirqNumber >= MAX_PIRQ_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }

//[-start-160704-IB07400752-modify]//
#ifdef LEGACY_IRQ_SUPPORT
  if (PirqNumber < 4) { // PIRQA~D
    Data32 = (SideBandRead32 (SB_ITSS_PORT, R_ITSS_SB_PARC)) & (~((UINT32)(0xFF) << (PirqNumber * 8)));
    Data32 |= ((UINT32)PirqData << (PirqNumber * 8));
    SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PARC, Data32);
  } else { // PIRQE~H
    Data32 = (SideBandRead32 (SB_ITSS_PORT, R_ITSS_SB_PERC)) & (~((UINT32)(0xFF) << ((PirqNumber - 4) * 8)));
    Data32 |= ((UINT32)PirqData << ((PirqNumber - 4) * 8));
    SideBandWrite32 (SB_ITSS_PORT, R_ITSS_SB_PERC, Data32);
  }
#endif  
//[-end-160704-IB07400752-modify]//
  return EFI_SUCCESS;
}

/**

  @brief
  Install Driver to produce Legacy Interrupt protocol.

  @param[in] ImageHandle          Handle for this drivers loaded image protocol.
  @param[in] SystemTable          EFI system table.

  @retval EFI_SUCCESS             Legacy Interrupt protocol installed
  @retval Other                   No protocol installed, unload driver.

**/
EFI_STATUS
EFIAPI
LegacyInterruptInstall (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  DEBUG ((EFI_D_INFO, "LegacyInterruptInstall() Start\n"));

  ///
  /// Make sure the Legacy Interrupt Protocol is not already installed in the system
  ///
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiLegacyInterruptProtocolGuid);

  ///
  /// Make a new handle and install the protocol
  ///
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mLegacyInterruptHandle,
                  &gEfiLegacyInterruptProtocolGuid,
                  &mLegacyInterrupt,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "LegacyInterruptInstall() End\n"));
  return Status;
}
