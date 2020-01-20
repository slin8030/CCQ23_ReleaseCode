/*++

Copyright (c)  1999 - 2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SmmSmbus.c

Abstract:

    Contains code to read and write to SMBus devices.

Revision History

--*/

#include "SmmPlatform.h"
#include "ScAccess.h"
#include "SmmSmbus.h"

#define MAX_STALL_TIME                  1000000   // 1,000,000us = 1s
#define STALL_TIME                      100       // 100us between checks


BOOLEAN
IoDone(
  IN      UINT16 SmbusBase,
  IN OUT  UINT8* StsReg
  );

VOID
StallInternalFunction(
  IN UINTN              Microseconds
  );
//
// SM_BUS:RestrictedBegin & 
//
EFI_STATUS
BlockSmBusWrite(
  IN      EFI_SMBUS_DEVICE_ADDRESS  Address,
  IN      UINTN                     Command,
  IN      UINTN                     Length,
  IN      VOID                      *Buffer
  )
{
  // Local Variables
  EFI_STATUS  Status;
  UINT16      SmbusBase;
  UINT16      SmbusCommand;
  UINT8       AuxcReg;
  UINT8       AuxStsReg;
  UINT8       SmbusOperation;
  UINT8       StsReg;
  UINT8       SlvAddrReg;
  UINT8       HostCmdReg;
  UINT8       BlockCount;
  UINT8       *CallBuffer;
  UINT8       trys;
  UINT8       Index;

  Status = EFI_SUCCESS;

  // Check input values
  if ((Length < 1 || Length > 32) || Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Get the base address for the controller
  SmbusBase = 0;
  SmbusBase = MmioRead16 (
                MmPciAddress (0,
                       DEFAULT_PCI_BUS_NUMBER_SC,
                       PCI_DEVICE_NUMBER_PCH_SMBUS,
                       PCI_FUNCTION_NUMBER_PCH_SMBUS,
                  R_PCH_SMBUS_BASE
                )
              );

  if ((SmbusBase == 0xFFFF) || ((SmbusBase & ~(BIT0)) == 0)) {
    return EFI_DEVICE_ERROR;
  }
  SmbusBase &= ~(BIT0);

  //
  // Now that we know the base address seems ok make sure that I/O space is
  // enabled.
  //
  SmbusCommand = MmioRead16 (
                   MmPciAddress (0,
                          DEFAULT_PCI_BUS_NUMBER_SC,
                          PCI_DEVICE_NUMBER_PCH_SMBUS,
                          PCI_FUNCTION_NUMBER_PCH_SMBUS,
                     R_PCH_SMBUS_PCICMD
                   )
                 );
  MmioOr16 (
    MmPciAddress (0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_PCH_SMBUS,
             PCI_FUNCTION_NUMBER_PCH_SMBUS,
      R_PCH_SMBUS_PCICMD
    ),
    B_PCH_SMBUS_PCICMD_IOSE
  );

  // Check to see if the bus is in use
  StsReg = IoRead8(SmbusBase + R_PCH_SMBUS_HSTS);
  if (StsReg & B_PCH_SMBUS_IUS) {
    MmioWrite16 (
      MmPciAddress (0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_PCH_SMBUS,
             PCI_FUNCTION_NUMBER_PCH_SMBUS,
        R_PCH_SMBUS_PCICMD
      ),
      SmbusCommand
    );
    return EFI_TIMEOUT;
  } else if (StsReg & B_PCH_SMBUS_HBSY) {
    IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
    MmioWrite16 (
      MmPciAddress (0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_PCH_SMBUS,
             PCI_FUNCTION_NUMBER_PCH_SMBUS,
        R_PCH_SMBUS_PCICMD
      ),
      SmbusCommand
    );
    return EFI_TIMEOUT;
  }

  // Try three times to perform transaction.
  for (trys = 0; trys < 3; trys++) {

    // Setup values
    CallBuffer = (UINT8*) Buffer;
    HostCmdReg = (UINT8) Command;
    SlvAddrReg = (UINT8) (Address.SmbusDeviceAddress << 1);
    SmbusOperation = V_PCH_SMBUS_SMB_CMD_BLOCK;
    BlockCount = (UINT8) Length;
    AuxcReg = 0;

    //
    // Set controller up to start block write
    //
    // Clear the byte done status
    IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BYTE_DONE_STS);

    // Set the SMBus slave address
    IoWrite8(SmbusBase + R_PCH_SMBUS_TSA, SlvAddrReg);

    // Set the command register
    IoWrite8(SmbusBase + R_PCH_SMBUS_HCMD, HostCmdReg);

    // Set transfer length
    IoWrite8(SmbusBase + R_PCH_SMBUS_HD0, (UINT8) Length);

    // Set AUX control register
    IoWrite8(SmbusBase + R_PCH_SMBUS_AUXC, AuxcReg);

    // Set first piece of data
    Index = 0;
    IoWrite8(SmbusBase + R_PCH_SMBUS_HBD, CallBuffer[Index]);

    // Set the control register
    IoWrite8(SmbusBase + R_PCH_SMBUS_HCTL, (SmbusOperation + B_PCH_SMBUS_START));

    // Write the rest of the data
    for (Index = 1; Index < BlockCount; Index++) {
      // Wait for IoDone
      if (!IoDone(SmbusBase, &StsReg)) {
        Status = EFI_TIMEOUT;
        break;
      } else if (StsReg & B_PCH_SMBUS_DERR) {
        AuxStsReg = IoRead8(SmbusBase + R_PCH_SMBUS_AUXS);
        if (AuxStsReg & B_PCH_SMBUS_CRCE) {
          Status = EFI_CRC_ERROR;
        } else {
          Status = EFI_DEVICE_ERROR;
        }
        break;
      } else if (StsReg & B_PCH_SMBUS_BERR) {
        // Set a device error and clear the status
        Status = EFI_DEVICE_ERROR;
        IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);
        continue;
      }

      // Write the next byte
      IoWrite8(SmbusBase + R_PCH_SMBUS_HBD, CallBuffer[Index]);

      // Clear the byte done status
      IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BYTE_DONE_STS);
    }

    // Wait for last IO to be done
    if (!IoDone(SmbusBase, &StsReg)) {
      Status = EFI_TIMEOUT;
      break;
    } else if (StsReg & B_PCH_SMBUS_DERR) {
      AuxStsReg = IoRead8(SmbusBase + R_PCH_SMBUS_AUXS);
      if (AuxStsReg & B_PCH_SMBUS_CRCE) {
        Status = EFI_CRC_ERROR;
      } else {
        Status = EFI_DEVICE_ERROR;
      }
      break;
    } else if (StsReg & B_PCH_SMBUS_BERR) {
      // Set a device error and clear the status
      Status = EFI_DEVICE_ERROR;
      IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);
      continue;
    }

    // If you are here the write is complete so exit
    break;

  } // End of for loop

  // Clean up status registers
  IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
  IoWrite8(SmbusBase + R_PCH_SMBUS_AUXS, B_PCH_SMBUS_CRCE);

  //
  // Leave I/O space as it was
  //
  MmioWrite16 (
    MmPciAddress (0,
           DEFAULT_PCI_BUS_NUMBER_SC,
           PCI_DEVICE_NUMBER_PCH_SMBUS,
           PCI_FUNCTION_NUMBER_PCH_SMBUS,
      R_PCH_SMBUS_PCICMD
    ),
    SmbusCommand
  );
  return Status;
}

EFI_STATUS
BlockSmBusRead(
  IN      EFI_SMBUS_DEVICE_ADDRESS  Address,
  IN      UINTN                     Command,
  IN OUT  UINTN                     *Length,
  IN OUT  VOID                      *Buffer
  )
{
  // Local Variables
  EFI_STATUS  Status;
  UINT16      SmbusBase;
  UINT16      SmbusCommand;
  UINT8       AuxcReg;
  UINT8       AuxStsReg;
  UINT8       SmbusOperation;
  UINT8       StsReg;
  UINT8       SlvAddrReg;
  UINT8       HostCmdReg;
  UINT8       BlockCount;
  UINT8       *CallBuffer;
  UINT8       TempByte;
  UINT8       trys;
  UINT8       Index;
  BOOLEAN     BufferTooSmall;

  Status = EFI_SUCCESS;

  // Check input values
  if ((*Length < 1 || *Length > 32) || Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Get the base address for the controller
  SmbusBase = 0;
  SmbusBase = MmioRead16 (
                MmPciAddress (0,
                       DEFAULT_PCI_BUS_NUMBER_SC,
                       PCI_DEVICE_NUMBER_PCH_SMBUS,
                       PCI_FUNCTION_NUMBER_PCH_SMBUS,
                  R_PCH_SMBUS_BASE
                )
              );

  if ((SmbusBase == 0xFFFF) || ((SmbusBase & ~(BIT0)) == 0)) {
    return EFI_DEVICE_ERROR;
  }
  SmbusBase &= ~(BIT0);

  //
  // Now that we know the base address seems ok make sure that I/O space is
  // enabled.
  //
  SmbusCommand = MmioRead16 (
                   MmPciAddress (0,
                          DEFAULT_PCI_BUS_NUMBER_SC,
                          PCI_DEVICE_NUMBER_PCH_SMBUS,
                          PCI_FUNCTION_NUMBER_PCH_SMBUS,
                     R_PCH_SMBUS_PCICMD
                   )
                 );
  MmioOr16 (
    MmPciAddress (0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_PCH_SMBUS,
             PCI_FUNCTION_NUMBER_PCH_SMBUS,
      R_PCH_SMBUS_PCICMD
    ),
    B_PCH_SMBUS_PCICMD_IOSE
  );

  // Check to see if the bus is in use
  StsReg = IoRead8(SmbusBase + R_PCH_SMBUS_HSTS);
  if (StsReg & B_PCH_SMBUS_IUS) {
    MmioWrite16 (
      MmPciAddress (0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_PCH_SMBUS,
             PCI_FUNCTION_NUMBER_PCH_SMBUS,
        R_PCH_SMBUS_PCICMD
      ),
      SmbusCommand
    );
  }
  else if (StsReg & B_PCH_SMBUS_HBSY) {
    IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
    MmioWrite16 (
      MmPciAddress (0,
             DEFAULT_PCI_BUS_NUMBER_SC,
             PCI_DEVICE_NUMBER_PCH_SMBUS,
             PCI_FUNCTION_NUMBER_PCH_SMBUS,
        R_PCH_SMBUS_PCICMD
      ),
      SmbusCommand
    );
    return EFI_TIMEOUT;
  }

  // Try three times to perform transaction.
  for (trys = 0; trys < 3; trys++) {

    // Setup values
    CallBuffer = (UINT8*) Buffer;
    HostCmdReg = (UINT8) Command;
    SlvAddrReg = (UINT8) ((Address.SmbusDeviceAddress << 1) | 1);
    SmbusOperation = V_PCH_SMBUS_SMB_CMD_BLOCK;
    AuxcReg = B_PCH_SMBUS_E32B;

    //
    // Set controller up to start block read
    //
    // Clear byte pointer of 32byte buffer
    TempByte = IoRead8(SmbusBase + R_PCH_SMBUS_HCTL);

    // Clear the byte done status
    IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BYTE_DONE_STS);

    // Set AUX control register
    IoWrite8(SmbusBase + R_PCH_SMBUS_AUXC, AuxcReg);

    // Set the SMBus slave address
    IoWrite8(SmbusBase + R_PCH_SMBUS_TSA, SlvAddrReg);

    // Set the command register
    IoWrite8(SmbusBase + R_PCH_SMBUS_HCMD, HostCmdReg);

    // Set the control register
    IoWrite8(SmbusBase + R_PCH_SMBUS_HCTL, (SmbusOperation + B_PCH_SMBUS_START));

    // Check for an error
    if (!IoDone(SmbusBase, &StsReg)) {
      Status = EFI_TIMEOUT;
      break;
    } else if (StsReg & B_PCH_SMBUS_DERR) {
      AuxStsReg = IoRead8(SmbusBase + R_PCH_SMBUS_AUXS);
      if (AuxStsReg & B_PCH_SMBUS_CRCE) {
        Status = EFI_CRC_ERROR;
      } else {
        Status = EFI_DEVICE_ERROR;
      }
      break;
    } else if (StsReg & B_PCH_SMBUS_BERR) {
      // Set a device error and clear the status
      Status = EFI_DEVICE_ERROR;
      IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);
      continue;
    }

    // Get transaction size and check dest buffer
    BufferTooSmall = FALSE;
    BlockCount = IoRead8(SmbusBase + R_PCH_SMBUS_HD0);
    if (BlockCount > *Length) {
      BufferTooSmall = TRUE;
    } else {
      for (Index = 0; Index < BlockCount; Index++) {
        CallBuffer[Index] = IoRead8(SmbusBase + R_PCH_SMBUS_HBD);
      }
    }

    *Length = BlockCount;
    if (BufferTooSmall) {
      Status = EFI_BUFFER_TOO_SMALL;
    }
    break;

  } // End for loop

  // Clean up status registers
  IoWrite8(SmbusBase + R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
  IoWrite8(SmbusBase + R_PCH_SMBUS_AUXS, B_PCH_SMBUS_CRCE);

  //
  // Leave I/O space as it was
  //
  MmioWrite16 (
    MmPciAddress (0,
           DEFAULT_PCI_BUS_NUMBER_SC,
           PCI_DEVICE_NUMBER_PCH_SMBUS,
           PCI_FUNCTION_NUMBER_PCH_SMBUS,
      R_PCH_SMBUS_PCICMD
    ),
    SmbusCommand
  );

  return Status;
}
//
// SM_BUS:RestrictedEnd
//
BOOLEAN
IoDone(
  IN      UINT16 SmbusBase,
  IN OUT  UINT8* StsReg
  )
{
  BOOLEAN     IoDone;
//
// SM_BUS:RestrictedBegin & 
//
  UINTN       i;
//
// SM_BUS:RestrictedEnd
//
  UINTN       Trys;

  // Init values
  IoDone = FALSE;
  Trys = MAX_STALL_TIME / STALL_TIME;
//
// SM_BUS:RestrictedBegin & 
//
  for (i = 0; i < Trys; i++) {
    *StsReg = IoRead8(SmbusBase + R_PCH_SMBUS_HSTS);

    // Test for an error condition
    if (*StsReg & (B_PCH_SMBUS_INTR | B_PCH_SMBUS_BYTE_DONE_STS | B_PCH_SMBUS_DERR | B_PCH_SMBUS_BERR)) {
      IoDone = TRUE;
      break;
    }

    // If no error and transaction not done wait and try again
    Stall(STALL_TIME);
  }
//
// SM_BUS:RestrictedEnd
//
  return IoDone;
}


EFI_STATUS
Stall (
  IN UINTN              Microseconds
  )
/*++

Routine Description:

  Waits for at least the given number of microseconds.

Arguments:

  Microseconds          - Desired length of time to wait

Returns:

  EFI_SUCCESS           - If the desired amount of time passed.

--*/
{
  if (Microseconds == 0) {
    return EFI_SUCCESS;
  }

  StallInternalFunction(Microseconds);

  return EFI_SUCCESS;
}


VOID
StallInternalFunction(
  IN UINTN              Microseconds
  )
{
  UINTN                 Ticks;
  UINTN                 Counts;
  UINT32                CurrentTick;
  UINT32                OriginalTick;
  UINT64                RemainingTick;
  UINT16                mAcpiBaseAddr;

  mAcpiBaseAddr = (UINT16)PcdGet16(PcdScAcpiIoPortBaseAddress);

//[-start-180813-IB07400997-modify]//
  OriginalTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//

  CurrentTick = OriginalTick;

  //
  //The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;
  Counts = Ticks / V_ACPI_PM1_TMR_MAX_VAL;  //The loops needed by timer overflow
  RemainingTick = Ticks % V_ACPI_PM1_TMR_MAX_VAL;  //remaining clocks within one loop

  //
  //not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  //one I/O operation, and maybe generate SMI
  //

  while (Counts != 0) {
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
    if (CurrentTick < OriginalTick) {
//[-end-180813-IB07400997-modify]//
      Counts --;
    }
    OriginalTick = CurrentTick;
  }

  while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick) ) {
    OriginalTick = CurrentTick;
//[-start-180813-IB07400997-modify]//
    CurrentTick = IoRead32(mAcpiBaseAddr + R_ACPI_PM1_TMR) & (V_ACPI_PM1_TMR_MAX_VAL - 1);
//[-end-180813-IB07400997-modify]//
  }
}

