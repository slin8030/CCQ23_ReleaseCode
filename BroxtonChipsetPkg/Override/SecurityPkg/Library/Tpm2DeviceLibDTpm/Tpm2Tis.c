/** @file
  TIS (TPM Interface Specification) functions used by dTPM2.0 library.
  
Copyright (c) 2013 - 2015, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials 
are licensed and made available under the terms and conditions of the BSD License 
which accompanies this distribution.  The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, 
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/Tpm20.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/PcdLib.h>

//
// Set structure alignment to 1-byte
//
#pragma pack (1)

//
// Register set map as specified in TIS specification Chapter 10
//
typedef struct {
  ///
  /// Used to gain ownership for this particular port.
  ///
  UINT8                             Access;             // 0
  UINT8                             Reserved1[7];       // 1
  ///
  /// Controls interrupts.
  ///
  UINT32                            IntEnable;          // 8
  ///
  /// SIRQ vector to be used by the TPM.
  ///
  UINT8                             IntVector;          // 0ch
  UINT8                             Reserved2[3];       // 0dh
  ///
  /// What caused interrupt.
  ///
  UINT32                            IntSts;             // 10h
  ///
  /// Shows which interrupts are supported by that particular TPM.
  ///
  UINT32                            IntfCapability;     // 14h
  ///
  /// Status Register. Provides status of the TPM.
  ///
  UINT8                             Status;             // 18h
  ///
  /// Number of consecutive writes that can be done to the TPM.
  ///
  UINT16                            BurstCount;         // 19h
  ///
  /// TPM2 support CANCEL at BIT[24] of STATUS register (WO)
  ///
  UINT8                             StatusEx;           // 1Bh
  UINT8                             Reserved3[8];
  ///
  /// Read or write FIFO, depending on transaction.
  ///
  UINT32                            DataFifo;           // 24h
  UINT8                             Reserved4[0xed8];   // 28h
  ///
  /// Vendor ID
  ///
  UINT16                            Vid;                // 0f00h
  ///
  /// Device ID
  ///
  UINT16                            Did;                // 0f02h
  ///
  /// Revision ID
  ///
  UINT8                             Rid;                // 0f04h
  ///
  /// TCG defined configuration registers.
  ///
  UINT8                             TcgDefined[0x7b];   // 0f05h
  ///
  /// Alias to I/O legacy space.
  ///
  UINT32                            LegacyAddress1;     // 0f80h
  ///
  /// Additional 8 bits for I/O legacy space extension.
  ///
  UINT32                            LegacyAddress1Ex;   // 0f84h
  ///
  /// Alias to second I/O legacy space.
  ///
  UINT32                            LegacyAddress2;     // 0f88h
  ///
  /// Additional 8 bits for second I/O legacy space extension.
  ///
  UINT32                            LegacyAddress2Ex;   // 0f8ch
  ///
  /// Vendor-defined configuration registers.
  ///
  UINT8                             VendorDefined[0x70];// 0f90h
} TIS_PC_REGISTERS;

//
// Restore original structure alignment
//
#pragma pack ()

//
// Define pointer types used to access TIS registers on PC
//
typedef TIS_PC_REGISTERS  *TIS_PC_REGISTERS_PTR;

//
// Define bits of ACCESS and STATUS registers
//

///
/// This bit is a 1 to indicate that the other bits in this register are valid.
///
#define TIS_PC_VALID                BIT7
///
/// Indicate that this locality is active.
///
#define TIS_PC_ACC_ACTIVE           BIT5
///
/// Set to 1 to indicate that this locality had the TPM taken away while
/// this locality had the TIS_PC_ACC_ACTIVE bit set.
///
#define TIS_PC_ACC_SEIZED           BIT4
///
/// Set to 1 to indicate that TPM MUST reset the
/// TIS_PC_ACC_ACTIVE bit and remove ownership for localities less than the
/// locality that is writing this bit.
///
#define TIS_PC_ACC_SEIZE            BIT3
///
/// When this bit is 1, another locality is requesting usage of the TPM.
///
#define TIS_PC_ACC_PENDIND          BIT2
///
/// Set to 1 to indicate that this locality is requesting to use TPM.
///
#define TIS_PC_ACC_RQUUSE           BIT1
///
/// A value of 1 indicates that a T/OS has not been established on the platform
///
#define TIS_PC_ACC_ESTABLISH        BIT0

///
/// When this bit is 1, TPM is in the Ready state, 
/// indicating it is ready to receive a new command.
///
#define TIS_PC_STS_READY            BIT6
///
/// Write a 1 to this bit to cause the TPM to execute that command.
///
#define TIS_PC_STS_GO               BIT5
///
/// This bit indicates that the TPM has data available as a response.
///
#define TIS_PC_STS_DATA             BIT4
///
/// The TPM sets this bit to a value of 1 when it expects another byte of data for a command.
///
#define TIS_PC_STS_EXPECT           BIT3
///
/// Writes a 1 to this bit to force the TPM to re-send the response.
///
#define TIS_PC_STS_RETRY            BIT1

//
// Default TimeOut value
//
#define TIS_TIMEOUT_A               (1000 * 1000)  // 1s
#define TIS_TIMEOUT_B               (2000 * 1000)  // 2s
#define TIS_TIMEOUT_C               (1000 * 1000)  // 1s
#define TIS_TIMEOUT_D               (1000 * 1000)  // 1s

#define TIS_TIMEOUT_MAX             (90000 * 1000)  // 90s

//
// Max TPM command/reponse length
//
#define TPMCMDBUFLENGTH             0x500

/**
  Check whether TPM chip exist.

  @param[in] TisReg  Pointer to TIS register.

  @retval    TRUE    TPM chip exists.
  @retval    FALSE   TPM chip is not found.
**/
//[-start-160421-IB08450341-modify]//
STATIC
BOOLEAN
TisPcPresenceCheck (
  IN      TIS_PC_REGISTERS_PTR      TisReg
  )
//[-end-160421-IB08450341-modify]//
{
  UINT8                             RegRead;
  
  RegRead = MmioRead8 ((UINTN)&TisReg->Access);
  return (BOOLEAN)(RegRead != (UINT8)-1);
}

/**
  Check whether the value of a TPM chip register satisfies the input BIT setting.

  @param[in]  Register     Address port of register to be checked.
  @param[in]  BitSet       Check these data bits are set.
  @param[in]  BitClear     Check these data bits are clear.
  @param[in]  TimeOut      The max wait time (unit MicroSecond) when checking register.

  @retval     EFI_SUCCESS  The register satisfies the check bit.
  @retval     EFI_TIMEOUT  The register can't run into the expected status in time.
**/
//[-start-160421-IB08450341-modify]//
STATIC
EFI_STATUS
TisPcWaitRegisterBits (
  IN      UINT8                     *Register,
  IN      UINT8                     BitSet,
  IN      UINT8                     BitClear,
  IN      UINT32                    TimeOut
  )
//[-end-160421-IB08450341-modify]//
{
  UINT8                             RegRead;
  UINT32                            WaitTime;

  for (WaitTime = 0; WaitTime < TimeOut; WaitTime += 30){
    RegRead = MmioRead8 ((UINTN)Register);
    if ((RegRead & BitSet) == BitSet && (RegRead & BitClear) == 0)
      return EFI_SUCCESS;
    MicroSecondDelay (30);
  }
  return EFI_TIMEOUT;
}

/**
  Get BurstCount by reading the burstCount field of a TIS regiger 
  in the time of default TIS_TIMEOUT_D.

  @param[in]  TisReg                Pointer to TIS register.
  @param[out] BurstCount            Pointer to a buffer to store the got BurstConut.

  @retval     EFI_SUCCESS           Get BurstCount.
  @retval     EFI_INVALID_PARAMETER TisReg is NULL or BurstCount is NULL.
  @retval     EFI_TIMEOUT           BurstCount can't be got in time.
**/
//[-start-160421-IB08450341-modify]//
STATIC
EFI_STATUS
TisPcReadBurstCount (
  IN      TIS_PC_REGISTERS_PTR      TisReg,
     OUT  UINT16                    *BurstCount
  )
//[-end-160421-IB08450341-modify]//
{
  UINT32                            WaitTime;
  UINT8                             DataByte0;
  UINT8                             DataByte1;

  if (BurstCount == NULL || TisReg == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  WaitTime = 0;
  do {
    //
    // TIS_PC_REGISTERS_PTR->burstCount is UINT16, but it is not 2bytes aligned,
    // so it needs to use MmioRead8 to read two times
    //
    DataByte0   = MmioRead8 ((UINTN)&TisReg->BurstCount);
    DataByte1   = MmioRead8 ((UINTN)&TisReg->BurstCount + 1);
    *BurstCount = (UINT16)((DataByte1 << 8) + DataByte0);
    if (*BurstCount != 0) {
      return EFI_SUCCESS;
    }
    MicroSecondDelay (30);
    WaitTime += 30;
  } while (WaitTime < TIS_TIMEOUT_D);

  return EFI_TIMEOUT;
}

/**
  Set TPM chip to ready state by sending ready command TIS_PC_STS_READY 
  to Status Register in time.

  @param[in] TisReg                Pointer to TIS register.

  @retval    EFI_SUCCESS           TPM chip enters into ready state.
  @retval    EFI_INVALID_PARAMETER TisReg is NULL.
  @retval    EFI_TIMEOUT           TPM chip can't be set to ready state in time.
**/
//[-start-160421-IB08450341-modify]//
STATIC
EFI_STATUS
TisPcPrepareCommand (
  IN      TIS_PC_REGISTERS_PTR      TisReg
  )
//[-end-160421-IB08450341-modify]//
{
  EFI_STATUS                        Status;

  if (TisReg == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  MmioWrite8((UINTN)&TisReg->Status, TIS_PC_STS_READY);
  Status = TisPcWaitRegisterBits (
             &TisReg->Status,
             TIS_PC_STS_READY,
             0,
             TIS_TIMEOUT_B
             );
  return Status;
}

/**
  Get the control of TPM chip by sending requestUse command TIS_PC_ACC_RQUUSE 
  to ACCESS Register in the time of default TIS_TIMEOUT_A.

  @param[in] TisReg                Pointer to TIS register.

  @retval    EFI_SUCCESS           Get the control of TPM chip.
  @retval    EFI_INVALID_PARAMETER TisReg is NULL.
  @retval    EFI_NOT_FOUND         TPM chip doesn't exit.
  @retval    EFI_TIMEOUT           Can't get the TPM control in time.
**/
//[-start-160421-IB08450341-modify]//
STATIC
EFI_STATUS
TisPcRequestUseTpm (
  IN      TIS_PC_REGISTERS_PTR      TisReg
  )
//[-end-160421-IB08450341-modify]//
{
  EFI_STATUS                        Status;
  
  if (TisReg == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (!TisPcPresenceCheck (TisReg)) {
    return EFI_NOT_FOUND;
  }

  MmioWrite8((UINTN)&TisReg->Access, TIS_PC_ACC_RQUUSE);
  Status = TisPcWaitRegisterBits (
             &TisReg->Access,
             (UINT8)(TIS_PC_ACC_ACTIVE |TIS_PC_VALID),
             0,
             TIS_TIMEOUT_A
             );
  return Status;
}

/**
  Send a command to TPM for execution and return response data.

  @param[in]      TisReg        TPM register space base address.  
  @param[in]      BufferIn      Buffer for command data.  
  @param[in]      SizeIn        Size of command data.  
  @param[in, out] BufferOut     Buffer for response data.  
  @param[in, out] SizeOut       Size of response data.  
 
  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_BUFFER_TOO_SMALL  Response data buffer is too small.
  @retval EFI_DEVICE_ERROR      Unexpected device behavior.
  @retval EFI_UNSUPPORTED       Unsupported TPM version

**/
//[-start-160421-IB08450341-modify]//
STATIC
EFI_STATUS
TisTpmCommand (
  IN     TIS_PC_REGISTERS_PTR       TisReg,
  IN     UINT8                      *BufferIn,
  IN     UINT32                     SizeIn,
  IN OUT UINT8                      *BufferOut,
  IN OUT UINT32                     *SizeOut
  )
//[-end-160421-IB08450341-modify]//
{
  EFI_STATUS                        Status;
  UINT16                            BurstCount;
  UINT32                            Index;
  UINT32                            TpmOutSize;
  UINT16                            Data16;
  UINT32                            Data32;

  DEBUG_CODE (
    UINTN  DebugSize;

    DEBUG ((EFI_D_INFO, "Tpm2TisTpmCommand Send - "));
    if (SizeIn > 0x100) {
      DebugSize = 0x40;
    } else {
      DebugSize = SizeIn;
    }
    for (Index = 0; Index < DebugSize; Index++) {
      DEBUG ((EFI_D_INFO, "%02x ", BufferIn[Index]));
    }
    if (DebugSize != SizeIn) {
      DEBUG ((EFI_D_INFO, "...... "));
      for (Index = SizeIn - 0x20; Index < SizeIn; Index++) {
        DEBUG ((EFI_D_INFO, "%02x ", BufferIn[Index]));
      }
    }
    DEBUG ((EFI_D_INFO, "\n"));
  );
  TpmOutSize = 0;

  Status = TisPcPrepareCommand (TisReg);
  if (EFI_ERROR (Status)){
    DEBUG ((DEBUG_ERROR, "Tpm2 is not ready for command!\n"));
    return EFI_DEVICE_ERROR;
  }
  //
  // Send the command data to Tpm
  //
  Index = 0;
  while (Index < SizeIn) {
    Status = TisPcReadBurstCount (TisReg, &BurstCount);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }
    for (; BurstCount > 0 && Index < SizeIn; BurstCount--) {
      MmioWrite8((UINTN)&TisReg->DataFifo, *(BufferIn + Index));
      Index++;
    }
  }
  //
  // Check the Tpm status STS_EXPECT change from 1 to 0
  //
  Status = TisPcWaitRegisterBits (
             &TisReg->Status,
             (UINT8) TIS_PC_VALID,
             TIS_PC_STS_EXPECT,
             TIS_TIMEOUT_C
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Tpm2 The send buffer too small!\n"));
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }
  //
  // Executed the TPM command and waiting for the response data ready
  //
  MmioWrite8((UINTN)&TisReg->Status, TIS_PC_STS_GO);

  //
  // NOTE: That may take many seconds to minutes for certain commands, such as key generation.
  //
  Status = TisPcWaitRegisterBits (
             &TisReg->Status,
             (UINT8) (TIS_PC_VALID | TIS_PC_STS_DATA),
             0,
             TIS_TIMEOUT_MAX
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Wait for Tpm2 response data time out!!\n"));
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }
  //
  // Get response data header
  //
  Index = 0;
  BurstCount = 0;
  while (Index < sizeof (TPM2_RESPONSE_HEADER)) {
    Status = TisPcReadBurstCount (TisReg, &BurstCount);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }
    for (; BurstCount > 0; BurstCount--) {
      *(BufferOut + Index) = MmioRead8 ((UINTN)&TisReg->DataFifo);
      Index++;
      if (Index == sizeof (TPM2_RESPONSE_HEADER)) break;
    }
  }
  DEBUG_CODE (
    DEBUG ((EFI_D_INFO, "TisTpmCommand ReceiveHeader - "));
    for (Index = 0; Index < sizeof (TPM2_RESPONSE_HEADER); Index++) {
      DEBUG ((EFI_D_INFO, "%02x ", BufferOut[Index]));
    }
    DEBUG ((EFI_D_INFO, "\n"));
  );
  //
  // Check the reponse data header (tag,parasize and returncode )
  //
  CopyMem (&Data16, BufferOut, sizeof (UINT16));
  // TPM2 should not use this RSP_COMMAND
  if (SwapBytes16 (Data16) == TPM_ST_RSP_COMMAND) {
    DEBUG ((EFI_D_ERROR, "TPM2: TPM_ST_RSP error - %x\n", TPM_ST_RSP_COMMAND));
    Status = EFI_UNSUPPORTED;
    goto Exit;
  }

  CopyMem (&Data32, (BufferOut + 2), sizeof (UINT32));
  TpmOutSize  = SwapBytes32 (Data32);
  if (*SizeOut < TpmOutSize) {
    Status = EFI_BUFFER_TOO_SMALL;
    goto Exit;
  }
  *SizeOut = TpmOutSize;
  //
  // Continue reading the remaining data
  //
  while ( Index < TpmOutSize ) {
    for (; BurstCount > 0; BurstCount--) {
      *(BufferOut + Index) = MmioRead8 ((UINTN)&TisReg->DataFifo);
      Index++;
      if (Index == TpmOutSize) {
        Status = EFI_SUCCESS;
        goto Exit;
      }
    }
    Status = TisPcReadBurstCount (TisReg, &BurstCount);
    if (EFI_ERROR (Status)) {
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }
  }
Exit:
  DEBUG_CODE (
    DEBUG ((EFI_D_INFO, "Tpm2TisTpmCommand Receive - "));
    for (Index = 0; Index < TpmOutSize; Index++) {
      DEBUG ((EFI_D_INFO, "%02x ", BufferOut[Index]));
    }
    DEBUG ((EFI_D_INFO, "\n"));
  );
  MmioWrite8((UINTN)&TisReg->Status, TIS_PC_STS_READY);
  return Status;
}

/**
  This service enables the sending of commands to the TPM2.

  @param[in]      InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small. 
**/
EFI_STATUS
EFIAPI
DTpm2SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
  return TisTpmCommand (
           (TIS_PC_REGISTERS_PTR) (UINTN) PcdGet64 (PcdTpmBaseAddress),
           InputParameterBlock,
           InputParameterBlockSize,
           OutputParameterBlock,
           OutputParameterBlockSize
           );
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
DTpm2RequestUseTpm (
  VOID
  )
{
  return TisPcRequestUseTpm ((TIS_PC_REGISTERS_PTR) (UINTN) PcdGet64 (PcdTpmBaseAddress));
}
