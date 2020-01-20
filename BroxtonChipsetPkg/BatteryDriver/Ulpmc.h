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

//
//Function reads current state of charge from Ulpmc. Returns a word value which corresponds to charge in %ge.
//

UINT16 
UlpmcGetStateofCharge(
VOID				
);

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
);

EFI_STATUS 
SendCommandToUlpmcForChargerRead (
UINT8 BusNo, 
UINT8 Slaveaddress, 
UINT8 Offset, 
UINTN ReadBytes, 
UINT8 *BufferForReadData
);

EFI_STATUS 
ReadByteUseEfiI2CBusProtocol (
UINT8 BusNo, 
UINT8 Slaveaddress, 
UINT8 Offset, 
UINTN ReadBytes, 
UINT8 *BufferForReadData
);

#define UlpmcI2CCommunicationC 0x54
#define UlpmcProgrammedC 0x54
#define FGI2CCommunicationC 40
#define FullChargeCapacityOfBatteryC 18
#define StateOfChargeC 44
#define RemainingCapacityOfBatteryC 16
#define BatteryVoltageC 8
#define BatteryChargeDischargeCurrentC 0x14
#define DesignCapacityC 40
#define FgFwVersionC 0x48
#define FgDataFlashVersionC 0x3B

#define Bat1ConnectivityC 0x4E
#define Bat2ConnectivityC 0x4E

#define ChargerI2CCommunicationC 0x47
#define VsysVoltageC 0x3E
#define InputCurrentLimitC 0x3D
#define Bat1TempC 0x46
#define Bat2TempC 0x46
#define ChargingStatusC 0X2E
#define FastChargeCurrentLimitC 0x3F
#define PreChargeCurrentLimitC 0x40
#define TerminationCurrentLimitC 0x40
#define ChargeVoltageLimitC 0x41
#define SkinTempC 52

#define UlpmcControllerId 0
#define UlpmcSlaveAddress 120

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/Pmiclib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
//#include "PnPVerifierByt.h"
//#include "PnPAppVersions.h"

//#include <WriteFile.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellParameters.h>
//#include <Print.h>


#include <IndustryStandard/Pci22.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
//#include "VlvAccess.h"
//#include <Guid/SetupVariable.h>
#include <Library/UefiRuntimeServicesTableLib.h>
//#include <Protocol/SetupMode.h>


#include <Library/UefiLib.h>
#include <Library/BaseLib.h>



#define I2C_MODE_NORMAL 0
#define I2C_MODE_FAST 1
#define I2C_MODE_HIGH 2

#define I2C_ACPI_ID_LEN 16   //I2C0X\\SFFFF\\XXXK
#define I2C_ADDRESS_MAX 0x3FE
#define I2C_TIMEOUT_DEFAULT 1000
#define I2C_READ_MAX 1024*1024*10+1   //Defined by the tool itself for maximum read length.

#define DID_ACPI_ID_PREFIX   "I2C0"
#define DID_ACPI_ID_SUFFIX   "\\SFFFF"
#define DID_ACPI_ID_SUFFIX_100K "\\100K"
#define DID_ACPI_ID_SUFFIX_400K "\\400K"

typedef struct _I2CCommandParam{
    UINT8 ControllerID;    //Controller ID 
    UINT16 SlaveAddress;   //Device's slave address
    UINT32 ReadLength;     //Read request length. If 0, must be NULL.
    UINT8 WriteLength;
    UINT8  BusMode;
    BOOLEAN SuppressStopBit;
    UINT8 *WriteBuffer;
}I2CCommandParam, *PI2CCommandParam;


EFI_STATUS 
ReadByteUseEfiI2CBusProtocol (UINT8 BusNo, UINT8 Slaveaddress, UINT8 Offset, UINTN ReadBytes, UINT8 *BufferForReadData);
