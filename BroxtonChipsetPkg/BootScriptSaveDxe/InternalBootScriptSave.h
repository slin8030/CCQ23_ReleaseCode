//
// This file contains a 'Sample Driver' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may be modified by the user, subject to  
// the additional terms of the license agreement               
//
/** @file
  Internal header file for S3 Boot Script Saver driver.

  Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef _INTERNAL_BOOT_SCRIPT_SAVE_H_
#define _INTERNAL_BOOT_SCRIPT_SAVE_H_
#include <FrameworkDxe.h>

#include <Protocol/BootScriptSave.h>
//[-start-151228-IB03090424-modify]//
#include <Protocol/FirmwareVolume2.h>
//[-end-151228-IB03090424-modify]//

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/PcdLib.h>
#include <Library/SmbusLib.h>
#include <IndustryStandard/SmBus.h>
/**
  Adds a record into a specified Framework boot script table.

  This function is used to store a boot script record into a given boot
  script table. If the table specified by TableName is nonexistent in the 
  system, a new table will automatically be created and then the script record 
  will be added into the new table. A boot script table can add new script records
  until EFI_BOOT_SCRIPT_SAVE_PROTOCOL.CloseTable() is called. Currently, the only 
  meaningful table name is EFI_ACPI_S3_RESUME_SCRIPT_TABLE. This function is
  responsible for allocating necessary memory for the script.

  This function has a variable parameter list. The exact parameter list depends on 
  the OpCode that is passed into the function. If an unsupported OpCode or illegal 
  parameter list is passed in, this function returns EFI_INVALID_PARAMETER.
  If there are not enough resources available for storing more scripts, this function returns
  EFI_OUT_OF_RESOURCES.

  @param  This                  A pointer to the EFI_BOOT_SCRIPT_SAVE_PROTOCOL instance.
  @param  TableName             Name of the script table. Currently, the only meaningful value is
                                EFI_ACPI_S3_RESUME_SCRIPT_TABLE.
  @param  OpCode                The operation code (opcode) number.
  @param  ...                   Argument list that is specific to each opcode. 
                   
  @retval EFI_SUCCESS           The operation succeeded. A record was added into the
                                specified script table.
  @retval EFI_INVALID_PARAMETER The parameter is illegal or the given boot script is not supported.
                                If the opcode is unknow or not supported because of the PCD 
                                Feature Flags.
  @retval EFI_OUT_OF_RESOURCES  There is insufficient memory to store the boot script.

**/
EFI_STATUS
EFIAPI
BootScriptWrite (
  IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL    *This,
  IN UINT16                           TableName,
  IN UINT16                           OpCode,
  ...
  );
/**
  Closes the specified script table.

  This function closes the specified boot script table and returns the base address 
  of the table. It allocates a new pool to duplicate all the boot scripts in the specified 
  table. Once this function is called, the specified table will be destroyed after it is 
  copied into the allocated pool. As a result, any attempts to add a script record into a 
  closed table will cause a new table to be created. The base address of the allocated pool 
  will be returned in Address. After using the boot script table, the caller is responsible 
  for freeing the pool that is allocated by this function. If the boot script table,
  such as EFI_ACPI_S3_RESUME_SCRIPT_TABLE, is required to be stored in a nonperturbed
  memory region, the caller should copy the table into the nonperturbed memory region by itself.

  @param  This                  A pointer to the EFI_BOOT_SCRIPT_SAVE_PROTOCOL instance.
  @param  TableName             Name of the script table. Currently, the only meaningful value is
                                 EFI_ACPI_S3_RESUME_SCRIPT_TABLE.
  @param  Address               A pointer to the physical address where the table begins. 
                               
  @retval EFI_SUCCESS           The table was successfully returned.
  @retval EFI_NOT_FOUND         The specified table was not created previously.
  @retval EFI_OUT_OF_RESOURCE   Memory is insufficient to hold the reorganized boot script table.
  @retval EFI_UNSUPPORTED       the table type is not EFI_ACPI_S3_RESUME_SCRIPT_TABLE
  
**/
EFI_STATUS
EFIAPI
BootScriptCloseTable (
  IN EFI_BOOT_SCRIPT_SAVE_PROTOCOL    *This,
  IN UINT16                           TableName,
  OUT EFI_PHYSICAL_ADDRESS            *Address
  );
#endif //_INTERNAL_BOOT_SCRIPT_SAVE_H_
