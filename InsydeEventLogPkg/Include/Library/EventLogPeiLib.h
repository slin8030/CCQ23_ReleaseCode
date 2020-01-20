/** @file

  Header file of Event Log PEI Lib implementation.

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

#ifndef _EVENT_LOG_PEI_LIB_H_
#define _EVENT_LOG_PEI_LIB_H_

/**
Log event data in PEI phase by Pei Event Log PPI. 
        
@param[in] PeiServices     Efi Pei services.
@param[in] EventID         Event ID of the event data. Prototype: EVENT_TYPE_ID
@param[in] DataSize        Size of event data.
@param[in] *LogData        Event data which will be logged.
 
@retval EFI Status                  
*/
EFI_STATUS
EFIAPI
PeiLogEventLib (
 IN  CONST EFI_PEI_SERVICES                  **PeiServices,
 IN  VOID                                    *EventID,
 IN  UINTN                                   DataSize,
 IN  UINT8                                   *LogData
 );

#endif
