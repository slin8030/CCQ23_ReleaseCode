
#include <CompalEcLib.h>

//[COM] Start - EC Lib 00.00.16 modify Intel EC Index IO to 0xFD60
//[COM-START] Auto detect EC interface
//Memory Mapping check
#define  EC_MEMMAPPING_CHECK_OFFSET       0xF991
#define  EC_MEMMAPPING_CHECK_DATA         0xAA

//Debug Code on Port 80
#define  EC_DEBUG_MEMMAPPING_TEST         0xB0
#define  EC_DEBUG_INDEXIO_TEST            0xB1
//[COM-End]

//[COM-START] Auto detect EC interface
VOID
ECMemoryMappingWriteDirect (
  IN  UINTN  Offset,
  IN  UINT8  WriteData
)
/*!
Routine Description: \n
  - Write specific data to intnerl EC RAM through Memory Mapping

Arguments:
  - Offset - the address of EC RAM which you want to access
  - WriteData - data of ECRAM for writing

Returns:
	- N/A
*/
{
	UINT16	MemOffset;
	
	MemOffset = (UINT16)(Offset - EC_RAM_START_ADDRESS);
	MmioWrite8 (EC_MEMORY_MAPPING_BASE_ADDRESS+MemOffset,WriteData);
}

UINT8
EcMemoryMappingTest(
  IN UINT8  MemoryData
)
/*!
Routine Description: \n
  - Verify Write/Read Data through Memory Mapping

Arguments:
  - MemoryData - data of ECRAM for writing

Returns:
	- TRUE - Supported
	- FALSE - Not Support
*/
{
  ECMemoryMappingWriteDirect(EC_MEMMAPPING_CHECK_OFFSET, MemoryData);
  if( CompalECMemoryMappingRead(EC_MEMMAPPING_CHECK_OFFSET) == MemoryData)
    return TRUE;
  else
    return FALSE;
}

UINT8
EcIndexIoTest(
  IN UINT8  MemoryData
)
/*!
Routine Description: \n
  - Verify Write/Read Data through Index IO

Arguments:
  - MemoryData - data of ECRAM for writing

Returns:
	- TRUE - Supported
	- FALSE - Not Support
*/
{
  CompalECIndexIOWrite(EC_MEMMAPPING_CHECK_OFFSET, MemoryData);
  if( CompalECIndexIORead(EC_MEMMAPPING_CHECK_OFFSET) == MemoryData)
    return TRUE;
  else
    return FALSE;
}
//[COM-END]


/**
  EC initialize.

  @retval EFI_UNSUPPORTED    Returns unsupported by default.
  @retval EFI_SUCCESS        The service is customized in the project.
  @retval EFI_MEDIA_CHANGED  The value of IN OUT parameter is changed. 
**/
EFI_STATUS
EcInit (
  )
{
//[COM] Start - EC Lib 00.00.17 modify Intel EC Memory Mapping Base Address
  UINT8    B_BIOS_DEC_EN1;
  UINT8    CheckResult;
 
  //
  //  Set EC indexed IO
  //  
  IoWrite32 ( 0xCF8, 0x8000F888 );
// [COM] Start - EC Lib 00.00.16 modify Intel EC Index IO to 0xFD60
  IoWrite16 ( 0xCFC, (ENEIOBASE | 0x01) );	
// [COM] End

  //
  // Set EC Memory Mapping Base Address
  //
  IoWrite32 ( 0xCF8, 0x8000F8D8 );
  B_BIOS_DEC_EN1 = IoRead8(0xCFC);
  IoWrite32 ( 0xCF8, 0x8000F8D8 );
  IoWrite8 ( 0xCFC, (B_BIOS_DEC_EN1 & 0xFE) );
  
  IoWrite32 ( 0xCF8, 0x8000F898 );
  IoWrite32 ( 0xCFC, (EC_MEMORY_MAPPING_BASE_ADDRESS | 0x01) );
  
  //[COM-START] Auto detect EC interface
  //Test Memory Mapping command
  //POSTCODE(EC_DEBUG_MEMMAPPING_TEST);
  CheckResult = EcMemoryMappingTest(EC_MEMMAPPING_CHECK_DATA);
  if( CheckResult == TRUE )
    return EFI_UNSUPPORTED;
  else {
  //Test Index IO
	//POSTCODE(EC_DEBUG_INDEXIO_TEST); 
    do {
      CheckResult = EcIndexIoTest(EC_MEMMAPPING_CHECK_DATA);
	} while( CheckResult == FALSE );
  }
  //[COM-END]

//[COM] End - EC Lib 00.00.17 modify Intel EC Memory Mapping Base Address  
	
  return EFI_UNSUPPORTED;
}
