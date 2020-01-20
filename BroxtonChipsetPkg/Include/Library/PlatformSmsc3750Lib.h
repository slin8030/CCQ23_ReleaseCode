/*++
Copyright (c) 2013 Intel Corporation. All rights reserved.
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
--*/
#ifndef _SMSC3750_LIB_H
#define _SMSC3750_LIB_H
#include <Uefi.h>
#include <Uefi/UefiSpec.h>

#define SMSC3750_I2C_BUS_NO        0x00
#define SMSC3750_I2C_SLAVE_ADDR    0x68

#define SMSC3750_STATUS_REG        0x00
#define CHG_DET_COMPLETE_MASK      BIT4
#define CHG_DET_COMPLETE_DONE      BIT4
#define CHARGER_TYPE_MASK          (BIT5 | BIT6 | BIT7)
#define CHARGER_TYPE_DCP           BIT5
#define CHARGER_TYPE_CDP           BIT6
#define CHARGER_TYPE_SDP           (BIT5 | BIT6)
#define CHARGER_TYPE_SEL1_LOW      BIT7
#define CHARGER_TYPE_SEL1_HIGH     (BIT5 | BIT7)

#define SMSC3750_CONFIG_REG        0x01
#define ENABLE_MUX1                BIT5
#define ENABLE_MUX2                BIT6

#define SMSC3750_BATT_CHG_REG      0x02

#define SMSC3750_BATT_CHG_STS_REG  0x03

#define HOST_MODE                  0x01
#define OTG_MODE                   0x02

extern
EFI_STATUS
EFIAPI
DisableSmsc3750Timer (
  VOID
);  

extern
EFI_STATUS
EFIAPI
Smsc3750Init (
 VOID 
);

EFI_STATUS
EFIAPI
SmscEnableHostOrOtgMode (
  IN UINT8     Mode
);

EFI_STATUS
EFIAPI
VbusControl (
  BOOLEAN bTurnOn
);
EFI_STATUS
EFIAPI
EnableULPI (
  BOOLEAN bTurnOn
);
#endif
