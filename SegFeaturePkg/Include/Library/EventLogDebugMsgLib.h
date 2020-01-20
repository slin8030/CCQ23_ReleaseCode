/** @file

  Header file of Event Log Get Debug message Lib implementation.

;******************************************************************************
;* Copyright (c) 2017, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _EVENT_LOG_GET_DEBUG_MSG_LIB_H_
#define _EVENT_LOG_GET_DEBUG_MSG_LIB_H_

EFI_STATUS
GetDebugMessageBufferInfo (
  OUT  UINT32     *AdmgStartAddr,
  OUT  UINT32     *AdmgEndAddr,
  OUT  UINT32     *AdmgSize,  OPTIONAL
  OUT  UINT32     *TotalSize  OPTIONAL
  );

#endif

