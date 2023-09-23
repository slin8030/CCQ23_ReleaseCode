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
//[PRJ]+ >>>> Hidden T66 unsupported items in SCU and add T66 showAllPage function
//  UINT8         OEMRSV[70];                   // Reserve for OEM team
  #define _IMPORT_T66_COMMON_SETUP_
  #include <T66SetupConfig.h>                     // T66 common setup (offset 530~560, total 30 bytes)
  #undef _IMPORT_T66_COMMON_SETUP_
//Offset(560);
  UINT8         OEMRSV[40];                     // Reserve for OEM team
//[PRJ]+ <<<< Hidden T66 unsupported items in SCU and add T66 showAllPage function

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
  UINT8         ODMRSV[83];                       // Reserve for ODM
  UINT8         SlideShutDown;                     // Slide Shut Down
  UINT8         ATX_Flag;                             // ATX/AT
  UINT8         NCT5104;                          // NCT5104 detect
  UINT8         ComPortA_Pin9;                        // COM Pin9 :Ring/5V/12V
  UINT8         ComPortA_PULL;                        // COM Pull up/down
  UINT8         ComPortA_Termination;                 // COM Termination
  UINT8         ComPortB_Pin9;                        // COM Pin9 :Ring/5V/12V
  UINT8         ComPortB_PULL;                        // COM Pull up/down
  UINT8         ComPortB_Termination;                 // COM Termination
  UINT8         ComPortC_Pin9;                        // COM Pin9 :Ring/5V/12V
  UINT8         ComPortC_PULL;                        // COM Pull up/down
  UINT8         ComPortC_Termination;                 // COM Termination
  UINT8         ComPortD_Pin9;                        // COM Pin9 :Ring/5V/12V
  UINT8         ComPortD_PULL;                        // COM Pull up/down
  UINT8         ComPortD_Termination;                 // COM Termination

  UINT8         CompalFastBoot;                   // [COM][FastBoot]
  UINT8         CompalSecureBoot;                 // [COM][Compal SecureBoot]
//OEM_End

//----------------------------------------------------------------------------------------------------------
// End of area for OEM team use.
//----------------------------------------------------------------------------------------------------------
} SYSTEM_CONFIGURATION;
#pragma pack()

#define CHIPSET_CONFIGURATION   SYSTEM_CONFIGURATION

#define C_ASSERT(expn) typedef char __C_ASSERT__[(expn)?1:-1]

#define VFR_FORMID_SIO_RESOURCE         0x40

#endif

