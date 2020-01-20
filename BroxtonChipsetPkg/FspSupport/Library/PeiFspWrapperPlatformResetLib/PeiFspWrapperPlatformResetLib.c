/**@file
 Pei library function to reset the system from FSP wrapper.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

@par Specification
**/

#include <PiPei.h>
#include <FspEas.h>
#include <Library/HeciMsgLib.h>
#include <ScRegs/RegsPcu.h>
#include <ScRegs/RegsPmc.h>
#include <Library/IoLib.h>

/**
  Perform platform related reset in FSP wrapper.

  @param[in] ResetType  The type of reset the platform has to perform.

  @return Will reset the system based on Reset status provided.
**/
VOID
EFIAPI
CallFspWrapperResetSystem (
  IN UINT32    ResetType
  )
{
  UINT8   Reset;
  UINT16  AcpiBaseAddr;
  UINT16  Data16;
  UINT32  Data32;

  switch(ResetType) {
    case FSP_STATUS_RESET_REQUIRED_COLD:
    	Reset = IoRead8 (R_RST_CNT);
      Reset &= 0xF1;
      Reset |= 0xE;
      IoWrite8(R_RST_CNT, Reset);
      break;
    case FSP_STATUS_RESET_REQUIRED_WARM:
    	Reset = IoRead8 (R_RST_CNT);
      Reset &= 0xF1;
      Reset |= 0x6;
      IoWrite8(R_RST_CNT, Reset);
      break;
    case FSP_STATUS_RESET_REQUIRED_3:
      ///
      /// Read ACPI Base Address
      ///
      AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

      //
      // Then, GPE0_EN should be disabled to avoid any GPI waking up the system from S5
      //
      Data16 = 0;
      IoWrite16 (
        (UINTN)(AcpiBaseAddr + R_ACPI_GPE0a_EN),
        (UINT16)Data16
        );

      //
      // Secondly, PwrSts register must be cleared
      //
      // Write a "1" to bit[8] of power button status register at
      // (ABASE + R_ACPI_PM1_STS) to clear this bit
      //
      Data16 = B_ACPI_PM1_STS_PWRBTN;
      IoWrite16 (
        (UINTN)(AcpiBaseAddr + R_ACPI_PM1_STS),
        (UINT16)Data16
        );

      //
      // Finally, transform system into S5 sleep state
      //
      Data32 = IoRead32 ((UINTN)(AcpiBaseAddr + R_ACPI_PM1_CNT));

      Data32  = (UINT32) ((Data32 & ~(B_ACPI_PM1_CNT_SLP_TYP + B_ACPI_PM1_CNT_SLP_EN)) | V_ACPI_PM1_CNT_S5);

      IoWrite32 (
        (UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT),
        (UINT32)Data32
        );

      Data32 = Data32 | B_ACPI_PM1_CNT_SLP_EN;

      IoWrite32 (
        (UINTN) (AcpiBaseAddr + R_ACPI_PM1_CNT),
        (UINT32)Data32
        );
      break;
    case FSP_STATUS_RESET_REQUIRED_5:
      HeciSendResetRequest (CBM_RR_REQ_ORIGIN_BIOS_POST, CBM_HRR_GLOBAL_RESET);
      break;
    default:
      return;
  }
  CpuDeadLoop();
}
