/*
 * (C) Copyright 2016 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2016 Compal Electronics, Inc.. All rights reserved.

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
 0.01      17/03/09    Daiven_Chen   Base Sideband read/write.
*/
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/ShellLib.h>
#include <Library/SideBandLib.h>

VOID
PrintHelpInfo(){
  Print(L"Read/Write SideBand \n");
  Print(L"  SideBand r <Port> <Offset> \n");
  Print(L"  SideBand w <Port> <Offset> <Data>\n");
  Print(L"    Example read : SideBand.efi r 0xA7 0x4000 \n");
  Print(L"    Example write: SideBand.efi w 0xA7 0x4000 0xAA5555AA\n");
}

/**
  Read/Write SideBand data

  @param[In] List             The list include utility input parameter.

  @return EFI_SUCCESS         Function execute successful.
  @return Others
**/
INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
  UINT8                     TargetPortId;
  UINT16                    TargetRegister;
  UINT32                    Data;

  Print(L"SideBand Config V0.01\n");

  if (Argc < 4) {
    PrintHelpInfo();
    return EFI_INVALID_PARAMETER;
  }

  if (SideBandMmioValid() == FALSE) {
    Print(L"SideBand access unsupport.\n");
    return EFI_UNSUPPORTED;
  }

  TargetPortId = (UINT8)ShellStrToUintn(Argv[2]);
  TargetRegister = (UINT16)ShellStrToUintn(Argv[3]);

  if(0 == StrCmp(Argv[1], L"r")) {
    if (Argc < 4) {
      PrintHelpInfo();
      return EFI_INVALID_PARAMETER;
    }
    Data = SideBandRead32 (TargetPortId, TargetRegister);
    Print(L"SideBand read Port 0x%x, Register:0x%x, Data = 0x%08x \n", TargetPortId, TargetRegister, Data);
  } else if (0 == StrCmp(Argv[1], L"w")) {
    if (Argc < 5) {
      PrintHelpInfo();
      return EFI_INVALID_PARAMETER;
    }
    Data = (UINT32)ShellStrToUintn(Argv[4]);
    SideBandWrite32(TargetPortId, TargetRegister,Data);
    Print(L"SideBand write Port 0x%x, Register:0x%x, Data = 0x%08x \n", TargetPortId, TargetRegister, Data);
  }

  return EFI_SUCCESS;
}

