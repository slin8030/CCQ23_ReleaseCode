//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

#include <DnxFastBootDxe.h>
#include <Guid/GlobalVariable.h>
#include <MmioAccess.h>
#include <IndustryStandard/Pci.h>
#include <Guid/PlatformInfo.h>

//
// Firmware should boot into fastboot
//
#define EFI_OS_INDICATIONS_RESCUE_MODE   0x0000000000000020
#define EFI_OS_INDICATIONS_OSINITIATED_RESET   0x0000000000000040

//
// Volume button key combo
//
#define VOLUME_DOWN_CONF0  0xFED8C440      // North Community - 08
#define VOLUME_DOWN_CONF0_CR  0xFED85408   // Southwest Community - 61
#define VOLUME_UP_CONF0    0xFED85C18      // Southwest Community - 93

EFI_PLATFORM_INFO_HOB    *gPlatformInfo=NULL;

#define LOADER_ENTRY_ONE_SHOT L"LoaderEntryOneShot"
extern EFI_GUID               gLoaderEntryOneShotVariableGuid;

//
// Power button wake source status
//

BOOLEAN
EFIAPI
DnXFastBootPwrBtnGetLastWakeSource (
  VOID
  )
{
  UINT8         PmicWakeSrc = 0;
  UINT16        PmcPm1En = 0;
  UINT16        PmcPm1Sts = 0;

  PmicGetWakeCause(&PmicWakeSrc);
  PmcPm1En    = IoRead16 (ACPI_BASE_ADDRESS + R_ACPI_PM1_EN);
  PmcPm1Sts   = IoRead16 (ACPI_BASE_ADDRESS + R_ACPI_PM1_STS);

  if((BOOLEAN)(PmicWakeSrc & PMIC_MASK_WAKEPBTN)
    || ((BOOLEAN)(PmcPm1En & B_ACPI_PM1_EN_PWRBTN) && (BOOLEAN)(PmcPm1Sts & B_ACPI_PM1_STS_PWRBTN))){
    //DEBUG ((DEBUG_INFO, "\nPower button wake source detected..\n"));
    return TRUE;
  }else{
    //DEBUG ((DEBUG_INFO, "\nPower button wake source was not detected..\n"));
    return FALSE;
  }
}

//
// Timer Event Handler for polling Volume Up and Volume Down buttons
//

VOID
EFIAPI
DnxFastBootHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  EFI_STATUS              Status = EFI_SUCCESS;
  volatile UINT32         GpioValue = 0;
  DNX_FB_DEVICE_CONTEXT    *DnxFbDeviceContext = NULL;
  BOOLEAN          WakeStatus = FALSE;

  //DEBUG ((DEBUG_INFO, "\nEntered DnxFastBootHandler()\n")); // Floods UART console

  DnxFbDeviceContext = (DNX_FB_DEVICE_CONTEXT *) Context;

  //
  // Check for Volume Up Key
  //
  GpioValue = MmioRead32 (VOLUME_UP_CONF0);    // Southwest Community - 93
  if ((GpioValue & BIT0) == 0) {  // Rx state
    DEBUG ((DEBUG_INFO, "\nVolume Up button press detected! = %x \n",GpioValue));
    //
    // Check for Volume Down Key
    //
  if(gPlatformInfo->BoardId == BOARD_ID_CHT_CR) {
    GpioValue = MmioRead32 (VOLUME_DOWN_CONF0_CR);  // Southwest Community - 61
  } else {
    GpioValue = MmioRead32 (VOLUME_DOWN_CONF0);  // North Community - 08
  }
  if ((GpioValue & BIT0) == 0) {  // Rx state
    DEBUG ((DEBUG_INFO, "\nVolume Down button press detected!\n"));
      
    if(gPlatformInfo->BoardId == BOARD_ID_CHT_CR || gPlatformInfo->BoardId == BOARD_ID_CHT_FFD) {
      WakeStatus = TRUE; // Skip power button status check for CR only
    } else {
      WakeStatus = DnXFastBootPwrBtnGetLastWakeSource();
    }
      //
      // Check for Power Button wake source
      //
      if (WakeStatus) {      
            //
            //Call Fastboot here
            //
            DEBUG ((DEBUG_INFO, "\nDnX Fastboot request receieved...\n"));
            gBS->Stall(2*1000*1000); //Wait for 2 seconds for user to notice

            Status = gBS->CloseEvent(DnxFbDeviceContext->TimerEvent);

            PcdSetBool(PcdDnxFastboot, TRUE);

            if (EFI_ERROR (Status)) {
              DEBUG ((EFI_D_INFO, "DnxFastBootHandler() CloseEvent Failed\n"));
            }
            }
          }else{
            //DEBUG ((DEBUG_INFO, "\nNo change in the Volume Down button status\n"));
          }
      }else{
        //DEBUG ((DEBUG_INFO, "\nNo change in the Volume Up button status\n"));
      }
}

//
// Creates Timer Event to poll for DnX FastBoot request
//

EFI_STATUS
EFIAPI
DnXFastBoot (
  VOID
  )
{
  EFI_STATUS      Status = EFI_SUCCESS;
  EFI_EVENT        DnxFastBootEvent = 0;
  DNX_FB_DEVICE_CONTEXT  *DnxFbDeviceContext = NULL;

  DnxFbDeviceContext = AllocateZeroPool (sizeof(DNX_FB_DEVICE_CONTEXT));

  DEBUG ((EFI_D_INFO, "\nEntered DnXFastBoot()\n"));

  if (DnxFbDeviceContext == NULL) {
    DEBUG ((EFI_D_INFO, "DnXFastBoot() AllocateZeroPool Failed\n"));
    return  EFI_OUT_OF_RESOURCES;
  }

  DnxFbDeviceContext->TimerEvent = DnxFastBootEvent;

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  DnxFastBootHandler,
                  DnxFbDeviceContext,
                  &DnxFbDeviceContext->TimerEvent
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "DnXFastBoot() CreateEvent Failed\n"));
    return  Status;
  }

  Status = gBS->SetTimer (
                  DnxFbDeviceContext->TimerEvent,
                  TimerPeriodic,
                  500000             // 100ns * 500000 = (10^(-7)) * 500000 = 5 * (10^-2) = 50ms
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "DnXFastBoot() SetTimer Failed\n"));
    return Status;
  }

  return EFI_SUCCESS;
}

VOID
SetOsIndicationsSupported(
  VOID
  )
{
  UINTN   DataSize = 8;
  //UINT32   Attributes = 0;
  UINT64  OsIndicationSupport = 0;
  EFI_STATUS  Status = EFI_SUCCESS;


  Status = gRT->GetVariable (
                  L"OsIndicationsSupported",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  &OsIndicationSupport
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "SetOsIndicationsSupported - GetVariable() Failed - Status = 0x%x\n", Status));
  } else {
    DEBUG ((EFI_D_INFO, "SetOsIndicationsSupported - GetVariable() Passed - Status = 0x%x\n", Status));
  }

  //
  // Set the EFI_OS_INDICATIONS_RESCUE_MODE bit, to indicate that rescue mode is supported.
  // Set the EFI_OS_INDICATIONS_OSINITIATED_RESET bit, to indicate that Os initiated reset is supported.
  //

  OsIndicationSupport = OsIndicationSupport | EFI_OS_INDICATIONS_RESCUE_MODE | EFI_OS_INDICATIONS_OSINITIATED_RESET;

  Status = gRT->SetVariable (
                  L"OsIndicationsSupported",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  8,
                  &OsIndicationSupport
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "SetOsIndicationsSupported - SetVariable() Failed - Status = 0x%x\n", Status));
  }

  Status = gRT->GetVariable (
                  L"OsIndicationsSupported",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  &OsIndicationSupport
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "SetOsIndicationsSupported - GetVariable() Failed - Status = 0x%x\n", Status));
  } else {
    DEBUG ((EFI_D_INFO, "SetOsIndicationsSupported - Value = 0x%x\n", OsIndicationSupport));
  }
}

/**

  Check the EFI_OS_INDICATIONS_RESCUE_MODE in OsIndications Variable
  1, Delete OsIndications variable if it is not NV/BS/RT UINT64\
  2, Return the bit value of EFI_OS_INDICATIONS_RESCUE_MODE if OsIndications is available
 Item 1 is used to solve case when OS corrupts OsIndications. Here simply delete this NV variable.

**/

BOOLEAN
GetIfRescueMode(
  VOID
  )
{
  EFI_STATUS Status;
  UINT64     OsIndication;
  UINTN      DataSize;
  UINT32     Attributes;

  //
  // If OsIndications is invalid, remove it.
  // Invalid case
  //   1. Data size != UINT64
  //   2. OsIndication value inconsistent
  //   3. OsIndication attribute inconsistent
  //
  OsIndication = 0;
  Attributes = 0;
  DataSize = sizeof(UINT64);
  Status = gRT->GetVariable (
                  L"OsIndications",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  &OsIndication
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "GetIfRescueMode - GetVariable() Failed - Status = 0x%x\n", Status));
  }

  DEBUG ((DEBUG_INFO, "OsIndication = %x\n", OsIndication));

 /* if (!EFI_ERROR(Status)) {
    if (DataSize != sizeof(UINT64) ||
       (OsIndication & ~(EFI_OS_INDICATIONS_BOOT_TO_FW_UI | EFI_OS_INDICATIONS_RESCUE_MODE)) != 0 ||
        Attributes != (EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE)){

      DEBUG ((EFI_D_ERROR, "Unformalized OsIndications variable exists. Delete it\n"));
      Status = gRT->SetVariable (
                      L"OsIndications",
                      &gEfiGlobalVariableGuid,
                      Attributes,
                      0,
                      &OsIndication
                      );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "GetIfRescueMode - SetVariable() Failed - Status = 0x%x\n", Status));
    }

      return FALSE;
    }
  }
  */

  //
  // Check the EFI_OS_INDICATIONS_RESCUE_MODE bit, if it is set, clear it and return true.
  //
  if (OsIndication & EFI_OS_INDICATIONS_RESCUE_MODE) {
    OsIndication &= (~EFI_OS_INDICATIONS_RESCUE_MODE);
    Status = gRT->SetVariable (
                    L"OsIndications",
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS| EFI_VARIABLE_NON_VOLATILE,
                    sizeof(UINT64),
                    &OsIndication
                    );

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "GetIfRescueMode - SetVariable() Failed - Status = 0x%x\n", Status));
    }
    //
    // Set PCD to indicate that fast boot needs to be triggered
    //
    PcdSetBool(PcdDnxFastboot, TRUE);

    return TRUE;
  }

  return FALSE;

}


/**

  Check the LoaderEntryOneShot Variable

**/
BOOLEAN 
CheckLoaderEntryOneShot (
  VOID
  )
{
  EFI_STATUS  Status;
  UINTN       DataSize;
  UINT32      Attributes;
  UINT8       *LoaderEntryOneShot = NULL;

  //
  // If LoaderEntryOneShot is invalid, remove it.
  // Invalid case
  //   LoaderEntryOneShot value inconsistence
  //
  Attributes = 0;
  DataSize = 0;
  Status = gRT->GetVariable (
                  LOADER_ENTRY_ONE_SHOT,
                  &gLoaderEntryOneShotVariableGuid,
                  &Attributes,
                  &DataSize,
                  LoaderEntryOneShot
                  );

  if ((Status != EFI_BUFFER_TOO_SMALL) || (DataSize == 0))
    return FALSE;

  LoaderEntryOneShot = AllocateZeroPool (DataSize);
  Status = gRT->GetVariable (
                  LOADER_ENTRY_ONE_SHOT,
                  &gLoaderEntryOneShotVariableGuid,
                  &Attributes,
                  &DataSize,
                  LoaderEntryOneShot
                  ); 
  DEBUG ((DEBUG_INFO, "LoaderEntryOneShot = %s\n", LoaderEntryOneShot));

  if (!EFI_ERROR(Status)) {
    //
    // Check the LoaderEntryOneShot, If is valid, clear it and return true. 
    //
    if (!CompareMem (LoaderEntryOneShot, L"dnx", (StrLen (L"dnx") + 1) * sizeof(CHAR16))) {
      Status = gRT->SetVariable (
                    LOADER_ENTRY_ONE_SHOT,
                    &gLoaderEntryOneShotVariableGuid,
                    Attributes,
                    0,
                    NULL
                    );
      ASSERT_EFI_ERROR (Status);
      if(LoaderEntryOneShot != NULL)
        gBS->FreePool(LoaderEntryOneShot);

      PcdSetBool (PcdDnxFastboot, TRUE);
      return TRUE;
    }
  }

  if(LoaderEntryOneShot != NULL)
    gBS->FreePool(LoaderEntryOneShot);  

  return FALSE;
}

VOID
EFIAPI
DnxFbOnReadyToBoot (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  BOOLEAN    DnXFastBoot = FALSE;
  //
  // Set OsIndicationsSupported variable's bit 5 (EFI_OS_INDICATIONS_RESCUE_MODE) to indicate that fastboot is supported
  //
  SetOsIndicationsSupported ();
  //
  // Check if OsIndications variable's bit 5 (EFI_OS_INDICATIONS_RESCUE_MODE) is set
  // If it is set, set the PCD to trigger fast boot and clear the bit
  //
  GetIfRescueMode ();

  //
  // Check the LoaderEntryOneShot Variable
  //
  CheckLoaderEntryOneShot();

  DnXFastBoot = PcdGetBool (PcdDnxFastboot);

#if (_SIMIC_ || _SLE_HYB_ || _SLE_COMP_ || VP_FLAG)
  DnXFastBoot = 0;
#endif

  DEBUG ((DEBUG_INFO, "DnxFbOnReadyToBoot() - Entry\n"));

  if(DnXFastBoot){
    UINT32  OtgPciMmBase;

    OtgPciMmBase = MmPciAddress (0, 0, 0x16, 0, 0);

    Mmio32Or (OtgPciMmBase, PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_BUS_MASTER);
    
    DEBUG ((DEBUG_INFO, "FASTBOOT: Entering FastBoot Application..\n"));
    FastBootDataInit();
    FastBootStart();
    
    DEBUG ((DEBUG_INFO, "FASTBOOT: Exiting FastBoot Application..\n"));
    
    Mmio32And (OtgPciMmBase, PCI_COMMAND_OFFSET, ~EFI_PCI_COMMAND_BUS_MASTER);
  }

  gBS->CloseEvent(Event);
}


//
// Entrypoint of the driver
//

EFI_STATUS
EFIAPI
DnxFastBootDxeEntry (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{

  EFI_STATUS                  Status = EFI_SUCCESS;
  EFI_EVENT                   Event;
  EFI_PEI_HOB_POINTERS        GuidHob;
  
  //
  // Set polling for button combination
  //
  DnXFastBoot();

  // Get the HOB list.
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    if ((GuidHob.Raw = GetNextGuidHob (&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
     gPlatformInfo = GET_GUID_HOB_DATA (GuidHob.Guid);
    }
  }

  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             DnxFbOnReadyToBoot,
             NULL,
             &Event
             );

  return EFI_SUCCESS;
}

