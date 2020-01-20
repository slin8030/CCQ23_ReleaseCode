/** @file
  Gpio setting for multiplatform..


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

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/
//[-start-160513-IB03090427-modify]//
#include <BoardGpios.h>
#include <Library/GpioLib.h>
#include <Library/SteppingLib.h>
//[-start-160803-IB07220122-remove]//
// #include <Library/PlatformConfigDataLib.h>
//[-end-160803-IB07220122-remove]//
//[-start-170512-IB07400866-add]//
#include <Library/GpioCfgLib.h>
//[-end-170512-IB07400866-add]//

//[-start-151216-IB11270137-add]//
#include <Library/PeiOemSvcChipsetLib.h>
//[-end-151216-IB11270137-add]//
/**
  Returns the Correct GPIO table for Mobile/Desktop respectively.
  Before call it, make sure PlatformInfoHob->BoardId&PlatformFlavor is get correctly.

  @param[in] PeiServices      General purpose services available to every PEIM.
  @param[in] PlatformInfoHob  PlatformInfoHob pointer with PlatformFlavor specified.

  @retval EFI_SUCCESS         The function completed successfully.
  @retval EFI_DEVICE_ERROR    KSC fails to respond.
**/
EFI_STATUS
MultiPlatformGpioTableInit (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN EFI_PLATFORM_INFO_HOB      *PlatformInfoHob
  )
{
  DEBUG ((DEBUG_INFO, "MultiPlatformGpioTableInit()...\n"));
  DEBUG ((DEBUG_INFO, "PlatformInfoHob->BoardId: 0x%02X\n", PlatformInfoHob->BoardId));
//[-start-170517-IB07400866-add]//
#ifdef H2OIDE_GPIO_CONFIG_SUPPORT
  // H2OIDE GPIO Config Support, Get GPIO data form GpioCfgLib
#else
//[-end-170517-IB07400866-add]//
//[-start-160427-IB07400720-add]//
#ifndef APOLLOLAKE_CRB
  //
  // OEM GPIO Setting
  //
  PlatformInfoHob->PlatformGpioSetting_SW = &mBXT_GpioInitData_SW_OEM[0];
  PlatformInfoHob->PlatformGpioSetting_W = &mBXT_GpioInitData_W_OEM[0];
  PlatformInfoHob->PlatformGpioSetting_NW = &mBXT_GpioInitData_NW_OEM[0];
  PlatformInfoHob->PlatformGpioSetting_N = &mBXT_GpioInitData_N_OEM[0];

#else
//[-end-160427-IB07400720-add]//

  //
  // Select/modify the GPIO initialization data based on the Board ID.
  //
  switch (PlatformInfoHob->BoardId) {
  //
  // TODO: To sync with MCG/PCCG/ISG platform team later on platform specific GPIO init.
  // TODO: I have no idea about board IDs yet
  //
   case BOARD_ID_BXT_RVVP:
#if RVVP_ENABLE == 1
      //
      // For the RVVP all gpio setting
      //
      PlatformInfoHob->PlatformGpioSetting_SW = &mBXT_RVVP_GpioInitData_SW[0];
      PlatformInfoHob->PlatformGpioSetting_W = &mBXT_RVVP_GpioInitData_W[0];
      PlatformInfoHob->PlatformGpioSetting_NW = &mBXT_RVVP_GpioInitData_NW[0];
      PlatformInfoHob->PlatformGpioSetting_N = &mBXT_RVVP_GpioInitData_N[0];
      break;
#endif
//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
    case BOARD_ID_BFH_IVI:
      PlatformInfoHob->PlatformGpioSetting_SW = &mBXT_GpioInitData_SW_BFH_IVI[0];
      PlatformInfoHob->PlatformGpioSetting_W = &mBXT_GpioInitData_W_BFH_IVI[0];
      PlatformInfoHob->PlatformGpioSetting_NW = &mBXT_GpioInitData_NW_BFH_IVI[0];
      PlatformInfoHob->PlatformGpioSetting_N = &mBXT_GpioInitData_N_BFH_IVI[0];
      break;
    case BOARD_ID_MRB_IVI:
      PlatformInfoHob->PlatformGpioSetting_SW = &mBXT_GpioInitData_SW_MRB_IVI[0];
      PlatformInfoHob->PlatformGpioSetting_W = &mBXT_GpioInitData_W_MRB_IVI[0];
      PlatformInfoHob->PlatformGpioSetting_NW = &mBXT_GpioInitData_NW_MRB_IVI[0];
      PlatformInfoHob->PlatformGpioSetting_N = &mBXT_GpioInitData_N_MRB_IVI[0];
      break;
    case BOARD_ID_LFH_CRB:
    case BOARD_ID_OXH_CRB:
    case BOARD_ID_JNH_CRB:
//[-start-161108-IB07400810-modify]//
      PlatformInfoHob->PlatformGpioSetting_SW = &mBXT_GpioInitData_SW_LH[0];
      PlatformInfoHob->PlatformGpioSetting_W = &mBXT_GpioInitData_W_LH[0];
      PlatformInfoHob->PlatformGpioSetting_NW = &mBXT_GpioInitData_NW_LH[0];
      PlatformInfoHob->PlatformGpioSetting_N = &mBXT_GpioInitData_N_LH[0];
//[-end-161108-IB07400810-modify]//
      break;
//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//

    case BOARD_ID_APL_ERBP:
    case BOARD_ID_APL_RVP_1A:
    case BOARD_ID_APL_RVP_1C_LITE:
    case BOARD_ID_MNH_RVP:
    default:
      // GPIO PAD setting for MCG tablet
      PlatformInfoHob->PlatformGpioSetting_SW = &mBXT_GpioInitData_SW[0];
      PlatformInfoHob->PlatformGpioSetting_W = &mBXT_GpioInitData_W[0];
      PlatformInfoHob->PlatformGpioSetting_NW = &mBXT_GpioInitData_NW[0];
      PlatformInfoHob->PlatformGpioSetting_N = &mBXT_GpioInitData_N[0];

      // Do nothing for other RVP boards.
      break;
  }
  
//[-start-160427-IB07400720-add]//
#endif  
//[-end-160427-IB07400720-add]//
//[-start-170517-IB07400866-add]//
#endif
//[-end-170517-IB07400866-add]//
  return EFI_SUCCESS;
}

//[-start-161109-IB07400810-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// JTAG GPIO Lock
//
#define N_TCK                 (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x01F0     //TCK
#define N_TRST_B              (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x01F8     //TRST_B
#define N_TMS                 (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0200     //TMS
#define N_TDI                 (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0208     //TDI
#define N_TDO                 (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0230     //TDO

#define NW_PMIC_THERMTRIP_B   (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x00E8    //PMIC_THERMTRIP_B
#define NW_PROCHOT_B          (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x00F8    //PROCHOT_B

//
// Touch
//
#define NW_GPIO_118           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0238    //GP_SSP_2_CLK
#define NW_GPIO_119           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0240    //GP_SSP_2_FS0
#define NW_GPIO_120           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0248    //GP_SSP_2_FS1
#define NW_GPIO_121           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0250    //GP_SSP_2_FS2
#define NW_GPIO_122           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0258    //GP_SSP_2_RXD
#define NW_GPIO_123           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0260    //GP_SSP_2_TXD

//
// SPI
//
#define NW_GPIO_97            (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01A8    //FST_SPI_CS0_B
#define NW_GPIO_98            (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01B0    //FST_SPI_CS1_B
#define NW_GPIO_99            (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01B8    //FST_SPI_MOSI_IO0
#define NW_GPIO_100           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01C0    //FST_SPI_MISO_IO1
#define NW_GPIO_101           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01C8    //FST_SPI_IO2
#define NW_GPIO_102           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01D0    //FST_SPI_IO3
#define NW_GPIO_103           (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01D8    //FST_SPI_CLK
#define NW_FST_SPI_CLK_FB     (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x01E0    //FST_SPI_CLK_FB

//
// Smbus
//
#define SW_SMB_ALERTB         (((UINT32)GPIO_MMIO_OFFSET_SW)<<16)+GPIO_PADBAR+0x00F8    //SMB_ALERTB
#define SW_SMB_CLK            (((UINT32)GPIO_MMIO_OFFSET_SW)<<16)+GPIO_PADBAR+0x0100    //SMB_CLK
#define SW_SMB_DATA           (((UINT32)GPIO_MMIO_OFFSET_SW)<<16)+GPIO_PADBAR+0x0108    //SMB_DATA
#endif
//[-end-161109-IB07400810-add]//

//[-start-160801-IB03090430-modify]//
/**
  Set GPIO Lock for security
**/
VOID
SetGpioPadCfgLock (
  VOID
  )
{
  UINT32 Data32;

  Data32 = 0;

  // JTAG
  GpioLockPadCfg (N_TCK);
  GpioLockPadCfg (N_TRST_B);
  GpioLockPadCfg (N_TMS);
  GpioLockPadCfg (N_TDI);
  GpioLockPadCfg (N_TDO);
  // Power
  GpioLockPadCfg (NW_PMIC_THERMTRIP_B);
  GpioLockPadCfg (NW_PROCHOT_B);

  // Touch
  GpioLockPadCfg (NW_GPIO_118);
  GpioLockPadCfg (NW_GPIO_119);
  GpioLockPadCfg (NW_GPIO_120);
  GpioLockPadCfg (NW_GPIO_121);
  GpioLockPadCfg (NW_GPIO_122);
  GpioLockPadCfg (NW_GPIO_123);

  // SPI
  GpioLockPadCfg (NW_GPIO_97);
  GpioLockPadCfg (NW_GPIO_98);
  GpioLockPadCfg (NW_GPIO_99);
  GpioLockPadCfg (NW_GPIO_100);
  GpioLockPadCfg (NW_GPIO_101);
  GpioLockPadCfg (NW_GPIO_102);
  GpioLockPadCfg (NW_GPIO_103);
  GpioLockPadCfg (NW_FST_SPI_CLK_FB);

  // SMBus
  // Set SMBus GPIO PAD_CFG.PADRSTCFG to Powergood
  Data32 = GpioPadRead (SW_SMB_ALERTB);
  Data32 &= ~(BIT31 | BIT30);
  GpioPadWrite (SW_SMB_ALERTB, Data32);

  Data32 = GpioPadRead (SW_SMB_CLK);
  Data32 &= ~(BIT31 | BIT30);
  GpioPadWrite (SW_SMB_CLK, Data32);

  Data32 = GpioPadRead (SW_SMB_DATA);
  Data32 &= ~(BIT31 | BIT30);
  GpioPadWrite (SW_SMB_DATA, Data32);

  GpioLockPadCfg (SW_SMB_ALERTB);
  GpioLockPadCfg (SW_SMB_CLK);
  GpioLockPadCfg (SW_SMB_DATA);
}
//[-end-160801-IB03090430-modify]//

/**
  Returns the Correct GPIO table for Mobile/Desktop respectively.
  Before call it, make sure PlatformInfoHob->BoardId&PlatformFlavor is get correctly.

  @param[in] PeiServices       General purpose services available to every PEIM.
  @param[in] PlatformInfoHob   PlatformInfoHob pointer with PlatformFlavor specified.

  @retval EFI_SUCCESS          The function completed successfully.
  @retval EFI_DEVICE_ERROR     KSC fails to respond.
**/
EFI_STATUS
MultiPlatformGpioProgram (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN EFI_PLATFORM_INFO_HOB      *PlatformInfoHob
  )
{
   UINTN                             VariableSize;
   EFI_STATUS                        Status;
//[-start-160803-IB07220122-add]//
   EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariableServices;
//[-end-160803-IB07220122-add]//
   CHIPSET_CONFIGURATION             SystemConfiguration;
//[-start-170517-IB07400866-add]//
#ifdef H2OIDE_GPIO_CONFIG_SUPPORT
  BXT_GPIO_PAD_INIT*                 GpioInitData;
  UINT32                             GpioInitDataCount;
#endif
//[-end-170517-IB07400866-add]//

  VariableSize = PcdGet32 (PcdSetupConfigSize);
//[-start-160806-IB07400769-modify]//
  ASSERT (PcdGet32 (PcdSetupConfigSize) == sizeof (CHIPSET_CONFIGURATION));
//[-end-160806-IB07400769-modify]//
  ZeroMem (&SystemConfiguration, VariableSize);

//[-start-160803-IB07220122-add]//
  Status = (*PeiServices)->LocatePpi (
                              PeiServices,
                              &gEfiPeiReadOnlyVariable2PpiGuid,
                              0,
                              NULL,
                              (VOID **) &VariableServices
                              );
//[-end-160803-IB07220122-add]//
  //
  // SC_PO: Use PlatformConfigDataLib instead of GetVariable.
  //
//[-start-160803-IB07220122-modify]//
//   Status = GetSystemConfigData(&SystemConfiguration, &VariableSize);
  Status = VariableServices->GetVariable (
                               VariableServices,
                               SETUP_VARIABLE_NAME,
                               &gSystemConfigurationGuid,
                               NULL,
                               &VariableSize,
                               &SystemConfiguration
                               );
//[-end-160803-IB07220122-modify]//

  DEBUG ((DEBUG_INFO, "MultiPlatformGpioProgram()...\n"));

//[-start-170517-IB07400866-add]//
#ifdef H2OIDE_GPIO_CONFIG_SUPPORT
  //
  // H2OIDE GPIO Config Support, Get GPIO data from GpioCfgLib
  //
  DEBUG ((DEBUG_INFO, "H2OIDE GPIO Config Support, get GPIO data form GpioCfgLib\n"));

//[-start-170518-IB07400867-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)

  DEBUG ((DEBUG_INFO, "Board ID: 0x%X\n", PlatformInfoHob->BoardId));

  switch (PlatformInfoHob->BoardId) {
  case BOARD_ID_LFH_CRB:
  case BOARD_ID_OXH_CRB:
  case BOARD_ID_JNH_CRB:
    Status = GetGpioCfg(BOARD_ID_OXH_CRB, &GpioInitData, &GpioInitDataCount);
    break;
  case BOARD_ID_APL_RVP_1A:
  case BOARD_ID_APL_RVP_2A:
    default:
    Status = GetGpioCfg(BOARD_ID_APL_RVP_1A, &GpioInitData, &GpioInitDataCount);
    break;
  }

#else

  Status = GetGpioCfg(0, &GpioInitData, &GpioInitDataCount);
  
#endif
//[-end-170518-IB07400867-modify]//

  if (!EFI_ERROR (Status)) {
    GpioPadConfigTable(GpioInitDataCount, GpioInitData);

    // Dump Community registers
    DumpGpioCommunityRegisters (NORTH);
    DumpGpioCommunityRegisters (NORTHWEST);
    DumpGpioCommunityRegisters (WEST);
    DumpGpioCommunityRegisters (SOUTHWEST);
    
    DumpGpioPadTable(GpioInitDataCount, GpioInitData);

//[-start-170518-IB07400867-modify]//
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
    //
    // GPIO chagned by Setup and Board ID
    //
	  switch (PlatformInfoHob->BoardId) {
  	  case BOARD_ID_LFH_CRB:
  	  case BOARD_ID_OXH_CRB:
  	  case BOARD_ID_JNH_CRB:
        if (SystemConfiguration.ScIshEnabled == 0) {
          DEBUG ((DEBUG_INFO, "Switch ISH_I2C0 & ISH_I2C1 to LPSS_I2C5 and LPSS I2C6. \n" ));
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_LPSS_I2C)/sizeof(mBXT_GpioInitData_LPSS_I2C[0]), mBXT_GpioInitData_LPSS_I2C);
        }
    	  //Checking for Linux and Android to detect the sd card.
    	  if (SystemConfiguration.OsSelection == 3 || SystemConfiguration.OsSelection == 1 ) {
          DEBUG ((DEBUG_INFO, "Switch GPIO 177 value based on OS. \n"));
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_GPIO_177)/sizeof(mBXT_GpioInitData_GPIO_177[0]), mBXT_GpioInitData_GPIO_177);
        }
  	    break;
  	  case BOARD_ID_APL_RVP_1A:
  	  case BOARD_ID_APL_RVP_2A:
  	  default:
        
        //
        // Program the GPIO [9-17] in normal mode if destination is not PTI or PTI is not in x16
        //
        if (SystemConfiguration.FwTraceDestination != 4 || SystemConfiguration.PtiMode != 8) {
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_Trace)/sizeof(mBXT_GpioInitData_Trace[0]), mBXT_GpioInitData_Trace);
        }
        
        // Note1: This BXT BIOS WA needs to be applied after PAD programming to overwrite the GPIO setting to take effect.
        // Note2: Enable TDO in BIOS SETUP as default for BXT Power-On only, need to set to AUTO prior to deliver to customer.
        // For BXT A0 Stepping only, to disable TDO GPIO to save power.
        if (PlatformInfoHob->BoardId == BOARD_ID_APL_RVP_2A) {
          DEBUG ((DEBUG_INFO, "BoardId: RVP2A\n"));
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_N_RVP2)/sizeof(mBXT_GpioInitData_N_RVP2[0]), mBXT_GpioInitData_N_RVP2);
        }
    
        if (PlatformInfoHob->FABID == FAB2) {
          DEBUG ((DEBUG_INFO, "FAB ID: FAB2\n"));
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_FAB2)/sizeof(mBXT_GpioInitData_FAB2[0]), mBXT_GpioInitData_FAB2);
        }
    
        if (SystemConfiguration.ScHdAudioIoBufferOwnership == 3) {
          DEBUG ((DEBUG_INFO, "HD Audio IO Buffer Ownership is I2S. Change GPIO pin settings for it. \n" ));
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_Audio_SSP6)/sizeof(mBXT_GpioInitData_Audio_SSP6[0]), mBXT_GpioInitData_Audio_SSP6);
        }
    
        if (SystemConfiguration.PcieRootPortEn[4] == FALSE) {
          DEBUG ((DEBUG_INFO, "Onboard LAN disable. \n" ));
          GpioPadConfigTable(sizeof(LomDisableGpio)/sizeof(LomDisableGpio[0]), LomDisableGpio);
        }
    
        if (SystemConfiguration.EPIEnable == 1) {
          DEBUG((DEBUG_INFO, "Overriding GPIO 191 for EPI\n"));
          GpioPadConfigTable(sizeof(mBXT_GpioInitData_EPI_Override) / sizeof(mBXT_GpioInitData_EPI_Override[0]), mBXT_GpioInitData_EPI_Override);
        }
        if (SystemConfiguration.GpioLock == TRUE) {
          SetGpioPadCfgLock ();
        }
  	    break;
	  }
#endif
//[-end-170518-IB07400867-modify]//
  }

#else
//[-end-170517-IB07400866-add]//
//[-start-151216-IB11270137-add]//
  Status = OemSvcModifyGpioSettingTable(
             (BXT_GPIO_PAD_INIT **)&PlatformInfoHob->PlatformGpioSetting_N,
             (BXT_GPIO_PAD_INIT **)&PlatformInfoHob->PlatformGpioSetting_NW,
             (BXT_GPIO_PAD_INIT **)&PlatformInfoHob->PlatformGpioSetting_W,
             (BXT_GPIO_PAD_INIT **)&PlatformInfoHob->PlatformGpioSetting_SW
             );
  DEBUG ( ( EFI_D_ERROR | EFI_D_INFO, "PeiOemSvcChipsetLib OemSvcModifyGpioSettingTable, Status : %r\n", Status));
  if (Status == EFI_SUCCESS) {
    return EFI_SUCCESS;
  }
//[-end-151216-IB11270137-add]//

//[-start-160427-IB07400720-add]//
#ifndef APOLLOLAKE_CRB
  //
  // Program OEM GPIO Setting
  //
  DEBUG ((EFI_D_ERROR, "PAD programing, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
  GpioPadConfigTable(sizeof(mBXT_GpioInitData_N_OEM)/sizeof(mBXT_GpioInitData_N_OEM[0]), PlatformInfoHob->PlatformGpioSetting_N);
  GpioPadConfigTable(sizeof(mBXT_GpioInitData_NW_OEM)/sizeof(mBXT_GpioInitData_NW_OEM[0]), PlatformInfoHob->PlatformGpioSetting_NW);
  GpioPadConfigTable(sizeof(mBXT_GpioInitData_W_OEM)/sizeof(mBXT_GpioInitData_W_OEM[0]), PlatformInfoHob->PlatformGpioSetting_W);
  GpioPadConfigTable(sizeof(mBXT_GpioInitData_SW_OEM)/sizeof(mBXT_GpioInitData_SW_OEM[0]), PlatformInfoHob->PlatformGpioSetting_SW);

  //
  // Dump OEM GPIO Setting
  //
  
  // Dump Community registers
  DumpGpioCommunityRegisters (NORTH);
  DumpGpioCommunityRegisters (NORTHWEST);
  DumpGpioCommunityRegisters (WEST);
  DumpGpioCommunityRegisters (SOUTHWEST);
  
  DumpGpioPadTable(sizeof(mBXT_GpioInitData_N_OEM) / sizeof(mBXT_GpioInitData_N_OEM[0]), PlatformInfoHob->PlatformGpioSetting_N);
  DumpGpioPadTable(sizeof(mBXT_GpioInitData_NW_OEM) / sizeof(mBXT_GpioInitData_NW_OEM[0]), PlatformInfoHob->PlatformGpioSetting_NW);
  DumpGpioPadTable(sizeof(mBXT_GpioInitData_W_OEM) / sizeof(mBXT_GpioInitData_W_OEM[0]), PlatformInfoHob->PlatformGpioSetting_W);
  DumpGpioPadTable(sizeof(mBXT_GpioInitData_SW_OEM) / sizeof(mBXT_GpioInitData_SW_OEM[0]), PlatformInfoHob->PlatformGpioSetting_SW);
#else
//[-end-160427-IB07400720-add]//

  switch (PlatformInfoHob->BoardId) {
//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
  case BOARD_ID_LFH_CRB:
  case BOARD_ID_OXH_CRB:
  case BOARD_ID_JNH_CRB:
    // PAD programming
    DEBUG ((DEBUG_INFO, "PAD programming, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
//[-start-160427-IB07400720-remove]//
//    GpioPadConfigTable(sizeof(mBXT_GpioInitData_N)/sizeof(mBXT_GpioInitData_N[0]), PlatformInfoHob->PlatformGpioSetting_N);
//    GpioPadConfigTable(sizeof(mBXT_GpioInitData_NW)/sizeof(mBXT_GpioInitData_NW[0]), PlatformInfoHob->PlatformGpioSetting_NW);
//    GpioPadConfigTable(sizeof(mBXT_GpioInitData_W)/sizeof(mBXT_GpioInitData_W[0]), PlatformInfoHob->PlatformGpioSetting_W);
//    GpioPadConfigTable(sizeof(mBXT_GpioInitData_SW)/sizeof(mBXT_GpioInitData_SW[0]), PlatformInfoHob->PlatformGpioSetting_SW);
//[-end-160427-IB07400720-remove]//
//[-start-161108-IB07400810-modify]//
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_N_LH)/sizeof(mBXT_GpioInitData_N_LH[0]), PlatformInfoHob->PlatformGpioSetting_N);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_SW_LH)/sizeof(mBXT_GpioInitData_SW_LH[0]), PlatformInfoHob->PlatformGpioSetting_SW);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_W_LH)/sizeof(mBXT_GpioInitData_W_LH[0]), PlatformInfoHob->PlatformGpioSetting_W);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_NW_LH)/sizeof(mBXT_GpioInitData_NW_LH[0]), PlatformInfoHob->PlatformGpioSetting_NW);
//[-end-161108-IB07400810-modify]//

    if (SystemConfiguration.ScIshEnabled == 0) {
      DEBUG ((DEBUG_INFO, "Switch ISH_I2C0 & ISH_I2C1 to LPSS_I2C5 and LPSS I2C6. \n" ));
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_LPSS_I2C)/sizeof(mBXT_GpioInitData_LPSS_I2C[0]), mBXT_GpioInitData_LPSS_I2C);
    }
//[-start-161018-IB06740518-remove]//
////[-start-160801-IB03090430-add]//
//    if (SystemConfiguration.OsSelection == 3) {
//      DEBUG ((DEBUG_INFO, "Switch GPIO 177 value based on OS. \n"));
//      GpioPadConfigTable(sizeof(mBXT_GpioInitData_GPIO_177)/sizeof(mBXT_GpioInitData_GPIO_177[0]), mBXT_GpioInitData_GPIO_177);
//    }
////[-end-160801-IB03090430-add]//
//[-end-161018-IB06740518-remove]//
//[-start-170516-IB08450375-add]//
	//Checking for Linux and Android to detect the sd card.
	if (SystemConfiguration.OsSelection == 3 || SystemConfiguration.OsSelection == 1 ) {
      DEBUG ((DEBUG_INFO, "Switch GPIO 177 value based on OS. \n"));
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_GPIO_177)/sizeof(mBXT_GpioInitData_GPIO_177[0]), mBXT_GpioInitData_GPIO_177);
    }
//[-end-170516-IB08450375-add]//
    break;

  case BOARD_ID_BFH_IVI:
    // PAD programming
    DEBUG ((DEBUG_INFO, "PAD programming, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_N_MRB_IVI)/sizeof(mBXT_GpioInitData_N_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_N);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_NW_BFH_IVI)/sizeof(mBXT_GpioInitData_NW_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_W_BFH_IVI)/sizeof(mBXT_GpioInitData_W_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_W);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_SW_BFH_IVI)/sizeof(mBXT_GpioInitData_SW_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_SW);
    break;

  case BOARD_ID_MRB_IVI:
    // PAD programming
    DEBUG ((DEBUG_INFO, "PAD programming, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_N_MRB_IVI)/sizeof(mBXT_GpioInitData_N_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_N);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_NW_MRB_IVI)/sizeof(mBXT_GpioInitData_NW_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_W_MRB_IVI)/sizeof(mBXT_GpioInitData_W_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_W);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_SW_MRB_IVI)/sizeof(mBXT_GpioInitData_SW_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_SW);
    break;

//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//
  case BOARD_ID_BXT_RVP:
    DEBUG ((DEBUG_INFO, "Board ID: 0x%X\n", BOARD_ID_BXT_RVP));
  default:

    // PAD programming
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_N)/sizeof(mBXT_GpioInitData_N[0]), PlatformInfoHob->PlatformGpioSetting_N);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_NW)/sizeof(mBXT_GpioInitData_NW[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_W)/sizeof(mBXT_GpioInitData_W[0]), PlatformInfoHob->PlatformGpioSetting_W);
    GpioPadConfigTable(sizeof(mBXT_GpioInitData_SW)/sizeof(mBXT_GpioInitData_SW[0]), PlatformInfoHob->PlatformGpioSetting_SW);

//[-start-160719-IB03090429-add]//
    //
    // Program the GPIO [9-17] in normal mode if destination is not PTI or PTI is not in x16
    //
    if (SystemConfiguration.FwTraceDestination != 4 || SystemConfiguration.PtiMode != 8) {
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_Trace)/sizeof(mBXT_GpioInitData_Trace[0]), mBXT_GpioInitData_Trace);
    }
//[-end-160719-IB03090429-add]//
    // Note1: This BXT BIOS WA needs to be applied after PAD programming to overwrite the GPIO setting to take effect.
    // Note2: Enable TDO in BIOS SETUP as default for BXT Power-On only, need to set to AUTO prior to deliver to customer.
    // For BXT A0 Stepping only, to disable TDO GPIO to save power.

    if (PlatformInfoHob->BoardId == BOARD_ID_APL_RVP_2A) {
      DEBUG ((DEBUG_INFO, "BoardId: RVP2A\n"));
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_N_RVP2)/sizeof(mBXT_GpioInitData_N_RVP2[0]), mBXT_GpioInitData_N_RVP2);
    }

    if (PlatformInfoHob->FABID == FAB2) {
      DEBUG ((DEBUG_INFO, "FAB ID: FAB2\n"));
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_FAB2)/sizeof(mBXT_GpioInitData_FAB2[0]), mBXT_GpioInitData_FAB2);
    }

//[-start-160817-IB03090432-modify]//
    if (SystemConfiguration.TDO == 0) { // Disable
      DEBUG ((DEBUG_INFO, " Setup option to disable TDO\n" ));
      //GpioPadWrite(N_TDO, 0x0); // Disable TDO
    }
//[-end-160817-IB03090432-modify]//

    if (SystemConfiguration.ScHdAudioIoBufferOwnership == 3) {
      DEBUG ((DEBUG_INFO, "HD Audio IO Buffer Ownership is I2S. Change GPIO pin settings for it. \n" ));
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_Audio_SSP6)/sizeof(mBXT_GpioInitData_Audio_SSP6[0]), mBXT_GpioInitData_Audio_SSP6);
    }

    if (SystemConfiguration.PcieRootPortEn[4] == FALSE) {
      DEBUG ((DEBUG_INFO, "Onboard LAN disable. \n" ));
      GpioPadConfigTable(sizeof(LomDisableGpio)/sizeof(LomDisableGpio[0]), LomDisableGpio);
    }

    if (SystemConfiguration.EPIEnable == 1) {
      DEBUG((DEBUG_INFO, "Overriding GPIO 191 for EPI\n"));
      GpioPadConfigTable(sizeof(mBXT_GpioInitData_EPI_Override) / sizeof(mBXT_GpioInitData_EPI_Override[0]), mBXT_GpioInitData_EPI_Override);
    }
    if (SystemConfiguration.GpioLock == TRUE) {
      SetGpioPadCfgLock ();
    }
    DEBUG ((DEBUG_INFO, "No board ID available for this board ....\n"));
    break;
  }

  // Dump Community registers
  DumpGpioCommunityRegisters (NORTH);
  DumpGpioCommunityRegisters (NORTHWEST);
  DumpGpioCommunityRegisters (WEST);
  DumpGpioCommunityRegisters (SOUTHWEST);

  switch (PlatformInfoHob->BoardId) {
//[-start-160406-IB07400715-remove]//
//#if (BXTI_PF_ENABLE == 1)
//[-end-160406-IB07400715-remove]//
  case BOARD_ID_LFH_CRB:
  case BOARD_ID_OXH_CRB:
  case BOARD_ID_JNH_CRB:
    // PAD programming
    DEBUG ((DEBUG_INFO, "Dump Community pad registers, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
//[-start-170517-IB07400866-modify]//
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_N_LH)/sizeof(mBXT_GpioInitData_N_LH[0]), PlatformInfoHob->PlatformGpioSetting_N);
//[-end-170517-IB07400866-modify]//
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_NW_LH) / sizeof(mBXT_GpioInitData_NW_LH[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_W_LH) / sizeof(mBXT_GpioInitData_W_LH[0]), PlatformInfoHob->PlatformGpioSetting_W);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_SW_LH) / sizeof(mBXT_GpioInitData_SW_LH[0]), PlatformInfoHob->PlatformGpioSetting_SW);
    break;

  case BOARD_ID_BFH_IVI:
    DEBUG ((DEBUG_INFO, "Dump Community pad registers, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_N_BFH_IVI) / sizeof(mBXT_GpioInitData_N_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_N);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_NW_BFH_IVI) / sizeof(mBXT_GpioInitData_NW_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_W_BFH_IVI) / sizeof(mBXT_GpioInitData_W_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_W);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_SW_BFH_IVI) / sizeof(mBXT_GpioInitData_SW_BFH_IVI[0]), PlatformInfoHob->PlatformGpioSetting_SW);
    break;

  case BOARD_ID_MRB_IVI:
    DEBUG ((DEBUG_INFO, "Dump Community pad registers, Board ID: 0x%X\n", PlatformInfoHob->BoardId));
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_N_MRB_IVI) / sizeof(mBXT_GpioInitData_N_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_N);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_NW_MRB_IVI) / sizeof(mBXT_GpioInitData_NW_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_W_MRB_IVI) / sizeof(mBXT_GpioInitData_W_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_W);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_SW_MRB_IVI) / sizeof(mBXT_GpioInitData_SW_MRB_IVI[0]), PlatformInfoHob->PlatformGpioSetting_SW);
    break;

//[-start-160406-IB07400715-remove]//
//#endif
//[-end-160406-IB07400715-remove]//

  case BOARD_ID_BXT_RVP:
  case BOARD_ID_BXT_RVVP: // RVV BXTP
  default:

    // Dump Community pad registers
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_N)/sizeof(mBXT_GpioInitData_N[0]), PlatformInfoHob->PlatformGpioSetting_N);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_NW)/sizeof(mBXT_GpioInitData_NW[0]), PlatformInfoHob->PlatformGpioSetting_NW);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_W)/sizeof(mBXT_GpioInitData_W[0]), PlatformInfoHob->PlatformGpioSetting_W);
    DumpGpioPadTable(sizeof(mBXT_GpioInitData_SW)/sizeof(mBXT_GpioInitData_SW[0]), PlatformInfoHob->PlatformGpioSetting_SW);

    break;
  }
  
//[-start-160427-IB07400720-add]//
#endif
//[-end-160427-IB07400720-add]//
//[-start-170517-IB07400866-add]//
#endif
//[-end-170517-IB07400866-add]//
  return EFI_SUCCESS;
}
//[-end-160513-IB03090427-modify]//
