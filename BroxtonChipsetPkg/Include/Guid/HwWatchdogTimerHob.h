/*++

Copyright (c)  2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  HwWatchdogTimerHob.h

Abstract:

  GUID used for Watchdog Timer status in the HOB list.

--*/

#ifndef _EFI_WATCHDOG_TIMER_HOB_GUID_H_
#define _EFI_WATCHDOG_TIMER_HOB_GUID_H_

#define EFI_WATCHDOG_TIMER_HOB_GUID \
  { 0x226cd3f, 0x69b5, 0x4150, 0xac, 0xbe, 0xbf, 0xbf, 0x18, 0xe3, 0x3, 0xd5 }

#define EFI_WATCHDOG_TIMER_DEFINITION_HOB_GUID \
  { 0xd29302b0, 0x11ba, 0x4073, 0xa2, 0x27, 0x53, 0x8d, 0x25, 0x42, 0x70, 0x9f }

typedef enum {
  HWWD_NONE,
  HWWD_TIMER_EXPIRED,
  HWWD_SPONTANEOUS_REBOOT,
  HWWD_FORCED_TIMEOUT
} HW_WATCHDOG_TIMEOUT;

typedef struct {
  HW_WATCHDOG_TIMEOUT         TimeoutStatus;
} HW_WATCHDOG_INFO;

//
// Watchdog timer action values
//
#define WDT_ACTION_RESET                    0x01    // reload/reset timer
#define WDT_ACTION_QUERY_CURRENT_VALUE      0x04    // get current value                     // DON'T NEED FOR OVERCLOCK UTILITY
#define WDT_ACTION_QUERY_COUNTDOWN_PERIOD   0x05    // get countdown period
#define WDT_ACTION_SET_COUNTDOWN_PERIOD     0x06    // set countdown period
#define WDT_ACTION_QUERY_RUNNING_STATE      0x08    // query if running
#define WDT_ACTION_SET_RUNNING_STATE        0x09    // start timer
#define WDT_ACTION_QUERY_STOPPED_STATE      0x0A    // query if stopped
#define WDT_ACTION_SET_STOPPED_STATE        0x0B    // stop timer
// #define WDT_ACTION_QUERY_REBOOT             0x10    // will wdt cause reboot?                // DON'T NEED FOR OVERCLOCK UTILITY
// #define WDT_ACTION_SET_REBOOT               0x11    // make wdt cause reboot
// #define WDT_ACTION_QUERY_SHUTDOWN           0x12    // will wdt cause shutdown?              // DON'T NEED FOR OVERCLOCK UTILITY
// #define WDT_ACTION_SET_SHUTDOWN             0x13    // make wdt cause shutdown               // DON'T NEED FOR OVERCLOCK UTILITY
#define WDT_ACTION_QUERY_STATUS             0x20    // is current boot cause by wdt timeout?
#define WDT_ACTION_SET_STATUS               0x21    // resets wdt status bit

//
// Watchdog timer instruction values
//
#define WDT_INSTR_VALUE_MASK                0x03    // Mask for just the value
#define WDT_INSTR_READ_CMP_VALUE            0x00    // Read / compare value
#define WDT_INSTR_READ_COUNTDOWN            0x01    // read countdown value
#define WDT_INSTR_WRITE_VALUE               0x02    // Write value
#define WDT_INSTR_WRITE_COUNTDOWN           0x03    // write countdown value
#define WDT_INSTR_PRESERVE_REG              0x80    // preserve reg; used in Write Value / Write Countdown
#define WDT_INSTR_WRITE_VALUE_PRES         (0x02 | WDT_INSTR_PRESERVE_REG)   // Write value with preserve
#define WDT_INSTR_WRITE_COUNTDOWN_PRES     (0x03 | WDT_INSTR_PRESERVE_REG)   // write countdown value with preserve


// Notes:
//
// WDT_INSTR_READ_CMP_VALUE
//    X = Read(Register)
//    X = X >> Bit Offset described in Register Region
//    X = X & Mask
//    if (X != Value) FAIL
//    SUCCEED
//
//
// WDT_INSTR_READ_COUNTDOWN
//    X = Read(Register)
//    X = X >> Bit Offset described in Register Region
//    X = X & Mask
//    return X
//
//
// WDT_INSTR_WRITE_VALUE
//    X = Value & Mask
//    X = X << Bit Offset described in Register Region
//    if (Preserve Register)
//        Y = Read(Register)
//        Y = Y & ~(Mask << Bit Offset)
//        X = X | Y
//    Write(X, Register)
//
//
// WDT_INSTR_WRITE_COUNTDOWN
//    X = countdown value
//    X = X & Mask
//    X = X << Bit Offset described in Register Region
//    if (Preserve Register)
//        Y = Read(Register)
//        Y = Y & ~(Mask << Bit Offset)
//        X = X | Y
//    Write(X, Register)
//

//
// The Generic Address Structure is defined in the ACPI Specification and should only be
// changed to match updated revisions of that specification.  The GAS_ADDRESS_SPACE and
// GAS_ACCESS_SIZE enumerations are also defined by the ACPI Specification.
//

typedef enum {
  GAS_SYSTEM_MEMORY,
  GAS_SYSTEM_IO,
  GAS_PCI_CONFIG_SPACE,
  GAS_EMBEDDED_CONTROLLER,
  GAS_SMBUS
} GAS_ADDRESS_SPACE;

typedef enum {
  GAS_UNDEFINED,
  GAS_BYTE_ACCESS,
  GAS_WORD_ACCESS,
  GAS_DWORD_ACCESS,
  GAS_QWORD_ACCESS
} GAS_ACCESS_SIZE;

#pragma pack(1)

typedef struct {
  UINT8                       AddressSpaceId;
  UINT8                       RegisterBitWidth;
  UINT8                       RegisterBitOffset;
  UINT8                       AccessSize;
  UINT64                      Address;
} GENERIC_ADDRESS_STRUCTURE;

// GAS_SYSTEM_MEMORY -    When used as the AddressSpaceId, the 64-bit physical memory address
//                        of the register.  32-bit platforms must have the high DWORD set to 0.
// GAS_SYSTEM_IO -        The 64-bit I/O address of the register.  32-bit platforms must have
//                        the high DWORD set to 0.
// GAS_PCI_CONFIG_SPACE - PCI Configuration space addresses must be confined to devices on PCI
//                        Sepment Group 0, Bus 0.  This restriction exists to accommodate access
//                        to fixed hardware prior to PCI bus enumeration.  The format of addresses
//                        are defined as follows:
//                            Highest WORD: Reserved and must be -0-
//                            ...           PCI Device number on bus 0
//                            ...           PCI Function number
//                            Lowest WORD:  Offset in the configuration space header


typedef struct {
  UINT8                       WdAction;
  UINT8                       Flag;
  UINT16                      Res;
  GENERIC_ADDRESS_STRUCTURE   GenericAddressStructures;
  UINT32                      Value;
  UINT32                      Mask;
} WD_INSTRUCTION;

typedef struct {
  UINT32                      TimerPeriod;
  UINT32                      MaxTimerCount;
  UINT32                      MinTimerCount;
  UINT16                      InstructionCount;
  WD_INSTRUCTION              ActionDefinitions[1];
} WD_HOB_DEFINITION;

#pragma pack()

extern EFI_GUID gWatchdogTimerHobGuid;
extern EFI_GUID gWatchdogTimerDefinitionHobGuid;

#endif // _EFI_WATCHDOG_TIMER_HOB_GUID_H_
