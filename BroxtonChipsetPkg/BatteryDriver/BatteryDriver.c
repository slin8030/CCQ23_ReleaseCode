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
#include <ChipsetSetupConfig.h>
#include <Library/DxeInsydeChipsetLib.h>
//
//This function shows animation and keeps waiting until BatteryDevice->gStatus
//of context becomes 0xFF. BatteryDevice->gStatus is updated by
//timer event handler when state of charge reaches required 
//minimum charge level.
//

EFI_STATUS
WaitTillCharged (
  BATTERY_DEV          *Context
  )
{
  EFI_STATUS              Status = EFI_SUCCESS; 
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL       *ConOut;
  EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsOutput;
  UINTN                  BlockHeight;
  UINTN                  BlockWidth;
  UINTN                  BlockNum;
  UINTN                  PosX;
  UINTN                  PosY;
  UINT32                SizeOfX = 0;
  UINT32                SizeOfY = 0;
  //UINT8                  RemainingCharge=0;
  UINTN                  mStoMaxRows;
  UINTN                  mStoMaxColumns;
  UINTN                  i = 0;
  BATTERY_DEV              *BatteryDevice = NULL;
  UINT16                Data = 0;

  EFI_GRAPHICS_OUTPUT_BLT_PIXEL        mGraphicsColors[16] = {
  //
  // B    G    R   reserved
  //
  {0x00, 0x00, 0x00, 0x00},  // BLACK
  {0x98, 0x00, 0x00, 0x00},  // LIGHTBLUE
  {0x00, 0x98, 0x00, 0x00},  // LIGHGREEN
  {0x98, 0x98, 0x00, 0x00},  // LIGHCYAN
  {0x00, 0x00, 0x98, 0x00},  // LIGHRED
  {0x98, 0x00, 0x98, 0x00},  // MAGENTA
  {0x00, 0x98, 0x98, 0x00},  // BROWN
  {0x98, 0x98, 0x98, 0x00},  // LIGHTGRAY
  {0x30, 0x30, 0x30, 0x00},  // DARKGRAY - BRIGHT BLACK
  {0xff, 0x00, 0x00, 0x00},  // BLUE
  {0x00, 0xff, 0x00, 0x00},  // LIME
  {0xff, 0xff, 0x00, 0x00},  // CYAN
  {0x00, 0x00, 0xff, 0x00},  // RED
  {0xff, 0x00, 0xff, 0x00},  // FUCHSIA
  {0x00, 0xff, 0xff, 0x00},  // YELLOW
  {0xff, 0xff, 0xff, 0x00}   // WHITE
};
  BlockWidth  = 100;
  BlockHeight = 200;
  BlockNum    = 1;
  PosX        = 100;
  PosY        = 100;

  BatteryDevice = (BATTERY_DEV *) Context;

  gBS->LocateProtocol (&gEfiSimpleTextOutProtocolGuid,  NULL, (VOID**) &ConOut);
  if (EFI_ERROR(Status)) {
    return Status;  
  }
  gBS->LocateProtocol (&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**) &GraphicsOutput);
  if (EFI_ERROR(Status)) {   
    return Status;
  }

  Status = gBS->HandleProtocol (
                  gST->ConsoleOutHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput
                  );

  if (EFI_ERROR (Status)) {
    Print(L"\nGOP could not be located!\n");
    gBS->Stall(10*1000*1000);
    return EFI_UNSUPPORTED;
  }

  gST->ConOut->EnableCursor (gST->ConOut, FALSE); //Disable cursor
  gST->ConOut->ClearScreen (gST->ConOut); //Clear screen
 
  if (GraphicsOutput != NULL) {
    SizeOfX = GraphicsOutput->Mode->Info->HorizontalResolution;
    SizeOfY = GraphicsOutput->Mode->Info->VerticalResolution;
  }
   // Center Position
  PosX = (SizeOfX - BlockWidth) / 2;
  PosY = (SizeOfY - BlockHeight) / 2;

  do {
 
    Status = GraphicsOutput->Blt (
                               GraphicsOutput,
                               &mGraphicsColors[15], // White
                               EfiBltVideoFill,
                               0,
                               0,
                               (PosX+45),
                               (PosY-4),
                               10,
                               10,
                               (BlockWidth) * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                               );
    
    Status = GraphicsOutput->Blt (
                               GraphicsOutput,
                               &mGraphicsColors[15], // White
                               EfiBltVideoFill,
                               0,
                               0,
                               PosX,
                               PosY,
                               BlockWidth - 1,
                               BlockHeight,
                               (BlockWidth) * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                               );

    Status = GraphicsOutput->Blt (
                               GraphicsOutput,
                               &mGraphicsColors[0],
                               EfiBltVideoFill,
                               0,
                               0,
                               (PosX+2),
                               (PosY+2),
                               (BlockWidth - 1 -4),
                               (BlockHeight-4),
                               (BlockWidth) * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                               );

    for(i=0; i<100; i = i + 5){
  
      Status = GraphicsOutput->Blt (
                                 GraphicsOutput,
                                 &mGraphicsColors[1],  // For Light Blue
                                 EfiBltVideoFill,
                                 0,
                                 0,
                                 (PosX+2),
                                 (PosY+2+(BlockHeight-(2*i))),
                                 (BlockWidth - 1 -4),
                                 ((2*i)-4),
                                 (BlockWidth) * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
                                 );
   
      gBS->Stall(1000000);

      Data = UlpmcGetChargingStatus();

      if((Data != 0x1 && Data != 0x2 && Data != 0x3) && Data == 0x0){
        //Charger got disconnected. Stop showing animation.
        break;
      }
    }//End of for loop

    gST->ConOut->ClearScreen (gST->ConOut);

  }while(BatteryDevice->gStatus != 0xFF);//End of do-while loop
   
  gST->ConOut->ClearScreen (gST->ConOut);
  gST->ConOut->QueryMode (gST->ConOut, gST->ConOut->Mode->Mode, &mStoMaxColumns, &mStoMaxRows);
  gST->ConOut->SetCursorPosition (gST->ConOut, (mStoMaxColumns/2-10), (mStoMaxRows-5));

   return EFI_SUCCESS;
}

//
//This timer event handler gets called every 2 seconds.
//It is registered as running under TPL_CALLBACK for 
//using i2c driver.
//

VOID
EFIAPI
BatteryTimerHandler (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  BATTERY_DEV     *BatteryDevice = NULL;
  EFI_STATUS    Status = EFI_SUCCESS;
  UINT16      SoC = 0;
  
  BatteryDevice = (BATTERY_DEV *) Context;

  if (BatteryDevice == NULL){
    DEBUG((EFI_D_WARN, "Context is null\n"));
    return;
  }

  SoC = UlpmcGetStateofCharge();
  //Print(L"\nState of charge = %d\n", SoC);

  if(SoC >= BatteryDevice->gCriticalBatteryLimit){
    Status = gBS->CloseEvent(BatteryDevice->TimerEvent);
    BatteryDevice->gStatus = 0xFF;
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "BatteryTimerHandler() CloseEvent Failed\n"));
    } else {
      //Print(L"\nCloseEvent() success!\n");
    }
  }

  return;
}

//
// Entrypoint of the driver 
//

EFI_STATUS
EFIAPI 
BatteryDriverEntry (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
{
  EFI_STATUS      Status = EFI_SUCCESS;
  BATTERY_DEV      *BatteryDevice = NULL;
  UINT16        SoC = 0;
  UINT8        Data = 0;
  UINTN        VarSize = 0;
  UINT8        mCriticalBatteryLimitFeature = 0;
  UINT8        mCriticalBatteryLimit = 0;
  CHIPSET_CONFIGURATION    *mSystemConfiguration;

  VarSize = PcdGet32 (PcdSetupConfigSize);
  mSystemConfiguration = (CHIPSET_CONFIGURATION *) AllocatePool(VarSize);
  
  Status = gRT->GetVariable(
                   SETUP_VARIABLE_NAME,
                   &gSystemConfigurationGuid,
                   NULL,
                   &VarSize,
                   mSystemConfiguration
                   );
  if (EFI_ERROR (Status)) {
    Status = GetChipsetSetupVariableDxe (mSystemConfiguration, sizeof (CHIPSET_CONFIGURATION));  
  }

  if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "BatteryDriverEntry() GetVariable Failed\n"));
      return Status;
  }

  //
  // Proceed only if the Critical Battery Limit feature is enabled in setup
  //

  mCriticalBatteryLimitFeature = mSystemConfiguration->CriticalBatteryLimitFeature;

  if (mCriticalBatteryLimitFeature == 0x1) {

    mCriticalBatteryLimit = mSystemConfiguration->CriticalBatteryLimit; 

    if (mSystemConfiguration) {
      FreePool(mSystemConfiguration);
    }
    
  Data = UlpmcGetChargingStatus();
  SoC = UlpmcGetStateofCharge();

  if(SoC >= mCriticalBatteryLimit){
    DEBUG ((EFI_D_INFO, "BatteryDriverEntry() - Battery Charge = %x\n", Data));
    Print(L"\nBattery charge is above critical. Limit is %d. Current Battery Charge is %d.\n", mCriticalBatteryLimit, SoC);
  }else{

  Print(L"\nBattery charge is below critical. Limit is %d. Current Battery Charge is %d.\n", mCriticalBatteryLimit, SoC);

  if(Data == 0x0){
  Print(L"\nPlease plugin the charger and power the system.\n");
  gBS->Stall(10*1000000);
  // Power down the system here
  gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
  return EFI_SUCCESS;
  }

  // If we are here, then charger should be connected. Confirm it.
  if(Data != 0x1 && Data != 0x2 && Data != 0x3){
  Print(L"\nPlease plugin the charger and power the system.\n");
  gBS->Stall(10*1000000);
  // Power down the system here
  gRT->ResetSystem (EfiResetShutdown, EFI_SUCCESS, 0, NULL);
  return EFI_SUCCESS;
  }

  // Charger is connected.
  Print(L"\nPlease wait while Battery is Charged to minimum limit of %d\n", mCriticalBatteryLimit);
  gBS->Stall(10*1000000);

  //
  // Setup a periodic timer, to read state of charge periodically
  //

  BatteryDevice = AllocateZeroPool (sizeof (BATTERY_DEV));

  BatteryDevice->gCriticalBatteryLimit = mCriticalBatteryLimit;

  Status = gBS->CreateEvent (
          EVT_TIMER | EVT_NOTIFY_SIGNAL,
          TPL_CALLBACK,
          BatteryTimerHandler,
          BatteryDevice,
          &BatteryDevice->TimerEvent
          );

  if (EFI_ERROR (Status)) {
  DEBUG ((EFI_D_INFO, "BatteryDriverEntry() CreateEvent Failed\n"));
  }else{
  //Print(L"\nCreateEvent() success!\n");
  }

  Status = gBS->SetTimer (
          BatteryDevice->TimerEvent,
          TimerPeriodic,
          BUTTON_ARRAY_TIMER_INTERVAL
          );

  if (EFI_ERROR (Status)) {
  DEBUG ((EFI_D_INFO, "BatteryDriverEntry() SetTimer Failed\n"));
  }else{
  //Print(L"\nSetTimer() success!\n");
  }

  Status = WaitTillCharged(BatteryDevice);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "BatteryDriverEntry() WaitTillCharged Failed\n"));
  }else{
  //Print(L"\nWaitTillCharged() success!\n");
  }

  }//End of else

  FreePool(BatteryDevice);

    }else{
      DEBUG ((EFI_D_INFO, "BatteryDriverEntry() - CriticalBatteryFeature is not enabled in setup\n"));
    }

  return EFI_SUCCESS;
}
