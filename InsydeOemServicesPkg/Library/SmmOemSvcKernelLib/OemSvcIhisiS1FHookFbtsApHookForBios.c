/** @file
 This function offers an interface to Hook IHISI Sub function AH=1Fh,function "FbtsApHookForBios"

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

#include <Library/SmmOemSvcKernelLib.h>

/**
 This function provides a hook for IHISI sub-function AH=1Fh and should be implemented to do AP Hook for BIOS.
 Please refer to Insyde H2O internal Soft-SMI interface (IHISI) Specification.

 @param[in]         ApState             denote the start of AP process.
                                        ApState = 0x00, AP terminate. (Before IHISI 0x16)
                                        ApState = 0x01, AP start. (After IHISI 0x10)
                                        ApState = 0x02, Start to read ROM. (Before IHISI 0x14)
                                        ApState = 0x03, Start to write ROM. (Before IHISI 0x15)
                                        ApState = 0x04, Start to write EC. (Before IHISI 0x20)
                                        ApState = 0x05, Before dialog popup.
                                        ApState = 0x06, After dialog close and continue running.

 @retval            EFI_UNSUPPORTED     Returns unsupported by default.
 @retval            EFI_MEDIA_CHANGED   Alter the Configuration Parameter.
 @retval            EFI_SUCCESS         The function performs the same operation as caller.
                                        The caller will skip the specified behavior and assuming
                                        that it has been handled completely by this function.
*/
EFI_STATUS
OemSvcIhisiS1FHookFbtsApHookForBios (
  IN UINT8          ApState
  )
{
/*++
  Todo:
  Add project specific code in here.
--*/

  return EFI_UNSUPPORTED;
}
