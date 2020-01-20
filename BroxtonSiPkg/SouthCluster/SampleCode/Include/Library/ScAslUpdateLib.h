/** @file
  ASL dynamic update library definitions.

  This library provides dymanic update to various ASL structures.

  There may be different libraries for different environments (PEI, BS, RT, SMM).
  Make sure you meet the requirements for the library (protocol dependencies, use
  restrictions, etc).

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/
#ifndef _ASL_UPDATE_LIB_H_
#define _ASL_UPDATE_LIB_H_

//
// Include files
//
#include <PiDxe.h>
#include <IndustryStandard/Acpi30.h>


#include <Protocol/AcpiSupport.h>
#include <Protocol/AcpiTable.h>

//
// AML parsing definitions
//
#define AML_NAME_OP           0x08
#define AML_BUFFER_OP         0x11
#define AML_DMA_FIXED_DESC_OP 0x55
#define AML_DEVICE_OP         0x82
#define AML_MEMORY32_FIXED_OP 0x86
#define AML_DWORD_OP          0x87
#define AML_INTERRUPT_DESC_OP 0x89

/**
  Initialize the ASL update library state.
  This must be called prior to invoking other library functions.

  @param[in]  None

  @retval  EFI_SUCCESS                  - The function completed successfully.
**/
EFI_STATUS
InitializeScAslUpdateLib (
  VOID
  )
;

/**
  @brief

  This procedure will update immediate value assigned to a Name

  @param[in] AslSignature      - The signature of Operation Region that we want to update.
  @param[in] Buffer            - source of data to be written over original aml
  @param[in] Length            - length of data to be overwritten

  @retval EFI_SUCCESS                  - The function completed successfully.

**/
EFI_STATUS
UpdateNameAslCode(
  IN     UINT32                        AslSignature,
  IN     VOID                          *Buffer,
  IN     UINTN                         Length
  )
;
/**

  This function locates an ACPI structure and updates it.
  This function knows how to update operation regions and BUFA/BUFB resource structures.

  This function may not be implemented in all instantiations of this library.

  @param[in]  AslSignature      - The signature of Operation Region that we want to update.
  @param[in]  BufferName        - signature of the Buffer inside OpRegion that we want to update
  @param[in]  MacroAmlEncoding  - type of entry inside Buffer.
  @param[in]  MacroEntryNumber  - number of entry of the above type
  @param[in]  Offset            - offset (in bytes) inside entry where update will be performed
  @param[in]  Buffer            - source of data to be written over original aml
  @param[in]  Length            - length of data to be overwritten

  @retval  EFI_SUCCESS       - The function completed successfully.

**/
EFI_STATUS
UpdateResourceTemplateAslCode (
  IN     UINT32                        AslSignature,
  IN     UINT32                        BufferName,
  IN     UINT8                         MacroAmlEncoding,
  IN     UINT8                         MacroEntryNumber,
  IN     UINT8                         Offset,
  IN     VOID                          *Buffer,
  IN     UINTN                         Length
  )
;

/**

  This function uses the ACPI support protocol to locate an ACPI table using the .
  It is really only useful for finding tables that only have a single instance,
  e.g. FADT, FACS, MADT, etc.  It is not good for locating SSDT, etc.
  Matches are determined by finding the table with ACPI table that has
  a matching signature and version.

  @param[in]  TableId           - Pointer to an ASCII string containing the Signature to match
  @param[in]  Table             - Updated with a pointer to the table
  @param[in]  Handle            - AcpiSupport protocol table handle for the table found
  @param[in]  Version           - On input, the version of the table desired,
                                  on output, the versions the table belongs to
                                  (see AcpiSupport protocol for details)

  @retval  EFI_SUCCESS       - The function completed successfully.

**/
EFI_STATUS
LocateAcpiTableBySignature (
  IN      UINT32                        Signature,
  IN OUT  EFI_ACPI_DESCRIPTION_HEADER   **Table,
  IN OUT  UINTN                         *Handle,
  IN OUT  EFI_ACPI_TABLE_VERSION        *Version
  )
;

/**

  This function uses the ACPI support protocol to locate an ACPI SSDT table.
  The table is located by searching for a matching OEM Table ID field.
  Partial match searches are supported via the TableIdSize parameter.

  @param[in]      TableId           - Pointer to an ASCII string containing the OEM Table ID from the ACPI table header
  @param[in]      TableIdSize       - Length of the TableId to match.  Table ID are 8 bytes long, this function
                                      will consider it a match if the first TableIdSize bytes match
  @param[in,out]  Table             - Updated with a pointer to the table
  @param[in,out]  Handle            - AcpiSupport protocol table handle for the table found
  @param[in,out]  Version           - See AcpiSupport protocol, GetAcpiTable function for use

  @retval  EFI_SUCCESS       - The function completed successfully.

**/
EFI_STATUS
LocateAcpiTableByOemTableId (
  IN      UINT8                         *TableId,
  IN      UINT8                         TableIdSize,
  IN OUT  EFI_ACPI_DESCRIPTION_HEADER   **Table,
  IN OUT  UINTN                         *Handle,
  IN OUT  EFI_ACPI_TABLE_VERSION        *Version
  )
;

/**

  This function calculates and updates an UINT8 checksum.

  @param[in]      Buffer          Pointer to buffer to checksum
  @param[in]      Size            Number of bytes to checksum
  @param[in]      ChecksumOffset  Offset to place the checksum result in

  @retval  EFI_SUCCESS       - The function completed successfully.

**/
EFI_STATUS
AcpiChecksum (
  IN VOID       *Buffer,
  IN UINTN      Size,
  IN UINTN      ChecksumOffset
  )
;
#endif
