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
*/

#ifndef _COMPAL_PLATFORM_HOOK_PEI_PPI_H_
#define _COMPAL_PLATFORM_HOOK_PEI_PPI_H_

#define COMPAL_PLATFORM_HOOK_PEI_PPI_GUID  \
  { \
    0x5097ea4b, 0x7dda, 0x4c99, 0x87, 0xae, 0x9, 0xc5, 0x54, 0x6a, 0x2, 0x1 \
  }

typedef struct _COMPAL_PLATFORM_HOOK_PEI_PPI COMPAL_PLATFORM_HOOK_PEI_PPI;

typedef enum {
  COMPAL_PLATFORM_HOOK_PEI_PLATFORMSTAGE1_OVERRIDE,
  COMPAL_PLATFORM_HOOK_PEI_PLATFORMSTAGE2_OVERRIDE
} COMPAL_PLATFORM_HOOK_PEI_OVERRIDE_SETTING;

//
// Member functions
//
typedef
EFI_STATUS
(EFIAPI *COMPAL_PLATFORM_HOOK_PEI_OVERRIDE) (
  IN     CONST EFI_PEI_SERVICES                         **PeiServices,
  IN     COMPAL_PLATFORM_HOOK_PEI_PPI                   * This,
  IN     COMPAL_PLATFORM_HOOK_PEI_OVERRIDE_SETTING      OverrideType,
  ...
);

//
// Extern GUID
//
extern EFI_GUID gCompalPlatformHookPeiPpiGuid;

//
// COMPAL PLATFORM HOOK PEI PPI
//
typedef struct  _COMPAL_PLATFORM_HOOK_PEI_PPI {
  COMPAL_PLATFORM_HOOK_PEI_OVERRIDE  CompalPlatformHookPeiDispatch;
} COMPAL_PLATFORM_HOOK_PEI_PPI;

#endif
