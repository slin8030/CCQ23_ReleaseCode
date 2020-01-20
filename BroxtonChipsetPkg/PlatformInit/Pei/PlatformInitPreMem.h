/** @file
  The header file of Platform PEIM.


 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2017 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains a 'Sample Driver' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may be modified
  by the user, subject to the additional terms of the license agreement.

@par Specification
**/


//[-start-160517-IB03090427-modify]//
#ifndef __PLATFORM_INIT_PREMEM_H__
#define __PLATFORM_INIT_PREMEM_H__

//[-start-160817-IB03090432-modify]//
typedef struct  {
  UINT32  Sign           ; //Signiture#
  UINT32  CarBase        ; //Cache As Ram Base Address
  UINT32  CarSize        ; //Cache As Ram Size
  UINT32  IBBSource      ; //IBB Address in SRAM
  UINT32  IBBBase        ; //IBB Base in CAR.
  UINT32  IBBSize        ; //IBB Size
  UINT32  IBBLSource     ; //IBBL Address in SRAM
  UINT32  IBBLBase       ; //IBBL Base in CAR.
  UINT32  IBBLSize       ; //IBBL Size
  UINT32  FITBase        ; //FIT Base Address
  UINT32  TempRamBase    ; //PEI TempRamBase
  UINT32  TempRamSize    ; //PEI TempRamSize
  UINT32  HostToCse      ;
  UINT32  CseToHost      ;
  UINT32  ChunkIndex     ;
  UINT32  NumberOfChunks ;
  UINT32  IbbSizeLeft    ;
  UINT32  Chunksize      ;
  UINT64  IbblPerfRecord0; //The QWORD Performance record0 of IBBL
  UINT64  IbblPerfRecord1; //The QWORD Performance record1 of IBBL
  UINT64  IbblPerfRecord2; //The QWORD Performance record2 of IBBL
  UINT64  IbblPerfRecord3; //The QWORD Performance record3 of IBBL
  UINT64  IbblPerfRecord4; //The QWORD Performance record4 of IBBL
  UINT64  IbblPerfRecord5; //The QWORD Performance record5 of IBBL
//[-start-170516-IB08450375-add]//
  UINT32  BootPolicyExist;
  UINT32  TxeHashExist;
  UINT32  BootGuard;
  UINT8   TxeHash[32];
//[-end-170516-IB08450375-add]//
}CarMapStruc;
//[-end-160817-IB03090432-modify]//

//
//Function Prototypes Only - please do not add #includes here
//

/**
  This is the callback function notified by FvFileLoader PPI, it depends on FvFileLoader PPI to load
  the PEIM into memory.

  @param  PeiServices General purpose services available to every PEIM.
  @param  NotifyDescriptor The context of notification.
  @param  Ppi The notify PPI.

  @retval EFI_SUCCESS if it completed successfully.
**/
EFI_STATUS
EFIAPI
FvFileLoaderPpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );


EFI_STATUS
EFIAPI
SeCUmaEntry (
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
  );

//[-start-160317-IB03090425-modify]//
EFI_STATUS
EFIAPI
UpdateSaPreMemPolicy (
  IN OUT SA_PRE_MEM_CONFIG   *SaPreMemConfig
  );

EFI_STATUS
EFIAPI
UpdateNpkPreMemPolicy (
  IN OUT NPK_PRE_MEM_CONFIG   *NpkPreMemConfig
  );
//[-end-160317-IB03090425-modify]//

EFI_STATUS
EFIAPI
GetBiosReservedMemoryPolicy (
  IN CONST EFI_PEI_SERVICES               **PeiServices,
  IN PEI_BIOS_RESERVED_MEMORY_POLICY_PPI  *This,
  IN OUT BIOS_RESERVED_MEMORY_CONFIG      *BiosReservedMemoryPolicy
  );

EFI_STATUS
EFIAPI
MfgMemoryTest (
  IN  CONST EFI_PEI_SERVICES        **PeiServices,
  IN  PEI_MFG_MEMORY_TEST_PPI       *This,
  IN  UINT32                        BeginAddress,
  IN  UINT32                        MemoryLength
  );


BOOLEAN
IsRtcUipAlwaysSet (
  IN CONST EFI_PEI_SERVICES       **PeiServices
  );


VOID
RtcPowerFailureHandler (
  VOID
  );

  
VOID
ScBaseInit (
  VOID
  );

EFI_STATUS
UpdateBootMode (
  IN CONST EFI_PEI_SERVICES                       **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB                    *PlatformInfoHob
  );
  
/**
  This function performs Silicon Policy initialization.

  @param[in]  FirmwareConfiguration  It uses to skip specific policy init that depends
                                     on the 'FirmwareConfiguration' variable.
  @retval     EFI_SUCCESS            The PPI is installed and initialized.
  @retval     EFI ERRORS             The PPI is not successfully installed.
  @retval     EFI_OUT_OF_RESOURCES   Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
PeiSiPolicyInit (
  VOID
  );

//[-start-151204-IB02950555-modify]//  
//[-start-160406-IB07400715-modify]//
//#if BXTI_PF_ENABLE == 1
EFI_STATUS
ReadBxtIPlatformIds (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB    *PlatformInfoHob
  );

//#else

EFI_STATUS
ReadBxtPlatformIds (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN OUT EFI_PLATFORM_INFO_HOB    *PlatformInfoHob
  );
//#endif
//[-end-160406-IB07400715-modify]//
//[-end-151204-IB02950555-modify]//  
  
EFI_STATUS
EFIAPI
InstallMonoStatusCode (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  );

#endif
//[-end-160517-IB03090427-modify]//