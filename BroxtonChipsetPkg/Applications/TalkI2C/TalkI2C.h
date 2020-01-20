//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#ifndef __TALKI2C_H__
#define __TALKI2C_H__

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
    UINTN ReadLength;     //Read request length. If 0, must be NULL.
    UINT8 WriteLength;
    UINT8  BusMode;
    BOOLEAN SuppressStopBit;
    UINT8 *WriteBuffer;
}I2CCommandParam, *PI2CCommandParam;

#endif
