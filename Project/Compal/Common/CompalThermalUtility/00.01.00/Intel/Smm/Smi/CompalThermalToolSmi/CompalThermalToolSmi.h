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
 1.01	   07/15/16  Joy Hsiao	  Update SW SMI to 0xD9

*/

#ifndef _COMPAL_SMM_PLATFORM_H_
#define _COMPAL_SMM_PLATFORM_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <CompalGlobalNvsProtocol.h>
#include "ProjectDefinition.h"

extern EFI_BOOT_SERVICES  *gBS;

//**********************************************************
// Compal Common Thermal Utility SMI event number.
//**********************************************************
#define ThermalUtility_SW_SMI 0xD9
#define SW_SMI_IO_ADDRESS  0xB2
#define SW_SMI_IO_DATA     SW_SMI_IO_ADDRESS+1

//Define STPM_Thermal_Utility
#define EFI_MSR_CPU_THERM_TEMPERATURE          0x1a2
#define EFI_MSR_IA32_THERM_STATUS              0x19C
#define B_OUT_OF_SPEC_STATUS                   (1 << 4)
#define DTS_CRITICAL_TEMPERATURE               255
#define B_READING_VALID                        (1 << 31)
#define OFFSET_MASK                            (0x7F)

typedef union _MSR_REGISTER {
  UINT64  Qword;

  struct _DWORDS {
    UINT32  Low;
    UINT32  High;
  } Dwords;

  struct _BYTES {
    UINT8 FirstByte;
    UINT8 SecondByte;
    UINT8 ThirdByte;
    UINT8 FouthByte;
    UINT8 FifthByte;
    UINT8 SixthByte;
    UINT8 SeventhByte;
    UINT8 EighthByte;
  } Bytes;

} MSR_REGISTER;

typedef struct {
    UINT8   ThermalUTSmiFunNo;
    VOID    (*ThermalUTSmiFunService)();
} THERMAL_UT_SMI_FUNCTIONS;

EFI_STATUS
EFIAPI
CompalThermalUTSmiService(
    IN  EFI_HANDLE  DispatchHandle,
    IN CONST VOID   *DispatchContext OPTIONAL,
    IN OUT VOID     *CommBuffer      OPTIONAL,
    IN OUT UINTN    *CommBufferSize  OPTIONAL
);

VOID
ThermalUTGetCPUTemp(
    VOID
);

VOID
GetDigitalThermalSensorTemperature (
  UINT8         *Buffer
);

#endif
