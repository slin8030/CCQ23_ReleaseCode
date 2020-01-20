/** @file
  Provide PS2 policy protocol.

;******************************************************************************
;* Copyright (c) 2014 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************

Module Name:

  KeyboardSetup.c

Abstract:

  D845GRG Platform Initialization Driver.

--*/

#include "PlatformDxe.h"
#include <Protocol/Ps2Policy.h>

//
// Port address
//
#define CONFIG_PORT               0x02E
#define INDEX_PORT                0x02E
#define DATA_PORT                 INDEX_PORT + 1

//[-start-161116-IB11270167-modify]//
//#ifdef WIN7_SUPPORT
//BOOLEAN           Ps2PortSwapDone = FALSE;
//BOOLEAN           Ps2PortSwapState = FALSE;
//EFI_STATUS        mInitializedStatus;
//#else
BOOLEAN           Ps2PortSwapDone = TRUE;
BOOLEAN           Ps2PortSwapState = FALSE;
EFI_STATUS        mInitializedStatus = EFI_SUCCESS;
//#endif
//[-end-161116-IB11270167-modify]//

EFI_STATUS
EFIAPI
Ps2PortSwap (
  IN  EFI_HANDLE  Handle
  );

EFI_PS2_POLICY_PROTOCOL       mPs2PolicyData = {
  EFI_KEYBOARD_NUMLOCK,
  (EFI_PS2_INIT_HARDWARE) Ps2PortSwap,
};


UINT8
KeyReadDataRegister(
  )
/*++

Routine Description:

  Read Keyboard Data Register

Arguments:

  None

Returns:

  Data Register

--*/
{
  return IoRead8(0x60);
}

UINT8
KeyReadStatusRegister(
  )
/*++

Routine Description:

  Read Keyboard Status Register

Arguments:

  None

Returns:

  Status Register

--*/
{
  return IoRead8(0x64);
}

VOID
KeyWriteCommandRegister(
  IN UINT8              Data
  )
/*++

Routine Description:

  Write Command Register

Arguments:

  Data to write

Returns:

  None

--*/
{
  IoWrite8(0x64, Data);
}

VOID
KeyWriteDataRegister(
  IN UINT8              Data
  )
/*++

Routine Description:

  Write Data Register

Arguments:

  Data to write

Returns:

  None

--*/
{
  IoWrite8(0x60, Data);
}

VOID
KeyboardRead(
  OUT UINT8            *Data
  )
/*++

Routine Description:

  Read Keyboard Data

Arguments:

  Pointer to where the data to read into

Returns:

  Put the data to inside the pointer

--*/
{
  UINT32   TimeOut;

  TimeOut = 0;

  //
  //wait till output buffer full then perform the read
  //
  for ( TimeOut = 0; TimeOut < 1000000; TimeOut += 30) {
    if (KeyReadStatusRegister() & 0x01) {
      *Data = KeyReadDataRegister();
      break;
    }
    gBS->Stall(30);
  }

  return;
}

VOID
KeyboardWrite(
  IN UINT8              Data
  )
/*++

Routine Description:

  Write Keyboard Data

Arguments:

  Data to write

Returns:

--*/
{
  UINT32      TimeOut;
  UINT32      RegEmptied;

  TimeOut = 0;
  RegEmptied = 0;

  //
  //wait for input buffer empty
  //
  for ( TimeOut = 0; TimeOut < 65536; TimeOut += 30) {
    if (!(KeyReadStatusRegister() & 0x02)) {
      break;
    }
    gBS->Stall(30);
  }

  //
  //Write it
  //
  KeyWriteDataRegister(Data);

  return;
}

VOID
KeyboardCommand  (
  IN UINT8              Data
  )
/*++

Routine Description:

  Write Keyboard Command

Arguments:

  Data to write

Returns:

--*/
{
  UINT32      TimeOut;

  TimeOut = 0;

  //
  //Wait For Input Buffer Empty
  //
  for ( TimeOut = 0; TimeOut < 65536; TimeOut += 30) {
    if (!(KeyReadStatusRegister() & 0x02)) {
      break;
    }
    gBS->Stall(30);
  }

  //
  //issue the command
  //
  KeyWriteCommandRegister(Data);

  //
  //Wait For Input Buffer Empty again
  //
  for ( TimeOut = 0; TimeOut < 65536; TimeOut += 30) {
    if (!(KeyReadStatusRegister() & 0x02)) {
      break;
    }
    gBS->Stall(30);
  }

  return;
}

VOID
SwapPs2Command(
  IN EFI_CPU_IO2_PROTOCOL             *CpuIo
  )
/*++

Routine Description:

  Swap Ps2 devices

Arguments:

Returns:

--*/
{
  UINT8             Data;
  UINT8                           Index;
  EFI_STATUS       Status;
  //
  // Swap it and then reset
  //

  //
  // Enter SIO config mode
  //
  STATIC EFI_BOOT_SCRIPT_SAVE_PROTOCOL  *mBootScriptSave=NULL;
  Status = gBS->LocateProtocol (&gEfiBootScriptSaveProtocolGuid, NULL, (VOID **)&mBootScriptSave);
  ASSERT_EFI_ERROR (Status);
  
  ASSERT_EFI_ERROR(mBootScriptSave == NULL);
  
  Index = 0x87;
  CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, INDEX_PORT, 1, &Index);
  CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, INDEX_PORT, 1, &Index);
#if 0
  SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   INDEX_PORT, 
                   1, 
                   &Index);
  SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   INDEX_PORT, 
                   1, 
                   &Index);
#endif
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           INDEX_PORT,
                           1, 
                           &Index);
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           INDEX_PORT,
                           1, 
                           &Index);    
 
  
  //
  // Device ID register 0x20.
  //
  Index = 0x20;
  CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, INDEX_PORT, 1, &Index);
  CpuIo->Io.Read (CpuIo, EfiCpuIoWidthUint8, DATA_PORT, 1, &Data);
  //
  // Nuvoton W83627DHG
  //
 
   if(Data == 0xB0) {  
    // Logical device number register is 7
    Index = 0x07;
    CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, INDEX_PORT, 1, &Index);
    
#if 0    
    SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   INDEX_PORT, 
                   1, 
                   &Index);
#endif    
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           INDEX_PORT,
                           1, 
                           &Index);    
   
    // ACPI is logical device 0x0A
    Data = 0x0A;
    CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, DATA_PORT, 1, &Data);
  
#if 0    
    SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   DATA_PORT, 
                   1, 
                   &Data);
#endif    
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           DATA_PORT,
                           1, 
                           &Data);    
    
    // Set SWAP bit register 0xE0, bit2
    Index = 0xE0;
    CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, INDEX_PORT, 1, &Index);
     
#if 0    
    SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   INDEX_PORT, 
                   1, 
                   &Index);
#endif
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           INDEX_PORT,
                           1, 
                           &Index);        
     
    CpuIo->Io.Read (CpuIo, EfiCpuIoWidthUint8, DATA_PORT, 1, &Data);
    Data |= 0x04;
    CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, DATA_PORT, 1, &Data);
 
#if 0    
    SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   DATA_PORT, 
                   1, 
                   &Data);
#endif    
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           DATA_PORT,
                           1, 
                           &Data);     
   
    Ps2PortSwapState = FALSE;
  } else {
    KeyboardCommand (0xc9);
  }
///  KeyboardCommand (0x60);
///  KeyboardWrite (0xff);
///  KeyboardRead (&Data);
   
  //
  // Exit SIO config mode
  //
  Index = 0xaa;
  CpuIo->Io.Write (CpuIo, EfiCpuIoWidthUint8, INDEX_PORT, 1, &Index);
#if 0  
  SCRIPT_IO_WRITE (EFI_ACPI_S3_RESUME_SCRIPT_TABLE, 
                   EfiBootScriptWidthUint8, 
                   INDEX_PORT, 
                   1, 
                   &Index);
#endif
    mBootScriptSave->Write (
                           mBootScriptSave, 
                           FRAMEWORK_EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
                           EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                           EfiBootScriptWidthUint8,
                           INDEX_PORT,
                           1, 
                           &Index); 

  

  return;
}

VOID
InitPs2PlatformPolicy(
  )
/*++

Routine Description:

  Detect which device is connected to which port.

Arguments:

Returns:
  EFI_SUCCESS

--*/
{

  EFI_HANDLE              Handle;
  EFI_STATUS              Status;

//[-start-160531-IB08450347-add]//
//[-start-170502-IB07400864-remove]//
//  if(mSystemConfiguration.Win8FastBoot == 0) {
//    //
//    // Enable gEfiIntelFrameworkModulePkgTokenSpaceGuid PcdFastPS2Detection to enable quick PS2 device detection
//    //
//    PcdSetBool (PcdFastPS2Detection, TRUE);
//  }
//[-end-170502-IB07400864-remove]//
//[-end-160531-IB08450347-add]//

  if (!mSystemConfiguration.NumLock ) {
    mPs2PolicyData.KeyboardLight      &= ~EFI_KEYBOARD_NUMLOCK;
  }

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gEfiPs2PolicyProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPs2PolicyData
                  );
  ASSERT_EFI_ERROR(Status);

}

EFI_STATUS
Ps2PortSwap (
  IN  EFI_HANDLE  Handle
  )
/*++

Routine Description:

  Detect which device is connected to which port.

Arguments:

Returns:
  EFI_SUCCESS

--*/
{
  UINT8                   Primary;
  UINT8                   Secondary;
  UINT8                   Data, KBCCommandByte;
  EFI_STATUS              Status;
  UINTN                   Size;
  BOOLEAN                 Ps2PortSwapVariable;
  EFI_CPU_IO2_PROTOCOL     *CpuIo;
  
#ifndef WIN7_SUPPORT
  if (!PcdGetBool (PcdFastPS2Detection)) {
#endif  
  //If this routine has already been executed fully, just return execution status
  if (Ps2PortSwapDone) {
    return mInitializedStatus;
  }

  Status = gBS->LocateProtocol (&gEfiCpuIo2ProtocolGuid, NULL, (VOID **)&CpuIo);
  ASSERT_EFI_ERROR(Status);

  // If Controller Handle is NULL, just return that PS/2 device presence status
  if (Handle == NULL)
  {
    return EFI_SUCCESS;
  }

  // Save contents of keyboard controller command byte
  KeyboardCommand (0x20);
  KeyboardRead (&KBCCommandByte);

  //
  // Disable secondary (Auxiliary) port
  //
  KeyboardCommand (0xA7);

  //
  // Check if something is connected to primary port
  //
  KeyboardWrite (0xee);
  KeyboardRead (&Primary);

  if ((Primary == 0xfa) || (Primary == 0xee)) {
    // Something was detected in Primary port

    //
    // Read rest of the bytes
    //
///    KeyboardRead (&Data);

    //
    // Check if keyboard in Primary port
    //
///    KeyboardWrite (0xee);
///    KeyboardRead (&Data);
    if (Primary != 0xfa) {
      //mPs2PolicyData.KeyboardPresent = TRUE;
    } else {
      // Turn off echo to mouse
      KeyboardWrite (0xec);
      KeyboardRead (&Data);
     // mPs2PolicyData.MousePresent = TRUE;
      Ps2PortSwapState = TRUE;
    }
  }

  //
  // Disable primary port
  //
  KeyboardCommand (0xAD);

  //
  // Enable secondary (Auxiliary) port
  //
  KeyboardCommand (0xA8);

  //
  // Check if something is connected to Secondary port
  //
  KeyboardCommand (0xd4);
  KeyboardWrite (0xee);
  KeyboardRead (&Secondary);

  if ((Secondary == 0xfa) || (Secondary == 0xee)) {
    // Something was detected in Secondary port

    //
    // Read rest of the bytes
    //
///    KeyboardRead (&Data);

    //
    // Check if keyboard in Secondary port
    //
    //Select secondary port
///    KeyboardCommand (0xd4);
///    KeyboardWrite (0xee);
///    KeyboardRead (&Data);
    if (Secondary != 0xfa) {
#if 0
      if (mPs2PolicyData.KeyboardPresent != TRUE) {
        // Keyboard detected in secondary port
        mPs2PolicyData.KeyboardPresent = TRUE;
        Ps2PortSwapState = TRUE;
      }
#endif
    } else {
      //Select secondary port
      KeyboardCommand (0xd4);
      // Turn off echo to mouse
      KeyboardWrite (0xec);
      KeyboardRead (&Data);
      //mPs2PolicyData.MousePresent = TRUE;
      Ps2PortSwapState = FALSE;
    }
  }

  //
  // Enable primary port
  //
  KeyboardCommand (0xAE);

  if (Ps2PortSwapState) {
    SwapPs2Command(CpuIo);
  }
#if 0
  if (!mPs2PolicyData.KeyboardPresent) {
    //
    // PS/2 Keyboard not detected.
    //
  }
#endif
  Size = sizeof(BOOLEAN);
  Status = gRT->GetVariable(L"Ps2Swap",
                            &gEfiGlobalVariableGuid,
                            NULL,
                            &Size,
                            &Ps2PortSwapVariable);

  if (Status == EFI_NOT_FOUND) {
      Size = sizeof(BOOLEAN);
      gRT->SetVariable(L"Ps2Swap",
                       &gEfiGlobalVariableGuid,
                       EFI_VARIABLE_BOOTSERVICE_ACCESS|EFI_VARIABLE_NON_VOLATILE,
                       Size,
                       &Ps2PortSwapState);
  } else {
    if (Ps2PortSwapState != Ps2PortSwapVariable) {
      Size = sizeof(BOOLEAN);
      gRT->SetVariable(L"Ps2Swap",
                       &gEfiGlobalVariableGuid,
                       EFI_VARIABLE_BOOTSERVICE_ACCESS|EFI_VARIABLE_NON_VOLATILE,
                       Size,
                       &Ps2PortSwapState);
    }
  }

  //
  // Do the PS2 port swapping code only once
  //
  Ps2PortSwapDone = TRUE;

  // Restore contents of keyboard controller command byte
  KeyboardCommand (0x60);
  KeyboardWrite (KBCCommandByte);
  mInitializedStatus = EFI_SUCCESS;
#ifndef WIN7_SUPPORT  
  }
#endif
  return  EFI_SUCCESS;
}
