/** @file
  Thunk 64 to 32 library header file
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

#ifndef _THUNK_64_TO_32_LIBRARY_H_
#define _THUNK_64_TO_32_LIBRARY_H_


/**
  Do thunk 64 to 32 and jmp to run code.

  @param[in]   PeiServicesPoint   Pei Services Point
  @param[in]   FunctionPoint      Function Point
  
  @retval EFI_SUCCESS     The thunk completed successfully.
  @retval EFI_NOT_READY   The thunk constructor function has not been performed
  
**/
EFI_STATUS
EFIAPI
Thunk64To32 (
  IN UINT32        PeiServicesPoint,
  IN UINT32        FunctionPoint
  );


#endif
