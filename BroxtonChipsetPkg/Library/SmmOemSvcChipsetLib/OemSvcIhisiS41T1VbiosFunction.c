/** @file
  This function offers an interface to do IHISI Sub function AH=41h,
  OEM Extra Data Communication type 1h for VBIOS relative function.
;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmOemSvcChipsetLib.h>

/**
 This function offers an interface to do IHISI sub-function AH=41h, OEM Extra Data Communication 
 type 1h for VBIOS relative function. Please refer to Insyde H2O internal Soft-SMI interface (IHISI) 
 Specification.

 @param[in]         ApCommDataBuffer    Pointer to AP communication data buffer.
 @param[in, out]    BiosCommDataBuffer  On entry, pointer to BIOS communication data buffer.
                                        On exit, points to updated BIOS communication data buffer.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS41T1VbiosFunction (
  IN AP_COMMUNICATION_DATA_TABLE        *ApCommDataBuffer,
  IN OUT BIOS_COMMUNICATION_DATA_TABLE  *BiosCommDataBuffer
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/

  return EFI_UNSUPPORTED;
}
