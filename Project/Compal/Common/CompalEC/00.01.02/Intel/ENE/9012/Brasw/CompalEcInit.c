/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Champion Liu Init version.
*/

#include <Library/PcdLib.h>
#include <CompalEcLib.h>

#define R_PCH_ACPI_PM1_CNT                0x04
#define B_PCH_ACPI_PM1_CNT_SLP_TYP        0x00001C00
#define V_PCH_ACPI_PM1_CNT_S3             0x00001400

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
  UINT16   Pm1Cnt;

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
  if( CheckResult != TRUE ) {
    //Test Index IO
    //POSTCODE(EC_DEBUG_INDEXIO_TEST); 
      CheckResult = EcIndexIoTest(EC_MEMMAPPING_CHECK_DATA);
      if( CheckResult != TRUE )
        return EFI_UNSUPPORTED;
  }
  //[COM-END]

//[COM] End - EC Lib 00.00.17 modify Intel EC Memory Mapping Base Address  

  //
  // Read the ACPI registers
  //
  Pm1Cnt = IoRead16 (PcdGet16 (PcdPerfPkgAcpiIoPortBaseAddress) + R_PCH_ACPI_PM1_CNT);

  //
  // if boot mode isn't S3, then disable EC ACPI mode.
  //
  if ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) != V_PCH_ACPI_PM1_CNT_S3) {
      CompalECAcpiMode(FALSE); 
  } 

  return EFI_SUCCESS;
}

EFI_STATUS
CompalEcInit (
  )
{
  EcInit ();
  return EFI_SUCCESS;
}
