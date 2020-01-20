/** @file

  Event Log POST Display DXE implementation.

;******************************************************************************
;* Copyright (c) 2014, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#include <EventLogPostDisplay.h>
#include <EventLogPostDisplayCommonFunc.h>
#include <Library/UefiLib.h>
#include <Library/CmosLib.h>
#include <Library/TimerLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Guid/EventLogPostDisplayVariable.h>
//[-start-171207-IB08400539-add]//
#include <EventLogDefine.h>
//[-end-171207-IB08400539-add]//


UINTN                               gColumn;
UINTN                               gTopRow;
UINTN                               gBottomRow;
UINTN                               gMaxItemPerPage;
UINTN                               ItemNum;
UINTN                               SelectedItem;
UINTN                               SelectedMenu;
VIEW_EVENT_LOG_MENU                 *MenuList;
EVENT_LOG_PROTOCOL                  *gEventLog;
BOOLEAN                             gShowUtilVer; 
CHAR16                              *gUtilityVersion = L"Version 100.00.01";
EL_POST_DISP_STORAGE                gElPostDispStorage = {
                                      FALSE,  // BIOS
                                      FALSE,  // BMC SEL
                                      FALSE   // Memory
                                      };
EL_POST_DISPLAY_VARIABLE            gElPostDispVar;

CHAR16                              *gStorageSupported[] = {
                                       L"BIOS",
                                       L"BMC SEL",
                                       L"MEMORY"
                                       }; // should be related to MAX_STORAGE_SUPPORT
BOOLEAN                             gIsFilterEvent;

/**
  Function waits for a given event to fire, or for an optional timeout to expire.

  @param   Event              The event to wait for
  @param   Timeout            An optional timeout value in 100 ns units.

  @retval  EFI_SUCCESS      Event fired before Timeout expired.
  @retval  EFI_TIME_OUT     Timout expired before Event fired..

**/
EFI_STATUS
EFIAPI
WaitForSingleEvent (
  EFI_EVENT  Event,
  UINT64     Timeout OPTIONAL
  )
{
  UINTN       Index;
  EFI_STATUS  Status;
  EFI_EVENT   TimerEvent;
  EFI_EVENT   WaitList[2];

  if (Timeout != 0) {
    //
    // Create a timer event
    //
    Status = gBS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &TimerEvent);
    if (!EFI_ERROR (Status)) {
      //
      // Set the timer event
      //
      gBS->SetTimer (
             TimerEvent,
             TimerRelative,
             Timeout
             );

      //
      // Wait for the original event or the timer
      //
      WaitList[0] = Event;
      WaitList[1] = TimerEvent;
      Status      = gBS->WaitForEvent (2, WaitList, &Index);
      gBS->CloseEvent (TimerEvent);

      //
      // If the timer expired, change the return to timed out
      //
      if (!EFI_ERROR (Status) && Index == 1) {
        Status = EFI_TIMEOUT;
      }
    }
  } else {
    //
    // No timeout... just wait on the event
    //
    Status = gBS->WaitForEvent (1, &Event, &Index);
  }

  return Status;
}

/**
 Get RTC time data. Before reading every RTC data, it will check RTC update status
 to make sure every RTC data is correct.

 @param[out] Second             Pointer to output time data(Second).

 @retval EFI_SUCCESS            Get RTC time data successfully.
**/
EFI_STATUS
EFIAPI
GetTimeForSecond (
  OUT UINT8                             *Second
  )
{
  RTC_REGISTER_A  RegisterA;
  UINTN           Index;
  UINTN           RtcComponentNum;
  RTC_COMPONENT   RtcComponent[] = {{9, 0}, // Year
                                    {8, 0}, // Month
                                    {7, 0}, // Day
                                    {4, 0}, // Hour
                                    {2, 0}, // Minute
                                    {0, 0}  // Second
                                    };

  RtcComponentNum = sizeof(RtcComponent) / sizeof(RTC_COMPONENT);

  //
  // In order to preventing RTC data reading error upon RTC updating due to interrupt/SMI caused delay,
  // making RTC update checking before every RTC data reading to make sure every RTC data corrected
  //
  for (Index = 0; Index < RtcComponentNum; Index++) {
    RegisterA.Data = ReadCmos8 (10);
    if (RegisterA.Bits.Uip == 1) {
      MicroSecondDelay (10);
      return EFI_NOT_READY;
    }

    RtcComponent[Index].Data = ReadCmos8 (RtcComponent[Index].Address);
  }

  *Second = ((RtcComponent[5].Data >> 4) * 10) + (RtcComponent[5].Data &0x0F);

  return EFI_SUCCESS;
}

/**
 Check if needing to display the Event Log Viewer.         
 
 @retval VOID                  
*/
BOOLEAN
EFIAPI
CheckDisplayElv (
  VOID
  )
{
  EFI_STATUS                           Status;
  UINTN                                TimeoutRemain;
  EFI_INPUT_KEY                        Key;
  UINTN                                Index;
  UINT8                                OrgSec;
  UINT8                                Sec;
  EFI_TPL                              OldTpl;

  Index = 0;
  OldTpl = 0;

  TimeoutRemain = MAX_WAIT_SECOND;
  gST->ConOut->ClearScreen (gST->ConOut);
  while (TimeoutRemain != 0) {  
    SetCurPos (0, 0);
    Print (L"Press F1 to show last power-on events, or any key to skip... Remaining %d Seconds! \n", TimeoutRemain);

    OrgSec = 0;
    Sec    = 0;
    while (TRUE) {
      OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL); 
      gBS->RestoreTPL (TPL_APPLICATION);
      
      GetTimeForSecond (&OrgSec);
      Status = WaitForSingleEvent (gST->ConIn->WaitForKey, ONE_SECOND / 5);
      if (Status != EFI_TIMEOUT) {
        Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
        if (!EFI_ERROR (Status)) {
          if (Key.ScanCode == SCAN_F1) {
            //
            // User pressed enter, equivalent to select "continue"
            //
            return TRUE;
          } else {
            return FALSE;
          }
        }
      }
      
      GetTimeForSecond (&Sec);
      if (Sec != OrgSec) {
        break;
      }
      if (OldTpl != 0) {
        gBS->RaiseTPL (TPL_HIGH_LEVEL); 
        gBS->RestoreTPL (OldTpl);
      }
    }
    
    TimeoutRemain--;
  }

  //
  // Timeout expired
  //
  if (TimeoutRemain == 0) {
    return FALSE;
  }
  
  return FALSE;
}

/**
 Free the memory space which used by MenuList.         
 
 @retval VOID                  
*/
VOID
EFIAPI
FreeMenuList (
  VOID
  )
{
  UINTN            Index;


  for (Index = 0; Index < MAX_STORAGE_SUPPORT; Index++) {
    if (MenuList[Index].FilteredEventNum != 0) {
      FreePool (MenuList[Index].StorageString);
      FreePool (MenuList[Index].ShowString);
    } 
  }

  FreePool (MenuList);

}

/**
 Function of draw frame.          
 
 @retval EFI Status                  
*/
VOID
EFIAPI
DrawFrame (
  EFI_SCREEN_DESCRIPTOR             LocalScreen
  )
{
  UINTN                  Index;
  CHAR16                 *ViewEventTitle = L"H2O Event Log Viewer";
  DsplayHotkeyInfo1 (LocalScreen);
  ClearLines (
    LocalScreen.LeftColumn, 
    LocalScreen.RightColumn, 
    LocalScreen.TopRow, 
    LocalScreen.TopRow, 
    VE_TITLE_TEXT);
  //
  // Show "H2O Event Log Utility" Title
  //
  SetCurPos ((LocalScreen.RightColumn - GetStringWidth(ViewEventTitle)/2)/2, 0);
  Print (L"%s", ViewEventTitle);
  ClearLines (LocalScreen.LeftColumn, 
              LocalScreen.RightColumn, 
              LocalScreen.TopRow + 1, 
              LocalScreen.TopRow + 1, 
              VE_SUBTITLE_TEXT);

  SetColor (VE_BODER_LINE_COLOR);
  
  SetCurPos (LocalScreen.LeftColumn, gTopRow - 1);
  Print (L"%c", BOXDRAW_DOWN_RIGHT);
  
  SetCurPos (LocalScreen.LeftColumn, gBottomRow + 1);
  Print (L"%c", BOXDRAW_UP_RIGHT);
  
  SetCurPos (LocalScreen.RightColumn - 1, gTopRow - 1);
  Print (L"%c", BOXDRAW_DOWN_LEFT);
  
  SetCurPos (LocalScreen.RightColumn - 1, gBottomRow + 1);
  Print (L"%c", BOXDRAW_UP_LEFT);

  for (Index = LocalScreen.LeftColumn + 1; Index < (LocalScreen.RightColumn - 1) ; Index++) {
    SetCurPos (Index, gTopRow - 1);
    Print (L"%c", BOXDRAW_HORIZONTAL);
    
    SetCurPos (Index, gBottomRow + 1);
    Print (L"%c", BOXDRAW_HORIZONTAL);
  }

  for (Index = gTopRow; Index < gBottomRow + 1; Index++) {
    SetCurPos (LocalScreen.LeftColumn, Index);
    Print (L"%c", BOXDRAW_VERTICAL);
    
    SetCurPos (LocalScreen.RightColumn - 1, Index);
    Print (L"%c", BOXDRAW_VERTICAL);
  }

  ClearLines (
    gColumn,
    LocalScreen.RightColumn - 1,
    gTopRow,
    gBottomRow,
    VE_NORMAL_MENU_TEXT);
  
  return;
}

/**
 Init Function.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
InitMenu (
  VOID
  )
{
  EFI_STATUS                            Status = EFI_SUCCESS;
  UINTN                                 StorageNum;

  ItemNum = 0;
  
  StorageNum = MAX_STORAGE_SUPPORT;
  MenuList = NULL;
  MenuList = (VIEW_EVENT_LOG_MENU *) AllocateZeroPool (sizeof(VIEW_EVENT_LOG_MENU) * StorageNum);
  if (MenuList == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return Status;

}

/**
 Print function.         
 
 @retval EFI Status                  
*/
VOID
EFIAPI
PrintMenu (
  VOID
  )
{ 
  UINT8     Index, Index2;

  Index = 0;
  Index2 = 0;
  
  for (Index = 0; Index < ItemNum; Index++){  
    if (Index2 == SelectedItem) {
      SetColor (VE_SELECTED_MENU_TEXT);
      SelectedMenu = Index;
    } else {
      SetColor (VE_NORMAL_MENU_TEXT);
    }
    SetCurPos (gColumn, gTopRow + Index2);
    Print (L"%s", MenuList[Index].ShowString);
    Index2++;
  }

  //Restore color setting
  SetColor (VE_NORMAL_MENU_TEXT);
}

/**
 Get the Event String.    

 Param[in]  EventInfo     The information of the event to translate string message.
 Param[out] EventString   String returned for the event.
 Param[out] StringSize    Size of EventString.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventString (
  IN     VOID                           *EventInfo,
  IN OUT CHAR16                         **EventString,
  IN OUT UINTN                          *StringSize
  )
{
  EFI_STATUS                              Status;
  UINTN                                   HandleCount;
  EFI_HANDLE                              *HandleBuffer;
  UINTN                                   Index;
  H2O_EVENT_LOG_STRING_PROTOCOL           *EventLogString;
  H2O_EVENT_LOG_STRING_OVERWRITE_PROTOCOL *EventLogStringOverwrite;

  Status = gBS->LocateProtocol (
                            &gH2OEventLogStringOverwriteProtocolGuid,
                            NULL,
                            (VOID **)&EventLogStringOverwrite
                            );
  if (!EFI_ERROR(Status)) {
    //
    // There exist RAS String Overwrite protocol. So, chech it firstly.
    //
    Status = EventLogStringOverwrite->EventLogStringOverwriteHandler (EventInfo, EventString, StringSize);
    if (!EFI_ERROR(Status)) {
      //
      // We Got it.
      //
      return EFI_SUCCESS;
    }
  }
  
  HandleCount  = 0;
  HandleBuffer = NULL;
  Status = gBS->LocateHandleBuffer (
                                ByProtocol,
                                &gH2OEventLogStringProtocolGuid,
                                NULL,
                                &HandleCount,
                                &HandleBuffer
                                );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (HandleBuffer);
    return Status;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    //
    // Double check which device connected on PCI(1D|0). Is keyboard??
    //
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gH2OEventLogStringProtocolGuid,
                    (VOID **)&EventLogString
                    );  
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = EventLogString->EventLogStringHandler (EventInfo, EventString, StringSize);
    if (!EFI_ERROR (Status)) {
      //
      // We Got It
      //
      return Status;
    }
  }

  gBS->FreePool (HandleBuffer);

  return EFI_NOT_FOUND;
}

/**
 Execute function of Event Log Viewer.                        
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
ExecuteFilterEventLogViewer (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_INPUT_KEY                             Key;
  UINTN                                     Row, Column;
  UINTN                                     Index;
  EFI_SCREEN_DESCRIPTOR                     LocalScreen;
  UINTN                                     OrgMode;
  BOOLEAN                                   DisplayElvEnable;

  gBS->CloseEvent (Event);
  
  DisplayElvEnable = FALSE;
  DisplayElvEnable = CheckDisplayElv ();
  if (DisplayElvEnable == FALSE) {
    FreeMenuList ();
    return EFI_SUCCESS;
  }
  
  OrgMode = gST->ConOut->Mode->Mode;
  gST->ConOut->QueryMode (
                 gST->ConOut,
                 gST->ConOut->Mode->Mode,
                 &LocalScreen.RightColumn,
                 &LocalScreen.BottomRow
                 );
  LocalScreen.LeftColumn = 0;
  LocalScreen.TopRow = 0;
  
  Column            = LocalScreen.LeftColumn + 1;
  Row               = LocalScreen.TopRow + 3;
  gColumn           = LocalScreen.LeftColumn + 1;
  gTopRow           = LocalScreen.TopRow + 3;
  gBottomRow        = LocalScreen.BottomRow - 4;
  gMaxItemPerPage   = gBottomRow - gTopRow;
  SelectedItem      = 0;
  SelectedMenu      = 0;
  
  EnCursor(FALSE);

  //
  // Exit if no Event Log Storages exist
  //
  if (ItemNum == 0) {
    DisplayPopupMessage (L"No Event Log Storage Exist!!", VeDlgOk, LocalScreen);
    FreeMenuList ();
    return EFI_NOT_FOUND;
  }
  
  DrawFrame (LocalScreen);
  
  ClearLines (
    gColumn,
    LocalScreen.RightColumn - 1,
    gTopRow,
    gBottomRow,
    VE_NORMAL_MENU_TEXT);
  
  do {
    PrintMenu ();  
    
    SetCurPos (Column, Row);
    
    gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &Index);
    gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);  

    switch (Key.UnicodeChar) {
      
    default:
      if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
        if (MenuList[SelectedMenu].Id < (ItemNum)) {
          //
          // Process View Storage Event Log function.
          //
          ViewStorageEventLog (MenuList[SelectedMenu].Id, LocalScreen);
          
          ClearLines (
            gColumn,
            LocalScreen.RightColumn - 1,
            gTopRow,
            gBottomRow,
            VE_NORMAL_MENU_TEXT);

          DsplayHotkeyInfo1 (LocalScreen);
        }
        
        SetColor (VE_NORMAL_MENU_TEXT);
        break;
      }
      
    case 0:
      switch (Key.ScanCode) {
        
      case SCAN_UP:
        if(Row <= gTopRow)
          Row = gTopRow + ItemNum - 1;
        else
          Row--;

        SelectedItem = Row - gTopRow;
        break;
   
      case SCAN_DOWN:
        if(Row >= (gTopRow + ItemNum - 1)) 
          Row = gTopRow;
        else
          Row++;
  
        SelectedItem = Row -gTopRow;
        break;
        
      case SCAN_F1:
        if (gShowUtilVer == TRUE) {
          gShowUtilVer = FALSE;
        } else {
          gShowUtilVer = TRUE;
        }
        DsplayHotkeyInfo1 (LocalScreen);
        break;
      case SCAN_ESC:
        FreeMenuList ();
        SetColor (EFI_BACKGROUND_BLACK);
        gST->ConOut->ClearScreen (gST->ConOut);
        gST->ConOut->SetMode (gST->ConOut, OrgMode);
        EnCursor(TRUE);
        return EFI_SUCCESS;
        
      default:
        break;
      }
    }
  }while (TRUE);
}

/**
 Get RTC time data. Before reading every RTC data, it will check RTC update status
 to make sure every RTC data is correct.

 @param[out] ELPostDispVar          Pointer to output time data

 @retval EFI_SUCCESS            Get RTC time data successfully.
 @retval EFI_NOT_READY          RTC data is in update progress. Fail to get time data.
 @retval EFI_INVALID_PARAMETER  Pointer of output time data is NULL.
**/
EFI_STATUS
EFIAPI
GetTimeWithRtcUpdateCheckForDisplayElv (
  OUT EL_POST_DISPLAY_VARIABLE              *ELPostDispVar
  )
{
  RTC_REGISTER_A  RegisterA;
  UINTN           Index;
  UINTN           RtcComponentNum;
  RTC_COMPONENT   RtcComponent[] = {{9, 0}, // Year
                                    {8, 0}, // Month
                                    {7, 0}, // Day
                                    {4, 0}, // Hour
                                    {2, 0}, // Minute
                                    {0, 0}  // Second
                                    };

  if (ELPostDispVar == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  RtcComponentNum = sizeof(RtcComponent) / sizeof(RTC_COMPONENT);

  //
  // In order to preventing RTC data reading error upon RTC updating due to interrupt/SMI caused delay,
  // making RTC update checking before every RTC data reading to make sure every RTC data corrected
  //
  for (Index = 0; Index < RtcComponentNum; Index++) {
    RegisterA.Data = ReadCmos8 (10);
    if (RegisterA.Bits.Uip == 1) {
      MicroSecondDelay (10);
      return EFI_NOT_READY;
    }

    RtcComponent[Index].Data = ReadCmos8 (RtcComponent[Index].Address);
  }

  ELPostDispVar->Year   = (UINT16) (2000 + ((RtcComponent[0].Data >> 4) * 10) + (RtcComponent[0].Data &0x0F));
  ELPostDispVar->Month  = ((RtcComponent[1].Data >> 4) * 10) + (RtcComponent[1].Data &0x0F);
  ELPostDispVar->Day    = ((RtcComponent[2].Data >> 4) * 10) + (RtcComponent[2].Data &0x0F);
  ELPostDispVar->Hour   = ((RtcComponent[3].Data >> 4) * 10) + (RtcComponent[3].Data &0x0F);
  ELPostDispVar->Minute = ((RtcComponent[4].Data >> 4) * 10) + (RtcComponent[4].Data &0x0F);
  ELPostDispVar->Second = ((RtcComponent[5].Data >> 4) * 10) + (RtcComponent[5].Data &0x0F);

  return EFI_SUCCESS;
}

/**
 Check the existence of "ELPostDisp" variable.    

 Param[in, out] VariableExist    The boolean to stand for the existence of ELPostDisp.
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
CheckVariableExist (
  IN OUT BOOLEAN                         *VariableExist
  )
{
  EFI_STATUS                   Status;
  CHAR16                       *VarName = EVENT_LOG_POST_DISPLAY_VARIABLE_NAME;
  UINTN                        VarSize;

  VarSize = sizeof (EL_POST_DISPLAY_VARIABLE);
  Status = gRT->GetVariable(
                  VarName,
                  &gH2OEventLogPostDisplayDxeVariableGuid,
                  NULL,
                  &VarSize,
                  &gElPostDispVar
                  );
  DEBUG((EFI_D_INFO, "Get ELPostDisp variable: %r \n", Status));

  if (!EFI_ERROR (Status)) {
    *VariableExist = TRUE;
  }
  
  return Status;
}

/**
 Create ELPostDisp variable.    

 Param[] void
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
CreateElPostDispVariable (
  void
  )
{
  EFI_STATUS                       Status;
  EL_POST_DISPLAY_VARIABLE         ELPostDispVar;
  CHAR16                           *VarName = EVENT_LOG_POST_DISPLAY_VARIABLE_NAME;

  GetTimeWithRtcUpdateCheckForDisplayElv (&ELPostDispVar);
  
  Status = gRT->SetVariable (
                  VarName,
                  &gH2OEventLogPostDisplayDxeVariableGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                  sizeof (EL_POST_DISPLAY_VARIABLE),
                  &ELPostDispVar
                  );
  DEBUG((EFI_D_INFO, "Set ELPostDisp variable: %r \n", Status));
  
  return Status;
}

/**
 Check the Filter Condition from PCD.    

 Param[] void
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetFilterCondition (
  VOID
  )
{
  EL_POST_DISP_FILTER           *Filter;
  UINTN                         Index;
    
//[-start-171212-IB08400542-modify]//
  Filter = (EL_POST_DISP_FILTER *) PcdGetPtr (PcdH2OEventsOnPostDisplayFilterList);
//[-end-171212-IB08400542-modify]//
  for (Index = 0;(Filter[Index].StorageBitMap != 0x00);Index++) {
    if ((Filter[Index].StorageBitMap & EVENT_STORAGE_BIOS) != 0) {
      gElPostDispStorage.BiosEnable = TRUE;
    }

    if ((Filter[Index].StorageBitMap & EVENT_STORAGE_BMC_SEL) != 0) {
      gElPostDispStorage.BmcSelEnable = TRUE;
    }

    if ((Filter[Index].StorageBitMap & EVENT_STORAGE_MEMORY) != 0) {
      gElPostDispStorage.MemoryEnable = TRUE;
    }
  }

  return EFI_SUCCESS;
}

/**
 Check if the event is match the filter Condition.    

 Param[in] EventData         
 
 @retval Boolean                  
*/
BOOLEAN
EFIAPI
CheckTimeWithVariable (
  IN STORAGE_EVENT_LOG_INFO                 *EventData
  )
{
  
  if (gElPostDispVar.Year < EventData->Year) {
    return TRUE;
  } else if (gElPostDispVar.Year > EventData->Year) {
    return FALSE;
  } 

  if (gElPostDispVar.Month < EventData->Month) {
    return TRUE;
  } else if (gElPostDispVar.Month > EventData->Month) {
    return FALSE;
  }

  if (gElPostDispVar.Day < EventData->Date) {
    return TRUE;
  } else if (gElPostDispVar.Day> EventData->Date) {
    return FALSE;
  }

  if (gElPostDispVar.Hour < EventData->Hour) {
    return TRUE;
  } else if (gElPostDispVar.Hour > EventData->Hour) {
    return FALSE;
  }

  if (gElPostDispVar.Minute < EventData->Minute) {
    return TRUE;
  } else if (gElPostDispVar.Minute > EventData->Minute) {
    return FALSE;
  }

  if (gElPostDispVar.Second < EventData->Second) {
    return TRUE;
  } else if (gElPostDispVar.Second > EventData->Second) {
    return FALSE;
  }

  return TRUE;
}

/**
 Check if the event is match the filter Condition in BIOS.    

 Param[in] EventData        Event Data.
 
 @retval Boolean                  
*/
BOOLEAN
EFIAPI
CheckBiosFilterCondition (
  IN STORAGE_EVENT_LOG_INFO                 *EventData
  )
{
  EL_POST_DISP_FILTER           *Filter;
  UINTN                         Index;
  EVENT_TYPE_ID                 EventID;

  
//[-start-171212-IB08400542-modify]//
  Filter = (EL_POST_DISP_FILTER *) PcdGetPtr (PcdH2OEventsOnPostDisplayFilterList);
//[-end-171212-IB08400542-modify]//
  for (Index = 0;(Filter[Index].StorageBitMap != 0);Index++) {
    if ((Filter[Index].StorageBitMap & EVENT_STORAGE_BIOS) != 0) {
      CopyMem (&EventID, &(Filter[Index].EventID), sizeof(EVENT_TYPE_ID));      
      if (CompareMem (&EventID, &(EventData->EventID), sizeof(EVENT_TYPE_ID)) == 0) {
        return TRUE;
      }
    }
  }

  //
  // No more filter condition can matched.
  //
  return FALSE;

}

/**
 Check if the event is match the filter Condition in MEMORY.    

 Param[in] EventData        Event Data.
 
 @retval Boolean                  
*/
BOOLEAN
EFIAPI
CheckMemoryFilterCondition (
  IN STORAGE_EVENT_LOG_INFO                 *EventData
  )
{
  EL_POST_DISP_FILTER           *Filter;
  UINTN                         Index;
  EVENT_TYPE_ID                 EventID;

  
//[-start-171212-IB08400542-modify]//
  Filter = (EL_POST_DISP_FILTER *) PcdGetPtr (PcdH2OEventsOnPostDisplayFilterList);
//[-end-171212-IB08400542-modify]//
  for (Index = 0;(Filter[Index].StorageBitMap != 0);Index++) {
    if ((Filter[Index].StorageBitMap & EVENT_STORAGE_MEMORY) != 0) {
      CopyMem (&EventID, &(Filter[Index].EventID), sizeof(EVENT_TYPE_ID));
      if (CompareMem (&EventID, &(EventData->EventID), sizeof(EVENT_TYPE_ID)) == 0) {
        return TRUE;
      }
    }
  }

  //
  // No more filter condition can matched.
  //
  return FALSE;

}

/**
 Check if the event is match the filter Condition in BMC SEL.    

 Param[in] EventData        Event Data.
 
 @retval Boolean                  
*/
BOOLEAN
EFIAPI
CheckBmcSelFilterCondition (
  IN STORAGE_EVENT_LOG_INFO                 *EventData
  )
{
  EL_POST_DISP_FILTER           *Filter;
  UINTN                         Index;
  EVENT_TYPE_ID                 EventID;

  
//[-start-171212-IB08400542-modify]//
  Filter = (EL_POST_DISP_FILTER *) PcdGetPtr (PcdH2OEventsOnPostDisplayFilterList);
//[-end-171212-IB08400542-modify]//
  for (Index = 0;(Filter[Index].StorageBitMap != 0);Index++) {
    if ((Filter[Index].StorageBitMap & EVENT_STORAGE_BMC_SEL) != 0) {
      CopyMem (&EventID, &(Filter[Index].EventID), sizeof(EVENT_TYPE_ID));
      if (CompareMem (&EventID, &(EventData->EventID), sizeof(EVENT_TYPE_ID)) == 0) {
        return TRUE;
      }
    }
  }

  //
  // No more filter condition can matched.
  //
  return FALSE;

}

/**
 Check if the event is match the filter Condition.    

 Param[in] StorageName      Storage where the event comes from. 
 Param[in] EventData        Event Data.
 
 @retval Boolean                  
*/
BOOLEAN
EFIAPI
CheckEventWithFilterCondition (
  IN CHAR16                                 *StorageName,
  IN STORAGE_EVENT_LOG_INFO                 *EventData
  )
{
  BOOLEAN                                FilterStatus;

  FilterStatus = FALSE;
  //
  // 1. Check the time with the variable. 
  //
  FilterStatus = CheckTimeWithVariable (EventData);
  if (FilterStatus == FALSE) {    
    return FALSE;
  }
  
  //
  // 2. Check with the filter conditions.
  //
  if ((!StrCmp(L"BIOS", StorageName)) && (gElPostDispStorage.BiosEnable == TRUE)) {
    // Check BIOS storage condition.
    FilterStatus = FALSE;
    FilterStatus = CheckBiosFilterCondition (EventData);
    return FilterStatus;
  }

  if ((!StrCmp(L"BMC SEL", StorageName)) && (gElPostDispStorage.BmcSelEnable == TRUE)) {
    // Check BMC SEL storage condition.
    FilterStatus = FALSE;
    FilterStatus = CheckBmcSelFilterCondition (EventData);
    return FilterStatus;
  }

  if ((!StrCmp(L"MEMORY", StorageName)) && (gElPostDispStorage.MemoryEnable == TRUE)) {
    // Check MEMORY storage condition.
    FilterStatus = FALSE;
    FilterStatus = CheckMemoryFilterCondition (EventData);
    return FilterStatus;
  }
  
  return FALSE;
}

/**
 Get Event Log from Filter Condition.    

 Param[in] StorageName      Storage where the event comes from.  
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventLogFromFilterConditionByStorage (
  IN CHAR16                                 *StorageName
  )
{
  EFI_STATUS                             Status;  
  UINTN                                  TotalEventNum;
  UINTN                                  FilteredEventNum;
  BOOLEAN                                *EventFilter;
  UINTN                                  Index;
  UINTN                                  Index2;
  STORAGE_EVENT_LOG_INFO                 *EventData;
  STORAGE_EVENT_LOG_INFO                 *FilteredEventData;

  TotalEventNum = 0;
  FilteredEventNum = 0;
  Status = gEventLog->RefreshDatabase (StorageName, &TotalEventNum);
  if (EFI_ERROR (Status) || (TotalEventNum == 0)) {
    return Status;
  }

  EventFilter = NULL;
  EventFilter = (BOOLEAN *) AllocateZeroPool (sizeof(BOOLEAN) * TotalEventNum);
  if (EventFilter == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG((EFI_D_INFO, "%s storage total event number: %d \n", StorageName, TotalEventNum));
    
  for (Index = 0; Index < TotalEventNum; Index++) {
    Status = gEventLog->GetEvent (StorageName, Index, (UINT8 **)&EventData);
    if (EFI_ERROR (Status)) {
      break;
    }

    EventFilter[Index] = CheckEventWithFilterCondition (StorageName, EventData);

    if (EventFilter[Index] == TRUE) {
      FilteredEventNum++;
    }
  }

  DEBUG((EFI_D_INFO, "FilteredEventNum: %d\n", FilteredEventNum));
  if (FilteredEventNum == 0) {
    FreePool (EventFilter);
    return EFI_NOT_FOUND;
  }    

  gIsFilterEvent = TRUE;
  
  //
  // Allocate memory space to store the filtered Event Data.
  //
  FilteredEventData = (STORAGE_EVENT_LOG_INFO *) AllocateZeroPool (sizeof(STORAGE_EVENT_LOG_INFO) * FilteredEventNum);
  if (EventFilter == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Index2 = 0;
  for (Index = 0; Index < TotalEventNum; Index++) {
    if (EventFilter[Index] == FALSE) {
      continue;
    }
    
    Status = gEventLog->GetEvent (StorageName, Index, (UINT8 **)&EventData);
    if (EFI_ERROR (Status)) {
      continue;
    }
    CopyMem (&(FilteredEventData[FilteredEventNum - Index2 - 1]), EventData, sizeof(STORAGE_EVENT_LOG_INFO));
    Index2++;
  }

  for (Index = 0; Index < MAX_STORAGE_SUPPORT; Index++) {
    if (MenuList[Index].FilteredEventAddress == 0) { 
      MenuList[Index].StorageString = NULL;
      MenuList[Index].StorageString = (CHAR16 *) AllocateZeroPool (sizeof(CHAR16) * StrLen (StorageName));
      if (MenuList[Index].StorageString == NULL) {
        break;
      }
//[-start-180724-IB08400617-modify]//
      StrCpyS (MenuList[Index].StorageString, MAX_STORAGE_NAME_NUM, StorageName);
//[-end-180724-IB08400617-modify]//
      MenuList[Index].FilteredEventAddress = (UINTN)FilteredEventData;
      MenuList[Index].FilteredEventNum = Index2;
      MenuList[Index].Id = ItemNum;

      MenuList[Index].ShowString = (CHAR16 *) AllocateZeroPool (sizeof(CHAR16) * MAX_BUFFER_SIZE);
      if (MenuList[Index].ShowString == NULL) {
        break;
      }
      
      UnicodeSPrint (MenuList[Index].ShowString, sizeof(CHAR16) * MAX_BUFFER_SIZE, L"Show %s last power-on Events ", StorageName);
      ItemNum++;
      break;
    }
  }
  
  for (Index = 0; Index < MAX_STORAGE_SUPPORT; Index++) {
    if (MenuList[Index].FilteredEventAddress != 0) {  
      DEBUG((EFI_D_INFO, "%s Storage, Filter events buffer: 0x%x, Num: %d \n", MenuList[Index].StorageString, MenuList[Index].FilteredEventAddress, MenuList[Index].FilteredEventNum));
    }
  }    
  
  return Status;
}

/**
 Get Event Log from Filter Condition.    

 Param[] void
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
GetEventLogFromFilterCondition (
  VOID
  )
{
  EFI_STATUS                             Status;  
  UINTN                                  Index;

  //
  // Get Event Log by supported storage names.
  //
  for (Index = 0; Index < MAX_STORAGE_SUPPORT; Index++) {
    Status = GetEventLogFromFilterConditionByStorage (gStorageSupported[Index]);
    DEBUG((EFI_D_INFO, "Get %s Event Log from filter condition: %r \n", gStorageSupported[Index], Status));
  }
  
  return EFI_SUCCESS;
}

/**
 Entry point of this driver. Install Event Log Viewer protocol into DXE.

 @param[in] ImageHandle       Image handle of this driver.
 @param[in] SystemTable       Global system service table.          
 
 @retval EFI Status                  
*/
EFI_STATUS
EFIAPI
EventLogPostDisplayDxeEntry (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  ) 
{
  EFI_STATUS                   Status;
  BOOLEAN                      VariableExist;
  EFI_EVENT                    ReadyToBootEvent;

  gEventLog = NULL;
  gShowUtilVer  = FALSE;
  gIsFilterEvent = FALSE;

  //
  // 1. Check if variable exists. 
  //
  VariableExist = FALSE;
  Status = CheckVariableExist (&VariableExist);
  if (EFI_ERROR (Status)) {
    //
    // Check variable failure.
    // Just update/create the variable with new content, and then continue to POST.
    //
    CreateElPostDispVariable ();
    return EFI_SUCCESS;
  } else {
    CreateElPostDispVariable ();
    if (VariableExist == FALSE) {
      // ELPostDisp variable doesn't exist, so create it and then continue to POST.
      return EFI_SUCCESS;
    } 
  }

  //
  // The ELPostDisp variable exists, prepare to display the Event Log.
  //
  
  //
  // 2. Get Fileter condition.
  //
  Status = GetFilterCondition ();
  if (EFI_ERROR (Status)) {
    return Status;
  }  

  //
  // 3. Get Event Log with filter condition
  //
  Status = InitMenu();
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "InitMenu(): %r \n", Status));
    return Status;
  }
  
  Status = gBS->LocateProtocol (&gH2OEventLogProtocolGuid, NULL, (VOID **)&gEventLog);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  Status = GetEventLogFromFilterCondition ();
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "GetEventLogFromFilterCondition(): %r \n", Status));
    return Status;
  }

  if (gIsFilterEvent == TRUE) {
    //
    // There are some events which match the filter condition, then display the Event Log Viewer.
    //
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               ExecuteFilterEventLogViewer,
               NULL,
               &ReadyToBootEvent
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
  return EFI_SUCCESS;
}

