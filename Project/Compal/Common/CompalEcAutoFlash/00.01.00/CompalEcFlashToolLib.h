/*
 * (C) Copyright 2011-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2020 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal’s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y

 Ver       Date         Who          Change
 --------  ----------   --------     ----------------------------------------------------
 1.00      2016.07.20   Stan         Init version.
*/

#ifndef _COMPAL_EC_FLASH_TOOL_LIB_H_
#define _COMPAL_EC_FLASH_TOOL_LIB_H_

#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <CompalEcLib.h>
#include <CompalEcCommoncmd.h>
#include <CompalEepromDxe.h>
#include <CompalEcSpiLib.h>


#ifndef EC_CHIPSET_VENDOR
#define EC_CHIP               UnknownEc
#define EC_VER_OFFSET         0x4001
#define FLASH_SPEED_UP        EcMappingWindows
#define ERASE_ALL_BLOCK       EraseAllPages
#else

//
// ENE Chip
//
#if (EC_CHIPSET_VENDOR == 0x00)
#define EC_CHIP               Ene90xx
#define EC_VER_OFFSET         0x4001
#define FLASH_SPEED_UP        EcMappingWindows
#define ERASE_ALL_BLOCK       EraseAllPages
#endif

//
// Nuvoton Chip
//
#if (EC_CHIPSET_VENDOR == 0x01)
#define EC_CHIP               Nuvoton8xx
#define EC_VER_OFFSET         0x2181
#define FLASH_SPEED_UP        EcWcb2
#define ERASE_ALL_BLOCK       EraseAllSectors
#endif

#endif

#define EC_SIZE_TO_FLASH      0x20000
#define EC_AUTO_FLASH_OFFSET  0x2FD


VOID
CompalEcAutoFlashToggle (
  VOID
  );

EFI_STATUS
CompalEcInformation (
  VOID
  );

UINT8
CompalEcChipDetect (
  VOID
  );

EFI_STATUS
CompalEcProjectCompare (
  IN CHAR8          *EcRomBuffer
  );

EFI_STATUS
CompalEcReadProject (
  VOID
  );

EFI_STATUS
CompalEcReadVersion (
  VOID
  );

EFI_STATUS
CompalEcRomIdentify (
  IN  OUT    UINT8  *OnboardEcVersion,
  IN  OUT    CHAR8  *RomFileEcVersion,
  IN         CHAR8  *EcRomBuffer,
  IN         UINT8  EcChipId
  );

EFI_STATUS
ReadRomFile (
    IN OUT CHAR8          *EcRomBuffer,
    IN LIST_ENTRY         *ParamPackage
  );

EFI_STATUS
DumpdRomFile (
    IN OUT CHAR8          *EcRomBuffer,
    IN LIST_ENTRY         *ParamPackage
  );


typedef enum _EcChip {
  Ene90xx = 0,
  Nuvoton8xx,
  UnknownEc
} EcChip;

#endif
