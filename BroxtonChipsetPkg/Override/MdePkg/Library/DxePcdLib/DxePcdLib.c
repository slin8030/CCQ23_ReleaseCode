/** @file
Implementation of PcdLib class library for DXE phase.

Copyright (c) 2006 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


**/


#include <PiDxe.h>

#include <Protocol/Pcd.h>
#include <Protocol/PiPcd.h>
#include <Protocol/PcdInfo.h>
#include <Protocol/PiPcdInfo.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>

PCD_PROTOCOL                *mPcd       = NULL;
EFI_PCD_PROTOCOL            *mPiPcd     = NULL;
GET_PCD_INFO_PROTOCOL       *mPcdInfo   = NULL;
EFI_GET_PCD_INFO_PROTOCOL   *mPiPcdInfo = NULL;

/**
  Retrieves the PI PCD protocol from the handle database.

  @retval EFI_PCD_PROTOCOL * The pointer to the EFI_PCD_PROTOCOL.
**/
EFI_PCD_PROTOCOL *
EFIAPI
GetPiPcdProtocol (
  VOID
  )
{
  EFI_STATUS  Status;

  if (mPiPcd == NULL) {
    //
    // PI Pcd protocol defined in PI 1.2 vol3 should be installed before the module 
    // access DynamicEx type PCD.
    //
    Status = gBS->LocateProtocol (&gEfiPcdProtocolGuid, NULL, (VOID **) &mPiPcd);
    ASSERT_EFI_ERROR (Status);
    ASSERT (mPiPcd != NULL);
  }
  return mPiPcd;
}

/**
  Retrieves the PCD protocol from the handle database.

  @retval PCD_PROTOCOL * The pointer to the PCD_PROTOCOL.
**/
PCD_PROTOCOL *
EFIAPI
GetPcdProtocol (
  VOID
  )
{
  EFI_STATUS  Status;

  if (mPcd == NULL) {
    //
    // PCD protocol need to be installed before the module access Dynamic type PCD.
    // But dynamic type PCD is not required in PI 1.2 specification.
    // 
    Status = gBS->LocateProtocol (&gPcdProtocolGuid, NULL, (VOID **)&mPcd);
    ASSERT_EFI_ERROR (Status);
    ASSERT (mPcd != NULL);
  }
  return mPcd;
}

/**
  Retrieves the PI PCD info protocol from the handle database.

  @retval EFI_GET_PCD_INFO_PROTOCOL * The pointer to the EFI_GET_PCD_INFO_PROTOCOL defined in PI 1.2.1 Vol 3.
**/
EFI_GET_PCD_INFO_PROTOCOL *
GetPiPcdInfoProtocolPointer (
  VOID
  )
{
  EFI_STATUS  Status;

  if (mPiPcdInfo == NULL) {
    Status = gBS->LocateProtocol (&gEfiGetPcdInfoProtocolGuid, NULL, (VOID **)&mPiPcdInfo);
    ASSERT_EFI_ERROR (Status);
    ASSERT (mPiPcdInfo != NULL);
  }
  return mPiPcdInfo;
}

/**
  Retrieves the PCD info protocol from the handle database.

  @retval GET_PCD_INFO_PROTOCOL * The pointer to the GET_PCD_INFO_PROTOCOL.
**/
GET_PCD_INFO_PROTOCOL *
GetPcdInfoProtocolPointer (
  VOID
  ) 
{
  EFI_STATUS  Status;

  if (mPcdInfo == NULL) {
    Status = gBS->LocateProtocol (&gGetPcdInfoProtocolGuid, NULL, (VOID **)&mPcdInfo);
    ASSERT_EFI_ERROR (Status);
    ASSERT (mPcdInfo != NULL);
  }
  return mPcdInfo;
}

/**
  This function provides a means by which SKU support can be established in the PCD infrastructure.

  Sets the current SKU in the PCD database to the value specified by SkuId.  SkuId is returned.
  If SkuId >= PCD_MAX_SKU_ID, then ASSERT(). 

  @param  SkuId   The SKU value that will be used when the PCD service retrieves and sets values
                  associated with a PCD token.

  @return  Return the SKU ID that just be set.

**/
UINTN
EFIAPI
LibPcdSetSku (
  IN UINTN   SkuId
  )
{
  ASSERT (SkuId < PCD_MAX_SKU_ID);

  GetPcdProtocol()->SetSku (SkuId);

  return SkuId;
}



/**
  This function provides a means by which to retrieve a value for a given PCD token.
  
  Returns the 8-bit value for the token specified by TokenNumber. 

  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @return Returns the 8-bit value for the token specified by TokenNumber. 

**/
UINT8
EFIAPI
LibPcdGetEx8 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);
  
  return GetPiPcdProtocol()->Get8 (Guid, TokenNumber);
}


/**
  This function provides a means by which to retrieve a value for a given PCD token.

  Returns the 16-bit value for the token specified by TokenNumber and Guid.
  
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid         The pointer to a 128-bit unique value that designates 
                           which namespace to retrieve a value from.
  @param[in]  TokenNumber  The PCD token number to retrieve a current value for.

  @return Return the UINT16.

**/
UINT16
EFIAPI
LibPcdGetEx16 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->Get16 (Guid, TokenNumber);
}


/**
  Returns the 32-bit value for the token specified by TokenNumber and Guid.
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid         The pointer to a 128-bit unique value that designates 
                           which namespace to retrieve a value from.
  @param[in]  TokenNumber  The PCD token number to retrieve a current value for.

  @return Return the UINT32.

**/
UINT32
EFIAPI
LibPcdGetEx32 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->Get32 (Guid, TokenNumber);
}



/**
  This function provides a means by which to retrieve a value for a given PCD token.
  
  Returns the 64-bit value for the token specified by TokenNumber and Guid.
  
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid          The pointer to a 128-bit unique value that designates 
                            which namespace to retrieve a value from.
  @param[in]  TokenNumber   The PCD token number to retrieve a current value for.

  @return Return the UINT64.

**/
UINT64
EFIAPI
LibPcdGetEx64 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);
  
  return GetPiPcdProtocol()->Get64 (Guid, TokenNumber);
}



/**
  This function provides a means by which to retrieve a value for a given PCD token.
  
  Returns the pointer to the buffer of token specified by TokenNumber and Guid.
  
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid          The pointer to a 128-bit unique value that designates 
                            which namespace to retrieve a value from.
  @param[in]  TokenNumber   The PCD token number to retrieve a current value for.

  @return Return the VOID* pointer.

**/
VOID *
EFIAPI
LibPcdGetExPtr (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->GetPtr (Guid, TokenNumber);
}



/**
  This function provides a means by which to retrieve a value for a given PCD token.
  
  Returns the Boolean value of the token specified by TokenNumber and Guid. 
  
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid          The pointer to a 128-bit unique value that designates 
                            which namespace to retrieve a value from.
  @param[in]  TokenNumber   The PCD token number to retrieve a current value for.

  @return Return the BOOLEAN.

**/
BOOLEAN
EFIAPI
LibPcdGetExBool (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->GetBool (Guid, TokenNumber);
}



/**
  This function provides a means by which to retrieve the size of a given PCD token.
  
  Returns the size of the token specified by TokenNumber and Guid. 
  
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid          The pointer to a 128-bit unique value that designates 
                            which namespace to retrieve a value from.
  @param[in]  TokenNumber   The PCD token number to retrieve a current value for.

  @return Return the size.

**/
UINTN
EFIAPI
LibPcdGetExSize (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->GetSize (Guid, TokenNumber);
}



#ifndef DISABLE_NEW_DEPRECATED_INTERFACES




/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 8-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.

  If Guid is NULL, then ASSERT().

  @param[in]  Guid          The pointer to a 128-bit unique value that 
                            designates which namespace to set a value from.
  @param[in]  TokenNumber   The PCD token number to set a current value for.
  @param[in]  Value         The 8-bit value to set.

  @return Return the value that was set.

**/
UINT8
EFIAPI
LibPcdSetEx8 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber,
  IN UINT8             Value
  )
{
  ASSERT (Guid != NULL);

  GetPiPcdProtocol()->Set8 (Guid, TokenNumber, Value);

  return Value;
}



/**
  This function provides a means by which to set a value for a given PCD token.
  
  Sets the 16-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.

  If Guid is NULL, then ASSERT().

  @param[in]  Guid          The pointer to a 128-bit unique value that 
                            designates which namespace to set a value from.
  @param[in]  TokenNumber   The PCD token number to set a current value for.
  @param[in]  Value         The 16-bit value to set.

  @return Return the value that was set.

**/
UINT16
EFIAPI
LibPcdSetEx16 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber,
  IN UINT16            Value
  )
{
  ASSERT (Guid != NULL);

  GetPiPcdProtocol()->Set16 (Guid, TokenNumber, Value);

  return Value;
}



/**
  This function provides a means by which to set a value for a given PCD token.
  
  Sets the 32-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.

  If Guid is NULL, then ASSERT().

  @param[in]  Guid          The pointer to a 128-bit unique value that 
                            designates which namespace to set a value from.
  @param[in]  TokenNumber   The PCD token number to set a current value for.
  @param[in]  Value         The 32-bit value to set.

  @return Return the value that was set.

**/
UINT32
EFIAPI
LibPcdSetEx32 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber,
  IN UINT32            Value
  )
{
  ASSERT (Guid != NULL);

  GetPiPcdProtocol()->Set32 (Guid, TokenNumber, Value);

  return Value;
}



/**
  This function provides a means by which to set a value for a given PCD token.
  
  Sets the 64-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.

  If Guid is NULL, then ASSERT().

  @param[in]  Guid          The pointer to a 128-bit unique value that 
                            designates which namespace to set a value from.
  @param[in]  TokenNumber   The PCD token number to set a current value for.
  @param[in]  Value         The 64-bit value to set.

  @return Return the value that was set.

**/
UINT64
EFIAPI
LibPcdSetEx64 (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber,
  IN UINT64            Value
  )
{
  ASSERT (Guid != NULL);

  GetPiPcdProtocol()->Set64 (Guid, TokenNumber, Value);

  return Value;
}



/**
  This function provides a means by which to set a value for a given PCD token.
  
  Sets a buffer for the token specified by TokenNumber to the value specified by 
  Buffer and SizeOfBuffer.  Buffer is returned.  If SizeOfBuffer is greater than 
  the maximum size support by TokenNumber, then set SizeOfBuffer to the maximum size 
  supported by TokenNumber and return NULL to indicate that the set operation 
  was not actually performed.
  
  If Guid is NULL, then ASSERT().
  If SizeOfBuffer is NULL, then ASSERT().
  If SizeOfBuffer > 0 and Buffer is NULL, then ASSERT().
  
  @param[in]  Guid              The pointer to a 128-bit unique value that 
                                designates which namespace to set a value from.
  @param[in]  TokenNumber       The PCD token number to set a current value for.
  @param[in, out] SizeOfBuffer  The size, in bytes, of Buffer.
  @param[in]  Buffer            A pointer to the buffer to set.

  @return Return the pointer to the buffer been set.

**/
VOID *
EFIAPI
LibPcdSetExPtr (
  IN      CONST GUID        *Guid,
  IN      UINTN             TokenNumber,
  IN OUT  UINTN             *SizeOfBuffer,
  IN      VOID              *Buffer
  )
{
  EFI_STATUS  Status;
  UINTN       InputSizeOfBuffer;

  ASSERT (Guid != NULL);

  ASSERT (SizeOfBuffer != NULL);

  if (*SizeOfBuffer > 0) {
    ASSERT (Buffer != NULL);
  }

  InputSizeOfBuffer = *SizeOfBuffer;
  Status = GetPiPcdProtocol()->SetPtr (Guid, TokenNumber, SizeOfBuffer, Buffer);
  if (EFI_ERROR (Status) && (*SizeOfBuffer < InputSizeOfBuffer)) {
    return NULL;
  }

  return Buffer;
}



/**
  This function provides a means by which to set a value for a given PCD token.
  
  Sets the Boolean value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.

  If Guid is NULL, then ASSERT().

  @param[in]  Guid          The pointer to a 128-bit unique value that 
                            designates which namespace to set a value from.
  @param[in]  TokenNumber   The PCD token number to set a current value for.
  @param[in]  Value         The Boolean value to set.

  @return Return the value that was set.

**/
BOOLEAN
EFIAPI
LibPcdSetExBool (
  IN CONST GUID        *Guid,
  IN UINTN             TokenNumber,
  IN BOOLEAN           Value
  )
{
  ASSERT (Guid != NULL);

  GetPiPcdProtocol()->SetBool (Guid, TokenNumber, Value);

  return Value;
}
#endif

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 8-bit value for the token specified by TokenNumber
  to the value specified by Value.

  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 8-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSet8S (
  IN UINTN          TokenNumber,
  IN UINT8          Value
  )
{
  return GetPcdProtocol()->Set8 (TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 16-bit value for the token specified by TokenNumber
  to the value specified by Value.

  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 16-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSet16S (
  IN UINTN          TokenNumber,
  IN UINT16         Value
  )
{
  return GetPcdProtocol()->Set16 (TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 32-bit value for the token specified by TokenNumber
  to the value specified by Value.

  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 32-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSet32S (
  IN UINTN          TokenNumber,
  IN UINT32         Value
  )
{
  return GetPcdProtocol()->Set32 (TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 64-bit value for the token specified by TokenNumber
  to the value specified by Value.

  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 64-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSet64S (
  IN UINTN          TokenNumber,
  IN UINT64         Value
  )
{
  return GetPcdProtocol()->Set64 (TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets a buffer for the token specified by TokenNumber to the value specified
  by Buffer and SizeOfBuffer. If SizeOfBuffer is greater than the maximum size
  support by TokenNumber, then set SizeOfBuffer to the maximum size supported by
  TokenNumber and return EFI_INVALID_PARAMETER to indicate that the set operation
  was not actually performed.

  If SizeOfBuffer is set to MAX_ADDRESS, then SizeOfBuffer must be set to the
  maximum size supported by TokenName and EFI_INVALID_PARAMETER must be returned.

  If SizeOfBuffer is NULL, then ASSERT().
  If SizeOfBuffer > 0 and Buffer is NULL, then ASSERT().

  @param[in]      TokenNumber   The PCD token number to set a current value for.
  @param[in, out] SizeOfBuffer  The size, in bytes, of Buffer.
  @param[in]      Buffer        A pointer to the buffer to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetPtrS (
  IN       UINTN    TokenNumber,
  IN OUT   UINTN    *SizeOfBuffer,
  IN CONST VOID     *Buffer
  )
{
  ASSERT (SizeOfBuffer != NULL);

  if (*SizeOfBuffer > 0) {
    ASSERT (Buffer != NULL);
  }

  return GetPcdProtocol()->SetPtr (TokenNumber, SizeOfBuffer, (VOID *) Buffer);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the boolean value for the token specified by TokenNumber
  to the value specified by Value.

  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The boolean value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetBoolS (
  IN UINTN          TokenNumber,
  IN BOOLEAN        Value
  )
{
  return GetPcdProtocol()->SetBool (TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 8-bit value for the token specified by TokenNumber
  to the value specified by Value.

  If Guid is NULL, then ASSERT().

  @param[in] Guid           The pointer to a 128-bit unique value that
                            designates which namespace to set a value from.
  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 8-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetEx8S (
  IN CONST GUID     *Guid,
  IN UINTN          TokenNumber,
  IN UINT8          Value
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->Set8 (Guid, TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 16-bit value for the token specified by TokenNumber
  to the value specified by Value.

  If Guid is NULL, then ASSERT().

  @param[in] Guid           The pointer to a 128-bit unique value that
                            designates which namespace to set a value from.
  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 16-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetEx16S (
  IN CONST GUID     *Guid,
  IN UINTN          TokenNumber,
  IN UINT16         Value
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->Set16 (Guid, TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 32-bit value for the token specified by TokenNumber
  to the value specified by Value.

  If Guid is NULL, then ASSERT().

  @param[in] Guid           The pointer to a 128-bit unique value that
                            designates which namespace to set a value from.
  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 32-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetEx32S (
  IN CONST GUID     *Guid,
  IN UINTN          TokenNumber,
  IN UINT32         Value
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->Set32 (Guid, TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the 64-bit value for the token specified by TokenNumber
  to the value specified by Value.

  If Guid is NULL, then ASSERT().

  @param[in] Guid           The pointer to a 128-bit unique value that
                            designates which namespace to set a value from.
  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The 64-bit value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetEx64S (
  IN CONST GUID     *Guid,
  IN UINTN          TokenNumber,
  IN UINT64         Value
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->Set64 (Guid, TokenNumber, Value);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets a buffer for the token specified by TokenNumber to the value specified by
  Buffer and SizeOfBuffer. If SizeOfBuffer is greater than the maximum size
  support by TokenNumber, then set SizeOfBuffer to the maximum size supported by
  TokenNumber and return EFI_INVALID_PARAMETER to indicate that the set operation
  was not actually performed.

  If Guid is NULL, then ASSERT().
  If SizeOfBuffer is NULL, then ASSERT().
  If SizeOfBuffer > 0 and Buffer is NULL, then ASSERT().

  @param[in]      Guid          Pointer to a 128-bit unique value that
                                designates which namespace to set a value from.
  @param[in]      TokenNumber   The PCD token number to set a current value for.
  @param[in, out] SizeOfBuffer  The size, in bytes, of Buffer.
  @param[in]      Buffer        A pointer to the buffer to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetExPtrS (
  IN CONST GUID     *Guid,
  IN       UINTN    TokenNumber,
  IN OUT   UINTN    *SizeOfBuffer,
  IN       VOID     *Buffer
  )
{
  ASSERT (Guid != NULL);

  ASSERT (SizeOfBuffer != NULL);

  if (*SizeOfBuffer > 0) {
    ASSERT (Buffer != NULL);
  }

  return GetPiPcdProtocol()->SetPtr (Guid, TokenNumber, SizeOfBuffer, Buffer);
}

/**
  This function provides a means by which to set a value for a given PCD token.

  Sets the boolean value for the token specified by TokenNumber
  to the value specified by Value.

  If Guid is NULL, then ASSERT().

  @param[in] Guid           The pointer to a 128-bit unique value that
                            designates which namespace to set a value from.
  @param[in] TokenNumber    The PCD token number to set a current value for.
  @param[in] Value          The boolean value to set.

  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPcdSetExBoolS (
  IN CONST GUID     *Guid,
  IN UINTN          TokenNumber,
  IN BOOLEAN        Value
  )
{
  ASSERT (Guid != NULL);

  return GetPiPcdProtocol()->SetBool (Guid, TokenNumber, Value);
}

/**
  Set up a notification function that is called when a specified token is set.
  
  When the token specified by TokenNumber and Guid is set, 
  then notification function specified by NotificationFunction is called.  
  If Guid is NULL, then the default token space is used.
  If NotificationFunction is NULL, then ASSERT().

  @param[in]  Guid                  The pointer to a 128-bit unique value that designates which 
                                    namespace to set a value from.  If NULL, then the default 
                                    token space is used.
  @param[in]  TokenNumber           The PCD token number to monitor.
  @param[in]  NotificationFunction  The function to call when the token 
                                    specified by Guid and TokenNumber is set.

**/
VOID
EFIAPI
LibPcdExCallbackOnSet (
  IN CONST GUID               *Guid,       OPTIONAL
  IN UINTN                    TokenNumber,
  IN PCD_CALLBACK             NotificationFunction
  )
{
  EFI_STATUS Status;

  ASSERT (NotificationFunction != NULL);

  Status = GetPiPcdProtocol()->CallbackOnSet (Guid, TokenNumber, (EFI_PCD_PROTOCOL_CALLBACK) NotificationFunction);
  ASSERT_EFI_ERROR (Status);

  return;
}



/**
  Disable a notification function that was established with LibPcdCallbackonSet().
  
  Disable a notification function that was previously established with LibPcdCallbackOnSet().
  If NotificationFunction is NULL, then ASSERT().
  If LibPcdCallbackOnSet() was not previously called with Guid, TokenNumber, 
  and NotificationFunction, then ASSERT().
  
  @param[in]  Guid                 Specify the GUID token space.
  @param[in]  TokenNumber          Specify the token number.
  @param[in]  NotificationFunction The callback function to be unregistered.

**/
VOID
EFIAPI
LibPcdExCancelCallback (
  IN CONST GUID               *Guid,       OPTIONAL
  IN UINTN                    TokenNumber,
  IN PCD_CALLBACK             NotificationFunction
  )
{
  EFI_STATUS Status;

  ASSERT (NotificationFunction != NULL);
    
  Status = GetPiPcdProtocol()->CancelCallback (Guid, TokenNumber, (EFI_PCD_PROTOCOL_CALLBACK) NotificationFunction);
  ASSERT_EFI_ERROR (Status);

  return;
}



/**
  Retrieves the next token in a token space.
  
  Retrieves the next PCD token number from the token space specified by Guid.  
  If Guid is NULL, then the default token space is used.  If TokenNumber is 0, 
  then the first token number is returned.  Otherwise, the token number that 
  follows TokenNumber in the token space is returned.  If TokenNumber is the last 
  token number in the token space, then 0 is returned.  
  
  If TokenNumber is not 0 and is not in the token space specified by Guid, then ASSERT().

  @param[in]  Guid        The pointer to a 128-bit unique value that designates which namespace 
                          to set a value from.  If NULL, then the default token space is used.
  @param[in]  TokenNumber The previous PCD token number.  If 0, then retrieves the first PCD 
                          token number.

  @return The next valid token number.

**/
UINTN           
EFIAPI
LibPcdGetNextToken (
  IN CONST GUID               *Guid,       OPTIONAL
  IN UINTN                    TokenNumber
  )
{
  EFI_STATUS    Status;

  Status = GetPiPcdProtocol()->GetNextToken (Guid, &TokenNumber);
  ASSERT (!EFI_ERROR (Status) || TokenNumber == 0);

  return TokenNumber;
}



/**
  Used to retrieve the list of available PCD token space GUIDs.
  
  Returns the PCD token space GUID that follows TokenSpaceGuid in the list of token spaces
  in the platform.
  If TokenSpaceGuid is NULL, then a pointer to the first PCD token spaces returned.
  If TokenSpaceGuid is the last PCD token space GUID in the list, then NULL is returned.
  
  @param  TokenSpaceGuid  The pointer to the a PCD token space GUID.

  @return The next valid token namespace.

**/
GUID *
EFIAPI
LibPcdGetNextTokenSpace (
  IN CONST GUID  *TokenSpaceGuid
  )
{
  GetPiPcdProtocol()->GetNextTokenSpace (&TokenSpaceGuid);

  return (GUID *)TokenSpaceGuid;
}


/**
  Sets a value of a patchable PCD entry that is type pointer.
  
  Sets the PCD entry specified by PatchVariable to the value specified by Buffer 
  and SizeOfBuffer.  Buffer is returned.  If SizeOfBuffer is greater than 
  MaximumDatumSize, then set SizeOfBuffer to MaximumDatumSize and return 
  NULL to indicate that the set operation was not actually performed.  
  If SizeOfBuffer is set to MAX_ADDRESS, then SizeOfBuffer must be set to 
  MaximumDatumSize and NULL must be returned.
  
  If PatchVariable is NULL, then ASSERT().
  If SizeOfBuffer is NULL, then ASSERT().
  If SizeOfBuffer > 0 and Buffer is NULL, then ASSERT().

  @param[in] PatchVariable      A pointer to the global variable in a module that is 
                                the target of the set operation.
  @param[in] MaximumDatumSize   The maximum size allowed for the PCD entry specified by PatchVariable.
  @param[in, out] SizeOfBuffer  A pointer to the size, in bytes, of Buffer.
  @param[in] Buffer             A pointer to the buffer to used to set the target variable.
  
  @return Return the pointer to the buffer been set.

**/
VOID *
EFIAPI
LibPatchPcdSetPtr (
  IN        VOID        *PatchVariable,
  IN        UINTN       MaximumDatumSize,
  IN OUT    UINTN       *SizeOfBuffer,
  IN CONST  VOID        *Buffer
  )
{
  ASSERT (PatchVariable != NULL);
  ASSERT (SizeOfBuffer  != NULL);
  
  if (*SizeOfBuffer > 0) {
    ASSERT (Buffer != NULL);
  }

  if ((*SizeOfBuffer > MaximumDatumSize) ||
      (*SizeOfBuffer == MAX_ADDRESS)) {
    *SizeOfBuffer = MaximumDatumSize;
    return NULL;
  }
    
  CopyMem (PatchVariable, Buffer, *SizeOfBuffer);
  
  return (VOID *) Buffer;
}

/**
  Sets a value of a patchable PCD entry that is type pointer.

  Sets the PCD entry specified by PatchVariable to the value specified
  by Buffer and SizeOfBuffer. If SizeOfBuffer is greater than MaximumDatumSize,
  then set SizeOfBuffer to MaximumDatumSize and return RETURN_INVALID_PARAMETER
  to indicate that the set operation was not actually performed.
  If SizeOfBuffer is set to MAX_ADDRESS, then SizeOfBuffer must be set to
  MaximumDatumSize and RETURN_INVALID_PARAMETER must be returned.

  If PatchVariable is NULL, then ASSERT().
  If SizeOfBuffer is NULL, then ASSERT().
  If SizeOfBuffer > 0 and Buffer is NULL, then ASSERT().

  @param[in] PatchVariable      A pointer to the global variable in a module that is
                                the target of the set operation.
  @param[in] MaximumDatumSize   The maximum size allowed for the PCD entry specified by PatchVariable.
  @param[in, out] SizeOfBuffer  A pointer to the size, in bytes, of Buffer.
  @param[in] Buffer             A pointer to the buffer to used to set the target variable.
  
  @return The status of the set operation.

**/
RETURN_STATUS
EFIAPI
LibPatchPcdSetPtrS (
  IN       VOID     *PatchVariable,
  IN       UINTN    MaximumDatumSize,
  IN OUT   UINTN    *SizeOfBuffer,
  IN CONST VOID     *Buffer
  )
{
  ASSERT (PatchVariable != NULL);
  ASSERT (SizeOfBuffer  != NULL);
  
  if (*SizeOfBuffer > 0) {
    ASSERT (Buffer != NULL);
  }

  if ((*SizeOfBuffer > MaximumDatumSize) ||
      (*SizeOfBuffer == MAX_ADDRESS)) {
    *SizeOfBuffer = MaximumDatumSize;
    return RETURN_INVALID_PARAMETER;
  }

  CopyMem (PatchVariable, Buffer, *SizeOfBuffer);

  return RETURN_SUCCESS;
}

/**
  Retrieve additional information associated with a PCD token.

  This includes information such as the type of value the TokenNumber is associated with as well as possible
  human readable name that is associated with the token.

  If TokenNumber is not in the default token space specified, then ASSERT().

  @param[in]    TokenNumber The PCD token number.
  @param[out]   PcdInfo     The returned information associated with the requested TokenNumber.
                            The caller is responsible for freeing the buffer that is allocated by callee for PcdInfo->PcdName.
**/
VOID
EFIAPI
LibPcdGetInfo (
  IN        UINTN           TokenNumber,
  OUT       PCD_INFO        *PcdInfo
  )
{
  EFI_STATUS Status;

  Status = GetPcdInfoProtocolPointer()->GetInfo (TokenNumber, (EFI_PCD_INFO *) PcdInfo);
  ASSERT_EFI_ERROR (Status);
}

/**
  Retrieve additional information associated with a PCD token.

  This includes information such as the type of value the TokenNumber is associated with as well as possible
  human readable name that is associated with the token.

  If TokenNumber is not in the token space specified by Guid, then ASSERT().

  @param[in]    Guid        The 128-bit unique value that designates the namespace from which to extract the value.
  @param[in]    TokenNumber The PCD token number.
  @param[out]   PcdInfo     The returned information associated with the requested TokenNumber.
                            The caller is responsible for freeing the buffer that is allocated by callee for PcdInfo->PcdName.
**/
VOID
EFIAPI
LibPcdGetInfoEx (
  IN CONST  GUID            *Guid,
  IN        UINTN           TokenNumber,
  OUT       PCD_INFO        *PcdInfo
  )
{
  EFI_STATUS Status;

  Status = GetPiPcdInfoProtocolPointer()->GetInfo (Guid, TokenNumber, (EFI_PCD_INFO *) PcdInfo);
  ASSERT_EFI_ERROR (Status);
}

/**
  Retrieve the currently set SKU Id.

  If the sku id got >= PCD_MAX_SKU_ID, then ASSERT().

  @return   The currently set SKU Id. If the platform has not set at a SKU Id, then the
            default SKU Id value of 0 is returned. If the platform has set a SKU Id, then the currently set SKU
            Id is returned.
**/
UINTN
EFIAPI
LibPcdGetSku (
  VOID
  )
{
  UINTN SkuId;

  SkuId = GetPiPcdInfoProtocolPointer()->GetSku ();
  ASSERT (SkuId < PCD_MAX_SKU_ID);

  return SkuId;
}

