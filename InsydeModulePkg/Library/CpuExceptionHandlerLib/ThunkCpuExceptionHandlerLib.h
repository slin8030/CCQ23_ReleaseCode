/** @file
  CPU Exception Handler library header file
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

#ifndef _THUNK_CPU_EXCEPTION_HANDLER_H_
#define _THUNK_CPU_EXCEPTION_HANDLER_H_

#include <Library/CpuExceptionHandlerLib.h>
#include <Library/Thunk64To32Lib.h>
#include <Library/BaseLib.h>
#include <Library/HobLib.h>
#include <Guid/PeiTimerDataHob.h>

/**
  Type of function point

  @param[in] None

  @retval None.
**/
typedef
VOID
(EFIAPI *INTERRUPT_CALLBACK)(
  VOID
  );

/**
  Assembly code of interrupt service routine.

  @param[in] None

  @retval None.
**/
VOID
InterruptEntry (
  VOID
  );

/**
 Save callback function pointer to assembley variable

 @param[in]   CallBackFunction   Call backe function point

 @retval None.

**/
VOID
InitializeTimerCallbackPtr (
  INTERRUPT_CALLBACK CallBackFunction
  );

#endif
