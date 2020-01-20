/** @file
  This file contain memory DIMM initialization function.

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

#include <Library/PcdLib.h>
//#include <ScRegs/RegsPcu.h>
#include <Library/TimerLib.h>

#include "McFunc.h"
#include "MrcFunc.h"
#include "IchRegs.h"
#include "Regs.h"
#include "DetectDimms.h"
#include "ConfigMem.h"
#include "MmrcHooks.h"
#include "Mailbox.h"
#include "Jedec_spec.h"

//
// Divide Configuration Register (DCR)
//
typedef union {
  struct {
    UINT32  DivideValue1:2;  ///< Low 2 bits of the divide value.
    UINT32  Reserved0:1;     ///< Always 0.
    UINT32  DivideValue2:1;  ///< Highest 1 bit of the divide value.
    UINT32  Reserved1:28;    ///< Reserved.
  } Bits;
  UINT32    Data32;
} LOCAL_APIC_DCR;

#define SMRAM_MRC_OFFSET   0x1410

CHAR8 g_ChDisabledStr[] = "CH[%d] is disabled (%d), skipping\n";

/**
  It delays the execution in the MRC for 'DelayUs' microseconds
  The delay must never be between 0-9 microseconds
  in order to do not create an overlap in the HPET and max value
  should be ('desired' / 0xF) < (2^32 - 1)

  @param[in]  DdelayUs   Delay specified in microseconds.

  @retval  None
**/
VOID
McStall (
  IN    UINT32  DelayUs
)
{
  MicroSecondDelay(DelayUs);
  return;
}

UINT16    mTimerLibLocalApicFrequencies[] = {
  100,
  133,
  200,
  167,
  83,
  400,
  267,
  333
};

UINT8     mTimerLibLocalApicDivisor[] = {
  0x02, 0x04, 0x08, 0x10,
  0x02, 0x04, 0x08, 0x10,
  0x20, 0x40, 0x80, 0x01,
  0x20, 0x40, 0x80, 0x01
};

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif
/**
  AsmReadApicBase

  @param[in]  None

  @retval  None
**/
UINTN
AsmReadApicBase()
{

  UINT32 Data32;
  Data32 = 0;
#ifndef SIM
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
  "\n\t pushl  %%eax"
  "\n\t pushl  %%ebx"
  "\n\t pushl  %%ecx"
  "\n\t pushl  %%edx"
  "\n\t movl   $0x1b, %%ecx"
  "\n\t rdmsr"
  "\n\t movl   %%eax, %0"
  "\n\t popl   %%edx"
  "\n\t popl   %%ecx"
  "\n\t popl   %%ebx"
  "\n\t popl   %%eax"
  : "=m"(Data32)
  :
  :
  );
#else //MSFT compiler
  _asm {

    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     ecx, 27
    rdmsr
    mov     Data32, eax

    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
  }
#endif
#endif //SIM
  return Data32;
}
#if defined __GNUC__
#pragma GCC pop_options
#endif


#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif
/**
  AsmReadApicFreq

  @param[in]  None

  @retval  Data32
**/
UINT32
AsmReadApicFreq()
{

  UINT32  Data32;
  Data32 = 0;
#ifndef SIM
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
  "\n\t pushl  %%eax"
  "\n\t pushl  %%ebx"
  "\n\t pushl  %%ecx"
  "\n\t pushl  %%edx"
  "\n\t movl   $0xCE, %%ecx"
  "\n\t rdmsr"
  "\n\t movl   %%eax, %0"
  "\n\t popl   %%edx"
  "\n\t popl   %%ecx"
  "\n\t popl   %%ebx"
  "\n\t popl   %%eax"
  : "=m"(Data32)
  :
  :
  );
#else //MSFT compiler
  _asm {

    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     ecx, 206
    rdmsr
    mov     Data32, eax

    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
  }
#endif
#endif //SIM
  return Data32;
}

VOID
HandleCheckpoint (
  IN        MMRC_DATA *MrcData,
  IN        UINT8      CheckPoint
  )
  /*++

  Routine Description:

  Configures a checkpoint to halt the CPU in a specific memory training step


  Arguments:

  CheckPoint:       Specifies the code related to an MRC step

  Returns:

  None

--*/
{

  MmrcVitalPrint ((MMRC_DBG_MIN, "CP %02X\n", (CheckPoint & 0xFF)));
//[-start-160216-IB07400702-add]//
  if (CheckPoint != 0xFF) {
    CHECKPOINT (CheckPoint);
  }
//[-end-160216-IB07400702-add]//
}

typedef struct {
  CHAR8  *String;
  UINT16 FieldOffset;
  UINT8  FieldSize;
} STRUCT_FIELD_DESCRIPTOR;

#define OFFSETOF_FIELD(type, field)    ((UINT16) (UINT32) &(((type *) 0)->field))
#define SIZEOF_FIELD(type, field)    ((UINT8) sizeof(((type *) 0)->field))
#define STRUCT_FIELD_DESCRIPTOR_ENTRY(type, field) {#field, OFFSETOF_FIELD(type, field), SIZEOF_FIELD(type, field)}
GLOBAL_REMOVE_IF_UNREFERENCED STRUCT_FIELD_DESCRIPTOR DramPolicyFields[] = {
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, ChannelHashMask),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, SliceHashMask),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, ChannelsSlicesEnabled),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, ScramblerSupport),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, InterleavedMode),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, MinRefRate2xEnabled),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, DualRankSupportEnabled),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, Profile),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, SpdAddress[0]),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, SpdAddress[1]),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, SystemMemorySizeLimit),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, LowMemMaxVal),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, HighMemMaxVal),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, DisableFastBoot),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, RmtMode),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, RmtCheckRun),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, RmtMarginCheckScaleHighThreshold),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, MsgLevelMask),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRAM_POLICY, MemoryDown),
};

GLOBAL_REMOVE_IF_UNREFERENCED STRUCT_FIELD_DESCRIPTOR DrpPolicyFields[] = {
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, RankEnable),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, DeviceWidth),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, DramDensity),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, Option),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, OdtConfig),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, TristateClk1),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, Mode2N),
  STRUCT_FIELD_DESCRIPTOR_ENTRY(DRP_DRAM_POLICY, OdtLevels),
};

VOID PrintStructField (
  EFI_PHYSICAL_ADDRESS    StructBaseOffset,
  STRUCT_FIELD_DESCRIPTOR *StructFieldDescriptor
)
{
  UINT32 Value;
  EFI_PHYSICAL_ADDRESS DataPointer;
  DataPointer = (EFI_PHYSICAL_ADDRESS)(((UINT32)StructBaseOffset) + ((UINT32) StructFieldDescriptor->FieldOffset));
  switch (StructFieldDescriptor->FieldSize) {
    case sizeof(UINT8):
      Value =  *((UINT8 *)(UINT32)DataPointer);
      break;
    case sizeof(UINT16):
      Value =  *((UINT16 *)(UINT32)DataPointer);
      break;
    default:
      Value =  *((UINT32 *)(UINT32)DataPointer);
      break;
  }
  MmrcVitalPrint((MMRC_DBG_MIN, "%-23a:0x%x\n", StructFieldDescriptor->String, Value));
}

MMRC_STATUS
DisplayInputParameters(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT8 Ch;
  UINT8 Bit;
  UINT8 i;
  MmrcVitalPrint((MMRC_DBG_MIN, "DRAM Policy:\n"));
  for (i = 0; i < (sizeof(DramPolicyFields)/sizeof(DramPolicyFields[0])); i++) {
    PrintStructField((EFI_PHYSICAL_ADDRESS)(UINTN)&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData, &DramPolicyFields[i]);
  }

  for (Ch = 0; Ch < DRAM_POLICY_NUMBER_CHANNELS; Ch++) {
    if (!RunOnThisChannel (MrcData, Ch, 0)) {
      continue;
    }

    MmrcVitalPrint((MMRC_DBG_MIN, "Channel%d:\n", Ch));

    for (i = 0; i < (sizeof(DrpPolicyFields)/sizeof(DrpPolicyFields[0])); i++) {
      PrintStructField((EFI_PHYSICAL_ADDRESS)(UINTN)&MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChDrp[Ch], &DrpPolicyFields[i]);
    }

    MmrcVitalPrint((MMRC_DBG_MIN, "Swizzling:\n"));
    for (Bit = 0; Bit < DRAM_POLICY_NUMBER_BITS; Bit++) {
      MmrcVitalPrint((MMRC_DBG_MIN, "%2x ", MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.ChSwizzle[Ch][Bit]));
    }
    MmrcVitalPrint((MMRC_DBG_MIN, "\n"));
  }

  return MMRC_SUCCESS;
}

MMRC_STATUS
DISPLAYMMRCVERSION (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  MrcVersion IafwVersion;
  GetIafwVersion(MrcData, &IafwVersion);
  MmrcVitalPrint((MMRC_DBG_MIN, "MRC REVISION ID %d.%02d.%02d  \n", IafwVersion.Bits.Major, IafwVersion.Version.Minor, IafwVersion.Version.Rev, IafwVersion.Version.Build));
  if (IafwVersion.Bits.TestBuild == 1) {
    MmrcVitalPrint((MMRC_DBG_MIN, "<!> Test Build DEVID: %d \n", IafwVersion.Bits.DevId));
  }
  MmrcVitalPrint((MMRC_DBG_MIN, "+ MMRC REVISION ID %d.%02d  \n", VERSION_MAJOR, VERSION_MINOR));
  DisplayInputParameters(MrcData, CapsuleStartIndex, StringIndex, Channel);

  return MMRC_SUCCESS;
}

/**
  ReadApicFreqOffset

  @param[in]  None

  @retval  ApicFreq
**/
UINT8
ReadApicFreqOffset()
{
  UINT32  ApicFreq;

  ApicFreq = (UINT32) AsmReadApicFreq ();
  ApicFreq = ((ApicFreq & 0x0000FF00) >> 8);

  return (UINT8) ApicFreq;
}

/**
  InternalX86GetTimerTick

  @param[in]  ApicBase

  @retval  INT32
**/
INT32
InternalX86GetTimerTick (
  UINTN                     ApicBase
)
{
#if SIM || JTAG || MINIBIOS
  return 0;
#else
  return Mmio32Read (ApicBase + 0x390);
#endif
}

/**
  It delays the execution in the MRC for 'DelayNs' nanoseconds
  There is a restriction this delay should be a multiple 50

  @param[in]  DelayNs   Delay specified in nanoseconds. It should be a mutiple of 50

  @retval  None
**/
VOID
McStallNanoSecond (
  IN        UINT32    DelayNs
)
{
#if SIM || JTAG
  return;
#else
  UINTN             ApicBase;
  UINT32            Buffer32;
  UINT32            Ticks;
  LOCAL_APIC_DCR    Dcr;
  UINT32            PsfClock;
  UINT32            Divisor;

  ApicBase = (AsmReadApicBase() & 0xFFFFF000);

  Dcr.Data32 =  Mmio32Read (ApicBase + 0x3e0);
  Divisor = Dcr.Bits.DivideValue1 | (Dcr.Bits.DivideValue2 << 2);
  Divisor = (Divisor + 1) & 0x7;
  Divisor = (1 << Divisor);

  PsfClock = mTimerLibLocalApicFrequencies[ReadApicFreqOffset()];
  Buffer32 = (PsfClock / Divisor);
  ///
  /// The target timer count is calculated here
  /// RoundUp the delay, and convert to a multiple of '50'(x)
  /// 1Seg / '20'(y) = 50ms / '1000000'(z) (due to adjust in mTimerLibLocalApicFrequencies) = 50nS
  /// x * y * z = 1000000000
  ///
  DelayNs = (DelayNs + 25) / 50;
  Ticks = ((Buffer32 * DelayNs) / 20);
  Mmio32Write (ApicBase + 0x380, Ticks);

  while (InternalX86GetTimerTick (ApicBase) != 0);
#endif
}

/**
  GetCoreStepping

  @param[in]  None

  @retval  UINT8
**/
UINT8
GetCoreStepping (
)
{
  UINT8 Data8;
#ifndef SIM
  Data8 = PCI_CFG_32B_READ_CF8CFC (0x00, 0x00, 0x00, 0xf8) & 0xff;
#else
  //
  // If in simulation, force A0 stepping.
  //
  Data8 = 0x00;
#endif
  return Data8;
}

/**
  GetCpuType

  @param[in]  None

  @retval  Data16
**/
UINT16
GetCpuType (
)
{
  UINT16 Data16;
#if !defined SIM && !defined JTAG
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
  "\n\t pushl  %%eax"
  "\n\t pushl  %%ebx"
  "\n\t pushl  %%ecx"
  "\n\t pushl  %%edx"
  "\n\t movl   $0x01, %%eax"
  "\n\t cpuid"
  "\n\t and    $0x00000FF0, %%eax"
  "\n\t mov    %%ax, %0"
  "\n\t popl   %%edx"
  "\n\t popl   %%ecx"
  "\n\t popl   %%ebx"
  "\n\t popl   %%eax"
  : "=m"(Data16)
  :
  :
  );
#else //MSFT compiler
  _asm {

    push    eax
    push    ebx
    push    ecx
    push    edx

    mov     eax, 01h
    cpuid

    and     eax, 00000FF0h

    mov     Data16, ax
    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
  }
#endif
#else
  //
  // If in simulation, force AVN CPU type.
  //
  Data16 = 0x6D0;
#endif
  return Data16;
}
#if defined __GNUC__
#pragma GCC pop_options
#endif

/**
  Because setup options can only be read after second boot; this
  function includes default values for all the mrc inputs that
  will later be read from system setup options

  @param[in]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
MrcInitializeSetupOptionsDefaults (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  MrcData->MrcInputs.ChannelHashMask         = 0x36;
  MrcData->MrcInputs.SliceHashMask           = 0x9;
  MrcData->MrcInputs.SlicesChannelsEnabled   = 0;
  MrcData->MrcInputs.InterleavedMode         = 2;
  MrcData->MrcInputs.ForceMin2xRefresh       = 0;
  MrcData->MrcInputs.DualRankSupportEnabled  = 1;

  return MMRC_SUCCESS;
}
/**
  Initilialize the MRC parameters with default values and/or from setup options

  @param[in]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
MrcInitializeParameters (
  IN  OUT   MMRC_DATA   *MrcData
)
{
  UINT16 SaveBootMode;

  SaveBootMode = MrcData->BootMode;
  ///
  /// Initialize data structures
  ///
  MmrcMemset (MrcData, 0, sizeof (MMRC_DATA));
  ///
  /// Set level of MRC serial debug messages.
  ///
  MrcData->MrcDebugMsgLevel = MMRC_DBG_MIN;

  ///
  /// Default. This will be overriden later.
  ///
  MrcData->BootMode = S5;
#if RMT_PRODUCTION_MODE_SUPPORT == 1
  ///
  /// Default for Production RMT Check
  ///
  MrcData->NvData.MrcParamsSaveRestore.MarginCheckResultState = 0;
#endif //RMT_PRODUCTION_MODE_SUPPORT
  ///
  /// PowerOn is set to 1 initially for power on to enforce
  /// safe settings.  Once memory is stable, we can set this
  /// to 0 to enable additional features and power savings.
  ///
  //MrcData->PowerOn = 0;
  ///
  /// set dummy channel 0 to true for executing the bar address read/write.
  /// this will eventually be overriden by the gathersocConfig function.
  ///
   MrcData->NvData.MrcParamsSaveRestore.Channel[0].Enabled = TRUE;

  ///
  /// EXP_LOOP_CNT field of CPGC_CMD_CTL. This CANNOT be less than 4.
  ///
  MrcData->MrcTrainingCpgcExpLoopCnt = 8;
  MrcData->MrcRmtCpgcExpLoopCnt = 12;
//  MrcData->MrcCPGCNumBursts = 6;

#if MEM_SV_HOOKS
  ///
  /// Memory Select between performance or power savings
  ///
  MrcData->Favor = 0;
  MrcData->InputCustomFrequency = 0;
  MrcData->InputCustomVoltage = 0;
  ///
  /// Disable by default any voltage override
  ///
  MrcData->InputRun125VDimmAt135V = 0;
  MrcData->InputRun135VDimmAt150V = 0;
  ///
  /// Parallel Enable and
  /// Memory for LAKEMORE_SUPPORT
  ///
  MrcData->OdlaParalPrtEn = 0;
  MrcData->MemoryDepth = 0;
#endif // MEM_SV_HOOKS
  ///
  /// Scrambler Enabled by default
  ///
  MrcData->EnableScrambler = 1;
  MrcData->NvData.MrcParamsSaveRestore.SystemMemorySize = 0;
  MrcData->NumberOfEnabledChannels[TypeLpDdr3] = 0;
  MrcData->NumberOfEnabledChannels[TypeLpDdr4] = 0;
  ///
  /// Slow power down exit Disabled by default
  ///
  //MrcData->EnableSlowPowerDownExit = 0;
  ///
  /// Select support for AnalogDll
  ///
  MrcData->DigitalDll = 1;
  MrcInitializeSetupOptionsDefaults(MrcData);

  MrcData->BootMode = SaveBootMode;

  return MMRC_SUCCESS;
}

/**
  This function gets the platform setup

  @param[in,out]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
GetPlatformSettings (
  IN  OUT   MMRC_DATA   *MrcData
)
{

  UINT32        Data32;
#if !defined SIM && !defined JTAG
  UINT8         Channel;
  //  UINT32        MchBar;
  //  UINT32        P2sbBar;
#endif

  Data32 = 0;

#if !defined SIM && !defined JTAG

  //MmrcExecuteTask (MrcData, ReadBarAddress, NO_PRINT, 0);
  //
  //read mchbar 15:31
  //
  MrcData->DynamicVars[0][MCHBAR] = PCI_CFG_32B_READ_CF8CFC(0x0, 0x0, 0x0, 0x48) & (~BIT0);
  //
  //Read ECBASE 28:31
  //
  MrcData->DynamicVars[0][ECBASE_LO] = PCI_CFG_32B_READ_CF8CFC(0x0, 0x0, 0x0, 0x60);
  //
  //Read ECBASE enable bit
  //
  MrcData->DynamicVars[0][ECBASE_EN] = MrcData->DynamicVars[0][ECBASE_LO] & BIT0;

  MrcData->DynamicVars[0][P2SBBAR] = (PCI_CFG_32B_READ_CF8CFC(0x0, 0xD, 0x0, 0x10) & 0xFFFFFFF0);

  MrcData->P2sbBar = (UINT32) MrcData->DynamicVars[0][P2SBBAR];
  MrcData->MchBar = (UINT32) MrcData->DynamicVars[0][MCHBAR];

  if (MrcData->DynamicVars[0][ECBASE_EN] == 0) {
    MrcData->EcBase = EC_BASE;
    MrcData->DynamicVars[0][ECBASE_LO] = EC_BASE >> 28;//RShiftU64 (EC_BASE, 28);
    PCI_CFG_32B_WRITE_CF8CFC(0x0, 0x0, 0x0, 0x60, EC_BASE | BIT0);
    PCI_CFG_32B_WRITE_CF8CFC(0x0, 0x0, 0x0, 0x64, 0x00000000);
    //MmrcExecuteTask (MrcData, WritePCIEXBAR, NO_PRINT, 0);
  } else {
    MrcData->EcBase = MrcData->DynamicVars[0][ECBASE_LO] & (~BIT0);
  }
  // MmrcDebugPrint ((MMRC_DBG_MAX, "EcBase = %08X\n", MrcData->EcBase));

  if (MrcData->DynamicVars[0][MCHBAR] == 0) {
    MrcData->DynamicVars[0][MCHBAR] = MCHBAR_BASE >> 15;
    MmrcExecuteTask (MrcData, WriteMchBarAddress, NO_PRINT, 0);
    MrcData->MchBar  = MCHBAR_BASE;
  }

  if (MrcData->DynamicVars[0][P2SBBAR] == 0) {
    MrcData->DynamicVars[0][P2SBBAR] = P2SB_BASE >> 24;
    MmrcExecuteTask (MrcData, WriteBarAddress, NO_PRINT, 0);
    MrcData->P2sbBar = P2SB_BASE;
  }

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    MrcData->DynamicVars[Channel][MCHBAR]  = MrcData->MchBar;
    MrcData->DynamicVars[Channel][P2SBBAR] = MrcData->P2sbBar;
  }

  //
  // Temporary not needed. Need to revisit
  //

  if (MrcData->SmbusBar == 0) {
    MrcData->SmbusBar = PCI_CFG_16B_READ (MrcData->EcBase, MC_BUS, 0x1F, 0x1, 0x20) & ~ (BIT1 | BIT0);
  }


  if (MrcData->PmcBar == 0) {
    MrcData->PmcBar = PCI_CFG_32B_READ (MrcData->EcBase, MC_BUS, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, PMC_BAR_BASE_ADDRESS);
    //MrcData->PmcBar = PCI_CFG_32B_READ_CF8CFC (MC_BUS, PCI_DEVICE_NUMBER_PMC, PCI_FUNCTION_NUMBER_PMC, PMC_BAR_BASE_ADDRESS);
    MrcData->PmcBar &= 0xfffff000;
    //MmrcDebugPrint ((MMRC_DBG_MED, "PmcBar = %08X\n", MrcData->PmcBar));
  } else {
    MrcData->PmcBar = PMC_BASEADDRESS;
  }

#if MINIBIOS
  MrcData->PmcBar = PMC_BASEADDRESS;
#endif

#endif  //#if !defined SIM && !defined JTAG


  return MMRC_SUCCESS;
}
/**
  This function determines which kind of boot path needs to be followed in MRC flow

  @param[in]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
DetermineBootMode (
  IN  OUT   MMRC_DATA   *MrcData
)
{

  UINT8               ColdBootStatus;
  UINT8               ColdResetStatus;
  UINT8               WarmResetStatus;
  UINT8               DramInSelfRefresh;
  UINT8               DramInitStatus;
  UINT32              Data32;
  BOOLEAN             ColdBootRequired;
#ifdef BXTPMRC
  UINT32              SmramLocation;
#endif
#ifdef FSP_FLAG
  FSPM_UPD            *FspmUpd;
#endif

  PRINT_FUNCTION_INFO;
  ///
  /// Read channel 0's warm reset indicators to determine if the system has just undergone
  /// a warm reset.
  ///
  DramInSelfRefresh = 0;

  //MmrcExecuteTask (MrcData, CheckForWarmReset, NO_PRINT, 0);
  ///
  /// Read warm reset/cold boot/cold reset status bit from PMC GCR PMCON1 register BIT 25 warm_reset_sts
  ///
  Data32 = Mmio32Read(MrcData->PmcBar+PMC_GCR_BASE_ADDRESS+PMC_GCR_GEN_PMCON1);
  WarmResetStatus = (UINT8) ((Data32 & PMC_GCR_GEN_PMCON1_WARM_RESET_STS) >> 25);
  ColdBootStatus  = (UINT8) ((Data32 & PMC_GCR_GEN_PMCON1_COLD_BOOT_STS)  >> 27);
  ColdResetStatus = (UINT8) ((Data32 & PMC_GCR_GEN_PMCON1_COLD_RESET_STS) >> 26);
  DramInitStatus  = (UINT8) ((Data32 & PMC_GCR_GEN_PMCON1_DRAM_INIT_BIT_STS) >> 23);
#ifdef BXTPMRC
  if (MrcData->BxtSeries == SERIES_BXT_P || MrcData->BxtSeries == SERIES_BXT_1) {
    SmramLocation = PcdGet32 ( PcdPmcSsramBaseAddress0 ) + SMRAM_MRC_OFFSET;  //MRC reserved DWORD
    DramInSelfRefresh = (UINT8) (0x1 & Mmio32Read(SmramLocation));            //BIT0 = memory contents preserved when set
    MmrcDebugPrint ((MMRC_DBG_MIN, "MRC: DRAM in SR: %d\n", DramInSelfRefresh));
  }
#endif

  if (MrcData->CpuStepping == STEPPING_A0) {
    Data32 = Mmio32Read(MrcData->MchBar+P_CR_MC_BIOS_REQ_0_0_0_MCHBAR_PUNIT_BOXTYPE_REG);
    if (Data32 == 0xffffffff) {
      ///<punit not found - enable Sus_itp bypass flow
      MmrcDebugPrint ((MMRC_DBG_MIN, "Bad Punit detected!  Forcing A0 Bootpath\n"));
      MrcData->BootMode = A0;
    }  else {
      MmrcDebugPrint ((MMRC_DBG_MIN, "Punit detected functional!\n"));
    }
  }

  if ((WarmResetStatus == 1) || (ColdResetStatus == 1)) {
    if (MrcData->BxtSeries == SERIES_BXT_P || MrcData->BxtSeries == SERIES_BXT_1) {

      if ( !IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) && (CheckColdBootRequired (MrcData))) {
        MrcData->BootMode = S5;
        MmrcDebugPrint ((MMRC_DBG_MIN, "Reset path failed, perform S5 path.\n"));
      } else if ((DramInSelfRefresh == 1) && (MrcData->MrcParamsValidFlag == 1)) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Self-refresh detected! "));
        //
        // A warm reset happened after MRC completed and DRAM was in self refresh.
        // We can take the S0 to fast track through MRC since the DDRIO IO is already
        // initialized.
        //
        if (WarmResetStatus == 1) {
          MrcData->BootMode = S0W;
          MmrcDebugPrint ((MMRC_DBG_MIN, "S0 Warm"));
        } else {
          //Cold Reset PATH - does the memory stay powered up?
          MmrcDebugPrint ((MMRC_DBG_MIN, "S0 Cold"));
          MrcData->BootMode = S0C;
        }
        MmrcDebugPrint ((MMRC_DBG_MIN, " reset detected.\n"));
      } else if ((DramInSelfRefresh == 1) && (MrcData->MrcParamsValidFlag == 0)) {
        //
        // A warm reset happened but the MRC Training data is not available
        // Perform a full S5 boot in this case.
        //
        MrcData->BootMode = S5;
        MmrcDebugPrint ((MMRC_DBG_MIN, "S0 Warm reset detected with no FB Data\n"));
      } else if (DramInitStatus) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "MRC didn't start/complete last time. Full training.\n"));
        MrcData->BootMode = S5;
        //Clear the "mrc in progress" aka DramInitStatus bit in the PMC
        Mmio32Write((MrcData->PmcBar+PMC_GCR_BASE_ADDRESS+PMC_GCR_GEN_PMCON1), (Data32 & ~BIT23));
      } else if (ColdResetStatus == 1) {
        if (MrcData->MrcParamsValidFlag == 0)
          MrcData->BootMode = S5;
        else
          MrcData->BootMode = FB;
      } else {
        // //
        // // We're here if a warm reset happened, and this occurred either during MRC or
        // // we detected that the memory was not in self refresh for whatever reason.
        // // Need to force a powergood reset and take the cold path through MRC.
        // //

        MmrcDebugPrint ((
          MMRC_DBG_MIN,
          "S0:DBG:Warm reset detected before memory was properly initialized or memory was not\nin self refresh. Forcing powergood reset.\n"
          ));
#ifdef FSP_FLAG
        FspmUpd = (FSPM_UPD *)GetFspMemoryInitUpdDataPointer ();
        if (!(FspmUpd->FspmConfig.EnableResetSystem)) {
          FspApiReturnStatusReset (FSP_STATUS_RESET_REQUIRED_COLD);
        }
#endif
        IoOut8 (0xCF9, 0x0);
        IoOut8 (0xCF9, 0xE);
#if !defined SUSSW
#if defined __GNUC__  // GCC compiler
  __asm__ __volatile__ (
      "hlt"
  );
#else // MSFT compiler
      _asm hlt
#endif
#endif
      }
    } else {  // not PO
      if (DramInitStatus) {
        MmrcDebugPrint ((MMRC_DBG_MIN, "MRC didn't start/complete last time. Full training.\n"));
        MrcData->BootMode = S5;
      } else {
        MmrcDebugPrint ((MMRC_DBG_MIN, "Reset detected. Skipping training.\n"));
        MrcData->BootMode = S0;
      }
    }
  } else {   ///< Not a WARM RESET
    MmrcDebugPrint ((MMRC_DBG_MIN, "Cold boot detected.\n"));

    //check S5 and FB path here
    if ( IS_MEM_DOWN(MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.MemoryDown) ) {
      if (MrcData->MrcParamsValidFlag == 1) {
        MrcData->BootMode = FB;
      } else {
        MrcData->BootMode = S5;
      }
    } else {
      //DDR3L/ECC sodimm, check dimm info

      if (MrcData->MrcParamsValidFlag == 0)
        ColdBootRequired = 1;
      else
        ColdBootRequired = CheckColdBootRequired (MrcData);

      if (ColdBootRequired) {
        //DEBUG ((EFI_D_INFO, "DetectDimms\n"));
        MrcData->BootMode = S5;
        MmrcDebugPrint ((MMRC_DBG_MIN, "FBpath failed, perform S5 path.\n"));
      } else {
        MrcData->BootMode = FB;
      }
    }
  } ///< Not WARM RESET path

//DBMExit:

  if (MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtMode != 0) {
    MmrcDebugPrint ((MMRC_DBG_MIN, " >>> ### Rank Margin Tool Enabled on smip= [%d] ### <<<\n", MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtMode));
    MrcData->MrcRmtSupport = (UINT8)MrcData->NvData.MrcParamsSaveRestore.DramPolicyData.RmtMode;
  }

  //
  // Final check - if we are executing any sort of fast path through MRC
  // which relies on restoring register values from ones which were saved
  // on a previous boot, make sure those saved values are valid before
  // attempting to restore. If they are not, force a powergood reset and
  // retrain memory from scratch.
  //
  /*if ((MrcData->BootMode != S5) && (MrcData->CpuStepping != STEPPING_A0)) {
    if (MrcData->MrcParamsValidFlag == 0) {
      ///
      /// Do cold boot
      ///
      //MmrcDebugPrint ((MMRC_DBG_MIN, "Forcing power good reset. Variables are not valid\n"));
      IoOut8 (0xCF9, 0x0);
      IoOut8 (0xCF9, 0xE);
#if !defined SUSSW
      _asm hlt
#endif
    }
  }*/
  return MMRC_SUCCESS;
}

/**
  Enables the High Precision Event Timer

  @param[in]  MrcData   Host structure for all MRC global data.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
McEnableHpet (
  IN  OUT   MMRC_DATA   *MrcData
)
{
#if defined SIM || defined JTAG || defined SUSSW || defined MINIBIOS
  return MMRC_SUCCESS;
#else

  ///
  /// Set HPET Timer enable to start counter spinning
  ///
  Mmio32Or (ICH_HPET_BASE_ADDRESS + R_PCH_PCH_HPET_GCFG, B_PCH_PCH_HPET_GCFG_EN);
  return MMRC_SUCCESS;
#endif
}

/**
  Disables the High Precision Event Timer

  @param[in]  MrcData   Host structure for all MRC global data.
  @param[in]  Channel   Current Channel being examined.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
McDisableHPET (
  IN  OUT   MMRC_DATA   *MrcData,
  IN        UINT8        Channel
)
{

#if SIM || JTAG
  return MMRC_SUCCESS;
#else
  UINT32  Data32;
  ///
  /// Clear HPET Timer enable to stop counter spinning
  ///
  Mmio32And (ICH_HPET_BASE_ADDRESS + R_PCH_PCH_HPET_GCFG, ~ (B_PCH_PCH_HPET_GCFG_LRE | B_PCH_PCH_HPET_GCFG_EN) );
  Data32 = Mmio32Read (ICH_HPET_BASE_ADDRESS + R_PCH_PCH_HPET_GCFG); // Read back to flush posted write

  return MMRC_SUCCESS;
#endif
}

/**
  The function takes pointer to previous 32 bit value and modifies it to next value.
  32 bit LFSR with characteristic polinomial:  X^32 + X^22 +X^2 + X^1

  @param[in]  LfsrWordPtr    Pointer to previous value

  @retval  None
**/
VOID Lfsr16 (
  IN  OUT     UINT16  *LfsrWordPtr
)
{
  UINT16 bit;
  UINT16 lfsr;
  UINT8 i;

  lfsr = *LfsrWordPtr;

  for (i = 0; i < 16; i++) {
    bit  =   1 ^ (lfsr & BIT0)        ;
    bit  = bit ^ ((lfsr & BIT1)  >>  1);
    bit  = bit ^ ((lfsr & BIT2)  >>  2);
    bit  = bit ^ ((lfsr & BIT6)  >>  6);
    lfsr = ((lfsr >> 1) | (bit << 15) );
  }
  *LfsrWordPtr = lfsr;

  return;
}

/**
  This function configures the scrambler

  @param[in]  MrcData            Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  Success
  @retval  Failure
**/
MMRC_STATUS
SetScrambler (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  UINT16              lfsr;
  lfsr = 0;

  if (MrcData->EnableScrambler) {

    //
    // 16 bit seed is always stored in BIOS NVM.
    //
    lfsr = MrcData->NvData.BootVariableNvData.ScramblerSeed[Channel];
    //
    // On cold boot, or on a boot where the scrambler seed wasn't successfully saved
    // into NVRAM...
    //
    if (MrcData->BootMode == S5 || (MrcData->BootMode == FB) || (MrcData->BootMode != S5 && MrcData->MrcParamsValidFlag == 0) ) {
      //
      // Factory value is 0 and in first boot, a clock based seed is loaded.
      //
      if (lfsr == 0) {
        lfsr = (UINT16) (get_initial_seed() & 0x0000FFFF); // get seed and make sure it is not all 1's
      }
      //
      // Need to replace scrambler.
      // Get next 16bit LFSR which is the previous scrambler vector.
      //
       else {
         Lfsr16 (&lfsr);
       }
      MrcData->NvData.BootVariableNvData.ScramblerSeed[Channel] = lfsr;               // save new seed.
    } // if (cold_boot)

    ///
    /// In warm boot or S3 exit, we have the previous seed.
    /// In cold boot, we have the last 16bit LFSR which is the new seed.
    ///
    //Lfsr16 (&lfsr);

    MrcData->DynamicVars[Channel][SCRAMBLER_KEY]   = MrcData->NvData.BootVariableNvData.ScramblerSeed[Channel];
    MmrcExecuteTask (MrcData, EnableScrambling, NO_PRINT, Channel);
  } else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "SetScrambler: fuse is set.  Skipping scrambler init\n"));
  }

  return MMRC_SUCCESS;
}
/**
  get_initial_seed

  @param[in]  None

  @retval  Data32
**/
UINT32
get_initial_seed ()
{
  UINT32  Data32;
#ifndef SIM
#ifndef _MSC_EXTENSIONS
    asm("pushl %%eax;"
        "pushl %%ebx;"
        "pushl %%ecx;"
        "pushl %%edx;"
        "rdtsc;"
        "mov   %%eax, %0;"
        "popl  %%edx;"
        "popl  %%ecx;"
        "popl  %%ebx;"
        "popl  %%eax;"
         : "=m"(Data32) : :
    );
#else
  _asm {

    push    eax
    push    ebx
    push    ecx
    push    edx

    //
    // Per d-unit HAS, attempt to use the DRNG for our seed.
    // Per DRNG HAS, DRNG is accessed through the RDRAND CPU instruction.
    // If CPUID ECX bit 30 is not set, cpu doesn't support RDRAND
    //
    mov     eax, 1
    cpuid
    bt      ecx, 30
    jnc     rdrand_not_supported

    RDRAND_EAX
    jc      rdrand_done
    //
    // If carry is clear, then random number "not available" per instruction definition
    //

    rdrand_not_supported:
    //
    // If either RDRAND is not supported by the CPU or is unavailable, fall back on use of cpu tsc
    //
    rdtsc

    rdrand_done:
    mov     Data32, eax

    pop     edx
    pop     ecx
    pop     ebx
    pop     eax
  }
#endif
#else
  Data32 = 1;
#endif
  return Data32;
}

/**
  Calculates the largest integer that is both
  a power of two and less than Input

  @param[in]  Operand   value to calculate power of two and less than Input

  @retval    the largest integer that is both a power of
             two and less than Input
**/
UINT32
GetPowerOfTwo (
  IN        UINT32        Operand
)
{
  UINT8 Bitpos;
  Bitpos = 0;

  if (Operand == 0) {
    return 0;
  }

  while (Operand != 0) {
    Operand >>= 1;
    Bitpos++;
  }

  Operand = 1;
  Bitpos--;
  while (Bitpos != 0) {
    Operand <<= 1;
    Bitpos--;
  }

  return Operand;
}

/**

@todo   add description

**/
MMRC_STATUS
  SaveRestoreAlgos (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel,
  SAVE_RESTORE_TASK Task
  )
{
  UINT8 Strobe;
  UINT8 Rank;
  UINT8 Bit;
  UINT32 temp;
  UINT8 i;
  UINT8 BitBased = 0;
  UINT8 MaxStrobe = 1;
  UINT8 MaxRank = 1;

  for (i = 0; i < RestoreDataDescriptorTableCount; i++) {
    BitBased = 0;
    MaxStrobe = 1;
    MaxRank = 1;
    if ((RestoreDataDescriptorTable[i].AccessType & STROBE_BASED) == STROBE_BASED) {
      MaxStrobe = MAX_STROBES;
    }
    if((RestoreDataDescriptorTable[i].AccessType & RANK_BASED) == RANK_BASED) {
      MaxRank = MAX_RANKS;
    }
    if((RestoreDataDescriptorTable[i].AccessType & BIT_BASED) == BIT_BASED) {
      BitBased = 1;
    }

    for (Rank = 0; Rank < MaxRank; Rank++) {
      for (Strobe = 0; Strobe < MaxStrobe; Strobe++) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Strobe))
          continue;
        if (Task == SAVE) {
          if (BitBased) {
            for (Bit = 0; Bit < MAX_BITS; Bit++) {
              GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RestoreDataDescriptorTable[i].TypeIndex, CMD_GET_REG_UC, &temp);
              MmrcDebugPrint ((MMRC_DBG_TRACE, "PB FBs:CH%02d RK%02d ST%02d Idx:%2d RDDTTI:%x VAL:0x%08x\n", Channel, Rank, Strobe, i, RestoreDataDescriptorTable[i].TypeIndex, temp));
            }
          } else {
            GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, RestoreDataDescriptorTable[i].TypeIndex, CMD_GET_REG_UC, &temp);
            MmrcDebugPrint ((MMRC_DBG_TRACE, "   FBs:CH%02d RK%02d ST%02d Idx:%2d RDDTTI:%x VAL:0x%08x\n", Channel, Rank, Strobe, i, RestoreDataDescriptorTable[i].TypeIndex, temp));
          }
        } else {
          if ( (RestoreDataDescriptorTable[i].RestoreOnS3Resume == FALSE) && ((MrcData->BootMode & S3) == S3) ) {
            // Skip programming PHY registers on S3 resume, only restore DUNIT registers
            continue;
          } else {
            if (BitBased) {
              for (Bit = 0; Bit < MAX_BITS; Bit++) {
                GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RestoreDataDescriptorTable[i].TypeIndex, CMD_GET_CACHE, &temp);
                MmrcDebugPrint ((MMRC_DBG_TRACE, "PB FBr:CH%02d RK%02d ST%02d Idx:%2d RDDTTI:%x VAL:0x%08x\n", Channel, Rank, Strobe, i, RestoreDataDescriptorTable[i].TypeIndex, temp));
                GetSetDdrioGroup (MrcData, 0, Channel, 0, Rank, Strobe, Bit, MrcData->NvData.MrcParamsSaveRestore.DVFSFreqIndex, 0xFF, RestoreDataDescriptorTable[i].TypeIndex, CMD_SET_VAL_FC, &temp);
              }
            } else {
              GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, RestoreDataDescriptorTable[i].TypeIndex, CMD_GET_CACHE, &temp);
              MmrcDebugPrint ((MMRC_DBG_TRACE, "   FBr:CH%02d RK%02d ST%02d Idx:%2d RDDTTI:%x VAL:0x%08x\n", Channel, Rank, Strobe, i, RestoreDataDescriptorTable[i].TypeIndex, temp));
              GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, RestoreDataDescriptorTable[i].TypeIndex, CMD_SET_VAL_FC, &temp);
            }
          }
        }
      }
    }

  }
  return MMRC_SUCCESS;
}

MMRC_STATUS
  RestoreMuxCode (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  UINT8 Strobe;
  UINT8 Rank;
  UINT32 temp;

    for (Rank = 0; Rank < 1; Rank++) {
      for (Strobe = 0; Strobe < MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].MaxDq[0]; Strobe++) {
        if (!IsStrobeValid (MrcData, Channel, Rank, Strobe))
          continue;

            GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, MuxcodeMin, CMD_GET_CACHE, &temp);
            MmrcDebugPrint ((MMRC_DBG_TRACE, "FBr:CH%02d RK%02d ST%02d RDDTTI:%x VAL:0x%08x\n", Channel, Rank, Strobe,  MuxcodeMin, temp));
            GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, MuxcodeMin, CMD_SET_VAL_FC, &temp);
            GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, MuxcodeMax, CMD_GET_CACHE, &temp);
            MmrcDebugPrint ((MMRC_DBG_TRACE, "FBr:CH%02d RK%02d ST%02d RDDTTI:%x VAL:0x%08x\n", Channel, Rank, Strobe,  MuxcodeMin, temp));
            GetSetDdrioGroup2(MrcData, Channel, Rank, Strobe, MuxcodeMax, CMD_SET_VAL_FC, &temp);

      }
    }

  return MMRC_SUCCESS;
}


/**

@todo   add description

**/
MMRC_STATUS
SaveAlgos(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  ProjectSpecificSaveRestores(MrcData, CapsuleStartIndex, StringIndex, Channel, SAVE);

  return SaveRestoreAlgos(MrcData, CapsuleStartIndex, StringIndex, Channel, SAVE);
}

/**

@todo   add description

**/
MMRC_STATUS
  RestoreAlgos(
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
  )
{
  MMRC_STATUS Status = MMRC_SUCCESS;

  ProjectSpecificSaveRestores(MrcData, CapsuleStartIndex, StringIndex, Channel, RESTORE);
  Status = SaveRestoreAlgos(MrcData, CapsuleStartIndex, StringIndex, Channel, RESTORE);

  return Status;
}

/**
  This function verifies DQ swizzling does not cross LPDDR4 x16 border per PDG guideslines.

  @param[in]  MrcData            Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  Success               no border violation
  @retval  MMRC_FAILURE          A mismatch will result failure.
**/
MMRC_STATUS
CheckDQSwizzle (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  MMRC_STATUS Status = MMRC_SUCCESS;

  typedef enum {
    DQLow = 0,
    DQHigh,
  } DQ_Swizzle_Type;

  const UINT8 MidBit = NUM_BITS_PER_DEVICE / 2;
  CHAR8 HumanSwizzle[2][8] = {"x%02dLo", "x%02dHi"};
  CHAR8 bufSwizzle[8];

//[-start-161129-IB03090436-modify]//
  DQ_Swizzle_Type thisBit, chkBit = DQLow;
//[-end-161129-IB03090436-modify]//
  UINT8 *Dimm2DunitMapping = (UINT8 *) MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Dimm2DunitMapping;  // Single-dimensional array.  The index into the array is the swizzled location.  The array value is the unswizzled destination
  UINT8 CurrBit;
//[-start-161129-IB03090436-add]//
  UINT16 DramType;
//[-end-161129-IB03090436-add]//

#if ( MRC_DEBUG_DISABLE==1 )
  // Skip compiling strings in the debug disable build
#else
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  CHAR8 dictateStr[] = "CH[%d] DQ[%d] dictates %a (%d) for %02x:%02x\n";
  MmrcVitalPrint((MMRC_DBG_MIN, "%a %a %d\n", __FILE__, __FUNCTION__, __LINE__));
#endif
#endif

  if ( ! MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled ) {
    MmrcDebugPrint ((MMRC_DBG_MIN, g_ChDisabledStr, Channel,
                     MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled ));
    return MMRC_SUCCESS;
  }

#if (LPDDR4_SUPPORT < 1)
  MmrcDebugPrint ((MMRC_DBG_MIN, "No support for LPDDR4(%d)\n", LPDDR4_SUPPORT ));
  return MMRC_SUCCESS;
#else
//[-start-161129-IB03090436-modify]//
  DramType = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType;
//[-end-161129-IB03090436-modify]//

  if (DramType != TypeLpDdr4) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d].DramType (%d) not LPDDR4, skipping\n", Channel,
                     MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType ));
    return MMRC_SUCCESS;
  }
#endif

//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d] MidBit=%d\n", Channel, MidBit));
#endif

  //create human readable strings for enum based on calculated midbit
  for ( CurrBit=0; CurrBit <= 1; CurrBit++ ) {
   AsciiSPrint ( bufSwizzle, sizeof(bufSwizzle), HumanSwizzle[CurrBit], MidBit );
   AsciiStrCpyS ( HumanSwizzle[CurrBit], sizeof(HumanSwizzle[CurrBit]), bufSwizzle );
  }

  //parse all DQs ensuring they don't cross the MidBit boundry
  // ie the LSB of the boundary dictates where the remaining bits reside.
  for (CurrBit = 0; CurrBit < NUM_BITS_PER_DEVICE; CurrBit++) {
   switch ( CurrBit ) {
    case 0:
     chkBit = (Dimm2DunitMapping[CurrBit] >= MidBit) ? DQHigh : DQLow;
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
     MmrcDebugPrint ((MMRC_DBG_MIN, dictateStr, Channel, CurrBit, HumanSwizzle[chkBit], chkBit, 0, MidBit));
#endif
     continue; //no need to process this bit go to next bit
     break;

    default:
     if ( CurrBit == MidBit ) {
      chkBit = (Dimm2DunitMapping[CurrBit] >= MidBit) ? DQHigh : DQLow;
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
      MmrcDebugPrint ((MMRC_DBG_MIN, dictateStr, Channel, CurrBit, HumanSwizzle[chkBit], chkBit, MidBit, NUM_BITS_PER_DEVICE-1));
#endif
      continue; //no need to process this bit go to next bit
     }
   }

   thisBit = (Dimm2DunitMapping[CurrBit] >= MidBit) ? DQHigh : DQLow;
   if ( thisBit != chkBit ) {
    Status = MMRC_FAILURE;
    MmrcVitalPrint((MMRC_DBG_MIN, "CH[%d] DQ[%d] crosses boundary %a!=%a!\n", Channel, CurrBit, HumanSwizzle[thisBit], HumanSwizzle[chkBit]));
   }
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
   else {
    MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d] DQ[%d] doesn't cross boundary %a==%a.\n", Channel, CurrBit, HumanSwizzle[thisBit], HumanSwizzle[chkBit]));
   }
#endif
  }

  //check done; report errors and hang
  if ( Status != MMRC_SUCCESS ) {
   MmrcVitalPrint((MMRC_DBG_MIN, "%a detected DQ Swizzle guideline violation on CH[%d]!\nFatal Error!\n", __FUNCTION__, Channel));
  }
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  else {
   MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d] no DQ swizzle violations detected...\n", Channel));
  }
#endif

#if (CHECKDQSWIZZLE_FATAL_ERROR == 1)
  return Status;
#else
  return MMRC_SUCCESS;
#endif
}

/**
  This function verifies RAM Device matches finalized MRC configuration to prevent misconfiguration.

  @param[in]  MrcData            Host structure for all MRC global data.
  @param[in]  CapsuleStartIndex
  @param[in]  StringIndex
  @param[in]  Channel            Current Channel being examined.

  @retval  Success
  @retval  Failure               A mismatch will result in a hang in this function
**/
MMRC_STATUS
VerifyMR8onRAM (
  MMRC_DATA         *MrcData,
  UINT16            CapsuleStartIndex,
  UINT16            StringIndex,
  UINT8             Channel
)
{
  MMRC_STATUS Status = MMRC_SUCCESS;

  JEDEC_MR8_LPDDR3N4_STRUCT mr8;
  UINT16 DramType;
  UINT8  DataWidth;
  MEMORY_DEVICE_DENSITY Density;
  
//[-start-161207-IB07400824-add]//
#if ((LPDDR3_SUPPORT < 1) && (LPDDR4_SUPPORT < 1))
#else
#if ( ( MRC_DEBUG_DISABLE==1 ) || ( MRC_TIMER == 1) )
    // Skip compiling strings in the debug disable build
#else
  CHAR8 typeNStr[] = "CH[%d].DramType(%d) not S%d_SDRAM(%d). Failure!\n";
  CHAR8 densityStr[] = "CH[%d] Density(%d) %amatch MR8.density (%d)%a\n";
  CHAR8 denDecodeNStr[] = "CH[%d].DramType(%d) DeviceDensity%dGb not decodeable. Failure!\n";

  CHAR8 ioOKStr[] = "CH[%d].mr8.Bits.io_width=%d confirmed OK.\n";
  CHAR8 ioMismatchStr[] = "CH[%d].D_DataWidth x%d(%d) with DramType(%x) NOT match MR8.iowidth (%d). Failure!\n";
  CHAR8 ioUnknownStr[] = "CH[%d].D_DataWidth x%d(%d) with unknown DramType(%x). Failure!\n";
#endif
#endif
//[-end-161207-IB07400824-add]//

//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  MmrcVitalPrint((MMRC_DBG_MIN, "%a %a %d\n", __FILE__, __FUNCTION__, __LINE__));
#endif

#if ((LPDDR3_SUPPORT < 1) && (LPDDR4_SUPPORT < 1))
  MmrcDebugPrint ((MMRC_DBG_MIN, "No support LPDDR3(%d)||LPDDR4(%d)\n", LPDDR3_SUPPORT, LPDDR4_SUPPORT ));
  return MMRC_SUCCESS;
#else
//[-start-161207-IB07400824-remove]//
//#if ( ( MRC_DEBUG_DISABLE==1 ) || ( MRC_TIMER == 1) )
//  // Skip compiling strings in the debug disable build
//#else
//  CHAR8 typeNStr[] = "CH[%d].DramType(%d) not S%d_SDRAM(%d). Failure!\n";
//  CHAR8 densityStr[] = "CH[%d] Density(%d) %amatch MR8.density (%d)%a\n";
//  CHAR8 denDecodeNStr[] = "CH[%d].DramType(%d) DeviceDensity%dGb not decodeable. Failure!\n";
//
//  CHAR8 ioOKStr[] = "CH[%d].mr8.Bits.io_width=%d confirmed OK.\n";
//  CHAR8 ioMismatchStr[] = "CH[%d].D_DataWidth x%d(%d) with DramType(%x) NOT match MR8.iowidth (%d). Failure!\n";
//  CHAR8 ioUnknownStr[] = "CH[%d].D_DataWidth x%d(%d) with unknown DramType(%x). Failure!\n";
//#endif
//[-end-161207-IB07400824-remove]//
#endif

  if ( ! MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled ) {
    MmrcDebugPrint ((MMRC_DBG_MIN, g_ChDisabledStr, Channel,
                     MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].Enabled ));
    return MMRC_SUCCESS;
  }

  DramType = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType;

  if ( (DramType != TypeLpDdr3) && (DramType != TypeLpDdr4)  ) {
    MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d].DramType (%d) not LPDDR3/4\n", Channel,
                     MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].DramType ));
    return MMRC_SUCCESS;
  }

  mr8.Raw = ReadMRValue_Deswizzle (MrcData, Channel, JEDEC_MR8, TRUE );
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d] mr8 = 0x%02x\n", Channel, mr8.Raw ));
  MmrcDebugPrint ((MMRC_DBG_MIN, " CH[%d].mr8.Bits.type=%d\n", Channel, mr8.Bits.type));
  MmrcDebugPrint ((MMRC_DBG_MIN, " CH[%d].mr8.Bits.density=%d\n", Channel, mr8.Bits.density));
  MmrcDebugPrint ((MMRC_DBG_MIN, " CH[%d].mr8.Bits.io_width=%d\n", Channel, mr8.Bits.io_width));
#endif

  //verify MR8 Type
  switch (mr8.Bits.type) {
    case S8_SDRAM:
        if ( DramType != TypeLpDdr3 ) {
            Status = MMRC_FAILURE;
            MmrcDebugPrint ((MMRC_DBG_MIN, typeNStr, Channel, DramType, 8, S8_SDRAM  ));
        }
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
        else MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d].DramType(%d) match MR8.type(%d)\n", Channel, DramType, S8_SDRAM ));
#endif
        break;

    case S16_SDRAM:
        if ( DramType != TypeLpDdr4 ) {
            Status = MMRC_FAILURE;
            MmrcDebugPrint ((MMRC_DBG_MIN, typeNStr, Channel, DramType, 16, S16_SDRAM  ));
        }
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
        else MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d].DramType(%d) match MR8.type(%d)\n", Channel, DramType, S16_SDRAM ));
#endif
        break;

    default:
        Status = MMRC_FAILURE;
        MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d].DramType(%d) not known. Failure!\n", Channel, DramType ));
  }

  Density = GetDeviceDensity( MrcData, Channel );

  //verify MR8 Density
  switch ( Density ) {
   case DeviceDensity4Gb:
        if ( DramType == TypeLpDdr3 ) {
          switch ( (JEDEC_LPDDR3_Density) mr8.Bits.density ) {
           case Die4Gb_LPDDR3 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die4Gb_LPDDR3, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else if ( DramType == TypeLpDdr4 ){
          switch ( (JEDEC_LPDDR4_Density) mr8.Bits.density ) {
           case Die4Gb_2GbCh_LPDDR4 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die4Gb_2GbCh_LPDDR4, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else {
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN, denDecodeNStr, Channel, DramType, (UINT8)4 ));
        }
        break;

   case DeviceDensity6Gb:
        if ( DramType == TypeLpDdr3 ) {
          switch ( (JEDEC_LPDDR3_Density) mr8.Bits.density ) {
           case Die6Gb_LPDDR3 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die6Gb_LPDDR3, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else if ( DramType == TypeLpDdr4 ){
          switch ( (JEDEC_LPDDR4_Density) mr8.Bits.density ) {
           case Die6Gb_3GbCh_LPDDR4 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die6Gb_3GbCh_LPDDR4, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else {
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN, denDecodeNStr, Channel, DramType, (UINT8)6 ));
        }
        break;

   case DeviceDensity8Gb:
        if ( DramType == TypeLpDdr3 ) {
          switch ( (JEDEC_LPDDR3_Density) mr8.Bits.density ) {
           case Die8Gb_LPDDR3 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die8Gb_LPDDR3, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else if ( DramType == TypeLpDdr4 ){
          switch ( (JEDEC_LPDDR4_Density) mr8.Bits.density ) {
           case Die8Gb_4GbCh_LPDDR4 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die8Gb_4GbCh_LPDDR4, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else {
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN, denDecodeNStr, Channel, DramType, (UINT8)8 ));
        }
        break;

   case DeviceDensity12Gb:
        if ( DramType == TypeLpDdr3 ) {
          switch ( (JEDEC_LPDDR3_Density) mr8.Bits.density ) {
           case Die12Gb_LPDDR3 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die12Gb_LPDDR3, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else if ( DramType == TypeLpDdr4 ){
          switch ( (JEDEC_LPDDR4_Density) mr8.Bits.density ) {
           case Die12Gb_6GbCh_LPDDR4 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die12Gb_6GbCh_LPDDR4, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else {
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN, denDecodeNStr, Channel, DramType, (UINT8)12 ));
        }
        break;

   case DeviceDensity16Gb:
        if ( DramType == TypeLpDdr3 ) {
          switch ( (JEDEC_LPDDR3_Density) mr8.Bits.density ) {
           case Die16Gb_LPDDR3 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die16Gb_LPDDR3, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else if ( DramType == TypeLpDdr4 ){
          switch ( (JEDEC_LPDDR4_Density) mr8.Bits.density ) {
           case Die16Gb_8GbCh_LPDDR4 : //legal match, good
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "", Die16Gb_8GbCh_LPDDR4, "." ));
               break;

           default :
               Status = MMRC_FAILURE;
               MmrcDebugPrint ((MMRC_DBG_MIN, densityStr, Channel, Density, "NOT ", mr8.Bits.density, ". Failure!" ));
          }
        } else {
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN, denDecodeNStr, Channel, DramType, (UINT8)16 ));
        }
        break;

   default:
        Status = MMRC_FAILURE;
        MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d] Density(%d) not known. Failure!\n", Channel, Density ));
  }

  //verify MR8 IO Width
  DataWidth = MrcData->NvData.MrcParamsSaveRestore.Channel[Channel].D_DataWidth[0];
  switch ( DataWidth ) {
    case 1: // = x16
     if ( DramType == TypeLpDdr3 ) {
        switch ( (JEDEC_LPDDR3_IOW) mr8.Bits.io_width ) {
         case x16 :
          //legal match, good
          MmrcDebugPrint ((MMRC_DBG_MIN, ioOKStr, Channel, mr8.Bits.io_width));
          break;

         default:
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN,
                         ioMismatchStr,
                         Channel, 16, DataWidth, DramType, mr8.Bits.io_width ));
        }
     } else if ( DramType == TypeLpDdr4 ) {
        switch ( (JEDEC_LPDDR4_IOW) mr8.Bits.io_width ) {
         case x16_4I :
          //legal match, good
          MmrcDebugPrint ((MMRC_DBG_MIN, ioOKStr, Channel, mr8.Bits.io_width));
          break;

         default:
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN,
                         ioMismatchStr,
                         Channel, 16, DataWidth, DramType, mr8.Bits.io_width ));
        }
     } else {
        Status = MMRC_FAILURE;
        MmrcDebugPrint ((MMRC_DBG_MIN,
                         ioUnknownStr,
                         Channel, 16, DataWidth, DramType));
     }
     break;

    case 2: // = x32
     if ( DramType == TypeLpDdr3 ) {
        switch ( (JEDEC_LPDDR3_IOW) mr8.Bits.io_width ) {
         case x32 :
          //legal match, good
          MmrcDebugPrint ((MMRC_DBG_MIN, ioOKStr, Channel, mr8.Bits.io_width));
          break;

         default:
          Status = MMRC_FAILURE;
          MmrcDebugPrint ((MMRC_DBG_MIN,
                         ioMismatchStr,
                         Channel, 32, DataWidth, DramType, mr8.Bits.io_width ));
        }
     } else if ( DramType == TypeLpDdr4 ) {
        Status = MMRC_FAILURE;
        MmrcDebugPrint ((MMRC_DBG_MIN,
                         "CH[%d].D_DataWidth x32(%d) with DramType(%x) NOT legal per LPDDR4 spec where MR8.iowidth (%d). Failure!\n",
                         Channel, DataWidth, DramType, mr8.Bits.io_width ));
     } else {
        MmrcDebugPrint ((MMRC_DBG_MIN,
                         ioUnknownStr,
                         Channel, 32, DataWidth, DramType));
     }
     break;

    default:
        Status = MMRC_FAILURE;
        MmrcDebugPrint ((MMRC_DBG_MIN, "CH[%d].D_DataWidth(%d) not known. Failure!\n", Channel, DataWidth ));
  }

  //MR8 check done; report errors and hang
  if ( Status != MMRC_SUCCESS ) {
   MmrcVitalPrint((MMRC_DBG_MIN, "%a mismatch against RAM's mr8 CH[%d]!\nHang on Error\n", __FUNCTION__, Channel));
   //MrcDeadLoop(); //MRC will hang/fail if MMRC_FAILURE is returned to caller
  }
//[-start-161207-IB07400823-modify]//
#if ( MRC_DEBUG_DISABLE == 0 )
//[-end-161207-IB07400823-modify]//
  else {
   MmrcDebugPrint ((MMRC_DBG_TRACE, "CH[%d] config matched mr8\n", Channel));
  }
#endif

#if (VERIFYMR8_FATAL_ERROR == 1)
  return Status;
#else
  return MMRC_SUCCESS;
#endif
}
