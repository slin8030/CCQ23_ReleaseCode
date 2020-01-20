/** @file
  For I2C Master Driver

;******************************************************************************
;* Copyright (c) 2012 - 2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/


#ifndef _I2C_MASTER_DXE_H_
#define _I2C_MASTER_DXE_H_
#include <IndustryStandard/Pci.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/I2cMaster.h>
#include <Protocol/DevicePath.h>
#include <Protocol/I2cBusConfigurationManagement.h>
#include <Protocol/AdapterInformation.h>
#include <Protocol/I2cEnumerate.h>
#include <I2cHidDeviceInfo.h>
#include "I2cRegs.h"

//
// Definitions
//
#define I2C_PRIVATE_DATA_SIGNATURE   SIGNATURE_32('p','i','2','c')

#define MICROSECOND               10
#define MILLISECOND              (1000 * MICROSECOND)

#define DEFAULT_CACHE_SIZE        0x100

#define I2C_HC_TYPE_PCI           0
#define I2C_HC_TYPE_MMIO          1

//
//  CPU vendor definition
//
#define INTEL_VENDOR_ID                 0x8086
#define AMD_VENDOR_ID                   0x1002
#define VIA_VENDOR_ID                   0x1106

#ifndef PCIEX_BASE_ADDRESS
#define PCIEX_BASE_ADDRESS  0xE0000000
#endif

#define FIFO_SIZE_IN_BYTES    (UINT32)-1

#ifndef PCI_EXPRESS_BASE_ADDRESS
#define PCI_EXPRESS_BASE_ADDRESS ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)
#endif

#ifndef MmPciAddress
#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )

#endif

typedef struct _I2C_BUS_INSTANCE I2C_BUS_INSTANCE;
//
// Private Data Structures
//
#define I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL(a)  CR (a, I2C_BUS_INSTANCE, MasterApi, I2C_PRIVATE_DATA_SIGNATURE)

#define I2C_PORT_CONTEXT_FROM_COFNIG_PROTOCOL(a)  CR (a, I2C_BUS_INSTANCE, ConfigApi, I2C_PRIVATE_DATA_SIGNATURE)

#define I2C_ASYNC_CONTEXT_FROM_LINK(a)            CR (a, I2C_ASYNC_NODE, Link, I2C_PRIVATE_DATA_SIGNATURE)

typedef
UINT8
(EFIAPI *I2C_IO_READ8) (
  IN      I2C_BUS_INSTANCE          *Private,
  IN      UINT16                    Offset
  )
;

typedef
UINT16
(EFIAPI *I2C_IO_READ16) (
  IN      I2C_BUS_INSTANCE          *Private,
  IN      UINT16                    Offset
  )
;

typedef
UINT32
(EFIAPI *I2C_IO_READ32) (
  IN      I2C_BUS_INSTANCE          *Private,
  IN      UINT16                    Offset
  )
;

typedef
VOID
(EFIAPI *I2C_IO_WRITE8) (
  IN      I2C_BUS_INSTANCE          *Private,
  IN      UINT16                    Offset,
  IN      UINT8                     Data
  )
;

typedef
VOID
(EFIAPI *I2C_IO_WRITE16) (
  IN      I2C_BUS_INSTANCE          *Private,
  IN      UINT16                    Offset,
  IN      UINT16                    Data
  )
;

typedef
VOID
(EFIAPI *I2C_IO_WRITE32) (
  IN      I2C_BUS_INSTANCE          *Private,
  IN      UINT16                    Offset,
  IN      UINT32                    Data
  )
;

typedef struct {
  UINT8                 SlaveAddress;
  UINT8                 GpioIntPin;
  UINT8                 IntPinLevel;
  UINT8                 GpioController;
  UINT16                HidDescReg;
  UINT16                Devicetype;
  UINT16                HostNum;
  UINT16                SpeedOverrid;
}I2C_HID_DEV;

typedef struct {
  UINT8                 SlaveAddress;
  UINT8                 Reserved;
  UINT16                Length;
  UINT32                Command;
} CACHE_HEADER;

enum CLK_SPEED{
  I2C_STANDARD = 0,
  I2C_FAST,
  I2C_HIGH,
  MAX_I2C_SPEED
};

typedef 
struct _LVL_INFO{
  UINT16   H;
  UINT16   L;
  UINT8    S;
  UINT8    Reserved1;
  UINT16   Reserved2;  
}LVL_INFO;

#define I2C_DEVICE_MODE_MMIO  1
#define I2C_DEVICE_MODE_PCI   2
#define I2C_DEVICE_MODE_ACPI  3


typedef
UINTN
(*GET_DEVICE_MODE) (
  IN I2C_BUS_INSTANCE    *Private
  );

typedef
VOID
(*SWITCH_DEVICE_MODE) (
  IN I2C_BUS_INSTANCE    *Private,
  IN UINT32              DeviceMode
  );


typedef
BOOLEAN
(*GPIO_EVENT_ARRIVED)(
  IN  I2C_BUS_INSTANCE   *this,
  IN  UINT8              GpioController,  
  IN  UINT8              GpioIntPin,
  IN  UINT8              ActiveLevel
  );

typedef
VOID
(*CHIP_INIT)(
  IN  I2C_BUS_INSTANCE   *this
  );

typedef
struct _I2C_INFO{
  UINT32              Cpuid;
  UINT16              CRG;
  UINT16              SHR;
  LVL_INFO            QkR[MAX_I2C_SPEED];
  UINTN               TotalBus;

  CHIP_INIT           Init;
  GET_DEVICE_MODE     GetDeviceMode;
  SWITCH_DEVICE_MODE  SwitcDevhMode;
  GPIO_EVENT_ARRIVED  GpioArrived;  
}I2C_INFO;

typedef 
struct _PCI_DEV_DEF{
  UINTN  Seg;
  UINTN  Bus;
  UINTN  Dev;
  UINTN  Func;
}PCI_DEV_DEF;

typedef struct {
  PCI_DEVICE_PATH        Pci;
  EFI_DEVICE_PATH        End;
}I2C_PCI_HC_DEV_PATH;

typedef struct {
  MEMMAP_DEVICE_PATH     Mmio;
  EFI_DEVICE_PATH        End;
}I2C_MEM_HC_DEV_PATH;

typedef
struct _I2C_ASYNC_NODE{
  UINTN                      Signature;
  LIST_ENTRY                 Link;
  H2O_I2C_HID_DEVICE         *I2cDev;
  EFI_I2C_REQUEST_PACKET     *RequestPacket;
  EFI_EVENT                  Event;
  EFI_STATUS                 *I2cStatus;
}I2C_ASYNC_NODE;
//
// Declare a local instance structure for this driver
//
struct _I2C_BUS_INSTANCE {
  //
  // Private Data
  //
  UINTN                 Signature;
  
  EFI_HANDLE            Handle;

  LIST_ENTRY            AsyncQueue;

  UINTN                 CacheMode;
  UINTN                 CacheAvailable;
  UINTN                 CacheSize;
  UINT8                 *Cache;
  
  UINT32                I2cBase;
  UINT32                I2cBaseLen;
  UINT32                I2cBusSpeed;
    
  UINT32                GpioBase;
  UINT32                MaxFifoLen;
  UINT32                IntervalTime;
  
  UINTN                 RetryCount;
  UINTN                 CurrentRetryCount;

  EFI_EVENT             TimEvent;

  PCI_DEV_DEF           PciI2c;
  
  I2C_IO_READ8          I2cHcRead8;
  I2C_IO_READ16         I2cHcRead16;
  I2C_IO_READ32         I2cHcRead32;
  
  I2C_IO_WRITE8         I2cHcWrite8;
  I2C_IO_WRITE16        I2cHcWrite16;
  I2C_IO_WRITE32        I2cHcWrite32;
  
  I2C_INFO              Info;  
  //
  // Published interface
  //
  EFI_DEVICE_PATH_PROTOCOL                       *DevicePath;  
  EFI_I2C_MASTER_PROTOCOL                        MasterApi;
  EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL  ConfigApi;
};

//
// I2C bus internal function
//
UINT8
I2cHcRead8 (
  IN  I2C_BUS_INSTANCE                  *Private,
  IN  UINT16                            Offset
  );

UINT16
I2cHcRead16 (
  IN  I2C_BUS_INSTANCE                  *Private,
  IN  UINT16                            Offset
  );

UINT32
I2cHcRead32 (
  IN  I2C_BUS_INSTANCE                  *Private,
  IN  UINT16                            Offset
  );

VOID
I2cHcWrite8 (
  IN  I2C_BUS_INSTANCE                  *Private,
  IN  UINT16                            Offset,
  IN  UINT8                             Data
  );

VOID
I2cHcWrite16 (
  IN  I2C_BUS_INSTANCE                  *Private,
  IN  UINT16                            Offset,
  IN  UINT16                             Data
  );

VOID
I2cHcWrite32 (
  IN  I2C_BUS_INSTANCE                  *Private,
  IN  UINT16                            Offset,
  IN  UINT32                             Data
  );

BOOLEAN
I2cHardwareActive (
  IN  I2C_BUS_INSTANCE                 *Private
  );

EFI_STATUS
TxAbortChk(
  IN I2C_BUS_INSTANCE                  *Private
  );

VOID 
LocalIrqSave(
  OUT UINT32 *eflags
  );

VOID 
LocalIrqRestore(
  IN UINT32  eflags
  );

UINTN
GetTotalChipBus (
  IN UINT32   Cpuid
  );

EFI_STATUS
GetChipInfo(
  IN  I2C_BUS_INSTANCE                     *I2cContext,
  IN  UINTN                                FuncIndex,
  OUT UINT8                                *I2cHcType
  );

VOID
I2cReset (
  IN  I2C_BUS_INSTANCE                 *Private
  );

RETURN_STATUS
I2cStartRequest (
  IN  I2C_BUS_INSTANCE    *Private,
  IN  UINTN               SlaveAddress,
  IN  UINTN               *WriteBytes,
  IN  UINT8               *WriteBuffer,
  IN  UINTN               *ReadBytes,
  OUT UINT8               *ReadBuffer,
  IN  BOOLEAN             RepeatStart
  );

EFI_STATUS
I2cPusAsyncQueue (
  IN  I2C_BUS_INSTANCE            *I2cContext,
  IN UINTN                         SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET        *RequestPacket,
  IN EFI_EVENT                     Event,
  OUT EFI_STATUS                   *I2cStatus
  );

EFI_STATUS
LocateI2cBase(
  IN  I2C_BUS_INSTANCE      *Private,
  OUT EFI_PHYSICAL_ADDRESS  *Base0BackUp,
  OUT UINT32                *PciCmdBackUp
  );

VOID
FreeI2cBase(
  IN I2C_BUS_INSTANCE      *Private,
  IN EFI_PHYSICAL_ADDRESS  Base0Org,
  IN UINT32                PciCmdOrg
  );

//
//  Publish protocol
//
/**
  Set the I2C controller bus clock frequency.

  This routine must be called at or below TPL_NOTIFY.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the controller will use a slightly lower frequency for the I2C to avoid
  exceeding the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the controller would be set to 398
  KHz.  However if the desired frequency is 400 KHz and the controller only
  supports 1 MHz and 100 KHz then this routine would return EFI_UNSUPPORTED.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval EFI_SUCCESS       The bus frequency was set successfully.
  @retval EFI_UNSUPPORTED   The controller does not support this frequency.

**/
EFI_STATUS
EFIAPI
I2cPortSetBusFrequency (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This,
  IN OUT UINTN                       *BusClockHertz
  );

/**
  Reset the I2C controller and configure it for use

  This routine must be called at or below TPL_NOTIFY.

  The controller's I2C bus frequency is set to 100 KHz.

  @param[in]     This       Address of an EFI_I2C_MASTER_PROTOCOL
                            structure

  @retval EFI_SUCCESS       The bus reset was set successfully.

**/
EFI_STATUS
I2cPortReset (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This
  );

/**
  Start an I2C operation on the controller

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  N.B. The typical consumer of this API is the I2C host driver.
  Extreme care must be taken by other consumers of this API to
  prevent confusing the third party I2C drivers due to a state
  change at the I2C device which the third party I2C drivers did
  not initiate.  I2C platform drivers may use this API within
  these guidelines.

  This function initiates an I2C operation on the controller.

  N.B. This API supports only one operation, no queuing support
  exists at this layer.

  The operation is performed by selecting the I2C device with its slave
  address and then sending all write data to the I2C device.  If read data
  is requested, a restart is sent followed by the slave address and then
  the read data is clocked into the I2C controller and placed in the read
  buffer.  When the operation completes, the status value is returned and
  then the event is set.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] SlaveAddress   Address of the device on the I2C bus.
  @param[in] Event          Event to set for asynchronous operations,
                            NULL for synchronous operations
  @param[in] RequestPacket  Address of an EFI_I2C_REQUEST_PACKET
                            structure describing the I2C operation
  @param[out] I2cStatus     Optional buffer to receive the I2C operation
                            completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status pointed to by
                                the request packet.
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cPortMasterStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL   *This,
  IN UINTN                           SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET          *RequestPacket,
  IN EFI_EVENT                       Event      OPTIONAL,
  OUT EFI_STATUS                     *I2cStatus OPTIONAL
  );

/**
  Enable access to an I2C bus configuration.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  Reconfigure the switches and multiplexers in the I2C bus to enable
  access to a specific I2C bus configuration.  Also select the maximum
  clock frequency for this I2C bus configuration.

  This routine uses the I2C Master protocol to perform I2C transactions
  on the local bus.  This eliminates any recursion in the I2C stack for
  configuration transactions on the same I2C bus.  This works because the
  local I2C bus is idle while the I2C bus configuration is being enabled.

  If I2C transactions must be performed on other I2C busses, then the
  EFI_I2C_HOST_PROTOCOL, the EFI_I2C_IO_PROTCOL, or a third party I2C
  driver interface for a specific device must be used.  This requirement
  is because the I2C host protocol controls the flow of requests to the
  I2C controller.  Use the EFI_I2C_HOST_PROTOCOL when the I2C device is
  not enumerated by the EFI_I2C_ENUMERATE_PROTOCOL.  Use a protocol
  produced by a third party driver when it is available or the
  EFI_I2C_IO_PROTOCOL when the third party driver is not available but
  the device is enumerated with the EFI_I2C_ENUMERATE_PROTOCOL.

  When Event is NULL, EnableI2cBusConfiguration operates synchronously
  and returns the I2C completion status as its return value.

  @param[in]  This            Pointer to an EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL
                              structure.
  @param[in]  I2cBusConfiguration Index of an I2C bus configuration.  All
                                  values in the range of zero to N-1 are
                                  valid where N is the total number of I2C
                                  bus configurations for an I2C bus.
  @param[in]  Event           Event to signal when the transaction is complete
  @param[out] I2cStatus       Buffer to receive the transaction status.

  @return  When Event is NULL, EnableI2cBusConfiguration operates synchrouously
  and returns the I2C completion status as its return value.  In this case it is
  recommended to use NULL for I2cStatus.  The values returned from
  EnableI2cBusConfiguration are:

  @retval EFI_SUCCESS           The asynchronous bus configuration request
                                was successfully started when Event is not
                                NULL.
  @retval EFI_SUCCESS           The bus configuration request completed
                                successfully when Event is NULL.
  @retval EFI_DEVICE_ERROR      The bus configuration failed.
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value

**/
EFI_STATUS
EFIAPI
I2cBusConfiguration (
  IN CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *This,
  IN UINTN                                               I2cBusConfiguration,
  IN EFI_EVENT                                           Event      OPTIONAL,
  IN EFI_STATUS                                          *I2cStatus OPTIONAL
  );

#endif
