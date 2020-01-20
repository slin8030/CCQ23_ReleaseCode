//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/** @file
  Miscellaneous services internal to USB debug port implementation.

  Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include "UsbDebugPortLibInternal.h"

/**
  Verifies if the bit positions specified by a mask are set in a register.

  @param[in, out] Register		UNITN register
  @param[in] BitMask			32-bit mask

  @return	BOOLEAN	- TRUE if all bits specified by the mask are enabled.
					- FALSE even if one of the bits specified by the mask 
					  is not enabled.
**/
BOOLEAN
IsBitSet(
	IN OUT	UINTN	Register, 
	IN		UINT32	BitMask
	)
{
	BOOLEAN		Result = FALSE;
  if ((MmioRead32 (Register) & (BitMask)) != 0)
		Result = TRUE;
	return Result;
}

/**
  Verifies if the bit positions specified by a mask are cleared in a register.

  @param[in, out] Register		UINTN register
  @param[in] BitMask			32-bit mask

  @return	BOOLEAN	- TRUE if all bits specified by the mask are disabled.
					- FALSE even if one of the bits specified by the mask 
					  is enabled.
**/
BOOLEAN
IsBitClear(
	IN OUT	UINTN	Register, 
	IN		UINT32	BitMask
	)
{
	BOOLEAN		Result = FALSE;
  if ((MmioRead32 (Register) & (BitMask)) == 0)
		Result = TRUE;
	return Result;
}

/**
  Sets bits as per the enabled bit positions in the mask

  @param[in, out] Register		UINTN register
  @param[in] BitMask			32-bit mask

**/
VOID
SetR32Bit(
	IN OUT	UINTN	Register, 
	IN		UINT32	BitMask
)                 
{                                                   
    UINT32    RegisterValue;

    RegisterValue = MmioRead32 (Register);
    RegisterValue |= (UINT32)(BitMask);               
    MmioWrite32 (Register, RegisterValue);
}

/**
  Clears bits as per the enabled bit positions in the mask

  @param[in, out] Register		UINTN register
  @param[in] BitMask			32-bit mask

**/
VOID
ClrR32Bit(
	IN OUT	UINTN	Register, 
	IN		UINT32	BitMask
)                 
{                                                   
    UINT32    RegisterValue;

    RegisterValue = MmioRead32 (Register);
    RegisterValue &= (UINT32)(~(BitMask));            
    MmioWrite32 (Register, RegisterValue);         
}

/**
  Clears bits as per the enabled bit positions in the mask
  and sets new value to the register.

  @param[in, out] Register		UINTN register
  @param[in] BitMask			32-bit mask
  @param[in] Value			    32-bit Value

**/
VOID
ClrAndSetR32Bit(
	IN OUT	UINTN	Register, 
	IN		UINT32	BitMask,
	IN		UINT32	Value
)                 
{                                                   
    UINT32    RegisterValue;

    RegisterValue = MmioRead32 (Register);  
    RegisterValue &= (UINT32)(~(BitMask));            
    RegisterValue |= (UINT32)(Value);                 
    MmioWrite32 (Register, RegisterValue);            
}

/**
  Sets bits as per the enabled bit positions in the mask

  @param[in, out] Register		UINTN register
  @param[in] BitMask			26-bit mask

**/
VOID
Set16Bit(
	IN OUT	UINTN	Register, 
	IN		UINT16	BitMask
)                 
{                                                   
    UINT16    RegisterValue;

    RegisterValue = MmioRead16 (Register);
    RegisterValue |= (UINT16)(BitMask);                
    MmioWrite16 (Register, RegisterValue);
        
}

/**
  Clears bits as per the enabled bit positions in the mask

  @param[in, out] Register		UINTN register
  @param[in] BitMask			16-bit mask

**/
VOID
ClrR16Bit(
	IN OUT	UINTN	Register, 
	IN		UINT16	BitMask
)                 
{                                                   
    UINT16    RegisterValue;

    RegisterValue = MmioRead16 (Register);
    RegisterValue &= (UINT16)(~(BitMask));           
    MmioWrite16 (Register, RegisterValue); 
          
}

/**
  Clears bits as per the enabled bit positions in the mask

  @param[in, out] Register		UINTN register
  @param[in] BitMask			8-bit mask

**/
VOID
SetR8Bit(
	IN OUT	UINTN	Register, 
	IN		UINT8	BitMask
) 
{                                                   
    UINT8    RegisterValue;

    RegisterValue = MmioRead8 (Register);
    RegisterValue |= (UINT8)(BitMask);                
    MmioWrite8 (Register, RegisterValue);           
}

/**
  Clears bits as per the enabled bit positions in the mask

  @param[in, out] Register		UINTN register
  @param[in] BitMask			8-bit mask

**/
VOID
ClrR8Bit(
	IN OUT	UINTN	Register, 
	IN		UINT8	BitMask
)                 
{                                                   
    UINT8    RegisterValue;

    RegisterValue = MmioRead8 (Register);
    RegisterValue &= (UINT8)(~(BitMask));           
    MmioWrite8 (Register, RegisterValue);           
}


/**
  Initialize the USB debug port hardware.
  
  If no initialization is required, then return RETURN_SUCCESS.
  If the serial device was successfully initialized, then return RETURN_SUCCESS.
  If the serial device could not be initialized, then return RETURN_DEVICE_ERROR.
  
  @retval RETURN_SUCCESS        The serial device was initialized.
  @retval RETURN_DEVICE_ERROR   The serial device could not be initialized.

**/
RETURN_STATUS
EFIAPI
UsbDebugPortInitialize (
  VOID
  )
{
  USBInitialize ();
  return RETURN_SUCCESS;
}

/**
  Write data from buffer to USB debug port. 
 
  Writes NumberOfBytes data bytes from Buffer to the serial device.  
  The number of bytes actually written to the serial device is returned.
  If the return value is less than NumberOfBytes, then the write operation failed.
  If Buffer is NULL, then ASSERT(). 
  If NumberOfBytes is zero, then return 0.

  @param  Buffer           Pointer to the data buffer to be written.
  @param  NumberOfBytes    Number of bytes to written to the serial device.

  @retval 0                NumberOfBytes is 0.
  @retval >0               The number of bytes written to the serial device.  
                           If this value is less than NumberOfBytes, then the read operation failed.

**/
UINTN
EFIAPI
UsbDebugPortWrite (
  IN UINT8     *Buffer,
  IN UINTN     NumberOfBytes
  )
{
  UsbDbgOut (Buffer, &NumberOfBytes);
  return NumberOfBytes;
}

/**
  Read data from USB debug port and save the datas in buffer.
 
  Reads NumberOfBytes data bytes from a serial device into the buffer
  specified by Buffer. The number of bytes actually read is returned. 
  If the return value is less than NumberOfBytes, then the rest operation failed.
  If Buffer is NULL, then ASSERT(). 
  If NumberOfBytes is zero, then return 0.

  @param  Buffer           Pointer to the data buffer to store the data read from the serial device.
  @param  NumberOfBytes    Number of bytes which will be read.

  @retval 0                Read data failed, no data is to be read.
  @retval >0               Actual number of bytes read from serial device.

**/
UINTN
EFIAPI
UsbDebugPortRead (
  OUT UINT8   *Buffer,
  IN  UINTN   NumberOfBytes
  )
{
  UsbDbgIn (Buffer, &NumberOfBytes);
  return NumberOfBytes;
}
