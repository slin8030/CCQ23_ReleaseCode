/** @file

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _SAVE_UNLOCKED_BAR_VARIABLE_H_
#define _SAVE_UNLOCKED_BAR_VARIABLE_H_

#define _SAVE_UNLOCKED_BAR_VARIABLE_GUID \
  { \
    0x39473DE5, 0xDF3B, 0x49A1, 0x9F, 0xA6, 0x41, 0xB3, 0x5B, 0x36, 0xFA, 0x39 \
  }

#define FIXED_BAR_VARIABLE_NAME    L"FixedBar"
#define DYNAMIC_BAR_VARIABLE_NAME  L"DynamicBar"
     
extern EFI_GUID gSaveUnLockedBarVariableGuid;

typedef struct {
	UINT8               Bus;	  
	UINT8               Dev;	  
	UINT8               Func;  
	UINT8               Offset; 
	UINT32              Bar;		            
} DEVICE_BASE_ADDRESS;

#endif
