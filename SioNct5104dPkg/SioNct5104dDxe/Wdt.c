/** @file

Device Protocol

;******************************************************************************
;* Copyright (c) 2014-2015, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include "SioInitDxe.h"
#include <Protocol/IsaNonPnpDevice.h>
#include <IndustryStandard/WatchdogActionTable.h>
#include <AcpiHeaderDefaultValue.h>
#include <Protocol/AcpiTable.h>

static UINT8         Ldn;
static EFI_WDT_INFO  *mWdtInfo;

EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE  gAcpiWdatTable = {
  {
    EFI_SIGNATURE_32 ('W', 'D', 'A', 'T'),
    sizeof (EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE),
    EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE_REVISION,
    0,
    EFI_ACPI_OEM_ID,
    EFI_ACPI_OEM_TABLE_ID,
    EFI_ACPI_OEM_REVISION,
    EFI_ACPI_CREATOR_ID,
    EFI_ACPI_CREATOR_REVISION,
  },
  sizeof (EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE) - sizeof (EFI_ACPI_DESCRIPTION_HEADER),
  0xFF,     // PCISegment;
  0xFF,     // PCIBusNumber;
  0xFF,     // PCIDeviceNumber;
  0xFF,     // PCIFunctionNumber;
  0, 0, 0,  // Reserved_45[3];
  1000,     // TimerPeriod;
  301,      // MaxCount;
  1,        // MinCount;
  EFI_ACPI_WDAT_1_0_WATCHDOG_ENABLED | EFI_ACPI_WDAT_1_0_WATCHDOG_STOPPED_IN_SLEEP_STATE,
  0, 0, 0,
  0,
};

EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY gAcpiWdatActEntryTable [] = {
  //
  //================================================================
  //                   Restart the Watchdog Timer List
  //================================================================
  //
  // Restarts the watchdog timer's countdown.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_LDN,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    SIO_WDT,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_WDT_COUNTER_REG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    DEFAULT_TIME_OUT_VALUE,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    EXIT_CONFIG,
    0xFF
  },
  //
  //================================================================
  //                   Set Countdown Period List
  //================================================================
  //
  // Sets the countdown value (in count intervals) to be used when
  // the watchdog timer is reset. This action is required if
  // WATCHDOG_ACTION_RESET does not explicitly write a new countdown
  // value to a register during a reset.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_LDN,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    SIO_WDT,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_WDT_COUNTER_REG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    DEFAULT_TIME_OUT_VALUE,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    EXIT_CONFIG,
    0xFF
  },
  //
  //================================================================
  //                   Query Running State List
  //================================================================
  //
  // Determines if the watchdog hardware is currently in enabled/running
  // state. The same result must occur when performed from both 
  // from enabled/stopped state and enabled/running state.
  // If the watchdog hardware is disabled, results are indeterminate.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x70},
    SIO_WDT_STATUS_CMOS_INDEX,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x71},
    WDT_ENABLE_CMOS_STATUS_BIT,
    WDT_ENABLE_CMOS_STATUS_BIT
  },
  //
  //================================================================
  //                   Set Running State List
  //================================================================
  //
  // Starts the watchdog, if not already in running state.
  // If the watchdog hardware is disabled, results are indeterminate.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_LDN,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    SIO_WDT,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_WDT_COUNTER_REG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    DEFAULT_TIME_OUT_VALUE,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    EXIT_CONFIG,
    0xFF
  },
  //
  //================================================================
  //                   Query Stopped State List
  //================================================================
  //
  // Determines if the watchdog hardware is currently in enabled/stopped 
  // state. The sameresult must occur when performed from both the 
  // enabled/stopped state and enabled/running state.
  // If the watchdog hardware is disabled, results are indeterminate.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x70},
    SIO_WDT_STATUS_CMOS_INDEX,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_READ_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x71},
    0x00,
    WDT_ENABLE_CMOS_STATUS_BIT
  },
  //
  //================================================================
  //                   Set Stopped State List
  //================================================================
  //
  // Stops the watchdog, if not already in stopped state.
  // If the watchdog hardware is disabled, results are indeterminate.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_LDN,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    SIO_WDT,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    SIO_WDT_COUNTER_REG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2F},
    0x00,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_STOPPED_STATE,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    EXIT_CONFIG,
    0xFF
  },
  //
  //================================================================
  //                   Query Watchdog Status List
  //================================================================
  //
  // Determines if the current boot was caused by the watchdog firing.
  // The boot status is required to be set if the watchdog fired 
  // and caused a reboot.
  // It is recommended that the Watchdog Status be set if the watchdog
  // fired and cause shutdown.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_WATCHDOG_STATUS,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_WATCHDOG_STATUS,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_QUERY_WATCHDOG_STATUS,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    EXIT_CONFIG,
    0xFF
  },
  //
  //================================================================
  //                   Clear Watchdog Status List
  //================================================================
  //
  // Sets the watchdog's boot status to the default value.
  // This action is required.
  //
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_WATCHDOG_STATUS,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_WATCHDOG_STATUS,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    ENTER_CONFIG,
    0xFF
  },
  {
    EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_WATCHDOG_STATUS,
    EFI_ACPI_WDAT_1_0_WATCHDOG_INSTRUCTION_WRITE_VALUE,
    0,
    0,
    {EFI_ACPI_3_0_SYSTEM_IO, 8, 0, EFI_ACPI_3_0_BYTE, 0x2E},
    EXIT_CONFIG,
    0xFF
  },
};

/**
  This function check if already exists a WDAT table in system, it will delete it.
**/
EFI_STATUS
DeleteDefaultWdat (
  VOID
  )
{
  INTN                     Index;
  UINTN                    Handle;
  EFI_STATUS               Status;
  EFI_ACPI_SDT_HEADER      *Table;
  EFI_ACPI_SDT_PROTOCOL    *AcpiSdtProtocol;
  EFI_ACPI_TABLE_VERSION   Version;
  EFI_ACPI_TABLE_PROTOCOL  *AcpiTableProtocol;

  Index = 0;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiSdtProtocolGuid,
                  NULL,
                  (VOID**)&AcpiSdtProtocol
                  );
  if (EFI_ERROR (Status)) return Status;

  Status = gBS->LocateProtocol (
                  &gEfiAcpiTableProtocolGuid,
                  NULL,
                  (VOID **)&AcpiTableProtocol
                  );
  if (EFI_ERROR (Status)) return Status;

  //
  // Search WDAT table
  //
  while (TRUE) {
    Table  = NULL;
    Status = AcpiSdtProtocol->GetAcpiTable (
                                Index,
                                &Table,
                                &Version,
                                &Handle
                                );
    if (EFI_ERROR (Status)) break;

    //
    // Check Signture and delete default WDAT table
    //
    if (Table->Signature == EFI_SIGNATURE_32 ('W', 'D', 'A', 'T')) {
      Status = AcpiTableProtocol->UninstallAcpiTable (AcpiTableProtocol, Handle);
      FreePool (Table);
      return Status;
    }

    //
    // Not found, get next
    //
    FreePool (Table);
    Index++;
  }

  return EFI_NOT_FOUND;
}

/**
  This function get watch dog time out value setting from PcdSioXXXXExtensiveCfg or SIO WDT setup variable.
**/
EFI_STATUS
GetSioWdtInfo (
  IN OUT UINT8   *TimerCountMode,
  IN OUT UINT16  *TimeOutValue
  )
{
  UINT8                       *PcdExtensiveCfgPointer;
  UINTN                       BufferSize;
  EFI_STATUS                  Status;
  SIO_NCT5104D_CONFIGURATION  *SioConfiguration;
  SIO_WATCH_DOG_SETTING       *WdtSetting;
  SIO_DEVICE_LIST_TABLE       *PcdPointer;

  Status     = EFI_UNSUPPORTED;
  PcdPointer = mTablePtr;
  BufferSize = sizeof (SIO_NCT5104D_CONFIGURATION);

  if (mFirstBoot) {
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      if ((PcdPointer->Device == WDT) && (PcdPointer->DeviceEnable == TRUE)) {
        PcdExtensiveCfgPointer = mExtensiveTablePtr;
        FindExtensiveDevice (
          &PcdExtensiveCfgPointer,
          PcdPointer->TypeInstance,
          PcdPointer->Device,
          PcdPointer->DeviceInstance
          );
        if (PcdExtensiveCfgPointer != NULL) {
          do {
            PcdExtensiveCfgPointer += *(PcdExtensiveCfgPointer + 1);
            if (*PcdExtensiveCfgPointer == WATCH_DOG_SETTING_TYPE) {
              WdtSetting = (SIO_WATCH_DOG_SETTING*)PcdExtensiveCfgPointer;
              *TimeOutValue   = WdtSetting->TimeOut;
              *TimerCountMode = WdtSetting->UnitSelection;
              return EFI_SUCCESS;
            }
          } while ((*PcdExtensiveCfgPointer != DEVICE_EXTENSIVE_RESOURCE) && (*(UINT32*)PcdExtensiveCfgPointer != EXTENSIVE_TABLE_ENDING));
        }
      }
      PcdPointer++;
    }
  } else {
    SioConfiguration = (SIO_NCT5104D_CONFIGURATION*)AllocateZeroPool (BufferSize);
    ASSERT (SioConfiguration != NULL);
    Status = gRT->GetVariable (
                    mSioVariableName,
                    &mSioFormSetGuid,
                    NULL,
                    &BufferSize,
                    SioConfiguration
                    );
    ASSERT_EFI_ERROR (Status);
    *TimeOutValue   = SioConfiguration->WatchDogConfig.TimeOutValue;
    *TimerCountMode = SioConfiguration->WatchDogConfig.TimerCountMode;
    FreePool (SioConfiguration);
  }

  return Status;
}

/**
  This function register a ready to boot event to install WDAT table.

  @param[in]   Event             Event whose notification function is being invoked.
  @param[in]   Context           Pointer to the notification function's context. 
**/
VOID
EFIAPI
WatchDogTimerReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  UINTN                      Index;
  UINTN                      TableHandle;
  UINT8                      *UpdatedBuffer;
  UINT8                      TimerUnit;
  UINT16                     TimeOutValue;
  UINT32                     WdataActEntryNum;
  EFI_STATUS                 Status;
  EFI_ACPI_TABLE_VERSION     TableVersion;
  EFI_ACPI_SUPPORT_PROTOCOL  *AcpiSupport;
  EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY  *WdatActEntry;
  EFI_GUID                   Guid = EFI_ACPI_SUPPORT_GUID;

  AcpiSupport      = NULL;
  UpdatedBuffer    = NULL;
  TableHandle      = 0;
  TimeOutValue     = 0;
  TableVersion     = EFI_ACPI_TABLE_VERSION_2_0;
  WdataActEntryNum = sizeof (gAcpiWdatActEntryTable) / sizeof (EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY);

  Status = gBS->LocateProtocol (
                  &Guid,
                  NULL,
                  (VOID **)&AcpiSupport
                  );
  if (EFI_ERROR (Status)) return;

  UpdatedBuffer = (UINT8*)AllocateZeroPool (sizeof (gAcpiWdatTable) + sizeof (gAcpiWdatActEntryTable));
  ASSERT (UpdatedBuffer != NULL);
  CopyMem (UpdatedBuffer, &gAcpiWdatTable, sizeof (gAcpiWdatTable));

  IoWrite8 (0x70, SIO_WDT_STATUS_CMOS_INDEX);
  if ((IoRead8 (0x71) & WDT_ENABLE_CMOS_STATUS_BIT)) {
    DeleteDefaultWdat ();
    GetSioWdtInfo (&TimerUnit, &TimeOutValue);
    switch (TimerUnit) {
    case 0: // Second
      if (TimeOutValue <= WDAT_MINIMUM_SECONDS) {
        TimeOutValue = 0;
      }
      break;
    case 1: // Minute
      if ((TimeOutValue * 60) <= WDAT_MINIMUM_SECONDS) {
        TimeOutValue = 0;
      }
      break;
    case 2: // Millisecond
      if (TimeOutValue <= (WDAT_MINIMUM_SECONDS * 1000)) {
        TimeOutValue = 0;
      }
      break;
    case 3: // MilliMinute
      if ((TimeOutValue * 60) <= (WDAT_MINIMUM_SECONDS * 1000)) {
        TimeOutValue = 0;
      }
      break;
    default: // Not define
      TimeOutValue = 0;
      break;
    }
  }

  WdatActEntry = (EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY*)(UpdatedBuffer + sizeof (gAcpiWdatTable));

  for (Index = 0; Index < WdataActEntryNum; Index++, WdatActEntry++) {
    //
    // Sync SuperIO index and data port
    //
    if (gAcpiWdatActEntryTable[Index].RegisterRegion.Address == 0x2E) {
      gAcpiWdatActEntryTable[Index].RegisterRegion.Address = mSioResourceFunction->SioCfgPort;
    }
    if (gAcpiWdatActEntryTable[Index].RegisterRegion.Address == 0x2F) {
      gAcpiWdatActEntryTable[Index].RegisterRegion.Address = (mSioResourceFunction->SioCfgPort + 1);
    }

    if ((gAcpiWdatActEntryTable[Index].WatchdogAction == EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_RESET) ||
        (gAcpiWdatActEntryTable[Index].WatchdogAction == EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_RUNNING_STATE) ||
        (gAcpiWdatActEntryTable[Index].WatchdogAction == EFI_ACPI_WDAT_1_0_WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD)) {
      if (gAcpiWdatActEntryTable[Index].Value == DEFAULT_TIME_OUT_VALUE) {
        gAcpiWdatActEntryTable[Index].Value = (UINT32)TimeOutValue;
      }
    }

    CopyMem (WdatActEntry, &gAcpiWdatActEntryTable[Index], sizeof (EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY));
    ((EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE*)UpdatedBuffer)->Header.Length += sizeof (EFI_ACPI_WATCHDOG_ACTION_1_0_WATCHDOG_ACTION_INSTRUCTION_ENTRY);
    ((EFI_ACPI_WATCHDOG_ACTION_1_0_TABLE*)UpdatedBuffer)->NumberWatchdogInstructionEntries++;
  }

  Status = AcpiSupport->SetAcpiTable (
                          AcpiSupport,
                          UpdatedBuffer,
                          TRUE,
                          TableVersion,
                          &TableHandle
                          );
  ASSERT_EFI_ERROR (Status);
  FreePool (UpdatedBuffer);
  gBS->CloseEvent (Event);

  return;
}

EFI_STATUS
WriteOnOff (
  BOOLEAN  OnOff
  )
{
  UINT8 Value;

  EnterConfigMode ();

  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
  Value = IDR8 (SIO_DEV_ACTIVE, mSioResourceFunction);
  Value &= ~BIT0;
  Value |= OnOff;
  IDW8 (SIO_DEV_ACTIVE, Value, mSioResourceFunction);

  ExitConfigMode ();

  IoWrite8 (0x70, SIO_WDT_STATUS_CMOS_INDEX);
  if (OnOff == FALSE) {
    Value = (IoRead8 (0x71) & ~WDT_ENABLE_CMOS_STATUS_BIT);
  } else {
    Value = (IoRead8 (0x71) | WDT_ENABLE_CMOS_STATUS_BIT);
  }
  IoWrite8 (0x71, Value);

  return EFI_SUCCESS;
}

EFI_STATUS
WriteMode (
  UINT8  Mode
  )
{
  UINT8  Value;

  EnterConfigMode ();

  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
  Value = IDR8 (SIO_WDT_CONTROL_MODE_REG, mSioResourceFunction);

  switch (Mode) {
  case 0: // Second
    IDW8 (SIO_WDT_CONTROL_MODE_REG, (Value & ~BIT3) & ~BIT4, mSioResourceFunction);
    break;
  case 1: // Minute
    IDW8 (SIO_WDT_CONTROL_MODE_REG, (Value | BIT3) & ~BIT4, mSioResourceFunction);
    break;
  case 2: // Millisecond
    IDW8 (SIO_WDT_CONTROL_MODE_REG, (Value & ~BIT3) | BIT4, mSioResourceFunction);
    break;
  case 3: // Milli Minute
    IDW8 (SIO_WDT_CONTROL_MODE_REG, (Value | BIT3 | BIT4), mSioResourceFunction);
    break;
  default:
    break;
  }

  ExitConfigMode ();

  return EFI_SUCCESS;
}

EFI_STATUS
WriteCounter (
  UINT16  Counter
  )
{
  EnterConfigMode ();

  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
  IDW8 (SIO_WDT_COUNTER_REG, (UINT8)(Counter & 0xFF), mSioResourceFunction);

  ExitConfigMode ();

  return EFI_SUCCESS;
}

EFI_STATUS
WriteReg (
  UINT8  Offset,
  UINT8  Reg
  )
{
  EnterConfigMode ();

  IDW8 (SIO_LDN, Ldn, mSioResourceFunction);
  IDW8 (Offset, Reg, mSioResourceFunction);

  ExitConfigMode ();

  return EFI_SUCCESS;
}

EFI_STATUS
ReadData (
  EFI_WDT_INFO  **Info
  )
{
  *Info = mWdtInfo;

  return EFI_SUCCESS;
}

VOID
WdtInitFunction (
  )
{
  EFI_EVENT   Event;
  EFI_STATUS  Status;

  Status = EfiCreateEventReadyToBootEx (
             TPL_NOTIFY,
             WatchDogTimerReadyToBoot,
             NULL,
             &Event
             );
  ASSERT_EFI_ERROR (Status);
}

STATIC
EFI_SIO_WDT_PROTOCOL mSioWdtProtocol = {
  WriteOnOff,
  WriteMode,
  WriteCounter,
  WriteReg,
  ReadData
};

/**
  brief-description of function.

  extended description of function.

**/
EFI_STATUS
InstallWdtProtocol (
  IN SIO_DEVICE_LIST_TABLE  *DeviceList
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  DeviceHandle;

  Ldn = DeviceList->DeviceLdn;
  DeviceHandle=NULL;

  Status = gBS->InstallProtocolInterface (
                  &DeviceHandle,
                  &gEfiSioWdtProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mSioWdtProtocol
                  );

  WdtInitFunction();

  return Status;
}
