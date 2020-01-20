/** @file
  Common driver entry point implementation.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/TimerLib.h>
#include <PlatformBaseAddresses.h>
#include <library/I2cLib.h>
#include <Library/PlatformSmsc3750Lib.h>
#include <Library/PmicLib.h>

EFI_EVENT                             mMuxEvent;
UINT8                                 ChargerFlg;
UINT32                                CurrentState=8;//1=device;0=host
BOOLEAN                               I2cERROR=FALSE;
#define IO_EXPANDER_I2C_BUS_NO  0x00
#define IO_EXPANDER_SLAVE_ADDR  0x23
#define INPUT_REGISTER          2
#define OUTPUT_REGISTER_2       5
#define CONFIG_REGISTER_2       13

/**
  Enable SMSC3750 Mux for Host mode or OTG mode, Mux 1 path for Host mode, Mux 2 path for OTG mode.

  @param  Index                     Host/OTG mode to enable.
**/
EFI_STATUS
EFIAPI
SmscEnableHostOrOtgMode (
  IN UINT8     Mode
  )
{
  UINT8        Value;
  EFI_STATUS   Status;

  ASSERT(Mode == HOST_MODE || Mode == OTG_MODE);

  Status = ByteReadI2C(SMSC3750_I2C_BUS_NO,
                       SMSC3750_I2C_SLAVE_ADDR,
                       SMSC3750_CONFIG_REG,
                       1,
                       &Value
                       );

  if (EFI_ERROR(Status)){
    DEBUG ((DEBUG_ERROR, "ByteReadI2C Addr 0x%x, 0ffset 0x%x, Status %r\n",
            SMSC3750_I2C_SLAVE_ADDR, SMSC3750_CONFIG_REG, Status));
    return Status;
  }

  Value = Value & 0x9F;
  if (Mode == HOST_MODE){
    Value = Value | ENABLE_MUX1;
  } else {
    Value = Value | ENABLE_MUX2;
  }

  Status = ByteWriteI2C(SMSC3750_I2C_BUS_NO,
                        SMSC3750_I2C_SLAVE_ADDR,
                        SMSC3750_CONFIG_REG,
                        1,
                        &Value
                        );
  return Status;
}
/**
  Turn on or off VBUS for OTG

  @param  bTurnOn    TRUE-turn on VBUS   FALSE-turn off VBUS

**/
EFI_STATUS
EFIAPI
VbusControl (BOOLEAN bTurnOn)
{
  EFI_STATUS                Status = EFI_NO_RESPONSE;
  UINT8                     Data8;

  Status = ByteReadI2C(IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, CONFIG_REGISTER_2, 1, &Data8); // Read register 13, configure register 2
  ASSERT_EFI_ERROR (Status);
  if (EFI_SUCCESS == Status){
    DEBUG((EFI_D_INFO, "data: 0x%02X\n", Data8));
    Data8 = Data8 & (~BIT4);
    // As BayTrail-CR design, configure pin 14 to GPO hig for battery power
    DEBUG((EFI_D_INFO, "Write I/O Expander, I2C Bus:%d, slave address:%d, register:%d, Value:0x%x\n", \
         IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, CONFIG_REGISTER_2, Data8));
    Status = ByteWriteI2C(IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, CONFIG_REGISTER_2, 1, &Data8); 
    DEBUG((EFI_D_INFO, "Config pin 14 to GPO, %r\n", Status));
    
    if (bTurnOn == TRUE){
      Data8 = BIT4;
    }else{
      Data8 = 0;
    }
    DEBUG((EFI_D_INFO, "Write I/O Expander, I2C Bus:%d, slave address:%d, register:%d, Value:0x%x\n", \
         IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, OUTPUT_REGISTER_2, Data8));
    Status = ByteWriteI2C(IO_EXPANDER_I2C_BUS_NO, IO_EXPANDER_SLAVE_ADDR, OUTPUT_REGISTER_2, 1, &Data8); 
    DEBUG((EFI_D_INFO, "Set pin 14 to output ==%r\n", Data8));
  } else {
    Status = EFI_NO_RESPONSE;
  }

  return Status;
}
EFI_STATUS
EFIAPI
EnableULPI (BOOLEAN bTurnOn)
{
  EFI_STATUS   Status;
  UINT32 mmio_padval=0x00;

  //GPIO_INIT_ITEM("PMU_SLP_LAN_B     GPIOS_14 "     ,GPO      ,HI           ,F1           ,             ,                ,20K_L      ,0x11),//ULPI_RESET_N
  //ULPI_RESET_N pin
  // Get GPIO  padval of ID(gpi) GPIOS_14
  // Calculate the MMIO Address for specific GPIO pin  PAD_VAL register pointed by index.
  mmio_padval= IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x118;//0x110+8//R_PCH_CFIO_PAD_VAL
  if (bTurnOn == TRUE){
    Status = MmioWrite32(mmio_padval, 1);//set ouput to high
  }else{
    Status = MmioWrite32(mmio_padval, 0);//set ouput to low
  }
  return Status;
}
/**
  Timer handler for SMSC3750 to enable USB OTG/HOST mode

  @param  Event                     Indicates the event that invoke this function.
  @param  Context                  Indicates the calling context.
**/
VOID
EFIAPI
Smsc3750TimerHandler (
  IN  EFI_EVENT                 Event,
  IN  VOID                      *Context
  )
{
  UINT8        Value;
  EFI_STATUS   Status;

  UINT32 PadValue = 0x00;
  UINT32 mmio_padval;

  // USB ID pin
  // Get GPIO  padval of ID(gpi) GPIO_DFX_04/GPIOS_26
  // Calculate the MMIO Address for specific GPIO pin  PAD_VAL register pointed by index.
  mmio_padval = IO_BASE_ADDRESS + GPIO_SSUS_OFFSET + 0x168;//0x16*16+8//R_PCH_CFIO_PAD_VAL
  PadValue = MmioRead32 (mmio_padval) & 0x1;
  if((PadValue == CurrentState) && (PadValue == 0) && (!I2cERROR)){//attached device
    return;
  } else {
    CurrentState = PadValue;
    DEBUG ((DEBUG_INFO, "MmioRead32 GPIO_DFX_04/GPIOS_26's mmio_padval:%x = %x\n",mmio_padval,PadValue)); 
  }
  if (CurrentState == 0) {//host mode to turn on Vbus
    Status = VbusControl(TRUE);
    if (EFI_ERROR(Status)){
      I2cERROR = TRUE;
      return;
    }
    MicroSecondDelay(200);//delay for switch	
    Status = SmscEnableHostOrOtgMode(HOST_MODE);
    if (EFI_ERROR(Status)){
      I2cERROR = TRUE;
      return;
    }
    I2cERROR = FALSE;
  } else {//SDP & charger & empty

    Status = ByteReadI2C(SMSC3750_I2C_BUS_NO,
                         SMSC3750_I2C_SLAVE_ADDR,
                         SMSC3750_STATUS_REG,
                         1,
                         &Value
                         );
    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_ERROR, "ByteReadI2C Addr 0x%x, 0ffset 0x%x, Status %r\n", 
              SMSC3750_I2C_SLAVE_ADDR, SMSC3750_STATUS_REG, Status));
      I2cERROR = TRUE;
      return;
    }  
    if ((Value & CHG_DET_COMPLETE_MASK) != CHG_DET_COMPLETE_DONE) {
      DEBUG ((DEBUG_INFO, "Charger detect not completed!\n"));
      return;
    }
    if (((Value & CHARGER_TYPE_MASK) == ChargerFlg) && !I2cERROR ) {// no change
      return;
    } else {
      ChargerFlg = (Value & CHARGER_TYPE_MASK);
    }
    //Charger            GPIOC3  GPIOC2  Comment
    //SE1-low           Low        Low       100mA
    //SDP/SE1-high  Low        High       500mA
    //CDP/DCP         HIGH       Low       3.0A
    switch (ChargerFlg) {
      case CHARGER_TYPE_SEL1_LOW:
        DEBUG ((DEBUG_INFO, "SMSC3750 enable mux for Device Mode, charger: 0x%x!\n", ChargerFlg));
        MmioWrite32(IO_BASE_ADDRESS + 0x5D8, 0);//GPIOC_2 low
        MmioWrite32(IO_BASE_ADDRESS + 0x608, 0);//GPIOC_3 low
        break;
      case CHARGER_TYPE_SDP:
      case CHARGER_TYPE_SEL1_HIGH:
        DEBUG ((DEBUG_INFO, "SMSC3750 enable mux for Device Mode, charger: 0x%x!\n", ChargerFlg));
        MmioWrite32(IO_BASE_ADDRESS + 0x5D8, 1);//GPIOC_2 high
        MmioWrite32(IO_BASE_ADDRESS + 0x608, 0);//GPIOC_3 low
        break;
      case CHARGER_TYPE_DCP:
      case CHARGER_TYPE_CDP:
        DEBUG ((DEBUG_INFO, "SMSC3750 enable mux for Device Mode, charger: 0x%x!\n", ChargerFlg));
        MmioWrite32(IO_BASE_ADDRESS + 0x5D8, 0);//GPIOC_2 low
        MmioWrite32(IO_BASE_ADDRESS + 0x608, 1);//GPIOC_3 high
        break;
   }    

   Status = VbusControl(FALSE);
   if (EFI_ERROR(Status)){
     I2cERROR = TRUE;
     return;
   }
    MicroSecondDelay(50);//delay for switch
   Status = SmscEnableHostOrOtgMode(OTG_MODE);
   if (EFI_ERROR(Status)){
     I2cERROR = TRUE;
     return;
   }
    I2cERROR = FALSE;
  }

  return;
}

EFI_STATUS
EFIAPI
DisableSmsc3750Timer (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = gBS->SetTimer (
                  mMuxEvent, 
                  TimerCancel, 
                  0 // 1s //200000=0.02s
                  );

  return Status;
}

EFI_STATUS
EFIAPI
Smsc3750Init (
  VOID
  )
{
  EFI_STATUS               Status;

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  Smsc3750TimerHandler,
                  NULL,
                  &mMuxEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->SetTimer (
                  mMuxEvent, 
                  TimerPeriodic, 
                  10000000 // 1s //200000=0.02s
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}


EFI_STATUS
EFIAPI
Smsc3750LibEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  mMuxEvent = NULL;
  ChargerFlg = 0;
  return EFI_SUCCESS;
}

VOID 
Smsc3750DeviceMode(
  VOID
  )
{
  SmscEnableHostOrOtgMode(OTG_MODE);
}

