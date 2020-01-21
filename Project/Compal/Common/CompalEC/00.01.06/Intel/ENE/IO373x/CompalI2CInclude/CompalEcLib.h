/*
 * (C) Copyright 2013-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2013-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
 R E V I S I O N    H I S T O R Y
 
 Ver       Date       Who          Change           Support Chipset
 --------  --------   ------------ ---------------- -----------------------------------
 1.00      13/8/22    Jeter_Wang   Init version.     BTT/BSW/CHT
*/


#ifndef __Compal_EcLib_H__
#define __Compal_EcLib_H__

#include <Uefi.h>
#include <Library/DebugLib.h>

#include <CompalECI2CLib.h>
#include <Library/UefiBootServicesTableLib.h>


/*Compal EC FW Mode CMD - Start*/
#define ULPMC_I2C_BUSNO 		0
#define ULPMC_FWmode_I2C_ADDR 	( 0x70)	//bit8 = 0xE0

#define EC_FWMode_READ_CMD      0x01
#define EC_FWMode_WRITE_CMD     0x02

#define PLATFORM_ID_CMD			0x41
#define PROJECT_ID_CMD			0x44

//[PRJ] Start
#define PROJECT_SPECIFIC_CMD    0x45
#define   PROJECT_DETECT_POWER_ON_TYPE_CMD        0x20
#define   PROJECT_DETECT_POWER_BUTTON_HOLD_CMD    0x21
#define   PROJECT_GET_GASGUAGE_CMD                0x22
#define   PROJECT_DETECT_POWER_BUTTON_PRESS_CMD   0x23
//[PRJ] End

#define EepromWriteCMD          0x4D
#define EepromReadCMD           0x4E
#define EC_FW_VER_CMD           0x51
#define PRJ_CODE_CMD            0x52

#define EC_SYSTEM_NOTIFI_CMD    0x59
#define  Enable_ACPI_Mode       0xE8
#define  Disable_ACPI_Mode      0xE9

#define	Namespace_READ_CMD      0x80
#define Namespace_WRITE_CMD     0x81
#define SCI_EventID_READ_CMD    0x84



/*Compal EC FW Mode CMD - End*/

/*Compal EC HW Mode CMD - Start*/
#define ULPMC_HWmode_I2C_ADDR 	( 0x60)	//bit8 = 0xC0

#define	SET_ADDR_CMD            0x00
#define	READ_ADDR_CMD           0x11

#define WRITE_BYTE_CMD          0x01
#define WRITE_WORD_CMD          0x02
#define	WRITE_BLOCK_CMD         0x03


#define READ_BYTE_CMD           0x81
#define READ_WORD_CMD           0x82
#define READ_BLOCK_CMD          0x80
/*Compal EC HW Mode CMD - End*/


enum
{
    EcRegWrite,
    EcRegRead
};


EFI_STATUS
EFIAPI
CompalSetAddr_IO373X (
  IN  UINT16				RegAddr
  );
  
EFI_STATUS
EFIAPI
CompalECRegByteRW_IO373X (
  IN  UINT16				RegAddr,
  IN  OUT UINT8 			*p1byte,
  IN  UINT8 				Accesstype
  );
  
EFI_STATUS
EFIAPI
CompalECRegWordRW_IO373X (
  IN  UINT16				RegAddr,
  IN  OUT UINT16 			p2byte,
  IN  UINT8 				Accesstype
  );
  
EFI_STATUS
EFIAPI
CompalGetPlatformID_IO373X (
  OUT UINT8					*Platform_ID
  );

EFI_STATUS
CompalGetProjectID_IO373X (
  OUT UINT8					*Project_ID
  );
  
EFI_STATUS
EFIAPI
CompalECAcpiMode (
  IN  BOOLEAN				Mode
  );

EFI_STATUS
EFIAPI
CompalECWriteNamespace_IO373X (
  IN  UINT8 				Addr,
  IN  UINT8 				Data
  );
  
EFI_STATUS
EFIAPI
CompalECReadNamespace_IO373X (
  IN  UINT8 				Addr,
  OUT UINT8 				*DataBuffer
  ); 
  
EFI_STATUS
EFIAPI
CompalEepromWriteByte_IO373X(
   IN  UINT8   				Offset,
   IN  UINT8   				WriteData
    );

EFI_STATUS
EFIAPI
CompalEepromReadByte_IO373X(
   IN  	UINT8   			Offset,
   OUT 	UINT8   			ReadData
    );

EFI_STATUS
EFIAPI
CompalGetEcFwVersion_IO373X(
   OUT 	UINT8 				*EcVerBuffer 
    );

EFI_STATUS
EFIAPI
CompalGetPrjCode_IO373X(
   OUT 	CHAR8 				*PrjCodeBuffer 
    );  

EFI_STATUS
EFIAPI
CompalGetSCIEvenID_IO373X(
   OUT 	UINT8 				*EventID
    );	
  
#endif  // __Compal_EcLib_H__
