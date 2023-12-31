/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

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
 1.00                Champion Liu Init version.
 1.01      12/26/14  Champion Liu Add CMFC new function for CMFC spec 00.00.13.
 1.06      03/04/16  Champion Liu Modify CMFC 0x28 function for SkyLake and KabyLake.
*/

#include "CompalCMFCCommonSwSmi.h"
#include <Library/DebugLib.h>

EFI_SMM_SYSTEM_TABLE2                   *mSmst;
EFI_SMM_VARIABLE_PROTOCOL               *SmmVariable;

COMPAL_CMFC_SMM_HOOK_PROTOCOL           *CompalCMFCSmmHookProtocol;
COMPAL_GLOBAL_NVS_PROTOCOL              *CompalGlobalNvsAreaProtocol;
COMPAL_GLOBAL_NVS_AREA                  *CompalGlobalNvsArea;

UINT32                                  mSpiBaseAddress;
//
// Global Variables
//
EFI_STATUS
CompalCMFCCommonSwSmiEntryPoint (
    IN EFI_HANDLE                         ImageHandle,
    IN EFI_SYSTEM_TABLE                   *SystemTable
)
{

    EFI_STATUS                    Status;
    EFI_SMM_SW_DISPATCH2_PROTOCOL *SwDispatch;
    EFI_SMM_SW_REGISTER_CONTEXT   SwContext;
    EFI_HANDLE                    SwHandle;
    EFI_SMM_BASE2_PROTOCOL        *mSmmBase;
    BOOLEAN                       InSmm = FALSE;

    Status = gBS->LocateProtocol (
                 &gEfiSmmBase2ProtocolGuid,
                 NULL,
                 (VOID **) &mSmmBase
             );

    if (!EFI_ERROR (Status)) {
      mSmmBase->InSmm (mSmmBase, &InSmm);
    }

    if (!InSmm) {
      return EFI_NOT_READY;
    }
        //
        // Great!  We're now in SMM!
        //
        //
        mSmmBase->GetSmstLocation (mSmmBase, &mSmst);

        Status = mSmst->SmmLocateProtocol (
                     &gEfiSmmVariableProtocolGuid,
                     NULL,
                     (VOID **) &SmmVariable
                 );

        if (EFI_ERROR(Status)) {
            return Status;
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
    // CMFC Smm Hook
    //
    Status = mSmst->SmmLocateProtocol(
                 &gCompalCMFCSmmHookProtocolGuid,
                 NULL,
                 &CompalCMFCSmmHookProtocol
             );

    if (EFI_ERROR (Status)) {
        return Status;
    }

    //
    // Get the Sw dispatch protocol
    //
    Status = mSmst->SmmLocateProtocol (
                  &gEfiSmmSwDispatch2ProtocolGuid, 
                  NULL, 
                  &SwDispatch
                  );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Register for Compal CMFC SMI
    //
    SwContext.SwSmiInputValue = CMFC_SW_SMI;
    Status = SwDispatch->Register (
                 SwDispatch,
                 CompalCMFCSwSMIFunctionEntry,
                 &SwContext,
                 &SwHandle
             );

    mSpiBaseAddress = MmioRead32 (
                              MmPciAddress (0,
                                DEFAULT_PCI_BUS_NUMBER_SC,
                                PCI_DEVICE_NUMBER_SPI,
                                PCI_FUNCTION_NUMBER_SPI,
                                R_SPI_BASE
                              ) & B_SPI_BASE_BAR);

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
CompalCMFCSwSMIFunctionEntry (
    IN  EFI_HANDLE                    DispatchHandle,
    IN CONST VOID  *DispatchContext OPTIONAL,
    IN OUT VOID    *CommBuffer      OPTIONAL,
    IN OUT UINTN   *CommBufferSize  OPTIONAL
)
{
    FunctionBuffer FB;
    UINT8   FunNo;
    UINT16  SubFunNo;

    FunNo = IoRead8(SW_SMI_IO_DATA);     //Get SubFunction Index
    IoWrite8(SW_SMI_IO_DATA, 0x00);      // Clear data
    SubFunNo = (CompalGlobalNvsArea->SubFunctionNo);
    CompalGlobalNvsArea->StatusData = 0x8000;

    FB.cbFunNo  = FunNo;
    FB.cbSubFunNo = SubFunNo;

    CompalCMFCFunctionEntry(FB);
    return EFI_SUCCESS;
}
