/** @file

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
#ifndef _IODECODELIB_H_
#define _IODECODELIB_H_

/**
 IO address decode

 @param[in]         Address             Decode address
 @param[in]         Length              Decode length
                                
 @retval            EFI_SUCCESS           Function returns successfully
 @retval            EFI_INVALID_PARAMETER One of the parameters has an invalid value
                                          1. Address big then 0xFFFF
                                          2. Length need to be 4-byte aligned
 @retval            EFI_OUT_OF_RESOURCES  There are not enough resources available to set IO decode
 @retval            EFI_UNSUPPORTED       Null library
*/
EFI_STATUS
IoDecodeControl (
  IN UINT16                    Address,
  IN UINT16                    Length
  );
#endif  