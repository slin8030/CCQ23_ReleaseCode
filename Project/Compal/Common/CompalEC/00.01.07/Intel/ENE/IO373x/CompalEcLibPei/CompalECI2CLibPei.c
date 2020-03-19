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

#include "CompalECI2CDelayPei.h"
#include "CompalECI2CIoLibPei.h"
#include "CompalECI2CAccessPei.h"
#include "CompalECI2CLibPei.h"
#include <PlatformBaseAddresses.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/HobLib.h>
#include <pchregs/PchRegsPcu.h> 
#include <pchregs/PchRegsLpss.h> 

/*
#define ULPMC_I2C_BUSNO 		0x0
#define ULPMC_FWmode_I2C_ADDR 	0x70	//bit8 = 0xE0
enum
{
    EcRegWrite,
    EcRegRead
};
*/
#define LPSS_PCI_DEVICE_NUMBER  8

#define R_PCH_LPIO_I2C_MEM_RESETS                 0x804 // Software Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_FUNC            BIT1  // Function Clock Domain Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_APB             BIT0  // APB Domain Reset
#define R_PCH_LPSS_I2C_MEM_PCP                    0x800 // Private Clock Parameters
UINT16 CompalI2CGPIO[]= {
 /*
  19.1.6  I2C0
  I2C0_SDA-OD-O -    write 0x2003CC81 to IOBASE + 0x0210
  I2C0_SCL-OD-O -    write 0x2003CC81 to IOBASE + 0x0200
  */
   0x0210,
   0x0200,

  /*
  19.1.7  I2C1
  I2C1_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01F0
  I2C1_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01E0
  */
   0x01F0,
   0x01E0,

  /*
  19.1.8  I2C2
  I2C2_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01D0
  I2C2_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01B0
  */
   0x01D0, 
   0x01B0,

  /*
  19.1.9  I2C3
  I2C3_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0190
  I2C3_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x01C0
  */
   0x0190,
   0x01C0,

  /*
  19.1.10 I2C4
  I2C4_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x01A0
  I2C4_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x0170
  */
   0x01A0,
   0x0170,

  /*
  19.1.11 I2C5
  I2C5_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0150
  I2C5_SCL-OD-O/I - write 0x2003CC81 to IOBASE + 0x0140
  */
   0x0150,
   0x0140,

  /*
  19.1.12 I2C6
  I2C6_SDA-OD-O/I - write 0x2003CC81 to IOBASE + 0x0180
  I2C6_SCL-OD-O/I -  write 0x2003CC81 to IOBASE + 0x0160
  */
   0x0180,
   0x0160
   };
  
EFI_STATUS
EFIAPI
CompalI2CPeiLibConstructor (
  VOID
  )
{
  UINTN Index;
 for (Index = 0; Index <   sizeof(CompalI2CGPIO)/sizeof(UINT16); Index ++){
    CompalI2CLibPeiMmioWrite32(IO_BASE_ADDRESS+CompalI2CGPIO[Index], 0x2003CC81);
 }


  return EFI_SUCCESS;
}


EFI_STATUS
CompalProgramPciLpssI2C (
 VOID
 )
{
  UINT32       PmcBase;
  UINT32       DevID;
  UINTN        PciMmBase=0;
  UINTN        Index;
  UINTN        Bar0;
  UINTN        Bar1;  
  DEBUG ((EFI_D_INFO, "Pei CompalProgramPciLpssI2C() Start\n"));  
  //
  // Set the VLV Function Disable Register to ZERO 
  //
  PmcBase         = CompalI2CLibPeiMmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  if(CompalI2CLibPeiMmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)&(B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5
      |B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7))
    {
      CompalI2CLibPeiMmioWrite32(
      PmcBase+R_PCH_PMC_FUNC_DIS,
      CompalI2CLibPeiMmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)& \
        ~(B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2 \
         |B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4 \
         |B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6|B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7)
         );
       DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C() enable all I2C controllers\n"));  
  	}

  for(Index = 0; Index < LPSS_PCI_DEVICE_NUMBER; Index ++) {
  
    PciMmBase = MmPciAddress (
                      0, 
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                      Index, 
                      0
                    );
    DevID =  CompalI2CLibPeiMmioRead32(PciMmBase);

    Bar0 = 0xFE900000+ Index*0x10000;
    Bar1 = Bar0+0x8000;

    //DEBUG((EFI_D_ERROR, "Program Pci Lpss I2C Device  Function=%x DevID=%08x\n", Index, DevID));    
    ///
    /// Check if device present
    ///
    if (DevID  != 0xFFFFFFFF)  {
      if(!(CompalI2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_STSCMD)& B_PCH_LPSS_I2C_STSCMD_MSE)){
        ///
        /// Program BAR 0
        ///
        CompalI2CLibPeiMmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (Bar0 & B_PCH_LPSS_I2C_BAR_BA));
        //DEBUG ((EFI_D_ERROR, "mI2CBaseAddress1 = 0x%x \n",CompalI2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR)));  
        ///
        /// Program BAR 1
        ///
        CompalI2CLibPeiMmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR1), (UINT32) (Bar1 & B_PCH_LPSS_I2C_BAR1_BA));
        //DEBUG ((EFI_D_ERROR, "mI2CBaseAddress1 = 0x%x \n",CompalI2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR1)));  
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        CompalI2CLibPeiMmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        //ASSERT (CompalI2CLibPeiMmioRead32 (Bar0) != 0xFFFFFFFF);
      }
      ///
      /// Release Resets
      ///
      CompalI2CLibPeiMmioWrite32 (Bar0 + R_PCH_LPIO_I2C_MEM_RESETS,(B_PCH_LPIO_I2C_MEM_RESETS_FUNC | B_PCH_LPIO_I2C_MEM_RESETS_APB));     
      //
      // Activate Clocks
      //
      CompalI2CLibPeiMmioWrite32 (Bar0 + R_PCH_LPSS_I2C_MEM_PCP,0x80020003);//No use for A0

      //DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C() Programmed()\n"));      
    }
      
  }    
  DEBUG ((EFI_D_INFO, "Pei CompalProgramPciLpssI2C() End\n"));
    
  return EFI_SUCCESS;
}

EFI_STATUS
CompalI2cDisable (
  IN UINT8 BusNo
  )
{
  UINTN   mI2CBaseAddress;
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  mI2CBaseAddress = (UINT32) 0xFE910000+ BusNo *0x10000;
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_ENABLE, 0 );
  while ( 0 != ( CompalI2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_ENABLE_STATUS ) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return EFI_NOT_READY;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
CompalI2cEnable (
  IN UINT8 BusNo
  )
{
  UINTN   mI2CBaseAddress;
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  mI2CBaseAddress = (UINT32) 0xFE910000+ BusNo *0x10000;
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_ENABLE, 1 );
  while ( 0 == ( CompalI2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_ENABLE_STATUS ) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return EFI_NOT_READY;
  }
  return EFI_SUCCESS;
}


/**
  Set the I2C controller bus clock frequency.

  @param[in] This           Address of the library's I2C context structure
  @param[in] PlatformData   Address of the platform configuration data
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval RETURN_SUCCESS      The bus frequency was set successfully.
  @retval RETURN_UNSUPPORTED  The controller does not support this frequency.

**/
EFI_STATUS
CompalI2cBusFrequencySet (
  IN UINTN   mI2CBaseAddress,
  IN UINTN   BusClockHertz,
  IN UINT16  *I2cMode
  )
{
  DEBUG((EFI_D_INFO,"InputFreq BusClockHertz: %d\r\n",BusClockHertz));

 *I2cMode = B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE;
 
  //
  //  Set the 100 KHz clock divider
  //
  //  From Table 10 of the I2C specification
  //
  //    High: 4.00 uS
  //    Low:  4.70 uS
  //
  //DEBUG((EFI_D_INFO, "100khz SS_SCL_CNT High:%d Low:%d\r\n", High, Low));
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_SS_SCL_HCNT, (UINT16)0x214 );
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_SS_SCL_LCNT, (UINT16)0x272 );  
  //
  //	Set the 400 KHz clock divider
  //
  //	From Table 10 of the I2C specification
  //
  //	  High: 0.60 uS
  //	  Low:	1.30 uS
  //
  //DEBUG((EFI_D_INFO, "400khz SS_SCL_CNT High:%d Low:%d\r\n", High, Low));  
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_FS_SCL_HCNT, (UINT16)0x50 );
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_FS_SCL_LCNT, (UINT16)0xAD );
  
  switch ( BusClockHertz ) {
    case 100 * 1000:
      CompalI2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x40);//100K     
      *I2cMode |= V_SPEED_STANDARD;
      break;
    case 400 * 1000:
      CompalI2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x32);//400K     
      *I2cMode |= V_SPEED_FAST;
      break;
    default:
      CompalI2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x09);//3.4M
      *I2cMode |= V_SPEED_HIGH;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
CompalI2CInit (UINT8 BusNo, UINT16 SlaveAddress)
{
  EFI_STATUS Status;
  UINT32		NumTries = 0;
  UINTN          mI2CBaseAddress;
  UINT16        I2cMode;
  UINTN         PciMmBase=0;


  PciMmBase = MmPciAddress (
                      0, 
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
                      (BusNo + 1), 
                      0
                    );

  mI2CBaseAddress = CompalI2CLibPeiMmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR);

  //
  //  Verify the parameters
  //
  if ( 1023 < SlaveAddress ) {
    Status =  EFI_INVALID_PARAMETER;
    DEBUG((EFI_D_INFO,"I2cStartRequest Exit with Status %r\r\n", Status));
    return Status;
  }

  if(mI2CBaseAddress !=  (0xFE910000 + BusNo *0x10000)){
    CompalProgramPciLpssI2C();
    //**
    mI2CBaseAddress = (UINT32) (0xFE910000 + BusNo *0x10000); 
    DEBUG ((EFI_D_ERROR, "mI2CBaseAddress = 0x%x \n",mI2CBaseAddress));  
    NumTries = 10000; /* 1 seconds */
    while (( 1 == ( CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_STATUS) & STAT_MST_ACTIVITY ))) {
    MicroSecondDelay(10);
    NumTries --;
    if(0 == NumTries) 
      return EFI_DEVICE_ERROR;
    } 

    Status = CompalI2cDisable ( BusNo);
    DEBUG((EFI_D_INFO, "CompalI2cDisable Status = %r\r\n", Status));
    //**
  }  
  
  mI2CBaseAddress = (UINT32) (0xFE910000 + BusNo *0x10000);  

//**
//  DEBUG ((EFI_D_ERROR, "mI2CBaseAddress = 0x%x \n",mI2CBaseAddress));  
//  NumTries = 10000; /* 1 seconds */
//  while (( 1 == ( CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_STATUS) & STAT_MST_ACTIVITY ))) {
//    MicroSecondDelay(10);
//    NumTries --;
//    if(0 == NumTries) 
//      return EFI_DEVICE_ERROR;
//  }

//  Status = CompalI2cDisable ( BusNo);
//  DEBUG((EFI_D_INFO, "CompalI2cDisable Status = %r\r\n", Status));
//**
  CompalI2cBusFrequencySet(mI2CBaseAddress, 400 * 1000, &I2cMode);//Set I2cMode
  
  CompalI2CLibPeiMmioWrite16( mI2CBaseAddress + R_IC_INTR_MASK, 0x0);
  if ( 0x7F < SlaveAddress ) {
    SlaveAddress = ( SlaveAddress & 0x3ff ) | IC_TAR_10BITADDR_MASTER;
  }
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_TAR, (UINT16) SlaveAddress );
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_RX_TL, 0);
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_TX_TL, 0 );  
  CompalI2CLibPeiMmioWrite16 ( mI2CBaseAddress + R_IC_CON, I2cMode);
  
  Status = CompalI2cEnable(BusNo);
  CompalI2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
  return EFI_SUCCESS;  
}

EFI_STATUS 
CompalByteReadI2CPei_Serial(
  IN  UINT8 BusNo,
  IN  UINT8 SlaveAddress, 
  IN  UINTN ReadBytes,
  OUT UINT8 *ReadBuffer
)
/*++

Routine Description:
  
  Reads Bytes from I2C Device
    
               
Input:

  BusNo             I2C Bus no to which the I2C device has been connected        
  SlaveAddress      Device Address from which the byte value has to be read
  ReadBytes         Count of which the data has to be read
  *ReadBuffer       Address to which the value read has to be stored

Return:

  EFI_SUCCESS       IF the byte value has been successfully read
  EFI_DEVICE_ERROR  Operation Failed, Device Error
    
--*/
{

  EFI_STATUS Status;
  UINT32 I2cStatus;
  UINT16 ReceiveData;
  UINT8 *ReceiveDataEnd;
  UINT8 *ReceiveRequest;
  UINT16 raw_intr_stat;
  UINT32 Count=0;
  UINT8  i=1;
  UINTN   mI2CBaseAddress;

  mI2CBaseAddress = (UINT32)  (0xFE910000+ BusNo *0x10000);  

  Status = EFI_SUCCESS;
  

  ReceiveDataEnd = &ReadBuffer [ ReadBytes ];  
  if( ReadBytes ) {
    ReceiveRequest = ReadBuffer;

    while ( (ReceiveDataEnd > ReceiveRequest) || 
              (ReceiveDataEnd > ReadBuffer)) {
      // Check for NACK
      raw_intr_stat = (UINT16)CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        break;
      }

      //	Determine if another byte was received
      I2cStatus = CompalI2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_STATUS );
      if ( 0 != ( I2cStatus & STAT_RFNE )) {
        ReceiveData = CompalI2CLibPeiMmioRead16 ( mI2CBaseAddress + R_IC_DATA_CMD );
        *ReadBuffer++ = (UINT8)ReceiveData;
      }

      if(ReceiveDataEnd==ReceiveRequest){ 
		        MicroSecondDelay ( FIFO_WRITE_DELAY );
                Count++;
                if(Count<1024)//to avoid sys hung  without ul-pmc device  on RVP
                    continue;//Waiting the last request to get data and make (ReceiveDataEnd > ReadBuffer) =TRUE.
                else
                    break;     
      }
      //Wait until a read request will fit
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
      MicroSecondDelay ( 10 );
      continue;
      }
      // Issue the next read request
      CompalI2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_TAR, (UINT16) SlaveAddress );


      if (ReadBytes == 1)
	   CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART|B_CMD_STOP);				   
	  else if (i == 1)		   
		   CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART); 
	  else if (i == ReadBytes )
		   CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_STOP);
	  else
		   CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD);
   
      MicroSecondDelay ( FIFO_WRITE_DELAY );//wait after send cmd

      ReceiveRequest += 1;
	  i += 1;
      }
      
  }
  return Status;

}

EFI_STATUS 
CompalByteWriteI2CPei_Serial(
    IN  UINT8 BusNo, 
    IN  UINT8 SlaveAddress,
    IN  UINTN WriteBytes,
    IN  UINT8 *WriteBuffer,
    IN  UINT8 Mode    
)
/*++

Routine Description:
  
  Writes Bytes to I2C Device
    
               
Input:

  BusNo             I2C Bus no to which the I2C device has been connected        
  SlaveAddress      Device Address from which the byte value has to be read
  WriteBytes        Count from which the data has to be Write
  *WriteBuffer      Address to which the byte value has to be written
  Mode              Read Write I2C select.

Return:

  EFI_SUCCESS       IF the byte value written successfully
  EFI_DEVICE_ERROR  Operation Failed, Device Error
    
--*/
{

  EFI_STATUS Status;
  UINT32 I2cStatus;
  UINT8 *TransmitEnd;
  UINT16 raw_intr_stat;
  UINT8	 i=1;
  UINTN   mI2CBaseAddress;

  mI2CBaseAddress = (UINT32)  0xFE910000+ BusNo *0x10000;  

  Status = EFI_SUCCESS;
  
  // Initialise I2C Device
  // Status = I2CInitCOMPAL(BusNo, (UINT16)SlaveAddress);

  TransmitEnd = &WriteBuffer [ WriteBytes ];  
  if( WriteBytes ) {
    
    while ( TransmitEnd > WriteBuffer ) {
       // **Initialise I2C Device**
	  Status = CompalI2CInit(BusNo, (UINT16)SlaveAddress);
      
      I2cStatus = CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_STATUS );
      raw_intr_stat = (UINT16)CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        CompalI2CLibPeiMmioRead32 ( mI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_ERROR,"TX ABRT TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
        break;
      }
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
        continue;
      }
      CompalI2CLibPeiMmioWrite32 ( mI2CBaseAddress + R_IC_TAR, (UINT16) SlaveAddress );
		
	  if(!Mode)//Write = 0
	  {
	  
		if (WriteBytes == 1)
			CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART|B_CMD_STOP);				   
		else if (i == 1)		   
			CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART); 
		else if (i == WriteBytes )
			CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_STOP);
		else
			CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++));

	  }

	   if(Mode)//Read=1
	  {  
		if (i == 1)
			CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART);
	  	else 
			CompalI2CLibPeiMmioWrite32 (mI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++));  
	  }
      // Add a small delay to work around some odd behavior being seen.  Without this delay bytes get dropped.
      MicroSecondDelay ( FIFO_WRITE_DELAY );//wait after send cmd
      
	  i += 1;
    }
  }

  return Status;
}


