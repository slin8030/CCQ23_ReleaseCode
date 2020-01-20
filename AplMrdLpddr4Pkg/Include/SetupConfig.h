/** @file

;******************************************************************************
;* Copyright (c) 2012-2014, Insyde Software Corp. All Rights Reserved.
;*
;* You may not reproduce, distribute, publish, display, perform, modify, adapt,
;* transmit, broadcast, present, recite, release, license or otherwise exploit
;* any part of this publication in any form, by any means, without the prior
;* written permission of Insyde Software Corporation.
;*
;******************************************************************************
*/

#ifndef _SETUP_CONFIG_H
#define _SETUP_CONFIG_H

#ifndef VFRCOMPILE
#include <Uefi/UefiInternalFormRepresentation.h>
#else
//
// BUGBUG. 
//    If running VfrCompile avoid to include Uefi.h & 
//    UefiInternalFormRepresentation.h even in KernelSetupConfig.h .
//        
//
#define __PI_UEFI_H__
#define __UEFI_INTERNAL_FORMREPRESENTATION_H__
#endif

#include <KernelSetupConfig.h>
#include <ChipsetSetupConfig.h>


#pragma pack(1)

//
//  Setup Utility Structure
//
//-----------------------------------------------------------------------------------------------------------------
// Important!!! The following setup utility structure should be syncronize with OperationRegion MBOX in mailbox.asi.
// If you do NOT follow it, you may face on unexpected issue. The total size are 700bytes.
// (Common 146bytes + Kernel 84bytes + Chipset 300bytes + OEM 70bytes + ODM 100bytes)
//-----------------------------------------------------------------------------------------------------------------

typedef struct {
  //
  // Kernel system configuration
  //
  #define _IMPORT_KERNEL_SETUP_
  #include <KernelSetupData.h>
  #undef _IMPORT_KERNEL_SETUP_


  //
  // Chipset system configuration (offset 230~700, total 470 bytes)
  //
  #define _IMPORT_CHIPSET_SPECIFIC_SETUP_
  #include <ChipsetSpecificSetupData.h>
  #undef _IMPORT_CHIPSET_SPECIFIC_SETUP_

//==========================================================================================================
//==========================================================================================================
//----------------------------------------------------------------------------------------------------------
// Start area for OEM team. The following area is used by OEM team to modify.
// The total size of variable in this part are fixed (70bytes). That means if you need to add or remove
// variables, please modify the OEMRSV buffer size as well.
//----------------------------------------------------------------------------------------------------------
//OEM_Start
//Offset(530);
//[-start-171225-IB07400935-modify]//
  UINT8         EcAslCodeSupport;
  UINT8         OEMRSV[69];                        // Reserve for OEM team
//[-end-171225-IB07400935-modify]//
//OEM_End
//----------------------------------------------------------------------------------------------------------
// End of area for OEM team use.
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
// Start area for ODM team. The following area is used by ODM to modify.
// The total size of variable in this part are fixed (100bytes). That means if you need to add or remove
// variables, please modify the ODMRSV buffer size as well.
//----------------------------------------------------------------------------------------------------------
//OEM_Start
//Offset(600);
  UINT8         ODMRSV[100];                       // Reserve for ODM
//OEM_End

//----------------------------------------------------------------------------------------------------------
// End of area for OEM team use.
//----------------------------------------------------------------------------------------------------------
} SYSTEM_CONFIGURATION;
#pragma pack()

//[-start-171225-IB07400935-modify]//
#ifndef CHIPSET_CONFIGURATION_SKIP_REDEFINE
#define CHIPSET_CONFIGURATION   SYSTEM_CONFIGURATION
#endif
//[-end-171225-IB07400935-modify]//

#define C_ASSERT(expn) typedef char __C_ASSERT__[(expn)?1:-1]
#endif

