/** @file
  Macros to simplify and abstract the interface to PCI configuration.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

/**
        1)  may want to split out PMC access vs punit Access vs CSE access stuff into other *.h files
           ie.  PMC stuff can go to SouthPMCAccess.h  or something generic
                Punit stuff can go to NorthPUnitAccess.h or something more generic
                etc.
           We can keep all in one file but should organize and delineate the code by creating headers
           & subheaders for major access sections like:

                =======================================================
                Other Engine (Controller) Communication Interfaces
                IPCs
                =======================================================
                        PUNIT Interfaces:
                =======================================================
                        etc.
                =======================================================
                Standard Access Interfaces
                =======================================================
                        MMIO (Memory Mapped I/O registers)
                =======================================================
                        IO (I/O Mapped registers)
                =======================================================
                        SideBand (IOSF 2.0 Mapped registers)
                =======================================================
                        etc.
                =======================================================
**/
#ifndef _SAACCESS_H_
#define _SAACCESS_H_

#include "SaRegs.h"
#include "SaCommonDefinitions.h"
#include <Library/IoLib.h>
#include "PlatformBaseAddresses.h"


//=============================================================================
//              Other Engine (Contoller) Communication Interfaces
//              IPCs
//=============================================================================
//                      PUNIT Interfaces:
//=============================================================================
// Punit mailbox reg offset
#define BIOS_MAILBOX_INTERFACE  0x5DA0
#define MAILBOX_BIOS_CMD_READ_PCS  0x80000001
#define MAILBOX_BIOS_CMD_WRITE_PCS 0x80000002
#define MAILBOX_BIOS_CMD_ADDR_MASK 0x1fffff00 // Bit 8~28

#define BIOS_MAILBOX_DATA   0x5DA4

/**
  Punit Mailbox Read

  PcsAddr      - PCS indexed addr in Punit mailbox
  Data         - Variable to store the value read from Punit
**/
#define PunitMailboxRead(PcsAddr, Data) \
{ \
  *(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_DATA) = 0;\
  *(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_INTERFACE) = MAILBOX_BIOS_CMD_READ_PCS | (MAILBOX_BIOS_CMD_ADDR_MASK & (PcsAddr << 8)); \
  while((*(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_INTERFACE)) & 0x80000000); \
  Data = *(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_INTERFACE); \
}

/**
  Punit Mailbox Write

  PcsAddr  - PCS indexed addr in Punit mailbox
  Data     - Value write to Punit via mailbox
**/
#define PunitMailboxWrite(PcsAddr, Data)  \
{ \
  *(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_DATA) = Data; \
  *(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_INTERFACE) = MAILBOX_BIOS_CMD_WRITE_PCS | (MAILBOX_BIOS_CMD_ADDR_MASK & (PcsAddr << 8));\
  while((*(UINT32 *)(MCH_BASE_ADDRESS + BIOS_MAILBOX_INTERFACE)) & 0x80000000); \
}

//=============================================================================
//              Standard Access Interfaces
//=============================================================================
//                      MMIO (Memory Mapped I/O registers)
//=============================================================================
//
// Memory Mapped IO access macros used by MSG BUS LIBRARY
//
#define MmioAddress( BaseAddr, Register ) \
  ( (UINTN)BaseAddr + \
    (UINTN)(Register) \
  )

//
// UINT64
//

#define Mmio64Ptr( BaseAddr, Register ) \
  ( (volatile UINT64 *)MmioAddress( BaseAddr, Register ) )

#define Mmio64( BaseAddr, Register ) \
  *Mmio64Ptr( BaseAddr, Register )

#define Mmio64Or( BaseAddr, Register, OrData ) \
  Mmio64( BaseAddr, Register ) = \
    (UINT64) ( \
      Mmio64( BaseAddr, Register ) | \
      (UINT64)(OrData) \
    )

#define Mmio64And( BaseAddr, Register, AndData ) \
  Mmio64( BaseAddr, Register ) = \
    (UINT64) ( \
      Mmio64( BaseAddr, Register ) & \
      (UINT64)(AndData) \
    )

#define Mmio64AndThenOr( BaseAddr, Register, AndData, OrData ) \
  Mmio64( BaseAddr, Register ) = \
    (UINT64) ( \
      ( Mmio64( BaseAddr, Register ) & \
          (UINT64)(AndData) \
      ) | \
      (UINT64)(OrData) \
    )

//
// UINT32
//

#define Mmio32Ptr( BaseAddr, Register ) \
  ( (volatile UINT32 *)MmioAddress( BaseAddr, Register ) )

#define Mmio32( BaseAddr, Register ) \
  *Mmio32Ptr( BaseAddr, Register )

#define Mmio32Or( BaseAddr, Register, OrData ) \
  Mmio32( BaseAddr, Register ) = \
    (UINT32) ( \
      Mmio32( BaseAddr, Register ) | \
      (UINT32)(OrData) \
    )

#define Mmio32And( BaseAddr, Register, AndData ) \
  Mmio32( BaseAddr, Register ) = \
    (UINT32) ( \
      Mmio32( BaseAddr, Register ) & \
      (UINT32)(AndData) \
    )

#define Mmio32AndThenOr( BaseAddr, Register, AndData, OrData ) \
  Mmio32( BaseAddr, Register ) = \
    (UINT32) ( \
      ( Mmio32( BaseAddr, Register ) & \
          (UINT32)(AndData) \
      ) | \
      (UINT32)(OrData) \
    )

//
// UINT16
//

#define Mmio16Ptr( BaseAddr, Register ) \
  ( (volatile UINT16 *)MmioAddress( BaseAddr, Register ) )

#define Mmio16( BaseAddr, Register ) \
  *Mmio16Ptr( BaseAddr, Register )

#define Mmio16Or( BaseAddr, Register, OrData ) \
  Mmio16( BaseAddr, Register ) = \
    (UINT16) ( \
      Mmio16( BaseAddr, Register ) | \
      (UINT16)(OrData) \
    )

#define Mmio16And( BaseAddr, Register, AndData ) \
  Mmio16( BaseAddr, Register ) = \
    (UINT16) ( \
      Mmio16( BaseAddr, Register ) & \
      (UINT16)(AndData) \
    )

#define Mmio16AndThenOr( BaseAddr, Register, AndData, OrData ) \
  Mmio16( BaseAddr, Register ) = \
    (UINT16) ( \
      ( Mmio16( BaseAddr, Register ) & \
          (UINT16)(AndData) \
      ) | \
      (UINT16)(OrData) \
    )

//
// UINT8
//

#define Mmio8Ptr( BaseAddr, Register ) \
  ( (volatile UINT8 *)MmioAddress( BaseAddr, Register ) )

#define Mmio8( BaseAddr, Register ) \
  *Mmio8Ptr( BaseAddr, Register )

#define Mmio8Or( BaseAddr, Register, OrData ) \
  Mmio8( BaseAddr, Register ) = \
    (UINT8) ( \
      Mmio8( BaseAddr, Register ) | \
      (UINT8)(OrData) \
    )

#define Mmio8And( BaseAddr, Register, AndData ) \
  Mmio8( BaseAddr, Register ) = \
    (UINT8) ( \
      Mmio8( BaseAddr, Register ) & \
      (UINT8)(AndData) \
    )

#define Mmio8AndThenOr( BaseAddr, Register, AndData, OrData ) \
  Mmio8( BaseAddr, Register ) = \
    (UINT8) ( \
      ( Mmio8( BaseAddr, Register ) & \
          (UINT8)(AndData) \
        ) | \
      (UINT8)(OrData) \
    )

/**
  All sideband access has moved to SideBandLib in BxtSocRefCodePkg/BroxtonSoc/Library/SideBandLib.

  Any sideband read and write operations should be performed by calling SideBandRead32() and
  SideBandWrite32() which will internally determine whether to access the sideband private
  configuration registers via MMIO or Sideband Message Interface based on whether the P2SB
  BAR (SBREG_BAR) is set.
**/

#define N_PCICFGCTRL_PCI_IRQ    20       ///< Configure DirectIRQ# for PCI mode
#define N_PCICFGCTRL_ACPI_IRQ   12       ///< Configure DirectIRQ# for ACPI mode
#define N_PCICFGCTRL_INT_PIN    8        ///< Configure PIN# for legacy mode
#define V_PCICFG_CTRL_NONE          0
#define V_PCICFG_CTRL_INTA          1
#define V_PCICFG_CTRL_INTB          2
#define V_PCICFG_CTRL_INTC          3
#define V_PCICFG_CTRL_INTD          4
//
//=============================================================================
//                      MM Pci Cfg (Memory Mapped Pci Cfg Space registers)
//=============================================================================
//
// Memory mapped PCI IO
//

#define PciCfgPtr(Bus, Device, Function, Register )\
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register)

#define PciCfg32Read_CF8CFC(B,D,F,R) \
  (UINT32)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn32(0xCFC))

#define PciCfg32Write_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut32(0xCFC,Data))

#define PciCfg32Or_CF8CFC(B,D,F,R,O) \
  PciCfg32Write_CF8CFC(B,D,F,R, \
    (PciCfg32Read_CF8CFC(B,D,F,R) | (O)))

#define PciCfg32And_CF8CFC(B,D,F,R,A) \
  PciCfg32Write_CF8CFC(B,D,F,R, \
    (PciCfg32Read_CF8CFC(B,D,F,R) & (A)))

#define PciCfg32AndThenOr_CF8CFC(B,D,F,R,A,O) \
  PciCfg32Write_CF8CFC(B,D,F,R, \
    (PciCfg32Read_CF8CFC(B,D,F,R) & (A)) | (O))
//
//=============================================================================
//                      MM Pci Cfg For Commonly Used Devices
//                      (Memory Mapped Pci Cfg Space registers)
//=============================================================================
//
// Device 0, Function 0
//
#define McD0PciCfg64(Register)                              MmPci64           (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg64Or(Register, OrData)                    MmPci64Or         (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg64And(Register, AndData)                  MmPci64And        (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg64AndThenOr(Register, AndData, OrData)    MmPci64AndThenOr  (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

#define McD0PciCfg32(Register)                              MmPci32           (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg32Or(Register, OrData)                    MmPci32Or         (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg32And(Register, AndData)                  MmPci32And        (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg32AndThenOr(Register, AndData, OrData)    MmPci32AndThenOr  (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

#define McD0PciCfg16(Register)                              MmPci16           (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg16Or(Register, OrData)                    MmPci16Or         (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg16And(Register, AndData)                  MmPci16And        (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg16AndThenOr(Register, AndData, OrData)    MmPci16AndThenOr  (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

#define McD0PciCfg8(Register)                               MmPci8            (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg8Or(Register, OrData)                     MmPci8Or          (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg8And(Register, AndData)                   MmPci8And         (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg8AndThenOr( Register, AndData, OrData )   MmPci8AndThenOr   (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)


//
// Device 2, Function 0
//
#define McD2PciCfg64(Register)                              MmPci64           (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg64Or(Register, OrData)                    MmPci64Or         (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg64And(Register, AndData)                  MmPci64And        (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg64AndThenOr(Register, AndData, OrData)    MmPci64AndThenOr  (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg32(Register)                              MmPci32           (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg32Or(Register, OrData)                    MmPci32Or         (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg32And(Register, AndData)                  MmPci32And        (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg32AndThenOr(Register, AndData, OrData)    MmPci32AndThenOr  (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg16(Register)                              MmPci16           (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg16Or(Register, OrData)                    MmPci16Or         (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg16And(Register, AndData)                  MmPci16And        (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg16AndThenOr(Register, AndData, OrData)    MmPci16AndThenOr  (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg8(Register)                               MmPci8            (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg8Or(Register, OrData)                     MmPci8Or          (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg8And(Register, AndData)                   MmPci8And         (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg8AndThenOr(Register, AndData, OrData)     MmPci8AndThenOr   (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)
//
//=============================================================================
//                      GCC I/O (I/O Space registers)
//=============================================================================
//
// IO
//

#ifndef IoIn8
//
//=============================================================================
//                      Generic I/O (I/O Space registers)
//=============================================================================
#define IoIn8(Port) \
  IoRead8(Port)

#define IoIn16(Port) \
  IoRead16(Port)

#define IoIn32(Port) \
  IoRead32(Port)

#define IoOut8(Port, Data) \
  IoWrite8(Port, Data)

#define IoOut16(Port, Data) \
  IoWrite16(Port, Data)

#define IoOut32(Port, Data) \
  IoWrite32(Port, Data)

#endif

#endif
