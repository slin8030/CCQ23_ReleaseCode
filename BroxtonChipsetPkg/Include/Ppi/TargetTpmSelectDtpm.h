/** @file
  This PPI is installed when target TPM select dTPM. 

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

#ifndef __TARGET_TPM_SELECT_DTPM_PPI_H__
#define __TARGET_TPM_SELECT_DTPM_PPI_H__

#define TARGET_TPM_SELECT_DTPM_PPI { 0x814DBC9C, 0xF775, 0x4292, { 0x98, 0x71, 0x52, 0x6d, 0x3b, 0xb1, 0x85, 0xe2 } }



extern EFI_GUID gTargetTpmSelectDtpmPpiGuid;

#endif
