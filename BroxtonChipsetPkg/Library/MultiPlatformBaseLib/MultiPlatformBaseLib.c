/** @file
  MultiPlatformBaseLib

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
  
#include <MultiPlatformBaseLib.h>

//[-start-161022-IB07400803-remove]//
//BOOLEAN 
//IsIOTGBoardIds (
//  IN UINT8 BoardId
//) {
//  if ((BoardId == BOARD_ID_OXH_CRB) || 
//      (BoardId == BOARD_ID_LFH_CRB) || 
//      (BoardId == BOARD_ID_JNH_CRB) || 
//      (BoardId == BOARD_ID_BFH_IVI) || 
//      (BoardId == BOARD_ID_MRB_IVI)) {    
//    return TRUE;
//  } else {
//    return FALSE;
//  }
//}
//[-end-161022-IB07400803-remove]//

//[-start-160913-IB07400784-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
//[-end-160913-IB07400784-modify]//

//[-start-160913-IB07400784-add]//
#include <Library/EcMiscLib.h>
#include <Library/GpioLib.h>
#include <Library/BaseMemoryLib.h>

#define GetCommOffset(Community, Offset) (((UINT32)Community) << 16) + GPIO_PADBAR + Offset

EFI_STATUS
EFIAPI
MpGetBoardIdFabId (
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  EFI_STATUS                Status;
  UINT8                     Data8[13];

  DEBUG ((DEBUG_INFO, "GetBoardIdFabId++\n"));
  ZeroMem (Data8, 13);

  Status = GetBoardFabId(Data8);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error Reading Board Id and Fab Id from EC\n"));
    return Status;
  }

  // BoardId - Bits 2:0
  // FabId - Bits 10:8
  *BoardId = Data8[0] & 0x7;
  *FabId   = Data8[1] & 0x7;

  return EFI_SUCCESS;
}

EFI_STATUS
MpReadBxtPlatformIds (
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  EFI_STATUS  Status;

  // Get Board ID and FAB ID
  Status = MpGetBoardIdFabId(BoardId, FabId);

  if (EFI_ERROR(Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MpGetEmbeddedBoardIdFabId(
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  BXT_CONF_PAD0   padConfg0;
  BXT_CONF_PAD1   padConfg1;
  UINT32          CommAndOffset;

  DEBUG ((DEBUG_INFO, "MpGetEmbeddedBoardIdFabId++\n"));

  //Board_ID0: PMIC_STDBY
  CommAndOffset = GetCommOffset(NORTHWEST, 0x00F0);
  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
  padConfg0.r.PMode = 0;
  padConfg0.r.GPIORxTxDis = 0x1;
  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
  
  padConfg1.r.Term = 0; // Pull None
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

//[-start-170829-IB07400902-remove]//
  //
  // Skip Fab ID detction in this Libary to keep GPIO status
  //
  *FabId = 0;
//  //Fab_ID0: PMIC_I2C_SDA
//  CommAndOffset = GetCommOffset(NORTHWEST, 0x0108);
//  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
//  padConfg0.r.PMode = 0;
//  padConfg0.r.GPIORxTxDis = 0x1;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
//  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
//  //Set to Pull Up 20K
//  padConfg1.r.Term = 0xC;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
//  //Fab_ID1: PMIC_I2C_SCL
//  CommAndOffset = GetCommOffset(NORTHWEST, 0x0100);
//  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
//  padConfg0.r.PMode = 0;
//  padConfg0.r.GPIORxTxDis = 0x1;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
//  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
//  //Set to Pull Up 20K
//  padConfg1.r.Term = 0xC;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
//  //Fab_ID2: PMIC_BCUDISW2
//  CommAndOffset = GetCommOffset(NORTHWEST, 0x00D8);
//  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
//  padConfg0.r.PMode = 0;
//  padConfg0.r.GPIORxTxDis = 0x1;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
//  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
//  //Set to Pull Up 20K
//  padConfg1.r.Term = 0xC;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
//  //Fab_ID3: PMIC_BCUDISCRIT
//  CommAndOffset = GetCommOffset(NORTHWEST, 0x00E0);
//  padConfg0.padCnf0 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET);
//  padConfg0.r.PMode = 0;
//  padConfg0.r.GPIORxTxDis = 0x1;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF0_OFFSET, padConfg0.padCnf0);
//  padConfg1.padCnf1 = GpioPadRead(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET);
//  //Set to Pull Up 20K
//  padConfg1.r.Term = 0xC;
//  GpioPadWrite(CommAndOffset + BXT_GPIO_PAD_CONF1_OFFSET, padConfg1.padCnf1);
//
//  *FabId = (UINT8)(((GpioPadRead(GetCommOffset(NORTHWEST, 0x0108) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) | \
//                  (((GpioPadRead(GetCommOffset(NORTHWEST, 0x0100) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 1) | \
//                  (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00D8) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 2) | \
//                  (((GpioPadRead(GetCommOffset(NORTHWEST, 0x00E0) + BXT_GPIO_PAD_CONF0_OFFSET) & BIT1) >> 1) << 3));
//
//  DEBUG ((EFI_D_INFO,  "FabId from GPIO strap: %02X\n", *FabId));
//[-end-170829-IB07400902-remove]//
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
MpGetIVIBoardIdFabId (
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  BXT_CONF_PAD0   padConfg0;
  BXT_CONF_PAD1   padConfg1;
  UINT32          CommAndOffset;

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

EFI_STATUS
MpReadBxtIPlatformIds (
  OUT UINT8                     *BoardId,
  OUT UINT8                     *FabId
  )
{
  EFI_STATUS                  Status;

  DEBUG((DEBUG_INFO, "Port(0x62) = %02X\n", IoRead8 (0x62)));

  if (IoRead8(0x62) != 0xFF) {
    //
    // EC exist, get platform Ids from EC
    //
//[-start-161206-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
//[-end-170111-IB07400832-modify]//
    Status = MpReadBxtPlatformIds(BoardId, FabId);
#else
    //
    // EC/KSC support is disabled, 
    // We can not get the boardId, using builded BoardId.
    //
    *BoardId = PcdGet8 (PcdBuildPlatformType);
    *FabId = 0;
    Status = EFI_SUCCESS;
#endif
//[-end-161206-IB07400822-modify]//
  } else {
    Status = MpGetEmbeddedBoardIdFabId (BoardId, FabId);

    if (*BoardId == BOARD_ID_OXH_CRB) {
      DEBUG ((DEBUG_INFO, "I'm Oxbow Hill \n\n"));
    } else if (*BoardId == BOARD_ID_LFH_CRB) {
      DEBUG ((DEBUG_INFO, "I'm Leaf Hill \n\n"));
    } else if (*BoardId == BOARD_ID_JNH_CRB) {
      DEBUG ((DEBUG_INFO, "I'm Juniper Hill \n\n"));
    } else {

      Status = MpGetIVIBoardIdFabId (BoardId, FabId);

      if (*BoardId == BOARD_ID_BFH_IVI) {
        DEBUG ((DEBUG_INFO, "I'm Gordon Ridge BMP Interposer \n\n"));
      } else if (*BoardId == BOARD_ID_MRB_IVI) {
        DEBUG ((DEBUG_INFO, "I'm Gordon Ridge BMP MRB \n\n"));
      } else {
        //to be added
        DEBUG ((DEBUG_INFO, "BoardId form GPIO strap: %02X\n", *BoardId));
      }
    }
  }

  DEBUG ((DEBUG_INFO, "BoardId:  [0x%08x]\n", *BoardId));
  DEBUG ((DEBUG_INFO, "FabId:    [0x%08x]\n", *FabId));

  return Status;
}
//[-end-160913-IB07400784-add]//

VOID
MultiPlatformDetectPlatformType (
  VOID
  )
{
  UINT8                       PlatformType = 0xFF;
  UINT8                       BoardId = 0;
  UINT8                       FabId = 0;
//[-start-160913-IB07400784-add]//
  EFI_STATUS                  Status;

  Status = MpReadBxtIPlatformIds (&BoardId, &FabId);
  if (!EFI_ERROR (Status)) {
    PlatformType = BoardId;
  }
//[-end-160913-IB07400784-add]//
  //
  // Update Platform Type to PCD
  //
  PcdSet8 (PcdDynamicBoardPlatformType, PlatformType);
}

EFI_STATUS
MultiPlatformSetupConfig(
  IN OUT  VOID  *SetupConfig 
  )
{
  UINT8                   BoardId;
  CHIPSET_CONFIGURATION   *ChipsetSetupConfig;

  ChipsetSetupConfig = (CHIPSET_CONFIGURATION*)SetupConfig; 

  BoardId = PcdGet8 (PcdDynamicBoardPlatformType);

  if (BoardId == 0xFF) {
//[-start-160913-IB07400784-modify]//
#if defined (APOLLOLAKE_CRB)
    MultiPlatformDetectPlatformType ();
#elif defined (USE_CRB_HW_CONFIG)
    PcdSet8 (PcdDynamicBoardPlatformType, PcdGet8 (PcdBuildPlatformType));
#endif
    BoardId = PcdGet8 (PcdDynamicBoardPlatformType);
//[-end-160913-IB07400784-modify]//
  }

  switch (BoardId) {
    
  case (UINT8)BOARD_ID_APL_RVP_1A: // APL RVP 1A
  case (UINT8)BOARD_ID_APL_RVP_2A: // APL RVP 2A
//[-start-160914-IB07400784-add]//
    // SouthClusterConfig.vfi
    ChipsetSetupConfig->TcoLock = 1;
    // RTD3.vfi
    ChipsetSetupConfig->Rtd3Support = 1;
    // SystemComponent.vfi
    ChipsetSetupConfig->PmicEnable = 1;
    ChipsetSetupConfig->ResetSelect = 0x6;
//[-start-161022-IB07400802-add]//
    // Thermal.vfi
    ChipsetSetupConfig->CriticalThermalTripPoint = 100;
    ChipsetSetupConfig->PassiveThermalTripPoint = 95;
//[-end-161022-IB07400802-add]//
    break;
//[-end-160914-IB07400784-add]//
  case (UINT8)BOARD_ID_APL_RVP_1C_LITE: // APL RVP 1C Lite
    break;
    
//[-start-160913-IB07400784-modify]//
  case (UINT8)BOARD_ID_MNH_RVP: // Mineral Hill RVP
    break;
  case (UINT8)BOARD_ID_OXH_CRB: // Oxbow Hill
  case (UINT8)BOARD_ID_LFH_CRB: // Leaf Hill
  case (UINT8)BOARD_ID_JNH_CRB: // Juniper Hill
//[-start-160920-IB05400707-add]//
 #ifdef VBT_HOOK_SUPPORT
//[-end-160920-IB05400707-add]//
    //
    // LFP
    //
    ChipsetSetupConfig->VbtLfpType            = SCU_VBT_LFP_EDP;
    ChipsetSetupConfig->VbtLfpPort            = SCU_VBT_LFP_EDP_PORTA;
    ChipsetSetupConfig->VbtLfpAux             = SCU_VBT_DP_PORTA;
    ChipsetSetupConfig->VbtLfpPanel           = 3;
    ChipsetSetupConfig->VbtLfpEdid            = SCU_VBT_ENABLE;
//[-start-161013-IB07400797-modify]//
    ChipsetSetupConfig->VbtLfpHpdInversion    = SCU_VBT_ENABLE;
//[-end-161013-IB07400797-modify]//
    ChipsetSetupConfig->VbtLfpDdiLaneReversal = SCU_VBT_DISABLE;
    //
    // EFP1
    //
    ChipsetSetupConfig->VbtEfpType[0]             = SCU_VBT_HDMI_DVI;
    ChipsetSetupConfig->VbtEfpPort[0]             = SCU_VBT_HDMI_PORTB;
    ChipsetSetupConfig->VbtEfpDdc[0]              = SCU_VBT_HDMI_PORTB;
    ChipsetSetupConfig->VbtEfpAux[0]              = SCU_VBT_DP_PORTB;
//    ChipsetSetupConfig->VbtEfpEdid[0]             = SCU_VBT_DEFAULT;
    ChipsetSetupConfig->VbtEfpHdmiLevelShifter[0] = SCU_VBT_400MV_0P0_DB;
    ChipsetSetupConfig->VbtEfpOnboardLspcon[0]    = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpHpdInversion[0]     = SCU_VBT_ENABLE;
    ChipsetSetupConfig->VbtEfpDdiLaneReversal[0]  = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpUsbTypeC[0]         = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpDockablePort[0]     = SCU_VBT_DISABLE;
    //
    // EFP2
    //
    ChipsetSetupConfig->VbtEfpType[1]             = SCU_VBT_DP_HDMI_DVI;
    ChipsetSetupConfig->VbtEfpPort[1]             = SCU_VBT_DP_PORTC;
    ChipsetSetupConfig->VbtEfpDdc[1]              = SCU_VBT_HDMI_PORTC;
    ChipsetSetupConfig->VbtEfpAux[1]              = SCU_VBT_DP_PORTC;
//    ChipsetSetupConfig->VbtEfpEdid[1]             = SCU_VBT_DEFAULT;
    ChipsetSetupConfig->VbtEfpHdmiLevelShifter[1] = SCU_VBT_800MV_3P5_DB;
    ChipsetSetupConfig->VbtEfpOnboardLspcon[1]    = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpHpdInversion[1]     = SCU_VBT_ENABLE;
    ChipsetSetupConfig->VbtEfpDdiLaneReversal[1]  = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpUsbTypeC[1]         = SCU_VBT_ENABLE;
    ChipsetSetupConfig->VbtEfpDockablePort[1]     = SCU_VBT_DISABLE;
    //
    // EFP3
    //
    ChipsetSetupConfig->VbtEfpType[2]             = SCU_VBT_NO_DEVICE;
    ChipsetSetupConfig->VbtEfpPort[2]             = SCU_VBT_NO_DEVICE;
    ChipsetSetupConfig->VbtEfpDdc[2]              = SCU_VBT_NO_DEVICE;
    ChipsetSetupConfig->VbtEfpAux[2]              = SCU_VBT_NO_DEVICE;
//    ChipsetSetupConfig->VbtEfpEdid[2]             = SCU_VBT_DEFAULT;
    ChipsetSetupConfig->VbtEfpHdmiLevelShifter[2] = SCU_VBT_400MV_0P0_DB;
    ChipsetSetupConfig->VbtEfpOnboardLspcon[2]    = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpHpdInversion[2]     = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpDdiLaneReversal[2]  = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpUsbTypeC[2]         = SCU_VBT_DISABLE;
    ChipsetSetupConfig->VbtEfpDockablePort[2]     = SCU_VBT_DISABLE;
//[-start-160920-IB05400707-add]//
#endif
//[-end-160920-IB05400707-add]//

    // SouthClusterConfig.vfi
    ChipsetSetupConfig->TcoLock = 0;
    // RTD3.vfi
    ChipsetSetupConfig->Rtd3Support = 0;
    // SystemComponent.vfi
    ChipsetSetupConfig->PmicEnable = 0;
    if (PcdGetBool (PcdSecureFlashSupported)) {
      ChipsetSetupConfig->ResetSelect = 0x6; // Warm reboot for Secure flash via Memory.
    } else {
      ChipsetSetupConfig->ResetSelect = 0xE;
    }
//[-start-161022-IB07400802-add]//
    // Thermal.vfi
    ChipsetSetupConfig->CriticalThermalTripPoint = 125;
    ChipsetSetupConfig->PassiveThermalTripPoint = 111;
//[-end-161022-IB07400802-add]//
    break;
//[-end-160913-IB07400784-modify]//
    
  case (UINT8)0xFF:
    //
    // Pcd do not be update, unknown platform type
    //
    DEBUG ((EFI_D_ERROR, "MultiPlatformSetupConfig: Unknow Platform!!"));
    break;
    
  default:
    break;
  }
  
//[-start-170222-IB07400843-add]//
#ifdef W8374LF2_SIO_SUPPORT
  //
  // Disable LPC CLKRUN# for W8374LF LDC card
  //
  ChipsetSetupConfig->LpcClockRun = FALSE;
#endif
//[-end-170222-IB07400843-add]//

  return  EFI_SUCCESS;
}

#else

EFI_STATUS
MultiPlatformSetupConfig(
  IN OUT  VOID  *SetupConfig 
  )
{
  return EFI_UNSUPPORTED;
}

#endif  

//[-start-161022-IB07400803-add]//
BOOLEAN 
IsIOTGBoardIds (
  VOID
  ) 
{
  UINT8 BoardId;

  BoardId = PcdGet8 (PcdDynamicBoardPlatformType);

  if (BoardId == 0xFF) {
#if defined (APOLLOLAKE_CRB)
    MultiPlatformDetectPlatformType ();
#else
    PcdSet8 (PcdDynamicBoardPlatformType, PcdGet8 (PcdBuildPlatformType));
#endif
    BoardId = PcdGet8 (PcdDynamicBoardPlatformType);
  }
  
  DEBUG ((EFI_D_ERROR, "IsIOTGBoardIds: BoardId = %x!!\n", BoardId));

  if ((BoardId == BOARD_ID_OXH_CRB) || 
      (BoardId == BOARD_ID_LFH_CRB) || 
      (BoardId == BOARD_ID_JNH_CRB) || 
      (BoardId == BOARD_ID_BFH_IVI) || 
      (BoardId == BOARD_ID_MRB_IVI)) {    
    return TRUE;
  } else {
    return FALSE;
  }
}
//[-end-161022-IB07400803-add]//
//[-start-161107-IB07400810-add]//
UINT8 
MultiPlatformGetBoardIds (
  VOID
  ) 
{
  UINT8 BoardId;

  BoardId = PcdGet8 (PcdDynamicBoardPlatformType);

  if (BoardId == 0xFF) {
#if defined (APOLLOLAKE_CRB)
    MultiPlatformDetectPlatformType ();
#else
    PcdSet8 (PcdDynamicBoardPlatformType, PcdGet8 (PcdBuildPlatformType));
#endif
    BoardId = PcdGet8 (PcdDynamicBoardPlatformType);
  }
  DEBUG ((EFI_D_ERROR, "MultiPlatformGetBoardIds: BoardId = %x!!\n", BoardId));
  
  return BoardId;
}
//[-end-161107-IB07400810-add]//