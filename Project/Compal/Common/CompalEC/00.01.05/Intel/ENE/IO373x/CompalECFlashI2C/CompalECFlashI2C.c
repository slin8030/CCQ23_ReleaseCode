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

#include <CompalECFlashI2C.h>

SHELL_PARAM_ITEM ParamList[] = {
  {L"-n", TypePosition},
  {L"-f", TypePosition},
  {L"-v", TypeValue},  
  {L"-i", TypeValue},
  {L"-t", TypeValue},//test only
  {NULL, TypeMax}
  };
  
VOID
ShowTitle (
  unsigned short  *u_name,
  unsigned short  *u_version,
  unsigned short  *u_author,
  unsigned short  *u_vendor,
  unsigned short  *u_copyright,
  unsigned short  *u_last_modify,
  unsigned short  *u_last_modifier
  )
{
    Print(L"\n======================================================================\n");
	gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
	Print(L"%s  \n", u_name);
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY);
    Print(L"%s %s\nVersion: %s\nAuthor: %s\nLast Modified: %s by %s\n",
          u_vendor,
          u_copyright,
          u_version,
          u_author,		  
          u_last_modify,
          u_last_modifier);
    Print(L"======================================================================\n");
}

VOID
CompalHelpInfo()
{
    Print(L" Usage:\n");
    Print(L"        CompalECFlashI2C [CMD] [EC BIN FILE PATH]\n\n");
	Print(L"        [CMD]\n");
	Print(L"         -f    : Flash EC ROM from [EC BIN FILE PATH].\n");
	Print(L"         -v    : Show current EC version.\n");
	Print(L"         -i    : Show EC CHIP ID.\n");
	Print(L"\n");
    Print(L" Example:\n");
    Print(L"         CompalECFlashI2C -f fs1:\\EcFileName.rom\n");
	Print(L"         CompalECFlashI2C -v\n");
	Print(L"         CompalECFlashI2C -i\n");
	Print(L"\n");
}

VOID
CompalTestResult (
  EFI_STATUS  f_Status, 
  BOOLEAN     *f_TestFailed
  )
{
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY);
    Print(L"[ ");
    if(f_Status == EFI_SUCCESS) {
        gST->ConOut->SetAttribute (gST->ConOut, EFI_GREEN);
        Print(L"PASS");
    } else {
        gST->ConOut->SetAttribute (gST->ConOut, EFI_RED);
        Print(L"FAILED");
        *f_TestFailed = TRUE;
    }
    gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY);
    Print(L" ]\n");
}

EFI_STATUS
EFIAPI
CompalGetChipID_IO373X (
  OUT UINT16				*Chip_ID
  )
{	
	EFI_STATUS		Status;
	UINT8 			Data8;
	
	// Read Chip ID high byte
	Status = CompalECRegByteRW_IO373X(REG373_CHIPID_H, &Data8, EcRegRead);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to read CHIPID_H\n"));
		return	Status;
	}
	*Chip_ID = (UINT16)(Data8 << 8);
	
	// Read Chip ID low byte
	Status = CompalECRegByteRW_IO373X(REG373_CHIPID_L, &Data8, EcRegRead);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to read CHIPID_L\n"));
		return	Status;
	}
	*Chip_ID = *Chip_ID | (UINT16)Data8;
	
	if(*Chip_ID != IO373x_CHIPID) {
		DEBUG((EFI_D_ERROR,"This EC is not ENE IO373X\n"));
		return	EFI_DEVICE_ERROR;
	}
	
	return	Status;
}

EFI_STATUS
EFIAPI
CompalDisableWatchDog_IO373X (
  )
{
	EFI_STATUS		Status;
	UINT8 			Data8;
	
	// Disable WDT(WatchDog Timer) to prevent from EC reset 
	Data8 = 0x48;
	Status = CompalECRegByteRW_IO373X(REG373_WDTCFG, &Data8, EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to disable WDT\n"));
		return Status;
	}
	
	// Clear WDT pending flags which are the reset records
	Data8 = 0x03;
	Status = CompalECRegByteRW_IO373X(REG373_WDTPF, &Data8, EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to clear WDT pending flags\n"));
		return Status;
	}
	
	return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CompalGetAvaliableBuffer (
  OUT UINT16  				*AddrOffset
  )
{
  EFI_STATUS Status = EFI_NOT_FOUND;
  UINT8 Data8;

  CompalECRegByteRW_IO373X(0x80F6, &Data8, EcRegRead);
  Data8 &= 0x0f;
  if(Data8 == 0x02) {
    *AddrOffset = 0x0000;
    return EFI_SUCCESS;
  }
  CompalECRegByteRW_IO373X(0x81F6, &Data8, EcRegRead);
  Data8 &= 0x0f;
  if(Data8 == 0x02) {
    *AddrOffset = 0x0100;
    return EFI_SUCCESS;
  }
  return Status;
}

EFI_STATUS
EFIAPI
CompalCheckRegVal (
  IN UINT16  				AddrOffset,
  IN UINT8 					RegVal,
  IN UINT8 					RegMask
  )
{
  UINT8 Counter, Data8;
  EFI_STATUS Status = EFI_SUCCESS;

  Counter = 100;
  do {
    CompalECRegByteRW_IO373X(AddrOffset, &Data8, EcRegRead);
    Data8 &= RegMask;
    if(Data8 == RegVal) {
      break;
    }
    gBS->Stall(1000);   // delay 1ms
    Counter--;
  } while (Counter !=0);

  if(Counter == 0) {
    return EFI_NOT_FOUND;
  }
  return Status;
}

//we have to ensure that previously Buffer has done before we start to run command from another buffer
EFI_STATUS
EFIAPI
CompalWaitBufferFree (
  IN UINT16  *AddrOffset
  )
{
  EFI_STATUS Status = EFI_NOT_FOUND;

  if ( *AddrOffset == 0x0000 ) {
    //check last buffer action whether done, current is use Buffer0, so check buffer 1
    Status = CompalCheckRegVal(0x81F6, 0x02, 0x02);
    if(Status != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "EC Buffer 1 is busy\n."));
    }
  }
  else {
    //check last buffer action whether done, current is use Buffer1, so check buffer 0
    Status = CompalCheckRegVal(0x80F6, 0x02, 0x02);
    if(Status != EFI_SUCCESS) {
      DEBUG((EFI_D_ERROR, "EC Buffer 1 is busy\n."));
    }
  }
  return Status;
}
 
VOID
EFIAPI
CompalGet_Updated_Version_IO373X (
  IN  UINT8		*FileBuffer,
  OUT UINT32	*EcVersion
  )
{
	*EcVersion = *(UINT32*)(FileBuffer+ROM_VERSION_ADDR);
}

EFI_STATUS
EFIAPI
CompalErase_IO373X (

  )
{
	EFI_STATUS	Status = EFI_SUCCESS;
	UINT8		Data8;
	UINT16		i;
	UINT16		AvalAddr;
	UINT16		Address;
	UINTN		Complete_Percent;
	
	for(i = 0; i < EC_32KB; i += EC_128B) {
		// Get Avaliable Buffer, Buffer[0] or Buffer[1]
		Status = CompalGetAvaliableBuffer(&AvalAddr);
		if(EFI_ERROR(Status)) {
			DEBUG((EFI_D_ERROR, "There is no avaliabe buffer\n."));
			return	Status;
		}

		// Page erase, 0x80F7
		Address = AvalAddr | 0x80F7;  
		Data8 = PAGE_ERASE;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		if(EFI_ERROR(Status)) {
			DEBUG((EFI_D_ERROR, "There is no avaliabe buffer\n."));
			return	Status;
		}
		
		// Size is EC_128B, 0x80F3 - X080F5 (Hi->Lo Byte)
		Address = AvalAddr | 0x80F3;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F4;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F5;
		Data8 = EC_128B;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Set Address, 0x80F0 - 0x80F2 (Hi->Lo Byte)
		Address = AvalAddr | 0x80F0;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F1;
		Data8 = (UINT8)((i & 0xFF00) >> 8);
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F2;
		Data8 = (UINT8)(i & 0x00FF);
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		Status = CompalWaitBufferFree(&AvalAddr);
		// Here, the EC should be free to run new instruction
		// Clear 0x8xF6 bit[7:4] to start EC update
		Address = AvalAddr | 0x80F6;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X( Address, &Data8, EcRegWrite);
		
		// Show complete progress
		Complete_Percent = ((i * 100) / EC_32KB) + 1;
		gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_ERASE);
		Print(L"Erasing EC ... %d %c", Complete_Percent, PERCENT_SYMBOL);
	}
	
	// Show erasing process whether is PASS of FAILED
	gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_ERASE);
	Print(L"Erasing EC ... ");
	CompalTestResult(Status, NULL);
	
	return	Status;
}

EFI_STATUS
EFIAPI
CompalProgram_IO373X (
  IN  UINT8					*FileBuffer
  )
{
	EFI_STATUS	Status;
	UINT8		Data8;
	UINT8		Buffer8[EC_128B];
	UINTN		DataLength;
	UINT16		i,j;
	UINT16		AvalAddr = 0;
	UINT16		Address;
	UINTN		Complete_Percent;
	
	for(i = 0; i < EC_32KB; i += EC_128B) {
		// Get Avaliable Buffer, Buffer[0] or Buffer[1]
		Status = CompalGetAvaliableBuffer(&AvalAddr);
		if(EFI_ERROR(Status)) {
			DEBUG((EFI_D_ERROR, "There is no avaliabe buffer\n."));
			return	Status;
		}
  
		Address = AvalAddr;
		Address |= 0x8000; // data from 0x8000

		// Send 32 bytes to EC, total is 128 bytes once
		for(j = 0; j < EC_128B; j += EC_32B) {
			//ECSetAddr(Address);
			CompalSetAddr_IO373X(Address);
			DataLength = EC_32B+2;
			Buffer8[0] = WRITE_BLOCK_CMD;// Block write command
			Buffer8[1] = EC_32B;  // Byte Count
			CopyMem(&Buffer8[2], (FileBuffer + i + j), EC_32B);
			Status = CompalByteWriteI2C_Serial(ULPMC_I2C_BUSNO, ULPMC_HWmode_I2C_ADDR,DataLength,Buffer8,EcRegWrite);
			Address += EC_32B;
            gBS->Stall(1000);   // delay 1ms
		}
    
		// Set Write CMD 
		Data8 = WRITE_ROM;
		Address = AvalAddr | 0x80F7;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Size is 0x80, 0x80F3 - 0x80F5 (Hi->Lo Byte)
		Address = AvalAddr | 0x80F3;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F4;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F5;
		Data8 = EC_128B;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Set address, 0x80F0 - 0x80F2 (Hi->Lo Byte)
		Address = AvalAddr | 0x80F0;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F1;
		Data8 = (UINT8)((i & 0xFF00) >> 8);
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F2;
		Data8 = (UINT8)(i & 0x00FF);
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);

		
		Status = CompalWaitBufferFree(&AvalAddr);
		// Here, the EC should be free to run new instruction
		// Clear 0x8xF6 bit[7:4] to start EC update
		// Announce EC to start to run this command, Clear 8xF6 = 0x00
		Address = AvalAddr | 0x80F6;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Show complete progress
		Complete_Percent = ((i * 100) / EC_32KB) + 1;
		gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_PROGRAM);
		Print(L"Flashing EC ... %d %c", Complete_Percent, PERCENT_SYMBOL);
	}

	// Announce EC to start to run this command, Clear 8xF6 = 0x00
	Address = AvalAddr | 0x80F6;
	Data8 = 0x00;
	Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
	
	// Show flashing process whether is PASS of FAILED
	gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_PROGRAM);
	Print(L"Flashing EC ... ");
	CompalTestResult(Status, NULL);

	return	Status;
}

EFI_STATUS
EFIAPI
CompalVerify_IO373X (
  IN  UINT32				VerifyChecksum
  )
{
	EFI_STATUS	Status=EFI_SUCCESS;
	UINT8		Data8;
	UINT8		Buffer8[EC_128B];
	UINT16		i,j,k;
	UINT16		AvalAddr;
	UINT16		Address;
	UINT32		EcRomChecksum=0;
	UINTN		Complete_Percent;

	for(i = 0; i < EC_32KB; i += EC_128B) {
		// Get Avaliable Buffer, Buffer[0] or Buffer[1]
		Status = CompalGetAvaliableBuffer(&AvalAddr);
		if(EFI_ERROR(Status)) {
			DEBUG((EFI_D_ERROR, "There is no avaliabe buffer\n."));
			return	Status;
		}
		
		// Size is 0x80, 0x80F3 - 0x80F5 (Hi->Lo Byte)
		Address = AvalAddr | 0x80F3;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F4;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F5;
		Data8 = EC_128B;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Set address, 0x80F0 - 0x80F2 (Hi->Lo Byte)
		Address = AvalAddr | 0x80F0;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F1;
		Data8 = (UINT8)((i & 0xFF00) >> 8);
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		Address = AvalAddr | 0x80F2;
		Data8 = (UINT8)(i & 0x00FF);
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Clean status to 0x00
		Address = AvalAddr | 0x80F6;
		Data8 = 0x00;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
		
		// Set Read CMD 
		Data8 = READ_ROM;
		Address = AvalAddr | 0x80F7;
		Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
	
		// Wait for buffer free
		Status = CompalWaitBufferFree(&AvalAddr);
		
		Address = AvalAddr;
		Address |= 0x8000; // data from 0x8000

		for(j = 0; j < EC_128B; j += EC_32B) {	
			// Set Address
			CompalSetAddr_IO373X(Address);
			
			for(k=0;k<EC_32B;k++) {
				// Read EC ROM data
				Data8 = READ_BYTE_CMD;
				Status = CompalByteWriteI2C_Serial(ULPMC_I2C_BUSNO, ULPMC_HWmode_I2C_ADDR,1,&Data8,EcRegRead);
				Status = CompalByteReadI2C_Serial(ULPMC_I2C_BUSNO, ULPMC_HWmode_I2C_ADDR,1,&Buffer8[j+k]);
				EcRomChecksum = EcRomChecksum + (UINT32)Buffer8[j+k];
			}

			Address += EC_32B;
		}
	
		// Show complete progress
		Complete_Percent = ((i * 100) / EC_32KB) + 1;
		gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_VERIFY);
		Print(L"Verifying EC ... %d %c", Complete_Percent, PERCENT_SYMBOL);
	}
	
	// Show Verifying process whether is PASS of FAILED
	gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_VERIFY);
	Print(L"Verifying EC ... ");
	if(EcRomChecksum != VerifyChecksum) {
		Status = EFI_DEVICE_ERROR;
	} else {
		Status = EFI_SUCCESS;
	}
	CompalTestResult(Status, NULL);
	
	return	Status;
}

EFI_STATUS
EFIAPI
CompalFinish_IO373X (
  )
{
	EFI_STATUS		Status;
	UINT16			AvalAddr;
	UINT16			Address;
	UINT8			Data8;
	
	// Get Avaliable Buffer, Buffer[0] or Buffer[1]
	Status = CompalGetAvaliableBuffer(&AvalAddr);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "There is no avaliabe buffer\n."));
		return	Status;
	}
		
	// Make sure buffer 0 is free
	Status = CompalCheckRegVal(0x80F6, 0x02, 0x02);  // Check buffer 0
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Time out for checking buffer 0.\n"));
		return Status;
	}

	// Make sure buffer 1 is free
	Status = CompalCheckRegVal(0x81F6, 0x02, 0x02);  // Check buffer 1
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Time out for checking buffer 1.\n"));
		return Status;
	}
	
	//Clear pending flag
	Data8 = 0x03;
	Status = CompalECRegByteRW_IO373X(REG373_SMBDPF, &Data8, EcRegWrite);

	// Send 0x80 cmd to announce EC all flash process has been done
	Data8 = 0x80;  // flash cmd
	Address = AvalAddr | 0x80F7;
	Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);

	// Announce EC to start to run this command, Clear 8xF6 = 0x00
	Address = AvalAddr | 0x80F6;
	Data8 = 0x00;
	Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);
	
	return Status;
}

EFI_STATUS
EFIAPI
CompalFlash_IO373X (
  IN  UINT8					*FileBuffer
  )
{
	EFI_STATUS		Status;
	UINT8 			Data8;
	UINT16			i;
	UINT16			EcChipID;
	UINT32			FileCheckSum=0;
	
	// Calculate ROM file Chechsum
	for(i = 0; i < EC_32KB; i++) {
		FileCheckSum += (UINT32)FileBuffer[i];
	}
	gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_CHECKSUM);
	Print(L"EC ROM Checksum : 0x%X", FileCheckSum);
	
	// Display waning message
	gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_WARNING);
	gST->ConOut->SetAttribute(gST->ConOut, EFI_RED|EFI_BACKGROUND_BLACK);
	Print(L"!!! PLEASE DO NOT RESTART OR TURN OFF THIS SYSTEM !!!");
	gST->ConOut->SetAttribute (gST->ConOut, EFI_LIGHTGRAY);
	
	// 1. Check chip ID
	Status = CompalGetChipID_IO373X(&EcChipID);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Check Chip ID process was failed.\n"));
		return Status;
	}
	
	// 2. Disable WatchDog Timer to prevent from EC reset
	Status = CompalDisableWatchDog_IO373X();
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"WDT disable process was failed.\n"));
		return Status;
	}
	
	// 3. Stop 8051 (0XF010 = 0X01)
	Data8 = 0x01;
	Status = CompalECRegByteRW_IO373X(REG373_E51_RST, &Data8, EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to Stop 8051\n"));
		return Status;
	}
	
	// 4. Stop Watch Dog
	Status = CompalDisableWatchDog_IO373X();
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"WDT disable process was failed.\n"));
		return Status;
	}

	// 5. Host update request
	Data8 = 0;
	Status = CompalECRegByteRW_IO373X(REG373_HOST_HS, &Data8, EcRegRead);
	Data8 |= 0x01;
	Status = CompalECRegByteRW_IO373X(REG373_HOST_HS, &Data8, EcRegWrite);
	
	// 6. Switch 8051 to Mask ROM
	Data8 = 0x00;
	Status = CompalECRegByteRW_IO373X(REG373_CODE_SEL, &Data8, EcRegWrite);
		
	// 7. Restart 8051 (Running on Mask ROM)
	Data8 = 0x00;
	Status = CompalECRegByteRW_IO373X(REG373_E51_RST, &Data8, EcRegWrite);
	
	// 8. Check whether Switch to Mask ROM
	Status = CompalCheckRegVal(0xF012, 0x02, 0x02);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Time out for checking Switch to Mask ROM\n."));
		return Status;
	}
	
	// 9. Erase ROM, Need to Erase 32k bytes. 128 bytes one time.
	Status = CompalErase_IO373X();
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Erase process was failed\n."));
		return Status;
	}
	
	// 10. Program EC
	
	Status = CompalProgram_IO373X(FileBuffer);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Program process was failed\n."));
		return Status;
	}

	// 11. Verify
	Status = CompalVerify_IO373X(FileCheckSum);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Varify process was failed\n."));
		return Status;
	}

	// 12. Finish 
	Status = CompalFinish_IO373X();
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Finish process was failed\n."));
		return Status;
	}
	
	return Status;
}

EFI_STATUS
EFIAPI
CompalUefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
	EFI_STATUS				Status;
	LIST_ENTRY              *List = NULL;
	EFI_SHELL_ENVIRONMENT2  *ShellEnv2;
	CONST CHAR16			*FileName;
	EFI_FILE_HANDLE         FileHandle;
	EFI_FILE_INFO  	 	    *FileInfo;
	UINTN					DataSize = 0;
	UINT8					*EcRomBuffer;
	UINT8					DataBuffer8[32];
	UINT8					i=0;

	// Disable cursor 
	gST->ConOut->EnableCursor (gST->ConOut, FALSE);
			
	// Clean screen
	gST->ConOut->ClearScreen(gST->ConOut);
	
	// Show title
    ShowTitle( 
		UTILITY_NAME,
        UTILITY_VERSION,
        UTILITY_AUTHOR,
        UTILITY_VENDOR,
        UTILITY_COPYRIGHT,
        UTILITY_LAST_MODIFY,
        UTILITY_LAST_MODIFIER 
		);
		
	Status = gBS->LocateProtocol (&gEfiShellEnvironment2Guid, NULL, (VOID**) &ShellEnv2);
	if (EFI_ERROR(Status)) {
		Print (L"Can not locate Shell Env2 Protocol\n");
		return Status;
	}
  
	Status = ShellCommandLineParse(ParamList, &List, NULL, FALSE);
	
						
	if (List == NULL) 
	{
		CompalHelpInfo();
		return EFI_INVALID_PARAMETER;
	}
	
	else if(ShellCommandLineGetFlag(List, L"-f")&&(ShellCommandLineGetCount(List) == 2)) 
	{
		// Get EC bin file path
		FileName = ShellCommandLineGetRawValue(List, 1);

		// Open file
		Status = ShellOpenFileByName((CHAR16 *)FileName, 
                               &FileHandle, 
                               EFI_FILE_MODE_READ, 
                               0
                               );
		
		// Get file information
		FileInfo = ShellGetFileInfo (FileHandle);
		if (FileInfo == NULL) {
			Print (L"Can not get file info for input file\n");
			return EFI_NOT_FOUND;
		} else {
			DEBUG((EFI_D_ERROR, "File Size = 0x%X\n", FileInfo->FileSize));
			DataSize = (UINTN)FileInfo->FileSize;
		}
		//Print(L"File Size = 0x%X\n", FileInfo->FileSize);
		
		DataSize = 0x8000;//32K
		// Allocate memory size for check buffer 
		Status = gBS->AllocatePool (
					EfiBootServicesData,
					DataSize,
					(VOID **) &EcRomBuffer
					);
		ZeroMem (EcRomBuffer, DataSize);
	
		// Read EC bin file to EcRomBuffer
		Status = ShellReadFile (FileHandle, &DataSize, (VOID *)EcRomBuffer);
		if (EFI_ERROR (Status)) {
			DEBUG((EFI_D_ERROR,"ERROR: ReadFile from SOURCE device failed\n"));
			return Status;
		}
		
		// Show Current EC version
		Status = CompalGetEcFwVersion_IO373X(DataBuffer8);
		if(!EFI_ERROR (Status)) {
			gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_CURRENT_VER);
			if(DataBuffer8[2] == 0x00) {
				// Formal Release EC Version
				Print(L"Current EC Version : v%d.%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[3]);
			} else {
				// Test EC Version
				Print(L"Current EC Version : v%d.%02xT%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[2], DataBuffer8[3]);
			}
		} else {
			Print(L"Can not get EC information\n");
			return Status;
		}
		
		// Show Updated EC veriosn
		CompalGet_Updated_Version_IO373X(EcRomBuffer, (UINT32*)DataBuffer8);
		gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_UPDATED_VER);
		if(DataBuffer8[3] == 0x00) {
			// Formal Release EC Version
			Print(L"Updated EC Version : v%x.%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[2]);
		} else {
			// Test EC Version
			Print(L"Updated EC Version : v%x.%02xT%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[3], DataBuffer8[2]);
		}
			
		// Flashe EC 
		Status = CompalFlash_IO373X(EcRomBuffer);
		
		if(EFI_ERROR (Status)) {
			gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_FINAL_INFO);
			gST->ConOut->SetAttribute(gST->ConOut, EFI_RED|EFI_BACKGROUND_BLACK);
			Print(L"It was failed to update EC firmware !!");
		} else {
			gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_FINAL_INFO);
			gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN|EFI_BACKGROUND_BLACK);
			Print(L"It was successful to update EC firmware, please restart this system !!");
		}
    
      Status = gBS->FreePool(EcRomBuffer);
      if ( EFI_ERROR ( Status ) ) {
    	Print( L"Free EC ROM buffer failed." );
	  }
	} // else if
  
  	else if(ShellCommandLineGetFlag(List, L"-n")&&(ShellCommandLineGetCount(List) == 2)) 
	{
		// Get EC bin file path
		FileName = ShellCommandLineGetRawValue(List, 1);

		// Open file
		Status = ShellOpenFileByName((CHAR16 *)FileName, 
                               &FileHandle, 
                               EFI_FILE_MODE_READ, 
                               0
                               );
		
		// Get file information
		FileInfo = ShellGetFileInfo (FileHandle);
		if (FileInfo == NULL) {
			Print (L"Can not get file info for input file\n");
			return EFI_NOT_FOUND;
		} else {
			DEBUG((EFI_D_ERROR, "File Size = 0x%X\n", FileInfo->FileSize));
			DataSize = (UINTN)FileInfo->FileSize;
		}
		//Print(L"File Size = 0x%X\n", FileInfo->FileSize);
		
		DataSize = 0x8000;//32K
		// Allocate memory size for check buffer 
		Status = gBS->AllocatePool (
					EfiBootServicesData,
					DataSize,
					(VOID **) &EcRomBuffer
					);
		ZeroMem (EcRomBuffer, DataSize);
	
		// Read EC bin file to EcRomBuffer
		Status = ShellReadFile (FileHandle, &DataSize, (VOID *)EcRomBuffer);
		if (EFI_ERROR (Status)) {
			DEBUG((EFI_D_ERROR,"ERROR: ReadFile from SOURCE device failed\n"));
			return Status;
		}
		
/*
		// Show Current EC version
		Status = CompalGetEcFwVersion_IO373X(DataBuffer8);
		if(!EFI_ERROR (Status)) {
			gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_CURRENT_VER);
			if(DataBuffer8[2] == 0x00) {
				// Formal Release EC Version
				Print(L"Current EC Version : v%d.%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[3]);
			} else {
				// Test EC Version
				Print(L"Current EC Version : v%d.%02dT%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[2], DataBuffer8[3]);
			}
		} else {
			Print(L"Can not get EC information\n");
			return Status;
		}

*/
		// Show Updated EC veriosn
		CompalGet_Updated_Version_IO373X(EcRomBuffer, (UINT32*)DataBuffer8);
		gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_UPDATED_VER);
		if(DataBuffer8[3] == 0x00) {
			// Formal Release EC Version
			Print(L"Updated EC Version : v%x.%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[2]);
		} else {
			// Test EC Version
			Print(L"Updated EC Version : v%x.%02xT%02x%c", DataBuffer8[0], DataBuffer8[1], DataBuffer8[3], DataBuffer8[2]);
		}
			
		// Flashe EC 
		Status = CompalFlash_IO373X(EcRomBuffer);
		
		if(EFI_ERROR (Status)) {
			gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_FINAL_INFO);
			gST->ConOut->SetAttribute(gST->ConOut, EFI_RED|EFI_BACKGROUND_BLACK);
			Print(L"It was failed to update EC firmware !!");
		} else {
			gST->ConOut->SetCursorPosition(gST->ConOut, 0, SHOW_FINAL_INFO);
			gST->ConOut->SetAttribute(gST->ConOut, EFI_GREEN|EFI_BACKGROUND_BLACK);
			Print(L"It was successful to update EC firmware, please restart this system !!");
		}
    
      Status = gBS->FreePool(EcRomBuffer);
      if ( EFI_ERROR ( Status ) ) {
    	Print( L"Free EC ROM buffer failed." );
	  }
	} // else if
	
	else if(ShellCommandLineGetFlag(List, L"-v")) 
	{
		//Get Platfrom ID
		Status = CompalGetPlatformID_IO373X(DataBuffer8);
		if(!EFI_ERROR (Status)) {
			Print(L"\nPlatform ID : ");
			for(i=0;i<6;i++) {
				Print(L"%c", DataBuffer8[i]);
			}
			Print(L"\n");
		} else {
			Print(L"\nCan not get Platform ID\n");
		}		
		
		// Get Project ID
		Status = CompalGetProjectID_IO373X(DataBuffer8);
		if(!EFI_ERROR (Status)) {
			Print(L"\nProject ID : %02x\n", DataBuffer8[1]);
		} else {
			Print(L"\nCan not get Project ID\n");
		}
		
		//Get onboard EC version		
		Status = CompalGetEcFwVersion_IO373X(DataBuffer8);
		
		if(!EFI_ERROR (Status)) {
			if(DataBuffer8[2] == 0x00) {
				Print(L"\nCurrent EC Version : v%x.%02x%c .\n", DataBuffer8[0], DataBuffer8[1], DataBuffer8[3]);
			} else {
				Print(L"\nCurrent EC Version : v%x.%02xT%02x%c\n", DataBuffer8[0], DataBuffer8[1], DataBuffer8[2], DataBuffer8[3]);
			}
		} else {
			Print(L"\nCan not get EC information\n");
		}

	} 
	
	else if(ShellCommandLineGetFlag(List, L"-i")) 
	{


		// Get EC Chip ID
		Status = CompalGetChipID_IO373X((UINT16*)DataBuffer8);
		if(!EFI_ERROR (Status)) {
			Print(L"\nEC Chip ID : 0x%02X%02X\n", DataBuffer8[1], DataBuffer8[0]);
		} else {
			Print(L"\nCan not get EC information\n");
		}
		
	}
	else if(ShellCommandLineGetFlag(List, L"-t")) //test only
	{


		Status = CompalGetPlatformID_IO373X(DataBuffer8);
		
		if(!EFI_ERROR (Status)) {
			Print(L"\nPlatform ID : ");
			for(i=0;i<6;i++) {
				Print(L"%c", DataBuffer8[i]);
			}
			Print(L"\n");
		} else {
			Print(L"\nCan not get Platform ID\n");
		}		
			
		Status = CompalECAcpiMode(1);	

		Print(L"\nACPI EFI_ERROR( %x )\n" , Status);
		Print(L"\nEcRegWrite:%x  EcRegRead:%x\n" , EcRegWrite,EcRegRead);

		
			// Get EC Chip ID
		Status = CompalGetChipID_IO373X((UINT16*)DataBuffer8);
		if(!EFI_ERROR (Status)) {
			Print(L"\nEC Chip ID : 0x%02X%02X\n", DataBuffer8[1], DataBuffer8[0]);
		} else {
			Print(L"\nCan not get EC information\n");
		}	
		
		Status = CompalGetEcFwVersion_IO373X(DataBuffer8);
		
		if(!EFI_ERROR (Status)) {
			if(DataBuffer8[2] == 0x00) {
				Print(L"\nCurrent EC Version : v%x.%02x%c .\n", DataBuffer8[0], DataBuffer8[1], DataBuffer8[3]);
			} else {
				Print(L"\nCurrent EC Version : v%x.%02xT%02x%c\n", DataBuffer8[0], DataBuffer8[1], DataBuffer8[2], DataBuffer8[3]);
			}
		} else {
			Print(L"\nCan not get EC information\n");
		}
	
	}
	else 
	{
		CompalHelpInfo();
	}
	
	// Enable cursor 
	gST->ConOut->EnableCursor (gST->ConOut, TRUE);
	Print(L"\n");
	
	return EFI_SUCCESS;
}
