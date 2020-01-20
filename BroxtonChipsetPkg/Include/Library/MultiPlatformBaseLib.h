/** @file
;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
  
#ifndef _MULTI_PLATFORM_BASE_LIB_H_
#define _MULTI_PLATFORM_BASE_LIB_H_

#include <PiDxe.h>

//[-start-161022-IB07400803-modify]//
BOOLEAN 
IsIOTGBoardIds (
  VOID
  );
//[-end-161022-IB07400803-modify]//

VOID
MultiPlatformDetectPlatformType (
  VOID
  );

//[-start-161107-IB07400810-add]//
UINT8 
MultiPlatformGetBoardIds (
  VOID
  );
//[-end-161107-IB07400810-add]//

#endif
