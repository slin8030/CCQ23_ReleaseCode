//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#include <BatteryDriver.h>
#include <Ulpmc.h>

//
//Function reads current state of charge from Ulpmc. Returns a word value which corresponds to charge in %ge.
//

UINT16 
UlpmcGetStateofCharge(
VOID				
)
{
	EFI_STATUS		Status = EFI_SUCCESS;
	UINT16			Value = 0;
	UINT8			*Buffer = NULL;

	Buffer = AllocateZeroPool(2);
	if(Buffer == NULL){
		DEBUG ((EFI_D_INFO, "UlpmcGetStateofCharge() AllocateZeroPool Failed\n"));
		return Value;
	}

	Status = ReadByteUseEfiI2CBusProtocol (UlpmcControllerId, UlpmcSlaveAddress, StateOfChargeC, 2, Buffer); 
	if (EFI_ERROR (Status)) {
		DEBUG ((EFI_D_INFO, "UlpmcGetStateofCharge() ReadByteUseEfiI2CBusProtocol Failed - Status - %x\n", Status));
	}else{
	//Print(L"\nReadByteUseEfiI2CBusProtocol() success!\n");
	Value = Buffer[0] | (Buffer[1] << 8);
	}

	FreePool(Buffer);

	return Value; 
}

//
//Function reads charging status from Ulpmc. Returns a byte value.
//

/*
Return value identifiation:
0 - No input
1 - USB
2 - A/C wall charger
3 - OTG
*/

UINT8 
UlpmcGetChargingStatus(
VOID
)
{
	EFI_STATUS		Status = EFI_SUCCESS;
	UINT8			Value = 0;
	UINT8			*Buffer = NULL;

	Buffer = AllocateZeroPool(2);
	if(Buffer == NULL){
		DEBUG ((EFI_D_INFO, "UlpmcGetChargingStatus() AllocateZeroPool Failed\n"));
		return Value;
	}
	
	/*
	Status = SendCommandToUlpmcForChargerRead(UlpmcControllerId, UlpmcSlaveAddress, 0x3A, 2, Buffer);
	if (EFI_ERROR (Status)) {
	DEBUG ((EFI_D_INFO, "UlpmcGetChargingStatus() SendCommandToUlpmcForChargerRead Failed - Status - %x\n", Status));
	}else{
	Print(L"\nSendCommandToUlpmcForChargerRead() success!\n");
	}
	*/

	Status = ReadByteUseEfiI2CBusProtocol (UlpmcControllerId, UlpmcSlaveAddress, ChargingStatusC, 2, Buffer);
	if (EFI_ERROR (Status)) {
	DEBUG ((EFI_D_INFO, "UlpmcGetChargingStatus() ReadByteUseEfiI2CBusProtocol Failed - Status - %x\n", Status));
	}else{
	//Print(L"\nReadByteUseEfiI2CBusProtocol() success!\n");
	}
	
	Value = (Buffer[0] & 0xC0)	 >> 6;

	FreePool(Buffer);

	return Value; 
}

