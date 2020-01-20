/** @file
  SC Platform Lib implementation.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2016 Intel Corporation.

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
#include "ScPlatformLibrary.h"
#include <Library/SteppingLib.h>
#include <ScRegs/RegsP2sb.h>

///
/// For BXT-P, it supports two express port controllers:
///   Controller 1:
///     Port 1-2, Device 20, function 0-1
///   Controller 2:
///     Port 3-6, Device 19, function 0-3
/// For BXT-X, it supports one express port controllers:
///   Controller 1:
///     Port 1-4, Device 19, function 0-3
/// For BXT-0/1, it supports one express port controllers:
///   Controller 1:
///     Port 1-2, Device 20, function 0-1
///
GLOBAL_REMOVE_IF_UNREFERENCED CONST PCIE_CONTROLLER_INFO mPcieControllerInfo[] = {
  { PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1, PID_PCIE0,  0 },
  { PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2, PID_PCIE1,  2 }
};
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT32 mPcieControllerInfoSize = sizeof (mPcieControllerInfo) / sizeof (mPcieControllerInfo[0]);

/**
  Get Sc Maximum Pcie Root Port Number

  @retval UINT8                         Sc Maximum Pcie Root Port Number
**/
UINT8
EFIAPI
GetScMaxPciePortNum (
  VOID
  )
{
  BXT_SERIES  BxtSeries;

  BxtSeries = GetBxtSeries ();
  switch (BxtSeries) {
    case BxtP:
      return SC_BXTP_PCIE_MAX_ROOT_PORTS;
      break;
    case Bxt1:
      return SC_BXT_PCIE_MAX_ROOT_PORTS;
      break;
    default:
      return 0;
      break;
  }
}

/**
  Delay for at least the request number of microseconds.
  This function would be called by runtime driver, please do not use any MMIO marco here.

  @param[in] Microseconds               Number of microseconds to delay.
**/
VOID
EFIAPI
ScPmTimerStall (
  IN  UINTN   Microseconds
  )
{
  UINTN   Ticks;
  UINTN   Counts;
  UINTN   CurrentTick;
  UINTN   OriginalTick;
  UINTN   RemainingTick;
  UINT16  AcpiBaseAddr;

  //
  //pre-silicon start
  //
  if (PLATFORM_ID != VALUE_REAL_PLATFORM){
    Microseconds = 0;
  }//pre-silicon end

  if (Microseconds == 0) {
    return;
  }

  //
  // Please use PciRead here, it will link to MmioRead
  // if the caller is a Runtime driver, please use ScDxeRuntimePciLibPciExpress library, refer
  // PciExpressRead() on Library\DxeRuntimePciLibPciExpress\DxeRuntimePciLibPciExpress.c for the details.
  // For the rest please use EdkIIGlueBasePciLibPciExpress library
  //

  //
  // Read ACPI Base Address
  //
  AcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

  OriginalTick  = IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_TMR)) & B_ACPI_PM1_TMR_VAL;
  CurrentTick   = OriginalTick;

  //
  // The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

  //
  // The loops needed by timer overflow
  //
  Counts = Ticks / V_ACPI_PM1_TMR_MAX_VAL;

  //
  // Remaining clocks within one loop
  //
  RemainingTick = Ticks % V_ACPI_PM1_TMR_MAX_VAL;

  //
  // not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  // one I/O operation, and maybe generate SMI
  //
  while ((Counts != 0) || (RemainingTick > CurrentTick)) {
    CurrentTick = IoRead32 ((UINTN) (AcpiBaseAddr + R_ACPI_PM1_TMR)) & B_ACPI_PM1_TMR_VAL;
    //
    // Check if timer overflow
    //
    if ((CurrentTick < OriginalTick)) {
      if (Counts != 0) {
        Counts--;
      } else {
        //
        // If timer overflow and Counts equ to 0, that means we already stalled more than
        // RemainingTick, break the loop here
        //
        break;
      }
    }

    OriginalTick = CurrentTick;
  }
}

/**
  Check whether SPI is in descriptor mode

  @param[in] SpiBase                The SC SPI Base Address

  @retval TRUE                      SPI is in descriptor mode
  @retval FALSE                     SPI is not in descriptor mode
**/
BOOLEAN
EFIAPI
ScIsSpiDescriptorMode (
  IN  UINTN   SpiBase
  )
{
  if ((MmioRead16 (SpiBase + R_SPI_HSFS) & B_SPI_HSFS_FDV) == B_SPI_HSFS_FDV) {
    MmioAndThenOr32 (
      SpiBase + R_SPI_FDOC,
      (UINT32) (~(B_SPI_FDOC_FDSS_MASK | B_SPI_FDOC_FDSI_MASK)),
      (UINT32) (V_SPI_FDOC_FDSS_FSDM | R_SPI_FDBAR_FLVALSIG)
      );
    if ((MmioRead32 (SpiBase + R_SPI_FDOD)) == V_SPI_FDBAR_FLVALSIG) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
}

/**
  This function can be called to poll for certain value within a time given.

  @param[in] MmioAddress            The Mmio Address.
  @param[in] BitMask                Bits to be masked.
  @param[in] BitValue               Value to be polled.
  @param[in] DelayTime              Delay time in terms of 100 micro seconds.

  @retval EFI_SUCCESS               Successfully polled the value.
  @retval EFI_TIMEOUT               Timeout while polling the value.
**/
EFI_STATUS
EFIAPI
ScMmioPoll32 (
  IN  UINTN         MmioAddress,
  IN  UINT32        BitMask,
  IN  UINT32        BitValue,
  IN  UINT16        DelayTime
  )
{
  UINT32  LoopTime;
  UINT8   PollSuccess;

  LoopTime    = 0;
  PollSuccess = 0;

  for (LoopTime = 0; LoopTime < DelayTime; LoopTime++) {
    if ((MmioRead32 (MmioAddress) & BitMask) == BitValue) {
      PollSuccess = 1;
      break;
    } else {
      ScPmTimerStall (100);
    }
  }

  if (PollSuccess) {
    return EFI_SUCCESS;
  } else {
    return EFI_TIMEOUT;
  }
}

/**
  Determine if the specificed device is available

  @param[in] RpDev  Device number

  @retval FALSE     Device is not available.
          TRUE      Device is available.
**/
BOOLEAN
IsPcieControllerAvailable (
  IN UINTN   *RpDev
)
{
  UINT32  FuncDisableReg;
  UINT32  FuncDisableMask;

  if (*RpDev == PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1) {
    FuncDisableReg  = MmioRead32 (PMC_BASE_ADDRESS + R_PMC_FUNC_DIS);
    FuncDisableMask = (B_PMC_FUNC_DIS_PCIE0_P1 | B_PMC_FUNC_DIS_PCIE0_P0);
  } else {
    FuncDisableReg = MmioRead32 (PMC_BASE_ADDRESS + R_PMC_FUNC_DIS_1);
    FuncDisableMask = (B_PMC_FUNC_DIS_1_PCIE1_P3 | B_PMC_FUNC_DIS_1_PCIE1_P2 | B_PMC_FUNC_DIS_1_PCIE1_P1 | B_PMC_FUNC_DIS_1_PCIE1_P0);
  }

  if ((FuncDisableReg & FuncDisableMask) == FuncDisableMask) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Get Sc Pcie Root Port Device and Function Number by Root Port physical Number

  @param[in]  RpNumber              Root port physical number. (0-based)
  @param[out] RpDev                 Return corresponding root port device number.
  @param[out] RpFun                 Return corresponding root port function number.

  @retval     EFI_SUCCESS           Root port device and function is retrieved
  @retval     EFI_INVALID_PARAMETER RpNumber is invalid
**/
EFI_STATUS
EFIAPI
GetScPcieRpDevFun (
  IN  UINTN   RpNumber,
  OUT UINTN   *RpDev,
  OUT UINTN   *RpFun
  )
{
  UINTN       Index;
  UINTN       FuncIndex;
  UINT32      PciePcd;

  //
  // if BXT SC , RpNumber must be < 6.
  //
  if (RpNumber >= GetScMaxPciePortNum ()) {
    DEBUG ((DEBUG_ERROR, "GetScPcieRpDevFun invalid RpNumber %x\n", RpNumber));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }
  if (RpNumber >= 2) {
    Index = 1;
  }else {
    Index = 0;
  }
  FuncIndex = RpNumber - mPcieControllerInfo[Index].RpNumBase;
  *RpDev = mPcieControllerInfo[Index].DevNum;

  // Before doing the actual PCD read, ensure the device is available
  if (IsPcieControllerAvailable (RpDev) == TRUE) {
    PchPcrRead32 (mPcieControllerInfo[Index].Pid, R_PCH_PCR_SPX_PCD, &PciePcd);
    *RpFun = (PciePcd >> (FuncIndex * S_PCH_PCR_SPX_PCD_RP_FIELD)) & B_PCH_PCR_SPX_PCD_RP1FN;
  } else {
    *RpFun = FuncIndex;
  }

  return EFI_SUCCESS;
}

/**
  Get Root Port physical Number by Sc Pcie Root Port Device and Function Number

  @param[in]  RpDev                 Root port device number.
  @param[in]  RpFun                 Root port function number.
  @param[out] RpNumber              Return corresponding Root port physical number.

  @retval     EFI_SUCCESS           Physical root port is retrieved
  @retval     EFI_INVALID_PARAMETER RpDev and/or RpFun are invalid
  @retval     EFI_UNSUPPORTED       Root port device and function is not assigned to any physical root port
**/
EFI_STATUS
EFIAPI
GetScPcieRpNumber (
  IN  UINTN   RpDev,
  IN  UINTN   RpFun,
  OUT UINTN   *RpNumber
  )
{
  UINTN       Index;
  UINTN       FuncIndex;
  UINT32      PciePcd;
  //
  // if BXT SC, must be Dev == 19 and Fun < 4, or Dev == 20 and Fun < 2.
  //

  if (!((RpDev == PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_1) && (RpFun < 2)) &&
      !((RpDev == PCI_DEVICE_NUMBER_SC_PCIE_DEVICE_2) && (RpFun < 4)))
  {
    DEBUG ((DEBUG_ERROR, "GetPchPcieRpNumber invalid RpDev %x RpFun %x", RpDev, RpFun));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < mPcieControllerInfoSize; Index++) {
    if (mPcieControllerInfo[Index].DevNum == RpDev) {
      PchPcrRead32 (mPcieControllerInfo[Index].Pid, R_PCH_PCR_SPX_PCD, &PciePcd);
      for (FuncIndex = 0; FuncIndex < 4; FuncIndex ++) {
        if (RpFun == ((PciePcd >> (FuncIndex * S_PCH_PCR_SPX_PCD_RP_FIELD)) & B_PCH_PCR_SPX_PCD_RP1FN)) {
          break;
        }
      }
      if (FuncIndex < 4) {
        *RpNumber = mPcieControllerInfo[Index].RpNumBase + FuncIndex;
        break;
      }
    }
  }
  if (Index >= mPcieControllerInfoSize) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}
/**
  Read PCR register. (This is internal function)
  It returns PCR register and size in 1byte/2bytes/4bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of this Port ID
  @param[in]  Size                      Size for read. Must be 1 or 2 or 4.
  @param[out] OutData                   Buffer of Output Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
STATIC
EFI_STATUS
PchPcrRead (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINTN                             Size,
  OUT UINT32                            *OutData
  )
{
  UINTN    P2sbBase;
  BOOLEAN  P2sbOrgStatus;

  P2sbOrgStatus = FALSE;
  P2sbBase = (UINT32)MmPciAddress(0, \
                                  0, \
                                  PCI_DEVICE_NUMBER_P2SB, \
                                  PCI_FUNCTION_NUMBER_P2SB, \
                                  0
                                  );

  if ((Offset & (Size - 1)) != 0) {
    DEBUG ((DEBUG_ERROR, "PchPcrRead error. Invalid Offset: %x Size: %x", Offset, Size));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }
  //
  // @todo SKL PCH: check PID that not expected to use this routine, such as CAM_FLIS, CSME0
  //

  PchRevealP2sb (P2sbBase, &P2sbOrgStatus);

  switch (Size) {
  case 4:
    *(UINT32*)OutData = MmioRead32 (SC_PCR_ADDRESS (Pid, Offset));
    break;
  case 2:
    *(UINT16*)OutData = MmioRead16 (SC_PCR_ADDRESS (Pid, Offset));
    break;
  case 1:
    *(UINT8*) OutData = MmioRead8  (SC_PCR_ADDRESS (Pid, Offset));
    break;
  default:
    break;
  }

  if (!P2sbOrgStatus) {
    PchHideP2sb (P2sbBase);
  }

  return EFI_SUCCESS;
}

/**
  Read PCR register.
  It returns PCR register and size in 4bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of this Port ID
  @param[out] OutData                   Buffer of Output Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrRead32 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  OUT UINT32                            *OutData
  )
{
  return PchPcrRead (Pid, Offset, 4, (UINT32*) OutData);
}

/**
  Read PCR register.
  It returns PCR register and size in 2bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of this Port ID
  @param[out] OutData                   Buffer of Output Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrRead16 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  OUT UINT16                            *OutData
  )
{
  return PchPcrRead (Pid, Offset, 2, (UINT32*) OutData);
}

/**
  Read PCR register.
  It returns PCR register and size in 1bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of this Port ID
  @param[out] OutData                   Buffer of Output Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrRead8 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  OUT UINT8                             *OutData
  )
{
  return PchPcrRead (Pid, Offset, 1, (UINT32*) OutData);
}

#ifdef EFI_DEBUG
/**
  Check if the port ID is available for sideband mmio read/write

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of this Port ID

  @retval TRUE                          available for sideband Mmio read/write method
  @retval FALSE                         inavailable for sideband Mmio read/write method
**/
BOOLEAN
PchPcrWriteMmioCheck (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset
  )
{
  //
  // 1. USB2 AFE register must use SBI method
  //

  //
  // 2. GPIO unlock register field must use SBI method
  //
//
// @todo: to check BXT register
//
/*
  if (Pid == PID_GPIOCOM0) {
    if (((PchSeries == PchLp) &&
      ((Offset == R_PCH_LP_PCR_GPIO_GPP_A_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_A_PADCFGLOCKTX) ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_B_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_B_PADCFGLOCKTX))) ||
       ((PchSeries == PchH) &&
      ((Offset == R_PCH_H_PCR_GPIO_GPP_A_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_A_PADCFGLOCKTX) ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_B_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_B_PADCFGLOCKTX))))
    {
      return FALSE;
    }
  }
  if (Pid == PID_GPIOCOM1) {
    if (((PchSeries == PchLp) &&
      ((Offset == R_PCH_LP_PCR_GPIO_GPP_C_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_C_PADCFGLOCKTX) ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_D_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_D_PADCFGLOCKTX) ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_E_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_E_PADCFGLOCKTX))) ||
       ((PchSeries == PchH) &&
      ((Offset == R_PCH_H_PCR_GPIO_GPP_C_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_C_PADCFGLOCKTX) ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_D_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_D_PADCFGLOCKTX) ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_E_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_E_PADCFGLOCKTX) ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_F_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_F_PADCFGLOCKTX) ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_G_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_G_PADCFGLOCKTX) ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_H_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_H_PADCFGLOCKTX))))
    {
      return FALSE;
    }
  }
  if (Pid == PID_GPIOCOM2) {
    if (((PchSeries == PchLp) &&
      ((Offset == R_PCH_LP_PCR_GPIO_GPD_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPD_PADCFGLOCKTX))) ||
       ((PchSeries == PchH) &&
      ((Offset == R_PCH_H_PCR_GPIO_GPD_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPD_PADCFGLOCKTX))))
    {
      return FALSE;
    }
  }
  if (Pid == PID_GPIOCOM3) {
    if (((PchSeries == PchLp) &&
      ((Offset == R_PCH_LP_PCR_GPIO_GPP_F_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_F_PADCFGLOCKTX) ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_G_PADCFGLOCK)   ||
       (Offset == R_PCH_LP_PCR_GPIO_GPP_G_PADCFGLOCKTX))) ||
       ((PchSeries == PchH) &&
      ((Offset == R_PCH_H_PCR_GPIO_GPP_I_PADCFGLOCK)   ||
       (Offset == R_PCH_H_PCR_GPIO_GPP_I_PADCFGLOCKTX))))
    {
      return FALSE;
    }
  }
*/
  //
  // 3. CIO2 FLIS regsiter must use SBI method
  //

  //
  // 4. CSME0 based PCR should use the SBI method due to the FID requirement
  //
//
// @todo: to check BXT register
//
/*
  if (Pid == PID_CSME0) {
    return FALSE;
  }
*/
  return TRUE;
}
#endif

/**
  Write PCR register. (This is internal function)
  It programs PCR register and size in 1byte/2bytes/4bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  Size                      Size for read. Must be 1 or 2 or 4.
  @param[in]  InData                    Input Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
STATIC
EFI_STATUS
PchPcrWrite (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINTN                             Size,
  IN  UINT32                            InData
  )
{
  UINTN    P2sbBase;
  BOOLEAN  P2sbOrgStatus;

  P2sbOrgStatus = FALSE;
  P2sbBase = (UINT32)MmPciAddress(0, \
                                  0, \
                                  PCI_DEVICE_NUMBER_P2SB, \
                                  PCI_FUNCTION_NUMBER_P2SB, \
                                  0
                                  );

  if ((Offset & (Size - 1)) != 0) {
    DEBUG ((DEBUG_ERROR, "PchPcrWrite error. Invalid Offset: %x Size: %x", Offset, Size));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }
#ifdef EFI_DEBUG
  if (!PchPcrWriteMmioCheck (Pid, Offset)) {
    DEBUG ((DEBUG_ERROR, "PchPcrWrite error. Pid: %x Offset: %x should access through SBI interface", Pid, Offset));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }
#endif

  PchRevealP2sb (P2sbBase, &P2sbOrgStatus);
  //
  // Write the PCR register with provided data
  // Then read back PCR register to prevent from back to back write.
  //
  switch (Size) {
    case 4:
      MmioWrite32 (SC_PCR_ADDRESS (Pid, Offset), (UINT32)InData);
      break;
    case 2:
      MmioWrite16 (SC_PCR_ADDRESS (Pid, Offset), (UINT16)InData);
      break;
    case 1:
      MmioWrite8  (SC_PCR_ADDRESS (Pid, Offset), (UINT8) InData);
      break;
    default:
      break;
  }
  //
  // @todo: The Function Disable offset is TBD in the latest BXT-p LPC HAS rev0.8, Sept., 2014.
  //
  //MmioRead32  (SC_PCR_ADDRESS (PID_LPC, R_PCH_PCR_LPC_GCFD));

  if (!P2sbOrgStatus) {
    PchHideP2sb (P2sbBase);
  }

  return EFI_SUCCESS;
}

/**
  Write PCR register.
  It programs PCR register and size in 4bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  InData                    Input Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrWrite32 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINT32                            InData
  )
{
  return PchPcrWrite (Pid, Offset, 4, InData);
}

/**
  Write PCR register.
  It programs PCR register and size in 2bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  InData                    Input Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrWrite16 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINT16                            InData
  )
{
  return PchPcrWrite (Pid, Offset, 2, InData);
}

/**
  Write PCR register.
  It programs PCR register and size in 1bytes.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  InData                    Input Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrWrite8 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINT8                             InData
  )
{
  return PchPcrWrite (Pid, Offset, 1, InData);
}

/**
  Reads an 4-byte Pcr register, performs a bitwise AND followed by a bitwise
  inclusive OR, and writes the result back to the 4-byte Pcr register.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  AndData                   AND Data. Must be the same size as Size parameter.
  @param[in]  OrData                    OR Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrAndThenOr32 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINT32                            AndData,
  IN  UINT32                            OrData
  )
{
  EFI_STATUS                            Status;
  UINT32                                Data32;

  Status  = PchPcrRead (Pid, Offset, 4, &Data32);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Data32 &= AndData;
  Data32 |= OrData;
  Status  = PchPcrWrite (Pid, Offset, 4, Data32);
  return Status;
}

/**
  Reads an 2-byte Pcr register, performs a bitwise AND followed by a bitwise
  inclusive OR, and writes the result back to the 2-byte Pcr register.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  AndData                   AND Data. Must be the same size as Size parameter.
  @param[in]  OrData                    OR Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrAndThenOr16 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINT16                            AndData,
  IN  UINT16                            OrData
  )
{
  EFI_STATUS                            Status;
  UINT16                                Data16;

  Status  = PchPcrRead (Pid, Offset, 2, (UINT32*) &Data16);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Data16 &= AndData;
  Data16 |= OrData;
  Status  = PchPcrWrite (Pid, Offset, 2, Data16);
  return Status;
}

/**
  Reads an 1-byte Pcr register, performs a bitwise AND followed by a bitwise
  inclusive OR, and writes the result back to the 1-byte Pcr register.
  The Offset should not exceed 0xFFFF and must be aligned with size.

  @param[in]  Pid                       Port ID
  @param[in]  Offset                    Register offset of Port ID.
  @param[in]  AndData                   AND Data. Must be the same size as Size parameter.
  @param[in]  OrData                    OR Data. Must be the same size as Size parameter.

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_INVALID_PARAMETER         Invalid offset passed.
**/
EFI_STATUS
PchPcrAndThenOr8 (
  IN  SC_SBI_PID                        Pid,
  IN  UINT16                            Offset,
  IN  UINT8                             AndData,
  IN  UINT8                             OrData
  )
{
  EFI_STATUS                            Status;
  UINT8                                 Data8;

  Status  = PchPcrRead (Pid, Offset, 1, (UINT32*) &Data8);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Data8 &= AndData;
  Data8 |= OrData;
  Status  = PchPcrWrite (Pid, Offset, 1, Data8);
  return Status;
}

/**
  Hide P2SB device.

  @param[in]  P2sbBase                  Pci base address of P2SB controller.

  @retval EFI_SUCCESS                   Always return success.
**/
EFI_STATUS
PchHideP2sb (
  IN UINTN                                 P2sbBase
  )
{
  MmioWrite8 (P2sbBase + R_P2SB_P2SBC + 1, BIT0);
  return EFI_SUCCESS;
}

/**
  Reveal P2SB device.
  Also return the original P2SB status which is for Hidding P2SB or not after.
  If OrgStatus is not NULL, then TRUE means P2SB is unhidden,
  and FALSE means P2SB is hidden originally.

  @param[in]  P2sbBase                  Pci base address of P2SB controller.
  @param[out] OrgStatus                 Original P2SB hidding/unhidden status

  @retval EFI_SUCCESS                   Always return success.
**/
EFI_STATUS
PchRevealP2sb (
  IN UINTN                                 P2sbBase,
  OUT BOOLEAN                              *OrgStatus
  )
{
  BOOLEAN                               DevicePresent;

  DevicePresent = (MmioRead16 (P2sbBase + PCI_VENDOR_ID_OFFSET) != 0xFFFF);
  if (OrgStatus != NULL) {
    *OrgStatus = DevicePresent;
  }
  if (!DevicePresent) {
    MmioWrite8 (P2sbBase + R_P2SB_P2SBC + 1, 0);
  }
  return EFI_SUCCESS;
}

/**
  Enable VTd support in PSF.

  @retval EFI_SUCCESS                   Successfully completed.
**/
EFI_STATUS
PchPsfEnableVtd (
  VOID
  )
{
  UINT8 PsfPidIndex;
  UINT8 PsfList [5] = {SB_PORTID_PSF0, SB_PORTID_PSF1, SB_PORTID_PSF2, SB_PORTID_PSF3, SB_PORTID_PSF4};

  for (PsfPidIndex = 0; PsfPidIndex < sizeof (PsfList); ++PsfPidIndex) {
    SideBandAndThenOr32 (
      PsfList[PsfPidIndex],
      R_PCH_PCR_PSF_ROOTSPACE_CONFIG_RS0,
      ~0u,
      B_PCH_PCR_PSF_ROOTSPACE_CONFIG_RS0_ENADDRP2P | B_PCH_PCR_PSF_ROOTSPACE_CONFIG_RS0_VTDEN
      );
  }
  return EFI_SUCCESS;
}

/**
  Get Sc Maximum Usb3 Port Number of XHCI Controller

  @retval UINT8                         Sc Maximum Usb3 Port Number of XHCI Controller
**/
UINT8
EFIAPI
GetScXhciMaxUsb3PortNum (
  VOID
  )
{
  BXT_SERIES  BxtSeries;

  BxtSeries = GetBxtSeries ();
  switch (BxtSeries) {
    case Bxt1:
      return SC_BXT_MAX_USB3_PORTS;
      break;
    case BxtP:
      return SC_BXTP_MAX_USB3_PORTS;
      break;
    default:
      return 0;
      break;
  }
}

/**
  Get Sc Maximum Usb2 Port Number of XHCI Controller

  @retval UINT8                         Sc Maximum Usb2 Port Number of XHCI Controller
**/
UINT8
EFIAPI
GetScXhciMaxUsb2PortNum (
  VOID
  )
{
  BXT_SERIES  BxtSeries;

  BxtSeries = GetBxtSeries ();
  switch (BxtSeries) {
    case Bxt1:
      return SC_BXT_MAX_USB2_PORTS;
      break;
    case BxtP:
      return SC_BXTP_MAX_USB2_PORTS;
      break;
    default:
      return 0;
      break;
  }
}

/**
  Determine if the specificed Port is mapped onto a lane

  @param[in] DevicePort                 Device Port

  @retval EFI_SUCCESS                   If the port is mapped.
          EFI_UNSUPPORTED               If the port is not mapped.
**/
EFI_STATUS
IsPortAvailable (
  SC_DEVICE_PORT DevicePort
  )
{
  BXT_SERIES  BxtSeries;
  UINT32      Los1;

  BxtSeries = GetBxtSeries();
  Los1 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS1_REG_BASE);

  if( BxtSeries == BxtP ) {
    switch(DevicePort) {
      case PcieP1:
        if (((Los1 & B_SC_PCR_FIA_L3O) >> 12) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      case PcieP2:
        if (((Los1 & B_SC_PCR_FIA_L2O) >> 8) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      case PcieP3:
        if (((Los1 & B_SC_PCR_FIA_L7O) >> 28) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      case PcieP4:
        if (((Los1 & B_SC_PCR_FIA_L6O) >> 24)== V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      case PcieP5:
        if (((Los1 & B_SC_PCR_FIA_L5O) >> 20) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      case PcieP6:
        if (((Los1 & B_SC_PCR_FIA_L4O) >> 16) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "Unsupported DevicePort"));
        ASSERT (FALSE);
        return EFI_UNSUPPORTED;
        break;
    }
  } else if( BxtSeries == Bxt1 ) {
    switch(DevicePort) {
      case PcieP1:
        if (((Los1 & B_SC_PCR_FIA_L2O) >> 8) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      case PcieP2:
        if (((Los1 & B_SC_PCR_FIA_L1O) >> 4) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          return EFI_SUCCESS;
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "Unsupported DevicePort"));
        ASSERT (FALSE);
        return EFI_UNSUPPORTED;
        break;
    }
  }

  return EFI_UNSUPPORTED;
}