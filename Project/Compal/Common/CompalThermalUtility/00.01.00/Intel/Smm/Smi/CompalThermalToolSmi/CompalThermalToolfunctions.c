/*
 * (C) Copyright 2015-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2015-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  ------------ ----------------------------------------------------
 1.00                Vanessa Chen Init version.
 1.01      18/07/16  Joy Hsiao    Add sub function#01 for thermal utility to get CPU temp.
*/

#include <CompalThermalToolSmi.h>

extern EFI_SMM_SYSTEM_TABLE2             *mSmst;
extern COMPAL_GLOBAL_NVS_AREA            *CompalGlobalNvsArea;

//*************************************************************************************
// For Compal Thermal Utility used SW SMI Event number. define at CompalThermalToolSmi.h
//*************************************************************************************
THERMAL_UT_SMI_FUNCTIONS    THERMAL_SMI_EventTable[] = 
{
  {0x01, ThermalUTGetCPUTemp} 
};

#define THERMAL_UT_SMI_EVENT_COUNT sizeof(THERMAL_SMI_EventTable)/ sizeof(THERMAL_UT_SMI_FUNCTIONS)

/**
 ThermalUTGetCPUTemp 

 @retval Status                 

**/
VOID
ThermalUTGetCPUTemp(
    VOID
)
{
    UINT8       Data=0;
    UINTN       Index;
    EFI_STATUS  Status = EFI_SUCCESS;

    GetDigitalThermalSensorTemperature(&Data); 
  
    for (Index = 1; Index < mSmst->NumberOfCpus ; Index++) {
   
    Status=mSmst->SmmStartupThisAp (GetDigitalThermalSensorTemperature, Index, &Data); 

    }
    CompalGlobalNvsArea->STPMThermalUtilityData=Data;

    return;
}

VOID
GetDigitalThermalSensorTemperature (
  UINT8         *Buffer
  )
{
  MSR_REGISTER          MsrData;
  UINT8                 Temperature;
  UINT8                 DtsTjMax;
  
  //
  // Read the TjMax
  //
  MsrData.Qword = AsmReadMsr64 (EFI_MSR_CPU_THERM_TEMPERATURE);
  DtsTjMax      = (MsrData.Bytes.ThirdByte);

  //
  // Read the temperature
  //
  MsrData.Qword = AsmReadMsr64 (EFI_MSR_IA32_THERM_STATUS);

  //
  // If Out-Of-Spec, return the critical shutdown temperature.
  //
  if (MsrData.Qword & B_OUT_OF_SPEC_STATUS) {
    *((UINT8*) Buffer) = DTS_CRITICAL_TEMPERATURE;
    return;    
  } else if (MsrData.Qword & B_READING_VALID) {
    //
    // Find the DTS temperature.
    //
    Temperature = DtsTjMax - (MsrData.Bytes.ThirdByte & OFFSET_MASK);
    //
    // We only update the temperature if it is above the current temperature.
    //
    if (Temperature > *((UINT8*) Buffer)) {
      *((UINT8*) Buffer) = Temperature; 
    }
 
  }
}

/**
 CompalThermalUTSmiService Check Thermal Utitlity SW SMI event to do it's function.

 @param [in]   DispatchHandle   The handle of this callback, obtained when registering
 @param [in]   GpiRegisterContext  Pointer to the dispatch function's context.
 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
EFIAPI
CompalThermalUTSmiService(
    IN  EFI_HANDLE  DispatchHandle,
    IN CONST VOID   *DispatchContext OPTIONAL,
    IN OUT VOID     *CommBuffer      OPTIONAL,
    IN OUT UINTN    *CommBufferSize  OPTIONAL
)
{
    UINT8   Index;
    UINT8   TUFunNo;
  
    TUFunNo = IoRead8(SW_SMI_IO_DATA);          // Get SubFunction Index
    IoWrite8(SW_SMI_IO_DATA, 0x00);             // Clear data

    for(Index = 0 ; Index < THERMAL_UT_SMI_EVENT_COUNT ; Index++)
    {
        // for Compal Common EC SMI Event number
        if(TUFunNo == THERMAL_SMI_EventTable[Index].ThermalUTSmiFunNo)
        {
            THERMAL_SMI_EventTable[Index].ThermalUTSmiFunService();
            break;
        }
    }

  return EFI_SUCCESS;
}
