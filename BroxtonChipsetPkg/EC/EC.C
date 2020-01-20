/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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


//
// Module specific Includes
//
#include <ScAccess.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Protocol/SmmGpiDispatch2.h>
#include <Protocol/SmmIchnDispatch.h>
#include <Protocol/SmmCpu.h>
#include <Framework/SmmCis.h>
#include <Protocol/ScSmmIoTrapControl.h>
#include <Protocol/EcAccess.h>
#include <Protocol/GlobalNvsArea.h>

#include "EC.h"
//[-start-160906-IB03090434-modify]//
#include <ChipsetSetupConfig.h>
//[-end-160906-IB03090434-modify]//
#include "CpuAccess.h"
#include <Library/GpioLib.h>
#include <Library/TimerLib.h>
#include <Library/DxeInsydeChipsetLib.h>
//[-start-161022-IB07400803-modify]//
#include <Library/MultiPlatformBaseLib.h>
//[-end-161022-IB07400803-modify]//

//[-start-161109-IB07400810-add]//
#ifdef BUILD_TIME_CHECK_UNKNOWN_GPIO
//
// Apollo Lake RVP EC pins:
//
// GPIO_74(G62) -- SOC_CS_WAKE -- CS_WAKE_EC -- NPCE285LA0DX.GPIO47/SCL4A/N2TCK
// GPIO_49(M41) -- SOC_EXTSMI_N -- SMC_EXTSMI_N -- NPCE285LA0DX.GPIO65/SMI#
//
#define NW_GPIO_74            (((UINT32)GPIO_MMIO_OFFSET_NW)<<16)+GPIO_PADBAR+0x0110 
#define N_GPIO_49             (((UINT32)GPIO_MMIO_OFFSET_N)<<16)+GPIO_PADBAR+0x0188
#endif
//[-end-161109-IB07400810-add]//

#define EC_LOW_POWER_EXIT_GPIO          NW_GPIO_74      // Platform specific

//
// Global variables
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMM_GPI_DISPATCH2_PROTOCOL         *mSmmGpiDispatch;
GLOBAL_REMOVE_IF_UNREFERENCED SC_SMM_IO_TRAP_CONTROL_PROTOCOL        *mScSmmIoTrapControl;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_HANDLE                              mIchIoTrapHandle;

static EFI_EC_ACCESS_PROTOCOL                                         mEcAccess;

GLOBAL_REMOVE_IF_UNREFERENCED UINT32                                  mGpioSmiPad = 0;      // SMC_EXT_SMI GPIO pad

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GLOBAL_NVS_AREA_PROTOCOL            *mGlobalNvsAreaPtr;

#define PCI_CFG_ADDR(bus, dev, func, reg) \
    ((VOID *) (UINTN) (PCIEX_BASE_ADDRESS + ((bus) << 20) + ((dev) << 15) + ((func) << 12) + reg))

/**
  EC get mother board ID

  @param[in] FabID            Pointer to a UINT8 buffer.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECGetMotherBoardID (
  UINT8       *FabID
  )
{
  EFI_STATUS  Status;
  EcWriteCmd (SMC_FAB_ID);
  Status = EcReadData (FabID);
  return Status;
}

/**
  Switch EC to ACPI Mode.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECEnableACPIMode (
)
{
//  ECDisableSMINotify();
  EcWriteCmd (SMC_SMI_DISABLE);
  EcWriteCmd (SMC_ENABLE_ACPI_MODE);
  if ( mGlobalNvsAreaPtr->Area->AlsEnable ) {
    EcWriteCmd (SMC_ALS_ENABLE);
    EcWriteData(0x01); // enable ALS
  }
  else {
    EcWriteCmd (SMC_ALS_ENABLE);
    EcWriteData(0x00); // disable ALS
  }
  return EFI_SUCCESS;

}


/**
  Switch EC to Non-ACPI Mode.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECDisableACPIMode (
)
{
  EcWriteCmd (SMC_DISABLE_ACPI_MODE);
  return EFI_SUCCESS;
}

/**
  EC disable SMI notify.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECDisableSMINotify (
)
{
  EcWriteCmd (SMC_DISABLE_SMI_NOTIFY);
  return EFI_SUCCESS;
}


/**
  EC enable SMI notify.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECEnableSMINotify (
)
{
  EcWriteCmd (SMC_ENABLE_SMI_NOTIFY);
  return EFI_SUCCESS;
}


/**
  EC shut down system.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECShutDownSystem(
  )
{
  EFI_STATUS  Status;
  Status = EcWriteCmd (SMC_SYSTEM_POWER_OFF);
  return Status;
}

/**
  Switch EC to ACPI Mode.

  @param[in] Revision

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECGetECVersion(
  UINT8       *Revision
)
{
  EFI_STATUS  Status;
  EcWriteCmd (SMC_READ_REVISION);
  Status = EcReadData (Revision);
  return Status;
}

/**
  EC enable Lan.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECEnableLan(
)
{
  EFI_STATUS  Status;
  Status = EcWriteCmd (SMC_LAN_ON);
  return Status;
}


/**
  EC disable Lan.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECDisableLan(
)
{
  EFI_STATUS  Status;
  Status = EcWriteCmd (SMC_LAN_OFF);
  return Status;
}

/**
  EC deep Sx config.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
ECDeepSxConfig(
  UINT8     EcData
)
{
  EFI_STATUS  Status;
  Status = EcWriteCmd (SMC_DEEPSX_CMD);
  Status = EcWriteData (EcData);
  return Status;
}

/**
  EC turbo ctrl mode.

        @param[in] Enable             Enable / Disable turbo ctrl test mode
        @param[in] ACBrickCap         Data of AC Brick Capacity
        @param[in] ECPollPeriod       Data of EC Poll Period
        @param[in] ECGuardBandValue   Value of EC Guard Band
        @param[in] ECAlgorithmSel     Value of EC Algorithm Selection

  @retval EFI_SUCCESS           Function successfully executed.
**/
EFI_STATUS
EcTurboCtrlMode(
        UINT8    Enable,
        UINT8    ACBrickCap,
        UINT8    ECPollPeriod,
        UINT8    ECGuardBandValue,
        UINT8    ECAlgorithmSel
)
{
  EFI_STATUS  Status;

  if (Enable) {
    Status = EcWriteCmd (SMC_TURBOCTRL_TESTMODE_ENABLE);
    Status = EcWriteData (ACBrickCap);
    Status = EcWriteData (ECPollPeriod);
    Status = EcWriteData (ECGuardBandValue);
    Status = EcWriteData (ECAlgorithmSel);
  } else {
    Status = EcWriteCmd (SMC_TURBOCTRL_TESTMODE_DISABLE);
  }

  return Status;

}

/**
  Allow EC to control fan when OS is hung.

  @param[in] CpuTemp
  @param[in] CpuFanSpeed

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcFailSafeFanCtrl(
        UINT8    CpuTemp,
        UINT8    CpuFanSpeed
)
{
  EFI_STATUS  Status;
  Status = EcWriteCmd (SMC_FAIL_SAFE_FAN_CTRL);
  Status = EcWriteData (CpuTemp);
  Status = EcWriteData (CpuFanSpeed);

  return Status;
}

/**
  Description: Enable and Disable Port 62/66 IO trap When EC 
  enters and exits low power mode 

  @param[in] DispatchHandle  - Handle of this dispatch function
  @param[in] DispatchContext - Pointer to the dispatch function's context.
                                        The DispatchContext fields are filled in by the dispatching driver
                                        prior to invoke this dispatch function

**/
VOID EcCsSmiHandler (
  IN EFI_HANDLE  DispatchHandle,
  IN EFI_SMM_GPI_REGISTER_CONTEXT  *DispatchContext
)
{

  EFI_STATUS  Status;
  UINT32      EcSmiGpiVal;
  UINT32      EcLowPowerExitGpioVal;
  UINT16      AcpiBase;
  
  DEBUG ((DEBUG_INFO, "EcCsSmiHandler, start \n"));

  AcpiBase = PcdGet16(PcdScAcpiIoPortBaseAddress);

  //
  // Read config register for EC SMI# GPIO
  //
  GpioGetInputInversion (mGpioSmiPad, &EcSmiGpiVal);

  //
  // Change GPI_INV bit of EC SMI# so we enter handler on next edge.
  //
  EcSmiGpiVal ^= BIT0;
  GpioSetInputInversion (mGpioSmiPad, EcSmiGpiVal);

  //
  // Clear ALT_GPI_SMI_STS register for GPIO (EC SMI#)
  //
  GpioClearGpiSmiSts (mGpioSmiPad);

  //
  // Clear pmcSmiSts for GPIO
  //
  IoWrite16(AcpiBase + R_SMI_STS, (IoRead16(AcpiBase + R_SMI_STS) | B_GPIO_GP_SMI_STS));

  //
  // Read config register for GPIO (EC Low Power Exit)
  //
  GpioGetInputValue (EC_LOW_POWER_EXIT_GPIO, &EcLowPowerExitGpioVal);

  //
  // If GPIO (EC SMI#) is Low, then Enable IO trap, and Drive GPIO (EC Low Power Exit) Low
  //
  if(EcSmiGpiVal == 0) {
    DEBUG ((DEBUG_INFO, "EC SMI raised, Resuming I/O Trap Control Handler \n"));
    //
    // Enable I/O Trap for Ports 62/66
    //
    Status = mScSmmIoTrapControl->Resume (
        mScSmmIoTrapControl,
        mIchIoTrapHandle
    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "mScSmmIoTrapControl->Resume returned %r\n", Status));
      return;
   }

    //
    // Drive GPIO (EC Low Power Exit) Low
    //
    EcLowPowerExitGpioVal = 0;

  //
  // Else GPIO (EC SMI#) is High, Diable IO trap, and Drive (EC Low Power Exit) High
  //
  } else {
    DEBUG ((DEBUG_INFO, "Disabling IO Trap \n"));
    //
    // Disable IO Trap for Ports 62/66
    //
    Status = mScSmmIoTrapControl->Pause (
        mScSmmIoTrapControl,
        mIchIoTrapHandle
    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "mScSmmIoTrapControl->Pause returned %r\n", Status));
      return;
   }

    //
    // Drive GPIO (EC Low Power Exit)  High
    //
    EcLowPowerExitGpioVal = 1;
  }

  //
  // Write new value to GPIO (EC Low Power Exit)
  //
  GpioSetOutputValue (EC_LOW_POWER_EXIT_GPIO, EcLowPowerExitGpioVal);

  DEBUG ((DEBUG_INFO, "EcCsSmiHandler, end \n"));
  return;

}

/**
  Traps on reads to IO ports 62/66 and when a read occurs, a 
    wake signal is sent to the EC. The funtion will wait until
    the EC is awake then perform the read and stuff the EC
    return value in AL for the OS.

  @param[in] DispatchHandle       - Handle of this dispatch function
  @param[in] DispatchContext      - Pointer to the dispatch function's context.
                                             The DispatchContext fields are filled in by the dispatching driver
                                             prior to invoke this dispatch function
  @param[in] CommBuffer           - Point to the CommBuffer structure
  @param[in] CommBufferSize       - Point to the Size of CommBuffer structure 
  
**/
VOID
EcCsIoTrapHandler (
  IN EFI_HANDLE                  DispatchHandle,
  IN CONST VOID                  *DispatchContext,
  IN OUT VOID                    *CommBuffer,
  IN OUT UINTN                   *CommBufferSize
)
{
  UINT16  CpuIndex;
  UINT8   IoPort=0;
  UINT8   IoPortValue;
  UINT32  EcSmiGpiVal;
  UINT32  EAX;
  EFI_STATUS Status;

  EFI_SMM_CPU_PROTOCOL          *mSmmCpuProtocol;
  EFI_SMM_SAVE_STATE_IO_INFO    SmiIoInfo;

  EFI_SMM_CPU_IO2_PROTOCOL *mSmmIo;
  
  DEBUG((DEBUG_INFO,"EcCsIoTrapHandler, Entry\n"));
        
  Status = gSmst->SmmLocateProtocol (&gEfiSmmCpuProtocolGuid, NULL, (VOID **)&mSmmCpuProtocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "\nSMM: Cpu Protocol Locate Failure!!, %r\n", Status));
  }
  ASSERT_EFI_ERROR (Status);
  //
  // Make sure SMM CPU I/O 2 Procol has been installed into the handle database
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmCpuIo2ProtocolGuid, NULL,  &mSmmIo);
  ASSERT_EFI_ERROR(Status); 
  //
  // Find which Thread caused the IO Trap AND which IoPort is accessed 
  //
  for (CpuIndex = 0; CpuIndex < gSmst->NumberOfCpus; CpuIndex++) {
    Status = mSmmCpuProtocol->ReadSaveState (
                              mSmmCpuProtocol,
                              sizeof(EFI_SMM_SAVE_STATE_IO_INFO),
                              EFI_SMM_SAVE_STATE_REGISTER_IO,
                              CpuIndex,
                              &SmiIoInfo
                              );
    if (EFI_ERROR (Status)) {
      continue;
    }
    if ((SmiIoInfo.IoPort == 0x62) || (SmiIoInfo.IoPort == 0x66)) {
      //
      // Get value of port that was trapped
      //
      IoPort = (UINT8)SmiIoInfo.IoPort;
      DEBUG ((DEBUG_INFO, "Found Matching CPU\n"));
      break;
    }
  }
  
  //
  // Drive GPIO (EC Low Power Exit) Low->High Edge to wake EC
  //
  GpioSetOutputValue (EC_LOW_POWER_EXIT_GPIO, 0);
  MicroSecondDelay(10); //10us
  GpioSetOutputValue (EC_LOW_POWER_EXIT_GPIO, 1);

  //
  // Wait until EC has exited low power mode
  //
  DEBUG ((DEBUG_INFO, "Wait For EC to exit from Low Power Mode\n"));
  
  do {
    //
    // Read config register for GPIO
    //
    GpioGetInputValue (mGpioSmiPad, &EcSmiGpiVal);
  } while((EcSmiGpiVal != 1));
  DEBUG ((DEBUG_INFO, "EC exited from Low Power Mode\n"));

  //
  // Read Port 66 or 62
  //
  Status = mSmmIo->Io.Read( &gSmst->SmmIo,
                    SMM_IO_UINT8,
                    IoPort,
                    1,
                    &IoPortValue);
  //InternalContent second read for timing WA @todo
  mSmmIo->Io.Read( &gSmst->SmmIo,
                    SMM_IO_UINT8,
                    IoPort,
                    1,
                    &IoPortValue);

  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "SmmIo Read Failure: %r\n", Status));
  }

  //
  // Return value read from EC to the OS
  //  
  Status = mSmmCpuProtocol->ReadSaveState (
                        mSmmCpuProtocol,
                        sizeof (EAX),
                        EFI_SMM_SAVE_STATE_REGISTER_RAX,
                        CpuIndex,
                        &EAX
                        );
    DEBUG ((DEBUG_INFO, "EC:EFI_SMM_CPU_PROTOCOL->ReadSaveState() returned %r\n", Status));
    ASSERT_EFI_ERROR(Status);

    EAX &= 0xFFFFFF00;
    EAX |= (UINT32)IoPortValue;

    Status = mSmmCpuProtocol->WriteSaveState (
                        mSmmCpuProtocol,
                        sizeof (EAX),
                        EFI_SMM_SAVE_STATE_REGISTER_RAX,
                        CpuIndex,
                        &EAX
                        );
    DEBUG ((DEBUG_INFO, "EC:EFI_SMM_CPU_PROTOCOL->WriteSaveState() returned %r\n", Status));
    ASSERT_EFI_ERROR(Status);
  

  DEBUG ((DEBUG_INFO, "EcCsIoTrapHandler, Exit\n"));

  return;
}

/**
  Initialize and register for the IO Trap and EC SMI Handler to deal with the EC entering low power mode.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS SetupEcIoTrap()
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      EcHandle;
  EFI_SMM_GPI_REGISTER_CONTEXT    EcContext;
  EFI_SMM_ICHN_DISPATCH_CONTEXT   IchnContext;
  EFI_SMM_ICHN_DISPATCH_PROTOCOL  *IchnDispatch;
  UINT16                          AcpiBase;

  DEBUG ((DEBUG_INFO, "SetupEcIoTrap, start \n"));
  mGpioSmiPad = N_GPIO_49;  // SOC_EXTSMI_N

  AcpiBase = PcdGet16(PcdScAcpiIoPortBaseAddress);
  //
  // Get GpiNum according to GPIO Group+Pad
  //
  GpioGetGpiSmiNum (mGpioSmiPad, &EcContext.GpiNum);

  //
  // Register SMI handler EC SMI usage during Connected Standby
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmGpiDispatch2ProtocolGuid, NULL, &mSmmGpiDispatch);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  } else {
    //
    // Set GPI_SMI_EN
    //
    IoWrite16(AcpiBase + R_SMI_EN, (IoRead16(AcpiBase + R_SMI_EN) | B_GPIO_GP_SMI_EN));
  }

  DEBUG ((DEBUG_ERROR, "Installing EcCsSmiHandler Handler \n"));
  Status = mSmmGpiDispatch->Register (
                              mSmmGpiDispatch,
                              (EFI_SMM_HANDLER_ENTRY_POINT2)EcCsSmiHandler,
                              &EcContext,
                              &EcHandle
                              );
  DEBUG ((DEBUG_INFO, "EcCsSmiHandler Install Status: %r\n", Status)); 
  //
  // Get the ICHn protocol
  //
  Status = gSmst->SmmLocateProtocol(&gEfiSmmIchnDispatchProtocolGuid,
                                    NULL,
                                    (VOID **) &IchnDispatch);
  ASSERT_EFI_ERROR(Status);
  
  //
  // Register EC Low Power IO Trap Handler
  //
  DEBUG ((DEBUG_INFO, "Installing EcCsIoTrapHandler Handler \n"));
  IchnContext.Type = IchnMcSmi;
  Status = IchnDispatch->Register(
                        IchnDispatch,
                        (EFI_SMM_ICHN_DISPATCH)EcCsIoTrapHandler,
                        &IchnContext,
                        &mIchIoTrapHandle
                      );
  ASSERT_EFI_ERROR( Status );  
  
  //
  // Disable IO Trap Handler
  //
  Status = gSmst->SmmLocateProtocol (&gScSmmIoTrapControlGuid, NULL, (VOID **)&mScSmmIoTrapControl);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  Status = mScSmmIoTrapControl->Pause (
                        mScSmmIoTrapControl,
                        mIchIoTrapHandle
                        );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "mScSmmIoTrapControl->Pause returned %r\n", Status));
    return Status;
  }
  DEBUG ((DEBUG_INFO, "SetupEcIoTrap, exit \n"));
  return EFI_SUCCESS;
}

/**
  EC initializes.

  @param[in] ImageHandle      Image handle of this driver.
  @param[in] SystemTable      Pointer to the System Table.

  @retval EFI_SUCCESS         Function successfully executed.
  @retval EFI_UNSUPPORTED     Install protocol failure.
**/
EFI_STATUS
InitializeEcSmm (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  EFI_STATUS                                Status;
//[-start-160906-IB03090434-modify]//
  CHIPSET_CONFIGURATION                     SetupData;
//[-end-160906-IB03090434-modify]//
  UINTN                                     VariableSize;
  EFI_GLOBAL_NVS_AREA_PROTOCOL              *GlobalNvsAreaProtocol;

  DEBUG ((DEBUG_INFO, "InitializeEcSmm, Start\n"));

//[-start-161018-IB06740518-add]//
//[-start-161022-IB07400802-modify]//
//#if BXTI_PF_ENABLE
  //For IOTG CRB
  if (IoRead8(0x62) == 0xFF) {
    //
    // EC do not exist
    //
    return EFI_UNSUPPORTED;
  }
//#endif
//[-end-161022-IB07400802-modify]//
//[-end-161018-IB06740518-add]//

  mEcAccess.Handle            = NULL;
  mEcAccess.QuerryCmd         = EcQueryCommand;
  mEcAccess.WriteCmd          = EcWriteCmd;
  mEcAccess.WriteData         = EcWriteData;
  mEcAccess.ReadData          = EcReadData;
  mEcAccess.ReadMem           = EcReadMem;
  mEcAccess.WriteMem          = EcWriteMem;
  mEcAccess.AcpiEnable        = ECEnableACPIMode;
  mEcAccess.AcpiDisable       = ECDisableACPIMode;
  mEcAccess.SMINotifyEnable   = ECEnableSMINotify;
  mEcAccess.SMINotifyDisable  = ECDisableSMINotify;
  mEcAccess.ShutDownSystem    = ECShutDownSystem;
  mEcAccess.GetMotherBoardID  = ECGetMotherBoardID;
  mEcAccess.GetECVersion      = ECGetECVersion;
  mEcAccess.EnableLan         = ECEnableLan;
  mEcAccess.DisableLan        = ECDisableLan;
  mEcAccess.DeepSxConfig      = ECDeepSxConfig;
  mEcAccess.TurboCtrlMode     = EcTurboCtrlMode;
  mEcAccess.GetSwitchStatus   = EcGetSwitchStatus;
  mEcAccess.FailSafeFanCtrl   = EcFailSafeFanCtrl;

  Status = gSmst->SmmInstallProtocolInterface (
                  &mEcAccess.Handle,
                  &gEfiEcAccessProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mEcAccess
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "InstallMultipleProtocolInterfaces returned %r\n", Status));
    return EFI_UNSUPPORTED;
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Couldn't register the EC SMI handler.  Status: %r\n", Status));
    return Status;
  }

  //
  // Locate our shared data area
  //
  Status = gBS->LocateProtocol (&gEfiGlobalNvsAreaProtocolGuid, NULL, &GlobalNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);
  mGlobalNvsAreaPtr         = GlobalNvsAreaProtocol;

  VariableSize = sizeof (SETUP_DATA);
  
//[-start-160906-IB03090434-modify]//
  VariableSize = PcdGet32 (PcdSetupConfigSize);
  Status = gRT->GetVariable (
                SETUP_VARIABLE_NAME,
                &gSystemConfigurationGuid,
                NULL,
                &VariableSize,
                &SetupData
                );  
//[-end-160906-IB03090434-modify]//
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (&SetupData, sizeof (CHIPSET_CONFIGURATION));  
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR,"Error getting Setup Variable, Status: %r\n", Status));
      return Status;  
    }
  }

//[-start-161018-IB06740518-add]//
//[-start-161022-IB07400803-modify]//
//#if BXTI_PF_ENABLE == 1
  if (IsIOTGBoardIds()) {
    EcWriteCmd (SMC_SET_SHUTDOWN_TEMP);
    EcWriteData (SetupData.CriticalThermalTripPoint);
  }
//#endif
//[-end-161022-IB07400803-modify]//
//[-end-161018-IB06740518-add]//

  //
  // Send the command to put the EC into low power mode if it is enabled in setup
  //
  if (SetupData.LowPowerS0Idle && SetupData.EcLowPowerMode && SetupData.CSNotifyEC) {
    DEBUG ((DEBUG_INFO, "Setup Options Enabled for S0Idle\n"));
    EcWriteCmd (EC_LOW_POWER_MODE);
    EcWriteData(0x01);
    SetupEcIoTrap();
  } else {
    EcWriteCmd (EC_LOW_POWER_MODE);
    EcWriteData(0x00);
  }
  
  DEBUG ((DEBUG_INFO, "InitializeEcSmm, End\n"));
  return EFI_SUCCESS;

}

/**
  Read the EC Query Value

  @param[in] pQdata           Pointer to a UINT8 buffer.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcQueryCommand (
  UINT8   *pQdata
)
{
  EFI_STATUS  Status;
  Status = EcWriteCmd (SMC_QUERY_SMI);
  Status = EcReadData (pQdata);
  return EFI_SUCCESS;
}


/**
  Wait till EC I/P buffer is free.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcIbFree ()
{
  UINT8   Status;
  do {
    gSmst->SmmIo.Io.Read( &gSmst->SmmIo,
            SMM_IO_UINT8,
            EcCommandPort,
            1,
            &Status);
  } while (Status & 2);
  return EFI_SUCCESS;

}


/**
  Wait till EC O/P buffer is full

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcObFull ()
{
  UINT8    Status;
  do {
    gSmst->SmmIo.Io.Read( &gSmst->SmmIo, SMM_IO_UINT8, EcCommandPort, 1, &Status);
  } while (!(Status & 1));

  return EFI_SUCCESS;

}


/**
  Send EC command

  @param[in] cmd              Writing command to EC's command port.

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcWriteCmd (
  UINT8    cmd
)
{
  EcIbFree();
  gSmst->SmmIo.Io.Write( &gSmst->SmmIo,
           SMM_IO_UINT8,
           EcCommandPort,
           1,
           &cmd);
  return EFI_SUCCESS;
}


/**
     Write Data from EC data port

     @param[in] data             Writing data to EC's data port.

     @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcWriteData (
  UINT8    data
)
{
  EcIbFree();
  gSmst->SmmIo.Io.Write( &gSmst->SmmIo,
                    SMM_IO_UINT8,
                    EcDataPort,
                    1,
                    &data);
  return EFI_SUCCESS;
}

/**
     Read Data from EC data Port.

     @param[in] pData            Pointer to a UINT8 buffer.

     @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcReadData (
  UINT8   *pData
)
{
  if (EFI_ERROR(EcObFull())) return EFI_DEVICE_ERROR;
  gSmst->SmmIo.Io.Read( &gSmst->SmmIo, SMM_IO_UINT8, EcDataPort, 1, pData);
  return EFI_SUCCESS;
}


/**
  Read Data from EC Memory from location pointed by Index.

  @param[in] Index            Index of the read data
  @param[in] Data             Data to be read

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcReadMem (
  UINT8  Index,
  UINT8  *Data
)
{
  UINT8  cmd = SMC_READ_EC;
  EcWriteCmd (cmd);
  EcWriteData(Index);
  EcReadData(Data);
  return EFI_SUCCESS;
}


/**
  Write Data to EC memory at location pointed by Index.

  @param[in] Index            Index of the written data
  @param[in] Data             Data to be written

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcWriteMem (
  UINT8  Index,
  UINT8  Data
)
{
  UINT8  cmd = SMC_WRITE_EC;
  EcWriteCmd (cmd);
  EcWriteData(Index);
  EcWriteData(Data);
  return EFI_SUCCESS;
}

/**
  Get the info from EC on Developer Mode / Recovery Mode switch.
     EC will return 0 in bit3 when the Recovery Mode switch is pressed and
     will return 0 in bit6 when Developer Mode switch is pressed.

  @param[in] Data             Read data from EC

  @retval EFI_SUCCESS         Function successfully executed.
**/
EFI_STATUS
EcGetSwitchStatus (
  UINT8   *Data
)
{
  UINT8  cmd = SMC_GET_SWITCH_STATUS;
  EcWriteCmd (cmd);
  EcReadData(Data);
  return EFI_SUCCESS;
}
