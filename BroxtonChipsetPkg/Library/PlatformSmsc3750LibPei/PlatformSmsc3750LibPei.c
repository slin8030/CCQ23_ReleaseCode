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
#include <PlatformBaseAddresses.h>
#include <library/I2CLib.h>
#include <Library/PlatformSmsc3750Lib.h>
#include <Library/PmicLib.h>
#include "PiPei.h"
#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>

/**
  Turn on or off VBUS for OTG

  @param  bTurnOn    TRUE-turn on VBUS   FALSE-turn off VBUS

**/
#define IO_EXPANDER_I2C_BUS_NO  0x00
#define IO_EXPANDER_SLAVE_ADDR  0x23
#define INPUT_REGISTER          2
#define OUTPUT_REGISTER_2       5
#define CONFIG_REGISTER_2       13

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
FsaLibEntryPoint (
  IN EFI_HANDLE        _ImageHandle,
  IN EFI_SYSTEM_TABLE  *_SystemTable
  )
{
  return EFI_SUCCESS;  
}

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

