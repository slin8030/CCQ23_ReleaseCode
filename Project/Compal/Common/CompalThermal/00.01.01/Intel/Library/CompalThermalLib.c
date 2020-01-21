
#include <CompalThermalLib.h>
#include <CompalEclib.h>
#include <Library/BaseLib.h>

//****************************************************************************************
//*                            Common Routine
//****************************************************************************************

VOID
CompalSetIntelTccTemp (
  UINT8         ActivationTempOffset
)
/*
Routine Description: \n
  - Set PROCHOT# Trigger Temperature via TCC Activation Offset
    It's only for the platform which PROCHOT# did not Link to EC.

Arguments:
  - ActivationTempOffset = Tjmax - Trigger Temp

Returns:
  - None
*/
{
  UINT8         Value8 = 0;
  UINT64        MsrValue = 0;

  if (AsmReadMsr64 (PLATFORM_INFORMATION) & PROGRAMMABLE_TCC_ACTIVATION_OFFSET) { // TCC Activation Offset Programmable
        // TCC Activation Offset is 4 BITs, the Max Value is 15.
        // Trigger Temperature can't less than Tjmax - 15 degree C.
        if (ActivationTempOffset > TCC_OFFSET_WIDTH_MASK) {
        		Value8 = TCC_OFFSET_WIDTH_MASK;
        } else {
        		Value8 = ActivationTempOffset;
        }

        // Write Temperature Setting into TCC Activation Offset
        MsrValue = AsmReadMsr64 (IA32_TEMPERATURE_TARGET) & ~(LShiftU64 ((UINT64)(TCC_OFFSET_WIDTH_MASK), TCC_OFFSET));
        MsrValue |= LShiftU64 ((UINT64)(Value8 & TCC_OFFSET_WIDTH_MASK), TCC_OFFSET);
        AsmWriteMsr64 (IA32_TEMPERATURE_TARGET, MsrValue);
  }
  return;
}


//****************************************************************************************
//*                            EC Routine
//****************************************************************************************
VOID
CompalSendCpuMaxPstateToEC (
  UINT8         MaxPstate
)
/*
Routine Description: \n
  - Send CPU Max P-State to EC RAM 0xF4AF, BIT[7:4]

Arguments:
  - Max P-State Value

Returns:
  - None
*/
{
  UINT8         ECValue;

  // Fill the Value in EC Name Space via EC Index IO
  ECValue = CompalECACPINVSReadByteByIndexIO (EC_NAME_CPU_MAX_PSTATE_OFFSET) & ~EC_NAME_CPU_MAX_PSTATE_MASK_BIT;
  ECValue |= MaxPstate << EC_NAME_CPU_MAX_PSTATE_SHIFT;
  CompalECACPINVSWriteByteByIndexIO (EC_NAME_CPU_MAX_PSTATE_OFFSET, ECValue);
  return;
}


//****************************************************************************************
//*                            Intel Routine
//****************************************************************************************
VOID
CompalIntelProchotBiDirectional (
  UINT8         Setting
)
/*
Routine Description: \n
  - Set Intel CPU PROCHOT# BiDirectional Enabled/Disabled

Arguments:
  - "ENABLED" or "DISABLED" the BiDirectional Feature

Returns:
  - None
*/
{
  UINT64        MsrValue = 0;

  MsrValue = AsmReadMsr64 (POWER_CTL) & ~(LShiftU64 ((UINT64)(BI_DIRECTIONAL_ENABLE_MASK), BI_DIRECTIONAL_ENABLE_OFFSET));
  MsrValue |= LShiftU64 ((UINT64)(Setting), BI_DIRECTIONAL_ENABLE_OFFSET);
  AsmWriteMsr64 (POWER_CTL, MsrValue);
  return;
}


UINT8
CompalGetIntelCpuMaxPstate (
)
/*
Routine Description: \n
  - Calculate Intel CPU Max P-State

Arguments:
  - None

Returns:
  - Max P-State Value
*/
{
  //EFI_CPUID_REGISTER  Cpuid06;
  UINT64        MsrValue = 0;
  UINT8         PStateValue = 0, MaxRatio = 0, MinRatio = 0 ;
  UINT32        RegEax;

  // Get CPU MSR Value of MaxRatio and MinRatio
  MsrValue = AsmReadMsr64 (PLATFORM_INFORMATION);
  MaxRatio = (UINT8)(RShiftU64 (MsrValue, MAXIMUM_NON_TURBO_RATIO));
  MinRatio = (UINT8)(RShiftU64 (MsrValue, MAXIMUM_EFFICIENCY_RATIO));

  PStateValue = (MaxRatio - MinRatio);
  PStateValue = PStateValue + 1;

  // To check if Turbo mode is enable or disable (read MSR 1A0h bit [38] = 0 is Turbo enable).
  MsrValue = AsmReadMsr64 (IA32_MISC_ENABLES);

  // To check if Turbo is supported in CPU, via CPU function 06 bit1.
  AsmCpuid (POWER_MANAGEMENT_PARAMETERS, &RegEax, NULL, NULL, NULL);

  // If Cpuid function 06 bit1 vlaue is 1 and MSR 0x1A0 bit38 is 0,
  // means Turbo is available  and enable, so total P Satae level add one.
  if (((RegEax & TURBO_MODE_SUPPORTED) == TURBO_MODE_SUPPORTED) &&
  	((MsrValue & TURBO_MODE_DISABLE) == 0)) {
        PStateValue = PStateValue + 1; // P satae value add one.
  }

  // Calculate CPU Max P-State, if  total Level is over 16 level.
  if (PStateValue <= LIMIT_MAX_PSTATE) {
        // CPU P-State = (MaxRatio - MinRatio) + 2 (if turbo mode is available and enable)
        // CPU P-State = (MaxRatio - MinRatio) + 1 (if turbo mode is not availabe or disable)
        PStateValue = PStateValue;
  } else {
        // Limit CPU Max P-State = 16 Level (total P state value, P0 ~ P15)
        PStateValue = LIMIT_MAX_PSTATE;
  }

  PStateValue -= 1; // It return Max P-State, not Total P-State.
  return PStateValue;
}

VOID
CompalDetectIntelConfigTDPSupported (
)
/*
Routine Description: \n
  - Detect cTDP Supported or not then Set cTDP Flag into EC Name Space

Arguments:
  - None

Returns:
  - None
*/
{
  UINT8         ECValue = 0;
  UINT64        MsrValue = 0;
//  EFI_CPUID_REGISTER  Cpuid01;
  UINT32        RegEax;

  // Get and Clear cTDP Flag of EC Name Space Offset 0xF4AF, BIT3
  ECValue = CompalECACPINVSReadByteByIndexIO (EC_NAME_CTDP_FLAG_OFFSET) & ~EC_NAME_CTDP_FLAG_MASK_BIT;

  // Identify CPU is SNB or IVB, only IVB support cTDP
  AsmCpuid (FEATURE_INFORMATION, &RegEax, NULL, NULL, NULL);

  if (RegEax >= IVY_BRIDGE) { // IVB cTDP Supported
        // Check Number of Config TDP Levels
        MsrValue = AsmReadMsr64 (PLATFORM_INFORMATION);
        if ((RShiftU64 (MsrValue, NUMBER_OF_CTDP_LEVELS_OFFSET) & NUMBER_OF_CTDP_LEVELS_MASK) != 0) { // Support cTDP
                // Set cTDP Flag
                ECValue |= EC_NAME_CTDP_FLAG_MASK_BIT; // Set BIT3
        }
  }

  // Pass cTDP Flag to EC Name Space
  CompalECACPINVSWriteByteByIndexIO (EC_NAME_CTDP_FLAG_OFFSET, ECValue);
  return;
}
