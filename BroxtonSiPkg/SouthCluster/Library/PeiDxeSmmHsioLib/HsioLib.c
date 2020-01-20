/** @file
  SC HSIO Library.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

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

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <ScAccess.h>
#include <Library/SteppingLib.h>
#include <Library/SideBandLib.h>
#include <Library/HsioLib.h>

/**
  The function returns the Port Id and lane owner for the specified lane

  @param[in]  LaneNum             Lane number that needs to be checked
  @param[out] PortId              Common Lane End Point ID
  @param[out] LaneOwner           Lane Owner

  @retval EFI_SUCCESS             Read success
  @retval EFI_INVALID_PARAMETER   Invalid lane number
**/
EFI_STATUS
EFIAPI
ScGetLaneInfo (
  IN  UINT32                            LaneNum,
  OUT UINT8                             *PortId,
  OUT UINT8                             *LaneOwner
  )
{
  BXT_SERIES  BxtSeries;
  UINT32      Los1;
  UINT32      Los2;

  Los1 = 0;
  Los2 = 0;

  BxtSeries = GetBxtSeries ();

  if ( ((LaneNum > 2) && (BxtSeries == Bxt1)) || ((LaneNum > 10) && (BxtSeries == BxtP))) {
    return EFI_INVALID_PARAMETER;
  } else if (LaneNum < 8) {
    Los1 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS1_REG_BASE);
  } else if (LaneNum < 16) {
    Los2 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS2_REG_BASE);
  }

  if (BxtSeries == BxtP) {
    switch (LaneNum) {
      case 0:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L0O));
        *PortId = PID_MODPHY0;
        break;
      case 1:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L1O)>>4);
        *PortId = PID_MODPHY0;
        break;
      case 2:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L2O)>>8);
        *PortId = PID_MODPHY0;
        break;
      case 3:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L3O)>>12);
        *PortId = PID_MODPHY0;
        break;
      case 4:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L4O)>>16);
        *PortId = PID_MODPHY0;
        break;
      case 5:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L5O)>>20);
        *PortId = PID_MODPHY0;
        break;
      case 6:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L6O)>>24);
        *PortId = PID_MODPHY0;
        break;
      case 7:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L7O)>>28);
        *PortId = PID_MODPHY0;
        break;
      case 8:
        *LaneOwner = (UINT8) (Los2 & B_SC_PCR_FIA_L8O);
        *PortId = PID_MODPHY1;
        break;
      case 9:
        *LaneOwner = (UINT8) ((Los2 & B_SC_PCR_FIA_L9O)>>4);
        *PortId = PID_MODPHY1;
        break;
    }
  } else {
    switch (LaneNum) {
      case 0:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L0O));
        *PortId = PID_MODPHY0;
        break;
      case 1:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L1O)>>4);
        *PortId = PID_MODPHY0;
        break;
      case 2:
        *LaneOwner = (UINT8) ((Los1 & B_SC_PCR_FIA_L2O)>>8);
        *PortId = PID_MODPHY0;
        break;
    }
  }
  return EFI_SUCCESS;
}

/**
  Determine the lane number of a specified port

  @param[in]  Usb3LaneIndex             USB3 Lane Index
  @param[out] LaneNum                   Lane Number

  @retval EFI_SUCCESS                   Lane number valid.
  @retval EFI_UNSUPPORTED               Incorrect input device port
**/
EFI_STATUS
ScGetUsb3LaneNum (
  IN UINT32              Usb3LaneIndex,
  OUT UINT8              *LaneNum
  )
{
  UINT32 Los1;
  UINT32 Los2;

  Los1 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS1_REG_BASE);
  Los2 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS2_REG_BASE);

  switch (Usb3LaneIndex) {
    case 0:
      if ((Los1 & B_SC_PCR_FIA_L0O) == V_SC_PCR_FIA_LANE_OWN_USB3) {
        *LaneNum = 0;
        return EFI_SUCCESS;
      }
      break;
    case 1:
      if (((Los1 & B_SC_PCR_FIA_L1O) >> 4) == V_SC_PCR_FIA_LANE_OWN_USB3) {
        *LaneNum = 1;
        return EFI_SUCCESS;
      }
      break;
    case 2:
      if (((Los1 & B_SC_PCR_FIA_L2O) >> 8) == V_SC_PCR_FIA_LANE_OWN_USB3) {
        *LaneNum = 2;
        return EFI_SUCCESS;
      }
      break;
    case 3:
      if (((Los1 & B_SC_PCR_FIA_L3O) >> 12) == V_SC_PCR_FIA_LANE_OWN_USB3) {
        *LaneNum = 3;
        return EFI_SUCCESS;
      }
      break;
    case 4:
      if (((Los1 & B_SC_PCR_FIA_L4O) >> 16) == V_SC_PCR_FIA_LANE_OWN_USB3) {
        *LaneNum = 4;
        return EFI_SUCCESS;
      }
      break;
    case 5:
      if ((Los2 & B_SC_PCR_FIA_L8O) == V_SC_PCR_FIA_LANE_OWN_USB3) {
        *LaneNum = 8;
        return EFI_SUCCESS;
      }
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Unsupported USB3 Lane Index"));
      ASSERT (FALSE);
      return EFI_UNSUPPORTED;
      break;
  }

  return EFI_UNSUPPORTED;
}

/**
  Determine the lane number of a specified port

  @param[in]  SataLaneIndex             Sata Lane Index
  @param[out] LaneNum                   Lane Number

  @retval EFI_SUCCESS                   Lane number valid.
  @retval EFI_UNSUPPORTED               Incorrect input device port
**/
EFI_STATUS
ScGetSataLaneNum (
  IN UINT32              SataLaneIndex,
  OUT UINT8              *LaneNum
  )
{
  UINT32 Los2;

  Los2 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS2_REG_BASE);

  switch (SataLaneIndex) {
    case 0:
      if (((Los2 & B_SC_PCR_FIA_L9O) >> 4) == V_SC_PCR_FIA_LANE_OWN_SATA) {
        *LaneNum = 9;
        return EFI_SUCCESS;
      }
      break;
    case 1:
      if ((Los2 & B_SC_PCR_FIA_L8O) == V_SC_PCR_FIA_LANE_OWN_SATA) {
        *LaneNum = 8;
        return EFI_SUCCESS;
      }
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Unsupported SATA Lane Index"));
      ASSERT (FALSE);
      return EFI_UNSUPPORTED;
      break;
  }

  return EFI_UNSUPPORTED;
}

/**
  Determine the lane number of a specified port

  @param[in]  PcieLaneIndex             PCIE Root Port Lane Index
  @param[out] LaneNum                   Lane Number

  @retval EFI_SUCCESS                   Lane number valid.
  @retval EFI_UNSUPPORTED               Incorrect input device port
**/
EFI_STATUS
ScGetPcieLaneNum (
  IN UINT32              PcieLaneIndex,
  OUT UINT8              *LaneNum
  )
{
  UINT32 Los1;

  Los1 = SideBandRead32 (PID_FIA, R_SC_PCR_FIA_LOS1_REG_BASE);

  if (GetBxtSeries () == BxtP) {
    switch (PcieLaneIndex) {
      case 0:
        if (((Los1 & B_SC_PCR_FIA_L7O) >> 28) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 7;
          return EFI_SUCCESS;
        }
        break;
      case 1:
        if (((Los1 & B_SC_PCR_FIA_L6O) >> 24)== V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 6;
          return EFI_SUCCESS;
        }
        break;
      case 2:
        if (((Los1 & B_SC_PCR_FIA_L5O) >> 20) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 5;
          return EFI_SUCCESS;
        }
        break;
      case 3:
        if (((Los1 & B_SC_PCR_FIA_L4O) >> 16) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 4;
          return EFI_SUCCESS;
        }
        break;
      case 4:
        if (((Los1 & B_SC_PCR_FIA_L3O) >> 12) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 3;
          return EFI_SUCCESS;
        }
        break;
      case 5:
        if (((Los1 & B_SC_PCR_FIA_L2O) >> 8) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 2;
          return EFI_SUCCESS;
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "Unsupported PCIE Root Port Lane Index"));
        ASSERT (FALSE);
        return EFI_UNSUPPORTED;
        break;
    }
  } else {
    switch (PcieLaneIndex) {
      case 0:
        if (((Los1 & B_SC_PCR_FIA_L2O) >> 8) == V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 0;
          return EFI_SUCCESS;
        }
        break;
      case 1:
        if (((Los1 & B_SC_PCR_FIA_L1O) >> 4)== V_SC_PCR_FIA_LANE_OWN_PCIE) {
          *LaneNum = 1;
          return EFI_SUCCESS;
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "Unsupported PCIE Root Port Lane Index"));
        ASSERT (FALSE);
        return EFI_UNSUPPORTED;
        break;
    }
  }
  return EFI_UNSUPPORTED;
}
