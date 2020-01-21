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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal's subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who           Change
 --------  --------  ------------  ----------------------------------------------------
 1.00                Vanessa Chen  Init version.
 1.01      07/28/16  Antonie Cheng Add ESPI SMI function of virtual wire.
 1.02      16/08/22  Vanessa Chen  Support CPU turbo On/Off SW SMI 0xDA
 1.03      16/08/23  Vanessa Chen  Add Turbo support in CPU for Turbo On/Off SMI.
*/

#include <CompalPlatformSmm.h>
//*************************************************************************************
// 0x10~0x3F for Compal OEM Project EC SMI Event number. define at CompalPlatformSmm.h
//*************************************************************************************
EC_SMI_FUNCTIONS    EC_SMI_EventTable[] = 
{
  {EC_SMI_KBRST, CompalEcSmiKbRst}              // 0x26 for KBC_Cmd_FE to send SMI
};

#define EC_SMI_EVENT_COUNT sizeof(EC_SMI_EventTable)/ sizeof(EC_SMI_FUNCTIONS)

/**
 CompalEcSmiKbRst If get any one send "0xFE" to 64 port, send 06 to CF9 port.

 @retval Status                 

**/
VOID
CompalEcSmiKbRst(
    VOID
)
{
    IoWrite8(0xcf9, 0x06);                      // Platform reset.
    return;
}

/**
 CompalEcSmiEventCommon for compal common EC smi ID.
 
 @param [in]   EcSmiEvent       The event number got from EC 
 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
CompalEcSmiEventCommon(
	IN       UINT8								EcSmiEvent
)
{
  UINT8     Index;
  
        for(Index = 0 ; Index < EC_SMI_EVENT_COUNT ; Index++)
        {
            // for Compal Common EC SMI Event number
            if(EcSmiEvent == EC_SMI_EventTable[Index].EcSmiFunNo)
            {
                EC_SMI_EventTable[Index].EcSmiFunService();
                break;
            }
        }
        
  return EFI_SUCCESS;

}

/**
 CompalEcSmiService Check EC SMI event number to do it's function.

 @param [in]   DispatchHandle   The handle of this callback, obtained when registering
 @param [in]   GpiRegisterContext  Pointer to the dispatch function's context.
 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
EFIAPI
CompalEcSmiService(
	IN		 EFI_HANDLE 						DispatchHandle,
	IN		 CONST EFI_SMM_GPI_REGISTER_CONTEXT *GpiRegisterContext,
	IN OUT	 EFI_SMM_GPI_REGISTER_CONTEXT		*CurrentContext,
	IN OUT	 UINTN								*SizeOfContext
)
{
  UINT8     EcSmiBuffer8=0xFF ;

  //Command 0x56 to read this SMI ID.
  CompalECReadCmd(EC_MEMORY_MAPPING_PORT, EC_CMD_SMIID_INDEX, 0, EC_CMD_SMIID_INDEX_DATA_LEN, &EcSmiBuffer8);
  // 0x10~0x3F for Compal Common EC SMI Event number
  if((EcSmiBuffer8 >= 0x10)&&(EcSmiBuffer8 <= 0x3F))
  {
    CompalEcSmiEventCommon(EcSmiBuffer8);
  }
  else if ((EcSmiBuffer8 >= 0x40)&&(EcSmiBuffer8 <= 0x6F))
  {            
    CompalEcSmiEventOEM(EcSmiBuffer8);
  }
  return EFI_SUCCESS;
}

/**
 CompalESPISmiService Check eSPI EC SMI event number to do it's function.

 @param [in]   DispatchHandle   The handle of this callback, obtained when registering
 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
VOID
EFIAPI
CompalESPISmiService(
	IN		 EFI_HANDLE 						DispatchHandle
)
{

  UINT8     EcSmiBuffer8=0xFF ;

  //Command 0x56 to read this SMI ID.
  CompalECReadCmd(EC_MEMORY_MAPPING_PORT, EC_CMD_SMIID_INDEX, 0, EC_CMD_SMIID_INDEX_DATA_LEN, &EcSmiBuffer8);
  // 0x10~0x3F for Compal Common EC SMI Event number
  if((EcSmiBuffer8 >= 0x10)&&(EcSmiBuffer8 <= 0x3F))
  {
    CompalEcSmiEventCommon(EcSmiBuffer8);
  }
  else if ((EcSmiBuffer8 >= 0x40)&&(EcSmiBuffer8 <= 0x6F))
  {            
    CompalEcSmiEventOEM(EcSmiBuffer8);
  }
}

/**
 CompalCPUTurboSmiService Check Turbo SW SMI event to do it's function.

 @param [in]   DispatchHandle   The handle of this callback, obtained when registering
 @param [in]   GpiRegisterContext  Pointer to the dispatch function's context.
 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
EFIAPI
CompalCPUTurboSmiService(
    IN  EFI_HANDLE  DispatchHandle,
    IN CONST VOID   *DispatchContext OPTIONAL,
    IN OUT VOID     *CommBuffer      OPTIONAL,
    IN OUT UINTN    *CommBufferSize  OPTIONAL
)
{
    MSR_REGISTER    Ia32MiscEnableMsr;
    UINT32          RegEax;

  // To check if Turbo mode is enable or disable (read MSR 1A0h bit [38] = 0 is Turbo enable).
  Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_MISC_ENABLE);

  // To check if Turbo is supported in CPU, via CPU function 06 bit1.
  AsmCpuid (POWER_MANAGEMENT_PARAMETERS, &RegEax, NULL, NULL, NULL);

  // If Cpuid function 06 bit1 vlaue is 1 means Turbo is availablee.
  if ((RegEax & TURBO_MODE_SUPPORTED) == TURBO_MODE_SUPPORTED)
  {
    switch (IoRead8(0xB3)) {
      case CPU_Turbo_On:                        //Sub fun 0x00 : Trigger by turbo on Qevent(Q_1F)
           Ia32MiscEnableMsr.Qword &= ~B_MSR_IA32_MISC_DISABLE_TURBO;
           AsmWriteMsr64 (MSR_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
           break;
      case CPU_Turbo_Off:                       //Sub fun 0x01 : Trigger by turbo Off Qeven(Q_1E)
           Ia32MiscEnableMsr.Qword |= B_MSR_IA32_MISC_DISABLE_TURBO;
           AsmWriteMsr64 (MSR_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
           break;
      default:
           break;
    }
  }

  return EFI_SUCCESS;
}

/**
 Initializes the Compal SMM Platfrom Driver

 @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
EFIAPI
CompalPlatformSmmInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;
  EFI_SMM_SYSTEM_TABLE2                     *mSmst;
  EFI_SMM_BASE2_PROTOCOL                    *SmmBase;
  EFI_SMM_SW_REGISTER_CONTEXT               SwContext;
  EFI_SMM_SW_DISPATCH2_PROTOCOL             *SwDispatch;
  EFI_HANDLE                                SwHandle;
#if (EC_PLATFORM_SETTING == LPC )                 // 0: LPC mode
  EFI_SMM_GPI_DISPATCH2_PROTOCOL            *GpiDispatch;
  EFI_SMM_GPI_REGISTER_CONTEXT              GpiRegisterContext;
  EFI_HANDLE                                GpiHandle;
#else if (EC_PLATFORM_SETTING == eSPI )            // 2:eSPI mode
  UINT16                                    mAcpiBaseAddr;
  PCH_ESPI_SMI_DISPATCH_PROTOCOL            *EspiSmmDispatch;
  EFI_HANDLE                                EspiHandle;
#endif

  Status = gBS->LocateProtocol (
                  &gEfiSmmBase2ProtocolGuid,
                  NULL,
                  (VOID **) &SmmBase
                  );
  if (EFI_ERROR (Status)) {
      Status = EFI_UNSUPPORTED;
  }
  
  Status = SmmBase->GetSmstLocation(
                    SmmBase,
                    &mSmst
                    );
  
  if (EFI_ERROR (Status)) {
      Status = EFI_UNSUPPORTED;
  }
  
#if (EC_PLATFORM_SETTING == LPC )                 // 0: LPC mode
////////////////////////////////////////////////////////////////////////////////////////////
// Register GPI EC_SMI event for LPC EC
////////////////////////////////////////////////////////////////////////////////////////////
    // LPC mode
  	//
	// Locate Smi GPI protocol
	//
	Status = gSmst->SmmLocateProtocol (
					&gEfiSmmGpiDispatch2ProtocolGuid,
					NULL,
					&GpiDispatch
					);

                             
    if (EFI_ERROR (Status)) {
        Status = EFI_UNSUPPORTED;
    }
	//Enable EC-SMI
    GpioGetGpiSmiNum (PRJ_EC_SMI_GPI, &GpiRegisterContext.GpiNum);
 
	
	Status = GpiDispatch->Register (
							   GpiDispatch,
							   CompalEcSmiService,
							   &GpiRegisterContext,
							   &GpiHandle
							   );
    DEBUG ((DEBUG_INFO, "[Compal][Common]Install CompalEcSmiService() \n"));

    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "[Compal][Common]Install CompalEcSmiService() fail \n"));
        Status = EFI_UNSUPPORTED;
    }

#else if (EC_PLATFORM_SETTING == eSPI )            // 2:eSPI mode
////////////////////////////////////////////////////////////////////////////////////////////
// Register GPI EC_SMI event for eSPI EC
////////////////////////////////////////////////////////////////////////////////////////////
    // eSPI mode
    //
    // Enable ESPI_SMI_EN
    //
    PchAcpiBaseGet (&mAcpiBaseAddr);
    ASSERT (mAcpiBaseAddr != 0);
    
    IoOr32 (mAcpiBaseAddr + R_PCH_SMI_EN, B_PCH_SMI_EN_ESPI);
    
    //
    // Locate eSPI Smi protocol
    //
    Status = gSmst->SmmLocateProtocol (
                    &gPchEspiSmiDispatchProtocolGuid, 
                    NULL, 
                    &EspiSmmDispatch
                    );
      
    Status = EspiSmmDispatch->EspiSlaveSmiRegister (
  							 EspiSmmDispatch,
  							 CompalESPISmiService,
  							 &EspiHandle
  							 ); 
    DEBUG ((DEBUG_INFO, "[Compal][Common]Install CompalESPISmiService() \n"));

    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "[Compal][Common]Install CompalESPISmiService() fail \n"));
        Status = EFI_UNSUPPORTED;
    }

#endif 
////////////////////////////////////////////////////////////////////////////////////////////
// Register SW SMI function 0xDA for CPU IA turbo On/Off
////////////////////////////////////////////////////////////////////////////////////////////

    //
    // Get the SW dispatch protocol
    //
    Status = mSmst->SmmLocateProtocol (
                  &gEfiSmmSwDispatch2ProtocolGuid, 
                  NULL, 
                  &SwDispatch
                  );
                            
    if (EFI_ERROR (Status)) {
        Status = EFI_UNSUPPORTED;
    }

    //
    // Register for Compal SW SMI
    //
    SwContext.SwSmiInputValue = CPU_TurboOnOff_SW_SMI;
    Status = SwDispatch->Register (
                 SwDispatch,
                 CompalCPUTurboSmiService,
                 &SwContext,
                 &SwHandle
             );
    DEBUG ((DEBUG_INFO, "[Compal][Common]Install CompalCPUTurboSmiService() \n"));

    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "[Compal][Common]Install CompalCPUTurboSmiService() fail \n"));
        Status = EFI_UNSUPPORTED;
    }
  
  return Status;
}
