/** @file
  UsbPlatformPolicyUpdatePei.c

;******************************************************************************
;* Copyright (c) 2015, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _USB_PLATFORM_POLICY_UPDATE_PEI_LIB_H_
#define _USB_PLATFORM_POLICY_UPDATE_PEI_LIB_H_

EFI_STATUS
EFIAPI
UpdatePeiUsbPlatformPolicy (
  IN CONST    EFI_PEI_SERVICES        **PeiServices,
  IN OUT      SC_USB_POLICY_PPI      *ScUsbPolicyPpi
  )
/*++

Routine Description:

  This function performs Usb PEI Platform Policy initialzation.

Arguments:

  PeiServices             General purpose services available to every PEIM.
  ScUsbPolicyPpi    The Usb Platform Policy PPI instance

Returns:

  EFI_SUCCESS             The PPI is installed and initialized.
  EFI ERRORS              The PPI is not successfully installed.
  EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver

--*/
;
#endif
