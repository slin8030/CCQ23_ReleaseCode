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
  EFI_SMM_GPI_DISPATCH2_PROTOCOL            *GpiDispatch;
  EFI_SMM_GPI_REGISTER_CONTEXT              GpiRegisterContext;
  EFI_HANDLE                                GpiHandle;

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
    GpioGetGpiSmiNum ((GPIO_PAD)PRJ_EC_SMI_GPI, &GpiRegisterContext.GpiNum);
	
	Status = GpiDispatch->Register (
							   GpiDispatch,
							   CompalEcSmiService,
							   &GpiRegisterContext,
							   &GpiHandle
							   );

    if (EFI_ERROR (Status)) {
        Status = EFI_UNSUPPORTED;
    }

  return Status;
}
