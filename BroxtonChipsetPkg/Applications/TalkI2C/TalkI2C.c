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

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DriverLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Protocol/I2cBus.h>

#include "TalkI2C.h"
#include "Utilities.h"

BOOLEAN IsListCommand = FALSE;
BOOLEAN FastModeSpecified = FALSE;
UINT8 ReadBuffer [I2C_READ_MAX];
UINT32 RequestTimeout = I2C_TIMEOUT_DEFAULT;

VOID PrintUsage(){
    Print(L"TalkI2C ver. 0.1\n");
    Print(L"Copyright (c) 2012, Intel Corporation. All rights reserved.\n");
    Print(L"This application interfaces with I2C device for write-read operation. ");
    Print(L"Write executes first-followed by the read. Read and write are both optional. ");
    Print(L"All numbers are decimal .\n");
    Print(L"\n");
    Print(L"Usage: ");
    Print(L"TalkI2C /c <controller> [options]\n");
    Print(L"Options:\n");
    Print(L"/c <0...6 Controller>                     Specify decimal id of the controller.\n");
    Print(L"/d <0...1022 SlaveAddress>                Specify decimal address of the target device.\n");
    Print(L"/w <length> {data1,data2,data3...dataN}   Write operation.\n");
    Print(L"                                            length limited to shell input. Data in decimal, \n");
    Print(L"                                            seperated by \",\",grouped by \"{\" and \"}\" .\n");
    Print(L"/r <length>                               Read operation.\n");  
    Print(L"                                            length range from 0 to 1024 restricted by tool.\n");
    Print(L"/l                                        List devices that respond to a read request.\n");
    Print(L"/t  <timeout>                             Specify timeout for the request in milli-seconds.\n");
    Print(L"/f                                        fast mode at 400KHz. w/o /f, runs at 100KHz.\n");
    Print(L"/h                                        Print this help message.\n");
    Print(L"Example:\n");
    Print(L"Controller 6, device 110, write 16 & 32, and read 32 bytes\n");
    Print(L"      >TalkI2C /c 6 /d 110 /w 2 {16,32} /r 32\n");
    Print(L"Controller 3, list all devices.\n");
    Print(L"      >TalkI2C /c 3 /l\n");

    return;
}



/**
Command line parser
@param  Argc  Arg number
@param  Argv  Arg array
@return FALSE if command line parse fails
        TRUE if command line parse succeeds
*/


BOOLEAN
CheckParams(
  IN UINTN Argc, 
  IN CHAR16 **Argv,
  OUT PI2CCommandParam pPI2CCommandParam
  )
{
    BOOLEAN ParseResult = FALSE;

    BOOLEAN ControllerSpecified = FALSE;
    BOOLEAN AddressSpecified = FALSE;
    UINTN   ReadLength = 0;


#ifdef PRINT_INPUT
    UINT8 WriteBufferLen = 0;
#endif

    UINTN i=0;  //Arg index
    
    if(pPI2CCommandParam == NULL || Argc<2){
        PrintUsage();
        ParseResult = FALSE;
        goto _ExitParamCheck;
    }

    ZeroMem(pPI2CCommandParam, sizeof(pPI2CCommandParam));
    

    for(i=1; i<Argc;i++){

        if(StrCmp(Argv[i], L"/d") == 0){
            if(i+1>Argc-1){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            ParseResult = StringToUint16(Argv[i+1], &(pPI2CCommandParam->SlaveAddress));
            if(!ParseResult){
                Print(L"Slave address parsing error.\n");
                goto _ExitParamCheck;
            }
            AddressSpecified = TRUE;
            i++;
        }
        else if(StrCmp(Argv[i], L"/c") == 0){
            if(i+1>Argc-1){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            ParseResult = StringToUint8(Argv[i+1], &(pPI2CCommandParam->ControllerID));
            if(!ParseResult){
                Print(L"Controller ID parsing error.\n");
                goto _ExitParamCheck;
            }
            ControllerSpecified = TRUE;
            i++;
        }
        else if(StrCmp(Argv[i], L"/w") == 0){
            if(i+2>Argc-1){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            ParseResult = StringToUint8(Argv[i+1], &(pPI2CCommandParam->WriteLength));
            if(!ParseResult){
                goto _ExitParamCheck;
            }
            pPI2CCommandParam->WriteBuffer = AllocateZeroPool(sizeof(UINT8)*(pPI2CCommandParam->WriteLength));
            if(pPI2CCommandParam->WriteBuffer == NULL){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }

            ParseResult = GetWriteArray(Argv[i+2], &(pPI2CCommandParam->WriteLength), pPI2CCommandParam->WriteBuffer);
            if(!ParseResult){
                goto _ExitParamCheck;
            }
            
            if(pPI2CCommandParam->WriteLength>0){
#ifdef PRINT_INPUT
                Print(L"In ParseCommand: Write buffer:{");
                for(WriteBufferLen=0; WriteBufferLen<pPI2CCommandParam->WriteLength; WriteBufferLen++){
                    if(WriteBufferLen<pPI2CCommandParam->WriteLength-1){
                        Print(L"%d,", pPI2CCommandParam->WriteBuffer[WriteBufferLen]);
                    }
                    else{
                        Print(L"%d}\n",pPI2CCommandParam->WriteBuffer[WriteBufferLen]);
                    }
                }
#endif
            }
            i+=2;
        }
        else if(StrCmp(Argv[i], L"/r") == 0){
            if(i+1>Argc-1){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            ReadLength = (UINTN)pPI2CCommandParam->ReadLength;
            ParseResult = StringToUint32(Argv[i+1], &ReadLength);
            pPI2CCommandParam->ReadLength = (UINT32)ReadLength;
            if(!ParseResult){
                Print(L"Read length parsing error.\n");
                goto _ExitParamCheck;
            }
            i++;
        }
        else if(StrCmp(Argv[i], L"/l") == 0){
            IsListCommand = TRUE;
            ParseResult = TRUE;
        }
        else if(StrCmp(Argv[i], L"/f") == 0){
            FastModeSpecified = TRUE;
            ParseResult = TRUE;
        }
        else if(StrCmp(Argv[i], L"/m") == 0){
            //
            //Current I2C driver does not support changing mode dynamically. For forwared comptablitiy, put param check here.
            //
            if(i+1>Argc-1){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            ParseResult = StringToUint8(Argv[i+1], &(pPI2CCommandParam->BusMode));
            if(!ParseResult){
                goto _ExitParamCheck;
            }
            if(pPI2CCommandParam->BusMode > 2){
                //
                //Invalid bus mode
                //
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            i+=1;
        }
        else if(StrCmp(Argv[i], L"/h") ==0){
            //
            //Currently restart is by default enabled, and I2C driver does not support dynamically change the setting.
            //Ignore this param at present.
            //
            PrintUsage();
            ParseResult = FALSE;
            goto _ExitParamCheck;
        }
        else if(StrCmp(Argv[i], L"/s") ==0){
            //
            //Currently restart is by default enabled, and I2C driver does not support dynamically change the setting.
            //Ignore this param at present.
            //
            ParseResult = TRUE;
        }        
        else if(StrCmp(Argv[i], L"/t") ==0){
            //
            //Timeout is by default 1000ms. use /t option to override to any value in ms.
            //
            if(i+1>Argc-1){
                ParseResult = FALSE;
                goto _ExitParamCheck;
            }
            ParseResult = StringToUint32(Argv[i+1], (UINTN*)&RequestTimeout);
            if(!ParseResult){
                goto _ExitParamCheck;
            }
            i++;
            
        }

        else{
            //
            //For all other cases, fail the parse
            //
            ParseResult = FALSE;
            Print(L"Error parsing command line. Run TalkI2C /h for more help");
            goto _ExitParamCheck;
        }
    
    }

    //
    //Checks the combination of options to make sure it's a valid command
    //
    if(IsListCommand){
        if(!ControllerSpecified){
            Print(L"Controller ID not specified for list command. Run TalkI2C /h for more help.\n");
            ParseResult = FALSE;
            goto _ExitParamCheck;
        }
        ParseResult = TRUE;
        goto _ExitParamCheck;
    }
    if(!ControllerSpecified && ( !AddressSpecified || ( (pPI2CCommandParam->SlaveAddress&0x1) ==1) ) ){
        Print(L"Controller ID or slave address not specified/wrong. Run TalkI2C /h for more help.\n");
        ParseResult = FALSE;
        goto _ExitParamCheck;
    }
    if(pPI2CCommandParam->WriteLength == 0 && pPI2CCommandParam->ReadLength == 0){
        Print(L"Invalid read or write length.\n");
        ParseResult = FALSE;
        goto _ExitParamCheck;
    }

_ExitParamCheck:
    if(ParseResult != TRUE){
      if(pPI2CCommandParam != NULL){
        if(pPI2CCommandParam->WriteBuffer != NULL){
            FreePool(pPI2CCommandParam->WriteBuffer);
            pPI2CCommandParam->WriteBuffer = NULL;
        }
      }
    }

    return ParseResult;
}





/**
  I2C shell command

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/


INTN
EFIAPI 
ShellAppMain (
  IN UINTN Argc, 
  IN CHAR16 **Argv
  )
{
  
    EFI_STATUS Status = EFI_SUCCESS;
    PI2CCommandParam CommandParam = NULL;
    UINTN RequestType = 0;   //0. No request; 1. read request; 2. write request. 3. rw request.

    UINTN Index = 0;
    EFI_HANDLE *HandleArray = NULL;
    UINTN HandleArrayCount =0;
    EFI_I2C_BUS_PROTOCOL *I2cBusProtocol = NULL;
    EFI_I2C_STACK_REQUEST_PACKET Request;

    CHAR8 AcpiID[I2C_ACPI_ID_LEN+1];

    CommandParam = AllocateZeroPool(sizeof(I2CCommandParam));
    if(CommandParam == NULL){
        Print(L"Failed to allocate resource for parsing command.\n");
        Status = EFI_ABORTED;
        goto _end;
    }

    if(!CheckParams(Argc, Argv, CommandParam)){
        goto _end;
    }

    Print(L"Received command: Controller ID: %d, SlaveAddress: %d, ReadLength: %d, WriteLength: %d.\n", CommandParam->ControllerID, 
        CommandParam->SlaveAddress, CommandParam->ReadLength, CommandParam->WriteLength);

    if(CommandParam->ControllerID>6){
        Print(L"Invalid controller ID. aborting...\n");
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
        Print(L"Failed to locate i2c bus protocol.\n");
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
      Print(L"Error -- I2C device not found.\n");
      goto _end;
  }
  else{
      RequestType += CommandParam->ReadLength>0?1:0;
      RequestType += CommandParam->WriteLength>0?2:0;
  }
  //
  //In case a list command is issued, run thru 0-255 addresses and wait for a response
  //
  if(IsListCommand){
      Print(L"Scanning devices on Controller %d (0-based)...\n", CommandParam->ControllerID);

      Request.ReadBytes = 1;
      Request.ReadBuffer = &ReadBuffer[0];
      Request.WriteBytes = 0;
      Request.WriteBuffer = NULL;
      Request.Timeout = (UINT32)RequestTimeout;

      
      for(Index=0; Index<I2C_ADDRESS_MAX; Index+=2){
        
        Status = I2cBusProtocol->StartRequest ( I2cBusProtocol,
                                                    Index|0x400,
                                                    NULL,
                                                    &Request,
                                                    NULL );
      
        if(EFI_ERROR(Status)){
            Print(L".");
        }
        else{
            Print(L"\nSlave detected. Address: %d\n", Index);
        }
      }
      Print(L"\nScan finished.\n");
      goto _end;
  }


  Print(L"Sending Request.....\r\n");
  Request.ReadBytes   = (UINT32)CommandParam->ReadLength;
  Request.ReadBuffer  = Request.ReadBytes>0?&ReadBuffer[0]:NULL;
  Request.WriteBytes  = CommandParam->WriteLength;
  Request.WriteBuffer = Request.WriteBytes>0?CommandParam->WriteBuffer:NULL;
  Request.Timeout = (UINT32)RequestTimeout;

  Status = I2cBusProtocol->StartRequest ( I2cBusProtocol,
                                                    (CommandParam->SlaveAddress)|0x400,
                                                    NULL,
                                                    &Request,
                                                    NULL );
  if ( EFI_ERROR ( Status )) {
      if ( EFI_DEVICE_ERROR == Status ) {
        Print ( L"ERROR - This platform does not support this device!\r\n" );
        goto _end;
      } else {
        Print ( L"ERROR - I2C synchronous request error, Status: %r\r\n", Status );
        goto _end;
      }
   } 
   else {
     Print ( L"INFO - Synchronous I2C request completed successfully!\r\n" );
   }

   if((RequestType&0x1)==1){
       //
       //either a rw request or read request finished. Print the read buffer.
       //
      for(Index = 0; Index < CommandParam->ReadLength; Index ++){
        if(0 == Index % 16) Print(L"\r\n0x%04x: ",Index);
        Print(L"%02x ",ReadBuffer[Index]);    
      }
   }
      
_end:    
   if(CommandParam != NULL){
       if(CommandParam->WriteBuffer != NULL){
           FreePool(CommandParam->WriteBuffer);
       }

       FreePool(CommandParam);
    }

    return (UINTN)Status;
}
