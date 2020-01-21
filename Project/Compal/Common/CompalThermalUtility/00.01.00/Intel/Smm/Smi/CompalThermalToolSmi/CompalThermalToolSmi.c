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

#include <CompalThermalToolSmi.h>

EFI_SMM_SYSTEM_TABLE2                     *mSmst;
COMPAL_GLOBAL_NVS_PROTOCOL                *CompalGlobalNvsAreaProtocol;
COMPAL_GLOBAL_NVS_AREA                    *CompalGlobalNvsArea;
/**
 Initializes the Compal SMM Platfrom Driver

 @param [in]   ImageHandle      Pointer to the loaded image protocol for this driver
 @param [in]   SystemTable      Pointer to the EFI System Table

 @retval Status                 EFI_SUCCESS
 @return Assert, otherwise.

**/
EFI_STATUS
EFIAPI
CompalThermalUtilitySmiEntryPoint (
    IN EFI_HANDLE                         ImageHandle,
    IN EFI_SYSTEM_TABLE                   *SystemTable
  )
{
  EFI_STATUS                                Status = EFI_SUCCESS;
  EFI_SMM_BASE2_PROTOCOL                    *SmmBase;
  EFI_SMM_SW_REGISTER_CONTEXT               SwContext;
  EFI_SMM_SW_DISPATCH2_PROTOCOL             *SwDispatch;
  EFI_HANDLE                                SwHandle;
  BOOLEAN                       InSmm = FALSE;

    Status = gBS->LocateProtocol (
                    &gEfiSmmBase2ProtocolGuid,
                    NULL,
                    (VOID **) &SmmBase
                    );
    
    if (!EFI_ERROR (Status)) {
      SmmBase->InSmm (SmmBase, &InSmm);
    }
    
    if (InSmm) {
        //
        // Great!  We're now in SMM!
        //
        //

        Status = SmmBase->GetSmstLocation(
                        SmmBase,
                        &mSmst
                      );
        
        if (EFI_ERROR (Status)) {
            Status = EFI_UNSUPPORTED;
        }
    }

    Status = gBS->LocateProtocol (
                 &gCompalGlobalNvsProtocolGuid,
                 NULL,
                 &CompalGlobalNvsAreaProtocol
             );

    if ( EFI_ERROR(Status) ) {
        return Status;
    }

    CompalGlobalNvsArea = CompalGlobalNvsAreaProtocol->CompalArea;

    
    //
    // Get the Sw dispatch protocol
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
    SwContext.SwSmiInputValue = ThermalUtility_SW_SMI;
    Status = SwDispatch->Register (
                 SwDispatch,
                 CompalThermalUTSmiService,
                 &SwContext,
                 &SwHandle
             );

    if (EFI_ERROR (Status)) {
        Status = EFI_UNSUPPORTED;
    }

  return Status;
}
