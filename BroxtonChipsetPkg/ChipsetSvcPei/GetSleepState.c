/** @file
 PEI Chipset Services Library.

 This file contains only one function that is PeiCsSvcGetSleepState().
 The function PeiCsSvcGetSleepState() use chipset services to identify
 sleep state.

***************************************************************************
* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

#include <Pi/PiPeiCis.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <PlatformBaseAddresses.h>
#include <ScAccess.h>


/**
 Get sleep type after wakeup

 @param[in]         PeiServices       Pointer to the PEI Service Table.
 @param[in]         SleepType         Sleep type to be returned.

 @retval            TRUE                A wake event occured without power failure.
 @retval            FALSE               Power failure occured or not a wakeup.

**/
BOOLEAN
GetSleepTypeAfterWakeup (
  IN  CONST EFI_PEI_SERVICES          **PeiServices,
  OUT UINT16                    *SleepType
  )
{
  UINT16  Pm1Sts;
  UINT16  Pm1Cnt;
  UINT16  GenPmCon1;
  UINT16  OriginalPm1Cnt;
  UINT16  AcpiBaseAddr;


  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);
  //
  // Chv BIOS Specification 0.6.2 - Section 18.4, "Power Failure Consideration"
  //
  // When the SUS_PWR_FLR bit is set, it indicates the SUS well power is lost.
  // This bit is in the SUS Well and defaults to 1?b1 based on RSMRST# assertion (not cleared by any type of reset).
  // System BIOS should follow cold boot path if SUS_PWR_FLR (PBASE + 0x20[14]),
  // GEN_RST_STS (PBASE + 0x20[9]) or PWRBTNOR_STS (ABASE + 0x00[11]) is set to 1?b1
  // regardless of the value in the SLP_TYP (ABASE + 0x04[12:10]) field.
  //
  GenPmCon1 = MmioRead16 (PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1);
  
  //
  // Read the ACPI registers
  //
  Pm1Sts  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_STS);
  OriginalPm1Cnt = Pm1Cnt  = IoRead16 (AcpiBaseAddr + R_ACPI_PM1_CNT);


  DEBUG ((EFI_D_ERROR, "ACPI Pm1Cnt Register: %04x\n", Pm1Cnt));
  DEBUG ((EFI_D_ERROR, "ACPI Pm1Sts Register: %04x\n", Pm1Sts));

  DEBUG ((EFI_D_ERROR, "PMC GenPmCon1 Register: %04x\n", GenPmCon1));
  if(GenPmCon1 != 0xFFFF){
    if ((GenPmCon1 & (B_PMC_GEN_PMCON_SUS_PWR_FLR | B_PMC_GEN_PMCON_GEN_RST_STS)) ||
       (Pm1Sts & B_ACPI_PM1_STS_PRBTNOR)) {
      // If power failure indicator, then don't attempt s3 resume.
      // Clear PM1_CNT of S3 and set it to S5 as we just had a power failure, and memory has
      // lost already.  This is to make sure no one will use PM1_CNT to check for S3 after
      // power failure.
      if ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) {
        Pm1Cnt = ((Pm1Cnt & ~B_ACPI_PM1_CNT_SLP_TYP) | V_ACPI_PM1_CNT_S5);
        IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_CNT, Pm1Cnt);
      }
      // Clear Wake Status (WAK_STS)
      //
      IoWrite16 ((AcpiBaseAddr + R_ACPI_PM1_STS), B_ACPI_PM1_STS_WAK);
    }
  }

//  //
//  // BWG 39.11.1 Step 3
//  //   Check the power failure bits that follow to make sure that the system 
//  //   can be resumed in a reliable manner: PRBTNOR_STS (ABASE + 0x00[11]), 
//  //   SUS_PWR_FLR (PBASE + 0x20[14]), and PWR_FLR PBASE + 0x22[0]). If any of
//  //   these bits are set, then the memory contents cannot be relied upon. So
//  //   the System BIOS should execute a CF9h reset and go through a normal
//  //   boot process instead of executing any of the steps described below. 
//  //
//  if (FeaturePcdGet (PcdSusPwrFlrReset)) {
//    if (((OriginalPm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) &&
//        (((Pm1Sts & B_ACPI_PM1_STS_PRBTNOR) != 0) ||
//        ((GenPmCon1 & (B_PMC_GEN_PMCON_SUS_PWR_FLR | B_PMC_GEN_PMCON_PWROK_FLR)) != 0))) {
//  
//        //
//        // Clear these failure bits
//        //
//        MmioOr32(PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, B_PMC_GEN_PMCON_SUS_PWR_FLR | B_PMC_GEN_PMCON_PWROK_FLR);
//        IoWrite16 (AcpiBaseAddr + R_ACPI_PM1_STS, B_ACPI_PM1_STS_PRBTNOR);
//    
//        IoWrite8 (R_PCH_RST_CNT, V_PCH_RST_CNT_HARDRESET);
//        CpuDeadLoop();
//    }
//  } else if (((OriginalPm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) &&
//             ((GenPmCon1 & B_PMC_GEN_PMCON_PWROK_FLR) != 0)) {
//    //
//    // Clear the failure bit
//    //
//    MmioOr32(PMC_BASE_ADDRESS + R_PMC_GEN_PMCON_1, B_PMC_GEN_PMCON_PWROK_FLR);
//    IoWrite8 (R_PCH_RST_CNT, V_PCH_RST_CNT_HARDRESET);
//    CpuDeadLoop();
//  }

  //
  // Get sleep type if a wake event occurred and there is no power failure
  //
  if ((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S3) {
    *SleepType = Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP;
    return TRUE;
  } else if((Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP) == V_ACPI_PM1_CNT_S4){
    *SleepType = Pm1Cnt & B_ACPI_PM1_CNT_SLP_TYP;
#if (TABLET_PF_ENABLE == 1)
    return FALSE;
#else
    return TRUE;
#endif
  }
  return FALSE;
}



/**
 To identify sleep state.

 @param[in, out]    SleepState          3 - This is an S3 restart
                                        4 - This is an S4 restart

 @retval            EFI_SUCCESS         Function always returns successfully
*/
EFI_STATUS
GetSleepState (
  IN OUT  UINT8              *SleepState
  )
{
  CONST EFI_PEI_SERVICES  **PeiServices;
  UINT16                  SleepType; 
  
  PeiServices = GetPeiServicesTablePointer ();
  if(GetSleepTypeAfterWakeup (PeiServices, &SleepType)) {
    switch (SleepType) {
      case V_ACPI_PM1_CNT_S3:
        *SleepState = 0x3;
        break;

      case V_ACPI_PM1_CNT_S4:
        *SleepState = 0x4;
        break;       
    }
  }

  return EFI_SUCCESS;
}

