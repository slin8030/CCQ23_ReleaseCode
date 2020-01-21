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
 --------  --------   ------------ -------------------- -----------------------------------------------------------------
 1.00      13/08/22   Jeter_Wang   BTT/BSW/CHT          Init version.
 1.01      16/05/26   Brics_Chen   BTT/BSW/CHT          Fix CompalECReadNamespace_IO373X function fail.
*/


#include <CompalEcLibPei.h>

EFI_STATUS
EFIAPI
CompalGetPlatformIDPei_IO373X (
  OUT UINT8					*Platform_ID
  )
{	
	EFI_STATUS		Status;
	UINT8 			Data8[6];
	
	// Send PlatformID CMD
    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = PLATFORM_ID_CMD;
	Data8[2] = 0xA1;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to send PlatformID command\n"));
		return	Status;
	}	
	
	// Read Platform ID - 6 bytes
    
    Data8[0] = EC_FWMode_READ_CMD;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
	Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,6,Platform_ID);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to read Platform ID \n"));
		return	Status;
	}	

	
	return	Status;
}

EFI_STATUS
EFIAPI
CompalGetProjectIDPei_IO373X (
  OUT UINT8					*Project_ID
  )
{	
	EFI_STATUS		Status;
	UINT8 			Data8[2];
	
	// Send ProjectID CMD
    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = PROJECT_ID_CMD;
    
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,2,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to send ProjectID command\n"));
		return	Status;
	}	
	
	// Read Project ID - 2 bytes
    Data8[0] = EC_FWMode_READ_CMD;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
	Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,2,Project_ID);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to read Project ID \n"));
		return	Status;
	}	
	
	return	Status;
}

EFI_STATUS
EFIAPI
CompalECAcpiMode (
	IN  BOOLEAN	 			Mode
  )
{
	EFI_STATUS		Status;
	UINT8 			Data8[3];
    
    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = EC_SYSTEM_NOTIFI_CMD;
	if (Mode){	//Enable Acpi mode
		Data8[2] = Enable_ACPI_Mode;	
		Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite);
		if(EFI_ERROR(Status)) {
			DEBUG((EFI_D_ERROR,"Unable to send Acpi enable command\n"));
			return	Status;
		}		
	}
	else
	{	//Disable Acpi mode	
		Data8[2] = Disable_ACPI_Mode;
		Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite);
		if(EFI_ERROR(Status)) {
			DEBUG((EFI_D_ERROR,"Unable to send Acpi disable command\n"));
			return	Status;
		}
	}

	return	Status;
}  

EFI_STATUS
EFIAPI
CompalECWriteNamespacePei_IO373X (
  IN  	UINT8 				Addr,
  IN  	UINT8 				Data
  )
{
	EFI_STATUS		Status;
	UINT8 			Data8[4];
	
    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = Namespace_WRITE_CMD;
	Data8[2] = Addr;
	Data8[3] = Data;
	//Send Write Namespace command	
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,4,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to write EEPROM\n"));
		return	Status;
	}	

	return	Status;
}  
 

EFI_STATUS
EFIAPI
CompalECReadNamespacePei_IO373X (
  IN  	UINT8 				Addr,
  OUT  	UINT8 				*DataBuffer
  )
{
	EFI_STATUS		Status;
	UINT8 			Data8[3];
	
    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = Namespace_READ_CMD;
	Data8[2] = Addr;
	// Send Read Namespace command	
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to write Namespace command\n"));
		return	Status;
	}	

	// Read Namespace - 1 byte
    Data8[0] = EC_FWMode_READ_CMD;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
	Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,DataBuffer);	
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to read Namespace command\n"));
		return	Status;
	}	
	return	Status;
} 
EFI_STATUS
EFIAPI
CompalEepromWriteBytePei_IO373X(
    IN  	UINT8   			Offset,
    IN  	UINT8   			WriteData
    )
{
	EFI_STATUS		Status;
	UINT8 			  Data8[6];

  Data8[0] = EepromWriteCMD;
  Data8[1] = Offset;
  Data8[2] = WriteData;

 	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite); 

	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Write Failed\n"));
		return	Status;
	}	
	
	return	Status;
}

EFI_STATUS
EFIAPI
CompalEepromReadBytePei_IO373X(
    IN  	UINT8   			Offset,
    OUT 	UINT8   			ReadData
    )
{
	EFI_STATUS			Status;
	UINT8 			    Data8[6];
    
    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = EepromReadCMD;
	Data8[2] = Offset;

	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite);  
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Write Failed\n"));
		return	Status;
	}	
	
    Data8[0] = EC_FWMode_READ_CMD;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
  	Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,&ReadData);

	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Read Failed\n"));
		return	Status;
	}	

	return	Status;
}

EFI_STATUS
EFIAPI
CompalGetEcFwVersionPei_IO373X(
    OUT 	UINT8 			*EcVerBuffer 
    )
{
	EFI_STATUS		Status;
	UINT8 			Data8[6]={0};

    Data8[0] = EC_FWMode_WRITE_CMD;
    Data8[1] = EC_FW_VER_CMD;
    Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,2,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Send EC FW Version Failed\n"));
		return	Status;
	}
    Data8[0] = EC_FWMode_READ_CMD;
    Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
    Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,4,EcVerBuffer);
  	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Get EC FW Version Failed\n"));
		return	Status;
	}	
	return	Status;
}


EFI_STATUS
EFIAPI
CompalGetPrjCodePei_IO373X(
    OUT 	CHAR8 			*PrjCodeBuffer 
    )
{
	EFI_STATUS			Status;
	UINT8 			 	Data8[6];

    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = PRJ_CODE_CMD;
	Data8[2] = 0xA0;
    Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,3,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Send project code failed\n"));
		return	Status;
	}	
    
    Data8[0] = EC_FWMode_READ_CMD;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
    Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,5,PrjCodeBuffer);
  	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Get project code failed\n"));
		return	Status;
	}	

	return	Status;
}

EFI_STATUS
EFIAPI
CompalGetSCIEvenIDPei_IO373X(
    OUT 	UINT8 			*EventID
    )
{
	EFI_STATUS		Status;
	UINT8 			Data8[2];

    Data8[0] = EC_FWMode_WRITE_CMD;
	Data8[1] = SCI_EventID_READ_CMD;
    Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Send Q-EventID failed\n"));
		return	Status;
	}	

    Data8[0]  = EC_FWMode_READ_CMD;
	Status = CompalByteWriteI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,Data8,EcRegRead);
    Status = CompalByteReadI2CPei_Serial(ULPMC_I2C_BUSNO, ULPMC_FWmode_I2C_ADDR,1,EventID);
  	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Get Q-EventID failed\n"));
		return	Status;
	}	
	
	
	return	Status;
}


