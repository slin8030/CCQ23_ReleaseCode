#ifndef _I2C_CHIP_DEFINITION_H_
#define _I2C_CHIP_DEFINITION_H_

#define MSGBUS_MASKHI		0xFFFFFF00
#define MSGBUS_MASKLO		0x000000FF
#define MESSAGE_DWORD_EN      BIT4 | BIT5 | BIT6 | BIT7

//Msg Bus Registers
#define MC_MCR			0x000000D0		//PCI 0/0/0 Message Control Register
#define MC_MDR			0x000000D4		//PCI 0/0/0 Message Data Register
#define MC_MCRX			0x000000D8		//PCI 0/0/0 Message Control Register Extension

//
// Memory Mapped IO access macros used by MSG BUS LIBRARY
//
#define MmioAddress( BaseAddr, Register ) \
  ( (UINTN)BaseAddr + \
    (UINTN)(Register) \
  )


//
// UINT32
//
#define Mmio32Ptr( BaseAddr, Register ) \
  ( (volatile UINT32 *)MmioAddress( BaseAddr, Register ) )

#define Mmio32( BaseAddr, Register ) \
  *Mmio32Ptr( BaseAddr, Register )
  
#define MsgBus32Read(PortId, Cmd, Register, Dbuff)  \
  { \
    Mmio32( PCIEX_BASE_ADDRESS, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( PCIEX_BASE_ADDRESS, MC_MCR ) = (UINT32)( (Cmd)  | ((PortId) <<16)  | ((Register & MSGBUS_MASKLO)<<8) | MESSAGE_DWORD_EN); \
    (UINT32)(Dbuff) = Mmio32( PCIEX_BASE_ADDRESS, MC_MDR ); \
  }

#define MsgBus32Write(PortId, Cmd, Register,Dbuff) \
  { \
    Mmio32( PCIEX_BASE_ADDRESS, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( PCIEX_BASE_ADDRESS, MC_MDR ) = Dbuff; \
    Mmio32( PCIEX_BASE_ADDRESS, MC_MCR ) = (UINT32)( (Cmd)  | ((PortId) <<16) | ((Register & MSGBUS_MASKLO)<<8) | MESSAGE_DWORD_EN); \
  }

//
//  LPC Device ID macros
//
//
// Device IDs that are PCH-H Desktop specific
//
#define IS_PCH_H_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    (  \
      (DeviceId == 0xA142) || \
      (DeviceId == 0xA143) || \
      (DeviceId == 0xA144) || \
      (DeviceId == 0xA145) || \
      (DeviceId == 0xA146) || \
      (DeviceId == 0xA147) || \
      (DeviceId == 0xA140) || \
      (DeviceId == 0xA141) \
    )

#define IS_PCH_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    ( \
      IS_PCH_H_LPC_DEVICE_ID_DESKTOP(DeviceId) \
    )

//
// Device IDs that are PCH-H Mobile specific
//

#define IS_PCH_H_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      (DeviceId == 0x8C49) || \
      (DeviceId == 0x8C4B) || \
      (DeviceId == 0x8C4F) || \
      (DeviceId == 0x8C41) \
    )


//
// Device IDs that are PCH-LP Mobile specific
//
#define IS_PCH_LP_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      (DeviceId == 0x9D40) || \
      (DeviceId == 0x9D41) || \
      (DeviceId == 0x9D42) || \
      (DeviceId == 0x9D43) || \
      (DeviceId == 0x9D44) || \
      (DeviceId == 0x9D45) || \
      (DeviceId == 0x9D46) || \
      (DeviceId == 0x9D47) \
    )

#define IS_PCH_LPC_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      IS_PCH_H_LPC_DEVICE_ID_MOBILE(DeviceId) || \
      IS_PCH_LP_LPC_DEVICE_ID_MOBILE(DeviceId) \
    )

//
// Device IDS that are PCH WorkStation specific
//
#define IS_PCH_H_LPC_DEVICE_ID_WS(DeviceId) \
    ( \
      (DeviceId == 0x9D58) \
    )

#define IS_PCH_LP_LPC_DEVICE_ID_WS(DeviceId) (FALSE)

#define IS_PCH_LPC_DEVICE_ID_WS(DeviceId) \
    ( \
      IS_PCH_H_LPC_DEVICE_ID_WS(DeviceId) || \
      IS_PCH_LP_LPC_DEVICE_ID_WS(DeviceId) \
    )

//
// Device IDS that are PCH Server specific
//
#define IS_PCH_H_LPC_DEVICE_ID_SERVER(DeviceId) \
    ( \
      (DeviceId == 0x9D52) || \
      (DeviceId == 0x9D54) || \
      (DeviceId == 0x9D56) \
    )

#define IS_PCH_H_LPC_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_H_LPC_DEVICE_ID_DESKTOP (DeviceId) || \
      IS_PCH_H_LPC_DEVICE_ID_MOBILE (DeviceId) || \
      IS_PCH_H_LPC_DEVICE_ID_WS (DeviceId) || \
      IS_PCH_H_LPC_DEVICE_ID_SERVER (DeviceId) \
    )

#define IS_PCH_LP_LPC_DEVICE_ID(DeviceId) \
    ( \
     IS_PCH_LP_LPC_DEVICE_ID_MOBILE (DeviceId) \
    )

#define IS_PCH_LPC_DEVICE_ID(DeviceId) \
    ( \
      IS_PCH_H_LPC_DEVICE_ID(DeviceId) || \
      IS_PCH_LP_LPC_DEVICE_ID(DeviceId) \
    )

typedef enum {
  PchH          = 1,
  PchLp,
  PchUnknownSeries
} PCH_SERIES;

UINTN
LpssI2cGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  );

VOID
LpssSwitchDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort,
  IN UINT32                 DeviceMode
  );

UINTN
SerialIoGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  );

VOID
SerialIowitchDevMode (
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN UINT32                DeviceMode
  );

UINTN
AmdI2cGetDevMode (
  IN I2C_BUS_INSTANCE       *I2cPort
  );

VOID
AmdI2cwitchDevMode (
  IN I2C_BUS_INSTANCE      *I2cPort,
  IN UINT32                DeviceMode
  );

BOOLEAN
ClvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

BOOLEAN
ChvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

BOOLEAN
VlvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

BOOLEAN
SkbGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

BOOLEAN
BdwGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

BOOLEAN
ClvGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

BOOLEAN
CarrizoGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

VOID
VlvChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  );

VOID
ChvChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  );

VOID
SkbChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  );

VOID
BdwChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  );

VOID
SklChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  );

UINTN
SklGetDevMode (
  IN I2C_BUS_INSTANCE      *this
  );

VOID
SklSwitchDevMode (
  IN I2C_BUS_INSTANCE      *this,
  IN UINT32                DeviceMode
  );

BOOLEAN
SklGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              IntGpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

VOID
AplChipInit (
  IN  I2C_BUS_INSTANCE   *this    
  );

UINTN
AplGetDevMode (
  IN I2C_BUS_INSTANCE      *this
  );

VOID
AplSwitchDevMode (
  IN I2C_BUS_INSTANCE      *this,
  IN UINT32                DeviceMode
  );

BOOLEAN
AplGpioEventArrived(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              IntGpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );


#endif
