/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  IsctAcpiTableStorage.h
    
Abstract:

  GUID definition for the Isct ACPI table storage file name

--*/
#ifndef _RESERVED_NVS_ACPI_TABLE_STORAGE_H_
#define _RESERVED_NVS_ACPI_TABLE_STORAGE_H_
//B5BB9F06-3F32-4db2-B910-B428464CC8EC
#define RESERVED_NVS_ACPI_TABLE_STORAGE_GUID \
  { \
    0xB5BB9F06, 0x3F32, 0x4db2, 0xB9, 0x10, 0xB4, 0x28, 0x46, 0x4C, 0xC8, 0xEC \
  }

extern EFI_GUID gReservedNvsAcpiTableStorageGuid;

#endif