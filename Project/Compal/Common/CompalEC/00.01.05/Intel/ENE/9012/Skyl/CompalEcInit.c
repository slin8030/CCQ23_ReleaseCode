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
 1.02      11/17/15  Champion Liu Modify the "CompalECIndexIOWrite" function to add loop counter.
 1.03      12/25/15  Vanessa Chen Move APM cmd to Crisis function code.
 1.04      05/09/16  Thomas Chen  Modfiy Memory Mapping check offset to 0xF980
*/

#include <Library/PcdLib.h>
#include <CompalEcLib.h>

#define R_PCH_ACPI_PM1_CNT                0x04
#define B_PCH_ACPI_PM1_CNT_SLP_TYP        0x00001C00
#define V_PCH_ACPI_PM1_CNT_S3             0x00001400

//[COM] Start - EC Lib 00.00.16 modify Intel EC Index IO to 0xFD60
//[COM-START] Auto detect EC interface
// Index IO/Memory Mapping check
//  EC RAM Offset F980 -> 5A and Offset F981 -> A5
//
#define  EC_MEMMAPPING_CHECK_OFFSET       0xF980
#define  EC_MEMMAPPING_CHECK_DATA1        0x5A
#define  EC_MEMMAPPING_CHECK_DATA2        0xA5

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
EcMemoryMappingTest()
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
  if (CompalECMemoryMappingRead(EC_MEMMAPPING_CHECK_OFFSET) == EC_MEMMAPPING_CHECK_DATA1)
  {
     if (CompalECMemoryMappingRead(EC_MEMMAPPING_CHECK_OFFSET + 1) == EC_MEMMAPPING_CHECK_DATA2) return TRUE;
     else return FALSE;
  }
  else return FALSE;
}

UINT8
EcIndexIoTest()
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
  if (CompalECIndexIORead(EC_MEMMAPPING_CHECK_OFFSET) == EC_MEMMAPPING_CHECK_DATA1)
  {
     if (CompalECIndexIORead(EC_MEMMAPPING_CHECK_OFFSET + 1) == EC_MEMMAPPING_CHECK_DATA2) return TRUE;
     else return FALSE;
  }
  else return FALSE;
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

  UINT8    B_BIOS_DEC_EN1;
  UINT8    CheckResult;
  UINT16   Data16;
  EFI_STATUS    Status=EFI_SUCCESS;

  //
  // Set EC indexed IO
  //
  //[Set PCH LPC/eSPI generic IO range]
  //    This cycle decoding is allowed to set when DMIC.SRL is 0.
  //    Steps of programming generic IO range:
  //    1. Program LPC/eSPI PCI Offset 84h ~ 93h of Mask, Address, and Enable.
  //    2. Program LPC/eSPI Generic IO Range #, PCR[DMI] + 2730h ~ 273Fh to the same value programmed in LPC/eSPI PCI Offset 84h~93h.

  //
  // 1.Program LPC/eSPI PCI Offset 84h ~ 93h of Mask, Address, and Enable.
  //
  IoWrite32 ( 0xCF8, 0x8000F888 );                  // LPC Generic I/O Range 2
  IoWrite16 ( 0xCFC, (ENEIOBASE | BIT0) );	

  IoWrite32 ( 0xCF8, 0x8000F88C );                  // LPC Generic I/O Range 3
  IoWrite32 ( 0xCFC, 0x68 | BIT18+BIT0);	

  //
  // 2.Program LPC Generic IO Range #, PCR[DMI] + 2730h ~ 273Fh to the same value programmed in LPC/eSPI PCI Offset 84h~93h.
  // PCH_PCR_BASE_ADDRESS 0xFD000000 >SBREG MMIO base address
  // PID_DMI = 0xEF
  MmioWrite32 (0xFDEF2734, (UINT32)(ENEIOBASE | (BIT18+BIT19+BIT0)));
  MmioWrite32 (0xFDEF2738, (UINT32)(0x68 | BIT18+BIT0));

  //
  // Set EC Memory Mapping Base Address
  //
  //[Set PCH LPC/eSPI memory range decoding]
  //    This cycle decoding is allowed to set when DMIC.SRL is 0.
  //    Programming steps:
  //    1. Program LPC/eSPI PCI Offset 98h [0] to [0] to disable memory decoding first before changing base address.
  //    2. Program LPC/eSPI PCI Offset 98h [31:16, 0] to [Address, 1].
  //    3. Program LPC/eSPI Memory Range, PCR[DMI] + 2740h to the same value programmed in LPC/eSPI PCI Offset 98h.

  // Enable decoding of 0xff000000-0xff0fffff
  IoWrite32 ( 0xCF8, 0x8000F8D8 );
  B_BIOS_DEC_EN1 = IoRead8(0xCFC);
  IoWrite32 ( 0xCF8, 0x8000F8D8 );
  IoWrite8 ( 0xCFC, (B_BIOS_DEC_EN1 & 0xFE) );

  //
  // program LPC BIOS Decode Enable, PCR[DMI] + 2744h to the same value programmed in LPC or SPI Offset D8h.
  //
  Data16 = MmioRead16 (0xFDEF2744);
  MmioWrite16 (0xFDEF2744, (Data16 & ~BIT0));

  //
  // 1.Program LPC/eSPI PCI Offset 98h [0] to [0] to disable memory decoding first before changing base address.
  //
  IoWrite32 ( 0xCF8, 0x8000F898 );
  IoWrite32 ( 0xCFC, (EC_MEMORY_MAPPING_BASE_ADDRESS & ~0x01));

  //
  // 2.Program LPC/eSPI PCI Offset 98h [31:16, 0] to [Address, 1].
  //
  IoWrite32 ( 0xCFC, (EC_MEMORY_MAPPING_BASE_ADDRESS | 0x01));

  //
  // 3.Program LPC Memory Range, PCR[DMI] + 2740h to the same value programmed in LPC/eSPI PCI Offset 98h.
  // R_PCH_PCR_DMI_LPCGMR            0x2740                ///< LPC Generic Memory Range
  //
  MmioWrite32 (0xFDEF2740, (EC_MEMORY_MAPPING_BASE_ADDRESS | 0x01));

  //Auto detect EC interface
  //Test Memory Mapping command
  //POSTCODE(EC_DEBUG_MEMMAPPING_TEST);
  CheckResult = EcMemoryMappingTest();
  if( CheckResult != TRUE ) {
      //Test Index IO
      //POSTCODE(EC_DEBUG_INDEXIO_TEST); 
      CheckResult = EcIndexIoTest();
      if( CheckResult != TRUE )
        Status = EFI_UNSUPPORTED;
  }
  DEBUG((DEBUG_ERROR, "Compal EC init -INDEX I/O and Memeory decoding fail \n"));
	
  //
  // Read the ACPI registers
  //
  Pm1Cnt = IoRead16 (PcdGet16 (PcdPerfPkgAcpiIoPortBaseAddress) + R_PCH_ACPI_PM1_CNT);

  return Status;
}

EFI_STATUS
CompalEcInit (
  )
{
  EcInit ();
  return EFI_SUCCESS;
}
