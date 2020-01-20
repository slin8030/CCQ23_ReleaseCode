/** @file
 H2O IPMI SEL Data library implement code.

 This c file contains SEL Data library instance in DXE phase.

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

//[-start-170524-IB09330350-modify]//
/**
 Get Sel Data of SelId.

 @param[in]         SelId               The SEL record ID to get which sel data.
 @param[in]         IsHeci              Sent to Heci or not
 @param[out]        SelData             A buffer to H2O_IPMI_SEL_ENTRY.

 @retval EFI_SUCCESS                    Get SEL data success.
 @retval EFI_UNSUPPORTED                Get SEL data fail or speificed Id does not exist.
*/
EFI_STATUS
IpmiLibGetSelData2 (
  IN  UINT16                            SelId,
//[-start-170718-IB09330369-add]//
  IN  BOOLEAN                           IsHeci,
//[-end-170718-IB09330369-add]//
  OUT VOID                              *SelData
  )
{
  return EFI_UNSUPPORTED;
}


/**
 Get All Sel entry data.

 @param[out]        SelCount            Number of log entries in SEL.
 @param[out]        SelData             A H2O_IPMI_SEL_ENTRY array to store all H2O_IPMI_SEL_ENTRY.

 @retval EFI_SUCCESS                    Get SEL data success.
 @retval EFI_UNSUPPORTED                Get SEL data fail.
*/
EFI_STATUS
IpmiLibGetAllSelData2 (
//[-start-170718-IB09330369-modify]//
  IN  BOOLEAN                           IsHeci,
  OUT UINT16                            *SelCount,
//[-end-170718-IB09330369-modify]//
  OUT VOID                              **SelData
  )
{
  return EFI_UNSUPPORTED;
}
//[-end-170524-IB09330350-modify]//

