/*++

Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PowerFailHandle.c

Abstract:



--*/

#include "PlatformInit.h"

EFI_STATUS
GeneralPowerFailureHandler (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  )
/*++

Routine Description:


Arguments:


Returns:


--*/
{

  UINT16  DataUint16;
  UINTN   GenPmCon1;
  UINT16  Pm1Sts;
  UINT16  AcpiBaseAddr;
  
  ///
  /// Read ACPI Base Address
  ///
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  
  //
  // VLV BIOS Specification 0.6.2 - Section 18.4, "Power Failure Consideration"
  //
  // When the SUS_PWR_FLR bit is set, it indicates the SUS well power is lost.
  // This bit is in the SUS Well and defaults to 1'b1 based on RSMRST# assertion (not cleared by any type of reset).
  // System BIOS should follow cold boot path if SUS_PWR_FLR (PBASE + 0x20[14]),
  // GEN_RST_STS (PBASE + 0x20[9]) or PWRBTNOR_STS (ABASE + 0x00[11]) is set to 1'b1
  // regardless of the value in the SLP_TYP (ABASE + 0x04[12:10]) field.
  //
  GenPmCon1 = MmioRead32 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  Pm1Sts    = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_STS);
  if ((GenPmCon1 & (B_PMC_GEN_PMCON_GEN_RST_STS | B_PMC_GEN_PMCON_GLOBAL_RST_STS)) ||
     (Pm1Sts & B_ACPI_PM1_STS_PRBTNOR)) {
  
    //
    // Software writes a 1'b1 to clear this bit.
    //
    //GenPmCon1 &= ~B_PMC_GEN_PMCON_SUS_PWR_FLR;
    //MmioWrite32 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, GenPmCon1);
  
    //
    // Clear Wake Status (WAK_STS) and Sleep Type (SLP_TYP)
    //
    IoWrite16 ((AcpiBaseAddr + R_ACPI_PM1_STS), B_ACPI_PM1_STS_WAK);
    DataUint16 = (IoRead16 (AcpiBaseAddr + R_ACPI_PM1_CNT) & ~B_ACPI_PM1_CNT_SLP_TYP);
    IoWrite16 ((AcpiBaseAddr + R_ACPI_PM1_CNT), DataUint16);
  }

  return EFI_SUCCESS;
}

BOOLEAN
IsRtcUipAlwaysSet (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  )
/*++

Routine Description:


Arguments:


Returns:


--*/
{

  EFI_PEI_STALL_PPI *StallPpi;
  UINTN             Count;


  (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);

  for (Count = 0; Count < 500; Count++) { // Maximum waiting approximates to 1.5 seconds (= 3 msec * 500)
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERA);
    if ((IoRead8 (R_RTC_TARGET2) & B_RTC_REGISTERA_UIP) == 0) {
      return FALSE;
    }

    StallPpi->Stall (PeiServices, StallPpi, 3000);
  }

  return TRUE;
}

EFI_STATUS
RtcPowerFailureHandler (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  )
/*++

Routine Description:


Arguments:


Returns:


--*/
{

  UINT16          DataUint16;
  UINT8           DataUint8;
  BOOLEAN         RtcUipIsAlwaysSet;
  //
  // VLV BIOS Specification 0.6.2 - Section 18.4.3, "Power Failure Consideration"
  //
  // When the RTC_PWR_STS bit is set, it indicates that the RTCRST# signal went low.
  // Software should clear this bit. Changing the RTC battery sets this bit.
  // System BIOS should reset CMOS to default values if the RTC_PWR_STS bit is set.
  //
  // VLV BIOS Specification Update 0.6.2
  // The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set before memory initialization.
  // This will ensure that the RTC state machine has been initialized.
  //  1.  If the RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion or a battery failure,
  //        steps 2 through 5 should be executed.
  //  2.  Set RTC Register 0x0A[6:4] to '110' or '111'.
  //  3.  Set RTC Register 0x0B[7].
  //  4.  Set RTC Register 0x0A[6:4] to '010'.
  //  5.  Clear RTC Register 0x0B[7].
  //
  DataUint16        = MmioRead16 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  RtcUipIsAlwaysSet = IsRtcUipAlwaysSet (PeiServices);
  if ((DataUint16 & B_PMC_GEN_PMCON_RTC_PWR_STS) || (RtcUipIsAlwaysSet)) {
    //
    // Execute the sequence below. This will ensure that the RTC state machine has been initialized.
    //
    // Step 1.
    // BIOS clears this bit by writing a '0' to it.
    //
    if (DataUint16 & B_PMC_GEN_PMCON_RTC_PWR_STS) {
//[-start-160628-IB07400749-remove]//
//      DataUint16 &= ~B_PMC_GEN_PMCON_RTC_PWR_STS;
//      MmioWrite16 ((PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1), DataUint16);
//[-end-160628-IB07400749-remove]//

      //
      // Set to invalid date in order to reset the time to
      // BIOS build time later in the boot (SBRUN.c file).
      //
      IoWrite8 (R_RTC_INDEX2, R_RTC_YEAR);
      IoWrite8 (R_RTC_TARGET2, 0x0FF);
      IoWrite8 (R_RTC_INDEX2, R_RTC_MONTH);
      IoWrite8 (R_RTC_TARGET2, 0x0FF);
      IoWrite8 (R_RTC_INDEX2, R_RTC_DAYOFMONTH);
      IoWrite8 (R_RTC_TARGET2, 0x0FF);
      IoWrite8 (R_RTC_INDEX2, R_RTC_DAYOFWEEK);
      IoWrite8 (R_RTC_TARGET2, 0x0FF);
    }

    //
    // Step 2.
    // Set RTC Register 0Ah[6:4] to '110' or '111'.
    //
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERA);
    IoWrite8 (R_RTC_TARGET2, (V_RTC_REGISTERA_DV_DIV_RST1 | V_RTC_REGISTERA_RS_976P5US));

    //
    // Step 3.
    // Set RTC Register 0Bh[7].
    //
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERB);
    DataUint8 = (IoRead8 (R_RTC_TARGET2) | B_RTC_REGISTERB_SET);
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERB);
    IoWrite8 (R_RTC_TARGET2, DataUint8);

    //
    // Step 4.
    // Set RTC Register 0Ah[6:4] to '010'.
    //
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERA);
    IoWrite8 (R_RTC_TARGET2, (V_RTC_REGISTERA_DV_NORM_OP | V_RTC_REGISTERA_RS_976P5US));

    //
    // Step 5.
    // Clear RTC Register 0Bh[7].
    //
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERB);
    DataUint8 = (IoRead8 (R_RTC_TARGET2) & (UINT8)~B_RTC_REGISTERB_SET);
    IoWrite8 (R_RTC_INDEX2, R_RTC_REGISTERB);
    IoWrite8 (R_RTC_TARGET2, DataUint8);
  }

  return EFI_SUCCESS;
}


