/** @file
  Implements Overclocking Interface for OS Application ie Iron city

@copyright
 Copyright (c) 2015 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.
 This file contains a 'Sample Driver' and is licensed as such
 under the terms of your license agreement with Intel or your
 vendor. This file may be modified by the user, subject to
 the additional terms of the license agreement.

@par Specification Reference:
**/

#include <UsbTypeC.h>

#include <Protocol/AcpiTable.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/EcMiscLib.h>
#include <Protocol/GlobalNvsArea.h>
//#include <PlatformInfo.h>

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsAreaProtocol;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_GLOBAL_NVS_AREA           *mGlobalNvsAreaPtr;


#define USBTYPEC_DATA_VAR         L"UsbTypeC"
EFI_GUID UsbTypeCVarGuid      =  { 0xfc876842, 0xd8f0, 0x4844, {0xae, 0x32, 0x1f, 0xf8, 0x43, 0x79, 0x7b, 0x17} };

/**
  Allocate MemoryType below 4G memory address.

  @param[in] Size           Size of memory to allocate.
  @param[in] Buffer         Allocated address for output.

  @retval EFI_SUCCESS       Memory successfully allocated.
  @retval Other             Other errors occur.
**/
static EFI_STATUS
AllocateMemoryBelow4G (
  IN   EFI_MEMORY_TYPE MemoryType,
  IN   UINTN           Size,
  OUT  VOID           **Buffer
  )
{
  UINTN                 Pages;
  EFI_PHYSICAL_ADDRESS  Address;
  EFI_STATUS            Status;

  Pages = EFI_SIZE_TO_PAGES (Size);
  Address = 0xffffffff;

  Status  = (gBS->AllocatePages) (
                   AllocateMaxAddress,
                   MemoryType,
                   Pages,
                   &Address
                   );

  *Buffer = (VOID *) (UINTN) Address;

  return Status;
};

static UINT16 GetUcsiRev(VOID)
{
//[-start-170116-IB0540078-modify]//
//[-start-161205-IB07400822-modify]//
//[-start-170111-IB07400832-modify]//
//#if FeaturePcdGet(PcdKscSupport)
#if defined (KSC_SUPPORT)
#if defined (APOLLOLAKE_CRB) || defined (USE_CRB_HW_CONFIG)
  UINT8         Rev1, Rev2;
  //
  // CRB EC code
  //
  Rev1 = 0;
  Rev2 = 0;
  SendEcCommand( 0xE1);
  ReceiveEcData( &Rev1 ); 
  ReceiveEcData( &Rev2 ); 
  return ((Rev1 << 8) + Rev2 );
#else
//[-start-171223-IB07400935-add]//
  UINT8         Rev1, Rev2;

  //
  // OEM EC code
  //
  Rev1 = 0xE0;
  ReadEcRam (&Rev1);  
  Rev2 = 0xE1;
  ReadEcRam (&Rev2);
  return ((Rev1 << 8) + Rev2 );
//[-end-171223-IB07400935-add]//
#endif
#else
  //
  // No KSC
  //
  return 0;
#endif
//[-end-170111-IB07400832-modify]//
//[-end-161205-IB07400822-modify]//
//[-end-170116-IB0540078-modify]//

}

/**
  This function is the entry point for this DXE driver. It implements
  necessary interace (ASPT) for the OS Application (Iron City) to work.
  It creates ASPT table and publishes it.

  @param[in] ImageHandle       Image handle of this driver.
  @param[in] SystemTable       Global system service table.

  @retval EFI_DEVICE_ERROR     Initialization fails
  @retval EFI_SUCCESS          Initialization completes successfully.

**/
EFI_STATUS
UsbTypeCEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS			Status;
  USBTYPEC_OPM_BUF  	*OpmBuffer;
  DEBUG ((DEBUG_INFO, "UsbTypeC entrypoint.\n"));

  Status = gBS->LocateProtocol (&gEfiGlobalNvsAreaProtocolGuid, NULL, &GlobalNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  mGlobalNvsAreaPtr = GlobalNvsAreaProtocol->Area;
  //
  // Allocate memory in ACPI NVS
  //
  Status = AllocateMemoryBelow4G (EfiACPIMemoryNVS, 0x1000, &OpmBuffer);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  SetMem (OpmBuffer, sizeof (USBTYPEC_OPM_BUF), 0x0);
  OpmBuffer->Version =  GetUcsiRev();
  
  mGlobalNvsAreaPtr->UsbTypeCOpBaseAddr = (UINT32) (UINTN) OpmBuffer;
  DEBUG ((DEBUG_INFO, "UsbTypeC EntryPoint: mGlobalNvsAreaPtr->UsbTypeCOpBaseAddr = 0x%X\n", mGlobalNvsAreaPtr->UsbTypeCOpBaseAddr));
  
  Status = gRT->SetVariable (
		USBTYPEC_DATA_VAR,
		&UsbTypeCVarGuid,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
		sizeof (UINT32),
		&(mGlobalNvsAreaPtr->UsbTypeCOpBaseAddr)
	);
	
  return Status;
}

