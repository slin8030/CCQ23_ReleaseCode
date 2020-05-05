/** @file
  SIO Library.

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

#include <Library/SioLib.h>

/**

  Pass in informatioin to find LDN of SIO device.  

  @param[in]       *DeviceProtocolTable  DeviceProtocolFunction table pointer.
  @param[in]       Devicee               What kind of the device.
  @param[in]       DeviceInstance        SIO instance.
  @param[out]      LDN                   LDN of SIO device.
**/
VOID
FindDeviceLdn (
  IN  EFI_INSTALL_DEVICE_FUNCTION *DeviceProtocolTable,
  IN  UINT8                       Device,
  IN  UINT8                       DeviceInstance,
  OUT UINT8                       *Ldn
  )
{  
  if (DeviceProtocolTable != NULL) {
    while (DeviceProtocolTable->Device != NULL_ID) {
      if ((DeviceProtocolTable->Device == Device) && (DeviceProtocolTable->Instance == DeviceInstance)) {
        *Ldn = DeviceProtocolTable->Ldn;
        return;
      }
      DeviceProtocolTable++;
    }
  }
  return;
}

/**
  Write data to the specific register.  

  @param[in]  Index    SIO index port.
  @param[in]  Data     The 8 bits value write to index port.
**/
VOID
IDW8 (
  IN UINT8                      Index,
  IN UINT8                      Data,
  IN EFI_SIO_RESOURCE_FUNCTION  *SioResourceFunction
  )
{
  SioWrite8 (
    SioResourceFunction->SioCfgPort,
    Index,
    SioResourceFunction->ScriptFlag
    );
  SioWrite8 (
    SioResourceFunction->SioCfgPort + 1,
    Data,
    SioResourceFunction->ScriptFlag
    );
}

/**
  Read data to the specific register.  

  @param[in]  Index    SIO index port.

  @retval     UINT8    Read value from SIO. 
**/
UINT8
IDR8 (
  IN UINT8                      Index,
  IN EFI_SIO_RESOURCE_FUNCTION  *SioResourceFunction
  )  
{  
  SioWrite8 (
    SioResourceFunction->SioCfgPort, 
    Index,
    FALSE
    );

  return SioRead8(SioResourceFunction->SioCfgPort + 1);
}

/**
  Writes an I/O port.  

  @param[in]  Port     I/O Port to write.
  @param[in]  Value    The 8 bits value write to port.
**/
VOID
SioWrite8 (
  IN UINT16   Port,
  IN UINT8    Value,
  IN BOOLEAN  ScriptFlag
  )
{
  IoWrite8 ((UINTN)Port, Value);
  
  if (ScriptFlag == TRUE) {
    S3BootScriptSaveIoWrite (
      EfiBootScriptWidthUint8,
      Port,
      1,      
      &Value    
      );  
  }  
}

/**
  Writes an I/O port.   

  @param[in]  Port     I/O Port to write.

  @retval     Value    Get data from SIO.
**/
UINT8
SioRead8 (
  IN UINT16  Port
  )
{
  return IoRead8 ((UINTN)Port);
}

/**

  Pass in informatioin to find extensive resource from PCD data.

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       TypeInstance          SIO instance.
  @param[in]       Device                What kind of the device.
  @param[in]       DeviceInstance        Device instance.
**/
VOID
FindExtensiveDevice (
  UINT8                    **PcdExtensiveCfgPointer,
  UINT8                    TypeInstance,
  UINT8                    Device,
  UINT8                    DeviceInstance
  ) 
{
  BOOLEAN                       Found;
  SIO_IDENTIFY_DEVICE           *Table; 

  Found = FALSE;
  
  if (*PcdExtensiveCfgPointer != NULL) {
    while (*(UINT32*)(*PcdExtensiveCfgPointer) != EXTENSIVE_TABLE_ENDING) {
      if (*(UINT8*)(*PcdExtensiveCfgPointer) == DEVICE_EXTENSIVE_RESOURCE) {   
        Table = (SIO_IDENTIFY_DEVICE*)*PcdExtensiveCfgPointer;
        if ((Device == Table->Device) && (DeviceInstance == Table->DeviceInstance)) {
          Found = TRUE;        
          break;                  
        }  
      }        
      *PcdExtensiveCfgPointer += *(UINT8*)(*PcdExtensiveCfgPointer + 1);    
    }
  }    
  if (!Found) {  
    *PcdExtensiveCfgPointer = NULL;
  }    
}

/**
  Pass in informatioin to find device resource from PCD data.  

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       TypeInstance          SIO instance.
  @param[in]       Device                What kind of the device.
  @param[in]       DeviceInstance        Device instance.

  @retval          PcdPointer            If found device, it'll return pointer or else it return NULL. 
**/
SIO_DEVICE_LIST_TABLE*
FindSioDevice (
  SIO_DEVICE_LIST_TABLE  *PcdPointer,
  UINT8                  TypeInstance,
  UINT8                  Device,
  UINT8                  DeviceInstance
  ) 
{
  if (PcdPointer != NULL) {
    //
    // Calculate the number of non-zero entries in the table
    //
    while (!((PcdPointer->TypeH == NONE_ID) && (PcdPointer->TypeL == NONE_ID))) {
      if ((PcdPointer->TypeInstance == TypeInstance) && (PcdPointer->Device == Device) &&
        (PcdPointer->DeviceInstance == DeviceInstance)) {
        return PcdPointer;
      }
      PcdPointer++;
    }
  }
  PcdPointer = NULL;
  return PcdPointer;
}

/**
  Word to chage place for high byte, low byte
   
  @param[in] Value    The number need to chage.

  @retval    Value    Function complete successfully. 
**/
UINT16
UpsideDown (
  IN UINT16  Value
  )
{
  UINT16     High;
  UINT8      Low;

  High = Value & 0xFF;
  Low = Value >> 8;

  Value = (High << 8) + Low;

  return Value;
}

/**
  Convert a single number to character.
  It assumes the input Char is in the scope of L'0' ~ L'9' and L'A' ~ L'F'.
  
  @param[in] Num    The input char which need to change to a hex number.
**/
UINT16
UintToChar (
  IN UINT8  Num
  )
{
  UINT8     Index;
  UINT16    Value;
  UINT8     TempValue;

  Value = 0;

  for (Index = 0; Index < (sizeof(UINT16)/sizeof(UINT8)); Index++) {
    TempValue = Num & 0xF;
    if ((TempValue >= 0) && (TempValue <= 9)) {
      Value = (Value << 8) + ((UINT16) (TempValue + L'0'));
    }

    if ((TempValue >= 0xA) && (TempValue <= 0xF)) {
      Value = (Value << 8) + ((UINT16) (TempValue + L'A' - 0xA ));
    }
    Num = Num >> 4;
  }
  
  return Value;
}

/**
  Decode character in ASCII to hexadecimal

  @param[in] Asc           Character in ASCII

  @retval Value in hexadecimal.
**/
UINT8
AsciiToHexadecimal (
  IN UINT8  Asc
  )
{
  if (Asc >= '0' && Asc <= '9') {
    return Asc - '0';
  } else if (Asc >= 'A' && Asc <= 'F') {
    return Asc - 'A' + 0xA;
  } else if (Asc >= 'a' && Asc <= 'f') {
    return Asc - 'a' + 0xA;
  }

  return 0;
}

/**
  
  Pass in informatioin to find hardware monitor resource from PCD data.  

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       Type                  Hwm type.
**/
VOID
FindExtensiveDeviceType (
  IN OUT UINT8          **PcdExtensiveCfgPointer,
  IN     UINT8          Type
  ) 
{
  BOOLEAN               Found;

  Found = FALSE;
  
  if (*PcdExtensiveCfgPointer != NULL) {
    do {
      *PcdExtensiveCfgPointer += *(UINT8*)(*PcdExtensiveCfgPointer + 1);
      if (*(UINT8*)(*PcdExtensiveCfgPointer) == Type) {
        Found = TRUE;
        break;
      }
    } while (*(UINT8*)(*PcdExtensiveCfgPointer) != DEVICE_EXTENSIVE_RESOURCE &&
             *(UINT32*)(*PcdExtensiveCfgPointer) != EXTENSIVE_TABLE_ENDING);
  }
  if (!Found) {  
    *PcdExtensiveCfgPointer = NULL;
  }
}

/**
  
  Pass in informatioin to find fan configuration from PCD data.  

  @param[in, out]  *TableList            PCD data pointer.
  @param[in]       Type                  Fan type.
**/
UINT8*
FindSmartFanType (
  IN OUT UINT8          *PcdExtensiveCfgPointer,
  IN     UINT8          Type
  ) 
{
  BOOLEAN               Found;

  Found = FALSE;
  
  if (PcdExtensiveCfgPointer != NULL) {
    do {
      PcdExtensiveCfgPointer += *(PcdExtensiveCfgPointer + 1);
      if (*PcdExtensiveCfgPointer == Type) {
        Found = TRUE;
        break;
      }
    } while (*PcdExtensiveCfgPointer != DEVICE_EXTENSIVE_RESOURCE &&
             *(UINT32*)PcdExtensiveCfgPointer != EXTENSIVE_TABLE_ENDING &&
             *PcdExtensiveCfgPointer != FAN_CONFIG_TYPE);
  }
  if (!Found) {  
    PcdExtensiveCfgPointer = NULL;
  }

  return PcdExtensiveCfgPointer;
}

/**
  I/O work flow to wait input buffer empty in given time.

  @param Timeout Wating time.

  @retval EFI_SIO_KBC_TIMEOUT if input is still not empty in given time.
  @retval EFI_SUCCESS input is empty.
**/
EFI_STATUS
WaitInputEmpty (
  IN UINTN                                Timeout
  )
{
  UINTN Delay;
  UINT8 Data;

  Delay = Timeout / 50;

  do {
    Data = IoRead8 (SIO_KBC_STATUS_PORT);

    //
    // Check keyboard controller status bit 1(input buffer status)
    //
    if ((Data & B_SIO_KBC_IBF_FULL) == V_SIO_KBC_IBF_EMPTY) {
      break;
    }

    MicroSecondDelay (50);
    Delay--;
  } while (Delay != 0);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  I/O work flow to wait output buffer empty in given time.

  @param Timeout given time

  @retval EFI_SIO_KBC_TIMEOUT  output is not empty in given time
  @retval EFI_SUCCESS  output is empty in given time.
**/
EFI_STATUS
WaitOutputEmpty (
  IN UINTN                                Timeout
  )
{
  UINTN Delay;
  UINT8 Data;

  Delay = Timeout / 50;

  do {
    Data = IoRead8 (SIO_KBC_STATUS_PORT);

    //
    // Check keyboard controller status bit 1(input buffer status)
    //
    if ((Data & B_SIO_KBC_OBF_FULL) == V_SIO_KBC_OBF_EMPTY) {
      break;
    }

    Data = IoRead8 (SIO_KBC_DATA_PORT);

    MicroSecondDelay (50);
    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}


/**
  I/O work flow of outing Kbc command.

  @param Command I/O command.

  @retval EFI_SUCCESS Success to excute I/O work flow
  @retval EFI_SIO_KBC_TIMEOUT Keyboard controller time out.
**/
EFI_STATUS
OutKbcCommand (
  IN UINT8                                Command
  )
{
  EFI_STATUS  Status;
  UINT8       Data;

  //
  // Wait keyboard controller input buffer empty
  //
  Status = WaitInputEmpty (SIO_KBC_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Send command
  //
  Data = Command;
  IoWrite8 (SIO_KBC_STATUS_PORT, Data);

  Status = WaitInputEmpty (SIO_KBC_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  I/O work flow of outing Kbc data.

  @param Data    Data value

  @retval EFI_SUCCESS Success to excute I/O work flow
  @retval EFI_SIO_KBC_TIMEOUT Keyboard controller time out.
**/
EFI_STATUS
OutKbcData (
  IN UINT8                                Data
  )
{
  EFI_STATUS  Status;
  UINT8       Temp;
  //
  // Wait keyboard controller input buffer empty
  //
  Status = WaitInputEmpty (SIO_KBC_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Temp = Data;
  IoWrite8 (SIO_KBC_DATA_PORT, Temp);

  Status = WaitInputEmpty (SIO_KBC_TIMEOUT);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  I/O work flow of in Kbc data.

  @retval EFI_SUCCESS Success to excute I/O work flow
  @retval EFI_SIO_KBC_TIMEOUT Keyboard controller time out.
**/
EFI_STATUS
InKbcData (
  )
{
  UINTN      Delay;
  UINT8      Temp;
  EFI_STATUS Status;
  UINT8      Data;

  Status = EFI_TIMEOUT;
  Delay  = SIO_KBC_TIMEOUT / 50;

  do {
    Temp  = IoRead8 (SIO_KBC_STATUS_PORT);
    Data  = IoRead8 (SIO_KBC_DATA_PORT);
    if (Data == 0x55) {
      Status = EFI_SUCCESS;
    }

    //
    // Check keyboard controller status bit 0(output buffer status)
    //
    if ((Temp & B_SIO_KBC_OBF_FULL) == V_SIO_KBC_OBF_EMPTY) {
      break;
    }

    MicroSecondDelay (50);
    Delay--;
  } while (Delay != 0);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return Status;
}

/**
  Issue self test command via IsaIo interface.

  @return EFI_SUCCESS  Success to do keyboard self testing.
  @return others       Fail to do keyboard self testing.
**/
EFI_STATUS
KbcSelfTest (
  )
{
  EFI_STATUS Status;
  UINT8      Data;

  //
  // Check Kbc is exist
  //
  Data = IoRead8 (SIO_KBC_STATUS_PORT);
  if (Data == 0xFF) {
    DEBUG ((EFI_D_INFO, "KBC isn't exist!\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Keyboard controller self test
  //
  Status = OutKbcCommand (SIO_KBC_SELF_TEST);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Read return code
  //
  Status = InKbcData ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

