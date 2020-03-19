/*
 * (C) Copyright 2013-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2013-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal!|s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  
 R E V I S I O N    H I S T O R Y
 
 Ver       Date       Who          Change           Support Chipset
 --------  --------   ------------ ---------------- -----------------------------------
 1.00      13/8/22    Jeter_Wang   Init version.     BTT/BSW/CHT
*/


#include <PiDxe.h>
#include <CompalECFwAutoLoad.h>
#include <CompalECFwAutoLoadDxe.h>
#include <PostCode.h>

EFI_STATUS
EFIAPI
CompalECFwAutoLoadFromMemory(
)
{
  EFI_STATUS               Status;
  UINT8*                   FileBuffer;

  Status = EFI_SUCCESS;
  
  //Get Fv EC Base
  FileBuffer = (VOID*)(UINTN)FixedPcdGet32 (PcdFlashFvEcBase);  
  
  Status = CompalECIO373XFlashFunction(FileBuffer);
  return Status;
}

COMPAL_COMMON_EC_FIRMWARE_AUTO_LOAD_DXE_HOOK_PROTOCOL gCompalCommonEcFwAutoLoadDxeHookProtocol={
  CompalECFwAutoLoadFromMemory
};

EFI_STATUS
CompalECAutoLoad (
  )
{
  EFI_STATUS                                                Status;
  COMPAL_COMMON_EC_FIRMWARE_AUTO_LOAD_DXE_HOOK_PROTOCOL    *FvInstance;

  Status = gBS->LocateProtocol (
                &gCompalCommonEcFwAutoLoadDxeHookProtocolGuid,
                NULL,
                (VOID **) &FvInstance
                );
                
  if (EFI_ERROR (Status)) {
      return Status;
  }
    
  Status = FvInstance->CompalECFwAutoLoadFromMemory();

  return Status;
}


/**
  Entrypoint of Compal EC Fw AutoLoad.

  @param  ImageHandle
  @param  SystemTable

  @return EFI_SUCCESS
  @return EFI_LOAD_ERROR
  @return EFI_OUT_OF_RESOURCES

**/
EFI_STATUS
EFIAPI
CompalECFwAutoLoadEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{

  EFI_STATUS  Status;

   POST_CODE (0xDA);    //POST CODE 0xDA 

  // Install the Protocol onto a new handle
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gCompalCommonEcFwAutoLoadDxeHookProtocolGuid, &gCompalCommonEcFwAutoLoadDxeHookProtocol,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
      return Status;
  }
                   
  Status = CompalECAutoLoad();  //Implement EC Auto Load          
                  
  return Status;
}
