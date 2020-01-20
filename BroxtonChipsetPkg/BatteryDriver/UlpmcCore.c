/** @file

Copyright (c) 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
**/          

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/Pmiclib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
//#include "PnPVerifierByt.h"
//#include "PnPAppVersions.h"

//#include <WriteFile.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/EfiShellInterface.h>
#include <Protocol/EfiShellParameters.h>
//#include <Print.h>

#include <Uefi.h>
//#include <UlpmcMfgTestUtility.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DriverLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Protocol/I2cBus.h>

#include "Ulpmc.h"
//#include "Utilities.h"
//#include "Tests.h"
//#include "Screen.h"

EFI_INPUT_KEY Key;
//BOOLEAN Accept = FALSE;
CONST CHAR16* FILENAME;
CHAR16 FILENAME_final[128];
BOOLEAN FILENAME_CHECK = FALSE;
/*CHAR16  TempUnicodeStr[256];
CHAR8   TempAsciiStr[256];*/
EFI_FILE       *FilePtr;
UINTN in;
UINT8* Buffer = NULL;
EFI_SYSTEM_TABLE *SystemTable = NULL;

BOOLEAN IsListCommand = FALSE;
BOOLEAN FastModeSpecified = FALSE;
UINT8 ReadBuffer [I2C_READ_MAX];
UINTN RequestTimeout = I2C_TIMEOUT_DEFAULT;

/*Function that writes 0x0 to offset 0x0 and reads specified number of bytes from specified offset*/

EFI_STATUS 
ReadByteUseEfiI2CBusProtocol (UINT8 BusNo, UINT8 Slaveaddress, UINT8 Offset, UINTN ReadBytes, UINT8 *BufferForReadData)
{
  
    EFI_STATUS Status = EFI_SUCCESS;
    PI2CCommandParam CommandParam = NULL;
    UINTN RequestType = 0;   //0. No request; 1. read request; 2. write request. 3. rw request.

    UINTN Index = 0;
    EFI_HANDLE *HandleArray = NULL;
    UINTN HandleArrayCount =0;
    EFI_I2C_BUS_PROTOCOL *I2cBusProtocol = NULL;
    EFI_I2C_STACK_REQUEST_PACKET Request;
    UINT8 *Wbuf = NULL;
    CHAR8 AcpiID[I2C_ACPI_ID_LEN+1];
    UINT8 *BufferDump = NULL;

    BufferDump = AllocateZeroPool(90);

    Wbuf = AllocateZeroPool(1);

    *Wbuf = 0;

    CommandParam = AllocateZeroPool(sizeof(I2CCommandParam));

    if(CommandParam == NULL){
      Print(L"\nFailed to allocate resource for parsing command.\n");
      Status = EFI_ABORTED;
      goto _end;
    }
    /*
      if(!CheckParams(Argc, Argv, CommandParam)){
          goto _end;
      }
    */
    //Override
    CommandParam->ControllerID = BusNo;
    CommandParam->SlaveAddress = Slaveaddress;
    CommandParam->ReadLength = 90;
    CommandParam->WriteLength = 1;
    
    //Print(L"Received command: Controller ID: %d, SlaveAddress: %d, ReadLength: %d, WriteLength: %d.\n", CommandParam->ControllerID, 
       // CommandParam->SlaveAddress, CommandParam->ReadLength, CommandParam->WriteLength);

    if(CommandParam->ControllerID>6){
        Print(L"\nInvalid controller ID. aborting...\n");
        Status = EFI_ABORTED;
        goto _end;
    }

    //
    //For TalkI2C utility, the ACPI path is always in the format of I2C0X\\SFFFF before sending request to driver.
    //where X is 0-6
    //

    AsciiStrCpy(AcpiID, DID_ACPI_ID_PREFIX); 
    AcpiID[4] = '0'+CommandParam->ControllerID;
    AsciiStrCpy(AcpiID+5, DID_ACPI_ID_SUFFIX);
    if(FastModeSpecified == TRUE){
      AsciiStrCpy(AcpiID+11, DID_ACPI_ID_SUFFIX_400K);
    }


    //
    //Proceed to send command to I2C driver. For the tool, we always use synchronized mode.
    //

    
    Status = gBS->LocateHandleBuffer( ByProtocol, 
                                      &gEfiI2cBusProtocolGuid,
                                      NULL,
                                      &HandleArrayCount,
                                      &HandleArray);
    if(EFI_ERROR(Status)){
        Print(L"\nFailed to locate i2c bus protocol.\n");
        goto _end;
    }

    for ( Index = 0; HandleArrayCount > Index; Index ++ ) {
      //
      //  Determine if the device is available
      //
      if ( NULL != DlAcpiFindDeviceWithMatchingCid ( HandleArray [ Index ],
                      0,
                      (CONST CHAR8 *)AcpiID
                    )) {
        //
        //  The device was found
        //
        Status = gBS->OpenProtocol ( HandleArray [ Index ],
                                     &gEfiI2cBusProtocolGuid,
                                     (VOID **)&I2cBusProtocol,
                                     NULL,
                                     NULL,
                                     EFI_OPEN_PROTOCOL_GET_PROTOCOL );
        break;
      }
    }
    //
    //  Done with the handle array
    //
   gBS->FreePool ( HandleArray );
  

  if(NULL == I2cBusProtocol){
      Print(L"\nError -- I2C device not found.\n");
      goto _end;
  }
  else{
      RequestType += CommandParam->ReadLength>0?1:0;
      RequestType += CommandParam->WriteLength>0?2:0; //Should not be a problem
  }



  //Print(L"Sending Request.....\r\n");
  Request.ReadBytes   = CommandParam->ReadLength;
  Request.ReadBuffer  = Request.ReadBytes>0?BufferDump:NULL;
  Request.WriteBytes  = CommandParam->WriteLength;
  Request.WriteBuffer = Request.WriteBytes>0?Wbuf:NULL;
  Request.Timeout = (UINT32)RequestTimeout;
   // Print(L"Received command - right call: Controller ID: %d, SlaveAddress: %d, ReadLength: %d, WriteLength: %d.\n", CommandParam->ControllerID, 
     //   CommandParam->SlaveAddress, CommandParam->ReadLength, CommandParam->WriteLength);
  Status = I2cBusProtocol->StartRequest ( I2cBusProtocol,
                                                    (CommandParam->SlaveAddress)|0x400,
                                                    NULL,
                                                    &Request,
                                                    NULL );
  if ( EFI_ERROR ( Status )) {
      if ( EFI_DEVICE_ERROR == Status ) {
        Print ( L"\nERROR - This platform does not support this device!\r\n" );
        goto _end;
      } else {
        Print ( L"\nERROR - I2C synchronous request error, Status: %r\r\n", Status );
        goto _end;
      }
   } 
   else {
   //  Print ( L"INFO - Synchronous I2C request completed successfully!\r\n" );
   }

   if((RequestType&0x1)==1){
       //
       //either a rw request or read request finished. Print the read buffer.
       //



      for(Index = 0; Index < CommandParam->ReadLength; Index ++){

    //Print(L"0x%x ",  *(BufferDump + Index));
        if(Index == Offset)
    {
      *(BufferForReadData) = *(BufferDump + Index);
      *(BufferForReadData + 1) =  *(BufferDump + Index + 1);
    }
    //Print(L"\nReturned -> 0x%x & 0x%x\n", *(BufferForReadData), *(BufferForReadData + 1));
      }
   }
      
_end:    
   if(CommandParam != NULL){
       if(CommandParam->WriteBuffer != NULL){
           FreePool(CommandParam->WriteBuffer);
       }

       FreePool(CommandParam);
    }

    return Status;
}

/*Write 0x1 to offset 0x3A*/

EFI_STATUS 
SendCommandToUlpmcForChargerRead (UINT8 BusNo, UINT8 Slaveaddress, UINT8 Offset, UINTN ReadBytes, UINT8 *BufferForReadData)
{
    EFI_STATUS Status = EFI_SUCCESS;
    PI2CCommandParam CommandParam = NULL;
    UINTN RequestType = 0;   //0. No request; 1. read request; 2. write request. 3. rw request.

    UINTN Index = 0;
    EFI_HANDLE *HandleArray = NULL;
    UINTN HandleArrayCount =0;
    EFI_I2C_BUS_PROTOCOL *I2cBusProtocol = NULL;
    EFI_I2C_STACK_REQUEST_PACKET Request;
    UINT8 *Wbuf = NULL;
    CHAR8 AcpiID[I2C_ACPI_ID_LEN+1];
    UINT8 *BufferDump = NULL;

    BufferDump = AllocateZeroPool(90);

    Wbuf = AllocateZeroPool(2);
  
    Wbuf[0] = 0x3A;
    Wbuf[1] = 0x1;

    CommandParam = AllocateZeroPool(sizeof(I2CCommandParam));

    if(CommandParam == NULL){
      Print(L"\nFailed to allocate resource for parsing command.\n");
      Status = EFI_ABORTED;
      goto _end;
    }
    /*
      if(!CheckParams(Argc, Argv, CommandParam)){
          goto _end;
      }
    */
    //Override
    CommandParam->ControllerID = BusNo;
    CommandParam->SlaveAddress = Slaveaddress;
    CommandParam->ReadLength = 0;
    CommandParam->WriteLength = 2;

    //Print(L"Received command: Controller ID: %d, SlaveAddress: %d, ReadLength: %d, WriteLength: %d.\n", CommandParam->ControllerID, 
    // CommandParam->SlaveAddress, CommandParam->ReadLength, CommandParam->WriteLength);

    if(CommandParam->ControllerID>6){
        Print(L"\nInvalid controller ID. aborting...\n");
        Status = EFI_ABORTED;
        goto _end;
    }

    //
    //For TalkI2C utility, the ACPI path is always in the format of I2C0X\\SFFFF before sending request to driver.
    //where X is 0-6
    //

    AsciiStrCpy(AcpiID, DID_ACPI_ID_PREFIX); 
    AcpiID[4] = '0'+CommandParam->ControllerID;
    AsciiStrCpy(AcpiID+5, DID_ACPI_ID_SUFFIX);
    if(FastModeSpecified == TRUE){
      AsciiStrCpy(AcpiID+11, DID_ACPI_ID_SUFFIX_400K);
    }


    //
    //Proceed to send command to I2C driver. For the tool, we always use synchronized mode.
    //

    
    Status = gBS->LocateHandleBuffer( ByProtocol, 
                                      &gEfiI2cBusProtocolGuid,
                                      NULL,
                                      &HandleArrayCount,
                                      &HandleArray);
    if(EFI_ERROR(Status)){
        Print(L"\nFailed to locate i2c bus protocol.\n");
        goto _end;
    }

    for ( Index = 0; HandleArrayCount > Index; Index ++ ) {
      //
      //  Determine if the device is available
      //
      if ( NULL != DlAcpiFindDeviceWithMatchingCid ( HandleArray [ Index ],
                      0,
                      (CONST CHAR8 *)AcpiID
                    )) {
        //
        //  The device was found
        //
        Status = gBS->OpenProtocol ( HandleArray [ Index ],
                                     &gEfiI2cBusProtocolGuid,
                                     (VOID **)&I2cBusProtocol,
                                     NULL,
                                     NULL,
                                     EFI_OPEN_PROTOCOL_GET_PROTOCOL );
        break;
      }
    }
    //
    //  Done with the handle array
    //
   gBS->FreePool ( HandleArray );
  

  if(NULL == I2cBusProtocol){
      Print(L"\nError -- I2C device not found.\n");
      goto _end;
  }
  else{
      RequestType += CommandParam->ReadLength>0?1:0;
      RequestType += CommandParam->WriteLength>0?2:0; //Should not be a problem
  }



  //Print(L"Sending Request.....\r\n");
  Request.ReadBytes   = CommandParam->ReadLength;
  Request.ReadBuffer  = Request.ReadBytes>0?BufferDump:NULL;
  Request.WriteBytes  = CommandParam->WriteLength;
  Request.WriteBuffer = Request.WriteBytes>0?Wbuf:NULL;
  Request.Timeout = (UINT32)RequestTimeout;
    //Print(L"Received command: Controller ID: %d, SlaveAddress: %d, ReadLength: %d, WriteLength: %d.\n", CommandParam->ControllerID, 
      //  CommandParam->SlaveAddress, CommandParam->ReadLength, CommandParam->WriteLength);
  Status = I2cBusProtocol->StartRequest ( I2cBusProtocol,
                                                    (CommandParam->SlaveAddress)|0x400,
                                                    NULL,
                                                    &Request,
                                                    NULL );
  if ( EFI_ERROR ( Status )) {
      if ( EFI_DEVICE_ERROR == Status ) {
        Print ( L"\nERROR - This platform does not support this device!\r\n" );
        goto _end;
      } else {
        Print ( L"\nERROR - I2C synchronous request error, Status: %r\r\n", Status );
        goto _end;
      }
   } 
   else {
   //  Print ( L"INFO - Synchronous I2C request completed successfully!\r\n" );
   }

   if((RequestType&0x1)==1){
       //
       //either a rw request or read request finished. Print the read buffer.
       //

      for(Index = 0; Index < CommandParam->ReadLength; Index ++){
     
      if(Index % 0xF == 0)
      {
      //  Print(L"\n");
      }
    
      //Print(L"0x%x ",  *(BufferDump + Index));
        if(Index == Offset)
    {
      *(BufferForReadData) = *(BufferDump + Index);
      *(BufferForReadData + 1) =  *(BufferDump + Index + 1);
    }
    //Print(L"\nReturned -> 0x%x & 0x%x\n", *(BufferForReadData), *(BufferForReadData + 1));
      }
   }
      
_end:    
   if(CommandParam != NULL){
       if(CommandParam->WriteBuffer != NULL){
           FreePool(CommandParam->WriteBuffer);
       }

       FreePool(CommandParam);
    }

   SystemTable->BootServices->Stall(2000000);

    return Status;
}
