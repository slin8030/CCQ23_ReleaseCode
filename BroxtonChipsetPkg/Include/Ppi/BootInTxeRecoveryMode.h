/** @file
  This PPI is installed by the platform PEIM to designate that a TXE recovery boot 
  is in progress.

;******************************************************************************
;* Copyright (c) 2013-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef __BOOT_IN_TXE_RECOVERY_MODE_PPI_H__
#define __BOOT_IN_TXE_RECOVERY_MODE_PPI_H__

#define EFI_PEI_BOOT_IN_TXE_RECOVERY_MODE_PPI { 0xF6A4BABE, 0xF03C, 0x4CCB, {0x87, 0xA4, 0x07, 0x91, 0x57, 0x6D, 0x1F, 0x87}}


extern EFI_GUID gEfiPeiBootInTxeRecoveryModePpiGuid;

#endif
