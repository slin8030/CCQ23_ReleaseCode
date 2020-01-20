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
#ifndef _FSA_LIB_H
#define _FSA_LIB_H
#include <Uefi.h>
#include <Uefi/UefiSpec.h>

 
#define FSA_REG_DEVID           0x1
#define FSA_REG_CTRL            0x2
#define FSA_REG_INTR            0x3
#define FSA_REG_INTR_MSK        0x5
#define FSA_REG_RESISTOR_CODE   0x7
#define FSA_REG_TIMING_SET      0x8
#define FSA_REG_STATUS          0x9
#define FSA_REG_DEV_TYPE        0xA
#define FSA_REG_DAC_SAR         0xB
#define FSA_REG_MANUAL_SW       0x13
#define FSA_REG_MANUAL_CHG_CTRL 0x14
 
extern
EFI_STATUS
EFIAPI
FsaUsbDeviceMode (
  VOID
);

extern
EFI_STATUS
EFIAPI
MuxUsbDeviceMode (
  VOID
);

extern
EFI_STATUS
EFIAPI
MuxUsbHostMode (
  VOID
);

extern
EFI_STATUS
EFIAPI
DisableFsaTimerHandler (
  VOID
);  

extern
EFI_STATUS
EFIAPI
FSAInit (
 IN UINT32 FFRDVer  
);
#endif
