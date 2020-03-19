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

 Ver       Date         Who          Change
 --------  ----------   --------     ----------------------------------------------------
 1.00      2016.06.08   Stan         Init version.
*/

#ifndef _COMPAL_EC_SPI_LIBRARY_H
#define _COMPAL_EC_SPI_LIBRARY_H

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#ifdef PROGRESS_BAR_DISPLAY
#include <Library/UefiLib.h>
#endif

#define EC_INDEX_IO_BASE                FixedPcdGet16(PcdEcIoBaseAddress)     // ec index io address

#define ISP_SPIADD0                     0xFEA8
#define ISP_SPIADD1                     0xFEA9
#define ISP_SPIADD2                     0xFEAA
#define ISP_SPIDAT                      0xFEAB
#define ISP_SPICMD                      0xFEAC
#define ISP_SPICFG                      0xFEAD
#define ISP_ISPDATR                     0xFEAE
#define ISP_SCON3                       0xFEAF
#define ISP_LPCCSR                      0xFE9E
#define ISP_ECSTS                       0xFF1D
#define ISP_CLKCFG2                     0xFF1E 

#define EFCMD_PageLatch                 0x02
#define EFCMD_Read                      0x03
#define EFCMD_PageErase                 0x20
#define EFCMD_ProgramSP                 0x70
#define EFCMD_ClrHVPL                   0x80
#define SPI_REMS                        0x9F
#define PAGE_SIZE                       0x80

#define FMWCSR                          0x100A
#define FMWBA                           0x100B
#define EC_MAPPING_WINDOWS_SIZE         0x4000
#define EC_MAPPING_WINDOWS_BASE         0x8000
#define END_OF_ERASE_TABLE              0xFFFF

//
// erase table policy
//
typedef enum _EneErasePolicy {
  EcIndexIo = 0,
  EcMappingWindows,
  EraseAllPages
} EneErasePolicy;


UINT8
EcSpiRead (
  IN UINT16    ENESPIReg
  );

VOID
EcSpiWrite (
  IN UINT16    ENESPIReg,
  IN UINT8     ENESPICMD
  );

VOID
EcSpiWait (
  VOID
  );

VOID
ProgressBar (
  IN  UINT8  Percent
  );

VOID
FlashSpeedUp (
  IN         UINTN    Address,
  IN         UINT8    *Data, 
  IN  OUT    UINT16   *EraseTable,
  IN         UINT32   Pages,
  IN  CONST  UINT8    ErasePolicy
  );

EFI_STATUS
CompalEcReadSPIData (
  IN      BOOLEAN    IdleMode,
  IN  OUT UINT8      *DataBuffer,
  IN      UINT8      *Address,
  IN      UINTN      BufferSize
  );

VOID
CompalWriteEcRom (
  IN  UINTN         Address,
  IN  UINT8         *FlashingDataBuffer,
  IN  UINTN         FlashImageSize,
  IN  CONST  UINT8  ErasePolicy
  );

EFI_STATUS
StopSpi (
  VOID
  );

EFI_STATUS
StartSpi (
  VOID    
  );

VOID
CompalPageErase (
  IN UINTN Address
  );

EFI_STATUS
CompalSpiEcFlash (
  IN        UINT8  *FlashingDataBuffer,
  IN        UINTN  SizeToFlash,
  IN        UINT8  DestBlockNo, 
  IN CONST  UINT8  ErasePolicy
  );

VOID
CompalEcRomDump (
  IN OUT   CHAR8   *EcRomBuffer,
  IN       UINTN   SizeToFlash
  );


#ifdef PROGRESS_BAR_DISPLAY
extern EFI_SYSTEM_TABLE        *gST;
#endif

#endif

