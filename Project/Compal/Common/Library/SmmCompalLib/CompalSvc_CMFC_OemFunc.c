/** @file
  GPNV is a persistent general-purpose storage area managed by SMBIOS.
  The attribute of GPNV is OEM - specific that the format and location of GPNV are not defined in SMBIOS specification.
  This OemServiece provides OEM to define the handle parameter which is passed into GPNV function (55H, 56H, and 57H) to access specific GPNV area.
  OEM can define accessible GPNV area by modifying the handle value of the UpdateableGpnvs Table.

;******************************************************************************
;* Copyright (c) 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <Library/SmmCompalSvcLib.h>

/**
  Update CMFC Oem function table.

  @param[IN ]  *CompalGlobalNvsArea     Pointer to CompalGlobalNvsArea.
  @param[IN ]  *SetupVariable           Pointer to SetupVariable.
  @param[IN ]  *FB                      Pointer to CMFC function number .

  @retval      EFI_UNSUPPORTED       Returns unsupported by default.
  @retval      EFI_SUCCESS           Get Updatable string success.
  @retval      EFI_MEDIA_CHANGED     The value of IN OUT parameter is changed.
  @retval      Others                Base on OEM design.
**/
EFI_STATUS
CompalSvc_CMFC_OemFunc (
  IN COMPAL_GLOBAL_NVS_AREA                  *CompalGlobalNvsArea,
  IN SYSTEM_CONFIGURATION                    *SetupVariable,
  IN OEMFunctionBuffer                       *FB
  )
{
  return EFI_UNSUPPORTED;
}
