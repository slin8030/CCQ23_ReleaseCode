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

#include <CompalECI2CLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <pchregs/PchRegsPcu.h> 
#include <PchRegs.h>
#include <platformBaseAddresses.h>
#include <pchregs/PchRegsLpss.h> 

#include <Protocol/GlobalNvsArea.h>
#include <Library/UefiBootServicesTableLib.h>

#define GLOBAL_NVS_OFFSET(Field)    (UINTN)((CHAR8*)&((EFI_GLOBAL_NVS_AREA*)0)->Field - (CHAR8*)0)

#define PCIEX_BASE_ADDRESS  0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#define PciD31F0RegBase             PCIEX_BASE_ADDRESS + (UINT32) (31 << 15)


typedef struct _COMPAL_LPSS_PCI_DEVICE_INFO {
  UINTN        Segment;
  UINTN        BusNum;
  UINTN        DeviceNum;
  UINTN        FunctionNum;
  UINTN        Bar0;
  UINTN        Bar1;
}COMPAL_LPSS_PCI_DEVICE_INFO;

COMPAL_LPSS_PCI_DEVICE_INFO  mCompalLpssPciDeviceList[] = {  
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_DMAC1, PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC, 0xFE900000, 0xFE908000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0, 0xFE910000, 0xFE918000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1, 0xFE920000, 0xFE928000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2, 0xFE930000, 0xFE938000},    
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3, 0xFE940000, 0xFE948000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4, 0xFE950000, 0xFE958000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5, 0xFE960000, 0xFE968000},
  {0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPSS_I2C,   PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6, 0xFE970000, 0xFE978000}
};
#define LPSS_PCI_DEVICE_NUMBER  sizeof(mCompalLpssPciDeviceList)/sizeof(COMPAL_LPSS_PCI_DEVICE_INFO)
//
// LPIO I2C Module
// Memory Space Registers
//
#define R_PCH_LPIO_I2C_MEM_RESETS                 0x804 // Software Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_FUNC            BIT1  // Function Clock Domain Reset
#define B_PCH_LPIO_I2C_MEM_RESETS_APB             BIT0  // APB Domain Reset
#define R_PCH_LPSS_I2C_MEM_PCP                    0x800 // Private Clock Parameters

STATIC UINTN mCompalI2CBaseAddress = 0;
STATIC UINT16 mCompalI2CSlaveAddress = 0;

STATIC UINT16      CompalI2cMode=B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE ;

STATIC UINTN Compali2cNvsBaseAddress[] = {
  GLOBAL_NVS_OFFSET(LDMA2Addr),
  GLOBAL_NVS_OFFSET(I2C1Addr), 
  GLOBAL_NVS_OFFSET(I2C2Addr), 
  GLOBAL_NVS_OFFSET(I2C3Addr), 
  GLOBAL_NVS_OFFSET(I2C4Addr),
  GLOBAL_NVS_OFFSET(I2C5Addr),
  GLOBAL_NVS_OFFSET(I2C6Addr),
  GLOBAL_NVS_OFFSET(I2C7Addr)
};


//get I2Cx controller base address (bar0)

UINTN
CompalGetI2cBarAddr(
  IN	UINT8 BusNo
  )
{
  EFI_STATUS           Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;
  UINTN  AcpiBaseAddr;
  UINTN  PciMmBase=0;

  ASSERT(gBS!=NULL);
  
  Status = gBS->LocateProtocol (
                  &gEfiGlobalNvsAreaProtocolGuid,
                  NULL,
                  &GlobalNvsArea
                  );
  //
  //PCI mode from PEI ( Global NVS is not ready)
  //
  if (EFI_ERROR(Status)) {
    DEBUG ((EFI_D_INFO, "CompalGetI2cBarAddr() gEfiGlobalNvsAreaProtocolGuid:%r\n", Status));  
    return 0;//not ready
  }  

  AcpiBaseAddr =  *(UINTN*)((CHAR8*)GlobalNvsArea->Area + Compali2cNvsBaseAddress[BusNo+1]);
  //
  //PCI mode from DXE (global NVS protocal) to LPSS OnReadytoBoot(swith to ACPI)
  //
  if(AcpiBaseAddr==0){
      PciMmBase = MmPciAddress (
      					mCompalLpssPciDeviceList[BusNo+1].Segment,
      					mCompalLpssPciDeviceList[BusNo+1].BusNum,
      					mCompalLpssPciDeviceList[BusNo+1].DeviceNum,
      					mCompalLpssPciDeviceList[BusNo+1].FunctionNum,
      					0
      				  );
      DEBUG((EFI_D_ERROR, "\nCompalGetI2cBarAddr() I2C Device	%x %x %x PciMmBase:%x\n", \
      	       mCompalLpssPciDeviceList[BusNo+1].BusNum, \
      	       mCompalLpssPciDeviceList[BusNo+1].DeviceNum, \
      	       mCompalLpssPciDeviceList[BusNo+1].FunctionNum, PciMmBase));    

      if (MmioRead32 (PciMmBase) != 0xFFFFFFFF)	   {
        if((MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_STSCMD)& B_PCH_LPSS_I2C_STSCMD_MSE)){
         mCompalLpssPciDeviceList[BusNo+1].Bar0=MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR);	 //get the address allocted.
         mCompalLpssPciDeviceList[BusNo+1].Bar1=MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR1);
         DEBUG((EFI_D_ERROR, "CompalGetI2cBarAddr() bar0:0x%x bar1:0x%x\n",mCompalLpssPciDeviceList[BusNo+1].Bar0, mCompalLpssPciDeviceList[BusNo+1].Bar1));
        }
      }
	  AcpiBaseAddr =mCompalLpssPciDeviceList[BusNo+1].Bar0;
  }
  //
  //ACPI mode from BDS: LPSS OnReadytoBoot
  //
  else{  
	   DEBUG ((EFI_D_INFO, "CompalGetI2cBarAddr() NVS Varialable is updated by this LIB or LPSS  \n"));
  }
  DEBUG ((EFI_D_INFO, "CompalGetI2cBarAddr() BusNo+1 0x%x AcpiBaseAddr:0x%x \n", BusNo+1, AcpiBaseAddr));
  return AcpiBaseAddr;
}



EFI_STATUS
CompalProgramPciLpssI2C (
  IN  UINT8 BusNo
 )
{
  UINT32 PmcBase;
  UINTN  PciMmBase=0;
  EFI_STATUS           Status;
  EFI_GLOBAL_NVS_AREA_PROTOCOL  *GlobalNvsArea;

  UINT32 PMC_DIS[]=
    {
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC1,
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC2,
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC3,
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC4,
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC5,
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC6,
      B_PCH_PMC_FUNC_DIS_LPSS2_FUNC7
    };
  
  DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C() Start\n")); 

  //
  // Set the VLV Function Disable Register to ZERO 
  //
  PmcBase = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  if(MmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)&PMC_DIS[BusNo]){
       DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C() End:I2C[%x] is disabled\n",BusNo));  
       return EFI_NOT_READY;
  }
  DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C()------------BusNo=%x,PMC=%x\n",BusNo,MmioRead32(PmcBase+R_PCH_PMC_FUNC_DIS)));  

  //for(Index = 0; Index < LPSS_PCI_DEVICE_NUMBER; Index ++) 
  {
  
    PciMmBase = MmPciAddress (
                      mCompalLpssPciDeviceList[BusNo+1].Segment,
                      mCompalLpssPciDeviceList[BusNo+1].BusNum,
                      mCompalLpssPciDeviceList[BusNo+1].DeviceNum,
                      mCompalLpssPciDeviceList[BusNo+1].FunctionNum,
                      0
                    );
    DEBUG((EFI_D_ERROR, "Program Pci Lpss I2C Device  %x %x %x PciMmBase:%x\n", \
        mCompalLpssPciDeviceList[BusNo+1].BusNum, \
        mCompalLpssPciDeviceList[BusNo+1].DeviceNum, \
        mCompalLpssPciDeviceList[BusNo+1].FunctionNum, PciMmBase));    

    ///
    /// Check if device present
    ///
    if (MmioRead32 (PciMmBase) != 0xFFFFFFFF)     {
      if((MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_STSCMD)& B_PCH_LPSS_I2C_STSCMD_MSE)){
        mCompalLpssPciDeviceList[BusNo+1].Bar0=MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR);     //get the address allocted.
        mCompalLpssPciDeviceList[BusNo+1].Bar1=MmioRead32 (PciMmBase+R_PCH_LPSS_I2C_BAR1);
        DEBUG((EFI_D_ERROR, "CompalProgramPciLpssI2C() bar0:0x%x bar1:0x%x\n",mCompalLpssPciDeviceList[BusNo+1].Bar0, mCompalLpssPciDeviceList[BusNo+1].Bar1));
      }else
      {
        ///
        /// Program BAR 0
        ///
        ASSERT (((mCompalLpssPciDeviceList[BusNo+1].Bar0 & B_PCH_LPSS_I2C_BAR_BA) == mCompalLpssPciDeviceList[BusNo+1].Bar0) && (mCompalLpssPciDeviceList[BusNo+1].Bar0 != 0));
        MmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR), (UINT32) (mCompalLpssPciDeviceList[BusNo+1].Bar0 & B_PCH_LPSS_I2C_BAR_BA));
        ///
        /// Program BAR 1
        ///
        ASSERT (((mCompalLpssPciDeviceList[BusNo+1].Bar1 & B_PCH_LPSS_I2C_BAR1_BA) == mCompalLpssPciDeviceList[BusNo+1].Bar1) && (mCompalLpssPciDeviceList[BusNo+1].Bar1 != 0));
        MmioWrite32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_BAR1), (UINT32) (mCompalLpssPciDeviceList[BusNo+1].Bar1 & B_PCH_LPSS_I2C_BAR1_BA));
        ///
        /// Bus Master Enable & Memory Space Enable
        ///

        MmioOr32 ((UINTN) (PciMmBase + R_PCH_LPSS_I2C_STSCMD), (UINT32) (B_PCH_LPSS_I2C_STSCMD_BME | B_PCH_LPSS_I2C_STSCMD_MSE));
        ASSERT (MmioRead32 (mCompalLpssPciDeviceList[BusNo+1].Bar0) != 0xFFFFFFFF);
      }
      ///
      /// Release Resets
      ///
      MmioWrite32 (mCompalLpssPciDeviceList[BusNo+1].Bar0 + R_PCH_LPIO_I2C_MEM_RESETS,(B_PCH_LPIO_I2C_MEM_RESETS_FUNC | B_PCH_LPIO_I2C_MEM_RESETS_APB));     
      //
      // Activate Clocks
      //
      MmioWrite32 (mCompalLpssPciDeviceList[BusNo+1].Bar0 + R_PCH_LPSS_I2C_MEM_PCP,0x80020003);//No use for A0

      DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C() Programmed()\n"));      
    }
	//
	//BDS: already switched to ACPI mode
	//
	else{
      ASSERT(gBS!=NULL);      
      Status = gBS->LocateProtocol (
                      &gEfiGlobalNvsAreaProtocolGuid,
                      NULL,
                      &GlobalNvsArea
                      );
      if (EFI_ERROR(Status)) {
        DEBUG ((EFI_D_INFO, "CompalGetI2cBarAddr() gEfiGlobalNvsAreaProtocolGuid:%r\n", Status));  
        return 0;//not ready
      }     
      mCompalLpssPciDeviceList[BusNo+1].Bar0 = *(UINTN*)((CHAR8*)GlobalNvsArea->Area + Compali2cNvsBaseAddress[BusNo+1]);   
      DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C(): is switched to ACPI 0x:%x \n",mCompalLpssPciDeviceList[BusNo+1].Bar0));
    }
  }
  DEBUG ((EFI_D_INFO, "CompalProgramPciLpssI2C() End\n"));  
    
  return EFI_SUCCESS;
}

RETURN_STATUS
CompalI2cDisable (
  VOID
)
{
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_ENABLE, 0 );
  while ( 0 != ( MmioRead32 ( mCompalI2CBaseAddress + R_IC_ENABLE_STATUS) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return RETURN_NOT_READY;
  }
  return RETURN_SUCCESS;
}

RETURN_STATUS
CompalI2cEnable (
  VOID
)
{
  UINT32 NumTries = 10000;  /* 0.1 seconds */
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_ENABLE, 1 );
  while ( 0 == ( MmioRead32 ( mCompalI2CBaseAddress + R_IC_ENABLE_STATUS) & 1 )) {
    MicroSecondDelay ( 10 );
    NumTries --;
    if(0 == NumTries) return RETURN_NOT_READY;
  }
  return RETURN_SUCCESS;
}
RETURN_STATUS
CompalI2cBusFrequencySet (
  IN UINTN BusClockHertz
)
{
  DEBUG((EFI_D_INFO,"InputFreq BusClockHertz: %d\r\n",BusClockHertz));
  //
  //  Set the 100 KHz clock divider according to SV result and I2C spec
  //
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_SS_SCL_HCNT, (UINT16)0x214 );
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_SS_SCL_LCNT, (UINT16)0x272 );  
  //
  //  Set the 400 KHz clock divider according to SV result and I2C spec
  //
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_FS_SCL_HCNT, (UINT16)0x50 );
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_FS_SCL_LCNT, (UINT16)0xAD );
  
  switch ( BusClockHertz ) {
    case 100 * 1000:
      MmioWrite32 ( mCompalI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x40);//100K     
      CompalI2cMode = V_SPEED_STANDARD;
      break;
    case 400 * 1000:
      MmioWrite32 ( mCompalI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x32);//400K     
      CompalI2cMode = V_SPEED_FAST;
      break;
    default:
      MmioWrite32 ( mCompalI2CBaseAddress + R_IC_SDA_HOLD, (UINT16)0x09);//3.4M
      CompalI2cMode = V_SPEED_HIGH;
  }
  
  //
  //  Select the frequency counter
  //  Enable restart condition,
  //  Enable master FSM, disable slave FSM
  //
  CompalI2cMode |= B_IC_RESTART_EN | B_IC_SLAVE_DISABLE | B_MASTER_MODE;

  return EFI_SUCCESS;
}

EFI_STATUS
CompalI2CInit (
  IN  UINT8  BusNo,
  IN  UINT16 SlaveAddress
)
/*++

Routine Description:
  
  Initializes the host controller to execute I2C commands.
    
               
Returns:

  EFI_SUCCESS             Opcode initialization on the I2C host controller completed.
  EFI_DEVICE_ERROR        Device error, operation failed.
    
--*/
{
  EFI_STATUS Status=RETURN_SUCCESS;
  UINT32    NumTries = 0;
  UINTN    GnvsI2cBarAddr=0;
  //
  //  Verify the parameters
  //
  if (( 1023 < SlaveAddress) || ( 6 < BusNo)) {
    Status =  RETURN_INVALID_PARAMETER;
    DEBUG((EFI_D_INFO,"CompalI2CInit Exit with RETURN_INVALID_PARAMETER\r\n"));
    return Status;
  }
  
  if  (mCompalI2CSlaveAddress != SlaveAddress)
  {
    MmioWrite32 ( mCompalI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );
    mCompalI2CSlaveAddress = SlaveAddress;
  }

  //
  //1.PEI: program and init ( before pci enumeration).
  //2.DXE:update address and re-init ( after pci enumeration).
  //3.BDS:update ACPI address and re-init ( after acpi mode is enabled).  
  //
  if(mCompalI2CBaseAddress == mCompalLpssPciDeviceList[BusNo+1].Bar0) {
  	// CompalI2CInit is already  called
    GnvsI2cBarAddr=CompalGetI2cBarAddr(BusNo);
    if(
		(GnvsI2cBarAddr == 0)||//PEI: Gnvs not ready.
		(GnvsI2cBarAddr == mCompalI2CBaseAddress)//DXE and BDS
		){
      DEBUG((EFI_D_INFO,"CompalI2CInit Exit with mCompalI2CBaseAddress:%x == [%x].Bar0\r\n",mCompalI2CBaseAddress,BusNo+1));
      return RETURN_SUCCESS;
    }
  }
  Status=CompalProgramPciLpssI2C(BusNo);	  
  if(Status!=EFI_SUCCESS)
    return Status;    
  
  
  mCompalI2CBaseAddress = (UINT32) mCompalLpssPciDeviceList[BusNo+1].Bar0;  
  DEBUG ((EFI_D_ERROR, "mCompalI2CBaseAddress = 0x%x \n",mCompalI2CBaseAddress)); 
  
  NumTries = 10000; /* 1 seconds */
  while (( 1 == ( MmioRead32 ( mCompalI2CBaseAddress + R_IC_STATUS) & STAT_MST_ACTIVITY ))) {
    MicroSecondDelay(10);
    NumTries --;
    if(0 == NumTries) {
      DEBUG((EFI_D_INFO, "Try timeout\r\n"));  
      return RETURN_DEVICE_ERROR;
    }
  }
  
  Status = CompalI2cDisable();
  DEBUG((EFI_D_INFO, "CompalI2cDisable Status = %r\r\n", Status));  
  CompalI2cBusFrequencySet(400 * 1000);//Set CompalI2cMode  
  //MmioWrite32( mCompalI2CBaseAddress + R_IC_CLK_GATE, 0x01);//No use for A0
  MmioWrite32( mCompalI2CBaseAddress + R_IC_INTR_MASK, 0x0);
  if ( 0x7f < SlaveAddress ) 
    SlaveAddress = ( SlaveAddress & 0x3ff ) | IC_TAR_10BITADDR_MASTER;
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_RX_TL, 0);
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_TX_TL, 0 );  
  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_CON, CompalI2cMode);  
  Status = CompalI2cEnable();  

  DEBUG((EFI_D_INFO, "CompalI2cEnable Status = %r\r\n", Status));
  MmioRead32 ( mCompalI2CBaseAddress + R_IC_CLR_TX_ABRT );
  return EFI_SUCCESS;  
}


EFI_STATUS 
CompalByteReadI2C_Serial(
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

  Status = EFI_SUCCESS;
  
  ReceiveDataEnd = &ReadBuffer [ ReadBytes ];  
  if( ReadBytes ) {
    
      ReceiveRequest = ReadBuffer;
      //DEBUG((EFI_D_INFO,"Read: ---------------%d bytes to RX\r\n",ReceiveDataEnd - ReceiveRequest));
      while ( (ReceiveDataEnd > ReceiveRequest) || 
              (ReceiveDataEnd > ReadBuffer)) {
      //  Check for NACK
      raw_intr_stat = (UINT16)MmioRead32 ( mCompalI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        MmioRead32 ( mCompalI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_INFO,"TX ABRT ,%d bytes hasn't been transferred\r\n",ReceiveDataEnd - ReceiveRequest));
        break;
      }
      
      //  Determine if another byte was received
      I2cStatus = (UINT16)MmioRead32 ( mCompalI2CBaseAddress + R_IC_STATUS );
      if ( 0 != ( I2cStatus & STAT_RFNE )) {
        ReceiveData = (UINT16)MmioRead32 ( mCompalI2CBaseAddress + R_IC_DATA_CMD );
        *ReadBuffer = (UINT8)ReceiveData;
		*ReadBuffer++;
        DEBUG((EFI_D_INFO,"MmioRead32 ,1 byte 0x:%x is received\r\n",ReceiveData));
              
      }
    
      if(ReceiveDataEnd==ReceiveRequest){ 
		        MicroSecondDelay ( FIFO_WRITE_DELAY );
		        DEBUG((EFI_D_INFO,"ReceiveDataEnd==ReceiveRequest------------%x\r\n",I2cStatus & STAT_RFNE));
                Count++;
                if(Count<1024)//to avoid sys hung  without ul-pmc device  on RVP
                    continue;//Waiting the last request to get data and make (ReceiveDataEnd > ReadBuffer) =TRUE.
                else
                    break;     
             }
      //  Wait until a read request will fit
      if ( 0 == ( I2cStatus & STAT_TFNF )) {
		DEBUG((EFI_D_INFO,"Wait until a read request will fit\r\n"));
        MicroSecondDelay ( 10 );
        continue;
      }
      //  Issue the next read request
	  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );


      if (ReadBytes == 1)
	   MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART|B_CMD_STOP);				   
	  else if (i == 1)		   
		   MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_RESTART); 
	  else if (i == ReadBytes )
		   MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD|B_CMD_STOP);
	  else
		   MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, B_READ_CMD);
   
      MicroSecondDelay ( FIFO_WRITE_DELAY );//wait after send cmd

      ReceiveRequest += 1;
	  i += 1;
      }
      
  }
  return Status;

}

EFI_STATUS 
CompalByteWriteI2C_Serial(
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

  Status = EFI_SUCCESS;
  
  
  Status=CompalI2CInit(BusNo, (UINT16)SlaveAddress);
  if(Status!=EFI_SUCCESS)
    return Status;
  
  TransmitEnd = &WriteBuffer [ WriteBytes ];  
  if( WriteBytes ) {
     
    while ( TransmitEnd > WriteBuffer ) {

      I2cStatus = MmioRead32 ( mCompalI2CBaseAddress + R_IC_STATUS );
      raw_intr_stat = (UINT16)MmioRead32 ( mCompalI2CBaseAddress + R_IC_RAW_INTR_STAT );
      if ( 0 != ( raw_intr_stat & I2C_INTR_TX_ABRT )) {
        MmioRead32 ( mCompalI2CBaseAddress + R_IC_CLR_TX_ABRT );
        Status = RETURN_DEVICE_ERROR;
        DEBUG((EFI_D_ERROR,"TX ABRT TransmitEnd:0x%x WriteBuffer:0x%x\r\n", TransmitEnd, WriteBuffer));
        break;
      }
      if ( 0 == ( I2cStatus & STAT_TFNF )) {//if tx not full , will  send cmd  or continue to wait 
		MicroSecondDelay ( FIFO_WRITE_DELAY );
        continue;
      }
	  
	  MmioWrite32 ( mCompalI2CBaseAddress + R_IC_TAR, (UINT16)SlaveAddress );

		
	  if(!Mode)//Write = 0
	  {
		if (WriteBytes == 1)
			MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART|B_CMD_STOP);				   
		else if (i == 1)		   
			MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART); 
		else if (i == WriteBytes )
			MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_STOP);
		else
			MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++));

	  }

	   if(Mode)//Read=1
	  {  
		if (i == 1)
			MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++)|B_CMD_RESTART);
	  	else 
			MmioWrite32 (mCompalI2CBaseAddress + R_IC_DATA_CMD, (*WriteBuffer++));  
	  }
      // Add a small delay to work around some odd behavior being seen.  Without this delay bytes get dropped.
      MicroSecondDelay ( FIFO_WRITE_DELAY );//wait after send cmd
      
	  i += 1;
    }
  }

  return Status;
}

