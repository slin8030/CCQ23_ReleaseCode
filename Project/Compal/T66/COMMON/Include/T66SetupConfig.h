/** @file

;******************************************************************************
;* * (C) Copyright 2016 Compal Electronics, Inc.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
//----------------------------------------------------------------------------------------------------------
// Start area for Compal common define. The following area is used by compal common to modify.
// The total size of variable in this part are fixed (20bytes). That means if you need to add or remove
// variables, please modify the OEMRSV buffer size as well.
//----------------------------------------------------------------------------------------------------------
//T66_SetupConfig_Start
#ifdef _IMPORT_T66_COMMON_SETUP_
//PRJ+ >>>> Hidden T66 unsupported items in SCU and add T66 showAllPage function
  UINT8         T66ShowSCU;
//PRJ+ <<<< Hidden T66 unsupported items in SCU and add T66 showAllPage function  
  UINT8         T66CommonRSV[29];                    // Reserve
#endif
