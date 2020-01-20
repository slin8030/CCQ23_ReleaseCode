/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

@par Specification Reference:
**/

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/I2CLib.h>
#include <Library/EcMiscLib.h>
#include <Guid/PlatformInfo.h>
#include "PlatformId.h"
//[-start-161022-IB07400802-modify]//
//#if (BXTI_PF_ENABLE == 1)
#include <Library/PmcIpcLib.h>
//#endif
//[-end-161022-IB07400802-modify]//

/**

Routine Description:

  Returns the Strap FW Config ID of the platform from the TCA6424 IC.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  FwCfgId                   - FW Config ID as determined through the TCA6424.

Returns:

  EFI_SUCCESS               The function completed successfully.
  EFI_DEVICE_ERROR          TCA6424 fails to respond.

**/
EFI_STATUS
EFIAPI
GetFwCfgId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *FwCfgId
  )
{
  EFI_STATUS      Status;
  UINT8           Data8 = 0;

  //
  // Read TCA6424 IO Expander register 0 (Input port register) Bit[3, 2, 1, 0] for Strap FW Config ID
  //
  DEBUG ((DEBUG_INFO, "GetFwCfgId++\n"));
  Status = ByteReadI2C (IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, IO_EXPANDER_INPUT_REG_0, 1, &Data8);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Reading trap FW Config ID failed."));
    return Status;
  }

  Data8 &= 0x0F;
  *FwCfgId = Data8;

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
PeiReadPssData (
  UINT8     *Buffer,
  UINT32    Address,
  UINT32    Size
  )
{
  EFI_STATUS                Status = EFI_SUCCESS;
  UINT8                     Offset[2];
  UINT32                    i;

  if (Size == 0) {
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "PeiReadPssData++\n"));

  for (i = 0; i < Size; i++) {
    Offset[0] = (UINT8)(Address >> 8);
    Offset[1] = (UINT8)(Address & 0xFF);

    Status = ByteWriteI2C_Basic (PSS_I2C_BUS_NUM, PSS_I2C_SLAVE_ADDR, 1 ,&Offset[0], TRUE, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "First step error.\n"));
      break;
    }
    Status = ByteWriteI2C_Basic (PSS_I2C_BUS_NUM, PSS_I2C_SLAVE_ADDR, 1, &Offset[1], FALSE, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Second step error.\n"));
      break;
    }

    Status = ByteReadI2C_Basic (PSS_I2C_BUS_NUM, PSS_I2C_SLAVE_ADDR, 1, (UINT8*)(Buffer+i), FALSE, FALSE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Third step error.\n"));
      break;
    }

    i += 1;
    Status = ByteReadI2C_Basic (PSS_I2C_BUS_NUM, PSS_I2C_SLAVE_ADDR, 1, (UINT8*)(Buffer+i), FALSE, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Fourth step error.\n"));
      break;
    }
    Address += 2;
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to Read Data from PSS.\n"));
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}


/**
**/
EFI_STATUS
EFIAPI
PeiPssDetect (
  VOID
  )
{
  EFI_STATUS    Status;
  UINT8         PssData[4];

  //
  // Read the chip's Class ID from the TID bank, it should be 0xE2 (Gen2)
  // Check whether the PSS IC is Monza X-8K
  //
  ZeroMem (PssData, 4);
  Status = PeiReadPssData(&PssData[0], PSS_BASE_ADDR_CLASSID, 4);
  DEBUG ((DEBUG_INFO, "Get PSS Class ID: %2X-%2X-%2X-%2X\n",\
         PssData[0], PssData[1], PssData[2], PssData[3]));

  if (!(EFI_ERROR(Status)) &&
      (PssData[0] == 0xE2) &&
      ((PssData[2] & 0x0F) == 0x01) &&
      (PssData[3] == 0x50)
     ) {
    return EFI_SUCCESS;  //Monza X-8K
  } else {
    return EFI_DEVICE_ERROR;
  }
}


/**

Routine Description:

  Returns the Board ID and FAB ID of the platform from the PSS chip.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  BoardId                   - BoardId ID as determined through the PSS IPN region.
  FabId                     - FabId ID as determined through the PSS IPN region.

Returns:

  EFI_SUCCESS               The function completed successfully.
  EFI_DEVICE_ERROR          PSS fails to respond.

**/
EFI_STATUS
EFIAPI
GetBoardIdFabId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  EFI_STATUS                Status;
  UINT8                     Data8[13];

  DEBUG ((DEBUG_INFO, "GetBoardIdFabId++\n"));
  ZeroMem (Data8, 13);

#if (TABLET_PF_ENABLE == 0)
    Status = GetBoardFabId(Data8);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Error Reading Board Id and Fab Id from EC\n"));
      return Status;
    }

    // BoardId - Bits 2:0
    // FabId - Bits 10:8
    *BoardId = Data8[0] & 0x7;
    *FabId   = Data8[1] & 0x7;

#else
    Status = PeiPssDetect ();
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "PSS chip not found.\n"));
      return Status;
    }

    Status = PeiReadPssData (&Data8[0], PSS_BASE_ADDR_USER + PSS_CHIP_INTEL_PART_NUMBER_OFFSET, 12);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Failed to Read Board ID and FAB ID from PSS.\n"));
      return Status;
    }
    DEBUG ((DEBUG_INFO, "IPN: %a, Data Length: 0x%X\n", Data8, AsciiStrLen (Data8)));

    //
    // Read Board ID and FAB ID from PSS chip that prefix 2 additional characters to IPN string,
    // This design start from FAB-D board. (e.g. RDH28336-201, RD for Broxton RVP FAB-D)
    // The first character can be used for identify different board (RVP, PR, FFD...),
    // and second character can be used for different Board revision (FAB-A, FAB-B...)
    //
    if (AsciiStrLen (Data8) == 12) {
      // Board ID
      switch (Data8[0]) {
      case 'R':
        *BoardId = BOARD_ID_BXT_RVP;
        break;
      case 'P':
        *BoardId = BOARD_ID_BXT_PR;
        break;
      case 'F':
        *BoardId = BOARD_ID_BXT_FFD;
        break;
      default:
        *BoardId = UNKNOWN_BOARD;
        DEBUG ((DEBUG_ERROR, "Unknown BoardId found in IPN.\n"));
        return EFI_UNSUPPORTED;
        break;
      }

      // FAB ID
      switch (Data8[1]) {
      case 'A':
        *FabId = FAB_ID_A;
        break;
      case 'B':
        *FabId = FAB_ID_B;
        break;
      case 'C':
        *FabId = FAB_ID_C;
        break;
      case 'D':
        *FabId = FAB_ID_D;
        break;
      case 'E':
        *FabId = FAB_ID_E;
        break;
      case 'F':
        *FabId = FAB_ID_F;
        break;

      default:
        *FabId = UNKNOWN_FAB;
        DEBUG ((DEBUG_ERROR, "Unknown FabId found in IPN.\n"));
        return EFI_UNSUPPORTED;
        break;
      }
    } else {
      DEBUG ((DEBUG_INFO, "\n -- 12 character IPN not found. --\n\n"));
      *BoardId = BOARD_ID_BXT_RVP;
      *FabId   = FAB_ID_RVP_B_C;
    }

#endif

  return EFI_SUCCESS;
}


//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
EFI_STATUS
EFIAPI
GetEmbeddedBoardIdFabId(
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  //BXTI start
  BXT_CONF_PAD0   padConfg0;
  BXT_CONF_PAD1   padConfg1;
  IN UINT32       CommAndOffset;
  UINT32                        Buffer;
  //BXTI end

  DEBUG ((DEBUG_INFO, "GetEmbeddedBoardIdFabId++\n"));

//[-start-160427-IB07400720-modify]//
  //Board_ID0: PMIC_STDBY
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00F0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
//[-start-160506-IB07400720-modify]//
//  //Set to Pull Up 20K
//  padConfg1.r.Term = 0xC;
  padConfg1.r.Term = 0; // Pull None
//[-end-160506-IB07400720-modify]//
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
  //Board_ID1: PMIC_SDWN_B
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00D0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.Term = 0; // Pull None
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
    
  //Board_ID2: PMIC_RESET_B
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00C8);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.Term = 0; // Pull None
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);

#ifdef FAB_A_BOARD_ID3    
  //Board_ID3: PMIC_PWRGOOD
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00C0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.Term = 0; // Pull None
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);

  *BoardId = (UINT8)(((GpioPadRead(GetCommOffset(NORTHWEST, 0x00F0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) | \
                    (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00D0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 1) | \
                    (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00C8) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 2) |
                    (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00C0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 3) );

#else

    //Board_ID3: GP_CAMERASB10
  CommAndOffset = GetCommOffset(NORTH, 0x01E0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0; // GPIO
  padConfg0.r.GPIORxTxDis = 0x1; // GPI
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.Term = 0; // Pull None
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
    
  *BoardId = (UINT8)(((GpioPadRead(GetCommOffset(NORTHWEST, 0x00F0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) | \
                    (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00D0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 1) | \
                    (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00C8) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 2) |
                    (((GpioPadRead(GetCommOffset(NORTH, 0x01E0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 3) );
#endif

  DEBUG ((DEBUG_INFO,  "BoardId from PMIC strap: %02X\n", *BoardId));

  // Rev E - 7, Rev F - 47, Rev G - 87, Rev H - C7
  Buffer = 0x5EB3;
  IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_READ, &Buffer, 2);
  DEBUG(( DEBUG_INFO, "PMIC PROD_ID %x\n", Buffer & 0xFF ));

  if ((Buffer & 0x000F) != 0x07) {
    DEBUG ((DEBUG_INFO, "PMIC SMBUS Interface\n"));
    //Fab_ID0: PMIC_I2C_SDA
    CommAndOffset = GetCommOffset(NORTHWEST, 0x0108);
    padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
    padConfg0.r.PMode = 0;
    padConfg0.r.GPIORxTxDis = 0x1;
    GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
    padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
    //Set to Pull Up 20K
    padConfg1.r.Term = 0xC;
    GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
    //Fab_ID1: PMIC_I2C_SCL
    CommAndOffset = GetCommOffset(NORTHWEST, 0x0100);
    padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
    padConfg0.r.PMode = 0;
    padConfg0.r.GPIORxTxDis = 0x1;
    GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
    padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
    //Set to Pull Up 20K
    padConfg1.r.Term = 0xC;
    GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
  } else {
    DEBUG ((DEBUG_INFO, "PMIC I2C Interface\n"));

    Buffer = 0x1A5EAD;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
//[-start-190102-IB07401061-modify]//
#ifndef IOTG_SIC_RC_CODE_SUPPORTED
    Buffer = 0x015E1F;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
#endif
//[-end-190102-IB07401061-modify]//
    Buffer = 0x095E2A;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
    Buffer = 0xF35EA5;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
//[-start-190102-IB07401061-modify]//
#ifndef IOTG_SIC_RC_CODE_SUPPORTED
    Buffer = 0x445EC3;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
    Buffer = 0x005ED4;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
    Buffer = 0x3E5EE6;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
#endif
//[-end-190102-IB07401061-modify]//
    Buffer = 0x3C5EE7;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
    Buffer = 0x3D5EE9;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
    Buffer = 0x0D5EEB;
    IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
//[-start-170829-IB07400902-add]//
    // Disable DPU for Leaf Hill only to avoild 0x11 hang VDDQ failure issue.
    if (*BoardId == BOARD_ID_LFH_CRB){
      DEBUG ((DEBUG_INFO, "Disable DPU for Leaf Hill.\n"));
      Buffer = 0x3C5E3B;
      IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
      Buffer = 0x3C5E3C;
      IpcSendCommandEx (IPC_CMD_ID_PMIC, IPC_SUBCMD_ID_PMIC_WRITE, &Buffer, 3);
    }
//[-end-170829-IB07400902-add]//
  }
  //Fab_ID2: PMIC_BCUDISW2
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00D8);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  //Set to Pull Up 20K
  padConfg1.r.Term = 0xC;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
  //Fab_ID3: PMIC_BCUDISCRIT
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00E0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  //Set to Pull Up 20K
  padConfg1.r.Term = 0xC;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);

  *FabId = (UINT8)(((GpioPadRead(GetCommOffset(NORTHWEST, 0x0108) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) | \
                  (((GpioPadRead(GetCommOffset(NORTHWEST, 0x0100) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 1) | \
                  (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00D8) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 2) | \
                  (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00E0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 3));

  DEBUG ((EFI_D_INFO,  "FabId from GPIO strap: %02X\n", *FabId));
//[-end-160427-IB07400720-modify]//

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetIVIBoardIdFabId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  //BXTI start
  BXT_CONF_PAD0   padConfg0;
  BXT_CONF_PAD1   padConfg1;
  IN UINT32       CommAndOffset;
  //BXTI end

  DEBUG((DEBUG_INFO, "GetIVIBoardIdFabId++\n"));

  //Board_ID0: GPIO_62
  CommAndOffset = GetCommOffset(NORTH, 0x0190);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.IOSTerm = 0x3; //Enable Pullup
  padConfg1.r.Term = 0xC;    //20k wpu
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
  //Board_ID1: GPIO_63
  CommAndOffset = GetCommOffset(NORTH, 0x0198);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.IOSTerm = 0x3; //Enable Pullup
  padConfg1.r.Term = 0xC;    //20k wpu
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
  //Board_ID2: GPIO_64
  CommAndOffset = GetCommOffset(NORTH, 0x01A0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.IOSTerm = 0x3; //Enable Pullup
  padConfg1.r.Term = 0xC;    //20k wpu
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
  //Board_ID3: GPIO_65
  CommAndOffset = GetCommOffset(NORTH, 0x01A8);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  padConfg1.r.IOSTerm = 0x3; //Enable Pullup
  padConfg1.r.Term = 0xC;    //20k wpu
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);

  *BoardId = (UINT8)(((GpioPadRead(GetCommOffset(NORTH, 0x0190) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) | \
                    (((GpioPadRead(GetCommOffset(NORTH, 0x0198) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 1) | \
                    (((GpioPadRead(GetCommOffset(NORTH, 0x01A0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 2) | \
                    (((GpioPadRead(GetCommOffset(NORTH, 0x01A8) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 3));

  return EFI_SUCCESS;
}
//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//


/**

Routine Description:

  Returns the Dock ID of the platform from the TCA6424 GPIO expander IC.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  DockId                    - Dock ID as determined through the TCA6424.

Returns:

  EFI_SUCCESS               The function completed successfully.
  EFI_DEVICE_ERROR          TCA6424 fails to respond.

**/
EFI_STATUS
EFIAPI
GetDockId (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *DockId
  )
{
  EFI_STATUS      Status;
  UINT8           Port1Data = 0;
  UINT8           Port2Data = 0;

  //
  // Read TCA6424 IO Expander register 2 (Input port register) Bit[2] and register 1 Bit[5, 4, 3] for Dock ID
  //
  DEBUG ((DEBUG_INFO, "GetDockId++\n"));

  Status = ByteReadI2C (IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, IO_EXPANDER_INPUT_REG_1, 1, &Port1Data);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Read GPIO expander input reg 1 failed.\n"));
    return Status;
  }
  Port1Data &= (BIT5|BIT4|BIT3);

  Status = ByteReadI2C (IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, IO_EXPANDER_INPUT_REG_2, 1, &Port2Data);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Read GPIO expander input reg 2 failed.\n"));
    return Status;
  }
  Port2Data &= BIT2;

  *DockId = (UINT8)((Port2Data << 1 ) | (Port1Data >> 3));

  return EFI_SUCCESS;
}


/*++

Routine Description:

  Returns the OS Selection from the PSS chip.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  OsSelPss                  - OS Selection as determined through the PSS OS region.

Returns:

  EFI_SUCCESS               The function completed successfully.
  EFI_DEVICE_ERROR          PSS fails to respond.

--*/
EFI_STATUS
EFIAPI
GetOsSelPss (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *OsSelPss
  )
{
  EFI_STATUS      Status;
  UINT8           Data8[4];

  DEBUG ((DEBUG_INFO, "GetOsSelPss++\n"));
  ZeroMem (Data8, sizeof(Data8));
  Status = PeiReadPssData (&Data8[0], PSS_BASE_ADDR_USER + PSS_CHIP_OS_OFFSET, 3);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to Read OS field from PSS.\n"));
    return Status;
  }
  DEBUG ((DEBUG_INFO, "PSS OS: %a\n", Data8));

  if (!CompareMem (Data8, "WOS", 3)) {
    *OsSelPss = PSS_WOS;
  } else if (!CompareMem(Data8, "AOS", 3)) {
    *OsSelPss = PSS_AOS;
  } else {
    DEBUG ((DEBUG_ERROR, "Invalid OS ID read from PSS: 0x%X\n", Data8));
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/*++

Routine Description:

  Returns the BOM ID from the PSS chip.

Arguments:

  PeiServices               - General purpose services available to every PEIM.
  BomIdPss                  - BOM ID as determined through the PSS BKC region.

Returns:

  EFI_SUCCESS               The function completed successfully.
  EFI_DEVICE_ERROR          PSS fails to respond.

--*/
EFI_STATUS
EFIAPI
GetBomIdPss (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  OUT UINT8                     *BomIdPss
  )
{
  EFI_STATUS      Status;
  UINT8           Data8[5];

  DEBUG ((DEBUG_INFO, "GetBomIdPss++\n"));
  ZeroMem (Data8, sizeof(Data8));
  Status = PeiReadPssData (&Data8[0], PSS_BASE_ADDR_USER + PSS_CHIP_BKC_VERSION_OFFSET, 4);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to Read BOM ID from PSS.\n"));
    return Status;
  }
  DEBUG ((DEBUG_INFO, "PSS BOM ID: %a\n", Data8));

  if (!CompareMem (Data8, "BOM0", 4)) {
    *BomIdPss = PSS_BOM_0;
  } else if (!CompareMem (Data8, "BOM1", 4)) {
    *BomIdPss = PSS_BOM_1;
  } else if (!CompareMem (Data8, "BOM2", 4)) {
    *BomIdPss = PSS_BOM_2;
  } else if (!CompareMem (Data8, "BOM3", 4)) {
    *BomIdPss = PSS_BOM_3;
  } else if (!CompareMem (Data8, "BOM4", 4)) {
    *BomIdPss = PSS_BOM_4;
  } else {
    DEBUG ((DEBUG_ERROR, "Invalid BOM ID read from PSS: 0x%X\n", Data8));
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}
//[-end-160413-IB03090426-modify]//
