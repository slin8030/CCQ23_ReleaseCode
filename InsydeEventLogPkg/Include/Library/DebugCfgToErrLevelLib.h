/** @file

  Header file of Debug Config to Debug Print Error Level Lib implementation.

;******************************************************************************
;* Copyright (c) 2016, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _DEBUG_CFG_TO_ERR_LEVEL_LIB_H_
#define _DEBUG_CFG_TO_ERR_LEVEL_LIB_H_

//[-start-160218-IB08400332-modify]//
/**
  Translate the Debug Message configurate to Debug Print Error Level value.
  
  @param   DebugMsgCfg         Debug config setting.
  
  @retval  DebugPrintErrLevel  The debug print error level value.

**/
UINT32
EFIAPI
DebugCfgToErrLevel (
  IN VOID   *DebugCfg
  );
//[-end-160218-IB08400332-modify]//

//[-start-160219-IB08400332-modify]//
/**
  Translate the Debug Print Error Level value to Debug Message configuration.
  
  @param[in]     DebugPrintErrLevel  The debug print error level value.
  @param[in,out] DebugCfg            Debug config setting.  
  @retval      

**/
VOID
EFIAPI
ErrLevelToDebugCfg (
  IN     UINT32       DebugPrintErrLevel,
  IN OUT VOID         *DebugCfg
  );
//[-end-160219-IB08400332-modify]//



#endif
