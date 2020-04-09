/** @file

Device Protocol

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SioInitPei.h"

EFI_STATUS
WriteOnOff (
  UINT16            Indexport,
  UINT8             Ldn, 
  BOOLEAN           OnOff
  )
{
  IoWrite8 (Indexport , SIO_LDN);
  IoWrite8 (Indexport + 1, SIO_WDT);
  IoWrite8 (Indexport , SIO_DEV_ACTIVE);
  IoWrite8 (Indexport + 1, OnOff);

  return EFI_SUCCESS;
}

EFI_STATUS
WriteMode (
  UINT16            Indexport,
  UINT8             Ldn, 
  UINT8             Mode
  )
{
  UINT8     Value;
  UINT8     Bit3;

  Bit3 = 0x08;

  IoWrite8 (Indexport , SIO_LDN);
  IoWrite8 (Indexport + 1, SIO_WDT);
  IoWrite8 (Indexport , SIO_WDT_CONTROL_MODE_REG);
  Value = IoRead8 (Indexport + 1);

  if (Mode == 0x00) {
    IoWrite8 (Indexport + 1, Value & ~Bit3);
  } else {
    IoWrite8 (Indexport + 1, Value | Bit3);
  }

  return EFI_SUCCESS;
}


EFI_STATUS
WriteCounter (
  UINT16            Indexport,
  UINT8             Ldn, 
  UINT16            Counter
  )
{
  IoWrite8 (Indexport , SIO_LDN);
  IoWrite8 (Indexport + 1, SIO_WDT);
  IoWrite8 (Indexport , SIO_WDT_COUNTER_REG);
  IoWrite8 (Indexport + 1, (UINT8)(Counter & 0xFF));

  return EFI_SUCCESS;  
}

EFI_STATUS
WriteReg (
  UINT16            Indexport,
  UINT8             Ldn, 
  UINT8             Offset,
  UINT8             Reg  
  )
{
  IoWrite8 (Indexport , Offset);
  IoWrite8 (Indexport + 1, Reg);
  
  return EFI_SUCCESS;
}

