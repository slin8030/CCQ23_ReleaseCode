/** @file
This file is SampleCode for Intel Silicon DXE Platform Policy initialzation.

@copyright
Copyright (c) 2015 - 2016 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor.  This file may be modified by the user, subject to
the additional terms of the license agreement
**/
#include <Library/BaseLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Guid/StatusCodeDataTypeId.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/SiPolicyProtocol.h>

DXE_SI_POLICY_PROTOCOL mSiPolicyData = { 0 };

/**
Initilize Intel Cpu DXE Platform Policy

@param[in] ImageHandle        Image handle of this driver.

@retval EFI_SUCCESS           Initialization complete.
@exception EFI_UNSUPPORTED    The chipset is unsupported by this driver.
@retval EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
@retval EFI_DEVICE_ERROR      Device error, driver exits abnormally.
**/
EFI_STATUS
EFIAPI
InitSiPolicy(
)
{
	EFI_STATUS Status;
	EFI_HANDLE ImageHandle;

	ImageHandle = NULL;
	mSiPolicyData.Revision = DXE_SI_POLICY_PROTOCOL_REVISION_1;

	///
	/// Install the DXE_SI_POLICY_PROTOCOL interface
	///
	Status = gBS->InstallMultipleProtocolInterfaces(
		&ImageHandle,
		&gDxeSiPolicyProtocolGuid,
		&mSiPolicyData,
		NULL
		);
	ASSERT_EFI_ERROR(Status);
	return Status;
}
