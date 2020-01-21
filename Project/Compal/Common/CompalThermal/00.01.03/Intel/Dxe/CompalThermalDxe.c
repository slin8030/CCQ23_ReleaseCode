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

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.02      2015/9/2  Stan         Add CpuTjMax, CpuCoreNum GetVgaType function
 1.03      1016/9/21 VanessaChen  Add CPU Turbo boots supported.
*/


#include "CompalThermalDxe.h"

EFI_STATUS
CompalThermalInitDxeEntryPoint (
	IN  EFI_HANDLE			ImageHandle,
	IN  EFI_SYSTEM_TABLE		*SystemTable
)
/*++

  Routine Description:
    The entry point for an Socket Service Driver simply allocates space for
    an  EFI_DRIVER_BINDING_PROTOCOL protocol instance, initializes its contents,
    and attaches this protocol instance to the image handle of the Socket
    Service Driver.

  Arguments:
    ImageHandle   EFI_HANDLE: The firmware allocated handle for the EFI
                  Socket Service Driver image.
    SystemTable   EFI_SYSTEM_TABLE: A pointer to the EFI system table

  Returns:
    EFI_SUCCESS:            The ISA bus driver was initialized.
    EFI_ALREADY_STARTED:    The driver has already been initialized.
    EFI_INVALID_PARAMETER:  One of the parameters has an invalid value.
    EFI_OUT_OF_RESOURCES:   The request could not be completed due to a lack of
                            resources.

--*/
{
  CompalIntelProchotBiDirectional (ENABLED);
  CompalSendCpuMaxPstateToEC (CompalGetIntelCpuMaxPstate ());
  CompalSendCpuTjMaxToEC(CompalGetIntelCpuTjMax());
  CompalSendCpuCoreNumToEC(CompalGetIntelCpuCoreNum());
  CompalSendVGATypeToEC(CompalGetVGAType());
  CompalDetectIntelConfigTDPSupported ();
  CompalDetectIntelCPUTurboBoostupported();
  return EFI_SUCCESS;
}
