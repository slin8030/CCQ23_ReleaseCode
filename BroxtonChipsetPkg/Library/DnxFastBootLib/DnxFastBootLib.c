/** 
Copyright (c) 2013, Intel Corporation. All rights reserved. This
software and associated documentation (if any) is furnished 
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation. 
**/          


#include <Library/DnxFastBootLib.h>

//
// Launches the OSIP image
//

EFI_STATUS
EFIAPI
FastbootStartImage (
	IN UINT8	*Buffer
	)
{
    InstallLegacyAcpi();
    BdsBootAndroidFromEmmc (Buffer);
	return EFI_SUCCESS;
}

