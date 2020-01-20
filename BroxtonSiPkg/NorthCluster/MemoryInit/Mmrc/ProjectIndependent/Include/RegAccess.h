/** @file
  This file include register access.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification Reference:
**/

#ifndef _REGACCESS_H_
#define _REGACCESS_H_

#include "Mmrc.h"
#include "MmrcProjectData.h"

#if defined __GNUC__
#include <Library/IoLib.h>
#endif

#if SIM
#include "vpi_user.h"
#include "mrc_wrapper.h"
#endif
#if SUSSW
#include "Susapi.h"
#endif

typedef enum {
  NoError,
  InvalidInstance,
  InvalidBoxType,
  InvalidMode,
  BoxTypeNotInSimulation,
  InvalidRemapType,
  UnknownError
} ACCESS_ERROR_TYPE;

typedef enum {
  ModeRead,
  ModeWrite,
  ModeOther
} ACCESS_MODE;

#if SIM || JTAG
#define MmioCacheFlush()

#define Mmio32Read(Register)            0
#define Mmio32Write(Register, Value)

#define Mmio16Read(Register)            0
#define Mmio16Write(Register, Value)

#define Mmio8Read(Register)             0
#define Mmio8Write(Register, Value)
#else
//
// Memory Mapped IO
//
/**
  Mmio read 32 bytes.

  @param[in]       RegisterAddress

  @return     Value read
**/
UINT32
Mmio32Read (
  IN        UINT32      RegisterAddress
)
;

/**
  Mmio write 32 bytes.

  @param[in]       RegisterAddress
  @param[in]       Value

  @return     None
**/
VOID
Mmio32Write (
  IN        UINT32      RegisterAddress,
  IN        UINT32      Value
)
;

/**
  Mmio read 16 bytes.

  @param[in]       RegisterAddress

  @return     Value read
**/
UINT16
Mmio16Read (
  IN        UINT32      RegisterAddress
)
;

/**
  Mmio write 16 bytes.

  @param[in]       RegisterAddress
  @param[in]       Value

  @return     None
**/
VOID
Mmio16Write (
  IN        UINT32      RegisterAddress,
  IN        UINT16      Value
)
;

/**
  Mmio read 8 bytes.

  @param[in]       RegisterAddress

  @return     Value read
**/
UINT8
Mmio8Read (
  IN        UINT32      RegisterAddress
)
;

/**
  Mmio write 8 bytes.

  @param[in]       RegisterAddress
  @param[in]       Value

  @return     None
**/
VOID
Mmio8Write (
  IN        UINT32      RegisterAddress,
  IN        UINT8       Value
)
;

#endif


#ifndef Mmio32Or
#define Mmio32Or(Register, OrData)    Mmio32Write (Register, Mmio32Read (Register) | OrData)
#define Mmio16Or(Register, OrData)    Mmio16Write (Register, Mmio16Read (Register) | OrData)
#define Mmio8Or(Register, OrData)     Mmio8Write (Register, Mmio8Read (Register) | OrData)

#define Mmio32And(Register, AndData)  Mmio32Write (Register, Mmio32Read (Register) & (AndData))
#define Mmio16And(Register, AndData)  Mmio16Write (Register, Mmio16Read (Register) & (AndData))
#define Mmio8And(Register, AndData)   Mmio8Write (Register, Mmio8Read (Register) & (AndData))

#define Mmio32AndThenOr(Register, AndData, OrData)  Mmio32Write (Register, (((Mmio32Read (Register)& (AndData))) | OrData))
#define Mmio16AndThenOr(Register, AndData, OrData)  Mmio16Write (Register, (((Mmio16Read (Register)& (AndData))) | OrData))
#define Mmio8AndThenOr(Register, AndData, OrData)   Mmio8Write (Register, (((Mmio8Read (Register)& (AndData))) | OrData))
#endif

//
// Memory mapped PCI IO
//
#define PCI_CFG_PTR(Bus, Device, Function, Register )\
    ((UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register))

#define PCI_CFG_32B_READ_CF8CFC(B,D,F,R)\
  (UINT32)(IoOut32 (0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn32 (0xCFC))

#define PCI_CFG_32B_WRITE_CF8CFC(B,D,F,R,Data) \
  (IoOut32 (0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut32 (0xCFC,Data))

#define PCI_CFG_32B_AND_THEN_OR_CF8CFC(B,D,F,R,A,O) \
  PCI_CFG_32B_WRITE_CF8CFC (B,D,F,R, \
    (PCI_CFG_32B_READ_CF8CFC (B,D,F,R) & (A)) | (O))

#define PCI_CFG_16B_READ_CF8CFC(B,D,F,R) \
  (UINT16)(IoOut32 (0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn16 (0xCFC))

#define PCI_CFG_16B_WRITE_CF8CFC(B,D,F,R,Data) \
  (IoOut32 (0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut16 (0xCFC,Data))

#define PCI_CFG_16B_AND_THEN_OR_CF8CFC(B,D,F,R,A,O) \
  PCI_CFG_16B_WRITE_CF8CFC (B,D,F,R, \
    (PCI_CFG_16B_READ_CF8CFC (B,D,F,R) & (A)) | (O))

#define PCI_CFG_8B_READ_CF8CFC(B,D,F,R) \
  (UINT8)(IoOut32 (0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn8 (0xCFC))

#define PCI_CFG_8B_WRITE_CF8CFC(B,D,F,R,Data) \
  (IoOut32 (0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut8 (0xCFC,Data))

#define PCI_CFG_8B_AND_THEN_OR_CF8CFC(B,D,F,R,A,O) \
  PCI_CFG_8B_WRITE_CF8CFC (B,D,F,R, \
    (PCI_CFG_8B_READ_CF8CFC (B,D,F,R) & (A)) | (O))


#define PCI_CFG_32B_READ(PciExpressBase, Bus, Device, Function, Register) \
  Mmio32Read (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register))

#define PCI_CFG_32B_WRITE(PciExpressBase, Bus, Device, Function, Register, Value) \
  Mmio32Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    Value)

#define PCI_CFG_16B_READ(PciExpressBase, Bus, Device, Function, Register) \
  Mmio16Read (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register))

#define PCI_CFG_16B_WRITE(PciExpressBase, Bus, Device, Function, Register, Value) \
  Mmio16Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    Value)

#define PCI_CFG_8B_READ(PciExpressBase, Bus, Device, Function, Register) \
  Mmio8Read (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register))

#define PCI_CFG_8B_WRITE(PciExpressBase, Bus, Device, Function, Register, Value) \
  Mmio8Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    Value)

#define PCI_CFG_32B_OR(PciExpressBase, Bus, Device, Function, Register, OrValue) \
  Mmio32Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    (PCI_CFG_32B_READ (PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PCI_CFG_32B_AND(PciExpressBase, Bus, Device, Function, Register, AndValue) \
  Mmio32Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    (PCI_CFG_32B_READ (PciExpressBase, Bus, Device, Function, Register)& (AndValue)))

#define PCI_CFG_16B_OR(PciExpressBase, Bus, Device, Function, Register, OrValue) \
  Mmio16Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    (PCI_CFG_16B_READ(PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PCI_CFG_16B_AND(PciExpressBase, Bus, Device, Function, Register, AndValue) \
  Mmio16Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    (PCI_CFG_16B_READ (PciExpressBase, Bus, Device, Function, Register)& (AndValue)))

#define PCI_CFG_8B_OR(PciExpressBase, Bus, Device, Function, Register, OrValue) \
  Mmio8Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    (PCI_CFG_8B_READ (PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PCI_CFG_8B_AND(PciExpressBase, Bus, Device, Function, Register, AndValue) \
  Mmio8Write (PciExpressBase + \
    (UINT32)(Bus << 20) + \
    (UINT32)(Device << 15) + \
    (UINT32)(Function << 12) + \
    (UINT32)(Register), \
    (PCI_CFG_8B_READ (PciExpressBase, Bus, Device, Function, Register)& (AndValue)))


//
// Read Message Register
//
#define MSG_BUS_32B_READ(portid,offset,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, 0); \
  Mmio32Write (EC_BASE + 0xD0, ((0x06000000) | (portid <<16)| ((offset) << 8) + 0xF0))   ; \
  (data) = Mmio32Read (EC_BASE + 0xD4);\
}

//
// Write Message Register
//
#define MSG_BUS_32B_WRITE(portid,offset,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, 0); \
  Mmio32Write (EC_BASE + 0xD4, data)   ; \
  Mmio32Write (EC_BASE + 0xD0, ((0x07000000) | (portid <<16)| ((offset) << 8) + 0xF0)); \
}

//
// Read Message Register with Offset Hi
//
#define PSF_MSG_BUS_32B_READ(portid,offsethi,offsetlo,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, offsethi); \
  Mmio32Write (EC_BASE + 0xD0, ((0x06000000) | (portid <<16)| ((offsetlo) << 8) + 0xF0))   ; \
  (data) = Mmio32Read (EC_BASE + 0xD4); \
}

//
// Write Message Register with Offset Hi
//
#define PSF_MSG_BUS_32B_WRITE(portid,offsethi,offsetlo,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, offsethi); \
  Mmio32Write (EC_BASE + 0xD4, data)   ; \
  Mmio32Write (EC_BASE + 0xD0, ((0x07000000) | (portid <<16)| ((offsetlo) << 8) + 0xF0)); \
}

/**
  Get register access information.

  @param[in]       BoxType
  @param[in]       Channel
  @param[in]       Instance
  @param[in,out]   Command
  @param[in,out]   PortId
  @param[in,out]   Offset
  @param[in,out]   Bus
  @param[in,out]   Device
  @param[in,out]   Func
  @param[in]       Mode

  @return     MMRC_STATUS
**/
UINT32
GetRegisterAccessInfo (
  IN        UINT8       BoxType,
  IN        UINT8       Channel,
  IN        UINT8       Instance,
  IN  OUT   UINT32      *Command,
  IN  OUT   UINT8       *PortId,
  IN  OUT   UINT32      *Offset,
  IN  OUT   UINT8       *Bus,
  IN  OUT   UINT8       *Device,
  IN  OUT   UINT8       *Func,
  IN        UINT32      Mode
);

/**
  Reads registers from a specified Unit

  @param[in]       BoxType
  @param[in]       Channel
  @param[in]       Instance
  @param[in,out]   Offset

  @return     Value read
**/
UINTX
MemRegRead (
  IN        UINT8           BoxType,
  IN        UINT8           Channel,
  IN        UINT8           Instance,
  IN        REGISTER_ACCESS Offset
)
;

/**
  Writes registers to a specified Unit

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in]       Offset       Offset of register to write.
  @param[in]       Data         Data to be written
  @param[in]       Be

  @return     None
**/
VOID
MemRegWrite (
  IN        UINT8           BoxType,
  IN        UINT8           Channel,
  IN        UINT8           Instance,
  IN        REGISTER_ACCESS Offset,
  IN        UINTX           Data,
  IN        UINT8           Be
)
;

/**
  Read memory field.

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in]       Register

  @return     Value read
**/
UINTX
MemFieldRead (
              IN        UINT8           BoxType,
              IN        UINT8           Channel,
              IN        UINT8           Instance,
              IN        REGISTER_ACCESS Register
)
;

/**
  Write memory field.

  @param[in]       BoxType      Unit to select
  @param[in]       Channel
  @param[in]       Instance     Channel under test
  @param[in]       Register
  @param[in]       Value
  @param[in]       Be

  @return     Value read
**/
VOID
MemFieldWrite (
               IN        UINT8           BoxType,
               IN        UINT8           Channel,
               IN        UINT8           Instance,
               IN        REGISTER_ACCESS Register,
               IN        UINTX           Value,
               IN        UINT8           Be
)
;
//
// IO
//
#if defined SIM || defined JTAG
#define IoIn8(Port)         MySimIoRead (0x1, Port)
#define IoIn16(Port)        MySimIoRead (0x3, Port)
#define IoIn32(Port)        MySimIoRead (0xf, Port)
#define IoOut8(Port, Data)  MySimIoWrite (0x1, Port, Data)
#define IoOut16(Port, Data) MySimIoWrite (0x3, Port, Data)
#define IoOut32(Port, Data) MySimIoWrite (0xf, Port, Data)
#else
//
// GCC compiler
//
#if defined __GNUC__
#ifndef IoIn8
#define IoIn8(Port)    IoRead8(Port)

#define IoIn16(Port)   IoRead16(Port)

#define IoIn32(Port)   IoRead32(Port)

#define IoOut8(Port, Data)   IoWrite8(Port, Data)

#define IoOut16(Port, Data)  IoWrite16(Port, Data)

#define IoOut32(Port, Data)  IoWrite32(Port, Data)
#endif // IoIn8
//
//MSFT compiler
//
#else
#pragma intrinsic(_inp, _outp, _inpw, _inpd, _outpd)
#ifndef IoIn8
#define IoIn8(Port)         _inp (Port)
#define IoIn16(Port)        _inpw (Port)

#define IoIn32(Port)        _inpd (Port)

#define IoOut8(Port, Data)  _outp (Port, Data)

#define IoOut16(Port, Data) _outpw (Port, Data)

#define IoOut32(Port, Data) _outpd (Port, Data)
#endif // IoIn8
#endif
#endif // SIM

#endif // _REGACCESS_H

