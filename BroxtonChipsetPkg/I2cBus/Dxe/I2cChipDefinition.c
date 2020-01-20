#include "I2cMasterDxe.h"
#include "I2cChipDefinition.h"


I2C_INFO mI2cBusInfo[] = {
  { 0x00030650,  0xC0,    0xD0, { {0x005C, 0x0063, 0x0B}, {0x000B, 0x001E, 0x0B}, {0x0004, 0x000C, 0x0B}, } ,6, NULL,        LpssI2cGetDevMode,   LpssSwitchDevMode,     ClvGpioEventArrived    },
  { 0x00040650,  0x100*8, 0x7C, { {0x0190, 0x10D6, 0x09}, {0x01B0, 0x01FB, 0x09}, {0x000B, 0x0016, 0x09}, } ,2, SkbChipInit, SerialIoGetDevMode,  SerialIowitchDevMode , SkbGpioEventArrived    }, 
  { 0x000306D0,  0x100*8, 0x7C, { {0x0190, 0x10D6, 0x09}, {0x0048, 0x00A0, 0x09}, {0x000B, 0x0016, 0x09}, } ,2, BdwChipInit, SerialIoGetDevMode,  SerialIowitchDevMode , BdwGpioEventArrived    },
  { 0x000406E0,  0x100*8, 0x7C, { {0x01F4, 0x024C, 0x1C}, {0x0080, 0x00AD, 0x1C}, {0x000F, 0x0028, 0x1C}, } ,2, SklChipInit, SklGetDevMode,       SklSwitchDevMode,      SklGpioEventArrived    },  
  { 0x000506E0,  0x100*8, 0x7C, { {0x01F4, 0x024C, 0x1C}, {0x0080, 0x00AD, 0x1C}, {0x000F, 0x0028, 0x1C}, } ,2, SklChipInit, SklGetDevMode,       SklSwitchDevMode,      SklGpioEventArrived    },    
  { 0x00030670,  0xC0,    0x7C, { {0x0264, 0x02C2, 0x15}, {0x006E, 0x00CF, 0x15}, {0x000B, 0x0013, 0x09}, } ,7, VlvChipInit, LpssI2cGetDevMode,   LpssSwitchDevMode,     VlvGpioEventArrived    },
  { 0x000406C0,  0xC0,    0x7C, { {0x0214, 0x0272, 0x40}, {0x0050, 0x00AD, 0x32}, {0x000B, 0x0013, 0x09}, } ,7, ChvChipInit, LpssI2cGetDevMode,   LpssSwitchDevMode,     ChvGpioEventArrived    },

  { 0x000506C0,  0xC0,    0x7C, { {0x0244, 0x02D0, 0x40}, {0x0064, 0x00C8, 0x32}, {0x0006, 0x0013, 0x09}, } ,8, AplChipInit, AplGetDevMode,       AplSwitchDevMode,      AplGpioEventArrived    },

  { 0x00660F00,  0x0000,  0x00, { {0x023C, 0x02F7, 0x38}, {0x0033, 0x0053, 0x30}, {0x0006, 0x0010, 0x09}, } ,4, NULL,        AmdI2cGetDevMode,    AmdI2cwitchDevMode,    CarrizoGpioEventArrived},
  { 0xFFFFFFFF,  0x0000,  0x00, { {0}, {0}, {0},}}
};


BOOLEAN
ClvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;
  
  if(GpioIntPin < 32){
    Gplr = MmioRead32(this->GpioBase + 0x00);
    if (((Gplr >> GpioIntPin) & 1) == ActiveLevel)             return TRUE;
  }else if(GpioIntPin >= 32 && GpioIntPin < 64){
    Gplr = MmioRead32(this->GpioBase + 0x04);
    if (((Gplr >> (GpioIntPin - 32)) & 1) == ActiveLevel)      return TRUE;
  }else if(GpioIntPin >= 64 && GpioIntPin < 96){
    Gplr = MmioRead32(this->GpioBase + 0x08);
    if (((Gplr >> (GpioIntPin - 64)) & 1) == ActiveLevel)      return TRUE;
  }
  return FALSE;
}

VOID
VlvChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  )
{
  this->I2cHcWrite32(this, 0x804, 3);
  this->I2cHcWrite32(this, 0x808, 0);    
  this->I2cHcWrite32(this, 0x800, 0x80010003);
  
  while ((this->I2cHcRead32(this, 0x804) & (BIT0|BIT1)) != (BIT0|BIT1));
  
  this->I2cHcWrite32(this, 0x800, 0x80010003);
  gBS->Stall (1000);    
}

BOOLEAN
VlvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;
  
  Gplr = MmioRead32(this->GpioBase + (0x1000 * GpioController) + (GpioIntPin * 0x10) + 0x08);
  
  if ((Gplr & BIT0) == ActiveLevel) {
    return TRUE;
  }
  
  return FALSE;
}

VOID
ChvChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  )
{
  this->I2cHcWrite32(this, 0x804, 3);
  this->I2cHcWrite32(this, 0x808, 0);    
  this->I2cHcWrite32(this, 0x800, 0x80010003);
  
  while ((this->I2cHcRead32(this, 0x804) & (BIT0|BIT1)) != (BIT0|BIT1));
  
  this->I2cHcWrite32(this, 0x800, 0x80010003);
  gBS->Stall (1000);    
}

BOOLEAN
ChvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;

  Gplr = MmioRead32 (this->GpioBase + 1024 + (0x4000 * GpioController * 2) + 0x4000 + (((GpioIntPin/15) * 0x400) + ((GpioIntPin%15) * 8)));
  
  if ((Gplr & BIT0) == ActiveLevel) {
    return TRUE;
  }
  return FALSE;
}

VOID
SkbChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  )
{
  UINT32  Reg32;
  
  Reg32 = this->I2cHcRead32(this, 0x808);
  Reg32 &= ~(BIT2|BIT3);
  this->I2cHcWrite32(this, 0x808, Reg32);
  
  Reg32 = this->I2cHcRead32(this, 0x804);
  Reg32 &= ~(BIT0|BIT1);
  this->I2cHcWrite32(this, 0x804, Reg32);
  this->I2cHcWrite32(this, 0x804, 3);
  this->I2cHcWrite32(this, 0x814, 0);
  this->I2cHcWrite32(this, 0x808, BIT3);
}

BOOLEAN
SkbGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;
  
  Gplr = IoRead32(this->GpioBase + 0x100 + (GpioIntPin * 0x08) );
  
  if (((Gplr >> 30) & 1) == ActiveLevel)                       return TRUE;
  return FALSE;
}


VOID
BdwChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  )
{
  UINT32  Reg32;
  
  Reg32 = this->I2cHcRead32(this, 0x808);
  Reg32 &= ~(BIT2|BIT3);
  this->I2cHcWrite32(this, 0x808, Reg32);
  
  Reg32 = this->I2cHcRead32(this, 0x804);
  Reg32 &= ~(BIT0|BIT1);
  this->I2cHcWrite32(this, 0x804, Reg32);
  this->I2cHcWrite32(this, 0x804, 3);
  this->I2cHcWrite32(this, 0x814, 0);
  this->I2cHcWrite32(this, 0x808, BIT3);
}

BOOLEAN
BdwGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;
  
  Gplr = IoRead32(this->GpioBase + 0x100 + (GpioIntPin * 0x08) );
  
  if (((Gplr >> 30) & 1) == ActiveLevel)                       return TRUE;
  return FALSE;
}


BOOLEAN
CarrizoGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;
  
  Gplr = MmioRead32 (this->GpioBase + (GpioIntPin * 0x04));
  
  if (((Gplr >> 16) & 1) == ActiveLevel) {
    return TRUE;
  }
  
  return FALSE;
}

UINTN
GetTotalChipBus (
  IN UINT32   Cpuid
  )
{
  UINT32                   Index;

  Index = 0;
  
  while(mI2cBusInfo[Index].Cpuid != 0xFFFFFFFF){
    if(mI2cBusInfo[Index].Cpuid == Cpuid){
      return mI2cBusInfo[Index].TotalBus;
    }
    Index++;
  }

  return 0;
}

EFI_STATUS
GetChipInfo(
  IN  I2C_BUS_INSTANCE                     *I2cContext,
  IN  UINTN                                FuncIndex,
  OUT UINT8                                *I2cHcType
  )
{
  UINT32                   Index;
  UINTN                    PciDevBase;
  EFI_STATUS               Status;
  
  Index     = 0;
  *I2cHcType = 0;
  
  Status = EFI_NOT_FOUND;
  while(mI2cBusInfo[Index].Cpuid != 0xFFFFFFFF){
    if(mI2cBusInfo[Index].Cpuid == I2cContext->Info.Cpuid){

      CopyMem( &I2cContext->Info, &(mI2cBusInfo[Index]), sizeof(I2C_INFO));

      switch(mI2cBusInfo[Index].Cpuid){
      case 0x00030650:
        I2cContext->GpioBase        = 0xFF119000;
        I2cContext->I2cBase         = (UINT32)(0xFF138000 + (FuncIndex*0x1000));
        I2cContext->I2cBaseLen      = 0x1000;
        
        *I2cHcType = I2C_HC_TYPE_MMIO;
        break;

      case 0x000406E0:
      case 0x000506E0:
        I2cContext->PciI2c.Bus  = 0;
        I2cContext->PciI2c.Dev  = 21;
        I2cContext->PciI2c.Func = FuncIndex;
        I2cContext->I2cBase     = 0xFE020000 + (UINT32)(0x1000 * FuncIndex * 2);
        I2cContext->I2cBaseLen  = 0x1000;
        I2cContext->GpioBase   = 0xFD000000;
        
        *I2cHcType = I2C_HC_TYPE_PCI;        
        break;
        
      case 0x000306D0:
      case 0x00040650:
        I2cContext->PciI2c.Bus  = 0;
        I2cContext->PciI2c.Dev  = 21;
        I2cContext->PciI2c.Func = 1 + FuncIndex;
        I2cContext->I2cBase     = 0xFE103000 + (UINT32)(0x1000 * FuncIndex * 2);
        I2cContext->I2cBaseLen  = 0x1000;
            
        PciDevBase = MmPciAddress ( 0, 0, 31, 0,  0x48);
        I2cContext->GpioBase   = MmioRead32 (PciDevBase) & 0x0000FFC0;
        
        *I2cHcType = I2C_HC_TYPE_PCI;
        break;
        
      case 0x000406C0:
        I2cContext->PciI2c.Bus  = 0;
        I2cContext->PciI2c.Dev  = 24;
        I2cContext->PciI2c.Func = 1 + FuncIndex;
        I2cContext->I2cBaseLen  = 0x1000;

        I2cContext->GpioBase    = 0xFED80000;
        
        *I2cHcType = I2C_HC_TYPE_PCI;
        break;

      case 0x00030670:
        I2cContext->PciI2c.Bus  = 0;
        I2cContext->PciI2c.Dev  = 24;
        I2cContext->PciI2c.Func = 1 + FuncIndex;
        I2cContext->I2cBaseLen  = 0x1000;

        *I2cHcType = I2C_HC_TYPE_PCI;

        I2cContext->GpioBase    = 0xFED0C000;
        break;

      case 0x00660F00:
        I2cContext->GpioBase    = 0xFED81500;
        I2cContext->I2cBase     = (UINT32)(0xFEDC2000 + FuncIndex * 0x1000);
        I2cContext->I2cBaseLen  = 0x1000;
        
        *I2cHcType = I2C_HC_TYPE_MMIO;
        break;

      case 0x000506C0:
        if (FuncIndex < 4) {
          I2cContext->PciI2c.Bus  = 0;
          I2cContext->PciI2c.Dev  = 22;
          I2cContext->PciI2c.Func = FuncIndex;
        } else {
          I2cContext->PciI2c.Bus  = 0;
          I2cContext->PciI2c.Dev  = 23;
          I2cContext->PciI2c.Func = FuncIndex - 4;
        }
        I2cContext->I2cBaseLen  = 0x1000;
        I2cContext->GpioBase    = 0xD0000000;
        
        *I2cHcType = I2C_HC_TYPE_PCI;
        break;
        
      }

      Status = EFI_SUCCESS;
      break;
    }
    Index++;
  }


  //
  //  Used the default setting
  //
  if(EFI_ERROR(Status)){
    CopyMem( &I2cContext->Info, &(mI2cBusInfo[0]), sizeof(I2C_INFO));
  }

  return EFI_SUCCESS;
}

UINT32
LpssPortRead32 (
  IN UINT32    RegAddress
  )
{
  UINT32   Buf32;

  MsgBus32Read(0xA0, 0x06000000, RegAddress, Buf32);

  return Buf32;
}

VOID
LpssPortWrite32 (
  IN UINT32    RegAddress,
  IN UINT32    Value
  )
{
  MsgBus32Write(0xA0, 0x07000000, RegAddress, Value);
}

UINTN
LpssI2cGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  )
{
  if (LpssPortRead32 (0x280 + (UINT32)(I2cPort->PciI2c.Func * 8)) & (BIT20|BIT21)) {
    return I2C_DEVICE_MODE_ACPI;
  }

  return I2C_DEVICE_MODE_PCI;
}

VOID
LpssSwitchDevMode (
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN UINT32                DeviceMode
  )
{
  UINT32              Reg32;  

  Reg32 = LpssPortRead32 ( 0x280 + (UINT32)(I2cPort->PciI2c.Func * 8));
  switch (DeviceMode) {
  case I2C_DEVICE_MODE_PCI:
    Reg32 &= ~(BIT20|BIT21);
    break;
    
  case I2C_DEVICE_MODE_ACPI:
    Reg32 |= (BIT20|BIT21);
    break;
  }
  
  LpssPortWrite32 ( 0x280 + (UINT32)(I2cPort->PciI2c.Func * 8), Reg32);
  
  return;
}

/**
  Write data to PCH IOBP register block

  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] Address              Address of the IOBP register block
  @param[in] Data                 Data write to the IOBP register block

  @retval EFI_SUCCESS             Successfully completed.
  @retval EFI_DEVICE_ERROR        Transaction fail
**/
EFI_STATUS
EFIAPI
WriteIobp (
  IN UINT32                              RootComplexBar,
  IN UINT32                              Address,
  IN UINT32                              Data32
  )
{
  UINT8 ResponseStatus;

  ///
  /// Step 1 Poll RCBA + 2338[0] = 0b
  ///
  while (MmioRead8 (RootComplexBar + 0x2338) & BIT0);

  ///
  /// Step 2
  /// Write RCBA + Offset 2330h[31:0] with IOBP register address
  ///
  MmioWrite32 ((RootComplexBar + 0x2330), Address);
  ///
  /// Step 10
  /// Set RCBA + 2338h[15:8] = 00000111b
  ///
  MmioAndThenOr16 ((RootComplexBar + 0x2338), (UINT16) (~0xFF00), (UINT16) 0x0700);
  ///
  /// Step 11
  /// Write RCBA + 2334h[31:0] with updated SBI data
  ///
  MmioWrite32 ((RootComplexBar + 0x2334), Data32);

  ///
  /// Step 12
  /// Set RCBA + 233Ah[15:0] = F000h
  ///
  MmioWrite16 ((RootComplexBar + 0x233A), 0xF000);

  ///
  /// Step 13
  /// Set RCBA + 2338h[0] = 1b
  ///
  MmioOr16 ((RootComplexBar + 0x2338), (UINT16) BIT0);

  ///
  /// Step 14
  /// Poll RCBA + Offset 2338h[0] = 0b
  ///
  while (MmioRead8 (RootComplexBar + 0x2338) & BIT0);

  ///
  /// Step 15
  /// Check if RCBA + 2338h[2:1] = 00b for successful transaction
  ///
  ResponseStatus = MmioRead8 (RootComplexBar + 0x2338) & (BIT2|BIT1);
  if (ResponseStatus == 0) {
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

/**
  Read data from PCH IOBP register block

  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] Address              Address of the IOBP register block
  @param[out] Data                Data contain in the IOBP register block

  @retval EFI_SUCCESS             Successfully completed.
  @retval EFI_DEVICE_ERROR        Transaction fail
**/
EFI_STATUS
EFIAPI
ReadIobp (
  IN UINT32                              RootComplexBar,
  IN UINT32                              Address,
  OUT UINT32                             *Data
  )
{
  UINT8 ResponseStatus;

  ///
  /// Step 1 Poll RCBA + 2338[0] = 0b
  ///
  while (MmioRead8 (RootComplexBar + 0x2338) & BIT0);

  ///
  /// Step 2
  /// Write RCBA + Offset 2330h[31:0] with IOBP register address
  ///
  MmioWrite32 ((RootComplexBar + 0x2330), Address);
  ///
  /// Step 3
  /// Set RCBA + 2338h[15:8] = 00000110b
  ///
  MmioAndThenOr16 ( (RootComplexBar + 0x2338), (UINT16) (~0xFF00), (UINT16) 0x0600);
  ///
  /// Step 4
  /// Set RCBA + 233Ah[15:0] = F000h
  ///
  MmioWrite16 ((RootComplexBar + 0x233A), 0xF000);
  ///
  /// Step 5
  /// Set RCBA + 2338h[0] = 1b
  ///
  MmioOr16 ((RootComplexBar + 0x2338), (UINT16) BIT0);

  ///
  /// Step 6
  /// Poll RCBA + Offset 2338h[0] = 0b, Polling for Busy bit
  ///
  while (MmioRead8 (RootComplexBar + 0x2338) & BIT0);

  ///
  /// Step 7
  /// Check if RCBA + 2338h[2:1] = 00b for successful transaction
  ///
  ResponseStatus = MmioRead8 (RootComplexBar + 0x2338) & (BIT2|BIT1);
  if (ResponseStatus == 0) {
    ///
    /// Step 8
    /// Read RCBA + 2334h[31:0] for IOBP data
    ///
    *Data = MmioRead32 (RootComplexBar + 0x2334);
    return EFI_SUCCESS;
  } else {
    return EFI_DEVICE_ERROR;
  }
}

UINTN
SerialIoGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  )
{
  UINT32              Reg32;  
  UINT32              RootComplexBar;
  EFI_STATUS          Status;

  RootComplexBar = MmioRead32 (MmPciAddress ( 0, 0, 31, 0, 0xF0)) & 0xFFFFC000;
  
  Status = ReadIobp (RootComplexBar, 0xCB000240 + (UINT32)(I2cPort->PciI2c.Func * 8), &Reg32);

  if (Reg32 & BIT20) {
    return I2C_DEVICE_MODE_ACPI;
  }

  return I2C_DEVICE_MODE_PCI;
}

VOID
SerialIowitchDevMode (
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN UINT32                DeviceMode
  )
{
  UINT32              Reg32;  
  UINT32              RootComplexBar;
  EFI_STATUS          Status;

  RootComplexBar = MmioRead32 (MmPciAddress ( 0, 0, 31, 0, 0xF0)) & 0xFFFFC000;

  Status = ReadIobp (RootComplexBar, 0xCB000240 + (UINT32)(I2cPort->PciI2c.Func * 8), &Reg32);

  switch (DeviceMode) {
  case I2C_DEVICE_MODE_PCI:
    Reg32 &= ~BIT20;
    break;
    
  case I2C_DEVICE_MODE_ACPI:
    Reg32 |= BIT20;
    break;
  }
  
  Status = WriteIobp (RootComplexBar, 0xCB000240 + (UINT32)(I2cPort->PciI2c.Func * 8), Reg32);
  
  return;
}

UINTN
AmdI2cGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  )
{
  return I2C_DEVICE_MODE_MMIO;
}

VOID
AmdI2cwitchDevMode (
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN UINT32                DeviceMode
  )
{
  return;
}

typedef struct  _SKL_GPIO {
  UINT32 Community;
  UINT32 Offset;  
}SKL_GPIO;

SKL_GPIO *mCurrentGpioTable = NULL;

SKL_GPIO mPchLpGpioGroupInfo[] = {
  {0xAF,0x400 },
  {0xAF,0x4C0 },
  {0xAE,0x400 },
  {0xAE,0x4C0 },
  {0xAE,0x580 },
  {0xAC,0x400 },
  {0xAC,0x4c0 },
  {0xAD,0x400 },
};

SKL_GPIO mPchHGpioGroupInfo[] = {
  {0xAF,0x400 },
  {0xAF,0x4C0 },
  {0xAE,0x400 },
  {0xAE,0x4C0 },
  {0xAE,0x580 },
  {0xAE,0x5E8 },
  {0xAE,0x6A8 },
  {0xAE,0x768 },
  {0xAC,0x400 },
  {0xAD,0x400 } 
};
VOID
SklChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  )
{

}

UINTN
SklGetDevMode (
  IN I2C_BUS_INSTANCE      *this
  )
{
  return I2C_DEVICE_MODE_PCI;
}

VOID
SklSwitchDevMode (
  IN I2C_BUS_INSTANCE      *this,
  IN UINT32                DeviceMode
  )
{
  return;
}

PCH_SERIES
EFIAPI
GetPchSeries (
  VOID
  )
{
  UINT16  LpcDeviceId;
  UINT32  PchSeries;

  LpcDeviceId = MmioRead16 (MmPciAddress ( 0, 0, 31, 0, 2));

  if (IS_PCH_H_LPC_DEVICE_ID (LpcDeviceId)) {
    PchSeries = PchH;
  } else if (IS_PCH_LP_LPC_DEVICE_ID (LpcDeviceId)) {
    PchSeries = PchLp;
  } else {
    PchSeries = PchUnknownSeries;
  }

  return PchSeries;
}

SKL_GPIO*
GpioGetGroupInfoTable (
  OUT UINTN               *GpioGroupInfoTableLength    
  )
{
  if (mCurrentGpioTable) {
    return mCurrentGpioTable;
  }
  
  if (GetPchSeries () == PchLp) {
    *GpioGroupInfoTableLength = sizeof (mPchLpGpioGroupInfo) / sizeof (SKL_GPIO);
    mCurrentGpioTable = mPchLpGpioGroupInfo;
  } else {
    *GpioGroupInfoTableLength = sizeof (mPchHGpioGroupInfo) / sizeof (SKL_GPIO);
    mCurrentGpioTable = mPchHGpioGroupInfo;
  }

  return mCurrentGpioTable;  
}

BOOLEAN
SklGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              IntGpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINTN            GpioTableLength;
  UINT32           Gplr;
  SKL_GPIO         *GpioTable;
  
  GpioTable = GpioGetGroupInfoTable (&GpioTableLength);

  if (IntGpioController  >= GpioTableLength) {
    return FALSE;    
  }
  
  Gplr = MmioRead32 ((UINTN)((GpioTable[IntGpioController].Community<<16)|(0x8 * GpioIntPin + GpioTable[IntGpioController].Offset)|0xFD000000));
  
  if ((Gplr & 2) == ActiveLevel) {
    return TRUE;
  }
  
  return FALSE;
}

VOID
AplChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  )
{

}

UINTN
AplGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  )
{
//
// [2016/01/04] APL FRC 0.6.1 I2C ACPI mode is not POR'd. (BroxtonSiPkg\SouthCluster\LibraryPrivate\PeiScInitLib\ScLpss.c)
//              Return PCI mode.
//
//  if (LpssPortRead32 (0x280 + (UINT32)(I2cPort->PciI2c.Func * 8)) & (BIT20|BIT21)) {
//    return I2C_DEVICE_MODE_ACPI;
//  }

  return I2C_DEVICE_MODE_PCI;
}

VOID
AplSwitchDevMode (
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN UINT32                DeviceMode
  )
{
//
// [2016/01/04] APL FRC 0.6.1 I2C ACPI mode is not POR'd.
//              Just return.
//
//  UINT32              Reg32;  
//
//  Reg32 = LpssPortRead32 ( 0x280 + (UINT32)(I2cPort->PciI2c.Func * 8));
//  switch (DeviceMode) {
//  case I2C_DEVICE_MODE_PCI:
//    Reg32 &= ~(BIT20|BIT21);
//    break;
//    
//  case I2C_DEVICE_MODE_ACPI:
//    Reg32 |= (BIT20|BIT21);
//    break;
//  }
//  
//  LpssPortWrite32 ( 0x280 + (UINT32)(I2cPort->PciI2c.Func * 8), Reg32);
  
  return;
}

#define APL_GPIO_PADBAR              0x500
//
// APL GPIO Community Port ID definition
//
#define GPIO_SOUTHWEST_COMMUNITY     0xC0
#define GPIO_SOUTH_COMMUNITY         0xC2
#define GPIO_NORTHWEST_COMMUNITY     0xC4
#define GPIO_NORTH_COMMUNITY         0xC5
#define GPIO_WEST_COMMUNITY          0xC7

// GPIO controller 0 based, North = 0, NorthWest = 1, West = 2, Southwest = 3, South = 4.
UINT8  mAplGpioCommunityTable[] = {GPIO_NORTH_COMMUNITY, GPIO_NORTHWEST_COMMUNITY, GPIO_WEST_COMMUNITY, GPIO_SOUTHWEST_COMMUNITY, GPIO_SOUTH_COMMUNITY};

BOOLEAN
AplGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  )
{
  UINT32  Gplr;

  Gplr = MmioRead32 (this->GpioBase + (mAplGpioCommunityTable[GpioController] << 16) + APL_GPIO_PADBAR + (GpioIntPin * 0x8));
  
  if (((Gplr & BIT1) > 1) == ActiveLevel) {
    return TRUE;
  }
  return FALSE;
}

