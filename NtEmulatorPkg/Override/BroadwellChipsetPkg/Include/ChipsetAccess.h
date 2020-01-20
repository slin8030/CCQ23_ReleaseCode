/** @file

;******************************************************************************
;* Copyright (c) 1999 - 2013, Insyde Software Corporation. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement

@file:

  Cpu.h

@brief:

  Various CPU-specific definitions.

**/
#ifndef _CHIPSET_PCH_ACCESS_H_
#define _CHIPSET_PCH_ACCESS_H_

#include <Library/PciExpressLib.h>
#include <Library/IoLib.h>
#include <ScAccess.h>

///
/// Memory Mapped PCI Access macros
///
///
/// PCI Device MM Base
///
#ifndef MmPciAddress
#define MmPciAddress(Segment, Bus, Device, Function, Register) \
  ((UINTN) (PciExpressRead32 (PCI_EXPRESS_LIB_ADDRESS (0,0,0,0x60)) & 0xFC000000) + \
   (UINTN) (Bus << 20) + \
   (UINTN) (Device << 15) + \
   (UINTN) (Function << 12) + \
   (UINTN) (Register) \
  )
#endif
///
/// Pch Controller PCI access macros
///
#define PCH_RCRB_BASE ( \
  MmioRead32 (MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  R_LPC_RCBA)) &~BIT0 \
  )

//
// Device 0x1b, Function 0
//
#define PchAzaliaPciCfg32(Register) \
  MmioRead32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register) \
  )

#define PchAzaliaPciCfg32Or(Register, OrData) \
  MmioOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  OrData \
  )

#define PchAzaliaPciCfg32And(Register, AndData) \
  MmioAnd32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  AndData \
  )

#define PchAzaliaPciCfg32AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  OrData \
  )

#define PchAzaliaPciCfg16(Register) \
  MmioRead16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register) \
  )

#define PchAzaliaPciCfg16Or(Register, OrData) \
  MmioOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  OrData \
  )

#define PchAzaliaPciCfg16And(Register, AndData) \
  MmioAnd16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  AndData \
  )

#define PchAzaliaPciCfg16AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  AndData, \
  OrData \
  )

#define PchAzaliaPciCfg8(Register)  MmioRead8 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_AZALIA, 0, Register))

#define PchAzaliaPciCfg8Or(Register, OrData) \
  MmioOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  OrData \
  )

#define PchAzaliaPciCfg8And(Register, AndData) \
  MmioAnd8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  AndData \
  )

#define PchAzaliaPciCfg8AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_AZALIA, \
  0, \
  Register), \
  AndData, \
  OrData \
  )

//
// Device 0x1f, Function 0
//
#define PchLpcPciCfg32(Register)  MmioRead32 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, Register))

#define PchLpcPciCfg32Or(Register, OrData) \
  MmioOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  OrData \
  )

#define PchLpcPciCfg32And(Register, AndData) \
  MmioAnd32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  AndData \
  )

#define PchLpcPciCfg32AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  AndData, \
  OrData \
  )

#define PchLpcPciCfg16(Register)  MmioRead16 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, Register))

#define PchLpcPciCfg16Or(Register, OrData) \
  MmioOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  OrData \
  )

#define PchLpcPciCfg16And(Register, AndData) \
  MmioAnd16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  AndData \
  )

#define PchLpcPciCfg16AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  AndData, \
  OrData \
  )

#define PchLpcPciCfg8(Register) MmioRead8 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, Register))

#define PchLpcPciCfg8Or(Register, OrData) \
  MmioOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  OrData \
  )

#define PchLpcPciCfg8And(Register, AndData) \
  MmioAnd8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  AndData \
  )

#define PchLpcPciCfg8AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_PCH_LPC, \
  0, \
  Register), \
  AndData, \
  OrData \
  )

//
// SATA 1 device 0x1f, Function 2
//
#define PchSataPciCfg32(Register) MmioRead32 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_SATA, 2, Register))

#define PchSataPciCfg32Or(Register, OrData) \
  MmioOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  OrData \
  )

#define PchSataPciCfg32And(Register, AndData) \
  MmioAnd32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  AndData \
  )

#define PchSataPciCfg32AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  AndData, \
  OrData \
  )

#define PchSataPciCfg16(Register) MmioRead16 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_SATA, 2, Register))

#define PchSataPciCfg16Or(Register, OrData) \
  MmioOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  OrData \
  )

#define PchSataPciCfg16And(Register, AndData) \
  MmioAnd16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  AndData \
  )

#define PchSataPciCfg16AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  AndData, \
  OrData \
  )

#define PchSataPciCfg8(Register)  MmioRead8 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_SATA, 2, Register))

#define PchSataPciCfg8Or(Register, OrData) \
  MmioOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  OrData \
  )

#define PchSataPciCfg8And(Register, AndData) \
  MmioAnd8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  AndData \
  )

#define PchSataPciCfg8AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA, \
  2, \
  Register), \
  AndData, \
  OrData \
  )

//
// SATA 2 device 0x1f, Function 5
//
#define PchSata2PciCfg32(Register)  MmioRead32 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_SATA2, 5, Register))

#define PchSata2PciCfg32Or(Register, OrData) \
  MmioOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  OrData \
  )

#define PchSata2PciCfg32And(Register, AndData) \
  MmioAnd32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  AndData \
  )

#define PchSata2PciCfg32AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr32 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  AndData, \
  OrData \
  )

#define PchSata2PciCfg16(Register)  MmioRead16 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_SATA2, 5, Register))

#define PchSata2PciCfg16Or(Register, OrData) \
  MmioOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  OrData \
  )

#define PchSata2PciCfg16And(Register, AndData) \
  MmioAnd16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  AndData \
  )

#define PchSata2PciCfg16AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr16 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  AndData, \
  OrData \
  )

#define PchSata2PciCfg8(Register) MmioRead8 (MmPciAddress (DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_SATA2, 5, Register))

#define PchSata2PciCfg8Or(Register, OrData) \
  MmioOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  OrData \
  )

#define PchSata2PciCfg8And(Register, AndData) \
  MmioAnd8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  AndData \
  )

#define PchSata2PciCfg8AndThenOr(Register, AndData, OrData) \
  MmioAndThenOr8 ( \
  MmPciAddress (0, \
  DEFAULT_PCI_BUS_NUMBER_PCH, \
  PCI_DEVICE_NUMBER_SATA2, \
  5, \
  Register), \
  AndData, \
  OrData \
  )

//
// Root Complex Register Block
//
#define PchMmRcrb32(Register)                           MmioRead32 (PCH_RCRB_BASE + Register)

#define PchMmRcrb32Or(Register, OrData)                 MmioOr32 (PCH_RCRB_BASE + Register, OrData)

#define PchMmRcrb32And(Register, AndData)               MmioAnd32 (PCH_RCRB_BASE + Register, AndData)

#define PchMmRcrb32AndThenOr(Register, AndData, OrData) MmioAndThenOr32 (PCH_RCRB_BASE + Register, AndData, OrData)

#define PchMmRcrb16(Register)                           MmioRead16 (PCH_RCRB_BASE + Register)

#define PchMmRcrb16Or(Register, OrData)                 MmioOr16 (PCH_RCRB_BASE + Register, OrData)

#define PchMmRcrb16And(Register, AndData)               MmioAnd16 (PCH_RCRB_BASE + Register, AndData)

#define PchMmRcrb16AndThenOr(Register, AndData, OrData) MmioAndThenOr16 (PCH_RCRB_BASE + Register, AndData, OrData)

#define PchMmRcrb8(Register)                            MmioRead8 (PCH_RCRB_BASE + Register)

#define PchMmRcrb8Or(Register, OrData)                  MmioOr8 (PCH_RCRB_BASE + Register, OrData)

#define PchMmRcrb8And(Register, AndData)                MmioAnd8 (PCH_RCRB_BASE + Register, AndData)

#define PchMmRcrb8AndThenOr(Register, AndData, OrData)  MmioAndThenOr8 (PCH_RCRB_BASE + Register, AndData, OrData)

///
/// System Agent PCI access macros
///
///
/// Device #, Function #
///
#define McDevFunPciCfg64(Bus, Dev, Func, Register)              MmPci64 (0, Bus, Dev, Func, Register)
#define McDevFunPciCfg64Or(Bus, Dev, Func, Register, OrData)    MmPci64Or (0, Bus, Dev, Func, Register, OrData)
#define McDevFunPciCfg64And(Bus, Dev, Func, Register, AndData)  MmPci64And (0, Bus, Dev, Func, Register, AndData)
#define McDevFunPciCfg64AndThenOr(Bus, Dev, Func, Register, AndData, OrData) \
  MmPci64AndThenOr (0, \
                    Bus, \
                    Dev, \
                    Func, \
                    Register, \
                    AndData, \
                    OrData \
      )

#define McDevFunPciCfg32(Bus, Dev, Func, Register)              MmPci32 (0, Bus, Dev, Func, Register)
#define McDevFunPciCfg32Or(Bus, Dev, Func, Register, OrData)    MmPci32Or (0, Bus, Dev, Func, Register, OrData)
#define McDevFunPciCfg32And(Bus, Dev, Func, Register, AndData)  MmPci32And (0, Bus, Dev, Func, Register, AndData)
#define McDevFunPciCfg32AndThenOr(Bus, Dev, Func, Register, AndData, OrData) \
  MmPci32AndThenOr (0, \
                    Bus, \
                    Dev, \
                    Func, \
                    Register, \
                    AndData, \
                    OrData \
      )

#define McDevFunPciCfg16(Bus, Dev, Func, Register)              MmPci16 (0, Bus, Dev, Func, Register)
#define McDevFunPciCfg16Or(Bus, Dev, Func, Register, OrData)    MmPci16Or (0, Bus, Dev, Func, Register, OrData)
#define McDevFunPciCfg16And(Bus, Dev, Func, Register, AndData)  MmPci16And (0, Bus, Dev, Func, Register, AndData)
#define McDevFunPciCfg16AndThenOr(Bus, Dev, Func, Register, AndData, OrData) \
  MmPci16AndThenOr (0, \
                    Bus, \
                    Dev, \
                    Func, \
                    Register, \
                    AndData, \
                    OrData \
      )

#define McDevFunPciCfg8(Bus, Dev, Func, Register)             MmPci8 (0, Bus, Dev, Func, Register)
#define McDevFunPciCfg8Or(Bus, Dev, Func, Register, OrData)   MmPci8Or (0, Bus, Dev, Func, Register, OrData)
#define McDevFunPciCfg8And(Bus, Dev, Func, Register, AndData) MmPci8And (0, Bus, Dev, Func, Register, AndData)
#define McDevFunPciCfg8AndThenOr(Bus, Dev, Func, Register, AndData, OrData) \
  MmPci8AndThenOr (0, \
                   Bus, \
                   Dev, \
                   Func, \
                   Register, \
                   AndData, \
                   OrData \
      )

///
/// Device 0, Function 0
///
#define McD0PciCfg64(Register)                            MmPci64 (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg64Or(Register, OrData)                  MmPci64Or (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg64And(Register, AndData)                MmPci64And (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg64AndThenOr(Register, AndData, OrData)  MmPci64AndThenOr (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

#define McD0PciCfg32(Register)                            MmPci32 (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg32Or(Register, OrData)                  MmPci32Or (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg32And(Register, AndData)                MmPci32And (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg32AndThenOr(Register, AndData, OrData)  MmPci32AndThenOr (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

#define McD0PciCfg16(Register)                            MmPci16 (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg16Or(Register, OrData)                  MmPci16Or (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg16And(Register, AndData)                MmPci16And (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg16AndThenOr(Register, AndData, OrData)  MmPci16AndThenOr (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

#define McD0PciCfg8(Register)                             MmPci8 (0, SA_MC_BUS, 0, 0, Register)
#define McD0PciCfg8Or(Register, OrData)                   MmPci8Or (0, SA_MC_BUS, 0, 0, Register, OrData)
#define McD0PciCfg8And(Register, AndData)                 MmPci8And (0, SA_MC_BUS, 0, 0, Register, AndData)
#define McD0PciCfg8AndThenOr(Register, AndData, OrData)   MmPci8AndThenOr (0, SA_MC_BUS, 0, 0, Register, AndData, OrData)

///
/// Device 1, Function 0
///
#define McD1PciCfg64(Register)                            MmPci64 (0, SA_MC_BUS, 1, 0, Register)
#define McD1PciCfg64Or(Register, OrData)                  MmPci64Or (0, SA_MC_BUS, 1, 0, Register, OrData)
#define McD1PciCfg64And(Register, AndData)                MmPci64And (0, SA_MC_BUS, 1, 0, Register, AndData)
#define McD1PciCfg64AndThenOr(Register, AndData, OrData)  MmPci64AndThenOr (0, SA_MC_BUS, 1, 0, Register, AndData, OrData)

#define McD1PciCfg32(Register)                            MmPci32 (0, SA_MC_BUS, 1, 0, Register)
#define McD1PciCfg32Or(Register, OrData)                  MmPci32Or (0, SA_MC_BUS, 1, 0, Register, OrData)
#define McD1PciCfg32And(Register, AndData)                MmPci32And (0, SA_MC_BUS, 1, 0, Register, AndData)
#define McD1PciCfg32AndThenOr(Register, AndData, OrData)  MmPci32AndThenOr (0, SA_MC_BUS, 1, 0, Register, AndData, OrData)

#define McD1PciCfg16(Register)                            MmPci16 (0, SA_MC_BUS, 1, 0, Register)
#define McD1PciCfg16Or(Register, OrData)                  MmPci16Or (0, SA_MC_BUS, 1, 0, Register, OrData)
#define McD1PciCfg16And(Register, AndData)                MmPci16And (0, SA_MC_BUS, 1, 0, Register, AndData)
#define McD1PciCfg16AndThenOr(Register, AndData, OrData)  MmPci16AndThenOr (0, SA_MC_BUS, 1, 0, Register, AndData, OrData)

#define McD1PciCfg8(Register)                             MmPci8 (0, SA_MC_BUS, 1, 0, Register)
#define McD1PciCfg8Or(Register, OrData)                   MmPci8Or (0, SA_MC_BUS, 1, 0, Register, OrData)
#define McD1PciCfg8And(Register, AndData)                 MmPci8And (0, SA_MC_BUS, 1, 0, Register, AndData)
#define McD1PciCfg8AndThenOr(Register, AndData, OrData)   MmPci8AndThenOr (0, SA_MC_BUS, 1, 0, Register, AndData, OrData)

///
/// Device 1, Function 1
///
#define McD1F1PciCfg64(Register)                            MmPci64 (0, SA_MC_BUS, 1, 1, Register)
#define McD1F1PciCfg64Or(Register, OrData)                  MmPci64Or (0, SA_MC_BUS, 1, 1, Register, OrData)
#define McD1F1PciCfg64And(Register, AndData)                MmPci64And (0, SA_MC_BUS, 1, 1, Register, AndData)
#define McD1F1PciCfg64AndThenOr(Register, AndData, OrData)  MmPci64AndThenOr (0, SA_MC_BUS, 1, 1, Register, AndData, OrData)

#define McD1F1PciCfg32(Register)                            MmPci32 (0, SA_MC_BUS, 1, 1, Register)
#define McD1F1PciCfg32Or(Register, OrData)                  MmPci32Or (0, SA_MC_BUS, 1, 1, Register, OrData)
#define McD1F1PciCfg32And(Register, AndData)                MmPci32And (0, SA_MC_BUS, 1, 1, Register, AndData)
#define McD1F1PciCfg32AndThenOr(Register, AndData, OrData)  MmPci32AndThenOr (0, SA_MC_BUS, 1, 1, Register, AndData, OrData)

#define McD1F1PciCfg16(Register)                            MmPci16 (0, SA_MC_BUS, 1, 1, Register)
#define McD1F1PciCfg16Or(Register, OrData)                  MmPci16Or (0, SA_MC_BUS, 1, 1, Register, OrData)
#define McD1F1PciCfg16And(Register, AndData)                MmPci16And (0, SA_MC_BUS, 1, 1, Register, AndData)
#define McD1F1PciCfg16AndThenOr(Register, AndData, OrData)  MmPci16AndThenOr (0, SA_MC_BUS, 1, 1, Register, AndData, OrData)

#define McD1F1PciCfg8(Register)                             MmPci8 (0, SA_MC_BUS, 1, 1, Register)
#define McD1F1PciCfg8Or(Register, OrData)                   MmPci8Or (0, SA_MC_BUS, 1, 1, Register, OrData)
#define McD1F1PciCfg8And(Register, AndData)                 MmPci8And (0, SA_MC_BUS, 1, 1, Register, AndData)
#define McD1F1PciCfg8AndThenOr(Register, AndData, OrData)   MmPci8AndThenOr (0, SA_MC_BUS, 1, 1, Register, AndData, OrData)

///
/// Device 1, Function 2
///
#define McD1F2PciCfg64(Register)                            MmPci64 (0, SA_MC_BUS, 1, 2, Register)
#define McD1F2PciCfg64Or(Register, OrData)                  MmPci64Or (0, SA_MC_BUS, 1, 2, Register, OrData)
#define McD1F2PciCfg64And(Register, AndData)                MmPci64And (0, SA_MC_BUS, 1, 2, Register, AndData)
#define McD1F2PciCfg64AndThenOr(Register, AndData, OrData)  MmPci64AndThenOr (0, SA_MC_BUS, 1, 2, Register, AndData, OrData)

#define McD1F2PciCfg32(Register)                            MmPci32 (0, SA_MC_BUS, 1, 2, Register)
#define McD1F2PciCfg32Or(Register, OrData)                  MmPci32Or (0, SA_MC_BUS, 1, 2, Register, OrData)
#define McD1F2PciCfg32And(Register, AndData)                MmPci32And (0, SA_MC_BUS, 1, 2, Register, AndData)
#define McD1F2PciCfg32AndThenOr(Register, AndData, OrData)  MmPci32AndThenOr (0, SA_MC_BUS, 1, 2, Register, AndData, OrData)

#define McD1F2PciCfg16(Register)                            MmPci16 (0, SA_MC_BUS, 1, 2, Register)
#define McD1F2PciCfg16Or(Register, OrData)                  MmPci16Or (0, SA_MC_BUS, 1, 2, Register, OrData)
#define McD1F2PciCfg16And(Register, AndData)                MmPci16And (0, SA_MC_BUS, 1, 2, Register, AndData)
#define McD1F2PciCfg16AndThenOr(Register, AndData, OrData)  MmPci16AndThenOr (0, SA_MC_BUS, 1, 2, Register, AndData, OrData)

#define McD1F2PciCfg8(Register)                             MmPci8 (0, SA_MC_BUS, 1, 2, Register)
#define McD1F2PciCfg8Or(Register, OrData)                   MmPci8Or (0, SA_MC_BUS, 1, 2, Register, OrData)
#define McD1F2PciCfg8And(Register, AndData)                 MmPci8And (0, SA_MC_BUS, 1, 2, Register, AndData)
#define McD1F2PciCfg8AndThenOr(Register, AndData, OrData)   MmPci8AndThenOr (0, SA_MC_BUS, 1, 2, Register, AndData, OrData)

///
/// Device 2, Function 0
///
#define McD2PciCfg64(Register)                            MmPci64 (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg64Or(Register, OrData)                  MmPci64Or (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg64And(Register, AndData)                MmPci64And (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg64AndThenOr(Register, AndData, OrData)  MmPci64AndThenOr (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg32(Register)                            MmPci32 (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg32Or(Register, OrData)                  MmPci32Or (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg32And(Register, AndData)                MmPci32And (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg32AndThenOr(Register, AndData, OrData)  MmPci32AndThenOr (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg16(Register)                            MmPci16 (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg16Or(Register, OrData)                  MmPci16Or (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg16And(Register, AndData)                MmPci16And (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg16AndThenOr(Register, AndData, OrData)  MmPci16AndThenOr (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg8(Register)                             MmPci8 (0, SA_MC_BUS, 2, 0, Register)
#define McD2PciCfg8Or(Register, OrData)                   MmPci8Or (0, SA_MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg8And(Register, AndData)                 MmPci8And (0, SA_MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg8AndThenOr(Register, AndData, OrData)   MmPci8AndThenOr (0, SA_MC_BUS, 2, 0, Register, AndData, OrData)

///
/// Device 22, Function 0
///
#define PchD22PciCfg32(Register)                            MmPci32 (0, 0, 22, 0, Register)
#define PchD22PciCfg32Or(Register, OrData)                  MmPci32Or (0, 0, 22, 0, Register, OrData)
#define PchD22PciCfg32And(Register, AndData)                MmPci32And (0, 0, 22, 0, Register, AndData)
#define PchD22PciCfg32AndThenOr(Register, AndData, OrData)  MmPci32AndThenOr (0, 0, 22, 0, Register, AndData, OrData)

///
/// Memory Controller Hub Memory Mapped IO register access
///
#define MCH_REGION_BASE                 (McD0PciCfg64 (R_SA_MCHBAR) &~BIT0)
#define McMmioAddress(Register)         ((UINTN) MCH_REGION_BASE + (UINTN) (Register))

#define McMmio32Ptr(Register)           ((volatile UINT32 *) McMmioAddress (Register))
#define McMmio64Ptr(Register)           ((volatile UINT64 *) McMmioAddress (Register))

#define McMmio64(Register)              *McMmio64Ptr (Register)
#define McMmio64Or(Register, OrData)    (McMmio64 (Register) |= (UINT64) (OrData))
#define McMmio64And(Register, AndData)  (McMmio64 (Register) &= (UINT64) (AndData))
#define McMmio64AndThenOr(Register, AndData, OrData) \
    (McMmio64 (Register) = (McMmio64 (Register) & (UINT64) (AndData)) | (UINT64) (OrData))

#define McMmio32(Register)              *McMmio32Ptr (Register)
#define McMmio32Or(Register, OrData)    (McMmio32 (Register) |= (UINT32) (OrData))
#define McMmio32And(Register, AndData)  (McMmio32 (Register) &= (UINT32) (AndData))
#define McMmio32AndThenOr(Register, AndData, OrData) \
    (McMmio32 (Register) = (McMmio32 (Register) & (UINT32) (AndData)) | (UINT32) (OrData))

#define McMmio16Ptr(Register)           ((volatile UINT16 *) McMmioAddress (Register))
#define McMmio16(Register)              *McMmio16Ptr (Register)
#define McMmio16Or(Register, OrData)    (McMmio16 (Register) |= (UINT16) (OrData))
#define McMmio16And(Register, AndData)  (McMmio16 (Register) &= (UINT16) (AndData))
#define McMmio16AndThenOr(Register, AndData, OrData) \
    (McMmio16 (Register) = (McMmio16 (Register) & (UINT16) (AndData)) | (UINT16) (OrData))

#define McMmio8Ptr(Register)          ((volatile UINT8 *) McMmioAddress (Register))
#define McMmio8(Register)             *McMmio8Ptr (Register)
#define McMmio8Or(Register, OrData)   (McMmio8 (Register) |= (UINT8) (OrData))
#define McMmio8And(Register, AndData) (McMmio8 (Register) &= (UINT8) (AndData))
#define McMmio8AndThenOr(Register, AndData, OrData) \
    (McMmio8 (Register) = (McMmio8 (Register) & (UINT8) (AndData)) | (UINT8) (OrData))

#define MmioR32(Address)        (*(volatile UINT32 *) (UINTN) (Address))
#define MmioW32(Address, Value) (*(volatile UINT32 *) (UINTN) (Address) = (Value))



///
///  Common Memory mapped Io access macros
///
#ifndef MmioAddress
#define MmioAddress(BaseAddr, Register) \
  ((UINTN) BaseAddr + (UINTN) (Register))
#endif
///
/// Macro to point to a 64 bit MMIO Address
///
#define Mmio64Ptr(BaseAddr, Register) \
  ((volatile UINT64 *) MmioAddress (BaseAddr, Register))
///
/// Macro to get the value from a 64 bit MMIO Address
///
#define Mmio64(BaseAddr, Register) \
  *Mmio64Ptr (BaseAddr, Register)
///
/// Macro to get a value from a 64 bit MMIO Address, perform a logical OR with
/// the given data and then write it back to the same address
///
#define Mmio64Or(BaseAddr, Register, OrData) \
  Mmio64 (BaseAddr, Register) = (UINT64) \
    (Mmio64 (BaseAddr, Register) | (UINT64) (OrData))
///
/// Macro to get a value from a 64 bit MMIO Address, perform a logical AND with
/// the given data and then write it back to the same address
///
#define Mmio64And(BaseAddr, Register, AndData) \
  Mmio64 (BaseAddr, Register) = (UINT64) \
    (Mmio64 (BaseAddr, Register) & (UINT64) (AndData))
///
/// Macro to get a value from a 64 bit MMIO Address, perform a logical AND and
/// then a OR with the given data and then write it back to the same address
///
#define Mmio64AndThenOr(BaseAddr, Register, AndData, OrData) \
  Mmio64 (BaseAddr, Register) = (UINT64) \
    ((Mmio64 (BaseAddr, Register) & (UINT64) (AndData)) | (UINT64) (OrData))

///
/// Macro to point to a 32 bit MMIO Address
///
#ifndef Mmio32Ptr
#define Mmio32Ptr(BaseAddr, Register) \
  ((volatile UINT32 *) MmioAddress (BaseAddr, Register))
#endif
///
/// Macro to get the value from a 32 bit MMIO Address
///
#ifndef Mmio32
#define Mmio32(BaseAddr, Register) \
  *Mmio32Ptr (BaseAddr, Register)
#endif
///
/// Macro to get a value from a 32 bit MMIO Address, perform a logical OR with
/// the given data and then write it back to the same address
///
#ifndef Mmio32Or
#define Mmio32Or(BaseAddr, Register, OrData) \
  Mmio32 (BaseAddr, Register) = (UINT32) \
    (Mmio32 (BaseAddr, Register) | (UINT32) (OrData))
#endif
///
/// Macro to get a value from a 32 bit MMIO Address, perform a logical AND with
/// the given data and then write it back to the same address
///
#ifndef Mmio32And
#define Mmio32And(BaseAddr, Register, AndData) \
  Mmio32 (BaseAddr, Register) = (UINT32) \
    (Mmio32 (BaseAddr, Register) & (UINT32) (AndData))
#endif
///
/// Macro to get a value from a 32 bit MMIO Address, perform a logical AND and
/// then a OR with the given data and then write it back to the same address
///
#ifndef Mmio32AndThenOr
#define Mmio32AndThenOr(BaseAddr, Register, AndData, OrData) \
  Mmio32 (BaseAddr, Register) = (UINT32) \
    ((Mmio32 (BaseAddr, Register) & (UINT32) (AndData)) | (UINT32) (OrData))
#endif
///
/// Macro to point to a 16 bit MMIO Address
///
#ifndef Mmio16Ptr
#define Mmio16Ptr(BaseAddr, Register) \
  ((volatile UINT16 *) MmioAddress (BaseAddr, Register))
#endif
///
/// Macro to get the value from a 16 bit MMIO Address
///
#ifndef Mmio16
#define Mmio16(BaseAddr, Register) \
  *Mmio16Ptr (BaseAddr, Register)
#endif
///
/// Macro to get a value from a 16 bit MMIO Address, perform a logical OR with
/// the given data and then write it back to the same address
///
#ifndef Mmio16Or
#define Mmio16Or(BaseAddr, Register, OrData) \
  Mmio16 (BaseAddr, Register) = (UINT16) \
    (Mmio16 (BaseAddr, Register) | (UINT16) (OrData))
#endif
///
/// Macro to get a value from a 16 bit MMIO Address, perform a logical AND with
/// the given data and then write it back to the same address
///
#ifndef Mmio16And
#define Mmio16And(BaseAddr, Register, AndData) \
  Mmio16 (BaseAddr, Register) = (UINT16) \
    (Mmio16 (BaseAddr, Register) & (UINT16) (AndData))
#endif
///
/// Macro to get a value from a 16 bit MMIO Address, perform a logical AND and
/// then a OR with the given data and then write it back to the same address
///
#ifndef Mmio16AndThenOr
#define Mmio16AndThenOr(BaseAddr, Register, AndData, OrData) \
  Mmio16 (BaseAddr, Register) = (UINT16) \
    ((Mmio16 (BaseAddr, Register) & (UINT16) (AndData)) | (UINT16) (OrData))
#endif
///
///  Macro to point to a 8 bit MMIO Address
///
#ifndef Mmio8Ptr
#define Mmio8Ptr(BaseAddr, Register) \
  ((volatile UINT8 *) MmioAddress (BaseAddr, Register))
#endif
///
/// Macro to get the value from a 8 bit MMIO Address
///
#ifndef Mmio8
#define Mmio8(BaseAddr, Register) \
  *Mmio8Ptr (BaseAddr, Register)
#endif
///
/// Macro to get a value from a 8 bit MMIO Address, perform a logical OR with
/// the given data and then write it back to the same address
///
#ifndef Mmio8Or
#define Mmio8Or(BaseAddr, Register, OrData) \
  Mmio8 (BaseAddr, Register) = (UINT8) \
    (Mmio8 (BaseAddr, Register) | (UINT8) (OrData))
#endif
///
/// Macro to get a value from a 8 bit MMIO Address, perform a logical AND with
/// the given data and then write it back to the same address
///
#ifndef Mmio8And
#define Mmio8And(BaseAddr, Register, AndData) \
  Mmio8 (BaseAddr, Register) = (UINT8) \
    (Mmio8 (BaseAddr, Register) & (UINT8) (AndData))
#endif
///
/// Macro to get a value from a 8 bit MMIO Address, perform a logical AND and
/// then a OR with the given data and then write it back to the same address
///
#ifndef Mmio8AndThenOr
#define Mmio8AndThenOr(BaseAddr, Register, AndData, OrData) \
  Mmio8 (BaseAddr, Register) = (UINT8) \
    ((Mmio8 (BaseAddr, Register) & (UINT8) (AndData)) | (UINT8) (OrData))
#endif
///
/// Macro to point to a 64 bit PCI Configuration Space Address
///
#define MmPci64Ptr(Segment, Bus, Device, Function, Register) \
  ((volatile UINT64 *) MmPciAddress (Segment, Bus, Device, Function, Register))
///
/// Macro to get the value from a 64 bit PCI Configuration Space Address
///
#define MmPci64(Segment, Bus, Device, Function, Register) (0)
///
/// Macro to get a value from a 64 bit PCI Configuration Space Address, perform
/// a logical OR with the given data and then write it back to the same address
///
#define MmPci64Or(Segment, Bus, Device, Function, Register, OrData) \
  MmPci64 (Segment, Bus, Device, Function, Register) = \
    (UINT64) (MmPci64 (Segment, Bus, Device, Function, Register) | (UINT64) (OrData))
///
/// Macro to get a value from a 64 bit PCI Configuration Space Address, perform
/// a logical AND with the given data and then write it back to the same address
///
#define MmPci64And(Segment, Bus, Device, Function, Register, AndData) \
  MmPci64 (Segment, \
           Bus, \
           Device, \
           Function, \
           Register \
      ) = (UINT64) (MmPci64 (Segment, Bus, Device, Function, Register) & (UINT64) (AndData))
///
/// Macro to get a value from a 64 bit PCI Configuration Space Address, perform
/// a logical AND and then a OR with the given data and then write it back to the
/// same address
///
#define MmPci64AndThenOr(Segment, Bus, Device, Function, Register, AndData, OrData) \
  MmPci64 (Segment, \
           Bus, \
           Device, \
           Function, \
           Register \
      ) = (UINT64) ((MmPci64 (Segment, Bus, Device, Function, Register) & (UINT64) (AndData)) | (UINT64) (OrData))

///
/// Macro to point to a 32 bit PCI Configuration Space Address
///
#define MmPci32Ptr(Segment, Bus, Device, Function, Register) \
   ((volatile UINT32 *) MmPciAddress (Segment, Bus, Device, Function, Register))
///
/// Macro to get the value from a 32 bit PCI Configuration Space Address
///
UINT32 __MmPci32_Value;

#define MmPci32( Segment, Bus, Device, Function, Register ) (__MmPci32_Value)
///
/// Macro to get a value from a 32 bit PCI Configuration Space Address, perform
/// a logical OR with the given data and then write it back to the same address
///
#define MmPci32Or(Segment, Bus, Device, Function, Register, OrData) \
  MmPci32 (Segment, Bus, Device, Function, Register) = \
    (UINT32) (MmPci32 (Segment, Bus, Device, Function, Register) | (UINT32) (OrData))
///
/// Macro to get a value from a 32 bit PCI Configuration Space Address, perform
/// a logical AND with the given data and then write it back to the same address
///
#define MmPci32And(Segment, Bus, Device, Function, Register, AndData) \
  MmPci32 (Segment, \
           Bus, \
           Device, \
           Function, \
           Register \
      ) = (UINT32) (MmPci32 (Segment, Bus, Device, Function, Register) & (UINT32) (AndData))
///
/// Macro to get a value from a 32 bit PCI Configuration Space Address, perform
/// a logical AND and then a OR with the given data and then write it back to the
/// same address
///
#define MmPci32AndThenOr(Segment, Bus, Device, Function, Register, AndData, OrData) \
  MmPci32 (Segment, \
           Bus, \
           Device, \
           Function, \
           Register \
      ) = (UINT32) ((MmPci32 (Segment, Bus, Device, Function, Register) & (UINT32) (AndData)) | (UINT32) (OrData))

///
/// Macro to point to a 16 bit PCI Configuration Space Address
///
#define MmPci16Ptr(Segment, Bus, Device, Function, Register) \
  ((volatile UINT16 *) MmPciAddress (Segment, Bus, Device, Function, Register))
///
/// Macro to get the value from a 16 bit PCI Configuration Space Address
///
#define MmPci16(Segment, Bus, Device, Function, Register) (0)
///
/// Macro to get a value from a 16 bit PCI Configuration Space Address, perform
/// a logical OR with the given data and then write it back to the same address
///
#define MmPci16Or(Segment, Bus, Device, Function, Register, OrData) \
  MmPci16 (Segment, Bus, Device, Function, Register) = \
    (UINT16) (MmPci16 (Segment, Bus, Device, Function, Register) | (UINT16) (OrData))
///
/// Macro to get a value from a 16 bit PCI Configuration Space Address, perform
/// a logical AND with the given data and then write it back to the same address
///
#define MmPci16And(Segment, Bus, Device, Function, Register, AndData) \
  MmPci16 (Segment, \
           Bus, \
           Device, \
           Function, \
           Register \
      ) = (UINT16) (MmPci16 (Segment, Bus, Device, Function, Register) & (UINT16) (AndData))
///
/// Macro to get a value from a 16 bit PCI Configuration Space Address, perform
/// a logical AND and then a OR with the given data and then write it back to the
/// same address
///
#define MmPci16AndThenOr(Segment, Bus, Device, Function, Register, AndData, OrData) \
  MmPci16 (Segment, \
           Bus, \
           Device, \
           Function, \
           Register \
      ) = (UINT16) ((MmPci16 (Segment, Bus, Device, Function, Register) & (UINT16) (AndData)) | (UINT16) (OrData))

///
/// Macro to point to a 8 bit PCI Configuration Space Address
///
#define MmPci8Ptr(Segment, Bus, Device, Function, Register) \
  ((volatile UINT8 *) MmPciAddress (Segment, Bus, Device, Function, Register))
///
/// Macro to get the value from a 8 bit PCI Configuration Space Address
///
#define MmPci8(Segment, Bus, Device, Function, Register) (0)

///
/// Macro to get a value from a 8 bit PCI Configuration Space Address, perform
/// a logical OR with the given data and then write it back to the same address
///
#define MmPci8Or(Segment, Bus, Device, Function, Register, OrData) \
  MmPci8 (Segment, Bus, Device, Function, Register) = \
    (UINT8) (MmPci8 (Segment, Bus, Device, Function, Register) | (UINT8) (OrData))
///
/// Macro to get a value from a 8 bit PCI Configuration Space Address, perform
/// a logical AND with the given data and then write it back to the same address
///
#define MmPci8And(Segment, Bus, Device, Function, Register, AndData) \
  MmPci8 (Segment, Bus, Device, Function, Register) = \
    (UINT8) (MmPci8 (Segment, Bus, Device, Function, Register) & (UINT8) (AndData))
///
/// Macro to get a value from a 8 bit PCI Configuration Space Address, perform
/// a logical AND and then a OR with the given data and then write it back to the
/// same address
///
#define MmPci8AndThenOr(Segment, Bus, Device, Function, Register, AndData, OrData) \
  MmPci8 (Segment, \
          Bus, \
          Device, \
          Function, \
          Register \
      ) = (UINT8) ((MmPci8 (Segment, Bus, Device, Function, Register) & (UINT8) (AndData)) | (UINT8) (OrData))
#endif
