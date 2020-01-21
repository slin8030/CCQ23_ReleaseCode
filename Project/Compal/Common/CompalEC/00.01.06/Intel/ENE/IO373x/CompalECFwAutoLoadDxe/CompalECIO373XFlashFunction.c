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


#include "CompalECIO373XFlashFunction.h"
//[PRJ] Start
#include <Library/CmosLib.h>
#include <OemCmos.h>
#include <PchRegs.h>
#include <ChipsetCmos.h>

#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)

#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
//[PRJ] End

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
		return EC_FLASH_ERROR_CODE_CHIPID;
	}
	
	// Clear WDT pending flags which are the reset records
	Data8 = 0x03;
	Status = CompalECRegByteRW_IO373X(REG373_WDTPF, &Data8, EcRegWrite);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Unable to clear WDT pending flags\n"));
		return EC_FLASH_ERROR_CODE_CHIPID;
	}
	
	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CompalGetAvaliableBuffer (
  OUT UINT16  				*AddrOffset
  )
{
  EFI_STATUS Status = EC_FLASH_ERROR_CODE_GET_AVALIABLE_BUFFER;
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
    return EC_FLASH_ERROR_CODE_CHECK_REG_VAL; 
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
  EFI_STATUS Status = EC_FLASH_ERROR_CODE_WAIT_BUFFER_FREE;
  

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


EFI_STATUS
EFIAPI
Compare_EC_Version(
  IN UINT8*        FileBuffer
)
{
    EFI_STATUS  Status;
    UINT8       OnBoard_EC_Version[8];
    UINT8       BinFile_EC_Version[8];
    
    CompalGet_Updated_Version_IO373X(FileBuffer, (UINT32*)BinFile_EC_Version);
    Status = CompalGetEcFwVersion_IO373X(OnBoard_EC_Version);

    Status = EFI_NOT_FOUND;
    if ( OnBoard_EC_Version[0] == BinFile_EC_Version[0]) {			//Compare Major Version
        if ( OnBoard_EC_Version[1] == BinFile_EC_Version[1]) {		//Compare Minor Version
            if ( OnBoard_EC_Version[2] == BinFile_EC_Version[3]) {	//Compare Test Version
              if ( OnBoard_EC_Version[3] == BinFile_EC_Version[2]) {	//Compare Re-release Version
                  Status = EFI_SUCCESS;
                  }
                }
        }   
	}

	return Status;
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
	//UINTN		Complete_Percent;
	
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
		
	}
	
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
	//UINTN		Complete_Percent;
    

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
			CompalSetAddr_IO373X(Address);
			DataLength = EC_32B+2;
			Buffer8[0] = WRITE_BLOCK_CMD;   // Block write command
			Buffer8[1] = EC_32B;            // Byte Count
			CopyMem(&Buffer8[2], (FileBuffer + i + j), EC_32B);
			Status = CompalByteWriteI2C_Serial(ULPMC_I2C_BUSNO, ULPMC_HWmode_I2C_ADDR,DataLength,Buffer8,EcRegWrite);
			Address += EC_32B;
            gBS->Stall(1000);               // delay 1ms
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
		
	}

	// Announce EC to start to run this command, Clear 8xF6 = 0x00
	Address = AvalAddr | 0x80F6;
	Data8 = 0x00;
	Status = CompalECRegByteRW_IO373X(Address, &Data8, EcRegWrite);

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
		return EC_FLASH_ERROR_CODE_NO_FREE_BUFFER;
	}

	// Make sure buffer 1 is free
	Status = CompalCheckRegVal(0x81F6, 0x02, 0x02);  // Check buffer 1
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Time out for checking buffer 1.\n"));
		return EC_FLASH_ERROR_CODE_NO_FREE_BUFFER;
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
	UINT16			EcChipID;
	
	
	// 1. Check chip ID
	Status = CompalGetChipID_IO373X(&EcChipID);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR,"Check Chip ID process was failed.\n"));
		return EC_FLASH_ERROR_CODE_CHIPID;
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
		return EC_FLASH_ERROR_CODE_SWITCH_MASKROM;
	}
	
	// 9. Erase ROM, Need to Erase 32k bytes. 128 bytes one time.
	Status = CompalErase_IO373X();
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Erase process was failed\n."));
		return EC_FLASH_ERROR_CODE_ERASE_ROM;
	}
	
	// 10. Program EC
	
	Status = CompalProgram_IO373X(FileBuffer);
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Program process was failed\n."));
		return EC_FLASH_ERROR_CODE_PROGRAM_EC;
	}

	// 11. Verify
	//Status = CompalVerify_IO373X(FileCheckSum);
	//if(EFI_ERROR(Status)) {
	//	DEBUG((EFI_D_ERROR, "Varify process was failed\n."));
	//	return Status;
	//}

	// 12. Finish 
	Status = CompalFinish_IO373X();
	if(EFI_ERROR(Status)) {
		DEBUG((EFI_D_ERROR, "Finish process was failed\n."));
		return EC_FLASH_ERROR_CODE_FINISH_CMD;
	}
	
	return Status;
}


EFI_STATUS
CompalECIO373XFlashFunction(
    IN UINT8*        FileBuffer
)
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT8      RetryCount;
//[PRJ] Start
 UINTN      PmcBase;
//[PRJ] End

  

  Status = Compare_EC_Version(FileBuffer);
  if (Status == EFI_SUCCESS) {
  	
    return Status;
  }
 

  //EC update process, it will retry 3 times if update failed
    for (RetryCount = 0; RetryCount < 3; RetryCount++)
    {
 

        Status = CompalFlash_IO373X(FileBuffer);
        if ( Status == EFI_SUCCESS) {
			
		   
           gBS->Stall (400000);
           gBS->Stall (400000);
           
            PmcBase = MmioRead32 ( MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, 
                                                    PCI_DEVICE_NUMBER_PCH_LPC, 
                                                    PCI_FUNCTION_NUMBER_PCH_LPC, 
                                                    R_PCH_LPC_PMC_BASE)
                                                    ) & B_PCH_LPC_PMC_BASE_BAR;
     
            MmioOr32 (PmcBase + R_PCH_PMC_PMIR, (UINT32) (B_PCH_PMC_PMIR_CF9GR));
            IoWrite8 ((UINTN) R_PCH_RST_CNT, (UINT8) V_PCH_RST_CNT_HARDRESET);
            //
            // Waiting for system reset
            //
            CpuDeadLoop();
           	
        break;		

       }
//[PRJ] End
    }
  return Status;
}
