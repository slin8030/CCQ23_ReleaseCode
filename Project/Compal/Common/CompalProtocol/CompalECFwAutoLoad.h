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





#ifndef __COMPALECFWAUTOLOAD_H__
#define __COMPALECFWAUTOLOAD_H__


#include <Library/UefiBootServicesTableLib.h>


#define COMPAL_COMMON_EC_FW_AUTO_LOAD_DXE_HOOK_PROTOCOL_GUID \
  { \
    0x7dc29005, 0x0622, 0x4483, { 0xac, 0x99, 0x4d, 0xfb, 0x19, 0x12, 0xed, 0x67 } \
  }


//
// Forward reference.
//
typedef struct _COMPAL_COMMON_EC_FIRMWARE_AUTO_LOAD_DXE_HOOK_PROTOCOL COMPAL_COMMON_EC_FIRMWARE_AUTO_LOAD_DXE_HOOK_PROTOCOL;

/*
 * Protocol members
 */


typedef
EFI_STATUS
(EFIAPI *COMPAL_EC_FW_AUTO_LOAD_FROM_MEMORY)(
);


//
// Interface structure
//
struct _COMPAL_COMMON_EC_FIRMWARE_AUTO_LOAD_DXE_HOOK_PROTOCOL {
  COMPAL_EC_FW_AUTO_LOAD_FROM_MEMORY CompalECFwAutoLoadFromMemory;
};

extern EFI_GUID gCompalCommonEcFwAutoLoadDxeHookProtocolGuid;

#endif // __COMPALECFWAUTOLOAD_H__
