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

#ifndef __Compal_ECFlashI2C_H__
#define __Compal_ECFlashI2C_H__

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Protocol/EfiShellEnvironment2.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/HidDescriptorLib.h>
#include <CompalEcLib.h>

// ======================== Version Define ==========================
#define UTILITY_VENDOR          L"Copyright 2013 Compal Electronics, Inc.."
#define UTILITY_COPYRIGHT       L"All Rights Reserved."
#define UTILITY_AUTHOR          L"Jeter_Wang/Stan_Chen"
#define UTILITY_LAST_MODIFY     L"2013/08/13"
#define UTILITY_LAST_MODIFIER   L"Jeter_Wang"
#define UTILITY_NAME            L"This program tool just support on Bay Trail-T with ENE IO373x."
#define UTILITY_VERSION         L"1.01"
//===================================================================

#define SHOW_INFO_BASE          8
#define SHOW_CURRENT_VER        SHOW_INFO_BASE
#define SHOW_UPDATED_VER        (SHOW_INFO_BASE+1)
#define SHOW_CHECKSUM           (SHOW_INFO_BASE+3)
#define SHOW_WARNING            (SHOW_INFO_BASE+5)
#define SHOW_ERASE              (SHOW_INFO_BASE+7)
#define SHOW_PROGRAM            (SHOW_INFO_BASE+9)
#define SHOW_VERIFY             (SHOW_INFO_BASE+11)
#define SHOW_FINAL_INFO         (SHOW_INFO_BASE+13)

#define PERCENT_SYMBOL          0x25 
  
#define	EC_32B                  0x20	
#define EC_128B                 0x80
#define EC_256B                 0x100
#define EC_512B                 0x200
#define EC_1KB                  0x400
#define EC_8KB                  0x2000
#define EC_32KB                 0x8000

#define IO373x_CHIPID           0x3730
#define ROM_VERSION_ADDR        0x6DF2

/*Compal EC HW Mode CMD - Start*/

#define	SET_ADDR_CMD            0x00
#define	READ_ADDR_CMD           0x11

#define WRITE_BYTE_CMD          0x01
#define WRITE_WORD_CMD          0x02
#define	WRITE_BLOCK_CMD         0x03


#define READ_BYTE_CMD           0x81
#define READ_WORD_CMD           0x82
#define READ_BLOCK_CMD          0x80
/*Compal EC HW Mode CMD - End*/

/*Compal EC HW Mode Address - Start*/
#define REG373_E51_RST          0xF010
#define	REG373_CODE_SEL         0xF011
#define	REG373_HOST_HS          0xF012
#define REG373_CHIPID_H         0xF01C
#define REG373_CHIPID_L         0xF01D

#define REG373_SMBDPF           0xFD12

#define REG373_WDTCFG           0xFE80
#define REG373_WDTPF            0xFE81
/*Compal EC HW Mode Address - End*/

/* SRAM CMD - Start */
#define	CHIP_ERASE              0x10
#define	PAGE_ERASE              0x11
#define WRITE_ROM               0x20
#define	READ_ROM                0x30
#define COMMAND_FINISH          0x80

#define	DEVICE_IDLE             0x02
/* SRAM CMD - End */


EFI_STATUS
EFIAPI
CompalGetChipID_IO373X (
  OUT UINT16                    *Chip_ID
  );
  
EFI_STATUS
EFIAPI
CompalDisableWatchDog_IO373X (
  );  

EFI_STATUS
EFIAPI
CompalGetAvaliableBuffer (
  OUT UINT16                    *AddrOffset
  );  

EFI_STATUS
EFIAPI
CompalCheckRegVal (
  IN UINT16                     AddrOffset,
  IN UINT8                      RegVal,
  IN UINT8                      RegMask
  );

EFI_STATUS
EFIAPI
CompalWaitBufferFree (
  IN UINT16                     *AddrOffset
  );  

VOID
EFIAPI
CompalGet_Updated_Version_IO373X (
  IN  UINT8                     *FileBuffer,
  OUT UINT32                    *EcVersion
  );
  
EFI_STATUS
EFIAPI
CompalErase_IO373X (
  );

EFI_STATUS
EFIAPI
CompalProgram_IO373X (
  IN  UINT8                     *FileBuffer
  );
  
EFI_STATUS
EFIAPI
CompalVerify_IO373X (
  IN  UINT32                    VerifyChecksum
  );

EFI_STATUS
EFIAPI
CompalFinish_IO373X (
  );


#endif  // __Compal_ECFlashI2C_H__
