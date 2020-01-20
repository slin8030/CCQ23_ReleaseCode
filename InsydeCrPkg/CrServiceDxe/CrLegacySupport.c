/** @file
  CrLegacySupport

;******************************************************************************
;* Copyright (c) 2012 - 2016, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/
#include "CrLegacySupport.h"
#include "CrBdaMemManager.h"
#include "CrServiceMisc.h"

#include <Library/DxeChipsetSvcLib.h>
#include <Library/DxeOemSvcKernelLib.h>

EFI_CONSOLE_REDIRECTION_SERVICE_PROTOCOL     *gCrService                      = NULL;
EFI_CONSOLE_REDIRECTION_INFO                 *gCrInfo                         = NULL;
EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL              *mRootBridgeIo                   = NULL;
EFI_LEGACY_BIOS_PLATFORM_PROTOCOL            *mLegacyBiosPlatformProtocol     = NULL;
CR_EFI_MEM_MANAGER                           *mCrEbdaMemManager               = NULL;
EFI_LEGACY_BIOS_PROTOCOL                     *mLegacyBios                     = NULL;

EFI_EVENT                                    mLegacyBootEvent;
EFI_LEGACY_BIOS_PLATFORM_HOOKS               mPlatformHooks;
EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL  mCrOpROMInfo             = {0};
UINT32                                       mLastEbda                = 0;
UINT8                                        mUartUid                 = 0;

//
// Baud Rate Divisor.   Baud Rate 1200 Divisor = 115200/1200 = 96
//
static UINT16 mBaudRateDivisor [] = {96, 48, 24, 12, 6, 3, 2, 1 };
//
// This table use for translate CrInfo terminal type to EBdaCrInfo's TerminalType
// This table is depend on AdvanceVfr.vfr (vt100=Bit0 vt100P=Bit1 vt-utf8=Bit2 pc-ansi=Bit3)
//
static UINT8  gTerminalFlagTable [] = {TP_VT100, TP_VT100P, TP_VTUTF8, TP_PCANSI};

BOOLEAN
IsUartFlowControlNode (
  IN UART_FLOW_CONTROL_DEVICE_PATH *FlowControl
  )
{
  return (BOOLEAN) (
           (DevicePathType (FlowControl) == MESSAGING_DEVICE_PATH) &&
           (DevicePathSubType (FlowControl) == MSG_VENDOR_DP) &&
           (CompareGuid (&FlowControl->Guid, &gEfiUartDevicePathGuid))
           );
}

UINT8
SupportFlowControl (
  IN  EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
{
  UART_FLOW_CONTROL_DEVICE_PATH       *FlowControl;

  while (!IsDevicePathEnd (DevicePath)) {
    if (IsUartFlowControlNode ((UART_FLOW_CONTROL_DEVICE_PATH *) DevicePath)) {
      FlowControl = (UART_FLOW_CONTROL_DEVICE_PATH *)DevicePath;
      return (UINT8)(ReadUnaligned32 (&FlowControl->FlowControlMap));

    }
    DevicePath = NextDevicePathNode (DevicePath);
  }

  return 0;
}

UINT8
CrGetUartUid (
  IN   EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL    *This
  )
{
  return mUartUid++;
}

CR_COMMON_AREA_INFO*
GetCrCommonArea (
  IN   EFI_CONSOLE_REDIRECTION_OPROM_INFO_PROTOCOL    *This
  )
{
  CR_EFI_MEM_MANAGER  *CrEfiMemManager;
  UINT32              Ebda;
  UINT8               BiosPhase;
  UINT32              Signature;
  UINT32              EbdaBlock;


  if (This->CrOpROMInstalled) {
    Ebda = (UINTN)((*(UINT16*)(UINTN)0x40e) << 4);
    BiosPhase = EBDA_DATA(BIOS_PHASE, UINT8);

    if (BiosPhase == BIOS_PHASE_DOS_RUNING || BiosPhase == BIOS_PHASE_OPROM_RUNING || BiosPhase == BIOS_PHASE_OPROM) {
      return NULL;
    }

    if (mLastEbda != Ebda) {
      mCrEbdaMemManager = NULL;
      mLastEbda = Ebda;
    }

    //
    // Search CROpROM allocated EBDA memory
    //
    if (mCrEbdaMemManager == NULL) {
      for (EbdaBlock = 0; EbdaBlock < (0xA0000 - Ebda); EbdaBlock+=0x400) {
        Signature = EBDA_DATA(EbdaBlock, UINT32);
        if (Signature == CR_MEM_MANAGER_SIGNATURE) {
          mCrEbdaMemManager = (CR_EFI_MEM_MANAGER *)(UINTN)(((*(UINT16*)(UINTN)0x40e) << 4) + (EbdaBlock));
          break;
        }
      }
    }

    if (mCrEbdaMemManager != NULL) {
      //
      // Return CR Common Area
      //
      return (CR_COMMON_AREA_INFO *)((UINTN)mCrEbdaMemManager + (UINTN)mCrEbdaMemManager->CommonAreaOffset);
    }
  } else {
   if (mCrMemStart != 0) {
     CrEfiMemManager = (CR_EFI_MEM_MANAGER *)(UINTN)mCrMemStart;
     return (CR_COMMON_AREA_INFO *)(UINTN)(mCrMemStart + CrEfiMemManager->CommonAreaOffset);
   }
  }

  return NULL;
}


/**

  Clear UART IER Register to disable UART interrupt

**/
VOID
ClearUartIer (
  VOID
  )
{
  UINTN  Index;

  if (gCrInfo == NULL) {
    return;
  }
  for (Index = 0; Index < gCrInfo->DeviceCount; Index++) {
    gCrInfo->CRDevice[Index].Uart16550protocol->RegWrite(
                                                  gCrInfo->CRDevice[Index].Uart16550protocol,
                                                  UART_IER_OFFSET,
                                                  0
                                                  );
  }
}


/**

  Set interrupt of UART device

  @param  EnableIntr            Enable or disable UART interrupt

**/
VOID
SetSerialInterrupt (
  IN UINT8    EnableIntr
  )
{
  H2O_UART_16550_ACCESS_PROTOCOL *UartProtocol;
  CR_COMMON_AREA_INFO            *CrOpromInfo;
  UINT64                         PciAddress;
  UINT16                         u16;
  UINT8                          u8;
  UINTN                          Index;


  if (gCrInfo == NULL) {
    return;
  }

  u16 = 0;
  u8  = 0;
  CrOpromInfo = NULL;
  for (Index = 0; Index < gCrInfo->DeviceCount; Index++) {
    UartProtocol = gCrInfo->CRDevice[Index].Uart16550protocol;

    switch (gCrInfo->CRDevice[Index].Type) {

      case ISA_SERIAL_DEVICE:
        UartProtocol->RegRead (UartProtocol, UART_MCR_OFFSET, &u8);
        if (EnableIntr) {
          u8 = u8 | UART_MCR_OUT2_BIT;
        } else {
          u8 = u8 & ~UART_MCR_OUT2_BIT;
        }
        UartProtocol->RegWrite(UartProtocol, UART_MCR_OFFSET, u8);

        break;

      case PCI_SERIAL_DEVICE:
      case PCI_HS_SERIAL_DEVICE:
        u16 = gCrInfo->CRDevice[Index].Uart16550protocol->DeviceInfo->UID;
        PciAddress = EFI_PCI_ADDRESS (GET_BUS(u16), GET_DEV(u16), GET_FUN(u16), PCI_CONFIG_COMMAND_WORD);
        mRootBridgeIo->Pci.Read (mRootBridgeIo, EfiPciWidthUint16, PciAddress, 1, &u16);

        if (EnableIntr) {
          u16 = u16 & ~PCI_CONFIG_DISABLE_INTERRUPT;
          u16 = u16 | PCI_CONFIG_BUS_MASTER;
        } else {
          u16 = u16 | PCI_CONFIG_DISABLE_INTERRUPT;
          u16 = u16 & ~PCI_CONFIG_BUS_MASTER;
        }
        mRootBridgeIo->Pci.Write (mRootBridgeIo, EfiPciWidthUint16, PciAddress, 1, &u16);
        break;

      default:
        break;
    }
  }
}


/**

  Enable interrupt of UART device

**/
VOID
EnableSerialInterrupt (
  VOID
  )
{
  SetSerialInterrupt (1);
}


/**

  Disable interrupt of UART device

**/
VOID
DisableSerialInterrupt (
  VOID
  )
{
  SetSerialInterrupt (0);

  //
  // Due to some UART device can't be disable interrupt by SetSerialInterrupt (0)
  // So, we need clear IER register setting to ensure UART device interrupt disable
  //
  ClearUartIer ();
}


/**

  Translate Esc Sequence Code into KeyBoard scan code

  @param  TerminalEscCode  Pointer to Terminal ESC Sequence Code protocol
  @param  EscTableIndex      Index of Terminal ESC Sequence Code

  @retval   return translate result

**/
UINT8
TranslateEfiScanCode (
  IN EFI_TERMINAL_ESC_CODE_PROTOCOL    *TerminalEscCode,
  IN UINTN                             EscTableIndex
  )
{
  EFI_TO_KB_SCANCODE_MAP    *pScanCodeMap;
  ESC_SEQUENCE_CODE         *pEscSeqCode;
  UINT8                     DataType;
  UINTN                     Index;


  DataType = TerminalEscCode->EscSequenceCode[EscTableIndex].DataType;

  if ((DataType == ESC_CODE_SCANCODE) || (DataType == ESC_CODE_EXTENTION)) {

    pEscSeqCode = TerminalEscCode->EscSequenceCode;
    pScanCodeMap = TerminalEscCode->EfiToKbScanCode;

    for (Index = 0; pScanCodeMap[Index].EfiScanCode != 0; Index++) {
      if (pScanCodeMap[Index].EfiScanCode == pEscSeqCode[EscTableIndex].Data) {
        return (UINT8) (pScanCodeMap[Index].KbScanCode);
      }
    }
  }

  return (UINT8)(TerminalEscCode->EscSequenceCode[EscTableIndex].Data);
}


/**

  Fill Console Redirection supported special commands to CR_EFI_INFO structure

  @param  TerminalEscCode  Pointer to Console Redirection EFI Information structure

  @retval   EFI_SUCCESS               Special commands fill success
  @retval   EFI_UNSUPPORTED       Special commands fill fail

**/
EFI_STATUS
FillCrSpecialCommandTable (
  IN OUT CR_EFI_INFO    *CrEfiInfo
  )
{
  EFI_STATUS                        Status;
  EFI_TERMINAL_ESC_CODE_PROTOCOL    *TerminalEscCode;
  CR_EFI_SPECIAL_COMMAND_TABLE      *CrSpcTable;
  UINTN                             Index;
  CHAR8                             AsciiStr[128];
  UINTN                             StringLength;
  CHAR8                             *pCrSpcStr;

  Status = gBS->LocateProtocol (&gTerminalEscCodeProtocolGuid, NULL, (VOID **)&TerminalEscCode);
  if (EFI_ERROR (Status) || (TerminalEscCode->CrSpecialCommandCount == 0)) {
    CrEfiInfo->CrSpecialCommandTableOffset = 0;
    return EFI_UNSUPPORTED;
  }

  CrSpcTable = (CR_EFI_SPECIAL_COMMAND_TABLE *) CrEfiMemAlloc ( sizeof(CR_EFI_SPECIAL_COMMAND_TABLE) +
                                                                sizeof (CR_EFI_SPECIAL_COMMAND) *
                                                                (TerminalEscCode->CrSpecialCommandCount - 1));
  if (CrSpcTable == NULL) {
    ASSERT (0);
    return EFI_UNSUPPORTED;
  }

  CrEfiInfo->CrSpecialCommandTableOffset = (UINT16)((UINTN)CrSpcTable - mCrMemStart);

  CrSpcTable->Count = TerminalEscCode->CrSpecialCommandCount;

  for (Index = 0; Index < TerminalEscCode->CrSpecialCommandCount; Index++) {
    CrSpcTable->CrSpecialCommand[Index].Command = TerminalEscCode->CrSpecialCommand[Index].Command;
    CrSpcTable->CrSpecialCommand[Index].CommandType = TerminalEscCode->CrSpecialCommand[Index].CommandType;

    UnicodeStrToAsciiStr ((CONST CHAR16 *)TerminalEscCode->CrSpecialCommand[Index].CommandStr, AsciiStr);

    StringLength = AsciiStrLen (AsciiStr);
    pCrSpcStr = (CHAR8 *) CrEfiMemAlloc (StringLength + 1);
    if (pCrSpcStr == NULL) {
      ASSERT (0);
      return EFI_UNSUPPORTED;
    }
    CopyMem (pCrSpcStr ,AsciiStr, StringLength + 1);

    CrSpcTable->CrSpecialCommand[Index].CommandStrOffset = (UINT16)((UINTN)pCrSpcStr - mCrMemStart);
  }

  return EFI_SUCCESS;
}


/**

  Fill Console Redirection supported Terminal ESC Sequence Code to CR_EFI_INFO structure

  @param  TerminalEscCode  Pointer to Console Redirection EFI Information structure

  @retval   EFI_SUCCESS              Terminal ESC Sequence Code fill success
  @retval   EFI_UNSUPPORTED      Terminal ESC Sequence Code fill fail

**/
EFI_STATUS
FillTerminalEscCode (
  IN OUT CR_EFI_INFO    *CrEfiInfo
  )
{
  EFI_STATUS                        Status;
  EFI_TERMINAL_ESC_CODE_PROTOCOL    *TerminalEscCode;
  CR_EFI_ESC_SEQUENCE_CODE_TABLE    *CrEscCodeTable;
  UINTN                             Index;
  CHAR8                             AsciiStr[128];
  UINTN                             StringLength;
  CHAR8                             *pCrEscCode;
  UINT8                             Data;

  Status = gBS->LocateProtocol (&gTerminalEscCodeProtocolGuid, NULL, (VOID **)&TerminalEscCode);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Build ESC sequence code table
  //
  CrEscCodeTable = (CR_EFI_ESC_SEQUENCE_CODE_TABLE *) CrEfiMemAlloc (sizeof(CR_EFI_ESC_SEQUENCE_CODE_TABLE) +
                                                                     sizeof(CR_EFI_ESC_SEQUENCE_CODE) *
                                                                     (TerminalEscCode->EscCodeCount - 1));
  if (CrEscCodeTable == NULL) {
    ASSERT (0);
    return EFI_UNSUPPORTED;
  }

  CrEfiInfo->TerminalEscCodeOffset = (UINT16)((UINTN)CrEscCodeTable - mCrMemStart);

  CrEscCodeTable->Count = TerminalEscCode->EscCodeCount;

  for (Index = 0; Index < TerminalEscCode->EscCodeCount; Index++) {

    CrEscCodeTable->CrEscSequenceCode[Index].TerminalType = TerminalEscCode->EscSequenceCode[Index].TerminalType;
    CrEscCodeTable->CrEscSequenceCode[Index].Reserve = TerminalEscCode->EscSequenceCode[Index].Reserve;
    CrEscCodeTable->CrEscSequenceCode[Index].DataType = TerminalEscCode->EscSequenceCode[Index].DataType;
    Data = TranslateEfiScanCode(TerminalEscCode, Index);

    CrEscCodeTable->CrEscSequenceCode[Index].Data = Data;

    //
    // Process ESC sequence code string
    //
    UnicodeStrToAsciiStr ((CONST CHAR16 *)TerminalEscCode->EscSequenceCode[Index].EscSequenceCode, AsciiStr);
    StringLength = AsciiStrLen(AsciiStr);
    pCrEscCode = (CHAR8 *) CrEfiMemAlloc (StringLength + 1);
    if (pCrEscCode == NULL) {
      ASSERT (0);
      return EFI_UNSUPPORTED;
    }
    CopyMem (pCrEscCode ,AsciiStr, StringLength + 1);

    CrEscCodeTable->CrEscSequenceCode[Index].OffsetOfString = (UINT16)((UINTN)pCrEscCode - mCrMemStart);

  }

  return EFI_SUCCESS;
}

BOOLEAN
IsCrOpROMDevice (
  IN EFI_CONSOLE_REDIRECTION_DEVICE *CrDevice
  )
{
  return CrDevice->Uart16550protocol->DeviceInfo->LegacySupport;
}

/**

  Fill Console Redirection Terminal device information to CR_EFI_INFO structure

  @param  CrPolicy              Pointer to Console Redirection policy protocol
  @param  CRInfo               Pointer to Console Redirection information structure

  @retval   EFI_SUCCESS                      Console Redirection Terminal device information fill success
  @retval   EFI_INVALID_PARAMETER     Console Redirection Terminal device information fill fail

**/
EFI_STATUS
FillCRInfo (
  IN EFI_CONSOLE_REDIRECTION_INFO    *CRInfo
  )
{
  CR_EFI_INFO             *CrEfiInfo;
  CR_DEVICE               *CRDevice;
  UINT8                   UartProtocol;
  UINTN                   CRInfoSize;
  UINTN                   Index;
  UINTN                   CrOpromDevNum;
  UINTN                   DevCount;
  UART_16550_DEVICE_INFO  *UartDevInfo;

  if (CRInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  UartProtocol = 0;
  if (mCrPolicy.GlobalDataBits == CR_DATA_8BIT) {
    UartProtocol = UART_DATA_BITS_8;
  } else {
    UartProtocol = UART_DATA_BITS_7;
  }

  if (mCrPolicy.GlobalStopBits == CR_STOP_2BIT) {
    UartProtocol |= UART_STOP_BITS_2;
  }

  switch (mCrPolicy.GlobalParity) {
    case CR_PARITY_EVEN:
      UartProtocol |= UART_PARITY_EVEN;
      break;

    case CR_PARITY_ODD:
      UartProtocol |= UART_PARITY_ODD;
      break;

    default:
      break;
  }

  //
  // Count CR legacy support device
  //
  CrOpromDevNum = 0;
  for (Index = 0; Index < CRInfo->DeviceCount; Index++) {
    if ( IsCrOpROMDevice (&(CRInfo->CRDevice[Index])) ) {
       CrOpromDevNum++;
    }
  }

  //
  // Caculate memory size that CRINFO need
  //
  CRInfoSize = sizeof(CR_EFI_INFO) + CrOpromDevNum * sizeof(CR_DEVICE);

  //
  // Allocate memory from CR Efi Memory Manager
  //
  CrEfiInfo = (CR_EFI_INFO *) CrEfiMemAlloc (CRInfoSize);
  if (CrEfiInfo == NULL) {
    ASSERT (0);
    return EFI_UNSUPPORTED;
  }
  CrEfiInfo->Signature = SIGNATURE_16 ('C', 'R');
  CrEfiInfo->Revision = CR_INFO_REVISION;
  CrEfiInfo->HLength = sizeof (CR_EFI_INFO);
  CrEfiInfo->DeviceCount = (UINT8)CrOpromDevNum;
  CRDevice = (CR_DEVICE *)((UINT8 *)CrEfiInfo + sizeof(CR_EFI_INFO));

  for (DevCount = 0, Index = 0; Index < CRInfo->DeviceCount; Index++) {
    if ( !IsCrOpROMDevice (&(CRInfo->CRDevice[Index])) ) {
      continue;
    }
    UartDevInfo = CRInfo->CRDevice[Index].Uart16550protocol->DeviceInfo;
    CRDevice[DevCount].Type            = UartDevInfo->DeviceType;
    CRDevice[DevCount].BaudRateDivisor = CRInfo->CRDevice[Index].BaudRateDivisor;
    CRDevice[DevCount].BaudRate        = (UINT32)(UartDevInfo->SerialClockFreq / UartDevInfo->SampleRate / CRDevice[DevCount].BaudRateDivisor);
    CRDevice[DevCount].UartUid         = UartDevInfo->UartUid;
    CRDevice[DevCount].FlowControl     = SupportFlowControl(CRInfo->CRDevice[Index].DevicePath);

    if (CRInfo->CRDevice[Index].Type == ISA_SERIAL_DEVICE) {
      CRDevice[DevCount].Device.IsaSerial.ComPortAddress = (UINT16)CRInfo->CRDevice[Index].Uart16550protocol->DeviceInfo->BaseAddress;
      CRDevice[DevCount].Device.IsaSerial.ComPortIrq     = CRInfo->CRDevice[Index].Uart16550protocol->DeviceInfo->DevIRQ;

    } else if (CRInfo->CRDevice[Index].Type == PCI_SERIAL_DEVICE) {
      CRDevice[DevCount].Device.PciSerial.Bus      = GET_BUS(CRInfo->CRDevice[Index].Uart16550protocol->DeviceInfo->UID);
      CRDevice[DevCount].Device.PciSerial.Device   = GET_DEV(CRInfo->CRDevice[Index].Uart16550protocol->DeviceInfo->UID);
      CRDevice[DevCount].Device.PciSerial.Function = GET_FUN(CRInfo->CRDevice[Index].Uart16550protocol->DeviceInfo->UID);

    } else if (CRInfo->CRDevice[Index].Type == PCI_HS_SERIAL_DEVICE) {
      CRDevice[DevCount].Device.PciHsSerial.Bus         = GET_BUS(UartDevInfo->UID);
      CRDevice[DevCount].Device.PciHsSerial.Device      = GET_DEV(UartDevInfo->UID);
      CRDevice[DevCount].Device.PciHsSerial.Function    = GET_FUN(UartDevInfo->UID);
      CRDevice[DevCount].Device.PciHsSerial.AccessWidth = UartDevInfo->RegisterByteWidth;

    } else if (CRInfo->CRDevice[Index].Type == PCH_HS_SERIAL_DEVICE) {
      CRDevice[DevCount].Device.PchHsSerial.BaseAddr_Irq = (UINT32)(UartDevInfo->BaseAddress + UartDevInfo->DevIRQ);
      CRDevice[DevCount].Device.PchHsSerial.AccessWidth  = UartDevInfo->RegisterByteWidth;
    }

    DevCount++;
  }

  CrEfiInfo->BaudRateDivisor     = mBaudRateDivisor[mCrPolicy.GlobalBaudRate];
  CrEfiInfo->TerminalType        = gTerminalFlagTable[mCrPolicy.GlobalTerminalType];
  CrEfiInfo->Protocol            = UartProtocol;
  CrEfiInfo->FIFOLength          = mCrPolicy.CRFifoLength;
  CrEfiInfo->InfoWaitTime        = mCrPolicy.CRInfoWaitTime;
  CrEfiInfo->CRWriteCharInterval = mCrPolicy.CRWriteCharInterval;
  CrEfiInfo->ShowHelp            = mCrPolicy.Feature.Bit.CRShowHelp;
  CrEfiInfo->CRAfterPost         = mCrPolicy.Feature.Bit.CRAfterPost;
  CrEfiInfo->CRHeadlessVBuffer   = mCrPolicy.Feature.Bit.CRHeadlessVBuffer;
  CrEfiInfo->FeatureFlag         = mCrPolicy.Feature.Data16;
  CrEfiInfo->SmiPort             = PcdGet16 (PcdSoftwareSmiPort);
  CrEfiInfo->CrsSmi              = PcdGet8 (PcdH2OCrSoftwareSmi);

  //
  // Set Headless flag to notify serial OPROM.  (H:Headless  V:Vga card)
  //
  if (CRInfo->Headless == TRUE) {
    CrEfiInfo->Headless = 'H';
  } else {
    CrEfiInfo->Headless = 'V';
  }

  FillTerminalEscCode (CrEfiInfo);

  FillCrSpecialCommandTable (CrEfiInfo);

  BDA_DATA(CR_MEM_MANAGER, UINT16) = (UINT16) mCrMemStart;

  BDA_DATA(CR_CRINFO, UINT16) = (UINT16)((UINTN)CrEfiInfo - mCrMemStart);

  return EFI_SUCCESS;
}


/**

  Load Console Redirection Option ROM

  @param  CrPolicy              Pointer to Console Redirection policy protocol

  @retval   EFI_SUCCESS             Console Redirection Option ROM load success
  @retval   EFI_UNSUPPORTED     Console Redirection Option ROM load fail

**/
EFI_STATUS
LoadOpRomImage (
  VOID
  )
{
  EFI_STATUS                  Status;
  VOID                        *LocalRomImage;
  UINTN                       LocalRomSize;
  UINTN                       Flags;
  EFI_IA32_REGISTER_SET       RegisterSet;
  VOID                        *LegacyRegion;
  UINT16                      Legacy16CallSegment;
  UINT16                      Legacy16CallOffset;

  LocalRomSize = 0;
  LocalRomImage = NULL;
  Status = GetSectionFromAnyFv (
             PcdGetPtr(PcdConsoleRedirectionOpRomFile),
             EFI_SECTION_RAW,
             0,
             &LocalRomImage,
             &LocalRomSize
             );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  if (LocalRomImage != NULL || LocalRomSize != 0) {

    if (mCrPolicy.Feature.Bit.CROpROMLoadOnESegment == TRUE) {

      //
      // Get space from E-Seg for CrOpROM
      //
      Status = mLegacyBios->GetLegacyRegion (
                             mLegacyBios,
                             LocalRomSize,
                             0,
                             0x200,
                             &LegacyRegion
                             );
      if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;
      }
      //
      // Copy CrOpROM to reserved space in E-Seg
      //
      Status = mLegacyBios->CopyLegacyRegion (
                             mLegacyBios,
                             LocalRomSize,
                             LegacyRegion,
                             (VOID *)(UINTN)LocalRomImage
                             );
      if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;
      }

      EBDA_DATA(BIOS_PHASE, UINT8) = BIOS_PHASE_OPROM;
      EnableSerialInterrupt ();

      ZeroMem (&RegisterSet, sizeof (RegisterSet));
      Legacy16CallSegment = (UINT16) (((UINTN)LegacyRegion) >> 4);
      Legacy16CallOffset  = 0x0003;
      Status = mLegacyBios->FarCall86 (
                             mLegacyBios,
                             Legacy16CallSegment,
                             Legacy16CallOffset,
                             &RegisterSet,
                             NULL,
                             0
                             );
      ASSERT_EFI_ERROR(Status);

      DisableSerialInterrupt ();
      EBDA_DATA(BIOS_PHASE, UINT8) = BIOS_PHASE_INIT;
    } else {
      Status = mLegacyBios->InstallPciRom (
                             mLegacyBios,
                             NULL,
                             &LocalRomImage,
                             &Flags,
                             NULL,
                             NULL,
                             NULL,
                             NULL
                             );
      ASSERT_EFI_ERROR(Status);
    }

  }
  return Status;
}

/**

  Set Terminal mode to 80x25 or restore text mode in efi

  @param  SetMode0     TRUE  : Set mode0 (80x25)
                       FALSE : Restore Termainl mode.

**/
VOID
SetTerminalModeForLegacy (
  BOOLEAN  SetMode0
  )
{
  UINTN                               Index;
  EFI_STATUS                          Status;
  EFI_DEVICE_PATH                     *DPath;
  EFI_HANDLE                          TerminalHandle;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL     *SimpleTextOutput;

  for (Index = 0; Index < gCrInfo->DeviceCount; Index++) {
    DPath  = gCrInfo->CRDevice[Index].DevicePath;
    Status = gBS->LocateDevicePath (
                    &gEfiSimpleTextOutProtocolGuid,
                    &DPath,
                    &TerminalHandle
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    Status = gBS->HandleProtocol (TerminalHandle, &gEfiSimpleTextOutProtocolGuid, &SimpleTextOutput);
    if (EFI_ERROR(Status)) {
      continue;
    }

    if (SetMode0) {
      gCrInfo->CRDevice[Index].TextMode = SimpleTextOutput->Mode->Mode;
      SimpleTextOutput->SetMode (SimpleTextOutput, 0);
    } else {
      SimpleTextOutput->SetMode (SimpleTextOutput, gCrInfo->CRDevice[Index].TextMode);
    }
  }

}

/**

  Notify function for LegacyBoot event be invoked

  @param  Event     The Event that is being processed
  @param  Context  The Event Context

**/
VOID
EFIAPI
LegacyBootNotifyFunction (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  UINT16    *BdaEBdaOffset;
  UINT16    EBdaSegment;
  UINT8     *pBiosPhase;

  //
  // End of POST time for console redirection
  //
  BdaEBdaOffset = (UINT16 *)(UINTN)BDA_EBDA_OFFSET;
  EBdaSegment   = *BdaEBdaOffset;
  pBiosPhase    = (UINT8 *)(UINTN)(((UINT32)EBdaSegment << SEGMENT_SHIFT) + BIOS_PHASE);
  *pBiosPhase   = BIOS_PHASE_OS;

  SetTerminalModeForLegacy (TRUE);

  EnableSerialInterrupt ();

  gBS->CloseEvent (mLegacyBootEvent);

  return;
}

/**

  Set text mode or restore original video mode.

  @param  DeviceHandle          Mode specific.  See EFI_GET_PLATFORM_HOOK_MODE enum.
  @param  SwitchMode            Switch to text mode or restore original mode.

  @retval None.

**/
VOID
SwitchVideoMode (
  IN EFI_HANDLE   DeviceHandle,
  IN BOOLEAN      SwitchMode
  )
{
  EFI_IA32_REGISTER_SET       RegisterSet;
  BOOLEAN                     ForceSwitchTextMode;
  static UINT16               VideoOriginalMode;

  if (SwitchMode == SWITCH_TEXT_MODE) {

    ForceSwitchTextMode = TRUE;
    //
    // Check platform & oem policy for 'ForceSwitchTextMode'.
    //
    DxeCsSvcInstallPciRomSwitchTextMode (DeviceHandle, &ForceSwitchTextMode);
    DEBUG ((DEBUG_INFO, "Cr << %a >> DxeChipsetService() SwitchTextMode:%d\n", __FUNCTION__, ForceSwitchTextMode));

    //
    // OemServices
    //
    OemSvcInstallPciRomSwitchTextMode (DeviceHandle, &ForceSwitchTextMode);
    DEBUG ((DEBUG_INFO, "Cr << %a >> OemService()        SwitchTextMode:%d\n", __FUNCTION__, ForceSwitchTextMode));

    if (ForceSwitchTextMode) {
      //
      // Save Original VESA Vedio Mode
      //
      VideoOriginalMode = MAX_UINT16;
      RegisterSet.X.AX  = VESA_GET_MODE;
      RegisterSet.X.BX  = 0;
      mLegacyBios->Int86 (mLegacyBios, INT_10, &RegisterSet);

      if (RegisterSet.H.AL == VESA_SUPPORTED && RegisterSet.X.BX != VGA_TEXT_MODE) {
        VideoOriginalMode = RegisterSet.X.BX;
        //
        // Set the 80x25 Text VGA Mode before dispatch CRS ROM
        //
        RegisterSet.H.AH = VGA_SET_MODE;
        RegisterSet.H.AL = VGA_TEXT_MODE;
        mLegacyBios->Int86 (mLegacyBios, INT_10, &RegisterSet);
      } else {
        DEBUG ((DEBUG_WARN,
                "\nCr << %a >> Do not save the original video mode. VESA_SUPPORTED:0x%x, Mode:0x%x\n",
                __FUNCTION__,
                RegisterSet.X.AX,
                RegisterSet.X.BX
                ));
      }
    }

  } else {

    if (VideoOriginalMode != MAX_UINT16) {
      //
      // Restore Original Vedio Mode
      //
      RegisterSet.X.AX = VESA_SET_MODE;
      RegisterSet.X.BX = VideoOriginalMode;
      mLegacyBios->Int86 (mLegacyBios, INT_10, &RegisterSet);
      DEBUG ((DEBUG_INFO, "Cr << %a >> Restore Original Vedio Mode:0x%x\n", __FUNCTION__, VideoOriginalMode));
    }
  }
}

/**

  Allows Console Redirection perform specific required action after a LegacyBios operation.

  @param  This                  The protocol instance pointer
  @param  Mode                  Will bypass to next caller
  @param  Type                  Will bypass to next caller
  @param  DeviceHandle          Will bypass to next caller
  @param  ShadowAddress         Will bypass to next caller
  @param  Compatibility16Table  Will bypass to next caller
  @param  AdditionalData        Will bypass to next caller

  @retval   EFI_SUCCESS         The operation performed successfully.
  @retval   EFI_UNSUPPORTED     Mode is not supported on the platform.

**/
EFI_STATUS
EFIAPI
CrsPlatformHooks (
  IN EFI_LEGACY_BIOS_PLATFORM_PROTOCOL   *This,
  IN EFI_GET_PLATFORM_HOOK_MODE          Mode,
  IN UINT16                              Type,
  IN EFI_HANDLE                          DeviceHandle,
  IN OUT UINTN                           *ShadowAddress,
  IN EFI_COMPATIBILITY16_TABLE           *Compatibility16Table,
  IN VOID                                **AdditionalData
  )
{
  EFI_STATUS     Status;
  BOOLEAN        IsCrRom;

  IsCrRom = IS_CRROM(ShadowAddress)? TRUE: FALSE;

  //
  // If not CROpRom, execute original LegacyBios function for setup legacy environment.
  //
  if (!IsCrRom) {
    Status = mPlatformHooks (This, Mode, Type, DeviceHandle, ShadowAddress, Compatibility16Table, AdditionalData);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  switch (Mode) {

  case EfiPlatformHookPrepareToScanRom:

    if (IsCrRom) {
      SwitchVideoMode (DeviceHandle, SWITCH_TEXT_MODE);
    }

    SetTerminalModeForLegacy (TRUE);
    //
    // Set flag for CRS before Oprom runing
    //
    EBDA_DATA(BIOS_PHASE, UINT8) = BIOS_PHASE_OPROM;

    EnableSerialInterrupt ();
    break;

  case EfiPlatformHookShadowServiceRoms:
    break;

  case EfiPlatformHookAfterRomInit:

    DisableSerialInterrupt ();
    //
    // Set flag for CRS after Oprom runing
    //
    EBDA_DATA(BIOS_PHASE, UINT8) = BIOS_PHASE_INIT;
    SetTerminalModeForLegacy (FALSE);

    if (IsCrRom) {
      SwitchVideoMode (DeviceHandle, SWITCH_ORIGINAL_MODE);
    }
    break;

  default:
    break;

  }

  return EFI_SUCCESS;
}


/**

  Confirm AB segment can be accessed

  @retval   EFI_SUCCESS               AB segment can be accessed.
  @retval   EFI_ACCESS_DENIED     AB segment can not be accessed.

**/
EFI_STATUS
AbSegAccessConfirm (
  VOID
  )
{
  UINT8    RestoreData;
  UINT8    *pVBuffer;
  UINT8    Data[2];
  UINT8    Index;

  RestoreData = 0;
  Data[0] = 0x55;
  Data[1] = 0xAA;
  pVBuffer = (UINT8 *)((UINTN)(TEXT_PAGE_0_START));

  CopyMem (&RestoreData, pVBuffer, sizeof (UINT8));

  for (Index = 0; Index < 2; Index++) {

    CopyMem (pVBuffer, &Data[Index], sizeof (UINT8));
    if (CompareMem (pVBuffer, &Data[Index], sizeof (UINT8)) != 0) {
      return EFI_ACCESS_DENIED;
    }
  }

  CopyMem (pVBuffer, &RestoreData, sizeof (UINT8));

  return EFI_SUCCESS;
}

EFI_STATUS
CrOpRomSupport (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_SMM_ACCESS2_PROTOCOL        *SmmAccess;
  UINT16                          *pVgaBuffer;
  UINT16                          TextElement;
  UINTN                           TextCount;

  Status = gBS->LocateProtocol (&gEfiPciRootBridgeIoProtocolGuid, NULL,(VOID **) &mRootBridgeIo);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR(Status);
    return Status;
  }

  //
  //  Check Console Redirection is Enable or Disable
  //
  Status = gBS->LocateProtocol (&gConsoleRedirectionServiceProtocolGuid, NULL, (VOID **)&gCrService);
  if (EFI_ERROR (Status)) {
    ASSERT_EFI_ERROR(Status);
    return Status;
  }

  if (gCrService->CRInfo == NULL) {
    return EFI_NOT_FOUND;
  } else {
    gCrInfo = gCrService->CRInfo;
  }

  //
  // Get LegacyBiosProtocol for Load Serial Redirect Oprom
  //
  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **)&mLegacyBios);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Hook in LegacyBiosPlatform to do update BiosPhase Flag
  //
  if (mLegacyBiosPlatformProtocol == NULL) {
    Status = gBS->LocateProtocol (&gEfiLegacyBiosPlatformProtocolGuid, NULL, (VOID **)&mLegacyBiosPlatformProtocol);
    ASSERT_EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
      mPlatformHooks                             = mLegacyBiosPlatformProtocol->PlatformHooks;
      mLegacyBiosPlatformProtocol->PlatformHooks = CrsPlatformHooks;
    }
  }

  //
  // If Headless , Enable AB segment
  //
  if (mHeadless && (mCrPolicy.Feature.Bit.CRHeadlessVBuffer == CR_HEADLESS_USE_VBUFFER)) {

    Status = gBS->LocateProtocol (&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **)&SmmAccess);

    if (!EFI_ERROR(Status)) {
      Status = SmmAccess->Open (SmmAccess);

      if (!EFI_ERROR(Status)) {
        Status = AbSegAccessConfirm ();

        if (!EFI_ERROR(Status)) {
       	  //
          // Initialize VGA buffer (B8000 ~ B9000) for Option Rom ,EX: SCSI , PXE ... etc.
          //
          TextElement = CHAR_SPACE | ((COLOR_BG_BLACK | COLOR_FG_WHITE) << 8);
          pVgaBuffer = (UINT16 *)((UINTN)(TEXT_PAGE_0_START));
          TextCount = 0;
          do {
            *pVgaBuffer = TextElement;
            pVgaBuffer++;
            TextCount++;
          } while (TextCount < TEXT_PAGE_SIZE / 2);
          DEBUG ((EFI_D_INFO | EFI_D_ERROR, "\nConsole Redirection : Usb AB Segment as VBuffer in Headless system\n" ) );
        }
      }
    }

    //
    // If CAN NOT use AB segment(VBuffer) be used, auto change to use EBDA space
    //
    if (EFI_ERROR (Status)) {
      mCrPolicy.Feature.Bit.CRHeadlessVBuffer = CR_HEADLESS_USE_EBDA;
      DEBUG ((EFI_D_INFO | EFI_D_ERROR, "\nConsole Redirection : Usb EBDA as VBuffer in Headless system\n" ) );
    }
  }
  //
  // Fill CRInfo in CrEfiMemory space. That hand off CRInfo to Option Rom.
  //
  FillCRInfo (gCrInfo);

  mCrOpROMInfo.CrOpROMInstalled = FALSE;
  mCrOpROMInfo.CrGetCommonArea  = GetCrCommonArea;
  mCrOpROMInfo.CrGetUartUid     = CrGetUartUid;
  //
  // Install CROpROMInfo Protocol for Cr Usb terminal reference
  //
  gBS->InstallProtocolInterface (
           &mCrServiceImageHandle,
           &gCROpROMInfoProtocolGuid,
           EFI_NATIVE_INTERFACE,
           &mCrOpROMInfo
           );

  //
  // Load Console Redirection Option ROM
  //
  Status = LoadOpRomImage ();
  if (Status == EFI_SUCCESS) {
    mCrOpROMInfo.CrOpROMInstalled = TRUE;

    //
    //  When After Post time, we must notify OPRom to work and close SMM AB segment.
    //
    EfiCreateEventLegacyBootEx (TPL_NOTIFY, LegacyBootNotifyFunction, NULL, &mLegacyBootEvent);
  }

  //
  // Load OpROM complete, free CREfiMemory space
  //
  FreeCrEfiMemorySpace ();

  return Status;
}

