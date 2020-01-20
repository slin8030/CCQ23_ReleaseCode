/** @file
  Header file for PeiCapsuleLib Library Class

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

#ifndef _PEI_CAPSULE_LIB_H_
#define _PEI_CAPSULE_LIB_H_

/**
  Check secure flash image under S3 boot mode

  @param[in]  None

  @retval   Returns TRUE if pending capsule existed

**/
BOOLEAN
DetectPendingUpdateImage (
  VOID
  );

#endif
